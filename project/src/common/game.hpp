#ifndef GAME_H
#define GAME_H

#include <cstddef>
#include <cstdint>

class Game {
 protected:
  uint32_t numErrors = 0;
  uint32_t currentTrial = 1;
  bool onGoing = true;
  uint32_t playerId;
  uint32_t wordLen;
  uint32_t maxErrors;

 public:
  uint32_t getPlayerId();
  uint32_t getWordLen();
  uint32_t getMaxErrors();
  uint32_t getCurrentTrial();
  uint32_t getNumErrors();
  bool isOnGoing();
  void finishGame();  // setOnGoing(false)
};

#endif
