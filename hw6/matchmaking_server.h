#pragma once

#include <enet/enet.h>

#include "utilities.h"

struct room 
{
    std::string name;
    uint maxPlayers;
    uint difficulty;
    std::vector<uint> players;
};

struct gameServer
{
    uint port;
    ENetPeer* peer;
};

room initRoom(std::string name, uint maxPlayers, uint difficulty)
{
    return 
    {
      .name = name,
      .maxPlayers = maxPlayers,
      .difficulty = difficulty
    };
}