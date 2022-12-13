#include "client_game.hpp"

#include <cstring>

ClientGame::ClientGame(int playerId, int wordLen, int maxErrors) {
  this->playerId = playerId;
  this->wordLen = wordLen;
  this->maxErrors = maxErrors;
  this->wordProgress = new char[wordLen];
  memset(this->wordProgress, '_', wordLen);
}

ClientGame::~ClientGame() {
  delete[] this->wordProgress;
}

void ClientGame::finishGame() {
  this->isActive = false;
}
