#include "commands.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "client_game.hpp"
#include "common/protocol.hpp"

extern bool is_shutting_down;

void CommandManager::printHelp() {
  std::cout << std::endl << "Available commands:" << std::endl << std::left;

  for (auto it = this->handlerList.begin(); it != this->handlerList.end();
       ++it) {
    auto handler = *it;
    std::ostringstream ss;
    ss << handler->name;
    if (handler->usage) {
      ss << " " << *handler->usage;
    }
    std::cout << std::setw(HELP_MENU_COMMAND_COLUMN_WIDTH) << ss.str();
    std::cout << std::setw(HELP_MENU_DESCRIPTION_COLUMN_WIDTH)
              << handler->description;
    if (handler->alias) {
      std::cout << "(Alias: " << *handler->alias << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl << std::resetiosflags(std::ios_base::basefield);
}

void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
  this->handlerList.push_back(handler);
  this->handlers.insert({handler->name, handler});
  if (handler->alias) {
    this->handlers.insert({*handler->alias, handler});
  }
}

void CommandManager::waitForCommand(PlayerState& state) {
  print_game_progress(state);
  std::cout << "> ";

  std::string line;
  std::getline(std::cin, line);

  if (std::cin.eof() || is_shutting_down) {
    return;
  }

  auto splitIndex = line.find(' ');

  std::string commandName;
  if (splitIndex == std::string::npos) {
    commandName = line;
    line = "";
  } else {
    commandName = line.substr(0, splitIndex);
    line.erase(0, splitIndex + 1);
  }

  if (commandName.length() == 0) {
    return;
  }

  auto handler = this->handlers.find(commandName);
  if (handler == this->handlers.end()) {
    std::cout << "Unknown command" << std::endl;
    return;
  }

  try {
    handler->second->handle(line, state);
  } catch (std::exception& e) {
    std::cout << "[ERROR] " << e.what() << std::endl;
  } catch (...) {
    std::cout << "[ERROR] An unknown error occurred." << std::endl;
  }
}

/* Command handlers */

void StartCommand::handle(std::string args, PlayerState& state) {
  uint32_t player_id;
  // Argument parsing
  try {
    player_id = parse_player_id(args);
  } catch (...) {
    std::cout << "Invalid player ID. It must be a positive number up to "
              << PLAYER_ID_MAX_LEN << " digits" << std::endl;
    return;
  }

  // Populate and send packet
  StartGameServerbound packet_out;
  packet_out.player_id = player_id;

  ReplyStartGameClientbound rsg;
  state.sendUdpPacketAndWaitForReply(packet_out, rsg);

  // Check status
  ClientGame* game;
  switch (rsg.status) {
    case ReplyStartGameClientbound::status::OK:
      // Start game
      game = new ClientGame(player_id, rsg.n_letters, rsg.max_errors);
      state.startGame(game);
      // Output game info
      std::cout << "Game started successfully!" << std::endl;
      break;

    case ReplyStartGameClientbound::status::NOK:
      std::cout
          << "Game failed to start: that player already has an on-going game."
          << std::endl;
      break;

    case ReplyStartGameClientbound::status::ERR:
    default:
      std::cout << "Game failed to start: packet was wrongly structured."
                << std::endl;
      break;
  }
}

void GuessLetterCommand::handle(std::string args, PlayerState& state) {
  // Check if there is a game running
  if (!is_game_active(state)) {
    return;
  }
  // Argument parsing
  if (args.length() != 1 || args[0] < 'a' || args[0] > 'z') {
    std::cout << "Invalid letter. It must be a single lowercase letter"
              << std::endl;
    return;
  }
  char guess = args[0];

  // Populate and send packet
  GuessLetterServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  packet_out.guess = guess;
  packet_out.trial = state.game->getCurrentTrial();

  GuessLetterClientbound rlg;
  state.sendUdpPacketAndWaitForReply(packet_out, rlg);

  // Check packet status
  if (rlg.status == GuessLetterClientbound::status::ERR) {
    std::cout
        << "Word guess failed: there is no on-going game for this player on "
           "the server. Use 'state' command to check the state of the game."
        << std::endl;
    return;
  }

  switch (rlg.status) {
    case GuessLetterClientbound::status::OK:
      // Update game state
      for (auto it = rlg.pos.begin(); it != rlg.pos.end(); ++it) {
        state.game->updateWordChar(*it - 1, guess);
      }
      state.game->increaseTrial();
      std::cout << "Letter '" << guess << "' is part of the word!" << std::endl;
      break;

    case GuessLetterClientbound::status::WIN:
      // Update game state
      for (uint32_t i = 0; i < state.game->getWordLen(); i++) {
        if (state.game->getWordProgress()[i] == '_') {
          state.game->updateWordChar(i, guess);
        }
      }
      state.game->increaseTrial();
      // Output game info
      print_game_progress(state);
      state.game->finishGame();
      std::cout << "Letter '" << guess << "' is part of the word!" << std::endl;
      std::cout << "YOU WON!" << std::endl;
      break;

    case GuessLetterClientbound::status::DUP:
      std::cout << "Letter '" << guess << "' has already been guessed before."
                << std::endl;
      break;

    case GuessLetterClientbound::status::NOK:
      std::cout << "Letter '" << guess << "' is NOT part of the word."
                << std::endl;
      state.game->increaseTrial();
      state.game->updateNumErrors();
      break;

    case GuessLetterClientbound::status::OVR:
      state.game->updateNumErrors();
      state.game->increaseTrial();
      print_game_progress(state);
      state.game->finishGame();
      std::cout << "Letter '" << guess << "' is NOT part of the word."
                << std::endl;
      std::cout << "Game over. You've lost." << std::endl;
      break;

    case GuessLetterClientbound::status::INV:
      std::cout << "Client and game server are out-of-sync. Please quit the "
                   "current game and start a new one."
                << std::endl;
      break;

    case GuessLetterClientbound::status::ERR:
    default:
      break;
  }
}

void GuessWordCommand::handle(std::string args, PlayerState& state) {
  // Check if there is a game running
  if (!is_game_active(state)) {
    return;
  }

  // Argument parsing
  if (args.length() != state.game->getWordLen()) {
    std::cout << "Invalid argument. It must be a word of length "
              << state.game->getWordLen() << std::endl;
    return;
  }

  // Populate and send packet
  GuessWordServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();
  packet_out.trial = state.game->getCurrentTrial();
  packet_out.guess = args;

  GuessWordClientbound rwg;
  state.sendUdpPacketAndWaitForReply(packet_out, rwg);

  // Check packet status
  if (rwg.status == GuessWordClientbound::status::ERR) {
    std::cout
        << "Word guess failed: there is no on-going game for this player on "
           "the server. Use 'state' command to check the state of the game."
        << std::endl;
    return;
  }

  switch (rwg.status) {
    case GuessWordClientbound::status::WIN:
      // Update game state
      for (uint32_t i = 0; i < state.game->getWordLen(); i++) {
        state.game->updateWordChar(i, args[i]);
      }
      state.game->increaseTrial();
      // Output game info
      print_game_progress(state);
      state.game->finishGame();
      std::cout << "Word '" << args << "' is the correct word." << std::endl;
      std::cout << "YOU WON!" << std::endl;
      break;
    case GuessWordClientbound::status::DUP:
      std::cout << "Word '" << args << "' has already been guessed before."
                << std::endl;
      break;
    case GuessWordClientbound::status::NOK:
      state.game->updateNumErrors();
      state.game->increaseTrial();
      std::cout << "Word '" << args << "' is NOT the correct word."
                << std::endl;
      break;

    case GuessWordClientbound::status::OVR:
      state.game->updateNumErrors();
      state.game->increaseTrial();
      std::cout << "Word '" << args << "' is NOT the correct word."
                << std::endl;
      print_game_progress(state);
      state.game->finishGame();
      std::cout << "Game over. You've lost." << std::endl;
      break;

    case GuessWordClientbound::status::INV:
      std::cout << "Communicated wrong trial number" << std::endl;
      break;

    case GuessWordClientbound::status::ERR:
    default:
      break;
  }
}

void QuitCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args
  // Check if there is a game running
  if (!is_game_active(state)) {
    return;
  }

  // Populate and send packet
  QuitGameServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();

  QuitGameClientbound rq;
  state.sendUdpPacketAndWaitForReply(packet_out, rq);

  // Check packet status
  switch (rq.status) {
    case QuitGameClientbound::status::OK:
      std::cout << "Game quit successfully." << std::endl;
      state.game->finishGame();
      break;

    case QuitGameClientbound::status::NOK:
      std::cout << "Game had already finished." << std::endl;
      state.game->finishGame();
      break;

    case QuitGameClientbound::status::ERR:
    default:
      std::cout << "Error with the request. Please try again." << std::endl;
      break;
  }
}

void ExitCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args
  if (state.hasActiveGame()) {
    // Populate and send packet
    QuitGameServerbound packet_out;
    packet_out.player_id = state.game->getPlayerId();

    QuitGameClientbound rq;
    state.sendUdpPacketAndWaitForReply(packet_out, rq);

    // Check packet status
    switch (rq.status) {
      case QuitGameClientbound::status::OK:
        std::cout << "Game quit successfully." << std::endl;
        state.game->finishGame();
        break;

      case QuitGameClientbound::status::NOK:
        std::cout << "Game had already finished." << std::endl;
        state.game->finishGame();
        break;

      case QuitGameClientbound::status::ERR:
      default:
        std::cout << "Failed to quit game." << std::endl;
        break;
    }
  }
  is_shutting_down = true;
}

void RevealCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args
  // Check if there is a game running
  if (!is_game_active(state)) {
    return;
  }
  // Populate and send packet
  RevealWordServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();

  RevealWordClientbound rrv;
  state.sendUdpPacketAndWaitForReply(packet_out, rrv);

  std::cout << "Word: " << rrv.word << std::endl;
}

void ScoreboardCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args

  ScoreboardServerbound scoreboard_packet;
  ScoreboardClientbound packet_reply;

  state.sendTcpPacketAndWaitForReply(scoreboard_packet, packet_reply);
  switch (packet_reply.status) {
    case ScoreboardClientbound::status::OK:
      std::cout << "Received scoreboard and saved to file." << std::endl;
      std::cout << "Path: " << packet_reply.file_name << std::endl;
      display_file(packet_reply.file_name);
      break;

    case ScoreboardClientbound::status::EMPTY:
      std::cout << "Empty scoreboard" << std::endl;
      break;

    default:
      break;
  }
}

void HintCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args
  // Check if there is a game running
  if (!is_game_active(state)) {
    return;
  }

  HintServerbound hint_packet;
  hint_packet.player_id = state.game->getPlayerId();
  HintClientbound packet_reply;

  state.sendTcpPacketAndWaitForReply(hint_packet, packet_reply);

  switch (packet_reply.status) {
    case HintClientbound::status::OK:
      std::cout << "Received hint and saved to file." << std::endl;
      std::cout << "Path: " << packet_reply.file_name << std::endl;
      break;

    case HintClientbound::status::NOK:
      std::cout << "No hint available :(" << std::endl;
      break;

    default:
      break;
  }
}

void StateCommand::handle(std::string args, PlayerState& state) {
  (void)args;  // unused - no args
  if (!state.hasGame()) {
    std::cout << "You need to start a game to use this command." << std::endl;
    return;
  }

  StateServerbound packet_out;
  packet_out.player_id = state.game->getPlayerId();

  StateClientbound packet_reply;
  state.sendTcpPacketAndWaitForReply(packet_out, packet_reply);

  switch (packet_reply.status) {
    case StateClientbound::status::ACT:
      std::cout << "There is an active game." << std::endl;
      std::cout << "Path to file: " << packet_reply.file_name << std::endl;
      display_file(packet_reply.file_name);
      break;
    case StateClientbound::status::FIN:
      std::cout << "There is a finished game." << std::endl;
      std::cout << "Path to file: " << packet_reply.file_name << std::endl;
      display_file(packet_reply.file_name);
      if (state.hasActiveGame()) {
        state.game->finishGame();
      }
      break;
    case StateClientbound::status::NOK:
      std::cout << "There is no game history available for this player."
                << std::endl;
      break;

    default:
      break;
  }
}

void HelpCommand::handle(std::string args, PlayerState& state) {
  (void)args;   // unused - no args
  (void)state;  // unused
  manager.printHelp();
}

void KillCommand::handle(std::string args, PlayerState& state) {
  uint32_t player_id;
  // Argument parsing
  try {
    player_id = parse_player_id(args);
  } catch (...) {
    std::cout << "Invalid player ID. It must be a positive number up to "
              << PLAYER_ID_MAX_LEN << " digits" << std::endl;
    return;
  }

  // Populate and send packet
  QuitGameServerbound packet_out;
  packet_out.player_id = player_id;

  QuitGameClientbound rq;
  state.sendUdpPacketAndWaitForReply(packet_out, rq);
  // Check packet status
  switch (rq.status) {
    case QuitGameClientbound::status::OK:
      std::cout << "Game quit successfully." << std::endl;
      break;

    case QuitGameClientbound::status::NOK:
      std::cout << "There is no on-going game for this player." << std::endl;
      // Game was already finished
      break;

    case QuitGameClientbound::status::ERR:
    default:
      std::cout << "Failed to quit game on server." << std::endl;
      break;
  }
}

void write_word(std::ostream& stream, char* word, uint32_t word_len) {
  for (uint32_t i = 0; i < word_len; ++i) {
    if (i != 0) {
      stream << ' ';
    }
    stream << word[i];
  }
}

bool is_game_active(PlayerState& state) {
  if (!state.hasActiveGame()) {
    std::cout << "There is no on-going game. Please start a game using the "
                 "'start' command."
              << std::endl;
  }
  return state.hasActiveGame();
}

void print_game_progress(PlayerState& state) {
  if (!state.hasActiveGame()) {
    return;
  }
  std::cout << std::endl << "Word progress: ";
  write_word(std::cout, state.game->getWordProgress(),
             state.game->getWordLen());
  std::cout << std::endl;
  std::cout << "Current trial: " << state.game->getCurrentTrial() << std::endl;
  std::cout << "Number of errors: " << state.game->getNumErrors() << "/"
            << state.game->getMaxErrors() << std::endl
            << std::endl;
}

uint32_t parse_player_id(std::string& args) {
  size_t converted = 0;
  long player_id = std::stol(args, &converted, 10);
  if (converted != args.length() || player_id <= 0 ||
      player_id > PLAYER_ID_MAX) {
    throw std::runtime_error("invalid player id");
  }

  return (uint32_t)player_id;
}

void display_file(std::string filename) {
  std::ifstream f(filename);
  if (f.is_open()) {
    std::cout << f.rdbuf() << std::endl;
  } else {
    std::cout
        << "Failed to open file to display. Please open the file manually:"
        << filename << std::endl;
  }
}
