/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/assert_helpers.h"
#include <stdbool.h>

/* See http://confluence.broadcom.com/x/ygAgAg */

typedef enum
{
   GLXX_BUFSTATE_MISSING, /* There is no buffer */

                                       /* At start       Read/written in renderstate?     At end */
   GLXX_BUFSTATE_UNDEFINED,            /* Nothing        Possibly read undefined data     Discard, invalidate resource */
   GLXX_BUFSTATE_CLEAR,                /* Fast clear     Possible read undefined data     Store */
   GLXX_BUFSTATE_CLEAR_READ,           /* Fast clear     Read                             Store */
   GLXX_BUFSTATE_CLEAR_RW,             /* Fast clear     Read & written                   Store */
   GLXX_BUFSTATE_CLEAR_RW_INVALIDATED, /* Fast clear     Read & written                   Discard, invalidate resource */
   GLXX_BUFSTATE_MEM,                  /* Nothing        No                               Discard */
   GLXX_BUFSTATE_MEM_READ,             /* Load           Read                             Discard */
   GLXX_BUFSTATE_MEM_RW,               /* Load           Read & written                   Store */
   GLXX_BUFSTATE_MEM_RW_INVALIDATED    /* Load           Read & written                   Discard, invalidate resource */
} glxx_bufstate_t;

static inline glxx_bufstate_t glxx_bufstate_begin(bool undefined)
{
   return undefined ? GLXX_BUFSTATE_UNDEFINED : GLXX_BUFSTATE_MEM;
}

static inline void glxx_bufstate_invalidate(glxx_bufstate_t *state)
{
   if (*state == GLXX_BUFSTATE_MISSING)
      return;

   switch (*state)
   {
   case GLXX_BUFSTATE_UNDEFINED:
   case GLXX_BUFSTATE_CLEAR:
   case GLXX_BUFSTATE_MEM:
      *state = GLXX_BUFSTATE_UNDEFINED;
      break;
   case GLXX_BUFSTATE_CLEAR_READ:
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
      *state = GLXX_BUFSTATE_CLEAR_RW_INVALIDATED;
      break;
   case GLXX_BUFSTATE_MEM_READ:
   case GLXX_BUFSTATE_MEM_RW:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GLXX_BUFSTATE_MEM_RW_INVALIDATED;
      break;
   default:
      unreachable();
   }
}

static inline void glxx_bufstate_read(glxx_bufstate_t *state)
{
   if (*state == GLXX_BUFSTATE_MISSING)
      return;

   if (*state == GLXX_BUFSTATE_CLEAR)
      *state = GLXX_BUFSTATE_CLEAR_READ;
   else if (*state == GLXX_BUFSTATE_MEM)
      *state = GLXX_BUFSTATE_MEM_READ;
}

static inline void glxx_bufstate_rw(glxx_bufstate_t *state)
{
   if (*state == GLXX_BUFSTATE_MISSING)
      return;

   switch (*state)
   {
   case GLXX_BUFSTATE_UNDEFINED:
   case GLXX_BUFSTATE_CLEAR:
   case GLXX_BUFSTATE_CLEAR_READ:
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
      *state = GLXX_BUFSTATE_CLEAR_RW;
      break;
   case GLXX_BUFSTATE_MEM:
   case GLXX_BUFSTATE_MEM_READ:
   case GLXX_BUFSTATE_MEM_RW:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GLXX_BUFSTATE_MEM_RW;
      break;
   default:
      unreachable();
   }
}

static inline bool glxx_bufstate_is_undefined(glxx_bufstate_t state)
{
   if (state == GLXX_BUFSTATE_MISSING)
      return true;

   return (state == GLXX_BUFSTATE_UNDEFINED) ||
          (state == GLXX_BUFSTATE_CLEAR_RW_INVALIDATED) ||
          (state == GLXX_BUFSTATE_MEM_RW_INVALIDATED);
}

/* Can we discard the render-state without impacting the final contents of *this* buffer? */
static inline bool glxx_bufstate_can_discard_rs(glxx_bufstate_t state)
{
   if (state == GLXX_BUFSTATE_MISSING)
      return true;

   switch (state)
   {
   case GLXX_BUFSTATE_UNDEFINED:
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GLXX_BUFSTATE_MEM:
   case GLXX_BUFSTATE_MEM_READ:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      return true;
   case GLXX_BUFSTATE_CLEAR:
   case GLXX_BUFSTATE_CLEAR_READ:
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_MEM_RW:
      return false;
   default:
      unreachable();
      return false;
   }
}

// Returns true if hw clear will work (in that case, the bufstate is updated),
// false when hw clearing was not possible and fallback with rectangle clear
// should be used
static inline bool glxx_bufstate_try_fast_clear(glxx_bufstate_t *state, bool partial)
{
   if (*state == GLXX_BUFSTATE_MISSING)
      return true;

   switch (*state)
   {
   case GLXX_BUFSTATE_CLEAR:
   case GLXX_BUFSTATE_MEM:
      if (partial)
         return false;
      /* Fall through... */
   case GLXX_BUFSTATE_UNDEFINED: /* Promote partial clear to full clear */
      *state = GLXX_BUFSTATE_CLEAR;
      return true;
   case GLXX_BUFSTATE_CLEAR_READ:
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GLXX_BUFSTATE_MEM_READ:
   case GLXX_BUFSTATE_MEM_RW:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      /* Can't fast clear as clear must happen after existing reads/writes... */
      return false;
   default:
      unreachable();
      return false;
   }
}

static inline bool glxx_bufstate_need_clear(glxx_bufstate_t state)
{
   return (state == GLXX_BUFSTATE_CLEAR) ||
          (state == GLXX_BUFSTATE_CLEAR_READ) ||
          (state == GLXX_BUFSTATE_CLEAR_RW) ||
          (state == GLXX_BUFSTATE_CLEAR_RW_INVALIDATED);
}

static inline bool glxx_bufstate_need_load(glxx_bufstate_t state)
{
   return (state == GLXX_BUFSTATE_MEM_READ) ||
          (state == GLXX_BUFSTATE_MEM_RW) ||
          (state == GLXX_BUFSTATE_MEM_RW_INVALIDATED);
}

static inline bool glxx_bufstate_need_store(glxx_bufstate_t state)
{
   return (state == GLXX_BUFSTATE_CLEAR) ||
          (state == GLXX_BUFSTATE_CLEAR_READ) ||
          (state == GLXX_BUFSTATE_CLEAR_RW) ||
          (state == GLXX_BUFSTATE_MEM_RW);
}

static inline const char *glxx_bufstate_desc(glxx_bufstate_t state)
{
   switch (state)
   {
   case GLXX_BUFSTATE_MISSING:               return "missing";
   case GLXX_BUFSTATE_UNDEFINED:             return "undefined";
   case GLXX_BUFSTATE_CLEAR:                 return "clear";
   case GLXX_BUFSTATE_CLEAR_READ:            return "clear+read";
   case GLXX_BUFSTATE_CLEAR_RW:              return "clear+rw";
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:  return "clear+rw+invalidated";
   case GLXX_BUFSTATE_MEM:                   return "mem";
   case GLXX_BUFSTATE_MEM_READ:              return "mem+read";
   case GLXX_BUFSTATE_MEM_RW:                return "mem+rw";
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:    return "mem+rw+invalidated";
   default:                                  unreachable(); return NULL;
   }
}
