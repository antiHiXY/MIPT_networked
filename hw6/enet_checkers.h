#pragma once

#include <enet/enet.h>

#include <iostream>


void portCheck(int argc, const char **argv, uint& gameServerPort)
{
    if (argc > 1)
    {
        gameServerPort = strtol(argv[1],nullptr,10);
        std::cout << "Port: " << gameServerPort << std::endl;
    }
    else
    {
        std::cout << "Port not specified" << std::endl;
        exit(1);
    }
}

void enetInit()
{
    if (enet_initialize() != 0)
    {
        std::cout << "Cannot init ENet" << std::endl;
        exit(1);
    }
}

void enetHostCreateCheck(ENetHost * host)
{
    if (!host)
    {
        std::cout << "Cannot create ENet server" << std::endl;
        exit(1);
    }
    else
    {  
        std::cout << "Game server started" << std::endl;
    }
}

void enetHostMatchmakingCreateCheck(ENetHost * host)
{
    if (!host)
    {
        std::cout << "Cannot create ENet server" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "Matchmaker started" << std::endl;
        std::cout << "~~~~~" << std::endl;
        std::cout << "Command list:" << std::endl;
        std::cout << "/start - Start room" << std::endl;
        std::cout << "/create - Create room" << std::endl;
        std::cout << std::endl;
        std::cout << "Press SPACE to start writing command" << std::endl;
        std::cout << "~~~~~" << std::endl;
    }
}

void enetHostMatchmakingCreateCheck(enet_uint16 port, const int matchmakerPort, bool& inMatchmaker, message& msg)
{
    inMatchmaker = true;

    std::cout << "Connected to matchmaker" << std::endl;
    std::cout << "~~~~~" << std::endl;
    std::cout << "Command list:" << std::endl;
    std::cout << "/rooms - Get room list" << std::endl;
    std::cout << "/select - Select room" << std::endl;
    std::cout << "/leave - Leave room" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Press SPACE to start writing command" << std::endl;
    std::cout << "~~~~~" << std::endl;

    msg.type = MessageType::CLIENT_TO_MATCHMAKER_GET_ROOM_LIST;
    msg.data = {};
    send_message(&msg, serverPeer, 0, true);
}

void enetHostConnectCheck(ENetPeer * peer)
{
    if (!peer)
    {
        std::cout << "Cannot connect to matchmaker" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "Connecting to matchmaker" << std::endl;
    }
}

void enetHostConnectClientCheck(ENetPeer * peer, bool& inMatchmaker, bool& inGame)
{
    if (!peer)
    {
        std::cout << "Cannot connect to server" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "Connected to server" << std::endl;
        inMatchmaker = false;
        inGame = true;
    }
}