#include "packet_handlers.hpp"

#include <iostream>

#include "common/protocol.hpp"

void handle_start_game(std::stringstream &buffer, Address &addr_from,
                       GameServerState &state) {
  StartGameServerbound packet;
  packet.deserialize(buffer);
  // TODO
  printf("Received SNG packet with player_id: %d\n", packet.player_id);

  ReplyStartGameClientbound response;
  try {
    auto game = state.createGame(packet.player_id);
    response.success = true;
    response.n_letters = game.getWordLen();
    response.max_errors = game.getMaxErrors();
  } catch (GameAlreadyStartedException &e) {
    response.success = false;
  } catch (std::exception &e) {
    std::cerr << "There was an unhandled exception that prevented the server "
                 "from starting a new game:"
              << e.what() << std::endl;
  }

  send_packet(response, addr_from.socket, (struct sockaddr *)&addr_from.addr,
              addr_from.size);
}
