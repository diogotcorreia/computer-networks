#include "game.hpp"

#include <cstring>

#include "protocol.hpp"

uint32_t Game::getWordLen() {
  return wordLen;
}

uint32_t Game::getMaxErrors() {
  return maxErrors;
}

uint32_t Game::getPlayerId() {
  return playerId;
}

uint32_t Game::getCurrentTrial() {
  return currentTrial;
}

char* Game::getWordProgress() {
  return wordProgress;
}

uint32_t Game::getNumErrors() {
  return numErrors;
}

bool Game::getIsActive() {
  return isActive;
}

void Game::updateWordChar(uint32_t index, char letter) {
  if (index >= wordLen) {
    return;
  }
  wordProgress[index] = letter;
}

void Game::updateCurrentTrial(uint32_t num) {
  this->currentTrial = num;
}

void Game::updateNumErrors() {
  this->numErrors++;
}
