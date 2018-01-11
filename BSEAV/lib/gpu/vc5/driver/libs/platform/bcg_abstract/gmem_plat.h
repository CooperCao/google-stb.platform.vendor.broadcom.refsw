/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define GMEM_PLAT_INLINE static inline

typedef struct gmem_alloc_item *gmem_handle_t;
typedef void *BEGL_MemHandle;

typedef void (*GMEM_TERM_T)(void *);

#define GMEM_HANDLE_MAGIC 0xfee1900d

typedef enum gmem_alloc_type_t
{
   GMEM_ALLOC_REGULAR,
   GMEM_ALLOC_TALLOC,
   GMEM_ALLOC_EXTERNAL,
} gmem_alloc_type_t;

typedef struct gmem_alloc_item
{
#ifndef NDEBUG
   uint32_t                 magic;
#endif
   struct gmem_alloc_item  *prev;
   struct gmem_alloc_item  *next;
   const char              *desc;

   BEGL_MemHandle          memory_handle;
   v3d_size_t              size;
   gmem_usage_flags_t      usage_flags;
   gmem_alloc_type_t       type;

   GMEM_TERM_T             external_term;
   void                    *external_context;      /* Some external modes need a destructor */

   void*                   cpu_ptr;
   v3d_addr_t              v3d_addr;
   uint64_t                external_addr;

} gmem_alloc_item;

#ifdef __cplusplus
extern "C" {
#endif

static inline gmem_alloc_item* gmem_validate_handle(gmem_handle_t handle);

/* Wrap an existing platform memory handle in a gmem handle. */
gmem_handle_t gmem_from_external_memory(GMEM_TERM_T external_term, void *external_context,
                                        uint64_t physOffset, void *cpu_ptr,
                                        size_t length, bool secure, bool contiguous,
                                        const char *desc);

uint64_t gmem_get_external_addr(gmem_handle_t handle);

/* Retrieve the external context if there is one. May return NULL. */
void *gmem_get_external_context(gmem_handle_t handle);

/* Retrieve the platform memory handle */
BEGL_MemHandle gmem_get_platform_handle(gmem_handle_t handle);

gmem_alloc_item* gmem_alloc_and_map_internal(size_t size, v3d_size_t align, uint32_t usage_flags, const char *desc);
void gmem_free_internal(gmem_handle_t handle);
void* gmem_map_and_get_ptr_internal(gmem_alloc_item* item);

#ifdef __cplusplus
}
#endif
