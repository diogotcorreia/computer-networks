#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

#include <netdb.h>

#include "client_game.hpp"
#include "common/protocol.hpp"

class PlayerState {
  void setupSockets();
  void sendUdpPacket(Packet& packet);
  void waitForUdpPacket(Packet& packet);

 public:
  ClientGame* game = NULL;
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  PlayerState();
  ~PlayerState();
  bool hasActiveGame();
  bool hasGame();
  void startGame(ClientGame* game);
  void resolveServerAddress(std::string hostname, std::string port);
  void sendUdpPacketAndWaitForReply(Packet& out_packet, Packet& in_packet);
  void sendPacket(TcpPacket& packet);
  void waitForPacket(TcpPacket& packet);
  void openTCPSocket();
  void closeTCPSocket();
};

#endif