import platform, os, sys, commands

def plat_get():
	plat = platform.system()
	if plat == "Linux":
		# Which Linux is this?
		plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID_LIKE.*'")[1][8:].capitalize()
		if not plat:
			plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID=[A-Za-z]'")[1][3:].capitalize()
	return plat
