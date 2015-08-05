BUILD_DIR ?= $(CURDIR)/build

# include .d files (if available and only if not doing a clean)
#ifneq ($(MAKECMDGOALS),clean)
#ifneq ($(strip $(DEPS)),)
#-include $(DEPS)
#endif
#endif

ifeq '$(OR1K_TOOLCHAIN_TYPE)' 'llvm'
CC =     clang -target or1kle-elf -mcpu=$(FABRIC_ARCHI)
AR =     or1kle-elf-ar
else
CC =     or1kle-elf-gcc
AR =     or1kle-elf-ar
endif

clean:
	rm -rf $(BUILD_DIR)

build: $(BUILD_DIR)/libgomp.a

$(BUILD_DIR)/root.o: src/root.c
	@mkdir -p `dirname $@`	
	$(CC)  -MMD -MP -MF"$$(@:%.o=%.d)" -MT"$$(@:%.o=%.d)" -DPULP3_HWSW_BAR -DPULP3_HW_BAR_ONLY -O2 -Wextra -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -fdata-sections -ffunction-sections -O3 -DNDEBUG -Iconfig -Iinc -I$(PULP_SDK_HOME)/install/rtl/mia/include -o $@ -c $<

#PULP3_HW_BAR_ONLY

$(BUILD_DIR)/libgomp.a: $(BUILD_DIR)/root.o
	$(AR) cr $@ $^

sdk.clean: clean

sdk.build.comp: build
	mkdir -p $(PULP_SDK_HOME)/install/or1k/lib/$(OR1K_TOOLCHAIN_TYPE)
	cp $(BUILD_DIR)/libgomp.a $(PULP_SDK_HOME)/install/or1k/lib/$(OR1K_TOOLCHAIN_TYPE)

sdk.header:
	mkdir -p $(PULP_SDK_HOME)/install/include/omp
	cp -r config/pulp3 $(PULP_SDK_HOME)/install/include/omp
	cp inc/*.h $(PULP_SDK_HOME)/install/include/omp
	cp config/*.h $(PULP_SDK_HOME)/install/include/omp	

sdk.build: build
	for compiler in $(FABRIC_COMPILERS); do \
		make sdk.build.comp BUILD_DIR=$(BUILD_DIR)/$$compiler OR1K_TOOLCHAIN_TYPE=$$compiler; if [ $$? -ne 0 ]; then exit 1; fi; \
	done
	mkdir -p $(PULP_SDK_HOME)/rules
	cp rules/* $(PULP_SDK_HOME)/rules

.PHONY: build
