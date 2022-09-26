#include <enet/enet.h>
#include <iostream>
#include <cstring>

void send_time_packet(ENetPeer *peer, uint32_t time);
void send_start(ENetPeer *peer);

int enet_init_check ();
int enet_client_check(ENetHost* client);
int enet_lobby_check(ENetPeer* lobby);

int main(int argc, const char **argv)
{
  srand(time(nullptr));
  enet_init_check();

  uint32_t startMatchTime = 0;
  if (argc == 2)
  {
    printf("Start session after %s seconds\n", argv[1]);
    startMatchTime = atoi(argv[1]) * 1000;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10887;
  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
  ENetPeer *lobbyPeer = enet_host_connect(client, &address, 2, 0);
  ENetPeer *serverPeer = nullptr;

  enet_client_check(client);
  enet_lobby_check(lobbyPeer);
  
  uint32_t timeStart = enet_time_get();
  uint32_t lastFragmentedSendTime = timeStart;
  uint32_t lastMicroSendTime = timeStart;
  bool connected = false;
  
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        if (!serverPeer && event.peer->address.host == address.port)
        {
          char *buffer = (char *)event.packet->data;
          char *save;
          char *host = strtok_r(buffer, " ", &save);
          char *port = strtok_r(nullptr, " ", &save);
          ENetAddress serverAddress;
          enet_address_set_host(&serverAddress, host);
          serverAddress.port = atoi(port);
          serverPeer = enet_host_connect(client, &serverAddress, 2, 0);
          startMatchTime = 0;
          if (!serverPeer)
          {
            printf("Cannot connect to server\n");
            return 1;
          }
        }
        printf("Packet received '%s' From:%x\n", event.packet->data, event.peer->address.host);
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    if (connected)
    {
      uint32_t curTime = enet_time_get();
      if (curTime - lastMicroSendTime > 1000 && serverPeer)
      {
        lastMicroSendTime = curTime;
        send_time_packet(serverPeer, curTime);
      }
      if (curTime -  timeStart > startMatchTime && startMatchTime > 0)
      {
        printf("Send start\n");
        send_start(lobbyPeer);
        startMatchTime = 0;
      }
    }
  }
  return 0;
}

void send_time_packet(ENetPeer *peer, uint32_t time)
{
  const std::string message = std::to_string(time + rand() % 100);

  ENetPacket *packet = enet_packet_create(message.c_str(), strlen(message.c_str()), ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}

void send_start(ENetPeer *peer)
{
  const std::string message = "Start!";
  ENetPacket *packet = enet_packet_create(message.c_str(), strlen(message.c_str()), ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
}

int enet_init_check()
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
}

int enet_client_check(ENetHost* client)
{
  if (!client)
    {
      printf("Cannot create ENet client\n");
      return 1;
    }
}

int enet_lobby_check(ENetPeer* lobby)
{
  if (!lobby)
  {
    printf("Cannot connect to lobby");
    return 1;
  }
}