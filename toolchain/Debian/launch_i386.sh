#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

printf "\nLaunching Kernel... (x86_64)\n"
# Launch GDB in a new terminal:
x-terminal-emulator -e gdb -x toolchain/gdb.gdbinit
# Open QEMU:
qemu-system-i386 -cdrom iso/KernelSharp.iso -serial stdio -gdb tcp:127.0.0.1:1234 -m 128
