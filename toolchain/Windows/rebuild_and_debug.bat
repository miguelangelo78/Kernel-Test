@cd "%~dp0"
call rebuild.bat && call toolchain/Windows/prepare_hda.bat && call toolchain/Windows/gkd-qemu-debugger.bat
