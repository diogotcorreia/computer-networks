#include "player.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO read from args
#define HOSTNAME "127.0.0.1"
#define PORT "8080"

int main() {
  PlayerState state;
  state.resolveServerAddress(HOSTNAME, PORT);

  // TESTING
  // send SNG packet
  start_game(1, state.udp_socket_fd, state.server_udp_addr);
  CommandManager commandManager;
  registerCommands(commandManager);

  while (true) {
    commandManager.waitForCommand();
  }
  return 0;
}

void registerCommands(CommandManager &manager) {
  manager.registerCommand(std::make_shared<StartCommand>());
}

void start_game(int player_id, int socket, addrinfo *res) {
  // Create a new SNG packet
  StartGameServerbound packet_out;
  packet_out.player_id = player_id;

  // TESTING: Sending and receiving a packet
  send_packet(packet_out, socket, res->ai_addr, res->ai_addrlen);

  ReplyStartGameClientbound rsg;
  wait_for_packet(rsg, socket);
  if (rsg.success) {
    printf("Game started successfully\n");
    printf("Number of letters: %d, Max errors: %d", rsg.n_letters,
           rsg.max_errors);
  } else {
    printf("Game failed to start");
  }
  fflush(stdout);
};

PlayerState::PlayerState() {
  this->setup_sockets();
}

PlayerState::~PlayerState() {
  // TODO check return of close (?)
  close(this->udp_socket_fd);
  close(this->tcp_socket_fd);
  freeaddrinfo(this->server_udp_addr);
  freeaddrinfo(this->server_tcp_addr);
}

void PlayerState::setup_sockets() {
  // Create a UDP socket
  if ((this->udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a UDP socket");
    exit(EXIT_FAILURE);
  }

  // Create a TCP socket
  if ((this->tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a TCP socket");
    exit(EXIT_FAILURE);
  }
}

void PlayerState::resolveServerAddress(std::string hostname, std::string port) {
  struct addrinfo hints;
  const char *host = hostname.c_str();
  const char *port_str = port.c_str();

  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if (getaddrinfo(host, port_str, &hints, &this->server_udp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for UDP connection");
    exit(EXIT_FAILURE);
  }

  // Get TCP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP socket
  if (getaddrinfo(host, port_str, &hints, &this->server_tcp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for TCP connection");
    exit(1);
  }
}
