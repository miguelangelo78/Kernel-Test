import platform, os, sys, commands

def plat_get():
	plat = platform.system()
	if plat == "Linux":
		# Which Linux is this?
		plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID_LIKE.*'")[1][8:].capitalize()
	return plat