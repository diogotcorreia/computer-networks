#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cmath>

#define DEFAULT_HOSTNAME "localhost"
#define DEFAULT_PORT "58044"

#define SOCKET_BUFFER_LEN (256)
#define PACKET_ID_LEN (3)

#define PLAYER_ID_MAX_LEN (6)
#define PLAYER_ID_MAX (pow(10, PLAYER_ID_MAX_LEN) - 1)

#endif
