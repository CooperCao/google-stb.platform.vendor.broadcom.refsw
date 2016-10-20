/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/


#include "bhdm.h"
#include "../common/bhdm_priv.h"
#include "bchp_dvp_ht.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BHDM_CONFIG_MHL_SUPPORT
#include "bchp_clkgen.h"
#endif

BDBG_MODULE(BHDM_PRIV) ;

/* For boot loader usage */

static BERR_Code BHDM_P_ConfigurePreemphasis(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
    BERR_Code rc = BERR_SUCCESS ;
    BHDM_PreEmphasis_Configuration
        stCurPreEmphasisConfig,
        stNewPreEmphasisConfig;
    uint8_t index ;
    uint32_t tmdsRate ;
    bool validTmdsRate = false ;
    BHDM_TmdsPreEmphasisRegisters *PreEmphasisRegisters ;

    BDBG_ENTER(BHDM_P_ConfigurePreemphasis) ;

    rc = BHDM_TMDS_P_VideoFormatSettingsToTmdsRate(hHDMI,
        NewHdmiSettings->eInputVideoFmt, &NewHdmiSettings->stVideoSettings,
        &tmdsRate) ;
    if (rc)
    {
        BDBG_WRN(("No Configuration change; Unable to determine TMDS Rate from video settings")) ;
        rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
        goto done;
    }


    tmdsRate = tmdsRate * 1000000 ;

    for (index = 0 ; index < BHDM_TMDS_RANGES ; index++)
    {
        if ((tmdsRate >= hHDMI->TmdsPreEmphasisRegisters[index].MinTmdsRate)
        && (tmdsRate <= hHDMI->TmdsPreEmphasisRegisters[index].MaxTmdsRate))
        {
            PreEmphasisRegisters = &hHDMI->TmdsPreEmphasisRegisters[index] ;

            validTmdsRate = true ;
            break ;
        }
    }

    if (!validTmdsRate)
    {
        BDBG_WRN(("No Configuration change; Unknown TMDS Rate %d", tmdsRate)) ;
        rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
        goto done ;
    }


    rc = BHDM_P_GetPreEmphasisConfiguration(hHDMI, &stCurPreEmphasisConfig);
    if (rc) {rc = BERR_TRACE(rc);}

    /* copy current PreEmphasis settings to new PreEmphasis settings
        and then update based on New HDMI Settings */

    BKNI_Memcpy(&stNewPreEmphasisConfig, &stCurPreEmphasisConfig,
        sizeof(BHDM_PreEmphasis_Configuration)) ;

    stNewPreEmphasisConfig.uiPreEmphasis_Ch0 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_0, HDMI_TX_PHY_CTL_0, PREEMP_0) ;
    stNewPreEmphasisConfig.uiResSelData0 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_1, HDMI_TX_PHY_CTL_1, RES_SEL_DATA0) ;
    stNewPreEmphasisConfig.uiTermResSelData0 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_2, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA0) ;

    stNewPreEmphasisConfig.uiPreEmphasis_Ch1 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_0, HDMI_TX_PHY_CTL_0, PREEMP_1) ;
    stNewPreEmphasisConfig.uiResSelData1 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_1, HDMI_TX_PHY_CTL_1, RES_SEL_DATA1) ;
    stNewPreEmphasisConfig.uiTermResSelData1 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_2, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA1) ;

    stNewPreEmphasisConfig.uiPreEmphasis_Ch2 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_0, HDMI_TX_PHY_CTL_0, PREEMP_2) ;
    stNewPreEmphasisConfig.uiResSelData2 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_1, HDMI_TX_PHY_CTL_1, RES_SEL_DATA2) ;
    stNewPreEmphasisConfig.uiTermResSelData2 =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_2, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA2) ;

    stNewPreEmphasisConfig.uiPreEmphasis_CK =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_0, HDMI_TX_PHY_CTL_0, PREEMP_CK) ;
    stNewPreEmphasisConfig.uiResSelDataCK =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_1, HDMI_TX_PHY_CTL_1, RES_SEL_CK) ;
    stNewPreEmphasisConfig.uiTermResSelDataCK =
        BCHP_GET_FIELD_DATA(PreEmphasisRegisters->HDMI_TX_PHY_CTL_2, HDMI_TX_PHY_CTL_2, TERM_RES_SELCK) ;


    {
        static const char msgFormat[] = "%s     (%#02x / %#02x)        %#02x      %#02x      %#02x" ;

        BDBG_MSG(("Using TMDS Range: %d -- %d ; Use PreEmphasisTable values at Index %d",
            PreEmphasisRegisters->MinTmdsRate, PreEmphasisRegisters->MaxTmdsRate, index)) ;
        BDBG_MSG(("Chn (PreEmphasis / Main) PREEMP   RES_SEL   TERM_RES_SEL")) ;
        BDBG_MSG((msgFormat, "Ch0",
            (stNewPreEmphasisConfig.uiPreEmphasis_Ch0 & 0xC0) >> 5,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch0 & 0x3F,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch0,
            stNewPreEmphasisConfig.uiResSelData0, stNewPreEmphasisConfig.uiTermResSelData0)) ;

        BDBG_MSG((msgFormat, "Ch1",
            (stNewPreEmphasisConfig.uiPreEmphasis_Ch1 & 0xC0) >> 5,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch1 & 0x3F,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch1,
            stNewPreEmphasisConfig.uiResSelData1, stNewPreEmphasisConfig.uiTermResSelData1)) ;

        BDBG_MSG((msgFormat, "Ch2",
            (stNewPreEmphasisConfig.uiPreEmphasis_Ch2 & 0xC0) >> 5,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch2 & 0x3F,
            stNewPreEmphasisConfig.uiPreEmphasis_Ch2,
            stNewPreEmphasisConfig.uiResSelData2, stNewPreEmphasisConfig.uiTermResSelData2)) ;

        BDBG_MSG((msgFormat, "Clk",
            (stNewPreEmphasisConfig.uiPreEmphasis_CK & 0xC0) >> 5,
            stNewPreEmphasisConfig.uiPreEmphasis_CK & 0x3F,
            stNewPreEmphasisConfig.uiPreEmphasis_CK,
            stNewPreEmphasisConfig.uiResSelDataCK, stNewPreEmphasisConfig.uiTermResSelDataCK)) ;
        BSTD_UNUSED(msgFormat);
    }


#if BHDM_CONFIG_MHL_SUPPORT
    /* This overrides certain settings for HDMI/MHL combo PHY. */
    rc = BHDM_MHL_P_ConfigPreemphasis(hHDMI, NewHdmiSettings, &stNewPreEmphasisConfig);
#endif


    /* Update Preemphasis Configuration if there are updates */
    if (BKNI_Memcmp(&stCurPreEmphasisConfig, &stNewPreEmphasisConfig, sizeof(BHDM_PreEmphasis_Configuration)))
    {
        BHDM_P_SetPreEmphasisConfiguration(hHDMI, &stNewPreEmphasisConfig);
    }

    BDBG_LEAVE(BHDM_P_ConfigurePreemphasis);

done:
    return rc ;
}


static BERR_Code BHDM_P_ConfigurePixelEncoding(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
    BERR_Code rc = BERR_SUCCESS ;

#if BHDM_CONFIG_HAS_HDCP22
    BREG_Handle hRegister ;
    uint32_t Register ;

    uint8_t
        pixelSelect, pixelMap,
        videoOutSel0, videoOutSel1,
        videoOutSel2, videoOutSel3,
        videoOutSel4, videoOutSel5 ;

    BDBG_ENTER(BHDM_P_ConfigurePixelEncoding) ;
    hRegister = hHDMI->hRegister ;

    if (NewHdmiSettings->stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr422)
    {
#ifdef BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_FORMAT_422_Legacy
        pixelSelect = BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_FORMAT_422_Legacy ;
#else
        pixelSelect = 1 ;
#endif
        pixelMap = 0 ;
    }
    else if (NewHdmiSettings->stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr420)
    {
#ifdef BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_FORMAT_420
        pixelSelect = BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_FORMAT_420 ;
#else
        pixelSelect = 0 ;
#endif
        pixelMap = 1 ;
    }
    else /* keep default */
    {
        pixelSelect = BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_DEFAULT ;
        pixelMap = BCHP_DVP_HT_VEC_INTERFACE_CFG_PIXEL_MAP_DEFAULT ;
    }

    switch(NewHdmiSettings->stVideoSettings.eColorSpace)
    {
    case BAVC_Colorspace_eRGB :
    case BAVC_Colorspace_eYCbCr422 :
    case BAVC_Colorspace_eYCbCr444 :
#if BHDM_CONFIG_MHL_SUPPORT
        if (hHDMI->bMhlMode &&
            (NewHdmiSettings->eInputVideoFmt == BFMT_VideoFmt_e1080p ||
             NewHdmiSettings->eInputVideoFmt == BFMT_VideoFmt_e1080p_50Hz))
        {
            videoOutSel0 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT2_SEL_DEFAULT ;
            videoOutSel1 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT1_SEL_DEFAULT ;
            videoOutSel2 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT0_SEL_DEFAULT ;
            videoOutSel3 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT3_SEL_DEFAULT ;
            videoOutSel4 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT4_SEL_DEFAULT ;
            videoOutSel5 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT5_SEL_DEFAULT ;
            pixelSelect = BCHP_DVP_HT_VEC_INTERFACE_CFG_SEL_422_FORMAT_422_PackedPixel;
            BDBG_MSG(("Set Phy Configuration for 422 pixel encoding for MHL Packed Pixel; BAVC_Colorspace: %d",
                NewHdmiSettings->stVideoSettings.eColorSpace)) ;
        }
        else
#endif
        {
            videoOutSel0 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT0_SEL_DEFAULT ;
            videoOutSel1 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT1_SEL_DEFAULT ;
            videoOutSel2 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT2_SEL_DEFAULT ;
            videoOutSel3 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT3_SEL_DEFAULT ;
            videoOutSel4 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT4_SEL_DEFAULT ;
            videoOutSel5 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT5_SEL_DEFAULT ;
            BDBG_MSG(("Set Phy Configuration for 444/422 pixel encodings; BAVC_Colorspace: %d",
                NewHdmiSettings->stVideoSettings.eColorSpace)) ;
        }
        break;

    case BAVC_Colorspace_eYCbCr420 :
        videoOutSel0 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT0_SEL_DEFAULT ;
        videoOutSel1 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT1_SEL_DEFAULT ;
        videoOutSel2 = 4 ;
        videoOutSel3 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT3_SEL_DEFAULT ;
        videoOutSel4 = 1 ;
        videoOutSel5 = BCHP_DVP_HT_VEC_INTERFACE_XBAR_VID_OUT5_SEL_DEFAULT ;
        BDBG_MSG(("Set Phy Configuration for 420 pixel encodings")) ;
        break ;

    default :
        BDBG_ERR(("Unsupported BAVC_Colorspace %d", NewHdmiSettings->stVideoSettings.eColorSpace)) ;
        rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
        goto done ;
    }

    Register = BREG_Read32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_CFG) ;
        Register &= ~ (
              BCHP_MASK(DVP_HT_VEC_INTERFACE_CFG, SEL_422)
            | BCHP_MASK(DVP_HT_VEC_INTERFACE_CFG, PIXEL_MAP)) ;
        Register |=
              BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_CFG, SEL_422, pixelSelect)
            | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_CFG, PIXEL_MAP, pixelMap) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_CFG, Register) ;
    BDBG_MSG(("DVP HT Interface Config: %#x", Register)) ;

    Register = BREG_Read32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_XBAR) ;
        Register &= ~ (
                BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT0_SEL)
              | BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT1_SEL)
              | BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT2_SEL)
              | BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT3_SEL)
              | BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT4_SEL)
              | BCHP_MASK(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT5_SEL) ) ;
        Register |=
                BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT0_SEL, videoOutSel0)
              | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT1_SEL, videoOutSel1)
              | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT2_SEL, videoOutSel2)
              | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT3_SEL, videoOutSel3)
              | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT4_SEL, videoOutSel4)
              | BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_XBAR, VID_OUT5_SEL, videoOutSel5) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_XBAR, Register) ;

    BDBG_MSG(("DVP HT Cross Bar: %#x", Register)) ;


done:
    BDBG_LEAVE(BHDM_P_ConfigurePixelEncoding);
#else
    BSTD_UNUSED(hHDMI) ;
    BSTD_UNUSED(NewHdmiSettings) ;
#endif

    return rc;
}


BERR_Code BHDM_P_ConfigurePhy(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
    BERR_Code rc = BERR_SUCCESS ;

    BDBG_ENTER(BHDM_P_ConfigurePhy) ;

    rc = BHDM_P_ConfigurePixelEncoding(hHDMI, NewHdmiSettings) ;
    if (rc)
    {
        rc = BERR_TRACE(rc) ;
        goto done ;
    }

    /* Configure Pre-Emphasis */
    rc = BHDM_P_ConfigurePreemphasis(hHDMI, NewHdmiSettings) ;
    if (rc)
    {
        rc = BERR_TRACE(rc) ;
        goto done ;
    }

#if BHDM_HAS_HDMI_20_SUPPORT
    /****************************************************/
    /* Set scramble configration if there is a change in the Bit Clock Ratio */
    /****************************************************/
    if (hHDMI->TmdsBitClockRatioChange)
    {
        rc = BHDM_SCDC_ConfigureScrambling(hHDMI) ;
        if (rc)
        {
            rc = BERR_TRACE(rc) ;
            goto done ;
        }
    }
#endif

done:
    BDBG_LEAVE(BHDM_P_ConfigurePhy);
    return rc;
}

/******************************************************************************
BERR_Code BHDM_EnableTmdsOutput_isr
Summary: Enable (Display) TMDS Output
*******************************************************************************/
void BHDM_P_EnableTmdsData_isr(
   const BHDM_Handle hHDMI,     /* [in] HDMI handle */
   bool bEnableTmdsOutput   /* [in] boolean to enable/disable */
)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;
    uint32_t TmdsOutput ;
    uint8_t DeviceAttached ;

    BDBG_ENTER(BHDM_P_EnableTmdsData_isr) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_DVO_SUPPORT
    /* TMDS is always off when DVO is enabled */
    bEnableTmdsOutput = false ;
#endif

    if (bEnableTmdsOutput)
        TmdsOutput = 0x0 ; /* TMDS ON */
    else
        TmdsOutput = 0x1 ; /* TMDS OFF */

    BHDM_P_RxDeviceAttached_isr(hHDMI,&DeviceAttached) ;

#if BHDM_CONFIG_DEBUG_RSEN
    BDBG_WRN(("Tx%d: Configure TMDS Data %s",
        hHDMI->eCoreId, TmdsOutput ? "OFF" : "ON"));
#endif

    Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset) ;
    /* set TMDS lines to power on*/
    Register &= ~(
           BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN)
         | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN)
         | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN))  ;

    /* set TMDS data lines to requested value on/off */
    Register |=
          /* set remaining channels as requested */
         BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, TmdsOutput)
        | BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN, TmdsOutput)
        | BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, TmdsOutput);

    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset, Register) ;

    if (bEnableTmdsOutput)
    {
#if BHDM_CONFIG_MHL_SUPPORT
        /* overrides if needed */
        BHDM_MHL_P_EnableTmdsData_isr(hHDMI);
#endif

        /* take TMDS lines out of reset */
        Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL + ulOffset) ;
            Register &=
                ~( BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_CK_RESET)
                 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_2_RESET)
                 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_1_RESET)
                 | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_0_RESET))  ;

        BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL + ulOffset, Register) ;

        BHDM_MONITOR_P_StatusChanges_isr(hHDMI) ;
    }

    hHDMI->DeviceStatus.tmds.dataEnabled = bEnableTmdsOutput ;
    BDBG_LEAVE(BHDM_P_EnableTmdsData_isr) ;
}


/******************************************************************************
BERR_Code BHDM_P_EnableTmdsClock_isr
Summary: Enable/Disable  TMDS Clock
*******************************************************************************/
void BHDM_P_EnableTmdsClock_isr(
   const BHDM_Handle hHDMI,     /* [in] HDMI handle */
   bool bEnableTmdsClock    /* [in] boolean to enable/disable */
)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;
    uint32_t TmdsOutput ;

    BDBG_ENTER(BHDM_P_EnableTmdsClock_isr) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_DVO_SUPPORT
    /* TMDS is always off when DVO is enabled */
    bEnableTmdsClock = false ;
#endif

    if (bEnableTmdsClock)
        TmdsOutput = 0x0 ; /* TMDS ON */
    else
        TmdsOutput = 0x1 ; /* TMDS OFF */

#if BHDM_CONFIG_DEBUG_RSEN
    BDBG_WRN(("Tx%d: Configure TMDS Clock %s", hHDMI->eCoreId,
        TmdsOutput ? "OFF" : "ON"));
#endif


    Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset) ;
        /* set TMDS lines to power on*/
        Register &=
            ~( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN) ) ;

        /* set TMDS lines to requested value on/off */
        Register |=
              BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, TmdsOutput) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset, Register) ;


    if (bEnableTmdsClock)
    {
        /* take TMDS lines out of reset */
        Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL + ulOffset) ;
        Register &=
            ~( BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_CK_RESET)
             | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_2_RESET)
             | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_1_RESET)
             | BCHP_MASK(HDMI_TX_PHY_RESET_CTL, TX_0_RESET))  ;

        BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_RESET_CTL + ulOffset, Register) ;
    }

    hHDMI->DeviceStatus.tmds.clockEnabled = bEnableTmdsClock ;
    BDBG_LEAVE(BHDM_P_EnableTmdsClock_isr) ;
}



/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
#if !B_REFSW_MINIMAL
/******************************************************************************
BERR_Code BHDM_SetAudioMute
Summary: Implements HDMI Audio (only) mute enable/disable.
*******************************************************************************/
BERR_Code BHDM_SetAudioMute(
    const BHDM_Handle hHDMI,               /* [in] HDMI handle */
    bool bEnableAudioMute          /* [in] boolean to enable/disable */
)
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister ;

    BDBG_ENTER(BHDM_SetAudioMute) ;

#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
{
    uint32_t Register, ulOffset;
    BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    /* AudioMute valid for HDMI only */
    if  (hHDMI->DeviceSettings.eOutputFormat != BHDM_OutputFormat_eHDMIMode)
    {
        BDBG_ERR(("Tx%d: Audio Mute only applies in HDMI mode.", hHDMI->eCoreId));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
        goto done ;
    }
    Register = BREG_Read32(hRegister, BCHP_HDMI_MAI_CONFIG + ulOffset) ;
    if (bEnableAudioMute) {
        Register |= BCHP_FIELD_DATA(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO, 1) ;
    }
    else
    {
        Register &= ~BCHP_MASK(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO);
    }
    BREG_Write32(hRegister, BCHP_HDMI_MAI_CONFIG + ulOffset, Register) ;

    hHDMI->AudioMuteState = bEnableAudioMute ;
    BDBG_MSG(("Tx%d: AudioMute %d", hHDMI->eCoreId, bEnableAudioMute)) ;
}

done:
#else

    BSTD_UNUSED(hHDMI) ;
    BSTD_UNUSED(bEnableAudioMute) ;

#endif

    BDBG_LEAVE(BHDM_SetAudioMute) ;
    return rc ;
}  /* END BHDM_SetAudioMute */
#endif



/******************************************************************************
Summary:
Configure the MAI Audio Input Bus
*******************************************************************************/
void BHDM_P_ConfigureInputAudioFmt(
   const BHDM_Handle hHDMI,        /* [in] HDMI handle */
   const BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame
)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;
    uint8_t ChannelMask = 0x03 ;  /* default to 2 channels */

#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
    uint8_t DisableMai = hHDMI->AudioMuteState == true ? 1 : 0;
#endif

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    switch (stAudioInfoFrame->SpeakerAllocation)
    {
     case BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx___xx_FR__FL :
        ChannelMask = 0x03 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx__LFE_FR__FL  :
        ChannelMask = 0x07 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC___xx_FR__FL  :
        ChannelMask = 0x0B ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC__LFE_FR__FL  :
        ChannelMask = 0x0F ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx___xx_FR__FL :
        ChannelMask = 0x13 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx__LFE_FR__FL :
        ChannelMask = 0x17 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC___xx_FR__FL :
        ChannelMask = 0x1B ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC__LFE_FR__FL :
        ChannelMask = 0x1F ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx___xx_FR__FL :
        ChannelMask = 0x33 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx__LFE_FR__FL :
        ChannelMask = 0x37 ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC___xx_FR__FL :
        ChannelMask = 0x3B ;        break ;

     case BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC__LFE_FR__FL :
        ChannelMask = 0x3F ;        break ;

     case BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx___xx_FR__FL :
        ChannelMask = 0x73 ;        break ;

     case BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx__LFE_FR__FL :
        ChannelMask = 0x77 ;        break ;

     case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC___xx_FR__FL :
        ChannelMask = 0x7B ;        break ;

     case BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC__LFE_FR__FL :
        ChannelMask = 0x7F ;        break ;

    case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx__xx_FR__FL :
        ChannelMask = 0xF3 ;       break ;

    case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx_LFE_FR__FL :
        ChannelMask = 0xF7 ;       break ;

    case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC__xx_FR__FL:
        ChannelMask = 0xFB ;       break ;

     case BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC_LFE_FR__FL :
        ChannelMask = 0xFF ;        break ;

    default :
        BDBG_WRN(("Tx%d: UnSupported Speaker/Channel Mapping; %#X",
            hHDMI->eCoreId, stAudioInfoFrame->SpeakerAllocation)) ;

    }
    /*CP*  10 Configure the MAI Bus */
    /****       Set Channel Mask    */
    /* clear MAI_BIT_REVERSE bit  - reset value */
    /* set MAI_CHANNEL_MASK = 3   - reset value */

    Register = BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_BIT_REVERSE, 0)
#if BHDM_CONFIG_AUDIO_MAI_BUS_DISABLE_SUPPORT
        | BCHP_FIELD_DATA(HDMI_MAI_CONFIG, DISABLE_MAI_AUDIO, DisableMai)
#endif
        | BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_CHANNEL_MASK, 0xFF) ;
    BREG_Write32(hRegister, BCHP_HDMI_MAI_CONFIG + ulOffset, Register) ;

    /*CP*  11 Configure Audio */

    /* clear ZERO_DATA_ON_SAMPLE_FLAT       - reset value */
    /* clear AUDIO_SAMPLE_FLAT = 4'd0       - reset value */
    /* clear ZERO_DATA_ON_INACTIVE_CHANNELS - reset value */
    /* clear SAMPLE_PRESENT = 4'd0          - reset value */
    /* clear FORCE_SAMPLE_PRESENT           - reset value */
    /* clear FORCE_B_FRAME                  - reset value */
    /* clear B_FRAME = 4'd0                 - reset value */
    /* clear B_FRAME_IDENTIFIER = 4'd1                    */
    /* clear AUDIO_LAYOUT                   - reset value */
    /* clear FORCE_AUDIO_LAYOUT             - reset value */
    /* clear AUDIO_CEA_MASK = 8'd0          - reset value */
    Register =
          BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, ZERO_DATA_ON_SAMPLE_FLAT, 1)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_SAMPLE_FLAT, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, ZERO_DATA_ON_INACTIVE_CHANNELS, 1)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, SAMPLE_PRESENT, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_SAMPLE_PRESENT, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_B_FRAME, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, B_FRAME, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, B_FRAME_IDENTIFIER, 1)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_LAYOUT, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, FORCE_AUDIO_LAYOUT, 0)
        | BCHP_FIELD_DATA(HDMI_AUDIO_PACKET_CONFIG, AUDIO_CEA_MASK, ChannelMask) ;
    BREG_Write32(hRegister, BCHP_HDMI_AUDIO_PACKET_CONFIG + ulOffset, Register) ;

    BDBG_MSG(("Tx%d: Channel Mask: %#x", hHDMI->eCoreId, ChannelMask)) ;
}


#if !B_REFSW_MINIMAL
/******************************************************************************
Summary:
    Set pixel data override
*******************************************************************************/
BERR_Code BHDM_SetPixelDataOverride(
    const BHDM_Handle hHDMI,           /* [in] HDMI handle */
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset;

    uint16_t uiRed12bits   = red;
    uint16_t uiGreen12bits = green;
    uint16_t uiBlue12bits  = blue;

    BDBG_ENTER(BHDM_SetPixelDataOverride) ;
    BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;


#if BHDM_CONFIG_PIXEL_OVERRIDE_UPDATE
    /* Red */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1A, CH2);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1A, CH2, (uiRed12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset, Register);


    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2A, CH2);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2A, CH2, (uiRed12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset, Register);


    /* Green */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1A, CH1);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1A, CH1, (uiGreen12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1A + ulOffset, Register);


    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2A, CH1);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2A, CH1, (uiGreen12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2A + ulOffset, Register);


    /* Blue */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_1B, CH0);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_1B, CH0, (uiBlue12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_1B + ulOffset, Register);


    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_BAR_CFG_2B, CH0);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_BAR_CFG_2B, CH0, (uiBlue12bits << 4));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_BAR_CFG_2B + ulOffset, Register);


    /* Setup mode & Enable */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset) ;
    Register &= ~( BCHP_MASK(DVP_HT_TVG_CFG_0, PATTERN_SELECT)
                | BCHP_MASK(DVP_HT_TVG_CFG_0, TEST_MODE));
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset, Register) ;

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, PATTERN_SELECT, 4)
            | BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, TEST_MODE, 3);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset, Register);

#else
    /* Red */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10 + ulOffset) ;
    Register &=
        ~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_1_RED)
         | BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_2_RED) );
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10 + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_1_RED, (uiRed12bits << 4))
            | BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_10, DVO_0_FLAT_FIELD_2_RED, (uiRed12bits << 4)) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_10 + ulOffset, Register);

    /* Green */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11 + ulOffset) ;
    Register &=
        ~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_1_GREEN)
        | BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_2_GREEN) );
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11 + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_1_GREEN, (uiGreen12bits << 4))
            | BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_11, DVO_0_FLAT_FIELD_2_GREEN, (uiGreen12bits << 4)) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_11 + ulOffset, Register);

    /* Blue */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12 + ulOffset) ;
    Register &=
        ~( BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_1_BLUE)
        | BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_2_BLUE) );
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12 + ulOffset, Register);

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_1_BLUE, (uiBlue12bits << 4))
            | BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_12, DVO_0_FLAT_FIELD_2_BLUE, (uiBlue12bits << 4)) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_12 + ulOffset, Register);


    /* Setup mode */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13 + ulOffset) ;
    Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_13, DVO_0_GEN_TEST_MODE) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13 + ulOffset, Register) ;

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_13, DVO_0_GEN_TEST_MODE, 4);
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_13 + ulOffset, Register);


    /* Enable */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset) ;
    Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset, Register) ;

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE, 3);
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset, Register);
#endif

    BDBG_LEAVE(BHDM_SetPixelDataOverride) ;
    return rc;
}


/******************************************************************************
Summary:
    Clear pixel data override
*******************************************************************************/
BERR_Code BHDM_ClearPixelDataOverride(
    const BHDM_Handle hHDMI        /* [in] HDMI handle */
)
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset;

    BDBG_ENTER(BHDM_ClearPixelDataOverride) ;
    BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;


#if BHDM_CONFIG_PIXEL_OVERRIDE_UPDATE

    /* Disable */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset) ;
    Register &= ~ BCHP_MASK(DVP_HT_TVG_CFG_0, TEST_MODE);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset, Register) ;

    Register |= BCHP_FIELD_DATA(DVP_HT_TVG_CFG_0, TEST_MODE, 0);
    BREG_Write32(hRegister, BCHP_DVP_HT_TVG_CFG_0 + ulOffset, Register);

#else
    /* Disable */
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset) ;
    Register &= ~BCHP_MASK(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE) ;
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset, Register) ;

    Register |= BCHP_FIELD_DATA(DVP_HT_HDMI_TX_0_TDG_CFG_0, DVO_0_TEST_MODE, 0);
    BREG_Write32(hRegister, BCHP_DVP_HT_HDMI_TX_0_TDG_CFG_0 + ulOffset, Register);
#endif

    BDBG_LEAVE(BHDM_ClearPixelDataOverride) ;
    return rc;
}


/******************************************************************************
Summary:
    Wait for stable video in HDMI core a specific amount of time
*******************************************************************************/
BERR_Code BHDM_WaitForStableVideo(
    const BHDM_Handle hHDMI,            /* [in] HDMI handle */
    uint32_t stablePeriod,      /* [in] Period of time video should be stable */
    uint32_t maxWait            /* [in] Max amount of time to wait */
)
{
    BERR_Code rc = BERR_TIMEOUT;
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset;

    uint32_t waitThusFar = 0;
    uint32_t stableTime  = 0;
    uint32_t waitIncr    = 10;
    uint8_t bHPInterrupt = false;
    uint32_t driftFifoErrors = 0;
    uint32_t prevLineCount1 = 0;
    uint32_t prevLineCount2 = 0;
    uint32_t currLineCount;
    bool masterMode;

    BDBG_ENTER(BHDM_WaitForStableVideo);
    BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

    BHDM_ClearHotPlugInterrupt(hHDMI);

    BHDM_GetHdmiDataTransferMode(hHDMI, &masterMode);

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    while (waitThusFar < maxWait)
    {
        uint8_t notStable = false;

        /*
        * First, ensure video is really flowing in from the VEC
        */
        Register = BREG_Read32(hRegister, BCHP_HDMI_FORMAT_DET_7 + ulOffset) ;
        currLineCount = BCHP_GET_FIELD_DATA(Register, HDMI_FORMAT_DET_7, UUT_CURRENT_LINE_COUNT) ;
        if (currLineCount == prevLineCount1 && currLineCount == prevLineCount2)
        {
            notStable = true;
        }
        else
        {
            prevLineCount2 = prevLineCount1;
            prevLineCount1 = currLineCount;

            Register = BREG_Read32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_STATUS + ulOffset) ;

            if (Register & (BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HAP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBLANK2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBLANK1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSYNC_HIGH)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSYNC_LOW)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HFP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HSP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_HBP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VAL1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VFP_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBP_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VSPO_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VAL2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VFP_2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VBP_2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_STATUS, UPDATED_VSPO_2)))
            {
                Register = BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HAP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBLANK2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBLANK1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSYNC_HIGH)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSYNC_LOW)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HFP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HSP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_HBP)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VAL1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VFP_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBP_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VSPO_1)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VAL2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VFP_2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VBP_2)
                            | BCHP_MASK(HDMI_FORMAT_DET_UPDATE_CLEAR, CLEAR_UPDATED_VSPO_2) ;

                BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;
                BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, 0) ;
                notStable = true;
            }

            else if (masterMode == false)
            {
                /*
                * Capture (pointers) status before we read it.
                */
                Register = BREG_Read32( hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;

                Register &= ~BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;
                BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;

                Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, CAPTURE_POINTERS, 1) ;
                BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;

                Register = BREG_Read32(hRegister, BCHP_HDMI_READ_POINTERS + ulOffset) ;
                if (Register & (BCHP_MASK(HDMI_READ_POINTERS, DRIFT_UNDERFLOW)
                                | BCHP_MASK(HDMI_READ_POINTERS, DRIFT_OVERFLOW)))
                {
                    notStable = true;

                    /*
                    * Re-center the Drift FIFO if we get excessive overflow or underflow
                    * errors. There is a bug with the 76xx where the auto re-center
                    * logic (use_full, use_empty) does not work as expected in terms of
                    * clearing these errors.
                    */
                    if (++driftFifoErrors % 10 == 0)
                    {
                        Register = BREG_Read32( hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;

                        Register &= ~BCHP_MASK(HDMI_FIFO_CTL, RECENTER) ;
                        BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;

                        Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 1) ;
                        BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;
                    }
                }
            }
        }

        BKNI_Sleep(waitIncr);
        waitThusFar += waitIncr;

        if (notStable == false)
        {
            stableTime += waitIncr;
            if (stableTime >= stablePeriod)
            {
                rc = BERR_SUCCESS;
                goto done;
            }
        }
        else
        {
            stableTime = 0;
        }

        BHDM_CheckHotPlugInterrupt(hHDMI, &bHPInterrupt);
        if (bHPInterrupt == true)
            goto done;
    }

done:
    BDBG_LEAVE(BHDM_WaitForStableVideo);
    return rc;

}
#endif
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


/**********************************
**          PRIVATE FUNCTIONS
***********************************/
void BHDM_P_PowerOffPhy (const BHDM_Handle hHDMI)
{

#if BCHP_PWR_RESOURCE_HDMI_TX_PHY || BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
    if (hHDMI->phyPowered)
    {
        BDBG_MSG(("Release BCHP_PWR_RESOURCE_HDMI_TX_PHY")) ;
        BCHP_PWR_ReleaseResource(hHDMI->hChip, hHDMI->phyPwrResource[hHDMI->eCoreId]) ;
    }
#else
    BDBG_MSG(("Power Management not enabled")) ;
#endif

    hHDMI->phyPowered = false ;
}


void BHDM_P_PowerOnPhy (const BHDM_Handle hHDMI)
{
    BREG_Handle hRegister = hHDMI->hRegister ;
    uint32_t Register, ulOffset ;
    bool masterMode;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    BHDM_GetHdmiDataTransferMode(hHDMI, &masterMode);

    /* set the Post divider reset select to 0 - reset triggered by lock */
    Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_PLL_CTL_1 + ulOffset) ;
        Register &= ~ BCHP_MASK(HDMI_TX_PHY_PLL_CTL_1, POST_RST_SEL) ;
        Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_1, POST_RST_SEL, 0) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_PLL_CTL_1 + ulOffset, Register) ;

#if BHDM_CONFIG_CLOCK_STOP_SUPPORT
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_CLOCK_STOP + ulOffset);
    Register &= ~ BCHP_MASK(DVP_HT_CLOCK_STOP, PIXEL);
    BREG_Write32(hRegister, BCHP_DVP_HT_CLOCK_STOP + ulOffset, Register) ;
#endif

    Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset);
        Register &= ~(
              BCHP_MASK(HDMI_FIFO_CTL, USE_EMPTY)
            | BCHP_MASK(HDMI_FIFO_CTL, USE_FULL)) ;
        Register |=
              BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_EMPTY, 1)
            | BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, masterMode?0:1);
    BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;

    /* Power Up everything PLL, etc. */
#if BCHP_PWR_RESOURCE_HDMI_TX_PHY || BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
    if (!hHDMI->phyPowered)
    {
        BDBG_MSG(("Acquire BCHP_PWR_RESOURCE_HDMI_TX_PHY")) ;
        BCHP_PWR_AcquireResource(hHDMI->hChip, hHDMI->phyPwrResource[hHDMI->eCoreId]);
    }
#else
    BDBG_MSG(("Power Management not enabled")) ;
    /* make sure phy is left on */
    Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset);
        Register &= ~(
              BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, RNDGEN_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BG_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, LDO_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BIAS_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_LDO_PWRDN)
            | BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PHY_PWRDN)) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset, Register) ;
#endif

    /* clear the format detection registers */
    Register = 0xFFFFFFFF;
    BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;
    Register = 0;
    BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

#if BHDM_CONFIG_HAS_HDCP22
    /* make sure scrambling is disabled when powered up */
    {
        BHDM_ScrambleConfig ScrambleSettings ;

        BKNI_Memset(&ScrambleSettings, 0,  sizeof(BHDM_ScrambleConfig)) ;
        BHDM_SCDC_P_ConfigureScramblingTx(hHDMI, &ScrambleSettings) ;
    }
#endif

#if defined NEXUS_USE_7250_DGL && ((BCHP_CHIP == 7439) || (BCHP_CHIP == 7250 && BCHP_VER >= BCHP_VER_B0))
        BDBG_WRN((" 7250DGL Board,  BCHP_HDMI_TX_PHY_CHANNEL_SWAP = 0x3012  !!!")) ;
        BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CHANNEL_SWAP + ulOffset, 0x3012) ;
#endif

    hHDMI->phyPowered = true ;
    return ;
}


void BHDM_P_SetPreEmphasisMode (const BHDM_Handle hHDMI, uint8_t uValue, uint8_t uDriverAmp)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_0 + ulOffset) ;
    Register &= ~(BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_2)
              | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_1)
              | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_0)
              | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_CK));

    Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_2, uValue)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_1, uValue)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_0, uValue)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_CK, uValue) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CTL_0 + ulOffset, Register) ;

    BSTD_UNUSED(uDriverAmp);
    return;
}


BERR_Code BHDM_P_GetPreEmphasisConfiguration (
    const BHDM_Handle hHDMI,
    BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
)
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset;

    BKNI_Memset(stPreEmphasisConfig, 0, sizeof(BHDM_PreEmphasis_Configuration)) ;

    stPreEmphasisConfig->eCore = BHDM_Core28nm ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_0 + ulOffset) ;
        stPreEmphasisConfig->uiPreEmphasis_Ch2 =
            BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_2);
        stPreEmphasisConfig->uiPreEmphasis_Ch1 =
            BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_1);
        stPreEmphasisConfig->uiPreEmphasis_Ch0 =
            BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_0);
        stPreEmphasisConfig->uiPreEmphasis_CK =
            BCHP_GET_FIELD_DATA(Register,HDMI_TX_PHY_CTL_0,PREEMP_CK);

    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_1 + ulOffset) ;
        stPreEmphasisConfig->uiResSelData2 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_1, RES_SEL_DATA2) ;
        stPreEmphasisConfig->uiResSelData1 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_1, RES_SEL_DATA1) ;
        stPreEmphasisConfig->uiResSelData0 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_1, RES_SEL_DATA0) ;
        stPreEmphasisConfig->uiResSelDataCK =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_1, RES_SEL_CK) ;

    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_2 + ulOffset) ;
        stPreEmphasisConfig->uiTermResSelDataCK =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, TERM_RES_SELCK);
        stPreEmphasisConfig->uiTermResSelData0 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA0);
        stPreEmphasisConfig->uiTermResSelData1 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA1);
        stPreEmphasisConfig->uiTermResSelData2 =
            BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA2);

    return rc ;
}


BERR_Code BHDM_P_SetPreEmphasisConfiguration(
    const BHDM_Handle hHDMI,
    BHDM_PreEmphasis_Configuration *stPreEmphasisConfig)
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    /* Set Preemphasis configurations */
    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_0 + ulOffset) ;
    Register &= ~(
         BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_2)
        | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_1)
        | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_0)
        | BCHP_MASK(HDMI_TX_PHY_CTL_0, PREEMP_CK)) ;

    Register |=
         BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_2, stPreEmphasisConfig->uiPreEmphasis_Ch2)
        | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_1, stPreEmphasisConfig->uiPreEmphasis_Ch1)
        | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_0, stPreEmphasisConfig->uiPreEmphasis_Ch0)
        | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_CK, stPreEmphasisConfig->uiPreEmphasis_CK);
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CTL_0 + ulOffset, Register) ;


    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_1 + ulOffset) ;
        Register &= ~ (
              BCHP_MASK(HDMI_TX_PHY_CTL_1, RES_SEL_DATA2)
            | BCHP_MASK(HDMI_TX_PHY_CTL_1, RES_SEL_DATA1)
            | BCHP_MASK(HDMI_TX_PHY_CTL_1, RES_SEL_DATA0)
            | BCHP_MASK(HDMI_TX_PHY_CTL_1, RES_SEL_CK)) ;
        Register |=
               BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA2, stPreEmphasisConfig->uiResSelData2)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA1, stPreEmphasisConfig->uiResSelData1)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA0, stPreEmphasisConfig->uiResSelData0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_CK, stPreEmphasisConfig->uiResSelDataCK) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CTL_1 + ulOffset, Register) ;

    Register = BREG_Read32( hRegister, BCHP_HDMI_TX_PHY_CTL_2 + ulOffset) ;
        Register &= ~ (
              BCHP_MASK(HDMI_TX_PHY_CTL_2, TERM_RES_SELCK)
            | BCHP_MASK(HDMI_TX_PHY_CTL_2,TERM_RES_SELDATA0)
            | BCHP_MASK(HDMI_TX_PHY_CTL_2,TERM_RES_SELDATA1)
            | BCHP_MASK(HDMI_TX_PHY_CTL_2,TERM_RES_SELDATA2)) ;
        Register |=
               BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELCK, stPreEmphasisConfig->uiTermResSelDataCK)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA0, stPreEmphasisConfig->uiTermResSelData0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA1, stPreEmphasisConfig->uiTermResSelData1)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA2, stPreEmphasisConfig->uiTermResSelData2) ;
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CTL_2 + ulOffset, Register) ;

    return rc;
}


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
void BHDM_P_ClearHotPlugInterrupt(
   const BHDM_Handle hHDMI      /* [in] HDMI handle */
)
{
#if BHDM_CONFIG_DUAL_HPD_SUPPORT
    /* reset boolean status */
    hHDMI->hotplugInterruptFired = false;

#else
    uint32_t Register, ulOffset;

    ulOffset = hHDMI->ulOffset ;

    Register = BREG_Read32(hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL + ulOffset) ;
    Register &= ~BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS) ;
    Register |= BCHP_FIELD_DATA(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS, 1) ;
    BREG_Write32(hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL + ulOffset, Register) ;

    Register &= ~BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS) ;
    Register |= BCHP_FIELD_DATA(AON_HDMI_TX_HDMI_HOTPLUG_CONTROL, CLEAR_HOTPLUG_INT_STATUS, 0) ;
    BREG_Write32(hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_CONTROL + ulOffset, Register) ;
#endif

    return;
}


void BHDM_P_CheckHotPlugInterrupt(
    const BHDM_Handle hHDMI,         /* [in] HDMI handle */
    uint8_t *bHotPlugInterrupt  /* [out] Interrupt asserted or not */
)
{
#if BHDM_CONFIG_DUAL_HPD_SUPPORT
    *bHotPlugInterrupt = hHDMI->hotplugInterruptFired;
#else
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;

    Register = BREG_Read32(hRegister, BCHP_AON_HDMI_TX_HDMI_HOTPLUG_STATUS + ulOffset) ;

    if (Register & BCHP_MASK(AON_HDMI_TX_HDMI_HOTPLUG_STATUS, HOTPLUG_INT_STATUS))
        *bHotPlugInterrupt = true;
    else
        *bHotPlugInterrupt = false;
#endif

    return;
}

#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


void BHDM_P_RxDeviceAttached_isr(
    const BHDM_Handle hHDMI,         /* [in] HDMI handle */
    uint8_t *DeviceAttached /* [out] Device Attached Status  */
)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;

    BDBG_ENTER(BHDM_P_RxDeviceAttached_isr) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    Register = BREG_Read32(hRegister, BCHP_HDMI_HOTPLUG_STATUS + ulOffset) ;
    *DeviceAttached =
        BCHP_GET_FIELD_DATA(Register, HDMI_HOTPLUG_STATUS, HOTPLUG_STATUS) ;

    BDBG_LEAVE(BHDM_P_RxDeviceAttached_isr) ;
    return ;
}


#if BHDM_CONFIG_HAS_HDCP22

void BHDM_P_ResetHDCPI2C_isr(const BHDM_Handle hHDMI)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;
    uint8_t i=0;
    uint32_t regAddress = BCHP_HDMI_REG_START;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

    /***********************************
    ** Save register values before resetting **
    ***********************************/
    {
        /* clear buffer before starting */
        BKNI_Memset(hHDMI->hdmiRegBuff, 0, hdmiRegBuffSize);
        i=0;
        /* To pass HDCP 1.4 1B-01 on Simplay tester (FW 2.0.1), we need to skip registers from HDMI_BKSV0..HDMI_HDCP_KEY_2*/
        regAddress = BCHP_HDMI_HDCP_CTL;
        while (regAddress <=  BCHP_HDMI_HDCP2TX_CTRL0)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->hdmiRegBuff[i++] = Register;
            regAddress+=4;
        }

        /* clear buffer before starting */
        BKNI_Memset(hHDMI->autoI2cRegBuff, 0, autoI2cRegBuffSize);
        i=0;
        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH0_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH0_STAT)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->autoI2cRegBuff[i++] = Register;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH1_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH1_STAT)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->autoI2cRegBuff[i++] = Register;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH2_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH2_STAT)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->autoI2cRegBuff[i++] = Register;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_CFG)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->autoI2cRegBuff[i++] = Register;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_WD;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_RD_1)
        {
            Register = BREG_Read32(hRegister, regAddress + ulOffset);
            hHDMI->autoI2cRegBuff[i++] = Register;
            regAddress+=4;
        }

        Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH3_STAT + ulOffset);
        hHDMI->autoI2cRegBuff[i++] = Register;

        Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset);
        hHDMI->autoI2cRegBuff[i++] = Register;

        /* To pass 1B-07 on QD980, do not save/restore registers from BCHP_HDMI_TX_AUTO_I2C_TRANSACTION_DONE_STAT_CLEAR to BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_FSM_DEBUG */
    }


    /******************************
    ** All the register values are saved.
    ** Now reset the cores
    ******************************/
    Register = BREG_Read32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset);
    Register |= 0x000000C0; /* write 1 to HDCP2_I2C - a private field */
    BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register);

    Register &= 0x0000013F; /* write 0 to HDCP2_I2C - a private field */
    BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register) ;


    /********************************
    ** Reload register values after reset  **
    *********************************/
    {
        i=0;
        regAddress = BCHP_HDMI_HDCP_CTL;
        while (regAddress <=  BCHP_HDMI_HDCP2TX_CTRL0)
        {
            Register = hHDMI->hdmiRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        i=0;
        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH0_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH0_STAT)
        {
            Register = hHDMI->autoI2cRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH1_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH1_STAT)
        {
            Register = hHDMI->autoI2cRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH2_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH2_STAT)
        {
            Register = hHDMI->autoI2cRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_CFG;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_CFG)
        {
            Register = hHDMI->autoI2cRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_WD;
        while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_RD_1)
        {
            Register = hHDMI->autoI2cRegBuff[i++];
            BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
            regAddress+=4;
        }

        Register = hHDMI->autoI2cRegBuff[i++];
        BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH3_STAT + ulOffset, Register) ;

        Register = hHDMI->autoI2cRegBuff[i++];
        BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset, Register);

    }
}
#endif
