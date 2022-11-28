#include "commands.hpp"

#include <iostream>

#include "game.hpp"
#include "packet.hpp"

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
  long player_id;
  try {
    size_t converted = 0;
    player_id = std::stol(args, &converted, 10);
    if (converted != args.length() || player_id <= 0 ||
        player_id > PLAYER_ID_MAX) {
      throw std::runtime_error("invalid player id");
    }
  } catch (...) {
    std::cout << "Invalid player ID. It must be a positive number up to "
              << PLAYER_ID_MAX_LEN << " digits" << std::endl;
    return;
  }

  // Ask the game server to start a game
  StartGameServerbound packet_out;
  packet_out.player_id = player_id;

  // TESTING: Sending and receiving a packet
  state.sendPacket(packet_out);

  ReplyStartGameClientbound rsg;
  state.waitForPacket(rsg);
  if (rsg.success) {
    ClientGame* game = new ClientGame(player_id, rsg.n_letters, rsg.max_errors);
    state.startGame(game);

    std::cout << "Game started successfully" << std::endl;
    std::cout << "Number of letters: " << rsg.n_letters
              << ", Max errors: " << rsg.max_errors << std::endl;
    std::cout << "Guess the word: ";
    write_word(std::cout, game->getWordProgress(), game->getWordLen());
    std::cout << std::endl;
  } else {
    std::cout << "Game failed to start" << std::endl;
  }
}

void ScoreboardCommand::handle(std::string args, PlayerState& state) {
  ScoreboardServerbound scoreboard_packet;

  state.sendPacket(scoreboard_packet);

  std::cout << "[DEBUG] Asked for scoreboard" << std::endl;
}

void write_word(std::ostream& stream, char* word, int word_len) {
  for (int i = 0; i < word_len; ++i) {
    if (i != 0) {
      stream << ' ';
    }
    stream << word[i];
  }
}
