#ifndef SERVER_H
#define SERVER_H

#include "server_game.hpp"
#include "server_state.hpp"

void wait_for_udp_packet(GameServerState& server_state);

void handle_packet(std::stringstream& buffer, Address& addr_from,
                   GameServerState& server_state);

#endif
