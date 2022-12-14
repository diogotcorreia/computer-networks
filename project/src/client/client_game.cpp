#include "client_game.hpp"

#include <cstring>

ClientGame::ClientGame(uint32_t __playerId, uint32_t __wordLen,
                       uint32_t __maxErrors) {
  this->playerId = __playerId;
  this->wordLen = __wordLen;
  this->maxErrors = __maxErrors;
  this->wordProgress = new char[wordLen];
  memset(this->wordProgress, '_', wordLen);
}

ClientGame::~ClientGame() {
  delete[] this->wordProgress;
}

char* ClientGame::getWordProgress() {
  return wordProgress;
}

void ClientGame::updateWordChar(uint32_t index, char letter) {
  if (index >= wordLen) {
    return;
  }
  wordProgress[index] = letter;
}

void ClientGame::increaseTrial() {
  ++this->currentTrial;
}

void ClientGame::updateNumErrors() {
  this->numErrors++;
}
