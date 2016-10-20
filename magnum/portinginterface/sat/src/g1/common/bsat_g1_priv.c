/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv);

/* extern bool bEnableDebugLog; */

#if defined(BSAT_HAS_ACM) && defined(BCHP_AFEC_0_BCH_STREAM_ID0_STAT) && !defined(BSAT_EXCLUDE_AFEC)
static const uint32_t afec_stream_stat_reg[8] = {
   BCHP_AFEC_BCH_STREAM_ID0_STAT, BCHP_AFEC_BCH_STREAM_ID1_STAT,
   BCHP_AFEC_BCH_STREAM_ID2_STAT, BCHP_AFEC_BCH_STREAM_ID3_STAT,
   BCHP_AFEC_BCH_STREAM_ID4_STAT, BCHP_AFEC_BCH_STREAM_ID5_STAT,
   BCHP_AFEC_BCH_STREAM_ID6_STAT, BCHP_AFEC_BCH_STREAM_ID7_STAT
};

static const uint32_t afec_pls_ldpc_iter_cnt_reg[8] = {
   BCHP_AFEC_PLS0_LDPC_ITER_CNT, BCHP_AFEC_PLS1_LDPC_ITER_CNT,
   BCHP_AFEC_PLS2_LDPC_ITER_CNT, BCHP_AFEC_PLS3_LDPC_ITER_CNT,
   BCHP_AFEC_PLS4_LDPC_ITER_CNT, BCHP_AFEC_PLS5_LDPC_ITER_CNT,
   BCHP_AFEC_PLS6_LDPC_ITER_CNT, BCHP_AFEC_PLS7_LDPC_ITER_CNT,
};

static const uint32_t afec_pls_bch_decnblk_reg[8] = {
   BCHP_AFEC_PLS0_BCH_DECNBLK, BCHP_AFEC_PLS1_BCH_DECNBLK,
   BCHP_AFEC_PLS2_BCH_DECNBLK, BCHP_AFEC_PLS3_BCH_DECNBLK,
   BCHP_AFEC_PLS4_BCH_DECNBLK, BCHP_AFEC_PLS5_BCH_DECNBLK,
   BCHP_AFEC_PLS6_BCH_DECNBLK, BCHP_AFEC_PLS7_BCH_DECNBLK,
};

#endif


/******************************************************************************
 BSAT_g1_P_Open()
******************************************************************************/
BERR_Code BSAT_g1_P_Open(BSAT_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BSAT_Settings *pSettings)
{
   BERR_Code retCode;
   BSAT_Handle hDev;
   BSAT_g1_P_Handle *hG1Dev;

   BDBG_ENTER(BSAT_g1_P_Open);

   /* allocate memory for the handle */
   hDev = (BSAT_Handle)BKNI_Malloc(sizeof(BSAT_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BSAT_P_Handle));
   hG1Dev = (BSAT_g1_P_Handle *)BKNI_Malloc(sizeof(BSAT_g1_P_Handle));
   BDBG_ASSERT(hG1Dev);
   BKNI_Memset((void*)hG1Dev, 0, sizeof(BSAT_g1_P_Handle));
   hDev->pImpl = (void*)hG1Dev;
   hDev->pChannels = (BSAT_P_ChannelHandle **)BKNI_Malloc(BSAT_G1_MAX_CHANNELS * sizeof(BSAT_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BSAT_G1_MAX_CHANNELS * sizeof(BSAT_P_ChannelHandle *));

   /* initialize the device handle */
   hG1Dev->hChip = hChip;
   hG1Dev->hRegister = (BREG_Handle)pReg;
   hG1Dev->hInterrupt = hInterrupt;
   hG1Dev->bResetDone = false;
   retCode = BKNI_CreateEvent(&(hG1Dev->hInitDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   BSAT_g1_P_InitHandle(hDev); /* initialize chip-specific device handle settings */

   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pSettings, sizeof(BSAT_Settings));
   hDev->totalChannels = BSAT_G1_MAX_CHANNELS;

   *h = hDev;

   BDBG_LEAVE(BSAT_g1_P_Open);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_Close()
******************************************************************************/
BERR_Code BSAT_g1_P_Close(BSAT_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_Handle *hG1Dev = (BSAT_g1_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_g1_P_Close);

   BKNI_DestroyEvent(hG1Dev->hInitDoneEvent);

   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BSAT_g1_P_Close);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_OpenChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_OpenChannel(BSAT_Handle h, BSAT_ChannelHandle *pChannelHandle, uint32_t chan, const BSAT_ChannelSettings *pSettings)
{
   extern const uint32_t BSAT_g1_ChannelIntrID[BSAT_G1_MAX_CHANNELS][BSAT_g1_MaxIntID];

   BERR_Code retCode;
   BSAT_ChannelSettings cs;
   BSAT_P_ChannelHandle *ch;
   BSAT_g1_P_ChannelHandle *pChan;
   BSAT_g1_P_Handle *pDev = h->pImpl;

   BDBG_ENTER(BSAT_g1_P_OpenChannel);

   BDBG_ASSERT(chan < BSAT_G1_MAX_CHANNELS);

   if (pSettings == NULL)
      BSAT_g1_GetChannelDefaultSettings(h, chan, &cs);
   else
      cs = *pSettings;

   /* allocate memory for the channel handle */
   ch = (BSAT_P_ChannelHandle *)BKNI_Malloc(sizeof(BSAT_P_ChannelHandle));
   BDBG_ASSERT(ch);
   BKNI_Memset((void*)ch, 0, sizeof(BSAT_P_ChannelHandle));
   BKNI_Memcpy((void*)(&ch->settings), (void*)&cs, sizeof(BSAT_ChannelSettings));
   ch->channel = (uint8_t)chan;
   ch->pDevice = h;
   h->pChannels[chan] = ch;

   pChan = (BSAT_g1_P_ChannelHandle *)BKNI_Malloc(sizeof(BSAT_g1_P_ChannelHandle));
   BDBG_ASSERT(pChan);
   BKNI_Memset((void*)pChan, 0, sizeof(BSAT_g1_P_ChannelHandle));
   ch->pImpl = (void*)pChan;

   /* acquire sds clock for interrupts */
   BSAT_g1_P_SdsPowerUp(h->pChannels[chan]);

   /* create callbacks for legacy qpsk lock interrupts */
   retCode = BINT_CreateCallback(&(pChan->hQpskLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eSdsLock], BSAT_g1_P_Lock_isr, (void*)ch, BSAT_g1_IntID_eSdsLock);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hQpskNotLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eSdsNotLock], BSAT_g1_P_NotLock_isr, (void*)ch, BSAT_g1_IntID_eSdsNotLock);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hDftDoneCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eDftDone], BSAT_g1_P_DftDone_isr, (void*)ch, BSAT_g1_IntID_eDftDone);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   /* create callbacks for timer interrupts */
   retCode = BINT_CreateCallback(&(pChan->hBaudTimerCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eBaudTimer], BSAT_g1_P_BaudTimer_isr, (void*)ch, BSAT_g1_IntID_eBaudTimer);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hBerTimerCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eBerTimer], BSAT_g1_P_BerTimer_isr, (void*)ch, BSAT_g1_IntID_eBerTimer);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hGen1TimerCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eGenTimer1], BSAT_g1_P_Gen1Timer_isr, (void*)ch, BSAT_g1_IntID_eGenTimer1);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hGen2TimerCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eGenTimer2], BSAT_g1_P_Gen2Timer_isr, (void*)ch, BSAT_g1_IntID_eGenTimer2);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_CreateCallback(&(pChan->hGen3TimerCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eGenTimer3], BSAT_g1_P_Gen3Timer_isr, (void*)ch, BSAT_g1_IntID_eGenTimer3);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

#ifndef BSAT_EXCLUDE_MI2C
   /* create callback for MI2C interrupt */
   retCode = BINT_CreateCallback(&(pChan->hMi2cCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eMi2c], BSAT_g1_P_Mi2c_isr, (void*)ch, BSAT_g1_IntID_eMi2c);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(pChan->hMi2cEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
#endif

   retCode = BKNI_CreateEvent(&(pChan->hLockChangeEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(pChan->hAcqDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(pChan->hSignalNotificationEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(pChan->hReadyEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   BSAT_g1_P_InitChannelHandle(ch);

   /* create HP callback */
   if (pChan->bHasAfec || pChan->bHasTfec)
   {
      /* create event for HP state match */
      retCode = BINT_CreateCallback(&(pChan->hHpFrameBoundaryCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eHpFrameBoundary], BSAT_g1_P_HpFrameBoundary_isr, (void*)ch, BSAT_g1_IntID_eHpFrameBoundary);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hHpStateMatchCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eHpStateMatch], BSAT_g1_P_HpStateMatch_isr, (void*)ch, BSAT_g1_IntID_eHpStateMatch);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hHpStateChangeCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eHpStateChange], BSAT_g1_P_HpStateChange_isr, (void*)ch, BSAT_g1_IntID_eHpStateChange);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }

#ifndef BSAT_EXCLUDE_AFEC
   if (pChan->bHasAfec)
   {
      /* acquire afec clock for interrupts */
      BSAT_g1_P_AfecPowerUp_isr(h->pChannels[chan]);

      retCode = BINT_CreateCallback(&(pChan->hAfecLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eAfecLock], BSAT_g1_P_Lock_isr, (void*)ch, BSAT_g1_IntID_eAfecLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hAfecNotLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eAfecNotLock], BSAT_g1_P_NotLock_isr, (void*)ch, BSAT_g1_IntID_eAfecNotLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hAfecMpegLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eAfecMpLock], BSAT_g1_P_AfecMpegLock_isr, (void*)ch, BSAT_g1_IntID_eAfecMpLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hAfecMpegNotLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eAfecMpNotLock], BSAT_g1_P_AfecMpegNotLock_isr, (void*)ch, BSAT_g1_IntID_eAfecMpNotLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);

      /* release afec clock */
      BSAT_g1_P_AfecPowerDown_isrsafe(h->pChannels[chan]);
   }
#endif

#ifndef BSAT_EXCLUDE_TFEC
   if (pChan->bHasTfec)
   {
      retCode = BINT_CreateCallback(&(pChan->hTfecLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eTfecLock], BSAT_g1_P_Lock_isr, (void*)ch, BSAT_g1_IntID_eTfecLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hTfecNotLockCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eTfecNotLock], BSAT_g1_P_NotLock_isr, (void*)ch, BSAT_g1_IntID_eTfecNotLock);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_CreateCallback(&(pChan->hTfecSyncCb), pDev->hInterrupt, BSAT_g1_ChannelIntrID[chan][BSAT_g1_IntID_eTfecSync], BSAT_g1_P_TfecSync_isr, (void*)ch, BSAT_g1_IntID_eTfecSync);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
#endif

   /* release sds clock for interrupts */
   BSAT_g1_P_SdsPowerDown(h->pChannels[chan]);

   *pChannelHandle = ch;

   BDBG_LEAVE(BSAT_g1_P_OpenChannel);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_CloseChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_CloseChannel(BSAT_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_ChannelHandle *pChan;

   BDBG_ENTER(BSAT_g1_P_CloseChannel);

   BSAT_g1_P_ResetChannel(h, true);

   BSAT_g1_P_IndicateNotLocked_isrsafe(h);

   pChan = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(pChan->hLockChangeEvent);
   BKNI_DestroyEvent(pChan->hAcqDoneEvent);
   BKNI_DestroyEvent(pChan->hSignalNotificationEvent);
   BKNI_DestroyEvent(pChan->hReadyEvent);

   BINT_DestroyCallback(pChan->hBaudTimerCb);
   BINT_DestroyCallback(pChan->hBerTimerCb);
   BINT_DestroyCallback(pChan->hGen1TimerCb);
   BINT_DestroyCallback(pChan->hGen2TimerCb);
   BINT_DestroyCallback(pChan->hGen3TimerCb);
   BINT_DestroyCallback(pChan->hQpskLockCb);
   BINT_DestroyCallback(pChan->hQpskNotLockCb);
   BINT_DestroyCallback(pChan->hHpFrameBoundaryCb);
   BINT_DestroyCallback(pChan->hHpStateMatchCb);
   BINT_DestroyCallback(pChan->hHpStateChangeCb);
   BINT_DestroyCallback(pChan->hDftDoneCb);

#ifndef BSAT_EXCLUDE_MI2C
   BKNI_DestroyEvent(pChan->hMi2cEvent);
   BINT_DestroyCallback(pChan->hMi2cCb);
#endif

#ifndef BSAT_EXCLUDE_AFEC
   if (pChan->bHasAfec)
   {
      BINT_DestroyCallback(pChan->hAfecLockCb);
      BINT_DestroyCallback(pChan->hAfecNotLockCb);
      BINT_DestroyCallback(pChan->hAfecMpegLockCb);
      BINT_DestroyCallback(pChan->hAfecMpegNotLockCb);
   }
#endif

#ifndef BSAT_EXCLUDE_TFEC
   if (pChan->bHasTfec)
   {
      BINT_DestroyCallback(pChan->hTfecLockCb);
      BINT_DestroyCallback(pChan->hTfecNotLockCb);
      BINT_DestroyCallback(pChan->hTfecSyncCb);
   }
#endif

   BKNI_Free((void*)pChan);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BSAT_g1_P_CloseChannel);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetDevice()
******************************************************************************/
BERR_Code BSAT_g1_P_GetDevice(BSAT_ChannelHandle h, BSAT_Handle *pDev)
{
   BDBG_ENTER(BSAT_g1_P_GetDevice);

   *pDev = h->pDevice;

   BDBG_LEAVE(BSAT_g1_P_GetDevice);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetVersionInfo()
******************************************************************************/
BERR_Code BSAT_g1_P_GetVersionInfo(BSAT_Handle h, BFEC_VersionInfo *pVersion)
{
   BDBG_ENTER(BSAT_g1_P_GetVersionInfo);

   BSTD_UNUSED(h);

   pVersion->majorVersion = BSAT_API_VERSION;
   pVersion->minorVersion = BSAT_G1_MAJOR_VERSION;
   pVersion->buildId = BSAT_G1_MINOR_VERSION;
   pVersion->buildType = 0;

   BDBG_LEAVE(BSAT_g1_P_GetVersionInfo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_Reset()
******************************************************************************/
BERR_Code BSAT_g1_P_Reset(BSAT_Handle h)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);
   BSAT_ChannelHandle hChn;
   BERR_Code retCode = BERR_SUCCESS;
   int i;

   BDBG_ENTER(BSAT_g1_P_Reset);

   if (hDev->bResetDone)
      goto reset_done;

   for (i = 0; i < h->totalChannels; i++)
   {
      if (h->pChannels[i] != NULL)
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_SdsPowerUp(h->pChannels[i]));
         BSAT_CHK_RETCODE(BSAT_g1_P_TunerPowerUp(h->pChannels[i]));
      }
   }

   /* disable interrupts */
   BSAT_CHK_RETCODE(BSAT_g1_P_DisableInterrupts(h));

   BSAT_g1_P_InitHandle(h);

   for (i = 0; i < h->totalChannels; i++)
   {
      if (h->pChannels[i] != NULL)
      {
         hChn = h->pChannels[i];
         BSAT_CHK_RETCODE(BSAT_g1_P_InitChannel(hChn));
      }
   }

   for (i = 0; i < h->totalChannels; i++)
   {
      if (h->pChannels[i] != NULL)
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_SdsPowerDown(h->pChannels[i]));
         BSAT_CHK_RETCODE(BSAT_g1_P_TunerPowerDown(h->pChannels[i]));
      }
   }

   hDev->bResetDone = true;

   reset_done:
   BKNI_SetEvent(hDev->hInitDoneEvent);

   done:
   BDBG_LEAVE(BSAT_g1_P_Reset);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_PowerDownChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_PowerDownChannel(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_PowerDownChannel);

   retCode = BSAT_g1_P_ResetChannel(h, true); /* this turns of oif, afec, tfec */
   if ((retCode != BERR_SUCCESS) && (retCode != BSAT_ERR_POWERED_DOWN))
      goto done;
   BSAT_CHK_RETCODE(BSAT_g1_P_TunerPowerDown(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_SdsPowerDown(h));
   BKNI_SetEvent(hChn->hReadyEvent);

   done:
   BDBG_LEAVE(BSAT_g1_P_PowerDownChannel);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_PowerUpChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_PowerUpChannel(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_PowerUpChannel);

   BSAT_CHK_RETCODE(BSAT_g1_P_SdsPowerUp(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_TunerPowerUp(h));
   /* TBD: call BSAT_g1_P_TunerInit()... */
   BKNI_SetEvent(hChn->hReadyEvent);

   done:
   BDBG_LEAVE(BSAT_g1_P_PowerUpChannel);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_IsChannelOn()
******************************************************************************/
BERR_Code BSAT_g1_P_IsChannelOn(BSAT_ChannelHandle h, bool *pOn)
{
   BDBG_ENTER(BSAT_g1_P_IsChannelOn);

   *pOn = BSAT_g1_P_IsSdsOn(h);

   BDBG_LEAVE(BSAT_g1_P_IsChannelOn);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_Acquire()
******************************************************************************/
BERR_Code BSAT_g1_P_Acquire(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
#ifdef BSAT_HAS_DVBS2X
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
#endif
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_Acquire);

   BDBG_MSG(("BSAT_g1_P_Acquire(chan %d):", h->channel));
   BDBG_MSG(("   mode=0x%X", pParams->mode));
   BDBG_MSG(("   freq=%u", pParams->freq));
   BDBG_MSG(("   Fb=%u", pParams->symbolRate));
   BDBG_MSG(("   options=0x%X", pParams->options));
   BDBG_MSG(("   adcSelect=%d", pParams->adcSelect));

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (pParams->adcSelect >= 4)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidSymbolRate(pParams->symbolRate))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidFreq(pParams->freq))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (pParams->mode > BSAT_Mode_eBlindScan)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if ((!hChn->bHasAfec) && (BSAT_MODE_IS_DVBS2(pParams->mode)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if ((!hChn->bHasTfec) && (BSAT_MODE_IS_TURBO(pParams->mode)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

#ifdef BSAT_HAS_DVBS2X
   if ((pParams->mode == BSAT_Mode_eDvbs2_ACM) && (hDevImpl->sdsRevId < 0x74))
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
#else
   if (((pParams->mode >= BSAT_Mode_eDvbs2_16apsk_2_3) && (pParams->mode <= BSAT_Mode_eDvbs2_32apsk_9_10)) || (pParams->mode == BSAT_Mode_eDvbs2_ACM))
   {
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   }
   if (BSAT_MODE_IS_DVBS2X(pParams->mode))
   {
      BDBG_ERR(("DVB-S2X not supported"));
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   }

   if ((pParams->options & BSAT_ACQ_NYQUIST_MASK) == BSAT_ACQ_NYQUIST_5)
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
#endif

   BSAT_CHK_RETCODE(BSAT_g1_P_PrepareNewAcquisition(h));

   hChn->acqSettings = *pParams;
   hChn->operation = BSAT_Operation_eAcq;

   /* complete tuning and acquisition in ISR context */
   hChn->bAbortAcq = false;
   retCode = BSAT_g1_P_StartBackgroundAcq(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_Acquire);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetLockStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetLockStatus(BSAT_ChannelHandle h, bool *pbLocked, bool *pbAcqDone)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetLockStatus);

   *pbLocked = hChn->bLocked && (hChn->acqState == BSAT_AcqState_eMonitorLock);
   *pbAcqDone = hChn->bAcqDone;

   BDBG_LEAVE(BSAT_g1_P_GetLockStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetChannelStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetChannelStatus(BSAT_ChannelHandle h, BSAT_ChannelStatus *pStatus)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bAcqDone;
   BSAT_Mode actualMode;

   BDBG_ENTER(BSAT_g1_P_GetChannelStatus);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   BSAT_g1_P_GetLockStatus(h, &(pStatus->bDemodLocked), &bAcqDone);
   pStatus->bTunerLocked = true;
   BKNI_Memcpy(&(pStatus->acqSettings), &(hChn->acqSettings), sizeof(BSAT_AcqSettings));

   actualMode = hChn->actualMode;
   if (actualMode == BSAT_Mode_eUnknown)
   {
      if (pStatus->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
         actualMode = BSAT_Mode_eDvbs2_ACM;
   }
   pStatus->mode = actualMode;

   BSAT_CHK_RETCODE(BSAT_g1_P_GetSymbolRateError(h, &(pStatus->symbolRateError)));
   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_GetCarrierError_isrsafe(h, &(pStatus->carrierError));
   BKNI_LeaveCriticalSection();
   if (retCode != BERR_SUCCESS)
      pStatus->carrierError = 0;
   BSAT_CHK_RETCODE(BSAT_g1_P_UpdateMpegCount(h));
   pStatus->mpegCount = hChn->mpegFrameCount;
   pStatus->mpegErrors = hChn->mpegErrorCount;
   BSAT_CHK_RETCODE(BSAT_g1_P_GetSnr(h, &(pStatus->snr)));
   pStatus->outputBitrate = hChn->outputBitrate;
   pStatus->sampleFreq = hChn->sampleFreq;
   pStatus->reacqCount = hChn->reacqCount;
   pStatus->reacqCause = hChn->reacqCause;
   pStatus->aciBandwidth = hChn->aciBandwidth;
   pStatus->notification = hChn->notification;
   BSAT_CHK_RETCODE(BSAT_g1_P_GetAgcStatus(h, &(pStatus->agc)));

   BKNI_EnterCriticalSection();
   pStatus->bSpinv = BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsSpinv(h);
   BKNI_LeaveCriticalSection();

   if (BSAT_MODE_IS_LEGACY_QPSK(actualMode))
   {
      BKNI_EnterCriticalSection();
      retCode = BSAT_g1_P_QpskUpdateErrorCount_isrsafe(h);
      BKNI_LeaveCriticalSection();
      pStatus->modeStatus.qpsk = hChn->qpskStatus;
   }
#ifndef BSAT_EXCLUDE_AFEC
   else if (BSAT_MODE_IS_DVBS2(actualMode) || BSAT_MODE_IS_DVBS2X(actualMode))
   {
      BSAT_Dvbs2Status dvbs2;
      BSAT_g1_P_AfecGetStatus(h, &dvbs2);
      pStatus->modeStatus.dvbs2 = dvbs2;
      if (actualMode == BSAT_Mode_eDvbs2_ACM)
      {
         BSAT_g1_P_AcmStatus acm;
         if (BSAT_g1_P_HpGetAcmStatus(h, &acm) == BERR_SUCCESS)
         {
            pStatus->bSpinv = acm.bSpinv;
            pStatus->mode = acm.mode;
         }
      }
   }
#endif
#ifndef BSAT_EXCLUDE_TFEC
   else if (BSAT_MODE_IS_TURBO(actualMode))
   {
      BSAT_TurboStatus turbo;
      if (BSAT_g1_P_TfecGetStatus(h, &turbo) == BERR_SUCCESS)
         pStatus->modeStatus.turbo = turbo;
      else
         pStatus->modeStatus.turbo.bValid = false;
   }
#endif

   done:
   BDBG_LEAVE(BSAT_g1_P_GetChannelStatus);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_ResetChannelStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetChannelStatus(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_ResetChannelStatus);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   BKNI_EnterCriticalSection();
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetMpegCount_isrsafe(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_QpskUpdateErrorCount_isrsafe(h));
   BKNI_LeaveCriticalSection();

   /* reset the BER counters */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_BERT_BERCTL, 1<<8);

#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode) || BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode))
   {
      BKNI_EnterCriticalSection();
      BSAT_g1_P_AfecResetBlockCount_isrsafe(h);
      BKNI_LeaveCriticalSection();
   }
#endif
#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      BKNI_EnterCriticalSection();
      BSAT_g1_P_TfecResetBlockCount_isrsafe(h);
      BKNI_LeaveCriticalSection();
   }
#endif

   hChn->qpskStatus.bValid = true;
   hChn->qpskStatus.rsCorrCount = 0;
   hChn->qpskStatus.rsUncorrCount = 0;
   hChn->qpskStatus.preVitErrCount = 0;
   hChn->reacqCause = 0;
   hChn->reacqCount = 0;

   done:
   BDBG_LEAVE(BSAT_g1_P_ResetChannelStatus);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetSoftDecisions()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSoftDecisions(BSAT_ChannelHandle h, uint32_t n, int16_t *pI, int16_t *pQ)
{
   uint32_t val, i;
#if defined(BSAT_HAS_ACM) && defined(BCHP_SDS_EQ_0_EQSFT_CTL)
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
#endif

   BDBG_ENTER(BSAT_g1_P_GetSoftDecisions);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   for (i = 0; i < n; i++)
   {
#ifdef BCHP_SDS_EQ_0_EQSFT_invalid_MASK
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQSFT);
      if (val & BCHP_SDS_EQ_0_EQSFT_invalid_MASK)
         val = 0;
#else
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQSFT);
#endif

      pQ[i] = (int16_t)((val >> 24) & 0xFF);
      if (pQ[i] & 0x80)
         pQ[i] -= 0x100;

      pI[i] = (int16_t)((val >> 16) & 0xFF);
      if (pI[i] & 0x80)
         pI[i] -= 0x100;
   }

   BDBG_LEAVE(BSAT_g1_P_GetSoftDecisions);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetChannel(BSAT_ChannelHandle h, bool bDisableDemod)
{
#ifdef BSAT_HAS_DVBS2X
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
#endif
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   BDBG_ENTER(BSAT_g1_P_ResetChannel);

/* bEnableDebugLog = false; */

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   hChn->bAbortAcq = true;

   BSAT_CHK_RETCODE(BSAT_g1_P_DisableChannelInterrupts(h));

   hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME] = 0;
   hChn->acqState = BSAT_AcqState_eIdle;

   if (bDisableDemod)
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL);
      if (val & BCHP_SDS_HP_0_HPCONTROL_HPEN_MASK)
      {
         /* HP was enabled */
         /* per Hiroshi: reset HP first, wait a while (say 5 msec), then reset FEC and OI.  5ms should be long enough to flush out the FEC to avoid partial XPT packets */
         BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable(h, false));
         BKNI_Sleep(5);
      }
      else
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable(h, false));
      }

#ifndef BSAT_EXCLUDE_TFEC
      if (hChn->bHasTfec)
      {
#ifdef BSAT_HAS_DUAL_TFEC
         BKNI_EnterCriticalSection();
         if (BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(h) == false)
            BSAT_g1_P_TfecPowerDown_isrsafe(h);
         BKNI_LeaveCriticalSection();
#else
         BSAT_CHK_RETCODE(BSAT_g1_P_TfecPowerDown_isrsafe(h));
#endif
      }
#endif
#ifndef BSAT_EXCLUDE_AFEC
      if (hChn->bHasAfec)
      {
         if (BSAT_g1_P_IsAfecOn_isrsafe(h))
         {
#ifdef BSAT_HAS_DVBS2X
            BSAT_CHK_RETCODE(BSAT_g1_P_AfecPowerDown_isrsafe(h));
#else
            BKNI_EnterCriticalSection();
            BSAT_g1_P_AfecEnableChannel_isrsafe(h, false);
            if (BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe(h) == false)
               BSAT_g1_P_AfecPowerDown_isrsafe(h);
            BKNI_LeaveCriticalSection();
#endif
         }
      }
#endif


#if 0 /* keep SDS_CG_PMCG_CTL.xpt_bclk_en to its default value (=1) */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CG_PMCG_CTL, ~BCHP_SDS_CG_0_PMCG_CTL_xpt_bclk_en_MASK);
#endif

      BSAT_g1_P_PowerDownOpll_isrsafe(h);
   }

   BSAT_g1_P_IndicateNotLocked_isrsafe(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_ResetChannel);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetBertSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetBertSettings);

   BKNI_Memcpy(&(hChn->bertSettings), pSettings, sizeof(BSAT_BertSettings));

   BDBG_LEAVE(BSAT_g1_P_SetBertSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetBertSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetBertSettings);

   BKNI_Memcpy(pSettings, &(hChn->bertSettings), sizeof(BSAT_BertSettings));

   BDBG_LEAVE(BSAT_g1_P_GetBertSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetBertStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetBertStatus(BSAT_ChannelHandle h, BSAT_BertStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   bool bDemodLocked, bAcqDone;

   BDBG_ENTER(BSAT_g1_P_GetBertStatus);

   pStatus->bEnabled = hChn->acqSettings.options & BSAT_ACQ_ENABLE_BERT ? true : false;
   if (pStatus->bEnabled)
   {
      /* take snapshot */
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_BERT_BERCTL, BCHP_SDS_BERT_0_BERCTL_BERT_SNAP_MASK);
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BEST);

      BSAT_g1_P_GetLockStatus(h, &bDemodLocked, &bAcqDone);

      pStatus->bLock = (bDemodLocked && (val & (1<<25))) ? true : false;
      pStatus->bSyncLost = (val & (1<<26)) ? true : false;
      pStatus->bSyncAcquired = (val & (1<<27)) ? true : false;
      pStatus->bitCountHi = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BIT_CNT_H);
      pStatus->bitCountLo = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BIT_CNT_L);
      pStatus->errorCountHi = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_ERR_CNT_H);
      pStatus->errorCountLo = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_ERR_CNT_L);
   }
   else
      BKNI_Memset(pStatus, 0, sizeof(BSAT_BertStatus));

   BDBG_LEAVE(BSAT_g1_P_GetBertStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetSearchRange()
******************************************************************************/
BERR_Code BSAT_g1_P_SetSearchRange(BSAT_ChannelHandle h, uint32_t searchRange)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetSearchRange);

   hChn->searchRange = searchRange;

   BDBG_LEAVE(BSAT_g1_P_SetSearchRange);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetSearchRange()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSearchRange(BSAT_ChannelHandle h, uint32_t *pSearchRange)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetSearchRange);

   *pSearchRange = hChn->searchRange;

   BDBG_LEAVE(BSAT_g1_P_GetSearchRange);
   return BERR_SUCCESS;
}


#ifndef BSAT_EXCLUDE_AFEC
/******************************************************************************
 BSAT_g1_P_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BSAT_g1_P_SetAmcScramblingSeq(BSAT_ChannelHandle h, BSAT_AmcScramblingSeq *pSeq)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetAmcScramblingSeq);

   BKNI_Memcpy(&(hChn->scramblingSeq), pSeq, sizeof(BSAT_AmcScramblingSeq));

   BDBG_LEAVE(BSAT_g1_P_SetAmcScramblingSeq);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_SetAmcScramblingSeq(BSAT_ChannelHandle h, BSAT_AmcScramblingSeq *pSeq)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSeq);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_SetNetworkSpec()
******************************************************************************/
BERR_Code BSAT_g1_P_SetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec networkSpec)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_g1_P_SetNetworkSpec);

   hDev->networkSpec = networkSpec;

   BDBG_LEAVE(BSAT_g1_P_SetNetworkSpec);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetNetworkSpec_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetNetworkSpec_isrsafe(BSAT_Handle h, BSAT_NetworkSpec *pNetworkSpec)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_g1_P_GetNetworkSpec_isrsafe);

   *pNetworkSpec = hDev->networkSpec;

   BDBG_LEAVE(BSAT_g1_P_GetNetworkSpec_isrsafe);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetOutputTransportSettings);

   BKNI_Memcpy(&(hChn->xportSettings), pSettings, sizeof(BSAT_OutputTransportSettings));

   BDBG_LEAVE(BSAT_g1_P_SetOutputTransportSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetOutputTransportSettings);

   BKNI_Memcpy(pSettings, &(hChn->xportSettings), sizeof(BSAT_OutputTransportSettings));

   BDBG_LEAVE(BSAT_g1_P_GetOutputTransportSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetInitDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_g1_P_GetInitDoneEventHandle(BSAT_Handle h, BKNI_EventHandle *hEvent)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_g1_P_GetInitDoneEventHandle);

   *hEvent = hDev->hInitDoneEvent;

   BDBG_LEAVE(BSAT_g1_P_GetInitDoneEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BSAT_g1_P_GetLockStateChangeEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetLockStateChangeEventHandle);

   *hEvent = hChn->hLockChangeEvent;

   BDBG_LEAVE(BSAT_g1_P_GetLockStateChangeEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAcqDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_g1_P_GetAcqDoneEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetAcqDoneEventHandle);

   *hEvent = hChn->hAcqDoneEvent;

   BDBG_LEAVE(BSAT_g1_P_GetAcqDoneEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetSignalNotificationEventHandle()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSignalNotificationEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetSignalNotificationEventHandle);

   *hEvent = hChn->hSignalNotificationEvent;

   BDBG_LEAVE(BSAT_g1_P_GetSignalNotificationEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetReadyEventHandle()
******************************************************************************/
BERR_Code BSAT_g1_P_GetReadyEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetReadyEventHandle);

   *hEvent = hChn->hReadyEvent;

   BDBG_LEAVE(BSAT_g1_P_GetReadyEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_StartToneDetect()
******************************************************************************/
BERR_Code BSAT_g1_P_StartToneDetect(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_StartToneDetect);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (adcSelect >= 4)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidFreq(freq))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BSAT_CHK_RETCODE(BSAT_g1_P_PrepareNewAcquisition(h));

   hChn->acqSettings.freq = freq;
   hChn->acqSettings.adcSelect = adcSelect;
   hChn->acqSettings.symbolRate = 20000000;
   hChn->acqSettings.options = 0;
   hChn->operation = BSAT_Operation_eToneDetect;
   hChn->toneDetectStatus.tunerFreq = freq;
   hChn->toneDetectStatus.bEnabled = true;

   /* complete tuning and acquisition in ISR context */
   retCode = BSAT_g1_P_StartBackgroundAcq(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_StartToneDetect);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_StartPsdScan()
******************************************************************************/
BERR_Code BSAT_g1_P_StartPsdScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_StartPsdScan);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (adcSelect >= 4)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidFreq(freq))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BSAT_CHK_RETCODE(BSAT_g1_P_PrepareNewAcquisition(h));

   hChn->acqSettings.freq = freq;
   hChn->acqSettings.adcSelect = adcSelect;
   hChn->acqSettings.symbolRate = 4000000;
   hChn->acqSettings.options = 0;
   hChn->operation = BSAT_Operation_ePsd;
   hChn->psdScanStatus.tunerFreq = freq;
   hChn->psdScanStatus.bEnabled = true;

   /* complete tuning and acquisition in ISR context */
   retCode = BSAT_g1_P_StartBackgroundAcq(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_StartPsdScan);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetToneDetectStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetToneDetectStatus(BSAT_ChannelHandle h, BSAT_ToneDetectStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetToneDetectStatus);
   BKNI_Memcpy((void*)pStatus, (void*)&(hChn->toneDetectStatus), sizeof(BSAT_ToneDetectStatus));
   BDBG_LEAVE(BSAT_g1_P_GetToneDetectStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetPsdScanStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetPsdScanStatus(BSAT_ChannelHandle h, BSAT_PsdScanStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetPsdScanStatus);
   BKNI_Memcpy((void*)pStatus, (void*)&(hChn->psdScanStatus), sizeof(BSAT_PsdScanStatus));
   BDBG_LEAVE(BSAT_g1_P_GetPsdScanStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_StartSymbolRateScan()
******************************************************************************/
BERR_Code BSAT_g1_P_StartSymbolRateScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t minSymbolRate, uint32_t maxSymbolRate, uint32_t adcSelect)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_StartSymbolRateScan);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (adcSelect >= 4)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidFreq(freq))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidSymbolRate(minSymbolRate) || !BSAT_g1_P_IsValidSymbolRate(maxSymbolRate) || (minSymbolRate >= maxSymbolRate))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BSAT_CHK_RETCODE(BSAT_g1_P_PrepareNewAcquisition(h));

   hChn->acqSettings.freq = freq;
   hChn->acqSettings.adcSelect = adcSelect;
   hChn->acqSettings.symbolRate = minSymbolRate * 2;
   hChn->acqSettings.options = 0;
   hChn->operation = BSAT_Operation_eSymbolRateScan;
   hChn->symbolRateScanStatus.bEnabled = true;
   hChn->symbolRateScanStatus.bScanDone = false;
   hChn->symbolRateScanStatus.tunerFreq = freq;
   hChn->symbolRateScanStatus.symbolRate = minSymbolRate;
   hChn->symbolRateScanStatus.minSymbolRate = minSymbolRate;
   hChn->symbolRateScanStatus.maxSymbolRate = maxSymbolRate;

   /* complete tuning and acquisition in ISR context */
   retCode = BSAT_g1_P_StartBackgroundAcq(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_StartSymbolRateScan);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetSymbolRateScanStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSymbolRateScanStatus(BSAT_ChannelHandle h, BSAT_SymbolRateScanStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetSymbolRateScanStatus);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   BKNI_Memcpy((void*)pStatus, (void*)&(hChn->symbolRateScanStatus), sizeof(BSAT_SymbolRateScanStatus));

   BDBG_LEAVE(BSAT_g1_P_GetSymbolRateScanStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetConfig()
******************************************************************************/
BERR_Code BSAT_g1_P_SetConfig(BSAT_Handle h, uint32_t addr, uint32_t val)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(val);

   BDBG_ENTER(BSAT_g1_P_SetConfig);
   BDBG_LEAVE(BSAT_g1_P_SetConfig);
   return BSAT_ERR_NOT_IMPLEMENTED;
}


/******************************************************************************
 BSAT_g1_P_GetConfig()
******************************************************************************/
BERR_Code BSAT_g1_P_GetConfig(BSAT_Handle h, uint32_t addr, uint32_t *pVal)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(addr);
   BSTD_UNUSED(pVal);

   BDBG_ENTER(BSAT_g1_P_GetConfig);
   BDBG_LEAVE(BSAT_g1_P_GetConfig);
   return BSAT_ERR_NOT_IMPLEMENTED;
}


/******************************************************************************
 BSAT_g1_P_SetChannelConfig()
******************************************************************************/
BERR_Code BSAT_g1_P_SetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t val)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ENTER(BSAT_g1_P_SetChannelConfig);

   if (addr >= BSAT_g1_CONFIG_MAX)
      retCode = BERR_INVALID_PARAMETER;
   else
      hChn->configParam[addr] = val;

   BDBG_LEAVE(BSAT_g1_P_SetChannelConfig);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetChannelConfig()
******************************************************************************/
BERR_Code BSAT_g1_P_GetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t *pVal)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ENTER(BSAT_g1_P_GetChannelConfig);

   if (addr >= BSAT_g1_CONFIG_MAX)
      retCode = BERR_INVALID_PARAMETER;
   else
      *pVal = hChn->configParam[addr];

   BDBG_LEAVE(BSAT_g1_P_GetChannelConfig);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetAcqDoneEventSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetAcqDoneEventSettings(BSAT_Handle h, uint8_t reacqs)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);

   BDBG_ENTER(BSAT_g1_P_SetAcqDoneEventSettings);

   hDev->acqDoneThreshold = reacqs;

   BDBG_LEAVE(BSAT_g1_P_SetAcqDoneEventSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetSignalNotificationSettings);

   BKNI_Memcpy(&(hChn->sigNotSettings), pSettings, sizeof(BSAT_SignalNotificationSettings));

   BDBG_LEAVE(BSAT_g1_P_SetSignalNotificationSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetSignalNotificationSettings);

   BKNI_Memcpy(pSettings, &(hChn->sigNotSettings), sizeof(BSAT_SignalNotificationSettings));

   BDBG_LEAVE(BSAT_g1_P_GetSignalNotificationSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetACIBandwidth()
******************************************************************************/
BERR_Code BSAT_g1_P_SetACIBandwidth(BSAT_ChannelHandle h, uint32_t bw)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ENTER(BSAT_g1_P_SetACIBandwidth);

   if (bw == 0)
      hChn->bOverrideAciBandwidth = false;
   else
   {
      if (BSAT_g1_P_IsValidSymbolRate(bw))
      {
         hChn->bOverrideAciBandwidth = true;
         hChn->aciBandwidth = bw;
      }
      else
         retCode = BERR_INVALID_PARAMETER;
   }

   BDBG_LEAVE(BSAT_g1_P_SetACIBandwidth);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_StartSignalDetect()
******************************************************************************/
BERR_Code BSAT_g1_P_StartSignalDetect(BSAT_ChannelHandle h, uint32_t symbolRate, uint32_t freq, uint32_t adcSelect)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BDBG_ENTER(BSAT_g1_P_StartSignalDetect);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (adcSelect >= 4)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidFreq(freq))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   if (!BSAT_g1_P_IsValidSymbolRate(symbolRate))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BSAT_CHK_RETCODE(BSAT_g1_P_PrepareNewAcquisition(h));

   hChn->acqSettings.freq = freq;
   hChn->acqSettings.adcSelect = adcSelect;
   hChn->acqSettings.symbolRate = symbolRate;
   hChn->acqSettings.mode = BSAT_Mode_eDvbs_scan;
   hChn->acqSettings.options = 0;
   hChn->operation = BSAT_Operation_eSignalDetect;
   hChn->signalDetectStatus.bEnabled = true;

   /* complete tuning and acquisition in ISR context */
   retCode = BSAT_g1_P_StartBackgroundAcq(h);

   done:
   BDBG_LEAVE(BSAT_g1_P_StartSignalDetect);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSignalDetectStatus(BSAT_ChannelHandle h, BSAT_SignalDetectStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetSignalDetectStatus);

   if (BSAT_g1_P_IsSdsOn(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   BKNI_Memcpy((void*)pStatus, (void*)&(hChn->signalDetectStatus), sizeof(BSAT_SignalDetectStatus));

   BDBG_LEAVE(BSAT_g1_P_GetSignalDetectStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetLegacyQpskAcqSettings);

   BKNI_Memcpy(pSettings, &(hChn->qpskSettings), sizeof(BSAT_LegacyQpskAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_GetLegacyQpskAcqSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetLegacyQpskAcqSettings);

   BKNI_Memcpy(&(hChn->qpskSettings), pSettings, sizeof(BSAT_LegacyQpskAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_SetLegacyQpskAcqSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetExtAcqSettings);

   BKNI_Memcpy(pSettings, &(hChn->miscSettings), sizeof(BSAT_ExtAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_GetExtAcqSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetExtAcqSettings);

   BKNI_Memcpy(&(hChn->miscSettings), pSettings, sizeof(BSAT_ExtAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_SetExtAcqSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetDvbs2AcqSettings()
******************************************************************************/
#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_GetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetDvbs2AcqSettings);

   BKNI_Memcpy(pSettings, &(hChn->dvbs2Settings), sizeof(BSAT_Dvbs2AcqSettings));

   BDBG_LEAVE(BSAT_g1_P_GetDvbs2AcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_GetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_SetDvbs2AcqSettings()
******************************************************************************/
#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_SetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
#ifdef BSAT_HAS_ACM /* 45308-B0 or later */
   bool bOn;
#endif

   BDBG_ENTER(BSAT_g1_P_SetDvbs2AcqSettings);

   BKNI_Memcpy(&(hChn->dvbs2Settings), pSettings, sizeof(BSAT_Dvbs2AcqSettings));

#ifdef BSAT_HAS_ACM /* 45308-B0 or later */
   BKNI_EnterCriticalSection();
   if (BSAT_IsChannelOn(h, &bOn) == BERR_SUCCESS)
   {
      if (bOn)
      {
         bOn = BSAT_g1_P_IsAfecOn_isrsafe(h);
         if (bOn)
         {
            if (pSettings->ctl & BSAT_DVBS2_CTL_SEL_UPL)
               BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFEC_BCH_BBHDR3, 1<<17);
            else
               BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_BCH_BBHDR3, ~(1<<17));
         }
      }
   }
   BKNI_LeaveCriticalSection();
#endif

   BDBG_LEAVE(BSAT_g1_P_SetDvbs2AcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_SetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif /* BSAT_EXCLUDE_AFEC */


/******************************************************************************
 BSAT_g1_P_GetDvbs2xAcqSettings()
******************************************************************************/
#ifdef BSAT_HAS_DVBS2X
BERR_Code BSAT_g1_P_GetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetDvbs2xAcqSettings);

   BKNI_Memcpy(pSettings, &(hChn->dvbs2xSettings), sizeof(BSAT_Dvbs2xAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_GetDvbs2xAcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_GetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_SetDvbs2xAcqSettings()
******************************************************************************/
#ifdef BSAT_HAS_DVBS2X
BERR_Code BSAT_g1_P_SetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetDvbs2xAcqSettings);

   BKNI_Memcpy(&(hChn->dvbs2xSettings), pSettings, sizeof(BSAT_Dvbs2xAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_SetDvbs2xAcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_SetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif /* BSAT_EXCLUDE_AFEC */


/******************************************************************************
 BSAT_g1_P_GetTurboAcqSettings()
******************************************************************************/
#ifndef BSAT_EXCLUDE_TFEC
BERR_Code BSAT_g1_P_GetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_GetTurboAcqSettings);

   BKNI_Memcpy(pSettings, &(hChn->turboSettings), sizeof(BSAT_TurboAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_GetTurboAcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_GetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_SetTurboAcqSettings()
******************************************************************************/
#ifndef BSAT_EXCLUDE_TFEC
BERR_Code BSAT_g1_P_SetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BSAT_g1_P_SetTurboAcqSettings);

   BKNI_Memcpy(&(hChn->turboSettings), pSettings, sizeof(BSAT_TurboAcqSettings));

   BDBG_LEAVE(BSAT_g1_P_SetTurboAcqSettings);
   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_SetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_SetAcmSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_SetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings)
{
#ifdef BSAT_HAS_ACM
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bOn;
   uint8_t pls;
#endif
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ENTER(BSAT_g1_P_SetAcmSettings);

#ifdef BSAT_HAS_ACM
   if ((pSettings->softDecisions.captureMode > BSAT_AcmSoftDecisionCapture_eModulation) ||
       (pSettings->snr.filterMode > BSAT_AcmSnrFilter_eModulationLE))
      retCode = BERR_INVALID_PARAMETER;
   else if (((pSettings->softDecisions.captureMode == BSAT_AcmSoftDecisionCapture_eModulation) && ((uint32_t)pSettings->softDecisions.param.modulation > (uint32_t)BSAT_Modulation_e32apsk)) ||
            ((pSettings->snr.filterMode >= BSAT_AcmSnrFilter_eModulation) && ((uint32_t)pSettings->snr.param.modulation > (uint32_t)BSAT_Modulation_e32apsk)))
      retCode = BERR_INVALID_PARAMETER;
   if ((retCode == BERR_SUCCESS) && (pSettings->softDecisions.captureMode == BSAT_AcmSoftDecisionCapture_eMode))
      retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(pSettings->softDecisions.param.mode, &pls); /* validate mode */
   if ((retCode == BERR_SUCCESS) && (pSettings->snr.filterMode == BSAT_AcmSnrFilter_eMode))
      retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(pSettings->snr.param.mode, &pls); /* validate mode */
   if ((retCode == BERR_SUCCESS) && pSettings->bert.bEnable && (pSettings->bert.bFilterByStreamId == false))
      retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(pSettings->bert.param.mode, &pls); /* validate mode */
   if (retCode == BERR_SUCCESS)
   {
      BKNI_Memcpy(&(hChn->acmSettings), pSettings, sizeof(BSAT_AcmSettings));
#if 0
      BDBG_ERR(("softDecision.captureMode=%d", hChn->acmSettings.softDecisions.captureMode));
      if (hChn->acmSettings.softDecisions.captureMode == BSAT_AcmSoftDecisionCapture_eMode)
      {
         BDBG_ERR(("softDecision.mode=0x%X", hChn->acmSettings.softDecisions.param.mode));
      }
      else if (hChn->acmSettings.softDecisions.captureMode == BSAT_AcmSoftDecisionCapture_eModulation)
      {
         BDBG_ERR(("softDecision.modulation=%d", hChn->acmSettings.softDecisions.param.modulation));
      }
#endif
      BKNI_EnterCriticalSection();
      if (BSAT_IsChannelOn(h, &bOn) == BERR_SUCCESS)
      {
         if (bOn)
            BSAT_g1_P_AfecSetEqsftctl_isrsafe(h);
      }
      BKNI_LeaveCriticalSection();
   }
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
   retCode = BERR_NOT_SUPPORTED;
#endif
   BDBG_LEAVE(BSAT_g1_P_SetAcmSettings);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetAcmSettings()
******************************************************************************/
BERR_Code BSAT_g1_P_GetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings)
{
#ifdef BSAT_HAS_ACM
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
#endif
   BERR_Code retCode = BERR_NOT_SUPPORTED;

   BDBG_ENTER(BSAT_g1_P_GetAcmSettings);

#ifdef BSAT_HAS_ACM
   BKNI_Memcpy(pSettings, &(hChn->acmSettings), sizeof(BSAT_AcmSettings));
   retCode = BERR_SUCCESS;
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(pSettings);
#endif
   BDBG_LEAVE(BSAT_g1_P_GetAcmSettings);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetNotchSettings()
******************************************************************************/
#ifdef BSAT_HAS_WFE
BERR_Code BSAT_g1_P_SetNotchSettings(BSAT_Handle h, uint32_t n, BSAT_NotchSettings *pSettings)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)h->pImpl;
   uint8_t i;

   BDBG_ASSERT(h);

   if (n > BSAT_G1_MAX_NOTCH)
      return BERR_INVALID_PARAMETER;

   pDev->nNotch = n;
   for (i = 0; i < BSAT_G1_MAX_NOTCH; i++)
   {
      if (i < n)
      {
         BKNI_Memcpy(&(pDev->notchSettings[i]), (const void*)&pSettings[i], sizeof(BSAT_NotchSettings));
      }
   }

   if (n == 0)
   {
      for (i = 0; i < BSAT_G1_MAX_CHANNELS; i++)
      {
         if (h->pChannels[i] == NULL)
            continue;
         BSAT_g1_P_DisableAllNotch_isrsafe(h->pChannels[i]);
      }
   }

   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_SetNotchSettings(BSAT_Handle h, uint32_t n, BSAT_NotchSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(n);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_GetNotchSettings()
******************************************************************************/
#ifdef BSAT_HAS_WFE
BERR_Code BSAT_g1_P_GetNotchSettings(BSAT_Handle h, uint32_t *pNum, BSAT_NotchSettings *pSettings)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)h->pImpl;
   uint8_t i;

   BDBG_ASSERT(h);

   *pNum = pDev->nNotch;
   if (pSettings)
   {
      for (i = 0; i < pDev->nNotch; i++)
      {
         BKNI_Memcpy(&pSettings[i], &(pDev->notchSettings[i]), sizeof(BSAT_NotchSettings));
      }
   }

   return BERR_SUCCESS;
}
#elif defined(BSAT_EXCLUDE_API_TABLE)
BERR_Code BSAT_g1_P_GetNotchSettings(BSAT_Handle h, uint32_t *pNum, BSAT_NotchSettings *pSettings)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pNum);
   BSTD_UNUSED(pSettings);
   return BERR_NOT_SUPPORTED;
}
#endif


/******************************************************************************
 BSAT_g1_P_GetNotchSettings()
******************************************************************************/
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
BERR_Code BSAT_g1_P_SetCwc(BSAT_Handle h, uint32_t n, uint32_t *pFreqs)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)h->pImpl;
   uint32_t i;

   for (i = 0; i < 6; i++)
   {
      if ((n > 0) && (i < n))
         pDev->cwcFreq[i] = pFreqs[i];
      else
         pDev->cwcFreq[i] = 0;
   }
   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BSAT_g1_P_GetStreamList()
******************************************************************************/
BERR_Code BSAT_g1_P_GetStreamList(BSAT_ChannelHandle h, int bufsize, int *pNumStreams, uint8_t *pStreamIdList)
{
#if defined(BSAT_HAS_ACM) && defined(BCHP_SDS_OI_0_MPEG_CTL0) && !defined(BSAT_EXCLUDE_AFEC)
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_Mode mode;
   uint32_t stat[8], i;
   uint8_t pls, streamId;

   if (bufsize < 1)
      return BERR_INVALID_PARAMETER;

   *pNumStreams = 0;
   BKNI_EnterCriticalSection();

   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      for (i = 0; i < 8; i++)
         BSAT_g1_P_GetStreamRegStat_isrsafe(h, i, &stat[i]);
      BKNI_LeaveCriticalSection();

      for (i = 0; (i < 8) && (*pNumStreams < bufsize); i++)
      {
         streamId = (uint8_t)((stat[i] & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_SHIFT);
         pls = (stat[i] & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_SHIFT;
         if (((stat[i] & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_avail_MASK) == 0) && (streamId != 0xFF))
         {
            if (BSAT_g1_P_GetDvbs2ModeFromPls_isrsafe(pls, &mode) == BERR_SUCCESS)
            {
               pStreamIdList[*pNumStreams] = streamId;
               *pNumStreams = *pNumStreams + 1;
            }
            else
            {
               BDBG_WRN(("BSAT_g1_P_GetStreamList: Invalid plscode (0x%X)", pls));
            }
         }
      }
   }
   else
   {
      BKNI_LeaveCriticalSection();
      retCode = BSAT_ERR_POWERED_DOWN;
   }
   return retCode;
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(bufsize);
   BSTD_UNUSED(pNumStreams);
   BSTD_UNUSED(pStreamIdList);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSAT_g1_P_GetStreamStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_GetStreamStatus(BSAT_ChannelHandle h, uint8_t streamId, BSAT_StreamStatus *pStatus)
{
#if defined(BSAT_HAS_ACM) && defined(BCHP_SDS_OI_0_MPEG_CTL0) && !defined(BSAT_EXCLUDE_AFEC)
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t stat = 0, addr, val = 0, shift;
   int i, pls_idx = -1, sid_idx = -1, mp_idx = -1;
   bool bFound = false, bRetry = false;
   BSAT_Mode mode;
   uint8_t sid, pls = 0, val8;

   if (streamId == 0xFF)
      return BERR_INVALID_PARAMETER;

   BKNI_Memset((void*)pStatus, 0, sizeof(BSAT_StreamStatus));

   BKNI_EnterCriticalSection();
   if (BSAT_g1_P_IsHpLocked_isr(h) == 0)
      goto stream_not_found;
   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      for (i = 0; !bFound && (i < 8); i++)
      {
         BSAT_g1_P_GetStreamRegStat_isrsafe(h, i, &stat);
         sid = (uint8_t)((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_SHIFT);
         pls = (stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_SHIFT;
         if ((streamId == sid) && ((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_avail_MASK) == 0))
         {
            if (BSAT_g1_P_GetDvbs2ModeFromPls_isrsafe(pls, &mode) == BERR_SUCCESS)
            {
               bFound = true;
               sid_idx = i;
               break;
            }
         }
      }

      if (!bFound)
      {
         stream_not_found:
         retCode = BSAT_ERR_INVALID_STREAM_ID;
         goto done;
      }

      pStatus->bValid = (stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_avail_MASK) ? false : true;
      pStatus->matype = (uint8_t)((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_matype_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_matype_SHIFT);
      pStatus->pls = (uint8_t)((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_PLS_SHIFT);
      pStatus->streamId = (uint8_t)((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_SHIFT);
      pStatus->mode = mode;
      pStatus->lock = 0;

      /* find which slot the pls takes up in AFEC_STATS_PLS_ACTUAL */
      for (i = 0, bFound = false; !bFound && (i < 8); i++)
      {
         if ((i == 0) || (i == 4))
            val = BSAT_g1_P_ReadRegister_isrsafe(h, i < 4 ? BCHP_AFEC_STATS_PLS_ACTUAL0 : BCHP_AFEC_STATS_PLS_ACTUAL1);
         shift = (i - (i < 4 ? 0 : 4)) * 8;
         val8 = (val >> shift) & 0xFF;
         if (((val8 ^ pls) & 0xFE) == 0)
         {
            bFound = true;
            pls_idx = i;
            break;
         }
      }

      if (bFound)
      {
         addr = afec_pls_ldpc_iter_cnt_reg[pls_idx];
         pStatus->iterationCount = BSAT_g1_P_ReadRegister_isrsafe(h, addr);
         pStatus->ldpcErrorCount = BSAT_g1_P_ReadRegister_isrsafe(h, addr+4);
         pStatus->ldpcFrameCount = BSAT_g1_P_ReadRegister_isrsafe(h, addr+8);

         addr = afec_pls_bch_decnblk_reg[pls_idx];
         pStatus->totalBlocks = BSAT_g1_P_ReadRegister_isrsafe(h, addr);
         pStatus->corrBlocks = BSAT_g1_P_ReadRegister_isrsafe(h, addr+4);
         pStatus->badBlocks = BSAT_g1_P_ReadRegister_isrsafe(h, addr+8);
         pStatus->corrBits = BSAT_g1_P_ReadRegister_isrsafe(h, addr+12);

         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
         if ((val >> pls_idx) & 1)
            pStatus->lock = 1; /* this stream is LDPC_LOCK'd */
      }
      else
      {
         BDBG_WRN(("pls 0x%X not found in AFEC_STATS_PLS_ACTUAL", pls));
      }

      /* find which slot the streamId is in SDS_OI_MPEG_CTL1/2 */
      retry:
      for (i = 0, bFound = false; !bFound && (i < 8); i++)
      {
         if ((i == 0) || (i == 4))
            val = BSAT_g1_P_ReadRegister_isrsafe(h, i < 4 ? BCHP_SDS_OI_MPEG_CTL1 : BCHP_SDS_OI_MPEG_CTL2);
         shift = (i - (i < 4 ? 0 : 4)) * 8;
         if ((uint8_t)((val >> shift) & 0xFF) == streamId)
         {
            bFound = true;
            mp_idx = i;
         }
      }

      if (bFound)
      {
         val = mp_idx << BCHP_SDS_OI_0_MPEG_CTL0_mpeg_counter_select_SHIFT;
         val |= BCHP_SDS_OI_0_MPEG_CTL0_mpeg_counter_mode_MASK; /* multi-stream mode */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_MPEG_CTL0, val);
         BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_MPEG_CTL0, BCHP_SDS_OI_0_MPEG_CTL0_mpeg_counter_snap_MASK); /* take snapshot */
         hChn->mpegFrameCountMs[mp_idx] += BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FRC);
         hChn->mpegErrorCountMs[mp_idx] += BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FERC);
         pStatus->mpegErrorCount = hChn->mpegErrorCountMs[mp_idx];
         pStatus->mpegFrameCount = hChn->mpegFrameCountMs[mp_idx];
      }
      else if (pStatus->lock)
      {
         BDBG_WRN(("locked streamId 0x%X not found in SDS_OI_MPEG_CTL1/2", streamId));
         if (!bRetry)
         {
            bRetry = true;
            BDBG_WRN(("reprogramming stream IDs for mpeg counters..."));
            BSAT_g1_P_AfecUpdateStreamIdsForMpegCounters_isrsafe(h);
            goto retry;
         }
      }
   }
   else
      retCode = BSAT_ERR_POWERED_DOWN;

   /* BDBG_ERR(("sid_idx=%d, pls_idx=%d, mp_idx=%d", sid_idx, pls_idx, mp_idx)); */

   done:
   BKNI_LeaveCriticalSection();
   return retCode;
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(streamId);
   BSTD_UNUSED(pStatus);
   return BERR_NOT_SUPPORTED;
#endif
}
