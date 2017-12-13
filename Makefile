PULP_PROPERTIES += pulp_chip
include $(PULP_SDK_HOME)/install/rules/pulp_properties.mk

OMP_CONFIG           = -DOMP_NOWAIT_SUPPORT
PULP_LIBS            = gomp
PULP_LIB_CL_SRCS_gomp   += root.c
PULP_CL_CFLAGS_gomp = -Wall -O2 -g ${OMP_CONFIG}

ifeq '$(pulp_chip)' 'bigpulp'
PULP_CL_CFLAGS_gomp += -I$(CURDIR)/config/bigpulp
else
PULP_CL_CFLAGS_gomp += -I$(CURDIR)/config/pulp
endif

-include $(PULP_SDK_HOME)/install/rules/pulp.mk

header::
	mkdir -p $(PULP_SDK_HOME)/install/include/libgomp
	mkdir -p $(PULP_SDK_HOME)/install/include/libgomp/pulp
	mkdir -p $(PULP_SDK_HOME)/install/include/libgomp/bigpulp
	cp *.h $(PULP_SDK_HOME)/install/include/libgomp
	cp config/common/*.h $(PULP_SDK_HOME)/install/include/libgomp
	cp config/pulp/*.h $(PULP_SDK_HOME)/install/include/libgomp/pulp/
	cp config/bigpulp/*.h $(PULP_SDK_HOME)/install/include/libgomp/bigpulp/
