#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include <mutex>
#include <stdexcept>
#include <vector>

#include "common/game.hpp"

class ServerGame : public Game {
 private:
  std::string word;
  std::string image_path;
  uint32_t lettersRemaining;
  std::vector<char> plays;
  std::vector<std::string> word_guesses;

  std::vector<uint32_t> getIndexesOfLetter(char letter);

 public:
  std::mutex lock;

  ServerGame(uint32_t playerId, std::string word, std::string image_path);
  ~ServerGame();
  std::vector<uint32_t> guessLetter(char letter, uint32_t trial);
  bool guessWord(std::string& word, uint32_t trial);
  bool hasLost();
  bool hasWon();
  bool hasStarted();
  std::string getWord();
  std::string getImagePath();
};

class ServerGameSync {
 private:
  std::unique_lock<std::mutex> slock;
  ServerGame& game;

 public:
  ServerGameSync(ServerGame& __game) : slock{__game.lock}, game{__game} {};

  ServerGame& operator*() {
    return game;
  }
  ServerGame* operator->() {
    return &game;
  }
};

/** Exceptions **/

class DuplicateLetterGuessException : public std::runtime_error {
 public:
  DuplicateLetterGuessException()
      : std::runtime_error(
            "That letter has already been guessed in this game before.") {}
};

class DuplicateWordGuessException : public std::runtime_error {
 public:
  DuplicateWordGuessException()
      : std::runtime_error(
            "That word has already been guessed in this game before.") {}
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
