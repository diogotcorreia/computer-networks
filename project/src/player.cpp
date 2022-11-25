#include "player.hpp"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

void start_game(int player_id, int socket, sockaddr_in *address) {
  // Create a new SNG packet
  SNG *packet_out = new SNG();
  packet_out->player_id = player_id;

  // Send the packet to the server
  send_packet(packet_out, socket, address);
  delete packet_out;
  Packet *packet_in = receive_packet(socket, address, 1024);
}

int main() {
  int fd;
  // Create a socket
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Create a server address
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // send SNG packet
  start_game(1, fd, &address);

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
