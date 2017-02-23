/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Reference counted object for memory handle protected by an interlock
=============================================================================*/
#pragma once

#include "khrn_interlock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct khrn_res_interlock
{
   gmem_handle_t handle;
   KHRN_INTERLOCK_T interlock;
   uint32_t ref_count;
   v3d_size_t align;
   v3d_size_t synced_start;
   v3d_size_t synced_end;
}KHRN_RES_INTERLOCK_T;

// same as GL_MAP_ flags
enum
{
   KHRN_MAP_READ_BIT                = 0x0001,
   KHRN_MAP_WRITE_BIT               = 0x0002,
   KHRN_MAP_INVALIDATE_RANGE_BIT    = 0x0004,
   KHRN_MAP_INVALIDATE_BUFFER_BIT   = 0x0008,
   KHRN_MAP_FLUSH_EXPLICIT_BIT      = 0x0010,
   KHRN_MAP_UNSYNCHRONIZED_BIT      = 0x0020,
};

/* creates a res_interlock; the handle contained by res_interlock gets
 * allocated from shared memory according with the specified params for size,
 * align, usage_flags and description (see params for function new_mem_alloc in
 * platform_if/mem.h"
 */
KHRN_RES_INTERLOCK_T* khrn_res_interlock_create(size_t size,
      v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc);

/* Creates a res_interlock without allocating memory.
 * Memory can be allocated later by calling khrn_res_interlock_alloc. */
KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_no_handle(void);

/* Creates a res_interlock with a pre-existing handle.
 * The ownership of the handle is transferred to this object and it will get
 * freed when the last reference of this object gets to 0.
 * Contents of the gmem handle are assume to not be coherent with the CPU.
 * Buffer renaming is disabled for externally provided handles.
 */
KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_with_handle(gmem_handle_t handle);

/* Allocate memory for the interlock now. Return false if failed. */
bool khrn_res_interlock_alloc(KHRN_RES_INTERLOCK_T* res, size_t size,
      v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc);

//! Acquire a reference to this res-interlock.
static inline void khrn_res_interlock_refinc(KHRN_RES_INTERLOCK_T *res_i);

//! Release a reference to this res-interlock.
static inline void khrn_res_interlock_refdec(KHRN_RES_INTERLOCK_T *ref_i);

//! Invalidate the mapped range of this res-interlock without waiting or flushing interlocks.
void khrn_res_interlock_invalidate_mapped_range(KHRN_RES_INTERLOCK_T* res, v3d_size_t start, v3d_size_t length);

//! Wait and sync resource pre CPU access and return pointer
//! Can update *res_ptr if a rename is performed, but only if KHRN_MAP_WRITE_BIT is passed.
void* khrn_res_interlock_pre_cpu_access_now(
   KHRN_RES_INTERLOCK_T** res_ptr,
   v3d_size_t offset,
   v3d_size_t length,
   uint32_t map_flags);

//! Sync resource post CPU access.
void khrn_res_interlock_post_cpu_access(
   KHRN_RES_INTERLOCK_T* res,
   v3d_size_t offset,
   v3d_size_t length,
   uint32_t map_flags);

static inline void khrn_res_interlock_invalidate_synced_range(KHRN_RES_INTERLOCK_T* res);

#ifdef __cplusplus
}
#endif

#include "khrn_res_interlock.inl"
