/******************************************************************************
*     (c)2010-2013 Broadcom Corporation
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
 *****************************************************************************/
/***************************************************************************
*     (c)2005-2013 Broadcom Corporation
*  
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
#include "bstd.h"
#include "bmth.h"
#include "bkni.h"
#include "btmr.h"
#ifndef LEAP_BASED_CODE
#include "bthd_3x7x.h"
#endif
#include "bchp_thd_core.h"
#include "bthd_api.h"
#include "bthd_acq.h"
#include "bthd_acq_dvbt.h"
#include "bthd_coef.h"

#include "bchp_thd_intr2.h"
#include "bchp_thd_intr2b.h"
#ifdef LEAP_BASED_CODE
#include "bchp_leap_ctrl.h"
#include "bthd_irq.h"
#endif

#ifndef LEAP_BASED_CODE
BDBG_MODULE(bthd_acq_dvbt);
#endif

/***************************************************************************
 * BTHD_P_DvbtSetFWFtt()
 ***************************************************************************/
void BTHD_P_DvbtSetFWFtt( BTHD_3x7x_ChnHandle h, 
		   THD_FFTWindowMode_t FFTWindowMode, 
		   THD_TransmissionMode_t TransmissionMode, 
		   THD_GuardInterval_t GuardInterval)
{

  uint32_t fw=0,fw_search=0,N=bthd_transmission_mode[TransmissionMode];
  BSTD_UNUSED(FFTWindowMode);
  BSTD_UNUSED(GuardInterval);

  /* First Peak */
  fw_search = 0x11202000 | (8 * (N/2048));  /* FFT window min_scale=1.125 (0.5dB),start_index_mode=1,1st peak mode,L=8*N/2048 */
  /* fw_search = 0x11802000 | (8 * (N/2048)); */  /* FFT window min_scale=1.5,start_index_mode=1,1st peak mode,L=8*N/2048*/
  /*fw = 0x03214050; */  /* Decrease average beta from 2^(-7) to 2^(-6) for freq-domain trigger position algorithm for faster convergence time */
  fw = 0x03214051; /* Decrease average beta from 2^(-7) to 2^(-6) for freq-domain trigger position algorithm for faster convergence time, enable slip */

  /* Disable expanded search mode for GI=1/4 due to acquisition instability */
  /* if (GuardInterval == 3) */
  fw = fw & 0xfcffffff;

#ifdef FASTACQ
  fw = (fw & 0xffff00ff) | 0x00002000;
#endif
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FW,fw );
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FW_SEARCH,fw_search);
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FW_OFFSET,0x00000010 );  /* FFT window offset=16 */
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FW_SPAN,0x00000200 );      /* FFT window span scale = 0.25 */
}


/******************************************************************************
 * BTHD_P_GetDvbtSoftDecisionBuf()
 ******************************************************************************/
BERR_Code BTHD_P_GetDvbtSoftDecisionBuf(BTHD_3x7x_ChnHandle h,  /* [in] BTHD handle */
    int16_t nbrToGet,                 /* [in] Number values to get */
    int16_t *pI,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *pQ,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                /* [out] Number of values gotten/read */
    )
{

  uint32_t val, tps;
  uint8_t idx=0;
  uint32_t N = bthd_transmission_mode[BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, TRANS_MODE )];
  uint32_t GuardInterval = BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, GUARD);
  BSTD_UNUSED(nbrGotten);
  BSTD_UNUSED(nbrToGet);
  
  tps = BREG_Read32(h->hRegister, BCHP_THD_CORE_TPS);
  tps &= BCHP_THD_CORE_TPS_QAM_MASK;
  tps >>= BCHP_THD_CORE_TPS_QAM_SHIFT;
  
  while (idx < 30) {
    val = BREG_Read32(h->hRegister, BCHP_THD_CORE_SOFT_READ_DATA);
    pI[idx] = (int16_t)((val & 0xFFFC0000) >> (14+tps));
    pQ[idx] = (int16_t)((val & 0x0003fFF0) >> tps);
    idx++;
    BTHD_P_OSleep(h,1,N,GuardInterval);
  }

  return BERR_SUCCESS;
}
/***************************************************************************
 * BTHD_P_DvbtSetMode()
 ***************************************************************************/
void BTHD_P_DvbtSetMode( BTHD_3x7x_ChnHandle h, 
			 THD_TransmissionMode_t TransmissionMode, 
			 THD_GuardInterval_t GuardInterval, 
			 THD_Qam_t Qam, 
			 THD_CodeRate_t CodeRateHP, 
			 THD_CodeRate_t CodeRateLP, 
			 THD_DvbtHierarchy_t Hierarchy)
{
  uint32_t  tps;
  tps = (Qam << 4) | (Hierarchy << 8) | (CodeRateHP << 12) | (CodeRateLP << 16) | (GuardInterval << 20) | (TransmissionMode << 24) | (1U << 31);
  BREG_Write32(h->hRegister, BCHP_THD_CORE_TPS_OV, tps );
}

/***************************************************************************
 * BTHD_P_DvbtSetTPS()
 ***************************************************************************/
BTHD_RESULT BTHD_P_DvbtSetTPS( BTHD_3x7x_ChnHandle h, 
			       THD_TransmissionMode_t TransmissionMode, 
			       THD_GuardInterval_t GuardInterval)
{
  BTHD_RESULT return_val = THD_AcquireResult_InitLockState;
  uint32_t tps,N=bthd_transmission_mode[TransmissionMode];
  BERR_Code eEventWaitResult;

  BREG_WriteField(h->hRegister, THD_CORE_RST, TPS_RST, 1 ); /* Force TPS reset */
  BREG_WriteField(h->hRegister, THD_CORE_RST, TPS_RST, 0 ); /* Release TPS reset */

  if (h->pAcquireParam->StdAcquire.DvbtAcquireParam.TPSMode == THD_DvbtTPSMode_Manual) {
    BREG_WriteField(h->hRegister, THD_CORE_TPS_OV, QAM, h->pAcquireParam->StdAcquire.DvbtAcquireParam.Qam);
    BREG_WriteField(h->hRegister, THD_CORE_TPS_OV, HIERARCHY, h->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy);
    BREG_WriteField(h->hRegister, THD_CORE_TPS_OV, CRATE_HP,  h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP);
    BREG_WriteField(h->hRegister, THD_CORE_TPS_OV, CRATE_LP,  h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP);
  } else {
#if (BTHD_P_BCHP_THD_CORE_VER == BTHD_P_BCHP_CORE_V_4_0)
	if (BKNI_CreateEvent(&(h->hTpsSyncEvent)) != BERR_SUCCESS)
		BDBG_ERR(("Not enough event for hTpsSyncEvent"));
#endif
    BINT_EnableCallback( h->hTpsSyncCallback);
     eEventWaitResult = BTHD_P_WaitForEventOrAbort(h, h->hTpsSyncEvent, 150);
     /* eEventWaitResult success: return_val, Failure: THD_AcquireResult_NoTPSLock, or,
     * THD_AcquireResult_AbortedEarly if "Acquire Aborted Early" Event took place */
     return_val = BTHD_P_MapWaitForEventResult_To_THD_AcqResult(eEventWaitResult, return_val, THD_AcquireResult_NoTPSLock);


	BINT_DisableCallback( h->hTpsSyncCallback);
#if (BTHD_P_BCHP_THD_CORE_VER == BTHD_P_BCHP_CORE_V_4_0)
	BKNI_DestroyEvent(h->hTpsSyncEvent);
#endif
    if (return_val != THD_AcquireResult_NoTPSLock && return_val != THD_AcquireResult_AbortedEarly) {
      eEventWaitResult = BTHD_P_OSleep(h,1,N,GuardInterval);
      tps = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS );
      tps = (BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV) & 0xfff00000) | (tps & 0x000fffff);
      BREG_Write32(h->hRegister,  BCHP_THD_CORE_TPS_OV, tps );
      if ( eEventWaitResult == BTHD_ERR_ACQUIRE_ABORTED ) {
        return_val = THD_AcquireResult_AbortedEarly;
      }
    }
  }
  return return_val;
}

/***************************************************************************
 * BTHD_P_DvbtSetOI()
 ***************************************************************************/
void BTHD_P_DvbtSetOI( BTHD_3x7x_ChnHandle h )
{
  const bthd_oi_cw_t *cw;
  uint32_t tps;
  THD_GuardInterval_t GuardInterval;
  THD_Qam_t Qam;
  THD_DvbtHierarchy_t Hierarchy;
  THD_CodeRate_t CodeRateLP, CodeRateHP, CodeRate;

  tps = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV );
  GuardInterval = (tps >> 20) & 0x3;
  Qam = (tps >> 4) & 0x3;
  Hierarchy  = (tps >> 8) & 0x3;
  CodeRateHP = (tps >> 12) & 0x7;
  CodeRateLP = (tps >> 16) & 0x7;

  if ( h->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode == THD_DvbtPriorityMode_High) {
    CodeRate = CodeRateHP;
    if (Hierarchy)
      Qam = THD_Qam_Qpsk;
  } else {
    CodeRate = CodeRateHP;
    if (Hierarchy)
      Qam -= 1;
  }
  
  cw = &bthd_oi_table[GuardInterval][Qam][CodeRate];
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_OI_N, cw->n );
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_OI_D, cw->d );
}

/***************************************************************************
 * BTHD_P_DvbtSetEq()
 ***************************************************************************/
void BTHD_P_DvbtSetEq( BTHD_3x7x_ChnHandle h, 
		       THD_CoChannelMode_t CoChannelMode)
{
  uint32_t format_ov,exp_offset=0,tps;
  THD_Qam_t Qam;
  THD_DvbtHierarchy_t Hierarchy;
  THD_TransmissionMode_t TransmissionMode;

  const uint8_t format_ov_table_cci_off[][3] = {{0xd,0xe,0xe},
						 {0xd,0xe,0xe},
						 {0xd,0xe,0xf},
						 {0xd,0xe,0xf}};
  const uint8_t format_ov_table_cci_on [][3] = {{0xb,0xb,0xb},
						 {0xb,0xb,0xb},
						 {0xb,0xe,0xf},
						 {0xb,0xe,0xf}};
  const uint32_t exp_offset_table_cci_off[][3] = {{0x000,0x3f0,0x3f0},
						   {0x000,0x3f8,0x3f8},
						   {0x000,0x3f4,0x3f4}};
  const uint32_t exp_offset_table_cci_on [][3] = {{0x000,0x000,0x000},
						   {0x000,0x000,0x000},
						   {0x000,0x000,0x000}};

  tps = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV );
  TransmissionMode = (tps >> 24) & 0x3;
  Qam = (tps >> 4) & 0x3;
  Hierarchy  = (tps >> 8) & 0x3;

  if (CoChannelMode == THD_CoChannelMode_None) {
    format_ov  = format_ov_table_cci_off[Hierarchy][Qam];
    exp_offset = exp_offset_table_cci_off[TransmissionMode][Qam];
  } else {
    format_ov  = format_ov_table_cci_on[Hierarchy][Qam];
    exp_offset = exp_offset_table_cci_on[TransmissionMode][Qam];
  }

  BREG_WriteField(h->hRegister, THD_CORE_EQ, EXP_USE_FORMAT_OV, format_ov );
  BREG_WriteField(h->hRegister, THD_CORE_EQ, EXP_OFFSET, exp_offset );      
}


/***************************************************************************
 * BTHD_P_DvbtSetViterbi()
 ***************************************************************************/
void BTHD_P_DvbtSetViterbi( BTHD_3x7x_ChnHandle h, 
			    THD_CodeRate_t CodeRateHP, 
			    THD_CodeRate_t CodeRateLP)
{
  uint32_t Dly;
  THD_CodeRate_t CodeRate;
  const uint32_t DlyTable[] = {0x000,0x110,0x208,0x310,0x518};

  if (h->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode == THD_DvbtPriorityMode_High)
    CodeRate = CodeRateHP;
  else
    CodeRate = CodeRateLP;
  Dly = DlyTable[CodeRate];

  BREG_Write32(h->hRegister,  BCHP_THD_CORE_VIT_OVS,Dly );        /* Viterbi overwrite sync mode delay */
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_VIT,0x00000607 );     /* Viterbi overwrite sync mode enabled */
}

/***************************************************************************
 * BTHD_P_DvbtSetFEC()
 ***************************************************************************/
BTHD_RESULT BTHD_P_DvbtSetFEC( BTHD_3x7x_ChnHandle h )
{

  BTHD_RESULT return_val = THD_AcquireResult_InitLockState;
  uint32_t tps;
  THD_CodeRate_t CodeRateLP, CodeRateHP;
  BERR_Code eEventWaitResult;

  tps = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV );
  CodeRateHP = (tps >> 12) & 0x7;
  CodeRateLP = (tps >> 16) & 0x7;

  if ( h->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode == THD_DvbtPriorityMode_High)
	  BREG_WriteField(h->hRegister, THD_CORE_FEC, HP, 1);
  else
	  BREG_WriteField(h->hRegister, THD_CORE_FEC, HP, 0);
  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FEC_SYNC,0x07021f1e ); /* FEC sync allow 2 bad headers out of 8 */
  BTHD_P_DvbtSetEq(h, THD_CoChannelMode_None);
  BTHD_P_DvbtSetViterbi(h, CodeRateHP, CodeRateLP);
  BREG_WriteField(h->hRegister, THD_CORE_FEC, SYNC_MODE, 1 );
  BREG_WriteField(h->hRegister, THD_CORE_RST, FEC_RST, 0 );  
  BREG_WriteField(h->hRegister, THD_CORE_RST, RS_RST, 0 );
#if (BTHD_P_BCHP_THD_CORE_VER == BTHD_P_BCHP_CORE_V_4_0)
  if (BKNI_CreateEvent(&(h->hFecSyncEvent)) != BERR_SUCCESS)
		BDBG_ERR(("Not enough event for hFecSyncEvent"));
#endif
  BINT_EnableCallback( h->hFecSyncCallback);

  eEventWaitResult = BTHD_P_WaitForEventOrAbort(h, h->hFecSyncEvent, 100);
  /* eEventWaitResult success: return_val, Failure: THD_AcquireResult_NoFECLock, or,
  * THD_AcquireResult_AbortedEarly if "Acquire Aborted Early" Event took place */
  return_val = BTHD_P_MapWaitForEventResult_To_THD_AcqResult(eEventWaitResult, return_val, THD_AcquireResult_NoFECLock);


  BINT_DisableCallback( h->hFecSyncCallback);
#if (BTHD_P_BCHP_THD_CORE_VER == BTHD_P_BCHP_CORE_V_4_0)
  BKNI_DestroyEvent(h->hFecSyncEvent);
#endif
  return return_val;
}
/***************************************************************************
 * BTHD_P_DvbtGetNotch()
 ***************************************************************************/
BTHD_RESULT BTHD_P_DvbtGetNotch( BTHD_3x7x_ChnHandle h, 
				 THD_TransmissionMode_t TransmissionMode)
{ 
  uint32_t iteration=0, start=0,idx,value,cur_exp,min_exp=0,mean_exp,min_idx,NumCoChannelFound = 0, cci_threshold;
  bool CoChannelPresent = false;

  /* Determine whether analog TV CCI is present using snooper. 
     Look for the presence of any 2 of the video, color, or audio carriers.  
     This 2-out-of-3 criterion allows differentiation between true analog TV CCI 
     and spurs which often occur around sensitivity. If this criterion is too 
     strict, consider using a 1-out-of-3 criterion for input power levels above 
     sensitivity and this 2-out-of-3 criterion for input power levels near sensitivity */
  iteration = 0;
  while ((iteration < 3) && !CoChannelPresent) {
    switch (iteration) {
    case 0: if (TransmissionMode == THD_TransmissionMode_2k) start=000; else start = 0561; break;  /* Video carrier */
    case 1: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 5280; break;  /* Color carrier */
    case 2: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 6048; break;  /* Audio carrier */
    }
    BREG_Write32(h->hRegister,  BCHP_THD_CORE_CE_RECORD_CFG,0x07030000 + start );  /* CE snooper capture exponents, snapshot mode, step=3 */

    /* while (!BREG_ReadField(h->hRegister,  THD_CORE_CE_RECORD_CFG, DONE)); */
	if ( ! BTHD_P_Wait_CE_Recording_Status_Done(h, 1000) ) { /* units are in msec */
	  BDBG_MSG(("BTHD_P_DvbtGetNotch timeout waiting for THD_CORE_CE_RECORD_CFG DONE"));
	  goto BTHD_P_DvbtGetNotch_Abort;
	}
    BREG_Write32(h->hRegister,  BCHP_THD_CORE_CE_READ_INDEX,0x00000000 );
    for (idx=0; idx<256; idx++) {
      value = BREG_Read32(h->hRegister,  BCHP_THD_CORE_CE_READ_DATA );
      cur_exp = (value & 0x0000fc00) >> 10;
      if ((idx == 0) || (cur_exp < min_exp)) { /* Compute min, smaller noise exponent -> larger noise */
        min_exp = cur_exp;
        min_idx = idx;
      }
    }
    mean_exp = (value & 0x000003f0) >> 4;
    
    if (TransmissionMode == THD_TransmissionMode_2k)
        cci_threshold = 0x0f;
    else
        cci_threshold = 0x0c;
	
	
    if (mean_exp > (min_exp + cci_threshold))           /* CCI present if min_exp-mean_exp > cci_threshold */
        NumCoChannelFound++;
    if (NumCoChannelFound > 1)
        {CoChannelPresent = true;}
    
    iteration++;
  }
  /* Normal-Exit */
  goto BTHD_P_DvbtGetNotch_Exit;
  
  /* Abort-Exit */
BTHD_P_DvbtGetNotch_Abort:
  CoChannelPresent = false;
  goto BTHD_P_DvbtGetNotch_Exit;
  
BTHD_P_DvbtGetNotch_Exit:
  return CoChannelPresent;
}

/***************************************************************************
 * BTHD_P_DvbtSetICE ()
 ***************************************************************************/
BTHD_RESULT BTHD_P_DvbtSetICE( BTHD_3x7x_ChnHandle h, 
			       THD_TransmissionMode_t TransmissionMode,
			       THD_GuardInterval_t GuardInterval)
{
  int n,k;
  uint32_t ice[16],ice_val[16],ice_cnt[16],ice_found,ice_val_max,ice_cnt_max,ice_index_max;
  uint32_t ice_cnt_best=0,ice_val_best=0,ice_index_best=0,spinv_best=0;
  uint32_t N=bthd_transmission_mode[TransmissionMode];
  uint32_t iteration;
  int32_t  cl_int, cl_fcw;

  /* Collect ICE values with carrier loop frozen */
  BREG_WriteField(h->hRegister, THD_CORE_FRZ, CL_FRZ, 1);
  cl_int = BREG_Read32(h->hRegister,  BCHP_THD_CORE_CL_INT); 
  cl_fcw = BREG_Read32(h->hRegister,  BCHP_THD_CORE_CL_FCW);
          
  for (iteration=0; iteration<2; iteration++) {
    BREG_WriteField(h->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 0 );
    BTHD_P_OSleep(h,3,N,GuardInterval);
    for (n=0; n<16; n++) {
      ice[n] = BREG_ReadField(h->hRegister,  THD_CORE_CL_IEST, INT);
      ice_val[n] = 0;
      ice_cnt[n] = 0;
      BTHD_P_OSleep(h,1,N,GuardInterval);
      /* BDBG_MSG(("BTHD_P_DvbtSetICE:\tICE = 0x%08x",ice[n]); */
    }

    /* Identify unique ICE values and count number-of-occurrences of each */
    ice_val[0] = ice[0];
    ice_cnt[0] = 1;
    for (n=1; n<16; n++) {
      ice_found = 0;
      for (k=0; k<n; k++) {
	      if (ice[n] == ice_val[k]) {  /* found, increment the count */
	        ice_cnt[k] = ice_cnt[k]+1;
	        ice_found = 1;        
	      }
      }
      if (!ice_found) {              /* not found, add it to the table */
	      ice_val[k] = ice[n];
	      ice_cnt[k] = 1;
      }
    }

    /* Determine value which occurs most often */  
    ice_val_max = ice_val[0];
    ice_cnt_max = ice_cnt[0];
    ice_index_max = 0;
    for (n=1; n<16; n++) {
      if (ice_cnt[n] > ice_cnt_max) {
	      ice_val_max = ice_val[n];
	      ice_cnt_max = ice_cnt[n];
	      ice_index_max = n;
      }
    }
    /* BDBG_MSG(("BTHD_P_DvbtSetICE:\tIC Offset = %d, ICE Max = 0x%08x (%d occurences)", carrier_off, ice_val[ice_index_max],ice_cnt_max); */

    /* Keep track of the best result for comparison */
    if (ice_cnt_max > ice_cnt_best) {
      ice_cnt_best   =  ice_cnt_max;
      ice_val_best   =  ice_val_max;
      ice_index_best =  ice_index_max;
      spinv_best     =  iteration;
    }

    /* Check for valid ICE and exit */
    if ( ice_cnt_max >= THD_ICESpectrumInversionThreshold )
      break;
 
    /* Try spectral inversion on second iteration */
    /* BDBG_MSG(("BTHD_P_DvbtSetICE:\tTrying spectral inversion")); */
    if (h->pInternalAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
      BREG_WriteField(h->hRegister, THD_CORE_FE, NEGATE_Q, 1);
	  BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_INT, (cl_int ^ 0xffffffff) + 1U);
    } else {
      /*  Negate carrier fcw and integrator */ 
      BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_FCW, (cl_fcw ^ 0xffffffff) + 1U);
      BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_INT, (cl_int ^ 0xffffffff) + 1U);   
    }

    /* Freeze and wait 10 symbols */
    BREG_WriteField(h->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 1 );             /*  Freeze integer carrier estimator*/
    BTHD_P_OSleep(h,10,N,GuardInterval);
  }

  /* Reset CL_INT back to initial value (allowing for spectral inversion) */
  if (iteration)
    BREG_Write32(h->hRegister, BCHP_THD_CORE_CL_INT, (cl_int ^ 0xffffffff) + 1U);
  else
    BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_INT, cl_int);

  /* Uninvert signal if needed.  
     This can happen if spectral inversion was tested but noninverted spectrum yielded a better ICE */
  if (iteration && !spinv_best) {
    if (h->pInternalAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
      BREG_WriteField(h->hRegister, THD_CORE_FE, NEGATE_Q, 0);
	  BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_INT, cl_int);	  
    } else {
      BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_FCW, cl_fcw);
      BREG_Write32(h->hRegister,  BCHP_THD_CORE_CL_INT, cl_int);
    }
  }
  if (spinv_best)
    BDBG_MSG(("\tSpectral Inversion Detected"));
  
  /* Detection flag for suspect ICE result */
  if (ice_cnt_best < 6) {
    /* BDBG_WRN(("BTHD_P_DvbtSetICE:\tLow ICE Count Detected")); */
    h->pStatus->ThdCommonStatus.LowICECount = 1;
  }
  else {
    h->pStatus->ThdCommonStatus.LowICECount = 0;
  }

  BREG_WriteField(h->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 1 );
  BTHD_P_OSleep(h,1,N,GuardInterval);

  /* BDBG_MSG(("BTHD_P_DvbtSetICE:\tICE Max = 0x%08x (%d occurences)",ice_val_best,ice_cnt_best)); */
  return(ice_val_best);
}

/***************************************************************************
 * BTHD_P_DvbtSetFrame()
 ***************************************************************************/
void BTHD_P_DvbtSetFrame( BTHD_3x7x_ChnHandle h )
{
  uint32_t tps;
  uint32_t fsize;
  unsigned D;
  unsigned Q;
  unsigned R;
  uint32_t num;
  uint32_t den;
  uint32_t bw;

  /* Extract the guard interval, QAM mode, and code rate from the TPS value */
  tps = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV );
  D = (1U << (5-((tps >> 20) & 0x3)));
  Q = (tps >> 4)  & 0x3;
  R = bthd_rate_denominator_table[ (tps >> 12) & 0x7 ];

  /* Handle errors in TPS.*/
  if (Q == 3) /* Handle invalid QAM.*/
    Q = 0;  /* Arbitrary QAM.*/

  /* Calculate the frame size */
  bw = bthd_bandwidth[h->pAcquireParam->CommonAcquireParam.Bandwidth];
  num = 281250 * bw * D * (Q + 1) * (R - 1);  
  den = 272 * (D + 1) * R;
  fsize = (num + (den >> 1)) / den;

  BREG_Write32(h->hRegister,  BCHP_THD_CORE_FSIZE, (uint32_t)fsize );
  /* BDBG_MSG(("BTHD_P_DvbtSetFrame:\tFSIZE  = 0x%08x", fsize);  */
}

/***************************************************************************
 * BTHD_P_DvbtCheckChannelEstimator()
 ***************************************************************************/
THD_ChannelEstimatorMode_t BTHD_P_DvbtCheckChannelEstimator(BTHD_3x7x_ChnHandle h, THD_ChannelEstimatorMode_t ChannelEstimatorMode)
{
  uint32_t N;
   uint32_t RSClnBlkInit, RSCorBlkInit, RSUncBlkInit, RSTotBlkInit;
   uint32_t RSClnBlkFixed, RSCorBlkFixed, RSUncBlkFixed, RSTotBlkFixed;
   uint32_t RSClnBlkPedestrian, RSCorBlkPedestrian, RSUncBlkPedestrian, RSTotBlkPedestrian;
  THD_TransmissionMode_t TransmissionMode;
  THD_GuardInterval_t GuardInterval;
	THD_CodeRate_t CodeRate;
	THD_ChannelEstimatorMode_t return_val = ChannelEstimatorMode;
	bool DTGOption7;

  TransmissionMode = BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, TRANS_MODE );
  N = bthd_transmission_mode[TransmissionMode];
  GuardInterval = BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, GUARD);
	CodeRate = BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, CRATE_HP);
	DTGOption7 = (TransmissionMode == THD_TransmissionMode_8k) && (GuardInterval == THD_GuardInterval_1_32) && (CodeRate == THD_CodeRate_3_4);

  /* Set final CE averager mode based on FEC performance between Pedestrian and Fixed for QPSK */
  if ((h->pInternalAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Auto) &&
	    (ChannelEstimatorMode == THD_ChannelEstimatorMode_Fixed) &&
			(BREG_ReadField(h->hRegister, THD_CORE_TPS_OV, QAM ) < THD_Qam_16Qam)){
		/* Record Initial FEC Statistics */
		RSClnBlkInit = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_NBERC );
		RSCorBlkInit = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_CBERC );
		RSUncBlkInit = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_UBERC );
		RSTotBlkInit = RSClnBlkInit + RSCorBlkInit + RSUncBlkInit;

		/* Record Fixed FEC Statistics */
    BTHD_P_OSleep(h,16,N,GuardInterval);
		RSClnBlkFixed = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_NBERC ) - RSClnBlkInit;
		RSCorBlkFixed = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_CBERC ) - RSCorBlkInit;
		RSUncBlkFixed = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_UBERC ) - RSUncBlkInit;
		RSTotBlkFixed = RSClnBlkFixed + RSCorBlkFixed + RSUncBlkFixed;

    /* Record Pedestrian FEC Statistics */
    BTHD_P_SetCE(h, THD_ChannelEstimatorMode_Pedestrian);
		BTHD_P_OSleep(h,16,N,GuardInterval);
		RSClnBlkPedestrian = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_NBERC ) - (RSClnBlkFixed+RSClnBlkInit);
		RSCorBlkPedestrian = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_CBERC ) - (RSCorBlkFixed+RSCorBlkInit);
		RSUncBlkPedestrian = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_UBERC ) - (RSUncBlkFixed+RSUncBlkInit);
		RSTotBlkPedestrian = RSClnBlkPedestrian + RSCorBlkPedestrian + RSUncBlkPedestrian;

    /* Determine final mode */
    /* BDBG_MSG(("\tFix={%d,%d,%d,%d}, Ped={%d,%d,%d,%d}",RSClnBlkFixed,RSCorBlkFixed,RSUncBlkFixed,RSTotBlkFixed,RSClnBlkPedestrian,RSCorBlkPedestrian,RSUncBlkPedestrian,RSTotBlkPedestrian)); */
		/* Leave in Pedestrian if ((ClnPed > 1.5*ClnFix) and (1.5*CorPed < CorFix)) and (UncPed <= UncFix) */
		if (((RSUncBlkPedestrian+10) < RSUncBlkFixed) ||
		   ((RSUncBlkPedestrian <= RSUncBlkFixed) && ((2*RSClnBlkPedestrian > 3*RSClnBlkFixed) && (3*RSCorBlkPedestrian < 2*RSCorBlkFixed)))) {
      BTHD_P_SetCE(h, THD_ChannelEstimatorMode_Pedestrian);
			return_val = THD_ChannelEstimatorMode_Pedestrian;
			BDBG_MSG(("\tTHD CheckLock Mode = Pedestrian"));
		} else {
      BTHD_P_SetCE(h, THD_ChannelEstimatorMode_Fixed);
			return_val = THD_ChannelEstimatorMode_Fixed;
			BDBG_MSG(("\tTHD CheckLock Mode = Fixed"));
		}
	}

	if (DTGOption7) {
		if (return_val == THD_ChannelEstimatorMode_Pedestrian) {
			BREG_WriteField(h->hRegister, THD_CORE_CE_LMS, FREQ_ENABLE, 1 );      /* Adaptive frequency interpolator enable */
			BREG_WriteField(h->hRegister, THD_CORE_CE_LMS, FREQ_LEAK_ENABLE, 1 ); /* Adaptive frequency interpolator leakage enable */
		} else {
			BREG_WriteField(h->hRegister, THD_CORE_CE, AVG_FF, 1 ); /* Decrease averaging to degrade Option7 fixed-mode performance so that delta C/N between 0Hz and 20Hz Doppler is less than 1dB */
		/*
      BTHD_P_SetCE(h, THD_ChannelEstimatorMode_Pedestrian);
			return_val = THD_ChannelEstimatorMode_Pedestrian;
			BDBG_MSG(("\tTHD CheckLock Mode = Pedestrian"));
		*/
		}
	}

	return(return_val);
}

/***************************************************************************
 * BTHD_P_DvbtStatus()
 ***************************************************************************/
void BTHD_P_DvbtStatus( BTHD_3x7x_ChnHandle h ) 
{
  uint32_t value,value2,CERC,CBERC,NBERC,UBERC,TBERC,NFERC,UFERC,TFERC,eq_snr;
  uint32_t scale, sft_scale;
  int32_t tmp;
  uint32_t pouthi, poutlo, poutlo2, utmp;

  const uint8_t scale_table[] = { 29, 27, 25, 29 }; /* Support invalid QAM.*/
  const uint8_t sft_scale_table[][4] = \
  {
    {  2,  2,  2,  2 },
    { 10, 10, 20, 52 },
    { 42, 42, 60, 108},
    {  2,  2,  2,  2 } 
  };

  /* Determine transmission parameters */
  value = BREG_Read32(h->hRegister,  BCHP_THD_CORE_TPS_OV);
	h->pStatus->StdStatus.DvbtStatus.MpeFec       = (value >> 27) & 0x1;
  h->pStatus->StdStatus.DvbtStatus.TimeSlicing  = (value >> 26) & 0x1;
  h->pStatus->StdStatus.DvbtStatus.CodeRateLP   = (value >> 16) & 0x7;
  h->pStatus->StdStatus.DvbtStatus.CodeRateHP   = (value >> 12) & 0x7;
  h->pStatus->StdStatus.DvbtStatus.Interleaving = (value >> 10) & 0x1;
  h->pStatus->StdStatus.DvbtStatus.Hierarchy    = (value >> 8)  & 0x3;
  h->pStatus->StdStatus.DvbtStatus.Qam          = (value >> 4)  & 0x3;
  h->pStatus->StdStatus.DvbtStatus.CellID       = BREG_ReadField(h->hRegister, THD_CORE_TPS_CELL_ID, CELL_ID );

  /* Determine stream priority */
  if (BREG_ReadField(h->hRegister, THD_CORE_FEC, HP ))
    h->pStatus->StdStatus.DvbtStatus.Priority = THD_DvbtPriorityMode_High;
  else
    h->pStatus->StdStatus.DvbtStatus.Priority = THD_DvbtPriorityMode_Low; 
 
  /* Compute data SNR */
  eq_snr = BREG_Read32(h->hRegister, BCHP_THD_CORE_EQ_SNR);
  if (eq_snr >> (8+2)) {
    scale     = scale_table[h->pStatus->StdStatus.DvbtStatus.Qam];
    sft_scale = sft_scale_table[h->pStatus->StdStatus.DvbtStatus.Qam][h->pStatus->StdStatus.DvbtStatus.Hierarchy];

    /* Take 10*log10(value2/256.0) to obtain value in dB */
    tmp = (sft_scale << (scale-2))/(eq_snr >> (8+2));
    /* 256*10*log10(1/256) = -6165, application then takes SNR_p/256 for SNR in dB */  
    h->pStatus->StdStatus.DvbtStatus.SNRData = BMTH_2560log10(tmp) - 6165;
  }

  /* Compute pre-Viterbi BER */
  value  = BREG_Read32(h->hRegister,  BCHP_THD_CORE_VIT_RCNT );
  value2 = BREG_Read32(h->hRegister,  BCHP_THD_CORE_VIT_RERR );
  if (value2)
  {
    /* h->pStatus->ThdCommonStatus.PreVitBER = (((value * 2147483648) / value2) >> 16);  */          
    BMTH_HILO_64TO64_Mul(0, value, 0, 0x80000000, &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, value2, &utmp, &poutlo2);
    h->pStatus->StdStatus.DvbtStatus.PreVitBER = utmp; /* Application takes x/(2^31) to obtain error rate */
  }

  /* Read hardware error counters */
  BREG_WriteField(h->hRegister, THD_CORE_FEC, CAPERC, 1 ); 

  CERC  = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_CERC );
  NBERC = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_NBERC );
  CBERC = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_CBERC );
  UBERC = BREG_Read32(h->hRegister,  BCHP_THD_CORE_RS_UBERC );
  NFERC = BREG_Read32(h->hRegister,  BCHP_THD_CORE_NFERC );
  UFERC = BREG_Read32(h->hRegister,  BCHP_THD_CORE_UFERC );
  TBERC = NBERC + CBERC + UBERC;
  TFERC = NFERC + UFERC; 

  BREG_WriteField(h->hRegister, THD_CORE_FEC, CAPERC, 0 ); 

  /* Update software error counters */
  h->pStatus->StdStatus.DvbtStatus.TS_CERC  = h->pStatus->StdStatus.DvbtStatus.TS_CERC_ref  + CERC;
  h->pStatus->StdStatus.DvbtStatus.TS_NBERC = h->pStatus->StdStatus.DvbtStatus.TS_NBERC_ref + NBERC;
  h->pStatus->StdStatus.DvbtStatus.TS_CBERC = h->pStatus->StdStatus.DvbtStatus.TS_CBERC_ref + CBERC;
  h->pStatus->StdStatus.DvbtStatus.TS_UBERC = h->pStatus->StdStatus.DvbtStatus.TS_UBERC_ref + UBERC;
  h->pStatus->StdStatus.DvbtStatus.TS_TBERC = h->pStatus->StdStatus.DvbtStatus.TS_TBERC_ref + TBERC;
  h->pStatus->StdStatus.DvbtStatus.TS_NFERC = h->pStatus->StdStatus.DvbtStatus.TS_NFERC_ref + NFERC;
  h->pStatus->StdStatus.DvbtStatus.TS_UFERC = h->pStatus->StdStatus.DvbtStatus.TS_UFERC_ref + UFERC;
  h->pStatus->StdStatus.DvbtStatus.TS_TFERC = h->pStatus->StdStatus.DvbtStatus.TS_TFERC_ref + TFERC;

  /* Reset hardware error counters every 10 one-second frames */
  if (TFERC >= THD_StatusFramesForReset) {
    BTHD_P_ResetStatusHW(h);
    
    h->pStatus->StdStatus.DvbtStatus.TS_CERC_ref  = h->pStatus->StdStatus.DvbtStatus.TS_CERC;
    h->pStatus->StdStatus.DvbtStatus.TS_NBERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_NBERC;
    h->pStatus->StdStatus.DvbtStatus.TS_CBERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_CBERC;
    h->pStatus->StdStatus.DvbtStatus.TS_UBERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_UBERC;
    h->pStatus->StdStatus.DvbtStatus.TS_TBERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_TBERC;
    h->pStatus->StdStatus.DvbtStatus.TS_NFERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_NFERC;
    h->pStatus->StdStatus.DvbtStatus.TS_UFERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_UFERC;
    h->pStatus->StdStatus.DvbtStatus.TS_TFERC_ref = h->pStatus->StdStatus.DvbtStatus.TS_TFERC;
  }  

  /* Compute Viterbi BER and TS PER */
  if (h->pStatus->StdStatus.DvbtStatus.TS_TBERC) {
    /* h->pStatus->ThdCommonStatus.TS_PER = (h->pStatus->ThdCommonStatus.TS_UBERC * (1ULL<<31)) / h->pStatus->ThdCommonStatus.TS_TBERC; */
    BMTH_HILO_32TO64_Mul(0x80000000, (h->pStatus->StdStatus.DvbtStatus.TS_UBERC), &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, (h->pStatus->StdStatus.DvbtStatus.TS_TBERC), &pouthi, &h->pStatus->StdStatus.DvbtStatus.TS_PER );
    /* h->pStatus->ThdCommonStatus.VitBER = (h->pStatus->ThdCommonStatus.TS_CERC * (1ULL<<31)) / (h->pStatus->ThdCommonStatus.TS_TBERC * 8 * 188ULL); */
    BMTH_HILO_32TO64_Mul(0x80000000, (h->pStatus->StdStatus.DvbtStatus.TS_CERC), &pouthi, &poutlo);
    /*BMTH_HILO_64TO64_Div32(pouthi, poutlo, (h->pStatus->StdStatus.DvbtStatus.TS_TBERC * 8 * 188), &pouthi, &h->pStatus->StdStatus.DvbtStatus.VitBER ); */
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, (8 * 188), &pouthi, &poutlo );
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, (h->pStatus->StdStatus.DvbtStatus.TS_TBERC), &pouthi, &h->pStatus->StdStatus.DvbtStatus.VitBER );
  }
  if (h->pStatus->StdStatus.DvbtStatus.TS_TFERC){
    /* h->pStatus->ThdCommonStatus.TS_ESR = ((100ULL * h->pStatus->ThdCommonStatus.TS_UFERC) / h->pStatus->ThdCommonStatus.TS_TFERC); */
    BMTH_HILO_32TO64_Mul(0x10000000, (h->pStatus->StdStatus.DvbtStatus.TS_UFERC), &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, (h->pStatus->StdStatus.DvbtStatus.TS_TFERC), &pouthi, &h->pStatus->StdStatus.DvbtStatus.TS_ESR );
  }
}

/***************************************************************************
 * BTHD_P_DvbtResetStatus()
 ***************************************************************************/
void BTHD_P_DvbtResetStatus( BTHD_3x7x_ChnHandle h )
{
  h->pStatus->StdStatus.DvbtStatus.TS_CERC      = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_CBERC     = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_UBERC     = 0;  
  h->pStatus->StdStatus.DvbtStatus.TS_NBERC     = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_TBERC     = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_UFERC     = 0;  
  h->pStatus->StdStatus.DvbtStatus.TS_NFERC     = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_TFERC     = 0;

  h->pStatus->StdStatus.DvbtStatus.TS_CERC_ref  = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_CBERC_ref = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_UBERC_ref = 0;  
  h->pStatus->StdStatus.DvbtStatus.TS_NBERC_ref = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_TBERC_ref = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_UFERC_ref = 0;  
  h->pStatus->StdStatus.DvbtStatus.TS_NFERC_ref = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_TFERC_ref = 0;

  h->pStatus->StdStatus.DvbtStatus.PreVitBER    = 0;
  h->pStatus->StdStatus.DvbtStatus.VitBER       = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_PER       = 0;
  h->pStatus->StdStatus.DvbtStatus.TS_ESR       = 0;
}

/***************************************************************************
 * BTHD_P_DvbtAcquire()
 ***************************************************************************/
BTHD_RESULT BTHD_P_DvbtAcquire( BTHD_3x7x_ChnHandle h )
{
  BTHD_RESULT return_val=THD_AcquireResult_InitLockState;

  THD_FFTWindowMode_t FFTWindowMode=THD_FFTWindowMode_InSpan;
  THD_ChannelEstimatorMode_t ChannelEstimatorMode=THD_ChannelEstimatorMode_Fixed, ChannelEstimatorModeList[2];
  THD_State_t NextState=THD_State_Init, LastState=THD_State_Init, State=THD_State_Init;
  bool Done=false, acqProfileValid[16];
  uint32_t retries=0, ChannelEstimatorRetries, fscntInit=0xffffffff, acqProfile[16], k;
  uint32_t TPSRetries=0, AllowedTPSRetries=1;
  char     *acqProfileString[16];

  /* Initialize Candidate Channel Estimator Modes */
  ChannelEstimatorModeList[0] = THD_ChannelEstimatorMode_Fixed;
  ChannelEstimatorModeList[1] = THD_ChannelEstimatorMode_Pedestrian;
  if (h->pInternalAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Auto) {
    ChannelEstimatorMode = ChannelEstimatorModeList[0];
    ChannelEstimatorRetries = 1;
  } else {
    ChannelEstimatorModeList[0] = h->pInternalAcquireParam->ChannelEstimatorMode;
    ChannelEstimatorRetries = 0;
  }

  /* Use Fscnt for acquisition time profiling */
  BREG_Write32(h->hRegister, BCHP_THD_CORE_FSCNT,fscntInit);

  acqProfileString[THD_State_Init] = "Init";
  acqProfileString[THD_State_SP] = "SP";
  acqProfileString[THD_State_FFTTrigger] = "FFTTrigger";  
  acqProfileString[THD_State_TPS] = "TPS";
  acqProfileString[THD_State_FEC] = "FEC";
  acqProfileString[THD_State_Track] = "Track";
  acqProfileString[THD_State_CheckLock] = "CheckLock";
  acqProfileString[THD_State_ChangeChannelEstimator] = "ChangeChannelEstimator";
  acqProfileString[THD_State_Done] = "Done";
  for (k=0; k<16; k++)
    acqProfileValid[k] = false;

  /* State Machine */
  while (!Done) {
    switch(State) {

    case THD_State_Init:
      BDBG_MSG(("\tTHD Init"));
      h->pInternalAcquireParam->AllowRsSyncEvent = true;
      return_val = BTHD_P_AcquireInit(h,FFTWindowMode);
      if ((return_val == THD_AcquireResult_NoSignal) || (return_val == THD_AcquireResult_NoFFTLock) || (return_val == THD_AcquireResult_AbortedEarly))
	      NextState = THD_State_Done;
      else
	      NextState = THD_State_SP;
      break;

    case THD_State_SP:
      BDBG_MSG(("\tTHD SP"));
      return_val = BTHD_P_AcquireSP(h);
      if ((return_val == THD_AcquireResult_NoSPLock) || (return_val == THD_AcquireResult_AbortedEarly))
	      NextState = THD_State_Done;
      else
          NextState = THD_State_TPS;
      break;

    case THD_State_TPS:
      BDBG_MSG(("\tTHD TPS"));
      return_val = BTHD_P_AcquireTPS(h);
      if (return_val == THD_AcquireResult_AbortedEarly) {
          NextState = THD_State_Done;
      } else if (TPSRetries < AllowedTPSRetries) {
	      NextState = THD_State_FFTTrigger;
          TPSRetries++;
      }
      else {
        if (return_val == THD_AcquireResult_NoTPSLock)
	        NextState = THD_State_Done;
        else
	        NextState = THD_State_FEC;
      }
      break;

    case THD_State_FFTTrigger:
      BDBG_MSG(("\tTHD FFTTrigger"));
      return_val = BTHD_P_AcquireFFTTrigger(h);
      if (return_val == THD_AcquireResult_NoSPLock)
        BDBG_MSG(("\tTHD FFTTrigger obtained NO SPLock"));
      if (return_val == THD_AcquireResult_AbortedEarly) {
          NextState = THD_State_Done;
      }
      else {
        if (h->pStatus->ThdCommonStatus.FFTTriggerMissed || h->pStatus->ThdCommonStatus.FFTTriggerOnGuard)
          NextState  = THD_State_TPS; 
        else
	      NextState  = THD_State_FEC; 
      }
      break;

    case THD_State_FEC:
      BDBG_MSG(("\tTHD FEC"));
      return_val = BTHD_P_AcquireFEC(h,(retries==0));
      if ((return_val == THD_AcquireResult_NoFECLock) || (return_val == THD_AcquireResult_AbortedEarly)) {
	      NextState = THD_State_Done;
      } else {
	      NextState = THD_State_Track;
	      retries = 0;
      }
      break;

    case THD_State_Track:
      BDBG_MSG(("\tTHD Track"));
      ChannelEstimatorMode = ChannelEstimatorModeList[retries];
      return_val = BTHD_P_AcquireTrack(h,FFTWindowMode,ChannelEstimatorMode);
      if (return_val == THD_AcquireResult_AbortedEarly) {
          NextState = THD_State_Done;
      }
      else {
	      NextState = THD_State_CheckLock;
      } 
      
      /*Turn off impulsive noise canceller bypass when ImpulseMode is auto*/
      if (h->pInternalAcquireParam->ImpulseMode == THD_ImpulseMode_Auto)
        BREG_WriteField(h->hRegister, THD_CORE_BYP, IMPE_BYP, 0);
      else
        BREG_WriteField(h->hRegister, THD_CORE_BYP, IMPE_BYP, 1);
        
      break;

    case THD_State_CheckLock:
      BDBG_MSG(("\tTHD CheckLock"));
      return_val = BTHD_P_AcquireCheckLock(h);
      if (return_val == THD_AcquireResult_NoFECLock) {
	      if (retries < ChannelEstimatorRetries) {
	        retries++;
	        NextState = THD_State_ChangeChannelEstimator;
	      } else
	        NextState = THD_State_Done;
      } else {
				ChannelEstimatorMode = BTHD_P_DvbtCheckChannelEstimator(h,ChannelEstimatorMode);
	      NextState = THD_State_Done;
			}
      break;

    case THD_State_ChangeChannelEstimator:
      BDBG_MSG(("\tTHD ChangeChannelestimator"));
      ChannelEstimatorMode = ChannelEstimatorModeList[retries];
      return_val = BTHD_P_AcquireChangeChannelEstimator(h,ChannelEstimatorMode);
      if (return_val == THD_AcquireResult_AbortedEarly) {
        NextState = THD_State_Done;
      }
      else {
        NextState = THD_State_CheckLock;
      }
      break;

    case THD_State_Done:
      BDBG_MSG(("\tTHD Done"));
      h->pInternalAcquireParam->AllowRsSyncEvent = false;
      BKNI_ResetEvent(h->hRsSyncEvent);      
      Done = true;
#ifdef EMULATION_ENABLE
	  BREG_Write32(h->hRegister,  BCHP_THD_INTR_AP_SET, 0x00000080 );
#endif
      break;

	default:
	  BDBG_MSG(("\tNOT SUPPORTED MODE"));
	  break;
    }

    acqProfile[State] = BREG_Read32(h->hRegister, BCHP_THD_CORE_FSCNT);
    acqProfileValid[State] = true;

    if (!Done)
      LastState=State;
    State=NextState;
  }

  /* Update status structure with auto-detected modes */
  h->pStatus->ThdCommonStatus.ChannelEstimatorMode = ChannelEstimatorMode;
  h->pStatus->ThdCommonStatus.FFTWindowMode = FFTWindowMode;

  /* Report acquisition time */
  h->pStatus->ThdCommonStatus.AcquisitionTime = (fscntInit - acqProfile[LastState])/(h->pInternalAcquireParam->SampleFreq/1000);
  /*BDBG_MSG(("\tAcquisition Time = %d msec",h->pStatus->ThdCommonStatus.AcquisitionTime));  */
  
  for (k=0; k<16; k++)
    if (acqProfileValid[k])
      BDBG_MSG(("\t\t%d msec: %s", (fscntInit - acqProfile[k])/(h->pInternalAcquireParam->SampleFreq/1000), acqProfileString[k]));
  
  /* Reset the Abort-Early request */
  BTHD_P_AbortAcquireReset(h);

  return(return_val);
}

