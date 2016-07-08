/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Reference counted object for memory handle protected by an interlock
=============================================================================*/
#ifndef KHRN_RES_INTERLOCK_H
#define KHRN_RES_INTERLOCK_H

#include "khrn_int_common.h"
#include "khrn_interlock.h"
#include "libs/platform/v3d_scheduler.h"

typedef struct
{
   gmem_handle_t handle;
   KHRN_INTERLOCK_T interlock;
   volatile int ref_count;
}KHRN_RES_INTERLOCK_T;

typedef struct khrn_res_interlock_gmem_args
{
   size_t align;
   uint32_t usage;
   const char* desc;
} khrn_res_interlock_gmem_args;

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
      size_t align, uint32_t usage_flags, const char *desc);

/* creates a res_interlock with a handle;
 * the ownership of the handle is transferred to this object and it will get
 * freed when the last refernce of this object gets to 0; handle cannot be
 * NEW_MEM_HANDLE_INVALID
 */
KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_with_handle(gmem_handle_t handle);

/* creates a res_interlok with GMEM_HANDLE_INVALID and we will probably
 * allocate the storage at some later point. We need this functions for cases
 * as ancillary buffers when we choose to allocate storage or not at flush
 * time, but we need the interlock to get to the stage of flushing even though
 * we might end up no needing storage and having nothing to protect.
 */
KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_no_handle(void);

/* you should call this function only when you've created a res_interlock with
 * no handle and you want to set a handle
 */
void khrn_res_interlock_set_handle(KHRN_RES_INTERLOCK_T *res_i, gmem_handle_t handle);

static inline void khrn_res_interlock_refinc(KHRN_RES_INTERLOCK_T *res_i)
{
   int before_inc = vcos_atomic_inc(&res_i->ref_count);
   vcos_unused_in_release(before_inc);
   assert(before_inc > 0);
}

void khrn_res_interlock_refdec(KHRN_RES_INTERLOCK_T *ref_i);

/* Convenience function - allocate a new resource interlock and upload data from the CPU to it
   in one go */
KHRN_RES_INTERLOCK_T *khrn_res_interlock_from_data(const void   *src_data,
                                                   size_t        src_data_bytes,
                                                   size_t        padding,
                                                   size_t        align,
                                                   uint32_t      usage_flags,
                                                   const char   *desc);

//! Map resource for CPU access, can update *res_ptr if a rename is performed.
void* khrn_res_interlock_map_range(KHRN_RES_INTERLOCK_T** res_ptr, size_t offset, size_t size, unsigned map_flags, const khrn_res_interlock_gmem_args* rename_args);

//! Unmap resource following call to khrn_res_interlock_map_range.
void khrn_res_interlock_unmap_range(KHRN_RES_INTERLOCK_T* res, size_t offset, size_t size, unsigned map_flags);

#endif
