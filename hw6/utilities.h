#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <termios.h>
#include <unistd.h>

const char* roomsCommand = "/rooms";
const char* selectCommand = "/select";
const char* leaveCommand = "/leave";
const char* matchmakerHost = "localhost";
const int matchmakerPort = 10887;

const char* startRoomCommand = "/start";
const char* createRoomCommand = "/create";
const char* gameServerHost = "localhost";
const uint matchmakerPort = 10887;

const char* matchmakerHost = "localhost";
const int matchmakerPort = 10887;


enum class MessageType 
{ 
    CLIENT_TO_MATCHMAKER_GET_ROOM_LIST,
    CLIENT_TO_MATCHMAKER_SELECT_ROOM,
    CLIENT_TO_MATCHMAKER_LEAVE_ROOM,

    MATCHMAKER_TO_CLIENT_ROOM_LIST,
    MATCHMAKER_TO_CLIENT_HOST_LINK,
    MATCHMAKER_TO_CLIENT_MESSAGE,

    SERVER_TO_MATCHMAKER_READY,
    MATCHMAKER_TO_SERVER_START,

    SERVER_TO_CLIENT_INIT,
    SERVER_TO_CLIENT_NEW_CONNECTED,

    CLIENT_TO_SERVER_UPDATE,

    SERVER_TO_CLIENT_UPDATE,
    SERVER_TO_CLIENT_PING_LIST
};

struct addrinfo;

struct message
{
  MessageType type;
  std::vector<std::string> data;

};

int khbit();
void nonblock(int state);
bool keyState(int key);
std::string getCommand(int pauseKey);
void send_message(message * msg, ENetPeer * peer,uint channel,bool isReliable);
void data_to_message(enet_uint8 * data, message &msg);
