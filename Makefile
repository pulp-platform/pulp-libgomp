pulpCompiler ?= gcc
pulpCompilers ?= $(pulpCompiler)

pulpArchi ?= mia
pulpArchis ?= $(pulpArchi)

ifeq '$(pulpCoreArchi)' 'riscv'
pulpCore = riscv
FABRIC_CFLAGS += -DPULP4
else
ifeq '$(pulpArchi)' 'pulp4'
FABRIC_CFLAGS += -DPULP4
pulpCore=or10n4
else
ifeq '$(pulpArchi)' 'pulpevo'
pulpCore=or10n4
FABRIC_CFLAGS += -DPULPEVO
else
pulpCore=or10n3
FABRIC_CFLAGS += -DMIA
endif
endif
endif

GOMP_LIBNAME = libgomp-$(pulpArchi)-$(pulpCompiler)-$(pulpRtVersion).a

BUILD_DIR ?= $(CURDIR)/build

ifeq '$(pulpRtVersion)' 'profile0'
FABRIC_CFLAGS += -DPROFILE0
endif

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
CC =     clang -target or1kle-elf -mcpu=pulp3 -mabi=new
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
	$(CC) -O2 -g -Wextra -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -fdata-sections -ffunction-sections -O3 -DNDEBUG -Iconfig -Iinc -I$(PULP_SDK_HOME)/install/include -I$(PULP_SDK_HOME)/install/include/$(pulpArchi) -I$(PULP_SDK_HOME)/install/include/$(pulpCore) $(FABRIC_CFLAGS) -o $@ -c $<

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
	mkdir -p $(PULP_SDK_HOME)/install/include/ompNative
	cp inc/ompNative.h $(PULP_SDK_HOME)/install/include/ompNative/omp.h

sdk.build:
	for pulpArchi in $(pulpArchis); do \
		for pulpRtVersion in $(pulpRtVersions); do \
			make sdk.build.comp BUILD_DIR=$(BUILD_DIR)/gcc-$$pulpArchi-riscv-$$pulpRtVersion pulpCoreArchi=riscv pulpArchi=$$pulpArchi pulpCompiler=gcc pulpRtVersion=$$pulpRtVersion; if [ $$? -ne 0 ]; then exit 1; fi; \
		done \
	done

	for pulpArchi in $(pulpArchis); do \
		for pulpCompiler in $(pulpCompilers); do \
			for pulpRtVersion in $(pulpRtVersions); do \
				make sdk.build.comp BUILD_DIR=$(BUILD_DIR)/$$pulpCompiler-$$pulpArchi-or10n-$$pulpRtVersion pulpCoreArchi=or10n pulpArchi=$$pulpArchi pulpCompiler=$$pulpCompiler pulpRtVersion=$$pulpRtVersion; if [ $$? -ne 0 ]; then exit 1; fi; \
			done \
		done \
	done

.PHONY: build
