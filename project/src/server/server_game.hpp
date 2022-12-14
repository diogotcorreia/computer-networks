#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include <stdexcept>
#include <vector>

#include "common/game.hpp"

class ServerGame : public Game {
 private:
  const char *word;
  uint32_t lettersRemaining;
  std::vector<char> plays;

  std::vector<uint32_t> getIndexesOfLetter(char letter);

 public:
  ServerGame(uint32_t playerId);
  ~ServerGame();
  std::vector<uint32_t> guessLetter(char letter, uint32_t trial);
  bool guessWord(char *word, uint32_t trial);
  bool hasLost();
  bool hasWon();
  bool hasStarted();
};

/** Exceptions **/

class DuplicateLetterGuessException : public std::runtime_error {
 public:
  DuplicateLetterGuessException()
      : std::runtime_error(
            "That letter has already been guessed in this game before.") {}
};

class InvalidTrialException : public std::runtime_error {
 public:
  InvalidTrialException()
      : std::runtime_error(
            "The trial number does not match what was expected.") {}
};

class GameHasEndedException : public std::runtime_error {
 public:
  GameHasEndedException()
      : std::runtime_error(
            "Game has already ended, therefore no actions can take place.") {}
};

#endif
