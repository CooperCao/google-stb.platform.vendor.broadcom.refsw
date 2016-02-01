/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

/* CPU access functions */

static inline gmem_alloc_item *gmem_validate_handle(gmem_handle_t handle)
{
   gmem_alloc_item *item = (gmem_alloc_item *)handle;
   assert(item->magic == GMEM_HANDLE_MAGIC);
   return item;
}

static inline void gmem_sync_post_cpu_write_internal(gmem_sync_flags_t sync_flags)
{
   // CPU has finished writing, set flag so that kernel driver will flush the CPU cache.
   if (sync_flags & GMEM_SYNC_CPU_WRITE)
      __atomic_store_n(&gmem_sync_cpu_write, true, __ATOMIC_RELEASE);
}

static inline void gmem_sync_post_cpu_write(gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   debug_only(gmem_validate_cpu_access(handle, sync_flags));
   gmem_sync_post_cpu_write_internal(sync_flags);
}

static inline void gmem_sync_post_cpu_write_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
)
{
   debug_only(gmem_validate_cpu_access_range(handle, offset, length, sync_flags));
   gmem_sync_post_cpu_write_internal(sync_flags);
}

static inline void gmem_sync_post_cpu_write_list(gmem_cpu_sync_list *sync_list)
{
   gmem_sync_post_cpu_write_internal((gmem_sync_flags_t)sync_list->flags);
}

/* V3D access functions */
static inline void gmem_v3d_sync_list_init(struct gmem_v3d_sync_list *sync_list)
{
   sync_list->sync_cpu_write = &gmem_sync_cpu_write;
   sync_list->flags = 0;
}

static inline void gmem_v3d_sync_list_destroy(struct gmem_v3d_sync_list *sync_list)
{
}

static inline void gmem_v3d_sync_list_add(struct gmem_v3d_sync_list *sync_list,
   gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   debug_only(gmem_validate_v3d_access(handle, sync_flags));
   sync_list->flags |= sync_flags;

   gmem_alloc_item* item = gmem_validate_handle(handle);
   if (sync_flags & GMEM_SYNC_V3D_WRITE)
      __atomic_store_n(&item->v3d_writes, 1, __ATOMIC_RELAXED);
}

static inline void gmem_v3d_sync_list_add_range(struct gmem_v3d_sync_list *sync_list,
   gmem_handle_t handle, size_t offset, size_t length, gmem_sync_flags_t sync_flags)
{
   gmem_v3d_sync_list_add(sync_list, handle, sync_flags);
}
