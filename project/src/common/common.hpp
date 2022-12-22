#ifndef COMMON_H
#define COMMON_H

#include <cstring>
#include <stdexcept>

class UnrecoverableError : public std::runtime_error {
 public:
  UnrecoverableError(const std::string& __what) : std::runtime_error(__what) {}
  UnrecoverableError(const std::string& __what, const int __errno)
      : std::runtime_error(__what + ": " + strerror(__errno)) {}
};

void validate_port_number(std::string& port);

void setup_signal_handlers();
void terminate_signal_handler(int sig);

#endif
