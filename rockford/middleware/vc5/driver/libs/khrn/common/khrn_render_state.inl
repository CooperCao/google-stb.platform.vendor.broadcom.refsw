/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
=============================================================================*/
#include "libs/util/assert_helpers.h"

static inline khrn_render_state_set_t khrn_render_state_get_bit(const void *rs)
{
   const KHRN_RENDER_STATE_T *krs = rs;
   assert(khrn_render_state_is_valid(krs));
   return 1 << (krs - render_states);
}

static inline bool khrn_render_state_set_contains(khrn_render_state_set_t set, const void *rs)
{
   khrn_render_state_set_t rs_bit = khrn_render_state_get_bit(rs);
   return (set & rs_bit) != 0;
}

static inline bool khrn_render_state_set_add(khrn_render_state_set_t *set, const void *rs)
{
   khrn_render_state_set_t rs_bit = khrn_render_state_get_bit(rs);
   bool ret = (*set & rs_bit) == 0;
   *set |= rs_bit;
   return ret;
}

static inline bool khrn_render_state_set_remove(khrn_render_state_set_t *set, const void *rs)
{
   khrn_render_state_set_t rs_bit = khrn_render_state_get_bit(rs);
   bool ret = (*set & rs_bit) != 0;
   *set &= ~rs_bit;
   return ret;
}

static inline bool khrn_render_state_set_is_subset(const khrn_render_state_set_t *b,
      const khrn_render_state_set_t *a)
{
   if ((*b & *a) == *a)
      return true;
   return false;
}

static inline void khrn_render_state_begin_flush(KHRN_RENDER_STATE_T* rs)
{
   assert(rs->flush_state == KHRN_RENDER_STATE_FLUSH_ALLOWED);
   rs->flush_state = KHRN_RENDER_STATE_FLUSHING;
}

static inline void khrn_render_state_disallow_flush(KHRN_RENDER_STATE_T* rs)
{
   assert(rs->flush_state == KHRN_RENDER_STATE_FLUSH_ALLOWED);
   rs->flush_state = KHRN_RENDER_STATE_FLUSH_DISALLOWED;
}

static inline void khrn_render_state_allow_flush(KHRN_RENDER_STATE_T* rs)
{
   assert(rs->flush_state == KHRN_RENDER_STATE_FLUSH_DISALLOWED);
   rs->flush_state = KHRN_RENDER_STATE_FLUSH_ALLOWED;
}

static inline GLXX_HW_RENDER_STATE_T *khrn_render_state_get_glxx(KHRN_RENDER_STATE_T *rs)
{
   static_assrt(offsetof(KHRN_RENDER_STATE_T, data.glxx) == 0);
   assert(rs->type == KHRN_RENDER_STATE_TYPE_GLXX);
   return &rs->data.glxx;
}


static inline glxx_compute_render_state* khrn_render_state_get_glxx_compute(KHRN_RENDER_STATE_T* rs)
{
   static_assrt(offsetof(KHRN_RENDER_STATE_T, data.glxx_compute) == 0);
   assert(rs->type == KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE);
   return &rs->data.glxx_compute;
}