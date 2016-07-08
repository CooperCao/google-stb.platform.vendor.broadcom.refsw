/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "bsagelib_tools.h"
#include "blst_slist.h"

BDBG_MODULE(BSAGElib_tools);

#if !(BSAGELIB_TOOLS_DISABLE_CONTAINER_CACHE)
typedef struct BSAGElib_P_Tools_ContainerItem {
    BSAGElib_InOutContainer container;
    BLST_S_ENTRY(BSAGElib_P_Tools_ContainerItem) link; /* member of a linked list */
} BSAGElib_P_Tools_ContainerItem;

BDBG_OBJECT_ID_DECLARE(BSAGElib_P_Tools_ContainerCache);
typedef struct BSAGElib_P_Tools_ContainerCache {
    BDBG_OBJECT(BSAGElib_P_Tools_ContainerCache)
    BSAGElib_MemoryAllocInterface i_memory_alloc;
    BSAGElib_SyncInterface i_sync_cache;
    BLST_S_HEAD(BSAGElib_P_Tools_ContainerItemList, BSAGElib_P_Tools_ContainerItem) head;
    uint16_t nb;
    uint16_t max;
} BSAGElib_P_Tools_ContainerCache;
#endif

#if BSAGELIB_HARDEN_MMAP
#define _iOffsetToAddr(OFFSET, LENGTH) i_memory_map->offset_to_addr(OFFSET, LENGTH)
#define _iAddrToOffset(ADDR, LENGTH) i_memory_map->addr_to_offset(ADDR, LENGTH)
#else
#define _iOffsetToAddr(OFFSET, LENGTH) i_memory_map->offset_to_addr(OFFSET)
#define _iAddrToOffset(ADDR, LENGTH) i_memory_map->addr_to_offset(ADDR)
#endif

BSAGElib_InOutContainer *
BSAGElib_Tools_ContainerOffsetToAddress(
    uint64_t containerOffset,
    BSAGElib_MemorySyncInterface *i_memory_sync,
    BSAGElib_MemoryMapInterface *i_memory_map)
{
    BSAGElib_InOutContainer *containerAddr = NULL;
    int i;

    BDBG_ENTER(BSAGElib_Tools_ContainerOffsetToAddress);

    BDBG_ASSERT(i_memory_map);
    BDBG_ASSERT(i_memory_map->offset_to_addr);
    BDBG_ASSERT(i_memory_sync);
    BDBG_ASSERT(i_memory_sync->invalidate);

    if (!containerOffset) {
        goto end;
    }

    /* convert physical addresse to virtual addresses */
    containerAddr = _iOffsetToAddr(containerOffset, sizeof(*containerAddr));
    if (!containerAddr) {
        BDBG_ERR(("%s: Cannot convert container @ offset=" BDBG_UINT64_FMT "",
                  __FUNCTION__, BDBG_UINT64_ARG(containerOffset)));
        goto end;
    }

    BDBG_MSG(("%s: container @ offset=" BDBG_UINT64_FMT " --> addr=%p",
              __FUNCTION__, BDBG_UINT64_ARG(containerOffset), (void *)containerAddr));

    /* force read from physical memory */
    i_memory_sync->invalidate((const void *)containerAddr, sizeof(*containerAddr));

    for (i = 0; i < BSAGE_CONTAINER_MAX_SHARED_BLOCKS; i++) {
        BSAGElib_SharedBlock *block = &containerAddr->blocks[i];
        if ((block->data.offset) && (block->len != 0)) {
            uint8_t *addr;
            /* convert physical addresses to local (virtual) addresses */
            addr = (uint8_t *)_iOffsetToAddr(block->data.offset, block->len);
            BDBG_MSG(("%s: #%i %d length block @ offset=" BDBG_UINT64_FMT " --> addr=%p",
                      __FUNCTION__, i, block->len, BDBG_UINT64_ARG(block->data.offset), (void *)addr));
            block->data.ptr = addr;
            /* force read from physical memory */
            i_memory_sync->invalidate((const void *)block->data.ptr, block->len);
        }
        else {
            block->data.ptr = NULL;
        }
    }
end:
    BDBG_LEAVE(BSAGElib_Tools_ContainerOffsetToAddress);
    return containerAddr;
}

uint64_t
BSAGElib_Tools_ContainerAddressToOffset(
    BSAGElib_InOutContainer *containerAddr,
    BSAGElib_MemorySyncInterface *i_memory_sync,
    BSAGElib_MemoryMapInterface *i_memory_map)
{
    int i;

    uint64_t containerOffset = 0;

    BDBG_ENTER(BSAGElib_Tools_ContainerAddressToOffset);

    BDBG_ASSERT(i_memory_map);
    BDBG_ASSERT(i_memory_map->addr_to_offset);
    BDBG_ASSERT(i_memory_sync);
    BDBG_ASSERT(i_memory_sync->flush);

    if (!containerAddr) {
        goto end;
    }

    for (i = 0; i < BSAGE_CONTAINER_MAX_SHARED_BLOCKS; i++) {
        BSAGElib_SharedBlock *block = &containerAddr->blocks[i];
        if (block->data.ptr) {
            uint64_t offset;
            if (!block->len) {
                BDBG_WRN(("%s: will not convert empty block address", __FUNCTION__));
                continue;
            }
            /* sync physical memory */
            i_memory_sync->flush((const void *)block->data.ptr, block->len);

            offset = _iAddrToOffset((const void *)block->data.ptr, block->len);
            BDBG_MSG(("%s: #%i %d length block @ addr=%p --> offset=" BDBG_UINT64_FMT "",
                      __FUNCTION__, i, block->len, block->data.ptr, BDBG_UINT64_ARG(offset)));
            block->data.offset = offset;
        }
    }

    /* finally sync the container itself */
    i_memory_sync->flush(containerAddr, sizeof(*containerAddr));
    containerOffset = _iAddrToOffset(containerAddr, sizeof(*containerAddr));
    BDBG_MSG(("%s: container @ addr=%p --> offset=" BDBG_UINT64_FMT "",
              __FUNCTION__, (void *)containerAddr, BDBG_UINT64_ARG(containerOffset)));

end:
    BDBG_LEAVE(BSAGElib_Tools_ContainerAddressToOffset);
    return containerOffset;
}

#if !(BSAGELIB_TOOLS_DISABLE_CONTAINER_CACHE)

BDBG_OBJECT_ID(BSAGElib_P_Tools_ContainerCache);

#define _BSAGElib_Tools_iLockCache()   if (hContainerCache->i_sync_cache.lock) { hContainerCache->i_sync_cache.lock(); }
#define _BSAGElib_Tools_iUnlockCache() if (hContainerCache->i_sync_cache.unlock) { hContainerCache->i_sync_cache.unlock(); }

BSAGElib_Tools_ContainerCacheHandle
BSAGElib_Tools_ContainerCache_Open(
    BSAGElib_MemoryAllocInterface *i_memory_alloc,
    BSAGElib_SyncInterface *i_sync_cache)
{
    BSAGElib_Tools_ContainerCacheHandle hContainerCache = NULL;

    BDBG_ENTER(BSAGElib_Tools_ContainerCache_Open);

    BDBG_ASSERT(i_memory_alloc);
    BDBG_ASSERT(i_memory_alloc->malloc);
    BDBG_ASSERT(i_memory_alloc->free);

    hContainerCache = (BSAGElib_Tools_ContainerCacheHandle)BKNI_Malloc(sizeof(*hContainerCache));
    if (!hContainerCache) {
        goto end;
    }

    BKNI_Memset(hContainerCache, 0, sizeof(*hContainerCache));
    BDBG_OBJECT_SET(hContainerCache, BSAGElib_P_Tools_ContainerCache);

    BLST_S_INIT(&hContainerCache->head);

    hContainerCache->i_memory_alloc = *i_memory_alloc;
    if (i_sync_cache) {
        hContainerCache->i_sync_cache = *i_sync_cache;
    }

    hContainerCache->max = ~((uint16_t)(0));

end:
    BDBG_LEAVE(BSAGElib_Tools_ContainerCache_Open);
    return hContainerCache;
}

static void
BSAGElib_Tools_ContainerCache_SetMax_UNLOCKED(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache,
    uint16_t max)
{
    hContainerCache->max = max;

    while (hContainerCache->nb > max) {
        BSAGElib_P_Tools_ContainerItem *item = BLST_S_FIRST(&hContainerCache->head);
        if (!item) {
            /* bug: nb > max >= 0 so FIRST cannot be null */
            BDBG_ERR(("%s: container cache inconsitent cache.nb=%u, max=%u, FIRST=NULL",
                      __FUNCTION__, hContainerCache->nb, max));
            break;
        }
        BLST_S_REMOVE_HEAD(&hContainerCache->head, link);
        hContainerCache->nb--;
        hContainerCache->i_memory_alloc.free(item);
    }
}

void
BSAGElib_Tools_ContainerCache_Close(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache)
{
    BDBG_ENTER(BSAGElib_Tools_ContainerCache_Close);

    BDBG_OBJECT_ASSERT(hContainerCache, BSAGElib_P_Tools_ContainerCache);

    _BSAGElib_Tools_iLockCache();
    BSAGElib_Tools_ContainerCache_SetMax_UNLOCKED(hContainerCache, 0);
    _BSAGElib_Tools_iUnlockCache();

    BDBG_OBJECT_DESTROY(hContainerCache, BSAGElib_P_Tools_ContainerCache);
    BKNI_Free(hContainerCache);

    BDBG_LEAVE(BSAGElib_Tools_ContainerCache_Close);
}


void
BSAGElib_Tools_ContainerCache_SetMax(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache,
    uint32_t max)
{
    BDBG_ENTER(BSAGElib_Tools_ContainerCache_SetMax);

    BDBG_OBJECT_ASSERT(hContainerCache, BSAGElib_P_Tools_ContainerCache);

    _BSAGElib_Tools_iLockCache();
    BSAGElib_Tools_ContainerCache_SetMax_UNLOCKED(hContainerCache, max);
    _BSAGElib_Tools_iUnlockCache();

    BDBG_LEAVE(BSAGElib_Tools_ContainerCache_SetMax);
}

BSAGElib_InOutContainer *
BSAGElib_Tools_ContainerCache_Allocate(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache)
{
    BSAGElib_P_Tools_ContainerItem *item;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(BSAGElib_Tools_ContainerCache_Allocate);

    BDBG_OBJECT_ASSERT(hContainerCache, BSAGElib_P_Tools_ContainerCache);

    _BSAGElib_Tools_iLockCache();
    item = BLST_S_FIRST(&hContainerCache->head);
    if (item) {
        /* get from cache */
        BLST_S_REMOVE_HEAD(&hContainerCache->head, link);
        hContainerCache->nb--;
        _BSAGElib_Tools_iUnlockCache();
    }
    else {
        _BSAGElib_Tools_iUnlockCache();
        /* allocate new memory block on heap outside of the mutex */
        item = (BSAGElib_P_Tools_ContainerItem *)hContainerCache->i_memory_alloc.malloc(sizeof(*item));
    }
    /* i_sync_cache lock is released. */

    if (!item) {
        BDBG_ERR(("%s: cannot allocate container item", __FUNCTION__));
        goto end;
    }

    /* Zeroed container */
    container = (BSAGElib_InOutContainer *)&item->container;
    BKNI_Memset(container, 0, sizeof(*container));

end:
    BDBG_LEAVE(BSAGElib_Tools_ContainerCache_Allocate);
    return container;
}

void
BSAGElib_Tools_ContainerCache_Free(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache,
    BSAGElib_InOutContainer *container)
{
    BSAGElib_P_Tools_ContainerItem * item = (BSAGElib_P_Tools_ContainerItem *)container;

    BDBG_ENTER(BSAGElib_Tools_ContainerCache_Free);

    BDBG_OBJECT_ASSERT(hContainerCache, BSAGElib_P_Tools_ContainerCache);

    _BSAGElib_Tools_iLockCache();
    if (hContainerCache->nb < hContainerCache->max) {
        /* put in cache */
        BLST_S_INSERT_HEAD(&hContainerCache->head, item, link);
        hContainerCache->nb++;
        _BSAGElib_Tools_iUnlockCache();
    }
    else {
        /* release heap memory outside of the i_sync_cache lock */
        _BSAGElib_Tools_iUnlockCache();
        hContainerCache->i_memory_alloc.free(item);
    }

    BDBG_LEAVE(BSAGElib_Tools_ContainerCache_Free);
    return;
}

const char *BSAGElib_Tools_ReturnCodeToString(BERR_Code returnCode)
{
    BDBG_MSG(("%s: return code = 0x%08x", __FUNCTION__, returnCode));

    switch (returnCode)
    {
    case BSAGE_ERR_HSM:
        return BSAGE_ERR_HSM_STRING;
    case BSAGE_ERR_ALREADY_INITIALIZED:
        return BSAGE_ERR_ALREADY_INITIALIZED_STRING;
    case BSAGE_ERR_INSTANCE:
        return BSAGE_ERR_INSTANCE_STRING;
    case BSAGE_ERR_INTERNAL:
        return BSAGE_ERR_INTERNAL_STRING;
    case BSAGE_ERR_SHI:
        return BSAGE_ERR_SHI_STRING;
    case BSAGE_ERR_MODULE_ID:
        return BSAGE_ERR_MODULE_ID_STRING;
    case BSAGE_ERR_PLATFORM_ID:
        return BSAGE_ERR_PLATFORM_ID_STRING;
    case BSAGE_ERR_MODULE_COMMAND_ID:
        return BSAGE_ERR_MODULE_COMMAND_ID_STRING;
    case BSAGE_ERR_SYSTEM_COMMAND_ID:
        return BSAGE_ERR_SYSTEM_COMMAND_ID_STRING;
    case BSAGE_ERR_STATE:
        return BSAGE_ERR_STATE_STRING;
    case BSAGE_ERR_CONTAINER_REQUIRED:
        return BSAGE_ERR_CONTAINER_REQUIRED_STRING;
    case BSAGE_ERR_SIGNATURE_MISMATCH:
        return BSAGE_ERR_SIGNATURE_MISMATCH_STRING;
    case BSAGE_ERR_RESET:
        return BSAGE_ERR_RESET_STRING;

        /* BinFile Manager specified return codes */
    case BSAGE_ERR_BFM_BIN_FILE_LENGTH_SC:
        return BSAGE_ERR_BFM_BIN_FILE_LENGTH_SC_STRING;
    case BSAGE_ERR_BFM_NUM_SUPPORTED_SC:
        return BSAGE_ERR_BFM_NUM_SUPPORTED_SC_STRING;
    case BSAGE_ERR_BFM_PARSE_HEADER_OFFSET:
        return BSAGE_ERR_BFM_PARSE_HEADER_OFFSET_STRING;
    case BSAGE_ERR_BFM_REGION_FIELD_SIZE:
        return BSAGE_ERR_BFM_REGION_FIELD_SIZE_STRING;
    case BSAGE_ERR_BFM_OTP_READ:
        return BSAGE_ERR_BFM_OTP_READ_STRING;
    case BSAGE_ERR_BFM_PROC_IN1_MISMATCH:
        return BSAGE_ERR_BFM_PROC_IN1_MISMATCH_STRING;
    case BSAGE_ERR_BFM_PROC_IN2_MISMATCH:
        return BSAGE_ERR_BFM_PROC_IN2_MISMATCH_STRING;
    case BSAGE_ERR_BFM_INVALID_HEADER_FORMAT:
        return BSAGE_ERR_BFM_INVALID_HEADER_FORMAT_STRING;
    case BSAGE_ERR_BFM_INVALID_BINFILE_FORMAT_TYPE1:
        return BSAGE_ERR_BFM_INVALID_BINFILE_FORMAT_TYPE1_STRING;
    case BSAGE_ERR_BFM_DRM_TYPE_NOT_FOUND:
        return BSAGE_ERR_BFM_DRM_TYPE_NOT_FOUND_STRING;
    case BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_NOT_FOUND:
        return BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_NOT_FOUND_STRING;
    case BSAGE_ERR_BFM_DATA_SECTION_HASH_MISMATCH:
        return BSAGE_ERR_BFM_DATA_SECTION_HASH_MISMATCH_STRING;
    case BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_INDEX:
       return BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_INDEX_STRING;
    case BSAGE_ERR_BFM_FILE_SIZE_SC:
       return BSAGE_ERR_BFM_FILE_SIZE_SC_STRING;
    case BSAGE_ERR_BFM_NUM_DATA_FIELDS_EXCEEDED:
       return BSAGE_ERR_BFM_NUM_DATA_FIELDS_EXCEEDED_STRING;
    case BSAGE_ERR_BFM_PROCESS_RPK_SETUP:
       return BSAGE_ERR_BFM_PROCESS_RPK_SETUP_STRING;
    case BSAGE_ERR_BFM_PROCESS_RPK_BINDING:
       return BSAGE_ERR_BFM_PROCESS_RPK_BINDING_STRING;

        /* HDCP 2.2 specified return codes */
    case BSAGE_ERR_HDCP22_STB_OWN_ID_MISMATCH:
        return BSAGE_ERR_HDCP22_STB_OWN_ID_MISMATCH_STRING;
    case BSAGE_ERR_HDCP22_INVALID_STB_OWN_ID:
        return BSAGE_ERR_HDCP22_INVALID_STB_OWN_ID_STRING;
    case BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP0_MISMATCH:
        return BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP0_MISMATCH_STRING;
    case BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP1_MISMATCH:
        return BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP1_MISMATCH_STRING;
    case BSAGE_ERR_HDCP22_INVALID_GLOBAL_KEY_OWN_ID:
        return BSAGE_ERR_HDCP22_INVALID_GLOBAL_KEY_OWN_ID_STRING;

        /* SDL related */
    case BSAGE_ERR_SDL_BAD_SLOT:
        return BSAGE_ERR_SDL_BAD_SLOT_STRING;
    case BSAGE_ERR_SDL_SLOT_IN_USE:
        return BSAGE_ERR_SDL_SLOT_IN_USE_STRING;
    case BSAGE_ERR_SDL_ALREADY_LOADED:
        return BSAGE_ERR_SDL_ALREADY_LOADED_STRING;

    case BSAGE_ERR_TA_TERMINATED:
        return BSAGE_ERR_TA_TERMINATED_STRING;

    /* SVP error codes */
    case BSAGE_ERR_SVP_VIOLATION:
        return BSAGE_ERR_SVP_VIOLATION_STRING;
    case BSAGE_ERR_SVP_INVALID_URR:
        return BSAGE_ERR_SVP_INVALID_URR_STRING;

    case BSAGE_INSUFFICIENT_HDCP_VERSION:
        return BSAGE_INSUFFICIENT_HDCP_VERSION_STRING;

    default:
        return "Not a SAGE specific return code, see BERR_Code definition";
    }
}


#endif /* BSAGELIB_TOOLS_DISABLE_CONTAINER_CACHE) */
