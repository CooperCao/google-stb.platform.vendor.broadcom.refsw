/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_BUFSTATE_H
#define GFX_BUFSTATE_H

/* See http://confluence.broadcom.com/x/ygAgAg */

typedef enum
{
   /* There is no buffer. */
   GFX_BUFSTATE_MISSING,

   /* Memory contents are undefined. No rendering has touched this buffer. */
   GFX_BUFSTATE_UNDEFINED,

   /* All we have done is fast clear the buffer. */
   GFX_BUFSTATE_CLEAR,

   /* We started with a fast clear, then did some rendering. */
   GFX_BUFSTATE_CLEAR_RW,

   /* We started with a fast clear, did some rendering, then invalidated the
    * buffer. */
   GFX_BUFSTATE_CLEAR_RW_INVALIDATED,

   /* Memory contents are not undefined. No rendering has touched this buffer. */
   GFX_BUFSTATE_MEM,

   /* The original memory contents have been read. */
   GFX_BUFSTATE_MEM_READ,

   /* We did some rendering on top of the original memory contents. */
   GFX_BUFSTATE_MEM_RW,

   /* We did some rendering on top of the original memory contents, but have
    * since invalidated the buffer. */
   GFX_BUFSTATE_MEM_RW_INVALIDATED
} gfx_bufstate_t;

static inline gfx_bufstate_t gfx_bufstate_begin(bool undefined)
{
   return undefined ? GFX_BUFSTATE_UNDEFINED : GFX_BUFSTATE_MEM;
}

static inline void gfx_bufstate_invalidate(gfx_bufstate_t *state)
{
   if (*state == GFX_BUFSTATE_MISSING)
      return;

   switch (*state)
   {
   case GFX_BUFSTATE_UNDEFINED:
   case GFX_BUFSTATE_CLEAR:
   case GFX_BUFSTATE_MEM:
      *state = GFX_BUFSTATE_UNDEFINED;
      break;
   case GFX_BUFSTATE_CLEAR_RW:
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:
      *state = GFX_BUFSTATE_CLEAR_RW_INVALIDATED;
      break;
   case GFX_BUFSTATE_MEM_READ:
   case GFX_BUFSTATE_MEM_RW:
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GFX_BUFSTATE_MEM_RW_INVALIDATED;
      break;
   default:
      unreachable();
   }
}

static inline void gfx_bufstate_read(gfx_bufstate_t *state)
{
   if (*state == GFX_BUFSTATE_MISSING)
      return;

   if (*state == GFX_BUFSTATE_CLEAR)
      *state = GFX_BUFSTATE_CLEAR_RW;
   else if (*state == GFX_BUFSTATE_MEM)
      *state = GFX_BUFSTATE_MEM_READ;
}

static inline void gfx_bufstate_rw(gfx_bufstate_t *state)
{
   if (*state == GFX_BUFSTATE_MISSING)
      return;

   switch (*state)
   {
   case GFX_BUFSTATE_UNDEFINED:
   case GFX_BUFSTATE_CLEAR:
   case GFX_BUFSTATE_CLEAR_RW:
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:
      *state = GFX_BUFSTATE_CLEAR_RW;
      break;
   case GFX_BUFSTATE_MEM:
   case GFX_BUFSTATE_MEM_READ:
   case GFX_BUFSTATE_MEM_RW:
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GFX_BUFSTATE_MEM_RW;
      break;
   default:
      unreachable();
   }
}

static inline bool gfx_bufstate_is_undefined(gfx_bufstate_t state)
{
   if (state == GFX_BUFSTATE_MISSING)
      return true;

   return (state == GFX_BUFSTATE_UNDEFINED) ||
          (state == GFX_BUFSTATE_CLEAR_RW_INVALIDATED) ||
          (state == GFX_BUFSTATE_MEM_RW_INVALIDATED);
}

/* Can we discard the bin CL without impacting the final contents of *this*
 * buffer? */
static inline bool gfx_bufstate_can_discard_cl(
   gfx_bufstate_t state, bool clear, bool partial)
{
   if (state == GFX_BUFSTATE_MISSING)
      return true;

   if (clear && !partial)
      return true;

   switch (state)
   {
   case GFX_BUFSTATE_UNDEFINED:
   case GFX_BUFSTATE_CLEAR: /* Fast clear is not part of bin CL */
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GFX_BUFSTATE_MEM:
   case GFX_BUFSTATE_MEM_READ:
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:
      return true;
   case GFX_BUFSTATE_CLEAR_RW:
   case GFX_BUFSTATE_MEM_RW:
      return false;
   default:
      unreachable();
      return false;
   }
}

static inline void gfx_bufstate_discard_cl(gfx_bufstate_t *state)
{
   if (*state == GFX_BUFSTATE_MISSING)
      return;

   /* We're throwing the bin CL away, which means we're confident that any
    * rendering it was going to do is irrelevant. This essentially means that
    * we can ignore reads and turn writes into invalidates... */
   switch (*state)
   {
   case GFX_BUFSTATE_UNDEFINED:
   case GFX_BUFSTATE_CLEAR_RW:
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GFX_BUFSTATE_MEM_RW:
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:
      *state = GFX_BUFSTATE_UNDEFINED;
      break;
   case GFX_BUFSTATE_CLEAR:
      /* Fast clear is not part of bin CL, so won't get discarded. */
      break;
   case GFX_BUFSTATE_MEM:
   case GFX_BUFSTATE_MEM_READ:
      *state = GFX_BUFSTATE_MEM;
      break;
   default:
      unreachable();
   }
}

// Returns true if hw clear will work (in that case, the bufstate is updated),
// false when hw clearing was not possible and fallback with rectangle clear
// should be used
static inline bool gfx_bufstate_try_fast_clear(gfx_bufstate_t *state, bool partial)
{
   if (*state == GFX_BUFSTATE_MISSING)
      return true;

   switch (*state)
   {
   case GFX_BUFSTATE_CLEAR:
   case GFX_BUFSTATE_MEM:
      if (partial)
         return false;
      /* Fall through... */
   case GFX_BUFSTATE_UNDEFINED: /* Promote partial clear to full clear */
      *state = GFX_BUFSTATE_CLEAR;
      return true;
   case GFX_BUFSTATE_CLEAR_RW:
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:
   case GFX_BUFSTATE_MEM_READ:
   case GFX_BUFSTATE_MEM_RW:
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:
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
} GFX_BUFSTATE_FLUSH_T;

static inline void gfx_bufstate_flush(
   GFX_BUFSTATE_FLUSH_T *f, gfx_bufstate_t state)
{
   f->load = (state == GFX_BUFSTATE_MEM_READ) ||
             (state == GFX_BUFSTATE_MEM_RW) ||
             (state == GFX_BUFSTATE_MEM_RW_INVALIDATED);
   f->store = (state == GFX_BUFSTATE_CLEAR) ||
              (state == GFX_BUFSTATE_CLEAR_RW) ||
              (state == GFX_BUFSTATE_MEM_RW);
   f->undefined = gfx_bufstate_is_undefined(state);
}

/* Call this if the buffer you're going to load from isn't the same as the
 * buffer you're going to store to */
static inline void gfx_bufstate_flush_ldbuf_ne_stbuf(
   GFX_BUFSTATE_FLUSH_T *f, gfx_bufstate_t state)
{
   gfx_bufstate_flush(f, state);

   if ((state == GFX_BUFSTATE_MEM) ||
      (state == GFX_BUFSTATE_MEM_READ))
   {
      /* Need to load & store to copy from load buf into store buf */
      f->load = true;
      f->store = true;
   }
}

static inline const char *gfx_bufstate_desc(gfx_bufstate_t state)
{
   switch (state)
   {
   case GFX_BUFSTATE_MISSING:                return "missing";
   case GFX_BUFSTATE_UNDEFINED:              return "undefined";
   case GFX_BUFSTATE_CLEAR:                  return "clear";
   case GFX_BUFSTATE_CLEAR_RW:               return "clear+rw";
   case GFX_BUFSTATE_CLEAR_RW_INVALIDATED:   return "clear+rw+invalidated";
   case GFX_BUFSTATE_MEM:                    return "mem";
   case GFX_BUFSTATE_MEM_READ:               return "mem+read";
   case GFX_BUFSTATE_MEM_RW:                 return "mem+rw";
   case GFX_BUFSTATE_MEM_RW_INVALIDATED:     return "mem+rw+invalidated";
   default:                                  unreachable(); return NULL;
   }
}

#endif
