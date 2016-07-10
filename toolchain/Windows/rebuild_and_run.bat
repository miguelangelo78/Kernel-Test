@cd "%~dp0"
call rebuild.bat && call toolchain/Windows/prepare_hda.bat && call toolchain/Windows/launch_x86_64.bat
