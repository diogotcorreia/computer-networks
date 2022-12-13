#include "player.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

int main(int argc, char *argv[]) {
  ClientConfig config(argc, argv);
  if (config.help) {
    config.printHelp(std::cout);
    exit(EXIT_SUCCESS);
  }
  PlayerState state;
  state.resolveServerAddress(config.host, config.port);

  CommandManager commandManager;
  registerCommands(commandManager);

  commandManager.printHelp();

  while (!std::cin.eof()) {
    commandManager.waitForCommand(state);
  }
  return 0;
}

void registerCommands(CommandManager &manager) {
  manager.registerCommand(std::make_shared<StartCommand>());
  manager.registerCommand(std::make_shared<GuessLetterCommand>());
  manager.registerCommand(std::make_shared<GuessWordCommand>());
  manager.registerCommand(std::make_shared<ScoreboardCommand>());
  manager.registerCommand(std::make_shared<HintCommand>());
  manager.registerCommand(std::make_shared<QuitCommand>());
  manager.registerCommand(std::make_shared<ExitCommand>());
  manager.registerCommand(std::make_shared<RevealCommand>());
  manager.registerCommand(std::make_shared<StateCommand>());
  manager.registerCommand(std::make_shared<KillCommand>());
  manager.registerCommand(std::make_shared<HelpCommand>(manager));
}

ClientConfig::ClientConfig(int argc, char *argv[]) {
  program_path = argv[0];
  int opt;

  opterr = 0;  // don't print default error message on unknown arg
  while ((opt = getopt(argc, argv, "hn:p:")) != -1) {
    switch (opt) {
      case 'n':
        host = std::string(optarg);
        break;
      case 'p':
        port = std::string(optarg);
        break;
      case 'h':
        help = true;
        break;
      default:
        std::cerr << "Unknown argument -" << (char)optopt << std::endl
                  << std::endl;
        printHelp(std::cerr);
        exit(EXIT_FAILURE);
    }
  }
}

void ClientConfig::printHelp(std::ostream &stream) {
  stream << "Usage: " << program_path << " [-n GSIP] [-p GSport] [-h]"
         << std::endl;
  stream << "Available arguments:" << std::endl;
  stream << "-n GSIP\t\tSet hostname of Game Server. Default: "
         << DEFAULT_HOSTNAME << std::endl;
  stream << "-p GSport\tSet port of Game Server. Default: " << DEFAULT_PORT
         << std::endl;
  stream << "-h\t\tPrint this menu." << std::endl;
}
