import fnmatch
import os
import re
import subprocess
import datetime

#################################
######## Var Declaration ########
#################################
class ProgEnt:
	genprog = '' # Generation Program struct
	gensym  = '' # Generation Symbol Table struct

progents = [] # Program Entries that will replace whatever is on the file symbol_db.c
progs = [] # Array of program filepath objects/executables/binaries
uniques = [] # Array of programs with unique names
srcfiles = [] # Array of source files so that we can compare the .o files with the .c/.cpp
ffmts = ['o','bin','mod'] # File formats that will be used on the recursive search
srcfmts = ['c','cpp','s','asm'] # 'Source file' File formats accepted
blacklist = [] # Files that are objects/executables/binaries but will be ignored
whitelist = [] # Files that are objects/executables/binaries but are special. The kernel itself is an example
#################################

###########################
######## Functions ########
###########################
def modification_date(filename):
	return os.path.getmtime(filename)

def runcmd(cmd):
	proc = subprocess.Popen(cmd, stderr = subprocess.PIPE, stdout = subprocess.PIPE, shell = True)
	(ret, err) = proc.communicate()
	proc.wait()
	return ret, err

def is_diff(file_original):
	file_clean = os.path.splitext(os.path.basename(file_original))[0]
	if file_clean in uniques:
		return 0
	uniques.append(file_clean)
	
	srcmatch = None
	for src in range(len(srcfiles)):
		if file_clean + "." in srcfiles[src]: # Match found
			ignore = 0
			for i in range(len(ffmts)):
				if "." + ffmts[i] in srcfiles[src] or ".h" in srcfiles[src]:
					ignore = 1
					break
			if not ignore:
				srcmatch = srcfiles[src]
				break
	# If no match is found, then the object file is on its own without source file
	if(srcmatch == None):
		return 0
	# Otherwise, we need to compare the modification dates between the source file and object file
	# Compare 'file_original' VS 'srcmatch'
	return modification_date(srcmatch) > modification_date(file_original)

def is_elf(filename): # Check if given file is an ELF file
	ret, err = runcmd("readelf -h " + filename)
	if err:
		return 0
	else:
		return 1

def prog_populate(prog): # Fetch all the data of this elf and write it into its own class (in the array)
	# >> Fields to populate: #
	# 1.1 - Program name
	# 1.2 - Program's absolute name
	# 1.3 - Program size
	# 1.4 - Loaded address, must be zeroed
	# 1.5 - Symbol Table Size, must be zeroed
	# 1.6 - Program Type
	# 1.7 - Assembly Source (from objdump)
	# 1.8 - C/C++ (or even .s/.asm Assembly) Source
	# >> Also, for every program... #
	# 2.1 - Symbol name
	# 2.2 - Symbol address
	# >> Each program has a variable amount of symbols, thus, an array #
	readelf, readelf_err = runcmd("readelf -a " + prog)
	objdump, objdump_err = runcmd("objdump -D " + prog)

	print prog
	return 0

def make_replacements(): # Replace the Source File's 'Replacement Area' with all the entries that we collected

	pass
###########################

#########################################
####### Fetching all Source Files #######
#########################################
# To prevent unnecessary recursion in every loop:
for root, dirnames, filenames in os.walk('.'):
	if "toolchain" in root: 
		continue
	for i in range(len(srcfmts)):
		for filename in fnmatch.filter(filenames, '*.' + srcfmts[i]):
			srcfiles.append(os.path.join(root, filename))
#########################################

#############################################
####### Capture All Files Recursively #######
#############################################
print "1> Collecting Programs ..."
for root, dirnames, filenames in os.walk('.'):
	if "toolchain" in root: 
		continue
	for i in range(len(ffmts)):
		for filename in fnmatch.filter(filenames, '*.' + ffmts[i]):
			f = os.path.join(root, filename)
			if is_elf(f) and is_diff(f):
				progs.append(f)
#############################################

#########################################
####### Populate Generation Areas #######
#########################################
print "2> Fetching the Programs' Data ..."
for i in range(len(progs)):
	prog_populate(progs[i])
#########################################

#####################################
####### Make all Replacements #######
#####################################
print "3> Making String Replacements ..."
make_replacements()
#####################################