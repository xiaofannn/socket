#include "stdafx.hpp"
#include <sys/epoll.h>

#define EPOLLSIZE 1024
#define SOCKETCLIENTSCOUNT 128
/*
epoll 优点:
1、不需要遍历 会告知 哪些fd 出现事件，所以不会随着连接数的增加 导致吞吐量变低。
2、 线程安全
function:
 1、epoll_crete(int size); 创建内核的fd 用来管理。
 2、epoll_ctl(epoll_fd,FLAGE,client_fd,evevent);
FLAGE:EPOLL_CTL_ADD、EPOLL_CTL_DEL、EPOLL_CTL_MOD
 3、int epoll_wait(int epollfd, struct epoll_event * events, int maxevents, int
timeout);

strcut:
struct epoll_event {
    __uint32_t events;  Epoll events
    epoll_data_t data;  User data variable
};
*/

static void addEvent(int epoll_fd, int add_fd, int event);
static void handle_event(int epoll_fd, epoll_event *events, int num,
                         int listen_fd);
static void do_read(int epoll_fd, int client_fd);
static void handle_accept(int epoll_fd, int listen_fd);
static void delete_event(int epoll_fd, int client_fd, int event);

int main(int argc, char const *argv[]) {
  int listen_fd = socket_bind(IPADDRESS, PORT);

  int epoll_fd = epoll_create(1024);
  epoll_event events[SOCKETCLIENTSCOUNT];
  addEvent(epoll_fd, listen_fd, EPOLLIN);

  while (true) {
    int ret = epoll_wait(epoll_fd, epoll_events, EPOLLSIZE, -1);
    handle_event(epoll_fd, epoll_events, ret, listen_fd);
  }
  return 0;
}

static void addEvent(int epoll_fd, int add_fd, int event) {
  epoll_event ev;
  ev.events = event;
  ev.data.fd = add_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &ev);
}

static void handle_event(int epoll_fd, epoll_wait *events, int num,
                         int listen_fd) {
  for (size_t i = 0; i < num; i++) {
    int fd = events[i].data.fd;

    if ((fd == listen_fd) && (events[i].events & EPOLLIN))
      handle_accept(epoll_fd, listen_fd);
    else if (events[i].events & EPOLLIN)
      do_read(events[i].data.fd);
  }
}

static void handle_accept(int epoll_fd, int listen_fd) {
  int client_fd = accept(listen_fd, NULL, NULL);
  addEvent(epoll_fd, client_fd, EPOLLIN);
}

static void do_read(int epoll_fd, int client_fd) {
  char buf[1024] = {0};
  int nread = read(client_fd, buf, 1024);

  if (nread <= 0) {
    perror("read filed:");
    close(client_fd);
    delete_event(epoll_fd, client_fd, EPOLLIN);
  } else {
    printf("read: %s.\n", buf);
    size_t size = strlen(buf);

    if (size != write(client_fd, buf, strlen(buf))) {
      perror("write filed:");
      close(client_fd);
      delete_event(epoll_fd, client_fd, EPOLLIN);
    }
  }
}

static void delete_event(int epoll_fd, int client_fd, int event) {
  epoll_event ev;
  ev.events = event;
  ev.data.fd = client_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, &ev);
}
