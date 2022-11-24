#include <sstream>

class Packet {
 public:
  virtual std::stringstream serialize();
  void virtual deserialize(std::stringstream buffer);
};

// Start New Game Packet
class SNG : public Packet {
 public:
  int player_id;
  std::stringstream serialize();
  void deserialize(std::stringstream buffer);
};

// Reply to Start Game Packet
class RSG : public Packet {
 public:
  bool success;
  int n_letters;
  int max_errors;

  std::stringstream serialize();
  void deserialize(std::stringstream buffer);
};
