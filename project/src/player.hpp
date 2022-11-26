#ifndef PLAYER_H
#define PLAYER_H

#include <netdb.h>

#include "commands.hpp"
#include "packet.hpp"

class PlayerState {
  void setup_sockets();

 public:
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  PlayerState();
  ~PlayerState();
  void resolve_server_address(std::string hostname, std::string port);
};

void registerCommands(CommandManager& manager);

void start_game(int player_id, int socket, addrinfo* res);

#endif
