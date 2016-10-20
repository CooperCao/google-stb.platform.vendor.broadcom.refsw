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
#ifndef _BSRF_G1_H__
#define _BSRF_G1_H__


/* define device configuration parameters */
/* TBD... */

/* define channel configuration parameters */
/* TBD... */


#ifdef BSRF_EXCLUDE_API_TABLE

#define BSRF_Open                      BSRF_g1_P_Open
#define BSRF_Close                     BSRF_g1_P_Close
#define BSRF_GetTotalChannels          BSRF_g1_P_GetTotalChannels
#define BSRF_GetChannelDefaultSettings BSRF_g1_GetChannelDefaultSettings
#define BSRF_OpenChannel               BSRF_g1_P_OpenChannel
#define BSRF_CloseChannel              BSRF_g1_P_CloseChannel
#define BSRF_Reset                     BSRF_g1_P_Reset
#define BSRF_GetVersion                BSRF_g1_P_GetVersion
#define BSRF_PowerUp                   BSRF_g1_P_PowerUp
#define BSRF_PowerDown                 BSRF_g1_P_PowerDown
#define BSRF_FreezeRfAgc               BSRF_g1_P_FreezeRfAgc
#define BSRF_UnfreezeRfAgc             BSRF_g1_P_UnfreezeRfAgc
#define BSRF_WriteRfAgc                BSRF_g1_P_WriteRfAgc
#define BSRF_ReadRfAgc                 BSRF_g1_P_ReadRfAgc
#define BSRF_WriteRfGain               BSRF_g1_P_WriteRfGain
#define BSRF_ReadRfGain                BSRF_g1_P_ReadRfGain
#define BSRF_GetInputPower             BSRF_g1_P_GetInputPower
#define BSRF_SetRfAgcSettings          BSRF_g1_P_SetRfAgcSettings
#define BSRF_GetRfAgcSettings          BSRF_g1_P_GetRfAgcSettings
#define BSRF_EnableFastDecayMode       BSRF_g1_P_EnableFastDecayMode
#define BSRF_SetFastDecayGainThreshold BSRF_g1_P_SetFastDecayGainThreshold
#define BSRF_GetFastDecayGainThreshold BSRF_g1_P_GetFastDecayGainThreshold
#define BSRF_SetAntennaOverThreshold   BSRF_g1_P_SetAntennaOverThreshold
#define BSRF_GetAntennaOverThreshold   BSRF_g1_P_GetAntennaOverThreshold
#define BSRF_SetAntennaDetectThreshold BSRF_g1_P_SetAntennaDetectThreshold
#define BSRF_GetAntennaDetectThreshold BSRF_g1_P_GetAntennaDetectThreshold
#define BSRF_GetAntennaStatus          BSRF_g1_P_GetAntennaStatus
#define BSRF_PowerUpAntennaSense       BSRF_g1_Ana_P_PowerUpAntennaSense
#define BSRF_PowerDownAntennaSense     BSRF_g1_Ana_P_PowerDownAntennaSense
#define BSRF_Tune                      BSRF_g1_P_Tune
#define BSRF_GetTunerStatus            BSRF_g1_P_GetTunerStatus
#define BSRF_ResetClipCount            BSRF_g1_P_ResetClipCount
#define BSRF_GetClipCount              BSRF_g1_P_GetClipCount
#define BSRF_ConfigTestMode            BSRF_g1_P_ConfigTestMode
#define BSRF_ConfigOutput              BSRF_g1_P_ConfigOutput
#define BSRF_ConfigTestDac             BSRF_g1_P_ConfigTestDac
#define BSRF_EnableTestDac             BSRF_g1_P_EnableTestDac
#define BSRF_DisableTestDac            BSRF_g1_P_DisableTestDac
#define BSRF_EnableTestDacTone         BSRF_g1_P_EnableTestDacTone
#define BSRF_RunDataCapture            BSRF_g1_P_RunDataCapture
#define BSRF_DeleteAgcLutCodes         BSRF_g1_P_DeleteAgcLutCodes
#define BSRF_ConfigOutputClockPhase    BSRF_g1_P_ConfigOutputClockPhase
#define BSRF_SetIqEqCoeff              BSRF_g1_P_SetIqEqCoeff
#define BSRF_SetIqEqSettings           BSRF_g1_P_SetIqEqSettings

#endif


/* interrupts */
typedef enum BSRF_g1_IntID{
   BSRF_g1_IntID_eAttackCountOvf = 0,
   BSRF_g1_IntID_eDecayCountOvf,
   BSRF_g1_IntID_eFsCountOvf,
   BSRF_g1_IntID_eWinDetect,
   BSRF_g1_IntID_eRampActive,
   BSRF_g1_IntID_eRampInactive,
   BSRF_g1_MaxIntID
} BSRF_g1_IntID;


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
