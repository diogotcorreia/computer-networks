#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <cstdint>
#include <fstream>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>

#include "server_game.hpp"

class ScoreboardEntry {
 public:
  uint32_t score;
  uint32_t playerId;
  std::string word;
  uint32_t goodTrials;
  uint32_t totalTrials;

  ScoreboardEntry(ServerGame& game);
  ScoreboardEntry(std::ifstream& file);
  void serializeEntry(std::ofstream& file);

  bool operator<(const ScoreboardEntry& r) const {  // Comparison function
    return score < r.score || (score == r.score && totalTrials > r.totalTrials);
  }
};

class Scoreboard {
  std::shared_mutex rwlock;

 public:
  std::multiset<ScoreboardEntry> entries;

  void addGame(ServerGame& game);
  void saveToFile();
  void loadFromFile();
  std::optional<std::string> toString();
};

#endif
