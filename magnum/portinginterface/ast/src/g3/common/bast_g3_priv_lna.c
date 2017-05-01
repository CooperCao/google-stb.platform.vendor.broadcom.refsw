/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/
/* #define BAST_BCM3445_NEW_CONFIG */
#include "bstd.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

#ifndef BAST_EXCLUDE_BCM3445

#ifdef BAST_EXCLUDE_MI2C
#error "BCM3440 cannot be supported because BAST_EXCLUDE_MI2C is defined"
#endif

BDBG_MODULE(bast_g3_priv_lna);

#define BAST_MI2C_ASSERT(x) \
   if ((retCode = x) != BERR_SUCCESS) goto done;


/******************************************************************************
 BAST_g3_P_WriteBcm3445()
******************************************************************************/
static BERR_Code BAST_g3_P_WriteBcm3445(BAST_ChannelHandle hChn, uint8_t reg, uint8_t val)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(hChn->pDevice->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i, buf[2];

   buf[0] = reg;
   buf[1] = val;

   for (i = 0; i < 3; i++)
   {
      retCode = BAST_g3_P_WriteMi2c(hChn, hDev->bcm3445Address, buf, 2);
      if (retCode == BERR_SUCCESS) break;
      BKNI_Sleep(5); /* wait before retry */
      BDBG_WRN(("retry mi2c write"));
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_ReadBcm3445()
******************************************************************************/
static BERR_Code BAST_g3_P_ReadBcm3445(BAST_ChannelHandle hChn, uint8_t reg, uint8_t *val)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(hChn->pDevice->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i, buf[2];

   buf[0] = reg;

   for (i = 0; i < 3; i++)
   {
      retCode = BAST_g3_P_ReadMi2c(hChn, hDev->bcm3445Address, buf, 1, val, 1);
      if (retCode == BERR_SUCCESS) break;
      BKNI_Sleep(5); /* wait before retry */
      BDBG_WRN(("retry mi2c read"));
   }
   return retCode;
}


#if 0
/******************************************************************************
 BAST_g3_P_LnaSetNotchFilter() - return BAST_ERR_MI2C_BUSY if waiting for mi2c - Non-ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_LnaSetNotchFilter(BAST_ChannelHandle h, bool bEnable)
{
   BAST_Handle hAst = h->pDevice;
   BAST_ChannelHandle hI2cChn;
   BAST_g3_P_ChannelHandle *hI2cChnImpl;
   BAST_g3_P_ChannelHandle *hMyChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint8_t pd2_ctrl2, data0;

   /* no notch required for external tuner */
#ifndef BAST_EXCLUDE_EXT_TUNER
   if (hMyChn->bExternalTuner)
      return BERR_SUCCESS;
#endif

   if ((int)(hMyChn->bcm3445Mi2cChannel) != (int)(h->channel))
   {
      /* the BCM3445 we're trying to control is on the other channel's mi2c */
      hI2cChn = hAst->pChannels[(int)(hMyChn->bcm3445Mi2cChannel)];
   }
   else
      hI2cChn = h;
   hI2cChnImpl = (BAST_g3_P_ChannelHandle *)(hI2cChn->pImpl);

   /* check for mi2c busy */
   if (hI2cChnImpl->bMi2cInProgress)
      return BERR_TRACE(BAST_ERR_MI2C_BUSY);

   data0 = 0;  /* no notch enabled by default */
   if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eDaisy)
      data0 = 0x04; /* Daisy Notch */
   else if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eOut2)
      data0 = 0x02; /* OUT2 Notch */
   else if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eOut1)
      data0 = 0x01; /* OUT1 Notch */

   /* set notch filters */
   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hI2cChn, 0x11, &pd2_ctrl2));
   pd2_ctrl2 &= 0x07;
   pd2_ctrl2 |= 0x10;
   if (bEnable)
      pd2_ctrl2 |= data0;
   else
      pd2_ctrl2 &= ~data0;
   BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hI2cChn, 0x11, pd2_ctrl2));

   done:
   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_ConfigBcm3445() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)h->pImpl;
   BAST_ChannelHandle hChn;
   BAST_g3_P_ChannelHandle *hChnImpl;
   uint8_t ob_ctrl1, vga_ctrl1, db_ctrl;
   uint8_t bandgap_ctrl, ob_ctrl2, vga_amp2_ctrl, pd_ctrl1;
   BERR_Code retCode;

   /* check for invalid configurations */
   BDBG_ASSERT(pSettings);
   BDBG_ENTER(BAST_g3_P_ConfigBcm3445);

   /* check maximum i2c channel */
   if (pSettings->mi2c >= h->totalChannels)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   /* daisy cannot loop back to itself */
   if (pSettings->daisy == BAST_Bcm3445OutputConfig_eDaisy)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   /* input driver 1 and daisy driver 2 cannot be enabled simultaneously */
   if (pSettings->out1 == BAST_Bcm3445OutputConfig_eIn1Db)
   {
      if ((pSettings->out2 == BAST_Bcm3445OutputConfig_eIn1Vga) || (pSettings->daisy == BAST_Bcm3445OutputConfig_eIn1Vga))
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   else if (pSettings->out2 == BAST_Bcm3445OutputConfig_eIn1Db)
   {
      if ((pSettings->out1 == BAST_Bcm3445OutputConfig_eIn1Vga) || (pSettings->daisy == BAST_Bcm3445OutputConfig_eIn1Vga))
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   else if (pSettings->daisy == BAST_Bcm3445OutputConfig_eIn1Db)
   {
      if ((pSettings->out1 == BAST_Bcm3445OutputConfig_eIn1Vga) || (pSettings->out2 == BAST_Bcm3445OutputConfig_eIn1Vga))
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* set up handles */
   hChn = (BAST_ChannelHandle)(h->pChannels[(int)(pSettings->mi2c)]);
   hChnImpl = (BAST_g3_P_ChannelHandle *)(hChn->pImpl);
   hChnImpl->bcm3445Settings = *pSettings;
   hChnImpl->bcm3445Status = 0;

   /* check for mi2c busy */
   if (hChnImpl->bMi2cInProgress)
      return BERR_TRACE(BAST_ERR_MI2C_BUSY);

   /* reset 3445 */
   BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x17, 0x80));
   BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x17, 0x00));

   /* read driver settings for configuration */
   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x0C, &ob_ctrl1));
   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x09, &vga_ctrl1));
   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x0B, &db_ctrl));

   /* configure out1 path */
   ob_ctrl1 &= 0xF8;    /* enable output drivers 1, 2, 3 as basis */
   switch (pSettings->out1)
   {
      case BAST_Bcm3445OutputConfig_eOff:
         ob_ctrl1 |= 0x07;    /* disable output drivers 1, 2, 3 */
         break;
      case BAST_Bcm3445OutputConfig_eDaisy:
         ob_ctrl1 |= 0x03;    /* disable output driver 1 and 2 */
         db_ctrl &= 0xBF;     /* enable daisy driver 1 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Vga:
         ob_ctrl1 |= 0x06;    /* disable output driver 2 and 3 */
         vga_ctrl1 &= 0x7F;   /* enable input driver 1 */
         db_ctrl |= 0x80;     /* disable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Db:
         ob_ctrl1 |= 0x06;    /* disable output driver 2 and 3 */
         vga_ctrl1 |= 0x80;   /* disable input driver 1 */
         db_ctrl &= 0x7F;     /* enable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn2Vga:
         ob_ctrl1 |= 0x05;    /* disable output driver 1 and 3 */
         vga_ctrl1 &= 0xBF;   /* enable input driver 2 */
         break;
      default:
         hChnImpl->bcm3445Status |= BAST_BCM3445_STATUS_ERR_INVALID_CFG;
   }

   /* configure out2 path */
   ob_ctrl1 &= 0xC7;   /* enable output drivers 4, 5, 6 as basis */
   switch (pSettings->out2)
   {
      case BAST_Bcm3445OutputConfig_eOff:
         ob_ctrl1 |= 0x38;    /* disable output drivers 4, 5, 6 */
         break;
      case BAST_Bcm3445OutputConfig_eDaisy:
         ob_ctrl1 |= 0x18;    /* disable output driver 4 and 5 */
         db_ctrl &= 0xBF;     /* enable daisy driver 1 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Vga:
         ob_ctrl1 |= 0x30;    /* disable output driver 5 and 6 */
         vga_ctrl1 &= 0x7F;   /* enable input driver 1 */
         db_ctrl |= 0x80;     /* disable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Db:
         ob_ctrl1 |= 0x30;    /* disable output driver 5 and 6 */
         vga_ctrl1 |= 0x80;   /* disable input driver 1 */
         db_ctrl &= 0x7F;     /* enable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn2Vga:
         ob_ctrl1 |= 0x28;    /* disable output driver 4 and 6 */
         vga_ctrl1 &= 0xBF;   /* enable input driver 2 */
         break;
      default:
         hChnImpl->bcm3445Status |= BAST_BCM3445_STATUS_ERR_INVALID_CFG;
   }

   /* configure daisy path */
   ob_ctrl1 &= 0x3F;   /*enable output drivers 7, 8 as basis */
   switch (pSettings->daisy)
   {
      case BAST_Bcm3445OutputConfig_eOff:
         ob_ctrl1 |= 0xC0;    /* disable output drivers 7, 8 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Vga:
         ob_ctrl1 |= 0x40;    /* disable driver 7 */
         vga_ctrl1 &= 0x7F;   /* enable input driver 1 */
         db_ctrl |= 0x80;     /* disable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn1Db:
         ob_ctrl1 |= 0x40;    /* disable driver 7 */
         vga_ctrl1 |= 0x80;   /* disable input driver 1 */
         db_ctrl &= 0x7F;     /* enable daisy driver 2 */
         break;
      case BAST_Bcm3445OutputConfig_eIn2Vga:
         ob_ctrl1 |= 0x80;    /* disable driver 8 */
         vga_ctrl1 &= 0xBF;   /* enable input driver 2 */
         break;
      default:
         hChnImpl->bcm3445Status |= BAST_BCM3445_STATUS_ERR_INVALID_CFG;
   }

   /* check for errors */
   if ((hChnImpl->bcm3445Status & BAST_BCM3445_STATUS_ERROR) == 0)
   {
      /* configure crossbar before power optimizations to avoid glitches */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x09, vga_ctrl1));
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0B, db_ctrl));
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0C, ob_ctrl1));

      /* power off bandgap if all elements off */
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x17, &bandgap_ctrl));
      if (ob_ctrl1 == 0xFF)
         bandgap_ctrl |= 0x40;
      else
         bandgap_ctrl &= ~0x40;
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x17, bandgap_ctrl)); /* bandgap_ctrl1 */

      /* power down unused inputs */
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x18, &bandgap_ctrl));
      if ((ob_ctrl1 & 0x89) == 0x89)   /* check if input driver 1 and daisy driver 2 are used */
      {
         db_ctrl |= 0x80;        /* disable daisy driver 2 */
         vga_ctrl1 |= 0x80;      /* disable input driver 1 */
         bandgap_ctrl |= 0x48;   /* power down input buffer 1 and input amp 1 */
      }
      if ((ob_ctrl1 & 0x52) == 0x52)   /* check if input driver 2 is used */
      {
         vga_ctrl1 |= 0x40;      /* disable input driver 2 */
         bandgap_ctrl |= 0xA0;   /* power down input buffer 2 and input amp 2 */
      }
      if ((ob_ctrl1 & 0x24) == 0x24)   /* check if daisy driver 1 is used */
         db_ctrl |= 0x40;        /* disable daisy driver 1 */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x18, bandgap_ctrl)); /* bandgap_ctrl2 */

      /* toggle low power settings */
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x0D, &ob_ctrl2));
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hChn, 0x1E, &vga_amp2_ctrl));
      if (hChnImpl->bcm3445Ctl & BAST_G3_BCM3445_CTL_LOW_POWER_MODE)
      {
         vga_ctrl1 = (vga_ctrl1 & ~0x3F) | 0x03;   /* high gain amp bias 27.5mA (vga_amp_i_ctrl=b'011), input buffer bias 6.4mA (vga_buf_i_ctrl=b'000) */
         ob_ctrl2 = (ob_ctrl2 & ~0xE0) | 0x40;     /* output driver bias 30mA (ob_i_ctrl=b'010) */
         vga_amp2_ctrl &= ~0x70;                   /* low gain amp bias 13.7mA (vga_amp2_i_ctrl=b'000) */
      }
      else
      {
         vga_ctrl1 = (vga_ctrl1 & ~0x3F) | 0x04 | 0x38;  /* high gain amp bias 32mA(vga_amp_i_ctrl=b'100), input buffer bias 20.4mA (vga_buf_i_ctrl=b'111) */
         ob_ctrl2 = (ob_ctrl2 & ~0xE0) | 0xC0;           /* output driver bias 50mA(ob_i_ctrl=b'110) */
         vga_amp2_ctrl = (vga_amp2_ctrl & ~0x70) | 0x30; /* low gain amp bias 27.5mA(vga_amp2_i_ctrl=b'011) */
      }
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0D, ob_ctrl2));
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x1E, vga_amp2_ctrl));

      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x09, vga_ctrl1));
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0B, db_ctrl));
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0C, ob_ctrl1));

      /* for better IP3 setting, per Dave Chang email */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x19, 0xC9)); /* VGA1_OS_CTRL */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x1A, 0x90)); /* VGA2_OS_CTRL */

      /* AGC takeover point setting 5dB move up */
      switch (hChnImpl->bcm3445Ctl & BAST_G3_BCM3445_CTL_AGC_TOP)
      {
         case BAST_G3_BCM3445_CTL_AGC_TOP_LOW:
            pd_ctrl1 = 0x42; /* -39dBm backoff @ 2150MHz, -35 dBm backoff @ 1000 MHz */
            break;
         case BAST_G3_BCM3445_CTL_AGC_TOP_HI:
            pd_ctrl1 = 0x82; /* -29dBm backoff @ 2150MHz, -25 dBm backoff @ 1000 MHz */
            break;
         case BAST_G3_BCM3445_CTL_AGC_TOP_MID:
         default:
            pd_ctrl1 = 0x02; /* -34dBm backoff @ 2150MHz, -30 dBm backoff @ 1000 MHz */
      }
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x0E, pd_ctrl1)); /* PD1_CTRL1 */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hChn, 0x10, pd_ctrl1)); /* PD2_CTRL1 */
      hDev->bBcm3445 = true;
      hChnImpl->bcm3445Status = BAST_BCM3445_STATUS_INITIALIZED;
   }
   else
      hDev->bBcm3445 = false;

   done:
   BDBG_LEAVE(BAST_g3_P_ConfigBcm3445);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_MapBcm3445ToTuner()
******************************************************************************/
BERR_Code BAST_g3_P_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
)
{
   BAST_g3_P_ChannelHandle *hMyChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);

   BDBG_ENTER(BAST_g3_P_MapBcm3445ToTuner);

   /* check maximum i2c channel */
   if (mi2c >= BAST_G3_MAX_CHANNELS)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   hMyChn->bcm3445Mi2cChannel = mi2c;
   hMyChn->bcm3445TunerInput = out;

   BDBG_LEAVE(BAST_g3_P_MapBcm3445ToTuner);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetBcm3445Status() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
)
{
   BAST_g3_P_ChannelHandle *hMyChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BAST_Handle hAst = h->pDevice;
   BAST_ChannelHandle hI2cChn = hAst->pChannels[(int)(hMyChn->bcm3445Mi2cChannel)];
   BAST_g3_P_ChannelHandle *hI2cChnImpl = hI2cChn->pImpl;
   BERR_Code retCode;
   uint8_t val8, agc_dac_ctrl1_addr;

   BDBG_ENTER(BAST_g3_P_GetBcm3445Status);

   /* retrieve tuner mapping s*/
   pStatus->mi2c = hMyChn->bcm3445Mi2cChannel;
   pStatus->tuner_input = hMyChn->bcm3445TunerInput;
   pStatus->status = hI2cChnImpl->bcm3445Status;

   /* retrieve input mappings */
   if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eOut1)
      pStatus->out_cfg = hI2cChnImpl->bcm3445Settings.out1;
   else if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eOut2)
      pStatus->out_cfg = hI2cChnImpl->bcm3445Settings.out2;
   else if (hMyChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eDaisy)
      pStatus->out_cfg = hI2cChnImpl->bcm3445Settings.daisy;
   else
      pStatus->out_cfg = BAST_Bcm3445OutputConfig_eOff;

   /* check for mi2c busy */
   if (hI2cChnImpl->bMi2cInProgress)
      return BERR_TRACE(BAST_ERR_MI2C_BUSY);

   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hI2cChn, 0x02, &(pStatus->version)));

   BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hI2cChn, ((pStatus->out_cfg == BAST_Bcm3445OutputConfig_eIn2Vga) ? 1 : 0), &val8));
   if (val8 & 0x80)
   {
      agc_dac_ctrl1_addr = ((pStatus->out_cfg == BAST_Bcm3445OutputConfig_eIn2Vga) ? 0x15 : 0x13);
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hI2cChn, agc_dac_ctrl1_addr, 0xC0));   /* AGCx_DAC_CTRL1 */
      BAST_MI2C_ASSERT(BAST_g3_P_WriteBcm3445(hI2cChn, agc_dac_ctrl1_addr+1, 0x80)); /* AGCx_DAC_CTRL2 */
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hI2cChn, agc_dac_ctrl1_addr, &val8));
      BAST_MI2C_ASSERT(BAST_g3_P_ReadBcm3445(hI2cChn, agc_dac_ctrl1_addr, &(pStatus->agc))); /* need to read twice */
      /* BDBG_MSG(("BCM3445 AGC is 0x%X", pStatus->agc)); */
      pStatus->agc &= 0x3F;
   }
   else
   {
      /* BDBG_MSG(("max power")); */
      pStatus->agc = 0x3F; /* maximum power level */
   }

   done:
   BDBG_LEAVE(BAST_g3_P_GetBcm3445Status);
   return retCode;
}

#endif

