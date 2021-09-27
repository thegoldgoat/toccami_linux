#include "udp.h"

void *startUDPDaemon(void *data) {

  bool *canContinue = (bool *)((struct UDPDaemonData *)data)->canContinue;

  char udpResponse[1024];
  int udpResponseLen = snprintf(udpResponse, sizeof(udpResponse), "%s:%d",
                                ((struct UDPDaemonData *)data)->serverName,
                                ((struct UDPDaemonData *)data)->port);

  struct sockaddr_in serverSocketAddress, clientSocketAddress;

  int serverSocket, recv_len;
  socklen_t slen = sizeof(clientSocketAddress);
  char buf[BUFLEN];

  // create a UDP socket
  if ((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    UDPdie("socket");
  }

  // zero out the structure
  memset((char *)&serverSocketAddress, 0, sizeof(serverSocketAddress));

  serverSocketAddress.sin_family = AF_INET;
  serverSocketAddress.sin_port = htons(PORT);
  serverSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind socket to port
  if (bind(serverSocket, (struct sockaddr *)&serverSocketAddress,
           sizeof(serverSocketAddress)) == -1) {
    UDPdie("bind");
  }

  // keep listening for data
  while (*canContinue) {
    // try to receive some data, this is a blocking call
    if ((recv_len = recvfrom(serverSocket, buf, BUFLEN, 0,
                             (struct sockaddr *)&clientSocketAddress, &slen)) ==
        -1) {
      perror("recvfrom()");
      continue;
    }

    // print details of the client/peer and the data received

    if (buf[0] != 127) {
      printf("Weird data received!\n");
      continue;
    }

    // now reply the client with the same data
    if (sendto(serverSocket, udpResponse, udpResponseLen, 0,
               (struct sockaddr *)&clientSocketAddress, slen) == -1) {
      printf("sendto()");
      continue;
    }
  }

  close(serverSocket);
  return NULL;
}