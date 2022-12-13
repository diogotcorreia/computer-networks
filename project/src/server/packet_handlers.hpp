#ifndef PACKET_HANDLERS_H
#define PACKET_HANDLERS_H

#include <sstream>

#include "server_state.hpp"

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state);

#endif
