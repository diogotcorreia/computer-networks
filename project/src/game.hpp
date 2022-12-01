#ifndef GAME_H
#define GAME_H

#include <cstddef>

class Game {
 protected:
  int numErrors = 0;
  int currentTrial = 1;
  bool isActive = true;
  int playerId;
  int wordLen;
  int maxErrors;
  char *wordProgress;

 public:
  int getPlayerId();
  size_t getWordLen();
  int getMaxErrors();
  int getCurrentTrial();
  char *getWordProgress();
  int getNumErrors();
  void updateWordChar(int index, char letter);
  void updateCurrentTrial(int num);
  void updateNumErrors();
  bool getIsActive();
};

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
};

class ClientGame : public Game {
 public:
  ClientGame(int playerId, int wordLen, int maxErrors);
  ~ClientGame();
  void finishGame();
};

#endif
