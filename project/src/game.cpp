#include "game.hpp"

#include <cstring>

#include "packet.hpp"

ClientGame::ClientGame(int player_id, int word_len, int max_errors) {
  this->player_id = player_id;
  this->word_len = word_len;
  this->max_errors = max_errors;
  this->current_trial = 0;
  this->word_progess = new char[word_len + 1];
  memset(this->word_progess, '_', word_len);
  this->word_progess[word_len] = '\0';
  this->num_errors = 0;
}

ClientGame::~ClientGame() {
  delete[] this->word_progess;
}

void ClientGame::update_word_progress(char *word) {
  strcpy(this->word_progess, word);
}

void ClientGame::update_current_trial() {
  this->current_trial++;
}

void ClientGame::update_num_errors() {
  this->num_errors++;
}

ClientGame::~ClientGame() {
  delete[] this->word_progess;
}

ServerGame::ServerGame(int player_id) {
  this->player_id = player_id;
  // TODO: Get word from file
  word = "test";
  this->word_len = strlen(word);
  // TODO: Get max errors from one liner
  if (word_len <= 6) {
    this->max_errors = 7;
  } else if (word_len <= 10) {
    this->max_errors = 8;
  } else if (word_len <= 30) {
    this->max_errors = 9;
  } else {
    // handle exception
  }
  this->current_trial = 0;
  this->word_progess = new char[word_len];
  for (int i = 0; i < word_len; i++) {
    this->word_progess[i] = '_';
  }
}

int Game::get_word_len() {
  return word_len;
}
int Game::get_max_errors() {
  return max_errors;
}

int Game::get_player_id() {
  return player_id;
}

int Game::get_current_trial() {
  return current_trial;
}

bool ServerGame::play(char letter) {
  bool found = false;
  for (int i = 0; i < word_len; i++) {
    if (word[i] == letter) {
      word_progess[i] = letter;
      found = true;
    }
  }
  if (!found) {
    num_errors++;
  }
  current_trial++;
  return found;
}

bool ServerGame::guess(char *word) {
  if (strcmp(this->word, word) == 0) {
    return true;
  }
  this->num_errors++;
  this->current_trial++;
  return false;
}

bool ServerGame::is_over() {
  if (num_errors >= max_errors) {
    return true;
  }
  for (int i = 0; i < word_len; i++) {
    if (word_progess[i] == '_') {
      return false;
    }
  }
  return true;
}
