import platform, os, sys, commands
from os import listdir
from os.path import isfile, join

pwd = os.path.abspath(os.path.dirname(sys.argv[0]))
sys.path.append(pwd + "/script_sources")
from platform_inc import plat_get

plat = plat_get()

# Show help message:
if len(sys.argv) > 1:
	if(sys.argv[1] == "-h"):
		print "Usage: launcher.pyc script_name [no script file extension]\n\nAvailable scripts are: "
		script_path = pwd + "/" + plat
		script_files = [f for f in listdir(script_path) if isfile(join(script_path, f))]
		for i in range(len(script_files)):
			print str(i+1) + "> " + script_files[i].replace(".bat", "").replace(".sh", "")
		sys.exit(0)

# Run scripts:
if plat == "Debian":
	# Run Debian's scripts
	if len(sys.argv) > 1:
		os.system(pwd + "/Debian/" + sys.argv[1] + ".sh")
	else:
		os.system(pwd + "/Debian/launch_x86_64.sh")
if plat == "Windows":
	# Run Windows' scripts
	if len(sys.argv) > 1:
		os.system(pwd + "/Windows/" + sys.argv[1]+".bat")
	else:
		os.system(pwd + "/Windows/launch_x86_64.bat")
