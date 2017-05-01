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
#include "bstd.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"
#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
#include "bchp_ftm_phy_ana.h"
#endif

BDBG_MODULE(bast_g3_priv_diseqc);


/* local routines */
static BERR_Code BAST_g3_P_CalibrateDcOffset(BAST_ChannelHandle h);
static BERR_Code BAST_g3_P_CalibrateDcOffset1_isr(BAST_ChannelHandle h);
static BERR_Code BAST_g3_P_ResetVsense_isr(BAST_ChannelHandle h);
static BERR_Code BAST_g3_P_SendDiseqcCommand1_isr(BAST_ChannelHandle h);
static BERR_Code BAST_g3_P_Diseqc_UpdateSettings(BAST_ChannelHandle h);


/******************************************************************************
 BAST_g3_P_DiseqcPowerUp() - power up the diseqc core
******************************************************************************/
BERR_Code BAST_g3_P_DiseqcPowerUp(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (!hChn->bHasDiseqc)
      return BERR_NOT_SUPPORTED;

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x2);  /* power up adc */
   BAST_g3_P_PowerUpFskphy_isrsafe(h);   /* power up fskphy */

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DiseqcPowerDown() - power down the diseqc core
******************************************************************************/
BERR_Code BAST_g3_P_DiseqcPowerDown(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#ifndef BAST_EXCLUDE_FTM
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
#endif

   if (!hChn->bHasDiseqc)
      return BERR_NOT_SUPPORTED;

   /* power down adc */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x2);

   /* power off diseqc rx */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL02, 0x00800000);

#ifndef BAST_EXCLUDE_FTM
   /* power down fskphy if ftm off */
   if (hDev->bFtmPoweredDown)
#endif
   {
      BAST_g3_P_PowerDownFskphy_isrsafe(h);
   }

   return BERR_SUCCESS;
}


/* #define BAST_DISEQC_ANALOG_MODE */
/******************************************************************************
 BAST_g3_P_ResetDiseqc() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
)
{
#ifdef BAST_DISEQC_ANALOG_MODE
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(h->pDevice->pImpl);
#endif
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   static const uint32_t script_diseqc_init[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DS_MISC_CONTROL, 0x260C7F80), /* ADC and DAC format is offset binary */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DS_SAR_CONTROL, 0x00500020),  /* offset binary format for SAR ADC */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000000), /*Initialize DS_COMMON_CONTROL[6]  (for 4528/4550 only */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSRST, 0x00000001), /* flush unknown caused by DS_COMMON_CONTROL[6]  (for 4528/4550 only) */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSRST, 0x00000000), /* flush unknown caused by DS_COMMON_CONTROL[6]  (for 4528/4550 only) */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000003), /* Reset FSKPHY  */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000000), /* Reset FSKPHY  */
#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
      BAST_SCRIPT_WRITE(BCHP_FTM_PHY_ANA_0_0, 0x03007753), /* Setup FSKPHY  */
      BAST_SCRIPT_WRITE(BCHP_FTM_PHY_ANA_0_1, 0x00000000), /* Setup FSKPHY  */
#endif
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMADR, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FFC0FF6),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FF70FF8),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FF90FFB),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FFC0FFE),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00000002),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00030003),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00030001),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FFF0FFC),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FF70FF2),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FED0FE7),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FE00FDA),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FD50FD0),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FCC0FCA),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FCA0FCB),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FCF0FD5),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FDE0FE8),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x0FF40002),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00110020),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00300040),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x004F005D),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00690073),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x007A007F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCMEMDAT, 0x00800000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSFIRCTL,  0x00000158),

      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DDIO, 0x25100000),       /* set diseqc i/o muxing */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCTL00, 0x00091000),    /* enable VCNT, add tone to dac1, LNB power up gpio */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCTL01, 0x00000013),    /* set ddfs gain 630mV */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCTL02, 0x1B810000),    /* set dac1 trim to 0V, power down diseqc rx by default to save power */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCTL03, 0x00314060),    /* noise estimator decimate by 8, slow CIC, differential mode output */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RXBT, 0x08340384),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RERT, 0x1770660D),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_TPWC, 0x3C200A15), /* decrease tone absent timing, period count to 0x20 */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCT, 0x08000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RXRT, 0x3C033450),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_SLEW, 0x06060600),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_ADCTL, 0x02000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RTDC1, 0x7F810000),      /* hi and lo clip thresholds */
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RTDC2, 0x00000704),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_CICC, 0x031F4100),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_FCIC, 0x00410307),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_SCIC, 0x00410307),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_TCTL, 0x1FD40010),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RBDT, 0x012C0320),
      BAST_SCRIPT_OR(BCHP_SDS_DSEC_DSCTL00, 0x7F006301),       /* diseqc resets, freeze integrators, clear status */
      BAST_SCRIPT_AND(BCHP_SDS_DSEC_DSCTL00, ~0x7F006301),     /* release resets, unfreeze integrators, unclear status */
      BAST_SCRIPT_EXIT
   };

   BSTD_UNUSED(options);

   if (((BAST_g3_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g3_P_ResetDiseqc);

   /* diseqc initialization */
   hChn->coresPoweredDown &= ~BAST_CORE_DISEQC;
   BAST_g3_P_DiseqcPowerUp(h);
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_diseqc_init));

   /* disable vsense interrupts for reset */
   BAST_g3_P_EnableVsenseInterrupts(h, false);

   /* clear and disable diseqc done interrupt */
   BINT_ClearCallback(hChn->diseqc->hDiseqcDoneCb);
   BINT_DisableCallback(hChn->diseqc->hDiseqcDoneCb);

   /* configure diseqc settings */
   BAST_g3_P_SetDiseqcVoltage(h, false);
   BAST_g3_P_SetDiseqcTone(h, false);
   BAST_g3_P_Diseqc_UpdateSettings(h);

   /* clear diseqc busy flag */
   hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess;
#ifndef BAST_EXCLUDE_ACW
   hChn->diseqc->diseqcAcw = 0;
#endif

   /* power up phy for vsense and rx functionality */
   BAST_g3_P_PowerUpFskphy_isrsafe(h);
   if (h->channel == 0)
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x00780000, 0x00700000);
   else
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x00780000, 0x00700000);

   if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEcho)
   {
      /* route selvtop signal to dsec_vctl pin */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DDIO, ~0x07000000, 0x04000000);
   }

#ifdef BAST_DISEQC_ANALOG_MODE
   /* selecting diseqc path to fskphy dac essentially powers down ftm */
   hDev->bFtmPoweredDown = true;
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_FTM_PHY_ANA_MISC, 0x00000010);    /* analog DiSEqC */
#else
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_FTM_PHY_ANA_MISC, ~0x00000010);  /* digital DiSEqC */
#endif

   /* initialize vsense by setting thresholds */
   BDBG_MSG(("BAST_g3_P_ResetDiseqc: hi=%02X, lo=%02X", hChn->diseqc->dsecSettings.vsenseThresholdHi, hChn->diseqc->dsecSettings.vsenseThresholdLo));
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_THRSH, ~0xFFFF,
      (hChn->diseqc->dsecSettings.vsenseThresholdHi << 8) | hChn->diseqc->dsecSettings.vsenseThresholdLo);

   /* calibrate vsense dc offset */
   retCode = BAST_g3_P_CalibrateDcOffset(h);

   done:
   BDBG_LEAVE(BAST_g3_P_ResetDiseqc);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_CalibrateDcOffset() - Non-ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_CalibrateDcOffset(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* disable vsense interrupts for calibration */
   BINT_DisableCallback(hChn->diseqc->hDiseqcOverVoltageCb);
   BINT_DisableCallback(hChn->diseqc->hDiseqcUnderVoltageCb);

   /* set lpf alpha for vsense calibration */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xF00, 0x500);

#if 0
   /* TBD */
   if (h->channel == 0)
   {
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x00, 0x08000000);  /* disable input switch for rx input voltage */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_0, ~0x700, 0xB00);      /* select bandgap, 2's comp ADC format */
   }
   else
   {
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x00, 0x08000000);
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_0, ~0x700, 0xB00);
   }
#endif

   /* kick start the SAR adc (self-clearing) */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0x01);

   /* wait for filter value to converge */
   retCode = BAST_g3_P_EnableTimer(h, BAST_TimerSelect_eDiseqc2, 50, BAST_g3_P_CalibrateDcOffset1_isr);

   return retCode;
}


/******************************************************************************
 BAST_g3_P_CalibrateDcOffset1_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_CalibrateDcOffset1_isr(BAST_ChannelHandle h)
{
   uint32_t mb;


   /* configure DC offset */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DATA_OUT, &mb);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DC_OFFSET, &mb);

#if 0
   /* TBD */
   if (h->channel == 0)
   {
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_0, ~0x700, 0x00);       /* deselect bandgap */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x08000000, 0x00);  /* enable input switch for rx input voltage */
   }
   else
   {
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_0, ~0x700, 0x00);
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x08000000, 0x00);
   }
#endif

   /* set lpf alpha for vsense operation */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xF00, 0x0B00);

   /* soft reset after calibration */
   return BAST_g3_P_ResetVsense_isr(h);
}


/******************************************************************************
 BAST_g3_P_ResetVsense_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_ResetVsense_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   /* configure adc for vsense */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000028);   /* disable diseqc input, select adc for vsense */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00002400);  /* vsense mode 3MHz */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000080, 0x00000040);   /* start vsense */

   /* reset lpf integrator, then clear bits */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0x2000);
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x2000, 0x00);

   /* kick start the SAR adc (self-clearing) */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0x01);

   /* reset max/min values, then clear bits */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0xC0);
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0xC0, 0x00);

   /* re-enable interrupts if previously on */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_THRSH, &mb);
   if ((mb & 0x10000) == 0x10000)
   {
      BINT_EnableCallback_isr(hChn->diseqc->hDiseqcOverVoltageCb);
      BINT_EnableCallback_isr(hChn->diseqc->hDiseqcUnderVoltageCb);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableDiseqcInterrupts_isr() - ISR context
******************************************************************************/
void BAST_g3_P_DisableDiseqcInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eDiseqc1);
   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eDiseqc2);

   BINT_DisableCallback_isr(hChn->diseqc->hDiseqcDoneCb);
   BINT_DisableCallback_isr(hChn->diseqc->hDiseqcOverVoltageCb);
   BINT_DisableCallback_isr(hChn->diseqc->hDiseqcUnderVoltageCb);

   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcDoneCb);
   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcOverVoltageCb);
   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcUnderVoltageCb);
}


/******************************************************************************
 BAST_g3_P_SetDiseqcTone() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   BDBG_ENTER(BAST_g3_P_SetDiseqcTone);
   hChn->diseqc->bDiseqcToneOn = bTone;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);
   if (bTone)
      mb |= 0x30; /* tone on */
   else
      mb &= ~0x30; /* tone off */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);

   BDBG_LEAVE(BAST_g3_P_SetDiseqcTone);
   return retCode;
}


#define BAST_DISEQC_TONE_DETECT_LEGACY
/******************************************************************************
 BAST_g3_P_GetDiseqcTone() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *bTone            /* [out] true = tone present, false = tone absent */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t adctl, rtdc2, dsctl3, mb;

#ifndef BAST_DISEQC_TONE_DETECT_LEGACY
   static const uint32_t script_diseqc_detect_tone[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_ADCTL, 0x06000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_RTDC2, 0x0000060F),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSEC_DSCTL03, 0x003F4060),    /* noise estimator decimate by 8, slow CIC, soft demod mode for tone detect */
      BAST_SCRIPT_OR(BCHP_SDS_DSEC_DSCTL00, 0x7F000300),       /* diseqc resets, freeze integrators, clear status */
      BAST_SCRIPT_AND(BCHP_SDS_DSEC_DSCTL00, ~0x7F000300),     /* release resets, unfreeze integrators, unclear status */
      BAST_SCRIPT_EXIT
   };
#endif

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   if (hChn->diseqc->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
      return BAST_ERR_DISEQC_BUSY;

   BDBG_ENTER(BAST_g3_P_GetDiseqcTone);

   /* enable diseqc rx for tone detect */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL02, ~0x00800000, 0x00000000);

   /* configure adc for diseqc rx */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000080);   /* stop vsense */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00000900);  /* AFE mode 12MHz */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000028);   /* enable diseqc input, select adc for diseqc rx */
   BKNI_Sleep(45);   /* use bkni sleep because task level function */

   /* save registers we are going to modify */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_ADCTL, &adctl);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_RTDC2, &rtdc2);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL03, &dsctl3);

#ifdef BAST_DISEQC_TONE_DETECT_LEGACY
   /* DIRECT AMPLITUDE COMPARISON method */
   /* set diseqc to legacy mode and use abs value of ADC output */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL03, ~0x00070000, 0x00080000);
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSFIRCTL, 0x00001000);   /* bypass FIR filter */

   /* lower PGA gain control */
   if (h->channel == 0)
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x00780000, 0x00500000);
   else
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x00780000, 0x00500000);

   /* set thresholds for 300mV */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL03, ~0x00FF, 0x0048); /* (300mV)/(1060mV/2^8) = 72 = 0x48 */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL03, ~0xFF00, 0x2400); /* set this lower than DSCTL12 for hysterisis */

   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, 0x00008000);     /* clear tone status */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, 0x00004000);    /* enable tone detection */

   /* reset integrators and diseqc */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x7F000301);
   BKNI_Sleep(5);

   /* check if tone present */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST2, &mb);
   if (mb & 0x01)
      *bTone = true;
   else
      *bTone = false;

   /* restore PGA gain control */
   if (h->channel == 0)
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA0_1, ~0x00780000, 0x00700000);
   else
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_FTM_PHY_ANA1_1, ~0x00780000, 0x00700000);

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, ~0x00004000);   /* disable tone detection */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, 0x00008000);      /* clear tone status */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DSFIRCTL, ~0x00001000);  /* re-enable FIR filter */
#else
   /* ENERGY DETECT method */
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_diseqc_detect_tone));
   BKNI_Sleep(5);    /* use bkni sleep because task level function */

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_RTDC2, &mb);
   mb = mb >> 23;
   if (mb > hChn->diseqc->dsecSettings.toneThreshold)
      *bTone = true;
   else
      *bTone = false;
#endif

   /* restore registers */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_ADCTL, &adctl);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_RTDC2, &rtdc2);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL03, &dsctl3);

   /* reset diseqc and packet pointers */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00000300);

   /* disable diseqc rx after tone detect */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL02, 0x00800000);

   /* calibrate vsense dc offset */
   retCode = BAST_g3_P_CalibrateDcOffset(h);

#ifndef BAST_DISEQC_TONE_DETECT_LEGACY
   done:
#endif
   BDBG_LEAVE(BAST_g3_P_GetDiseqcTone);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SetDiseqcVoltage() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   BDBG_ENTER(BAST_g3_P_SetDiseqcVoltage);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);
   if (bVtop)
      mb |= 0x00100400;    /* vtop */
   else
      mb &= ~0x00100400;   /* vbot */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);

   BDBG_LEAVE(BAST_g3_P_SetDiseqcVoltage);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetDiseqcVoltage() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_GetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t *pVoltage     /* [out] voltage estimation */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   if (hChn->diseqc->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
      return BAST_ERR_DISEQC_BUSY;

   BDBG_ENTER(BAST_g3_P_GetDiseqcVoltage);

   /* read ADC output */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_DATA_OUT, &mb);
   BDBG_MSG(("SDS_DS_SAR_DATA_OUT = %08X", mb));
   *pVoltage = (uint8_t)(mb & 0xFF);

   /* re-calibrate vsense dc offset (offset may change due to temperature drift) */
   retCode = BAST_g3_P_CalibrateDcOffset(h);

   BDBG_LEAVE(BAST_g3_P_GetDiseqcVoltage);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_EnableDiseqcLnb() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_EnableDiseqcLnb(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bEnable           /* [in] true = LNB on, false = LNB off */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   if (((BAST_g3_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g3_P_EnableDiseqcLnb);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);
   if (bEnable)
      mb |= 0x1000; /* LNB on */
   else
      mb &= ~0x1000; /* LNB off */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);

   BDBG_LEAVE(BAST_g3_P_EnableDiseqcLnb);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_EnableVsenseInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_EnableVsenseInterrupts(BAST_ChannelHandle h, bool bEnable)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g3_P_EnableVsenseInterrupts);

   if (bEnable)
   {
      /* BDBG_MSG(("ENABLE vsense\n")); */
	  /* configure adc for vsense */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000028);   /* disable diseqc input, select adc for vsense */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00002400);  /* vsense mode 3MHz */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, ~0x00000080, 0x00000040);   /* start vsense */

      /* reset lpf integrator, then clear bits */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x00, 0x2000);
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_CONTROL, ~0x2000, 0x00);
      /* enable status output */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_THRSH, 0xFFFFFFFF, 0x10000);

      /* enable the diseqc transaction done interrupt */
      retCode = BINT_ClearCallback(hChn->diseqc->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_ClearCallback(hChn->diseqc->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);

      retCode = BINT_EnableCallback(hChn->diseqc->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_EnableCallback(hChn->diseqc->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   else
   {
      /* BDBG_MSG(("DISABLE vsense\n")); */

      /* disable status output */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DS_SAR_THRSH, ~0x10000, 0x00);

      /* disable the diseqc transaction done interrupt */
      retCode = BINT_ClearCallback(hChn->diseqc->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_ClearCallback(hChn->diseqc->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);

      retCode = BINT_DisableCallback(hChn->diseqc->hDiseqcOverVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      retCode = BINT_DisableCallback(hChn->diseqc->hDiseqcUnderVoltageCb);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }

   BDBG_LEAVE(BAST_g3_P_EnableVsenseInterrupts);
   return retCode;
}

/******************************************************************************
 BAST_g3_P_GetDiseqcVsenseEventHandle() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h,
   BKNI_EventHandle *hDiseqcOverVoltageEvent,
   BKNI_EventHandle *hDiseqcUnderVoltageEvent)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (((BAST_g3_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   *hDiseqcOverVoltageEvent = ((BAST_g3_P_ChannelHandle *)h->pImpl)->diseqc->hDiseqcOverVoltageEvent;
   *hDiseqcUnderVoltageEvent = ((BAST_g3_P_ChannelHandle *)h->pImpl)->diseqc->hDiseqcUnderVoltageEvent;
   return retCode;
}


/******************************************************************************
 BAST_g3_P_DiseqcOverVoltage_isr()
******************************************************************************/
void BAST_g3_P_DiseqcOverVoltage_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(param);

   /* BDBG_MSG(("Diseqc%d voltage ABOVE upper threshold(%02X)", h->channel, hChn->diseqc->dsecSettings.vsenseThresholdHi)); */
   BAST_g3_P_IncrementInterruptCounter_isr(h, BAST_g3_IntID_eDiseqcVoltageGtHiThresh);

   retCode = BINT_ClearCallback_isr(hChn->diseqc->hDiseqcOverVoltageCb);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   BKNI_SetEvent(hChn->diseqc->hDiseqcOverVoltageEvent);
}


/******************************************************************************
 BAST_g3_P_DiseqcUnderVoltage_isr()
******************************************************************************/
void BAST_g3_P_DiseqcUnderVoltage_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(param);

   /* BDBG_MSG(("Diseqc%d voltage BELOW lower threshold(%02X)", h->channel, hChn->diseqc->dsecSettings.vsenseThresholdLo)); */
   BAST_g3_P_IncrementInterruptCounter_isr(h, BAST_g3_IntID_eDiseqcVoltageLtLoThresh);

   retCode = BINT_ClearCallback_isr(hChn->diseqc->hDiseqcUnderVoltageCb);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   BKNI_SetEvent(hChn->diseqc->hDiseqcUnderVoltageEvent);
}


#ifndef BAST_EXCLUDE_ACW
/******************************************************************************
 BAST_g3_P_SendACW() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_SendACW(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   uint8_t            acw /* [in] auto control word */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   BDBG_ENTER(BAST_g3_P_SendACW);

   /* clear diseqc status and set send status to busy */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00000001);
   hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eBusy;
   hChn->diseqc->diseqcAcw = acw;

   /* set control word */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, ~0xFF000000, (acw << 24));

   /* enable auto control word tx */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00200000);

   /* start acw transmission */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00008000);

   /* 32ms setup + 8ms start bit + 32ms control + 24ms error correction + 8ms polarity ~ 104ms */
   /* protect enable timer from interrupts since SDS_MISC_TMRCTL shared */
   retCode = BAST_g3_P_EnableTimer(h, BAST_TimerSelect_eDiseqc1, 150000, BAST_g3_P_DoneACW);

   BDBG_LEAVE(BAST_g3_P_SendACW);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_DoneACW() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_DoneACW(
   BAST_ChannelHandle h    /* [in] BAST channel handle */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t dst1;

   /* reset status */
   hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess;

   /* check for cw done indicator */
#if (BCHP_CHIP==4517)
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_INTR2_CPU_STATUS, &dst1);
   /* BDBG_MSG(("DST1=%08X", dst1)); */
   if ((dst1 & 0x20) == 0)
      hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eAcwTimeout;
#else
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &dst1);
   /* BDBG_MSG(("DST1=%08X", dst1)); */
   if ((dst1 & 0x10000000) == 0)
      hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eAcwTimeout;
#endif

   /* check polarity bit of acw */
   if (hChn->diseqc->diseqcAcw & 0x80)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00100000);    /* finish with vtop */
   else
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, ~0x00100000);  /* finish with vbot */

   /* revert to non-ACW tx */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, ~0x00200000);

   /* reset control word and state machine */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00006000);

   /* set diseqc event */
   BKNI_SetEvent(hChn->diseqc->hDiseqcEvent);

   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_SendDiseqcCommand() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   if (hChn->coresPoweredDown & BAST_CORE_DISEQC)
      return BAST_ERR_POWERED_DOWN;

   BDBG_ENTER(BAST_g3_P_SendDiseqcCommand);

   /* reset statuses */
   hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eBusy;
   hChn->diseqc->diseqcStatus.nReplyBytes = 0;

   /* update settings based on diseqc ctl parameter */
   retCode = BAST_g3_P_Diseqc_UpdateSettings(h);

   /* copy diseqc message to channel handle */
   hChn->diseqc->diseqcSendCount = 0;
   hChn->diseqc->diseqcSendLen = sendBufLen;
   for (i = 0; i < sendBufLen; i++)
      hChn->diseqc->diseqcSendBuffer[i] = pSendBuf[i];

   /* delay since h/w does not insert idle time for tone off case */
   /* protect enable timer from interrupts since SDS_MISC_TMRCTL shared */
   if ((hChn->diseqc->bDiseqcToneOn == false) && hChn->diseqc->dsecSettings.preTxDelay)
      retCode = BAST_g3_P_EnableTimer(h, BAST_TimerSelect_eDiseqc1, hChn->diseqc->dsecSettings.preTxDelay * 1000, BAST_g3_P_SendDiseqcCommand1_isr);
   else
      retCode = BAST_g3_P_EnableTimer(h, BAST_TimerSelect_eDiseqc1, 50, BAST_g3_P_SendDiseqcCommand1_isr);

   BDBG_LEAVE(BAST_g3_P_SendDiseqcCommand);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SendDiseqcCommand1_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_SendDiseqcCommand1_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, mb, dsctl;
   uint8_t nbytes;

   /* clear diseqc status */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x00000001);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &dsctl);

   /* don't expect reply by default */
   hChn->diseqc->diseqcStatus.bReplyExpected = false;

   if (hChn->diseqc->diseqcSendLen)
   {
      if (hChn->diseqc->dsecSettings.bOverrideFraming)
      {
         if (hChn->diseqc->dsecSettings.bExpectReply)
         {
            /* always expect reply byte */
            hChn->diseqc->diseqcStatus.bReplyExpected = true;
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
         if (hChn->diseqc->diseqcSendBuffer[0] & 0x02)
            hChn->diseqc->diseqcStatus.bReplyExpected = true;
      }

      /* fifo limit is 16 bytes */
      if (hChn->diseqc->diseqcSendLen <= 16)
         nbytes = hChn->diseqc->diseqcSendLen;
      else
         nbytes = 16;

      for (i = 0; i < nbytes; i++)
      {
         mb = hChn->diseqc->diseqcSendBuffer[hChn->diseqc->diseqcSendCount++];
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCMD, &mb);
      }
   }
   else
   {
      if (hChn->diseqc->dsecSettings.bExpectReply)
      {
         /* receive only mode */
         hChn->diseqc->diseqcStatus.bReplyExpected = true;
         dsctl |= 0x0000000C;
      }
      else
      {
         /* don't expect reply byte */
         dsctl &= ~0x00000008;
         dsctl |= 0x00000004;
      }
   }

   if (hChn->diseqc->diseqcStatus.bReplyExpected)
   {
      /* power on diseqc rx if reply expected */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL02, ~0x00800000, 0x00000000);

      /* configure adc for diseqc rx */
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000080);   /* stop vsense */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCGDIV, ~0x0000FF00, 0x00000900);  /* AFE mode 12MHz */
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DS_COMMON_CONTROL, 0x00000028);   /* enable diseqc input, select adc for diseqc rx */
   }

   /* check for extended send */
   if (hChn->diseqc->diseqcSendLen > 16)
   {
      /* set almost empty levels */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, ~0x00070000, 0x00020000);

      /* set up tx fifo almost empty interrupt */
      BINT_ClearCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);
      BINT_EnableCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);
   }

   /* clear and enable the diseqc transaction done interrupt */
   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcDoneCb);
   BINT_EnableCallback_isr(hChn->diseqc->hDiseqcDoneCb);

   /* start the transaction */
   dsctl |= 0x00000002;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &dsctl);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, ~0x00000002);  /* clear start txn */

   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetDiseqcStatus()
******************************************************************************/
BERR_Code BAST_g3_P_GetDiseqcStatus(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_DiseqcStatus  *pStatus /* [out] status of last transaction */
)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g3_P_GetDiseqcStatus);

   *pStatus = hChn->diseqc->diseqcStatus;

   BDBG_LEAVE(BAST_g3_P_GetDiseqcStatus);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetDiseqcEventHandle()
******************************************************************************/
BERR_Code BAST_g3_P_GetDiseqcEventHandle(BAST_ChannelHandle h, BKNI_EventHandle *hDiseqcEvent)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (((BAST_g3_P_ChannelHandle *)h->pImpl)->bHasDiseqc == false)
      return BERR_NOT_SUPPORTED;

   BDBG_ENTER(BAST_g3_P_GetDiseqcEventHandle);

   *hDiseqcEvent = ((BAST_g3_P_ChannelHandle *)h->pImpl)->diseqc->hDiseqcEvent;

   BDBG_LEAVE(BAST_g3_P_GetDiseqcEventHandle);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_DiseqcDone_isr()
******************************************************************************/
void BAST_g3_P_DiseqcDone_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t i, mb, dst1;

   BSTD_UNUSED(param);

   BAST_g3_P_IncrementInterruptCounter_isr(h, BAST_g3_IntID_eDiseqcDone);
   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcDoneCb);
   BINT_DisableCallback_isr(hChn->diseqc->hDiseqcDoneCb);
   hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess; /* clear diseqc busy flag */

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &dst1);
   if (dst1 & 0x01000000)
   {
      /* error detected */
      if (dst1 & 0x1000)
         hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eRxOverflow;
      else if (dst1 & 0x0800)
         hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eRxReplyTimeout;
      else if (dst1 & 0x0400)
         hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eRxParityError;
   }

   /* get size of diseqc reply */
   hChn->diseqc->diseqcStatus.nReplyBytes = (dst1 >> 19) & 0x1F;

   /* limit size of reply to maximum buffer size */
   if (hChn->diseqc->diseqcStatus.nReplyBytes > DISEQC_REPLY_BUF_SIZE)
      hChn->diseqc->diseqcStatus.nReplyBytes = DISEQC_REPLY_BUF_SIZE;

   if (hChn->diseqc->diseqcStatus.nReplyBytes > 0)
   {
      for (i = 0; i < hChn->diseqc->diseqcStatus.nReplyBytes; i++)
      {
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSRPLY, &mb);
         hChn->diseqc->diseqcStatus.replyBuffer[i] = (uint8_t)(mb & 0xFF);
      }
   }

   /* reset the FIFO, memory, noise integrator, etc. */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, 0x7F002201);

   /* power off diseqc rx */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL02, 0x00800000);

   /* set diseqc event */
   BKNI_SetEvent(hChn->diseqc->hDiseqcEvent);
}


/******************************************************************************
* BAST_g3_P_Diseqc_UpdateSettings()
******************************************************************************/
static BERR_Code BAST_g3_P_Diseqc_UpdateSettings(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;

   /* set envelope or tone mode */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DDIO, &mb);
   mb &= ~0x700000;
   if (hChn->diseqc->dsecSettings.bEnvelope)
      mb |= 0x100000;
   if (hChn->diseqc->dsecSettings.bEnableLNBPU)
      mb |= 0x10000000;
   else
      mb &= ~0x10000000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DDIO, &mb);

   /* tone align enable or disable */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &mb);
   if (hChn->diseqc->dsecSettings.bToneAlign)
      mb |= 0x0800;
   else
      mb &= ~0x0800;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &mb);

   /* receive reply timeout enable or disable */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &mb);
   if (hChn->diseqc->dsecSettings.bDisableRRTO)
      mb |= 0x400000;
   else
      mb &= ~0x400000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL01, &mb);

   /* receive reply timeout setting */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_RXRT, &mb);
   mb &= 0xFFF00000;
   mb |= (hChn->diseqc->dsecSettings.rrtoUsec & 0x000FFFFF);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_RXRT, &mb);

   /* burst enable or disable */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);
   if (hChn->diseqc->dsecSettings.bEnableToneburst)
      mb |= 0x40;
   else
      mb &= ~0x40;
   /* tone A or tone B */
   if (hChn->diseqc->dsecSettings.bToneburstB)
      mb |= 0x80;
   else
      mb &= ~0x80;
   /* expect reply enable or disable */
   if ((hChn->diseqc->dsecSettings.bOverrideFraming) && (hChn->diseqc->dsecSettings.bExpectReply == false))
      mb |= 0x04;
   else
      mb &= ~0x04;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCTL00, &mb);

   /* program pretx delay */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_Q15T, &mb);
   mb |= (hChn->diseqc->dsecSettings.preTxDelay * 1000) << 16;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_Q15T, &mb);

   return retCode;
}


/******************************************************************************
 BAST_g3_P_DiseqcTxFifoAlmostEmpty_isr()
******************************************************************************/
void BAST_g3_P_DiseqcTxFifoAlmostEmpty_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t mb;
   uint8_t bytes_avail, i, nbytes;

   BSTD_UNUSED(param);

   BAST_g3_P_IncrementInterruptCounter_isr(h, BAST_g3_IntID_eDiseqcTxFifoAlmostEmpty);
   BINT_ClearCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);
   BINT_DisableCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);

   /* determine number of bytes available in tx fifo */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_DSEC_DST1, &mb);
   bytes_avail = (mb >> 14) & 0x1F;

   /* cannot send more bytes than what's available */
   if (hChn->diseqc->diseqcSendLen - hChn->diseqc->diseqcSendCount <= bytes_avail)
      nbytes = hChn->diseqc->diseqcSendLen - hChn->diseqc->diseqcSendCount;
   else
      nbytes = bytes_avail;

   for (i = 0; i < nbytes; i++)
   {
      mb = hChn->diseqc->diseqcSendBuffer[hChn->diseqc->diseqcSendCount++];
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_DSEC_DSCMD, &mb);
   }

   if (hChn->diseqc->diseqcSendCount < hChn->diseqc->diseqcSendLen)
   {
      /* re-arm tx fifo almost empty interrupt if not done */
      BINT_ClearCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);
      BINT_EnableCallback_isr(hChn->diseqc->hDiseqcTxFifoAlmostEmptyCb);
   }
}
