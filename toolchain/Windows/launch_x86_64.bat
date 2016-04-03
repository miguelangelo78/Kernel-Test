@cd "%~dp0"
@echo off

cd ..\..
printf "\nLaunching Kernel... (x86_64)\n"
start toolchain/Tools/Windows/putty.exe -load KernelSerial
@rem start gdb -x toolchain/gdb.gdbinit
@rem "C:\Program Files (x86)\qemu\qemu-system-x86_64" -cdrom iso\KernelSharp.iso -serial COM1 -gdb tcp:127.0.0.1:1234 -m 128
VirtualBox --startvm KernelSharp --dbg
if ERRORLEVEL 1 ( call:errorhandle "Launching Kernel" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF