#include "matchmaking_server.h"

message genRoomListMessage(std::vector<room>& rooms)
{
    message msg;
    msg.type = MessageType::MATCHMAKER_TO_CLIENT_ROOM_LIST;
    msg.data = {std::to_string(rooms.size())};

    for (int i = 0; i<rooms.size(); ++i)
    {
        msg.data.push_back(rooms[i].name);
        msg.data.push_back(std::to_string(rooms[i].players.size()));
        msg.data.push_back(std::to_string(rooms[i].maxPlayers));
        msg.data.push_back(std::to_string(rooms[i].difficulty));
    }
    return msg;
}

void printRooms(std::vector<room>& rooms)
{
    std::cout << "ROOMS:" << std::endl;
    printf("name\t\t\t\tplayers\tdifficulty\n");
    for (int i = 0; i<rooms.size(); ++i)
        printf("%s\t\t\t\t%d/%u\t%d\n",rooms[i].name.c_str(),(int)rooms[i].players.size(),rooms[i].maxPlayers,rooms[i].difficulty);
}

int main(int argc, const char **argv)
{
    std::vector<room> rooms;
    std::vector<gameServer> readyServers;

    message msg;
    uint userId;
    std::string command;

    void enetInit();
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = matchmakerPort;

    ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);
    enetHostMatchmakingCreateCheck(server);

    rooms.push_back(initRoom("[EN] MATCH.com",3,2));
    rooms.push_back(initRoom("[RU] HARD GAME",4,5));
    rooms.push_back(initRoom("for n00bs",8,0));
    rooms.push_back(initRoom("[EN] hi!",3,3));

    printRooms(rooms);

    while (true)
    {

        command = getCommand(int (' '));
        if (command == startRoomCommand)
        {
            int roomN;
            std::cout << "Type desired room number from list:" << std::endl;
            std::cin >> roomN;
            std::cout << "Starting room " << roomN << std::endl;

            if (roomN>=rooms.size())
            {
                std::cout << "Room doesn't exist" << std::endl;
            }
            else if (readyServers.size()==0)
            {
                std::cout << "ERROR, there are no ready servers. Start a new server." << std::endl;
            }
            else
            {
                gameServer gameServer = readyServers.back();
                readyServers.pop_back();
                msg.type = MessageType::MATCHMAKER_TO_SERVER_START;
                msg.data = {std::to_string(rooms[roomN].difficulty)};
                send_message(&msg,gameServer.peer,0,true);

                std::vector<ENetPeer*> peers;

                for (int i = 0; i < rooms[roomN].players.size(); ++i)
                {
                    for (int j = 0; j < server->connectedPeers; ++j)
                    if (server->peers[j].connectID == rooms[roomN].players[i])
                    {
                        peers.push_back(&server->peers[j]);
                    }
                }
                for (int i = 0; i < peers.size(); ++i)
                {
                    msg.type = MessageType::MATCHMAKER_TO_CLIENT_HOST_LINK;
                    msg.data = {gameServerHost, std::to_string(gameServer.port)};
                    send_message(&msg, peers[i], 0, true);
                }
                rooms.erase(rooms.begin() + roomN);
                std::cout << "ROOM STARTED" << std::endl;
                printRooms(rooms);
            }
        }
        if (command == createRoomCommand)
        {
            std::string name;
            int pl, d;
            std::cout << "Type desired room name:" << std::endl;
            std::cin >> name;
            std::cout << "Type max players number:" << std::endl;
            std::cin >> pl;
            std::cout << "Type gamemode difficulty (0-8):" << std::endl;
            std::cin >> d;

            rooms.push_back(initRoom(name,pl,d));
            std::cout << "ROOM ADDED" << std::endl;
            printRooms(rooms);
        }
        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Connection with " << event.peer->address.host << ":" event.peer->address.port << " established" << std::endl;

                break;
                case ENET_EVENT_TYPE_RECEIVE:
                    userId = event.peer->connectID;
                    data_to_message(event.packet->data,msg);

                    if (msg.type == MessageType::SERVER_TO_MATCHMAKER_READY)
                    {
                        uint newPort = stoi(msg.data[0]);

                        gameServer ns;
                        ns.peer = event.peer;
                        ns.port = newPort;
                        readyServers.push_back(ns);
                        std::cout << "New game server ready:" << newPort << std::endl;
                    }
                    if (msg.type == MessageType::CLIENT_TO_MATCHMAKER_GET_ROOM_LIST)
                    {
                        msg = genRoomListMessage(rooms);
                        send_message(&msg,event.peer,0,true);
                    }
                    else if (msg.type == MessageType::CLIENT_TO_MATCHMAKER_SELECT_ROOM)
                    {
                        bool inRoom = false;
                        for (int i = 0; i < rooms.size(); ++i)
                            for (int j = 0; j < rooms[i].players.size(); ++j)
                                if (rooms[i].players[j] == userId)
                                    inRoom = true;
                        uint roomN = stoi(msg.data[0]);
                        std::cout << "Player " << userId <<" trying to join room " << roomN << std::endl;

                        msg.type = MessageType::MATCHMAKER_TO_CLIENT_MESSAGE;
                        if (inRoom)
                            {
                                std::cout << "Player is already in some room" << std::endl;
                                msg.data = {"You're already in some room"};

                            }
                        else if (roomN >= rooms.size())
                        {
                                std::cout << "Room doesn't exist" << std::endl;
                                msg.data = {"Room doesn't exist"};
                        }
                        else if (rooms[roomN].players.size() >= rooms[roomN].maxPlayers)
                        {
                                std::cout << "Room is full" << std::endl;
                                msg.data = {"Room is full"};
                        }
                        else
                        {                    
                            std::cout << "Player is added to the room" << std::endl;
                            rooms[roomN].players.push_back(userId);
                            msg.data = {"You're added to the room"};
                            printRooms(rooms);
                        }
                        send_message(&msg, event.peer, 0, true);
                    }
                    else if (msg.type == MessageType::CLIENT_TO_MATCHMAKER_LEAVE_ROOM)
                    {
                        msg.type = MessageType::MATCHMAKER_TO_CLIENT_MESSAGE;
                        msg.data = {"You're not in any room"};
                        for (int i = 0; i < rooms.size(); ++i)
                        {
                            int pos = -1;
                            for (int j = 0; j < rooms[i].players.size(); ++j)
                                if (rooms[i].players[j] == userId)
                                    pos = j;
                            if (pos != -1)
                            {
                                rooms[i].players.erase(rooms[i].players.begin()+pos);
                                msg.data = {"You left the room"};
                            }
                        }
                        send_message(&msg,event.peer,0,true);
                    }
                    enet_packet_destroy(event.packet);
                break;
                default:
                break;
            }
        }
    }
    enet_host_destroy(server);
    atexit(enet_deinitialize);
    return 0;
}
