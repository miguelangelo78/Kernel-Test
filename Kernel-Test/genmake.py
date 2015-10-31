import os, fnmatch, re
cls = lambda: os.system('cls')
cls()

# File formats that will be added to the makefile project:
formats = ["c", "cpp", "s", "asm"]

# These are the compilers which will be used. In this script, it'll be adjusted for OSDeving purpose
compiler_cpp = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-g++.exe"
compiler_c = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-gcc.exe"
assembler = "$(TOOLCH)\Tools\Cross\i686-elf\\bin\i686-elf-as.exe"
assembler_nasm = "$(TOOLCH)\Tools\NASM\\nasm.exe"

# Makefile's data: (this data should be inputted through the user, but this time it'll be constant for my OSDeving)
top_path = "src" # The top path where the source files are
build_path = "build" # Where the object files will go
runnable_path = "iso"
main_make_path = "toolchain" # Where the final makefile.mak will go

execname = "ksharp.bin" # Final executable/file that will be outputted by the linker
linker = "linker.ld" # The linker that will be used while linking the final binary file
cflags = "-T $(TOOLCH)\$(LINKER) -nostartfiles -nostdlib -Wall -Wno-unknown-pragmas" # C/C++ flags
asmflags = ""
nasmflags = "-g -f elf" # Flags for the nasm assembler

# Special constants for OSDeving:
stage2obj = "$(BOUT)\ksharp_stage2.o" # This will be linked with the kernel and is built with NASM

# Parses one source file and injects into the makefile some flags, dependencies or something else that the programmer wants
def parse_sourcefile(source_content):
	inj_flags = ""
	inj_deps = ""
	inj_misc = ""
	
	# Search for flag injection:
	match_flags = re.search(r'\$FLAGS\(((?:.|\n)+?)\)', source_content, re.M)
	if match_flags:
		inj_flags = match_flags.group(1)
	match_deps = re.search(r'\$DEPS\(((?:.|\n)+?)\)', source_content, re.M)
	if match_deps:
		inj_deps = match_deps.group(1)
	match_misc = re.search(r'\$INJ\(((?:.|\n)+?)\)', source_content, re.M)
	if match_misc:
		inj_misc = match_misc.group(1)
	return [inj_flags, inj_deps, inj_misc] # Injection of: flags, dependencies (objects) and misc (respectively)

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
		deps = ""
		injection = ""
		for ffile in dir:
			# Parse the source file and add custom flags
			# to this makefile (if any match is found):
			src_file_content = open(ffile)
			src_file_meta = parse_sourcefile(src_file_content.read())
			src_file_content.close()

			# Collect the metadata and inject it into the subdir.mk
			customflags = src_file_meta[0]
			deps = src_file_meta[1]
			injection = src_file_meta[2]

			#Decide what compiler/assembler/other tool to use for this file:
			fformat = ffile[ffile.index('.'):]
			toolname = "Cross i686-elf GCC Compiler"
			compiler_to_use = "CXX" # C++ by default
			flags_to_use = "CFLAGS"
			is_asm = "-c" # If the file is an assembly file then we must remove the '-c' option, which will only work for C/C++
			if fformat == '.c':
				compiler_to_use = "CC"
			elif fformat == '.s' or fformat == '.S':
				toolname = "Cross i686-elf GCC Assembler"
				compiler_to_use = 'AS'
				flags_to_use = 'ASFLAGS'
				is_asm = ""
			elif fformat == '.asm' or fformat == '.ASM':
				toolname = "NASM Assembler"
				compiler_to_use = 'NAS'
				flags_to_use = 'NASFLAGS'
				is_asm = ""

			subdirmk.write('\n$(BOUT)\\'+files[i]+'.o: '+ffile+'\n\
	@echo \'>> Building file $<\'\n\
	@echo \'>> Invoking ' + toolname + '\'\n\
	$(' + compiler_to_use  + ') $(' + flags_to_use + ') ' + customflags + ' -o $@ '+is_asm+' $< '+ deps + ' '+ injection +'\n\
	@echo \'>> Finished building: $<\'\n\
	@echo \' \'\n')
			i+=1

		subdirmk.close() # Subdir.mk file done for this directory

	# Now build the main makefile.mak:
	makefile = open(main_make_path+"\\makefile.mak", "wb")
	makefile.write("# Tools, tools' path and flags\n\
TOOLCH = " + main_make_path + "\n\
LINKER = " + linker + "\n\
CXX = " + compiler_cpp + "\n\
CC = " + compiler_c + "\n\
AS = " + assembler + "\n\
NAS = " + assembler_nasm + "\n\
CFLAGS = " + cflags + "\n\n\
ASFLAGS = " + asmflags + "\n\
NASFLAGS = " + nasmflags + "\n\
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
	$(CXX) $(CFLAGS) -o $(DISKPATH)\$(KOUT) $(OBJS)\n\
	@echo '>>>> Finished building target: $@ <<<<'\n\
	@echo '----------'\n\n\
clean:\n\
	rm $(BOUT)/*.o\n")
	makefile.close()


print "**** Generating Makefile project. ****\n**** Source code's top path: '"+top_path+"'. ****\n**** Main makefile's path: '"+main_make_path+"'. ****"
gen_make(scan_tree())
print "**** Makefile project generation completed. ****"
