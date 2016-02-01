/***************************************************************************
*     (c)2005-2013 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#ifndef _BTHD_ACQ_H__
#define _BTHD_ACQ_H__

#if __cplusplus
extern "C" {
#endif

/* #define ChangeFFTWindowSeamless */
#define BTHD_CHK_RETCODE(x) \
  { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

typedef uint32_t BTHD_RESULT;

/***************************************************************************
 * Register Field Combinations Definition
 ***************************************************************************/
#define BCHP_THD_CORE_EQ_EXP_USE_FORMAT_OV_SHIFT BCHP_THD_CORE_EQ_FORMAT_OV_SHIFT 
#define BCHP_THD_CORE_EQ_EXP_USE_FORMAT_OV_MASK  (BCHP_THD_CORE_EQ_FORMAT_OV_MASK | BCHP_THD_CORE_EQ_USE_FORMAT_OV_MASK) 

#define BCHP_THD_CORE_BYP_EXP_AVG_NSE_SHIFT BCHP_THD_CORE_BYP_EXP_NSE_BYP_SHIFT 
#define BCHP_THD_CORE_BYP_EXP_AVG_NSE_MASK  (BCHP_THD_CORE_BYP_EXP_NSE_BYP_MASK | BCHP_THD_CORE_BYP_EXP_AVG_NSE_BYP_MASK) 

#define BCHP_THD_CORE_CE_ACE_ACT_SHIFT BCHP_THD_CORE_CE_ACE_SHIFT 
#define BCHP_THD_CORE_CE_ACE_ACT_MASK  (BCHP_THD_CORE_CE_ACE_MASK | BCHP_THD_CORE_CE_ACT_MASK)

#define BCHP_THD_CORE_RST_RS_FEC_BCH_TPS_SNR_RST_SHIFT BCHP_THD_CORE_RST_SNR_RST_SHIFT 
#define BCHP_THD_CORE_RST_RS_FEC_BCH_TPS_SNR_RST_MASK  (BCHP_THD_CORE_RST_SNR_RST_MASK | BCHP_THD_CORE_RST_TPS_RST_MASK  | BCHP_THD_CORE_RST_TPS_SAVE_RST_MASK | BCHP_THD_CORE_RST_BCH_ERC_RST_MASK | BCHP_THD_CORE_RST_FEC_RST_MASK | BCHP_THD_CORE_RST_RS_RST_MASK) 

#define BCHP_THD_CORE_FRZ_NOTCH_FRZ_SHIFT BCHP_THD_CORE_FRZ_NOTCH0M_FRZ_SHIFT 
#define BCHP_THD_CORE_FRZ_NOTCH_FRZ_MASK  (BCHP_THD_CORE_FRZ_NOTCH0M_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH0_FRZ_MASK  | BCHP_THD_CORE_FRZ_NOTCH1_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH2_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH3_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH4_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH_DDFS_FRZ_MASK)

#define BCHP_THD_CORE_TPS_OV_RX_INFO_SHIFT BCHP_THD_CORE_TPS_OV_QAM_SHIFT
#define BCHP_THD_CORE_TPS_OV_RX_INFO_MASK  (BCHP_THD_CORE_TPS_OV_QAM_MASK | BCHP_THD_CORE_TPS_OV_reserved5_MASK  | BCHP_THD_CORE_TPS_OV_HIERARCHY_MASK | BCHP_THD_CORE_TPS_OV_INDEPTH_MASK | BCHP_THD_CORE_TPS_OV_reserved4_MASK | BCHP_THD_CORE_TPS_OV_CRATE_HP_MASK)

#define BCHP_THD_CORE_TPS_RX_INFO_SHIFT BCHP_THD_CORE_TPS_QAM_SHIFT
#define BCHP_THD_CORE_TPS_RX_INFO_MASK  (BCHP_THD_CORE_TPS_QAM_MASK | BCHP_THD_CORE_TPS_reserved5_MASK  | BCHP_THD_CORE_TPS_HIERARCHY_MASK | BCHP_THD_CORE_TPS_INDEPTH_MASK | BCHP_THD_CORE_TPS_reserved4_MASK | BCHP_THD_CORE_TPS_CRATE_HP_MASK)

#define BCHP_THD_CORE_BYP_EQ_BYP_SHIFT BCHP_THD_CORE_BYP_EQ_BIT_SCALING_BYP_SHIFT
#define BCHP_THD_CORE_BYP_EQ_BYP_MASK  (BCHP_THD_CORE_BYP_EQ_BIT_SCALING_BYP_MASK | BCHP_THD_CORE_BYP_NSE_AVG_BYP_MASK  | BCHP_THD_CORE_BYP_EXP_CHAN_BYP_MASK | BCHP_THD_CORE_BYP_EXP_NSE_BYP_MASK | BCHP_THD_CORE_BYP_EXP_AVG_NSE_BYP_MASK) 

/* Implementation of API functions */

    
/* Internal THD Functions */
BERR_Code BTHD_P_GetChannelEstimateBuf(BTHD_3x7x_ChnHandle, uint32_t *, THD_SnooperMode_t, uint32_t uDataLenOutputMax, uint32_t *puDataLenOutput );
BERR_Code BTHD_P_GetInterpolatorCoefficientsBuf(BTHD_3x7x_ChnHandle, uint32_t *, THD_InterpolatorMode_t, uint32_t uDataLenOutputMax, uint32_t *puDataLenOutput );
BERR_Code BTHD_P_OSleep(BTHD_3x7x_ChnHandle,uint32_t,uint32_t,uint32_t);
void BTHD_P_UpdateStatusChange(BTHD_3x7x_ChnHandle,uint32_t);
BERR_Code BTHD_P_Init(BTHD_3x7x_ChnHandle);
BERR_Code BTHD_P_Acquire(BTHD_3x7x_ChnHandle);
void BTHD_P_SetTs(BTHD_3x7x_ChnHandle);
void BTHD_P_SetBer(BTHD_3x7x_ChnHandle);
void BTHD_P_SetFrequency(BTHD_3x7x_ChnHandle);
void BTHD_P_SetFrontEnd(BTHD_3x7x_ChnHandle);
void BTHD_P_SetNotch(BTHD_3x7x_ChnHandle);
THD_TransGuardResult_t BTHD_P_SetTransGuard(BTHD_3x7x_ChnHandle);

void BTHD_P_SetCE( BTHD_3x7x_ChnHandle,THD_ChannelEstimatorMode_t);
void BTHD_P_WriteFICoef( BTHD_3x7x_ChnHandle,THD_FrequencyInterpolatorMode_t);
void BTHD_P_Status(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_Lock(BTHD_3x7x_ChnHandle);
void BTHD_P_ResetAll(BTHD_3x7x_ChnHandle);
void BTHD_P_ResetAcquire(BTHD_3x7x_ChnHandle);
void BTHD_P_ResetStatus(BTHD_3x7x_ChnHandle);
void BTHD_P_ResetStatusHW(BTHD_3x7x_ChnHandle);
void BTHD_P_Config(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_AcquireInit(BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t);
BTHD_RESULT BTHD_P_AcquireSP(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_AcquireFFTTrigger(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_AcquireTPS(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_AcquireFEC(BTHD_3x7x_ChnHandle,bool);
BTHD_RESULT BTHD_P_AcquireChangeFFTWindow(BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t);
BTHD_RESULT BTHD_P_AcquireTrack(BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t,THD_ChannelEstimatorMode_t);
BTHD_RESULT BTHD_P_AcquireCheckLock(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_AcquireChangeChannelEstimator(BTHD_3x7x_ChnHandle,THD_ChannelEstimatorMode_t);
BTHD_RESULT BTHD_P_MapWaitForEventResult_To_THD_AcqResult ( BERR_Code eEventWaitResult, BTHD_RESULT successCode, BTHD_RESULT failureCode );
bool BTHD_P_Wait_CE_Recording_Status_Done ( BTHD_3x7x_ChnHandle h, uint32_t uTimeOutMsec );
void BTHD_P_AbortAcquire ( BTHD_3x7x_ChnHandle h) ;

/* Sets the Abort-Early event/condition during an acquisition */
void BTHD_P_AbortAcquire ( BTHD_3x7x_ChnHandle h );
/* Resets the Abort-Early event/condition during an acquisition */
void BTHD_P_AbortAcquireReset ( BTHD_3x7x_ChnHandle h );
/* Queries an aquisition's Abort-Early event/condition */
bool BTHD_P_IsAbortAcquireRequested ( BTHD_3x7x_ChnHandle h );
/* Wait for an event (or Abort-Early) during an acquisition */
BERR_Code BTHD_P_WaitForEventOrAbort ( BTHD_3x7x_ChnHandle h, BKNI_EventHandle event, int timeoutMsec );
/* Acquire Aborted Early BERR_Code */
#define BTHD_ERR_ACQUIRE_ABORTED  BERR_MAKE_CODE(BERR_THD_ID, 1)

#ifdef __cplusplus
}
#endif

#endif /* BTHD_ACQ_H__ */

