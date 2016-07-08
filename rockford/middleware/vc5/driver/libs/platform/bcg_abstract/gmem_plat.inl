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

/* V3D access functions */
static inline void gmem_v3d_sync_list_init(struct gmem_v3d_sync_list *sync_list)
{
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
