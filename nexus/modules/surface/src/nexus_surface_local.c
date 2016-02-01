/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_surface.h"
#include "priv/nexus_surface_module_local.h"

BDBG_MODULE(nexus_surface_local);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

struct NEXUS_SurfaceLocal {
    BLST_AA_TREE_ENTRY(NEXUS_P_SurfaceLocalTree) node;
    NEXUS_SurfaceHandle surface;
    unsigned lockCnt;
    bool pinned;
    void *lockedPlane;
    void *lockedPalette;
    bool statusValid;
    bool memoryPropertiesValid;
    NEXUS_SurfaceMemoryProperties memoryProperties;
    NEXUS_SurfaceStatus status;
};

BLST_AA_TREE_HEAD(NEXUS_P_SurfaceLocalTree, NEXUS_SurfaceLocal);

struct NEXUS_SurfaceModule_StateLocal {
    BKNI_MutexHandle lockTree; /* lock used to access tree */
    BKNI_MutexHandle lockState; /* lock used to modify state of NEXUS_SurfaceLocal and call into */
    struct NEXUS_P_SurfaceLocalTree surfaces;
};

static struct NEXUS_SurfaceModule_StateLocal g_NexusSurfaceLocal;

static int NEXUS_P_SurfaceLocal_Compare(const struct NEXUS_SurfaceLocal * node, NEXUS_SurfaceHandle surface)
{
    if((char *)surface > (char *)node->surface) {
        return 1;
    } else if(surface==node->surface) {
        return 0;
    } else {
        return -1;
    }
}


BLST_AA_TREE_GENERATE_FIND(NEXUS_P_SurfaceLocalTree , NEXUS_SurfaceHandle , NEXUS_SurfaceLocal, node, NEXUS_P_SurfaceLocal_Compare)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_SurfaceLocalTree, NEXUS_SurfaceHandle , NEXUS_SurfaceLocal, node, NEXUS_P_SurfaceLocal_Compare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_SurfaceLocalTree, NEXUS_SurfaceLocal, node)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_SurfaceLocalTree, NEXUS_SurfaceLocal, node)

static struct NEXUS_SurfaceLocal *NEXUS_P_Surface_CreateLocal_lockedTree(NEXUS_SurfaceHandle surface)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;
    struct NEXUS_SurfaceLocal *surfaceLocalInserted;

    surfaceLocal = BKNI_Malloc(sizeof(*surfaceLocal));
    if(!surfaceLocal) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    surfaceLocal->lockedPlane = NULL;
    surfaceLocal->lockedPalette = NULL;
    surfaceLocal->lockCnt = 0;
    surfaceLocal->surface = surface;
    surfaceLocal->pinned = false;
    surfaceLocal->statusValid = false;
    surfaceLocal->memoryPropertiesValid = false;
    surfaceLocalInserted=BLST_AA_TREE_INSERT(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces, surfaceLocal->surface, surfaceLocal);
    BDBG_ASSERT(surfaceLocalInserted==surfaceLocal);
    return surfaceLocal;
}

static struct NEXUS_SurfaceLocal *NEXUS_P_Surface_GetLocal_lockedTree(NEXUS_SurfaceHandle surface)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    surfaceLocal=BLST_AA_TREE_FIND(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces, surface);
    if(surfaceLocal==NULL) {
        surfaceLocal = NEXUS_P_Surface_CreateLocal_lockedTree(surface);
    }
    return surfaceLocal;
}

NEXUS_Error NEXUS_SurfaceModule_LocalInit(void)
{
    BERR_Code rc;
    BLST_AA_TREE_INIT(NEXUS_P_SurfaceLocalTree, &g_NexusSurfaceLocal.surfaces);
    rc = BKNI_CreateMutex(&g_NexusSurfaceLocal.lockTree);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
    rc = BKNI_CreateMutex(&g_NexusSurfaceLocal.lockState);
    if(rc!=BERR_SUCCESS) {
        BKNI_DestroyMutex(g_NexusSurfaceLocal.lockTree);
        return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_SurfaceModule_LocalUninit(void)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    for(surfaceLocal=BLST_AA_TREE_FIRST(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces);surfaceLocal;surfaceLocal=BLST_AA_TREE_FIRST(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces)) {
        /* Can't call NEXUS_Surface_Destroy, since SURFACE module is long gone */
        BLST_AA_TREE_REMOVE(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces, surfaceLocal);
        BKNI_Free(surfaceLocal);
    }
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    BKNI_DestroyMutex(g_NexusSurfaceLocal.lockState);
    BKNI_DestroyMutex(g_NexusSurfaceLocal.lockTree);
    g_NexusSurfaceLocal.lockTree=NULL;
    g_NexusSurfaceLocal.lockState=NULL;

    return;
}

static void NEXUS_Surface_P_Local_UpdateMemoryProperties_lockedState(struct NEXUS_SurfaceLocal *surfaceLocal)
{
    if(!surfaceLocal->memoryPropertiesValid) {
        NEXUS_Surface_GetMemoryProperties(surfaceLocal->surface, &surfaceLocal->memoryProperties);
        surfaceLocal->memoryPropertiesValid = true;
    }
    return;
}

static NEXUS_Error NEXUS_Surface_P_Local_LockPlaneAndPalette_lockedState(struct NEXUS_SurfaceLocal *surfaceLocal)
{
    NEXUS_Error rc;
    BM2MC_PACKET_Plane plane;
    NEXUS_Addr paletteOffset;

    rc = NEXUS_Surface_LockPlaneAndPalette(surfaceLocal->surface, &plane, &paletteOffset);
    if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }
    NEXUS_Surface_P_Local_UpdateMemoryProperties_lockedState(surfaceLocal);
    if(surfaceLocal->memoryProperties.pixelMemory) {
        surfaceLocal->lockedPlane = NULL;
        rc = NEXUS_MemoryBlock_Lock(surfaceLocal->memoryProperties.pixelMemory, &surfaceLocal->lockedPlane);
        if(rc == NEXUS_SUCCESS) {
            surfaceLocal->lockedPlane = (uint8_t *)surfaceLocal->lockedPlane + surfaceLocal->memoryProperties.pixelMemoryOffset;
        }
    } else {
        surfaceLocal->lockedPlane = NEXUS_OffsetToCachedAddr(plane.address);
    }
    surfaceLocal->lockedPalette = NULL;
    if(paletteOffset) {
        if(surfaceLocal->memoryProperties.paletteMemory) {
            surfaceLocal->lockedPalette = NULL;
            rc = NEXUS_MemoryBlock_Lock(surfaceLocal->memoryProperties.paletteMemory, &surfaceLocal->lockedPalette);
            if(rc == NEXUS_SUCCESS) {
                surfaceLocal->lockedPalette = (uint8_t *)surfaceLocal->lockedPalette + surfaceLocal->memoryProperties.paletteMemoryOffset;
            }
        } else {
            surfaceLocal->lockedPalette = NEXUS_OffsetToCachedAddr(paletteOffset);
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_Surface_P_Local_UpdateStatus_lockedState(struct NEXUS_SurfaceLocal *surfaceLocal)
{
    if(!surfaceLocal->statusValid) {
        NEXUS_Surface_GetStatus(surfaceLocal->surface, &surfaceLocal->status);
        surfaceLocal->statusValid = true;
    }
    return;
}


NEXUS_Error NEXUS_Surface_Lock(NEXUS_SurfaceHandle surface, void **ppMemory)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    surfaceLocal = NEXUS_P_Surface_GetLocal_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    if(surfaceLocal == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockState);
    if(!surfaceLocal->pinned) {
        if(surfaceLocal->lockCnt==0) {
            NEXUS_Error rc;
            rc = NEXUS_Surface_P_Local_LockPlaneAndPalette_lockedState(surfaceLocal);
            if(rc!=NEXUS_SUCCESS) {
                BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
                return BERR_TRACE(rc);
            }
        }
        surfaceLocal->lockCnt++;
    }
    *ppMemory = surfaceLocal->lockedPlane;
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
    return NEXUS_SUCCESS;
}

void NEXUS_Surface_Unlock(NEXUS_SurfaceHandle surface)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    surfaceLocal = NEXUS_P_Surface_GetLocal_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    if(surfaceLocal == NULL) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockState);
    if(!surfaceLocal->pinned) {
        if(surfaceLocal->lockCnt>0) {
            surfaceLocal->lockCnt--;
            if(surfaceLocal->lockCnt==0) {
                NEXUS_Surface_P_Local_UpdateMemoryProperties_lockedState(surfaceLocal);
                if(surfaceLocal->memoryProperties.pixelMemory) {
                    NEXUS_MemoryBlock_Unlock(surfaceLocal->memoryProperties.pixelMemory);
                }
                if(surfaceLocal->memoryProperties.paletteMemory) {
                    NEXUS_MemoryBlock_Unlock(surfaceLocal->memoryProperties.paletteMemory);
                }
                NEXUS_Surface_UnlockPlaneAndPalette(surface);

            }
        } else {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
    return;
}

void NEXUS_Surface_UnlockPalette(NEXUS_SurfaceHandle surface)
{
    NEXUS_Surface_Unlock(surface);
}

NEXUS_Error NEXUS_Surface_LockPalette(NEXUS_SurfaceHandle surface, void **ppMemory)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    surfaceLocal = NEXUS_P_Surface_GetLocal_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    if(surfaceLocal == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockState);
    if(!surfaceLocal->pinned) {
        if(surfaceLocal->lockCnt==0) {
            NEXUS_Error rc = NEXUS_Surface_P_Local_LockPlaneAndPalette_lockedState(surfaceLocal);
            if(rc!=NEXUS_SUCCESS) {
                BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
                return BERR_TRACE(rc);
            }
        }
        surfaceLocal->lockCnt++;
    }
    *ppMemory = surfaceLocal->lockedPalette;
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
    if(*ppMemory==NULL) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return NEXUS_SUCCESS;
}


static bool NEXUS_P_Surface_LocalRelease_lockedTree(NEXUS_SurfaceHandle surface)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;
    bool result = false;

    /* here we can't call any regular NEXUS_Surface API */
    surfaceLocal=BLST_AA_TREE_FIND(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces, surface);
    if(surfaceLocal) {
        BLST_AA_TREE_REMOVE(NEXUS_P_SurfaceLocalTree,&g_NexusSurfaceLocal.surfaces, surfaceLocal);
        if(surfaceLocal->lockCnt>0) {
            result = true;
        }
        if(surfaceLocal->memoryPropertiesValid) {
            if(surfaceLocal->memoryProperties.pixelMemory && surfaceLocal->memoryProperties.pixelMemoryTransient) {
                NEXUS_MemoryBlock_Free_local(surfaceLocal->memoryProperties.pixelMemory);
            }
            if(surfaceLocal->memoryProperties.paletteMemory && surfaceLocal->memoryProperties.paletteMemoryTransient) {
                NEXUS_MemoryBlock_Free_local(surfaceLocal->memoryProperties.paletteMemory);
            }
        }
        BKNI_Free(surfaceLocal);
    }
    return result;
}

bool NEXUS_P_Surface_LocalRelease(NEXUS_SurfaceHandle surface)
{
    bool result;

    /* this function can not call any regular NEXUS_Surface API */

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    result = NEXUS_P_Surface_LocalRelease_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    return result;
}

void NEXUS_Surface_Destroy(NEXUS_SurfaceHandle surface)
{
    if(NEXUS_P_Surface_LocalRelease(surface)) {
        NEXUS_Surface_UnlockPlaneAndPalette(surface);
    }
    NEXUS_Surface_Destroy_driver(surface);
    return ;
}

NEXUS_Error NEXUS_Surface_GetMemory(NEXUS_SurfaceHandle surface, NEXUS_SurfaceMemory *pMemory)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    surfaceLocal = NEXUS_P_Surface_GetLocal_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    if(surfaceLocal == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockState);
    if(!surfaceLocal->pinned) {
        NEXUS_Error rc;
        if(surfaceLocal->lockCnt==0) {
            rc = NEXUS_Surface_P_Local_LockPlaneAndPalette_lockedState(surfaceLocal);
            if(rc!=NEXUS_SUCCESS) {
                BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
                return BERR_TRACE(rc);
            }
        }
        surfaceLocal->lockCnt = 1;
        surfaceLocal->pinned = true;
    }
    NEXUS_Surface_P_Local_UpdateStatus_lockedState(surfaceLocal);
    pMemory->buffer = surfaceLocal->lockedPlane;
    pMemory->palette = surfaceLocal->lockedPalette;
    pMemory->pitch = surfaceLocal->status.pitch;
    pMemory->numPaletteEntries = surfaceLocal->status.numPaletteEntries;
    pMemory->bufferSize = surfaceLocal->status.height * surfaceLocal->status.pitch;
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
    return NEXUS_SUCCESS;
}

void NEXUS_Surface_Flush(NEXUS_SurfaceHandle surface)
{
    struct NEXUS_SurfaceLocal *surfaceLocal;

    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockTree);
    surfaceLocal = NEXUS_P_Surface_GetLocal_lockedTree(surface);
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockTree);
    if(surfaceLocal == NULL) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    if(!surfaceLocal->lockCnt) {
        /* Surface_Flush only works when surface is locked (or pinned) */
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }
    BKNI_AcquireMutex(g_NexusSurfaceLocal.lockState);
    NEXUS_Surface_P_Local_UpdateStatus_lockedState(surfaceLocal);
    NEXUS_FlushCache(surfaceLocal->lockedPlane, surfaceLocal->status.height*surfaceLocal->status.pitch);
    if(surfaceLocal->status.numPaletteEntries) {
        NEXUS_FlushCache(surfaceLocal->lockedPalette, sizeof(NEXUS_Pixel) * surfaceLocal->status.numPaletteEntries);
    }
    BKNI_ReleaseMutex(g_NexusSurfaceLocal.lockState);
    return;
}
