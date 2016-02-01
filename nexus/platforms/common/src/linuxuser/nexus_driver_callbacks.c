/***************************************************************************
 *     (c)2004-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ************************************************************/

#include "nexus_types.h"
#include "nexus_base_driver.h"
#include "nexus_base_os.h"
#include "nexus_base_os_types.h"
#include "nexus_platform_priv.h"
#include "nexus_driver.h"
#include "bkni.h"
#include "blst_slist.h"

BDBG_MODULE(nexus_driver_callbacks);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

/* unique key that describes callback */
struct nexus_driver_callback_key {
    unsigned type;  /* callback type, e.g. id */
    void *object; /* callback handle */
};

struct nexus_driver_callback_data {
    NEXUS_Callback callback; /* Function pointer */
    void *context;           /* First parameter to callback function. */
};

struct nexus_driver_callback_map;

BDBG_OBJECT_ID(nexus_driver_callback_entry);
struct nexus_driver_callback_entry {
    /* this structure is managed in the module context, except as noted below */
    BDBG_OBJECT(nexus_driver_callback_entry)
    BLST_S_ENTRY(nexus_driver_callback_entry) list;
    struct nexus_driver_callback_key key;
    struct nexus_driver_callback_data now,prev; /* store current and previous data, previous is used when need to restore mapping */
    int callback_param; /* this is bypass argument, it's passed as is between user/kernel callbacks */
    void *client;
    void *context;
};

BDBG_OBJECT_ID(nexus_driver_callback_map);

BLST_S_HEAD(nexus_driver_entry_list, nexus_driver_callback_entry);

/* one map per module */
struct nexus_driver_callback_map {
    BDBG_OBJECT(nexus_driver_callback_map)
    struct nexus_driver_entry_list callbacks; /* list of mapped callbacks, managed in the module context */
    struct nexus_driver_module_header *header; /* link back to module, used for debug purposes */
    /* we never free allocated callback map, since it could easily lead to reference of deallocated memory,
     * instead we move un-mapped callbacks into the freelist, and then reused mapping
     * for matching id's and objects */
    struct nexus_driver_entry_list free_callbacks; /* list of un-mapped callbacks, managed in the module context */
};

/* this is the callback that is replaced */
static void nexus_driver_p_process_callback(void *context, int param)
{
    struct nexus_driver_callback_entry *entry = context;
    NEXUS_CallbackDesc callback;
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    if (entry->context) {
        callback.callback = entry->now.callback;
        callback.context = entry->now.context;
        callback.param = param;
        nexus_server_send_callback(entry->context, entry->key.object, (size_t)entry, &callback);
    }
}

void
nexus_driver_callback_uninit(struct nexus_driver_module_header *header)
{
    struct nexus_driver_callback_map *map = header->callback_map;

    if(map) {
        struct nexus_driver_callback_entry *entry;

        BDBG_MSG(("%#lx(%s:%#lx) destroying callback map", (unsigned long)map, map->header->name, (unsigned long)map->header->module));
        BDBG_OBJECT_ASSERT(map, nexus_driver_callback_map);
        while( NULL != (entry = BLST_S_FIRST(&map->callbacks))) {
            BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
            BLST_S_REMOVE_HEAD(&map->callbacks, list);
            BDBG_OBJECT_DESTROY(entry, nexus_driver_callback_entry);
            BKNI_Free(entry);
        }
        while( NULL != (entry = BLST_S_FIRST(&map->free_callbacks))) {
            BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
            BLST_S_REMOVE_HEAD(&map->free_callbacks, list);
            BDBG_OBJECT_DESTROY(entry, nexus_driver_callback_entry);
            BKNI_Free(entry);
        }
        BKNI_Free(map);
        header->callback_map = NULL;
    }
    return;
}

static struct nexus_driver_callback_map *
nexus_driver_callback_map_create_locked(struct nexus_driver_module_header *header)
{
    struct nexus_driver_callback_map *map = header->callback_map;

    if(map) {
        return map;
    }
    map = BKNI_Malloc(sizeof(*map));
    if(map==NULL) { goto err_alloc; }
    BDBG_OBJECT_INIT(map, nexus_driver_callback_map);
    BLST_S_INIT(&map->callbacks);
    BLST_S_INIT(&map->free_callbacks);
    map->header = header;
    header->callback_map = map;
    BDBG_MSG(("%#lx(%s:%#lx) new callback map", (unsigned long)map, map->header->name, (unsigned long)map->header->module));
    return map;
err_alloc:
    return NULL;
}

/* returns an existing entry */
static struct nexus_driver_callback_entry *
nexus_driver_p_callback_find_entry_locked(const struct nexus_driver_callback_map *map, void *handle, unsigned id)
{
    struct nexus_driver_callback_entry *entry;

    /* all callback xxx functions are executed in context of their own modules, and synchronized, that means we don't need to perform extra synchronization */
    for( entry = BLST_S_FIRST(&map->callbacks); entry ; entry = BLST_S_NEXT(entry, list)) {
        BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
        if(entry->key.type == id && entry->key.object == handle) {
            return entry;
        }
    }
    return NULL;
}

static void
nexus_driver_p_callback_recycle_entry_locked(struct nexus_driver_callback_map *map, struct nexus_driver_callback_entry *entry)
{
    BDBG_OBJECT_ASSERT(map, nexus_driver_callback_map);
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    BLST_S_INSERT_HEAD(&map->free_callbacks, entry, list);
    entry->now.callback = NULL;
    entry->prev.callback = NULL;
    entry->now.context = NULL;
    entry->prev.context = NULL;
    return;
}

static void
nexus_driver_p_callback_remove_entry_locked(struct nexus_driver_callback_map *map, struct nexus_driver_callback_entry *entry)
{
    BDBG_MSG(("%#lx(%s:%#lx) remove callback entry %#x:%#lx", (unsigned long)map, map->header->name, (unsigned long)map->header->module, entry->key.type, (unsigned long)entry->key.object));
    BLST_S_REMOVE(&map->callbacks, entry, nexus_driver_callback_entry, list);
    nexus_driver_p_callback_recycle_entry_locked(map, entry);
    return;
}

/* returns an existing entry or, if not found, creates a new one */
static struct nexus_driver_callback_entry *
nexus_driver_p_callback_get_entry_locked(struct nexus_driver_callback_map *map, void *handle, unsigned id, struct nexus_driver_client_state *client, void *context)
{
    struct nexus_driver_callback_entry *entry, *prev, *id_match, *prev_id;

    /* NOTE: handle may be NULL for an open function. */

    /* 1. find already mapped entry */
    entry = nexus_driver_p_callback_find_entry_locked(map, handle, id);
    if(entry) {
        if (entry->client != client) {
            BDBG_ERR(("two apps can't register for the same callback for the same handle: %p %p %p", (void *)entry, (void *)entry->client, (void *)client));
            return NULL;
        }
        return entry;
    }
    /* 2. Search for exact match in freed entries */
    for( id_match = prev = prev_id = NULL, entry = BLST_S_FIRST(&map->free_callbacks); entry ; entry = BLST_S_NEXT(entry, list)) {
        if(entry->key.type == id) {
            if( entry->key.object == handle) {
                break;
            }
            id_match = entry; /* save partial match */
            prev_id = prev;
        }
        prev = entry;
    }
    if(entry==NULL) {
        /* 3. Use id match in freed entries */
        entry = id_match;
        prev = prev_id;
    }
    if(entry) {
        BDBG_MSG(("%#lx(%s:%#lx) reusing old callback entry %#x:%#lx for %#x:%#lx", (unsigned long)map, map->header->name, (unsigned long)map->header->module, entry->key.type, (unsigned long)entry->key.object, id, (unsigned long)handle));
        /* Remove entry from the list */
        if(prev) {
            BLST_S_REMOVE_NEXT(&map->free_callbacks, prev, list);
        } else {
            BLST_S_REMOVE_HEAD(&map->free_callbacks, list);
        }
    } else {
        entry = BKNI_Malloc(sizeof(*entry));
    }
    if(entry) {
        BDBG_OBJECT_INIT(entry, nexus_driver_callback_entry);
        entry->key.type = id;
        entry->key.object = handle;
        entry->now.callback = NULL;
        entry->prev.callback = NULL;
        entry->now.context = NULL;
        entry->prev.context = NULL;
        entry->client = client;
        entry->context = context;
        BLST_S_INSERT_HEAD(&map->callbacks, entry, list);
        BDBG_MSG(("%#lx(%s:%#lx) new callback entry for %#x:%#lx", (unsigned long)map, map->header->name, (unsigned long)map->header->module, id, (unsigned long)handle));
    }
    return entry;
}


void
nexus_driver_callback_to_driver(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id, void *client, void *context)
{
    struct nexus_driver_callback_map *map;
    struct nexus_driver_callback_entry *entry;

    /* no need to map empty callbacks */
    if (callback->callback==NULL) { /* TODO: restore code? */
        return;
    }

    map = nexus_driver_callback_map_create_locked(header);
    if(!map) {goto err_map; }

    entry = nexus_driver_p_callback_get_entry_locked(map, handle, id, client, context);
    if(!entry) { goto err_entry; }

    entry->prev = entry->now; /* save entry */
    entry->now.callback = callback->callback;
    entry->now.context = callback->context;

    if (callback->callback) {
        /* replace not empty callback with our information */
        callback->callback = nexus_driver_p_process_callback;
        callback->context = entry;
    }
done:
    return;

err_entry:
err_map:
    BDBG_WRN(("nexus_driver_callback_to_driver: (%s:%#lx) cannot map callback %#x:%#lx", header->name, (unsigned long)header->module, (unsigned)id, (unsigned long)handle));
    callback->callback = NULL; /* clear callback */
    goto done;
}

void
nexus_driver_callback_to_user(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    struct nexus_driver_callback_map *map;
    struct nexus_driver_callback_entry *entry;

    if (callback->callback==NULL) {
        return; /* no need to map empty callbacks */
    }

    map = nexus_driver_callback_map_create_locked(header);
    if(!map) {
        BDBG_WRN(("nexus_driver_callback_to_user: (%s:%#lx) nexus_driver_callback_map_create_locked failed", header->name, (unsigned long)header->module));
        goto err_map;
    }
    entry = nexus_driver_p_callback_find_entry_locked(map, handle, id);
    if (!entry) {
        /* in unprotected mode, callback may have been set by previous app. the entry will be deleted when the client exits, but the nexus module will still have
        the dangling callback. this will return NULL to the app when it calls Get. if the app calls Set, it will overwrite the state inside the nexus module. 
        the callback will work. */
        callback->context = NULL;
        callback->callback = NULL;
    }
    else {
        callback->context = entry->now.context;
        callback->callback = entry->now.callback;
    }
done:
    return;

err_map:
    callback->callback=NULL;
    goto done;
}

void
nexus_driver_callback_update(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *old_handle, unsigned id, void *new_handle)
{
    struct nexus_driver_callback_entry *entry;

    BSTD_UNUSED(header);
    if(callback->callback == NULL) { goto done; }
    if(callback->callback != nexus_driver_p_process_callback) { goto err_calback;}
    entry=callback->context;
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    BDBG_ASSERT(entry->key.object == old_handle && entry->key.type == id); /* verify a mapping */
    /* replace old object, we could do that because we use simple unordered linked list */
    BDBG_MSG(("nexus_driver_callback_update %p = %p", entry->key.object, new_handle));
    entry->key.object = new_handle;
done:
    return;

err_calback:
    BDBG_WRN(("nexus_driver_callback_update: %s unknown handler %p for %#x:%p", header->name, (void *)(long)callback->callback, id, (void *)new_handle));
    goto done;
}

/* if function failed, we need to restore callback mapping */
void
nexus_driver_callback_to_driver_cancel(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    struct nexus_driver_callback_entry *entry;

    if(callback->callback == NULL) { goto done; }

    if(callback->callback != nexus_driver_p_process_callback) { goto err_calback;}
    entry=callback->context;
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    BDBG_ASSERT(entry->key.object == handle && entry->key.type == id); /* verify a mapping */
    entry->now = entry->prev; /* restore entry */
    if(entry->now.callback==NULL) { /* if was empty callback prior to function call, need to remove it from the map */
        if(!header->callback_map) {goto err_map; }
        nexus_driver_p_callback_remove_entry_locked(header->callback_map, entry);
    }
done:
    return;

err_calback:
    BDBG_WRN(("nexus_driver_callback_to_driver_cancel: %s unknown handler %p for %#x:%p", header->name, (void *)(long)callback->callback, id, (void *)handle));
err_map:
    goto done;
}

void
nexus_driver_callback_to_driver_commit(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    if(callback->callback == NULL) {
        struct nexus_driver_callback_map *map = header->callback_map;
        if(map) {
            struct nexus_driver_callback_entry *entry = nexus_driver_p_callback_find_entry_locked(map, handle, id);
            if(entry) {
                /* if callback cleared out, then remove entry from the map */
                nexus_driver_p_callback_remove_entry_locked(map, entry);
            }
        }
    }
    return;

}

/* called from objdb callback with module lock already acquired */
void
nexus_driver_deactivate_callbacks(void *context, void *object, const struct b_objdb_client *client)
{
    struct nexus_driver_module_header *header = context;
    struct nexus_driver_callback_map *map;

    map = header->callback_map;
    if(map) {
        struct nexus_driver_callback_entry *entry,*prev,*next;
        for( prev = NULL, entry = BLST_S_FIRST(&map->callbacks); entry ; ) {
            BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
            next = BLST_S_NEXT(entry, list);
            /* if object == NULL, remove all objects for the client (client is being closed)
               if client == NULL, remove the object regardless of ownership (object is being deleted) 
               else, remove object if both client & object matches (only ownership is being released) */
            if ((!object && client && entry->client == client) ||
                (!client && object && entry->key.object == object) ||
                (object && client && entry->key.object == object && entry->client == client)) 
            {
                BDBG_MSG(("nexus_driver_deactivate_callbacks: %s(%#lx) deactivating callback %#x:%#lx (%#lx)", header->name, (unsigned long)header->module, entry->key.type, (unsigned long)object, (unsigned long)entry ));
                entry->context = NULL; /* this callback can no longer fire */
                if(prev) {
                    BLST_S_REMOVE_NEXT(&map->callbacks, prev, list);
                } else {
                    BLST_S_REMOVE_HEAD(&map->callbacks, list);
                }
                nexus_driver_p_callback_recycle_entry_locked(map, entry); /* this would move entry into another list */
                /* continue traversing list */
            } else {
                prev = entry;
            }
            entry = next;
        }
    }
    return;
}
