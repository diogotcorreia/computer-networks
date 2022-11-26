#include "GS.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define PORT "8080"

int main() {
  GameServerState state;
  state.registerPacketHandlers();
  state.resolveServerAddress(PORT);

  // listen for connections

  std::cout << "Listening for connections" << std::endl;

  // TODO put inside loop
  // TESTING: receiving and sending a packet
  wait_for_udp_packet(state);
}

void handleStartGame(std::stringstream &buffer, Address &addr_from) {
  StartGameServerbound packet;
  packet.deserialize(buffer);
  // TODO
  printf("Received SNG packet with player_id: %d\n", packet.player_id);

  ReplyStartGameClientbound response;
  response.success = true;
  response.n_letters = 5;
  response.max_errors = 5;
  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void GameServerState::registerPacketHandlers() {
  packet_handlers.insert({StartGameServerbound::ID, handleStartGame});
}

void wait_for_udp_packet(GameServerState &server_state) {
  Address addr_from;
  std::stringstream stream;
  // TODO: change this to a dynamic buffer
  char buffer[SOCKET_BUFFER_LEN];

  // TODO: change hardcoded len to dynamic buffer size
  int n = recvfrom(server_state.udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
                   (struct sockaddr *)&addr_from.addr, &addr_from.size);
  addr_from.socket = server_state.udp_socket_fd;
  if (n == -1) {
    perror("recvfrom");
    exit(1);
  }

  stream << buffer;

  return handle_packet(stream, addr_from, server_state);
}

void handle_packet(std::stringstream &buffer, Address &addr_from,
                   GameServerState &server_state) {
  char packet_id[PACKET_ID_LEN + 1];
  buffer >> packet_id;

  if (!buffer.good()) {
    // TODO consider using exceptions (?)
    std::cerr << "Received malformated packet ID" << std::endl;
    return;
  }

  std::string packet_id_str = std::string(packet_id);

  server_state.callPacketHandler(packet_id_str, buffer, addr_from);
}

GameServerState::GameServerState() {
  this->setup_sockets();
}

GameServerState::~GameServerState() {
  close(this->udp_socket_fd);
  close(this->tcp_socket_fd);
  freeaddrinfo(this->server_udp_addr);
  freeaddrinfo(this->server_tcp_addr);
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

void GameServerState::resolveServerAddress(std::string port) {
  struct addrinfo hints;

  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if (getaddrinfo(NULL, port.c_str(), &hints, &this->server_udp_addr) != 0) {
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
}

void GameServerState::callPacketHandler(std::string packet_id,
                                        std::stringstream &stream,
                                        Address &addr_from) {
  auto handler = this->packet_handlers.find(packet_id);
  if (handler == this->packet_handlers.end()) {
    std::cout << "Unknown Packet ID" << std::endl;
    return;
  }

  handler->second(stream, addr_from);
}
