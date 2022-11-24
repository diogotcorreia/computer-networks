#include "player.hpp"

int main() {
  CommandManager commandManager;
  registerCommands(commandManager);

  while (true) {
    commandManager.waitForCommand();
  }
  return 0;
}

void registerCommands(CommandManager& manager) {
  StartCommand startCommand;
  manager.registerCommand(startCommand);
}
