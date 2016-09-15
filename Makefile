OMP_CONFIG           = -DOMP_NOWAIT_SUPPORT
PULP_LIBS            = gomp
PULP_LIB_SRCS_gomp   += root.c
PULP_LIB_CFLAGS_gomp = -Wall -O2 -g3 ${OMP_CONFIG} -Iconfig/pulp -I.

-include $(PULP_SDK_HOME)/install/rules/pulp.mk

header:
	mkdir -p $(PULP_SDK_HOME)/install/include/ompBare
	cp *.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/common/*.h $(PULP_SDK_HOME)/install/include/ompBare
	cp config/pulp/*.h $(PULP_SDK_HOME)/install/include/ompBare
	mkdir -p $(PULP_SDK_HOME)/install/include/ompNative
	cp ompNative.h $(PULP_SDK_HOME)/install/include/ompNative/omp.h
