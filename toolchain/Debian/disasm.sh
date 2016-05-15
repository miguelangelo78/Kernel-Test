#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

objdump -M intel -d iso/ksharp.bin > out_disasm
xdg-open ./out_disasm
rm -f out_disasm