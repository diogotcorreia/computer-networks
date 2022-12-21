#include "server_game.hpp"

#include <cstring>
#include <stdexcept>

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

bool ServerGame::guessWord(std::string &word_guess, uint32_t trial) {
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

std::string ServerGame::getWord() {
  return word;
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
