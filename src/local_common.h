#ifndef LOCAL_COMMON_H
#define LOCAL_COMMON_H

#include <stdbool.h>

#define DEBUG

// wstatus is the exec'ed program return code, which we only allow 16 bits
// so as to also have exec routine error codes
#define WSTATUS_MASK ((1 << 16) - 1)

typedef unsigned int uint;


#endif
