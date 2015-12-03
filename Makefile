PULP_LIBS = omp

PULP_LIB_SRCS_omp += src/root.c

PULP_LIB_CFLAGS_omp = -Iconfig -Wall -O2 -g -Iinc

include $(PULP_SDK_HOME)/install/rules/pulp.mk

header:
