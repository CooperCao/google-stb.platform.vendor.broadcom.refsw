/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#ifndef _BSRF_G1_H__
#define _BSRF_G1_H__


/* define device configuration parameters */
/* TBD... */

/* define channel configuration parameters */
/* TBD... */


#ifdef BSRF_EXCLUDE_API_TABLE

#define BSRF_Open                            BSRF_g1_P_Open
#define BSRF_Close                           BSRF_g1_P_Close
#define BSRF_GetTotalChannels                BSRF_g1_P_GetTotalChannels
#define BSRF_GetChannelDefaultSettings       BSRF_g1_GetChannelDefaultSettings
#define BSRF_OpenChannel                     BSRF_g1_P_OpenChannel
#define BSRF_CloseChannel                    BSRF_g1_P_CloseChannel
#define BSRF_Reset                           BSRF_g1_P_Reset
#define BSRF_GetVersion                      BSRF_g1_P_GetVersion
#define BSRF_g1_P_PowerUp                    BSRF_g1_P_PowerUp
#define BSRF_g1_P_PowerDown                  BSRF_g1_P_PowerDown
#define BSRF_g1_P_FreezeRfAgc                BSRF_g1_P_FreezeRfAgc
#define BSRF_g1_P_UnfreezeRfAgc              BSRF_g1_P_UnfreezeRfAgc
#define BSRF_g1_P_WriteRfAgc                 BSRF_g1_P_WriteRfAgc
#define BSRF_g1_P_ReadRfAgc                  BSRF_g1_P_ReadRfAgc
#define BSRF_g1_P_GetInputPower              BSRF_g1_P_GetInputPower
#define BSRF_g1_P_SetRfAgcSettings           BSRF_g1_P_SetRfAgcSettings
#define BSRF_g1_P_GetRfAgcSettings           BSRF_g1_P_GetRfAgcSettings
#define BSRF_g1_P_SetFastDecayGainThreshold  BSRF_g1_P_SetFastDecayGainThreshold
#define BSRF_g1_P_GetFastDecayGainThreshold  BSRF_g1_P_GetFastDecayGainThreshold
#define BSRF_g1_P_SetAntennaOverThreshold    BSRF_g1_P_SetAntennaOverThreshold
#define BSRF_g1_P_GetAntennaOverThreshold    BSRF_g1_P_GetAntennaOverThreshold
#define BSRF_g1_P_SetAntennaDetectThreshold  BSRF_g1_P_SetAntennaDetectThreshold
#define BSRF_g1_P_GetAntennaDetectThreshold  BSRF_g1_P_GetAntennaDetectThreshold
#define BSRF_g1_P_GetAntennaStatus           BSRF_g1_P_GetAntennaStatus
#define BSRF_g1_P_Tune                       BSRF_g1_P_Tune
#define BSRF_g1_P_GetTunerStatus             BSRF_g1_P_GetTunerStatus
#define BSRF_g1_P_ResetClipCount             BSRF_g1_P_ResetClipCount
#define BSRF_g1_P_GetClipCount               BSRF_g1_P_GetClipCount
#define BSRF_g1_P_ConfigTestMode             BSRF_g1_P_ConfigTestMode
#define BSRF_g1_P_ConfigOutput               BSRF_g1_P_ConfigOutput
#define BSRF_g1_P_ConfigTestDac              BSRF_g1_P_ConfigTestDac
#define BSRF_g1_P_EnableTestDac              BSRF_g1_P_EnableTestDac
#define BSRF_g1_P_DisableTestDac             BSRF_g1_P_DisableTestDac
#define BSRF_g1_P_EnableTestDacTone          BSRF_g1_P_EnableTestDacTone


#endif


/***************************************************************************
Summary:
   This function returns the default settings for g1 module.
Description:
   This function returns the default settings for g1 module.
Returns:
   BERR_Code
See Also:
   BSRF_Open()
****************************************************************************/
BERR_Code BSRF_g1_GetDefaultSettings(
   BSRF_Settings *pDefSettings   /* [out] default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for g2 channel device.
Description:
   This function returns the default settings for g2 channel device.
Returns:
   BERR_Code
See Also:
   BSRF_Open()
****************************************************************************/
BERR_Code BSRF_g1_GetChannelDefaultSettings(
   BSRF_Handle h,                         /* [in] BSRF handle */
   uint8_t chanNum,                       /* [in] channel number */
   BSRF_ChannelSettings *pChnDefSettings  /* [out] default channel settings */
);


#endif /* BSRF_G1_H__ */
