cls
@echo off

set arg=%1

printf "STEP 1 - Compiling and Linking...\n\n"
make -f toolchain\makefile.mak all
if ERRORLEVEL 1 ( call:error_compile "Compiling and Linking" )

printf "\nSTEP 2 - Creating disk (ISO9660)...\n\n"
del iso\KernelSharp.iso
toolchain\Tools\ISO9660Generator.exe 4 "%CD%\iso\KernelSharp.iso" "%CD%\iso\isolinux-debug.bin" true "%CD%\iso"
if ERRORLEVEL 1 ( call:errorhandle "Creating Disk" )

call launch_i386.bat

exit

:errorhandle
IF NOT "%1"=="" (printf "****\nERROR: %~1\n****\n")
pause
exit

:error_compile
printf "****\nERROR: %~1\n****\n"
IF [%arg%] == [2] (printf "\n**** Exhausted build tries. The problem is on the source code. ****\n\n" && call:errorhandle)
printf "**** Trying again by rebuilding (due to outdated symbols)... ****\n"
set /a arg_inc =%arg%+1
call rebuild.bat %arg_inc%