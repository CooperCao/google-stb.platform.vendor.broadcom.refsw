/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_surface_module.h"
#include "priv/nexus_core.h"
#include "priv/nexus_core_video.h"
#include "nexus_client_resources.h"
#include "priv/nexus_surface_module_local.h"

BDBG_MODULE(nexus_surface);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

#define NEXUS_P_SURFACE_PIN(s,kind) do { if(!(s)->kind.pinned) {NEXUS_Surface_P_DoPin_##kind (s);} } while(0)

/* global instances */
NEXUS_ModuleHandle g_NEXUS_surfaceModule;
static struct NEXUS_SurfaceModuleData {
    NEXUS_SurfaceModuleSettings settings;
    BLST_D_HEAD(NEXUS_Surface_P_List, NEXUS_Surface) allocatedSurfaces;
} g_NEXUS_SurfaceModuleData;


void NEXUS_SurfaceModule_GetDefaultSettings(NEXUS_SurfaceModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static void NEXUS_SurfaceModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_SurfaceHandle surface;
    unsigned allocatedCount=0;
    unsigned allocatedSize=0;
    unsigned pinnedCount=0;
    unsigned pinnedSize=0;
    unsigned relocatableCount=0;
    unsigned relocatableSize=0;

    for(surface=BLST_D_FIRST(&g_NEXUS_SurfaceModuleData.allocatedSurfaces);surface; surface=BLST_D_NEXT(surface, link)) {
        size_t size = surface->plane.ulBufSize;
        allocatedCount ++;
        allocatedSize += size;
        if(surface->offset.pinned) {
            pinnedCount ++;
            pinnedSize += size;
        } else if(surface->lockCnt==0) {
            relocatableCount ++;
            relocatableSize += size;
        }
    }
    BDBG_LOG(("SurfaceModule: allocated:%u(%u) pinned:%u(%u) relocatable:%u(%u)", allocatedCount, allocatedSize, pinnedCount, pinnedSize, relocatableCount, relocatableSize));
    return;
#endif
}

NEXUS_ModuleHandle NEXUS_SurfaceModule_Init(const NEXUS_SurfaceModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;

    BDBG_ASSERT(!g_NEXUS_surfaceModule);

    BKNI_Memset(&g_NEXUS_SurfaceModuleData, 0, sizeof(g_NEXUS_SurfaceModuleData));
    if (pSettings) {
        g_NEXUS_SurfaceModuleData.settings = *pSettings;
    }
    else {
        NEXUS_SurfaceModule_GetDefaultSettings(&g_NEXUS_SurfaceModuleData.settings);
    }
    BLST_D_INIT(&g_NEXUS_SurfaceModuleData.allocatedSurfaces);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eHigh; /* surface interface is fast */
    moduleSettings.dbgPrint = NEXUS_SurfaceModule_P_Print;
    moduleSettings.dbgModules = "nexus_surface";
    g_NEXUS_surfaceModule = NEXUS_Module_Create("surface", &moduleSettings);
    if (!g_NEXUS_surfaceModule) {
        return NULL;
    }
    return g_NEXUS_surfaceModule;
}

void NEXUS_SurfaceModule_Uninit(void)
{
    NEXUS_Module_Destroy(g_NEXUS_surfaceModule);
    g_NEXUS_surfaceModule = NULL;
}

void NEXUS_Surface_GetDefaultCreateSettings(NEXUS_SurfaceCreateSettings *pCreateSettings)
{
    BKNI_Memset(pCreateSettings, 0, sizeof(*pCreateSettings));
    pCreateSettings->pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    pCreateSettings->palettePixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    pCreateSettings->width = 720;
    pCreateSettings->height = 480;
    pCreateSettings->alignment = 12; /* 2^12 = 4K alignment needed for RAC. see nexus_dma.h for detailed description of cache coherency requirements. */
}

static void NEXUS_Surface_P_DoPin_offset(NEXUS_SurfaceHandle surface)
{
    surface->offset.pinned = true;
    surface->offset.pixels = BMMA_LockOffset(surface->plane.hPixels);
    if(surface->offset.pixels) {
        surface->offset.pixels += surface->plane.ulPixelsOffset;
    }
    if(surface->plane.hPalette) {
        surface->offset.palette = BMMA_LockOffset(surface->plane.hPalette);
        if(surface->offset.palette) {
            surface->offset.palette += surface->plane.ulPaletteOffset;
        }
    }
    return;
}

static void NEXUS_Surface_P_Release(NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_Surface, surface, Destroy);
    NEXUS_CLIENT_RESOURCES_RELEASE(surface,Count,NEXUS_ANY_ID);
}

static void NEXUS_Surface_P_Finalizer(NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Surface, surface);

    BDBG_MSG(("NEXUS_Surface_P_Finalizer:%p", (void *)surface));
    if(NEXUS_P_Surface_LocalRelease(surface)) {
        NEXUS_Surface_UnlockPlaneAndPalette(surface);
    }

    if(surface->memoryPropertiesValid) {
        if(surface->memoryProperties.pixelMemory!=NULL && surface->createSettings.pixelMemory == NULL) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, surface->memoryProperties.pixelMemory, Release);
            NEXUS_MemoryBlock_Free(surface->memoryProperties.pixelMemory);
        }
        if(surface->memoryProperties.paletteMemory!=NULL && surface->createSettings.paletteMemory == NULL) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, surface->memoryProperties.paletteMemory, Release);
            NEXUS_MemoryBlock_Free(surface->memoryProperties.paletteMemory);
        }
    }

    /* Release offset locks that were locally acquired */
    if(surface->offset.pinned) {
        BMMA_UnlockOffset(surface->plane.hPixels,surface->offset.pixels - surface->plane.ulPixelsOffset);
        if(surface->plane.hPalette) {
            BMMA_UnlockOffset(surface->plane.hPalette, surface->offset.palette - surface->plane.ulPaletteOffset);
        }
    }
    while(surface->lockCnt>0) {
        surface->lockCnt--;
        BMMA_UnlockOffset(surface->plane.hPixels,surface->offset.pixels - surface->plane.ulPixelsOffset);
        if(surface->plane.hPalette) {
            BMMA_UnlockOffset(surface->plane.hPalette, surface->offset.palette - surface->plane.ulPaletteOffset);
        }
    }

    /* it's expected that BMMA_Free would handle case where buffer is still locked */

    if (surface->plane.hPixels) {
        BMMA_Free(surface->plane.hPixels);
    }
    if (surface->plane.hPalette) {
        BMMA_Free(surface->plane.hPalette);
    }
    BLST_D_REMOVE(&g_NEXUS_SurfaceModuleData.allocatedSurfaces, surface, link);
    NEXUS_OBJECT_DESTROY(NEXUS_Surface, surface);
    BKNI_Free(surface);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Surface, NEXUS_Surface_Destroy_driver);

static void NEXUS_Surface_P_Init(NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_INIT(NEXUS_Surface, surface);
    surface->lockCnt = 0;
    surface->offset.pinned = false;
    surface->offset.pixels = 0;
    surface->offset.palette = 0;
    surface->settings.autoFlush = false;
    BLST_D_INSERT_HEAD(&g_NEXUS_SurfaceModuleData.allocatedSurfaces, surface, link);
    return;
}

NEXUS_SurfaceHandle NEXUS_Surface_Create(const NEXUS_SurfaceCreateSettings *pCreateSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BPXL_Format pixel_format;
    BPXL_Format palette_pixel_format;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings defaultSettings;
    BMMA_Heap_Handle heap;
    NEXUS_MemoryStatus heapStatus;
    BMMA_AllocationSettings mmaSettings;

    if(!pCreateSettings) {
        NEXUS_Surface_GetDefaultCreateSettings(&defaultSettings);
        pCreateSettings = &defaultSettings;
    }
    BDBG_MSG(("NEXUS_Surface_Create %ux%u, pixel: %u",
        pCreateSettings->width, pCreateSettings->height, pCreateSettings->pixelFormat));

    if (NEXUS_P_PixelFormat_ToMagnum_isrsafe(pCreateSettings->pixelFormat, &pixel_format)) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (NEXUS_P_PixelFormat_ToMagnum_isrsafe(pCreateSettings->palettePixelFormat, &palette_pixel_format)) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (!pCreateSettings->width || !pCreateSettings->height) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(surface,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    surface = BKNI_Malloc(sizeof(*surface));
    if (!surface) {
        NEXUS_CLIENT_RESOURCES_RELEASE(surface,Count,NEXUS_ANY_ID);
        rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_Surface_P_Init(surface);
    surface->createSettings = *pCreateSettings;
    surface->memoryPropertiesValid = false;
    if(pixel_format == BPXL_eUIF_R8_G8_B8_A8)
        BPXL_Plane_Uif_Init(&surface->plane, surface->createSettings.width, surface->createSettings.height, pixel_format, (BCHP_Handle)g_pCoreHandles->chp);
    else
        BPXL_Plane_Init(&surface->plane, surface->createSettings.width, surface->createSettings.height, pixel_format);

    if ( pCreateSettings->pMemory || pCreateSettings->pixelMemory) {
        surface->createSettings.heap = NULL;
        heap = NULL;
    }
    else {
        /* if NULL, get the registered default heap. modify the stored createSettings for GetCreateSettings. */
        surface->createSettings.heap = NEXUS_P_DefaultHeap(pCreateSettings->heap, BPXL_IS_PALETTE_FORMAT(pixel_format) ? NEXUS_DefaultHeapType_eFull : NEXUS_DefaultHeapType_eAny);
        /* retain support for module-level default index, but this should not be used. */
        if (!surface->createSettings.heap && g_NEXUS_SurfaceModuleData.settings.heapIndex < NEXUS_MAX_HEAPS) {
            surface->createSettings.heap = g_pCoreHandles->heap[g_NEXUS_SurfaceModuleData.settings.heapIndex].nexus;
        }
        if (!surface->createSettings.heap) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        heap = NEXUS_Heap_GetMmaHandle(surface->createSettings.heap);

        NEXUS_Heap_GetStatus(surface->createSettings.heap, &heapStatus);
        BMMA_GetDefaultAllocationSettings(&mmaSettings);
    }

    if(pCreateSettings->pitch) {
        if (pCreateSettings->pitch < surface->plane.ulPitch) {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            BDBG_ERR(("specified pitch(%u) is too small(%u)", (unsigned)pCreateSettings->pitch, (unsigned)surface->plane.ulPitch));
            goto error;
        }
        surface->plane.ulPitch = pCreateSettings->pitch;
        surface->plane.ulBufSize = surface->plane.ulHeight * surface->plane.ulPitch;
    }
    if ((uint64_t)surface->plane.ulHeight * surface->plane.ulPitch > surface->plane.ulBufSize) {
        BDBG_ERR(("surface too large: pitch %u x height %u", (unsigned)surface->plane.ulPitch, (unsigned)surface->plane.ulHeight));
        goto error;
    }

    if ( pCreateSettings->pMemory )
    {
        NEXUS_Addr offset = NEXUS_AddrToOffset(pCreateSettings->pMemory);
        if ( 0 == offset )
        {
            BDBG_ERR(("Invalid pMemory address %p specified", pCreateSettings->pMemory));
            goto error;
        }
        surface->plane.hPixels = BMMA_Block_Create(g_NEXUS_pCoreHandles->mma, offset, surface->plane.ulBufSize, pCreateSettings->pMemory);
        if(surface->plane.hPixels==NULL) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
    } else if (pCreateSettings->pixelMemory) {
        surface->plane.ulPixelsOffset = pCreateSettings->pixelMemoryOffset;
        surface->plane.hPixels = NEXUS_MemoryBlock_GetBlock_priv(pCreateSettings->pixelMemory);
        BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPixels);
    } else {
        size_t sz = surface->plane.ulBufSize;
        unsigned alignment_in_bytes;

        /* alignment applies to the backend of the buffer too. */
        alignment_in_bytes = 1 << pCreateSettings->alignment;
        alignment_in_bytes = (surface->plane.ulAlignment > alignment_in_bytes)? surface->plane.ulAlignment : alignment_in_bytes;
        if (alignment_in_bytes && (sz % alignment_in_bytes)) {
            sz += alignment_in_bytes - (sz % alignment_in_bytes);
        }
        surface->plane.hPixels = BMMA_Alloc(heap, sz, alignment_in_bytes, &mmaSettings);
        if (!surface->plane.hPixels) {
            rc = ((heapStatus.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)==NEXUS_MEMORY_TYPE_DYNAMIC) ?
               BERR_OUT_OF_DEVICE_MEMORY : BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            if (!((heapStatus.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)==NEXUS_MEMORY_TYPE_DYNAMIC)) {
               BDBG_ERR(("unable to allocate surface size=%u alignment=2^%d", (unsigned)sz, pCreateSettings->alignment));
            }
            goto error;
        }
        BDBG_MSG(("allocated surface mem=%p size=%d alignment=2^%d", (void *)surface->plane.hPixels, (unsigned)sz, pCreateSettings->alignment));
    }

    if (BPXL_IS_PALETTE_FORMAT(pixel_format)) {
        surface->plane.ulNumPaletteEntries = BPXL_NUM_PALETTE_ENTRIES(pixel_format);
        if (pCreateSettings->pPaletteMemory) {
            uint32_t offset;
            offset = NEXUS_AddrToOffset(pCreateSettings->pPaletteMemory);
            if ( 0 == offset )
            {
                BDBG_ERR(("Invalid pPaletteMemory address %p specified", pCreateSettings->pPaletteMemory));
                goto error;
            }
            surface->plane.hPalette = BMMA_Block_Create(g_NEXUS_pCoreHandles->mma, offset, 256 * 4, pCreateSettings->pPaletteMemory);
            if(surface->plane.hPalette==NULL) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto error;
            }
        } else if (pCreateSettings->paletteMemory) {
            surface->plane.ulPaletteOffset = pCreateSettings->paletteMemoryOffset;
            surface->plane.hPalette = NEXUS_MemoryBlock_GetBlock_priv(pCreateSettings->paletteMemory);
            BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPalette);
        } else {
            surface->plane.hPalette = BMMA_Alloc(heap, 256 * 4, 1<<5, &mmaSettings);
            if(surface->plane.hPalette==NULL) {
                rc = ((heapStatus.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)==NEXUS_MEMORY_TYPE_DYNAMIC) ?
                   NEXUS_OUT_OF_DEVICE_MEMORY : BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
                goto error;
            }
        }
    }
    if (pCreateSettings->pMemory || pCreateSettings->pPaletteMemory) {
        NEXUS_P_SURFACE_PIN(surface, offset);
    }

    BDBG_MSG(("NEXUS_Surface_Create:%p %ux%u, pixel: %u", (void*)surface, pCreateSettings->width, pCreateSettings->height, pCreateSettings->pixelFormat));
    return surface;

error:
    NEXUS_Surface_Destroy_driver(surface);
    return NULL;
}

NEXUS_SurfaceHandle
NEXUS_Surface_CreateFromPixelPlane_priv(const BPXL_Plane *pPlane)
{
    NEXUS_SurfaceHandle surface;

    NEXUS_ASSERT_MODULE();

    surface = BKNI_Malloc(sizeof(*surface));
    if (!surface) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}
    NEXUS_Surface_P_Init(surface);
    NEXUS_Surface_GetDefaultCreateSettings(&surface->createSettings);
    surface->plane = *pPlane;
    surface->createSettings.width = surface->plane.ulWidth;
    surface->createSettings.height = surface->plane.ulHeight;
    surface->createSettings.pitch = surface->plane.ulPitch;
    surface->createSettings.pixelFormat = NEXUS_P_PixelFormat_FromMagnum_isrsafe(surface->plane.eFormat);
    surface->createSettings.palettePixelFormat = NEXUS_P_PixelFormat_FromMagnum_isrsafe(surface->plane.ePalettePixelFormat);
    BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPixels);
    if(surface->plane.hPalette) {
        BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPalette);
    }
    return surface;

err_alloc:
    return NULL;
}

const BPXL_Plane *NEXUS_Surface_GetPixelPlane_priv(NEXUS_SurfaceHandle surface)
{
    BDBG_OBJECT_ASSERT(surface, NEXUS_Surface);
    NEXUS_ASSERT_MODULE();
    NEXUS_P_SURFACE_PIN(surface, offset); /* this is required, until all consumers of GetPixelPlane would get converted to properly Lock/Unlock memory blocks */
    return &surface->plane;
}

NEXUS_Error NEXUS_Surface_SetSettings(NEXUS_SurfaceHandle surface, const NEXUS_SurfaceSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(surface, NEXUS_Surface);
    if (pSettings->autoFlush) {
        /* See header file comments and SW7420-1205 for rationale for this failure. */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    surface->settings = *pSettings;
    return 0;
}

void NEXUS_Surface_GetSettings(NEXUS_SurfaceHandle surface, NEXUS_SurfaceSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(surface, NEXUS_Surface);
    *pSettings = surface->settings;
}


void NEXUS_Surface_GetCreateSettings(NEXUS_SurfaceHandle surface, NEXUS_SurfaceCreateSettings *pCreateSettings)
{
    BDBG_OBJECT_ASSERT(surface, NEXUS_Surface);
    *pCreateSettings = surface->createSettings;
    return;
}

NEXUS_Error NEXUS_Surface_InitPlaneAndPaletteOffset( NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset)
{
    return NEXUS_Surface_InitPlaneAndPaletteOffset_priv( surface, pPlane, pPaletteOffset);
}

NEXUS_Error NEXUS_Surface_InitPlaneAndPaletteOffset_priv( NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset)
{
    BDBG_OBJECT_ASSERT(surface, NEXUS_Surface);
    if(surface->createSettings.managedAccess) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    NEXUS_P_SURFACE_PIN(surface,offset);
    pPlane->address = surface->offset.pixels;
    pPlane->pitch = surface->plane.ulPitch;
    pPlane->format = surface->createSettings.pixelFormat;
    pPlane->width = surface->createSettings.width;
    pPlane->height = surface->createSettings.height;
    if(pPaletteOffset) {
        *pPaletteOffset = surface->offset.palette; /* will be 0 if no palette */
    }

    return NEXUS_SUCCESS;
}

/*********************************
* striped surface
**********************************/

struct NEXUS_StripedSurface
{
    NEXUS_OBJECT(NEXUS_StripedSurface);
    NEXUS_StripedSurfaceCreateSettings createSettings;
    unsigned lockCnt;
    struct {
        NEXUS_Addr luma; /* valid if pined */
        NEXUS_Addr chroma; /* valid if pined */
    } offset;
    NEXUS_MemoryBlockHandle lumaBuffer, chromaBuffer; /* if set, internally allocated */
};

void NEXUS_StripedSurface_GetDefaultCreateSettings( NEXUS_StripedSurfaceCreateSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bufferType   = NEXUS_VideoBufferType_eFrame;
    pSettings->lumaPixelFormat = NEXUS_PixelFormat_eY8;
    pSettings->chromaPixelFormat = NEXUS_PixelFormat_eCb8_Cr8;
}

#define MB_HEIGHT           16
NEXUS_StripedSurfaceHandle NEXUS_StripedSurface_Create( const NEXUS_StripedSurfaceCreateSettings *pSettings )
{
    BCHP_MemoryInfo memInfo;
    unsigned totalByteWidth;
    unsigned nmby, mbMultiplier;
    unsigned mbRemainder, mbAddition=0;
    unsigned lumaBufSize, chromaBufSize;
    unsigned alignment = 0x400; /* 1KB */
    NEXUS_StripedSurfaceCreateSettings picCfg;
    NEXUS_MemoryStatus s;
    NEXUS_StripedSurfaceHandle stripedSurface;

    NEXUS_StripedSurface_GetDefaultCreateSettings(&picCfg);
    if(pSettings==NULL) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_done;
    }
    /* lumaBuffer and lumaHeap are mutual exclusive */
    if((pSettings->lumaBuffer==NULL && pSettings->lumaHeap==NULL) ||
       (pSettings->lumaBuffer && pSettings->lumaHeap)) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_done;
    }
    /* chromaBuffer and chromaHeap are mutual exclusive */
    if((pSettings->chromaBuffer==NULL && pSettings->chromaHeap==NULL) ||
       (pSettings->chromaBuffer && pSettings->chromaHeap)) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_done;
    }

    /* if internally create stripe surface from the given heaps */
    if(pSettings->lumaHeap && pSettings->chromaHeap) {
        /* validate the rest fields */
        picCfg.lumaHeap = pSettings->lumaHeap;
        picCfg.chromaHeap = pSettings->chromaHeap;
        picCfg.imageWidth = pSettings->imageWidth;
        picCfg.imageHeight = pSettings->imageHeight;
        if(BKNI_Memcmp(&picCfg, pSettings, sizeof(picCfg))) {
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_done;
        }

        /* find the luma/chroma stripe surface parameters (stripeWidth, NMBY_M/R) */
        /* assume luma/chroma stripe parameters are the same */
        NEXUS_Heap_GetStatus(pSettings->lumaHeap, &s);
        BCHP_GetMemoryInfo(g_pCoreHandles->chp, &memInfo);
        picCfg.stripedWidth = memInfo.memc[s.memcIndex].ulStripeWidth;
        mbMultiplier = memInfo.memc[s.memcIndex].ulMbMultiplier;
        mbRemainder  = memInfo.memc[s.memcIndex].ulMbRemainder;

        /* 1) round up height to closest MB size */
        nmby   = (pSettings->imageHeight+ MB_HEIGHT - 1)/MB_HEIGHT;

        /* 2) get the MB height addition to make NMBY = mbMultiplier * N + mbRemainder */
        mbRemainder = nmby & (mbMultiplier - 1);
        mbAddition   = ((mbMultiplier + mbRemainder) - mbRemainder) & (mbMultiplier - 1);

        /* 3) calculate the stripe height in lines */
        picCfg.lumaStripedHeight = (nmby + mbAddition) * MB_HEIGHT;

        /* 4) calculate chroma stripe height */
        nmby = ((pSettings->imageHeight / 2) + MB_HEIGHT - 1) / MB_HEIGHT;/* 420 chroma */
        mbRemainder = nmby & (mbMultiplier - 1);
        mbAddition = ((mbMultiplier + mbRemainder) - mbRemainder) & (mbMultiplier - 1);
        picCfg.chromaStripedHeight = (nmby + mbAddition) * MB_HEIGHT;

        /* Calculate size of luma and chroma buffers in bytes */
        totalByteWidth = picCfg.stripedWidth * (picCfg.imageWidth + (picCfg.stripedWidth - 1)) / picCfg.stripedWidth;
        lumaBufSize   = totalByteWidth * picCfg.lumaStripedHeight;
        chromaBufSize = totalByteWidth * picCfg.chromaStripedHeight;

        alignment = (alignment > s.alignment)? alignment : s.alignment;
        picCfg.lumaBuffer = NEXUS_MemoryBlock_Allocate(pSettings->lumaHeap, lumaBufSize, alignment, NULL);
        if(!picCfg.lumaBuffer)
        {
            BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_noluma;
        }

        picCfg.chromaBuffer = NEXUS_MemoryBlock_Allocate(pSettings->chromaHeap, chromaBufSize, alignment, NULL);
        if(!picCfg.chromaBuffer)
        {
            BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_nochroma;
        }
        BDBG_MSG(("lumaBuffer=%p, chromaBuffer=%p", (void *)picCfg.lumaBuffer, (void *)picCfg.chromaBuffer));
        pSettings = &picCfg;
    }

    stripedSurface = BKNI_Malloc(sizeof(*stripedSurface));
    if (!stripedSurface) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_nomalloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_StripedSurface, stripedSurface);
    stripedSurface->createSettings = *pSettings;
    stripedSurface->lumaBuffer = picCfg.lumaBuffer;
    stripedSurface->chromaBuffer = picCfg.chromaBuffer;
    return stripedSurface;

err_nomalloc:
    if (picCfg.chromaBuffer) {
        NEXUS_MemoryBlock_Free(picCfg.chromaBuffer);
    }
err_nochroma:
    if (picCfg.lumaBuffer) {
        NEXUS_MemoryBlock_Free(picCfg.lumaBuffer);
    }
err_noluma:
err_done:
    return NULL;
}

static void NEXUS_StripedSurface_P_Finalizer( NEXUS_StripedSurfaceHandle stripedSurface )
{
    BDBG_OBJECT_ASSERT(stripedSurface, NEXUS_StripedSurface);

    /* free allocated memory if the stripe surface was created internally from given heaps */
    if(stripedSurface->lumaBuffer) {
        NEXUS_MemoryBlock_Free(stripedSurface->lumaBuffer);
    }
    if(stripedSurface->chromaBuffer) {
        NEXUS_MemoryBlock_Free(stripedSurface->chromaBuffer);
    }
    NEXUS_OBJECT_DESTROY(NEXUS_StripedSurface, stripedSurface);
    BKNI_Free(stripedSurface);
}

static void NEXUS_StripedSurface_P_Release( NEXUS_StripedSurfaceHandle stripedSurface )
{
    BDBG_OBJECT_ASSERT(stripedSurface, NEXUS_StripedSurface);

    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_StripedSurface, NEXUS_StripedSurface_Destroy);

void NEXUS_StripedSurface_GetCreateSettings( NEXUS_StripedSurfaceHandle stripedSurface, NEXUS_StripedSurfaceCreateSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(stripedSurface, NEXUS_StripedSurface);
    *pSettings = stripedSurface->createSettings;
}

NEXUS_Error NEXUS_StripedSurface_GetStatus( NEXUS_StripedSurfaceHandle stripedSurface, NEXUS_StripedSurfaceStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(stripedSurface, NEXUS_StripedSurface);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->width = stripedSurface->createSettings.imageWidth;
    pStatus->height = stripedSurface->createSettings.imageHeight;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Surface_LockPlaneAndPalette( NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Surface, surface);
    surface->offset.pixels = BMMA_LockOffset(surface->plane.hPixels) + surface->plane.ulPixelsOffset; /* save result of BMMA_LockOffset */
    pPlane->address = surface->offset.pixels;
    pPlane->pitch = surface->plane.ulPitch;
    pPlane->format = surface->createSettings.pixelFormat;
    pPlane->width = surface->createSettings.width;
    pPlane->height = surface->createSettings.height;
    surface->lockCnt++;
    if(surface->plane.hPalette) {
        surface->offset.palette = BMMA_LockOffset(surface->plane.hPalette) + surface->plane.ulPaletteOffset; /* save result of BMMA_LockOffset */
    }
    if(pPaletteOffset) {
        *pPaletteOffset = surface->offset.palette; /* will be 0 if no palette */
    }
    return NEXUS_SUCCESS;
}

void NEXUS_Surface_UnlockPlaneAndPalette_priv( NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Surface, surface);
    BMMA_UnlockOffset(surface->plane.hPixels,surface->offset.pixels - surface->plane.ulPixelsOffset);
    surface->lockCnt--;
    if(surface->plane.hPalette) {
        BMMA_UnlockOffset(surface->plane.hPalette, surface->offset.palette - surface->plane.ulPaletteOffset);
    }
    return ;
}

void NEXUS_Surface_UnlockPlaneAndPalette( NEXUS_SurfaceHandle surface)
{
    NEXUS_Surface_UnlockPlaneAndPalette_priv(surface);
    return;
}

void NEXUS_Surface_MarkDiscardable(NEXUS_SurfaceHandle surface)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Surface, surface);
    BMMA_MarkDiscarable(surface->plane.hPixels);
    if(surface->plane.hPalette) {
        BMMA_MarkDiscarable(surface->plane.hPalette);
    }
    return ;
}

void NEXUS_Surface_GetStatus(NEXUS_SurfaceHandle surface, NEXUS_SurfaceStatus *pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Surface, surface);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->pixelFormat = surface->createSettings.pixelFormat;
    pStatus->palettePixelFormat = surface->createSettings.palettePixelFormat;
    pStatus->width = surface->createSettings.width;
    pStatus->height = surface->createSettings.height;
    pStatus->pitch = surface->plane.ulPitch;
    pStatus->numPaletteEntries = surface->plane.ulNumPaletteEntries;
    return;
}

void NEXUS_Surface_GetMemoryProperties( NEXUS_SurfaceHandle surface, NEXUS_SurfaceMemoryProperties *pProperties)
{
    pProperties->pixelMemory = NULL;
    pProperties->pixelMemoryOffset = 0;
    pProperties->paletteMemory = NULL;
    pProperties->paletteMemoryOffset = 0;
    pProperties->pixelMemoryTransient = false;
    pProperties->paletteMemoryTransient = false;
    if(!surface->memoryPropertiesValid) {
        /* only create NEXUS_MemoryBlock if application has requested it */
        surface->memoryProperties.pixelMemory = NULL;
        surface->memoryProperties.pixelMemoryOffset = 0;
        surface->memoryProperties.paletteMemory = NULL;
        surface->memoryProperties.paletteMemoryOffset = 0;
        if(surface->createSettings.pMemory ) {
            surface->memoryProperties.pixelMemory = NULL;
        } else if(surface->createSettings.pixelMemory) {
            surface->memoryProperties.pixelMemory = surface->createSettings.pixelMemory;
            surface->memoryProperties.pixelMemoryOffset = surface->createSettings.pixelMemoryOffset;
        } else {
            NEXUS_Module_Lock(g_NEXUS_SurfaceModuleData.settings.core);
            surface->memoryProperties.pixelMemory = NEXUS_MemoryBlock_P_CreateFromMma_priv(surface->plane.hPixels);
            NEXUS_Module_Unlock(g_NEXUS_SurfaceModuleData.settings.core);
            if(surface->memoryProperties.pixelMemory==NULL) {
                (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                return;
            }
            surface->memoryProperties.pixelMemoryTransient = true;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, surface->memoryProperties.pixelMemory, Acquire);
            BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPixels);
        }
        if(surface->createSettings.pPaletteMemory) {
            surface->memoryProperties.paletteMemory = NULL;
        } else if(surface->createSettings.paletteMemory) {
            surface->memoryProperties.paletteMemory = surface->createSettings.paletteMemory;
            surface->memoryProperties.paletteMemoryOffset = surface->createSettings.paletteMemoryOffset;
        } else if(surface->plane.hPalette) {
            NEXUS_Module_Lock(g_NEXUS_SurfaceModuleData.settings.core);
            surface->memoryProperties.paletteMemory = NEXUS_MemoryBlock_P_CreateFromMma_priv(surface->plane.hPalette);
            NEXUS_Module_Unlock(g_NEXUS_SurfaceModuleData.settings.core);
            if(surface->memoryProperties.paletteMemory==NULL) {
                (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                if(surface->memoryProperties.pixelMemory!=NULL && surface->createSettings.pixelMemory == NULL) {
                    surface->memoryProperties.pixelMemoryTransient = false;
                    NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, surface->memoryProperties.pixelMemory, Release);
                    NEXUS_MemoryBlock_Free(surface->memoryProperties.pixelMemory);
                }
                return;
            }
            surface->memoryProperties.paletteMemoryTransient = true;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, surface->memoryProperties.paletteMemory, Acquire);
            BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, surface->plane.hPalette);
        }
        surface->memoryPropertiesValid = true;
    }
    *pProperties = surface->memoryProperties;
    return;
}

