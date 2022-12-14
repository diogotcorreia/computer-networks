#ifndef SERVER_H
#define SERVER_H

#include "common/constants.hpp"
#include "server_game.hpp"
#include "server_state.hpp"

class ServerConfig {
 public:
  char* program_path;
  std::string word_file_path;
  std::string port = DEFAULT_PORT;  // TODO consider parsing port as int
  bool help = false;
  bool verbose = false;

  ServerConfig(int argc, char* argv[]);
  void printHelp(std::ostream& stream);
};

void wait_for_udp_packet(GameServerState& server_state);

void handle_packet(std::stringstream& buffer, Address& addr_from,
                   GameServerState& server_state);

#endif
