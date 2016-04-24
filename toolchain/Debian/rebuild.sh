#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

# Generate makefile project:
python toolchain/genmake.py
# Build makefile project:
toolchain/Debian/build.sh
