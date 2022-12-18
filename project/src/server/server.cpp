#include "server.hpp"

#include <unistd.h>

#include <iostream>

#include "common/constants.hpp"

int main(int argc, char *argv[]) {
  ServerConfig config(argc, argv);
  if (config.help) {
    config.printHelp(std::cout);
    exit(EXIT_SUCCESS);
  }
  GameServerState state(config.wordFilePath, config.port, config.verbose);
  state.registerPacketHandlers();

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

ServerConfig::ServerConfig(int argc, char *argv[]) {
  programPath = argv[0];
  int opt;

  opterr = 0;
  while ((opt = getopt(argc, argv, "-:p:vh")) != -1) {
    switch (opt) {
      case 'p':
        port = std::string(optarg);
        break;
      case 'h':
        help = true;
        return;
        break;
      case 'v':
        verbose = true;
        break;
      case 1:
        // The `-` flag in `getopt` makes non-options behave as if they
        // were values of an option -0x01
        if (wordFilePath.empty()) {
          // Only keep the first non-option argument
          wordFilePath = std::string(optarg);
        }
        break;
      case ':':
        std::cerr << "Missing required value for option -" << (char)optopt
                  << std::endl
                  << std::endl;
        printHelp(std::cerr);
        exit(EXIT_FAILURE);
        break;
      default:
        std::cerr << "Unknown argument -" << (char)optopt << std::endl
                  << std::endl;
        printHelp(std::cerr);
        exit(EXIT_FAILURE);
    }
  }

  if (wordFilePath.empty()) {
    std::cerr << "Required argument word_file not provided" << std::endl
              << std::endl;
    printHelp(std::cerr);
    exit(EXIT_FAILURE);
  }
}

void ServerConfig::printHelp(std::ostream &stream) {
  stream << "Usage: " << programPath << " word_file [-p GSport] [-v]"
         << std::endl;
  stream << "Available arguments:" << std::endl;
  stream << "word_file\tPath to the word file" << std::endl;
  stream << "-p GSport\tSet port of Game Server. Default: " << DEFAULT_PORT
         << std::endl;
  stream << "-h\t\tEnable verbose mode." << std::endl;
}
