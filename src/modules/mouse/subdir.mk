OBJS +=
MODS += \
$(BOUT)/modules/mouse.mod

$(BOUT)/modules/mouse.mod: src/modules/mouse/mouse.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/mouse.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
