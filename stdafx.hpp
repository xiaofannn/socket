#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>  // standard IO
#include <stdlib.h> //standard  libary
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> //unix standard

#define IPADDRESS "127.0.0.1"
#define PORT 8787
#define LISTENQ 5

int socket_bind(const char *ip, int port) {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    perror("create socket error");
    exit(1);
  }

  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr(ip);

  if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind socket error");
    exit(1);
  }

  if (listen(listen_fd, LISTENQ) == -1) {
    perror("listen socket error");
    exit(1);
  }
  printf("listen success\n");
  return listen_fd;
}

int socket_connect(const char *ip, int port) {
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    perror("create socket filed.");
    exit(1);
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);

  if (connect(client_fd, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("connect server filed");
    exit(1);
  }

  printf("connect success\n");
  return client_fd;
}
