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
#ifndef _BWFE_G3_H__
#define _BWFE_G3_H__


#ifdef BWFE_EXCLUDE_API_TABLE

#define BWFE_Open                      BWFE_g3_P_Open
#define BWFE_Close                     BWFE_g3_P_Close
#define BWFE_GetTotalChannels          BWFE_g3_P_GetTotalChannels
#define BWFE_GetTotalRefChannels       BWFE_g3_P_GetTotalRefChannels
#define BWFE_GetChannelDefaultSettings BWFE_g3_GetChannelDefaultSettings
#define BWFE_OpenChannel               BWFE_g3_P_OpenChannel
#define BWFE_CloseChannel              BWFE_g3_P_CloseChannel
#define BWFE_Reset                     BWFE_g3_P_Reset
#define BWFE_GetVersion                BWFE_g3_P_GetVersion
#define BWFE_IsInputEnabled            BWFE_g3_P_IsInputEnabled
#define BWFE_EnableInput               BWFE_g3_P_EnableInput
#define BWFE_DisableInput              BWFE_g3_P_DisableInput
#define BWFE_GetChannelStatus          BWFE_g3_P_GetChannelStatus
#define BWFE_GetLicTaps                BWFE_g3_P_GetLicTaps
#define BWFE_GetEqTaps                 BWFE_g3_P_GetEqTaps
#define BWFE_FreezeRfAgc               BWFE_g3_P_FreezeRfAgc
#define BWFE_UnfreezeRfAgc             BWFE_g3_P_UnfreezeRfAgc
#define BWFE_RunLaneCorrection         BWFE_g3_P_RunLaneCorrection
#define BWFE_CalibrateINL              BWFE_g3_P_CalibrateINL
#define BWFE_EqualizePipeline          BWFE_g3_P_EqualizePipeline
#define BWFE_SetAdcSampleFreq          BWFE_g3_P_SetAdcSampleFreq
#define BWFE_GetAdcSampleFreq          BWFE_g3_P_GetAdcSampleFreq
#define BWFE_ReadConfig                BWFE_g3_P_GetChannelConfig
#define BWFE_WriteConfig               BWFE_g3_P_SetChannelConfig
#define BWFE_ResetDgsLut               BWFE_g3_P_ResetDgsLut
#define BWFE_ResetEqTaps               BWFE_g3_P_ResetEqTaps
#define BWFE_ResetLicTaps              BWFE_g3_P_ResetLicTaps
#define BWFE_CancelDCOffset            BWFE_g3_P_CancelDCOffset
#define BWFE_GetWfeReadyEventHandle    BWFE_g3_P_GetWfeReadyEventHandle
#define BWFE_CalibrateAnalogDelay      BWFE_g3_P_CalibrateAnalogDelay
#define BWFE_GetAnalogDelay            BWFE_g3_P_GetAnalogDelay
#define BWFE_CompensateDelay           BWFE_g3_P_CompensateDelay
#define BWFE_GetSaDoneEventHandle      BWFE_g3_P_GetSaDoneEventHandle
#define BWFE_ScanSpectrum              BWFE_g3_P_ScanSpectrum
#define BWFE_GetSaSamples              BWFE_g3_P_GetSaSamples
#define BWFE_MirrorEquRefTaps          BWFE_g3_P_MirrorEquRefTaps
#define BWFE_CalibrateAdcPhase         BWFE_g3_P_CalibrateAdcPhase
#define BWFE_CompensateDelay           BWFE_g3_P_CompensateDelay

#endif


/******************************************************************************
Summary:
   Configuration Parameter IDs
Description:
   These are the configuration parameters that can be accessed via 
   BWFE_ReadConfig() and BWFE_WriteConfig().
See Also:
   BWFE_ReadConfig(), BWFE_WriteConfig()
******************************************************************************/
#define BWFE_G3_CONFIG_RF_AGC_THRESH               0x0000   /* rf agc threshold */
#define BWFE_G3_CONFIG_RF_AGC_K1                   0x0001   /* rf agc k1 */
#define BWFE_G3_CONFIG_RF_AGC_INT                  0x0002   /* rf agc integrator */
#define BWFE_G3_CONFIG_EQU_RUNTIME_MS              0x0003   /* digisum equalization run time */
#define BWFE_G3_CONFIG_EQU_MAIN_MU1                0x0004   /* digisum equalization mu1 */
#define BWFE_G3_CONFIG_EQU_MAIN_MU2                0x0005   /* digisum equalization mu2 */
#define BWFE_G3_CONFIG_EQU_MU1                     0x0006   /* digisum equalization mu1 */
#define BWFE_G3_CONFIG_EQU_MU2                     0x0007   /* digisum equalization mu2 */
#define BWFE_G3_CONFIG_EQU_TAP_MASK                0x0008   /* digisum equalization tap mask */
#define BWFE_G3_CONFIG_DGS_CLIP_SLICE_SELECT       0x0009   /* dgs clip slice select */
#define BWFE_G3_CONFIG_DGS_CLIP_CTL_PI             0x000A   /* dgs clip counter control for ping lane */
#define BWFE_G3_CONFIG_DGS_CLIP_CTL_PO             0x000B   /* dgs clip counter control for pong lane */
#define BWFE_G3_CONFIG_DGS_CLIP_COUNT_PI           0x000C   /* dgs clip counter for ping lane */
#define BWFE_G3_CONFIG_DGS_CLIP_COUNT_PO           0x000D   /* dgs clip counter for pong lane */
#define BWFE_G3_CONFIG_EQU_MDAC_ENABLE             0x000E   /* digisum equalization mdac enable */
#define BWFE_G3_CONFIG_CORR_INPUT_SELECT           0x000F   /* correlator input select */
#define BWFE_G3_CONFIG_RF_AGC_BETA                 0x0010   /* rf agc beta */
#define BWFE_G3_CONFIG_NR_AGC_K1                   0x0011   /* lic2 agc k1 */
#define BWFE_G3_CONFIG_CORR_DELAY_BETA             0x0012   /* corr delay tracking beta */
#define BWFE_G3_CONFIG_CORR_DPM_DEBUG              0x0013   /* corr dpm debug controls */


/* interrupts */
typedef enum BWFE_g3_IntID{
   BWFE_g3_IntID_eTimer0 = 0,
   BWFE_g3_IntID_eTimer1,
   BWFE_g3_IntID_eCorrDone,
   BWFE_g3_IntID_eClipDetectPong,
   BWFE_g3_IntID_eClipDetectPing,
   BWFE_g3_MaxIntID
} BWFE_g3_IntID;


/* timers */
typedef enum BWFE_g3_TimerSelect
{
   BWFE_g3_TimerSelect_e0 = 0,
   BWFE_g3_TimerSelect_e1
} BWFE_g3_TimerSelect;


/* correlator modes */
typedef enum BWFE_g3_CorrMode
{
   BWFE_g3_CorrMode_eSpecAnalyzer = 0,
   BWFE_g3_CorrMode_eDpmTone
} BWFE_g3_CorrMode;


/***************************************************************************
Summary:
   This function returns the default settings for g3 module.
Description:
   This function returns the default settings for g3 module.
Returns:
   BERR_Code
See Also:
   BWFE_Open()
****************************************************************************/
BERR_Code BWFE_g3_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for g3 channel device.
Description:
   This function returns the default settings for g3 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BWFE_g3_GetChannelDefaultSettings(
   BWFE_Handle h, 
   uint8_t chanNum, 
   BWFE_ChannelSettings *pChnDefSettings
);


#endif /* BWFE_G3_H__ */

