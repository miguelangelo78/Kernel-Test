setlocal enabledelayedexpansion enableextensions

@cd "%~dp0"
@echo off
cls

cd ..\..

printf "\nSTEP 1 - Updating Kernel Version...\n\n"
python toolchain\versioning.py
if ERRORLEVEL 1 ( call:errorhandle "Version Update" )

printf "STEP 2 - Compiling and Linking...\n\n"
make -f toolchain\makefile.mak all
if ERRORLEVEL 1 ( call:errorhandle "Compiling and Linking" )

printf "\nSTEP 3 - Generating initrd disk ...\n\n"
del iso\initrd.img 2>NUL
set MODLIST=
for %%x in (obj\modules\*) do set MODLIST=!MODLIST! %%x
set MODLIST=%MODLIST:~1%
for /F %%i in ('dir /b "obj\modules\*.*"') do (
	toolchain\Windows\Tools\initrd_gen.exe %MODLIST%
	goto :build_step4
)
printf "The modules' folder is empty. No modules will be installed.\n"

:build_step4

printf "\n\nSTEP 4 - Creating disk (ISO9660)...\n\n"
del iso\KernelSharp.iso 2>NUL
toolchain\Windows\Tools\mkisofs -o iso/KernelSharp.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -input-charset utf-8 iso/boot/syslinux iso/ksharp.bin iso/initrd.img
if ERRORLEVEL 1 ( call:errorhandle "Creating Disk" )

@GOTO:EOF

:errorhandle
IF NOT "%1"=="" (printf "****\nERROR: %~1\n****\n")
pause
exit