/***************************************************************************
 *  Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ************************************************************/

#include "nexus_types.h"
#include "nexus_base_driver.h"
#include "nexus_base_os.h"
#include "nexus_base_os_types.h"
#include "nexus_platform_priv.h"
#include "nexus_generic_driver_impl.h"
#include "bkni.h"
#include "blst_slist.h"

BDBG_MODULE(nexus_driver_callbacks);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

static BKNI_MutexHandle g_callback_mutex;
#define LOCK()   BKNI_AcquireMutex(g_callback_mutex)
#define UNLOCK() BKNI_ReleaseMutex(g_callback_mutex)

/* unique key that describes callback */
struct nexus_driver_callback_key {
    unsigned type;  /* callback type, e.g. id */
    const void *object; /* callback handle */
};

struct nexus_driver_callback_data {
    NEXUS_Callback callback; /* Function pointer */
    void *context;           /* First parameter to callback function. */
};

struct nexus_driver_callback_map;
struct nexus_driver_callback_scheduler;

BDBG_OBJECT_ID(nexus_driver_callback_entry);
struct nexus_driver_callback_entry {
    /* this structure is managed in the module context, except as noted below */
    BDBG_OBJECT(nexus_driver_callback_entry)
    BLST_S_ENTRY(nexus_driver_callback_entry) list;
    struct nexus_driver_callback_scheduler *scheduler;
    const struct b_objdb_client *client;
    struct nexus_driver_slave_scheduler *slave;
    struct nexus_driver_callback_key key;
    struct nexus_driver_callback_data now,prev; /* store current and previous data, previous is used when need to restore mapping */
    /* members below are managed in the global context */
    bool fired;
    bool stopped;
    bool zombie;
    BLST_S_ENTRY(nexus_driver_callback_entry) list_fired; /* entry could be simultaneously in multiple lists */
    int callback_param; /* this is bypass argument, it's passed as is between user/kernel callbacks */
};

BDBG_OBJECT_ID(nexus_driver_callback_map);

BLST_S_HEAD(nexus_driver_entry_list, nexus_driver_callback_entry);

struct nexus_driver_callback_map {
    BDBG_OBJECT(nexus_driver_callback_map)
    struct nexus_driver_entry_list callbacks; /* list of mapped callbacks, managed in the module context */
    struct nexus_driver_module_header *header; /* link back to module, used for debug purposes */
    /* we never free allocated callback map, since it could easily lead to reference of deallocated memory,
     * instead we move un-mapped callbacks into the freelist, and then reused mapping
     * for matching id's and objects */
    struct nexus_driver_entry_list free_callbacks; /* list of un-mapped callbacks, managed in the module context */
};

BDBG_OBJECT_ID(nexus_driver_callback_scheduler);
struct nexus_driver_callback_scheduler {
    struct nexus_driver_entry_list fired_callbacks; /* list of fired callbacks, managed in the global context */
    struct nexus_driver_entry_list zombie_callbacks; /* list of callbacks that were not freed during module close time */
    BKNI_MutexHandle callback_lock; /* callback that is acquired when callback active */
    void *lock_client; /* client that has the callback_lock */
    BDBG_OBJECT(nexus_driver_callback_scheduler)
};

static struct nexus_driver_callback_scheduler nexus_driver_callback_schedulers[NEXUS_ModulePriority_eMax];

BDBG_OBJECT_ID(nexus_driver_slave_scheduler);
struct nexus_driver_slave_scheduler {
    BDBG_OBJECT(nexus_driver_slave_scheduler)
    struct nexus_driver_callback_scheduler scheduler;
    BKNI_EventHandle event;
};

#if !NEXUS_BASE_EXTERNAL_SCHEDULER
/* linuxkernel/nexus_platform_os.c assumes schedulers are created before module init,
so just create stubs here; they will not be used anyway. */
static void
nexus_driver_scheduler_init_one(struct nexus_driver_callback_scheduler *scheduler, unsigned priority)
{
    BERR_Code rc;
    BKNI_Memset(scheduler, 0, sizeof(*scheduler));
    BDBG_OBJECT_SET(scheduler, nexus_driver_callback_scheduler);
    rc = BKNI_CreateMutex(&scheduler->callback_lock);
    if(rc!=BERR_SUCCESS) {(void)BERR_TRACE(rc);}
}
static void
nexus_driver_scheduler_uninit_one(struct nexus_driver_callback_scheduler *scheduler)
{
    BKNI_DestroyMutex(scheduler->callback_lock);
    BDBG_OBJECT_DESTROY(scheduler, nexus_driver_callback_scheduler);
}
static void
nexus_driver_scheduler_init_slave(struct nexus_driver_callback_scheduler *scheduler)
{
    /* init_slave is called on join. uninit not required because it's just a copy. */
    BKNI_Memset(scheduler, 0, sizeof(*scheduler));
    BDBG_OBJECT_SET(scheduler, nexus_driver_callback_scheduler);
    NEXUS_Base_Scheduler_Config config;
    NEXUS_P_Base_GetSchedulerConfig(NEXUS_ModulePriority_eDefault, &config);
    scheduler->callback_lock = config.callback_lock;
}
#else
static void
nexus_driver_scheduler_init_one(struct nexus_driver_callback_scheduler *scheduler, unsigned priority)
{
    NEXUS_Base_Scheduler_Config config;
    BDBG_OBJECT_INIT(scheduler, nexus_driver_callback_scheduler);
    NEXUS_P_Base_GetSchedulerConfig(priority, &config);
    BLST_S_INIT(&scheduler->fired_callbacks);
    BLST_S_INIT(&scheduler->zombie_callbacks);
    BDBG_ASSERT(config.callback_lock);
    scheduler->callback_lock = config.callback_lock;
    return;
}
static void
nexus_driver_scheduler_uninit_one(struct nexus_driver_callback_scheduler *scheduler)
{
    struct nexus_driver_callback_entry *entry;
    /* free callbacks that were referenced by dead code */
    while( NULL != (entry = BLST_S_FIRST(&scheduler->zombie_callbacks))) {
        BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
        BLST_S_REMOVE_HEAD(&scheduler->zombie_callbacks, list);
        BDBG_OBJECT_DESTROY(entry, nexus_driver_callback_entry);
        BKNI_Free(entry);
    }
    /* force mutex release */
    BKNI_TryAcquireMutex(scheduler->callback_lock);
    BKNI_ReleaseMutex(scheduler->callback_lock);
    BDBG_OBJECT_DESTROY(scheduler, nexus_driver_callback_scheduler);
}
static void
nexus_driver_scheduler_init_slave(struct nexus_driver_callback_scheduler *scheduler)
{
    /* init_slave is called on join. uninit not required because it's just a copy. */
    nexus_driver_scheduler_init_one(scheduler, NEXUS_ModulePriority_eDefault);
}
#endif


/*
 * Note on synchronization
 * Functions are called in two contexts:
 * 1. nexus_driver_callback_to_driver_xxx and nexus_driver_callback_to_user called in the module context, with module lock already acquired
 * 2. Rest of functions including  nexus_driver_scheduler_p_queue are freerunning, in these functions we acquire global mutex
 * global mutex is local to this file. we cannot share nexus_driver_state.lock because it would require an unlock/lock
 *
 */
BERR_Code
nexus_driver_scheduler_init(void)
{
    unsigned i;
    BERR_Code rc;

    rc = BKNI_CreateMutex(&g_callback_mutex);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct nexus_driver_callback_scheduler *scheduler = &nexus_driver_callback_schedulers[i];
        nexus_driver_scheduler_init_one(scheduler, i);
    }
    return BERR_SUCCESS;
}

void
nexus_driver_scheduler_uninit(void)
{
    unsigned i;

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct nexus_driver_callback_scheduler *scheduler = &nexus_driver_callback_schedulers[i];
        nexus_driver_scheduler_uninit_one(scheduler);
    }
    BKNI_DestroyMutex(g_callback_mutex);
    g_callback_mutex = NULL;
    /* there is no need to free entries from the fired_callbacks, this list only used for accounting */
    return;
}

/**
This is the function called directly by the nexus callback.
**/
static void
nexus_driver_scheduler_p_queue(void *context, int param)
{
    struct nexus_driver_callback_entry *entry = context;
    struct nexus_driver_callback_scheduler *scheduler;

    LOCK();
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    if (entry->zombie || !entry->key.object) {
        goto done;
    }
    entry->callback_param = param;
    if(!entry->fired) {
        /* only insert into queue if wasn't there previously */
        entry->fired = true;
        if(entry->slave==NULL) {
            scheduler = entry->scheduler;
            BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
            BLST_S_INSERT_HEAD(&scheduler->fired_callbacks, entry, list_fired);
            BDBG_MSG_TRACE(("nexus_driver_scheduler_p_queue:%#lx callback:%#lx context:%#lx param:%d", (unsigned long)scheduler, (unsigned long)entry->now.callback, (unsigned long)entry->now.context, entry->callback_param));
        } else {
            BDBG_OBJECT_ASSERT(entry->slave, nexus_driver_slave_scheduler);
            scheduler = &entry->slave->scheduler;
            BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
            BLST_S_INSERT_HEAD(&scheduler->fired_callbacks, entry, list_fired);
            BDBG_MSG_TRACE(("nexus_driver_scheduler_p_queue:%#lx callback:%#lx context:%#lx param:%d [slave]", (unsigned long)scheduler, (unsigned long)entry->now.callback, (unsigned long)entry->now.context, entry->callback_param));
            BKNI_SetEvent(entry->slave->event);
        }
    }
done:
    UNLOCK();
    return;
}

static bool
nexus_driver_scheduler_p_check_callback(void *scheduler_)
{
    struct nexus_driver_callback_scheduler *scheduler = scheduler_;
    return BLST_S_FIRST(&scheduler->fired_callbacks) != NULL;
}

/* this function is blocking and drives the scheduler state machine */
int
nexus_driver_run_scheduler(NEXUS_ModulePriority priority, unsigned timeout, bool *p_has_callbacks, struct nexus_driver_slave_scheduler *slave)
{
    struct nexus_driver_callback_scheduler *scheduler;

    *p_has_callbacks = false;
    if (priority >= NEXUS_ModulePriority_eMax) {
        return -1;
    }

    if(slave==NULL) {
#if NEXUS_BASE_EXTERNAL_SCHEDULER
        NEXUS_P_Base_Scheduler_Status status;

        scheduler = &nexus_driver_callback_schedulers[priority];
        BDBG_MSG_TRACE(("nexus_driver_run_scheduler>:%#lx(%u)", (unsigned long)scheduler, (unsigned)priority));

        NEXUS_P_Base_ExternalScheduler_Step(priority, timeout, &status, nexus_driver_scheduler_p_check_callback, scheduler);
        if(status.exit) {
            /* no need to fail. the proxy schedulers will shutdown themselves. */
            *p_has_callbacks = 0;
            return 0;
        }
#else
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
#endif
    } else {
        int rc;
        
        BDBG_OBJECT_ASSERT(slave, nexus_driver_slave_scheduler);
        scheduler = &slave->scheduler;
        BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
        rc = BKNI_WaitForEvent(slave->event, 100);
        if (rc == BERR_TIMEOUT) {
            /* BKNI_WaitForEvent timeout is ok. this avoids a busy loop, but is more responsive than a sleep */
            rc = 0;
        }
        else if (rc) {
            /* non-zero normal on abnormal termination of application */
            return 0; /* don't return rc. return "no callbacks". */
        }
    }
    
    LOCK();
    *p_has_callbacks = (BLST_S_FIRST(&scheduler->fired_callbacks) != NULL);
    UNLOCK();
    
    return 0;
}

/* this function is non-blocking and returns available callbacks. */
unsigned
nexus_driver_scheduler_dequeue(NEXUS_ModulePriority priority, nexus_driver_callback_desc *desc, unsigned nentries, struct nexus_driver_slave_scheduler *slave, bool compat)
{
    struct nexus_driver_callback_scheduler *scheduler;
    unsigned count;
    struct nexus_driver_callback_entry *entry;

    BSTD_UNUSED(compat);
    
    if (priority >= NEXUS_ModulePriority_eMax) {
        return 0;
    }
    if(slave==NULL) {
        scheduler = &nexus_driver_callback_schedulers[priority];
    } else {
        scheduler = &slave->scheduler;
    }
    
    LOCK();
    for( count = 0; NULL != (entry = BLST_S_FIRST(&scheduler->fired_callbacks)) && count < nentries;)  {
        BLST_S_REMOVE_HEAD(&scheduler->fired_callbacks, list_fired);
        entry->fired = false;
        if(entry->stopped) {
            entry->stopped = false;
            continue;
        }
        desc[count].desc.callback = (unsigned long)entry->now.callback;
        desc[count].desc.context = (unsigned long)entry->now.context;
        desc[count].desc.param = entry->callback_param;
#if NEXUS_COMPAT_32ABI
        if(compat) {
            NEXUS_BaseObjectId id;
            NEXUS_Error rc = NEXUS_BaseObject_GetId(entry->key.object, &id);
            desc[count].interfaceHandle = id;
            if(rc!=NEXUS_SUCCESS) {
                (void)BERR_TRACE(rc);
            }
        } else
#endif
        {
            desc[count].interfaceHandle = (unsigned long)(void *)entry->key.object;
        }
        count++;
        BDBG_MSG_TRACE(("nexus_driver_scheduler_dequeue:%#lx(%u) callback:%#lx context:%#lx param:%d", (unsigned long)scheduler, (unsigned)priority, (unsigned long)entry->now.callback, (unsigned long)entry->now.context, entry->callback_param));
    }
    UNLOCK();
    BDBG_MSG_TRACE(("nexus_driver_scheduler_dequeue<:%#lx(%u) %d" , (unsigned long)scheduler, (unsigned)priority, count));
    return count;
}

static void
NEXUS_P_Proxy_MarkCallbacks( void *interfaceHandle, bool stopped )
{
    unsigned i;
    LOCK();

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct nexus_driver_callback_scheduler *scheduler = &nexus_driver_callback_schedulers[i];
        struct nexus_driver_callback_entry *entry;
        for( entry = BLST_S_FIRST(&scheduler->fired_callbacks); entry ; entry = BLST_S_NEXT(entry, list_fired)) {
            if(entry->key.object == interfaceHandle || interfaceHandle==NULL) {
                entry->stopped = stopped;
            }
        }
    }
    UNLOCK();
    return;
}

void
NEXUS_P_Proxy_StopCallbacks( void *interfaceHandle )
{
    NEXUS_P_Proxy_MarkCallbacks(interfaceHandle, true);
    return;
}

void
NEXUS_P_Proxy_StartCallbacks( void *interfaceHandle )
{
    NEXUS_P_Proxy_MarkCallbacks(interfaceHandle, false);
    return;
}

void
nexus_driver_callback_uninit(struct nexus_driver_module_header *header)
{
    struct nexus_driver_callback_map *map = header->callback_map;

    LOCK();

    if(map) {
        struct nexus_driver_callback_entry *entry;

        BDBG_MSG(("%#lx(%s:%#lx) destroying callback map", (unsigned long)map, map->header->name, (unsigned long)map->header->module));
        BDBG_OBJECT_ASSERT(map, nexus_driver_callback_map);
        while( NULL != (entry = BLST_S_FIRST(&map->callbacks))) {
            BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
            BLST_S_REMOVE_HEAD(&map->callbacks, list);
            BDBG_ASSERT(entry->scheduler);
            BLST_S_INSERT_HEAD(&entry->scheduler->zombie_callbacks, entry, list);
            BDBG_MSG(("nexus_driver_callback_uninit:%s leaking callback entry: %#x:%#lx", map->header->name, entry->key.type, (unsigned long)entry->key.object));
        }
        while( NULL != (entry = BLST_S_FIRST(&map->free_callbacks))) {
            BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
            BLST_S_REMOVE_HEAD(&map->free_callbacks, list);
            if(!entry->fired) {
                BDBG_OBJECT_DESTROY(entry, nexus_driver_callback_entry);
                BKNI_Free(entry);
            } else {
                BDBG_ASSERT(entry->scheduler);
                BLST_S_INSERT_HEAD(&entry->scheduler->zombie_callbacks, entry, list);
                BDBG_WRN(("nexus_driver_callback_uninit:%s leaking callback entry: %#x:%#lx", map->header->name, entry->key.type, (unsigned long)entry->key.object));
            }
        }
        BKNI_Free(map);
        header->callback_map = NULL;
    }
    UNLOCK();
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
nexus_driver_p_callback_find_entry_locked(const struct nexus_driver_callback_map *map, const void *handle, unsigned id)
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
    if(entry->fired) {
        struct nexus_driver_callback_scheduler *scheduler;
        entry->fired = false;
        if(entry->slave==NULL) {
            scheduler = entry->scheduler;
        }
        else {
            scheduler = &entry->slave->scheduler;
        }
        BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
        BLST_S_REMOVE(&scheduler->fired_callbacks, entry, nexus_driver_callback_entry, list_fired);
    }
    
	/* the callback may not be removed from the interface. if it fires, we need it to bounce off. */
    entry->slave = NULL;
    entry->client = NULL;
    entry->key.object = NULL;
    
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
nexus_driver_p_callback_get_entry_locked(const struct nexus_driver_module_header *header, struct nexus_driver_callback_map *map, const void *handle, unsigned id,
    const struct b_objdb_client *client, struct nexus_driver_slave_scheduler *slave)
{
    struct nexus_driver_callback_entry *entry, *prev, *id_match, *prev_id;
    struct nexus_driver_callback_scheduler *scheduler;

    /* 1. find already mapped entry */
    entry = nexus_driver_p_callback_find_entry_locked(map, handle, id);
    if(entry) {
#if 0
    /* don't verify the client is the same. for protected clients, they can't set a callback that isn't theirs.
    for unprotected clients, we shouldn't prevent two clients to cooperating on the same struct. 
    remove this code once it has proven itself. */
        if (entry->client != client) {
            BDBG_ERR(("two apps can't register for the same callback for the same handle"));
            return NULL;
        }
#endif
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
        scheduler = entry->scheduler;
    } else {
        entry = BKNI_Malloc(sizeof(*entry));
        if(entry) {
            NEXUS_ModulePriority priority;
            BDBG_ASSERT(header->module);
            NEXUS_Module_GetPriority(header->module, &priority);
            scheduler = &nexus_driver_callback_schedulers[priority];
        }
    }
    if(entry) {
        BDBG_OBJECT_INIT(entry, nexus_driver_callback_entry);
        entry->key.type = id;
        entry->key.object = handle;


        entry->now.callback = NULL;
        entry->prev.callback = NULL;
        entry->now.context = NULL;
        entry->prev.context = NULL;
        entry->fired = false;
        entry->stopped = false;
        entry->zombie = false;
        BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
        entry->scheduler = scheduler;
        if (slave) {
            BDBG_OBJECT_ASSERT(slave, nexus_driver_slave_scheduler);
        }
        entry->client = client;
        entry->slave = slave;
        BLST_S_INSERT_HEAD(&map->callbacks, entry, list);
        BDBG_MSG(("%#lx(%s:%#lx) new callback entry for %#x:%#lx", (unsigned long)map, map->header->name, (unsigned long)map->header->module, id, (unsigned long)handle));
    }
    return entry;
}


void
nexus_driver_callback_to_driver(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id, 
    const struct b_objdb_client *client, struct nexus_driver_slave_scheduler *slave)
{
    struct nexus_driver_callback_map *map;
    struct nexus_driver_callback_entry *entry;

    if(callback->callback==NULL && header->callback_map == NULL) {
        return; /* no need to map empty callbacks, if there no entries */
    }

    LOCK();
    map = nexus_driver_callback_map_create_locked(header);
    if(!map) {goto err_map; }
    if(callback->callback==NULL) {
        /* if callback empty we need special processing only if mapping already exists */
        entry = nexus_driver_p_callback_find_entry_locked(map, handle, id);
        if(!entry) { goto done; }
    } else {
        entry = nexus_driver_p_callback_get_entry_locked(header, map, handle, id, client, slave);
        if(!entry) { goto err_entry; }
    }
    entry->prev = entry->now; /* save entry */
    entry->now.callback = callback->callback;
    entry->now.context = callback->context;

    if(callback->callback) {
        /* replace not empty callback with our information */
        callback->context = entry;
        callback->callback = nexus_driver_scheduler_p_queue;
    }
done:
    UNLOCK();
    return;

err_entry:
err_map:
    BDBG_WRN(("nexus_driver_callback_to_driver: (%s:%#lx) not enough resources to map callback %#x:%#lx", header->name, (unsigned long)header->module, (unsigned)id, (unsigned long)handle));
    callback->callback = NULL; /* clear callback */
    goto done;
}

void
nexus_driver_callback_to_user(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    struct nexus_driver_callback_map *map;
    struct nexus_driver_callback_entry *entry;

    if(callback->callback==NULL) {
        return; /* no need to map empty callbacks */
    }

    LOCK();
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
    UNLOCK();
    return;

err_map:
    callback->callback=NULL;
    goto done;
}

void
nexus_driver_callback_update(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *old_handle, unsigned id, void *new_handle)
{
    struct nexus_driver_callback_entry *entry;

    if(callback->callback == NULL) { 
        return;
    }
    if(callback->callback != nexus_driver_scheduler_p_queue) { goto err_calback;}
    entry=callback->context;
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    BDBG_ASSERT(entry->key.object == old_handle && entry->key.type == id); /* verify a mapping */
    /* replace old object, we could do that because we use simple unordered linked list */
    entry->key.object = new_handle;
done:
    return;

err_calback:
    BDBG_WRN(("nexus_driver_callback_update: %s unknown handler %p for %#x:%p", header->name, (void *)(unsigned long)callback->callback, (unsigned)id, (void *)new_handle));
    goto done;
}

/* if function failed, we need to restore callback mapping */
void
nexus_driver_callback_to_driver_cancel(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    struct nexus_driver_callback_entry *entry;

    if(callback->callback == NULL) { 
        return;
    }

    LOCK();
    if(callback->callback != nexus_driver_scheduler_p_queue) { goto err_calback;}
    entry=callback->context;
    BDBG_OBJECT_ASSERT(entry, nexus_driver_callback_entry);
    BDBG_ASSERT(entry->key.object == handle && entry->key.type == id); /* verify a mapping */
    entry->now = entry->prev; /* restore entry */
    if(entry->now.callback==NULL) { /* if was empty callback prior to function call, need to remove it from the map */
        if(!header->callback_map) {goto err_map; }
        nexus_driver_p_callback_remove_entry_locked(header->callback_map, entry);
    }
done:
    UNLOCK();
    return;

err_calback:
    BDBG_WRN(("nexus_driver_callback_to_driver_cancel: %s unknown handler %p for %#x:%p", header->name, (void *)(unsigned long)callback->callback, (unsigned)id, (void *)handle));
err_map:
    goto done;
}

void
nexus_driver_callback_to_driver_commit(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id)
{
    LOCK();
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
    UNLOCK();
    return;
}

void
nexus_driver_deactivate_callbacks(void *context, void *object, const struct b_objdb_client *client)
{
    struct nexus_driver_module_header *header = context;
    struct nexus_driver_callback_map *map;

    LOCK();
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
                BDBG_MSG(("nexus_driver_scheduler_deactivate_object: %s(%#lx) deactivating callback %#x:%#lx (%#lx)", header->name, (unsigned long)header->module, entry->key.type, (unsigned long)object, (unsigned long)entry ));
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
    UNLOCK();
    return;
}

void
nexus_driver_scheduler_lock(void *client, NEXUS_ModulePriority priority, bool lock)
{
    struct nexus_driver_callback_scheduler *scheduler = &nexus_driver_callback_schedulers[priority];
    if(lock) {
        BKNI_AcquireMutex(scheduler->callback_lock);
        scheduler->lock_client = client;
    } else {
        if (client == scheduler->lock_client) {
            scheduler->lock_client = NULL;
            BKNI_ReleaseMutex(scheduler->callback_lock);
        }
        else {
            BDBG_ERR(("nexus_driver_scheduler_lock rejecting client %p unlocking for client %p", client, scheduler->lock_client));
        }
    }
    return;
}

/**
The client is exiting, so ensure that it doesn't hold the lock on any scheduler.
**/
void
nexus_driver_unlock_schedulers(void *client)
{
    unsigned i;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct nexus_driver_callback_scheduler *scheduler = &nexus_driver_callback_schedulers[i];
        if (scheduler->lock_client == client) {
            scheduler->lock_client = NULL;
            BKNI_ReleaseMutex(scheduler->callback_lock);
        }
    }
    return;
}

struct nexus_driver_slave_scheduler *
nexus_driver_slave_scheduler_create(void)
{
    NEXUS_Error rc;
    struct nexus_driver_slave_scheduler *slave;

    slave = BKNI_Malloc(sizeof(*slave));
    if(!slave) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    BDBG_OBJECT_INIT(slave, nexus_driver_slave_scheduler);
    rc = BKNI_CreateEvent(&slave->event);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_event;}

    nexus_driver_scheduler_init_slave(&slave->scheduler);
    return slave;


err_event:
    BKNI_Free(slave);
err_alloc:
    return NULL;
}


void
nexus_driver_slave_scheduler_release(struct nexus_driver_module_header *header, struct nexus_driver_slave_scheduler *slave)
{
    BDBG_OBJECT_ASSERT(slave, nexus_driver_slave_scheduler);
    LOCK();

    /* don't leave any entries tied to this slave scheduler */
    if (header && header->callback_map) {
        struct nexus_driver_callback_entry *entry;
        for (entry = BLST_S_FIRST(&header->callback_map->callbacks); entry; entry = BLST_S_NEXT(entry, list)) {
            if (entry->slave == slave) {
                /* we can't recycle the entry until the post-close cleanup removes the nexus object.
                setting zombie = true causes any callbacks to bounce off until then. */
                entry->zombie = true;

                /* must remove from fired_callbacks because ref_cnt might be >1 and we can't have client pick this up. */
                if (entry->fired) {
                    struct nexus_driver_callback_scheduler *scheduler = &entry->slave->scheduler;
                    entry->fired = false;
                    BDBG_OBJECT_ASSERT(scheduler, nexus_driver_callback_scheduler);
                    BLST_S_REMOVE(&scheduler->fired_callbacks, entry, nexus_driver_callback_entry, list_fired);
                }

                /* prevent this from being removed from regular scheduler's fired_callbacks */
                entry->slave = NULL;
            }
        }
    }
    UNLOCK();
    return;
}

void
nexus_driver_slave_scheduler_destroy(struct nexus_driver_module_header *header, struct nexus_driver_slave_scheduler *slave)
{
    LOCK();
    BKNI_DestroyEvent(slave->event);
    BDBG_OBJECT_DESTROY(slave, nexus_driver_slave_scheduler);
    BKNI_Free(slave);
    UNLOCK();
    return;
}
