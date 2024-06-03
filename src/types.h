#ifndef _ZNC_TYPES_H
#define _ZNC_TYPES_H
#include <stdint.h>

#if defined(__LP64__)
#define ZN64
#endif

// variable integer
#ifdef ZN64
typedef int64_t  var;
typedef uint64_t uvar;
#else
typedef int32_t  var;
typedef uint32_t uvar;
#endif

#endif // _ZNC_TYPES_H

