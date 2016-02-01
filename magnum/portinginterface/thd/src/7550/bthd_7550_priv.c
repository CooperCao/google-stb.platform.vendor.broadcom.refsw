/***************************************************************************
*     Copyright (c) 2005-2012, Broadcom Corporation
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
#include "bstd.h"
#include "bmth.h"
#include "bchp_7550.h"
#include "bkni.h"
#include "bthd.h"
#include "bthd_priv.h"
#include "bchp_thd_core.h"
#include "bthd_7550_priv.h"
#include "bthd_7550_thdcoef.h"
#include "bchp_int_id_thd_intr2.h"
#include "bthd_7550_platform.h"
#define  FASTACQ

BDBG_MODULE(bthd_7550_priv);


/******************************************************************************
* BTHD_7550_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BTHD_7550_P_GetSoftDecisionBuf( 
  BTHD_Handle h,  /* [in] BTHD handle */
  int16_t *pI,    /* [out] 30 I-values */
  int16_t *pQ     /* [out] 30 Q-values */
  )
{
  uint8_t idx;
  uint16_t iVal, qVal;
  uint32_t val, tps;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);


  tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS);
  tps &= BCHP_THD_CORE_TPS_QAM_MASK;
  tps >>= BCHP_THD_CORE_TPS_QAM_SHIFT;

  iVal = qVal = 0;

  for (idx = 0;idx < 30; idx++ )
  {
    val = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_SOFT_READ_DATA);

    switch (tps)
    {
    case THD_Qam_Qpsk:
      iVal = (uint32_t)(val & 0x3FFC0000) >> 18;
      qVal = (uint32_t)(val & 0x0000fFF0) >> 4;

      iVal += 0x800;
      qVal += 0x800;
      break;
    case THD_Qam_16Qam:
      iVal = (uint32_t)(val & 0xFFFC0000) >> 18;
      qVal = (uint32_t)(val & 0x0001fFF0) >> 4;

      iVal += 0x1000;
      qVal += 0x1000;
      iVal /= 2;
      qVal /= 2;
      break;
    default:
      BDBG_WRN(("Unrecognized Modulation settings"));
	       /* fall through */
    case THD_Qam_64Qam:
      iVal = (uint32_t)(val & 0xFFFC0000) >> 18;
      qVal = (uint32_t)(val & 0x0003fFF0) >> 4;

      iVal += 0x2000;
      qVal += 0x2000;
      iVal /= 4;
      qVal /= 4;
      break;

    }
    pI[idx] = (int16_t)(iVal & 0xfFF);
    pQ[idx] = (int16_t)(qVal & 0xfFF);
  }

  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_PI_OSleep() 
***************************************************************************/

BERR_Code BTHD_7550_PI_OSleep(BTHD_Handle h, uint32_t symbols, uint32_t N, uint32_t D)
{
  uint32_t samples = symbols*(N + (N >> (5-D)));  
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  BINT_EnableCallback(((BTHD_7550_P_Handle *)(h->pImpl))->hFbcCntCallback);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FBCNT,samples);
  if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFbcntZeroEvent, 400) != BERR_SUCCESS) 
    BDBG_MSG(("BTHD_7550_PI_OSleep:\tNo FBCNT zero"));
  BINT_DisableCallback(((BTHD_7550_P_Handle *)(h->pImpl))->hFbcCntCallback);
  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_P_AcquireThd()  
***************************************************************************/
BERR_Code BTHD_7550_P_AcquireThd(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
#if BCHP_VER == BCHP_VER_A0
  if ((p7550->pAcquireParam->Standard  == THD_Standard_ISDBT) && (p7550->pAcquireParam->SoftwareWorkaroundMode == 0) ) {
    p7550->pAcquireParam->SoftwareWorkaroundMode = 1;
    BTHD_7550_P_ISDBT_Softwareworkaround (h);
  }
#endif
  p7550->ThdLockStatus = 0;
  p7550->pAcquireParam->EventMode |= (THD_Event_LockUpdate | THD_Event_UnlockStopEWS | THD_Event_LockStartEWS);
  BTHD_7550_P_ThdTop(h);
  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_GetDefaultSettings() 
***************************************************************************/
BERR_Code BTHD_7550_SetDefaultSettings(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  BERR_Code retCode = BERR_SUCCESS;
  BDBG_ASSERT( h );
  /*
  p7550->pAcquireParam->Standard               = THD_Standard_DVBT;
  */
  p7550->pAcquireParam->Standard               = THD_Standard_ISDBT; 

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    p7550->pAcquireParam->FrontEndMode         = THD_FrontEndMode_Baseband; /* IF; */
    p7550->pAcquireParam->ChannelEstimatorMode = THD_ChannelEstimatorMode_Auto;
    p7550->pAcquireParam->FFTWindowMode        = THD_FFTWindowMode_Auto;
    p7550->pAcquireParam->CoChannelMode        = THD_CoChannelMode_Auto;
    p7550->pAcquireParam->ImpulseMode          = THD_ImpulseMode_None; 
    p7550->pAcquireParam->TransGuardMode       = THD_TransGuardMode_Auto_ISDBT;
    p7550->pAcquireParam->TPSMode              = THD_TPSMode_Auto;
    p7550->pAcquireParam->CarrierRange         = THD_CarrierRange_Narrow; 
    p7550->pAcquireParam->TSMode               = THD_TSMode_Serial;
    p7550->pAcquireParam->PriorityMode         = THD_PriorityMode_High;
    p7550->pAcquireParam->TransmissionMode     = THD_TransmissionMode_8k;
    p7550->pAcquireParam->GuardInterval        = THD_GuardInterval_1_8;
    p7550->pAcquireParam->Qam                  = THD_Qam_16Qam;
    p7550->pAcquireParam->Hierarchy            = THD_Hierarchy_None;
    p7550->pAcquireParam->CodeRateHP           = THD_CodeRate_3_4;
    p7550->pAcquireParam->CodeRateLP           = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_TMCCMode       = THD_TMCCMode_Auto;
    p7550->pAcquireParam->ISDBT_PR             = 0;
    p7550->pAcquireParam->ISDBT_A_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_B_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_C_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_A_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_B_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_C_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_A_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_B_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_C_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_A_Segments     = 13;
    p7550->pAcquireParam->ISDBT_B_Segments     = 0;
    p7550->pAcquireParam->ISDBT_C_Segments     = 0;
    p7550->pAcquireParam->Bandwidth            = THD_Bandwidth_6MHz;
    p7550->pAcquireParam->CenterFreq           = 0;
    p7550->pAcquireParam->SampleFreq           = 54000000;
    p7550->pAcquireParam->RfFreq               = 666000000;
    p7550->pAcquireParam->TransGuardMaxThreshold            = 0x00050000;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[0] = 296;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[1] = 210;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[2] = 150;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[3] = 104;
  } else {
    p7550->pAcquireParam->FrontEndMode         = THD_FrontEndMode_Baseband; /* IF */
    p7550->pAcquireParam->ChannelEstimatorMode = THD_ChannelEstimatorMode_Auto;
    p7550->pAcquireParam->FFTWindowMode        = THD_FFTWindowMode_Auto;
    p7550->pAcquireParam->CoChannelMode        = THD_CoChannelMode_Auto;
    p7550->pAcquireParam->ImpulseMode          = THD_ImpulseMode_None; 
    p7550->pAcquireParam->TransGuardMode       = THD_TransGuardMode_Auto_DVBT;
    p7550->pAcquireParam->TPSMode              = THD_TPSMode_Auto;
    p7550->pAcquireParam->CarrierRange         = THD_CarrierRange_Narrow; 
    p7550->pAcquireParam->TSMode               = THD_TSMode_Serial;
    p7550->pAcquireParam->PriorityMode         = THD_PriorityMode_High;
    p7550->pAcquireParam->TransmissionMode     = THD_TransmissionMode_2k;
    p7550->pAcquireParam->GuardInterval        = THD_GuardInterval_1_32;
    p7550->pAcquireParam->Qam                  = THD_Qam_16Qam;
    p7550->pAcquireParam->Hierarchy            = THD_Hierarchy_None;
    p7550->pAcquireParam->CodeRateHP           = THD_CodeRate_3_4;
    p7550->pAcquireParam->CodeRateLP           = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_TMCCMode       = THD_TMCCMode_Auto;
    p7550->pAcquireParam->ISDBT_PR             = 0;
    p7550->pAcquireParam->ISDBT_A_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_B_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_C_Qam          = THD_Qam_16Qam;
    p7550->pAcquireParam->ISDBT_A_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_B_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_C_CodeRate     = THD_CodeRate_1_2;
    p7550->pAcquireParam->ISDBT_A_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_B_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_C_TimeInt      = THD_TimeInt_0X;
    p7550->pAcquireParam->ISDBT_A_Segments     = 13;
    p7550->pAcquireParam->ISDBT_B_Segments     = 0;
    p7550->pAcquireParam->ISDBT_C_Segments     = 0;
    p7550->pAcquireParam->Bandwidth            = THD_Bandwidth_8MHz;
    p7550->pAcquireParam->CenterFreq           = 0;
    p7550->pAcquireParam->SampleFreq           = 54000000;
    p7550->pAcquireParam->RfFreq               = 666000000;
    p7550->pAcquireParam->TransGuardMaxThreshold            = 0x00050000;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[0] = 296;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[1] = 210;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[2] = 150;
    p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[3] = 104;
  }
  return( retCode );
}

/***************************************************************************
* BTHD_7550_P_ThdTop()  
***************************************************************************/
BERR_Code BTHD_7550_P_ThdTop(BTHD_Handle h)
{
  uint32_t  ReacquireCount;
  BERR_Code retCode;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  BDBG_MSG(("BTHD_7550_P_ThdTop:\tAcquireStartMode = %d", p7550->pAcquireParam->AcquireStartMode));

  /* BTHD_7550_SetDefaultSettings(h); */

  BTHD_7550_P_ResetStatus(h);
  BTHD_7550_P_ResetAll(h);
  BTHD_7550_P_Config(h);
  BTHD_7550_P_ResetAcquire(h);

  retCode = BTHD_7550_P_Acquire(h);
  p7550->pStatus->ReacquireCount++;

  switch (retCode) {
  case THD_AcquireResult_Lock:          BDBG_MSG(("BTHD_7550_P_ThdTop:\tLock"));            break;
  case THD_AcquireResult_NoFFTLock:     BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo FFT Lock"));     break;
  case THD_AcquireResult_NoCarrierLock: BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo Carrier Lock")); break;
  case THD_AcquireResult_NoSPLock:      BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo SP Lock"));      break;
  case THD_AcquireResult_NoTPSLock:     BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo TPS Lock"));     break;
  case THD_AcquireResult_NoFECLock:     BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo FEC Lock"));     break;
  case THD_AcquireResult_NoSignal:      BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo Signal"));       break;
  case THD_AcquireResult_NoDVBTSignal:  BDBG_MSG(("BTHD_7550_P_ThdTop:\tNo DVBT Signal"));  break;
  default: BDBG_MSG(("BTHD_7550_P_ThdTop:\tretCode = %d", retCode));
  }

  if ((retCode == THD_AcquireResult_NoSignal) || (retCode == THD_AcquireResult_NoDVBTSignal) || (retCode == THD_AcquireResult_NoTPSLock))
    p7550->ThdLockStatus |= (1U << THD_LockStatusBit_NoDVBTSignal);

  if ((retCode == THD_AcquireResult_NoSignal) || (retCode == THD_AcquireResult_NoDVBTSignal) || (retCode == THD_AcquireResult_NoTPSLock)  || (retCode == THD_AcquireResult_NoFECLock))
  {
    if (p7550->pAcquireParam->AcquireStartMode & THD_AcquireStartMode_Scan)
    {
      p7550->pAcquireParam->AcquireStartMode &= ~THD_AcquireStartMode_Scan;
      BKNI_SetEvent(p7550->hLockEvent);
      return retCode;
    }
  }


  /*** If receiver lock was achieved, then clear the status but preserve the reacquire count. ***/

  if ((retCode == THD_AcquireResult_Lock) && (retCode != THD_AcquireResult_InitLockState)) {    
    ReacquireCount = p7550->pStatus->ReacquireCount;
    BTHD_7550_P_ResetStatusHW(h);
    BTHD_7550_P_ResetStatus(h);
    p7550->pStatus->ReacquireCount = ReacquireCount;
    BTHD_7550_P_SetTs(h);
  }

  /* Check for loss-of-lock in ce_mode=Fixed with timing FD enabled.  If lock is lost, try timing PD */
  /* Skip this fallback for ISDBT for now */
  if (!(p7550->pAcquireParam->Standard == THD_Standard_ISDBT)) {
    /*** Check for FEC lock ***/
    BTHD_7550_P_LockOld(h);
    if (!((p7550->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1) && (p7550->pStatus->ChannelEstimatorMode == THD_ChannelEstimatorMode_Fixed) && !BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_CP, PHASE_FREQ_SELECT )) {
      BDBG_MSG(("BTHD_7550_P_ThdTop:\tLock fallback from timing FD to PD"));
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, F_SPACE, 2 );                              /* 12-bin timing pd pilot spacing (mandatory for long-echo channels)*/
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, PHASE_FREQ_SELECT, 1 );                    /* Timing pd enable*/
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_TPHASE_FRZ, 0 );                       /* Unfreeze timing pd*/  
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00010200 * (2048/2048) );    /* Ki=2^(-13),Kl=2^(-4) (for CCI)*/
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000200 );                  /* FFT window leak to timing loop disabled (prevents "wandering" at low SNR)*/
    }
  }

  /*** Initialize Lock Detector State ***/
  BTHD_7550_P_Lock(h, 1);

  if((p7550->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1) {
    p7550->pAcquireParam->EventMode |= THD_Event_LockUpdate;
    BKNI_SetEvent(p7550->hLockEvent);
  }

  BTMR_StopTimer(p7550->hTimer);
  BTMR_StartTimer(p7550->hTimer, 1000);   /* the timer is in Micro second */

  return retCode;
}

/***************************************************************************
* BTHD_7550_P_ReadMemory()
***************************************************************************/
BERR_Code BTHD_7550_P_ReadMemory(BTHD_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
  BSTD_UNUSED(h);
  BSTD_UNUSED(addr);
  BSTD_UNUSED(buf);
  BSTD_UNUSED(n);
  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_P_WriteMemory()
*** ************************************************************************/
BERR_Code BTHD_7550_P_WriteMemory(BTHD_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
  BSTD_UNUSED(h);
  BSTD_UNUSED(addr);
  BSTD_UNUSED(buf);
  BSTD_UNUSED(n);
  return BERR_SUCCESS;
}

/***************************************************************************
* Function: BTHD_7550_P_SetMode()
***************************************************************************/
void BTHD_7550_P_SetMode( BTHD_Handle h, THD_TransmissionMode_t TransmissionMode, THD_GuardInterval_t GuardInterval, THD_Qam_t Qam, THD_CodeRate_t CodeRateHP, THD_CodeRate_t CodeRateLP, THD_Hierarchy_t Hierarchy)
{
  uint32_t  tps;
  tps = (Qam << 4) | (Hierarchy << 8) | (CodeRateHP << 12) | (CodeRateLP << 16) | (GuardInterval << 20) | (TransmissionMode << 24) | (1U << 31);
  BREG_Write32(((BTHD_7550_P_Handle *)(h->pImpl))->hRegister, BCHP_THD_CORE_TPS_OV, tps );
}

/***************************************************************************
* BTHD_7550_P_SetTPS()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_SetTPS(BTHD_Handle h, THD_TransmissionMode_t TransmissionMode, THD_GuardInterval_t GuardInterval)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BTHD_RESULT return_val = THD_AcquireResult_InitLockState;
  uint32_t tps,N=2048;

  switch (TransmissionMode) {
  case THD_TransmissionMode_2k: N = 2048; break; 
  case THD_TransmissionMode_4k: N = 4096; break; 
  case THD_TransmissionMode_8k: N = 8192; break; 
  }

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, TPS_RST, 0 );                      /* Release TPS reset */

  if (p7550->pAcquireParam->TPSMode == THD_TPSMode_Manual) {
    /* BDBG_MSG(("BTHD_7550_P_SetTPS:\tManual TPS Mode")); */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, QAM, p7550->pAcquireParam->Qam);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, HIERARCHY, p7550->pAcquireParam->Hierarchy);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, CRATE_HP,  p7550->pAcquireParam->CodeRateHP);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, CRATE_LP,  p7550->pAcquireParam->CodeRateLP);
  } else {
    /* BDBG_MSG(("BTHD_7550_P_SetTPS:\tWaiting for TPS Lock ...")); */
    BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hTpsSyncCallback );
    if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hTpsSyncEvent, 150) != BERR_SUCCESS) {
      /* BDBG_MSG(("BTHD_7550_P_SetTPS:\tNo TPS Lock")); */
      return_val = THD_AcquireResult_NoTPSLock;
    }
    BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hTpsSyncCallback ); 
    if (return_val != THD_AcquireResult_NoTPSLock) {
      /* BDBG_MSG(("BTHD_7550_P_SetTPS:\tTPS Lock")); */
      BTHD_7550_PI_OSleep(h,1,N,GuardInterval);                            /* Wait 1 symbol */
      tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS );
      tps = (BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV) & 0xfff00000) | (tps & 0x000fffff);
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TPS_OV, tps );
    }
  }
  /* BDBG_MSG(("BTHD_7550_P_SetTPS:\tTHD_CORE_TPS_OV = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV))); */

  return return_val;
}

/***************************************************************************
* BTHD_7550_P_SetTMCC()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_SetTMCC(BTHD_Handle h, THD_TransmissionMode_t TransmissionMode, THD_GuardInterval_t GuardInterval)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BTHD_RESULT return_val = THD_AcquireResult_InitLockState;
  uint32_t temp_0, temp_1, N=2048;
  uint32_t fscntInit;

  switch (TransmissionMode) {
  case THD_TransmissionMode_2k: N = 2048; break; 
  case THD_TransmissionMode_4k: N = 4096; break; 
  case THD_TransmissionMode_8k: N = 8192; break; 
  }

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, TMCC_RST, 1);                        /* Set TMCC reset */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, TMCC_ERC_RST, 1);                    /* Set TMCC error count reset */ 
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, TMCC_ERC_RST, 0);                    /* Release TMCC error count reset */ 
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, ISDBT_TDI_RST, 1);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, ISDBT_TDI_RST, 0);

  if (p7550->pAcquireParam->ISDBT_TMCCMode == THD_TMCCMode_Manual) {
    /* BDBG_MSG(("BTHD_7550_P_SetTMCC:\tManual TMCC Mode")); */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, PRFLAG, p7550->pAcquireParam->ISDBT_PR);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_QAM, p7550->pAcquireParam->ISDBT_A_Qam);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_CR, p7550->pAcquireParam->ISDBT_A_CodeRate);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_TI, p7550->pAcquireParam->ISDBT_A_TimeInt);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_SEG, p7550->pAcquireParam->ISDBT_A_Segments);

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_QAM, p7550->pAcquireParam->ISDBT_B_Qam);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_CR, p7550->pAcquireParam->ISDBT_B_CodeRate);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_TI, p7550->pAcquireParam->ISDBT_B_TimeInt);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG, p7550->pAcquireParam->ISDBT_B_Segments);

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_QAM, p7550->pAcquireParam->ISDBT_C_Qam);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_CR, p7550->pAcquireParam->ISDBT_C_CodeRate);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_TI, p7550->pAcquireParam->ISDBT_C_TimeInt);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG, p7550->pAcquireParam->ISDBT_C_Segments);

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, USE_OV, 1);                  /* Set TMCC override */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, TMCC_RST, 0 );                    /* Release TMCC reset */
  } 
  else {
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, TMCC_RST, 0 );                    /* Release TMCC reset */ 
    /* BDBG_MSG(("BTHD_7550_P_SetTMCC:\tWaiting for TMCC Lock ...")); */
    BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hTmccSyncCallback );
    fscntInit = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT); 
    if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hTmccSyncEvent, 514) != BERR_SUCCESS) {
      BDBG_MSG(("BTHD_7550_P_SetTMCC:\tNo TMCC Lock"));
      return_val = THD_AcquireResult_NoTPSLock;
    }
    BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hTmccSyncCallback ); 
    /*
    BDBG_MSG(("BTHD_7550_P_SetTMCC:\tTMCC Search Time: %d ms", (fscntInit -BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT)) / 54000));
    BDBG_MSG(("BTHD_7550_P_SetTMCC:\tTMCC_SYNC=%x, TMCC_DEC=%x, TMCC_NBERC=%x, TMCC_CBERC=%x, TMCC_UBERC=%x", 
    BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_SYNC),
    BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_DEC),
    BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_NBERC),
    BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_CBERC),
    BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_UBERC)));
    */

    if (return_val != THD_AcquireResult_NoTPSLock) {
      /* BDBG_MSG(("BTHD_7550_P_SetTMCC:\tTMCC Lock")); */
      BTHD_7550_PI_OSleep(h,1,N,GuardInterval);                            /* Wait 1 symbol */

      temp_0 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_OV_0) & 0xfffe0000;
      temp_0 |= BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_MISC, PRFLAG) << 16;
      temp_0 |= BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_LAYER_0) & 0x0000ffff;
      temp_1 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_LAYER_1);
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TMCC_OV_0, temp_0);
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TMCC_OV_1, temp_1);
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TMCC_OV_0, USE_OV, 1);                /* Set TMCC override */
    }
  }
  /*
  BDBG_MSG(("BTHD_7550_P_SetTMCC:\tTHD_CORE_TMCC_OV_0 = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_OV_0)));
  BDBG_MSG(("BTHD_7550_P_SetTMCC:\tTHD_CORE_TMCC_OV_1 = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TMCC_OV_1)));
  */

  return return_val;
}

/***************************************************************************
* BTHD_7550_P_ISDBT_SetOI()
***************************************************************************/
void BTHD_7550_P_ISDBT_SetOI(BTHD_Handle h)
{

  /* OI smoothing only used for parallel mode. */
  /* Transport output rate (in bytes/sec) for ISDBT is always (FFT clock rate / 2). */
  BREG_Write32(((BTHD_7550_P_Handle *)(h->pImpl))->hRegister, BCHP_THD_CORE_OI_N, 0x00000001);
  BREG_Write32(((BTHD_7550_P_Handle *)(h->pImpl))->hRegister, BCHP_THD_CORE_OI_D, 0x00000002);

  /*
  BDBG_MSG(("BTHD_7550_P_ISDBT_SetOI:\tOIN = 0x%08x", 0x00000001));
  BDBG_MSG(("                        \tOID = 0x%08x", 0x00000002));
  */
}

/***************************************************************************
* BTHD_7550_P_SetOI()
***************************************************************************/
void BTHD_7550_P_SetOI(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  const thd_oi_cw_t * cw;
  uint32_t tps;
  THD_GuardInterval_t GuardInterval;
  THD_Qam_t Qam;
  THD_Hierarchy_t Hierarchy;
  THD_CodeRate_t CodeRateLP, CodeRateHP, CodeRate;

  tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV );
  GuardInterval = (tps >> 20) & 0x3;
  Qam = (tps >> 4) & 0x3;
  Hierarchy  = (tps >> 8) & 0x3;
  CodeRateHP = (tps >> 12) & 0x7;
  CodeRateLP = (tps >> 16) & 0x7;

  if ( p7550->pAcquireParam->PriorityMode == THD_PriorityMode_High) {
    CodeRate = CodeRateHP;
    if (Hierarchy)
      Qam = THD_Qam_Qpsk;
  } else {
    CodeRate = CodeRateHP;
    if (Hierarchy)
      Qam -= 1;
  }

  cw = &thd_oi_table[GuardInterval][Qam][CodeRate];
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_OI_N, cw->n );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_OI_D, cw->d );

  /* BDBG_MSG(("BTHD_7550_P_SetOI:\t\t(%d,%d,%d)",GuardInterval,Qam,CodeRate)); */
  /* BDBG_MSG(("BTHD_7550_P_SetOI:\t\tOIN = 0x%08x", cw->n)); */
  /* BDBG_MSG(("                  \t\tOID = 0x%08x", cw->d)); */
}

/***************************************************************************
* BTHD_7550_P_SetFEC()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_SetFEC(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BTHD_RESULT return_val = THD_AcquireResult_InitLockState;
  uint32_t tps;
  THD_CodeRate_t CodeRateLP, CodeRateHP;

  tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV );
  CodeRateHP = (tps >> 12) & 0x7;
  CodeRateLP = (tps >> 16) & 0x7;

  if ( p7550->pAcquireParam->PriorityMode == THD_PriorityMode_High)
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, HP, 1);
  else
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, HP, 0);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FEC_SYNC,0x07021f1e );      /* FEC sync allow 2 bad headers out of 8 */
  BTHD_7550_P_SetEq(h, THD_CoChannelMode_None);
  BTHD_7550_P_SetViterbi(h, CodeRateHP, CodeRateLP);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, SYNC_MODE, 1 );                /* TPS sync mode */    
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FEC_RST, 0 );  
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_RST, 0 );
  /* BDBG_MSG(("BTHD_7550_P_SetFEC:\tWaiting for FEC Lock ...")); */
  BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFecSyncCallback );
  if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFecSyncEvent, 100) != BERR_SUCCESS) {                    
    return_val = THD_AcquireResult_NoFECLock;
    /* BDBG_MSG(("BTHD_7550_P_SetFEC:\tNo FEC Lock")); */
  }
  BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFecSyncCallback ); 
  /* if (return_val != THD_AcquireResult_NoFECLock) */
  /* BDBG_MSG(("BTHD_7550_P_SetFEC:\tFEC Lock")); */

  return return_val;
}

/***************************************************************************
* BTHD_7550_P_ISDBT_SetFEC()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_ISDBT_SetFEC(BTHD_Handle h)
{
  BTHD_RESULT            return_val = THD_AcquireResult_InitLockState;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  /* Set soft decision exponent parameters */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, FORMAT_OV, 0);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, EXP_OFFSET, 0);

  /* Configure Viterbi and release resets */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_VIT, 0x00000100);           /* VIT reacq, no cycle slip, no spectrum inversion */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FEC_RST, 1 );  
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_RST, 1 );
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FEC_RST, 0 );  
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_RST, 0 );
  return return_val;
}

/***************************************************************************
* BTHD_7550_P_SetTs()
***************************************************************************/
void BTHD_7550_P_SetTs(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t  oi =0;

  switch (p7550->pAcquireParam->TSMode){
  case THD_TSMode_Serial:   oi = 0x00000003; break;                          /* OI serial mode  */
  case THD_TSMode_Parallel: oi = 0x00000101; break;                          /* OI suppress clock when not valid, parallel smooth mode */
  default:  BDBG_WRN(("BTHD_7550_P_SetTs: THD_TSMode_None not supported")); break;
  }  
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_OI, oi);
}

/***************************************************************************
* BTHD_7550_P_SetFrequency()
***************************************************************************/
void BTHD_7550_P_SetFrequency(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t bw=0, tl_fcw, tl_n, tl_d, cl_n;
  uint32_t cl_fcw=0,CenterFreq;
  uint32_t pouthi, poutlo, pouthi2;


  /* Set the baseband rate */
  switch(p7550->pAcquireParam->Bandwidth) {
  case THD_Bandwidth_8MHz: bw = 8; break;
  case THD_Bandwidth_7MHz: bw = 7; break;
  case THD_Bandwidth_6MHz: bw = 6; break;
  case THD_Bandwidth_5MHz: bw = 5; break;
  }
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    tl_n   = 512000000 * bw / 3;      /* at bw = 6MHz, fcw = 2 * (512/63) MHz */
    tl_d   = p7550->pAcquireParam->SampleFreq * 63;
  }
  else {
    tl_n   = (2 * 8000000) * bw;
    tl_d   = p7550->pAcquireParam->SampleFreq * 7;
  }
  /*tl_fcw = (tl_n * (1ULL << 32)) / tl_d; */
  BMTH_HILO_32TO64_Mul((tl_n * 2), 0x80000000, &pouthi, &poutlo);
  BMTH_HILO_64TO64_Div32(pouthi, poutlo, tl_d, &pouthi2, &tl_fcw);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_FCW, tl_fcw );

  /* Set the carrier frequency */
  if ((p7550->pAcquireParam->Standard == THD_Standard_ISDBT) && 
    (p7550->pAcquireParam->CenterFreq > 54000000)) {                
      CenterFreq = p7550->pAcquireParam->CenterFreq - 45000000 - 11250000 + 13500000;
  } else
    CenterFreq = p7550->pAcquireParam->CenterFreq;

  cl_n   = CenterFreq ? (long)(p7550->pAcquireParam->SampleFreq - CenterFreq) : 0L;
  /*cl_fcw = (cl_n * (1LL << 32)) / (long)p7550->pAcquireParam->SampleFreq;    */    /* pow(2,32) = 4294967296 */
  BMTH_HILO_32TO64_Mul((cl_n * 2), 0x80000000, &pouthi, &poutlo);
  BMTH_HILO_64TO64_Div32(pouthi, poutlo, p7550->pAcquireParam->SampleFreq, &pouthi2, &cl_fcw);

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_FCW, cl_fcw );

  /* BDBG_MSG(("BTHD_7550_P_SetFrequency:\tTL_FCW = 0x%08x", tl_fcw)); */
  /* BDBG_MSG(("BTHD_7550_P_SetFrequency:\tCL_FCW = 0x%08x", cl_fcw)); */
}
																	 
/***************************************************************************
* BTHD_7550_P_SetFrontEnd()
***************************************************************************/
void BTHD_7550_P_SetFrontEnd(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  if (p7550->pAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_GLB,0x00000028 );               /* Fs=54,Baseband input,BE=disabled,ICE=acquisition */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FE,0x040000c8 );                /* DCO BW=2^(-20), posedge, spinv=0, 2's complement */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, FIR0_BYP, 0 );                     /* Enable ping-pong compensation filter */

  } else {
    BDBG_MSG(("BTHD_7550_P_SetFrontEnd:\tIF input mode"));
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_GLB,0x00000020 );               /* Fs=54,IF input,BE=disabled,ICE=acquisition */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FE,0x04000008 );                /* DCO BW=2^(-20), posedge, spinv=0, 2's complement */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, FIR0_BYP, 1 );                     /* Disable ping-pong compensation filter */


    /* External tuner w/sample rate converter */
    if ((p7550->pAcquireParam->Standard == THD_Standard_ISDBT) && 
      (p7550->pAcquireParam->CenterFreq > 54000000)) {                
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_SRC_M, 0x00d55552 );
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_SRC_N, 0x00fffffc );
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, FE_INPUT, 2 );                   /* Use SRC output */

    }
  }
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT)			
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, STD, 2);                            /* ISDBT Mode */
}

/***************************************************************************
* BTHD_7550_P_SetAnalogAGC()
***************************************************************************/
void BTHD_7550_P_SetAnalogAGC(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  if (p7550->pAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
  } else {
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC,0x00004004 );              /* IF AGC out IF delsig, IF delsig 1st order, RF AGC out RF delsig, RF delsig 1st order, signal beta=2^(-10) */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC_COEF,0x00001014 );         /* Kif=2^(-16),Krf=-2^(-20) */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC_THRESH,0x94005eca );
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC_ILIMIT,0xffff0000 );       /* Limit IF sigma-delta swing (lower=min,upper=max) */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC_RLIMIT,0xffff0000 );       /* Limit RF sigma-delta swing (lower=min,upper=max) */
  }
}

/***************************************************************************
* BTHD_7550_P_GetNotchISDBT()
***************************************************************************/
/* ISDBT needs this new GetNotch function because the noise exponents used by GetNotch are not captured in the 7550 snooper in ISDBT mode */
BTHD_RESULT  BTHD_7550_P_GetNotchISDBT(BTHD_Handle h, THD_TransmissionMode_t TransmissionMode)
{ 
  uint32_t iteration=0, start=0,idx,value,NumCoChannelFound = 0;
  uint32_t max_idx=0, max=0, mean;
  uint32_t errpwr;
  bool CoChannelPresent = false;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  /* Get average pilot noise level.  Scale factor of 45 gives good match to mean snooper level for white noise */
  errpwr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNRP)/45;
  /* BDBG_MSG(("BTHD_7550_P_GetNotch:\terrpwr = %d", errpwr)); */

  /* Determine whether analog TV CCI is present using snooper. Look for the presence of any 2 of the video, color, or audio carriers.  
  This 2-out-of-3 criterion allows differentiation between true analog TV CCI and spurs which often occur around sensitivity. 
  If this criterion is too strict, consider using a 1-out-of-3 criterion for input power levels above sensitivity and
  this 2-out-of-3 criterion for input power levels near sensitivity */
  iteration = 0;
  while ((iteration < 3) && !CoChannelPresent) {
    switch (iteration) {
      /* Snooper captures only active carriers.  For ISDBT, there are 4617 active carriers. */
      /* start = 2808 - 128*3 - f*8192*(63/512) */
      case 0: if (TransmissionMode == THD_TransmissionMode_2k) start=000; else start = 0516; break;  /* Video carrier (f = -1.893) */
      case 1: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 4124; break;  /* Color carrier (f =  1.687) */
      case 2: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 4848; break;  /* Audio carrier (f =  2.607) */
    }
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_RECORD_CFG,0x05030000 + start );  /* CE snooper capture exponents, snapshot mode, step=3 */

    while (!BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_CE_RECORD_CFG, DONE));
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_READ_INDEX,0x00000000 );
    mean = 0;
    for (idx=0; idx<256; idx++) {
      value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CE_READ_DATA );
      if ((idx == 0) || (value > max)) {                                       /* Compute min, smaller noise exponent -> larger noise */
        max = value;
        max_idx = idx;
      }
      /* mean += value; */
    }
    /* mean = mean / 256; */
    /* BDBG_MSG(("BTHD_7550_P_GetNotchISDBT:\titer=%d, max_idx = %d, max = %d, 16*errpwr = %d", iteration, max_idx, max, (16*errpwr)));*/

    if (max >= 16*errpwr)                                                                     /* CCI present if min_exp-mean_exp > 3 */
      NumCoChannelFound++;
    if (NumCoChannelFound > 1)
      CoChannelPresent = true;
    iteration++;
  }

  return CoChannelPresent;
}


/***************************************************************************
* BTHD_7550_P_GetNotch()
***************************************************************************/
BTHD_RESULT  BTHD_7550_P_GetNotch(BTHD_Handle h, THD_TransmissionMode_t TransmissionMode)
{ 
  uint32_t iteration=0, start=0,idx,value,cur_exp,min_exp=0,mean_exp,min_idx,NumCoChannelFound = 0;
  bool CoChannelPresent = false;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  /* Determine whether analog TV CCI is present using snooper. Look for the presence of any 2 of the video, color, or audio carriers.  
  This 2-out-of-3 criterion allows differentiation between true analog TV CCI and spurs which often occur around sensitivity. 
  If this criterion is too strict, consider using a 1-out-of-3 criterion for input power levels above sensitivity and
  this 2-out-of-3 criterion for input power levels near sensitivity */
  iteration = 0;
  while ((iteration < 3) && !CoChannelPresent) {
    switch (iteration) {
    case 0: if (TransmissionMode == THD_TransmissionMode_2k) start=000; else start = 0561; break;  /* Video carrier */
    case 1: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 5280; break;  /* Color carrier */
    case 2: if (TransmissionMode == THD_TransmissionMode_2k) start=936; else start = 6048; break;  /* Audio carrier */
    }
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_RECORD_CFG,0x07030000 + start );  /* CE snooper capture exponents, snapshot mode, step=3 */

    while (!BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_CE_RECORD_CFG, DONE));
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_READ_INDEX,0x00000000 );
    for (idx=0; idx<256; idx++) {
      value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CE_READ_DATA );
      cur_exp = (value & 0x0000fc00) >> 10;
      if ((idx == 0) || (cur_exp < min_exp)) {                                 /* Compute min, smaller noise exponent -> larger noise */
        min_exp = cur_exp;
        min_idx = idx;
      }
    }
    mean_exp = (value & 0x000003f0) >> 4;
	
	/*BYP = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_BYP,EXP_AVG_NSE_BYP);
    BDBG_MSG(("BTHD_7550_P_GetNotch:\titer=%d, min_idx = %d, min_exp+0x0c = %d, mean_exp = %d, BYP=%X", iteration, min_idx, (min_exp+0x0c), mean_exp, BYP));*/

    if (mean_exp > (min_exp + 0x0c))                                           /* CCI present if min_exp-mean_exp > 3 */
      NumCoChannelFound++;
    if (NumCoChannelFound > 1)
      {CoChannelPresent = true;}
    iteration++;
  }
  return CoChannelPresent;
}


/***************************************************************************
* BTHD_7550_P_SetNotch()
***************************************************************************/
void BTHD_7550_P_SetNotch(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t bw=0,fcw;
  int32_t x[3],y,n,num_notches;
  uint32_t pouthi, poutlo, poutlo2, utmp, tmp;

  /* Set the baseband rate */
  switch(p7550->pAcquireParam->Bandwidth) {
  case THD_Bandwidth_8MHz: bw = 8; break;
  case THD_Bandwidth_7MHz: bw = 7; break;
  case THD_Bandwidth_6MHz: bw = 6; break;
  case THD_Bandwidth_5MHz: bw = 5; break;
  }

  switch(p7550->pAcquireParam->CoChannelMode) {
  case THD_CoChannelMode_None:  break;
  case THD_CoChannelMode_Auto:
    if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
      /* NTSC Co-channel */
      x[0] = -1893000;                                  /* NTSC picture carrier */             
      x[1] =  1687000;                                  /* NTSC color carrier */
      x[2] =  2607000;                                  /* NTSC FM audio carrier */
      num_notches = 3;
    }
    else {
      /* PAL Co-channel */
      x[0] = -2750000 + (long)(500000*(8-bw));					/* PALB/G picture  carrier {-2.75,-2.25,-1.75} MHz for BW={8,7,6} MHz */
      x[1] =  1550000 + (long)(500000*(8-bw));					/* PALB/G color    carrier { 1.55, 2.05, 2.55} MHz for BW={8,7,6} MHz */
      x[2] =  2750000 + (long)(500000*(8-bw));					/* PALB/G FM audio carrier { 2.75, 3.25, 3.75} MHz for BW={8,7,6} MHz */
      num_notches = 3;
    }

    /* Compute FCWs */
    for (n=0; n<num_notches; n++) {
      if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {      
        /* Hard-coded for 6MHz channels */
        if (n)
        {
          if ((x[n]-x[n-1]) < 0)
          {
            tmp = ~(x[n]-x[n-1]);		
            tmp = tmp + 1;		
          }
          else
            tmp = (x[n]-x[n-1]);

          BMTH_HILO_32TO64_Mul(tmp, (0x1000000 * 63), &pouthi, &poutlo);
          BMTH_HILO_64TO64_Div32(pouthi, poutlo, 512000000, &utmp, &poutlo2);
          if ((x[n]-x[n-1]) < 0)
          {
            y = ~poutlo2;
            y = y + 1;	
          }
          else
            y = poutlo2;
          /*y = ((x[n]-x[n-1]) * (1LL<<24) * 63)/(512000000);*/
        }
        else
        {
          if ((x[n]-0     ) < 0)
          {
            tmp = ~(x[n]-0     );		
            tmp = tmp + 1;		
          }
          else
            tmp = (x[n]-0     );

          BMTH_HILO_32TO64_Mul(tmp, (0x1000000 * 63), &pouthi, &poutlo);
          BMTH_HILO_64TO64_Div32(pouthi, poutlo, 512000000, &utmp, &poutlo2);
          if ((x[n]-0     ) < 0)
          {
            y = ~poutlo2;
            y = y + 1;	
          }
          else
            y = poutlo2;

          /*y = ((x[n]-0     ) * (1LL<<24) * 63)/(512000000);	*/
        }
      } else {
        if (n)
        {
          if ((x[n]-x[n-1]) < 0)
          {
            tmp = ~(x[n]-x[n-1]);		
            tmp = tmp + 1;		
          }
          else
            tmp = (x[n]-x[n-1]);

          BMTH_HILO_32TO64_Mul(tmp, (0x1000000 * 7), &pouthi, &poutlo);
          BMTH_HILO_64TO64_Div32(pouthi, poutlo, (bw*8000000), &utmp, &poutlo2);
          if ((x[n]-x[n-1]) < 0)
          {
            y = ~poutlo2;
            y = y + 1;	
          }
          else
            y = poutlo2;
          /*y = ((x[n]-x[n-1]) * (1LL<<24) * 7)/(bw*8000000);*/

        }
        else
        {
          if ((x[n]-0     ) < 0)
          {
            tmp = ~(x[n]-0     );		
            tmp = tmp + 1;		
          }
          else
            tmp = (x[n]-0     );
          BMTH_HILO_32TO64_Mul(tmp, (0x1000000 * 7), &pouthi, &poutlo);
          BMTH_HILO_64TO64_Div32(pouthi, poutlo, (bw*8000000), &utmp, &poutlo2);
          if ((x[n]-0     ) < 0)
          {
            y = ~poutlo2;
            y = y + 1;	
          }
          else
            y = poutlo2;

          /*y = ((x[n]-0     ) * (1LL<<24) * 7)/(bw*8000000);	*/
        }
      }
      if (y < 0)
        fcw = ((uint32_t)((1U << 24) + y)) << 8;
      else
        fcw = ((uint32_t)y) << 8; 
      switch(n) {
  case 0: BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH0_FCW, fcw ); break;
  case 1: BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH1_FCW, fcw ); break;
  case 2: BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH2_FCW, fcw ); break;
  default: break;
      }
    }

    /* Enable notch filter */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, NOTCH_BYP, 0);  
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_DEPTH,0x00000aaa );        /* Acquisition notch depth and width */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_WIDTH,0x00000ddd );                        

    /* BDBG_MSG(("BTHD_7550_P_SetNotch:\tTHD_CORE_NOTCH0_FCW = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NOTCH0_FCW))); */
    /* BDBG_MSG(("BTHD_7550_P_SetNotch:\tTHD_CORE_NOTCH1_FCW = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NOTCH1_FCW))); */
    /* BDBG_MSG(("BTHD_7550_P_SetNotch:\tTHD_CORE_NOTCH2_FCW = 0x%08x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NOTCH2_FCW))); */

    break;        
  case THD_CoChannelMode_Other: break;
  default: break;
  }
  return;
}

/***************************************************************************
* BTHD_7550_P_SetTransGuard()
*** ************************************************************************/
THD_TransGuardResult_t BTHD_7550_P_SetTransGuard(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t n,d,maxN=0,maxD=0,N=2048;
  uint32_t curCorr, maxMinRatio, maxThreshold;
  uint32_t maxCorr = 0;              /* maxCorr initialization */
  uint32_t minCorr[3];
  THD_TransGuardResult_t mode_guard_result = THD_TransGuardResult_None;
  BERR_Code retCode = BERR_SUCCESS;
  uint32_t nMin = 0, nMax = 0, nInc = 1;
  uint32_t dMin = 0, dMax = 3;
  uint32_t length;

  switch (p7550->pAcquireParam->TransGuardMode) {
    case THD_TransGuardMode_Auto_DVBT:    nMin = 0; nMax = 1; nInc = 1; break;
    case THD_TransGuardMode_Auto_ISDBT:   nMin = 1; nMax = 2; nInc = 1; break;
    case THD_TransGuardMode_Auto:         nMin = 0; nMax = 2; nInc = 1; break;
    default:                              break;
  }

  /* BDBG_MSG(("BTHD_7550_P_SetTransGuard:\tnMin=%d nMax=%d nInc=%d", nMin, nMax, nInc)); */
  if (p7550->pAcquireParam->TransGuardMode != THD_TransGuardMode_Manual) {
    BTHD_7550_P_WriteLpf1CoefAcq(h);                                           /* Setup LPF1 acquisition coefficients */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW,0x00a01020 );                /* FFT window max correlation,postcursor bias,disable fft,sync_symbols=32,beta=2^(-3),slip_limit=0 */
    for (n=nMin; n<=nMax; n+=nInc) {
      minCorr[n] = 0x7fffffff;                                                 /* minCorr initialization */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, TRANS_MODE, n);               /* Write N */

      /*** Define Guard Interval search range ***/

      if (p7550->pAcquireParam->TransGuardMode == THD_TransGuardMode_Auto_ISDBT)
        switch (n) {
          case 1:   dMin = 1; dMax = 3; break;                                 /* 8K: {1/4, 1/8, 1/16} */                                        
          case 2:   dMin = 2; dMax = 3; break;                                 /* 4K: {1/4, 1/8} */
          default:  dMin = 0; dMax = 3; break;
      }
      else {                                                                   /* Default: {1/4, 1/8, 1/16, 1/32} */
        dMin = 0;
        dMax = 3;
      }

      for (d=dMin; d<=dMax; d++) {
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, GUARD, d);                  /* Write D */
        switch (n) {
          case 0:  length = 2048 >> (5-d); break;
          case 1:  length = 8192 >> (5-d); break;
          case 2:  length = 4096 >> (5-d); break;
          default: length = 2048 >> (5-d); break;
        }
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FW_SEARCH, MVG_SUM_LENGTH, length); /* L=D */ 
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FW_RST, 0);                    /* Release FFT window reset */
        BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwCorrMaxCallback );
        retCode = BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwCorrMaxEvent, 500); /* Wait for FFT window sync */
        if (retCode != BERR_SUCCESS) {
          /*BDBG_MSG(("BTHD_7550_P_SetTransGuard:\tNo hFwCorrMaxEvent")); */
        }
        BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwCorrMaxCallback ); 
        curCorr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FW_CORR_MAX ) >> d;

        if (curCorr > maxCorr) {
          maxCorr = curCorr;    /* Update maxCorr */
          maxN = n;             /* Update maxN*/
          maxD = d;             /* Update maxD*/
        }
        if (curCorr < minCorr[n])
          minCorr[n] = curCorr; /* Update minCorr */

        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FW_RST, 1);                   /* Hold FFT window reset*/

        /*BDBG_MSG(("N=%d,D=%d,Cur=%08x,Max=%08x,Min=%08x,maxN=%d,maxD=%d",n,d,curCorr,maxCorr,minCorr[n],maxN,maxD)); */
      }
    }

    maxMinRatio = minCorr[maxN] * p7550->pAcquireParam->TransGuardMaxMinRatioThreshold[maxD];

    /* Normalize max correlation threshold relative to current DAGC2 threshold. For consistency with previous chips (2930,3556), 
    p7550->pAcquireParam->TransGuardMaxThreshold is defined assuming a DAGC2 threshold=14dB=0x028c4 ((uint32_t)(pow(10.0,-14/10)* 262144)). */
    maxThreshold = (p7550->pAcquireParam->TransGuardMaxThreshold / 0x028c4) * BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_DAGC2, THRESH); 

    /* Threshold scaling based on mode (2k,4k,8k)*/
    switch (maxN) {
          case 1:	maxThreshold = 3* maxThreshold ; break;                                 /* 8K (multiply by 3)*/                                        
          case 2:   maxThreshold = (maxThreshold >> 1) ; break;                             /* 4K (multiply by 2)*/
          default:  break;
    }

    if ((maxCorr < maxThreshold) && ((maxCorr << 4) < maxMinRatio))
      mode_guard_result = THD_TransGuardResult_NoSignal;
    else
      if ((maxCorr << 4) < maxMinRatio)
        mode_guard_result = THD_TransGuardResult_CoChannelPresent;

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, TRANS_MODE, maxN);              /* Write N*/
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_TPS_OV, GUARD, maxD);                   /* Write D*/
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW, 0x00202050 );               /* FFT window postcursor bias,disable fft,sync_symbols=32,beta=2^(-5),slip_limit=0*/
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FW_SEARCH, MVG_SUM_LENGTH, 8);          /* L=8*/

    switch (maxN) {
    case THD_TransmissionMode_2k: N = 2048; break; 
    case THD_TransmissionMode_4k: N = 4096; break; 
    case THD_TransmissionMode_8k: N = 8192; break; 
    }

    BTHD_7550_P_WriteLpf1Coef(h);                                              /* Setup LPF1 tracking coefficients */
    BTHD_7550_PI_OSleep(h,2,N,maxD);
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, BE_RST, 1 );                       /* Reset BE (Necessary to improve acquisition probability) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, BE_RST, 0 );      
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FFT_RST, 1 );                      /* Reset FFT (Necessary to improve acquisition probability) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FFT_RST, 0 );
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FW, OUT_ENABLE, 1);

    /*BDBG_MSG(("BTHD_7550_P_SetTransGuard:\tN=%d, D=%d", maxN, maxD)); */
  }
  return mode_guard_result;
}

/***************************************************************************
* BTHD_7550_P_SetFW()
***************************************************************************/
void BTHD_7550_P_SetFW( BTHD_Handle h, THD_FFTWindowMode_t FFTWindowMode, THD_TransmissionMode_t TransmissionMode, THD_GuardInterval_t GuardInterval)
{
  uint32_t fw= 0, fw_search=0, N=2048;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  switch (TransmissionMode) {
  case THD_TransmissionMode_2k: N = 2048; break; 
  case THD_TransmissionMode_4k: N = 4096; break; 
  case THD_TransmissionMode_8k: N = 8192; break; 
  }

  switch (FFTWindowMode) {
  case THD_FFTWindowMode_InSpan:
    fw_search = 0x11802000 | (8 * (N/2048));  /* FFT window min_scale=1.5,start_index_mode=1,1st peak mode,L=8*N/2048*/
    fw = 0x03214031;
    break;
  case THD_FFTWindowMode_OutOfSpanPost:  
    fw_search = 0x11400000 | (8 * (N/2048));  /* FFT window min_scale=1.25,start_index_mode=0,1st peak mode,L=8*N/2048  */    
    fw = 0x03014031;      
    break;
  case THD_FFTWindowMode_OutOfSpanPre:  
    fw_search = 0x11402000 | (8 * (N/2048));  /* FFT window min_scale=1.25,start_index_mode=1,1st peak mode,L=8*N/2048  */    
    fw = 0x03014031;
    break;
  default:
    BDBG_WRN(("BTHD_7550_P_SetFW: THD_FFTWindowMode_Auto not supported")); 
    break;
  } 

  /* Disable expanded search mode for GI=1/4 due to acquisition instability */
  /* if (GuardInterval == 3) */
  fw = fw & 0xfcffffff;

#ifdef FASTACQ
  /* Added to reduce acquisition time by estimating initial position in 32 vs 64 symbols.  Not used for OutOfSpanPost due
     to slow convergence if initial position estimate in error */
  if (FFTWindowMode != THD_FFTWindowMode_OutOfSpanPost)
	fw = (fw & 0xffff00ff) | 0x00002000;
#endif
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW,fw );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_SEARCH,fw_search);
  if (FFTWindowMode == THD_FFTWindowMode_OutOfSpanPre)
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_OFFSET,0x00000000 );         /* FFT window offset=0 */
  else if (FFTWindowMode == THD_FFTWindowMode_InSpan)
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_OFFSET,0x00000010 );         /* FFT window offset=16 */
  else
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_OFFSET,0x00000018 );         /* FFT window offset=24 */

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x0000020a );             /* FFT window leak to timing loop disabled, disallow zeros */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_SPAN,0x00000200 );             /* FFT window span scale = 0.25 */

  /* Setup frequency interpolator coefficients for out-of-span conditions */
  if (FFTWindowMode != THD_FFTWindowMode_InSpan) {
    if (FFTWindowMode == THD_FFTWindowMode_OutOfSpanPre) {
      if (GuardInterval == THD_GuardInterval_1_4)
        BTHD_7550_P_WriteFICoef(h,THD_FrequencyInterpolatorMode_OutOfSpanPre_1_4);
      else if (GuardInterval == THD_GuardInterval_1_8)
        BTHD_7550_P_WriteFICoef(h,THD_FrequencyInterpolatorMode_OutOfSpanPre_1_8);
      else
        BTHD_7550_P_WriteFICoef(h,THD_FrequencyInterpolatorMode_OutOfSpanPre_1_16);
    }
    else
      BTHD_7550_P_WriteFICoef(h,THD_FrequencyInterpolatorMode_OutOfSpanPost);
  }

  /*
  switch (FFTWindowMode) {
  case THD_FFTWindowMode_InSpan:        BDBG_MSG(("BTHD_7550_P_SetFW:\t\tIn-Span")); break;
  case THD_FFTWindowMode_OutOfSpanPost: BDBG_MSG(("BTHD_7550_P_SetFW:\t\tOut-of-Span Postcursor")); break;
  case THD_FFTWindowMode_OutOfSpanPre:  BDBG_MSG(("BTHD_7550_P_SetFW:\t\tOut-of-Span Precursor")); break;
  default:                              BDBG_WRN(("BTHD_7550_P_SetFW: THD_FFTWindowMode_Auto not supported")); break;
  }
  */
}

/***************************************************************************
* BTHD_7550_P_SetCE()
***************************************************************************/
void BTHD_7550_P_SetCE( BTHD_Handle h, THD_ChannelEstimatorMode_t ChannelEstimatorMode)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  switch (ChannelEstimatorMode) {
  case THD_ChannelEstimatorMode_Fixed:      
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, AVG_FF, 0x3);                       /* CE averager beta = 2^(-4) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 0);                    /* CE averager unbypassed */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, NSE_AVG_FF, 0x5 );                  /* EQ noise averager beta=2^(-8) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, EXP_AVG_NSE, 0x0 );                /* EQ nse + avg nse exponents unbypassed */
    break;
  case THD_ChannelEstimatorMode_Pedestrian: 
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, AVG_FF, 0x0);                       /* CE averager beta = 2^(-1) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 0);                    /* CE averager bypassed */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, NSE_AVG_FF, 0x5 );                  /* EQ noise averager beta=2^(-8) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, EXP_AVG_NSE, 0x0 );                /* EQ nse + avg nse exponents unbypassed */
    break;
  case THD_ChannelEstimatorMode_Mobile:   
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, AVG_FF, 0x0);                       /* CE averager beta = 2^(-1) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 1);                    /* CE averager bypassed */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, NSE_AVG_FF, 0x5 );                  /* EQ noise averager beta=2^(-8) */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, EXP_AVG_NSE, 0x3 );                /* EQ nse + avg nse exponents unbypassed */
    break;
  default:
    BDBG_WRN(("BTHD_7550_P_SetCE: THD_ChannelEstimatorMode_Auto not supported")); 
    break;
  }    

  /*
  switch (ChannelEstimatorMode) {
  case THD_ChannelEstimatorMode_Fixed:      BDBG_MSG(("BTHD_7550_P_SetCE:\t\tFixed")); break;
  case THD_ChannelEstimatorMode_Pedestrian: BDBG_MSG(("BTHD_7550_P_SetCE:\t\tPedestrian")); break;
  case THD_ChannelEstimatorMode_Mobile:     BDBG_MSG(("BTHD_7550_P_SetCE:\t\tMobile")); break;
  default:                                  BDBG_WRN(("BTHD_7550_P_SetCE: THD_ChannelEstimatorMode_Auto not supported")); break;
  }
  */
}

/***************************************************************************
* BTHD_7550_P_SetICE ()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_SetICE(BTHD_Handle h, THD_TransmissionMode_t TransmissionMode,THD_GuardInterval_t GuardInterval)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  int n,k;
  uint32_t ice[16],ice_val[16],ice_cnt[16],ice_found,ice_val_max,ice_cnt_max,ice_index_max,N=2048;
  uint32_t ice_val_best=0, ice_cnt_best=0, ice_index_best=0, spinv_best=0;
  uint32_t iteration = 0;
  uint32_t carrier_off   = 0;
  uint32_t carrier_step  = 1;
  uint32_t carrier_limit = 1;
  int32_t  cl_int, cl_fcw;
  bool     ice_done=0;

#if (BCHP_VER >= BCHP_VER_A1)
  /* Determine if ISDBT AC1 data demodulation mode is enabled by checking msb of RESERVED2 register.  This mode requires the integer carrier
     offset to be correct to modulo N/(2*D) for proper phase alignment at the AC1 data slicer:
     
     GI = 1/4,  modulo 2
     GI = 1/8,  modulo 4
     GI = 1/16, modulo 8
     GI = 1/32, modulo 16
     
     The correct phasing can be found by trying multiple offsets to obtain the best alignment.  Stepping the offset by half the
     modulo range will produce rotations of 0/180 and 90/270 degrees. One of these two offsets should achieve good phasing for AC1 slicing. */

  if ( (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) && !(BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RESERVED2) & 0x80000000) )
    switch (GuardInterval) {
      case THD_GuardInterval_1_4:  carrier_limit = 2;  carrier_step = 1; break;
      case THD_GuardInterval_1_8:  carrier_limit = 4;  carrier_step = 2; break;
      case THD_GuardInterval_1_16: carrier_limit = 8;  carrier_step = 4; break;
      case THD_GuardInterval_1_32: carrier_limit = 16; carrier_step = 8; break;
  }
#endif

  switch (TransmissionMode) {
  case THD_TransmissionMode_2k: N = 2048; break; 
  case THD_TransmissionMode_4k: N = 4096; break; 
  case THD_TransmissionMode_8k: N = 8192; break; 
  }
 
  /* Collect ICE values with carrier loop frozen */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CL_FRZ, 1);                          /* Freeze carrier loop */
  cl_int = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_INT); 
  cl_fcw = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_FCW);
          
  for (iteration=0; iteration <2; iteration++) {
    for (carrier_off=0; carrier_off < carrier_limit; carrier_off += carrier_step) {

      /* To aid AC1 demodulation, offset initial carrier frequency */
      if (iteration == 0)
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, cl_int + (carrier_off << 21));
	  else  /* bug fix for spectral inversion.  need to keep the integrator programmed to the inverted value */
		BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, ((cl_int ^ 0xffffffff) + 1U) + (carrier_off << 21));

      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 0 );                /* Unfreeze integer carrier estimator */
      BTHD_7550_PI_OSleep(h,3,N,GuardInterval);
      for (n=0; n<16; n++) {
        ice[n] = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_IEST) + carrier_off;
        ice_val[n] = 0;
        ice_cnt[n] = 0;
        BTHD_7550_PI_OSleep(h,1,N,GuardInterval);                                             /* Wait 1 symbols */
        /* BDBG_MSG(("BTHD_7550_P_SetICE:\tICE = 0x%08x",ice[n])); */
      }

      /* Identify unique ICE values and count number-of-occurrences of each */
      ice_val[0] = ice[0];
      ice_cnt[0] = 1;
      for (n=1; n<16; n++) {
        ice_found = 0;
        for (k=0; k<n; k++) {
          if (ice[n] == ice_val[k]) {                                           /* found, increment the count */
            ice_cnt[k] = ice_cnt[k]+1;
            ice_found = 1;        
          }
        }
        if (!ice_found) {                                                       /* not found, add it to the table */
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
      /* BDBG_MSG(("BTHD_7550_P_SetICE:\tIC Offset = %d, ICE Max = 0x%08x (%d occurences)", carrier_off, ice_val[ice_index_max],ice_cnt_max)); */ 


      /* Keep track of the best result for comparison */

      if (ice_cnt_max > ice_cnt_best) {
        ice_cnt_best   =  ice_cnt_max;
        ice_val_best   =  ice_val_max;
        ice_index_best =  ice_index_max;
        spinv_best     =  iteration;
      }

      /* Check for valid ICE and exit */

      if ( ((p7550->pAcquireParam->Standard == THD_Standard_ISDBT) && (ice_cnt_best >= 12)) ||
        (!(p7550->pAcquireParam->Standard == THD_Standard_ISDBT) && (ice_cnt_best >= THD_ICESpectrumInversionThreshold)) ) {
          ice_done = 1;
          break;
      }
    }
 
    if (ice_done)
      break;

    /* Try spectral inversion on second iteration */
    /* BDBG_MSG(("BTHD_7550_P_SetICE:\tTrying spectral inversion")); */
    if (p7550->pAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FE, NEGATE_Q, 1);
    } else {
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_FCW, (cl_fcw ^ 0xffffffff) + 1U);
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, (cl_int ^ 0xffffffff) + 1U);       /*  Negate carrier integrator */ 
    }

    /* Freeze and wait 10 symbols */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 1 );                  /*  Freeze integer carrier estimator*/
    BTHD_7550_PI_OSleep(h,10,N,GuardInterval);
  }

  /* Reset CL_INT back to initial value (allowing for spectral inversion) */

  if (iteration)
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, (cl_int ^ 0xffffffff) + 1U);
  else
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, cl_int);


  /* Uninvert signal if needed.  This can happen if spectral inversion was tested but noninverted spectrum yielded a better ICE */

  if (iteration && !spinv_best) {
    BDBG_MSG(("BTHD_7550_P_SetICE:\tUndoing spectral inversion"));
    if (p7550->pAcquireParam->FrontEndMode == THD_FrontEndMode_Baseband) {
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FE, NEGATE_Q, 0);
    } else {
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_FCW, cl_fcw);
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT, cl_int);
    }
  }

  if (spinv_best)
    BDBG_WRN(("BTHD_7550_P_SetICE:\tSpectral Inversion Detected"));
  
  
  /* Detection flag for suspect ICE result */

  if (ice_cnt_best < 6) {
    BDBG_WRN(("BTHD_7550_P_SetICE:\tLow ICE Count Detected"));
    p7550->pStatus->LowICECount = 1;
  }
  else {
    p7550->pStatus->LowICECount = 0;
  }

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_CINT_FRZ, 1 );                    /* Freeze integer carrier estimator */
  BTHD_7550_PI_OSleep(h,1,N,GuardInterval);                                                   /* Wait 1 symbols */

   /* BDBG_MSG(("BTHD_7550_P_SetICE:\tICE Max = 0x%08x (%d occurences)",ice_val_best,ice_cnt_best)); */
  return(ice_val_best);
}


/***************************************************************************
* BTHD_7550_P_WriteFICoef()
***************************************************************************/
void BTHD_7550_P_WriteFICoef( BTHD_Handle h, THD_FrequencyInterpolatorMode_t FrequencyInterpolatorMode)
{
  int ndx;
  uint32_t coef;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FI_ADDR, 0 );

  for (ndx = 0; ndx < THD_FrequencyInterpolatorCoefLength; ++ndx) {
    coef = thd_freq_coef_table[ FrequencyInterpolatorMode*THD_FrequencyInterpolatorCoefLength + ndx ];
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FI_COEF, coef );
  }

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FI, 0x00000000);
}

/***************************************************************************
* BTHD_7550_P_WriteTICoef()
***************************************************************************/
void BTHD_7550_P_WriteTICoef( BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  /* time_interp_polyphase_1pre_3post_1_16_15dB (1/16) */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF01,  0x0f8922af);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF23,  0x3cf55eb4);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF45,  0x5a9a4ede);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF67,  0x324b02d5);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF89,  0xf51beb17);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF1011,0xecdafd40);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF1213,0x00a6034b);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF14,  0x03d00000);

  /* 2-tap time interpolator coefficients */
  /*
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF01,  0x18003000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF23,  0x48006000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF45,  0x48003000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF67,  0x18000000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF89,  0x00000000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF1011,0x00000000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF1213,0x00000000);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE_TCOEF14,  0x00000000);
  */
}   

/***************************************************************************
* BTHD_7550_P_WriteLpf1Coef()
***************************************************************************/
void BTHD_7550_P_WriteLpf1Coef( BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  /* firls(24,[0 4+1 14-1 27]/27,[1 1 0 0],[100 1]) */
  /*
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF0, 0x00000005 );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF1, 0x00000001 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF2, 0x00001fee ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF3, 0x00001fe9 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF4, 0x00000014 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF5, 0x0000004f ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF6, 0x00000034 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF7, 0x00001f95 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF8, 0x00001f0d ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF9, 0x00001faf ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF10, 0x000001e0 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF11, 0x00000482 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF12, 0x000005b2 );     
  BBREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF12, 0x000005b2 ); 
  */

  /* firls(24,[0 3.5 6 27]/27,[1 1 0 0],[1 0.1])'; */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF0,  0x0000003f );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF1,  0x00000022 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF2,  0x00001fef ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF3,  0x00001fb3 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF4,  0x00001f85 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF5,  0x00001f7e ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF6,  0x00001fb2 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF7,  0x00000028 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF8,  0x000000d3 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF9,  0x00000199 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF10, 0x00000252 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF11, 0x000002d4 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF12, 0x00000304 ); 
}

/***************************************************************************
* BTHD_7550_P_WriteLpf1CoefAcq()
***************************************************************************/
void BTHD_7550_P_WriteLpf1CoefAcq( BTHD_Handle h)
{
  /* firls(24,[0 3.125 3.95 6 7 27.0]/27.0,[1 1 0 0 0 0],[100 20 1])'; */
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF0,  0x00001fe0 );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF1,  0x00001f67 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF2,  0x00001f61 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF3,  0x00001f9b ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF4,  0x00001fe9 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF5,  0x00000035 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF6,  0x0000007c ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF7,  0x000000c8 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF8,  0x00000121 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF9,  0x00000187 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF10, 0x000001ea ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF11, 0x00000233 ); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_LPF_COEF12, 0x0000024e );  
}

/***************************************************************************
* BTHD_7550_P_SetEq()
***************************************************************************/
void BTHD_7550_P_SetEq( BTHD_Handle h, THD_CoChannelMode_t CoChannelMode)
{
  uint32_t format_ov,exp_offset=0,tps;
  THD_Qam_t Qam;
  THD_Hierarchy_t Hierarchy;
  THD_TransmissionMode_t TransmissionMode;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  static uint8_t format_ov_table_cci_off[][3] = {{0xd,0xe,0xe},
  {0xd,0xe,0xe},
  {0xd,0xe,0xf},
  {0xd,0xe,0xf}};
  static uint8_t format_ov_table_cci_on [][3] = {{0xb,0xb,0xb},
  {0xb,0xb,0xb},
  {0xb,0xe,0xf},
  {0xb,0xe,0xf}};
  static uint32_t exp_offset_table_cci_off[][3] = {{0x000,0x3f0,0x3f0},
  {0x000,0x3f8,0x3f8},
  {0x000,0x3f4,0x3f4}};
  static uint32_t exp_offset_table_cci_on [][3] = {{0x000,0x000,0x000},
  {0x000,0x000,0x000},
  {0x000,0x000,0x000}};

  tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV );
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
  /* BDBG_MSG(("BTHD_7550_P_SetEq:\t\t(H,Q,T) = (%d,%d,%d), format_ov = %d, exp_offset = 0x%03x", Hierarchy, Qam, TransmissionMode, format_ov, exp_offset)); */

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, EXP_USE_FORMAT_OV, format_ov );  /* EQ override bsft format */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, EXP_OFFSET, exp_offset );      
}

/***************************************************************************
* BTHD_7550_P_SetViterbi()
***************************************************************************/
void BTHD_7550_P_SetViterbi( BTHD_Handle h, THD_CodeRate_t CodeRateHP, THD_CodeRate_t CodeRateLP)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t Dly;
  THD_CodeRate_t CodeRate;

  if (p7550->pAcquireParam->PriorityMode == THD_PriorityMode_High)
    CodeRate = CodeRateHP;
  else
    CodeRate = CodeRateLP;

  switch(CodeRate) {
  case THD_CodeRate_1_2:  Dly = 0x00000000; break;
  case THD_CodeRate_2_3:  Dly = 0x00000110; break;
  case THD_CodeRate_3_4:  Dly = 0x00000208; break;
  case THD_CodeRate_5_6:  Dly = 0x00000310; break;
  case THD_CodeRate_7_8:  Dly = 0x00000518; break;  
  default:                Dly = 0x00000000; break;  
  }

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_VIT_OVS,Dly );     /* Viterbi overwrite sync mode delay */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_VIT,0x00000607 );  /* Viterbi overwrite sync mode enabled */
}

/***************************************************************************
* BTHD_7550_P_SetFrame()
***************************************************************************/
void BTHD_7550_P_SetFrame( BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  uint32_t tps;
  uint32_t fsize;
  unsigned D;
  unsigned Q;
  unsigned R;
  uint32_t num;
  uint32_t den;
  uint32_t bw=0;
#if 0
  uint32_t pouthi;
  uint32_t poutlo;
  ldiv_t snr_result;
#endif

  /* Extract the guard interval, QAM mode, and code rate from the TPS value. */
  tps = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV );
  D = (1U << (5-((tps >> 20) & 0x3)));
  Q = (tps >> 4)  & 0x3;
  R = thd_rate_denominator_tbl[ (tps >> 12) & 0x7 ];

  /* Handle errors in TPS.*/
  if (Q == 3) /* Handle invalid QAM.*/
    Q = 0;  /* Arbitrary QAM.*/

  /* Calculate the frame size. */
  switch(p7550->pAcquireParam->Bandwidth) {
    case THD_Bandwidth_8MHz: bw = 8; break;
    case THD_Bandwidth_7MHz: bw = 7; break;
    case THD_Bandwidth_6MHz: bw = 6; break;
    case THD_Bandwidth_5MHz: bw = 5; break;
  }
  num = 281250 * bw * D * (Q + 1) * (R - 1);  
  den = 272 * (D + 1) * R;
  fsize = (num + (den >> 1)) / den;

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FSIZE, (uint32_t)fsize );
  /* BDBG_MSG(("BTHD_7550_P_SetFrame:\tFSIZE  = 0x%08x", fsize));  */
#if 0
  BMTH_HILO_64TO64_Div32(0, 19, 5, &pouthi, &poutlo);
  BDBG_MSG(("BTHD_7550_P_SetFrame:\thigh: %d, low:%d", pouthi, poutlo)); 
  snr_result = ldiv( 19, 5 );
  BDBG_MSG(("BTHD_7550_P_SetFrame:\tquot: %d", snr_result.quot)); 
#endif
}

/***************************************************************************
* BTHD_7550_P_2560log10()
***************************************************************************/
uint32_t BTHD_7550_P_2560log10(uint32_t x)
{
  int32_t  x1;
  int32_t  x2;
  int32_t  x3;
  int32_t  x4;

  if (x == 0)
    return 0;

  x1 = 30;
  while (!((x >> x1) & 1))
    x1 = x1-1;
  x1 = x1+1;

  x2 = (int32_t)((x << 8) >> x1);
  x3 = -24381739 + x2*(62348 + (x2 << 7));
  x4 = 5907991 + (x2 << 16);

  return (unsigned)((770*(x3/(x4 >> 8) + (x1 << 8))) >> 8);
}

/***************************************************************************
* BTHD_7550_P_Status()
***************************************************************************/
void BTHD_7550_P_Status( BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  uint32_t  value,value2,CERC,CBERC,NBERC,UBERC,TBERC,NFERC,UFERC,TFERC,Nlog2=0, eq_snr, bw=0, N=2;
  uint32_t  CERC_B,CBERC_B,NBERC_B,UBERC_B,TBERC_B;
  uint32_t  CERC_C,CBERC_C,NBERC_C,UBERC_C,TBERC_C;
  long int cl_int, tl_int;
  uint32_t  lval1, encoded_qam, encoded_alpha, sft_scale, eq_snrp;
  uint32_t tmp_a=0, tmp_b=0, tmp_c=0, i, j, scale;
  int32_t tmp;
  long snr_curr;
  uint32_t pouthi, poutlo, pouthi2, poutlo2, pouthi3, poutlo3, utmp;
  uint32_t encoded_guard_interval, ce_beta, snr_factor;
  static uint8_t scale_table[] = { 29, 27, 25, 29 }; /* Support invalid QAM.*/
  static uint8_t sft_scale_table[][4] = 
  {
    {  2,  2,  2,  2 },
    { 10, 10, 20, 52 },
    { 42, 42, 60, 108},
    {  2,  2,  2,  2 } 
  };

  /* Determine transmission parameters */
  value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TPS_OV);
  switch((value >> 24) & 0x3) {
  case 0: p7550->pStatus->TransmissionMode=THD_TransmissionMode_2k; Nlog2=0; break;
  case 1: p7550->pStatus->TransmissionMode=THD_TransmissionMode_8k; Nlog2=2; break;
  case 2: p7550->pStatus->TransmissionMode=THD_TransmissionMode_4k; Nlog2=1; break;      
  }

  p7550->pStatus->GuardInterval = ((value >> 20) & 0x3);

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* Get EWS Flag */
    p7550->pStatus->ISDBT_EWS = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_MISC, ALERT );

    /* Get Partial Reception Flag */
    p7550->pStatus->ISDBT_PR  = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, PRFLAG);

    /* Get QAM Modes */
    p7550->pStatus->ISDBT_A_Qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_QAM);
    p7550->pStatus->ISDBT_B_Qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_QAM);
    p7550->pStatus->ISDBT_C_Qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_QAM);

    /* Get Code Rates */
    p7550->pStatus->ISDBT_A_CodeRate = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_CR);
    p7550->pStatus->ISDBT_B_CodeRate = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_CR);
    p7550->pStatus->ISDBT_C_CodeRate = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_CR);

    /* Get Time Interleaver Modes */
    p7550->pStatus->ISDBT_A_TimeInt = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_TI);
    p7550->pStatus->ISDBT_B_TimeInt = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_TI);
    p7550->pStatus->ISDBT_C_TimeInt = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_TI);

    /* Get Segments */
    p7550->pStatus->ISDBT_A_Segments = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_SEG);
    p7550->pStatus->ISDBT_B_Segments = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG);
    p7550->pStatus->ISDBT_C_Segments = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG);
  }
  else {
    p7550->pStatus->CodeRateLP   = (value >> 16) & 0x7;
    p7550->pStatus->CodeRateHP   = (value >> 12) & 0x7;
    p7550->pStatus->Hierarchy    = (value >> 8) & 0x3;
    p7550->pStatus->Interleaving = (value >> 10) & 0x1;
    p7550->pStatus->Qam = ((value >> 4) & 0x3);
  }

  /* Determine stream priority */
  if (BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_FEC, HP ))
    p7550->pStatus->Priority = THD_PriorityMode_High;
  else
    p7550->pStatus->Priority = THD_PriorityMode_Low;

  /* Compute RSSI */
  /*p7550->pStatus->RSSI = 0;  thd_get_rssi(h,frf,bbtarget); */

  /* Compute RF AGC gain */
  lval1 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_AAGC_RFILT);
  p7550->pStatus->RagcGain = (lval1 >> 16);

  /* Compute IF AGC gain */
  lval1 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_AAGC_IFILT);
  p7550->pStatus->IagcGain = (lval1 >> 16);

  /* Compute DAGC1 gain */
  value  = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_DAGC1_FILT,  LFO );
  p7550->pStatus->Dagc1Gain = (short)((long int)(BTHD_7550_P_2560log10(value) << 1) - 35449);  /* Application takes 20*log10(x/256.0) to obtain value in dB */

  /* Compute DAGC2 gain */
  value  = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_DAGC2_FILT,  LFO  ) ;
  p7550->pStatus->Dagc2Gain = (short)((long int)(BTHD_7550_P_2560log10(value) << 1) - 35449);  /* Application takes 20*log10(x/256.0) to obtain value in dB */

  /* Compute carrier offset */
  switch(p7550->pAcquireParam->Bandwidth) {
    case THD_Bandwidth_8MHz: bw = 8; break;
    case THD_Bandwidth_7MHz: bw = 7; break;
    case THD_Bandwidth_6MHz: bw = 6; break;
    case THD_Bandwidth_5MHz: bw = 5; break;
  }
  cl_int = (long int)BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_INT );
  if (cl_int < 0)
    cl_int = -cl_int;

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* freq = (cl_int / N / 2^32) * (2 * 512 / 63) * (bw / 6) * 1000000  */
    utmp = (1000000*bw);
    BMTH_HILO_32TO64_Mul(utmp, cl_int, &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, 378, &utmp, &poutlo2);
    BMTH_HILO_64TO64_Div32(utmp, poutlo2, 0x400000, &pouthi, &poutlo);  /* this is the same than >> 22 */
  }
  else {
    /* freq = (cl_int / N / 2^32) * (2 * 64 / 7) * (bw / 8) * 1000000 */
    utmp = (15625*bw);
    BMTH_HILO_32TO64_Mul(utmp, cl_int, &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, 7, &utmp, &poutlo2);
    BMTH_HILO_64TO64_Div32(utmp, poutlo2, 0x400000, &pouthi, &poutlo);  /* this is the same than >> 22 */
  }
  switch(p7550->pStatus->TransmissionMode) {
    case THD_TransmissionMode_2k: N=2; break;
    case THD_TransmissionMode_8k: N=8; break;
    case THD_TransmissionMode_4k: N=4; break;      
    default: N=2; break;
  }
  p7550->pStatus->CarrierOffset = (poutlo / N);

  cl_int = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_INT );
  if (cl_int < 0)
    p7550->pStatus->CarrierOffset = -p7550->pStatus->CarrierOffset;

  /* Compute timing offset */
  tl_int  = (long int)BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TL_INT );  
  if (tl_int < 0)
    tl_int = -tl_int;

  /* tmp = (((((long long int)p7550->pAcquireParam->SampleFreq)*tl_int)/1000000)*15625) >> (16 + 6 + 10);
  p7550->pStatus->TimingOffset = (long int)(tmp / N);
  BDBG_MSG((" timing offset 0x%08x  ", p7550->pStatus->TimingOffset));
  */
  BMTH_HILO_32TO64_Mul(p7550->pAcquireParam->SampleFreq, tl_int, &pouthi, &poutlo);
  BMTH_HILO_64TO64_Div32(pouthi, poutlo, 1000000, &utmp, &poutlo2);
  BMTH_HILO_64TO64_Mul(utmp, poutlo2, 0, 15625, &utmp, &poutlo);
  p7550->pStatus->TimingOffset = (long int)(utmp / N);

  tl_int  = (long int)BREG_Read32(p7550->hRegister, BCHP_THD_CORE_TL_INT );  
  if (tl_int < 0)
    p7550->pStatus->TimingOffset = -p7550->pStatus->TimingOffset;


  /* Compute data SNR */
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* Layer A SNR */
    eq_snr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNR);           /* error power read in 3.29 format */
    if (eq_snr >> (8+2)) {
      encoded_qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_0, LAYERA_QAM);   
      scale       = scale_table[encoded_qam];
      sft_scale   = sft_scale_table[encoded_qam][0];
      /*tmp_a       = (sft_scale << (scale-2))/(eq_snr >> (8+2));   */        /* 10*log10(tmp/256.0) to obtain value in dB */
      for (i=0, j=1; i<(scale-2); i++)
        j = j * 2;
      BMTH_HILO_32TO64_Mul(sft_scale, j, &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (eq_snr >> (8+2)), &pouthi, &tmp_a);
      p7550->pStatus->ISDBT_A_SNRData = BTHD_7550_P_2560log10(tmp_a) - 6165;  /* 10*log10(1/256.) = 6165 */


    }

    /* Layer B SNR */
    eq_snr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNR_B);         /* error power read in 3.29 format */
    if (eq_snr >> (8+2)) {
      encoded_qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERB_QAM);   
      scale       = scale_table[encoded_qam];
      sft_scale   = sft_scale_table[encoded_qam][0];
      for (i=0, j=1; i<(scale-2); i++)
        j = j * 2;
      BMTH_HILO_32TO64_Mul(sft_scale, j, &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (eq_snr >> (8+2)), &pouthi, &tmp_b);
      /*tmp_b       = (sft_scale << (scale-2))/(eq_snr >> (8+2));     */      /* 10*log10(tmp/256.0) to obtain value in dB */
      p7550->pStatus->ISDBT_B_SNRData = BTHD_7550_P_2560log10(tmp_b) - 6165;  /* 10*log10(1/256.) = 6165 */

    }

    /* Layer C SNR */
    eq_snr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNR_C);         /* error power read in 3.29 format */
    if (eq_snr >> (8+2)) {
      encoded_qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERC_QAM);   
      scale       = scale_table[encoded_qam];
      sft_scale   = sft_scale_table[encoded_qam][0];
      for (i=0, j=1; i<(scale-2); i++)
        j = j * 2;
      BMTH_HILO_32TO64_Mul(sft_scale, j, &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (eq_snr >> (8+2)), &pouthi, &tmp_c);
      /* tmp_c       = (sft_scale << (scale-2))/(eq_snr >> (8+2));     */      /* 10*log10(tmp/256.0) to obtain value in dB */
      p7550->pStatus->ISDBT_C_SNRData = BTHD_7550_P_2560log10(tmp_c) - 6165;  /* 10*log10(1/256.) = 6165 */   
    }

    /* Layer Weighted SNR */
    /*tmp = (tmp_a*BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_0, LAYERA_SEG)
    + tmp_b*BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERB_SEG)
    + tmp_c*BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERC_SEG)) / 13;*/

    BMTH_HILO_32TO64_Mul(tmp_a, BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_0, LAYERA_SEG), &pouthi, &poutlo);
    BMTH_HILO_32TO64_Mul(tmp_b, BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERB_SEG), &pouthi2, &poutlo2);
    BMTH_HILO_64TO64_Add(pouthi, poutlo, pouthi2, poutlo2, &pouthi3, &poutlo3);
    BMTH_HILO_32TO64_Mul(tmp_c, BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_LAYER_1, LAYERC_SEG), &pouthi, &poutlo);
    BMTH_HILO_64TO64_Add(pouthi, poutlo, pouthi3, poutlo3, &pouthi2, &poutlo2);
    BMTH_HILO_64TO64_Div32(pouthi2, poutlo2,  13, &pouthi, &poutlo);
    p7550->pStatus->SNRData = BTHD_7550_P_2560log10(poutlo) - 6165;
  }
  else {
    eq_snr = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNR);
    if (eq_snr >> (8+2)) {
      encoded_qam = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS, QAM);
      /* We have see encoded QAM have invalid value '3'.*/
      encoded_alpha = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS, HIERARCHY);

      scale     = scale_table[encoded_qam];
      sft_scale = sft_scale_table[encoded_qam][encoded_alpha];

      /* Take 10*log10(value2/256.0) to obtain value in dB */
      tmp = (sft_scale << (scale-2))/(eq_snr >> (8+2));
      /* 256*10*log10(1/256) = -6165, application then takes SNR_p/256 for SNR in dB */  
      p7550->pStatus->SNRData = BTHD_7550_P_2560log10(tmp) - 6165;
      p7550->pStatus->SNRPilot += 70;  /*+(0.274)*256. Adjustment for the fact the pilot is 2.5 dB higher than signal  */
    }
  }

  /* Compute pilot SNR */
  p7550->pStatus->SNRPilot = 0;
  eq_snrp = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_EQ_SNRP);
  if (eq_snrp) {
    /* ce_beta has 16 possible values.*/
    static uint16_t snr_factor_table[][16] = {
      { /* Guard interval: 1/32 */
        309, /*  0, snr_factor = 10^(-5.2/10) * 1024 */
          347, /*  1, snr_factor = 10^(-4.7/10) * 1024 */
          372, /*  2, snr_factor = 10^(-4.4/10) * 1024 */
          389, /*  3, snr_factor = 10^(-4.2/10) * 1024 */
          398, /*  4, snr_factor = 10^(-4.1/10) * 1024 */   
          398, /*  5, snr_factor = 10^(-4.1/10) * 1024 */       
          398, /*  6, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /*  7, snr_factor = 10^(-4.1/10) * 1024 */      
          282, /*  8, snr_factor = 10^(-5.6/10) * 1024 */
          398, /*  9, snr_factor = 10^(-4.1/10) * 1024 */  
          398, /* 10, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 11, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 12, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 13, snr_factor = 10^(-4.1/10) * 1024 */  
          398, /* 14, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 15, snr_factor = 10^(-4.1/10) * 1024 */      
      },
      { /* Guard interval: 1/16 */
        309, /*  0, snr_factor = 10^(-5.2/10) * 1024 */
          347, /*  1, snr_factor = 10^(-4.7/10) * 1024 */
          372, /*  2, snr_factor = 10^(-4.4/10) * 1024 */
          389, /*  3, snr_factor = 10^(-4.2/10) * 1024 */
          398, /*  4, snr_factor = 10^(-4.1/10) * 1024 */
          398, /*  5, snr_factor = 10^(-4.1/10) * 1024 */       
          398, /*  6, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /*  7, snr_factor = 10^(-4.1/10) * 1024 */      
          282, /*  8, snr_factor = 10^(-5.6/10) * 1024 */
          398, /*  9, snr_factor = 10^(-4.1/10) * 1024 */
          398, /* 10, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 11, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 12, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 13, snr_factor = 10^(-4.1/10) * 1024 */
          398, /* 14, snr_factor = 10^(-4.1/10) * 1024 */      
          398, /* 15, snr_factor = 10^(-4.1/10) * 1024 */      
        },
        { /* Guard interval: 1/8 */
          246, /*  0, snr_factor = 10^(-6.2/10) * 1024 */
            302, /*  1, snr_factor = 10^(-5.3/10) * 1024 */
            347, /*  2, snr_factor = 10^(-4.7/10) * 1024 */
            372, /*  3, snr_factor = 10^(-4.4/10) * 1024 */
            389, /*  4, snr_factor = 10^(-4.2/10) * 1024 */
            389, /*  5, snr_factor = 10^(-4.2/10) * 1024 */
            389, /*  6, snr_factor = 10^(-4.2/10) * 1024 */
            389, /*  7, snr_factor = 10^(-4.2/10) * 1024 */
            209, /*  8, snr_factor = 10^(-6.9/10) * 1024 */
            389, /*  9, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 10, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 11, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 12, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 13, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 14, snr_factor = 10^(-4.2/10) * 1024 */
            389, /* 15, snr_factor = 10^(-4.2/10) * 1024 */
        },
        { /* Guard interval: 1/4 */
          152, /*  0, snr_factor = 10^(-8.3/10) * 1024 */
            235, /*  1, snr_factor = 10^(-6.4/10) * 1024 */
            309, /*  2, snr_factor = 10^(-5.2/10) * 1024 */
            355, /*  3, snr_factor = 10^(-4.6/10) * 1024 */
            380, /*  4, snr_factor = 10^(-4.3/10) * 1024 */
            380, /*  5, snr_factor = 10^(-4.3/10) * 1024 */
            380, /*  6, snr_factor = 10^(-4.3/10) * 1024 */
            380, /*  7, snr_factor = 10^(-4.3/10) * 1024 */
            120, /*  8, snr_factor = 10^(-9.3/10) * 1024 */
            380, /*  9, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 10, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 11, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 12, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 13, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 14, snr_factor = 10^(-4.3/10) * 1024 */
            380, /* 15, snr_factor = 10^(-4.3/10) * 1024 */
          }
    };

    encoded_guard_interval = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS, GUARD);

    ce_beta = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP) << 3 | BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_CE, AVG_FF);

    /* Extract the snr_factor from the table.*/
    snr_factor = snr_factor_table[encoded_guard_interval][ce_beta];

    if (eq_snrp >> 8) {
      /* Take 10*log10(value2/256.0) to obtain value in dB*/
      tmp = ((1U << 29) / (eq_snrp >> 8) * snr_factor) >> 10;
      /* 256*10*log10(1/256) = -6165, application then takes SNR_p/256 for SNR in dB*/
      p7550->pStatus->SNRPilot = BTHD_7550_P_2560log10(tmp) - 6165;
      /*p7550->pStatus->SNRPilot += 3*256;  // Add 3dB to SNR because DAGC2 back-off has been adjust from 20dB to 17dB  */
      p7550->pStatus->SNRPilot -= 569;  /* -(2.5 - 0.274)*256. Adjustment for the fact the pilot is 2.5 dB higher than signal  */
    }
  }

  /*  Compute final SNR. */
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {                                 /* Customer request for data snr instead of pilot snr */
    snr_curr = p7550->pStatus->SNRData;
  }
  else {
    if ((p7550->pAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Mobile) &&!(BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_ICI, ICI_ITER)))
      snr_curr = p7550->pStatus->SNRData;
    else
      snr_curr = p7550->pStatus->SNRPilot;
  }
  p7550->pStatus->SNR = snr_curr;

  /* update status Lock */
  p7550->pStatus->Lock = ((p7550->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1);

  /* Compute pre-Viterbi BER */
  value  = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_VIT_RCNT );
  value2 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_VIT_RERR );
  if (value2)
  {
    BMTH_HILO_64TO64_Mul(0, value, 0, 0x80000000, &pouthi, &poutlo);
    BMTH_HILO_64TO64_Div32(pouthi, poutlo, value2, &utmp, &poutlo2);
    p7550->pStatus->PreVitBER = utmp; 
    /*
    BDBG_MSG((" PreVitBER 0x%08x  ", p7550->pStatus->PreVitBER));
    p7550->pStatus->PreVitBER = (((value * 2147483648) / value2) >> 16);  */          /* Application takes x/(2^31) to obtain error rate */
  }

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* Read hardware error counters */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, CAPERC, 1 ); 

    NFERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NFERC );
    UFERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_UFERC );
    TFERC = NFERC + UFERC; 

    CERC  = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CERC );
    NBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC );
    CBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC );
    UBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC );
    TBERC = NBERC + CBERC + UBERC;

    CERC_B  = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CERC_B );
    NBERC_B = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B );
    CBERC_B = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B );
    UBERC_B = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_B );
    TBERC_B = NBERC_B + CBERC_B + UBERC_B;

    CERC_C  = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CERC_C );
    NBERC_C = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C );
    CBERC_C = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C );
    UBERC_C = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_C );
    TBERC_C = NBERC_C + CBERC_C + UBERC_C;

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, CAPERC, 0 ); 

    /* Update software error counters */
    p7550->pStatus->TS_NFERC = p7550->pStatus->TS_NFERC_ref + NFERC;
    p7550->pStatus->TS_UFERC = p7550->pStatus->TS_UFERC_ref + UFERC;
    p7550->pStatus->TS_TFERC = p7550->pStatus->TS_TFERC_ref + TFERC;

    p7550->pStatus->ISDBT_A_TS_CERC  = p7550->pStatus->ISDBT_A_TS_CERC_ref  + CERC;
    p7550->pStatus->ISDBT_A_TS_NBERC = p7550->pStatus->ISDBT_A_TS_NBERC_ref + NBERC;
    p7550->pStatus->ISDBT_A_TS_CBERC = p7550->pStatus->ISDBT_A_TS_CBERC_ref + CBERC;
    p7550->pStatus->ISDBT_A_TS_UBERC = p7550->pStatus->ISDBT_A_TS_UBERC_ref + UBERC;
    p7550->pStatus->ISDBT_A_TS_TBERC = p7550->pStatus->ISDBT_A_TS_TBERC_ref + TBERC;

    p7550->pStatus->ISDBT_B_TS_CERC  = p7550->pStatus->ISDBT_B_TS_CERC_ref  + CERC_B;
    p7550->pStatus->ISDBT_B_TS_NBERC = p7550->pStatus->ISDBT_B_TS_NBERC_ref + NBERC_B;
    p7550->pStatus->ISDBT_B_TS_CBERC = p7550->pStatus->ISDBT_B_TS_CBERC_ref + CBERC_B;
    p7550->pStatus->ISDBT_B_TS_UBERC = p7550->pStatus->ISDBT_B_TS_UBERC_ref + UBERC_B;
    p7550->pStatus->ISDBT_B_TS_TBERC = p7550->pStatus->ISDBT_B_TS_TBERC_ref + TBERC_B;

    p7550->pStatus->ISDBT_C_TS_CERC  = p7550->pStatus->ISDBT_C_TS_CERC_ref  + CERC_C;
    p7550->pStatus->ISDBT_C_TS_NBERC = p7550->pStatus->ISDBT_C_TS_NBERC_ref + NBERC_C;
    p7550->pStatus->ISDBT_C_TS_CBERC = p7550->pStatus->ISDBT_C_TS_CBERC_ref + CBERC_C;
    p7550->pStatus->ISDBT_C_TS_UBERC = p7550->pStatus->ISDBT_C_TS_UBERC_ref + UBERC_C;
    p7550->pStatus->ISDBT_C_TS_TBERC = p7550->pStatus->ISDBT_C_TS_TBERC_ref + TBERC_C;

    /* Reset hardware error counters every 10 one-second frames */
    if (TFERC >= THD_StatusFramesForReset) {
      BTHD_7550_P_ResetStatusHW(h);

      p7550->pStatus->TS_NFERC_ref = p7550->pStatus->TS_NFERC;
      p7550->pStatus->TS_UFERC_ref = p7550->pStatus->TS_UFERC;
      p7550->pStatus->TS_TFERC_ref = p7550->pStatus->TS_TFERC;

      p7550->pStatus->ISDBT_A_TS_CERC_ref  = p7550->pStatus->ISDBT_A_TS_CERC;
      p7550->pStatus->ISDBT_A_TS_NBERC_ref = p7550->pStatus->ISDBT_A_TS_NBERC;
      p7550->pStatus->ISDBT_A_TS_CBERC_ref = p7550->pStatus->ISDBT_A_TS_CBERC;
      p7550->pStatus->ISDBT_A_TS_UBERC_ref = p7550->pStatus->ISDBT_A_TS_UBERC;
      p7550->pStatus->ISDBT_A_TS_TBERC_ref = p7550->pStatus->ISDBT_A_TS_TBERC;

      p7550->pStatus->ISDBT_B_TS_CERC_ref  = p7550->pStatus->ISDBT_B_TS_CERC;
      p7550->pStatus->ISDBT_B_TS_NBERC_ref = p7550->pStatus->ISDBT_B_TS_NBERC;
      p7550->pStatus->ISDBT_B_TS_CBERC_ref = p7550->pStatus->ISDBT_B_TS_CBERC;
      p7550->pStatus->ISDBT_B_TS_UBERC_ref = p7550->pStatus->ISDBT_B_TS_UBERC;
      p7550->pStatus->ISDBT_B_TS_TBERC_ref = p7550->pStatus->ISDBT_B_TS_TBERC;

      p7550->pStatus->ISDBT_C_TS_CERC_ref  = p7550->pStatus->ISDBT_C_TS_CERC;
      p7550->pStatus->ISDBT_C_TS_NBERC_ref = p7550->pStatus->ISDBT_C_TS_NBERC;
      p7550->pStatus->ISDBT_C_TS_CBERC_ref = p7550->pStatus->ISDBT_C_TS_CBERC;
      p7550->pStatus->ISDBT_C_TS_UBERC_ref = p7550->pStatus->ISDBT_C_TS_UBERC;
      p7550->pStatus->ISDBT_C_TS_TBERC_ref = p7550->pStatus->ISDBT_C_TS_TBERC;
    }

    /* Compute Viterbi BER and TS PER */
    if (p7550->pStatus->TS_TFERC)
    {
      BMTH_HILO_32TO64_Mul(100, p7550->pStatus->TS_UFERC, &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, p7550->pStatus->TS_TFERC, &utmp, &p7550->pStatus->TS_ESR );
      /*p7550->pStatus->TS_ESR = ((100ULL * p7550->pStatus->TS_UFERC) / p7550->pStatus->TS_TFERC);*/
    }
    if (p7550->pStatus->ISDBT_A_TS_TBERC) {
      /*p7550->pStatus->ISDBT_A_TS_PER = (p7550->pStatus->ISDBT_A_TS_UBERC * (1ULL<<31)) / p7550->pStatus->ISDBT_A_TS_TBERC; */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_A_TS_UBERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo,  p7550->pStatus->ISDBT_A_TS_TBERC, &utmp, &p7550->pStatus->ISDBT_A_TS_PER );
      /* p7550->pStatus->ISDBT_A_VitBER = (p7550->pStatus->ISDBT_A_TS_CERC * (1ULL<<31)) / (p7550->pStatus->ISDBT_A_TS_TBERC * 8 * 188ULL); */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_A_TS_CERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->ISDBT_A_TS_TBERC * 8 * 188), &utmp, &p7550->pStatus->ISDBT_A_VitBER );
    }
    if (p7550->pStatus->ISDBT_B_TS_TBERC) {
      /*p7550->pStatus->ISDBT_B_TS_PER = (p7550->pStatus->ISDBT_B_TS_UBERC * (1ULL<<31)) / p7550->pStatus->ISDBT_B_TS_TBERC; */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_B_TS_UBERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->ISDBT_B_TS_TBERC), &pouthi, &p7550->pStatus->ISDBT_B_TS_PER );
      /* p7550->pStatus->ISDBT_B_VitBER = (p7550->pStatus->ISDBT_B_TS_CERC * (1ULL<<31)) / (p7550->pStatus->ISDBT_B_TS_TBERC * 8 * 188ULL); */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_B_TS_CERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->ISDBT_B_TS_TBERC * 8 * 188), &pouthi, &p7550->pStatus->ISDBT_B_VitBER );
    }
    if (p7550->pStatus->ISDBT_C_TS_TBERC) {
      /* p7550->pStatus->ISDBT_C_TS_PER = (p7550->pStatus->ISDBT_C_TS_UBERC * (1ULL<<31)) / p7550->pStatus->ISDBT_C_TS_TBERC; */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_C_TS_UBERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->ISDBT_C_TS_TBERC), &pouthi, &p7550->pStatus->ISDBT_C_TS_PER );
      /*p7550->pStatus->ISDBT_C_VitBER = (p7550->pStatus->ISDBT_C_TS_CERC * (1ULL<<31)) / (p7550->pStatus->ISDBT_C_TS_TBERC * 8 * 188ULL); */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->ISDBT_C_TS_CERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->ISDBT_C_TS_TBERC * 8 * 188), &pouthi, &p7550->pStatus->ISDBT_C_VitBER );
    }
  }
  else {
    /* Read hardware error counters */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, CAPERC, 1 ); 

    CERC  = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CERC );
    NBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC );
    CBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC );
    UBERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC );
    NFERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NFERC );
    UFERC = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_UFERC );
    TBERC = NBERC + CBERC + UBERC;
    TFERC = NFERC + UFERC; 

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, CAPERC, 0 ); 

    /* Update software error counters */
    p7550->pStatus->TS_CERC  = p7550->pStatus->TS_CERC_ref  + CERC;
    p7550->pStatus->TS_NBERC = p7550->pStatus->TS_NBERC_ref + NBERC;
    p7550->pStatus->TS_CBERC = p7550->pStatus->TS_CBERC_ref + CBERC;
    p7550->pStatus->TS_UBERC = p7550->pStatus->TS_UBERC_ref + UBERC;
    p7550->pStatus->TS_TBERC = p7550->pStatus->TS_TBERC_ref + TBERC;
    p7550->pStatus->TS_NFERC = p7550->pStatus->TS_NFERC_ref + NFERC;
    p7550->pStatus->TS_UFERC = p7550->pStatus->TS_UFERC_ref + UFERC;
    p7550->pStatus->TS_TFERC = p7550->pStatus->TS_TFERC_ref + TFERC;

    /* Reset hardware error counters every 10 one-second frames */
    if (TFERC >= THD_StatusFramesForReset) {
      BTHD_7550_P_ResetStatusHW(h);

      p7550->pStatus->TS_CERC_ref  = p7550->pStatus->TS_CERC;
      p7550->pStatus->TS_NBERC_ref = p7550->pStatus->TS_NBERC;
      p7550->pStatus->TS_CBERC_ref = p7550->pStatus->TS_CBERC;
      p7550->pStatus->TS_UBERC_ref = p7550->pStatus->TS_UBERC;
      p7550->pStatus->TS_TBERC_ref = p7550->pStatus->TS_TBERC;
      p7550->pStatus->TS_NFERC_ref = p7550->pStatus->TS_NFERC;
      p7550->pStatus->TS_UFERC_ref = p7550->pStatus->TS_UFERC;
      p7550->pStatus->TS_TFERC_ref = p7550->pStatus->TS_TFERC;
    }  

    /* Compute Viterbi BER and TS PER */
    if (p7550->pStatus->TS_TBERC) {
      /* p7550->pStatus->TS_PER = (p7550->pStatus->TS_UBERC * (1ULL<<31)) / p7550->pStatus->TS_TBERC; */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->TS_UBERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->TS_TBERC), &pouthi, &p7550->pStatus->TS_PER );
      /* p7550->pStatus->VitBER = (p7550->pStatus->TS_CERC * (1ULL<<31)) / (p7550->pStatus->TS_TBERC * 8 * 188ULL); */
      BMTH_HILO_32TO64_Mul(0x80000000, (p7550->pStatus->TS_CERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->TS_TBERC * 8 * 188), &pouthi, &p7550->pStatus->VitBER );
    }
    if (p7550->pStatus->TS_TFERC)
    {
      /* p7550->pStatus->TS_ESR = ((100ULL * p7550->pStatus->TS_UFERC) / p7550->pStatus->TS_TFERC); */
      BMTH_HILO_32TO64_Mul(0x10000000, (p7550->pStatus->TS_UFERC), &pouthi, &poutlo);
      BMTH_HILO_64TO64_Div32(pouthi, poutlo, (p7550->pStatus->TS_TFERC), &pouthi, &p7550->pStatus->TS_ESR );
    }
  }	

  /* Bandwidth */
  p7550->pStatus->Bandwidth = p7550->pAcquireParam->Bandwidth;

#if 0   
  BDBG_MSG(("BTHD_7550_P_Status:\tBTHD_7550_P_2560log10 = %d",BTHD_7550_P_2560log10(100)));
  BDBG_MSG(("BTHD_7550_P_Status:\tLock             = 0x%08x",p7550->pStatus->Lock));
  BDBG_MSG(("BTHD_7550_P_Status:\tRsCorrectedBlocks= %d",p7550->pStatus->TS_CBERC));
  BDBG_MSG(("BTHD_7550_P_Status:\tUncorrectedBlocks= %d",p7550->pStatus->TS_UBERC));
  BDBG_MSG(("BTHD_7550_P_Status:\tRsCleanBlocks    = %d",p7550->pStatus->TS_NBERC));
  BDBG_MSG(("BTHD_7550_P_Status:\tViterbiBER       = %d",p7550->pStatus->VitBER));
  BDBG_MSG(("BTHD_7550_P_Status:\tTS_PER           = %d",p7550->pStatus->TS_PER));
  BDBG_MSG(("BTHD_7550_P_Status:\tTS_ESR           = %d",p7550->pStatus->TS_ESR));
  BDBG_MSG(("BTHD_7550_P_Status:\tTS_TFERC         = %d",p7550->pStatus->TS_TFERC));
  BDBG_MSG(("BTHD_7550_P_Status:\tPreVitBER        = %d",p7550->pStatus->PreVitBER));
  BDBG_MSG(("BTHD_7550_P_Status:\tTS_NBERC         = %d",p7550->pStatus->TS_NBERC)); 
  BDBG_MSG(("BTHD_7550_P_Status:\tPriority         = %d",p7550->pStatus->Priority));
  BDBG_MSG(("BTHD_7550_P_Status:\tTransmissionMode = %d",p7550->pStatus->TransmissionMode));
  BDBG_MSG(("BTHD_7550_P_Status:\tGuardInterval    = %d",p7550->pStatus->GuardInterval));
  BDBG_MSG(("BTHD_7550_P_Status:\tQam              = %d",p7550->pStatus->Qam));
  BDBG_MSG(("BTHD_7550_P_Status:\tHierarchy        = %d",p7550->pStatus->Hierarchy));
  BDBG_MSG(("BTHD_7550_P_Status:\tInterleaving     = %d",p7550->pStatus->Interleaving));
  BDBG_MSG(("BTHD_7550_P_Status:\tCodeRateHP       = %d",p7550->pStatus->CodeRateHP));
  BDBG_MSG(("BTHD_7550_P_Status:\tCodeRateLP       = %d",p7550->pStatus->CodeRateLP));
  BDBG_MSG(("BTHD_7550_P_Status:\tCarrierOffset    = %d",p7550->pStatus->CarrierOffset));
  BDBG_MSG(("BTHD_7550_P_Status:\tTimingOffset     = %d",p7550->pStatus->TimingOffset));
  BDBG_MSG(("BTHD_7550_P_Status:\tRagcGain         = %d",p7550->pStatus->RagcGain));
  BDBG_MSG(("BTHD_7550_P_Status:\tIagcGain         = %d",p7550->pStatus->IagcGain));
  BDBG_MSG(("BTHD_7550_P_Status:\tDagc1Gain        = %d",p7550->pStatus->Dagc1Gain));
  BDBG_MSG(("BTHD_7550_P_Status:\tDagc1Gain/256    = %f",p7550->pStatus->Dagc1Gain/256));
  BDBG_MSG(("BTHD_7550_P_Status:\tDagc2Gain        = %d",p7550->pStatus->Dagc2Gain));
  BDBG_MSG(("BTHD_7550_P_Status:\tSNR              = %d",p7550->pStatus->SNR));
  BDBG_MSG(("BTHD_7550_P_Status:\tSNRData          = %d",p7550->pStatus->SNRData));
  BDBG_MSG(("BTHD_7550_P_Status:\tSNRPilot         = %d",p7550->pStatus->SNRPilot));
  BDBG_MSG(("BTHD_7550_P_Status:\tReacquireCount   = %d",p7550->pStatus->ReacquireCount));
#endif
}

/***************************************************************************
* BTHD_7550_P_Lock()
***************************************************************************/
BTHD_RESULT BTHD_7550_P_Lock( BTHD_Handle h, int init_vars)
{
  uint32_t  tps_lock,vit_lock,fec_lock;
  uint32_t  segs_a=13, segs_b, segs_c;
  uint32_t  clean=0, cberc=0, uberc=0, total=0;
  uint32_t  clean_b=0, cberc_b=0, uberc_b=0;
  uint32_t  clean_c=0, cberc_c=0, uberc_c=0;
  uint32_t  N;
  uint32_t  fallback;
  bool      set_lock=0, clr_lock=0, set_lock_b=0, clr_lock_b=0, set_lock_c=0, clr_lock_c=0;

  static uint32_t prev_clean  =0, prev_cberc  =0, prev_uberc  =0, prev_total=0;
  static uint32_t prev_clean_b=0, prev_cberc_b=0, prev_uberc_b=0;
  static uint32_t prev_clean_c=0, prev_cberc_c=0, prev_uberc_c=0;
  static uint32_t lock_a=1, lock_b=1, lock_c=1, lock=1;
  static uint32_t DisableLockUpdate;
  volatile uint32_t value;

  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  /*** Collect FEC Lock Status ***/

  clean       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC);
  cberc       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC);
  uberc       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC);
  clean_b     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B);
  cberc_b     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B);
  uberc_b     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_B);
  clean_c     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C);
  cberc_c     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C);
  uberc_c     = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_C);
  total       = clean + cberc + uberc + clean_b + cberc_b + uberc_b + clean_c + cberc_c + uberc_c;


  /*** Reset internal variables ***/
  if (init_vars) {
    prev_clean        = 0;
    prev_cberc        = 0;
    prev_uberc        = 0;
    prev_total        = 0;
    prev_clean_b      = 0;
    prev_cberc_b      = 0;
    prev_uberc_b      = 0;
    prev_clean_c      = 0;
    prev_cberc_c      = 0;
    prev_uberc_c      = 0;
    lock_a            = clean || !uberc;
    lock_b            = clean_b || !uberc_b;
    lock_c            = clean_c || !uberc_c;
    DisableLockUpdate = 0;
  }


  value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_STATUS );  
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* ISDB-T */
    tps_lock = (value >> 6)  & 1;
    if (tps_lock) {
      /* If TMCC lock, automatically set Viterbi and FEC lock otherwise do not */
      vit_lock = 1;
      fec_lock = 1;
    } else {
      vit_lock = 0;
      fec_lock = 0;
    }
  } else {
    /* DVB-T */
    tps_lock = (value >> 3)  & 1;
    vit_lock = (value >> 9)  & 1;
    fec_lock = (value >> 11) & 1;
  }

  p7550->ThdLockStatus = (tps_lock << THD_LockStatusBit_TPSLock) | (((p7550->ThdLockStatus >> THD_LockStatusBit_NoDVBTSignal) & 1) << THD_LockStatusBit_NoDVBTSignal);
  p7550->ThdLockStatus|=  vit_lock << THD_LockStatusBit_VitLock;
  p7550->ThdLockStatus|=  fec_lock << THD_LockStatusBit_FECLock;

  /*  For ISDB-T, determine which segments are active */
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    segs_a = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_SEG);
    segs_b = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG);
    segs_c = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG);
  }
  else {
    segs_a = 13;
    segs_b = 0;
    segs_c = 0;
  }

  /*  FEC Lock Detectors 
  Lock status for a layer is updated when a clean packet is found or when 300 packets are found. */

  /*** If counters have been reset, the previous counters are not reliable.  Reset the previous counters. ***/

  if (total < prev_total) {
    prev_clean    = clean;
    prev_cberc    = cberc;
    prev_uberc    = uberc;
    prev_clean_b  = clean_b;
    prev_cberc_b  = cberc_b;
    prev_uberc_b  = uberc_b;
    prev_clean_c  = clean_c;
    prev_cberc_c  = cberc_c;
    prev_uberc_c  = uberc_c;
  }


  /*** Lock detectors ***/

  set_lock  = (clean > prev_clean);
  clr_lock  = (uberc > prev_uberc) && (clean + cberc + uberc - (prev_clean + prev_cberc + prev_uberc) > segs_a*230);
  lock_a    = set_lock ? 1 : (clr_lock ? 0 : lock_a);

  if (segs_b) {
    set_lock_b  = (clean_b > prev_clean_b);
    clr_lock_b  = (uberc_b > prev_uberc_b) && (clean_b + cberc_b + uberc_b - (prev_clean_b + prev_cberc_b + prev_uberc_b) > segs_b*230);
    lock_b      = set_lock_b ? 1 : (clr_lock_b ? 0 : lock_b);
  }

  if (segs_c) {
    set_lock_c  = (clean_c > prev_clean_c);
    clr_lock_c  = (uberc_c > prev_uberc_c) && (clean_c + cberc_c + uberc_c - (prev_clean_c + prev_cberc_c + prev_uberc_c) > segs_c*230);
    lock_c      = set_lock_c ? 1 : (clr_lock_c ? 0 : lock_c);
  }


  /*** Test For Pedestrian Fallback ***/

  fallback = 0;
  if (clr_lock || clr_lock_b || clr_lock_c)
    fallback = 1;
  if ((prev_total > 6000) && ((uberc > prev_uberc) || (uberc_b > prev_uberc_b) || (uberc_c > prev_uberc_c)))
    fallback = 1;      
  if (!DisableLockUpdate && fallback && (p7550->pStatus->ChannelEstimatorMode == THD_ChannelEstimatorMode_Fixed) 
    && (p7550->pAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Auto) && !p7550->pStatus->CoChannelPresent) {
      BDBG_MSG(("BTHD_7550_P_Lock:\tUncorrected errors detected.  Fallback to Pedestrian mode."));
      p7550->pStatus->ChannelEstimatorMode = THD_ChannelEstimatorMode_Pedestrian;
      BTHD_7550_P_SetCE(h, THD_ChannelEstimatorMode_Pedestrian);
      if (p7550->pAcquireParam->FFTWindowMode == THD_FFTWindowMode_InSpan) {
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000208 );                      /*  FFT window leak to timing loop enabled w/limiting*/
        N = p7550->pStatus->TransmissionMode;
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00020000 * (N/2048));            /*  Ki=2^(-14)*N/2048,Kl=2^0 (better for echo tests inside/outside guard interval)*/
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, PHASE_FREQ_SELECT, 0 );           /*  Timing FD enable */ 
      }
      DisableLockUpdate = 10;
  }

  /*** On tuning, first try to lock all layers.  Then accept whatever will lock. ***/

  if (!DisableLockUpdate) {
    if (((p7550->pStatus->ReacquireCount-1) % 2) == 0) /* || (p7550->pStatus->FFTWindowMode != THD_FFTWindowMode_InSpan))*/
      /* Lock all layers */
      lock  = tps_lock && vit_lock && fec_lock && lock_a && (!segs_b || lock_b) && (!segs_c || lock_c);
    else
      /* Lock more than one segment */
      lock  = tps_lock && vit_lock && fec_lock && (lock_a*segs_a + lock_b*segs_b + lock_c*segs_c > 1);
  }
  else 
    DisableLockUpdate--;


  /*
  BDBG_MSG(("BTHD_7550_P_Lock:"));
  BDBG_MSG(("BTHD_7550_P_Lock:\tlock=%d, tps_lock=%d, lock_(a,b,c)=(%d,%d,%d), total=%d", lock, tps_lock, lock_a, lock_b, lock_c, total-prev_total)); 
  BDBG_MSG(("BTHD_7550_P_Lock:\tsegs_a=%3d, clean=%6d,\tcorr=%6d,\terror=%6d", segs_a, clean-prev_clean,     cberc-prev_cberc,     uberc-prev_uberc));
  BDBG_MSG(("BTHD_7550_P_Lock:\tsegs_b=%3d, clean=%6d,\tcorr=%6d,\terror=%6d", segs_b, clean_b-prev_clean_b, cberc_b-prev_cberc_b, uberc_b-prev_uberc_b));
  BDBG_MSG(("BTHD_7550_P_Lock:\tsegs_c=%3d, clean=%6d,\tcorr=%6d,\terror=%6d", segs_c, clean_c-prev_clean_c, cberc_c-prev_cberc_c, uberc_c-prev_uberc_c));
  */

  if (set_lock || clr_lock) {
    prev_clean = clean;
    prev_cberc = cberc;
    prev_uberc = uberc;
  }
  if (set_lock_b || clr_lock_b) {
    prev_clean_b = clean_b;
    prev_cberc_b = cberc_b;
    prev_uberc_b = uberc_b;
  }
  if (set_lock_c || clr_lock_c) {
    prev_clean_c = clean_c;
    prev_cberc_c = cberc_c;
    prev_uberc_c = uberc_c;
  }


  /*** Check for FEC counters stuck ***/

  if (!init_vars && (total == prev_total))
    BDBG_MSG(("BTHD_7550_P_Lock:\tNo FEC data observed during lock check interval."));
  prev_total = total;

  p7550->ThdLockStatus |= lock << THD_LockStatusBit_SystemLock;

  return(p7550->ThdLockStatus);    
}


BTHD_RESULT BTHD_7550_P_LockOld( BTHD_Handle h)
{
  uint32_t tps_lock,vit_lock,fec_lock, retries=0;
  uint32_t clean=0,cleanInit=0,cleanDiff=0,total=0,totalInit,lock;
  uint32_t clean_b=0, clean_c=0, cleanInit_b=0, cleanInit_c=0, segs_b=0, segs_c=0, segs_a=0;
  uint32_t cberc=0, cberc_b=0, cberc_c=0, uberc=0, uberc_b=0, uberc_c=0;
  uint32_t cbercInit=0, cbercInit_b=0, cbercInit_c=0, ubercInit=0, ubercInit_b=0, ubercInit_c=0;
  uint32_t fscnt;
  uint32_t cleanDiff_a, cleanDiff_b, cleanDiff_c;
  uint32_t fscntInit;
  bool done;
  volatile uint32_t value;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_STATUS );  
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    /* ISDB-T */
    tps_lock = (value >> 6)  & 1;
    if (tps_lock) {
      /* If TMCC lock, automatically set Viterbi and FEC lock otherwise do not */
      vit_lock = 1;
      fec_lock = 1;
    } else {
      vit_lock = 0;
      fec_lock = 0;
    }
  } else {
    /* DVB-T */
    tps_lock = (value >> 3)  & 1;
    vit_lock = (value >> 9)  & 1;
    fec_lock = (value >> 11) & 1;
  }

  p7550->ThdLockStatus = (tps_lock << THD_LockStatusBit_TPSLock)  | (((p7550->ThdLockStatus >> THD_LockStatusBit_NoDVBTSignal) & 1) << THD_LockStatusBit_NoDVBTSignal);
  p7550->ThdLockStatus|=  vit_lock << THD_LockStatusBit_VitLock;
  p7550->ThdLockStatus|=  fec_lock << THD_LockStatusBit_FECLock;

  /*  For ISDB-T, determine which segments are active */
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    segs_a = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_0, LAYERA_SEG);
    segs_b = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG);
    segs_c = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG);
  }
  else {
    segs_b = 0;
    segs_c = 0;
  }

  /*  Try 3 times to achieve 1 clean packet in 100 */
  while (retries < 3) {
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC, CAPERC, 1 ); 
    cleanInit   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC);                     /*  Initial clean packet count*/
    cleanInit_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B);                   /*  Initial clean packet count*/
    cleanInit_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C);                   /*  Initial clean packet count*/
    cbercInit   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC);
    cbercInit_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B);
    cbercInit_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C);
    ubercInit   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC);
    ubercInit_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_B);
    ubercInit_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_C);
    totalInit   = cleanInit + cleanInit_b + cleanInit_c + cbercInit + cbercInit_b + cbercInit_c + ubercInit + ubercInit_b + ubercInit_c;
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FEC,CAPERC, 0 ); 

    total = 0;    /* Total packet count*/
    done = false; /* Done indicator*/
    fscntInit = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);                           /*  Use FSCNT as timer*/
    while ((total < 100) && !done) { /*  Wait for 100 packets */
      fscnt   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);
      clean   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC);
      clean_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B);
      clean_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C);
      cberc   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC);
      cberc_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B);
      cberc_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C);
      uberc   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC);
      uberc_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_B);
      uberc_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_C);
      total   = clean + clean_b + clean_c + cberc + cberc_b + cberc_c + uberc + uberc_b + uberc_c - totalInit; 

      cleanDiff   = (clean - cleanInit > 0) && (!segs_b || (clean_b - cleanInit_b > 0)) && (!segs_c || (clean_c - cleanInit_c > 0)); 
      if ((cleanDiff > 0) || ((fscntInit - fscnt) > 1350000))                                /*  Check for 1 clean packet or 25ms timeout */
        done = true;
    }

    /*** Prevent zero packet counts from one layer triggering an out-of-lock condition ***/
    cleanDiff_a = (clean - cleanInit > 0) || (cberc - cbercInit > 0) || (clean + cberc + uberc == cleanInit + cbercInit + ubercInit);
    cleanDiff_b = segs_b ? (clean_b - cleanInit_b > 0) || (clean_b + cberc_b + uberc_b == cleanInit_b + cbercInit_b + ubercInit_b) : 1;
    cleanDiff_c = segs_c ? (clean_c - cleanInit_c > 0) || (clean_c + cberc_c + uberc_c == cleanInit_c + cbercInit_c + ubercInit_c) : 1;     
    cleanDiff = cleanDiff_a && cleanDiff_b && cleanDiff_c && (total > 0);

    if (cleanDiff)
      retries = 3;
    else
      retries++;

    /* BDBG_MSG(("DEBUG -- total:%d\tA:%d\tB:%d\tC:%d", total, clean-cleanInit, clean_b-cleanInit_b, clean_c-cleanInit_c)); */
  }

  lock = tps_lock && vit_lock && fec_lock && ( cleanDiff > 0 );

  BDBG_MSG(("######## lock=%d, tps_lock=%d, cleanDiff=%d, total=%d!!!! ########", lock, tps_lock, cleanDiff, total)); 
  BDBG_MSG(("######## segs_a=%2d, clean=%d, corr=%d, error=%d ########", segs_a, clean-cleanInit, cberc-cbercInit, uberc-ubercInit));
  BDBG_MSG(("######## segs_b=%2d, clean=%d, corr=%d, error=%d ########", segs_b, clean_b-cleanInit_b, cberc_b-cbercInit_b, uberc_b-ubercInit_b));
  BDBG_MSG(("######## segs_c=%2d, clean=%d, corr=%d, error=%d ########", segs_c, clean_c-cleanInit_c, cberc_c-cbercInit_c, uberc_c-ubercInit_c));


  p7550->ThdLockStatus |= lock << THD_LockStatusBit_SystemLock;

  /* BDBG_MSG(("BTHD_7550_P_Lock:\t\tLock = %02x (%d,%d,%d)", lock,tps_lock,vit_lock,fec_lock)); */
  /* BDBG_MSG(("BTHD_7550_P_Lock:\t\tcleanDiff = %0x", cleanDiff)); */
  /* BDBG_MSG(("BTHD_7550_P_Lock:\t\treturn_val = %0x", p7550->ThdLockStatus)); */

  return(p7550->ThdLockStatus);    
}

/***************************************************************************
* BTHD_7550_P_ResetAll()
***************************************************************************/
void BTHD_7550_P_ResetAll(BTHD_Handle h)
{
  uint32_t value_thd, value_mem;
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  value_mem = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RESERVED2);           /* Preserve register holding BBS address pointer for isdbt memory */ 
  value_thd = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RESERVED1);           /* Preserve register holding BBS address pointer for THD params */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST ,0xffffffff );               /* Reset datapath and register interface */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST2,0x000001ff );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST ,0x007a1008 );               /* Hold RS_ERC,RS,FEC,BCH_ERC,TPS,FW,FE FIFO in reset */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST2,0x00000061 );               /* Hold TMCC_ERC,TMCC,NSE_AVG in reset */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RESERVED1, value_thd);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RESERVED2, value_mem);
  /* Clear all MIPS interrupt status */
  BINT_ClearCallback(p7550->hFwCorrMaxCallback);
  BINT_ClearCallback(p7550->hFwSyncCallback);
  BINT_ClearCallback(p7550->hSpSyncCallback);
  BINT_ClearCallback(p7550->hTpsSyncCallback);
  BINT_ClearCallback(p7550->hFecSyncCallback);
}

/***************************************************************************
* BTHD_7550_P_ResetAcquire()
***************************************************************************/
void BTHD_7550_P_ResetAcquire(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST ,0xfffffffe );               /* Reset datapath */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST2,0x000001ff );
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FRZ,0xffffffff );                /* Freeze all */

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST ,0x007a1008 );               /* Hold RS_ERC,RS,FEC,BCH_ERC,TPS,FW,FE FIFO in reset */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RST2,0x00000061 );               /* Hold TMCC_ERC,TMCC,NSE_AVG in reset */
  /* Clear all MIPS interrupt status */
  BINT_ClearCallback(p7550->hFwCorrMaxCallback);
  BINT_ClearCallback(p7550->hFwSyncCallback);
  BINT_ClearCallback(p7550->hSpSyncCallback);
  BINT_ClearCallback(p7550->hTpsSyncCallback);
  BINT_ClearCallback(p7550->hFecSyncCallback);
}

/***************************************************************************
* BTHD_7550_P_ResetStatus()
***************************************************************************/
BERR_Code BTHD_7550_P_ResetStatus(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
    p7550->pStatus->TS_UFERC              = 0;
    p7550->pStatus->TS_NFERC              = 0;
    p7550->pStatus->TS_TFERC              = 0;

    p7550->pStatus->ISDBT_A_TS_CERC       = 0;
    p7550->pStatus->ISDBT_A_TS_CBERC      = 0;
    p7550->pStatus->ISDBT_A_TS_UBERC      = 0;
    p7550->pStatus->ISDBT_A_TS_NBERC      = 0;
    p7550->pStatus->ISDBT_A_TS_TBERC      = 0;

    p7550->pStatus->ISDBT_B_TS_CERC       = 0;
    p7550->pStatus->ISDBT_B_TS_CBERC      = 0;
    p7550->pStatus->ISDBT_B_TS_UBERC      = 0;
    p7550->pStatus->ISDBT_B_TS_NBERC      = 0;
    p7550->pStatus->ISDBT_B_TS_TBERC      = 0;

    p7550->pStatus->ISDBT_C_TS_CERC       = 0;
    p7550->pStatus->ISDBT_C_TS_CBERC      = 0;
    p7550->pStatus->ISDBT_C_TS_UBERC      = 0;
    p7550->pStatus->ISDBT_C_TS_NBERC      = 0;
    p7550->pStatus->ISDBT_C_TS_TBERC      = 0;

    p7550->pStatus->TS_UFERC_ref          = 0;
    p7550->pStatus->TS_NFERC_ref          = 0;
    p7550->pStatus->TS_TFERC_ref          = 0;

    p7550->pStatus->ISDBT_A_TS_CERC_ref   = 0;
    p7550->pStatus->ISDBT_A_TS_CBERC_ref  = 0;
    p7550->pStatus->ISDBT_A_TS_UBERC_ref  = 0;
    p7550->pStatus->ISDBT_A_TS_NBERC_ref  = 0;
    p7550->pStatus->ISDBT_A_TS_TBERC_ref  = 0;

    p7550->pStatus->ISDBT_B_TS_CERC_ref   = 0;
    p7550->pStatus->ISDBT_B_TS_CBERC_ref  = 0;
    p7550->pStatus->ISDBT_B_TS_UBERC_ref  = 0;
    p7550->pStatus->ISDBT_B_TS_NBERC_ref  = 0;
    p7550->pStatus->ISDBT_B_TS_TBERC_ref  = 0;

    p7550->pStatus->ISDBT_C_TS_CERC_ref   = 0;
    p7550->pStatus->ISDBT_C_TS_CBERC_ref  = 0;
    p7550->pStatus->ISDBT_C_TS_UBERC_ref  = 0;
    p7550->pStatus->ISDBT_C_TS_NBERC_ref  = 0;
    p7550->pStatus->ISDBT_C_TS_TBERC_ref  = 0;

    p7550->pStatus->TS_ESR                = 0;

    p7550->pStatus->ISDBT_A_VitBER        = 0;
    p7550->pStatus->ISDBT_A_TS_PER        = 0;

    p7550->pStatus->ISDBT_B_VitBER        = 0;
    p7550->pStatus->ISDBT_B_TS_PER        = 0;

    p7550->pStatus->ISDBT_C_VitBER        = 0;
    p7550->pStatus->ISDBT_C_TS_PER        = 0; 

/*    p7550->pStatus->Lock				  = 0; */
  }
  else {
    p7550->pStatus->TS_CERC      = 0;
    p7550->pStatus->TS_CBERC     = 0;
    p7550->pStatus->TS_UBERC     = 0;  
    p7550->pStatus->TS_NBERC     = 0;
    p7550->pStatus->TS_TBERC     = 0;
    p7550->pStatus->TS_UFERC     = 0;  
    p7550->pStatus->TS_NFERC     = 0;
    p7550->pStatus->TS_TFERC     = 0;

    p7550->pStatus->TS_CERC_ref  = 0;
    p7550->pStatus->TS_CBERC_ref = 0;
    p7550->pStatus->TS_UBERC_ref = 0;  
    p7550->pStatus->TS_NBERC_ref = 0;
    p7550->pStatus->TS_TBERC_ref = 0;
    p7550->pStatus->TS_UFERC_ref = 0;  
    p7550->pStatus->TS_NFERC_ref = 0;
    p7550->pStatus->TS_TFERC_ref = 0;

    p7550->pStatus->PreVitBER    = 0;
    p7550->pStatus->VitBER       = 0;
    p7550->pStatus->TS_PER       = 0;
    p7550->pStatus->TS_ESR       = 0;

    /*p7550->pStatus->Lock		 = 0;*/ 
  }
  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_P_ResetStatusHW()
***************************************************************************/
void BTHD_7550_P_ResetStatusHW(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 1 );                    /* Reset FEC error counters */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 0 );
}


/***************************************************************************
* BTHD_7550_P_Config()
***************************************************************************/
void BTHD_7550_P_Config(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  /*************************************************************************
  * Configuration
  *************************************************************************/
  BTHD_7550_P_SetFrontEnd(h);                                                 /* Setup frontend */
  BTHD_7550_P_SetAnalogAGC(h);                                                /* Setup analog AGC */
  BTHD_7550_P_SetFrequency(h);                                                /* Setup frequencies (channel bandwidth,carrier frequency) */
  /* BDBG_MSG(("BTHD_7550_P_Config:\tSetup LPF1 and TI coefficients")); */
  BTHD_7550_P_WriteLpf1Coef(h);                                               /* Setup LPF1 coefficients */
  BTHD_7550_P_WriteTICoef(h);                                                 /* Setup time interpolator coefficients */

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC1, THRESH, 0x00a3d );                /* DAGC1 threshold=20dB (uint32_t)(pow(10.0,-20/10)* 262144) */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, THRESH, 0x00a3d );                /* DAGC2 threshold=20dB (uint32_t)(pow(10.0,-20/10)* 262144) */

  /* BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL,0x00001600 ); */           /* Carrier loop negate sense (when spinv=1), int_wait=6 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL,0x00000600 );                 /* Carrier loop int_wait=6 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW,0x00212050 );                 /* FFT window postcursor bias,enable fft,sync_symbols=32,beta=2^(-5),slip_limit=0 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_SEARCH,0x01802008 );          /* FFT window min_scale=1.5,out-of-span pre or in-span echoes,1st peak mode,L=8 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_OFFSET,0x00000010 );          /* FFT window offset=16 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000200 );            /* FFT window leak to timing loop disabled, disallow zeros */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_SPAN,0x00000200 );            /* FFT window span scale = 0.25 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_EQ_SNR_CTL,0x00000003 );         /* EQ SNR estimator beta=2^(-12) */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_BER_SYNC,0x004001ff );           /* BERT sync thresholds */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TPS_SYNC,0x00000076 );           /* TPS sync thresholds */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_OI,0x00000003);                  /* OI burst mode */

  /* BDBG_MSG(("BTHD_7550_P_Config:\tSetup FI coefficients")); */
  BTHD_7550_P_WriteFICoef(h,THD_FrequencyInterpolatorMode_InSpan);            /* Setup frequency interpolator coefficients */
}

/***************************************************************************
* BTHD_7550_P_Acquire() (rom + 2930)
***************************************************************************/
BTHD_RESULT BTHD_7550_P_Acquire(BTHD_Handle h)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);

  uint32_t value, data8=0;
  uint32_t N=2048;
  uint32_t fscntInit=0xffffffff;
  uint32_t ice,cl_int,cl_int_ice;
  uint32_t lock,retries,FFTWindowRetries;
  uint32_t ce_beta, aagc_K ;
  uint32_t clean = 0,  fscntInit2;
  uint32_t clean_b, clean_c, segs_b=0, segs_c=0;
  uint32_t cberc, cberc_b, cberc_c;
  uint32_t uberc, uberc_b, uberc_c;
  uint32_t total, total_b, total_c;
  BTHD_RESULT return_val = THD_AcquireResult_InitLockState ;
  char     *acqProfileString[16];
  uint32_t acqProfile[16];
  int      kk;
  uint32_t timeout, limit;
  uint32_t notchwidth;

  bool CoChannelPresent=false;
  THD_TransGuardResult_t TransGuardResult;
  THD_ChannelEstimatorMode_t ChannelEstimatorMode;
  THD_FFTWindowMode_t FFTWindowMode;
  THD_TransmissionMode_t TransmissionMode = p7550->pAcquireParam->TransmissionMode;
  THD_GuardInterval_t GuardInterval;

  /*
  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT)
  BDBG_MSG(("BTHD_7550_P_Acquire:\tISDB-T"));
  else
  BDBG_MSG(("BTHD_7550_P_Acquire:\tDVB-T"));
  */

  /*************************************************************************
  * Configuration
  *************************************************************************/
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FSCNT,fscntInit);                 /* Acquisition time profiling */

  if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT)
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, STD, 2);                           /* ISDBT mode */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, BE_MODE, 0 );                        /* BE=disabled */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, ICE_MODE, 1 );                       /* ICE=acquisition */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FE_FIFO_RST, 0 );                    /* Relese FE FIFO reset */

  value = ((uint32_t)p7550->pAcquireParam->GuardInterval << 20) | ((uint32_t)p7550->pAcquireParam->TransmissionMode << 24) | (1U << 31);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TPS_OV, value );
  BTHD_7550_P_SetNotch(h);                                                     /* Setup notch filter */

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC1, KL, 0x4 );                         /* DAGC1 K=2(-10) */    
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, KL, 0x6 );                         /* DAGC2 K=2(-12) */

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_AAGC_COEF,0x00001014 );                        /* Analog AGCs Kif=2^(-16),Krf=-2^(-20) */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW,0x00212051 );                               /* FFT window postcursor bias,enable fft,sync_symbols=32,beta=2^(-5),slip_limit=1 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE,0x02020008 );                               /* CE use CPs, ACT=2, ACB=0, auto-reacq SP phase,beta=2^(-1) */
  /* BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CE,0x0a020008 ); */                         /* CE DC pilot mode on, use CPs, ACT=2, ACB=0, auto-reacq SP phase,beta=2^(-1) */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 1 );                     /* CE bypass averager */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CP,0xc2029080 );                               /* CP t/c_modulo_add=1,timing PD average 32 symbols,timing PD beta=2^(-2),use freq detector,enable phase ramp comp,trk_range=4,acq_range=128 */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_EQ,0x00033100 );                               /* EQ nse avg all beta=2^(-11),nse avg beta=2^(-4),linear nse interpolation */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, EQ_BYP, 0x19);                       /* EQ bypass nse + avg nse exp, unbypass chan exp, unbypass nse averager, bypass bit scaling */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_IQ,0x00000440 );                               /* IQ Ka=Kp=2^(-14) */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,CP_BAD_RST, 1 );                     /* Assert CP bad memory reset*/
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CP_BAD,0x00000c40 );                           /* CP bad thresh=9dB, window=+/-4 pilots, auto, disabled for CPP */
#if 0
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CP_BAD,0x00000c41 );                           /* CP bad thresh=9dB, window=+/-4 pilots, auto, enabled for CPP */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP_CPE, USE_CP_BAD, 1 );                  /* CPE use bad pilot indicator */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP_CGE, USE_CP_BAD, 1 );                  /* CGE use bad pilot indicator */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE_LMS, TIME_USE_CP_BAD, 1 );             /* CE TI adapter use bad pilot indicator */
#endif
  /*************************************************************************
  * Acquisition
  *************************************************************************/
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FRZ,0x07f0f1e0 );                              /* Unfreeze NOTCH,CE AVG,DCOC,FFT Win,DAGC2,DAGC1,RF AGC,IF AGC */
#ifndef FASTACQ
  BKNI_Sleep(10);
#endif

  acqProfileString[0] = "Unfreeze";
  acqProfile[0] = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);

  /* BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, THRESH, 0x028c4 ); */               /* DAGC2 threshold=14dB (set to 14dB for improved acquisition probability w/spurs) (uint32_t)(pow(10.0,-14/10)* 262144) */

  /*  Co-Channel (Always enable video carrier notch before set_mode_guard() - improves 2K mode/guard detection with analog TV CCI) */
  if (p7550->pAcquireParam->CoChannelMode == THD_CoChannelMode_Auto) {
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, NOTCH_FRZ ,0x3c );                 /* Unfreeze Notch DDFS 0,0M    */                
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_SWP2,0x04001001 );                     /* Dwell=0x40,Step_Size=0x4,Num_Steps=0x1 */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_SWP3,0x00000000 );                     /* Threshold = 0 */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_SWP1,0x801b2200 );                     /* Start sweep,enable carrier adjust,enable notch0 pwr,sweep  notch width = 0xb,enable thresh,auto enable */
    while (  BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_STATUS1, DONE ) != 1 );
  }

  TransGuardResult = BTHD_7550_P_SetTransGuard(h);
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_INTR_STATUS2_CLR,0xffffffff);
  /* BREG_Write32(p7550->hRegister, BCHP_THD_CORE_RESERVED1,TransGuardResult); */
  if (TransGuardResult == THD_TransGuardResult_NoSignal) {
    return (THD_AcquireResult_NoSignal);
  }

  acqProfileString[1] = "TransGuard";
  acqProfile[1]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);

  /* Determine N */
  TransmissionMode = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS_OV, TRANS_MODE );
  switch (TransmissionMode) {
  case THD_TransmissionMode_2k: N = 2048; break; 
  case THD_TransmissionMode_4k: N = 4096; break;
  case THD_TransmissionMode_8k: N = 8192; break;
  }
  GuardInterval = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS_OV, GUARD);
  BDBG_MSG(("BTHD_7550_P_Acquire:\tTransmissionMode = %d, GuardInterval = %d", TransmissionMode, GuardInterval));

  /* Co-Channel */
  if (p7550->pAcquireParam->CoChannelMode == THD_CoChannelMode_Auto) 
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, NOTCH_FRZ, 0x30 );                    

  /* Impulse Noise */
  if (p7550->pAcquireParam->ImpulseMode == THD_ImpulseMode_Auto) {                        
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_IMPE_MA_THR,0x00118eed);        /* Set impulse eraser moving averager threshold to -7dB */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_IMPE_INST_THR,0x000a24b0);      /* Set impulse eraser instantaneous threshold to -9dB */
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_IMPE,0x00000040);               /* Enable impulse eraser,11-tap moving average */
    value = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_IMPE_ERASE_CNT);
    if (value < 50)
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_IMPE,0x00000041);             /* Disable impulse eraser,11-tap moving average */
  }

  /* FFT Window and Loops */
  /* BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, THRESH, 0x00a3d ); */           /* DAGC2 threshold=20dB (uint32_t)(pow(10.0,-20/10)* 262144) */
  /* BTHD_7550_PI_OSleep(h,2,N,GuardInterval); */                              /* Wait 2 symbols */ 
#ifndef FASTACQ
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_COEF,0x02000000);              /* Carrier loop Ki=2^(-6),Kl=0 */
#else
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_COEF,0x08000000);              /* Carrier loop Ki=2^(-4),Kl=0 */
#endif
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x01000000 * (N/2048));   /* Timing loop  Ki=2^(-7)*N/2048,Kl=0 (freq detector acquisition BW) */

  BDBG_MSG(("BTHD_7550_P_Acquire:\tp7550->pAcquireParam->FFTWindowMode == %d", p7550->pAcquireParam->FFTWindowMode));
  if (p7550->pAcquireParam->FFTWindowMode == THD_FFTWindowMode_Auto) {
    switch (p7550->pStatus->ReacquireCount % 3) {
      case 0:   FFTWindowMode = THD_FFTWindowMode_InSpan;         break;
      case 1:   FFTWindowMode = THD_FFTWindowMode_OutOfSpanPre;   break;
      case 2:   FFTWindowMode = THD_FFTWindowMode_OutOfSpanPost;  break;
      default:  FFTWindowMode = THD_FFTWindowMode_InSpan;         break;
    }
    FFTWindowRetries = 3;
  } else {
    FFTWindowMode = p7550->pAcquireParam->FFTWindowMode;
    FFTWindowRetries = 1;
  }
  BTHD_7550_P_SetFW(h,FFTWindowMode,TransmissionMode,GuardInterval);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CL_FRZ, 0 );                         /* Unfreeze carrier loop */ 
  if (BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_INTR_STATUS, FW_SYNC))
    BDBG_WRN(("BTHD_7550_P_Acquire:\tFW_SYNC Interrupt Status is set before FW Search!"));
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_INTR_STATUS_CLR, FW_SYNC, 1);             /* Clear interrupt */
  BKNI_ResetEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncEvent);
  BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncCallback );
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FW_RST, 0);                          /* Release FFT window reset */  
  /* BDBG_MSG(("BTHD_7550_P_Acquire:\tWaiting for FFT Lock ...")); */
  if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncEvent, 200) != BERR_SUCCESS) {
    BDBG_MSG(("BTHD_7550_P_Acquire:\t failed THD_CORE_INTR_STATUS = %80x", BREG_Read32(p7550->hRegister, BCHP_THD_CORE_INTR_STATUS)));
    return_val = THD_AcquireResult_NoFFTLock;
    BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncCallback ); 
    BKNI_ResetEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncEvent);
    return(return_val);
  }
  BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncCallback );  
  BKNI_ResetEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncEvent);
  /* BDBG_MSG(("BTHD_7550_P_Acquire:\tFFT Lock")); */

  acqProfileString[2] = "FFT Lock";
  acqProfile[2]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);


  /*** Integer Carrier Acquisition ***/

  if (p7550->pAcquireParam->CarrierRange == THD_CarrierRange_Narrow)
    value =  48 * (N/2048);                                                                   /* CP acq_range={ 48, 96,192} for {2K,4K,8K} (works for +/-210kHz offset in 8MHz channels)*/
  else
    value = 144 * (N/2048);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, ACQ_RANGE, value);                    /* CP acq_range=192 for {4K,8K} (temporary fix for ICE acquisition failure in Portable channel at low SNR) */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, THRESH, 0x028c4 );                 /* DAGC2 threshold=14dB (uint32_t)(pow(10.0,-14/10)* 262144) */

  BTHD_7550_PI_OSleep(h,2,N,GuardInterval);                                                   /* Wait 2 symbols */ 

  ice = BTHD_7550_P_SetICE(h,TransmissionMode,GuardInterval);                                 /* Determine ICE value which occurs most often */
  if (p7550->pStatus->LowICECount)                                                            /* Exit on ICE lock failure */
    return (THD_AcquireResult_NoSignal);
  cl_int = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_CL_INT);
  if (ice >> 10)
    cl_int_ice = cl_int + ((0xfffff800 | ice) << 21);                                         /* Sign extend for negative ICE value */
  else
    cl_int_ice = cl_int + (ice << 21); 
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_CL_INT,cl_int_ice);  
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CL_FRZ, 0 );                         /* Unfreeze carrier loop */

  acqProfileString[3] = "ICE";
  acqProfile[3]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);

  /*** Reduce input level to BE and DAGC bandwidths ***/

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, THRESH, 0x00a3d );                 /* DAGC2 threshold=20dB (uint32_t)(pow(10.0,-20/10)* 262144) */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC1, KL,0x8 );                          /* DAGC1 K=2(-14) */    
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC2, KL,0x8 );                          /* DAGC2 K=2(-14) */
#ifndef FASTACQ
  BTHD_7550_PI_OSleep(h,10,N,GuardInterval);                                                  /* Wait 10 symbols */ 
#endif

  acqProfileString[4] = "DAGC";
  acqProfile[4]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);


  /*************************************************************************
  * Acquisition Loop                                                   
  ************************************************************************/

  /*** Try various In-span and Out-of-span modes until lock is achieved ***/

  lock = 0;
  retries = 0;
  while (!lock && (retries < FFTWindowRetries)) {
    return_val = 0;    
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, BE_MODE, 0 );                      /* BE=disabled */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, ICE_MODE, 0 );                     /* ICE=tracking */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, TL_FRZ, 1 );                       /* Freeze timing loop */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_CFINE_FRZ,1 );                  /* Freeze fine carrier estimator */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_TFINE_FRZ,1 );                  /* Freeze fine timing estimator */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CE_AVG_FRZ, 1 );                   /* Freeze CE averager */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CPE_BYP, 1 );                      /* Bypass CPE */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, SP_PHASE_REACQ_MODE, 1 );           /* Enable auto-reacq SP phase */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, TL_RST,1 );                        /* Reset timing loop */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, TL_RST,0 );
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_FEC_BCH_TPS_SNR_RST ,0x3b );    /* Assert RS, FEC, BCH, TPS, and SNR resets */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, NSE_AVG_RST, 1 );                 /* Assert noise averager reset*/

    if (retries) {
      if (p7550->pAcquireParam->FFTWindowMode == THD_FFTWindowMode_Auto)                      /* Re-acquire FFT window for out-of-span post echoes */
        switch ((p7550->pStatus->ReacquireCount + retries) % 3) {
          case 0:   FFTWindowMode = THD_FFTWindowMode_InSpan;         break;
          case 1:   FFTWindowMode = THD_FFTWindowMode_OutOfSpanPre;   break;
          case 2:   FFTWindowMode = THD_FFTWindowMode_OutOfSpanPost;  break;
          default:  FFTWindowMode = THD_FFTWindowMode_InSpan;         break;
      }
      else
        FFTWindowMode = p7550->pAcquireParam->FFTWindowMode;
      BTHD_7550_P_SetFW(h, FFTWindowMode, TransmissionMode, GuardInterval);
      /*BDBG_MSG(("DEBUG -- ReacquireCount = %d, RetryCount = %d, WindowMode = %d", p7550->pStatus->ReacquireCount, retries, FFTWindowMode));*/
#if 0
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FW_RST, 1);                      /* Assert FFT window reset */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, FW_RST, 0);                      /* Release FFT window reset */
      BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncCallback ); 
      if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncEvent, 200) != BERR_SUCCESS)
        return_val = THD_AcquireResult_NoFFTLock;
      /* BDBG_MSG(("BTHD_7550_P_Acquire:\tFFT Lock")); */
      BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hFwSyncCallback ); 
#endif
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, FW_REF_RST, 1 );                /* Reset FFT window reference index */
      BTHD_7550_PI_OSleep(h,1,N,GuardInterval);  
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, FW_REF_RST, 0 );
    }
    BDBG_MSG(("BTHD_7550_P_Acquire:\tReacquire Count = %d, Retry Count = %d, FFTWindowMode = %d", p7550->pStatus->ReacquireCount, retries, FFTWindowMode));

    /*** Enable continuous pilot processing and the BE ***/

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, TL_FRZ, 0 );                       /* Unfreeze timing loop */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_TFINE_FRZ, 0 );                 /* Unfreeze fine timing estimator */  
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, BE_MODE, 1 );                      /* BE=enabled */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_GLB, ICE_MODE, 0 );                     /* ICE=tracking */


    /*** Scattered Pilot Synchronization ***/ 
    if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
      /* Do not wait for SP Lock... unreliable at this point because TMCC has not yet identified
      which segments contain scattered pilots */ 
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, ACE_ACT ,0x00);                    
    }
    else {
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, ACE_ACT ,0x82);                   /* FFT window center */

      /* BDBG_MSG(("BTHD_7550_P_Acquire:\tWaiting for SP Lock ...")); */
      BINT_EnableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hSpSyncCallback );
      if (BKNI_WaitForEvent(((BTHD_7550_P_Handle *)(h->pImpl))->hSpSyncEvent, 100) != BERR_SUCCESS) {                      
        return_val = THD_AcquireResult_NoSPLock;
      }
      BINT_DisableCallback( ((BTHD_7550_P_Handle *)(h->pImpl))->hSpSyncCallback);
      /* if (return_val != THD_AcquireResult_NoSPLock) */
      /* BDBG_MSG(("BTHD_7550_P_Acquire:\tSP Lock")); */
    }
    acqProfileString[5] = "SP";
    acqProfile[5]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);



    /*** Check for Scattered Pilot Lock and continue ***/

    if (return_val != THD_AcquireResult_NoSPLock) {
#if 0
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,NSE_AVG_RST, 0 );                /* Release noise averager */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,CP_BAD_RST, 0 );                 /* Release CP bad memory reset*/
      BTHD_7550_PI_OSleep(h,1,N,GuardInterval);                                               /* Wait 1 OFDM symbol */
#endif
#ifndef FASTACQ
      BTHD_7550_PI_OSleep(h,64,N,GuardInterval);                                              /* Wait 64 OFDM symbols for span calculation to settle */     
#else
      BTHD_7550_PI_OSleep(h,10,N,GuardInterval);
#endif
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_SPAN,0x00000140 );                      /* FFT window span scale = 0.1875 (better for detecting long weak echoes) */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_CFINE_FRZ, 0 );               /* Unfreeze fine carrier estimator */


      /*** Begin Channel Estimation and Common Phase Estimation ***/

      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CE_AVG_FRZ, 0 );                 /* Unfreeze CE averager */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CPE_BYP, 0 );                    /* Unbypass CPE */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, SP_PHASE_REACQ_MODE, 0 );         /* Disable auto-reacq SP phase */

      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, SNR_RST, 0 );                    /* Release SNR reset */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,NSE_AVG_RST, 1 );                /* Reset noie averager (necessary for proper co-channel filter detection when CP bad is used) */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,NSE_AVG_RST, 0 );                /* Release noise averager */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2,CP_BAD_RST, 0 );                 /* Release CP bad memory reset*/

      acqProfileString[6] = "CP";
      acqProfile[6]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);


      /*** Search for TPS (DVB-T) or TMCC (ISDB-T) and configure FEC and OI ***/

      if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
        BTHD_7550_P_ISDBT_SetFEC(h);
        BTHD_7550_P_ISDBT_SetOI(h);
        return_val = BTHD_7550_P_SetTMCC(h, TransmissionMode, GuardInterval);  /* Setup TMCC and wait for lock or timeout */
      } 
      else {
        return_val = BTHD_7550_P_SetTPS(h, TransmissionMode, GuardInterval);   /* Setup TPS and wait for lock or timeout */
        if (return_val != THD_AcquireResult_NoTPSLock) {
          BTHD_7550_P_SetOI(h);                                                /* Setup output interface */
          return_val = BTHD_7550_P_SetFEC(h);                                  /* Setup FEC and wait for lock or timeout */
        }
      }
    }

    if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT)
      acqProfileString[7] = "TMCC";
    else
      acqProfileString[7] = "TPS";
    acqProfile[7]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);


    /*** Check for Cochannel Interference and Remove Notch Filter if not Needed ***/

    if ((return_val != THD_AcquireResult_NoSPLock) && (return_val != THD_AcquireResult_NoTPSLock)) {
      if (p7550->pAcquireParam->CoChannelMode != THD_CoChannelMode_None) {
        /* Need to enable the CE averager in order to estimate the noise */
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, AVG_FF, 0x0);                       /* CE averager beta = 2^(-4) */
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 0);                    /* CE averager unbypassed */
        BTHD_7550_PI_OSleep(h,32,N,GuardInterval);
        /* Determine CCI presence using snooper (Look for video, color, or audio to guarantee robust CCI detection) */
        if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT)
          CoChannelPresent = BTHD_7550_P_GetNotchISDBT(h, TransmissionMode);
        else
          {
			BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, EXP_AVG_NSE, 0x0 );                /* EQ nse + avg nse exponents unbypassed */
			CoChannelPresent = BTHD_7550_P_GetNotch(h, TransmissionMode);
		  }        
		
		BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, CE_AVG_BYP, 1);                    /* CE averager unbypassed */

        if (!CoChannelPresent) {
          do {                                                                                    /*  Gradually reduce notch to avoid burst error */
            notchwidth = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH4);
            if (notchwidth)
              BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH4, notchwidth - 1);
            notchwidth = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH3);
            if (notchwidth)
              BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH3, notchwidth - 1);
            notchwidth = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH2);
            if (notchwidth)
              BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH2, notchwidth - 1);
            notchwidth = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH1);
            if (notchwidth)
              BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH1, notchwidth - 1);
            notchwidth = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH0);
            if (notchwidth)
              BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_NOTCH_WIDTH, WIDTH0, notchwidth - 1);
            notchwidth = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_NOTCH_WIDTH);
            BTHD_7550_PI_OSleep(h,3,N,GuardInterval); 
          } while (notchwidth);
        }
        else
          BDBG_MSG(("BTHD_7550_P_Acquire:\tCochannel Interference Detected."));
      }
    }


    /*** Wait for data at FEC output ***/

    if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
      segs_b = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG);
      segs_c = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG);
    }
    else {
      segs_b = 0;
      segs_c = 0;
    }

    if ((return_val != THD_AcquireResult_NoTPSLock) && (return_val != THD_AcquireResult_NoSPLock)) {
      if (p7550->pAcquireParam->Standard == THD_Standard_ISDBT) {
        timeout = 540*54000;                                                                      /* 540 ms timeout (2 frames 8K, 1/4)*/
      }
      else {
        timeout = 25*54000;                                                                       /* 25 ms timeout */
      }
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 1 );                     /* Reset FEC error counters */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 0 );
      fscntInit2 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);                            /* Use FSCNT as timer */
      do {
        value = fscntInit2 - BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);
        clean   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC) + BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC);
        clean_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B) + BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B);
        clean_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C) + BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C); 
        /*** Wait for FEC data on all layers.  Ignore layer A if only one segment in order to speed up acquisition. ***/
        if ((clean || (segs_b + segs_c == 12)) && (!segs_b || clean_b) && (!segs_c || clean_c))
          break;
        BKNI_Sleep(1);
      } while (value < timeout);                                                  
      if (value >= timeout)  {
        return_val = THD_AcquireResult_NoFECLock; 
        BDBG_WRN(("BTHD_7550_P_Acquire:\tTimeout at %d ms waiting for FEC data during FFTWindow check.", value/54000));
      }
      /* BDBG_MSG(("clean = %d, clean_b = %d, clean_c = %d, return_val = %d", clean, clean_b, clean_c, return_val)); */
    }

    acqProfileString[8] = "FEC Lock";
    acqProfile[8]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);

    /*** Check for lock failure and retry ***/

    if  (return_val == THD_AcquireResult_NoTPSLock) {
      lock = 0;
      retries = FFTWindowRetries;
    }
    else if ((return_val == THD_AcquireResult_NoSPLock) || (return_val == THD_AcquireResult_NoFECLock)) {
      lock = 0;
      retries++;
      if (return_val == THD_AcquireResult_NoSPLock)
        data8++;
      continue;
    } else
      lock = 1;
  }


  /*************************************************************************
  * Check Lock Status
  *   At this point, lock has been achieved or the retry limit has been exceeded.
  *     If locked, TMCC lock has been achieved for ISDB-T.  For DVB-T, TPS
  *     lock has been found along with SP lock, and FEC (0x47) lock.
  ************************************************************************/

  /*** Check for false lock in manual TPS mode (prevents false lock of 64-QAM,1/2 to 16-QAM,3/4) ***/

  if ((p7550->pAcquireParam->TPSMode == THD_TPSMode_Manual) && (BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS, RX_INFO) != BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_TPS_OV, RX_INFO)))  
    return_val = THD_AcquireResult_NoFECLock;


  /*** Check for lock failure and exit ***/

  if ((return_val == THD_AcquireResult_NoSPLock) || (return_val == THD_AcquireResult_NoTPSLock) || (return_val == THD_AcquireResult_NoFECLock)) {

    /*** Report no signal if scattered pilot synchronization could not be achieved ***/

    if ((return_val == THD_AcquireResult_NoSPLock) && (data8 == 3)) {
      return_val = THD_AcquireResult_NoDVBTSignal;
    }

    /*** Report acquisition time ***/

    value = fscntInit - BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);                   /* Acquisition time profiling */
    p7550->pStatus->AcquisitionTime = value;
    return(return_val);
  }


  /*************************************************************************
  * Tracking
  ************************************************************************/

  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FEC_SYNC,0x07001f1f );                         /* Force FEC sync retention */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_VIT,0x00000687 );                              /* Force Viterbi sync retention */    
  BTHD_7550_P_SetFrame(h);                                                                    /* Setup frame size (for ESR) */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_DAGC2,0x04000A3D );                            /* DAGC2 threshold=20dB, thresh = (thd_u32)(pow(10.0,-17/10)* 262144), K=2^(-8) */


  /*** Channel Estimator Tracking ***/

  if (p7550->pAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Auto)
    ChannelEstimatorMode = THD_ChannelEstimatorMode_Fixed;
  else
    ChannelEstimatorMode = p7550->pAcquireParam->ChannelEstimatorMode;
  BTHD_7550_P_SetCE(h, ChannelEstimatorMode);
  /* BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000208 ); */             /* FFT window leak to timing loop enabled w/limiting (prevents "V" noise response) */
  BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x0000020a );             /* FFT window leak to timing loop enabled w/limiting (prevents "V" noise response) */


  /*** Common Gain Estimator ***/

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP_CGE, THRESH, 0x16b );                  /* CGE target back-off = 9dB (floor(10^(-9/20)*2^10)) */
  if (p7550->pAcquireParam->Standard != THD_Standard_ISDBT)
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP_CGE, ENABLE, 1 );                    /* CGE enable */


  /*** Reduce Timing and Carrier Bandwidths ***/

  if (!BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_NOTCH_STATUS1,BYP_DP))
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00100000 * (N/2048) );             /* Ki=2^(-11)*N/2048,Kl=2^0 (better for CCI) */  
  else
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00020000 * (N/2048) );             /* Ki=2^(-14)*N/2048,Kl=2^0 (better for echo tests inside/outside guard interval) */


  /*** Reduce AGC Bandwidths ***/

  ce_beta = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_CE,AVG_FF );
  aagc_K = 0x10 + ce_beta; /* 0x17 + ce_beta; */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_AAGC_COEF, KI, aagc_K );                  /* Kif=2^(-26),Krf=-2^(-20) */
  aagc_K = 8 + (ce_beta << 1);
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_DAGC1, KL, (uint32_t)aagc_K );            /* Set DAGC1 K proportional to CE averaging */


  /*** FW Tracking Parameters ***/

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FW, LIMIT, 0);                            /* FW slip_limit=0 (fixes "breathing noise response") */
  if ((N > 2048) && (ChannelEstimatorMode != THD_ChannelEstimatorMode_Fixed))
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_WIN,0x00000001);                          /* Enable time-domain windowing in auto mode */
  if (FFTWindowMode == THD_FFTWindowMode_OutOfSpanPre)
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FW, POST_CB, 0);                        /* FW precursor bias */

  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, FW_REF_RST, 1 );                    /* Reset FFT window reference index */
  BTHD_7550_PI_OSleep(h,1,N,GuardInterval);  
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST2, FW_REF_RST, 0 );


  /*** Cochannel Interference Tracking Parameters ***/

  if ((p7550->pAcquireParam->CoChannelMode != THD_CoChannelMode_None) && CoChannelPresent) {
   
    if (ChannelEstimatorMode == THD_ChannelEstimatorMode_Fixed) {                           /*  Enable timing PD for fixed mode */
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, F_SPACE, 2 );                   /*  12-bin timing pd pilot spacing (mandatory for long-echo channels)*/
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, PHASE_FREQ_SELECT, 1 );         /*  Timing pd enable*/
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, CP_TPHASE_FRZ, 0 );            /*  Unfreeze timing pd  */
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00010200 * (N/2048) );         /*  Ki=2^(-13),Kl=2^(-4) (for CCI)*/
      BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000200 );                    /*  FFT window leak to timing loop disabled (prevents "wandering" at low SNR) */
    }  

    BTHD_7550_P_SetEq(h, THD_CoChannelMode_Auto);  

    if (TransmissionMode == THD_TransmissionMode_2k)
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_EQ, EXP_OFFSET, 0x3f8);             /*  EQ exponent offset = -8 for 2K (helps with color pattern (not color bars))*/
    if (TransmissionMode == THD_TransmissionMode_8k)
      BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE, AVG_FF, 0x1);                   /*  CE set beta=2^-2 for 8K w/CCI*/

    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_BYP, NOTCH_BYP, 0);                   /*  Unbypass notch filter*/
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_DEPTH,0x0001281a );
    BREG_Write32(p7550->hRegister, BCHP_THD_CORE_NOTCH_WIDTH,0x0008bbcb );
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_FRZ, NOTCH_FRZ ,0x30 );               /*  Unfreeze Notch DDFS,2,1,0,0M*/
  
  } 

  /*** Adaptive Time/Frequency Interpolators ***/

  if (p7550->pAcquireParam->Standard != THD_Standard_ISDBT) {
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE_LMS, TIME_ENABLE, 1 );               /* Adaptive time interpolator enable */
    BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE_LMS, FREQ_MU_CENTER, 1 );            /* Adaptive frequency interpolator mu=2^(-4) */
    /* BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CE_LMS, FREQ_ENABLE, 1 ); */         /* Adaptive frequency interpolator enable */
  }

  acqProfileString[9] = "Time/Freq Interp";
  acqProfile[9]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);

  /*** Wait for data at FEC output ***/


  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 1 );                     /* Reset FEC error counters */
  BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_RST, RS_ERC_RST, 0 );
  timeout = 514*54000;
  limit   = 0;
  fscntInit2 = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);                            /* Use FSCNT as watchdog timer */
  do {
    value = fscntInit2 - BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);
    clean   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC);
    clean_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_B);
    clean_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_NBERC_C);
    cberc   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC);
    cberc_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_B);
    cberc_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_CBERC_C);
    uberc   = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC);
    uberc_b = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_B);
    uberc_c = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_RS_UBERC_C);
    total   = clean + cberc + uberc;
    total_b = clean_b + cberc_b + uberc_b;
    total_c = clean_c + cberc_c + uberc_c;
    /* data_found = (total + total_b + total_c > 300); */
    /* if ((clean + cberc) || (segs_b && (clean_b + cberc_b)) || (segs_c && (clean_c + cberc_c)) */
    if (clean || (segs_b && clean_b) || (segs_c && clean_c))    
      break;
    limit = ( (total + total_b + total_c > 1000) && (total > 10) && (!segs_b || (total_b > 10)) && (!segs_c || (total_c > 10)) ); 
    BKNI_Sleep(1);
  } while ((value < timeout) && !limit);
  if ((value >= timeout) || limit) {
    BDBG_WRN(("BTHD_7550_P_Acquire:\tFEC timed out without detecting clean data. value=%d. limit=%d", value/54000, limit)); 
    BDBG_WRN(("BTHD_7550_P_Acquire:\tclean=%d, cberc=%d, uberc=%d, total=%d",clean, cberc, uberc, total));
    return_val = THD_AcquireResult_NoFECLock;
  }
  else
    return_val = THD_AcquireResult_Lock;

  acqProfileString[10] = "FEC Output";
  acqProfile[10]       = BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);


  /*** For Setup=Auto, check for lock, if not locked then switch to Pedestrian mode ***/
  if (!(p7550->pAcquireParam->Standard == THD_Standard_ISDBT)) {
    if ((p7550->pAcquireParam->ChannelEstimatorMode == THD_ChannelEstimatorMode_Auto) && !CoChannelPresent && (FFTWindowMode == THD_FFTWindowMode_InSpan)) {
      BTHD_7550_PI_OSleep(h,30,N,GuardInterval);
      BTHD_7550_P_LockOld(h);
      if (!((p7550->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1))
        return_val = THD_AcquireResult_NoFECLock;
      else
        return_val = THD_AcquireResult_Lock;
      if (/*(p7550->pStatus->ReacquireCount % 2) || */!((p7550->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)) {
        BDBG_MSG(("BTHD_7550_P_Acquire:\tLock fallback to Pedestrian CE mode"));
        ChannelEstimatorMode = THD_ChannelEstimatorMode_Pedestrian;
        BTHD_7550_P_SetCE(h, ChannelEstimatorMode);
        if (TransmissionMode != THD_TransmissionMode_2k)
          BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_WIN,0x00000001);                      /*  Enable time-domain windowing in auto mode*/
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_FW_MISC,0x00000208 );                      /*  FFT window leak to timing loop enabled w/limiting*/
        BREG_Write32(p7550->hRegister, BCHP_THD_CORE_TL_COEF,0x00020000 * (N/2048));            /*  Ki=2^(-14)*N/2048,Kl=2^0 (better for echo tests inside/outside guard interval)*/
        BTHD_7550_P_WriteField(p7550->hRegister, THD_CORE_CP, PHASE_FREQ_SELECT, 0 );           /*  Timing FD enable */
      }
    }
  }


  /***  Acquisition time profiling (must occur before call to thd_lock which also uses THD_FSCNT) ***/

  value = fscntInit - BREG_Read32(p7550->hRegister, BCHP_THD_CORE_FSCNT);        
  p7550->pStatus->AcquisitionTime = value/54000;
  BDBG_MSG(("BTHD_7550_P_Acquire:\tAcquisition Time = %d msec",p7550->pStatus->AcquisitionTime));  
  for (kk=0; kk<11; kk++)
    BDBG_MSG(("BTHD_7550_P_Acquire:\t%d msec: %s", (fscntInit - acqProfile[kk])/54000, acqProfileString[kk]));


  /*** Update status structure with auto-detected modes ***/

  p7550->pStatus->ChannelEstimatorMode = ChannelEstimatorMode;
  p7550->pStatus->FFTWindowMode = FFTWindowMode;
  p7550->pStatus->CoChannelPresent = CoChannelPresent;
  if (CoChannelPresent)
    p7550->pStatus->CoChannelMode = THD_CoChannelMode_Auto;
  else
    p7550->pStatus->CoChannelMode = THD_CoChannelMode_None;

  /* BDBG_MSG(("BTHD_7550_P_Acquire:\tLock")); */
  return(return_val);
}

/***************************************************************************
* BTHD_7550_P_PowerDisable()
***************************************************************************/
BERR_Code BTHD_7550_P_PowerDown(BTHD_Handle h)
{
  BSTD_UNUSED(h);    
  return BERR_SUCCESS;
}

/***************************************************************************
* BTHD_7550_P_GetThdStatus()
***************************************************************************/
BERR_Code BTHD_7550_P_GetThdStatus(BTHD_Handle h, BTHD_THDStatus *pStatus)
{
  BTHD_7550_P_Handle *p7550 = (BTHD_7550_P_Handle *)(h->pImpl);
  BTHD_7550_P_Status(h);

  /* ---------------  Mapping Common Fields ---------------- */

  pStatus->bReceiverLock             = ((p7550->ThdLockStatus >> THD_LockStatusBit_SystemLock) & 1);    /* Indicates whether the receiver is locked */
  pStatus->bFecLock                  = ((p7550->ThdLockStatus >> THD_LockStatusBit_FECLock) & 1);       /* Indicates whether FEC is locked */
  pStatus->bNoSignalDetected         = ((p7550->ThdLockStatus >> THD_LockStatusBit_NoDVBTSignal) & 1);  /* Indicates whether signal is detected. Only should be used by channel scan as a flag to move on to the next channel */                                       

  /* Transmission Mode */
  switch (p7550->pStatus->TransmissionMode)
  {
  case THD_TransmissionMode_2k:
    pStatus->eTransmissionMode  = BTHD_TransmissionMode_e2K; break;
  case THD_TransmissionMode_4k:
    pStatus->eTransmissionMode  = BTHD_TransmissionMode_e4K; break;
  default:
    BDBG_WRN(("Unrecognized TransmissionMode - defaulting to THD_TransmissionMode_8k"));
    /* fall through */
  case THD_TransmissionMode_8k:
    pStatus->eTransmissionMode  = BTHD_TransmissionMode_e8K; break;
  }

  /* Guard Interval */
  switch (p7550->pStatus->GuardInterval)
  {
  case THD_GuardInterval_1_32:
    pStatus->eGuardInterval  = BTHD_GuardInterval_e1_32; break;
  case THD_GuardInterval_1_16:
    pStatus->eGuardInterval  = BTHD_GuardInterval_e1_16; break;
  default:
    BDBG_WRN(("Unrecognized GuardInterval - defaulting to BTHD_GuardInterval_e1_8"));
    /* fall through */
  case THD_GuardInterval_1_8:
    pStatus->eGuardInterval  = BTHD_GuardInterval_e1_8; break;
  case THD_GuardInterval_1_4:
    pStatus->eGuardInterval  = BTHD_GuardInterval_e1_4; break;
  }

  pStatus->ulRFAgc                   = p7550->pStatus->RagcGain;                 /* RF Agc value in 1/10 percent */
  pStatus->ulIFAgc                   = p7550->pStatus->IagcGain;                 /* IF Agc value in 1/10 percent */
  /*pStatus->nSignalStrength */
  pStatus->lCarrierOffset            = p7550->pStatus->CarrierOffset;            /* Offset of carrier loop in Hz */
  pStatus->lTimingOffset             = p7550->pStatus->TimingOffset;             /* Offset of timing loop in Hz */
  pStatus->nSnr                      = p7550->pStatus->SNR;                      /* SNR value of reciever in 1/256th dB */   
  pStatus->bSpectrumInverted         = BTHD_7550_P_ReadField(p7550->hRegister, THD_CORE_FE, NEGATE_Q);                  /* Indicates the signal is inverted */
  pStatus->ulReacqCount              = p7550->pStatus->ReacquireCount;           /* number of reacquisitions performed */
  /*pStatus->bValid*/

  /* ---------------  Mapping DVB-T Specific Fields ---------------- */

  /* Decode Priority */
  switch (p7550->pStatus->Priority)
  {
  case THD_PriorityMode_High:
    pStatus->eDecodeMode  = BTHD_Decode_Hp; break;
  default:
    BDBG_WRN(("Unrecognized Priority - defaulting to BTHD_Decode_Lp"));
    /* fall through */
  case THD_PriorityMode_Low:
    pStatus->eDecodeMode  = BTHD_Decode_Lp; break;
  }

  /*QAM Mode*/
  switch (p7550->pStatus->Qam)
  {
  case THD_Qam_Qpsk:
    pStatus->eModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eModulation  = BTHD_Modulation_e16Qam; break;
  case THD_Qam_64Qam:
    pStatus->eModulation  = BTHD_Modulation_e64Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_Dqpsk:
    pStatus->eModulation  = BTHD_Modulation_e64Qam; break;
  }  

  /* Code Rate */
  switch (p7550->pStatus->CodeRateHP)
  {
  case THD_CodeRate_1_2:
    pStatus->eCodeRate  = BTHD_CodeRate_e1_2; break;
  case THD_CodeRate_2_3:
    pStatus->eCodeRate  = BTHD_CodeRate_e2_3; break;
  default:
    BDBG_WRN(("Unrecognized CodeRateHP - defaulting to BTHD_CodeRate_e3_4"));
    /* fall through */
  case THD_CodeRate_3_4:
    pStatus->eCodeRate  = BTHD_CodeRate_e3_4; break;
  case THD_CodeRate_5_6:
    pStatus->eCodeRate  = BTHD_CodeRate_e5_6; break;
  case THD_CodeRate_7_8:
    pStatus->eCodeRate  = BTHD_CodeRate_e7_8; break;
  }

  /* Hierarchy */
  switch (p7550->pStatus->Hierarchy)
  {
  case THD_Hierarchy_None:
    pStatus->eHierarchy  = BTHD_Hierarchy_0; break;
  case THD_Hierarchy_1:
    pStatus->eHierarchy  = BTHD_Hierarchy_1; break;
  default:
    BDBG_WRN(("Unrecognized Hierarchy - defaulting to THD_Hierarchy_2"));
    /* fall through */
  case THD_Hierarchy_2:
    pStatus->eHierarchy  = BTHD_Hierarchy_2; break;
  case THD_Hierarchy_4:
    pStatus->eHierarchy  = BTHD_Hierarchy_4; break;;
  }

  /*pStatus->nCellId*/
  pStatus->ulRsCorrectedBlocks       = p7550->pStatus->TS_CBERC;                 /* accumulated corrected blocks by FEC */
  pStatus->ulRsUncorrectedBlocks     = p7550->pStatus->TS_UBERC;                 /* accumlated uncorrected blocks */
  pStatus->ulRsCleanBlocks           = p7550->pStatus->TS_NBERC;                 /* accumlated clean RS blocks */
  pStatus->ulRsTotalBlocks			     = p7550->pStatus->TS_TBERC;
  pStatus->ulViterbiUncorrectedBits  = p7550->pStatus->TS_CERC;                  /* accumlated error bits after Viterbi kk*/
  pStatus->ulViterbiTotalbits        = (p7550->pStatus->TS_TBERC)*8*188;                  /* accumlated number of bits after Viterbi kk*/
  pStatus->ulViterbiBer              = p7550->pStatus->VitBER;                   /* This indicates the viterbi error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
  pStatus->ulPreViterbiBer           = p7550->pStatus->PreVitBER;                /* Indicates the pre-viterbi error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */

  /* ---------------  Mapping ISDB-T Specific Fields ---------------- */

  pStatus->bIsdbtEWS					= p7550->pStatus->ISDBT_EWS;                /* This return the EWS status bit */
  pStatus->bIsdbtPartialReception		= p7550->pStatus->ISDBT_PR;

  /*Layer A Modulation*/
  switch (p7550->pStatus->ISDBT_A_Qam)
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_e16Qam; break;
  case THD_Qam_64Qam:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_e64Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_e64Qam; break;
  }

  /*Layer A CodeRate*/
  switch (p7550->pStatus->ISDBT_A_CodeRate)
  {
  case THD_CodeRate_1_2:
    pStatus->eIsdbtACodeRate  = BTHD_CodeRate_e1_2; break;
  case THD_CodeRate_2_3:
    pStatus->eIsdbtACodeRate  = BTHD_CodeRate_e2_3; break;
  default:
    BDBG_WRN(("Unrecognized CodeRateHP - defaulting to BTHD_CodeRate_e3_4"));
    /* fall through */
  case THD_CodeRate_3_4:
    pStatus->eIsdbtACodeRate  = BTHD_CodeRate_e3_4; break;
  case THD_CodeRate_5_6:
    pStatus->eIsdbtACodeRate  = BTHD_CodeRate_e5_6; break;
  case THD_CodeRate_7_8:
    pStatus->eIsdbtACodeRate  = BTHD_CodeRate_e7_8; break;
  }

  /*Layer A TimeInterleaving*/       
  switch (p7550->pStatus->ISDBT_A_TimeInt)
  {
  case THD_TimeInt_0X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_TimeInt_1X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_TimeInt_2X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_TimeInt_4X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtASegments = p7550->pStatus->ISDBT_A_Segments;
  pStatus->ulIsdbtARsCorrectedBlocks    = p7550->pStatus->ISDBT_A_TS_CBERC; 
  pStatus->ulIsdbtARsUncorrectedBlocks  = p7550->pStatus->ISDBT_A_TS_UBERC;
  pStatus->ulIsdbtARsCleanBlocks		= p7550->pStatus->ISDBT_A_TS_NBERC;
  pStatus->ulIsdbtARsTotalBlocks		= p7550->pStatus->ISDBT_A_TS_TBERC;         
  pStatus->ulIsdbtAViterbiUncorrectedBits	= p7550->pStatus->ISDBT_A_VitBER;
  pStatus->ulIsdbtAViterbiTotalbits			= (p7550->pStatus->ISDBT_A_TS_TBERC)*8*188;

  /*Layer B Modulation*/      
  switch (p7550->pStatus->ISDBT_B_Qam)
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_e16Qam; break;
  case THD_Qam_64Qam:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_e64Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_e64Qam; break;
  }

  /*Layer B CodeRate*/
  switch (p7550->pStatus->ISDBT_B_CodeRate)
  {
  case THD_CodeRate_1_2:
    pStatus->eIsdbtBCodeRate  = BTHD_CodeRate_e1_2; break;
  case THD_CodeRate_2_3:
    pStatus->eIsdbtBCodeRate  = BTHD_CodeRate_e2_3; break;
  default:
    BDBG_WRN(("Unrecognized CodeRateHP - defaulting to BTHD_CodeRate_e3_4"));
    /* fall through */
  case THD_CodeRate_3_4:
    pStatus->eIsdbtBCodeRate  = BTHD_CodeRate_e3_4; break;
  case THD_CodeRate_5_6:
    pStatus->eIsdbtBCodeRate  = BTHD_CodeRate_e5_6; break;
  case THD_CodeRate_7_8:
    pStatus->eIsdbtBCodeRate  = BTHD_CodeRate_e7_8; break;
  }

  /*Layer B TimeInterleaving*/       
  switch (p7550->pStatus->ISDBT_B_TimeInt)
  {
  case THD_TimeInt_0X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_TimeInt_1X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_TimeInt_2X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_TimeInt_4X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtBSegments				= p7550->pStatus->ISDBT_B_Segments;
  pStatus->ulIsdbtBRsCorrectedBlocks	= p7550->pStatus->ISDBT_B_TS_CBERC;
  pStatus->ulIsdbtBRsUncorrectedBlocks	= p7550->pStatus->ISDBT_B_TS_UBERC;
  pStatus->ulIsdbtBRsCleanBlocks		= p7550->pStatus->ISDBT_B_TS_NBERC;
  pStatus->ulIsdbtBRsTotalBlocks		= p7550->pStatus->ISDBT_B_TS_TBERC;
  pStatus->ulIsdbtBViterbiUncorrectedBits	= p7550->pStatus->ISDBT_B_VitBER;
  pStatus->ulIsdbtBViterbiTotalbits			= (p7550->pStatus->ISDBT_B_TS_TBERC)*8*188;

  /*Layer C Modulation*/      
  switch (p7550->pStatus->ISDBT_C_Qam)
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_e16Qam; break;
  case THD_Qam_64Qam:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_e64Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_e64Qam; break;
  }

  /*Layer C CodeRate*/
  switch (p7550->pStatus->ISDBT_C_CodeRate)
  {
  case THD_CodeRate_1_2:
    pStatus->eIsdbtCCodeRate  = BTHD_CodeRate_e1_2; break;
  case THD_CodeRate_2_3:
    pStatus->eIsdbtCCodeRate  = BTHD_CodeRate_e2_3; break;
  default:
    BDBG_WRN(("Unrecognized CodeRateHP - defaulting to BTHD_CodeRate_e3_4"));
    /* fall through */
  case THD_CodeRate_3_4:
    pStatus->eIsdbtCCodeRate  = BTHD_CodeRate_e3_4; break;
  case THD_CodeRate_5_6:
    pStatus->eIsdbtCCodeRate  = BTHD_CodeRate_e5_6; break;
  case THD_CodeRate_7_8:
    pStatus->eIsdbtCCodeRate  = BTHD_CodeRate_e7_8; break;
  }

  /*Layer C TimeInterleaving*/       
  switch (p7550->pStatus->ISDBT_C_TimeInt)
  {
  case THD_TimeInt_0X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_TimeInt_1X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_TimeInt_2X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_TimeInt_4X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtCSegments				=  p7550->pStatus->ISDBT_C_Segments;
  pStatus->ulIsdbtCRsCorrectedBlocks	= p7550->pStatus->ISDBT_C_TS_CBERC;
  pStatus->ulIsdbtCRsUncorrectedBlocks	= p7550->pStatus->ISDBT_C_TS_UBERC;
  pStatus->ulIsdbtCRsCleanBlocks		= p7550->pStatus->ISDBT_C_TS_NBERC;
  pStatus->ulIsdbtCRsTotalBlocks		= p7550->pStatus->ISDBT_C_TS_TBERC;
  pStatus->ulIsdbtCViterbiUncorrectedBits	= p7550->pStatus->ISDBT_C_VitBER;
  pStatus->ulIsdbtCViterbiTotalbits			= (p7550->pStatus->ISDBT_C_TS_TBERC)*8*188;

  return BERR_SUCCESS;
}


