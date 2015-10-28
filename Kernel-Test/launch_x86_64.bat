@echo off

printf "\nLaunching Kernel... (x86_64)\n"
qemu-system-x86_64 -boot d -cdrom iso\KernelSharp.iso -m 512
@rem vboxmanage.exe startvm KernelSharp
rem if ERRORLEVEL 1 ( call:errorhandle "Launching Virtual Box" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF