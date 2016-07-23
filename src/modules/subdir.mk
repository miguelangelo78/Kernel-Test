OBJS +=
MODS += \
$(BOUT)/modules/ata.mod \
$(BOUT)/modules/fpu.mod \
$(BOUT)/modules/symbol_db.mod \
$(BOUT)/modules/video_vesa.mod

$(BOUT)/modules/ata.mod: src/modules/ata.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/ata.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/modules/fpu.mod: src/modules/fpu.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/fpu.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/modules/symbol_db.mod: src/modules/symbol_db.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/symbol_db.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/modules/video_vesa.mod: src/modules/video_vesa.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/video_vesa.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
