#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sys/socket.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "constants.hpp"

// Thrown when the PacketID does not match what was expected
class UnexpectedPacketException : public std::runtime_error {
 public:
  UnexpectedPacketException()
      : std::runtime_error(
            "The server did not reply with the expected response, so it was "
            "ignored. Please try again or connect to a different game "
            "server.") {}
};

// Thrown when the PacketID is correct, but the schema is wrong
class InvalidPacketException : public std::runtime_error {
 public:
  InvalidPacketException()
      : std::runtime_error(
            "The response given by the server is not correctly structured, so "
            "it was ignored. Please try again or connect to a different game "
            "server.") {}
};

// Thrown when serialization error occurs
class PacketSerializationException : public std::runtime_error {
 public:
  PacketSerializationException()
      : std::runtime_error(
            "There was an error while preparing a request to the game server, "
            "please try again and restart the client if the problem "
            "persists.") {}
};

// Thrown when timeout for reading/writing packet occurs
class ConnectionTimeoutException : public std::runtime_error {
 public:
  ConnectionTimeoutException()
      : std::runtime_error(
            "Could not connect to the game server, please check your internet "
            "connection and try again.") {}
};

// Thrown when an error related to I/O occurs
class IOException : public std::runtime_error {
 public:
  IOException()
      : std::runtime_error(
            "IO error while reading/writting from/to filesystem") {}
};

class UdpPacket {
 private:
  void readChar(std::stringstream &buffer, char chr);

 protected:
  void readPacketId(std::stringstream &buffer, const char *id);
  void readSpace(std::stringstream &buffer);
  char readChar(std::stringstream &buffer);
  char readAlphabeticalChar(std::stringstream &buffer);
  void readPacketDelimiter(std::stringstream &buffer);
  std::string readString(std::stringstream &buffer, uint32_t max_len);
  std::string readAlphabeticalString(std::stringstream &buffer,
                                     uint32_t max_len);
  uint32_t readInt(std::stringstream &buffer);
  uint32_t readPlayerId(std::stringstream &buffer);

 public:
  virtual std::stringstream serialize() = 0;
  virtual void deserialize(std::stringstream &buffer) = 0;

  virtual ~UdpPacket() = default;
};

// Start New Game Packet (SNG)
class StartGameServerbound : public UdpPacket {
 public:
  static constexpr const char *ID = "SNG";
  uint32_t player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

// Reply to Start Game Packet (RSG)
class ReplyStartGameClientbound : public UdpPacket {
 public:
  enum status { OK, NOK, ERR };
  static constexpr const char *ID = "RSG";
  status status;
  uint32_t n_letters;
  uint32_t max_errors;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessLetterServerbound : public UdpPacket {
 public:
  static constexpr const char *ID = "PLG";
  uint32_t player_id;
  char guess;
  uint32_t trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessLetterClientbound : public UdpPacket {
 public:
  enum status { OK, WIN, DUP, NOK, OVR, INV, ERR };
  static constexpr const char *ID = "RLG";
  status status;
  uint32_t trial;
  std::vector<uint32_t> pos;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessWordServerbound : public UdpPacket {
 public:
  static constexpr const char *ID = "PWG";
  uint32_t player_id;
  std::string guess;
  uint32_t trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class GuessWordClientbound : public UdpPacket {
 public:
  enum status { WIN, DUP, NOK, OVR, INV, ERR };
  static constexpr const char *ID = "RWG";
  status status;
  uint32_t trial;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class QuitGameServerbound : public UdpPacket {
 public:
  static constexpr const char *ID = "QUT";
  uint32_t player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class QuitGameClientbound : public UdpPacket {
 public:
  enum status { OK, NOK, ERR };
  static constexpr const char *ID = "RQT";
  status status;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class RevealWordServerbound : public UdpPacket {
 public:
  static constexpr const char *ID = "REV";
  uint32_t player_id;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class RevealWordClientbound : public UdpPacket {
 public:
  static constexpr const char *ID = "RRV";
  uint32_t wordLen;
  std::string word;

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class ErrorUdpPacket : public UdpPacket {
 public:
  static constexpr const char *ID = "ERR";

  std::stringstream serialize();
  void deserialize(std::stringstream &buffer);
};

class TcpPacket {
 private:
  char delimiter = 0;

  void readChar(int fd, char chr);

 protected:
  void writeString(int fd, const std::string &str);
  void readPacketId(int fd, const char *id);
  void readSpace(int fd);
  char readChar(int fd);
  void readPacketDelimiter(int fd);
  std::string readString(const int fd);
  uint32_t readInt(const int fd);
  uint32_t readPlayerId(const int fd);
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
 public:
  enum status { OK, EMPTY };
  static constexpr const char *ID = "RSB";
  status status;
  std::string file_name;
  std::string file_data;

  void send(int fd);
  void receive(int fd);
};

class HintServerbound : public TcpPacket {
 public:
  static constexpr const char *ID = "GHL";
  uint32_t player_id;

  void send(int fd);
  void receive(int fd);
};

class StateServerbound : public TcpPacket {
 public:
  static constexpr const char *ID = "STA";
  uint32_t player_id;

  void send(int fd);
  void receive(int fd);
};

class StateClientbound : public TcpPacket {
 public:
  enum status { ACT, FIN, NOK };
  static constexpr const char *ID = "RST";
  status status;
  std::string file_name;
  std::string file_data;

  void send(int fd);
  void receive(int fd);
};

class HintClientbound : public TcpPacket {
 public:
  enum status { OK, NOK };
  static constexpr const char *ID = "RHL";
  status status;
  std::filesystem::path file_path;
  std::string file_name;

  void send(int fd);
  void receive(int fd);
};

class ErrorTcpPacket : public TcpPacket {
 public:
  static constexpr const char *ID = "ERR";

  void send(int fd);
  void receive(int fd);
};

void send_packet(UdpPacket &packet, int socket, struct sockaddr *address,
                 socklen_t addrlen);

void wait_for_packet(UdpPacket &packet, int socket);

void write_player_id(std::stringstream &buffer, const uint32_t player_id);

uint32_t parse_packet_player_id(std::string &id_str);

void sendFile(int connection_fd, std::filesystem::path image_path);

uint32_t getFileSize(std::filesystem::path file_path);

#endif
