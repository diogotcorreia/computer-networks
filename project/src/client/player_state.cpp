#include "player_state.hpp"

#include <unistd.h>

#include <cstring>

PlayerState::PlayerState(std::string &hostname, std::string &port) {
  this->setupSockets();
  this->resolveServerAddress(hostname, port);
}

PlayerState::~PlayerState() {
  if (this->udp_socket_fd != -1) {
    close(this->udp_socket_fd);
  }
  if (this->tcp_socket_fd != -1) {
    close(this->tcp_socket_fd);
  }
  if (this->server_udp_addr != NULL) {
    freeaddrinfo(this->server_udp_addr);
  }
  if (this->server_tcp_addr != NULL) {
    freeaddrinfo(this->server_tcp_addr);
  }
  delete this->game;
}

bool PlayerState::hasActiveGame() {
  return hasGame() && this->game->isOnGoing();
}

bool PlayerState::hasGame() {
  return this->game != NULL;
}

void PlayerState::startGame(ClientGame *__game) {
  if (this->game != __game) {
    delete this->game;
  }
  this->game = __game;
}

void PlayerState::setupSockets() {
  // Create a UDP socket
  if ((this->udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a UDP socket");
    exit(EXIT_FAILURE);
  }
}

void PlayerState::resolveServerAddress(std::string &hostname,
                                       std::string &port) {
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
    exit(EXIT_FAILURE);
  }
}

void PlayerState::sendUdpPacketAndWaitForReply(UdpPacket &out_packet,
                                               UdpPacket &in_packet) {
  int triesLeft = UDP_RESEND_TRIES;
  while (triesLeft > 0) {
    --triesLeft;
    try {
      this->sendUdpPacket(out_packet);
      this->waitForUdpPacket(in_packet);
      return;
    } catch (ConnectionTimeoutException &e) {
      if (triesLeft == 0) {
        throw;
      }
    }
  }
}

void PlayerState::sendUdpPacket(UdpPacket &packet) {
  send_packet(packet, udp_socket_fd, server_udp_addr->ai_addr,
              server_udp_addr->ai_addrlen);
}

void PlayerState::waitForUdpPacket(UdpPacket &packet) {
  wait_for_packet(packet, udp_socket_fd);
}

void PlayerState::sendTcpPacketAndWaitForReply(TcpPacket &out_packet,
                                               TcpPacket &in_packet) {
  openTcpSocket();
  sendTcpPacket(out_packet);
  waitForTcpPacket(in_packet);
  // TODO ensure TCP socket is always closed
  closeTcpSocket();
};

void PlayerState::sendTcpPacket(TcpPacket &packet) {
  if (connect(tcp_socket_fd, server_tcp_addr->ai_addr,
              server_tcp_addr->ai_addrlen) != 0) {
    throw ConnectionTimeoutException();
  }
  packet.send(tcp_socket_fd);
  // TODO does this need closing?
}

void PlayerState::waitForTcpPacket(TcpPacket &packet) {
  packet.receive(tcp_socket_fd);
  // TODO does this need closing?
}

void PlayerState::openTcpSocket() {
  if ((this->tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a TCP socket");
    exit(EXIT_FAILURE);
  }
  struct timeval read_timeout;
  read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
  read_timeout.tv_usec = 0;
  if (setsockopt(this->tcp_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
                 sizeof(read_timeout)) < 0) {
    // TODO consider using exceptions (?)
    perror("Failed to set socket options");
    exit(EXIT_FAILURE);
  }
  struct timeval write_timeout;
  write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
  write_timeout.tv_usec = 0;
  if (setsockopt(this->tcp_socket_fd, SOL_SOCKET, SO_SNDTIMEO, &write_timeout,
                 sizeof(write_timeout)) < 0) {
    // TODO consider using exceptions (?)
    perror("Failed to set socket options");
    exit(EXIT_FAILURE);
  }
}

void PlayerState::closeTcpSocket() {
  if (close(this->tcp_socket_fd) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to close TCP socket");
    exit(EXIT_FAILURE);
  }
}
