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
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g2_priv.h"

BDBG_MODULE(bast_g2_priv_diseqc);


/* local routines */
BERR_Code BAST_g2_P_CalibrateDcOffset(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_CalibrateDcOffset1(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_ResetVsense(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_SendDiseqcCommand1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_Diseqc_UpdateSettings(BAST_ChannelHandle h);


/******************************************************************************
 BAST_g2_P_ResetDiseqc() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   static const uint32_t script_diseqc_init[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_DS_MISC_CONTROL, 0x26047f80), /* ADC format is 2's complement, DAC format is offset binary */
      BAST_SCRIPT_WRITE(BCHP_SDS_DS_SAR_CONTROL, 0x00500030),  /* 2's comp format for SAR ADC */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMADR, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FFC0FF6),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FF70FF8),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FF90FFB),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FFC0FFE),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00000002),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00030003),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00030001),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FFF0FFC),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FF70FF2),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FED0FE7),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FE00FDA),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FD50FD0),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FCC0FCA),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FCA0FCB),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FCF0FD5),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FDE0FE8),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x0FF40002),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00110020),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00300040),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x004F005D),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00690073),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x007A007F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCMEMDAT, 0x00800000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSFIRCTL,  0x00000158),

      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL14, 0x31),
      BAST_SCRIPT_WRITE(BCHP_SDS_DDIO,  0x25100000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TPDIR, 0xFF4b008F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL10, 0x81), /* power down diseqc rx by default to save power */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL1, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL5, 0x13),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL3, 0x09), /* previously 0x0B  in 7313 */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL2, 0x10),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL11, 0x1B),
      BAST_SCRIPT_WRITE(BCHP_SDS_RXBT, 0x08340384),
      BAST_SCRIPT_WRITE(BCHP_SDS_RERT, 0x1770660D),
      BAST_SCRIPT_WRITE(BCHP_SDS_TPWC, 0x3C210A15),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCT, 0x08000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_RXRT, 0x3C033450),
      BAST_SCRIPT_WRITE(BCHP_SDS_SLEW, 0x06060600),
      BAST_SCRIPT_WRITE(BCHP_SDS_ADCTL, 0x02000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_RTDC1, 0x0F110000),
      BAST_SCRIPT_WRITE(BCHP_SDS_RTDC2, 0x00000704),
      BAST_SCRIPT_WRITE(BCHP_SDS_CICC, 0x031F4100),
      BAST_SCRIPT_WRITE(BCHP_SDS_FCIC, 0x00410307),
      BAST_SCRIPT_WRITE(BCHP_SDS_SCIC, 0x00410307),
      BAST_SCRIPT_WRITE(BCHP_SDS_TCTL, 0x1FD40010),
      BAST_SCRIPT_WRITE(BCHP_SDS_RBDT, 0x012C0320),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL4, 0x7F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL4, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL2, 0x73),
      BAST_SCRIPT_EXIT
   };

   BSTD_UNUSED(options);

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_ResetDiseqc);

   /* diseqc initialization */
   BAST_g2_P_ProcessScript_isrsafe(h, script_diseqc_init);

   /* disable vsense interrupts for reset */
   BAST_g2_P_EnableVsenseInterrupts(h, false);

   /* configure diseqc settings */
   BAST_g2_P_SetDiseqcVoltage(h, false);
   BAST_g2_P_SetDiseqcTone(h, false);
   BAST_g2_P_Diseqc_UpdateSettings(h);

   /* clear diseqc busy flag */
   hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess;

   /* power up phy for vsense functionality */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA_MISC, ~0x2080, 0x00);

   /* initialize vsense by setting thresholds */
   BDBG_MSG(("BAST_g2_P_ResetDiseqc: hi=%02X, lo=%02X\n", hChn->diseqcVsenseThreshHi, hChn->diseqcVsenseThreshLo));
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_THRSH, ~0xFFFF,
      (hChn->diseqcVsenseThreshHi << 8) | hChn->diseqcVsenseThreshLo);

   /* calibrate vsense dc offset */
   retCode = BAST_g2_P_CalibrateDcOffset(h);

   BDBG_LEAVE(BAST_g2_P_ResetDiseqc);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_CalibrateDcOffset() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_CalibrateDcOffset(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* disable vsense interrupts for calibration */
   BINT_DisableCallback(hChn->hDiseqcOverVoltageCb);
   BINT_DisableCallback(hChn->hDiseqcUnderVoltageCb);

   /* set lpf alpha for vsense calibration */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0xF00, 0x500);

   if (h->channel == 0)
   {
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x00, 0x08000000);  /* disable input switch for rx input voltage */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_0, ~0x700, 0xB00);      /* select bandgap, 2's comp ADC format */
   }
   else
   {
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x00, 0x08000000);
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_0, ~0x700, 0xB00);
   }

   /* kick start the SAR adc (self-clearing) */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0x00, 0x01);

   /* wait for filter value to converge */
   retCode = BAST_g2_P_EnableTimer(h, BAST_TIMER_DISEQC, 50, BAST_g2_P_CalibrateDcOffset1);

   return retCode;
}


/******************************************************************************
 BAST_g2_P_CalibrateDcOffset1() - ISR context
******************************************************************************/
BERR_Code BAST_g2_P_CalibrateDcOffset1(BAST_ChannelHandle h)
{
   uint32_t mb;

   /* configure DC offset */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DS_SAR_DATA_OUT, &mb);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_DC_OFFSET, &mb);

   if (h->channel == 0)
   {
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_0, ~0x700, 0x00);       /* deselect bandgap */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x08000000, 0x00);  /* enable input switch for rx input voltage */
   }
   else
   {
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_0, ~0x700, 0x00);
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x08000000, 0x00);
   }

   /* set lpf alpha for vsense operation */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0xF00, 0x0B00);

   /* soft reset after calibration */
   return BAST_g2_P_ResetVsense(h);
}


/******************************************************************************
 BAST_g2_P_ResetVsense() - ISR context
******************************************************************************/
BERR_Code BAST_g2_P_ResetVsense(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   /* reset lpf integrator, then clear bits */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0x00, 0x2000);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0x2000, 0x00);

   /* kick start the SAR adc (self-clearing) */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0x00, 0x01);

   /* reset max/min values, then clear bits */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0x00, 0xC0);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_CONTROL, ~0xC0, 0x00);

   /* re-enable interrupts if previously on */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DS_SAR_THRSH, &mb);
   if ((mb & 0x10000) == 0x10000)
   {
      BINT_EnableCallback_isr(hChn->hDiseqcOverVoltageCb);
      BINT_EnableCallback_isr(hChn->hDiseqcUnderVoltageCb);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_DisableDiseqcInterrupts_isr() - ISR context
******************************************************************************/
void BAST_g2_P_DisableDiseqcInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hDiseqcDoneCb);
   BINT_DisableCallback_isr(hChn->hDiseqcOverVoltageCb);
   BINT_DisableCallback_isr(hChn->hDiseqcUnderVoltageCb);

   BINT_ClearCallback_isr(hChn->hDiseqcDoneCb);
   BINT_ClearCallback_isr(hChn->hDiseqcOverVoltageCb);
   BINT_ClearCallback_isr(hChn->hDiseqcUnderVoltageCb);
}


/******************************************************************************
 BAST_g2_P_SetDiseqcTone() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_SetDiseqcTone);
   hChn->bDiseqcToneOn = bTone;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL1, &mb);
   if (bTone)
      mb |= 0x30; /* tone on */
   else
      mb &= ~0x30; /* tone off */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL1, &mb);

   BDBG_LEAVE(BAST_g2_P_SetDiseqcTone);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_GetDiseqcTone() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t adctl, rtdc2, dsctl14, mb;

#ifndef BAST_DISEQC_TONE_DETECT_LEGACY
   static const uint32_t script_diseqc_detect_tone[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_ADCTL, 0x06000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_RTDC2, 0x0000060F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL14, 0x3F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL4, 0x7F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSCTL4, 0x00),
      BAST_SCRIPT_OR(BCHP_SDS_DSCTL2, 0x03),
      BAST_SCRIPT_EXIT
   };
#endif

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
      return BAST_ERR_DISEQC_BUSY;

   BDBG_ENTER(BAST_g2_P_GetDiseqcTone);

   /* enable diseqc rx for tone detect */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL10, ~0x80, 0x00);
   BKNI_Sleep(45);   /* use bkni sleep because task level function */

   /* save registers we are going to modify */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_ADCTL, &adctl);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_RTDC2, &rtdc2);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL14, &dsctl14);

#ifdef BAST_DISEQC_TONE_DETECT_LEGACY
   /* DIRECT AMPLITUDE COMPARISON method */
   /* set diseqc to legacy mode and use abs value of ADC output */
   mb = 0x08;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL14, &mb);

   /* bypass FIR filter */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSFIRCTL, ~0x00, 0x1000);

   /* set diseqc ADC input to 1060mV full scale */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_ADCTL, ~0xF000000, 0x00);

   /* set thresholds for 300mV */
   mb = 0x48;     /* (300mV)/(1060mV/2^8) = 72 = 0x48 */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL12, &mb);
   mb = 0x24;     /* set this lower than DSCTL12 for hysterisis */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL13, &mb);

   /* clear diseqc status and enable tone detection */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL6, ~0x00, 0xC0);

   /* reset integrators and diseqc */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL4, ~0x00, 0x7F);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL4, ~0xFF, 0x00);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL2, ~0x00, 0x03);
   BKNI_Sleep(5);

   /* check if tone present */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DST2, &mb);
   if (mb & 0x01)
      *bTone = true;
   else
      *bTone = false;

   /* clear diseqc status and disable tone detection */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL6, ~0x40, 0x80);

   /* re-enable FIR filter */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSFIRCTL, ~0x1000, 0x00);
#else
   /* ENERGY DETECT method */
   BAST_g2_P_ProcessScript_isrsafe(h, script_diseqc_detect_tone);
   BKNI_Sleep(5);    /* use bkni sleep because task level function */

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_RTDC2, &mb);
   mb = mb >> 23;
   if (mb > hChn->diseqcToneThreshold)
      *bTone = true;
   else
      *bTone = false;
#endif

   /* restore registers */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_ADCTL, &adctl);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_RTDC2, &rtdc2);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL14, &dsctl14);

   /* disable diseqc rx after tone detect */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL10, ~0x00, 0x80);

   /* reset diseqc and packet pointers */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL2, ~0x00, 0x03);

   /* calibrate vsense dc offset */
   retCode = BAST_g2_P_CalibrateDcOffset(h);

   BDBG_LEAVE(BAST_g2_P_GetDiseqcTone);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_SetDiseqcVoltage() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_SetDiseqcVoltage);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL2, &mb);
   if (bVtop)
      mb |= 0x04; /* vtop */
   else
      mb &= ~0x04; /* vbot */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL2, &mb);

   BDBG_LEAVE(BAST_g2_P_SetDiseqcVoltage);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_GetDiseqcVoltage() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_GetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t *pVoltage     /* [out] voltage estimation */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
      return BAST_ERR_DISEQC_BUSY;

   BDBG_ENTER(BAST_g2_P_GetDiseqcVoltage);

   /* read ADC output */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DS_SAR_DATA_OUT, &mb);
   BDBG_MSG(("SDS_DS_SAR_DATA_OUT = %08X", mb));
   *pVoltage = (uint8_t)(mb & 0xFF);

   /* re-calibrate vsense dc offset (offset may change due to temperature drift) */
   retCode = BAST_g2_P_CalibrateDcOffset(h);

   BDBG_LEAVE(BAST_g2_P_GetDiseqcVoltage);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_EnableDiseqcLnb() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_EnableDiseqcLnb(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bEnable           /* [in] true = LNB on, false = LNB off */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_EnableDiseqcLnb);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL2, &mb);
   if (bEnable)
      mb |= 0x10; /* LNB on */
   else
      mb &= ~0x10; /* LNB off */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL2, &mb);

   BDBG_LEAVE(BAST_g2_P_EnableDiseqcLnb);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_EnableVsenseInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_EnableVsenseInterrupts(BAST_ChannelHandle h, bool bEnable)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_EnableVsenseInterrupts);

   if (bEnable)
   {
      /* BDBG_MSG(("ENABLE vsense\n")); */

      /* enable status output */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_THRSH, 0xFFFFFFFF, 0x10000);

      /* enable the diseqc transaction done interrupt */
      retCode = BINT_ClearCallback(hChn->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_ClearCallback(hChn->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);

      retCode = BINT_EnableCallback(hChn->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_EnableCallback(hChn->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   else
   {
      /* BDBG_MSG(("DISABLE vsense\n")); */

      /* disable status output */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DS_SAR_THRSH, ~0x10000, 0x00);

      /* disable the diseqc transaction done interrupt */
      retCode = BINT_ClearCallback(hChn->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_ClearCallback(hChn->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);

      retCode = BINT_DisableCallback(hChn->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_DisableCallback(hChn->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }

   BDBG_LEAVE(BAST_g2_P_EnableVsenseInterrupts);
   return retCode;
}

/******************************************************************************
 BAST_g2_P_GetDiseqcVsenseEventHandle() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h,
   BKNI_EventHandle *hDiseqcOverVoltageEvent,
   BKNI_EventHandle *hDiseqcUnderVoltageEvent)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   *hDiseqcOverVoltageEvent = ((BAST_g2_P_ChannelHandle *)h->pImpl)->hDiseqcOverVoltageEvent;
   *hDiseqcUnderVoltageEvent = ((BAST_g2_P_ChannelHandle *)h->pImpl)->hDiseqcUnderVoltageEvent;
   return retCode;
}


/******************************************************************************
 BAST_g2_P_DiseqcOverVoltage_isr()
******************************************************************************/
void BAST_g2_P_DiseqcOverVoltage_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(param);

   /* BDBG_MSG(("Diseqc%d voltage ABOVE upper threshold(%02X)", h->channel, hChn->diseqcVsenseThreshHi)); */
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eDiseqcVoltageGtHiThresh);

   retCode = BINT_ClearCallback_isr(hChn->hDiseqcOverVoltageCb);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   BKNI_SetEvent(hChn->hDiseqcOverVoltageEvent);
}


/******************************************************************************
 BAST_g2_P_DiseqcUnderVoltage_isr()
******************************************************************************/
void BAST_g2_P_DiseqcUnderVoltage_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(param);

   /* BDBG_MSG(("Diseqc%d voltage BELOW lower threshold(%02X)", h->channel, hChn->diseqcVsenseThreshLo)); */
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eDiseqcVoltageLtLoThresh);

   retCode = BINT_ClearCallback_isr(hChn->hDiseqcUnderVoltageCb);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   BKNI_SetEvent(hChn->hDiseqcUnderVoltageEvent);
}


/******************************************************************************
 BAST_g2_P_SendACW() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_SendACW(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint8_t            acw /* [in] auto control word */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(acw);

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   /* TBD... */
   return retCode;
}


/******************************************************************************
 BAST_g2_P_SendDiseqcCommand() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_SendDiseqcCommand);

   /* reset statuses */
   hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eBusy;
   hChn->diseqcStatus.nReplyBytes = 0;

   /* update settings based on diseqc ctl parameter */
   retCode = BAST_g2_P_Diseqc_UpdateSettings(h);

   /* copy diseqc message to channel handle */
   hChn->diseqcSendLen = sendBufLen;
   for (i = 0; i < sendBufLen; i++)
      hChn->diseqcSendBuffer[i] = pSendBuf[i];

   /* delay since h/w does not insert idle time for tone off case */
   /* protect enable timer from interrupts since SDS_BCKTMR shared */
   if ((hChn->bDiseqcToneOn == false) && hChn->diseqcPreTxDelay)
      retCode = BAST_g2_P_EnableTimer(h, BAST_TIMER_DISEQC, hChn->diseqcPreTxDelay * 1000, BAST_g2_P_SendDiseqcCommand1_isr);
   else
      retCode = BAST_g2_P_EnableTimer(h, BAST_TIMER_DISEQC, 50, BAST_g2_P_SendDiseqcCommand1_isr);

   BDBG_LEAVE(BAST_g2_P_SendDiseqcCommand);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_SendDiseqcCommand1_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g2_P_SendDiseqcCommand1_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t i, mb, dsctl1, dsec_clk, lval;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   /* clear diseqc block */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL1, &dsctl1);
   dsctl1 |= 0x01;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL1, &dsctl1);
   dsctl1 &= ~0x01;

   /* don't expect reply by default */
   hChn->diseqcStatus.bReplyExpected = false;

   if (hChn->diseqcSendLen)
   {
      if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_EXP_REPLY_DISABLE)
      {
         /* don't expect reply byte */
         dsctl1 &= ~0x08;
         dsctl1 |= 0x04;
      }
      else
      {
         /* reply expectation depends on framing byte */
         dsctl1 &= ~0x04;
         if (hChn->diseqcSendBuffer[0] & 0x02)
         {
            /* power on diseqc rx */
            BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL10, ~0x80, 0x00);
            hChn->diseqcStatus.bReplyExpected = true;
         }
      }

      for (i = 0; i < hChn->diseqcSendLen; i++)
      {
         mb = hChn->diseqcSendBuffer[i];
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCMD, &mb);
      }
   }
   else
   {
      if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_EXP_REPLY_DISABLE)
      {
         /* don't expect reply byte */
         dsctl1 &= ~0x08;
         dsctl1 |= 0x04;
      }
      else
      {
         /* receive only mode */
         hChn->diseqcStatus.bReplyExpected = true;
         dsctl1 |= 0x0C;

         /* power on diseqc rx */
         BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL10, ~0x80, 0x00);
      }
   }

   /* get the diseqc clock divider */
   BAST_g2_P_GetDiseqcClock_isrsafe(h);
   dsec_clk = hChn->diseqcClk;

   /* adjust 22KHz fcw DTCT[23:0] */
   BMTH_HILO_32TO64_Mul(16777216 * 2, 22587, &P_hi, &P_lo);  /* scaling factor 2^24 = 16777216 */ /* 22000 * 1.026667 = 22587 */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, dsec_clk, &Q_hi, &Q_lo);  /* divide by dsec_clk last to preserve precision */
   lval = Q_lo;
   mb = ((lval + 1) >> 1) & 0x00FFFFFF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DTCT, &mb);

   /* adjust rx bit time RBDT[11:0] */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_RBDT, &mb);
   mb &= 0xFFFFF000;	/* clear bits 11:0 */
   lval = ((hChn->diseqcBitThreshold & 0xFFF) * 2 * dsec_clk) / 1000000;
   mb |= ((lval + 1) >> 1) & 0x00000FFF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_RBDT, &mb);

   /* adjust tone absent time TPWC[31:24] */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_TPWC, &mb);
   mb &= 0x00FFFFFF;	/* clear bits 31:24 */
   lval = (0x3C * 2 * dsec_clk) / 1000000;
   mb |= (((lval + 1) >> 1) & 0x000000FF) << 24;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_TPWC, &mb);

   /* adjust RXRT[31:24] */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_RXRT, &mb);
   mb &= 0x00FFFFFF;	/* clear bits 31:24 */
   mb |= (((lval + 1) >> 1) & 0x000000FF) << 24;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_RXRT, &mb);

   /* adjust quiet 15msec Q15T[31:16] */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_Q15T, &mb);
   mb &= 0x0000FFFF;	/* clear bits 31:16 */
   lval = (hChn->diseqcPreTxDelay * 20 * dsec_clk) / 10000;
   mb |= (((lval + 1) >> 1) & 0x0000FFFF) << 16;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_Q15T, &mb);

   /* clear and enable the diseqc transaction done interrupt */
   BINT_ClearCallback_isr(hChn->hDiseqcDoneCb);
   BINT_EnableCallback_isr(hChn->hDiseqcDoneCb);

   /* start the transaction */
   dsctl1 |= 0x02;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL1, &dsctl1);

   return retCode;
}


/******************************************************************************
 BAST_g2_P_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_g2_P_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus  *pStatus /* [out] status of last transaction */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_GetDiseqcStatus);

   *pStatus = hChn->diseqcStatus;

   BDBG_LEAVE(BAST_g2_P_GetDiseqcStatus);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_g2_P_GetDiseqcEventHandle(BAST_ChannelHandle h, BKNI_EventHandle *hDiseqcEvent)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (((BAST_g2_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g2_P_GetDiseqcEventHandle);

   *hDiseqcEvent = ((BAST_g2_P_ChannelHandle *)h->pImpl)->hDiseqcEvent;

   BDBG_LEAVE(BAST_g2_P_GetDiseqcEventHandle);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_Diseqc_DoneIsr()
******************************************************************************/
void BAST_g2_P_DiseqcDone_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t i, mb, dst1;

   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eDiseqcDone);
   BINT_ClearCallback_isr(hChn->hDiseqcDoneCb);
   BINT_DisableCallback_isr(hChn->hDiseqcDoneCb);
   hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess; /* clear diseqc busy flag */

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DST1, &dst1);
   if (dst1 & 0x01000000)
   {
      /* error detected */
      if (dst1 & 0x1000)
         hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eRxOverflow;
      else if (dst1 & 0x0800)
         hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eRxReplyTimeout;
      else if (dst1 & 0x0400)
         hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eRxParityError;
   }

   /* get size of diseqc reply */
   hChn->diseqcStatus.nReplyBytes = (dst1 >> 19) & 0x1F;

   /* limit size of reply to maximum buffer size */
   if (hChn->diseqcStatus.nReplyBytes > DISEQC_REPLY_BUF_SIZE)
      hChn->diseqcStatus.nReplyBytes = DISEQC_REPLY_BUF_SIZE;

   if (hChn->diseqcStatus.nReplyBytes > 0)
   {
      for (i = 0; i < hChn->diseqcStatus.nReplyBytes; i++)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSRPLY, &mb);
         hChn->diseqcStatus.replyBuffer[i] = (uint8_t)(mb & 0xFF);
      }
   }

   /* reset the FIFO, memory, noise integrator, etc. */
   mb = 0x7F;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL4, &mb);
   mb = 0x00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL4, &mb);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL2, ~0x00, 0x22);
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL1, ~0x00, 0x01);

   /* power off diseqc rx */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSCTL10, ~0x00, 0x80);

   /* set diseqc event */
   BKNI_SetEvent(hChn->hDiseqcEvent);
}


/******************************************************************************
* BAST_g2_P_Diseqc_UpdateSettings()
******************************************************************************/
BERR_Code BAST_g2_P_Diseqc_UpdateSettings(BAST_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   /* set envelope or tone mode */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DDIO, &mb);
   mb &= ~0x700000;
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_ENVELOPE)
      mb |= 0x100000;
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_LNBPU_TXEN)
      mb |= 0x10000000;
   else
      mb &= ~0x10000000;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DDIO, &mb);

   /* tone align enable or disable */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL6, &mb);
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_TONE_ALIGN_ENABLE)
      mb |= 0x08;
   else
      mb &= ~0x08;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL6, &mb);

   /* receive reply timeout enable or disable */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL7, &mb);
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_RRTO_DISABLE)
      mb |= 0x40;
   else
      mb &= ~0x40;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL7, &mb);

   /* receive reply timeout setting */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_RXRT, &mb);
   mb &= 0xFFF00000;
   mb |= (hChn->diseqcRRTOuSec & 0x000FFFFF);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_RXRT, &mb);

   /* burst enable or disable */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DSCTL1, &mb);
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_TONEBURST_ENABLE)
      mb |= 0x40;
   else
      mb &= ~0x40;
   /* tone A or tone B */
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_TONEBURST_B)
      mb |= 0x80;
   else
      mb &= ~0x80;
   /* expect reply enable or disable */
   if (hChn->diseqcCtl & BAST_G2_DISEQC_CTL_EXP_REPLY_DISABLE)
      mb |= 0x04;
   else
      mb &= ~0x04;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DSCTL1, &mb);

   return retCode;
}

#if 0
/******************************************************************************
 BAST_g2_P_Diseqc_ReadReply()
******************************************************************************/
BERR_Code BAST_g2_P_Diseqc_ReadReply(BAST_ChannelHandle h, uint8_t *pRcvBuf, uint8_t *pRcvBufLen, BAST_DiseqcStatus *pStatus)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint8_t i;

   *pStatus = hChn->diseqcStatus;
   *pRcvBufLen = 0;
   if (hChn->diseqcStatus.bReplyExpected)
   {
      *pRcvBufLen = hChn->diseqcStatus.nReplyBytes;
      for (i = 0; i < hChn->diseqcStatus.nReplyBytes; i++)
         pRcvBuf[i] = hChn->diseqcStatus.replyBuffer[i];
   }
   return BERR_SUCCESS;
}
#endif
