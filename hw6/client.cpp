#include <enet/enet.h>

#include "enet_checkers.h"
#include "utilities.h"

int main(int argc, const char **argv)
{
    message msg = {};

    std::string command;
    std::string myName;
    uint myId;
    bool inMatchmaker = false;
    bool inGame = false;
    bool connected = false;

    enetInit();

    ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
    enetHostCreateCheck(client);

    ENetAddress address;
    enet_address_set_host(&address, matchmakerHost);
    address.port = matchmakerPort;

    ENetPeer *serverPeer = enet_host_connect(client, &address, 2, 0);
    enetHostConnectCheck(serverPeer);

    uint32_t timeStart = enet_time_get();
    uint32_t lastUpdateSendTime = timeStart;

    while (true)
    {
        command = getCommand(int (' '));
        if (command == roomsCommand)
        {
            std::cout << "Refreshing room list" << std::endl;
            msg.type = MessageType::CLIENT_TO_MATCHMAKER_GET_ROOM_LIST;
            msg.data = {};
            send_message(&msg, serverPeer, 0, true);
        }
        else if (command == selectCommand)
        {
            int i;
            std::cout << "Type desired room number from list:" << std::endl;
            std::cin >> i;
            std::cout << "Joining room " << i << std::endl;
            msg.type = MessageType::CLIENT_TO_MATCHMAKER_SELECT_ROOM;
            msg.data = {std::to_string(i)};
            send_message(&msg, serverPeer, 0, true);
        }
        else if (command == leaveCommand)
        {
            std::cout << "Leaving current room" << std::endl;
            msg.type = MessageType::CLIENT_TO_MATCHMAKER_LEAVE_ROOM;
            msg.data = {};
            send_message(&msg, serverPeer, 0, true);
        }

        ENetEvent event;
        while (enet_host_service(client, &event, 10) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Connection with " << event.peer->address.host << ":" << event.peer->address.port << " established" << std::endl;
                    enetHostMatchmakingCreateCheck(event.peer->address.port, matchmakerPort, inMatchmaker, msg, serverPeer);
                    connected = true;
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                    data_to_message(event.packet->data,msg);
                    if (inMatchmaker)
                    {
                        if (msg.type == MessageType::MATCHMAKER_TO_CLIENT_HOST_LINK)
                        {
                            enet_address_set_host(&address, msg.data[0].c_str());
                            address.port = std::atoi(msg.data[1].c_str());
                            serverPeer = enet_host_connect(client, &address, 2, 0);
                            enetHostConnectClientCheck(serverPeer, inMatchmaker, inGame);
                        }
                        else if (msg.type == MessageType::MATCHMAKER_TO_CLIENT_MESSAGE)
                        {
                            std::cout << "MATCHMAKER: " << msg.data[0] << std::endl;
                        }
                        else if (msg.type == MessageType::MATCHMAKER_TO_CLIENT_ROOM_LIST)
                        {
                            int n = std::stoi(msg.data[0]);
                            std::cout << "ROOM LIST:" << std::endl;
                            printf("name\t\t\t\tplayers\tdifficulty\n");
                            for (int i=0;i<n;i++)
                            {
                                std::string name = msg.data[1+i*4];
                                uint pl = std::stoi(msg.data[2+i*4]);
                                uint total = std::stoi(msg.data[3+i*4]);
                                uint diff = std::stoi(msg.data[4+i*4]);
                                printf("%s\t\t\t\t%u/%u\t%u\n",name.c_str(),pl,total,diff);
                            }
                        }
                    }
                    else if (inGame)
                    {
                        if (msg.type == MessageType::SERVER_TO_CLIENT_INIT)
                        {
                            myId = std::stoi(msg.data[0]);
                            myName = msg.data[1];
                            std::cout << "My ID: " << myId << "; " << "My name: " << myName << std::endl;
                            std::cout << "Other players (total " << (msg.data.size()-2)/2 << "):" << std::endl;
                            for (int i = 0; i < (msg.data.size()-2)/2; ++i)
                                std::cout << "(" << msg.data[3+2*i] << ") [" << msg.data[2+2*i] << "]" << std::endl;

                        }
                        else if (msg.type == MessageType::SERVER_TO_CLIENT_NEW_CONNECTED)
                        {
                            std::cout << "New Player connected (" << msg.data[1] << ") [" << msg.data[0] << "]" << std::endl;
                        }
                        else if (msg.type == MessageType::SERVER_TO_CLIENT_UPDATE)
                        {
                            std::cout << "Server time " << msg.data[0] << std::endl;
                        }
                        else if (msg.type == MessageType::SERVER_TO_CLIENT_PING_LIST)
                        {
                            std::cout << "PING LIST (id:ping): ";
                            for (int i = 0; i < (msg.data.size())/2; ++i)
                                std::cout << "(" << msg.data[2*i] << ":" << msg.data[2*i+1] << ")";
                            std::cout << std::endl;
                        }
                    }
                    enet_packet_destroy(event.packet);  
                break;
                default:
                break;
            }
        }
        if (connected && inGame)
        {
            uint32_t curTime = enet_time_get();
            if (curTime - lastUpdateSendTime > 1000)
            {
                lastUpdateSendTime = curTime;
                msg = {.type = MessageType::CLIENT_TO_SERVER_UPDATE,.data = {std::to_string(curTime-timeStart)}};
                std::cout << "SENT MESSAGE TO SERVER" << std::endl;
                send_message(&msg,serverPeer,0,true);
            }
        }
    }
    return 0;
}