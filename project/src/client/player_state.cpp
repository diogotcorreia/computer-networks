#include "player_state.hpp"

#include <unistd.h>

#include <cstring>
#include <iostream>

#include "common/common.hpp"

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
    throw UnrecoverableError("Failed to create a UDP socket", errno);
  }
}

void PlayerState::resolveServerAddress(std::string &hostname,
                                       std::string &port) {
  struct addrinfo hints;
  int addr_res;
  const char *host = hostname.c_str();
  const char *port_str = port.c_str();

  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if ((addr_res =
           getaddrinfo(host, port_str, &hints, &this->server_udp_addr)) != 0) {
    throw UnrecoverableError(
        std::string("Failed to get address for UDP connection: ") +
        gai_strerror(addr_res));
  }

  // Get TCP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP socket
  if ((addr_res =
           getaddrinfo(host, port_str, &hints, &this->server_tcp_addr)) != 0) {
    throw UnrecoverableError(
        std::string("Failed to get address for TCP connection: ") +
        gai_strerror(addr_res));
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
  try {
    openTcpSocket();
    sendTcpPacket(out_packet);
    waitForTcpPacket(in_packet);
  } catch (...) {
    closeTcpSocket();
    throw;
  }
  closeTcpSocket();
};

void PlayerState::sendTcpPacket(TcpPacket &packet) {
  if (connect(tcp_socket_fd, server_tcp_addr->ai_addr,
              server_tcp_addr->ai_addrlen) != 0) {
    throw ConnectionTimeoutException();
  }
  packet.send(tcp_socket_fd);
}

void PlayerState::waitForTcpPacket(TcpPacket &packet) {
  packet.receive(tcp_socket_fd);
}

void PlayerState::openTcpSocket() {
  if ((this->tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    throw UnrecoverableError("Failed to create a TCP socket", errno);
  }
  struct timeval read_timeout;
  read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
  read_timeout.tv_usec = 0;
  if (setsockopt(this->tcp_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
                 sizeof(read_timeout)) < 0) {
    throw UnrecoverableError("Failed to set TCP read timeout socket option",
                             errno);
  }
  struct timeval write_timeout;
  write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
  write_timeout.tv_usec = 0;
  if (setsockopt(this->tcp_socket_fd, SOL_SOCKET, SO_SNDTIMEO, &write_timeout,
                 sizeof(write_timeout)) < 0) {
    throw UnrecoverableError("Failed to set TCP send timeout socket option",
                             errno);
  }
}

void PlayerState::closeTcpSocket() {
  if (close(this->tcp_socket_fd) != 0) {
    if (errno == EBADF) {
      // was already closed
      return;
    }
    throw UnrecoverableError("Failed to close TCP socket", errno);
  }
}
