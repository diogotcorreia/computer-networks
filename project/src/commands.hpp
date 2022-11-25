#ifndef COMMANDS_H
#define COMMANDS_H

#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

class CommandHandler;

class CommandManager {
  std::unordered_map<std::string, CommandHandler&> handlers;
  void printHelp();

 public:
  void registerCommand(CommandHandler& handler);
  void waitForCommand();
};

class CommandHandler {
 public:
  virtual std::string getName() = 0;
  virtual std::optional<std::string> getAlias() = 0;
  virtual std::string getDescription() = 0;
  virtual void handle(std::string args) = 0;
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
  void handle(std::string args);
};

#endif
