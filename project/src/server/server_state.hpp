#ifndef SERVER_STATE_H
#define SERVER_STATE_H

#include <netdb.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "server_game.hpp"

class Address {
 public:
  int socket;
  struct sockaddr_in addr;
  socklen_t size;
};

class DebugStream {
  bool active;

 public:
  DebugStream(bool __active) : active{__active} {};

  template <class T>
  DebugStream& operator<<(T val) {
    if (active) {
      std::cout << val;
    }
    return *this;
  }

  DebugStream& operator<<(std::ostream& (*f)(std::ostream&)) {
    if (active) {
      f(std::cout);
    }
    return *this;
  }

  DebugStream& operator<<(std::ostream& (*f)(std::ios&)) {
    if (active) {
      f(std::cout);
    }
    return *this;
  }

  DebugStream& operator<<(std::ostream& (*f)(std::ios_base&)) {
    if (active) {
      f(std::cout);
    }
    return *this;
  }
};

class GameServerState;

typedef void (*PacketHandler)(std::stringstream&, Address&, GameServerState&);

class GameServerState {
  std::unordered_map<std::string, PacketHandler> packet_handlers;
  std::unordered_map<uint32_t, ServerGame> games;
  std::vector<ServerGame> scoreboard{};
  std::string word_file_path;
  void setup_sockets();

 public:
  int udp_socket_fd;
  int tcp_socket_fd;
  struct addrinfo* server_udp_addr;
  struct addrinfo* server_tcp_addr;
  DebugStream cdebug;

  GameServerState(std::string& __word_file_path, std::string& port,
                  bool __verbose);
  ~GameServerState();
  void resolveServerAddress(std::string& port);
  void registerPacketHandlers();
  void callPacketHandler(std::string packet_id, std::stringstream& stream,
                         Address& addr_from);
  ServerGame& getGame(uint32_t player_id);
  ServerGame& createGame(uint32_t player_id);
  void addtoScoreboard(ServerGame* game);
  std::stringstream getScoreboard();
  static bool cmpGames(ServerGame& a, ServerGame& b);
};

/** Exceptions **/

// There is an on-going game with a player ID
class GameAlreadyStartedException : public std::runtime_error {
 public:
  GameAlreadyStartedException()
      : std::runtime_error(
            "There is already an on-going game with this player ID.") {}
};

// There is no on-going game with a player ID
class NoGameFoundException : public std::runtime_error {
 public:
  NoGameFoundException()
      : std::runtime_error("There is no on-going game with this player ID.") {}
};

#endif
