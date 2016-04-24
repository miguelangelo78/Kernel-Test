#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

printf "\nLaunching Kernel... (x86_64)\n"
konsole -e gdb -x toolchain/gdb.gdbinit

#*********** AVAILABLE VIRTUAL MACHINES: ***********

qemu-system-x86_64 -cdrom iso/KernelSharp.iso -serial stdio -gdb tcp:127.0.0.1:1234 -m 128
#virtualbox --startvm KernelSharp --dbg
#toolchain\Tools\Bochs\kerneltest.bxrc
#"C:\Program Files (x86)\VMware\VMware Workstation\vmrun" start toolchain\Tools\VMWare\KernelTest\KernelTest.vmx

#****************************************************