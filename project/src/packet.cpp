#include "packet.hpp"

#include <sys/types.h>

#include <cstring>

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
  char c = buffer.get();
  if (!buffer.good() || c != chr) {
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
  buffer << StartGameServerbound::ID << " " << player_id << std::endl;
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
  buffer << GuessLetterServerbound::ID << " " << player_id << " " << guess
         << " " << trial << std::endl;
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
  buffer << GuessWordServerbound::ID << " " << player_id << " " << guess
         << std::endl;
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

void wait_for_packet(Packet &packet, int socket) {
  std::stringstream data;
  char buffer[SOCKET_BUFFER_LEN];

  int n = recvfrom(socket, buffer, SOCKET_BUFFER_LEN, 0, NULL, NULL);
  // TODO throw exception instead
  // TODO handle case where UDP response never arrives
  if (n == -1) {
    perror("recvfrom");
    exit(1);
  }

  data << buffer;

  packet.deserialize(data);
}