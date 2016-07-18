@cd "%~dp0"
call build.bat && call prepare_hda.bat && call toolchain/Windows/gkd-qemu-debugger.bat
