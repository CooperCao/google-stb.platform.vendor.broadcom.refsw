/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
***************************************************************************/

#include "nexus_types.h"
#include "nexus_base.h"
#include "nexus_base_os_types.h"
#include "nexus_platform.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_local_priv.h"
#if NEXUS_HAS_FILE
#include "nexus_file_init.h"
#endif

#include "nexus_platform_audio_log.h"
#include "nexus_platform_sage_log.h"
#include "blst_list.h"

#include <string.h>
/* stdio is needed to print a failure message when KNI is not started */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/mman.h>
#include <linux/kdev_t.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>
#include "nexus_driver_ioctl.h"
#include "nexus_core_init.h"
#include "priv/nexus_base_platform.h"
#include "nexus_map.h"
#include "nexus_platform_debug_log.h"
#include "priv/nexus_core_features.h"
#include "priv/nexus_core_module_local.h"
#if NEXUS_HAS_SURFACE
#include "priv/nexus_surface_module_local.h"
#endif

#if NEXUS_CONFIG_IMAGE
#include "priv/nexus_core_img_id.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "bxvd_image.h"
#endif
#if defined BDSP_RAAGA_SUPPORT
#include "bdsp_raaga_img.h"
#include "priv/nexus_audio_image_priv.h"
#endif
#if BDSP_ARM_AUDIO_SUPPORT
#include "bdsp_arm_img.h"
#endif
#include "nexus_platform_image.h"
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "bvce_image.h"
#endif
#if NEXUS_HAS_SAGE
#include "../src/nexus_sage_image.h"
#endif
#if NEXUS_HAS_SCM
extern void *SCM_IMAGE_Context;
extern BIMG_Interface SCM_IMAGE_Interface;
#endif
#if NEXUS_HAS_PICTURE_DECODER
#include "bsid_img.h"
#endif
#if NEXUS_USE_OTT_TRANSPORT
#include "priv/swxpt_image.h"
#endif
#if NEXUS_HAS_FRONTEND
#if NEXUS_HAS_FRONTEND_CTFE_IMG
#include "bhab_ctfe_img.h"
#if NEXUS_FRONTEND_7364
#include "bhab_7364_fw.h"
#endif
#if NEXUS_FRONTEND_3128
#include "bhab_3128_fw.h"
#endif
#if NEXUS_FRONTEND_3158
#include "bhab_3158_fw.h"
#endif
#if NEXUS_FRONTEND_7584
#include "bhab_7584_fw.h"
#endif
#if NEXUS_FRONTEND_7255
#include "bhab_7255_fw.h"
#endif
#if NEXUS_FRONTEND_3461
#include "bhab_3461_fw.h"
#endif
#if NEXUS_FRONTEND_7563
#include "bhab_7563_fw.h"
#endif
#endif
#if NEXUS_HAS_FRONTEND_SATFE_IMG
#include "bhab_satfe_img.h"
#if NEXUS_FRONTEND_7366
extern const void* BHAB_7366_IMG_Context;
#endif
#if NEXUS_FRONTEND_4538
extern const void* BHAB_4538_IMG_Context;
#endif
#if NEXUS_FRONTEND_45216
extern const void* BHAB_45216_IMG_Context;
#endif
#if NEXUS_FRONTEND_45308_FW
extern const void* BHAB_45308_IMG_Context;
#endif
#if NEXUS_FRONTEND_45316_FW
extern const void* BHAB_45316_IMG_Context;
#endif
#if NEXUS_FRONTEND_45402_FW
extern const void* BHAB_45402_IMG_Context;
#endif
#endif
#endif
#endif /* NEXUS_CONFIG_IMAGE */

BDBG_MODULE(nexus_platform_os);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */
#if BDBG_DEBUG_BUILD
#define NEXUS_P_DEBUG_PROXY_CALLBACKS 1
#ifndef NEXUS_PROXY_STATISTICS
#define NEXUS_PROXY_STATISTICS 0
#endif
#endif

/* define prototypes for init/uninit functions */
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) \
    BERR_Code nexus_proxy_##module##_init(int fd); \
    void nexus_proxy_##module##_uninit(void);
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE

static NEXUS_Error NEXUS_Platform_P_InitProxy(int fd);
static void NEXUS_Platform_P_UninitProxy(void);
static void NEXUS_Platform_P_SchedulerThread(void *scheduler_);
static void NEXUS_Platform_P_SchedulerSlaveThread(void *scheduler_);
static void NEXUS_Platform_P_UninitOS(void);

static const struct {
    const char *name;
    NEXUS_Error (*init)(int fd);
    void (*uninit)(void);
} g_nexus_proxy_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) {#module, nexus_proxy_##module##_init, nexus_proxy_##module##_uninit},
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
    {"proxy", NEXUS_Platform_P_InitProxy, NEXUS_Platform_P_UninitProxy} /* special entry for callback interface */
};
#define NEXUS_PLATFORM_P_NUM_DRIVERS  (sizeof(g_nexus_proxy_handlers)/sizeof(*g_nexus_proxy_handlers))

struct NEXUS_Platform_P_ProxyScheduler {
    NEXUS_ThreadHandle thread;
    NEXUS_ModulePriority priority;

    PROXY_NEXUS_Scheduler data;
    void *current_callback;

#if NEXUS_P_DEBUG_PROXY_CALLBACKS
#if NEXUS_PROXY_STATISTICS
    struct {
        unsigned callbackCount;
        unsigned runCallbacksCount;
        unsigned totalCallbackTime;
        unsigned maxCallbackTime;
        NEXUS_Callback maxCallback; /* function pointer for maxCallbackTime */
    } stats;
#endif
    struct {
        nexus_driver_callback_desc callback;
        NEXUS_Time startTime;
        bool valid;
    } monitor;
#endif
};

static struct NEXUS_Platform_P_State
{
    NEXUS_ModuleHandle module;
    struct {
        NEXUS_HeapHandle heap;
        void *addr;
        unsigned long length;
        bool dynamic;
        NEXUS_AddrType memoryMapType;
    } mmaps[NEXUS_MAX_HEAPS];
    int proxy_fd;
    bool stop;
    bool init; /* NEXUS_P_Uninit should be called because either NEXUS_P_Init or NEXUS_Platform_AuthenticatedJoin was called */
    bool platform_init; /* NEXUS_Platform_Init called */
    bool slave;
    struct NEXUS_Platform_P_ProxyScheduler schedulers[NEXUS_ModulePriority_eMax];
    int fd;
    bool opened[NEXUS_PLATFORM_P_NUM_DRIVERS];
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_Platform_P_DebugLog debugLog;
    struct {
        pthread_rwlock_t ioctl_lock;
        pthread_rwlock_t park_lock;
        pthread_rwlock_t callback_lock[NEXUS_ModulePriority_eMax];
    } stopCallbacks;
} NEXUS_Platform_P_State;

int proxy_ioctl_fd = -1;

#define NEXUS_MODULE_SELF NEXUS_Platform_P_State.module

NEXUS_Error NEXUS_GetPlatformConfigCapabilities_tagged_proxy( const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, NEXUS_PlatformConfigCapabilities *pCap, unsigned size );
NEXUS_Error NEXUS_Platform_InitStandby_proxy(const NEXUS_StandbySettings *pSettings);
NEXUS_Error NEXUS_Platform_Init_tagged_proxy(const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck);
NEXUS_Error NEXUS_Platform_GetMemory_tagged_proxy(NEXUS_PlatformMemory *memory, unsigned size);
void NEXUS_GetDefaultMemoryConfigurationSettings_tagged_proxy( NEXUS_MemoryConfigurationSettings *pSettings, size_t size );
void NEXUS_GetPlatformCapabilities_tagged_proxy( NEXUS_PlatformCapabilities *pCap, size_t size );
void NEXUS_Platform_GetDefaultSettings_tagged_proxy(NEXUS_PlatformSettings *pSettings, size_t size);
void NEXUS_Platform_Uninit_proxy(void);
void NEXUS_Platform_UninitStandby_proxy(void);

void *NEXUS_Platform_P_MapMemory( NEXUS_Addr offset, size_t length, NEXUS_AddrType type)
{
    void *addr;
    if (type!=NEXUS_AddrType_eCached) return NULL; /* only cached mmap allows in proxy */
    addr = mmap64(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, NEXUS_Platform_P_State.fd, offset);
    if (addr == MAP_FAILED) {
        BDBG_ERR(("mmap failed: offset " BDBG_UINT64_FMT ", size=%u, errno=%d", BDBG_UINT64_ARG(offset), (unsigned)length, errno));
        addr = NULL;
    }
    BDBG_MSG(("mmap  offset:" BDBG_UINT64_FMT " size:%u -> %p", BDBG_UINT64_ARG(offset), (unsigned)length, addr));
    return addr;
}

void NEXUS_Platform_P_UnmapMemory( void *addr, size_t length, NEXUS_AddrType memoryMapType)
{
    BSTD_UNUSED(memoryMapType);
    BDBG_MSG(("unmap: addr:%p size:%u", addr, (unsigned)length));
    munmap(addr, length);
    return;
}

static NEXUS_Error nexus_p_add_heap(unsigned i, NEXUS_HeapHandle heap, NEXUS_Addr base, unsigned length, NEXUS_MemoryType memoryType, bool dynamic, void *user_address)
{
    void *addr = NULL;
    NEXUS_AddrType memoryMapType = NEXUS_AddrType_eFake;

    if (user_address) {
        addr = user_address;
    }
    else if( (memoryType & (NEXUS_MEMORY_TYPE_NOT_MAPPED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)) == 0) {
        memoryMapType = ((memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED)==NEXUS_MEMORY_TYPE_APPLICATION_CACHED)?NEXUS_AddrType_eCached:NEXUS_AddrType_eFake;
        addr = nexus_p_map_memory(base, length, memoryMapType);
        if (!addr) return BERR_TRACE(BERR_OS_ERROR);
        NEXUS_P_AddMap(base, addr, memoryMapType, addr, memoryMapType, length);
    }

    NEXUS_Platform_P_State.mmaps[i].heap = heap;
    NEXUS_Platform_P_State.mmaps[i].addr = addr;
    NEXUS_Platform_P_State.mmaps[i].length = length;
    NEXUS_Platform_P_State.mmaps[i].dynamic = dynamic;
    NEXUS_Platform_P_State.mmaps[i].memoryMapType = memoryMapType;
    return 0;
}

static void nexus_p_remove_heap(unsigned i)
{
    if (NEXUS_Platform_P_State.mmaps[i].heap) {
        if(NEXUS_Platform_P_State.mmaps[i].addr) {
            nexus_p_unmap_memory(NEXUS_Platform_P_State.mmaps[i].addr, NEXUS_Platform_P_State.mmaps[i].length, NEXUS_Platform_P_State.mmaps[i].memoryMapType);
        }
        NEXUS_Platform_P_State.mmaps[i].heap = NULL;
    }
}

#if NEXUS_P_DEBUG_PROXY_CALLBACKS
BDBG_FILE_MODULE(nexus_proxy_statistics);
#endif

static NEXUS_TimerHandle g_NEXUS_CallbackMonitorTimer = NULL;

static void NEXUS_P_Platform_CallbackMonitor(void *context)
{
    unsigned i;
    unsigned timeout = 100;

    BSTD_UNUSED(context);
    g_NEXUS_CallbackMonitorTimer = NULL;

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct NEXUS_Platform_P_ProxyScheduler *scheduler = &NEXUS_Platform_P_State.schedulers[i];

        NEXUS_P_BaseCallback_Monitor(i, timeout);

#if NEXUS_P_DEBUG_PROXY_CALLBACKS
        if(scheduler->thread) {
            NEXUS_Time now;
            NEXUS_Time_Get(&now);

#if NEXUS_PROXY_STATISTICS
            BDBG_MODULE_MSG(nexus_proxy_statistics, ("scheduler[%d]: %d runCallbacks: %d callbacks, %d avg time, %d max time (function %p)",
                i,
                scheduler->stats.runCallbacksCount,
                scheduler->stats.callbackCount,
                scheduler->stats.callbackCount ? scheduler->stats.totalCallbackTime/scheduler->stats.callbackCount : 0,
                scheduler->stats.maxCallbackTime,
                scheduler->stats.maxCallback
                ));
            BKNI_Memset(&scheduler->stats, 0, sizeof(scheduler->stats));
#endif
            if (scheduler->monitor.valid) {
                unsigned duration;
                duration = NEXUS_Time_Diff(&now, &scheduler->monitor.startTime);
                if (duration > timeout) {
                    BDBG_ERR(("stuck callback %p(interface %p) for %u msec", (void *)(unsigned long)scheduler->monitor.callback.desc.callback, (void *)(unsigned long)scheduler->monitor.callback.interfaceHandle, duration));
                }
            }
        }
#endif /* #if NEXUS_P_DEBUG_PROXY_CALLBACKS */
    }

    g_NEXUS_CallbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, timeout, NEXUS_P_Platform_CallbackMonitor, NULL);
}


static NEXUS_Error
NEXUS_Platform_P_InitOS(void)
{
    NEXUS_Error rc;
    unsigned i;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    struct nexus_map_settings map_settings;
    unsigned valid_heaps;

    NEXUS_LockModule();

    rc = pthread_rwlock_init(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock, NULL);
    if (rc) {BERR_TRACE(rc); goto err_ioctl_lock;}
    rc = pthread_rwlock_init(&NEXUS_Platform_P_State.stopCallbacks.park_lock, NULL);
    if (rc) {BERR_TRACE(rc); goto err_park_lock;}

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        NEXUS_Base_Scheduler_Config config;
        void (*scheduler)(void *) = NEXUS_Platform_P_SchedulerThread;
        if(state->slave) {
            /* only create one */
            BDBG_CASSERT(0==NEXUS_ModulePriority_eIdle);
            if(i==NEXUS_ModulePriority_eIdle || i==NEXUS_ModulePriority_eInternal) {
                scheduler = NEXUS_Platform_P_SchedulerSlaveThread;
            } else {
                continue;
            }
        }

        BDBG_MSG((">CALLBACK[%u]", i));
        rc = pthread_rwlock_init(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[i], NULL);
        if (rc) {BERR_TRACE(rc); goto err_threads;}
        NEXUS_P_Base_GetSchedulerConfig(i, &config);
        state->schedulers[i].priority = i;
        state->schedulers[i].thread = NEXUS_Thread_Create(config.name, scheduler, &state->schedulers[i], config.pSettings);
        if(!state->schedulers[i].thread) { rc = BERR_TRACE(NEXUS_OS_ERROR); goto err_threads; }
    }
    g_NEXUS_CallbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, 100, NEXUS_P_Platform_CallbackMonitor, NULL);
    BDBG_ASSERT(g_NEXUS_CallbackMonitorTimer);

    BKNI_Memset(&state->clientConfig, 0, sizeof(state->clientConfig));
    NEXUS_Platform_GetClientConfiguration(&state->clientConfig);

    nexus_p_get_default_map_settings(&map_settings);
    map_settings.mmap = NEXUS_Platform_P_MapMemory;
    map_settings.munmap = NEXUS_Platform_P_UnmapMemory;
    rc = nexus_p_init_map(&map_settings);
    if (rc) {rc = BERR_TRACE(rc); goto err_init_map;}

    for (valid_heaps=0,i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_HeapHandle heap = state->clientConfig.heap[i];
        if (heap) {
            NEXUS_MemoryStatus status;
            rc = NEXUS_Platform_GetHeapStatus_driver(heap, &status);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_heap;}
            rc = nexus_p_add_heap(i, heap, status.offset, status.size, status.memoryType, false, NULL);
            if (rc) { rc = BERR_TRACE(BERR_OS_ERROR); goto err_mmap; }
            valid_heaps++;
        }
    }
    if(valid_heaps==0) {
        /* likely NEXUS_Platform_GetClientConfiguration call have failed and client was disconnected */
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_heap;
    }


    if(!state->slave) {
#if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT
        (void)NEXUS_Platform_P_InitWakeupDriver();
#endif
        (void)NEXUS_Platform_P_InitThermalMonitor();
    }

    NEXUS_UnlockModule();
    return NEXUS_SUCCESS;

err_mmap:
err_heap:
    NEXUS_Platform_P_UninitOS();
err_init_map:
err_threads:
    state->stop = true;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if (state->schedulers[i].thread) {
            NEXUS_Thread_Destroy(state->schedulers[i].thread);
            pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[i]);
        }
    }
    pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
err_park_lock:
    pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
err_ioctl_lock:
    NEXUS_UnlockModule();
    return rc;
}

static void
NEXUS_Platform_P_UninitOS(void)
{
    unsigned i;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    NEXUS_LockModule();
    if(!state->slave) {
        NEXUS_Platform_P_UninitThermalMonitor();
#if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT
        NEXUS_Platform_P_UninitWakeupDriver();
#endif
    }
    if(g_NEXUS_CallbackMonitorTimer) {
        NEXUS_CancelTimer(g_NEXUS_CallbackMonitorTimer);
        g_NEXUS_CallbackMonitorTimer=NULL;
    }

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        nexus_p_remove_heap(i);
    }
    nexus_p_uninit_map();

    state->stop = true;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if (state->schedulers[i].thread) {
            NEXUS_Thread_Destroy(state->schedulers[i].thread);
            pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[i]);
        }
    }
    pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
    pthread_rwlock_destroy(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
    NEXUS_UnlockModule();
    return;
}

static NEXUS_Error
NEXUS_Platform_P_OpenDriver(void)
{
    const char *devName;
    int rc, fd;
    struct stat st;
    bool create_node = true;

    devName = NEXUS_GetEnv("NEXUS_DEVICE_NODE");
    if (!devName) {
        devName = "/dev/nexus";
    }

#ifdef B_REFSW_ANDROID
    /* driver creates its nodes */
    BSTD_UNUSED(create_node);
    BSTD_UNUSED(st);
#else
    rc = stat(devName, &st);
    if(rc==0) {
        if(S_ISCHR(st.st_mode) && MAJOR(st.st_rdev) == NEXUS_PLATFORM_P_DRIVER_MAJOR && MINOR(st.st_rdev) == 0) {
            create_node = false;
        } else {
            BDBG_MSG(("Removing stale device node %s (%#x major=%u(%u) minor=%u(%u))", devName, (unsigned)st.st_mode, (unsigned)MAJOR(st.st_rdev), NEXUS_PLATFORM_P_DRIVER_MAJOR, (unsigned)MINOR(st.st_rdev), 0));
            /* coverity[toctou] - it's ok to use the filename after the stat() check */
            rc = unlink(devName);
            if(rc!=0) {
                BDBG_ERR(("unlink:'%s' failed", devName));
                return BERR_TRACE(NEXUS_OS_ERROR);
            }
        }
    }
    if(create_node) {
        BDBG_MSG(("Making device node %s (major=%u minor=%u)", devName,  NEXUS_PLATFORM_P_DRIVER_MAJOR, 0));
        /* coverity[toctou] - it's ok to use the filename after the stat() check */
        rc = mknod(devName, 0666 | S_IFCHR, makedev(NEXUS_PLATFORM_P_DRIVER_MAJOR, 0));
        if(rc!=0) {
            BDBG_ERR(("mknode:'%s' failed", devName));
            return BERR_TRACE(NEXUS_OS_ERROR);
        }
        /* allow non-root access */
        rc = chmod(devName, 0666);
        if (rc) BERR_TRACE(rc); /* keep going */
    }
#endif
    /* coverity[toctou] - it's ok to use the filename after the stat() check */
    fd = open(devName, O_RDWR);
    if(fd<0) {
        /* if client starts before server, it will fail here */
        BDBG_ERR(("can't access device '%s'", devName));
        return NEXUS_OS_ERROR; /* no BERR_TRACE */
    }
    rc = ioctl(fd, IOCTL_PROXY_NEXUS_Open, 0);
    if(rc!=0) {
        close(fd);
        return BERR_TRACE(NEXUS_OS_ERROR);
    }
    rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (rc) BERR_TRACE(rc); /* keep going */

#if !defined(NEXUS_CPU_ARM64)
    {
    PROXY_NEXUS_GetOsConfig os_cfg;
    rc = ioctl(fd, IOCTL_PROXY_NEXUS_GetOsConfig, &os_cfg);
    if(rc==0) {
        if(os_cfg.os_64bit) {
            g_NEXUS_P_CpuNotAccessibleRange.start = NULL;
            g_NEXUS_P_CpuNotAccessibleRange.length = 0;
        }
    }
    }
#endif

    NEXUS_Platform_P_State.fd = fd;
    NEXUS_Platform_P_State.proxy_fd = fd; /* redundant */

    /* open platform and proxy immediately (for slave, only proxy) */
    if (!NEXUS_Platform_P_State.slave) {
        rc = g_nexus_proxy_handlers[0].init(NEXUS_Platform_P_State.fd);
        if (rc) return BERR_TRACE(rc);
        NEXUS_Platform_P_State.opened[0] = true;
    }
    rc = g_nexus_proxy_handlers[NEXUS_PLATFORM_P_NUM_DRIVERS-1].init(NEXUS_Platform_P_State.fd);
    if (rc) return BERR_TRACE(rc);
    NEXUS_Platform_P_State.opened[NEXUS_PLATFORM_P_NUM_DRIVERS-1] = true;

    return NEXUS_SUCCESS;
}

static NEXUS_Error
NEXUS_Platform_P_InitOtherDrivers(void)
{
    unsigned i;
    for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS;i++) {
        if (!NEXUS_Platform_P_State.opened[i] && !g_nexus_proxy_handlers[i].init(NEXUS_Platform_P_State.fd)) {
            NEXUS_Platform_P_State.opened[i] = true;
        }
        /* else keep going. a subset of modules may be blocked. */
    }
    return NEXUS_SUCCESS;
}

static void
NEXUS_Platform_P_UninitDriver(void)
{
    unsigned i;
    for(i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS;i++) {
        if (NEXUS_Platform_P_State.opened[i]) {
            g_nexus_proxy_handlers[i].uninit();
            NEXUS_Platform_P_State.opened[i] = false;
        }
    }
    close(NEXUS_Platform_P_State.fd);
    NEXUS_Platform_P_State.fd = -1;
    return;
}

void NEXUS_Platform_P_SetEnv(const char *name)
{
    const char *value = NEXUS_GetEnv(name);
    if (value) {
        char buf[256];
        FILE *f = fopen("/proc/brcm/config", "w");
        if (!f) {BERR_TRACE(NEXUS_UNKNOWN); return;}
        BKNI_Snprintf(buf, sizeof(buf), "%s=%s", name, value);
        fputs(buf, f);
        fclose(f);
    }
}

/* bring up the proxy base and the driver.
NEXUS_P_Init is only called by the server, not the client. */
static NEXUS_Error NEXUS_P_Init(void)
{
    NEXUS_Error errCode;
    NEXUS_ModuleHandle proxy;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    char device_fd[8];
    unsigned flags;
    int rc;

    BDBG_ASSERT(state->init == false);
    BKNI_Memset(state, 0, sizeof(*state));
    state->fd = -1; /* unopened == -1, not 0 */

    errCode = NEXUS_Platform_P_Magnum_Init();
    if( errCode != BERR_SUCCESS)  { errCode = BERR_TRACE(errCode); goto err_magnum; }

    /* Now, proceed to boot the board. -- Always initialize base first */
    errCode = NEXUS_Base_Init(NULL);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_base; }

    NEXUS_Base_ExportEnvVariables();

    BDBG_MSG((">CORE_LOCAL"));
    errCode = NEXUS_CoreModule_LocalInit();
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_core_local; }

#if NEXUS_HAS_SURFACE
    BDBG_MSG((">SURFACE_LOCAL"));
    errCode = NEXUS_SurfaceModule_LocalInit();
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_surface_local; }
#endif


    BDBG_MSG((">MODULE"));
    proxy = NEXUS_Module_Create("proxy", NULL);
    if ( !proxy) { errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_proxy; }
    state->module = proxy;

    BDBG_MSG((">PLATFORM DRIVER"));
    errCode = NEXUS_Platform_P_OpenDriver();
    if ( errCode!=BERR_SUCCESS ) {
        BDBG_WRN(("Nexus may already be initialized by another process. Check driver log."));
        errCode = BERR_TRACE(errCode);
        goto err_driver;
    }

    BDBG_MSG((">DEBUG_LOG"));
    BDBG_ASSERT(state->proxy_fd >= 0);
    BKNI_Snprintf(device_fd, sizeof(device_fd), "%d", state->proxy_fd);
    flags = fcntl(state->proxy_fd, F_GETFD, 0);
    if (flags == (unsigned)-1) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
    }
    else {
        rc = fcntl(state->proxy_fd, F_SETFD, flags & ~FD_CLOEXEC);
        if (rc) BERR_TRACE(rc); /* keep going */
    }
    NEXUS_Platform_P_DebugLog_Init(&state->debugLog, device_fd);
    rc = fcntl(state->proxy_fd, F_SETFD, FD_CLOEXEC);
    if (rc) BERR_TRACE(rc); /* keep going */
    if(state->debugLog.logWriter || !strcmp(state->debugLog.fname, "disabled")) {
        const char *debug_log_size;

        PROXY_NEXUS_Log_Activate activate;
        activate.debug_log_size = state->debugLog.logSize;
        if(activate.debug_log_size>256) {
            activate.debug_log_size /= 4;
        }
        debug_log_size = NEXUS_GetEnv("kernel_debug_log_size");
        if(debug_log_size) {
            activate.debug_log_size = 2*NEXUS_atoi(debug_log_size);
        }
        if (activate.debug_log_size) {
            rc = ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_Log_Activate, &activate);
            if (rc) BERR_TRACE(rc); /* keep going */
        }
    }

    state->init = true;
    return 0;

err_driver:
    NEXUS_Module_Destroy(proxy);
    state->module = NULL;
err_proxy:
#if NEXUS_HAS_SURFACE
    NEXUS_SurfaceModule_LocalUninit();
err_surface_local:
#endif
    NEXUS_CoreModule_LocalUninit();
err_core_local:
    NEXUS_Base_Uninit();
err_base:
    NEXUS_Platform_P_Magnum_Uninit();
err_magnum:
    BDBG_ASSERT(errCode); /* if we've taken this path, it's only because of failure */
    return errCode;
}

/* NEXUS_P_Uninit is called by the server and the client. */
static void NEXUS_P_Uninit(void)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    if (!state->init) return;

    if (!state->slave) {
        int urc;
        BDBG_MSG(("<DEBUG_LOG"));
        urc = ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_Log_Deactivate, NULL);
        BSTD_UNUSED(urc);
        NEXUS_Platform_P_DebugLog_Uninit(&state->debugLog);
    }

    if (state->module) {
        BDBG_MSG(("<DRIVER"));
        NEXUS_Platform_P_UninitDriver();

        BDBG_MSG(("<MODULE"));
        NEXUS_Module_Destroy(state->module);

#if NEXUS_HAS_SURFACE
        BDBG_MSG(("<SURFACE_LOCAL"));
        NEXUS_SurfaceModule_LocalUninit();
#endif

        BDBG_MSG(("<CORE_LOCAL"));
        NEXUS_CoreModule_LocalUninit();

        BDBG_MSG(("<BASE"));
        NEXUS_Base_Uninit();

        BDBG_MSG(("<MAGNUM"));
        NEXUS_Platform_P_Magnum_Uninit();
    }

    memset(state, 0, sizeof(*state)); /* can't use BKNI_Memset here */
    return;
}

/* local copy of NEXUS_Platform_GetDefaultSettings, it initializes base and opens driver.
a partial bringup of the kernel driver is necessary to allow NEXUS_Platform_GetDefaultSettings to be
proxied so it can get defaults from Magnum. */
void
NEXUS_Platform_GetDefaultSettings_tagged(NEXUS_PlatformSettings *pSettings, size_t size)
{
    if (!NEXUS_Platform_P_State.init) {
        NEXUS_Error errCode = NEXUS_P_Init();
        if (errCode) {
            /* If NEXUS_P_Init fails here, it will likely fail again in NEXUS_Platform_Init and return an error */
            errCode = BERR_TRACE(errCode);
            /* app can't know that this function failed, so at least prevent access to uninitialized memory */
            memset(pSettings, 0, size);
            return;
        }
    }
    if(size!=sizeof(*pSettings)) {
        BDBG_ERR(("NEXUS_Platform_GetDefaultSettings:size mismatch %u != %u", (unsigned)sizeof(*pSettings), (unsigned)size));
        BKNI_Memset(pSettings, 0, size);
        return;
    }

    /* call the proxy version */
    NEXUS_Platform_GetDefaultSettings_tagged_proxy(pSettings, size);
#if NEXUS_HAS_SAGE
    NEXUS_SageImage_SetImageExists_priv(&pSettings->sageModuleSettings);
#endif
#if NEXUS_HAS_FILE
    NEXUS_FileModule_GetDefaultSettings(&pSettings->fileModuleSettings);
#endif
    #if NEXUS_HAS_AUDIO && NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if ( NEXUS_GetEnv("audio_logs_enabled") )
    {
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eUartMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eDramMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eTargetPrint].enabled = true;
    }
    else
    {
        if ( NEXUS_GetEnv("audio_uart_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eUartMessage].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_debug_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eDramMessage].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_core_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_target_print_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eTargetPrint].enabled = true;
        }
    }
    #endif
}

#if NEXUS_CONFIG_IMAGE

static NEXUS_ThreadHandle gImageThread=NULL;
static void NEXUS_Platform_P_ImageThread(void *context)
{
    BSTD_UNUSED(context);
    BDBG_MSG(("Image Thread"));
    BDBG_ASSERT(NEXUS_Platform_P_State.proxy_fd >= 0);
    Nexus_Platform_P_Image_Handler(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_Image);
    return;
}

static NEXUS_Error NEXUS_Platform_P_InitImage(const NEXUS_PlatformImgInterface *pImgInterface)
{
    NEXUS_ThreadSettings threadSettings;
    NEXUS_Error rc;
    BDBG_MSG(("Image init"));

    rc = Nexus_Platform_P_Image_Init(pImgInterface);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#ifndef NEXUS_SECURITY_VIDEO_VERIFICATION_LEGACY_65NM
    #if NEXUS_HAS_VIDEO_DECODER
    /* BXVD_IMAGE_Interface is neither required nor defined when region verification is enabled */
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BXVD_IMAGE_Interface, BXVD_IMAGE_Context, NEXUS_CORE_IMG_ID_XVD);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    #endif
#endif
#if BDSP_RAAGA_SUPPORT
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BDSP_IMG_Interface, BDSP_IMG_Context, NEXUS_CORE_IMG_ID_AUDIO_DSP);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    rc = Nexus_Platform_P_Image_Interfaces_Register(&NEXUS_AUDIO_IMG_Interface, NEXUS_AUDIO_IMG_Context, NEXUS_CORE_IMG_ID_AUDIO_PAK);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif
#if BDSP_ARM_AUDIO_SUPPORT
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BDSP_ARM_IMG_Interface, BDSP_ARM_IMG_Context, NEXUS_CORE_IMG_ID_SOFT_AUDIO);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BVCE_IMAGE_Interface, BVCE_IMAGE_Context, NEXUS_CORE_IMG_ID_VCE);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif

#if NEXUS_HAS_SAGE
    rc = Nexus_Platform_P_Image_Interfaces_Register(&SAGE_IMAGE_Interface, SAGE_IMAGE_Context, NEXUS_CORE_IMG_ID_SAGE);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif

#if NEXUS_HAS_PICTURE_DECODER
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BSID_ImageInterface, BSID_ImageContext, NEXUS_CORE_IMG_ID_SID);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif

#if NEXUS_HAS_SCM
    rc = Nexus_Platform_P_Image_Interfaces_Register(&SCM_IMAGE_Interface, SCM_IMAGE_Context, NEXUS_CORE_IMG_ID_SAGE);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif

#if NEXUS_HAS_FRONTEND
#if NEXUS_FRONTEND_7364
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm7364_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_7364); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_3128
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm3128_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_3128); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_3158
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm3158_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_3158); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_7584
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm7584_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_7584); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_7255
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm7255_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_7255); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_3461
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm3461_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_3461); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_7563
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm7563_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_7563); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_7366
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_7366_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_7366); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_4538
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_4538_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_4538); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_45216
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_45216_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_45216); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_45308_FW
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_45308_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_45308); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_45316_FW
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_45316_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_45316); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#if NEXUS_FRONTEND_45402_FW
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_45402_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_45402); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
#endif
#if NEXUS_USE_OTT_TRANSPORT
    rc = Nexus_Platform_P_Image_Interfaces_Register(&SWXPT_IMAGE_Interface, SWXPT_IMAGE_Context, NEXUS_CORE_IMG_ID_SWXPT);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
#endif


    NEXUS_Thread_GetDefaultSettings(&threadSettings);
    gImageThread = NEXUS_Thread_Create("Nexus_Image",NEXUS_Platform_P_ImageThread,NULL,&threadSettings);
    if(gImageThread == NULL)
    {
        BDBG_ERR(("Failed to create image thread"));
        return BERR_TRACE(NEXUS_OS_ERROR);
    }
    BDBG_MSG(("Created Image Thread"));
    return NEXUS_SUCCESS;

}
static void NEXUS_Platform_P_UninitImage(void)
{
    if (gImageThread) {
        BDBG_MSG(("Image Uninit"));
        Nexus_Platform_P_Image_Stop(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_Image);
        NEXUS_Thread_Destroy(gImageThread);
        Nexus_Platform_P_Image_Shutdown();
        gImageThread=NULL;
    }
}
#endif


/* local copy of NEXUS_Platform_Init, it initializes base, opens driver (if not already done by NEXUS_Platform_GetDefaultSettings, then calls driver NEXUS_Platform_Init, then initializes local modules */
NEXUS_Error
NEXUS_Platform_Init_tagged(const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck)
{
    NEXUS_Error errCode;
    NEXUS_PlatformSettings *pLocalSettings = NULL;

    if (NEXUS_Platform_P_State.platform_init) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (!NEXUS_Platform_P_State.init) {
        errCode = NEXUS_P_Init();
        if (errCode) return BERR_TRACE(errCode);
    }

#if !NEXUS_PLATFORM_NON_NUMERIC
    if (platformCheck != NEXUS_PLATFORM) {
        /* This code also ensures that NEXUS_PLATFORM is numeric, not alpha-numeric */
        BDBG_ERR(("NEXUS_Platform_Init failed with platform mismatch (nexus=%d, caller=%d). The application must be recompiled for this platform.",
            NEXUS_PLATFORM, platformCheck));
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_platform_init;
    }
#endif

    if (versionCheck != NEXUS_P_GET_VERSION(NEXUS_PLATFORM)) {
        BDBG_ERR(("NEXUS_Platform_Init failed with version mismatch (nexus=%d.%d, caller=%d.%d). Please recompile application and/or nexus.",
            NEXUS_P_GET_VERSION(NEXUS_PLATFORM) / NEXUS_PLATFORM_VERSION_UNITS,
            NEXUS_P_GET_VERSION(NEXUS_PLATFORM) % NEXUS_PLATFORM_VERSION_UNITS,
            versionCheck / NEXUS_PLATFORM_VERSION_UNITS,
            versionCheck % NEXUS_PLATFORM_VERSION_UNITS));
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_platform_init;
    }

    if (structSizeCheck != NEXUS_P_GET_STRUCT_SIZES()) {
        BDBG_ERR(("NEXUS_Platform failed with struct size mismatch (nexus=%d, caller=%d). Please recompile application and/or nexus.",
            (unsigned)NEXUS_P_GET_STRUCT_SIZES(), structSizeCheck));
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_platform_init;
    }

    if (!pSettings) {
        pLocalSettings = BKNI_Malloc(sizeof(*pLocalSettings));
        if (!pLocalSettings) {
            errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_platform_init;
        }
        NEXUS_Platform_GetDefaultSettings(pLocalSettings);
        pSettings = pLocalSettings;
    }

#if NEXUS_CONFIG_IMAGE
    BDBG_MSG(("Image"));
    errCode = NEXUS_Platform_P_InitImage(&pSettings->imgInterface);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_platform_init; }
#endif
    NEXUS_P_PrintEnv(BDBG_STRING("proxy: "));

    BDBG_MSG((">PLATFORM"));
    errCode = NEXUS_Platform_Init_tagged_proxy(pSettings, pMemConfig, platformCheck, versionCheck, structSizeCheck);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_image_init; }

    BDBG_MSG((">ALL OTHER DRIVERS"));
    errCode = NEXUS_Platform_P_InitOtherDrivers( );
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_driver; }

    BDBG_MSG((">OS"));
    errCode = NEXUS_Platform_P_InitOS();
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_os; }

    errCode = NEXUS_Platform_P_ModulesInit(pSettings);
    if(errCode!=NEXUS_SUCCESS) {errCode=BERR_TRACE(errCode);goto err;}

    errCode = NEXUS_Platform_P_InitAudioLog();
    if(errCode!=NEXUS_SUCCESS) {NEXUS_Platform_Uninit(); return BERR_TRACE(errCode);}

    errCode = NEXUS_Platform_P_InitSageLog();
    if(errCode!=NEXUS_SUCCESS) {NEXUS_Platform_Uninit(); return BERR_TRACE(errCode);}

    errCode = NEXUS_Platform_P_DropPrivilege(pSettings);
    BDBG_ASSERT(!errCode); /* failure will terminate */

    /* Success */
    NEXUS_Platform_P_State.platform_init = true;
    BDBG_MSG((">DONE"));
    if (pLocalSettings) {
        BKNI_Free(pLocalSettings);
    }
    return NEXUS_SUCCESS;

err:
    NEXUS_Platform_P_UninitOS();
    NEXUS_Platform_P_State.module = NULL;
err_os:
    NEXUS_Platform_P_UninitDriver();
err_driver:
    NEXUS_Platform_Uninit_proxy();
err_image_init:
#if NEXUS_CONFIG_IMAGE
    NEXUS_Platform_P_UninitImage();
#endif
    if (pLocalSettings) {
        BKNI_Free(pLocalSettings);
    }
err_platform_init:
    NEXUS_P_Uninit();
    return errCode;
}

void
NEXUS_Platform_Uninit(void)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    if (!state->platform_init) {
        /* even if NEXUS_Platform_Init was not called, we may need to clean up NEXUS_Platform_GetDefaultSettings. */
        goto local_uninit;
    }
    state->platform_init = false;

    NEXUS_Platform_P_UninitAudioLog();
    NEXUS_Platform_P_UninitSageLog();
    NEXUS_Platform_P_ModulesUninit();
#if NEXUS_CONFIG_IMAGE
    NEXUS_Platform_P_UninitImage();
#endif

    if(!state->slave) {
        BDBG_MSG(("<OS"));
        NEXUS_Platform_P_UninitOS();

        BDBG_MSG(("<PLATFORM_UNINIT"));
        NEXUS_Platform_Uninit_proxy();
    } else {
        BDBG_MSG(("<OS"));
        NEXUS_Platform_P_UninitOS();

        /* tell driver that shutdown is clean */
        (void)ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_Platform_Uninit, NULL);
    }
local_uninit:
    BDBG_MSG(("<NEXUS UNINIT"));
    NEXUS_P_Uninit();
    return ;
}

/* local copy */
NEXUS_Error NEXUS_Platform_InitStandby( const NEXUS_StandbySettings *pSettings )
{
    NEXUS_Error errCode;

    if (!NEXUS_Platform_P_State.init) {
        errCode = NEXUS_P_Init();
        if (errCode) return BERR_TRACE(errCode);
    }

    BDBG_MSG((">PLATFORM"));
    errCode = NEXUS_Platform_InitStandby_proxy(pSettings);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err; }

    return 0;

err:
    NEXUS_P_Uninit();
    return errCode;
}

void NEXUS_Platform_UninitStandby(void)
{
    BDBG_MSG(("<PLATFORM"));
    NEXUS_Platform_UninitStandby_proxy();
    NEXUS_P_Uninit();
}

void NEXUS_GetDefaultMemoryConfigurationSettings_tagged( NEXUS_MemoryConfigurationSettings *pSettings, size_t size )
{
    if (!NEXUS_Platform_P_State.init) {
        if (NEXUS_P_Init()) return;
    }
    NEXUS_GetDefaultMemoryConfigurationSettings_tagged_proxy(pSettings, size);
}

void NEXUS_GetPlatformCapabilities_tagged( NEXUS_PlatformCapabilities *pCap, size_t size )
{
    if (!NEXUS_Platform_P_State.init) {
        if (NEXUS_P_Init()) return;
    }
    NEXUS_GetPlatformCapabilities_tagged_proxy(pCap, size);
}

NEXUS_Error NEXUS_Platform_GetMemory_tagged( NEXUS_PlatformMemory *memory, unsigned size )
{
    if (!NEXUS_Platform_P_State.init) {
        int rc = NEXUS_P_Init();
        if (rc) return rc;
    }
    return NEXUS_Platform_GetMemory_tagged_proxy(memory, size);
}

NEXUS_Error NEXUS_GetPlatformConfigCapabilities_tagged(const NEXUS_PlatformSettings *pSettings,
    const NEXUS_MemoryConfigurationSettings *pMemConfig, NEXUS_PlatformConfigCapabilities *pCap, unsigned size )
{
    if (!NEXUS_Platform_P_State.init) {
        int rc = NEXUS_P_Init();
        if (rc) return rc;
    }
    return NEXUS_GetPlatformConfigCapabilities_tagged_proxy(pSettings, pMemConfig, pCap, size);
}

static
NEXUS_Error NEXUS_Platform_P_InitProxy(int fd)
{
    NEXUS_Platform_P_State.proxy_fd = fd;
    proxy_ioctl_fd = fd;
    return NEXUS_SUCCESS;
}

static
void NEXUS_Platform_P_UninitProxy(void)
{
    NEXUS_Platform_P_State.proxy_fd = -1;
    proxy_ioctl_fd = -1;
    return;
}

enum nexus_platform_lockscheduler {
    nexus_platform_lockscheduler_lock,
    nexus_platform_lockscheduler_no_lock
};

static int
NEXUS_Platform_P_RunCallbacks(int proxy_fd, unsigned timeout, enum nexus_platform_lockscheduler lock, struct NEXUS_Platform_P_ProxyScheduler *scheduler)
{
    unsigned i;
    int rc;
    int prc; /* we will BERR_TRACE but otherwise ignore pthread errors. there is no recovery. */
    PROXY_NEXUS_Scheduler *data = &scheduler->data;
    PROXY_NEXUS_RunScheduler runScheduler;
    NEXUS_ModulePriority priority = scheduler->priority;

    runScheduler.in.priority = priority;
    runScheduler.in.timeout = timeout;
#if NEXUS_PROXY_STATISTICS && NEXUS_P_DEBUG_PROXY_CALLBACKS
    scheduler->stats.runCallbacksCount++;
#endif

    /* we need schedulers to park outside of ioctl */
    prc = pthread_rwlock_rdlock(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
    if (prc) {
        BERR_TRACE(prc); /* keep going */
    }
    else {
        prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
        if (prc) BERR_TRACE(prc); /* keep going */
    }

    /* first, call blocking function to drive kernel state machine */
    prc = pthread_rwlock_rdlock(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
    if (prc) BERR_TRACE(prc); /* keep going */
    rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_RunScheduler, &runScheduler);
    if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR);goto unlock_ioctl_lock; }

    BDBG_ASSERT(data->out.count == 0); /* nothing should have been left over */
    if (!runScheduler.out.has_callbacks) {
        BDBG_MSG_TRACE(("NEXUS_Platform_P_RunCallbacks(%p): no callbacks", scheduler));
    }
    else {
        data->in.priority = runScheduler.in.priority;
        rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_Scheduler, data);
        if(rc!=0) {
            rc = BERR_TRACE(BERR_OS_ERROR);goto unlock_ioctl_lock;
        }
        BDBG_ASSERT(data->out.count<=PROXY_NEXUS_CALLBACK_PACKET);
    }
    prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
    if (prc) BERR_TRACE(prc); /* keep going */

    if (data->out.count == 0) {
        return NEXUS_SUCCESS;
    }

    for(i=0;i<data->out.count;i++) {
        BDBG_MSG(("NEXUS_Platform_P_RunCallbacks: callback:%#lx context:%#lx param:%d", (unsigned long)data->out.callbacks[i].desc.callback, (unsigned long)data->out.callbacks[i].desc.context, data->out.callbacks[i].desc.param));
        if(data->out.callbacks[i].desc.callback) {
            prc = pthread_rwlock_rdlock(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[priority]);
            if (prc) BERR_TRACE(prc); /* keep going */

            if (lock == nexus_platform_lockscheduler_lock) {
                PROXY_NEXUS_SchedulerLock schedulerLock;
                schedulerLock.priority = priority;
                schedulerLock.lock = true;
                rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_SchedulerLock, &schedulerLock);
                if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR); break; }
            }

            if (data->out.callbacks[i].interfaceHandle) {
                /* remember the current callback so we only sync with schedulers where necessary */
                scheduler->current_callback = (void *)(unsigned long)data->out.callbacks[i].interfaceHandle;

                {
                    NEXUS_Callback callback;
#if NEXUS_P_DEBUG_PROXY_CALLBACKS
#if NEXUS_PROXY_STATISTICS
                    NEXUS_Time stop_callback;
                    unsigned dur;
                    scheduler->stats.callbackCount++;
#endif
                    NEXUS_Time_Get(&scheduler->monitor.startTime);
                    scheduler->monitor.callback = data->out.callbacks[i];
                    scheduler->monitor.valid = true;
#endif
                    callback = (NEXUS_Callback)(unsigned long)data->out.callbacks[i].desc.callback;
                    callback((void *)(unsigned long)data->out.callbacks[i].desc.context, data->out.callbacks[i].desc.param);
#if NEXUS_P_DEBUG_PROXY_CALLBACKS
                    scheduler->monitor.valid = false;
#if NEXUS_PROXY_STATISTICS
                    NEXUS_Time_Get(&stop_callback);
                    dur = NEXUS_Time_Diff(&stop_callback, &scheduler->monitor.startTime);
                    scheduler->stats.totalCallbackTime += dur;
                    if (dur > scheduler->stats.maxCallbackTime) {
                        scheduler->stats.maxCallbackTime = dur;
                        scheduler->stats.maxCallback = data->out.callbacks[i].desc.callback;
                    }
#endif
#endif
                }

                BDBG_ASSERT(scheduler->current_callback);
                scheduler->current_callback = NULL;
            }
            if (lock == nexus_platform_lockscheduler_lock) {
                PROXY_NEXUS_SchedulerLock schedulerLock;
                schedulerLock.priority = priority;
                schedulerLock.lock = false;
                rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_SchedulerLock, &schedulerLock);
                if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR); break; }
            }
            prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[priority]);
            if (prc) BERR_TRACE(prc); /* keep going */
        }
    }

    data->out.count = 0; /* all have been consumed */
    return rc;

unlock_ioctl_lock:
    prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
    if (prc) BERR_TRACE(prc); /* keep going */
    return rc;
}

static void
NEXUS_Platform_P_SchedulerThread(void *scheduler_)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    struct NEXUS_Platform_P_ProxyScheduler *scheduler = scheduler_;
    int proxy_fd;
    NEXUS_ModulePriority priority;

    nexus_p_set_scheduler_thread_priority(scheduler->priority);

    BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerThread>>:%#lx[%u]", (unsigned long)scheduler, scheduler->priority));
    priority = scheduler->priority;
    proxy_fd = state->proxy_fd;
    while(!state->stop) {
        NEXUS_P_Base_Scheduler_Status status;
        unsigned timeout;

        NEXUS_P_Base_ExternalScheduler_Step(priority, 0, &status, NULL, NULL);
         /* since user thread can't wakeup on kernel events, we do polling in user scheduler */
        if(status.exit) {
            break;
        }
        timeout = status.idle?100:10; /* since kernel thread wouldn't makeup on user events/timers we limit sleep time to 10ms or 100ms */
        BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerThread:%#lx[%u] timeout:%u(%u)", (unsigned long)scheduler, (unsigned)scheduler->priority, (unsigned)data.in.timeout, (unsigned)status.timeout));
        if(timeout>status.timeout) {
           timeout=status.timeout;
        }
        if(NEXUS_Platform_P_RunCallbacks(proxy_fd, timeout, nexus_platform_lockscheduler_lock, scheduler)!=0) {
            break;
        }
    }
    BDBG_MSG(("NEXUS_Platform_P_SchedulerThread<<:%#lx[%u]", (unsigned long)scheduler, scheduler->priority));
    return;
}

static void
NEXUS_Platform_P_SchedulerSlaveThread(void *scheduler_)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    int proxy_fd;
    struct NEXUS_Platform_P_ProxyScheduler *scheduler = scheduler_;

    /* only one scheduler thread for slaves */
    if(scheduler->priority==NEXUS_ModulePriority_eIdle) {
        nexus_p_set_scheduler_thread_priority(NEXUS_ModulePriority_eMax);
    } else {
        nexus_p_set_scheduler_thread_priority(scheduler->priority);
    }

    BDBG_MSG(("NEXUS_Platform_P_SchedulerSlaveThread>>"));
    proxy_fd = state->proxy_fd;
    while(!state->stop) {
        unsigned i;

        if(scheduler->priority==NEXUS_ModulePriority_eInternal) { /* there is a dedicated thread for NEXUS_ModulePriority_eInternal */
            NEXUS_P_Base_Scheduler_Status status;
            NEXUS_P_Base_ExternalScheduler_Step(scheduler->priority, 1000, &status, NULL, NULL);
            continue;
        } else {
            for(i=0;i<NEXUS_ModulePriority_eMax;i++) { /* in a single shared thread just step over all schedulers */
                NEXUS_P_Base_Scheduler_Status status;
                if(i==NEXUS_ModulePriority_eInternal) {
                    continue;
                }
                NEXUS_P_Base_ExternalScheduler_Step((NEXUS_ModulePriority)i, 0, &status, NULL, NULL);
            }
        }

        BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerSlaveThread"));
        if(NEXUS_Platform_P_RunCallbacks(proxy_fd, 10, nexus_platform_lockscheduler_no_lock, scheduler)!=0) {
            break;
        }
    }
    BDBG_MSG(("NEXUS_Platform_P_SchedulerSlaveThread<<"));
    return ;
}

#if NEXUS_TRACK_STOP_CALLBACKS
void NEXUS_Platform_P_StopCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
#else
void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle)
#endif
{
    unsigned i;
    int rc;
    int prc; /* we will BERR_TRACE but otherwise ignore pthread errors. there is no recovery. */
    NEXUS_ModulePriority priority;

    if (!interfaceHandle) return;

    /* first, stop callbacks in the driver */
    rc = ioctl(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_StopCallbacks, interfaceHandle);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}

    /* second, stop callbacks that are local */
    NEXUS_Base_P_StopCallbacks(interfaceHandle);

    /* park all schedulers before ioctl, which allows us to get ioctl_lock */
    prc = pthread_rwlock_wrlock(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
    if (prc) BERR_TRACE(prc); /* keep going */

    while (1) {
        struct timespec timeout = {0, 1000*1000}; /* 1 msec */
        /* wake up all scheduler threads, which will block on callback_lock */
        rc = ioctl(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_WakeupScheduler, NULL);
        if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}
        prc = pthread_rwlock_timedwrlock(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock, &timeout);
        if (prc == ETIMEDOUT) {
            continue;
        }
        if (prc) BERR_TRACE(prc); /* keep going */
        break;
    }

    /* clear interface pending in scheduler */
    for(priority=0;priority<NEXUS_ModulePriority_eMax;priority++) {
        PROXY_NEXUS_Scheduler *data = &NEXUS_Platform_P_State.schedulers[priority].data;
        if (!NEXUS_Platform_P_State.schedulers[priority].thread) continue;
        for(i=0;i<data->out.count;i++) {
            if ((void *)(unsigned long)data->out.callbacks[i].interfaceHandle == interfaceHandle) {
                prc = pthread_rwlock_wrlock(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[priority]);
                if (prc) BERR_TRACE(prc); /* keep going */
                BDBG_ASSERT(!NEXUS_Platform_P_State.schedulers[priority].current_callback);
                data->out.callbacks[i].interfaceHandle = 0;
                prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.callback_lock[priority]);
                if (prc) BERR_TRACE(prc); /* keep going */
            }
        }
    }

    prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.ioctl_lock);
    if (prc) BERR_TRACE(prc); /* keep going */
    prc = pthread_rwlock_unlock(&NEXUS_Platform_P_State.stopCallbacks.park_lock);
    if (prc) BERR_TRACE(prc); /* keep going */

    return;
}

#if NEXUS_TRACK_STOP_CALLBACKS
void NEXUS_Platform_P_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
#else
void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
#endif
{
    int rc;

    if (!interfaceHandle) return;

    rc = ioctl(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_StartCallbacks, interfaceHandle);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}

    NEXUS_Base_P_StartCallbacks(interfaceHandle);
    return;
}

NEXUS_Error
NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    BERR_Code rc;
    int urc;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    if (state->platform_init) {
        /* if you init (don't uninit) then join, you will fail here.
        likewise, if you init, fork (but don't exec), then join, you will fail here. */
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (state->init) {
        BDBG_WRN(("clients should not call NEXUS_Platform_GetDefaultSettings"));
        NEXUS_P_Uninit();
        /* fall through. console will contain errors, but it will work. */
    }

    state->fd = -1; /* unopened == -1, not 0 */
    state->slave = true;

    rc = NEXUS_Platform_P_Magnum_Init();
    if( rc != BERR_SUCCESS)  { rc = BERR_TRACE(rc); goto err_magnum; }

    /* Now, proceed to boot the board. -- Always initialize base first */
    rc = NEXUS_Base_Init(NULL);
    if ( rc !=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_base; }

    BDBG_MSG((">CORE_LOCAL"));
    rc = NEXUS_CoreModule_LocalInit();
    if ( rc!=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_core_local; }

#if NEXUS_HAS_SURFACE
    BDBG_MSG((">SURFACE_LOCAL"));
    rc = NEXUS_SurfaceModule_LocalInit();
    if ( rc!=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_surface_local; }
#endif

    BDBG_MSG((">MODULE"));
    state->module = NEXUS_Module_Create("proxy", NULL);
    if ( !state->module ) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_proxy; }

    BDBG_MSG((">PROXY DRIVER"));
    rc = NEXUS_Platform_P_OpenDriver();
    if(rc!=BERR_SUCCESS) {
        goto err_driver;
    }

    urc = ioctl(state->fd, IOCTL_PROXY_NEXUS_AuthenticatedJoin, pSettings);
    if(urc!=0) { rc = BERR_TRACE(BERR_OS_ERROR);goto err_join; }

    BDBG_MSG((">ALL OTHER DRIVERS"));
    rc = NEXUS_Platform_P_InitOtherDrivers();
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_driver; }

    BDBG_MSG((">OS"));
    rc = NEXUS_Platform_P_InitOS();
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_os; }

    rc = NEXUS_Platform_P_ModulesInit(NULL);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_modules;}

    state->platform_init = true;
    state->init = true;

    return NEXUS_SUCCESS;

err_modules:
    NEXUS_Platform_P_UninitOS();
err_driver:
err_join:
    NEXUS_Platform_P_UninitDriver();
err_os:
    NEXUS_Module_Destroy(state->module);
err_proxy:
#if NEXUS_HAS_SURFACE
    NEXUS_SurfaceModule_LocalUninit();
err_surface_local:
#endif
    NEXUS_CoreModule_LocalUninit();
err_core_local:
    NEXUS_Base_Uninit();
err_base:
    NEXUS_Platform_P_Magnum_Uninit();
err_magnum:
    return NEXUS_NOT_SUPPORTED;
}

void *NEXUS_P_ProxyCall_OffsetToAddr(NEXUS_Addr addr)
{
    if(addr!=0) {
        return NEXUS_OffsetToCachedAddr(addr);
    }
    return NULL;
}

NEXUS_Addr NEXUS_P_ProxyCall_AddrToOffset(const void *ptr)
{
    if(ptr!=NULL) {
        NEXUS_Addr addr = NEXUS_AddrToOffset(ptr);
        if(addr==0) {
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        return addr;
    }
    return 0;
}

NEXUS_Error NEXUS_P_ProxyCall_InVarArg_AddrField(NEXUS_Addr *dst, const void *src, unsigned struct_size, unsigned field_offset, unsigned count)
{
    unsigned i;
    for(i=0;i<count;i++) {
        const void *varArgData = src;
        dst[i] = NEXUS_P_ProxyCall_AddrToOffset( *(void **)((uint8_t *)varArgData + i*struct_size + field_offset));
    }
    return NEXUS_SUCCESS;
}

#if NEXUS_CPU_ARM
BERR_Code  NEXUS_Platform_P_CacheFlush( void* addr, size_t nbytes )
{
    BERR_Code rc;
    PROXY_NEXUS_CacheFlush data;

    data.address = (unsigned long)addr;
    data.length = nbytes;
    rc = ioctl(NEXUS_Platform_P_State.fd, IOCTL_PROXY_NEXUS_CacheFlush, &data);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}
    return BERR_SUCCESS;
}
#endif

/* including the c code requires less refactoring of internal apis */
#include "nexus_platform_proxy_local.c"
