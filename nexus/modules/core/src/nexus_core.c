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
 ******************************************************************************/
#include "nexus_core_module.h"
#include "priv/nexus_implicit_objects.h"
#if NEXUS_POWER_MANAGEMENT
#include "bchp_pwr.h"
#endif
#include "bint_stats.h"
#include "nexus_base_statistics.h"
#include "bkni_metrics.h"
#include "priv/nexus_core_preinit.h"
#if NEXUS_HAS_SAGE
#include "bchp_cmp_0.h"
#endif

BDBG_MODULE(nexus_core);
BTRC_MODULE(500ms_tick, ENABLE);

NEXUS_Core_P_State g_NexusCore;

const NEXUS_Core_MagnumHandles *g_pCoreHandles = NULL;

void
NEXUS_CoreModule_GetDefaultSettings(NEXUS_Core_Settings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

#if B_HAS_TRC
/* reduce period to make more frequent snapshots */
#define NEXUS_CORE_TIMER_FREQ 500
#else
#define NEXUS_CORE_TIMER_FREQ 5000
#endif


static void
NEXUS_Core_P_PrintStatistics(void)
{
    /* you can see these stats with msg_modules=int */
    BINT_DumpInfo(g_NexusCore.publicHandles.bint);
    /* you can see these stats with msg_modules=nexus_statistics\* */
    NEXUS_P_Base_Stats_Report();
#if defined(BKNI_P_Stats_Print)
    /* you can see these stats with msg_modules=bkni_statistics\* */
    BKNI_P_Stats_Print();
#endif

    if (NEXUS_GetEnv("debug_mem")) {
        unsigned i;
        BDBG_WRN(("calling BMEM_Heap_Validate for all heaps"));
        for(i=0;i<NEXUS_MAX_HEAPS;i++) {
#if !BMMA_USE_STUB
            if (g_pCoreHandles->heap[i].mma) {
                BMMA_Dbg_ValidateHeap(g_pCoreHandles->heap[i].mma);
            }
#else
            if (g_pCoreHandles->heap[i].mem) {
                BMEM_Heap_Validate(g_pCoreHandles->heap[i].mem);
            }
#endif
        }
    }
    return;
}
static void
NEXUS_Core_P_Timer(void *cntx)
{
    unsigned i;

    BSTD_UNUSED(cntx);

    BTRC_TRACE(500ms_tick, STOP); BTRC_TRACE(500ms_tick, START); /* snapshot counters to prevent underflow */

    NEXUS_Core_P_PrintStatistics();
    for(i=0;i<sizeof(g_NexusCore.publicHandles.memc)/sizeof(g_NexusCore.publicHandles.memc[0]);i++) {
        BMRC_Monitor_Handle mrc = g_NexusCore.publicHandles.memc[i].rmm;
        if(mrc) {
            BMRC_Monitor_RestoreInterrupts(mrc);
        }
    }

    g_NexusCore.timer = NEXUS_ScheduleTimer(NEXUS_CORE_TIMER_FREQ, NEXUS_Core_P_Timer, NULL);
    if(!g_NexusCore.timer) {
        BDBG_WRN(("NEXUS_Core_P_Timer: can't schedule timer"));
    }
    return;
}

static NEXUS_AvsSettings g_avsSettings;

void NEXUS_GetAvsSettings( NEXUS_AvsSettings *pSettings )
{
    *pSettings = g_avsSettings;
}

NEXUS_Error NEXUS_SetAvsSettings( const NEXUS_AvsSettings *pSettings )
{
    BDBG_ASSERT(pSettings);

    if (pSettings->hardStopOffset > 15)
        return NEXUS_INVALID_PARAMETER;
    if (pSettings->maxVoltageStopOffset > 15)
        return NEXUS_INVALID_PARAMETER;

    /* Note: if this hardware does not support these settings they will not be used */
    g_avsSettings = *pSettings;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_GetAvsDomainStatus(
    NEXUS_AvsDomain domain,  /* [in] index of domain to fetch status */
    NEXUS_AvsStatus *pStatus /* [out] the current domain-specific status */
)
{
    BCHP_AvsData data;
    BERR_Code rc;

    /* Note: if the AVS hardware is not supported this call will return an error */
    rc = BCHP_GetAvsData_isrsafe(g_NexusCore.publicHandles.chp, &data);
    if(rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    pStatus->enabled     = data.enabled;
    pStatus->tracking    = data.tracking;
    switch (domain) {
    case NEXUS_AvsDomain_eMain :
        pStatus->voltage      = data.voltage;
        pStatus->temperature  = data.temperature;
        break;

    case NEXUS_AvsDomain_eCpu :
        pStatus->voltage      = data.voltage1;
        pStatus->temperature  = data.temperature1;
        break;

    default :
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return rc;
}

#if NEXUS_AVS_MONITOR
static void
NEXUS_Core_P_MonitorPvt(void *context)
{
    BCHP_AvsSettings avsSettings;

    /* Note: if this hardware does not support these settings they will not be used */
    avsSettings.hardStopOffset = g_avsSettings.hardStopOffset;
    avsSettings.maxVoltageStopOffset = g_avsSettings.maxVoltageStopOffset;

    BSTD_UNUSED(context);

    BCHP_MonitorPvt(g_NexusCore.publicHandles.chp, &avsSettings);

    g_NexusCore.pvtTimer = NEXUS_ScheduleTimer(1000, NEXUS_Core_P_MonitorPvt, NULL);

    if(!g_NexusCore.pvtTimer) {
        BDBG_WRN(("NEXUS_Core_P_Timer: can't schedule PVT timer"));
    }
    return;
}
#endif /*NEXUS_AVS_MONITOR*/

static void NEXUS_CoreModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    BDBG_LOG(("Core:"));
    NEXUS_Memory_PrintHeaps();
    NEXUS_MemoryBlock_P_Print();
    return;
#endif
}

#if BCHP_UNIFIED_IMPL
static unsigned NEXUS_hextoi(const char *s)
{
    unsigned val = 0;
    for (;*s;s++) {
        if (*s == 'x') {
            continue;
        }
        else if (*s >= 'a' && *s <= 'f') {
            val = val*16 + (*s - 'a');
        }
        else if (*s >= 'A' && *s <= 'F') {
            val = val*16 + (*s - 'A');
        }
        else if (*s >= '0' && *s <= '9') {
            val = val*16 + (*s - '0');
        }
        else {
            break;
        }
    }
    return val;
}
#endif

NEXUS_ModuleHandle
NEXUS_CoreModule_Init(const NEXUS_Core_Settings *pSettings, const NEXUS_Core_PreInitState *preInitState, const BINT_Settings *pIntSettings)
{
    BERR_Code rc;
    BINT_CustomSettings intCustomSettings;
    BCHP_FeatureData chpFeature;
    BTMR_DefaultSettings tmr_settings;
    unsigned i;
    NEXUS_ModuleSettings moduleSettings;
#if !BMEM_DEPRECATED
    BMEM_Settings mem_module_settings;
#endif
    BMMA_PoolAllocator_CreateSettings poolSettings;
#if !BMMA_USE_STUB
    BMMA_CreateSettings mmaSettings;
#endif
    bool skipInitialReset = false;

#if NEXUS_NUM_MEMC
    /* verify API macros aren't below nexus_platform_features.h. NEXUS_NUM_MEMC is no longer used inside nexus. */
    BDBG_CASSERT(NEXUS_MAX_MEMC >= NEXUS_NUM_MEMC);
#endif

    BDBG_CASSERT(NEXUS_AudioCodec_eMax==NEXUS_MAX_AUDIOCODECS);

    if (!pSettings) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    BKNI_Memset(&g_NexusCore, 0, sizeof(g_NexusCore));
    BLST_D_INIT(&g_NexusCore.allocatedBlocks);

    if(!pSettings->interruptInterface.pDisconnectInterrupt || !pSettings->interruptInterface.pConnectInterrupt || !pSettings->interruptInterface.pEnableInterrupt_isr || !pSettings->interruptInterface.pDisableInterrupt_isr) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_params;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.dbgPrint = NEXUS_CoreModule_P_Print;
    moduleSettings.dbgModules = "nexus_core";
    moduleSettings.priority = NEXUS_ModulePriority_eAlwaysOn;
    g_NexusCore.module = NEXUS_Module_Create("core", &moduleSettings);
    if(!g_NexusCore.module) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_module;
    }
    NEXUS_LockModule();

    BMMA_PoolAllocator_GetDefaultCreateSettings(&poolSettings);
    poolSettings.allocationSize = sizeof(struct NEXUS_MemoryBlock);
    rc = BMMA_PoolAllocator_Create(&g_NexusCore.memoryBlockPool, &poolSettings);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_heap_handle_pool;
    }

    rc = NEXUS_Memory_P_Init();
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_memory;
    }

    g_NexusCore.cfg = *pSettings;
    BDBG_ASSERT(NULL != pSettings->regHandle);
    g_NexusCore.publicHandles.reg = pSettings->regHandle;
    g_NexusCore.publicHandles.box = preInitState->hBox;
    g_NexusCore.publicHandles.tee = pSettings->teeHandle;
    g_NexusCore.publicHandles.memoryLayout = pSettings->memoryLayout;

#if NEXUS_HAS_SAGE
    /* If SAGE is enabled but not started, then this is a fresh boot. */
    if (!BCHP_SAGE_HasEverStarted(g_NexusCore.publicHandles.reg)) {
        /* If GFD0 is enabled, it is splash. */
        uint32_t val = BREG_Read32(g_NexusCore.publicHandles.reg, BCHP_CMP_0_G0_SURFACE_CTRL);
        skipInitialReset = BCHP_GET_FIELD_DATA(val,CMP_0_G0_SURFACE_CTRL,ENABLE);
        if (skipInitialReset) {
            BDBG_LOG(("skipping initial reset to extend splash across SAGE boot"));
        }
    }
#endif

#if BCHP_UNIFIED_IMPL
    {
    BCHP_OpenSettings openSettings;
    const char *str = NEXUS_GetEnv("B_REFSW_OVERRIDE_PRODUCT_ID_TO");
    BCHP_GetDefaultOpenSettings(&openSettings);
    openSettings.reg = g_NexusCore.publicHandles.reg;
    openSettings.memoryLayout = pSettings->memoryLayout;
    openSettings.pMapId = preInitState->pMapId;
    openSettings.pMapSettings = preInitState->pMapSettings;
    openSettings.skipInitialReset = skipInitialReset;
    if (str) {
        openSettings.productId = NEXUS_hextoi(str);
    }
    rc = BCHP_Open(&g_NexusCore.publicHandles.chp, &openSettings);
    }
#else
    rc = BCHP_OPEN(&g_NexusCore.publicHandles.chp, g_NexusCore.publicHandles.reg);
#endif
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_chp;
    }

    g_NexusCore.publicHandles.boxConfig = &preInitState->boxConfig;

    {
    const char *str = NEXUS_GetEnv("B_REFSW_DRAM_REFRESH_RATE");
    BBOX_LoadRtsSettings loadRtsSettings;
    BBOX_GetDefaultLoadRtsSettings(&loadRtsSettings);
    if (str) {
        loadRtsSettings.eRefreshRate = NEXUS_atoi(str);
    }
    rc = BBOX_LoadRts(g_NexusCore.publicHandles.box, g_NexusCore.publicHandles.reg, &loadRtsSettings);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_boxloadrts;
    }
    }

#if NEXUS_HAS_SAGE
    rc = BCHP_SAGE_Reset(g_NexusCore.publicHandles.reg);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_mem_cfg;
    }
#endif

#if !BMEM_DEPRECATED
    rc = BMEM_GetDefaultSettings(&mem_module_settings);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_mem_cfg;
    }
    mem_module_settings.flush = NEXUS_FlushCache;
    mem_module_settings.flush_isr = NEXUS_FlushCache_isr;
    rc = BMEM_Open(&g_NexusCore.publicHandles.mem, &mem_module_settings);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_mem;
    }
#endif /* !BMEM_DEPRECATED */
#if !BMMA_USE_STUB
    BMMA_GetDefaultCreateSettings(&mmaSettings);
    mmaSettings.flush_cache = NEXUS_FlushCache;

    rc = BMMA_Create(&g_NexusCore.publicHandles.mma, &mmaSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_mma; }
#endif /* BMMA_USE_STUB */

    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        /* platform may have runtime option for memory configuration. length == 0 means no heap. */
        if (pSettings->heapRegion[i].length == 0) {
            continue;
        }
        if (!NEXUS_Heap_Create_priv(i, g_NexusCore.publicHandles.reg, &pSettings->heapRegion[i])) {
            rc = BERR_TRACE(BERR_UNKNOWN);
            goto err_heap;
        }
        /* successful heap is stored in g_pCoreHandles->heap[i] */
    }


    g_NexusCore.publicHandles.bint_map = pIntSettings->pIntMap;
#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_AcquireResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif
    /* BINT_Open needs to write to L2 interrupts, and should ideally guarantee its own power by
       acquiring/releasing within BINT_Open. However, there's no BCHP handle available inside BINT_Open,
       and we can't make it available without breaking the BINT_Open prototype.
       Hence, upper-level software must guarantee power to BINT_Open. */
    BINT_GetDefaultCustomSettings(&intCustomSettings);
    BCHP_GetFeature(g_NexusCore.publicHandles.chp, BCHP_Feature_eDisabledL2Registers, &chpFeature);
    intCustomSettings.disabledL2Registers = chpFeature.data.disabledL2Registers;
    rc = BINT_Open(&g_NexusCore.publicHandles.bint, g_NexusCore.publicHandles.reg, pIntSettings, &intCustomSettings);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_int;
    }
#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_ReleaseResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif

    /* User can export custom_arc=y to have direct control of the MEMC ARC (address range checker) HW */
    if (!NEXUS_GetEnv("custom_arc")) {
        for(i=0;i<NEXUS_MAX_HEAPS;i++) {
            NEXUS_MemoryStatus memStatus;

            if (!g_NexusCore.publicHandles.heap[i].nexus) continue;

            if (!g_NexusCore.publicHandles.heap[i].mma) continue; /* NEXUS_MEMORY_TYPE_RESERVED may have no BMMA heap */

            (void)NEXUS_Heap_GetStatus_driver(g_NexusCore.publicHandles.heap[i].nexus, &memStatus);

            /* must have at least one heap for MEMC. also check memc[] bounds. */
            if (memStatus.memcIndex >= NEXUS_MAX_HEAPS || memStatus.memcIndex >= NEXUS_MAX_MEMC) {
                BDBG_ERR(("Invalid memcIndex %d", memStatus.memcIndex));
                continue;
            }

            if (!g_NexusCore.publicHandles.memc[memStatus.memcIndex].mrc) {
                BMRC_Settings mrcSettings;
                BMRC_Monitor_Settings mrcMonitorSettings;
                NEXUS_Addr memcOffset;
                uint64_t memcSize;

                (void)BMRC_GetDefaultSettings(&mrcSettings);
                mrcSettings.usMemcId = memStatus.memcIndex;
                rc = BMRC_Open(&g_NexusCore.publicHandles.memc[memStatus.memcIndex].mrc, g_NexusCore.publicHandles.reg, g_NexusCore.publicHandles.bint, &mrcSettings);
                if (rc!=BERR_SUCCESS) {
                    rc = BERR_TRACE(rc);
                    goto err_mrc;
                }

                BMRC_Monitor_GetDefaultSettings(&mrcMonitorSettings);

                memcOffset = g_NexusCore.cfg.memoryLayout.memc[memStatus.memcIndex].region[0].addr;
                memcSize = g_NexusCore.cfg.memoryLayout.memc[memStatus.memcIndex].region[0].size;
                BDBG_MSG(("MEMC%u " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT "", memStatus.memcIndex, BDBG_UINT64_ARG(memcOffset), BDBG_UINT64_ARG(memcSize)));

                BDBG_ASSERT(memcOffset+memcSize >= memcOffset); /* check that addresses wouldn't wrap */
                mrcMonitorSettings.startDisabled = skipInitialReset;
                rc = BMRC_Monitor_Open(&g_NexusCore.publicHandles.memc[memStatus.memcIndex].rmm, g_NexusCore.publicHandles.reg, g_NexusCore.publicHandles.bint, g_NexusCore.publicHandles.chp,
                        g_NexusCore.publicHandles.memc[memStatus.memcIndex].mrc, memcOffset, memcOffset+memcSize, &mrcMonitorSettings);
                if (rc!=BERR_SUCCESS) {
                    rc = BERR_TRACE(rc);
                    goto err_mrc;
                }
            }

            rc = BMRC_Monitor_GetMemoryInterface(g_NexusCore.publicHandles.memc[memStatus.memcIndex].rmm, &g_NexusCore.publicHandles.memc[memStatus.memcIndex].mem_monitor);
            if (rc!=BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto err_mrc;
            }

#if BMMA_USE_STUB
            rc = BMEM_InstallMonitor(g_NexusCore.publicHandles.heap[i].mem, &g_NexusCore.publicHandles.memc[memStatus.memcIndex].mem_monitor);
            if (rc!=BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto err_mrc;
            }
#endif
            if( (memStatus.memoryType & NEXUS_MEMORY_TYPE_SECURE) == NEXUS_MEMORY_TYPE_SECURE) {
                NEXUS_HeapRuntimeSettings settings;
                NEXUS_Heap_GetRuntimeSettings_priv(g_NexusCore.publicHandles.heap[i].nexus, &settings);
                settings.secure = true;
                NEXUS_Heap_SetRuntimeSettings_priv(g_NexusCore.publicHandles.heap[i].nexus, &settings);
            }
        }
    }
    else {
        BDBG_WRN(("************************************************"));
        BDBG_WRN(("************************************************"));
        BDBG_WRN(("custom_arc=y tells Nexus to leave ARC's alone so they can be manually programmed. A reboot is required to disable them."));
        BDBG_WRN(("************************************************"));
        BDBG_WRN(("************************************************"));
        BMRC_PrintBlockingArcs(g_NexusCore.publicHandles.reg);
    }

    rc = NEXUS_Core_P_Profile_Init();
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_profile;
    }

    rc = BTMR_GetDefaultSettings(&tmr_settings);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_tmr_cfg;
    }

    #if NEXUS_TMR_EXCLUSION_MASK
    /* Use default timer settings unless user sets NEXUS_TMR_EXCLUSION_MASK in nexus_platform_features.h. Linux sometimes uses the last timer on certain chips.
    The timerMask (below) indicates which timers should be EXCLUDED from the list of available timers for nexus/magnum allocation and use.
    If 0 (default), no timers are using used externally. If 2, timer 1 is being used; if 6, timers 1&2 are being used, etc. */
    tmr_settings.timerMask = NEXUS_TMR_EXCLUSION_MASK;
    #endif

    rc = BTMR_Open(&g_NexusCore.publicHandles.tmr, g_NexusCore.publicHandles.chp, g_NexusCore.publicHandles.reg, g_NexusCore.publicHandles.bint, &tmr_settings);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_tmr;
    }
    g_NexusCore.timer = NEXUS_ScheduleTimer(NEXUS_CORE_TIMER_FREQ, NEXUS_Core_P_Timer, NULL);
    if(!g_NexusCore.timer) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err_coretimer;
    }
#if NEXUS_AVS_MONITOR
    if (!NEXUS_GetEnv("disable_avs")) {
        g_NexusCore.pvtTimer = NEXUS_ScheduleTimer(2000, NEXUS_Core_P_MonitorPvt, NULL);
        if(!g_NexusCore.pvtTimer) {
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err_pvttimer;
        }
    }
#endif

#if 0
    /* adds isr time to NEXUS_Core_P_Timer stats. you also need to enable code in bint.c. */
    rc = BINT_Stats_Enable(g_NexusCore.publicHandles.bint, g_NexusCore.publicHandles.tmr);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err_int;
    }
#endif

    g_NexusCore.publicHandles.defaultHeapIndex = pSettings->defaultHeapIndex;
    g_pCoreHandles = &g_NexusCore.publicHandles;

    NEXUS_PowerManagement_Init();
    BTRC_TRACE(500ms_tick, START);

    rc = NEXUS_Watchdog_P_Init();
    if (rc) {rc = BERR_TRACE(rc); goto err_watchdog;}

    NEXUS_UnlockModule();
    return g_NexusCore.module;

err_watchdog:
    NEXUS_PowerManagement_Uninit();
#if NEXUS_AVS_MONITOR
    NEXUS_CancelTimer(g_NexusCore.pvtTimer);
err_pvttimer:
#endif
    NEXUS_CancelTimer(g_NexusCore.timer);
err_coretimer:
    BTMR_Close(g_NexusCore.publicHandles.tmr);
err_tmr:
err_tmr_cfg:
    NEXUS_Core_P_Profile_Uninit();
err_profile:
err_mrc:
    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_NexusCore.publicHandles.memc[i].rmm) {
            BMRC_Monitor_Close(g_NexusCore.publicHandles.memc[i].rmm);
        }
        if (g_NexusCore.publicHandles.memc[i].mrc) {
            BMRC_Close(g_NexusCore.publicHandles.memc[i].mrc);
        }
    }
    BINT_Close(g_NexusCore.publicHandles.bint);
err_int:
err_heap:
    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_NexusCore.publicHandles.heap[i].nexus) {
            NEXUS_Heap_Destroy_priv(g_NexusCore.publicHandles.heap[i].nexus);
        }
    }
#if !BMMA_USE_STUB
    BMMA_Destroy(g_NexusCore.publicHandles.mma);
err_mma:
#endif
#if !BMEM_DEPRECATED
    BMEM_Close(g_NexusCore.publicHandles.mem);
err_mem:
#endif
err_mem_cfg:
    BCHP_Close(g_NexusCore.publicHandles.chp);
err_boxloadrts:
err_chp:
    g_NexusCore.publicHandles.reg = NULL; /* reg handle is passed in */
    NEXUS_Memory_P_Uninit();
err_memory:
    BMMA_PoolAllocator_Destroy(g_NexusCore.memoryBlockPool);
err_heap_handle_pool:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NexusCore.module);
err_module:
err_params:
    return NULL;
    /* coverity[unreachable] */
    goto err_mem_cfg; /* never reached, silences compiler warning about unused label */
}

void NEXUS_CoreModule_PostInit(void)
{
    unsigned i;
#if NEXUS_POWER_MANAGEMENT
    /* BCHP_Open will initialize (i.e. power down) all MAGNUM_CONTROLLED nodes.
       This will initialize the rest */
    BCHP_PWR_InitAllHwResources(g_NexusCore.publicHandles.chp);
#endif
    for(i=0;i<NEXUS_MAX_MEMC;i++) {
        if (g_NexusCore.publicHandles.memc[i].rmm) {
            BMRC_Monitor_SetEnabled(g_NexusCore.publicHandles.memc[i].rmm, true);
        }
    }
}

void
NEXUS_CoreModule_Uninit(void)
{
    unsigned i;

    NEXUS_LockModule();
    NEXUS_Core_P_PrintStatistics();
    BTRC_Module_Report(BTRC_MODULE_HANDLE(500ms_tick));

    NEXUS_Watchdog_P_Uninit();
    NEXUS_PowerManagement_Uninit();

    BDBG_ASSERT(g_pCoreHandles);

    if(g_NexusCore.timer) {
        NEXUS_CancelTimer(g_NexusCore.timer);
    }
#if NEXUS_AVS_MONITOR
    if(g_NexusCore.pvtTimer) {
      NEXUS_CancelTimer(g_NexusCore.pvtTimer);
    }
#endif

    BTMR_Close(g_NexusCore.publicHandles.tmr);
    NEXUS_Core_P_Profile_Uninit();
    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_NexusCore.publicHandles.memc[i].rmm) {
            BMRC_Monitor_Close(g_NexusCore.publicHandles.memc[i].rmm);
        }
        if (g_NexusCore.publicHandles.memc[i].mrc) {
            BMRC_Close(g_NexusCore.publicHandles.memc[i].mrc);
        }
        if (g_NexusCore.publicHandles.memc[i].dtu) {
            BDTU_Destroy(g_NexusCore.publicHandles.memc[i].dtu);
        }
        if(g_NexusCore.publicHandles.heap[i].nexus) {
            NEXUS_Heap_Destroy_priv(g_NexusCore.publicHandles.heap[i].nexus);
        }
    }
#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_AcquireResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif
    BINT_Close(g_NexusCore.publicHandles.bint);
#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_ReleaseResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif
#if !BMMA_USE_STUB
    BMMA_Destroy(g_NexusCore.publicHandles.mma);
#endif
#if !BMEM_DEPRECATED
    BMEM_Close(g_NexusCore.publicHandles.mem);
#endif
    BCHP_Close(g_NexusCore.publicHandles.chp);
    g_NexusCore.publicHandles.reg = NULL; /* reg handle is passed in */
    g_pCoreHandles = NULL;
    NEXUS_Memory_P_Uninit();
    BMMA_PoolAllocator_Destroy(g_NexusCore.memoryBlockPool);
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NexusCore.module);
    BTRC_TRACE(500ms_tick, STOP);
    return;
}

NEXUS_Error NEXUS_CoreModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned i;

    if(enabled) {
        if(pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
            BTMR_Standby(g_NexusCore.publicHandles.tmr);
            for(i=0;i<NEXUS_MAX_MEMC;i++) {
                if(g_NexusCore.publicHandles.memc[i].mrc) {
                    BMRC_Standby(g_NexusCore.publicHandles.memc[i].mrc);
                }
            }
            BCHP_Standby(g_NexusCore.publicHandles.chp);
            BCHP_PWR_Standby(g_NexusCore.publicHandles.chp, NULL); /* currently a no-op, because BCHP_PWR_Standby() doesn't have to do anything special */
            /* DMA clock needs to be powered ON for secure memory hash */
#ifdef BCHP_PWR_RESOURCE_DMA
            BCHP_PWR_AcquireResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_DMA);
#endif
            g_NexusCore.standby = true;
        }
    } else {
        if(g_NexusCore.standby) {
            /* DMA clock was acquired for secure memory hash. So release it firs after resume */
#ifdef BCHP_PWR_RESOURCE_DMA
            BCHP_PWR_ReleaseResource(g_NexusCore.publicHandles.chp, BCHP_PWR_RESOURCE_DMA);
#endif
            BCHP_PWR_Resume(g_NexusCore.publicHandles.chp); /* on a resume from S3, the HW is running at full power. BCHP_PWR_Resume() powers down so the SW and HW states match */
            BCHP_Resume(g_NexusCore.publicHandles.chp);
            for(i=0;i<NEXUS_MAX_MEMC;i++) {
                if(g_NexusCore.publicHandles.memc[i].mrc){
                    BMRC_Resume(g_NexusCore.publicHandles.memc[i].mrc);
                }
            }
            BTMR_Resume(g_NexusCore.publicHandles.tmr);
            g_NexusCore.standby = false;
        }
    }
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif

    return NEXUS_SUCCESS;
}

BERR_Code
NEXUS_Core_EnableInterrupt(unsigned irqNum)
{
    BERR_Code rc;
    BKNI_EnterCriticalSection();
    rc = g_NexusCore.cfg.interruptInterface.pEnableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();
    return rc;
}

BERR_Code
NEXUS_Core_EnableInterrupt_isr(unsigned irqNum)
{
    BERR_Code rc;
    rc = g_NexusCore.cfg.interruptInterface.pEnableInterrupt_isr(irqNum);
    return rc;
}

void
NEXUS_Core_DisableInterrupt(unsigned irqNum)
{
    BKNI_EnterCriticalSection();
    g_NexusCore.cfg.interruptInterface.pDisableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();
    return;
}

void
NEXUS_Core_DisableInterrupt_isr(unsigned irqNum)
{
    g_NexusCore.cfg.interruptInterface.pDisableInterrupt_isr(irqNum);
    return;
}

BERR_Code
NEXUS_Core_ConnectInterrupt(unsigned irqNum, NEXUS_Core_InterruptFunction pIsrFunc, void *pFuncParam, int iFuncParam)
{
    return g_NexusCore.cfg.interruptInterface.pConnectInterrupt(irqNum, pIsrFunc, pFuncParam, iFuncParam);
}

void
NEXUS_Core_DisconnectInterrupt(unsigned irqNum)
{
    g_NexusCore.cfg.interruptInterface.pDisconnectInterrupt(irqNum);
    return;
}

static bool NEXUS_Core_P_AddressInRegion(NEXUS_Core_MemoryRegion *pRegion, void *pAddress, void **ppUncachedAddress)
{
    if ( pAddress >= pRegion->pvAddr && pAddress < (void *)((uint8_t *)pRegion->pvAddr + pRegion->length) )
    {
        if (ppUncachedAddress) {
            *ppUncachedAddress = pAddress;
        }
        return true;
    }
    else if ( pAddress >= pRegion->pvAddrCached && pAddress < (void *)((uint8_t *)pRegion->pvAddrCached + pRegion->length) )
    {
        if (ppUncachedAddress) {
            *ppUncachedAddress = ((uint8_t *)pRegion->pvAddr + ((uint8_t *)pAddress - (uint8_t *)pRegion->pvAddrCached));
        }
        return true;
    }

    return false;
}

#if !BMEM_DEPRECATED
BMEM_Heap_Handle NEXUS_Core_P_AddressToHeap(void *pAddress, void **ppUncachedAddress)
{
    int i;
    for ( i = 0; i < NEXUS_MAX_HEAPS; i++ )
    {
        NEXUS_Core_MemoryRegion *pRegion = &g_NexusCore.cfg.heapRegion[i];
        if ( NEXUS_Core_P_AddressInRegion(pRegion, pAddress, ppUncachedAddress) )
        {
            return g_pCoreHandles->heap[i].mem;
        }
    }
    return NULL;
}
#endif

#include "nexus_security_types.h"
#include "priv/nexus_core_security.h"

NEXUS_KeySlotHandle
NEXUS_KeySlot_Create(void)
{
    NEXUS_KeySlotHandle keyslot;
    keyslot = (NEXUS_KeySlotHandle)BKNI_Malloc(sizeof(*keyslot));
    if ( !keyslot) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }
    NEXUS_OBJECT_INIT(NEXUS_KeySlot, keyslot);
    NEXUS_OBJECT_REGISTER(NEXUS_KeySlot, keyslot, Create);
    keyslot->settings.keySlotEngine = NEXUS_SecurityEngine_eGeneric;
    return keyslot;
}

void NEXUS_KeySlot_GetFastInfo( NEXUS_KeySlotHandle keyslot, NEXUS_KeySlotFastInfo *pInfo )
{
    BDBG_OBJECT_ASSERT(keyslot, NEXUS_KeySlot);

    if (!pInfo){ BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    pInfo->dma.pidChannelIndex = keyslot->dma.pidChannelIndex;
    pInfo->dma.valid = keyslot->dma.valid;
    pInfo->keySlotNumber = keyslot->keySlotNumber;

    return;
}

void NEXUS_KeySlot_GetInfo( NEXUS_KeySlotHandle keyHandle, NEXUS_SecurityKeySlotInfo *pKeyslotInfo )
{
  #if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION==2)
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED); /* use NEXUS_KeySlot_GetInfomation */
    return;
  #else
    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    BKNI_Memset(pKeyslotInfo, 0, sizeof(*pKeyslotInfo));
    pKeyslotInfo->keySlotNumber = keyHandle->keySlotNumber;
    pKeyslotInfo->keySlotEngine = keyHandle->settings.keySlotEngine;
    pKeyslotInfo->keySlotType = keyHandle->settings.keySlotType;
    pKeyslotInfo->dma.pidChannelIndex = keyHandle->dma.pidChannelIndex;
    return;
  #endif
}


static struct {
    BLST_S_HEAD(closed_keyslots, NEXUS_KeySlot) closed;
} g_keyslot;

NEXUS_KeySlotHandle NEXUS_KeySlot_P_GetDeferredDestroy(void)
{
    NEXUS_KeySlotHandle keyslot;
    NEXUS_LockModule();
    keyslot = BLST_S_FIRST(&g_keyslot.closed);
    if (keyslot) {
        BLST_S_REMOVE_HEAD(&g_keyslot.closed, closed_link);
    }
    NEXUS_UnlockModule();
    return keyslot;
}

static void
NEXUS_KeySlot_P_Release(NEXUS_KeySlotHandle keyslot)
{
    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keyslot);
    NEXUS_OBJECT_UNREGISTER(NEXUS_KeySlot, keyslot, Destroy);
}

static void
NEXUS_KeySlot_P_Finalizer(NEXUS_KeySlotHandle keyslot)
{
    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keyslot);
    if (keyslot->deferDestroy) {
        BLST_S_INSERT_HEAD(&g_keyslot.closed, keyslot, closed_link);
    }
    else {
        NEXUS_OBJECT_DESTROY(NEXUS_KeySlot, keyslot);
        BKNI_Free(keyslot);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_KeySlot, NEXUS_KeySlot_Destroy);

void NEXUS_KeySlot_P_DeferredDestroy(NEXUS_KeySlotHandle keyslot)
{
    NEXUS_LockModule();
    BDBG_ASSERT(keyslot->deferDestroy);
    keyslot->deferDestroy = false;
    NEXUS_KeySlot_P_Finalizer(keyslot);
    NEXUS_UnlockModule();
}

static void NEXUS_VideoInput_P_Finalizer(NEXUS_VideoInputHandle videoInput)
{
    BSTD_UNUSED(videoInput);
    return;
}

NEXUS_VideoInputHandle NEXUS_VideoInput_Create(void)
{
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_VideoInput, NEXUS_VideoInput_Destroy);

static void NEXUS_VideoOutput_P_Finalizer(NEXUS_VideoOutputHandle videoOutput)
{
    BSTD_UNUSED(videoOutput);
    return;
}

NEXUS_VideoOutputHandle NEXUS_VideoOutput_Create(void)
{
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_VideoOutput, NEXUS_VideoOutput_Destroy);

static void NEXUS_AudioOutput_P_Finalizer(NEXUS_AudioOutputHandle audioOutput)
{
    BSTD_UNUSED(audioOutput);
    return;
}

NEXUS_AudioOutputHandle NEXUS_AudioOutput_Create(void)
{
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioOutput, NEXUS_AudioOutput_Destroy);

static void NEXUS_AudioInput_P_Finalizer(NEXUS_AudioInputHandle audioInput)
{
    BSTD_UNUSED(audioInput);
    return;
}

NEXUS_AudioInputHandle NEXUS_AudioInput_Create(void)
{
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioInput, NEXUS_AudioInput_Destroy);

void NEXUS_GetDefaultCommonModuleSettings( NEXUS_CommonModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModulePriority NEXUS_AdjustModulePriority( NEXUS_ModulePriority priority, const NEXUS_CommonModuleSettings *pSettings )
{
    switch (pSettings->standbyLevel) {
        case NEXUS_ModuleStandbyLevel_eActive:
            switch (priority) {
                case NEXUS_ModulePriority_eIdle: return NEXUS_ModulePriority_eIdleActiveStandby;
                case NEXUS_ModulePriority_eLow: return NEXUS_ModulePriority_eLowActiveStandby;
                case NEXUS_ModulePriority_eDefault: return NEXUS_ModulePriority_eDefaultActiveStandby;
                case NEXUS_ModulePriority_eHigh: return NEXUS_ModulePriority_eHighActiveStandby;
                default: break;
            }
            break;
        case NEXUS_ModuleStandbyLevel_eAlwaysOn:
                return NEXUS_ModulePriority_eAlwaysOn;
            break;
        default:
            break;
    }
    return priority;
}
