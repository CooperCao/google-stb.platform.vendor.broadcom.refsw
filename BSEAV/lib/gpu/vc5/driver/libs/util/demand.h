/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef DEMAND_H
#define DEMAND_H

#include "libs/util/assert_helpers.h"

/* demand is supposed to be just like assert but it is always enabled, ie it is
 * not disabled when NDEBUG is defined. It should only be used for catastrophic,
 * unrecoverable error conditions that cannot be reported back via the api.
 */

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
