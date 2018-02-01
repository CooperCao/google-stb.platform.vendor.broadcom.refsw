/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nexus_graphics2d_module.h"
#include "nexus_graphics2d_impl.h"
#include "nexus_power_management.h"
#include "nexus_client_resources.h"
#include "priv/nexus_surface_priv.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif
#include "bchp_common.h"
#include "bchp_pwr.h"

BDBG_MODULE(nexus_graphics2d);
BTRC_MODULE(nexus_graphics2d_blit, ENABLE);

NEXUS_ModuleHandle g_NEXUS_graphics2DModule;
struct Graphics2DData g_NEXUS_graphics2DData;

#define BDBG_PRINT_OP(x)    /* BDBG_MSG(x) */

#ifdef BCHP_MM_M2MC0_REG_START
#define NEXUS_NUM_MIPMAP_ENGINES               (1)
#else
#define NEXUS_NUM_MIPMAP_ENGINES               (0)
#endif

#define NEXUS_NUM_GRAPHICS2D_CORES (NEXUS_NUM_2D_ENGINES + NEXUS_NUM_MIPMAP_ENGINES)


static struct NEXUS_Graphics2DEngine g_grcInstance[NEXUS_NUM_GRAPHICS2D_CORES];

static BERR_Code NEXUS_Graphics2D_PacketCallback_isr( BGRC_Handle grc, void *data );
static void NEXUS_Graphics2D_P_PacketAdvance( void *context );
static void nexus_graphics2d_p_checkpoint_watchdog(void *context);
static void nexus_p_check_and_switch_secure(NEXUS_Graphics2DHandle gfx);

/****************************************
* Module functions
***************/

static void NEXUS_Graphics2DModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;
    for (i=0; i<NEXUS_NUM_GRAPHICS2D_CORES; i++) {
        struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[i];
        if (engine->grc) {
            NEXUS_Graphics2DHandle gfx;
            BDBG_LOG(("Graphics2D M2MC%d:", i));
            for (gfx = BLST_S_FIRST(&engine->contexts);gfx; gfx = BLST_S_NEXT(gfx, link)) {
                BDBG_LOG(("%p: packetspace %d, checkpoint %d, packet write completes %d, function blits %d, FIFO:%u..%u(%u), %s", (void *)gfx, gfx->packetSpaceAvailableCount, gfx->checkpointCount,
                    gfx->packetWriteCompleteCount, gfx->blitCount, gfx->surfaceFifoRead, gfx->surfaceFifoWrite,gfx->openSettings.maxOperations,
                    gfx->openSettings.secure?"secure":"unsecure"));
            }
        }
    }
#endif
}

void NEXUS_Graphics2DModule_GetDefaultInternalSettings(NEXUS_Graphics2DModuleInternalSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void NEXUS_Graphics2DModule_GetDefaultSettings(NEXUS_Graphics2DModuleSettings *pSettings)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_GRAPHICS2D_CORES;i++) {
        pSettings->core[i].hwFifoSize = 128*1024;
    }
}

NEXUS_ModuleHandle NEXUS_Graphics2DModule_Init(const NEXUS_Graphics2DModuleInternalSettings *pModuleSettings, const NEXUS_Graphics2DModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BDBG_ASSERT(!g_NEXUS_graphics2DModule);

    BDBG_ASSERT(pSettings);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eHigh;
    moduleSettings.dbgPrint = NEXUS_Graphics2DModule_P_Print;
    moduleSettings.dbgModules = "nexus_graphics2d";
    g_NEXUS_graphics2DModule = NEXUS_Module_Create("graphics2d", &moduleSettings);
    if (!g_NEXUS_graphics2DModule) {
        return NULL;
    }

    BKNI_Memset(g_grcInstance, 0, sizeof(g_grcInstance));
    g_NEXUS_graphics2DData.settings = *pSettings;
    g_NEXUS_graphics2DData.moduleSettings = *pModuleSettings;

    return g_NEXUS_graphics2DModule;
}

void NEXUS_Graphics2DModule_Uninit(void)
{
    unsigned i;

    NEXUS_LockModule();
    /* check if handles left open */
    for (i=0;i<NEXUS_NUM_GRAPHICS2D_CORES;i++) {
        struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[i];
        NEXUS_Graphics2DHandle gfx;
        /* coverity[use_after_free] */
        while ((gfx = BLST_S_FIRST(&engine->contexts))) {
            BDBG_WRN(("auto-close NEXUS_Graphics2D %p", (void *)gfx));
            NEXUS_Graphics2D_Close(gfx);
        }
    }

    NEXUS_UnlockModule();

    NEXUS_Module_Destroy(g_NEXUS_graphics2DModule);
    g_NEXUS_graphics2DModule = NULL;
}

/****************************************
* API functions
***************/

static NEXUS_Error NEXUS_Graphics2D_P_OpenGrc(unsigned index)
{
    BGRC_Settings grcSettings;
    int rc;
    struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[index];
    NEXUS_HeapHandle heap;

    BGRC_GetDefaultSettings(&grcSettings);
#if NEXUS_MODE_client && NEXUS_WEBCPU_core1_server
    /* force webcpu to use M2MC1, regardless of index param */
    grcSettings.ulDeviceNum = 1;
#else
    grcSettings.ulDeviceNum = index;
#endif
    if (grcSettings.ulDeviceNum >= NEXUS_NUM_GRAPHICS2D_CORES) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    grcSettings.ulPacketMemoryMax = g_NEXUS_graphics2DData.settings.core[grcSettings.ulDeviceNum].hwFifoSize;
    /* do not use pSettings->heap to open GRC. it may be from client that doesn't need packet blit. just use heap[0]. */
    heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    if (!NEXUS_P_CpuAccessibleHeap(heap)) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    engine->standby.heap = heap;
    engine->standby.grcSettings = grcSettings;
    rc = BGRC_Open(&engine->grc, g_pCoreHandles->chp, g_pCoreHandles->reg,
        NEXUS_Heap_GetMmaHandle(heap),
        g_pCoreHandles->bint, &grcSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_opengrc;}

    rc = BKNI_CreateEvent(&engine->advanceEvent);
    if (rc) {rc = BERR_TRACE(rc); goto err_createevent;}

    engine->advanceEventCallback = NEXUS_RegisterEvent(engine->advanceEvent, NEXUS_Graphics2D_P_PacketAdvance, engine);
    if (!engine->advanceEventCallback) {rc = BERR_TRACE(-1); goto err_regevent;}

#if (BCHP_CHIP!=11360)
    rc = BGRC_Packet_SetCallback(engine->grc, NEXUS_Graphics2D_PacketCallback_isr, engine);
    if (rc) {rc = BERR_TRACE(rc); goto err_setcallback;}
#endif

    return 0;

err_setcallback:
    NEXUS_UnregisterEvent(engine->advanceEventCallback);
err_regevent:
    BKNI_DestroyEvent(engine->advanceEvent);
err_createevent:
    BGRC_Close(engine->grc);
err_opengrc:
    return rc;
}

static void NEXUS_Graphics2D_P_CloseGrc(unsigned index)
{
    struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[index];

    if (engine->grc) {
        BGRC_Close(engine->grc);
        engine->grc = NULL;
    }
    if (engine->advanceEventCallback) {
        NEXUS_UnregisterEvent(engine->advanceEventCallback);
    }
    if (engine->advanceEvent) {
        BKNI_DestroyEvent(engine->advanceEvent);
    }
}

NEXUS_Graphics2DHandle NEXUS_Graphics2D_Open(unsigned index, const NEXUS_Graphics2DOpenSettings *pSettings)
{
    NEXUS_Graphics2DHandle gfx;
    BERR_Code rc;
    NEXUS_Graphics2DOpenSettings defaultOpenSettings;
    struct NEXUS_Graphics2DEngine *engine;
    BGRC_PacketContext_CreateSettings packetContextSettings;
    NEXUS_HeapHandle heap;
    unsigned surfaceFifoSize;

    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(graphics2d,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    if(!pSettings){
        NEXUS_Graphics2D_GetDefaultOpenSettings(&defaultOpenSettings);
        pSettings = &defaultOpenSettings;
    }

    BDBG_CASSERT(NEXUS_Graphics2DMode_eMax == (NEXUS_Graphics2DMode)BGRC_eMax);
    BDBG_ASSERT(pSettings->mode < NEXUS_Graphics2DMode_eMax);

    if (index == NEXUS_ANY_ID) {
        for(index=0;index<NEXUS_NUM_GRAPHICS2D_CORES;index++) {
            if(pSettings->mode == (NEXUS_Graphics2DMode)BGRC_GetMode_isrsafe(index))  break;
        }
    }
    else
    {
        if(pSettings->mode != (NEXUS_Graphics2DMode)BGRC_GetMode_isrsafe(index)) {
            BDBG_WRN(("failed to open m2mc[%d] mode %s: blit/mipmap mode unmatch", index,
                (pSettings->mode==NEXUS_Graphics2DMode_eBlitter)?"Blitter":"Mipmap"));
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL;
        }
    }

    if (index >= NEXUS_NUM_GRAPHICS2D_CORES) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if(pSettings->maxOperations == 0) { (void)BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL; }
    surfaceFifoSize = 0;
    if(pSettings->compatibleWithSurfaceCompaction) {
#if NEXUS_GRAPHICS2D_SUPPORT_SURFACE_COMPACTION
        surfaceFifoSize = pSettings->maxOperations;
#else
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); return NULL;
#endif
    }

    gfx = BKNI_Malloc(sizeof(*gfx)+sizeof(gfx->surfaceFifo)*surfaceFifoSize);
    if(!gfx) {
        NEXUS_CLIENT_RESOURCES_RELEASE(graphics2d,Count,NEXUS_ANY_ID);
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Graphics2D, gfx);
    gfx->surfaceFifoCheckpoint = 0;
    gfx->surfaceFifoRead = 0;
    gfx->surfaceFifoWrite = 0;
    gfx->settings.colorFormatType = NEXUS_ColorFormatType_eStandardFormat;
    NEXUS_CallbackDesc_Init(&gfx->settings.checkpointCallback);
    NEXUS_CallbackDesc_Init(&gfx->settings.packetSpaceAvailable);
    gfx->settings.pollingCheckpoint = false;
    gfx->settings.blockedSync = false;
    gfx->settings.completionTimeout = 0;
    gfx->index = index;


    engine = &g_grcInstance[index];
    if (!BLST_S_FIRST(&engine->contexts)) {
        rc = NEXUS_Graphics2D_P_OpenGrc(index);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
    BLST_S_INSERT_HEAD(&engine->contexts, gfx, link);
    gfx->engine = engine;
    gfx->grc = engine->grc;

    /* The nexus packet API requires that BM2MC_PACKET_PixelFormat and NEXUS_PixelFormat must match.
    There is no conversion function. These CASSERT's help ensure this. nexus_core_convert.c also have ASSERT's. */
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eA8_R8_G8_B8);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eY8 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eY8);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eCb8_Cr8 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eCb8_Cr8);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eY10 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eY10);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eCr10_Cb10 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eCr10_Cb10);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eCr10_Y010_Cb10_Y110 == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110);
    BDBG_CASSERT(BM2MC_PACKET_PixelFormat_eMax == (BM2MC_PACKET_PixelFormat)NEXUS_PixelFormat_eMax);
    BDBG_CASSERT(sizeof(BRect) == sizeof(NEXUS_Rect));

    gfx->openSettings = *pSettings;
    BGRC_Packet_GetDefaultCreateContextSettings(&packetContextSettings);
    heap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (heap) {
        /* allow null for packet buffer.
        application CPU access is only required for GetPacketBuffer, not function-blit, so server-only heap is ok.
        but driver CPU access is required in all cases. */
        if (!NEXUS_P_CpuAccessibleHeap(heap)) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        packetContextSettings.packet_buffer_heap = NEXUS_Heap_GetMmaHandle(heap);
    }
    packetContextSettings.packet_buffer_size = pSettings->packetFifoSize;
    packetContextSettings.packet_buffer_store = pSettings->packetFifoThreshold;
    packetContextSettings.private_data = gfx;
    packetContextSettings.secure = pSettings->secure;
    heap = NEXUS_P_DefaultHeap(pSettings->boundsHeap, NEXUS_DefaultHeapType_eBounds);
    if (heap) {
        NEXUS_MemoryStatus status;
        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        packetContextSettings.memory_bounds.offset = status.offset;
        packetContextSettings.memory_bounds.size = status.size;
    }
    gfx->standby.packetContextSettings = packetContextSettings;
    rc = BGRC_Packet_CreateContext( gfx->grc, &gfx->packetContext, &packetContextSettings );
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BGRC_Packet_CreateContext( gfx->grc, &gfx->functionContext, &packetContextSettings );
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BGRClib_Open(&gfx->grclib, gfx->grc, gfx->functionContext);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    gfx->checkpoint = NEXUS_TaskCallback_Create(gfx, NULL);
    if(!gfx->checkpoint) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    gfx->packetSpaceAvailable = NEXUS_TaskCallback_Create(gfx, NULL);
    if(!gfx->packetSpaceAvailable) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    /* call timer func once to kick off repeated timer */
    nexus_graphics2d_p_checkpoint_watchdog(gfx);

    return gfx;

error:
    NEXUS_Graphics2D_Close(gfx);
    return NULL;
}

#if NEXUS_GRAPHICS2D_SUPPORT_SURFACE_COMPACTION
NEXUS_Error NEXUS_Graphics2D_P_LockPlaneAndPalette(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset)
{
    if (!surface) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(!gfx->openSettings.compatibleWithSurfaceCompaction) {
        int rc;
        NEXUS_Module_Lock(g_NEXUS_graphics2DData.moduleSettings.surface);
        rc = NEXUS_Surface_InitPlaneAndPaletteOffset_priv(surface, pPlane, pPaletteOffset);
        NEXUS_Module_Unlock(g_NEXUS_graphics2DData.moduleSettings.surface);
        return rc;
    }
    for(;;) {
        NEXUS_Error rc;
        unsigned write = gfx->surfaceFifoWrite;
        unsigned read = gfx->surfaceFifoRead;
        BDBG_ASSERT(write >= read);
        BDBG_ASSERT(write >= gfx->surfaceFifoCheckpoint);
        if(write < gfx->openSettings.maxOperations) {
            gfx->surfaceFifo[write] = surface;
            rc = NEXUS_Surface_LockPlaneAndPalette(surface, pPlane, pPaletteOffset);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
            NEXUS_OBJECT_ACQUIRE(gfx, NEXUS_Surface, surface);
            gfx->surfaceFifoWrite = write+1;
            return NEXUS_SUCCESS;
        }
        if(gfx->surfaceFifoRead==0) { return BERR_TRACE(NEXUS_NOT_AVAILABLE); }
        /* instead of circular FIFO  copy data, and reset pointers */
        BDBG_MSG(("NEXUS_Graphics2D_P_LockPlaneAndPalette:%p move %u:%u -> 0", (void *)gfx, read, write-read));
        BKNI_Memmove(gfx->surfaceFifo, gfx->surfaceFifo+read, sizeof(gfx->surfaceFifo)*(write-read));
        gfx->surfaceFifoWrite = write - read;
        gfx->surfaceFifoCheckpoint -= read;
        gfx->surfaceFifoRead = 0;
    }
}

static void NEXUS_Graphics2D_P_UnlockPlaneAndPalette(NEXUS_Graphics2DHandle gfx)
{
    unsigned read;
    unsigned checkpoint;
    if(!gfx->openSettings.compatibleWithSurfaceCompaction) {
        return;
    }

    BDBG_MSG(("NEXUS_Graphics2D_P_UnlockPlaneAndPalette:%p %u->%u", (void *)gfx, gfx->surfaceFifoRead, gfx->surfaceFifoCheckpoint));

    BDBG_ASSERT(gfx->surfaceFifoRead <= gfx->surfaceFifoWrite);
    BDBG_ASSERT(gfx->surfaceFifoCheckpoint <= gfx->surfaceFifoWrite);
    NEXUS_Module_Lock(g_NEXUS_graphics2DData.moduleSettings.surface);
    checkpoint = gfx->surfaceFifoCheckpoint;
    for(read=gfx->surfaceFifoRead;read<checkpoint;read++) {
        NEXUS_SurfaceHandle surface = gfx->surfaceFifo[read];
        NEXUS_Surface_UnlockPlaneAndPalette_priv(surface);
    }
    NEXUS_Module_Unlock(g_NEXUS_graphics2DData.moduleSettings.surface);
    for(read=gfx->surfaceFifoRead;read<checkpoint;read++) {
        NEXUS_SurfaceHandle surface = gfx->surfaceFifo[read];
        NEXUS_OBJECT_RELEASE(gfx, NEXUS_Surface, surface);
    }
    gfx->surfaceFifoRead = read;
    return;
}
#else /* NEXUS_GRAPHICS2D_SUPPORT_SURFACE_COMPACTION */
#define NEXUS_Graphics2D_P_UnlockPlaneAndPalette(gfx)
#endif

static void NEXUS_Graphics2D_P_Release(NEXUS_Graphics2DHandle gfx)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Graphics2D, gfx);
    NEXUS_CLIENT_RESOURCES_RELEASE(graphics2d,Count,NEXUS_ANY_ID);
}

static void NEXUS_Graphics2D_P_Finalizer(NEXUS_Graphics2DHandle gfx)
{
    struct NEXUS_Graphics2DEngine *engine;

    NEXUS_OBJECT_ASSERT(NEXUS_Graphics2D, gfx);

    gfx->openSettings.secure = false;
    nexus_p_check_and_switch_secure(gfx);

    engine = &g_grcInstance[gfx->index];

    if (gfx->grclib) {
        BGRClib_Close(gfx->grclib);
    }
    if (gfx->functionContext) {
        BGRC_Packet_DestroyContext( gfx->grc, gfx->functionContext );
    }
    if (gfx->packetContext) {
        BGRC_Packet_DestroyContext( gfx->grc, gfx->packetContext );
    }

    if (gfx->grc) {
        BLST_S_REMOVE(&engine->contexts, gfx, NEXUS_Graphics2D, link);
        if (!BLST_S_FIRST(&engine->contexts)) {
            NEXUS_Graphics2D_P_CloseGrc(gfx->index);
        }
    }

    gfx->surfaceFifoCheckpoint = gfx->surfaceFifoWrite;
    NEXUS_Graphics2D_P_UnlockPlaneAndPalette(gfx);
    if(gfx->checkpoint) {
        NEXUS_TaskCallback_Destroy(gfx->checkpoint);
    }
    if(gfx->packetSpaceAvailable) {
        NEXUS_TaskCallback_Destroy(gfx->packetSpaceAvailable);
    }
    if (gfx->checkpointWatchdog.timer) {
        NEXUS_CancelTimer(gfx->checkpointWatchdog.timer);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Graphics2D, gfx);
    BKNI_Free(gfx);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Graphics2D, NEXUS_Graphics2D_Close);

void NEXUS_Graphics2D_GetDefaultOpenSettings(NEXUS_Graphics2DOpenSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->packetFifoSize = 128*1024;
    pSettings->packetFifoThreshold = pSettings->packetFifoSize; /* maximum throughput */
    pSettings->maxOperations = 1024;
    pSettings->compatibleWithSurfaceCompaction = false;
    return;
}

void NEXUS_Graphics2D_GetSettings(NEXUS_Graphics2DHandle gfx, NEXUS_Graphics2DSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
    *pSettings = gfx->settings;
}

NEXUS_Error NEXUS_Graphics2D_SetSettings(NEXUS_Graphics2DHandle gfx, const NEXUS_Graphics2DSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
    if (pSettings->blockedSync) {
        /* Converting your app to not used blockedSync is easy. And, you then have an opportunity to get much better performance
        by queueing multiple async blits without having the overhead of interrupts and checkpoints on each one.
        See the NEXUS_Graphics2D_Checkpoint logic in nexus/examples/graphics for sample code. */
        BDBG_ERR(("blockedSync is not supported"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    gfx->settings = *pSettings;
    NEXUS_TaskCallback_Set(gfx->checkpoint, &gfx->settings.checkpointCallback);
    NEXUS_TaskCallback_Set(gfx->packetSpaceAvailable, &gfx->settings.packetSpaceAvailable);
    return NEXUS_SUCCESS;
}

void NEXUS_Graphics2D_GetDefaultFillSettings(NEXUS_Graphics2DFillSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->colorOp = NEXUS_FillOp_eCopy;
    pSettings->alphaOp = NEXUS_FillOp_eCopy;
}

void NEXUS_Graphics2D_GetDefaultPorterDuffFillSettings(NEXUS_Graphics2DPorterDuffFillSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static NEXUS_Error
NEXUS_Graphics2D_P_FinishOperation(NEXUS_Graphics2DHandle gfx, BERR_Code rc)
{
    gfx->blitCount++;

    /* if there's a threshold, then a */
    if (gfx->openSettings.packetFifoThreshold && !gfx->checkpointWatchdog.counter) {
        gfx->checkpointWatchdog.counter = 40; /* wait between 5 and 10 seconds */
    }
    if (rc==BERR_SUCCESS) {
        return NEXUS_SUCCESS;
    }
    else if (rc == BERR_OUT_OF_SYSTEM_MEMORY || rc == BERR_OUT_OF_DEVICE_MEMORY) {
        BDBG_MSG(("Blitter SW fifo is full. Increase NEXUS_Graphics2DOpenSettings.packetBufferSize or wait for packetSpaceAvailable."));
        gfx->packetSpaceAvailableCount++;
        return NEXUS_GRAPHICS2D_QUEUE_FULL; /* not an error */
    }
    else {
        return BERR_TRACE(rc);
    }
}

/* convert pixel from surface's pixel format to universal ARGB or YCrCb which M2MC HW understands */
static uint32_t NEXUS_Graphics2D_P_ConvertPixel(NEXUS_PixelFormat format, uint32_t pixel)
{
    /* BPXL_ConvertPixel has the logic for this. It's not efficient, but we would have to duplicate BPXL logic. */
    uint32_t ulColor = 0;
    BPXL_Format srcformat;
    if (!NEXUS_P_PixelFormat_ToMagnum_isrsafe(format, &srcformat)) {
        BPXL_Format dstformat = NEXUS_PIXEL_FORMAT_IS_YCRCB(format) ? BPXL_eA8_Y8_Cb8_Cr8 : BPXL_eA8_R8_G8_B8;
        BPXL_ConvertPixel( dstformat, srcformat, pixel, (unsigned int *) &ulColor );
    }
    return ulColor;
}

NEXUS_Error NEXUS_Graphics2D_Fill(NEXUS_Graphics2DHandle gfx, const NEXUS_Graphics2DFillSettings *pSettings)
{
    BERR_Code rc;
    BM2MC_PACKET_Plane surface;
    BGRCLib_FillOp colorOp, alphaOp;
    NEXUS_Addr srcPaletteOffset;
    uint32_t color;

    BDBG_CASSERT((NEXUS_FillOp_eMax - 2) == (NEXUS_FillOp)BGRCLib_FillOp_eBlend); /* skip over NEXUS_FillOp_eUseBlendEquation */
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
    nexus_p_check_and_switch_secure(gfx);

    if (!pSettings->surface || pSettings->colorOp >= NEXUS_FillOp_eMax || pSettings->alphaOp >= NEXUS_FillOp_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (pSettings->colorOp == NEXUS_FillOp_eUseBlendEquation ||
        pSettings->alphaOp == NEXUS_FillOp_eUseBlendEquation)
    {
        NEXUS_Graphics2DBlitSettings blitSettings;
        static const NEXUS_BlendEquation g_copyEq = {NEXUS_BlendFactor_eConstantColor, NEXUS_BlendFactor_eOne, false,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};
        static const NEXUS_BlendEquation g_blendEq = {NEXUS_BlendFactor_eConstantColor, NEXUS_BlendFactor_eConstantAlpha, false,
            NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eInverseConstantAlpha, false, NEXUS_BlendFactor_eZero};
        static const NEXUS_BlendEquation g_copyAlphaEq = {NEXUS_BlendFactor_eConstantAlpha, NEXUS_BlendFactor_eOne, false,
            NEXUS_BlendFactor_eZero, NEXUS_BlendFactor_eZero, false, NEXUS_BlendFactor_eZero};

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = pSettings->surface;
        blitSettings.source.rect = pSettings->rect;
        blitSettings.dest.surface = pSettings->surface;
        blitSettings.dest.rect = pSettings->rect;
        blitSettings.output.surface = pSettings->surface;
        blitSettings.output.rect = pSettings->rect;
        blitSettings.constantColor = pSettings->color;
        switch (pSettings->colorOp) {
        case NEXUS_FillOp_eIgnore:
            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            break;
        case NEXUS_FillOp_eCopy:
            blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
            blitSettings.colorBlend = g_copyEq;
            break;
        case NEXUS_FillOp_eBlend:
            blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
            blitSettings.colorBlend = g_blendEq;
            break;
        case NEXUS_FillOp_eUseBlendEquation:
            blitSettings.colorOp = NEXUS_BlitColorOp_eUseBlendEquation;
            blitSettings.colorBlend = pSettings->colorBlend;
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        switch (pSettings->alphaOp) {
        case NEXUS_FillOp_eIgnore:
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
            break;
        case NEXUS_FillOp_eCopy:
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
            blitSettings.alphaBlend = g_copyAlphaEq;
            break;
        case NEXUS_FillOp_eUseBlendEquation:
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eUseBlendEquation;
            blitSettings.alphaBlend = pSettings->alphaBlend;
            break;
        case NEXUS_FillOp_eBlend:
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        return NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    }

    BTRC_TRACE(nexus_graphics2d_blit, START);
    colorOp = pSettings->colorOp;
    alphaOp = pSettings->alphaOp;

    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->surface, &surface, &srcPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_PRINT_OP(("fill " BDBG_UINT64_FMT "(%u,%u,%u,%u) pixel=%#x %u,%u", BDBG_UINT64_ARG(surface.address),
        pSettings->rect.x, pSettings->rect.y, pSettings->rect.width, pSettings->rect.height,
        pSettings->color, pSettings->colorOp, pSettings->alphaOp));

    if (gfx->settings.colorFormatType == NEXUS_ColorFormatType_eSameAsSurface || srcPaletteOffset) {
        color = NEXUS_Graphics2D_P_ConvertPixel(surface.format, pSettings->color);
    }
    else {
        color = pSettings->color;
    }

    rc = BGRClib_Blended_Fill(gfx->grclib,
        &surface,
        color,
        (const BRect *)(void*)&pSettings->rect,
        colorOp,
        alphaOp);
    BTRC_TRACE(nexus_graphics2d_blit, STOP);
    return NEXUS_Graphics2D_P_FinishOperation(gfx, rc);
}

NEXUS_Error NEXUS_Graphics2D_PorterDuffFill(NEXUS_Graphics2DHandle gfx, const NEXUS_Graphics2DPorterDuffFillSettings *pSettings)
{
    BERR_Code rc;
    BM2MC_PACKET_Plane surface;
    NEXUS_Addr srcPaletteOffset;
    uint32_t color;

    BTRC_TRACE(nexus_graphics2d_blit, START);
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
    nexus_p_check_and_switch_secure(gfx);

    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->surface, &surface, &srcPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_PRINT_OP(("Porter Duff fill " BDBG_UINT64_FMT "(%u,%u,%u,%u) pixel=%#x op=%u", BDBG_UINT64_ARG(surface.address),
        pSettings->rect.x, pSettings->rect.y, pSettings->rect.width, pSettings->rect.height,
        pSettings->color, pSettings->operation));

    if (gfx->settings.colorFormatType == NEXUS_ColorFormatType_eSameAsSurface || srcPaletteOffset) {
        color = NEXUS_Graphics2D_P_ConvertPixel(surface.format, pSettings->color);
    }
    else {
        color = pSettings->color;
    }

    rc = BGRClib_PorterDuffFill(gfx->grclib,
        (BGRCLib_PorterDuffOp)pSettings->operation,
        &surface,
        color,
        (const BRect *)(void*)&pSettings->rect);
    BTRC_TRACE(nexus_graphics2d_blit, STOP);
    return NEXUS_Graphics2D_P_FinishOperation(gfx, rc);
}

void NEXUS_Graphics2D_GetDefaultBlitSettings(NEXUS_Graphics2DBlitSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->horizontalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    pSettings->verticalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    pSettings->constantColor = 0xFF000000; /* match GRClib */
    pSettings->chromaFilter = true;
    /* don't default dest and source constant color. rarely used and user must set. */
}

/***************************************************************************
Description:
    RGB to YCbCr color matrix table.
    Taken from rockford/applications/bmetest/grc_test/bgfx_blit.c
****************************************************************************/
/* Y  = R *  0.257 + G *  0.504 + B *  0.098 + 16  */
/* Cb = R * -0.148 + G * -0.291 + B *  0.439 + 128 */
/* Cr = R *  0.439 + G * -0.368 + B * -0.071 + 128 */
static const int32_t g_NEXUS_ai32_Matrix_RGBtoYCbCr[20] =
{
    (int32_t) ( 0.257f * (1 << 10)),   /*  R factor for Y */
    (int32_t) ( 0.504f * (1 << 10)),   /*  G factor for Y */
    (int32_t) ( 0.098f * (1 << 10)),   /*  B factor for Y */
    (int32_t) 0,                       /*  A factor for Y */
    (int32_t) (16 * (1 << 10)),        /* Increment for Y */
    (int32_t) (-0.148f * (1 << 10)),   /*  R factor for Cb */
    (int32_t) (-0.291f * (1 << 10)),   /*  G factor for Cb */
    (int32_t) ( 0.439f * (1 << 10)),   /*  B factor for Cb */
    (int32_t) 0,                       /*  A factor for Cb */
    (int32_t) (128 * (1 << 10)),       /* Increment for Cb */
    (int32_t) ( 0.439f * (1 << 10)),   /*  R factor for Cr */
    (int32_t) (-0.368f * (1 << 10)),   /*  G factor for Cr */
    (int32_t) (-0.071f * (1 << 10)),   /*  B factor for Cr */
    (int32_t) 0,                       /*  A factor for Cr */
    (int32_t) (128 * (1 << 10)),       /* Increment for Cr */
    (int32_t) 0,                       /*  R factor for A */
    (int32_t) 0,                       /*  G factor for A */
    (int32_t) 0,                       /*  B factor for A */
    (int32_t) (1 << 10),               /*  A factor for A */
    (int32_t) 0,                       /* Increment for A */
};

/***************************************************************************
Description:
    YCbCr to RGB color matrix table.
    Taken from rockford/applications/bmetest/grc_test/bgfx_blit.c
****************************************************************************/
/* R = Y * 1.164 + Cr * 1.596 - 223 */
/* G = Y * 1.164 - Cr * 0.813 - Cb * 0.391 + 135 */
/* B = Y * 1.164 + Cb * 2.018 - 277 */
const int32_t g_NEXUS_ai32_Matrix_YCbCrtoRGB[20] =
{
    (int32_t) ( 1.164f * (1 << 10)),   /*  Y factor for R */
    (int32_t) 0,                       /* Cb factor for R */
    (int32_t) ( 1.596f * (1 << 10)),   /* Cr factor for R */
    (int32_t) 0,                       /*  A factor for R */
    (int32_t) (-223 * (1 << 10)),      /* Increment for R */
    (int32_t) ( 1.164f * (1 << 10)),   /*  Y factor for G */
    (int32_t) (-0.391f * (1 << 10)),   /* Cb factor for G */
    (int32_t) (-0.813f * (1 << 10)),   /* Cr factor for G */
    (int32_t) 0,                       /*  A factor for G */
    (int32_t) (134 * (1 << 10)),       /* Increment for G */
    (int32_t) ( 1.164f * (1 << 10)),   /*  Y factor for B */
    (int32_t) ( 2.018f * (1 << 10)),   /* Cb factor for B */
    (int32_t) 0,                       /* Cr factor for B */
    (int32_t) 0,                       /*  A factor for B */
    (int32_t) (-277 * (1 << 10)),      /* Increment for B */
    (int32_t) 0,                       /*  Y factor for A */
    (int32_t) 0,                       /* Cb factor for A */
    (int32_t) 0,                       /* Cr factor for A */
    (int32_t) (1 << 10),               /*  A factor for A */
    (int32_t) 0,                       /* Increment for A */
};

static void NEXUS_P_CopyBlendEq(BGRClib_BlendEquation *dst, const void *src)
{
    *dst = *((BGRClib_BlendEquation *)src);
    return;
}
#define NEXUS_P_COPYBLENDEQ(dst,src) NEXUS_P_CopyBlendEq(dst, src)

NEXUS_Error NEXUS_Graphics2D_Blit(NEXUS_Graphics2DHandle gfx, const NEXUS_Graphics2DBlitSettings*  pSettings)
{
    BERR_Code rc;
    BGRClib_BlitColorKeyParams *pColorKeyParams = NULL;
    BGRClib_BlitMatrixParams *pMatrixParams = NULL;
    BGRClib_BlitPatternParams *pPatternParams = NULL;
    BGRClib_BlitScalingControlParams *pScalingControlParams = NULL;
    BM2MC_PACKET_Plane src, dst, out;
    NEXUS_Addr srcPaletteOffset, dstPaletteOffset = 0, outPaletteOffset;
    BGRCLib_BlitColorOp colorOp;
    BGRCLib_BlitAlphaOp alphaOp;
    uint32_t constantColor = pSettings->constantColor;

    BTRC_TRACE(nexus_graphics2d_blit, START);

#if 0
    BKNI_Printf("NEXUS_Graphics2D_Blit\n"
        "  source %p, %d,%d,%d,%d\n"
        "  dest   %p, %d,%d,%d,%d\n"
        "  output %p, %d,%d,%d,%d\n"
        "  colorop %d, alphaop %d, constant %08x\n"
        "  enabled %d %d %d %d\n"
        "  horiz %d, vert %d\n"
        "  enabled %d %d %d %d %d %d\n",
        pSettings->source.surface, pSettings->source.rect.x, pSettings->source.rect.y, pSettings->source.rect.width, pSettings->source.rect.height,
        pSettings->dest.surface, pSettings->dest.rect.x, pSettings->dest.rect.y, pSettings->dest.rect.width, pSettings->dest.rect.height,
        pSettings->output.surface, pSettings->output.rect.x, pSettings->output.rect.y, pSettings->output.rect.width, pSettings->output.rect.height,
        pSettings->colorOp, pSettings->alphaOp, pSettings->constantColor,
        pSettings->colorKey.source.enabled, pSettings->colorKey.dest.enabled, pSettings->conversionMatrixEnabled, pSettings->patternSettingsEnabled,
        pSettings->horizontalFilter, pSettings->verticalFilter,
        pSettings->scalingControlSettingsEnabled, pSettings->mirrorSourceVertically, pSettings->mirrorSourceHorizontally, pSettings->mirrorDestVertically, pSettings->mirrorDestHorizontally, pSettings->alphaPremultiplySourceEnabled, pSettings->chromaFilter);
#endif

    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);

    nexus_p_check_and_switch_secure(gfx);

    /* copy surfaces */
    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->source.surface, &src, &srcPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    if (pSettings->dest.surface) {
        rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->dest.surface, &dst, &dstPaletteOffset);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    }
    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->output.surface, &out, &outPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    if (!srcPaletteOffset && outPaletteOffset) {
        BDBG_ERR(("Cannot blit from non-palette to palette."));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* prep blitParams here so we can start setting it up in the switch statements */
    if (srcPaletteOffset) {
        BGRClib_GetDefaultPaletteBlitParams(&gfx->blitData.blitParams);
    } else {
        BGRClib_GetDefaultBlitParams(&gfx->blitData.blitParams);
    }

    if (out.format == BM2MC_PACKET_PixelFormat_eUIF_R8_G8_B8_A8)
    {
       NEXUS_SurfaceCreateSettings createSettings;
       NEXUS_Surface_GetCreateSettings(pSettings->output.surface, &createSettings);
       gfx->blitData.blitParams.miplevel = createSettings.mipLevel;
    }
    switch (pSettings->colorOp) {
    case NEXUS_BlitColorOp_eCopySource:
        colorOp = BGRCLib_BlitColorOp_eCopySource;
        break;
    case NEXUS_BlitColorOp_eUseConstantAlpha:
        /* this does not require dest. if no dest, source will be multiplied (i.e. reduced) with the constant alpha */
        colorOp = BGRCLib_BlitColorOp_eUseConstantAlpha;
        break;
    case NEXUS_BlitColorOp_eUseSourceAlpha:
        /* this does not require dest. if no dest, source will be multiplied (i.e. reduced) with its own alpha (i.e. self reducing) */
        colorOp = BGRCLib_BlitColorOp_eUseSourceAlpha;
        break;
    case NEXUS_BlitColorOp_eUseDestAlpha:
        if (!pSettings->dest.surface) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        colorOp = BGRCLib_BlitColorOp_eUseDestAlpha;
        break;
    case NEXUS_BlitColorOp_eSelectPaletteWithColorkey:
        colorOp = BGRCLib_BlitColorOp_eSelectPaletteWithColorkey;
        break;
    case NEXUS_BlitColorOp_eAdd:
        if (!pSettings->dest.surface) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        colorOp = BGRCLib_BlitColorOp_eAdd;
        break;
    case NEXUS_BlitColorOp_eUseBlendEquation:
        colorOp = BGRCLib_BlitColorOp_eUseBlendFactors;
        BDBG_CASSERT((NEXUS_BlendFactor_eMax-1) == (NEXUS_BlendFactor)BGRC_Blend_Source_eInverseConstantAlpha);
        BDBG_CASSERT(sizeof(gfx->blitData.blitParams.colorBlend) == sizeof(pSettings->colorBlend));
        NEXUS_P_COPYBLENDEQ(&gfx->blitData.blitParams.colorBlend, &pSettings->colorBlend);
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    switch (pSettings->alphaOp) {
    case NEXUS_BlitAlphaOp_eCopySource:
        alphaOp = BGRCLib_BlitAlphaOp_eCopySource;
        {
            /* loop up pixel format info, but only when needed */
            const NEXUS_PixelFormatConvertInfo *pSourcePixelFormatInfo = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(src.format);
            if (!pSourcePixelFormatInfo->info.alphaPerPixel && !srcPaletteOffset) {
                if (pSettings->colorOp == NEXUS_BlitColorOp_eUseConstantAlpha) {
                    BDBG_ERR(("Incompatible colorOp and alphaOp settings for this blit. Use NEXUS_BlitAlphaOp_eCopyDest or eCopyConstant."));
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
                alphaOp = BGRCLib_BlitAlphaOp_eUseConstAlpha;
                constantColor = 0xFF000000;
            }
        }
        break;
    case NEXUS_BlitAlphaOp_eCopyDest:
        if (!pSettings->dest.surface) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        alphaOp = BGRCLib_BlitAlphaOp_eUseDestAlpha;
        {
            /* loop up pixel format info, but only when needed */
            const NEXUS_PixelFormatConvertInfo *pDestPixelFormatInfo = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(dst.format);
            if (!pDestPixelFormatInfo->info.alphaPerPixel && !dstPaletteOffset) {
                if (pSettings->colorOp == NEXUS_BlitColorOp_eUseConstantAlpha) {
                    BDBG_ERR(("Incompatible colorOp and alphaOp settings for this blit. Use NEXUS_BlitAlphaOp_eCopySource or eCopyConstant."));
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
                alphaOp = BGRCLib_BlitAlphaOp_eUseConstAlpha;
                constantColor = 0xFF000000;
            }
        }
        break;
    case NEXUS_BlitAlphaOp_eCopyConstant:
        alphaOp = BGRCLib_BlitAlphaOp_eUseConstAlpha;
        break;
    case NEXUS_BlitAlphaOp_eCombine:
        alphaOp = BGRCLib_BlitAlphaOp_eUseCombinedAlpha;
        break;
    case NEXUS_BlitAlphaOp_eEmulateTransparentVideo:
        alphaOp = BGRCLib_BlitAlphaOp_eEmulateTransparentVideo;
        break;
    case NEXUS_BlitAlphaOp_eAdd:
        alphaOp = BGRCLib_BlitAlphaOp_eAdd;
        break;
    case NEXUS_BlitAlphaOp_eUseBlendEquation:
        alphaOp = BGRCLib_BlitAlphaOp_eUseBlendFactors;
        BDBG_CASSERT(sizeof(gfx->blitData.blitParams.alphaBlend) == sizeof(pSettings->alphaBlend));
        NEXUS_P_COPYBLENDEQ(&gfx->blitData.blitParams.alphaBlend, &pSettings->alphaBlend);
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    gfx->blitData.blitParams.srcSurface = &src;
    gfx->blitData.blitParams.srcRect = (const BRect *)(void*)&pSettings->source.rect;
    gfx->blitData.blitParams.srcPaletteOffset = srcPaletteOffset;
    gfx->blitData.blitParams.dstSurface = pSettings->dest.surface?&dst:NULL;
    gfx->blitData.blitParams.dstRect = (const BRect *)(void*)&pSettings->dest.rect;
    gfx->blitData.blitParams.outSurface = &out;
    gfx->blitData.blitParams.outRect = (const BRect *)(void*)&pSettings->output.rect;
    gfx->blitData.blitParams.colorOp = colorOp;
    gfx->blitData.blitParams.alphaOp = alphaOp;
    gfx->blitData.blitParams.constantColor = constantColor; /* blend constant color */
    if (pSettings->useDestAndSourceConstantColor) {
        gfx->blitData.blitParams.destConstantColor = pSettings->destConstantColor;
        gfx->blitData.blitParams.sourceConstantColor = pSettings->sourceConstantColor;
    }
    else {
        gfx->blitData.blitParams.destConstantColor = constantColor;
        gfx->blitData.blitParams.sourceConstantColor = constantColor;
    }
    gfx->blitData.blitParams.colorKeySelect = BGRC_Output_ColorKeySelection_eTakeBlend;

    /* copy mirror flags */
    gfx->blitData.blitParams.mirrorSrcVertically = pSettings->mirrorSourceVertically;
    gfx->blitData.blitParams.mirrorSrcHorizontally = pSettings->mirrorSourceHorizontally;
    gfx->blitData.blitParams.mirrorDstVertically = pSettings->mirrorDestVertically;
    gfx->blitData.blitParams.mirrorDstHorizontally = pSettings->mirrorDestHorizontally;
    gfx->blitData.blitParams.mirrorOutVertically = pSettings->mirrorOutputVertically;
    gfx->blitData.blitParams.mirrorOutHorizontally = pSettings->mirrorOutputHorizontally;
#if 0
    /* auto-mirror if the output overlaps the source. */
    if (pSettings->source.surface == pSettings->output.surface) {
        if (pSettings->output.rect.y > pSettings->source.rect.y && !gfx->blitData.blitParams.mirrorSrcVertically
                                                                && !gfx->blitData.blitParams.mirrorDstVertically
                                                                && !gfx->blitData.blitParams.mirrorOutVertically) {
            gfx->blitData.blitParams.mirrorSrcVertically = true;
            gfx->blitData.blitParams.mirrorDstVertically = true;
            gfx->blitData.blitParams.mirrorOutVertically = true;
        }
        if (pSettings->output.rect.x > pSettings->source.rect.x && !gfx->blitData.blitParams.mirrorSrcHorizontally
                                                                && !gfx->blitData.blitParams.mirrorDstHorizontally
                                                                && !gfx->blitData.blitParams.mirrorOutHorizontally) {
            gfx->blitData.blitParams.mirrorSrcHorizontally = true;
            gfx->blitData.blitParams.mirrorDstHorizontally = true;
            gfx->blitData.blitParams.mirrorOutHorizontally = true;
        }
    }
#endif

    gfx->blitData.blitParams.sourceFilterOrder = pSettings->sourceFilterOrder;

    gfx->blitData.blitParams.srcAlphaPremult = pSettings->alphaPremultiplySourceEnabled;

    if (pSettings->colorKey.source.enabled || pSettings->colorKey.dest.enabled) {
        BGRClib_GetDefaultColorKeyParams(&gfx->blitData.colorKeyParams);
        /* Use the output of the blend block instead of the source if any form of blending is required */
        if ( (pSettings->colorOp != NEXUS_BlitColorOp_eCopySource &&
              pSettings->colorOp != NEXUS_BlitColorOp_eSelectPaletteWithColorkey) ||
              pSettings->alphaOp != NEXUS_BlitAlphaOp_eCopySource ) {
            gfx->blitData.colorKeyParams.cksOnlyDstColorKeyed = BGRC_Output_ColorKeySelection_eTakeBlend;
        }
        gfx->blitData.colorKeyParams.enableColorKey = pSettings->colorKey.source.enabled;
        gfx->blitData.colorKeyParams.enableDstColorKey = pSettings->colorKey.dest.enabled;
        /* GRClib's colorKeySelect is derived from other params */
        if (pSettings->colorKey.dest.enabled) {
            /* if dest.enabled then we eTakeDestination. if there's a dest surface, we take that. if not, we take dest constant color. */
            gfx->blitData.blitParams.colorKeySelect = BGRC_Output_ColorKeySelection_eTakeDestination;
        }

        gfx->blitData.colorKeyParams.colorKeyLower = pSettings->colorKey.source.lower;
        gfx->blitData.colorKeyParams.colorKeyUpper = pSettings->colorKey.source.upper;
        gfx->blitData.colorKeyParams.colorKeyMask = pSettings->colorKey.source.mask;
        gfx->blitData.colorKeyParams.colorKeyReplace = pSettings->colorKey.source.replace;
        gfx->blitData.colorKeyParams.colorKeyRplMask = pSettings->colorKey.source.replaceMask;
        gfx->blitData.colorKeyParams.dstColorKeyLower = pSettings->colorKey.dest.lower;
        gfx->blitData.colorKeyParams.dstColorKeyUpper = pSettings->colorKey.dest.upper;
        gfx->blitData.colorKeyParams.dstColorKeyMask = pSettings->colorKey.dest.mask;
        gfx->blitData.colorKeyParams.dstColorKeyReplace = pSettings->colorKey.dest.replace;
        gfx->blitData.colorKeyParams.dstColorKeyRplMask = pSettings->colorKey.dest.replaceMask;

        pColorKeyParams = &gfx->blitData.colorKeyParams;
    }

    if (pSettings->conversionMatrixEnabled) {
        gfx->blitData.matrixParams.conversionMatrix = (int32_t *)pSettings->conversionMatrix.coeffMatrix;
        gfx->blitData.matrixParams.matrixShift = pSettings->conversionMatrix.shift;
        pMatrixParams = &gfx->blitData.matrixParams;
    }
    else {
        bool sourceYCrCb, outputYCrCb;

        if (srcPaletteOffset) {
            NEXUS_SurfaceCreateSettings createSettings;
            NEXUS_Surface_GetCreateSettings(pSettings->source.surface, &createSettings);
            sourceYCrCb = NEXUS_PIXEL_FORMAT_IS_YCRCB(createSettings.palettePixelFormat);
        }
        else {
            sourceYCrCb = NEXUS_PIXEL_FORMAT_IS_YCRCB(src.format);
        }
        if (outPaletteOffset) {
            NEXUS_SurfaceCreateSettings createSettings;
            NEXUS_Surface_GetCreateSettings(pSettings->output.surface, &createSettings);
            outputYCrCb = NEXUS_PIXEL_FORMAT_IS_YCRCB(createSettings.palettePixelFormat);
        }
        else {
            outputYCrCb = NEXUS_PIXEL_FORMAT_IS_YCRCB(out.format);
        }

        /* TODO: handle both flavors of YCrCb */
        if (sourceYCrCb && !outputYCrCb) {
            BDBG_MSG(("YCrCb->RGB conversion"));
            gfx->blitData.matrixParams.conversionMatrix = (int32_t *)g_NEXUS_ai32_Matrix_YCbCrtoRGB;
            gfx->blitData.matrixParams.matrixShift = 10;
            pMatrixParams = &gfx->blitData.matrixParams;
        }
        else if (!sourceYCrCb && outputYCrCb) {
            BDBG_MSG(("RGB->YCrCb conversion"));
            gfx->blitData.matrixParams.conversionMatrix = (int32_t *)g_NEXUS_ai32_Matrix_RGBtoYCbCr;
            gfx->blitData.matrixParams.matrixShift = 10;
            pMatrixParams = &gfx->blitData.matrixParams;
        }
    }

    if (pSettings->patternSettingsEnabled) {
        gfx->blitData.blitParams.colorKeySelect = BGRC_Output_ColorKeySelection_eTakePattern;
        gfx->blitData.patternParams.ropVector = pSettings->patternSettings.ropVector;
        gfx->blitData.patternParams.pattern = (uint8_t *)pSettings->patternSettings.pattern;
        gfx->blitData.patternParams.foreColor = pSettings->patternSettings.foreColor;
        gfx->blitData.patternParams.backColor = pSettings->patternSettings.backColor;
        pPatternParams = &gfx->blitData.patternParams;
    }

    if (pSettings->scalingControlSettingsEnabled) {
        BDBG_CASSERT(sizeof(gfx->blitData.scalingControlParams) == sizeof(pSettings->scalingControlSettings));
        BKNI_Memcpy(&gfx->blitData.scalingControlParams, &pSettings->scalingControlSettings, sizeof(gfx->blitData.scalingControlParams));;
        pScalingControlParams = &gfx->blitData.scalingControlParams;
    }

    BDBG_CASSERT(NEXUS_Graphics2DFilterCoeffs_eAntiFlutterSharp == (NEXUS_Graphics2DFilterCoeffs)BGRC_FilterCoeffs_eAntiFlutterSharp);
    gfx->blitData.blitParams.horzFilter = pSettings->horizontalFilter;
    gfx->blitData.blitParams.vertFilter = pSettings->verticalFilter;

    gfx->blitData.blitParams.chromaFilter = pSettings->chromaFilter;

    if(pSettings->colorOp==NEXUS_BlitColorOp_eUseBlendEquation) {
        BDBG_PRINT_OP(("blit colorBlend a=%u b=%u %c c=%u d=%u %c e=%u", pSettings->colorBlend.a, pSettings->colorBlend.b, pSettings->colorBlend.subtract_cd?'-':'+', pSettings->colorBlend.c, pSettings->colorBlend.d, pSettings->colorBlend.subtract_e?'-':'+', pSettings->colorBlend.e));
    }
    if(pSettings->alphaOp==NEXUS_BlitAlphaOp_eUseBlendEquation) {
        BDBG_PRINT_OP(("blit alphaBlend a=%u b=%u %c c=%u d=%u %c e=%u", pSettings->alphaBlend.a, pSettings->alphaBlend.b, pSettings->alphaBlend.subtract_cd?'-':'+', pSettings->alphaBlend.c, pSettings->alphaBlend.d, pSettings->alphaBlend.subtract_e?'-':'+', pSettings->alphaBlend.e));
    }

    BDBG_PRINT_OP(("blit source=" BDBG_UINT64_FMT "(%u,%u,%u,%u) output=" BDBG_UINT64_FMT "(%u,%u,%u,%u) dest=" BDBG_UINT64_FMT "(%u,%u,%u,%u) ops=%u,%u",
        BDBG_UINT64_ARG(src.address),
        pSettings->source.rect.x, pSettings->source.rect.y, pSettings->source.rect.width, pSettings->source.rect.height,
        BDBG_UINT64_ARG(out.address),
        pSettings->output.rect.x, pSettings->output.rect.y, pSettings->output.rect.width, pSettings->output.rect.height,
        BDBG_UINT64_ARG(dst.address),
        pSettings->source.rect.x, pSettings->dest.rect.y, pSettings->dest.rect.width, pSettings->dest.rect.height,
        pSettings->colorOp, pSettings->alphaOp ));

    rc = BGRClib_Blit(gfx->grclib, &gfx->blitData.blitParams, pColorKeyParams, pMatrixParams, pPatternParams, pScalingControlParams);
    BTRC_TRACE(nexus_graphics2d_blit, STOP);
    return NEXUS_Graphics2D_P_FinishOperation(gfx, rc);
}

NEXUS_Error NEXUS_Graphics2D_FastBlit( NEXUS_Graphics2DHandle gfx,
    NEXUS_SurfaceHandle outputSurface, const NEXUS_Rect *pOutputRect, NEXUS_SurfaceHandle sourceSurface, const NEXUS_Rect *pSourceRect, NEXUS_BlitColorOp a_colorOp,
    NEXUS_BlitAlphaOp a_alphaOp, uint32_t constantColor )
{
    BERR_Code rc;
    BM2MC_PACKET_Plane src, out;
    NEXUS_Addr srcPaletteOffset, temp;
    BGRCLib_BlitColorOp colorOp;
    BGRCLib_BlitAlphaOp alphaOp;

    BTRC_TRACE(nexus_graphics2d_blit, START);
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);

    nexus_p_check_and_switch_secure(gfx);

    /* copy surfaces */
    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, sourceSurface, &src, &srcPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, outputSurface, &out, &temp);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    /* Only CopySource is supported now. In the future, we could add NEXUS_BlitColorOp_eCopyConstant. This would be different from NEXUS_Graphics2D_Fill
    because you could fill color while copying alpha. */
    switch (a_colorOp) {
    case NEXUS_BlitColorOp_eCopySource:
        colorOp = BGRCLib_BlitColorOp_eCopySource;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    switch (a_alphaOp) {
    case NEXUS_BlitAlphaOp_eCopySource:
        alphaOp = BGRCLib_BlitAlphaOp_eCopySource;
        {
            /* look up pixel format info, but only when needed */
            const NEXUS_PixelFormatConvertInfo *pSourcePixelFormatInfo = NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(src.format);
            if(pSourcePixelFormatInfo==NULL) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            if (!pSourcePixelFormatInfo->info.alphaPerPixel) {
                alphaOp = BGRCLib_BlitAlphaOp_eUseConstAlpha;
                constantColor = 0xFF000000;
            }
        }
        break;
    case NEXUS_BlitAlphaOp_eCopyConstant:
        alphaOp = BGRCLib_BlitAlphaOp_eUseConstAlpha;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (srcPaletteOffset) {
        BGRClib_GetDefaultPaletteBlitParams(&gfx->blitData.blitParams);
    } else {
        BGRClib_GetDefaultBlitParams(&gfx->blitData.blitParams);
    }
    gfx->blitData.blitParams.srcSurface = &src;
    gfx->blitData.blitParams.srcRect = (const BRect *)(void*)pSourceRect;
    gfx->blitData.blitParams.srcPaletteOffset = srcPaletteOffset;
    gfx->blitData.blitParams.outSurface = &out;
    gfx->blitData.blitParams.outRect = (const BRect *)(void*)pOutputRect;
    gfx->blitData.blitParams.colorOp = colorOp;
    gfx->blitData.blitParams.alphaOp = alphaOp;
    gfx->blitData.blitParams.constantColor = constantColor;
    gfx->blitData.blitParams.colorKeySelect = BGRC_Output_ColorKeySelection_eTakeBlend;

    if (out.format == BM2MC_PACKET_PixelFormat_eUIF_R8_G8_B8_A8)
    {
       NEXUS_SurfaceCreateSettings createSettings;
       NEXUS_Surface_GetCreateSettings(outputSurface, &createSettings);
       gfx->blitData.blitParams.miplevel = createSettings.mipLevel;
    }
    BDBG_PRINT_OP(("fastblit source=" BDBG_UINT64_FMT "(%u,%u,%u,%u) output=" BDBG_UINT64_FMT "(%u,%u,%u,%u) op=%u,%u,%#x",
        BDBG_UINT64_ARG(src.address),
        pSourceRect?pSourceRect->x:0, pSourceRect?pSourceRect->y:0, pSourceRect?pSourceRect->width:0, pSourceRect?pSourceRect->height:0,
        BDBG_UINT64_ARG(out.address),
        pOutputRect?pOutputRect->x:0, pOutputRect?pOutputRect->y:0, pOutputRect?pOutputRect->width:0, pOutputRect?pOutputRect->height:0,
        a_colorOp, a_alphaOp, constantColor
        ));

    rc = BGRClib_Blit(gfx->grclib, &gfx->blitData.blitParams, NULL, NULL, NULL, NULL);
    BTRC_TRACE(nexus_graphics2d_blit, STOP);

    return NEXUS_Graphics2D_P_FinishOperation(gfx, rc);
}

void NEXUS_Graphics2D_GetDefaultPorterDuffBlitSettings(NEXUS_Graphics2DPorterDuffBlitSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Graphics2D_PorterDuffBlit(NEXUS_Graphics2DHandle gfx, const NEXUS_Graphics2DPorterDuffBlitSettings *pSettings)
{
    BM2MC_PACKET_Plane src, dst, out;
    BERR_Code rc;
    NEXUS_Addr srcPaletteOffset, temp;

    BTRC_TRACE(nexus_graphics2d_blit, START);
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);

    nexus_p_check_and_switch_secure(gfx);

    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->sourceSurface, &src, &srcPaletteOffset);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    if (pSettings->destSurface) {
        NEXUS_Addr temp;
        rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->destSurface, &dst, &temp);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    }
    rc = NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, pSettings->outSurface, &out, &temp);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_CASSERT(BGRCLib_PorterDuffOp_Count == (BGRCLib_PorterDuffOp)NEXUS_PorterDuffOp_eMax);

    BDBG_PRINT_OP(("porterDuff source=" BDBG_UINT64_FMT "(%u,%u,%u,%u) output=" BDBG_UINT64_FMT "(%u,%u,%u,%u) dest=" BDBG_UINT64_FMT "(%u,%u,%u,%u) op=%u",
        BDBG_UINT64_ARG(src.address),
        pSettings->sourceRect.x, pSettings->sourceRect.y, pSettings->sourceRect.width, pSettings->sourceRect.height,
        BDBG_UINT64_ARG(out.address),
        pSettings->outRect.x, pSettings->outRect.y, pSettings->outRect.width, pSettings->outRect.height,
        BDBG_UINT64_ARG(dst.address),
        pSettings->destRect.x, pSettings->destRect.y, pSettings->destRect.width, pSettings->destRect.height,
        pSettings->operation ));

    rc = BGRClib_PorterDuffBlit(gfx->grclib, (BGRCLib_PorterDuffOp)pSettings->operation,
        &src, (const BRect *)(void*)&pSettings->sourceRect, srcPaletteOffset,
        pSettings->destSurface?&dst:NULL, (const BRect *)(void*)&pSettings->destRect,
        &out, (const BRect *)(void*)&pSettings->outRect);
    BTRC_TRACE(nexus_graphics2d_blit, STOP);
    return NEXUS_Graphics2D_P_FinishOperation(gfx, rc);
}

static BERR_Code NEXUS_Graphics2D_PacketCallback_isr( BGRC_Handle grc, void *data )
{
    struct NEXUS_Graphics2DEngine *engine = data;
    BSTD_UNUSED(grc);
    BDBG_PRINT_OP(("NEXUS_Graphics2D_PacketCallback_isr"));
    BKNI_SetEvent(engine->advanceEvent);
    return NEXUS_SUCCESS;
}

static int nexus_p_switch_secure(struct NEXUS_Graphics2DEngine *engine)
{
#if NEXUS_HAS_SAGE
    /* if nothing more to advance, check for mode switch */
    BAVC_CoreList coreList;
    unsigned index = (engine - g_grcInstance);
    bool secure = !engine->secure;
    int rc;

    /* attempt switch. will only work if all contexts synced. */
    rc = BGRC_SetSecureMode(engine->grc, secure);
    if (rc) {
        BDBG_MSG(("cannot switch M2MC%u from %u now", index, engine->secure));
        return rc;
    }

    BDBG_MSG(("M2MC%u secure %u -> %u", index, engine->secure, secure));
    BKNI_Memset(&coreList, 0, sizeof(coreList));
    BDBG_ASSERT(index <= BAVC_CoreId_eGFX_2 - BAVC_CoreId_eGFX_0); /* don't overrun BAVC_CoreId GFX types */
    coreList.aeCores[BAVC_CoreId_eGFX_0 + index] = true;
    if (secure) {
        rc = NEXUS_Sage_AddSecureCores(&coreList, NEXUS_SageUrrType_eDisplay);
        if (rc) BERR_TRACE(rc);
    }
    else {
        NEXUS_Sage_RemoveSecureCores(&coreList, NEXUS_SageUrrType_eDisplay);
    }
    engine->secure = secure;
#else
    if (!engine->secure) return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
    return NEXUS_SUCCESS;
}

static void nexus_p_check_and_switch_secure(NEXUS_Graphics2DHandle gfx)
{
#if NEXUS_HAS_SAGE
    if (gfx->openSettings.secure != gfx->engine->secure) {
        nexus_p_switch_secure(gfx->engine);
    }
#else
    BSTD_UNUSED(gfx);
#endif
}

static void NEXUS_Graphics2D_P_PacketAdvance( void *context )
{
    BERR_Code rc;
    struct NEXUS_Graphics2DEngine *engine = context;
    size_t n;
    unsigned i;
    BGRC_Packet_ContextStatus *array = engine->contextStatus;
    NEXUS_Graphics2DHandle gfx;
    bool secureModeSwitchNeeded;

    /* advance all */
    rc = BGRC_Packet_AdvancePackets(engine->grc, NULL);
    if (rc && rc != BGRC_PACKET_MSG_PACKETS_INCOMPLETE) {
        rc = BERR_TRACE(rc);
        return;
    }

    rc = BGRC_Packet_GetContextStatus(engine->grc, array, &n, NEXUS_GRAPHICS2D_MAX_CONTEXTS, &secureModeSwitchNeeded);
    if (rc) {rc = BERR_TRACE(rc); return;}

    BDBG_PRINT_OP(("NEXUS_Graphics2D_P_PacketAdvance %d contexts", n));
    for (i=0;i<n;i++) {
        gfx = array[i].private_data;
        BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
        BDBG_PRINT_OP(("  %p: avail=%d, sync=%d, checkpointCount=%d, %s", (void*)gfx, array[i].packet_buffer_available, array[i].sync, gfx->checkpointCount,
            gfx->openSettings.secure?"secure":"unsecure"));

        if (gfx->packetSpaceAvailableCount && array[i].packet_buffer_available) {
            if (--gfx->packetSpaceAvailableCount == 0) {
                NEXUS_TaskCallback_Fire(gfx->packetSpaceAvailable);
            }
        }
        if (gfx->checkpointCount && array[i].sync) {
            if (--gfx->checkpointCount == 0 && !gfx->settings.pollingCheckpoint) {
                NEXUS_Graphics2D_P_UnlockPlaneAndPalette(gfx);
                NEXUS_TaskCallback_Fire(gfx->checkpoint);
            }
        }
        else if (!gfx->checkpointCount && array[i].sync) {
            BDBG_ERR(("unexpected checkpoint %p", (void *)gfx));
        }
    }

    /* if a context needs a mode switch, attempt */
    if (secureModeSwitchNeeded && !nexus_p_switch_secure(engine)) {
        /* call recursively if we can switch */
        NEXUS_Graphics2D_P_PacketAdvance(engine);
    }
}

NEXUS_Error NEXUS_Graphics2D_Checkpoint( NEXUS_Graphics2DHandle gfx, const NEXUS_CallbackDesc *pCallback )
{
    NEXUS_Error rc, rc2;

    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);

    gfx->checkpointWatchdog.counter = 0;

    if (gfx->settings.pollingCheckpoint) {
        /* pollingCheckpoint still requires an internal sync if no checkpoint is pending.
        the one difference is that we avoid scheduling extra checkpoints and we prevent any callback to the app. */
        if (gfx->checkpointCount) {
            return NEXUS_GRAPHICS2D_BUSY;
        }
    }
    else {
        if ( pCallback && gfx->settings.checkpointCallback.callback ) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        if (gfx->checkpointCount) {
            /* after calling checkpoint, you must wait for the checkpoint callback before calling checkpoint again.
            this is a failure because we cannot reset the checkpoint. the app must wait. if it does not, graphics tearing will likely result. */
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }

        /* it's now allowed to call checkpoint with no callback. it can be used to "submit" blits
        regardless of the threshold */
        if (pCallback) {
            NEXUS_TaskCallback_Set( gfx->checkpoint, pCallback );
        }
    }

    if (gfx->openSettings.secure != g_grcInstance[gfx->index].secure) {
        nexus_p_switch_secure(&g_grcInstance[gfx->index]);
        /* if we can't mode switch now, BGRC_Packet_SyncPackets will be queued for later */
    }

    rc = BGRC_Packet_SyncPackets(gfx->grc, gfx->packetContext);
    switch (rc) {
    case BERR_SUCCESS:
    case BGRC_PACKET_MSG_PACKETS_INCOMPLETE:
        rc = BERR_SUCCESS;
        break;
    case BGRC_PACKET_MSG_BLITS_COMPLETE:
        break;
    default:
        return BERR_TRACE(rc);
    }

    rc2 = BGRC_Packet_SyncPackets(gfx->grc, gfx->functionContext);
    switch (rc2) {
    case BERR_SUCCESS:
    case BGRC_PACKET_MSG_PACKETS_INCOMPLETE:
        rc2 = BERR_SUCCESS;
        break;
    case BGRC_PACKET_MSG_BLITS_COMPLETE:
        break;
    default:
        return BERR_TRACE(rc2);
    }

    if (rc == BGRC_PACKET_MSG_BLITS_COMPLETE && rc2 == BGRC_PACKET_MSG_BLITS_COMPLETE) {
        /* no blits queue in either context. */
        gfx->surfaceFifoCheckpoint = gfx->surfaceFifoWrite;
        NEXUS_Graphics2D_P_UnlockPlaneAndPalette(gfx);
        return NEXUS_SUCCESS; /* no blits queued */
    }
    else if (rc==BERR_SUCCESS || rc2==BERR_SUCCESS) {
        /* blits are queued in at least one context */
        BDBG_ASSERT(!gfx->checkpointCount);
        if (rc==BERR_SUCCESS) gfx->checkpointCount++;
        if (rc2==BERR_SUCCESS) gfx->checkpointCount++;
        if (gfx->checkpointCount == 2) {
            /* In theory, this is supported. To simplify things, let's avoid it. */
            BDBG_WRN(("graphics2d %p mixing packet and function blit: %d %d", (void *)gfx, gfx->packetWriteCompleteCount, gfx->blitCount));
        }
        gfx->checkpointWaitCount = 0;
        if (gfx->settings.pollingCheckpoint) {
            return NEXUS_GRAPHICS2D_BUSY;
        }
        else {
            gfx->surfaceFifoCheckpoint = gfx->surfaceFifoWrite;
            return NEXUS_GRAPHICS2D_QUEUED;
        }
    }
    else {
        return BERR_TRACE(rc);
    }
}

NEXUS_Error NEXUS_Graphics2D_Memset32( NEXUS_Graphics2DHandle gfx, void *address, uint32_t data, unsigned count )
{
    NEXUS_Error err = NEXUS_SUCCESS;
    uint32_t offset;

    nexus_p_check_and_switch_secure(gfx);

    if (count < 0x400 || count > 0x1000000) { /* 1K to 16M */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (count % 0x400) { /* must be multiple of 1K */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    offset = NEXUS_AddrToOffset(address);
    if( offset == 0 )
    {
        BDBG_ERR(("Invalid memory address %p specified", address));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    err = BGRClib_Memset32( gfx->grclib, offset, data, count);

    return BERR_TRACE(err);
}

NEXUS_Error NEXUS_Graphics2D_GetPacketBuffer( NEXUS_Graphics2DHandle gfx, void **buffer, size_t *pSize, size_t minSize )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);

    nexus_p_check_and_switch_secure(gfx);

    if (!gfx->verifyPacketHeap) {
        /* packet buffer memory must be CPU accessible for application and driver. we can only check driver mapping here.
        only verify once for optimal performance. */
        if (gfx->openSettings.heap && !NEXUS_P_CpuAccessibleHeap(gfx->openSettings.heap)) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        gfx->verifyPacketHeap = true;
    }

    rc = BGRC_Packet_GetPacketMemory(gfx->grc, gfx->packetContext, buffer, pSize, minSize);
    if (rc || *pSize < minSize) {
        *buffer = NULL;
        *pSize = 0;
        gfx->packetSpaceAvailableCount++;
    }
    return 0;
}

NEXUS_Error NEXUS_Graphics2D_PacketWriteComplete( NEXUS_Graphics2DHandle gfx, size_t size )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(gfx, NEXUS_Graphics2D);
    gfx->packetWriteCompleteCount++;
    rc = BGRC_Packet_SubmitPackets(gfx->grc, gfx->packetContext, size);
    return !rc || (rc == BGRC_PACKET_MSG_PACKETS_INCOMPLETE) ? BERR_SUCCESS : BERR_TRACE(rc);
}

NEXUS_Error NEXUS_Graphics2D_ConvertFilter( NEXUS_Graphics2DFilterCoeffs filter, size_t srcSize, size_t outSize, BM2MC_PACKET_FilterCoeffs *pCoeffs )
{
    return BGRC_Packet_ConvertFilter(pCoeffs, filter, srcSize, outSize);
}

NEXUS_Error NEXUS_Graphics2D_ConvertColorMatrix( const NEXUS_Graphics2DColorMatrix *pMatrixIn, BM2MC_PACKET_ColorMatrix *pMatrixOut )
{
    return BGRC_Packet_ConvertColorMatrix(pMatrixOut, pMatrixIn->coeffMatrix, pMatrixIn->shift);
}

NEXUS_Error NEXUS_Graphics2DModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    int i;
    NEXUS_Error rc;

    BSTD_UNUSED(pSettings);

    if (!enabled) {
        for (i=0; i<NEXUS_NUM_GRAPHICS2D_CORES; i++) {
            struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[i];

            if (NULL==engine->grc) { continue; }

            rc = BGRC_Resume(engine->grc);
            if (rc) { return rc = BERR_TRACE(rc); }
        }
    } else {
        /* power down */
        for (i=0; i<NEXUS_NUM_GRAPHICS2D_CORES; i++) {
            struct NEXUS_Graphics2DEngine *engine = &g_grcInstance[i];
            NEXUS_Graphics2DHandle gfx;
            BGRC_Packet_Status grcStatus;
            BGRC_StandbySettings standbySettings;

            unsigned timeout;

            if (NULL==engine->grc) { continue; }

            BGRC_GetDefaultStandbySettings(&standbySettings);
            standbySettings.bS3Standby = (pSettings->mode==NEXUS_StandbyMode_eDeepSleep);

            /* flush packets in all contexts of this engine */
            for (gfx = BLST_S_FIRST(&engine->contexts); gfx; gfx = BLST_S_NEXT(gfx, link)) {
                bool callbackDisabled=false;

                /* Check if checkpoint was called already. If not then we need to do it to sync blits */
                /* Disable the task callback first, becuase app is not excepting it and may result in app
                   re-submiting a checkpoint before we decrement the count */
                if(!gfx->checkpointCount) {
                    NEXUS_TaskCallback_Set(gfx->checkpoint, NULL);
                    callbackDisabled = true;
                    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
                    if(rc && rc != NEXUS_GRAPHICS2D_QUEUED) { return BERR_TRACE(rc); }
                }

                /* Advance packets as long as checkpoint count does not go to zero, which indicates M2M queue is empty */
                timeout = 100;
                while(gfx->checkpointCount) {
                    NEXUS_Graphics2D_P_PacketAdvance((void*)engine);
                    if(gfx->checkpointCount) {
                        BKNI_Sleep(10);
                        timeout--;
                        if(!timeout) { BDBG_ERR(("Checkpoint Timeout")); break; }
                    }
                }

                /* Re-enable callback that may be disabled earlier for sync/advance */
                if(callbackDisabled) {
                    NEXUS_TaskCallback_Set(gfx->checkpoint, &gfx->settings.checkpointCallback);
                }
            }

            /* Wait while h/w is busy */
            timeout = 100;
            do {
                BGRC_Packet_GetStatus(engine->grc, &grcStatus);
                if(grcStatus.m2mc_busy) {
                    BKNI_Sleep(10);
                    timeout--;
                    if(!timeout) { BDBG_ERR(("M2MC Busy Timeout")); break; }
                }
            } while (grcStatus.m2mc_busy);

            rc = BGRC_Standby(engine->grc, &standbySettings);
            if (rc) { return rc = BERR_TRACE(rc); }
        }
    }
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return NEXUS_SUCCESS;
}

#define NEXUS_P_WATCHDOG_INTEVAL   250  /* 0.25 second */
static void nexus_graphics2d_p_checkpoint_watchdog(void *context)
{
    NEXUS_Graphics2DHandle gfx = context;
    if (gfx->checkpointWatchdog.counter) {
        if (--gfx->checkpointWatchdog.counter == 0) {
            /* if packetFifoThreshold is non-zero, blits will be queued until the fifo is full
            or NEXUS_Graphics2D_Checkpoint is issued. an app should not rely on filling the fifo, so a checkpoint
            is required. */
            BDBG_ERR(("missing call to NEXUS_Graphics2D_Checkpoint. %p blit is queued.", (void *)gfx));
        }
    }

    if (gfx->checkpointCount && ++gfx->checkpointWaitCount >= 3) { /* after 0.5 to 0.75 seconds */
        if (gfx->checkpointWaitCount < 6) {
            BGRC_Packet_CheckpointWatchdog(gfx->grc, gfx->packetContext, false);
            BGRC_Packet_CheckpointWatchdog(gfx->grc, gfx->functionContext, false);
        }
        else if (gfx->checkpointWaitCount < 16) {
            BGRC_Packet_CheckpointWatchdog(gfx->grc, gfx->packetContext, true);
            BGRC_Packet_CheckpointWatchdog(gfx->grc, gfx->functionContext, true);
        }
        else {
            /* bgrc failed to handle, we fire anyway */
            BDBG_WRN(("%p: checkpoint watchdog %d %d", (void *)gfx, gfx->checkpointCount, gfx->checkpointWaitCount));
            gfx->checkpointCount = 0;
            gfx->checkpointWaitCount = 0;
            NEXUS_Graphics2D_P_UnlockPlaneAndPalette(gfx);
            NEXUS_TaskCallback_Fire(gfx->checkpoint);
        }
    }

    gfx->checkpointWatchdog.timer = NEXUS_ScheduleTimer(NEXUS_P_WATCHDOG_INTEVAL, nexus_graphics2d_p_checkpoint_watchdog, gfx);
    if (!gfx->checkpointWatchdog.timer) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); }
}

void NEXUS_Graphics2D_SetAlphaColorMatrix( uint8_t alpha, NEXUS_Graphics2DColorMatrix *pMatrix )
{
    BKNI_Memset(pMatrix, 0, sizeof(*pMatrix));
    pMatrix->shift = 8; /* 2^8 == 256. this causes alpha to be 0%...100%. */
    pMatrix->coeffMatrix[0] = alpha;
    pMatrix->coeffMatrix[6] = alpha;
    pMatrix->coeffMatrix[12] = alpha;
    pMatrix->coeffMatrix[18] = 256;
    return;
}

void NEXUS_Graphics2D_GetCapabilities(NEXUS_Graphics2DHandle handle, NEXUS_Graphics2DCapabilities *pCapabilities)
{
    BGRC_Capabilities capabilities;

    BGRC_GetCapabilities(handle->grc, &capabilities);
    pCapabilities->maxVerticalDownScale = capabilities.ulMaxVerDownSclRatio;
    pCapabilities->maxHorizontalDownScale = capabilities.ulMaxHrzDownSclRatio;
    return;
}

bool NEXUS_Graphics2D_MipmapModeSupported_isrsafe(void)
{
    unsigned index;
    for(index=0;index<NEXUS_NUM_GRAPHICS2D_CORES;index++) {
        if ((NEXUS_Graphics2DMode)BGRC_GetMode_isrsafe(index) == NEXUS_Graphics2DMode_eMipmap) return true;
    }
    return false;
}

NEXUS_Error NEXUS_Graphics2D_SetFrequencyScaling(unsigned percent)
{
#if NEXUS_POWER_MANAGEMENT && BCHP_PWR_RESOURCE_M2MC0
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned clkRate;

    if(percent > 100) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
#if BCHP_PWR_RESOURCE_M2MC0
    rc = BCHP_PWR_GetMaxClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_M2MC0, &clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
    clkRate = percent*(clkRate/100);
    rc = BCHP_PWR_SetClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_M2MC0, clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
#endif
#if BCHP_PWR_RESOURCE_M2MC1
    rc = BCHP_PWR_GetMaxClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_M2MC1, &clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
    clkRate = percent*(clkRate/100);
    rc = BCHP_PWR_SetClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_M2MC1, clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
#endif

    return rc;
#else
    BSTD_UNUSED(percent);
    return NEXUS_NOT_SUPPORTED;
#endif
}
