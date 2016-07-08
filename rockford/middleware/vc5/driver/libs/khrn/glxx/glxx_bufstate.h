/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "libs/util/assert_helpers.h"
#include <stdbool.h>

/* See http://confluence.broadcom.com/x/ygAgAg */

typedef enum
{
   /* There is no buffer. */
   GLXX_BUFSTATE_MISSING,

   /* Memory contents are undefined. No rendering has touched this buffer. */
   GLXX_BUFSTATE_UNDEFINED,

   /* All we have done is fast clear the buffer. */
   GLXX_BUFSTATE_CLEAR,

   /* We started with a fast clear, then did some rendering. */
   GLXX_BUFSTATE_CLEAR_RW,

   /* We started with a fast clear, did some rendering, then invalidated the
    * buffer. */
   GLXX_BUFSTATE_CLEAR_RW_INVALIDATED,

   /* Memory contents are not undefined. No rendering has touched this buffer. */
   GLXX_BUFSTATE_MEM,

   /* The original memory contents have been read. */
   GLXX_BUFSTATE_MEM_READ,

   /* We did some rendering on top of the original memory contents. */
   GLXX_BUFSTATE_MEM_RW,

   /* We did some rendering on top of the original memory contents, but have
    * since invalidated the buffer. */
   GLXX_BUFSTATE_MEM_RW_INVALIDATED
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
      *state = GLXX_BUFSTATE_CLEAR_RW;
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

/* Can we discard the bin CL without impacting the final contents of *this*
 * buffer? */
static inline bool glxx_bufstate_can_discard_cl(
   glxx_bufstate_t state, bool clear, bool partial)
{
   if (state == GLXX_BUFSTATE_MISSING)
      return true;

   if (clear && !partial)
      return true;

   switch (state)
   {
   case GLXX_BUFSTATE_UNDEFINED:
   case GLXX_BUFSTATE_CLEAR: /* Fast clear is not part of bin CL */
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GLXX_BUFSTATE_MEM:
   case GLXX_BUFSTATE_MEM_READ:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      return true;
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_MEM_RW:
      return false;
   default:
      unreachable();
      return false;
   }
}

static inline void glxx_bufstate_discard_cl(glxx_bufstate_t *state)
{
   if (*state == GLXX_BUFSTATE_MISSING)
      return;

   /* We're throwing the bin CL away, which means we're confident that any
    * rendering it was going to do is irrelevant. This essentially means that
    * we can ignore reads and turn writes into invalidates... */
   switch (*state)
   {
   case GLXX_BUFSTATE_UNDEFINED:
   case GLXX_BUFSTATE_CLEAR_RW:
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GLXX_BUFSTATE_MEM_RW:
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GLXX_BUFSTATE_UNDEFINED;
      break;
   case GLXX_BUFSTATE_CLEAR:
      /* Fast clear is not part of bin CL, so won't get discarded. */
      break;
   case GLXX_BUFSTATE_MEM:
   case GLXX_BUFSTATE_MEM_READ:
      *state = GLXX_BUFSTATE_MEM;
      break;
   default:
      unreachable();
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

typedef struct
{
   bool load, store, undefined;
} GLXX_BUFSTATE_FLUSH_T;

static inline void glxx_bufstate_flush(
   GLXX_BUFSTATE_FLUSH_T *f, glxx_bufstate_t state)
{
   f->load = (state == GLXX_BUFSTATE_MEM_READ) ||
             (state == GLXX_BUFSTATE_MEM_RW) ||
             (state == GLXX_BUFSTATE_MEM_RW_INVALIDATED);
   f->store = (state == GLXX_BUFSTATE_CLEAR) ||
              (state == GLXX_BUFSTATE_CLEAR_RW) ||
              (state == GLXX_BUFSTATE_MEM_RW);
   f->undefined = glxx_bufstate_is_undefined(state);
}

/* Call this if the buffer you're going to load from isn't the same as the
 * buffer you're going to store to */
static inline void glxx_bufstate_flush_ldbuf_ne_stbuf(
   GLXX_BUFSTATE_FLUSH_T *f, glxx_bufstate_t state)
{
   glxx_bufstate_flush(f, state);

   if ((state == GLXX_BUFSTATE_MEM) ||
      (state == GLXX_BUFSTATE_MEM_READ))
   {
      /* Need to load & store to copy from load buf into store buf */
      f->load = true;
      f->store = true;
   }
}

static inline const char *glxx_bufstate_desc(glxx_bufstate_t state)
{
   switch (state)
   {
   case GLXX_BUFSTATE_MISSING:               return "missing";
   case GLXX_BUFSTATE_UNDEFINED:             return "undefined";
   case GLXX_BUFSTATE_CLEAR:                 return "clear";
   case GLXX_BUFSTATE_CLEAR_RW:              return "clear+rw";
   case GLXX_BUFSTATE_CLEAR_RW_INVALIDATED:  return "clear+rw+invalidated";
   case GLXX_BUFSTATE_MEM:                   return "mem";
   case GLXX_BUFSTATE_MEM_READ:              return "mem+read";
   case GLXX_BUFSTATE_MEM_RW:                return "mem+rw";
   case GLXX_BUFSTATE_MEM_RW_INVALIDATED:    return "mem+rw+invalidated";
   default:                                  unreachable(); return NULL;
   }
}
