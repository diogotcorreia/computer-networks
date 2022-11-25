#ifndef PACKET_H
#define PACKET_H

#include <sstream>

class Packet {
 public:
  virtual std::stringstream serialize() = 0;
  virtual void deserialize(std::stringstream &buffer) = 0;
};

// Start New Game Packet
class SNG : public Packet {
 public:
  int player_id;
  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

// Reply to Start Game Packet
class RSG : public Packet {
 public:
  bool success;
  int n_letters;
  int max_errors;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

Packet *deserialize(char *buffer);

void send_packet(Packet *packet, int socket, struct sockaddr_in *address);

Packet *receive_packet(int socket, struct sockaddr_in *address, size_t len);

#endif
