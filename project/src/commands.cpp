#include "commands.hpp"

#include <iostream>

void CommandManager::printHelp() {
  std::cout << std::endl << "Available commands:" << std::endl;

  for (auto it = this->handlerList.begin(); it != this->handlerList.end();
       ++it) {
    CommandHandler* handler = *it;
    std::cout << handler->getName();
    if (handler->getUsage()) {
      std::cout << " " << *handler->getUsage();
    }
    std::cout << "\t" << handler->getDescription();
    if (handler->getAlias()) {
      std::cout << "\t (Alias: " << *handler->getAlias() << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void CommandManager::registerCommand(CommandHandler& handler) {
  this->handlerList.push_back(&handler);
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
