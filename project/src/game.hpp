#ifndef GAME_H
#define GAME_H

#include <cstddef>

class Game {
 protected:
  int playerId;
  int wordLen;
  int maxErrors;
  int currentTrial;
  char *wordProgress;
  int numErrors;

 public:
  int getPlayerId();
  size_t getWordLen();
  int getMaxErrors();
  int getCurrentTrial();
  char *getWordProgress();
  int getNumErrors();
  void updateWordChar(int index, char letter);
  void updateCurrentTrial();
  void updateNumErrors();
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
};

class ClientGame : public Game {
 public:
  ClientGame(int playerId, int wordLen, int maxErrors);
  ~ClientGame();
};

#endif
