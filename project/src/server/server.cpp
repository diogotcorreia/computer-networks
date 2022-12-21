#include "server.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  ServerConfig config(argc, argv);
  if (config.help) {
    config.printHelp(std::cout);
    exit(EXIT_SUCCESS);
  }
  GameServerState state(config.wordFilePath, config.port, config.verbose,
                        config.test);
  state.registerPacketHandlers();

  if (config.test) {
    std::cout << "Words will be selected sequentially" << std::endl;
  } else {
    std::cout << "Words will be selected randomly" << std::endl;
  }

  state.cdebug << "Verbose mode is active" << std::endl << std::endl;

  std::thread tcp_thread(main_tcp, std::ref(state));
  // TODO gracefully stop server
  while (true) {
    // TESTING: receiving and sending a packet
    wait_for_udp_packet(state);
  }

  // TODO find a way to signal the TCP thread to stop gracefully
  tcp_thread.join();
}

void main_tcp(GameServerState &state) {
  WorkerPool worker_pool(state);

  if (listen(state.tcp_socket_fd, TCP_MAX_QUEUE_SIZE) < 0) {
    perror("listen");
    std::cerr << "TCP server is being shutdown..." << std::endl;
    exit(EXIT_FAILURE);
  }

  // TODO gracefully stop server
  while (true) {
    wait_for_tcp_packet(state, worker_pool);
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

  char addr_str[INET_ADDRSTRLEN + 1] = {0};
  inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
  std::cout << "Receiving incoming UDP message from " << addr_str << ":"
            << ntohs(addr_from.addr.sin_port) << std::endl;

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

  // TODO add exception catch to send back ERR response
  server_state.callUdpPacketHandler(packet_id_str, buffer, addr_from);
}

void wait_for_tcp_packet(GameServerState &server_state, WorkerPool &pool) {
  Address addr_from;

  addr_from.size = sizeof(addr_from.addr);
  int connection_fd =
      accept(server_state.tcp_socket_fd, (struct sockaddr *)&addr_from.addr,
             &addr_from.size);
  if (connection_fd < 0) {
    perror("accept");
    std::cerr << "[ERROR] Failed to accept a connection" << std::endl;
    return;
  }

  char addr_str[INET_ADDRSTRLEN + 1] = {0};
  inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
  std::cout << "Receiving incoming TCP connection from " << addr_str << ":"
            << ntohs(addr_from.addr.sin_port) << std::endl;

  try {
    pool.delegateConnection(connection_fd);
  } catch (std::exception &e) {
    std::cerr << "[ERROR] Failed to delegate connection to worker: " << e.what()
              << " Closing connection." << std::endl;
    close(connection_fd);
  }
}

ServerConfig::ServerConfig(int argc, char *argv[]) {
  programPath = argv[0];
  int opt;

  while ((opt = getopt(argc, argv, "-p:vht")) != -1) {
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
      case 't':
        test = true;
        break;
      case 1:
        // The `-` flag in `getopt` makes non-options behave as if they
        // were values of an option -0x01
        if (wordFilePath.empty()) {
          // Only keep the first non-option argument
          wordFilePath = std::string(optarg);
        }
        break;
      default:
        std::cerr << std::endl;
        printHelp(std::cerr);
        exit(EXIT_FAILURE);
    }
  }

  if (wordFilePath.empty()) {
    std::cerr << programPath << ": required argument 'word_file' not provided"
              << std::endl
              << std::endl;
    printHelp(std::cerr);
    exit(EXIT_FAILURE);
  }
}

void ServerConfig::printHelp(std::ostream &stream) {
  stream << "Usage: " << programPath << " word_file [-p GSport] [-v] [-t]"
         << std::endl;
  stream << "Available options:" << std::endl;
  stream << "word_file\tPath to the word file" << std::endl;
  stream << "-p GSport\tSet port of Game Server. Default: " << DEFAULT_PORT
         << std::endl;
  stream << "-h\t\tEnable verbose mode." << std::endl;
  stream << "-t\t\tEnable test mode. Words will be selected sequentially."
         << std::endl;
}
