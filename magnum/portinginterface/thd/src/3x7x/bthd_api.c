/******************************************************************************
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
*****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btmr.h"
#include "bthd.h"
#include "bthd_priv.h"
#include "bthd_3x7x.h"
#include "bthd_api.h"
#include "bthd_acq.h"
#include "bthd_acq_dvbt.h"
#include "bthd_acq_isdbt.h"
#include "bchp_thd_core.h"
#include "bchp_thd_intr2.h"
#include "bchp_thd_intr2b.h"
#include "bchp_int_id_thd_intr2.h"
#include "bchp_int_id_thd_intr2b.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr_resources.h"
#include "bchp_pwr.h"
#endif

BDBG_MODULE(bthd_api);


/***************************************************************************
 * BTHD_P_EventIsdbtIsr()
 ***************************************************************************/
#ifdef BTHD_ISDBT_SUPPORT
static void BTHD_P_EventIsdbtIsr(void *p, int param)
{
  BTHD_3x7x_Handle pHandle = (BTHD_3x7x_Handle)p;
  uint32_t      segs_b=0, segs_c=0;
  uint32_t      lock_acq0, lock_acq1, lock_acq;
  uint32_t      lock_chk, clr_chk;
  uint32_t      a, ra;

  if (pHandle->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On)
      return;
  segs_b = BREG_ReadField(pHandle->hRegister, THD_CORE_TMCC_OV_1, LAYERB_SEG);
  segs_c = BREG_ReadField(pHandle->hRegister, THD_CORE_TMCC_OV_1, LAYERC_SEG);
  a = (pHandle->pInternalAcquireParam->AllowRsSyncEvent);
  ra = (pHandle->pStatus->ThdCommonStatus.ReacquireCount % 2);
  pHandle->pAcquireParam->IsdbtLocaleParam.phase = !a * pHandle->pAcquireParam->IsdbtLocaleParam.phase + a * ra;
  switch(param) {
  case THD_EVENT_RS_SYNC:
    pHandle->pAcquireParam->IsdbtLocaleParam.set_a  = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc)); */
    break;
  case THD_EVENT_RS_SYNC_B:
    pHandle->pAcquireParam->IsdbtLocaleParam.set_b = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC_B);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC_B);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC_B);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc));   */
    break;
  case THD_EVENT_RS_SYNC_C:
    pHandle->pAcquireParam->IsdbtLocaleParam.set_c = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC_C);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC_C);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC_C);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc)); */
    break;
  case THD_EVENT_RS_SYNC_LOSS:
    pHandle->pAcquireParam->IsdbtLocaleParam.clr_a = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc)); */
    break;
  case THD_EVENT_RS_SYNC_B_LOSS:
    pHandle->pAcquireParam->IsdbtLocaleParam.clr_b = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC_B);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC_B);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC_B);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc)); */
    break;
  case THD_EVENT_RS_SYNC_C_LOSS:
    pHandle->pAcquireParam->IsdbtLocaleParam.clr_c = 1;
/*  clean   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_NBERC_C);
    cberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_CBERC_C);
    uberc   = BREG_Read32(pHandle->hRegister, BCHP_THD_CORE_RS_UBERC_C);
    BDBG_WRN(("clean=%0d, cberc=%0d, uberc=%0d", clean, cberc, uberc));*/
    break;
  }

  /* On tuning, first try to lock all layers.  Then accept whatever will lock with the exception of 1-seg for layerA.  lock_acq is used from init to trk state */
  lock_acq0 = ( (pHandle->pAcquireParam->IsdbtLocaleParam.set_a || (segs_b + segs_c == 12))  &&  (!segs_b || pHandle->pAcquireParam->IsdbtLocaleParam.set_b)  &&  (!segs_c || pHandle->pAcquireParam->IsdbtLocaleParam.set_c) );
  lock_acq1 = ( (pHandle->pAcquireParam->IsdbtLocaleParam.set_a && (segs_b + segs_c != 12))  ||   (segs_b && pHandle->pAcquireParam->IsdbtLocaleParam.set_b)  ||  (segs_c && pHandle->pAcquireParam->IsdbtLocaleParam.set_c) );
  lock_acq  = (lock_acq0 && !pHandle->pAcquireParam->IsdbtLocaleParam.phase) || (lock_acq1 && pHandle->pAcquireParam->IsdbtLocaleParam.phase);

  clr_chk   = pHandle->pAcquireParam->IsdbtLocaleParam.clr_a || pHandle->pAcquireParam->IsdbtLocaleParam.clr_b || pHandle->pAcquireParam->IsdbtLocaleParam.clr_c;

  lock_chk  = ( (pHandle->pAcquireParam->IsdbtLocaleParam.set_a && (segs_b + segs_c != 12))  ||   (segs_b && pHandle->pAcquireParam->IsdbtLocaleParam.set_b)  ||  (segs_c && pHandle->pAcquireParam->IsdbtLocaleParam.set_c) );
/*  lock_chk = (lock_chk0 && !pHandle->pAcquireParam->IsdbtLocaleParam.phase) || (lock_chk1 && pHandle->pAcquireParam->IsdbtLocaleParam. phase); */

  BDBG_MSG(("lock_acq=%0x, lock_chk=%0x, set_a=%0d, set_b=%0d, set_c=%0d, clr_a=%0d, clr_b=%0d, clr_c=%0d, phase=%0d", lock_acq, lock_chk, pHandle->pAcquireParam->IsdbtLocaleParam.set_a, pHandle->pAcquireParam->IsdbtLocaleParam.set_b,
            pHandle->pAcquireParam->IsdbtLocaleParam.set_c,pHandle->pAcquireParam->IsdbtLocaleParam.clr_a, pHandle->pAcquireParam->IsdbtLocaleParam.clr_b, pHandle->pAcquireParam->IsdbtLocaleParam.clr_c, pHandle->pAcquireParam->IsdbtLocaleParam.phase));

  if (pHandle->pInternalAcquireParam->AllowRsSyncEvent) {
    if (lock_acq)
      BKNI_SetEvent(pHandle->hRsSyncEvent);
  }
  else {
    if (lock_chk) {
      pHandle->ThdLockStatus |= (1 << THD_LockStatusBit_SystemLock);
      if (pHandle->LockStatusTracking != ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)) {
        BDBG_MSG(("locked %0x",  ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)));
        pHandle->LockStatusTracking = ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1);
         if ( pHandle->pCallback[BTHD_Callback_eLockChange] != NULL )
          (pHandle->pCallback[BTHD_Callback_eLockChange])(pHandle->pCallbackParam[BTHD_Callback_eLockChange] );

      }
    }
    else if (clr_chk) {                   /* enabled @ state_checklock stage */
      pHandle->ThdLockStatus &= (0xffffffff ^ (1 << THD_LockStatusBit_SystemLock));
      BDBG_MSG(("loose lock. ThdLockStatus: %0x", pHandle->ThdLockStatus));
      /*BKNI_SetEvent(pHandle->hIntEvent);  you can do, it will creat a loop between Nexus and Pi and will not allow any THD call, let the timer check it*/
    }
    BTHD_P_IsdbtResetLockSetClrFlag(pHandle);
  }

return;
}
#endif

/***************************************************************************
 * BTHD_P_EventIsr()
 ***************************************************************************/
static void BTHD_P_EventIsr(void *p, int param)
{
  BTHD_3x7x_Handle pHandle = (BTHD_3x7x_Handle)p;

  if (pHandle->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On)
      return;

  switch(param) {
  case THD_EVENT_FFT_SYNC:
    BKNI_SetEvent(pHandle->hFwSyncEvent);
    break;
  case THD_EVENT_SP_SYNC:
    BKNI_SetEvent(pHandle->hSpSyncEvent);
    break;
  case THD_EVENT_TPS_SYNC:
    BKNI_SetEvent(pHandle->hTpsSyncEvent);
    break;
  case THD_EVENT_FEC_SYNC:
    BKNI_SetEvent(pHandle->hFecSyncEvent);
    break;
  case THD_EVENT_FW_CORR_MAX_RDY:
    BKNI_SetEvent(pHandle->hFwCorrMaxEvent);
    break;
#ifdef BTHD_ISDBT_SUPPORT
  case THD_EVENT_TMCC_SYNC:
    BKNI_SetEvent(pHandle->hTmccSyncEvent);
    break;
#endif
  case THD_EVENT_FBCNT_ZERO:
    BKNI_SetEvent(pHandle->hFbcntZeroEvent);
    break;
  case THD_EVENT_RS_SYNC:
    if (pHandle->pInternalAcquireParam->AllowRsSyncEvent)
      BKNI_SetEvent(pHandle->hRsSyncEvent);
    else
    {
      pHandle->ThdLockStatus |= (1 << THD_LockStatusBit_SystemLock);
      if (pHandle->LockStatusTracking != ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1))
      {
        BDBG_MSG(("locked %0x",  ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)));
        pHandle->LockStatusTracking = ((pHandle->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1);
        if ( pHandle->pCallback[BTHD_Callback_eLockChange] != NULL )
          (pHandle->pCallback[BTHD_Callback_eLockChange])(pHandle->pCallbackParam[BTHD_Callback_eLockChange] );

      }
    }
    break;
  case THD_EVENT_RS_SYNC_LOSS:
    pHandle->ThdLockStatus &= (0xffffffff ^ (1 << THD_LockStatusBit_SystemLock));
    /*BKNI_SetEvent(pHandle->hIntEvent); you can do, it will creat a loop between Nexus and Pi and will not allow any THD call, let the timer check it*/
    break;
  }
}
/******************************************************************************
 * BTHD_P_ResetEvent()
 ******************************************************************************/
static BERR_Code BTHD_P_ResetEvent(BTHD_3x7x_Handle h)
{
  /* BDBG_WRN(("ResetThdEvent")); */
  BKNI_ResetEvent(h->hFwCorrMaxEvent);
  BKNI_ResetEvent(h->hFwSyncEvent);
  BKNI_ResetEvent(h->hSpSyncEvent);
  BKNI_ResetEvent(h->hTpsSyncEvent);
  BKNI_ResetEvent(h->hFecSyncEvent);
  BKNI_ResetEvent(h->hFbcntZeroEvent);
  BKNI_ResetEvent(h->hRsSyncEvent);
#ifdef BTHD_ISDBT_SUPPORT
  BKNI_ResetEvent(h->hTmccSyncEvent);
#endif

  return BERR_SUCCESS;
}


/******************************************************************************
BTHD_P_BBSTimerFunc_isr()
******************************************************************************/
static BERR_Code BTHD_P_BBSTimerFunc_isr(BTHD_Handle h)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
  /*BDBG_MSG(("BTHD_P_BBSTimerFunc_isr")); */
  BKNI_SetEvent(p3x7x->hBBSInterruptEvent);
  return BERR_SUCCESS;
}


/******************************************************************************
BTHD_P_TimerFunc_isr()
******************************************************************************/
static BERR_Code BTHD_P_TimerFunc_isr(BTHD_Handle h)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
  /*BDBG_ERR(("BTHD_P_TimerFunc_isr")); */
  BKNI_SetEvent(p3x7x->hIntEvent);
  return BERR_SUCCESS;
}


/******************************************************************************
 * BTHD_Open()
 ******************************************************************************/
BERR_Code BTHD_3x7x_Open(   BTHD_Handle *pthd,       /* [out] BTHD handle */
                            BCHP_Handle hChip,    /* [in] chip handle */
                            void* hRegister,              /* Register handle */
                            BINT_Handle hInterrupt,             /* Interrupt handle */
                            const BTHD_Settings *pDefSettings /* [in] default settings */
                       )
{
  BERR_Code retCode;
  BTHD_Handle hDev;
  BTHD_3x7x_P_Handle *p3x7x=NULL;
  uint32_t BufSrc;
  void *cached_ptr, *tmpAddress;
  BDBG_ASSERT(hInterrupt);
  BDBG_ASSERT(hChip);
  BDBG_ASSERT(hRegister);
  BDBG_ASSERT(pDefSettings);
  BDBG_ASSERT(pDefSettings->hTmr);

  /* allocate memory for the handle */
  hDev = (BTHD_Handle)BKNI_Malloc(sizeof(BTHD_P_Handle));
  if( hDev == NULL )
  {
     retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
     BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
     goto done;
  }
  BKNI_Memset(hDev, 0, sizeof(BTHD_P_Handle));
  BKNI_Memcpy(&hDev->settings, pDefSettings, sizeof(*pDefSettings));
   /* allocate memory for the handle */
  p3x7x = (BTHD_3x7x_P_Handle *)BKNI_Malloc(sizeof(BTHD_3x7x_P_Handle));
  if( p3x7x == NULL )
  {
     retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
     BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
     goto done;
  }

  BKNI_Memset(p3x7x, 0, sizeof(BTHD_3x7x_P_Handle));
  hDev->pImpl = (void*)p3x7x;
  BDBG_MSG(("BTHD_3x7x_Open"));
  /* initialize our handle */
  p3x7x->hRegister = (BREG_Handle)hRegister;
  p3x7x->hChip = hChip;
  p3x7x->hInterrupt = hInterrupt;
#ifdef BCHP_PWR_RESOURCE_THD_TOP
    BCHP_PWR_AcquireResource(p3x7x->hChip, BCHP_PWR_RESOURCE_THD_TOP);
#endif

  /*BREG_AtomicUpdate32(p3x7x->hRegister, BCHP_SUN_TOP_CTRL_SW_RESET,        BCHP_MASK(SUN_TOP_CTRL_SW_RESET, thd_sw_reset),         BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_RESET, thd_sw_reset, 1));
  BREG_AtomicUpdate32(p3x7x->hRegister, BCHP_SUN_TOP_CTRL_SW_RESET,          BCHP_MASK(SUN_TOP_CTRL_SW_RESET, thd_sw_reset),         BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_RESET, thd_sw_reset, 0));
*/

#ifdef BTHD_ISDBT_SUPPORT
 if (hDev->settings.supportIsdbt)
  {
    tmpAddress = (uint32_t *)BMEM_AllocAligned(hDev->settings.hHeap, 2334720, 3, 0 );
    if ( !tmpAddress )
    {
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto done;
    }
    BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
    BKNI_Memset(cached_ptr, 0x00, 2334720 );
    p3x7x->pIsdbtMemory = cached_ptr;
    BMEM_FlushCache(hDev->settings.hHeap, p3x7x->pIsdbtMemory, 2334720);
    BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
    BufSrc = BufSrc >> 3;
    BREG_Write32(p3x7x->hRegister,  BCHP_THD_CORE_SCB_ADDR_OFFSET, BufSrc);
  }
#endif
  tmpAddress = (THD_3x7x_AcquireParam_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(THD_3x7x_AcquireParam_t), 0, 0 );
  if (tmpAddress == NULL) {
    retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BDBG_ERR(("BERR_OUT_OF_SYSTEM_MEMORY"));
    goto done;
  }
  BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
  BKNI_Memset(cached_ptr, 0x00, sizeof( THD_3x7x_AcquireParam_t ) );
  p3x7x->pAcquireParam = cached_ptr;
  BMEM_FlushCache(hDev->settings.hHeap, p3x7x->pAcquireParam, sizeof( THD_3x7x_AcquireParam_t ) );
  BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
  BREG_Write32(p3x7x->hRegister, BCHP_THD_CORE_SWSPARE1, BufSrc);

#ifdef BTHD_ISDBT_SUPPORT
  p3x7x->pAcquireParam->IsdbtLocaleParam.set_a = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.set_b = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.set_c = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.clr_a = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.clr_b = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.clr_c = 0;
  p3x7x->pAcquireParam->IsdbtLocaleParam.phase = 0;
#endif
  tmpAddress = (THD_3x7x_Status_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(THD_3x7x_Status_t), 0, 0 );
  if (tmpAddress == NULL) {
    retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BDBG_ERR(("BERR_OUT_OF_SYSTEM_MEMORY"));
    goto done;
  }
  BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
  BKNI_Memset(cached_ptr, 0x00, sizeof( THD_3x7x_Status_t ) );
  p3x7x->pStatus = cached_ptr;
  BMEM_FlushCache(hDev->settings.hHeap, p3x7x->pStatus, sizeof( THD_3x7x_Status_t ) );
  BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
  BREG_Write32(p3x7x->hRegister, BCHP_THD_CORE_SWSPARE4, BufSrc);

  tmpAddress = (THD_InternalAcquireParam_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(THD_InternalAcquireParam_t), 0, 0 );
  if (tmpAddress == NULL) {
    retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BDBG_ERR(("BERR_OUT_OF_SYSTEM_MEMORY"));
    goto done;
  }
  BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
  BKNI_Memset(cached_ptr, 0x00, sizeof( THD_InternalAcquireParam_t ) );
  p3x7x->pInternalAcquireParam = cached_ptr;
  BMEM_FlushCache(hDev->settings.hHeap, p3x7x->pInternalAcquireParam, sizeof( THD_InternalAcquireParam_t ) );
  BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
  BREG_Write32(p3x7x->hRegister, BCHP_THD_CORE_SWSPARE5, BufSrc);

   tmpAddress = (THD_3x7x_ConfigParam_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(THD_3x7x_ConfigParam_t), 0, 0 );
  if (tmpAddress == NULL) {
    retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BDBG_ERR(("BERR_OUT_OF_SYSTEM_MEMORY"));
    goto done;
  }
  BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
  BKNI_Memset(cached_ptr, 0x00, sizeof( THD_3x7x_ConfigParam_t ) );
  p3x7x->pConfigParam = cached_ptr;
  BMEM_FlushCache(hDev->settings.hHeap, p3x7x->pConfigParam, sizeof( THD_3x7x_ConfigParam_t ) );

  BDBG_MSG(("THD:Creating THD Core interrupts"));

  /* THD interrupts */
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hFwCorrMaxCallback), hInterrupt,
    BCHP_INT_ID_FW_CORR_MAX_RDY_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_FW_CORR_MAX_RDY ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hFwSyncCallback), hInterrupt,
    BCHP_INT_ID_FW_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_FFT_SYNC));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hSpSyncCallback), hInterrupt,
    BCHP_INT_ID_SP_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_SP_SYNC ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hTpsSyncCallback), hInterrupt,
    BCHP_INT_ID_TPS_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_TPS_SYNC ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hFecSyncCallback), hInterrupt,
    BCHP_INT_ID_FEC_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_FEC_SYNC ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hFbcCntCallback), hInterrupt,
    BCHP_INT_ID_FBCNT_ZERO_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_FBCNT_ZERO ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncCallback), hInterrupt,
    BCHP_INT_ID_RS_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_RS_SYNC ));
  BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncLossCallback), hInterrupt,
    BCHP_INT_ID_RS_SYNC_LOSS_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_RS_SYNC_LOSS ));
#ifdef BTHD_ISDBT_SUPPORT
    BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hTmccSyncCallback), hInterrupt,
        BCHP_INT_ID_TMCC_SYNC_INTR, BTHD_P_EventIsr, (void*)p3x7x, THD_EVENT_TMCC_SYNC ));
    BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncBCallback), hInterrupt,
        BCHP_INT_ID_RS_SYNC_B_INTR, BTHD_P_EventIsdbtIsr, (void*)p3x7x, THD_EVENT_RS_SYNC_B ));
    BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncCCallback), hInterrupt,
        BCHP_INT_ID_RS_SYNC_C_INTR, BTHD_P_EventIsdbtIsr, (void*)p3x7x, THD_EVENT_RS_SYNC_C ));
    BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncLossBCallback), hInterrupt,
        BCHP_INT_ID_RS_SYNC_LOSS_B_INTR, BTHD_P_EventIsdbtIsr, (void*)p3x7x, THD_EVENT_RS_SYNC_B_LOSS ));
    BTHD_CHK_RETCODE(BINT_CreateCallback( &(p3x7x->hRsSyncLossCCallback), hInterrupt,
        BCHP_INT_ID_RS_SYNC_LOSS_C_INTR, BTHD_P_EventIsdbtIsr, (void*)p3x7x, THD_EVENT_RS_SYNC_C_LOSS ));
#endif


  p3x7x->LockStatusTracking = 0xFFFFFFFF;

  /* create events */
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hFwCorrMaxEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hFwSyncEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hSpSyncEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hTpsSyncEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hFecSyncEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hFbcntZeroEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hRsSyncEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hIntEvent)));
#ifdef BTHD_ISDBT_SUPPORT
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hTmccSyncEvent)));
#endif
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hEWSEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hLockEvent)));
  BTHD_CHK_RETCODE(BKNI_CreateEvent(&(p3x7x->hBBSInterruptEvent)));

  *pthd = hDev;
done:
#ifdef BCHP_PWR_RESOURCE_THD_TOP
    BCHP_PWR_ReleaseResource(p3x7x->hChip, BCHP_PWR_RESOURCE_THD_TOP);
#endif
   if( retCode != BERR_SUCCESS )   /* clean up the memory free */
    {
       if (p3x7x->pInternalAcquireParam != NULL)
            BMEM_Heap_FreeCached(hDev->settings.hHeap, p3x7x->pInternalAcquireParam);
       if (p3x7x->pConfigParam  != NULL)
            BMEM_Heap_FreeCached(hDev->settings.hHeap, p3x7x->pConfigParam);
       if ( p3x7x->pStatus  != NULL)
            BMEM_Heap_FreeCached(hDev->settings.hHeap, p3x7x->pStatus);
        if ( p3x7x->pStatus  != NULL)
            BMEM_Heap_FreeCached(hDev->settings.hHeap, p3x7x->pAcquireParam);
        if ( p3x7x->pStatus  != NULL)
            BMEM_Heap_FreeCached(hDev->settings.hHeap, p3x7x->pAcquireParam);
#ifdef BTHD_ISDBT_SUPPORT
        if ( p3x7x->pIsdbtMemory  != NULL)
            BMEM_Free(hDev->settings.hHeap, p3x7x->pIsdbtMemory);
#endif
        if( p3x7x != NULL )
              BKNI_Free( p3x7x );
         if( hDev != NULL )
              BKNI_Free( hDev );

        *pthd = NULL;
    }
  return retCode;
}
/******************************************************************************
 * BTHD_Close()
 ******************************************************************************/
BERR_Code BTHD_3x7x_Close(BTHD_Handle h)
{

  BTHD_3x7x_P_Handle *p3x7x;
  BTHD_Settings DefSettings;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
  DefSettings = h->settings;


   BREG_WriteField(p3x7x->hRegister, THD_CORE_GLB, BE_MODE, 0 );               /* stop the ISDB-T dump to memory*/


  BINT_DisableCallback( p3x7x->hFwCorrMaxCallback);
  BINT_DisableCallback( p3x7x->hFwSyncCallback);
  BINT_DisableCallback( p3x7x->hSpSyncCallback);
  BINT_DisableCallback( p3x7x->hTpsSyncCallback);
  BINT_DisableCallback( p3x7x->hFecSyncCallback);
  BINT_DisableCallback( p3x7x->hFbcCntCallback);
  BINT_DisableCallback( p3x7x->hRsSyncCallback);
  BINT_DisableCallback( p3x7x->hRsSyncLossCallback);
#ifdef BTHD_ISDBT_SUPPORT
  BINT_DisableCallback( p3x7x->hTmccSyncCallback);
  BINT_DisableCallback( p3x7x->hRsSyncBCallback);
  BINT_DisableCallback( p3x7x->hRsSyncCCallback);
  BINT_DisableCallback( p3x7x->hRsSyncLossBCallback);
  BINT_DisableCallback( p3x7x->hRsSyncLossCCallback);
#endif
  /* since those events might already be deleted by ResetEvent function */
  BKNI_DestroyEvent(p3x7x->hFwCorrMaxEvent);
  BKNI_DestroyEvent(p3x7x->hFwSyncEvent);
  BKNI_DestroyEvent(p3x7x->hSpSyncEvent);
  BKNI_DestroyEvent(p3x7x->hTpsSyncEvent);
  BKNI_DestroyEvent(p3x7x->hFecSyncEvent);
  BKNI_DestroyEvent(p3x7x->hFbcntZeroEvent);
  BKNI_DestroyEvent(p3x7x->hRsSyncEvent);
  BKNI_DestroyEvent(p3x7x->hIntEvent);
  BKNI_DestroyEvent(p3x7x->hBBSInterruptEvent);
  BKNI_DestroyEvent(p3x7x->hLockEvent);
  BKNI_DestroyEvent(p3x7x->hEWSEvent);
#ifdef BTHD_ISDBT_SUPPORT
  BKNI_DestroyEvent(p3x7x->hTmccSyncEvent);
#endif


#ifdef BTHD_ISDBT_SUPPORT
  if (p3x7x->pIsdbtMemory != NULL)
     BMEM_Heap_FreeCached(DefSettings.hHeap, p3x7x->pIsdbtMemory);
#endif
  if (p3x7x->pAcquireParam != NULL)
    BMEM_Heap_FreeCached(DefSettings.hHeap, p3x7x->pAcquireParam);
  if (p3x7x->pInternalAcquireParam != NULL)
    BMEM_Heap_FreeCached(DefSettings.hHeap, p3x7x->pInternalAcquireParam);
  if (p3x7x->pConfigParam  != NULL)
    BMEM_Heap_FreeCached(DefSettings.hHeap, p3x7x->pConfigParam);
  if (p3x7x->pStatus != NULL)
    BMEM_Heap_FreeCached(DefSettings.hHeap, p3x7x->pStatus);
  if (p3x7x != NULL)
    BKNI_Free((void*)p3x7x);
  if (h != NULL)
    BKNI_Free((void*)h);

  return BERR_SUCCESS;
}

/***************************************************************************
 * BTHD_InitializeParams()
 ***************************************************************************/
BERR_Code BTHD_3x7x_InitializeParams(BTHD_Handle h, const uint8_t *var1, uint32_t var2)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
  BSTD_UNUSED(var1);
  BSTD_UNUSED(var2);

  /* General Parameters (Public) */
  p3x7x->pAcquireParam->CommonAcquireParam.Standard          = THD_Standard_DVBT;
  p3x7x->pAcquireParam->CommonAcquireParam.Bandwidth         = THD_Bandwidth_8MHz;
  p3x7x->pAcquireParam->CommonAcquireParam.CenterFreq        = 0;
  p3x7x->pAcquireParam->CommonAcquireParam.TransGuardMode    = THD_TransGuardMode_Auto_DVBT;
  p3x7x->pAcquireParam->CommonAcquireParam.TransmissionMode  = THD_TransmissionMode_2k;
  p3x7x->pAcquireParam->CommonAcquireParam.GuardInterval     = THD_GuardInterval_1_32;
  p3x7x->pAcquireParam->CommonAcquireParam.CoChannelMode     = THD_CoChannelMode_Auto;
  p3x7x->pAcquireParam->CommonAcquireParam.CarrierRange      = THD_CarrierRange_Narrow;

  /* General Paramters (Private) */
  p3x7x->pInternalAcquireParam->FrontEndMode           = THD_FrontEndMode_Baseband;
  p3x7x->pInternalAcquireParam->ChannelEstimatorMode   = THD_ChannelEstimatorMode_Auto;
  p3x7x->pInternalAcquireParam->FFTWindowMode          = THD_FFTWindowMode_Auto;
  p3x7x->pInternalAcquireParam->ImpulseMode            = THD_ImpulseMode_None;
  p3x7x->pInternalAcquireParam->SampleFreq             = 54000000;
  p3x7x->pInternalAcquireParam->AllowRsSyncEvent       = false;
  p3x7x->pInternalAcquireParam->TransGuardMaxThreshold            = 0x00050000;
  p3x7x->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[0] = 296;
  p3x7x->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[1] = 210;
  p3x7x->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[2] = 150;
  p3x7x->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[3] = 104;

  /* DVB-T Specific Parameters (Public) */
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.TPSMode       = THD_DvbtTPSMode_Auto;
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode  = THD_DvbtPriorityMode_High;
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.Qam           = THD_Qam_16Qam;
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy     = THD_DvbtHierarchy_None;
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP    = THD_CodeRate_3_4;
  p3x7x->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP    = THD_CodeRate_1_2;
#ifdef BTHD_ISDBT_SUPPORT
  /* ISDB-T Specific Parameters (Public) */
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.TMCCMode     = THD_IsdbtTMCCMode_Auto;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Pr           = THD_IsdbtPr_Disable;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[0]       = THD_Qam_16Qam;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[1]       = THD_Qam_16Qam;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[2]       = THD_Qam_16Qam;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_1_2;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_1_2;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_1_2;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[0]   = THD_IsdbtTimeInt_0X;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[1]   = THD_IsdbtTimeInt_0X;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[2]   = THD_IsdbtTimeInt_0X;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[0]  = 13;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[1]  = 0;
  p3x7x->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[2]  = 0;
#endif
  return BERR_SUCCESS;
}


/***************************************************************************
* BTHD_P_AppDVBTSettings()
***************************************************************************/
static void BTHD_P_AppDVBTSettings(BTHD_3x7x_Handle h, const BTHD_InbandParams *pParams)
{
  BDBG_ASSERT(h);
  BDBG_ASSERT(pParams);


  BDBG_MSG(("BTHD_P_AppDVBTSettings"));

  BKNI_ResetEvent(h->hRsSyncEvent);
  if (h->hRsSyncCallback != (BINT_CallbackHandle)NULL)
  {
    BINT_DisableCallback(h->hRsSyncCallback);
    BINT_DestroyCallback(h->hRsSyncCallback);
    h->hRsSyncCallback = (BINT_CallbackHandle)NULL;
  }
  if (h->hRsSyncLossCallback != (BINT_CallbackHandle)NULL)
  {
    BINT_DisableCallback(h->hRsSyncLossCallback);
    BINT_DestroyCallback(h->hRsSyncLossCallback);
    h->hRsSyncLossCallback = (BINT_CallbackHandle)NULL;
   }

  BINT_CreateCallback( &(h->hRsSyncCallback), h->hInterrupt,
    BCHP_INT_ID_RS_SYNC_INTR, BTHD_P_EventIsr, (void*)h, THD_EVENT_RS_SYNC );
  BINT_CreateCallback( &(h->hRsSyncLossCallback), h->hInterrupt,
    BCHP_INT_ID_RS_SYNC_LOSS_INTR, BTHD_P_EventIsr, (void*)h, THD_EVENT_RS_SYNC_LOSS );
  BINT_DisableCallback( h->hRsSyncCallback);
  BINT_DisableCallback( h->hRsSyncLossCallback);


  switch (pParams->decodeMode)
  {
  case BTHD_Decode_Lp:
   h->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode  = THD_DvbtPriorityMode_Low;
    break;
  default:
    BDBG_WRN(("Unrecognized PriorityMode setting - defaulting to THD_PriorityMode_High"));
    /* fall through */
  case BTHD_Decode_Hp:
   h->pAcquireParam->StdAcquire.DvbtAcquireParam.PriorityMode  = THD_DvbtPriorityMode_High;
    break;
  }
  if (pParams->bTpsAcquire == false)
  {
   h->pAcquireParam->StdAcquire.DvbtAcquireParam.TPSMode = THD_DvbtTPSMode_Manual;

    switch (pParams->eModulation)
    {
    case BTHD_Modulation_eQpsk:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Qam  = THD_Qam_Qpsk;
      break;
    case BTHD_Modulation_e16Qam:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Qam  = THD_Qam_16Qam;
      break;
    default:
      BDBG_WRN(("Unrecognized modulaton setting - defaulting to THD_Qam_64Qam"));
      /* fall through */
    case BTHD_Modulation_e64Qam:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Qam  = THD_Qam_64Qam;
      break;
    }

    switch (pParams->eCodeRateHP)
    {
    case BTHD_CodeRate_e2_3:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP  = THD_CodeRate_2_3;
      break;
    case BTHD_CodeRate_e3_4:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP  = THD_CodeRate_3_4;
      break;
    case BTHD_CodeRate_e5_6:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP  = THD_CodeRate_5_6;
      break;
    case BTHD_CodeRate_e7_8:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP  = THD_CodeRate_7_8;
      break;
    default:
      BDBG_WRN(("Unrecognized CodeRateHP setting - defaulting to BTHD_CodeRate_e1_2"));
      /* fall through */
    case BTHD_CodeRate_e1_2:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateHP  = THD_CodeRate_1_2;
      break;
    }

    switch (pParams->eCodeRateLP)
    {
    case BTHD_CodeRate_e2_3:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP  = THD_CodeRate_2_3;
      break;
    case BTHD_CodeRate_e3_4:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP  = THD_CodeRate_3_4;
      break;
    case BTHD_CodeRate_e5_6:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP  = THD_CodeRate_5_6;
      break;
    case BTHD_CodeRate_e7_8:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP  = THD_CodeRate_7_8;
      break;
    default:
      BDBG_WRN(("Unrecognized CodeRateLP setting - defaulting to BTHD_CodeRate_e1_2"));
      /* fall through */
    case BTHD_CodeRate_e1_2:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.CodeRateLP  = THD_CodeRate_1_2;
      break;
    }


    switch (pParams->eHierarchy)
    {
    case BTHD_Hierarchy_1:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy  = THD_DvbtHierarchy_1;
      break;
    case BTHD_Hierarchy_2:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy  = THD_DvbtHierarchy_2;
      break;
    case BTHD_Hierarchy_4:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy  = THD_DvbtHierarchy_4;
      break;
    default:
      BDBG_WRN(("Unrecognized Hierarchy setting - defaulting to THD_DvbtHierarchy_None"));
      /* fall through */
    case BTHD_Hierarchy_0:
     h->pAcquireParam->StdAcquire.DvbtAcquireParam.Hierarchy  = THD_DvbtHierarchy_None;
      break;
    }
  }
  else
   h->pAcquireParam->StdAcquire.DvbtAcquireParam.TPSMode = THD_DvbtTPSMode_Auto;

  return;
}
/***************************************************************************
* BTHD_P_AppISDBTSettings()
***************************************************************************/
#ifdef BTHD_ISDBT_SUPPORT
static void BTHD_P_AppISDBTSettings(BTHD_3x7x_Handle h, const BTHD_InbandParams *pParams)
{

  BDBG_ASSERT(h);
  BDBG_ASSERT(pParams);
  BDBG_MSG(("BTHD_P_AppISDBTSettings"));

  BKNI_ResetEvent(h->hRsSyncEvent);
  if (h->hRsSyncCallback != (BINT_CallbackHandle)NULL)
  {
    BINT_DisableCallback(h->hRsSyncCallback);
    BINT_DestroyCallback(h->hRsSyncCallback);
    h->hRsSyncCallback = (BINT_CallbackHandle)NULL;
  }
  if (h->hRsSyncLossCallback != (BINT_CallbackHandle)NULL)
  {
    BINT_DisableCallback(h->hRsSyncLossCallback);
    BINT_DestroyCallback(h->hRsSyncLossCallback);
    h->hRsSyncLossCallback = (BINT_CallbackHandle)NULL;
   }

 BINT_CreateCallback( &(h->hRsSyncCallback), h->hInterrupt,
    BCHP_INT_ID_RS_SYNC_INTR, BTHD_P_EventIsdbtIsr, (void*)h, THD_EVENT_RS_SYNC );
  BINT_CreateCallback( &(h->hRsSyncLossCallback), h->hInterrupt,
    BCHP_INT_ID_RS_SYNC_LOSS_INTR, BTHD_P_EventIsdbtIsr, (void*)h, THD_EVENT_RS_SYNC_LOSS );
  BINT_DisableCallback( h->hRsSyncCallback);
  BINT_DisableCallback(h->hRsSyncBCallback);
  BINT_DisableCallback(h->hRsSyncCCallback);
  BINT_DisableCallback( h->hRsSyncLossCallback);
  BINT_DisableCallback( h->hRsSyncLossBCallback);
  BINT_DisableCallback( h->hRsSyncLossCCallback);


  if (pParams->bTmccAcquire == false)
  {
   h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TMCCMode = THD_IsdbtTMCCMode_Manual;

    switch (pParams->bTmccAcquire)
    {
    case 0:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Pr  = THD_IsdbtPr_Disable;
      break;
    default:
      BDBG_WRN(("Unrecognized TmccAcquire setting - defaulting to THD_Pr_Enable"));
       /* fall through */
    case 1:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Pr  = THD_IsdbtPr_Enable;
      break;
    }

    switch (pParams->eIsdbtAModulation)
    {
    case BTHD_Modulation_eQpsk:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[0]  = THD_Qam_Qpsk;
      break;
    case BTHD_Modulation_e64Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[0]  = THD_Qam_64Qam;
      break;
    default:
      BDBG_WRN(("Unrecognized A modulaton setting - defaulting to THD_Qam_16Qam"));
      /* fall through */
    case BTHD_Modulation_e16Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[0]  = THD_Qam_16Qam;
      break;
    }

    switch (pParams->eIsdbtACodeRate)
    {
    case BTHD_CodeRate_e2_3:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_2_3;
      break;
    case BTHD_CodeRate_e3_4:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_3_4;
      break;
    case BTHD_CodeRate_e5_6:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_5_6;
      break;
    case BTHD_CodeRate_e7_8:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_7_8;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_CodeRate setting - defaulting to BTHD_CodeRate_e1_2"));
      /* fall through */
    case BTHD_CodeRate_e1_2:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[0]  = THD_CodeRate_1_2;
      break;
    }

    switch (pParams->eIsdbtATimeInterleaving)
    {
    case BTHD_IsdbtTimeInterleaving_1X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[0]  = THD_IsdbtTimeInt_1X;
      break;
    case BTHD_IsdbtTimeInterleaving_2X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[0]  = THD_IsdbtTimeInt_2X;
      break;
    case BTHD_IsdbtTimeInterleaving_3X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[0]  = THD_IsdbtTimeInt_4X;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_TimeInt setting - defaulting to THD_TimeInt_0X"));
      /* fall through */
    case BTHD_IsdbtTimeInterleaving_0X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[0]  = THD_IsdbtTimeInt_0X;
      break;
    }
   h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[0] = pParams->eIsdbtASegments;

    /********************************** B ****************************/
    switch (pParams->eIsdbtBModulation)
    {
     case BTHD_Modulation_eQpsk:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[1]  = THD_Qam_Qpsk;
      break;
    case BTHD_Modulation_e64Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[1]  = THD_Qam_64Qam;
      break;
    default:
      BDBG_WRN(("Unrecognized A modulaton setting - defaulting to THD_Qam_16Qam"));
      /* fall through */
    case BTHD_Modulation_e16Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[1]  = THD_Qam_16Qam;
      break;
    }

    switch (pParams->eIsdbtBCodeRate)
    {
    case BTHD_CodeRate_e2_3:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_2_3;
      break;
    case BTHD_CodeRate_e3_4:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_3_4;
      break;
    case BTHD_CodeRate_e5_6:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_5_6;
      break;
    case BTHD_CodeRate_e7_8:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_7_8;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_CodeRate setting - defaulting to BTHD_CodeRate_e1_2"));
      /* fall through */
    case BTHD_CodeRate_e1_2:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[1]  = THD_CodeRate_1_2;
      break;
    }

    switch (pParams->eIsdbtBTimeInterleaving)
    {
    case BTHD_IsdbtTimeInterleaving_1X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[1]  = THD_IsdbtTimeInt_1X;
      break;
    case BTHD_IsdbtTimeInterleaving_2X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[1]  = THD_IsdbtTimeInt_2X;
      break;
    case BTHD_IsdbtTimeInterleaving_3X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[1]  = THD_IsdbtTimeInt_4X;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_TimeInt setting - defaulting to THD_TimeInt_0X"));
      /* fall through */
    case BTHD_IsdbtTimeInterleaving_0X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[1]  = THD_IsdbtTimeInt_0X;
      break;
    }

   h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[1] = pParams->eIsdbtBSegments;
    /********************************** C ****************************/
    switch (pParams->eIsdbtCModulation)
    {
     case BTHD_Modulation_eQpsk:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[2]  = THD_Qam_Qpsk;
      break;
    case BTHD_Modulation_e64Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[2]  = THD_Qam_64Qam;
      break;
    default:
      BDBG_WRN(("Unrecognized A modulaton setting - defaulting to THD_Qam_16Qam"));
      /* fall through */
    case BTHD_Modulation_e16Qam:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Qam[2]  = THD_Qam_16Qam;
      break;
    }

    switch (pParams->eIsdbtCCodeRate)
    {
    case BTHD_CodeRate_e2_3:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_2_3;
      break;
    case BTHD_CodeRate_e3_4:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_3_4;
      break;
    case BTHD_CodeRate_e5_6:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_5_6;
      break;
    case BTHD_CodeRate_e7_8:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_7_8;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_CodeRate setting - defaulting to BTHD_CodeRate_e1_2"));
      /* fall through */
    case BTHD_CodeRate_e1_2:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.CodeRate[2]  = THD_CodeRate_1_2;
      break;
    }

    switch (pParams->eIsdbtCTimeInterleaving)
    {
    case BTHD_IsdbtTimeInterleaving_1X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[2]  = THD_IsdbtTimeInt_1X;
      break;
    case BTHD_IsdbtTimeInterleaving_2X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[2]  = THD_IsdbtTimeInt_2X;
      break;
    case BTHD_IsdbtTimeInterleaving_3X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[2]  = THD_IsdbtTimeInt_4X;
      break;
    default:
      BDBG_WRN(("Unrecognized ISDBT_A_TimeInt setting - defaulting to THD_TimeInt_0X"));
      /* fall through */
    case BTHD_IsdbtTimeInterleaving_0X:
     h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TimeInt[2]  = THD_IsdbtTimeInt_0X;
      break;
    }

   h->pAcquireParam->StdAcquire.IsdbtAcquireParam.Segments[2] = pParams->eIsdbtCSegments;
  }
  else
   h->pAcquireParam->StdAcquire.IsdbtAcquireParam.TMCCMode = THD_IsdbtTMCCMode_Auto;

  return;
}

#endif
/*******************************************************************************
*   BTHD_SetAcqSettings
*******************************************************************************/
static BERR_Code BTHD_P_SetAcqSettings(BTHD_3x7x_Handle h, const BTHD_InbandParams *pParams)
{
    BERR_Code eResult = BERR_SUCCESS;
    BDBG_MSG(("BTHD_3x7x_SetAcqSettings"));

  /* Clear and set Acquisition Start Mode bits */
  h->pAcquireParam->CommonAcquireParam.AcquireMode &= (~THD_AcquireMode_Auto & ~THD_AcquireMode_Scan & ~THD_AcquireMode_Manual);
  switch ( pParams->eAcquisitionMode )
  {
  case BTHD_ThdAcquisitionMode_eManual:
    break;
  case BTHD_ThdAcquisitionMode_eScan:
    h->pAcquireParam->CommonAcquireParam.AcquireMode  |= THD_AcquireMode_Scan;
    break;
  default:
    BDBG_WRN(("Unrecognized mode setting - defaulting to THD_AcquireMode_Auto"));
    /* fall through */
  case BTHD_ThdAcquisitionMode_eAuto:
    h->pAcquireParam->CommonAcquireParam.AcquireMode  |= THD_AcquireMode_Auto;
    break;
  }

  switch ( pParams->mode )
  {
  case BTHD_InbandMode_eIsdbt:
    h->pAcquireParam->CommonAcquireParam.Standard  = THD_Standard_ISDBT;
    break;
  default:
    BDBG_WRN(("Unrecognized mode setting - defaulting to THD_Standard_DVBT"));
    /* fall through */
  case BTHD_InbandMode_eDvbt:
    h->pAcquireParam->CommonAcquireParam.Standard  = THD_Standard_DVBT;
    break;
  }

  switch ( pParams->bandwidth )
  {
  case BTHD_Bandwidth_5Mhz:
    h->pAcquireParam->CommonAcquireParam.Bandwidth  = THD_Bandwidth_5MHz;
    break;
  case BTHD_Bandwidth_6Mhz:
    h->pAcquireParam->CommonAcquireParam.Bandwidth  = THD_Bandwidth_6MHz;
    break;
  case BTHD_Bandwidth_7Mhz:
    h->pAcquireParam->CommonAcquireParam.Bandwidth  = THD_Bandwidth_7MHz;
    break;
  default:
    BDBG_WRN(("Unrecognized bandwidth setting - defaulting to THD_Bandwidth_8MHz"));
    /* fall through */
  case BTHD_Bandwidth_8Mhz:
    h->pAcquireParam->CommonAcquireParam.Bandwidth  = THD_Bandwidth_8MHz;
    break;
  }
  h->pAcquireParam->CommonAcquireParam.CenterFreq     = pParams->ifFreq;
  switch (pParams->cciMode)
  {
  case BTHD_CCI_None:
    h->pAcquireParam->CommonAcquireParam.CoChannelMode  = THD_CoChannelMode_None;
    break;
  default:
    BDBG_WRN(("Unrecognized CoChannelMode setting - defaulting to THD_CoChannelMode_Auto"));
    /* fall through */
  case BTHD_CCI_Auto:
    h->pAcquireParam->CommonAcquireParam.CoChannelMode  = THD_CoChannelMode_Auto;
    break;
  }

  switch (pParams->ePullinRange)
  {
  case BTHD_PullInRange_eNarrow:
    h->pAcquireParam->CommonAcquireParam.CarrierRange  = THD_CarrierRange_Narrow;
    break;
  default:
    BDBG_WRN(("Unrecognized CarrierRange setting - defaulting to THD_CarrierRange_Wide"));
    /* fall through */
  case BTHD_PullInRange_eWide:
    h->pAcquireParam->CommonAcquireParam.CarrierRange  = THD_CarrierRange_Wide;
    break;
  }

  switch (pParams->eModeGuardAcquire)
  {
    case BTHD_ModeGuard_eManual:
        h->pAcquireParam->CommonAcquireParam.TransGuardMode = THD_TransGuardMode_Manual;
        switch (pParams->eTransmissionMode)
        {
            case BTHD_TransmissionMode_e4K:
                h->pAcquireParam->CommonAcquireParam.TransmissionMode  = THD_TransmissionMode_4k;
                break;
            case BTHD_TransmissionMode_e8K:
                h->pAcquireParam->CommonAcquireParam.TransmissionMode  = THD_TransmissionMode_8k;
                break;
            default:
                BDBG_WRN(("Unrecognized TransmissionMode setting - defaulting to BTHD_TransmissionMode_e2K"));
                /* fall through */
                case BTHD_TransmissionMode_e2K:
                h->pAcquireParam->CommonAcquireParam.TransmissionMode  = THD_TransmissionMode_2k;
                break;
        }
        switch (pParams->eGuardInterval)
        {
            case BTHD_GuardInterval_e1_32:
                h->pAcquireParam->CommonAcquireParam.GuardInterval  = THD_GuardInterval_1_32;
                break;
            case BTHD_GuardInterval_e1_16:
                h->pAcquireParam->CommonAcquireParam.GuardInterval  = THD_GuardInterval_1_16;
                break;
            case BTHD_GuardInterval_e1_8:
                h->pAcquireParam->CommonAcquireParam.GuardInterval  = THD_GuardInterval_1_8;
                break;
            default:
                BDBG_WRN(("Unrecognized GuardInterval setting - defaulting to THD_GuardInterval_1_4"));
                /* fall through */
            case BTHD_GuardInterval_e1_4:
                h->pAcquireParam->CommonAcquireParam.GuardInterval  = THD_GuardInterval_1_4;
                break;
        }
        break;
    case BTHD_ModeGuard_eAutoDvbt:
        h->pAcquireParam->CommonAcquireParam.TransGuardMode  = THD_TransGuardMode_Auto_DVBT;
        break;
    case BTHD_ModeGuard_eAutoIsdbt:
        h->pAcquireParam->CommonAcquireParam.TransGuardMode  = THD_TransGuardMode_Auto_ISDBT;
        break;
    default:
        BDBG_WRN(("Unrecognized TransGuardMode setting - defaulting to THD_TransGuardMode_Auto"));
        /* fall through */
    case BTHD_ModeGuard_eAuto:
        h->pAcquireParam->CommonAcquireParam.TransGuardMode  = THD_TransGuardMode_Auto;
        break;
  }

    h->pConfigParam->TSMode  = THD_TSMode_Serial;

    if (h->pAcquireParam->CommonAcquireParam.Standard  == THD_Standard_DVBT)
  {
    BDBG_MSG(("BTHD_P_TuneAcquire = THD_Standard_DVBT"));
    BTHD_P_AppDVBTSettings(h, pParams);
  }
  else
  {
    BDBG_MSG(("BTHD_P_TuneAcquire = THD_Standard_ISDBT"));
#ifdef BTHD_ISDBT_SUPPORT
    BTHD_P_AppISDBTSettings(h, pParams);
#endif
  }

  /*     Inetrnal Settings    */

  h->pInternalAcquireParam->FrontEndMode         = THD_FrontEndMode_Baseband;
  h->pInternalAcquireParam->ChannelEstimatorMode = THD_ChannelEstimatorMode_Auto;
  h->pInternalAcquireParam->FFTWindowMode        = THD_FFTWindowMode_Auto;
  h->pInternalAcquireParam->ImpulseMode          = THD_ImpulseMode_None;
  h->pInternalAcquireParam->TransGuardMaxThreshold            = 0x00050000;
  h->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[0] = 296;
  h->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[1] = 210;
  h->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[2] = 150;
  h->pInternalAcquireParam->TransGuardMaxMinRatioThreshold[3] = 104;
  h->pInternalAcquireParam->SampleFreq                      = 54000000;

  /* Config Parameters (Public) */
  h->pConfigParam->TSMode              = THD_TSMode_Serial;
  h->pConfigParam->TSInvertClock       = false;
  h->pConfigParam->TSInvertValid       = false;
  h->pConfigParam->TSInvertSync        = false;
  h->pConfigParam->TSInvertError       = false;
  h->pConfigParam->TSSuppressClock     = THD_TSSuppressClock_WhenNotValid;
  h->pConfigParam->TSSuppressClockEdge = THD_TSSuppressClockEdge_Positive;
  h->pConfigParam->TSSuppressData      = false;
  h->pConfigParam->TSSyncLength        = THD_TSSyncLength_Byte;
  h->pConfigParam->TSHeaderLength      = THD_TSHeaderLength_1Byte;
  h->pConfigParam->TSDeleteHeader      = false;


  return ( eResult );
}


/***************************************************************************
 * BTHD_Acquire()
 ***************************************************************************/
BERR_Code BTHD_3x7x_Acquire(BTHD_Handle h, const BTHD_InbandParams *pParams)
{
  BERR_Code retCode;
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_ERR(("BTHD_3x7x_Acquire: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }

#if 0
  BTHD_3x7x_InitializeParams(h);
#else
  BTHD_P_SetAcqSettings(p3x7x, pParams);
#endif
  if (p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode & (THD_AcquireMode_Manual)) {
    BTHD_P_ResetEvent(p3x7x);
    p3x7x->pStatus->ThdCommonStatus.ReacquireCount = 0;
  }
  BTMR_StopTimer(p3x7x->hTimer);

  p3x7x->LockStatusTracking = 0xFFFFFFFF;

#ifdef BTHD_ISDBT_SUPPORT
  p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag |= (THD_IsdbtEwsFlag_eLockUpdate | THD_IsdbtEwsFlag_eUnlockStopEWS | THD_IsdbtEwsFlag_eLockStartEWS);
#endif

  p3x7x->ThdLockStatus = 0;
  BDBG_MSG(("Acquire %0x",  ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)));
  BKNI_EnterCriticalSection();
  if ( p3x7x->pCallback[BTHD_Callback_eLockChange] != NULL )
          (p3x7x->pCallback[BTHD_Callback_eLockChange])(p3x7x->pCallbackParam[BTHD_Callback_eLockChange] );
  BKNI_LeaveCriticalSection();

  retCode = BTHD_P_Acquire(p3x7x);
  if (retCode != THD_AcquireResult_Lock) {
    BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_INTR, 1 );
    BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_INTR, 0 );
  }
  p3x7x->AutoAcquireMasked = false;
  BTMR_StartTimer(p3x7x->hTimer, 150000);   /* the timer is in Micro second */
  BTMR_StartTimer(p3x7x->hBBSTimer, 1000000);   /* the timer is in Micro second */
  return BERR_SUCCESS;
}



/***************************************************************************
 * BTHD_P_GetDvbtStatus()
 ***************************************************************************/
static void BTHD_P_GetDvbtStatus (BTHD_3x7x_Handle p3x7x, BTHD_THDStatus *pStatus)
{
  /* Decode Priority */
  switch (p3x7x->pStatus->StdStatus.DvbtStatus.Priority)
  {
  case THD_DvbtPriorityMode_High:
    pStatus->eDecodeMode  = BTHD_Decode_Hp; break;
  default:
    BDBG_WRN(("Unrecognized Priority - defaulting to BTHD_Decode_Lp"));
    /* fall through */
  case THD_DvbtPriorityMode_Low:
    pStatus->eDecodeMode  = BTHD_Decode_Lp; break;
  }

  /*QAM Mode*/
  switch (p3x7x->pStatus->StdStatus.DvbtStatus.Qam)
  {
  case THD_Qam_Qpsk:
    pStatus->eModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eModulation  = BTHD_Modulation_e16Qam; break;
 default:
    BDBG_WRN(("Unrecognized Modulation setting - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_64Qam:
    pStatus->eModulation  = BTHD_Modulation_e64Qam; break;

  case THD_Qam_Dqpsk:
    pStatus->eModulation  = BTHD_Modulation_eDqpsk; break;
  }

  /* Code Rate */
  switch (p3x7x->pStatus->StdStatus.DvbtStatus.CodeRateHP)
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
  switch (p3x7x->pStatus->StdStatus.DvbtStatus.Hierarchy)
  {
  case THD_DvbtHierarchy_None:
    pStatus->eHierarchy  = BTHD_Hierarchy_0; break;
  case THD_DvbtHierarchy_1:
    pStatus->eHierarchy  = BTHD_Hierarchy_1; break;
  default:
    BDBG_WRN(("Unrecognized Hierarchy - defaulting to THD_Hierarchy_2"));
    /* fall through */
  case THD_DvbtHierarchy_2:
    pStatus->eHierarchy  = BTHD_Hierarchy_2; break;
  case THD_DvbtHierarchy_4:
    pStatus->eHierarchy  = BTHD_Hierarchy_4; break;;
  }

  /*pStatus->nCellId*/
  pStatus->ulRsCorrectedBlocks       = p3x7x->pStatus->StdStatus.DvbtStatus.TS_CBERC;                 /* accumulated corrected blocks by FEC */
  pStatus->ulRsUncorrectedBlocks     = p3x7x->pStatus->StdStatus.DvbtStatus.TS_UBERC;                 /* accumlated uncorrected blocks */
  pStatus->ulRsCleanBlocks           = p3x7x->pStatus->StdStatus.DvbtStatus.TS_NBERC;                 /* accumlated clean RS blocks */
  pStatus->ulRsTotalBlocks               = p3x7x->pStatus->StdStatus.DvbtStatus.TS_TBERC;
  pStatus->ulViterbiUncorrectedBits  = p3x7x->pStatus->StdStatus.DvbtStatus.TS_CERC;                  /* accumlated error bits after Viterbi kk*/
  pStatus->ulViterbiTotalbits        = (p3x7x->pStatus->StdStatus.DvbtStatus.TS_TBERC)*8*188;                  /* accumlated number of bits after Viterbi kk*/
  pStatus->ulViterbiBer              = p3x7x->pStatus->StdStatus.DvbtStatus.VitBER;                   /* This indicates the viterbi error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
  pStatus->ulPreViterbiBer           = p3x7x->pStatus->StdStatus.DvbtStatus.PreVitBER;                /* Indicates the pre-viterbi error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
}



/***************************************************************************
 * BTHD_P_GetIsdbtStatus()
 ***************************************************************************/
static void BTHD_P_GetIsdbtStatus (BTHD_3x7x_Handle p3x7x, BTHD_THDStatus *pStatus)
{
#ifdef BTHD_ISDBT_SUPPORT
  pStatus->bIsdbtEWS                    = p3x7x->pStatus->StdStatus.IsdbtStatus.Ews;                /* This return the EWS status bit */
  pStatus->bIsdbtPartialReception       = p3x7x->pStatus->StdStatus.IsdbtStatus.Pr;

  /*Layer A Modulation*/

  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.Qam[0])
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_e16Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting 0 - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_64Qam:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_e64Qam; break;
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtAModulation  = BTHD_Modulation_eDqpsk; break;
  }

  /*Layer A CodeRate*/
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.CodeRate[0])
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
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.TimeInt[0])
  {
  case THD_IsdbtTimeInt_0X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_IsdbtTimeInt_1X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_IsdbtTimeInt_2X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_IsdbtTimeInt_4X:
    pStatus->eIsdbtATimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtASegments                  = p3x7x->pStatus->StdStatus.IsdbtStatus.Segments[0];
  pStatus->ulIsdbtARsCorrectedBlocks        = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_CBERC[0];
  pStatus->ulIsdbtARsUncorrectedBlocks      = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_UBERC[0];
  pStatus->ulIsdbtARsCleanBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_NBERC[0];
  pStatus->ulIsdbtARsTotalBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[0];
  pStatus->ulIsdbtAViterbiUncorrectedBits   = p3x7x->pStatus->StdStatus.IsdbtStatus.VitBER[0];
  pStatus->ulIsdbtAViterbiTotalbits         = (p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[0])*8*188;
  pStatus->signalLevelAPercent              = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Ssi[0];
  pStatus->signalQualityAPercent            = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Sqi[0];

  /*Layer B Modulation*/
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.Qam[1])
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_e16Qam; break;
 default:
    BDBG_WRN(("Unrecognized Modulation setting 1 - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_64Qam:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_e64Qam; break;
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtBModulation  = BTHD_Modulation_eDqpsk; break;
  }

  /*Layer B CodeRate*/
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.CodeRate[1])
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
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.TimeInt[1])
  {
  case THD_IsdbtTimeInt_0X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_IsdbtTimeInt_1X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_IsdbtTimeInt_2X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_IsdbtTimeInt_4X:
    pStatus->eIsdbtBTimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtBSegments                  = p3x7x->pStatus->StdStatus.IsdbtStatus.Segments[1];
  pStatus->ulIsdbtBRsCorrectedBlocks        = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_CBERC[1];
  pStatus->ulIsdbtBRsUncorrectedBlocks      = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_UBERC[1];
  pStatus->ulIsdbtBRsCleanBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_NBERC[1];
  pStatus->ulIsdbtBRsTotalBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[1];
  pStatus->ulIsdbtBViterbiUncorrectedBits   = p3x7x->pStatus->StdStatus.IsdbtStatus.VitBER[1];
  pStatus->ulIsdbtBViterbiTotalbits         = (p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[1])*8*188;
  pStatus->signalLevelBPercent              = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Ssi[1];
  pStatus->signalQualityBPercent            = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Sqi[1];
  /*Layer C Modulation*/
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.Qam[2])
  {
  case THD_Qam_Qpsk:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_eQpsk; break;
  case THD_Qam_16Qam:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_e16Qam; break;
  default:
    BDBG_WRN(("Unrecognized Modulation setting 2 - defaulting to THD_Qam_64Qam"));
    /* fall through */
  case THD_Qam_64Qam:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_e64Qam; break;
  case THD_Qam_Dqpsk:
    pStatus->eIsdbtCModulation  = BTHD_Modulation_eDqpsk; break;
  }

  /*Layer C CodeRate*/
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.CodeRate[2])
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
  switch (p3x7x->pStatus->StdStatus.IsdbtStatus.TimeInt[2])
  {
  case THD_IsdbtTimeInt_0X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_0X; break;
  case THD_IsdbtTimeInt_1X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_1X; break;
  case THD_IsdbtTimeInt_2X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_2X; break;
  default:
    BDBG_WRN(("Unrecognized Time Interleaving - defaulting to THD_TimeInt_4X"));
    /* fall through */
  case THD_IsdbtTimeInt_4X:
    pStatus->eIsdbtCTimeInterleaving  = BTHD_IsdbtTimeInterleaving_3X; break;
  }

  pStatus->eIsdbtCSegments                  = p3x7x->pStatus->StdStatus.IsdbtStatus.Segments[2];
  pStatus->ulIsdbtCRsCorrectedBlocks        = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_CBERC[2];
  pStatus->ulIsdbtCRsUncorrectedBlocks      = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_UBERC[2];
  pStatus->ulIsdbtCRsCleanBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_NBERC[2];
  pStatus->ulIsdbtCRsTotalBlocks            = p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[2];
  pStatus->ulIsdbtCViterbiUncorrectedBits   = p3x7x->pStatus->StdStatus.IsdbtStatus.VitBER[2];
  pStatus->ulIsdbtCViterbiTotalbits         = (p3x7x->pStatus->StdStatus.IsdbtStatus.TS_TBERC[2])*8*188;
  pStatus->signalLevelCPercent              = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Ssi[2];
  pStatus->signalQualityCPercent            = (uint8_t)p3x7x->pStatus->StdStatus.IsdbtStatus.Sqi[2];
#else
    BSTD_UNUSED(p3x7x);
    BSTD_UNUSED(pStatus);
#endif
}

/***************************************************************************
 * BTHD_3x7x_GetStatus()
 ***************************************************************************/
BERR_Code BTHD_3x7x_GetStatus (BTHD_Handle h, BTHD_THDStatus *pStatus)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
  BDBG_MSG(("BTHD_3x7x_GetStatus"));
  BTHD_P_Status(p3x7x);

  BDBG_ASSERT( NULL != pStatus );

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On)
      return BERR_NOT_INITIALIZED;

  /* ---------------  Mapping Common Fields ---------------- */

  pStatus->bReceiverLock             = ((p3x7x->ThdLockStatus >> THD_LockStatusBit_SystemLock) & 1);    /* Indicates whether the receiver is locked */
  pStatus->bFecLock                  = ((p3x7x->ThdLockStatus >> THD_LockStatusBit_SystemLock) & 1);       /* Indicates whether FEC is locked */
  pStatus->bNoSignalDetected         = ((p3x7x->ThdLockStatus >> THD_LockStatusBit_NoDVBTSignal) & 1);  /* Indicates whether signal is detected. Only should be used by channel scan as a flag to move on to the next channel */


  /* Transmission Mode */
  switch (p3x7x->pStatus->ThdCommonStatus.TransmissionMode)
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
  switch (p3x7x->pStatus->ThdCommonStatus.GuardInterval)
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

  pStatus->ulRFAgc                   = p3x7x->pStatus->ThdCommonStatus.RagcGain;                 /* RF Agc value in 1/10 percent */
  pStatus->ulIFAgc                   = p3x7x->pStatus->ThdCommonStatus.IagcGain;                 /* IF Agc value in 1/10 percent */
  /*pStatus->nSignalStrength */
  pStatus->lCarrierOffset            = p3x7x->pStatus->ThdCommonStatus.CarrierOffset;            /* Offset of carrier loop in Hz */
  pStatus->lTimingOffset             = p3x7x->pStatus->ThdCommonStatus.TimingOffset;             /* Offset of timing loop in Hz */
  pStatus->nSnr                      = p3x7x->pStatus->ThdCommonStatus.SNR;                      /* SNR value of reciever in 1/256th dB */
  pStatus->bSpectrumInverted         = p3x7x->pStatus->ThdCommonStatus.Spectrum;                  /* Indicates the signal is inverted */
  pStatus->ulReacqCount              = p3x7x->pStatus->ThdCommonStatus.ReacquireCount;           /* number of reacquisitions performed */
  /*pStatus->bValid*/
  pStatus->nSignalStrength           = (p3x7x->pStatus->ThdCommonStatus.SignalStrength*10/256);


  /* ---------------  Mapping DVB-T Specific Fields ---------------- */
 if (p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_DVBT)
 {
    BTHD_P_GetDvbtStatus (p3x7x, pStatus);
    pStatus->signalLevelPercent =  p3x7x->pStatus->ThdCommonStatus.Ssi;
    pStatus->signalQualityPercent =  p3x7x->pStatus->ThdCommonStatus.Sqi;
 }
  /* ---------------  Mapping ISDB-T Specific Fields ---------------- */
#ifdef BTHD_ISDBT_SUPPORT
 if (p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_ISDBT)
    BTHD_P_GetIsdbtStatus (p3x7x, pStatus);
#endif
  return BERR_SUCCESS;

}

/******************************************************************************
 * BTHD_LockStatus()
 ******************************************************************************/
BERR_Code BTHD_3x7x_GetLockStatus(BTHD_Handle h, BTHD_LockStatus *pLockStatus)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_ERR(("BTHD_3x7x_LockStatus: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }
  if ((p3x7x->ThdLockStatus >> THD_LockStatusBit_NoDVBTSignal) & 1)
        *pLockStatus = BTHD_LockStatus_eNoSignal;
  else
    *pLockStatus = ((p3x7x->ThdLockStatus >> THD_LockStatusBit_SystemLock) & 1) != 0;  /* compare ((p3x7x->ThdLockStatus >> THD_LockStatusBit_SystemLock) & 1) with 0, result is true or false */
  return BERR_SUCCESS;
}

/******************************************************************************
 * BTHD_ResetStatus()
 ******************************************************************************/
BERR_Code BTHD_3x7x_ResetStatus(BTHD_Handle h)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_ERR(("BTHD_3x7x_ResetStatus: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }

  BTHD_P_ResetStatusHW(p3x7x);
  BTHD_P_ResetStatus(p3x7x);
  return BERR_SUCCESS;
}

/***************************************************************************
 * BTHD_3x7x_GetConstellation()
 ***************************************************************************/
BERR_Code BTHD_3x7x_GetConstellation(BTHD_Handle h, int16_t *pI, int16_t *pQ)
{

  int16_t nbrGotten ;
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

    if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_ERR(("BTHD_3x7x_GetConstellation: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }
#ifdef BTHD_ISDBT_SUPPORT
  if (p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_ISDBT)
    BTHD_P_GetIsdbtSoftDecisionBuf(p3x7x, THD_IQ_CONSTELLATION_VECTORS, pI, pQ, &nbrGotten);
  else
#endif
  BTHD_P_GetDvbtSoftDecisionBuf(p3x7x, THD_IQ_CONSTELLATION_VECTORS, pI, pQ, &nbrGotten);


  return BERR_SUCCESS;
}
/******************************************************************************
 * BTHD_3x7x_ProcessBBSInterruptEvent()
 ******************************************************************************/
BERR_Code BTHD_3x7x_ProcessBBSInterruptEvent(BTHD_Handle h)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

    BTMR_StopTimer(p3x7x->hBBSTimer); /* just tobe safe */

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_MSG(("BTHD_3x7x_ProcessBBSInterruptEvent: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }
  BMEM_FlushCache(h->settings.hHeap, p3x7x->pAcquireParam, sizeof( THD_3x7x_AcquireParam_t ) );
  /*BDBG_WRN(("BTHD_3x7x_ProcessBBSInterruptEvent "));*/
  if (p3x7x->pAcquireParam->CommonAcquireParam.NexusStatusMode & THD_NexusStatusMode_EnableStatusForNexus)
  {
    BMEM_FlushCache(h->settings.hHeap, p3x7x->pStatus, sizeof( THD_3x7x_Status_t ) );
    BTHD_P_Status(p3x7x);
    p3x7x->pAcquireParam->CommonAcquireParam.NexusStatusMode &= (~THD_NexusStatusMode_EnableStatusForNexus);
  }

  if (p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode & THD_AcquireMode_ResetStatus) {
    p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode &= ~THD_AcquireMode_ResetStatus;
    BMEM_FlushCache(h->settings.hHeap, p3x7x->pStatus, sizeof( THD_3x7x_Status_t ) );
    BTHD_3x7x_ResetStatus(h);
    p3x7x->pStatus->ThdCommonStatus.ReacquireCount = 0;

  }

  if (p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode & THD_AcquireMode_Manual)
  {
    p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode &= ~THD_AcquireMode_Manual;
    BTHD_P_ResetEvent(p3x7x);
    p3x7x->pStatus->ThdCommonStatus.ReacquireCount = 0;
    p3x7x->LockStatusTracking = 0xFFFFFFFF;
    BTHD_P_Acquire(p3x7x);
    p3x7x->AutoAcquireMasked = false;
  }

  BTMR_StartTimer(p3x7x->hBBSTimer, 1000000);   /* the timer is in Micro second */

  return BERR_SUCCESS;
}


/******************************************************************************
 * BTHD_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BTHD_3x7x_ProcessInterruptEvent(BTHD_Handle h)
{
  BTHD_3x7x_P_Handle *p3x7x;
  BERR_Code retCode;
#ifdef BTHD_ISDBT_SUPPORT
  uint32_t TMCC_SYNC_value, TMCC_Alert_value, TMCC_Err_update;
#endif
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);


  BDBG_MSG(("BTHD_ProcessInterruptEvent "));

  if (p3x7x->pStatus->ThdCommonStatus.PowerStatus != THD_ePower_On) {
        BDBG_ERR(("BTHD_3x7x_ProcessInterruptEvent: power is still off  "));
        return BERR_NOT_INITIALIZED;
  }
  BTMR_StopTimer(p3x7x->hTimer);

#ifdef BTHD_ISDBT_SUPPORT
  if ((p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_ISDBT) && ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)) {
    /* EWS bit flips when the cable is not connected, so we need to read the error count to make sure the cable is connected before */
    /* reporting the EWS status event change */
    TMCC_SYNC_value = BREG_ReadField(p3x7x->hRegister, THD_CORE_STATUS, TMCC_SYNC);
    if (TMCC_SYNC_value)
    {
        TMCC_Err_update = BREG_Read32(p3x7x->hRegister, BCHP_THD_CORE_TMCC_UBERC);
      if (p3x7x->pAcquireParam->IsdbtLocaleParam.TMCC_Err == TMCC_Err_update)
      {
          TMCC_Alert_value = BREG_ReadField(p3x7x->hRegister, THD_CORE_TMCC_MISC, ALERT);
        /* If the channel is lock after tuning, set EWS event when EWS is on */
        if(p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag & THD_IsdbtEwsFlag_eLockStartEWS)
        {
            if (1 == TMCC_Alert_value)
          {
            p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag &= ~THD_IsdbtEwsFlag_eLockStartEWS;
            p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag &= ~THD_IsdbtEwsFlag_eUnlockStopEWS;
            BKNI_SetEvent(p3x7x->hEWSEvent);
            p3x7x->pAcquireParam->IsdbtLocaleParam.EWS_Setting = TMCC_Alert_value;
            p3x7x->pAcquireParam->IsdbtLocaleParam.TMCC_Err = TMCC_Err_update;
          }
        }
        /* In other cases, set EWS event when EWS changes */
        else if (p3x7x->pAcquireParam->IsdbtLocaleParam.EWS_Setting != TMCC_Alert_value)
        {
          p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag &= ~THD_IsdbtEwsFlag_eUnlockStopEWS;
          BKNI_SetEvent(p3x7x->hEWSEvent);
          p3x7x->pAcquireParam->IsdbtLocaleParam.EWS_Setting = TMCC_Alert_value;
          p3x7x->pAcquireParam->IsdbtLocaleParam.TMCC_Err = TMCC_Err_update;
        }
      }
    }
  }
#endif

  if ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1) {
    BDBG_MSG(("Still locked"));
  } else {
    BDBG_MSG(("Not locked %0x  %0x",  ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1), p3x7x->LockStatusTracking));
    if (p3x7x->LockStatusTracking != ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1))
    {
        p3x7x->LockStatusTracking = ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1);
        BDBG_MSG(("process %0x",  ((p3x7x->ThdLockStatus>> THD_LockStatusBit_SystemLock) & 1)));

        BKNI_EnterCriticalSection();
        if(p3x7x->pCallback[BTHD_Callback_eLockChange] != NULL )
            (p3x7x->pCallback[BTHD_Callback_eLockChange])(p3x7x->pCallbackParam[BTHD_Callback_eLockChange] );
        BKNI_LeaveCriticalSection();

#ifdef BTHD_ISDBT_SUPPORT
        if (p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_ISDBT) {
        /* If the channel is unlock, set EWS event if EWS has been started before. For the unlock channel after tuning , don't set EWS event */
          if (!(p3x7x->pAcquireParam->IsdbtLocaleParam.EwsFlag & THD_IsdbtEwsFlag_eUnlockStopEWS) && p3x7x->pAcquireParam->IsdbtLocaleParam.EWS_Setting == 1)
          {
              BKNI_SetEvent(p3x7x->hEWSEvent);
          }
          p3x7x->pAcquireParam->IsdbtLocaleParam.EWS_Setting = 0;
        }
#endif
    }
    if (p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode & (THD_AcquireMode_Auto | THD_AcquireMode_Scan) && ( ! p3x7x->AutoAcquireMasked) ) {
   /* if (p3x7x->pAcquireParam->CommonAcquireParam.AcquireMode & THD_AcquireMode_Auto) { */
      BTHD_P_ResetEvent(p3x7x);
      retCode = BTHD_P_Acquire(p3x7x);
      if (retCode != THD_AcquireResult_Lock) {
        BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_INTR, 1 );
        BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_INTR, 0 );
#ifdef BTHD_ISDBT_SUPPORT
        if (p3x7x->pAcquireParam->CommonAcquireParam.Standard == THD_Standard_ISDBT) {
          BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_B_INTR, 1 );
          BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_C_INTR, 1 );
          BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_B_INTR, 0 );
          BREG_WriteField(p3x7x->hRegister, THD_INTR2B_CPU_SET, RS_SYNC_LOSS_C_INTR, 0 );
        }
#endif
      }

   }
    /* In case of single acquire: if we lose lock, as long as the app is informed the AP is done */
  }
  BTMR_StartTimer(p3x7x->hTimer, 150000);   /* the timer is in Micro second */


  return BERR_SUCCESS;
}

/******************************************************************************
BTHD_3x7x_GetLockStateChangeEvent()
******************************************************************************/
BERR_Code BTHD_3x7x_GetLockStateChangeEvent(BTHD_Handle h, BKNI_EventHandle* hEvent)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  *hEvent = p3x7x->hLockEvent;
  return BERR_SUCCESS;
}

/******************************************************************************
BTHD_3x7x_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BTHD_3x7x_GetInterruptEventHandle(BTHD_Handle h, BKNI_EventHandle* hEvent)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  *hEvent = p3x7x->hIntEvent;
  return BERR_SUCCESS;

}

/******************************************************************************
BTHD_3x7x_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BTHD_3x7x_GetBBSInterruptEventHandle(BTHD_Handle h, BKNI_EventHandle* hEvent)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  *hEvent = p3x7x->hBBSInterruptEvent;
  return BERR_SUCCESS;

}

/******************************************************************************
BTHD_3x7x_GetEWSEventHandle()
******************************************************************************/
BERR_Code BTHD_3x7x_GetEWSEventHandle(BTHD_Handle h, BKNI_EventHandle* hEvent)
{
  BTHD_3x7x_P_Handle *p3x7x;
  p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

  *hEvent = p3x7x->hEWSEvent;
  return BERR_SUCCESS;
}

/******************************************************************************
BTHD_3x7x_PowerDown()
******************************************************************************/
BERR_Code BTHD_3x7x_PowerDown(BTHD_Handle h)
{
    BTHD_3x7x_P_Handle *p3x7x;
    uint32_t BufSrc;
    p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

    if (p3x7x->hTimer != NULL)
    {
        BTMR_StopTimer(p3x7x->hTimer);
        BTMR_DestroyTimer(p3x7x->hTimer);
        p3x7x->hTimer = NULL;
    }
    if (p3x7x->hBBSTimer != NULL)
    {
        BTMR_StopTimer(p3x7x->hBBSTimer);
        BTMR_DestroyTimer(p3x7x->hBBSTimer);
        p3x7x->hBBSTimer = NULL;
    }

    BREG_WriteField(p3x7x->hRegister, THD_CORE_GLB, BE_MODE, 0 );              /* stop the ISDB-T dump to memory, needs to be set before shuting the clock*/
    BREG_WriteField(p3x7x->hRegister, THD_CORE_GLB, SOFT_SHUTDOWN , 1 );       /* soft shutdown, needs to be set before shuting the clock */

    BufSrc = BREG_Read32(p3x7x->hRegister,  BCHP_THD_CORE_SCB_ADDR_OFFSET);

    BREG_Write32(p3x7x->hRegister,  BCHP_THD_CORE_RST ,0xffffffff );       /* Reset datapath and register interface */
    BREG_Write32(p3x7x->hRegister,  BCHP_THD_CORE_RST2,0x000001ff );

    BREG_Write32(p3x7x->hRegister,  BCHP_THD_CORE_SCB_ADDR_OFFSET, BufSrc);

    p3x7x->pStatus->ThdCommonStatus.PowerStatus = THD_ePower_Off;
    BREG_Write32(p3x7x->hRegister, BCHP_THD_CORE_CLK, 0xFFFFFFFF);

    return BERR_SUCCESS;
}

/******************************************************************************
BTHD_3x7x_PowerUp()
******************************************************************************/
BERR_Code BTHD_3x7x_PowerUp(BTHD_Handle h)
{
    BERR_Code retCode;
    BTHD_3x7x_P_Handle *p3x7x;
    BTMR_Settings sTimerSettings;
    p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);

    BREG_Write32(p3x7x->hRegister, BCHP_THD_CORE_CLK, 0);

    BREG_WriteField(p3x7x->hRegister, THD_CORE_GLB, BE_MODE, 1 );              /* undo stop the ISDB-T dump to memory from power down*/
    BREG_WriteField(p3x7x->hRegister, THD_CORE_GLB, SOFT_SHUTDOWN , 0 );       /* undo soft shutdown  from power down*/

    BTHD_P_ResetEvent(p3x7x);
    BINT_DisableCallback( p3x7x->hRsSyncCallback);
    BINT_DisableCallback( p3x7x->hRsSyncLossCallback);
#ifdef BTHD_ISDBT_SUPPORT
    BINT_DisableCallback(p3x7x->hRsSyncBCallback);
    BINT_DisableCallback(p3x7x->hRsSyncCCallback);
    BINT_DisableCallback( p3x7x->hRsSyncLossBCallback);
    BINT_DisableCallback( p3x7x->hRsSyncLossCCallback);
#endif
    BKNI_ResetEvent(p3x7x->hIntEvent);
    BKNI_ResetEvent(p3x7x->hBBSInterruptEvent);

    /* Create timer for status lock check */
     BTMR_GetDefaultTimerSettings(&sTimerSettings);
     sTimerSettings.type = BTMR_Type_eCountDown;
     sTimerSettings.cb_isr = (BTMR_CallbackFunc)BTHD_P_TimerFunc_isr;
     sTimerSettings.pParm1 = (void*)h;
     sTimerSettings.parm2 = 0;
     sTimerSettings.exclusive = false;

     if (p3x7x->hTimer) {
        BTMR_StopTimer(p3x7x->hTimer);
        BTMR_DestroyTimer(p3x7x->hTimer);
        p3x7x->hTimer = NULL;
     }
     retCode = BTMR_CreateTimer (h->settings.hTmr, &p3x7x->hTimer, &sTimerSettings);
     if ( retCode != BERR_SUCCESS )
     {
        BDBG_ERR(("BTHD_Open: Create Timer Failed"));
        retCode = BERR_TRACE(retCode);
        goto done;
     }

    /* Create timer for status lock check */
     BTMR_GetDefaultTimerSettings(&sTimerSettings);
     sTimerSettings.type = BTMR_Type_eCountDown;
     sTimerSettings.cb_isr = (BTMR_CallbackFunc)BTHD_P_BBSTimerFunc_isr;
     sTimerSettings.pParm1 = (void*)h;
     sTimerSettings.parm2 = 0;
     sTimerSettings.exclusive = false;

     if (p3x7x->hBBSTimer) {
        BTMR_StopTimer(p3x7x->hBBSTimer);
        BTMR_DestroyTimer(p3x7x->hBBSTimer);
        p3x7x->hBBSTimer = NULL;
     }
     retCode = BTMR_CreateTimer (h->settings.hTmr, &p3x7x->hBBSTimer, &sTimerSettings);
      if ( retCode != BERR_SUCCESS )
      {
        BDBG_ERR(("BTHD_Open: Create Timer Failed"));
        retCode = BERR_TRACE(retCode);
        goto done;
      }

    p3x7x->pStatus->ThdCommonStatus.PowerStatus = THD_ePower_On;
done:
    return BERR_SUCCESS;
}

/***************************************************************************
BTHD_3x7x_InstallCallback
****************************************************************************/
BERR_Code BTHD_3x7x_InstallCallback(BTHD_Handle     h,  /* [in] Device channel handle */
                               BTHD_Callback        callbackType, /* [in] Type of callback */
                               BTHD_CallbackFunc    pCallback,     /* [in] Function Ptr to callback */
                               void                 *pParam        /* [in] Generic parameter send on callback */
                               )
{
   BERR_Code retCode = BERR_SUCCESS;
   BTHD_3x7x_P_Handle *p3x7x;
   p3x7x = (BTHD_3x7x_P_Handle *)(h->pImpl);
   BDBG_ENTER(BTHD_InstallCallback);

    switch( callbackType )
    {
        case BTHD_Callback_eLockChange:
            p3x7x->pCallback[callbackType] = pCallback;
            p3x7x->pCallbackParam[callbackType] = pParam;
            break;
        case BTHD_Callback_eUpdateGain:
            p3x7x->pCallback[callbackType] = pCallback;
            p3x7x->pCallbackParam[callbackType] = pParam;
            break;
        case BTHD_Callback_eNoSignal:
            p3x7x->pCallback[callbackType] = pCallback;
            p3x7x->pCallbackParam[callbackType] = pParam;
            break;
        case BTHD_Callback_eTuner:
            p3x7x->pCallback[callbackType] = pCallback;
            p3x7x->pCallbackParam[callbackType] = pParam;
            break;

        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BTHD_InstallCallback);
    return( retCode );

}
/******************************************************************************
BTHD_3x7x_P_TNR_callback()
******************************************************************************/
BERR_Code BTHD_3x7x_P_TNR_callback(
BTHD_3x7x_Handle    h,
uint16_t            Mode,
int16_t             *Gain,
uint16_t            *SmartTune,
uint32_t            *RF_Freq
)
{
     BTHD_P_ThdCallbackData_t TunerCallback;
#if ((BCHP_CHIP==7552) || (BCHP_CHIP==3461))
    TunerCallback.hTunerChn = (h->pCallbackParam[BTHD_Callback_eTuner]);
    switch (Mode)
    {
        case BTHD_CallbackMode_eDisablePower:
              TunerCallback.Mode = BTHD_CallbackMode_eDisablePower;
              break;
        case BTHD_CallbackMode_eEnablePower:
              TunerCallback.Mode = BTHD_CallbackMode_eEnablePower;
              break;
        case BTHD_CallbackMode_eSetMode:
              TunerCallback.Mode = BTHD_CallbackMode_eSetMode;
              TunerCallback.Freq_Offset = 0;
              TunerCallback.Symbol_Rate = 0;
              break;
        case BTHD_CallbackMode_eRequestMode:
              TunerCallback.Mode = BTHD_CallbackMode_eRequestMode;
              break;
        case BTHD_CallbackMode_eSmartTune:
              TunerCallback.Mode = BTHD_CallbackMode_eSmartTune;
              break;
    }
    BKNI_EnterCriticalSection();
    (h->pCallback[BTHD_Callback_eTuner])(&TunerCallback);
    BKNI_LeaveCriticalSection();

    *SmartTune = TunerCallback.SmartTune;

  /* Compute total tuner gain */
    *Gain = TunerCallback.External_Gain_x256db;
    if (!((TunerCallback.PreADC_Gain_x256db == -32768) || (TunerCallback.PostADC_Gain_x256db == -32768)))
      *Gain += (TunerCallback.PreADC_Gain_x256db + TunerCallback.PostADC_Gain_x256db);
    *RF_Freq = TunerCallback.RF_Freq;

#else
    TunerCallback.RF_Freq = 0;
    TunerCallback.Total_Mix_After_ADC = 0;
    TunerCallback.PreADC_Gain_x256db  = 0;
    TunerCallback.PostADC_Gain_x256db = 0;
    TunerCallback.External_Gain_x256db = 0;
    TunerCallback.SmartTune = 0;
#endif

  return BERR_SUCCESS;
}



