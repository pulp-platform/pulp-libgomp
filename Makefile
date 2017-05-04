OMP_CONFIG           = -DOMP_NOWAIT_SUPPORT -DGCC_OFFLOAD_SUPPORT
PULP_LIBS            = gomp
PULP_LIB_SRCS_gomp   += root.c
PULP_LIB_CFLAGS_gomp = -Wall -O2 -g3 ${OMP_CONFIG}


-include $(PULP_SDK_HOME)/install/rules/pulp.mk

header:
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare/pulp
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare/bigpulp
	cp *.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/common/*.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/pulp/*.h $(PULP_SDK_HOME)/install/include/ompBare/pulp/
	cp config/bigpulp/*.h $(PULP_SDK_HOME)/install/include/ompBare/bigpulp/
	mkdir -p $(PULP_SDK_HOME)/install/include/ompNative
	cp ompNative.h $(PULP_SDK_HOME)/install/include/ompNative/omp.h
