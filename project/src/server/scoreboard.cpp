#include "scoreboard.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "common/constants.hpp"
#include "stream_utils.hpp"

ScoreboardEntry::ScoreboardEntry(ServerGame& game) {
  score = game.getScore();
  playerId = game.getPlayerId();
  word = game.getWord();
  goodTrials = game.getGoodTrials();
  totalTrials = game.getCurrentTrial() - 1;
}

ScoreboardEntry::ScoreboardEntry(std::ifstream& file) {
  score = read_uint32_t(file);
  playerId = read_uint32_t(file);
  word = read_string(file);
  goodTrials = read_uint32_t(file);
  totalTrials = read_uint32_t(file);
}

void ScoreboardEntry::serializeEntry(std::ofstream& file) {
  write_uint32_t(file, score);
  write_uint32_t(file, playerId);
  write_string(file, word);
  write_uint32_t(file, goodTrials);
  write_uint32_t(file, totalTrials);
}

void Scoreboard::addGame(ServerGame& game) {
  if (!game.hasWon()) {
    return;
  }

  std::unique_lock scoreboard_lock(rwlock);

  ScoreboardEntry entry(game);

  if (entries.size() >= SCOREBOARD_MAX_ENTRIES && entry < *entries.begin()) {
    return;
  }

  entries.insert(entry);
  while (entries.size() > SCOREBOARD_MAX_ENTRIES) {
    entries.erase(entries.begin());
  }

  saveToFile();
}

void Scoreboard::saveToFile() {
  try {
    std::filesystem::path folder(GAMEDATA_FOLDER_NAME);
    std::filesystem::create_directory(folder);

    std::filesystem::path file_sb(folder);
    file_sb.append(SCOREBOARD_FILE_NAME);

    std::ofstream sb_stream(file_sb, std::ios::out | std::ios::binary);

    write_uint32_t(sb_stream, (uint32_t)entries.size());
    for (auto entry : entries) {
      entry.serializeEntry(sb_stream);
    }
  } catch (std::exception& e) {
    std::cerr << "[ERROR] Failed to save scoreboard to file: " << e.what()
              << std::endl;
  } catch (...) {
    std::cerr << "[ERROR] Failed to save scoreboard to file: unknown"
              << std::endl;
  }
}

void Scoreboard::loadFromFile() {
  try {
    std::filesystem::path file_sb(GAMEDATA_FOLDER_NAME);
    file_sb.append(SCOREBOARD_FILE_NAME);

    if (!std::filesystem::exists(file_sb)) {
      std::cout << "There is no stored scoreboard data from previous sessions, "
                   "starting with an empty scoreboard..."
                << std::endl;
      return;
    }

    std::ifstream sb_stream(file_sb, std::ios::in | std::ios::binary);

    uint32_t size = read_uint32_t(sb_stream);
    for (uint32_t i = 0; i < size; ++i) {
      entries.insert(ScoreboardEntry(sb_stream));
    }

    if (!sb_stream.good()) {
      throw std::runtime_error("file content ended too early");
    }

    while (entries.size() > SCOREBOARD_MAX_ENTRIES) {
      entries.erase(entries.begin());
    }

    std::cout << "Loaded " << size << " entries from stored scoreboard!"
              << std::endl;
  } catch (std::exception& e) {
    std::cerr << "[ERROR] Failed to load scoreboard from file: " << e.what()
              << std::endl;
    entries.clear();
  } catch (...) {
    std::cerr << "[ERROR] Failed to load scoreboard from file: unknown"
              << std::endl;
    entries.clear();
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
         << std::setfill('0') << std::setw(PLAYER_ID_MAX_LEN) << it->playerId
         << "  " << std::setfill(' ') << std::left << std::setw(38) << it->word
         << "  " << std::setfill(' ') << std::setw(2) << it->goodTrials
         << "            " << std::setfill(' ') << std::setw(2)
         << it->totalTrials << std::endl;
  }
  file << std::endl;
  file << std::endl;

  return file.str();
}
