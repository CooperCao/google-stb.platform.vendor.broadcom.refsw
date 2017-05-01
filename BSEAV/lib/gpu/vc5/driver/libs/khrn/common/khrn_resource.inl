/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "khrn_process.h"
#include "vcos_atomic.h"
#include "libs/platform/v3d_scheduler.h"

static inline bool khrn_resource_has_reader(khrn_resource const* resource)
{
   return resource->readers != 0;
}

static inline bool khrn_resource_has_writer(khrn_resource const* resource)
{
   return resource->writer != 0;
}

static inline bool khrn_resource_has_reader_or_writer(khrn_resource const* resource)
{
   return resource->readers != 0 || resource->writer != 0;
}

static inline bool khrn_resource_read_now_would_stall(khrn_resource* resource)
{
   return khrn_resource_has_writer(resource)
      || !v3d_scheduler_jobs_reached_state(&resource->pre_read,
            V3D_SCHED_DEPS_COMPLETED, false);
}

static inline bool khrn_resource_write_now_would_stall(khrn_resource* resource)
{
   /* Workaround to avoid needless renames due to stack side of scheduler
    * not knowing that deps have completed until they are finalised. */
   bool const call_kernel = true;

   return khrn_resource_has_reader_or_writer(resource)
      || !v3d_scheduler_jobs_reached_state(&resource->pre_write,
            V3D_SCHED_DEPS_COMPLETED, call_kernel);
}

void khrn_resource_destroy(khrn_resource* res);

static inline void khrn_resource_refinc(khrn_resource* res)
{
   assert(res != NULL);
   verif(vcos_atomic_fetch_add_uint32(&res->ref_count, 1, VCOS_MEMORY_ORDER_RELAXED) != 0);
}

static inline void khrn_resource_refdec(khrn_resource* res)
{
   if (res != NULL)
   {
      uint32_t old = vcos_atomic_fetch_sub_uint32(&res->ref_count, 1, VCOS_MEMORY_ORDER_ACQ_REL);
      assert(old != 0);
      if (old == 1)
         khrn_resource_destroy(res);
   }
}

static inline void khrn_resource_end_access(
   khrn_resource* res,
   v3d_size_t start,
   v3d_size_t length,
   khrn_access_flags_t map_flags)
{
   assert(start + length >= length);

   // Assert that data was synced to CPU.
   assert(res->synced_start <= start && start + length <= res->synced_end);

   // todo: use KHRN_ACCESS_FLUSH_EXPLICIT
   if (map_flags & KHRN_ACCESS_WRITE)
   {
      gmem_flush_mapped_range(
         res->handle,
         start,
         length);
   }
}