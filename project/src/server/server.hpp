#ifndef SERVER_H
#define SERVER_H

#include "common/constants.hpp"
#include "server_game.hpp"
#include "server_state.hpp"
#include "worker_pool.hpp"

class ServerConfig {
 public:
  char* programPath;
  std::string wordFilePath;
  std::string port = DEFAULT_PORT;  // TODO consider parsing port as int
  bool help = false;
  bool verbose = false;
  bool random = false;

  ServerConfig(int argc, char* argv[]);
  void printHelp(std::ostream& stream);
};

void main_tcp(GameServerState& state);

void wait_for_udp_packet(GameServerState& server_state);

void handle_packet(std::stringstream& buffer, Address& addr_from,
                   GameServerState& server_state);

void wait_for_tcp_packet(GameServerState& server_state, WorkerPool& pool);

#endif
