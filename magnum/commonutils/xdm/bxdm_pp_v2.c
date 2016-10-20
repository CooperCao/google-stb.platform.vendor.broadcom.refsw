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
 * [File Description:]
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_output.h"
#include "bxdm_pp_tsm.h"
#include "bxdm_pp_vtsm.h"
#include "bxdm_pp_callback_priv.h"
#include "bxdm_pp_dbg.h"
#include "bxdm_pp_clip.h"
#include "bxdm_pp_fic.h"
#include "bxdm_pp_jrc.h"
#include "bxdm_pp_avg.h"
#include "bxdm_pp_ver.h"
#include "bxdm_pp_dbg_fifo.h"
#include "bxdm_pp_dbg_fifo_priv.h"


BDBG_MODULE(BXDM_PPV2); /* Register software module with debug interface */
BDBG_FILE_MODULE(BXDM_PPV2);
BDBG_FILE_MODULE(BXDM_PPQM);
BDBG_FILE_MODULE(BXDM_PPDBG);

extern uint32_t BXDM_PPTMR_lutVsyncsPersSecond[];

/*
** Local function prototypes.
*/
static void BXDM_PP_S_ResetCommon_isr(
   BXDM_PictureProvider_Handle hXdmPP
   );

static void BXDM_PP_S_InitPerVsync_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo
   );

static bool BXDM_PP_S_RateMatchDetection_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_STCTrickModeDetection_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_SnapShotXvdState_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_SnapShotFirmwareState_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo
   );

static void BXDM_PP_S_SelectPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_SelectElement_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_UpdatePublicStatus_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   );

static void BXDM_PP_S_PostIsr_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture *pMFDPicture,
   bool bPrimaryPicture
   );

/* sDeltaStcLUT is a lookup table for the delta STC per vsync based on
 * the clock rate type (MPEG or DTV) and the monitor refresh rate. It
 * is calculated assuming the fractional component's denominator is 65536.
 * SW7425-3103: added support for display rates of 12.5, 14.985, 15 and 20 Hz.
 */
static const BXDM_PPFP_P_DataType sDeltaStcLUT[BXDM_PictureProvider_P_ClockRate_eMax][BXDM_PictureProvider_MonitorRefreshRate_eMax]=
{
   /* BXDM_PictureProvider_P_ClockRate_eMPEG2 (45Khz) */
   {
      {    0,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_eUnknown - NOT POSSIBLE */
      { 6006,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz */
      { 6000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz */
      { 4504, 32768},   /* BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz */
      { 4500,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e10Hz */
      { 3753, 49152},   /* BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz */
      { 3750,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e12Hz */
      { 3600,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz */
      { 3003,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz */
      { 3000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e15Hz */
      { 2252, 16384},   /* BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz */
      { 2250,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e20Hz */
      { 1876, 57344},   /* BXDM_PictureProvider_MonitorRefreshRate_e23_97 */
      { 1875,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e24 */
      { 1800,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e25 */
      { 1501, 32768},   /* BXDM_PictureProvider_MonitorRefreshRate_e29_97 */
      { 1500,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e30 */
      {  937, 32768},   /* BXDM_PictureProvider_MonitorRefreshRate_e48 */
      {  900,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e50 */
      {  750, 49152},   /* BXDM_PictureProvider_MonitorRefreshRate_e59_94 */
      {  750,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e60 */
      {  450,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e100 */
      {  375, 24576},   /* BXDM_PictureProvider_MonitorRefreshRate_e119_88 */
      {  375,     0}   /* BXDM_PictureProvider_MonitorRefreshRate_e120 */
   },

   /* BXDM_PictureProvider_P_ClockRate_eDirecTV (27 Mhz) */
   {
      {       0,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_eUnknown - NOT POSSIBLE */
      { 3603600,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz */
      { 3600000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz */
      { 2702700,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz */
      { 2700000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e10Hz */
      { 2252250,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz */
      { 2250000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e12Hz */
      { 2160000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz */
      { 1801800,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz */
      { 1800000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e15Hz */
      { 1351350,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz */
      { 1350000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e20Hz */
      { 1126125,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e23_97 */
      { 1125000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e24 */
      { 1080000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e25 */
      {  900900,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e29_97 */
      {  900000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e30 */
      {  562500,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e48 */
      {  540000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e50 */
      {  450450,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e59_94 */
      {  450000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e60 */
      {  270000,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e100 */
      {  225225,     0},   /* BXDM_PictureProvider_MonitorRefreshRate_e119_88 */
      {  225000,     0}    /* BXDM_PictureProvider_MonitorRefreshRate_e120 */
   }
}; /* end of sDeltaStcLUT[] */

#define BXDM_PictureProvider_P_FRAMERATE_UNINITIALIZED ((BAVC_FrameRateCode) 0xFFFFFFFF)
#define BXDM_PictureProvider_P_PULLDOWN_UNINITIALIZED ((BXDM_Picture_PullDown) 0xFFFFFFFF)
#define BXDM_PictureProvider_P_PTS_JITTER_LOWER_THRESHOLD 1
#define BXDM_PictureProvider_P_PTS_JITTER_UPPER_THRESHOLD 90 /* CDSTRMANA-1092, bump from 1 ms to 2 ms. */
#define BXDM_PictureProvider_P_RATE_MATCH_DETECTION_SAMPLES 30

/*
** Local function implementations
*/
static void BXDM_PP_S_ResetCommon_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_Picture_Context *pstPictureUnderEvaluation;

   BDBG_ENTER(BXDM_PP_S_ResetCommon_isr);

   /* Reset picture parameter info struct */
   BKNI_Memset( &(hXdmPP->stDMStatus.stPictureParameterInfo), 0, sizeof( BXDM_PictureProvider_Callback_PictureParameterInfo ) );

   /* SW7405-4378: reset the cached the picture parameters */
   BKNI_Memset( &(hXdmPP->stDMStatus.stCachedPictureParameters), 0, sizeof( BXDM_PictureProvider_P_PictureParameterInfo ) );

   /* Reset the decode persistent state */
   BKNI_Memset( &(hXdmPP->stDMState.stDecode), 0, sizeof( BXDM_PictureProvider_P_State_Decode ) );

   hXdmPP->stDMState.stDecode.stLatestCodedPts.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;
   hXdmPP->stDMState.stDecode.stFirstCodedPts.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;

   hXdmPP->stDMState.stDecode.stErrorPts.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;
   hXdmPP->stDMState.stDecode.eLastUnsupportedFrameRate = BXDM_PictureProvider_P_FRAMERATE_UNINITIALIZED;
   hXdmPP->stDMState.stDecode.eLastIncorrectMonitorRefreshRate = 0xFFFFFFFF;
   hXdmPP->stDMState.stDecode.eLastUnsupportedPullDown = BXDM_PictureProvider_P_PULLDOWN_UNINITIALIZED;
   hXdmPP->stDMState.stDecode.eLastSelectionModeOverride = BXDM_PictureProvider_DisplayMode_eTSM;

   hXdmPP->stDMState.stDecode.bRateMatchedLastReported = true;

   /* Reset Removal Delay */
   hXdmPP->stDMState.stDecode.uiRemovalDelay = hXdmPP->stDMConfig.uiRemovalDelay;

   /* eChannelChangeMode: Handle first picture preview feature of
    * channel change - BXVD_StartDecode() */
   hXdmPP->stDMState.stDecode.bForceDisplayFirstPPB = hXdmPP->stDMConfig.stChannelChangeSettings.bFirstPicturePreview;

   /* SW7445-2645: need to tag the first picture which passes the TSM test. */
   hXdmPP->stDMState.stDecode.eTsmPassState = BXDM_PictureProvider_P_TsmPassState_eNoneHavePassed;

   /* We set STC to invalid */
   hXdmPP->stDMConfig.bSTCValid = false;

   /* Set the current selected picture's PTS to invalid, so that we
    * don't interpolate from it */
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
   hXdmPP->stDMState.stChannel.stSelectedPicture.bValidated = false;

   BKNI_Memset( &hXdmPP->stDMState.stChannel.stPreviousPictureUnderEvaluation, 0, sizeof ( BXDM_PictureProvider_P_Picture_Context ) );
   hXdmPP->stDMState.stChannel.stPreviousPictureUnderEvaluation.stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
   hXdmPP->stDMState.stChannel.stPreviousPictureUnderEvaluation.bValidated = false;

   /* Reset the picture tag. */
   hXdmPP->stDMStatus.uiPictureTag = 0;

   /* Reset clip info structure */
   BXDM_PPCLIP_P_Reset_isr(hXdmPP);

   /* Disable clip interrupts */
   hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = false;
   hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = false;
   hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable = false;

   /* Enable the default callbacks */
   BXDM_PPCB_P_EnableAutomaticCallbacks_isr( hXdmPP );

   /*
   ** "stLastDispCodedPTS" and "stCurrentPTSInfo" are accessed outsided of the DM.
   ** For now we'll keep supporting these structures.
   ** We should be able to simply update them whenever a picture is promoted
   ** to display status.
   */
   BKNI_Memset( &(hXdmPP->stDMStatus.stCodedPTS), 0, sizeof( BXDM_PictureProvider_PTSInfo ) );
   hXdmPP->stDMStatus.stCodedPTS.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;

   BKNI_Memset( &(hXdmPP->stDMStatus.stCurrentPTS), 0, sizeof( BXDM_PictureProvider_PTSInfo ) );
   hXdmPP->stDMStatus.stCurrentPTS.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;

   BKNI_Memset( &(hXdmPP->stDMStatus.stNextPTS), 0, sizeof( BXDM_PictureProvider_PTSInfo ) );
   hXdmPP->stDMStatus.stNextPTS.ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS ;

   /* forces a STC/PTS offset callback for the first picture with a TSM pass */
   hXdmPP->stDMState.stDecode.uiStcPtsDeltaSentToApp = (uint32_t)(-1);

   /* Reset the "I" picture flag, this will be set when the first I picture passes TSM. */
   hXdmPP->stDMStatus.bIPictureFound = false;

   /* Base the "prerolling" state on the pre-roll rate.
    * If it is greater than '1', then "pre-roll" until a picture
    * passes the TSM test.
    * "pre-roll" means two things
    * - run in vsync mode
    * - use the "pre-roll" rate as a slow motion rate
    */
   if ( hXdmPP->stDMConfig.stPreRollRate.uiNumerator <  hXdmPP->stDMConfig.stPreRollRate.uiDenominator )
   {
      hXdmPP->stDMState.stDecode.bPreRolling = true;
      hXdmPP->stDMState.stDecode.uiPreRollPlaybackRate = (hXdmPP->stDMConfig.stPreRollRate.uiNumerator * BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE) / hXdmPP->stDMConfig.stPreRollRate.uiDenominator;
   }
   else
   {
      hXdmPP->stDMState.stDecode.uiPreRollPlaybackRate = BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE;
   }

   /* If "BXDM_PictureProvider_ErrorHandlingMode_ePrognostic" error handling mode is set, drop pictures
    * at start decode time until the first error free RAP picture is found.
    */
   if ( BXDM_PictureProvider_ErrorHandlingMode_ePrognostic == hXdmPP->stDMConfig.eErrorHandlingMode )
   {
      hXdmPP->stDMState.stDecode.bDropUntilNextRAP = true;
   }

   hXdmPP->stDMConfig.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;
   hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount = 0;

   /* Initialized the first element in the Picture Context Queue. */
   pstPictureUnderEvaluation = BXDM_PPQM_P_GetFirstPictureContext_isr(hXdmPP);
   pstPictureUnderEvaluation->stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
   pstPictureUnderEvaluation->bValidated = false;

   BXDM_PPJRC_P_Reset_isrsafe( hXdmPP->stDMState.stChannel.hJrc );
   BXDM_PPAVG_P_Reset_isrsafe( hXdmPP->stDMState.stChannel.hDeltaSTCAvg );
   if ( (BAVC_VideoCompressionStd_eMPEG2DTV == hXdmPP->stDMConfig.eProtocol)
        || (BAVC_VideoCompressionStd_eMPEG2_DSS_PES == hXdmPP->stDMConfig.eProtocol)
      )
   {
      BXDM_PPJRC_P_SetLowerThreshold_isrsafe(
               hXdmPP->stDMState.stChannel.hJrc,
               BXDM_PictureProvider_P_PTS_JITTER_LOWER_THRESHOLD*600);

      BXDM_PPJRC_P_SetUpperThreshold_isrsafe(
               hXdmPP->stDMState.stChannel.hJrc,
               BXDM_PictureProvider_P_PTS_JITTER_UPPER_THRESHOLD*600);
   }
   else
   {
      BXDM_PPJRC_P_SetLowerThreshold_isrsafe(
               hXdmPP->stDMState.stChannel.hJrc,
               BXDM_PictureProvider_P_PTS_JITTER_LOWER_THRESHOLD);

      BXDM_PPJRC_P_SetUpperThreshold_isrsafe(
               hXdmPP->stDMState.stChannel.hJrc,
               BXDM_PictureProvider_P_PTS_JITTER_UPPER_THRESHOLD);
   }

   /* SW7405-4736: To force the debug messages to be printed at start decode time. */
   hXdmPP->stDMConfig.uiDirtyBits_1 = BXDM_PictureProvider_P_DIRTY_ALL;
   hXdmPP->stDMConfig.uiDirtyBits_2 = BXDM_PictureProvider_P_DIRTY_ALL;

   BDBG_LEAVE(BXDM_PP_S_ResetCommon_isr);
   return;
} /* end of BXDM_PP_S_ResetCommon_isr() */

/*
 * SW7425-2536: map the BAVC_FrameRateCode_eMax enum to a numerator/denominator pair.
 */
static const BXDM_Picture_Rate s_aFrameRateCodeToValue[BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
   { 60000, 1000 },   /* BAVC_FrameRateCode_eUnknown */
   { 24000, 1001 },   /* BAVC_FrameRateCode_e23_976 */
   { 24000, 1000 },   /* BAVC_FrameRateCode_e24 */
   { 25000, 1000 },   /* BAVC_FrameRateCode_e25 */
   { 30000, 1001 },   /* BAVC_FrameRateCode_e29_97 */
   { 30000, 1000 },   /* BAVC_FrameRateCode_e30 */
   { 50000, 1000 },   /* BAVC_FrameRateCode_e50 */
   { 60000, 1001 },   /* BAVC_FrameRateCode_e59_94 */
   { 60000, 1000 },   /* BAVC_FrameRateCode_e60 */
   { 15000, 1001 },   /* BAVC_FrameRateCode_e14_985 */
   { 15000, 2002 },   /* BAVC_FrameRateCode_e7_493 */
   { 10000, 1000 },   /* BAVC_FrameRateCode_e10 */
   { 15000, 1000 },   /* BAVC_FrameRateCode_e15 */
   { 20000, 1000 },   /* BAVC_FrameRateCode_e20 */
   { 12500, 1000 },   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
   {100000, 1000 },   /* BAVC_FrameRateCode_e100 */
   {120000, 1001 },   /* BAVC_FrameRateCode_e119_88 */
   {120000, 1000 },   /* BAVC_FrameRateCode_e120 */
   { 20000, 1001 },   /* BAVC_FrameRateCode_e19_98 */
   {  7500, 1000 },   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
   { 12000, 1000 },   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
   { 12000, 1001 },   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
   { 10000, 1001 },   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
};

/*
 * SW7425-2536: Currently for updating status returned by the "displayInterruptStatus_isr" callback.
 */
static void BXDM_PP_S_UpdateStatus_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_Status * pstStatus
   )
{
   BXDM_PictureProvider_P_TSM_Picture_Parms * pstTsm;
   const BXDM_Picture *pstUnifiedPicture;

   BDBG_ENTER(BXDM_PP_S_UpdateStatus_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pstStatus);

   BKNI_Memset( pstStatus, 0, sizeof( BXDM_PictureProvider_Status ) );

   /* Grab the vsync count from the counters. */
   pstStatus->uiVsyncCount = hXdmPP->stDMStatus.stCounters.uiVsyncCount;

   /* Get the state of the delivery queue. */
   pstStatus->bPictureQueueIsEmpty = pLocalState->bPictureQueueIsEmpty;

   /* Set the parameters for the currently selected picture. */
   pstTsm = &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM);

   pstStatus->stCurrentPicture.stFrameRate.eType   = BXDM_Picture_FrameRateType_eFixed;
   pstStatus->stCurrentPicture.stFrameRate.bValid  = true;
   pstStatus->stCurrentPicture.stFrameRate.stRate  = s_aFrameRateCodeToValue[pstTsm->stStatic.eFrameRateXVD];

   pstStatus->stCurrentPicture.uiEffectivePts = pstTsm->stDynamic.uiEffectivePTS[BXDM_PictureProvider_P_PTSIndex_eActual];
   pstStatus->stCurrentPicture.uiEffectiveStc = pstTsm->stDynamic.uiEffectiveStc[BXDM_PictureProvider_P_PTSIndex_eActual];

   pstStatus->stCurrentPicture.bSelectionModeTsm = ( BXDM_PictureProvider_DisplayMode_eTSM == pstTsm->stDynamic.eSelectionMode );

   pstUnifiedPicture = hXdmPP->stDMState.stChannel.stSelectedPicture.pstUnifiedPicture;

   if ( NULL != pstUnifiedPicture )
   {
      pstStatus->stCurrentPicture.bValid = true;
      pstStatus->stCurrentPicture.uiSerialNumber = pstUnifiedPicture->uiSerialNumber;
   }

   BDBG_LEAVE(BXDM_PP_S_UpdateStatus_isr);

   return;

} /* end of BXDM_PP_S_UpdateStatus_isr() */


static void BXDM_PP_S_InitPerVsync_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo
   )
{
   BDBG_ENTER(BXDM_PP_S_InitPerVsync_isr);

   /* the count of vsync's since BXVD_StartDecode() was called */
   hXdmPP->stDMStatus.stCounters.uiVsyncCount++;

   /* SW7405-4736: a vsync count that is reset about once per second.
    * Used to conditionally print QM debug messages.
    */
   hXdmPP->stDMState.stDecode.uiVsyncCountQM++;

   /* Call the Decoder Display Interrupt Event callback if one is registered */
   if ( NULL != hXdmPP->stDMConfig.stDecoderInterface.displayInterruptEvent_isr )
   {
      /* Snapshot the start time. */
      BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderDisplayInterruptEventIsr );

      hXdmPP->stDMConfig.stDecoderInterface.displayInterruptEvent_isr(
               hXdmPP->stDMConfig.pDecoderPrivateContext
               );

      /* Snapshot the end time. */
      BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderDisplayInterruptEventIsr );
   }

   /* BXDM_PP_S_SnapShotFirmwareState_isr sets internal DM state based on
    * the AVD state */
   BXDM_PP_S_SnapShotFirmwareState_isr( hXdmPP, pLocalState, pstDisplayInterruptInfo );

   if ( BXDM_PictureProvider_P_InterruptType_eDependent != pLocalState->eInterruptType )
   {
   /* BXDM_PP_S_SnapShotXvdState_isr sets internal DM state based on the
    * XVD state */
      BXDM_PP_S_SnapShotXvdState_isr( hXdmPP, pLocalState );
   }

   BDBG_LEAVE(BXDM_PP_S_InitPerVsync_isr);
   return;
} /* end of BXDM_PP_S_InitPerVsync_isr() */


/* SW7405-5043: Perform rate matching based on average deltaPTS vs average deltaSTC.

   If they are within .05% then they are considered matched.
   Note: here dPTS and dSTC are the raw dPTS from the stream (after interpolation), and the raw dSTC from the decoder
   before any manipulation/correction.

   NOTE: Rate matching only affects application of the JTI.

   For purposes of JTI, 29.97 != 30, 59.94 != 60 and 23.98 != 24 (i.e. expect a .1% diff)
   So rate match is true if diff between dPTSAvg and dSTCAvg is < .05% of the smaller of the two values
   Note: 1/2048 ~ 0.05%, so >> by 11 bits (keeping in mind that the averages are always positive values)

   TODO: What about 30fps progressive on a 60Hz display?  Ideally, JTI should be applied for any integer multiple of the
   source rate, since the same STC jitter problem can occur.
   It is questionable whether we need to even do rate match at all - seems we should be doing this always!
*/
static bool BXDM_PP_S_RateMatchDetection_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   /* default to true, since we want to perform JTI as soon as possible if necessary.  If rate is NOT matched, it will soon
      figure it out and change settings */
   bool bRateMatched = true;
   /* get raw dSTC (does not include PCR Offsets etc) */
   int32_t iDeltaSTC;

   if (hXdmPP->stDMConfig.bSTCValid == false)
   {
      /* STC not yet valid, so can't determine avg. dSTC */
      return hXdmPP->stDMState.stDecode.bRateMatchedLastReported;
   }

   /* SW7425-4389: when the SW STC is enabled, use iStcDelta for the STC delta. */

   if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
   {
      iDeltaSTC = ( hXdmPP->stDMConfig.stClockOverride.iStcDelta < 0 ) ?
                        -hXdmPP->stDMConfig.stClockOverride.iStcDelta :
                        hXdmPP->stDMConfig.stClockOverride.iStcDelta ;
   }
   else
   {
      /* SW7445-2460: at start up, this will hold off computing the average until
       * the STC have been valid for two vsync's. */
      iDeltaSTC = ( true == hXdmPP->stDMState.stDecode.bLastStcValid ) ? pLocalState->uiStcSnapshot - hXdmPP->stDMState.stDecode.uiLastStcSnapshot : 0;
   }

   if (iDeltaSTC > 0)
   {
      /* STC is advancing ... */
      BXDM_PPFP_P_DataType * pstDeltaSTCAvg = &(pLocalState->stDeltaSTCAvg);
      uint32_t * puiNumSamples = &(pLocalState->uiNumSTCSamples);
      BXDM_PPFP_P_DataType stDeltaPTSAvg;
      BXDM_PPFP_P_DataType stDiff;
      uint32_t uiHi, uiLo;

      /* determine Average dSTC ... */
      BXDM_PPAVG_P_AddValueInteger_isrsafe(hXdmPP->stDMState.stChannel.hDeltaSTCAvg, iDeltaSTC);
      BXDM_PPAVG_P_GetAverage_isrsafe(hXdmPP->stDMState.stChannel.hDeltaSTCAvg, pstDeltaSTCAvg, puiNumSamples);

      /* NOTE: Don't do FixPtDiv until at least ONE picture is accounted for (else divide-by-zero error!). */
      if ((*puiNumSamples < BXDM_PictureProvider_P_RATE_MATCH_DETECTION_SAMPLES)
            || (hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount == 0))
         /* not enough samples to get a stable average */
         return bRateMatched;

      /* determine Average dPTS (using FRD parameters) ... */
      BXDM_PPFP_P_FixPtDiv_isr(
               &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
               hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
               &stDeltaPTSAvg
               );

      /* determine the difference between Avg. dPTS and Avg. dSTC ... */
      /* since there is a flaw in the Fixed point math, determine which is larger, then subtract to ensure result is positive */
      if ((stDeltaPTSAvg.uiWhole > pstDeltaSTCAvg->uiWhole)
         || ((stDeltaPTSAvg.uiWhole == pstDeltaSTCAvg->uiWhole) && (stDeltaPTSAvg.uiFractional > pstDeltaSTCAvg->uiFractional)))
      {
         /* Avg PTS > Avg STC */
         BXDM_PPFP_P_FixPtSub_isr(&stDeltaPTSAvg, pstDeltaSTCAvg, &stDiff);
         /* determine 0.05% of dSTC */
         uiHi = pstDeltaSTCAvg->uiWhole >> 11;
         uiLo = (pstDeltaSTCAvg->uiFractional >> 11) | ((pstDeltaSTCAvg->uiWhole & 0x7FF) << 5);
         /* rate matched if diff <= 0.05% of dSTC */
      }
      else
      {
         /* Avg PTS <= Avg PTS */
         BXDM_PPFP_P_FixPtSub_isr(pstDeltaSTCAvg, &stDeltaPTSAvg, &stDiff);
         /* determine 0.05% of dPTS */
         uiHi = stDeltaPTSAvg.uiWhole >> 11;
         uiLo = (stDeltaPTSAvg.uiFractional >> 11) | ((stDeltaPTSAvg.uiWhole & 0x7FF) << 5);
         /* rate matched if diff <= 0.05% of dPTS */
      }
      bRateMatched = (stDiff.uiWhole <= uiHi) && ((stDiff.uiWhole != uiHi) || (stDiff.uiFractional <= uiLo));

#if 0
      BKNI_Printf("Avg dPTS: %d.%d [%03d], Avg dSTC: %d.%d [%03d] diff: %d.%d uiHi:%d uiLo:%d (%smatched)\n",
         stDeltaPTSAvg.uiWhole,
         stDeltaPTSAvg.uiFractional,
         hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
         pstDeltaSTCAvg->uiWhole,
         pstDeltaSTCAvg->uiFractional,
         *puiNumSamples,
         stDiff.uiWhole,
         stDiff.uiFractional,
         uiHi,
         uiLo,
         (bRateMatched)?"":"not ");
#endif
      /* TODO: Nilesh says rate is also considered to be matched if they are integer multiples of each other.
         e.g. 30fps progressive is rate-matched to 60Hz display (dPTS = 2 * dSTC) */
   }
   else
   {
      /* else, STC not advancing (or there is a discontinuity - i.e. stream has "wrapped"), so cannot be rate matched */
      bRateMatched = false;
   }

   hXdmPP->stDMState.stDecode.bRateMatchedLastReported = bRateMatched;
   return bRateMatched;
}

static void BXDM_PP_S_STCTrickModeDetection_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   pLocalState->eSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_eOff;
   pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM] = pLocalState->stSTCDelta;
   pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM] = pLocalState->stSTCDelta;

   if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
   {
      uint32_t uiTempStcDelta;


      /* SW7425-4389: flag when using the SW STC to run in reverse */
      if ( hXdmPP->stDMConfig.stClockOverride.iStcDelta < 0 )
      {
         pLocalState->bUsingSwStcToRunInReverse = true;
         uiTempStcDelta = -hXdmPP->stDMConfig.stClockOverride.iStcDelta;
      }
      else
      {
         uiTempStcDelta = hXdmPP->stDMConfig.stClockOverride.iStcDelta;
      }

      /*
       * SW7425-1264: set the STC trick mode based on the sign and magnitude of "stClockOverride.iStcDelta".
       */
      if ( uiTempStcDelta == 0 )
      {
         pLocalState->eSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_ePause;
      }
      else if ( uiTempStcDelta == pLocalState->stSTCDelta.uiWhole )
      {
         pLocalState->eSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_eOff;
      }
      else if ( uiTempStcDelta < pLocalState->stSTCDelta.uiWhole )
      {
         pLocalState->eSTCTrickMode = ( pLocalState->bUsingSwStcToRunInReverse ) ?
                                             BXDM_PictureProvider_P_STCTrickMode_eSlowRewind :
                                             BXDM_PictureProvider_P_STCTrickMode_eSlowMotion ;
      }
      else
      {
         pLocalState->eSTCTrickMode = ( pLocalState->bUsingSwStcToRunInReverse ) ?
                                             BXDM_PictureProvider_P_STCTrickMode_eFastRewind :
                                             BXDM_PictureProvider_P_STCTrickMode_eFastForward ;
      }

      /*  SW7425-1264: "stClockOverride.iStcDelta" is a whole number, clear the fractional component of "stSTCDelta".
       */
      pLocalState->stSTCDelta.uiWhole = uiTempStcDelta;
      pLocalState->stSTCDelta.uiFractional = 0;

      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM] = pLocalState->stSTCDelta;
      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM] = pLocalState->stSTCDelta;

      hXdmPP->stDMState.stDecode.eLastCalculatedSTCTrickMode = pLocalState->eSTCTrickMode;

   }
   else if ( ( BXDM_PictureProvider_DisplayMode_eTSM == pLocalState->eDisplayMode ) /* In TSM mode */
            && ( true == hXdmPP->stDMState.stDecode.bLastStcValid ) /* Last STC was valid */
            && ( true == hXdmPP->stDMConfig.bSTCValid ) /* Current STC is valid */
           )
   {
      int32_t iDeltaStcDiff;
      uint32_t uiDeltaStcDiffAbs;
      BXDM_PictureProvider_P_STCTrickMode eCalculatedSTCTrickMode = pLocalState->eSTCTrickMode;

      /* Note: the following can only be NOT true if stream wrap occurs ... */
      if ( pLocalState->uiStcSnapshot >= hXdmPP->stDMState.stDecode.uiLastStcSnapshot )
      {
         pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole = pLocalState->uiStcSnapshot - hXdmPP->stDMState.stDecode.uiLastStcSnapshot;
         pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiFractional = 0;

         if ( pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole == 0 )
         {
            /* The STC has not changed, so we're in an STC pause */
            eCalculatedSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_ePause;
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_ePPV2, "%x:[%02x.xxx] STC Pause Detected",
                                    hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                    BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));

            /* SW7445-2544: this is a very narrow fix for when the STC stalls in NRT mode.
             * The IC (ignore cadence) flag was being set on the picture after the stall.
             * XDM interpreted the STC stall as a STC trick mode of pause. This caused
             * the FIC logic to be disabled and the IC flag to be set.
             * We use the fact that the vsync interrupt polarity did not change to
             * differentiate between normal operation and NRT mode. */

            pLocalState->bNRTModeStall = pLocalState->bRepeatedVsyncPolarity;
         }
         else
         {
            if (pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole >= pLocalState->stSTCDelta.uiWhole)
            {
               uiDeltaStcDiffAbs = pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole - pLocalState->stSTCDelta.uiWhole;
               iDeltaStcDiff = uiDeltaStcDiffAbs;
            }
            else
            {
               uiDeltaStcDiffAbs = pLocalState->stSTCDelta.uiWhole - pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole;
               iDeltaStcDiff = -uiDeltaStcDiffAbs;
            }

            if ( uiDeltaStcDiffAbs <= pLocalState->uiSTCJitterThreshold )
            {
               /* The STC is within +/- of the STC jitter threshold, so
                * we're not in an STC trick mode */
               eCalculatedSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_eOff;

               /* SW7601-179: We set the effective STC delta to the
                * expected STC delta to prevent any issues with STC
                * jitter when using the effective STC delta (e.g. for
                * computing the display duration for the currently
                * selected picture) */
               pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM] = pLocalState->stSTCDelta;

            }
            else if ( iDeltaStcDiff < 0 )
            {
               /* The STC has incremented slower than expected, so we
                * are in STC slow motion */
               eCalculatedSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_eSlowMotion;
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_ePPV2, "%x:[%02x.xxx] STC Slow Motion Detected",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
            }
            else if ( iDeltaStcDiff > 0 )
            {
               /* The STC has incremented faster than expected, so we
                * are in STC fast forward */
               eCalculatedSTCTrickMode = BXDM_PictureProvider_P_STCTrickMode_eFastForward;
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_ePPV2, "%x:[%02x.xxx] STC Fast Forward Detected",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
            }
         }
      }
      pLocalState->eSTCTrickMode = eCalculatedSTCTrickMode;

      /* PR54680: We need to differentiate between "pause -> frame
       * advance -> pause" and pause -> normal/fast forward cases.  We
       * assume we're in a frame advance until we determine that we're
       * in a normal/fast forward for more than 2 vsyncs in a row. */
      if ( ( BXDM_PictureProvider_P_STCTrickMode_ePause == hXdmPP->stDMState.stDecode.eLastReportedSTCTrickMode ) /* In pause last vsync */
           && ( ( BXDM_PictureProvider_P_STCTrickMode_eFastForward == pLocalState->eSTCTrickMode ) /* We seem to be in fast forward or normal on this vsync */
                || ( BXDM_PictureProvider_P_STCTrickMode_eOff == pLocalState->eSTCTrickMode ) ) )
      {
         if ( pLocalState->eSTCTrickMode != hXdmPP->stDMState.stDecode.eLastCalculatedSTCTrickMode )
         {
            /* We haven't calculated the same STC trick mode for 2
             * vsyncs in a row, so override the reported stc trick
             * mode to be the same as the previous vsync */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_ePPV2, "%x:[%02x.xxx] STC Trick Mode Override (%d -> %d)",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       pLocalState->eSTCTrickMode,
                                       hXdmPP->stDMState.stDecode.eLastReportedSTCTrickMode ));

            pLocalState->eSTCTrickMode = hXdmPP->stDMState.stDecode.eLastReportedSTCTrickMode;
         }
      }
      hXdmPP->stDMState.stDecode.eLastCalculatedSTCTrickMode = eCalculatedSTCTrickMode;
   }
   else
   {
      hXdmPP->stDMState.stDecode.eLastCalculatedSTCTrickMode = pLocalState->eSTCTrickMode;
   }
   hXdmPP->stDMState.stDecode.eLastReportedSTCTrickMode = pLocalState->eSTCTrickMode;
}

static void BXDM_PP_S_SnapShotXvdState_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   bool bRateMatched = false;
   bool bApplyJitterOffset = false;
   BXDM_PictureProvider_P_Picture_Context * pstPreviousPicture = &(hXdmPP->stDMState.stChannel.stPreviousPicture);

   BDBG_ENTER(BXDM_PP_S_SnapShotXvdState_isr);

   /* Operate in VSYNC mode if the application has set the "DisplayMode"
    * to VSYNC or if either CRC or DQT mode has been enabled.
    */
   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == hXdmPP->stDMConfig.eDisplayMode
         || true == hXdmPP->stDMConfig.bCRCMode
      )
   {
      pLocalState->eDisplayMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;

      /* Don't perform "preroll" if in vsync mode. */
      hXdmPP->stDMState.stDecode.bPreRolling = false;
   }
   else
   {
      pLocalState->eDisplayMode = BXDM_PictureProvider_DisplayMode_eTSM;
   }
   /* Set global picture selection mode equal to the display mode by
    * default */
   pLocalState->eSelectionMode = pLocalState->eDisplayMode;

   /* Determine if a decoder trick mode is enabled */
   pLocalState->uiSlowMotionRate = BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE;

   if ( 0 == hXdmPP->stDMConfig.stPlaybackRate.uiNumerator  )
   {
      pLocalState->uiSlowMotionRate = 0;
   }
   else if ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != hXdmPP->stDMState.stDecode.uiPreRollPlaybackRate
            && true == hXdmPP->stDMState.stDecode.bPreRolling )
   {
      /* If "prerolling",  use the preroll rate as a slow motion rate. */
      pLocalState->uiSlowMotionRate = hXdmPP->stDMState.stDecode.uiPreRollPlaybackRate;
   }
   else if ( hXdmPP->stDMConfig.stPlaybackRate.uiNumerator != hXdmPP->stDMConfig.stPlaybackRate.uiDenominator )
   {
      pLocalState->uiSlowMotionRate = (hXdmPP->stDMConfig.stPlaybackRate.uiNumerator * BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE) / hXdmPP->stDMConfig.stPlaybackRate.uiDenominator;
   }

   /* Set selection mode to VSYNC if we're in a decoder or vsync trick mode. */
   if ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != pLocalState->uiSlowMotionRate )
   {
      /* We're in a decoder or vsync trick mode */
      pLocalState->eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
      hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;

      /* PR50235: We invalidate the STC when in a decoder or vsync trick mode.
       * One result of this action is that "bForcePtsError" will be set
       * to "true" upon exiting trick mode.
       *
       * Only invalidate the STC if the following are true.
       * - have not already transistioned into trick mode, i.e. only set
       *   the flag to false once
       * - not in preroll mode (looking for TSM lock to exit preroll)
       * - in playback mode
       */
      if ( false == hXdmPP->stDMState.stDecode.bPreviousVsyncWasTrickMode
          && false == hXdmPP->stDMState.stDecode.bPreRolling
          && true == hXdmPP->stDMConfig.bPlayback )
      {
         BXDM_PictureProvider_SetSTCValid_isr(hXdmPP, false);

         /* SW7425-4163: i.e. multiple frame advance commands need to be issued before the next picture
          * on the queue was displayed.  The issue was a race condition between calling
          * BXPT_PcrOffset_SetStc_isr to reload the STC and BXVD_PVR_EnablePause to pause XDM.
          *
          * The virtual STC is reloaded at the end of the vsync with the actual STC if the current
          * picture was selected in TSM mode AND the STC is valid.  The race condition was:
          *
          * 1) the middleware would reseed the STC with the current PTS - 6006
          * 2) XDM would be called to process a vsync interrupt and reload the
          *    virtual STC with the new smaller STC
          * 3) the middleware would call BXVD_PVR_EnablePause
          * 4) XDM would run again and switch to vsync mode due to the pause request;  the picture at the
          *    head of the queue would be a little early requiring multiple frame advance commands to pass TSM.
          *
          * Typically XDM would not run between the STC being reseeded and the pause command being issued, step 2 from above.
          * As a result, the virtual STC would not be reloaded with the new STC value and frame advances would work fine.
          *
          * The fix is to reload the virtual STC with the virtual PTS of the current picture whenever XDM
          * transitions into a trick mode.  The current PTS Offset needs to be factored in as well.
          */
         {
            BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture = &hXdmPP->stDMState.stChannel.stSelectedPicture;
            BXDM_PPFP_P_DataType stTempPts;

            BXDM_PPQM_P_GetPtsWithFrac_isr(
                     pstSelectedPicture,
                     BXDM_PictureProvider_P_PTSIndex_eVirtual,
                     pstSelectedPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement,
                     &stTempPts
                     );

            hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole = stTempPts.uiWhole;
            hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole += ( stTempPts.uiFractional ) ? 1 : 0 ;

            hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole += hXdmPP->stDMConfig.uiPTSOffset;

            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_ePPV2, "%x:[%02x.xxx] entering trick mode, virtual STC was reset: uiSlowMotionRate:%08x vSTC:%08x",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       pLocalState->uiSlowMotionRate,
                                       hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole ));
         }

      }

      /* We keep track of when we're in VSYNC mode to detect
       * transitions from VSYNC --> TSM mode and handle features such
       * as bAutoValidateStcOnPause */
      hXdmPP->stDMState.stDecode.bPreviousVsyncWasTrickMode = true;
   }
   else
   {
      /* We're not in a decoder trick mode */

      /* Reset the default frame advance mode */
      hXdmPP->stDMConfig.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;
      hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount = 0;

      if ( (BXDM_PictureProvider_DisplayMode_eTSM == pLocalState->eDisplayMode)
           && hXdmPP->stDMState.stDecode.bPreviousVsyncWasTrickMode
         )
      {
         /* TSM mode transition: trick -> play (i.e. exit trick mode) ... */
         BXDM_PictureProvider_P_Picture_Context *pstPictureUnderEvaluation;

         /* SW7405-3895:
               if STC not valid
                  if RequestSTC handler installed
                     re-enable STC handler
                  else
                     force PTS error */
         if (false == hXdmPP->stDMConfig.bSTCValid)
         {
            /* STC is not valid, so request an update from the app ... */
            if (NULL != hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eRequestSTC].stFunction.fRequestSTC)
            {
               /* RequestSTC handler is installed, so re-enable RequestSTC callback ... */
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eRequestSTC].bEnable = true;
            }
            else
            {
               /* We force a PTS error callback when we first transition
                * from trick mode --> play so that the app can set the STC
                * accordingly */
               /* SW7405-4563: Only force the PTS Error if a valid coded PTS
                  has been seen.  If no valid PTS has been seen, it is assumed
                  that this transition occurs due to decoder flush, and hence
                  FirstPTSReady will be triggered (when appropriate) instead. */
               if (true == hXdmPP->stDMState.stDecode.bFirstCodedPtsReceived)
                  pLocalState->bForcePtsError = true;
            }
         }
         /* else, STC is valid, so do nothing (TSM evaluation will proceed as normal) */

         if ( true == hXdmPP->stDMConfig.bAutoValidateStcOnPause )
         {
            /* PR50235: bAutoValidateStcOnPause - If we're in TSM
             * mode, and the auto validate stc on pause feature is
             * enabled, and the previous vsync was a decoder trick
             * mode, and the current vsync is not a trick mode, we
             * enable auto STC validation */
            hXdmPP->stDMState.stDecode.bAutoValidateStc = true;
         }

         /* Reset the selection mode for the picture under evaluation.
          * If the picture under evaluation was looked at during the
          * previous vsync, which was a trick mode, then it's
          * selection mode will be VSYNC mode.  We want to reset it if
          * we're exiting trick mode.  If we don't then the picture
          * may advance for one frame and pause briefly (stutter)
          * until the actual TSM passed on the next picture.  */
         pstPictureUnderEvaluation = BXDM_PPQM_P_GetFirstPictureContext_isr(hXdmPP);
            pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eSelectionMode = pLocalState->eSelectionMode;

         hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stDynamic.eSelectionMode = pLocalState->eSelectionMode;

         BXDM_PPFIC_P_ResetFieldInversionCorrectionOffset_isr(
            hXdmPP,
            pLocalState);

         BXDM_PPCLIP_P_ClipTimeTrickModeTransitionHandler_isr(
            hXdmPP,
            pLocalState);

         BXDM_PPVTSM_P_ClipTimeTrickModeTransitionHandler_isr(
            hXdmPP,
            pLocalState);

         /* We need to update bPreviousVsyncWasTrickMode at the END of
          * this code block because the flag is used to determine if a
          * trick mode transition has occurred, so we don't want to
          * reset it until the previous value has been evaluated */
         hXdmPP->stDMState.stDecode.bPreviousVsyncWasTrickMode = false;
      }
   }

   /* Determine the clock rate */
   if ( (BAVC_VideoCompressionStd_eMPEG2DTV == hXdmPP->stDMConfig.eProtocol)
        || (BAVC_VideoCompressionStd_eMPEG2_DSS_PES == hXdmPP->stDMConfig.eProtocol)
      )
   {
      pLocalState->eClockRate = BXDM_PictureProvider_P_ClockRate_eDirecTV;
   }
   else
   {
      pLocalState->eClockRate = BXDM_PictureProvider_P_ClockRate_eMPEG2;
   }

   /* Determine the expected delta STC for the specified monitor rate */
   /* NOTE: a monitor rate of "unknown" cannot occur via the configuration API.  See BXDM_PictureProvider_SetMonitorRefreshRate_isr */
   pLocalState->stSTCDelta = sDeltaStcLUT[pLocalState->eClockRate][hXdmPP->stDMConfig.eMonitorRefreshRate];

   bRateMatched = BXDM_PP_S_RateMatchDetection_isr(
      hXdmPP,
      pLocalState);

   pLocalState->uiSTCJitterThreshold = BXDM_PictureProvider_P_STC_JITTER_THRESHOLD;
   if ( BXDM_PictureProvider_P_ClockRate_eDirecTV == pLocalState->eClockRate )
   {
      pLocalState->uiSTCJitterThreshold *= 600;
   }

   /* Determine if we're in an STC trick mode */
   BXDM_PP_S_STCTrickModeDetection_isr(hXdmPP, pLocalState);

   /* SW7425-2915: added to help debug NRT issues. */
   hXdmPP->stDMState.stDecode.bStcStalled = ( hXdmPP->stDMState.stDecode.uiLastStcSnapshot == pLocalState->uiStcSnapshot );
   hXdmPP->stDMState.stDecode.uiLastStcSnapshot = pLocalState->uiStcSnapshot;
   hXdmPP->stDMState.stDecode.bLastStcValid = ( true == hXdmPP->stDMConfig.bSTCValid );

   /* SW7405-5043: JTI should be applied for STC trick modes also, since for example 50p content on a 60Hz
    * display will rate match when an STC trick rate of 1.2 is applied
    * This correction will NOT activate for STC pause, since this cannot be rate-matched
    *
    * SW7405-3358: Adjust STC snapshot to move (STC-PTS) away from 0 and deltaPTS so that STC/PTS jitter
    * doesn't affect the scan cadence
    *
    * SW7445-2645: for the first picture, don't bother to check if the stream and display are rate
    * matched.  Apply the jitter offset if the STC/PTS delta is within the jitter threshold of either
    * '0' or '375'.  The '375' value can occur when the stream has an effective 3:2 pulldown, i.e for
    * 23.97/24 -> 59.94/60.  By applying the offset at the beginning, it should not need to be changed
    * again for the duration of the stream (if it is well behaved).
    */
   if ( true == hXdmPP->stDMConfig.bJitterToleranceImprovement )
   {
      int32_t iStcPtsDifference = pstPreviousPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated;
      int32_t iPtsDelta = (int32_t)pstPreviousPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole;
      bool bWithinThreshold = false;

      /* Check that the picture has been validated, needed at startup. */
      bApplyJitterOffset = ( true == pstPreviousPicture->bValidated );

      /* Only use the picture the vsync after it was selected for display. */
      bApplyJitterOffset &= ( false == pstPreviousPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated );

      /* Picture was selected in TSM mode */
      bApplyJitterOffset &=
            ( BXDM_PictureProvider_DisplayMode_eTSM == pstPreviousPicture->stPicParms.stTSM.stDynamic.eSelectionMode );

      /* The picture passed. */
      bApplyJitterOffset &= ( iStcPtsDifference >= 0 );


      /* The picture is with in the jitter threshold at either end of the pass window. */
      bWithinThreshold = ( iStcPtsDifference < (int32_t) pLocalState->uiSTCJitterThreshold );
      bWithinThreshold |= ( ( iStcPtsDifference <= (int32_t) iPtsDelta )
                                    && ( ( iPtsDelta - iStcPtsDifference ) < (int32_t)pLocalState->uiSTCJitterThreshold ) );

      /* Added for SW7445-2645: check if STC/PTS delta for the first picture to
       * pass the TSM test is within the jitter threshold of 375. */
      if ( BXDM_PictureProvider_P_TsmPassState_eFirstToPass == hXdmPP->stDMState.stDecode.eTsmPassState )
      {
         bWithinThreshold |= ( ( iStcPtsDifference < (int32_t)pLocalState->uiSTCJitterThreshold + 375 )
                              && ( iStcPtsDifference > (int32_t)pLocalState->uiSTCJitterThreshold - 375 ) );
      }

      bApplyJitterOffset &= bWithinThreshold;

      /* For pictures after the first one to pass the TSM test,
       * check if the frame and display rates are matched. */
      if ( BXDM_PictureProvider_P_TsmPassState_ePostFirstPass == hXdmPP->stDMState.stDecode.eTsmPassState )
      {
         /* The pre SW7445-2645: logic. */
         bApplyJitterOffset &= ( true == bRateMatched );
      }

   }

#if 0
   if ( ( true == hXdmPP->stDMConfig.bJitterToleranceImprovement )
        && ( true == hXdmPP->stDMState.stChannel.stPreviousPicture.bValidated )
        && ( false == hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stDisplay.stDynamic.bPPBRepeated )
        /* Picture was selected in TSM mode */
        && ( BXDM_PictureProvider_DisplayMode_eTSM == hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.eSelectionMode )
        /* The dStcPts is within the jitter threshold */
        && ( hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated >= 0 )
        && ( ( hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated < (int32_t) pLocalState->uiSTCJitterThreshold )
             || ( ( hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated <= (int32_t) hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stStatic.stPTSDelta.uiWhole )
                  && ( ( hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stStatic.stPTSDelta.uiWhole - hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated ) < pLocalState->uiSTCJitterThreshold ) ) )
        && ( true == bRateMatched )
      )
#endif
   if ( true == bApplyJitterOffset )
   {
      if ( 0 == hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eDBG, "%x:[%02x.xxx] Adding STC jitter correction offset (dStcPts = %d/%d, stc=%08x)",
                                    hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                    BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                    pstPreviousPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated,
                                    pstPreviousPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual,
                                    pLocalState->uiStcSnapshot ));

         hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset = -(4*pLocalState->uiSTCJitterThreshold);
      }
      else
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eDBG, "%x:[%02x.xxx] Removing STC jitter correction offset (dStcPts = %d/%d, stc=%08x)",
                                    hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                    BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                    pstPreviousPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated,
                                    pstPreviousPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual,
                                    pLocalState->uiStcSnapshot ));
         hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset = 0;
      }
    }

   /* SW7405-3358: Apply STC Snapshot offset AFTER we record the
    * original value so that we don't disturb the STC trick mode
    * detection */
   pLocalState->uiAdjustedStc = pLocalState->uiStcSnapshot + hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset;

   /* PR56325: Try to detect an incorrect monitor refresh rate setting and print a warning */
   if ( ( false == hXdmPP->stDMConfig.bPlayback ) /* In live mode */
        && ( BXDM_PictureProvider_P_STCTrickMode_eOff != pLocalState->eSTCTrickMode )
      )
   {
      /* SW7405-3113: Add 30 vsync hysteresis to incorrect monitor refresh rate
       * setting to prevent false detection during display reconfiguration
       * (e.g. 1080i to 720p).
       */
      hXdmPP->stDMState.stDecode.ui32IncorrectMonitorRefreshRateCount++;

      if ( hXdmPP->stDMState.stDecode.ui32IncorrectMonitorRefreshRateCount > 30 )
      {
         if ( hXdmPP->stDMState.stDecode.eLastIncorrectMonitorRefreshRate != hXdmPP->stDMConfig.eMonitorRefreshRate )
         {
            BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.xxx] The STC advanced by an unexpected amount since the previous vsync (%d ticks instead of %d ticks). The monitor refresh rate (%08x) may not be set correctly!",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                     pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole,
                     pLocalState->stSTCDelta.uiWhole,
                     hXdmPP->stDMConfig.eMonitorRefreshRate));
            hXdmPP->stDMState.stDecode.eLastIncorrectMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;
         }
      }
   }
   else
   {
      hXdmPP->stDMState.stDecode.ui32IncorrectMonitorRefreshRateCount = 0;
   }

   /* Set the vSTC on this vsync */
   BXDM_PPVTSM_P_VirtualStcIncrement_isr(
      hXdmPP,
      pLocalState
      );

   /* We set the repeated flags in the selected PPB to true on every
    * vsync, because they are changed by picture/element selection
    * accordingly */
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bPPBRepeated = true;
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bElementRepeated = true;

   /* Add to the currently displayed PPB's display duration */
   BXDM_PPFP_P_FixPtAdd_isr(
      &hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.stDisplayDuration,
      &pLocalState->stEffectiveSTCDelta[hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stDynamic.eSelectionMode],
      &hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.stDisplayDuration
      );

   /* Reset the picture's local SPIM mode if the global SPIM is
    * "both" */
   if ( hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.eSourceFormatOverride != BXDM_PictureProvider_SourceFormatOverride_eProgressive )
   {
      hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.eSourceFormatOverride = hXdmPP->stDMConfig.eSourceFormatOverride;
   }

   BDBG_LEAVE(BXDM_PP_S_SnapShotXvdState_isr);
   return;
} /* end of BXDM_PP_S_SnapShotXvdState_isr() */

static void BXDM_PP_S_SnapShotFirmwareState_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo
   )
{
   BDBG_ENTER(BXDM_PP_S_SnapShotFirmwareState_isr);

   pLocalState->eVsyncPolarity = pstDisplayInterruptInfo->eInterruptPolarity;

   /* SW7405-3137: initially set "eInterruptType" assuming that VDC is expecting a single picture
    * to be delivered.
    * If the video protocol is "BAVC_VideoCompressionStd_eMVC", override "eInterruptType" based on
    * the interrupt polarity.  For now, assume that "BAVC_Polarity_eTopField" is the base interrupt
    * and "BAVC_Polarity_eBotField" is the dependent interrupt.  TODO: the interrupt masks should
    * be changed to base/dependent when playing MVC streams.
    */
   pLocalState->eInterruptType = BXDM_PictureProvider_P_InterruptType_eSingle;
   if (( BAVC_VideoCompressionStd_eMVC == hXdmPP->stDMConfig.eProtocol )
        && ( true == hXdmPP->stDMConfig.st3DSettings.bSetNextPointer ))
   {
      switch ( pLocalState->eVsyncPolarity )
      {
         case BAVC_Polarity_eTopField:
            pLocalState->eInterruptType = BXDM_PictureProvider_P_InterruptType_eBase;
            break;

         case BAVC_Polarity_eBotField:
            pLocalState->eInterruptType = BXDM_PictureProvider_P_InterruptType_eDependent;
            break;

         default:
            break;
      }
   }

   /* SW7445-2544: used to avoid setting the IC (ignore cadence) flag when the STC stalls in NRT mode. */
   pLocalState->bRepeatedVsyncPolarity = false;

   if ( true == hXdmPP->stDMState.stDecode.bPreviousVsyncPolarityValid )
   {
      if ( ( BAVC_Polarity_eFrame != hXdmPP->stDMState.stDecode.ePreviousVsyncPolarity )
           && ( BAVC_Polarity_eFrame != pLocalState->eVsyncPolarity )
           && ( hXdmPP->stDMState.stDecode.ePreviousVsyncPolarity == pLocalState->eVsyncPolarity ) )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eQM, " %x:[%02x.xxx] Same polarity (%s) seen two vsync's in a row",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       ( BAVC_Polarity_eTopField == pLocalState->eVsyncPolarity ) ? "top" : "bottom"
                                       ));

         pLocalState->bRepeatedVsyncPolarity = true;
      }
   }
   hXdmPP->stDMState.stDecode.bPreviousVsyncPolarityValid = true;
   hXdmPP->stDMState.stDecode.ePreviousVsyncPolarity = pLocalState->eVsyncPolarity;

   BDBG_ASSERT( hXdmPP->stDMConfig.uiSTCIndex < pstDisplayInterruptInfo->uiSTCCount );

   /* SW7425-1264: support for a synthesized STC; can create a clock that run backwards.
    * If enabled, use and interpolated the synthesized STC , otherwise use the HW STC.
    */
   if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
   {
      /* SW7425-1264: if a new SW STC value has been specified, load it at this time.*/

      if ( true == hXdmPP->stDMConfig.bInitializeSwStc )
      {
         hXdmPP->stDMState.stChannel.uiSwStc = hXdmPP->stDMConfig.stClockOverride.uiStcValue;
         hXdmPP->stDMConfig.bInitializeSwStc = false;
      }

      pLocalState->uiStcSnapshot = hXdmPP->stDMState.stChannel.uiSwStc;

      /* Interpolate the SW STC. */
      hXdmPP->stDMState.stChannel.uiSwStc += hXdmPP->stDMConfig.stClockOverride.iStcDelta;

   }
   else
   {
      pLocalState->uiStcSnapshot = pstDisplayInterruptInfo->astSTC[hXdmPP->stDMConfig.uiSTCIndex].uiValue;
   }

   /* SW7405-3085: Channel Synchronization */
   pLocalState->uiDisplayInterruptCount = pstDisplayInterruptInfo->stInterruptCount.uiValue;
   pLocalState->bDisplayInterruptCountValid = pstDisplayInterruptInfo->stInterruptCount.bValid && hXdmPP->stDMConfig.bChannelSyncMode;

   BDBG_LEAVE(BXDM_PP_S_SnapShotFirmwareState_isr);
   return;
} /* end of BXDM_PP_S_SnapShotFirmwareState_isr() */

static void BXDM_PP_S_UpdatePublicStatus_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   BXDM_PictureProvider_P_Picture_Context* pstPicCntxt;
   BXDM_PictureProvider_PTSInfo* pstCurPtsInfo;

   BDBG_ENTER(BXDM_PP_S_UpdatePublicStatus_isr);

   pstPicCntxt = &(hXdmPP->stDMState.stChannel.stSelectedPicture);

   pstCurPtsInfo = &(hXdmPP->stDMStatus.stCurrentPTS);

   /* Update "stCurrentPTSInfo".
    * If a picture has been delivered to VDC, use that PTS data.
    * Otherwise use the most recently received coded PTS; the need for
    * this became apparent when debugging  the watch dog reset.  The
    * system had trouble dealing with a very large delta between the audio
    * and video PTS values.
    * Note: "bFirstPPBHasBeenDisplayed" simply means a picture has been
    * displayed.  It may or may not have passed the TSM test.
    */
   if ( true == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed )
   {
      /* PR53039: return the actual PTS value and type.
       *
       * After calling "BXVD_StopDecode()", the intent is to return the PTS value and type of
       * the last displayed picture until "BXVD_StartDecode()" is called.  However, the values
       * returned by "..._GetPts()" and "..._GetPtsType()" are qualified by "pstPicCntxt->bValidated".
       *
       * This presented a problem after "BXVD_StopDecode()" was called when the last picture
       * was not being "held".  Several vsync callbacks were processed before the decoder
       * stopped.  During that period, "stCurrentPTS.ui32RunningPTS" was being incorrectly
       * returned as "0" and "stCurrentPTS.ePTSType" as "BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS".
       */
      BXDM_PPQM_P_GetPtsUnfiltered_isr(
            pstPicCntxt,
            BXDM_PictureProvider_P_PTSIndex_eActual,
            0,
            &(pstCurPtsInfo->ui32RunningPTS)
            );

      BXDM_PPQM_P_GetPtsTypeUnfiltered_isr( pstPicCntxt, &(pstCurPtsInfo->ePTSType) );

      pstCurPtsInfo->ui32EffectivePTS = pstPicCntxt->stPicParms.stTSM.stDynamic.uiEffectivePTS[BXDM_PictureProvider_P_PTSIndex_eActual];

      /* PR39962: save the PCR offset and associated flags for the picture being delivered to VDC. */
      pstCurPtsInfo->uiPCROffset = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue;
      pstCurPtsInfo->bPcrOffsetValid = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;
      pstCurPtsInfo->bPcrDiscontinuity = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity;
   }
   else
   {
      *pstCurPtsInfo = hXdmPP->stDMState.stDecode.stLatestCodedPts;
   }

   /* SWSTB-1613: when more than 1 picture is promoted for display on a vsync, only the last is sent
    * for display, the others are essentially dropped.  We can get multiple passes in two different
    * situations, frame rate conversion and when pictures are "too late".  In the latter case,
    * XDM burns through the pictures on the queue and displays the least late one. */

    if ( pLocalState->uiPicturesPromoted )
    {
      hXdmPP->stDMStatus.stCounters.uiDisplayManagerDroppedCount += pLocalState->uiPicturesPromoted - 1;
    }

   /* TODO: from DM V1, PR34979
    * Does this happen with the correct frequency, i.e. does it need to happen every vsync?
    */
   pstCurPtsInfo->uiDisplayManagerDroppedCount = hXdmPP->stDMStatus.stCounters.uiDisplayManagerDroppedCount;
   pstCurPtsInfo->uiPicturesDecodedCount = hXdmPP->stDMStatus.stCounters.uiPicturesDecodedCount;

   /* PR39962: Save the STC for the picture being delivered to VDC. */
   pstCurPtsInfo->uiSTCSnapShot = pLocalState->uiAdjustedStc;

   /* PR50623: Do the following need to be sampled on a per TSM evaluation basis? */
   pstCurPtsInfo->bUseHwPcrOffset = hXdmPP->stDMConfig.bUseHardwarePCROffset;

   /* SW7635-51: if a picture has been displayed, report the associated
    * software PCR offset. Othewise use the "global" value.
    */
   BXDM_PPQM_P_GetSoftwarePcrOffset_isr( hXdmPP, pstPicCntxt, &(pstCurPtsInfo->uiSwPcrOffset) );

   /* PR53013: Updated reported picture tag with the one from the
    * currently displayed picture */
   if ( ( true == pstPicCntxt->bValidated )
        && ( true == pstPicCntxt->pstUnifiedPicture->stPictureTag.bValid )
      )
   {
      hXdmPP->stDMStatus.uiPictureTag = pstPicCntxt->pstUnifiedPicture->stPictureTag.uiValue;
   }

   /* PR54677: Update reported PPBParameterInfo */
   if ( true == pstPicCntxt->bValidated )
   {
      hXdmPP->stDMStatus.stCurrentPPBParameterInfo = *(pstPicCntxt->pstUnifiedPicture);
      hXdmPP->stDMStatus.bCurrentPPBParameterInfoValid = true;

      /* If this is a pair of pictures, save the linked picture as well,
       * otherwise set "pNextPicture" to NULL to indicated that this is a
       * single picture.
       */
      if ( NULL != pstPicCntxt->pstUnifiedPicture->pNextPicture )
      {
         hXdmPP->stDMStatus.stCurrentLinkedPPBParameterInfo = *(BXDM_Picture *)(pstPicCntxt->pstUnifiedPicture->pNextPicture);
         hXdmPP->stDMStatus.stCurrentPPBParameterInfo.pNextPicture = &(hXdmPP->stDMStatus.stCurrentLinkedPPBParameterInfo);
      }
      else
      {
         hXdmPP->stDMStatus.stCurrentPPBParameterInfo.pNextPicture = NULL;
      }
   }
   else
   {
      hXdmPP->stDMStatus.bCurrentPPBParameterInfoValid = false;
   }

   /* SW7445-2544: bPreviousNRTModeStall is used to avoid setting the IC (ignore cadence) flag on
    * the last field of a picture when the STC stalls in NRT mode.*/
   hXdmPP->stDMStatus.bPreviousNRTModeStall = pLocalState->bNRTModeStall;

   BDBG_LEAVE(BXDM_PP_S_UpdatePublicStatus_isr);

   return;
} /* end of BXDM_PP_S_UpdatePublicStatus_isr() */

static void
BXDM_PP_S_PostIsr_isr(BXDM_PictureProvider_Handle hXdmPP,
                  BXDM_PictureProvider_P_LocalState* pLocalState,
                  BAVC_MFD_Picture *pMFDPicture,
                  bool bPrimaryPicture)
{
   uint32_t uiStateBits = hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiStateBits;

   BDBG_ENTER(BXDM_PP_S_PostIsr_isr);

   if ( true == bPrimaryPicture )
   {
      /* Keep track of previous PPB */
      hXdmPP->stDMState.stChannel.stPreviousPicture = hXdmPP->stDMState.stChannel.stSelectedPicture;

      /* Save last evaluated Stc/Pts Offset */
      if ( false == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bElementRepeated )
      {
         hXdmPP->stDMState.stDecode.iPreviousStcPtsDifferenceEvaluated
                  = hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated;

         /* SWBLURAY-18143: In order for the clip stop time to be
          * triggered at the correct time, we need to assume the
          * picture's initial dStcPts as counting towards the display
          * duration.  This is needed specifically for 24p content
          * displayed at 60Hz where implicit 3:2 pulldown is applied and
          * the dStcPts oscillates between two values. */
         if ( false == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bPPBRepeated )
         {
            hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.stDisplayDuration.uiWhole
                     += hXdmPP->stDMState.stDecode.iPreviousStcPtsDifferenceEvaluated;
         }
      }

      if ( hXdmPP->stDMState.stChannel.stSelectedPicture.bValidated )
      {
         /* We need to set the STCinvalid flag to false if ALL of the following are true:
          *
          * 1) We'e in LIVE mode AND
          * 2) The selected picture has a PCR that is not discontinuous
          * 3) The selected picture has a PCR that is valid
          *
          * In PLAYBACK mode, the STCinvalid flag is set to false by the
          * application after it jams the STC with the proper value.
          *
          */
         if ( !hXdmPP->stDMConfig.bPlayback
                  && !hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded
                  && hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded )
         {
            if ( false == hXdmPP->stDMConfig.bSTCValid )
            {
               /* SW7405-4736: triggers the debug message the first time bSTCValid is set to true.
                */
               hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_STC_VALID;
            }

            hXdmPP->stDMConfig.bSTCValid = true;
         }
      }
   }

   /* SWBLURAY-22286: don't reloaded the virtual STC on the dependent interrupt. */
   if ( true == bPrimaryPicture )
   {
      /* vSTC tracks actual STC on every vsync (in TSM mode) */
      BXDM_PPVTSM_P_VirtualStcSet_isr(hXdmPP, pLocalState);
   }

   /* Keep track of various DM state for reporting */
   BXDM_PPDBG_P_DecoderDropLog_isr(hXdmPP, hXdmPP->stDMState.stDecode.uiPendingDropCount);

   switch ( pLocalState->eDisplayMode )
   {
      case BXDM_PictureProvider_DisplayMode_eVirtualTSM:
         uiStateBits |= BXDM_PPDBG_State_DisplayMode_VSYNC;
         break;
      case BXDM_PictureProvider_DisplayMode_eTSM:
         uiStateBits |= BXDM_PPDBG_State_DisplayMode_TSM;
         break;
      default:
         break;
   }

   switch ( hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      case BXDM_PictureProvider_DisplayMode_eVirtualTSM:
         uiStateBits |= BXDM_PPDBG_State_SelectionMode_VSYNC;
         break;
      case BXDM_PictureProvider_DisplayMode_eTSM:
         uiStateBits |= BXDM_PPDBG_State_SelectionMode_TSM;
         break;
      default:
         break;
   }

   if ( true == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bIgnoreCadenceMatch )
   {
      uiStateBits |= BXDM_PPDBG_State_IgnoreCadenceMatch;
   }

   if ( hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity )
   {
      uiStateBits |= BXDM_PPDBG_State_PCRDiscontinuity;
   }

   if ( hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid )
   {
      uiStateBits |= BXDM_PPDBG_State_PCRPresent;
   }

   if ( !hXdmPP->stDMConfig.bSTCValid )
   {
      uiStateBits |= BXDM_PPDBG_State_STCInvalid;
   }

   if ( hXdmPP->stDMConfig.bUseHardwarePCROffset )
   {
      uiStateBits |= BXDM_PPDBG_State_UseHwPcrOffset;
   }

   BXDM_PPDBG_P_StateLog_isr(hXdmPP, uiStateBits);

   /* Dereference system and picture state for the second debug status line.  */

   pLocalState->uiState2Bits |= ( true == hXdmPP->stDMState.stDecode.bStcStalled ) ? BXDM_PPDBG_State2_StcStalled : 0 ;

   pLocalState->uiState2Bits |= ( false == pLocalState->bInitialSTCValid ) ? BXDM_PPDBG_State2_InitialSTCInvalid : 0 ;
   pLocalState->uiState2Bits |= ( true == hXdmPP->stDMState.stDecode.bPreviousVsyncWasTrickMode ) ? BXDM_PPDBG_State2_PreviousVsyncTrick : 0 ;
   pLocalState->uiState2Bits |= ( true == hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition) ? BXDM_PPDBG_State2_TrickModeTransition : 0 ;

   pLocalState->uiState2Bits |= ( true == pMFDPicture->bStallStc ) ? BXDM_PPDBG_State2_RequestedStcStall : 0 ;
   pLocalState->uiState2Bits |= ( true == pMFDPicture->bIgnorePicture ) ? BXDM_PPDBG_State2_IgnorePicture : 0 ;

   BXDM_PPDBG_P_State2Log_isr( hXdmPP, pLocalState->uiState2Bits );


   BXDM_PPDBG_P_StcDeltaLog_isr( hXdmPP, pLocalState );

   /* SW7405-4736: conditionally dump the MFD structure */
   BXDM_PPDBG_P_PrintMFD_isr( hXdmPP, pLocalState, pMFDPicture );

   /* SW7405-4736: look to print the input parameters now that we are all done.
    * Only those that have changed during the execution of this event handler will be printed.
    */
   BXDM_PPDBG_P_PrintDMConfig_isr( hXdmPP, pLocalState, true );

   /* We print the DM Debug logs */
   BXDM_PPDBG_P_Print_isr(hXdmPP, pLocalState, false);

   /* SWSTB-1380: when using the debug fifo (as opposed to debug prints),
    * print all the entries on the debug fifo. */
   BXDM_PictureProvider_ReadFifo_isrsafe( hXdmPP );

   BDBG_LEAVE(BXDM_PP_S_PostIsr_isr);
   return;
} /* end of BXDM_PP_S_PostIsr_isr() */

static void BXDM_PP_S_SelectPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   bool bDispQueueHasData = false;
   bool bFindNextPPB = true;
   bool bDisplayPPB = false;
   bool bDropPPB = false;
   bool bForceDisplayPPB = false;
   uint32_t uiLoopCount=0;
   BXDM_PictureProvider_P_Picture_Context *  pstPictureUnderEvaluation;
   BXDM_PictureProvider_P_Picture_Context *  pstPreviousPictureUnderEvaluation;
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;

   BXDM_PictureProvider_PTSInfo * pstNextPTSInfo;

   BDBG_ENTER(BXDM_PP_S_SelectPicture_isr);

   pstPictureUnderEvaluation = BXDM_PPQM_P_GetFirstPictureContext_isr(hXdmPP);
   pstPreviousPictureUnderEvaluation = &(hXdmPP->stDMState.stChannel.stPreviousPictureUnderEvaluation);
   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);
   pstNextPTSInfo = &(hXdmPP->stDMStatus.stNextPTS);

   /* Previously the first time through the "while( bFindNextPPB )" loop,  the value of
    * stNextPTS.uiSTCSnapShot would be stale.  It contained uiAdjustedStc from the previous vsync.
    * This might be an issue if BXDM_PictureProvider_GetNextPTSInfo_isr were called during the
    * execution of BXDM_PPTSM_P_EvaluateTsmState_isr, say in response to the TSM result callback.  To get
    * around this problem, stNextPTS.uiSTCSnapShot in now updated before jumping into the loop.
    */
   pstNextPTSInfo->uiSTCSnapShot = pLocalState->uiAdjustedStc;

   /* Re-display the current picture; re-evaluate TSM to select the
    * best element within the picture.
    */
   BXDM_PPTSM_P_EvaluateTsmState_isr(
      hXdmPP,
      pLocalState,
      pstSelectedPicture
      );

   if ( BXDM_PictureProvider_P_DecodeState_eStarted == hXdmPP->stDMState.stChannel.eDecodeState )
   {
      /* We select the best PPB in this loop.  If a new picture is not found,
       * the old one will continue to be displayed.
       */
      while ( bFindNextPPB )
      {
         BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic1 );

         /* By default:
          * 1) we do want the loop to wrap
          * 2) We don't want to display a new PPB
          * 3) We don't want to drop a new PPB
          */
         bDisplayPPB = false;
         bForceDisplayPPB = false;
         bDropPPB = false;

         /*
         ** Update "pstPictureUnderEvaluation" with the data of the next picture in the display queue.
         ** If the display queue is empty, "bDispQueueHasData" will be set to false.
         */
         bDispQueueHasData = BXDM_PPQM_P_PeekAtNextPicture_isr(
            hXdmPP,
            pLocalState,
            pstPictureUnderEvaluation,
            pstPreviousPictureUnderEvaluation,
            pstSelectedPicture
            );

         if ( false == bDispQueueHasData )
         {
            pLocalState->bPictureQueueIsEmpty = true;

            BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_ePPBNotFound );

            /* The delivery queue has underflowed if...
             * - this is the first time through the while loop on this vsync
             * - AND the first PPB has been displayed.
             */
            if ( 0 == uiLoopCount && true == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed )
            {
              hXdmPP->stDMStatus.stCounters.uiVsyncUnderflowCount++;

               /* Count the number of times the system has gotten into an
                * underflow "state" (as opposed to the number of vsync's
                * with an underflow).
                */
               if ( false == hXdmPP->stDMState.stDecode.bDqUnderflowPrevVsync )
               {
                  hXdmPP->stDMStatus.stCounters.uiUnderflowCount++;
               }

               hXdmPP->stDMState.stDecode.bDqUnderflowPrevVsync = true;
            }

            /* Force a wait state, so that we re-display the current
             * picture */
            pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;

            /* Reset GetNextPTS() values since we don't have a "next"
             * picture in the queue */
            hXdmPP->stDMStatus.bNextPTSInfoValid = false;
            pstNextPTSInfo->ePTSType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
            pstNextPTSInfo->ui32RunningPTS = 0;
            pstNextPTSInfo->ui32EffectivePTS = 0;
            /* SW7425-875: pstNextPTSInfo->uiSTCSnapShot = pLocalState->uiAdjustedStc;*/
            pstNextPTSInfo->bPcrOffsetValid = false;
            pstNextPTSInfo->bPcrDiscontinuity = false;

            /* We need to see if the decoder supports drop@decode */
            if ( ( NULL != hXdmPP->stDMConfig.stDecoderInterface.requestPictureDrop_isr )
                 && ( NULL != hXdmPP->stDMConfig.stDecoderInterface.getPictureDropPendingCount_isr )
               )
            {
            /* In an attempt to catch up, we start dropping late
             * pictures after XX consecutive "too late" results in a
             * row and the delivery queue is dry */
               if ( hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount >= BXDM_PictureProvider_P_MAX_LATE_BEFORE_DECODER_DROP )
               {
                  /* Snapshot the start time. */
                  BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureDropPendingCountIsr );

                  hXdmPP->stDMConfig.stDecoderInterface.getPictureDropPendingCount_isr(
                           hXdmPP->stDMConfig.pDecoderPrivateContext,
                           &hXdmPP->stDMState.stDecode.uiPendingDropCount
                           );

                  /* Snapshot the end time. */
                  BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureDropPendingCountIsr );

                  /* We've been late XX vsync in a row, so we try to recover
                   * by asking the decoder to drop pictures */

                  /* We only request more drops if the decoder has caught up
                   * with the previous drop request */
                  if ( 0 == hXdmPP->stDMState.stDecode.uiPendingDropCount )
                  {
                     BXDM_Picture_PullDown ePulldown;
                     uint32_t uiNumToDrop = 0;

                     /* SW7445-586: use accessor method to retrieve pulldwon */
                     BXDM_PPQM_P_GetPicturePulldown_isr( pstSelectedPicture, &ePulldown );

                     /* We request dropping of fields such that after the
                     * decoder performs the drop the STC/PTS diff will be
                     * close to 0 */
                     uiNumToDrop = ( pstSelectedPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual / pstSelectedPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole );

                     /* SW7405-3925: For progressive content, uiNumToDrop value needs to be converted to fields since the decoder
                        always expects drop count in fields, yet the DeltaPTS yields a value in elements (hences uiNumToDrop is
                        half what it should be).
                        It also needs to account for the extended duration of the frames if pulldown is set to FX1...FX4
                        (e.g. if STC-PTS is 6000, and deltaPTS is 1500, this would indicate 4 elements (8 fields) need to be dropped,
                         but if pulldown is FX4, then we really only need to drop 1 element (2 fields) to achieve the desired catchup)

                        Thus desired #fields to drop = uiNumToDrop (progressive elements) * 2 / #elements frame repeated
                     */
                     switch ( ePulldown )
                     {
                        case BXDM_Picture_PullDown_eFrameX4:
                           uiNumToDrop /= 2;
                           break;
                        case BXDM_Picture_PullDown_eFrameX3:
                           uiNumToDrop *= 2;
                           uiNumToDrop /= 3;
                           break;
                        case BXDM_Picture_PullDown_eFrameX2:
                           /* Do nothing - required #fields already correct due to artifical scaling introduced by deltaPTS
                              being in frames not fields */
                           break;
                        case BXDM_Picture_PullDown_eFrameX1:
                           uiNumToDrop *= 2;
                           break;
                        default:
                           /* Do nothing for non-progressive pulldowns - uiNumToDrop already specifies fields */
                           break;
                     }

                     /* Snapshot the start time. */
                     BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderRequestPictureDropIsr );

                     /* We update the actual drop count */
                     /* SW7405-3925: This function must return the modified drop count, since the requested amount may not match the
                        amount requested from the decoder due to truncation, clipping of the request, or simply not performing the
                        request if the decoder is not ready.  This ensures the uiDecoderDroppedCoun, below, is kept up-to-date. */
                     hXdmPP->stDMConfig.stDecoderInterface.requestPictureDrop_isr(
                           hXdmPP->stDMConfig.pDecoderPrivateContext,
                              &uiNumToDrop
                           );

                     /* Snapshot the end time. */
                     BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderRequestPictureDropIsr );

                     /* Update the drop count visible to the application */
                     hXdmPP->stDMStatus.stCounters.uiDecoderDroppedCount += uiNumToDrop;
                  }
               }
            }
         }     /* end of if ( false == bDispQueueHasData ) */
         else
         {
            /* If a picture is available, no underflow on this vsync. */
            hXdmPP->stDMState.stDecode.bDqUnderflowPrevVsync = false;

            BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_ePPBFound );

            /* PR47521: Keep track of actual new PPBs that were
             * evaluated on this vsync */
            pLocalState->uiPicturesEvaluated++;

            /* eChannelChangeMode: First picture preview.
             * bForceDisplayFirstPPB is a one-shot flag, so we keep a
             * local copy and reset the persistent one */
            bForceDisplayPPB = hXdmPP->stDMState.stDecode.bForceDisplayFirstPPB;

            /* Get the TSM result for the new picture
             * Note: the result will be stored in
             *    "pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult"
             */
            BXDM_PPTSM_P_EvaluateTsmState_isr(
               hXdmPP,
               pLocalState,
               pstPictureUnderEvaluation
               );


            /* Update GetNextPTS() values w/ values from this picture */
            /* Set the next PTS to be the actual PTS from the next
             * picture in the queue */
            hXdmPP->stDMStatus.bNextPTSInfoValid = true;
            pstNextPTSInfo->ePTSType = pstPictureUnderEvaluation->stPicParms.stTSM.stStatic.ePtsType;
            BXDM_PPQM_P_GetPts_isr(
               pstPictureUnderEvaluation,
               BXDM_PictureProvider_P_PTSIndex_eActual,
               0,
               &(pstNextPTSInfo->ui32RunningPTS)
               );
            pstNextPTSInfo->ui32EffectivePTS = pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.uiEffectivePTS[BXDM_PictureProvider_P_PTSIndex_eActual];
            /* SW7425-875: pstNextPTSInfo->uiSTCSnapShot = pLocalState->uiAdjustedStc;*/
            pstNextPTSInfo->uiPCROffset =  pstPictureUnderEvaluation->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue;
            pstNextPTSInfo->bPcrOffsetValid =  pstPictureUnderEvaluation->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;
            pstNextPTSInfo->bPcrDiscontinuity = pstPictureUnderEvaluation->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity;
         }     /* end of else ( true == bDispQueueHasData ) */

         BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic1 );

         BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic2 );

         /* We decide what to do with the picture based on the TSM
          * result */
         switch ( pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult )
         {
            case BXDM_PictureProvider_TSMResult_ePass:
               /* The NEW picture has PASSED TSM */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_ePass );

               /* We should display this picture */
               bDisplayPPB = true;
               break;

            case BXDM_PictureProvider_TSMResult_eTooEarly:
               /* The NEW picture is TOO EARLY */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eTooEarly );

               /* In an attempt to catch up, we start dropping early
                * pictures after XX consecutive "too early" results in a
                * row. Otherwise, we don't do anything with this
                * picture */
               if ( ( ( hXdmPP->stDMState.stDecode.uiPPBConsecutiveTooEarlyCount >= BXDM_PictureProvider_P_MAX_EARLY_BEFORE_DROP )
                      || ( false == hXdmPP->stDMConfig.bPlayback ) )
                    && ( BXDM_PictureProvider_PictureHandlingMode_eHonorPTS != pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
                  )
               {
                  bDropPPB = true;
               }
               else
               {
                  bFindNextPPB = false;
               }
               break;

            case BXDM_PictureProvider_TSMResult_eTooLate:
               /* The NEW picture is TOO LATE */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eLate );

               /* We always display pictures that are late because it is
                * less late that the currently displayed picture.  We
                * also loop back around evaluate the NEXT picture in the
                * queue in an attempt to fine a better picture for
                * display */
               bDisplayPPB = true;
               break;

            case BXDM_PictureProvider_TSMResult_eWait:
               /* The NEW picture is EARLY */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eWait );

               /* We don't do anything with the queues */
               bFindNextPPB = false;

               /* SW7405-4736: when QM debug enabled, dump the picture state if an eWait
                * result for roughly a second.
                */
               pstPictureUnderEvaluation->stPicParms.uiNumTimesWait++;

               if ( pstPictureUnderEvaluation->stPicParms.uiNumTimesWait >= BXDM_PPTMR_lutVsyncsPersSecond[ hXdmPP->stDMConfig.eMonitorRefreshRate ] )
               {
                  BXDM_PPDBG_P_PrintUnifiedPicture_isr( hXdmPP, pLocalState, pstPictureUnderEvaluation );
                  pstPictureUnderEvaluation->stPicParms.uiNumTimesWait = 0;
               }
               break;

            case BXDM_PictureProvider_TSMResult_eDrop:
               /* The NEW picture is DROP */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eDrop );

               /* We indicate that we need to drop the picture.  We also
                * loop back around and evaluate the NEXT picture in the
                * queue in an attempt to find a better picture for
                * displayq */
               bDropPPB = true;
               break;

            default:
               BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unrecognized TSM Result (%d)",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          pstPictureUnderEvaluation->stPicParms.uiPPBIndex & 0xFFF,
                                          pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult));
               break;
         }

         /* PR47521: Reset late count if we ever evaluate a non-late
          * picture.  We could run into a scenario where pictures are
          * +P+L+W which would cause the late count to always increment,
          * when it really shouldn't */
         if ( BXDM_PictureProvider_TSMResult_eTooLate != pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult )
         {
            if ( BXDM_PictureProvider_TSMResult_eWait == pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult )
            {
               if ( bDispQueueHasData )
               {
                  hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount=0;
               }
            }
            else
            {
               hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount=0;
            }
         }

         /* Keep track of too early PPBs */
         if (BXDM_PictureProvider_TSMResult_eTooEarly == pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult )
         {
            hXdmPP->stDMState.stDecode.uiPPBConsecutiveTooEarlyCount++;
         }
         else
         {
            hXdmPP->stDMState.stDecode.uiPPBConsecutiveTooEarlyCount=0;
         }

         /* Handle Freeze */
         if ( hXdmPP->stDMConfig.bFreeze )
         {
            if ( bDisplayPPB )
            {
               /* We're in a frozen state, so instead of displaying the
                * PPB, we force drop the PPB */
               BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eFreeze );

               bDisplayPPB = false;
               bDropPPB = true;
            }
         }

         /* Handle Force Display - note: overrides freeze
          *
          * SW7435-105:  If the first picture is to be dropped, don't force
          * it to be displayed.  Displaying it can create a deadlock situation.
          * The deadlock comes about if the first picture does not have a valid
          * PTS and it is promoted for display. The Request STC callback will
          * not fire because the "current" picture does not have a valid PTS.
          * Without the callback, the application will never set the STC.
          */
         if (( true == bForceDisplayPPB ) && ( false == bDropPPB ))
         {
            /* We're in a force display mode */
            BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eForce );

            /* Force the picture to be displayed. */
            bDisplayPPB = true;

            /* First picture preview a one-shot, reset flag in stDMState */
            hXdmPP->stDMState.stDecode.bForceDisplayFirstPPB = false;

            /* SW7405-3873: We set bHolding = true during first picture preview so
             * that the bIgnoreCadenceMatch is set properly to prevent display bounce
             * with interlaced content on non-MAD display path
             */
            pstPictureUnderEvaluation->stPicParms.stDisplay.stDynamic.bHolding = true;
         }

         BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic2 );

         BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic3 );

         /* See if we need to pull the picture off the queue or not */
         if ( bDisplayPPB || bDropPPB )
         {
            /* SW7445-586: for H265/HEVC interlaced, if the picture is to be displayed
             * and it is marked as SiRepeat, check if it should be "appended" to
             * the preceeding picture. See "BXDM_PPQM_P_AppendField_isr()" for more comments.
             */
            if ( true == bDisplayPPB
                && BXDM_Picture_BufferHandlingMode_eSiRepeat == pstPictureUnderEvaluation->pstUnifiedPicture->stBufferInfo.eBufferHandlingMode )
            {
               BXDM_PPQM_P_AppendField_isr( pLocalState, pstSelectedPicture, pstPictureUnderEvaluation );

               /* If the picture was "appended" to the preceeding picture, it should be dropped. */
               if ( true == pstPictureUnderEvaluation->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture )
               {
                  bDisplayPPB = false;
                  bDropPPB = true;

                  /* Needed to cause the selected picture to be reevaluate down below.  The newly
                   * appended elemented of the selected picture is going to be displayed. */
                  pLocalState->bFICChanged = true;

                  /* causes the vPTS to be reset, don't do this or else gate the vPTS reset. */
                  /*pstPictureUnderEvaluation->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eDrop;*/
               }
            }

            /* Get the picture from the queue */
            BXDM_PPQM_P_GetNextPicture_isr(
               hXdmPP,
               pLocalState,
               pstPictureUnderEvaluation
               );

            *pstPreviousPictureUnderEvaluation = *pstPictureUnderEvaluation;

            /* Decide what to do with the picture */
            if ( bDropPPB )
            {
               hXdmPP->stDMStatus.stCounters.uiDisplayManagerDroppedCount++;

               /* Release the NEW picture */
               BXDM_PPQM_P_ReleasePictureExt_isr( hXdmPP, pLocalState, pstPictureUnderEvaluation );

               BXDM_PPQM_P_InvalidatePictureContext_isr( pstPictureUnderEvaluation );
            }
            else
            {
               /* Release the OLD picture and display the NEW picture */
               BXDM_PPQM_P_ReleasePictureExt_isr( hXdmPP, pLocalState, pstSelectedPicture );

               BXDM_PPQM_P_PromotePictureToDisplayStatus_isr(
                  hXdmPP,
                  pLocalState,
                  pstPictureUnderEvaluation
                  );

               BXDM_PPQM_P_InvalidatePictureContext_isr( pstPictureUnderEvaluation );

               /* SWSTB-1613: count the number of pictures promoted for display on this vsync. */
               pLocalState->uiPicturesPromoted++;

            }
         }

         /* Currently only used to know if the delivery queue has underflowed. */
         uiLoopCount++;

         BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eSelPic3 );

      }  /* end of while() */
   }

   /* If the FIC changed and we're re-displaying the selected picture
    * (i.e. we haven't selected a new picture), we need to re-evaluate
    * TSM using the new FIC offset so that the TSM values are accurate
    * for logic that relies on it.  E.g. dStcPtsEvaluated will change
    * if FIC changes.
    */
   if ( ( true == pLocalState->bFICChanged )
        && ( true == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated ) )
   {
      BXDM_PPTSM_P_EvaluateTsmState_isr(
         hXdmPP,
         pLocalState,
         pstSelectedPicture
         );
   }

   /* SW7335-781: Output warning when picture selection override ceases */
   BXDM_PPDBG_P_PrintEndSelectionModeOverride_isr(hXdmPP, pstSelectedPicture);

   BDBG_LEAVE(BXDM_PP_S_SelectPicture_isr);
   return;
} /* end of BXDM_PP_S_SelectPicture_isr() */

static void BXDM_PP_S_SelectElement_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   BXDM_PictureProvider_P_Picture_Params* pstPicParms= &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms);

   BDBG_ENTER(BXDM_PP_S_SelectElement_isr);

   /*
   ** Selects best PPB from pictures in delivery queue using TSM */
   BXDM_PP_S_SelectPicture_isr( hXdmPP, pLocalState );

   /* Keep track of consecutively late unique PPBs */
   if ( !pstPicParms->stDisplay.stDynamic.bPPBRepeated )
   {
      if ( BXDM_PictureProvider_TSMResult_eTooLate == pstPicParms->stTSM.stDynamic.eTsmResult )
      {
         /* SW7435-1790: This change came about in response to a stream being played in "live" mode.
          * At a splice point in the stream, the PCR offset jumped and the PTS values were
          * marked as invalid.  The PCR jump caused the time base to change but the invalid PTS's were
          * interpreted on the old time line.  The result was that the pictures were all late and
          * eventually the "drop@decode" logic would kick in; in response to the drop requests, XDM
          * would switch to vsync mode. When the PCR and PTS values were finally back in sync, the
          * video would pause until the TSM comparison matured.
          * The "drop@decode" logic is triggered by " uiPPBConsecutiveLateCount".  To avoid this
          * whole scenario, only increment "uiPPBConsecutiveLateCount" if the PTS is valid.
          */
         if ( true == hXdmPP->stDMState.stChannel.stSelectedPicture.pstUnifiedPicture->stPTS.bValid )
         {
            hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount++;
         }
      }
      else
      {
         hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount=0;
      }
   }

   /* PR47521: We reset the late count also if the queue was
    * completely empty during this vsync.  An empty queue is an
    * indication of packet loss or other stream discontinuity, so we
    * don't want to prematurely trigger the drop@decode logic in these
    * cases because after a few pictures, the system should settle
    * down */
   if ( pLocalState->uiPicturesEvaluated == 0)
   {
      hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount=0;
   }

   /* We reset the late count if the PCR is discontinuous */
   if ( ( false == hXdmPP->stDMConfig.bPlayback )
        && pstPicParms->stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded
      )
   {
      hXdmPP->stDMState.stDecode.uiPPBConsecutiveLateCount=0;
   }

   /* Set the bElementRepeated flag */
   if ( pstPicParms->stDisplay.stDynamic.bPPBRepeated
        && (hXdmPP->stDMState.stChannel.stPreviousPicture.stPicParms.stDisplay.stDynamic.uiSelectedElement != pstPicParms->stDisplay.stDynamic.uiSelectedElement)
      )
   {
      pstPicParms->stDisplay.stDynamic.bElementRepeated = false;
   }

   BDBG_LEAVE(BXDM_PP_S_SelectElement_isr);
   return;
} /* end of BXDM_PP_S_SelectElement_isr() */

/*******************************************************************************
 **
 ** Public functions
 **
 *******************************************************************************/
BERR_Code
BXDM_PictureProvider_GetPicture_isr(
   void* pPrivateContext,
   const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo,
   BAVC_MVD_Picture **pDispMgrToVDC
   )
{
   BXDM_PictureProvider_Handle hXdmPP = (BXDM_PictureProvider_Handle) pPrivateContext;
   /*BXDM_PictureProvider_P_LocalState stLocalDmState;*/
   BXDM_PictureProvider_P_LocalState* pLocalState;
   bool bPrimaryPicture;
   uint32_t i;

   BDBG_ASSERT(pDispMgrToVDC);

   BDBG_ENTER(BXDM_PictureProvider_GetPicture_isr);

   /* Snapshot the start time. */
   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eMainIsr );

   /*
    * Initialize the local variables.
    */

   /* SW7584-344: move the per vsync local state from
    * the stack to the XDM context. */
   pLocalState = &(hXdmPP->stPerVsyncState);

   BKNI_Memset( pLocalState, 0, sizeof( BXDM_PictureProvider_P_LocalState ) );
   for (i = 0; i < BXDM_PictureProvider_P_PTSIndex_eMax; i++)
   {
      pLocalState->iStcPtsDifferenceEvaluatedDeltaAbsMin[i] = 0x7FFFFFFE;
      pLocalState->iStcPtsDifferenceEvaluatedBest[i] = 0x7FFFFFFF;
   }

   /* To aid with debug. */
   pLocalState->bInitialSTCValid = hXdmPP->stDMConfig.bSTCValid;

   /* SW7405-4736: print the input parameters before getting started.
    * Only those that have changed since the last execution will be printed.
    */
   BXDM_PPDBG_P_PrintDMConfig_isr( hXdmPP, pLocalState, false );

   /*
    * Perform per vsync validation; primarily evaluating the XVD and firmware state.
    */
   BXDM_PP_S_InitPerVsync_isr( hXdmPP, pLocalState, pstDisplayInterruptInfo );

   /* SW7405-3137: "pLocalState->eInterruptType" indicates what type of picture VDC is
    * expecting to have delivered.  If VDC is NOT expecting the dependent picture of
    * a set, then perform TSM, generate the MFD data, update status ...
    */
   bPrimaryPicture = ( BXDM_PictureProvider_P_InterruptType_eDependent != pLocalState->eInterruptType );

   if ( true == bPrimaryPicture )
   {
      /*
       * Select the best element from a PPB using TSM, polarity inversion correction, and repeat modes.
       * But only if the delivery queue is in a known state, i.e. when the decoder is up and running.
       *
       * Note:
       * - "eVideoState" is set to "BXVD_P_VideoState_DecodeStopped" in "BXVD_StopDecode()".
       * - it is set to "BXVD_P_VideoState_DecodeStarted" at the end of "BXVD_P_DisplayManager_StartDecode()"
       *       ("BXVD_P_DisplayManager_StartDecode()" is called from "BXVD_StartDecode()".
       */
      BXDM_PP_S_SelectElement_isr( hXdmPP, pLocalState );

      /*
       * Now that a picture has been selected, generate the picture data for VDC.
       */
      BXDM_PPOUT_P_CalculateVdcData_isr( hXdmPP, pLocalState, pDispMgrToVDC );

      /*
       * Update the status that is visible to the application, e.g. "stCurrentPTSInfo"
       */
      BXDM_PP_S_UpdatePublicStatus_isr( hXdmPP,  pLocalState );

      /*
       * Conditionally execute the callbacks.
       */
      BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eMainCallbacks );

      BXDM_PPCB_P_ExecuteCallbacks_isr( hXdmPP, pLocalState );

      BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eMainCallbacks );

      /* SW7425-2536: if registered, execute "displayInterruptStatus_isr" to return
       * the status of the currently selected picture and other assorted information.
       */
      if ( NULL != hXdmPP->stDMConfig.stDecoderInterface.displayInterruptStatus_isr )
      {
         BXDM_PictureProvider_Status stStatus;

         BXDM_PP_S_UpdateStatus_isr( hXdmPP, pLocalState, &(stStatus) );

         hXdmPP->stDMConfig.stDecoderInterface.displayInterruptStatus_isr(
                           hXdmPP->stDMConfig.pDecoderPrivateContext,
                           &stStatus
                           );
      }

      /* Handle bMute AFTER the callbacks have occurred to ensure the
       * app hasn't requested a mute in one of the callbacks */
      if ( true == hXdmPP->stDMConfig.bMute )
      {
         BAVC_MVD_Picture *pMFDPicture = *pDispMgrToVDC;
         while ( NULL != pMFDPicture )
         {
            pMFDPicture->bMute = hXdmPP->stDMConfig.bMute;
            pMFDPicture = (BAVC_MVD_Picture *) pMFDPicture->pNext;
         }
      }
   }
   else
   {
      *pDispMgrToVDC = &(hXdmPP->astMFDPicture[0]);

      {
         BAVC_MVD_Picture *pMFDPicture = *pDispMgrToVDC;
         while ( NULL != pMFDPicture )
         {
            pMFDPicture->eInterruptPolarity = pLocalState->eVsyncPolarity;

            /* SWBLURAY-20436: toggle the source polarity on the dependent interrupt. */
            if ( BAVC_Polarity_eTopField == pMFDPicture->eSourcePolarity )
            {
               pMFDPicture->eSourcePolarity = BAVC_Polarity_eBotField;
            }
            else if ( BAVC_Polarity_eBotField == pMFDPicture->eSourcePolarity )
            {
               pMFDPicture->eSourcePolarity = BAVC_Polarity_eTopField;
            }

            pMFDPicture = (BAVC_MVD_Picture *) pMFDPicture->pNext;
         }
      }

      BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eDependentPicture );  /* TSM */
      BXDM_PPDBG_P_OutputLog_isr( hXdmPP, pLocalState, *pDispMgrToVDC );                /* T/B/F: */
      BXDM_PPDBG_P_OutputSPOLog_isr( hXdmPP, 0 );                                       /* SPO */
      BXDM_PPDBG_P_CallbackTriggeredLog_isr(hXdmPP, pLocalState->uiCallbackTriggered);  /* CB */
   }

   /*
    * Update the post run state and status.
    */
   BXDM_PP_S_PostIsr_isr( hXdmPP, pLocalState, *pDispMgrToVDC, bPrimaryPicture );

   /* Snapshot the end time. */
   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eMainIsr );

   BXDM_PPTMR_P_PrintResults_isr( hXdmPP );

   BDBG_LEAVE(BXDM_PictureProvider_GetPicture_isr);
   return BERR_TRACE(BERR_SUCCESS);
} /* end of BXDM_PictureProvider_GetPicture_isr() */

BERR_Code
BXDM_PictureProvider_StartDecode_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_StartDecode_isr);

#if 0    /* SW7445-1259: reduce messages printed at start decode time */
   if (!hXdmPP->stDMState.stChannel.bDMVersionDisplayed)
   {
      BXVD_DBG_WRN(hXdmPP,("Display Manager:[%02x.xxx] [v%02d.%02d.%04d]",
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                BXDM_PictureProvider_P_VERSION_MAJOR,
                                BXDM_PictureProvider_P_VERSION_MINOR,
                                BXDM_PictureProvider_P_VERSION_SUBMINOR));
      hXdmPP->stDMState.stChannel.bDMVersionDisplayed = true;
   }
#endif
   BXDM_PPDBG_P_PrintStartDecode_isr(hXdmPP);

   /* SW7405-3984: if the "last" picture is being held but the channel change mode
    * does not indicate to "hold last picture", the mode changed from "hold" to "mute" after
    * StopDecode was called.  Release the picture at this point to avoid redisplaying it.
    *
    * Note: to avoid releasing a picture on a "flush decode", this test needs to happen prior to the
    * logic below which copies "stSavedChannelChangeSettings" to "stChannelChangeSettings".
    * The call to BXVD_DisableForFlush results in "bHoldLastPicture" being set to "true"
    * in BXDM_PictureProvider_StopDecode_isr. In turn, the restore below could result in
    * "bHoldLastPicture" being set to "false".  In that situation, the held picture would be
    * release prematurely.
    */
   if ( true == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bHolding
        && false == hXdmPP->stDMConfig.stChannelChangeSettings.bHoldLastPicture
      )
   {
      /* This check should be redundant.  Keep here in the event I've missed a corner case.
       */
      if ( NULL != hXdmPP->stDMState.stChannel.stSelectedPicture.pstUnifiedPicture )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eQM, " %x:[%02x.%03x] %s: CCM was changed to mute, release last picture",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiPPBIndex & 0xFFF,
                             __FUNCTION__ ));

         BXDM_PPQM_P_ReleasePicture_isr( hXdmPP, &hXdmPP->stDMState.stChannel.stSelectedPicture );
      }
   }

   /* SW7425-2656: both of the following will only be true if a "flush" has occured,
    * i.e. BXVD_DisableForFlush, BXVD_SetSkipPictureModeDecode or BXVD_PVR_SetHostSparseMode
    * has been called.  Used in BXDM_PPVTSM_P_VirtualPtsInterpolate_isr to determine how to reset
    * the virtual PTS.
    */
   hXdmPP->stDMState.stChannel.bPostFlushDecode = ( true == hXdmPP->stDMConfig.stPreserveStateSettings.bDisplay );
   hXdmPP->stDMState.stChannel.bPostFlushDecode &= ( true == hXdmPP->stDMState.stChannel.bSavedChannelChangeSettingsValid );

   if ( false == hXdmPP->stDMConfig.stPreserveStateSettings.bDisplay )
   {
      /* Discard threshold defaults. PR: 26199 */
      switch (hXdmPP->stDMConfig.eProtocol)
      {
         case BAVC_VideoCompressionStd_eH264:
         case BAVC_VideoCompressionStd_eH265:
         case BAVC_VideoCompressionStd_eSVC:
         case BAVC_VideoCompressionStd_eSVC_BL:
         case BAVC_VideoCompressionStd_eMVC:
            hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooEarlyThreshold =
               BXDM_PictureProvider_P_DISCARD_THRESHOLD_AVC;
            break;
         case BAVC_VideoCompressionStd_eMPEG2:
         case BAVC_VideoCompressionStd_eMPEG1:
         case BAVC_VideoCompressionStd_eMPEG2DTV:
         case BAVC_VideoCompressionStd_eMPEG4Part2:
         case BAVC_VideoCompressionStd_eH263:
            hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooEarlyThreshold =
               BXDM_PictureProvider_P_DISCARD_THRESHOLD_MPEG;
            break;
         case BAVC_VideoCompressionStd_eVC1:
         case BAVC_VideoCompressionStd_eVC1SimpleMain:
            hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooEarlyThreshold =
               BXDM_PictureProvider_P_DISCARD_THRESHOLD_VC1;
            break;
         default:
            hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooEarlyThreshold =
               BXDM_PictureProvider_P_DISCARD_THRESHOLD_MPEG;
            break;
      }

      hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooLateThreshold =
         BXDM_PictureProvider_P_VERYLATE_THRESHOLD;
   } else {
      if ( true == hXdmPP->stDMState.stChannel.bSavedChannelChangeSettingsValid )
      {
         hXdmPP->stDMConfig.stChannelChangeSettings = hXdmPP->stDMState.stChannel.stSavedChannelChangeSettings;
      }
   }
   hXdmPP->stDMState.stChannel.bSavedChannelChangeSettingsValid = false;

   hXdmPP->stDMStatus.bNextPTSInfoValid = false;

   /* Conditionally Reset all the status counters */
   if ( false == hXdmPP->stDMConfig.stPreserveStateSettings.bCounters )
   {
      BKNI_Memset( &(hXdmPP->stDMStatus.stCounters), 0, sizeof( BXDM_PictureProvider_Counters ) );
   }

   /* Reset the preserve state settings to the defaults */
   BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe(
            hXdmPP,
            &hXdmPP->stDMConfig.stPreserveStateSettings
            );

   BXDM_PP_S_ResetCommon_isr( hXdmPP );

   hXdmPP->stDMState.stChannel.eDecodeState = BXDM_PictureProvider_P_DecodeState_eStarted;

   BDBG_LEAVE(BXDM_PictureProvider_StartDecode_isr);

   return BERR_TRACE(BERR_SUCCESS);
} /* end of BXVD_P_DisplayManager_StartDecode() */


BERR_Code
BXDM_PictureProvider_WatchdogReset_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_WatchdogReset_isr);

   /* Re-initialize the "stSelectedPicture" structure to be certain we don't try to do use the PPB pointers. */

   BKNI_Memset( &(hXdmPP->stDMState.stChannel.stSelectedPicture), 0, sizeof ( BXDM_PictureProvider_P_Picture_Context ));

   /* PR53039: also be certain that the PTS type is "invalid" */
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;

   hXdmPP->stDMState.stChannel.eDecodeState = BXDM_PictureProvider_P_DecodeState_eStopped;

   BDBG_LEAVE(BXDM_PictureProvider_WatchdogReset_isr);

   return BERR_TRACE( BERR_SUCCESS );
} /* end of BXDM_PictureProvider_WatchdogReset_isr() */

BERR_Code
BXDM_PictureProvider_StopDecode_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_StopDecode_isr);

   BXDM_PPDBG_P_PrintStopDecode_isr(hXdmPP);

   hXdmPP->stDMState.stChannel.eDecodeState = BXDM_PictureProvider_P_DecodeState_eStopped;

   /* SW7425-4201: bLast should not span a stop/start decode, reset the appropriate state. */
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bLastPictureSeen = false;
   hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bSetLastPictureFlag = false;

   if ( true == hXdmPP->stDMConfig.stPreserveStateSettings.bDisplay )
   {
      hXdmPP->stDMState.stChannel.stSavedChannelChangeSettings = hXdmPP->stDMConfig.stChannelChangeSettings;
      hXdmPP->stDMState.stChannel.bSavedChannelChangeSettingsValid = true;

      hXdmPP->stDMConfig.stChannelChangeSettings.bHoldLastPicture = true;
   }

   /* eChannelChangeMode: Handle hold last picture feature of channel
    * change - BXVD_StopDecode() */
   if ( true == hXdmPP->stDMConfig.stChannelChangeSettings.bHoldLastPicture )
   {
      hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bHolding = true;
   }
   else
   {
      if ( NULL != hXdmPP->stDMState.stChannel.stSelectedPicture.pstUnifiedPicture )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eQM, " %x:[%02x.%03x] %s: bHoldLastPicture is false, release last picture",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiPPBIndex & 0xFFF,
                             __FUNCTION__  ));

         BXDM_PPQM_P_ReleasePicture_isr( hXdmPP, &hXdmPP->stDMState.stChannel.stSelectedPicture );
      }
   }

   BXDM_PPDBG_P_Print_isr(hXdmPP, (BXDM_PictureProvider_P_LocalState *)&hXdmPP->stPerVsyncState, true);


   /* SW7439-929: If there is a pending clip request and the first picture has NOT
    * been received, assume that the clip request is for the current video segment
    * and clear it. This prevents a stale request from being applied to the next
    * video segment. */

   if ( false == hXdmPP->stDMState.stDecode.bFirstPPBSeen )
   {
      BKNI_Memset(&hXdmPP->stDMConfig.stClipTimeSettings, 0, sizeof( BXDM_PictureProvider_ClipTimeSettings ));
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eDBG, " %x:[%02x.%03x] %s: clearing clip request",
                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                          hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiPPBIndex & 0xFFF,
                          __FUNCTION__  ));
   }

   /* PR55616: setting "bFirstPPBSeen" to false will disable the
    * "BXVD_Interrupt_ePictureParameters" callback until the first picture
    * the next stream is delivered to the DM.
    */
   hXdmPP->stDMState.stDecode.bFirstPPBSeen = false;

   if (hXdmPP->stDMConfig.stPreserveStateSettings.bDisplay == false)
   {
      hXdmPP->stDMConfig.bAutoValidateStcOnPause = true;
      hXdmPP->stDMConfig.stPlaybackRate.uiNumerator = 1;
      hXdmPP->stDMConfig.stPlaybackRate.uiDenominator = 1;
      hXdmPP->stDMConfig.bReverseFields = false;

      /* PR21416: Make sure DM is displaying both top and bottom fields */
      hXdmPP->stDMConfig.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;

      hXdmPP->stDMConfig.uiPTSOffset = 0;
   }

   /* SWSTB-1380: when using the debug fifo (as opposed to debug prints),
    * print any outstanding messages on the debug fifo. */
   BXDM_PictureProvider_ReadFifo_isrsafe( hXdmPP );

   BDBG_LEAVE(BXDM_PictureProvider_StopDecode_isr);

   return BERR_TRACE( BERR_SUCCESS );

} /* end of BXDM_PictureProvider_StopDecode_isr() */


BERR_Code
BXDM_PictureProvider_FlushHeldPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_FlushHeldPicture_isr);


   /* This was added for SW7425-1064.  XDM on the enhanced channel is not used when running
    * with the XMO. There needed to be a way to force XDM to release a picture a held
    * picture so that it could be used on this decode.
    */
   if ( true == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bHolding
        && NULL != hXdmPP->stDMState.stChannel.stSelectedPicture.pstUnifiedPicture
      )
   {
      hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.bHolding = false;
      BXDM_PPQM_P_ReleasePicture_isr( hXdmPP, &hXdmPP->stDMState.stChannel.stSelectedPicture );
   }

   return BERR_TRACE(BERR_SUCCESS);

} /* end of BXDM_PictureProvider_FlushHeldPicture_isr() */


#if 0    /* SWSTB-1380: */

static const BXDM_PictureProvider_Settings s_stDefaultPictureProviderSettings =
{
    2 /* uiMaxBuffersPerPicture */
};
#endif

BERR_Code
BXDM_PictureProvider_GetDefaultSettings(
         BXDM_PictureProvider_Settings *pstPictureProviderSettings
         )
{
   BDBG_ENTER( BXDM_PictureProvider_GetDefaultSettings );
   BDBG_ASSERT( pstPictureProviderSettings );

   BKNI_Memset( pstPictureProviderSettings, 0, sizeof( BXDM_PictureProvider_Settings ));

   pstPictureProviderSettings->uiMaxBuffersPerPicture = 2;

   /* *pstPictureProviderSettings = s_stDefaultPictureProviderSettings; */

   BDBG_LEAVE( BXDM_PictureProvider_GetDefaultSettings );

   return BERR_TRACE( BERR_SUCCESS );
}

/*
 * Handle any initialization that only needs to be done once
 * on "OpenChannel"
 */
BERR_Code
BXDM_PictureProvider_Create(
         BXDM_PictureProvider_Handle *phXdmPP,
         const BXDM_PictureProvider_Settings *pstPictureProviderSettings
         )
{
   BERR_Code rc;
   uint32_t i;
   BXDM_PictureProvider_P_Context *pXdmPP = NULL;
   BXDM_PPJRC_P_Settings stJrcSettings;
   BXDM_PPAVG_P_Settings stAvgSettings;

   BDBG_ENTER( BXDM_PictureProvider_Create );

   BDBG_ASSERT( phXdmPP );
   BDBG_ASSERT( pstPictureProviderSettings );

   /* Set the handle to NULL in case the allocation fails */
   *phXdmPP = NULL;

   pXdmPP = ( BXDM_PictureProvider_P_Context* ) BKNI_Malloc( sizeof( BXDM_PictureProvider_P_Context ) );
   if ( NULL == pXdmPP )
   {
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( ( void * ) pXdmPP, 0, sizeof( BXDM_PictureProvider_P_Context ) );

   /* Allocate MFD Pictures */
   pXdmPP->uiMFDPictureCount = pstPictureProviderSettings->uiMaxBuffersPerPicture;
   pXdmPP->astMFDPicture = (BAVC_MFD_Picture *) BKNI_Malloc( sizeof ( BAVC_MFD_Picture ) * pXdmPP->uiMFDPictureCount );
   if ( NULL == pXdmPP->astMFDPicture )
   {
      BXDM_PictureProvider_Destroy( pXdmPP );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset( pXdmPP->astMFDPicture, 0, sizeof ( BAVC_MFD_Picture ) * pXdmPP->uiMFDPictureCount );

   /* This issue was found while trying to reproduce SW7630-85 with a check
    * test program. At startup, the mute flag was not set on the dependent picture.
    * As a result, VDC was going off the rails with the NULL picture pointers.
    */
   for ( i=0; i < pXdmPP->uiMFDPictureCount; i++ )
   {
      pXdmPP->astMFDPicture[ i ].bMute = true;
   }

   BXDM_PPOUT_P_OpenChannel( pXdmPP );

   /* Initialize GOP timecode */
   pXdmPP->stDMStatus.stGOPTimeCode.bValid = false;

   /* Initialize PVR defaults */
   pXdmPP->stDMConfig.bFreeze = false;
   pXdmPP->stDMConfig.bAutoValidateStcOnPause = true;
   pXdmPP->stDMConfig.stPlaybackRate.uiNumerator = 1;
   pXdmPP->stDMConfig.stPlaybackRate.uiDenominator = 1;
   pXdmPP->stDMConfig.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eFrame;
   pXdmPP->stDMConfig.bReverseFields = false;

   pXdmPP->stDMConfig.eDisplayMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;

   pXdmPP->stDMConfig.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;
   pXdmPP->stDMConfig.eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eDefault;

   /* ensure we have a valid refresh rate */
   pXdmPP->stDMConfig.eMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz;

   /* PR50623: the default is to use the HW PCR Offset */
   pXdmPP->stDMConfig.bUseHardwarePCROffset = true;

   pXdmPP->stDMConfig.e240iScanMode = BXDM_PictureProvider_240iScanMode_eForceProgressive;

   pXdmPP->stDMConfig.bJitterToleranceImprovement = true;

   BXDM_PPJRC_P_GetDefaultSettings(&stJrcSettings);
   rc = BXDM_PPJRC_P_Create(
            &pXdmPP->stDMState.stChannel.hJrc,
            &stJrcSettings);

   if ( BERR_SUCCESS != rc )
   {
      BXDM_PictureProvider_Destroy( pXdmPP );
      return BERR_TRACE( rc );
   }

   BXDM_PPAVG_P_GetDefaultSettings(&stAvgSettings);
   stAvgSettings.uiNumSamples = BXDM_PictureProvider_P_RATE_MATCH_DETECTION_SAMPLES;
   rc = BXDM_PPAVG_P_Create(
            &pXdmPP->stDMState.stChannel.hDeltaSTCAvg,
            &stAvgSettings);

   if ( BERR_SUCCESS != rc )
   {
      BXDM_PictureProvider_Destroy( pXdmPP );
      return BERR_TRACE( rc );
   }

   /* SWSTB-1380: if the heap handle has been passed in, create both a
    * debug fifo and a reader to print the data on the fifo. */

   if ( NULL != pstPictureProviderSettings->hHeap )
   {
      BKNI_Memset( &(pXdmPP->stDMConfig.stDebugFifo), 0, sizeof(BXDM_PictureProvider_P_DebugFifo) );
      pXdmPP->stDMConfig.stDebugFifo.hBMMAHeap = pstPictureProviderSettings->hHeap;
      BXDM_PPDFIFO_P_Fifo_Create( pXdmPP );
      BXDM_PPDFIFO_P_Reader_Create( pXdmPP );
   }

   *phXdmPP = pXdmPP;

   BDBG_LEAVE( BXDM_PictureProvider_Create );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_PictureProvider_Destroy(
         BXDM_PictureProvider_Handle hXdmPP
         )
{
   BDBG_ENTER( BXDM_PictureProvider_Destroy );

   BDBG_ASSERT(hXdmPP);

   if ( NULL != hXdmPP->stDMState.stChannel.hDeltaSTCAvg )
   {
      BXDM_PPAVG_P_Destroy( hXdmPP->stDMState.stChannel.hDeltaSTCAvg );
   }

   if ( NULL != hXdmPP->stDMState.stChannel.hJrc )
   {
      BXDM_PPJRC_P_Destroy( hXdmPP->stDMState.stChannel.hJrc );
   }

   if ( NULL != hXdmPP->astMFDPicture )
   {
      BKNI_Free( hXdmPP->astMFDPicture );
   }

   /* SWSTB-1380: if created, destroy both the debug fifo and reader. */
   BXDM_PPDFIFO_P_Reader_Destroy( hXdmPP );
   BXDM_PPDFIFO_P_Fifo_Destroy( hXdmPP );

   BKNI_Free( hXdmPP );

   BDBG_LEAVE( BXDM_PictureProvider_Destroy );

   return BERR_TRACE( BERR_SUCCESS );
}

/*******************************************************************************
 **
 ** end public functions
 **
 *******************************************************************************/
