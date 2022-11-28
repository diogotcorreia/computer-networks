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
  state.game = start_game(1, state.udp_socket_fd, state.server_udp_addr);
  CommandManager commandManager;
  registerCommands(commandManager);

  while (true) {
    commandManager.waitForCommand(state);
  }
  return 0;
}

void registerCommands(CommandManager &manager) {
  manager.registerCommand(std::make_shared<StartCommand>());
}

ClientGame *start_game(int player_id, int socket, addrinfo *res) {
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
    return new ClientGame(player_id, rsg.n_letters, rsg.max_errors);
  } else {
    printf("Game failed to start");
  }
  fflush(stdout);
  return NULL;
};
