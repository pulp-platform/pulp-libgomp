PULP_LIBS = gomp

PULP_LIB_SRCS_gomp += src/root.c

PULP_LIB_CFLAGS_gomp = -Iconfig -Wall -O2 -g -Iinc -DNDEBUG

include $(PULP_SDK_HOME)/install/rules/pulp.mk

header:
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare
	cp -r config/pulp3 $(PULP_SDK_HOME)/install/include/ompBare
	cp inc/*.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/*.h $(PULP_SDK_HOME)/install/include/ompBare  
	mkdir -p $(PULP_SDK_HOME)/install/include/ompNative
	cp inc/ompNative.h $(PULP_SDK_HOME)/install/include/ompNative/omp.h
