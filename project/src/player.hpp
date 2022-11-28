#ifndef PLAYER_H
#define PLAYER_H

#include "commands.hpp"
#include "constants.hpp"
#include "game.hpp"
#include "packet.hpp"
#include "player_state.hpp"

class ClientConfig {
 public:
  char* program_path;
  std::string host = DEFAULT_HOSTNAME;
  std::string port = DEFAULT_PORT;  // TODO consider parsing port as int
  bool help = false;

  ClientConfig(int argc, char* argv[]);
  void printHelp(std::ostream& stream);
};

void registerCommands(CommandManager& manager);

#endif
