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
#include "bvdc_compositor_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_tnt.h"
#include "bvdc_tnt_priv.h"

BDBG_MODULE(BVDC_TNT);

#if (BVDC_P_SUPPORT_TNT)
void BVDC_P_Tnt_BuildInit_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BDBG_ENTER(BVDC_P_Tnt_BuildInit_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_TNT_VER == 6)
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(3);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ARRAY_BASE + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ = 0x51e;
    *pList->pulCurrent++ = 0x1b4;
    *pList->pulCurrent++ = 0x4f;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(TNT_CMP_0_V0_LPEAK_OUT_CORE, MODE,        TOTAL) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_OUT_CORE, LCORE_BAND2, 0x8  ) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_OUT_CORE, LCORE_BAND1, 0x8  );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(TNT_CMP_0_V0_LPEAK_CLIP_AVOID, CLIPAVOID_EN, ENABLE) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_CLIP_AVOID, SLOPE2,       0x1)    |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_CLIP_AVOID, SLOPE1,       0x1)    |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_CLIP_AVOID, CLIPVAL2,     0x334)  |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_CLIP_AVOID, CLIPVAL1,     0xc8);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LTI_FILTER + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(TNT_CMP_0_V0_LTI_FILTER, BLUR_EN,    ENABLE) |
        BCHP_FIELD_ENUM(TNT_CMP_0_V0_LTI_FILTER, H_FILT_SEL, TAP9);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LTI_INCORE_THR + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_THR, T2, 0xc8) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_THR, T1, 0x32);

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(3);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ARRAY_BASE + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ = 0x51e;
    *pList->pulCurrent++ = 0x1b4;
    *pList->pulCurrent++ = 0x4f;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF + hWindow->ulTntRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_GOFF, OFFSET4, 0x0) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_GOFF, OFFSET3, 0x0) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_GOFF, OFFSET2, 0xfffffffc) |
        BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_INCORE_GOFF, OFFSET1, 0xfffffffc);
#else
    BSTD_UNUSED(pList);
#endif

    BSTD_UNUSED(hWindow);
    BDBG_LEAVE(BVDC_P_Tnt_BuildInit_isr);
    return;
}

int16_t BVDC_P_Tnt_MapSharpnessForHdr_isr
    ( int16_t                      sSharpness )
{
    int16_t sImplSharpness;

    if(sSharpness <= -8192)
    {
        sImplSharpness = -32768 +(int16_t)((sSharpness + 32768) * 1 / 3);
    }
    else if(sSharpness <= 8192)
    {
        sImplSharpness = -24576 + sSharpness + 8192;
    }
    else if(sSharpness < 32767)
    {
        sImplSharpness = -8192 + (int16_t)((sSharpness - 8192) * 5 / 3);
    }
    else
    {
        sImplSharpness = 32767;
    }
    return sImplSharpness;
}

void BVDC_P_Tnt_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BVDC_P_Window_Info    *pCurInfo;
    int16_t sImplSharpness;

    BDBG_ENTER(BVDC_P_Tnt_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;

    if(pCurInfo->bSharpnessEnable)
    {
        bool bIsInputSd;
        bool bIsOutputSd;
        uint32_t ulCtiHGain;

#if (BVDC_P_SUPPORT_TNT_VER == 6)
        uint32_t aulLPeakGain[4];
        uint32_t ulLPeakGoffLow;
        uint32_t ulLPeakGoffHigh;
        uint32_t ulLPeakIncoreThrLow;
        uint32_t aulPeakIncoreThrDivLow[3];
        uint32_t ulLPeakIncoreThrHigh;
        uint32_t ulCoreLevel;
        uint32_t ulGainA, ulGainB;
        uint32_t ulLtiGain;
#endif

        /* Treat both SD and ED as SD for TNT settings */
        bIsInputSd = VIDEO_FORMAT_IS_SD(pCurInfo->hSource->stCurInfo.pFmtInfo->eVideoFmt) ||
            VIDEO_FORMAT_IS_ED(pCurInfo->hSource->stCurInfo.pFmtInfo->eVideoFmt);
        bIsOutputSd = VIDEO_FORMAT_IS_SD(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) ||
            VIDEO_FORMAT_IS_ED(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt);
        BDBG_MSG(("Building TNT RUL %s In %s Out",
            bIsInputSd ? "SD" : "HD", bIsOutputSd ? "SD" : "HD"));

        if(hWindow->bPqNcl)
        {
            sImplSharpness = BVDC_P_Tnt_MapSharpnessForHdr_isr(pCurInfo->sSharpness);
        }
        else
        {
            sImplSharpness = pCurInfo->sSharpness;
        }
        BDBG_MSG(("sSharpness=%d && bPqNcl=%d => sImplSharpness=%d",
            pCurInfo->sSharpness, hWindow->bPqNcl, sImplSharpness));

        /* Setting up the demo mode register */
        if(pCurInfo->stSplitScreenSetting.eSharpness != BVDC_SplitScreenMode_eDisable)
        {
            uint32_t ulBoundary = hWindow->stAdjDstRect.ulWidth / 2;
            uint32_t ulDemoSide = (pCurInfo->stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eLeft) ?
                                   BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_LEFT :
                                   BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_RIGHT;

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_DEMO_SETTING + hWindow->ulTntRegOffset);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_DEMO_SETTING, DEMO_L_R, ulDemoSide) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_DEMO_SETTING, DEMO_BOUNDARY, ulBoundary) ;

            BDBG_MSG(("TNT Demo Mode: L_R = %s, BOUNDARY = %d",
                      ((ulDemoSide == BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_LEFT) ? "L" : "R"),
                      ulBoundary));
        }

        if(bIsInputSd && !bIsOutputSd)
        {
            /* SD in HD out => SD mode */
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 0, 10, 20,
                                                  &ulCtiHGain);
#if (BVDC_P_SUPPORT_TNT_VER == 6)
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulGainA);
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulGainB);
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulLtiGain);

            aulLPeakGain[0] = 0;
            aulLPeakGain[1] =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_NEG, ulGainA) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_POS, ulGainA) ;
            aulLPeakGain[2] =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_NEG, ulGainB) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_POS, ulGainB) ;
            aulLPeakGain[3] = 0;

            ulLPeakGoffLow =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET4, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET3, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET2, 0xfffffffc) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET1, 0xfffffffc);

            ulLPeakGoffHigh =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET4, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET3, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET2, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET1, 0x0 );

            ulLPeakIncoreThrLow =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW, T2, 0xc8) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW, T1, 0x32);

            aulPeakIncoreThrDivLow[0] = 0x51e;
            aulPeakIncoreThrDivLow[1] = 0x1b4;
            aulPeakIncoreThrDivLow[2] = 0x4f;

            ulLPeakIncoreThrHigh =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH, T2, 0x0) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH, T1, 0x0);

            ulCoreLevel = 0x4;
#endif
        }
        else
        {
            /* SD in SD out => HD mode */
            /* HD in SD out => HD mode */
            /* HD in HD out => HD mode */
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 0, 10, 20,
                                                  &ulCtiHGain);
#if (BVDC_P_SUPPORT_TNT_VER == 6)
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulGainA);
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulGainB);
            BVDC_P_Sharpness_Calculate_Gain_Value_isr(sImplSharpness, 4, 12, 20,
                                                  &ulLtiGain);

            aulLPeakGain[0] = 0;
            aulLPeakGain[1] = 0;
            aulLPeakGain[2] =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_NEG, ulGainA) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_POS, ulGainA) ;
            aulLPeakGain[3] =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_NEG, ulGainB) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_GAINSi, GAIN_POS, ulGainB) ;

            ulLPeakGoffLow =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET4, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET3, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET2, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW, OFFSET1, 0x0 );

            ulLPeakGoffHigh =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET4, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET3, 0x0 ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET2, 0xfffffffc ) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH, OFFSET1, 0xfffffffc );

            ulLPeakIncoreThrLow =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW, T2, 0x0) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW, T1, 0x0);

            aulPeakIncoreThrDivLow[0] = 0x0;
            aulPeakIncoreThrDivLow[1] = 0x0;
            aulPeakIncoreThrDivLow[2] = 0x0;

            ulLPeakIncoreThrHigh =
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH, T2, 0xc8) |
                BCHP_FIELD_DATA(TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH, T1, 0x32);

            ulCoreLevel = 0x4;
#endif
        }

#if (BVDC_P_SUPPORT_TNT_VER == 6)
        BDBG_MSG(("GainA=0x%x, GainB=0x%x", ulGainA, ulGainB));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(4);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_ARRAY_BASE + hWindow->ulTntRegOffset);
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)&aulLPeakGain, 4 * sizeof(uint32_t));
        pList->pulCurrent += 4;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ = ulLPeakGoffLow;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ = ulLPeakGoffHigh;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ = ulLPeakIncoreThrLow;

        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(3);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ARRAY_BASE + hWindow->ulTntRegOffset);
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)&aulPeakIncoreThrDivLow, 3 * sizeof(uint32_t));
        pList->pulCurrent += 3;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ = ulLPeakIncoreThrHigh;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_LTI_CONTROL + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_LTI_CONTROL, LTI_INCORE_EN, ENABLE)|
            BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_CONTROL, GAIN,          ulLtiGain)  |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_CONTROL, HAVOID,        0xc)   |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_LTI_CONTROL, CORE_LEVEL,    ulCoreLevel);
#endif

        BDBG_MSG(("CtiHGain=0x%x", ulCtiHGain));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_CTI_H + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_CTI_H, FILT_SEL,   TAP7) |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_CTI_H, GAIN,       ulCtiHGain) |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_CTI_H, CORE_LEVEL, 0xa);
    }

    BDBG_LEAVE(BVDC_P_Tnt_BuildRul_isr);
    return;
}

void BVDC_P_Tnt_BuildVysncRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BVDC_P_Window_Info    *pCurInfo;
    bool bEnable;

    BDBG_ENTER(BVDC_P_Tnt_BuildVysncRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* Get current pointer to RUL */
    pCurInfo = &hWindow->stCurInfo;
    bEnable = pCurInfo->bSharpnessEnable;

#if BVDC_P_DBV_SUPPORT && (BVDC_DBV_MODE_BVN_CONFORM)
    if(BVDC_P_CMP_DBV_MODE(hWindow->hCompositor)) bEnable = false;
#endif
    if(bEnable)
    {
        /* TOP Control */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_TOP_CONTROL + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, UPDATE_SEL, NORMAL) |
            ((pCurInfo->stSplitScreenSetting.eSharpness == BVDC_SplitScreenMode_eDisable) ?
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, DEMO_MODE, DISABLE) :
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, DEMO_MODE, ENABLE)) |
#if (BVDC_P_SUPPORT_TNT_VER == 6)
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, LPEAK_INCORE_EN, ENABLE) |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_TOP_CONTROL, H_WINDOW, 2) |
#endif
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, MASTER_EN, ENABLE);
    }
    else
    {
        /* Bypass TNT */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_TNT_CMP_0_V0_TOP_CONTROL + hWindow->ulTntRegOffset);
        *pList->pulCurrent++ =
#if (BVDC_P_SUPPORT_TNT_VER == 6)
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, LPEAK_INCORE_EN, DISABLE) |
            BCHP_FIELD_DATA(TNT_CMP_0_V0_TOP_CONTROL, H_WINDOW,        2      ) |
#endif
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, DEMO_MODE,       DISABLE) |
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, UPDATE_SEL,      NORMAL ) |
            BCHP_FIELD_ENUM(TNT_CMP_0_V0_TOP_CONTROL, MASTER_EN,       DISABLE);
    }

    BDBG_LEAVE(BVDC_P_Tnt_BuildVysncRul_isr);
    return;
}
#else  /* BVDC_P_SUPPORT_TNT == 0*/
void BVDC_P_Tnt_BuildInit_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pList);
}
void BVDC_P_Tnt_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pList);
}

void BVDC_P_Tnt_BuildVysncRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList )
{
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pList);
}
#endif /* BVDC_P_SUPPORT_TNT */

/* End of File */
