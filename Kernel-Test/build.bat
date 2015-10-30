cls
@echo off

printf "STEP 1 - Building Stage 2 (ksharp_stage2.asm)...\n\n"
toolchain\Tools\NASM\nasm.exe -g -f elf -o build\ksharp_stage2.o src\arch\x86\ksharp_stage2.asm
if ERRORLEVEL 1 ( call:errorhandle "Assembling ksharp_stage2.asm" )

printf "STEP 2 - Compiling and Linking...\n\n"
make -f toolchain\makefile.mak all
@rem toolchain\Tools\Cross\i686-elf\bin\i686-elf-g++.exe build\ksharp_stage2.o src\kmain.cpp -o iso\ksharp.bin -T toolchain\linker.ld -nostartfiles -nostdlib
if ERRORLEVEL 1 ( call:errorhandle "Compiling and Linking" )

printf "\nSTEP 3 - Creating disk (ISO9660)...\n\n"
del iso\KernelSharp.iso
toolchain\Tools\ISO9660Generator.exe 4 "%CD%\iso\KernelSharp.iso" "%CD%\iso\isolinux-debug.bin" true "%CD%\iso"
if ERRORLEVEL 1 ( call:errorhandle "Creating Disk" )

call launch_i386.bat

exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF