@cd "%~dp0"
@echo off

printf "\nLaunching Kernel... (QEMU i386)\n"
"C:\Program Files (x86)\qemu\qemu-system-i386" -cdrom iso\KernelSharp.iso -m 512
if ERRORLEVEL 1 ( call:errorhandle "Launching QEMU" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF