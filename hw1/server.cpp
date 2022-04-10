#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include "socket_tools.h"

struct UserInfo 
{
    struct sockaddr from;
    uint32_t addr_len;
    std::string name;
};

std::vector <UserInfo> info;

int find_user(const std::string &name);

void send_message_from_user(int sfd, const std::string &name, const std::string &message);

int main(int argc, const char **argv)
{
  constexpr size_t buf_size = 1000;
  static char buffer[buf_size];
  memset(buffer, 0, buf_size);

  const char *port = "2022";

  int sfd = create_dgram_socket(nullptr, port, nullptr);

  if (sfd == -1)
    return 1;
  printf("listening!\n");

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd + 1, &readSet, NULL, NULL, &timeout);

    if (FD_ISSET(sfd, &readSet))
    {
      struct sockaddr from;
      uint32_t addr_len = sizeof(from);
      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, &from, &addr_len);

      if (numBytes > 0)
      {
        char* name;
        char* code;
        char* save;
        int user_id;
        code = strtok_r(buffer, " ", &save);
        name = strtok_r(nullptr, " ", &save);

        if (buffer[0] == '0')
        {
          int idx = find_user(name);
          if (idx == -1)
          {
            printf("Add new user: %s\n", name);
            info.push_back({from, addr_len, name});
          }
          else
          {
            printf("Info for user with id %d info updated: %s\n", user_id, name);
            info[idx].from = from;
            info[idx].addr_len = addr_len;
          }
        }
        else if (buffer[0] == '1')
        {
          printf("From: %s get message: %s \n", name, save);
          send_message_from_user(sfd, std::string(name), save);
        }
        else if (buffer[0] == '2')
        {
          printf("Get keep alive packet from: %s\n", name);
        }
        else
        {
          printf("Read invalid message: %s\n", buffer);
        }
      }
    }
  }
  return 0;
}

void send_message_from_user(int sfd, const std::string &name, const std::string &message)
{
 for (int i = 0; i < info.size(); ++i)
  {
    if (name != info[i].name)
    {
      std::string output = name + ": " + message;
      sendto(sfd, output.c_str(), output.size(), 0, &info[i].from, info[i].addr_len);
    }
  }
}

int find_user (const std::string &name)
{
  for (int i = 0; i < info.size(); ++i)
  {
    if (name == info[i].name)
      return i;
  }
  return -1;
}