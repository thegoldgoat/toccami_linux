# Toccami Linux

Desktop version of the Virtual Multitouch Touchpad Mobile Application, for Linux.

## Architecture

1. Create a virtual kernel device using libevdev
2. Start a TCP server waiting for events from the remote mobile application
3. Start a UDP server to automatically show the server on the mobile application

## Build and Run

```bash

# Install build dependencies (Ubuntu)
sudo apt install ninja-build meson libevdev-dev

# Build
meson setup build
cd build
ninja

# Run with a custom name, 7777 port and resolution (sensitivity) of 5
sudo ./toccami_linux -n "Desktop Name" -p 7777 -r 5

```
