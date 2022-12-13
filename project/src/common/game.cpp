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

bool Game::getIsActive() {
  return isActive;
}

void Game::updateWordChar(int index, char letter) {
  if (index >= wordLen || index < 0) {
    return;
  }
  wordProgress[index] = letter;
}

void Game::updateCurrentTrial(int num) {
  this->currentTrial = num;
}

void Game::updateNumErrors() {
  this->numErrors++;
}
