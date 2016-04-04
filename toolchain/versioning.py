import os, sys, re, getpass, socket

version_obj_file = "build/version.o"
version_src_file = "src/version/version.cpp"
version_header_file = "src/version/version.h"

if os.path.exists(version_obj_file):
	os.remove(version_obj_file)

if os.path.exists(version_src_file) and os.path.exists(version_header_file):
	# Update Version:
	with open(version_src_file, 'r+') as ver_src:
		whole_file = ver_src.read()
		
		# Updating lower version: 
		lower_match = re.search(r'ver_kernel_lower(?:.+?)?=(.+?);', whole_file, re.M)
		if(lower_match):
			whole_file = re.sub(r'ver_kernel_lower(?:.+?)?=.+?;','ver_kernel_lower = '+str(int(lower_match.group(1))+1)+';', whole_file)
		
		# Updating user (who is building the Kernel): 
		builtby_match = re.search(r'ver_kernel_builtby(?:.+?)?=(.+?);', whole_file, re.M)
		if(builtby_match):
			whole_file = re.sub(r'ver_kernel_builtby(?:.+?)?=.+?;','ver_kernel_builtby = (char*)"'+getpass.getuser()+'@'+socket.gethostname()+'";', whole_file)
	
		ver_src.seek(0)
		ver_src.write(whole_file)
