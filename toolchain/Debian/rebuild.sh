#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

python toolchain/genmake.py

toolchain/Debian/build.sh
