#ifndef SERVER_STATE_H
#define SERVER_STATE_H

#include <netdb.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "scoreboard.hpp"
#include "server_game.hpp"

class Address {
 public:
  int socket;
  struct sockaddr_in addr;
  socklen_t size;
};

struct Word {
  std::string word;
  std::optional<std::filesystem::path> hint_path;
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

typedef void (*UdpPacketHandler)(std::stringstream&, Address&,
                                 GameServerState&);
typedef void (*TcpPacketHandler)(int connection_fd, GameServerState&);

class GameServerState {
  std::unordered_map<std::string, UdpPacketHandler> udp_packet_handlers;
  std::unordered_map<std::string, TcpPacketHandler> tcp_packet_handlers;
  std::unordered_map<uint32_t, ServerGame> games;
  std::vector<Word> words;
  std::mutex gamesLock;
  std::string word_file_dir;
  uint32_t current_word_index = 0;
  bool select_randomly;
  void setup_sockets();

 public:
  int udp_socket_fd = -1;
  int tcp_socket_fd = -1;
  struct addrinfo* server_udp_addr = NULL;
  struct addrinfo* server_tcp_addr = NULL;
  Scoreboard scoreboard;
  DebugStream cdebug;

  GameServerState(std::string& __word_file_path, std::string& port,
                  bool __verbose, bool __select_randomly);
  ~GameServerState();
  void resolveServerAddress(std::string& port);
  void registerPacketHandlers();
  void registerWords(std::string& __word_file_path);
  Word& selectRandomWord();
  void callUdpPacketHandler(std::string packet_id, std::stringstream& stream,
                            Address& addr_from);
  void callTcpPacketHandler(std::string packet_id, int connection_fd);
  ServerGameSync getGame(uint32_t player_id);
  ServerGameSync createGame(uint32_t player_id);
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
