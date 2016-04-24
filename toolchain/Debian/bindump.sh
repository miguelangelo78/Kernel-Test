#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

readelf -a iso/ksharp.bin > out
xdg-open ./out 
rm -f out