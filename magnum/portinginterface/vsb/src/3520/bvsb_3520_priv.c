/***************************************************************************
 *     Copyright (c) 2004-2014, Broadcom Corporation
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
#include "bchp_3520.h"
#include "bi2c.h"
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3520_priv.h"


BDBG_MODULE(bvsb_3520_priv);


/******************************************************************************
 BVSB_3520_P_Open()
******************************************************************************/
BERR_Code BVSB_3520_P_Open(
   BVSB_Handle *h,       /* [out] BVSB handle */
   BCHP_Handle hChip,    /* [in] chip handle */
   void        *pReg,    /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt,    /* [in] Interrupt handle */   
   const BVSB_Settings *pDefSettings /* [in] default settings */
)
{
   BERR_Code retCode;
   BVSB_Handle hDev;
   BVSB_3520_P_Handle *h3520Dev;
   
   BSTD_UNUSED(hInterrupt);
   BSTD_UNUSED(hChip);

   BDBG_ASSERT(pDefSettings->i2c.interruptEnableFunc);

   /* allocate memory for the handle */
   hDev = (BVSB_Handle)BKNI_Malloc(sizeof(BVSB_P_Handle));
   BDBG_ASSERT(hDev);
   *h = hDev;

   h3520Dev = (BVSB_3520_P_Handle *)BKNI_Malloc(sizeof(BVSB_3520_P_Handle));
   BDBG_ASSERT(h3520Dev);
   hDev->pImpl = (void*)h3520Dev;

   /* initialize our handle */
   h3520Dev->hRegister = (BREG_I2C_Handle)pReg;
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BVSB_Settings));

   /* create events */
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hInterruptEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hApiEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hLockChangeEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hOobLockChangeEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hInitDoneEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hHabDoneEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hAntennaEvent)));
   BVSB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hBscEvent)));

   h3520Dev->last_page_16_15 = 0xFF;
   h3520Dev->last_page_14_7 = 0xFF;
   h3520Dev->last_mbox_15_8 = 0xFF;
   h3520Dev->last_bsc_slave_addr = 0xFF;

   retCode = BVSB_3520_P_DisableInterrupts(hDev);

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_Close()
******************************************************************************/
BERR_Code BVSB_3520_P_Close(BVSB_Handle h)
{
   BERR_Code retCode;

   BDBG_ASSERT(h);

   retCode = BVSB_3520_P_DisableInterrupts(h);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hInterruptEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hApiEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hLockChangeEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hOobLockChangeEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hInitDoneEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hHabDoneEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hAntennaEvent);
   BKNI_DestroyEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hBscEvent);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);

   return retCode;
}


/******************************************************************************
 BVSB_3520_P_InitAp()
******************************************************************************/
BERR_Code BVSB_3520_P_InitAp(
   BVSB_Handle h,            /* [in] BVSB handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM3520 microcode image */
)
{
   BERR_Code retCode;
   uint16_t n, addr;
   uint8_t *pImage;
   uint8_t sb, retries;

   /* disable interrupts */
   BVSB_CHK_RETCODE(BVSB_3520_P_DisableInterrupts(h));
   BVSB_CHK_RETCODE(BVSB_3520_P_EnableHostInterrupt(h, false));

   /* reset the AP before downloading the microcode */
   BVSB_CHK_RETCODE(BVSB_3520_P_ResetAp(h));

   /* download to RAM */
   pImage = (uint8_t*)pHexImage;
   while (1)
   {
      n = (*pImage++ << 8);
      n |= *pImage++;

      if (n == 0)
         break;

      addr = (*pImage++ << 8);
      addr |= *pImage++;

      for (retries = 0; retries < 3; retries++)
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_WriteMemory(h, addr, (uint8_t *)pImage, n));
         pImage += n; 

         /* check for host transfer error */
         BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_STAT1, &sb));
         if ((sb & BCM3520_STAT1_H_ER) == 0)
            break;
            
         BDBG_WRN(("host transfer error\n"));
         BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT1, &sb));
      }
   }
   
   /* enable init done interrupt */
   BVSB_CHK_RETCODE(BVSB_3520_P_EnableInitDoneInterrupt(h));

   /* clear H_STAT1 */
   sb = 0xFF;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT1, &sb));

   /* start running the AP */
   if ((retCode = BVSB_3520_P_RunAp(h)) != BERR_SUCCESS)
      goto done;

   /* wait for init done interrupt */
   if (BVSB_3520_P_WaitForEvent(h, ((BVSB_3520_P_Handle *)(h->pImpl))->hInitDoneEvent, 10) != BERR_SUCCESS)
   {
      BDBG_ERR(("AP initialization timeout\n")); 
      BERR_TRACE(retCode = BVSB_ERR_AP_NOT_INIT);            
   }

#if 0
   /* clear lock interrupts */
   sb = BCM3520_STAT2_LOCK_MASK;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT2, &sb));
#endif

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetApStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetApStatus(
   BVSB_Handle h,           /* [in] VSB device handle */
   BVSB_ApStatus *pStatus   /* [out] AP status */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   *pStatus = 0;
   BVSB_CHK_RETCODE(BREG_I2C_Read(((BVSB_3520_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_CTL1, buf, 3));
   *pStatus = buf[0] | (buf[1] << 8) | (buf[2] << 16);
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetApVersion()
******************************************************************************/
BERR_Code BVSB_3520_P_GetApVersion(
   BVSB_Handle h,          /* [in] BVSB handle */
   uint16_t    *pChipId,   /* [out] BVSB chip ID */
   uint16_t    *pChipVer,  /* [out] chip revision number */
   uint8_t     *pApVer,    /* [out] AP microcode version */
   uint8_t     *pScrVer,   /* [out] acquisition script version */
   uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
   BERR_Code retCode;
   uint8_t buf[16];

   buf[0] = 0x09;
   buf[14] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 15, buf, 14, true));

   *pChipId = ((buf[1] << 8) | buf[2]);
   *pChipVer = ((buf[3] << 8) | buf[4]);
   *pApVer = buf[5];
   *pScrVer = buf[6];
   *pCfgVer = buf[7];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ReadRegister()
******************************************************************************/
BERR_Code BVSB_3520_P_ReadRegister(
   BVSB_Handle h,     /* [in] BVSB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;
   uint8_t sb;

   if ((reg >= 0x80) && (reg <= 0xFF))
   {
      /* accessing host register space */
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, reg, &sb));
      *val = (uint32_t)sb;
   }
   else
   {
      /* accessing internal registers through io_mbox */
      BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, (uint16_t)reg, val));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_WriteRegister()
******************************************************************************/
BERR_Code BVSB_3520_P_WriteRegister(
   BVSB_Handle h,     /* [in] BVSB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
)
{
   BERR_Code retCode;
   uint8_t sb;

   if ((reg >= 0x80) && (reg <= 0xFF))
   {
      /* accessing host register space */
      sb = (uint8_t)(*val);
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, reg, &sb));
   }
   else
   {
      /* accessing internal registers through io_mbox */
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, (uint16_t)reg, val));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_Mi2cWrite()
******************************************************************************/ 
BERR_Code BVSB_3520_P_Mi2cWrite(
   BVSB_Handle h,      /* [in] BVSB handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *buf,       /* [in] specifies the data to transmit */
   uint8_t n           /* [in] number of bytes to transmit after the i2c slave address */
)
{
   BERR_Code retCode;
   int idx;
   uint32_t mb, in0_3, in4_7, cnt, iic_enable;
   bool bFirstTime, bWrite4_7;

   mb = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &mb)); 

   if (((BVSB_3520_P_Handle *)(h->pImpl))->last_bsc_slave_addr != slave_addr)
   {
      mb = (uint32_t)slave_addr;
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CHIP_ADDRESS, &mb)); 
      ((BVSB_3520_P_Handle *)(h->pImpl))->last_bsc_slave_addr = slave_addr;
   }

   /* 400 KHz, write only */
   mb = 0x10;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CTL_REG, &mb));

   idx = 0;
   in0_3 = 0;
   in4_7 = 0;
   bFirstTime = true;
   while (n > 0)
   {
      bWrite4_7 = false;
      in0_3 = buf[idx] << 24;
      if (n > 1)
         in0_3 |= (buf[idx+1] << 16);
      if (n > 2)
         in0_3 |= (buf[idx+2] << 8);
      if (n > 3)
         in0_3 |= buf[idx+3];
      if (n > 4)
      {
         bWrite4_7 = true;
         in4_7 = (buf[idx+4] << 24);
      }
      if (n > 5)
         in4_7 |= (buf[idx+5] << 16);
      if (n > 6)
         in4_7 |= (buf[idx+6] << 8);
      if (n > 7)
         in4_7 |= buf[idx+7];

      if (n >= 8) 
         cnt = 8;
      else
         cnt = n;

      n -= cnt;
      idx += cnt;
 
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_DATA_IN0_3, &in0_3));
      if (bWrite4_7)
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_DATA_IN4_7, &in4_7));
      }
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CNT_REG, &cnt));

      iic_enable = 1;
      if (bFirstTime == false)
         iic_enable |= 0x20;  /* no start condition */
      else
         bFirstTime = false;
      if (n > 0)
         iic_enable  |= 0x10;   /* no stop condition */
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
   
      do
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
      } while ((iic_enable & 0x06) == 0);
      if (iic_enable & 0x04)
      {
         BDBG_ERR(("BVSB: no ack from tuner\n"));
         return (BERR_TRACE(BVSB_ERR_MI2C_NO_ACK));
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_Mi2cRead()
******************************************************************************/
BERR_Code BVSB_3520_P_Mi2cRead(
   BVSB_Handle h,      /* [in] BVSB handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf,   /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,      /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,    /* [out] holds the data read */
   uint8_t in_n        /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   int idx;
   uint32_t mb, in0_3, in4_7, out0_3, out4_7, iic_enable;
   bool bWrite4_7, bRead4_7;
   uint8_t n;

   if ((out_n > 8) || (in_n < 1))
      return BERR_INVALID_PARAMETER; 
      
   mb = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &mb)); 

   if (((BVSB_3520_P_Handle *)(h->pImpl))->last_bsc_slave_addr != slave_addr)
   {
      mb = (uint32_t)slave_addr;
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CHIP_ADDRESS, &mb)); 
      ((BVSB_3520_P_Handle *)(h->pImpl))->last_bsc_slave_addr = slave_addr;
   }

   /* transmit data*/
   if ( out_n )
   {
      mb = 0x10; /* Issue write only, 400 kHz */
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CTL_REG, &mb));   

      idx = 0;
      in0_3 = 0;
      in4_7 = 0;
      bWrite4_7 = false;
      in0_3 = out_buf[idx] << 24;

      if ( out_n > 1 )
         in0_3 |= (out_buf[idx+1] << 16);
      if (out_n > 2)
         in0_3 |= (out_buf[idx+2] << 8);
      if (out_n > 3)
         in0_3 |= out_buf[idx+3];
      if (out_n > 4)
      {
         bWrite4_7 = true;
         in4_7 = out_buf[idx+4] << 24;
      }
      if (out_n > 5)
         in4_7 |= (out_buf[idx+5] << 16);
      if (out_n > 6)
         in4_7 |= (out_buf[idx+6] << 8);
      if (out_n > 7)
         in4_7 |= out_buf[idx+7];

      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_DATA_IN0_3, &in0_3));
      if (bWrite4_7)
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_DATA_IN4_7, &in4_7));
      }

      /* Write cnt register */
      mb = out_n;
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CNT_REG, &mb));

      /* Issue send command */
      iic_enable = 1;
      iic_enable |= 0x10;   /* no stop condition */
      iic_enable |= 0x40;   /* restart condition */
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));

      do
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
      } while ((iic_enable & 0x06) == 0);

      if (iic_enable & 0x04)
      {
         BDBG_ERR(("BVSB: no ack from tuner\n"));
         return (BERR_TRACE(BVSB_ERR_MI2C_NO_ACK));
      }
   }

   /* issue fake read to work around hardware bug*/
   iic_enable = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));

   mb = 0x11; /* Issue read-only */
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CTL_REG, &mb));   

   mb = 0x0; /* Not really reading anything */
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CNT_REG, &mb));   

   iic_enable = 1;
   iic_enable |= 0x10;   /* no stop condition */
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
   
   do
   {
      BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
   } while ((iic_enable & 0x06) == 0);
   if (iic_enable & 0x04)
   {
      BDBG_ERR(("BVSB: no ack from tuner\n"));
      return (BERR_TRACE(BVSB_ERR_MI2C_NO_ACK));
   }
   /* end of fake read section */    
   
   /* Finally read the data */
   idx = 0;

   iic_enable = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));

   mb = 0x11; /* Issue read-only, 400 kHz */
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CTL_REG, &mb));   

   while (in_n > 0)
   {
      if (in_n > 8)
         n = 8;
      else
         n = in_n;

      if (n > 4)
         bRead4_7 = true;
      else
         bRead4_7 = false;

      in_n -= n;

      /* Set number of bytes to read */
      mb = n;
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_CNT_REG, &mb));   

      iic_enable = 1;
      iic_enable |= 0x20;   /* no start condition because we issued a fake read */

      if ( in_n > 0 )
         iic_enable |= 0x10; /* no stop condition if we want to continue */

      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));

      do
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
      } while ((iic_enable & 0x06) == 0);
      if (iic_enable & 0x04)
      {
         BDBG_ERR(("BVSB: no ack from tuner\n"));
         return (BERR_TRACE(BVSB_ERR_MI2C_NO_ACK));
      }

      BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_DATA_OUT0_3, &out0_3));
      in_buf[idx++] = (out0_3 >> 24) & 0xFF;
      if (n > 1)
         in_buf[idx++] = (out0_3 >> 16) & 0xFF;
      if (n > 2)
         in_buf[idx++] = (out0_3 >> 8) & 0xFF;
      if (n > 3)
         in_buf[idx++] = out0_3 & 0xFF;

      if (bRead4_7)
      {
         BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox(h, BCM3520_BSC_DATA_OUT4_7, &out4_7));
         in_buf[idx++] = (out4_7 >> 24) & 0xFF;
         if (n > 5)
            in_buf[idx++] = (out4_7 >> 16) & 0xFF;
         if (n > 6)
            in_buf[idx++] = (out4_7 >> 8) & 0xFF;
         if (n > 7)
            in_buf[idx++] = out4_7 & 0xFF;
      }

      iic_enable = 0;
      BVSB_CHK_RETCODE(BVSB_3520_P_WriteMbox(h, BCM3520_BSC_IIC_ENABLE, &iic_enable));
   }
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_AcquireInband()
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireInband(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{   
   BERR_Code retCode;

   BDBG_ASSERT(h);
   BDBG_ASSERT(pParams);

   BVSB_CHK_RETCODE(BVSB_3520_P_EnableLockInterrupt(h, false));
   
   switch (pParams->mode)
   {
      case BVSB_InbandMode_e8VSB:      /* 8-VSB */
      case BVSB_InbandMode_e16VSB:     /* 16-VSB */
         BVSB_CHK_RETCODE(BVSB_3520_P_AcquireVsb(h, pParams));
         break;

      case BVSB_InbandMode_e1024QAM_B: /* 1024-QAM Annex B */
      case BVSB_InbandMode_e512QAM_B: /* 512-QAM Annex B */
      case BVSB_InbandMode_e256QAM_B:  /* 256-QAM Annex B */
      case BVSB_InbandMode_e128QAM_B: /* 128-QAM Annex B */
      case BVSB_InbandMode_e64QAM_B:   /* 64-QAM Annex B  */
      case BVSB_InbandMode_e256QAM_A:  /* 256-QAM Annex A */
      case BVSB_InbandMode_e128QAM_A:  /* 128-QAM Annex A */
      case BVSB_InbandMode_e64QAM_A:   /* 64-QAM Annex A  */
      case BVSB_InbandMode_e32QAM_A:   /* 32-QAM Annex A  */
      case BVSB_InbandMode_e16QAM_A:   /* 16-QAM Annex A  */
         BVSB_CHK_RETCODE(BVSB_3520_P_AcquireQam(h, pParams));
         break;

      case BVSB_InbandMode_eNTSC:      /* NTSC */
         BVSB_CHK_RETCODE(BVSB_3520_P_AcquireNtsc(h, pParams));
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER)); 
   }

   BVSB_CHK_RETCODE(BVSB_3520_P_EnableLockInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetVsbStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetVsbStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_VsbStatus *pStatus  /* [out] VSB status */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   buf[0] = 0x1E;
   buf[0x3D] = 0x00;
   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x3D, &buf[0x3D], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, buf, 0x3D, true, 0x9E));

   switch ((buf[2] >> 4) & 0x07)
   {
      case 0:
         pStatus->acqParams.mode = BVSB_InbandMode_e8VSB;
         break;
         
      case 4:
         pStatus->acqParams.mode = BVSB_InbandMode_e16VSB;
         break;
         
      default:
         pStatus->acqParams.mode = BVSB_InbandMode_eUnknown;
         break;
   }
   
   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bNtscSweep = buf[0x01] & 0x08 ? true : false;
   pStatus->acqSettings.bRfiSweep = buf[0x01] & 0x04 ? true : false;

   pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
   pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
   pStatus->bNtscEngaged = buf[0x12] & 0x20 ? true : false;
   pStatus->bRfiEngaged = buf[0x12] & 0x10 ? true : false;
   pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
   pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
   pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
   pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
   pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
   pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
   pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
   pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
   pStatus->snr = (buf[0x13] << 8) + buf[0x14];
   pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
   pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
   pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetQamStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetQamStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_QamStatus *pStatus  /* [out] QAM status   */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   buf[0] = 0x20;
   buf[0x3D] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x3D, &buf[0x3D], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, buf, 0x3D, true, 0xA0));

   if (buf[2] & 0x08)
   {
      /* annex B */
      switch ((buf[2] >> 4) & 0x07)
      {
         case 3:
            pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_B;
            break;
         case 4:
            pStatus->acqParams.mode = BVSB_InbandMode_e128QAM_B;
			break;
         case 5:
            pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_B;
            break;
         case 6:
            pStatus->acqParams.mode = BVSB_InbandMode_e512QAM_B;
            break;
         default:
            pStatus->acqParams.mode = BVSB_InbandMode_e1024QAM_B;
            break;
      }      
   }
   else
   {
      /* annex A */
      switch ((buf[2] >> 4) & 0x07)
      {
         case 1:
            pStatus->acqParams.mode = BVSB_InbandMode_e16QAM_A;
            break;
         
         case 2:
            pStatus->acqParams.mode = BVSB_InbandMode_e32QAM_A;
            break;
         
         case 3:
            pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_A;
            break;
         
         case 4:
            pStatus->acqParams.mode = BVSB_InbandMode_e128QAM_A;
            break;
         
         default:
            pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_A;
            break;
      }            
   }
   
   switch ((buf[1] >> 6) & 0x03)
   {
      case 0:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e12;
         break;

      case 1:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e15;
         break;

      default:
         pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e18;
         break;
   }

   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   switch ((buf[3] >> 4) & 0x0F)
   {
      case 0:
         pStatus->acqSettings.idepth = BVSB_IDepth_e204_1;
         break;
         
      case 1:
         pStatus->acqSettings.idepth = BVSB_IDepth_e102_2;
         break;
         
      case 2:
         pStatus->acqSettings.idepth = BVSB_IDepth_e68_3;
         break;
         
      case 3:
         pStatus->acqSettings.idepth = BVSB_IDepth_e51_4;
         break;
         
      case 4:
         pStatus->acqSettings.idepth = BVSB_IDepth_e34_6;
         break;
         
      case 5:
         pStatus->acqSettings.idepth = BVSB_IDepth_e17_12;
         break;
         
      case 6:
         pStatus->acqSettings.idepth = BVSB_IDepth_e12_17;
         break;
         
      case 7:
         pStatus->acqSettings.idepth = BVSB_IDepth_e6_34;
         break;
         
      case 8:
         pStatus->acqSettings.idepth = BVSB_IDepth_e4_51;
         break;
         
      case 9:
         pStatus->acqSettings.idepth = BVSB_IDepth_e3_68;
         break;
         
      case 10:
         pStatus->acqSettings.idepth = BVSB_IDepth_e2_102;
         break;
         
      default:
         pStatus->acqSettings.idepth = BVSB_IDepth_e1_204;
         break;
   }
   
   pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bSpinv = buf[0x01] & 0x10 ? true : false;
   pStatus->acqSettings.bEq = buf[0x01] & 0x04 ? true : false;
   pStatus->acqSettings.bCh = buf[0x01] & 0x02 ? true : false;
   pStatus->acqSettings.bDavic = buf[0x03] & 0x08 ? true : false;
   pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
   pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
   pStatus->bSpinv = (buf[0x12] & 0x01 ? true : false);
   pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
   pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
   pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
   pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
   pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
   pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
   pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
   pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
   pStatus->snr = (buf[0x13] << 8) + buf[0x14];
   pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
   pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
   pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetNtscStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetNtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_NtscStatus *pStatus /* [out] NTSC status */
)
{
   BERR_Code retCode;
   uint8_t buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }
   
   buf[0] = 0x1C;
   buf[0x19] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x19, &buf[0x19], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, buf, 0x19, true, 0x9C));

   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
   pStatus->acqParams.symbolRateOffset = 0;
   pStatus->acqParams.mode = BVSB_InbandMode_eNTSC;
   pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
   pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->pixCarrFreq = (int32_t)((buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0A] << 8) + buf[0x0B]);
   pStatus->pixCarrFreqError = (int32_t)((buf[0x0D] << 24) + (buf[0x0E] << 16) + (buf[0x0F] << 8) + buf[0x10]);
   pStatus->rf_agc = (((buf[0x11] << 8) + buf[0x12]) * 1000) / 65535;
   pStatus->if_agc = (((buf[0x13] << 8) + buf[0x14]) * 1000) / 65535;
   pStatus->bHsyncLocked = buf[0x0C] & 0x08 ? true : false;

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetBtscStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetBtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_BtscStatus *pStatus /* [out] BTSC status */
)
{
   BERR_Code retCode;
   uint8_t buf[128];

   if (pStatus == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   buf[0] = 0x1C;
   buf[0x19] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 0x1A, buf, 0x19, true));

   switch (buf[0x16] & BVSB_AUDIO_CONFIG_DECODE_MODE_MASK)
   {
      case BVSB_AUDIO_CONFIG_DECODE_MODE_MONO:
         pStatus->decodeMode = BVSB_BtscDecodeMode_eMono;
         break;
         
      case BVSB_AUDIO_CONFIG_DECODE_MODE_STEREO:
         pStatus->decodeMode = BVSB_BtscDecodeMode_eStereo;
         break;
         
      case BVSB_AUDIO_CONFIG_DECODE_MODE_SAP:
         pStatus->decodeMode = BVSB_BtscDecodeMode_eSAP;
         break;
         
      default: /* BVSB_AUDIO_CONFIG_DECODE_MODE_SAP_MONO: */
         pStatus->decodeMode = BVSB_BtscDecodeMode_eSAP_Mono;
         break;
   }
   
   switch (buf[0x16] & BVSB_AUDIO_CONFIG_SR_MASK)
   {
      case BVSB_AUDIO_CONFIG_SR_32KHZ:
         pStatus->sampleRate = BVSB_BtscSampleRate_e32KHz;
         break;
      
      case BVSB_AUDIO_CONFIG_SR_44_1KHZ:
         pStatus->sampleRate = BVSB_BtscSampleRate_e44_1KHz;
         break;
      
      default: /* BVSB_AUDIO_CONFIG_SR_48KHZ: */
         pStatus->sampleRate = BVSB_BtscSampleRate_e48KHz;
         break;
   }
 
   pStatus->bI2sOut = buf[0x16] & BVSB_AUDIO_CONFIG_SEL_MASK ? true : false;
   pStatus->bBtscMute = (buf[0x17] & BVSB_AUDIO_CONFIG_MUTE) ? true : false;
   pStatus->bSapDetected = (buf[0x15] & BVSB_AUDIO_SAP_DETECTED) ? true : false;
   pStatus->bStereoDetected = (buf[0x15] & BVSB_AUDIO_STEREO_DETECTED) ? true : false;
   pStatus->btscLeftVolume = buf[0x17] & 0x7F;
   pStatus->btscRightVolume = buf[0x18] & 0x7F;
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ResetInbandStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_ResetInbandStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x1F;
   buf[1] = 0x04;
   buf[2] = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 3, buf, 1, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_AcquireOob()
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireOob(
   BVSB_Handle h,           /* [in] BVSB handle */
   const BVSB_OobParams *pParams  /* [in] acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[5];

   BVSB_CHK_RETCODE(BVSB_3520_P_EnableOobLockInterrupt(h, false));

   hab[0] = 0x0D;
   hab[1] = hab[2] = hab[3] = hab[4] = 0x00;
   
   hab[1] |= ((h->settings.oob.bSpinv) ? 0x10 : 0x00);   
   hab[1] |= ((h->settings.oob.bAutoAcq) ? 0x20 : 0x00);
   hab[2] |= ((h->settings.oob.bBypassFEC) ? 0x80 : 0x00);
   switch (pParams->mode)
   {
      case BVSB_OobMode_eDs178:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_OobMode_eDs167_A:
         hab[2] |= 0x18;
         break;
         
      case BVSB_OobMode_eDs167_B:
         hab[2] |= 0x28;  
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   switch (h->settings.oob.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 5, hab, 1, true));
   BVSB_CHK_RETCODE(BVSB_3520_P_EnableOobLockInterrupt(h, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetOobStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_GetOobStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobStatus *pStatus  /* [out] OOB status   */
)
{
   BERR_Code retCode;
   uint8_t   buf[128];

   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   buf[0] = 0x19;
   buf[0x37] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, buf, 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x37, &buf[0x37], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, buf, 0x37, true, 0x99));

   switch ((buf[2] >> 4) & 0x07)
   {
      case 0:
         pStatus->acqParams.mode = BVSB_OobMode_eDs178;
         break;
         
      case 1:
         pStatus->acqParams.mode = BVSB_OobMode_eDs167_A;
         break;
         
      case 2:
         pStatus->acqParams.mode = BVSB_OobMode_eDs167_B;
         break;
         
      default:
         pStatus->acqParams.mode = BVSB_OobMode_eUnknown;
         break;
   }
   
   switch ((buf[2] >> 1) & 0x03)
   {
      case 0:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
         break;
      case 1:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
         break;
      default:
         pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
         break;
   }
   
   pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
   pStatus->acqSettings.bSpinv = buf[0x01] & 0x10 ? true : false;
   pStatus->acqSettings.bBypassFEC = buf[0x02] & 0x80 ? true : false;
   pStatus->bRcvrLocked = buf[0x10] & 0x08 ? true : false;
   pStatus->bFecLocked = buf[0x10] & 0x04 ? true : false;
   pStatus->bSpinv = buf[0x10] & 0x01 ? true : false;
   pStatus->loFreq = (buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07];
   pStatus->ifFreq = (buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0A] << 8) + buf[0x0B];
   pStatus->ifFreqError = (int32_t)((buf[0x15] << 24) + (buf[0x16] << 16) + (buf[0x17] << 8) + buf[0x18]);
   pStatus->symbolRate = (int32_t)((buf[0x0C] << 24) + (buf[0x0D] << 16) + (buf[0x0E] << 8) + buf[0x0F]);
   pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x19] << 8) + buf[0x1A]));
   pStatus->corrBlockCount = (buf[0x23] << 24) + (buf[0x24] << 16) + (buf[0x25] << 8) + buf[0x26];
   pStatus->ucorrBlockCount = (buf[0x27] << 24) + (buf[0x28] << 16) + (buf[0x29] << 8) + buf[0x2A];
   pStatus->berCount = (buf[0x1F] << 24) + (buf[0x20] << 16) + (buf[0x21] << 8) + buf[0x22];
   pStatus->snr = (buf[0x11] << 8) + buf[0x12];
   pStatus->oob_agc = (buf[0x1B] << 8) + buf[0x1C];
   pStatus->ext_agc = (buf[0x1D] << 8) + buf[0x1E];   
   pStatus->reacqCount = (buf[0x2B] << 24) + (buf[0x2C] << 16) + (buf[0x2D] << 8) + buf[0x2E];
   pStatus->atm_cell_total_count = (buf[0x2F] << 24) + (buf[0x30] << 16) + (buf[0x31] << 8) + buf[0x32];
   pStatus->atm_cell_loss_count = (buf[0x33] << 24) + (buf[0x34] << 16) + (buf[0x35] << 8) + buf[0x36];

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ResetOobStatus()
******************************************************************************/
BERR_Code BVSB_3520_P_ResetOobStatus(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x21;
   buf[1] = 0x04;
   buf[2] = 0;
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 3, buf, 1, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ConfigBtsc()
******************************************************************************/
BERR_Code BVSB_3520_P_ConfigBtsc(
   BVSB_Handle h,             /* [in] BVSB handle */
   BVSB_BtscSettings *pParams /* [in] BTSC configuration parameters */
)
{
   BERR_Code retCode;
   uint8_t buf[3];

   buf[0] = 0x1D;
   buf[2] = 0x00;
   
   switch (pParams->decodeMode)
   {
      case BVSB_BtscDecodeMode_eMono:
         buf[1] = BVSB_AUDIO_CONFIG_DECODE_MODE_MONO;
         break;
         
      case BVSB_BtscDecodeMode_eStereo:
         buf[1] = BVSB_AUDIO_CONFIG_DECODE_MODE_STEREO;      
         break;
      
      case BVSB_BtscDecodeMode_eSAP:
         buf[1] = BVSB_AUDIO_CONFIG_DECODE_MODE_SAP;      
         break;
         
      case BVSB_BtscDecodeMode_eSAP_Mono:
         buf[1] = BVSB_AUDIO_CONFIG_DECODE_MODE_SAP_MONO;      
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
     
   switch (pParams->sampleRate)
   {
      case BVSB_BtscSampleRate_e32KHz:
         buf[1] |= BVSB_AUDIO_CONFIG_SR_32KHZ;
         break;
         
      case BVSB_BtscSampleRate_e44_1KHz:
         buf[1] |= BVSB_AUDIO_CONFIG_SR_44_1KHZ;
         break;
         
      case BVSB_BtscSampleRate_e48KHz:
         buf[1] |= BVSB_AUDIO_CONFIG_SR_48KHZ;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   buf[1] |= pParams->bI2sOut ? 0x40 : 0x00;
   
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 3, buf, 1, true));
   BKNI_Memcpy(&(h->settings.btsc), pParams, sizeof(BVSB_BtscSettings));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_SetBtscVolume()
******************************************************************************/
BERR_Code BVSB_3520_P_SetBtscVolume(
   BVSB_Handle h,        /* [in] BVSB handle */
   uint8_t left_volume,  /* [in] left volume attenuation in dB (0 to 95 dB) */
   uint8_t right_volume, /* [in] left volume attenuation in dB (0 to 95 dB) */
   bool    bMute
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x17;
   buf[1] = left_volume & 0x7F;
   buf[1] |= (bMute ? BVSB_AUDIO_CONFIG_MUTE : 0x00);
   buf[2] = right_volume & 0x7F;
   buf[3] = 0;
   
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 4, buf, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_SetInbandOi()
******************************************************************************/
BERR_Code BVSB_3520_P_SetInbandOi(
   BVSB_Handle           h,         /* [in] BVSB handle */
   BVSB_InbandOiSettings *pInbandOi /* [in] inband transport output interface settings */
)
{
   BERR_Code retCode;
   uint8_t buf[4];

   buf[0] = 0x0F;
   buf[1] = buf[2] = buf[3] = 0;
   buf[1] |= (pInbandOi->bHead4 ? 0x40 : 0x00); 
   buf[1] |= (pInbandOi->bSync1 ? 0x20 : 0x00); 
   buf[1] |= (pInbandOi->bXBERT ? 0x04 : 0x00); 
   buf[2] |= (pInbandOi->bErrinv ? 0x80 : 0x00); 
   buf[2] |= (pInbandOi->bSyncinv ? 0x40 : 0x00); 
   buf[2] |= (pInbandOi->bVldinv ? 0x20 : 0x00); 
   buf[2] |= (pInbandOi->bClksup ? 0x10 : 0x00); 
   buf[2] |= (pInbandOi->bClkinv ? 0x08 : 0x00); 
   buf[2] |= (pInbandOi->bSerial ? 0x02 : 0x00); 

   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, buf, 4, buf, 1, true));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_3520_P_GetSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;
   
   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   hab[0] = 0x15;
   hab[1] = 0x4A;
   hab[2] = 0x28;
   hab[3] = 0x1E;
   hab[0x7C] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, hab, 4));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x7C, &hab[0x7C], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, hab, 0x7C, true, 0x95));
   
   for (i = 0; i < 30; i++)
   {
      pI[i] = (hab[4+(4*i)] << 4) | ((hab[5+(4*i)] >> 4) & 0x0F);
      pQ[i] = (hab[6+(4*i)] << 4) | ((hab[7+(4*i)] >> 4) & 0x0F);
      
      if (pI[i] & 0x800)
         pI[i] -= 0x1000;
      if (pQ[i] & 0x800)
         pQ[i] -= 0x1000;
   }
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetOobSoftDecisionBuf()
******************************************************************************/
BERR_Code BVSB_3520_P_GetOobSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;
   
   if ((retCode = BVSB_3520_P_CheckHab(h)) != BERR_SUCCESS)
   {
      BDBG_WRN(("HAB not available\n"));
      goto done;
   }

   hab[0] = 0x15;
   hab[1] = 0x28;
   hab[2] = 0x4C;
   hab[3] = 0x1E;
   hab[0x7C] = 0x00;
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, hab, 4));
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0x7C, &hab[0x7C], 1));
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, hab, 0x7C, true, 0x95));
   
   for (i = 0; i < 30; i++)
   {
      pI[i] = (int16_t)(hab[4+(4*i)]);
      pQ[i] = (int16_t)(hab[5+(4*i)]);
   }
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_SetTmConfig()
******************************************************************************/
BERR_Code BVSB_3520_P_SetTmConfig(
   BVSB_Handle h,    /* [in] BVSB handle */
   void        *p    /* [in] pad configuration parameters */
)
{
   BERR_Code retCode;
   uint8_t hab[16];
   BVSB_3520_PadConfig *pCfg = (BVSB_3520_PadConfig *)p;

   /* pad_control_1 */
   hab[0] = 0x1B;
   hab[1] = ((uint8_t)(pCfg->driveStrength_ps_data_7) & 0x03) << 4;
   hab[1] |= (((uint8_t)(pCfg->driveStrength_ps_data_6) & 0x03) << 2);
   hab[1] |= ((uint8_t)(pCfg->driveStrength_ps_data_5) & 0x03);
   hab[2] = ((uint8_t)(pCfg->driveStrength_ps_data_4) & 0x03) << 6;
   hab[2] |= (((uint8_t)(pCfg->driveStrength_ps_data_3) & 0x03) << 4);
   hab[2] |= (((uint8_t)(pCfg->driveStrength_ps_data_2) & 0x03) << 2);
   hab[2] |= ((uint8_t)(pCfg->driveStrength_ps_data_1) & 0x03);
   hab[3] = ((uint8_t)(pCfg->driveStrength_ps_data_0) & 0x03) << 6;
   hab[3] |= (((uint8_t)(pCfg->driveStrength_ps_clk) & 0x03) << 4);
   hab[3] |= (((uint8_t)(pCfg->driveStrength_ps_err) & 0x03) << 2);
   hab[3] |= ((uint8_t)(pCfg->driveStrength_ps_valid) & 0x03);
   hab[4] = ((uint8_t)(pCfg->driveStrength_ps_sync) & 0x03) << 6;
   hab[4] |= (pCfg->bTristate_clk_adc_f1b ? 0x20 : 0x00);
   hab[4] |= (pCfg->bTristate_clk_adc ? 0x10 : 0x00);
   hab[4] |= (((uint8_t)(pCfg->driveStrength_oob_clk) & 0x03) << 2);
   hab[4] |= ((uint8_t)(pCfg->driveStrength_oob_sdata) & 0x03);
   hab[5] = (pCfg->bTristate_gpo_7 ? 0x80 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_6 ? 0x40 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_5 ? 0x20 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_4 ? 0x10 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_3 ? 0x08 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_2 ? 0x04 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_1 ? 0x02 : 0x00);
   hab[5] |= (pCfg->bTristate_gpo_0 ? 0x01 : 0x00);
   hab[6] = ((uint8_t)(pCfg->driveStrength_rn_mclk) & 0x03) << 6;
   hab[6] |= (((uint8_t)(pCfg->driveStrength_rp_sclk) & 0x03) << 4);
   hab[6] |= (((uint8_t)(pCfg->driveStrength_ln_lr) & 0x03) << 2);
   hab[6] |= ((uint8_t)(pCfg->driveStrength_lp_data) & 0x03);
  
   /* pad_control_2 */
   hab[7] = 0x22;
   hab[8] = ((uint8_t)(pCfg->f1bsel) & 0x03) << 6;
   hab[8] |= (((uint8_t)(pCfg->adcsel) & 0x03) << 4);
   hab[8] |= (pCfg->bOpenDrain_hm ? 0x00 : 0x02);
   hab[8] |= (pCfg->bOpenDrain_gm ? 0x00 : 0x01);
   hab[9] = (pCfg->gpio_01 & 0x07) << 5;
   hab[9] |= (pCfg->bErs ? 0x10 : 0x00);
   hab[9] |= (pCfg->bTristate_at ? 0x01 : 0x00);
   hab[10] = (pCfg->oob_gpo & 0x07);
   hab[10] |= (pCfg->bOpenDrain_am ? 0x00 : 0x80);
   hab[10] |= (pCfg->bTristate_it ? 0x40 : 0x00);
   hab[10] |= (pCfg->bOpenDrain_im ? 0x00 : 0x20);
   hab[10] |= (pCfg->bTristate_ot ? 0x10 : 0x00);
   hab[10] |= (pCfg->bOpenDrain_om ? 0x00 : 0x08);
   hab[11] = (pCfg->agcgpsel & 0x03) << 1; 
   hab[11] |= ((pCfg->agcisel & 0x03) << 3); 
   hab[11] |= ((pCfg->agctsel & 0x03) << 5); 

   hab[12] = 0;

   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 13, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_GetTmConfig()
******************************************************************************/
BERR_Code BVSB_3520_P_GetTmConfig(
   BVSB_Handle h,  /* [in] BVSB handle */
   void        *p  /* [out] pad configuration parameters */
)
{
   BERR_Code retCode;
   BVSB_3520_PadConfig *pCfg = (BVSB_3520_PadConfig *)p;
   uint8_t buf[10];

   BVSB_CHK_RETCODE(BVSB_3520_P_ReadConfig(h, BVSB_CONFIG_PAD_CTRL_1, buf, 10));

   /* pad_control_1 */
   pCfg->driveStrength_ps_data_7 = (BVSB_3520_PadDriveStrength)((buf[0] >> 4) & 0x03);
   pCfg->driveStrength_ps_data_6 = (BVSB_3520_PadDriveStrength)((buf[0] >> 2) & 0x03);
   pCfg->driveStrength_ps_data_5 = (BVSB_3520_PadDriveStrength)(buf[0] & 0x03);
   pCfg->driveStrength_ps_data_4 = (BVSB_3520_PadDriveStrength)((buf[1] >> 6) & 0x03);
   pCfg->driveStrength_ps_data_3 = (BVSB_3520_PadDriveStrength)((buf[1] >> 4) & 0x03);
   pCfg->driveStrength_ps_data_2 = (BVSB_3520_PadDriveStrength)((buf[1] >> 2) & 0x03);
   pCfg->driveStrength_ps_data_1 = (BVSB_3520_PadDriveStrength)(buf[1] & 0x03);
   pCfg->driveStrength_ps_data_0 = (BVSB_3520_PadDriveStrength)((buf[2] >> 6) & 0x03);
   pCfg->driveStrength_ps_clk = (BVSB_3520_PadDriveStrength)((buf[2] >> 4) & 0x03);
   pCfg->driveStrength_ps_err = (BVSB_3520_PadDriveStrength)((buf[2] >> 2) & 0x03);
   pCfg->driveStrength_ps_valid = (BVSB_3520_PadDriveStrength)(buf[2] & 0x03);
   pCfg->driveStrength_ps_sync = (BVSB_3520_PadDriveStrength)((buf[3] >> 6) & 0x03);
   pCfg->bTristate_clk_adc_f1b = (buf[3] & 0x20) ? true : false;
   pCfg->bTristate_clk_adc = (buf[3] & 0x10) ? true : false;
   pCfg->driveStrength_oob_clk = (BVSB_3520_PadDriveStrength)((buf[3] & 0x03) >> 2);
   pCfg->driveStrength_oob_sdata = (BVSB_3520_PadDriveStrength)(buf[3] & 0x03);
   pCfg->bTristate_gpo_7 = (buf[4] & 0x80 ? true : false);
   pCfg->bTristate_gpo_6 = (buf[4] & 0x40 ? true : false);
   pCfg->bTristate_gpo_5 = (buf[4] & 0x20 ? true : false);
   pCfg->bTristate_gpo_4 = (buf[4] & 0x10 ? true : false);
   pCfg->bTristate_gpo_3 = (buf[4] & 0x08 ? true : false);
   pCfg->bTristate_gpo_2 = (buf[4] & 0x04 ? true : false);
   pCfg->bTristate_gpo_1 = (buf[4] & 0x02 ? true : false);
   pCfg->bTristate_gpo_0 = (buf[4] & 0x01 ? true : false);
   pCfg->driveStrength_rn_mclk = (BVSB_3520_AudioPadDriveStrength)((buf[5] >> 6) & 0x03);
   pCfg->driveStrength_rp_sclk = (BVSB_3520_AudioPadDriveStrength)((buf[5] >> 4) & 0x03);
   pCfg->driveStrength_ln_lr = (BVSB_3520_AudioPadDriveStrength)((buf[5] >> 2) & 0x03);
   pCfg->driveStrength_lp_data = (BVSB_3520_AudioPadDriveStrength)(buf[5] & 0x03);

   /* pad_control_2 */
   pCfg->f1bsel = (BVSB_F1BADC_OutputFormat)((buf[6] >> 6) & 0x03);
   pCfg->adcsel = (BVSB_ADCSEL_OutputFormat)((buf[6] >> 4) & 0x03);
   pCfg->bOpenDrain_hm = (buf[6] & 0x02) ? false : true;
   pCfg->bOpenDrain_gm = (buf[6] & 0x01) ? false : true;
   pCfg->gpio_01 = ((buf[7] >> 5) & 0x07);
   pCfg->bErs = (buf[7] & 0x10) ? true : false;
   pCfg->bTristate_at = (buf[7] & 0x01) ? true : false;
   pCfg->bOpenDrain_am = (buf[8] & 0x80) ? false : true;
   pCfg->bTristate_it = (buf[8] & 0x40) ? true : false;
   pCfg->bOpenDrain_im = (buf[8] & 0x20) ? false : true;
   pCfg->bTristate_ot = (buf[8] & 0x10) ? true : false;
   pCfg->bOpenDrain_om = (buf[8] & 0x08) ? false : true;
   pCfg->oob_gpo = (buf[8] & 0x07);
   pCfg->agctsel = ((buf[9] >> 5) & 0x03);
   pCfg->agcisel = ((buf[9] >> 3) & 0x03);
   pCfg->agcgpsel = ((buf[9] >> 1) & 0x03);

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_WriteConfig()
******************************************************************************/
BERR_Code BVSB_3520_P_WriteConfig(
   BVSB_Handle h,   /* [in] BVSB handle */
   uint16_t offset, /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,    /* [in] data to write */
   uint8_t len      /* [in] number of bytes to write */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;

   if ((len < 1) || (len > 121))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   hab[0] = 0x13;
   hab[1] = (offset >> 8);
   hab[2] = (offset & 0xFF);   
   hab[3] = len;
   hab[4+len] = 0x00;

   for (i = 0; i < len; i++)
      hab[4+i] = buf[i];
   
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 5+len, hab, 1, true));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ReadConfig()
******************************************************************************/
BERR_Code BVSB_3520_P_ReadConfig(
   BVSB_Handle h,  /* [in] BVSB handle */
   uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,      /* [in] buffer to hold the read data */
   uint8_t len        /* [in] number of bytes to read */
)
{
   BERR_Code retCode;
   uint8_t   hab[128], i;

   if ((len < 1) || (len > 121))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   hab[0] = 0x12;
   hab[1] = (offset >> 8);
   hab[2] = (offset & 0xFF);   
   hab[3] = len;
   hab[4+len] = 0;

   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 5+len, hab, 5+len, true));

   for (i = 0; i < len; i++)
      buf[i] = hab[4+i];

   done:
   return retCode;
}

/******************************************************************************
 BVSB_3520_P_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_3520_P_GetLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3520_P_Handle *)(h->pImpl))->hLockChangeEvent;
   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_3520_P_GetOobLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BVSB_3520_P_GetOobLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3520_P_Handle *)(h->pImpl))->hOobLockChangeEvent;
   return BERR_SUCCESS;
}



/******************************************************************************
 BVSB_3520_P_GetAntennaEventHandle()
******************************************************************************/
BERR_Code BVSB_3520_P_GetAntennaEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
)
{
   *hEvent = ((BVSB_3520_P_Handle *)(h->pImpl))->hAntennaEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3520_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BVSB_3520_P_GetInterruptEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
)
{
   *hEvent = ((BVSB_3520_P_Handle *)(h->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3520_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BVSB_3520_P_HandleInterrupt_isr(
   BVSB_Handle h /* [in] BVSB handle */
)
{
   BDBG_ASSERT(h);
   
/*uarta_out('!');*/
   h->settings.i2c.interruptEnableFunc(false, h->settings.i2c.interruptEnableFuncParam);
   BKNI_SetEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hApiEvent);   
   BKNI_SetEvent(((BVSB_3520_P_Handle *)(h->pImpl))->hInterruptEvent);  
   return BERR_SUCCESS;
}


/******************************************************************************
 BVSB_3520_P_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BVSB_3520_P_ProcessInterruptEvent(
   BVSB_Handle h  /* [in] BVSB handle */
)
{
   BERR_Code retCode;
   
   BDBG_ASSERT(h);
   BVSB_CHK_RETCODE(BVSB_3520_P_DecodeInterrupt(h));
   BVSB_3520_P_EnableHostInterrupt(h, true);
   
   done:
   return retCode;
}

/******************************************************************************
 BVSB_3520_P_ReadMbox()
******************************************************************************/
BERR_Code BVSB_3520_P_ReadMbox(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [out] value read from register */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb, i, buf[4];

   /* update bits 15:8 of mbox address if changed */
   sb = reg >> 8;
   if (sb != p3520->last_mbox_15_8)
   {
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_SFR_IO_MBOX_A_15_8, &sb));
      p3520->last_mbox_15_8 = sb;
   }

   /* read from mbox */
   sb = reg & 0xFC;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_SFR_IO_MBOX_CMD, &sb));

   /* check for mbox transfer complete */
   for (i = 0; i < 3; i++)
   {
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_SFR_IO_MBOX_STATUS, &sb));
      if ((sb & 0x80) == 0)
      {
         if (sb & 0x40)
         {
            BERR_TRACE(retCode = BVSB_ERR_IOMB_XFER);
            goto done;
         }
         else
         {
            /* transfer completed - now get the data */
            BVSB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_SFR_IO_MBOX_D_31_24, buf, 4));
            *val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
         }
         break;
      }
   }

   if (i >= 3)
   {
      /* this should not happen */
      BERR_TRACE(retCode = BVSB_ERR_IOMB_BUSY);
      BDBG_ERR(("IO_MBOX busy\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_WriteMbox()
******************************************************************************/
BERR_Code BVSB_3520_P_WriteMbox(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [in] value to write */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t buf[6], sb, i;

   /* write addr, data, and cmd in one i2c transaction */
   buf[0] = reg >> 8;
   buf[1] = *val >> 24;
   buf[2] = *val >> 16;
   buf[3] = *val >> 8;
   buf[4] = *val & 0xFF; 
   buf[5] = (reg & 0xFC) | 0x01;
   i = (buf[0] != p3520->last_mbox_15_8) ? 0 : 1;
   p3520->last_mbox_15_8 = buf[0];
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, i ? BCM3520_SH_SFR_IO_MBOX_D_31_24 : BCM3520_SH_SFR_IO_MBOX_A_15_8, &buf[i], 6-i));

   for (i = 0; i < 3; i++)
   {
      /* check for mbox transfer complete */
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_SFR_IO_MBOX_STATUS, &sb));
      if ((sb & 0x80) == 0)
      {
         if (sb & 0x40)
         {
            BERR_TRACE(retCode = BVSB_ERR_IOMB_XFER);
         }
         goto done;
      }
   }

   if (sb & 0x80)
   {
      /* this should not happen */
      BERR_TRACE(retCode = BVSB_ERR_IOMB_BUSY);
      BDBG_ERR(("IO_MBOX busy\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ReadHab()
******************************************************************************/
BERR_Code BVSB_3520_P_ReadHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BVSB_CHK_RETCODE(BVSB_3520_P_SetApWindow(h, BVSB_WINDOW_HAB));
   BVSB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_WriteHab()
******************************************************************************/
BERR_Code BVSB_3520_P_WriteHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if ((addr & 0x80) || (n & 0x80))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 0x80)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BVSB_CHK_RETCODE(BVSB_3520_P_SetApWindow(h, BVSB_WINDOW_HAB));
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, addr, buf, n));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ReadMemory()
******************************************************************************/
BERR_Code BVSB_3520_P_ReadMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;

   if (((uint32_t)addr + (uint32_t)n) > 0x10000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   BVSB_CHK_RETCODE(BVSB_3520_P_SetApWindow(h, BVSB_WINDOW_IRAM + addr));
   BVSB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, addr & 0x7F, buf, n));
   p3520->last_page_16_15 = p3520->last_page_14_7 = 0xFF; 

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_WriteMemory()
******************************************************************************/
BERR_Code BVSB_3520_P_WriteMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t  curr_addr, nbytes, bytes_left;

   if ((addr >= 0x8000) || (n >= 0x8000))
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   if ((addr + n) > 0x8000)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   curr_addr = addr;
   bytes_left = n;
   while (bytes_left > 0)
   {
      BVSB_CHK_RETCODE(BVSB_3520_P_SetApWindow(h, BVSB_WINDOW_IRAM + curr_addr));
      nbytes = 128 - (curr_addr % 128);
      if (nbytes > bytes_left)
         nbytes = bytes_left;
      bytes_left -= nbytes;
      BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, curr_addr & 0x7F, buf, nbytes));
      curr_addr += nbytes;
      buf += nbytes;
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_SetApWindow()
******************************************************************************/
BERR_Code BVSB_3520_P_SetApWindow(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint32_t window   /* [in] base address of the 128-byte window */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t   haddr_16_15 = (window >> 15) & 0x03;
   uint8_t   haddr_14_7 = (window >> 7) & 0xFF;
   uint8_t   buf[2];

   if (p3520->last_page_16_15 != haddr_16_15)
   {
      buf[0] = haddr_16_15;
      buf[1] = haddr_14_7;
      BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_SFR_H_ADR_16_15, buf, 2));
      p3520->last_page_16_15 = haddr_16_15;
      p3520->last_page_14_7 = haddr_14_7;
   }
   else if (p3520->last_page_14_7 != haddr_14_7)
   {
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_SFR_H_ADR_14_7, &haddr_14_7));
      p3520->last_page_14_7 = haddr_14_7;
   }

   done:
   return retCode;
}

/******************************************************************************
 BVSB_3520_P_ResetAp()
******************************************************************************/
BERR_Code BVSB_3520_P_ResetAp(BVSB_Handle h)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t i, sb, buf[4];

   /* initialize JDEC */
   sb = BCM3520_JDEC_RAM;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_JDEC, &sb));

   /* reset the AP */
   sb = BCM3520_AP_RESET;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb));

   /* reset AP status */
   sb = 0x0B;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_AP_CMD, &sb));

   /* reset all interrupt status */
   for (i = 0; i < 4; i++)
      buf[i] = 0xFF;
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_STAT1, buf,
 4));

   /* clear MSGx registers */
   buf[0] = buf[1] = 0x00;
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_MSG1, buf,
2));

   /* verify that AP is reset */
   BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb));
   if ((sb & BCM3520_AP_MASK) != BCM3520_AP_RESET)
   {
      BDBG_ERR(("unable to reset the AP\n"));
      BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
   }

   done:
   p3520->last_page_16_15 = 0xFF;
   p3520->last_page_14_7 = 0xFF;
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_RunAp()
******************************************************************************/
BERR_Code BVSB_3520_P_RunAp(BVSB_Handle h)
{
   BERR_Code retCode;
   uint8_t   sb, sb2;

   /* check if the AP is currently running */
   BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb)); 

   if ((sb & BCM3520_AP_MASK) != BCM3520_AP_RUN)
   {
      /* start running the AP */
      sb2 = BCM3520_AP_RUN;
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb2));

      /* verify that the AP is running */
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb));
      if ((sb & BCM3520_AP_MASK) != BCM3520_AP_RUN)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
         goto done;
      }

      /* clear AP_change state bit */
      sb2 = BCM3520_STAT1_AP_OP_CHG;
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT1, &sb2)); 
   }
   else
   {
      BDBG_WRN(("BVSB_3520_P_RunAp(): AP already running\n"));
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BVSB_3520_P_DisableInterrupts(
   BVSB_Handle h   /* [in] BVSB PI Handle */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code err;
   const uint8_t val[4] = {0, 0, 0, 0};
   
   /* clear IEx registers */
   err = BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_IE1, val, 4);
   return err;   
}


/******************************************************************************
 BVSB_3520_P_DecodeError()
******************************************************************************/
BERR_Code BVSB_3520_P_DecodeError(
   BVSB_Handle h,           /* [in] BVSB PI Handle */
   BVSB_ApStatus *pApStatus /* [in] AP status returned by BVSB_GetApStatus */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t sb;

   if (*pApStatus & BVSB_APSTATUS_HAB_ERR)
      retCode = BVSB_ERR_HABAV;
   else if (*pApStatus & BVSB_APSTATUS_MEM_ERR)
      retCode = BVSB_ERR_MEMAV;
   else if (*pApStatus & BVSB_APSTATUS_H_ERR)
      retCode = BVSB_ERR_HOST_XFER;
   else if (*pApStatus & BVSB_APSTATUS_IOMB_ERR)
      retCode = BVSB_ERR_IOMB_XFER;
   else if (*pApStatus & BVSB_APSTATUS_HABCMD_ERR)
      retCode = BVSB_ERR_HAB_ERR;
   else if (*pApStatus & BVSB_APSTATUS_AP_ERR)
   {
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_MSG1, &sb));
      switch (sb)
      {
         case 1:
            retCode = BVSB_ERR_AP_BSC;
            break;

         case 2:
            retCode = BVSB_ERR_AP_STACK;
            break;

         case 3:
            retCode = BVSB_ERR_AP_WD;
            break;

         case 4:
            retCode = BVSB_ERR_AP_ISB;
            break;

         case 5:
            retCode = BVSB_ERR_AP_SCR;
            break;

         case 6:
            retCode = BVSB_ERR_AP_IRQ;
            break;

         case 7:
            retCode = BVSB_ERR_AP_COPY;
            break;

         case 8:
            retCode = BVSB_ERR_AP_EEPROM;
            break;

         case 9:
            retCode = BVSB_ERR_AP_HABAV;
            break;

         default:
            BDBG_ERR(("unknown MSG1 (=0x%02X)\n", sb));
            retCode = BVSB_ERR_AP_UNKNOWN;
            break;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_SendHabCommand()
******************************************************************************/
BERR_Code BVSB_3520_P_SendHabCommand(
   BVSB_Handle h,      /* [in] BVSB PI Handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send */
   uint8_t write_len,  /* [in] number of bytes in the HAB command */ 
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck   /* [in] true = determine if the AP has serviced the command */
)
{
   BERR_Code retCode;
   
   if ((write_len > 127) || (read_len > 127) || (write_len == 0))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BVSB_CHK_RETCODE(BVSB_3520_P_CheckHab(h));
  
   /* write the command to the HAB */
   BVSB_CHK_RETCODE(BVSB_3520_P_WriteHab(h, 0, write_buf, write_len));

   /* wait for the AP to service the HAB, and then read any return data */
   BVSB_CHK_RETCODE(BVSB_3520_P_ServiceHab(h, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80));
 
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_ServiceHab()
******************************************************************************/
#define BVSB_USE_HAB_INTERRUPT
BERR_Code BVSB_3520_P_ServiceHab(
   BVSB_Handle h,   /* [in] BVSB PI Handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
   uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t   sb, ie2, ie3, buf[3];

#ifdef BVSB_USE_HAB_INTERRUPT
/*uarta_out('S');*/
   BVSB_CHK_RETCODE(BVSB_3520_P_EnableHostInterrupt(h, false));
   
   /* save ie2 */
   if ((retCode = BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_IE2, &ie2)) != BERR_SUCCESS)
      return retCode;
   if ((retCode = BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_IE3, &ie3)) != BERR_SUCCESS)
      return retCode;
   
   /* clear ie1/2/3 */
   buf[0] = buf[1] = buf[2] = 0;
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_IE1, buf, 3));

   BKNI_WaitForEvent(p3520->hHabDoneEvent, 0);      
#endif
   
   /* send the command */
   sb = BCM3520_AP_HABR;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb));
   
#ifdef BVSB_USE_HAB_INTERRUPT
   /* enable the hab done interrupt mask */
   sb = BCM3520_STAT1_HAB_DONE;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE1, &sb));
   
   /* wait for HAB done interrupt */  
   if (BVSB_3520_P_WaitForEvent(h, p3520->hHabDoneEvent, 100) == BERR_TIMEOUT)
   {
      BDBG_ERR(("HAB timeout\n"));   
      sb = 0;
      BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE1, &sb));
      BERR_TRACE(retCode = BVSB_ERR_HAB_TIMEOUT);
      goto done;
   }
#else
{
   int i;
   uint8_t sb;

   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 500; i++)
   {
      BVSB_CHK_RETCODE(BVSB_ReadHostRegister(h, BCM3520_SH_AP_SFR_H_CTL1, &sb));
      if ((sb & BCM3520_AP_HABR) == 0)
      {
         break;
      }
   }
   if (sb & BCM3520_AP_HABR)
   {
      BERR_TRACE(retCode = BVSB_ERR_HAB_TIMEOUT);
      goto done;
   }
}      
#endif
   
   if (read_len > 0)
   {
      BVSB_CHK_RETCODE(BVSB_3520_P_ReadHab(h, 0, read_buf, read_len));
      if (bCheckForAck)
      {
         if (ack_byte != read_buf[0])
         {
            BDBG_ERR(("HAB command not serviced!\n"));
            BERR_TRACE(retCode = BVSB_ERR_HAB_NO_ACK);
         }
      }
   }

   done:
#ifdef BVSB_USE_HAB_INTERRUPT
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE2, &ie2));
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE3, &ie3));
#endif
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_CheckHab()
******************************************************************************/
BERR_Code BVSB_3520_P_CheckHab(
   BVSB_Handle h    /* [in] BVSB PI Handle */
)
{
   BERR_Code retCode;
   BVSB_ApStatus status;

   BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
   if ((status & BVSB_APSTATUS_HAB_MASK) == BVSB_APSTATUS_HAB_READY)
      retCode = BERR_SUCCESS;
   else
   {
      BDBG_ERR(("AP status = 0x%08X\n", (uint32_t)status));
      BERR_TRACE(retCode = BVSB_3520_P_DecodeError(h, &status));
   }

   done:
   return retCode;
}



/******************************************************************************
 BVSB_3520_P_EnableHostInterrupt() 
******************************************************************************/
BERR_Code BVSB_3520_P_EnableHostInterrupt(
   BVSB_Handle h, /* [in] BVSB PI handle */
   bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
)
{
   BKNI_EnterCriticalSection();
   h->settings.i2c.interruptEnableFunc(bEnable, h->settings.i2c.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();   

   return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_3520_P_EnableOobLockInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3520_P_EnableOobLockInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb;

   BVSB_3520_P_EnableHostInterrupt(h, false);
   sb = BCM3520_STAT3_OOB_LOCK_MASK;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT3, &sb));
   sb = bEnable ? BCM3520_STAT3_OOB_LOCKED : 0;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE3, &sb));
   /* SW3560-38: This causes a race condition */
   /* BKNI_WaitForEvent(p3520->hInterruptEvent, 0); */
   BKNI_WaitForEvent(p3520->hApiEvent, 0);   
   BKNI_WaitForEvent(p3520->hOobLockChangeEvent, 0);   
   BVSB_3520_P_EnableHostInterrupt(h, true);

done:
    return retCode;   
}

/******************************************************************************
 BVSB_3520_P_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3520_P_EnableLockInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb;
   
   BVSB_3520_P_EnableHostInterrupt(h, false);
   sb = BCM3520_STAT2_LOCK_MASK;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT2, &sb));
   sb = bEnable ? BCM3520_STAT2_IN_LOCK : 0;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE2, &sb));
   /* SW3560-38: This causes a race condition */
   /* BKNI_WaitForEvent(p3520->hInterruptEvent, 0); */
   BKNI_WaitForEvent(p3520->hApiEvent, 0);   
   BKNI_WaitForEvent(p3520->hLockChangeEvent, 0);   
   BVSB_3520_P_EnableHostInterrupt(h, true);
   
   done:
   return retCode;   
}


/******************************************************************************
 BVSB_3520_P_EnableInitDoneInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3520_P_EnableInitDoneInterrupt(
   BVSB_Handle h /* [in] BVSB PI Handle */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb;
   
   sb = BCM3520_STAT2_INIT_DONE;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT2, &sb));
   sb = BCM3520_STAT2_INIT_DONE;
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE2, &sb));
   /* SW3560-38: This causes a race condition */
   /* BKNI_WaitForEvent(p3520->hInterruptEvent, 0); */
   BKNI_WaitForEvent(p3520->hApiEvent, 0);   
   BKNI_WaitForEvent(p3520->hInitDoneEvent, 0);   
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_EnableBscInterrupt()
******************************************************************************/ 
BERR_Code BVSB_3520_P_EnableBscInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable BSC finished interrupt */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t sb = bEnable ? BCM3520_STAT2_BSC : 0;
   
   BVSB_3520_P_EnableHostInterrupt(h, false);
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_IE2, &sb));
   /* SW3560-38: This causes a race condition */
   /* BKNI_WaitForEvent(p3520->hInterruptEvent, 0); */
   BKNI_WaitForEvent(p3520->hBscEvent, 0);   
   BVSB_3520_P_EnableHostInterrupt(h, true);
   
   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BVSB_3520_P_DecodeInterrupt(BVSB_Handle h)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   uint8_t   h_ie[3], h_fstat[3], new_stat2, new_stat3;

   BVSB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_FSTAT1, h_fstat, 3));
   BVSB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_IE1, h_ie, 3));
   
   new_stat2 = 0;
   new_stat3 = 0;
   
   if (!h_fstat[0] && !h_fstat[1] && !h_fstat[2])
   {
      return BERR_SUCCESS;
   }
   
   if (h_fstat[0] & BCM3520_STAT1_HAB_DONE)
   {
      h_ie[0] = 0;
      BKNI_SetEvent(p3520->hHabDoneEvent);
   }
   
   if (h_fstat[1] & BCM3520_STAT2_INIT_DONE)
   {
      h_ie[1] &= ~BCM3520_STAT2_INIT_DONE;
      BKNI_SetEvent(p3520->hInitDoneEvent);
   }
      
   if (h_fstat[1] & BCM3520_STAT2_LOCK_MASK)
   {
      new_stat2 |= BCM3520_STAT2_LOCK_MASK;
      h_ie[1] &= ~BCM3520_STAT2_LOCK_MASK;
      BKNI_SetEvent(p3520->hLockChangeEvent);
      if (h_fstat[1] & BCM3520_STAT2_IN_LOCK)
      {
         h_ie[1] |= BCM3520_STAT2_OUT_OF_LOCK;
      }
      else
      {
         h_ie[1] |= BCM3520_STAT2_IN_LOCK;  
      }
   }

   if (h_fstat[2] & BCM3520_STAT3_OOB_LOCK_MASK)
   {
      new_stat3 |= BCM3520_STAT3_OOB_LOCK_MASK;
      h_ie[2] &= ~BCM3520_STAT3_OOB_LOCK_MASK;
      BKNI_SetEvent(p3520->hOobLockChangeEvent);
      if (h_fstat[2] & BCM3520_STAT3_OOB_LOCKED)
      {
         h_ie[2] |= BCM3520_STAT3_OOB_NOT_LOCKED;
      }
      else
      {
         h_ie[2] |= BCM3520_STAT3_OOB_LOCKED;  
      }
   }
         
   /* reset ie registers */
   BVSB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_IE1, h_ie, 3));   

   /* clear the interrupt status */
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT2, &new_stat2));             
   BVSB_CHK_RETCODE(BVSB_WriteHostRegister(h, BCM3520_SH_AP_SFR_H_STAT3, &new_stat3));             
   done:
   return retCode;
}



/******************************************************************************
 BVSB_3520_P_AcquireVsb()
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireVsb(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[16];

   hab[0] = 0x0A;
   hab[1] = hab[2] = hab[10] = 0x00;
   hab[3] = 0x01;
   
   hab[1] |= ((h->settings.vsb.bAutoAcq) ? 0x20 : 0x00);
   hab[1] |= ((h->settings.vsb.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.vsb.bNtscSweep) ? 0x08 : 0x00);
   hab[1] |= ((h->settings.vsb.bRfiSweep) ? 0x04 : 0x00);
   hab[2] |= ((h->settings.vsb.bTei) ? 0x01 : 0x00);
   hab[2] |= ((h->settings.vsb.bTerr) ? 0x80 : 0x00);

   switch (pParams->mode)
   {
      case BVSB_InbandMode_e8VSB:
         /*hab[2] |= 0x00;*/
         break;

      case BVSB_InbandMode_e16VSB:
         hab[2] |= 0x40;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   switch (h->settings.vsb.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   hab[8] = (pParams->symbolRateOffset >> 8);
   hab[9] = (pParams->symbolRateOffset & 0xFF);

   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 11, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_AcquireQam()
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireQam(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[16];
   uint32_t  symbol_rate;

   /* If AnnexA then first set symbolrate */
   if ( pParams->mode >= BVSB_InbandMode_e256QAM_A )
   {
       /* not possible to be greater than 8 Mhz channel */
       if (( pParams->symbolRate > 8000000 ) || ( pParams->symbolRate < 0) )
       {
           BDBG_WRN(("symbolrate greater than 8Mhz!\n"));
           symbol_rate = 8000000;
       }
       else
           symbol_rate = pParams->symbolRate;

       hab[0] = 0x13;
       hab[1] = 0x00;
       hab[2] = 0x24;
       hab[3] = 0x04;
       hab[4] = ((symbol_rate & 0xFF000000) >> 24);
       hab[5] = ((symbol_rate & 0xFF0000) >> 16 );
       hab[6] = ((symbol_rate & 0xFF00) >> 8 );
       hab[7] = (symbol_rate & 0xFF);
       hab[8] = 0x00;
        
       BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 9, hab, 1, true));
   }

   hab[0] = 0x0B;
   hab[1] = hab[2] = hab[0x0A] = 0x00;
   hab[3] = 0x03;
   
   hab[1] |= ((h->settings.qam.bAutoAcq) ? 0x20 : 0x00);
   hab[1] |= ((h->settings.qam.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.qam.bSpinv) ? 0x10 : 0x00);
   hab[1] |= ((h->settings.qam.bEq) ? 0x04 : 0x00);
   hab[1] |= ((h->settings.qam.bCh) ? 0x02 : 0x00);
   
   hab[2] |= ((h->settings.qam.bTerr) ? 0x80 : 0x00);
   hab[2] |= ((pParams->mode >= BVSB_InbandMode_e256QAM_A) ? 0x00 : 0x08);
   hab[3] |= ((h->settings.qam.bDavic) ? 0x08 : 0x00);
   hab[3] |= (((uint8_t)(h->settings.qam.idepth) << 4) & 0xF0);

   switch (pParams->mode)
   {
      case BVSB_InbandMode_e1024QAM_B:
         hab[2] |= 0x70;
         break;

      case BVSB_InbandMode_e512QAM_B:
         hab[2] |= 0x60;
         break;
            
      case BVSB_InbandMode_e256QAM_B:
         hab[2] |= 0x50;
         break;

      case BVSB_InbandMode_e128QAM_B:
         hab[2] |= 0x40;
         break;

      case BVSB_InbandMode_e64QAM_B:
         hab[2] |= 0x30;
         break;

      case BVSB_InbandMode_e256QAM_A:
         hab[2] |= 0x50;
         break;

      case BVSB_InbandMode_e128QAM_A:
         hab[2] |= 0x40;
         break;

      case BVSB_InbandMode_e64QAM_A:
         hab[2] |= 0x30;
         break;

      case BVSB_InbandMode_e32QAM_A:
         hab[2] |= 0x20;
         break;

      case BVSB_InbandMode_e16QAM_A:
         hab[2] |= 0x10;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   switch (h->settings.qam.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   switch (h->settings.qam.nyquist)
   {
      case BVSB_NyquistFilter_e12:
         /*hab[1] |= 0x00;*/
         break;

      case BVSB_NyquistFilter_e15:
         hab[1] |= 0x40;
         break;

      case BVSB_NyquistFilter_e18:
         hab[1] |= 0x80;
         break;

      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
      
   if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   hab[8] = (pParams->symbolRateOffset >> 8);
   hab[9] = (pParams->symbolRateOffset & 0xFF);

   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 11, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BVSB_3520_P_AcquireNtsc()
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireNtsc(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
)
{
   BERR_Code retCode;
   uint8_t   hab[9];

   hab[0] = 0x0C;
   hab[1] = hab[2] = hab[8] = 0x00;
   hab[3] = 0x01;
   
   hab[1] |= ((h->settings.ntsc.bFastAcq) ? 0x01 : 0x00);
   hab[1] |= ((h->settings.ntsc.bAutoAcq) ? 0x20 : 0x00);
   hab[2] |= ((h->settings.ntsc.bTerr) ? 0x80 : 0x00);

   switch (h->settings.ntsc.bw)
   {
      case BVSB_PhaseLoopBw_eLow:
         /*hab[2] |= 0x00;*/
         break;
         
      case BVSB_PhaseLoopBw_eMedium:
         hab[2] |= 0x02;
         break;
         
      case BVSB_PhaseLoopBw_eHigh:
         hab[2] |= 0x04;
         break;
         
      default:
         return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }
   
   if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
      return BERR_INVALID_PARAMETER;
      
   hab[4] = (pParams->ifFreqOffset >> 24);
   hab[5] = (pParams->ifFreqOffset >> 16);
   hab[6] = (pParams->ifFreqOffset >> 8);
   hab[7] = (pParams->ifFreqOffset & 0xFF);
   
   BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 9, hab, 1, true));

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BVSB_3520_P_WaitForEvent()
******************************************************************************/
BERR_Code BVSB_3520_P_WaitForEvent(
   BVSB_Handle h,             /* [in] BVSB PI Handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
)
{
   BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   while (1)
   {   
      BVSB_3520_P_EnableHostInterrupt(h, true);
      if ((retCode = BKNI_WaitForEvent(p3520->hApiEvent, timeoutMsec)) == BERR_TIMEOUT)
         break;

      BVSB_3520_P_DecodeInterrupt(h);
      if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
         break;
   }

   BVSB_3520_P_EnableHostInterrupt(h, true);
   
#if 0   
   if (retCode == BERR_TIMEOUT)
   {
         BVSB_3520_P_Handle *p3520 = (BVSB_3520_P_Handle *)(h->pImpl);      
         BVSB_ApStatus apstatus;
         unsigned char h_ie[2];
         
         BVSB_3520_P_GetApStatus(h, &apstatus);
         BREG_I2C_Read(p3520->hRegister, h->settings.i2c.chipAddr, BCM3520_SH_AP_SFR_H_IE1, h_ie, 2);
         
         printf("BVSB_3520_P_WaitForEvent() timeout, %08X, %02X, %02X\n", apstatus, h_ie[0], h_ie[1]);
   }
#endif   
   return retCode;

}

/******************************************************************************
 GetChipRevision()
******************************************************************************/
BERR_Code BVSB_3520_P_GetChipRevision(
   BVSB_Handle h,             /* [in] BVSB PI Handle */
   uint8_t *revision          /* [out] revision id of the chip */
)
{
	BERR_Code retCode;
	uint32_t val;

	BVSB_CHK_RETCODE(BVSB_3520_P_ReadMbox( h, BCM3520_TM_CHIP_ID, &val ));
	*revision = (uint8_t)(val & 0xFF);

done:
	return retCode;
	  
}

/******************************************************************************
 SetInbandIfFreq()
******************************************************************************/
BERR_Code BVSB_3520_P_SetInbandIfFreq(
	BVSB_Handle h,
	uint32_t If_freq_hz
	)
{
    uint8_t buf[4];

	buf[0] = (uint8_t)(If_freq_hz >> 24);
	buf[1] = (uint8_t)(If_freq_hz >> 16);
	buf[2] = (uint8_t)(If_freq_hz >> 8);
	buf[3] = (uint8_t)(If_freq_hz & 0xFF);

	return BVSB_3520_P_WriteConfig( h, BVSB_CONFIG_INBAND_IF_FREQ, buf, 4);
}

/******************************************************************************
   SetOobInterfaceControl()
******************************************************************************/
BERR_Code BVSB_3520_P_SetOobInterfaceControl(
   BVSB_Handle  h,    /* [in] BVSB handle */
   BVSB_OobClockSuppression clockSuppression,
   BVSB_OobClock clock
)
{
    BERR_Code retCode;
    uint8_t pin_ctrl = 0;
    uint8_t hab[3];

    BVSB_CHK_RETCODE(BVSB_3520_P_ReadConfig(h, BVSB_CONFIG_PIN_CTRL_5, &pin_ctrl, 1));
    pin_ctrl &= 0x3F;

    if (clockSuppression == BVSB_OobClockSuppression_eSuppressed)
        pin_ctrl |= 0x80;

    if (clock == BVSB_OobClock_eInverted)
        pin_ctrl |= 0x40;

    /* pad_control_1 */
    hab[0] = 0x23;
    hab[1] = pin_ctrl;
    hab[2] = 0;

    BVSB_CHK_RETCODE(BVSB_3520_P_SendHabCommand(h, hab, 3, hab, 1, true));

    BVSB_CHK_RETCODE(BVSB_3520_P_ReadConfig(h, BVSB_CONFIG_PIN_CTRL_5, &pin_ctrl, 1));
done:
   return retCode;
}




									
