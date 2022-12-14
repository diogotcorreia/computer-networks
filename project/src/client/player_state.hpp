#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

#include <netdb.h>

#include "client_game.hpp"
#include "common/protocol.hpp"

class PlayerState {
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  void setupSockets();
  void resolveServerAddress(std::string hostname, std::string port);
  void sendUdpPacket(UdpPacket& packet);
  void waitForUdpPacket(UdpPacket& packet);
  void openTcpSocket();
  void sendTcpPacket(TcpPacket& packet);
  void waitForTcpPacket(TcpPacket& packet);
  void closeTcpSocket();

 public:
  ClientGame* game = NULL;

  PlayerState(std::string hostname, std::string port);
  ~PlayerState();
  bool hasActiveGame();
  bool hasGame();
  void startGame(ClientGame* game);
  void sendUdpPacketAndWaitForReply(UdpPacket& out_packet,
                                    UdpPacket& in_packet);
  void sendTcpPacketAndWaitForReply(TcpPacket& out_packet,
                                    TcpPacket& in_packet);
};

#endif