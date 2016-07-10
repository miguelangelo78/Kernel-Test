@cd "%~dp0"
call build.bat && call prepare_hda.bat && call toolchain/Windows/launch_x86_64.bat
