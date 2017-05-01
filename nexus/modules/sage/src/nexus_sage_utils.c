/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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



#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "nexus_platform_features.h"

static struct NEXUS_Sage_P_Utils_State {
    NEXUS_MemoryAllocationSettings defaultAllocSettings;
    NEXUS_MemoryAllocationSettings restrictedAllocSettings;
    uint8_t *restrictedAddrStart;
    uint8_t *restrictedAddrEnd;
} g_sage_utils;

BDBG_MODULE(nexus_sage);

static NEXUS_Error NEXUS_Sage_P_GetHeapConfig(NEXUS_MemoryType memoryType, uint32_t heap_index,
                                              NEXUS_MemoryAllocationSettings *allocSettings);
static void * NEXUS_Sage_P_AllocGeneric(size_t size, const NEXUS_MemoryAllocationSettings *allocSettings);

/* check if heap is accessible from both Nexus Sage and SAGE-side */
int NEXUS_Sage_P_IsHeapValid(NEXUS_HeapHandle heap, NEXUS_MemoryType memoryType)
{
    NEXUS_Error rc;
    NEXUS_MemoryStatus status;

    if (!heap) {
        return 0;
    }

    rc = NEXUS_Heap_GetStatus(heap, &status);
    if (!rc == NEXUS_SUCCESS) {
        ;/*     kindda sad :( */
        return 0;
    }
    if (((status.memoryType & memoryType) == memoryType)
#if ((BCHP_VER >= BCHP_VER_A0) && (BCHP_CHIP == 7584)) || ((BCHP_VER >= BCHP_VER_B0) && (BCHP_CHIP == 7435))
        && status.memcIndex == 0                    /* on Zeus30 chips, SAGE-side can only access MEMC0 */
#endif
        ) {
        BDBG_MSG(("%s: VALID heap=%p [offset=" BDBG_UINT64_FMT ", size=%u]", __FUNCTION__, (void *)heap, BDBG_UINT64_ARG(status.offset), status.size));
        return 1; /* the heap is valid */
    }

    return 0;
}

static NEXUS_Error NEXUS_Sage_P_GetHeapConfig(NEXUS_MemoryType memoryType, uint32_t heap_index,
                                              NEXUS_MemoryAllocationSettings *allocSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HeapHandle validHeap = NULL;
    NEXUS_HeapHandle heap = g_pCoreHandles->heap[heap_index].nexus;

    if (NEXUS_Sage_P_IsHeapValid(heap, memoryType)) {
        validHeap = heap;
    }
    else {/* default is not valid, fall back to heap search */
        int i;
        BDBG_WRN(("%s: default heap is not compatible. do a heap search.", __FUNCTION__));
        for (i = 0; i < NEXUS_MAX_HEAPS; i++) {
            heap = g_pCoreHandles->heap[i].nexus;
            if (NEXUS_Sage_P_IsHeapValid(heap, memoryType)) {
                validHeap = heap;
                break;
            }
        }
    }

    if (validHeap) {
        NEXUS_Memory_GetDefaultAllocationSettings(allocSettings);
        allocSettings->heap = validHeap;
        allocSettings->alignment = 4096; /* Align to 4096 for SAGE-side concerns */
    }
    else {
        BDBG_ERR(("%s: Cannot find default heap of type %u", __FUNCTION__, memoryType));
        rc = NEXUS_NOT_INITIALIZED;
    }

    return rc;
}

NEXUS_Error NEXUS_Sage_P_ConfigureAlloc(void)
{
    NEXUS_Error rc;

    rc = NEXUS_Sage_P_GetHeapConfig(NEXUS_MemoryType_eFull,
                                    NEXUS_MEMC0_MAIN_HEAP,
                                    &g_sage_utils.defaultAllocSettings);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Cannot get global heap config (error=%u)", __FUNCTION__, rc));
        goto end;
    }

    rc = NEXUS_Sage_P_GetHeapConfig(NEXUS_MemoryType_eSecure,
                                    NEXUS_VIDEO_SECURE_HEAP,
                                    &g_sage_utils.restrictedAllocSettings);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Cannot get restricted heap config (error=%u)", __FUNCTION__, rc));
        goto end;
    }

    {
        NEXUS_MemoryStatus status;
        rc = NEXUS_Heap_GetStatus(g_sage_utils.restrictedAllocSettings.heap,
                                  &status);
        if (rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Cannot get restricted heap status     (error=%u)", __FUNCTION__, rc));
            goto end;
        }
        g_sage_utils.restrictedAddrStart = (uint8_t *)NEXUS_OffsetToCachedAddr(status.offset);
        g_sage_utils.restrictedAddrEnd = g_sage_utils.restrictedAddrStart + status.size;
    }

end:
    return rc;
}

int NEXUS_Sage_P_IsMemoryRestricted_isrsafe(const void *address)
{
    return ((uint8_t *)address >= g_sage_utils.restrictedAddrStart) &&
           ((uint8_t *)address < g_sage_utils.restrictedAddrEnd);
}

static void * NEXUS_Sage_P_AllocGeneric(size_t size, const NEXUS_MemoryAllocationSettings *allocSettings)
{
    NEXUS_Error rc;
    void *mem;
    rc = NEXUS_Memory_Allocate(size,
                               allocSettings,
                               &mem);
    if (rc == NEXUS_SUCCESS) {
        return mem;
    }
    return NULL;
}

void * NEXUS_Sage_P_Malloc(size_t size)
{
    return NEXUS_Sage_P_AllocGeneric(size, &g_sage_utils.defaultAllocSettings);
}
void * NEXUS_Sage_P_MallocRestricted(size_t size)
{
    return NEXUS_Sage_P_AllocGeneric(size, &g_sage_utils.restrictedAllocSettings);
}
