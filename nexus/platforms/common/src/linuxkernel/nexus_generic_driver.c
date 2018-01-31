/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_base_driver.h"
#include "nexus_base_mmap.h"
#include "nexus_base_os.h"
#include "nexus_platform_priv.h"
#include "bkni.h"
#include "nexus_generic_driver_impl.h"
#include "nexus_driver_procfs.h"
#include "nexus_driver_ioctl.h"
#if NEXUS_ABICOMPAT_MODE
#include "nexus_platform_abiverify_ioctl.h" /* allows driver to watch for certain platform ioctls */
#if NEXUS_COMPAT_32ABI
#include "nexus_platform_abiverify_ioctl_compat.h" /* allows driver to watch for certain platform ioctls */
#endif
#else
#include "nexus_platform_ioctl.h" /* allows driver to watch for certain platform ioctls */
#endif
#include "nexus_client_resources.h"
#include "bkni_track_mallocs.h"

/* do not #include linux header files here. this file is generic and must compile with no OS dependency.
if a new OS feature is needed, an API must be added. */

BDBG_MODULE(nexus_generic_driver);

#define BDBG_MSG_TRACE(x) /*BDBG_MSG(x)*/
#define LOCK() BKNI_AcquireMutex(nexus_driver_state.lock)
#define UNLOCK() BKNI_ReleaseMutex(nexus_driver_state.lock)

static int nexus_driver_proxy_ioctl(void *context, unsigned int cmd, unsigned long arg, unsigned long type, bool unlocked, bool compat);
static void nexus_driver_print_kni(void *context);

/* define list of ioctl handlers */
static const struct {
   int (*ioctl)(void *context, unsigned int cmd, unsigned long arg, unsigned long type, bool unlocked, bool compat);
} g_nexus_driver_ioctl_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) { nexus_driver_##module##_ioctl } ,
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
{nexus_driver_proxy_ioctl} /* special entry for callback driver */
};

/* NEXUS_PLATFORM_P_NUM_DRIVERS == the number of modules + 1 for the proxy driver */
#define NEXUS_PLATFORM_P_NUM_DRIVERS  (sizeof(g_nexus_driver_ioctl_handlers)/sizeof(*g_nexus_driver_ioctl_handlers))


/* define list of open/close handlers, we keep open/close handlers separate to not polute ioctl table (ioctl table accessed more ofter and likely to fit in L1 cache) */
static struct {
    int (*open)(unsigned);
    void (*close)(void);
    struct nexus_driver_module_header *header; /* pointer into the module meta-data */
} g_nexus_driver_state_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) {nexus_driver_##module##_open, nexus_driver_##module##_close, NULL},
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
    {NULL, NULL, NULL} /* no special open/close for proxy */
};

static struct module_order {
    unsigned index; /* index to g_nexus_driver_state_handlers[] */
    unsigned order; /* module order, not necessarily packed */
} g_nexus_driver_state_order[NEXUS_PLATFORM_P_NUM_DRIVERS-1];

struct nexus_driver_state nexus_driver_state;
static int      nexus_driver_server_init_lock(unsigned pid);
static int      nexus_driver_server_preinit(void);
static int      nexus_driver_server_postinit_lock(void);
static void     nexus_driver_server_preuninit_lock(void);
static void     nexus_driver_server_uninit_lock(void);
static void     nexus_driver_process_cleanup_queue_lock(void);
static struct   nexus_driver_client_state *nexus_driver_open_client_lock(unsigned pid);
static void     nexus_driver_close_client_lock(struct nexus_driver_client_state *client);
static struct   nexus_driver_client_state *nexus_driver_connect_client_lock(unsigned pid, const NEXUS_Certificate *pCertificate);
static void     nexus_driver_disable_client_lock(struct nexus_driver_client_state *client);
static struct nexus_driver_client_state *nexus_driver_create_client_lock(const NEXUS_Certificate *pCertificate, const NEXUS_ClientConfiguration *pConfig);
static void     nexus_driver_disable_clients_lock(bool including_server);

void
nexus_driver_free(void *addr)
{
    if(addr) {
        BKNI_Free(addr);
    }
    return;
}

/* route all address conversions through dedicated functions */
void
nexus_driver_send_addr(void **paddr)
{
    /* inplace convert address from virtual to physical */
    void *addr = *paddr;
    if(addr) {
        *paddr = (void *)(unsigned long)NEXUS_AddrToOffset(addr);
    }
    return;
}


void
nexus_driver_recv_addr_cached(void **paddr)
{
    void *addr = *paddr;
    /* inplace convert address from physical to virtual */
    if(addr) {
        *paddr = NEXUS_OffsetToCachedAddr((unsigned long)addr);
    }
    return;
}

/* nexus_driver_module_init is called from each module's nexus_driver_module_open handler */
int
nexus_driver_module_init(unsigned index, struct nexus_driver_module_header *header, NEXUS_ModuleHandle module,
        const struct b_objdb_class *class_table, const char *name)
{
    header->entry_count = 0;
    header->callback_map = NULL; /* lazily allocate callback map */
    header->name = name;
    /* module may be NULL if module is compiled in but not initialized by NEXUS_Platform_Init */
    header->module = module;
    b_objdb_add_module(&header->objdb, class_table, module);
    header->objdb.name = name;
    header->objdb.module = module;
    header->objdb.cancel_callbacks_locked = nexus_driver_deactivate_callbacks;
    header->objdb.cancel_callbacks_context = header;
    g_nexus_driver_state_handlers[index].header = header;
    BDBG_MSG(("nexus_driver_module_init: registering %d: %p (%s:%p)", index, (void *)header, name, (void *)module));
    return 0;
}

/* nexus_driver_module_uninit is called from each module's nexus_driver_module_close handler */
void
nexus_driver_module_uninit(struct nexus_driver_module_header *header)
{
    b_objdb_remove_module(&header->objdb, NEXUS_MODULE_SELF);
    nexus_driver_callback_uninit(header);
}

static int
nexus_driver_ioctl_proxy_nexus_open(struct nexus_generic_driver_state *state)
{
    struct nexus_driver_module_driver_state *driver_state = NULL;
    int rc = 0;
    if(state->kind != nexus_generic_driver_kind_uninitialized) {
        return BERR_TRACE(-1);
    }
    BDBG_MSG_TRACE((">>nexus_driver_ioctl_proxy_nexus_open:"));
    /* alloc per-instance data */
    driver_state = BKNI_Malloc(sizeof(*driver_state));
    if(driver_state==NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }
    BDBG_CASSERT(sizeof(driver_state->process_name) == sizeof(state->process_name));
    BDBG_CASSERT(sizeof(driver_state->process_name) == NEXUS_MAX_PROCESS_NAME);
    b_strncpy(driver_state->process_name, state->process_name, NEXUS_MAX_PROCESS_NAME);
    driver_state->pid = state->pid;

    LOCK();

    if (nexus_driver_state.uninit_pending) {
        nexus_driver_server_uninit_lock();
    }

    /* init driver on first open */
    if (!nexus_driver_state.uninit_in_progress && !nexus_driver_state.server) {
        if (!state->trusted) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto lockeddone;
        }
        rc = nexus_driver_server_init_lock(state->pid);
        if (rc) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto lockeddone;
        }
        /* server's client has been created, so nexus_driver_open_client will work. */
    }

    driver_state->client = nexus_driver_open_client_lock(state->pid);
    if (!driver_state->client) {
        if (nexus_driver_state.uninit_in_progress) {
            /* client was not created because we are shutting down. uninit_in_progress must be checked after nexus_driver_open_client
            because the client could be pending inside on the LOCK(). */
            rc = NEXUS_NOT_AVAILABLE; /* don't use BERR_TRACE. this is not an error in the driver. */
            goto lockeddone;
        }
        /* it's normal for nexus_driver_open_client to return NULL until Join ioctl. however, no other driver
        will be opened until Join, so we can populate driver_state->client then. */
    }
    else {
        b_strncpy(driver_state->client->process_name, driver_state->process_name, NEXUS_MAX_PROCESS_NAME);
    }

    nexus_driver_state.open_count++;
    driver_state->slave_scheduler = NULL;

    state->state.nexus_driver = driver_state;
    state->kind = nexus_generic_driver_kind_nexus;

lockeddone:
    UNLOCK();
done:
    if (rc) {
        if (driver_state) {
            BKNI_Free(driver_state);
        }
    }
    BDBG_MSG_TRACE(("<<nexus_driver_ioctl_proxy_nexus_open: -> %d", rc));
    return rc;
}

int
nexus_generic_driver_open(struct nexus_generic_driver_state **pstate, unsigned pid, const char *process_name, bool trusted)
{
    struct nexus_generic_driver_state *driver_state = NULL;
    int rc = 0;

    BDBG_MSG_TRACE((">>nexus_driver_open: %u", module));

    /* alloc per-instance data */
    driver_state = BKNI_Malloc(sizeof(*driver_state));
    if (driver_state==NULL) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    driver_state->pid = pid;
    b_strncpy(driver_state->process_name, process_name?process_name:"", NEXUS_MAX_PROCESS_NAME);
    driver_state->kind = nexus_generic_driver_kind_uninitialized;
    driver_state->trusted = trusted;
    *pstate = driver_state;

    return rc;
}

void nexus_driver_disable_clients(bool including_server)
{
    LOCK();
    nexus_driver_disable_clients_lock(including_server);
    UNLOCK();
}

static void nexus_driver_disable_clients_lock(bool including_server)
{
    struct nexus_driver_client_state *client;
    for (client = BLST_S_FIRST(&nexus_driver_state.clients); client;) {
        struct nexus_driver_client_state *next = BLST_S_NEXT(client, link);
        if (including_server || client != nexus_driver_state.server) {
            nexus_driver_disable_client_lock(client);
        }
        client = next;
    }
}

static void
nexus_driver_nexus_close(struct nexus_driver_module_driver_state *driver_state, bool abnormal_termination)
{
    BDBG_MSG_TRACE((">>nexus_driver_nexus_close: %s", abnormal_termination?"SIGNAL":""));

    BDBG_ASSERT(driver_state);

    LOCK();

    /* may be NULL if client closes before Join */
    if (driver_state->client) {
        nexus_driver_close_client_lock(driver_state->client);
    }

    if (driver_state->slave_scheduler) {
        unsigned i;
        /* Prevent all callbacks for each modules  for this slave scheduler from firing */
        for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
            nexus_driver_slave_scheduler_release(g_nexus_driver_state_handlers[i].header, driver_state->slave_scheduler);
        }
        if (nexus_driver_state.cleanup_pending) {
            nexus_driver_process_cleanup_queue_lock();
        }
        /* Slave scheduler can be disabled even though clean up may not happen.
         * Callbacks are prevented from firing already and can safely be disabled during cleanup */
        nexus_driver_slave_scheduler_destroy(g_nexus_driver_state_handlers[0].header, driver_state->slave_scheduler);
    }

    /* test if the server is closing without a full shutdown */
    if ((driver_state->client == nexus_driver_state.server) && !nexus_driver_state.uninit_in_progress && driver_state->client && !driver_state->client->refcnt) {
        BDBG_WRN(("forcing driver shutdown"));
        /* the server is going down without having shutdown all clients. signal is likely pending.
        the best we can do is to kill all clients, which will then cause the full shutdown. */
        nexus_driver_disable_clients_lock(!abnormal_termination || !nexus_driver_state.active);
    }
    BKNI_Free(driver_state);

    /* if this is the last driver closed, we can do the full shutdown */
    BDBG_ASSERT(nexus_driver_state.open_count);
    if (--nexus_driver_state.open_count == 0) {
        /* we can only uninit if there's no signal pending. if not, we must defer until open or rmmod. */
        if (!abnormal_termination) {
            nexus_driver_server_uninit_lock();
        }
        else {
            nexus_driver_state.uninit_pending = true;
        }
    }
    UNLOCK();

    BDBG_MSG_TRACE(("<<nexus_driver_nexus_close: done"));
    return;
}

void
nexus_generic_driver_close(struct nexus_generic_driver_state *state, bool abnormal_termination)
{
    if(state->kind == nexus_generic_driver_kind_nexus) {
        nexus_driver_nexus_close(state->state.nexus_driver, abnormal_termination);
    }
    BKNI_Free(state);
    return;
}


int nexus_generic_driver_validate_mmap(struct nexus_generic_driver_state *state, uint64_t offset, unsigned size)
{
    struct nexus_driver_module_driver_state *context;
    struct nexus_driver_client_state *client;
    unsigned i;
    bool hasDynamicHeap = false;

    if(state->kind != nexus_generic_driver_kind_nexus) {
        return NEXUS_NOT_AVAILABLE;
    }
    context = state->state.nexus_driver;
    client = context->client;
    /* don't allow mmap without join */
    if (!client) return NEXUS_NOT_AVAILABLE;

    BDBG_MSG(("nexus_generic_driver_validate_mmap " BDBG_UINT64_FMT ", %u", BDBG_UINT64_ARG(offset), size));

    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    BDBG_ASSERT(client->refcnt);

    if (!client->pid) {
        /* client has been disabled by the server */
        return -1; /* don't use BERR_TRACE. this is not an error in the driver. */
    }

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_HeapHandle heap;
        if (client->client.mode == NEXUS_ClientMode_eUntrusted) {
            heap = client->client.config.heap[i];
        } else {
            heap = g_NEXUS_pCoreHandles->heap[i].nexus;
        }
        if(heap) {
            NEXUS_MemoryStatus status;
            NEXUS_Heap_GetStatus_priv(heap, &status);
            BDBG_MSG(("heap:  " BDBG_UINT64_FMT ")%d", BDBG_UINT64_ARG(status.offset), status.size));
            if( (status.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) == NEXUS_MEMORY_TYPE_DYNAMIC) {
                hasDynamicHeap = true;
                continue;
            }
            if (
                ((status.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED||
                 (status.memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED) == NEXUS_MEMORY_TYPE_APPLICATION_CACHED) &&
                 (offset >= status.offset && offset + size <= status.offset + status.size)) {
                /* it's good */
                break;
            }
        }
    }
    if (i == NEXUS_MAX_HEAPS) {
        bool match = false;
        if(hasDynamicHeap) {
            b_memory_region region;
            region.base = offset;
            region.length = size;
            LOCK();
            match = b_memory_region_match(&nexus_driver_state.dynamic_memory, &region);
            UNLOCK();
        }
        if(!match) {
            /* not within a heap, so we fail */
            BDBG_ERR(("nexus_driver_mmap: no matching client heap found"));
            return -1;
        }
    }
    /* it's good */
    return 0;
}

int
nexus_generic_driver_ioctl(unsigned module, void *context_, unsigned int cmd, unsigned long arg, bool compat)
{
    int rc;
    struct nexus_generic_driver_state *state = context_;
    struct nexus_driver_module_driver_state *context;
    struct nexus_driver_client_state *client;
    bool unlocked = false;
    bool lock_standby = false;

    switch(cmd) {
    case IOCTL_PROXY_NEXUS_Open:
         rc = nexus_driver_ioctl_proxy_nexus_open(state);
         return rc;
    default:
         break;
    }
    if(state->kind != nexus_generic_driver_kind_nexus) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return -1;
    }
    context = state->state.nexus_driver;
    client = context->client;

    if (module == NEXUS_IOCTL_PROXY_MODULE) {
        module = NEXUS_PLATFORM_P_NUM_DRIVERS-1;
    }
    BDBG_MSG_TRACE((">>nexus_driver_ioctl %u %#x %#lx %s", module, cmd, arg, compat?"COMPAT":""));

    if (module == NEXUS_PLATFORM_P_NUM_DRIVERS-1 && cmd == IOCTL_PROXY_NEXUS_AuthenticatedJoin) {
        /* one exception: client == NULL when a client first Joins because it has not been authenticated yet. */
    }
    else {
        if (!client) {
            /* have special WRN's for more likely cases */
            switch (cmd) {
            case IOCTL_PLATFORM_NEXUS_INIT:
                BDBG_WRN(("client process cannot call NEXUS_Platform_GetDefaultSettings or NEXUS_Platform_Init"));
                rc = NEXUS_NOT_AVAILABLE; /* don't use BERR_TRACE. this is not an error in the driver. */
                break;
            default:
                BDBG_ERR(("ioctl without client %u %#x %#lx", module, cmd, arg));
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                break;
            }
            goto done;
        }
        BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
        BDBG_ASSERT(client->refcnt);
        if(!client->pid) {
            if(module==(NEXUS_PLATFORM_P_NUM_DRIVERS-1) && (cmd==IOCTL_PROXY_NEXUS_Log_Dequeue || cmd==IOCTL_PROXY_NEXUS_Log_Deactivate || cmd==IOCTL_PROXY_NEXUS_Log_Destroy)) {
                /* one more exception allow Log_Dequeue ioctl's in */
            } else {
                BDBG_WRN(("client disabled by server"));
                rc = NEXUS_NOT_AVAILABLE; /* don't use BERR_TRACE. this is not an error in the driver. */
                goto done;
            }
        }
    }

    /* special server ioctls.
    unlocked = true if platform module must be called with no lock (e.g. because the lock will be created or destroyed in the call) */
    if (module == 0 && client == nexus_driver_state.server) {
#if NEXUS_COMPAT_32ABI
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_GetDefaultMemoryConfigurationSettings_tagged == IOCTL_PLATFORM_NEXUS_GetDefaultMemoryConfigurationSettings_tagged_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_GetPlatformCapabilities_tagged == IOCTL_PLATFORM_NEXUS_GetPlatformCapabilities_tagged_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_GetPlatformConfigCapabilities_tagged == IOCTL_PLATFORM_NEXUS_GetPlatformConfigCapabilities_tagged_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_GetDefaultSettings_tagged == IOCTL_PLATFORM_NEXUS_Platform_GetDefaultSettings_tagged_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_GetMemory_tagged == IOCTL_PLATFORM_NEXUS_Platform_GetMemory_tagged);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_InitStandby == IOCTL_PLATFORM_NEXUS_Platform_InitStandby_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_Init_tagged == IOCTL_PLATFORM_NEXUS_Platform_Init_tagged_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_SetStandbySettings_driver == IOCTL_PLATFORM_NEXUS_Platform_SetStandbySettings_driver_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_Uninit == IOCTL_PLATFORM_NEXUS_Platform_Uninit_compat);
        BDBG_CASSERT(IOCTL_PLATFORM_NEXUS_Platform_UninitStandby == IOCTL_PLATFORM_NEXUS_Platform_UninitStandby_compat);
#endif
        switch (cmd) {
        case IOCTL_PLATFORM_NEXUS_Platform_Uninit:
            unlocked = true;
            break;
        case IOCTL_PLATFORM_NEXUS_Platform_Init_tagged:
#if NEXUS_COMPAT_32ABI
            client->abi = compat ? 32 : NEXUS_P_NATIVE_ABI;
#endif
            nexus_driver_server_preinit();
            unlocked = true;
            break;
        case IOCTL_PLATFORM_NEXUS_GetDefaultMemoryConfigurationSettings_tagged:
        case IOCTL_PLATFORM_NEXUS_GetPlatformCapabilities_tagged:
        case IOCTL_PLATFORM_NEXUS_GetPlatformConfigCapabilities_tagged:
        case IOCTL_PLATFORM_NEXUS_INIT:
        case IOCTL_PLATFORM_NEXUS_Platform_GetDefaultSettings_tagged:
        case IOCTL_PLATFORM_NEXUS_Platform_GetMemory_tagged:
        case IOCTL_PLATFORM_NEXUS_Platform_InitStandby:
        case IOCTL_PLATFORM_NEXUS_Platform_UninitStandby:
            unlocked = true;
            break;
        case IOCTL_PLATFORM_NEXUS_Platform_SetStandbySettings_driver:
            lock_standby = true;
            break;
        }
    }

    if (module != NEXUS_PLATFORM_P_NUM_DRIVERS-1) {
        LOCK();
        if (nexus_driver_state.cleanup_pending) {
            /* cleanup of the desired client may be pending. we can clean up now. if nothing is pending, this is harmless. */
            nexus_driver_process_cleanup_queue_lock();
        }
        UNLOCK();
    }

    /* Standby is handled as a special case because it needs to be synchronized with driver_close */
    if (lock_standby) LOCK();
    rc = g_nexus_driver_ioctl_handlers[module].ioctl(context, cmd, arg, NEXUS_IOCTL_TYPE(cmd), unlocked, compat);
    if (lock_standby) UNLOCK();
    if (rc) {return BERR_TRACE(rc);}

done:
    BDBG_MSG_TRACE(("<<nexus_driver_ioctl %u %#x %#x -> %d", module, cmd, arg, rc));

    return rc;
}

int nexus_generic_driver_init(const struct nexus_generic_driver_init_settings *settings)
{
    int rc;
    unsigned i;

    rc = BKNI_Init();
    if (rc) return rc;
    rc = BDBG_Init();
    if (rc) return rc;

    rc = NEXUS_Base_Core_Init();
    if (rc) return rc;

    rc = BKNI_CreateMutex(&nexus_driver_state.lock);
    if (rc) return rc;

    b_memory_region_set_init(&nexus_driver_state.dynamic_memory);

    nexus_driver_state.settings = *settings;

    /* set default nexus_driver_state state */
    /* unless server calls NEXUS_Platform_StartServer setting this to false, we default to allowing. */
    NEXUS_Platform_GetDefaultStartServerSettings(&nexus_driver_state.serverSettings);
    for(i=0;i<NEXUS_DRIVER_MAX_LOG_READERS;i++) {
        nexus_driver_state.debugLog.readers[i].fifo = NULL;
    }
    rc = nexus_driver_proc_init();
    if (rc) return rc;

    return 0;
}

void nexus_generic_driver_uninit(void)
{
    if (nexus_driver_state.uninit_pending) {
        /* if we didn't have a clean close, we need to clean on rmmod */
        LOCK();
        nexus_driver_server_uninit_lock();
        UNLOCK();
    }

    nexus_driver_proc_done();
    b_memory_region_set_shutdown(&nexus_driver_state.dynamic_memory);

    BKNI_DestroyMutex(nexus_driver_state.lock);
    NEXUS_Base_Core_Uninit();

    BDBG_WRN(("uninitialized"));
    BDBG_Uninit();
    BKNI_Uninit();
}


static int
nexus_driver_proxy_ioctl(void *context, unsigned int cmd, unsigned long arg, unsigned long type, bool unlocked, bool compat)
{
    int rc;
    struct nexus_driver_module_driver_state *state = context;
    unsigned i;

    BSTD_UNUSED(unlocked);
    BSTD_UNUSED(compat);

    switch(cmd) {
    case IOCTL_PROXY_NEXUS_AuthenticatedJoin:
        {{
            PROXY_NEXUS_AuthenticatedJoin data;

            /* NEXUS_Platform_Init must already be called */
            if(!nexus_driver_state.active) {
                rc=(int)BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto err_ioctl;
            }

            if (arg) {
                rc = copy_from_user_small(&data, (void *)arg, sizeof(data));
                if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            }

            LOCK();
            /* authenticate and register pid with client */
            if (!nexus_driver_connect_client_lock(state->pid, arg ? &data.certificate : NULL)) {
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                BDBG_ASSERT(!state->client);
            }
            else {
                /* now we can open the client and attach to the driver_state */
                state->client = nexus_driver_open_client_lock(state->pid);
            }
            UNLOCK();
            if (!state->client) {
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                goto err_ioctl;
            }

            /* create callback scheduler */
            state->slave_scheduler = nexus_driver_slave_scheduler_create();
            if (!state->slave_scheduler) {
                rc =(int)BERR_TRACE(NEXUS_UNKNOWN);
                if (state->client) {
                    LOCK();
                    nexus_driver_close_client_lock(state->client);
                    UNLOCK();
                    state->client = NULL;
                }
                goto err_ioctl;
            }
            state->client->joined = true;
            state->client->numJoins++;
#if NEXUS_COMPAT_32ABI
            state->client->abi = compat ? 32 : NEXUS_P_NATIVE_ABI;
#endif
            b_strncpy(state->client->process_name, state->process_name, NEXUS_MAX_PROCESS_NAME);
            BDBG_MSG(("client join: %p(%s) pid=%d", state->client, state->client->process_name, state->client->pid));
            break;
        }}
    case IOCTL_PROXY_NEXUS_Platform_Uninit:
        {{
            /* clean uninit of client */
            BDBG_OBJECT_ASSERT(state->client, nexus_driver_client_state);
            state->client->joined = false;
            break;
        }}
    case IOCTL_PROXY_NEXUS_RunScheduler:
        {{
            PROXY_NEXUS_RunScheduler data;
            rc = copy_from_user_small(&data.in, (void *)arg, sizeof(data.in));
            /* coverity[dead_error_condition] - it's important to do this check in case it's not a dead condition in the future. */
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            /* coverity[stack_use_overflow] - coverity jumps from this kernel mode ioctl into user mode nexus_playback stack, which is not possible */
            rc = nexus_driver_run_scheduler(data.in.priority, data.in.timeout, &data.out.has_callbacks, state->slave_scheduler);
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            rc = copy_to_user_small((&((PROXY_NEXUS_RunScheduler *)arg)->out), &data.out, sizeof(data.out));
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            break;
        }}
    case IOCTL_PROXY_NEXUS_WakeupScheduler:
        {{
            nexus_driver_wakeup_scheduler(state->slave_scheduler);
            break;
        }}
    case IOCTL_PROXY_NEXUS_Scheduler:
        {{
            PROXY_NEXUS_Scheduler data;
            rc = copy_from_user_small(&data.in, (void *)arg, sizeof(data.in));
            /* coverity[dead_error_condition] - it's important to do this check in case it's not a dead condition in the future. */
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            data.out.count = nexus_driver_scheduler_dequeue(data.in.priority, data.out.callbacks, PROXY_NEXUS_CALLBACK_PACKET, state->slave_scheduler, compat);
            rc = copy_to_user_small((&((PROXY_NEXUS_Scheduler *)arg)->out), &data.out, sizeof(data.out));
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            break;
        }}
    case IOCTL_PROXY_NEXUS_SchedulerLock:
        {{
            PROXY_NEXUS_SchedulerLock data;
            rc = copy_from_user_small(&data, (void *)arg, sizeof(data));
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            if (state->slave_scheduler) {
                /* clients are not allowed to lock the scheduler */
                return BERR_TRACE(-1);
            }
            nexus_driver_scheduler_lock(state->client, data.priority, data.lock);
            break;
        }}
#ifdef NEXUS_CONFIG_IMAGE
    case IOCTL_PROXY_NEXUS_Image:
        {{
            nexus_img_ioctl(cmd, arg);
            break;
        }}
#endif
    case IOCTL_PROXY_NEXUS_Log_Activate:
    {{
        BDBG_Fifo_CreateSettings logSettings;
        PROXY_NEXUS_Log_Activate data;
        BERR_Code mrc;
        struct nexus_driver_state *state = &nexus_driver_state;

        if (!state->debugLog.logWriter) {
            unsigned debug_log_size_limit = 128*1024;
            rc = copy_from_user_small(&data, (void *)arg, sizeof(data));
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            if(data.debug_log_size > debug_log_size_limit) {
               BDBG_WRN(("limiting size of debug log from %u to %u lines", data.debug_log_size, debug_log_size_limit));
                data.debug_log_size = debug_log_size_limit;
            }
            BDBG_Fifo_GetDefaultCreateSettings(&logSettings);
            BDBG_Log_GetElementSize(&logSettings.elementSize);
            logSettings.bufferSize = logSettings.elementSize * data.debug_log_size;
            logSettings.buffer = state->debugLog.buffer;
            if(logSettings.bufferSize>sizeof(state->debugLog.buffer)) {
                state->debugLog.dynamicBuffer = BKNI_Malloc(logSettings.bufferSize);
                if(state->debugLog.dynamicBuffer) {
                    logSettings.buffer = state->debugLog.dynamicBuffer;
                } else {
                    BDBG_WRN(("can't allocate buffer of requested size (%u) bytes, use default  %u bytes", (unsigned)logSettings.bufferSize, (unsigned)sizeof(state->debugLog.buffer)));
                    logSettings.bufferSize = sizeof(state->debugLog.buffer);
                }
            }

            mrc = BDBG_Fifo_Create(&state->debugLog.logWriter, &logSettings);
            if(mrc!=BERR_SUCCESS) {rc=BERR_TRACE(mrc);goto err_ioctl;}
        }
        BDBG_Log_SetFifo(state->debugLog.logWriter);
        break;
    }}
    case IOCTL_PROXY_NEXUS_Log_Deactivate:
    {{
        BDBG_Log_SetFifo(NULL);
        break;
    }}
    case IOCTL_PROXY_NEXUS_Log_Test:
    {{
        int result;
        result = nexus_driver_state.debugLog.logWriter!=NULL;
        rc = copy_to_user_small((void *)arg, &result, sizeof(result));
        if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
        break;
    }}
    case IOCTL_PROXY_NEXUS_Log_Create:
    {{
        if(nexus_driver_state.debugLog.logWriter==NULL) {rc=BERR_TRACE(NEXUS_NOT_AVAILABLE);goto err_ioctl;}
        for(i=0;i<NEXUS_DRIVER_MAX_LOG_READERS;i++) {
            PROXY_NEXUS_Log_Instance  data;
            if(nexus_driver_state.debugLog.readers[i].fifo == NULL) {
                BERR_Code mrc;
                mrc = BDBG_FifoReader_Create(&nexus_driver_state.debugLog.readers[i].fifo, nexus_driver_state.debugLog.logWriter);
                if(mrc!=BERR_SUCCESS) {rc=BERR_TRACE(mrc);goto err_ioctl;}
                data.instance  = i;
                rc = copy_to_user_small((void *)arg, &data, sizeof(data));
                if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
                goto done;
            }
        }
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_ioctl;
    }}
    case IOCTL_PROXY_NEXUS_Log_Dequeue:
    {{
        PROXY_NEXUS_Log_Dequeue data;
        struct nexus_driver_state *state = &nexus_driver_state;
        BERR_Code mrc;
        char dbgStr[257];
        size_t dbgStrLen;
        size_t buffer_size;

        rc = copy_from_user_small(&data, (void *)arg, sizeof(data));
        if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
        if(data.instance.instance>=NEXUS_DRIVER_MAX_LOG_READERS) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_ioctl; }
        if(state->debugLog.readers[data.instance.instance].fifo == NULL) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_ioctl; }
        dbgStrLen = sizeof(dbgStr);
        if(dbgStrLen > data.buffer_size) {
            dbgStrLen = data.buffer_size;
        }

        mrc = BDBG_Log_Dequeue(state->debugLog.readers[data.instance.instance].fifo, &data.timeout, dbgStr, dbgStrLen, &buffer_size);
        data.buffer_size = buffer_size;
        if(mrc!=BERR_SUCCESS) {rc=BERR_TRACE(mrc);break;}
        rc=copy_to_user_small((void *)(unsigned long)data.buffer, dbgStr, data.buffer_size);
        if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
        rc=copy_to_user_small((void *)arg, &data, sizeof(data));
        if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
        break;
    }}
    case IOCTL_PROXY_NEXUS_Log_Destroy:
    {{
        PROXY_NEXUS_Log_Instance data;
        struct nexus_driver_state *state = &nexus_driver_state;

        rc = copy_from_user_small(&data, (void *)arg, sizeof(data));
        if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
        if(data.instance>=NEXUS_DRIVER_MAX_LOG_READERS) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_ioctl; }
        if(state->debugLog.readers[data.instance].fifo == NULL) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_ioctl; }
        BDBG_FifoReader_Destroy(state->debugLog.readers[data.instance].fifo);
        state->debugLog.readers[data.instance].fifo = NULL;
        break;
    }}
    case IOCTL_PROXY_NEXUS_StopCallbacks:
    {{
        struct nexus_driver_client_state *client = nexus_driver_client_id(context);
        NEXUS_BaseObject *object;
        b_objdb_set_client(&client->client);
#if NEXUS_COMPAT_32ABI
        if(compat) {
            rc = NEXUS_BaseObject_FromId(arg, &object);
            if(rc!=NEXUS_SUCCESS) {
                BDBG_ERR(("IOCTL_PROXY_NEXUS_StopCallbacks: unknown object id:%#x", (unsigned)arg));
            }
        } else
#endif
        {
            object = (void *)arg;
        }
        NEXUS_Platform_P_StopCallbacks(object);
        b_objdb_set_client(NULL);
        break;
    }}
    case IOCTL_PROXY_NEXUS_StartCallbacks:
    {{
        struct nexus_driver_client_state *client = nexus_driver_client_id(context);
        NEXUS_BaseObject *object;
        b_objdb_set_client(&client->client);
#if NEXUS_COMPAT_32ABI
        if(compat) {
            rc = NEXUS_BaseObject_FromId(arg, &object);
            /* don't test result, see NEXUS_Platform_P_StartCallbacks */
        } else
#endif
        {
            object = (void *)arg;
        }
        NEXUS_Platform_P_StartCallbacks(object);
        b_objdb_set_client(NULL);
        break;
     }}
#if NEXUS_CPU_ARM
        case IOCTL_PROXY_NEXUS_CacheFlush:
        {{
            PROXY_NEXUS_CacheFlush data;
            rc = copy_from_user_small(&data, (void*)arg,sizeof(data));
            if(rc!=0) {rc=BERR_TRACE(rc);goto err_fault;}
            /* coverity[ tainted_data : FALSE ] */
            rc = brcm_cpu_dcache_flush((void *)(unsigned long)data.address, data.length);
        }}
        break;
#endif
    default:
        rc = NEXUS_UNKNOWN;
        goto err_ioctl;
    }
done:
    return 0;

err_ioctl:
err_fault:
    return rc;
}

static void
nexus_driver_module_print_cb_null(void *context)
{
    void (*callback)(void) = (void(*)(void))context;
    callback();
}

NEXUS_Error
nexus_platform_p_add_proc(NEXUS_ModuleHandle module, const char *filename, const char *module_name, void (*dbgPrint)(void))
{
    nexus_driver_proc_register_status(filename, module, module_name, nexus_driver_module_print_cb_null, (void *)dbgPrint);
    return NEXUS_SUCCESS;
}

void
nexus_platform_p_remove_proc(NEXUS_ModuleHandle module, const char *filename)
{
    BSTD_UNUSED(module);
    nexus_driver_proc_unregister_status(filename);
}

/**
client management

a client is added to one of three lists. it cannot be in two lists at the same time.

the three lists are:
    nexus_driver_state.allowed_clients - client is explicitly registered by server, but not opened
        client->refcnt == 0 && client->pid == 0
        only in this list if not open and cleanup has been performed.
    nexus_driver_state.clients - open clients
        client->refcnt > 0 && client->pid != 0
        this client is in use.
    nexus_driver_state.cleanup_clients - closed clients that need object cleanup
        client->pid == 0
        if client->refcnt > 0, then we must wait for drivers to close
        if client->refcnt == 0, this will be finalized, either by deleting or returning to allowed_clients list

some notes:
it is assumed that NEXUS_Platform_Init/Uninit will be called non-reentrantly.
NEXUS_Platform_Init or NEXUS_Platform_Uninit calls may fail. use nexus_driver_state.active to know if nexus is up or not.
server cannot call NEXUS_Platform_Uninit during close with signal_pending(current).
clients are all closed and prevented from opening while server uninit is in progress.
however, once server uninit is complete, clients may started opening before server init is done.
**/

BDBG_OBJECT_ID(nexus_driver_client_state);

static struct nexus_driver_client_state *
nexus_driver_open_client_lock(unsigned pid)
{
    struct nexus_driver_client_state *client;

    BDBG_MSG_TRACE(("nexus_driver_open_client(%d)", pid));

    if (nexus_driver_state.uninit_in_progress) {
        client = NULL;
        goto done;
    }

    /* verify the pid is not currently in the cleanup list */
    for (client = BLST_S_FIRST(&nexus_driver_state.cleanup_clients); client; client = BLST_S_NEXT(client, link)) {
        if (client->pid == pid) {
            BDBG_ERR(("client %d cannot be started again until cleaned up", pid));
            client = NULL;
            goto done;
        }
    }

    /* check if already open */
    for (client = BLST_S_FIRST(&nexus_driver_state.clients); client; client = BLST_S_NEXT(client, link)) {
        if (client->pid == pid) {
            BDBG_MSG_TRACE(("  found %#x in open clients", client));
            break;
        }
    }

    /* if not open, check if allowed */
    if (!client) {
        for (client = BLST_S_FIRST(&nexus_driver_state.allowed_clients); client; client = BLST_S_NEXT(client, link)) {
            if (client->pid == pid) {
                /* move from allowed to open */
                BDBG_ASSERT(client->list == nexus_driver_allowed_clients_list);
                BLST_S_REMOVE(&nexus_driver_state.allowed_clients, client, nexus_driver_client_state, link);
                BLST_S_INSERT_HEAD(&nexus_driver_state.clients, client, link);
                client->list = nexus_driver_clients_list;
                BDBG_MSG_TRACE(("  found %#x in allowed clients", client));
                break;
            }
        }
    }

    if (!client) {
        BDBG_MSG(("cannot find client for %d", pid));
    }
    else {
        BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
        client->refcnt++;
        BDBG_ASSERT(client->pid && client->pid == pid);
        /* if client->pid is set, then client must be in nexus_driver_state.clients */
    }

done:
    return client;
}

/*
this is called by the client when its refcnt is decremented to zero or when the server wants to force a client to be disabled.
after this, the client is unusable and any driver connected to it will fail.
add to cleanup_clients for finalization.
*/
static void
nexus_driver_disable_client_lock(struct nexus_driver_client_state *client)
{
    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    client->joined = false;
    if (client->pid) {
        if (client->pid != nexus_driver_state.server->pid && client->refcnt) {
            NEXUS_Platform_P_TerminateProcess(client->pid);
        }

        BDBG_ASSERT(client->list == nexus_driver_clients_list);
        BLST_S_REMOVE(&nexus_driver_state.clients, client, nexus_driver_client_state, link);
        client->pid = 0;
        BDBG_MSG(("client disconnect: %p(%s)", client, client->process_name));
        if (client == nexus_driver_state.server) {
            /* If open_count is non-zero and active is false, then we are in the middle of a clean NEXUS_Platform_Uninit and
            should not add the server to the cleanup_clients list.
            Otherwise, this was deferred with uninit_pending; we must clean the server as a client. */
            if (nexus_driver_state.open_count && !nexus_driver_state.active) {
                BLST_S_INSERT_HEAD(&nexus_driver_state.allowed_clients, client, link);
                client->list = nexus_driver_allowed_clients_list;
            }
            else {
                /* insert server at end of cleanup_clients list. equivalent to BLST_S_INSERT_TAIL. */
                struct nexus_driver_client_state *c;
                for (c = BLST_S_FIRST(&nexus_driver_state.cleanup_clients); c; ) {
                    struct nexus_driver_client_state *next = BLST_S_NEXT(c, link);
                    if (!next) {
                        BLST_S_INSERT_AFTER(&nexus_driver_state.cleanup_clients, c, client, link);
                        break;
                    }
                    c = next;
                }
                if (!c) {
                    BLST_S_INSERT_HEAD(&nexus_driver_state.cleanup_clients, client, link);
                }
                client->list = nexus_driver_cleanup_clients_list;
            }
        }
        else {
            BLST_S_INSERT_HEAD(&nexus_driver_state.cleanup_clients, client, link);
            client->list = nexus_driver_cleanup_clients_list;
        }
        nexus_driver_state.cleanup_pending = true;
    }

    nexus_driver_unlock_schedulers(client);
}

/* must be paired one-for-one with nexus_driver_open_client */
static void
nexus_driver_close_client_lock(struct nexus_driver_client_state *client)
{
    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    BDBG_MSG_TRACE(("nexus_driver_close_client: %p %d", client, client->refcnt));
    BDBG_ASSERT(client->refcnt);
    if (--client->refcnt == 0 && client != nexus_driver_state.server) {
        nexus_driver_disable_client_lock(client);
    }
}

struct nexus_driver_client_state *
nexus_driver_client_id(struct nexus_driver_module_driver_state *state)
{
    BDBG_OBJECT_ASSERT(state->client, nexus_driver_client_state);
    return state->client;
}

struct nexus_driver_client_state *
nexus_driver_create_client(const NEXUS_Certificate *pCertificate, const NEXUS_ClientConfiguration *pConfig)
{
    struct nexus_driver_client_state *client;
    LOCK();
    client = nexus_driver_create_client_lock(pCertificate, pConfig);
    UNLOCK();
    return client;
}

static const char *g_clientModeStr[NEXUS_ClientMode_eMax] = {"unprotected","verified","protected","untrusted"};

NEXUS_Error nexus_p_set_client_mode(struct nexus_driver_client_state *client, NEXUS_ClientMode mode)
{
    switch (mode) {
    case NEXUS_ClientMode_eUnprotected: /* deprecated */
        client->client.mode = NEXUS_ClientMode_eVerified;
        break;
    case NEXUS_ClientMode_eVerified:
    case NEXUS_ClientMode_eProtected:
    case NEXUS_ClientMode_eUntrusted:
        client->client.mode = mode;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return NEXUS_SUCCESS;
}

/**
nexus_driver_create_client is called in three places:
1) from NEXUS_Platform_RegisterClient for authenticated clients
2) from nexus_driver_connect_client from unauthenticated clients (pCertificate == NULL)
3) from nexus_driver_server_init for the server's "client" (pCertificate == NULL)
**/
static struct nexus_driver_client_state *
nexus_driver_create_client_lock(const NEXUS_Certificate *pCertificate, const NEXUS_ClientConfiguration *pConfig)
{
    struct nexus_driver_client_state *client;
    int rc;

    client = BKNI_Malloc(sizeof(*client));
    if (!client) return NULL;
    BKNI_Memset(client, 0, sizeof(*client));
    BDBG_OBJECT_SET(client, nexus_driver_client_state);

    client->client.resources.allowed = pConfig->resources;

    client->client.config = *pConfig;
    if (pCertificate) {
        /* this path is only for authenticated clients */
        client->certificate = *pCertificate;
    }
    rc = nexus_p_set_client_mode(client, pConfig->mode);
    if (rc) { BERR_TRACE(rc); goto err_invalid_mode; }

    if (nexus_driver_state.server) {
        rc = b_get_client_default_heaps(&client->client.config, &client->client.default_heaps);
        if (rc) {
            BERR_TRACE(rc);
            goto err_no_default_heap;
        }
    }

    /* if dynamic is true, it can be deleted after it is closed */
    client->dynamic = (pCertificate == NULL);

    BLST_S_INSERT_HEAD(&nexus_driver_state.allowed_clients, client, link);
    client->list = nexus_driver_allowed_clients_list;

    BDBG_MSG(("create client: %p, cert length %d, %s", client, client->certificate.length, g_clientModeStr[pConfig->mode]));
    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    return client;

err_no_default_heap:
err_invalid_mode:
    BDBG_OBJECT_DESTROY(client, nexus_driver_client_state);
    BKNI_Free(client);
    return NULL;
}

static void
nexus_driver_destroy_client_lock(struct nexus_driver_client_state *client)
{
    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    BDBG_ASSERT(!client->refcnt);
    if (client->list == nexus_driver_cleanup_clients_list) {
        return;
    }
    BDBG_MSG(("destroy client: %p", client));
    BDBG_ASSERT(client->list == nexus_driver_allowed_clients_list);
    BLST_S_REMOVE(&nexus_driver_state.allowed_clients, client, nexus_driver_client_state, link);
    BDBG_OBJECT_DESTROY(client, nexus_driver_client_state);
    BKNI_Free(client);
}

void
nexus_driver_destroy_client(struct nexus_driver_client_state *client)
{
    LOCK();
    BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    if (client->refcnt) {
        /* cannot destroy now because client is still open, but server has unregistered, so we can destroy on final close */
        nexus_driver_disable_client_lock(client);
        client->dynamic = true;
    }
    else {
        nexus_driver_destroy_client_lock(client);
    }
    UNLOCK();
}

/*
process the cleanup_clients list.
this means searching the object database for handles.
if the refcnt hasn't fallen to zero, we have to leave it in the list.
*/
static void
nexus_driver_process_cleanup_queue_lock(void)
{
    struct nexus_driver_client_state *client;
    unsigned i;
    NEXUS_StandbySettings nexusStandbySettings;

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    if(nexusStandbySettings.mode != NEXUS_StandbyMode_eOn) {
        BDBG_MSG(("Standby mode %u. Defer cleanup!", nexusStandbySettings.mode));
        return;
    }


    /* first, clean up object database. to preserve calling order, we must process per module, then per client */
    for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
        struct nexus_driver_module_header *header;
        header = g_nexus_driver_state_handlers[g_nexus_driver_state_order[i].index].header;
        /* no objects to clean up in platform module. this would deadlock if run with an active client, so we avoid it.
        another option is ensure platform lock is acquired, then pass NEXUS_MODULE_SELF to b_objdb so it can avoid a re-acquire */
        if (!header || !header->module || header->module == NEXUS_MODULE_SELF) {
            continue;
        }

        for (client = BLST_S_FIRST(&nexus_driver_state.cleanup_clients); client; client = BLST_S_NEXT(client, link)) {
            BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
            /* all clients in this list must be closed */
            BDBG_ASSERT(client->pid == 0);
            BDBG_MSG(("process cleanup queue: client %p, module %s", (void *)client, header->name));
            NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
            NEXUS_CoreModule_Uninit_Client_priv(&client->client);
            NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
            b_objdb_module_uninit_client_objects(&header->objdb, &client->client);
        }
    }

    /* make a second pass to clean up clients */
    for (client = BLST_S_FIRST(&nexus_driver_state.cleanup_clients); client; ) {
        struct nexus_driver_client_state *next;
        next = BLST_S_NEXT(client, link);

        /* need to go per client, per header */
        for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
            struct nexus_driver_module_header *header;
            header = g_nexus_driver_state_handlers[g_nexus_driver_state_order[i].index].header;
            if (!header || !header->module || header->module == NEXUS_MODULE_SELF) {
                continue;
            }

            /* deactivate all callbacks for this client, even if object not in database */
            b_objdb_module_uninit_client_callbacks(&header->objdb, &client->client);
        }

        if (!client->refcnt) {
            /* finalize */
            BDBG_ASSERT(client->list == nexus_driver_cleanup_clients_list);
            BLST_S_REMOVE(&nexus_driver_state.cleanup_clients, client, nexus_driver_client_state, link);
            BLST_S_INSERT_HEAD(&nexus_driver_state.allowed_clients, client, link);
            client->list = nexus_driver_allowed_clients_list;
            if (client->dynamic) {
                nexus_driver_destroy_client_lock(client);
            }

            if (!nexus_driver_state.uninit_in_progress) {
                NEXUS_Platform_P_SweepModules();
            }
        }
        client = next;
    }
    nexus_driver_state.cleanup_pending = (BLST_S_FIRST(&nexus_driver_state.cleanup_clients) != NULL);
}

/* find client which is allowed, not open, and has no pid assigned */
static struct nexus_driver_client_state *
nexus_driver_connect_client_lock(unsigned pid, const NEXUS_Certificate *pCertificate)
{
    struct nexus_driver_client_state *client = NULL;

    BDBG_MSG(("nexus_driver_connect_client: pid=%d cert length %d", pid, pCertificate?pCertificate->length:0));
    if (pCertificate && pCertificate->length) {
        /* if you provide a certificate, it must match */
        for (client = BLST_S_FIRST(&nexus_driver_state.allowed_clients); client; client = BLST_S_NEXT(client, link)) {
            BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
            if (client->pid) {
                /* can only connect one process to a client */
                continue;
            }
            if (client->certificate.length == pCertificate->length &&
                pCertificate->length <= sizeof(pCertificate->data) &&
                !BKNI_Memcmp(&client->certificate.data, pCertificate->data, pCertificate->length))
            {
                client->pid = pid;
                break;
            }
        }
        if (!client) {
            BDBG_ERR(("unable to find certificate match"));
        }
    }
    else {
        /* cannot authenticate. does the server allow unauthenticated clients? */
        if (nexus_driver_state.serverSettings.allowUnauthenticatedClients) {
            client = nexus_driver_create_client_lock(NULL, &nexus_driver_state.serverSettings.unauthenticatedConfiguration);
            if (client) {
                client->pid = pid;
            }
            else {
                BDBG_ERR(("unable to create unauthenticated client"));
            }
        }
        else {
            BDBG_ERR(("connection of unauthenticated client will fail because server has not allowed it"));
        }
    }

    if (client) {
        BDBG_OBJECT_ASSERT(client, nexus_driver_client_state);
    }
    else {
        BDBG_ERR(("nexus_driver_connect_client: rejecting client: pid=%d, cert length %d", pid, pCertificate?pCertificate->length:0));
    }

    return client;
}

/*
nexus_driver_server_init is called when the first open is performed.
it cannot be done on insmod because the server's process (pid) is required
nexus_driver_server_init allows clients to be created on subsequent opens
nexus_driver_server_init is called before NEXUS_Platform_Init. the module handles are not open yet. therefore a nexus_driver_server_postinit is required.
*/
static int nexus_driver_server_init_lock(unsigned pid)
{
    if (!nexus_driver_state.server) {
        NEXUS_ClientConfiguration config;
        struct nexus_driver_client_state *client;
#ifdef NEXUS_CONFIG_IMAGE
        int rc;
        rc = nexus_img_interfaces_init();
        if (rc) return rc;
#endif

        NEXUS_Platform_GetDefaultClientConfiguration(&config);
        config.mode = NEXUS_ClientMode_eVerified;
        client = nexus_driver_create_client_lock(NULL, &config);
        if (!client) return BERR_TRACE(-1);

        client->pid = pid;
        nexus_driver_state.server = client;
        b_objdb_set_default_client(&client->client);

        NEXUS_Base_SetModuleDebugLevel();
    }
    return 0;
}

static int nexus_driver_server_preinit(void)
{
    int rc;
#ifdef NEXUS_CONFIG_IMAGE
    /* need proxy's IMG thread to connect before calling NEXUS_Platform_Init.
    this must be called while LOCK() is not held. */
    rc = nexus_img_wait_until_ready();
    if (rc) return BERR_TRACE(rc);
#endif
    return rc;
}

/*
called after NEXUS_Platform_Init completes
this connects with all proxy thunks
*/
static int nexus_driver_server_postinit_lock(void)
{
    int rc = 0;
    unsigned i;
    struct nexus_driver_client_state *client = nexus_driver_state.server;

    /* connect with the thunk headers now the module handles are available */
    for (i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
        rc = g_nexus_driver_state_handlers[i].open(i);
        if (rc) { rc = BERR_TRACE(rc); break; }

        /* for sorting */
        g_nexus_driver_state_order[i].index = i;
        g_nexus_driver_state_order[i].order = g_nexus_driver_state_handlers[i].header->module ? NEXUS_Module_GetOrder(g_nexus_driver_state_handlers[i].header->module) : 0;
    }

    /* sort g_nexus_driver_state_order[] by module initialization order, greatest to least, which makes objdb cleanup possible */
    for (i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
        unsigned j;
        for (j=i;j<NEXUS_PLATFORM_P_NUM_DRIVERS-1;j++) {
            if (g_nexus_driver_state_order[j].order > g_nexus_driver_state_order[i].order) {
                struct module_order temp = g_nexus_driver_state_order[j];
                g_nexus_driver_state_order[j] = g_nexus_driver_state_order[i];
                g_nexus_driver_state_order[i] = temp;
            }
        }
    }

    if (g_NEXUS_platformSettings.mode == NEXUS_ClientMode_eVerified) {
        client->client.mode = NEXUS_ClientMode_eVerified;
        client->client.config.mode = g_NEXUS_platformSettings.mode;
    }

    /* give the server's client all heaps */
    BDBG_CASSERT(sizeof(client->client.config.heap)/sizeof(client->client.config.heap[0]) == sizeof(g_pCoreHandles->heap)/sizeof(g_pCoreHandles->heap[0]));
    for(i=0;i<sizeof(client->client.config.heap)/sizeof(client->client.config.heap[0]);i++) {
        client->client.config.heap[i] = g_pCoreHandles->heap[i].nexus;
    }

    nexus_driver_proc_register_status("mma", g_NEXUS_platformHandles.core, "BMEM_ALLOCATED", NEXUS_Core_DumpHeaps_priv, NULL);
    nexus_driver_proc_register_status("kni", g_NEXUS_platformHandles.core, "unused", nexus_driver_print_kni, NULL);

    return rc;
}

/*
closes all clients before nexus modules are closed
*/
static void nexus_driver_server_preuninit_lock(void)
{
    struct nexus_driver_client_state *client;

    nexus_driver_proc_unregister_status("mma");
    nexus_driver_proc_unregister_status("kni");

    if (nexus_driver_state.uninit_in_progress) {
        /* this check is needed because the NEXUS_Platform_Uninit ioctl may fail, which would cause
        nexus_driver_status.active to remain true. */
        return;
    }
    nexus_driver_state.uninit_in_progress = true;

    /* close all open clients */
    nexus_driver_disable_clients_lock(true);

    /* now we wait for every client's driver interface to close */
    while (1) {
        unsigned refcnt = 0;
        for (client = BLST_S_FIRST(&nexus_driver_state.cleanup_clients); client; client = BLST_S_NEXT(client, link)) {
            refcnt += client->refcnt;
        }
        if (!refcnt) break;
        UNLOCK();
        BKNI_Sleep(1);
        LOCK();
    }

    /* final client cleanup before nexus is uninitialized */
    nexus_driver_process_cleanup_queue_lock();
    /* because we waited for client->refcnt to be 0, cleanup must be complete. */
    BDBG_ASSERT(!nexus_driver_state.cleanup_pending);
}

/*
closes module headers after modules have been closed
all interfaces must already be closed.
*/
void nexus_driver_server_close_module_headers(void)
{
    unsigned i;
    for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS-1;i++) {
        if (g_nexus_driver_state_handlers[i].header) {
            g_nexus_driver_state_handlers[i].close();
            g_nexus_driver_state_handlers[i].header = NULL;
        }
    }
}


/*
full shutdown
bring driver back to insmod'd state (as if there were no opens).
call NEXUS_Platform_Uninit if needed.
*/
static void nexus_driver_server_uninit_lock(void)
{
    struct nexus_driver_client_state *client;

    BDBG_MSG_TRACE((">nexus_driver_server_uninit %d", nexus_driver_state.active));
    BDBG_ASSERT(!nexus_driver_state.open_count);
    nexus_driver_state.uninit_pending = false;

    if (nexus_driver_state.active) {
        UNLOCK();
        NEXUS_Platform_Uninit();
        LOCK();
        BDBG_ASSERT(!nexus_driver_state.active);
        BDBG_ASSERT(nexus_driver_state.uninit_in_progress);
    }
    else {
        nexus_driver_state.uninit_in_progress = true;
    }
    BDBG_ASSERT(nexus_driver_state.server);

    /* one last cleanup. because all drivers are closed, this must delete all clients.
    the clients and cleanup_clients list must be empty afterwards. */
    BDBG_ASSERT(!BLST_S_FIRST(&nexus_driver_state.clients));
    nexus_driver_process_cleanup_queue_lock();
    BDBG_ASSERT(!BLST_S_FIRST(&nexus_driver_state.clients));
    BDBG_ASSERT(!BLST_S_FIRST(&nexus_driver_state.cleanup_clients));

    /* the only client left are the non-dynamic ones. we can destroy those too. */
    while ((client = BLST_S_FIRST(&nexus_driver_state.allowed_clients))) {
        nexus_driver_destroy_client_lock(client);
    }
    b_objdb_set_default_client(NULL);
#ifdef NEXUS_CONFIG_IMAGE
    nexus_img_interfaces_shutdown();
#endif
    nexus_driver_state.server = NULL;
    nexus_driver_state.uninit_in_progress = false;

    {
        struct nexus_driver_state *state = &nexus_driver_state;
        unsigned i;

        for(i=0;i<NEXUS_DRIVER_MAX_LOG_READERS;i++) {
            if(state->debugLog.readers[i].fifo != NULL) {
                BDBG_FifoReader_Destroy(nexus_driver_state.debugLog.readers[i].fifo);
                state->debugLog.readers[i].fifo = NULL;
            }
        }

        if(state->debugLog.logWriter!=NULL) {
            BDBG_Log_SetFifo(NULL);
            BDBG_Fifo_Destroy(state->debugLog.logWriter);
            state->debugLog.logWriter = NULL;
            if(state->debugLog.dynamicBuffer) {
                BKNI_Free(state->debugLog.dynamicBuffer);
                state->debugLog.dynamicBuffer = NULL;
            }
        }
    }

    BDBG_MSG_TRACE(("<nexus_driver_server_uninit"));
}

void nexus_generic_driver_read_register(uint32_t addr, uint32_t *p_value)
{
    NEXUS_Platform_ReadRegister(addr, p_value);
}

void nexus_generic_driver_write_register(uint32_t addr, uint32_t value)
{
    NEXUS_Platform_WriteRegister(addr, value);
}


void nexus_driver_get_client_configuration(const struct b_objdb_client *client_id, NEXUS_ClientConfiguration *pSettings)
{
    struct nexus_driver_client_state *client;
    LOCK();
    for (client = BLST_S_FIRST(&nexus_driver_state.clients); client; client = BLST_S_NEXT(client, link)) {
        if (client_id == &client->client) {
            *pSettings = client->client.config;
            goto done;
        }
    }
    BDBG_ERR(("invalid client %p", client_id));
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
done:
    UNLOCK();
}

int nexus_driver_server_postinit(void)
{
    int rc;

    LOCK();
    rc = nexus_driver_server_postinit_lock();
    UNLOCK();
    if (rc) return BERR_TRACE(rc);

    return 0;
}

void nexus_driver_server_preuninit(void)
{
    LOCK();
    nexus_driver_server_preuninit_lock();
    UNLOCK();
}

NEXUS_Error NEXUS_Platform_P_AddDynamicRegion(NEXUS_Addr addr, unsigned size)
{
    int rc;
    b_memory_region region;
    region.base = addr;
    region.length = size;

    LOCK();
    rc = b_memory_region_add(&nexus_driver_state.dynamic_memory, &region);
    UNLOCK();
    if(rc!=0) {return BERR_TRACE(BERR_NOT_SUPPORTED);}
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_RemoveDynamicRegion(NEXUS_Addr addr, unsigned size)
{
    int rc;
    b_memory_region region;
    region.base = addr;
    region.length = size;
    LOCK();
    rc = b_memory_region_delete(&nexus_driver_state.dynamic_memory, &region);
    UNLOCK();
    if(rc!=0) {return BERR_TRACE(BERR_NOT_SUPPORTED);}
    return NEXUS_SUCCESS;
}

void NEXUS_P_DriverInVararg_Init(NEXUS_P_DriverInVararg *state, struct b_objdb_client *client, void *data, unsigned size)
{
    state->data = data;
    state->original_data = data;
    state->size = size;
    state->varargs_begin = 0;
    state->varargs_offset = 0;
    state->client = client;
    return;
}

void NEXUS_P_DriverInVararg_Shutdown(NEXUS_P_DriverInVararg *state)
{
    if(state->data != state->original_data) {
        BDBG_MSG(("%p: free %p:%u", (void *)state, state->data, state->size));
        BKNI_Free(state->data);
    }
    return;
}

static void *NEXUS_P_DriverInVararg_Allocate(NEXUS_P_DriverInVararg *state, unsigned vararg_size)
{
    unsigned aligned_vararg_size = B_IPC_DATA_ALIGN(vararg_size);
    unsigned required_size = state->varargs_offset + state->varargs_begin + aligned_vararg_size;
    void *data;
    BDBG_MSG(("%p: alloc %p:%u -> %u", (void *)state, state->data, state->size, required_size));
    if( required_size > state->size) {
        void *buf = nexus_client_driver_alloc(state->client, required_size);
        BDBG_MSG(("%p: realloc %p:%u -> %p:%u", (void *)state, state->data, state->size, buf, required_size));
        if(buf==NULL) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memcpy(buf, state->data, state->size);
        state->size = required_size;
        if(state->data != state->original_data) {
            BDBG_MSG(("%p: reaalloc free %p:%u", (void *)state, state->data, state->size));
            BKNI_Free(state->data);
        }
        state->data = buf;
    }
    data = (uint8_t *)state->data + state->varargs_begin + state->varargs_offset;
    state->varargs_offset += aligned_vararg_size;
    return data;
}

NEXUS_Error NEXUS_P_DriverInVararg_InVarArg(NEXUS_P_DriverInVararg *state, unsigned vararg_size, const void *src, int *field, bool *is_null)
{
    if(src) {
        void *data;
        void *old_data = state->data;
        *is_null = false; /* assign is_null earlies so don't worry about relocation */
        data = NEXUS_P_DriverInVararg_Allocate(state, vararg_size);
        if(data==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* field is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        field = (void *)((uint8_t *)state->data + ((uint8_t *)field - (uint8_t *)old_data));
        *field = (uint8_t *)data - (uint8_t *)state->data;
        if(copy_from_user_small(data, src, vararg_size)!=0) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    } else {
        *is_null = true;
        *field = -1;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_P_DriverInVararg_InVarArg_AddrField(NEXUS_P_DriverInVararg *state, const NEXUS_Addr *src, unsigned count, int varArg, int *varArgField)
{
    if(varArg!=-1) {
        unsigned i;
        void *old_data = state->data;
        NEXUS_Addr *dataAddr = NEXUS_P_DriverInVararg_Allocate(state, count*sizeof(NEXUS_Addr));
        if(dataAddr==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* varArgField is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        varArgField = (void *)((uint8_t *)state->data + ((uint8_t *)varArgField - (uint8_t *)old_data));
        *varArgField= (uint8_t *)dataAddr - (uint8_t *)state->data;
        if(copy_from_user_small(dataAddr, src, count*sizeof(NEXUS_Addr))!=0) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        for(i=0;i<count;i++) {
            BDBG_MSG(("%p NEXUS_P_DriverInVararg_InVarArg_AddrField:%d(%d) %p[%u]=" BDBG_UINT64_FMT "", (void *)state, varArg, *varArgField, (void *)dataAddr, i, BDBG_UINT64_ARG(dataAddr[i])));
        }
    } else {
        *varArgField= -1;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_P_Driver_OutVarArg(const void *out_data, unsigned size, void *dest, int varArg)
{
    if(dest && varArg>=0) {
        if(copy_to_user_small(dest, (uint8_t *)out_data + varArg, size)!=0) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    return NEXUS_SUCCESS;
}
#include "b_memory_regions.inc"

static void nexus_driver_print_kni(void *context)
{
    BSTD_UNUSED(context);
#if BDBG_DEBUG_BUILD
    BKNI_DumpMallocs_WithCapture(nexus_driver_proc_print);
#endif
}
