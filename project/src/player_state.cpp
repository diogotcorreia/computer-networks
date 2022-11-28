#include "player_state.hpp"

#include <unistd.h>

#include <cstring>

PlayerState::PlayerState() {
  this->setupSockets();
}

PlayerState::~PlayerState() {
  // TODO check return of close (?)
  close(this->udp_socket_fd);
  close(this->tcp_socket_fd);
  freeaddrinfo(this->server_udp_addr);
  freeaddrinfo(this->server_tcp_addr);
}

void PlayerState::startGame(ClientGame *game) {
  if (this->game != game) {
    delete this->game;
  }
  this->game = game;
}

void PlayerState::setupSockets() {
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

void PlayerState::resolveServerAddress(std::string hostname, std::string port) {
  struct addrinfo hints;
  const char *host = hostname.c_str();
  const char *port_str = port.c_str();

  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if (getaddrinfo(host, port_str, &hints, &this->server_udp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for UDP connection");
    exit(EXIT_FAILURE);
  }

  // Get TCP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP socket
  if (getaddrinfo(host, port_str, &hints, &this->server_tcp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for TCP connection");
    exit(1);
  }
}

void PlayerState::sendPacket(Packet &packet) {
  send_packet(packet, udp_socket_fd, server_udp_addr->ai_addr,
              server_udp_addr->ai_addrlen);
}

void PlayerState::sendPacket(TcpPacket &packet) {
  if (connect(tcp_socket_fd, server_tcp_addr->ai_addr,
              server_tcp_addr->ai_addrlen) != 0) {
    throw new ConnectionTimeoutException();
  }
  packet.send(tcp_socket_fd);
  // TODO does this need closing?
}

void PlayerState::waitForPacket(Packet &packet) {
  wait_for_packet(packet, udp_socket_fd);
}

void PlayerState::waitForPacket(TcpPacket &packet) {
  packet.receive(tcp_socket_fd);
  // TODO does this need closing?
}
