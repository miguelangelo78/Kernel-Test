import os, sys
# coding: utf-8
"""
	Generate a symbol table from nm output.
"""
# Tools and Kernel Variables:
SYM_NM = "toolchain\\Tools\\Cross\\i686-elf\\bin\\i686-elf-nm"
KERNEL = "iso\\ksharp.bin"
NM_CMD = SYM_NM + " " + KERNEL + " -g"

def run_cmd(cmd):
	return os.popen(cmd).read()

ignore = [ "abs", "kernel_symbols_start", "kernel_symbols_end" ]
lines = [ x.strip().split(" ") for x in run_cmd(NM_CMD).split("\n") if x not in ignore ]

# Write extern + type
def extern(name):
    print ".extern %s" % (name)
    print ".type %s, @function" % (name)
    print ""

# Write an entry
def entry(name):
    print ".long %s" % (name)
    print ".asciz \"%s\"" % (name)
    print ""

# Generate the assembly
print ".section .symbols"
print ""
for name in lines:
	if len(name) > 1:
		extern(name[2])

print ".global kernel_symbols_start"
print "kernel_symbols_start:"
print ""
for name in lines:
	if len(name) > 1:
		entry(name[2])

print ".global kernel_symbols_end"
print "kernel_symbols_end:"
