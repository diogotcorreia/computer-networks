#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

#include "packet.hpp"

int main() {
  // Create a socket
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Create a server address
  struct sockaddr_in address, client_address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // bind the socket to the address
  if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // listen for connections
  while (true) {
    printf("Listening for connections");
    // receive a packet
    Packet *packet = receive_packet(fd, &client_address, 1024);
    SNG *sng = (SNG *)packet;
    printf("Received SNG packet with player_id: %d", sng->player_id);
  }
}
