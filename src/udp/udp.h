#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/utils.h"

#define BUFLEN 512 // Max length of buffer
#define PORT 3011  // The port on which to listen for incoming data

#define UDPdie(s)                                                              \
  perror(s);                                                                   \
  return NULL;

struct UDPDaemonData {
  bool *canContinue;
  char *serverName;
  int port;
};

void *startUDPDaemon(void *data);