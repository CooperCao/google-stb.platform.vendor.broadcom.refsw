/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once
#include "vcos_atomic.h"

static inline gmem_alloc_item *gmem_validate_handle(gmem_handle_t handle)
{
   gmem_alloc_item *item = (gmem_alloc_item *)handle;
   assert(item->magic == GMEM_HANDLE_MAGIC);
   return item;
}

static inline v3d_size_t gmem_get_size(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   return item->size;
}

static inline gmem_usage_flags_t gmem_get_usage(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   return item->usage_flags;
}

static inline char const* gmem_get_desc(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   return item->desc;
}

static inline v3d_addr_t gmem_get_addr(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   return item->v3d_addr;
}

static inline void* gmem_get_ptr(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   assert(item->cpu_ptr != 0);
   return item->cpu_ptr;
}

static inline void* gmem_map_and_get_ptr(gmem_handle_t handle)
{
   gmem_alloc_item* item = gmem_validate_handle(handle);
   void* ptr = vcos_atomic_load_ptr(&item->cpu_ptr, VCOS_MEMORY_ORDER_ACQUIRE);
   if (!ptr)
      ptr = gmem_map_and_get_ptr_internal(item);
   return ptr;
}
