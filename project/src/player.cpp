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
