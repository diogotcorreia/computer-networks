#ifndef GS_H
#define GS_H

#include <netdb.h>

#include <sstream>
#include <unordered_map>

#include "packet.hpp"

class Address;

typedef void (*PacketHandler)(std::stringstream&, Address&);

class GameServerState {
  std::unordered_map<std::string, PacketHandler> packet_handlers;

  void setup_sockets();

 public:
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;

  GameServerState();
  ~GameServerState();
  void resolveServerAddress(std::string port);
  void registerPacketHandlers();
  void callPacketHandler(std::string packet_id, std::stringstream& stream,
                         Address& addr_from);
};

class Address {
 public:
  int socket;
  struct sockaddr_in addr;
  socklen_t size;
};

void handleStartGame(Packet& packet);

void wait_for_udp_packet(GameServerState& server_state);

void handle_packet(std::stringstream& buffer, Address& addr_from,
                   GameServerState& server_state);

#endif
