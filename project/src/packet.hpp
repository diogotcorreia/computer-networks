#ifndef PACKET_H
#define PACKET_H

#include <sstream>

class Packet {
 public:
  virtual std::stringstream serialize() = 0;
  virtual void deserialize(std::stringstream &buffer) = 0;

  virtual ~Packet() = default;
};

// Start New Game Packet (SNG)
class StartGameServerbound : public Packet {
 public:
  int player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

// Reply to Start Game Packet (RSG)
class ReplyStartGameClientbound : public Packet {
 public:
  bool success;
  int n_letters;
  int max_errors;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

Packet *deserialize(char *buffer);

void send_packet(Packet &packet, int socket, struct sockaddr *address,
                 size_t addrlen);

Packet *receive_packet(int socket, struct sockaddr *address);

#endif
