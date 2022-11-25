#include "packet.hpp"

#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

std::stringstream SNG::serialize() {
  std::stringstream buffer;
  buffer << "SNG " << player_id << std::endl;
  return buffer;
};

void SNG::deserialize(std::stringstream &buffer) {
  char opcode[4];
  buffer >> opcode >> player_id;
  if (strcmp(opcode, "SNG") != 0) {
    throw std::runtime_error("Invalid opcode");
  }
};

std::stringstream RSG::serialize() {
  std::stringstream buffer;
  buffer << "RSG " << success << " " << n_letters << " " << max_errors
         << std::endl;
  return buffer;
};

void RSG::deserialize(std::stringstream &buffer) {
  char opcode[4];
  buffer >> opcode >> success >> n_letters >> max_errors;
  if (strcmp(opcode, "RSG") != 0) {
    throw std::runtime_error("Invalid opcode");
  }
};

Packet *deserialize(char *buffer) {
  char opcode[4];
  std::stringstream ss;
  ss << buffer;
  ss >> opcode;
  if (strcmp(opcode, "SNG") == 0) {
    SNG *sng = new SNG();
    sng->deserialize(ss);
    return sng;
  } else if (strcmp(opcode, "RSG") == 0) {
    RSG *rsg = new RSG();
    rsg->deserialize(ss);
    return rsg;
  } else {
    throw std::runtime_error("Invalid opcode");
  }
}

void send_packet(Packet *packet, int socket, struct sockaddr_in *address) {
  const std::stringstream buffer = packet->serialize();
  int n = sendto(socket, buffer.str().c_str(), buffer.str().length(), 0,
                 (const struct sockaddr *)address, sizeof(address));
  if (n == -1) {
    perror("sendto");
    exit(1);
  }
}

Packet *receive_packet(int socket, struct sockaddr_in *address, size_t len) {
  char *buffer = new char[len];
  socklen_t addrlen = sizeof(address);
  int n =
      recvfrom(socket, &buffer, len, 0, (struct sockaddr *)&address, &addrlen);
  if (n == -1) {
    perror("recvfrom");
    exit(1);
  }
  return deserialize(buffer);
}
