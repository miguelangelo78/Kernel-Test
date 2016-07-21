OBJS +=
MODS += \
$(BOUT)/modules/speaker.mod

$(BOUT)/modules/speaker.mod: src/modules/sound/speaker.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/speaker.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
