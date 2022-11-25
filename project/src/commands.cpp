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
  this->printHelp();
  std::cout << "> ";

  std::string line;
  std::getline(std::cin, line);

  int splitIndex = line.find(' ');

  std::string commandName;
  if (splitIndex == -1) {
    commandName = line;
    line = "";
  } else {
    commandName = line.substr(0, splitIndex);
    line.erase(0, splitIndex + 1);
  }

  auto handler = this->handlers.find(commandName);
  if (handler == this->handlers.end()) {
    std::cout << "Unknown command" << std::endl;
    return;
  }

  handler->second.handle(line);
}

void StartCommand::handle(std::string args) {
  std::cout << "Executed start command with args `" << args << "`! :)"
            << std::endl;
}
