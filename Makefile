pulpCompiler ?= gcc
pulpCompilers ?= $(pulpCompiler)

pulpArchi ?= mia
pulpArchis ?= $(pulpArchi)

ifeq '$(pulpArchi)' 'pulp4'
pulpCore=or10n4
else
ifeq '$(pulpArchi)' 'pulpevo'
pulpCore=or10n4
else
pulpCore=or10n3
endif
endif

GOMP_LIBNAME = libgomp-$(pulpArchi)-$(pulpCompiler).a

BUILD_DIR ?= $(CURDIR)/build

# include .d files (if available and only if not doing a clean)
#ifneq ($(MAKECMDGOALS),clean)
#ifneq ($(strip $(DEPS)),)
#-include $(DEPS)
#endif
#endif

ifeq '$(pulpCoreArchi)' 'riscv'
LD = riscv32-unknown-elf-gcc
CC = riscv32-unknown-elf-gcc -D__GCC__ -m32 -mtune=pulp3 -march=RV32I -Wa,-march=RV32IM -D__riscv__
AR = riscv32-unknown-elf-ar
else
ifeq '$(pulpCompiler)' 'llvm'
CC =     clang -target or1kle-elf -mcpu=$(PULP_ARCHI)
AR =     or1kle-elf-ar
else
CC =     or1kle-elf-gcc
AR =     or1kle-elf-ar
endif
endif

clean:
	rm -rf $(BUILD_DIR)

build: $(BUILD_DIR)/$(GOMP_LIBNAME)

$(BUILD_DIR)/root.o: src/root.c
	@mkdir -p `dirname $@`	
	$(CC) -O2 -g -Wextra -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -fdata-sections -ffunction-sections -O3 -DNDEBUG -Iconfig -Iinc -I$(PULP_SDK_HOME)/install/include -I$(PULP_SDK_HOME)/install/include/$(pulpArchi) -I$(PULP_SDK_HOME)/install/include/$(pulpCore) -o $@ -c $<

#PULP3_HW_BAR_ONLY

$(BUILD_DIR)/$(GOMP_LIBNAME): $(BUILD_DIR)/root.o
	$(AR) cr $@ $^

sdk.clean: clean

sdk.checkout:

sdk.build.comp: build
	mkdir -p $(PULP_SDK_HOME)/install/or1k/lib
	cp $(BUILD_DIR)/$(GOMP_LIBNAME) $(PULP_SDK_HOME)/install/$(pulpCoreArchi)/lib

sdk.header:
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare
	cp -r config/pulp3 $(PULP_SDK_HOME)/install/include/ompBare
	cp inc/*.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/*.h $(PULP_SDK_HOME)/install/include/ompBare	

sdk.build:
	for pulpArchi in $(pulpArchis); do \
		make sdk.build.comp BUILD_DIR=$(BUILD_DIR)/gcc-$$pulpArchi-riscv pulpCoreArchi=riscv pulpArchi=$$pulpArchi pulpCompiler=gcc; if [ $$? -ne 0 ]; then exit 1; fi; \
	done

	for pulpArchi in $(pulpArchis); do \
		for pulpCompiler in $(pulpCompilers); do \
			make sdk.build.comp BUILD_DIR=$(BUILD_DIR)/$$pulpCompiler-$$pulpArchi-or10n pulpCoreArchi=or10n pulpArchi=$$pulpArchi pulpCompiler=$$pulpCompiler; if [ $$? -ne 0 ]; then exit 1; fi; \
		done \
	done

.PHONY: build
