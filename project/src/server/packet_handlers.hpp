#ifndef PACKET_HANDLERS_H
#define PACKET_HANDLERS_H

#include <sstream>

#include "server_state.hpp"

// UDP

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state);

void handle_guess_letter(std::stringstream &buffer, Address &addr_from,
                         GameServerState &state);

void handle_guess_word(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state);

void handle_quit_game(std::stringstream &buffer, Address &addr_from,
                      GameServerState &state);

void handle_reveal_word(std::stringstream &buffer, Address &addr_from,
                        GameServerState &state);

// TCP

void handle_scoreboard(int connection_fd, GameServerState &state);

void handle_state(int connection_fd, GameServerState &state);

#endif
