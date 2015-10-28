@echo off

printf "\nLaunching Kernel... (i386)\n"
"C:\Program Files\qemu\qemu-system-i386" -boot d -cdrom iso\KernelSharp.iso -m 512
@rem vboxmanage.exe startvm KernelSharp
if ERRORLEVEL 1 ( call:errorhandle "Launching Virtual Box" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF