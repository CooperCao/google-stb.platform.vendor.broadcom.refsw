/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BWFE_G2_H__
#define _BWFE_G2_H__


#ifdef BWFE_EXCLUDE_API_TABLE

#define BWFE_Open                      BWFE_g2_P_Open
#define BWFE_Close                     BWFE_g2_P_Close
#define BWFE_GetTotalChannels          BWFE_g2_P_GetTotalChannels
#define BWFE_GetTotalRefChannels       BWFE_g2_P_GetTotalRefChannels
#define BWFE_GetChannelDefaultSettings BWFE_g2_GetChannelDefaultSettings
#define BWFE_OpenChannel               BWFE_g2_P_OpenChannel
#define BWFE_CloseChannel              BWFE_g2_P_CloseChannel
#define BWFE_Reset                     BWFE_g2_P_Reset
#define BWFE_GetVersion                BWFE_g2_P_GetVersion
#define BWFE_IsInputEnabled            BWFE_g2_P_IsInputEnabled
#define BWFE_EnableInput               BWFE_g2_P_EnableInput
#define BWFE_DisableInput              BWFE_g2_P_DisableInput
#define BWFE_GetChannelStatus          BWFE_g2_P_GetChannelStatus
#define BWFE_GetLicTaps                BWFE_g2_P_GetLicTaps
#define BWFE_GetEqTaps                 BWFE_g2_P_GetEqTaps
#define BWFE_FreezeRfAgc               BWFE_g2_P_FreezeRfAgc
#define BWFE_UnfreezeRfAgc             BWFE_g2_P_UnfreezeRfAgc
#define BWFE_RunLaneCorrection         BWFE_g2_P_RunLaneCorrection
#define BWFE_CalibrateINL              BWFE_g2_P_CalibrateINL
#define BWFE_EqualizePipeline          BWFE_g2_P_EqualizePipeline
#define BWFE_SetAdcSampleFreq          BWFE_g2_P_SetAdcSampleFreq
#define BWFE_GetAdcSampleFreq          BWFE_g2_P_GetAdcSampleFreq
#define BWFE_ReadConfig                BWFE_g2_P_ReadConfig
#define BWFE_WriteConfig               BWFE_g2_P_WriteConfig
#define BWFE_ResetDgsLut               BWFE_g2_P_ResetDgsLut
#define BWFE_ResetEqTaps               BWFE_g2_P_ResetEqTaps
#define BWFE_ResetLicTaps              BWFE_g2_P_ResetLicTaps
#define BWFE_CancelDCOffset            BWFE_g2_P_CancelDCOffset
#define BWFE_GetWfeReadyEventHandle    BWFE_g2_P_GetWfeReadyEventHandle
#define BWFE_CalibrateAnalogDelay      BWFE_g2_P_CalibrateAnalogDelay
#define BWFE_GetAnalogDelay            BWFE_g2_P_GetAnalogDelay
#define BWFE_GetSaDoneEventHandle      BWFE_g2_P_GetSaDoneEventHandle
#define BWFE_ScanSpectrum              BWFE_g2_P_ScanSpectrum
#define BWFE_GetSaSamples              BWFE_g2_P_GetSaSamples
#define BWFE_MirrorEquRefTaps          BWFE_g2_P_MirrorEquRefTaps
#define BWFE_CalibrateAdcPhase         BWFE_g2_P_CalibrateAdcPhase

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
#define BWFE_G2_CONFIG_RF_AGC_THRESH               0x0000   /* rf agc threshold */
#define BWFE_G2_CONFIG_LEN_RF_AGC_THRESH           2
#define BWFE_G2_CONFIG_RF_AGC_K1                   0x0001   /* rf agc k1 */
#define BWFE_G2_CONFIG_LEN_RF_AGC_K1               1
#define BWFE_G2_CONFIG_RF_AGC_INT                  0x0002   /* rf agc integrator */
#define BWFE_G2_CONFIG_LEN_RF_AGC_INT              4
#define BWFE_G2_CONFIG_EQU_RUNTIME_MS              0x0003   /* digisum equalization run time */
#define BWFE_G2_CONFIG_LEN_EQU_RUNTIME_MS          2
#define BWFE_G2_CONFIG_EQU_MAIN_MU1                0x0004   /* digisum equalization mu1 */
#define BWFE_G2_CONFIG_LEN_EQU_MAIN_MU1            1
#define BWFE_G2_CONFIG_EQU_MAIN_MU2                0x0005   /* digisum equalization mu2 */
#define BWFE_G2_CONFIG_LEN_EQU_MAIN_MU2            1
#define BWFE_G2_CONFIG_EQU_MU1                     0x0006   /* digisum equalization mu1 */
#define BWFE_G2_CONFIG_LEN_EQU_MU1                 1
#define BWFE_G2_CONFIG_EQU_MU2                     0x0007   /* digisum equalization mu2 */
#define BWFE_G2_CONFIG_LEN_EQU_MU2                 1
#define BWFE_G2_CONFIG_EQU_TAP_MASK                0x0008   /* digisum equalization tap mask */
#define BWFE_G2_CONFIG_LEN_EQU_TAP_MASK            1
#define BWFE_G2_CONFIG_DGS_CLIP_SLICE_SELECT       0x0009   /* dgs clip slice select */
#define BWFE_G2_CONFIG_LEN_DGS_CLIP_SLICE_SELECT   1
#define BWFE_G2_CONFIG_DGS_CLIP_CTL_PI             0x000A   /* dgs clip counter control for ping lane */
#define BWFE_G2_CONFIG_LEN_DGS_CLIP_CTL_PI         4
#define BWFE_G2_CONFIG_DGS_CLIP_CTL_PO             0x000B   /* dgs clip counter control for pong lane */
#define BWFE_G2_CONFIG_LEN_DGS_CLIP_CTL_PO         4
#define BWFE_G2_CONFIG_DGS_CLIP_COUNT_PI           0x000C   /* dgs clip counter for ping lane */
#define BWFE_G2_CONFIG_LEN_DGS_CLIP_COUNT_PI       4
#define BWFE_G2_CONFIG_DGS_CLIP_COUNT_PO           0x000D   /* dgs clip counter for pong lane */
#define BWFE_G2_CONFIG_LEN_DGS_CLIP_COUNT_PO       4
#define BWFE_G2_CONFIG_EQU_MDAC_ENABLE             0x000E   /* digisum equalization mdac enable */
#define BWFE_G2_CONFIG_LEN_EQU_MDAC_ENABLE         1
#define BWFE_G2_CONFIG_CORR_INPUT_SELECT           0x000F   /* correlator input select */
#define BWFE_G2_CONFIG_LEN_CORR_INPUT_SELECT       1
#define BWFE_G2_CONFIG_RF_AGC_BETA                 0x0010   /* rf agc beta */
#define BWFE_G2_CONFIG_LEN_RF_AGC_BETA             1
#define BWFE_G2_CONFIG_NR_AGC_K1                   0x0011   /* lic2 agc k1 */
#define BWFE_G2_CONFIG_LEN_NR_AGC_K1               1
#define BWFE_G2_CONFIG_CORR_DELAY_BETA             0x0012   /* corr delay tracking beta */
#define BWFE_G2_CONFIG_LEN_CORR_DELAY_BETA         1
#define BWFE_G2_CONFIG_CORR_DPM_DEBUG              0x0013   /* corr dpm debug controls */
#define BWFE_G2_CONFIG_LEN_CORR_DPM_DEBUG          1


/* interrupts */
typedef enum BWFE_g2_IntID{
   BWFE_g2_IntID_eTimer0 = 0,
   BWFE_g2_IntID_eTimer1,
   BWFE_g2_IntID_eCorrDone,
   BWFE_g2_IntID_eClipDetectPong,
   BWFE_g2_IntID_eClipDetectPing,
   BWFE_g2_MaxIntID
} BWFE_g2_IntID;


/* timers */
typedef enum BWFE_g2_TimerSelect
{
   BWFE_g2_TimerSelect_e0 = 0,
   BWFE_g2_TimerSelect_e1,
} BWFE_g2_TimerSelect;


/* correlator modes */
typedef enum BWFE_g2_CorrMode
{
   BWFE_g2_CorrMode_eSpecAnalyzer = 0,
   BWFE_g2_CorrMode_eDpmTone,
} BWFE_g2_CorrMode;


/***************************************************************************
Summary:
   This function returns the default settings for g2 module.
Description:
   This function returns the default settings for g2 module.
Returns:
   BERR_Code
See Also:
   BWFE_Open()
****************************************************************************/
BERR_Code BWFE_g2_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for g2 channel device.
Description:
   This function returns the default settings for g2 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BWFE_g2_GetChannelDefaultSettings(
   BWFE_Handle h, 
   uint8_t chanNum, 
   BWFE_ChannelSettings *pChnDefSettings
);


#endif /* BWFE_G2_H__ */

