import platform, os, sys, commands
pwd = os.path.abspath(os.path.dirname(sys.argv[0]))
sys.path.append(pwd + "/script_sources")
from platform_inc import plat_get

plat = plat_get()
if plat == "Windows":
	# Prepare toolchain for Windows:
	os.system(pwd + "/Windows/setup.bat")
if plat == "Debian":
	# Prepare toolchain for Debian:
	os.system("x-terminal-emulator --noclose -e /bin/bash " + pwd + "/Debian/setup.sh &")
