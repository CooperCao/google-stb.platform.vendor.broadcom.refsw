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
 
#include "bdsq.h"
#include "bdsq_priv.h"
#include "bdsq_g1_priv.h"

BDBG_MODULE(bdsq_g1_priv_txn);


#ifndef BDSQ_INCLUDE_VSENSE_ONLY
/******************************************************************************
 BDSQ_g1_Txn_P_GetOddParity_isrsafe()
******************************************************************************/
bool BDSQ_g1_Txn_P_GetOddParity_isrsafe(uint8_t data)
{
   uint8_t count = 0;
   while (data > 0)
   {
      /* accumulate set bits */
      if ((data & 1) == 1)
         count++;
      data >>= 1;
   }
   return (count & 1) ? false : true;
}
#endif


/******************************************************************************
 BDSQ_g1_Txn_P_UpdateSettings() - Non-ISR context
******************************************************************************/
BERR_Code BDSQ_g1_Txn_P_UpdateSettings(BDSQ_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   
   /* set envelope or tone mode */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DDIO, &val);
   val &= ~0x00700000;
   if (h->settings.bEnvelope)
      val |= 0x00100000;

   /* select LNBPU signal to go out on TXEN */
   if (hChn->configParam[BDSQ_g1_CONFIG_ENABLE_LNBPU])
      val |= 0x10000000;
   else
      val &= ~0x10000000;
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DDIO, val);
   
   /* tone align enable or disable */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &val);
   if (h->settings.bToneAlign)
      val |= 0x00000800;
   else
      val &= ~0x00000800;
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, val);
   
   /* receive reply timeout enable or disable */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &val);
   if (h->settings.bDisableRRTO)
      val |= 0x400000;
   else
      val &= ~0x400000;
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, val);
   
   /* program receive reply timeout duration */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_RXRT, ~0x000FFFFF, hChn->configParam[BDSQ_g1_CONFIG_RRTO_US] & 0x000FFFFF);
   
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &val);
   /* burst enable or disable */
   if (h->settings.bEnableToneburst)
      val |= 0x40;
   else
      val &= ~0x40;
   /* tone A or tone B */
   if (h->settings.bToneburstB)
      val |= 0x80;
   else
      val &= ~0x80;
   /* expect reply enable or disable */
   if ((h->settings.bOverrideFraming) && (h->settings.bExpectReply == false))
      val |= 0x04;
   else
      val &= ~0x04;
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, val);

   /* program pretx delay */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_Q15T, ~0xFFFF0000, (hChn->configParam[BDSQ_g1_CONFIG_PRETX_DELAY_MS] * 1000) << 16);

   return retCode;
}


#ifndef BDSQ_INCLUDE_VSENSE_ONLY
/******************************************************************************
 BDSQ_g1_Txn_P_Transmit_isr() - ISR context
******************************************************************************/
BERR_Code BDSQ_g1_Txn_P_Transmit_isr(BDSQ_ChannelHandle h)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, dsctl;
   uint8_t nBytesToTx, i;

   /* clear diseqc status */
   BDSQ_P_ToggleBit_isr(h, BCHP_SDS_DSEC_DSCTL00, 0x00000001);
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &dsctl);

   if (hChn->txLen)
   {
      if (h->settings.bOverrideFraming)
      {
         if (h->settings.bExpectReply)
         {
            /* always expect reply byte */
            hChn->dsecStatus.bRxExpected = true;
            dsctl |= 0x0000000C;
         }
         else
         {
            /* don't expect reply byte */
            dsctl &= ~0x00000008;
            dsctl |= 0x00000004;
         }
      }
      else
      {
         /* reply expectation depends on framing byte */
         dsctl &= ~0x00000004;
         if (hChn->txBuf[0] & 0x02)
            hChn->dsecStatus.bRxExpected = true;
      }

      /* fifo limit is 16 bytes */
      if (hChn->txLen <= 16)
         nBytesToTx = hChn->txLen;
      else
         nBytesToTx = 16;

      for (i = 0; i < nBytesToTx; i++)
      {
         val = hChn->txBuf[hChn->txCount++];
         BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCMD, val);
      }
   }
   else
   {
      if (h->settings.bExpectReply)
      {
         /* receive only mode */
         hChn->dsecStatus.bRxExpected = true;
         dsctl |= 0x0000000C;
      }
      else
      {
         /* don't expect reply byte */
         dsctl &= ~0x00000008;
         dsctl |= 0x00000004;
      }
   }

   if (hChn->dsecStatus.bRxExpected)
   {
      /* power on diseqc rx if reply expected */
      BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL02, ~0x00800000);
   }

   /* check for extended send */
   if (hChn->txLen > 16)
   {
      /* set tx almost empty trigger to 2 bytes */
      BDSQ_P_ReadModifyWriteRegister_isr(h, BCHP_SDS_DSEC_DSCTL01, ~0x00070000, 0x00020000);
      
      /* set up tx fifo almost empty interrupt */
      BINT_ClearCallback_isr(hChn->hTxAlmostEmptyCb);
      BINT_EnableCallback_isr(hChn->hTxAlmostEmptyCb);
   }

   /* set rx almost full trigger to 2 bytes */
   BDSQ_P_ReadModifyWriteRegister_isr(h, BCHP_SDS_DSEC_DSCTL01, ~0x00380000, 0x00100000);

   /* set up rx fifo almost empty interrupt */
   BINT_ClearCallback_isr(hChn->hRxAlmostFullCb);
   BINT_EnableCallback_isr(hChn->hRxAlmostFullCb);

   /* clear and enable the diseqc transaction done interrupt */
   BINT_ClearCallback_isr(hChn->hDiseqcDoneCb);
   BINT_EnableCallback_isr(hChn->hDiseqcDoneCb);

   /* start the transaction */
   dsctl |= 0x00000002;
   BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, dsctl);
   BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, ~0x00000002);  /* clear start txn */

   return retCode;
}


/******************************************************************************
 BDSQ_g1_Txn_P_TxAlmostEmpty_isr()
******************************************************************************/
void BDSQ_g1_Txn_P_TxAlmostEmpty_isr(void *p, int param)
{
   BDSQ_ChannelHandle h = (BDSQ_ChannelHandle)p;
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   uint8_t bytesAvail, nBytesToTx, i;
   
   BSTD_UNUSED(param);
   
   /* clear and disable tx almost empty interrupt */
   BINT_ClearCallback_isr(hChn->hTxAlmostEmptyCb);
   BINT_DisableCallback_isr(hChn->hTxAlmostEmptyCb);
   
   /* determine number of bytes available in tx fifo */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &val);
   bytesAvail = (val >> 14) & 0x1F;
   
   /* cannot send more bytes than what's available */
   if (hChn->txLen - hChn->txCount <= bytesAvail)
      nBytesToTx = hChn->txLen - hChn->txCount;
   else
      nBytesToTx = bytesAvail;
   
   for (i = 0; i < nBytesToTx; i++)
   {
      val = hChn->txBuf[hChn->txCount++];
      BDSQ_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCMD, val);
   }
   
   if (hChn->txCount < hChn->txLen)
   {
      /* re-arm tx fifo almost empty interrupt if more to transmit */
      BINT_ClearCallback_isr(hChn->hTxAlmostEmptyCb);
      BINT_EnableCallback_isr(hChn->hTxAlmostEmptyCb);
   }
}


/******************************************************************************
 BDSQ_g1_Txn_P_RxAlmostFull_isr()
******************************************************************************/
void BDSQ_g1_Txn_P_RxAlmostFull_isr(void *p, int param)
{
   BDSQ_ChannelHandle h = (BDSQ_ChannelHandle)p;
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t dst1, val;
   uint8_t bytesReceived, i;

   BSTD_UNUSED(param);

   /* clear rx almost full interrupt but keep it enabled */
   BINT_ClearCallback_isr(hChn->hRxAlmostFullCb);

   /* determine number of bytes received in rx fifo */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &dst1);
   bytesReceived = (dst1 >> 19) & 0x1F;

   /* flag parity error but leave parity error positions to diseqc done isr */
   if (dst1 & 0x0400)
   {
      /* flag parity error only if parity errors present in rx fifo */
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_PARITY, &val);
      if (val & 0xFFFF)
         hChn->dsecStatus.status = BDSQ_SendStatus_eRxParityError;
   }

   /* check for rx buffer overflow */
   if (hChn->dsecStatus.nRxBytes + bytesReceived > BDSQ_MAX_RX_LEN)
   {
      hChn->dsecStatus.status = BDSQ_SendStatus_eRxOverflow;
      hChn->dsecStatus.nRxBytes = BDSQ_MAX_RX_LEN;
      bytesReceived = BDSQ_MAX_RX_LEN - hChn->dsecStatus.nRxBytes;  /* limit bytes received if overflow */
   }
   else
   {
      /* accumulate reply length */
      hChn->dsecStatus.nRxBytes += bytesReceived;
   }

   /* BKNI_Printf("!RxAlmostEmpty: n=%d, dst1=0x%08X\n", bytesReceived, dst1); */

   for (i = 0; i < bytesReceived; i++)
   {
      /* unload data from rx fifo */
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSRPLY, &val);
      hChn->rxBuf[hChn->rxCount++] = (uint8_t)(val & 0xFF);
   }

   /* correct data and parity bits for adaptive scheme, if length has not exceeded 16 bytes */
   if ((h->settings.bAdaptiveRxSlice) && (hChn->rxCount <= 16))
   {
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_PARITY, &val);
      val >>= 16; /* retrieve original parity bits before correction */

      /* adaptive mode assumes first two rx bits are 1, for now... */
      hChn->rxBuf[0] |= 0xC0;

      /* compare calculated parity of corrected first byte against original parity*/
      if (BDSQ_g1_Txn_P_GetOddParity_isrsafe(hChn->rxBuf[0]) ^ (val & 1))
      {
         /* parity mismatch */
         hChn->dsecStatus.status = BDSQ_SendStatus_eRxParityError;
         hChn->dsecStatus.posParityErr |= 0x1;
      }
      else
      {
         /* original parity ok after correction */
         hChn->dsecStatus.posParityErr &= ~0x1;
         if ((hChn->dsecStatus.status == BDSQ_SendStatus_eRxParityError) && (hChn->dsecStatus.posParityErr == 0))
         {
            /* clear parity error if corrected */
            hChn->dsecStatus.status = BDSQ_SendStatus_eSuccess;
         }
      }
   }
}


/******************************************************************************
 BDSQ_g1_Txn_P_DiseqcDone_isr() - ISR context
******************************************************************************/
void BDSQ_g1_Txn_P_DiseqcDone_isr(void *p, int param)
{
   BDSQ_ChannelHandle h = (BDSQ_ChannelHandle)p;
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t dst1, dst2, val;
   uint8_t bytesReceived, i;

   BSTD_UNUSED(param);

   /* clear and disable interrupts */
   BINT_ClearCallback_isr(hChn->hDiseqcDoneCb);
   BINT_DisableCallback_isr(hChn->hDiseqcDoneCb);
   BINT_ClearCallback_isr(hChn->hRxAlmostFullCb);
   BINT_DisableCallback_isr(hChn->hRxAlmostFullCb);

   /* clear diseqc busy flag if no previous errors */
   if (hChn->dsecStatus.status == BDSQ_SendStatus_eBusy)
      hChn->dsecStatus.status = BDSQ_SendStatus_eSuccess;

   /* drop incoming packet if rx buffer not read yet */
   if ((hChn->bufStatus & BDSQ_BUF_RX_NOT_EMPTY) == 0)
   {
      /* check diseqc status, get size of reply */
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &dst1);
      bytesReceived = (dst1 >> 19) & 0x1F;

      /* BKNI_Printf("!DiseqcDone: n=%d/%d, dst1=0x%08X\n", bytesReceived, hChn->dsecStatus.nRxBytes, dst1); */

      if (dst1 & 0x01000000)
      {
         /* error detected */
         if (dst1 & 0x1000)
            hChn->dsecStatus.status = BDSQ_SendStatus_eRxOverflow;
         else if (dst1 & 0x0800)
            hChn->dsecStatus.status = BDSQ_SendStatus_eRxReplyTimeout;
         else if (dst1 & 0x0400)
         {
            /* flag parity error only if parity errors present in rx fifo */
            BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_PARITY, &val);
            if (val & 0xFFFF)
               hChn->dsecStatus.status = BDSQ_SendStatus_eRxParityError;

            /* store parity error positions for packets of less than 16 bytes */
            if (hChn->dsecStatus.nRxBytes + bytesReceived <= 16)
               hChn->dsecStatus.posParityErr = val & 0xFFFF;
         }
      }

      /* check additional diseqc status */
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST2, &dst2);
      if (dst2 & 0x8000)
         hChn->dsecStatus.status = BDSQ_SendStatus_eRxIncomplete;

      /* check for rx buffer overflow */
      if (hChn->dsecStatus.nRxBytes + bytesReceived > BDSQ_MAX_RX_LEN)
      {
         hChn->dsecStatus.status = BDSQ_SendStatus_eRxOverflow;
         hChn->dsecStatus.nRxBytes = BDSQ_MAX_RX_LEN;
         bytesReceived = BDSQ_MAX_RX_LEN - hChn->dsecStatus.nRxBytes;  /* limit bytes received if overflow */
      }
      else
      {
         /* accumulate reply length */
         hChn->dsecStatus.nRxBytes += bytesReceived;
      }

      for (i = 0; i < bytesReceived; i++)
      {
         /* unload data from rx fifo */
         BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSRPLY, &val);
         hChn->rxBuf[hChn->rxCount++] = (uint8_t)(val & 0xFF);
      }

      /* correct data and parity bits for adaptive scheme, if length has not exceeded 16 bytes */
      if ((h->settings.bAdaptiveRxSlice) && (hChn->rxCount <= 16))
      {
         BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_PARITY, &val);
         val >>= 16; /* retrieve original parity bits before correction */

         /* adaptive mode assumes 2nd rx bit is always 1 to derive rx bit timing */
         hChn->rxBuf[0] |= 0x40;

         /* deduce 1st rx bit based on 3rd and 4th rx bits */
         switch (hChn->rxBuf[0] & 0x30)
         {
            case 0x00:
            case 0x30:
               /* first nibble is 0x4 or 0x7, clear first bit */
               hChn->rxBuf[0] &= ~0x80;
               break;
            case 0x10:
            case 0x20:
               /* first nibble is 0xD, or 0xE, set first bit */
               hChn->rxBuf[0] |= 0x80;
               break;
         }

         /* compare calculated parity of corrected first byte against original parity*/
         if (BDSQ_g1_Txn_P_GetOddParity_isrsafe(hChn->rxBuf[0]) ^ (val & 1))
         {
            /* parity mismatch */
            hChn->dsecStatus.status = BDSQ_SendStatus_eRxParityError;
            hChn->dsecStatus.posParityErr |= 0x1;
         }
         else
         {
            /* original parity ok after correction */
            hChn->dsecStatus.posParityErr &= ~0x1;
            if ((hChn->dsecStatus.status == BDSQ_SendStatus_eRxParityError) && (hChn->dsecStatus.posParityErr == 0))
            {
               /* clear parity error if corrected */
               hChn->dsecStatus.status = BDSQ_SendStatus_eSuccess;
            }
         }
      }

      /* read max rssi level */
      BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_CICC, &val);
      hChn->dsecStatus.rssiLevel = (uint8_t)(val & 0xFF);

      /* prepare for read, signal rx event if rx bytes received */
      if (hChn->dsecStatus.nRxBytes)
      {
         hChn->rxIdx = 0;  /* start reading message from beginning */
         hChn->bufStatus |= BDSQ_BUF_RX_NOT_EMPTY;
         BKNI_SetEvent(hChn->hRxEvent);
      }

      /* reset rx bit count */
      hChn->rxBitCount = 0;
   }

   /* restore rx bit timing and thresholds if adaptive slicing */
   if (h->settings.bAdaptiveRxSlice)
   {
      BDSQ_P_ReadModifyWriteRegister_isr(h, BCHP_SDS_DSEC_RBDT, ~0x00000FFF, hChn->configParam[BDSQ_g1_CONFIG_RX_BIT_TIMING] & 0xFFF);
      val = (hChn->configParam[BDSQ_g1_CONFIG_CIC_DELTA_THRESH] << 24) | (hChn->configParam[BDSQ_g1_CONFIG_CIC_MIN_THRESH] << 16);
      BDSQ_P_ReadModifyWriteRegister_isr(h, BCHP_SDS_DSEC_FCIC, ~0xFFFF0000, val);
   }

   /* reset the FIFO, memory, noise integrator, etc. */
#ifdef BDSQ_DISABLE_NOISE_ESTIMATION
   BDSQ_P_OrRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, 0x7E002201);
   BKNI_Delay(10);   /* must wait at least 1us for 1MHz clock */
   BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, ~0x7E002201);
#else
   BDSQ_P_OrRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, 0x7F002201);
   BKNI_Delay(10);   /* must wait at least 1us for 1MHz clock */
   BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, ~0x7F002201);
#endif

   /* power off diseqc rx */
   BDSQ_P_OrRegister_isr(h, BCHP_SDS_DSEC_DSCTL02, 0x00800000);

   /* signal tx done event if non-zero bytes transmitted or if no reply expected */
   if ((hChn->txLen) || (!h->settings.bExpectReply))
      BKNI_SetEvent(hChn->hTxEvent);
}


/******************************************************************************
 BDSQ_g1_Txn_P_SendAcw() - Non-ISR context
******************************************************************************/
BERR_Code BDSQ_g1_Txn_P_SendAcw(BDSQ_ChannelHandle h, uint8_t acw)
{
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* clear diseqc status and set send status to busy */
   BDSQ_P_ToggleBit(h, BCHP_SDS_DSEC_DSCTL00, 0x00000001);
   hChn->dsecStatus.status = BDSQ_SendStatus_eBusy;
   hChn->acw = acw;  /* store acw */

   /* set control word */
   BDSQ_P_ReadModifyWriteRegister(h, BCHP_SDS_DSEC_DSCTL01, ~0xFF000000, (acw << 24));

   /* enable auto control word tx */
   BDSQ_P_OrRegister(h, BCHP_SDS_DSEC_DSCTL00, 0x00200000);

   /* clear and enable the acw done interrupt */
   BINT_ClearCallback(hChn->hAcwDoneCb);
   BINT_EnableCallback(hChn->hAcwDoneCb);

   /* start acw transmission */
   BDSQ_P_ToggleBit(h, BCHP_SDS_DSEC_DSCTL00, 0x00008000);

#if 0
   /* 32ms setup + 8ms start bit + 32ms control + 24ms error correction + 8ms polarity ~ 104ms */
   /* protect enable timer from interrupts since SDS_MISC_TMRCTL shared */
   retCode = BDSQ_g1_P_EnableTimer_isr(h, BDSQ_g1_TimerSelect_e1, 150000, BDSQ_g1_Txn_P_AcwDone_isr);
#endif

   return retCode;
}


/******************************************************************************
 BDSQ_g1_Txn_P_AcwDone_isr() - ISR context
******************************************************************************/
void BDSQ_g1_Txn_P_AcwDone_isr(void *p, int param)
{
   BDSQ_ChannelHandle h = (BDSQ_ChannelHandle)p;
   BDSQ_g1_P_ChannelHandle *hChn = (BDSQ_g1_P_ChannelHandle *)h->pImpl;
   uint32_t dst1;

   BSTD_UNUSED(param);

   /* reset status */
   hChn->dsecStatus.status = BDSQ_SendStatus_eSuccess;

   /* check for cw done indicator */
   BDSQ_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &dst1);
   /* BDBG_MSG(("DST1=%08X", dst1)); */
   if ((dst1 & 0x10000000) == 0)
      hChn->dsecStatus.status = BDSQ_SendStatus_eAcwTimeout;

   /* check polarity bit of acw */
   if (hChn->acw & 0x80)
      BDSQ_P_OrRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, 0x00100000);    /* finish with vtop */
   else
      BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, ~0x00100000);  /* finish with vbot */

   /* revert to non-ACW tx */
   BDSQ_P_AndRegister_isr(h, BCHP_SDS_DSEC_DSCTL00, ~0x00200000);

   /* reset control word and state machine */
   BDSQ_P_ToggleBit_isr(h, BCHP_SDS_DSEC_DSCTL00, 0x00006000);

   /* set tx done event */
   BKNI_SetEvent(hChn->hTxEvent);
}
#endif
