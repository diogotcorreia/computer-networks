#include "GS.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8080"

int main() {
  int fd, errcode;
  struct addrinfo hints, *res;
  struct sockaddr_in addr;

  // Create a socket
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM;  // UDP socket
  hints.ai_flags = AI_PASSIVE;

  errcode = getaddrinfo(NULL, PORT, &hints, &res);
  if (errcode != 0) /*error*/
    exit(1);
  if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) /*error*/
    exit(1);

  // listen for connections

  printf("Listening for connections\n");
  // TESTING: receiving and sending a packet
  Packet *packet = receive_packet(fd, (struct sockaddr *)&addr);
  SNG *sng = (SNG *)packet;
  printf("Received SNG packet with player_id: %d\n", sng->player_id);
  RSG *rsg = new RSG();
  rsg->success = true;
  rsg->n_letters = 5;
  rsg->max_errors = 5;
  send_packet(rsg, fd, (struct sockaddr *)&addr, sizeof(addr));
  freeaddrinfo(res);
  close(fd);
}
