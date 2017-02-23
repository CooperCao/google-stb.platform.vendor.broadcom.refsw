/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "gmem_plat.inl"

#ifdef __cplusplus

static inline gmem_usage_flags_t operator~(gmem_usage_flags_t a)
{
   return (gmem_usage_flags_t)(~(int)a);
}

static inline gmem_usage_flags_t operator|(gmem_usage_flags_t a, gmem_usage_flags_t b)
{
   return (gmem_usage_flags_t)((int)a | (int)b);
}

static inline gmem_usage_flags_t operator|=(gmem_usage_flags_t& a, gmem_usage_flags_t b)
{
   a = (gmem_usage_flags_t)((int)a | (int)b);
   return a;
}

static inline gmem_usage_flags_t operator&(gmem_usage_flags_t a, gmem_usage_flags_t b)
{
   return (gmem_usage_flags_t)((int)a & (int)b);
}

static inline gmem_usage_flags_t operator&=(gmem_usage_flags_t& a, gmem_usage_flags_t b)
{
   a = (gmem_usage_flags_t)((int)a & (int)b);
   return a;
}

#endif

static inline void* gmem_map_and_invalidate_buffer(gmem_handle_t handle)
{
   void* ptr = gmem_map_and_get_ptr(handle);
   if (!ptr)
      return NULL;

   gmem_invalidate_mapped_buffer(handle);
   return ptr;
}

static inline void* gmem_map_and_invalidate_range(
   gmem_handle_t handle,
   v3d_size_t offset,
   v3d_size_t length)
{
   void* ptr = gmem_map_and_get_ptr(handle);
   if (!ptr)
      return NULL;

   gmem_invalidate_mapped_range(handle, offset, length);
   return (char*)ptr + offset;
}
