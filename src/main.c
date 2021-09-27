#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdbool.h>

#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "udp/udp.h"
#include "utils/utils.h"

#define TOCCAMI_EVENT_RELEASED 0
#define TOCCAMI_EVENT_DOWN 1
#define TOCCAMI_EVENT_CHANGE_RESOLUTION 2

void sig_handler(int signo);

bool canContinue = true;

int gethostname();

int main(int argc, char **argv) {

  // Catch SIGINT
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    perror("Couldn't catch SIGINT signals (?)\n");
    return -1;
  }

  // Parse arguments from CLI
  int port = 7777;
  int resolution = 5;
  int c;
  char serverName[1024];
  gethostname(serverName);

  while ((c = getopt(argc, argv, "p:r:n:")) != -1) {
    switch (c) {
    case 'p':
      port = atoi(optarg);
      break;
    case 'r':
      resolution = atoi(optarg);
      break;
    case 'n':
      // Leave some space for the port
      strncpy(serverName, optarg, sizeof(serverName) - 10);
      break;
    case '?':
      fprintf(stderr, "Invalid arguments.\n");
      return -1;
    }
  }

  pthread_t tid;
  struct UDPDaemonData udpData;
  udpData.canContinue = &canContinue;
  udpData.port = port;
  udpData.serverName = serverName;

  pthread_create(&tid, NULL, startUDPDaemon, &udpData);

  struct libevdev_uinput *uidev = openDevice(resolution);

  if (!uidev) {
    perror("Error while creating device");

    return -1;
  }

  int serverSocket, clientSocketFd;

  serverSocket = openServer(port);

  if (serverSocket < 0) {
    perror("Error while opening socket");
    return -1;
  }

  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen = sizeof(clientAddress);

  char buffer[256];
  int bytesRead;
  uint16_t x, y, eventType, pointerIndex;

  while (canContinue) {
    printf("Listening on port %d\n", port);
    clientSocketFd = accept(serverSocket, (struct sockaddr *)&clientAddress,
                            &clientAddressLen);

    int fingersDownCount = 0;
    bool wasFingerDown[MAX_TRACKING_ID];

    if (clientSocketFd < 0) {
      // This happens when we send SIGTERM and the accept syscall gets
      // interrupted.
      continue;
    }

    printf("Client accepted!\n");

    while ((bytesRead = read(clientSocketFd, buffer, sizeof(buffer))) > 0) {
      if (bytesRead % 8 != 0) {
        printf("⚠️: Received payload with invalid lenght\n");
        continue;
      }

      char *offsetBuffer;

      int lastX = -1, lastY = -1;
      for (int i = 0; i < bytesRead / 8; i++) {

        offsetBuffer = buffer + i * 8;

        x = *((uint16_t *)offsetBuffer);
        y = *(uint16_t *)(offsetBuffer + 2);
        pointerIndex = *(uint16_t *)(offsetBuffer + 4);
        eventType = *(uint16_t *)(offsetBuffer + 6);

        switch (eventType) {

        case TOCCAMI_EVENT_DOWN:

          libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_SLOT,
                                      pointerIndex % MAX_TOUCHES);

          if (!wasFingerDown[pointerIndex]) {
            // Finger just pressed!
            fingersDownCount++;
            wasFingerDown[pointerIndex] = true;

            // When the finger is just pressed, register tracking ID
            libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_TRACKING_ID,
                                        pointerIndex);
          }

          libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_POSITION_X, x);
          libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_POSITION_Y, y);

          lastX = x;
          lastY = y;

          break;
        case TOCCAMI_EVENT_RELEASED:
          if (wasFingerDown[pointerIndex]) {
            fingersDownCount--;
            wasFingerDown[pointerIndex] = false;

            // Release Tracking ID after specifying SLOT
            libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_SLOT,
                                        pointerIndex % MAX_TOUCHES);
            libevdev_uinput_write_event(uidev, EV_ABS, ABS_MT_TRACKING_ID, -1);
          }
          break;
        case TOCCAMI_EVENT_CHANGE_RESOLUTION:
          // TODO: Do something!
          break;
        default:
          fprintf(stderr, "Warning: invalid eventType: %d\n", eventType);
          break;
        }

        // printf("fingCount = %d, x=%u, y=%u, pointer=%u, evType=%u\n",
        //        fingersDownCount, x, y, pointerIndex, eventType);
      }

      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_FINGER,
                                  fingersDownCount == 1);

      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOUCH,
                                  fingersDownCount >= 1);
      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_DOUBLETAP,
                                  fingersDownCount == 2);
      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_TRIPLETAP,
                                  fingersDownCount == 3);
      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_QUADTAP,
                                  fingersDownCount == 4);
      libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_QUINTTAP,
                                  fingersDownCount == 5);

      if (lastX != -1) {
        libevdev_uinput_write_event(uidev, EV_ABS, ABS_X, lastX);
        libevdev_uinput_write_event(uidev, EV_ABS, ABS_Y, lastY);
      }

      libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    }
    printf("Client disconnected.\n");
    close(clientSocketFd);
  }

  close(clientSocketFd);
  close(serverSocket);
  libevdev_uinput_destroy(uidev);

  pthread_kill(tid, SIGINT);

  return 0;
}

void sig_handler(int signo) { canContinue = false; }