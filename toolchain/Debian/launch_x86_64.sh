#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

printf "\nLaunching Kernel... (x86_64)\n"
# Launch GDB in a new terminal:
x-terminal-emulator -e gdb -x toolchain/gdb.gdbinit

# Launch one of the following available Virtual Machines:
#*********** AVAILABLE VIRTUAL MACHINES: ***********

qemu-system-x86_64 -cdrom iso/KernelSharp.iso -serial stdio -gdb tcp:127.0.0.1:1234 -m 128
#virtualbox --startvm KernelSharp --dbg
#vmplayer toolchain/VMachs/VMWare/Linux/KernelSharp/KernelSharp.vmx

#****************************************************