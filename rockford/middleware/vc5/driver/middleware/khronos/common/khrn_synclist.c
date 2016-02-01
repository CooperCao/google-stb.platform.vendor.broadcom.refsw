/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  sync-list util functions

FILE DESCRIPTION
sync-list util functions
=============================================================================*/
#include "middleware/khronos/common/khrn_synclist.h"
#include <stdlib.h>
#include <assert.h>

#define REALLOC_ITEMS_SIZE  20
void khrn_synclist_init(struct khrn_synclist *list)
{
#ifdef KHRN_VALIDATE_SYNCLIST
    memset(&list->handles, 0, sizeof(list->handles));
#endif
    gmem_v3d_sync_list_init(&list->sync_list);
}

#ifdef KHRN_VALIDATE_SYNCLIST
void khrn_synclist_record_access(
   struct khrn_synclist *list,
   gmem_handle_t handle,
   uint32_t offset,
   uint32_t length,
   gmem_sync_flags_t sync_flags)
{
    struct khrn_handle_list *hlist = &list->handles;
    struct khrn_handle_list_entry *entry;
    void *ptr;
    if (hlist->count == hlist->max_size) {
        ptr = realloc(hlist->array, (hlist->count + REALLOC_ITEMS_SIZE) *
                      sizeof (struct khrn_handle_list_entry));
        /* No method for reporting failure here but the driver doesn't want
         * failure paths for this kind of validation layer failure.
         */
        assert (ptr != NULL);

        hlist->array = ptr;
        hlist->max_size += REALLOC_ITEMS_SIZE;
    }
    entry = &hlist->array[hlist->count++];
    entry->handle = handle;
    entry->offset = offset;
    entry->length = length;
    entry->sync_flags = sync_flags;
}
#endif

void khrn_synclist_destroy(struct khrn_synclist *list)
{
#ifdef KHRN_VALIDATE_SYNCLIST
    struct khrn_handle_list *hlist = &list->handles;
    if (hlist->array != NULL)
        free(hlist->array);
    memset(&list->handles, 0, sizeof(list->handles));
#endif
    gmem_v3d_sync_list_destroy(&list->sync_list);
}
