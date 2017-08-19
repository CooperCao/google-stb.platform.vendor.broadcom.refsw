/***************************************************************************
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
 *
 * Module Description:
 *      Private module for No Macrovision support. Do not include
 * bvdc_macrovision_priv.c, if this file is included in your project.
 *
 ***************************************************************************/

#include "bstd.h"          /* standard types */
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"

BDBG_MODULE(BVDC_DISP);


/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate RamTable for display modes, which
 *  supports Macrovision (480i,PAL,480p). Should never get here otherwise!
 **************************************************************************/
const uint32_t* BVDC_P_GetRamTable_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo,
    bool                          bArib480p
)
{
    return BVDC_P_GetRamTableSub_isr(pDispInfo, bArib480p);
}

/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate ItTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetItTable_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo
)
{
    return BVDC_P_GetItTableSub_isr(pDispInfo);
}

#if !B_REFSW_MINIMAL
BERR_Code BVDC_P_ChangeMvType_isr
(
    BVDC_P_DisplayContext  *pDisplay,
    uint32_t              **ppulRul )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(ppulRul);
    return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *  {secret}
 **************************************************************************/
uint32_t BVDC_P_GetItConfig_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo
)
{
    return BVDC_P_GetItConfigSub_isr(pDispInfo);
}

#if BVDC_P_NUM_SHARED_VF
/*************************************************************************
 *  {secret}
 *  Returns SEC_VF_POS_SYNC_VALUES value
 **************************************************************************/
uint32_t BVDC_P_GetPosSyncValue_isr
(
    BVDC_P_DisplayContext  *pDisplay,
    uint32_t              **ppulRul
)
{
    uint32_t    ulVfPosSync = 0;
    BSTD_UNUSED(ppulRul);
    BSTD_UNUSED(pDisplay);

    return(ulVfPosSync);
}


/*************************************************************************
 *  {secret}
 *  Returns SEC_VF_NEG_SYNC_VALUES value
 *  The amplitude of the neg sync pulses reduces for NTSC/PAL.
 *************************************************************************/
void BVDC_P_Macrovision_GetNegSyncValue_isr
    ( BVDC_P_DisplayInfo              *pDispInfo,
      BVDC_P_Output                    eOutputColorSpace,
      bool                             bDacOutput_Green_NoSync,
      uint32_t*                        ulRegVal,
      uint32_t*                        ulRegValEx)
{
    uint32_t ulValue1, ulValue2;
    uint32_t ulValue;
    uint32_t ulValueEx;
    uint32_t ulValue0 = BVDC_P_NEG_SYNC_TIP_VALUE;

    BSTD_UNUSED(pDispInfo);

    if ((BVDC_P_Output_eSDRGB == eOutputColorSpace) && bDacOutput_Green_NoSync)
    {
        ulValue0 =
            BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_SYNC_LEVEL);
    }

    /* 525/60 CVBS/Svideo-Y outputs use 714/286 picture/sync ratio;
       525/60 YPbPr/RGB and 625/50 outputs use 700/300 picture/sync ratio */
    if((BVDC_P_Output_eYQI   == eOutputColorSpace) ||
       (BVDC_P_Output_eYQI_M == eOutputColorSpace) ||
       (BVDC_P_Output_eYUV_M == eOutputColorSpace) ||
       (BVDC_P_Output_eYUV_N == eOutputColorSpace))
    {
        ulValue1 = BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL; /* 286 mv */
    }
    else
    {
        ulValue1 = BVDC_P_DAC_OUTPUT_SYNC_LEVEL;      /* 300 mv */
    }
    ulValue2 = ulValue1;

    /* Convert from voltage to register bits */
    ulValue1 = BVDC_P_NEG_SYNC_AMPLITUDE_VALUE (ulValue1);
    ulValue2 = BVDC_P_NEG_SYNC_AMPLITUDE_VALUE (ulValue2);

    /* Format for hardware registers */
#if (BVDC_P_SUPPORT_VEC_VF_VER < 2)
    ulValue =
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE2, ulValue2) |
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE1, ulValue1) |
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE0, ulValue0) ;
    ulValueEx = 0;
#else
    ulValue =
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE1, ulValue1) |
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE0, ulValue0) ;
    ulValueEx =
        BCHP_FIELD_DATA(VF_0_NEG_SYNC_AMPLITUDE_EXTN, VALUE2, ulValue2);
#endif

    /* Return computed values */
    *ulRegVal   = ulValue;
    *ulRegValEx = ulValueEx;
}

/*************************************************************************
 *  {secret}
 *  This function is for SD RGB output only.
 *  Returns SEC_VF_FORMAT_ADDER value
 **************************************************************************/
uint32_t BVDC_P_GetFmtAdderValue_isr
(
    BVDC_P_DisplayInfo     *pDispInfo
)
{
    uint32_t ulTable[BVDC_P_VF_TABLE_SIZE];

    BVDC_P_FillVfTable_isr(
        pDispInfo, BVDC_P_Output_eSDRGB, ulTable, NULL, NULL);

    return ulTable[0];
}
#endif

/*************************************************************************
 *  {secret}
 *  This function is to validate the macrovision settings.
 **************************************************************************/
BERR_Code BVDC_P_ValidateMacrovision
(
    BVDC_P_DisplayContext   *pDisplay
)
{
    BSTD_UNUSED(pDisplay);
    return BERR_SUCCESS;
}

/* End of File */
