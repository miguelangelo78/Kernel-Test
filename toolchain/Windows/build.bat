setlocal enabledelayedexpansion enableextensions

@cd "%~dp0"
cls
@echo off

cd ..\..

printf "\nSTEP 1 - Updating Kernel Version...\n\n"
python toolchain\versioning.py
if ERRORLEVEL 1 ( call:errorhandle "Version Update" )

printf "STEP 2 - Compiling and Linking...\n\n"
make -f toolchain\makefile.mak all
if ERRORLEVEL 1 ( call:errorhandle "Compiling and Linking" )

printf "\nSTEP 3 - Generating initrd disk ...\n\n"

del iso\initrd.img 2>NUL
del iso\Windows\syslinux\initrd.img 2>NUL
set MODLIST=
for %%x in (build\modules\*) do set MODLIST=!MODLIST! %%x
set MODLIST=%MODLIST:~1%

for /F %%i in ('dir /b "build\modules\*.*"') do (
	toolchain\Tools\Windows\initrd_gen.exe %MODLIST%
	goto :build_step3
)
printf "The modules' folder is empty. No modules will be installed.\n"

:build_step3

printf "\n\nSTEP 4 - Creating disk (ISO9660)...\n\n"
del iso\KernelSharp.iso 2>NUL
copy iso\initrd.img iso\Windows\syslinux >NUL
copy iso\ksharp.bin iso\Windows\syslinux >NUL
toolchain\Tools\Windows\ISO9660Generator.exe 4 "%CD%\iso\KernelSharp.iso" "%CD%\iso\Windows\syslinux\isolinux.bin" true "%CD%\iso\Windows\syslinux"
if ERRORLEVEL 1 ( call:errorhandle "Creating Disk" )

@GOTO:EOF

:errorhandle
IF NOT "%1"=="" (printf "****\nERROR: %~1\n****\n")
pause
exit