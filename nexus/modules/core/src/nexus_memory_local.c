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
*   API name: Memory
*    Specific APIs related to device memory allocation
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
/* can't include nexus_core_modules.h */
#include "nexus_base.h"
#include "nexus_memory.h"
#include "nexus_core_utils.h"
#include "nexus_core_init.h"
#include "priv/nexus_core_module_local.h"
#include "priv/nexus_base_platform.h"
#include "bmma_pool.h"

BDBG_MODULE(nexus_memory_local);

BDBG_OBJECT_ID(NEXUS_MemoryBlockLocal);
BLST_AA_TREE_HEAD(NEXUS_P_MemoryBlockAddressTree, NEXUS_MemoryBlockLocal);
BLST_AA_TREE_HEAD(NEXUS_P_MemoryMapOffsetTree, NEXUS_MemoryMapNode);

static void NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(struct NEXUS_MemoryBlockLocal *memoryBlockLocal);

struct NEXUS_CoreModule_StateLocal {
    BKNI_MutexHandle lock;
    BLST_D_HEAD(NEXUS_P_MemoryBlockLocal_List,  NEXUS_MemoryBlockLocal) localBlocks;
    struct NEXUS_P_MemoryBlockAddressTree addressTree;
    BKNI_MutexHandle mmapLock;
    BKNI_MutexHandle dataLock; /* this lock protects _only_ the 'localBlocks' data structure, no other nexus functions could be called while holding this lock */
    BMMA_PoolAllocator_Handle poolAllocator;
    struct NEXUS_P_MemoryMapOffsetTree offsetTree;
};

static struct NEXUS_CoreModule_StateLocal g_NexusCoreLocal;

#if NEXUS_MODE_proxy || NEXUS_MODE_client
/* user per-client storage */
static NEXUS_Error NEXUS_MemoryBlock_GetUserState_local(NEXUS_MemoryBlockHandle memoryBlock, NEXUS_MemoryBlockUserState *userState)
{
    return NEXUS_MemoryBlock_GetUserState(memoryBlock, userState);
}

static void NEXUS_MemoryBlock_SetUserState_local( NEXUS_MemoryBlockHandle memoryBlock, const NEXUS_MemoryBlockUserState *userState)
{
    NEXUS_MemoryBlock_SetUserState( memoryBlock, userState);
    return;
}
#else
/* local access, use static (singleton in the driver) storage */
#include "nexus_memory_priv.h"

static NEXUS_Error NEXUS_MemoryBlock_GetUserState_local(NEXUS_MemoryBlockHandle memoryBlock, NEXUS_MemoryBlockUserState *userState)
{
    if(memoryBlock->state.driver.valid) {
        *userState = memoryBlock->state.driver.state;
        return NEXUS_SUCCESS;
    }
    return NEXUS_NOT_AVAILABLE;
}

static void NEXUS_MemoryBlock_SetUserState_local( NEXUS_MemoryBlockHandle memoryBlock, const NEXUS_MemoryBlockUserState *userState)
{
    if(userState) {
        memoryBlock->state.driver.valid = true;
        memoryBlock->state.driver.state = *userState;
    } else {
        memoryBlock->state.driver.valid = false;
    }
    return;
}

void NEXUS_MemoryBlock_Release_local(NEXUS_MemoryBlockHandle memoryBlock, const NEXUS_MemoryBlockUserState *state)
{
    /* called from within driver itself, due to run-time handling of state.driver.valid, should not be ever called by the thread, that still holds g_NexusCoreLocal.lock */
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;

    memoryBlockLocal = state->state;
    BDBG_OBJECT_ASSERT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    memoryBlock->state.driver.valid = false;

    BKNI_AcquireMutex(g_NexusCoreLocal.dataLock);
    NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(memoryBlockLocal);
    BKNI_ReleaseMutex(g_NexusCoreLocal.dataLock);
    return;
}
#endif

static int NEXUS_P_MemoryMapOffset_Compare(const struct NEXUS_MemoryMapNode * node, NEXUS_Addr offset)
{
    if(offset > node->offset) {
        return 1;
    } else if(offset==node->offset) {
        return 0;
    } else {
        return -1;
    }
}

BLST_AA_TREE_GENERATE_FIND(NEXUS_P_MemoryMapOffsetTree , NEXUS_Addr, NEXUS_MemoryMapNode, offsetNode, NEXUS_P_MemoryMapOffset_Compare)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_MemoryMapOffsetTree, NEXUS_Addr, NEXUS_MemoryMapNode, offsetNode, NEXUS_P_MemoryMapOffset_Compare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_MemoryMapOffsetTree, NEXUS_MemoryMapNode, offsetNode)

void NEXUS_P_MemoryMap_InitNode(struct NEXUS_MemoryMapNode *node)
{
    node->offset = 0;
    node->sibling = NULL;
    node->lockedMem = NULL;
    return;
}

NEXUS_Error NEXUS_P_MemoryMap_Map(struct NEXUS_MemoryMapNode *node, NEXUS_Addr offset, size_t size)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct NEXUS_MemoryMapNode *sibling;
    BKNI_AcquireMutex(g_NexusCoreLocal.mmapLock);
    sibling = BLST_AA_TREE_FIND(NEXUS_P_MemoryMapOffsetTree, &g_NexusCoreLocal.offsetTree, offset);
    if(sibling) {
        BDBG_ASSERT(sibling->sibling==NULL); /* we could have only one more entry with the same offset */
        sibling->sibling = node;
        node->sibling = sibling;
        node->lockedMem = sibling->lockedMem;
        node->offset = 0; /* NULL indicated that it's a slave */
    } else {
        node->lockedMem = NEXUS_Platform_P_MapMemory(offset, size, NEXUS_MemoryMapType_eCached);
        if(node->lockedMem==NULL) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        } else {
            struct NEXUS_MemoryMapNode *nodeInserted;
            node->offset = offset;
            node->sibling = NULL;
            nodeInserted=BLST_AA_TREE_INSERT(NEXUS_P_MemoryMapOffsetTree,&g_NexusCoreLocal.offsetTree, node->offset, node);
            BDBG_ASSERT(nodeInserted == node);
        }
    }
    BKNI_ReleaseMutex(g_NexusCoreLocal.mmapLock);
    return rc;
}

void NEXUS_P_MemoryMap_Unmap(struct NEXUS_MemoryMapNode *node, size_t size)
{
    struct NEXUS_MemoryMapNode *sibling;
    BKNI_AcquireMutex(g_NexusCoreLocal.mmapLock);
    BLST_AA_TREE_REMOVE(NEXUS_P_MemoryMapOffsetTree,&g_NexusCoreLocal.offsetTree, node);
    sibling = node->sibling;
    if(sibling) { /* there is another live instance with the same mapping */
        struct NEXUS_MemoryMapNode *nodeInserted;
        BDBG_ASSERT(sibling->offset==0);
        BDBG_ASSERT(sibling->sibling==node);
        BDBG_ASSERT(sibling->lockedMem==node->lockedMem);
        sibling->offset = node->offset;
        sibling->sibling = NULL;
        nodeInserted=BLST_AA_TREE_INSERT(NEXUS_P_MemoryMapOffsetTree,&g_NexusCoreLocal.offsetTree, sibling->offset, sibling);
        BDBG_ASSERT(nodeInserted == sibling);
    } else {
        NEXUS_Platform_P_UnmapMemory(node->lockedMem, size, NEXUS_MemoryMapType_eCached);
    }
    node->sibling = NULL;
    node->offset = 0;
    node->lockedMem = NULL;
    BKNI_ReleaseMutex(g_NexusCoreLocal.mmapLock);
    return;
}


void NEXUS_Memory_FlushCache( void *pMemory, size_t numBytes )
{
    BDBG_ASSERT(NULL != pMemory);
    NEXUS_FlushCache(pMemory, numBytes);
}

NEXUS_Error NEXUS_Memory_Allocate_tagged( size_t numBytes, const NEXUS_MemoryAllocationSettings *pSettings, void **ppMemory, const char *fileName, unsigned lineNumber )
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_MemoryAllocationSettings defaultSettings;
    NEXUS_Error rc;

    if ( NULL == pSettings )
    {
        NEXUS_Memory_GetDefaultAllocationSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    block = NEXUS_MemoryBlock_Allocate_tagged(pSettings->heap, numBytes, pSettings->alignment, NULL, fileName, lineNumber);
    if(block==NULL) {return BERR_TRACE(NEXUS_NOT_AVAILABLE);}
    rc = NEXUS_MemoryBlock_Lock(block, ppMemory);
    return BERR_TRACE(rc);
}

void NEXUS_Memory_Free( void *pMemory )
{
    NEXUS_MemoryBlockHandle block;
    block = NEXUS_MemoryBlock_FromAddress(pMemory);
    if(block) {
        NEXUS_MemoryBlock_Unlock(block);
        NEXUS_MemoryBlock_Free(block);
    }
    else {
        BDBG_ERR(("NEXUS_Memory_Free unable to find block for %p", pMemory));
    }
    return;
}

void NEXUS_StopCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
#if NEXUS_TRACK_STOP_CALLBACKS
    NEXUS_Platform_P_StopCallbacks_tagged(interfaceHandle, pFileName, lineNumber, pFunctionName);
#else
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(pFunctionName);
    NEXUS_Platform_P_StopCallbacks(interfaceHandle);
#endif
    return;
}

void NEXUS_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
#if NEXUS_TRACK_STOP_CALLBACKS
    NEXUS_Platform_P_StartCallbacks_tagged(interfaceHandle, pFileName, lineNumber, pFunctionName);
#else
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(pFunctionName);
    NEXUS_Platform_P_StartCallbacks(interfaceHandle);
#endif
    return;
}

static int NEXUS_P_MemoryBlockAddress_Compare(const struct NEXUS_MemoryBlockLocal * node, void *addr)
{
    if((char *)addr > (char *)node->memoryMap.lockedMem) {
        return 1;
    } else if(addr==node->memoryMap.lockedMem) {
        return 0;
    } else {
        return -1;
    }
}

BLST_AA_TREE_GENERATE_FIND(NEXUS_P_MemoryBlockAddressTree , void *, NEXUS_MemoryBlockLocal, addressNode, NEXUS_P_MemoryBlockAddress_Compare)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_MemoryBlockAddressTree, void *, NEXUS_MemoryBlockLocal, addressNode, NEXUS_P_MemoryBlockAddress_Compare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_MemoryBlockAddressTree, NEXUS_MemoryBlockLocal, addressNode)

static struct NEXUS_MemoryBlockLocal *NEXUS_P_MemoryBlock_CreateLocal_locked(NEXUS_MemoryBlockHandle memoryBlock)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;
    NEXUS_MemoryBlockUserState userState;


    memoryBlockLocal = BMMA_PoolAllocator_Alloc(g_NexusCoreLocal.poolAllocator);
    if(!memoryBlockLocal) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BDBG_OBJECT_INIT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    BKNI_AcquireMutex(g_NexusCoreLocal.dataLock);
    BLST_D_INSERT_HEAD(&g_NexusCoreLocal.localBlocks, memoryBlockLocal, link);
    BKNI_ReleaseMutex(g_NexusCoreLocal.dataLock);
    memoryBlockLocal->memoryMap.lockedMem = NULL;
    memoryBlockLocal->lockCnt = 0;
    memoryBlockLocal->memoryBlock = memoryBlock;
    userState.state = memoryBlockLocal;
    NEXUS_P_MemoryMap_InitNode(&memoryBlockLocal->memoryMap);
    NEXUS_MemoryBlock_SetUserState_local(memoryBlock, &userState);
    return memoryBlockLocal;
}

static struct NEXUS_MemoryBlockLocal *NEXUS_P_MemoryBlock_GetLocal_locked(NEXUS_MemoryBlockHandle memoryBlock)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;
    NEXUS_Error rc;
    NEXUS_MemoryBlockUserState userState;

    rc = NEXUS_MemoryBlock_GetUserState_local(memoryBlock, &userState);
    if(rc == NEXUS_SUCCESS) {
        memoryBlockLocal =  userState.state;
        BDBG_OBJECT_ASSERT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    } else if(rc == NEXUS_NOT_AVAILABLE) {
        memoryBlockLocal = NEXUS_P_MemoryBlock_CreateLocal_locked(memoryBlock);
        if(memoryBlockLocal) {
            NEXUS_MemoryBlock_GetProperties(memoryBlock, &memoryBlockLocal->properties);
        }
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        memoryBlockLocal = NULL;
    }
    return memoryBlockLocal;
}

/* copied from bkni_track_mallocs.inc */
static const char *
b_shorten_filename(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "unknown";
    }
    for(s=pFileName;*s != '\0';s++) { } /* search forward */

    for(i=0;s!=pFileName;s--) { /* search backward */
        if(*s=='/' || *s=='\\') {
            i++;
            if(i>4) {
                return s+1;
            }
        }
    }
    return pFileName;
}

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Allocate_tagged(NEXUS_HeapHandle heap, size_t numBytes, size_t alignment, const NEXUS_MemoryBlockAllocationSettings *pSettings,
    const char *fileName, unsigned lineNumber)
{
    NEXUS_MemoryBlockHandle memoryBlock;
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;
    struct NEXUS_MemoryBlockProperties properties;
    NEXUS_MemoryBlockTag tag;

    if (fileName) {
        b_strncpy(tag.fileName, b_shorten_filename(fileName), sizeof(tag.fileName));
    }
    else {
        tag.fileName[0] = '\0';
    }
    tag.fileName[sizeof(tag.fileName)-1] = '\0'; /* ensure that last character is EOS */
    tag.lineNumber = lineNumber;
    memoryBlock = NEXUS_MemoryBlock_Allocate_driver(heap, numBytes, alignment, pSettings, &tag, &properties);
    if(memoryBlock==NULL) {
        /* no BERR_TRACE. could be normal. rely on driver error. */
        return NULL;
    }

    BKNI_AcquireMutex(g_NexusCoreLocal.lock);
    memoryBlockLocal = NEXUS_P_MemoryBlock_CreateLocal_locked(memoryBlock);
    BKNI_ReleaseMutex(g_NexusCoreLocal.lock);
    if(memoryBlockLocal) {
        memoryBlockLocal->properties = properties;
        return memoryBlockLocal->memoryBlock;
    } else {
        return memoryBlock;
    }
}

static void NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(struct NEXUS_MemoryBlockLocal *memoryBlockLocal)
{
    BLST_D_REMOVE(&g_NexusCoreLocal.localBlocks, memoryBlockLocal, link);
    BDBG_OBJECT_DESTROY(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    BMMA_PoolAllocator_Free(g_NexusCoreLocal.poolAllocator, memoryBlockLocal);
    return;
}

void NEXUS_MemoryBlock_Free_local(NEXUS_MemoryBlockHandle memoryBlock)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal=NULL;
    NEXUS_MemoryBlockUserState userState;
    NEXUS_Error rc;

    BKNI_AcquireMutex(g_NexusCoreLocal.lock);

    rc = NEXUS_MemoryBlock_GetUserState_local(memoryBlock, &userState);
    if(rc == NEXUS_SUCCESS) {
        memoryBlockLocal=userState.state;
        BDBG_OBJECT_ASSERT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    }
    if(memoryBlockLocal) {
        BDBG_ASSERT(memoryBlock == memoryBlockLocal->memoryBlock);
        if(memoryBlockLocal->memoryMap.lockedMem) {
            BLST_AA_TREE_REMOVE(NEXUS_P_MemoryBlockAddressTree,&g_NexusCoreLocal.addressTree, memoryBlockLocal);
            if( (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
                NEXUS_P_MemoryMap_Unmap(&memoryBlockLocal->memoryMap, memoryBlockLocal->properties.size);
            }
        }
#if NEXUS_MODE_proxy || NEXUS_MODE_client
        if( (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED) == NEXUS_MEMORY_TYPE_APPLICATION_CACHED &&
            (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) != NEXUS_MEMORY_TYPE_DRIVER_CACHED) {
            if(memoryBlockLocal->memoryMap.offset) {
                void *ptr = NEXUS_OffsetToCachedAddr(memoryBlockLocal->memoryMap.offset);
                if(ptr) {
                    NEXUS_FlushCache(ptr, memoryBlockLocal->properties.size);
                }
            }
        }
#endif
        NEXUS_MemoryBlock_SetUserState_local(memoryBlock, NULL);
        BKNI_AcquireMutex(g_NexusCoreLocal.dataLock);
        NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(memoryBlockLocal);
        BKNI_ReleaseMutex(g_NexusCoreLocal.dataLock);
    } else {
        BDBG_MSG(("Unknown memoryBlock %p", (void *)memoryBlock));
    }
    BKNI_ReleaseMutex(g_NexusCoreLocal.lock);
}

void NEXUS_MemoryBlock_Free(NEXUS_MemoryBlockHandle memoryBlock)
{
    NEXUS_MemoryBlock_Free_local(memoryBlock);
    NEXUS_MemoryBlock_Free_driver(memoryBlock);
    return;
}

NEXUS_Error NEXUS_CoreModule_LocalInit(void)
{
    BERR_Code rc;
    BMMA_PoolAllocator_CreateSettings poolSettings;

    BLST_AA_TREE_INIT(NEXUS_P_MemoryBlockAddressTree, &g_NexusCoreLocal.addressTree);
    BLST_AA_TREE_INIT(NEXUS_P_MemoryMapOffsetTree, &g_NexusCoreLocal.offsetTree);
    BLST_D_INIT(&g_NexusCoreLocal.localBlocks);

    rc = BKNI_CreateMutex(&g_NexusCoreLocal.lock);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_lock; }

    rc = BKNI_CreateMutex(&g_NexusCoreLocal.mmapLock);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_mmap_lock; }

    rc = BKNI_CreateMutex(&g_NexusCoreLocal.dataLock);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_data_lock; }


    BMMA_PoolAllocator_GetDefaultCreateSettings(&poolSettings);
    poolSettings.allocationSize = sizeof(struct NEXUS_MemoryBlockLocal);
    rc = BMMA_PoolAllocator_Create(&g_NexusCoreLocal.poolAllocator, &poolSettings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_pool; }

    return NEXUS_SUCCESS;

err_pool:
    BKNI_DestroyMutex(g_NexusCoreLocal.dataLock);
err_data_lock:
    BKNI_DestroyMutex(g_NexusCoreLocal.mmapLock);
err_mmap_lock:
    BKNI_DestroyMutex(g_NexusCoreLocal.lock);
err_lock:
    return rc;
}

void NEXUS_CoreModule_LocalUninit(void)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;

    BKNI_AcquireMutex(g_NexusCoreLocal.dataLock);
    while( NULL != (memoryBlockLocal=BLST_D_FIRST(&g_NexusCoreLocal.localBlocks))) {
        NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(memoryBlockLocal);
    }
    BKNI_ReleaseMutex(g_NexusCoreLocal.dataLock);
    BMMA_PoolAllocator_Destroy(g_NexusCoreLocal.poolAllocator);
    BKNI_DestroyMutex(g_NexusCoreLocal.dataLock);
    BKNI_DestroyMutex(g_NexusCoreLocal.mmapLock);
    BKNI_DestroyMutex(g_NexusCoreLocal.lock);
    g_NexusCoreLocal.lock=NULL;

    return;
}

static NEXUS_Error NEXUS_MemoryBlock_Lock_locked(NEXUS_MemoryBlockHandle memoryBlock, void **ppMemory)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;
    NEXUS_Error rc = NEXUS_SUCCESS;

    memoryBlockLocal=NEXUS_P_MemoryBlock_GetLocal_locked(memoryBlock);
    if(memoryBlockLocal==NULL) {
        *ppMemory = NULL;
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if( (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_NOT_MAPPED) == NEXUS_MEMORY_TYPE_NOT_MAPPED) {
        *ppMemory = NULL;
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    BDBG_ASSERT(memoryBlock == memoryBlockLocal->memoryBlock);
    if(memoryBlockLocal->lockCnt==0) {
        NEXUS_Addr addr;
        BERR_Code rc = NEXUS_MemoryBlock_LockOffset(memoryBlock, &addr);

        if(rc!=NEXUS_SUCCESS) {
            return BERR_TRACE(rc);
        }
        if( (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
            rc = NEXUS_P_MemoryMap_Map(&memoryBlockLocal->memoryMap, addr, memoryBlockLocal->properties.size);
            if(rc!=NEXUS_SUCCESS) {
                rc = BERR_TRACE(rc);
                memoryBlockLocal->memoryMap.lockedMem = NULL;
            }
        } else {
            memoryBlockLocal->memoryMap.offset = addr;
            memoryBlockLocal->memoryMap.lockedMem = NEXUS_OffsetToCachedAddr(addr);
        }
        if(memoryBlockLocal->memoryMap.lockedMem) {
            struct NEXUS_MemoryBlockLocal *memoryBlockLocalInserted;

            memoryBlockLocalInserted=BLST_AA_TREE_INSERT(NEXUS_P_MemoryBlockAddressTree,&g_NexusCoreLocal.addressTree, memoryBlockLocal->memoryMap.lockedMem, memoryBlockLocal);
            if(memoryBlockLocalInserted!=memoryBlockLocal) {
                /* there was another 'zomby' block that uses the same virtual address, kill it and insert new block */
                BDBG_WRN(("Remove zomby local block %p and replace it with %p", (void *)memoryBlockLocalInserted, (void *)memoryBlockLocal));
                BLST_AA_TREE_REMOVE(NEXUS_P_MemoryBlockAddressTree,&g_NexusCoreLocal.addressTree, memoryBlockLocalInserted);
                BKNI_AcquireMutex(g_NexusCoreLocal.dataLock);
                NEXUS_P_MemoryBlockLocal_Destroy_dataLocked(memoryBlockLocalInserted);
                BKNI_ReleaseMutex(g_NexusCoreLocal.dataLock);
                memoryBlockLocalInserted=BLST_AA_TREE_INSERT(NEXUS_P_MemoryBlockAddressTree,&g_NexusCoreLocal.addressTree, memoryBlockLocal->memoryMap.lockedMem, memoryBlockLocal);
            }
            BDBG_ASSERT(memoryBlockLocalInserted==memoryBlockLocal);
        }
    }
    memoryBlockLocal->lockCnt++;
    *ppMemory = memoryBlockLocal->memoryMap.lockedMem;
    if(rc==NEXUS_SUCCESS && memoryBlockLocal->memoryMap.lockedMem==NULL) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return rc;

}

NEXUS_Error NEXUS_MemoryBlock_Lock(NEXUS_MemoryBlockHandle memoryBlock, void **ppMemory)
{
    BERR_Code rc;
    BKNI_AcquireMutex(g_NexusCoreLocal.lock);
    rc = NEXUS_MemoryBlock_Lock_locked(memoryBlock, ppMemory);
    BKNI_ReleaseMutex(g_NexusCoreLocal.lock);
    return rc;
}

static void NEXUS_MemoryBlock_Unlock_locked(NEXUS_MemoryBlockHandle memoryBlock)
{
    NEXUS_Error rc;
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal = NULL;
    NEXUS_MemoryBlockUserState userState;

    rc = NEXUS_MemoryBlock_GetUserState_local(memoryBlock, &userState);
    if(rc == NEXUS_SUCCESS) {
        memoryBlockLocal = userState.state;
        BDBG_OBJECT_ASSERT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    }

    if(memoryBlockLocal==NULL) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BDBG_ASSERT(memoryBlock == memoryBlockLocal->memoryBlock);
    if(memoryBlockLocal->lockCnt<=0) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    memoryBlockLocal->lockCnt--;
    if(memoryBlockLocal->lockCnt==0) {
        if(memoryBlockLocal->memoryMap.lockedMem) {
            BLST_AA_TREE_REMOVE(NEXUS_P_MemoryBlockAddressTree,&g_NexusCoreLocal.addressTree, memoryBlockLocal);
            if( (memoryBlockLocal->properties.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
                NEXUS_P_MemoryMap_Unmap(&memoryBlockLocal->memoryMap, memoryBlockLocal->properties.size);
            }
            memoryBlockLocal->memoryMap.lockedMem = NULL;
        }
        NEXUS_MemoryBlock_UnlockOffset(memoryBlock);
    }
    return ;
}

void NEXUS_MemoryBlock_Unlock(NEXUS_MemoryBlockHandle memoryBlock)
{
    BKNI_AcquireMutex(g_NexusCoreLocal.lock);
    NEXUS_MemoryBlock_Unlock_locked(memoryBlock);
    BKNI_ReleaseMutex(g_NexusCoreLocal.lock);
    return;
}

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_FromAddress(void *pMemory)
{
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal;
    BKNI_AcquireMutex(g_NexusCoreLocal.lock);
    memoryBlockLocal = BLST_AA_TREE_FIND(NEXUS_P_MemoryBlockAddressTree, &g_NexusCoreLocal.addressTree, pMemory);
    BKNI_ReleaseMutex(g_NexusCoreLocal.lock);
    if(memoryBlockLocal) {
        BDBG_ASSERT(pMemory==memoryBlockLocal->memoryMap.lockedMem);
        return memoryBlockLocal->memoryBlock; /* bingo */
    }
    return NULL;
}

void NEXUS_MemoryBlock_CheckIfLocked( NEXUS_MemoryBlockHandle memoryBlock )
{
    NEXUS_Error rc;
    struct NEXUS_MemoryBlockLocal *memoryBlockLocal = NULL;
    NEXUS_MemoryBlockUserState userState;

    rc = NEXUS_MemoryBlock_GetUserState_local(memoryBlock, &userState);
    if(rc == NEXUS_SUCCESS) {
        memoryBlockLocal = userState.state;
        BDBG_OBJECT_ASSERT(memoryBlockLocal, NEXUS_MemoryBlockLocal);
    }

    if(memoryBlockLocal==NULL) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BDBG_ASSERT(memoryBlock == memoryBlockLocal->memoryBlock);
    if(memoryBlockLocal->lockCnt) {
        BDBG_ERR(("MemoryBlock %p has invalid lockCnt %d", (void*)memoryBlock, memoryBlockLocal->lockCnt));
        /* to find location of error in application, add BDBG_ASSERT(0) here and get
        stack trace from the core dump or oops. */
    }
}

NEXUS_Error NEXUS_Heap_GetStatus( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus )
{
    NEXUS_Error rc;
    rc = NEXUS_Heap_GetStatus_driver(heap, pStatus);
    if (!rc && (pStatus->memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED)) {
        pStatus->addr = NEXUS_OffsetToCachedAddr(pStatus->offset);
    }
    return rc;
}
