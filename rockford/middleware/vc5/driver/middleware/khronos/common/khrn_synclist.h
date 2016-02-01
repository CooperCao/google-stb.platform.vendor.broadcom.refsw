/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  sync-list util functions

FILE DESCRIPTION
sync-list util functions
=============================================================================*/
#ifndef _KHRN_SYNCLIST_H_
#define _KHRN_SYNCLIST_H_

#include "interface/khronos/common/khrn_int_common.h"
#include "gmem.h"

struct khrn_handle_list_entry
{
    gmem_handle_t handle;
    size_t offset;
    size_t length;
    gmem_sync_flags_t sync_flags;
};

struct khrn_handle_list
{
    size_t count;
    size_t max_size;
    struct khrn_handle_list_entry *array;
};

struct khrn_synclist
{
    struct gmem_v3d_sync_list sync_list;
#ifdef KHRN_VALIDATE_SYNCLIST
    struct khrn_handle_list handles;
#endif
};

void khrn_synclist_init(struct khrn_synclist *list);

#ifdef KHRN_VALIDATE_SYNCLIST
void khrn_synclist_record_access(
   struct khrn_synclist *list,
   gmem_handle_t handle,
   uint32_t offset,
   uint32_t length,
   gmem_sync_flags_t sync_flags);
#endif

static inline void khrn_synclist_add(
   struct khrn_synclist *list,
   gmem_handle_t handle,
   gmem_sync_flags_t sync_flags
   )
{
#ifdef KHRN_VALIDATE_SYNCLIST
   khrn_synclist_record_access(list, handle, 0, gmem_get_size(handle), sync_flags);
#endif
   gmem_v3d_sync_list_add(&list->sync_list, handle, sync_flags);
}

static inline void khrn_synclist_add_range(
   struct khrn_synclist *list,
   gmem_handle_t handle,
   uint32_t offset,
   uint32_t length,
   gmem_sync_flags_t sync_flags
   )
{
#ifdef KHRN_VALIDATE_SYNCLIST
   khrn_synclist_record_access(list, handle, offset, length, sync_flags);
#endif
   gmem_v3d_sync_list_add_range(&list->sync_list, handle, offset, length, sync_flags);
}

void khrn_synclist_destroy(struct khrn_synclist *list);

#endif /* _KHRN_SYNCLIST_H_ */
