#!/bin/sh
echo "Setting up toolchain for Linux Debian"

echo "Press any key to exit ..."
read usr

# Close Terminal Emulator:
kill $(xdotool getactivewindow getwindowpid)