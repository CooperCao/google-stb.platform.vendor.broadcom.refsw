/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "gmem_plat.inl"

#ifdef __cplusplus

static inline gmem_usage_flags_t operator|(gmem_usage_flags_t a, gmem_usage_flags_t b)
{
   return (gmem_usage_flags_t)((int)a | (int)b);
}

static inline gmem_usage_flags_t operator|=(gmem_usage_flags_t& a, gmem_usage_flags_t b)
{
   a = (gmem_usage_flags_t)((int)a | (int)b);
   return a;
}

static inline gmem_sync_flags_t operator|(gmem_sync_flags_t a, gmem_sync_flags_t b)
{
   return (gmem_sync_flags_t)((int)a | (int)b);
}

static inline gmem_sync_flags_t operator|=(gmem_sync_flags_t& a, gmem_sync_flags_t b)
{
   a = (gmem_sync_flags_t)((int)a | (int)b);
   return a;
}

#endif

static inline void* gmem_map_and_begin_cpu_access(gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   void* ptr = gmem_map(handle);
   if (ptr)
   {
      gmem_sync_pre_cpu_access(handle, sync_flags);
   }
   return ptr;
}

static inline void gmem_end_cpu_access_and_unmap(gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   gmem_sync_post_cpu_write(handle, sync_flags);
   gmem_unmap(handle);
}

static inline void* gmem_map_and_begin_cpu_access_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
)
{
   void* ptr = gmem_map(handle);
   if (ptr)
   {
      gmem_sync_pre_cpu_access_range(handle, offset, length, sync_flags);
      ptr = (char*)ptr + offset;
   }
   return ptr;
}

static inline void gmem_end_cpu_access_range_and_unmap(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
)
{
   gmem_sync_post_cpu_write_range(handle, offset, length, sync_flags);
   gmem_unmap(handle);
}
