#ifndef __LIB_CFG_H
#define __LIB_CFG_H

#define LIB_USE_SHARED_MUTEX 1

#ifdef LIB_USE_SHARED_MUTEX
#define LIB_USE_RING_BUFF 1
#define LIB_USE_RING_LIST 1
#endif /* LIB_USE_SHARED_MUTEX */

#endif /* __LIB_CFG_H */