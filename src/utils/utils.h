#pragma once

#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define AXIS_X_MIN 0
#define AXIS_Y_MIN 0
#define AXIS_X_MAX 1000
#define AXIS_Y_MAX 400
#define MAX_TOUCHES 10

#define MAX_TRACKING_ID 65535

#define myLogAndQuit(message)                                                  \
  perror(message);                                                             \
  return NULL

struct libevdev_uinput *openDevice(int resolution);

int openServer(int port);