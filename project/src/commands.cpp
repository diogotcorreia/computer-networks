#include "commands.hpp"

#include <iostream>

void CommandManager::printHelp() {
  std::cout << "Available commands:" << std::endl;
  std::cout << "TODO" << std::endl;
}

void CommandManager::registerCommand(CommandHandler& handler) {
  this->handlers.insert({handler.getName(), handler});
  if (handler.getAlias()) {
    this->handlers.insert({*handler.getAlias(), handler});
  }
}

void CommandManager::waitForCommand() {
  std::string line;
  std::getline(std::cin, line);

  std::cout << line;
}

void StartCommand::handle(std::stringstream args) {
  std::cout << "Executed start command! :)" << std::endl;
}
