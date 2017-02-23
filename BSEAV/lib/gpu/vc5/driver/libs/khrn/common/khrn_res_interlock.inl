/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "vcos_atomic.h"

void khrn_res_interlock_destroy(KHRN_RES_INTERLOCK_T* res);

static inline void khrn_res_interlock_refinc(KHRN_RES_INTERLOCK_T* res)
{
   assert(res != NULL);
   verif(vcos_atomic_fetch_add_uint32(&res->ref_count, 1, VCOS_MEMORY_ORDER_RELAXED) != 0);
}

static inline void khrn_res_interlock_refdec(KHRN_RES_INTERLOCK_T* res)
{
   if (res != NULL)
   {
      uint32_t old = vcos_atomic_fetch_sub_uint32(&res->ref_count, 1, VCOS_MEMORY_ORDER_ACQ_REL);
      assert(old != 0);
      if (old == 1)
         khrn_res_interlock_destroy(res);
   }
}

static inline void khrn_res_interlock_set_handle(KHRN_RES_INTERLOCK_T *res_i, gmem_handle_t handle)
{
   assert(res_i->handle == GMEM_HANDLE_INVALID);
   res_i->handle = handle;
}

static inline void khrn_res_interlock_invalidate_synced_range(KHRN_RES_INTERLOCK_T* res)
{
   res->synced_start = ~0u;
   res->synced_end = 0;
}