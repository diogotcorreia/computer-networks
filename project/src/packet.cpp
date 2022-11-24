#include "packet.hpp"

std::stringstream SNG::serialize() {
  std::stringstream buffer;
  buffer << "SNG " << player_id << std::endl;
  return buffer;
};

void SNG::deserialize(std::stringstream buffer) {
  char opcode[4];
  buffer >> opcode >> player_id;
};

std::stringstream RSG::serialize(){
    // Serialize the packet
    // ...
};

void RSG::deserialize(std::stringstream buffer){
    // Deserialize the packet
    // ...
};
