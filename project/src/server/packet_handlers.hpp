#ifndef PACKET_HANDLERS_H
#define PACKET_HANDLERS_H

#include <sstream>

#include "common/constants.hpp"
#include "server_state.hpp"

class playerTag {  // IOManip helper
  uint32_t player_id;

 public:
  explicit playerTag(uint32_t __player_id) : player_id{__player_id} {}
  friend std::ostream &operator<<(std::ostream &os, const playerTag &obj) {
    os << "[Player " << std::setfill('0') << std::setw(PLAYER_ID_MAX_LEN)
       << obj.player_id << "] ";
    return os;
  }
};

// UDP

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state);

void handle_guess_letter(std::stringstream &buffer, Address &addr_from,
                         GameServerState &state);

void handle_guess_word(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state);

void handle_quit_game(std::stringstream &buffer, Address &addr_from,
                      GameServerState &state);

void handle_reveal_word(std::stringstream &buffer, Address &addr_from,
                        GameServerState &state);

// TCP

void handle_scoreboard(int connection_fd, GameServerState &state);

void handle_hint(int connection_fd, GameServerState &state);

void handle_state(int connection_fd, GameServerState &state);

#endif
