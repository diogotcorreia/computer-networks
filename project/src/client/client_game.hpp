#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "common/game.hpp"

class ClientGame : public Game {
 public:
  ClientGame(uint32_t playerId, uint32_t wordLen, uint32_t maxErrors);
  ~ClientGame();
  void finishGame();
};

#endif
