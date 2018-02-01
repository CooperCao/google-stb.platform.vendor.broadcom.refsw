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

#include "nexus_sage_module.h"

#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "priv/nexus_sage_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_security_client.h"
#endif
#include "bsagelib_boot.h"
#include "bkni.h"
#include "bchp_bsp_glb_control.h"
#include "priv/bsagelib_shared_types.h"
#include "priv/nexus_security_regver_priv.h"
#if (NEXUS_SECURITY_API_VERSION==1)
#include "bhsm_verify_reg.h"
#endif

#include "nexus_dma.h"
#include "nexus_memory.h"
#include "priv/nexus_sage_audio.h"

NEXUS_SageModule_P_Handle g_NEXUS_sageModule;


BDBG_MODULE(nexus_sage_module);

#define NEXUS_SAGE_LOGBUFFERSIZE   16*1024
#define NEXUS_SAGE_RSA2048_SIZE    256


#define _REGION_MAP_MAX_NUM 12

#define SAGE_RESET_REG       BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eReset)
#define SAGE_CRR_START_REG   BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eCRRStartOffset)
#define SAGE_CRR_END_REG     BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eCRREndOffset)
#define SAGE_SRR_START_REG   BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSRRStartOffset)
#define SAGE_SRR_END_REG     BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSRREndOffset)

#define SAGE_RESETVAL_DOWN  0x1FF1FEED /* Defined in multiple places */

static struct {
    NEXUS_SageModuleSettings settings; /* Nexus sage module settings, given in NEXUS_SageModule_Init */
    NEXUS_SageModuleInternalSettings internalSettings; /* Nexus sage module Internal settings, given in NEXUS_SageModule_Init */

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
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
static NEXUS_Error NEXUS_SageModule_P_CheckHeapOverlap(NEXUS_Addr offset, uint32_t len, NEXUS_Addr boundary);
#endif
static NEXUS_Error NEXUS_SageModule_P_ConfigureSecureRegions(void);
static void NEXUS_SageModule_P_MemoryBlockFree(NEXUS_SageMemoryBlock *block, int clear);
static NEXUS_Error NEXUS_SageModule_P_MemoryBlockAllocate(NEXUS_SageMemoryBlock *block, size_t size);
static void NEXUS_Sage_P_CleanBootVars(void);
static NEXUS_Error NEXUS_Sage_P_MonitorBoot(void);
static NEXUS_Error NEXUS_Sage_P_CheckSecureRegions(int heapid, NEXUS_Addr offset, uint32_t len);

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
        size_t roundedSize = RoundUpP2(numBytes, SAGE_ALIGN_SIZE);
        NEXUS_FlushCache_isr(address, roundedSize);
    }
}
/* BSAGElib_Sync_Interface interface, see bsagelib.h */
/* .lock_security : BSAGElib_Sync_LockCallback prototype */
static void NEXUS_Sage_P_Module_Lock_Security(void)
{
    NEXUS_Module_Lock(g_sage_module.internalSettings.security);
}
/* .unlock_security : BSAGElib_Sync_UnlockCallback prototype */
static void NEXUS_Sage_P_Module_Unlock_Security(void)
{
    NEXUS_Module_Unlock(g_sage_module.internalSettings.security);
}
/* Lock transport */
void NEXUS_Sage_P_Module_Lock_Transport(void)
{
    NEXUS_Module_Lock(g_sage_module.internalSettings.transport);
}
/* unlock transport*/
void NEXUS_Sage_P_Module_Unlock_Transport(void)
{
    NEXUS_Module_Unlock(g_sage_module.internalSettings.transport);
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

static void NEXUS_SageModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    int SageBooted;
    NEXUS_SageStatus status;

    if ( g_NEXUS_sageModule.hSAGElib != NULL) {
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
    NEXUS_Sage_P_PrintSecureLog();
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
        BDBG_ERR(("%s: Cannot get SAGElib instance settings (error=%d)", BSTD_FUNCTION, (int)SAGElib_rc));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    settings.hHsm = hHsm;
    SAGElib_rc = BSAGElib_SetDynamicSettings(g_NEXUS_sageModule.hSAGElib, &settings);
    if (SAGElib_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot set SAGElib instance settings (error=%d)", BSTD_FUNCTION, (int)SAGElib_rc));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

end:
    return rc;
}

void NEXUS_Sage_P_SAGELogUninit(void)
{
#if (NEXUS_SECURITY_API_VERSION==1)
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
#else
    BDBG_ERR(("%s: TBD not supported", BSTD_FUNCTION));
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
#endif
}

static NEXUS_Error NEXUS_Sage_P_SAGELogInit(void)
{
#if (NEXUS_SECURITY_API_VERSION==1)
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
        BDBG_ERR(("%s - Error allocating pSageLogBuffer", BSTD_FUNCTION));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }
    g_sage_module.pSageLogBuffer = pSageLogBuffer;
    pSageLogBuffer->crrBufferSize = NEXUS_SAGE_LOGBUFFERSIZE;
    pCRRBuffer = NEXUS_Sage_P_MallocRestricted(pSageLogBuffer->crrBufferSize + 16); /*Extra 16bytes for AES enc alignment*/
    if(pCRRBuffer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pCRRBuffer", BSTD_FUNCTION));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }
    pSageLogBuffer->encAESKeySize = NEXUS_SAGE_RSA2048_SIZE;
    pEncAESKeyBuffer = NEXUS_Sage_P_Malloc(pSageLogBuffer->encAESKeySize);
    if(pEncAESKeyBuffer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pEncAESKeyBuffer", BSTD_FUNCTION));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto handle_err;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    pSageLogBuffer->keySlotHandle = (void *)NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if(pSageLogBuffer->keySlotHandle == NULL)
    {
        BDBG_ERR(("%s - Error allocating keyslot", BSTD_FUNCTION));
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
#else
    BDBG_ERR(("%s: TBD not supported", BSTD_FUNCTION));
    return 0;
#endif
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
                BDBG_MSG(("%s: Realize SAGE pinmuxing in order to enable SAGE logs", BSTD_FUNCTION));
                settings.enablePinmux = 1;
            }
        }
    }

    SAGElib_rc = BSAGElib_Open(&g_NEXUS_sageModule.hSAGElib, &settings);
    if (SAGElib_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot open SAGElib instance (error=%d)", BSTD_FUNCTION, (int)SAGElib_rc));
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

static NEXUS_Error NEXUS_Sage_P_WaitSageRegion(void)
{
#if (NEXUS_SECURITY_API_VERSION==1)
    uint8_t count=0;
    NEXUS_SecurityRegionInfoQuery  regionSatus;
    BERR_Code rc=NEXUS_SUCCESS;

    NEXUS_Sage_P_Module_Lock_Security();
    NEXUS_Security_RegionQueryInformation_priv(&regionSatus);
    NEXUS_Sage_P_Module_Unlock_Security();

    BDBG_MSG(("Waiting for ScpuFsbl to be undefined"));

    while(regionSatus.regionStatus[NEXUS_SecurityRegverRegionID_eScpuFsbl] & REGION_STATUS_DEFINED)
    {
        if(count++>50)
        {
            BDBG_ERR(("Timeout waiting for ScpuFsbl to become undefined"));
            rc=NEXUS_TIMEOUT;
            goto EXIT;
        }
        BKNI_Sleep(100);

        NEXUS_Sage_P_Module_Lock_Security();
        NEXUS_Security_RegionQueryInformation_priv(&regionSatus);
        NEXUS_Sage_P_Module_Unlock_Security();
    }
    BDBG_MSG(("ScpuFsbl undefined"));

EXIT:
    return rc;
#else
    BDBG_ERR(("%s: TBD not supported", BSTD_FUNCTION));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
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
#if !defined(NEXUS_MODE_driver)
    NEXUS_SageImage_SetImageExists_priv(pSettings);
#endif
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
}

void NEXUS_SageModule_GetDefaultInternalSettings(NEXUS_SageModuleInternalSettings *pInternalSettings)
{
    BKNI_Memset(pInternalSettings, 0, sizeof(*pInternalSettings));
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
    size_t size)
{
    NEXUS_Error rc = BERR_SUCCESS;
    void *pMem;

    pMem = NEXUS_Sage_P_Malloc(size);
    if (pMem == NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        BDBG_ERR(("%s - Error, allocating (%u bytes)",
                  BSTD_FUNCTION, (unsigned)size));
        goto err;
    }
    block->buf = pMem;
    block->len = size;

err:
    return rc;
}

/* Retrieve heap's offset and length parameters */
NEXUS_Error NEXUS_SageModule_P_GetHeapBoundaries(int heapid, NEXUS_Addr *offset, uint32_t *len, uint32_t *max_free)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* Retrieve NEXUS Memory Heap info */
    if(g_pCoreHandles->heap[heapid].nexus) {
        NEXUS_MemoryStatus memoryStatus;
        rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[heapid].nexus, &memoryStatus);
        if (rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Heap_GetStatus( heapId == %d , handle == %p) failure (%u)",
                      BSTD_FUNCTION, heapid, (void *)g_pCoreHandles->heap[heapid].nexus, rc));
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
                  BSTD_FUNCTION, heapid, BDBG_UINT64_ARG(*offset), *len, memoryStatus.largestFreeBlock));
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

        if ((heapid == NEXUS_SAGE_SECURE_HEAP) ||
            (heapid == NEXUS_VIDEO_SECURE_HEAP)) {
            rc = NEXUS_Sage_P_CheckSecureRegions(heapid, memoryStatus.offset, memoryStatus.size);
            if (rc != NEXUS_SUCCESS) {
                goto end;
            }
        }
    }
    else {
        /* Only warn about manditory heaps */
        switch(heapid)
        {
            case NEXUS_SAGE_SECURE_HEAP:
            case NEXUS_VIDEO_SECURE_HEAP:
            case SAGE_FULL_HEAP:
                BDBG_WRN(("%s - heap[%d].nexus NOT available", BSTD_FUNCTION, heapid));
                break;
            default:
                break;
        }
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
                  BSTD_FUNCTION, BDBG_UINT64_ARG(offset), len, BDBG_UINT64_ARG(boundary)));
    }

    return rc;
}
#endif

/* Check if Secure heap CRR and SRR are consistent to what's in GlobalSRAM */
static NEXUS_Error NEXUS_Sage_P_CheckSecureRegions(int heapid, NEXUS_Addr offset, uint32_t len)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t size = 0;
    const char *heap_str = NULL;

    switch (heapid) {
    case NEXUS_VIDEO_SECURE_HEAP:
        start = BREG_Read32(g_pCoreHandles->reg, SAGE_CRR_START_REG);
        end = BREG_Read32(g_pCoreHandles->reg, SAGE_CRR_END_REG);
        heap_str = "CRR (NEXUS_VIDEO_SECURE_HEAP)";
        break;
    case NEXUS_SAGE_SECURE_HEAP:
        start = BREG_Read32(g_pCoreHandles->reg, SAGE_SRR_START_REG);
        end = BREG_Read32(g_pCoreHandles->reg, SAGE_SRR_END_REG);
        heap_str = "SRR (NEXUS_SAGE_SECURE_HEAP)";
        break;
    default:
        rc = NEXUS_NOT_SUPPORTED;
        BDBG_ERR(("%s: Unsupported secure region", BSTD_FUNCTION));
        goto end;
    }
    if (start || end) {
        /* translate size and offset per Zeus architecture */
        size = RESTRICTED_REGION_SIZE(start,end);
        start = RESTRICTED_REGION_OFFSET(start);
        if ((offset != (NEXUS_Addr)start) || (len != size)) {
            rc = NEXUS_INVALID_PARAMETER;
            BDBG_ERR(("%s - Error, heap '%s' [ID=%d] can only be set once per power up.",
                       BSTD_FUNCTION, heap_str, heapid));
            BDBG_ERR(("%s\tPrevious run configured it at (offset: %#x, size: %u bytes)",
                      BSTD_FUNCTION, start, size));
            BDBG_ERR(("%s\tCurrent run requests it at (offset: " BDBG_UINT64_FMT ", size: %u bytes)",
                      BSTD_FUNCTION, BDBG_UINT64_ARG(offset), len));
            BDBG_ERR(("%s - A change in '%s' requires a reboot.", BSTD_FUNCTION, heap_str));
            goto end;
        }

    }

end:
    return rc;
}

BERR_Code
NEXUS_SageModule_P_AddRegion(
    uint32_t id,
    NEXUS_Addr offset,
    uint32_t size)
{
    uint32_t offset32 = (uint32_t)offset;
    BERR_Code rc = BERR_SUCCESS;

    if (g_sage_module.regionMapNum >= _REGION_MAP_MAX_NUM) {
        BDBG_ERR(("%s: cannot add new heap (%u); increase _REGION_MAP_MAX_NUM",
                  BSTD_FUNCTION, id));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if ((NEXUS_Addr)offset32 != offset) {
        NEXUS_Addr offsetTmp;

        BDBG_MSG(("Using 40bit shift for region ID 0x%x", id));
        id = REGION_ID_SET_FLAG(id, REGION_ID_FLAG_40BIT);
        offsetTmp = REGION_ID_FLAG_40BIT_OFFSET_SET(offset);
        offset32 = (uint32_t)offsetTmp;

        if ((NEXUS_Addr)offset32 != offsetTmp) {
            BDBG_ERR(("%s - Error, out of bounds offset for region 0x%02x",
                      BSTD_FUNCTION, id));
            rc = NEXUS_INVALID_PARAMETER;
            goto end;
        }
    }

    BDBG_MSG(("%s: Adding region 0x%02x [.offset=0x%08x, .size=%u]",
              BSTD_FUNCTION, id, offset32, size));

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
                  BSTD_FUNCTION, (uint32_t)offset));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* TODO: Add a check to verify that memory allocated is bigger than a threshold necessary for SAGE usage */
    BDBG_MSG(("%s - SAGE secure heap[%d].nexus %d bytes @ 0x%08x",
              BSTD_FUNCTION, NEXUS_SAGE_SECURE_HEAP,
              sage_max_free, (unsigned)offset));

    rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Srr, offset, size);
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Add CRR */
    rc = NEXUS_SageModule_P_GetHeapBoundaries(NEXUS_VIDEO_SECURE_HEAP, &offset, &size, NULL);
    if (rc != NEXUS_SUCCESS) { goto err; }

    rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_Crr, offset, size);
    if (rc != NEXUS_SUCCESS) { goto err; }

err:
    return rc;
}



/* Initialize Sage Module. This is called during platform initialication. */
NEXUS_ModuleHandle NEXUS_SageModule_Init(const NEXUS_SageModuleSettings *pSettings,
                                         const NEXUS_SageModuleInternalSettings *pInternalSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ModuleSettings moduleSettings;
    int booted = 0;
    NEXUS_Addr offset;
    uint32_t size;

    BDBG_ASSERT(!g_NEXUS_sageModule.moduleHandle);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pInternalSettings);

    BKNI_Memset(&g_NEXUS_sageModule, 0, sizeof(g_NEXUS_sageModule));

    if((BREG_Read32(g_pCoreHandles->reg, BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT)
        & BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT_SCPU_SW_INIT_MASK))
    {
        g_NEXUS_sageModule.reset = 1;
    }

    rc = BKNI_CreateEvent(&g_NEXUS_sageModule.sageReadyEvent);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(( "Error creating sage sageReadyEvent" ));
        rc = NEXUS_NOT_AVAILABLE;
        goto err_unlocked;
    }
    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(moduleSettings.priority, &pSettings->common);
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

    g_sage_module.settings = *pSettings;
    g_sage_module.internalSettings = *pInternalSettings;

    rc = NEXUS_Sage_P_ConfigureAlloc();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    g_sage_module.regionMapNum = 0;
    g_sage_module.pRegionMap = NEXUS_Sage_P_Malloc(sizeof(*g_sage_module.pRegionMap) * _REGION_MAP_MAX_NUM);
    if (g_sage_module.pRegionMap == NULL) {
        BDBG_ERR(("%s: cannot allocate memory for region map", BSTD_FUNCTION));
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
                  BSTD_FUNCTION, g_sage_module.settings.clientHeapIndex));
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
    rc = NEXUS_Sage_P_SvpInit(&g_sage_module.internalSettings);
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
    BERR_Code magnum_rc;
    BSAGElib_BootSettings bootSettings;
    bool reconnect = false;

    g_NEXUS_sageModule.SWState = NEXUS_SageSWState_eUnknown;

    /* Check agian (may be called for watchdog/standby) */
    if((BREG_Read32(g_pCoreHandles->reg, BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT)
        & BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT_SCPU_SW_INIT_MASK)) {
        g_NEXUS_sageModule.reset = 1;
        reconnect = false;
    } else {
        g_NEXUS_sageModule.reset = 0;
        reconnect = true;
    }

    frameworkImg.raw = &g_sage_module.framework;
    blImg.raw = &bl;

    /* Stop SVP if already started */
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
        BDBG_ERR(("%s - Cannot use IMG interface", BSTD_FUNCTION));
        goto err;
    }

    /* Load SAGE bootloader into memory */
    rc = NEXUS_SageModule_P_Load(&blImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* Load SAGE framework into memory */
    rc = NEXUS_SageModule_P_Load(&frameworkImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* Get start timer */
    NEXUS_Time_Get(&g_NEXUS_sageModule.initTime);

    rc = NEXUS_Sage_P_SAGELogInit();
    if (rc != NEXUS_SUCCESS) { goto err; }

    BSAGElib_Boot_GetDefaultSettings(&bootSettings);

    bootSettings.pBootloader = bl.buf;
    bootSettings.bootloaderSize = bl.len;
    bootSettings.pFramework = g_sage_module.framework.buf;
    bootSettings.frameworkSize = g_sage_module.framework.len;
    bootSettings.pRegionMap = g_sage_module.pRegionMap;
    bootSettings.regionMapNum = g_sage_module.regionMapNum;
    bootSettings.logBufferOffset = (uint32_t)g_sage_module.logBufferHeapOffset;
    bootSettings.logBufferSize = g_sage_module.logBufferHeapLen;

    if(BREG_Read32(g_pCoreHandles->reg, SAGE_RESET_REG)==SAGE_RESETVAL_DOWN)
    {
        /* In case of previous shutdown.. wait for sage to be down */
        if(NEXUS_Sage_P_WaitSageRegion()!=NEXUS_SUCCESS)
        {
            BDBG_ERR(("Failed waiting for SAGE shutdown. Continue, but SAGE may not function"));
        }
    }

    BDBG_MSG(("SAGE is %s", g_NEXUS_sageModule.reset ? "STOPPED" : "RUNNING"));

    if(g_NEXUS_sageModule.reset) {
        /* Take SAGE CPU out of reset */
        magnum_rc = BSAGElib_Boot_Launch(g_NEXUS_sageModule.hSAGElib, &bootSettings);
        if(magnum_rc != BERR_SUCCESS) {
            char *_flavor = (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) ? "_dev" : "";
            BDBG_ERR(("%s - BSAGElib_Boot_Launch() failed [0x%x]", BSTD_FUNCTION, magnum_rc));
            BDBG_ERR(("*******  SAGE ERROR  >>>>>"));
            BDBG_ERR(("* The SAGE Bootloader cannot be loaded."));
            BDBG_ERR(("* Please check your sage_bl%s.bin", _flavor));
            BDBG_ERR(("*******  SAGE ERROR  <<<<<"));
        }
    } else {
        /* Do some checks, that we can restart w/ SAGE already running */
        magnum_rc = BSAGElib_Boot_HostReset(g_NEXUS_sageModule.hSAGElib, &bootSettings);
    }

    if(magnum_rc != BERR_SUCCESS) {
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto err;
    }

    /* success */
    g_NEXUS_sageModule.reset = 0;
    /* end of init is deferred in first NEXUS_Sage_Open() call */

    /* Check if SAGE BOOT happened and unblock BSageLib */
    rc = NEXUS_Sage_P_MonitorBoot();
    if(rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto err;
    }

    /* Init AR and system Crit */
    rc = NEXUS_Sage_P_ARInit(&g_sage_module.settings);
    if(rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto err;
    }

    NEXUS_Sage_P_SecureLog_Init(&g_sage_module.settings);
    NEXUS_Sage_P_SvpInitDelayed(NULL);

    /* Install BP3 TA.  Initialize the platform and BP3 module.  Read and process bp3.bin if it exists and not provisioning.  */
#if (BCHP_CHIP == 7278 && BCHP_VER < BCHP_VER_B0)
    rc= NEXUS_SUCCESS;
#else
    rc = NEXUS_Sage_P_BP3Init(&g_sage_module.settings);
#endif
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto err;
    }

#if NEXUS_SAGE_SARM_TEST /* BDSP_ARM_AUDIO_SUPPORT && !BDSP_RAAGA_AUDIO_SUPPORT */
    /* Install SARM TA.  Initialize the platform and SARM module.  */
    rc = NEXUS_Sage_P_SARMInit(&g_sage_module.settings);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto err;
    }
#endif

    if(reconnect)
    {
        /* Need to have SAGE verify region info has not changed.... */
        rc = NEXUS_Sage_P_SystemCritRestartCheck((void *)&bootSettings);
        if(rc != NEXUS_SUCCESS)
        {
            rc = BERR_TRACE(rc);
            goto err;
        }
    }

    /* No issues detected getting SW running */
    g_NEXUS_sageModule.SWState = NEXUS_SageSWState_eRunning;

err:
    if(rc != NEXUS_SUCCESS)
    {
        /* Some error happened in the SW boot process */
        g_NEXUS_sageModule.SWState = NEXUS_SageSWState_eError;
    }

    /* Set this event so that everyone (NEXUS_Sage_P_CheckSageBooted) knows SAGE is booted up
    * or otherwise FAILED to start */
    BKNI_SetEvent(g_NEXUS_sageModule.sageReadyEvent);

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
    BDBG_MSG(("%s - cleaning", BSTD_FUNCTION));

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

    NEXUS_Sage_P_ARUninit(BSAGElib_eStandbyModeOn);

    NEXUS_Sage_P_SARMUninit();

    NEXUS_Sage_P_SecureLog_Uninit();

#if (BCHP_CHIP != 7278 || BCHP_VER != BCHP_VER_A0)
    NEXUS_Sage_P_BP3Uninit();
#endif
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

        if(BREG_Read32(g_pCoreHandles->reg, SAGE_RESET_REG)==SAGE_RESETVAL_DOWN)
        {
            if(NEXUS_Sage_P_WaitSageRegion()!=NEXUS_SUCCESS)
            {
                BDBG_ERR(("Failed waiting for SAGE shutdown"));
            }
        }
    }

    NEXUS_MemoryBlock_Free(g_sage_module.sageSecureHeap);

    if (g_sage_module.pRegionMap != NULL) {
        NEXUS_Memory_Free(g_sage_module.pRegionMap);
        g_sage_module.pRegionMap = NULL;
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

    if(!g_sage_module.settings.imageExists[holder->id]) {
        /* Just because a file is not present, does not mean error... */
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Prepare memory to load binfile */
    {
        uint32_t *size = NULL;

        /* Open file */
        rc = img_interface->open(img_context, &image, holder->id);
        if(rc != NEXUS_SUCCESS) {
            BDBG_LOG(("%s - Error opening SAGE '%s' file",
                      BSTD_FUNCTION, holder->name));
            goto err;
        }

        /* Get size */
        rc = img_interface->next(image, 0, (const void **)&size, sizeof(uint32_t));
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error while reading '%s' file to get size",
                      BSTD_FUNCTION, holder->name));
            goto err;
        }

        /* Allocate buffer to save data */
        { /* PADD CHEAT : for bootloader, allocate at least SAGE_BL_LENGTH and pad the rest with zeroes */
            uint32_t alloc_size = *size;
            if (holder->id == SAGE_IMAGE_FirmwareID_eBootLoader) {
                if (alloc_size < SAGE_BL_LENGTH) {
                    alloc_size = SAGE_BL_LENGTH;
                    BDBG_MSG(("%s - adjusting BL size to %d bytes", BSTD_FUNCTION, SAGE_BL_LENGTH));
                }
            }

            BDBG_MSG(("alloc '%s' %u bytes", holder->name, alloc_size));
            /* TODO: use nexus_sage_util functions for alloc */
            rc = NEXUS_SageModule_P_MemoryBlockAllocate(holder->raw, alloc_size/* PADD CHEAT */);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Error allocating %u bytes memory for '%s' buffer",
                          BSTD_FUNCTION, *size, holder->name));
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
                BDBG_ERR(("%s - Error while reading '%s' file", BSTD_FUNCTION, holder->name));
                goto err;
            }

            /* BDBG_MSG(("%s - Read %u bytes from file (chunk: %u)", BSTD_FUNCTION, to_read, chunk)); */
            BKNI_Memcpy(buffer_ex, data, to_read);
            loop_size -= to_read;
            buffer_ex += to_read;
            chunk++;
        }
    }

    /* Sync physical memory for all areas */
    NEXUS_Memory_FlushCache(holder->raw->buf, holder->raw->len);
    BDBG_MSG(("%s - '%s' Raw@%p,  size=%u", BSTD_FUNCTION,
              holder->name, (void *)holder->raw->buf, (unsigned)holder->raw->len));

err:
    /* in case of error, Memory block is freed in NEXUS_SageModule_Uninit() */
    if (image) {
        img_interface->close(image);
    }

    return rc;
}

/* SAGE boot timing */
#if (BCHP_CHIP == 7278 && BCHP_VER < BCHP_VER_B0)
#define SAGE_MAX_BOOT_TIME_US (30 * 1000 * 1000)
#else
#define SAGE_MAX_BOOT_TIME_US (15 * 1000 * 1000)
#endif
#define SAGE_STEP_BOOT_TIME_US (50 * 1000)
/* Check is SAGE software is ready.
 * If not, waits for 15 seconds max to see if it becomes ready. */
/* prototype must be compatible with NEXUS_Thread_Create::pThreadFunc parameter */

/* SAGE ARC Prescreen error codes */
#define SAGE_SECUREBOOT_ARC_PRESCREEN_NUMBER_BAD_BIT_EXCEEDED  (0x60D)
#define SAGE_SECUREBOOT_ARC_PRESCREEN_MSP_PROG_FAILURE         (0x60E)

static NEXUS_Error
NEXUS_Sage_P_MonitorBoot(void)
{
    int totalBootTimeUs;
    int alreadyConsumedBootTimeUs;
    int overheadUs;
    NEXUS_Time now;
    uint32_t lastStatus=0x42;
    BSAGElib_BootState bootState = {BSAGElibBootStatus_eNotStarted, 0};

    /* This will calculate the time already consumed by the SAGE init process before we reach this point */
    NEXUS_Time_Get(&now);
    /* compute already comsumed time. */
    totalBootTimeUs = NEXUS_Time_Diff(&now, &g_NEXUS_sageModule.initTime) * 1000;

    alreadyConsumedBootTimeUs = totalBootTimeUs;
    /* Read Global SRAM registers for SAGE boot status every SAGE_STEP_BOOT_TIME_US us.
       Max to SAGE_MAX_BOOT_TIME_US us */

    do {
        BSAGElib_Boot_GetState(g_NEXUS_sageModule.hSAGElib, &bootState);
        if (bootState.status != lastStatus) {
            BDBG_MSG(("%s - last_status=%u,new_status=%u,total=%d us,overhead=%d us",
                      BSTD_FUNCTION, lastStatus,bootState.status,totalBootTimeUs,
                      totalBootTimeUs - alreadyConsumedBootTimeUs));
            lastStatus = bootState.status;
        }
        if(bootState.status == BSAGElibBootStatus_eStarted) {
            break;
        }
        if (bootState.status == BSAGElibBootStatus_eError) {
            if (bootState.lastError) {
                BDBG_ERR(("%s -  SAGE boot detected error", BSTD_FUNCTION));
                goto err;
            }
        }

        if (totalBootTimeUs > SAGE_MAX_BOOT_TIME_US) {
            BDBG_ERR(("%s - SAGE boot exceeds %d ms. Failure!!!!!!", BSTD_FUNCTION, SAGE_MAX_BOOT_TIME_US));
            goto err;
        }
        BKNI_Sleep(SAGE_STEP_BOOT_TIME_US/1000);
        totalBootTimeUs += SAGE_STEP_BOOT_TIME_US;
    } while (1);

    overheadUs = totalBootTimeUs - alreadyConsumedBootTimeUs;
    if (overheadUs > 0) {
        BDBG_MSG(("%s - SAGE boot took %d us, overhead is %d us",
                  BSTD_FUNCTION, totalBootTimeUs, overheadUs));
    }
    else {
        BDBG_MSG(("%s - SAGE booted, boot time is unknown", BSTD_FUNCTION));
    }

    /* Initialize bypass keyslot */
    {
        BERR_Code magnum_rc;

        magnum_rc = BSAGElib_Boot_Post(g_NEXUS_sageModule.hSAGElib);
        if (magnum_rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Boot_Post() fails %d.", BSTD_FUNCTION, magnum_rc));
            goto err;
        }
    }

    BDBG_MSG(("SAGE Boot done"));

err:
    if (bootState.status != BSAGElibBootStatus_eStarted) {
        char *_flavor = (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) ? "_dev" : "";
        BDBG_ERR(("*******  SAGE ERROR  >>>>>"));
        BDBG_ERR(("* The SAGE Software cannot boot."));
        if (bootState.status == BSAGElibBootStatus_eBlStarted) {
            BDBG_ERR(("* SAGE Framework hangs"));
            BDBG_ERR(("* Please check your sage_framework%s.bin", _flavor));
        }
        else if (bootState.status == BSAGElibBootStatus_eError) {
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
                    BDBG_ERR(("* Please check your sage_framework%s.bin", _flavor));
            }

        }
        else {
            BDBG_ERR(("* Reboot the system and try again."));
            BDBG_ERR(("* SAGE Bootloader status: 0x%08x", bootState.status));
            BDBG_ERR(("* SAGE Bootloader hangs"));
            BDBG_ERR(("* Please check your sage_bl%s.bin", _flavor));
        }
        BDBG_ERR(("*******  SAGE ERROR  <<<<<"));
        return NEXUS_UNKNOWN;
    }

    NEXUS_Sage_P_CleanBootVars();
    return NEXUS_SUCCESS;
}


int NEXUS_Sage_P_CheckSageBooted(void)
{
    BERR_Code rc;
    int ret = 0;

    if (g_NEXUS_sageModule.SWState == NEXUS_SageSWState_eUnknown) {
        BDBG_MSG(("BKNI_WaitForEvent"));
        rc = BKNI_WaitForEvent(g_NEXUS_sageModule.sageReadyEvent, BKNI_INFINITE);
        if(rc != BERR_SUCCESS)
        {
            rc = BERR_TRACE(rc);
        }
    }

    if (g_NEXUS_sageModule.SWState == NEXUS_SageSWState_eRunning) {
        ret = 1;
    }

    return ret;
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
        rc_magnum = BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
        if (rc_magnum != BERR_SUCCESS)
        {
            BDBG_ERR(("Nexus Dma Job Process Buf failed %s.",BSTD_FUNCTION));
            goto error;
        }
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
            rc_magnum = BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
            if (rc_magnum != BERR_SUCCESS)
            {
                BDBG_ERR(("Nexus Dma Job Process Buf failed %s.",BSTD_FUNCTION));
                goto error;
            }
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
