################################################# PROLOGUE: IMPORTS AND OS DEFINITIONS #################################################
import os, fnmatch, re, platform, commands

# Determine which operating system we're using:
os_plat = platform.system()
if os_plat != "Windows":
	# Which Linux is this?
	os_plat = commands.getstatusoutput("cat /etc/os-release | grep 'ID_LIKE.*'")[1][8:].capitalize()

osp = os_plat # Alias

cls = lambda: os.system('cls' if os_plat == 'Windows' else 'clear'); cls()

LLVM_ENABLED = 1 # DEFAULT CONSTANT LLVM FLAG

################################################# STAGE ONE: TOOLCHAIN DEFINITIONS #################################################

class ToolCompiler:
	def __init__(self):
		self.make_sym = ""
		self.execpath = ""
		self.make_flagsym = ""
		self.flags = ""
		self.name = ""
		self.is_asm = -1
		self.include_path = ""

class Toolchain:
	def __init__(self):
		self.toolname = ""
		self.file_formats = [""]
		self.compiler_cpp = ToolCompiler()
		self.compiler_c = ToolCompiler()
		self.linker = ToolCompiler()
		self.assembler_gas = ToolCompiler()
		self.assembler_nasm = ToolCompiler()
		self.top_path = ""
		self.build_path = ""
		self.runnable_path = ""
		self.make_path = ""
		self.output_file = ""
		self.linker_script = ""
		self.tool_ptrs = {}

# Create GCC toolchain: ############################################################################
gccToolchain = Toolchain(); gt = gccToolchain # Alias
gt.toolname = "GCC Toolchain"
gt.file_formats = ["c", "cpp", "s", "asm"] # Case insensitive
# Setup CPP compiler:
gt.compiler_cpp.make_sym = "CXX"
gt.compiler_cpp.execpath = "$(TOOLCH)/"+osp+"/Tools/Cross/i686-elf/bin/i686-elf-g++"
gt.compiler_cpp.make_flagsym = "CPPFLAGS"
gt.compiler_cpp.name = "Cross i686-elf GCC Compiler"
gt.compiler_cpp.is_asm = 0
gt.compiler_cpp.include_path = "-Itoolchain/"+osp+"/Tools/Cross/i686-elf/lib/gcc/i686-elf/include -Isrc"
gt.compiler_cpp.flags = "-T $(TOOLCH)/$(LINKER) " + gt.compiler_cpp.include_path + " -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions"
# Setup C compiler:
gt.compiler_c.make_sym = "CC"
gt.compiler_c.execpath = "$(TOOLCH)/"+osp+"/Tools/Cross/i686-elf/bin/i686-elf-gcc"
gt.compiler_c.make_flagsym = "CFLAGS"
gt.compiler_c.name = "Cross i686-elf GCC Compiler"
gt.compiler_c.is_asm = 0
gt.compiler_c.include_path = gt.compiler_cpp.include_path
gt.compiler_c.flags = "-T $(TOOLCH)/$(LINKER) " + gt.compiler_c.include_path + " -g -finline-functions -ffreestanding -fbuiltin -Wall -Wextra"
# Setup Linker:
gt.linker.make_sym = "LD"
gt.linker.execpath = "$(TOOLCH)/"+osp+"/Tools/Cross/i686-elf/bin/i686-elf-ld"
gt.linker.make_flagsym = "LDFLAGS"
gt.linker.name = "Cross i686-elf GCC Linker"
gt.linker.is_asm = 0
gt.linker.flags = "-f elf"
# Setup GAS Assembler:
gt.assembler_gas.make_sym = "AS"
gt.assembler_gas.execpath = "$(TOOLCH)/"+osp+"/Tools/Cross/i686-elf/bin/i686-elf-as"
gt.assembler_gas.make_flagsym = "ASFLAGS"
gt.assembler_gas.name = "Cross i686-elf GCC Assembler"
gt.assembler_gas.is_asm = 1
# Setup NASM/Intel Assembler:
gt.assembler_nasm.make_sym = "NAS"
gt.assembler_nasm.execpath = "$(TOOLCH)/"+osp+"/Tools/NASM/nasm"
gt.assembler_nasm.make_flagsym = "NASFLAGS"
gt.assembler_nasm.name = "NASM Assembler"
gt.assembler_nasm.is_asm = 1
gt.assembler_nasm.flags = "-g -f elf32"
# Everything else:
gt.top_path = "src"
gt.build_path = "obj"
gt.runnable_path = "iso"
gt.make_path = "toolchain"
gt.output_file = "ksharp.bin"
gt.linker_script = "linker.ld"
# 'Pointers' to all tools/compilers related to each file format:
gt.tool_ptrs = {'c': gt.compiler_c, 'cpp': gt.compiler_cpp, 's': gt.assembler_gas, 'S': gt.assembler_gas, 'asm': gt.assembler_nasm, 'ASM': gt.assembler_nasm }

# Create LLVM toolchain: ############################################################################
llvmToolchain = Toolchain(); lt = llvmToolchain # Alias
lt.toolname = "LLVM Toolchain"
lt.file_formats = ["c", "cpp", "s", "asm", "ll", "bc"] # Case insensitive
# Setup CPP compiler:
lt.compiler_cpp.make_sym = "CXX_LLVM"
lt.compiler_cpp.execpath = "clang++" + ("-3.8" if osp != "Windows" else "")
lt.compiler_cpp.make_flagsym = "LLVMCPPFLAGS"
lt.compiler_cpp.name = "LLVM C++ Clang++"
lt.compiler_cpp.is_asm = 0
lt.compiler_cpp.include_path = "-I" + ("C:/llvm/lib/clang/3.8.0/include" if osp == "Windows" else "/usr/lib/llvm-3.8/lib/clang/3.8.0/include") + " -Isrc"
lt.compiler_cpp.flags = "-ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions " + lt.compiler_cpp.include_path
# Setup C compiler:
lt.compiler_c.make_sym = "CC_LLVM"
lt.compiler_c.execpath = "clang" + ("-3.8" if osp != "Windows" else "")
lt.compiler_c.make_flagsym = "LLVMCFLAGS"
lt.compiler_c.name = "LLVM C Clang"
lt.compiler_c.is_asm = 0
lt.compiler_c.include_path = lt.compiler_cpp.include_path
lt.compiler_c.flags = "-ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -fno-exceptions " + lt.compiler_c.include_path
# Other compilers/assemblers/linkers:
lt.linker = gt.linker
lt.assembler_gas = gt.assembler_gas
lt.assembler_nasm = gt.assembler_nasm
# Everything else:
lt.top_path = gt.top_path
lt.build_path = gt.build_path
lt.runnable_path = gt.runnable_path
lt.make_path = gt.make_path
lt.output_file = gt.output_file
lt.linker_script = gt.linker_script
# 'Pointers' to all tools/compilers related to each file format:
lt.tool_ptrs = {'c': lt.compiler_c, 'cpp': lt.compiler_cpp, 's': lt.assembler_gas, 'S': lt.assembler_gas, 'asm': lt.assembler_nasm, 'ASM': lt.assembler_nasm }

# Toggle between LLVM and GCC toolchain:
curr_tool = lt if LLVM_ENABLED else gt; ct = curr_tool # Alias

################################################# STAGE TWO: MAKEFILE GENERATOR FUNCTIONS #################################################

def toggle_llvm(use_llvm):
	global ct
	ct = lt if use_llvm else gt;
	
# Used for packing metadata and to return it on the function below (parse_injections_sourcefile)
class Metadata_injector():
	flags = ""
	deps = ""
	misc = ""
	mods = ""
	disable_llvm = LLVM_ENABLED

# Parses one source file and injects into the makefile some flags, dependencies or something else that the programmer wants
def parse_injections_sourcefile(source_content):
	meta = Metadata_injector()

	# Search for flag injection (Format: $PROPERTY(VALUE) , it matches everything + the value and puts on group 1):
	# Property list: $FLAGS(...), MODULE_DEPS(...), $INJ(...), $LLVMENABLE(1/0)
	match_flags = re.search(r'\$FLAGS\(((?:\w|\n)+?)\)', source_content, re.M)
	if match_flags:
		meta.flags = match_flags.group(1)
	match_deps = re.search(r'MODULE_DEPS\(((?:\w|\n|,| |\\|\/|\.)+?)\)', source_content, re.M)
	if match_deps:
		deps = match_deps.group(1).split(',')
		for dep in deps:
			meta.deps += "obj/" + (dep.strip()) + " "
	match_misc = re.search(r'\$INJ\(((?:\w|\n|.)+?)\)', source_content, re.M)
	if match_misc:
		meta.misc = match_misc.group(1)

	match_ismod = re.search(r'^(?!(?:.+)?(?:\/\/|\/\*))(?:.+)?MODULE_(?:DEF|EXT)\((.+)?\)', source_content, re.M)
	if match_ismod:
		meta.mods = 1
	else:
		meta.mods = 0

	match_llvm_disable = re.search(r'\$LLVMENABLE\(((?:\w|\n)+?)\)', source_content, re.M)
	if match_llvm_disable:
		try:
			meta.disable_llvm = int(match_llvm_disable.group(1)) 
		except:
			meta.disable_llvm = LLVM_ENABLED

	return meta # Injection of: flags, dependencies (objects), misc and LLVM disabling (respectively)

# Scans the top_path for files with formats that belong to 'formats' list
def scan_tree():
	file_matches = []
	for root, dirs, files in os.walk(ct.top_path):
		appenddir = 0
		for format in ct.file_formats:
			for file in fnmatch.filter(files, "*." + format):
				if(appenddir == 0):
					file_matches.append([])
					appenddir = 1
				file_matches[-1].append(os.path.join(root, file).replace("\\","/"))
	return file_matches

class ToolchainData:
	toolname="null"
	compiler_in_use="null"
	flags_in_use="null"
	is_asm = -1
	build_path = "null"

# Determins the flags, the tool names and what tools to use for each different file:
def parseFileFormat(fileformat):
	dat = ToolchainData()
	try:
		dat.compiler_in_use = ct.tool_ptrs[fileformat].make_sym
		dat.flags_in_use = ct.tool_ptrs[fileformat].make_flagsym
		dat.toolname = ct.tool_ptrs[fileformat].name
		dat.is_asm = ct.tool_ptrs[fileformat].is_asm
		dat.build_path = ct.build_path
	except: # Invalid file format
		pass

	return dat

def write_subdir_entry(subdirmk_file, toolchain, file_objname, file_path, customflags, deps, injection, ismod):
	entry_output_path = "$@"
	if ismod:
		customflags += "-r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE"
		entry_output_path = toolchain.build_path + "/modules/"+file_objname+".mod"
	
	subdirmk_file.write('\n$(BOUT)/'+('modules/' if ismod else '') + file_objname + ('.o' if not ismod else'.mod')+': ' + file_path + ' ' + deps + '\n\
	@echo \'>> Building file $<\'\n\
	@echo \'>> Invoking ' + toolchain.toolname + '\'\n\
	$(' + ( toolchain.compiler_in_use if not deps else gt.compiler_cpp.make_sym) + ') $(' + (toolchain.flags_in_use if not deps else "CPPFLAGS_MODS") + ') ' + customflags + ' -o '+entry_output_path+' '+ ('-c' if not toolchain.is_asm and not deps else '') +' $< '+ deps + ' '+ injection +'\n\
	@echo \'>> Finished building: $<\'\n\
	@echo \' \'\n')
	
# Generates a makefile and the subdir makefiles:
def gen_make(tree):
	global gt, lt
	include_list = ""
	#Build subdir.mk files:
	for dir in tree:
		# Create subdir.mk for every directory on the list
		path = dir[0][:dir[0].rfind('/')] + "/subdir.mk"
		include_list += "\ninclude " + path

		subdirmk = open(path, "wb")
		# Write obj list into the current subdir.mk:
		subdirmk.write('OBJS +=')
		files = [] # File without extension nor path
		modcount = []
		for ffile in dir:
			files.append(ffile[ffile.rfind('/')+1:ffile.rfind('.')])
			
			# Prevent this mod from being linked to the core kernel:
			if re.search(r'^(?!(?:.+)?(?:\/\/|\/\*))(?:.+)?MODULE_(?:DEF|EXT)\((.+)?\)', open(ffile).read(), re.M):
				modcount.append(ffile)
				continue
			
			# Append objects to $(OBJS):
			subdirmk.write(' \\\n$(BOUT)/' + files[-1] + '.o')
		subdirmk.write('\n')

		if len(modcount) > 0:
			subdirmk.write('MODS +=')
			for ffile in files:
				subdirmk.write(' \\\n$(BOUT)/modules/' + ffile + '.mod')
			subdirmk.write('\n')

		# Add targets:
		i = 0
		for ffile in dir:
			# Parse the source file and add custom flags
			# to this makefile (if any match is found):
			src_file_content = open(ffile)
			# Collect the metadata in order to inject into the subdir.mk:
			src_file_meta = parse_injections_sourcefile(src_file_content.read())
			src_file_content.close()

			# Override llvm flag temporarily for this subdir entry only
			toggle_llvm(src_file_meta.disable_llvm)
			
			# Decide what compiler/assembler/other tool to use for this file:
			toolchainData = parseFileFormat(ffile[ffile.index('.'):][1:])
			# Input object entry into the current subdir.mk
			write_subdir_entry(subdirmk, toolchainData, files[i], ffile, src_file_meta.flags, src_file_meta.deps, src_file_meta.misc, src_file_meta.mods)

			toggle_llvm(LLVM_ENABLED) # Restore llvm flag to default

			i+=1

		subdirmk.close() # Subdir.mk file done for this directory

	# Now build the main makefile.mak (contains a mix of both toolchains):
	makefile = open(ct.make_path + "/makefile.mak", "wb")
	makefile.write("# Tools, tools' path and flags\n\
TOOLCH = " + ct.make_path 			+ "\n\
LINKER = " + ct.linker_script 		+ "\n\
" + gt.compiler_cpp.make_sym 		+ " = " + gt.compiler_cpp.execpath		+ "\n\
" + gt.compiler_c.make_sym 			+ " = " + gt.compiler_c.execpath 		+ "\n\
" + gt.linker.make_sym 			+ " = " + gt.linker.execpath 		+ "\n\
" + lt.compiler_cpp.make_sym 		+ " = " + lt.compiler_cpp.execpath 		+ "\n\
" + lt.compiler_c.make_sym 			+ " = " + lt.compiler_c.execpath 		+ "\n\
" + ct.assembler_gas.make_sym 		+ " = " + ct.assembler_gas.execpath 	+ "\n\
" + ct.assembler_nasm.make_sym 		+ " = " + ct.assembler_nasm.execpath 	+ "\n\
" + gt.compiler_cpp.make_flagsym 	+ " = " + gt.compiler_cpp.flags 		+ "\n\
" + lt.compiler_cpp.make_flagsym 	+ " = " + lt.compiler_cpp.flags 		+ "\n\
" + gt.compiler_c.make_flagsym 		+ " = " + gt.compiler_c.flags 			+ "\n\
" + lt.compiler_c.make_flagsym 		+ " = " + lt.compiler_c.flags 			+ "\n\
" + ct.assembler_gas.make_flagsym 	+ " = " + ct.assembler_gas.flags 		+ "\n\
" + ct.assembler_nasm.make_flagsym 	+ " = " + ct.assembler_nasm.flags 		+ "\n\
CPPFLAGS_MODS = " + gt.compiler_c.include_path + " -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -lgcc -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions\n\
\n\
# Output constants (filenames and paths)\n\
DISKPATH = " + ct.runnable_path + "\n\
BOUT = " + ct.build_path + "\n\
# Kernel filename:\n\
KOUT = " + ct.output_file + "\n\
################# Includes #################\n\
" + include_list + "\n\n\
############### Main targets ###############\n\n\
all: kernel-link\n\n\
# Link all those subdir.mk object files into the whole Kernel:\n\
kernel-link: $(OBJS) $(MODS)\n\
	@echo '----------'\n\
	@echo 'Toolchain: " + ct.toolname + "'\n\
	@echo '>>>> Linking Kernel <<<<'\n\
	@echo '>>>> Invoking: LLVM C++ Linker <<<<'\n\
	"+("$(CXX_LLVM) -T $(TOOLCH)/$(LINKER) $(LLVMCPPFLAGS) -o $(DISKPATH)/$(KOUT) $(OBJS)\n" if os_plat != "Windows" else "$(CXX) $(CPPFLAGS) -o $(DISKPATH)\$(KOUT) $(OBJS)\n") + "\
	@echo '>>>> Finished building target: $@ <<<<'\n\
	@echo '----------'\n\n\
clean:\n\
	rm $(BOUT)/*.o\n\
	rm $(BOUT)/modules/*.mod\n")
	makefile.close()

################################################# STAGE THREE: MAIN SECTION #################################################

print "**** Generating Makefile project. ****\n**** Source code's top path: '"+ct.top_path+"'. ****\n**** Main makefile's path: '"+ct.make_path+"'. ****"
gen_make(scan_tree())
print "**** Makefile project generation completed. ****"
