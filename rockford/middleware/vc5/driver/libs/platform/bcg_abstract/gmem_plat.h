/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef GMEM_PLAT_H
#define GMEM_PLAT_H

#include "list.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define GMEM_PLAT_INLINE static inline

typedef struct gmem_alloc_item *gmem_handle_t;
typedef void *BEGL_MemHandle;

#define GMEM_HANDLE_MAGIC 0xfee1900d

typedef struct gmem_alloc_item
{
   uint32_t                 magic;
   struct gmem_alloc_item  *prev;
   struct gmem_alloc_item  *next;
   const char              *desc;

   BEGL_MemHandle          memory_handle;
   size_t                  size;
   size_t                  align;
   uint16_t                usage_flags;
   volatile uint8_t        v3d_writes;
   bool                    externalAlloc;

   int                     driver_map_count;    /* Map calls made by the driver - external allocs not counted */
   void                    *cur_map_ptr;

   int                     driver_lock_count;   /* Lock calls made by the driver - external allocs not counted */
   v3d_addr_t              cur_lock_phys;
} gmem_alloc_item;

typedef struct gmem_lock_item
{
   gmem_handle_t            handle;
} gmem_lock_item;

struct gmem_lock_list
{
   //struct gmem_lock_item   head;

   struct gmem_lock_item   *items;
   unsigned int            size;
   unsigned int            capacity;

   unsigned int            bad;
};

typedef struct gmem_abstract_cpu_sync_block_t {
   LIST_ENTRY(gmem_abstract_cpu_sync_block_t)   link;
   gmem_handle_t                                handle;
   uint32_t                                     sync_flags;
   size_t                                       offset;
   size_t                                       length;
} gmem_abstract_cpu_sync_block_t;

struct gmem_cpu_sync_list
{
   LIST_HEAD(sync_list, gmem_abstract_cpu_sync_block_t) list;
};

struct gmem_v3d_sync_list
{
   uint32_t flags;
};

#ifdef __cplusplus
extern "C" {
#endif

static inline gmem_alloc_item* gmem_validate_handle(gmem_handle_t handle);
void gmem_validate_cpu_access(gmem_handle_t handle, uint32_t sync_flags);
void gmem_validate_cpu_access_range(gmem_handle_t handle, size_t offset, size_t length, uint32_t sync_flags);
void gmem_validate_v3d_access(gmem_handle_t handle, uint32_t sync_flags);
void gmem_validate_v3d_access_range(gmem_handle_t handle, size_t offset, size_t length, uint32_t sync_flags);

/* Wrap an existing platform memory handle in a gmem handle. This is only used for display buffers. */
extern gmem_handle_t gmem_from_external_memory(uint64_t physOffset, void *cachedPtr, size_t length, const char *desc);

#ifdef __cplusplus
}
#endif

#endif /* GMEM_PLAT_H */
