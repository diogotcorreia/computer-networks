#include "packet_handlers.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "common/protocol.hpp"

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state) {
  StartGameServerbound packet;
  ReplyStartGameClientbound response;

  try {
    packet.deserialize(buffer);
    state.cdebug << playerTag(packet.player_id) << "Asked to start game"
                 << std::endl;

    ServerGameSync game = state.createGame(packet.player_id);

    response.status = ReplyStartGameClientbound::OK;
    response.n_letters = game->getWordLen();
    response.max_errors = game->getMaxErrors();

    game->saveToFile();

    state.cdebug << playerTag(packet.player_id) << "Game started with word '"
                 << game->getWord() << "' and with " << game->getMaxErrors()
                 << " errors allowed" << std::endl;
  } catch (GameAlreadyStartedException &e) {
    state.cdebug << playerTag(packet.player_id) << "Game already started"
                 << std::endl;
    response.status = ReplyStartGameClientbound::NOK;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Start Game] Invalid packet received" << std::endl;
    response.status = ReplyStartGameClientbound::ERR;
  } catch (std::exception &e) {
    std::cerr << "[Start Game] There was an unhandled exception that prevented "
                 "the server from starting a new game:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_guess_letter(std::stringstream &buffer, Address &addr_from,
                         GameServerState &state) {
  GuessLetterServerbound packet;
  GuessLetterClientbound response;
  try {
    packet.deserialize(buffer);

    state.cdebug << playerTag(packet.player_id) << "Guessed letter '"
                 << packet.guess << "'" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    response.trial = game->getCurrentTrial();
    auto found = game->guessLetter(packet.guess, packet.trial);
    // Must set trial again in case of replays
    response.trial = game->getCurrentTrial() - 1;

    game->saveToFile();

    if (game->hasLost()) {
      response.status = GuessLetterClientbound::status::OVR;
      state.cdebug << playerTag(packet.player_id) << "Game lost" << std::endl;
    } else if (found.size() == 0) {
      response.status = GuessLetterClientbound::status::NOK;
      state.cdebug << playerTag(packet.player_id) << "Wrong letter '"
                   << packet.guess << "'" << std::endl;
    } else if (game->hasWon()) {
      response.status = GuessLetterClientbound::status::WIN;
      state.scoreboard.addGame(game.game);
      state.cdebug << playerTag(packet.player_id) << "Won the game. Word was '"
                   << game->getWord() << "'" << std::endl;
    } else {
      response.status = GuessLetterClientbound::status::OK;
      state.cdebug << playerTag(packet.player_id) << "Correct letter '"
                   << packet.guess << "'. Trial " << response.trial
                   << ". Progress: " << game->getWordProgress() << std::endl;
    }
    response.pos = found;
  } catch (NoGameFoundException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << playerTag(packet.player_id) << "No game found" << std::endl;
  } catch (DuplicateLetterGuessException &e) {
    response.status = GuessLetterClientbound::status::DUP;
    response.trial -= 1;
    state.cdebug << playerTag(packet.player_id) << "Guessed duplicate letter '"
                 << packet.guess << "'" << std::endl;
  } catch (InvalidTrialException &e) {
    response.status = GuessLetterClientbound::status::INV;
    response.trial -= 1;
    state.cdebug << playerTag(packet.player_id) << "Invalid trial "
                 << packet.trial << std::endl;
  } catch (GameHasEndedException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << playerTag(packet.player_id) << "Game has ended"
                 << std::endl;
  } catch (InvalidPacketException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << "[Guess Letter] Invalid packet received" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "[Guess Letter] There was an unhandled exception that "
                 "prevented the server from handling a letter guess:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_guess_word(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state) {
  GuessWordServerbound packet;
  GuessWordClientbound response;
  try {
    packet.deserialize(buffer);

    state.cdebug << playerTag(packet.player_id) << "Guessed word '"
                 << packet.guess << "'" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    response.trial = game->getCurrentTrial();
    bool correct = game->guessWord(packet.guess, packet.trial);
    // Must set trial again in case of replays
    response.trial = game->getCurrentTrial() - 1;

    game->saveToFile();

    if (game->hasLost()) {
      response.status = GuessWordClientbound::status::OVR;
      state.cdebug << playerTag(packet.player_id) << "Game lost" << std::endl;
    } else if (correct) {
      response.status = GuessWordClientbound::status::WIN;
      state.scoreboard.addGame(game.game);
      state.cdebug << playerTag(packet.player_id) << "Guess was correct"
                   << std::endl;
    } else {
      response.status = GuessWordClientbound::status::NOK;
      state.cdebug << playerTag(packet.player_id) << "Guess was wrong. Trial "
                   << response.trial << " Progress: " << game->getWordProgress()
                   << std::endl;
    }
  } catch (NoGameFoundException &e) {
    response.status = GuessWordClientbound::status::ERR;
    state.cdebug << playerTag(packet.player_id) << "No game found" << std::endl;
  } catch (DuplicateWordGuessException &e) {
    response.status = GuessWordClientbound::status::DUP;
    response.trial -= 1;
    state.cdebug << playerTag(packet.player_id) << "Guessed duplicate word '"
                 << packet.guess << "'" << std::endl;
  } catch (InvalidTrialException &e) {
    response.status = GuessWordClientbound::status::INV;
    response.trial -= 1;
    state.cdebug << playerTag(packet.player_id)
                 << "Invalid trial sent. Trial sent: " << packet.trial
                 << ". Correct trial: " << (response.trial + 1) << std::endl;
  } catch (GameHasEndedException &e) {
    response.status = GuessWordClientbound::status::ERR;
    state.cdebug << playerTag(packet.player_id) << "Game has ended"
                 << std::endl;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Guess Word] Invalid packet" << std::endl;
    response.status = GuessWordClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr << "[Guess Word] There was an unhandled exception that prevented "
                 "the server from handling a word guess:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_quit_game(std::stringstream &buffer, Address &addr_from,
                      GameServerState &state) {
  QuitGameServerbound packet;
  QuitGameClientbound response;
  try {
    packet.deserialize(buffer);

    state.cdebug << playerTag(packet.player_id) << "Quitting game" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    if (game->isOnGoing()) {
      game->finishGame();
      response.status = QuitGameClientbound::status::OK;
      state.cdebug << playerTag(packet.player_id) << "Fulfilling quit request"
                   << std::endl;
    } else {
      response.status = QuitGameClientbound::status::NOK;
      state.cdebug << playerTag(packet.player_id) << "Game had already ended"
                   << std::endl;
    }

    game->saveToFile();

  } catch (NoGameFoundException &e) {
    response.status = QuitGameClientbound::status::NOK;
    state.cdebug << playerTag(packet.player_id) << "No game found" << std::endl;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Quit] Invalid packet" << std::endl;
    response.status = QuitGameClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr << "[Quit] There was an unhandled exception that prevented the "
                 "server from handling a quit game request:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_reveal_word(std::stringstream &buffer, Address &addr_from,
                        GameServerState &state) {
  RevealWordServerbound packet;
  RevealWordClientbound response;
  try {
    packet.deserialize(buffer);

    state.cdebug << playerTag(packet.player_id) << "Asked to reveal word"
                 << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    state.cdebug << playerTag(packet.player_id) << "Word is " << game->getWord()
                 << std::endl;

    response.word = game->getWord();
  } catch (NoGameFoundException &e) {
    // The protocol says we should not reply if there is not an on-going game
    state.cdebug << playerTag(packet.player_id) << "No game found" << std::endl;
    return;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Reveal] Invalid packet" << std::endl;
    // Propagate error to reply with "ERR", since there is no error code here
    throw;
  } catch (std::exception &e) {
    std::cerr << "[Reveal] There was an unhandled exception that prevented the "
                 "server from handling a word reveal:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_scoreboard(int connection_fd, GameServerState &state) {
  ScoreboardServerbound packet;
  ScoreboardClientbound response;
  try {
    packet.receive(connection_fd);

    state.cdebug << "[Scoreboard] Received request" << std::endl;

    auto scoreboard_str = state.scoreboard.toString();
    if (scoreboard_str.has_value()) {
      response.status = ScoreboardClientbound::status::OK;
      response.file_name = "scoreboard.txt";
      response.file_data = scoreboard_str.value();
      state.cdebug << "[Scoreboard] Sending back scoreboard as per request"
                   << std::endl;
    } else {
      response.status = ScoreboardClientbound::status::EMPTY;
      state.cdebug
          << "[Scoreboard] There are no won games, sending empty scoreboard"
          << std::endl;
    }
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Scoreboard] Invalid packet" << std::endl;
    // Propagate error to reply with "ERR", since there is no error code here
    throw;
  } catch (std::exception &e) {
    std::cerr << "[Scoreboard] There was an unhandled exception that prevented "
                 "the server from handling a scoreboard request:"
              << e.what() << std::endl;
    return;
  }

  response.send(connection_fd);
}

void handle_hint(int connection_fd, GameServerState &state) {
  HintServerbound packet;
  HintClientbound response;
  try {
    packet.receive(connection_fd);

    state.cdebug << playerTag(packet.player_id) << "Requested hint"
                 << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    if (!game->isOnGoing()) {
      response.status = HintClientbound::status::NOK;
      state.cdebug << playerTag(packet.player_id)
                   << "Fulfilling hint request: game had already ended."
                   << std::endl;
    } else if (!game->getHintFilePath().has_value() ||
               !std::filesystem::exists(game->getHintFilePath().value())) {
      response.status = HintClientbound::status::NOK;
      state.cdebug << playerTag(packet.player_id)
                   << "Current game doesn't have a hint to send." << std::endl;
    } else {
      response.status = HintClientbound::status::OK;
      response.file_path = game->getHintFilePath().value();
      response.file_name = game->getHintFileName();
      state.cdebug << playerTag(packet.player_id)
                   << "Fulfilling hint request: sending hint from file: "
                   << game->getHintFilePath().value() << std::endl;
    }
  } catch (NoGameFoundException &e) {
    response.status = HintClientbound::status::NOK;
    state.cdebug << playerTag(packet.player_id) << "Game not found"
                 << std::endl;
  } catch (InvalidPacketException &e) {
    response.status = HintClientbound::status::NOK;
    state.cdebug << "[Hint] Invalid packet" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "[Hint] There was an unhandled exception that prevented the "
                 "server from handling a hint request:"
              << e.what() << std::endl;
    return;
  }

  response.send(connection_fd);
}

void handle_state(int connection_fd, GameServerState &state) {
  StateServerbound packet;
  StateClientbound response;
  try {
    packet.receive(connection_fd);

    state.cdebug << playerTag(packet.player_id) << "Requested game state"
                 << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    if (game->isOnGoing()) {
      response.status = StateClientbound::status::ACT;
      state.cdebug << playerTag(packet.player_id)
                   << "Fulfilling state request: sending active game."
                   << std::endl;
    } else {
      response.status = StateClientbound::status::FIN;
      state.cdebug << playerTag(packet.player_id)
                   << "Fulfilling state request: sending last finished game."
                   << std::endl;
    }
    std::stringstream file_name;
    file_name << "state_" << std::setfill('0') << std::setw(PLAYER_ID_MAX_LEN)
              << game->getPlayerId() << ".txt";
    response.file_name = file_name.str();
    response.file_data = game->getStateString();
  } catch (NoGameFoundException &e) {
    response.status = StateClientbound::status::NOK;
    state.cdebug << playerTag(packet.player_id) << "Game not found"
                 << std::endl;
  } catch (InvalidPacketException &e) {
    response.status = StateClientbound::status::NOK;
    state.cdebug << "[State] Invalid packet" << std::endl;
  } catch (std::exception &e) {
    std::cerr
        << "[State] There was an unhandled exception that prevented the server "
           "from handling a state request:"
        << e.what() << std::endl;
    return;
  }

  response.send(connection_fd);
}
