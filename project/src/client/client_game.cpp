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

void ClientGame::finishGame() {
  this->isActive = false;
}
