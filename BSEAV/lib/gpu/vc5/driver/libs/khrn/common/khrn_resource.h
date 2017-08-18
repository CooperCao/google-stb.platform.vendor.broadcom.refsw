/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include "khrn_types.h"
#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

// same as GL_MAP_ flags (except internal flags KHRN_ACCESS_ORPHAN)
typedef enum khrn_access_flags_t
{
   KHRN_ACCESS_READ              = 0x0001,
   KHRN_ACCESS_WRITE             = 0x0002,
   KHRN_ACCESS_INVALIDATE_RANGE  = 0x0004,
   KHRN_ACCESS_INVALIDATE_BUFFER = 0x0008,
   KHRN_ACCESS_FLUSH_EXPLICIT    = 0x0010,
   KHRN_ACCESS_UNSYNCHRONIZED    = 0x0020,
   KHRN_ACCESS_ORPHAN            = 0x0040,
} khrn_access_flags_t;

/*
 * Terminology: A "reader" is a render state that reads the resource, a
 * "writer" one that writes it.
 *
 * A render-state uses the resource during one or more "stages" which
 * are things like binning and rendering. This allows us to be more
 * fine-grained about synchronization-- if you want to wait for a
 * render-state to have finished with a resource then you only have
 * to wait for his specific stages on the resource to have completed.
 */
typedef enum khrn_stages_t
{
   KHRN_STAGE_PREPROCESS = 1 << 0,
   KHRN_STAGE_BIN        = 1 << 1,
   KHRN_STAGE_RENDER     = 1 << 2,
}
khrn_stages_t;
#define KHRN_STAGES_BIN_RENDER (KHRN_STAGE_BIN | KHRN_STAGE_RENDER)
#define KHRN_RESOURCE_NUM_STAGES 3u
#define KHRN_RESOURCE_BITS_PER_RS 4u

/* Resources are divided up into parts. Exactly how they are divided up is
 * defined on a per-resource basis.
 *
 * The first KHRN_RESOURCE_PARTS_BITS-1 bits of a khrn_resource_parts_t
 * correspond to the first KHRN_RESOURCE_PARTS_BITS-1 parts of a resource. The
 * last bit (KHRN_RESOURCE_PARTS_REST) corresponds to the rest of the parts. */
typedef uint32_t khrn_resource_parts_t;
#define KHRN_RESOURCE_PARTS_ALL ((khrn_resource_parts_t)-1)
#define KHRN_RESOURCE_PARTS_BITS (sizeof(khrn_resource_parts_t) * 8)
#define KHRN_RESOURCE_PARTS_REST ((khrn_resource_parts_t)1 << (KHRN_RESOURCE_PARTS_BITS - 1))

/* Make a khrn_resource_parts_t covering parts i through i+num-1. Note that
 * the returned value may cover more parts than requested -- if any of the
 * requested parts does not directly correspond to a bit in
 * khrn_resource_parts_t, KHRN_RESOURCE_PARTS_REST will be set in the
 * returned value. */
static inline khrn_resource_parts_t khrn_resource_parts(unsigned i, unsigned num)
{
   assert(num);
   if (i >= KHRN_RESOURCE_PARTS_BITS)
      return KHRN_RESOURCE_PARTS_REST;
   khrn_resource_parts_t parts = (num < KHRN_RESOURCE_PARTS_BITS) ?
      (KHRN_RESOURCE_PARTS_ALL >> (KHRN_RESOURCE_PARTS_BITS - num)) :
      KHRN_RESOURCE_PARTS_ALL;
   return parts << i;
}

typedef struct khrn_resource
{
   // KHRN_RESOURCE_BITS_PER_RS bits per render-state to indicate which
   // stages are reading this resource.
   uint64_t readers;

   // KHRN_RESOURCE_BITS_PER_RS bits per render-state to indicate which
   // stages are writing this resource.
   // There can be only one render-state writing an resource.
   uint64_t writer;

   // Valid iff there is an unflushed bin-stage writer. Number of transform
   // feedback blocks that must be waited for (V3D_CL_WAIT_TRANSFORM_FEEDBACK)
   // before a non-TF access of this buffer in the writer's bin control list.
   unsigned last_tf_write_count;

   /* Which parts of the resource will have undefined contents after all writers
    * have been flushed and waited for? This is conservative -- flushing may
    * cause more parts to be marked as undefined and undefined parts need not be
    * marked as such. */
   khrn_resource_parts_t undefined_parts;

#ifndef NDEBUG
   union
   {
      struct
      {
         uint8_t in_begin_submit_reader_jobs : 1;
         uint8_t in_begin_submit_writer_jobs : 1;
      };
      uint8_t in_begin_flags;
   };
#endif

   /*
    * When a writer or reader is flushed, any resulting jobs will be added to
    * these deps so we can wait for for them to complete/be finalised.
    *
    * If you are writing the buffer, you need to wait for the jobs in pre_write
    * to complete.
    * If you are reading the buffer, you need to wait for the jobs in pre_read
    * to complete.
    * If you are freeing the buffer, you need to wait for the jobs in pre_write
    * to be finalised.
    *
    * Note that:
    * - pre_write is always a superset of pre_read (ie after waiting for all
    *   the jobs in pre_write to complete/be finalised, it is guaranteed that
    *   all the jobs in pre_read will have completed/been finalised)
    * - jobs may be removed from pre_read as soon as they complete, but jobs
    *   may not be removed from pre_write until they have been finalised (or
    *   replaced with a dependent job)
    */
   v3d_scheduler_deps pre_write;
   v3d_scheduler_deps pre_read;

   gmem_handle_t handle;
   uint32_t ref_count;
   v3d_size_t align;
   v3d_size_t synced_start;
   v3d_size_t synced_end;
} khrn_resource;

/* Creates a resource; the handle contained by a resource gets allocated from
 * gmem with the specified params for size, align, usage_flags and description.
 */
khrn_resource* khrn_resource_create(size_t size,
      v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc);

/* Creates a resource without allocating memory.
 * Memory can be allocated later by calling khrn_resource_alloc. */
khrn_resource* khrn_resource_create_no_handle(void);

/* Creates a resource with a pre-existing handle.
 * The ownership of the handle is transferred to this object and it will get
 * freed when the last reference of this object gets to 0.
 * Contents of the gmem handle are assume to not be coherent with the CPU.
 * Buffer renaming is disabled for externally provided handles.
 */
khrn_resource* khrn_resource_create_with_handle(gmem_handle_t handle);

/* Allocate memory for the resource now. Return false if failed. */
bool khrn_resource_alloc(khrn_resource* res, size_t size,
      v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc);

//! Acquire a reference to this resource.
static inline void khrn_resource_refinc(khrn_resource *res);

//! Release a reference to this resource.
static inline void khrn_resource_refdec(khrn_resource *ref_i);

/*
 * Flush whatever render-states the resource has (whether it's a writer or a set of readers).
 */
void khrn_resource_flush(khrn_resource *resource);

/*
 * Flush the writer of an resource if it has one.
 */
void khrn_resource_flush_writer(khrn_resource *resource);

/*
 * Add rs as a reader of resource for the specified stages.
 *
 * If there is a writer (that isn't rs), it is flushed.
 */
void khrn_resource_add_reader(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs);

/*
 * Add a writer for the specified stages, flushing any existing
 * different writer or readers.
 *
 * If rs was already the reader, other readers are flushed, and rs still
 * becomes the writer, but rs is not flushed.
 *
 * parts should cover all the parts which might be written. true is returned
 * iff all the parts which might be written were marked as undefined.
 */
bool khrn_resource_add_writer(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs,
      khrn_resource_parts_t parts);

/*
 * If resource is already marked as being read by rs, returns false immediately.
 * Otherwise, calls khrn_resource_add_writer() and returns true.
 * parts should cover all the parts which might be written.
 */
bool khrn_resource_add_self_read_conflicting_writer(khrn_resource *resource,
      khrn_stages_t stages, khrn_render_state *rs,
      khrn_resource_parts_t parts);

/* Begin submitting jobs that read from this resource.
   The next call must be to khrn_resource_end_submit_reader_jobs().
   Returns the dependencies for the job to be submitted. */
v3d_scheduler_deps const* khrn_resource_begin_submit_reader_jobs(khrn_resource *resource);

/* End submitting jobs that read from this resource by providing
 * the job-ids of those jobs;
 * call with success = false and deps->n = 0, if we've failed submitting and reader jobs
 * call deps->n != 0, if we've submitted any asynchonrous reader jobs
 * call with success = true and deps->n = 0 if we've done synchronous reads from the data
 *                                          protected by this resource
 */
void khrn_resource_end_submit_reader_jobs(khrn_resource *resource,
      bool success, v3d_scheduler_deps const* deps);

/* Begin submitting jobs that write to this resource.
   The next call must be to khrn_resource_end_submit_writer_jobs().
   Returns the dependencies for the job to be submitted. */
v3d_scheduler_deps const* khrn_resource_begin_submit_writer_jobs(khrn_resource *resource);

/* End submitting jobs that write to this resource by providing the
 * job-ids of those jobs.
 * call with success = false and deps->n = 0, if we've failed submitting and writer jobs
 * call deps->n != 0, if we've submitted any asynchonrous writer jobs
 * call with success = true and deps->n = 0 if we've done synchronous writes to the data
 *                                           protected by this resource
 * parts should cover all the parts which might be/have been written by the
 * jobs.
 */
void khrn_resource_end_submit_writer_jobs(khrn_resource *resource,
      bool success, v3d_scheduler_deps const* deps,
      khrn_resource_parts_t parts);

/*
 * Replaces all dependencies with a single job. Only use this when you know
 * the job depends on everything in the resource. This will normally only
 * be the case when the job is a writer.
 */
void khrn_resource_job_replace(khrn_resource *resource, uint64_t job_id);

//! If rs is the writer or one of the readers, remove it.
void khrn_resource_remove_rs(khrn_resource *resource,
      const khrn_render_state *rs);

//! Get dependencies for render-state stages from this resource.
//! Dependencies are expected to be made transitive, so only the first stage is updated.
void khrn_resource_get_deps_for_rs(khrn_resource *resource,
   const khrn_render_state *rs, v3d_scheduler_deps stage_deps[KHRN_RESOURCE_NUM_STAGES]);

//! Update the resource with job-ids from the render-state and remove it.
void khrn_resource_update_from_rs(khrn_resource *resource,
      const khrn_render_state *rs, uint64_t const stage_jobs[KHRN_RESOURCE_NUM_STAGES]);

//! Returns the stages that rs has on resource for reading or writing.
khrn_stages_t khrn_resource_get_stages(
      const khrn_resource *resource, khrn_render_state const* rs);

//! Returns the stages that rs has on resource for writing.
khrn_stages_t khrn_resource_get_write_stages(
      const khrn_resource *resource, khrn_render_state const* rs);

/* Call this when the resource's contents have become undefined. Note
 * this will flush the writer if there is one. */
void khrn_resource_mark_undefined(khrn_resource *resource, khrn_resource_parts_t parts);

//! Returns true if resource has this render-state as a reader.
bool khrn_resource_is_reader(khrn_resource const* resource, khrn_render_state const* rs);

//! Returns true if resource has this render-state as a writer.
bool khrn_resource_is_writer(khrn_resource const* resource, khrn_render_state const* rs);

//! Returns true if resource has a reader.
static inline bool khrn_resource_has_reader(khrn_resource const* resource);

//! Returns true if resource has a writer.
static inline bool khrn_resource_has_writer(khrn_resource const* resource);

//! Returns true if resource has a reader or writer.
static inline bool khrn_resource_has_reader_or_writer(khrn_resource const* resource);

//! Returns true if CPU read access now would stall.
static inline bool khrn_resource_read_now_would_stall(khrn_resource * resource);

//! Returns true if CPU write access now would stall.
static inline bool khrn_resource_write_now_would_stall(khrn_resource* resource);

//! Invalidate the mapped range of this resource gmem handle without waiting or flushing.
void khrn_resource_gmem_invalidate_mapped_range(khrn_resource* res, v3d_size_t start, v3d_size_t length);

//! Perform synchronisation required for CPU access to this resource range now.
//! If flags contain (KHRN_ACCESS_WRITE|KHRN_ACCESS_ORPHAN) then *res_ptr
//! might be updated to a new resource in order to avoid stalling.
//! Parts should cover all the parts which might be written.
//! Returns CPU mapped pointer to buffer at offset.
void* khrn_resource_begin_access(
   khrn_resource** res_ptr,
   v3d_size_t offset,
   v3d_size_t length,
   khrn_access_flags_t flags,
   khrn_resource_parts_t parts);

//! Perform synchronisation required post CPU access of this resource range.
static inline void khrn_resource_end_access(
   khrn_resource* res,
   v3d_size_t offset,
   v3d_size_t length,
   khrn_access_flags_t flags);

//! Perform synchronisation required for CPU read access to this resource range now.
//! Return CPU mapped pointer to buffer at offset, or NULL on map failure.
void* khrn_resource_read_now(
   khrn_resource* res,
   v3d_size_t offset,
   v3d_size_t length);

//! Perform synchronisation required for CPU read access to this resource range now
//! if it would not stall. Otherwise return the pointer and set read_now to false.
//! Return CPU mapped pointer to buffer at offset, or NULL on map failure.
void* khrn_resource_try_read_now(
   khrn_resource* res,
   v3d_size_t offset,
   v3d_size_t length,
   bool* read_now);

#ifdef __cplusplus
}
#endif

#include "khrn_resource.inl"
