@cd "%~dp0"
@printf ">>>> Please be sure to format your USB with FAT32 before using this script <<<<\n"
@printf ">>>> Also, you must run this script as Administrator <<<<\n"
@set /p drive=Enter the USB's drive letter: 

@if "%drive%" == "c" ( @printf "ERROR: Do not use an invalid drive (like C:)\n" && pause && exit )
@if "%drive%" == "C" ( @printf "ERROR: Do not use an invalid drive (like C:)\n" && pause && exit )

@cd ..\..\iso\Windows\syslinux

copy isolinux.bin %drive%:\
copy libcom32.c32 %drive%:\
copy mboot.c32 %drive%:\
copy syslinux.cfg %drive%:\
copy syslinux.c32 %drive%:\
copy ldlinux.c32 %drive%:\
copy libutil.c32 %drive%:\
copy vesamenu.c32 %drive%:\
copy background.png %drive%:\

@cd ..\..

copy initrd.img %drive%:\
copy ksharp.bin %drive%:\

cd Windows\syslinux
syslinux.exe --mbr --active --directory / --install %drive%:


@printf "\nCongratulations. The USB %drive% is now bootable. Restart your computer to boot it.\n"

@pause