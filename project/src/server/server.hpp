#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>

#include <sstream>
#include <unordered_map>

#include "common/protocol.hpp"
#include "server_game.hpp"

class Address;
class GameServerState;

typedef void (*PacketHandler)(std::stringstream&, Address&, GameServerState&);

class GameServerState {
  std::unordered_map<std::string, PacketHandler> packet_handlers;
  std::unordered_map<uint32_t, ServerGame> games;

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
  ServerGame& getGame(uint32_t player_id);
  ServerGame& createGame(uint32_t player_id);
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

/** Exceptions **/

// There is an on-going game with a player ID
class GameAlreadyStartedException : public std::runtime_error {
 public:
  GameAlreadyStartedException()
      : std::runtime_error(
            "There is already an on-going game with this player ID.") {}
};

#endif
