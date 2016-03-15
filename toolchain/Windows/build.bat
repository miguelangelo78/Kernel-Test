@cd "%~dp0"
cls
@echo off

cd ..\..
printf "STEP 1 - Compiling and Linking...\n\n"
make -f toolchain\makefile.mak all
if ERRORLEVEL 1 ( call:errorhandle "Compiling and Linking" )

printf "\nSTEP 2 - Creating disk (ISO9660)...\n\n"
del iso\KernelSharp.iso
toolchain\Tools\Windows\ISO9660Generator.exe 4 "%CD%\iso\KernelSharp.iso" "%CD%\iso\isolinux-debug.bin" true "%CD%\iso"
if ERRORLEVEL 1 ( call:errorhandle "Creating Disk" )

call toolchain\Windows\launch_x86_64.bat

@GOTO:EOF

:errorhandle
IF NOT "%1"=="" (printf "****\nERROR: %~1\n****\n")
pause
exit