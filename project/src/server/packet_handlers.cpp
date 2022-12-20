#include "packet_handlers.hpp"

#include <iomanip>
#include <iostream>

#include "common/protocol.hpp"

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state) {
  StartGameServerbound packet;
  ReplyStartGameClientbound response;

  try {
    packet.deserialize(buffer);
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Asked to start game" << std::endl;

    ServerGameSync game = state.createGame(packet.player_id);

    response.status = ReplyStartGameClientbound::OK;
    response.n_letters = game->getWordLen();
    response.max_errors = game->getMaxErrors();
  } catch (GameAlreadyStartedException &e) {
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Game already started" << std::endl;
    response.status = ReplyStartGameClientbound::NOK;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Start Game]Invalid packet received" << std::endl;
    response.status = ReplyStartGameClientbound::ERR;
  } catch (std::exception &e) {
    std::cerr << "[Start Game]There was an unhandled exception that prevented "
                 "the server "
                 "from starting a new game:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
  std::cout << "[Player " << std::setfill('0') << std::setw(PLAYER_ID_MAX_LEN)
            << packet.player_id << "] Game started with word '"
            << state.getGame(packet.player_id)->getWord() << "' and with "
            << state.getGame(packet.player_id)->getMaxErrors()
            << " errors allowed" << std::endl;
}

void handle_guess_letter(std::stringstream &buffer, Address &addr_from,
                         GameServerState &state) {
  GuessLetterServerbound packet;
  GuessLetterClientbound response;
  try {
    packet.deserialize(buffer);

    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Guessed letter '" << packet.guess << "'" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    response.trial = game->getCurrentTrial();
    auto found = game->guessLetter(packet.guess, packet.trial);
    // Must set trial again in case of replays
    response.trial = game->getCurrentTrial() - 1;

    if (game->hasLost()) {
      response.status = GuessLetterClientbound::status::OVR;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Game lost" << std::endl;
    } else if (found.size() == 0) {
      response.status = GuessLetterClientbound::status::NOK;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Wrong letter '" << packet.guess << "'" << std::endl;
    } else if (game->hasWon()) {
      response.status = GuessLetterClientbound::status::WIN;
      state.scoreboard.addGame(game.game);
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Won the game" << std::endl;
    } else {
      response.status = GuessLetterClientbound::status::OK;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Correct letter '" << packet.guess << "'. Trial "
                   << response.trial << ". Progress: " << game->getWord()
                   << std::endl;
    }
    response.pos = found;
  } catch (NoGameFoundException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] No game found" << std::endl;
  } catch (DuplicateLetterGuessException &e) {
    response.status = GuessLetterClientbound::status::DUP;
    response.trial -= 1;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Duplicate letter guess '" << packet.guess << "'"
                 << std::endl;
  } catch (InvalidTrialException &e) {
    response.status = GuessLetterClientbound::status::INV;
    response.trial -= 1;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Invalid trial " << packet.trial << std::endl;
  } catch (GameHasEndedException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Game has ended" << std::endl;
  } catch (InvalidPacketException &e) {
    response.status = GuessLetterClientbound::status::ERR;
    state.cdebug << "[Guess Letter] Invalid packet received" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "[Guess Letter] There was an unhandled exception that "
                 "prevented the server "
                 "from handling a letter guess:"
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

    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Guessed word '" << packet.guess << "'" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    response.trial = game->getCurrentTrial();
    bool correct = game->guessWord(packet.guess, packet.trial);
    // Must set trial again in case of replays
    response.trial = game->getCurrentTrial() - 1;

    if (game->hasLost()) {
      response.status = GuessWordClientbound::status::OVR;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Game lost" << std::endl;
    } else if (correct) {
      response.status = GuessWordClientbound::status::WIN;
      state.scoreboard.addGame(game.game);
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Guess was correct" << std::endl;
    } else {
      response.status = GuessWordClientbound::status::NOK;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Guess was wrong. Trial " << response.trial
                   << " Progress: " << game->getWord() << std::endl;
    }
  } catch (NoGameFoundException &e) {
    response.status = GuessWordClientbound::status::ERR;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] No game initialized" << std::endl;
  } catch (DuplicateWordGuessException &e) {
    response.status = GuessWordClientbound::status::DUP;
    response.trial -= 1;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Guessed duplicate word " << std::endl;
  } catch (InvalidTrialException &e) {
    response.status = GuessWordClientbound::status::INV;
    response.trial -= 1;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Invalid trial sent. Trial sent: " << packet.trial
                 << ". Correct trial: " << response.trial << std::endl;
  } catch (GameHasEndedException &e) {
    response.status = GuessWordClientbound::status::ERR;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Game has ended" << std::endl;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Guess Word] Invalid packet" << std::endl;
    response.status = GuessWordClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr << "[Guess Word] There was an unhandled exception that prevented "
                 "the server "
                 "from handling a word guess:"
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

    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Quitting game" << std::endl;

    ServerGameSync game = state.getGame(packet.player_id);

    if (game->isOnGoing()) {
      game->finishGame();
      response.status = QuitGameClientbound::status::OK;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Fulfilling quit request" << std::endl;
    } else {
      response.status = QuitGameClientbound::status::NOK;
      state.cdebug << "[Player " << std::setfill('0')
                   << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                   << "] Game has already ended" << std::endl;
    }
  } catch (NoGameFoundException &e) {
    response.status = QuitGameClientbound::status::NOK;
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] No game initialized" << std::endl;
  } catch (InvalidPacketException &e) {
    state.cdebug << "[Quit] Invalid packet" << std::endl;
    response.status = QuitGameClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr
        << "[Quit] There was an unhandled exception that prevented the server "
           "from handling a quit game request:"
        << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_reveal_word(std::stringstream &buffer, Address &addr_from,
                        GameServerState &state) {
  RevealWordServerbound packet;
  packet.deserialize(buffer);

  state.cdebug << "[Player " << std::setfill('0')
               << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
               << "] Revealing word" << std::endl;

  RevealWordClientbound response;
  try {
    ServerGameSync game = state.getGame(packet.player_id);

    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Word is " << game->getWord() << std::endl;

    response.word = game->getWord();
  } catch (NoGameFoundException &e) {
    // The protocol says we should not reply if there is not an on-going game
    return;
  } catch (std::exception &e) {
    std::cerr << "[Reveal] There was an unhandled exception that prevented the "
                 "server "
                 "from handling a word reveal."
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_scoreboard(int connection_fd, GameServerState &state) {
  ScoreboardServerbound packet;
  packet.receive(connection_fd);  // handle outside try and propagate error

  state.cdebug << "[Scoreboard] Received request." << std::endl;

  ScoreboardClientbound response;
  try {
    auto scoreboard_str = state.scoreboard.toString();
    if (scoreboard_str.has_value()) {
      response.status = ScoreboardClientbound::status::OK;
      state.cdebug << "[Scoreboard] Fulfilling request." << std::endl;
      response.file_data = scoreboard_str.value();
    } else {
      response.status = ScoreboardClientbound::status::EMPTY;
      state.cdebug
          << "[Scoreboard] There is no score, sending empty scoreboard."
          << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << "[Scoreboard] There was an unhandled exception that prevented "
                 "the server "
                 "from handling a scoreboard request:"
              << e.what() << std::endl;
    return;
  }

  response.send(connection_fd);
}

void handle_state(int connection_fd, GameServerState &state) {
  StateServerbound packet;
  packet.receive(connection_fd);

  state.cdebug << "[Player " << std::setfill('0')
               << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
               << "] Received state request." << std::endl;

  StateClientbound response;
  try {
    ServerGameSync game = state.getGame(packet.player_id);

    // TODO
    response.status = StateClientbound::status::ACT;
    response.file_name = "state.txt";
    response.file_data = game->getWord();
    state.cdebug << "[Player " << std::setfill('0')
                 << std::setw(PLAYER_ID_MAX_LEN) << packet.player_id
                 << "] Fulfilling state request." << std::endl;
  } catch (NoGameFoundException &e) {
    // TODO
  } catch (std::exception &e) {
    std::cerr
        << "[State] There was an unhandled exception that prevented the server "
           "from handling a state request:"
        << e.what() << std::endl;
    return;
  }

  response.send(connection_fd);
}
