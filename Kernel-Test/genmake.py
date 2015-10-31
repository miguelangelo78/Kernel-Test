import os, fnmatch
cls = lambda: os.system('cls')
cls()

# File formats that will be added to the makefile project:
formats = ["c", "cpp", "s"]

# These are the compilers which will be used. In this script, it'll be adjusted for OSDeving purpose
compiler_cpp = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-g++.exe"
compiler_cpp = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-gcc.exe"
assembler = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-as.exe"

# Makefile's data: (this data should be inputted through the user, but this time it'll be constant for my OSDeving)
top_path = "src" # The top path where the source files are
build_path = "build" # Where the object files will go
runnable_path = "iso"
main_make_path = "toolchain" # Where the final makefile.mak will go

execname = "ksharp.bin" # Final executable/file that will be outputted by the linker
linker = "linker.ld" # The linker that will be used while linking the final binary file
cflags = "-T $(TOOLCH)\$(LINKER) -nostartfiles -nostdlib -Wall -Wno-unknown-pragmas" # C/C++ flags

# Special constants for OSDeving:
stage2obj = "$(BOUT)\ksharp_stage2.o" # This will be linked with the kernel and is built with NASM

# Scans the top_path for files with formats that belong to 'formats' list
def scan_tree():
	file_matches = []
	for root, dirs, files in os.walk(top_path):
		appenddir = 0
		for format in formats:
			for file in fnmatch.filter(files, "*." + format):
				if(appenddir == 0):
					file_matches.append([])
					appenddir = 1
				file_matches[-1].append(os.path.join(root, file))
	return file_matches

# Generates a makefile and the subdir makefiles:
def gen_make(tree):
	include_list = ""
	#Build subdir.mk files:
	for dir in tree:
		# Create subdir.mk for every directory on the list
		path = dir[0][:dir[0].rfind('\\')] + "\\subdir.mk"
		include_list += "\ninclude "+path

		subdirmk = open(path, "wb")
		subdirmk.write('OBJS +=')
		# Parse every source file. Then add their entry to this specific subdir.mk file
		files = [] # File without extension nor path
		for ffile in dir:
			files.append(ffile[ffile.rfind('\\')+1:ffile.rfind('.')])
			
			# Append objects to $(OBJS):
			subdirmk.write(' \\\n$(BOUT)\\'+files[-1]+'.o')

		subdirmk.write('\n')

		# Add target:
		i = 0
		customflags = ""
		for ffile in dir:
			# TODO: Parse the source file and add custom flags
			# to this makefile (if any match is found)
			#source = open(file).read()
		
			subdirmk.write('\n$(BOUT)\\'+files[i]+'.o: '+ffile+'\n\
	@echo \'>> Building file $<\'\n\
	@echo \'>> Invoking Cross i686-elf GCC Compiler\'\n\
	$(CXX) $(CFLAGS) ' + customflags + ' -o $@ -c $<\n\
	@echo \'>> Finished building: $<\'\n\
	@echo \' \'\n')
			i+=1

		subdirmk.close()

	# Now build the main makefile.mak:
	makefile = open(main_make_path+"\\makefile.mak", "wb")
	makefile.write("# Tools, tools' path and flags\n\
TOOLCH = " + main_make_path + "\n\
LINKER = " + linker + "\n\
CXX = " + compiler_cpp + "\n\
CFLAGS = " + cflags + "\n\n\
# Output constants (filenames and paths)\n\
DISKPATH = "+runnable_path+"\n\
BOUT = "+build_path+"\n\
# Kernel filename:\n\
KOUT = "+execname+"\n\
STAGE2OBJ = "+stage2obj+"\n\n\
################# Includes #################\n\
"+include_list+"\n\n\
############### Main targets ###############\n\n\
all: kernel-link\n\n\
# Link all those subdir.mk object files into the whole Kernel:\n\
kernel-link: $(OBJS)\n\
	@echo '----------'\n\
	@echo '>>>> Linking Kernel <<<<'\n\
	@echo '>>>> Invoking: Cross i686-elf GCC Linker <<<<'\n\
	$(CXX) $(CFLAGS) -o $(DISKPATH)\$(KOUT) $(STAGE2OBJ) $(OBJS)\n\
	@echo '>>>> Finished building target: $@ <<<<'\n\
	@echo '----------'\n\n\
clean:\n\
	rm $(BOUT)/*.o\n")
	makefile.close()


print "**** Generating Makefile project. ****\n**** Source code's top path: '"+top_path+"'. ****\n**** Main makefile's path: '"+main_make_path+"'. ****"
gen_make(scan_tree())
print "**** Makefile project generation completed. ****"
