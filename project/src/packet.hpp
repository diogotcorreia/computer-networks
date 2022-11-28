#ifndef PACKET_H
#define PACKET_H

#include <sys/socket.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>

#define SOCKET_BUFFER_LEN (256)
#define PACKET_ID_LEN (3)

class Packet {
 private:
  void readChar(std::stringstream &buffer, char chr);

 protected:
  void readPacketId(std::stringstream &buffer, const char *id);
  void readSpace(std::stringstream &buffer);
  char readChar(std::stringstream &buffer);
  void readPacketDelimiter(std::stringstream &buffer);
  std::unique_ptr<char[]> readString(std::stringstream &buffer, size_t max_len);
  int32_t readInt(std::stringstream &buffer);

 public:
  virtual std::stringstream serialize() = 0;
  virtual void deserialize(std::stringstream &buffer) = 0;

  virtual ~Packet() = default;
};

// Thrown when the PacketID does not match what was expected
class UnexpectedPacketException : public std::runtime_error {
 public:
  UnexpectedPacketException()
      : std::runtime_error("Packet ID does not match what was expected") {}
};

// Thrown when the PacketID is correct, but the schema is wrong
class InvalidPacketException : public std::runtime_error {
 public:
  InvalidPacketException()
      : std::runtime_error("Schema of the received packet is incorrect") {}
};

// Start New Game Packet (SNG)
class StartGameServerbound : public Packet {
 public:
  static constexpr const char *ID = "SNG";
  int32_t player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

// Reply to Start Game Packet (RSG)
class ReplyStartGameClientbound : public Packet {
 public:
  static constexpr const char *ID = "RSG";
  bool success;
  int32_t n_letters;
  int32_t max_errors;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessLetterServerbound : public Packet {
 public:
  static constexpr const char *ID = "PLG";
  int player_id;
  char guess;
  int trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessLetterClientbound : public Packet {
  enum status { OK, WIN, DUP, NOK, OVR, INV, ERR };

 public:
  static constexpr const char *ID = "RLG";
  status status;
  int trial;
  int n;
  int *pos;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessWordServerbound : public Packet {
 public:
  static constexpr const char *ID = "PWG";
  int player_id;
  char *guess;
  int trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer, int wordLen);
};

class GuessWordClientbound : public Packet {
  enum status { WIN, NOK, OVR, INV, ERR };

 public:
  static constexpr const char *ID = "RWG";
  status status;
  int trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class QuitGameServerbound : public Packet {
 public:
  static constexpr const char *ID = "QUT";
  int player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class QuitGameClientbound : public Packet {
 public:
  static constexpr const char *ID = "RQT";
  bool success;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class RevealWordServerbound : public Packet {
 public:
  static constexpr const char *ID = "REV";
  int player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class RevealWordClientbound : public Packet {
 public:
  static constexpr const char *ID = "RRV";
  char *word;
  bool success;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer, int wordLen);
};

Packet *deserialize(char *buffer);

void send_packet(Packet &packet, int socket, struct sockaddr *address,
                 socklen_t addrlen);

void wait_for_packet(Packet &packet, int socket);

#endif
