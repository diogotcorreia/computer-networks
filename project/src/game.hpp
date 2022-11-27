#ifndef GAME_H
#define GAME_H

class Game {
 protected:
  int player_id;
  int word_len;
  int max_errors;
  int current_trial;
  char *word_progess;
  int num_errors;

 public:
  ~Game();
  int get_player_id();
  int get_word_len();
  int get_max_errors();
  int get_current_trial();
  char *get_word_progess();
  int get_num_errors();
};

class ServerGame : public Game {
 private:
  char *word;

 public:
  ServerGame(int player_id);
  ~ServerGame();
  bool play(char letter);
  bool guess(char *word);
  bool is_over();
};

class ClientGame : public Game {
 public:
  ClientGame(int player_id, int word_len, int max_errors);
  ~ClientGame();
  void update_word_progress(char *word);
  void update_current_trial();
  void update_num_errors();
};

#endif
