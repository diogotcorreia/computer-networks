#include "packet.hpp"

#include <sys/types.h>

#include <cstring>

// Packet type seriliazation and deserialization methods
std::stringstream StartGameServerbound::serialize() {
  std::stringstream buffer;
  buffer << "SNG " << player_id << std::endl;
  return buffer;
};

void StartGameServerbound::deserialize(std::stringstream &buffer) {
  buffer >> player_id;
};

std::stringstream ReplyStartGameClientbound::serialize() {
  std::stringstream buffer;
  buffer << "RSG " << success << " " << n_letters << " " << max_errors
         << std::endl;
  return buffer;
};

void ReplyStartGameClientbound::deserialize(std::stringstream &buffer) {
  buffer >> success >> n_letters >> max_errors;
};

// Packet deserilization and creation
Packet *deserialize(char *buffer) {
  char opcode[4];
  std::stringstream ss;
  ss << buffer;
  if (ss >> opcode) {
    if (strcmp(opcode, "SNG") == 0) {
      StartGameServerbound *packet = new StartGameServerbound();
      packet->deserialize(ss);
      return packet;
    } else if (strcmp(opcode, "RSG") == 0) {
      ReplyStartGameClientbound *packet = new ReplyStartGameClientbound();
      packet->deserialize(ss);
      return packet;
    } else {
      throw std::runtime_error("Invalid opcode");
    }
  } else {
    throw std::runtime_error("Invalid opcode");
  }
};

// Packet sending and receiving
// TODO: probably can reduce number of arguments
void send_packet(Packet &packet, int socket, struct sockaddr *address,
                 socklen_t addrlen) {
  const std::stringstream buffer = packet.serialize();
  // ERROR HERE: address type changes in client and server
  int n = sendto(socket, buffer.str().c_str(), buffer.str().length(), 0,
                 address, addrlen);
  if (n == -1) {
    perror("sendto");
    exit(1);
  }
}

Packet *receive_packet(int socket, struct sockaddr *address,
                       socklen_t *addrlen) {
  // TODO: change this to a dynamic buffer
  char buffer[128];

  // TODO: change hardcoded 128 to dynamic buffer size
  int n = recvfrom(socket, buffer, 128, 0, address, addrlen);
  if (n == -1) {
    perror("recvfrom");
    exit(1);
  }
  return deserialize(buffer);
}
