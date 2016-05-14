#!/bin/bash
cd `dirname $0`
source ./sheader.sh
clear

printf "\nSTEP 1 - Updating Kernel Version...\n\n"
python toolchain/versioning.py

printf "STEP 2 - Compiling and Linking...\n\n"
make -f toolchain/makefile.mak all

printf "\nSTEP 3 - Generating initrd disk ...\n\n"
rm -f iso/initrd.img
toolchain/Debian/Tools/initrd_gen `find obj/modules/*` 

printf "\n\nSTEP 4 - Creating disk (ISO) ...\n\n"
rm -f iso/KernelSharp.iso
mkisofs -o iso/KernelSharp.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -input-charset utf-8 iso/boot/syslinux iso/ksharp.bin iso/initrd.img

printf "\n********** BUILD COMPLETED **********\n"
