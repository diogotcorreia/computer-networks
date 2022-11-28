#include "commands.hpp"

#include <iostream>

void CommandManager::printHelp() {
  std::cout << std::endl << "Available commands:" << std::endl;

  for (auto it = this->handlerList.begin(); it != this->handlerList.end();
       ++it) {
    auto handler = *it;
    std::cout << handler->name;
    if (handler->usage) {
      std::cout << " " << *handler->usage;
    }
    std::cout << "\t" << handler->description;
    if (handler->alias) {
      std::cout << "\t (Alias: " << *handler->alias << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
  this->handlerList.push_back(handler);
  this->handlers.insert({handler->name, handler});
  if (handler->alias) {
    this->handlers.insert({*handler->alias, handler});
  }
}

void CommandManager::waitForCommand(PlayerState& state) {
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

  handler->second->handle(line, state);
}

void StartCommand::handle(std::string args, PlayerState& state) {
  // TODO validate args
  int player_id = atoi(args.c_str());

  // Ask the game server to start a game
  StartGameServerbound packet_out;
  packet_out.player_id = player_id;

  // TESTING: Sending and receiving a packet
  state.sendPacket(packet_out);

  ReplyStartGameClientbound rsg;
  state.waitForPacket(rsg);
  if (rsg.success) {
    std::cout << "Game started successfully" << std::endl;
    std::cout << "Number of letters: " << rsg.n_letters
              << ", Max errors: " << rsg.max_errors << std::endl;
    ClientGame* game = new ClientGame(player_id, rsg.n_letters, rsg.max_errors);
    state.startGame(game);
  } else {
    std::cout << "Game failed to start" << std::endl;
  }
  std::cout << "Executed start command with args `" << args << "`! :)"
            << std::endl;
}
