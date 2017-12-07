/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "dglenum.h"

#include <string.h>
#include "libs/util/common.h"
#include "libs/util/assert_helpers.h"
#include <stdbool.h>
#include <stdint.h>

/* I'm avoiding including any GL header to aid portability */

typedef struct {
   const char *name;
   uint32_t value;
} DGLENUM_NAME_VALUE_T;

/* defines glenums[] */
#include "dglenum_gen.h"

uint32_t khrn_glenum_from_str_maybe(const char *name)
{
   int i;

   for (i = 0; i != countof(glenums); ++i) {
      if (!strcmp(glenums[i].name, name)) {
         return glenums[i].value;
      }
   }
   return ~0u;
}

uint32_t khrn_glenum_from_str(const char *name)
{
   uint32_t ret = khrn_glenum_from_str_maybe(name);
   assert(ret != ~0u);
   return ret;
}

const char *khrn_glenum_to_str(uint32_t value)
{
   int i;

   // GL_MAP_INVALIDATE_RANGE_BIT for 4 ?
   // GL_SYNC_FLUSH_COMMANDS_BIT for 1 ?
   switch (value)
   {
      case 1: return "GL_LINES (1)";
      case 4: return "GL_TRIANGLES";
      default: break;
   }

   for (i = 0; i != countof(glenums); ++i) {
      if (glenums[i].value == value) {
         return glenums[i].name;
      }
   }
   return "???";
}
