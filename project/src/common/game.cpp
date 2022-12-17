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

uint32_t Game::getGoodTrials() {
  return currentTrial - numErrors;
}

uint32_t Game::getNumErrors() {
  return numErrors;
}

bool Game::isOnGoing() {
  return onGoing;
}

void Game::finishGame() {
  onGoing = false;
}
