@cd "%~dp0"
@echo off

cd ..\..
printf "\nLaunching Kernel... (x86_64)\n"
start toolchain/Windows/Tools/putty.exe -load KernelSerial
start gdb -x toolchain/gdb.gdbinit

@rem *********** AVAILABLE VIRTUAL MACHINES: ***********

"C:\Program Files (x86)\qemu\qemu-system-x86_64.exe" -cdrom iso\KernelSharp.iso -hda iso/hda.img -serial COM1 -gdb tcp:127.0.0.1:1234 -m 128
@rem "C:\Program Files\Oracle\VirtualBox\VirtualBox.exe" --startvm KernelSharp --dbg
@rem toolchain\VMachs\Bochs\kerneltest.bxrc
@rem "C:\Program Files (x86)\VMware\VMware Workstation\vmrun" start toolchain\Tools\VMWare\KernelTest\KernelTest.vmx

@rem ****************************************************
if ERRORLEVEL 1 ( call:errorhandle "Launching Kernel" )

@echo on
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF