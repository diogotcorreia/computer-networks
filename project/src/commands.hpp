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

class GuessLetterCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  GuessLetterCommand()
      : CommandHandler("play", "pl", "letter", "Guess a letter") {}
};

class GuessWordCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  GuessWordCommand() : CommandHandler("guess", "gw", "word", "Guess a word") {}
};

class ScoreboardCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  ScoreboardCommand()
      : CommandHandler("scoreboard", "sb", std::nullopt,
                       "Display the scoreboard") {}
};

class HintCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);


 public:
  HintCommand()
      : CommandHandler("hint", "h", std::nullopt,
                       "Get a hint for the current word") {}
};

class StateCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  StateCommand() : CommandHandler("state", "st", std::nullopt, "Show state") {}
};

class QuitCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  QuitCommand() : CommandHandler("quit", std::nullopt, std::nullopt, "Quit game") {}
};

class ExitCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  ExitCommand() : CommandHandler("exit", std::nullopt, std::nullopt, "Exit application") {}
};

class RevealCommand : public CommandHandler {
  void handle(std::string args, PlayerState& state);

 public:
  RevealCommand() : CommandHandler("reveal", "rv", "", "Reveal word") {}
};

void write_word(std::ostream& stream, char* word, int word_len);

#endif
