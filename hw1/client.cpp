#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <array>
#include "socket_tools.h"

/*
constexpr size_t buf_size = 1000;
static char buffer[buf_size];
std::memset(buffer, 0, buf_size);
*/

enum class Condition {init, type1, type2, type3};

const char * initialization = "0 ";
const char * messaging = "1 ";
const char * keeping_alive = "2 ";

std::array<std::string, 4> type1_words {"Hi", "It's me", "Wow", "Fine"};
std::array<std::string, 4> type2_words {"Hello there!", "General Kenobi", "You are a bold one", "Nope"};
std::array<std::string, 5> type3_words {"Why", "are", "you", "bulling", "me"};

int main(int argc, const char **argv)
{
  const char *port = "2022";
  Condition condition = Condition::init;
  std::string user_name;
  char type;
  std::string output;
  std::string input;
  addrinfo resAddrInfo;

  int sfd = create_dgram_socket("localhost", port, &resAddrInfo);

  if (sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }

  uint32_t t_start = time(nullptr);
  uint32_t send_time = t_start;
  uint32_t keep_alive_time = t_start;

  printf("Hi what your name?\n");
  std::getline(std::cin, user_name);
  printf("Choose message type 1 or 2\n");
  std::cin >> type;

  switch (type)
  {
    case '1':
      condition = Condition::type1;
      break;
    case '2':
      condition = Condition::type2;
      break;
    default:
      condition = Condition::type3;
      break;
  }

  output = initialization + user_name;
  ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
  if (res == -1)
      std::cout << strerror(errno) << std::endl;

  while (true)
  {
    uint32_t current_time = time(nullptr);

    if (current_time - send_time > 0)
    {
      send_time = current_time;
      switch (condition)
      {
        case Condition::type1:
          input = type1_words[rand() % type1_words.size()];
          output = messaging + user_name + " " + input;
          break;
        case Condition::type2:
          input = type2_words[rand() % type2_words.size()];
          output = messaging + user_name + " " + input;
          break;
        case Condition::type3:
          input = type3_words[rand() % type3_words.size()];
          output = messaging + user_name + " " + input;
          break;
      }
      printf(">%s \n", input.c_str());
      ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
      if (res == -1)
        std::cout << hstrerror(errno) << std::endl;
    }
    if (current_time - keep_alive_time > 4)
    {
      keep_alive_time = current_time;
      output = keeping_alive + user_name;
      ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
      if (res == -1)
        std::cout << hstrerror(errno) << std::endl;
    }
    /*
    ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, nullptr, nullptr);
    if (numBytes > 0)
    {
      printf("%s\n", buffer);
    }
    */
  }
  return 0;
}
