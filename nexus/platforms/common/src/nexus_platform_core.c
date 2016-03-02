/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
#include "nexus_types.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#ifdef NEXUS_FPGA_SUPPORT
#include "nexus_platform_fpga.h"
#endif
#include "breg_mem.h"
#if NEXUS_CONFIG_IMAGE
#include "nexus_img_kernel.h"
#endif
#include "bchp_common.h"
#include "nexus_map.h"
#include "b_objdb.h"

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
static void nexus_platform_p_destroy_runtime_heaps(void);

static void NEXUS_Platform_P_MemcBsp_isr(void *pParam, int iParam)
{
    struct NEXUS_Platform_P_MemcBspInterrupts *memcs = pParam;
    BSTD_UNUSED(iParam);
    BDBG_MSG(("MEMC BSP ISR [%d]", iParam));
    BKNI_SetEvent(memcs->memcEvent);
    return;
}

static void NEXUS_Platform_P_MemcEventHandler(void * context)
{
    BSTD_UNUSED(context);
#if NEXUS_HAS_SECURITY
    if(g_NEXUS_platformHandles.security) { /* if NEXUS_Platform_P_MemcEventHandler called when there is no secure module */
        NEXUS_Module_Lock(g_NEXUS_platformHandles.security);
        NEXUS_Security_PrintArchViolation_priv();
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.security);
    }
#endif
    BDBG_LOG(("Detected SECURE MEMC ARCH violation. Terminating...."));
    BKNI_Fail();
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
#if defined(BCHP_INT_ID_MEMC_L2_0_0_BSP_WRCH_INTR)
            wrchIrq = BCHP_INT_ID_MEMC_L2_0_0_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_MEMC_L2_0_0_BSP_ARCH_INTR;
#endif
            break;
#endif
#if defined(BCHP_MEMC_L2_2_0_REG_START)
         case 2:
            wrchIrq = BCHP_INT_ID_MEMC_L2_2_0_BSP_WRCH_INTR;
            archIrq = BCHP_INT_ID_MEMC_L2_2_0_BSP_ARCH_INTR;
            break;
#endif
         default:
            break;
         }
         if(wrchIrq==0 || archIrq==0) {
             break;
         }
         rc = NEXUS_Platform_P_MemcBspInterrupt_InitOne(memc, &memc->memc[i], wrchIrq, archIrq, i);
         if(rc!=BERR_SUCCESS) {
             rc = BERR_TRACE(rc);
             for(j=0;j<i;j++) {
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
static bool NEXUS_Platform_P_IsGio(uint32_t reg)
{
#if defined(BCHP_GIO_REG_START)
    NEXUS_P_MATCH_REG_GROUP(reg,BCHP_GIO);
#endif
#if defined(BCHP_GIO_AON_REG_START)
    NEXUS_P_MATCH_REG_GROUP(reg,BCHP_GIO_AON);
#endif
    return false;
}

static bool NEXUS_Platform_P_IsSystemSharedRegister(uint32_t reg)
{
    return NEXUS_Platform_P_IsGio(reg);
}


static bool NEXUS_Platform_P_IsRegisterAtomic_isrsafe(void *context, uint32_t reg)
{
    const NEXUS_Core_PreInitState *preInitState = context;
    if(NEXUS_Platform_P_IsSystemSharedRegister(reg)) {
        return true;
    }
    return preInitState->privateState.regSettings.isRegisterAtomic_isrsafe(preInitState->hReg, reg);
}

static void NEXUS_Platform_P_SystemUpdate32_isrsafe(void *context, uint32_t reg, uint32_t mask, uint32_t value, bool atomic)
{
    const NEXUS_Core_PreInitState *preInitState = context;
    bool systemRegister = NEXUS_Platform_P_IsSystemSharedRegister(reg);

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

#define REGISTER_BASE   BCHP_PHYSICAL_OFFSET
#define REGISTER_SIZE   BCHP_REGISTER_END

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

NEXUS_Error NEXUS_Platform_P_InitCore( const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformSettings *pSettings)
{
    NEXUS_Error errCode = 0;
    unsigned int i = 0;
    NEXUS_PlatformMemory *pMemory = &g_platformMemory;
    struct nexus_map_settings map_settings;

/* TODO: for nfe image, g_mipsKernelMode could be passed in as runtime param */
#if NEXUS_MODE_driver || NEXUS_BASE_OS_linuxkernel
    g_mipsKernelMode = true;
#else
    g_mipsKernelMode = false;
#endif

    /* determine heaps based on MEMC config and user config */
    NEXUS_CoreModule_GetDefaultSettings(&g_coreSettings);
    if (!pMemory->max_dcache_line_size) {
        pMemory->max_dcache_line_size = 4096;
        BDBG_WRN(("max_dcache_line_size is 0. increasing to %d for cache coherency.", pMemory->max_dcache_line_size));
    }
    errCode = NEXUS_Platform_P_SetCoreModuleSettings(pSettings, pMemory, &g_coreSettings);
    if ( errCode ) { errCode=BERR_TRACE(errCode); goto err_memc; }

    nexus_p_get_default_map_settings(&map_settings);
    /* the fake address range is (2GB - 4K). subtracting 4K allows the code to avoid 0x0000_0000.
    for kernel mode, we can't have a valid fake address of 0x0000_0000. too much code depends on ptr == NULL meaning "no pointer".
    for user mode, we can't have base+size == 0x0000_0000. too much code will fail on the wrap around.
    only 1 byte is needed to avoid this situation, but using 4K avoid possible alignment bugs. */
    if (g_mipsKernelMode) {
        map_settings.offset = 0x00000000 + 4096;
        map_settings.size = NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE - 4096;
    }
    else {
        map_settings.offset = NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE;
        map_settings.size = (0- NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE) - 4096;
    }
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

    for (i = 0; i < NEXUS_MAX_MEMC; i++) {
        g_coreSettings.memcRegion[i].offset = pMemory->memc[i].region[0].base;
        g_coreSettings.memcRegion[i].length = (((uint64_t)1)<<40) - g_coreSettings.memcRegion[i].offset; /* fixed 40-bit range to cover all addressable memory */
    }
    g_coreSettings.regHandle = preInitState->hReg;
    g_coreSettings.interruptInterface.pDisconnectInterrupt = NEXUS_Platform_P_DisconnectInterrupt;
    g_coreSettings.interruptInterface.pConnectInterrupt = NEXUS_Platform_P_ConnectInterrupt;
    g_coreSettings.interruptInterface.pEnableInterrupt_isr = NEXUS_Platform_P_EnableInterrupt_isr;
    g_coreSettings.interruptInterface.pDisableInterrupt_isr = NEXUS_Platform_P_DisableInterrupt_isr;
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
    /* Initialize core module */
    g_NEXUS_platformHandles.core = NEXUS_CoreModule_Init(&g_coreSettings, preInitState);
    if ( !g_NEXUS_platformHandles.core ) {
        errCode=BERR_TRACE(NEXUS_UNKNOWN);
        goto err_core;
    }

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
            g_coreSettings.memcRegion[0].length,
            NULL,
            NULL);
#endif

    return BERR_SUCCESS;

/* Error cases */
err_memc_intr:
err_core:
err_map:
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_Platform_P_UnmapRegion(&g_coreSettings.heapRegion[i]);
    }
    nexus_p_uninit_map();
err_guardband:
err_memc:

    return errCode;
}

void NEXUS_Platform_P_UninitCore(void)
{
    unsigned i;
    NEXUS_Platform_P_MemcBspInterrupt_Uninit(&g_NEXUS_Platform_P_MemcBspInterrupts);

    nexus_platform_p_destroy_runtime_heaps();
    NEXUS_CoreModule_Uninit();
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
    NEXUS_MemoryMapType uncachedMapType=NEXUS_MemoryMapType_eFake, cachedMapType=NEXUS_MemoryMapType_eFake;

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

#if BCHP_CHIP==7125 ||\
    BCHP_CHIP==7325 || BCHP_CHIP==7336 || BCHP_CHIP==7340 || BCHP_CHIP==7342 ||\
    BCHP_CHIP==7400 || BCHP_CHIP==7405 || BCHP_CHIP==7408 || BCHP_CHIP==7420 || BCHP_CHIP==7468
        cachedMapOnly = false;
        if(NEXUS_GetEnv("without_uncached_mmap")) {
            cachedMapOnly = true;
        }
#else
        cachedMapOnly = true;
        if(NEXUS_GetEnv("with_uncached_mmap")) {
            cachedMapOnly = false;
        }
#endif
        if(cachedMapOnly) {
            uncachedMapType = NEXUS_MemoryMapType_eFake;
        } else {
            uncachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_UNCACHED) ? NEXUS_MemoryMapType_eUncached : NEXUS_MemoryMapType_eFake;
        }
        BDBG_MSG(("using %s mapping for heap[%d] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(%u), memoryType=%x", uncachedMapType==NEXUS_MemoryMapType_eFake?"cache-only":"cache and uncache", index, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), (unsigned)region->length, region->memoryType));
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
        cachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) ? NEXUS_MemoryMapType_eCached : NEXUS_MemoryMapType_eFake;
        }
        else {
        /* in user mode, do driver or application mapping */
        cachedMapType = (region->memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED || region->memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) ? NEXUS_MemoryMapType_eCached : NEXUS_MemoryMapType_eFake;
        }
        if (cachedMapType == uncachedMapType && uncachedMapType == NEXUS_MemoryMapType_eFake) {
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
/* TODO: Support Nexus heaps in upper memory (40 bit access).
Until then, extend this BCHP_CHIP list for systems that are populated with upper memory.
By limiting memory to 32 bit access, we avoid overlaps when calculating MEMC layout. */
#if BCHP_CHIP == 7445
#define NEXUS_MAX_MEMC0_SIZE (1024*1024*1024)
#define NEXUS_MAX_MEMC1_SIZE (1024*1024*1024)
#define NEXUS_MAX_MEMC2_SIZE (1024*1024*1024)
#elif ((BCHP_CHIP == 7439) && (BCHP_VER >= BCHP_VER_B0))
/* Only define Memc1, Memc0 can address up to 2Gb in the lower memory (32 bit access) */
#define NEXUS_MAX_MEMC1_SIZE (1024*1024*1024)
#endif
#endif /* !NEXUS_USE_CMA */

NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformMemory *pMemory)
{
    BCHP_MemoryInfo info;
    unsigned memcIndex;
    int rc;

    rc = BCHP_GetMemoryInfo(preInitState->hReg, &info);
    if (rc) return BERR_TRACE(rc);

#if NEXUS_MAX_MEMC0_SIZE
    if (info.memc[0].size > NEXUS_MAX_MEMC0_SIZE) {
       info.memc[0].size = NEXUS_MAX_MEMC0_SIZE;
    }
#endif
#if NEXUS_MAX_MEMC1_SIZE
    if (info.memc[1].size > NEXUS_MAX_MEMC1_SIZE) {
        info.memc[1].size = NEXUS_MAX_MEMC1_SIZE;
    }
#endif
#if NEXUS_MAX_MEMC2_SIZE
    if (info.memc[2].size > NEXUS_MAX_MEMC2_SIZE) {
        info.memc[2].size = NEXUS_MAX_MEMC2_SIZE;
    }
#endif

#if BCHP_CHIP == 7325 || BCHP_CHIP == 7335 || BCHP_CHIP == 7336 || BCHP_CHIP == 7400 || BCHP_CHIP == 7403 || BCHP_CHIP == 7405
    /* BCHP_GetMemoryInfo only populates offsets for 65nm, so get sizes from pMemory */
    for (memcIndex=0;memcIndex<NEXUS_NUM_MEMC;memcIndex++) {
        info.memc[memcIndex].size = pMemory->memc[memcIndex].length;
    }
#endif


    for (memcIndex=0;memcIndex<NEXUS_NUM_MEMC;memcIndex++) {
#if NEXUS_USE_CMA
        unsigned i;
        bool cma = pMemory->osRegion[0].cma;
        uint64_t size = info.memc[memcIndex].size;
        for(i=0;i<NEXUS_MAX_HEAPS;i++) {
            if(!cma && pMemory->osRegion[i].cma) {
                continue;
            }
            if(pMemory->osRegion[i].length==0 || pMemory->osRegion[i].memcIndex!=memcIndex) {
                continue;
            }
            if(pMemory->osRegion[i].subIndex >= NEXUS_NUM_MEMC_REGIONS) {
                BDBG_ERR(("MEMC%u/%u (%u MBytes) is not supported", memcIndex, pMemory->osRegion[i].subIndex, (unsigned)(pMemory->osRegion[i].length/(1024*1024))));
                continue;
            }
            if(pMemory->osRegion[i].length > size) {
                BDBG_ERR(("MEMC%u/%u (%u MBytes) is out of range", memcIndex, pMemory->osRegion[i].subIndex, (unsigned)(pMemory->osRegion[i].length/(1024*1024))));
                continue;
            }
            if(pMemory->memc[memcIndex].region[pMemory->osRegion[i].subIndex].length) {
                BDBG_ERR(("MEMC%u/%u (%u MBytes) duplicated", memcIndex, pMemory->osRegion[i].subIndex, (unsigned)(pMemory->osRegion[i].length/(1024*1024))));
                continue;
            }
            size -= pMemory->osRegion[i].length;
            pMemory->memc[memcIndex].region[pMemory->osRegion[i].subIndex].base = pMemory->osRegion[i].base;
            pMemory->memc[memcIndex].region[pMemory->osRegion[i].subIndex].length = pMemory->osRegion[i].length;
            if(pMemory->osRegion[i].subIndex==0) { /* make first region start at proper address */
                if(pMemory->osRegion[i].base >= info.memc[memcIndex].offset) {
                    pMemory->memc[memcIndex].region[pMemory->osRegion[i].subIndex].base = info.memc[memcIndex].offset;
                    pMemory->memc[memcIndex].region[pMemory->osRegion[i].subIndex].length += pMemory->osRegion[i].base - info.memc[memcIndex].offset;
                } else {
                    BDBG_ERR(("MEMC%u/%u unexpected offset " BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ")", memcIndex, pMemory->osRegion[i].subIndex, BDBG_UINT64_ARG(pMemory->osRegion[i].base), BDBG_UINT64_ARG(info.memc[memcIndex].offset)));
                }
            }
        }
        BDBG_MSG(("MEMC%u reserved for OS %u/%u MBytes", memcIndex, (unsigned)(size/(1024*1024)), (unsigned)(info.memc[memcIndex].size/(1024*1024))));
#else /* NEXUS_USE_CMA */
        pMemory->memc[memcIndex].region[0].base = info.memc[memcIndex].offset;
        pMemory->memc[memcIndex].region[0].length = info.memc[memcIndex].size;
#if NEXUS_CPU_ARM
        /* ARM CMA barrier, which corresponds to pMemory->osRegion[1].base. we may need a more sophisticated
        use of NEXUS_Platform_P_GetCmaPlatformMemory later. */
        if (memcIndex == 0 && pMemory->osRegion[1].base && pMemory->memc[0].region[0].length > pMemory->osRegion[1].base) {
            pMemory->memc[0].region[1].base = pMemory->osRegion[1].base;
            pMemory->memc[0].region[1].length = pMemory->memc[0].region[0].length - pMemory->osRegion[1].base;
            pMemory->memc[0].region[0].length = pMemory->osRegion[1].base;
        }
#else
        /* MIPS register hole */
        if (memcIndex == 0 && pMemory->memc[0].region[0].length > 0x10000000) {
            pMemory->memc[0].region[1].base = 0x20000000;
            pMemory->memc[0].region[1].length = pMemory->memc[0].region[0].length - 0x10000000;
            pMemory->memc[0].region[0].length = 0x10000000;
        }
#endif
#endif /* NEXUS_USE_CMA */
    }
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_GetDefaultCreateHeapSettings( NEXUS_PlatformCreateHeapSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/* g_runtimeHeaps[].heap mirrors g_pCoreHandles->nexusHeap[], but only for runtime-created heaps */
static struct {
    NEXUS_HeapHandle heap;
    NEXUS_Core_MemoryRegion region;
    NEXUS_PlatformCreateHeapSettings settings;
} *g_runtimeHeaps;

NEXUS_HeapHandle NEXUS_Platform_CreateHeap( const NEXUS_PlatformCreateHeapSettings *pSettings )
{
    NEXUS_HeapHandle heap;
    unsigned i;
    unsigned unused = NEXUS_MAX_HEAPS;
    NEXUS_Core_MemoryRegion *region;
    NEXUS_Error rc;

    if (!g_runtimeHeaps) {
        g_runtimeHeaps = BKNI_Malloc(sizeof(*g_runtimeHeaps)*NEXUS_MAX_HEAPS);
        if (!g_runtimeHeaps) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(g_runtimeHeaps, 0, sizeof(*g_runtimeHeaps)*NEXUS_MAX_HEAPS);
    }

    /* param validation */
    if (pSettings->offset + pSettings->size <= pSettings->offset) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    /* TODO: This function only works for MEMC0 now. To extend for MEMC1/2 support, we should consider
    refactoring NEXUS_Platform_P_CalcSubMemc to get generic physical base address per MEMC information.
    For now, use a simple, universal bound for MEMC0. */
    if (pSettings->offset >= 0x60000000) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    /* verify that new heap has no offset or userAddress overlap with existing heaps */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        heap = g_pCoreHandles->heap[i].nexus;
        if (heap) {
            NEXUS_MemoryStatus status;
            rc = NEXUS_Heap_GetStatus(heap, &status);
            if (rc) {
                rc = BERR_TRACE(rc);
                return NULL;
            }
			status.addr = NEXUS_OffsetToCachedAddr(status.offset);
            if (pSettings->offset >= status.offset && pSettings->offset + pSettings->size <= status.offset + status.size) {
                BDBG_ERR(("NEXUS_Platform_CreateHeap: runtime heap's offset cannot overlap existing heap"));
                return NULL;
            }
            if (pSettings->userAddress >= status.addr && (uint8_t*)pSettings->userAddress + pSettings->size <= (uint8_t*)status.addr + status.size) {
                BDBG_ERR(("NEXUS_Platform_CreateHeap: runtime heap's address cannot overlap existing heap"));
                return NULL;
            }
        }
        else if (unused == NEXUS_MAX_HEAPS) {
            unused = i;
        }
    }
    if (unused == NEXUS_MAX_HEAPS) {
        BDBG_ERR(("must increase NEXUS_MAX_HEAPS"));
        return NULL;
    }
    i = unused;
    BDBG_ASSERT(!g_runtimeHeaps[i].heap);
    g_runtimeHeaps[i].settings = *pSettings;

    /* we can create the heap now */
    region = &g_runtimeHeaps[i].region;
    BKNI_Memset(region, 0, sizeof(*region));
    region->memcIndex = 0; /* see above for restriction */
    region->offset = pSettings->offset;
    region->length = pSettings->size;
    region->memoryType = pSettings->memoryType;
    region->alignment = pSettings->alignment;
    region->pvAddrCached = pSettings->userAddress;
    region->locked = pSettings->locked;

    rc = NEXUS_Platform_P_MapRegion(i, region);
    if (rc) {
        rc = BERR_TRACE(rc);
        return NULL;
    }

    NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
    heap = NEXUS_Heap_Create_priv(i, region);
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
    if (!heap) {
        NEXUS_Platform_P_UnmapRegion(region);
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    g_runtimeHeaps[i].heap = heap;

    return heap;
}

void NEXUS_Platform_DestroyHeap( NEXUS_HeapHandle heap )
{
    unsigned i;
    /* only allow heaps created with NEXUS_Platform_CreateHeap to be destroyed here */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_runtimeHeaps[i].heap == heap) {
            BDBG_ASSERT(heap == g_pCoreHandles->heap[i].nexus);
            NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
            NEXUS_Heap_Destroy_priv(heap);
            NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
            if (g_runtimeHeaps[i].settings.userAddress) {
                /* don't unmap what the user mapped */
                g_runtimeHeaps[i].region.pvAddrCached = NULL;
            }
            NEXUS_Platform_P_UnmapRegion(&g_runtimeHeaps[i].region);
            g_runtimeHeaps[i].heap = NULL;
            return;
        }
    }
    BDBG_ERR(("NEXUS_Platform_DestroyHeap: %p was not created with NEXUS_Platform_CreateHeap", (void *)heap));
}

static void nexus_platform_p_destroy_runtime_heaps(void)
{
    if (g_runtimeHeaps) {
        unsigned i;
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if (g_runtimeHeaps[i].heap) {
                NEXUS_Platform_DestroyHeap(g_runtimeHeaps[i].heap);
            }
        }
        BKNI_Free(g_runtimeHeaps);
        g_runtimeHeaps = NULL;
    }
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

    g_NEXUS_preinit.refcnt++;
    return &g_NEXUS_preinit.state;

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
    bool recycled = false;
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
            recycled = true;
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
