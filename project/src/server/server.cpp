#include "server.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "common/protocol.hpp"

bool is_shutting_down = false;

int main(int argc, char *argv[]) {
  ServerConfig config(argc, argv);
  if (config.help) {
    config.printHelp(std::cout);
    exit(EXIT_SUCCESS);
  }
  GameServerState state(config.wordFilePath, config.port, config.verbose,
                        config.test);
  state.registerPacketHandlers();

  setup_signal_handlers();
  if (config.test) {
    std::cout << "Words will be selected sequentially" << std::endl;
  } else {
    std::cout << "Words will be selected randomly" << std::endl;
  }

  state.cdebug << "Verbose mode is active" << std::endl << std::endl;

  std::thread tcp_thread(main_tcp, std::ref(state));
  while (!is_shutting_down) {
    // TESTING: receiving and sending a packet
    wait_for_udp_packet(state);
  }

  std::cout << "Shutting down UDP server..." << std::endl;

  // TODO find a way to signal the TCP thread to stop gracefully
  tcp_thread.join();
}

void setup_signal_handlers() {
  // set SIGINT/SIGTERM handler to close server gracefully
  struct sigaction sa;
  sa.sa_handler = terminate_signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Setting SIGINT signal handler");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Setting SIGTERM signal handler");
    exit(EXIT_FAILURE);
  }

  // ignore SIGPIPE
  signal(SIGPIPE, SIG_IGN);
}

void terminate_signal_handler(int sig) {
  (void)sig;
  if (is_shutting_down) {
    exit(EXIT_SUCCESS);
  }
  is_shutting_down = true;
}

void main_tcp(GameServerState &state) {
  WorkerPool worker_pool(state);

  if (listen(state.tcp_socket_fd, TCP_MAX_QUEUE_SIZE) < 0) {
    perror("listen");
    std::cerr << "TCP server is being shutdown..." << std::endl;
    exit(EXIT_FAILURE);
  }

  while (!is_shutting_down) {
    wait_for_tcp_packet(state, worker_pool);
  }

  std::cout << "Shutting down TCP server... This might take a while if there "
               "are open connections. Press CTRL + C again to forcefully close "
               "the server."
            << std::endl;
}

void wait_for_udp_packet(GameServerState &server_state) {
  Address addr_from;
  std::stringstream stream;
  char buffer[SOCKET_BUFFER_LEN];

  addr_from.size = sizeof(addr_from.addr);
  ssize_t n = recvfrom(server_state.udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
                       (struct sockaddr *)&addr_from.addr, &addr_from.size);
  if (is_shutting_down) {
    return;
  }
  if (n == -1) {
    if (errno == EAGAIN) {
      return;
    }
    perror("recvfrom");
    exit(EXIT_FAILURE);
  }
  addr_from.socket = server_state.udp_socket_fd;

  char addr_str[INET_ADDRSTRLEN + 1] = {0};
  inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
  std::cout << "Receiving incoming UDP message from " << addr_str << ":"
            << ntohs(addr_from.addr.sin_port) << std::endl;

  stream.write(buffer, n);

  return handle_packet(stream, addr_from, server_state);
}

void handle_packet(std::stringstream &buffer, Address &addr_from,
                   GameServerState &server_state) {
  try {
    char packet_id[PACKET_ID_LEN + 1];
    buffer >> packet_id;

    if (!buffer.good()) {
      std::cerr << "Received malformatted packet ID" << std::endl;
      throw InvalidPacketException();
    }

    std::string packet_id_str = std::string(packet_id);

    server_state.callUdpPacketHandler(packet_id_str, buffer, addr_from);
  } catch (InvalidPacketException &e) {
    try {
      ErrorUdpPacket error_packet;
      send_packet(error_packet, addr_from.socket,
                  (struct sockaddr *)&addr_from.addr, addr_from.size);
    } catch (std::exception &ex) {
      std::cerr << "Failed to reply with ERR packet: " << ex.what()
                << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << "Failed to handle UDP packet: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Failed to handle UDP packet: unknown" << std::endl;
  }
}

void wait_for_tcp_packet(GameServerState &server_state, WorkerPool &pool) {
  Address addr_from;

  addr_from.size = sizeof(addr_from.addr);
  int connection_fd =
      accept(server_state.tcp_socket_fd, (struct sockaddr *)&addr_from.addr,
             &addr_from.size);
  if (is_shutting_down) {
    return;
  }
  if (connection_fd < 0) {
    if (errno == EAGAIN) {  // timeout, just go around and keep listening
      return;
    }
    perror("accept");
    std::cerr << "[ERROR] Failed to accept a connection" << std::endl;
    return;
  }

  struct timeval read_timeout;
  read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
  read_timeout.tv_usec = 0;
  if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
                 sizeof(read_timeout)) < 0) {
    // TODO consider using exceptions (?)
    perror("Failed to set socket options");
    exit(EXIT_FAILURE);
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
