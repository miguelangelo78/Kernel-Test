#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

readelf -a iso/ksharp.bin > out_bindump
xdg-open ./out_bindump
rm -f out_bindump