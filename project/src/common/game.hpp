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

#endif
