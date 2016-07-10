#!/bin/sh
cd `dirname $0`
echo "Setting up toolchain for Linux Debian"

./kerneldev_toolkit_downloader.sh
read usr

# Close Terminal Emulator:
kill $(xdotool getactivewindow getwindowpid)