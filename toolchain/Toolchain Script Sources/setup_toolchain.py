import platform, os, sys, commands

pwd = os.path.abspath(os.path.dirname(sys.argv[0]))

plat = platform.system()
if plat != "Windows":
	# Which Linux is this?
	plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID_LIKE.*'")[1][8:].capitalize()
	if plat == "Debian":
		print "x-terminal-emulator --noclose -e /bin/bash " + pwd + "/Debian/setup.sh"
		# Prepare toolchain for Debian:
		os.system("x-terminal-emulator --noclose -e /bin/bash " + pwd + "/Debian/setup.sh &")
else:
	# Prepare toolchain for Windows:
	os.system(pwd + "/Windows/setup.bat")
	