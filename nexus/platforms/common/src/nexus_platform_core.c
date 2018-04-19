/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "breg_mem.h"
#if NEXUS_CONFIG_IMAGE
#include "nexus_img_kernel.h"
#endif
#include "bchp_common.h"
#include "nexus_map.h"
#include "b_objdb.h"
#include "nexus_platform_shared_gpio.h"

#if defined(BCHP_MEMC_L2_0_REG_START)
#include "bchp_int_id_memc_l2_0.h"
#else
#if defined(BCHP_MEMC_L2_0_0_REG_START)
#include "bchp_int_id_memc_l2_0_0.h"
#endif
#if defined(BCHP_MEMC_L2_1_0_REG_START)
#include "bchp_int_id_memc_l2_1_0.h"
#endif
#if defined(BCHP_MEMC_L2_2_0_REG_START)
#include "bchp_int_id_memc_l2_2_0.h"
#endif
#endif /* BCHP_MEMC_L2_0_REG_START */

#if NEXUS_HAS_SECURITY
#include "priv/nexus_security_priv.h"
#endif
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

#include "priv/nexus_core_standby_priv.h"

#ifdef DIAGS_MEM_DMA_TEST
extern int run_dma_memory_test;
#endif

BDBG_MODULE(nexus_platform_core);

NEXUS_PlatformMemory g_platformMemory;
static NEXUS_Core_Settings g_coreSettings;
static bool g_mipsKernelMode;
struct NEXUS_Platform_P_MemcBspInterrupt {
    BINT_CallbackHandle wrch;
    BINT_CallbackHandle arch;
};

static struct NEXUS_Platform_P_MemcBspInterrupts {
    BKNI_EventHandle memcEvent;
    NEXUS_EventCallbackHandle memcEventHandler;
    struct NEXUS_Platform_P_MemcBspInterrupt memc[NEXUS_NUM_MEMC];
} g_NEXUS_Platform_P_MemcBspInterrupts;

/* map all heaps */
static NEXUS_Error NEXUS_Platform_P_MapRegion(unsigned index, NEXUS_Core_MemoryRegion *region);
static void NEXUS_Platform_P_UnmapRegion(NEXUS_Core_MemoryRegion *region);

#if BEXUS_P_TRACELOG_FOR_SECURE_ARCH
/* when enabled, this code will get be called after detected violation and will print print last captured transactions that are targeting the secure heaps */
#include "../dbg/memc_tracelog.h"
static tracelog_CalibrationData tracelog_calibration_data[NEXUS_NUM_MEMC];
static void NEXUS_Platform_P_TracelogForSecureArch_Setup(unsigned memc)
{
    BREG_Handle reg = g_pCoreHandles->reg;
    unsigned filter=0;
    unsigned j;

    if(!tracelog_Supported()) {
        if(memc==0) {
            BDBG_WRN(("TracelogForSecureArch: TRACELOG is not suppored"));
        }
        return;
    }

    tracelog_Reset(reg, memc);
    for (j=0;j<2;j++) {
        unsigned i;
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if(g_pCoreHandles->heap[i].nexus) {
                 NEXUS_MemoryStatus memStatus;
                 tracelog_MemoryFilter mem;
                 NEXUS_Error rc;

                 rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[i].nexus, &memStatus);
                 if(rc!=NEXUS_SUCCESS) {
                     continue;
                 }
                 if(memStatus.memoryType & NEXUS_MEMORY_TYPE_SECURE && memStatus.memcIndex==memc) {
                    /* use first filter for SRR */
                    if((memStatus.heapType & NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION) == NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION) {
                        if(j!=0) {
                            continue;
                        }
                    } else {
                        if(j==0) {
                            continue;
                        }
                    }
                    tracelog_GetDefaultMemoryFilter(&mem);
                    mem.match.address.enabled = true;
                    mem.match.address.low = memStatus.offset;
                    mem.match.address.high = memStatus.offset + memStatus.size;
                    BDBG_LOG(("TracelogForSecureArch MEMC:%u heap:%u filter:%u " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", memc, i, filter, BDBG_UINT64_ARG(mem.match.address.low), BDBG_UINT64_ARG(mem.match.address.high)));
                    tracelog_EnableMemoryFilter(reg, memc, filter, &mem, tracelog_FilterMode_eCapture);
                    filter++;
                 }
            }
        }
    }

    tracelog_Start(reg, memc, NULL);
    tracelog_Calibrate(reg, memc, &tracelog_calibration_data[memc]);
    return;
}

static void NEXUS_Platform_P_TracelogForSecureArch_Report(unsigned memc)
{
    BREG_Handle reg = g_pCoreHandles->reg;
    tracelog_Status status;

    if(!tracelog_Supported()) {
        return;
    }
    tracelog_Stop(reg, memc);
    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);
    tracelog_PrintLog(reg, memc, NULL, &tracelog_calibration_data[memc]);
    tracelog_Start(reg, memc, NULL);
    return;
}
#else
#define NEXUS_Platform_P_TracelogForSecureArch_Report(memc)
#define NEXUS_Platform_P_TracelogForSecureArch_Setup(memc)
#endif

static void NEXUS_Platform_P_MemcBsp_isr(void *pParam, int iParam)
{
    struct NEXUS_Platform_P_MemcBspInterrupts *memcs = pParam;
    unsigned i;

    BSTD_UNUSED(iParam);
    for(i=0;i<NEXUS_NUM_MEMC;i++) {
         NEXUS_Platform_P_TracelogForSecureArch_Report(i);
    }
    BDBG_MSG(("MEMC BSP ISR [%d]", iParam));
    BKNI_SetEvent(memcs->memcEvent);
    return;
}

static void NEXUS_Platform_P_MemcEventHandler(void * context)
{
    BSTD_UNUSED(context);
#if NEXUS_HAS_SECURITY
    if(g_NEXUS_platformHandles.security /* if NEXUS_Platform_P_MemcEventHandler called when there is no secure module */
#if NEXUS_POWER_MANAGEMENT
            && g_standbyState.settings.mode == NEXUS_StandbyMode_eOn
#endif
      ) {
        NEXUS_Module_Lock(g_NEXUS_platformHandles.security);
        NEXUS_Security_PrintArchViolation_priv();
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.security);
    }
#endif
    if (NEXUS_P_Core_SecureArchIssue_isrsafe()) {
        BDBG_LOG(("Detected SECURE MEMC ARCH violation. Terminating...."));
        BKNI_Fail();
    }
    return;
}

static NEXUS_Error NEXUS_Platform_P_MemcBspInterrupt_InitOne(struct NEXUS_Platform_P_MemcBspInterrupts *memcs, struct NEXUS_Platform_P_MemcBspInterrupt *memc, BINT_Id wrchIrq, BINT_Id archIrq, unsigned memcNo)
{
    NEXUS_Error rc;

    rc = BINT_CreateCallback(&memc->wrch, g_pCoreHandles->bint, wrchIrq, NEXUS_Platform_P_MemcBsp_isr, memcs, memcNo);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    rc = BINT_CreateCallback(&memc->arch, g_pCoreHandles->bint, archIrq, NEXUS_Platform_P_MemcBsp_isr, memcs, memcNo);
    if(rc!=BERR_SUCCESS) {
        BINT_DestroyCallback(memc->wrch);
        memc->wrch=NULL;
        return BERR_TRACE(rc);
    }
    BINT_EnableCallback(memc->wrch);
    BINT_EnableCallback(memc->arch);
    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_MemcBspInterrupt_UninitOne(struct NEXUS_Platform_P_MemcBspInterrupt *memc)
{
    if(memc->wrch) {
        BINT_DestroyCallback(memc->wrch);
        memc->wrch=NULL;
    }
    if(memc->arch) {
        BINT_DestroyCallback(memc->arch);
        memc->arch=NULL;
    }
    return;
}

static NEXUS_Error NEXUS_Platform_P_MemcBspInterrupt_Init(struct NEXUS_Platform_P_MemcBspInterrupts *memc)
{
    NEXUS_Error rc;
    unsigned i;
    rc = BKNI_CreateEvent(&memc->memcEvent);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_event; }

    memc->memcEventHandler =  NEXUS_RegisterEvent(memc->memcEvent, NEXUS_Platform_P_MemcEventHandler, memc);
    if(memc->memcEventHandler==NULL) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_event_hander;}

    for(i=0;i<NEXUS_NUM_MEMC;i++) {
         unsigned j;
         BINT_Id wrchIrq=0;
         BINT_Id archIrq=0;
         NEXUS_Platform_P_TracelogForSecureArch_Setup(i);
         switch(i) {
#if defined(BCHP_MEMC_L2_0_0_REG_START)
         case 0:
#if defined(BCHP_INT_ID_MEMC_L2_0_0_BSP_WRCH_INTR)
            wrchIrq = BCHP_INT_ID_MEMC_L2_0_0_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_MEMC_L2_0_0_BSP_ARCH_INTR;
#elif defined(BCHP_INT_ID_BSP_WRCH_INTR)
            wrchIrq = BCHP_INT_ID_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_BSP_ARCH_INTR;
#else
#error "Not supported"
#endif
            break;
#endif
#if defined(BCHP_MEMC_L2_1_0_REG_START)
         case 1:
             /* coverity[dead_error_line: FALSE] */
             /* coverity[-unreachable] for some platform this line is unreachable */
#if defined(BCHP_INT_ID_MEMC_L2_1_0_BSP_WRCH_INTR)
            wrchIrq = BCHP_INT_ID_MEMC_L2_1_0_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_MEMC_L2_1_0_BSP_ARCH_INTR;
#endif
            break;
#endif
#if defined(BCHP_MEMC_L2_2_0_REG_START)
         case 2:
            wrchIrq = BCHP_INT_ID_MEMC_L2_2_0_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_MEMC_L2_2_0_BSP_ARCH_INTR;
            break;
#endif
         /* coverity[dead_error_begin: FALSE] */
         default:
             /* coverity[dead_error_line: FALSE] */
             /* coverity[-unreachable] for some platform this line is unreachable */
            break;
         }
         if(wrchIrq==0 || archIrq==0) {
             /* coverity[dead_error_line: FALSE] */
             /* coverity[-unreachable] for some platform this line is unreachable */
             break;
         }
         rc = NEXUS_Platform_P_MemcBspInterrupt_InitOne(memc, &memc->memc[i], wrchIrq, archIrq, i);
         if(rc!=BERR_SUCCESS) {
             rc = BERR_TRACE(rc);
             for(j=0;j<i;j++) {
                /* coverity[dead_error_line: FALSE] */
                NEXUS_Platform_P_MemcBspInterrupt_UninitOne(&memc->memc[j]);
             }
             goto err_interrupt;
         }
    }
    return NEXUS_SUCCESS;

err_interrupt:
    NEXUS_UnregisterEvent(memc->memcEventHandler);
err_event_hander:
    BKNI_DestroyEvent(memc->memcEvent);
err_event:
    return BERR_TRACE(rc);
}

static void NEXUS_Platform_P_MemcBspInterrupt_Uninit(struct NEXUS_Platform_P_MemcBspInterrupts *memc)
{
    unsigned i;

    for(i=0;i<NEXUS_NUM_MEMC;i++) {
        NEXUS_Platform_P_MemcBspInterrupt_UninitOne(&memc->memc[i]);
    }
    NEXUS_UnregisterEvent(memc->memcEventHandler);
    BKNI_DestroyEvent(memc->memcEvent);
    return;
}


#define NEXUS_P_MATCH_REG(reg) case reg: return true
#define NEXUS_P_MATCH_REG_GROUP(reg,block) do { if (reg>=block##_REG_START && reg<=block##_REG_END) {return true;} } while(0)
#if NEXUS_HAS_GPIO
static bool NEXUS_Platform_P_IsGio_isrsafe(uint32_t reg)
{
#if defined(BCHP_GIO_REG_START)
    NEXUS_P_MATCH_REG_GROUP(reg,BCHP_GIO);
#endif
#if defined(BCHP_GIO_AON_REG_START)
    NEXUS_P_MATCH_REG_GROUP(reg,BCHP_GIO_AON);
#endif
    return false;
}
#endif
static bool NEXUS_Platform_P_IsSystemSharedRegister_isrsafe(uint32_t reg)
{
    /* this call must occur after shared gpio submodule is initialized */
#if NEXUS_HAS_GPIO
    return NEXUS_Platform_P_SharedGpioSupported() && NEXUS_Platform_P_IsGio_isrsafe(reg);
#else
    BSTD_UNUSED(reg);
    return false;
#endif
}

static bool NEXUS_Platform_P_IsRegisterAtomic_isrsafe(void *context, uint32_t reg)
{
    const NEXUS_Core_PreInitState *preInitState = context;
    if(NEXUS_Platform_P_IsSystemSharedRegister_isrsafe(reg)) {
        return true;
    }
    return preInitState->privateState.regSettings.isRegisterAtomic_isrsafe(preInitState->hReg, reg);
}

static void NEXUS_Platform_P_SystemUpdate32_isrsafe(void *context, uint32_t reg, uint32_t mask, uint32_t value, bool atomic)
{
    const NEXUS_Core_PreInitState *preInitState = context;
    bool systemRegister = NEXUS_Platform_P_IsSystemSharedRegister_isrsafe(reg);

    if(atomic || systemRegister || preInitState->privateState.regSettings.isRegisterAtomic_isrsafe(preInitState->hReg, reg)) {
        NEXUS_Platform_P_Os_SystemUpdate32_isrsafe(preInitState, reg, mask, value, systemRegister);
    } else { /* use not synchronous version */
        preInitState->privateState.regSettings.systemUpdate32_isrsafe(preInitState->hReg, reg, mask, value, false);
    }
    return;
}

static NEXUS_Error NEXUS_Platform_P_MapRegisters(NEXUS_Core_PreInitState *preInitState)
{
    BERR_Code rc;
    BREG_OpenSettings openSettings;

    if (preInitState->privateState.pRegAddress) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

#define REGISTER_BASE   (BCHP_PHYSICAL_OFFSET + (BCHP_REGISTER_START & ~0xFFF))
#define REGISTER_SIZE   (BCHP_REGISTER_END - (BCHP_REGISTER_START & ~0xFFF))

    /* mmap registers in uncached address space */
    preInitState->privateState.pRegAddress = NEXUS_Platform_P_MapRegisterMemory(REGISTER_BASE, REGISTER_SIZE);
    if ( NULL == preInitState->privateState.pRegAddress) {
        return BERR_TRACE(BERR_OS_ERROR);
    }

    BREG_GetDefaultOpenSettings(&openSettings);
    preInitState->privateState.regSettings = openSettings;
    openSettings.callbackContext = preInitState;
    openSettings.isRegisterAtomic_isrsafe = NEXUS_Platform_P_IsRegisterAtomic_isrsafe;
    openSettings.systemUpdate32_isrsafe = NEXUS_Platform_P_SystemUpdate32_isrsafe;


    /* Open register interface with mapped address */
    rc = BREG_Open(&preInitState->hReg, preInitState->privateState.pRegAddress, BCHP_REGISTER_END, &openSettings);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_UnmapRegisters(NEXUS_Core_PreInitState *preInitState)
{
    BREG_Close(preInitState->hReg);
    preInitState->hReg = NULL;
    NEXUS_Platform_P_UnmapRegisterMemory(preInitState->privateState.pRegAddress, REGISTER_SIZE);
    preInitState->privateState.pRegAddress = NULL;
    return;
}

static NEXUS_Error NEXUS_Platform_P_GetPictureBufferForAdjacent(const NEXUS_PlatformSettings *pSettings, unsigned adjacentHeap, unsigned *picbufHeap)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (pSettings->heap[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS &&
            pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_SECURE &&
            pSettings->heap[i].memcIndex == pSettings->heap[adjacentHeap].memcIndex) {
            *picbufHeap = i;
            return NEXUS_SUCCESS;
        }
    }
    *picbufHeap = 0;
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

/* bmem algo doesn't see NEXUS_HEAP_TYPE_SECURE_GRAPHICS or NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT or NEXUS_HEAP_TYPE_DTU heaps */
static NEXUS_PlatformSettings *NEXUS_Platform_P_AllocSettingsForAdjacentHeaps(const NEXUS_PlatformSettings *pSettings)
{
    unsigned i;
    NEXUS_PlatformSettings *pBmemSettings = NULL;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pSettings->heap[i].size) continue;
        if (pSettings->heap[i].heapType & (NEXUS_HEAP_TYPE_SECURE_GRAPHICS|NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT|NEXUS_HEAP_TYPE_DTU)) {
            if (!pBmemSettings) {
                pBmemSettings = BKNI_Malloc(sizeof(*pBmemSettings));
                if (!pBmemSettings) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);return NULL;}
                *pBmemSettings = *pSettings;
            }
        }
        if ((pSettings->heap[i].heapType & (NEXUS_HEAP_TYPE_SECURE_GRAPHICS|NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)) && pBmemSettings) {
            int rc;
            unsigned picbufHeap;
            rc = NEXUS_Platform_P_GetPictureBufferForAdjacent(pSettings, i, &picbufHeap);
            if (rc) {
                BERR_TRACE(rc);
                BKNI_Free(pBmemSettings);
                return NULL;
            }
            if (pSettings->heap[i].heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS) {
                /* additional memory */
                pBmemSettings->heap[picbufHeap].size += pBmemSettings->heap[i].size;
            }
            pBmemSettings->heap[i].size = 0;
        }
        if (pSettings->heap[i].heapType & NEXUS_HEAP_TYPE_DTU && pBmemSettings) {
            pBmemSettings->heap[i].offset = 0;
            pBmemSettings->heap[i].size = 0;
        }
    }
    return pBmemSettings;
}
static void NEXUS_Platform_P_FreeSettingsForAdjacentHeaps(NEXUS_PlatformSettings *pSettings)
{
    BKNI_Free(pSettings);
}
static NEXUS_Error NEXUS_Platform_P_BuildAdjacentHeaps(const NEXUS_PlatformSettings *pSettings, NEXUS_Core_Settings *pCoreSettings)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pSettings->heap[i].size) continue;
        if (pSettings->heap[i].heapType & (NEXUS_HEAP_TYPE_SECURE_GRAPHICS|NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)) {
            unsigned picbufHeap;
            int rc;
            rc = NEXUS_Platform_P_GetPictureBufferForAdjacent(pSettings, i, &picbufHeap);
            if (rc) return BERR_TRACE(rc);
            pCoreSettings->heapRegion[i] = pCoreSettings->heapRegion[picbufHeap];
            pCoreSettings->heapRegion[i].length = pSettings->heap[i].size;
            if (pCoreSettings->heapRegion[picbufHeap].length < (unsigned)pSettings->heap[i].size) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            pCoreSettings->heapRegion[picbufHeap].length -= pSettings->heap[i].size;
            if (pSettings->heap[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT) {
                pCoreSettings->heapRegion[picbufHeap].offset += pSettings->heap[i].size;
            }
            else {
                pCoreSettings->heapRegion[i].offset += pSettings->heap[picbufHeap].size;
            }
            pCoreSettings->heapRegion[i].heapType = pSettings->heap[i].heapType;
            pCoreSettings->heapRegion[i].memoryType = pSettings->heap[i].memoryType;
        }

        if (pSettings->heap[i].heapType & NEXUS_HEAP_TYPE_DTU) {
            pCoreSettings->heapRegion[i].offset = pSettings->heap[i].offset;
            pCoreSettings->heapRegion[i].length = pSettings->heap[i].size;
            pCoreSettings->heapRegion[i].heapType = pSettings->heap[i].heapType;
            pCoreSettings->heapRegion[i].memoryType = pSettings->heap[i].memoryType;
            pCoreSettings->heapRegion[i].memcIndex = pSettings->heap[i].memcIndex;
        }
    }
    return NEXUS_SUCCESS;
}

#if NEXUS_USE_CMA
static NEXUS_Addr NEXUS_Platform_P_AllocDeviceMemory(unsigned memcIndex, unsigned numBytes, unsigned alignment)
{
    return NEXUS_Platform_P_AllocCma(&g_platformMemory, memcIndex, 0, numBytes, alignment);
}
static void NEXUS_Platform_P_FreeDeviceMemory(unsigned memcIndex, NEXUS_Addr addr, unsigned numBytes)
{
    NEXUS_Platform_P_FreeCma(&g_platformMemory, memcIndex, 0, addr, numBytes);
}
#endif

NEXUS_Error NEXUS_Platform_P_InitCore( const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformSettings *pSettings)
{
    NEXUS_Error errCode = 0;
    unsigned int i = 0;
    NEXUS_PlatformMemory *pMemory = &g_platformMemory;
    struct nexus_map_settings map_settings;
    BINT_Settings intSettings;
    const BINT_Settings *intr_cfg;
    NEXUS_PlatformSettings *pBmemSettings = NULL;

/* TODO: for nfe image, g_mipsKernelMode could be passed in as runtime param */
#if NEXUS_MODE_driver || NEXUS_BASE_OS_linuxkernel
    g_mipsKernelMode = true;
#else
    g_mipsKernelMode = false;
#endif

    /* determine heaps based on MEMC config and user config */
    NEXUS_CoreModule_GetDefaultSettings(&g_coreSettings);

    pBmemSettings = NEXUS_Platform_P_AllocSettingsForAdjacentHeaps(pSettings);

    errCode = NEXUS_Platform_P_SetCoreModuleSettings(pBmemSettings?pBmemSettings:pSettings, pMemory, &g_coreSettings);
    if ( errCode ) { errCode=BERR_TRACE(errCode); goto err_memc; }

    if (pBmemSettings) {
        NEXUS_Platform_P_FreeSettingsForAdjacentHeaps(pBmemSettings);
        pBmemSettings = NULL;
        errCode = NEXUS_Platform_P_BuildAdjacentHeaps(pSettings, &g_coreSettings);
        if (errCode) return BERR_TRACE(errCode);
    }

    nexus_p_get_default_map_settings(&map_settings);
    /* the fake address range is (2GB - 4K). subtracting 4K allows the code to avoid 0x0000_0000.
    for kernel mode, we can't have a valid fake address of 0x0000_0000. too much code depends on ptr == NULL meaning "no pointer".
    for user mode, we can't have base+size == 0x0000_0000. too much code will fail on the wrap around.
    only 1 byte is needed to avoid this situation, but using 4K avoid possible alignment bugs. */
    map_settings.offset = (unsigned long)g_NEXUS_P_CpuNotAccessibleRange.start + 4096;
    map_settings.size = g_NEXUS_P_CpuNotAccessibleRange.length - 2 * 4096;

#if B_REFSW_SYSTEM_MODE_CLIENT
    map_settings.offset = 0x00000000 + 4096;
    map_settings.size = 4096;
#endif
    map_settings.mmap = NEXUS_Platform_P_MapMemory;
    map_settings.munmap = NEXUS_Platform_P_UnmapMemory;
    errCode = nexus_p_init_map(&map_settings);
    if ( errCode ) {
        errCode=BERR_TRACE(errCode);
        goto err_memc;
    }

    /* map all heaps based on user settings */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if(pSettings->heap[i].guardBanding) {
            errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err_guardband;
        }
        g_coreSettings.heapRegion[i].guardBanding = pSettings->heap[i].guardBanding;
        errCode = NEXUS_Platform_P_MapRegion(i, &g_coreSettings.heapRegion[i]);
        if (errCode) {errCode = BERR_TRACE(errCode); goto err_map;}
        if( (pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) ) {
            errCode = NEXUS_Platform_P_AddDynamicRegion(g_coreSettings.heapRegion[i].offset, g_coreSettings.heapRegion[i].length);
            if(errCode!=NEXUS_SUCCESS) {errCode=BERR_TRACE(errCode);goto err_map;}
        }
    }

    NEXUS_ASSERT_STRUCTURE(NEXUS_PlatformMemoryLayout, BCHP_MemoryLayout);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformMemoryLayout, memc , BCHP_MemoryLayout, memc);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformMemoryLayout, memc[0].size , BCHP_MemoryLayout, memc[0].size);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformMemoryLayout, memc[0].region, BCHP_MemoryLayout, memc[0].region);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformMemoryLayout, memc[0].region[0].addr, BCHP_MemoryLayout, memc[0].region[0].addr);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformMemoryLayout, memc[0].region[0].size, BCHP_MemoryLayout, memc[0].region[0].size);
    BKNI_Memcpy(&g_coreSettings.memoryLayout, &pMemory->memoryLayout, sizeof(g_coreSettings.memoryLayout));
    g_coreSettings.regHandle = preInitState->hReg;
    g_coreSettings.interruptInterface.pDisconnectInterrupt = NEXUS_Platform_P_DisconnectInterrupt;
    g_coreSettings.interruptInterface.pConnectInterrupt = NEXUS_Platform_P_ConnectInterrupt;
    g_coreSettings.interruptInterface.pEnableInterrupt_isr = NEXUS_Platform_P_EnableInterrupt_isr;
    g_coreSettings.interruptInterface.pDisableInterrupt_isr = NEXUS_Platform_P_DisableInterrupt_isr;
#if NEXUS_USE_CMA
    g_coreSettings.cma.alloc = NEXUS_Platform_P_AllocDeviceMemory;
    g_coreSettings.cma.free = NEXUS_Platform_P_FreeDeviceMemory;
#if NEXUS_HAS_SAGE
    g_coreSettings.cma.secure_remap = NEXUS_Sage_SecureRemap;
#endif
#endif
#if NEXUS_CONFIG_IMAGE
    g_coreSettings.imgInterface.create = Nexus_IMG_Driver_Create;
    g_coreSettings.imgInterface.destroy = Nexus_IMG_Driver_Destroy;
    g_coreSettings.imgInterface.open= Nexus_IMG_Driver_Open;
    g_coreSettings.imgInterface.close= Nexus_IMG_Driver_Close;
    g_coreSettings.imgInterface.next= Nexus_IMG_Driver_Next;
#endif
#if NEXUS_PLATFORM_DEFAULT_HEAP
    g_coreSettings.defaultHeapIndex = NEXUS_PLATFORM_DEFAULT_HEAP;
#endif
#if NEXUS_TEE_SUPPORT
    g_coreSettings.teeHandle = NEXUS_Platform_P_CreateTeeInstance();
    if ( NULL == g_coreSettings.teeHandle )
    {
        errCode = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_core;
    }
#endif
    g_coreSettings.os64 = NEXUS_Platform_P_IsOs64();
    g_coreSettings.pMapSettings = NEXUS_Platform_P_ReadPMapSettings();

    /* Initialize core module */
    intr_cfg = BINT_GETSETTINGS();
    BDBG_ASSERT(intr_cfg);
    intSettings = *intr_cfg;
    errCode = NEXUS_Platform_P_UpdateIntSettings(&intSettings);
    if(errCode!=NEXUS_SUCCESS) {errCode=BERR_TRACE(errCode);goto err_int;}


    g_NEXUS_platformHandles.core = NEXUS_CoreModule_Init(&g_coreSettings, preInitState, &intSettings);
    if ( !g_NEXUS_platformHandles.core ) {
        errCode=BERR_TRACE(NEXUS_UNKNOWN);
        goto err_core;
    }

    if (NEXUS_Platform_P_IsGisbTimeoutAvailable())
        NEXUS_Platform_P_ConfigureGisbTimeout();

    errCode = NEXUS_Platform_P_MemcBspInterrupt_Init(&g_NEXUS_Platform_P_MemcBspInterrupts);
    if(errCode!=NEXUS_SUCCESS) {errCode=BERR_TRACE(errCode);goto err_memc_intr;}

#ifdef DIAGS_MEM_DMA_TEST
    if (run_dma_memory_test)
        NO_OS_MemoryTest(g_pCoreHandles->chp,
            g_pCoreHandles->reg,
            g_pCoreHandles->bint,
            g_pCoreHandles->heap[0],
            g_pCoreHandles->heap[1],
            g_pCoreHandles->heap[2],
            g_coreSettings.memoryLayout[0].region[0].size,
            NULL,
            NULL);
#endif

    return BERR_SUCCESS;

/* Error cases */
err_memc_intr:
err_core:
    if (g_coreSettings.pMapSettings) {
        NEXUS_Platform_P_FreePMapSettings(g_coreSettings.pMapSettings);
    }
err_int:
err_map:
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_Platform_P_UnmapRegion(&g_coreSettings.heapRegion[i]);
    }
    nexus_p_uninit_map();
err_guardband:
err_memc:
    if (pBmemSettings) {
        NEXUS_Platform_P_FreeSettingsForAdjacentHeaps(pBmemSettings);
    }

    return errCode;
}

void NEXUS_Platform_P_UninitCore(void)
{
    unsigned i;
    NEXUS_Platform_P_MemcBspInterrupt_Uninit(&g_NEXUS_Platform_P_MemcBspInterrupts);

    NEXUS_CoreModule_Uninit();
    if (g_coreSettings.pMapSettings) {
        NEXUS_Platform_P_FreePMapSettings(g_coreSettings.pMapSettings);
    }
#if NEXUS_TEE_SUPPORT
    if ( g_coreSettings.teeHandle )
    {
        NEXUS_Platform_P_DestroyTeeInstance(g_coreSettings.teeHandle);
        g_coreSettings.teeHandle = NULL;
    }
#endif
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_Platform_P_UnmapRegion(&g_coreSettings.heapRegion[i]);
    }
    nexus_p_uninit_map();
}

static NEXUS_Error NEXUS_Platform_P_MapRegion(unsigned index, NEXUS_Core_MemoryRegion *region)
{
    NEXUS_AddrType uncachedMapType=NEXUS_AddrType_eFake, cachedMapType=NEXUS_AddrType_eFake;

    BSTD_UNUSED(index); /* index included for debug */
    if (!region->length) return 0;


    BDBG_MSG(("NEXUS_Platform_P_MapRegion heap[%d] " BDBG_UINT64_FMT ":%d, memoryType=%#x", index, BDBG_UINT64_ARG(region->offset), (unsigned)region->length, region->memoryType));
    /* this function runs in the driver. if in user mode, the driver's mapping is used for the application too. */
    if( (region->memoryType & NEXUS_MEMORY_TYPE_NOT_MAPPED) == NEXUS_MEMORY_TYPE_NOT_MAPPED ||
        (region->memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED
        ) {
        region->pvAddr = NULL;
        region->pvAddrCached = NULL;
        goto done;
    }

    if (!region->pvAddr) {
        bool cachedMapOnly;

        cachedMapOnly = true;
        if(NEXUS_GetEnv("with_uncached_mmap")) {
            cachedMapOnly = false;
        }

        if(cachedMapOnly) {
            uncachedMapType = NEXUS_AddrType_eFake;
        } else {
            uncachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_UNCACHED) ? NEXUS_AddrType_eUncached : NEXUS_AddrType_eFake;
        }
        BDBG_MSG(("using %s mapping for heap[%d] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(%u), memoryType=%x", uncachedMapType==NEXUS_AddrType_eFake?"cache-only":"cache and uncache", index, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), (unsigned)region->length, region->memoryType));
        region->pvAddr = nexus_p_map_memory(region->offset, region->length, uncachedMapType);
        if (!region->pvAddr) return BERR_TRACE(NEXUS_UNKNOWN);
    }
    else {
        /* this should not happen */
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    if (!region->pvAddrCached) {
        if (g_mipsKernelMode) {
        /* do driver mapping, don't do application mapping */
        cachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) ? NEXUS_AddrType_eCached : NEXUS_AddrType_eFake;
        }
        else {
        /* in user mode, do driver or application mapping */
        cachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED || region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) ? NEXUS_AddrType_eCached : NEXUS_AddrType_eFake;
        }
        if (cachedMapType == uncachedMapType && uncachedMapType == NEXUS_AddrType_eFake) {
            /* reuse the fake mapping so we don't exhaust fake addressing */
            region->pvAddrCached = region->pvAddr;
        }
        else {
            region->pvAddrCached = nexus_p_map_memory(region->offset, region->length, cachedMapType);
            if (!region->pvAddrCached) return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
    else {
        /* region->pvAddrCached may have already been mapped by app. see NEXUS_Platform_CreateHeap. */
    }
    region->cachedMapType = cachedMapType;
    region->uncachedMapType = uncachedMapType;

    /* on success, neither address can be zero */
    BDBG_ASSERT(region->pvAddr);
    BDBG_ASSERT(region->pvAddrCached);

done:
    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_UnmapRegion(NEXUS_Core_MemoryRegion *region)
{
    if (region->pvAddr) {
        nexus_p_unmap_memory(region->pvAddr, region->length, region->uncachedMapType);
    }
    if (region->pvAddrCached && region->pvAddrCached != region->pvAddr) {
        nexus_p_unmap_memory(region->pvAddrCached, region->length, region->cachedMapType);
    }
    region->pvAddr = NULL;
    region->pvAddrCached = NULL;
}

#if !NEXUS_USE_CMA
/* macros found in magnum/basemodules/chp */
#include "../../src/common/bchp_memc_offsets_priv.h"


NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformMemoryLayout *pMemory)
{
    unsigned memcIndex;

    for (memcIndex=0;memcIndex<NEXUS_NUM_MEMC;memcIndex++) {
        switch (memcIndex) {
        case 0: pMemory->memc[memcIndex].region[0].addr = BCHP_P_MEMC_0_OFFSET; break;
#ifdef BCHP_P_MEMC_1_OFFSET
        case 1: pMemory->memc[memcIndex].region[0].addr = BCHP_P_MEMC_1_OFFSET; break;
#endif
#ifdef BCHP_P_MEMC_2_OFFSET
        case 2: pMemory->memc[memcIndex].region[0].addr = BCHP_P_MEMC_2_OFFSET; break;
#endif
        /* coverity[dead_error_begin: FALSE] */
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        /* this size calculation is not valid for LPDDR4. only use for older silicon. */
        pMemory->memc[memcIndex].size = (uint64_t)preInitState->memoryInfo.memc[memcIndex].deviceTech / 8 * (preInitState->memoryInfo.memc[memcIndex].width/preInitState->memoryInfo.memc[memcIndex].deviceWidth) * 1024 * 1024;
        pMemory->memc[memcIndex].region[0].size = pMemory->memc[memcIndex].size;
        /* MIPS register hole */
        if (memcIndex == 0 && pMemory->memc[0].region[0].size > 0x10000000) {
            pMemory->memc[0].region[1].addr = 0x20000000;
            pMemory->memc[0].region[1].size = pMemory->memc[0].region[0].size - 0x10000000;
            pMemory->memc[0].region[0].size = 0x10000000;
        }
    }
    return NEXUS_SUCCESS;
}
#endif /* NEXUS_USE_CMA */

void NEXUS_Platform_GetDefaultCreateHeapSettings( NEXUS_PlatformCreateHeapSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->memoryType = NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
}

NEXUS_HeapHandle NEXUS_Platform_CreateHeap( const NEXUS_PlatformCreateHeapSettings *pSettings )
{
    NEXUS_HeapHandle heap;
    heap = NEXUS_Heap_CreateInternal(pSettings);
    if (heap) {
        nexus_platform_p_update_all_mmap_access();
    }
    return heap;
}

void NEXUS_Platform_DestroyHeap( NEXUS_HeapHandle heap )
{
    NEXUS_Heap_DestroyInternal(heap);
    nexus_platform_p_update_all_mmap_access();
}

static struct {
    NEXUS_Core_PreInitState state;
    unsigned refcnt;
} g_NEXUS_preinit;
const NEXUS_Core_PreInitState *g_pPreInitState;

const NEXUS_Core_PreInitState *NEXUS_Platform_P_PreInit(void)
{
    NEXUS_Core_PreInitState *preInitState = &g_NEXUS_preinit.state;
    int rc;
    BBOX_Settings boxSettings;

    if (g_NEXUS_preinit.refcnt) {
        g_NEXUS_preinit.refcnt++;
        return &g_NEXUS_preinit.state;
    }

    rc = NEXUS_Platform_P_Magnum_Init();
    if (rc) return NULL;
#if !BDBG_DEBUG_BUILD && !B_REFSW_DEBUG_COMPACT_ERR
    {
        static bool warn = false;
        if (!warn) {
            BKNI_Printf("*** Nexus was compiled with no error messages. For any failure, recompile with B_REFSW_DEBUG=y, =n or =minimal and retest.\n");
            warn = true;
        }
    }
#endif
    BKNI_Memset(preInitState, 0, sizeof(*preInitState));

    rc = NEXUS_Platform_P_InitOSMem();
    if (rc) {rc = BERR_TRACE(rc); goto err_osmem;}

    rc = NEXUS_Platform_P_MapRegisters(preInitState);
    if (rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto err_reg;}

    g_pPreInitState = &g_NEXUS_preinit.state;
    BKNI_Memset(&boxSettings, 0, sizeof(boxSettings)); /* TODO: need GetDefaultSettings */
    preInitState->boxMode = boxSettings.ulBoxId = NEXUS_Platform_P_ReadBoxMode();
    rc = BBOX_Open(&preInitState->hBox, &boxSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_box;}
    BBOX_GetConfig(preInitState->hBox, &preInitState->boxConfig);

    preInitState->pMapId = NEXUS_Platform_P_ReadPMapId();

    rc = BCHP_GetMemoryInfo_PreInit(preInitState->hReg, &preInitState->memoryInfo);
    if (rc) {rc = BERR_TRACE(rc); goto err_chpmeminfo;}

    g_NEXUS_preinit.refcnt++;
    return &g_NEXUS_preinit.state;

err_chpmeminfo:
err_box:
    NEXUS_Platform_P_UnmapRegisters(preInitState);
    g_pPreInitState = NULL;
err_reg:
    NEXUS_Platform_P_UninitOSMem();
err_osmem:
    BKNI_Memset(preInitState, 0, sizeof(*preInitState));
    NEXUS_Platform_P_Magnum_Uninit();
    return NULL;
}

void NEXUS_Platform_P_PreUninit(void)
{
    NEXUS_Core_PreInitState *preInitState = &g_NEXUS_preinit.state;

    if (!g_NEXUS_preinit.refcnt || --g_NEXUS_preinit.refcnt) return;

    g_pPreInitState = NULL;
    BBOX_Close(preInitState->hBox);
    NEXUS_Platform_P_UnmapRegisters(preInitState);
    NEXUS_Platform_P_UninitOSMem();
    BKNI_Memset(preInitState, 0, sizeof(*preInitState));
    NEXUS_Platform_P_Magnum_Uninit();
}

#if NEXUS_USE_CMA
NEXUS_Error NEXUS_Platform_GrowHeap(NEXUS_HeapHandle heap, size_t numBytes)
{
    NEXUS_MemoryStatus heapStatus;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned alignment = 4096;
    NEXUS_Addr offset;

    rc = NEXUS_Heap_GetStatus(heap, &heapStatus);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_get_status;}
    offset = NEXUS_Platform_P_AllocCma(&g_platformMemory, heapStatus.memcIndex, 0, numBytes, alignment);
    if(offset==0) { rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);goto err_cma;}
    BDBG_MSG(("GrowHeap:%p Adding region " BDBG_UINT64_FMT ":%u", (void*)heap, BDBG_UINT64_ARG(offset), (unsigned)numBytes));

    rc = NEXUS_Platform_P_AddDynamicRegion(offset, numBytes);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_add_dynamic_region;}

    NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
    rc = NEXUS_Heap_AddRegion_priv(heap, offset, numBytes);

    NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_add_region;}


    return NEXUS_SUCCESS;

err_add_region:
    NEXUS_Platform_P_RemoveDynamicRegion(offset, numBytes);
err_add_dynamic_region:
    NEXUS_Platform_P_FreeCma(&g_platformMemory, heapStatus.memcIndex, 0, offset, numBytes);

err_cma:
err_get_status:
    return rc;
}

void NEXUS_Platform_ShrinkHeap(NEXUS_HeapHandle heap, size_t continuousBytes, size_t lowThreshold )
{
    unsigned n;
    unsigned i;
    NEXUS_Error rc;
    NEXUS_MemoryStatus heapStatus;
    NEXUS_MemoryRegion *regions;
    bool continuousReserved;
    unsigned numFreeBlocks;

    rc = NEXUS_Heap_GetStatus(heap, &heapStatus);
    if(rc!=NEXUS_SUCCESS) {(void)BERR_TRACE(rc);return;}

    numFreeBlocks = heapStatus.numFreeBlocks;
    if(numFreeBlocks>256) { numFreeBlocks = 256; }

    regions = BKNI_Malloc(sizeof(*regions)*numFreeBlocks);
    if(regions==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);return;
    }

    NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
    NEXUS_Heap_GetFreeRegions_priv(heap, regions, numFreeBlocks, &n);
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
    for(continuousReserved=false,i=0;i<n;i++) {
        NEXUS_MemoryRegion region;
        unsigned alignment = 4096;
        region = regions[i];
        region.base = region.base + (alignment - 1);
        region.base -= region.base%alignment;
        BDBG_ASSERT(region.base >= regions[i].base);
        BDBG_ASSERT(region.base - regions[i].base <=  region.length);
        region.length -= region.base - regions[i].base;
        region.length -= region.length % alignment;
        BDBG_ASSERT(region.length <= regions[i].length);
        BDBG_MSG(("ShrinkHeap:%p Considering region %u/%u:" BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ") %u(%u) %s [%u/%u]", (void *)heap, i, n, BDBG_UINT64_ARG(region.base),  BDBG_UINT64_ARG(regions[i].base), (unsigned)region.length, (unsigned)regions[i].length, region.boundary?"B":"", (unsigned)continuousBytes, (unsigned)lowThreshold));
        if(!continuousReserved) {
            if(region.length >= continuousBytes) {
                region.length -= continuousBytes;
                region.base += continuousBytes;
                continuousReserved = true;
            } else if(i==0) { /* free blocks are sorted by size, so if largest block smaller then continuousBytes, then don't free it */
                continue;
            }
        }
        if(region.length == 0) {
            continue;
        }
        if(region.boundary || region.length >= lowThreshold) {
            BDBG_MSG(("ShrinkHeap:%p Removing region " BDBG_UINT64_FMT ":%u", (void*)heap, BDBG_UINT64_ARG(region.base), (unsigned)region.length));
            NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
            rc = NEXUS_Heap_RemoveRegion_priv(heap, region.base, region.length);
            NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
            if(rc!=NEXUS_SUCCESS) {
                continue;
            }
            NEXUS_Platform_P_FreeCma(&g_platformMemory, heapStatus.memcIndex, 0, region.base, region.length);
            NEXUS_Platform_P_RemoveDynamicRegion(region.base, region.length);
        }
    }
    if(!continuousReserved) {
        BDBG_MSG(("ShrinkHeap:%p not available %uKBytes of continuous free space", (void *)heap, (unsigned)continuousBytes/1024));
    }
    BKNI_Free(regions);
    return;
}
#else /* NEXUS_USE_CMA */
NEXUS_Error NEXUS_Platform_GrowHeap(NEXUS_HeapHandle heap, size_t numBytes)
{
    BSTD_UNUSED(heap);
    BSTD_UNUSED(numBytes);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_Platform_ShrinkHeap(NEXUS_HeapHandle heap, size_t continuousBytes, size_t lowThreshold )
{
    BSTD_UNUSED(heap);
    BSTD_UNUSED(continuousBytes);
    BSTD_UNUSED(lowThreshold);
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
}
#endif /* NEXUS_USE_CMA */

NEXUS_Error b_get_client_default_heaps(NEXUS_ClientConfiguration *config, struct b_objdb_client_default_heaps *default_heaps)
{
    unsigned i;
    bool hasDynamicCma = false;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus status;
        int rc;

        if (!config->heap[i]) continue;

        rc = NEXUS_Heap_GetStatus_priv(config->heap[i], &status);
        if (rc) continue;

        if (status.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) {
            hasDynamicCma = true;
        }
        if (!default_heaps->heap[NEXUS_DefaultHeapType_eAny]) {
            default_heaps->heap[NEXUS_DefaultHeapType_eAny] = config->heap[i];
            if (config->mode == NEXUS_ClientMode_eUntrusted) {
                default_heaps->heap[NEXUS_DefaultHeapType_eBounds] = config->heap[i];
            }
        }
        if (!default_heaps->heap[NEXUS_DefaultHeapType_eFull]) {
            if ((status.memoryType & (NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED)) == (NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED)) {
                default_heaps->heap[NEXUS_DefaultHeapType_eFull] = config->heap[i];
            }
        }
    }
    if (!default_heaps->heap[NEXUS_DefaultHeapType_eAny]) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (hasDynamicCma) {
        /* if the system has dynamic CMA, we can't have a bounds heap for M2MC */
        default_heaps->heap[NEXUS_DefaultHeapType_eBounds] = NULL;
    }
    return NEXUS_SUCCESS;
}
