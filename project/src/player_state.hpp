#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

#include <netdb.h>

class PlayerState {
  void setupSockets();

 public:
  ClientGame* game;
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  bool hasActiveGame();
  PlayerState();
  ~PlayerState();
  void startGame(ClientGame* game);
  void resolveServerAddress(std::string hostname, std::string port);
  void sendPacket(Packet& packet);
  void waitForPacket(Packet& packet);
};

#endif