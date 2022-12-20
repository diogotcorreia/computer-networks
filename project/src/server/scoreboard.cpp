#include "scoreboard.hpp"

#include <iomanip>
#include <sstream>

ScoreboardEntry::ScoreboardEntry(ServerGame &game) {
  score = game.getScore();
  playerId = game.getPlayerId();
  word = game.getWord();
  goodTrials = game.getGoodTrials();
  totalTrials = game.getCurrentTrial() - 1;
}

void Scoreboard::addGame(ServerGame &game) {
  if (!game.hasWon()) {
    return;
  }

  std::unique_lock scoreboard_lock(rwlock);

  ScoreboardEntry entry(game);

  if (entries.size() >= 10 && entry < *entries.begin()) {
    return;
  }

  entries.insert(entry);
  if (entries.size() > 10) {
    entries.erase(entries.begin());
  }
}

std::optional<std::string> Scoreboard::toString() {
  std::shared_lock scoreboard_lock(rwlock);

  if (entries.size() == 0) {
    return std::nullopt;
  }
  std::stringstream file;
  int i = 0;

  file << std::endl
       << "-------------------------------- TOP 10 SCORES "
          "--------------------------------"
       << std::endl;
  file << std::endl;
  file << "    SCORE PLAYER     WORD                             GOOD TRIALS  "
          "TOTAL TRIALS"
       << std::endl
       << std::endl;
  for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
    i++;
    file << std::right << std::setfill(' ') << std::setw(2) << i << " - "
         << std::setfill(' ') << std::setw(3) << it->score << "  "
         << std::setfill('0') << std::setw(6) << it->playerId << "  "
         << std::setfill(' ') << std::left << std::setw(38) << it->word << "  "
         << std::setfill(' ') << std::setw(2) << it->goodTrials
         << "            " << std::setfill(' ') << std::setw(2)
         << it->totalTrials << std::endl;
  }
  file << std::endl;
  file << std::endl;

  return file.str();
}
