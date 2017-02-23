/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Render state handler

FILE DESCRIPTION
Handles allocation and flushing of render states.
=============================================================================*/

#include "khrn_int_common.h"
#include "khrn_render_state.h"
#include "khrn_types.h"
#include "../glxx/glxx_hw.h"
#include "../glxx/glxx_inner.h"
#include "../glxx/glxx_compute.h"
#include <stddef.h>

KHRN_RENDER_STATE_T render_states[MAX_RENDER_STATES];

static KHRN_RENDER_STATE_T *init_render_state(KHRN_RENDER_STATE_T *rs,
      khrn_render_state_type_t type)
{
   memset(rs, 0, sizeof *rs);
   rs->type = type;
   return rs;
}

bool khrn_render_state_is_valid(const KHRN_RENDER_STATE_T *rs)
{
   uintptr_t offset = (uintptr_t)rs - (uintptr_t)&render_states[0];
   return   (offset / sizeof(KHRN_RENDER_STATE_T)) < MAX_RENDER_STATES
         && (offset % sizeof(KHRN_RENDER_STATE_T)) == 0;
}

static KHRN_RENDER_STATE_T* find_oldest_flushable_rs()
{
   KHRN_RENDER_STATE_T *oldest_rs = NULL;

   for (unsigned i = 0; i < MAX_RENDER_STATES; i++)
   {
      KHRN_RENDER_STATE_T *rs = render_states + i;
      if (  rs->type != KHRN_RENDER_STATE_TYPE_NONE
         && rs->flush_state == KHRN_RENDER_STATE_FLUSH_ALLOWED)
      {
         if (oldest_rs == NULL || rs->allocated_order < oldest_rs->allocated_order)
            oldest_rs = rs;
      }
   }
   return oldest_rs;
}

bool khrn_render_state_flush_oldest_possible(void)
{
   KHRN_RENDER_STATE_T *oldest_rs = find_oldest_flushable_rs();
   if (oldest_rs)
   {
      khrn_render_state_flush(oldest_rs);
      return true;
   }
   return false;
}

KHRN_RENDER_STATE_T *khrn_render_state_new(khrn_render_state_type_t type)
{
   KHRN_RENDER_STATE_T *rs;
   static unsigned allocated_order = 0;

   assert(type != KHRN_RENDER_STATE_TYPE_NONE);

   /* Eventually this will wrap, but not very harmfully */
   ++allocated_order;

   for (unsigned i = 0; i < MAX_RENDER_STATES; i++)
   {
      rs = render_states + i;

      if (rs->type == KHRN_RENDER_STATE_TYPE_NONE)
      {
         init_render_state(rs, type);
         rs->allocated_order = allocated_order;
         return rs;
      }
   }

   /* No free ones, so evict the one we allocated the longest ago */
   rs = find_oldest_flushable_rs();
   assert(rs);
   khrn_render_state_flush(rs);
   init_render_state(rs, type);
   rs->allocated_order = allocated_order;
   return rs;
}

void khrn_render_state_flush(KHRN_RENDER_STATE_T *rs)
{
   assert(khrn_render_state_is_valid(rs));
   switch (rs->type)
   {
   case KHRN_RENDER_STATE_TYPE_GLXX:
      glxx_hw_render_state_flush(&rs->data.glxx);
      break;
   case KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE:
      glxx_compute_render_state_flush(&rs->data.glxx_compute);
      break;
   default:
      unreachable();
   }
}

void khrn_render_state_flush_all(void)
{
   for (unsigned i = 0; i < MAX_RENDER_STATES; i++)
   {
      KHRN_RENDER_STATE_T *rs = render_states + i;
      if (rs->type == KHRN_RENDER_STATE_TYPE_GLXX)
         khrn_render_state_flush(rs);
   }
}

KHRN_RENDER_STATE_T *khrn_render_state_set_pop(khrn_render_state_set_t *set)
{
   if (!*set)
      return NULL;

   unsigned index = gfx_msb(*set);
   *set &= ~(1 << index);

   return &render_states[index];
}

void khrn_render_state_delete(KHRN_RENDER_STATE_T *rs)
{
   if (rs == NULL) return;

   assert(khrn_render_state_is_valid(rs));
   init_render_state(rs, KHRN_RENDER_STATE_TYPE_NONE);
}

bool khrn_render_state_record_fence_to_signal(KHRN_RENDER_STATE_T *rs,
      KHRN_FENCE_T *fence)
{
   assert(khrn_render_state_is_valid(rs));

   khrn_fmem* fmem;
   switch (rs->type)
   {
   case KHRN_RENDER_STATE_TYPE_GLXX:
      fmem = &rs->data.glxx.fmem;
      break;
   case KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE:
      fmem = &rs->data.glxx_compute.fmem;
      break;
   default:
      unreachable();
   }
   return khrn_fmem_record_fence_to_signal(fmem, fence);
}


/* For debugging */
unsigned khrn_hw_render_state_allocated_order(const GLXX_HW_RENDER_STATE_T *hw_rs)
{
   const KHRN_RENDER_STATE_T *rs;

   if (hw_rs == NULL)
      return 0;

   rs = (const KHRN_RENDER_STATE_T *)hw_rs;
   return rs->allocated_order;
}

void glxx_hw_render_state_foreach(glxx_hw_rs_foreach_callback callback, void *data)
{
   unsigned i;

   for (i = 0; i < MAX_RENDER_STATES; i++)
   {
      KHRN_RENDER_STATE_T *rs = render_states + i;
      if (rs->type == KHRN_RENDER_STATE_TYPE_GLXX)
      {
         GLXX_HW_RENDER_STATE_T *hw_rs = khrn_render_state_get_glxx(rs);
         if (callback(hw_rs, data))
            break;
      }
   }
}

bool khrn_render_state_set_iterate_cb(const khrn_render_state_set_t *set,
      RENDERSTATESET_CB_T cb, void *param)
{
   unsigned i;
   bool res;

   if (*set == 0)
      return true;

   for (i = 0; i < MAX_RENDER_STATES; i++)
   {
      if (*set & (1 << i))
      {
         KHRN_RENDER_STATE_T *rs = render_states + i;
         res = cb(rs, param);
         if (!res )
            return false;
      }
   }
   return true;
}
