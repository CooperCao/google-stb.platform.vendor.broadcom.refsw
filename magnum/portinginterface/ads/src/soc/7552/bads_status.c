/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bkni.h"
#include "btmr.h"
#ifndef LEAP_BASED_CODE
#include "bads.h"
#include "bads_priv.h"
#include "bads_global_clk.h"
#include "bads_mth.h"
#include "bads_3x7x_priv.h"
#else
#include "bads_api.h"
#endif
#include "bads_acquire.h"
#include "bads_utils.h"
#include "bads_priv.h"
#include "bads_def.h"
#if (BCHP_FAMILY==3128) || (BCHP_FAMILY==7584)
#include "bwfe_global_clk.h"
#endif
#if (BCHP_FAMILY==3461) || (BCHP_FAMILY==3462) 
#include "btnr_global_clk.h"
#endif
/*registers needed for the functions in this file*/
#include "bchp_ds.h" 

#ifndef LEAP_BASED_CODE
BDBG_MODULE(bads_status);
#endif
/************************************************************************************************
 * BADS_P_ChnStatusReset()	 This routine resets the status parameters 
 ************************************************************************************************/
void BADS_P_ChnStatusReset(BADS_3x7x_ChannelHandle hChn)
{
	BADS_3x7x_ChannelHandle h;
	h = hChn;

	/*Reset Status1 structure*/	
	/*hChn->pChnLockStatus->QLK_te;					DO NOT RESET THIS VALUE*/
	/*hChn->pChnLockStatus->FLK_te;					DO NOT RESET THIS VALUE*/
	/*hChn->pChnLockStatus->SNR_te;					DO NOT RESET THIS VALUE*/
	hChn->pChnLockStatus->SNRAVG_u32 = hChn->pChnLockStatus->SNR_u32;
	/*hChn->pChnLockStatus->ReAck_Count_u32 = 0;	    DO NOT RESET THIS VALUE*/
	hChn->pChnLockStatus->ReAcquire_Count_u32 = 0;

	/*Reset Status2 structure*/
	h->pChnStatus->Auto_te = BADS_3x7x_ChnStatus_eDisable;	
	h->pChnStatus->FLK_te = BADS_3x7x_ChnStatus_eUnlock; 
	h->pChnStatus->QLK_te = BADS_3x7x_ChnStatus_eUnlock; 
	h->pChnStatus->AcqType_te = BADS_3x7x_ChnStatus_AcqType_eFastAcquire; 
	h->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eNoPower;
	h->pChnStatus->AI_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->AB_te = BADS_3x7x_ChnStatus_Annex_eAnnexA;
	h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam16;
	h->pChnStatus->B64_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->B256_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->B1024_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A16_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A32_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A64_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A128_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A256_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A512_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A1024_te = BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->SI_te = BADS_3x7x_ChnStatus_eDisable; 
	h->pChnStatus->Interleaver_u32 = 0;
	h->pChnStatus->ReAck_Count_u32 = 0;
	h->pChnStatus->ReSync_Count_u32 = 0;
	h->pChnStatus->AcquisitionTime1_u32 = 0;
	h->pChnStatus->AcquisitionTime2_u32 = 0;
	h->pChnStatus->SNR_db_u32 = hChn->pChnLockStatus->SNR_u32;
	h->pChnStatus->SNRAVG_db_u32 = hChn->pChnLockStatus->SNR_u32;
	h->pChnStatus->EstChannelPower_dbm_i32 = 0;					
	h->pChnStatus->FrontEndGain_db_i32 = 0;
	h->pChnStatus->AGCB_Gain_db_i32 = 0;
	h->pChnStatus->EQ_Gain_db_i32 = 0;
	h->pChnStatus->Carrier_Scan_u32 = 0;
	h->pChnStatus->Carrier_Error_i32 = 0;
	h->pChnStatus->Symbol_Error_i32 = 0;
	h->pChnStatus->Phase_Error_i32 = 0;
	h->pChnStatus->FEC_Corr_RS_Bits_u32 = 0;
	h->pChnStatus->FEC_Corr_RS_Blocks_u32 = 0;
	h->pChnStatus->FEC_UCorr_RS_Blocks_u32 = 0;
	h->pChnStatus->FEC_Clean_RS_Blocks_u32 = 0;
	h->pChnStatus->PRE_FEC_BERT_u32 = 0;
	h->pChnStatus->BERT_u32 = 0;
	h->pChnStatus->RF_Frequency_u32 = 0;
	h->pChnStatus->RF_Offset_i32 = 0;
	h->pChnStatus->Upper_Symbol_Scan_u32 = 0;
	h->pChnStatus->Lower_Symbol_Scan_u32 = 0;
	h->pChnStatus->Symbol_Rate_u32 = 0;
	h->pChnStatus->EQ_Main_Tap_i32 = 0;

	/*Reset/Resync BER registers in chip*/
	BREG_Write32(hChn->hRegister, BCHP_DS_BER, 0x00000402);
	BREG_Write32(hChn->hRegister, BCHP_DS_BER, 0x0000004A);
	BREG_Write32(hChn->hRegister, BCHP_DS_BER, 0x0000000A);
	BREG_Write32(hChn->hRegister, BCHP_DS_OI_BER_CTL, 0x00000402);
	BREG_Write32(hChn->hRegister, BCHP_DS_OI_BER_CTL, 0x0000004A);
	BREG_Write32(hChn->hRegister, BCHP_DS_OI_BER_CTL, 0x0000000A);
	BKNI_Delay(6); /*This is needed to let the BERT sync*/
	BREG_Write32(hChn->hRegister, BCHP_DS_BER, 0x0000020A);
	BREG_Write32(hChn->hRegister, BCHP_DS_BERI, 0);
	BREG_Write32(hChn->hRegister, BCHP_DS_OI_BER_CTL, 0x0000020A);
	BREG_Write32(hChn->hRegister, BCHP_DS_OI_BER, 0);

	/*Reset Error Counters in chip ONLY for the second set, the first set is reset by the BADS_P_ChnLockStatus()*/
	BREG_Write32(hChn->hRegister, BCHP_DS_TPFEC, 0x00001F00);
}

/************************************************************************************************
 * BADS_P_Get_ChnStatus()	 This routine gets the status parameters 
 ************************************************************************************************/
BERR_Code BADS_P_ChnStatus(BADS_3x7x_ChannelHandle hChn)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	BADS_3x7x_ChannelHandle h;
	BADS_P_AdsCallbackData_t CallbackFrontend;
	uint32_t ReadReg_u32;
	int32_t  ReadReg1_i32, ReadReg2_i32;
	h = hChn;
	/*All of the Status2 is calculated here*/
	/*Status1 is calculated in BADS_P_Acquire() and BADS_P_ChnLockStatus()*/
	/*values in Status2 that are duplicate in Status1 are read from the Status1 structure*/
	/*any status that can be calculated by reading back from the core is done that way*/

	/*Get the following from AcqParam->BADS_Acquire_Params structure*/
	h->pChnStatus->Auto_te = (hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te == BADS_Acquire_Params_eEnable) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;

	/*Get FLK and QLK from Status->BADS_3x7x_ChnLockStatus structure*/
	h->pChnStatus->FLK_te = hChn->pChnLockStatus->FLK_te; 
	h->pChnStatus->QLK_te = hChn->pChnLockStatus->QLK_te;  

	/*Get the AcqType from the hChn->pChnAcqParam->BADS_Local_Params.AcqType_te*/
	switch (hChn->pChnAcqParam->BADS_Local_Params.AcqType_te)
	{                                                                              
		case BADS_Local_Params_AcqType_eFastAcquire:     h->pChnStatus->AcqType_te = BADS_3x7x_ChnStatus_AcqType_eFastAcquire;     break;
		case BADS_Local_Params_AcqType_eSlowAcquire:     h->pChnStatus->AcqType_te = BADS_3x7x_ChnStatus_AcqType_eSlowAcquire;     break;
		case BADS_Local_Params_AcqType_eScan:            h->pChnStatus->AcqType_te = BADS_3x7x_ChnStatus_AcqType_eScan;            break;
		case BADS_Local_Params_AcqType_eSlowAcquireScan: h->pChnStatus->AcqType_te = BADS_3x7x_ChnStatus_AcqType_eSlowAcquireScan; break;
		default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED AcqType")); break;
	}

	/*Get the AcqStatus from the hChn->pChnAcqParam->BADS_Local_Params.AcqStatus_te*/
	switch (hChn->pChnAcqParam->BADS_Local_Params.AcqStatus_te)
	{                                                                              
		case BADS_Local_Params_AcqStatus_eNoPower:    hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eNoPower;    break;
		case BADS_Local_Params_AcqStatus_eNoTiming:   hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eNoTiming;   break;
		case BADS_Local_Params_AcqStatus_eNoCarrier:  hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eNoCarrier;  break;
		case BADS_Local_Params_AcqStatus_eNoFECLock:  hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eNoFECLock;  break;
		case BADS_Local_Params_AcqStatus_eLockedFast: hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eLockedFast; break;
		case BADS_Local_Params_AcqStatus_eLockedSlow: hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eLockedSlow; break;
		case BADS_Local_Params_AcqStatus_eEarlyExit:  hChn->pChnStatus->AcqStatus_te = BADS_3x7x_ChnStatus_AcqStatus_eEarlyExit;  break;
		default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED AcqStatus")); break;
	}

	/*Get the Spectrum State from the hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b*/
	h->pChnStatus->AI_te = (hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b ==	true) ? BADS_3x7x_ChnScanStatus_Spectrum_eFlipped : BADS_3x7x_ChnScanStatus_Spectrum_eNormal;

	/*Get the FEC type, Interleaver Dept, Spectral Inversion State from the IC*/
	ReadReg_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_FECU);
	if ((ReadReg_u32 & 0x00A00000) == 0x00000000)
	{
		h->pChnStatus->AB_te = BADS_3x7x_ChnStatus_Annex_eAnnexA;
		h->pChnStatus->Interleaver_u32 = 0;
	}
	else if ((ReadReg_u32 & 0x00800000) == 0x00800000)	
	{
		h->pChnStatus->AB_te = BADS_3x7x_ChnStatus_Annex_eAnnexB;	/*Annex B Mode*/
		h->pChnStatus->Interleaver_u32 = BREG_ReadField(hChn->hRegister, DS_FEC, IDS);
	}
	else
	{
		BDBG_WRN(("ERROR!!! UNSUPPORTED OR UNDEFINED FEC_MODE"));
	}

	/*Get the QAM Mode from the bits/symbol from the FEC*/
	ReadReg_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_FECL);
	ReadReg_u32 = (ReadReg_u32 & 0x000000F0)>>4;
	switch (ReadReg_u32)
	{
		case 3: h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam16; break;			/*16 QAM Mode*/
		case 4: h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam32; break;			/*32 QAM Mode*/
		case 5: h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam64; break;			/*64 QAM Mode*/
		case 6: h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam128; break;		/*128 QAM Mode*/
		case 7:	h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam256; break;		/*256 QAM Mode*/
		case 8:	h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam512; break;		/*512 QAM Mode*/
		case 9:	h->pChnStatus->BPS_te = BADS_3x7x_ChnStatus_BPS_eQam1024; break;		/*1024 QAM Mode*/
		default: BDBG_MSG(("WARNING!!! UNSUPPORTED OR UNDEFINED QAM_MODE")); break;
	}

	/*Get the following from AcqParam->BADS_Local_Params structure*/
	h->pChnStatus->B64_te   = (hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->B256_te  = (hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->B1024_te = (hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A16_te   = (hChn->pChnAcqParam->BADS_Local_Params.Q16A_b   == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A32_te   = (hChn->pChnAcqParam->BADS_Local_Params.Q32A_b   == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A64_te   = (hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A128_te  = (hChn->pChnAcqParam->BADS_Local_Params.Q128A_b  == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A256_te  = (hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A512_te  = (hChn->pChnAcqParam->BADS_Local_Params.Q512A_b  == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
	h->pChnStatus->A1024_te = (hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b == true) ? BADS_3x7x_ChnStatus_eEnable : BADS_3x7x_ChnStatus_eDisable;
		
	/*Get the Spectrum State from the hChn->pChnAcqParam->BADS_Local_Params.FECSpectrum_te*/
	switch (hChn->pChnAcqParam->BADS_Local_Params.FECSpectrum_te)
	{                                                                              
		case BADS_Local_Params_FECSpectrum_eNotInverted:           h->pChnStatus->SI_te = BADS_3x7x_ChnStatus_eDisable; break;
		case BADS_Local_Params_FECSpectrum_eInverted:              h->pChnStatus->SI_te = BADS_3x7x_ChnStatus_eDisable; break;
		case BADS_Local_Params_FECSpectrum_eNotInvertedAutoInvert: h->pChnStatus->SI_te = BADS_3x7x_ChnStatus_eEnable;  break;
		case BADS_Local_Params_FECSpectrum_eInvertedAutoInvert:    h->pChnStatus->SI_te = BADS_3x7x_ChnStatus_eEnable;  break;
		default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED FECSpectrum")); break;
	}

	/*Get ReSync_Count and ReAck_Count from Status->BADS_3x7x_ChnLockStatus structure*/
	h->pChnStatus->ReAck_Count_u32  = hChn->pChnLockStatus->ReAck_Count_u32;
	h->pChnStatus->ReSync_Count_u32 = hChn->pChnLockStatus->ReAcquire_Count_u32;

	/*Get AcquisitionTime1 and AcquisitionTime2 from the hChn->pChnAcqParam->BADS_Local_Params structure*/
	h->pChnStatus->AcquisitionTime1_u32 = hChn->pChnAcqParam->BADS_Local_Params.ElapsedTime_u32;
	h->pChnStatus->AcquisitionTime2_u32 = hChn->pChnAcqParam->BADS_Local_Params.TotalTime_u32;
	
	/*Get SNR and SNRAVG from Status->BADS_3x7x_Status1 structure*/
	h->pChnStatus->SNR_db_u32 = hChn->pChnLockStatus->SNR_u32;
	h->pChnStatus->SNRAVG_db_u32 = hChn->pChnLockStatus->SNRAVG_u32;

	/*********************************************************************************************************
	*Gain Calculations, RF Frequency, RF Offset, EQ Main Tap
	*********************************************************************************************************/
	/*Get front end status*/
	if (hChn->pChnAcqParam->BADS_Internal_Params.Callback_Enable_te == BADS_Internal_Params_eEnable)
	{
		CallbackFrontend.hTunerChn = (hChn->pCallbackParam[BADS_Callback_eTuner]);
		CallbackFrontend.Mode = BADS_CallbackMode_eRequestMode;
		(hChn->pCallback[BADS_Callback_eTuner])(&CallbackFrontend);	
				
		h->pChnStatus->RF_Frequency_u32 = CallbackFrontend.RF_Freq;
		h->pChnStatus->RF_Offset_i32 = CallbackFrontend.Freq_Offset;
				
		/*check for unknown power levels sent from front end, -32768*/
		if ((CallbackFrontend.PreADC_Gain_x256db == -32768) || (CallbackFrontend.PostADC_Gain_x256db == -32768))   
		{
			h->pChnStatus->FrontEndGain_db_i32 = -32768;
		}
		else
		{
			h->pChnStatus->FrontEndGain_db_i32 = (int32_t)(CallbackFrontend.PreADC_Gain_x256db + CallbackFrontend.PostADC_Gain_x256db);
		} 
	}
	else
	{
		h->pChnStatus->RF_Frequency_u32 = 0;
		h->pChnStatus->RF_Offset_i32 = 0;
		h->pChnStatus->EstChannelPower_dbm_i32 = -32768;
		h->pChnStatus->FrontEndGain_db_i32 = -32768;
	}

	/*AGCB Gain and Level Calculation
	*AGCBVAL is 32-bit 7.25 unsigned number
	*Reset value is 0x04000000 which is a gain of 2
	*db = 20*log10(AGCBVAL/2^25)
	*256*db = 5120*log10(AGCBVAL/2^16)-5120*log10(2^9)*/ 
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_AGCBI, AGCBVAL);
	ReadReg_u32 = 2*BMTH_2560log10(ReadReg_u32/POWER2_16) - LOG10_POWER2_9_X5120;
	h->pChnStatus->AGCB_Gain_db_i32 = (int32_t)ReadReg_u32;  /*BBS will divide by 256 to get dB*/

	/*Get Hum AGC Gain and EQ Gain
	*DS_EQ_AGCPA is a 26 bit signed number
	*DS_EQ_AGCPA.AGCOFFSET is a 8 bit unsigned number
	*Hum_Agc_Gain db = 20*log10(2*(AGCOFFSET*2^18+AGCPA)/2^25)
	*Hum_Agc_Gain 256*db = 5120*log10(2*(AGCOFFSET*2^18+AGCPA)/2^25)
	*Hum_Agc_Gain 256*db = 2*BMTH_2560log10(2*(AGCOFFSET*2^18+AGCPA)) - LOG10_POWER2_25_X5120*/
	ReadReg1_i32 = 0;
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_EQ_CTL, HUM_EN);
	if (ReadReg_u32 == 1)
	{
		ReadReg_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_EQ_AGCPA);
		if ((ReadReg_u32 & 0x02000000) == 0) /*check sign*/
		{
			ReadReg1_i32 = (int32_t)ReadReg_u32;
		}
		else
		{
			ReadReg1_i32 = (int32_t)(ReadReg_u32 | 0xFC000000); /*sign extend*/
		}

		ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_EQ_AGC, AGCOFFSET);
		ReadReg1_i32 = ReadReg1_i32 + (int32_t)(ReadReg_u32<<18);
		ReadReg1_i32 = 2*ReadReg1_i32;

		/*remove negative numbers for log function*/
		ReadReg1_i32 = (ReadReg1_i32 < 0) ? 1 : ReadReg1_i32;

		ReadReg_u32 = (uint32_t)ReadReg1_i32;
		ReadReg_u32 = 2*BMTH_2560log10(ReadReg_u32) - LOG10_POWER2_25_X5120;
		ReadReg1_i32 = (int32_t)ReadReg_u32;
	}

	/*EQ Gain is taken by looking at the main tap real value, the imaginary tap is always frozen at 0
	*Reset value of main Tap I is 0x2000 which is a gain of 2
	*db = 20*log10(MAIN/2^12)
	*256*db = 5120*log10(MAIN)-5120*log10(2^12)*/ 
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_EQ_FFE, MAIN);
	ReadReg_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_EQ_FFEU0+(ReadReg_u32*8));
	ReadReg_u32 = (ReadReg_u32 & 0xffff0000)/POWER2_16;
	h->pChnStatus->EQ_Main_Tap_i32 = ((ReadReg_u32 & 0x00008000) == 0) ? (int32_t)ReadReg_u32 : (int32_t)(ReadReg_u32 | 0xFFFF0000);
	ReadReg_u32 = 2*BMTH_2560log10(ReadReg_u32) - LOG10_POWER2_12_X5120;   
	h->pChnStatus->EQ_Gain_db_i32 = ReadReg1_i32 + (int32_t)ReadReg_u32;  /*BBS will divide by 256 to get dB*/

	/*check for unknown power level sent from front end, -32768*/
	if (CallbackFrontend.External_Gain_x256db == -32768)
	{
		h->pChnStatus->EstChannelPower_dbm_i32 = -32768;
	}
	else
	{
		/*removed EQ gain from calculation*/
		h->pChnStatus->EstChannelPower_dbm_i32 = GAIN_OFFSET - 256*9 - h->pChnStatus->FrontEndGain_db_i32 - h->pChnStatus->AGCB_Gain_db_i32 - (int32_t)CallbackFrontend.External_Gain_x256db;
	}
	/********************************************************************************************************
	*********************************************************************************************************
	*********************************************************************************************************/

	/*Get the Carrier_Scan from hChn->pChnAcqParam->BADS_Local_Params and divide by 256*/
	h->pChnStatus->Carrier_Scan_u32 = hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32/256;

	/***************************************
	*Determine The Carrier Frequency Error
	***************************************/
	RetCode_u32 = BADS_P_Get_CFL_Frequency(hChn, hChn->pChnAcqParam->BADS_Local_Params.SampleRate_u32, &ReadReg1_i32);
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif	
	RetCode_u32 = BADS_P_Get_CFL_Error(hChn, hChn->pChnAcqParam->BADS_Local_Params.SampleRate_u32, &ReadReg2_i32);
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif	
	h->pChnStatus->Carrier_Error_i32 = -1*(ReadReg1_i32 + ReadReg2_i32);

	/*************************************** 
	*Determine The Timing Frequency Error 
	***************************************/
	RetCode_u32 = BADS_P_Get_VID_Error(hChn, hChn->pChnAcqParam->BADS_Local_Params.VIDRate_u32, &ReadReg1_i32);
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif	
	h->pChnStatus->Symbol_Error_i32 = ReadReg1_i32;

	/***********************************
	*Determine The Carrier Phase Error 
	***********************************/
	RetCode_u32 = BADS_P_Get_CPL_Error(hChn, h->pChnStatus->Symbol_Rate_u32, &ReadReg1_i32);
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif	
	h->pChnStatus->Phase_Error_i32 = -1*ReadReg1_i32;

	/***********************************
	*Read Error Counters
	***********************************/
	h->pChnStatus->FEC_Corr_RS_Bits_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_CERC2);
	h->pChnStatus->FEC_Clean_RS_Blocks_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_NBERC2);
	h->pChnStatus->FEC_UCorr_RS_Blocks_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_UERC2);
	h->pChnStatus->FEC_Corr_RS_Blocks_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_CBERC2);

	/***********************************
	*Read BERT Counters
	***********************************/
	h->pChnStatus->PRE_FEC_BERT_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_BERI);
	h->pChnStatus->BERT_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_OI_BER);

	/*Get the Upper_Symbol_Scan and Lower_Symbol_Scan from hChn->pChnAcqParam->BADS_Local_Params*/
	h->pChnStatus->Upper_Symbol_Scan_u32 = hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32;
	h->pChnStatus->Lower_Symbol_Scan_u32 = hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32;

	/*Get the Symbol Rate*/
	RetCode_u32 = BADS_P_Get_TL_Frequency(hChn, hChn->pChnAcqParam->BADS_Local_Params.VIDRate_u32, &ReadReg_u32);
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif	
	h->pChnStatus->Symbol_Rate_u32 = ReadReg_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}
