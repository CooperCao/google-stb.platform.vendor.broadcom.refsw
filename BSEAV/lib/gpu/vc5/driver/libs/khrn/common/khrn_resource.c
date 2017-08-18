/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "../egl/egl_platform.h"
#include "khrn_int_common.h"
#include "khrn_resource.h"
#include "khrn_process.h"
#include "khrn_render_state.h"

#include "libs/platform/v3d_scheduler.h"

static inline uint64_t get_render_state_mask(unsigned rs_index)
{
   return gfx_mask64(KHRN_RESOURCE_BITS_PER_RS) << rs_index*KHRN_RESOURCE_BITS_PER_RS;
}

static inline unsigned get_next_render_state_index(uint64_t rs_stages)
{
   return gfx_msb64(rs_stages) / KHRN_RESOURCE_BITS_PER_RS;
}

static inline uint64_t encode_stages(khrn_stages_t stages, unsigned rs_index)
{
   return (uint64_t)stages << rs_index*KHRN_RESOURCE_BITS_PER_RS;
}

static inline khrn_stages_t decode_stages(uint64_t rs_stages, unsigned rs_index)
{
   return (khrn_stages_t)((uint32_t)(rs_stages >> rs_index*KHRN_RESOURCE_BITS_PER_RS) & gfx_mask(KHRN_RESOURCE_BITS_PER_RS));
}

static inline void verify_resource(khrn_resource* resource)
{
#ifndef NDEBUG
   // If writer..
   if (resource->writer)
   {
      // then can be only reader (and writer).
      unsigned writer_index = get_next_render_state_index(resource->writer);
      uint64_t other_mask = ~get_render_state_mask(writer_index);
      assert(!(resource->readers & other_mask));
      assert(!(resource->writer & other_mask));
   }
#endif
}

static void flush_render_states(khrn_resource* resource, uint64_t rs_stages)
{
   assert(((resource->readers | resource->writer) & rs_stages) == rs_stages);

   do
   {
      unsigned rs_index = get_next_render_state_index(rs_stages);
      khrn_render_state_flush(&render_states[rs_index]);
      assert( !((resource->readers | resource->writer) & get_render_state_mask(rs_index)) );

      // Reload from resource in case additional render-states have been flushed.
      rs_stages &= resource->readers | resource->writer;
   }
   while (rs_stages);
}

void khrn_resource_flush(khrn_resource *resource)
{
   assert(!resource->in_begin_flags);

   uint64_t users = resource->readers | resource->writer;
   if (users)
      flush_render_states(resource, users);

   assert(!resource->writer && !resource->readers);
}

void khrn_resource_flush_writer(khrn_resource *resource)
{
   assert(!resource->in_begin_flags);

   if (resource->writer)
   {
      flush_render_states(resource, resource->writer);
      assert(!resource->writer && !resource->readers);
   }
}

void khrn_resource_add_reader(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs)
{
   assert(!resource->in_begin_flags);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   uint64_t other_writer = resource->writer & ~get_render_state_mask(rs_index);
   if (other_writer)
      flush_render_states(resource, other_writer);

   resource->readers |= encode_stages(stages, rs_index);
   verify_resource(resource);
}

bool khrn_resource_add_writer(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs,
      khrn_resource_parts_t parts)
{
   assert(!resource->in_begin_flags);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   // Flush any other render-states using this resource.
   uint64_t other_users = (resource->readers | resource->writer) & ~get_render_state_mask(rs_index);
   if (other_users)
      flush_render_states(resource, other_users);

   if (~decode_stages(resource->writer, rs_index) & stages & KHRN_STAGE_BIN)
      resource->last_tf_write_count = 0;
   resource->writer |= encode_stages(stages, rs_index);
   verify_resource(resource);

   bool all_undefined = (resource->undefined_parts & parts) == parts;
   resource->undefined_parts &= ~parts;
   return all_undefined;
}

bool khrn_resource_add_self_read_conflicting_writer(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs,
      khrn_resource_parts_t parts)
{
   assert(!resource->in_begin_flags);
   assert(rs != NULL);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   /* If writing only during render, no need to conflict with bin reads -- they
    * will be done before we even start writing */
   khrn_stages_t conflicting_read_stages = ~(gfx_lowest_bit(stages) - 1);
   if (decode_stages(resource->readers, rs_index) & conflicting_read_stages)
      return false;

   khrn_resource_add_writer(resource, stages, rs, parts);
   return true;
}

v3d_scheduler_deps const* khrn_resource_begin_submit_reader_jobs(khrn_resource *resource)
{
   assert(!resource->in_begin_flags);
   khrn_resource_flush_writer(resource);
   debug_only(resource->in_begin_submit_reader_jobs = 1);
   return &resource->pre_read;
}

void khrn_resource_end_submit_reader_jobs(khrn_resource *resource,
      bool success, v3d_scheduler_deps const* deps)
{
   assert(resource->in_begin_submit_reader_jobs);
   debug_only(resource->in_begin_submit_reader_jobs = 0);
   if (deps->n == 0)
   {
      if (!success)
         return;

      /* we've waited for all the pre_read jobs already */
      v3d_scheduler_copy_deps(&resource->pre_read, deps);
   }
   else
      v3d_scheduler_merge_deps(&resource->pre_write, deps);
}

v3d_scheduler_deps const* khrn_resource_begin_submit_writer_jobs(khrn_resource *resource)
{
   assert(!resource->in_begin_flags);
   khrn_resource_flush(resource);
   debug_only(resource->in_begin_submit_writer_jobs = 1);
   return &resource->pre_write;
}

void khrn_resource_end_submit_writer_jobs(khrn_resource *resource,
      bool success, v3d_scheduler_deps const* deps,
      khrn_resource_parts_t parts)
{
   assert(resource->in_begin_submit_writer_jobs);
   debug_only(resource->in_begin_submit_writer_jobs = 0);
   resource->synced_start = ~0u;
   resource->synced_end = 0;

   /* If deps->n != 0, we assume the jobs in deps are dependent on the jobs in
    * pre_write, so it is fine to replace pre_read/write with deps.
    * If deps->n == 0 && success, we know that all the jobs in pre_write have
    * completed, but they may not have been finalised yet. It is ok to remove jobs from
    * pre_read as soon as they complete, but jobs may only be removed from
    * pre_write if they have been finalised or are replaced by a dependent job.
    * So we leave pre_write alone in the deps->n == 0 case. */
   if (deps->n != 0 || success)
      v3d_scheduler_copy_deps(&resource->pre_read, deps);
   if (deps->n != 0)
      v3d_scheduler_copy_deps(&resource->pre_write, deps);
   if (success)
      resource->undefined_parts &= ~parts;
}

static inline void khrn_resource_remove_rs_by_index(khrn_resource *resource, unsigned rs_index)
{
   uint64_t rs_mask = ~get_render_state_mask(rs_index);
   resource->readers &= rs_mask;
   resource->writer &= rs_mask;
}

void khrn_resource_remove_rs(khrn_resource *resource,
      const khrn_render_state *rs)
{
   assert(!resource->in_begin_flags);
   khrn_resource_remove_rs_by_index(resource, khrn_render_state_get_index(rs));
}

void khrn_resource_get_deps_for_rs(khrn_resource *resource,
      const khrn_render_state *rs, v3d_scheduler_deps stage_deps[KHRN_RESOURCE_NUM_STAGES])
{
   unsigned rs_index = khrn_render_state_get_index(rs);

   // Transfer read deps from resource to deps for first reader stage.
   khrn_stages_t read_stages = decode_stages(resource->readers, rs_index);
   if (read_stages)
      v3d_scheduler_merge_deps(&stage_deps[gfx_msb(gfx_lowest_bit(read_stages))], &resource->pre_read);

   // Transfer write deps from resource to deps for first writer stage.
   khrn_stages_t write_stages = decode_stages(resource->writer, rs_index);
   if (write_stages)
      v3d_scheduler_merge_deps(&stage_deps[gfx_msb(gfx_lowest_bit(write_stages))], &resource->pre_write);
}

void khrn_resource_update_from_rs(khrn_resource *resource,
      const khrn_render_state *rs, job_t const stage_jobs[KHRN_RESOURCE_NUM_STAGES])
{
   unsigned rs_index = khrn_render_state_get_index(rs);
   khrn_stages_t write_stages = decode_stages(resource->writer, rs_index);
   if (write_stages)
   {
      // Replace the pre-read and pre-write dependencies with the job for the last writer stage.
      uint64_t last_write_job = stage_jobs[gfx_msb(write_stages)];
      assert(last_write_job != 0);
      v3d_scheduler_deps_set(&resource->pre_write, last_write_job);
      v3d_scheduler_deps_set(&resource->pre_read, last_write_job);
      if (write_stages & (KHRN_STAGE_BIN | KHRN_STAGE_RENDER))
      {
         resource->synced_start = ~0u;
         resource->synced_end = 0;
      }
   }
   else
   {
      // Add the job for the last reader stage to the pre-write dependencies.
      khrn_stages_t read_stages = decode_stages(resource->readers, rs_index);
      if (read_stages)
      {
         unsigned last_read_stage = gfx_msb(read_stages);
         uint64_t last_read_job = stage_jobs[last_read_stage];

         // It's possible for preprocess to not have a job ID if it ran immediately.
         assert((1 << last_read_stage) == KHRN_STAGE_PREPROCESS || last_read_job != 0);

         if (last_read_job != 0)
            v3d_scheduler_add_dep(&resource->pre_write, last_read_job);
      }
   }

   // Remove the render-state.
   khrn_resource_remove_rs_by_index(resource, rs_index);
}

khrn_stages_t khrn_resource_get_stages(
      const khrn_resource *resource, khrn_render_state const* rs)
{
   assert(!resource->in_begin_flags);
   unsigned rs_index = khrn_render_state_get_index(rs);
   return decode_stages(resource->readers | resource->writer, rs_index);
}

khrn_stages_t khrn_resource_get_write_stages(
      const khrn_resource *resource, khrn_render_state const* rs)
{
   assert(!resource->in_begin_flags);
   unsigned rs_index = khrn_render_state_get_index(rs);
   return decode_stages(resource->writer, rs_index);
}

void khrn_resource_mark_undefined(khrn_resource *resource, khrn_resource_parts_t parts)
{
   assert(!resource->in_begin_flags);

   khrn_resource_flush_writer(resource);
   resource->undefined_parts |= parts;
}

bool khrn_resource_is_reader(khrn_resource const* resource, khrn_render_state const* rs)
{
   unsigned rs_index = khrn_render_state_get_index(rs);
   return (resource->readers & get_render_state_mask(rs_index)) != 0;
}

bool khrn_resource_is_writer(khrn_resource const* resource, khrn_render_state const* rs)
{
   unsigned rs_index = khrn_render_state_get_index(rs);
   return (resource->writer & get_render_state_mask(rs_index)) != 0;
}

void khrn_resource_job_replace(khrn_resource *resource, uint64_t job_id)
{
   assert(!resource->readers && !resource->writer);
   v3d_scheduler_deps_set(&resource->pre_write, job_id);
   v3d_scheduler_deps_set(&resource->pre_read, job_id);
}


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

static khrn_resource* rename_for_write_now(
   khrn_resource* res,
   v3d_size_t start,
   v3d_size_t length,
   khrn_access_flags_t flags)
{
   v3d_size_t size = gmem_get_size(res->handle);

   // If not overwriting the whole resource, then we'll need to wait for CPU read.
   bool discard_all = ((flags & KHRN_ACCESS_INVALIDATE_BUFFER) != 0)
                   || ((flags & KHRN_ACCESS_INVALIDATE_RANGE) != 0 && length == size);
   if (!discard_all)
   {
      khrn_resource_flush_writer(res);
      v3d_scheduler_wait_jobs(&res->pre_read, V3D_SCHED_DEPS_COMPLETED);
   }

   // If a write wouldn't cause a stall, no need to rename.
   if (!khrn_resource_write_now_would_stall(res))
      return NULL;

   // If rename buffer can't be allocated, then give up and stall.
   gmem_handle_t handle = try_alloc_until(
      size,
      res->align,
      gmem_get_usage(res->handle),
      gmem_get_desc(res->handle),
      !khrn_resource_has_reader_or_writer(res) ? &res->pre_write : NULL);
   if (!handle)
      return NULL;

   // Create new resource object.
   khrn_resource* rename = khrn_resource_create_no_handle();
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

      khrn_resource_gmem_invalidate_mapped_range(res, 0, size);
      if (flags & KHRN_ACCESS_INVALIDATE_RANGE)
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
   khrn_resource_refdec(rename);
   return NULL;
}

khrn_resource* khrn_resource_create_no_handle(void)
{
   khrn_resource* res = (khrn_resource*)calloc(1, sizeof(khrn_resource));
   if (!res)
      return NULL;

   res->ref_count = 1;
   res->synced_start = 0u;
   res->synced_end = ~(v3d_size_t)0u;
   return res;
}

khrn_resource* khrn_resource_create(
   size_t size,
   v3d_size_t align,
   gmem_usage_flags_t usage_flags,
   const char *desc)
{
   khrn_resource* res = khrn_resource_create_no_handle();
   if (res != NULL)
   {
      if (khrn_resource_alloc(res, size, align, usage_flags, desc))
         return res;

      khrn_resource_refdec(res);
   }
   return NULL;
}

khrn_resource* khrn_resource_create_with_handle(gmem_handle_t handle)
{
   khrn_resource* res = khrn_resource_create_no_handle();
   if (res != NULL)
   {
      res->handle = handle;
      res->synced_start = ~0u;
      res->synced_end = 0;
      return res;
   }
   return NULL;
}

bool khrn_resource_alloc(khrn_resource* res, size_t size,
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

void khrn_resource_destroy(khrn_resource *res)
{
   assert(res->ref_count == 0);

   if (res->handle != NULL)
   {
      assert(!khrn_resource_has_reader_or_writer(res));

      /* We cannot free the handle right now, we need to wait for the jobs
       * that are using this resource to complete and for their completion
       * callbacks to be run.
       */
      v3d_scheduler_gmem_deferred_free(&res->pre_write, res->handle);
   }
   free(res);
}

void khrn_resource_gmem_invalidate_mapped_range(khrn_resource* res, v3d_size_t start, v3d_size_t length)
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

void* khrn_resource_begin_access(
   khrn_resource** res_ptr,
   v3d_size_t offset,
   v3d_size_t length,
   khrn_access_flags_t flags,
   khrn_resource_parts_t parts)
{
   khrn_resource* res = *res_ptr;
   assert(!res->in_begin_flags);

   if (flags & KHRN_ACCESS_WRITE)
   {
      // Synchronise write if KHRN_ACCESS_UNSYNCHRONIZED is not set,
      // or if there are pending device writes.
      bool synchronise = !(flags & KHRN_ACCESS_UNSYNCHRONIZED)
                      || khrn_resource_read_now_would_stall(res);

      if (synchronise)
      {
         if (res->align != 0) // Don't rename external allocations.
         {
            khrn_resource* rename = rename_for_write_now(res, offset, length, flags);
            if (rename)
            {
               khrn_resource_refdec(res);
               *res_ptr = rename;
               return (char*)gmem_get_ptr(rename->handle) + offset;
            }
         }

         khrn_resource_flush(res);
         v3d_scheduler_wait_jobs(&res->pre_write, V3D_SCHED_DEPS_COMPLETED);
      }
      res->undefined_parts &= ~parts;
   }
   else
   {
      khrn_resource_flush_writer(res);
      v3d_scheduler_wait_jobs(&res->pre_read, V3D_SCHED_DEPS_COMPLETED);
   }

   void* ptr = gmem_map_and_get_ptr(res->handle);
   if (!ptr)
      return NULL;

   khrn_resource_gmem_invalidate_mapped_range(res, offset, length);
   return (char*)ptr + offset;
}

void* khrn_resource_read_now(
   khrn_resource* res,
   v3d_size_t offset,
   v3d_size_t length)
{
   assert(!res->in_begin_flags);
   void* ptr = gmem_map_and_get_ptr(res->handle);
   if (!ptr)
      return NULL;

   khrn_resource_flush_writer(res);
   v3d_scheduler_wait_jobs(&res->pre_read, V3D_SCHED_DEPS_COMPLETED);
   khrn_resource_gmem_invalidate_mapped_range(res, offset, length);

   return (char*)ptr + offset;
}

void* khrn_resource_try_read_now(
   khrn_resource* res,
   v3d_size_t offset,
   v3d_size_t length,
   bool* read_now)
{
   assert(!res->in_begin_flags);
   void* ptr = gmem_map_and_get_ptr(res->handle);
   if (!ptr)
      return NULL;

   // Try to access the resource now if possible.
   *read_now = false;
   if (  khrn_options.no_async_host_reads
      || (!khrn_options.force_async_host_reads && !khrn_resource_read_now_would_stall(res)))
   {
      if (khrn_options.no_async_host_reads)
      {
         khrn_resource_flush_writer(res);
         v3d_scheduler_wait_jobs(&res->pre_read, V3D_SCHED_DEPS_COMPLETED);
      }
      khrn_resource_gmem_invalidate_mapped_range(res, offset, length);
      *read_now = true;
   }

   return (char*)ptr + offset;
}