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
#include "priv/nexus_security_priv.h"
#include "priv/nexus_sage_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "nexus_security_client.h"
#include "bsagelib_boot.h"
#include "bkni.h"

#include "nexus_dma.h"
#include "nexus_memory.h"
/* Grant access to BCHP_SAGE_Reset in order to reset SAGE */
BERR_Code BCHP_SAGE_Reset(BREG_Handle hReg);

NEXUS_SageModule_P_Handle g_NEXUS_sageModule;


BDBG_MODULE(nexus_sage_module);

#define NEXUS_SAGE_LOGBUFFERSIZE   16*1024
#define NEXUS_SAGE_RSA2048_SIZE    256


#define _REGION_MAP_MAX_NUM 8

static struct {
    NEXUS_SageModuleSettings settings;    /* Nexus sage module settings, given in NEXUS_SageModule_Init */

    /* Memory blocks */
    NEXUS_MemoryBlockHandle sageSecureHeap; /* whole SAGE heap to reserve memory for SAGE side */
    NEXUS_SageMemoryBlock bl;             /* raw bootloader binary in memory (during SAGE boot) */
    NEXUS_SageMemoryBlock framework;      /* raw framework binary in memory (during SAGE boot) */

    /* region map contains all the regions informations (offset, size) */
    BSAGElib_RegionInfo *pRegionMap;
    uint32_t regionMapNum; /* <= _REGION_MAP_MAX_NUM */

    uint64_t logBufferHeapOffset;
    uint32_t logBufferHeapLen;

    BSAGElib_SageLogBuffer *pSageLogBuffer;
} g_sage_module;


/****************************************
 * Local functions
 ****************************************/
static NEXUS_Error NEXUS_SageModule_P_GetHeapBoundaries(int heapid, NEXUS_Addr *offset, uint32_t *len, uint32_t *max_free);
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
static NEXUS_Error NEXUS_SageModule_P_CheckHeapOverlap(NEXUS_Addr offset, uint32_t len, NEXUS_Addr boundary);
#endif
static NEXUS_Error NEXUS_SageModule_P_ConfigureSecureRegions(void);
static NEXUS_Error NEXUS_SageModule_P_InitializeTimer(void);
static void NEXUS_SageModule_P_MemoryBlockFree(NEXUS_SageMemoryBlock *block, int clear);
static NEXUS_Error NEXUS_SageModule_P_MemoryBlockAllocate(NEXUS_SageMemoryBlock *block, size_t size, NEXUS_MemoryAllocationSettings *allocSettings);
static void NEXUS_Sage_P_CleanBootVars(void);
static void NEXUS_Sage_P_MonitorBoot(void);

/****************************************
 * Macros
 ****************************************/

#define SAGE_BL_LENGTH (BCHP_SCPU_LOCALRAM_REG_END - BCHP_SCPU_LOCALRAM_REG_START + 4)

#define _SageModule_EndianSwap(PVAL) (((PVAL)[0] << 24) | ((PVAL)[1] << 16) | ((PVAL)[2] << 8) | ((PVAL)[3]))

/* BSAGElib_MemoryMap_Interface interface, see bsagelib_interfaces.h */
/* .addr_to_offset : BSAGElib_MemoryMap_AddrToOffsetCallback prototype */
static uint64_t NEXUS_Sage_P_AddrToOffsetCallback(const void *addr)
{
    return (uint64_t)NEXUS_AddrToOffset(addr);
}
/* .offset_to_addr : BSAGElib_MemoryMap_OffsetToAddrCallback prototype */
static void *NEXUS_Sage_P_OffsetToAddr(uint64_t offset)
{
    return NEXUS_OffsetToCachedAddr((uint32_t)offset);
}
/* .flush and .invalidate are both compatible with NEXUS_FlushCache,
   but we need to check if memory is restricted to avoid flushing all caches.  */
static void NEXUS_Sage_P_FlushCache_isrsafe(const void *address, size_t numBytes)
{
    if (!NEXUS_Sage_P_IsMemoryRestricted_isrsafe(address)) {
        NEXUS_FlushCache_isr(address, numBytes);
    }
}
/* BSAGElib_Sync_Interface interface, see bsagelib.h */
/* .lock_security : BSAGElib_Sync_LockCallback prototype */
static void NEXUS_Sage_P_Module_Lock_Security(void)
{
    NEXUS_Module_Lock(g_sage_module.settings.security);
}
/* .unlock_security : BSAGElib_Sync_UnlockCallback prototype */
static void NEXUS_Sage_P_Module_Unlock_Security(void)
{
    NEXUS_Module_Unlock(g_sage_module.settings.security);
}
/* .lock_sage : BSAGElib_Sync_LockCallback prototype */
static void NEXUS_Sage_P_Module_Lock_Sage(void)
{
    NEXUS_Module_Lock(g_NEXUS_sageModule.moduleHandle);
}
/* .unlock_sage : BSAGElib_Sync_UnlockCallback prototype */
static void NEXUS_Sage_P_Module_Unlock_Sage(void)
{
    NEXUS_Module_Unlock(g_NEXUS_sageModule.moduleHandle);
}

void NEXUS_SageModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    int SageBooted;
    NEXUS_SageStatus status;

    if ( &g_NEXUS_sageModule.hSAGElib != NULL) {
        BSAGElib_ImageInfo bootloaderInfo, frameworkInfo;
        BSAGElib_Boot_GetBinariesInfo(g_NEXUS_sageModule.hSAGElib,
                                      &bootloaderInfo, &frameworkInfo);
        if (bootloaderInfo.versionString[0] != 0)
            BDBG_LOG(("%s", bootloaderInfo.versionString));
        if (frameworkInfo.versionString[0] != 0)
            BDBG_LOG(("%s", frameworkInfo.versionString));
    }

    SageBooted = NEXUS_Sage_P_CheckSageBooted();
    if (SageBooted) {
        BDBG_LOG(("Sage is booted."));
    } else {
        BDBG_LOG(("Sage is not booted."));
    }

    if (NEXUS_Sage_P_Status(&status) == NEXUS_SUCCESS) {
       BDBG_LOG(("Uncompressed Restricted Region is %s.", status.urr.secured ? "SECURED" : "OPEN"));
    }
    NEXUS_Sage_P_PrintSvp();
#endif
}

/* BSAGElib_MemoryAlloc_Interface interface, see bsagelib_interfaces.h */
/* .malloc : BSAGElib_MemoryAlloc_MallocCallback prototype: NEXUS_Sage_P_Malloc */
/* .malloc_restricted : BSAGElib_MemoryAlloc_MallocCallback prototype: NEXUS_Sage_P_MallocRestricted */
/* .free : is compatible with existing NEXUS_Memory_Free */
NEXUS_Error NEXUS_Sage_P_SAGElibUpdateHsm(bool set)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm = NULL;
    BSAGElib_DynamicSettings settings;
    BERR_Code SAGElib_rc;

    if (set) {
        NEXUS_Sage_P_Module_Lock_Security();
        NEXUS_Security_GetHsm_priv(&hHsm);
        NEXUS_Sage_P_Module_Unlock_Security();
    }

    SAGElib_rc = BSAGElib_GetDynamicSettings(g_NEXUS_sageModule.hSAGElib, &settings);
    if (SAGElib_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot get SAGElib instance settings (error=%d)", __FUNCTION__, (int)SAGElib_rc));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    settings.hHsm = hHsm;
    SAGElib_rc = BSAGElib_SetDynamicSettings(g_NEXUS_sageModule.hSAGElib, &settings);
    if (SAGElib_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot set SAGElib instance settings (error=%d)", __FUNCTION__, (int)SAGElib_rc));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

end:
    return rc;
}

void NEXUS_Sage_P_SAGELogUninit(void)
{
    BSAGElib_SageLogBuffer *pSageLogBuffer = NULL;
    uint8_t *pCRRBuffer = NULL,*pEncAESKeyBuffer = NULL;

    if(g_sage_module.pSageLogBuffer)
    {
        pSageLogBuffer = g_sage_module.pSageLogBuffer;
        pCRRBuffer = NEXUS_Sage_P_OffsetToAddr(pSageLogBuffer->crrBufferOffset);
        pEncAESKeyBuffer = NEXUS_Sage_P_OffsetToAddr(pSageLogBuffer->encAESKeyOffset);

        NEXUS_Security_FreeKeySlot(pSageLogBuffer->keySlotHandle);
        if(pEncAESKeyBuffer != NULL)
        {
            NEXUS_Memory_Free(pEncAESKeyBuffer);
        }
        if(pCRRBuffer != NULL)
        {
            NEXUS_Memory_Free(pCRRBuffer);
        }
        NEXUS_Memory_Free(pSageLogBuffer);
        g_sage_module.logBufferHeapOffset = 0;
        g_sage_module.pSageLogBuffer = NULL;
    }
    return;
}

static NEXUS_Error NEXUS_Sage_P_SAGELogInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BSAGElib_SageLogBuffer *pSageLogBuffer = NULL;
    uint8_t *pCRRBuffer = NULL,*pEncAESKeyBuffer = NULL;
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;

    if (!NEXUS_GetEnv("sage_logging")) {
        /* SAGE secure logging is disabled by default. export sage_logging=y to enable. */
        return NEXUS_SUCCESS;
    }

    pSageLogBuffer = NEXUS_Sage_P_Malloc( sizeof(BSAGElib_SageLogBuffer));
    if(pSageLogBuffer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pSageLogBuffer", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }
    g_sage_module.pSageLogBuffer = pSageLogBuffer;
    pSageLogBuffer->crrBufferSize = NEXUS_SAGE_LOGBUFFERSIZE;
    pCRRBuffer = NEXUS_Sage_P_MallocRestricted(pSageLogBuffer->crrBufferSize + 16); /*Extra 16bytes for AES enc alignment*/
    if(pCRRBuffer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pCRRBuffer", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }
    pSageLogBuffer->encAESKeySize = NEXUS_SAGE_RSA2048_SIZE;
    pEncAESKeyBuffer = NEXUS_Sage_P_Malloc(pSageLogBuffer->encAESKeySize);
    if(pEncAESKeyBuffer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pEncAESKeyBuffer", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    pSageLogBuffer->keySlotHandle = (void *)NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if(pSageLogBuffer->keySlotHandle == NULL)
    {
        BDBG_ERR(("%s - Error allocating keyslot", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }

    NEXUS_KeySlot_GetInfo(pSageLogBuffer->keySlotHandle, &keyslotInfo);

    pSageLogBuffer->keyslotId = keyslotInfo.keySlotNumber;
    pSageLogBuffer->engineType = keyslotInfo.keySlotEngine;
    pSageLogBuffer->crrBufferOffset = NEXUS_Sage_P_AddrToOffsetCallback(pCRRBuffer);
    pSageLogBuffer->encAESKeyOffset = NEXUS_Sage_P_AddrToOffsetCallback(pEncAESKeyBuffer);
    g_sage_module.logBufferHeapOffset = NEXUS_Sage_P_AddrToOffsetCallback(pSageLogBuffer);
    g_sage_module.logBufferHeapLen = sizeof(BSAGElib_SageLogBuffer);

    return rc;
handle_err:
    if(pEncAESKeyBuffer != NULL)
    {
        NEXUS_Memory_Free(pEncAESKeyBuffer);
    }
    if(pCRRBuffer != NULL)
    {
        NEXUS_Memory_Free(pCRRBuffer);
    }
    if(pSageLogBuffer != NULL)
    {
        NEXUS_Memory_Free(pSageLogBuffer);
        g_sage_module.pSageLogBuffer = NULL;
    }
    return rc;
}
static NEXUS_Error NEXUS_Sage_P_SAGElibInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code SAGElib_rc;
    BHSM_Handle hHsm;
    BSAGElib_Settings settings;

    NEXUS_Sage_P_Module_Lock_Security();
    NEXUS_Security_GetHsm_priv(&hHsm);
    NEXUS_Sage_P_Module_Unlock_Security();

    BSAGElib_GetDefaultSettings(&settings);
    settings.hReg = g_pCoreHandles->reg;
    settings.hChp = g_pCoreHandles->chp;
    settings.hInt = g_pCoreHandles->bint;
    settings.hTmr = g_pCoreHandles->tmr;
    settings.hHsm  = hHsm;

    settings.i_memory_sync.flush = NEXUS_Sage_P_FlushCache_isrsafe;
    settings.i_memory_sync.invalidate = NEXUS_Sage_P_FlushCache_isrsafe;
    settings.i_memory_sync_isrsafe.flush = NEXUS_Sage_P_FlushCache_isrsafe;
    settings.i_memory_sync_isrsafe.invalidate = NEXUS_Sage_P_FlushCache_isrsafe;
    settings.i_memory_map.offset_to_addr = NEXUS_Sage_P_OffsetToAddr;
    settings.i_memory_map.addr_to_offset = NEXUS_Sage_P_AddrToOffsetCallback;

    settings.i_memory_alloc.malloc = NEXUS_Sage_P_Malloc;
    settings.i_memory_alloc.malloc_restricted = NEXUS_Sage_P_MallocRestricted;
    settings.i_memory_alloc.free = NEXUS_Memory_Free;

    settings.i_sync_sage.lock = NEXUS_Sage_P_Module_Lock_Sage;
    settings.i_sync_sage.unlock = NEXUS_Sage_P_Module_Unlock_Sage;
    settings.i_sync_hsm.lock = NEXUS_Sage_P_Module_Lock_Security;
    settings.i_sync_hsm.unlock = NEXUS_Sage_P_Module_Unlock_Security;

    {
        const char *pinmux_env = NEXUS_GetEnv("sage_log");
        if (pinmux_env) {
            int pinmux_env_val = NEXUS_atoi(pinmux_env);
            if (pinmux_env_val == 1) {
                BDBG_MSG(("%s: Realize SAGE pinmuxing in order to enable SAGE logs", __FUNCTION__));
                settings.enablePinmux = 1;
            }
        }
    }

    SAGElib_rc = BSAGElib_Open(&g_NEXUS_sageModule.hSAGElib, &settings);
    if (SAGElib_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot open SAGElib instance (error=%d)", __FUNCTION__, (int)SAGElib_rc));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    BSAGElib_GetChipInfo(g_NEXUS_sageModule.hSAGElib, &g_NEXUS_sageModule.chipInfo);

end:
    return rc;
}

/*
 * Private API between module funcs and Sage internals
 * Initialize/cleanup nexus_sage.c private vars
 */
NEXUS_Error NEXUS_Sage_P_VarInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BKNI_Memset(&g_sage_module, 0, sizeof(g_sage_module));
    BLST_S_INIT(&g_NEXUS_sageModule.instances);
    NEXUS_Sage_P_Init();

    return rc;
}

void NEXUS_Sage_P_VarCleanup(void)
{
    NEXUS_Sage_P_Cleanup();
}

/****************************************
 * Module 'public' functions
 * called by the Nexus architecture
 * upon initialization/finalization of the
 * platform (see platform_init.c, ... )
 ****************************************/

void NEXUS_SageModule_GetDefaultSettings(NEXUS_SageModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->clientHeapIndex = NEXUS_MAX_HEAPS;
}

/* Init the timer Module */
static NEXUS_Error NEXUS_SageModule_P_InitializeTimer(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BTMR_TimerSettings timerSettings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, false };
    if(BTMR_CreateTimer(g_pCoreHandles->tmr, &g_NEXUS_sageModule.hTimer, &timerSettings) != BERR_SUCCESS) {
        BDBG_ERR(("%s - BTMR_CreateTimer failure", __FUNCTION__));
        rc = NEXUS_NOT_INITIALIZED;
    }
    else {
        g_NEXUS_sageModule.timerMax = BTMR_ReadTimerMax();
        /* failure is acceptable here */
    }

    return rc;
}

/* Free a memory block allocated using NEXUS_SageModule_P_MemoryBlockAllocate() */
static void NEXUS_SageModule_P_MemoryBlockFree(NEXUS_SageMemoryBlock *block, int clear)
{
    if (block->buf) {
        if (clear) {
            BKNI_Memset(block->buf, 0, block->len);
        }
        NEXUS_Memory_Free(block->buf);
        block->buf = NULL;
    }
}

/* Allocate a memory block using alloc settings. */
static NEXUS_Error NEXUS_SageModule_P_MemoryBlockAllocate(
    NEXUS_SageMemoryBlock *block,
    size_t size,
    NEXUS_MemoryAllocationSettings *allocSettings)
{
    NEXUS_Error rc;
    void *pMem;

    rc = NEXUS_Memory_Allocate(size, allocSettings, &pMem);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Error, allocating (%u bytes)",
                  __FUNCTION__, (unsigned)size));
        goto err;
    }
    block->buf = pMem;
    block->len = size;

err:
    return rc;
}

/* Retrieve heap's offset and length parameters */
static NEXUS_Error NEXUS_SageModule_P_GetHeapBoundaries(int heapid, NEXUS_Addr *offset, uint32_t *len, uint32_t *max_free)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* Retrieve NEXUS Memory Heap info */
    if(g_pCoreHandles->heap[heapid].nexus) {
        NEXUS_MemoryStatus memoryStatus;
        rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[heapid].nexus, &memoryStatus);
        if (rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Heap_GetStatus( heapId == %d , handle == %p) failure (%u)",
                      __FUNCTION__, heapid, (void *)g_pCoreHandles->heap[heapid].nexus, rc));
            goto end;
        }

        /* Get SAGE restricted region information */
        *offset = memoryStatus.offset;
        *len = memoryStatus.size;
        if (max_free) {
            *max_free = memoryStatus.largestFreeBlock;
        }

#if 0
        BDBG_LOG(("%s - heap[%d] physical offset " BDBG_UINT64_FMT ", size %u, maxfree %d",
                  __FUNCTION__, heapid, BDBG_UINT64_ARG(*offset), *len, memoryStatus.largestFreeBlock));
#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
        /* Output error message if heap overlaps memory translation */
        rc = NEXUS_SageModule_P_CheckHeapOverlap(*offset, *len, 0x10000000);

        if (rc != NEXUS_SUCCESS) {
            goto end;
        }

        rc = NEXUS_SageModule_P_CheckHeapOverlap(*offset, *len, 0x40000000);

        if (rc != NEXUS_SUCCESS) {
            goto end;
        }
#endif
    }
    else {
        BDBG_WRN(("%s - heap[%d].nexus NOT available", __FUNCTION__, heapid));
        rc = NEXUS_NOT_AVAILABLE;
    }

end:
    return rc;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
/* Check if heap overlap a certain boundary */
static NEXUS_Error NEXUS_SageModule_P_CheckHeapOverlap(NEXUS_Addr offset, uint32_t len, NEXUS_Addr boundary)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* The boundary can't be inside the specified range */
    if (boundary > offset && boundary < offset + len) {
        rc = NEXUS_INVALID_PARAMETER;

        BDBG_ERR(("%s - Error, heap (offset: " BDBG_UINT64_FMT ", size: %u bytes) can't cross address " BDBG_UINT64_FMT ")",
                  __FUNCTION__, BDBG_UINT64_ARG(offset), len, BDBG_UINT64_ARG(boundary)));
    }

    return rc;
}
#endif

BERR_Code
NEXUS_SageModule_P_AddRegion(
    uint32_t id,
    NEXUS_Addr offset,
    uint32_t size)
{
    uint32_t offset32 = (uint32_t)offset;
    BERR_Code rc = BERR_SUCCESS;

    if ((NEXUS_Addr)offset32 != offset) {
        BDBG_ERR(("%s - Error, out of band offset for region 0x%02x",
                  __FUNCTION__, id));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    if (g_sage_module.regionMapNum >= _REGION_MAP_MAX_NUM) {
        BDBG_ERR(("%s: cannot add new heap (%u); increase _REGION_MAP_MAX_NUM",
                  __FUNCTION__, id));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    BDBG_MSG(("%s: Adding region 0x%02x [.offset=0x%08x, .size=%u]",
              __FUNCTION__, id, offset32, size));

    g_sage_module.pRegionMap[g_sage_module.regionMapNum].id = id;
    g_sage_module.pRegionMap[g_sage_module.regionMapNum].offset = offset32;
    g_sage_module.pRegionMap[g_sage_module.regionMapNum].size = size;;
    g_sage_module.regionMapNum++;

end:
    return rc;
}

/* Get Secure heap boundaries
 * Allocate the full SAGE secure heap to reserve memory for SAGE-side
 * Retrieve the Secure Video heap boundaries */
static NEXUS_Error NEXUS_SageModule_P_ConfigureSecureRegions(void)
{
    NEXUS_Error rc;
    uint32_t sage_max_free = 0;
    NEXUS_Addr offset, offset2;
    uint32_t size;

    /* Add SRR */
    rc = NEXUS_SageModule_P_GetHeapBoundaries(NEXUS_SAGE_SECURE_HEAP, &offset, &size, &sage_max_free);
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Allocate the whole SAGE heap (for monitor Address Range Checker) */
    g_sage_module.sageSecureHeap = NEXUS_MemoryBlock_Allocate(g_pCoreHandles->heap[NEXUS_SAGE_SECURE_HEAP].nexus, sage_max_free, 0, NULL);
    if (!g_sage_module.sageSecureHeap) { rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto err; }

    rc = NEXUS_MemoryBlock_LockOffset(g_sage_module.sageSecureHeap, &offset2);
    if (rc || offset2 != offset) {
        BDBG_ERR(("%s - Error, returned block does not start at 0x%08x",
                  __FUNCTION__, (uint32_t)offset));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* TODO: Add a check to verify that memory allocated is bigger than a threshold necessary for SAGE usage */
    BDBG_MSG(("%s - SAGE secure heap[%d].nexus %d bytes @ 0x%08x",
              __FUNCTION__, NEXUS_SAGE_SECURE_HEAP,
              sage_max_free, (unsigned)offset));

    rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Srr, offset, size);
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Add CRR */
    rc = NEXUS_SageModule_P_GetHeapBoundaries(NEXUS_VIDEO_SECURE_HEAP, &offset, &size, NULL);
    if (rc != NEXUS_SUCCESS) { goto err; }

    rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Crr, offset, size);
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Add XRR */
    rc = NEXUS_SageModule_P_GetHeapBoundaries(NEXUS_EXPORT_HEAP, &offset, &size, NULL);
    if (rc == NEXUS_SUCCESS) {
        if (offset && size) {
            rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Xrr, offset, size);
            if (rc != NEXUS_SUCCESS) { goto err; }
        }
    }
    else {
        BDBG_WRN(("%s: No Export heap available", __FUNCTION__));
        rc = BERR_SUCCESS; /* this is not fatal */
    }

err:
    return rc;
}



/* Initialize Sage Module. This is called during platform initialication. */
NEXUS_ModuleHandle NEXUS_SageModule_Init(const NEXUS_SageModuleSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ModuleSettings moduleSettings;
    int booted = 0;
    NEXUS_Addr offset;
    uint32_t size;

    BDBG_ASSERT(!g_NEXUS_sageModule.moduleHandle);
    BDBG_ASSERT(pSettings);

    BKNI_Memset(&g_NEXUS_sageModule, 0, sizeof(g_NEXUS_sageModule));
    g_NEXUS_sageModule.reset = 1;

    rc = BKNI_CreateEvent(&g_NEXUS_sageModule.sageReadyEvent);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(( "Error creating sage sageReadyEvent" ));
        rc = NEXUS_NOT_AVAILABLE;
        goto err_unlocked;
    }
    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eDefault;
    moduleSettings.dbgPrint = NEXUS_SageModule_Print;
    moduleSettings.dbgModules = "nexus_sage_module";
    g_NEXUS_sageModule.moduleHandle = NEXUS_Module_Create("sage", &moduleSettings);
    if (!g_NEXUS_sageModule.moduleHandle) {
        rc = NEXUS_NOT_AVAILABLE;
        goto err_unlocked;
    }

    NEXUS_LockModule();

    rc = NEXUS_Sage_P_VarInit();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    g_sage_module.settings = *(NEXUS_SageModuleSettings *)pSettings;

    rc = NEXUS_Sage_P_ConfigureAlloc();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    /* configure timer for message timestamp and SAGE start time */
    rc = NEXUS_SageModule_P_InitializeTimer();
    if (rc != NEXUS_SUCCESS) { goto err; }

    g_sage_module.regionMapNum = 0;
    g_sage_module.pRegionMap = NEXUS_Sage_P_Malloc(sizeof(*g_sage_module.pRegionMap) * _REGION_MAP_MAX_NUM);
    if (g_sage_module.pRegionMap == NULL) {
        BDBG_ERR(("%s: cannot allocate memory for region map", __FUNCTION__));
        rc = NEXUS_OUT_OF_DEVICE_MEMORY;
        goto err;
    }

    /* GLR: get general heap boundaries */
    rc = NEXUS_SageModule_P_GetHeapBoundaries(SAGE_FULL_HEAP, &offset, &size, NULL);
    if (rc != NEXUS_SUCCESS) { goto err; }
    rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Glr, offset, size);
    if (rc != NEXUS_SUCCESS) { goto err; }

    switch (g_sage_module.settings.clientHeapIndex) {
    case NEXUS_SAGE_SECURE_HEAP:
    case NEXUS_VIDEO_SECURE_HEAP:
    case SAGE_FULL_HEAP:
        BDBG_ERR(("%s: heap index %u is already mapped",
                  __FUNCTION__, g_sage_module.settings.clientHeapIndex));
    case NEXUS_MAX_HEAPS:
        break;
    default:
        rc = NEXUS_SageModule_P_GetHeapBoundaries(g_sage_module.settings.clientHeapIndex,
                                                  &offset, &size, NULL);
        if (rc != NEXUS_SUCCESS) { goto err; }
        rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Glr2, offset, size);
        if (rc != NEXUS_SUCCESS) { goto err; }
        break;
    }

    /* SRR/CRR/... Get secure heaps info and allocate all Sage memory */
    rc = NEXUS_SageModule_P_ConfigureSecureRegions();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    /* Set URR regions (accounting for adjacent buffers) */
    rc = NEXUS_Sage_P_SvpInit();
    if (rc != NEXUS_SUCCESS) { goto err; }
    NEXUS_Sage_P_SvpSetRegions();

    rc = NEXUS_Sage_P_SAGElibInit();
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* To allow watchdog capability from the beginning */
    /* !! SAGElib has to be initialized first */
    rc = NEXUS_Sage_P_WatchdogInit();
    if (rc != NEXUS_SUCCESS) { goto err; }

    rc = NEXUS_SageModule_P_Start();
    if (rc != NEXUS_SUCCESS) { goto err; }

    booted = NEXUS_Sage_P_CheckSageBooted();
    if (!booted) { rc = NEXUS_INVALID_PARAMETER; goto err; }

    /* success */
err:
    NEXUS_UnlockModule();
err_unlocked:
    if (rc != NEXUS_SUCCESS) {
        /* failed to init... */;
       NEXUS_SageModule_Uninit();
    }

    return g_NEXUS_sageModule.moduleHandle;
}

NEXUS_Error Nexus_SageModule_P_Img_Create(
    const char *id,             /* Image Name */
    void **ppContext,           /* [out] Context */
    BIMG_Interface  *pInterface /* [out] Pointer to Image interface */
    )
{
    NEXUS_Error rc;
#if defined(NEXUS_MODE_driver)
    rc = Nexus_Core_P_Img_Create(id, ppContext, pInterface);
#else
    BSTD_UNUSED(id);
    *ppContext = SAGE_IMAGE_Context;
    *pInterface = SAGE_IMAGE_Interface;
    rc = NEXUS_SUCCESS;
#endif
    return rc;
}

void Nexus_SageModule_P_Img_Destroy(
    void *pContext              /* Context returned by previous call */
    )
{
#if defined(NEXUS_MODE_driver)
    Nexus_Core_P_Img_Destroy(pContext);
#else
    BSTD_UNUSED(pContext);
#endif
}

/* Start Sage instance: initialize and start Sage-side boot process
 * Can be called multiple time to restart Sage ( in watchdog for instance )
 * In case of restart, NEXUS_Sage_P_CleanBootVars() must be called first. */
NEXUS_Error NEXUS_SageModule_P_Start(void)
{
    NEXUS_Error rc;
    NEXUS_SageMemoryBlock bl = {0, NULL}; /* raw bootloader binary in memory */
    NEXUS_SageImageHolder frameworkImg =
        {"Framework image", SAGE_IMAGE_FirmwareID_eFramework,     NULL};
    NEXUS_SageImageHolder blImg     =
        {"Bootloader",   SAGE_IMAGE_FirmwareID_eBootLoader, NULL};
    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;

    frameworkImg.raw = &g_sage_module.framework;
    blImg.raw = &bl;

    /* Start SVP */
    rc = NEXUS_Sage_P_SvpStart();
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        blImg.id = SAGE_IMAGE_FirmwareID_eBootLoader_Development;
        frameworkImg.id = SAGE_IMAGE_FirmwareID_eFramework_Development;
    }

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot use IMG interface", __FUNCTION__));
        goto err;
    }

    /* Load SAGE bootloader into memory */
    rc = NEXUS_SageModule_P_Load(&blImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* Load SAGE framework into memory */
    rc = NEXUS_SageModule_P_Load(&frameworkImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* Get start timer */
    BTMR_ReadTimer(g_NEXUS_sageModule.hTimer, &g_NEXUS_sageModule.initTimeUs);
    BDBG_MSG(("%s - Initial timer value: %u", __FUNCTION__, g_NEXUS_sageModule.initTimeUs));
    rc = NEXUS_Sage_P_SAGELogInit();
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Put SAGE CPU out of reset */
    {
        BERR_Code magnum_rc;
        BSAGElib_BootSettings bootSettings;

        BSAGElib_Boot_GetDefaultSettings(&bootSettings);

        bootSettings.pBootloader = bl.buf;
        bootSettings.bootloaderSize = bl.len;
        bootSettings.pFramework = g_sage_module.framework.buf;
        bootSettings.frameworkSize = g_sage_module.framework.len;
        bootSettings.pRegionMap = g_sage_module.pRegionMap;
        bootSettings.regionMapNum = g_sage_module.regionMapNum;
        bootSettings.logBufferOffset = (uint32_t)g_sage_module.logBufferHeapOffset;
        bootSettings.logBufferSize = g_sage_module.logBufferHeapLen;

        magnum_rc = BSAGElib_Boot_Launch(g_NEXUS_sageModule.hSAGElib, &bootSettings);
        if(magnum_rc != BERR_SUCCESS) {
            rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
            BDBG_ERR(("%s - BSAGElib_Boot_Launch() fails %d", __FUNCTION__, magnum_rc));
            goto err;
        }
    }


    /* success */
    g_NEXUS_sageModule.reset = 0;
    /* end of init is deferred in first NEXUS_Sage_Open() call */

    /* Check if SAGE BOOT happened and unblock BSageLib */
    NEXUS_Sage_P_MonitorBoot();

    /* Init AR */
    NEXUS_Sage_P_ARInit();

    /* Set this event so that everybody else knows SAGE is booted up */
    BKNI_SetEvent(g_NEXUS_sageModule.sageReadyEvent);

err:
    if (img_context) {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }

    /* Wipe Bootloader */
    NEXUS_SageModule_P_MemoryBlockFree(&bl, false);

    return rc;
}

/* Clean all temporary variables that are used during boot process. */
/* This event handler is called whenever bootCleanAction registered event is set.
 * The bootCleanAction event is set inside NEXUS_Sage_P_MonitorBoot which is executed
 * inside a dedicated thread.
 * The main idea is that bootCleanAction Event handling is realised in sync
 * i.e. Nexus Sage module is locked in the meanwhile. */
static void NEXUS_Sage_P_CleanBootVars(void)
{
    BDBG_MSG(("%s - cleaning", __FUNCTION__));

    if (g_NEXUS_sageModule.hSAGElib) {
        BSAGElib_Boot_Clean(g_NEXUS_sageModule.hSAGElib);
    }
    NEXUS_SageModule_P_MemoryBlockFree(&g_sage_module.framework, false); /* framework header freed here as well */
}

/* Uninitialize Sage Module and wipe associated resources. */
void NEXUS_SageModule_Uninit(void)
{
    int booted = 0;

    if (!g_NEXUS_sageModule.moduleHandle) {
        return;
    }

    /* if appropriate wait for SAGE to become ready OR to fail */
    booted = NEXUS_Sage_P_CheckSageBooted();

    NEXUS_LockModule();

    NEXUS_Sage_P_ARUninit();

    NEXUS_Sage_P_SvpStop(false);
    NEXUS_Sage_P_SvpUninit();

    NEXUS_Sage_P_CleanBootVars();

    NEXUS_Sage_P_WatchdogUninit();

    NEXUS_Sage_P_SAGELogUninit();

    NEXUS_SageModule_P_PrivClean();

    if (g_NEXUS_sageModule.hSAGElib) {
        BSAGElib_Close(g_NEXUS_sageModule.hSAGElib);
        g_NEXUS_sageModule.hSAGElib = NULL;
    }

    if (booted) {
        BCHP_SAGE_Reset(g_pCoreHandles->reg);
    }

    NEXUS_MemoryBlock_Free(g_sage_module.sageSecureHeap);

    if (g_sage_module.pRegionMap != NULL) {
        NEXUS_Memory_Free(g_sage_module.pRegionMap);
        g_sage_module.pRegionMap = NULL;
    }

    if (g_NEXUS_sageModule.hTimer) {
        BTMR_DestroyTimer(g_NEXUS_sageModule.hTimer);
        g_NEXUS_sageModule.hTimer = NULL;
    }

    NEXUS_Sage_P_VarCleanup();
    NEXUS_UnlockModule();

    BKNI_DestroyEvent(g_NEXUS_sageModule.sageReadyEvent);

    NEXUS_Module_Destroy(g_NEXUS_sageModule.moduleHandle);
    g_NEXUS_sageModule.moduleHandle = NULL;
}

/* Load a Sage binary (bootloader or framework) located on the file system into memory */
NEXUS_Error NEXUS_SageModule_P_Load(
    NEXUS_SageImageHolder *holder,
    BIMG_Interface *img_interface,
    void *img_context)
{
    void *image = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* Prepare memory to load binfile */
    {
        uint32_t *size = NULL;

        /* Open file */
        rc = img_interface->open(img_context, &image, holder->id);
        if(rc != NEXUS_SUCCESS) {
            BDBG_LOG(("%s - Error opening SAGE '%s' file",
                      __FUNCTION__, holder->name));
            goto err;
        }

        /* Get size */
        rc = img_interface->next(image, 0, (const void **)&size, sizeof(uint32_t));
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error while reading '%s' file to get size",
                      __FUNCTION__, holder->name));
            goto err;
        }

        /* Allocate buffer to save data */
        { /* PADD CHEAT : for bootloader, allocate at least SAGE_BL_LENGTH and pad the rest with zeroes */
            uint32_t alloc_size = *size;
            if (holder->id == SAGE_IMAGE_FirmwareID_eBootLoader) {
                if (alloc_size < SAGE_BL_LENGTH) {
                    alloc_size = SAGE_BL_LENGTH;
                    BDBG_MSG(("%s - adjusting BL size to %d bytes\n", __FUNCTION__, SAGE_BL_LENGTH));
                }
            }

            BDBG_MSG(("alloc '%s' %u bytes", holder->name, alloc_size));
            /* TODO: use nexus_sage_util functions for alloc */
            rc = NEXUS_SageModule_P_MemoryBlockAllocate(holder->raw, alloc_size/* PADD CHEAT */, NULL);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Error allocating %u bytes memory for '%s' buffer",
                          __FUNCTION__, *size, holder->name));
                goto err;
            }

            if (*size < alloc_size) {
                BKNI_Memset((void *)((uint8_t *)holder->raw->buf+*size), 0, alloc_size - *size);
            }
            holder->raw->len = *size;
        } /* PADD CHEAT : end */
    }

    /* Load file into memory: read SAGE_IMG_BUFFER_SIZE bytes at once */
    {
        uint32_t loop_size = holder->raw->len;
        uint8_t *buffer_ex = holder->raw->buf;
        unsigned chunk = 0;

        while (loop_size) {
            void *data = NULL;
            const uint16_t to_read =
                (loop_size >= SAGE_IMG_BUFFER_SIZE) ? (SAGE_IMG_BUFFER_SIZE - 1) : loop_size;

            rc = img_interface->next(image, chunk, (const void **)&data, to_read);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Error while reading '%s' file", __FUNCTION__, holder->name));
                goto err;
            }

            /* BDBG_MSG(("%s - Read %u bytes from file (chunk: %u)", __FUNCTION__, to_read, chunk)); */
            BKNI_Memcpy(buffer_ex, data, to_read);
            loop_size -= to_read;
            buffer_ex += to_read;
            chunk++;
        }
    }

    /* Sync physical memory for all areas */
    NEXUS_Memory_FlushCache(holder->raw->buf, holder->raw->len);
    BDBG_MSG(("%s - '%s' Raw@%p,  size=%u", __FUNCTION__,
              holder->name, (void *)holder->raw->buf, (unsigned)holder->raw->len));

err:
    /* in case of error, Memory block is freed in NEXUS_SageModule_Uninit() */
    if (image) {
        img_interface->close(image);
    }

    return rc;
}

/* SAGE boot timing */
#define SAGE_MAX_BOOT_TIME_US (15 * 1000 * 1000)
#define SAGE_STEP_BOOT_TIME_US (50 * 1000)
/* Check is SAGE software is ready.
 * If not, waits for 15 seconds max to see if it becomes ready. */
/* prototype must be compatible with NEXUS_Thread_Create::pThreadFunc parameter */

/* SAGE ARC Prescreen error codes */
#define SAGE_SECUREBOOT_ARC_PRESCREEN_NUMBER_BAD_BIT_EXCEEDED  (0x60D)
#define SAGE_SECUREBOOT_ARC_PRESCREEN_MSP_PROG_FAILURE         (0x60E)

static void
NEXUS_Sage_P_MonitorBoot(void)
{
    int totalBootTimeUs;
    int alreadyConsumedBootTimeUs;
    int overheadUs;
    uint32_t timer;
    uint32_t lastStatus=0x42;
    BSAGElib_BootState bootState;

    g_NEXUS_sageModule.booted = 0;

    /* This will calculate the time already consumed by the SAGE init process before we reach this point */
    if(BTMR_ReadTimer(g_NEXUS_sageModule.hTimer, &timer) != BERR_SUCCESS) {
        BDBG_ERR(("%s - BTMR_ReadTimer failure", __FUNCTION__));
        goto err;
    }
    /* compute already comsumed time. */
    if (g_NEXUS_sageModule.initTimeUs < timer) {
        totalBootTimeUs = timer - g_NEXUS_sageModule.initTimeUs;
    } else { /* timer < g_NEXUS_sageModule.timerMax) ; timer wraps */
        /* if real total boot time > g_NEXUS_sageModule.timerMax (SAGE-side is probably hanging),
         * it may end up in SAGE_MAX_BOOT_TIME_US micro seconds before timeout
         * as total boot time computation will be false due to wrong wrap. */
        totalBootTimeUs = ((g_NEXUS_sageModule.timerMax - g_NEXUS_sageModule.initTimeUs) + timer);
    }

    alreadyConsumedBootTimeUs = totalBootTimeUs;
    /* Read Global SRAM registers for SAGE boot status every SAGE_STEP_BOOT_TIME_US us.
       Max to SAGE_MAX_BOOT_TIME_US us */

    do {
        BSAGElib_Boot_GetState(g_NEXUS_sageModule.hSAGElib, &bootState);
        if(bootState.status == BSAGElibBootStatus_eStarted) {
            break;
        }
        else if (bootState.status != lastStatus) {
            BDBG_MSG(("%s - last_status=%u,new_status=%u,total=%d us,overhead=%d us",
                      __FUNCTION__, lastStatus,bootState.status,totalBootTimeUs,
                      totalBootTimeUs - alreadyConsumedBootTimeUs));
            lastStatus = bootState.status;
        }
        if (bootState.status == BSAGElibBootStatus_eError) {
            if (bootState.lastError) {
                BDBG_ERR(("%s -  SAGE boot detected error", __FUNCTION__));
                goto err;
            }
        }

        if (totalBootTimeUs > SAGE_MAX_BOOT_TIME_US) {
            BDBG_ERR(("%s - SAGE boot exceeds %d ms. Failure!!!!!!", __FUNCTION__, SAGE_MAX_BOOT_TIME_US));
            goto err;
        }
        BKNI_Sleep(SAGE_STEP_BOOT_TIME_US/1000);
        totalBootTimeUs += SAGE_STEP_BOOT_TIME_US;
    } while (1);

    overheadUs = totalBootTimeUs - alreadyConsumedBootTimeUs;
    if (overheadUs > 0) {
        BDBG_MSG(("%s - SAGE boot tooks %d us, overhead is %d us",
                  __FUNCTION__, totalBootTimeUs, overheadUs));
    }
    else {
        BDBG_MSG(("%s - SAGE booted, boot time is unknown", __FUNCTION__));
    }

    /* Initialize bypass keyslot */
    {
        BERR_Code magnum_rc;

        magnum_rc = BSAGElib_Boot_Post(g_NEXUS_sageModule.hSAGElib);
        if (magnum_rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Boot_Post() fails %d.", __FUNCTION__, magnum_rc));
            goto err;
        }
    }

    BDBG_MSG(("SAGE Boot done"));

err:
    if (bootState.status != BSAGElibBootStatus_eStarted) {
        char *_flavor = (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) ? "_dev" : "";
        BDBG_ERR(("*******  SAGE ERROR  >>>>>"));
        BDBG_ERR(("* The SAGE side software cannot boot."));
        if (lastStatus == BSAGElibBootStatus_eBlStarted) {
            BDBG_ERR(("* SAGE Framework hangs"));
            BDBG_ERR(("* Please check your sage_framework%s.bin", _flavor));
        }
        else if (lastStatus == BSAGElibBootStatus_eError) {
            BDBG_ERR(("* SAGE Bootloader error: 0x%08x", bootState.lastError));
            switch(bootState.lastError)
            {
                case SAGE_SECUREBOOT_ARC_PRESCREEN_NUMBER_BAD_BIT_EXCEEDED:
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    BDBG_ERR(("* SAGE ARC Bad Bit Management: More than 2 SAGE ARC Bad bits"));
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    break;
                case SAGE_SECUREBOOT_ARC_PRESCREEN_MSP_PROG_FAILURE:
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    BDBG_ERR(("* SAGE ARC Bad Bit Management: MSP OTP programming error"));
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    break;
                default:
                    BDBG_ERR(("* Please check your sage_bl%s.bin and sage_framework%s.bin", _flavor, _flavor));
            }

        }
        else {
            BDBG_ERR(("* Reboot the system and try again."));
            BDBG_ERR(("* SAGE Bootloader status: 0x%08x", lastStatus));
            BDBG_ERR(("* SAGE Bootloader hangs"));
            BDBG_ERR(("* Please check your sage_bl%s.bin", _flavor));
        }
        BDBG_ERR(("*******  SAGE ERROR  <<<<<"));
        BKNI_Sleep(200);
        BDBG_ASSERT(false && "SAGE CANNOT BOOT");
    }
    NEXUS_Sage_P_CleanBootVars();
}


int NEXUS_Sage_P_CheckSageBooted(void)
{
    BERR_Code rc;

    if (!g_NEXUS_sageModule.booted) {
        BDBG_MSG(("BKNI_WaitForEvent"));
        rc = BKNI_WaitForEvent(g_NEXUS_sageModule.sageReadyEvent, BKNI_INFINITE);
        if (rc == BERR_SUCCESS) {
            g_NEXUS_sageModule.booted = 1;
        }
    }
    return g_NEXUS_sageModule.booted;
}

NEXUS_Error NEXUS_Sage_P_Status(NEXUS_SageStatus *pStatus)
{
    BSAGElib_Status sageStatus;
    BERR_Code rc_magnum;

    if (!g_NEXUS_sageModule.hSAGElib) {
       return NEXUS_NOT_AVAILABLE;
    }

    rc_magnum = BSAGElib_GetStatus(g_NEXUS_sageModule.hSAGElib, &sageStatus);
    if (rc_magnum != BERR_SUCCESS) {
       BDBG_ERR(("Failure to get SAGE status."));
       return (NEXUS_Error)rc_magnum;
    }

    pStatus->urr.secured = sageStatus.urr.secured;
    {
        BSAGElib_ImageInfo frameworkInfo;
        BSAGElib_Boot_GetBinariesInfo(g_NEXUS_sageModule.hSAGElib, NULL, &frameworkInfo);
        pStatus->framework.THLShortSig = frameworkInfo.THLShortSig;
        BDBG_ASSERT(sizeof(pStatus->framework.version) == sizeof(frameworkInfo.version));
        BKNI_Memcpy(pStatus->framework.version, frameworkInfo.version, sizeof(pStatus->framework.version));;
    }

    return NEXUS_SUCCESS;
}
static void CompleteCallback ( void *pParam, int iParam )
{
    BSTD_UNUSED ( iParam );
    BKNI_SetEvent ( pParam );
}

NEXUS_Error NEXUS_Sage_P_GetEncKey(uint8_t *pKeyBuff, uint32_t inputKeyBufSize,uint32_t *pOutKeySize)
{
    BSAGElib_SageLogBuffer *pSageLogBuffer = NULL;
    NEXUS_Error     rc = NEXUS_SUCCESS;
    uint8_t *pEncAESBuffer = NULL;

    if((pKeyBuff == NULL)||(pOutKeySize == NULL))
    {
        return NEXUS_INVALID_PARAMETER;
    }

    if(inputKeyBufSize < NEXUS_SAGE_RSA2048_SIZE)
    {
        return NEXUS_INVALID_PARAMETER;
    }

    if (!g_NEXUS_sageModule.hSAGElib) {
       return NEXUS_NOT_AVAILABLE;
    }

    pSageLogBuffer = NEXUS_Sage_P_OffsetToAddr(g_sage_module.logBufferHeapOffset);
    pEncAESBuffer = NEXUS_Sage_P_OffsetToAddr(pSageLogBuffer->encAESKeyOffset);

    *pOutKeySize = NEXUS_SAGE_RSA2048_SIZE;
    BKNI_Memcpy((void *)pKeyBuff, (void *)pEncAESBuffer, *pOutKeySize);
    return rc;
}

NEXUS_Error NEXUS_Sage_P_GetLogBuffer(uint8_t *pBuff, uint32_t inputBufSize,
                                      uint32_t *pBufSize, uint32_t *pWrapBufSize,/*Buffers aligned for 16byte*/
                                      uint32_t *pActualBufSize, uint32_t *pActualWrapBufSize)
{
    BERR_Code rc_magnum;
    NEXUS_Error     rc = NEXUS_SUCCESS;
    BSAGElib_SageLogBuffer *pSageLogBuffer = NULL;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;
    BKNI_EventHandle dmaEvent = NULL;
    uint8_t *pDest = NULL, *pDest1 = NULL;
    uint8_t *pCRRBuffer = NULL;

    uint64_t writeCount = 0;
    static uint64_t prevWriteCount = 0;

    uint32_t writeOffset = 0;
    static uint32_t rdOffset = 0;
    bool wrap = false;
    uint32_t bufLen = 0,wrapBufLen = 0;

    if(pBuff == NULL)
    {
        return NEXUS_INVALID_PARAMETER;
    }

    if (!g_NEXUS_sageModule.hSAGElib) {
       return NEXUS_NOT_AVAILABLE;
    }
    rc_magnum = BSAGElib_GetLogWriteCount(g_NEXUS_sageModule.hSAGElib, &writeCount);
    if (rc != BERR_SUCCESS) {
       BDBG_ERR(("Failure to BSAGElib_GetLogbuffer."));
       return (NEXUS_Error)rc_magnum;
    }

    if(prevWriteCount == writeCount)
    {
        BDBG_MSG(("Nothing to process. Hence return"));
        *pBufSize = 0;
        *pWrapBufSize = 0;
        *pActualBufSize = 0;
        *pActualWrapBufSize = 0;
        return rc;
    }
    pSageLogBuffer = NEXUS_Sage_P_OffsetToAddr(g_sage_module.logBufferHeapOffset);
    NEXUS_Memory_FlushCache(pSageLogBuffer, sizeof(BSAGElib_SageLogBuffer));

    writeOffset = (writeCount % pSageLogBuffer->crrBufferSize);
    if((writeCount - prevWriteCount) > pSageLogBuffer->crrBufferSize)
    {
        wrap = true;
        rdOffset = writeOffset;
    }
    else if(writeOffset < rdOffset)
    {
        wrap = true;
    }

    if(wrap == true)
    {
        bufLen = pSageLogBuffer->crrBufferSize - rdOffset;
        wrapBufLen = writeOffset;
    }
    else
    {
        bufLen = writeOffset-rdOffset;
        wrapBufLen = 0;
    }
    if(inputBufSize < (bufLen + wrapBufLen))
    {
        return NEXUS_INVALID_PARAMETER;
        /*Minimum buffer required is bufLen + wrapBufLen
        buffDiff = (bufLen + wrapBufLen)-inputBufSize;
        if(wrapBufLen > buffDiff)
        {
            wrapBufLen -= buffDiff;
        }
        else
        {
            bufLen -= (buffDiff-wrapBufLen);
            wrapBufLen = 0;
        }*/
    }
    pCRRBuffer = NEXUS_Sage_P_OffsetToAddr(pSageLogBuffer->crrBufferOffset);
    NEXUS_Memory_FlushCache(pCRRBuffer, pSageLogBuffer->crrBufferSize);

    /* Open DMA handle */
    dma = NEXUS_Dma_Open ( NEXUS_ANY_ID, NULL );
    /* and DMA event */
    BKNI_CreateEvent ( &dmaEvent );
    *pActualBufSize = bufLen;
    if(bufLen&0xF)
    {
        bufLen = ((bufLen>>4)+1)<<4;
    }
    rc = NEXUS_Memory_Allocate(bufLen, NULL, (void*)&pDest );
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    BKNI_Memset ( pDest, 0, bufLen * sizeof ( unsigned char ) );
    NEXUS_Memory_FlushCache(pDest, bufLen);

    NEXUS_DmaJob_GetDefaultSettings ( &jobSettings );
    jobSettings.numBlocks = 1;
    jobSettings.keySlot = pSageLogBuffer->keySlotHandle;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;

    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;
    jobSettings.useRPipe = true;

    dmaJob = NEXUS_DmaJob_Create ( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings ( &blockSettings );
    blockSettings.pSrcAddr = &pCRRBuffer[rdOffset];
    blockSettings.pDestAddr = pDest;
    blockSettings.blockSize = bufLen;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
    blockSettings.cached = false;

    rc = NEXUS_DmaJob_ProcessBlocks ( dmaJob, &blockSettings, 1 );
    if ( rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
        NEXUS_DmaJob_GetStatus ( dmaJob, &jobStatus );
        BDBG_ASSERT ( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
        rc = NEXUS_SUCCESS;
    }
    NEXUS_Memory_FlushCache(pDest, bufLen);
    BKNI_Memcpy((void *)pBuff, (void *)pDest, bufLen);

    if(wrapBufLen)
    {
        *pActualWrapBufSize = wrapBufLen;
        if(wrapBufLen&0xF)/*%16*/
        {
            wrapBufLen = ((wrapBufLen>>4)+1)<<4;/*((wrapBufLen/16)+1)*16;*/
        }
        rc = NEXUS_Memory_Allocate ( wrapBufLen, NULL, ( void * ) &pDest1 );
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
        BKNI_Memset ( pDest1, 0, wrapBufLen * sizeof ( unsigned char ) );
        NEXUS_Memory_FlushCache(pDest1, wrapBufLen);

        NEXUS_DmaJob_GetDefaultBlockSettings ( &blockSettings );
        blockSettings.pSrcAddr = pCRRBuffer;
        blockSettings.pDestAddr = pDest1;
        blockSettings.blockSize = wrapBufLen;
        blockSettings.resetCrypto = true;
        blockSettings.scatterGatherCryptoStart = true;
        blockSettings.scatterGatherCryptoEnd = true;
        blockSettings.cached = false;

        rc = NEXUS_DmaJob_ProcessBlocks ( dmaJob, &blockSettings, 1 );
        if ( rc == NEXUS_DMA_QUEUED )
        {
            BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
            NEXUS_DmaJob_GetStatus ( dmaJob, &jobStatus );
            BDBG_ASSERT ( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
            rc = NEXUS_SUCCESS;
        }
        NEXUS_Memory_FlushCache(pDest1, wrapBufLen);
        BKNI_Memcpy((void *)&pBuff[bufLen], (void *)pDest1, wrapBufLen);
    }

    rdOffset = writeOffset;
    prevWriteCount = writeCount;

    if(pBuff != NULL)
    {
        *pBufSize = bufLen+wrapBufLen;
        *pWrapBufSize = wrapBufLen;
    }
    NEXUS_DmaJob_Destroy ( dmaJob );
    NEXUS_Dma_Close(dma);

error:
    if(pDest1 != NULL)
    {
        NEXUS_Memory_Free(pDest1);
    }

    if(pDest != NULL)
    {
        NEXUS_Memory_Free(pDest);
    }
    return rc;
}
