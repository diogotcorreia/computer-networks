#ifndef PACKET_H
#define PACKET_H

#include <sys/socket.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "constants.hpp"

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

// Thrown when serialization error occurs
class PacketSerializationException : public std::runtime_error {
 public:
  PacketSerializationException()
      : std::runtime_error("Error while serializing packet") {}
};

// Thrown when timeout for reading/writing packet occurs
class ConnectionTimeoutException : public std::runtime_error {
 public:
  ConnectionTimeoutException()
      : std::runtime_error("Timed out while waiting for or sending packet") {}
};

// Thrown when an error related to I/O occurs
class IOException : public std::runtime_error {
 public:
  IOException()
      : std::runtime_error(
            "IO error while reading/writting from/to filesystem") {}
};

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
 public:
  enum status { OK, WIN, DUP, NOK, OVR, INV, ERR };
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
  int wordLen;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
  ~GuessWordServerbound();
};

class GuessWordClientbound : public Packet {
 public:
  enum status { WIN, NOK, OVR, INV, ERR };
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
  size_t wordLen;
  std::unique_ptr<char[]> word;
  bool success;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class TcpPacket {
 private:
  void readChar(int fd, char chr);

 protected:
  void writeString(int fd, const std::string &str);
  void writePlayerId(int fd, const int player_id);
  void readPacketId(int fd, const char *id);
  void readSpace(int fd);
  char readChar(int fd);
  void readPacketDelimiter(int fd);
  std::string readString(const int fd);
  int32_t readInt(const int fd);
  void readAndSaveToFile(const int fd, const std::string &file_name,
                         const size_t file_size);

 public:
  virtual void send(int fd) = 0;
  virtual void receive(int fd) = 0;

  virtual ~TcpPacket() = default;
};

class ScoreboardServerbound : public TcpPacket {
 public:
  static constexpr const char *ID = "GSB";

  void send(int fd);
  void receive(int fd);
};

class ScoreboardClientbound : public TcpPacket {
  enum status { OK, EMPTY };

 public:
  static constexpr const char *ID = "RSB";
  status status;
  std::string file_name;

  void send(int fd);
  void receive(int fd);
};

class HintServerbound : public TcpPacket {
 public:
  static constexpr const char *ID = "GHL";
  int player_id;

  void send(int fd);
  void receive(int fd);
};

class HintClientbound : public TcpPacket {
  enum status { OK, NOK };

 public:
  static constexpr const char *ID = "RHL";
  status status;
  std::string file_name;

  void send(int fd);
  void receive(int fd);
};

void send_packet(Packet &packet, int socket, struct sockaddr *address,
                 socklen_t addrlen);

void wait_for_packet(Packet &packet, int socket);

void write_player_id(std::stringstream &buffer, const int player_id);

#endif
