#include "server_game.hpp"

#include <cstring>
#include <stdexcept>

ServerGame::ServerGame(uint32_t __playerId) {
  this->playerId = __playerId;
  // TODO: Get word from file
  word = "test";
  size_t word_len = strlen(word);
  // TODO: Get max errors from one liner
  if (word_len <= 6) {
    this->maxErrors = 7;
  } else if (word_len <= 10) {
    this->maxErrors = 8;
  } else if (word_len <= 30) {
    this->maxErrors = 9;
  } else {
    throw std::runtime_error("word is more than 30 characters");
  }
  this->wordLen = (uint32_t)word_len;
  this->lettersRemaining = wordLen;
}

ServerGame::~ServerGame() {
  // TODO
}

// indexes start at 1
std::vector<uint32_t> ServerGame::getIndexesOfLetter(char letter) {
  std::vector<uint32_t> found_indexes;
  for (uint32_t i = 0; i < wordLen; i++) {
    if (word[i] == letter) {
      found_indexes.push_back(i + 1);
    }
  }
  return found_indexes;
}

std::vector<uint32_t> ServerGame::guessLetter(char letter, uint32_t trial) {
  if (!isOnGoing()) {
    throw GameHasEndedException();
  }

  if (trial != 0 && trial == plays.size()) {
    // replaying of last guess
    if (letter != plays.at(trial - 1)) {
      throw InvalidTrialException();
    }

    return getIndexesOfLetter(letter);
  }

  if (trial != plays.size() + 1) {
    throw InvalidTrialException();
  }

  for (auto it = plays.begin(); it != plays.end(); ++it) {
    // check if it is duplicate play
    if (letter == *it) {
      throw DuplicateLetterGuessException();
    }
  }

  plays.push_back(letter);
  auto found_indexes = getIndexesOfLetter(letter);
  if (found_indexes.size() == 0) {
    numErrors++;
  }
  currentTrial++;
  lettersRemaining -= (uint32_t)found_indexes.size();
  return found_indexes;
}

bool ServerGame::guessWord(char* word_guess, uint32_t trial) {
  (void)trial;
  // TODO
  if (strcmp(this->word, word_guess) == 0) {
    return true;
  }
  this->numErrors++;
  this->currentTrial++;
  return false;
}

bool ServerGame::hasLost() {
  return numErrors >= maxErrors;
}

bool ServerGame::hasWon() {
  return lettersRemaining == 0;
}

bool ServerGame::hasStarted() {
  return this->getCurrentTrial() > 1;
}
