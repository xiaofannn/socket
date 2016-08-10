#include "stdafx.hpp"

#include <poll.h>
/*
主要将其变成 使用 pollfd的结构体,内部结构
struct pollfd{
int fd;      //套接字
short  events; //感兴趣的事件 用户态
short revents; //发生的事件  内核态
}
相对于 select 模型的优点:
1、去除了 1024的连接数量限制
2、内核不会修改传入的数组,不需要像select 每次都需要copy
依旧存在缺点:
1、不会告知具体的fd 可读，依旧需要遍历
2、依旧是线程不安全
*/

#define SOCKETCLIENTSCOUNT 128

int main(int argc, char const *argv[]) {
  int listen_fd = socket_bind(IPADDRESS, PORT);
  struct pollfd clients_fd[SOCKETCLIENTSCOUNT];

  clients_fd[0].fd = listen_fd;
  clients_fd[0].events = POLLIN;
  for (size_t i = 1; i < SOCKETCLIENTSCOUNT; ++i) {
    clients_fd[i].fd = -1;
  }

  int total_fd = 1;
  while (true) {
    int nread = poll(clients_fd, total_fd, -1);
    if (nread == -1) {
      perror("poll error:");
      exit(1);
    }

    if (clients_fd[0].revents & POLLIN) {
      int client_fd = accept(clients_fd[0].fd, NULL, NULL);
      if (client_fd == -1) {
        perror("accpet filed");
        exit(1);
      }

      size_t i = 0;
      for (; i < SOCKETCLIENTSCOUNT; ++i) {
        if (clients_fd[i].fd == -1) {
          clients_fd[i].fd = client_fd;
          clients_fd[i].events = POLLIN;
          ++total_fd;
          break;
        }
      }

      if (i == SOCKETCLIENTSCOUNT) {
        close(client_fd);
        printf("too much clients\n");
      }

      if (--nread <= 0)
        continue;
    }

    for (size_t i = 1; i < SOCKETCLIENTSCOUNT; ++i) {
      if (clients_fd[i].fd != -1 && (clients_fd[i].revents & POLLIN)) {
        char buf[1024] = {0};
        int nread = read(clients_fd[i].fd, buf, 1024);
        if (nread <= 0) {
          close(clients_fd[i].fd);
          --total_fd;
          clients_fd[i].fd = -1;
        } else {
          write(clients_fd[i].fd, buf, strlen(buf));
        }

        if (--nread <= 0)
          continue;
      }
    }
  }
  return 0;
}
