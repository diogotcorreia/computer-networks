#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "common/game.hpp"

class ClientGame : public Game {
 public:
  ClientGame(int playerId, int wordLen, int maxErrors);
  ~ClientGame();
  void finishGame();
};

#endif
