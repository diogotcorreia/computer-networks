#ifndef COMMANDS_H
#define COMMANDS_H

#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "player_state.hpp"

class CommandHandler;

class CommandManager {
  std::vector<std::shared_ptr<CommandHandler>> handlerList;
  std::unordered_map<std::string, std::shared_ptr<CommandHandler>> handlers;
  void printHelp();

 public:
  void registerCommand(std::shared_ptr<CommandHandler> handler);
  void waitForCommand(PlayerState& state);
};

class CommandHandler {
 protected:
  CommandHandler(const char* name, const std::optional<const char*> alias,
                 const std::optional<const char*> usage,
                 const char* description)
      : name{name}, alias{alias}, usage{usage}, description{description} {}

 public:
  const char* name;
  const std::optional<const char*> alias;
  const std::optional<const char*> usage;
  const char* description;
  virtual void handle(std::string args, PlayerState& state) = 0;
};

class StartCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  StartCommand() : CommandHandler("start", "sg", "PLID", "Start a new game") {}
};

#endif
