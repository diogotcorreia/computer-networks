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
    } else if (found.size() == 0) {
      response.status = GuessLetterClientbound::status::NOK;
    } else if (game.hasWon()) {
      response.status = GuessLetterClientbound::status::WIN;
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
    } else if (correct) {
      response.status = GuessWordClientbound::status::WIN;
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
