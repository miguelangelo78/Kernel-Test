@cd "%~dp0"
@echo off

cd ..\..
printf "\nLaunching Kernel... (QEMU i386)\n"
start toolchain/Windows/Tools/putty.exe -load KernelSerial
start gdb -x toolchain/gdb.gdbinit
"C:\Program Files (x86)\qemu\qemu-system-i386" -cdrom iso\KernelSharp.iso -serial COM1 -gdb tcp:127.0.0.1:1234 -m 512
if ERRORLEVEL 1 ( call:errorhandle "Launching Kernel" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF