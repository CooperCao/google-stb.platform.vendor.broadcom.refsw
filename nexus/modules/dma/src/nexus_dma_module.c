/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_dma_module.h"
#include "nexus_dma_init.h"
#include "priv/nexus_dma_standby_priv.h"

#if BCHP_CHIP == 7408
#include "bchp_hif_cpu_intr1.h"
#endif

BDBG_MODULE(nexus_dma);

/* global module handle & data */
NEXUS_DmaModuleData g_NEXUS_dmaModuleData;

void NEXUS_DmaModule_GetDefaultSettings( NEXUS_DmaModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
}

NEXUS_ModuleHandle NEXUS_DmaModule_Init( const NEXUS_DmaModuleSettings *pSettings )
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_DmaModuleSettings defaultSettings;
    BMMD_Settings mmdSettings;
    NEXUS_Error errCode;
    unsigned i = 0;

    BDBG_ASSERT(NULL==g_NEXUS_dmaModuleData.dmaModule && NULL==g_NEXUS_dmaModuleData.channel[0].mmd);

    if (!pSettings) {
        NEXUS_DmaModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    
    /* init global module data */
    BKNI_Memset(&g_NEXUS_dmaModuleData, 0, sizeof(g_NEXUS_dmaModuleData));

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eHigh, &pSettings->common); /* dma interface is fast */
    g_NEXUS_dmaModuleData.dmaModule = NEXUS_Module_Create("dma", &moduleSettings);
    if ( NULL == g_NEXUS_dmaModuleData.dmaModule ) {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    NEXUS_LockModule();

    BMMD_GetDefaultSettings(&mmdSettings);
    mmdSettings.engineType = BMMD_EngineType_eDma;
    for (i=0; (int)i<NEXUS_NUM_DMA_CHANNELS; i++) { /* the cast to int suppresses a benign compiler warning */
        mmdSettings.engineType = BMMD_EngineType_eDma;
        mmdSettings.engineIndex = i;
        errCode = BMMD_Open(g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mem, g_pCoreHandles->bint, &mmdSettings, &g_NEXUS_dmaModuleData.channel[i].mmd);
        if (errCode) {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }
#if NEXUS_NUM_SHARF_DMA_CHANNELS
    for (; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
        mmdSettings.engineType = BMMD_EngineType_eSharf;
        mmdSettings.engineIndex = i-NEXUS_NUM_DMA_CHANNELS;
        errCode = BMMD_Open(g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex], g_pCoreHandles->bint, &mmdSettings, &g_NEXUS_dmaModuleData.channel[i].mmd);
        if (errCode) {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }
#endif

    NEXUS_UnlockModule();
    return g_NEXUS_dmaModuleData.dmaModule;

error:
    NEXUS_UnlockModule();
    NEXUS_DmaModule_Uninit();
    return NULL;
}

/***************************************************************************
Summary:
Shutdown all open channels and handles at module close time
***************************************************************************/
static void NEXUS_Dma_P_Shutdown(void)
{
    int i;
    NEXUS_DmaHandle dma;

    for (i=0; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
        while ( (dma = BLST_S_FIRST(&g_NEXUS_dmaModuleData.channel[i].dmaHandles)) ) {
            BDBG_WRN(("Automatically closing DMA %p", (void *)dma));
            NEXUS_Dma_Close(dma);
        }
    }
    return ;
}

void NEXUS_DmaModule_Uninit(void)
{
    unsigned i;
    BDBG_ASSERT(g_NEXUS_dmaModuleData.dmaModule);
      
    NEXUS_LockModule();
    g_NEXUS_dmaModuleData.shutdown = true;
    NEXUS_Dma_P_Shutdown();
    
    /* 7408 XPT hardware is broken. Need to disable MDMA L1 interrupt before XPT can be unintialised */
#if BCHP_CHIP == 7408
    NEXUS_Core_DisableInterrupt(BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS_XPT_MDMA_CPU_INTR_SHIFT);
    NEXUS_Core_DisconnectInterrupt(BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS_XPT_MDMA_CPU_INTR_SHIFT);
#endif

    for (i=0; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
        if (g_NEXUS_dmaModuleData.channel[i].mmd) {
            BMMD_Close(g_NEXUS_dmaModuleData.channel[i].mmd);
        }
    }
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_dmaModuleData.dmaModule);
    BKNI_Memset(&g_NEXUS_dmaModuleData, 0, sizeof(g_NEXUS_dmaModuleData));

    return;
}

NEXUS_Error NEXUS_DmaModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned i;
    BERR_Code rc;
    BSTD_UNUSED(pSettings);

    if (enabled) {
        for (i=0; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
            if (g_NEXUS_dmaModuleData.channel[i].mmd) {
                rc = BMMD_Standby(g_NEXUS_dmaModuleData.channel[i].mmd, NULL);
                if (rc!=BERR_SUCCESS) {
                    goto error;
                }
            }
        }
    }
    else {
        for (i=0; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
            if (g_NEXUS_dmaModuleData.channel[i].mmd) {
                BMMD_Resume(g_NEXUS_dmaModuleData.channel[i].mmd);
            }
        }
    }

    return NEXUS_SUCCESS;
    
error:
    /* resume all MMD modules */
    for (i=0; i<NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS; i++) {
        if (g_NEXUS_dmaModuleData.channel[i].mmd) {
            BMMD_Resume(g_NEXUS_dmaModuleData.channel[i].mmd);
        }
    }    
    return NEXUS_NOT_SUPPORTED;
#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(enabled);
    return NEXUS_SUCCESS;
#endif
}
