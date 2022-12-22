#include "server_game.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "common/constants.hpp"
#include "stream_utils.hpp"

ServerGame::ServerGame(uint32_t __playerId, std::string __word,
                       std::optional<std::filesystem::path> __hint_path)
    : word{__word}, hint_path{__hint_path} {
  this->playerId = __playerId;
  size_t word_len = word.size();
  // TODO: Get max errors from one liner
  if (word_len <= 6) {
    this->maxErrors = 7;
  } else if (word_len <= 10) {
    this->maxErrors = 8;
  } else if (word_len <= 30) {
    this->maxErrors = 9;
  } else {
    throw std::runtime_error("word is more than 30 characters");
  }
  this->wordLen = (uint32_t)word_len;
  this->lettersRemaining = wordLen;
}

ServerGame::~ServerGame() {
  // TODO
}

// indexes start at 1
std::vector<uint32_t> ServerGame::getIndexesOfLetter(char letter) {
  std::vector<uint32_t> found_indexes;
  for (uint32_t i = 0; i < wordLen; i++) {
    if (word[i] == letter) {
      found_indexes.push_back(i + 1);
    }
  }
  return found_indexes;
}

std::vector<uint32_t> ServerGame::guessLetter(char letter, uint32_t trial) {
  if (!isOnGoing()) {
    throw GameHasEndedException();
  }

  if (trial != 0 && trial == plays.size()) {
    // replaying of last guess
    if (letter != plays.at(trial - 1)) {
      throw InvalidTrialException();
    }

    return getIndexesOfLetter(letter);
  }

  if (trial != plays.size() + 1) {
    throw InvalidTrialException();
  }

  for (auto it = plays.begin(); it != plays.end(); ++it) {
    // check if it is duplicate play
    if (letter == *it) {
      throw DuplicateLetterGuessException();
    }
  }

  plays.push_back(letter);
  auto found_indexes = getIndexesOfLetter(letter);
  if (found_indexes.size() == 0) {
    numErrors++;
  }
  currentTrial++;
  lettersRemaining -= (uint32_t)found_indexes.size();
  if (hasWon() || hasLost()) {
    onGoing = false;
  }
  return found_indexes;
}

bool ServerGame::guessWord(std::string& word_guess, uint32_t trial) {
  if (!isOnGoing()) {
    throw GameHasEndedException();
  }

  if (word_guesses.size() > 0 && trial == plays.size() &&
      *(plays.end() - 1) == 0) {
    // replaying of last guess
    if (*(word_guesses.end() - 1) != word_guess) {
      throw InvalidTrialException();
    }

    return word == word_guess;
  }

  if (trial != plays.size() + 1) {
    throw InvalidTrialException();
  }

  for (auto it = word_guesses.begin(); it != word_guesses.end(); ++it) {
    // check if it is duplicate play
    if (word_guess == *it) {
      throw DuplicateWordGuessException();
    }
  }

  plays.push_back(0);
  word_guesses.push_back(word_guess);
  currentTrial++;
  if (word == word_guess) {
    lettersRemaining = 0;
    onGoing = false;
    return true;
  }
  numErrors++;
  if (hasLost()) {
    onGoing = false;
  }
  return false;
}

bool ServerGame::hasLost() {
  return numErrors >= maxErrors;
}

bool ServerGame::hasWon() {
  return lettersRemaining == 0;
}

bool ServerGame::hasStarted() {
  return this->getCurrentTrial() > 1;
}

uint32_t ServerGame::getScore() {
  if (!hasWon()) {
    return 0;
  }
  return (uint32_t)(getGoodTrials() * 100 / (currentTrial - 1));
}

std::string ServerGame::getStateString() {
  std::stringstream state;
  if (isOnGoing()) {
    state << "     Active game found for player " << std::setfill('0')
          << std::setw(PLAYER_ID_MAX_LEN) << playerId << std::endl;
  } else {
    state << "     Last finalized game for player " << std::setfill('0')
          << std::setw(PLAYER_ID_MAX_LEN) << playerId << std::endl;
    state << "     Word: " << word << "; Hint file: " << getHintFileName()
          << std::endl;
  }

  if (plays.size() == 0) {
    state << "     Game started - no transactions found" << std::endl;
  } else {
    state << "     --- Transactions found: " << plays.size() << " ---"
          << std::endl;
  }

  auto next_word = word_guesses.begin();
  for (char play : plays) {
    if (play == 0) {
      state << "     Word guess: " << *next_word << std::endl;
      ++next_word;
    } else {
      state << "     Letter trial: " << play << " - ";
      if (word.find(play) != std::string::npos) {
        state << "TRUE";
      } else {
        state << "FALSE";
      }
      state << std::endl;
    }
  }

  if (isOnGoing()) {
    state << "     Solved so far: " << getWordProgress();
  } else {
    state << "     Termination: ";
    if (hasWon()) {
      state << "WIN";
    } else if (hasLost()) {
      state << "FAIL";
    } else {
      state << "QUIT";
    }
  }
  state << std::endl;

  return state.str();
}

std::string ServerGame::getWord() {
  return word;
}

std::string ServerGame::getWordProgress() {
  std::string result;
  for (char c : word) {
    if (std::find(plays.begin(), plays.end(), c) != plays.end()) {
      result += c;
    } else {
      result += '-';
    }
  }
  return result;
}

std::optional<std::filesystem::path> ServerGame::getHintFilePath() {
  return hint_path;
}

std::string ServerGame::getHintFileName() {
  if (hint_path.has_value()) {
    return std::string(hint_path.value().filename());
  }
  return std::string();
}

void ServerGame::saveToFile() {
  try {
    std::filesystem::path folder(GAMEDATA_FOLDER_NAME);
    folder.append(GAMES_FOLDER_NAME);
    std::filesystem::create_directories(folder);

    std::stringstream file_name;
    file_name << std::setfill('0') << std::setw(6) << playerId << ".dat";
    std::filesystem::path file_game(folder);
    file_game.append(file_name.str());

    std::ofstream game_stream(file_game, std::ios::out | std::ios::binary);

    write_uint32_t(game_stream, playerId);
    write_string(game_stream, word);
    write_bool(game_stream, hint_path.has_value());
    if (hint_path.has_value()) {
      std::string hint_path_str = hint_path.value().string();
      write_string(game_stream, hint_path_str);
    }
    write_uint32_t(game_stream, numErrors);
    write_uint32_t(game_stream, currentTrial);
    write_bool(game_stream, onGoing);
    write_uint32_t(game_stream, maxErrors);
    write_uint32_t(game_stream, (uint32_t)plays.size());
    for (char c : plays) {
      game_stream.put(c);
    }
    write_uint32_t(game_stream, (uint32_t)word_guesses.size());
    for (std::string& guess : word_guesses) {
      write_string(game_stream, guess);
    }

    // Derived:
    // lettersRemaining
    // wordLen
  } catch (std::exception& e) {
    std::cerr << "[ERROR] Failed to save game (player " << playerId
              << ") to file: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "[ERROR] Failed to save game (player " << playerId
              << ") to file: unknown" << std::endl;
  }
}

bool ServerGame::loadFromFile(bool on_going_only) {
  try {
    std::filesystem::path file_game(GAMEDATA_FOLDER_NAME);
    file_game.append(GAMES_FOLDER_NAME);

    std::stringstream file_name;
    file_name << std::setfill('0') << std::setw(6) << playerId << ".dat";
    file_game.append(file_name.str());

    if (!std::filesystem::exists(file_game)) {
      // File does not exist, avoid loading
      return false;
    }

    std::ifstream game_stream(file_game, std::ios::in | std::ios::binary);

    uint32_t new_player_id = read_uint32_t(game_stream);

    if (new_player_id != playerId || !game_stream.good()) {
      throw std::runtime_error(
          "player ID of current game does not match the one in the saved file");
    }

    std::string new_word = read_string(game_stream);
    bool has_hint = read_bool(game_stream);
    std::string hint_path_str;
    if (has_hint) {
      hint_path_str = read_string(game_stream);
    }
    uint32_t new_num_errors = read_uint32_t(game_stream);
    uint32_t new_current_trial = read_uint32_t(game_stream);
    bool new_on_going = read_bool(game_stream);
    uint32_t new_max_errors = read_uint32_t(game_stream);
    uint32_t plays_size = read_uint32_t(game_stream);
    std::vector<char> new_plays;
    for (uint32_t i = 0; i < plays_size; ++i) {
      new_plays.push_back((char)game_stream.get());
    }
    uint32_t word_guesses_size = read_uint32_t(game_stream);
    std::vector<std::string> new_word_guesses;
    for (uint32_t i = 0; i < word_guesses_size; ++i) {
      new_word_guesses.push_back(read_string(game_stream));
    }

    if (!game_stream.good()) {
      throw std::runtime_error("file content ended too early");
    }

    if (on_going_only && !new_on_going) {
      return false;
    }

    word = new_word;
    if (has_hint) {
      hint_path = std::filesystem::path(hint_path_str);
    } else {
      hint_path = std::nullopt;
    }

    numErrors = new_num_errors;
    currentTrial = new_current_trial;
    onGoing = new_on_going;
    maxErrors = new_max_errors;
    plays = new_plays;
    word_guesses = new_word_guesses;

    // Derived:
    wordLen = (uint32_t)word.length();
    lettersRemaining = 0;
    for (char c : word) {
      if (std::find(plays.begin(), plays.end(), c) == plays.end()) {
        lettersRemaining += 1;
      }
    }
    if (word_guesses.size() >= 1 && word == word_guesses.back()) {
      lettersRemaining = 0;
    }

    std::cout << "Loaded game for player " << playerId << " from file"
              << std::endl;
    return true;
  } catch (std::exception& e) {
    std::cerr << "[ERROR] Failed to save game (player " << playerId
              << ") to file: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "[ERROR] Failed to save game (player " << playerId
              << ") to file: unknown" << std::endl;
  }
  return false;
}
