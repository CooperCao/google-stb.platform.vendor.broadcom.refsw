/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_int_common.h"
#include "khrn_render_state.h"
#include "khrn_types.h"
#include "../glxx/glxx_hw.h"
#include "../glxx/glxx_inner.h"
#include "../glxx/glxx_compute.h"
#include <stddef.h>

khrn_render_state render_states[MAX_RENDER_STATES];

static khrn_render_state *init_render_state(khrn_render_state *rs,
      khrn_render_state_type_t type)
{
   memset(rs, 0, sizeof *rs);
   rs->type = type;
   return rs;
}

bool khrn_render_state_is_valid(const khrn_render_state *rs)
{
   uintptr_t offset = (uintptr_t)rs - (uintptr_t)&render_states[0];
   return   (offset / sizeof(khrn_render_state)) < MAX_RENDER_STATES
         && (offset % sizeof(khrn_render_state)) == 0;
}

static khrn_render_state* find_oldest_flushable_rs()
{
   khrn_render_state *oldest_rs = NULL;

   for (unsigned i = 0; i < MAX_RENDER_STATES; i++)
   {
      khrn_render_state *rs = render_states + i;
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
   khrn_render_state *oldest_rs = find_oldest_flushable_rs();
   if (oldest_rs)
   {
      khrn_render_state_flush(oldest_rs);
      return true;
   }
   return false;
}

khrn_render_state *khrn_render_state_new(khrn_render_state_type_t type)
{
   khrn_render_state *rs;
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

void khrn_render_state_flush(khrn_render_state *rs)
{
   assert(khrn_render_state_is_valid(rs));
   switch (rs->type)
   {
   case KHRN_RENDER_STATE_TYPE_GLXX:
      glxx_hw_render_state_flush(&rs->data.glxx);
      break;
 #if V3D_VER_AT_LEAST(3,3,0,0)
   case KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE:
      glxx_compute_render_state_flush(&rs->data.glxx_compute);
      break;
 #endif
   default:
      unreachable();
   }
}

void khrn_render_state_flush_all(void)
{
   for (unsigned i = 0; i < MAX_RENDER_STATES; i++)
   {
      khrn_render_state *rs = render_states + i;
      if (rs->type == KHRN_RENDER_STATE_TYPE_GLXX)
         khrn_render_state_flush(rs);
   }
}

khrn_render_state *khrn_render_state_set_pop(khrn_render_state_set_t *set)
{
   if (!*set)
      return NULL;

   unsigned index = gfx_msb(*set);
   *set &= ~(1 << index);

   return &render_states[index];
}

void khrn_render_state_delete(khrn_render_state *rs)
{
   if (rs == NULL) return;

   assert(khrn_render_state_is_valid(rs));
   init_render_state(rs, KHRN_RENDER_STATE_TYPE_NONE);
}

bool khrn_render_state_record_fence_to_signal(khrn_render_state *rs,
      khrn_fence *fence)
{
   assert(khrn_render_state_is_valid(rs));

   khrn_fmem* fmem;
   switch (rs->type)
   {
   case KHRN_RENDER_STATE_TYPE_GLXX:
      fmem = &rs->data.glxx.fmem;
      break;
 #if V3D_VER_AT_LEAST(3,3,0,0)
   case KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE:
      fmem = &rs->data.glxx_compute.fmem;
      break;
 #endif
   default:
      unreachable();
   }
   return khrn_fmem_record_fence_to_signal(fmem, fence);
}


/* For debugging */
unsigned khrn_hw_render_state_allocated_order(const GLXX_HW_RENDER_STATE_T *hw_rs)
{
   const khrn_render_state *rs;

   if (hw_rs == NULL)
      return 0;

   rs = (const khrn_render_state *)hw_rs;
   return rs->allocated_order;
}

void glxx_hw_render_state_foreach(glxx_hw_rs_foreach_callback callback, void *data)
{
   unsigned i;

   for (i = 0; i < MAX_RENDER_STATES; i++)
   {
      khrn_render_state *rs = render_states + i;
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
         khrn_render_state *rs = render_states + i;
         res = cb(rs, param);
         if (!res )
            return false;
      }
   }
   return true;
}