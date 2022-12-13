#ifndef GAME_H
#define GAME_H

#include <cstddef>
#include <cstdint>

class Game {
 protected:
  uint32_t numErrors = 0;
  uint32_t currentTrial = 1;
  bool isActive = true;
  uint32_t playerId;
  uint32_t wordLen;
  uint32_t maxErrors;
  char *wordProgress;

 public:
  uint32_t getPlayerId();
  uint32_t getWordLen();
  uint32_t getMaxErrors();
  uint32_t getCurrentTrial();
  char *getWordProgress();
  uint32_t getNumErrors();
  void updateWordChar(uint32_t index, char letter);
  void updateCurrentTrial(uint32_t num);
  void updateNumErrors();
  bool getIsActive();
};

#endif
