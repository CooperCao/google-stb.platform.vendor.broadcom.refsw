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
#include "bstd.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"


#ifndef BAST_EXCLUDE_MI2C

BDBG_MODULE(bast_g3_priv_mi2c);

/* local functions */
BERR_Code BAST_g3_P_InitiateWriteMi2c(BAST_ChannelHandle h, uint8_t slave_addr, uint8_t *i2c_buf, uint8_t n);
BERR_Code BAST_g3_P_InitiateReadMi2c(BAST_ChannelHandle h, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t in_n);


/******************************************************************************
 BAST_g3_P_Mi2cDisableInterrupts_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_Mi2cDisableInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hMi2cCb);
   BINT_ClearCallback_isr(hChn->hMi2cCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_Mi2cEnableInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_Mi2cEnableInterrupts(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback(hChn->hMi2cCb);
   BINT_EnableCallback(hChn->hMi2cCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_Mi2cReset() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_Mi2cReset(BAST_ChannelHandle h)
{
   /* hold reset pulse due to slower i2c clock */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_MISC_IICTL2, 0x80);
   BKNI_Delay(200);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_MISC_IICTL2, ~0x81);
   BKNI_Delay(200);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_Mi2cInit() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_Mi2cInit(BAST_ChannelHandle h, uint8_t chip_address)
{
   uint32_t val;
   BERR_Code retCode;

   /* reset the mi2c */
   BAST_CHK_RETCODE(BAST_g3_P_Mi2cReset(h));

   /* program chip address */
   val = chip_address;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICHPA, &val);

#ifndef BAST_DONT_USE_MI2C_IRQ
   retCode = BAST_g3_P_Mi2cEnableInterrupts(h);
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_WriteMi2c() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_WriteMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, timeout = 0;
   bool bBusy;

   if ((n > 8) || (i2c_buf == NULL))
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BKNI_EnterCriticalSection();
   bBusy = hChn->bMi2cInProgress;
   if (!bBusy)
      hChn->bMi2cInProgress = true;
   BKNI_LeaveCriticalSection();

   if (bBusy)
   {
      BERR_TRACE(retCode = BAST_ERR_MI2C_BUSY);
      return retCode;
   }

   BKNI_WaitForEvent(hChn->hMi2cEvent, 0);
   retCode = BAST_g3_P_InitiateWriteMi2c(h, slave_addr, i2c_buf, n);
   if (retCode != BERR_SUCCESS)
      goto done;

#ifdef BAST_DONT_USE_MI2C_IRQ
   for (timeout = 0; timeout < 10000; timeout++)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x01000000)
         break;
   }
#else
   retCode = BKNI_WaitForEvent(hChn->hMi2cEvent, 2);
   if (retCode == BERR_TIMEOUT)
   {
      /* workaround for the missing irq on the first i2c transaction */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x01000000)
      {
         BDBG_MSG(("detected missing mi2c write irq"));
         retCode = BERR_SUCCESS;
      }
   }

   /* workaround for write transaction not finished after interrupt */
   if (retCode == BERR_SUCCESS)
   {
      for (timeout = 0; timeout < 2000; timeout++)
      {
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
         if (val & 0x01000000)
            break;
      }
   }
#endif

   if (retCode == BERR_SUCCESS)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x02000000)
      {
         BERR_TRACE(retCode = BAST_ERR_MI2C_NO_ACK);
         BDBG_MSG(("i2c no ack"));
      }
   }

   done:
   BKNI_EnterCriticalSection();
   hChn->bMi2cInProgress = false;
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BAST_g3_P_InitiateWriteMi2c() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_InitiateWriteMi2c(BAST_ChannelHandle h, uint8_t slave_addr, uint8_t *i2c_buf, uint8_t n)
{
   BERR_Code retCode;
   uint32_t val, val1, val2, cnt, i;

   BAST_CHK_RETCODE(BAST_g3_P_Mi2cInit(h, slave_addr));

   /* program number of bytes to transfer */
   cnt = n & 0x0F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICCNT, &cnt);

   /* set to normal mode */
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICTL1, &val);

   /* load the transmit buffer */
   val1 = val2 = 0;
   for (i = 0; i < 8; i++)
   {
      if (i < 4)
      {
         if (i < cnt)
            val1 |= ((i2c_buf[i] & 0xFF) << ((3-i)*8));
      }
      else
      {
         if (i < cnt)
            val2 |= ((i2c_buf[i] & 0xFF) << ((7-i)*8));
      }
   }

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_MIICTX1, &val1);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_MIICTX2, &val2);

   /* write-only mode, start transfer */
   val = 0x01;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICTL2, &val);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_ReadMi2c() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_ReadMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, timeout = 0;
   bool bBusy;

   if ((in_n == 0) || (in_n > 8) || (out_n > 8))
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BKNI_EnterCriticalSection();
   bBusy = hChn->bMi2cInProgress;
   if (!bBusy)
      hChn->bMi2cInProgress = true;
   BKNI_LeaveCriticalSection();

   if (bBusy)
   {
      BERR_TRACE(retCode = BAST_ERR_MI2C_BUSY);
      return retCode;
   }

   BKNI_WaitForEvent(hChn->hMi2cEvent, 0);
   retCode = BAST_g3_P_InitiateReadMi2c(h, slave_addr, out_buf, out_n, in_n);
   if (retCode != BERR_SUCCESS)
      goto done;

#ifdef BAST_DONT_USE_MI2C_IRQ
   for (timeout = 0; timeout < 10000; timeout++)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x01000000)
         break;
   }
#else
   retCode = BKNI_WaitForEvent(hChn->hMi2cEvent, 2);
   if (retCode == BERR_TIMEOUT)
   {
      /* workaround for the missing irq on the first i2c transaction */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x01000000)
      {
         BDBG_MSG(("detected missing mi2c read irq"));
         retCode = BERR_SUCCESS;
      }
   }

   /* workaround for read data not immediately available after interrupt */
   if (retCode == BERR_SUCCESS)
   {
      for (timeout = 0; timeout < 2000; timeout++)
      {
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
         if (val & 0x01000000)
            break;
      }
   }
#endif

   if (retCode == BERR_SUCCESS)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MI2CSA, &val);
      if (val & 0x02000000)
      {
         BERR_TRACE(retCode = BAST_ERR_MI2C_NO_ACK);
         BDBG_MSG(("i2c no ack"));
      }
      else
      {
         /* read read_n bytes into mi2c_buffer */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MIICRX1, &val);
         in_buf[0] = (uint8_t)((val >> 24) & 0xFF);
         if (in_n > 1)
            in_buf[1] = (uint8_t)((val >> 16) & 0xFF);
         if (in_n > 2)
            in_buf[2] = (uint8_t)((val >> 8) & 0xFF);
         if (in_n > 3)
            in_buf[3] = (uint8_t)(val & 0xFF);
         if (in_n > 4)
         {
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_MIICRX2, &val);
            in_buf[4] = (uint8_t)((val >> 24) & 0xFF);
         }
         if (in_n > 5)
            in_buf[5] = (uint8_t)((val >> 16) & 0xFF);
         if (in_n > 2)
            in_buf[6] = (uint8_t)((val >> 8) & 0xFF);
         if (in_n > 3)
            in_buf[7] = (uint8_t)(val & 0xFF);
      }
   }

   done:
   BKNI_EnterCriticalSection();
   hChn->bMi2cInProgress = false;
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BAST_g3_P_InitiateReadMi2c() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_InitiateReadMi2c(BAST_ChannelHandle h, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t in_n)
{
   uint32_t val, tx1, tx2, i;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_Mi2cInit(h, slave_addr));

   /* program number of bytes to transfer */
   if (out_n == 0)
      val = in_n;  /* read only mode */
   else
      val = (out_n | (in_n << 4));
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICCNT, &val);

   /* set to normal mode */
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICTL1, &val);

   /* load the transmit buffer */
   if (out_n)
   {
      /* load the transmit buffer */
      tx1 = tx2 = 0;
      for (i = 0; i < 8; i++)
      {
         if (i < 4)
         {
            if (i < out_n)
               tx1 |= ((out_buf[i] & 0xFF) << ((3-i)*8));
         }
         else
         {
            if (i < out_n)
               tx2 |= ((out_buf[i] & 0xFF) << ((7-i)*8));
         }
      }
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_MIICTX1, &tx1);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_MIICTX2, &tx2);
   }

   /* write-only mode, start transfer */
   if (out_n)
      val = 0x07; /* write-then-read mode */
   else
      val = 0x03; /* read only mode */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_IICTL2, &val);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_Mi2c_isr() - ISR context
******************************************************************************/
void BAST_g3_P_Mi2c_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BSTD_UNUSED(param);

   BAST_g3_P_IncrementInterruptCounter_isr(h, BAST_g3_IntID_eMi2c);
   BAST_g3_P_Mi2cDisableInterrupts_isr(h);
   BKNI_SetEvent(hChn->hMi2cEvent);
}

#endif
