#include <enet/enet.h>

#include "enet_checkers.h"
#include "utilities.h"

struct user {
    uint id;
    std::string name;
    ENetPeer * peer;
};


int main(int argc, const char **argv)
{
  std::vector<user> users;
  user newUser;

  message msg;

  uint gameServerPort;
  uint gameDifficulty = 0;

  portCheck(argc, argv, gameServerPort);
  enetInit();

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = gameServerPort;
  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  enetHostCreateCheck(server);
  enet_address_set_host(&address, matchmakerHost);
  address.port = matchmakerPort;

  ENetPeer *serverPeer = enet_host_connect(server, &address, 2, 0);
  enetHostConnectCheck(serverPeer);

  uint32_t timeStart = enet_time_get();
  uint32_t lastUpdateSendTime = timeStart;

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      switch (event.type)
      {
          case ENET_EVENT_TYPE_CONNECT:
            if (event.peer->address.port == matchmakerPort)
            {
                std::cout << "Connected to matchmaker. Waiting for start" << std::endl;
                msg.type = MessageType::SERVER_TO_MATCHMAKER_READY;
                msg.data = {std::to_string(gameServerPort)};
                send_message(&msg,event.peer,0,true);
            }
            else
            {
                newUser = {.id = 100+(uint)users.size(),
                           .name = "Player"+std::to_string(users.size()),
                           .peer = event.peer};

                users.push_back(newUser);

                std::cout << "Connection with " << event.peer->address.host << ":" << event.peer->address.port << " established. ID: " 
                          << newUser.id << "; Name: " << newUser.name << std::endl;

                msg.type = MessageType::SERVER_TO_CLIENT_INIT;
                msg.data = {std::to_string(newUser.id),newUser.name};

                for (int i = 0; i < users.size()-1; ++i)
                {
                    msg.data.push_back(std::to_string(users[i].id));
                    msg.data.push_back(users[i].name);
                }

                send_message(&msg, newUser.peer, 0, true);

                msg.type = MessageType::SERVER_TO_CLIENT_NEW_CONNECTED;
                msg.data = {std::to_string(newUser.id),newUser.name};

                for (int i = 0; i < server->connectedPeers; ++i)
                {
                    if (&server->peers[i] != event.peer)
                        send_message(&msg, &server->peers[i], 0, true);
                }
            }
            break;
          case ENET_EVENT_TYPE_RECEIVE:
            data_to_message(event.packet->data,msg);
            if (msg.type == MessageType::MATCHMAKER_TO_SERVER_START)
            {    
                gameDifficulty = std::stoi(msg.data[0]);
                std::cout << "STARTING GAME with difficulty " << gameDifficulty << std::endl;
            }
            if (msg.type == MessageType::CLIENT_TO_SERVER_UPDATE)
            {
                for (int k = 0; k < users.size(); ++k)
                {
                    if (users[k].peer==event.peer)
                    {
                        std::cout << "Player " << users[k].name << " time: " << msg.data[0] << std::endl;
                        break;
                    }
                }
            }
            enet_packet_destroy(event.packet);
            break;
          default:
            break;
      };
    }
    if (server->connectedPeers>0)
    {
      uint32_t curTime = enet_time_get();

      if (curTime - lastUpdateSendTime > 1000)
      {
        lastUpdateSendTime = curTime;
        msg = {.type = MessageType::SERVER_TO_CLIENT_UPDATE,.data = {std::to_string(curTime-timeStart)}};

        for (int i = 0; i < server->connectedPeers; ++i)
            send_message(&msg, &server->peers[i], 0, false);

        msg.type = MessageType::SERVER_TO_CLIENT_PING_LIST;
        msg.data = {};

        for (int i = 0; i < users.size(); ++i)
        {
            msg.data.push_back(std::to_string(users[i].id));
            msg.data.push_back(std::to_string(users[i].peer->roundTripTime));
        }
        for (int i = 0; i < server->connectedPeers; ++i)
            send_message(&msg, &server->peers[i], 0, false);
      }
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}