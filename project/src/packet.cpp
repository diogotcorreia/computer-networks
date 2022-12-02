#include "packet.hpp"

#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

void Packet::readPacketId(std::stringstream &buffer, const char *packet_id) {
  char current_char;
  while (*packet_id != '\0') {
    current_char = buffer.get();
    if (!buffer.good() || current_char != *packet_id) {
      throw UnexpectedPacketException();
    }
    ++packet_id;
  }
}

void Packet::readChar(std::stringstream &buffer, char chr) {
  if (readChar(buffer) != chr) {
    throw InvalidPacketException();
  }
}

char Packet::readChar(std::stringstream &buffer) {
  char c = buffer.get();
  if (!buffer.good()) {
    throw InvalidPacketException();
  }
  return c;
}

void Packet::readSpace(std::stringstream &buffer) {
  readChar(buffer, ' ');
}

void Packet::readPacketDelimiter(std::stringstream &buffer) {
  readChar(buffer, '\n');
}

std::unique_ptr<char[]> Packet::readString(std::stringstream &buffer,
                                           size_t max_len) {
  auto str = std::make_unique<char[]>(max_len + 1);
  buffer.get(str.get(), max_len + 1, ' ');
  if (!buffer.good()) {
    throw InvalidPacketException();
  }
  return str;
}

int32_t Packet::readInt(std::stringstream &buffer) {
  int32_t i;
  buffer >> i;
  if (!buffer.good()) {
    throw InvalidPacketException();
  }
  return i;
}

// Packet type seriliazation and deserialization methods
std::stringstream StartGameServerbound::serialize() {
  std::stringstream buffer;
  buffer << StartGameServerbound::ID << " ";
  write_player_id(buffer, player_id);
  buffer << std::endl;
  return buffer;
};

void StartGameServerbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  // Serverbound packets don't read their ID
  readSpace(buffer);
  player_id = readInt(buffer);
  readPacketDelimiter(buffer);
};

std::stringstream ReplyStartGameClientbound::serialize() {
  std::stringstream buffer;
  buffer << ReplyStartGameClientbound::ID << " ";
  if (success) {
    buffer << "OK " << n_letters << " " << max_errors;
  } else {
    buffer << "NOK";
  }
  buffer << std::endl;
  return buffer;
};

void ReplyStartGameClientbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, ReplyStartGameClientbound::ID);
  readSpace(buffer);
  auto success_str = readString(buffer, 3);
  if (strcmp(success_str.get(), "OK") == 0) {
    success = true;
    readSpace(buffer);
    n_letters = readInt(buffer);
    readSpace(buffer);
    max_errors = readInt(buffer);
  } else if (strcmp(success_str.get(), "NOK") == 0) {
    success = false;
  } else {
    throw InvalidPacketException();
  }
  readPacketDelimiter(buffer);
};

std::stringstream GuessLetterServerbound::serialize() {
  std::stringstream buffer;
  buffer << GuessLetterServerbound::ID << " ";
  write_player_id(buffer, player_id);
  buffer << " " << guess << " " << trial << std::endl;
  return buffer;
};

void GuessLetterServerbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, GuessLetterServerbound::ID);
  readSpace(buffer);
  player_id = readInt(buffer);
  readSpace(buffer);
  guess = readChar(buffer);
  readSpace(buffer);
  trial = readInt(buffer);
  readPacketDelimiter(buffer);
};

std::stringstream GuessLetterClientbound::serialize() {
  std::stringstream buffer;
  buffer << GuessLetterClientbound::ID << " " << status << " " << trial << " "
         << n << " " << pos << std::endl;
  return buffer;
};

void GuessLetterClientbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, GuessLetterClientbound::ID);
  readSpace(buffer);
  auto success = readString(buffer, 3);
  readSpace(buffer);
  trial = readInt(buffer);

  if (strcmp(success.get(), "OK") == 0) {
    status = OK;
    readSpace(buffer);
    n = readInt(buffer);
    int pos_[n];
    for (int i = 0; i < n; ++i) {
      readSpace(buffer);
      pos_[i] = readInt(buffer);
    }
    pos = pos_;
  } else if (strcmp(success.get(), "WIN") == 0) {
    status = WIN;
  } else if (strcmp(success.get(), "DUP") == 0) {
    status = DUP;
  } else if (strcmp(success.get(), "NOK") == 0) {
    status = NOK;
  } else if (strcmp(success.get(), "OVR") == 0) {
    status = OVR;
  } else if (strcmp(success.get(), "INV") == 0) {
    status = INV;
  } else if (strcmp(success.get(), "ERR") == 0) {
    status = ERR;
  } else {
    throw InvalidPacketException();
  }
};

std::stringstream GuessWordServerbound::serialize() {
  std::stringstream buffer;
  buffer << GuessWordServerbound::ID << " ";
  write_player_id(buffer, player_id);
  buffer << " " << guess << std::endl;
  return buffer;
};

void GuessWordServerbound::deserialize(std::stringstream &buffer, int wordLen) {
  buffer >> std::noskipws;
  readPacketId(buffer, GuessWordServerbound::ID);
  readSpace(buffer);
  player_id = readInt(buffer);
  readSpace(buffer);
  guess = readString(buffer, wordLen).get();
  readSpace(buffer);
  trial = readInt(buffer);
  readPacketDelimiter(buffer);
};

std::stringstream GuessWordClientbound::serialize() {
  std::stringstream buffer;
  buffer << GuessWordClientbound::ID << " " << status << " " << std::endl;
  return buffer;
};

void GuessWordClientbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, GuessWordClientbound::ID);
  readSpace(buffer);
  auto success = readString(buffer, 3);
  readSpace(buffer);
  trial = readInt(buffer);

  if (strcmp(success.get(), "WIN") == 0) {
    status = WIN;
  } else if (strcmp(success.get(), "NOK") == 0) {
    status = NOK;
  } else if (strcmp(success.get(), "OVR") == 0) {
    status = OVR;
  } else if (strcmp(success.get(), "INV") == 0) {
    status = INV;
  } else if (strcmp(success.get(), "ERR") == 0) {
    status = ERR;
  } else {
    throw InvalidPacketException();
  }
};

std::stringstream QuitGameServerbound::serialize() {
  std::stringstream buffer;
  buffer << QuitGameClientbound::ID << " " << player_id << std::endl;
  return buffer;
};

void QuitGameServerbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, QuitGameClientbound::ID);
  readSpace(buffer);
  player_id = readInt(buffer);
  readPacketDelimiter(buffer);
};

std::stringstream RevealWordServerbound::serialize() {
  std::stringstream buffer;
  buffer << RevealWordServerbound::ID << " " << player_id << std::endl;
  return buffer;
};

void RevealWordServerbound::deserialize(std::stringstream &buffer) {
  buffer >> std::noskipws;
  readPacketId(buffer, RevealWordServerbound::ID);
  readSpace(buffer);
  player_id = readInt(buffer);
  readPacketDelimiter(buffer);
};

std::stringstream RevealWordClientbound::serialize() {
  std::stringstream buffer;
  // TODO: check status formatting in buffer
  buffer << RevealWordClientbound::ID << " " << word << std::endl;
  return buffer;
};

void RevealWordClientbound::deserialize(std::stringstream &buffer,
                                        int wordLen) {
  buffer >> std::noskipws;
  readPacketId(buffer, RevealWordClientbound::ID);
  readSpace(buffer);
  word = readString(buffer, wordLen).get();
  readPacketDelimiter(buffer);
};

void TcpPacket::writeString(int fd, const std::string &str) {
  if (write(fd, str.c_str(), str.length()) != (ssize_t)str.length()) {
    throw PacketSerializationException();
  }
}

void TcpPacket::readPacketId(int fd, const char *packet_id) {
  char current_char;
  while (*packet_id != '\0') {
    if (read(fd, &current_char, 1) != 1 || current_char != *packet_id) {
      throw UnexpectedPacketException();
    }
    ++packet_id;
  }
}

void TcpPacket::readChar(int fd, char chr) {
  if (readChar(fd) != chr) {
    throw InvalidPacketException();
  }
}

char TcpPacket::readChar(int fd) {
  char c = 0;
  if (read(fd, &c, 1) != 1) {
    throw InvalidPacketException();
  }
  return c;
}

void TcpPacket::readSpace(int fd) {
  readChar(fd, ' ');
}

void TcpPacket::readPacketDelimiter(int fd) {
  readChar(fd, '\n');
}

std::string TcpPacket::readString(const int fd) {
  std::string result;
  char c = -1;

  while (c != ' ') {  // TODO do we need to check for EOF?
    if (read(fd, &c, 1) != 1) {
      throw new InvalidPacketException();
    }
    result += c;
  }

  result.pop_back();

  return result;
}

int32_t TcpPacket::readInt(const int fd) {
  std::string int_str = readString(fd);
  try {
    size_t converted = 0;
    int32_t result = std::stol(int_str, &converted, 10);
    if (converted != int_str.length() || std::iswspace(int_str.at(0))) {
      throw InvalidPacketException();
    }
    return result;
  } catch (InvalidPacketException &ex) {
    throw ex;
  } catch (...) {
    throw InvalidPacketException();
  }
}

void TcpPacket::readAndSaveToFile(int fd, const std::string &file_name,
                                  const size_t file_size) {
  std::ofstream file(file_name);

  if (!file.good()) {
    throw IOException();
  }

  size_t remaining_size = file_size;
  size_t to_read;
  ssize_t n;
  char buffer[FILE_BUFFER_LEN];
  while (remaining_size > 0) {
    to_read = std::min(remaining_size, (size_t)FILE_BUFFER_LEN);
    n = read(fd, buffer, to_read);
    if (n <= 0) {
      file.close();
      throw InvalidPacketException();
    }
    file.write(buffer, n);
    if (!file.good()) {
      file.close();
      throw IOException();
    }
    remaining_size -= n;
  }

  file.close();
}

void ScoreboardServerbound::send(int fd) {
  std::stringstream stream;
  stream << ScoreboardServerbound::ID << std::endl;
  writeString(fd, stream.str());
}

void ScoreboardServerbound::receive(int fd) {
  // Serverbound packets don't read their ID
  readPacketDelimiter(fd);
}

void ScoreboardClientbound::send(int fd) {
  std::stringstream stream;
  stream << ScoreboardClientbound::ID << " ";
  if (status == OK) {
    stream << "OK ";
    // TODO read from actual scoreboard or something
    stream << "scoreboard.txt 4 "
           << "test";
  } else if (status == EMPTY) {
    stream << "EMPTY ";
  } else {
    throw PacketSerializationException();
  }
  stream << std::endl;
  writeString(fd, stream.str());
}

void ScoreboardClientbound::receive(int fd) {
  readPacketId(fd, ScoreboardClientbound::ID);
  readSpace(fd);
  auto status = readString(fd);
  if (status == "OK") {
    this->status = OK;
    file_name = readString(fd);
    int file_size = readInt(fd);  // TODO change to long?
    readAndSaveToFile(fd, file_name, file_size);
  } else if (status == "EMTPY") {
    this->status = EMPTY;
  } else {
    throw InvalidPacketException();
  }
  readPacketDelimiter(fd);
}

void HintServerbound::send(int fd) {
  std::stringstream stream;
  stream << HintServerbound::ID << " ";
  write_player_id(stream, player_id);
  stream << std::endl;
  writeString(fd, stream.str());
}

void HintServerbound::receive(int fd) {
  // Serverbound packets don't read their ID
  readSpace(fd);
  player_id = readInt(fd);
  if (player_id < 0 || player_id > PLAYER_ID_MAX) {
    throw InvalidPacketException();  // TODO maybe need to verify that string is
                                     // 6 digits
  }
  readPacketDelimiter(fd);
}

void HintClientbound::send(int fd) {
  std::stringstream stream;
  stream << HintClientbound::ID << " ";
  if (status == OK) {
    stream << "OK ";
    // TODO read from file system
    stream << "hint.txt 4 "
           << "test";
  } else if (status == NOK) {
    stream << "NOK ";
  } else {
    throw PacketSerializationException();
  }
  stream << std::endl;
  writeString(fd, stream.str());
}

void HintClientbound::receive(int fd) {
  readPacketId(fd, HintClientbound::ID);
  readSpace(fd);
  auto status = readString(fd);
  if (status == "OK") {
    this->status = OK;
    file_name = readString(fd);
    int file_size = readInt(fd);  // TODO change to long?
    readAndSaveToFile(fd, file_name, file_size);
  } else if (status == "NOK") {
    this->status = NOK;
  } else {
    throw InvalidPacketException();
  }
  readPacketDelimiter(fd);
}

// Packet sending and receiving
void send_packet(Packet &packet, int socket, struct sockaddr *address,
                 socklen_t addrlen) {
  const std::stringstream buffer = packet.serialize();
  // ERROR HERE: address type changes in client and server
  int n = sendto(socket, buffer.str().c_str(), buffer.str().length(), 0,
                 address, addrlen);
  if (n == -1) {
    perror("sendto");
    exit(EXIT_FAILURE);
  }
}

void wait_for_packet(Packet &packet, int socket) {
  fd_set file_descriptors;
  FD_ZERO(&file_descriptors);
  FD_SET(socket, &file_descriptors);

  struct timeval timeout;
  timeout.tv_sec = UDP_TIMEOUT_SECONDS;  // wait for a response before throwing
  timeout.tv_usec = 0;

  int ready_fd = select(socket + 1, &file_descriptors, NULL, NULL, &timeout);
  if (ready_fd == -1) {
    // TODO consider throwing exception instead
    perror("select");
    exit(EXIT_FAILURE);
  } else if (ready_fd == 0) {
    throw ConnectionTimeoutException();
  }

  std::stringstream data;
  char buffer[SOCKET_BUFFER_LEN];

  int n = recvfrom(socket, buffer, SOCKET_BUFFER_LEN, 0, NULL, NULL);
  if (n == -1) {
    // TODO consider throwing exception instead
    perror("recvfrom");
    exit(EXIT_FAILURE);
  }

  data << buffer;

  packet.deserialize(data);
}

void write_player_id(std::stringstream &buffer, const int player_id) {
  buffer << std::setfill('0') << std::setw(PLAYER_ID_MAX_LEN) << player_id;
  buffer.copyfmt(std::ios(NULL));  // reset formatting
}
