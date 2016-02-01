/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Parallel execution utilities

FILE DESCRIPTION
Miscellaneous parallel utility functions.
=============================================================================*/

#ifndef KHRN_INT_PARALLEL_H
#define KHRN_INT_PARALLEL_H

#include <ctype.h>
#include <float.h>
#include <math.h>

#include "interface/khronos/common/khrn_int_common.h"

typedef struct
{
   void      *args;      /* Just an opaque pointer              */
   uint32_t  part;       /* Which particular part is this       */
   uint32_t  numParts;   /* How many parts are there?           */
} ABSTRACT_SERVER_ARGS_T;

typedef bool (*ABSTRACT_SERVER_FUNC_T)(ABSTRACT_SERVER_ARGS_T *);

extern void khrn_init_thread_pool(void);
extern void khrn_term_thread_pool(void);

extern bool  khrn_par_server_job(ABSTRACT_SERVER_FUNC_T func, void *opaqueArgs);

#ifdef DISABLE_PARALLEL_FUNCS

#define khrn_par_num_workers() (1)
#define khrn_par_memcpy(destination, source, num) memcpy((destination), (source), (num))
#define khrn_par_memset(destination, val, num)    memset((destination), (val), (num))
#define khrn_par_memcmp(p1, p2, num)              memcmp((p1), (p2), (num))

#else

extern uint32_t khrn_par_num_workers(void);
extern void *khrn_par_memcpy(void *destination, const void *source, size_t num);
extern void *khrn_par_memset(void *destination, int val, size_t num);
extern int   khrn_par_memcmp(const void *p1, const void *p2, size_t num);

#endif

#endif
