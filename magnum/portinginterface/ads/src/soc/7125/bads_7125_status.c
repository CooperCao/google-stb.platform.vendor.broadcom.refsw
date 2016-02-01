/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#include "bstd.h"
#include "bads.h"
#include "bads_priv.h"
#include "bads_7125_priv.h"
#include "bads_7125_acquire.h"
#include "bmth.h"

/*registers needed for the functions in this file*/
#include "bchp_ds_0.h"
#include "bchp_ds_1.h"
#include "bchp_ds_eq_0.h" 
#include "bchp_ds_eq_1.h" 
#include "bchp_ds_tuner_chnz_0.h"
#include "bchp_ds_tuner_chnz_1.h"

BDBG_MODULE(bads_7125_status);

/************************************************************************************************
 * BADS_7125_ResetStatus()	 This routine resets the status parameters 
 ************************************************************************************************/
BERR_Code BADS_7125_ResetStatus(BADS_ChannelHandle hChnDev)
{
	BERR_Code retCode;
	BADS_7125_ChannelHandle  h = (BADS_7125_ChannelHandle) (hChnDev->pImpl); 
	unsigned int chn = h->chnNo;

	/*Reset parts of the BADS_7125_P_Status_t structure*/	
/*	h->pStatus->SNR = 0;
	h->pStatus->SNRAVG = 0;*/
	h->pStatus->BerPreRS = 0;
	h->pStatus->BerPostRS = 0;
	h->pStatus->BERTMode = 0;
	h->pStatus->DS_CERC2 = 0;
	h->pStatus->DS_NBERC2 = 0;
	h->pStatus->DS_UERC2 = 0;
	h->pStatus->DS_CBERC2 = 0;
	h->pStatus->FEC_Corr_RS_Blocks = 0;
	h->pStatus->FEC_UCorr_RS_Blocks = 0;
	h->pStatus->FEC_Clean_RS_Blocks = 0;
	h->pStatus->ReAcquireCount = 0;
	h->pStatus->ReSyncCount = 0;
	h->pStatus->pCount = 0;
	h->pStatus->TimingStatus = 0;
	h->pStatus->SpectrumStatus = 0;
	h->pStatus->InterleaverDepth = 0;	
	h->pStatus->IsSpectrumInverted = 0;
	h->pStatus->IfFrequency = 0;
	h->pStatus->ModulationType = BADS_ModulationType_eAnnexBQam64;
	h->pStatus->AGF_Level = 0;
	h->pStatus->AGF2_Level = 0;


	/*Reset Error Counters in chip ONLY for the second set, the first set is reset by the BADS_7125_P_Get_LockStatus_ADS_Core0()*/
	BADS_7125_P_Write32(h->hRegister, BCHP_DS_0_TPFEC, 0x00001F00, chn);
	/*ADS Status Reset Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

/************************************************************************************************
 * BADS_7125_P_Get_Status()	 This routine gets the status parameters 
 ************************************************************************************************/
BERR_Code BADS_7125_P_Get_Status(BADS_ChannelHandle hChannel)
{
	BERR_Code retCode;
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;

	/*Local Variables*/
	uint32_t ReadReg;
	uint8_t FilterFactor, ARFFactor;			      
	uint32_t SampleRate;
		
	/*Local Variables used for multipliers and dividers in math calculations*/
	uint32_t ulMultA, ulMultB, ulDivisor, ulNrmHi, ulNrmLo;


	/**************************************************/
	/*Determine the Auto Acquisition Mode             */
	/**************************************************/
	
	h->pStatus->AutoAcquireMode = h->pAcquireParam->AutoAcquireMode;

	/**************************************************/
	/*Determine the FEC and QAM Modes                 */
	/**************************************************/
	/*Get the FEC type, Interleaver Dept, Spectral Inversion State*/
	ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECU, chn);
	if ((ReadReg & 0x00A00000) == 0x00000000)
	{
		h->pStatus->FecMode = 0;	/*Annex A Mode*/
		h->pStatus->InterleaverDepth = (ReadReg & 0x000F0000);
		ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
		if ((ReadReg & 0x00000008) == 0x00000000)
		{
			h->pStatus->IsSpectrumInverted = 0;
		}
			else
		{
			h->pStatus->IsSpectrumInverted = 1;

		}
	}
	else if ((ReadReg & 0x00800000) == 0x00800000)	
	{
		h->pStatus->FecMode = 1;	/*Annex B Mode*/
		h->pStatus->InterleaverDepth = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FEC, IDS, chn);
		ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
		if ((ReadReg & 0x00000008) == 0x00000000)
		{
			h->pStatus->IsSpectrumInverted = 1;
		}
			else
		{
			h->pStatus->IsSpectrumInverted = 0;

		}
	}
	else
	{
		BDBG_WRN(("ERROR!!! UNSUPPORTED OR UNDEFINED FEC_MODE"));
	}
	
	/*Get the QAM Mode from the bits/symbol from the FEC*/
	ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
	ReadReg = (ReadReg & 0x000000F0)>>4;
	switch (ReadReg)
	{
		case 3: h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam16; break;			/*16 QAM Mode*/
		case 4: h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam32; break;			/*32 QAM Mode*/
		case 5: h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam64; break;			/*64 QAM Mode*/
		case 6: h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam128; break;		/*128 QAM Mode*/
		case 7:	h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam256; break;		/*256 QAM Mode*/
		case 8:	h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam512; break;		/*512 QAM Mode*/
		case 9:	h->pStatus->QamMode = (uint32_t)BADS_QAM_eQam1024; break;		/*1024 QAM Mode*/
		default: BDBG_WRN(("WARNING!!! UNSUPPORTED OR UNDEFINED QAM_MODE")); break;
	}

	/*Determine the Combined QAM/FEC mode for Nexus*/
	if (h->pStatus->FecMode == 0) /*Annex A Mode*/
	{
		switch (ReadReg)
		{
			case 3: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam16; break;
			case 4: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam32; break;
			case 5: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam64; break;
			case 6: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam128; break;
			case 7: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam256; break;
			case 8: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam512; break;
			case 9: h->pStatus->ModulationType = BADS_ModulationType_eAnnexAQam1024; break;
			default: BDBG_WRN(("WARNING!!! UNSUPPORTED OR UNDEFINED QAM_MODE")); break;

		}
	}
	if (h->pStatus->FecMode == 1) /*Annex B Mode*/
	{
		switch (ReadReg)
		{
			case 5: h->pStatus->ModulationType = BADS_ModulationType_eAnnexBQam64; break;  
			case 7: h->pStatus->ModulationType = BADS_ModulationType_eAnnexBQam256; break;
			case 9: h->pStatus->ModulationType = BADS_ModulationType_eAnnexBQam1024; break;
			default: BDBG_WRN(("WARNING!!! UNSUPPORTED OR UNDEFINED QAM/FEC MODE")); break;

		}
	}
	/**************************************************/
	/*Determine QAM and FEC Lock Status               */
	/*This is done in the BADS_7125_P_Get_LockStatus()*/
	/**************************************************/
	
	/*Call the Lock Status Function to update the state of h->pStatus->QamStatus and h->pStatus->FecStatus*/
	/*BADS_7125_P_Get_LockStatus(hChannel);
	h->pStatus->QamStatus
	h->pStatus->FecStatus
	*/

	/*******************************************/
	/*Determine the VID_Sample and Symbol Rates*/
	/*******************************************/

	/*Get the Sample Rate*/
	SampleRate = BADS_7125_P_Get_SampleRate(hChannel);
	h->pStatus->SampleRate = SampleRate; 
	
	/*Get the Symbol Rate which depends on the SampleRate, ARF, FilterFactor, and timing loop NCO value*/
	ARFFactor = BADS_7125_P_Get_ARFFactor(hChannel);
	FilterFactor = BADS_7125_P_Get_FilterFactor(hChannel);

	h->pStatus->SymbolRate = BADS_7125_P_Get_SymbolRate(hChannel, ARFFactor, FilterFactor, SampleRate);

	/****************************************************************************************************/
	/*Calculate the IF Frequency */
	/****************************************************************************************************/
	
	h->pStatus->IfFrequency = (int32_t)BADS_7125_P_Get_IfFrequency(hChannel, SampleRate, ARFFactor);

	/***************************************/
	/*Determine The AGF and AGCB gains     */
	/*Also need then in 1/10 pf a percent  */
	/***************************************/

	/*AGF Gain and Level Calculation*/
	/*AGFVAL is 24-bit 5.19 unsigned number*/
	/*  db = 20*log10(AGFVAL/2^19)-20*log10(2)*/
	/*  256*db = 5120*log10(AGFVAL)-5120*log10(2^19)-5120*log10(2)*/ 
	/*AGF_Level is AGFVAL*1000/2^24 for 1/10 percent*/
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_TUNER_CHNZ_0_AGFI, AGFVAL, chn);
	h->pStatus->AGF = (BADS_7125_P_2560log10(ReadReg)<<1)-30825;  /*BBS will divide by 256 to get dB*/
	
	/*The max gain of AGF is 24 db, scale to 0-48 and clamp the lower limit to 0*/
	ReadReg = (uint32_t)((int32_t) h->pStatus->AGF + 6144);
	if ((ReadReg & 0x80000000) != 0)
	{
		ReadReg = 0;
	}

	ulMultA = ReadReg;
	ulMultB = 1000;
	ulDivisor = 12288;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);
	if ((ulNrmHi != 0))
	{
		BDBG_WRN(("64to32 division is not in a 32 bit unsigned int range"));
	}

	/*clamp to be 0 to 1000*/
	if ((ulNrmLo > 1000))
	{
		ulNrmLo = 1000;
	}
	
	h->pStatus->AGF_Level = ulNrmLo;

	/*AGF2 Gain and Level Calculation*/
	/*AGCBVAL is 32-bit 7.25 unsigned number*/
	/*  db = 20*log10(AGCBVAL/2^25)-20*log10(2)*/
	/*  256*db = 5120*log10(AGCBVAL)-5120*log10(2^25)-5120*log10(2)*/ 
	/*AGF_Level is AGFVAL*1000/2^32 or AGFVAL*125/2^24 for 1/10 percent*/
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_AGCBI, AGCBVAL, chn);
	h->pStatus->AGF2 = (BADS_7125_P_2560log10(ReadReg)<<1)-40073;  /*BBS will divide by 256 to get dB*/
	
	/*The max gain of AGF is 36 db, scale to 0-72 and clamp the lower limit to 0*/
	ReadReg = (uint32_t)((int32_t) h->pStatus->AGF2 + 9216);
	if ((ReadReg & 0x80000000) !=0)
	{
		ReadReg = 0;
	}

	ulMultA = ReadReg;
	ulMultB = 1000;
	ulDivisor = 18432;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);
	if ((ulNrmHi != 0))
	{
		BDBG_WRN(("64to32 division is not in a 32 bit unsigned int range"));
	}
	
	/*clamp to be 0 to 1000*/
	if ((ulNrmLo > 1000))
	{
		ulNrmLo = 1000;
	}
	h->pStatus->AGF2_Level = ulNrmLo;

	/****************************************************/
	/*Determine SNR and Average SNR                      */
	/*This is done in the BADS_7125_P_Get_LockStatus()  */
	/****************************************************/

	/*h->pStatus->SNR = 0;    */ /*BBS will divide by 256 to get dB*/
	/*h->pStatus->SNRAVG = 0;  */ /*BBS will divide by 256 to get dB*/

	/*************************************************************************************************/
	/*Get Counter Values of FEC so BBS can get determine BerPreRS and BerPostRS  and counter values  */
	/*************************************************************************************************/

	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_CERC2, CERCCNTVAL, chn);
	h->pStatus->DS_CERC2 = ReadReg;
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_NBERC2, NBERCCNTVAL, chn);
	h->pStatus->FEC_Clean_RS_Blocks = ReadReg - h->pStatus->DS_NBERC2; /* non-accumulated count */
	h->pStatus->DS_NBERC2 = ReadReg;	/* accumulated count */
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_UERC2, UERCCNTVAL, chn);
	h->pStatus->FEC_UCorr_RS_Blocks = ReadReg - h->pStatus->DS_UERC2;
	h->pStatus->DS_UERC2 = ReadReg;
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_CBERC2, CBERCCNTVAL, chn);
	h->pStatus->FEC_Corr_RS_Blocks = ReadReg - h->pStatus->DS_CBERC2;
	h->pStatus->DS_CBERC2 = ReadReg;
	

	/***************************************/
	/*Determine The Timing Frequency Error*/
	/***************************************/

	h->pStatus->TimingFrequencyError = (uint32_t)BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, h->pStatus->SampleRate);

	/***************************************/
	/*Determine The Carrier Frequency Error*/
	/***************************************/

	h->pStatus->CarrierFrequencyError = (uint32_t)BADS_7125_P_Get_CarrierFrequencyError(hChannel, h->pStatus->SampleRate);
	
	/***********************************/
	/*Determine The Carrier Phase Error*/
	/***********************************/
	
	h->pStatus->CarrierPhaseError = (uint32_t)BADS_7125_P_Get_CarrierPhaseError(hChannel, h->pStatus->SymbolRate);
	
	/***********************************/
	/*Get the FEC Error Counters       */
	/***********************************/
	/*
	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_CBERC1, CBERCCNTVAL);
	h->pStatus->FEC_Corr_RS_Blocks = ReadReg;

	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_UERC1, UERCCNTVAL);
	h->pStatus->FEC_UCorr_RS_Blocks = ReadReg;

	ReadReg = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_NBERC1, NBERCCNTVAL);
	h->pStatus->FEC_Clean_RS_Blocks = ReadReg;
	*/
	/***********************************/
	/*Get the Channel Power			   */
	/***********************************/	
	 h->pStatus->ChannelPower = 0;  /*BBS will divide by 256 to get dB*/

	/**********************************************/
	/*Get the ReAcquisitionCount	              */
	/*This is done in by the autoacquire in the PI*/
	/**********************************************/

	/*the h->pStatus->ReAcquireCount parameter is incremented by BADS_7125_P_Get_LockStatus_ADS_Core0()*/
  /*the h->pStatus->ReSyncCount parameter is incremented by BADS_7125_P_Get_LockStatus_ADS_Core0()*/
	/*the h->pStatus->pCount parameter is incremented by BADS_7125_P_Get_LockStatus_ADS_Core0()*/

	/*************************************************************/
	/*TimngLock, AnnexB 64/256 Qam detect and Spectral Inversion */
	/*This is done in the BADS_7125_P_Get_LockStatus()           */
	/*************************************************************/	
	/*h->pStatus->TimingStatus = 0;    */
	/*h->pStatus->SpectrumStatus = 0;  */

	/*ADS Status Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

