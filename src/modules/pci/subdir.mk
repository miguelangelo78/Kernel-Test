OBJS +=
MODS += \
$(BOUT)/modules/pci.mod

$(BOUT)/modules/pci.mod: src/modules/pci/pci.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/pci.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
