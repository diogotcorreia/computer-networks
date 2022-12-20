#include "server_state.hpp"

#include <unistd.h>

#include <cstring>
#include <iostream>

#include "common/protocol.hpp"
#include "packet_handlers.hpp"

GameServerState::GameServerState(std::string &__word_file_path,
                                 std::string &port, bool __verbose)
    : word_file_path{__word_file_path}, cdebug{DebugStream(__verbose)} {
  this->setup_sockets();
  this->resolveServerAddress(port);
}

GameServerState::~GameServerState() {
  close(this->udp_socket_fd);
  close(this->tcp_socket_fd);
  freeaddrinfo(this->server_udp_addr);
  freeaddrinfo(this->server_tcp_addr);
}

void GameServerState::registerPacketHandlers() {
  // UDP
  udp_packet_handlers.insert({StartGameServerbound::ID, handle_start_game});
  udp_packet_handlers.insert({GuessLetterServerbound::ID, handle_guess_letter});
  udp_packet_handlers.insert({GuessWordServerbound::ID, handle_guess_word});
  udp_packet_handlers.insert({QuitGameServerbound::ID, handle_quit_game});
  udp_packet_handlers.insert({RevealWordServerbound::ID, handle_reveal_word});

  // TCP
  tcp_packet_handlers.insert({StateServerbound::ID, handle_state});
}

void GameServerState::setup_sockets() {
  // Create a UDP socket
  if ((this->udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a UDP socket");
    exit(EXIT_FAILURE);
  }

  // Create a TCP socket
  if ((this->tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a TCP socket");
    exit(EXIT_FAILURE);
  }
}

void GameServerState::resolveServerAddress(std::string &port) {
  struct addrinfo hints;
  const char *port_str = port.c_str();
  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if (getaddrinfo(NULL, port_str, &hints, &this->server_udp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for UDP connection");
    exit(EXIT_FAILURE);
  }
  // bind socket
  if (bind(this->udp_socket_fd, this->server_udp_addr->ai_addr,
           this->server_udp_addr->ai_addrlen)) {
    // TODO consider using exceptions (?)
    perror("Failed to bind UDP address");
    exit(EXIT_FAILURE);
  }

  // Get TCP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP socket
  if (getaddrinfo(NULL, port.c_str(), &hints, &this->server_tcp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for TCP connection");
    exit(1);
  }

  if (bind(this->tcp_socket_fd, this->server_tcp_addr->ai_addr,
           this->server_tcp_addr->ai_addrlen)) {
    // TODO consider using exceptions (?)
    perror("Failed to bind TCP address");
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening for connections on port " << port << std::endl;
}

void GameServerState::callUdpPacketHandler(std::string packet_id,
                                           std::stringstream &stream,
                                           Address &addr_from) {
  auto handler = this->udp_packet_handlers.find(packet_id);
  if (handler == this->udp_packet_handlers.end()) {
    // TODO add exception
    std::cout << "Unknown Packet ID" << std::endl;
    return;
  }

  handler->second(stream, addr_from, *this);
}

void GameServerState::callTcpPacketHandler(std::string packet_id,
                                           int connection_fd) {
  auto handler = this->tcp_packet_handlers.find(packet_id);
  if (handler == this->tcp_packet_handlers.end()) {
    // TODO add exception
    std::cout << "Unknown Packet ID" << std::endl;
    return;
  }

  handler->second(connection_fd, *this);
}

ServerGame &GameServerState::createGame(uint32_t player_id) {
  auto game = games.find(player_id);
  if (game != games.end()) {
    if (game->second.hasStarted()) {
      throw GameAlreadyStartedException();
    }
    if (game->second.isOnGoing()) {
      return game->second;
    }

    // Delete existing game, so we can create a new one below
    games.erase(game);
  }

  auto new_game = ServerGame(player_id);
  games.insert(std::pair(player_id, new_game));

  return games.at(player_id);
}

ServerGame &GameServerState::getGame(uint32_t player_id) {
  auto game = games.find(player_id);
  if (game == games.end()) {
    throw NoGameFoundException();
  }

  return game->second;
}
