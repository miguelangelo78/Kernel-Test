@cd "%~dp0"
@echo off

cd ..\..
printf "\nLaunching Kernel... (x86_64)\n"
"C:\Program Files (x86)\qemu\qemu-system-x86_64" -cdrom iso\KernelSharp.iso -m 512
@rem VirtualBox --startvm KernelSharp --dbg
if ERRORLEVEL 1 ( call:errorhandle "Launching Virtual Box" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF