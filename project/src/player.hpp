#ifndef PLAYER_H
#define PLAYER_H

#include <netdb.h>

#include "commands.hpp"
#include "constants.hpp"
#include "packet.hpp"

class ClientConfig {
 public:
  char* program_path;
  std::string host = DEFAULT_HOSTNAME;
  std::string port = DEFAULT_PORT;  // TODO consider parsing port as int
  bool help = false;

  ClientConfig(int argc, char* argv[]);
  void printHelp(std::ostream& stream);
};

class PlayerState {
  void setup_sockets();

 public:
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  PlayerState();
  ~PlayerState();
  void resolveServerAddress(std::string hostname, std::string port);
};

void registerCommands(CommandManager& manager);

void start_game(int player_id, int socket, addrinfo* res);

#endif
