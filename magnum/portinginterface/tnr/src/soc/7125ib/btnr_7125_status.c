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
 * Module Description:
 *
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bmth.h"
#include "btnr_7125ib_priv.h"
#include "btnr_7125_tune.h"

/*registers needed for this function */
#include "bchp_ds_tuner_anactl_0.h"
#include "bchp_ds_tuner_anactl_1.h"
#include "bchp_ds_qafe_if_0_0.h"
#include "bchp_ds_qafe_if_0_1.h"
#include "bchp_ds_qdsafe_0_0.h"
#include "bchp_ds_qdsafe_0_1.h"
#include "bchp_ds_tuner_0.h"
#include "bchp_ds_tuner_1.h"
#include "bchp_ds_tuner_ref_0.h"
#include "bchp_ds_tuner_ref_1.h"
#include "bchp_ds_tuner_digctl_0_0.h"
#include "bchp_ds_tuner_digctl_0_1.h"

/*needed to calculate the DAGC1 and DAGC2 and DPM gains in the DS cores*/
#include "bchp_ds_tuner_chnz_0.h"
#include "bchp_ds_0.h" 

BDBG_MODULE(btnr_7125_status);

/*******************************************************************************************
 * BTNR_7125_P_Reset_Status_TNR_Core0()	This routine resets the status for the tuner
 *******************************************************************************************/
BERR_Code BTNR_7125_P_Reset_Status_TNR_Core0(BTNR_7125_Handle h)
{
	BERR_Code retCode;

	h->pTnrStatus->Tuner_Ref_Lock_Status = 0;
	h->pTnrStatus->Tuner_Mixer_Lock_Status = 0;
	h->pTnrStatus->Tuner_AGC_Status = 0;

	h->pTnrStatus->TunerFreq  = 0;
	h->pTnrStatus->TunerMode  = 0;
	h->pTnrStatus->TunerBW = 0;

	h->pTnrStatus->Cable_LNA_Gain = 0;
	h->pTnrStatus->Cable_RF_Attenuation = 0;
	h->pTnrStatus->Cable_RF_Tracking_Gain = 0;
	h->pTnrStatus->Cable_RF_Tracking_BW = 0;
	h->pTnrStatus->Cable_RF_Tracking_Roof = 0;

	h->pTnrStatus->Terrestrial_LNA_Gain = 0;
	h->pTnrStatus->Terrestrial_RFAGC_Gain = 0;
	h->pTnrStatus->Terrestrial_LNA2_Gain = 0;

	h->pTnrStatus->FGA_Gain = 0;
	h->pTnrStatus->IF_VGA_Gain_Range = 0; 
	h->pTnrStatus->IF_VGA_Control_Value = 0;

	h->pTnrStatus->LNA_AGC_LEVEL = 0;
	h->pTnrStatus->RF_AGC_LEVEL = 0;
	h->pTnrStatus->IF_AGC_LEVEL = 0;
	h->pTnrStatus->IFDCVAL_I = 0;
	h->pTnrStatus->IFDCVAL_Q = 0;

	h->pTnrStatus->LNA_Gain_256db = 0; 			/*Computed in btnr_7125_lna.c*/

	h->pTnrStatus->Cable_HarmonicFilter = 0;
	h->pTnrStatus->Cable_RF_Gain_256db = 0;
	h->pTnrStatus->Cable_RF_TRKGain_256db = 0;
	h->pTnrStatus->FGA_Gain_256db = 0;
	h->pTnrStatus->IF_PGA_Gain_256db = 0; 
	h->pTnrStatus->IF_VGA_Gain_256db = 0; 
	h->pTnrStatus->ADC_IPGA_Gain_256db = 0;  
	h->pTnrStatus->ADC_QPGA_Gain_256db = 0;
	h->pTnrStatus->ADC_EXTPGA_Gain_256db = 0;
	h->pTnrStatus->TotalGainToADC = 0;
	h->pTnrStatus->AGF_Gain_256db = 0;
	h->pTnrStatus->AGF2_Gain_256db = 0;
	h->pTnrStatus->InputPower_256db = 0;

	h->pTnrStatus->LNA_Boost_Status = 0;		/*Computed in btnr_7125_lna.c*/
	h->pTnrStatus->LNA_Tilt_Status = 0;		  /*Computed in btnr_7125_lna.c*/

	/*Tuner Status Reset Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

/*******************************************************************************************
 * BTNR_7125_P_Get_Status_TNR_Core0()	This routine gets the status for the tuner
 *******************************************************************************************/
BERR_Code BTNR_7125_P_Get_Status_TNR_Core0(BTNR_7125_Handle h)
{
	BERR_Code retCode;

	/* Local Variables */
	uint8_t div_sel;
	BTNR_7125_Bandwidth	TunerBw;
	uint8_t IFLPF_BWR_Sel, FGA_RC_CNTL_Sel, FGA_Gain, FGA_RC_Sel;
	uint32_t ReadReg;
	int32_t GainReg, SumGainReg, MaxLevel, MinLevel;
	unsigned unit = h->settings.unitNumber;

	/*Local Variables used for multipliers and dividers in math calculations*/
	uint32_t ulMultA, ulMultB, ulDivisor, ulNrmHi, ulNrmLo;

	/*The PI Needs to call the BTNR_7125_P_Get_LockStatus_TNR_Core0() to take out the following line*/
	BTNR_7125_P_Get_LockStatus_TNR_Core0(h);
	
	/* Read Ref Lock Status */
	/*h->pTnrStatus->Tuner_Ref_Lock_Status is updated by the BTNR_7125_P_Get_LockStatus_TNR_Core0()*/

	/* Read Mixer Lock Status */
	/*h->pTnrStatus->Tuner_Mixer_Lock_Status is updated by the BTNR_7125_P_Get_LockStatus_TNR_Core0()*/

	/* Read AGC Status */
	/*h->pTnrStatus->Tuner_AGC_Status is updated by the BTNR_7125_P_Get_LockStatus_TNR_Core0()*/

	/* Read Tuning Frequency */
	/* This is the reverse calculation of the formula used in the BTNR_7125_P_Set_Tuner_DDFS() routine*/
	/* These is a 1-1 relationship between the ssval1 and the div_sel in the Tuner_Freq_Selection_Table[] in btnr_7125_tune.h*/
	/* div_sel=0->ssval=8,   div_sel=1->ssval=4,   div_sel=2->ssval=32,   div_sel=4->ssval=16,   div_sel=6->ssval=64*/  
	/* TunerFreq = FCW*3^3*5^7/ssval*2^14*/  
											 
	ulMultA = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_0F, FCW, unit); 
	ulMultB = POWER3_3*POWER5_7;
	div_sel = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DIV_SEL, unit);
	switch(div_sel)
	{
		case 0:  ulDivisor = 8*POWER2_14; break;	 
		case 1:  ulDivisor = 4*POWER2_14; break;	 
		case 2:  ulDivisor = 32*POWER2_14; break;	 
		case 4:  ulDivisor = 16*POWER2_14; break;	 
		case 6:  ulDivisor = 64*POWER2_14; break;	 
		default: ulDivisor = 8; BDBG_WRN(("WARNING!!! INVALID TUNER DIVIDER, DIV_SEL = %d", div_sel));
	}
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);
	if (ulNrmHi != 0)
	{
		BDBG_ERR(("ERROR 64to32 division is not in a 32 bit range in BTNR_7125_P_Get_Status_TNR_Core0()"));
	}

	/*assign calculated Tuner Frequency*/
	h->pTnrStatus->TunerFreq = ulNrmLo;

	/* Read Tuning Bandwidth */
	/*Here BW needs to be calculated by reading back the IFLPF_BWR and FGA_RC_CNTL in DS_TUNER_ANACTL_0_WDATA_03*/
	/*Then the programmed values need to be compared to the values prgrammed by BTNR_7125_P_Tuner_FGA_BW_Selection()*/
	/*if the BTNR_7125_P_Tuner_FGA_BW_Selection() breakpoints change then so do these*/
	IFLPF_BWR_Sel = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_BWR, unit);
	FGA_RC_CNTL_Sel = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_RC_CNTL, unit);
	FGA_Gain =  BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);
 
	/*The IFLPF_BWR_BREAKPOINTs are used by the BTNR_7125_P_Tuner_FGA_BW_Selection() function as well*/
	/*Make sure any changes made here get copied to the other function as well*/
	if (IFLPF_BWR_Sel <= IFLPF_BWR_BREAKPOINT1) 
	{	
		FGA_RC_Sel = 0; 
	}
	else if ((IFLPF_BWR_Sel >= IFLPF_BWR_BREAKPOINT2) && (IFLPF_BWR_Sel <= IFLPF_BWR_BREAKPOINT3))
	{
		FGA_RC_Sel = IFLPF_BWR_Sel-3; 
	}
	else
	{
		FGA_RC_Sel = 13;              
	}	
	FGA_RC_CNTL_Sel = FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL;
 
 	TunerBw = BTNR_7125_Bandwidth_e8MHz; /*initial value*/

	if ((IFLPF_BWR_Sel == DS_IFLPF_BWR_SEL_8MHz) && (FGA_RC_CNTL_Sel == FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL))
	{
		TunerBw = BTNR_7125_Bandwidth_e8MHz;
	}
	else if ((IFLPF_BWR_Sel == DS_IFLPF_BWR_SEL_7MHz) && (FGA_RC_CNTL_Sel == FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL))
	{
		TunerBw = BTNR_7125_Bandwidth_e7MHz;
	}
	else if ((IFLPF_BWR_Sel == DS_IFLPF_BWR_SEL_6MHz) && (FGA_RC_CNTL_Sel == FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL))
	{
		TunerBw = BTNR_7125_Bandwidth_e6MHz;
	}
	else if ((IFLPF_BWR_Sel == DS_IFLPF_BWR_SEL_5MHz) && (FGA_RC_CNTL_Sel == FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL))
	{
		TunerBw = BTNR_7125_Bandwidth_e5MHz;
	}
	else
	{
		BDBG_WRN(("WARNING!!! h->pTnrModeData->TunerBW does not match tuner programming in cable more!!"));
	}

	
	/*assign Tuner Bandwidth*/
	h->pTnrStatus->TunerBW = (uint32_t)TunerBw;

	/*The BTNR_7125_P_Get_LNA_Gain function will only get the gain from the right LNA in the Cable or Terrestrial path*/
	if (h->hI2cLNA) {
		h->pTnrModeData->LNA_Gain = BTNR_7125_P_Get_LNA_Gain(h);
	}
	

	/*Get the various gain values from the RF portion of the tuner*/
	/*assign the LNA gain in the cable path and get the gains in the cable RF path*/
	h->pTnrStatus->Cable_LNA_Gain = (uint32_t)h->pTnrModeData->LNA_Gain;
	h->pTnrStatus->Cable_RF_Attenuation = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit);
	h->pTnrStatus->Cable_RF_Tracking_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);
	h->pTnrStatus->Cable_RF_Tracking_BW = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_BW, unit);
	h->pTnrStatus->Cable_RF_Tracking_Roof = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, unit);

	/*Terrestrial is not used*/
	h->pTnrStatus->Terrestrial_LNA_Gain = 0;
	h->pTnrStatus->Terrestrial_RFAGC_Gain = 0;
	h->pTnrStatus->Terrestrial_LNA2_Gain = 0;

	/*Get the baseband gains that are common to Cable and Terrestrial*/
	h->pTnrStatus->FGA_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);
	h->pTnrStatus->IF_VGA_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);
	h->pTnrStatus->IF_VGA_Control_Value = BTNR_7125_P_ReadField(h->hRegister, DS_QAFE_IF_0_0_AGCTI, AGCTDSVAL, unit) ;	/*scripts used the RF integrator for the IF path*/


	/*Get the state of the Harmonic Rejection Filter*/
	/*Which is needed for the calculations in cable*/
	/*The harmonic filter is enabled if FILTER_SEL = 0 or 2 AND HR_EN = 1*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, unit);
	h->pTnrStatus->Cable_HarmonicFilter = ReadReg;			 
	if (ReadReg == 1)
	{
		ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, unit);
		if ((ReadReg == 1) || (ReadReg == 3))
		{
			h->pTnrStatus->Cable_HarmonicFilter = 0; /*HR_EN set but no filter selected*/			 
		}
	}

	/*Calculate the LNA/RF/IF gains in pergentages and assign in 1/10 of a percent*/
	/*Since the tuner control words are over different gains/step and different number gains/block*/
	/*an approximation is needed to map the control values from 0 to 1000*/
	
	/*Calulate LNA gain in 1/10 percent*/
	/*LNA gain has 0 for minimum gain and 31 for maximum gain*/
	ReadReg = (h->pTnrStatus->Cable_LNA_Gain*1000)/31;
	
	/*assign LNA_AGC_LEVEL in 1/10 percent*/
	h->pTnrStatus->LNA_AGC_LEVEL = ReadReg;

	/*Calulate RF gain in 1/10 percent*/
	/*Cable RF Path uses the 3 blocks before the IF VGA*/
	/*h->pTnrStatus->Cable_RF_Attenuation   which is in 3 db steps 4 smallest 0 largest  used when Harmonic Filter is not enabled*/
	/*h->pTnrStatus->Cable_RF_Tracking_Gain which is in 2 db steps 0 smallest 3 largest  used when Harmonic Filter is enabled*/
	/*h->pTnrStatus->FGA_Gain which is in 1.5 db steps 0 smallest 7 largest*/
	/*h->pTnrStatus->IF_VGA_Gain_Range which is in 5 db steps 0 largest 3 smallest (reversed re:7550) */
	/*put all gains on a common scale by multiplying using a 1/256 scale*/
	/*(4-h->pTnrStatus->Cable_RF_Attenuation)*3*256*/
	/*h->pTnrStatus->Cable_RF_Tracking_Gain*2*256*/
	/*h->pTnrStatus->FGA_Gain*1.5*256*/
	/*h->pTnrStatus->IF_VGA_Gain_Range*5*256*/
	/*now we have 0 is smallest and 9600 is largest used when Harmonic Filter is not enabled*/
	/*now we have 0 is smallest and 8064 is largest used when Harmonic Filter is enabled*/			
		
	if (h->pTnrStatus->Cable_HarmonicFilter == 0) /*use h->pTnrStatus->Cable_RF_Attenuation*/
	{	
		ReadReg = (((4-h->pTnrStatus->Cable_RF_Attenuation)*768+h->pTnrStatus->FGA_Gain*384+(3-h->pTnrStatus->IF_VGA_Gain_Range)*1280)*1000)/9600;
	}
	else                                          /*h->pTnrStatus->Cable_RF_Tracking_Gain*/
	{
		ReadReg = ((h->pTnrStatus->Cable_RF_Tracking_Gain*512+h->pTnrStatus->FGA_Gain*384+(3-h->pTnrStatus->IF_VGA_Gain_Range)*1280)*1000)/8064;
	}
	
	/*assign RF_AGC_LEVEL in 1/10 percent*/
	h->pTnrStatus->RF_AGC_LEVEL = ReadReg;
	
		
	/*Calulate IF gain in 1/10 percent*/
	/*The scripts use the RF AGC for the IF control for some reason*/
	/*So use RF integrator to calculate for IF_AGC_LEVEL*/
	/*Read the 2's complement register value of AGCTI and scale it to be offset binary*/
	/*Map the 0x80000000->0x00000000 and the 0x00000000->0x80000000 and the 0x7FFFFFFF->0xFFFFFFFF*/ 
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_QAFE_IF_0_0_AGCTI, AGCTDSVAL, unit);
	ReadReg = (ReadReg + POWER2_31);
	
	/*Map the 0x00000000->0 and the 0xFFFFFFFF->1000 */
	ulMultA = ReadReg;
	ulMultB = 125;
	ulDivisor = POWER2_29;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);
	if ((ulNrmHi != 0))
	{
		BDBG_WRN(("64to32 division is not in a 32 bit unsigned int range"));
	}

	ulNrmLo = (ulNrmLo > 995) ? 1000 : ulNrmLo; 	/*clamp to be 0 to 1000*/
	
	/*assign IF_AGC_LEVEL in 1/10 percent*/
	h->pTnrStatus->IF_AGC_LEVEL = ulNrmLo;
	
	/*Read the IFDCVAL_I and IFDCVAL_Q values*/
	h->pTnrStatus->IFDCVAL_I    = BTNR_7125_P_ReadField(h->hRegister, DS_QAFE_IF_0_0_IFDCI, IFDCVAL, unit);
	h->pTnrStatus->IFDCVAL_Q    = BTNR_7125_P_ReadField(h->hRegister, DS_QAFE_IF_0_0_IFDCI_Q, IFDCVAL_Q, unit);

/**************************************************************************************************/

	/*Get the LNA Gain, it is calculated in 1/256 db by btnr_7125_lna.c*/	
	/*The LNA values for h->pTnrStatus->LNA_Gain_256db are read in the LNA.C routine*/
	/*The LNA values for h->pTnrStatus->h->pTnrStatus->LNA_Boost_Status are read in the LNA.C routine*/
	/*The LNA values for h->pTnrStatus->h->pTnrStatus->LNA_Tilt_Status are read in the LNA.C routine*/
	/*Also calculate a running gain total using SumGainReg*/

	if (h->hI2cLNA) {
		BTNR_7125_P_Get_LNA_Status(h);
	}
	SumGainReg = (int32_t)h->pTnrStatus->LNA_Gain_256db;
	
	/*Get the Cable_RF_Gain, it is reported in 1/256 db*/
	/*This gain is used if h->pTnrStatus->Cable_HarmonicFilter == 0*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit);
	GainReg = (-3*ReadReg)<<8;			 
	if (ReadReg > 4)
	{
			BDBG_WRN(("WARNING!!! Cable_RF_Gain is out of range, RFFE_ATTDB = %d",ReadReg));
	}
	/*Add to SumGainReg if Cable and Cable_Harmonic_Filter not-enabled*/
	if (h->pTnrStatus->Cable_HarmonicFilter == 0)
	{
		SumGainReg = SumGainReg + GainReg;
	} 
	/*assign Cable_RF_Gain_256db in 1/256 db*/
	h->pTnrStatus->Cable_RF_Gain_256db = (uint32_t)GainReg;	

	/*Get the Cable_RF_TRKGain, it is reported in 1/256 db*/
	/*This gain is used if h->pTnrStatus->Cable_HarmonicFilter == 1*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);
	switch(ReadReg)
	{
		case 0:  GainReg = -768; break;	/*-3.0*256*/
		case 1:  GainReg =  128; break;	/* 0.5*256*/
		case 2:  GainReg =  384; break;	/* 1.5*256*/
		case 3:  GainReg =  845; break;	/* 3.3*256*/
		default: BDBG_WRN(("WARNING!!! Invalid RFTRK1_GAIN detected, RFTRK1_GAIN = %d", ReadReg));
	}
	/*Add to SumGainReg if Cable and Cable_Harmonic_Filter enabled*/
	if (h->pTnrStatus->Cable_HarmonicFilter == 1)
	{
		SumGainReg = SumGainReg + GainReg;
	} 
	/*assign Cable_RF_TRKGain_256db in 1/256 db*/
	h->pTnrStatus->Cable_RF_TRKGain_256db = (uint32_t)GainReg;
	
	/*Get the baseband gains that are common to Cable and Terrestrial*/
	/*Get the FGA Gain, it is reported in 1/256 db*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);
	/*Add 1.5 db for div_sel = 2 in DS_TUNER_ANACTL_0_WDATA_02*/
	if (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DIV_SEL, unit) == 1)
	{
		GainReg = (5376+ReadReg*384);		/*scaled by 256, Gain db = (21.0 + 1.5*ReadReg)<<8*/
	}
	else
	{
		GainReg = (4992+ReadReg*384);		/*scaled by 256, Gain db = (19.5 + 1.5*ReadReg)<<8*/
	}
	h->pTnrStatus->FGA_Gain_256db = (uint32_t)GainReg;
	/*Add to SumGainReg for Cable and Terrestrial*/
	SumGainReg = SumGainReg + GainReg;
	/*assign FGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->FGA_Gain_256db = (uint32_t)GainReg;


 	/*Get the IF PGA Gain, it is reported in 1/256 db*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit); 
	#if 0	/* 7550  0 min gain, 3 max gain*/
	GainReg = (-15+5*ReadReg)<<8;		/*scaled by 256*/
	#else	/* 7125  0 max gain, 3 min gain*/
	GainReg = (-5*ReadReg)<<8;	/*scaled by 256*/
	#endif
	/*Add to SumGainReg for Cable and Terrestrial*/
	SumGainReg = SumGainReg + GainReg;
	/*assign IF_PGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->IF_PGA_Gain_256db = GainReg;
	BDBG_MSG(("%s() IF_PGA_Gain_256db %d",__FUNCTION__,(int32_t)h->pTnrStatus->IF_PGA_Gain_256db));

	/*Get the IF VGA Gain, it is reported in 1/256 db*/
	/*This is only an estimate and maps MaxLevel and MinLevel of the AGC interator in the DS core over the 20db or 30 db VGA range*/
	/*The AGCTDSVAL is a 2's complement register*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_QAFE_IF_0_0_AGCTI, AGCTDSVAL, unit);
	GainReg = (int32_t)ReadReg;
	/*Clamp the values to match 1.1v to 1.8V in the tuner*/
	/*The integrators can go outside this range but the change in VGA gain is minimal*/
	MaxLevel = MAX_GAIN_VGA_LEVEL;
	MinLevel = MIN_GAIN_VGA_LEVEL;
	GainReg = (GainReg > MaxLevel) ? MaxLevel : GainReg; /*clamp if GainRange > MaxLevel*/
	GainReg = (GainReg < MinLevel) ? MinLevel : GainReg; /*clamp if GainRange < MinLevel*/

	/*Normalize the value to 0 to 65535*/
	GainReg = ((GainReg>>2)-(MinLevel>>2))/(((MaxLevel>>2)-(MinLevel>>2))>>16);	/*scaled by 256*256*/
	#if 0 /*MCW not in 7125 */
	/*The IF VGA has 4 different settings to map the GainReg to based on VGA_GAIN_INC_GAIN and VGA_GAIN_INC_RANGE in DS_TUNER_ANACTL_0_WDATA_21*/
	/*Add the Offset to the normalized and scaled value*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_21, VGA_GAIN_INC_GAIN, unit);
	if (ReadReg == 0)
	{
		ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_21, VGA_GAIN_INC_RANGE, unit);
		GainReg = (ReadReg == 0) ? (-262144 + 20*GainReg)>>8 : (-917769 + 30*GainReg)>>8;	/*Gain Range 16 db to -4 db OR 16 db to -14 db: scaled by 256*/
	}
	else
	{
		ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_21, VGA_GAIN_INC_RANGE, unit);
		GainReg = (ReadReg == 0) ? (65536 + 20*GainReg)>>8 : (-589824  + 30*GainReg)>>8;	/*Gain Range 21 db to 1 db OR 21 db to -9 db: scaled by 256*/
	}
	#else /*7125 version */
	GainReg = (-196608 + 20*GainReg)>>8; /*Gain Range 17 db to -3 db*/
	#endif
	/*Add to SumGainReg for Cable and Terrestrial*/
	SumGainReg = SumGainReg + GainReg;
	/*assign IF_VGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->IF_VGA_Gain_256db = (uint32_t)GainReg;


	/*Get the PGA Gains in front of the ADC, in 1/256 db*/
	/*There is an external PGA, and an I and Q for using the internal tuner*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_I_PGASET, unit);
	GainReg = (-15+ReadReg)<<8;		/*scaled by 256*/
	/*Add to SumGainReg for the I channel only*/
	SumGainReg = SumGainReg + GainReg;
	/*assign ADC_IPGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->ADC_IPGA_Gain_256db = GainReg;
	
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, unit);
	GainReg = (-15+ReadReg)<<8;		/*scaled by 256*/

	if (h->pTnrStatus->ADC_IPGA_Gain_256db != h->pTnrStatus->ADC_IPGA_Gain_256db)
	{
		BDBG_WRN(("WARNING!!! IPGA and QPGA PGA values are different before ADC"));
		BDBG_WRN(("INT_Q_PGASET = %d, INT_Q_PGASET = %d",h->pTnrStatus->ADC_IPGA_Gain_256db,h->pTnrStatus->ADC_QPGA_Gain_256db));
	}
	/*assign ADC_QPGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->ADC_QPGA_Gain_256db = GainReg;
	
	/*get PGA gain if using external ADC path, bypassing the tuner*/
	/*Tis does not gets summed into the total gain calculation, it is a test path*/
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_QDSAFE_0_0_CTRL1, EXT_IF_PGASET, unit);
	GainReg = (-15+ReadReg)<<8;		/*scaled by 256*/
	/*assign ADC_EXTPGA_Gain_256db in 1/256 db*/
	h->pTnrStatus->ADC_EXTPGA_Gain_256db = GainReg;
	
	/*Report total gain to the ADC in 1/256 db*/
	/*This is an estimate of the sum of all of the gains up until the ADC*/
	/*assign TotalGainToADC in 1/256 db*/
	h->pTnrStatus->TotalGainToADC = (uint32_t)SumGainReg;
 
	/*Get the digital gains from DS*/
	/*Get the AGF Gain in the DS Core in 1/256 dB*/
	/*AGFVAL is 24-bit 5.19 unsigned number*/
	/*  db = 20*log10(AGFVAL/2^19)-20*log10(2)*/
	/*  256*db = 5120*log10(AGFVAL)-5120*log10(2^19)-5120*log10(2)*/ 
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_CHNZ_0_AGFI, AGFVAL, unit);
	GainReg = (BTNR_7125_P_2560log10(ReadReg)<<1)-30825;   
	/*Add to SumGainReg for Cable*/
	SumGainReg = SumGainReg + GainReg;
	/*assign AGF_Gain_256db in 1/256 db*/		
	h->pTnrStatus->AGF_Gain_256db  = GainReg;

	/*Get the AGCB Gain in the DS Core in 1/256 dB*/
	/*AGCBVAL is 32-bit 7.25 unsigned number*/
	/*  db = 20*log10(AGCBVAL/2^25)-20*log10(2)*/
	/*  256*db = 5120*log10(AGCBVAL)-5120*log10(2^25)-5120*log10(2)*/ 
	ReadReg = BTNR_7125_P_ReadField(h->hRegister, DS_0_AGCBI, AGCBVAL, unit);
	GainReg = (BTNR_7125_P_2560log10(ReadReg)<<1)-40073;  
	/*Add to SumGainReg for Cable*/
	SumGainReg = SumGainReg + GainReg;	
	/*assign AGF2_Gain_256db in 1/256 db*/		
	h->pTnrStatus->AGF2_Gain_256db = GainReg;

	/* Compute Input Power  for DS*/
	/*this is the sum of all of the gains from LNA to the second digital AGC in the DS core*/
	/*the -7304 is a scale factor to match SumGainReg to a 0 dbmv input power level @ 207 MHz*/
	/*assign InputPower_256db in 1/256 db*/
	h->pTnrStatus->InputPower_256db = (-7304 - SumGainReg); 

	/*Tuner Status Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

BERR_Code BTNR_7125_P_Get_DPM(BTNR_7125_Handle h)
{
	BERR_Code retCode;
	unsigned unit = h->settings.unitNumber;
	unsigned long rfFreq =  h->settings.rfFreq;
	#if 0
	BTNR_7125_AdsInterruptData intinfo;
	#endif
	/*Local variables*/
	uint8_t DPM_I_OUTDIV_M6;
	uint32_t DCIReg, DCQReg, ReadReg, ulNrmHi, ulNrmLo, DPMFCW;
	int32_t SumGainReg;

	/*Get the DPM Tone Frequency*/
	switch (h->pTnrModeData->TunerBw)
	{
	case BTNR_7125_Bandwidth_e8MHz : 		 
		DPM_I_OUTDIV_M6 = DPM_8MHZ_OUTDIV_M6;
		DPMFCW = DPM_8MHZ_DPMFCW;
		break;
	case BTNR_7125_Bandwidth_e7MHz :
		DPM_I_OUTDIV_M6 = DPM_7MHZ_OUTDIV_M6; 
		DPMFCW = DPM_7MHZ_DPMFCW;
		break;
	case BTNR_7125_Bandwidth_e6MHz : 		 
		DPM_I_OUTDIV_M6 = DPM_6MHZ_OUTDIV_M6; 
		DPMFCW = DPM_6MHZ_DPMFCW;
		break;
	case BTNR_7125_Bandwidth_e5MHz : 
		DPM_I_OUTDIV_M6 = DPM_5MHZ_OUTDIV_M6; 
		DPMFCW = DPM_5MHZ_DPMFCW;
		break;
	default :
		DPM_I_OUTDIV_M6 = DPM_6MHZ_OUTDIV_M6;	 
		DPMFCW = DPM_6MHZ_DPMFCW;
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected in BTNR_7125_P_Get_DPM , Value is %d",h->pTnrModeData->TunerBw));
		break;
	}
	#if 0
	/* just do it here */
	/*Set up the DPM in the ADS Core*/
	intinfo.action = BTNR_7125_SetDpm;
	intinfo.iOutdivMs = DPM_I_OUTDIV_M6;
	h->InterruptCallbackInfo.func(h->InterruptCallbackInfo.pParm1, h->InterruptCallbackInfo.Parm2, &intinfo);
	#else
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMFCW, DPMFCW, unit); 	/*FCW*/ 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMNOTCHBWC, 0x04, unit);	/*BW=2^-15*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCBYP, 0, unit);	 	/*unbypass*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMCMULTBYP, 0, unit);	/*enable entire block*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCRST, 1, unit);		/*reset the DPM integrator*/

	/*Unfreeze the DPM integrator*/
	/*This uses the DS_FRZ and so does the acquisition script*/
	BTNR_7125_P_WriteField(h->hRegister, DS_0_FRZ,	DPMDCFRZ, 0, unit);

	#endif

	/*Set the tone location*/	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTDIV_M6, DPM_I_OUTDIV_M6, unit);		 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_RESET_M, 0x40, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_RESET_M, 0x00, unit);

	/*Enable the DPM generator and configure the gain*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_RST, 1, unit); 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_RST, 0, unit);		 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_AMP, 4, unit); /* set smaller amp/gain */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DPM_6DB_GAIN, 0, unit);		 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 0, unit); /* power up */
	BKNI_Sleep(2);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DPM_6DB_GAIN, 1, unit); /* bring up slower */
	BKNI_Sleep(2);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_AMP, 3, unit);  /* bring up slower */

	if (rfFreq < 330000000)
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, EN_DC_DPM, 1, unit); /* increase RFF Att first */
	}
	else
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, EN_DC_DPM, 0, unit);
	}

	#if 0
	/*Reset up the DPM in the ADS Core*/
	intinfo.action = BTNR_7125_ResetDpm;
	intinfo.iOutdivMs = DPM_I_OUTDIV_M6;
    h->InterruptCallbackInfo.func(h->InterruptCallbackInfo.pParm1, h->InterruptCallbackInfo.Parm2, &intinfo);
	#else
	/* just do it here */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCRST, 1, unit);
	#endif
	/*wait for DPM to converge*/
	BKNI_Sleep(50);

	/*Calculate Gains*/

	/* Calculate DPM Gain scaled by 256*/
	/* DPM_DCI and DPM_DCQ are 32-bit unsigned numbers in 1.31 format*/
	/* The BTNR_7125_P_2560log10() breaks down if X is greater than 2^24 */
	/* Eq from original BBS script */ 
	/* db = 20*log10(sqrt(DPM_DCI/2^32*DPM_DCI/2^32 + DPM_DCQ/2^32*DPM_DCQ/2^32))*/
	
	/* fixed point implementation*/
	/* db = 10*log10(DPM_DCI*DPM_DCI + DPM_DCQ*DPM_DCQ) - 10*log10(1/2^64)*/
	/* db = 10*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7) - 10*log10(1/2^25)*/
	/* 256*db = 2560*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7)) - 2560*log10(1/2^25)*/
	/* 256*db = 2560*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7)) + 19266)*/ 
  
	/*These are 32 bit two's complement registers*/
	/*Calculate I*I*/
	ReadReg = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_DPM_DCI, unit);
	if ((ReadReg & 0x80000000) != 0)
	{
		if (ReadReg == 0x80000000)
		{
			ReadReg = 0x80000001; /*clamp value*/
		}
		ReadReg = ReadReg ^ 0xFFFFFFFF;	/*invert all bits*/
		ReadReg = ReadReg + 1;					/* add 1 */
	}

	/*square the I level, result will be 62 bits max*/
	BMTH_HILO_32TO64_Mul(ReadReg, ReadReg, &ulNrmHi, &ulNrmLo);
	/*Get (DPM_DCI*DPM_DCI)>>32: by keeping the 32 MSB's, 2 MSB's will be 0*/
	DCIReg = ulNrmHi;

	/*Calculate Q*Q*/
	ReadReg = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_DPM_DCQ, unit);
	if ((ReadReg & 0x80000000) != 0)
	{
		if (ReadReg == 0x80000000)
		{
			ReadReg = 0x80000001; /*clamp value*/
		}
		ReadReg = ReadReg ^ 0xFFFFFFFF;	/*invert all bits*/
		ReadReg = ReadReg + 1;					/* add 1 */
	}

	/*square the Q level, result will be 62 bits max*/
	BMTH_HILO_32TO64_Mul(ReadReg, ReadReg, &ulNrmHi, &ulNrmLo);
	/*Get (DPM_DCI*DPM_DCI)>>32: by keeping the 32 MSB's, 2 MSB's will be 0*/
	DCQReg = ulNrmHi;

	/*sum the I*I and Q*Q result will be 31 bits max, shift 7 to get into range of 24 bits*/
	/*Get ((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32)>>7)*/
	SumGainReg = (int32_t)BTNR_7125_P_2560log10((DCIReg+DCQReg)>>7) + 19266;

	/*Get LNA Gain scaled by 256*/
	if (h->hI2cLNA) {
		BTNR_7125_P_Get_LNA_Status(h);
	}
	SumGainReg = SumGainReg + (int32_t)h->pTnrStatus->LNA_Gain_256db;
	
	/*Get AGF and AGF2 Gains scaled by 256*/
	BTNR_7125_P_Get_Status_TNR_Core0(h);
	SumGainReg = SumGainReg + (int32_t)h->pTnrStatus->AGF_Gain_256db;
	SumGainReg = SumGainReg + (int32_t)h->pTnrStatus->AGF2_Gain_256db;

	/*DPM_Gain_256db = DPM_OFFSET_256db - LNA_Gain_256db - DPM_Gain_256db - AGF_Gain_256db - AGF2_Gain_256db*/
	SumGainReg = DPM_OFFSET_256dbmV - SumGainReg;
 
	/*assign the output and convert from dbm to dbmV or 256*48.75*/
	h->pTnrStatus->DPM_Gain_256dbmV = (uint32_t)(SumGainReg + 12480);

	/*Turn off DPM Tone*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 1, unit);	

	/*DPM Complete*/
	BDBG_MSG((" BTNR_7125_P_Get_DPM(%d) Complete %d.%02d (%d)\n",unit,((int)h->pTnrStatus->DPM_Gain_256dbmV)/256,
			  (h->pTnrStatus->DPM_Gain_256dbmV%256*100)/256, ((int)h->pTnrStatus->DPM_Gain_256dbmV)));
  
	retCode = BERR_SUCCESS;
  return retCode;
}

