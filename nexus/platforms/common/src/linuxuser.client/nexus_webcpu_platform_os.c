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
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_core_utils.h"
#include "bkni.h"
#include "bcm_driver.h"
#include "bdbg_log.h"
#include "nexus_core_init.h"
#include "nexus_graphics2d_init.h"
#include "bint.h"
#include "bmem.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>

BDBG_MODULE(nexus_platform_os);

#define BDBG_MSG_TRACE(X) /* BDBG_WRN(X) */

#ifdef B_REFSW_ANDROID
#define THREAD_STACK_SIZE (16*1024)
#else
#define THREAD_STACK_SIZE (8*1024)
#endif

static BMEM_Heap_Handle NEXUS_P_CreateBMEMHeap(NEXUS_HeapHandle heap);
static void NEXUS_P_DestroyBMEMHeap(BMEM_Heap_Handle pheap);

/* simulate core's private interface */
const NEXUS_Core_MagnumHandles *g_NEXUS_pCoreHandles;
NEXUS_Core_MagnumHandles g_NEXUS_coreHandles;

int g_NEXUS_driverFd = -1;

#include "bchp_common.h"
#include "bchp_webhif_l1_mask.h" /* TEMP. see below. */

static struct webcpu_state {
    void *regaddr;
    unsigned reglength;
    int mem_fd;
    BLST_S_HEAD(bint_callback_list, BINT_P_Callback) callbacks;
    struct {
        pthread_t thread;
        bool done;
    } isr;

    BLST_S_HEAD(heaplist, BMEM_P_Heap) heaps;

    uint32_t L1Mask[NEXUS_NUM_L1_REGISTERS]; /* L1's serviced by local BINT */
} g_webcpu;

static void *NEXUS_Platform_P_IsrTask(void *pParam)
{
    tbcm_linux_dd_interrupt isrData;
    struct webcpu_state *state = &g_webcpu;
    unsigned i;

    BSTD_UNUSED(pParam);

    BKNI_Memset(&isrData, 0, sizeof(isrData));
    isrData.timeout = 10000;

    while (!g_webcpu.isr.done) {
        BKNI_EnterCriticalSection();
        for (i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
            if (state->L1Mask[i]) break;
        }
        if (i == NEXUS_NUM_L1_REGISTERS) {
            struct timespec ts;

            BKNI_LeaveCriticalSection();
            ts.tv_sec = 0;
            ts.tv_nsec = 1000*1000*1;
            nanosleep(&ts, NULL);
        }
        else
        {
            for (i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
                isrData.interruptmask[i] = state->L1Mask[i];
                isrData.interruptstatus[i] = 0;
                BDBG_MSG_TRACE(("request %d: %x", i, isrData.interruptmask[i]));
            }
            BKNI_LeaveCriticalSection();

            ioctl(g_NEXUS_driverFd, BRCM_IOCTL_WAIT_FOR_INTERRUPTS, &isrData);

            BKNI_EnterCriticalSection();
            for (i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
                unsigned j;
                for (j=0;j<32;j++) {
                    if (isrData.interruptstatus[i] & (1 << j)) {
                        BDBG_MSG(("client L1 %d", i*32 + j));
                        BINT_Isr(g_pCoreHandles->bint, i*32 + j);
                    }
                }
            }
            BKNI_LeaveCriticalSection();
        }
    }
    return NULL;
}

/* copied from linuxuser's NEXUS_Platform_P_InitOS */
static NEXUS_Error NEXUS_Platform_P_StartInterruptThread(void)
{
    int rc;
    pthread_attr_t threadAttr;
    struct sched_param schedParam;

    /* Launch interrupt thread */
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    if (!NEXUS_GetEnv("not_realtime_isr")) {
        pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
        pthread_attr_getschedparam(&threadAttr, &schedParam);
        schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
        pthread_attr_setschedparam(&threadAttr, &schedParam);
    }
    pthread_attr_setstacksize(&threadAttr, THREAD_STACK_SIZE);
    BDBG_MSG(("Launching ISR task"));
    rc = pthread_create(&g_webcpu.isr.thread, &threadAttr, NEXUS_Platform_P_IsrTask, NULL);
    if ( rc )
    {
        BDBG_ERR(("Unable to create ISR task"));
        BERR_TRACE(BERR_OS_ERROR);
        goto err_thread;
    }
    if (!NEXUS_GetEnv("not_realtime_isr")) {
        pthread_setschedparam(g_webcpu.isr.thread, SCHED_FIFO, &schedParam);
    }

    /* Success */
    return BERR_SUCCESS;

err_thread:
    return BERR_OS_ERROR;
}

/* copied from linuxuser's NEXUS_Platform_P_UninitOS */
static NEXUS_Error NEXUS_Platform_P_StopInterruptThread(void)
{
    g_webcpu.isr.done = true;
    pthread_join(g_webcpu.isr.thread, NULL);
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_InitWebCpuServer(void)
{
    NEXUS_ModuleHandle module;
    NEXUS_Graphics2DModuleSettings graphics2dSettings;
    unsigned base, length;
    void *addr;
    NEXUS_Error rc;
    NEXUS_ClientConfiguration platformConfig;
    unsigned i;
    const BINT_Settings *intr_cfg;

    g_webcpu.mem_fd = open("/dev/mem", O_SYNC|O_RDWR);
    if (g_webcpu.mem_fd == -1) {rc=BERR_TRACE(BERR_OS_ERROR); goto open_error;}

    /* mmap registers in uncached address space */
    base = BCHP_PHYSICAL_OFFSET;
    length = base - BCHP_REGISTER_END;
    addr = mmap64(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, g_webcpu.mem_fd, base);
    BDBG_MSG(("reg mmap offset=%x=>%x size=%d fd=%d errno=%d", base, addr, length, g_webcpu.mem_fd, (addr==MAP_FAILED) ? errno : 0));
    if (addr==MAP_FAILED) {rc=BERR_TRACE(BERR_OS_ERROR); goto mmap_error;}
    g_webcpu.regaddr = addr;
    g_webcpu.reglength = length;

    /* Open user-mode driver */
    g_NEXUS_driverFd = open("/dev/brcm0", O_RDWR);
    if ( g_NEXUS_driverFd < 0 )
    {
        BDBG_ERR(("Unable to open user-mode driver"));
        /* give message which points to solution */
        switch (errno)
        {
        case ENXIO:
            BDBG_ERR(("bcmdriver has not been installed. Are you running the nexus script?"));
            break;
        case ENOENT:
            BDBG_ERR(("/dev/brcm0 does not exist. Are you running the nexus script?"));
            break;
        default:
            BDBG_ERR(("/dev/brcm0 error: %d", errno));
            break;
        }
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto bcmdriver_open_error;
    }
    if ( ioctl(g_NEXUS_driverFd, BRCM_IOCTL_INT_RESET, 0) )
    {
        BDBG_ERR(("cannot reset interrupts. another instance of nexus is probably running."));
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto bcmdriver_reset_error;
    }

    /* populate as much of g_pCoreHandles as is required, but no more. do not use the Core module. */
    BKNI_Memset(&g_NEXUS_coreHandles, 0, sizeof(g_NEXUS_coreHandles));
    g_NEXUS_pCoreHandles = &g_NEXUS_coreHandles;

    BREG_Open(&g_NEXUS_coreHandles.reg, g_webcpu.regaddr, (unsigned)g_webcpu.regaddr+g_webcpu.reglength);

    intr_cfg = BINT_GETSETTINGS();
    BDBG_ASSERT(intr_cfg);
    g_NEXUS_coreHandles.bint_map = intr_cfg->pIntMap;
    rc = BINT_Open(&g_NEXUS_coreHandles.bint, g_NEXUS_coreHandles.reg, intr_cfg, NULL);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto bint_error; }

    /* get L1's from BINT */
    {
        uint32_t bintL1BitMask[BINT_MAX_INTC_SIZE];
        unsigned i;
        BINT_GetL1BitMask(g_NEXUS_coreHandles.bint, bintL1BitMask);
        for (i=0; i<BINT_MAX_INTC_SIZE && i<NEXUS_NUM_L1_REGISTERS; i++) {
            g_webcpu.L1Mask[i] = bintL1BitMask[i];
        }
    }

    /* TEMP - enable M2MC1 L1 on core1. linux must do this. */
#ifndef BCHP_WEBHIF_L1_MASK_W3
#define BCHP_WEBHIF_L1_MASK_W3 BCHP_WEBHIF_L1_MASK_INTR_W3_MASK
#endif
    BREG_Write32(g_NEXUS_coreHandles.reg, BCHP_WEBHIF_L1_MASK_W3, 0x3);

    NEXUS_Platform_GetClientConfiguration(&platformConfig);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        g_NEXUS_coreHandles.heap[i].nexus = platformConfig.heap[i];
        if (platformConfig.heap[i]) {
            g_NEXUS_coreHandles.heap[i].mem = NEXUS_P_CreateBMEMHeap(platformConfig.heap[i]);
        }
    }

    rc = NEXUS_Platform_P_StartInterruptThread();
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto startinterrupts_error; }

    NEXUS_Graphics2DModule_GetDefaultSettings(&graphics2dSettings);
    module = NEXUS_Graphics2DModule_Init(&graphics2dSettings);
    if (!module) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto graphics2d_error; }

    return 0;

graphics2d_error:
    NEXUS_Platform_P_StopInterruptThread();
startinterrupts_error:
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_NEXUS_coreHandles.heap[i].mem) {
            NEXUS_P_DestroyBMEMHeap(g_NEXUS_coreHandles.heap[i].mem);
        }
    }
    BINT_Close(g_NEXUS_coreHandles.bint);
bint_error:
    BREG_Close(g_NEXUS_coreHandles.reg);
    munmap(g_webcpu.regaddr, g_webcpu.reglength);
bcmdriver_reset_error:
    close(g_NEXUS_driverFd);
    g_NEXUS_driverFd = -1;
bcmdriver_open_error:
mmap_error:
    close(g_webcpu.mem_fd);
open_error:
    return rc;
}

void NEXUS_Platform_P_UninitWebCpuServer(void)
{
    unsigned i;

    NEXUS_Graphics2DModule_Uninit();

    NEXUS_Platform_P_StopInterruptThread();

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_NEXUS_coreHandles.heap[i].mem) {
            NEXUS_P_DestroyBMEMHeap(g_NEXUS_coreHandles.heap[i].mem);
        }
    }

    BINT_Close(g_NEXUS_coreHandles.bint);
    BREG_Close(g_NEXUS_coreHandles.reg);
    munmap(g_webcpu.regaddr, g_webcpu.reglength);
    close(g_NEXUS_driverFd);
    close(g_webcpu.mem_fd);
}

/**
bmem.h shim

the BMEM stub allows calls from GRC to be routed back to the main nexus allocator
this allows each client to still have its own heap and be isolated from other clients
**/

BDBG_OBJECT_ID(BMEM_P_Heap);
struct BMEM_P_Heap
{
    BLST_S_ENTRY(BMEM_P_Heap) link;
    BDBG_OBJECT(BMEM_P_Heap)
    NEXUS_HeapHandle heap;
};

static BMEM_Heap_Handle NEXUS_P_CreateBMEMHeap(NEXUS_HeapHandle heap)
{
    BMEM_Heap_Handle pheap;
    pheap = BKNI_Malloc(sizeof(*pheap));
    if (!pheap) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pheap, 0, sizeof(*pheap));
    BDBG_OBJECT_SET(pheap, BMEM_P_Heap);
    pheap->heap = heap;
    BLST_S_INSERT_HEAD(&g_webcpu.heaps, pheap, link);
    return pheap;
}

static void NEXUS_P_DestroyBMEMHeap(BMEM_Heap_Handle pheap)
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    BLST_S_REMOVE(&g_webcpu.heaps, pheap, BMEM_P_Heap, link);
    BDBG_OBJECT_DESTROY(pheap, BMEM_P_Heap);
    BKNI_Free(pheap);
}

void *BMEM_P_Heap_TagAllocAligned ( BMEM_Heap_Handle pheap, size_t ulSize, unsigned int uiAlignment, unsigned int Boundary, const char* pchFile, int iLine )
{
    NEXUS_MemoryAllocationSettings settings;
    int rc;
    void *addr;

    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    BSTD_UNUSED(Boundary);
    BSTD_UNUSED(pchFile);
    BSTD_UNUSED(iLine);
    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
    settings.heap = pheap->heap;
    settings.alignment = 1;
    while (uiAlignment--) settings.alignment *= 2;
    rc = NEXUS_Memory_Allocate(ulSize, &settings, &addr);
    if (rc) {BERR_TRACE(rc); return NULL;}
    
    /* convert to uncached addr */
    return NEXUS_OffsetToUncachedAddr(NEXUS_AddrToOffset(addr));
}

BERR_Code BMEM_Heap_Free ( BMEM_Heap_Handle pheap, void *Address )
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    NEXUS_Memory_Free(NEXUS_OffsetToCachedAddr(NEXUS_AddrToOffset(Address)));
    return 0;
}

BERR_Code BMEM_Heap_FlushCache ( BMEM_Heap_Handle pheap, void *pvCachedAddress, size_t size )
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    NEXUS_FlushCache(pvCachedAddress, size);
    return 0;
}

BERR_Code BMEM_Heap_ConvertAddressToOffset ( BMEM_Heap_Handle pheap, void *pvAddress, uint32_t *pulOffset )
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    *pulOffset = NEXUS_AddrToOffset(pvAddress);
    return *pulOffset?0:BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

BERR_Code BMEM_Heap_ConvertAddressToCached ( BMEM_Heap_Handle pheap, void *pvAddress, void **ppvCachedAddress )
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    *ppvCachedAddress = NEXUS_OffsetToCachedAddr(NEXUS_AddrToOffset(pvAddress));
    return *ppvCachedAddress?0:BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

BERR_Code BMEM_Heap_ConvertOffsetToAddress( BMEM_Heap_Handle pheap, uint32_t ulOffset, void **ppvAddress )
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
    *ppvAddress = NEXUS_OffsetToUncachedAddr(ulOffset);
    return *ppvAddress?0:BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

BMEM_Heap_Handle NEXUS_Heap_GetMemHandle(NEXUS_HeapHandle heap)
{
    BMEM_Heap_Handle pheap;
    for (pheap = BLST_S_FIRST(&g_webcpu.heaps); pheap; pheap = BLST_S_NEXT(pheap, link)) {
        BDBG_OBJECT_ASSERT(pheap, BMEM_P_Heap);
        if (pheap->heap == heap) {
            return pheap;
        }
    }
    return NULL;
}

bool NEXUS_P_CpuAccessibleHeap( NEXUS_HeapHandle heap )
{
    NEXUS_MemoryStatus status;
    int rc;
    rc = NEXUS_Heap_GetStatus(heap, &status);
    /* can't rely on status.memoryType because webcpu client may force eFull mapping.
    instead, just try converting the base offset */
    return !rc && NEXUS_OffsetToUncachedAddr(status.offset) && NEXUS_OffsetToCachedAddr(status.offset);
}

NEXUS_Error NEXUS_Surface_InitPlaneAndPaletteOffset_priv( NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset)
{
    return NEXUS_Surface_InitPlaneAndPaletteOffset(surface, pPlane, pPaletteOffset);
}

void NEXUS_Surface_UnlockPlaneAndPalette_priv( NEXUS_SurfaceHandle surface)
{
    NEXUS_Surface_UnlockPlaneAndPalette(surface);
}

#include "bchp_pwr.h"
BERR_Code BCHP_PWR_AcquireResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    return 0;
}

BERR_Code BCHP_PWR_ReleaseResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    return 0;
}

NEXUS_Addr NEXUS_MemoryBlock_GetOffset_priv(NEXUS_MemoryBlockHandle block, unsigned blockOffset)
{
    NEXUS_Addr addr;
    if (!NEXUS_MemoryBlock_LockOffset(block, &addr)) {
        return addr + blockOffset;
    }
    else {
        return 0;
    }
}

#include "client/nexus_client_prologue.h"
BERR_Code nexus_client_graphics2d_init(NEXUS_P_ClientModuleHandle module)
{
    BSTD_UNUSED(module);
    return 0;
}

void nexus_client_graphics2d_uninit(void)
{
}

/**
btmr.h shim

called by bint, but only used for stats
**/

BERR_Code BTMR_CreateTimer_tagged(BTMR_Handle device, BTMR_TimerHandle *phTimer, const BTMR_TimerSettings *pSettings, const char *file, int line)
{
    BSTD_UNUSED(device);
    *phTimer = 0;
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
BERR_Code BTMR_DestroyTimer_tagged(BTMR_TimerHandle timer, const char*file, int line)
{
    BSTD_UNUSED(timer);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    return 0;
}
unsigned BTMR_ReadTimerMax(void)
{
    return 0;
}
BERR_Code BTMR_ReadTimer_isr(BTMR_TimerHandle timer, unsigned *pValue)
{
    BSTD_UNUSED(timer);
    *pValue = 0;
    return 0;
}
