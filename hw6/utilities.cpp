#include <enet/enet.h>

#include "utilities.h"


int khbit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);

    if ( state == 1)
    {
        ttystate.c_lflag &= (~ICANON & ~ECHO); 
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state == 0)
    {
        ttystate.c_lflag |= ICANON | ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

bool keyState(int key)
{
    bool pressed = false;
    int i = khbit();
    if (i != 0)
    {
        char c = fgetc(stdin);
        if (c == (char) key)
        {
            pressed = true;
        }
        else
        {
            pressed = false;
        }
    }
    return pressed;
}

std::string getCommand(int key)
{

    std::string command;
    bool isInput = false;
    nonblock(1);
    if (keyState(key))
        isInput = true;
    nonblock(0);
    if (isInput)
    {
        std::string input;
        std::cout << "Type command" << std::endl;
        std::cin >> input;
        return input;
    }
}

void send_message(message * msg, ENetPeer * peer, uint channel, bool isReliable)
{   
    size_t size = 2;

    for (int i = 0; i < msg->data.size(); ++i)
        size += msg->data[i].length() + 1;

    enet_uint8 *arr = new enet_uint8[size];
    arr[0] = static_cast<enet_uint8>(msg->type);
    arr[1] = msg->data.size();
    uint k = 2;

    for (int i = 0; i < msg->data.size(); ++i)
    {
        arr[k] = msg->data[i].length();
        k++;

        for (int j = 0; j < msg->data[i].length(); ++j)
        {
            arr[k] = msg->data[i][j];
            k++;
        }
    }

    ENetPacket *packet = enet_packet_create(arr, size, isReliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    enet_peer_send(peer, channel, packet);
}

void data_to_message(enet_uint8 * data, message &msg)
{
    msg.type = (MessageType)data[0];
    msg.data = {};

    uint vectorSize = data[1];
    uint k = 2;
    while (vectorSize > 0)
    {
        uint strLen = data[k];
        msg.data.push_back(std::string(&data[k+1], &data[k+strLen+1]));
        k += strLen++;
        vectorSize--;
    }
} 