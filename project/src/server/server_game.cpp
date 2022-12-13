#include "server_game.hpp"

#include <cstring>

ServerGame::ServerGame(int playerId) {
  this->playerId = playerId;
  // TODO: Get word from file
  word = "test";
  this->wordLen = strlen(word);
  // TODO: Get max errors from one liner
  if (wordLen <= 6) {
    this->maxErrors = 7;
  } else if (wordLen <= 10) {
    this->maxErrors = 8;
  } else if (wordLen <= 30) {
    this->maxErrors = 9;
  } else {
    // TODO handle exception
  }
  this->wordProgress = new char[wordLen];
  memset(this->wordProgress, '_', wordLen);
}

ServerGame::~ServerGame() {
  // TODO
}

bool ServerGame::play(char letter) {
  bool found = false;
  for (int i = 0; i < wordLen; i++) {
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

bool ServerGame::guess(char* word) {
  if (strcmp(this->word, word) == 0) {
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
  for (int i = 0; i < wordLen; i++) {
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
