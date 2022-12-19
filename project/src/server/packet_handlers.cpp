#include "packet_handlers.hpp"

#include <iostream>

#include "common/protocol.hpp"

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state) {
  StartGameServerbound packet;
  packet.deserialize(buffer);

  state.cdebug << "Received request to start new game from player '"
               << packet.player_id << "'" << std::endl;

  ReplyStartGameClientbound response;
  try {
    auto game = state.createGame(packet.player_id);
    response.success = true;
    response.n_letters = game.getWordLen();
    response.max_errors = game.getMaxErrors();
  } catch (GameAlreadyStartedException &e) {
    response.success = false;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
                 "from starting a new game:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_guess_letter(std::stringstream &buffer, Address &addr_from,
                         GameServerState &state) {
  GuessLetterServerbound packet;
  packet.deserialize(buffer);

  state.cdebug << "Received request to guess letter '" << packet.guess
               << "' from player '" << packet.player_id << "'" << std::endl;

  GuessLetterClientbound response;
  try {
    ServerGame &game = state.getGame(packet.player_id);
    response.trial = game.getCurrentTrial();
    auto found = game.guessLetter(packet.guess, packet.trial);

    if (game.hasLost()) {
      response.status = GuessLetterClientbound::status::OVR;
      state.addtoScoreboard(&game);
    } else if (found.size() == 0) {
      response.status = GuessLetterClientbound::status::NOK;
    } else if (game.hasWon()) {
      response.status = GuessLetterClientbound::status::WIN;
      state.addtoScoreboard(&game);
    } else {
      response.status = GuessLetterClientbound::status::OK;
    }
    response.pos = found;
  } catch (NoGameFoundException &e) {
    response.status = GuessLetterClientbound::status::ERR;
  } catch (DuplicateLetterGuessException &e) {
    response.status = GuessLetterClientbound::status::DUP;
    response.trial -= 1;
  } catch (InvalidTrialException &e) {
    response.status = GuessLetterClientbound::status::INV;
  } catch (GameHasEndedException &e) {
    response.status = GuessLetterClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
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
  packet.deserialize(buffer);

  state.cdebug << "Received request to guess word '" << packet.guess
               << "' from player '" << packet.player_id << "'" << std::endl;

  GuessWordClientbound response;
  try {
    ServerGame &game = state.getGame(packet.player_id);
    response.trial = game.getCurrentTrial();
    bool correct = game.guessWord(packet.guess, packet.trial);

    if (game.hasLost()) {
      response.status = GuessWordClientbound::status::OVR;
      state.addtoScoreboard(&game);
    } else if (correct) {
      response.status = GuessWordClientbound::status::WIN;
      state.addtoScoreboard(&game);
    } else {
      response.status = GuessWordClientbound::status::NOK;
    }
  } catch (NoGameFoundException &e) {
    response.status = GuessWordClientbound::status::ERR;
  } catch (InvalidTrialException &e) {
    response.status = GuessWordClientbound::status::INV;
  } catch (GameHasEndedException &e) {
    response.status = GuessWordClientbound::status::ERR;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
                 "from handling a word guess:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}

void handle_scoreboard(Address &addr_from, GameServerState &state) {
  ScoreboardServerbound packet;
  packet.receive(addr_from.socket);

  state.cdebug << "Received request for scoreboard" << std::endl;

  ScoreboardClientbound response;
  try {
    response.file_data = state.getScoreboard();
    response.status = ScoreboardClientbound::status::OK;
  } catch (NoGameFoundException &e) {
    response.status = ScoreboardClientbound::status::EMPTY;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
                 "from handling a scoreboard request:"
              << e.what() << std::endl;
    return;
  }

  response.send(addr_from.socket);
}

void handle_quit_game(std::stringstream &buffer, Address &addr_from,
                      GameServerState &state) {
  QuitGameServerbound packet;
  packet.deserialize(buffer);

  state.cdebug << "Received request to quit game from player '"
               << packet.player_id << "'" << std::endl;

  QuitGameClientbound response;
  try {
    ServerGame &game = state.getGame(packet.player_id);
    if (game.isOnGoing()) {
      game.finishGame();
      response.success = true;
    } else {
      response.success = false;
    }
  } catch (NoGameFoundException &e) {
    response.success = false;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
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

  state.cdebug << "Received request to reveal word from player '"
               << packet.player_id << "'" << std::endl;
  state.cdebug << "Word is: " << state.getGame(packet.player_id).getWord()
               << std::endl;

  RevealWordClientbound response;
  try {
    ServerGame &game = state.getGame(packet.player_id);
    response.word = game.getWord();
  } catch (NoGameFoundException &e) {
    // The protocol says we should not reply if there is not an on-going game
    return;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
                 "from handling a word reveal:"
              << e.what() << std::endl;
    return;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}
