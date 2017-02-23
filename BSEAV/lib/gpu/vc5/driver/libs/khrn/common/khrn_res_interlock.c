/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/
#include "khrn_res_interlock.h"

// either allocate a new gmem buffer, or wait for until to be completed
static gmem_handle_t try_alloc_until(
   size_t size,
   v3d_size_t align,
   gmem_usage_flags_t usage_flags,
   const char *desc,
   v3d_scheduler_deps* until)
{
   for (; ; )
   {
      gmem_handle_t handle = gmem_alloc(size, align, usage_flags, desc);
      if (handle != NULL)
         return handle;

      // Give up if no jobs completed.
      if (!v3d_scheduler_wait_any())
         return NULL;

      // Stop if dependencies are met.
      if (until && v3d_scheduler_jobs_reached_state(until, V3D_SCHED_DEPS_COMPLETED, true))
         return NULL;
   }
}

static KHRN_RES_INTERLOCK_T* rename_for_write_now(
   KHRN_RES_INTERLOCK_T* res,
   v3d_size_t start,
   v3d_size_t length,
   uint32_t map_flags)
{
   v3d_size_t size = gmem_get_size(res->handle);

   // If not overwriting the whole resource, then we'll need to wait for CPU read.
   bool discard_all = ((map_flags & KHRN_MAP_INVALIDATE_BUFFER_BIT) != 0)
                   || ((map_flags & KHRN_MAP_INVALIDATE_RANGE_BIT) != 0 && length == size);
   if (!discard_all)
      khrn_interlock_read_now(&res->interlock);

   // If a write wouldn't cause a stall, no need to rename.
   if (!khrn_interlock_write_now_would_stall(&res->interlock))
      return NULL;

   // If rename buffer can't be allocated, then give up and stall.
   gmem_handle_t handle = try_alloc_until(
      size,
      res->align,
      gmem_get_usage(res->handle),
      gmem_get_desc(res->handle),
      !khrn_interlock_has_reader_or_writer(&res->interlock) ? &res->interlock.pre_write : NULL);
   if (!handle)
      return NULL;

   // Create new interlock object.
   KHRN_RES_INTERLOCK_T* rename = khrn_res_interlock_create_no_handle();
   if (!rename)
   {
      gmem_free(handle);
      return NULL;
   }
   rename->handle = handle;
   rename->align = res->align;

   // If rename buffer can't be mapped, then give up and stall.
   char* dst = (char*)gmem_map_and_get_ptr(rename->handle);
   if (!dst)
      goto error;

   // Copy old data into rename buffer if required.
   if (!discard_all)
   {
      char* src = (char*)gmem_map_and_get_ptr(res->handle);
      if (!src)
         goto error;

      v3d_size_t end = start + length;

      khrn_res_interlock_invalidate_mapped_range(res, 0, size);
      if (map_flags & KHRN_MAP_INVALIDATE_RANGE_BIT)
      {
         // Copy data from start of buffer to offset.
         if (start > 0)
            memcpy(dst, src, start);

         // Copy data from offset+length to end of buffer.
         if (end < size)
            memcpy(dst + end, src + end, size - end);
      }
      else
      {
         // Copy all the data to rename.
         memcpy(dst, src, size);
      }

      // No need to sync the write-range now as this will happen in post-access.
      if (start > 0)
         gmem_flush_mapped_range(rename->handle, 0, start);
      if (end < size)
         gmem_flush_mapped_range(rename->handle, end, size - end);
   }

   return rename;

error:
   khrn_res_interlock_refdec(rename);
   return NULL;
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_no_handle(void)
{
   KHRN_RES_INTERLOCK_T* res = (KHRN_RES_INTERLOCK_T*)malloc(sizeof(KHRN_RES_INTERLOCK_T));
   if (!res)
      return NULL;

   khrn_interlock_init(&res->interlock);

   res->handle = NULL;
   res->ref_count = 1;
   res->align = 0;
   res->synced_start = 0u;
   res->synced_end = ~(v3d_size_t)0u;
   return res;
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create(
   size_t size,
   v3d_size_t align,
   gmem_usage_flags_t usage_flags,
   const char *desc)
{
   KHRN_RES_INTERLOCK_T* res = khrn_res_interlock_create_no_handle();
   if (res != NULL)
   {
      if (khrn_res_interlock_alloc(res, size, align, usage_flags, desc))
         return res;

      khrn_res_interlock_refdec(res);
   }
   return NULL;
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_with_handle(gmem_handle_t handle)
{
   KHRN_RES_INTERLOCK_T* res = khrn_res_interlock_create_no_handle();
   if (res != NULL)
   {
      res->handle = handle;
      khrn_res_interlock_invalidate_synced_range(res);
      return res;
   }
   return NULL;
}

bool khrn_res_interlock_alloc(KHRN_RES_INTERLOCK_T* res, size_t size,
      v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   assert(res->handle == NULL);

   gmem_handle_t handle = try_alloc_until(size, align, usage_flags, desc, NULL);
   if (!handle)
      return false;

   res->handle = handle;
   res->align = align;
   return true;
}

void khrn_res_interlock_destroy(KHRN_RES_INTERLOCK_T *res)
{
   assert(res->ref_count == 0);

   if (res->handle != NULL)
   {
      assert(!khrn_interlock_has_reader_or_writer(&res->interlock));

      /* We cannot free the handle right now, we need to wait for the jobs
       * that are using this resource to complete and for their completion
       * callbacks to be run.
       */
      v3d_scheduler_gmem_deferred_free(&res->interlock.pre_write, res->handle);
   }
   free(res);
}

void khrn_res_interlock_invalidate_mapped_range(KHRN_RES_INTERLOCK_T* res, v3d_size_t start, v3d_size_t length)
{
   v3d_size_t end = start + length;
   assert(end > start);

   // Avoid small syncs.
   v3d_size_t const sync_granularity = 1024u;
   start = gfx_zround_down_p2(start, sync_granularity);
   end = gfx_zmin(gfx_zround_up_p2(end, sync_granularity), gmem_get_size(res->handle));

   if (res->synced_end <= res->synced_start)
   {
      gmem_invalidate_mapped_range(res->handle, start, end - start);
      res->synced_start = start;
      res->synced_end = end;
   }
   else
   {
      if (start < res->synced_start)
      {
         gmem_invalidate_mapped_range(res->handle, start, res->synced_start - start);
         res->synced_start = start;
      }

      if (end > res->synced_end)
      {
         gmem_invalidate_mapped_range(res->handle, res->synced_end, end - res->synced_end);
         res->synced_end = end;
      }
   }
}

void* khrn_res_interlock_pre_cpu_access_now(
   KHRN_RES_INTERLOCK_T** res_ptr,
   v3d_size_t start,
   v3d_size_t length,
   uint32_t map_flags)
{
   KHRN_RES_INTERLOCK_T* res = *res_ptr;
   if (map_flags & KHRN_MAP_WRITE_BIT)
   {
      // Synchronise write if MAP_UNSYNCHRONIZED_BIT is not set,
      // or if there are pending device writes.
      bool synchronise = !(map_flags & KHRN_MAP_UNSYNCHRONIZED_BIT)
                      || khrn_interlock_read_now_would_stall(&res->interlock);

      if (synchronise)
      {
         if (res->align != 0) // Don't rename external allocations.
         {
            KHRN_RES_INTERLOCK_T* rename = rename_for_write_now(res, start, length, map_flags);
            if (rename)
            {
               khrn_res_interlock_refdec(res);
               *res_ptr = rename;
               return (char*)gmem_get_ptr(rename->handle) + start;
            }
         }

         khrn_interlock_write_now(&res->interlock, KHRN_INTERLOCK_PARTS_ALL);
      }
      else
      {
         khrn_interlock_write_now_unsynchronised(&res->interlock, KHRN_INTERLOCK_PARTS_ALL);
      }
   }
   else
   {
      // Always synchronise reads.
      khrn_interlock_read_now(&res->interlock);
   }

   void* ptr = gmem_map_and_get_ptr(res->handle);
   if (!ptr)
      return NULL;

   khrn_res_interlock_invalidate_mapped_range(res, start, length);
   return (char*)ptr + start;
}

void khrn_res_interlock_post_cpu_access(
   KHRN_RES_INTERLOCK_T* res,
   v3d_size_t start,
   v3d_size_t length,
   uint32_t map_flags)
{
   assert(start + length >= length);

   // Assert that data was synced to CPU.
   assert(res->synced_start <= start && start + length <= res->synced_end);

   // todo: use KHRN_MAP_FLUSH_EXPLICIT_BIT
   if (map_flags & KHRN_MAP_WRITE_BIT)
   {
      gmem_flush_mapped_range(
         res->handle,
         start,
         length);
   }
}
