#include "player.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "common/common.hpp"

bool is_shutting_down = false;

int main(int argc, char *argv[]) {
  try {
    setup_signal_handlers();

    ClientConfig config(argc, argv);
    if (config.help) {
      config.printHelp(std::cout);
      return EXIT_SUCCESS;
    }
    PlayerState state(config.host, config.port);

    CommandManager commandManager;
    registerCommands(commandManager);

    commandManager.printHelp();

    while (!std::cin.eof() && !is_shutting_down) {
      commandManager.waitForCommand(state);
    }

    std::cout << std::endl
              << "Shutting down... Press CTRL + C again to forcefully close "
                 "the player."
              << std::endl;

    if (state.hasActiveGame()) {
      std::cout << "Player has an active game, will attempt to quit it."
                << std::endl;
      try {
        QuitGameServerbound packet;
        QuitGameClientbound reply;
        packet.player_id = state.game->getPlayerId();
        is_shutting_down = false;  // otherwise can't send packets
        state.sendUdpPacketAndWaitForReply(packet, reply);
        if (reply.status == QuitGameClientbound::status::ERR) {
          throw std::runtime_error("Server replied with ERR");
        }
        std::cout << "Game quit successfully." << std::endl;
      } catch (...) {
        std::cerr << "Failed to quit game. Exiting anyway..." << std::endl;
      }
    }

  } catch (std::exception &e) {
    std::cerr << "Encountered unrecoverable error while running the "
                 "application. Shutting down..."
              << std::endl
              << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Encountered unrecoverable error while running the "
                 "application. Shutting down..."
              << std::endl;
  }

  return EXIT_SUCCESS;
}

void setup_signal_handlers() {
  // set SIGINT/SIGTERM handler to close server gracefully
  struct sigaction sa;
  sa.sa_handler = terminate_signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Setting SIGINT signal handler");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Setting SIGTERM signal handler");
    exit(EXIT_FAILURE);
  }

  // ignore SIGPIPE
  signal(SIGPIPE, SIG_IGN);
}

void terminate_signal_handler(int sig) {
  (void)sig;
  if (is_shutting_down) {
    exit(EXIT_SUCCESS);
  }
  is_shutting_down = true;
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
        std::cerr << std::endl;
        printHelp(std::cerr);
        exit(EXIT_FAILURE);
    }
  }

  for (char c : port) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw UnrecoverableError("Invalid port: not a number");
    }
  }

  try {
    if (std::stoi(port) > ((1 << 16) - 1)) {
      throw std::runtime_error("");
    }
  } catch (...) {
    throw UnrecoverableError("Invalid port: it must be, at maximum, 65535");
  }
}

void ClientConfig::printHelp(std::ostream &stream) {
  stream << "Usage: " << program_path << " [-n GSIP] [-p GSport] [-h]"
         << std::endl;
  stream << "Available options:" << std::endl;
  stream << "-n GSIP\t\tSet hostname of Game Server. Default: "
         << DEFAULT_HOSTNAME << std::endl;
  stream << "-p GSport\tSet port of Game Server. Default: " << DEFAULT_PORT
         << std::endl;
  stream << "-h\t\tPrint this menu." << std::endl;
}
