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
  PlayerState state(config.host, config.port);

  CommandManager commandManager;
  registerCommands(commandManager);

  commandManager.printHelp();

  while (!std::cin.eof() && !state.getExitState()) {
    commandManager.waitForCommand(state);
  }
  return 0;
}

void set_signals(PlayerState &state) {
  // set SIGINT handler to close server gracefully
  struct sigaction sa;
  sa.sa_sigaction = sigint_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Setting sigaction");
    exit(EXIT_FAILURE);
  }

  // send state to the signal handler
  union sigval value;
  value.sival_ptr = &state;
  if (sigqueue(getpid(), SIGINT, value) == -1) {
    perror("Sending signal");
    exit(EXIT_FAILURE);
  }

  // ignore SIGPIPE
  signal(SIGPIPE, SIG_IGN);
}

void sigint_handler(int sig, siginfo_t *siginfo, void *context) {
  (void)sig;
  (void)context;
  PlayerState *state = (PlayerState *)siginfo->si_value.sival_ptr;
  state->setExitState();
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
