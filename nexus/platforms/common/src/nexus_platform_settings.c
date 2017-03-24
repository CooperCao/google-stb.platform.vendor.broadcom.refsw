/******************************************************************************
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
 ******************************************************************************/
#include "nexus_platform_priv.h"
#include "nexus_types.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "bi2c.h"
#include "priv/nexus_i2c_priv.h"
#if NEXUS_HAS_FILE
#include "nexus_file_init.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "priv/nexus_hdmi_output_priv.h"
#endif
#if NEXUS_COMPAT_32ABI
#include "nexus_base_compat.h"
#include "nexus_platform_api_compat.h"
#endif
#include "bchp_common.h"

BDBG_MODULE(nexus_platform_settings);

#if BCHP_HDMI_TX_AUTO_I2C_REG_START && NEXUS_HAS_HDMI_OUTPUT
#define AUTO_I2C_ENABLED 1
#else
#define AUTO_I2C_ENABLED 0
#endif

static void NEXUS_Platform_P_AdjustBmemRegions(NEXUS_PlatformMemory *pMemory);

void NEXUS_Platform_GetDefaultSettings_tagged( NEXUS_PlatformSettings *pSettings, size_t size )
{
    const NEXUS_Core_PreInitState *preInitState;
    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) return; /* no BERR_TRACE */
    if(
       size==sizeof(*pSettings)
#if NEXUS_COMPAT_32ABI
       || size==sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformSettings))
#endif
       ) {
        NEXUS_Platform_Priv_GetDefaultSettings(preInitState, pSettings);
    } else {
#if NEXUS_COMPAT_32ABI
        BDBG_ERR(("NEXUS_Platform_GetDefaultSettings - Mixed ABI:size mismatch %u != %u/%u", (unsigned)sizeof(*pSettings), (unsigned)size,(unsigned)sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformSettings))));
#else
        BDBG_ERR(("NEXUS_Platform_GetDefaultSettings:size mismatch %u != %u", (unsigned)sizeof(*pSettings), (unsigned)size));
#endif
        BKNI_Memset(pSettings, 0, size);
    }
    NEXUS_Platform_P_PreUninit();
    return;
}

static void NEXUS_Platform_P_AdjustHeapSettings(NEXUS_PlatformSettings *pSettings)
{
    unsigned i;
    NEXUS_PlatformHeapSettings *heap = pSettings->heap;

    /* set heapType based on nexus_platform_features.h macros for now. if they are deprecated or become non-standard, this
    logic will move to platform-specific heap initialization. */
#if NEXUS_PLATFORM_DEFAULT_HEAP != 0
#error NEXUS_PLATFORM_DEFAULT_HEAP is deprecated. we should keep main heap as 0
#endif
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType |= NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memoryType |= NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].memcIndex = 2;
    pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].memoryType |= NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].memcIndex = 2;
    pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].heapType |= NEXUS_HEAP_TYPE_MAIN;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType |= NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].heapType |= NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION;
    pSettings->heap[NEXUS_EXPORT_HEAP].heapType |= NEXUS_HEAP_TYPE_EXPORT_REGION;
#if NEXUS_HAS_SAGE
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].placement.sage = true;
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].alignment = 16 * 1024 * 1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].alignment = 16 * 1024 * 1024;
#else
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].alignment = 1 * 1024 * 1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].alignment = 1 * 1024 * 1024;
#endif
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].placement.sage = true;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eSecure;
    pSettings->heap[NEXUS_EXPORT_HEAP].alignment = 1 * 1024 * 1024;
    pSettings->heap[NEXUS_EXPORT_HEAP].memoryType = NEXUS_MemoryType_eSecure;
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].size =  32*1024*1024; /*Sage Secure heap */
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].heapType |= NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION;
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memoryType = NEXUS_MemoryType_eSecure;
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].alignment = 16 * 1024 * 1024;
#else
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].alignment = 1 * 1024 * 1024;
#endif
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].placement.first = true;
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].placement.sage = true;
#endif

    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        size_t aligned_size;
        size_t heap_size;
        if(heap[i].size==0 || heap[i].size<0) {
            continue;
        }
        if(heap[i].alignment==0) {
            continue;
        }
        heap_size = heap[i].size;
        aligned_size = NEXUS_P_SizeAlign(heap_size, heap[i].alignment);

        if(aligned_size != heap_size) {
            BDBG_WRN(("Aligned size of heap%u on MEMC%u from %uMBytes to %uMByte", i, heap[i].memcIndex, (unsigned)(heap_size/(1024*1024)),(unsigned)(aligned_size/(1024*1024))));
            heap[i].size = aligned_size;
        }
    }
    return;
}

void NEXUS_Platform_P_GetDefaultHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    NEXUS_Platform_P_GetPlatformHeapSettings(pSettings, boxMode);
    NEXUS_Platform_P_AdjustHeapSettings(pSettings);
    return;
}

void NEXUS_Platform_Priv_GetDefaultSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformSettings *pSettings )
{
    NEXUS_Error errCode;
    NEXUS_PlatformMemory *pMemory = &g_platformMemory;
    NEXUS_MemoryConfigurationSettings *pMemConfigSettings;
    NEXUS_MemoryRtsSettings rtsSettings;
    unsigned i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings)); /* don't call BKNI_Memset prior to initializing magnum */
    if (!g_NEXUS_platformModule && g_NEXUS_platformSettings.heap[0].size) {
        /* if NEXUS_Platform_GetDefaultSettings is called more than once before Init, return a cached value */
        if (pSettings != &g_NEXUS_platformSettings) {
            *pSettings = g_NEXUS_platformSettings;
        }
        return;
    }

    pSettings->mode = NEXUS_ClientMode_eVerified;
    pSettings->cachedMemory = true; /* Default to cached memory */

    pSettings->openI2c = true;
    #if (NEXUS_PLATFORM==97019)
    pSettings->openFpga = false;
    #else
    pSettings->openFpga = true;
    #endif
    pSettings->openFrontend = true;
    pSettings->openOutputs = true;
    pSettings->openCec = true;
    if (!NEXUS_GetEnv("nexus_binary_compat")) {
        pSettings->checkPlatformType = true;
    }

#if NEXUS_HAS_TRANSPORT
    NEXUS_TransportModule_GetDefaultSettings(&pSettings->transportModuleSettings);
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_DisplayModule_GetDefaultSettings(preInitState, &pSettings->displayModuleSettings);
#endif

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModule_GetDefaultSettings(&pSettings->videoDecoderModuleSettings);
    if (NEXUS_GetEnv("avd_monitor")) {
        pSettings->videoDecoderModuleSettings.debugLogBufferSize = 10 * 1024;
    }
#endif

#if NEXUS_HAS_AUDIO
    NEXUS_AudioModule_GetDefaultSettings(preInitState, &pSettings->audioModuleSettings);
    #if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if ( NEXUS_GetEnv("audio_logs_enabled") )
    {
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eUartMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eDramMessage].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled = true;
        pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eTargetPrint].enabled = true;
    }
    else
    {
        if ( NEXUS_GetEnv("audio_uart_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eUartMessage].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_debug_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eDramMessage].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_core_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled = true;
        }
        if ( NEXUS_GetEnv("audio_target_print_file") )
        {
            pSettings->audioModuleSettings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eTargetPrint].enabled = true;
        }
    }
    #endif
#endif

#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0;i<NEXUS_NUM_HDMI_OUTPUTS;i++) {
        NEXUS_HdmiOutput_GetDefaultOpenSettings_isrsafe(&pSettings->hdmiOutputOpenSettings[i]);
    }
#else
    BSTD_UNUSED(i);
#endif

#if NEXUS_HAS_SMARTCARD
    NEXUS_SmartcardModule_GetDefaultSettings(&pSettings->smartCardSettings);
    pSettings->smartCardSettings.clockSource= NEXUS_SmartcardClockSource_eInternalClock;
#if NEXUS_BOARD_7530_CRB
    pSettings->smartCardSettings.clockFrequency = 24000000;
#else
    pSettings->smartCardSettings.clockFrequency = 27000000;
#endif
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModule_GetDefaultSettings(&pSettings->videoEncoderSettings);
#endif

#if NEXUS_HAS_SECURITY
    NEXUS_SecurityModule_GetDefaultSettings(&pSettings->securitySettings);
#endif

#if NEXUS_HAS_GRAPHICS2D
    NEXUS_Graphics2DModule_GetDefaultSettings(&pSettings->graphics2DModuleSettings);
#endif

#if NEXUS_HAS_FILE
    {
        NEXUS_FileModuleSettings fileModuleSettings;
        NEXUS_FileModule_GetDefaultSettings(&fileModuleSettings);
        pSettings->fileModuleSettings.workerThreads = fileModuleSettings.workerThreads;
        BDBG_CASSERT(sizeof(pSettings->fileModuleSettings.schedulerSettings) == sizeof(fileModuleSettings.schedulerSettings));
        BKNI_Memcpy(pSettings->fileModuleSettings.schedulerSettings, fileModuleSettings.schedulerSettings, sizeof(pSettings->fileModuleSettings.schedulerSettings));
    }
#endif

#if NEXUS_HAS_SAGE
    NEXUS_SageModule_GetDefaultSettings(&pSettings->sageModuleSettings);
#endif

#if NEXUS_HAS_PWM
    NEXUS_PwmModule_GetDefaultSettings(&pSettings->pwmSettings);
#endif

#if NEXUS_POWER_MANAGEMENT
    NEXUS_Platform_GetStandbySettings(&pSettings->standbySettings);
#endif
#if NEXUS_HAS_I2C
    for (i=0;i<NEXUS_MAX_I2C_CHANNELS;i++) {
        NEXUS_I2c_GetDefaultSettings_priv(&pSettings->i2c[i].settings);
        if ( i == NEXUS_I2C_CHANNEL_HDMI_TX )
        {
#if AUTO_I2C_ENABLED
            pSettings->i2c[i].settings.autoI2c.enabled = true;
#endif
            pSettings->i2c[i].settings.use5v = true;
        }
        pSettings->i2c[i].open = true;
    }
#endif

    pMemConfigSettings = BKNI_Malloc(sizeof(*pMemConfigSettings));
    if (!pMemConfigSettings) {errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_malloc_settings;}

    NEXUS_P_GetDefaultMemoryConfigurationSettings(preInitState, pMemConfigSettings);

    BKNI_Memset(pMemory, 0, sizeof(*pMemory));

    errCode = NEXUS_Platform_P_GetHostMemory(pMemory);
    if (errCode) {
        errCode = BERR_TRACE(errCode);
        /* fall through */
    }
    errCode = NEXUS_Platform_P_CalcSubMemc(preInitState, &pMemory->memoryLayout);
    if (errCode) {
        errCode = BERR_TRACE(errCode);
        /* fall through */
    }
    NEXUS_Platform_P_AdjustBmemRegions(pMemory);
    NEXUS_P_GetDefaultMemoryRtsSettings(preInitState, &rtsSettings);
    NEXUS_Platform_P_GetDefaultHeapSettings(pSettings, preInitState->boxMode);
    errCode = NEXUS_P_ApplyMemoryConfiguration(preInitState, pMemConfigSettings, &rtsSettings, pSettings, g_NEXUS_platformModule==NULL? &g_NEXUS_platformInternalSettings : NULL); /* update g_NEXUS_platformInternalSettings if nexus was not initialized */
    if (errCode) {
        /* we can't fail the function, but we can make sure we don't squeak by */
        BERR_TRACE(errCode);
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }

    if (!g_NEXUS_platformModule && pSettings != &g_NEXUS_platformSettings) {
        /* if nexus not initialized, use that storage to cache GetDefaultSettings */
        g_NEXUS_platformSettings = *pSettings;
    }

    BKNI_Free(pMemConfigSettings);
err_malloc_settings:
    return;
}

void NEXUS_Platform_GetSettings( NEXUS_PlatformSettings *pSettings )
{
    *pSettings = g_NEXUS_platformSettings;
}

/*
dynamically creates regions the OS leaves out.
assigns bmem regions to MEMC regions. if not assignment is possible, the bmem region is zeroed out.
*/
static void NEXUS_Platform_P_AdjustBmemRegions(NEXUS_PlatformMemory *pMemory)
{
    unsigned i, j, k;

    BSTD_UNUSED(j);
    BSTD_UNUSED(k);

    /* dump all OS regions and all nexus regions for debug */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (pMemory->osRegion[i].length) {
#if NEXUS_USE_CMA
            /* for ARM, cma regions are only potential device memory. whatever nexus doesn't use, linux can. */
            const char *labelstr = "max cma";
#else
            /* for MIPS, bmem regions represent allocated device memory. linux can never touch. */
            const char *labelstr = "os bmem";
#endif
            BDBG_MSG(("%s.%d offset " BDBG_UINT64_FMT ", size %u", labelstr, i, BDBG_UINT64_ARG(pMemory->osRegion[i].base), (unsigned)pMemory->osRegion[i].length));
        }
    }
    for(i=0;i<sizeof(pMemory->memoryLayout.memc)/sizeof(pMemory->memoryLayout.memc[0]);i++) {
        for(j=0;j<sizeof(pMemory->memoryLayout.memc[0].region)/sizeof(pMemory->memoryLayout.memc[0].region[0]);j++) {
            if(pMemory->memoryLayout.memc[i].region[j].size) {
                BDBG_MSG(("memc%u[%u] offset " BDBG_UINT64_FMT ", size %u", i, j, BDBG_UINT64_ARG(pMemory->memoryLayout.memc[i].region[j].addr), (unsigned)pMemory->memoryLayout.memc[i].region[j].size));
            }
        }
    }
#if NEXUS_USE_CMA
    return;
#endif

    /* coverity[unreachable: FALSE] */
    /* determine MEMC index/subIndex for each bmem region */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        bool done = false;
        bool cma = pMemory->osRegion[0].cma;

        if (!pMemory->osRegion[i].length) continue;
        if(!cma && pMemory->osRegion[i].cma) {
            continue;
        }

        for (j=0;j<NEXUS_MAX_MEMC && !done;j++) {
            for (k=0;k<NEXUS_NUM_MEMC_REGIONS;k++) {
                if (pMemory->osRegion[i].base >= pMemory->memoryLayout.memc[j].region[k].addr &&
                    pMemory->osRegion[i].base < pMemory->memoryLayout.memc[j].region[k].addr + pMemory->memoryLayout.memc[j].region[k].size)
                {
                    NEXUS_Addr bound;
                    pMemory->osRegion[i].memcIndex = j;
                    pMemory->osRegion[i].subIndex = k;
                    /* linux 2.6.18 doesn't report size, so use MEMC info to bound each bmem region */
                    bound = pMemory->memoryLayout.memc[j].region[k].addr + pMemory->memoryLayout.memc[j].region[k].size;
                    if (pMemory->osRegion[i].base + pMemory->osRegion[i].length > bound) {
                        pMemory->osRegion[i].length = bound - pMemory->osRegion[i].base;
                    }
                    done = true; break; /* double break */
                }
            }
            if (done) break;
        }
        if (j == NEXUS_MAX_MEMC) {
            BDBG_ERR(("unable to assign bmem.%d offset " BDBG_UINT64_FMT " to MEMC region", i, BDBG_UINT64_ARG(pMemory->osRegion[i].base)));
            pMemory->osRegion[i].length = 0;
        }
    }
}

#define PAGE_ALIGN(x) ((unsigned)(x)& ~0xfff)

static const char *NEXUS_P_PlatformHeapName(unsigned heapIndex, const NEXUS_PlatformSettings *pSettings, char *buf, unsigned buf_size)
{
    NEXUS_MemoryStatus status;
    /* Must copy all members neeeded by NEXUS_Heap_ToString. NEXUS_MemoryStatus is not available at this time. */
    status.memcIndex = pSettings->heap[heapIndex].memcIndex;
    status.heapType = pSettings->heap[heapIndex].heapType;
    status.memoryType = pSettings->heap[heapIndex].memoryType;
    NEXUS_Heap_ToString(&status, buf, buf_size);
    return buf;
}

/**
create heaps based on NEXUS_PlatformSettings.heap[] requests (which are based on MEMC index/subindex) and allowed bmem regions (stored in NEXUS_PlatformMemory.osRegion[]
**/
#if NEXUS_USE_CMA
NEXUS_Error NEXUS_Platform_P_SetCoreModuleSettings(const NEXUS_PlatformSettings *pSettings, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    NEXUS_Error rc;
    unsigned i;
    rc =  NEXUS_Platform_P_SetCoreCmaSettings(pSettings,pMemory,pCoreSettings);
#if BDBG_DEBUG_BUILD
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        char buf[64];

        if (!pCoreSettings->heapRegion[i].length) continue;
        BDBG_MSG(("creating heap[%d]: MEMC%d, offset " BDBG_UINT64_FMT ", size %u, %s",
            i,
            pCoreSettings->heapRegion[i].memcIndex,
            BDBG_UINT64_ARG(pCoreSettings->heapRegion[i].offset),
            (unsigned)pCoreSettings->heapRegion[i].length,
            NEXUS_P_PlatformHeapName(i,pSettings,buf,sizeof(buf))));
    }
#endif
    return rc;
}
#else
NEXUS_Error NEXUS_Platform_P_SetCoreModuleSettings(const NEXUS_PlatformSettings *pSettings, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    NEXUS_Error rc = 0;
    unsigned i, j;
    unsigned totalUsed[NEXUS_MAX_HEAPS];
    NEXUS_Addr heapOffset[NEXUS_MAX_HEAPS];
    int bmemIndex[NEXUS_MAX_HEAPS]; /* bmem index for each NEXUS_PlatformSettings.heap[] */
    char buf[64];

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        totalUsed[i] = heapOffset[i] = 0;
        bmemIndex[i] = -1;
    }

    /* step 1: assign bmemIndex[] and calculate the remainder per bmem region */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        int size = pSettings->heap[i].size;

        if (!size) {
            continue;
        }

        BDBG_MSG(("request heap[%d]: MEMC%d/%d, size %d, %s", i, pSettings->heap[i].memcIndex, pSettings->heap[i].subIndex,
            pSettings->heap[i].size, NEXUS_P_PlatformHeapName(i,pSettings,buf,sizeof(buf))));

        if (size == -1) {
            continue;
        }
        size = PAGE_ALIGN(size);

        /* find the first bmem in this MEMC region that has enough space.
        there is a dynamic mapping from NEXUS_PlatformSettings.heap[] to bmem region based on their order */
        for (j=0;j<NEXUS_MAX_HEAPS;j++) {
            BDBG_ASSERT(totalUsed[j] <= pMemory->osRegion[j].length);
            if (pSettings->heap[i].memcIndex == pMemory->osRegion[j].memcIndex &&
                pSettings->heap[i].subIndex == pMemory->osRegion[j].subIndex &&
                totalUsed[j] + size <= pMemory->osRegion[j].length)
            {
                bmemIndex[i] = j;
                break;
            }
        }
        if (j == NEXUS_MAX_HEAPS) {
            if (pSettings->heap[i].optional) {
                /* can't create this heap */
                continue;
            }
            else {
                BDBG_ERR(("No bmem found for heap[%d]: MEMC%d/%d, size %d, %s", i, pSettings->heap[i].memcIndex, pSettings->heap[i].subIndex,
                    pSettings->heap[i].size, NEXUS_P_PlatformHeapName(i,pSettings,buf,sizeof(buf))));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        BDBG_ASSERT(totalUsed[bmemIndex[i]] + size <= pMemory->osRegion[bmemIndex[i]].length);
        totalUsed[bmemIndex[i]] += size;
    }

    /* step 2: allocate the heaps, including determining the size=-1 heaps */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        int size;

        size = pSettings->heap[i].size;
        if (!size) {
            continue;
        }

        if (size == -1) {
            /* find bmem for the remainder heap */
            for (j=0;j<NEXUS_MAX_HEAPS;j++) {
                BDBG_ASSERT(totalUsed[j] <= pMemory->osRegion[j].length);
                if (totalUsed[j] == pMemory->osRegion[j].length) {
                    continue;
                }
                if (pSettings->heap[i].memcIndex == pMemory->osRegion[j].memcIndex &&
                    pSettings->heap[i].subIndex == pMemory->osRegion[j].subIndex)
                {
                    bmemIndex[i] = j;
                    break;
                }
            }
            if (j == NEXUS_MAX_HEAPS) {
                /* we cannot create a heap for this region */
                if (pSettings->heap[i].optional)
                    continue;
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto cannot_create_heap;
            }
            size = pMemory->osRegion[j].length - totalUsed[bmemIndex[i]];
            totalUsed[bmemIndex[i]] += size;
        }
        else {
            size = PAGE_ALIGN(size);
            if (bmemIndex[i] == -1) {
                BDBG_ASSERT(pSettings->heap[i].optional);
                /* fixed size optional heap that couldn't be created */
                continue;
            }
        }
        BDBG_ASSERT(bmemIndex[i] >= 0 && bmemIndex[i] < NEXUS_MAX_HEAPS);
        BDBG_ASSERT(size > 0);

        NEXUS_Heap_GetDefaultMemcSettings(pSettings->heap[i].memcIndex, &pCoreSettings->heapRegion[i]);
        /* if OS reports greater cache line size than BMEM's default, then increase */
        if (pMemory->max_dcache_line_size > pCoreSettings->heapRegion[i].alignment) {
            pCoreSettings->heapRegion[i].alignment = pMemory->max_dcache_line_size;
        }
        pCoreSettings->heapRegion[i].memoryType = pSettings->heap[i].memoryType;
        pCoreSettings->heapRegion[i].heapType = pSettings->heap[i].heapType;

#if !NEXUS_CPU_ARM
        if (pSettings->heap[i].alignment) {
            BDBG_WRN(("heap[%d].alignment %#x ignored on MIPS", i, pSettings->heap[i].alignment));
        }
#endif
        pCoreSettings->heapRegion[i].length = size;

#ifdef NEXUS_SECURITY_VIDEO_VERIFICATION_LEGACY_65NM
        if (i==0)
        {
            /* Reserve some memory for video FW */
            pCoreSettings->heapRegion[i].length = XVD_PHYSICAL_ADDRESS - (64 * 1024 * 1024);
            BDBG_WRN(("Forcing heap memory size to %d", pCoreSettings->heapRegion[i].length));
        }
#endif

        pCoreSettings->heapRegion[i].offset = pMemory->osRegion[bmemIndex[i]].base + heapOffset[bmemIndex[i]];

        heapOffset[bmemIndex[i]] += size;
    }

#if BDBG_DEBUG_BUILD
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pCoreSettings->heapRegion[i].length) continue;
        BDBG_MSG(("creating heap[%d]: MEMC%d, offset " BDBG_UINT64_FMT", size %u, %s",
            i,
            pCoreSettings->heapRegion[i].memcIndex,
            BDBG_UINT64_ARG(pCoreSettings->heapRegion[i].offset),
            pCoreSettings->heapRegion[i].length,
            NEXUS_P_PlatformHeapName(i,pSettings,buf,sizeof(buf))));
    }
#endif

    return rc;

cannot_create_heap:
    /* assumes i was left at heap of interest */
    BDBG_ERR(("cannot create heap[%d]: MEMC%d/%d, size %d, %s", i, pSettings->heap[i].memcIndex, pSettings->heap[i].subIndex,
        pSettings->heap[i].size, NEXUS_P_PlatformHeapName(i,pSettings,buf,sizeof(buf))));
    BDBG_ERR(("run with msg_modules=nexus_platform_settings for more config info"));
    return BERR_TRACE(rc);
}
#endif

#if NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX
/* resolve GFD and off-screen graphics heaps using BBOX and Nexus heapType */
static NEXUS_Error NEXUS_Platform_P_GetFramebufferHeapIndex(unsigned displayIndex, unsigned *pHeapIndex)
{
    const NEXUS_Core_PreInitState *preInitState;
    unsigned memcIndex;
    NEXUS_Error rc = NEXUS_SUCCESS;

    *pHeapIndex = NEXUS_MAX_HEAPS; /* invalid */
    if (displayIndex == NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE) {
        if (g_NEXUS_platformSettings.heap[NEXUS_MEMC2_SECURE_GRAPHICS_HEAP].size) {
            *pHeapIndex = NEXUS_MEMC2_SECURE_GRAPHICS_HEAP;
            return 0;
        }
        if (g_NEXUS_platformSettings.heap[NEXUS_MEMC1_SECURE_GRAPHICS_HEAP].size) {
            *pHeapIndex = NEXUS_MEMC1_SECURE_GRAPHICS_HEAP;
            return 0;
        }
        if (g_NEXUS_platformSettings.heap[NEXUS_MEMC0_SECURE_GRAPHICS_HEAP].size) {
            *pHeapIndex = NEXUS_MEMC0_SECURE_GRAPHICS_HEAP;
            return 0;
        }
        return NEXUS_NOT_AVAILABLE; /* no BERR_TRACE */
    }

    if (displayIndex == NEXUS_OFFSCREEN_SURFACE || displayIndex == NEXUS_SECONDARY_OFFSCREEN_SURFACE) {
        unsigned i;
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if (!g_NEXUS_platformSettings.heap[i].size) continue;
            if ((g_NEXUS_platformSettings.heap[i].heapType & NEXUS_HEAP_TYPE_GRAPHICS) && (displayIndex == NEXUS_OFFSCREEN_SURFACE)) {
                *pHeapIndex = i;
                return 0;
            }
            else if ((g_NEXUS_platformSettings.heap[i].heapType & NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS) && (displayIndex == NEXUS_SECONDARY_OFFSCREEN_SURFACE)) {
                *pHeapIndex = i;
                return 0;
            }
        }
        return NEXUS_NOT_AVAILABLE; /* no BERR_TRACE */
    }

    if (displayIndex >= BBOX_VDC_DISPLAY_COUNT) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (preInitState->boxMode) {
        memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[displayIndex].aulGfdWinMemcIndex[0];
    }
    else {
        memcIndex = 0;
    }
    switch (memcIndex) {
    case 0:
        if (g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC0_GRAPHICS_HEAP;
            break;
        }
        if (g_pCoreHandles->heap[NEXUS_MEMC0_MAIN_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC0_MAIN_HEAP;
            break;
        }
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        break;
    case 1:
        if (g_pCoreHandles->heap[NEXUS_MEMC1_GRAPHICS_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC1_GRAPHICS_HEAP;
            break;
        }
        if (g_pCoreHandles->heap[NEXUS_MEMC1_DRIVER_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC1_DRIVER_HEAP;
            break;
        }
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        break;
    case 2:
        if (g_pCoreHandles->heap[NEXUS_MEMC2_GRAPHICS_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC2_GRAPHICS_HEAP;
            break;
        }
        if (g_pCoreHandles->heap[NEXUS_MEMC2_DRIVER_HEAP].nexus) {
            *pHeapIndex = NEXUS_MEMC2_DRIVER_HEAP;
            break;
        }
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        break;
    default:
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        break;
    }
    NEXUS_Platform_P_PreUninit();
    return rc;
}
#endif

/*
 * Since nexus platform settings file is common for multiple platforms and
 * each of these platforms have individual nexus_platform_$(NEXUS_PLATFORM) files, this API would call into
 * a private API in the nexus_platform core
 */
NEXUS_HeapHandle NEXUS_Platform_GetFramebufferHeap(unsigned displayIndex)
{
#if NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX
    unsigned index;
    NEXUS_Error rc;
    rc = NEXUS_Platform_P_GetFramebufferHeapIndex(displayIndex, &index);
    if (rc) return NULL;
    BDBG_ASSERT(index < NEXUS_MAX_HEAPS);
    return g_pCoreHandles->heap[index].nexus;
#else
    return NEXUS_Platform_P_GetFramebufferHeap(displayIndex);
#endif
}

void NEXUS_GetPlatformCapabilities_tagged( NEXUS_PlatformCapabilities *pCap, size_t size )
{
    unsigned i;
    const NEXUS_Core_PreInitState *preInitState;

    BSTD_UNUSED(i);
    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) return; /* no BERR_TRACE possible */
    if (
        size != sizeof(*pCap)
#if NEXUS_COMPAT_32ABI
       && size != sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformCapabilities))
#endif
        ) {
#if NEXUS_COMPAT_32ABI
        BDBG_ERR(("NEXUS_GetDefaultMemoryConfigurationSettings - Mixed ABI: size mismatch %u/%u != %u", (unsigned)sizeof(*pCap), (unsigned)sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformCapabilities)), (unsigned)size));
#else
        BDBG_ERR(("NEXUS_GetDefaultMemoryConfigurationSettings: size mismatch %u != %u", (unsigned)sizeof(*pCap), (unsigned)size));
#endif
        BKNI_Memset(pCap, 0, size);
        return;
    }

    BKNI_Memset(pCap, 0, sizeof(*pCap));
    /* TODO: refactor NEXUS_GetDisplayCapabilities and NEXUS_GetVideoEncoderCapabilities to depend on _isrsafe
    functions which take BBOX_Config; then call those from here with preinit state. */
    if (!preInitState->boxMode) {
#if NEXUS_HAS_DISPLAY
        for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
            pCap->display[i].supported = true;
        }
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
        for (i=0;i<NEXUS_NUM_DSP_VIDEO_ENCODERS;i++) {
            pCap->videoEncoder[i].supported = true;
            /* TODO: display index */
        }
#elif BCHP_CHIP == 7425
        /* copied from NEXUS_GetVideoEncoderCapabilities */
        pCap->videoEncoder[0].supported = true;
        pCap->videoEncoder[0].displayIndex = 3;
        pCap->display[3].encoder = true;
        pCap->videoEncoder[1].supported = true;
        pCap->videoEncoder[1].displayIndex = 2;
        pCap->display[2].encoder = true;
#endif
#endif
    }
    else {
#if NEXUS_HAS_DISPLAY
        BDBG_CASSERT(NEXUS_MAX_DISPLAYS <= BBOX_VDC_DISPLAY_COUNT);
        for (i = 0 ; i < NEXUS_MAX_DISPLAYS ; i++) {
            pCap->display[i].supported = preInitState->boxConfig.stVdc.astDisplay[i].bAvailable;
            /* works for VCE and VIP DSP systems */
            pCap->display[i].encoder = preInitState->boxConfig.stVdc.astDisplay[i].stStgEnc.bAvailable;
        }
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
        pCap->videoEncoder[0].supported = true;
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            /* TODO: doesn't work for non-VIP DSP encode platforms */
            if (preInitState->boxConfig.stVdc.astDisplay[i].stStgEnc.bAvailable) {
                pCap->videoEncoder[0].displayIndex = i;
                break;
            }
        }
#else
        for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
            unsigned device = i / NEXUS_NUM_VCE_CHANNELS;
            unsigned channel = i % NEXUS_NUM_VCE_CHANNELS;
            unsigned j;
            for (j=0;j<BBOX_VDC_DISPLAY_COUNT;j++) {
                if (preInitState->boxConfig.stVdc.astDisplay[j].stStgEnc.ulEncoderCoreId == device &&
                    preInitState->boxConfig.stVdc.astDisplay[j].stStgEnc.ulEncoderChannel == channel)
                {
                    pCap->videoEncoder[i].displayIndex = j;
                    BDBG_ASSERT(pCap->display[j].encoder); /* BBOX must be consistent */
                    break;
                }
            }
            if (j == BBOX_VDC_DISPLAY_COUNT) continue;
            if (device < BBOX_VCE_MAX_INSTANCE_COUNT) {
                pCap->videoEncoder[i].supported = preInitState->boxConfig.stVce.stInstance[device].uiChannels & 1 << channel;
            }
        }
#endif
#endif
    }

    NEXUS_Platform_P_PreUninit();
}

NEXUS_Error NEXUS_GetPlatformConfigCapabilities_tagged( const NEXUS_PlatformSettings *pSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig,
    NEXUS_PlatformConfigCapabilities *pCap, unsigned size )
{
    const NEXUS_Core_PreInitState *preInitState;
    struct {
        NEXUS_MemoryRtsSettings rtsSettings;
        NEXUS_MemoryConfigurationSettings defaultMemConfig;
    } *state;
    unsigned internal_size = sizeof(NEXUS_PlatformSettings)+sizeof(NEXUS_MemoryConfigurationSettings)+sizeof(NEXUS_PlatformConfigCapabilities);
    unsigned memcIndex, heapIndex;
#if NEXUS_COMPAT_32ABI
    unsigned compat_internal_size = sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformSettings))+sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_MemoryConfigurationSettings))+sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformConfigCapabilities));
#endif

    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) return -1;
    if (
        size != internal_size
#if NEXUS_COMPAT_32ABI
       && size != compat_internal_size
#endif
        ) {
#if NEXUS_COMPAT_32ABI
        BDBG_ERR(("NEXUS_GetPlatformConfigCapabilities - Mixed ABI: size mismatch %u/%u != %u", internal_size, compat_internal_size, size));
#else
        BDBG_ERR(("NEXUS_GetPlatformConfigCapabilities: size mismatch %u != %u", internal_size, size));
#endif
        BKNI_Memset(pCap, 0, size);
        return -1;
    }
    state = BKNI_Malloc(sizeof(*state));
    if(state==NULL) {return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}

    BSTD_UNUSED(pSettings);
    if (!pMemConfig) {
        NEXUS_GetDefaultMemoryConfigurationSettings(&state->defaultMemConfig);
        pMemConfig = &state->defaultMemConfig;
    }

    NEXUS_P_GetDefaultMemoryRtsSettings(preInitState, &state->rtsSettings);

    BKNI_Memset(pCap, 0, sizeof(*pCap));

    /* secure graphics must be GFD0-accessible */
    memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[0].aulGfdWinMemcIndex[0];
    switch (memcIndex) {
    case 2: heapIndex = NEXUS_MEMC2_SECURE_GRAPHICS_HEAP; break;
    case 1: heapIndex = NEXUS_MEMC1_SECURE_GRAPHICS_HEAP; break;
    case 0: heapIndex = NEXUS_MEMC0_SECURE_GRAPHICS_HEAP; break;
    default: heapIndex = NEXUS_MAX_HEAPS; break;
    }
    /* coverity[dead_error_line: FALSE] */
    if (heapIndex < NEXUS_MAX_HEAPS && nexus_p_has_secure_decoder_on_memc(preInitState, &state->rtsSettings, pMemConfig, memcIndex)) {
        pCap->secureGraphics[0].valid = true;
        pCap->secureGraphics[0].heapIndex = heapIndex;
        pCap->secureGraphics[0].memcIndex = memcIndex;
    }
    BKNI_Free(state);

    NEXUS_Platform_P_PreUninit();
    return NEXUS_SUCCESS;
}
