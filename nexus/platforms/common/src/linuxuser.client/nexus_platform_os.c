/***************************************************************************
*  Copyright (C) 2008-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*   API name: Platform
*
***************************************************************************/

#include "nexus_types.h"
#include "nexus_base.h"
#include "client/nexus_client_prologue.h"
#include "nexus_platform_client_impl.h"
#include "nexus_platform.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_local_priv.h"
#include "blst_list.h"
#if NEXUS_WEBCPU_core1_server
NEXUS_Error NEXUS_Platform_P_InitWebCpuServer(void);
void NEXUS_Platform_P_UninitWebCpuServer(void);
#endif
#include "priv/nexus_core_module_local.h"
#if NEXUS_HAS_SURFACE
#include "priv/nexus_surface_module_local.h"
#else
#define NEXUS_SurfaceModule_LocalInit() 0
#define NEXUS_SurfaceModule_LocalUninit()
#endif

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
#include "priv/nexus_base_platform.h"

BDBG_MODULE(nexus_platform_os);

/* define prototypes for init/uninit functions */
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) \
    extern BERR_Code nexus_client_##module##_init(NEXUS_P_ClientModuleHandle module); \
    extern void nexus_client_##module##_uninit(void);
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE

static NEXUS_Error NEXUS_Platform_P_StartIpcClient(const NEXUS_ClientAuthenticationSettings *pSettings);
static void NEXUS_Platform_P_StopIpcClient(void);
static NEXUS_Error NEXUS_Platform_P_InitOS(void);
static void NEXUS_Platform_P_UninitOS(void);

static struct {
    const char *name;
    NEXUS_P_ClientModuleHandle module;
    NEXUS_Error (*init)(NEXUS_P_ClientModuleHandle module);
    void (*uninit)(void);
} g_nexus_client_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) {#module, NULL, nexus_client_##module##_init, nexus_client_##module##_uninit},
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
};
#define NEXUS_PLATFORM_P_NUM_DRIVERS  (sizeof(g_nexus_client_handlers)/sizeof(*g_nexus_client_handlers))

static struct NEXUS_Platform_P_State
{
    NEXUS_ModuleHandle module;
    int mem_fd;
    int uncached_mem_fd;
    int fake_mem_fd;
    struct {
        NEXUS_HeapHandle heap;
        void *addr;
        void *uncached_addr;
        unsigned long length;
        bool dynamic;
        NEXUS_MemoryMapType memoryMapType;
    } mmaps[NEXUS_MAX_HEAPS];
    bool init;
    struct nexus_client_init_data init_data;
} NEXUS_Platform_P_State;

#define NEXUS_MODULE_SELF NEXUS_Platform_P_State.module

void *NEXUS_Platform_P_MapMemory( NEXUS_Addr offset, size_t length, NEXUS_MemoryMapType type)
{
    void *addr;
    switch(type) {
    case NEXUS_MemoryMapType_eCached:
        addr = mmap64(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, NEXUS_Platform_P_State.mem_fd, offset);
        if (addr == MAP_FAILED) {
            BDBG_ERR(("mmap failed: offset " BDBG_UINT64_FMT ", size=%lu, errno=%d", BDBG_UINT64_ARG(offset), (unsigned long)length, errno));
            addr = NULL;
        }
        break;
    case NEXUS_MemoryMapType_eFake:
        addr = mmap64(0, length, PROT_NONE, MAP_SHARED, NEXUS_Platform_P_State.fake_mem_fd, 0);
        if (addr == MAP_FAILED) {
            BDBG_ERR(("mmap failed: offset " BDBG_UINT64_FMT ", size=%lu, errno=%d", BDBG_UINT64_ARG(offset), (unsigned long)length, errno));
            addr = NULL;
        }
        break;
    default:
        addr = NULL;
        break;
    }
    BDBG_MSG(("mmap  offset:" BDBG_UINT64_FMT " size:%lu -> %p", BDBG_UINT64_ARG(offset), (unsigned long)length, addr));
    return addr;
}

void NEXUS_Platform_P_UnmapMemory( void *addr, size_t length, NEXUS_MemoryMapType type)
{
    BDBG_MSG(("unmap: addr:%p size:%u", addr, (unsigned)length));
    BSTD_UNUSED(type);
    munmap(addr, length);
}


static NEXUS_Error nexus_p_add_heap(unsigned i, NEXUS_HeapHandle heap, unsigned base, unsigned length, NEXUS_MemoryType memoryType, bool dynamic, void *user_address)
{
    void *addr;
    void *uncached_addr = NULL;
    bool addmap = true;
    NEXUS_MemoryMapType memoryMapType = NEXUS_MemoryMapType_eFake;

    if (user_address) {
        addr = user_address;
    }
    else {
        /* clients don't deal with eDriver or fake addresses, just eApplication mapping.
        Nexus interfaces using eSecure memory require pointers, so we do an mmap, but it is not usable by the CPU. */
        if(memoryType & NEXUS_MEMORY_TYPE_SECURE) {
            memoryMapType = NEXUS_MemoryMapType_eFake;
            addr = NEXUS_Platform_P_MapMemory( base , length, memoryMapType);
            if (addr==MAP_FAILED) return BERR_TRACE(BERR_OS_ERROR);
        } else if (memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED) {
            memoryMapType = NEXUS_MemoryMapType_eCached;
            addr = NEXUS_Platform_P_MapMemory( base , length, memoryMapType);
            if (addr==MAP_FAILED) return BERR_TRACE(BERR_OS_ERROR);
        }
        else {
            /* use a fake address, but don't bother allocating it for unique addresses. this avoids errors in the thunk.
            because the client's fake addresses aren't unique, the client cannot use them to specific memory for server use.
            if allocation of fake addresses is required, use linuxuser/nexus_map.h. */
            addr = NULL;
            addmap = false;
        }
    }

    if( memoryType & (NEXUS_MEMORY_TYPE_NOT_MAPPED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)) {
        addr = NULL;
        addmap = false;
    }

#if NEXUS_WEBCPU_core1_server
    if (memoryType & NEXUS_MEMORY_TYPE_DRIVER_UNCACHED) {
        uncached_addr = mmap64(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, NEXUS_Platform_P_State.uncached_mem_fd, base);
        BDBG_MSG(("uncached mmap offset=%x=>%x size=%d fd=%d errno=%d", base, uncached_addr, length, NEXUS_Platform_P_State.uncached_mem_fd, (uncached_addr==MAP_FAILED) ? errno : 0));
        if (uncached_addr==MAP_FAILED) return BERR_TRACE(BERR_OS_ERROR);
    }
#endif

    if (addmap) {
        NEXUS_P_AddMap(base, addr, uncached_addr, length);
    }

    NEXUS_Platform_P_State.mmaps[i].memoryMapType = memoryMapType;
    NEXUS_Platform_P_State.mmaps[i].heap = heap;
    NEXUS_Platform_P_State.mmaps[i].addr = addr;
    NEXUS_Platform_P_State.mmaps[i].uncached_addr = uncached_addr;
    NEXUS_Platform_P_State.mmaps[i].length = length;
    NEXUS_Platform_P_State.mmaps[i].dynamic = dynamic;
    return 0;
}

static void nexus_p_remove_heap(unsigned i)
{
    if (NEXUS_Platform_P_State.mmaps[i].heap) {
        if (NEXUS_Platform_P_State.mmaps[i].addr) {
            NEXUS_Platform_P_UnmapMemory(NEXUS_Platform_P_State.mmaps[i].addr, NEXUS_Platform_P_State.mmaps[i].length, NEXUS_Platform_P_State.mmaps[i].memoryMapType );
        }
        if (NEXUS_Platform_P_State.mmaps[i].uncached_addr) {
            NEXUS_Platform_P_UnmapMemory(NEXUS_Platform_P_State.mmaps[i].uncached_addr, NEXUS_Platform_P_State.mmaps[i].length, NEXUS_MemoryMapType_eUncached);
        }
        NEXUS_Platform_P_State.mmaps[i].heap = NULL;
    }
}

static NEXUS_Error
NEXUS_Platform_P_InitOS(void)
{
    NEXUS_Error rc;
    int mem_fd;
    unsigned i;

    NEXUS_Platform_P_State.mem_fd = -1;
    NEXUS_Platform_P_State.uncached_mem_fd = -1;
    NEXUS_Platform_P_State.fake_mem_fd = -1;

    /* do memory map through bcmdriver. the server must grant access to heaps. */
    mem_fd = open("/dev/brcm0", O_RDWR);
    if (mem_fd == -1) {
        /* if that fails (because we have an old version of bcmdriver, we can try /dev/mem. this will only work for root user.  */
        mem_fd = open("/dev/mem", O_RDWR);
        if ( mem_fd < 0 ) { rc = BERR_TRACE(BERR_OS_ERROR); goto err_mem_open; }
    }
    NEXUS_Platform_P_State.mem_fd = mem_fd;
    if (fcntl(mem_fd, F_SETFD, FD_CLOEXEC) == -1 ) {
        BDBG_ERR(("fcntl(mem_fd, F_SETFD, FD_CLOEXEC) failed, errno=%d", errno));
        rc = BERR_TRACE(BERR_OS_ERROR); goto err_mem_open;
    }

#if NEXUS_WEBCPU_core1_server
    /* webcpu client needs uncached memory for GRC */
    mem_fd = open("/dev/brcm0", O_SYNC|O_RDWR);
    if (mem_fd == -1) {
        mem_fd = open("/dev/mem", O_SYNC|O_RDWR);
        if ( mem_fd < 0 ) { rc = BERR_TRACE(BERR_OS_ERROR); goto err_mem_open; }
    }
    NEXUS_Platform_P_State.uncached_mem_fd = mem_fd;
#else
    NEXUS_Platform_P_State.uncached_mem_fd = -1;
#endif
    mem_fd = open("/dev/zero", O_RDONLY);
    if ( mem_fd < 0 ) { rc = BERR_TRACE(BERR_OS_ERROR); goto err_mem_open; }
    NEXUS_Platform_P_State.fake_mem_fd = mem_fd;

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_HeapHandle heap = NEXUS_Platform_P_State.init_data.config.heap[i];
        if (heap) {
            BDBG_ASSERT(NEXUS_Platform_P_State.init_data.config.heap[i] == NEXUS_Platform_P_State.init_data.heap[i].heap);
#if NEXUS_WEBCPU_core1_server
            if (i == 0) {
                NEXUS_Platform_P_State.init_data.heap[i].memoryType = NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
            }
#endif
            rc = nexus_p_add_heap(i,
                NEXUS_Platform_P_State.init_data.heap[i].heap,
                NEXUS_Platform_P_State.init_data.heap[i].offset,
                NEXUS_Platform_P_State.init_data.heap[i].size,
                NEXUS_Platform_P_State.init_data.heap[i].memoryType,
                false,
                NULL);
            if (rc) {rc = BERR_TRACE(rc); goto err_mmap;}
        }
    }

    return NEXUS_SUCCESS;

err_mmap:
    NEXUS_Platform_P_UninitOS();
err_mem_open:
    return rc;
}

static void
NEXUS_Platform_P_UninitOS(void)
{
    unsigned i;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        nexus_p_remove_heap(i);

    }
    close(state->mem_fd);
    if (state->uncached_mem_fd != -1) {
        close(state->uncached_mem_fd);
    }
    if (state->fake_mem_fd != -1) {
        close(state->fake_mem_fd);
    }
    BKNI_Memset(state->mmaps, 0, sizeof(state->mmaps));
    return;
}



void
NEXUS_Platform_Uninit(void)
{
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    NEXUS_Platform_P_ModulesUninit();
#if NEXUS_CONFIG_IMAGE
    NEXUS_Platform_P_UninitImage();
#endif

#if NEXUS_WEBCPU_core1_server
    NEXUS_Platform_P_UninitWebCpuServer();
#endif

    NEXUS_Platform_P_StopIpcClient();

    BDBG_MSG(("<OS"));
    NEXUS_Platform_P_UninitOS();

    BDBG_MSG(("<MODULE"));
    NEXUS_Module_Destroy(state->module);
    state->module = NULL;

    BDBG_MSG(("<SURFACE_LOCAL"));
    NEXUS_SurfaceModule_LocalUninit();

    BDBG_MSG(("<CORE_LOCAL"));
    NEXUS_CoreModule_LocalUninit();

    BDBG_MSG(("<BASE"));
    NEXUS_Base_Uninit();

    BDBG_MSG(("<MAGNUM"));
    NEXUS_Platform_P_Magnum_Uninit();

    state->init = false;
}

NEXUS_P_ClientHandle g_client;

static NEXUS_Error NEXUS_Platform_P_StartIpcClient(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    unsigned i;

    g_client = NEXUS_P_Client_Init(pSettings, &NEXUS_Platform_P_State.init_data);
    if (!g_client) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    for (i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS;i++) {
        unsigned data_size = 4096; /* hardcoded max unix domain socket size */
        BDBG_MSG(("ipc init %d: %s (%d bytes)", i, g_nexus_client_handlers[i].name, data_size));

        g_nexus_client_handlers[i].module = NEXUS_P_Client_InitModule(g_client, i, data_size);
        if (!g_nexus_client_handlers[i].module) {
            BDBG_ERR(("can't connect to %s", g_nexus_client_handlers[i].name));
            continue; /* keep going. a subset of modules may be blocked. */
        }
        (*g_nexus_client_handlers[i].init)(g_nexus_client_handlers[i].module);
    }

    BDBG_MSG(("IPC client started"));
    return 0;
}

static void NEXUS_Platform_P_StopIpcClient()
{
    unsigned i;

    /* tell server immediately that we are going down */
    if (g_client) {
        NEXUS_P_Client_Disconnect(g_client);
    }

    /* then start tearing down */
    for (i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS;i++) {
        if (g_nexus_client_handlers[i].module) {
            NEXUS_P_Client_UninitModule(g_nexus_client_handlers[i].module);
            (*g_nexus_client_handlers[i].uninit)();
            g_nexus_client_handlers[i].module = NULL;
        }
    }
    if (g_client) {
        NEXUS_P_Client_Uninit(g_client);
        g_client = NULL;
    }
}

void NEXUS_Platform_GetDefaultSettings_tagged(NEXUS_PlatformSettings *pSettings, size_t size)
{
    memset(pSettings, 0, size);
    fprintf(stderr, "### libnexus_client.so does not implement NEXUS_Platform_GetDefaultSettings\n");
    return;
}

void NEXUS_GetPlatformCapabilities_tagged( NEXUS_PlatformCapabilities *pCap, size_t size )
{
    memset(pCap, 0, size);
    fprintf(stderr, "### libnexus_client.so does not implement NEXUS_GetPlatformCapabilities\n");
    return;
}

NEXUS_Error NEXUS_GetPlatformConfigCapabilities_tagged(const NEXUS_PlatformSettings *pSettings,
    const NEXUS_MemoryConfigurationSettings *pMemConfig, NEXUS_PlatformConfigCapabilities *pCap, unsigned size )
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pMemConfig);
    memset(pCap, 0, size);
    fprintf(stderr, "### libnexus_client.so does not implement NEXUS_GetPlatformConfigCapabilities\n");
    return -1;
}

NEXUS_Error NEXUS_Platform_Init_tagged( const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig,
    unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck )
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pMemConfig);
    BSTD_UNUSED(platformCheck);
    BSTD_UNUSED(versionCheck);
    BSTD_UNUSED(structSizeCheck);
    fprintf(stderr, "### libnexus_client.so does not implement NEXUS_Platform_Init\n");
    return -1;
}

NEXUS_Error NEXUS_Platform_AuthenticatedJoin( const NEXUS_ClientAuthenticationSettings *pSettings )
{
    BERR_Code rc;
    struct NEXUS_Platform_P_State *state = &NEXUS_Platform_P_State;

    rc = NEXUS_Platform_P_Magnum_Init();
    if( rc != BERR_SUCCESS)  { rc = BERR_TRACE(rc); goto err_magnum; }

    /* Now, proceed to boot the board. -- Always initialize base first */
    rc = NEXUS_Base_Init(NULL);
    if ( rc !=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_base; }

    BDBG_MSG((">CORE_LOCAL"));
    rc = NEXUS_CoreModule_LocalInit();
    if ( rc!=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_core_local; }

    BDBG_MSG((">SURFACE_LOCAL"));
    rc = NEXUS_SurfaceModule_LocalInit();
    if ( rc!=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_surface_local; }

    BDBG_MSG((">MODULE"));
    state->module = NEXUS_Module_Create("proxy", NULL);
    if ( !state->module ) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_proxy; }

    /* start ipc client modules */
    rc = NEXUS_Platform_P_StartIpcClient(pSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_ipcclient;}

    /* InitOS happens after the IPC connections are made. This allows us to learn the server-provided
    heap configuration and map according to it. */
    BDBG_MSG((">OS"));
    rc = NEXUS_Platform_P_InitOS();
    if(rc!=BERR_SUCCESS) { goto err_os; }

#if NEXUS_WEBCPU_core1_server
    /* must bring up local modules after ipc is up so we can make calls to the server */
    rc = NEXUS_Platform_P_InitWebCpuServer();
    if(rc!=BERR_SUCCESS) { goto err_os; }
#endif

    rc = NEXUS_Platform_P_ModulesInit(NULL);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_modules;}

    NEXUS_Platform_P_State.init = true;

    return NEXUS_SUCCESS;

err_modules:
    NEXUS_Platform_P_UninitOS();
err_os:
    NEXUS_Platform_P_StopIpcClient();
err_ipcclient:
    NEXUS_Module_Destroy(state->module);
err_proxy:
    NEXUS_SurfaceModule_LocalUninit();
err_surface_local:
    NEXUS_CoreModule_LocalUninit();
err_core_local:
    NEXUS_Base_Uninit();
err_base:
    NEXUS_Platform_P_Magnum_Uninit();
err_magnum:
    return NEXUS_NOT_SUPPORTED;
}

void NEXUS_Platform_GetClientConfiguration( NEXUS_ClientConfiguration *pSettings )
{
    *pSettings = NEXUS_Platform_P_State.init_data.config;
}

void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle)
{
    /* stop callbacks that are local */
    NEXUS_Base_P_StopCallbacks(interfaceHandle);

    /* stop callbacks in server and in flight */
    NEXUS_P_Client_StopCallbacks(g_client, interfaceHandle);
}

void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
{
    /* start callbacks in server */
    NEXUS_P_Client_StartCallbacks(g_client, interfaceHandle);

    /* start callbacks that are local */
    NEXUS_Base_P_StartCallbacks(interfaceHandle);
}

/* including the c code requires less refactoring of internal apis */
#include "../linuxuser.proxy/nexus_platform_proxy_local.c"
