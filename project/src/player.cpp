#include "player.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8080"

void start_game(int player_id, int socket, addrinfo *res) {
  // Create a new SNG packet
  StartGameServerbound *packet_out = new StartGameServerbound();
  packet_out->player_id = player_id;

  // TESTING: Sending and receiving a packet
  send_packet(packet_out, socket, res->ai_addr, res->ai_addrlen);
  Packet *packet_in = receive_packet(socket, res->ai_addr);
  ReplyStartGameClientbound *rsg = (ReplyStartGameClientbound *)packet_in;
  if (rsg->success) {
    printf("Game started successfully\n");
    printf("Number of letters: %d, Max errors: %d", rsg->n_letters,
           rsg->max_errors);
  } else {
    printf("Game failed to start");
  }
  fflush(stdout);
};

int main() {
  int fd, errcode;
  struct addrinfo hints, *res;

  // Create a socket
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Create a server address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket

  // Get the server address
  errcode = getaddrinfo("127.0.0.1", PORT, &hints, &res);
  if (errcode != 0) /*error*/
    exit(1);

  // TESTING
  // send SNG packet
  start_game(1, fd, res);
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
