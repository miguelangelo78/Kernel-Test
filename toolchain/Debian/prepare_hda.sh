#!/bin/bash
cd `dirname $0`
source ./sheader.sh

sector_size=4096
sector_count=1310

printf "\n>>>>>>> Preparing Hard Disk (iso/hda.img) with EXT2 <<<<<<<\n"
printf "\n> Sector Size: $sector_size | Sector Count: $sector_count\n"
toolchain/Debian/Tools/genext2fs -d hda -D toolchain/Debian/Tools/devtable -U -b $sector_count% -N $sector_size iso/hda.img
printf "\n>>>>>>> Done! <<<<<<<\n"