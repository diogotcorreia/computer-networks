#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "common/game.hpp"

class ClientGame : public Game {
  char *wordProgress;

 public:
  ClientGame(uint32_t playerId, uint32_t wordLen, uint32_t maxErrors);
  ~ClientGame();
  char *getWordProgress();
  void updateWordChar(uint32_t index, char letter);
  void increaseTrial();
  void updateNumErrors();
};

#endif
