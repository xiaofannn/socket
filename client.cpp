#include "stdafx.hpp"
#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  int client_fd = socket_connect(IPADDRESS, PORT);
  std::string buf;
  while (true) {
    std::getline(std::cin, buf);
    if (buf == "q") {
      close(client_fd);
      break;
    } else {
      size_t size = buf.size();
      if (size != write(client_fd, buf.c_str(), size)) {
        perror("send buf error");
      }

      char read_buf[1024] = {0};
      int nread = read(client_fd, read_buf, 1024);
      if (nread <= 0) {
        close(client_fd);
        break;
      } else {
        printf("recv %s.\n", read_buf);
      }
    }
  }
  return 0;
}
