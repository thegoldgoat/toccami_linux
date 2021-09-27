#include "utils.h"

struct libevdev_uinput *openDevice(int resolution) {
  struct libevdev *dev;
  struct libevdev_uinput *uidev;

  dev = libevdev_new();
  libevdev_set_name(dev, "Toccami");

  if (libevdev_enable_event_type(dev, EV_ABS)) {
    myLogAndQuit("EV_ABS\n");
  }

  if (libevdev_enable_event_type(dev, EV_KEY)) {
    myLogAndQuit("EV_KEY\n");
  }

  struct input_absinfo mtAbsInfo, mtTrackingIdInfo, absInfoX, absInfoY;

  memset(&mtAbsInfo, 0, sizeof(struct input_absinfo));
  memset(&mtTrackingIdInfo, 0, sizeof(struct input_absinfo));
  memset(&absInfoX, 0, sizeof(struct input_absinfo));
  memset(&absInfoY, 0, sizeof(struct input_absinfo));

  mtAbsInfo.maximum = MAX_TOUCHES;
  mtAbsInfo.value = 0;

  mtTrackingIdInfo.minimum = 0;
  mtTrackingIdInfo.maximum = MAX_TRACKING_ID;
  mtTrackingIdInfo.value = 0;

  absInfoX.minimum = AXIS_X_MIN;
  absInfoX.maximum = AXIS_X_MAX;
  absInfoX.resolution = resolution;

  absInfoY.minimum = AXIS_Y_MIN;
  absInfoY.maximum = AXIS_Y_MAX;
  absInfoY.resolution = resolution;

  if (libevdev_enable_event_code(dev, EV_ABS, ABS_MT_SLOT, &mtAbsInfo)) {
    myLogAndQuit("ABS_MT_SLOT\n");
  }

  if (libevdev_enable_event_code(dev, EV_ABS, ABS_MT_TRACKING_ID,
                                 &mtTrackingIdInfo)) {
    myLogAndQuit("ABS_MT_TRACKING_ID\n");
  }

  if (libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_X, &absInfoX)) {
    myLogAndQuit("ABS_MT_POSITION_X\n");
  }
  if (libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_Y, &absInfoY)) {
    myLogAndQuit("ABS_MT_POSITION_Y\n");
  }

  if (libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absInfoX)) {
    myLogAndQuit("ABS_X\n");
  }
  if (libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &absInfoY)) {
    myLogAndQuit("ABS_Y\n");
  }

  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOUCH, NULL)) {
    myLogAndQuit("BTN_TOUCH\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_FINGER, NULL)) {
    myLogAndQuit("BTN_TOOL_FINGER\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL)) {
    myLogAndQuit("BTN_LEFT\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_QUINTTAP, NULL)) {
    myLogAndQuit("BTN_TOOL_QUINTTAP\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_DOUBLETAP, NULL)) {
    myLogAndQuit("BTN_TOOL_DOUBLETAP\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_TRIPLETAP, NULL)) {
    myLogAndQuit("BTN_TOOL_TRIPLETAP\n");
  }
  if (libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_QUADTAP, NULL)) {
    myLogAndQuit("BTN_TOOL_QUADTAP\n");
  }

  if (libevdev_enable_property(dev, INPUT_PROP_POINTER)) {
    myLogAndQuit("INPUT_PROP_POINTER\n");
  }

  int err = libevdev_uinput_create_from_device(
      dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);

  if (err != 0)
    return NULL;

  return uidev;
}

int openServer(int port) {
  struct sockaddr_in serverAddress;
  int serverSocketFd;

  serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocketFd < 0)
    perror("Error while socket");

  memset((void *)&serverAddress, 0, sizeof(serverAddress));

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(port);

  if (bind(serverSocketFd, (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) < 0) {
    perror("ERROR on binding");
    return -1;
  }

  if (listen(serverSocketFd, 1) < 0) {
    perror("listen");
    return -1;
  }

  return serverSocketFd;
}