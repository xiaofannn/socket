#include "stdafx.hpp"

#include <sys/select.h> // select IO multiplexing model

#define SOCKETCLIENTSCOUNT 128
/*
IO multiplexing 的好处是可以管理多个IO流，反馈速度肯定没有pre thread 快.
好处就是一个thread 能够管理大量的 IO Steam
*/
/*
select model 是最简单的 也存在致命的缺点
1、每次都需要使用 FD_SET 将用户态的 fd copy到内核态
2、linux 存在着限制 只能使用1024的 FD_SETSIZE。
3、不会告知具体 fd 可读，需要将所有都遍历。 FD_ISSET() 函数
4、线程不安全,如果在其它线程close 则会导致不可预测
*/
static void accpet_client(int *clients_fd, int listen_fd);
static void recv_client_msg(int *clients_fd, fd_set *readfds);
static void handle_client_msg(int fd, char *buf);

int main(int argc, char const *argv[]) {
  int listen_fd = socket_bind(IPADDRESS, PORT);

  int max_fd = -1;
  fd_set readfds;
  int clients_fd[SOCKETCLIENTSCOUNT];
  memset(clients_fd, -1, sizeof(clients_fd));

  while (true) {
    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);
    max_fd = listen_fd;

    for (size_t i = 0; i < SOCKETCLIENTSCOUNT; ++i) {
      if (clients_fd[i] != -1) {
        FD_SET(clients_fd[i], &readfds);
        max_fd = clients_fd[i] > max_fd ? clients_fd[i] : max_fd;
      }
    }

    int nready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (nready == -1) {
      perror("select error.");
      return 1;
    }

    if (FD_ISSET(listen_fd, &readfds)) {
      accpet_client(clients_fd, listen_fd);
    } else {
      recv_client_msg(clients_fd, &readfds);
    }
  }

  return 0;
}

static void accpet_client(int *clients_fd, int listen_fd) {
  int client_fd = accept(listen_fd, NULL, NULL);
  if (client_fd == -1) {
    printf("accept failed: %s.\n", strerror(errno));
    return;
  } else {
    printf("new client accpeted\n");
  }

  size_t i = 0;
  for (; i < SOCKETCLIENTSCOUNT; ++i) {
    if (clients_fd[i] == -1) {
      clients_fd[i] = client_fd;
      break;
    }
  }

  if (i == SOCKETCLIENTSCOUNT) {
    close(client_fd);
    printf("too many clients connectioned \n");
  }
}

static void recv_client_msg(int *clients_fd, fd_set *readfds) {
  char buf[1024] = {0};

  for (size_t i = 0; i < SOCKETCLIENTSCOUNT; ++i) {
    if (clients_fd[i] == -1) {
      continue;
    } else if (FD_ISSET(clients_fd[i], readfds)) {
      int n = read(clients_fd[i], buf, 1024);
      if (n <= 0) {
        FD_CLR(clients_fd[i], readfds);
        printf("one socket close\n");
        close(clients_fd[i]);
        clients_fd[i] = -1;
        continue;
      }
      handle_client_msg(clients_fd[i], buf);
    }
  }
}

static void handle_client_msg(int fd, char *buf) {
  assert(buf);
  printf("recv buf is:%s", buf);
  write(fd, buf, strlen(buf));
}
