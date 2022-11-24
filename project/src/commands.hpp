#include <optional>
#include <sstream>
#include <string>

class CommandHandler {
 public:
  virtual std::string getName() = 0;
  virtual std::optional<std::string> getAlias() = 0;
  virtual std::string getDescription() = 0;
  virtual void handle(std::stringstream args) = 0;
};

class StartCommand : public CommandHandler {
  std::string getName() {
    return "start";
  }
  std::optional<std::string> getAlias() {
    return "sg";
  }
  std::string getDescription() {
    return "Start a new game";
  }
  void handle(std::stringstream args);
};
