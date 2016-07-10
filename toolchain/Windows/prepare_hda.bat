@cd "%~dp0"
@echo off

cd ..\..

SET sector_size=4096
SET sector_count=1310

printf "\n>>>>>>> Preparing Hard Disk with EXT2 <<<<<<<\n"
printf "\n> Sector Size: %sector_size% | Sector Count: %sector_count%\n"
toolchain\Windows\Tools\genext2fs -d hda -D toolchain/Windows/Tools/devtable -U -b %sector_count% -N %sector_size% iso/hda.img
printf "\n>>>>>>> Done! <<<<<<<\n"
