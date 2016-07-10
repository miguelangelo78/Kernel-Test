#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

printf "Cleaning built objects\n"
make -f toolchain/makefile.mak clean
