/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
/* #define BAST_BCM3445_NEW_CONFIG */
#include "bstd.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g2_priv.h"

BDBG_MODULE(bast_g2_priv_lna);

#define BAST_MI2C(x) \
while (1) { \
   retCode = x; \
   if (retCode != BAST_ERR_MI2C_BUSY) \
      break; \
} \
if (retCode != BERR_SUCCESS) goto done;


/* local functions */
BERR_Code BAST_g2_P_ReadBcm3445(BAST_ChannelHandle h, uint8_t reg, uint8_t *val);
BERR_Code BAST_g2_P_WriteBcm3445(BAST_ChannelHandle h, uint8_t reg, uint8_t val);


/******************************************************************************
 BAST_g2_P_WriteBcm3445()
******************************************************************************/
BERR_Code BAST_g2_P_WriteBcm3445(BAST_ChannelHandle hChn, uint8_t reg, uint8_t val)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(hChn->pDevice->pImpl);
   uint8_t buf[2];

   buf[0] = reg;
   buf[1] = val;
   return BAST_g2_P_WriteMi2c(hChn, hDev->bcm3445Address, buf, 2);
}


/******************************************************************************
 BAST_g2_P_ReadBcm3445()
******************************************************************************/
BERR_Code BAST_g2_P_ReadBcm3445(BAST_ChannelHandle hChn, uint8_t reg, uint8_t *val)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(hChn->pDevice->pImpl);
   uint8_t buf[2];

   buf[0] = reg;
   return BAST_g2_P_ReadMi2c(hChn, hDev->bcm3445Address, buf, 1, val, 1);
}


/******************************************************************************
 BAST_g2_P_ConfigLna()
******************************************************************************/
BERR_Code BAST_g2_P_ConfigLna(
   BAST_Handle    h,             /* [in] BAST handle */
   BAST_LnaIntConfig int_config, /* [in] BCM3445 internal switch configuration */
   BAST_LnaExtConfig ext_config  /* [in] BCM3445 output mapping to SDS channels */
)
{
#ifndef BAST_EXCLUDE_OLD_LNA
   BERR_Code retCode;
   BAST_g2_P_Handle *hDev = h->pImpl;
   int which_channel = (int)(hDev->bcm3445Mi2cChannel);
   BAST_Bcm3445Settings settings;
   BAST_Bcm3445OutputChannel out;
   
   BAST_ChannelHandle hChn0 = h->pChannels[0];
   BAST_ChannelHandle hChn1 = h->pChannels[1];

   if (which_channel == 0)
      settings.mi2c = BAST_Mi2cChannel_e0;
   else if (which_channel == 1)
      settings.mi2c = BAST_Mi2cChannel_e1;
   else
      settings.mi2c = BAST_Mi2cChannel_e2;
   
   switch (int_config)
   {
      case BAST_LnaIntConfig_eIn1Out1_In1Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn1Vga;
         break;
      case BAST_LnaIntConfig_eIn1Out1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eIn1DbOut1_In2Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn1Db;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
         break;
      case BAST_LnaIntConfig_eOff:
         settings.out1 = BAST_Bcm3445OutputConfig_eOff;
         settings.out2 = BAST_Bcm3445OutputConfig_eOff;
         settings.daisy = BAST_Bcm3445OutputConfig_eOff;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In1Out2:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn1Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eIn1Vga;
         break;
      case BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff:
         settings.out1 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
         settings.daisy = BAST_Bcm3445OutputConfig_eOff;
         break;
      default:
         retCode = BERR_NOT_SUPPORTED;
         
         exit_error:
         return BERR_TRACE(retCode);
   }
   retCode = BAST_g2_P_ConfigBcm3445(h, &settings);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   /* map tuner 0 input */
   if ((ext_config == BAST_LnaExtConfig_eOut1Sds0_Out2Sds1) || (ext_config == BAST_LnaExtConfig_eOut1Sds0_DaisySds1))
      out = BAST_Bcm3445OutputChannel_eOut1;
   else if (ext_config == BAST_LnaExtConfig_eOut1Sds1_Out2Sds0)
      out = BAST_Bcm3445OutputChannel_eOut2;
   else
   {
      retCode = BERR_INVALID_PARAMETER;
      goto exit_error;
   }
   retCode = BAST_g2_P_MapBcm3445ToTuner(hChn0, settings.mi2c, out);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   /* map tuner 1 input */
   if (ext_config == BAST_LnaExtConfig_eOut1Sds0_Out2Sds1)
      out = BAST_Bcm3445OutputChannel_eOut2;
   else if (ext_config == BAST_LnaExtConfig_eOut1Sds0_DaisySds1)
      out = BAST_Bcm3445OutputChannel_eDaisy;
   else if (ext_config == BAST_LnaExtConfig_eOut1Sds1_Out2Sds0)
      out = BAST_Bcm3445OutputChannel_eOut1;
   retCode = BAST_g2_P_MapBcm3445ToTuner(hChn1, settings.mi2c, out);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BAST_GetLnaStatus()
******************************************************************************/
BERR_Code BAST_g2_P_GetLnaStatus(
   BAST_Handle    h,        /* [in] BAST handle */
   BAST_LnaStatus *pStatus  /* [out] chip-specific status struct */
)
{
#ifndef BAST_EXCLUDE_OLD_LNA
   BERR_Code retCode;
   BAST_Bcm3445Status chnStatus;
   BAST_Bcm3445OutputConfig out0, out1;
   BAST_Bcm3445OutputChannel tuner0, tuner1;
   BAST_ChannelHandle hChn0 = h->pChannels[0];
   BAST_ChannelHandle hChn1 = h->pChannels[1];
   
   /* call new implementation of get status for tuner 0 */
   retCode = BAST_g2_P_GetBcm3445Status(hChn0, &chnStatus);
   if (retCode != BERR_SUCCESS)
   {
      exit_error:
      return BERR_TRACE(retCode);
   }
   
   pStatus->version = chnStatus.version;
   tuner0 = chnStatus.tuner_input;
   out0 = chnStatus.out_cfg;
   pStatus->agc0 = chnStatus.agc;
   
   /* call new implementation of get status for tuner 1 */
   retCode = BAST_g2_P_GetBcm3445Status(hChn1, &chnStatus);
   if (retCode != BERR_SUCCESS)
      goto exit_error;

   tuner1 = chnStatus.tuner_input;
   out1 = chnStatus.out_cfg;
   pStatus->agc1 = chnStatus.agc;
   
   BDBG_MSG(("BAST_g2_P_GetLnaStatus(): tuner0=%d, out0=%d, tuner1=%d, out1=%d", tuner0, out0, tuner1, out1));
   
   /* decode tuner mapping */
   if ((tuner0 == BAST_Bcm3445OutputChannel_eOut1) && (tuner1 == BAST_Bcm3445OutputChannel_eOut2))
      pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds0_Out2Sds1;
   else if ((tuner0 == BAST_Bcm3445OutputChannel_eOut1) && (tuner1 == BAST_Bcm3445OutputChannel_eDaisy))
      pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds0_DaisySds1;
   else
      pStatus->ext_config = BAST_LnaExtConfig_eOut1Sds1_Out2Sds0;

   /* decode lna mapping */
   if (out0 == BAST_Bcm3445OutputConfig_eOff)
      pStatus->int_config = BAST_LnaIntConfig_eOff;
   else if (out0 == BAST_Bcm3445OutputConfig_eIn1Vga)
   {
      if (out1 == BAST_Bcm3445OutputConfig_eIn1Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn1Out1_In1Out2;
      else /* out1 == BAST_Bcm3445OutputConfig_eIn2Vga */
         pStatus->int_config = BAST_LnaIntConfig_eIn1Out1_In2Out2;
   }
   else if (out0 == BAST_Bcm3445OutputConfig_eIn1Db)
      pStatus->int_config = BAST_LnaIntConfig_eIn1DbOut1_In2Out2;
   else
   {
      /* out0 = BAST_Bcm3445OutputConfig_eIn2Vga */
      if (out1 == BAST_Bcm3445OutputConfig_eIn1Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In1Out2;
      else if (out1 == BAST_Bcm3445OutputConfig_eIn2Vga)
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In2Out2;
      else
         pStatus->int_config = BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff;
   }

   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BAST_g2_P_LnaSetNotchFilter() - return BAST_ERR_MI2C_BUSY if waiting for mi2c - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_LnaSetNotchFilter(BAST_ChannelHandle h, bool bEnable)
{
   BAST_Handle hAst = h->pDevice;
   BAST_ChannelHandle hI2cChn;
   BAST_g2_P_ChannelHandle *hI2cChnImpl;
   BAST_g2_P_ChannelHandle *hMyChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;   
   uint8_t pd2_ctrl2, data0;
   
   /* no notch required for external tuner */
   if (hMyChn->bExternalTuner)
      return BERR_SUCCESS;
   
   if ((int)(hMyChn->bcm3445Mi2cChannel) != (int)(h->channel))
   {
      /* the BCM3445 we're trying to control is on the other channel's mi2c */
      hI2cChn = hAst->pChannels[(int)(hMyChn->bcm3445Mi2cChannel)];
   }
   else
      hI2cChn = h;
   hI2cChnImpl = (BAST_g2_P_ChannelHandle *)(hI2cChn->pImpl);
   
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
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hI2cChn, 0x11, &pd2_ctrl2));
   pd2_ctrl2 &= 0x07;
   pd2_ctrl2 |= 0x10;
   if (bEnable)
      pd2_ctrl2 |= data0;
   else
      pd2_ctrl2 &= ~data0;
   BAST_MI2C(BAST_g2_P_WriteBcm3445(hI2cChn, 0x11, pd2_ctrl2));
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_g2_P_ConfigBcm3445() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)h->pImpl;
   BAST_ChannelHandle hChn;
   BAST_g2_P_ChannelHandle *hChnImpl;
   uint8_t ob_ctrl1, vga_ctrl1, db_ctrl;
   uint8_t bandgap_ctrl, ob_ctrl2, vga_amp2_ctrl, pd_ctrl1;
   BERR_Code retCode;

   /* check for invalid configurations */
   BDBG_ASSERT(pSettings);
   BDBG_ENTER(BAST_g2_P_ConfigBcm3445);
   
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
   hChnImpl = (BAST_g2_P_ChannelHandle *)(hChn->pImpl);
   hChnImpl->bcm3445Settings = *pSettings;
   hChnImpl->bcm3445Status = 0;
   
   /* check for mi2c busy */
   if (hChnImpl->bMi2cInProgress)
      return BERR_TRACE(BAST_ERR_MI2C_BUSY);
   
   /* reset 3445 */
   BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x17, 0x80));
   BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x17, 0x00));

   /* read driver settings for configuration */
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x0C, &ob_ctrl1));
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x09, &vga_ctrl1));
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x0B, &db_ctrl));

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
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x09, vga_ctrl1));
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0B, db_ctrl));
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0C, ob_ctrl1));
      
      /* power off bandgap if all elements off */
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x17, &bandgap_ctrl));
      if (ob_ctrl1 == 0xFF)
         bandgap_ctrl |= 0x40;
      else
         bandgap_ctrl &= ~0x40;
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x17, bandgap_ctrl)); /* bandgap_ctrl1 */
      
      /* power down unused inputs */
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x18, &bandgap_ctrl));
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
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x18, bandgap_ctrl)); /* bandgap_ctrl2 */
      
      /* toggle low power settings */
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x0D, &ob_ctrl2));
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hChn, 0x1E, &vga_amp2_ctrl));
      if (hChnImpl->bcm3445Ctl & BAST_G2_BCM3445_CTL_LOW_POWER_MODE)
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
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0D, ob_ctrl2));
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x1E, vga_amp2_ctrl));
      
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x09, vga_ctrl1));
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0B, db_ctrl));
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0C, ob_ctrl1));
      
      /* for better IP3 setting, per Dave Chang email */
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x19, 0xC9)); /* VGA1_OS_CTRL */
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x1A, 0x90)); /* VGA2_OS_CTRL */
      
      /* AGC takeover point setting 5dB move up */
      switch (hChnImpl->bcm3445Ctl & BAST_G2_BCM3445_CTL_AGC_TOP)
      {
         case BAST_G2_BCM3445_CTL_AGC_TOP_LOW:
            pd_ctrl1 = 0x42; /* -39dBm backoff @ 2150MHz, -35 dBm backoff @ 1000 MHz */
            break;
         case BAST_G2_BCM3445_CTL_AGC_TOP_HI:
            pd_ctrl1 = 0x82; /* -29dBm backoff @ 2150MHz, -25 dBm backoff @ 1000 MHz */
            break;
         case BAST_G2_BCM3445_CTL_AGC_TOP_MID:
         default:
            pd_ctrl1 = 0x02; /* -34dBm backoff @ 2150MHz, -30 dBm backoff @ 1000 MHz */
      }
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x0E, pd_ctrl1)); /* PD1_CTRL1 */
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hChn, 0x10, pd_ctrl1)); /* PD2_CTRL1 */
      
      hDev->bBcm3445 = true;
      hChnImpl->bcm3445Status = BAST_BCM3445_STATUS_INITIALIZED;
   }
   else
      hDev->bBcm3445 = false; 
   
   done:
   BDBG_LEAVE(BAST_g2_P_ConfigBcm3445);
   return retCode;      
}


/******************************************************************************
 BAST_g2_P_MapBcm3445ToTuner()
******************************************************************************/
BERR_Code BAST_g2_P_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
)
{
   BAST_g2_P_ChannelHandle *hMyChn = (BAST_g2_P_ChannelHandle *)(h->pImpl);
   
   BDBG_ENTER(BAST_g2_P_MapBcm3445ToTuner);
   
   /* check maximum i2c channel */
   if (mi2c >= BAST_G2_MAX_CHANNELS)
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   
   hMyChn->bcm3445Mi2cChannel = mi2c;
   hMyChn->bcm3445TunerInput = out;
   
   BDBG_LEAVE(BAST_g2_P_MapBcm3445ToTuner);
   return BERR_SUCCESS;  
}


/******************************************************************************
 BAST_g2_P_GetBcm3445Status() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
)
{
   BAST_g2_P_ChannelHandle *hMyChn = (BAST_g2_P_ChannelHandle *)(h->pImpl);
   BAST_Handle hAst = h->pDevice;
   BAST_ChannelHandle hI2cChn = hAst->pChannels[(int)(hMyChn->bcm3445Mi2cChannel)];
   BAST_g2_P_ChannelHandle *hI2cChnImpl = hI2cChn->pImpl;   
   BERR_Code retCode;
   uint8_t val8, agc_dac_ctrl1_addr;
   
   BDBG_ENTER(BAST_g2_P_GetBcm3445Status);

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
 
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hI2cChn, 0x02, &(pStatus->version)));
 
   BAST_MI2C(BAST_g2_P_ReadBcm3445(hI2cChn, ((pStatus->out_cfg == BAST_Bcm3445OutputConfig_eIn2Vga) ? 1 : 0), &val8));
   if (val8 & 0x80)
   {
      agc_dac_ctrl1_addr = ((pStatus->out_cfg == BAST_Bcm3445OutputConfig_eIn2Vga) ? 0x15 : 0x13);
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hI2cChn, agc_dac_ctrl1_addr, 0xC0));   /* AGCx_DAC_CTRL1 */
      BAST_MI2C(BAST_g2_P_WriteBcm3445(hI2cChn, agc_dac_ctrl1_addr+1, 0x80)); /* AGCx_DAC_CTRL2 */
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hI2cChn, agc_dac_ctrl1_addr, &val8));
      BAST_MI2C(BAST_g2_P_ReadBcm3445(hI2cChn, agc_dac_ctrl1_addr, &(pStatus->agc))); /* need to read twice */      
      /* BDBG_MSG(("BCM3445 AGC is 0x%X", pStatus->agc)); */
      pStatus->agc &= 0x3F;      
   }
   else
   {
      /* BDBG_MSG(("max power")); */
      pStatus->agc = 0x3F; /* maximum power level */
   }

   done:
   BDBG_LEAVE(BAST_g2_P_GetBcm3445Status);
   return retCode;  
}

