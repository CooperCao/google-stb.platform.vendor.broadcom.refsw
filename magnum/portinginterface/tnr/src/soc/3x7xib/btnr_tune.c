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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btmr.h"
#ifndef LEAP_BASED_CODE
#include "bmem.h"
#include "btnr_3x7x.h"
#include "bdbg.h"
#include "btnr_priv.h"
#include "btnr_3x7xib_priv.h"
#endif
#include "btnr_global_clk.h"
#include "btnr_struct.h"
#ifdef  LEAP_BASED_CODE
#include "btnr_api.h"
#endif
#include "btnr_tune.h"
#include "bmth.h"
#if ((BCHP_CHIP == 3461) || (BCHP_CHIP == 3462) || (BCHP_CHIP == 3472))
#include "bchp_tm.h"
#endif
#if ((BCHP_CHIP == 7552) || (BCHP_CHIP == 7563) || (BCHP_CHIP == 75635))
#include "bchp_gio_aon.h"
#endif
#if ((BCHP_CHIP == 7563) || (BCHP_CHIP == 75635))
#include "bchp_jtag_otp.h"
#endif
#include "bchp_ufe_afe.h"
#include "bchp_sdadc.h"
#ifndef LEAP_BASED_CODE
BDBG_MODULE(btnr_tune);
#define POWER2_24 16777216
#define POWER2_3 8
#define POWER2_16 65536
#define POWER2_29 536870912
#define POWER2_27 134217728
#endif
#include "bchp_ufe.h"
#include "btnr_init.h"

#include "btnr_tune_ofdm.h"
#include "btnr_tune_ads.h"

#if ((BCHP_CHIP == 3472) || ((BCHP_CHIP == 3461) && (BCHP_VER == BCHP_VER_B0)) || (BCHP_CHIP == 3462) || (BCHP_CHIP == 7552) || (BCHP_CHIP == 7563) || (BCHP_CHIP == 75635))
#define LOW_POWER_MODE
#endif

#ifdef  LOW_POWER_MODE
#if (BCHP_CHIP == 7552)
#ifdef BTNR_LNAAGC_SAVE_POWER
#define ENABLE_LNA_AGC_CYCLE
#endif
#ifdef BTNR_LNAAGC_FRZ
#define ENABLE_LNA_AGC_FRZ
#endif
#else
#define ENABLE_LNA_AGC_CYCLE
#endif
#endif

#include "bchp_ufe.h"

/*******************************************************************************************************************
 * BTNR_P_TunerSleep()  This routine sets the tuner delay x ms
 ******************************************************************************************************************/
void BTNR_P_TunerSleep(uint8_t x)
{
#ifdef LEAP_BASED_CODE
        BKNI_Sleep(x);
#else
        BKNI_Delay(x*1000);
#endif
}

/******************************************************************************
 * BTNR_P_TunerLNAAGCUNFRZ() This routine runs LNAAGC for 100ms
******************************************************************************/
void BTNR_P_TunerLNAAGCUNFRZ(BTNR_3x7x_ChnHandle h)
{
	/*Reset WBADC & UnFreeze LNAAGC */
	BTNR_P_TunerSetADC6B(h);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 0x0);
	/* --- Sleep X (To control the duty cycle) 20% --- */
	BKNI_Sleep(100);
	h->pTunerStatus->LNAAGC_FRZ = 0;
}

/******************************************************************************
 * BTNR_P_TunerLNAAGCFRZ() This routine freezes LNAAGC
******************************************************************************/
void BTNR_P_TunerLNAAGCFRZ(BTNR_3x7x_ChnHandle h)
{
	/* --- Freeze LNA --- */
	/*Freeze LNA AGC  , Power Down 6-bit ADC & PHYPLL ch1*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 0x1);
	BKNI_Delay(20); /*20 usec*/
	BTNR_P_TunerSetADC6BDOWN(h);
	h->pTunerStatus->LNAAGC_FRZ = 1;
}

/******************************************************************************
 * BTNR_P_TunerLNAAGCUNUSED() This routine keeps LNAAGC running
******************************************************************************/
void BTNR_P_TunerLNAAGCUNUSED(BTNR_3x7x_ChnHandle h)
{
	if (h->pTunerStatus->LNAAGC_FRZ == 1) {BTNR_P_TunerLNAAGCUNFRZ(h);}
	BSTD_UNUSED(h);
}

/*******************************************************************************************
 *BTNR_P_LNAAGCCycle()		This routine keeps LNA AGC and 6 Bit ADC enabled for a small duty
 *cycle to save power. LNA AGC is slower than RF AGC so slower reaction time is not an issue.
 *The PI will call the BTNR_P_LNAAGCCycle() every Xmsec (presently 100ms), based on a timer
 *interrupt
 *******************************************************************************************/
BERR_Code BTNR_P_LNAAGCCycle(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;

#ifndef ENABLE_LNA_AGC_FRZ
#ifdef ENABLE_LNA_AGC_CYCLE
	if (h->pTunerStatus->RFVGA_byp == 1) {BTNR_P_TunerLNAAGCUNUSED(h);}
	else {BTNR_P_TunerLNAAGCUNFRZ(h); BTNR_P_TunerLNAAGCFRZ(h);}
#else
	BSTD_UNUSED(h);
#endif
#else
	if (h->pTunerStatus->LNAAGC_count > 0)
	{
		BTNR_P_TunerLNAAGCUNFRZ(h); BTNR_P_TunerLNAAGCFRZ(h);
		h->pTunerStatus->LNAAGC_count = h->pTunerStatus->LNAAGC_count -1;
		BDBG_MSG(("h->pTunerStatus->LNAAGC_count, value received is %d",h->pTunerStatus->LNAAGC_count));
	}
#endif
	/*goto label to return error code if something bad happened above*/
	return retCode;
}

/*******************************************************************************************
 * BTNR_P_Daisy_Control()		This routine controls the the Daisy output
 *The PI will call the BTNR_P_Tuner_Power_Control() when it detects a change in the
 *h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable THEN call this function
 *******************************************************************************************/
BERR_Code BTNR_P_Daisy_Control(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;

	if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable == BTNR_Daisy_Params_eEnable)
	{
		/*anything not handled by BTNR_P_Tuner_Power_Control()*/

		if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source == BTNR_Daisy_Source_eVHF)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_VHF, 0x1);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_UHF, 0x0);
		}
		else if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source == BTNR_Daisy_Source_eUHF)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_VHF, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_UHF, 0x1);
		}
		else if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source == BTNR_Daisy_Source_eUHF_VHF)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_VHF, 0x1);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_UHF, 0x1);
		}
	}
	else
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_VHF, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_UHF, 0x0);
	}

	BDBG_MSG(("BTNR_P_Daisy_Control() Complete"));

/*goto label to return error code if something bad happened above*/
  return retCode;
}

/*******************************************************************************************
 * BTNR_P_LoopThru_Control()		This routine controls the LoopThru output
 *The PI will call the BTNR_P_Tuner_Power_Control() when it detects a change in the
 *h->pTunerParams->BTNR_LoopThru_Params.LoopThru_Enable THEN call this function
 *******************************************************************************************/

BERR_Code BTNR_P_LoopThru_Control(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;

	if (h->pTunerParams->BTNR_LoopThru_Params.LoopThru_Enable == BTNR_LoopThru_Params_eEnable)
	{
        /* enable yje BIASs to enable LT without powering up the tuner */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_BIAS, 0x1);
        BKNI_Sleep(1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_LT, 0x1);
	}
	else
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_LT, 0x0);
	}

	BDBG_MSG(("BTNR_P_LoopThru_Control() Complete"));

/*goto label to return error code if something bad happened above*/
  return retCode;
}

/*******************************************************************************************
 * BTNR_P_DPM_Control()		This routine controls the LoopThru output
 *The PI will call the BTNR_P_Tuner_Power_Control() when it detects a change in the
 *h->pTunerParams->BTNR_DPM_Params.DPM_Enable THEN call this function
 *******************************************************************************************/
BERR_Code BTNR_P_DPM_Control(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint16_t temp;
	if (h->pTunerParams->BTNR_DPM_Params.DPM_Enable == BTNR_DPM_Params_eEnable)
	{
		BTNR_P_DPMSetFreq(h);
	}
	else
	{
	/*Power down DPM*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DPM_LOBUF_REG, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DPM, 0x0);
	/*Power down DPM fx clock*/
		temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
		temp = temp & 0x2F ;
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
	}
	BDBG_MSG(("BTNR_P_DPM_Control() Complete"));
  return retCode;
}

/******************************************************************************
 BTNR_P_TunerSetMXR()
******************************************************************************/
void BTNR_P_TunerSetMXR(BTNR_3x7x_ChnHandle h)
{
	/*MXR SR, pre_div reset*/
	if (h->pTunerStatus->LO_index == 10)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_div_ratio, 0x1);
		BKNI_Delay(20); /*20 usec*/
	}
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_SR_reset, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x1);
	if (BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_HRM_mode) == 1)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_SR_reset, 0x0);
	}
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x0);
	BKNI_Delay(20); /*20 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_SR_reset, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x1);
	if (BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_HRM_mode) == 1)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_SR_reset, 0x0);
	}
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x0);
	if ((h->pTunerStatus->LO_index == 8) || (h->pTunerStatus->LO_index == 9))
	{
		BKNI_Delay(20); /*20 usec*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_div_ratio, 0x0);
		BKNI_Delay(20); /*20 usec*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, MXR_PREDIV_reset, 0x0);
	}
	else if (h->pTunerStatus->LO_index == 10)
	{
		BKNI_Delay(h->pTunerStatus->dly);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_div_ratio, 0x0);
	}
	BDBG_MSG(("BTNR_P_TunerSetMXR() Complete"));
}

/******************************************************************************
 BTNR_P_TunerSetLO()
******************************************************************************/
void BTNR_P_TunerSetLO(BTNR_3x7x_ChnHandle h)
{
	/*local variables*/
	uint8_t		index, M, N, ndiv, pdiv;
	uint32_t	ulMultA, ulMultB, ulNrmHi, ulNrmLo, Freq;
	uint16_t	temp;

	/* Enable LO state-machine clock */
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp | 0x4 ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);

	/*Optimize LO settings*/
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_MXRPLL_01, 0xcf35ef14);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_MXRPLL_02, 0x4805185b);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_MXRPLL_03, 0xa8880300);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_MXRPLL_04, 0x0000004a);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_MXRPLL_05, 0x00ebb255);

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT_1p0, 0xC);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x4);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_VcREF_1p0, 0x15);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_04, lP_mainVbal_1p0, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_04, lP_mainIbal_1p0, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, P_divreg1p0_cntl_1p0, 0x3);

	/* Set the DDFS FCW*/
	if (h->pTunerParams->BTNR_DPM_Params.DPM_Enable != BTNR_DPM_Params_eEnable)
	{
		/*Program the values for Tuner LO*/
		/* lookup freq range */
		for (index = 0; index < BTNR_TUNER_LO_TABLE_SIZE; index++)
		{
			if (index == 10)
			{
				if (h->pTunerParams->BTNR_Local_Params.hardware_tune == 0) {Freq = h->pTunerParams->BTNR_Local_Params.VCO_max/10*1000;}
				else {Freq = h->pTunerParams->BTNR_Local_Params.freq_HRM_max;}
			}
			else
			{
				if (Tuner_LO_Freq_Table[index] < 8) {Freq = (1002000/Tuner_LO_Freq_Table[index])*1000;}
				else {Freq = (h->pTunerParams->BTNR_Local_Params.VCO_max/Tuner_LO_Freq_Table[index])*1000;}
			}
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= Freq)
			{
				/*index = BTNR_TUNER_LO_TABLE_SIZE;*/
				break;
			}
		}
		h->pTunerStatus->LO_index = index;
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MXR_SR6p8p12p16p,    Tuner_LO_Table[index].i_MXR_SR6p8p12p16p);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel_div_ratio, Tuner_LO_Table[index].i_MIXER_sel_div_ratio);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel,           Tuner_LO_Table[index].i_MIXER_sel);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_HRM_mode,      Tuner_LO_Table[index].i_MIXER_HRM_mode);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel_MUX0p6p8p, Tuner_LO_Table[index].i_MIXER_sel_MUX0p6p8p);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_fbdivn_1p0,         Tuner_LO_Table[index].lP_fbdivn_1p0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_div23_sel_1p0,      Tuner_LO_Table[index].lP_div23_sel_1p0);
		M = Tuner_LO_Table[index].M_Factor;
		N = Tuner_LO_Table[index].lP_fbdivn_1p0;
	}
	else
	{
		/*Program the values for DPM LO*/
		/* lookup freq range */
		for (index = 0; index < BTNR_DPM_LO_TABLE_SIZE; index++)
		{
			if (index == 10)
			{
				if (h->pTunerParams->BTNR_Local_Params.hardware_tune == 0) {Freq = h->pTunerParams->BTNR_Local_Params.VCO_max/10*1000;}
				else {Freq = h->pTunerParams->BTNR_Local_Params.freq_HRM_max;}
			}
			else
			{
				if (DPM_LO_Freq_Table[index] < 8) {Freq = (1002000/DPM_LO_Freq_Table[index])*1000;}
				else {Freq = (h->pTunerParams->BTNR_Local_Params.VCO_max/DPM_LO_Freq_Table[index])*1000;}
			}
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= Freq)
			{
				/*index = BTNR_TUNER_LO_TABLE_SIZE;*/
				break;
			}
		}
		h->pTunerStatus->LO_index = index; if (h->pTunerStatus->LO_index == 10) {h->pTunerStatus->REFPLL = 1;} else {h->pTunerStatus->REFPLL = 0;}
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MXR_SR6p8p12p16p,    DPM_LO_Table[index].i_MXR_SR6p8p12p16p);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel_div_ratio, DPM_LO_Table[index].i_MIXER_sel_div_ratio);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel,           DPM_LO_Table[index].i_MIXER_sel);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_HRM_mode,      DPM_LO_Table[index].i_MIXER_HRM_mode);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01,    i_MIXER_sel_MUX0p6p8p, DPM_LO_Table[index].i_MIXER_sel_MUX0p6p8p);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_fbdivn_1p0,         DPM_LO_Table[index].lP_fbdivn_1p0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_div23_sel_1p0,      DPM_LO_Table[index].lP_div23_sel_1p0);
		M = DPM_LO_Table[index].M_Factor;
		N = DPM_LO_Table[index].lP_fbdivn_1p0;
		/*Program the values for DPM*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01,    i_logen_PreSel,        DPM_Table[index].logen_PreSel);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01,    i_logen_two,           DPM_Table[index].logen_two);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01,    i_logen_six,           DPM_Table[index].logen_six);
		BKNI_Sleep(10);
	}
	index = h->pTunerStatus->LO_index;
	Freq = (h->pTunerParams->BTNR_Acquire_Params.RF_Freq);
	if (h->pTunerStatus->LO_index == 10) {h->pTunerStatus->REFPLL = 1;} else {h->pTunerStatus->REFPLL = 0;}
	if ((Freq > (719143000-3000000)) && (Freq < (719143000+3000000))) {h->pTunerStatus->REFPLL = 0;}
	BDBG_MSG(("BTNR_P_TunerSetLO: index %d",index));
	BDBG_MSG(("BTNR_P_TunerSetFreq: M=%d, N=%d",M, N));

	h->pTunerStatus->Tuner_Ref_Lock_Status  = (BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_04, o_REFPLL_lock) == 1) ? 1 : 0;
	BDBG_MSG(("Tuner_Ref_Lock_Status %d, h->pTunerStatus->REFPLL = %d",h->pTunerStatus->Tuner_Ref_Lock_Status,h->pTunerStatus->REFPLL));
	if (((h->pTunerStatus->REFPLL == 1) && (h->pTunerStatus->Tuner_Ref_Lock_Status  == 0))
	||  ((h->pTunerStatus->REFPLL == 0) && (h->pTunerStatus->Tuner_Ref_Lock_Status  == 1)))
	{BTNR_P_TunerSetREFPHYPLL(h);}

	/* FCW = (Fc/RFPLL_FREQ)*(M/N)*2^37* where Fc is desired frequency
	 * M[2 255], N[4,8,12,16,24,32,64]
	 * FCW = ((Fc/100)*M)*2^37))/(RFPLL_FREQ/100*N)) where Fc is desired frequency*/
	BDBG_MSG(("Rf Freq=%08x",h->pTunerParams->BTNR_Acquire_Params.RF_Freq));

	ulMultA = 0x1;
	ulMultB = 0;
	ulNrmHi = 0;
	if ((index == 8) || (index == 9) || (index == 10)) {ulNrmLo = Freq/2*(M/4);}
	else {ulNrmLo = Freq*(M/4);}

#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_01, REF_DIV_fb);
	pdiv = 1;
	ndiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_02, REF_DIV_ratio);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
{
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_01, REF_DIV_fb);
	pdiv = 1;
	ndiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_02, REF_DIV_ratio);
}
else
{
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int);
	pdiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div);
	ndiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m2div);
}
#endif
	BMTH_HILO_64TO64_Mul(ulMultA, ulMultB, ulNrmHi, ulNrmLo, &ulMultA, &ulMultB);
	BDBG_MSG(("First %08x, %08x",ulMultA, ulMultB));
	BMTH_HILO_64TO64_Div32(ulMultA, ulMultB,  ( (REF_FREQ*(temp/ndiv/pdiv)/128)*N ), &ulNrmHi, &ulNrmLo);
	BDBG_MSG(("Second %08x, %08x",ulNrmHi, ulNrmLo));

	if (ulNrmHi > 0x0000001F)
	{
     BDBG_ERR(("DDFS is outside of the 37 bit range in BTNR_P_TunerSetFreq()"));
	}

	/* Setup DDFS*/
	/* program fcw[36:5] and fcw[4:0]*/
	ulNrmHi = (((ulNrmHi<<27) & 0xF8000000) | ((ulNrmLo>>5) & 0x07FFFFFF)); /*Get 32 msb's*/
	ulNrmLo = (ulNrmLo & 0x0000001F);                                        /*Get 5 lsb's*/
	/*Reset and Release DDFS*/
	#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0) /* (BCHP_VER == BCHP_VER_A0) */   /* TO FIX - Fixed*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_MXRPLL, 0x7afe  );
		BKNI_Sleep(1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_MXRPLL, 0x7aff  );
	#else
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02,    loddfs_resetn, 0);
		BKNI_Sleep(1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02,    loddfs_resetn, 1);
	#endif

	/*Program FCW*/
	BDBG_MSG(("BTNR_P_TunerSetFreq: Hi=%x, Lo=%x",ulNrmHi, ulNrmLo));
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_DDFS_FCW_02, ulNrmHi);      /*32 MSB of 37 bit word*/
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_DDFS_FCW_01, ulNrmLo);      /* 5 LSB of 37 bit word*/

	/* De-assert reset/resetb for div23, cml, SR, pre_div */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_rstdiv23_1p0, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_cmlDIVRST, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_ld_RESET_STRT_1p0, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_rstdiv23_1p0, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_cmlDIVRST, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, lP_ld_RESET_STRT_1p0, 0x0);
	BKNI_Sleep(1);

	/* Startup DDFS*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DDFS_01, i_fcw_strb, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DDFS_01, i_fcw_strb, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DDFS_01, i_fcw_strb, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DDFS_01, i_fcw_strb, 0x0);
	if (h->pTunerParams->BTNR_Local_Params.hardware_tune == 1)
	{
		/*Enable auto tuner*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LO_SMtuner_resetb, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_05, LO_SMtuner_tune_en, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_05, LO_SMtuner_param_sel, 0x1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LO_SMtuner_resetb, 0x1);
		BKNI_Sleep(1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_05, LO_SMtuner_tune_en, 0x1);
		BKNI_Sleep(1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_05, LO_SMtuner_tune_en, 0x0);
	}
	else if (h->pTunerParams->BTNR_Local_Params.hardware_tune == 0) {BTNR_P_TunerSearchCapSoftware(h);}

  /*Power down state machine clock*/
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp & 0x3B ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
	/*Lock Status*/
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_07, MXRPLL_FREQ_LOCK);
	if (temp == 1)
	{
		BDBG_MSG(("MIXPLL is locked"));
	}
	else if (temp == 0)
	{
		BDBG_MSG(("MIXPLL is NOT locked"));
	}
}

/*******************************************************************************************
 * BTNR_P_Tuner_PowerUpPLL()		This routine controls the power up/down of the tuner blocks
 *******************************************************************************************/
BERR_Code BTNR_P_Tuner_PowerUpPLL(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	/* optimize ADC6B */
	h->pTunerStatus->REFPLL = 0;
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_ADC6B_01, 0xCA683600);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_ADC6B_02, 0x54300055);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_ADC6B_03, 0x39428002);
	/* Swap I/Q on 8/12/16-phase mixer */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_MIXER_setSR8_Q, 0xf0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_MIXER_setSR8_I, 0xc3);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_setSR12_I, 0xE07);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_setSR12_Q, 0xFC0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_04, i_MIXER_setSR16_I, 0xF00F);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_04, i_MIXER_setSR16_Q, 0xFF00);
	/*bias settings for LNA + MXR + FGA + LPF*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf_vcm, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_FB_bias_ctrl, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_vcm_ctrl, 0x0);
/*	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_Vb_sk, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_Ib_sk, 0x0);*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_vcmfb_gen_ctrl, 0x3);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_low_I_mode, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_CM_internal_sel, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_CMset, LPF_CM);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_freq_sel, 0x3);
	/* set RFAGC clock from REFPLL */
#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x1);
else
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x0);
#endif
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
#if (BCHP_CHIP == 7552)
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, spare00, 0x1);
#else
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x1);
#endif
#endif
	/*Set RFPLL and PHYPLL*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_01, REF_DIV_fb, 0x14);    /*REF_DIV_fb = 20*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_02, REF_DIV_ratio, 0x01); /*REF_DIV_ratio = 1*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_OUTDIV_m0, 0x01); /*REF_OUTDIV_m0 = 1*/
#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_CLK_bypass_enable, 0x0);
else
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_CLK_bypass_enable, 0x1);
#else
	BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_CLK_bypass_enable, 0x0);
#endif

#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_1)
	if ((h->pTunerParams->BTNR_Local_Params.RevId  == BCHP_VER_B1)|| (h->pTunerParams->BTNR_Local_Params.RevId  == BCHP_VER_B2))
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, i_clk_sel, 0x1);
	else
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, i_clk_sel, 0x0);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_2)  /* 3462 A0 */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, i_clk_sel, 0x1);
#endif
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_02, ndiv_frac, 0x0);/*ndiv_frac = 0*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div,    0x03);/*p1div = 3*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int, 0x78);/*ndiv_int = 150*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m1div,    0x01);/*m1div = 1*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m2div,    0x02);/*m2div = 2*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m3div,    0x0C);/*m3div = 12*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m4div,    0x1B);/*m4div = 27*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m5div,    0x00);/*m5div = 256*/
    BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m6div,    0x04);/*m6div = 4*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x3F);

#ifdef SmartTuneEnabled
    /* for multi channel tuner, TM_SYS_PLL regs are offset (from channel 0's base) using hTMSysPllReg */
#if ( BTNR_P_BCHP_TNR_NUM_CORES < 2 ) /* (e.g. not dual tuner [3472]) */
	BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_PDIV, 2);
    BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_NDIV_INT, 80);
#else
    BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_PDIV, 2);
    BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_NDIV_INT, 80);
#endif
        /* TM_SYS_PLL_CLK_XPT_216 not channelized */
#if (BTNR_P_BCHP_TNR_CORE_VER < BTNR_P_BCHP_CORE_V_1_3)
    BREG_WriteField(h->hTnr->hRegister, TM_SYS_PLL_CLK_216, DIV, 10);
#else
    BREG_WriteField(h->hTnr->hRegister, TM_REG_PLL_CLK_XPT_216, DIV, 10);
#endif
#endif /* SmartTuneEnabled */

	/*optimize PHYPLL*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, kvcox, 0x3);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, Icpx, 0x1F);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, Icpx2, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, Rz, 0xF);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, 0x00030001);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, 0x00208000);
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006CFF);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006CFF);
else
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006806);
#endif
	BKNI_Sleep(1);
	/* De-assert reset/resetb for REFPLL/PHYPLL */
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESETB_01, 0x00000003);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESET_01, 0xFFFFFFF0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 0);
	BKNI_Sleep(1);
	/* Initialize SD ADC */
	/*This Must Be Complete with calibration added*/
	BREG_WriteField(h->hRegister, SDADC_CTRL_SYS0, i_adcclk_reset, 0x0);
	BREG_Write32(h->hRegister, BCHP_SDADC_CTRL_PWRUP, 0x00000003);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x0);

	BDBG_MSG(("BTNR_P_TunerPowerUpPLL() Complete"));
	return retCode;
}

/*******************************************************************************************
 * BTNR_P_TunerSetInputMode()		This routine set tuner input mode
 *******************************************************************************************/
BERR_Code BTNR_P_TunerSetInputMode(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t temp_UFE_AFE_TNR0_PWRUP_01 = 0;
	switch (h->pTunerParams->BTNR_RF_Input_Mode)
	{
	case BTNR_3x7x_TunerRfInputMode_eExternalLna:
		temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x0F0F0015);
		if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable == BTNR_Daisy_Params_eEnable)
		{
			switch (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source)
			{
			case BTNR_Daisy_Source_eVHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00000800);
			break;
			case BTNR_Daisy_Source_eUHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00001000);
			break;
			case BTNR_Daisy_Source_eUHF_VHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00001800);
			break;
			default:
				BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable, value received is %d",h->pTunerParams->BTNR_Daisy_Params.Daisy_Source));
				retCode = BERR_INVALID_PARAMETER;
			goto something_bad_happened;
			}
		}
		if (h->pTunerParams->BTNR_LoopThru_Params.LoopThru_Enable == BTNR_LoopThru_Params_eEnable) {temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00000400);}
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, temp_UFE_AFE_TNR0_PWRUP_01);
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00208000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00002CFF);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00208000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00002CFF);
}
else
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00228000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00006806);
}
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_bypass, 0);
	break;
	case BTNR_3x7x_TunerRfInputMode_eInternalLna:
	case BTNR_3x7x_TunerRfInputMode_InternalLna_Daisy:
		temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x0FA30017);
		if (h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable == BTNR_Daisy_Params_eEnable)
		{
			switch (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source)
			{
			case BTNR_Daisy_Source_eVHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00000800);
			break;
			case BTNR_Daisy_Source_eUHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00001000);
			break;
			case BTNR_Daisy_Source_eUHF_VHF:
				temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00001800);
			break;
			default:
				BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_Daisy_Params.Daisy_Enable, value received is %d",h->pTunerParams->BTNR_Daisy_Params.Daisy_Source));
				retCode = BERR_INVALID_PARAMETER;
			goto something_bad_happened;
			}
		}
		if (h->pTunerParams->BTNR_LoopThru_Params.LoopThru_Enable == BTNR_LoopThru_Params_eEnable) {temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x00000400);}
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, temp_UFE_AFE_TNR0_PWRUP_01);
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00218000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00006CFF);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00218000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00006CFF);
}
else
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00238000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00006806);
}
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_bypass, 0);
	break;
	case BTNR_3x7x_TunerRfInputMode_eStandardIf:
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01,	0x0F030015);
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00208000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00002CFF);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00208000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00002CFF);
}
else
{
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02,	0x00228000);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,	0x00006806);
}
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_bypass, 1);
	break;
	case BTNR_3x7x_TunerRfInputMode_eLowIf:
	case BTNR_3x7x_TunerRfInputMode_eBaseband:
	case BTNR_3x7x_TunerRfInputMode_eOff:
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, 0x00030001);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, 0x00208000);
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00002CFF);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
if (h->pTunerStatus->REFPLL == 1)
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00002CFF);
else
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006806);
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_bypass, 0);
	break;
	default:
		BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_RF_Input_Mode, value received is %d",h->pTunerParams->BTNR_RF_Input_Mode));
		retCode = BERR_INVALID_PARAMETER;
		/*goto bottom of function to return error code*/
		goto something_bad_happened;
	}
	/* force to terrestrial mode */
	h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eACI;
	BTNR_P_TunerSetSignalPath(h);
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
	{h->pTunerStatus->AGC_TOP = 0; h->pTunerStatus->LNA_TOP = LNA_TOP_MAX; BTNR_P_Ods_TunerSetAGCTOP(h);}
	if (h->pTunerParams->BTNR_Acquire_Params.Application  == BTNR_TunerApplicationMode_eCable) BTNR_P_Ads_TunerSetAGCTOP(h);
	BTNR_P_TunerSetRFAGC(h);
	BTNR_P_TunerSetADC6B(h); h->pTunerStatus->LNAAGC_FRZ = 0; h->pTunerStatus->RFVGA_byp = 0;
	BTNR_P_TunerSetLNAAGC(h);
	if (h->pTunerParams->BTNR_Acquire_Params.Application  == BTNR_TunerApplicationMode_eTerrestrial) {h->pTunerStatus->AGC_BW = 0; BTNR_P_Ods_TunerSetAGCBW(h);}
	if (h->pTunerParams->BTNR_Acquire_Params.Application  == BTNR_TunerApplicationMode_eCable) {h->pTunerStatus->AGC_BW = 1; BTNR_P_Ads_TunerSetAGCBW(h);}
	#ifdef BTNR_ENABLE_SDADC_CAL
	BTNR_P_CalFlashSDADC(h);
	#endif
	h->pTunerParams->BTNR_Internal_Params.RFInputModeComplete = true;
    #ifdef LEAP_BASED_CODE
	BTNR_P_TunerAGCMonitor(h);
	BTMR_StartTimer(h->htmr, 500000);   /* the timer is in Micro second */
	#endif
	BDBG_MSG(("BTNR_P_TunerSetInputMode() Complete"));
	BDBG_MSG((" h->pTunerParams->BTNR_RF_Input_Mode, value received is %d",h->pTunerParams->BTNR_RF_Input_Mode));
/*goto label to return error code if something bad happened above*/
something_bad_happened:
  return retCode;
}

/******************************************************************************
 BTNR_P_TunerStatusReset() - reset the status structure
******************************************************************************/
BERR_Code BTNR_P_TunerStatusReset(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;

	/* clear the lock status */
	h->pTunerStatus->Tuner_Ref_Lock_Status = BTNR_Status_eUnlock;
	h->pTunerStatus->Tuner_Mixer_Lock_Status = BTNR_Status_eUnlock;
	h->pTunerStatus->Tuner_Phy_Lock_Status = BTNR_Status_eUnlock;
	h->pTunerStatus->Tuner_RF_Freq = 0;
	h->pTunerStatus->Tuner_PreADC_Gain_x256db = (int16_t)0x8000;
	h->pTunerStatus->External_Gain_x256db = (int16_t)0x8000;

  return retCode;
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetExternalFGLNASwitch()  This routine sets the external FGLNA switch
 ******************************************************************************************************************/
void BTNR_P_TunerSetExternalFGLNASwitch(BTNR_3x7x_ChnHandle h)
{
	uint32_t temp;
#if ((BCHP_CHIP == 7552)  || (BCHP_CHIP == 7563))
	if (h->pTunerStatus->External_FGLNA_Mode == 1)
	{
		temp = BREG_Read32(h->hRegister, BCHP_GIO_AON_DATA_LO);
		temp = temp | 0x00000010;
		BREG_Write32(h->hRegister, BCHP_GIO_AON_DATA_LO, temp);
		BDBG_MSG(("External FGLNA set to normal mode"));
		h->pTunerStatus->dly = 3; BTNR_P_TunerRSSI(h);
	}
	else if (h->pTunerStatus->External_FGLNA_Mode == 0)
	{
		temp = BREG_Read32(h->hRegister, BCHP_GIO_AON_DATA_LO);
		temp = temp & 0xFFFFFFEF;
		BREG_Write32(h->hRegister, BCHP_GIO_AON_DATA_LO, temp);
		BDBG_MSG(("External FGLNA set to bypass mode"));
	}
#endif
#if ((BCHP_CHIP == 3461) || (BCHP_CHIP == 3462) || (BCHP_CHIP == 3472))
	if (h->pTunerStatus->External_FGLNA_Mode == 1)
	{
		temp = BREG_Read32(h->hTnr->hRegister, BCHP_TM_GPIO_DATA);
		temp = temp | 0x00000008;
		BREG_Write32(h->hTnr->hRegister, BCHP_TM_GPIO_DATA, temp);
		BDBG_MSG(("External FGLNA set to normal mode"));
	}
	else if (h->pTunerStatus->External_FGLNA_Mode == 0)
	{
		temp = BREG_Read32(h->hTnr->hRegister, BCHP_TM_GPIO_DATA);
		temp = temp & 0xFFFFFFF7;
		BREG_Write32(h->hTnr->hRegister, BCHP_TM_GPIO_DATA, temp);
		BDBG_MSG(("External FGLNA set to bypass mode"));
	}
#endif
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetExternalFGLNA()  This routine sets the external FGLNA mode
 ******************************************************************************************************************/
static void BTNR_P_TunerSetExternalFGLNA(BTNR_3x7x_ChnHandle h)
{
	if ((h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable) &&
		(h->pTunerStatus->Tuner_Change_Settings == 0))
	{
		BTNR_P_TunerAGCReadBack(h);
		if ((h->pTunerStatus->External_FGLNA_Mode == 1) && (h->pTunerStatus->Tuner_LNA_Gain_Code <= FGLNA_BYPASS_THRESHOLD_HIGH) )
		{h->pTunerStatus->External_FGLNA_Mode = 0; BTNR_P_TunerSetExternalFGLNASwitch(h);}
		if ((h->pTunerStatus->External_FGLNA_Mode == 0) && (h->pTunerStatus->Tuner_LNA_Gain_Code >= FGLNA_BYPASS_THRESHOLD_LOW))
		{h->pTunerStatus->External_FGLNA_Mode = 1; BTNR_P_TunerSetExternalFGLNASwitch(h);}
	}
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetRFFILSwitch()  This routine sets RFFIL switches
 ******************************************************************************************************************/
static void BTNR_P_TunerSetRFFILSwitch(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerParams->BTNR_Internal_Params.RFFIL_Select)
	{
	case BTNR_Internal_Params_TunerRFFIL_eMOCATRAP:
		/*Use MOCA Trap*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, RF_DPD_sel, 1);/* RF peak-detector set to MoCA trap */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen, 1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en, 0x0);
	break;
	case BTNR_Internal_Params_TunerRFFIL_eTRKFIL:
		/*Use Tracking Filter*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, RF_DPD_sel, 0);/* RF peak-detector set to tracking filter */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL, 0x1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen, 0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en, 0x0);
	break;
	case BTNR_Internal_Params_TunerRFFIL_eTRKBYPASS:
		/*Use Tracking Filter Bypass*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, RF_DPD_sel, 0);/* RF peak-detector set to tracking filter */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en, 0x1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen, 0x0);
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.RFFIL_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.RFFIL_Select));
	}
	BDBG_MSG(("BTNR_P_TunerSetRFFILSwitch() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_TunerRFAGCIndirectRead()  This routine does RFAGC indirect write
 ******************************************************************************************************************/
 void BTNR_P_TunerRFAGCIndirectRead(BTNR_3x7x_ChnHandle h)
{
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_raddr, h->pTunerStatus->RFAGC_indirect_addr);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x0);
	h->pTunerStatus->RFAGC_indirect_data = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFAGC_05, o_rdata);
}

/*******************************************************************************************************************
 * BTNR_P_TunerRFAGCIndirectWrite()  This routine does RFAGC indirect write
 ******************************************************************************************************************/
 void BTNR_P_TunerRFAGCIndirectWrite(BTNR_3x7x_ChnHandle h)
{
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, h->pTunerStatus->RFAGC_indirect_addr);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, h->pTunerStatus->RFAGC_indirect_data);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
}

/*******************************************************************************************************************
 * BTNR_P_TunerAGCReadBack()  This routine read back the tuner AGC gain code
 ******************************************************************************************************************/
 void BTNR_P_TunerAGCReadBack(BTNR_3x7x_ChnHandle h)
{
	uint32_t ReadReg0;
	uint8_t ReadReg1;
	h->pTunerStatus->MAX_gain_code = 0;
	h->pTunerStatus->RFAGC_MAX = 0;
	/*LNA AGC level*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_TESTCTRL_00, i_test_LNAAGC, 0x5);
	ReadReg0 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_LNAAGC_READBUS_00);
	ReadReg0 = (ReadReg0>>23) & 0x0000001F;
	h->pTunerStatus->Tuner_LNA_Gain_Code = ReadReg0;  /*This value is used farther down to get gain in db*/

	/*RFVGA AGC level*/
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither, 0x1);
	h->pTunerStatus->RFAGC_indirect_addr = 0x5;
	BTNR_P_TunerRFAGCIndirectRead(h);
	ReadReg0 = h->pTunerStatus->RFAGC_indirect_data;
	h->pTunerStatus->Tuner_RFVGA_Gain_Code = ReadReg0; /*This value is used farther down to get gain in db*/
	if (ReadReg1 == 0)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither, 0x0);
	}
	if ((((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF) == 0x0000FFFF) && (h->pTunerStatus->Tuner_LNA_Gain_Code == 0x1D)) {h->pTunerStatus->MAX_gain_code = 1;}
	if (((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF) > 0x0000FF00) {h->pTunerStatus->RFAGC_MAX = 1;}
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetDither()  This routine sets the tuner AGC TOP
 ******************************************************************************************************************/
void BTNR_P_TunerSetDither(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerStatus->RFAGC_dither)
	{
	case 0:
		if (BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither) == 0x1)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither, 0x0);
			BDBG_MSG(("dithering off"));
		}
	break;
	case 1:
		if (BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither) == 0x0)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither, 0x1);
			BDBG_MSG(("dithering on"));
		}
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid BTNR_P_TunerSetDither"));
	}
}

/******************************************************************************
 BTNR_P_TunerAGCMonitor() - update the AGC
******************************************************************************/
void BTNR_P_TunerAGCMonitor(BTNR_3x7x_ChnHandle h)
{
	if (h->pTunerStatus->LO_index == 10) {BTNR_P_TunerReadIQIMB(h);}
	if (((h->pTunerStatus->g_IQ_phs_DEG)*(h->pTunerStatus->g_IQ_phs_DEG) > 50) && (h->pTunerStatus->LO_index == 10))
	{
		BTNR_P_TunerIQIMB(h);
	}
	switch (h->pTunerStatus->Tuner_Change_Settings)
	{
	case 0:
		/* smart dithering on & off algorithm */
		BTNR_P_InitStatus(h);
		if (h->pTunerParams->BTNR_Internal_Params.TunerInitComplete == true) {BTNR_P_TunerStatus(h);}
		h->pTunerStatus->EstChannelPower_dbm_i32 = -(int32_t)(h->pTunerStatus->Tuner_PreADC_Gain_x256db + h->pTunerParams->BTNR_Local_Params.PostADC_Gain_x256db + (h->pTunerStatus->External_FGLNA_Mode)*3584 - 1536);
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
		{
			h->pTunerStatus->LNA_TOP = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh);
			h->pTunerStatus->SNR_margin = h->pTunerStatus->EstChannelPower_dbm_i32 - h->pTunerStatus->Tuner_Sensitivity - h->pTunerStatus->LNA_atten;
			h->pTunerStatus->ACI_far = ((h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity < 6400) && (h->pTunerStatus->MAX_gain_code != 1) && (h->pTunerStatus->SNR_margin < 0)
			&& (h->pTunerStatus->LNA_TOP > (LNA_TOP_MAX-4)) && (h->pTunerStatus->RFAGC_MAX == 1)) ? 1 : 0;/* N+-5~inf; desired power < sensitivity + 25 */
			h->pTunerStatus->ACI_near = ((h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity < 6400) && (h->pTunerStatus->VGA_atten-h->pTunerStatus->SNR_margin > -400)) ? 1 : 0;/* N+-1~4 */
			h->pTunerStatus->TOP_DOWN_ACI = (((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin > 2304)) && (h->pTunerStatus->LNA_TOP > LNA_TOP_MIN)) ? 1 : 0;
			h->pTunerStatus->TOP_DOWN_ACI_BOOST = (((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin > 8960)) && (h->pTunerStatus->LNA_TOP > LNA_TOP_MIN)) ? 1 : 0;
			h->pTunerStatus->TOP_UP_ACI = (((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin < 768)) && (h->pTunerStatus->LNA_TOP < LNA_TOP_MAX)) ? 1 : 0;
			h->pTunerStatus->TOP_UP_ACI_BOOST = (((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin < -2560)) && (h->pTunerStatus->LNA_TOP < LNA_TOP_MAX)) ? 1 : 0;
			h->pTunerStatus->TOP_DOWN_LIN = (((h->pTunerStatus->RFAGC_MAX == 0) || ((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin > 2048)) || (h->pTunerStatus->LNA_atten == 0))
			&& (h->pTunerStatus->LNA_TOP > LNA_TOP_MIN)) ? 1 : 0;
			h->pTunerStatus->TOP_DOWN_LIN_BOOST = (((h->pTunerStatus->LNA_atten != 0) && (h->pTunerStatus->SNR_margin > 8960)) && (h->pTunerStatus->LNA_TOP > LNA_TOP_MIN)) ? 1 : 0;
			h->pTunerStatus->TOP_UP_LIN = ((h->pTunerStatus->MAX_gain_code != 1) && (h->pTunerStatus->SNR_margin < -1024) && (h->pTunerStatus->LNA_TOP < LNA_TOP_MAX)) ? 1 : 0;
			h->pTunerStatus->TOP_UP_LIN_BOOST = ((h->pTunerStatus->MAX_gain_code != 1) && (h->pTunerStatus->SNR_margin < -2560) && (h->pTunerStatus->LNA_TOP < LNA_TOP_MAX)) ? 1 : 0;
		}
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial) {BTNR_P_Ods_TunerDemodInfo(h);}
		if ((h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity) && (h->pTunerStatus->MAX_gain_code == 1))
		{

			if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
			{
				if ((h->pTunerStatus->EstChannelPower_dbm_i32 > -16640) && (h->pTunerStatus->RFAGC_dither == 0)) {h->pTunerStatus->RFAGC_dither = 1; BTNR_P_TunerSetDither(h);}
				if ((h->pTunerStatus->EstChannelPower_dbm_i32 < -17920) && (h->pTunerStatus->RFAGC_dither == 1)) {h->pTunerStatus->RFAGC_dither = 0; BTNR_P_TunerSetDither(h);}
			}
			if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
			{
				if ((h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity > 512) && (h->pTunerStatus->RFAGC_dither == 0)) {h->pTunerStatus->RFAGC_dither = 1; BTNR_P_TunerSetDither(h);}
				if ((h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity < 0) && (h->pTunerStatus->RFAGC_dither == 1)) {h->pTunerStatus->RFAGC_dither = 0; BTNR_P_TunerSetDither(h);}
			}
		}
		else
		{
			if (h->pTunerStatus->RFAGC_dither == 0) {h->pTunerStatus->RFAGC_dither = 1; BTNR_P_TunerSetDither(h);}
		}
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
		{BTNR_P_Ods_TunerBiasSelect(h); BTNR_P_Ods_TunerSetTOPSmooth(h);}
/*		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable) {BTNR_P_Ads_TunerBiasSelect(h);}*/
		BTNR_P_TunerSetExternalFGLNA(h);
	break;
	case 1:
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
		{
			BTNR_P_Ads_TunerSetTilt(h);
			BTNR_P_Ads_TunerSetRFFILSelect(h);
			if (h->pTunerParams->BTNR_Internal_Params.RFFIL_Select == BTNR_Internal_Params_TunerRFFIL_eTRKFIL)
			{
				BTNR_P_TunerSetRFFIL(h);
				BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_band_ctrl, h->pTunerStatus->Tuner_RFFIL_band);
				BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BW_tuning, h->pTunerStatus->Tuner_RFFIL_tune);
			}
			BTNR_P_TunerSetRFFILSwitch(h);
			BTNR_P_Ads_TunerSetBiasSmooth(h);
		}
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
		{
			BTNR_P_Ods_TunerSetTilt(h);
			BTNR_P_Ods_TunerSetBiasSmooth(h);
		}
	break;
	case 2:
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
			BTNR_P_Ads_TunerSetRdegSmooth(h);
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
			BTNR_P_Ods_TunerSetRdegSmooth(h);
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerStatus->Tuner_Change_Settings, value received is %d",h->pTunerStatus->Tuner_Change_Settings));
	}
}

/*******************************************************************************************
 * BTNR_P_Calculate_RFout_Gains()	This routine calculates RF to Loopthrough and Daisy Gains
 *******************************************************************************************/
BERR_Code BTNR_P_Calculate_RFout_Gains(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t ReadReg0;
	#ifdef NEW_GAIN_CODE
	int32_t LNA_Gain, LNA_Ctrl_Bias_Correction, LNA_Tilt;
	int32_t idx, x, y1, y2, frequency;
	#else
	int32_t GainReg=0, GainCorrection;
	#endif

	/*Loop Through Gain Calculation*/
	h->pTunerParams->BTNR_Gain_Params.InternalGain_To_LT = 0;

	/*Daisy Gain Calculation*/
	switch (h->pTunerParams->BTNR_Daisy_Params.Daisy_Source)
	{
	case BTNR_Daisy_Source_eUHF:
		h->pTunerParams->BTNR_Gain_Params.InternalGain_To_Daisy = 0;
		break;
	case BTNR_Daisy_Source_eVHF:

		/* Get fresh AGC values for status calculations */
		BTNR_P_TunerAGCReadBack(h);

		#ifndef NEW_GAIN_CODE
		/*LNA Gain*/
		/*This was derived from the Gain curve excel file from David*/
		/*=(290*Gain-4080-256*Correction)*/
		/*correction is 0 db for 100 MHz, 0.5 db for 200,300,400,500 MHz, 1.5 db for 600,700 MHz, 2.0 db for 800 MHz, 2.5 db for 800,1000 MHz*/
		ReadReg0 = (int32_t)h->pTunerStatus->Tuner_LNA_Gain_Code;
		if (h->pTunerStatus->Tuner_RF_Freq < 150000000)
		{
			GainCorrection = 0; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 250000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 350000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 450000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 550000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 650000000)
		{
			GainCorrection = -396; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 750000000)
		{
			GainCorrection = -396; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 850000000)
		{
			GainCorrection = -512; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 950000000)
		{
			GainCorrection = -640; /*unit is 256*db*/
		}
		else
		{
			GainCorrection = -640; /*unit is 256*db*/
		}
		GainReg += 256*(int32_t)ReadReg0-2560 + GainCorrection;
		/*printf("%d,", GainReg/256);*/


		/*LNA Bias Correction*/
		ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias);
		if (ReadReg0 == 1)
		{
		GainReg -= 1024;
		}
		/*printf("%d,", GainReg/256);*/

		/*Assign value of Daisy Gain*/
		h->pTunerParams->BTNR_Gain_Params.InternalGain_To_Daisy = GainReg;
		#endif

		#ifdef NEW_GAIN_CODE
		/*LNA Gain*/
		ReadReg0 = (int32_t)h->pTunerStatus->Tuner_LNA_Gain_Code; /*LNA Gain Code*/
		frequency = h->pTunerParams->BTNR_Gain_Params.Frequency;
		idx = (frequency/100000000);
		switch(idx)
		{
			case 0 :
				y1 = LNAcode[idx][ReadReg0]; y2 = LNAcode[idx][ReadReg0];
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				y1 = LNAcode[idx-1][ReadReg0]; y2 = LNAcode[idx][ReadReg0];
				break;
			default:
				y1 = LNAcode[7][ReadReg0]; y2 = LNAcode[7][ReadReg0];
		}
		x = (frequency/100000 - idx*1000);
		LNA_Gain = 256*16 + (y1*1000 + (y2-y1)*x)/(1000);

		/*LNA_Bias*/
		ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias);
		LNA_Ctrl_Bias_Correction = LNA_Ctrl_Bias[ReadReg0];

		/*LNA_Tilt*/
		ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_tilt);
		LNA_Tilt = (ReadReg0 == 0) ? 0 : -230;  /*unit is 256*db*/

		/*Assign value of Daisy Gain*/
		h->pTunerParams->BTNR_Gain_Params.InternalGain_To_Daisy = LNA_Gain + LNA_Ctrl_Bias_Correction + LNA_Tilt ;

		BDBG_MSG(("****** RFout Gain Calculations ******"));
		BDBG_MSG(("Gain_To_Daisy = %d ; LNA_Gain = %d ; LNA_Ctrl_Bias=%d ; LNA_Tilt = %d ", h->pTunerParams->BTNR_Gain_Params.InternalGain_To_Daisy*100/256, LNA_Gain*100/256,LNA_Ctrl_Bias_Correction*100/256,LNA_Tilt*100/256 ));
		BDBG_MSG(("---------------------------------"));
		#endif

		break;
	case BTNR_Daisy_Source_eUHF_VHF:
		h->pTunerParams->BTNR_Gain_Params.InternalGain_To_Daisy = 0;
		break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Daisy_Params.Daisy_Source, value received is %d",h->pTunerParams->BTNR_Daisy_Params.Daisy_Source));
		retCode = BERR_INVALID_PARAMETER;
	}

	BDBG_MSG(("BTNR_P_Calculate_RFout_Gains() Complete"));
   return retCode;
}

/******************************************************************************
 BTNR_P_TunerStatus() - get the status structure
******************************************************************************/
BERR_Code BTNR_P_TunerStatus(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t ReadReg0, ReadReg1, ReadReg2, ReadReg3;
	int32_t GainReg=0;
	uint8_t PreDivRatio, M, N , ndiv, pdiv;
	uint32_t ulMultA, ulMultB, ulDivisor, ulNrmHi, ulNrmLo;
	uint16_t temp;

	#ifdef NEW_GAIN_CODE
	int32_t Tuner_Gain, SDADC_PGA_Gain;
	int32_t LNA_Gain, RFVGA_Gain, MXR_Fga_Gain, FREQ_rsp=0, Q_rsp=0, MXR_rsp=0, RFFIL_rsp=0, freq_MHz=0;
	int32_t LNA_Ctrl_Bias_Correction, RFVGA_Ctrl_Rdeg_Correction, RFVGA_Ctrl_Bias_Correction, TRKFIL_Buf_I_Ctrl_Correction, MIXER_Bias_Ctrl_Correction, FGA_Higain_Correction;
	int32_t LNA_Tilt, RFVGA_Tilt;
	int32_t idx, x, y1, y2, frequency, dx_lut=4096, idx_lut;
	int8_t MXR_sel, MXR_HRM_mode, MXR_MUX_sel, MXR_SR, RFFIL_bank;
	#else
	int32_t GainCorrection;
	#endif

	/* Get fresh AGC values for status calculations */
	BTNR_P_TunerAGCReadBack(h);

	/*RefPLL Lock*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_04, o_REFPLL_lock);
	h->pTunerStatus->Tuner_Ref_Lock_Status  = (ReadReg0 == 1) ? BTNR_Status_eLock : BTNR_Status_eUnlock;

	/*PhyPLL Lock*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_04, o_PHYPLL_lock);
	h->pTunerStatus->Tuner_Phy_Lock_Status  = (ReadReg0 == 1) ? BTNR_Status_eLock : BTNR_Status_eUnlock;

	/*MixerPLL Lock*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_07, MXRPLL_FREQ_LOCK);
	h->pTunerStatus->Tuner_Mixer_Lock_Status  = (ReadReg0 == 1) ? BTNR_Status_eLock : BTNR_Status_eUnlock;

	/*Ref Freq*/
	h->pTunerStatus->Tuner_Ref_Freq = REF_FREQ;

	/*REFPLL_FREQ = REF_FREQ/REF_OUTDIV_m0*REF_DIV_fb/REF_DIV_ratio = 1350 MHz*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_01, REF_DIV_fb);    /*REF_DIV_fb*/
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_02, REF_DIV_ratio); /*REF_DIV_ratio*/
	ReadReg1 = (ReadReg1 >= 1) ? ReadReg1 : 8+ReadReg1;
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_OUTDIV_m0); /*REF_OUTDIV_m0*/

	h->pTunerStatus->Tuner_RefPll_Freq = (REF_FREQ*ReadReg0)/(ReadReg1*ReadReg2);

	/*PHYPLL1_FREQ=REF_FREQ*(ndiv_int+ndiv_frac/2^24)/p1div]/m1div, m1div is per channel*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int);   /*ndiv_int*/
	ReadReg1 = (ReadReg1 >= 10) ? ReadReg1 : 512+ReadReg1;
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_02, ndiv_frac);  /*ndiv_frac*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div);      /*p1div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 16+ReadReg2;
	ReadReg0 = (REF_FREQ*ReadReg0);
	ReadReg1 = (REF_FREQ*ReadReg1)/POWER2_24;
	ReadReg0 = (ReadReg0 + ReadReg1)/ReadReg2;

	/*now for each channels individual divider*/
	/*PHYPLL1_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m1div);      /*m1div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll1_Freq = ReadReg0/ReadReg2;
	/*PHYPLL2_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m2div);      /*m2div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll2_Freq = ReadReg0/ReadReg2;
	/*PHYPLL3_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m3div);      /*m3div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll3_Freq = ReadReg0/ReadReg2;
	/*PHYPLL4_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m4div);      /*m4div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll4_Freq = ReadReg0/ReadReg2;
	/*PHYPLL5_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m5div);      /*m5div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll5_Freq = ReadReg0/ReadReg2;
	/*PHYPLL6_Freq*/
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m6div);      /*m6div*/
	ReadReg2 = (ReadReg2 >= 1) ? ReadReg2 : 256+ReadReg2;
	h->pTunerStatus->Tuner_PhyPll6_Freq = ReadReg0/ReadReg2;


	/* The following is for the callbacak function to ADS*/
	/*To get the RF Frequency we need to get the FCW in the DDFS */
	/*as well as the parameters in the UFE_AFE_TNR0_MXR_01 register*/

	/*Get pre-div ratio*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_div_ratio);
	PreDivRatio = 1<<ReadReg0;

	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_HRM_mode);
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MXR_SR6p8p12p16p);
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_MUX0p6p8p);

	/*GainReg is used later for callback status*/
	if ((ReadReg0 == 0) && (ReadReg2 == 1))
	{
		M = PreDivRatio * 1;
		GainReg = 6912;
	}
	else if ((ReadReg0 == 1) && (ReadReg1 == 0) && (ReadReg2 == 2))
	{
		M = PreDivRatio * 6;
		GainReg = 6659;  /*unit is 256*db*/
	}
	else if ((ReadReg0 == 1) && (ReadReg1 == 1) && (ReadReg2 == 3))
	{
		M = PreDivRatio * 8;
		GainReg = 6528;  /*unit is 256*db*/
	}
	else if ((ReadReg0 == 1) && (ReadReg1 == 2))
	{
		M = PreDivRatio * 12;
		GainReg = 6528;  /*unit is 256*db*/
	}
	else if ((ReadReg0 == 1) && (ReadReg1 == 3))
	{
		M = PreDivRatio * 16;
		GainReg = 6528;  /*unit is 256*db*/
	}
	else
	{
		BDBG_MSG(("ERROR!!!!! Invalid parameters read fron IC in BTNR_P_TunerStatus()"));
		M = 1;
	}

	N = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_fbdivn_1p0);

	/*Get FCW*/
	ReadReg0 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_DDFS_FCW_02);      /*32 MSB of 37 bit word*/
	ReadReg1 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_DDFS_FCW_01);      /* 5 LSB of 37 bit word*/
	ReadReg1 = (ReadReg1 & 0x0000001F);

	/*Fc = (RFPLL_FREQ*(temp/ndiv/pdiv)/128)*N)*FCW/(M*2^37)*/
	/*Fc = (RFPLL_FREQ*(temp/ndiv/pdiv)/128)*N)/2^8*FCW/(M*2^29)*/
	/*largest N is 24*/
#if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_01, REF_DIV_fb);
	pdiv = 1;
	ndiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_02, REF_DIV_ratio);
#elif (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int);
	pdiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div);
	ndiv = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m2div);
#endif

	ulMultA = REF_FREQ*(temp/ndiv/pdiv);
	ulMultB = N;
	ulDivisor = 256;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo,  ulDivisor, &ulNrmHi, &ulNrmLo);

	ulMultA = (ReadReg0 & 0xF8000000) >> 27;
	ulMultB = ((ReadReg0 & 0x07FFFFFF)<<5) | (ReadReg1 & 0x0000001F);
	ulDivisor = M;
	if (ulDivisor == 0)
	{
		BDBG_ERR(("ERROR!!!!! ulDivisor == 0 in BTNR_P_TunerStatus()"));
	}
	/*BMTH_HILO_64TO64_Div32(ulMultA, ulMultB,  ulDivisor, &ulMultA, &ulMultB);*/
	BMTH_HILO_64TO64_Mul(ulMultA, ulMultB, ulNrmHi, ulNrmLo, &ulMultA, &ulMultB);
	BMTH_HILO_64TO64_Div32(ulMultA, ulMultB,  ulDivisor, &ulNrmHi, &ulNrmLo);

	/*Get bits 60:29*/
	ulNrmLo = (ulNrmHi*POWER2_3 & 0xFFFFFFF8) | (ulNrmLo/POWER2_29 & 0x0000007);

	/*Assign value*/
	h->pTunerStatus->Tuner_RF_Freq = ulNrmLo;

	#ifndef NEW_GAIN_CODE
	/***********************************************************/
	/*Get PreADC Gain*/
	/*PreADC Gain = LNA + RFVGA + RFFIL + Mixer_FGA + SDADC_PGA*/
	/*printf("\n\t FGA,SDADC,RFFIl,LNA,RFVGA,LNACorr,RFVGACorr:");*/

	/*Mixer_FGA Gain*/
	/*The GainReg is set above and now must be scaled based on high gain*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_HiGain);
	GainReg = (ReadReg0 == 0) ? GainReg - 768 : GainReg;  /*unit is 256*db*/
	/*printf("%d,", GainReg/256);*/

	/*SDADC_PGA Gain is 0, 2, 4, 6 ... up to 14 db*/
	ReadReg0 = BREG_ReadField(h->hRegister, SDADC_CTRL_SYS0, i_ctl_adc_gain);
	GainReg -= 512*ReadReg0; /*unit is 256*db*/
	/*printf("%d,", GainReg/256);*/

	/*RFFIL Gain is either 0 or 2 db*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen);
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en);
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL_BUF);
	ReadReg3 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL);

	if (((ReadReg0 == 1) && (ReadReg1 == 0) && (ReadReg2 == 1) && (ReadReg3 == 0)) || ((ReadReg0 == 0) && (ReadReg1 == 1) && (ReadReg2 == 1) && (ReadReg3 == 0)))
	{
		GainReg += 0; /*unit is 256*db*/
	}
	else if ((ReadReg0 == 0) && (ReadReg1 == 0) && (ReadReg2 == 1) && (ReadReg3 == 1))
	{
	  GainReg -= 512; /*unit is 256*db*/
	}
	else
	{
		BDBG_ERR(("ERROR!!!!! INVALID COMBINATION OF VALUES RECEIVED IN RFFIL Gain calculation in BTNR_P_TunerStatus())"));
		GainReg += 0; /*unit is 256*db*/
	}
	/*printf("%d,", GainReg/256);*/

	/*LNA Gain*/
	if (h->pTunerParams->BTNR_RF_Input_Mode != BTNR_3x7x_TunerRfInputMode_eExternalLna) /*Don't add LNA gain for external LNA case*/
		{

		/*This was derived from the Gain curve excel file from David*/
		/*=(290*Gain-4080-256*Correction)*/
		/*correction is 0 db for 100 MHz, 0.5 db for 200,300,400,500 MHz, 1.5 db for 600,700 MHz, 2.0 db for 800 MHz, 2.5 db for 800,1000 MHz*/
		ReadReg0 = (int32_t)h->pTunerStatus->Tuner_LNA_Gain_Code;
		if (h->pTunerStatus->Tuner_RF_Freq < 150000000)
		{
			GainCorrection = 0; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 250000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 350000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 450000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 550000000)
		{
			GainCorrection = -128; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 650000000)
		{
			GainCorrection = -396; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 750000000)
		{
			GainCorrection = -396; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 850000000)
		{
			GainCorrection = -512; /*unit is 256*db*/
		}
		else if (h->pTunerStatus->Tuner_RF_Freq < 950000000)
		{
			GainCorrection = -640; /*unit is 256*db*/
		}
		else
		{
			GainCorrection = -640; /*unit is 256*db*/
		}
		GainReg += 256*(int32_t)ReadReg0-2560 + GainCorrection;
		/*printf("%d,", GainReg/256);*/
	}

	/*RFVGA Gain*/
	/*This was derived from lab tests*/
	ReadReg0 = h->pTunerStatus->Tuner_RFVGA_Gain_Code;
	ReadReg0 = (ReadReg0>>16) & 0x0000FFFF;
	GainReg += -3840;
	GainReg += (ReadReg0 > 0xD200) ? 256 : 0;
	GainReg += (ReadReg0 > 0xc900) ? 256 : 0;
	GainReg += (ReadReg0 > 0xc100) ? 256 : 0;
	GainReg += (ReadReg0 > 0xb900) ? 256 : 0;
	GainReg += (ReadReg0 > 0xb200) ? 256 : 0;
	GainReg += (ReadReg0 > 0xaa00) ? 256 : 0;
	GainReg += (ReadReg0 > 0xa000) ? 256 : 0;
	GainReg += (ReadReg0 > 0x8f00) ? 256 : 0;
	GainReg += (ReadReg0 > 0x6500) ? 256 : 0;
	GainReg += (ReadReg0 > 0x5c00) ? 256 : 0;
	GainReg += (ReadReg0 > 0x5500) ? 256 : 0;
	GainReg += (ReadReg0 > 0x5000) ? 256 : 0;
	GainReg += (ReadReg0 > 0x4c00) ? 256 : 0;
	GainReg += (ReadReg0 > 0x4800) ? 256 : 0;
	GainReg += (ReadReg0 > 0x4400) ? 256 : 0;
	GainReg += (ReadReg0 > 0x4000) ? 256 : 0;
	GainReg += (ReadReg0 > 0x3c00) ? 256 : 0;
	GainReg += (ReadReg0 > 0x3800) ? 256 : 0;
	GainReg += (ReadReg0 > 0x3400) ? 256 : 0;
	GainReg += (ReadReg0 > 0x3000) ? 256 : 0;
	GainReg += (ReadReg0 > 0x2a00) ? 256 : 0;
	/*printf("%d,", GainReg/256);*/

	if (h->pTunerParams->BTNR_RF_Input_Mode != BTNR_3x7x_TunerRfInputMode_eExternalLna)
	{
		/*LNA Bias Correction*/
		ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias);
		if (ReadReg0 == 1)
		{
		GainReg -= 1024;
		}
		/*printf("%d,", GainReg/256);*/
	}

	/*RFVGA Gm Degeneration Correction*/
	/*000 = -0.75 dB
	001 = +2.00 dB
	010 = +3.50 dB
	011 = +5.00 dB
	100 = +6.00 dB
	101 = +7.00 dB
	110 = +7.50 dB*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg);
	if (ReadReg0 == 0)
	{	GainReg -= 192;	}
	else if (ReadReg0 == 1)
	{	GainReg += 512;	}
	else if (ReadReg0 == 2)
	{	GainReg += 896;	}
	else if (ReadReg0 == 3)
	{	GainReg += 1280;	}
	else if (ReadReg0 == 4)
	{	GainReg += 1536;	}
	else if (ReadReg0 == 5)
	{	GainReg += 1792;	}
	else if (ReadReg0 == 6)
	{	GainReg += 1920;	}
	else if (ReadReg0 == 7)
	{	GainReg += 2048;	}
	else
	{	GainReg += 2048;	}
	/*printf("%d \n", GainReg/256);*/

	/*Assign value*/
	h->pTunerStatus->Tuner_PreADC_Gain_x256db = (int16_t)GainReg;
	#endif

	#ifdef NEW_GAIN_CODE
	/***********************************************************/
	/*Get PreADC Gain*/
	/*PreADC Gain = Tuner Gain + SDADC_PGA Gain*/
	/*Tuner_Gain = LNA + RFVGA + Mixer_FGA + Freq_rsp + LNA_Bias + RFVGA_Rdeg + RFVGA_Bias + TRKFIl_Bias + MIXER_Bias + FGA_HiGain + LNA_Tilt + RFVGA_Tilt */
	/*LNA Gain*/
	ReadReg0 = (int32_t)h->pTunerStatus->Tuner_LNA_Gain_Code; /*LNA Gain Code*/
	frequency = h->pTunerStatus->Tuner_RF_Freq;
	idx = (frequency/100000000);
	switch(idx)
	{
		case 0 :
			y1 = LNAcode[idx][ReadReg0]; y2 = LNAcode[idx][ReadReg0];
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			y1 = LNAcode[idx-1][ReadReg0]; y2 = LNAcode[idx][ReadReg0];
			break;
		default:
			y1 = LNAcode[7][ReadReg0]; y2 = LNAcode[7][ReadReg0];
	}
	x = (frequency/100000 - idx*1000);
	LNA_Gain = 256*16 + (y1*1000 + (y2-y1)*x)/(1000);
	h->pTunerStatus->LNA_atten = -(y1*1000 + (y2-y1)*x)/(1000);

	/*RFVGA Gain*/
	idx = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg); /*RFVGA Rdeg*/
	ReadReg0 = h->pTunerStatus->Tuner_RFVGA_Gain_Code; /*RFVGA Gain Code*/
	ReadReg0 = (ReadReg0>>16) & 0x0000FFFF;
	idx_lut = ReadReg0/dx_lut;
	y1 = RFVGAcode[idx][idx_lut];
	y2 = RFVGAcode[idx][idx_lut+1];
	if (h->pTunerStatus->RFVGA_byp == 1) {RFVGA_Gain = 900;}
	else if (h->pTunerStatus->RFVGA_byp == 0) {RFVGA_Gain = 10*256 + (y1*dx_lut + (y2-y1)*( (int)ReadReg0-idx_lut*dx_lut))/dx_lut;}
	else {RFVGA_Gain = -25600;}
	h->pTunerStatus->VGA_atten =  2560 - RFVGA_Gain;

	/*Mixer FGA Gain*/
	MXR_Fga_Gain = 27*256;

	freq_MHz = frequency/1000000;
	/*Frequency Response (Combined freq. response of all blocks + Mixer Phase comp'n selected based on freq*/
	if ((freq_MHz > 520) && (freq_MHz < 580))
	{
		idx = (freq_MHz - 520)/5;
		Q_rsp = -( FREQ_Rsp_Q[idx]*1000 + (FREQ_Rsp_Q[idx+1]-FREQ_Rsp_Q[idx])*(freq_MHz-520-idx*5)/5)/1000;
	}
	else {Q_rsp = 0;}

	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen);
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en);
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL_BUF);
	ReadReg3 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL);
	MXR_sel = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel);
	MXR_HRM_mode = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_HRM_mode);
	MXR_MUX_sel = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MIXER_sel_MUX0p6p8p);
	MXR_SR = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_01, i_MXR_SR6p8p12p16p);
	if ((MXR_sel == 1) && (MXR_HRM_mode == 0) && (MXR_MUX_sel == 1)) {MXR_rsp = 205;}
	else if ((MXR_sel == 1) && (MXR_HRM_mode == 1) && (MXR_MUX_sel == 2) && (MXR_SR == 0)) {MXR_rsp = -162;}
	else if ((MXR_sel == 1) && (MXR_HRM_mode == 1) && (MXR_MUX_sel == 3) && (MXR_SR == 1)) {MXR_rsp = -213;}
	else if ((MXR_sel == 2) && (MXR_HRM_mode == 1) && (MXR_SR == 2)) {MXR_rsp = -308;}
	else if ((MXR_sel == 3) && (MXR_HRM_mode == 1) && (MXR_SR == 3)) {MXR_rsp = -308;}
	else {BDBG_ERR(("ERROR!!!!! Invalid combination of values received in HRM"));}
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_MoCATRAP_swen);
	ReadReg1 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_bypass_en);
	ReadReg2 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL_BUF);
	ReadReg3 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_TRKFIL);
	RFFIL_bank = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_band_ctrl);
	if ((ReadReg2 == 1) && (ReadReg3 == 0) && (ReadReg0 == 1)) {RFFIL_rsp = -(63*(freq_MHz-340))/100;}
	else if ((ReadReg2 == 1) && (ReadReg3 == 1) && (ReadReg0 == 0) && (ReadReg1 == 0))
	{
		if (RFFIL_bank == 0) {RFFIL_rsp = -159;}
		else if (RFFIL_bank == 1)
		{
			if ((frequency > 262000000) && (frequency <= 355000000)) {RFFIL_rsp = -998-(581*(freq_MHz-260))/100;}
			else {RFFIL_rsp = -159-(108*(freq_MHz-150))/100;}
		}
		else if (RFFIL_bank == 2) {RFFIL_rsp = -461-(258*(freq_MHz-360))/100;}
		else  {BDBG_ERR(("ERROR!!!!! Invalid combination of values received in tracking filter"));}
	}
	else if ((ReadReg2 == 1) && (ReadReg3 == 0) && (ReadReg0 == 0) && (ReadReg1 == 1)) {RFFIL_rsp = 128;}
	else {BDBG_ERR(("ERROR!!!!! Invalid combination of values received in RFFIL"));}
	FREQ_rsp = Q_rsp + MXR_rsp + RFFIL_rsp;

	/*LNA_Bias*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias);
	LNA_Ctrl_Bias_Correction = LNA_Ctrl_Bias[ReadReg0];

	/*RFVGA_Rdeg*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg);
	RFVGA_Ctrl_Rdeg_Correction = (h->pTunerStatus->RFVGA_byp == 1) ? 0 : RFVGA_Ctrl_Rdeg[ReadReg0];

	/*RFVGA_Bias*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias);
	RFVGA_Ctrl_Bias_Correction = (h->pTunerStatus->RFVGA_byp == 1) ? 0 : RFVGA_Ctrl_Bias[ReadReg0];

	/*TRKFIl_Bias*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl);
	TRKFIL_Buf_I_Ctrl_Correction = TRKFIL_Buf_I_Ctrl[ReadReg0];

	/*MIXER_Bias*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl);
	MIXER_Bias_Ctrl_Correction = MIXER_Bias_Ctrl[ReadReg0];

	/*FGA_HiGain*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_HiGain);
	FGA_Higain_Correction = (ReadReg0 == 0) ? -655 : 0;  /*unit is 256*db*/

	/*LNA_Tilt*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_tilt);
	LNA_Tilt = (ReadReg0 == 0) ? 0 : -230;  /*unit is 256*db*/

	/*RFVGA_Tilt*/
	ReadReg0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_tilt);
	RFVGA_Tilt = (ReadReg0 == 0) ? 0 : -358;  /*unit is 256*db*/

	/*Assign value for Tuner_Gain*/
	if (h->pTunerParams->BTNR_RF_Input_Mode != BTNR_3x7x_TunerRfInputMode_eExternalLna)
	{
		Tuner_Gain = LNA_Gain + RFVGA_Gain + MXR_Fga_Gain + FREQ_rsp + LNA_Ctrl_Bias_Correction + RFVGA_Ctrl_Rdeg_Correction + RFVGA_Ctrl_Bias_Correction + TRKFIL_Buf_I_Ctrl_Correction + MIXER_Bias_Ctrl_Correction + FGA_Higain_Correction + LNA_Tilt + RFVGA_Tilt;
		/*BDBG_MSG(("******  Gain Calculations ******"));
		BDBG_MSG(("LNA_Gain = %d ; RFVGA_Gain = %d ; MXR_Fga_Gain=%d ; FREQ_rsp = %d ",LNA_Gain*100/256, RFVGA_Gain*100/256, MXR_Fga_Gain*100/256,FREQ_rsp*100/256 ));
		BDBG_MSG(("LNA_Ctrl_Bias = %d ; RFVGA_Ctrl_Rdeg = %d ; RFVGA_Ctrl_Bias=%d ; RFFIL_Buffer_bias = %d ; MIXER_Bias = %d ",LNA_Ctrl_Bias_Correction*100/256,RFVGA_Ctrl_Rdeg_Correction*100/256,RFVGA_Ctrl_Bias_Correction*100/256, TRKFIL_Buf_I_Ctrl_Correction*100/256, MIXER_Bias_Ctrl_Correction*100/256 ));
		BDBG_MSG(("FGA_Higain = %d ; LNA_Tilt = %d ; RFVGA_Tilt=%d ",FGA_Higain_Correction*100/256, LNA_Tilt*100/256, RFVGA_Tilt*100/256 ));*/
	}
	else
	{
		Tuner_Gain = RFVGA_Gain + MXR_Fga_Gain + FREQ_rsp + RFVGA_Ctrl_Rdeg_Correction + RFVGA_Ctrl_Bias_Correction + TRKFIL_Buf_I_Ctrl_Correction + MIXER_Bias_Ctrl_Correction + FGA_Higain_Correction + RFVGA_Tilt;
		/*BDBG_MSG(("******  Gain Calculations ******"));
		BDBG_MSG(("LNA_Gain = %d ; RFVGA_Gain = %d ; MXR_Fga_Gain=%d ; FREQ_rsp = %d ",0, RFVGA_Gain*100/256, MXR_Fga_Gain*100/256,FREQ_rsp*100/256 ));
		BDBG_MSG(("LNA_Ctrl_Bias = %d ; RFVGA_Ctrl_Rdeg = %d ; RFVGA_Ctrl_Bias=%d ; RFFIL_Buffer_bias = %d ; MIXER_Bias_Ctrl = %d ",0,RFVGA_Ctrl_Rdeg_Correction*100/256,RFVGA_Ctrl_Bias_Correction*100/256, TRKFIL_Buf_I_Ctrl_Correction*100/256, MIXER_Bias_Ctrl_Correction*100/256 ));
		BDBG_MSG(("FGA_Higain = %d ; LNA_Tilt = %d ; RFVGA_Tilt=%d ",FGA_Higain_Correction*100/256, 0, RFVGA_Tilt*100/256 ));*/
	}
	h->pTunerStatus->TNR_backoff =  LNA_Gain + RFVGA_Gain + LNA_Ctrl_Bias_Correction + RFVGA_Ctrl_Rdeg_Correction + RFVGA_Ctrl_Bias_Correction - 256*26;
	/*Assign value for SDADC_PGA_Gain*/
	/*SDADC_PGA Gain is 0, 2, 4, 6 ... up to 14 db*/
	ReadReg0 = BREG_ReadField(h->hRegister, SDADC_CTRL_SYS0, i_ctl_adc_gain);
	SDADC_PGA_Gain = -512*ReadReg0;

	/*Assign value for PreADC_Gain */
	h->pTunerStatus->Tuner_PreADC_Gain_x256db = (int16_t)(Tuner_Gain + SDADC_PGA_Gain);

	/*BDBG_MSG(("Total_PreADC_Gain = %d ; Tuner_Gain = %d ; SDADC_PGA_Gain=%d",h->pTunerStatus->Tuner_PreADC_Gain_x256db*100/256, Tuner_Gain*100/256, SDADC_PGA_Gain*100/256));
	BDBG_MSG(("---------------------------------"));*/
	#endif

	/*Zero internal gain for Low-IF mode */
	if (h->pTunerParams->BTNR_RF_Input_Mode == BTNR_3x7x_TunerRfInputMode_eLowIf)
	{
	  h->pTunerStatus->Tuner_PreADC_Gain_x256db = 0;
	}


	/***********************************************************/
	/*Get External Gain*/
	/* External gain = External Gain Total - External Gain Bypassable (If LNA bypassed)*/

	/*External Gain Total*/
	h->pTunerStatus->External_Gain_x256db = h->pTunerParams->BTNR_Gain_Params.ExternalGain_Total;

	/*External Gain Bypassable*/
	if (h->pTunerStatus->External_FGLNA_Mode == 0) /* External LNA bypassed - subtract bypassable gain and enable bypassed flag*/
	{
		/*h->pTunerParams->BTNR_Gain_Params.ExternalGain_Bypassable = 16*256;*/
		h->pTunerStatus->External_Gain_x256db -= h->pTunerParams->BTNR_Gain_Params.ExternalGain_Bypassable;   /* in case of Bypass compenaste for the signal Strength */
		h->pTunerParams->BTNR_Gain_Params.Gain_Bypassed = 1; /*Enable the Gain Bypassed flag*/
	}
	else /*External LNA not bypassed - Disable the Gain Bypassed flag*/
	{
		h->pTunerParams->BTNR_Gain_Params.Gain_Bypassed = 0;
	}

/*	BDBG_MSG(("PreADC Gain = %d, ExternalGain = %d",h->pTunerStatus->Tuner_PreADC_Gain_x256db*100/256,h->pTunerStatus->External_Gain_x256db*100/256));*/
	return retCode;
}

/*******************************************************************************************
 * BTNR_P_TunerInit()		This routine initializes the tuner and is only run once
 *******************************************************************************************/
BERR_Code BTNR_P_TunerInit(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t temp_UFE_AFE_TNR0_PWRUP_01 = 0;
	uint32_t temp_UFE_AFE_TNR0_PWRUP_02 = 0;
	/* local variables */
	uint8_t PLL_Status, LockCount, temp;

	/*Initialize the BTNR_Internal_Params_t Structure
		 *these parameters are used locally by BBS to sent parameters into the tuner functions*/
	h->pTunerParams->BTNR_Internal_Params.LNA_Enable       = INIT_BBS_LNA_ENABLE;
	h->pTunerParams->BTNR_Internal_Params.RFFIL_Select     = INIT_BBS_RFFIL_SELECT;
	h->pTunerParams->BTNR_Internal_Params.HRC_Enable       = INIT_BBS_HRC_ENABLE;
	if (h->pTunerParams->BTNR_RF_Input_Mode == BTNR_3x7x_TunerRfInputMode_eLowIf)
	{
	  h->pTunerParams->BTNR_Internal_Params.SDADC_Input    = BTNR_Internal_Params_SDADC_Input_eExtReal;
    h->pTunerParams->BTNR_Internal_Params.IF_Freq        = h->pTunerParams->BTNR_Acquire_Params.RF_Freq;
	} else {
		h->pTunerParams->BTNR_Internal_Params.SDADC_Input    = INIT_BBS_SDADC_INPUT;
	  h->pTunerParams->BTNR_Internal_Params.IF_Freq        = INIT_BBS_IF_FREQ;
	}
	/*Initialize the BTNR_Local_Params_t Structure*/
	h->pTunerParams->BTNR_Local_Params.TunerCapCntl				 = 0;
	h->pTunerParams->BTNR_Local_Params.RF_Offset         = 0;
	h->pTunerParams->BTNR_Local_Params.Symbol_Rate         = 0;
	h->pTunerParams->BTNR_Local_Params.Total_Mix_After_ADC = (int16_t)0x8000;
	h->pTunerParams->BTNR_Local_Params.PostADC_Gain_x256db = (int16_t)0x8000;

	/* clear the lock status */
	h->pTunerStatus->Tuner_Ref_Lock_Status = BTNR_Status_eUnlock;
	h->pTunerStatus->Tuner_Mixer_Lock_Status = BTNR_Status_eUnlock;
	h->pTunerStatus->Tuner_Phy_Lock_Status = BTNR_Status_eUnlock;

	/*Get the REFPLL lock status*/
	PLL_Status = 0;
	LockCount = 0;
	while ((PLL_Status == 0) && (LockCount < REF_PLL_LOCK_TIMEOUT_MS))
	{
		BKNI_Sleep(1);
		PLL_Status = BREG_ReadField(h->hRegister,UFE_AFE_TNR_REFPLL_04, o_REFPLL_lock);
		switch (PLL_Status)
		{
		case 0 :
			if (BREG_ReadField(h->hRegister, UFE_AFE_TNR_PWRUP_01, REF_PLL_master_PWRUP) == 0)
				BDBG_MSG(("REFPLL is NOT used"));
			else
				BDBG_ERR(("REFPLL is NOT locked"));
			break;

		case 1 :
			BDBG_MSG(("REFPLL is locked"));
			break;
		default :
			BDBG_ERR(("ERROR!!! INVALID Reference PLL Locked Value: value is %d",PLL_Status));
			retCode = BERR_INVALID_PARAMETER;
			/*goto bottom of function to return error code*/
			goto something_bad_happened;
		}
		LockCount++;
	}

	/*Get the PHYPLL lock status*/
	PLL_Status = 0;
	LockCount = 0;
	while ((PLL_Status == 0) && (LockCount <PHY_PLL_LOCK_TIMEOUT_MS))
	{
		BKNI_Sleep(1);
		/*Get the PHYPLL lock status*/
		PLL_Status = BREG_ReadField(h->hRegister,UFE_AFE_TNR_REFPLL_04, o_PHYPLL_lock);
		switch (PLL_Status)
		{
		case 0 :
			temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, PHY_PLL_master_PWRUP);
			if (temp == 0)
				BDBG_ERR(("PHYPLL is NOT used"));
			else
				BDBG_ERR(("PHYPLL is NOT locked"));
			break;
		case 1 :
			BDBG_MSG(("PHYPLL is locked"));
			break;
		default :
			BDBG_ERR(("ERROR!!! INVALID PHY PLL Locked Value: value is %d",PLL_Status));
			retCode = BERR_INVALID_PARAMETER;
			/*goto bottom of function to return error code*/
			goto something_bad_happened;
		}
		LockCount++;
	}
	temp_UFE_AFE_TNR0_PWRUP_01 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01);
	temp_UFE_AFE_TNR0_PWRUP_02 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02);
	temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x000083E0);
	temp_UFE_AFE_TNR0_PWRUP_02 = (temp_UFE_AFE_TNR0_PWRUP_02 | 0x00087AFF);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, temp_UFE_AFE_TNR0_PWRUP_01);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, temp_UFE_AFE_TNR0_PWRUP_02);
	BKNI_Sleep(1);
	BTNR_P_TunerSetDCO(h);
	h->pTunerStatus->AGC_BW = 1;
	if (h->pTunerParams->BTNR_Acquire_Params.Application  == BTNR_TunerApplicationMode_eTerrestrial)
	{
		h->pTunerStatus->AGC_TOP = 0; h->pTunerStatus->LNA_TOP = LNA_TOP_MAX; BTNR_P_Ods_TunerSetAGCTOP(h);
		BTNR_P_Ods_TunerSetAGCBW(h);
	}
	if (h->pTunerParams->BTNR_Acquire_Params.Application  == BTNR_TunerApplicationMode_eCable)
	{
		BTNR_P_Ads_TunerSetAGCTOP(h);
		BTNR_P_Ads_TunerSetAGCBW(h);
	}
	h->pTunerStatus->rev = 0;
#if (BCHP_CHIP == 7563)
	h->pTunerStatus->rev = (BREG_Read32(h->hRegister, BCHP_JTAG_OTP_GENERAL_STATUS_4) >> 24) & 0x00000003;
#endif
	h->pTunerParams->BTNR_Internal_Params.TunerInitComplete = true;
	/*Initialization Complete*/
	BDBG_MSG(("BTNR_P_TunerInit() Complete"));

/*goto label to return error code if something bad happened above*/
something_bad_happened:
  return retCode;
}

#ifdef SmartTuneEnabled
/*******************************************************************************************
 * BTNR_P_TunerSmartTune()
 *******************************************************************************************/
static void BTNR_P_TunerSmartTune(BTNR_3x7x_ChnHandle h)
{
  bool ChangeFreqPlan = false;
  uint32_t RF_Freq = h->pTunerParams->BTNR_Acquire_Params.RF_Freq;
  if (((RF_Freq > (198000000-5000000)) && (RF_Freq < (198000000+5000000))) ||
      ((RF_Freq > (216000000-5000000)) && (RF_Freq < (216000000+5000000))) ||
	  ((RF_Freq > (400000000-5000000)) && (RF_Freq < (400000000+5000000))) ||
	  ((RF_Freq > (506000000-5000000)) && (RF_Freq < (506000000+5000000))) ||
      ((RF_Freq > (540000000-5000000)) && (RF_Freq < (540000000+5000000))) ||
	  ((RF_Freq > (600000000-5000000)) && (RF_Freq < (600000000+5000000))) ||
      ((RF_Freq > (675000000-5000000)) && (RF_Freq < (675000000+5000000))) ||
	  ((RF_Freq > (681500000-5000000)) && (RF_Freq < (681500000+5000000))) ||
	  ((RF_Freq > (756000000-5000000)) && (RF_Freq < (756000000+5000000))) ||
	  ((RF_Freq > (800000000-5000000)) && (RF_Freq < (800000000+5000000))) ||
      ((RF_Freq > (844000000-5000000)) && (RF_Freq < (844000000+5000000)))) {
    if (h->pTunerParams->BTNR_Local_Params.SmartTune != BTNR_Local_Params_SmartTune_FreqPlanA) {
      h->pTunerParams->BTNR_Local_Params.SmartTune = BTNR_Local_Params_SmartTune_FreqPlanA;
      ChangeFreqPlan = true;
    }
  } else {
    if (h->pTunerParams->BTNR_Local_Params.SmartTune != BTNR_Local_Params_SmartTune_FreqPlanDefault) {
      h->pTunerParams->BTNR_Local_Params.SmartTune = BTNR_Local_Params_SmartTune_FreqPlanDefault;
      ChangeFreqPlan = true;
    }
  }

  if (ChangeFreqPlan) {
	/*BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 1);*/
    switch (h->pTunerParams->BTNR_Local_Params.SmartTune) {
	  case BTNR_Local_Params_SmartTune_FreqPlanDefault :
	    BDBG_MSG(("SmartTune: Frequency Plan = Default"));
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int, 0x78);/*ndiv_int = 50*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m6div, 0x04);   /*m6div = 5*/

        /* for multi channel tuner, TM_SYS_PLL regs are offset (from channel 0's base) using hTMSysPllReg */
#if ( BTNR_P_BCHP_TNR_NUM_CORES < 2 ) /* (e.g. not dual tuner [3472]) */
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_PDIV, 2);
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_NDIV_INT, 80);
#else
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_PDIV, 2);
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_NDIV_INT, 80);
#endif
        /* TM_SYS_PLL_CLK_XPT_216 not channelized */
#if (BTNR_P_BCHP_TNR_CORE_VER < BTNR_P_BCHP_CORE_V_1_3)
        BREG_WriteField(h->hTnr->hRegister, TM_SYS_PLL_CLK_216, DIV, 10);
#else
        BREG_WriteField(h->hTnr->hRegister, TM_REG_PLL_CLK_XPT_216, DIV, 10);
#endif
		break;
	  case BTNR_Local_Params_SmartTune_FreqPlanA :
		BDBG_MSG(("SmartTune: Frequency Plan = A"));
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int, 114);/*ndiv_int = 38*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m6div, 4);    /*m6div = 4*/

        /* for multi channel tuner, TM_SYS_PLL regs are offset (from channel 0's base) using hTMSysPllReg */
#if ( BTNR_P_BCHP_TNR_NUM_CORES < 2 ) /* (e.g. not dual tuner [3472]) */
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_PDIV, 2);
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL_NDIV_INT, 76);
#else
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_PDIV, 2);
        BREG_Write32(h->hTMSysPllReg, BCHP_TM_SYS_PLL0_NDIV_INT, 76);
#endif
        /* TM_SYS_PLL_CLK_XPT_216 not channelized */
#if (BTNR_P_BCHP_TNR_CORE_VER < BTNR_P_BCHP_CORE_V_1_3)
        BREG_WriteField(h->hTnr->hRegister, TM_SYS_PLL_CLK_216, DIV, 10);
#else
        BREG_WriteField(h->hTnr->hRegister, TM_REG_PLL_CLK_XPT_216, DIV, 10);
#endif
		break;
	  default :
		BDBG_ERR(("SmartTune: Frequency Plan Invalid"));
    }
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x3f);
	/*BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 0);  */
	BTNR_P_TunerSetADC6B(h); h->pTunerStatus->LNAAGC_FRZ = 0;
  }
}
#endif

/*******************************************************************************************************************
 * BTNR_P_TunerSetSignalPath()  This routine sets the tuner for TOP, tilt, RFFIL mode, RFFIL switches, bias
 ******************************************************************************************************************/
void BTNR_P_TunerSetSignalPath(BTNR_3x7x_ChnHandle h)
{
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
	{
		BTNR_P_Ads_TunerSetTilt(h);
		BTNR_P_Ads_TunerSetRFFILSelect(h);
	}
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
	{
		BTNR_P_Ods_TunerSetTilt(h);
		BTNR_P_Ods_TunerSetRFFILSelect(h);
	}
	if (h->pTunerParams->BTNR_Internal_Params.RFFIL_Select == BTNR_Internal_Params_TunerRFFIL_eTRKFIL)
	{
		BTNR_P_TunerSetRFFIL(h); /* set bandwidth of RF tracking filter */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_band_ctrl, h->pTunerStatus->Tuner_RFFIL_band);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BW_tuning, h->pTunerStatus->Tuner_RFFIL_tune);
	}
	BTNR_P_TunerSetRFFILSwitch(h);	/* set RFFIL switches */
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
		BTNR_P_Ods_TunerSetBias(h);	/* set tuner bias */
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
		BTNR_P_Ads_TunerSetBias(h);	/* set tuner bias */
	h->pTunerStatus->Tuner_Change_Settings = 0;
	BDBG_MSG(("BTNR_P_TunerSetSignalPath() Complete"));
	BDBG_MSG(("Tuner Bias Select = %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
	BDBG_MSG(("Tuner RFFIL Select = %d",h->pTunerParams->BTNR_Internal_Params.RFFIL_Select));
}

/*******************************************************************************************
 * BTNR_P_TunerTune()	This routine tunes the tuner
 *******************************************************************************************/
BERR_Code BTNR_P_TunerTune(BTNR_3x7x_ChnHandle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t	Freq_pre=0, Freq_post=0;
	uint8_t temp;
	/*enable 6-bit ADC*/
	BTNR_P_TunerSetADC6B(h); h->pTunerStatus->LNAAGC_FRZ = 0;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 0x0);
	h->pTunerStatus->RFAGC_dither = 1; BTNR_P_TunerSetDither(h);
	Freq_pre = (h->pTunerStatus->Tuner_RF_Freq_pre); Freq_post = (h->pTunerParams->BTNR_Acquire_Params.RF_Freq);
	if (Freq_pre != Freq_post)
	{
		h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eACI;
		if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial)
		{
			h->pTunerStatus->RFVGA_byp = 0; BTNR_P_Ods_TunerSetRFVGA(h);
			h->pTunerStatus->AGC_TOP = 0; BTNR_P_Ods_TunerSetAGCTOP(h);
		}
	}
	BTNR_P_TunerSetSignalPath(h);
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial) {h->pTunerStatus->External_FGLNA_Mode = 1;}
#ifdef SmartTuneEnabled
	if ((h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial) && (BCHP_CHIP != 3472))
	{
		BTNR_P_TunerSmartTune(h);
	}
#endif
	BTNR_P_TunerSetDCOCLK(h);
	/*set the tuner input pads for testing*/
	if (h->pTunerParams->BTNR_Internal_Params.SDADC_Input != BTNR_Internal_Params_SDADC_Input_eTuner)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_pad_en, 1); /*enable pad switch*/
	}
	else
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_pad_en, 0); /*disable pad switch*/
	}
	/*Main Tuning*/
	switch (h->pTunerParams->BTNR_RF_Input_Mode)
	{
	case BTNR_3x7x_TunerRfInputMode_eExternalLna:
	case BTNR_3x7x_TunerRfInputMode_eInternalLna:
	case BTNR_3x7x_TunerRfInputMode_eStandardIf:
		temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_07, MXRPLL_FREQ_LOCK);
		if ((Freq_post > (h->pTunerParams->BTNR_Local_Params.freq_HRM_max)) && (h->pTunerStatus->rev == 0))
		{
			h->pTunerParams->BTNR_Acquire_Params.RF_Freq = 870000000;
			BTNR_P_TunerSetFGA_IFLPF(h);
			BTNR_P_TunerSetFreq(h);
			BTNR_P_TunerLDOCal(h);
			h->pTunerParams->BTNR_Acquire_Params.RF_Freq = Freq_post;
			BTNR_P_TunerSetFGA_IFLPF(h);
			BTNR_P_TunerSetFreq(h);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, P_divreg1p0_cntl_1p0, (h->pTunerStatus->LO_LDO));
		}
		else
		{
			if (((h->pTunerParams->BTNR_TuneType.TuneType == BTNR_TuneType_eMiniTune) || (((Freq_pre == Freq_post) && (h->pTunerParams->BTNR_TuneType.TuneType != BTNR_TuneType_eInitTune)) && (temp ==1)
			&& ((h->pTunerStatus->g_IQ_phs_DEG)*(h->pTunerStatus->g_IQ_phs_DEG) <= 50))) && (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable))
			{
				BTNR_P_TunerSetFGA_IFLPF(h);
			}
			else
			{
				BTNR_P_TunerSetFGA_IFLPF(h);
				BTNR_P_TunerSetFreq(h);
			}
		}
		h->pTunerStatus->Tuner_RF_Freq_pre = Freq_post;
	break;
	case BTNR_3x7x_TunerRfInputMode_eLowIf:
	case BTNR_3x7x_TunerRfInputMode_eBaseband:
	case BTNR_3x7x_TunerRfInputMode_eOff:
	break;
	default:
		BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_RF_Input_Mode, value received is %d",h->pTunerParams->BTNR_RF_Input_Mode));
		retCode = BERR_INVALID_PARAMETER;
		/*goto bottom of function to return error code*/
		goto something_bad_happened;
	}
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable) {BTNR_P_Ads_TunerShiftGearAGC(h); /*BTNR_P_Ads_TunerBiasSelect(h);*/}
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial) {h->pTunerStatus->dly = 10; BTNR_P_TunerRSSI(h); BTNR_P_Ods_TunerBiasSelect(h);}
	BTNR_P_TunerSetSignalPath(h);
	h->pTunerStatus->LNAAGC_count = 10;
	BDBG_MSG(("BTNR_P_TunerTune() Complete"));
/*goto label to return error code if something bad happened above*/
something_bad_happened:
  return retCode;
}

/*******************************************************************************************
 * BTNR_P_TunerSetRFFIL()    This routine selects the correct RF filter corner frequency
 *******************************************************************************************/
void BTNR_P_TunerSetRFFIL(BTNR_3x7x_ChnHandle h)
{
	/*local variables*/
	uint8_t i;
	uint8_t j;
	uint8_t RFTRK_band_ctrl=0;
	uint8_t RFTRK_tune_ctrl=0;
	/*sequence through the LPF_Selection_Table[] from highest freq value to lowest freq value*/
	/*table is in 100 KHz resolution*/
	/*when we get to the entry with a lower freq then the TunerFreq record index and rftrk1 then exit and program*/
	i = 0;
	j = 0;
	if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= 82000000 )
	{
	RFTRK_band_ctrl = 0;
	RFTRK_tune_ctrl = 17;
	}
	else if ((h->pTunerParams->BTNR_Acquire_Params.RF_Freq > 82000000) && (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= (1002000/7)*1000))
	{
	RFTRK_band_ctrl = 0;
		for (i=0;i<11;i++)
		{
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= (uint32_t)((82+i*6)*1000000)+1160000)
			{
				RFTRK_tune_ctrl = LPF_Selection_Table[i];
				break;
			}
		}
	}
	else if ((h->pTunerParams->BTNR_Acquire_Params.RF_Freq > (1002000/7)*1000) && (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= 262000000))
	{
	RFTRK_band_ctrl = 1;
		for (i=11;i<31;i++)
		{
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= (uint32_t)((82+i*6)*1000000))
			{
				RFTRK_tune_ctrl = LPF_Selection_Table[i];
				break;
			}
		}
	}
	else if ((h->pTunerParams->BTNR_Acquire_Params.RF_Freq > 262000000) && (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= 355000000))
	{
	RFTRK_band_ctrl = 1;
		for (i=31;i<71;i++)
		{
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= (uint32_t)((82+i*6)*1000000))
			{
				RFTRK_tune_ctrl = LPF_Selection_Table[i];
				break;
			}
		}
	}
	else if ((h->pTunerParams->BTNR_Acquire_Params.RF_Freq > 355000000) && (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= 502000000))
	{
	RFTRK_band_ctrl = 2;
		for (i=31;i<71;i++)
		{
			if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq <= (uint32_t)((82+i*6)*1000000))
			{
				RFTRK_tune_ctrl = LPF_Selection_Table[i];
				break;
			}
		}
	}
	else
	{
		BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_Acquire_Params.RF_Freq, value received is %d",h->pTunerParams->BTNR_Acquire_Params.RF_Freq));
	}
	h->pTunerStatus->Tuner_RFFIL_band = RFTRK_band_ctrl;
	h->pTunerStatus->Tuner_RFFIL_tune = RFTRK_tune_ctrl;
	BDBG_MSG(("BTNR_P_TunerSetRFFIL() Complete"));
}

/*******************************************************************************************
 * BTNR_P_TunerSetFGA_IFLPF()	This routine it to set the IF filter in the tuner
 *******************************************************************************************/
void BTNR_P_TunerSetFGA_IFLPF(BTNR_3x7x_ChnHandle h)
{
	/*local variables*/
	uint8_t	FGA_GAIN;
	uint8_t FGA_RC_Ctrl;
	uint8_t IFLPF_BW_Sel;
	uint8_t	IFLPF_WBW_Sel;

	/*Set FGA gain to high gain for terrestrial, low gain for cable*/
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable)
	{
		BREG_WriteField(h->hRegister,UFE_AFE_TNR0_MXR_02, i_FGA_HiGain, 0);
		FGA_GAIN = 0;
	}
	{
		BREG_WriteField(h->hRegister,UFE_AFE_TNR0_MXR_02, i_FGA_HiGain, 1);
		FGA_GAIN = 1;
	}

	/*lookup the IFLPF_BWR_Sel and IFLPF_WBW_Sel values, they are different for each bandwidth 2,4,5,8,10 MHz */
	switch (h->pTunerParams->BTNR_Acquire_Params.LPF_Bandwidth)
	{
	case BTNR_LPF_Bandwidth_e8MHz :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_8MHz : FGA_RC_CTRL_LOWG_8MHz;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_8MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_8MHz;
		break;
	case BTNR_LPF_Bandwidth_e7MHz :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_7MHz : FGA_RC_CTRL_LOWG_7MHz;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_7MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_7MHz;
		break;
	case BTNR_LPF_Bandwidth_e6MHz :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_6MHz : FGA_RC_CTRL_LOWG_6MHz;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_6MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_6MHz;
		break;
	case BTNR_LPF_Bandwidth_e5MHz :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_5MHz : FGA_RC_CTRL_LOWG_5MHz;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_5MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_5MHz;
		break;
	case BTNR_LPF_Bandwidth_e1_7MHz :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_1_7MHz : FGA_RC_CTRL_LOWG_1_7MHz;;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_1_7MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_1_7MHz;
		break;
	case BTNR_LPF_Bandwidth_eVariable :
		if ((h->pTunerParams->BTNR_Acquire_Params.LPF_Variable_Bandwidth > MAX_LPF_VARIABLE_BW) ||
			  (h->pTunerParams->BTNR_Acquire_Params.LPF_Variable_Bandwidth < MIN_LPF_VARIABLE_BW))
		{
			FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_8MHz : FGA_RC_CTRL_LOWG_8MHz;
			IFLPF_BW_Sel  = IFLPF_BW_SEL_8MHz;
			IFLPF_WBW_Sel = IFLPF_WBW_SEL_8MHz;
			BDBG_ERR(("ERROR!!!  VARIABLE LPF BANDWIDTH UNSUPPORTED SETTING TO 8 MHZ"));
			BDBG_ERR(("ERROR!!! Invalid Tuner_Variable_BW in BTNR_P_TunerSetFGA_IFLPF() , Value is %d", h->pTunerParams->BTNR_Acquire_Params.LPF_Variable_Bandwidth));
		}
		else
		{
			FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_8MHz : FGA_RC_CTRL_LOWG_8MHz;
			IFLPF_BW_Sel  = IFLPF_BW_SEL_8MHz;
			IFLPF_WBW_Sel = IFLPF_WBW_SEL_8MHz;
		}
		break;
	default :
		FGA_RC_Ctrl = (FGA_GAIN == 1) ? FGA_RC_CTRL_HIGHG_8MHz : FGA_RC_CTRL_LOWG_8MHz;
		IFLPF_BW_Sel  = IFLPF_BW_SEL_8MHz;
		IFLPF_WBW_Sel = IFLPF_WBW_SEL_8MHz;
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected in BTNR_P_TunerSetFGA_IFLPF() , Value is %d", h->pTunerParams->BTNR_Acquire_Params.LPF_Bandwidth));
		break;
	}

	/*write the bandwidth values*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_C_ctrl, FGA_RC_Ctrl);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_BW, IFLPF_BW_Sel);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_WBW, IFLPF_WBW_Sel);
	BDBG_MSG(("LPF_Bandwidth = %d", h->pTunerParams->BTNR_Acquire_Params.LPF_Bandwidth));
	BDBG_MSG(("LPF_Variable_Bandwidth = %d", h->pTunerParams->BTNR_Acquire_Params.LPF_Variable_Bandwidth));
	BDBG_MSG(("i_FGA_C_ctrl = %d", FGA_RC_Ctrl));
	BDBG_MSG(("i_LPF_BW = %d", IFLPF_BW_Sel));
	BDBG_MSG(("i_LPF_WBW = %d", IFLPF_WBW_Sel));
	BDBG_MSG(("BTNR_P_TunerSetFGA_IFLPF() Complete"));
}

/******************************************************************************
 BTNR_P_DPMSetFreq()
******************************************************************************/
void BTNR_P_DPMSetFreq(BTNR_3x7x_ChnHandle h)
{
	/*local variables*/
	uint16_t temp;

	/* power up DPM */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DPM_LOBUF_REG, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DPM, 0x1);
	/* Enable DPM fx clock */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m5div, 0x20);
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp | 0x10 ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
	/* Enable divider change on the fly */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x3f);
	/*Set LO*/
	BTNR_P_TunerSetLO(h);
	/* DPM Settings */
	/* DPM Amplitude Settings */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, DPM_6dB, 0x1); /*enabling 6dB boost*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, DPM_amp0, 0x1); /*setting the output power LSB*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, DPM_amp1, 0x1); /*setting the output power*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, DPM_amp2, 0x1); /*setting the output power MSB*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, sel_I, 0x1); /*Setting the mode for upconversion/downconversion. Toggle this bit to change the mode.*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, sel_Q, 0x0); /*Setting the mode for upconversion/downconversion*/
	/* LOGen Settings */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_logen_I_Qneg, 0x0);
	/* assert/de-assert DPM reset/resetb */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_logen_reset, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_logen_start, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_async_reset, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, reset_1p0, 0x0);
	BKNI_Delay(1); /*1 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_logen_reset, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_logen_start, 0x0);
	BKNI_Delay(2); /*2 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_logen_reset, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_logen_start, 0x1);
	BKNI_Delay(1); /*1 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, reset_1p0, 0x1);
	BKNI_Delay(2); /*2 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, reset_1p0, 0x0);
	/*enable DPM out*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_DPM_switch, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DAISY_UHF, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DS_01, i_DAISY_UHF_DPM_sel, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_DPM_01, i_atten, 0x1);
	BKNI_Sleep(1);
	/*MXR SR, pre_div reset*/
	BTNR_P_TunerSetMXR(h);
	/*MoCA trap*/
	if ((h->pTunerStatus->LO_index == 4) || (h->pTunerStatus->LO_index == 5))
	{
		h->pTunerParams->BTNR_Internal_Params.RFFIL_Select = BTNR_Internal_Params_TunerRFFIL_eMOCATRAP;
		BTNR_P_TunerSetRFFILSwitch(h);
	}
	BDBG_MSG(("BTNR_P_DPMSetFreq() Complete"));
}

/******************************************************************************
 BTNR_P_TunerSetFreq()
******************************************************************************/
void BTNR_P_TunerSetFreq(BTNR_3x7x_ChnHandle h)
{
	/*Set LO*/
	BTNR_P_TunerSetLO(h);
	/*MXR SR, pre_div reset*/
	BTNR_P_TunerSetMXR(h);
	BDBG_MSG(("BTNR_P_TunerSetFreq() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetADC6B()  This routine sets the tuner 6-bit ADC
 ******************************************************************************************************************/
void BTNR_P_TunerSetADC6B(BTNR_3x7x_ChnHandle h)
{
	uint16_t temp;

	/* Powerup Channel 1 */
	temp = BREG_ReadField(h->hRegister,UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp | 0x1 ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
	BKNI_Delay(20); /*20 usec*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_ADC6B, 0x1);
	BKNI_Delay(20); /*20 usec*/
	/* De-assert reset/resetb for 6-bit ADC */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, ADC6B_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, ADC6B_resetb, 1);
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetADC6BDOWN()  This routine power down the tuner 6-bit ADC
 ******************************************************************************************************************/
void BTNR_P_TunerSetADC6BDOWN(BTNR_3x7x_ChnHandle h)
{
	uint16_t temp;

	/* Power down Channel 1 & ADC6B*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_ADC6B, 0x0);
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp & 0x3E ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetLNAAGC()  This routine sets the tuner LNA AGC
 ******************************************************************************************************************/
void BTNR_P_TunerSetLNAAGC(BTNR_3x7x_ChnHandle h)
{
	uint32_t ReadReg0;
	/*BERR_Code retCode = BERR_SUCCESS;*/

	/* negative edge */
	ReadReg0 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_LNAAGC_02);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_LNAAGC_02, (ReadReg0 | 0x80000000));
	/* De-assert reset/resetb for LNA AGC */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LNAAGC_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LNAAGC_resetb, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_dsm_srst, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_dsm_srst, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_agc_srst, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_agc_srst, 0);

	/*set initial gain*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, init_LNA_gain, 15);

	/*unfreeze, un-bypass*/
    BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_byp, 0);
    BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 0);
	/*start LNA AGC*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, lna_start, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, lna_start, 0x1);

	BDBG_MSG(("BTNR_P_TunerSetLNAAGC() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetDCO()  This routine sets the tuner DCO
 ******************************************************************************************************************/
void BTNR_P_TunerSetDCO(BTNR_3x7x_ChnHandle h)
{
	uint8_t temp;

	/*Adjust LDO settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_REG_01, i_REG1p0_ctrl, 3);
	/*Enable DCO Clock*/
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch);
	temp = temp | 0x8 ;
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_02, i_pwrup_PHYPLL_ch, temp);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_DCO, 1);
	/* De-assert reset/resetb for DCO */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, i_rstb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, i_dco_rstb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, i_rstb, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, i_dco_rstb, 1);
	/*gear shifting*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRDCO_01, i_dco_k0, 16);
	BKNI_Sleep(10);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRDCO_01, i_dco_k0, 8);
	BKNI_Sleep(10);
	BDBG_MSG(("DCO is running"));
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetDCOCLK()  This routine sets the tuner DCO clock
 ******************************************************************************************************************/
void BTNR_P_TunerSetDCOCLK(BTNR_3x7x_ChnHandle h)
{
	uint16_t temp;
	int16_t offset;
	uint8_t count;
	uint16_t RF_Freq = h->pTunerParams->BTNR_Acquire_Params.RF_Freq/100000;
	temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int);
	temp = temp/BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div);
	offset = RF_Freq - (RF_Freq/temp/10)*temp*10;
	if (offset > temp*5) {offset = offset - temp*10;}
	if ((offset >= -40) && (offset <= 40))
	{
		for (count = 1; count < 10; count++)
		{
			temp = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int);
			temp = temp/BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div)*270/(27-count);
			offset = RF_Freq - (RF_Freq/temp)*temp;
			if (offset > temp/2) {offset = offset - temp;}
			if ((offset >= 40) ||(offset <= -40)) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m4div, 27-count);BDBG_MSG(("count = %d",count)); break;}
		}
	}
	else
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m4div, 0x1B);
	}
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x3F);
	BDBG_MSG(("DCO spur offset = %d",offset));
}

/******************************************************************************
 BTNR_P_CalFlashSDADC() - calibrate SD ADC flash
******************************************************************************/
void BTNR_P_CalFlashSDADC(BTNR_3x7x_ChnHandle h)
{
	uint8_t idx;
	uint32_t ICalOffset, QCalOffset;
	uint8_t statusIch, statusQch=0;

	/*I Channel Calibration*/
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_resetCal, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_resetCal, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_cal_on, 0x1);

	/*Q Channel Calibration*/
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_resetCal, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x1);
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_resetCal, 0x0);
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_cal_on, 0x1);

	for (statusIch=0, idx =0; (idx<2) && ( !(statusIch) || !(statusQch) ) ;idx++){
		BKNI_Sleep(1);
		#if (BTNR_P_BCHP_TNR_CORE_VER <= BTNR_P_BCHP_CORE_V_1_1)
			statusIch = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH, o_Ich_flash_cal_done);
			statusQch = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH, o_Qch_flash_cal_done);
		#else
			statusIch = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH, o_ch_master_done);
			statusQch = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH, o_ch_master_done);
		#endif
	}

	switch (statusIch)
	{
	case 0 : BDBG_MSG(("SDADC I channel calibration NOT done"));
			 if (idx>99){
				 BDBG_ERR(("SDADC I channel calibration timeout"));
				 }
			 break;
	case 1 : BDBG_MSG(("SDADC I channel calibrationis done")); break;
	default :BDBG_ERR(("ERROR!!! INVALID SDADC I Channel Calibration Value: value is %d",statusIch)); break;
	}

	#if (BTNR_P_BCHP_TNR_CORE_VER <= BTNR_P_BCHP_CORE_V_1_1)
		ICalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH, o_Ich_flash_caldata);
		BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_ctl_Ich_flash_offset, ICalOffset);
	#else
		ICalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH_CAL_DATA, ch_flash_caldata);
		BREG_WriteField(h->hRegister, SDADC_CTRL_ICH_CAL_DATA, i_ctl_ch_flash_offset, ICalOffset);
	#endif
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_cal_on, 0);

	switch (statusQch)
	{
	case 0 : BDBG_MSG(("SDADC Q channel calibration NOT done"));
			 if (idx>99){
				 BDBG_ERR(("SDADC Q channel calibration timeout"));
				 }
			 break;
	case 1 : BDBG_MSG(("SDADC Q channel calibrationis done")); break;
	default :BDBG_ERR(("ERROR!!! INVALID SDADC Q Channel Calibration Value: value is %d",statusQch)); break;
	}

	#if (BTNR_P_BCHP_TNR_CORE_VER <= BTNR_P_BCHP_CORE_V_1_1)
		QCalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH, o_Qch_flash_caldata);
		BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_ctl_Qch_flash_offset, QCalOffset);
	#else
		QCalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH_CAL_DATA, ch_flash_caldata);
		BREG_WriteField(h->hRegister, SDADC_CTRL_QCH_CAL_DATA, i_ctl_ch_flash_offset, QCalOffset);
	#endif
	BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_cal_on, 0);

	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x0);
}

/*******************************************************************************************************************
 * BTNR_P_TunerSetRFAGC()  This routine sets the tuner RF AGC
 ******************************************************************************************************************/
void BTNR_P_TunerSetRFAGC(BTNR_3x7x_ChnHandle h)
{
	uint32_t ReadReg;
	/*enable dither clock*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_clk_dither, 0x1);
	h->pTunerStatus->RFAGC_dither = 1;
	/*enable PN sequence*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, spare00, 0x2);

	/*RF AGC init*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_DAC_LPF_pwrup_RFAGC, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_RFDPD_pwrup_RFAGC, 1);
    #if (BTNR_P_BCHP_TNR_CORE_VER == BTNR_P_BCHP_CORE_V_1_0)/* (BCHP_VER == BCHP_VER_A0) */  /* TO FIX - Fixed */
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i__BB2DPD_pwrup_RFAGC, 1);
	#else
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_BB2DPD_pwrup_RFAGC, 1);
	#endif
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_BB1DPD_pwrup_RFAGC, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_dsm_sigdel_en, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_DSM_intg_reset, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, RFAGC_DSM_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_reset, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_DSM_intg_reset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, RFAGC_DSM_resetb, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_reset, 0);

	/*RF AGC settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_delay, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_clk_extend, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_sel_IQ, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_delay, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_clk_extend, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_sel_IQ, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_delay, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_clk_extend, 0x2);

	/*RF AGC initial gain settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x1);
	h->pTunerStatus->RFAGC_indirect_addr = 0x5;
	h->pTunerStatus->RFAGC_indirect_data = 0xFFFFFFFF;
	BTNR_P_TunerRFAGCIndirectWrite(h);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x0);

	/*RF AGC close loop settings*/
	h->pTunerStatus->RFAGC_indirect_addr = 0x0;
	BTNR_P_TunerRFAGCIndirectRead(h);
	ReadReg = h->pTunerStatus->RFAGC_indirect_data;

	ReadReg = ReadReg & 0xFFF3BDC0;
	if (h->pTunerParams->BTNR_TunePowerMode.Tuner_Power_Mode == BTNR_Tuner_Power_Mode_eLna_Daisy_Power)
		ReadReg = ReadReg | 0x0000401A;
	else
		ReadReg = (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eTerrestrial) ? ReadReg | 0x00004018 :  ReadReg | 0x0000401B;

	h->pTunerStatus->RFAGC_indirect_addr = 0x0;
	h->pTunerStatus->RFAGC_indirect_data = ReadReg;
	BTNR_P_TunerRFAGCIndirectWrite(h);

	BDBG_MSG(("BTNR_P_TunerSetRFAGC() Complete"));
}

/******************************************************************************
 BTNR_P_TunerSearchCapSoftware() - capacitor binary scan
******************************************************************************/
void BTNR_P_TunerSearchCapSoftware(BTNR_3x7x_ChnHandle h)
{
	uint8_t bin_end, idx1;
	uint32_t capMask, cval_t0;
	bin_end = 0;

	for (idx1 = 8; idx1 > bin_end; idx1--)
	{
		capMask = 1 << idx1;
		cval_t0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_cap_cntl_1p0) | capMask;
		if ( (cval_t0 <= 511) && (cval_t0 >= 311) )
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_nbr_lf, 0x1);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_wben_lf, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x9);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_kvco_cntl_1p0, 0x5);
		}
		else if ( (cval_t0 <= 310) && (cval_t0 >= 229) )
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_nbr_lf, 0x1);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_wben_lf, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x9);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_kvco_cntl_1p0,0x3);
		}
		else if ( (cval_t0 <= 228) && (cval_t0 >= 161) )
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_nbr_lf, 0x3);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_wben_lf, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x5);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_kvco_cntl_1p0, 0x2);
		}
		else if ( (cval_t0 <= 160) && (cval_t0 >= 123) )
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_nbr_lf, 0x1);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_wben_lf, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x9);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_kvco_cntl_1p0, 0x1);
		}
		else if (cval_t0 <= 122)
		{
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_nbr_lf, 0x3);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_01, lP_wben_lf, 0x0);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_QPbiasCNT2_1p0, 0x5);
			BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_kvco_cntl_1p0, 0x1);
		}
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_cap_cntl_1p0, cval_t0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_I_cntl_1p0, (cval_t0 >> 2));
		BKNI_Sleep(10);
		if (BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_07, MXRPLL_PopCap) == 1)
			cval_t0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_cap_cntl_1p0) & (511 - capMask);
		else
			cval_t0 = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_cap_cntl_1p0) | capMask;
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_02, lP_cap_cntl_1p0, cval_t0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, lP_I_cntl_1p0, (cval_t0 >> 2));
	}
	h->pTunerStatus->cval = cval_t0;
	BDBG_MSG(("cval_t0 = %d", h->pTunerStatus->cval));
}

/******************************************************************************
 BTNR_P_TunerVCOCal() - VCO calibration
******************************************************************************/
void BTNR_P_TunerVCOCal(BTNR_3x7x_ChnHandle h)
{
	uint32_t Freq;
	h->pTunerParams->BTNR_Local_Params.hardware_tune = 0;/* software tuning */
	/* VCO calibration: 1 */
	h->pTunerParams->BTNR_Local_Params.VCO_max = 8600000; /* set VCO_max to 8.6GHz */
	Freq = h->pTunerParams->BTNR_Acquire_Params.RF_Freq;
	h->pTunerParams->BTNR_Acquire_Params.RF_Freq = 358400000;/* VCO_max/16 + 100KHz */
	BTNR_P_TunerSetFreq(h);
	if (h->pTunerStatus->cval >= 0x1f8) {h->pTunerParams->BTNR_Local_Params.VCO_max = 9020000;}
	/* VCO calibration: 2 */
	h->pTunerParams->BTNR_Acquire_Params.RF_Freq = h->pTunerParams->BTNR_Local_Params.VCO_max*100;/* VCO_max/10 */
	BTNR_P_TunerSetFreq(h);
	if (h->pTunerStatus->cval >= 0x5) /* cval >= 0x5 */
	{
		h->pTunerParams->BTNR_Local_Params.freq_HRM_max = (h->pTunerParams->BTNR_Local_Params.VCO_max/10)*1000;
	}
	else
	{
		h->pTunerParams->BTNR_Local_Params.freq_HRM_max = (h->pTunerParams->BTNR_Local_Params.VCO_max/12)*1000;
	}
	h->pTunerParams->BTNR_Acquire_Params.RF_Freq = Freq;
	h->pTunerParams->BTNR_Local_Params.hardware_tune = 1;/* hardware tuning */
}
/******************************************************************************
 BTNR_P_TunerLDOCal() - LO LDO calibration
******************************************************************************/
void BTNR_P_TunerLDOCal(BTNR_3x7x_ChnHandle h)
{
	uint8_t i, index=0;
	int32_t noise_pwr;
	noise_pwr = 100;
	h->pTunerStatus->dly = 3;
	for (i = 0; i < 4; i++)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, P_divreg1p0_cntl_1p0, i);
		BTNR_P_TunerRSSI(h);
		if (noise_pwr >=  h->pTunerStatus->EstChannelPower_dbm_i32) {noise_pwr = h->pTunerStatus->EstChannelPower_dbm_i32; index = i;}
	}
	for (i = 1; i < 4; i++)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXRPLL_03, P_divreg1p0_cntl_1p0, 3-i);
		BTNR_P_TunerRSSI(h);
		if (noise_pwr >=  h->pTunerStatus->EstChannelPower_dbm_i32) {noise_pwr = h->pTunerStatus->EstChannelPower_dbm_i32; index = 3-i;}
	}
	h->pTunerStatus->LO_LDO = index;
	BDBG_MSG(("h->pTunerStatus->LO_LDO, value received is %d",h->pTunerStatus->LO_LDO));
}

/******************************************************************************
 BTNR_P_TunerReadIQIMB() - Read IQ imbalance
******************************************************************************/
void BTNR_P_TunerReadIQIMB(BTNR_3x7x_ChnHandle h)
{
	int32_t g_IQ_phs, g_IQ_amp;
	/*Read I/Q imbalance loop filter values*/
	g_IQ_amp = BREG_Read32(h->hRegister, BCHP_UFE_IQIMB_AMP_LF);
	g_IQ_phs = BREG_Read32(h->hRegister, BCHP_UFE_IQIMB_PHS_LF);
	h->pTunerStatus->g_IQ_phs_AMP = (g_IQ_amp>>22); /* % = g_IQ_phs_AMP (div) 256 */
	h->pTunerStatus->g_IQ_phs_DEG = (g_IQ_phs>>14)/1143;
/*	BDBG_MSG(("g_IQ_phs_AMP = %d, g_IQ_phs_DEG = %d",h->pTunerStatus->g_IQ_phs_AMP, h->pTunerStatus->g_IQ_phs_DEG));*/
}

/******************************************************************************
  BTNR_P_TunerIQIMB() - adjust IQ imbalance
******************************************************************************/
void BTNR_P_TunerIQIMB(BTNR_3x7x_ChnHandle h)
{
	int8_t count;
	if (h->pTunerStatus->LO_index == 10)
	{	BTNR_P_TunerReadIQIMB(h);
		for (count = 0; count < 10; count++)
		{
			if (((h->pTunerStatus->g_IQ_phs_DEG)*(h->pTunerStatus->g_IQ_phs_DEG) > 50) || ((h->pTunerStatus->g_IQ_phs_AMP)*(h->pTunerStatus->g_IQ_phs_AMP) > 1000))
			{
				h->pTunerStatus->UFE_AGC_BW = 0; BTNR_P_TunerUFEAGCSet(h);
				h->pTunerStatus->dly = count;
				BTNR_P_TunerSetMXR(h);
				BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_AMP_LF,    0x00000000 );
				BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_PHS_LF,    0x00000000 );
				BKNI_Sleep(10);
				BTNR_P_TunerReadIQIMB(h);
			}
			else
			{
				h->pTunerStatus->UFE_AGC_BW = 1; BTNR_P_TunerUFEAGCSet(h);
				BDBG_MSG(("g_IQ_phs_AMP = %d, g_IQ_phs_DEG = %d, count = %d",h->pTunerStatus->g_IQ_phs_AMP,h->pTunerStatus->g_IQ_phs_DEG,count));
				break;
			}
		}
	}
}

/******************************************************************************
  BTNR_P_TunerUFEAGCSet() - set digital AGC BW
******************************************************************************/
void BTNR_P_TunerUFEAGCSet(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerStatus->UFE_AGC_BW)
	{
	case 0:
		BREG_Write32(h->hRegister, BCHP_UFE_AGC1,            0x00000002 );
		BREG_Write32(h->hRegister, BCHP_UFE_AGC2,            0x00000002 );
		BREG_Write32(h->hRegister, BCHP_UFE_AGC3,            0x00000002 );
		BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_AMP_CTRL,  0x00000000 );
		BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_PHS_CTRL,  0x00000000 );
	break;
	case 1:
		BREG_Write32(h->hRegister, BCHP_UFE_AGC1,            0x0000000D );
		BREG_Write32(h->hRegister, BCHP_UFE_AGC2,            0x0000000B );
		BREG_Write32(h->hRegister, BCHP_UFE_AGC3,            0x00000009 );
		BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_AMP_CTRL,  0x00000007 );
		BREG_Write32(h->hRegister, BCHP_UFE_IQIMB_PHS_CTRL,  0x00000007 );
		break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerStatus->UFE_AGC_BW, value received is %d",h->pTunerStatus->UFE_AGC_BW));
	}
}

/*******************************************************************************************************************
 * BTNR_P_TunerRSSI()  This routine estimates channel power
 ******************************************************************************************************************/
void BTNR_P_TunerRSSI(BTNR_3x7x_ChnHandle h)
{
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable) BTNR_P_Ads_TunerSetAGCBW(h);
	h->pTunerStatus->UFE_AGC_BW = 0; BTNR_P_TunerUFEAGCSet(h);
	BKNI_Sleep(h->pTunerStatus->dly);
	if (h->pTunerParams->BTNR_Internal_Params.TunerInitComplete == true) {BTNR_P_TunerStatus(h);}
	BTNR_P_InitStatus(h);
	h->pTunerStatus->EstChannelPower_dbm_i32 = -(int32_t)(h->pTunerStatus->Tuner_PreADC_Gain_x256db + h->pTunerParams->BTNR_Local_Params.PostADC_Gain_x256db + (h->pTunerStatus->External_FGLNA_Mode)*3584 - 1536);
	if (h->pTunerParams->BTNR_Acquire_Params.Application == BTNR_TunerApplicationMode_eCable) BTNR_P_Ads_TunerSetAGCBW(h);
	h->pTunerStatus->UFE_AGC_BW = 1; BTNR_P_TunerUFEAGCSet(h);
	BDBG_MSG(("LNA gain = %d, RFVGA gain = %d, EstChannelPower_dbm_i32 = %d",
	h->pTunerStatus->Tuner_LNA_Gain_Code,((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF),h->pTunerStatus->EstChannelPower_dbm_i32/256));
}

/******************************************************************************
 BTNR_P_TunerSetREFPHYPLL()
******************************************************************************/
void BTNR_P_TunerSetREFPHYPLL(BTNR_3x7x_ChnHandle h)
{
	uint32_t ReadReg;
	if (h->pTunerStatus->REFPLL == 1)
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x1);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_CLK_bypass_enable, 0x0);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006CFF);
		ReadReg = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02);
		ReadReg = ReadReg & 0xFFFDFFFF;
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, ReadReg);
	}
	else
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR_REFPLL_03, REF_CLK_bypass_enable, 0x1);
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  0x00006806);
		ReadReg = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02);
		ReadReg = ReadReg | 0x00020000;
		BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, ReadReg);
	}
	BKNI_Sleep(1);
	/* Hold reset/resetb for REFPLL/PHYPLL */
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESETB_01, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESET_01, 0xFFFFFFFF);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 1);
	/* De-assert reset/resetb for REFPLL/PHYPLL */
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESETB_01, 0x00000003);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESET_01, 0xFFFFFFF0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 0);
	BKNI_Sleep(1);
	BDBG_MSG(("BTNR_P_TunerSetREFPHYPLL() Complete"));
}
