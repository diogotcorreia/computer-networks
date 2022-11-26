#ifndef PLAYER_H
#define PLAYER_H

#include <netdb.h>

#include "commands.hpp"
#include "packet.hpp"

void registerCommands(CommandManager& manager);

void start_game(int player_id, int socket, addrinfo* res);

#endif
