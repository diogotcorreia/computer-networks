#include "commands.hpp"

#include <cstring>
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

void GuessLetterCommand::handle(std::string args, PlayerState& state) {
  char guess;

  if (args.length() != 1 || args[0] < 'a' || args[0] > 'z') {
    std::cout << "Invalid letter. It must be a single letter" << std::endl;
    return;
  }

  guess = args[0];
  GuessLetterServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  packet_out.guess = guess;
  packet_out.trial = state.game->getCurrentTrial();

  state.sendPacket(packet_out);

  GuessLetterClientbound rlg;
  state.waitForPacket(rlg);
  if (rlg.status == GuessLetterClientbound::status::OK) {
    for (int i = 0; i < rlg.n; i++) {
      state.game->updateWordChar(rlg.pos[i], guess);
    }
    state.game->updateCurrentTrial();

    std::cout << "Letter guessed successfully" << std::endl;
    std::cout << "Word progress: ";
    write_word(std::cout, state.game->getWordProgress(),
               state.game->getWordLen());
    std::cout << std::endl;
    std::cout << "Current trial: " << state.game->getCurrentTrial()
              << std::endl;
    std::cout << "Number of errors: " << state.game->getNumErrors()
              << std::endl;
  } else if (rlg.status == GuessLetterClientbound::status::WIN) {
    state.game->updateCurrentTrial();
    for (size_t i = 0; i < state.game->getWordLen(); i++) {
      if (state.game->getWordProgress()[i] == '_') {
        state.game->updateWordChar(i, guess);
      }
    }
  } else if (rlg.status == GuessLetterClientbound::status::DUP) {
    std::cout << "Letter already guessed" << std::endl;
  } else if (rlg.status == GuessLetterClientbound::status::NOK) {
    state.game->updateCurrentTrial();
    std::cout << "Letter is not in the word" << std::endl;
  } else if (rlg.status == GuessLetterClientbound::status::OVR) {
    state.game->updateCurrentTrial();
    std::cout << "Game is over" << std::endl;
  } else if (rlg.status == GuessLetterClientbound::status::INV) {
    std::cout << "Communicated trial number is not valid" << std::endl;
  } else if (rlg.status == GuessLetterClientbound::status::ERR) {
    std::cout
        << "Letter guess failed. Player id is not valid or game is not started"
        << std::endl;
  }
}

void GuessWordCommand::handle(std::string args, PlayerState& state) {
  if (args.length() != state.game->getWordLen()) {
    std::cout << "Invalid argument. It must be a word of length "
              << state.game->getWordLen() << std::endl;
    return;
  }
  GuessWordServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  packet_out.trial = state.game->getCurrentTrial();
  packet_out.wordLen = state.game->getWordLen();
  packet_out.guess = args.c_str();
  state.sendPacket(packet_out);

  GuessWordClientbound rwg;
  state.waitForPacket(rwg);
  if (rwg.status == GuessWordClientbound::status::WIN) {
    state.game->updateCurrentTrial();
    for (size_t i = 0; i < state.game->getWordLen(); i++) {
      state.game->updateWordChar(i, args[i]);
    }
    std::cout << "Word guessed successfully" << std::endl;
    std::cout << "Current trial: " << state.game->getCurrentTrial()
              << std::endl;
    std::cout << "Number of errors: " << state.game->getNumErrors()
              << std::endl;
  } else if (rwg.status == GuessWordClientbound::status::NOK) {
    state.game->updateCurrentTrial();
    std::cout << "Word is not the correct one" << std::endl;
  } else if (rwg.status == GuessWordClientbound::status::OVR) {
    state.game->updateCurrentTrial();
    std::cout << "Game is over" << std::endl;
  } else if (rwg.status == GuessWordClientbound::status::INV) {
    std::cout << "Communicated wrong trial number" << std::endl;
  } else if (rwg.status == GuessWordClientbound::status::ERR) {
    std::cout
        << "Word guess failed. Player id is not valid or game is not started"
        << std::endl;
  }
}

void QuitCommand::handle(std::string args, PlayerState& state) {
  QuitGameServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  state.sendPacket(packet_out);

  QuitGameClientbound rq;
  state.waitForPacket(rq);
  if (rq.success) {
    std::cout << "Game quit successfully" << std::endl;
  } else {
    std::cout << "Game quit failed. Player id is not valid" << std::endl;
  }
}

void ExitCommand::handle(std::string args, PlayerState& state) {
  QuitGameServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  state.sendPacket(packet_out);

  QuitGameClientbound rq;
  state.waitForPacket(rq);
  if (rq.success) {
    std::cout << "Game quit successfully" << std::endl;
  } else {
    std::cout << "Game quit failed. Player id is not valid" << std::endl;
  }
  // TODO: exit.state() and closing everything down properly;
}

void RevealCommand::handle(std::string args, PlayerState& state) {
  RevealWordServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();

  state.sendPacket(packet_out);
  RevealWordClientbound rrv;
  state.waitForPacket(rrv);
  if (rrv.success) {
    std::cout << "Word: " << rrv.word << std::endl;
  } else {
    std::cout << "Error revealing the word" << std::endl;
  }
}

void ScoreboardCommand::handle(std::string args, PlayerState& state) {
  ScoreboardServerbound scoreboard_packet;

  state.sendPacket(scoreboard_packet);

  ScoreboardClientbound packet_reply;
  state.waitForPacket(packet_reply);
  if (packet_reply.status == 0) {  // TODO try to use enum
    std::cout << "Received scoreboard and saved to file." << std::endl;
    std::cout << "Path: " << packet_reply.file_name << std::endl;
    // TODO print scoreboard (?)
  } else {
    std::cout << "Empty scoreboard" << std::endl;
  }
}

void HintCommand::handle(std::string args, PlayerState& state) {
  if (!state.hasActiveGame()) {
    std::cout << "There is no on-going game. Please start a game using the "
                 "'start' command."
              << std::endl;
    return;
  }
  HintServerbound hint_packet;
  hint_packet.player_id = state.game->getPlayerId();

  state.sendPacket(hint_packet);

  HintClientbound packet_reply;
  state.waitForPacket(packet_reply);
  if (packet_reply.status == 0) {  // TODO try to use enum
    std::cout << "Received hint and saved to file." << std::endl;
    std::cout << "Path: " << packet_reply.file_name << std::endl;
  } else {
    std::cout << "No hint available :(" << std::endl;
  }
}

void write_word(std::ostream& stream, char* word, int word_len) {
  for (int i = 0; i < word_len; ++i) {
    if (i != 0) {
      stream << ' ';
    }
    stream << word[i];
  }
}
