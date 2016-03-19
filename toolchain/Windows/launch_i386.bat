@cd "%~dp0"
@echo off

cd ..\..
printf "\nLaunching Kernel... (QEMU i386)\n"
"C:\Program Files (x86)\qemu\qemu-system-i386" -cdrom iso\KernelSharp.iso -m 512
if ERRORLEVEL 1 ( call:errorhandle "Launching Kernel" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF