
/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
#include "bchp_thd_core.h"
#endif
#include "bchp_ufe_afe.h"
#include "bchp_sdadc.h"
#ifndef LEAP_BASED_CODE
BDBG_MODULE(btnr_tune_ofdm);
#define POWER2_24 16777216
#define POWER2_3 8
#define POWER2_16 65536
#define POWER2_29 536870912
#define POWER2_27 134217728
#endif
#include "bchp_ufe.h"

#include "btnr_tune_ofdm.h"

#define LOW_POWER_MODE

#ifdef  LOW_POWER_MODE
#define ENABLE_LNA_AGC_CYCLE
/*LNA*/
#define LNA_BIAS_SENSITIVITY 12
#define LNA_BIAS_ACI 1
#define LNA_BIAS_LINEAR 1
/*LNA SF*/
#define LNASF_BIAS_SENSITIVITY 1
#define LNASF_BIAS_ACI 1
#define LNASF_BIAS_LINEAR 1
/*RFVGA*/
#define RFVGA_BIAS_SENSITIVITY 4
#define RFVGA_BIAS_ACI 4
#define RFVGA_BIAS_LINEAR 4
/*RFFIL*/
#define TRKFIL_BIAS_SENSITIVITY 7
#define TRKFIL_BIAS_ACI 7
#define TRKFIL_BIAS_LINEAR 7
/*RFVGA RDEG*/
#define RFVGA_RDEG_SENSITIVITY 1
#define RFVGA_RDEG_ACI 1
#define RFVGA_RDEG_LINEAR 1
/*MXR*/
#define MXR_BIAS_SENSITIVITY 1
#define MXR_BIAS_ACI 1
#define MXR_BIAS_LINEAR 1
/*FGA*/
#define FGA_BIAS_SENSITIVITY 4
#define FGA_BIAS_ACI 4
#define FGA_BIAS_LINEAR 4
/*LPF*/
#define LPF_BIAS_SENSITIVITY 2
#define LPF_BIAS_ACI 2
#define LPF_BIAS_LINEAR 2
#endif

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetAGCTOP()  This routine sets the tuner AGC TOP
 ******************************************************************************************************************/
 void BTNR_P_Ods_TunerSetAGCTOP(BTNR_3x7x_ChnHandle h)
{	/* terrerstrial applications: DVB-T/T2 or ISBD-T */
	switch (h->pTunerStatus->AGC_TOP)
	{
	case 0 :
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, h->pTunerStatus->LNA_TOP);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_Ios_PRG, 0x8);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_Ios_PRG, 0x7);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_Ios_PRG, 0x7);
	break;
	case 1 :
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, h->pTunerStatus->LNA_TOP);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_Ios_PRG, 0x8);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_Ios_PRG, 0xD);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_Ios_PRG, 0xD);
	break;
	default :
		BDBG_ERR(("ERROR!!! h->pTunerStatus->AGC_TOP, value received is %d",h->pTunerStatus->AGC_TOP));
	break;
	}
	BDBG_MSG(("BTNR_P_Ods_TunerSetAGCTOP() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerBiasSelect()  This routine sets the tuner bias mode based AGC reading
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerBiasSelect(BTNR_3x7x_ChnHandle h)
{
	uint8_t LIN_ACI_1, LIN_ACI_2;
	LIN_ACI_1 = (h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity > 7680) ? 1: 0;/* desired power > sensitivity + 30 */
	LIN_ACI_2 = (((h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity > 2304) || (h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity < 0))
	&& (h->pTunerStatus->MAX_gain_code == 1));/* max gain code & desired power > sensitivity + 9 or < sensitivity */
	h->pTunerStatus->Tuner_Change_Settings = 0;

	switch (h->pTunerParams->BTNR_RF_Input_Mode)
	{
	case BTNR_3x7x_TunerRfInputMode_eInternalLna:
		switch (h->pTunerParams->BTNR_Internal_Params.Bias_Select)
		{
		case BTNR_Internal_Params_TunerBias_eSensitivity:
			if ((h->pTunerStatus->MAX_gain_code != 1) || (h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity > 512))
			{
				h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eACI;
				h->pTunerStatus->Tuner_Change_Settings = 1;	BDBG_MSG(("sen-->aci"));
			}
		break;
		case BTNR_Internal_Params_TunerBias_eACI:
			if ((h->pTunerStatus->MAX_gain_code == 1) && (h->pTunerStatus->EstChannelPower_dbm_i32-h->pTunerStatus->Tuner_Sensitivity < 0))
			{
				h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eSensitivity;
				h->pTunerStatus->Tuner_Change_Settings = 1;	BDBG_MSG(("aci-->sen"));
			}
			else if ((h->pTunerStatus->ACI_far == 1) || (h->pTunerStatus->ACI_near == 1))
			{
				h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eLINEAR;
                h->pTunerStatus->Tuner_Change_Settings = 1;	BDBG_MSG(("aci-->lin"));
				if (h->pTunerStatus->ACI_far == 1) 	BDBG_MSG(("ACI_far")); if (h->pTunerStatus->ACI_near == 1) BDBG_MSG(("ACI_near"));
			}
		break;
		case BTNR_Internal_Params_TunerBias_eLINEAR:
			if ((LIN_ACI_1 == 1) || (LIN_ACI_2 == 1))
			{
				h->pTunerParams->BTNR_Internal_Params.Bias_Select = BTNR_Internal_Params_TunerBias_eACI;
				h->pTunerStatus->Tuner_Change_Settings = 1;	BDBG_MSG(("lin-->aci"));
			}
		break;
		default:
			BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.Bias_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
		}
	break;
	case BTNR_3x7x_TunerRfInputMode_eExternalLna:
	case BTNR_3x7x_TunerRfInputMode_InternalLna_Daisy:
	case BTNR_3x7x_TunerRfInputMode_eStandardIf:
	case BTNR_3x7x_TunerRfInputMode_eLowIf:
	case BTNR_3x7x_TunerRfInputMode_eBaseband:
	case BTNR_3x7x_TunerRfInputMode_eOff:
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_RF_Input_Mode, value received is %d",h->pTunerParams->BTNR_RF_Input_Mode));
	}
	if (h->pTunerStatus->Tuner_Change_Settings == 1)
	{
		if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)
		{h->pTunerStatus->RFVGA_byp = 1; h->pTunerStatus->AGC_TOP = 1;}
		else
		{h->pTunerStatus->RFVGA_byp = 0; h->pTunerStatus->AGC_TOP = 0;}
		BTNR_P_Ods_TunerSetRFVGA(h); BTNR_P_Ods_TunerSetAGCTOP(h);
		BDBG_MSG(("LNA gain = %d, RFVGA gain = %d, SNR margin = %d", h->pTunerStatus->Tuner_LNA_Gain_Code,((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF),
		h->pTunerStatus->SNR_margin));
		BDBG_MSG(("RSSI = %d, LNA_TOP = %d, VGA atten = %d", h->pTunerStatus->EstChannelPower_dbm_i32, h->pTunerStatus->LNA_TOP, h->pTunerStatus->VGA_atten));
		BKNI_Sleep(50);
	}
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetTOPSmooth()  This routine sets the tuner transition smoothly
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetTOPSmooth(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerParams->BTNR_Internal_Params.Bias_Select)
	{
	case BTNR_Internal_Params_TunerBias_eSensitivity:
	break;
	case BTNR_Internal_Params_TunerBias_eACI:
		if ((h->pTunerStatus->TOP_DOWN_ACI == 1) && (h->pTunerStatus->TOP_UP_ACI == 0) && (h->pTunerStatus->TOP_DOWN_ACI_BOOST == 0))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP -= 1)); BDBG_MSG(("TOP_DOWN_ACI decrease to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_UP_ACI == 0) && (h->pTunerStatus->TOP_DOWN_ACI_BOOST == 1))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP = LNA_TOP_MIN)); BDBG_MSG(("TOP_DOWN_ACI_BOOST decrease to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_DOWN_ACI == 0) && (h->pTunerStatus->TOP_UP_ACI == 1) && (h->pTunerStatus->TOP_UP_ACI_BOOST == 0))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP += 1)); BDBG_MSG(("TOP_UP_ACI increase to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_DOWN_ACI == 0) && (h->pTunerStatus->TOP_UP_ACI_BOOST == 1))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP = LNA_TOP_MAX)); BDBG_MSG(("TOP_UP_ACI_BOOST increase to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
	break;
	case BTNR_Internal_Params_TunerBias_eLINEAR:
		if ((h->pTunerStatus->TOP_DOWN_LIN == 1) && (h->pTunerStatus->TOP_UP_LIN == 0) && (h->pTunerStatus->TOP_DOWN_LIN_BOOST == 0))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP -= 1)); BDBG_MSG(("TOP_DOWN_LIN decrease to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_UP_LIN == 0) && (h->pTunerStatus->TOP_DOWN_LIN_BOOST == 1))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP = LNA_TOP_MIN)); BDBG_MSG(("TOP_DOWN_LIN_BOOST decrease to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_DOWN_LIN == 0) && (h->pTunerStatus->TOP_UP_LIN == 1) && (h->pTunerStatus->TOP_UP_LIN_BOOST == 0))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP += 1)); BDBG_MSG(("TOP_UP_LIN increase to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
		if ((h->pTunerStatus->TOP_DOWN_LIN == 0) && (h->pTunerStatus->TOP_UP_LIN_BOOST == 1))
		{BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, (h->pTunerStatus->LNA_TOP = LNA_TOP_MAX)); BDBG_MSG(("TOP_UP_LIN_BOOST increase to %d", h->pTunerStatus->LNA_TOP)); BTNR_P_TunerSleep(10);}
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.Bias_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
	}
/*	BDBG_MSG(("LNA gain = %d, RFVGA gain = %d, SNR margin = %d, VGA_atten-SNR_margin = %d", h->pTunerStatus->Tuner_LNA_Gain_Code,((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF),
	h->pTunerStatus->SNR_margin, h->pTunerStatus->VGA_atten-h->pTunerStatus->SNR_margin));*/
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetBias()  This routine sets the tuner bias
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetBias(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerParams->BTNR_Internal_Params.Bias_Select)
	{
	case BTNR_Internal_Params_TunerBias_eSensitivity:
#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias, LNA_BIAS_SENSITIVITY);
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf, LNASF_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias, RFVGA_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg, RFVGA_RDEG_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl, TRKFIL_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl, MXR_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl, FGA_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias, LPF_BIAS_SENSITIVITY);
		BREG_WriteField(h->hRegister, SDADC_CTRL_SYS0, i_ctl_adc_gain, 0x0);
		BDBG_MSG(("----eSensitivity mode bias----"));
	break;
	case BTNR_Internal_Params_TunerBias_eACI:
#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias, LNA_BIAS_ACI);
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf, LNASF_BIAS_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias, RFVGA_BIAS_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg, RFVGA_RDEG_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl, TRKFIL_BIAS_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl, MXR_BIAS_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl, FGA_BIAS_ACI);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias, LPF_BIAS_ACI);
		BREG_WriteField(h->hRegister, SDADC_CTRL_SYS0, i_ctl_adc_gain, 0x1);
		BDBG_MSG(("----eACI mode bias----"));
	break;
	case BTNR_Internal_Params_TunerBias_eLINEAR:
#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias, LNA_BIAS_LINEAR);
#endif
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf, LNASF_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias, RFVGA_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg, RFVGA_RDEG_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl, TRKFIL_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl, MXR_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl, FGA_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias, LPF_BIAS_LINEAR);
		BREG_WriteField(h->hRegister, SDADC_CTRL_SYS0, i_ctl_adc_gain, 0x0);
		BDBG_MSG(("----eLINEAR mode bias----"));
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.Bias_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
	}
	BDBG_MSG(("BTNR_P_Ods_TunerSetBias() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetTilt()  This routine sets the tuner tilt
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetTilt(BTNR_3x7x_ChnHandle h)
{
	switch (h->pTunerParams->BTNR_Internal_Params.Bias_Select)
	{
	case BTNR_Internal_Params_TunerBias_eSensitivity:
	case BTNR_Internal_Params_TunerBias_eACI:
	case BTNR_Internal_Params_TunerBias_eLINEAR:
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_tilt, 0x0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_tilt, 0x0);
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.Bias_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
	}
	BDBG_MSG(("BTNR_P_Ods_TunerSetTilt() Complete"));
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetRFFILSelect()  This routine sets the tuner RFFIL mode
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetRFFILSelect(BTNR_3x7x_ChnHandle h)
{
	if (h->pTunerParams->BTNR_Internal_Params.LNA_Enable == BTNR_Internal_Params_eDisable)
	{
#ifdef LEAP_BASED_CODE
#if LNA_341X_ENABLE
		BLNA_P_Init_LNA(h->hTnr->pLna);																/* Initialize external LNA BCM3406 */
	    BLNA_P_Set_LNA_Boost(h->hTnr->pLna);
#endif
#endif
	}
	switch (h->pTunerParams->BTNR_Internal_Params.Bias_Select)
	{
	case BTNR_Internal_Params_TunerBias_eSensitivity:
	case BTNR_Internal_Params_TunerBias_eACI:
	case BTNR_Internal_Params_TunerBias_eLINEAR:
		if (h->pTunerParams->BTNR_Acquire_Params.RF_Freq > (1002000/3)*1000)
		{
			/*Use MOCA Trap*/
			h->pTunerParams->BTNR_Internal_Params.RFFIL_Select = BTNR_Internal_Params_TunerRFFIL_eMOCATRAP;
		}
		else
		{
			/*Use Tracking Filter*/
			h->pTunerParams->BTNR_Internal_Params.RFFIL_Select = BTNR_Internal_Params_TunerRFFIL_eTRKFIL;
		}
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerParams->BTNR_Internal_Params.Bias_Select, value received is %d",h->pTunerParams->BTNR_Internal_Params.Bias_Select));
	}
	BDBG_MSG(("BTNR_P_Ods_TunerSetRFFILSelect() Complete"));
}


/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetBiasSmooth()  This routine sets the tuner transition smoothly
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetBiasSmooth(BTNR_3x7x_ChnHandle h)
{
	uint8_t BIAS, READ_BIAS;
	/*LNA*/
	#if (BTNR_P_BCHP_TNR_CORE_VER >= BTNR_P_BCHP_CORE_V_1_1)
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = LNA_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = LNA_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = LNA_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_SPARE_01, i_LNA_ctrl_bias, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	#endif
	/*LNA SF*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = LNASF_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = LNASF_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = LNASF_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNA_01, i_LNA_sf, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	/*RFVGA*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = RFVGA_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = RFVGA_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = RFVGA_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_bias, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	/*RFFIL*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = TRKFIL_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = TRKFIL_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = TRKFIL_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFFIL_01, i_TRKFIL_BUF_I_ctrl, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	/*MXR*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = MXR_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = MXR_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = MXR_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_03, i_MIXER_bias_ctrl, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	/*FGA*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = FGA_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = FGA_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = FGA_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_MXR_02, i_FGA_bias_ctrl, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	/*LPF*/
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{BIAS = LPF_BIAS_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{BIAS = LPF_BIAS_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{BIAS = LPF_BIAS_LINEAR;}
	READ_BIAS = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias);
	while (READ_BIAS != BIAS)
	{
		if (READ_BIAS > BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias, (READ_BIAS -= 1)); BTNR_P_TunerSleep(1);}
		if (READ_BIAS < BIAS) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LPF_01, i_LPF_bias, (READ_BIAS += 1)); BTNR_P_TunerSleep(1);}
	}
	h->pTunerStatus->Tuner_Change_Settings = 2;
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetRdegSmooth()  This routine sets the tuner transition smoothly
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetRdegSmooth(BTNR_3x7x_ChnHandle h)
{
	uint8_t RFVGA_RDEG, RDEG;
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eSensitivity)	{RDEG = RFVGA_RDEG_SENSITIVITY;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eACI)	{RDEG = RFVGA_RDEG_ACI;}
	if (h->pTunerParams->BTNR_Internal_Params.Bias_Select == BTNR_Internal_Params_TunerBias_eLINEAR)	{RDEG = RFVGA_RDEG_LINEAR;}
	RFVGA_RDEG = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg);
	BDBG_MSG(("RFVGA_RDEG = %d",RFVGA_RDEG));
	if (RFVGA_RDEG != RDEG)
	{
		if (RFVGA_RDEG > RDEG) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg, (RFVGA_RDEG -= 1));}
		if (RFVGA_RDEG < RDEG) {BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_ctrl_rdeg, (RFVGA_RDEG += 1));}
	}
	else {h->pTunerStatus->Tuner_Change_Settings = 0;}
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetAGCBW()  This routine sets the tuner AGC BW
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetAGCBW(BTNR_3x7x_ChnHandle h)
{
/*	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x0);*/
	switch (h->pTunerStatus->AGC_BW)
	{
	case 0:
	case 1:
	case 2:
		/*LNA AGC bandwidth settings*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, peak_pwr_set_pt, 0x10B66);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, hi_thresh, 0x42D9);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, LNA_Kbw, 14);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, win_len, 0x18);

		/*RF AGC bandwidth settings*/
		h->pTunerStatus->RFAGC_indirect_addr = 0x4;
		h->pTunerStatus->RFAGC_indirect_data = 0x10;
		BTNR_P_TunerRFAGCIndirectWrite(h);

		h->pTunerStatus->RFAGC_indirect_addr = 0x2;
		h->pTunerStatus->RFAGC_indirect_data = 0x4E20;
		BTNR_P_TunerRFAGCIndirectWrite(h);

		h->pTunerStatus->RFAGC_indirect_addr = 0x1;
		h->pTunerStatus->RFAGC_indirect_data = 0x5AC25AC2;
		BTNR_P_TunerRFAGCIndirectWrite(h);
	break;
	default:
		BDBG_ERR(("ERROR!!! Invalid h->pTunerStatus->AGC_BW, value received is %d",h->pTunerStatus->AGC_BW));
	}
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerDemodInfo()  This routine checks Demod information
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerDemodInfo(BTNR_3x7x_ChnHandle h)
{
    switch (h->pTunerParams->BTNR_Acquire_Params.Standard)
	{
		case BTNR_Standard_eDVBT :
#ifndef LEAP_BASED_CODE
			h->pTunerStatus->CodeRate = (BREG_Read32(h->hRegister, BCHP_THD_CORE_TPS_OV) >> 12) & 0x7;
			h->pTunerStatus->QAM = (BREG_Read32(h->hRegister, BCHP_THD_CORE_TPS_OV) >> 4) & 0x3;
#endif
			h->pTunerStatus->Tuner_Sensitivity = DVBT_sensitivity[h->pTunerStatus->CodeRate][h->pTunerStatus->QAM];
		break;
		case BTNR_Standard_eT2 :
			h->pTunerStatus->Tuner_Sensitivity = T2_sensitivity[h->pTunerStatus->CodeRate][h->pTunerStatus->QAM];
		break;
		case BTNR_Standard_eISDBT :
/*#ifndef LEAP_BASED_CODE
			h->pTunerStatus->CodeRate = (BREG_Read32(h->hRegister, BCHP_THD_CORE_TMCC_OV_0) >> 8) & 0x7;
			h->pTunerStatus->QAM = (BREG_Read32(h->hRegister, BCHP_THD_CORE_TMCC_OV_0) >> 12) & 0x3;
#endif*/
			h->pTunerStatus->Tuner_Sensitivity = -20736;
		break;
		case BTNR_Standard_eQAM :
		break;
		default :
			BDBG_ERR(("ERROR!!! h->pTunerParams->BTNR_Acquire_Params.Standard, value received is %d",h->pTunerParams->BTNR_Acquire_Params.Standard));
		break;
    }
/*	BDBG_MSG(("Code Rate = %d ; QAM = %d",h->pTunerStatus->CodeRate,h->pTunerStatus->QAM));*/
}

/*******************************************************************************************************************
 * BTNR_P_Ods_TunerSetRFVGA()  This routine sets the RFVGA mode
 ******************************************************************************************************************/
void BTNR_P_Ods_TunerSetRFVGA(BTNR_3x7x_ChnHandle h)
{
	uint8_t i = 0;
	uint8_t pwrup_RFVGA, RFVGA_bypass;
	uint32_t ReadReg;
	pwrup_RFVGA = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_RFVGA);
	RFVGA_bypass = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_bypass);
	if ((h->pTunerStatus->RFVGA_byp == 1) && (pwrup_RFVGA == 1) && (RFVGA_bypass == 0))
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_RFVGA, 0);
		BTNR_P_TunerAGCReadBack(h);
		while (((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF) != 0x0000FFFF)
		{
			BKNI_Sleep(1);
			i = i+1;
			BTNR_P_TunerAGCReadBack(h);
		}
/*		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x1);*/
		BDBG_MSG(("RFVGA bypass settle time %d",i));
/*		BDBG_MSG(("RFVGA gain = %d", ((h->pTunerStatus->Tuner_RFVGA_Gain_Code>>16) & 0x0000FFFF)));*/
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_bypass, 1); i = 1;
	}
	else if ((h->pTunerStatus->RFVGA_byp == 0) && (pwrup_RFVGA == 0) && (RFVGA_bypass == 1))
	{
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFVGA_01, i_RFVGA_bypass, 0);
		BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PWRUP_01, i_pwrup_RFVGA, 1); i = 1;
	}
	if (i == 1)
	{
		/*RF AGC close loop settings*/
		h->pTunerStatus->RFAGC_indirect_addr = 0x0;
		BTNR_P_TunerRFAGCIndirectRead(h);
		ReadReg = h->pTunerStatus->RFAGC_indirect_data;
		ReadReg = ReadReg & 0xFFF3BDC0;
		ReadReg = ReadReg | 0x00004018;
		h->pTunerStatus->RFAGC_indirect_addr = 0x0;
		h->pTunerStatus->RFAGC_indirect_data = ReadReg;
		BTNR_P_TunerRFAGCIndirectWrite(h);
	}
}