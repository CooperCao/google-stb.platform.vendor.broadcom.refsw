/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef DEMAND_H
#define DEMAND_H

#include "assert.h"

/* demand is supposed to be just like assert but it is always enabled, ie it is
 * not disabled when NDEBUG is defined. demand should not be used in production
 * code; it is intended to be used for error "handling" in test applications
 * and such where just calling abort when something goes wrong is acceptable. */

#define demand_msg(COND, ...)             \
   do                                     \
   {                                      \
      if (!(COND))                        \
         assertion_failure(__VA_ARGS__);  \
   } while (0)

#ifdef NDEBUG
#define demand(COND) demand_msg(COND, "Assertion `%s' failed.", #COND)
#else
#define demand assert
#endif

#endif
