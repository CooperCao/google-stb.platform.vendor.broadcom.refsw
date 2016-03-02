/***************************************************************************
*     (c)2008-2014 Broadcom Corporation
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
#if BCHP_CHIP == 7408
#include "bape_img.h"
#elif defined BDSP_RAAGA_SUPPORT
#include "bdsp_raaga_img.h"
#else
#if NEXUS_HAS_AUDIO
#include "brap_img.h"
#endif
#endif
#include "nexus_platform_image.h"
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "bvce_image.h"
#endif
#if NEXUS_HAS_SAGE
extern void *SAGE_IMAGE_Context;
extern BIMG_Interface SAGE_IMAGE_Interface;
#endif
#if NEXUS_HAS_SCM
extern void *SCM_IMAGE_Context;
extern BIMG_Interface SCM_IMAGE_Interface;
#endif
#if NEXUS_HAS_PICTURE_DECODER
#include "bsid_img.h"
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
#if NEXUS_FRONTEND_7584
#include "bhab_7584_fw.h"
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
#if NEXUS_FRONTEND_45308
extern const void* BHAB_45308_IMG_Context;
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
    BKNI_MutexHandle callbackLock; /* held while callback is running */

    BKNI_MutexHandle dataLock; /* held when accessing data.out or current_callback */
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
        NEXUS_Callback callback;
        NEXUS_Time startTime;
    } monitor;
#endif
};

static int nexus_p_start_watchdog_thread(void);
static void nexus_p_stop_watchdog_thread(void);

typedef struct nexus_proxy_stopped_callback_entry {
    BLST_D_ENTRY(nexus_proxy_stopped_callback_entry) list;
    void *interfaceHandle;
    struct {
#if NEXUS_TRACK_STOP_CALLBACKS
        const char* pFileName;
        unsigned lineNumber;
        const char* pFunctionName;
        NEXUS_Time stopTime;
#endif
        unsigned blockedCount;
    } debug;
} nexus_proxy_stopped_callback_entry;

static struct NEXUS_Platform_P_State
{
    NEXUS_ModuleHandle module;
    struct {
        NEXUS_HeapHandle heap;
        void *addr;
        unsigned long length;
        bool dynamic;
        NEXUS_MemoryMapType memoryMapType;
    } mmaps[NEXUS_MAX_HEAPS];
    int proxy_fd;
    bool stop;
    bool init; /* NEXUS_P_Init called */
    bool platform_init; /* NEXUS_Platform_Init called */
    bool slave;
    struct NEXUS_Platform_P_ProxyScheduler schedulers[NEXUS_ModulePriority_eMax];
    int fd;
    bool opened[NEXUS_PLATFORM_P_NUM_DRIVERS];
    BLST_D_HEAD(nexus_proxy_stopped_callback_list, nexus_proxy_stopped_callback_entry) stopped_callbacks;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_Platform_P_DebugLog debugLog;
} NEXUS_Platform_P_State;

int proxy_ioctl_fd = -1;

#define NEXUS_MODULE_SELF NEXUS_Platform_P_State.module

void NEXUS_Platform_GetDefaultSettings_tagged_proxy(NEXUS_PlatformSettings *pSettings, size_t size);
void NEXUS_GetDefaultMemoryConfigurationSettings_tagged_proxy( NEXUS_MemoryConfigurationSettings *pSettings, size_t size );
void NEXUS_GetPlatformCapabilities_tagged_proxy( NEXUS_PlatformCapabilities *pCap, size_t size );
NEXUS_Error NEXUS_Platform_Init_tagged_proxy(const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck);
void NEXUS_Platform_Uninit_proxy(void);
NEXUS_Error NEXUS_Platform_InitStandby_proxy(const NEXUS_PlatformStandbySettings *pSettings);
void NEXUS_Platform_UninitStandby_proxy(void);

void *NEXUS_Platform_P_MapMemory( NEXUS_Addr offset, size_t length, NEXUS_MemoryMapType type)
{
    void *addr;
    if (type!=NEXUS_MemoryMapType_eCached) return NULL; /* only cached mmap allows in proxy */
    addr = mmap64(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, NEXUS_Platform_P_State.fd, offset);
    if (addr == MAP_FAILED) {
        BDBG_ERR(("mmap failed: offset " BDBG_UINT64_FMT ", size=%u, errno=%d", BDBG_UINT64_ARG(offset), length, errno));
        addr = NULL;
    }
    BDBG_MSG(("mmap  offset:" BDBG_UINT64_FMT " size:%u -> %p", BDBG_UINT64_ARG(offset), length, addr));
    return addr;
}

void NEXUS_Platform_P_UnmapMemory( void *addr, size_t length, NEXUS_MemoryMapType memoryMapType)
{
    BSTD_UNUSED(memoryMapType);
    BDBG_MSG(("unmap: addr:%p size:%u", addr, (unsigned)length));
    munmap(addr, length);
    return;
}

static NEXUS_Error nexus_p_add_heap(unsigned i, NEXUS_HeapHandle heap, NEXUS_Addr base, unsigned length, NEXUS_MemoryType memoryType, bool dynamic, void *user_address)
{
    void *addr = NULL;
    NEXUS_MemoryMapType memoryMapType = NEXUS_MemoryMapType_eFake;

    if (user_address) {
        addr = user_address;
    }
    else if( (memoryType & (NEXUS_MEMORY_TYPE_NOT_MAPPED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)) == 0) {
        memoryMapType = ((memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED)==NEXUS_MEMORY_TYPE_APPLICATION_CACHED)?NEXUS_MemoryMapType_eCached:NEXUS_MemoryMapType_eFake;
        addr = nexus_p_map_memory(base, length, memoryMapType);
        if (!addr) return BERR_TRACE(BERR_OS_ERROR);
        NEXUS_P_AddMap(base, addr, addr, length);
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

static NEXUS_Error
NEXUS_Platform_P_InitOS(void)
{
    NEXUS_Error rc;
    PROXY_NEXUS_GetMemory mem;
    unsigned i;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    struct nexus_map_settings map_settings;

    if(!state->slave) {
        for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
            NEXUS_Base_Scheduler_Config config;

            BDBG_MSG((">CALLBACK[%u]", i));
            rc = BKNI_CreateMutex(&state->schedulers[i].callbackLock);
            if (rc) {rc = BERR_TRACE(rc); goto err_create_mutex;}
            rc = BKNI_CreateMutex(&state->schedulers[i].dataLock);
            if (rc) {rc = BERR_TRACE(rc); goto err_create_mutex;}

            NEXUS_P_Base_GetSchedulerConfig(i, &config);
            state->schedulers[i].priority = i;
            state->schedulers[i].thread = NEXUS_Thread_Create(config.name, NEXUS_Platform_P_SchedulerThread, &state->schedulers[i], config.pSettings);
            if(!state->schedulers[i].thread) { rc = BERR_TRACE(NEXUS_OS_ERROR); goto err_threads; }
        }
    }
    else {
        /* only create one */
        state->schedulers[0].priority = 0;
        rc = BKNI_CreateMutex(&state->schedulers[0].callbackLock);
        if (rc) {rc = BERR_TRACE(rc); goto err_create_mutex;}
        rc = BKNI_CreateMutex(&state->schedulers[0].dataLock);
        if (rc) {rc = BERR_TRACE(rc); goto err_create_mutex;}
        state->schedulers[0].thread = NEXUS_Thread_Create("slave_callback", NEXUS_Platform_P_SchedulerSlaveThread, &state->schedulers[0], NULL);
        if (!state->schedulers[0].thread) { rc = BERR_TRACE(NEXUS_OS_ERROR); goto err_threads; }

        BDBG_ASSERT(!state->schedulers[1].callbackLock); /* be sure no state leaked over from previous Init */
    }

    nexus_p_start_watchdog_thread();

    NEXUS_Platform_GetClientConfiguration(&state->clientConfig);

    BDBG_MSG((">IOCTL_PROXY_NEXUS_GetMemory"));
    rc = ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_GetMemory, &mem);
    if (rc) {rc=BERR_TRACE(rc);goto err_get_memory;}

    nexus_p_get_default_map_settings(&map_settings);
    /* see nexus_platform_core.c for fake address range logic */
    map_settings.offset = NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE;
    map_settings.size = NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE - 4096;
    map_settings.mmap = NEXUS_Platform_P_MapMemory;
    map_settings.munmap = NEXUS_Platform_P_UnmapMemory;
    rc = nexus_p_init_map(&map_settings);
    if (rc) {rc = BERR_TRACE(rc); goto err_init_map;}

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        BDBG_MSG(("mem.region[%d]: %#x %#x %d", i, mem.region[i].base, mem.region[i].length, mem.region[i].memoryType));
        if (mem.region[i].length) {
            rc = nexus_p_add_heap(i, (NEXUS_HeapHandle)-1, mem.region[i].base, mem.region[i].length, mem.region[i].memoryType, false, NULL);
            if (rc) { rc = BERR_TRACE(BERR_OS_ERROR); goto err_mmap; }
        }
    }

    BLST_D_INIT(&state->stopped_callbacks);

#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM && !B_REFSW_SYSTEM_MODE_CLIENT
    if(!state->slave) {
        (void)NEXUS_Platform_P_InitWakeupDriver();
        (void)NEXUS_Platform_P_InitThermalMonitor();
    }
#endif

    return NEXUS_SUCCESS;

err_mmap:
    NEXUS_Platform_P_UninitOS();
err_init_map:
err_get_memory:
err_threads:
err_create_mutex:
    state->stop = true;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if (state->schedulers[i].thread) {
            NEXUS_Thread_Destroy(state->schedulers[i].thread);
        }
        if (state->schedulers[i].callbackLock) {
            BKNI_DestroyMutex(state->schedulers[i].callbackLock);
            BKNI_DestroyMutex(state->schedulers[i].dataLock);
        }
    }
    return rc;
}

static void
NEXUS_Platform_P_UninitOS(void)
{
    unsigned i;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    nexus_proxy_stopped_callback_entry *entry;

#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM && !B_REFSW_SYSTEM_MODE_CLIENT
    if(!state->slave) {
        NEXUS_Platform_P_UninitThermalMonitor();
        NEXUS_Platform_P_UninitWakeupDriver();
    }
#endif

    nexus_p_stop_watchdog_thread();

    while(NULL!=(entry=BLST_D_FIRST(&state->stopped_callbacks))) {
        BLST_D_REMOVE_HEAD(&state->stopped_callbacks, list);
        BKNI_Free(entry);
    }

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        nexus_p_remove_heap(i);
    }
    nexus_p_uninit_map();

    state->stop = true;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if (state->schedulers[i].thread) {
            NEXUS_Thread_Destroy(state->schedulers[i].thread);
        }
        if (state->schedulers[i].callbackLock) {
            BKNI_DestroyMutex(state->schedulers[i].callbackLock);
            BKNI_DestroyMutex(state->schedulers[i].dataLock);
        }
    }
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
    rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (rc) BERR_TRACE(rc); /* keep going */

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
    BDBG_ASSERT(state->proxy_fd>0);
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
    if(state->debugLog.logWriter) {
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

    /* use proc setenv to deliver boxmode from device tree (or env override) to kernel mode */
    {
        FILE *file;
        unsigned boardId = NEXUS_Platform_P_ReadBoardId();
#if !NEXUS_PLATFORM_P_READ_BOX_MODE
        unsigned boxMode = NEXUS_Platform_P_ReadBoxMode();
        file = fopen("/proc/brcm/config", "w");
        if (file) {
            fprintf(file, "B_REFSW_BOXMODE=%u", boxMode);
            fclose(file);
        }
#endif
        file = fopen("/proc/brcm/config", "w");
        if (file) {
            fprintf(file, "B_REFSW_BOARD_ID=%u", boardId);
            fclose(file);
        }
    }

    state->init = true;
    return 0;

err_driver:
    NEXUS_Module_Destroy(proxy);
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

    if (!state->slave) {
        int urc;
        BDBG_MSG(("<DEBUG_LOG"));
        urc = ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_Log_Deactivate, NULL);
        NEXUS_Platform_P_DebugLog_Uninit(&state->debugLog);
    }

    BDBG_MSG(("<DRIVER"));
    NEXUS_Platform_P_UninitDriver();

    BDBG_MSG(("<MODULE"));
    NEXUS_Module_Destroy(state->module);
    state->module = NULL;

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
        BDBG_ERR(("NEXUS_Platform_GetDefaultSettings:size mismatch %u != %u", sizeof(*pSettings), size));
        BKNI_Memset(pSettings, 0, size);
        return;
    }

    /* call the proxy version */
    NEXUS_Platform_GetDefaultSettings_tagged_proxy(pSettings, size);
#if NEXUS_HAS_FILE
    {
        NEXUS_FileModuleSettings fileModuleSettings;
        NEXUS_FileModule_GetDefaultSettings(&fileModuleSettings);
        pSettings->fileModuleSettings.workerThreads = fileModuleSettings.workerThreads;
        BDBG_CASSERT(sizeof(pSettings->fileModuleSettings.schedulerSettings) == sizeof(fileModuleSettings.schedulerSettings));
        BKNI_Memcpy(pSettings->fileModuleSettings.schedulerSettings, fileModuleSettings.schedulerSettings, sizeof(pSettings->fileModuleSettings.schedulerSettings));
    }
#endif
    #if NEXUS_HAS_AUDIO && NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if ( NEXUS_GetEnv("audio_logs_enabled") )
    {
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eUartMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eDramMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled = true;
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
    }
    #endif
}

#if NEXUS_CONFIG_IMAGE

static NEXUS_ThreadHandle gImageThread=NULL;
static void NEXUS_Platform_P_ImageThread(void *context)
{
    BSTD_UNUSED(context);
    BDBG_MSG(("Image Thread"));
    BDBG_ASSERT(NEXUS_Platform_P_State.proxy_fd>0);
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
    #if BCHP_CHIP == 7408
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BAPE_IMG_Interface, BAPE_IMG_Context, NEXUS_CORE_IMG_ID_RAP);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    #elif defined BDSP_RAAGA_SUPPORT
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BDSP_IMG_Interface, BDSP_IMG_Context, NEXUS_CORE_IMG_ID_RAP);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    #else
	#if NEXUS_HAS_AUDIO
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BRAP_IMG_Interface, BRAP_IMG_Context, NEXUS_CORE_IMG_ID_RAP);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
	#endif
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
#if NEXUS_FRONTEND_7584
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_CTFE_IMG_Interface, (void *)&bcm7584_leap_image, NEXUS_CORE_IMG_ID_FRONTEND_7584); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
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
#if NEXUS_FRONTEND_45308
    rc = Nexus_Platform_P_Image_Interfaces_Register(&BHAB_SATFE_IMG_Interface, &BHAB_45308_IMG_Context, NEXUS_CORE_IMG_ID_FRONTEND_45308); if (rc != NEXUS_SUCCESS) { return BERR_TRACE(NEXUS_UNKNOWN); }
#endif
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
            NEXUS_P_GET_STRUCT_SIZES(), structSizeCheck));
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_platform_init;
    }

#if NEXUS_CONFIG_IMAGE
    BDBG_MSG(("Image"));
    errCode = NEXUS_Platform_P_InitImage(pSettings?&pSettings->imgInterface:NULL);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_platform_init; }
#endif

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

    if (pSettings) {
        errCode = NEXUS_Platform_P_DropPrivilege(pSettings);
        BDBG_ASSERT(!errCode); /* failure will terminate */
    }

    /* Success */
    NEXUS_Platform_P_State.platform_init = true;
    BDBG_MSG((">DONE"));
    return NEXUS_SUCCESS;

err:
    NEXUS_Platform_P_State.module = NULL;
    NEXUS_Platform_P_UninitOS();
err_os:
    NEXUS_Platform_P_UninitDriver();
err_driver:
    NEXUS_Platform_Uninit_proxy();
err_image_init:
#if NEXUS_CONFIG_IMAGE
    NEXUS_Platform_P_UninitImage();
#endif
err_platform_init:
    NEXUS_P_Uninit();
    return errCode;
}

void
NEXUS_Platform_Uninit(void)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    if (!state->platform_init) {
        return;
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

        BDBG_MSG(("<NEXUS UNINIT"));
        NEXUS_P_Uninit();
    } else {
        BDBG_MSG(("<OS"));
        NEXUS_Platform_P_UninitOS();

        /* tell driver that shutdown is clean */
        (void)ioctl(state->proxy_fd, IOCTL_PROXY_NEXUS_Platform_Uninit, NULL);

        BDBG_MSG(("<NEXUS UNINIT"));
        NEXUS_P_Uninit();
    }
    return ;
}

/* local copy */
NEXUS_Error NEXUS_Platform_InitStandby( const NEXUS_PlatformStandbySettings *pSettings )
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

/* find a stopped interface handle */
static nexus_proxy_stopped_callback_entry *
NEXUS_P_Proxy_FindStoppedCallbacks( void *interfaceHandle )
{
    nexus_proxy_stopped_callback_entry *entry;
    for(entry=BLST_D_FIRST(&NEXUS_Platform_P_State.stopped_callbacks);entry;entry=BLST_D_NEXT(entry,list)) {
        if(entry->interfaceHandle == interfaceHandle) {
            return entry;
        }
    }
    return NULL;
}

static int
NEXUS_Platform_P_RunCallbacks(int proxy_fd, PROXY_NEXUS_RunScheduler *runScheduler, PROXY_NEXUS_SchedulerLock *schedulerLock, struct NEXUS_Platform_P_ProxyScheduler *scheduler)
{
    unsigned i;
    int rc;
    PROXY_NEXUS_Scheduler *data = &scheduler->data;

#if NEXUS_PROXY_STATISTICS && NEXUS_P_DEBUG_PROXY_CALLBACKS
    scheduler->stats.runCallbacksCount++;
#endif

    /* first, call blocking function to drive kernel state machine */
    rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_RunScheduler, runScheduler);
    if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR);goto done; }
    if (!runScheduler->out.has_callbacks) {
        BDBG_MSG_TRACE(("NEXUS_Platform_P_RunCallbacks(%p): no callbacks", scheduler));
        goto done;
    }

    BKNI_AcquireMutex(scheduler->dataLock);
    BDBG_ASSERT(data->out.count == 0); /* nothing should have been left over */
    data->in.priority = runScheduler->in.priority;
    rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_Scheduler, data);
    if(rc!=0) {
        rc = BERR_TRACE(BERR_OS_ERROR);goto unlock_data;
    }
    BDBG_ASSERT(data->out.count<=PROXY_NEXUS_CALLBACK_PACKET);
    for(i=0;i<data->out.count;i++) {
        BDBG_MSG(("NEXUS_Platform_P_RunCallbacks: callback:%#lx context:%#lx param:%d", (unsigned long)data->out.callbacks[i].desc.callback, (unsigned long)data->out.callbacks[i].desc.context, data->out.callbacks[i].desc.param));
        if(data->out.callbacks[i].desc.callback) {
            nexus_proxy_stopped_callback_entry *entry;

            if (schedulerLock) {
                schedulerLock->lock = true;
                rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_SchedulerLock, schedulerLock);
                if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR); break; }
            }

            if (data->out.callbacks[i].interfaceHandle) {
                /* remember the current callback so we only sync with schedulers where necessary */
                scheduler->current_callback = data->out.callbacks[i].interfaceHandle;

                /* must acquire callbackLock before releasing dataLock.
                dataLock cannot be held during callback. */
                BKNI_AcquireMutex(scheduler->callbackLock);
                BKNI_ReleaseMutex(scheduler->dataLock);

                NEXUS_LockModule();
                /* check NEXUS_P_Proxy_FindStoppedCallbacks. this is needed for non-Close StopCallbacks
                where the stop may be persistent. */
                entry = NEXUS_P_Proxy_FindStoppedCallbacks(data->out.callbacks[i].interfaceHandle);
                NEXUS_UnlockModule();
                if (!entry) {
#if NEXUS_P_DEBUG_PROXY_CALLBACKS
#if NEXUS_PROXY_STATISTICS
                    NEXUS_Time stop_callback;
                    unsigned dur;
                    scheduler->stats.callbackCount++;
#endif
                    NEXUS_Time_Get(&scheduler->monitor.startTime);
                    scheduler->monitor.callback = data->out.callbacks[i].desc.callback;
#endif
                    data->out.callbacks[i].desc.callback(data->out.callbacks[i].desc.context, data->out.callbacks[i].desc.param);
#if NEXUS_P_DEBUG_PROXY_CALLBACKS
                    scheduler->monitor.callback = NULL;
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
                else {
                    entry->debug.blockedCount++;
                }

                BKNI_AcquireMutex(scheduler->dataLock);
                BKNI_ReleaseMutex(scheduler->callbackLock);
                scheduler->current_callback = NULL;
            }
            if (schedulerLock) {
                schedulerLock->lock = false;
                rc = ioctl(proxy_fd, IOCTL_PROXY_NEXUS_SchedulerLock, schedulerLock);
                if(rc!=0) { rc = BERR_TRACE(BERR_OS_ERROR); break; }
            }
        }
    }
unlock_data:
    data->out.count = 0; /* all have been consumed */
    BKNI_ReleaseMutex(scheduler->dataLock);
done:
    return rc;
}

static void
NEXUS_Platform_P_SchedulerThread(void *scheduler_)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    struct NEXUS_Platform_P_ProxyScheduler *scheduler = scheduler_;
    PROXY_NEXUS_SchedulerLock schedulerLock;
    int proxy_fd;
    NEXUS_ModulePriority priority;

    nexus_p_set_scheduler_thread_priority(scheduler->priority);

    BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerThread>>:%#lx[%u]", (unsigned long)scheduler, scheduler->priority));
    priority = scheduler->priority;
    schedulerLock.priority = priority;
    proxy_fd = state->proxy_fd;
    while(!state->stop) {
        NEXUS_P_Base_Scheduler_Status status;
        PROXY_NEXUS_RunScheduler data;

        NEXUS_P_Base_ExternalScheduler_Step(priority, 0, &status, NULL, NULL);
         /* since user thread can't wakeup on kernel events, we do polling in user scheduler */
        if(status.exit) {
            break;
        }
        data.in.priority = priority;
        data.in.timeout = status.idle?100:10; /* since kernel thread wouldn't makeup on user events/timers we limit sleep time to 10ms or 100ms */
        BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerThread:%#lx[%u] timeout:%u(%u)", (unsigned long)scheduler, (unsigned)scheduler->priority, (unsigned)data.in.timeout, (unsigned)status.timeout));
        if(data.in.timeout>status.timeout) {
           data.in.timeout=status.timeout;
        }
        if(NEXUS_Platform_P_RunCallbacks(proxy_fd, &data, &schedulerLock, scheduler)!=0) {
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
    nexus_p_set_scheduler_thread_priority(NEXUS_ModulePriority_eMax);

    BDBG_MSG(("NEXUS_Platform_P_SchedulerSlaveThread>>"));
    proxy_fd = state->proxy_fd;
    while(!state->stop) {
        PROXY_NEXUS_RunScheduler data;
        unsigned i;

        for(i=0;i<NEXUS_ModulePriority_eMax;i++) { /* in a single shared thread just step over all schedulers */
            NEXUS_P_Base_Scheduler_Status status;

            NEXUS_P_Base_ExternalScheduler_Step((NEXUS_ModulePriority)i, 0, &status, NULL, NULL);
        }

        data.in.priority = NEXUS_ModulePriority_eDefault;
        data.in.timeout = 10;
        BDBG_MSG_TRACE(("NEXUS_Platform_P_SchedulerSlaveThread: timeout:%u(%u)", (unsigned)data.in.timeout, (unsigned)data.in.timeout));
        if(NEXUS_Platform_P_RunCallbacks(proxy_fd, &data, NULL, scheduler)!=0) {
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
    nexus_proxy_stopped_callback_entry *entry;
    unsigned i;
    int rc;

    if (!interfaceHandle) return;

    /* first, stop callbacks in the driver */
    rc = ioctl(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_StopCallbacks, interfaceHandle);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}

    /* second, stop callbacks that are local */
    NEXUS_Base_P_StopCallbacks(interfaceHandle);

    /* third, add to persistent list of stopped callbacks */
    NEXUS_LockModule();
    entry = NEXUS_P_Proxy_FindStoppedCallbacks(interfaceHandle);
    if(entry==NULL) {
        entry = BKNI_Malloc(sizeof(*entry));
        if(entry) {
            entry->interfaceHandle = interfaceHandle;
#if NEXUS_TRACK_STOP_CALLBACKS
            entry->debug.pFileName = pFileName;
            entry->debug.lineNumber = lineNumber;
            entry->debug.pFunctionName = pFunctionName;
            NEXUS_Time_Get(&entry->debug.stopTime);
#endif
            entry->debug.blockedCount = 0;
            BLST_D_INSERT_HEAD(&NEXUS_Platform_P_State.stopped_callbacks, entry, list);
        } else {
            NEXUS_Error rc =BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            BSTD_UNUSED(rc);
        }
    }
    else {
        BDBG_WRN(("Stopping already stopped callback interface %p", entry->interfaceHandle));
    }
    NEXUS_UnlockModule();

    /* here all relevant callbacks are marked as 'stopped'. now, we must synchronize with every scheduler
    to ensure this function returns with no stopped callback still running on any scheduler. */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct NEXUS_Platform_P_ProxyScheduler *scheduler = &NEXUS_Platform_P_State.schedulers[i];
        if (scheduler->callbackLock) {
            unsigned j;
            bool wait = false;

            /* delete in-flight callbacks that have been stopped */
            BKNI_AcquireMutex(scheduler->dataLock);
            for (j=0;j<PROXY_NEXUS_CALLBACK_PACKET && j<scheduler->data.out.count;j++) {
                if (scheduler->data.out.callbacks[j].interfaceHandle == interfaceHandle) {
                    scheduler->data.out.callbacks[j].interfaceHandle = NULL;
                }
            }
            wait = (scheduler->current_callback == interfaceHandle);
            BKNI_ReleaseMutex(scheduler->dataLock);

            /* only wait for stopped callback is current on the scheduler */
            if (wait) {
                /* a simple acquire/release pair is enough to ensure that any active callback (which may include a stopped callback) completes.
                if you find that your application has deadlocked here, please debug your application and don't relax this code.
                for instance, you may be trying to close an interface inside a callback, or your close function might be protected with the same
                mutex which is being acquired inside a callback. both of these cases will result in a deadlock here. */
                BKNI_AcquireMutex(scheduler->callbackLock);
                BKNI_ReleaseMutex(scheduler->callbackLock);
            }
        }
    }

    return;
}

#if NEXUS_TRACK_STOP_CALLBACKS
void NEXUS_Platform_P_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
#else
void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
#endif
{
    nexus_proxy_stopped_callback_entry *entry;
    int rc;

    if (!interfaceHandle) return;

    rc = ioctl(NEXUS_Platform_P_State.proxy_fd, IOCTL_PROXY_NEXUS_StartCallbacks, interfaceHandle);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}

    NEXUS_Base_P_StartCallbacks(interfaceHandle);

    NEXUS_LockModule();
    entry = NEXUS_P_Proxy_FindStoppedCallbacks(interfaceHandle);
    if(entry!=NULL) {
        if (entry->debug.blockedCount > 5) {
#if NEXUS_TRACK_STOP_CALLBACKS
            NEXUS_Time now;
            NEXUS_Time_Get(&now);
            BDBG_WRN(("Starting blocked callback interface %p:%u, %s:%s:%u, %s:%s:%u, time %u", entry->interfaceHandle, entry->debug.blockedCount,
                entry->debug.pFileName, entry->debug.pFunctionName, entry->debug.lineNumber,
                pFileName, pFunctionName, lineNumber,
                NEXUS_Time_Diff(&now, &entry->debug.stopTime)));
#else
            BDBG_WRN(("Starting blocked callback interface %p:%u", entry->interfaceHandle, entry->debug.blockedCount));
#endif
        }
        BLST_D_REMOVE(&NEXUS_Platform_P_State.stopped_callbacks, entry, list);
        BKNI_Free(entry);
    }
    NEXUS_UnlockModule();
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


#if NEXUS_CPU_ARM
BERR_Code  NEXUS_Platform_P_CacheFlush( void* addr, size_t nbytes )
{
    BERR_Code rc;
    PROXY_NEXUS_CacheFlush data;

    data.address = (void*)addr;
    data.length = (size_t)nbytes;
    rc = ioctl(NEXUS_Platform_P_State.fd, IOCTL_PROXY_NEXUS_CacheFlush, &data);
    if (rc!=0) { BERR_TRACE(BERR_OS_ERROR);}
    return BERR_SUCCESS;
}
#endif

/* including the c code requires less refactoring of internal apis */
#include "nexus_platform_proxy_local.c"

#if NEXUS_P_DEBUG_PROXY_CALLBACKS
BDBG_FILE_MODULE(nexus_proxy_statistics);

static struct {
    NEXUS_ThreadHandle thread;
    BKNI_EventHandle event;
} g_watchdog;

static void nexus_p_watchdog_thread(void *context)
{
    int rc;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    BSTD_UNUSED(context);
    do {
        unsigned i;
        unsigned total = !state->slave?NEXUS_ModulePriority_eMax:1;
        NEXUS_Time now;
        NEXUS_Time_Get(&now);
        for(i=0;i<total;i++) {
            struct NEXUS_Platform_P_ProxyScheduler *scheduler = &state->schedulers[i];
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
            if (scheduler->monitor.callback) {
                unsigned duration;
                duration = NEXUS_Time_Diff(&now, &scheduler->monitor.startTime);
                if (duration > 5000) {
                    BDBG_ERR(("stuck callback %p for %d msec. You can go from function pointer to name using 'nm'.", scheduler->monitor.callback, duration));
                }
            }
        }
        rc = BKNI_WaitForEvent(g_watchdog.event, 2000);
    } while (rc == BERR_TIMEOUT);
}

static int nexus_p_start_watchdog_thread(void)
{
    int rc;
    rc = BKNI_CreateEvent(&g_watchdog.event);
    if (rc) return BERR_TRACE(rc);
    g_watchdog.thread = NEXUS_Thread_Create("proxy_watchdog", nexus_p_watchdog_thread, NULL, NULL);
    if (!g_watchdog.thread) {
        BKNI_DestroyEvent(g_watchdog.event);
        return BERR_TRACE(-1);
    }
    return 0;
}

static void nexus_p_stop_watchdog_thread(void)
{
    if (g_watchdog.thread) {
        BKNI_SetEvent(g_watchdog.event);
        NEXUS_Thread_Destroy(g_watchdog.thread);
        g_watchdog.thread = NULL;
        BKNI_DestroyEvent(g_watchdog.event);
    }

}
#else
static int nexus_p_start_watchdog_thread(void)
{
    return 0;
}
static void nexus_p_stop_watchdog_thread(void)
{
}
#endif
