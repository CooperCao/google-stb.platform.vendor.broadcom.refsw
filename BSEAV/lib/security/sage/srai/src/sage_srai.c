/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/*
 * Includes
 */

/* system headers */
#include <pthread.h>

/* Magnum headers */
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "blst_list.h"

/* Nexus headers */
#include "nexus_base_os.h"
#include "nexus_base_mmap.h"
#include "nexus_memory.h"
#include "nexus_sage.h"
#include "nexus_platform_client.h"
#if NEXUS_SECURITY_API_VERSION == 2
#include "nexus_otp_msp.h"
#include "nexus_otp_msp_indexes.h"
#else
#include "nexus_otpmsp.h"
#endif

/* SAGE software framework headers */
#include "bsagelib_types.h"
#include "bsagelib_sdl_header.h"
#include "priv/suif.h"
#include "priv/suif_sdl.h"

/* SRAI header */
#include "sage_srai.h"


#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "sage_srai_extended.h"
#include "priv/bsagelib_rpc_shared.h"
#include "priv/bsagelib_shared_types.h"

#define SAGE_ALIGN_SIZE (4096)
#define RoundDownP2(VAL, PSIZE) ((VAL) & (~(PSIZE-1)))
#define RoundUpP2(VAL, PSIZE)   RoundDownP2((VAL) + PSIZE-1, PSIZE)

/*
 * Debug
 */

/* Name of the module. Used by BDBG_XXX() macros. */
BDBG_MODULE(srai);

/* DEBUG - only for development */
#if 0
#define SRAI_DUMP_COMMAND(STR, COMMAND)                         \
        BDBG_LOG(("DUMP command %s (%d bytes):\n"               \
                  "\tsystemCommandId=%08x\n"                    \
                  "\tplatformId=%08x\n"                         \
                  "\tmoduleId=%08x\n"                           \
                  "\tmoduleCommandId=%08x\n"                    \
                  "\tpayloadOffset=0x%08x\n",                   \
                  (STR), sizeof(*(COMMAND)),                    \
                  command.systemCommandId,                      \
                  command.platformId,                           \
                  command.moduleId,                             \
                  command.moduleCommandId,                      \
          (uint32_t)command.payloadOffset))
#else
#define SRAI_DUMP_COMMAND(STR, COMMAND)
#endif


/*
 * Local types definitions
 */

/* sync call context provides the means to run synchronous
 * module or platform command call using asynchronous Nexus Sage API. */
typedef struct srai_sync_call {
    BKNI_MutexHandle mutex;           /* protect concurrent access */
    BKNI_EventHandle event;           /* used to wait for event / set event */
    NEXUS_SageChannelHandle channel;  /* Nexus SAGE channel used to communicate with SAGE-side */
    NEXUS_SageCommand command;        /* command definition (IDs) */
    NEXUS_Error lastErrorNexus;       /* returned error at Nexus level */
    BERR_Code lastErrorSage;          /* returned error at SAGE level */
    uint8_t reset;                    /* if reset is set to 1, this sync context cannot be used anymore */
    uint8_t terminated;               /* if terminated is set to 1, this sync context cannot be used anymore */
} srai_sync_call;

/* SRAI platform context.
 * Carried through API calls using opaque SRAI_PlatformHandle
 * This context can be chained in linked list. */
typedef struct SRAI_Platform {
    srai_sync_call sync;              /* sync object used to run a synchronous call to SAGE */
    uint32_t id;                      /* platform identifier */
    struct {
        uint8_t notified;
        SRAI_RequestRecvCallback requestRecvCallback;
        BSAGElib_InOutContainer *container;
        BSAGElib_RpcMessage *message;
    } callbacks;
    BLST_D_ENTRY(SRAI_Platform) link; /* member of a linked list */
    uint8_t ta_terminate_reported;
} SRAI_Platform;

/* SRAI module context.
 * Carried through API cals using opaque SRAI_ModuleHandle
 * This context can be chained in linked list. */
typedef struct SRAI_Module {
    srai_sync_call sync;              /* sync object used to run a synchronous call to SAGE */
    uint32_t id;                      /* module identifier */
    SRAI_PlatformHandle platform;     /* platform that module belongs to */
    BLST_D_ENTRY(SRAI_Module) link;   /* member of a linked list */
    uint8_t used;                     /* module is currently being used (cannot remove) */
    uint32_t indication_count;        /* count of indication events received since last read*/
    SRAI_IndicationCallback indicationCallback;/* callback registered from application layer */
    uint32_t indicationCallbackArg;   /* opaque parameter to return to the callback */
} SRAI_Module;

typedef struct SRAI_CallbackItem {
    SRAI_Callback callback;
    BLST_D_ENTRY(SRAI_CallbackItem) link; /* member of a linked list */
} SRAI_CallbackItem;

typedef struct SRAI_TATerminateCallbackItem {
    SRAI_TATerminateCallback callback;
    BLST_D_ENTRY(SRAI_TATerminateCallbackItem) link; /* member of a linked list */
} SRAI_TATerminateCallbackItem;

/*
 * SRAI private context (used by sage_srai.c routines)
 * There can be only one instance of SRAI in each application
 * BUT multiple functionnal piece of software can take advantage of it.
 * i.e. open and use platforms/modules without having
 * to know if another piece of software is also using SRAI.
 */

static struct srai_context {
    int balance; /* use to track current number of SRAI API calls */

    uint8_t run; /* used to stop command processing */

    /* channel settings with default configuration for all channels. */
    NEXUS_SageChannelSettings channelSettings;
    /* default settings for Sage instance */
    NEXUS_SageOpenSettings sageSettings;

    /* allocation settings for NEXUS_MemoryAllocate() */
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_MemoryAllocationSettings secureAllocSettings;
    NEXUS_MemoryAllocationSettings exportAllocSettings;
    NEXUS_MemoryAllocationSettings arrAllocSettings;
    NEXUS_Addr secure_offset[3];
    unsigned secure_size[3];

    /* Nexus SAGE handle. One per SRAI instance. i.e. One per application. */
    NEXUS_SageHandle sage;

    /* container cache to optimize allocation and limit memory fragmentation */
    BSAGElib_Tools_ContainerCacheHandle hContainerCache;

    uint16_t platformNum; /* number of platforms in list */
    uint16_t moduleNum; /* number of modules in list */
    BLST_D_HEAD(SRAI_PlatformList, SRAI_Platform) platforms;
    BLST_D_HEAD(SRAI_ModuleList, SRAI_Module) modules;

    struct {
        /* async thread resources */
        uint8_t started;
        pthread_t thread;
        BKNI_EventHandle hEvent;
        BKNI_EventGroupHandle hEventGroup;

        struct {
            /* Management API is not available during callbacks processing:
             * use pthread_self to prevent deadlock from upper layer  */
            pthread_t lock_thread;
            uint8_t lock;
        } management;

        /* Watchdog event handling */
        struct {
            uint16_t count;
            BKNI_EventHandle hEvent;
            BLST_D_HEAD(SRAI_WatchdogCallbackList, SRAI_CallbackItem) callbacks;
        } watchdog;

        struct {
            BKNI_EventHandle hEvent;
            BLST_D_HEAD(SRAI_TATerminateCallbackList, SRAI_TATerminateCallbackItem) callbacks;
        } ta_terminate;

        /* Callback request event handling */
        struct {
            uint16_t count;
            BKNI_EventHandle hEvent;
        } callbackRequest;

        struct {
            BKNI_EventHandle hEvent;
        } indication;
    } async;

    BSAGElib_MemorySyncInterface i_memory_sync;
    BSAGElib_MemoryMapInterface i_memory_map;
    BSAGElib_MemoryAllocInterface i_memory_alloc;
    BSAGElib_SyncInterface i_sync_cache;

    SRAI_PlatformHandle antirollback_platform;
    SRAI_ModuleHandle antirollback_module;

    SRAI_PlatformHandle system_platform;
    SRAI_ModuleHandle system_module;

  int platform_refcount;

} _srai;

#include "nexus_platform_features.h" /* get access to heap indexes (platform specific) */
//static SRAI_Settings _srai_settings = {NEXUS_MEMC0_MAIN_HEAP, NEXUS_VIDEO_SECURE_HEAP};
static SRAI_Settings _srai_settings =
#ifdef NXCLIENT_SUPPORT
    {NXCLIENT_FULL_HEAP, NXCLIENT_VIDEO_SECURE_HEAP, NXCLIENT_EXPORT_HEAP, NXCLIENT_ARR_HEAP};
#else
    {NEXUS_MEMC0_MAIN_HEAP, NEXUS_VIDEO_SECURE_HEAP, NEXUS_EXPORT_HEAP, NEXUS_ARR_HEAP};
#endif

/* use .secure_* parameters to determine if a given memory block belongs to secure heap */
static inline bool _SRAI_IsMemoryInSecureHeap(uint64_t address)
{
    unsigned int i;
    unsigned int cnt=sizeof(_srai.secure_offset)/sizeof(_srai.secure_offset[0]);

    for(i=0;i<cnt;i++)
    {
        if((address >= _srai.secure_offset[i]) && ( address < (_srai.secure_offset[i] + _srai.secure_size[i])))
        {
            return true;
        }
    }

    return false;
}

#ifdef SRAI_GLOBAL_LOCK_BKNI
static BKNI_MutexHandle _srai_mutex = NULL;
#endif


/*
  Internal contexts are arranged as follow:

  +-------------------------------+
  |         Global _srai          |
  +--|-------|-----------------|--+
     |       |                 |     +--------------------------+
     |       |                 +-----| BSAGElib Container cache |
     |       |                       +--------------------------+
     |       |
     |       |
     |    +--+------------+
     |    |  Module List  +--> ... --> ModuleA --> ModuleB --> ... --> ModuleC --> ... --> NULL
     |    +--+------------+              |           |                   |
     |                                   |           |                   |
     |                                   |           |                   |
  +--+------------+                      |           |                   |
  | Platform List +                      |           |                   |
  +--+-------+----+                      |           |                   |
             |                           |           |                   |
             +---------> PlatformX <-----+-----------+                   |
                            |                                            |
                            |                                            |
                            +--> ... --> PlatformY <---------------------+
                                            |
                                            |
                                            +--> ... --> NULL
*/


/*
 * Utility functions definitions
 */

/*
 * SRAI can use a BKNI_Mutex as the lock/unlock mecanism
 * that is guaranteed to be available on all systems.
 *
 * No need of an explicit initialization function called by the upper layer.
 * Hence a defer init is implemented, but needs a static
 * initializer, (see PTHREAD_STATIC_INITIALIZER for an example).
 * However BKNI_Mutex* is not capable of providing a static initializer.
 *
 * The following provides a global lock that is static initializer capable, in
 * order to create/initialize resources safely.
 *
 * All SRAI could go with srai_global_lock / _unlock instead of BKNI_Mutex*
 * but srai_global_lock / _unlock is not guraranteed to exists on all systems.
 * If they are missing, SRAI_GLOBAL_LOCK_BKNI is defined and there will be no attempt to
 * use srai_global_lock(). Initialization will not be 100% thread proof.
 * It's beter-than-nothing in case a system is missing srai_global_lock()/_unlock()
 */
#ifdef SRAI_GLOBAL_LOCK_BKNI
#define SRAI_P_InitLock() {if (!_srai_mutex) { BKNI_CreateMutex(&_srai_mutex); }}
#define SRAI_P_CleanupLock() {if (_srai_mutex) { BKNI_DestroyMutex(_srai_mutex); _srai_mutex = NULL; }}
static void SRAI_P_Lock(void)
{
    BKNI_AcquireMutex(_srai_mutex);
}
static void SRAI_P_Unlock(void)
{
    BKNI_ReleaseMutex(_srai_mutex);
}
#define SRAI_P_LockLifecycle SRAI_P_Lock
#define SRAI_P_UnlockLifecycle SRAI_P_Unlock
/* management is not well protected in this mode. */
#define SRAI_P_LockManagement()
#define SRAI_P_UnlockManagement()
#else
void srai_global_lock(unsigned index);
void srai_global_unlock(unsigned index);
#define SRAI_P_InitLock()
#define SRAI_P_CleanupLock()
static void SRAI_P_Lock(void)
{
    srai_global_lock(0);
}
static void SRAI_P_Unlock(void)
{
    srai_global_unlock(0);
}
#define SRAI_P_LockLifecycle() srai_global_lock(1)
#define SRAI_P_UnlockLifecycle() srai_global_unlock(1)
#define SRAI_P_LockManagement() srai_global_lock(2)
#define SRAI_P_UnlockManagement() srai_global_unlock(2)
#endif

#define SRAI_P_InitLockLifecycle SRAI_P_InitLock
#define SRAI_P_CleanupLockLifecycle SRAI_P_CleanupLock

/*
 * Local functions definitions
 */

static void _srai_nexus_sage_success_callback(void *context, int param);
static void _srai_nexus_sage_last_error(srai_sync_call * sync);
static void _srai_nexus_sage_error_callback(void *context, int param);
static void _srai_nexus_sage_watchdog_callback(void *context, int param);
static void _srai_nexus_sage_callback_request_callback(void *context, int param);
static void _srai_nexus_sage_ta_terminate_callback(void *context, int param);
static NEXUS_Error _srai_nexus_sage_command(srai_sync_call * sync,
                                            NEXUS_SageCommand * command,
                                            BSAGElib_InOutContainer *container);
static int _srai_valid_heap(NEXUS_HeapHandle heap, NEXUS_MemoryType memoryType,
                            unsigned heapType, NEXUS_MemoryStatus *status);
static int _srai_get_heap(NEXUS_MemoryType memoryType, unsigned heapType, NEXUS_HeapHandle heap,
                          NEXUS_MemoryAllocationSettings *pSecureAllocSettings,
                          NEXUS_MemoryStatus *pHeapStatus, NEXUS_ClientConfiguration *pClientConfig);
static int _srai_nexus_sage_init(void);
static void _srai_nexus_sage_cleanup(void);
static void _srai_nexus_sage_settings_init(void);
static int _srai_init_settings(void);
static void _srai_init_vars(void);
static void _srai_init(void);
static void _srai_cleanup(void);
static int _srai_enter(void);
static void _srai_leave(void);
static void _srai_sync_cleanup(srai_sync_call *sync);
static int _srai_sync_init(srai_sync_call *sync, bool indication_callback);
static void _srai_containers_cache_adjust_max(void);
static void _srai_platform_cleanup(SRAI_PlatformHandle platform);
static void _srai_platform_push(SRAI_PlatformHandle platform);
static int _srai_platform_find(SRAI_PlatformHandle platform);
static int _srai_platform_acquire(SRAI_PlatformHandle platform, int pop);
static void _srai_module_push(SRAI_ModuleHandle module);
static int _srai_module_find(SRAI_ModuleHandle module);
static void _srai_module_pop(SRAI_ModuleHandle module);
static int _srai_module_acquire(SRAI_ModuleHandle module, int pop);
static void _srai_module_cleanup(SRAI_ModuleHandle module);
static void _srai_module_uninit_sage_command(SRAI_ModuleHandle module);
static BERR_Code _srai_convert_last_error(NEXUS_Error nexus_rc, BERR_Code sage_rc);
static void *_srai_memory_allocate_global(size_t size);
static void *_srai_memory_allocate_restricted(size_t size);
static void *_srai_memory_allocate(size_t size,
                                   NEXUS_MemoryAllocationSettings *settings);
static int _srai_containers_cache_init(void);
static void _srai_containers_cache_cleanup(void);

static void _srai_watchdog_handler(void);
static void _srai_ta_terminate_handler(void);
static void _srai_ta_terminate_mark_UNLOCKED(void);
static void _srai_ta_terminate_mark(void);
static void _srai_process_callback_request(SRAI_PlatformHandle platform);
static void _srai_callback_requests_handler(void);
static void _srai_indications_handler(void);
static void *_srai_async_handler(void *dummy);
static int _srai_async_init(void);
static void _srai_async_cleanup(void);

static void _srai_management_cleanup(void);

static void _srai_flush_cache(const void *addr, size_t size);
static void * _srai_offset_to_addr(uint64_t offset);
static uint64_t _srai_addr_to_offset(const void *addr);
#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
static bool _srai_is_sdl_valid(BSAGElib_SDLHeader *pHeader,uint32_t binSize);
#endif
/*
 * Functions implementation
 */


/* NEXUS_Callback prototype
 * fired by Nexus SAGE on successful call */
static void _srai_nexus_sage_success_callback(void *context, int param)
{
    srai_sync_call * sync = (srai_sync_call *)context;
    BSTD_UNUSED(param);

    sync->lastErrorNexus = NEXUS_SUCCESS;
    BKNI_SetEvent(sync->event);
}

/* Retrieve last error from Nexus SAGE by querying Channel Status */
static void _srai_nexus_sage_last_error(srai_sync_call * sync)
{
    NEXUS_SageChannelStatus status;

    if (NEXUS_SageChannel_GetStatus(sync->channel, &status) != NEXUS_SUCCESS) {
        sync->lastErrorNexus = NEXUS_UNKNOWN;
    }
    else {
        sync->lastErrorNexus = status.lastError;
        sync->lastErrorSage = status.lastErrorSage;
    }
}

/* NEXUS_Callback prototype
 * fired by Nexus SAGE on erroneous call */
static void _srai_nexus_sage_error_callback(void *context, int param)
{
    srai_sync_call * sync = (srai_sync_call *)context;
    BSTD_UNUSED(param);

    _srai_nexus_sage_last_error(sync);
    BKNI_SetEvent(sync->event);
}

/* NEXUS_Callback prototype
 * fired by Nexus SAGE on incoming callback request from SAGE */
static void _srai_nexus_sage_callback_request_callback(void *context, int param)
{
    SRAI_Platform *platform = (SRAI_Platform *)context;
    bool found;

    BSTD_UNUSED(param);

    SRAI_P_Lock();
    found = _srai_platform_find(platform);
    SRAI_P_Unlock();

    if (found != true) {
        BDBG_ERR(("%s: cannot find the platform %p", BSTD_FUNCTION, (void *)platform));
        goto end;
    }

    platform->callbacks.notified = 1;

    /* wake up callbacks handler (_srai_callback_request_handler is waiting on that event) */
    if (_srai.async.callbackRequest.hEvent) {
        BDBG_MSG(("%s: set callback request event", BSTD_FUNCTION));
        BKNI_SetEvent(_srai.async.callbackRequest.hEvent);
    }
    else {
        BDBG_WRN(("%s: callback request not enabled", BSTD_FUNCTION));
    }

end:
    return;
}

/* NEXUS_Callback prototype
 * fired by Nexus SAGE on incoming TA Termination indication from SAGE */
static void _srai_nexus_sage_ta_terminate_callback(void *context, int param)
{
    SRAI_Platform *platform = (SRAI_Platform *)context;
    bool found;
    BSTD_UNUSED(param);

    SRAI_P_Lock();
    found = _srai_platform_find(platform);

    if (found != true) {
        BDBG_ERR(("%s: cannot find the platform %p", BSTD_FUNCTION, (void *)platform));
        SRAI_P_Unlock();
        goto end;
    }

    platform->sync.terminated = 1;
    _srai_ta_terminate_mark_UNLOCKED();

    SRAI_P_Unlock();

end:
    return;
}

/* NEXUS_Callback prototype
 * fired by Nexus SAGE on incoming indication [id, value] from SAGE */
static void _srai_nexus_sage_indication_callback(void *context, int param)
{
    SRAI_Module *module = (SRAI_Module *)context;
    bool found;
    BSTD_UNUSED(param);

    SRAI_P_Lock();
    found = _srai_module_find(module);
    if (found != true) {
        BDBG_ERR(("%s: cannot find the module %p", BSTD_FUNCTION, (void *)module));
        SRAI_P_Unlock();
        goto end;
    }
    module->indication_count++;
    SRAI_P_Unlock();

    BKNI_SetEvent(_srai.async.indication.hEvent);
end:
    return;
}

/* NEXUS_Callback prototype
 * fired by Nexus SAGE on SAGE-side watchdog timeout event */
static void _srai_nexus_sage_watchdog_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    /* wake up watchdog handler (_srai_watchdog_handler is waiting on that event) */
    BDBG_MSG(("%s: set watchdog event", BSTD_FUNCTION));
    BKNI_SetEvent(_srai.async.watchdog.hEvent);
}

/* handle a watchdog event */
static void _srai_watchdog_handler(void)
{
    SRAI_PlatformHandle platform;
    SRAI_ModuleHandle module;

    _srai.async.watchdog.count++;
    BDBG_LOG(("%s: handle watchdog event #%u",
              BSTD_FUNCTION, _srai.async.watchdog.count));

    _srai_enter();
    SRAI_P_Lock();

    if (_srai.balance > 1) {
        /* Acquire Lock on all modules and platforms guarantee that there is nothing in progress
         * set reset flag on all channels will prevent sending commands through Nexus Sage. */
        for (platform = BLST_D_FIRST(&_srai.platforms); platform; platform = BLST_D_NEXT(platform, link)) {
            BKNI_AcquireMutex(platform->sync.mutex);
            platform->sync.reset = 1;
        }
        for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
            BKNI_AcquireMutex(module->sync.mutex);
            module->sync.reset = 1;
        }
    }

    /* reset Nexus Sage */
    _srai_nexus_sage_cleanup();
    if (_srai_nexus_sage_init()) {
        BDBG_ERR(("Nexus Sage reset failure"));
    }

    /* Unlock all modules and platforms. */
    if (_srai.balance > 1) {
        for (platform = BLST_D_FIRST(&_srai.platforms); platform; platform = BLST_D_NEXT(platform, link)) {
            BKNI_ReleaseMutex(platform->sync.mutex);
        }
        for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
            BKNI_ReleaseMutex(module->sync.mutex);
        }
    }

    SRAI_P_Unlock();
    _srai_leave();

    /* from now SRAI is unlocked; we can start firing watchdog callbacks */
    {
        SRAI_CallbackItem *item;
        SRAI_P_LockManagement();

        _srai.async.management.lock = 1;
        _srai.async.management.lock_thread = pthread_self();
        for (item = BLST_D_FIRST(&_srai.async.watchdog.callbacks); item; item = BLST_D_NEXT(item, link)) {
            item->callback();
        }
        _srai.async.management.lock_thread = 0;
        _srai.async.management.lock = 0;

        SRAI_P_UnlockManagement();
    }
}

static void _srai_ta_terminate_mark_UNLOCKED(void)
{
    SRAI_PlatformHandle platform;
    SRAI_ModuleHandle module;

    /* Find all terminated modules and set platforms flags accordingly */
    for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
        if (module->sync.terminated == 1) {
            module->platform->sync.terminated = 1;
        }
    }
    /* Now all terminated platforms are marked terminated, walk on all modules and set flags accordingly */
    for (platform = BLST_D_FIRST(&_srai.platforms); platform; platform = BLST_D_NEXT(platform, link)) {
        if (platform->sync.terminated == 0) {
            continue;
        }
        for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
            if (module->platform == platform) {
                module->sync.terminated = 1;
            }
        }
    }
    BKNI_SetEvent(_srai.async.ta_terminate.hEvent);
}

static void _srai_ta_terminate_mark(void)
{
    SRAI_P_Lock();
    _srai_ta_terminate_mark_UNLOCKED();
    SRAI_P_Unlock();
}

/* handle a TA terminate event (TA got killed on SAGE-side) */
static void _srai_ta_terminate_handler(void)
{
    _srai_enter();

    /* we need to loop on the list and find one by one the platforms that have been terminated
       to handle the callbacks logic, the SRAI lock is released
       hence after each processing, the platform is marked as already handled and
       the loop start over from the beggiing (since the platforms database may have changed) */
    do {
        SRAI_PlatformHandle platform;
        uint32_t platformId = 0;

        /* Acquire SRAI lock in order to walk on platform contexts */
        SRAI_P_Lock();
        for (platform = BLST_D_FIRST(&_srai.platforms); platform; platform = BLST_D_NEXT(platform, link)) {
            if ((platform->sync.terminated == 1) &&
                (platform->ta_terminate_reported == 0)) {
                platformId = platform->id;
                platform->ta_terminate_reported = 1;
                break;
            }
        }
        SRAI_P_Unlock();

        /* check stop condition */
        if (platform == NULL) {
            break;
        }
        /* from now SRAI is unlocked; we can start firing TA terminate callbacks
           SRAI_* API can be used from the callback, except for SRAI_Management_* ones */
        {
            SRAI_TATerminateCallbackItem *item;
            SRAI_P_LockManagement();

            _srai.async.management.lock = 1;
            _srai.async.management.lock_thread = pthread_self();
            for (item = BLST_D_FIRST(&_srai.async.ta_terminate.callbacks); item; item = BLST_D_NEXT(item, link)) {
                item->callback(platformId);
            }
            _srai.async.management.lock_thread = 0;
            _srai.async.management.lock = 0;

            SRAI_P_UnlockManagement();
        }

    } while (0);

    _srai_leave();
}

static void _srai_process_callback_request(SRAI_PlatformHandle platform)
{
    BERR_Code rsRc = BERR_NOT_AVAILABLE;
    BSAGElib_RpcMessage *message = platform->callbacks.message;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t sequence = 0;

    if (platform->callbacks.requestRecvCallback == NULL) {
        BDBG_WRN(("%s: received callback request for platform %p but no callback registered",
                  BSTD_FUNCTION, (void *)platform));
        goto callback_response;
    }

    BDBG_MSG(("%s: received callback request for platform %p, return code=%u",
              BSTD_FUNCTION, (void *)platform, rsRc));

    /* force read from physical memory */
    _srai.i_memory_sync.invalidate((const void *)message,
                                   sizeof(*message));

    sequence = message->sequence;

    /* container is not mandatory; but if any it must be the preallocated one */
    if (message->payloadOffset) {
        container = BSAGElib_Tools_ContainerOffsetToAddress(message->payloadOffset,
                                                            &_srai.i_memory_sync,
                                                            &_srai.i_memory_map);
        if (container != platform->callbacks.container) {
            BDBG_ERR(("%s: container inconsistency for callback request of platform %p",
                      BSTD_FUNCTION, (void *)platform));
            goto sync_back;
        }
    }

    /* Fireing the callback. It shall be non-blocking. */
    rsRc = platform->callbacks.requestRecvCallback(platform,
                                                   message->moduleCommandId,
                                                   container);

sync_back:
    /* regardless of any former return code, sync back the container if any */
    BSAGElib_Tools_ContainerAddressToOffset(container,
                                            &_srai.i_memory_sync,
                                            &_srai.i_memory_map);

    /* force write back to physical memory (should not have change) */
    _srai.i_memory_sync.flush((const void *)message,
                              sizeof(*message));

callback_response:
    {
        NEXUS_Error nexus_rc;
        NEXUS_SageResponse response =  {sequence, rsRc};
        nexus_rc = NEXUS_SageChannel_SendResponse(platform->sync.channel, &response);
        if (nexus_rc != NEXUS_SUCCESS) {
            BERR_Code rc = _srai_convert_last_error(nexus_rc, BERR_OS_ERROR);
            BDBG_ERR(("%s: cannot send callback response nexus_error=%u, rc=%u",
                      BSTD_FUNCTION, nexus_rc, rc));
        }
    }
}

/* handle a callback request event */
static void _srai_callback_requests_handler(void)
{
    SRAI_PlatformHandle platform;

    _srai.async.callbackRequest.count++;

    _srai_enter();

    SRAI_P_Lock();

    for (platform = BLST_D_FIRST(&_srai.platforms); platform; platform = BLST_D_NEXT(platform, link)) {
        if (platform->callbacks.notified) {
            _srai_process_callback_request(platform);
            platform->callbacks.notified = 0;
        }
    }

    SRAI_P_Unlock();
    _srai_leave();
}

/* handle a callback request event */
static void _srai_indications_handler(void)
{
    SRAI_ModuleHandle module;
    bool process_indication = true;
    int32_t  indication_count = 0;
    _srai_enter();

    while (process_indication) {
        process_indication = false;

        /* we should not held the global lock for too long
           strategy is to detect if there is another indication, handle next indication,
           then look again if there is still an indication to process */
        SRAI_P_Lock();

        for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
            if (module->indication_count > 0) {
                indication_count = module->indication_count;
                BDBG_MSG(("%s: %d Indications  available for module %p", BSTD_FUNCTION,indication_count, (void *)module));
                module->used = 1;
                module->indication_count =  0;
                process_indication = true;
                break;
            }
        }

        SRAI_P_Unlock();

        /* process all indications pending for module
           also during this whole time the module cannot be uninitialized */
        if (module && (indication_count > 0)) {
            NEXUS_Error nexus_err;
            do {
                uint32_t id = 0;
                uint32_t value = 0;
                nexus_err = NEXUS_SageChannel_GetNextIndication(module->sync.channel, &id, &value);
                switch (nexus_err) {
                case NEXUS_SUCCESS:
                    if (module->indicationCallback) {
                        module->indicationCallback(module, module->indicationCallbackArg, id, value);
                    }
                    else {
                        BDBG_ERR(("%s: internal ERR: module=%p received indication [id=%u, val=0x%x] but indicationCallback not set",
                                  BSTD_FUNCTION, (void *)module, id, value));
                    }
                    break;
                case NEXUS_NOT_AVAILABLE:
                    /* no more indication to process for this module,
                       it could be uninitialized from now on */
                    BDBG_MSG(("%s: No indication events available for module %p", BSTD_FUNCTION, (void *)module));
                    break;
                default:
                    BDBG_WRN(("%s: error getting next indication for module %p", BSTD_FUNCTION, (void *)module));
                    break;
                }
                indication_count--;
            } while ((nexus_err == NEXUS_SUCCESS) && (indication_count > 0) );
            module->used = 0;
        }
    }

    _srai_leave();
}

/* waits for an async event */
static void *_srai_async_handler(void *dummy)
{
    BSTD_UNUSED(dummy); /* pthread_create start routine prototype */

    do {
        unsigned nevents, i;
        BKNI_EventHandle notifiedEvents[5];

        BKNI_WaitForGroup(_srai.async.hEventGroup, BKNI_INFINITE,
                          notifiedEvents, 4, &nevents);
        if (!_srai.async.started || !_srai.run) {
            break;
        }

        BDBG_MSG(("%s: handle async event", BSTD_FUNCTION));

        for (i = 0; i < nevents; i++) {
            if (notifiedEvents[i] == NULL) {
                BDBG_ERR(("%s: invalid NULL event", BSTD_FUNCTION));
                continue;
            }
            if (notifiedEvents[i] == _srai.async.callbackRequest.hEvent) {
                _srai_callback_requests_handler();
            }
            else if (notifiedEvents[i] == _srai.async.watchdog.hEvent) {
                _srai_watchdog_handler();
            }
            else if (notifiedEvents[i] == _srai.async.ta_terminate.hEvent) {
                _srai_ta_terminate_handler();
            }
            else if (notifiedEvents[i] == _srai.async.indication.hEvent) {
                _srai_indications_handler();
            }
            else if (notifiedEvents[i] == _srai.async.hEvent) {
                BDBG_MSG(("%s: terminating", BSTD_FUNCTION));
            }
            else {
                BDBG_ERR(("%s: unknown event %p", BSTD_FUNCTION, (void *)(notifiedEvents[i])));
            }
        }

    } while (_srai.async.started && _srai.run);

    pthread_exit(NULL);
}

static int _srai_async_init(void)
{
    BERR_Code magnumRc = BERR_UNKNOWN;
    int rc = 0;
    int rc2 = 0;

    magnumRc = BKNI_CreateEventGroup(&_srai.async.hEventGroup);
    if (magnumRc != BERR_SUCCESS) { rc = -1; goto end; }

    magnumRc = BKNI_CreateEvent(&_srai.async.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -2; goto end; }

    magnumRc = BKNI_AddEventGroup(_srai.async.hEventGroup, _srai.async.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -3; goto end; }

    magnumRc = BKNI_CreateEvent(&_srai.async.watchdog.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -4; goto end; }

    magnumRc = BKNI_AddEventGroup(_srai.async.hEventGroup, _srai.async.watchdog.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -5; goto end; }

    magnumRc = BKNI_CreateEvent(&_srai.async.ta_terminate.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -4; goto end; }

    magnumRc = BKNI_AddEventGroup(_srai.async.hEventGroup, _srai.async.ta_terminate.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -5; goto end; }

    magnumRc = BKNI_CreateEvent(&_srai.async.callbackRequest.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -6; goto end; }

    magnumRc = BKNI_AddEventGroup(_srai.async.hEventGroup, _srai.async.callbackRequest.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -7; goto end; }

    magnumRc = BKNI_CreateEvent(&_srai.async.indication.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -8; goto end; }

    magnumRc = BKNI_AddEventGroup(_srai.async.hEventGroup, _srai.async.indication.hEvent);
    if (magnumRc != BERR_SUCCESS) { rc = -9; goto end; }

    rc2 = pthread_create(&_srai.async.thread, NULL, _srai_async_handler, NULL);
    if (rc2 != 0) { rc = -8; goto end; }

    _srai.async.started = 1;

end:
    if (rc) {
        BDBG_ERR(("cannot init async; failure at %d ; err = %u (%d)", rc, magnumRc, rc2));
        _srai_async_cleanup();
    }
    return rc;
}

static void _srai_async_cleanup(void)
{
    if (_srai.async.started) {
        _srai.async.started = 0;
        BKNI_SetEvent(_srai.async.hEvent);
        pthread_join(_srai.async.thread, NULL);
    }

    if (_srai.async.indication.hEvent) {
        BKNI_DestroyEvent(_srai.async.indication.hEvent);
        _srai.async.indication.hEvent = NULL;
    }

    if (_srai.async.callbackRequest.hEvent) {
        BKNI_DestroyEvent(_srai.async.callbackRequest.hEvent);
        _srai.async.callbackRequest.hEvent = NULL;
    }

    if (_srai.async.ta_terminate.hEvent) {
        BKNI_DestroyEvent(_srai.async.ta_terminate.hEvent);
        _srai.async.ta_terminate.hEvent = NULL;
    }

    if (_srai.async.watchdog.hEvent) {
        BKNI_DestroyEvent(_srai.async.watchdog.hEvent);
        _srai.async.watchdog.hEvent = NULL;
    }

    if (_srai.async.hEvent) {
        BKNI_DestroyEvent(_srai.async.hEvent);
        _srai.async.hEvent = NULL;
    }

    if (_srai.async.hEventGroup) {
        BKNI_DestroyEventGroup(_srai.async.hEventGroup);
        _srai.async.hEventGroup = NULL;
    }
}

/* convert/agregate last errors from Nexus SAGE in a last error for upper layer */
static BERR_Code _srai_convert_last_error(NEXUS_Error nexus_rc, BERR_Code sage_rc)
{
    BERR_Code rc;

    switch (nexus_rc) {
    case NEXUS_SUCCESS:
        rc = BERR_SUCCESS;
        /* BDBG_MSG(("%s: SUCCESS", BSTD_FUNCTION)); */
        break;
    case NEXUS_ERROR_SAGE_SIDE:
        BDBG_WRN(("%s: Sage failure %x", BSTD_FUNCTION, sage_rc));
        switch (sage_rc) {
        case BSAGE_ERR_ALREADY_INITIALIZED:
        case BSAGE_ERR_MODULE_ID:
        case BSAGE_ERR_PLATFORM_ID:
        case BSAGE_ERR_MODULE_COMMAND_ID:
        case BSAGE_ERR_SYSTEM_COMMAND_ID:
        case BSAGE_ERR_STATE:
        case BSAGE_ERR_CONTAINER_REQUIRED:
        case BSAGE_ERR_SIGNATURE_MISMATCH:
        case BSAGE_ERR_RESET:
        case BSAGE_ERR_TA_TERMINATED:
            rc = sage_rc;
            break;
        default:
            rc = BSAGE_ERR_INTERNAL;
            break;
        }
        break;
    case NEXUS_ERROR_SAGE_TRANSPORT:
        BDBG_WRN(("%s: Transport failure", BSTD_FUNCTION));
        rc = BERR_OS_ERROR;
        break;
    case NEXUS_ERROR_SAGE_WATCHDOG:
        BDBG_WRN(("%s: Watchdog failure. SAGE in reset.", BSTD_FUNCTION));
        rc = BSAGE_ERR_RESET;
        break;
    default:
        BDBG_WRN(("%s: Nexus failure %x", BSTD_FUNCTION, nexus_rc));
        /* BERR_Code codes are mapped to NEXUS_Error */
        rc = nexus_rc;
        break;
    }

    return rc;
}

/* timeout (in milliseconds) of command processing, at SRAI level. */
#define SRAI_NEXUS_SAGE_COMMAND_TIMEOUT_MS 100
/* process a sage command synchronously
 * -1- convert container for sage
 * -2- Send command to SAGE
 * -3- Wait for event from success or error callback
 * -4- get return code
 * -5- restore container */
static BERR_Code _srai_nexus_sage_command(srai_sync_call *sync,
                                           NEXUS_SageCommand *command,
                                           BSAGElib_InOutContainer *container)
{
    NEXUS_Error nexus_rc;
    BERR_Code rc = BSAGE_ERR_INTERNAL;
    int nb = 0;

    if (sync->reset) {
        BDBG_ERR(("SAGE has been reset, %p is not valid anymore.", (void *)sync));
        rc = BSAGE_ERR_RESET;
        goto end;
    }

    if (sync->terminated) {
        /* Do not print errors for deprovisioning commands */
        if ((command->systemCommandId == BSAGElib_SystemCommandId_ePlatformClose) ||
            (command->systemCommandId == BSAGElib_SystemCommandId_eModuleUninit)) {
            /* let the command go, so lower layers can clean their contexts */
        }
        else {
            BDBG_ERR(("SAGE-side TA had been terminated, %p is not valid anymore.", (void *)sync));
            rc = BSAGE_ERR_TA_TERMINATED;
            goto end;
        }
    }

    /* -1- prepare command and container to be used used by SAGE-side */
    command->payloadOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                     &_srai.i_memory_sync,
                                                                     &_srai.i_memory_map);

    /* -2- send the command and wait for the registered callback to be fired (and set the event) */
    nexus_rc = NEXUS_SageChannel_SendCommand(sync->channel, command);
    if (nexus_rc != NEXUS_SUCCESS) {
        rc = _srai_convert_last_error(nexus_rc, BERR_OS_ERROR);
        goto end_convert;
    }

    /* -3- Wait for return from SAGE: event from success or error callback */
    do {
        /* Force timeout error every SRAI_NEXUS_SAGE_COMMAND_TIMEOUT_MS milliseconds
         * in order to break in case of SRAI shutdown */
        BERR_Code magnum_rc = BKNI_WaitForEvent(sync->event, SRAI_NEXUS_SAGE_COMMAND_TIMEOUT_MS);
        if (magnum_rc == BERR_SUCCESS) {
            /* success or error callbacks had been triggered
             * --> sage command is consumed */
            break;
        }
        else if (magnum_rc == BERR_TIMEOUT) {
            BDBG_WRN(("BKNI_WaitForEvent TIMEOUT %u ms",
                      (++nb*SRAI_NEXUS_SAGE_COMMAND_TIMEOUT_MS)));
            if (!_srai.run) {
                rc = BERR_TIMEOUT;
                goto end_convert;
            }
        }
        else {
            /* ??! TODO handle properly */
            (void)BERR_TRACE(magnum_rc);
            BDBG_ERR(("BKNI_WaitForEvent failure (%u)", magnum_rc));
            rc = BERR_OS_ERROR;
            goto end_convert;
        }
    } while (1);

    /* -4- get return code */
    rc = _srai_convert_last_error(sync->lastErrorNexus, sync->lastErrorSage);
    switch (rc) {
        case BSAGE_ERR_RESET:
            sync->reset = 1;
            break;
        case BSAGE_ERR_TA_TERMINATED:
            sync->terminated = 1;
            _srai_ta_terminate_mark();
            break;
    default:
        break;
    }

end_convert:
    /* -5- restore container to be used by the Host */
    BSAGElib_Tools_ContainerOffsetToAddress(command->payloadOffset,
                                            &_srai.i_memory_sync,
                                            &_srai.i_memory_map);
end:
    return rc;
}

/* Check if heap is accessible from both Host-side and SAGE-side. */
static int _srai_valid_heap(NEXUS_HeapHandle heap,
                            NEXUS_MemoryType memoryType,
                            unsigned heapType,
                            NEXUS_MemoryStatus *status)
{
    NEXUS_Error nexus_rc;

    if (!heap) {
        return 0;
    }

    nexus_rc = NEXUS_Heap_GetStatus(heap, status);
    if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("NEXUS_Heap_GetStatus(%p) failure (%d) ", (void *)heap, nexus_rc));
        return 0;
    }
    if (status->memoryType == memoryType &&  /* memory type requested */
        (status->heapType & heapType)
#if ((BCHP_VER >= BCHP_VER_A0) && (BCHP_CHIP == 7584)) || ((BCHP_VER >= BCHP_VER_B0) && (BCHP_CHIP == 7435))
        && status->memcIndex == 0          /* only MEMC0 is accessible by SAGE-side on Zeus30 */
#endif
       ) {
        return 1; /* the heap is valid */
    }

    return 0;
}
/* get a valid heap with a specific memory type mapping
 * this function will try a heap search if given heap is not compatible */
static int _srai_get_heap(NEXUS_MemoryType memoryType, unsigned heapType, NEXUS_HeapHandle heap,
                          NEXUS_MemoryAllocationSettings *pSecureAllocSettings,
                          NEXUS_MemoryStatus *pHeapStatus, NEXUS_ClientConfiguration *pClientConfig)
{
    int rc = -1;
    NEXUS_HeapHandle validHeap = NULL;

    if (_srai_valid_heap(heap, memoryType, heapType, pHeapStatus)) {
        validHeap = heap;
    }
    else {
        int i;
        if (heap != NULL) {
            BDBG_WRN(("%s: given heap %p is not valid. Consider using SRAI_SetSettings() with proper heap indexes.",
                      BSTD_FUNCTION, (void *)heap));
        }
        for (i = 0; i < NEXUS_MAX_HEAPS; i++) {
            if (_srai_valid_heap(pClientConfig->heap[i], memoryType, heapType, pHeapStatus)) {
                validHeap = pClientConfig->heap[i];
                BDBG_WRN(("%s: heap search found matching heap #%d", BSTD_FUNCTION, i));
                break;
            }
        }
    }

    if (validHeap) {
        NEXUS_Memory_GetDefaultAllocationSettings(pSecureAllocSettings);
        pSecureAllocSettings->heap = validHeap;
        pSecureAllocSettings->alignment = SAGE_ALIGN_SIZE; /* align SAGE_ALIGN_SIZE for SAGE-side concerns. */
        BDBG_MSG(("%s: secure alloc settings: heap: %p of type %d",
                  BSTD_FUNCTION, (void *)validHeap, memoryType));
        rc = 0;
    }

    return rc;
}

/* initialize Sage settings once for all. */
static void _srai_nexus_sage_settings_init(void)
{
    NEXUS_Sage_GetDefaultOpenSettings(&_srai.sageSettings);
    _srai.sageSettings.watchdogCallback.callback = _srai_nexus_sage_watchdog_callback;
    NEXUS_SageChannel_GetDefaultSettings(&_srai.channelSettings);
    _srai.channelSettings.heap = _srai.allocSettings.heap;
}

/* Initialize ressources to communicate with SAGE-side through Nexus Sage */
static int _srai_nexus_sage_init(void)
{
    _srai.sage = NEXUS_Sage_Open(0, &_srai.sageSettings);
    if (!_srai.sage) {
        BDBG_ERR(("NEXUS_Sage_Open() failure"));
        return 1;
    }

    BDBG_MSG(("NEXUS Sage instance=%p", (void *)_srai.sage));
    return 0;
}

/* Cleanup ressources to related to Nexus Sage */
static void _srai_nexus_sage_cleanup(void)
{
    if (_srai.sage) {
        NEXUS_Sage_Close(_srai.sage);
        _srai.sage = NULL;
    }
}

/* Init globals to maintain init state */
static int _srai_init_state = 0; /* used by _srai_init() and _srai_cleanup() */
static int _srai_varinit_state = 0; /* used by _srai_init() and _srai_cleanup(). updated once. */

/* Initialize SRAI settings. Done once in defer init and once for each subsequent SRAI_SetSettings() */
static int _srai_init_settings(void)
{
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryStatus heapStatus;
    int rc = 0;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    if (_srai_get_heap(NEXUS_MemoryType_eFull, NEXUS_HEAP_TYPE_MAIN,
                       clientConfig.heap[_srai_settings.generalHeapIndex],
                       &_srai.allocSettings, &heapStatus, &clientConfig)) {
        BDBG_ERR(("%s: Cannot retrieve general heap from index %d",
                  BSTD_FUNCTION, _srai_settings.generalHeapIndex));
        rc -= 0x1;
    }

    if (_srai_get_heap(NEXUS_MemoryType_eSecure,
                       NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION,
                       clientConfig.heap[_srai_settings.videoSecureHeapIndex],
                       &_srai.secureAllocSettings, &heapStatus, &clientConfig)) {
        BDBG_ERR(("%s: Cannot retrieve secure heap from index %d.",
                  BSTD_FUNCTION, _srai_settings.videoSecureHeapIndex));
        BDBG_ERR(("%s: --> check for secure_heap=y environment variable", BSTD_FUNCTION));
        rc -= 0x2;
    }
    else
    {
        _srai.secure_offset[0] = heapStatus.offset;
        _srai.secure_size[0] = heapStatus.size;
        BDBG_MSG(("Found CRR: offset: " BDBG_UINT64_FMT " size: 0x%x", BDBG_UINT64_ARG(_srai.secure_offset[0]), _srai.secure_size[0]));
    }

    if (_srai_get_heap(NEXUS_MemoryType_eSecure,
                       NEXUS_HEAP_TYPE_EXPORT_REGION,
                       clientConfig.heap[_srai_settings.exportHeapIndex],
                       &_srai.exportAllocSettings, &heapStatus, &clientConfig)) {
        if (clientConfig.heap[_srai_settings.exportHeapIndex] != NULL) {
            BDBG_WRN(("%s: Export Secure heap could not be configured", BSTD_FUNCTION));
        }
        /* Explicitly clear the exportAllocSettings */
        BKNI_Memset(&_srai.exportAllocSettings, 0, sizeof(_srai.exportAllocSettings));
    }
    else {
        /* get secure heap boundaries for _SRAI_IsMemoryInSecureHeap() */
        _srai.secure_offset[1] = heapStatus.offset;
        _srai.secure_size[1] = heapStatus.size;
        BDBG_MSG(("Found XRR: offset: " BDBG_UINT64_FMT " size: 0x%x", BDBG_UINT64_ARG(_srai.secure_offset[1]), _srai.secure_size[1]));
    }

    if (_srai_get_heap(NEXUS_MemoryType_eSecure,
                       NEXUS_HEAP_TYPE_ARR,
                       clientConfig.heap[_srai_settings.arrHeapIndex],
                       &_srai.arrAllocSettings, &heapStatus, &clientConfig)) {
        if (clientConfig.heap[_srai_settings.arrHeapIndex] != NULL) {
            BDBG_WRN(("%s: ARR heap could not be configured", BSTD_FUNCTION));
        }
        /* Explicitly clear the exportAllocSettings */
        BKNI_Memset(&_srai.arrAllocSettings, 0, sizeof(_srai.arrAllocSettings));
    }
    else {
        /* get secure heap boundaries for _SRAI_IsMemoryInSecureHeap() */
        _srai.secure_offset[2] = heapStatus.offset;
        _srai.secure_size[2] = heapStatus.size;
        BDBG_MSG(("Found ARR: offset: " BDBG_UINT64_FMT " size: 0x%x", BDBG_UINT64_ARG(_srai.secure_offset[2]), _srai.secure_size[2]));
    }

    return rc;
}

static uint64_t _srai_addr_to_offset(const void *addr)
{
    return (uint64_t)NEXUS_AddrToOffset(addr);
}
static void * _srai_offset_to_addr(uint64_t offset)
{
    return NEXUS_OffsetToCachedAddr((uint32_t)offset);
}
static void _srai_flush_cache(const void *addr, size_t size)
{
    if (!_SRAI_IsMemoryInSecureHeap(_srai_addr_to_offset(addr))) {
        NEXUS_FlushCache(addr, size);
    }
}

static int _srai_containers_cache_init(void)
{
    _srai.i_memory_alloc.malloc = _srai_memory_allocate_global;
    _srai.i_memory_alloc.malloc_restricted = NULL;/* not required for ContainerCache */
    _srai.i_memory_alloc.free = NEXUS_Memory_Free;
    _srai.i_sync_cache.lock = SRAI_P_Lock;
    _srai.i_sync_cache.unlock = SRAI_P_Unlock;
    _srai.hContainerCache = BSAGElib_Tools_ContainerCache_Open(&_srai.i_memory_alloc,
                                                               &_srai.i_sync_cache);
    if (!_srai.hContainerCache) {
        return 1;
    }

    return 0;
}

static void _srai_containers_cache_cleanup(void)
{
    if (_srai.hContainerCache) {
        BSAGElib_Tools_ContainerCache_Close(_srai.hContainerCache);
        _srai.hContainerCache = NULL;
    }
}

/* Initialize SRAI variables to default value */
static void _srai_init_vars(void)
{
    if (_srai_varinit_state) {
        return;
    }

    BDBG_MSG(("Initialize SRAI variables"));
    BKNI_Memset(&_srai, 0, sizeof(_srai));
    BLST_D_INIT(&_srai.platforms);
    BLST_D_INIT(&_srai.modules);
    BLST_D_INIT(&_srai.async.watchdog.callbacks);
    if (_srai_init_settings()) {
        BDBG_ERR(("%s: Cannot initialize SRAI lib properly: Bad settings.", BSTD_FUNCTION));
    }

    /* Setup memory map interface */
    _srai.i_memory_sync.flush = _srai_flush_cache;
    _srai.i_memory_sync.invalidate = _srai_flush_cache;
    _srai.i_memory_map.addr_to_offset = _srai_addr_to_offset;
    _srai.i_memory_map.offset_to_addr = _srai_offset_to_addr;

    if (_srai_containers_cache_init()) {
        BDBG_ERR(("%s: cannot initialize container cache", BSTD_FUNCTION));
        _srai_cleanup();
        return;
    }

    _srai_nexus_sage_settings_init();
    _srai_varinit_state = 1;
}

/*
 * SRAI instance cleanup is problematic as multiple
 * clients can be using it in parallel.
 * Use _srai_enter() and _srai_leave() routines
 * ir order to have an updated balance of the number of
 * calls currently active.
 *
 * On leave, check if the SRAI can be cleaned,
 * i.e. balance is 0 and there is not any platform opened
 * nor module initialized.
 *
 * Note: SRAI cleanup does not garbage collect memory blocks
 */
static void _srai_init_vars_locked(void)
{
    SRAI_P_InitLockLifecycle();
    if (_srai_varinit_state == 0) {
        SRAI_P_LockLifecycle();
        _srai_init_vars();
        SRAI_P_UnlockLifecycle();
    }
}
static int _srai_enter(void)
{
    SRAI_P_InitLockLifecycle();
    SRAI_P_LockLifecycle();
    _srai_init();
    ++_srai.balance;
    SRAI_P_UnlockLifecycle();
    return !_srai_init_state;
}
static void _srai_leave(void)
{
    SRAI_P_LockLifecycle();
    --_srai.balance;
    if (!_srai.balance &&
        !_srai.platformNum &&
        BLST_D_EMPTY(&_srai.async.watchdog.callbacks)) {
        if (!_srai.moduleNum) {
            _srai_cleanup();
        } else {
            BDBG_ERR(("%s: cannot auto-cleanup; %u orphan module(s)",
                      BSTD_FUNCTION, _srai.moduleNum));
        }
    }
    SRAI_P_UnlockLifecycle();
}

/* Initialize SRAI instance
 * - set variables to default value
 * - retrieve default settings
 * - initialize communication with SAGE */
static void _srai_init(void)
{
   if (_srai_init_state) {
       return;
    }

    /* Initialize variables */
    _srai_init_vars();

    BDBG_MSG(("Initialize SRAI features"));
    if (_srai_nexus_sage_init() ||
        _srai_async_init()) {
        _srai_cleanup();/* TODO: how to handle that case? */
        return;
    }

    /* only when fully initialized set init flag */
    _srai.run = 1;
    _srai_init_state = 1;
 }

/* cleanup/shutdown SRAI instance */
static void _srai_cleanup(void)
{
    if (!_srai_varinit_state) {
        /* cannot check variables that has not been initialized once. */
        return;
    }

    _srai.run = 0;

    _srai_async_cleanup();
    _srai_management_cleanup();
    _srai_nexus_sage_cleanup();
    _srai_containers_cache_cleanup();

    _srai_init_state = 0;

    _srai_varinit_state = 0;

    BDBG_MSG(("SRAI features uninitialized"));
}

/* do the memory allocation */
static void *_srai_memory_allocate(size_t size,
                                      NEXUS_MemoryAllocationSettings *settings)
{
    void *ret = NULL;
    NEXUS_Error rc;
    size_t roundedSize = RoundUpP2(size, SAGE_ALIGN_SIZE);

    if (settings->heap == NULL) {
        BDBG_ERR(("%s: heap is not configured", BSTD_FUNCTION));
        goto end;
    }
    rc = NEXUS_Memory_Allocate(roundedSize, settings, &ret);

    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s(%u) failure (%d)", BSTD_FUNCTION, (uint32_t)size, rc));
        ret = NULL;
    }

end:
    return (uint8_t *)ret;
}

static void *_srai_memory_allocate_global(size_t size)
{
    return _srai_memory_allocate(size, &_srai.allocSettings);
}

static void *_srai_memory_allocate_restricted(size_t size)
{
    return _srai_memory_allocate(size, &_srai.secureAllocSettings);
}

static void *_srai_memory_allocate_export(size_t size)
{
    return _srai_memory_allocate(size, &_srai.exportAllocSettings);
}

static void *_srai_memory_allocate_arr(size_t size)
{
    return _srai_memory_allocate(size, &_srai.arrAllocSettings);
}

/* Get/Set settings */
void SRAI_GetSettings(SRAI_Settings *pSettings /* [out] */)
{
    *pSettings = _srai_settings;
}
BERR_Code SRAI_SetSettings(SRAI_Settings *pSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER(SRAI_SetSettings);

    if (!pSettings) {
        BDBG_WRN(("%s: NULL settings. i.e. will use defaults.", BSTD_FUNCTION));
        goto end;
    }

    if (pSettings->generalHeapIndex >= NEXUS_MAX_HEAPS) {
        BDBG_ERR(("%s: cannot set general heap index to %u",
                  BSTD_FUNCTION, pSettings->generalHeapIndex));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (pSettings->videoSecureHeapIndex >= NEXUS_MAX_HEAPS) {
        BDBG_ERR(("%s: cannot set video secure heap index to %u",
                  BSTD_FUNCTION, pSettings->videoSecureHeapIndex));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (pSettings->exportHeapIndex >= NEXUS_MAX_HEAPS) {
        BDBG_ERR(("%s: cannot set export secure heap index to %u",
                  BSTD_FUNCTION, pSettings->exportHeapIndex));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (pSettings->arrHeapIndex >= NEXUS_MAX_HEAPS) {
        BDBG_ERR(("%s: cannot set arr heap index to %u",
                  BSTD_FUNCTION, pSettings->arrHeapIndex));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    _srai_settings = *pSettings;

    if (_srai_varinit_state) {
        /* re do settings */
        _srai_init_settings();
    }
    /* else: will be achieved on first SRAI call */

 end:
    BDBG_LEAVE(SRAI_SetSettings);
    return rc;
}

/* Allocate a memory block of given size.
 * The returned memory block can be accessed from both the Host and the SAGE systems. */
/* size is rounded up in the memory allocation function, _srai_memory_allocate to
SAGE_ALIGN_SIZE */
uint8_t *SRAI_Memory_Allocate(uint32_t size, SRAI_MemoryType memoryType)
{
    uint8_t *ret;

    BDBG_ENTER(SRAI_Memory_Allocate);

    _srai_init_vars_locked();

    switch (memoryType) {
    case SRAI_MemoryType_Shared:
        ret = (uint8_t *)_srai_memory_allocate_global(size);
        break;
    case SRAI_MemoryType_Crr:
        ret = (uint8_t *)_srai_memory_allocate_restricted(size);
        break;
    case SRAI_MemoryType_SageExport:
        ret = (uint8_t *)_srai_memory_allocate_export(size);
        break;
    case SRAI_MemoryType_Arr:
        ret = (uint8_t *)_srai_memory_allocate_arr(size);
        break;
    default:
        ret = NULL;
        break;
    }

    /* BDBG_MSG(("%s: %d bytes at 0x%08x", BSTD_FUNCTION, size, ret)); */
    BDBG_LEAVE(SRAI_Memory_Allocate);
    return (uint8_t *)ret;
}

/* Free a memory block allocated using SRAI_Memory_Allocate() */
void SRAI_Memory_Free(uint8_t *pMemory)
{
    BDBG_ENTER(SRAI_Memory_Free);

/*    BDBG_MSG(("%s: 0x%08x", BSTD_FUNCTION, pMemory));*/
    NEXUS_Memory_Free((void *)pMemory);

    BDBG_LEAVE(SRAI_Memory_Free);
}

/* Allocate a SAGE In/Out Container used to backhaul parameters
 * during Host to SAGE communication.
 * Returned container is zeroed. */
BSAGElib_InOutContainer *SRAI_Container_Allocate(void)
{
    BSAGElib_InOutContainer *container;

    BDBG_ENTER(SRAI_Container_Allocate);

    _srai_init_vars_locked();

    container = BSAGElib_Tools_ContainerCache_Allocate(_srai.hContainerCache);

    BDBG_LEAVE(SRAI_Container_Allocate);
    return container;
}

/* Free an In/Out container allocated using SRAI_Container_Allocate() */
void SRAI_Container_Free(BSAGElib_InOutContainer *container)
{
    BDBG_ENTER(SRAI_Container_Free);

    _srai_init_vars_locked();

    BSAGElib_Tools_ContainerCache_Free(_srai.hContainerCache, container);

    BDBG_LEAVE(SRAI_Container_Free);
}

/* cleanup a sync context. free related resources */
static void _srai_sync_cleanup(srai_sync_call *sync)
{
    if (sync->channel) {
        NEXUS_Sage_DestroyChannel(sync->channel);
        sync->channel = NULL;
    }
    if (sync->event) {
        BKNI_DestroyEvent(sync->event);
        sync->event = NULL;
    }
    if (sync->mutex) {
        BKNI_DestroyMutex(sync->mutex);
        sync->mutex = NULL;
    }
}

/* Init module/platform sync context used to run sage commands synchronously */
static int _srai_sync_init(srai_sync_call *sync, bool indication_callback)
{
    int rc = 1;
    BERR_Code err;
    NEXUS_SageChannelSettings channelSettings;

    /* create resources: a mutex and an event */

    err = BKNI_CreateMutex(&sync->mutex);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("cannot create sync mutex BKNI_CreateMutex failure (%d)", err));
        sync->mutex = NULL;
        goto end;
    }

    err = BKNI_CreateEvent(&sync->event);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("cannot create sync event BKNI_CreateEvent failure (%d)", err));
        sync->event = NULL;
        goto end;
    }

    /* Channel settings */
    channelSettings = _srai.channelSettings;
    /* register result callbacks
     * Fired by Nexus SAGE when receiving response from SAGE-side:
     * - successCallback in case of success
     * - errorCallback for failure. In that case, SRAI will have to poll
     *   channel status in order to retrieve last error value. */
    channelSettings.successCallback.context = (void *)sync;
    channelSettings.successCallback.callback = _srai_nexus_sage_success_callback;
    channelSettings.errorCallback.context = (void *)sync;
    channelSettings.errorCallback.callback = _srai_nexus_sage_error_callback;
    /* register 'callback request receive' callback
     * Fired by Nexus SAGE when receiving a 'callback request' from SAGE-side */
    channelSettings.callbackRequestRecvCallback.context = (void *)sync;
    channelSettings.callbackRequestRecvCallback.callback = _srai_nexus_sage_callback_request_callback;
    channelSettings.taTerminateCallback.context = (void *)sync;
    channelSettings.taTerminateCallback.callback = _srai_nexus_sage_ta_terminate_callback;
    if (indication_callback) {
        channelSettings.indicationCallback.context = (void *)sync;
        channelSettings.indicationCallback.callback = _srai_nexus_sage_indication_callback;
    }
    else {
        channelSettings.indicationCallback.context = NULL;
        channelSettings.indicationCallback.callback = NULL;
    }

    /* Create Nexus Sage channels for all communication with SAGE-side */
    sync->channel = NEXUS_Sage_CreateChannel(_srai.sage, &channelSettings);
    if (!sync->channel) {
        BDBG_ERR(("cannot open NEXUS Sage channel"));
        goto end;
    }

    /* success */
    rc = 0;

end:
    if (rc) {
        _srai_sync_cleanup(sync);
    }
    return rc;
}

/* Adjust/Optimize containers cache size:
 * SRAI usage should be 1- init platforms 2- init modules 3- use modules 4- uninit modules 5- uninit platforms
 *   Once phase 1- is over, we can assume that application will use at least one module on that platform
 *   Once phase 2- is over, we can assume that application will use all initialized modules
 *   On phase 3- with no multithread access to module, only one container per module is used
 *   Phase 4- and 5- are symetrical to phase 2- and 1-
 * i.e. Platforms are only used on init phase, modules are limited to process one command at a time
 * ==> Configuring the deep of the container cache to the largest
 *     of <number of modules> and <number of platforms> should optimize
 *     cache resources and lowerize memory fragmentation.
 * Note: setting the cache size to 0 actually completely flush the cache */
static void _srai_containers_cache_adjust_max(void)
{
    uint16_t max = (_srai.platformNum > _srai.moduleNum) ? _srai.platformNum : _srai.moduleNum;
    BSAGElib_Tools_ContainerCache_SetMax(_srai.hContainerCache, max);
}

/* Cleanup a platform and all associated resources. */
static void _srai_platform_cleanup(SRAI_PlatformHandle platform)
{
    /* First, check that the platorm is empty. If not cleanup all attached modules. */
    do {
        SRAI_ModuleHandle module;

        /* search for module that refer to current platform */
        SRAI_P_Lock();
        for (module = BLST_D_FIRST(&_srai.modules); module; module = BLST_D_NEXT(module, link)) {
            if (module->platform == platform) {
                _srai_module_pop(module);
                break;
            }
        }
        SRAI_P_Unlock();

        if (!module) {
            /* no more module attached to the platform */
            break;
        }

        BDBG_WRN(("%s: uninit module=%p : should have been uninit before calling Platform_Close",
                  BSTD_FUNCTION, (void *)module));

        /* garbage collector: uninit module attached to that platform */

        BKNI_AcquireMutex(module->sync.mutex); /* TODO handle ret code */
        _srai_module_uninit_sage_command(module);
        BKNI_ReleaseMutex(module->sync.mutex);
        _srai_module_cleanup(module);

    } while(1);

    if (platform->callbacks.requestRecvCallback) {
        if (platform->callbacks.container) {
            SRAI_Container_Free(platform->callbacks.container);
            platform->callbacks.container = NULL;
        }
        if (platform->callbacks.message) {
            SRAI_Memory_Free((uint8_t *)platform->callbacks.message);
            platform->callbacks.message = NULL;
        }
        platform->callbacks.requestRecvCallback = NULL;
    }
    _srai_containers_cache_adjust_max();

    _srai_sync_cleanup(&platform->sync);
    BKNI_Free((uint8_t *)platform);
}

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
/* Install a platform.
 * SAGE side Platform/TA can by dynamically loaded from the Host.
 * Host application can use this API to install any Platform/TA on the SAGE side.
 * Once the Platform/TA has been installed on the SAGE side successfully, Host can then call:
 * SRAI_Platform_XXX APIs to Open and Init the Platform and Modules in the Platform. */
BERR_Code SRAI_Platform_Install(uint32_t platformId,
                             uint8_t *binBuff,
                             uint32_t binSize)
{
    BSTD_UNUSED(platformId);
    BSTD_UNUSED(binBuff);
    BSTD_UNUSED(binSize);

    BDBG_MSG(("This API is valid only for SAGE 3X and later binary"));

    return BERR_SUCCESS;

}

/* Un-install a platform.
* This API is used to Un-install the Platform/TA that has been installed by SRAI_Platform_Install API */
BERR_Code SRAI_Platform_UnInstall(uint32_t platformId )
{
    BSTD_UNUSED(platformId);

    BDBG_MSG(("This API is valid only for SAGE 3X and later binary"));

    return BERR_SUCCESS;
}

BERR_Code SRAI_Platform_EnableCallbacks(SRAI_PlatformHandle platform,
                                        SRAI_RequestRecvCallback callback)
{
    BSTD_UNUSED(platform);
    BSTD_UNUSED(callback);

    BDBG_MSG(("This API is valid only for SAGE 3X and later binary"));

    return BERR_SUCCESS;
}
#else
/* Install a platform.
 * SAGE side Platform/TA can by dynamically loaded from the Host.
 * Host application can use this API to install any Platform/TA on the SAGE side.
 * Once the Platform/TA has been installed on the SAGE side successfully, Host can then call:
 * SRAI_Platform_XXX APIs to Open and Init the Platform and Modules in the Platform.
 * Note: before loading the SDL sanity checks are realized on the presented binary:
 *   - verify that the Framework Version in the SDL header matches the running Framework one
 *   - verify that the Framework THL Signature Short in the SDL header matches the running Framework one
 */
static BERR_Code _srai_CheckTierKey(
    SUIF_TierKeyPackage *pKey)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned int otp_market_id_index;
    uint32_t *pMarketId_LE,*pMarketIdMask_LE;
    uint32_t otp_market_id, otp_market_id_lock, key_market_id,key_market_id_mask;
#if NEXUS_SECURITY_API_VERSION == 2
    NEXUS_OtpMspRead mspOtpRead;

    if(pKey->controllingParams.marketIdSelect == 0)
    {
        otp_market_id_index = NEXUS_OTPMSP_MARKET_ID0;
    }else if(pKey->controllingParams.marketIdSelect == 1)
    {
        otp_market_id_index = NEXUS_OTPMSP_MARKET_ID1;
    }
    else{
        BDBG_WRN(("%s - Tier Key's marketIdSelect %d is wrong.", BSTD_FUNCTION,pKey->controllingParams.marketIdSelect));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    rc = NEXUS_OtpMsp_Read( otp_market_id_index, &mspOtpRead );
    if (rc != BERR_SUCCESS) { goto end; }
    otp_market_id = mspOtpRead.data;
    otp_market_id_lock = mspOtpRead.valid;
#else
    NEXUS_ReadMspParms readMspParms;
    NEXUS_ReadMspIO readMspIO;
    unsigned i = 0;
    if(pKey->controllingParams.marketIdSelect == 0)
    {
        otp_market_id_index = NEXUS_OtpCmdMsp_eMarketId0;
    }else if(pKey->controllingParams.marketIdSelect == 1)
    {
        otp_market_id_index = NEXUS_OtpCmdMsp_eMarketId1;
    }
    else{
        BDBG_WRN(("%s - Tier Key's marketIdSelect %d is wrong.", BSTD_FUNCTION,pKey->controllingParams.marketIdSelect));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    BKNI_Memset( &readMspParms, 0, sizeof(readMspParms) );

    readMspParms.readMspEnum = otp_market_id_index;
    rc = NEXUS_Security_ReadMSP( &readMspParms, &readMspIO );
    otp_market_id = 0;
    otp_market_id_lock = 0;
    for( i = 0; i < readMspIO.mspDataSize; i++ )
    {
        otp_market_id = (otp_market_id << 8) | (readMspIO.mspDataBuf[i]);
        otp_market_id_lock = (otp_market_id_lock << 8) | (readMspIO.lockMspDataBuf[i]);
    }
#endif

    pMarketId_LE = (uint32_t *) pKey->controllingParams.marketId_LE;
    pMarketIdMask_LE = (uint32_t *) pKey->controllingParams.marketIdMask_LE;

    key_market_id = SUIF_Get32(pMarketId,LE);
    key_market_id_mask = SUIF_Get32(pMarketIdMask,LE);

    BDBG_MSG(("%s - marketID key-%x(mask %x) otp-%x(lock %x) (key marketID select %d)", BSTD_FUNCTION,key_market_id,key_market_id_mask,otp_market_id,otp_market_id_lock,pKey->controllingParams.marketIdSelect));

    if((otp_market_id&key_market_id_mask) != (key_market_id&key_market_id_mask))
    {
        BDBG_WRN(("%s: Key market id 0x%08x(mask 0x%08x) does not match chip OTP 0x%08x",
                  BSTD_FUNCTION, key_market_id,key_market_id_mask,otp_market_id));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
end:
    return rc;
}

static SUIF_TierKeyPackage *
_srai_GetTierKey(
    SUIF_TierKeyPackage *pTierKey_First)
{
    SUIF_TierKeyPackage *tierkey = NULL;
    SUIF_TierKeyPackage *currentkey = NULL;
    int i=0;

    if(pTierKey_First == NULL)
    {
        BDBG_ERR(("%s Invalid pTierKey_First %p",BSTD_FUNCTION,(void *)pTierKey_First));
        goto end;
    }

    for(currentkey=pTierKey_First,i=0; i< SUIF_TIER_KEY_PACKAGES_NUM; i++,currentkey++) /* 3 Tier keys availabe from pTierKey_First */
    {
        BDBG_MSG(("%s - Check Tier Key %d@%p", BSTD_FUNCTION,i,(void *)currentkey));
        if(_srai_CheckTierKey(currentkey) == BERR_SUCCESS)
        {
            tierkey = currentkey;
            break;
        }
    }
end:
    BDBG_MSG(("%s - found Tier Key (%d)@%p", BSTD_FUNCTION,i,(void *)tierkey));
    if(tierkey == NULL)
        BDBG_ERR(("%s NULL Tier Key, pTierKey_First %p",BSTD_FUNCTION,(void *)pTierKey_First));
    return tierkey;
}

static BERR_Code
_srai_Check_SUIF_Image(
    SUIF_PackageHeader  *header,
    uint32_t binarySize)
{
    BERR_Code rc = BERR_SUCCESS;

    SUIF_CommonHeader *pCommon = (SUIF_CommonHeader *)header;

    uint32_t *pDataSize_BE = (uint32_t *)(pCommon->dataSize_BE);
    uint32_t *pTextSize_BE = (uint32_t *)(pCommon->textSize_BE);
    uint32_t text_data_len = SUIF_Get32(pTextSize,BE) + SUIF_Get32(pDataSize,BE);

    uint32_t *pMeta_data_size_BE  = (uint32_t *)(pCommon->metadataSize_BE);
    uint32_t meta_data_size  = SUIF_Get32(pMeta_data_size,BE);

    uint16_t *pPkg_pad_size_BE = (uint16_t *)(pCommon->packagePadSize_BE);
    uint32_t pkg_pad_size    = SUIF_Get16(pPkg_pad_size,BE);

    SUIF_PackageFooter *footer = (SUIF_PackageFooter *)
                ((uint8_t *)header + sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size);

    SUIF_TierKeyPackage *pTierKey_First = header->tierKeyPackages;

    uint16_t *pTierKeyPackageSize_BE = (uint16_t *)(pCommon->tierKeyPackageSize_BE);
    uint16_t tierKeyPackageSize = SUIF_Get16(pTierKeyPackageSize,BE);

    if(tierKeyPackageSize != sizeof(SUIF_TierKeyPackage))
    {
        BDBG_ERR(("%s - tierKeyPackageSize (%u)wrong should be %u bytes", BSTD_FUNCTION, tierKeyPackageSize,(unsigned int)sizeof(SUIF_TierKeyPackage)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(binarySize != sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + sizeof(SUIF_PackageFooter))
    {
        BDBG_ERR(("%s - Image size (%u)wrong should be %u bytes(PKG header %u, Text+Data %u, mata Data %u, Pad %u, Footer %u)",
            BSTD_FUNCTION, binarySize,
            (unsigned int)sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + (unsigned int)sizeof(SUIF_PackageFooter),
            (unsigned int)sizeof(SUIF_PackageHeader),  text_data_len,  meta_data_size,  pkg_pad_size,  (unsigned int)sizeof(SUIF_PackageFooter)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(header->imageHeader.common.signingScheme == SUIF_SigningScheme_eTriple)
    {
        SUIF_TierKeyPackage *pTierKey;
        int tierkeyIndex=-1;

        pTierKey =  _srai_GetTierKey(pTierKey_First);

        tierkeyIndex = pTierKey-pTierKey_First;
        BDBG_MSG((" pTierKey index %d (%p,start %p pkg size %u)",
            tierkeyIndex,(void *)pTierKey,(void *)pTierKey_First,(unsigned int)sizeof(SUIF_TierKeyPackage)));

        if(tierkeyIndex == 0 || tierkeyIndex == 1)
        {
            /* test key, zero-fill production key and production signature */
            SUIF_TierKeyPackage *pTierKey_Production = &(header->tierKeyPackages[2]);
            uint8_t *text_signature_production = footer->textVerification.signature1;
            uint8_t *data_signature_production = footer->dataVerification.signature1;

            BDBG_MSG((" Test Tier Key, zero-fill production Tier Key and signatures"));

            BKNI_Memset( pTierKey_Production, 0, sizeof(SUIF_TierKeyPackage) );
            BKNI_Memset( text_signature_production, 0, SUIF_TIER_RSAKEY_SIG_SIZE_BYTES );
            BKNI_Memset( data_signature_production, 0, SUIF_TIER_RSAKEY_SIG_SIZE_BYTES );
        }
    }
out:
    return rc;
}

static bool
_srai_is_sdl_valid(
    BSAGElib_SDLHeader *pHeader,
    uint32_t binSize)
{
    bool rc = false;
    uint32_t sdlTHLSigShort;
    NEXUS_SageStatus status;
    SUIF_CommonHeader *pSUIFHeader = NULL;
    uint32_t magicNumber,*pMagicNumber_BE;

    if (NEXUS_Sage_GetStatus(&status) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: cannot retrieve nexus status", BSTD_FUNCTION));
        goto end;
    }

    if (pHeader == NULL) {
        BDBG_ERR(("%s: pHeader is NULL", BSTD_FUNCTION));
        goto end;
    }

    pSUIFHeader = (SUIF_CommonHeader *)pHeader;
    pMagicNumber_BE = (uint32_t *)(pSUIFHeader->magicNumber_BE);
    magicNumber = SUIF_Get32(pMagicNumber,BE);
    if(magicNumber == SUIF_MagicNumber_eSDL)
    {   /* this is SUIF format image */
        const SUIF_SDLSpecificHeader *pSUIFSDLHeader = SUIF_GetSdlHeaderFromPackageHeader((SUIF_PackageHeader *)pHeader);
        bool isAlphaSSF = (status.framework.version[2] == 0) && (status.framework.version[3] != 0);
        bool isAlphaSDK = (pSUIFSDLHeader->ssfVersion.revision == 0) && (pSUIFSDLHeader->ssfVersion.branch != 0);
        if (status.framework.version[0] < 4) {
            BDBG_ERR(("%s: ERROR: SAGE TA binary is a SUIF image which is not supported in SAGE 3.x", BSTD_FUNCTION));
            rc = false;
        }
        /* Check that any Alpha SSFs are matched with their SDLs and SDLs built from Alpha SDKs are matched with their SSF */
        else if (isAlphaSSF || isAlphaSDK)
        {
            rc = ((status.framework.version[0] == pSUIFSDLHeader->ssfVersion.major) &&
                  (status.framework.version[1] == pSUIFSDLHeader->ssfVersion.minor) &&
                  (status.framework.version[2] == pSUIFSDLHeader->ssfVersion.revision) &&
                  (status.framework.version[3] == pSUIFSDLHeader->ssfVersion.branch)    );
            if (rc != true) {
                if (isAlphaSSF) {
                    BDBG_ERR(("%s: The SAGE Alpha Framework does not match the SDK version (%u.%u.%u.%u) of the TA",
                              BSTD_FUNCTION, pSUIFSDLHeader->ssfVersion.major,
                              pSUIFSDLHeader->ssfVersion.minor,
                              pSUIFSDLHeader->ssfVersion.revision,
                              pSUIFSDLHeader->ssfVersion.branch));
                }
                else {
                    BDBG_ERR(("%s: The SAGE TA's Alpha SDK version (%u.%u.%u.%u) does not match the Framework",
                              BSTD_FUNCTION, pSUIFSDLHeader->ssfVersion.major,
                              pSUIFSDLHeader->ssfVersion.minor,
                              pSUIFSDLHeader->ssfVersion.revision,
                              pSUIFSDLHeader->ssfVersion.branch));
                }
                goto end;
            }
        }
        else {
            rc = true;
        }
        if(BERR_SUCCESS != _srai_Check_SUIF_Image((SUIF_PackageHeader  *)pHeader,binSize))
            rc = false;
    }else
    {
        BDBG_ASSERT(sizeof(pHeader->ucSsfVersion) == sizeof(status.framework.version));

        BKNI_Memcpy(&sdlTHLSigShort, pHeader->ucSsfThlShortSig, sizeof(sdlTHLSigShort));

        if (sdlTHLSigShort != 0) {
            rc = (sdlTHLSigShort == status.framework.THLShortSig);
            if (rc != true) {
                if ((pHeader->ucSsfVersion[0] | pHeader->ucSsfVersion[1] |
                     pHeader->ucSsfVersion[2] | pHeader->ucSsfVersion[3]) != 0) {
                    rc = (BKNI_Memcmp(pHeader->ucSsfVersion, status.framework.version, sizeof(status.framework.version)) == 0);
                    if (rc != true) {
                        BDBG_ERR(("%s: The SDL is compiled using SAGE version %u.%u.%u.%u SDK",
                                  BSTD_FUNCTION,
                                  pHeader->ucSsfVersion[0], pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2], pHeader->ucSsfVersion[3]));
                        BDBG_ERR(("%s: The SDL is not compiled from the same SDK as the running Framework",
                                  BSTD_FUNCTION));
                        BDBG_ERR(("%s: The SDL must be recompiled using SAGE version %u.%u.%u.%u SDK",
                                  BSTD_FUNCTION,
                                  status.framework.version[0], status.framework.version[1],
                                  status.framework.version[2], status.framework.version[3]));
                        goto end;
                    }
                }
                else {
                    BDBG_MSG(("%s: SSF version not found in SDL image, skipping SSF version check", BSTD_FUNCTION));
                }

                BDBG_ERR(("%s: The SDL THL Signature Short (0x%08x) differs from the one inside the loaded SAGE Framework (0x%08x)",
                          BSTD_FUNCTION, sdlTHLSigShort, status.framework.THLShortSig));
                BDBG_ERR(("%s: The SDL shall be linked against the same THL (Thin Layer) as the one inside the SAGE Framework",
                          BSTD_FUNCTION));
                goto end;
            }
        } else {
            bool isAlphaSSF = (status.framework.version[2] == 0) && (status.framework.version[3] != 0);
            bool isAlphaSDK = (pHeader->ucSsfVersion[2] == 0) && (pHeader->ucSsfVersion[3] != 0);
            BDBG_MSG(("%s: SDL THL zero signature indicates Load-Time-Resolution", BSTD_FUNCTION));
            if (status.framework.version[0] >= 4) {
                BDBG_ERR(("%s: ERROR: SAGE TA binary is not a SUIF image which is required for SAGE 4.x and above", BSTD_FUNCTION));
                rc = false;
            }
            /* Check that any Alpha SSFs are matched with their SDLs and SDLs built from Alpha SDKs are matched with their SSF */
            else if (isAlphaSSF || isAlphaSDK)
            {
                rc = (BKNI_Memcmp(pHeader->ucSsfVersion, status.framework.version, sizeof(status.framework.version)) == 0);
                if (rc != true) {
                    if (isAlphaSSF) {
                        BDBG_ERR(("%s: The SAGE Alpha Framework does not match the SDK version (%u.%u.%u.%u) of the TA",
                                  BSTD_FUNCTION,
                                  pHeader->ucSsfVersion[0], pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2], pHeader->ucSsfVersion[3]));
                    }
                    else {
                        BDBG_ERR(("%s: The SAGE TA's Alpha SDK version (%u.%u.%u.%u) does not match the Framework",
                                  BSTD_FUNCTION,
                                  pHeader->ucSsfVersion[0], pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2], pHeader->ucSsfVersion[3]));
                    }
                    goto end;
                }
            } else {
                rc = true;
            }
        }
    }
end:
    return rc;
}

#define CASE_PLATFORM_NAME_TO_STRING(name) case BSAGE_PLATFORM_ID_##name:  platform_name = #name; break

static char *
_srai_lookup_platform_name(uint32_t platformId)
{

  char *platform_name = NULL;
  switch (platformId) {
    /* NOTE: system and system_crit platforms are not SDL TAs */
    CASE_PLATFORM_NAME_TO_STRING(COMMONDRM);
    CASE_PLATFORM_NAME_TO_STRING(HDCP22);
    CASE_PLATFORM_NAME_TO_STRING(MANUFACTURING);
    CASE_PLATFORM_NAME_TO_STRING(UTILITY);
    CASE_PLATFORM_NAME_TO_STRING(PLAYREADY_30);
    CASE_PLATFORM_NAME_TO_STRING(ANTIROLLBACK);
    CASE_PLATFORM_NAME_TO_STRING(SECURE_VIDEO);
    CASE_PLATFORM_NAME_TO_STRING(SECURE_LOGGING);
    CASE_PLATFORM_NAME_TO_STRING(ADOBE_DRM);
    CASE_PLATFORM_NAME_TO_STRING(PLAYREADY_25);
    CASE_PLATFORM_NAME_TO_STRING(NETFLIX_ANNEXB);
    CASE_PLATFORM_NAME_TO_STRING(NETFLIX);
    CASE_PLATFORM_NAME_TO_STRING(WIDEVINE);
    CASE_PLATFORM_NAME_TO_STRING(PLAYBACK);
    CASE_PLATFORM_NAME_TO_STRING(DTCP_IP);
    CASE_PLATFORM_NAME_TO_STRING(MIRACAST);
    CASE_PLATFORM_NAME_TO_STRING(MARLIN);
    CASE_PLATFORM_NAME_TO_STRING(EDRM);
    CASE_PLATFORM_NAME_TO_STRING(BP3);
    default:
      break;
  }
  return platform_name;
}

BERR_Code SRAI_Platform_Install(uint32_t platformId,
                             uint8_t *binBuff,
                             uint32_t binSize)
{
    BERR_Code rc;
    BSAGElib_State state;
    BSAGElib_InOutContainer *container = NULL;
    BSAGElib_SDLHeader *pHeader = NULL;
    char *platform_name = _srai_lookup_platform_name(platformId);
    char revstr[9];
    uint8_t ucSdlVersion[4];                  /* SDL version */
    uint8_t ucSageSecureBootToolVersion[4];   /* Version of the secure boot tool used to signed the binary */
    SUIF_CommonHeader *pSUIFHeader = NULL;
    uint32_t magicNumber,*pMagicNumber_BE;

    BDBG_ENTER(SRAI_Platform_Install);

    /* Run defer/lazy init  */

    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (binBuff == NULL) {
        BDBG_ERR(("%s: The binBuff is NULL for TA 0x%X",
                  BSTD_FUNCTION, platformId));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (binSize < sizeof(BSAGElib_SDLHeader)) {
        BDBG_ERR(("%s: The binary size for TA 0x%X is less than the SDL header structure",
                  BSTD_FUNCTION, platformId));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    pHeader = (BSAGElib_SDLHeader *)binBuff;

    pSUIFHeader = (SUIF_CommonHeader *)binBuff;
    pMagicNumber_BE = (uint32_t *)(pSUIFHeader->magicNumber_BE);
    magicNumber = SUIF_Get32(pMagicNumber,BE);
    if(magicNumber == SUIF_MagicNumber_eSDL)
    {   /* this is SUIF format image */
        ucSdlVersion[0] = pSUIFHeader->imageVersion.major;
        ucSdlVersion[1] = pSUIFHeader->imageVersion.minor;
        ucSdlVersion[2] = pSUIFHeader->imageVersion.revision;
        ucSdlVersion[3] = pSUIFHeader->imageVersion.branch;
        ucSageSecureBootToolVersion[0] = pSUIFHeader->signingToolVersion.major;
        ucSageSecureBootToolVersion[1] = pSUIFHeader->signingToolVersion.minor;
        ucSageSecureBootToolVersion[2] = pSUIFHeader->signingToolVersion.revision;
        ucSageSecureBootToolVersion[3] = pSUIFHeader->signingToolVersion.branch;
    }else
    {   /* this is 3.x image */
        ucSdlVersion[0] = pHeader->ucSdlVersion[0];
        ucSdlVersion[1] = pHeader->ucSdlVersion[1];
        ucSdlVersion[2] = pHeader->ucSdlVersion[2];
        ucSdlVersion[3] = pHeader->ucSdlVersion[3];
        ucSageSecureBootToolVersion[0] = pHeader->ucSageSecureBootToolVersion[0];
        ucSageSecureBootToolVersion[1] = pHeader->ucSageSecureBootToolVersion[1];
        ucSageSecureBootToolVersion[2] = pHeader->ucSageSecureBootToolVersion[2];
        ucSageSecureBootToolVersion[3] = pHeader->ucSageSecureBootToolVersion[3];
    }

    BKNI_Snprintf(revstr, sizeof(revstr)-1, "%u", ucSdlVersion[3]);
    if ((ucSdlVersion[2] == 0) && (ucSdlVersion[3] != 0)) {
        BKNI_Snprintf(revstr, sizeof(revstr)-1, "Alpha%u", ucSdlVersion[3]);
    }
    if (platform_name == NULL) {
        BDBG_LOG(("SAGE TA: [0x%X] [Version=%u.%u.%u.%s, Signing Tool=%u.%u.%u.%u]",
                  platformId,
                  ucSdlVersion[0], ucSdlVersion[1],
                  ucSdlVersion[2], revstr,
                  ucSageSecureBootToolVersion[0], ucSageSecureBootToolVersion[1],
                  ucSageSecureBootToolVersion[2], ucSageSecureBootToolVersion[3]
                  ));
    }
    else {
        BDBG_LOG(("SAGE TA: %s [Version=%u.%u.%u.%s, Signing Tool=%u.%u.%u.%u]",
                  platform_name,
                  ucSdlVersion[0], ucSdlVersion[1],
                  ucSdlVersion[2], revstr,
                  ucSageSecureBootToolVersion[0], ucSageSecureBootToolVersion[1],
                  ucSageSecureBootToolVersion[2], ucSageSecureBootToolVersion[3]
                  ));
    }

    BDBG_MSG(("%s: System Platform %p ", BSTD_FUNCTION, (void *)_srai.system_platform ));

    if(_srai.system_platform == NULL){

        /* Open the platform first */
        rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_SYSTEM,
                                     &state,
                                     &_srai.system_platform);
        if (rc != BERR_SUCCESS) {
            goto end;
        }

        BDBG_MSG(("%s: System Platform %p ",BSTD_FUNCTION, (void *)_srai.system_platform));

        /* Check init state */
        if (state != BSAGElib_State_eInit) {
            /* Not yet initialized: send init command*/
            rc = SRAI_Platform_Init(_srai.system_platform, NULL);
            /* check for BSAGE_ERR_ALREADY_INITIALIZED to handle race conditions */
            if ((rc != BERR_SUCCESS) &&
                (rc != BSAGE_ERR_ALREADY_INITIALIZED)) {
                goto end;
            }
        }

        if(_srai.system_module == NULL){

            rc = SRAI_Module_Init(_srai.system_platform,
                                       System_ModuleId_eDynamicLoad,
                                       NULL,
                                       &_srai.system_module);
            if (rc != BERR_SUCCESS) {
                goto end;
            }
            BDBG_MSG(("%s: System Platform Module %p ", BSTD_FUNCTION, (void *)_srai.system_module ));

        }
    }


    BDBG_MSG(("%s: Anti-rollback Platform %p ", BSTD_FUNCTION, (void *)_srai.antirollback_platform ));
    if((platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK) && (_srai.antirollback_platform == NULL)){

        /* Open the platform first */
        rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_ANTIROLLBACK,
                                     &state,
                                     &_srai.antirollback_platform);
        if (rc != BERR_SUCCESS) {
            goto end;
        }

        BDBG_MSG(("%s: Anti-rollback Platform %p ",BSTD_FUNCTION, (void *)_srai.antirollback_platform));

        /* Check init state */
        if (state != BSAGElib_State_eInit) {
            /* Not yet initialized: send init command*/
            rc = SRAI_Platform_Init(_srai.antirollback_platform, NULL);
            /* check for BSAGE_ERR_ALREADY_INITIALIZED to handle race conditions */
            if ((rc != BERR_SUCCESS) &&
                (rc != BSAGE_ERR_ALREADY_INITIALIZED)) {
                goto end;
            }
        }

        if(_srai.antirollback_module == NULL){

            rc = SRAI_Module_Init(_srai.antirollback_platform,
                                  AntiRollback_ModuleId_eAntiRollback,
                                  NULL,
                                  &_srai.antirollback_module);
            if(rc == BSAGE_ERR_ALREADY_INITIALIZED)
            {
                /* This error can only occurs on SAGE 3.1.x where we do not need to send the RegisterTa command.  */
                rc = BERR_SUCCESS;
            }
            else if (rc != BERR_SUCCESS) {
                goto end;
            }
            BDBG_MSG(("%s: antirollback Platform Module %p ", BSTD_FUNCTION, (void *)_srai.antirollback_module ));
        }
    }


    if (!_srai_is_sdl_valid(pHeader,binSize)) {
        BDBG_ERR(("%s: Cannot install incompatible SDL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    container = SRAI_Container_Allocate();
    BDBG_ASSERT(container);

    /* Register the TA to the Anti-rollback TA (Skip when loading AR TA (although AR TA */
    /* is expected to be loaded with BSAGElib_Rai_Platform_Install())).                 */
    if((platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK) &&
       (_srai.antirollback_module != NULL))
    {
        container->basicIn[0] = platformId;

        BDBG_MSG(("%s: Registering TA (platform ID %u) to the Anti-rollback TA\n", BSTD_FUNCTION, platformId));
        rc = SRAI_Module_ProcessCommand(_srai.antirollback_module,
                                        AntiRollbackModule_CommandId_eRegisterTa,
                                        container);
        if(rc == BSAGE_ERR_MODULE_COMMAND_ID)
        {
            /* In SAGE 3.1.x, AR TA did not support RegisterTa command. Just ignore the request. */
            BDBG_MSG(("%s: Anti-rollback TA does not support the RegisterTa command. Ignoring the error.", BSTD_FUNCTION));
            rc = BERR_SUCCESS;
        }
        else if((rc != BERR_SUCCESS) || (container->basicOut[0] != BERR_SUCCESS))
        {
            BDBG_ERR(("%s: Cannot register TA to the Anti-rollback TA\n", BSTD_FUNCTION));
            rc = BERR_UNKNOWN;
            goto end;
        }
    }

    /* Load TA using the System TA */
    BKNI_Memset(container, 0, sizeof(*container));
    container->basicIn[0] = platformId;

    /* provide the SDL binary that's been pulled from FS */
    container->blocks[0].data.ptr = binBuff;
    container->blocks[0].len = binSize;

    if(magicNumber != SUIF_MagicNumber_eSDL)
    {
        /* this is not a SUIF format, check for triple signing
         * SUIF do not have triple signing yet */
        BSAGElib_SDLHeader *pHeader = (BSAGElib_SDLHeader *)binBuff;
        if(pHeader->ucSageImageSigningScheme == BSAGELIB_SDL_IMAGE_SIGNING_SCHEME_SINGLE){
            BDBG_MSG(("%s: Single signed image detected ^^^^^", BSTD_FUNCTION)); /* TODO: change to MSG */
            container->basicIn[1] = 0;
        }
        else if(pHeader->ucSageImageSigningScheme == BSAGELIB_SDL_IMAGE_SIGNING_SCHEME_TRIPLE){
            BDBG_MSG(("%s: Triple signed image detected ^^^^^", BSTD_FUNCTION)); /* TODO: change to MSG */
            container->basicIn[1] = 1;
        }
        else{
            BDBG_ERR(("%s: invalid Image Siging Scheme value (0x%02x) detected", BSTD_FUNCTION, pHeader->ucSageImageSigningScheme));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
    }

    rc = SRAI_Module_ProcessCommand(_srai.system_module,
                                    DynamicLoadModule_CommandId_eLoadSDL,
                                    container);

    BDBG_MSG(("%s Output Status %d \n",BSTD_FUNCTION,container->basicOut[0]));

    /* If RPC layer had no errors, then pass on the LoadSDL command response error, if applicable */
    if (rc == BERR_SUCCESS) {
        switch (container->basicOut[0]) {
        case BSAGE_ERR_SDL_ALREADY_LOADED:
        case BERR_SUCCESS:
            /* Leave rc as success */
            break;
        default:
            BDBG_ERR(("%s: SAGE failed to Load TA, (rc=0x%08x, %s)", BSTD_FUNCTION, container->basicOut[0], BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
            rc = container->basicOut[0];
            break;
        }
    }
    if (rc != BERR_SUCCESS) {
        goto end;
    }

    if(platformId != BSAGE_PLATFORM_ID_SECURE_LOGGING
    && platformId != BSAGE_PLATFORM_ID_SYSTEM
    && platformId != BSAGE_PLATFORM_ID_SYSTEM_CRIT
    && platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK)
    {
        NEXUS_Sage_SecureLog_Attach(platformId);
    }

end:
    if (container) {
        SRAI_Container_Free(container);
    }

    /* Keep refcount to make sure we close the */
    /* BSAGE_PLATFORM_ID_SYSTEM when all else are closed */
    _srai.platform_refcount++;

    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_Install);
    return rc;
}

/* Un-install a platform.
* This API is used to Un-install the Platform/TA that has been installed by SRAI_Platform_Install API */
BERR_Code SRAI_Platform_UnInstall(uint32_t platformId )
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container;
    BDBG_ENTER(SRAI_Platform_UnInstall);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto leave;
    }

    if((_srai.system_module == NULL) || (_srai.system_platform == NULL)){
        rc = BERR_INVALID_PARAMETER;
        goto leave;
    }

    if(platformId != BSAGE_PLATFORM_ID_SECURE_LOGGING
    && platformId != BSAGE_PLATFORM_ID_SYSTEM
    && platformId != BSAGE_PLATFORM_ID_SYSTEM_CRIT
    && platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK)
    {
        NEXUS_Sage_SecureLog_Detach(platformId);
    }

    container = SRAI_Container_Allocate();
    BDBG_ASSERT(container);

    container->basicIn[0] = platformId;

    BDBG_MSG(("%s: System Platform %p ", BSTD_FUNCTION, (void *)_srai.system_platform));
    BDBG_MSG(("%s: System Platform Module %p ", BSTD_FUNCTION, (void *)_srai.system_module));

    rc = SRAI_Module_ProcessCommand(_srai.system_module,
                                    DynamicLoadModule_CommandId_eUnLoadSDL,
                                    container);

    BDBG_MSG(("%s Output Status %d \n",BSTD_FUNCTION,container->basicOut[0]));
    if (container) {
        SRAI_Container_Free(container);
    }

leave:
    if (--_srai.platform_refcount == 0)
    {
        // We are done, cleanup
        if (_srai.system_module)
        {
            SRAI_Module_Uninit(_srai.system_module);
        }
        if (_srai.system_platform)
        {
            SRAI_Platform_Close(_srai.system_platform);
        }

      if(_srai.antirollback_module != NULL)
      {
          SRAI_Module_Uninit(_srai.antirollback_module);
          _srai.antirollback_module = NULL;
      }

      if(_srai.antirollback_platform != NULL)
      {
          SRAI_Platform_Close(_srai.antirollback_platform);
          _srai.antirollback_platform = NULL;
      }
    }
    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_UnInstall);
    return rc;
}

BERR_Code SRAI_Platform_EnableCallbacks(SRAI_PlatformHandle platform,
                                        SRAI_RequestRecvCallback callback)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    BSAGElib_InOutContainer *callbackReqContainer = NULL;
    BSAGElib_RpcMessage *message = NULL;
    NEXUS_SageCommand command;

    BDBG_ENTER(SRAI_Platform_EnableCallbacks);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (callback == NULL) {
        BDBG_ERR(("%s: missing callback pointer", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    callbackReqContainer = SRAI_Container_Allocate();
    if (callbackReqContainer == NULL) {
        BDBG_ERR(("%s: cannot allocate container for callback requests", BSTD_FUNCTION));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL) {
        BDBG_ERR(("%s: cannot allocate container", BSTD_FUNCTION));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }

    message = (BSAGElib_RpcMessage *)SRAI_Memory_Allocate(sizeof(*message), SRAI_MemoryType_Shared);
    if (message == NULL) {
        BDBG_ERR(("%s: cannot allocate Rpc message", BSTD_FUNCTION));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }
    BKNI_Memset(message, 0, sizeof(*message));

    container->blocks[0].data.ptr = (uint8_t *)message;
    container->blocks[0].len = sizeof(*message);
    container->blocks[1].data.ptr = (uint8_t *)callbackReqContainer;
    container->blocks[1].len = sizeof(*callbackReqContainer);

    if (!_srai_platform_acquire(platform, 0)) {
        BDBG_ERR(("%s: cannot acquire the platform", BSTD_FUNCTION));
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    /* setup ModuleProcessCommand command */
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformEnableCallbacks;
    command.platformId = platform->id;
    command.moduleId = 0;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("PlatformEnableCallbacks", &command);

    rc = _srai_nexus_sage_command(&platform->sync, &command, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: nexus_sage_command failure %x '%s'",
                  BSTD_FUNCTION, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        goto end_locked;
    }

    /* Success */
    platform->callbacks.requestRecvCallback = callback;
    platform->callbacks.container = callbackReqContainer;
    container = NULL;
    platform->callbacks.message = message;
    message = NULL;

end_locked:
    BKNI_ReleaseMutex(platform->sync.mutex);
end:
    if (container) {
        SRAI_Container_Free(container);
    }
    if (message) {
        SRAI_Memory_Free((uint8_t *)message);
    }

    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_EnableCallbacks);
    return rc;
}
#endif

/* Open a platform.
 * A platform can be opened from multiple applications at a time,
 * thus initialization might already have been done.
 * When opening the platform, platform current state is returned.
 * If returned state is BSAGElib_State_eUninit, platform needs to be initialized. */
BERR_Code SRAI_Platform_Open(uint32_t platformId,
                             BSAGElib_State *state /* [out] */,
                             SRAI_PlatformHandle *pPlatform /* [out] */)
{
    BSAGElib_InOutContainer *container = NULL;
    SRAI_PlatformHandle platform = NULL;
    NEXUS_SageCommand command;
    BERR_Code rc;

    BDBG_ENTER(SRAI_Platform_Open);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto leave;
    }

    BDBG_MSG(("%s: Platform %x ", BSTD_FUNCTION, platformId));

    if (!state || !pPlatform) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: state=%p, pPlatform=%p are mandatory parameters",
                  BSTD_FUNCTION, (void *)state, (void *)pPlatform));
        goto leave;
    }

    container = SRAI_Container_Allocate();
    if (!container) {
        BDBG_ERR(("%s: cannot allocate container", BSTD_FUNCTION));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }

    platform = BKNI_Malloc(sizeof(*platform));
    if (!platform) {
        BDBG_ERR(("%s: cannot allocate platform context", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto end;
    }
    BKNI_Memset(platform, 0, sizeof(*platform));
    platform->id = platformId;

    if (_srai_sync_init(&platform->sync, false)) {
        BDBG_ERR(("%s: cannot init platform sync", BSTD_FUNCTION));
        rc = BERR_OS_ERROR;
        goto end;
    }

    /* setup PlatformOpen command */
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformOpen;
    command.platformId = platformId;
    command.moduleId = 0;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("PlatformOpen", &command);

    /* run PlatformOpen command synchronously */
    rc = _srai_nexus_sage_command(&platform->sync, &command, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: nexus_sage_command failure %x '%s'",
                  BSTD_FUNCTION, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        goto end;
    }

    /* Platform is successfuly opened and context is initialized.
     * Make it available to public API */
    _srai_platform_push(platform);

    /* By internal convention with SAGE-side, state is backhauled using .basicOut[0] */
    *state = (BSAGElib_State)container->basicOut[0];

end:
    if (rc != BERR_SUCCESS && platform) {
        _srai_platform_cleanup(platform);
        platform = NULL;
    }
    if (container) {
        SRAI_Container_Free(container);
    }
    /* save resulting handle */
    *pPlatform = platform;
leave:
    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_Open);
    return rc;
}

/* Close an opened platform.
 * If a platform is not opened anymore, it is automatically uninitialized. */
void SRAI_Platform_Close(SRAI_PlatformHandle platform)
{
    NEXUS_SageCommand command;

    BDBG_ENTER(SRAI_Platform_Close);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        goto end;
    }

    if (!platform) {
        BDBG_ERR(("%s: platform=NULL is not valid", BSTD_FUNCTION));
        goto end;
    }

    /* Acquire and pop out the platform. */
    if (!_srai_platform_acquire(platform, 1)) {
        BDBG_ERR(("%s: cannot acquire/pop the platform", BSTD_FUNCTION));
        goto end;
    }
    /* from here, platform cannot be used any more by public API */

    /* setup PlatformClose command */
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformClose;
    command.platformId = platform->id;
    command.moduleId = 0;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("PlatformClose", &command);

    /* run PlatformClose command synchronously */
    _srai_nexus_sage_command(&platform->sync, &command, NULL);

    BKNI_ReleaseMutex(platform->sync.mutex);

    _srai_platform_cleanup(platform);

end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_Close);
}

/* Initialize a platform.
 * To be use if SRAI_Platform_Open() returned state is BSAGElib_State_eUninit.
 * Initialization parameters are backhauled using a SAGE In/Out container. */
BERR_Code SRAI_Platform_Init(SRAI_PlatformHandle platform,
                             BSAGElib_InOutContainer *container /* [in/out] */)
{
    NEXUS_SageCommand command;
    BERR_Code rc;

    BDBG_ENTER(SRAI_Platform_Init);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (!container) {
        BDBG_WRN(("%s: container=NULL", BSTD_FUNCTION));
    }

    if (!_srai_platform_acquire(platform, 0)) {
        BDBG_ERR(("%s: cannot acquire the platform", BSTD_FUNCTION));
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    /* setup PlatformInit command */
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformInit;
    command.platformId = platform->id;
    command.moduleId = 0;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("PlatformInit", &command);

    /* run PlatformInit command synchronously */
    rc = _srai_nexus_sage_command(&platform->sync, &command, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: nexus_sage_command failure %x '%s'",
                  BSTD_FUNCTION, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
    }

    BKNI_ReleaseMutex(platform->sync.mutex);

end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Platform_Init);
    return rc;
}

/* Initialize a module instance. This module identified by moduleId has to be
 * available within the platform associated with given PlatformHandle.
 * Initialization parameters are backhauled using a SAGE In/Out container. */
BERR_Code SRAI_Module_Init_Ext(SRAI_PlatformHandle platform,
                               uint32_t moduleId,
                               BSAGElib_InOutContainer *container /* [in/out] */,
                               SRAI_ModuleHandle *pModule /* [out] */,
                               SRAI_Module_InitSettings *pSettings /* [in] - optional */)
{
    NEXUS_SageCommand command;
    BERR_Code rc;
    SRAI_ModuleHandle module = NULL;

    BDBG_ENTER(SRAI_Module_Init_Ext);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto leave;
    }

    if (!pModule) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: pModule=NULL is invalid", BSTD_FUNCTION));
        goto leave;
    }

    if (!container) {
        BDBG_WRN(("%s: container=NULL", BSTD_FUNCTION));
    }

    module = BKNI_Malloc(sizeof(*module));
    if (!module) {
        BDBG_ERR(("%s: cannot allocate module", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto end;
    }
    BKNI_Memset(module, 0, sizeof(*module));
    module->platform = platform;
    module->id = moduleId;

    /* process optional settings */
    if (pSettings) {
        if (pSettings->indicationCallback) {
            module->indication_count = 0;
            module->indicationCallback = pSettings->indicationCallback;
            module->indicationCallbackArg = pSettings->indicationCallbackArg;
        }
    }

    if (_srai_sync_init(&module->sync, (module->indicationCallback != NULL))) {
        BDBG_ERR(("%s: cannot init module sync", BSTD_FUNCTION));
        rc = BERR_OS_ERROR;
        goto end;
    }

    if (!_srai_platform_acquire(platform, 0)) {
        BDBG_ERR(("%s: cannot acquire the platform", BSTD_FUNCTION));
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    /* setup ModuleInit command */
    command.systemCommandId = BSAGElib_SystemCommandId_eModuleInit;
    command.platformId = platform->id;
    command.moduleId = moduleId;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("ModuleInit", &command);

    /* run ModuleInit command synchronously */
    rc = _srai_nexus_sage_command(&module->sync, &command, container);
    BKNI_ReleaseMutex(platform->sync.mutex);

end:
    if (rc) {
        BDBG_ERR(("%s: nexus_sage_command failure %x '%s'",
                  BSTD_FUNCTION, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        if (module)
        {
            _srai_module_cleanup(module);
            module = NULL;
        }
    }
    else {
        /* success, insert module in modules list
         * !! do not push module under platform lock */
        _srai_module_push(module);
    }
    /* save resulting handle */
    *pModule = module;
leave:
    _srai_leave();
    BDBG_LEAVE(SRAI_Module_Init_Ext);
    return rc;
}

BERR_Code SRAI_Module_Init(SRAI_PlatformHandle platform,
                           uint32_t moduleId,
                           BSAGElib_InOutContainer *container /* [in/out] */,
                           SRAI_ModuleHandle *pModule /* [out] */)
{
    return SRAI_Module_Init_Ext(platform, moduleId, container, pModule, NULL);
}

/* Push/Pop logic:
 *
 * Ressources are elements of a root list.
 * Root list is controlled by a mutex.
 * Find/Pop/Push operations on root list are exclusive and require root list lock.
 *
 * Each ressource is a singleton, accessed once at a time,
 *   locked by a mutex during the whole operation.
 * Once pushed into root list, the ressource can be retrieved and used by public API
 *
 * While being used, a ressource can be poped out from the root list.
 * As Pop operation requires a lock, current operation will finish entirely,
 * before the *_pop request return.
 *
 * Note: lock of a multithreaded module instance will impact
 *       global latency while inside global lock during concurrent access
 *       (will lock find mecanism during module lock)
 */

/* Push/Insert platform in platforms list.
 * Only push platform when it is fully initialized i.e.
 * returned in success from Platform_Open. */
static void _srai_platform_push(SRAI_PlatformHandle platform)
{
    SRAI_P_Lock(); /* todo handle ret code */
    BLST_D_INSERT_HEAD(&_srai.platforms, platform, link);
    _srai.platformNum++;
    SRAI_P_Unlock();
    _srai_containers_cache_adjust_max();
}

/* Verify/Find platform handle in linked list */
static int _srai_platform_find(SRAI_PlatformHandle platform)
{
    SRAI_PlatformHandle pWalk;

    for (pWalk = BLST_D_FIRST(&_srai.platforms); pWalk; pWalk = BLST_D_NEXT(pWalk, link)) {
        if (pWalk == platform) {
            return 1;
        }
    }

    return 0;
}

/* Find and acquire/lock platform.
 * if pop is true, remove platform from linked list.
 * ==> platforms are only accessible from public API when in the linked list */
static int _srai_platform_acquire(SRAI_PlatformHandle platform, int pop)
{
    int found;

    /* always respect lock hierarchy: global lock THEN platform lock
     * !! never try to lock global lock when platform is locked */
    SRAI_P_Lock();
    found = _srai_platform_find(platform);
    if (found) {
        if (pop) {
            BLST_D_REMOVE(&_srai.platforms, platform, link);
            _srai.platformNum--;
        }
        BKNI_AcquireMutex(platform->sync.mutex); /* TODO handle ret code */
    }
    SRAI_P_Unlock();

    if (pop) {
        _srai_containers_cache_adjust_max();
    }

    return found;
}

/* Push/Insert module in modules list
 * only push module when it is fully initialized i.e.
 * returned in success from Module_Init.
 * once module is pushed, it is accessible from public API */
static void _srai_module_push(SRAI_ModuleHandle module)
{
   SRAI_P_Lock();

   BLST_D_INSERT_HEAD(&_srai.modules, module, link);
   _srai.moduleNum++;

   SRAI_P_Unlock();

   _srai_containers_cache_adjust_max();
}

/* Find/Verify module in linked list */
static int _srai_module_find(SRAI_ModuleHandle module)
{
    SRAI_ModuleHandle pWalk;

    for (pWalk = BLST_D_FIRST(&_srai.modules); pWalk; pWalk = BLST_D_NEXT(pWalk, link)) {
        if (pWalk == module) {
            return 1;
        }
    }

    return 0;
}

/* Pop/Remove module from linked list */
static void _srai_module_pop(SRAI_ModuleHandle module)
{
    BLST_D_REMOVE(&_srai.modules, module, link);
    _srai.moduleNum--;
}

/* Find and Acquire/Lock module.
 * If pop is true, remove module from linked list.
 * ==> platforms are only accessible from public API when in the linked list. */
static int _srai_module_acquire(SRAI_ModuleHandle module, int pop)
{
    int found;

    /* Always respect lock hierarchy: global lock THEN module lock
     * !! never try to lock global lock when module is locked */
    SRAI_P_Lock();
    found = _srai_module_find(module);
    if (found) {
        if (pop) {
            if (module->used) {
                BDBG_ERR(("%s: cannot pop the module (used)", BSTD_FUNCTION));
                found = 0;
            }
            else {
                _srai_module_pop(module);
                BKNI_AcquireMutex(module->sync.mutex);
            }
        }
        else {
            BKNI_AcquireMutex(module->sync.mutex); /* TODO handle ret code */
        }
    }
    SRAI_P_Unlock();

    if (pop) {
        _srai_containers_cache_adjust_max();
    }

    return found;
}

#if 0
/* this is an alternative way to ensure acquire without latency in global lock
 * but add random sleep (here 5ms) in collision case
 * note: could use WaitForEvent / ResetEvent / SetEvent
 *       will be more resource consuming but remove the random latency issue. */
static int _srai_module_acquire_smooth(SRAI_ModuleHandle module, int pop)
{
    int found = 0;
    int search = 1;
    int retry = 0;

    /* Always respect lock hierarchy: global lock THEN module lock
     * !! never try to lock global lock when module is locked */
    do {
        SRAI_P_Lock();
        if (search) {
            found = _srai_module_find(module);
            if (found && pop) {
                _srai_module_pop(module);
                search = 0;/* no need to search anymore */
            }
        }
        if (found) {
            switch (BKNI_TryAcquireMutex(module->sync.mutex)) {
            default:
                /* warn, err, whatever */
            case BERR_SUCCESS:
                break;
            case BERR_TIMEOUT:
                /* mutex is currently locked, retry later */
                retry = 1;
                break;
            }
        }
        SRAI_P_Unlock();
        if (retry) {
            BKNI_Sleep(5);
            retry = 0;
            continue;
        }
    } while (0);

    if (pop) {
        _srai_containers_cache_adjust_max();
    }

    return found;
}
#endif

/* Cleanup a module instance and all associated resources. */
static void _srai_module_cleanup(SRAI_ModuleHandle module)
{
    _srai_sync_cleanup(&module->sync);
    BKNI_Free(module);
}

/* Prepare and send a ModuleUinit command to SAGE for given module. */
static void _srai_module_uninit_sage_command(SRAI_ModuleHandle module)
{
    NEXUS_SageCommand command;

    /* setup ModuleUninit command */
    command.systemCommandId = BSAGElib_SystemCommandId_eModuleUninit;
    command.platformId = module->platform->id;
    command.moduleId = module->id;
    command.moduleCommandId = 0;

    SRAI_DUMP_COMMAND("ModuleUnInit", &command);

    /* run ModuleUnInit command synchronously */
    _srai_nexus_sage_command(&module->sync, &command, NULL);
}

/* Un-initialize a module instance. */
void SRAI_Module_Uninit(SRAI_ModuleHandle module)
{
    BDBG_ENTER(SRAI_Module_Uninit);

    /* Run defer/lazy init */
    if (_srai_enter()) {
        goto end;
    }

    /* Acquire/Lock and pop module out */
    if (!_srai_module_acquire(module, 1)) {
        BDBG_ERR(("%s: cannot acquire/pop the module", BSTD_FUNCTION));
        goto end;
    }
    /* Module is not accessible from public API anymore and is idling */

    _srai_module_uninit_sage_command(module);
    BKNI_ReleaseMutex(module->sync.mutex);
    _srai_module_cleanup(module);

end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Module_Uninit);
}

/* Process a command on the SAGE side of the module instance associated
 * with given ModuleHandle.
 * Command parameters are backhauled using a SAGE In/Out container. */
BERR_Code SRAI_Module_ProcessCommand(SRAI_ModuleHandle module,
                                      uint32_t commandId,
                                      BSAGElib_InOutContainer *container /* [in/out] */)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SageCommand command;

    BDBG_ENTER(SRAI_Module_ProcessCommand);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (!container) {
        /* Could be usefull for commands without parameters;
         * as void function(void); can be */
/*        BDBG_WRN(("%s: container=NULL", BSTD_FUNCTION));*/
    }

    if (!_srai_module_acquire(module, 0)) {
        BDBG_ERR(("%s: cannot acquire the module", BSTD_FUNCTION));
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    /* setup ModuleProcessCommand command */
    command.systemCommandId = BSAGElib_SystemCommandId_eModuleProcessCommand;
    command.platformId = module->platform->id;
    command.moduleId = module->id;
    command.moduleCommandId = commandId;

    SRAI_DUMP_COMMAND("ModuleProcessCommand", &command);

    /* run ModuleProcessCommand command synchronously */
    rc = _srai_nexus_sage_command(&module->sync, &command, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: nexus_sage_command failure %x '%s'",
                  BSTD_FUNCTION, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
    }

    /* release the module */
    BKNI_ReleaseMutex(module->sync.mutex);

end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Module_ProcessCommand);
    return rc;
}

static void _srai_management_cleanup(void)
{
    do {
        SRAI_CallbackItem *item = BLST_D_FIRST(&_srai.async.watchdog.callbacks);
        if (item) {
            BLST_D_REMOVE(&_srai.async.watchdog.callbacks, item, link);
            BKNI_Free(item);
            continue;
        }
    } while(0);
}

BERR_Code SRAI_Management_Register(SRAI_ManagementInterface *interface)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(SRAI_Management_Register);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (!interface) {
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (_srai.async.management.lock && _srai.async.management.lock_thread == pthread_self()) {
        BDBG_ERR(("SRAI_Management_* API is not available in watchdog callback."));
        rc = BERR_OS_ERROR;
        goto end;
    }

    if (interface->watchdog_callback) {
        SRAI_CallbackItem *item;
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            BDBG_ERR(("%s: cannot allocate watchdog callback context", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
        BKNI_Memset(item, 0, sizeof(*item));
        item->callback = interface->watchdog_callback;
        SRAI_P_LockManagement();
        BLST_D_INSERT_HEAD(&_srai.async.watchdog.callbacks, item, link);
        SRAI_P_UnlockManagement();
    }

    if (interface->ta_terminate_callback) {
        SRAI_TATerminateCallbackItem *item;
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            BDBG_ERR(("%s: cannot allocate ta_terminate callback context", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
        BKNI_Memset(item, 0, sizeof(*item));
        item->callback = interface->ta_terminate_callback;
        SRAI_P_LockManagement();
        BLST_D_INSERT_HEAD(&_srai.async.ta_terminate.callbacks, item, link);
        SRAI_P_UnlockManagement();
    }

end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Management_Register);
    return rc;
}

BERR_Code SRAI_Management_Unregister(SRAI_ManagementInterface *interface)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(SRAI_Management_Unregister);

    /* Run defer/lazy init  */
    if (_srai_enter()) {
        rc = BERR_NOT_INITIALIZED;
        goto end;
    }

    if (!interface) {
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (_srai.async.management.lock && _srai.async.management.lock_thread == pthread_self()) {
        BDBG_ERR(("SRAI_Management_* API is not available in watchdog callback."));
        rc = BERR_OS_ERROR;
        goto end;
    }

    if (interface->watchdog_callback) {
        SRAI_CallbackItem *item;
        for (item = BLST_D_FIRST(&_srai.async.watchdog.callbacks); item; item = BLST_D_NEXT(item, link)) {
            if (item->callback == interface->watchdog_callback) {
                SRAI_P_LockManagement();
                BLST_D_REMOVE(&_srai.async.watchdog.callbacks, item, link);
                SRAI_P_UnlockManagement();
                BKNI_Free(item);
                break;
            }
        }
    }

    if (interface->ta_terminate_callback) {
        SRAI_TATerminateCallbackItem *item;
        for (item = BLST_D_FIRST(&_srai.async.ta_terminate.callbacks); item; item = BLST_D_NEXT(item, link)) {
            if (item->callback == interface->ta_terminate_callback) {
                SRAI_P_LockManagement();
                BLST_D_REMOVE(&_srai.async.ta_terminate.callbacks, item, link);
                SRAI_P_UnlockManagement();
                BKNI_Free(item);
                break;
            }
        }
    }

 end:
    _srai_leave();
    BDBG_LEAVE(SRAI_Management_Unregister);
    return rc;
}

/* cleanup: run enter/leave logic to clean SRAI if nothing else is running */
void SRAI_Cleanup(void)
{
    BDBG_ENTER(SRAI_Cleanup);
    _srai_enter();
    _srai_leave();
    BDBG_LEAVE(SRAI_Cleanup);
}
