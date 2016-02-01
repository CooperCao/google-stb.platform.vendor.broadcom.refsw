/*****************************************************************************
 (c)2013 Broadcom Corporation.  All rights reserved.

 This program is the proprietary software of Broadcom Corporation and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").

 Except as set forth in an Authorized License, Broadcom grants no license
 (express or implied), right to use, or waiver of any kind with respect to the
 Software, and Broadcom expressly reserves all rights in and to the Software
 and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED
 LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,
 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use all
    reasonable efforts to protect the confidentiality thereof, and to use this
    information only in connection with your use of Broadcom integrated
    circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE
    SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef GMEM_H
#define GMEM_H

#include "../helpers/v3d/v3d_addr.h"

#include "gmem_plat.h"

#include "vcos.h"
#include "vcos_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Allow implementation to inline functions.
#ifndef GMEM_PLAT_INLINE
#define GMEM_PLAT_INLINE
#endif

typedef struct gmem_cpu_sync_list gmem_cpu_sync_list;
typedef struct gmem_v3d_sync_list gmem_v3d_sync_list;

/* gmem_handle_t is a platform-specific opaque type.
 * 0 is an invalid gmem_handle_t. */
#define GMEM_HANDLE_INVALID 0


/* Usage flags ============================================================= */
typedef enum
{
   GMEM_USAGE_NONE       = 0,

   GMEM_USAGE_CPU_READ   = 1 << 0,
   GMEM_USAGE_CPU_WRITE  = 1 << 1,
   GMEM_USAGE_V3D_READ   = 1 << 2,
   GMEM_USAGE_V3D_WRITE  = 1 << 3,

   GMEM_USAGE_CPU = GMEM_USAGE_CPU_READ | GMEM_USAGE_CPU_WRITE,
   GMEM_USAGE_V3D = GMEM_USAGE_V3D_READ | GMEM_USAGE_V3D_WRITE,
   GMEM_USAGE_ALL = GMEM_USAGE_CPU | GMEM_USAGE_V3D,

   /* Don't cache CPU reads/writes. If this hint is observed, this may
    * significantly impact the performance of reads/writes from the CPU, but will
    * avoid the need to flush the CPU caches before/after accessing from V3D. */
   GMEM_USAGE_HINT_CPU_UNCACHED = 1 << 5,

   /* Buffer is going to be short lived.  If this is observed, then it will
    * result in faster allocation operations */
   GMEM_USAGE_HINT_DYNAMIC = 1 << 6,
} gmem_usage_flags_t;


/* Alloc/free ============================================================== */

/* This will allocate physical memory. It is guaranteed to have been mapped
 * into V3D's address space before the next submitted job actually runs.
 *
 * An opaque handle is returned. To get a user-space pointer you need to call
 * gmem_map(). To get a V3D address you need to call gmem_lock().
 *
 * desc is purely for debugging.
 *
 * desc should be a short description of what the allocation contains or is
 * for, eg "texture" or "control list".
 *
 * On failure, GMEM_HANDLE_INVALID is returned. */
extern gmem_handle_t gmem_alloc(
   size_t size, size_t align,
   gmem_usage_flags_t usage_flags, /* Some combination of GMEM_USAGE_* */
   const char *desc);

/* While the process is running, the user side of the driver is responsible for
 * freeing memory by calling gmem_free().
 *
 * Once the process dies, the memory manager and scheduler together are
 * responsible for killing/waiting for outstanding V3D jobs and then freeing
 * all memory allocated by the process.
 *
 * The freeing conceptually happens immediately, so to avoid undefined
 * behaviour, the caller must wait for eg any V3D tasks that are using the
 * memory to finish before calling gmem_free().
 *
 * gmem_free(GMEM_HANDLE_INVALID) is a no-op. */
extern void gmem_free(gmem_handle_t handle);

/* "Locking" =============================================================== */

/* To get a V3D address, you must "lock" a gmem_handle_t in place by calling
 * gmem_lock(). gmem_lock() adds an entry to a lock_list and the V3D address is
 * valid until that lock_list is processed by calling
 * gmem_lock_list_unlock_and_destroy().
 *
 * The contents of struct gmem_lock_list are platform-specific.
 *
 * On a platform which doesn't support buffer relocation, struct gmem_lock_list
 * will likely be empty and the various gmem_lock_list_* functions won't do
 * anything. They are all declared inline for this reason. */

extern void gmem_lock_list_init(struct gmem_lock_list *lock_list);

/* Lock a gmem_handle_t in place and return its V3D address. The address is
 * valid until lock_list is processed by calling
 * gmem_lock_list_unlock_and_destroy().
 *
 * We might fail to add to the lock_list. In that case, 0 is returned, and the
 * lock_list is marked as "bad".
 *
 * The idea is that we don't need to deal with all failures immediately. eg If
 * we're just locking a bunch of handles and sticking the addresses straight
 * into a control list, there's no need to check for failure after every
 * gmem_lock() call, we can just check once at the end. */
extern v3d_addr_t gmem_lock(struct gmem_lock_list *lock_list, gmem_handle_t handle);

/* Check if the lock_list is marked as bad. A lock_list can only be marked as
 * bad by a gmem_lock() call. */
extern bool gmem_lock_list_is_bad(const struct gmem_lock_list *lock_list);

/* Unlock everything in the lock_list and destroy the set. This works whether
 * or not the lock_list is marked as bad. */
extern void gmem_lock_list_unlock_and_destroy(struct gmem_lock_list *lock_list);


/* Mapping into user space ================================================= */

/* Map the buffer into user space. Returns NULL on failure. */
extern void *gmem_map(gmem_handle_t handle);

/* Unmap from user space. */
extern void gmem_unmap(gmem_handle_t handle);

/* Synchronisation ========================================================= */

typedef enum
{
   /* Default value */
   GMEM_SYNC_NONE                = 0,

   /* V3D sync-list only flags */
   GMEM_SYNC_CORE_READ           = 1 << 0, /* For reads that are uncached within the V3D core, eg control list reads */
   GMEM_SYNC_CORE_WRITE          = 1 << 1, /* For writes that are uncached within the V3D core, eg TLB writes */
   GMEM_SYNC_CORE                = GMEM_SYNC_CORE_READ | GMEM_SYNC_CORE_WRITE,

   GMEM_SYNC_TMU_DATA_READ       = 1 << 2, /* Cached by L1T and L2T */
   GMEM_SYNC_TMU_DATA_WRITE      = 1 << 3, /* Cached by L2T */
   GMEM_SYNC_TMU_DATA            = GMEM_SYNC_TMU_DATA_READ | GMEM_SYNC_TMU_DATA_WRITE,

   GMEM_SYNC_TMU_CONFIG_READ     = 1 << 4, /* Cached by TMU VCR and L2C */
   GMEM_SYNC_QPU_IU_READ         = 1 << 5, /* QPU instrs/unifs, cached by L1 unif/instr caches and L2C */
   GMEM_SYNC_VCD_READ            = 1 << 6, /* Cached by VCD cache */
   GMEM_SYNC_TFU_READ            = 1 << 7,
   GMEM_SYNC_TFU_WRITE           = 1 << 8,
   GMEM_SYNC_TFU                 = GMEM_SYNC_TFU_READ | GMEM_SYNC_TFU_WRITE,

   GMEM_SYNC_V3D_READ         = GMEM_SYNC_CORE_READ
                              | GMEM_SYNC_TMU_DATA_READ
                              | GMEM_SYNC_TMU_CONFIG_READ
                              | GMEM_SYNC_QPU_IU_READ
                              | GMEM_SYNC_TFU_READ
                              | GMEM_SYNC_VCD_READ,
   GMEM_SYNC_V3D_WRITE        = GMEM_SYNC_CORE_WRITE
                              | GMEM_SYNC_TMU_DATA_WRITE
                              | GMEM_SYNC_TFU_WRITE,
   GMEM_SYNC_V3D              = GMEM_SYNC_V3D_READ | GMEM_SYNC_V3D_WRITE,

   /* CPU sync-list only flags */
   GMEM_SYNC_CPU_READ            = 1 << 9,
   GMEM_SYNC_CPU_WRITE           = 1 << 10,
   GMEM_SYNC_CPU                 = GMEM_SYNC_CPU_READ | GMEM_SYNC_CPU_WRITE,

   /* Common sync-list flags. */
   GMEM_SYNC_RELAXED = 1 << 14,
   GMEM_SYNC_DISCARD = 1 << 15,
} gmem_sync_flags_t;

/* Guaranteed to be a power of 2 >= V3D_MAX_CACHE_LINE_SIZE */
extern size_t gmem_get_sync_block_size(void);

/* Sync_flags should be some combination of the GMEM_SYNC_* flags. Write flags
 * *do not* imply the corresponding read flags.
 *
 * If GMEM_SYNC_RELAXED is not specified, the offset given to range based
 * sync functions must be aligned to a multiple of gmem_get_sync_block_size().
 * If GMEM_SYNC_RELAXED is specified, the offset will automatically aligned
 * down. The end address will always be automatically aligned up.
 *
 * GMEM_SYNC_DISCARD allows the implementation to discard the contents of the
 * memory being written to (valid only with GMEM_SYNC_*_WRITE).
 *
 * The platform will not synchronise the items in the list until requested.
 */

/* CPU access functions ===================================================== *
 *
 * When accessing gmem buffers on CPU, the caller must:
 *  - Call pre_cpu_access following a V3D write since the last CPU access.
 *  - Call post_cpu_write following a CPU write before the next V3D access.
 * In other cases the calls may be skipped.
 * Convenience functions that include mapping/unmapping are provided.
 */

/* Sync before CPU access to buffer. */
void gmem_sync_pre_cpu_access(gmem_handle_t handle, gmem_sync_flags_t sync_flags);

/* Sync before CPU access to buffer range. */
void gmem_sync_pre_cpu_access_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* Sync before CPU accesses in sync-list. */
void gmem_sync_pre_cpu_access_list(gmem_cpu_sync_list *sync_list);

/* Sync after CPU writes to buffer. */
GMEM_PLAT_INLINE void gmem_sync_post_cpu_write(gmem_handle_t handle, gmem_sync_flags_t sync_flags);

/* Sync after CPU writes to buffer range. */
GMEM_PLAT_INLINE void gmem_sync_post_cpu_write_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* Sync after CPU writes in sync-list (read-only accesses will be ignored). */
GMEM_PLAT_INLINE void gmem_sync_post_cpu_write_list(gmem_cpu_sync_list *sync_list);

/* Map and sync before CPU access to buffer. */
static inline void* gmem_map_and_begin_cpu_access(gmem_handle_t handle, gmem_sync_flags_t sync_flags);

/* Sync after CPU access to buffer and unmap. */
static inline void gmem_end_cpu_access_and_unmap(gmem_handle_t handle, gmem_sync_flags_t sync_flags);

/* Map and sync before CPU access to buffer range. */
static inline void* gmem_map_and_begin_cpu_access_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* Sync after CPU access to buffer range and unmap. */
static inline void gmem_end_cpu_access_range_and_unmap(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* CPU sync-list functions ================================================== *
 *
 * Alternatively, a CPU sync-list can be constructed and passed to
 * gmem_pre/post_cpu_access_list to batch the operations. It is not required
 * to pass the same list into both pre and post, the operations are per-handle.
 */

/* Initialise a CPU sync-list. */
void gmem_cpu_sync_list_init(gmem_cpu_sync_list *sync_list);

/* Record a read/write operation in a CPU sync-list. */
void gmem_cpu_sync_list_add(
   gmem_cpu_sync_list *sync_list,
   gmem_handle_t handle,
   gmem_sync_flags_t sync_flags
);

/* Record a ranged read/write operation in a CPU sync-list. */
void gmem_cpu_sync_list_add_range(
   gmem_cpu_sync_list *sync_list,
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* Destroy a CPU sync-list. */
void gmem_cpu_sync_list_destroy(gmem_cpu_sync_list *sync_list);

/* V3D sync-list functions ================================================== *
 *
 * The V3D sync-list should be passed to the scheduler with the job that will
 * actually do the reading/writing. The scheduler will perform the
 * synchronisation just before running the job. */

/* Initialise a V3D sync-list. */
GMEM_PLAT_INLINE void gmem_v3d_sync_list_init(gmem_v3d_sync_list *sync_list);

/* Destroy a V3D sync-list. */
GMEM_PLAT_INLINE void gmem_v3d_sync_list_destroy(gmem_v3d_sync_list *sync_list);

/* Record a read/write operation in a V3D sync-list. */
GMEM_PLAT_INLINE void gmem_v3d_sync_list_add(
   gmem_v3d_sync_list *sync_list,
   gmem_handle_t handle,
   gmem_sync_flags_t sync_flags
);

/* Record a ranged read/write operation in a V3D sync-list. */
GMEM_PLAT_INLINE void gmem_v3d_sync_list_add_range(
   gmem_v3d_sync_list *sync_list,
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
);

/* Queries ================================================================= */

extern size_t gmem_get_size(gmem_handle_t handle);
extern gmem_usage_flags_t gmem_get_usage(gmem_handle_t handle);

/*
 * Convert between V3D pointers and userspace pointers when you've forgotten
 * the handle that links them. These are no-ops on real silicon.
 */
extern void *gmem_addr_to_ptr(v3d_addr_t addr);
extern v3d_addr_t gmem_ptr_to_addr(void *p);

/* Debug  ================================================================= */

/* Print out a summary of the heap */
extern void gmem_print();
extern void gmem_print_level(VCOS_LOG_LEVEL_T level);

/* Example usage =========================================================== */

/* Inserting addresses into a control list:
 *
 *    cl = alloc_cl(...);
 *
 *    v3d_cl_my_instruction(&cl,
 *       v3d_mode_very_pretty,
 *       gmem_lock(&lock_list, some_handle));
 *
 *    v3d_cl_my_other_instruction(&cl,
 *       gmem_lock(&lock_list, another_handle),
 *       gmem_lock(&lock_list, yet_another_handle));
 *
 *    if (gmem_lock_list_is_bad(&lock_list))
 *       abandon_current_frame(); // Which will call gmem_lock_list_unlock_and_destroy(lock_list)
 */
#ifdef __cplusplus
}
#endif

#include "gmem.inl"

#endif /* GMEM_H */
