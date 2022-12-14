#include "server.hpp"

#include <iostream>

#include "common/constants.hpp"

int main() {
  GameServerState state;
  state.registerPacketHandlers();
  state.resolveServerAddress(DEFAULT_PORT);

  // TODO handle TCP
  while (true) {
    // TESTING: receiving and sending a packet
    wait_for_udp_packet(state);
  }
}

void wait_for_udp_packet(GameServerState &server_state) {
  Address addr_from;
  std::stringstream stream;
  char buffer[SOCKET_BUFFER_LEN];

  addr_from.size = sizeof(addr_from.addr);
  ssize_t n = recvfrom(server_state.udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
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
