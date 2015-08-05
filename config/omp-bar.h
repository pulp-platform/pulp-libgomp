#ifndef __OMP_BAR_H__
#define __OMP_BAR_H__

#if defined(PULP2)
#include "pulp2/bar.h"
#elif defined(PULP3_LEGACY)
#include "pulp3-legacy/bar.h"
#elif defined(PULP3)
#include "pulp3/bar.h"
#endif

#endif
