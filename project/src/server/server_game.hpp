#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "common/game.hpp"

class ServerGame : public Game {
 private:
  const char *word;

 public:
  ServerGame(int playerId);
  ~ServerGame();
  bool play(char letter);
  bool guess(char *word);
  bool isOver();
  void setActive(bool active);
  bool hasStarted();
};

#endif
