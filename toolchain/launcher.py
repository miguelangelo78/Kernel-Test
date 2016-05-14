import platform, os, sys, commands

if len(sys.argv) > 1:
	if(sys.argv[1] == "-h"):
		print "Usage: python launcher.py script_name   [no script file extension]"
		sys.exit(0)

pwd = os.path.abspath(os.path.dirname(sys.argv[0]))

plat = platform.system()
if plat != "Windows":
	# Which Linux is this?
	plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID_LIKE.*'")[1][8:].capitalize()
	if plat == "Debian":
		# Run Debian's scripts
		if len(sys.argv) > 1:
			os.system(pwd + "/Debian/"+sys.argv[1]+".sh")
		else:
			os.system(pwd + "/Debian/launch_x86_64.sh")
else:
	# Run Windows' scripts
	if len(sys.argv) > 1:
		os.system(pwd + "/Windows/"+sys.argv[1]+".bat")
	else:
		os.system(pwd + "/Windows/launch_x86_64.bat")