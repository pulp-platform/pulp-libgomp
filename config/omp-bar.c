#include "config.h"

#ifdef PULP2
#include "pulp2/bar.c"
#elif defined(PULP3_LEGACY)
#include "pulp3-legacy/bar.c"
#elif defined(PULP3)
#include "pulp3/bar.c"
#endif
