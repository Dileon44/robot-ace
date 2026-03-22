#ifndef __LIB_CFG_H
#define __LIB_CFG_H

#include "dbg_cfg.h"

#define LIB_USE_RAND 1

#if LIB_USE_RAND
#define LIB_USE_SHARED_MUTEX 1

#ifdef LIB_USE_SHARED_MUTEX
#define LIB_USE_RING_BUFF 1
#define LIB_USE_RING_LIST 1
#endif /* LIB_USE_SHARED_MUTEX */
#endif /* LIB_USE_RAND */

#endif /* __LIB_CFG_H */