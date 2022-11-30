#include "game.hpp"

#include <cstring>

#include "packet.hpp"

size_t Game::getWordLen() {
  return wordLen;
}
int Game::getMaxErrors() {
  return maxErrors;
}

int Game::getPlayerId() {
  return playerId;
}

int Game::getCurrentTrial() {
  return currentTrial;
}

char* Game::getWordProgress() {
  return wordProgress;
}

int Game::getNumErrors() {
  return numErrors;
}

void Game::updateWordChar(int index, char letter) {
  if (index > wordLen) {
    return;
  }
  wordProgress[index - 1] = letter;
}

void Game::updateCurrentTrial() {
  this->currentTrial++;
}

void Game::updateNumErrors() {
  this->numErrors++;
}

ClientGame::ClientGame(int playerId, int wordLen, int maxErrors) {
  this->playerId = playerId;
  this->wordLen = wordLen;
  this->maxErrors = maxErrors;
  this->currentTrial = 1;
  this->wordProgress = new char[wordLen];
  memset(this->wordProgress, '_', wordLen);
  this->numErrors = 0;
}

ClientGame::~ClientGame() {
  delete[] this->wordProgress;
}

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
  this->currentTrial = 0;
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
  return true;
}
