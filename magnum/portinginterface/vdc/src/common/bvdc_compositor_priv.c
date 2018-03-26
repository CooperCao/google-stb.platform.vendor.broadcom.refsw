/******************************************************************************
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
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"
#include "bvdc_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bchp_bmisc.h"
#include "bvdc_pep_priv.h"
#include "bchp_mmisc.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_csc_priv.h"

#define BVDC_P_MAKE_CMP(pCmp, id)                                     \
{                                                                     \
    (pCmp)->ulCoreResetAddr = BCHP_BMISC_SW_INIT;                     \
    (pCmp)->ulCoreResetMask = BCHP_BMISC_SW_INIT_CMP_##id##_MASK;     \
    (pCmp)->ulRegOffset = BCHP_CMP_##id##_REG_START - BCHP_CMP_0_REG_START;     \
}

/* Dither settings for COMP */
#define BVDC_P_DITHER_CMP_LFSR_VALUE                 (0xE0F82)
#define BVDC_P_DITHER_CMP_SCALE_8BIT                 (0x1)
#define BVDC_P_DITHER_CMP_SCALE_10BIT                (0x3)
#define BVDC_P_DITHER_CMP_SCALE_12BIT                (0xC)

BDBG_MODULE(BVDC_CMP);
BDBG_FILE_MODULE(BVDC_CMP_SIZE);
BDBG_FILE_MODULE(BVDC_CMP_CSC);
BDBG_FILE_MODULE(BVDC_REPEATPOLARITY);
BDBG_FILE_MODULE(BVDC_DITHER);
BDBG_OBJECT_ID(BVDC_CMP);

/* SW7250-211/ SW7364-291
 CMP0 RDB HW config register bit CORE_BVB_WIDTH_10 should be 1 instead 0*/
#if (((BCHP_CHIP==7364) && (BCHP_VER <= BCHP_VER_B0)) ||\
     ((BCHP_CHIP==7250) && (BCHP_VER == BCHP_VER_B0)))
#define BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND         (1)
#else
#define BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND         (0)
#endif


/* INDEX: by compositor id. */
static const BVDC_P_Compositor_Features s_aHydraCompositorFeatures
    [BVDC_P_MAX_COMPOSITOR_COUNT] =
{
    /* Compositor 0 (Primary) */
    {BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_0_MAX_WINDOW_COUNT},

    /* Compositor 1 (Secondary) */
    {BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_1_MAX_WINDOW_COUNT},

    /* Compositor Bypass */
    {BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_2_MAX_WINDOW_COUNT},

    /* VICE */
    {BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_3_MAX_WINDOW_COUNT},

    /* VICE */
    {BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_4_MAX_WINDOW_COUNT},

    /* VICE */
    {BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_5_MAX_WINDOW_COUNT},

    /* VICE */
    {BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT,
     BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT,
     BVDC_P_CMP_6_MAX_WINDOW_COUNT}
};

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_GetCfcCapabilities
    ( BREG_Handle                      hRegister,
      BVDC_CompositorId                eCmpId,
      BVDC_WindowId                    eWinId,
      BCFC_Capability                 *pCapability )
{
    uint32_t ulRegOffset = 0;
#if defined(BCHP_CMP_0_HW_CONFIGURATION) && !defined(BVDC_FOR_BOOTUPDATER)
    uint32_t ulHwCfg;
#endif

    BKNI_Memset((void*)pCapability, 0x0, sizeof(pCapability));
    pCapability->stBits.bMc = 1;

    switch(eCmpId)
    {
        case BVDC_CompositorId_eCompositor0:
            ulRegOffset = 0;
            break;
#if (BVDC_P_CMP_1_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor1:
            ulRegOffset = BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
#if (BVDC_P_CMP_2_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor2:
            ulRegOffset = BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
#if (BVDC_P_CMP_3_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor3:
            ulRegOffset = BCHP_CMP_3_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
#if (BVDC_P_CMP_4_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor4:
            ulRegOffset = BCHP_CMP_4_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
#if (BVDC_P_CMP_5_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor5:
            ulRegOffset = BCHP_CMP_5_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
#if (BVDC_P_CMP_6_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor6:
            ulRegOffset = BCHP_CMP_6_REG_START - BCHP_CMP_0_REG_START;
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_CompositorId_eCompositor%d", eCmpId));
            BDBG_ASSERT(0);
            break;
    }

#ifndef BVDC_FOR_BOOTUPDATER

#ifdef BCHP_CMP_0_HW_CONFIGURATION
    ulHwCfg = BREG_Read32(hRegister,
        BCHP_CMP_0_HW_CONFIGURATION + ulRegOffset);

    if(eWinId == BVDC_WindowId_eVideo0) {
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V0_Ma_CSC_Present_SHIFT
        pCapability->stBits.bMa = BVDC_P_GET_FIELD(
            ulHwCfg, CMP_0_HW_CONFIGURATION, V0_Ma_CSC_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V0_NL_LUT_Present_SHIFT
        pCapability->stBits.bNL2L = pCapability->stBits.bL2NL =
        BVDC_P_GET_FIELD(ulHwCfg, CMP_0_HW_CONFIGURATION, V0_NL_LUT_Present);
#endif
    } else if(eWinId == BVDC_WindowId_eVideo1) {
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V1_Ma_CSC_Present_SHIFT
        pCapability->stBits.bMa = BVDC_P_GET_FIELD(
            ulHwCfg, CMP_0_HW_CONFIGURATION, V1_Ma_CSC_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V1_NL_LUT_Present_SHIFT
        pCapability->stBits.bNL2L = pCapability->stBits.bL2NL =
            BVDC_P_GET_FIELD(ulHwCfg, CMP_0_HW_CONFIGURATION, V1_NL_LUT_Present);
#endif
    }

/* 7271 b0 and newer uses BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG
 * 7271 a0 uses BCHP_CMP_0_HW_CONFIGURATION as above */
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG
    if(BVDC_CompositorId_eCompositor0 == eCmpId)
    {
#ifdef BCHP_HDR_CMP_0_HDR_V1_HW_CONFIG
        ulHwCfg = BREG_Read32(hRegister,
            ((eWinId == BVDC_WindowId_eVideo0)? BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG : BCHP_HDR_CMP_0_HDR_V1_HW_CONFIG) + ulRegOffset);
#else
        ulHwCfg = BREG_Read32(hRegister,
            BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG + ulRegOffset);
#endif

#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_CSC_MA_PRESENT_SHIFT
        pCapability->stBits.bMa = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_CSC_MA_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_CSC_MB_PRESENT_SHIFT
        pCapability->stBits.bMb = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_CSC_MB_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_NL_LUT_PRESENT_SHIFT
        pCapability->stBits.bNL2L = pCapability->stBits.bL2NL = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_NL_LUT_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_LRNG_ADJ_PRESENT_SHIFT
        pCapability->stBits.bLRngAdj = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_LRNG_ADJ_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_LMR_PRESENT_SHIFT
        pCapability->stBits.bLMR = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_LMR_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_TP_PRESENT_SHIFT
        pCapability->stBits.bTpToneMapping = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_TP_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_DLBV_CVM_PRESENT_SHIFT
        pCapability->stBits.bDbvToneMapping = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_DLBV_CVM_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CFC_DLBV_COMP_PRESENT_SHIFT
        pCapability->stBits.bDbvCmp = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CFC_DLBV_COMP_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_ALPHA_DIV_PRESENT_SHIFT
        pCapability->stBits.bAlphaDiv = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, ALPHA_DIV_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CSC_BLD_IN_PRESENT_SHIFT
        pCapability->stBits.bCscBlendIn = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CSC_BLD_IN_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_CSC_BLD_OUT_PRESENT_SHIFT
        pCapability->stBits.bCscBlendOut = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, CSC_BLD_OUT_PRESENT);
#endif
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG_LUT_SCB_PRESENT_SHIFT
        pCapability->stBits.bRamLutScb = BVDC_P_GET_FIELD(
            ulHwCfg, HDR_CMP_0_HDR_V0_HW_CONFIG, LUT_SCB_PRESENT);
#endif
        pCapability->stBits.bRamNL2L = pCapability->stBits.bNL2L;
        pCapability->stBits.bRamL2NL = pCapability->stBits.bL2NL;
    }
#endif /* #ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG */

/* 7271 A */
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
    /* TODO: the following info in HW_CONFIGURATION is not right */
    if (eCmpId == BVDC_CompositorId_eCompositor0)
    {
        pCapability->stBits.bMb = 1;
        pCapability->stBits.bLRngAdj = 1;
        pCapability->stBits.bNL2L = pCapability->stBits.bL2NL = 1;
        if(eWinId == BVDC_WindowId_eVideo0) {
            pCapability->stBits.bRamNL2L = pCapability->stBits.bRamL2NL = 1;
        }
    }
#endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */

#if (BCHP_CHIP==7439) && (BCHP_VER <= BCHP_VER_B1)
    /* TODO: the following info in HW_CONFIGURATION is not right, create HW jira */
    pCapability->stBits.bNL2L = pCapability->stBits.bL2NL = 1;
    if (eCmpId == BVDC_CompositorId_eCompositor0)
    {
        pCapability->stBits.bMa = 1;
    }
#endif

#endif /* #ifdef BCHP_CMP_0_HW_CONFIGURATION */

#if (!defined(BCHP_CMP_0_HW_CONFIGURATION_V0_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V0_NL_LUT_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_NL_LUT_Present_SHIFT))
    BSTD_UNUSED(ulHwCfg);
#endif

#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    return;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Compositor_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Compositor_Handle          *phCompositor,
      BVDC_CompositorId                eCompositorId )
{
    uint32_t i;
    BVDC_P_CompositorContext *pCompositor;
    BERR_Code eStatus = BERR_SUCCESS;
#if defined(BCHP_CMP_0_HW_CONFIGURATION) && !defined(BVDC_FOR_BOOTUPDATER)
    uint32_t ulHwCfg;
#endif

    BDBG_ENTER(BVDC_P_Compositor_Create);
    BDBG_ASSERT(phCompositor);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* Ascertain that video window count per display correctly mirrors BOX's */
    BDBG_CASSERT(BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY == BVDC_P_MAX_VIDEO_WINS_PER_CMP);
    /* Ascertain that VDC window count complies to BOX's */
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_0_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_1_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_2_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_3_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_4_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_5_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BBOX_VDC_WINDOW_COUNT_PER_DISPLAY >= BVDC_P_CMP_6_MAX_WINDOW_COUNT);

    /* (1) Alloc the context. */
    pCompositor = (BVDC_P_CompositorContext*)
        (BKNI_Malloc(sizeof(BVDC_P_CompositorContext)));
    if(!pCompositor)
    {
        eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
        goto BVDC_P_Compositor_Create_Done;
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pCompositor, 0x0, sizeof(BVDC_P_CompositorContext));
    BDBG_OBJECT_SET(pCompositor, BVDC_CMP);

    /* Initialize non-changing states.  These are not to be changed by runtime. */
    pCompositor->eId          = eCompositorId;
    pCompositor->pFeatures    = &s_aHydraCompositorFeatures[eCompositorId];
    pCompositor->hVdc         = hVdc;
    pCompositor->bIsBypass    = (
        (hVdc->pFeatures->bCmpBIsBypass) &&
        (BVDC_CompositorId_eCompositor2 == pCompositor->eId));

    switch(pCompositor->eId)
    {
        case BVDC_CompositorId_eCompositor0:
            BVDC_P_MAKE_CMP(pCompositor, 0);
            break;
#if (BVDC_P_CMP_1_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor1:
            BVDC_P_MAKE_CMP(pCompositor, 1);
            break;
#endif
#if (BVDC_P_CMP_2_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor2:
            BVDC_P_MAKE_CMP(pCompositor, 2);
            break;
#endif
#if (BVDC_P_CMP_3_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor3:
            BVDC_P_MAKE_CMP(pCompositor, 3);
            break;
#endif
#if (BVDC_P_CMP_4_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor4:
            BVDC_P_MAKE_CMP(pCompositor, 4);
            break;
#endif
#if (BVDC_P_CMP_5_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor5:
            BVDC_P_MAKE_CMP(pCompositor, 5);
            break;
#endif
#if (BVDC_P_CMP_6_MAX_WINDOW_COUNT)
        case BVDC_CompositorId_eCompositor6:
            BVDC_P_MAKE_CMP(pCompositor, 6);
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_CompositorId_eCompositor%d", pCompositor->eId));
            BDBG_ASSERT(0);
            break;
    }

    BVDC_P_Compositor_GetCfcCapabilities(hVdc->hRegister, eCompositorId, BVDC_WindowId_eVideo0, &pCompositor->stCfcCapability[0]);
    BVDC_P_Compositor_GetCfcCapabilities(hVdc->hRegister, eCompositorId, BVDC_WindowId_eVideo1, &pCompositor->stCfcCapability[1]);

#ifndef BVDC_FOR_BOOTUPDATER

#ifdef BCHP_CMP_0_HW_CONFIGURATION
    ulHwCfg = BREG_Read32(hVdc->hRegister,
        BCHP_CMP_0_HW_CONFIGURATION + pCompositor->ulRegOffset);
#ifdef BCHP_CMP_0_HW_CONFIGURATION_CORE_BVB_WIDTH_10_SHIFT
    pCompositor->bIs10BitCore = BVDC_P_GET_FIELD(
        ulHwCfg, CMP_0_HW_CONFIGURATION, CORE_BVB_WIDTH_10);
#if BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND
    if(BVDC_CompositorId_eCompositor0 == pCompositor->eId)
        pCompositor->bIs10BitCore = true;
#endif
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_PROC_CLK_IS_2X_SHIFT
    pCompositor->bIs2xClk = BVDC_P_GET_FIELD(
        ulHwCfg, CMP_0_HW_CONFIGURATION, PROC_CLK_IS_2X);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_VIN_DITHER_Present_SHIFT
    pCompositor->bInDither = BVDC_P_GET_FIELD(
        ulHwCfg, CMP_0_HW_CONFIGURATION, VIN_DITHER_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_CMP_OUT_BPC_SHIFT
    pCompositor->bAlign12Bit = BVDC_P_GET_FIELD(
        ulHwCfg, CMP_0_HW_CONFIGURATION, CMP_OUT_BPC);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_HDR_Present_SHIFT
    pCompositor->bHdr = BVDC_P_GET_FIELD(
        ulHwCfg, CMP_0_HW_CONFIGURATION, HDR_Present);
#endif

/* 7271 b0 and newer uses BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG
 * 7271 a0 uses BCHP_CMP_0_HW_CONFIGURATION as above */
#ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG
    if(BVDC_CompositorId_eCompositor0 == pCompositor->eId)
    {
#if BVDC_P_DBV_SUPPORT
        if(pCompositor->stCfcCapability[0].stBits.bDbvToneMapping) {
            eStatus = BVDC_P_Compositor_DbvInit(pCompositor);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Compositor_Create_Done;
            }
        }
#endif
#if BVDC_P_TCH_SUPPORT
        if(pCompositor->stCfcCapability[0].stBits.bTpToneMapping) {
            eStatus = BVDC_P_Compositor_TchInit(pCompositor);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Compositor_Create_Done;
            }
        }
#endif
    }
#endif /* #ifdef BCHP_HDR_CMP_0_HDR_V0_HW_CONFIG */

#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

#if (!defined(BCHP_CMP_0_HW_CONFIGURATION_V0_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V0_NL_LUT_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_NL_LUT_Present_SHIFT))
    BSTD_UNUSED(ulHwCfg);
#endif

#endif

    /* all windows should have been disconnected. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        pCompositor->ahWindow[i] = NULL;
    }

    /* (2) Create RDC List */
    for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
    {
        eStatus = BRDC_List_Create(hVdc->hRdc, BVDC_P_MAX_ENTRY_PER_RUL,
            &pCompositor->ahList[i]);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Compositor_Create_Done;
        }
    }

    /* (3) Create RDC Slot */
    eStatus = BRDC_Slots_Create(hVdc->hRdc, pCompositor->ahSlot, BVDC_P_CMP_MAX_SLOT_COUNT);
    if(BERR_SUCCESS != eStatus)
    {
        goto BVDC_P_Compositor_Create_Done;
    }

    for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
    {
        BRDC_SlotId eSlotId;
        BRDC_Slot_Settings  stSlotSettings;

        BRDC_Slot_UpdateLastRulStatus_isr(pCompositor->ahSlot[i], pCompositor->ahList[i], true);
        eStatus = BRDC_Slot_GetId(pCompositor->ahSlot[i], &eSlotId);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Compositor_Create_Done;
        }

        /* Enable RDC priority */
        if(pCompositor->eId == BVDC_CompositorId_eCompositor0)
        {
            BRDC_Slot_GetConfiguration(pCompositor->ahSlot[i], &stSlotSettings);
            stSlotSettings.bHighPriority = true;
            eStatus = BRDC_Slot_SetConfiguration(pCompositor->ahSlot[i], &stSlotSettings);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Compositor_Create_Done;
            }
        }

        BDBG_MSG(("Compositor[%d] uses slot[%d]", pCompositor->eId, eSlotId));
    }

    /* (4) Create Slot interrupt callback */
    for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
    {
        eStatus = BINT_CreateCallback(&pCompositor->ahCallback[i], hVdc->hInterrupt,
            BRDC_Slot_GetIntId(pCompositor->ahSlot[i]),
            BVDC_P_CompositorDisplay_isr, (void*)pCompositor, i);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Compositor_Create_Done;
        }
    }

    /* (5) Added this compositor to hVdc */
    hVdc->ahCompositor[eCompositorId] = (BVDC_Compositor_Handle)pCompositor;

    /* All done. now return the new fresh context to user. */
    *phCompositor = (BVDC_Compositor_Handle)pCompositor;

BVDC_P_Compositor_Create_Done:
    BDBG_LEAVE(BVDC_P_Compositor_Create);

    if((BERR_SUCCESS != eStatus) && (NULL != pCompositor))
    {
        for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
        {
            if(NULL != pCompositor->ahCallback[i])
                BINT_DestroyCallback(pCompositor->ahCallback[i]);
        }
        for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
        {
            if(NULL != pCompositor->ahSlot[i])
                BRDC_Slot_Destroy(pCompositor->ahSlot[i]);
        }
        for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
        {
            if(NULL != pCompositor->ahList[i])
                BRDC_List_Destroy(pCompositor->ahList[i]);
        }

        BDBG_OBJECT_DESTROY(pCompositor, BVDC_CMP);
        BKNI_Free((void*)pCompositor);
    }

    return BERR_TRACE(eStatus);
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_Destroy
    ( BVDC_Compositor_Handle           hCompositor )
{
    uint32_t i;

    BDBG_ENTER(BVDC_P_Compositor_Destroy);
    if(!hCompositor)
    {
        BDBG_LEAVE(BVDC_P_Compositor_Destroy);
        return;
    }

    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* At this point application should have disable all the
     * callbacks &slots */
#if BVDC_P_CMP_CFC_VER >= 3
    if(hCompositor->stCfcLutList.hMmaBlock[0]) {
        for(i = 0; i < BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT; i++) {
            BMMA_Unlock(hCompositor->stCfcLutList.hMmaBlock[i], hCompositor->stCfcLutList.pulStart[i]);
            BMMA_UnlockOffset(hCompositor->stCfcLutList.hMmaBlock[i], hCompositor->stCfcLutList.ullStartDeviceAddr[i]);
            BMMA_Free(hCompositor->stCfcLutList.hMmaBlock[i]);
            hCompositor->stCfcLutList.hMmaBlock[i] = NULL;
        }
        hCompositor->hCfcHeap  = NULL;
    }
#endif

    /* [5] Removed this compositor from hVdc */
    hCompositor->hVdc->ahCompositor[hCompositor->eId] = NULL;

    /* Child windows for this compositor. */
    if(BVDC_CompositorId_eCompositor0 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V1]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G1]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G2]);
    }
    else if(BVDC_CompositorId_eCompositor1 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_V1]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_G0]);
    }
    else if(BVDC_CompositorId_eCompositor2 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_V0]);
        if(!hCompositor->hVdc->pFeatures->bCmpBIsBypass)
        {
            BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_G0]);
        }
    }
    else if(BVDC_CompositorId_eCompositor3 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp3_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp3_G0]);
    }
    else if(BVDC_CompositorId_eCompositor4 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp4_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp4_G0]);
    }
    else if(BVDC_CompositorId_eCompositor5 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp5_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp5_G0]);
    }
    else if(BVDC_CompositorId_eCompositor6 == hCompositor->eId)
    {
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp6_V0]);
        BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp6_G0]);
    }

    /* [4] Create Slot interrupt callback */
    for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
    {
        BINT_DestroyCallback(hCompositor->ahCallback[i]);
    }

    /* [3] Destroy RDC Slot */
    BRDC_Slots_Destroy(hCompositor->ahSlot, BVDC_P_CMP_MAX_SLOT_COUNT);

    /* [2] Destroy RDC List */
    for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
    {
        BRDC_List_Destroy(hCompositor->ahList[i]);
    }

#if BVDC_P_DBV_SUPPORT
    if(hCompositor->stCfcCapability[0].stBits.bDbvToneMapping) {
        BVDC_P_Compositor_DbvUninit(hCompositor);
    }
#endif
#if BVDC_P_TCH_SUPPORT
    if(hCompositor->stCfcCapability[0].stBits.bTpToneMapping) {
        BVDC_P_Compositor_TchUninit(hCompositor);
    }
#endif
    BDBG_OBJECT_DESTROY(hCompositor, BVDC_CMP);
    /* [1] Release context in system memory */
    BKNI_Free((void*)hCompositor);

    BDBG_LEAVE(BVDC_P_Compositor_Destroy);
    return;
}

/***************************************************************************
 * {private}
 *
 * By application calling BVDC_Compositor_Create().
 */
void BVDC_P_Compositor_Init
    ( BVDC_Compositor_Handle           hCompositor )
{
    uint32_t i;
    BVDC_P_Compositor_Info *pNewInfo;
    BVDC_P_Compositor_Info *pCurInfo;

    BDBG_ENTER(BVDC_P_Compositor_Init);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* Init to default state. */
    hCompositor->bInitial            = true;
    hCompositor->eState              = BVDC_P_State_eInactive;
    hCompositor->ulActiveVideoWindow = 0;
    hCompositor->ulActiveGfxWindow   = 0;
    hCompositor->bUserAppliedChanges = false;
    hCompositor->hSyncLockWin        = NULL;
    hCompositor->hSyncLockSrc        = NULL;
    hCompositor->hForceTrigPipSrc    = NULL;
    hCompositor->ulSlip2Lock         = 0;


    /* used for NRT mode transcode: default true to freeze STC and not get encoded */
    hCompositor->bIgnorePicture = true;
    hCompositor->bStallStc      = true;
    hCompositor->bMute          = true;
    /* used for xcode channel change */
    hCompositor->bChannelChange = true;
    hCompositor->bGfxChannelChange = true;


    /* reinitialized ruls. */
    for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
    {
        BRDC_List_SetNumEntries_isr(hCompositor->ahList[i], 0);
        BVDC_P_BuildNoOpsRul_isr(hCompositor->ahList[i]);
    }

    /* Assign fresh new no-op list. */
    for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
    {
        /* Initialized rul indexing. */
        hCompositor->aulRulIdx[i] = 0;
    }
    BRDC_Slots_SetList_isr(hCompositor->ahSlot, hCompositor->ahList[0], BVDC_P_CMP_MAX_SLOT_COUNT);

    /* Compositor's output framerate from selected master window */
    hCompositor->eSrcFRateCode = hCompositor->hVdc->stSettings.eDisplayFrameRate;
    hCompositor->bFullRate     = BVDC_P_IS_FULL_FRAMRATE(hCompositor->eSrcFRateCode);

    /* Clear out shadow registers. */
    BKNI_Memset((void*)hCompositor->aulRegs, 0x0, sizeof(hCompositor->aulRegs));

    /* Default compositor settings. */
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) &= ~(
        BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE) |
        BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE_CTRL));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE, ENABLE) |
        BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE_CTRL, STOP_ON_FIELD_COMPLETION));

    /* Default blender settings, no blending */
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_0_CTRL) &= ~(
        BCHP_MASK(CMP_0_BLEND_0_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_0_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_1_CTRL) &= ~(
        BCHP_MASK(CMP_0_BLEND_1_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_1_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_BLEND_1_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 2
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_2_CTRL) &= ~(
        BCHP_MASK(CMP_0_BLEND_2_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_2_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_BLEND_2_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 3
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_3_CTRL) &= ~(
        BCHP_MASK(CMP_0_BLEND_3_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_3_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_BLEND_3_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 4
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_4_CTRL) &= ~(
        BCHP_MASK(CMP_0_BLEND_4_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_4_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_BLEND_4_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));
#endif
#endif
#endif

    /* PR 11368: Configuring compositors for multi-tap filtering, for
     * eliminating distortion when using all-pass bandwidth filters. */
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_CTRL, FILT_CTRL));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_V0_SURFACE_CTRL, FILT_CTRL, TEN_TAPS_FILTERING));

    BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_CTRL, DERING_EN));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_V0_SURFACE_CTRL, DERING_EN, ENABLE));

#if BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V1_SURFACE_CTRL, FILT_CTRL));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_V1_SURFACE_CTRL, FILT_CTRL, TEN_TAPS_FILTERING));

    BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V1_SURFACE_CTRL, DERING_EN));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_V1_SURFACE_CTRL, DERING_EN, ENABLE));

#endif  /* BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1 */

    /* disable range clipping */
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_OUT_CTRL) &= ~(
        BCHP_MASK(CMP_0_CMP_OUT_CTRL, OUT_TO_DS_CTRL) |
        BCHP_MASK(CMP_0_CMP_OUT_CTRL, OUT_TO_VEC_CTRL) |
        BCHP_MASK(CMP_0_CMP_OUT_CTRL, CLIP_CTRL));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_OUT_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, OUT_TO_DS_CTRL, DISABLE) |
        BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, OUT_TO_VEC_CTRL, ENABLE) |
        BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, CLIP_CTRL, DISABLE));

    BVDC_P_CMP_GET_REG_DATA(CMP_0_CRC_CTRL) &= ~(
        BCHP_MASK(CMP_0_CRC_CTRL, INIT_VALUE) |
        BCHP_MASK(CMP_0_CRC_CTRL, PROBE_RATE) |
        BCHP_MASK(CMP_0_CRC_CTRL, CLEAR) |
        BCHP_MASK(CMP_0_CRC_CTRL, ENABLE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CRC_CTRL) |=  (
        BCHP_FIELD_DATA(CMP_0_CRC_CTRL, INIT_VALUE, 0) |
        BCHP_FIELD_ENUM(CMP_0_CRC_CTRL, PROBE_RATE, ONE_PICTURE_PERIOD) |
        BCHP_FIELD_ENUM(CMP_0_CRC_CTRL, CLEAR,      CLEAR) |
        BCHP_FIELD_DATA(CMP_0_CRC_CTRL, ENABLE,     1));

    /* Initial new/current public states */
    pNewInfo = &hCompositor->stNewInfo;
    pCurInfo = &hCompositor->stCurInfo;

    /* Clear out user's states. */
    BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Compositor_Info));
    BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Compositor_Info));
    BDBG_CASSERT(sizeof(pNewInfo->stDirty.stBits) <= sizeof(pNewInfo->stDirty.aulInts));

    /* default values. */
    pNewInfo->pFmtInfo = BFMT_GetVideoFormatInfoPtr(
        hCompositor->hVdc->stSettings.eVideoFormat);

    pNewInfo->ulBgColorYCrCb = BVDC_P_YCRCB_BLACK;

    pNewInfo->stColorClipSettings.ulCrYSlopeA = 0;
    pNewInfo->stColorClipSettings.ulCrYSlopeB = 0;
    pNewInfo->stColorClipSettings.ulCbYSlopeA = 0;
    pNewInfo->stColorClipSettings.ulCbYSlopeB = 0;
    pNewInfo->stColorClipSettings.ulCrJoint = 0;
    pNewInfo->stColorClipSettings.ulCbJoint = 0;
    pNewInfo->stColorClipSettings.eColorClipMode = BVDC_ColorClipMode_None;

    /* Make sure it a non-null pointer for window creation. */
    pCurInfo->pFmtInfo = BFMT_GetVideoFormatInfoPtr(
        hCompositor->hVdc->stSettings.eVideoFormat);

    BDBG_LEAVE(BVDC_P_Compositor_Init);
    return;
}


/* Miscellaneous functions. */
/***************************************************************************
 * {private}
 *
 * Determined if new user settings are valid.  It validates all new
 * settings of windows within this compositor.
 */
static BERR_Code BVDC_P_Compositor_Validate
    ( const BVDC_Compositor_Handle     hCompositor )
{
    uint32_t i, j, k, ulPhaseAdjCount, ulMasterCount, ulGameModeCount;
    uint32_t aulBlenderZOrder[BVDC_P_CMP_MAX_BLENDER];
    /*BVDC_P_WindowId aeBlenderWinId[BVDC_P_CMP_MAX_BLENDER];*/
    uint32_t ulVidWinCnt = 0;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Defer validations until display handle is created. */
    if(!hCompositor->hDisplay)
    {
        return BERR_SUCCESS;
    }

    /* Get the new validated format from BVDC_P_Display! */
    hCompositor->stNewInfo.pFmtInfo = hCompositor->hDisplay->stNewInfo.pFmtInfo;

    /* keep track of which blender get used */
    for(i = 0; i < BVDC_P_CMP_MAX_BLENDER; i++)
    {
        hCompositor->abBlenderUsed[i] = false;
        aulBlenderZOrder[i] = 0;
        hCompositor->aeBlenderWinId[i] = BVDC_P_WindowId_eUnknown;
    }
    ulPhaseAdjCount = 0;
    ulMasterCount = 0;
    ulGameModeCount = 0;

    /* Make sure all the changes to these windows are valid. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]))
        {
            uint8_t ucZOrder = hCompositor->ahWindow[i]->stNewInfo.ucZOrder;

            for(j=0; j<BVDC_P_CMP_MAX_BLENDER-1;j++)
            {
                if(hCompositor->abBlenderUsed[j]==false) break;

                if(ucZOrder < aulBlenderZOrder[j])
                {
                    /* shift everything down by 1 */
                    for(k=BVDC_P_CMP_MAX_BLENDER-2; k>=j; k--)
                    {
                        if(hCompositor->abBlenderUsed[k])
                        {
                            aulBlenderZOrder[k+1] = aulBlenderZOrder[k];
                            hCompositor->abBlenderUsed[k+1] = true;
                            hCompositor->aeBlenderWinId[k+1] = hCompositor->aeBlenderWinId[k];
                            hCompositor->ahWindow[hCompositor->aeBlenderWinId[k+1]]->ulBlenderId = k+1;
                        }

                        if(k==j) break;
                    }
                    break;
                }
            }

            hCompositor->abBlenderUsed[j] = true;
            aulBlenderZOrder[j] = ucZOrder;
            hCompositor->aeBlenderWinId[j] = hCompositor->ahWindow[i]->eId;
            hCompositor->ahWindow[hCompositor->aeBlenderWinId[j]]->ulBlenderId = j;

            /* Window validation is where the bulk of the work going to be. */
            eStatus = BVDC_P_Window_ValidateChanges(hCompositor->ahWindow[i],
                hCompositor->stNewInfo.pFmtInfo);
            if(BERR_SUCCESS != eStatus)
            {
                return BERR_TRACE(eStatus);
            }

            if(BVDC_P_WIN_IS_VIDEO_WINDOW(hCompositor->ahWindow[i]->eId))
            {
                ulVidWinCnt++;
            }

            if(hCompositor->ahWindow[i]->stNewInfo.bUseSrcFrameRate)
            {
                ulMasterCount++;
            }

            if(hCompositor->ahWindow[i]->stNewInfo.stGameDelaySetting.bEnable)
            {
                ulGameModeCount++;
            }
        }
    }

    /* Check number of video windows in a given compositor.  */
    if(ulVidWinCnt > BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT)
    {
        return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
    }

    /* Check phase adjustment mode settings.  Only one analog source
     * can be adjust with a given compositor.  */
    if(ulPhaseAdjCount > 1)
    {
        return BERR_TRACE(BVDC_ERR_MULTI_PHASE_ADJUST_VALUES);
    }

    /* Check master frame rate settings.  Only one video source
     * can be master of a given compositor.  */
    if(ulMasterCount > 1)
    {
        BDBG_ERR(("CMP[%d] Can not enable multiple master framerate tracking windows",
            hCompositor->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_FRAMERATE_USE);
    }

    /* Check game mode settings.  Only one video source
     * can be in game mode of a given compositor.  */
    if(ulGameModeCount > 1)
    {
        return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
    }

    return eStatus;
}


/***************************************************************************
 * {private}
 *
 * Determined if new user settings are valid.  It validates all new
 * settings of windows within this compositor.
 */
BERR_Code BVDC_P_Compositor_ValidateChanges
    ( const BVDC_Compositor_Handle     ahCompositor[] )
{
    BERR_Code eStatus;
    int i;

    BDBG_ENTER(BVDC_P_Compositor_ValidateChanges);


    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(ahCompositor[i]) ||
           BVDC_P_STATE_IS_CREATE(ahCompositor[i]) ||
           BVDC_P_STATE_IS_DESTROY(ahCompositor[i]))
        {
            eStatus = BVDC_P_Compositor_Validate(ahCompositor[i]);
            if(BERR_SUCCESS != eStatus)
            {
                return BERR_TRACE(eStatus);
            }
        }
    }

    BDBG_LEAVE(BVDC_P_Compositor_ValidateChanges);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 * All new window/compositor states are good.  Now we only need to apply
 * those changes when building the next RUL.
 */
void BVDC_P_Compositor_ApplyChanges_isr
    ( BVDC_Compositor_Handle           hCompositor )
{
    BVDC_P_Compositor_Info *pNewInfo;
    BVDC_P_Compositor_Info *pCurInfo;
    BVDC_P_Compositor_DirtyBits *pNewDirty;
    BVDC_P_Compositor_DirtyBits *pCurDirty;
    int i;

    BDBG_ENTER(BVDC_P_Compositor_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* To reduce the amount of typing */
    pNewInfo = &hCompositor->stNewInfo;
    pCurInfo = &hCompositor->stCurInfo;

    pNewDirty = &pNewInfo->stDirty;
    pCurDirty = &pCurInfo->stDirty;

    /* State transition for display/compositor. */
    if(BVDC_P_STATE_IS_CREATE(hCompositor))
    {
        BDBG_MSG(("Compositor%d activated.", hCompositor->eId));
        hCompositor->eState = BVDC_P_State_eActive;
        BVDC_P_SET_ALL_DIRTY(pNewDirty);

        /* Check to make sure if this compositor need to handle checking for
         * source lost. */
        if(!hCompositor->hVdc->hCmpCheckSource)
        {
            hCompositor->hVdc->hCmpCheckSource = hCompositor;
        }
    }
    else if(BVDC_P_STATE_IS_DESTROY(hCompositor))
    {
        BDBG_MSG(("Compositor%d de-activated", hCompositor->eId));
        hCompositor->eState = BVDC_P_State_eInactive;

        /* Check to make sure if this compositor need to delegate handle
         * checking for source lost to another compositor. */
        if(hCompositor == hCompositor->hVdc->hCmpCheckSource)
        {
            hCompositor->hVdc->hCmpCheckSource = NULL;
            for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
            {
                if(BVDC_P_STATE_IS_ACTIVE(hCompositor->hVdc->ahCompositor[i]) &&
                   hCompositor->hVdc->ahCompositor[i]->hDisplay &&
                   BVDC_P_STATE_IS_ACTIVE(hCompositor->hVdc->ahCompositor[i]->hDisplay))
                {
                    hCompositor->hVdc->hCmpCheckSource = hCompositor->hVdc->ahCompositor[i];
                }
            }
        }
    }
    else if(BVDC_P_STATE_IS_INACTIVE(hCompositor))
    {
        goto done;
    }

    /* Compositor is always re-enable every vsync. */
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) &= ~(
        BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) |=  (
        BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE, ENABLE));

    /* Applychanges for each windows. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_DESTROY(hCompositor->ahWindow[i]))
        {
            BVDC_P_Window_ApplyChanges_isr(hCompositor->ahWindow[i]);
        }
    }
    hCompositor->bDspAspRatDirty = false;

    /* defer compositor size -- number of lines. */
    /* set bgcolor */
    BVDC_P_CMP_SET_REG_DATA(CMP_0_BG_COLOR, pNewInfo->ulBgColorYCrCb);

    /* Any changes in compositor property. */
    if((BVDC_P_IS_DIRTY(pNewDirty)) ||
       (pCurInfo->pFmtInfo != pNewInfo->pFmtInfo) ||
       (pCurInfo->eOrientation != pNewInfo->eOrientation) ||
       (pCurInfo->ulBgColorYCrCb != pNewInfo->ulBgColorYCrCb))
    {
        /* Must be careful here not to globble current dirty bits set by compositor,
         * but rather OR them together. */
        BVDC_P_OR_ALL_DIRTY(pNewDirty, pCurDirty);

        /* Update current info here to let windows wr/rd sides see consistent info. */
        hCompositor->stCurInfo = hCompositor->stNewInfo ;

        /* Clear dirty bits since they're already OR'ed into current. */
        if ((NULL!=hCompositor->hDisplay) &&
            (BVDC_P_DISPLAY_USED_STG(hCompositor->hDisplay->eMasterTg)))
        {
            if(BVDC_P_DISPLAY_NODELAY(hCompositor->hDisplay->pStgFmtInfo, hCompositor->hDisplay->stCurInfo.pFmtInfo))
            {
                BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
                hCompositor->bUserAppliedChanges = true;
            }
        }
        else
        {
            BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
            hCompositor->bUserAppliedChanges = true;
        }
    }

done:
    BDBG_LEAVE(BVDC_P_Compositor_ApplyChanges_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_AbortChanges
    ( BVDC_Compositor_Handle           hCompositor )
{
    int i;

    BDBG_ENTER(BVDC_P_Compositor_AbortChanges);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Cancel the setting user set to the new state. */
    hCompositor->stNewInfo = hCompositor->stCurInfo;

    /* Applychanges for each windows. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_DESTROY(hCompositor->ahWindow[i]))
        {
            BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
            BVDC_P_Window_AbortChanges(hCompositor->ahWindow[i]);
        }
    }

    BDBG_LEAVE(BVDC_P_Compositor_AbortChanges);
    return;
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 * Configure dithering inside a compositor.
 */
static void BVDC_P_Window_BuildDitherRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList)
{
    BVDC_Compositor_Handle hCompositor;
    BVDC_WindowId eWinInCmp;

    BDBG_ENTER(BVDC_P_Window_BuildDitherRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hCompositor = hWindow->hCompositor;
    eWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hCompositor);

#ifdef BCHP_CMP_0_V0_IN_DITHER_422_CTRL
    /* Identify 8-bit compostior with Input Dither HW */
    if(!hCompositor->bIs10BitCore && hCompositor->bInDither &&
       hCompositor->pFeatures->ulMaxVideoWindow > eWinInCmp)
    {
#ifdef BCHP_CMP_0_V1_IN_DITHER_422_CTRL
        uint32_t ulDitherCtrlRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_IN_DITHER_422_CTRL : BCHP_CMP_0_V0_IN_DITHER_422_CTRL;
        uint32_t ulLfsrInitRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_IN_DITHER_LFSR_INIT : BCHP_CMP_0_V0_IN_DITHER_LFSR_INIT;
        uint32_t ulLfsrCtrlRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_IN_DITHER_LFSR_CTRL : BCHP_CMP_0_V0_IN_DITHER_LFSR_CTRL;
#else

        uint32_t ulDitherCtrlRegAddr = BCHP_CMP_0_V0_IN_DITHER_422_CTRL;
        uint32_t ulLfsrInitRegAddr = BCHP_CMP_0_V0_IN_DITHER_LFSR_INIT;
        uint32_t ulLfsrCtrlRegAddr = BCHP_CMP_0_V0_IN_DITHER_LFSR_CTRL;
#endif

        BDBG_MODULE_MSG(BVDC_DITHER,("CMP_%d_V%d IN_DITHER: %s",
            hCompositor->eId, eWinInCmp,
            (hCompositor->bInDitherEnable[hWindow->eId]) ? "ENABLE" : "DISABLE"));

        BVDC_P_Dither_Setting_isr(&hCompositor->stInDither,
            hCompositor->bInDitherEnable[hWindow->eId],
            BVDC_P_DITHER_CMP_LFSR_VALUE, BVDC_P_DITHER_CMP_SCALE_8BIT);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulDitherCtrlRegAddr + hCompositor->ulRegOffset);
        *pList->pulCurrent++ = hCompositor->stInDither.ulCtrlReg;

        if(hCompositor->bInDitherEnable[hWindow->eId])
        {
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulLfsrInitRegAddr + hCompositor->ulRegOffset);
            *pList->pulCurrent++ = hCompositor->stInDither.ulLfsrInitReg;

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulLfsrCtrlRegAddr + hCompositor->ulRegOffset);
            *pList->pulCurrent++ = hCompositor->stInDither.ulLfsrCtrlReg;
        }
    }
#endif

#ifdef BCHP_CMP_0_V0_CSC_DITHER_CTRL
    /* CSC Dither: only with 10-bit compositor */
#if BVDC_P_DBV_SUPPORT && (BVDC_DBV_MODE_BVN_CONFORM)
    if(!BVDC_P_CMP_DBV_MODE(hCompositor))
#endif
    if(hCompositor->bIs10BitCore &&
       hCompositor->pFeatures->ulMaxVideoWindow > eWinInCmp)
    {
        uint32_t ulScale = hCompositor->bAlign12Bit ?
            BVDC_P_DITHER_CMP_SCALE_12BIT : BVDC_P_DITHER_CMP_SCALE_10BIT;
#ifdef BCHP_CMP_0_V1_CSC_DITHER_CTRL
        uint32_t ulDitherCtrlRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_CSC_DITHER_CTRL : BCHP_CMP_0_V0_CSC_DITHER_CTRL;
        uint32_t ulLfsrInitRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_CSC_DITHER_LFSR_INIT : BCHP_CMP_0_V0_CSC_DITHER_LFSR_INIT;
        uint32_t ulLfsrCtrlRegAddr = (1==eWinInCmp)?
            BCHP_CMP_0_V1_CSC_DITHER_LFSR_CTRL : BCHP_CMP_0_V0_CSC_DITHER_LFSR_CTRL;
#else
        uint32_t ulDitherCtrlRegAddr = BCHP_CMP_0_V0_CSC_DITHER_CTRL;
        uint32_t ulLfsrInitRegAddr = BCHP_CMP_0_V0_CSC_DITHER_LFSR_INIT;
        uint32_t ulLfsrCtrlRegAddr = BCHP_CMP_0_V0_CSC_DITHER_LFSR_CTRL;
#endif

        BDBG_MODULE_MSG(BVDC_DITHER,("CMP_%d_V%d CSC_DITHER: %s",
            hCompositor->eId, eWinInCmp,
            (hCompositor->bCscDitherEnable[hWindow->eId]) ? "ENABLE" : "DISABLE"));

        BVDC_P_Dither_Setting_isr(&hCompositor->stCscDither,
            hCompositor->bCscDitherEnable[hWindow->eId],
            BVDC_P_DITHER_CMP_LFSR_VALUE, ulScale);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulDitherCtrlRegAddr + hCompositor->ulRegOffset);
        *pList->pulCurrent++ = hCompositor->stCscDither.ulCtrlReg;

        if(hCompositor->bCscDitherEnable[hWindow->eId])
        {
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulLfsrInitRegAddr + hCompositor->ulRegOffset);
            *pList->pulCurrent++ = hCompositor->stCscDither.ulLfsrInitReg;

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulLfsrCtrlRegAddr + hCompositor->ulRegOffset);
            *pList->pulCurrent++ = hCompositor->stCscDither.ulLfsrCtrlReg;
        }
    }
#endif

    BSTD_UNUSED(eWinInCmp);
    BSTD_UNUSED(pList);
    BDBG_LEAVE(BVDC_P_Window_BuildDitherRul_isr);
    return;
}
#endif

#if (BVDC_P_SUPPORT_COLOR_CLIP)
/*************************************************************************
 *  {secret}
 *  BVDC_P_ColorClip_BuildRul_isr
 *  Builds Color Clip block
 **************************************************************************/
static void BVDC_P_Compositor_BuildColorClipRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList)
{
    BVDC_ColorClipSettings *pColorClipSettings;

    BDBG_ENTER(BVDC_P_Compositor_BuildColorClipRul_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    pColorClipSettings = &hCompositor->stCurInfo.stColorClipSettings;

    BDBG_MSG(("Color Clip Mode = %d", pColorClipSettings->eColorClipMode));

    /* Build RUL */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_COLOR_CLIP_CTRL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, CLIP_COUNT_RESET, 0) |
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, ENABLE,
            (pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_None) ? 0 : 1) |
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, WHITE_DISABLE,
            (pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_White ||
             pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Both) ? 0 : 1) |
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, BLACK_DISABLE,
            (pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Black ||
             pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Both) ? 0 : 1) |
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, EXTENDED_WHITE_RANGE,
            pColorClipSettings->bExtendedWhite ? 1 : 0) |
        BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, EXTENDED_BLACK_RANGE,
            pColorClipSettings->bExtendedBlack ? 1 : 0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CB_LUMA_SLOPE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CMP_0_CB_LUMA_SLOPE, SLOPE_B, (pColorClipSettings->ulCbYSlopeB >> 8) & 0x7FF) |
        BCHP_FIELD_DATA(CMP_0_CB_LUMA_SLOPE, SLOPE_A, (pColorClipSettings->ulCbYSlopeA >> 8) & 0x7FF);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CR_LUMA_SLOPE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CMP_0_CR_LUMA_SLOPE, SLOPE_B, (pColorClipSettings->ulCrYSlopeB >> 8) & 0x7FF) |
        BCHP_FIELD_DATA(CMP_0_CR_LUMA_SLOPE, SLOPE_A, (pColorClipSettings->ulCrYSlopeA >> 8) & 0x7FF);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CB_CR_SLOPE_JOINT);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CMP_0_CB_CR_SLOPE_JOINT, CB_SLOPE_JOINT, pColorClipSettings->ulCbJoint) |
        BCHP_FIELD_DATA(CMP_0_CB_CR_SLOPE_JOINT, CR_SLOPE_JOINT, pColorClipSettings->ulCrJoint);

    BDBG_LEAVE(BVDC_P_ColorClip_BuildRul_isr);
    return;
}
#endif

/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x_Vx registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */

static void BVDC_P_Compositor_BuildRul_Graphics_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList)
{
    /* Gwin0 */
    if(BVDC_P_CMP_COMPARE_FIELD_DATA(CMP_0_G0_SURFACE_CTRL, ENABLE, 1))
    {
        BDBG_CASSERT(4 == (((BCHP_CMP_0_G0_CANVAS_OFFSET - BCHP_CMP_0_G0_SURFACE_SIZE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CMP_0_G0_CANVAS_OFFSET - BCHP_CMP_0_G0_SURFACE_SIZE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_G0_SURFACE_SIZE + hCompositor->ulRegOffset);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_G0_SURFACE_SIZE, 0);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_G0_SURFACE_OFFSET, 0);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_G0_DISPLAY_SIZE, 0);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_G0_CANVAS_OFFSET, 0);

        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_G0_CANVAS_X_OFFSET_R, pList->pulCurrent);
    }

#if (BVDC_P_SUPPORT_CSC_MAT_COEF_VER >= 2)
    if(hCompositor->stCurInfo.stDirty.stBits.bColorClip || hCompositor->bInitial)
    {
        /* CMP color clip configuration is only supported by compositor 0 of certain
         * chipsets.
         */
#if (BVDC_P_SUPPORT_COLOR_CLIP)
        if(hCompositor->eId == BVDC_CompositorId_eCompositor0)
            BVDC_P_Compositor_BuildColorClipRul_isr(hCompositor, pList);
#endif
        hCompositor->stCurInfo.stDirty.stBits.bColorClip = BVDC_P_CLEAN;
    }
#endif
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x_Vx registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */

static void BVDC_P_Compositor_BuildRul_Video_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_WindowId                  eVId,     /* window index */
      BVDC_WindowId                    eWinInCmp) /* 0 or 1 */
{
    uint32_t ulV0V1Offset = 0;

#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1)
    if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
        ulV0V1Offset = eWinInCmp*(BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_V0_SURFACE_SIZE);
#else
    BSTD_UNUSED(eWinInCmp);
#endif

#if BVDC_P_SUPPORT_WIN_CONST_COLOR
        /* window constant color settings */
    BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CONST_COLOR, ulV0V1Offset, pList->pulCurrent);
#endif

    if(BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, ulV0V1Offset, SURFACE_ENABLE, 1))
    {
        if (((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset, HSIZE) +
              BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET)) >
             (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset, HSIZE))) ||
            ((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset, VSIZE) +
              BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET)) >
             (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset, VSIZE))))
        {
            uint32_t  ulDspW, ulCutLeft, ulDspH, ulCutTop;

            BDBG_ERR(("Invalid CMP V%d disp size [H:%d, V:%d] vs surf size [H:%d, V:%d] - [X:%d, Y:%d]",
                      (uint32_t)eVId,
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, HSIZE),
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, VSIZE),
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset, HSIZE),
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset, VSIZE),
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET),
                      BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET)));

            ulDspW    = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, HSIZE);
            ulCutLeft = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET);
            ulDspH    = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, VSIZE);
            ulCutTop  = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET);

            if ((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset,HSIZE) +
                 BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset,X_OFFSET)) >
                (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset,HSIZE)))
            {
                ulDspW = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,ulV0V1Offset, HSIZE);
                ulCutLeft = 0;
            }

            if ((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset,VSIZE) +
                 BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset,Y_OFFSET)) >
                (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset,VSIZE)))
            {
                ulDspH = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset,VSIZE);
                ulCutTop = 0;
            }

            BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset) =
                BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, HSIZE, ulDspW) |
                BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, VSIZE, ulDspH);
            BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset) =
                BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, X_OFFSET, ulCutLeft) |
                BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, Y_OFFSET, ulCutTop);
        }

        BDBG_CASSERT(4 == (((BCHP_CMP_0_V0_CANVAS_OFFSET - BCHP_CMP_0_V0_SURFACE_SIZE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CMP_0_V0_CANVAS_OFFSET - BCHP_CMP_0_V0_SURFACE_SIZE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_V0_SURFACE_SIZE + hCompositor->ulRegOffset + ulV0V1Offset);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset);
        *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_CANVAS_OFFSET, ulV0V1Offset);

        BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CANVAS_X_OFFSET_R, ulV0V1Offset, pList->pulCurrent);

#if BVDC_P_SUPPORT_CMP_V0_CLEAR_RECT
        if(hCompositor->ulMosaicAdjust[eVId] || hCompositor->bInitial)
        {
            if(hCompositor->ahWindow[eVId]->stCurInfo.bClearRect)
            {
                BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_COLOR, ulV0V1Offset, pList->pulCurrent);

                BDBG_CASSERT(2 == (((BCHP_CMP_0_V0_RECT_ENABLE_MASK - BCHP_CMP_0_V0_RECT_TOP_CTRL) / sizeof(uint32_t)) + 1));
                *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CMP_0_V0_RECT_ENABLE_MASK - BCHP_CMP_0_V0_RECT_TOP_CTRL) / sizeof(uint32_t)) + 1);
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_V0_RECT_TOP_CTRL + hCompositor->ulRegOffset + ulV0V1Offset);
                *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_RECT_TOP_CTRL, ulV0V1Offset);
                *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_RECT_ENABLE_MASK, ulV0V1Offset);

                BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(CMP_0_V0_RECT_SIZEi_ARRAY_BASE,
                    hCompositor->ahWindow[eVId]->stCurInfo.ulMosaicCount, ulV0V1Offset, pList->pulCurrent);
                BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(CMP_0_V0_RECT_OFFSETi_ARRAY_BASE,
                    hCompositor->ahWindow[eVId]->stCurInfo.ulMosaicCount, ulV0V1Offset, pList->pulCurrent);
            }
            else
            {
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_V0_RECT_TOP_CTRL + hCompositor->ulRegOffset + ulV0V1Offset);
                *pList->pulCurrent++ = 0;
            }

            if(pList->bLastExecuted && hCompositor->ulMosaicAdjust[eVId])
            {
                hCompositor->ulMosaicAdjust[eVId]--;
            }
        }
#endif

        if(hCompositor->ulCscAdjust[eVId])
        {
            BVDC_Window_Handle hWindow = hCompositor->ahWindow[eVId];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

            if(hCompositor->bCscCompute[eVId])
            {
#if BVDC_P_CMP_0_MOSAIC_CFCS
                if(hWindow->stCurInfo.bClearRect)
                {
                    BVDC_P_Window_CalculateMosaicCsc_isr(hWindow);
                }
                else
#endif
                    BVDC_P_Window_CalculateCsc_isr(hWindow);

                hCompositor->bCscCompute[eVId] = false;
            }
            if(hCompositor->bCscDemoCompute[eVId])
            {
                BVDC_P_Window_SetSecCscDemo_isr(hWindow);
                hCompositor->bCscDemoCompute[eVId] = false;
            }

#if BVDC_P_CMP_0_MOSAIC_CFCS
            /* Mosaic mode with CMP mosaic cscs. */
            if(hWindow->stCurInfo.bClearRect)
            {
                uint32_t i;
                for (i = 0; i < hWindow->stCurInfo.ulMosaicCount; i++)
                {
                    BVDC_P_Window_BuildCfcRul_isr(hWindow, i, pList);
                }

                /* Copy mosaic csc indices to rul from shadow regs */
                BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_CSC_INDEX_0 , ulV0V1Offset, pList->pulCurrent);
                if(hWindow->stSettings.ulMaxMosaicRect > 8)
                {
                    BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_CSC_INDEX_1 , ulV0V1Offset, pList->pulCurrent);
                }
            }
            else
#endif
            {
                BVDC_P_Window_BuildCfcRul_isr(hWindow, 0, pList);
                if (BVDC_P_WIN_GET_FIELD_NAME(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_DEMO_ENABLE))
                {
                    BVDC_P_Window_BuildCfcRul_isr(hWindow, 1, pList);
                }
                /* CRBVN-782: disable CSC_INDEX to use R0_LSHT by default */
                #if BVDC_P_CRBVN_782_WORKAROUND && BCHP_CMP_0_V0_RECT_CSC_INDEX_0
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_V0_RECT_CSC_INDEX_0 + hCompositor->ulRegOffset + ulV0V1Offset);
                *pList->pulCurrent++ = BCHP_FIELD_ENUM(CMP_0_V0_RECT_CSC_INDEX_0, CSC_COEFF_INDEX_RECT0, DISABLE);
                #endif
            }

            if(pList->bLastExecuted)
            {
                hCompositor->ulCscAdjust[eVId]--;
            }
        }

#if BVDC_P_TCH_SUPPORT
        if ((hCompositor->stCfcCapability[0].stBits.bTpToneMapping) && (eVId == BVDC_P_WindowId_eComp0_V0))
        {
            const BVDC_P_CfcMetaData *pMetaData = (BVDC_P_CfcMetaData *)hCompositor->ahWindow[eVId]->pMainCfc->stColorSpaceExtIn.stColorSpace.pMetaData;
            if (pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType))
            {
                BVDC_P_Compositor_BuildTchVsyncRul_isr(hCompositor, pList);
            }
        }
#endif

        if(hCompositor->ulDitherChange[eVId])
        {
            BVDC_Window_Handle hWindow = hCompositor->ahWindow[eVId];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

            BVDC_P_Window_BuildDitherRul_isr(hWindow, pList);

            if(pList->bLastExecuted)
            {
                hCompositor->ulDitherChange[eVId]--;
            }
        }
    }

    /* Color key */
    if( hCompositor->ulColorKeyAdjust[eVId] )
    {
        if(pList->bLastExecuted)
        {
            hCompositor->ulColorKeyAdjust[eVId]--;
        }
        /* CMP_0_V0_LUMA_KEYING, CMP_0_V0_CB_KEYING and CMP_0_V0_CR_KEYING
         * are not always together */
        BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_LUMA_KEYING, ulV0V1Offset, pList->pulCurrent);
        BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CB_KEYING,   ulV0V1Offset, pList->pulCurrent);
        BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CR_KEYING,   ulV0V1Offset, pList->pulCurrent);
    }
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */
static void BVDC_P_Compositor_BuildRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId )
{
    BVDC_P_Window_DirtyBits *pV0CurDirty;
    BVDC_P_WindowId eV0Id, eV1Id = BVDC_P_WindowId_eComp0_V1;
#ifndef BVDC_FOR_BOOTUPDATER
    uint32_t i;
#endif

    BDBG_ENTER(BVDC_P_Compositor_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BSTD_UNUSED(eFieldId);

    if(hCompositor->bIsBypass)
    {
        /* Bypass compositor (dummy one) has nothing to program except
         * video path controlling. */
        pV0CurDirty = &(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_V0]->stCurInfo.stDirty);

        /* CSC already handle in 656out clear dirty bit now. */
        pV0CurDirty->stBits.bCscAdjust = BVDC_P_CLEAN;
        pV0CurDirty->stBits.bCabAdjust = BVDC_P_CLEAN;
        pV0CurDirty->stBits.bLabAdjust = BVDC_P_CLEAN;
        pV0CurDirty->stBits.bTntAdjust = BVDC_P_CLEAN;
        return;
    }

    eV0Id = BVDC_P_CMP_GET_V0ID(hCompositor);

    if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
    {
        eV1Id = hCompositor->eId? BVDC_P_WindowId_eComp1_V1 : BVDC_P_WindowId_eComp0_V1;
    }

    /* reset */
    if(hCompositor->bInitial)
    {
        /* Make sure these get re-programm when reset. */
        hCompositor->ulCscAdjust[eV0Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
        hCompositor->ulColorKeyAdjust[eV0Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
        hCompositor->bCscDemoCompute[eV0Id] = true;

        hCompositor->ulDitherChange[eV0Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
        hCompositor->bInDitherEnable[eV0Id] = false;
        hCompositor->bCscDitherEnable[eV0Id] = false;

        if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
        {
            hCompositor->ulCscAdjust[eV1Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
            hCompositor->ulColorKeyAdjust[eV1Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
            hCompositor->bCscDemoCompute[eV1Id] = true;
            hCompositor->ulDitherChange[eV1Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
            hCompositor->bInDitherEnable[eV1Id] = false;
            hCompositor->bCscDitherEnable[eV1Id] = false;
        }

        /* reset CMP for transcoder in case watchdog recovery */
        if(hCompositor->hDisplay->stCurInfo.bEnableStg) {
            BVDC_P_BUILD_RESET(pList->pulCurrent, hCompositor->ulCoreResetAddr, hCompositor->ulCoreResetMask);
        }

        /* Also make sure to enable CRC */
        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CRC_CTRL, pList->pulCurrent);

        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_OUT_CTRL, pList->pulCurrent);
    }

#ifndef BVDC_FOR_BOOTUPDATER
    BVDC_P_Pep_BuildRul_isr(hCompositor->ahWindow[eV0Id], pList, hCompositor->bInitial);
    BVDC_P_Pep_BuildRul_isr(hCompositor->ahWindow[eV1Id], pList, hCompositor->bInitial);
#endif

    /* canvas, bgcolor and blender setting */
    BDBG_CASSERT(2 == (((BCHP_CMP_0_BG_COLOR - BCHP_CMP_0_CANVAS_SIZE) / sizeof(uint32_t)) + 1));
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_CMP_0_BG_COLOR - BCHP_CMP_0_CANVAS_SIZE) / sizeof(uint32_t)) + 1);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CANVAS_SIZE + hCompositor->ulRegOffset);
    *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_CANVAS_SIZE, 0);
    *pList->pulCurrent++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_BG_COLOR, 0);

    BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_0_CTRL, pList->pulCurrent);

#ifndef BVDC_FOR_BOOTUPDATER
#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 1
    if(hCompositor->pFeatures->ulMaxWindow > 1)
    {
        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_1_CTRL, pList->pulCurrent);
    }
#endif

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 2
    if(hCompositor->pFeatures->ulMaxWindow > 2)
    {
        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_2_CTRL, pList->pulCurrent);
    }
#endif

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 3
    if(hCompositor->pFeatures->ulMaxWindow > 3)
    {
        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_3_CTRL, pList->pulCurrent);
    }
#endif

    BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_CTRL, pList->pulCurrent);

#if BVDC_P_SUPPORT_CMP_DEMO_MODE
    BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CSC_DEMO_SETTING, pList->pulCurrent);
#endif

    /* Vwin0 */
    for(i=eV0Id; i< (eV0Id + hCompositor->pFeatures->ulMaxVideoWindow); i++)
    {
        BVDC_P_Compositor_BuildRul_Video_isr(hCompositor, pList, (BVDC_P_WindowId)i, (BVDC_WindowId)(i - eV0Id));
    }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    /* Gwin0 */
    BVDC_P_Compositor_BuildRul_Graphics_isr(hCompositor, pList);

    if(hCompositor->bInitial)
    {
        hCompositor->bInitial = false;
    }

    BDBG_LEAVE(BVDC_P_Compositor_BuildRul_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_BuildSyncLockRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId )
{
    int i;
    bool abForceSlot[BVDC_P_MAX_SOURCE_COUNT];
    BDBG_ENTER(BVDC_P_Compositor_BuildSyncLockRul_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    hCompositor->hForceTrigPipSrc = NULL;

    /* to handle multiple windows coming from the same mpg source */
    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        abForceSlot[i] = false;
    }

    /* Add an entry into the RUL with sole purpose of force trigger the
     * the source slot.  Take advantage of the way register layout, each
     * slot immediate trigger is 16 bytes apart.
     *
     * There are two cases where we want to force trigger the source's
     * slot RUL.
     *
     * (1) The mpeg sync-lock source the LOCKED to this compositor.
     * (2) The other mpeg source's that hasn't connected to any compositor. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        BVDC_Source_Handle hSource;

        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hCompositor->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
        {
            continue;
        }

        BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
        if((hCompositor->ahWindow[i]->bUserAppliedChanges) &&
           (hCompositor->ahWindow[i]->stCurInfo.hSource == NULL))
        {
            hSource = hCompositor->ahWindow[i]->stNewInfo.hSource;
        }
        else
        {
            hSource = hCompositor->ahWindow[i]->stCurInfo.hSource;
        }
        BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

        /* Force trigger mpeg slot:
         * 1) MPEG Sync-lock source required force trigger.
         * 2) MPEG PIP source not locking to any compositor yet and not triggered by MTG. */
        if((BVDC_P_SRC_IS_MPEG(hSource->eId)) &&
           (!abForceSlot[hSource->eId]) &&
#if BVDC_P_SUPPORT_MTG
           (!hSource->bMtgSrc) &&
#endif
           ((hSource->hSyncLockCompositor == hCompositor) ||
            (hSource->hSyncLockCompositor == NULL)))
        {
            /* Note, we invert the field polarity if it's locked with a captured mpeg src
             *   to adapt to 2-field multi-buffering, which puts read pointer one field
             *   behind write pointer; the trigger field swap only makes sense with
             *   active window and interlaced formats;
             * PR43082: This flag needs to reflect the total BVN delay of the sync-locked
             * window to achieve precise lipsync goal with HD/SD simul mode; */
            /* FIELDSWAP 1) detection; */
            if(BVDC_P_STATE_IS_ACTIVE(hCompositor->hSyncLockWin))
            {
                uint32_t ulTotalBvnDelay;
                /* forced synclock window uses its own delay to decide bFieldSwap! */
                if(hCompositor->ahWindow[i]->stSettings.bForceSyncLock) {
                    ulTotalBvnDelay = hCompositor->ahWindow[i]->ulTotalBvnDelay;
                } else {
                    ulTotalBvnDelay = hCompositor->hSyncLockWin->ulTotalBvnDelay;
                }
                hSource->bFieldSwap = (ulTotalBvnDelay & 1);
                BDBG_MSG(("CMP[%d], SRC[%d], WIN[%d], bFieldSwap[%d], ulTotalBvnDelay[%d] pol[%u]",
                    hCompositor->eId,
                    hSource->eId,
                    hCompositor->hSyncLockWin->eId,
                    hSource->bFieldSwap,
                    ulTotalBvnDelay, eFieldId));
            }

            /* Force source slot to execute. */
            if((hSource->hSyncLockCompositor == NULL) && (hSource->ulTransferLock))
            {
                BDBG_MSG(("Don't force trigger the PIP window yet"));
                BVDC_P_BUILD_NO_OPS(pList->pulCurrent);

                /* Note: this is a backup to clear the transfer lock semaphore, in case
                   the critical source _isr, responsible to clear semaphore, is called
                   too early (_isr call disorder issue) such that it doesn't see the
                   semaphore, and source disconnection display _isr sets semaphore later
                   and stops building force trigger RUL for that mpeg source; that mpeg
                   source could lose driving interrupt forever! */
                if(--hSource->ulTransferLock == 0)
                {
                    /* clean up source slots to prepare for the trigger transfer */
                    BVDC_P_Source_CleanupSlots_isr(hSource);
                }
                BDBG_MSG(("CMP[%d] decrements SRC[%d]'s ulTransferLock semaphore count %d",
                    hCompositor->eId, hSource->eId, hSource->ulTransferLock));
            }
            else
            {
                abForceSlot[hSource->eId] = true;
                /* FIELDSWAP 3) force field swapped trigger; */
                /* interlaced non real time transcode mode*/
#if BVDC_P_SUPPORT_STG
                if((eFieldId != BAVC_Polarity_eFrame) &&
                    (BVDC_P_DISPLAY_NRT_STG(hCompositor->hDisplay)))
                {
                    bool bToggleSrcPolarity = hSource->bFieldSwap;
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* if stg hw cannot repeat pol for ignore pic, accounted here */
                    /* if stg hw cannot repeat cadence, after workaround forced repeat src cadence, toggle display cadence here */
                    /* NOTE: the src cadence toggle flag is derived at end of last round of NRT src/disp isr pair! */
                    bToggleSrcPolarity ^= hSource->bToggleCadence;
                    BDBG_MSG(("Cmp[%u] to toggle pol cadence = %u [%u]", hCompositor->eId, hSource->bToggleCadence, bToggleSrcPolarity));
#endif
                    *pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
                    BDBG_MODULE_MSG(BVDC_REPEATPOLARITY,("use src[%d] ignore flag", hSource->eId));
                    *pList->pulCurrent++ = BRDC_REGISTER(hSource->ulScratchPolReg);
                    *pList->pulCurrent++ = BRDC_OP_VAR_XOR_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);

                    if((bToggleSrcPolarity? BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId) == BAVC_Polarity_eTopField) {
                        *pList->pulCurrent++ = 0;
                    } else /* bottom */ {
                        *pList->pulCurrent++ = 1;
                    }
                    *pList->pulCurrent++ = BRDC_OP_COND_SKIP(BRDC_Variable_1);
                    *pList->pulCurrent++ = 5;/* if bottom(1) slot, skip 5 dwords */
                    BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[BAVC_Polarity_eTopField]);
                    *pList->pulCurrent++ = BRDC_OP_SKIP();/* continue by skip 3 dwords bot RUL; */
                    *pList->pulCurrent++ = 3;/* 3-dword for immediate trigger RUL */
                    BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[BAVC_Polarity_eBotField]);

                } else
#endif
                    BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[
                        hSource->bFieldSwap ? BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId]);

                hSource->ulDispVsyncFreq = hCompositor->hDisplay->stCurInfo.ulVertFreq;
            }

            /* Assume:
               1) two mpg sources at most in system;
               2) mpg source can only be sync slipped if it's in the PIP of a sync-locked
                  compositor; */
            if(hSource->hSyncLockCompositor == NULL)
            {
                hCompositor->hForceTrigPipSrc = hSource;
            }
        }
    }

    BDBG_LEAVE(BVDC_P_Compositor_BuildSyncLockRul_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
bool BVDC_P_Compositor_BuildSyncSlipRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId,
      bool                             bBuildCanvasCtrl )
{
    int i;
    bool bBuilldBothTopBotSlots = false;

    BDBG_ENTER(BVDC_P_Compositor_BuildSyncSlipRul_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hDisplay, BVDC_DSP);

    /* Now update windows state (including setInfo) for next picture to be displayed. */
    BVDC_P_Compositor_WindowsReader_isr(hCompositor, eFieldId, pList);

    /* Build sync slip Rul.  Basically this build playback portion of
     * RUL.  Most likely it will be the VEC, CMP, VFDs, GFD, [SCLs], and
     * VNET.  If no window is active (just bgcolor) it will just build the
     * VEC, CMP, and VNET. */
    BVDC_P_Vec_BuildRul_isr(hCompositor->hDisplay, pList, eFieldId);

    /* Build Compositor RUL */
    if(hCompositor->hDisplay->eState == BVDC_P_State_eActive ||
       hCompositor->hDisplay->eState == BVDC_P_State_eDestroy) {
        BVDC_P_Compositor_BuildRul_isr(hCompositor, pList, eFieldId);
    } else {
        bBuildCanvasCtrl = false;
        BDBG_MSG(("CMP%d disabled with disp%d state=%d", hCompositor->eId, hCompositor->hDisplay->eId, hCompositor->hDisplay->eState));
    }

    /* Build related video backend blocks that associated with active windows.
     * Only build the READER portion of the windows. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hCompositor->ahWindow[i] ||
           BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
           BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
        {
            continue;
        }

        BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
        /* we need to keep building the sliped window and the newly
         * transfered locked window on the consistent vec trigger; */
        if(!hCompositor->ahWindow[i]->bSyncLockSrc || hCompositor->ulSlip2Lock)
        {
            /* Note: sync-slipped window reader shutdown needs to build both slots to
             * make both top/bot slots consistent in case next time the isr is not
             * serviced on time, the staled playback RUL could unmute when the
             * capture side is disconnecting vnet;  */
            if(!hCompositor->ahWindow[i]->bSyncLockSrc &&
               BVDC_P_SRC_IS_VIDEO(hCompositor->ahWindow[i]->stCurInfo.hSource->eId))
            {
                bBuilldBothTopBotSlots |= (BVDC_P_State_eShutDownRul == hCompositor->ahWindow[i]->stCurInfo.eReaderState);
            }

            /* Only need to build the READER part of the slip. */
            BVDC_P_Window_BuildRul_isr(hCompositor->ahWindow[i], pList, eFieldId,
                false, /* writer*/
                true,  /* reader */
                false  /* CanvasCtrl */ );
        }
    }

    /* HDR blend out matrix */
    BVDC_P_Compositor_BuildBlendOutMatrixRul_isr(hCompositor, pList);

    /* build rdc rul to load ram luts */
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    if (BVDC_CompositorId_eCompositor0 == hCompositor->eId)
    {
        BVDC_P_Cfc_BuildRulForLutLoading_isr(&hCompositor->stCfcLutList, BCHP_HDR_CMP_0_LUT_DESC_ADDR, BCHP_HDR_CMP_0_LUT_DESC_CFG, pList);
    }
#endif

    /* clean bOutColorSpace dirty bit after both V0, V1 and Gfd rul are built */
    hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace = BVDC_P_CLEAN;

    /* convas should not be enabled until all surfaces are configured and enabled,
     * notice that syncLock window RUL is built after BuildSyncSlipRul_isr is called */
    if (bBuildCanvasCtrl)
    {
        BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CANVAS_CTRL, pList->pulCurrent);
    }

    BDBG_LEAVE(BVDC_P_Compositor_BuildSyncSlipRul_isr);
    return bBuilldBothTopBotSlots;
}

/***************************************************************************
 * {private}
 *
 * to be called for syncLock case since syncLock window RUL is built after
 * BuildSyncSlipRul_isr is called
 */
void BVDC_P_Compositor_BuildConvasCtrlRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList )
{
    BDBG_ENTER(BVDC_P_Compositor_BuildConvasCtrlRul_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CANVAS_CTRL, pList->pulCurrent);

    BDBG_LEAVE(BVDC_P_Compositor_BuildConvasCtrlRul_isr);
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Compositor_AssignTrigger_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BRDC_Trigger                     eTopTrigger,
      BRDC_Trigger                     eBotTrigger )
{
    BDBG_ENTER(BVDC_P_Compositor_AssignTrigger_isr);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    BRDC_Slot_ExecuteOnTrigger_isr(hCompositor->ahSlot[BAVC_Polarity_eTopField],
        eTopTrigger, true);
    BRDC_Slot_ExecuteOnTrigger_isr(hCompositor->ahSlot[BAVC_Polarity_eBotField],
        eBotTrigger, true);

    BDBG_LEAVE(BVDC_P_Compositor_AssignTrigger_isr);
    return BERR_SUCCESS;
}

/**************************************************************************
 * This function will call the READER of all the windows hold by this compositor,
 * and determine the input color space to VEC, also select the video window
 * color space conversion matrix if necessary.
 */
#ifndef BVDC_FOR_BOOTUPDATER
void BVDC_P_Compositor_WindowsReader_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BAVC_Polarity                    eNextFieldId,
      BVDC_P_ListInfo                 *pList )
{
    int                       i;
    uint32_t                  ulVSize, ulHSize;
    BVDC_Window_Handle        hWindow;
    BVDC_Window_Handle        hWindow0 = NULL;
#if BVDC_P_DBV_SUPPORT
    BVDC_P_GfxFeeder_Handle   hGfxFeeder = NULL;
#endif
    bool                      bWidthTrim = true;
    BVDC_DisplayTg            eMasterTg;
    bool                      bDTg;
    bool                      bBgCsc = false;
    BFMT_Orientation          eOrientation;
#if (BVDC_P_DCX_3D_WORKAROUND)
    bool                      bEnableDcxm = false;
#endif
    BVDC_P_Display_SrcInfo    stSrcInfo;
    const BFMT_VideoInfo     *pFmtInfo;
    BCFC_Colorimetry          eOutColorimetry;

    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    BVDC_P_Compositor_UpdateOutColorSpace_isr(hCompositor, false); /* for nxserver/client */
    eOutColorimetry = hCompositor->stOutColorSpaceExt.stColorSpace.eColorimetry;
    pFmtInfo = hCompositor->stCurInfo.pFmtInfo;

    /* second pass: to adjust non-vbi-pass-thru window position;
       Note: adjustment is done in the second pass in case vwin0 has no pass-thru,
             but vwin1 has; */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to program. */
        if(!hCompositor->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
        {
            continue;
        }

        hWindow = hCompositor->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
        BVDC_P_Window_Reader_isr(hCompositor->ahWindow[i], eNextFieldId, pList);
#if (BVDC_P_DCX_3D_WORKAROUND)
        if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
        {
            if(hWindow->pCurReaderNode)
                bEnableDcxm = hWindow->pCurReaderNode->bEnableDcxm;
        }
#endif
        if((BVDC_P_WindowId)i == BVDC_P_CMP_GET_V0ID(hCompositor))
        {
            hWindow0 = hWindow;
        }
#if BVDC_P_DBV_SUPPORT
        if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
        {
            hGfxFeeder = hWindow->stCurInfo.hSource->hGfxFeeder;
        }
#endif
        /* turn off 704-sample feature if any window is larger than 704 width */
        if(BVDC_P_WIN_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, HSIZE) > 704)
        {
            bWidthTrim = false;
        }
    }

    /* Turn off 704-sample feature if DCS. */
    /* TODO: remove this restriction */
#if DCS_SUPPORT
    if (VIDEO_FORMAT_SUPPORTS_DCS (pFmtInfo->eVideoFmt))
    {
        bWidthTrim = false;
    }
#endif

    /* We do not have a 704-sample ARIB array, so this is another case where we
     * have to remove this feature */
    if (hCompositor->hDisplay->bArib480p)
    {
        bWidthTrim = false;
    }

    /* set up display sourceinfo */
    /* detect SD 704-sample input: black at both edges, all windows are <= 704 width; */
    bWidthTrim = (
        (bWidthTrim) &&
        (hCompositor->stCurInfo.ulBgColorYCrCb == BVDC_P_YCRCB_BLACK) &&
        (VIDEO_FORMAT_IS_SD(pFmtInfo->eVideoFmt)));
    bBgCsc = (hCompositor->hDisplay->stCurInfo.eCmpColorimetry != eOutColorimetry);
    stSrcInfo.bWidthTrimmed = bWidthTrim;
    stSrcInfo.bBypassDviCsc = hCompositor->bBypassDviCsc;
    stSrcInfo.eCmpColorimetry = eOutColorimetry;

    if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOff)
    {
        hCompositor->bFullRate = true;
    }
    else if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOn)
    {
        hCompositor->bFullRate = false;
    }

    /* auto track source if enabled or last used */
    stSrcInfo.bFullRate = hCompositor->bFullRate;

    BVDC_P_Display_SetSourceInfo_isr(hCompositor->hDisplay, &stSrcInfo);

    /* set compositor size -- number of lines. */
    eMasterTg = hCompositor->hDisplay->eMasterTg;
    bDTg      =  BVDC_P_DISPLAY_USED_DIGTRIG(eMasterTg);

#if BVDC_P_SUPPORT_STG
    pFmtInfo = (BVDC_P_DISPLAY_USED_STG(eMasterTg) &&
        (hCompositor->hDisplay->pStgFmtInfo !=NULL))?
            hCompositor->hDisplay->pStgFmtInfo:hCompositor->stCurInfo.pFmtInfo;

    eOrientation = BVDC_P_DISPLAY_USED_STG(eMasterTg)?
        hCompositor->eDspOrientation: hCompositor->stCurInfo.eOrientation;
#else
    eOrientation = hCompositor->stCurInfo.eOrientation;
#endif

    ulHSize   = bDTg ? pFmtInfo->ulDigitalWidth :pFmtInfo->ulWidth;
    ulVSize   = bDTg ? pFmtInfo->ulDigitalHeight:pFmtInfo->ulHeight;
    ulVSize >>= pFmtInfo->bInterlaced;

    /* Handle 3d case */
    if(!BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
    {
        if(eOrientation == BFMT_Orientation_e3D_LeftRight)
        {
            ulHSize = ulHSize / 2;
        }
        else if(eOrientation == BFMT_Orientation_e3D_OverUnder)
        {
            ulVSize = ulVSize / 2;
        }
    }

    BDBG_MODULE_MSG(BVDC_CMP_SIZE,("Canvas[%d] %4d x %4d orientation %d bDtg%u pFmt%p",
        hCompositor->eId,  ulHSize,ulVSize, eOrientation, bDTg, (void *)pFmtInfo));
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_SIZE) = (
        BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, VSIZE, ulVSize));

    if(BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
        eOrientation = pFmtInfo->eOrientation;

#if (BVDC_P_DCX_3D_WORKAROUND)
    if(bEnableDcxm)
        eOrientation = BFMT_Orientation_e2D;
#endif

#if (BVDC_P_SUPPORT_3D==0)
    eOrientation = BFMT_Orientation_e2D;
#endif
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_CTRL) = (
        BCHP_FIELD_DATA(CMP_0_CMP_CTRL, BVB_VIDEO, eOrientation));

#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
#if BVDC_P_DBV_SUPPORT
    /* clear the flag first no matter input has dbv or not */
    if(hCompositor->pstDbv) hCompositor->pstDbv->metadataPresent = false;

    /* DBV will compose video inputs together */
    if ((hCompositor->stCfcCapability[0].stBits.bDbvCmp) &&
        (hCompositor->hDisplay->stCurInfo.stHdmiSettings.stSettings.bDolbyVisionEnabled ||
        (hWindow0)))
    {
        if(hCompositor->pstDbv) {
            BVDC_P_Compositor_ApplyDbvSettings_isr(hCompositor, hGfxFeeder);
        }
    } else if(hGfxFeeder) {
        hGfxFeeder->bDbvEnabled = false;/* gfd exits dvs mode */
    }
#endif
#if BVDC_P_TCH_SUPPORT
    if (hCompositor->stCfcCapability[0].stBits.bTpToneMapping && hWindow0)
    {
        const BVDC_P_CfcMetaData *pMetaData = (BVDC_P_CfcMetaData *)hWindow0->pMainCfc->stColorSpaceExtIn.stColorSpace.pMetaData;
        if (pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType))
        {
            BVDC_P_Compositor_ApplyTchSettings_isr(hCompositor);
        }
    }
#endif
#else
    BSTD_UNUSED(hWindow0);
#endif

    /* Update BG matrixCoeffs to match base on src/disp changes */
    bBgCsc = bBgCsc || hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace;
    if(bBgCsc)
    {
        uint32_t ulBgColorYCrCb;
        ulBgColorYCrCb = BVDC_P_Compositor_Update_Canvas_Background_isrsafe(hCompositor,
            hCompositor->stCurInfo.ucRed,
            hCompositor->stCurInfo.ucGreen,
            hCompositor->stCurInfo.ucBlue);

        hCompositor->stNewInfo.ulBgColorYCrCb = ulBgColorYCrCb;
        hCompositor->stCurInfo.ulBgColorYCrCb = ulBgColorYCrCb;
        BVDC_P_CMP_SET_REG_DATA(CMP_0_BG_COLOR, ulBgColorYCrCb);
    }


    BDBG_LEAVE(BVDC_P_Compositor_WindowsReader_isr);
    return;
}

void BVDC_P_Compositor_SetMBoxMetaData_isr
    ( const BVDC_P_PictureNode        *pPicture,
      BVDC_Compositor_Handle           hCompositor)
{
    BDBG_ENTER(BVDC_P_Compositor_SetMBoxMetaData);
    BDBG_OBJECT_ASSERT(hCompositor,     BVDC_CMP);

    if(NULL!=pPicture)
    {
        hCompositor->ulOrigPTS               = pPicture->ulOrigPTS;

        hCompositor->uiHorizontalPanScan     = pPicture->iHorzPanScan;
        hCompositor->uiVerticalPanScan       = pPicture->iVertPanScan;

        hCompositor->ulDisplayHorizontalSize = pPicture->ulDispHorzSize;
        hCompositor->ulDisplayVerticalSize   = pPicture->ulDispVertSize;

        hCompositor->ePictureType            = pPicture->ePictureType;

        hCompositor->eSourcePolarity         = pPicture->eSrcPolarity;
        hCompositor->bPictureRepeatFlag      = pPicture->stFlags.bPictureRepeatFlag;
        hCompositor->bIgnorePicture          = pPicture->bIgnorePicture;
        hCompositor->bStallStc               = pPicture->bStallStc;
        hCompositor->bLast                   = pPicture->bLast;
        hCompositor->bChannelChange          = pPicture->bChannelChange;
        hCompositor->bMute                   = pPicture->bMute;
        hCompositor->ulStgPxlAspRatio_x_y    = pPicture->ulStgPxlAspRatio_x_y;
        hCompositor->ulDecodePictureId       = pPicture->ulDecodePictureId;
        hCompositor->bValidAfd               = pPicture->bValidAfd;
        hCompositor->ulAfd                   = pPicture->ulAfd;
        hCompositor->eBarDataType            = pPicture->eBarDataType;
        hCompositor->ulTopLeftBarValue       = pPicture->ulTopLeftBarValue;
        hCompositor->ulBotRightBarValue      = pPicture->ulBotRightBarValue;
        hCompositor->bPreChargePicture       = pPicture->bPreChargePicture;
        hCompositor->bEndofChunk             = pPicture->bEndofChunk;
        hCompositor->ulChunkId               = pPicture->ulChunkId;
        hCompositor->eDspOrientation         = pPicture->eDispOrientation;

        /*stg path is controlled in bvdc_displaystg_priv.c */
        if(!BVDC_P_DISPLAY_USED_STG(hCompositor->hDisplay->eMasterTg))
        {
            hCompositor->ulPicId += (!hCompositor->bIgnorePicture);
        }
    }

    BDBG_LEAVE(BVDC_P_Compositor_SetMBoxMetaData);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/* End of file. */
