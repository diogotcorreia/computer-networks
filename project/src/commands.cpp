#include "commands.hpp"

#include <iostream>

void StartCommand::handle(std::stringstream args) {
  std::cout << "Executed start command! :)" << std::endl;
};
