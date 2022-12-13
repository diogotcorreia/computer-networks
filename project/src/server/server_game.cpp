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
  this->wordProgress = new char[wordLen];
  memset(this->wordProgress, '_', wordLen);
}

ServerGame::~ServerGame() {
  // TODO
}

bool ServerGame::play(char letter) {
  bool found = false;
  for (uint32_t i = 0; i < wordLen; i++) {
    if (word[i] == letter) {
      wordProgress[i] = letter;
      found = true;
    }
  }
  if (!found) {
    numErrors++;
  }
  currentTrial++;
  return found;
}

bool ServerGame::guess(char* word_guess) {
  if (strcmp(this->word, word_guess) == 0) {
    return true;
  }
  this->numErrors++;
  this->currentTrial++;
  return false;
}

bool ServerGame::isOver() {
  if (numErrors >= maxErrors) {
    return true;
  }
  for (uint32_t i = 0; i < wordLen; i++) {
    if (wordProgress[i] == '_') {
      return false;
    }
  }
  setActive(false);
  return true;
}

void ServerGame::setActive(bool active) {
  this->isActive = active;
}

bool ServerGame::hasStarted() {
  return this->getCurrentTrial() > 1;
}
