/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_addr.h"
#include "libs/util/log/log.h"
#include "libs/util/common.h"
#include <stdbool.h>

/* gmem_handle_t is a platform-specific opaque type.
 * 0 is an invalid gmem_handle_t. */
#define GMEM_HANDLE_INVALID 0

/* Usage flags ============================================================= */
typedef enum
{
   GMEM_USAGE_NONE       = 0,

   GMEM_USAGE_V3D_READ   = 1 << 0,
   GMEM_USAGE_V3D_WRITE  = 1 << 1,

   /* Don't cache CPU reads/writes. This may significantly impact the performance
    * of reads/writes from the CPU, but will avoid the need to flush the CPU
    * caches before/after accessing from V3D. */
   GMEM_USAGE_COHERENT = 1 << 5,

   /* Buffer is going to be short lived.  If this is observed, then it will
    * result in faster allocation operations */
   GMEM_USAGE_HINT_DYNAMIC = 1 << 6,

   /* Secure usage tells the allocator to grab memory from the secure heap. */
   GMEM_USAGE_SECURE = 1 << 7,

   /* External h/w blocks don't go via the MMU. Memory used by these blocks must be contiguous */
   GMEM_USAGE_CONTIGUOUS = 1 << 8,
} gmem_usage_flags_t;

#define GMEM_USAGE_V3D_RW (GMEM_USAGE_V3D_READ | GMEM_USAGE_V3D_WRITE)

#include "gmem_plat.h"

// Allow implementation to inline functions.
#ifndef GMEM_PLAT_INLINE
#define GMEM_PLAT_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Alloc/free ============================================================== */

/* This will allocate physical memory. It is guaranteed to have been mapped
 * into V3D's address space before the next submitted job actually runs.
 *
 * An opaque handle is returned. To get a user-space pointer you need to call
 * gmem_map_and_get_ptr(). To get a V3D address you need to call gmem_get_addr().
 *
 * desc is purely for debugging.
 *
 * desc should be a short description of what the allocation contains or is
 * for, eg "texture" or "control list".
 *
 * On failure, GMEM_HANDLE_INVALID is returned. */
gmem_handle_t gmem_alloc(
   size_t size, v3d_size_t align,
   gmem_usage_flags_t usage_flags, /* Some combination of GMEM_USAGE_* */
   const char *desc);

/* This will allocate physical memory. It is guaranteed to have been mapped
 * into V3D's address space before the next submitted job actually runs.
 *
 * An opaque handle is returned. To get a user-space pointer you need to call
 * gmem_get_ptr(). To get a V3D address you need to call gmem_get_addr().
 *
 * desc is purely for debugging.
 *
 * desc should be a short description of what the allocation contains or is
 * for, eg "texture" or "control list".
 *
 * On failure (either alloc or map), GMEM_HANDLE_INVALID is returned. */
gmem_handle_t gmem_alloc_and_map(
   size_t size, v3d_size_t align,
   gmem_usage_flags_t usage_flags, /* Some combination of GMEM_USAGE_* */
   const char *desc);

/* Immediately free the specified memory. Note that as the freeing is
 * immediate, to avoid undefined behaviour, the caller must wait for eg any V3D
 * tasks that are using the memory to finish before calling.
 *
 * gmem_free(GMEM_HANDLE_INVALID) is a no-op. */
void gmem_free(gmem_handle_t handle);

/* Return V3D address for gmem handle. This is guaranteed not to change. */
GMEM_PLAT_INLINE v3d_addr_t gmem_get_addr(gmem_handle_t handle);

/* Mapping into user space ================================================= */

/* Ensure the buffer is mapped and return the CPU pointer.
 * After a successful call it is possible to use gmem_get_ptr.
 * Returns NULL if the map operation failed. */
GMEM_PLAT_INLINE void* gmem_map_and_get_ptr(gmem_handle_t handle);

/* Return the CPU pointer to a previously mapped buffer. */
GMEM_PLAT_INLINE void* gmem_get_ptr(gmem_handle_t handle);

/* Access coherence ======================================================== */

/* Memory allocations are conceptually divided into atoms. For non-coherent
 * allocations, the byte at offset o is in atom (o /
 * gmem_non_coherent_atom_size()). For coherent allocations, each byte is a
 * separate atom.
 *
 * Undefined results here means:
 * - For a read, undefined data returned but contents of memory unaffected
 * - For a write, data for relevant atoms in memory becomes undefined
 *
 * Between a host write and a V3D read/write of an atom, if the following are
 * not done (in the order specified):
 * - (Non-coherent allocations only) gmem_flush_mapped_buffer/range() call
 *   covering range written by host
 * - Flush of relevant V3D caches, outside-in
 * then the V3D read/write will have undefined results.
 *
 * Between a V3D write and a host read/write of an atom, if the following are
 * not done (in the order specified):
 * - Clean of relevant V3D caches, inside-out
 * - (Non-coherent allocations only) gmem_invalidate_mapped_buffer/range() call
 *   covering range written by V3D
 * then the host read/write will have undefined results.
 *
 * (Non-coherent allocations only) Between a host write of an atom and a
 * gmem_invalidate_mapped_buffer/range() call involving the atom, if there is
 * not a gmem_flush_mapped_buffer/range() call covering the range written by
 * the host, the host write will have undefined results.
 *
 * (Non-coherent allocations only) Between a V3D write of an atom and a
 * gmem_flush_mapped_buffer/range() call involving the atom, if there is not a
 * gmem_invalidate_mapped_buffer/range() call covering the range written by
 * V3D, the V3D write will have undefined results.
 */

/* Guaranteed to be a power of 2 >= V3D_MAX_CACHE_LINE_SIZE */
v3d_size_t gmem_non_coherent_atom_size(void);

void gmem_invalidate_mapped_buffer(gmem_handle_t handle);
void gmem_invalidate_mapped_range(gmem_handle_t handle, v3d_size_t offset, v3d_size_t length);

void gmem_flush_mapped_buffer(gmem_handle_t handle);
void gmem_flush_mapped_range(gmem_handle_t handle, v3d_size_t offset, v3d_size_t length);

/* Map and call gmem_invalidate_mapped_buffer/range. */
static inline void* gmem_map_and_invalidate_buffer(gmem_handle_t handle);
static inline void* gmem_map_and_invalidate_range(gmem_handle_t handle,
   v3d_size_t offset, v3d_size_t length);

/* Queries ================================================================= */

v3d_size_t gmem_heap_size();

GMEM_PLAT_INLINE v3d_size_t gmem_get_size(gmem_handle_t handle);
GMEM_PLAT_INLINE gmem_usage_flags_t gmem_get_usage(gmem_handle_t handle);
GMEM_PLAT_INLINE char const* gmem_get_desc(gmem_handle_t handle);

/* Find a gmem handle for the given v3d address. Returns NULL if no match. */
gmem_handle_t gmem_find_handle_by_addr(v3d_addr_t addr);

/* Debug  ================================================================== */

/* Print out a summary of the heap */
void gmem_print();
void gmem_print_level(log_level_t level);

#if !GMEM_DEBUG
static inline void gmem_add_debug_ref(gmem_handle_t h) {}
static inline void gmem_release_debug_ref(gmem_handle_t h) {}
#endif

#ifdef __cplusplus
}
#endif

#include "gmem.inl"
