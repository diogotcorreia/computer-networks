#include "server_state.hpp"

#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "common/protocol.hpp"
#include "packet_handlers.hpp"

GameServerState::GameServerState(std::string &__word_file_path,
                                 std::string &port, bool __verbose, bool __test)
    : cdebug{DebugStream(__verbose)} {
  this->test = __test;
  this->setup_sockets();
  this->resolveServerAddress(port);
  this->registerWords(__word_file_path);
}

GameServerState::~GameServerState() {
  close(this->udp_socket_fd);
  close(this->tcp_socket_fd);
  freeaddrinfo(this->server_udp_addr);
  freeaddrinfo(this->server_tcp_addr);
}

void GameServerState::registerPacketHandlers() {
  // UDP
  udp_packet_handlers.insert({StartGameServerbound::ID, handle_start_game});
  udp_packet_handlers.insert({GuessLetterServerbound::ID, handle_guess_letter});
  udp_packet_handlers.insert({GuessWordServerbound::ID, handle_guess_word});
  udp_packet_handlers.insert({QuitGameServerbound::ID, handle_quit_game});
  udp_packet_handlers.insert({RevealWordServerbound::ID, handle_reveal_word});

  // TCP
  tcp_packet_handlers.insert({StateServerbound::ID, handle_state});
}

void GameServerState::setup_sockets() {
  // Create a UDP socket
  if ((this->udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a UDP socket");
    exit(EXIT_FAILURE);
  }

  // Create a TCP socket
  if ((this->tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    // TODO consider using exceptions (?)
    perror("Failed to create a TCP socket");
    exit(EXIT_FAILURE);
  }
  const int enable = 1;
  if (setsockopt(this->tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable,
                 sizeof(int)) < 0) {
    perror("Failed to set socket options");
    exit(EXIT_FAILURE);
  }
}

void GameServerState::resolveServerAddress(std::string &port) {
  struct addrinfo hints;
  const char *port_str = port.c_str();
  // Get UDP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  if (getaddrinfo(NULL, port_str, &hints, &this->server_udp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for UDP connection");
    exit(EXIT_FAILURE);
  }
  // bind socket
  if (bind(this->udp_socket_fd, this->server_udp_addr->ai_addr,
           this->server_udp_addr->ai_addrlen)) {
    // TODO consider using exceptions (?)
    perror("Failed to bind UDP address");
    exit(EXIT_FAILURE);
  }

  // Get TCP address
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP socket
  if (getaddrinfo(NULL, port.c_str(), &hints, &this->server_tcp_addr) != 0) {
    // TODO consider using exceptions (?)
    perror("Failed to get address for TCP connection");
    exit(1);
  }

  if (bind(this->tcp_socket_fd, this->server_tcp_addr->ai_addr,
           this->server_tcp_addr->ai_addrlen)) {
    // TODO consider using exceptions (?)
    perror("Failed to bind TCP address");
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening for connections on port " << port << std::endl;
}

void GameServerState::registerWords(std::string &__word_file_path) {
  try {
    std::filesystem::path word_file_path(std::filesystem::current_path());
    word_file_path.append(__word_file_path);

    std::cout << "Reading words from " << word_file_path << std::endl;

    std::ifstream word_file(word_file_path);
    if (!word_file.is_open()) {
      perror("Failed to open word file");
      exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(word_file, line)) {
      auto split_index = line.find(' ');
      Word word;
      word.word = line.substr(0, split_index);
      if (split_index != std::string::npos) {
        std::filesystem::path hint_file_path(word_file_path);
        hint_file_path.remove_filename().append(line.substr(split_index + 1));

        if (!hint_file_path.has_filename()) {
          std::cerr << "[WARNING] Hint file path " << hint_file_path
                    << ", for word '" << word.word << "' is not a file."
                    << std::endl;
        }

        word.hint_path = hint_file_path;
      } else {
        std::cerr << "[WARNING] Word '" << word.word
                  << "' does not have an hint file" << std::endl;
        word.hint_path = std::nullopt;
      }
      this->words.push_back(word);
    }

    std::cout << "Loaded " << words.size() << " word(s)";
  } catch (std::exception &e) {
    std::cerr << "Failed to open word file: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << "Failed to open word file: unknown" << std::endl;
    exit(EXIT_FAILURE);
  }
}

Word &GameServerState::selectRandomWord() {
  uint32_t index;
  if (test) {
    index = (this->current_word_index) % (uint32_t)this->words.size();
    this->current_word_index = index + 1;
  } else {
    index = (uint32_t)rand() % (uint32_t)this->words.size();
  }
  return this->words[index];
}

void GameServerState::callUdpPacketHandler(std::string packet_id,
                                           std::stringstream &stream,
                                           Address &addr_from) {
  auto handler = this->udp_packet_handlers.find(packet_id);
  if (handler == this->udp_packet_handlers.end()) {
    // TODO add exception
    std::cout << "Unknown Packet ID" << std::endl;
    return;
  }

  handler->second(stream, addr_from, *this);
}

void GameServerState::callTcpPacketHandler(std::string packet_id,
                                           int connection_fd) {
  auto handler = this->tcp_packet_handlers.find(packet_id);
  if (handler == this->tcp_packet_handlers.end()) {
    // TODO add exception
    std::cout << "Unknown Packet ID" << std::endl;
    return;
  }

  handler->second(connection_fd, *this);
}

ServerGameSync GameServerState::createGame(uint32_t player_id) {
  std::scoped_lock<std::mutex> g_lock(gamesLock);

  auto game = games.find(player_id);
  if (game != games.end()) {
    {
      ServerGameSync game_sync = ServerGameSync(game->second);
      if (game_sync->isOnGoing()) {
        if (game_sync->hasStarted()) {
          throw GameAlreadyStartedException();
        }
        return game_sync;
      }
    }

    std::cout << "Deleting game" << std::endl;
    // Delete existing game, so we can create a new one below
    games.erase(game);
  }

  Word &word = this->selectRandomWord();
  // Some C++ magic to create an instance of the class inside the map, without
  // moving it, since mutexes can't be moved
  games.emplace(std::piecewise_construct, std::forward_as_tuple(player_id),
                std::forward_as_tuple(player_id, word.word, word.hint_path));

  return ServerGameSync(games.at(player_id));
}

ServerGameSync GameServerState::getGame(uint32_t player_id) {
  std::scoped_lock<std::mutex> g_lock(gamesLock);

  auto game = games.find(player_id);
  if (game == games.end()) {
    throw NoGameFoundException();
  }

  return ServerGameSync(game->second);
}
