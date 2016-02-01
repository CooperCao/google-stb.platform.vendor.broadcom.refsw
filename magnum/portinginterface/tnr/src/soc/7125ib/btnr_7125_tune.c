/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#include "btnr_7125ib_priv.h"
#include "btnr_7125_tune.h"

/*registers needed for this function */
#include "bchp_ds_tuner_0.h"
#include "bchp_ds_tuner_1.h"
#include "bchp_ds_tuner_digctl_0_0.h"
#include "bchp_us_top.h"
#include "bchp_ds_qdsafe_0_0.h"
#include "bchp_ds_qafe_if_0_0.h" 
#include "bchp_ds_tuner_anactl_0.h"
#include "bchp_ds_tuner_ref_0.h"
#include "bchp_ds_tuner_chnz_0.h"
#include "bchp_ds_tuner_core_gen_0_0.h"

BDBG_MODULE(btnr_7125_tune);

static void RoofSel( BTNR_7125_Handle h );

/*******************************************************************************************
 * BTNR_7125_P_Init_TNR_Core0()		This routine initializes the tuner and is only run once
 *******************************************************************************************/
BERR_Code BTNR_7125_P_Init_TNR_Core0(BTNR_7125_Handle h)
{
	BERR_Code retCode;

	/* local variables */
	uint8_t Ref_PLL_Status;
	SignalSource_t InputSource;
	unsigned unit = h->settings.unitNumber;

    BTNR_7125_P_Tuner_Device_type(h); /* detect 60 vs. 65 nm. */

	/* Initialize the local part (NOT PI PART!!) of the BTNR_P_7125_AcquireSettings_t Structure*/
	/* This only happens once, the first time BTNR_7125_P_Init_TNR_Core0() is called*/
	/* The DoneFirstTimeFlag will be set at the end of the BTNR_7125_P_Tune_TNR_Core0()*/	
	if (h->pTnrModeData->DoneLoadFlag == BTNR_OFF) 
	{
		BTNR_7125_P_Tuner_Init_Buffer(h);
		h->pTnrModeData->DoneLoadFlag = BTNR_ON;				/*set DoneLoadFlag*/	
		BDBG_MSG(("INITIALIZING THE BTNR_P_7125_AcquireSettings_t STRUCTURE"));
	}

	/*clear the lock status*/
	h->pTnrStatus->Tuner_Ref_Lock_Status = 0;
	h->pTnrStatus->Tuner_Mixer_Lock_Status = 0;

	/* Range check the local Structure*/
	BTNR_7125_P_Range_Check(h);

	/*define local variables*/
	Ref_PLL_Status = 0;
	InputSource =  (SignalSource_t)h->pTnrModeData->BBS_Input_Source;
 
	/********************************************************************************************/
	/********************************************************************************************/
	/********************************************************************************************/
	/*SetUsPll() \/ */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_DIV1 , 0x004E0102, 0); /* Fclk_us_pll = 54.0 * (1 / 2) * 78 / 4 = 526.5 MHz */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_DIV2 , 0x00000000, 0); /* ndiv_fract = 0 */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_US , 0x00000001, 0); /* clk_us enabled */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_OB , 0x00002703, 0); /* clk_ds enabled  (Fclk_ob = Fclk_us_pll * 4 / 39) */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_DS0 , 0x00000c03, 0); /* clk_ds enabled  (Fclk_ds      = Fclk_us_pll * 4 / 12) */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_DS1 , 0x00000c03, 0); /* clk_ds enabled  (Fclk_ds      = Fclk_us_pll * 4 / 12) */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_DS_AFE0 , 0x00001207, 0); /* clk_ds_afe enabled (Fclk_ds_afe  = Fclk_us_pll * 4 / 18)*/
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CLK_DS_AFE1 , 0x00001207, 0); /* clk_ds_afe enabled (Fclk_ds_afe  = Fclk_us_pll * 4 / 18)*/
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_BYP , 0x00000000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_RST , 0x00000000, 0); /* disable resets */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_MISC , 0x00000000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CTRL_1 , 0x00000000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CTRL_0 , 0x57188058, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_PHASE_1 , 0x000002e4, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_PHASE_0 , 0x00000000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_BIAS_CTRL , 0x00004000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_ANALOG_CTRL_1 , 0x00000000, 0); /* default */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_ANALOG_CTRL_0 , 0x00000131, 0); /* default */

	/* Begin set up reference PLL */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0x00, unit); 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0x00, unit); /* DOCSIS code hits this twice. */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, PDB_PHOUT, 0x00, unit); 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_LODIVBY4EN, 0x00, unit); 

	BKNI_Sleep(100);
	#if 0 
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_CORE_EN , 0x0000001, 0); /* in DOCSIS SetUsPll() */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PDAC_EN , 0x0000001, 0); /* in DOCSIS SetUsPll() */
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PDAC_INP_MUX_EN , 0x0000000, 0); /* in DOCSIS SetUsPll() */
	#endif
	if ( BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_REF_0_REF_00,I_VCOBUF_LATCH_ON,0))
	{	/* The other tuner unit has already initialized TUNER_REF, don't disturb it. */
		h->pTnrModeData->DoneFirstTimeFlag = BTNR_ON;
		BDBG_MSG(("TUNER_REF already initialized, set DoneFirstTimeFlag and skip."));
	}
	if (h->pTnrModeData->DoneFirstTimeFlag == BTNR_OFF)
	{
		BDBG_MSG(("Init TUNER_REF."));
		/*SetUsPll() /\*/
		/* This sets up the reference PLL PLL_setup_impl() \/*/
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_REF_0_REF_00, 0x18782F15, 0);
		BKNI_Sleep(1);
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_REF_0_REF_00, 0x18282F15, 0);
		BKNI_Sleep(1);
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_REF_0_REF_00, 0x18202F15, 0);
		BKNI_Sleep(10);
	
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_REF_0_REF_00, 0x18002F15, 0);
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_REF_0_REF_01, 0x0617c0e1, 0);
		BKNI_Sleep(10);
	
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05,BG_CORE_ADJ,0x7, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01,I_QP_ICTRL,0x03, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_VCOBUF_LATCH_ON,0x1, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DIV_FB,0x12, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_02,I_OUTDIV_M0,0x04, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_03,I_OUTDIV_M1,0x16, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_03,I_OUTDIV_M2,0x16, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_03,I_OUTDIV_M3,0xDD, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_03,I_OUTDIV_M4,0x00, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04,I_OUTDIV_M5,0x04, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04,I_OUTDIV_M6,0x16, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_DIV2RST,0x1, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_FBDIVRST,0x1, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_DIV4RST,0x1, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_CLFCNT,0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_VCRST,0x1, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_02,I_CLKEN_M,0x67, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05,IP_LD_EN,0x1, 0);	
	
		/* release feedback divider resets */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_DIV2RST,0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_FBDIVRST,0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_DAC_VCRST,0x0, 0);
	
		BKNI_Sleep(10);
	
		/* release divider resets */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DAC_DIV4RST, 0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_RESET_M, 0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_QP_ICTRL, 0x7, 0);
	
		BKNI_Sleep(10);
	
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, LD_RESET_STRT, 0x0, 0);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, BG_CORE_ADJ, 0x04, 0);
		BKNI_Sleep(10);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, PWRDN_BG_1P2, 0, 0);  
	}
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05,BG_CORE_ADJ,0x7, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05,IP_LD_EN,0x1, unit);	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, BG_CORE_ADJ, 0x04, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, PWRDN_BG_1P2, 0, unit);  

	/* configure AFE, select CM rejection point @ ADC or VGA */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, I_ADC_CM_EN, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, VGA_CM_EN, 0x0, unit);

	/*Get the lock status*/
	Ref_PLL_Status = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, REF_PLL_LOCK_STATUS, unit);
	switch (Ref_PLL_Status)
	{
	case 0 : 	
		BDBG_MSG(("Reference PLL is NOT locked"));
		break;
	case 1 : 	
		BDBG_MSG(("Reference PLL is locked"));
		break;
	default :
		BDBG_ERR(("ERROR!!! INVALID Reference PLL Locked Value: value is %d",Ref_PLL_Status));
		break;
	}
	/* PLL_setup_impl() /\*/

	/* InitChannelizer() \/*/
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_AGF, 0x0c080001, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_AGFI,0x00080000, unit);
	BKNI_Sleep(30);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_AGF, AGFBW, 0x06, unit);
	/* InitChannelizer() /\*/

	if ((h->pTnrModeData->DoneFirstTimeFlag == BTNR_OFF) && h->hI2cLNA)
	{
		/*Initialize the LNA*/
		BTNR_7125_P_Init_LNA(h);
		BTNR_7125_P_ReConfig_LNA(h);
	}
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_0_TUNER_TOP_CNTL1, 0x3, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_US_TOP_PLL_CTRL_0, 
						((BTNR_7125_P_Read32(h->hRegister, BCHP_US_TOP_PLL_CTRL_0, 0) & 0xfffffff8) | 0x3), 0);

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, 0, unit);
	#ifdef IFAGC_OD_MODE
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, IF_OD_CTRL, 1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, RF_OD_CTRL, 1, unit);
	#else
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, IF_OD_CTRL, 0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, RF_OD_CTRL, 0, unit);
	#endif

	if (h->pTnrModeData->Min_AGC_Threshold == 0)
	{
		BTNR_7125_P_Tuner_VGA_CAL(h);
	}
	/*Initialization Complete*/
	BDBG_MSG((" BTNR_7125_P_Init_TNR_Core0() Complete\n"));

	retCode = BERR_SUCCESS;
	return retCode;
}

/*******************************************************************************************
 * BTNR_7125_P_Get_LockStatus_TNR_Core0()  This routine gets the status of the tuner
 *******************************************************************************************/
BERR_Code BTNR_7125_P_Get_LockStatus_TNR_Core0(BTNR_7125_Handle h)
{
	BERR_Code retCode;

	/* local variables */
	int32_t mvarAGCTI;
	uint8_t VGA_Gain_Range, FGA_Gain, HR_en, Filter_sel, RF_ATT_Range, RFTRK_Gain_Range; 
	unsigned unit = h->settings.unitNumber;

	/* Check Reference PLL Lock */
	h->pTnrStatus->Tuner_Ref_Lock_Status = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, REF_PLL_LOCK_STATUS, unit);

	/* Check Mixer Lock */
	h->pTnrStatus->Tuner_Mixer_Lock_Status = (unit == 0) ? BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T0_MIXER_PLL_LOCK_STATUS, unit) :
														   BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T1_MIXER_PLL_LOCK_STATUS, unit);

	/*Determine if IFVGA is out of Range or if the BTNR_7125_P_Tuner_LoopVGAGainRange() would run*/
	/*The RF path has fixed gains, if the IF VGA goes below MIN_AGC_THRESHOLD the the IF VGA is adding attenuation and clipping could be occuring unless*/
	/*all of the Fixed Gains are already at their MIN levels, then the signal is just strong*/
	/*The RF path has fixed gains, if the IF VGA goes above MAX_AGC_THRESHOLD the the IF VGA is adding gain and a better solution could be found unless*/
	/*all of the Fixed Gains are already at their MAX levels, then the signal is just weak*/

	/*The Cable mode only sets the VGA_Gain_Range, FGA_Gain and either RF_ATT_Range or RFTRK_Gain_Range at acquisition*/ 
	/*The Terrestrial mode only sets the VGA_Gain_Range at acquisition, LNA2 and FGA_Gain are always fixed and RF_ATT_Range and RFTRK_Gain_Range are not used*/ 
	
	/*Get Gain Values*/
	mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);
	VGA_Gain_Range = (3-BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit));	/*read the VGA gain range*/
	FGA_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);			/*read the FGA gain*/
	HR_en =  BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, unit);				/*read HR mode*/	
	Filter_sel = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, unit);	    /*check filter selection*/
	RF_ATT_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit);	/*read the RF Atten value*/
	RFTRK_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);/*read the Tracking Filter gain value*/

	if (mvarAGCTI < MIN_AGC_THRESHOLD)					/* IF VGA adding attenuation*/
	{
		/*Set status to 0 (out of range) if all gains are not minimum, VGA is attenuating and the paths in front can be reduced in gain, clipping potential!!!!*/
		if ((HR_en == 1) && (Filter_sel == 2)) /*harmonic rejection is ON VGA_Gain_Range, FGA_Gain, and RFTRK_Gain_Range used: {RFTRK_Gain_Range is 0 MinGain to 3 MaxGain}*/
		{
			h->pTnrStatus->Tuner_AGC_Status = ((VGA_Gain_Range > 0) || (FGA_Gain > 0) || (RFTRK_Gain_Range > 0)) ? 0 : 1;    
		}
		else  /*harmonic rejection is OFF, VGA_Gain_Range, FGA_Gain RF_ATT_Range used: {RF_ATT_Range is 3 MinGain to 0 MaxGain}*/
		{
			h->pTnrStatus->Tuner_AGC_Status = ((VGA_Gain_Range > 0) || (FGA_Gain > 0) || (RF_ATT_Range < 3)) ? 0 : 1;    
		} 
	}
	else if( mvarAGCTI > MAX_AGC_THRESHOLD)			/* IF VGA adding gain*/
	{
		/*Set status to 0 (out of range) if all gains are not maximum, VGA is adding gain and the paths in front can be increased in gain, NF potential!!!!*/
		if ((HR_en == 1) && (Filter_sel == 2)) /*harmonic rejection is ON VGA_Gain_Range, FGA_Gain, and RFTRK_Gain_Range used: {RFTRK_Gain_Range is 0 MinGain to 3 MaxGain}*/
		{
			h->pTnrStatus->Tuner_AGC_Status = ((VGA_Gain_Range < 3) || (FGA_Gain < 7) || (RFTRK_Gain_Range < 3)) ? 0 : 1;    
		}
		else  /*harmonic rejection is OFF, VGA_Gain_Range, FGA_Gain RF_ATT_Range used: {RF_ATT_Range is 3 MinGain to 0 MaxGain}*/
		{
			h->pTnrStatus->Tuner_AGC_Status = ((VGA_Gain_Range < 3) || (FGA_Gain < 7) || (RF_ATT_Range > 0 )) ? 0 : 1;    
		} 
	}
	else
	{
		/*IF AGC is in range*/
		h->pTnrStatus->Tuner_AGC_Status = 1;
	}

	/*Lock Check Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

/*******************************************************************************************
 * BTNR_7125_P_Tune_TNR_Core0()		This routine tunes the tuner
 *******************************************************************************************/
BERR_Code BTNR_7125_P_Tune_TNR_Core0(BTNR_7125_Handle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned unit = h->settings.unitNumber;

	/* Range check the local Structure*/
	BTNR_7125_P_Range_Check(h);

	/* this is needed because the Tuner is called from the DS or THD unlocking so nexus will overwrite and BBS values*/ 
	if (h->pTnrModeData->BBS_Force_TunerAcquisitionType != 0)
	{
		switch(h->pTnrModeData->BBS_Force_TunerAcquisitionType)
		{
		case 1:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7125_TunerAcquisitionType_eInitTune;
			break;
		case 2:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7125_TunerAcquisitionType_eTune;
			break;
		case 3:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7125_TunerAcquisitionType_eMiniTune;
			break;
		default:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7125_TunerAcquisitionType_eInitTune;
			BDBG_ERR(("ERROR!!! Invalid Force_TunerAcquisitionType from BBS selected, Value is %d",h->pTnrModeData->BBS_Force_TunerAcquisitionType));
			break;
		}
	}

	/*BBS or the PI wants to Rerun BTNR_7125_P_Init_TNR_Core0()*/
	/*RUN FOR TuneInit, NOT Tune or MiniTune*/
	if ((h->pTnrModeData->BBS_Rerun_Init == BTNR_ON) || (h->pTnrModeData->TunerAcquisitionType == BTNR_7125_TunerAcquisitionType_eInitTune))
	{
		BTNR_7125_P_Init_TNR_Core0(h);
	}

	/*RUN FOR TuneInit and Tune, NOT MiniTune*/
	if (h->pTnrModeData->TunerAcquisitionType != BTNR_7125_TunerAcquisitionType_eMiniTune)
	{
		BTNR_7125_P_Set_Tuner_Freq(h); 	

		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_DCOC, 0x000000CF, unit);		/* BBDCOC BW=2^(-20), rst, freeze*/	 
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AIFLT, 0xD0000000, unit);		/* configure IF AGC thresholds (not using RF loop) */
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AIFUT, 0x70000000, unit);
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_PGA, 0x02000000, unit);		/*PGA in fixed gain mode*/
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGC1, 0x00800071, unit);		/*freeze AGCI, break interaction, reset AGCT, fixed IF*/
        #ifdef IFAGC_OD_MODE
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGC_MISC, 0x000000cc, unit);	/*RF/IF AGC are OD, select AGCI on IF delta-sigma, AGCT and AGCI Frozen*/	
		#else
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGC_MISC, 0x0000000c, unit);	/*RF/IF AGC are push-pull, select AGCI on IF delta-sigma, AGCT and AGCI Frozen*/	
		#endif
		BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGC2, 0x000d07b0, unit);		/*AGCT, Set Acquisition BW = 2^-13, Theshold is 12 dB below sine wave clip level.*/
        BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, 0x00000000, unit);		/*Init AGCI Integrator Value Mid-Range*/

		/*Initialize PGA Starting Values*/
		BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_I_PGASET, 0xA, unit);	/* I Channel PGA initialized at mid gain */
		BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, 0xA, unit);	/* Q Channel PGA initialized at mid gain */

		/*Initialize Tuner Gain Stages*/
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, 0x0, unit);
		/* Min Gain */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x2, unit);
		/* IF FGA 28.5 dB setting, */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, 0x6, unit);
		/* start VGA gain in Max gain state */
		/* BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, 0x3, unit); */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, 0x0, unit);	/* HRW02032010: ray debug */

		/* Set BB LPF Filter for correct Annex mode */
		BTNR_7125_P_Tuner_Set_BB_LPF_Filter(h);
		/* Moved AGC release into BTNR_7125_P_Tuner_AlignAGC2 routine after fixed AGC values are configured */

		BKNI_Sleep(1);

		BTNR_7125_P_Tuner_MXR_DCO_DAC(h);		
		BTNR_7125_P_Tuner_MXR_DCO_LOOP(h);
		BTNR_7125_P_Tuner_VGA_DCO_LOOP(h);
	}

	if (h->pTnrModeData->TunerAcquisitionType == BTNR_7125_TunerAcquisitionType_eMiniTune)
	{
		BDBG_MSG(("BTNR_7125_TunerAcquisitionType_eMiniTune"));
	}
	else if (h->pTnrModeData->TunerAcquisitionType == BTNR_7125_TunerAcquisitionType_eTune)
	{
		BDBG_MSG(("BTNR_7125_TunerAcquisitionType_eTune"));
	}
	/*RUN EVERY TIME: TuneInit, Tune, and MiniTune: Perform AGC Alignment*/
	/*********************************************************************************************************************************************/
	if (h->hI2cLNA) {
		h->pTnrModeData->LNA_Gain = BTNR_7125_P_Get_LNA_Gain(h);			/*Get the LNA Gain*/
	}
	else { /* call back to request the app get and update the LNA gain.  Note, the gain needs to have been updated
			initially, this is not synchronous, and won't return a result in time to be used below. */
		if (h->InterruptCallbackInfo.func) {

			BTNR_7125_AdsInterruptData intinfo;
			BDBG_MSG(("%s() calling UpdateGain callback",__FUNCTION__));
			intinfo.action = BTNR_7125_RequestLnaGain;
			BKNI_EnterCriticalSection();
			h->InterruptCallbackInfo.func(h->InterruptCallbackInfo.pParm1, h->InterruptCallbackInfo.Parm2, &intinfo);
			BKNI_LeaveCriticalSection();
		}
	}

	/* Configure RF Splitter Attenuator Based upon LNA Status */
	if ( (h->pTnrModeData->LNA_Gain <= 31) && (h->pTnrModeData->LNA_Gain > 29) )
	{
	  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 1, 0);
	  BDBG_MSG(("%s() DS_TUNER_REF_0_SPLIT_00, RF_ATT, 1",__FUNCTION__));
	}
	else if ( h->pTnrModeData->LNA_Gain <= 29 )
	{
	  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 2, 0);
	  BDBG_MSG(("%s() DS_TUNER_REF_0_SPLIT_00, RF_ATT, 2",__FUNCTION__));
	}

	BTNR_7125_P_Tuner_AlignAGC2(h, h->pTnrModeData->LNA_Gain);/*Perform Fixed-Gain AGC Alignment for cable*/
	BDBG_MSG(("AlignAGC2 finished"));

	/* HRW02022010: Freeze IF AGC while DCO is performed */
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC1, AGCIZ, 1, unit);
	/* enable tuner DCO loops now that gains are configured */
	BTNR_7125_P_Tuner_MXR_DCO_DAC(h);		
	BTNR_7125_P_Tuner_MXR_DCO_LOOP(h);
	BTNR_7125_P_Tuner_VGA_DCO_LOOP(h);
	/* Release If AGC HRW 02022010 */
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC1, AGCIZ, 0, unit);

	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC2, AGCIFBW, 0x15, unit);  /* Narrow tracking BW, coef = 2e-20 */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_AGF, 0x06080000, unit);	/* Reduce AGF BW */
	#if 0 /* Configure3380DownstreamForAnyQAM_DualLoop() does this at this point... */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCB, 0x0a000f00, chn);		/*Reduce AGCB BW*/ 	 	
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCB_IR, 0x0a000f00, chn);	/*Reduce AGCB BW*/
	#endif
	/* Set up IFDCO ; release IFDCO after AGCs have been set and DCO is stable */
	/* DCOC is on */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_DCOC, 0x00000080, unit);		/* release rst, release DCOC */
	BKNI_Sleep(20);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_DCOC, 0x000000b0, unit);		/* Reduce BBDCOC Bw = 2^-18*/	 

	/* Release IQ imb after AGCs and DCOC have settled */
	/* IQ AMP BW=2^(-14), unfreeze  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_CORE_GEN_0_0_IQAMP, 0x00000060, unit);
	/* IQ PHS BW=2^(-14), unfreeze  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_CORE_GEN_0_0_IQPHS, 0x00000060, unit);

	/*clear the DoneFirstTimeFlag flag*/
	h->pTnrModeData->DoneFirstTimeFlag = BTNR_ON;

	BDBG_MSG(("BTNR_7125_P_Tune_TNR_Core0() Complete: TunerAcquisitionType = %d\n",h->pTnrModeData->TunerAcquisitionType));

	/*Tuning Complete*/
	return retCode;
}



/*******************************************************************************************
 * BTNR_7125_P_Set_Tuner_Freq() this rooutine set the tuner frequency and calls 4 subroutines
 * BTNR_7125_P_Set_Tuner_DDFS(), BTNR_7125_P_Set_Tuner_VCO(), BTNR_7125_P_Set_Tuner_VHForUHF()
 * and BTNR_7125_P_Set_Tuner_LPF() it is the only function that goes 3 levels deep
 *******************************************************************************************/
void BTNR_7125_P_Set_Tuner_Freq(BTNR_7125_Handle h)
{
	/*local variables*/
	uint8_t i,j,part60nm;
	int mix_freq_loc_stat;		/* mixer lock status flag */
	unsigned unit = h->settings.unitNumber;
	part60nm = h->pTnrStatus->Device_Tech_type;
	
  	/* Turns ON the tuner bandgap /LN041709: added this line because bg is OFF on powerup in B0 */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, PWRDN_BG_1P2, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PWDN_BG, 0x0, unit); /*power up QDSAFE BG (added because BG off on powerup in B0*/
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PD_ADC, 0x0, unit);	 /*power up QDSAFE ADC (added because ADC off on powerup in B0*/

	/* Enable(1) Tuner Output Pads for Test if needed */
    BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUT_PAD_EN, 0x00, unit);
	/* Disconnect Mixer PLL Control from Tuner Test Pin */
    BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0x00, unit);

	/* Added to freeze mixer DCO Loop - MDR 09/11/2008 */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_OUT_FRZ, 1, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21, 0x01, unit) ;

	/*Reset QP*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x00, unit);

	/*Clear integrator*/
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_24, 0x0, unit);	/* Clear LSB integrator I[31:0]  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_25, 0x0, unit);	/* Clear MSB integrator I[54:23] */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_26, 0x0, unit);	/* Clear LSB integrator Q[31:0]  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_27, 0x0, unit);	/* Clear MSB integrator Q[54:23] */

	/* Init some registers to help with the IMF */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_0_WDATA_07,0x10338301, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_DIV16_12B, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DAC_IF_HP, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DAC_ICTL, 0x0f, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DAC_GMX, 0x00, unit);

	/* Make sure Test Controls are off (?why do it twice again in bbs?) */
    BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0x00, unit);

	BKNI_Sleep( 1 );

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DAC_GMX, 0x07, unit);

	/* VGA gets its CM from ADC*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, I_ADC_CM_EN, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_MUX_PWRUP, 0x0, unit);	

	/* FG Optimization for SCTE40 */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_15, SPLT_BIAS_CNTL, 0x2, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_15, SPLT_VDD_CNTL, 0x3, unit);

	/*Main tuning routine starts here*/
	/*where in a lookup table Tuner_Freq_Selection_Table{} is the input frequency to get VCO and DDFS programming parameters*/
	i=0;
	j=0;
	while (i < Tuner_Freq_Selection_Table_Size)
	{
		if (h->pTnrModeData->TunerFreq <= Tuner_Freq_Selection_Table[part60nm][i].tuner_freq_range)  
		{
			j = i;
			i = (Tuner_Freq_Selection_Table_Size);
 		}
		else
		{
			i = i + 1;
		}
	}
	
	if ((j==0) || (j==Tuner_Freq_Selection_Table_Size))
	{
		BDBG_ERR(("ERROR Tuner_Freq is not in lookup table in BTNR_7125_P_Set_Tuner_Freq()"));
        if (j==Tuner_Freq_Selection_Table_Size)
        {
            j = j-1;
        }
	}

	/*program divider with value from lookup table*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DIV_SEL, Tuner_Freq_Selection_Table[part60nm][j].div_sel, unit);

	/*set the DDFS*/
	BTNR_7125_P_Set_Tuner_DDFS(h, Tuner_Freq_Selection_Table[part60nm][j].ssval1);

	/*find the optimum IP_CAP_CNTL*/
	/*select tune routine based on freq range*/
	if (Tuner_Freq_Selection_Table[part60nm][j].tunctl == 1)
    {
		/*Program the VCO*/
		BTNR_7125_P_Set_Tuner_VCO(h, Tuner_Freq_Selection_Table[part60nm][j].tunctl); 
	}
	else 
	{
		/*Program the VCO*/
		BTNR_7125_P_Set_Tuner_VCO(h, Tuner_Freq_Selection_Table[part60nm][j].tunctl); 
		if (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_CAP_CNTL, unit) == 63)
		{
			/*Program the VCO*/
			BTNR_7125_P_Set_Tuner_VCO(h, 1); 
		}
	}

	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, RSTCMOSBB, 0x1, unit);
	BKNI_Sleep(1);  

	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, RSTCMOSBB, 0x0, unit);
	BKNI_Delay(1000);   

	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT, 0x0D, unit);
	BKNI_Delay(1000);  

	mix_freq_loc_stat = (unit == 0) ? BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T0_MIXER_PLL_LOCK_STATUS, unit) : 
									  BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T1_MIXER_PLL_LOCK_STATUS, unit) ; 

	if (mix_freq_loc_stat == 1)
		BDBG_MSG(("Mixer PLL is locked."));
	else
		BDBG_MSG(("Mixer PLL is NOT locked."));

	if (h->pTnrModeData->TunerFreq < TUNER_F_RFTRK) 	/*select 3rd harmonic rejection filter*/
	{
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 1, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, 2, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_15, MIXBIAS_EN, 1, unit);
		/*select the Low Pass Filter*/
		BTNR_7125_P_Set_Tuner_LPF(h);
		/*BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, 0x1);*/
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 3, unit);
	}
	else
	{
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, 1, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_BW, 3, unit);
		BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_IN, 7, unit);
		/* BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, 5, unit); */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 2, unit);
	}

	RoofSel(h);
	BDBG_MSG(("Tuner_Freq = %d",h->pTnrModeData->TunerFreq));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].ssval1 = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].ssval1));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].div_sel = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].div_sel));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].tunctl = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].tunctl));
 
	/*Tuning Complete*/
	BDBG_MSG(("BTNR_7125_P_Set_Tuner_Freq() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Set_Tuner_DDFS()   This routine programs the DDFS in the ADS area
 *******************************************************************************************/
void BTNR_7125_P_Set_Tuner_DDFS(BTNR_7125_Handle h, uint8_t ssval1)		
{
	/*local variables*/
	uint32_t ulMultA, ulMultB;
	uint32_t ulNrmHi, ulNrmLo, ulDivisor;
	unsigned unit = h->settings.unitNumber;

	/*check ssval1 is 4,8,16,32, or 64*/
	if ((ssval1 != 4) && (ssval1 !=8) && (ssval1 != 16) && (ssval1 !=32) && (ssval1 !=64))
	{
		BDBG_ERR(("ERROR ssval1 is not in range in BTNR_7125_P_Set_Tuner_DDFS() value received is %d",ssval1));
	}

	/*Reset DDFS FCW and wait to settle*/
	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_DIGCTL_0_0_DIGCTL_40, DDFS_CORE_SRST, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_DIGCTL_0_0_DIGCTL_40, DDFS_CORE_SRST, 0x00, unit);
	BKNI_Delay(200);  

	/*  original code before integer math conversion*/
	/*	(unsigned long)(((float)(tuner_freq / 1000000.0)*ssval1/(32.0*1080.0))*POWER_2TO28);*/ /* Mixer PLL divider =32*/
	ulMultA = h->pTnrModeData->TunerFreq;
	ulMultB = ssval1*POWER2_14;
	ulDivisor = POWER3_3*POWER5_7;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);
	if (ulNrmHi != 0)
	{
		BDBG_ERR(("ERROR 64to32 division is not in a 32 bit range in BTNR_7125_P_Set_Tuner_DDFS()"));
	}

	/*Program the DDFS FCW and wait to settle*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_0F, FCW, ulNrmLo, unit);
	BKNI_Sleep(1);

	/*Tuner DDFS Complete*/
	BDBG_MSG(("DDFS programmed with 0x%x",ulNrmLo));

	BDBG_MSG(("BTNR_7125_P_Set_Tuner_DDFS() Complete\n"));
}


/*******************************************************************************************
 * BTNR_7125_P_Set_Tuner_VCO()    This routine uses VCO to select optimal caps
 *******************************************************************************************/
void BTNR_7125_P_Set_Tuner_VCO(BTNR_7125_Handle h, uint8_t tunctl)
{	
	/*local variables*/
	int8_t i;
	uint8_t cap_cntl, POPCAP_Status;
	int val1=0, val2=0;
	unsigned unit = h->settings.unitNumber;
	BDBG_MSG(("%s(tunctl = %u)",__FUNCTION__,tunctl));

	/*Initial value of IP_CAP_CNTL, this must be 32 or the binary search will go outside of its range*/
	cap_cntl = 32;

	/*check tunectl is 1 or 2*/
	if ((tunctl != 1) && (tunctl !=2))
	{
		BDBG_ERR(("ERROR tunctl is not in range in BTNR_7125_P_Tuner_TuneVCO() value received is %d",tunctl));
	}

	if (tunctl == 1)
	{
		val1 = 32;
		val2 = 11;
	}
	else if (tunctl == 2)
	{
		val1 = 47;
		val2 = 15;
	}

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_VCREF, 0x15, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT, 0x1B, unit);
	
	/*tunctl is used in this write*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_VCO_SEL, tunctl, unit);
	
	/*Initial value of IP_CAP_CNTL*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_CAP_CNTL, cap_cntl, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_CMLDIVRST, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_MANRSTCLF, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, RSTCMOSBB, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_CMLDIVRST, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_MANRSTCLF, 0x0, unit);

	BKNI_Sleep(1);   
		
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, RSTCMOSBB, 0x0, unit);
	BDBG_ASSERT(BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, RSTCMOSBB,unit)== 0x0);

	for(i=5;i>=0;i--)		/*for i = 5 to 0 step -1*/
	{
		/*read the current setting*/
		cap_cntl = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_05, IP_CAP_CNTL, unit);
		BDBG_MSG(("cap_cntl = %u",cap_cntl));

 		/* adjust Rctl and bias based on cap to set loop BW and gain for VCO */

		if ((cap_cntl <= 63) && (cap_cntl >= val1))
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT2, 0x0d, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_RCNTL, 0x00, unit);
		}
		else if ((cap_cntl <= (val1 - 1)) && (cap_cntl >= val2))
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT2, 0x0c, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_RCNTL, 0x01, unit);
		}
		else if ((cap_cntl <= (val2 - 1)) && (cap_cntl >= 5))
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT2, 0x08, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_RCNTL, 0x02, unit);
		}
		else if (cap_cntl <= 4)
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QPBIASCNT2, 0x04, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_RCNTL, 0x03, unit);
		}
		BKNI_Delay(1000);

		/*Read the status*/
		POPCAP_Status = (unit == 0) ? BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T0_MIXER_PLL_POPCAP_STATUS, unit) :
									  BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_0_TUNER_TOP_STATUS, T1_MIXER_PLL_POPCAP_STATUS, unit);

		/*perform a binary search to find the transistion point of POPCAP_Status going 1 to 0*/
		/*this search only works if the initial value of IP_CAP_CNTRL is 0x20 or 32*/
		if (i > 0)
		{
			cap_cntl = (POPCAP_Status == 0) ? cap_cntl+(1<<(i-1)) : cap_cntl-(1<<(i-1));  /*i=5,4,3,2,1: cap_cntl = cap_cntl +-16, +-8, +-4, +-2, +-1*/
		}
		else
		{
			cap_cntl = (POPCAP_Status == 0) ? cap_cntl : cap_cntl-1;

		}

		/*write the cap_cntl then allow settling time*/
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_CAP_CNTL, cap_cntl, unit);
		BDBG_MSG(("VCO Cap setting = 0x%x and VCO Tank used =0x%x unit =%u",cap_cntl,tunctl,unit));

		BKNI_Delay(2000);  

	}

	BDBG_MSG(("Final VCO Cap setting = 0x%x and VCO Tank used =0x%x",cap_cntl,tunctl));
	BDBG_MSG(("BTNR_7125_P_Set_Tuner_VCO(%u) Complete\n",unit));
}

/*******************************************************************************************
 * BTNR_7125_P_Set_Tuner_LPF()    This routine selects the correct LPF corner frequency
 *******************************************************************************************/
void BTNR_7125_P_Set_Tuner_LPF(BTNR_7125_Handle h)
{
	/*local variables*/
	uint8_t i,j;
	uint8_t rftrk1_in;
	unsigned unit = h->settings.unitNumber;

	/*initialize the tracking filter*/
	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0x1, unit);

	/*sequence through the LPF_Selection_Table[] from highest freq value to lowest freq value*/
	/*when we get to the entry with a lower freq then the TunerFreq record index and rftrk1 then exit and program*/
	i = 0;
	j = 0;
	while (i < LPF_Table_Size)
	{
		if (LPF_Selection_Table[i].freq <= h->pTnrModeData->TunerFreq)
		{
			j = i;
			i = LPF_Table_Size;
		}
		else
		{
			i = i + 1;
		}
	}

	if (j==LPF_Table_Size)
	{
		BDBG_ERR(("ERROR Tuner_Freq is not in lookup table in BTNR_7125_P_Set_Tuner_LPF()"));
        j = j-1;
	}
	rftrk1_in = LPF_Selection_Table[j].rftrk1_in;

	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_BW, j, unit);
	BTNR_7125_P_WriteField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_IN, rftrk1_in, unit);
	
	BDBG_MSG(("BTNR_7125_P_Set_Tuner_LPF() Complete\n"));
}

/*******************************************************************************************************************
 * BTNR_7125_P_Set_Tuner_IFVGA_BW()  This routine sets the RF/IF bandwidth in the QAFE
 ******************************************************************************************************************/
void BTNR_7125_P_Set_Tuner_IFVGA_BW(BTNR_7125_Handle h, IFVGA_BW_t IFVGA_BW)
{
	/*local variables*/
	uint32_t RF_AGC_BW, IF_AGC_BW;
	unsigned unit = h->settings.unitNumber;

	/*check IFVGA_BW is in range*/
	if ((IFVGA_BW != IFVGA_BW_eAcquisition) && (IFVGA_BW != IFVGA_BW_eTracking))
	{
		BDBG_ERR(("ERROR IFVGA_BW is not in range in BTNR_7125_P_Set_Tuner_IFVGA_BW() value received is %d",IFVGA_BW));
	}

	if (IFVGA_BW == IFVGA_BW_eAcquisition)
	{
		/*Set the AGC Acquisition bandwidths based on the DS setting*/
		RF_AGC_BW = h->pTnrModeData->BBS_DS_RF_AGC_Acq_BW;
		IF_AGC_BW = h->pTnrModeData->BBS_DS_IF_AGC_Acq_BW;
		BDBG_MSG(("AGC Acquisition BW's set: RF_BW =  0x%x  IF_BW =  0x%x",RF_AGC_BW, IF_AGC_BW));
	}
	else
	{
		/*Set the AGC Tracking bandwidths based on the DS setting*/
		RF_AGC_BW = h->pTnrModeData->BBS_DS_RF_AGC_Trk_BW;
		IF_AGC_BW = h->pTnrModeData->BBS_DS_IF_AGC_Trk_BW;
		BDBG_MSG(("AGC Tracking BW's set: RF_BW =  0x%x  IF_BW =  0x%x",RF_AGC_BW, IF_AGC_BW));
	}

	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC3, AGCTNBW, RF_AGC_BW, unit);	
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC2, AGCIFBW, IF_AGC_BW, unit);
	BDBG_MSG(("BTNR_7125_P_Set_Tuner_IFVGA_BW() Complete\n"));
}

/*******************************************************************************************************************
 * BTNR_7125_P_Set_Tuner_DCOC_Mode()  This routine sets the DCOC in the QAFE
 ******************************************************************************************************************/
void BTNR_7125_P_Set_Tuner_DCOC_Mode(BTNR_7125_Handle h, DCOC_Mode_t DCOC_Mode)
{
	/*local variables*/
	BTNR_OFFON_t DCOC_On;
	uint32_t DCOC_BW;
	unsigned unit = h->settings.unitNumber;

	/*check IFVGA_BW is in range*/
	if ((DCOC_Mode != DCOC_Mode_eFreezeReset) && (DCOC_Mode != DCOC_Mode_eRun))
	{
		BDBG_ERR(("ERROR DCOC_Mode is not in range in BTNR_7125_P_Set_Tuner_DCOC_Mode() value received is %d",DCOC_Mode));
	}

	if (DCOC_Mode == DCOC_Mode_eFreezeReset)
	{
		/*Set the DCO Acquisition BW based on the DS setting*/
		DCOC_BW = h->pTnrModeData->BBS_DS_DCOC_Acq_BW;
		BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_BWC, DCOC_BW, unit);

		/* Freeze DCO */
		BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_Q, 1, unit);
		BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_I, 1, unit);

		/* Reset DCO */	
		BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_Q, 1, unit);
		BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_I, 1, unit);

	BDBG_MSG(("DCOC is Reset and Frozen with initial BW = 0x%x", DCOC_BW));
	}
	else
	{
		/*Enable the DCO based on the DS setting*/
		DCOC_On = (BTNR_OFFON_t)h->pTnrModeData->BBS_DS_DCOC_On;
	
		/*Set the DCO bandwidth based on the DS setting*/
		DCOC_BW = h->pTnrModeData->BBS_DS_DCOC_Acq_BW;
		if (DCOC_On == BTNR_ON)			 
		{   
			/* DCO Acquisition BW*/
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_BWC, DCOC_BW, unit);

			/* Freeze DCO */
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_Q, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_I, 0, unit);

			/* Reset DCO */	
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_Q, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_I, 0, unit);
			BDBG_MSG(("DCOC is started with BW = 0x%x", DCOC_BW));

			BKNI_Sleep(2);

			/* DCO Tracking BW*/
			DCOC_BW = h->pTnrModeData->BBS_DS_DCOC_Trk_BW;
	
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_BWC, DCOC_BW, unit);
			BDBG_MSG(("DCOC Bandwidth changed, BW = 0x%x", DCOC_BW));
		}	
		else
   	{
			/* DCO Acquisition BW*/
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_BWC, DCOC_BW, unit);
	
	  	/* Freeze DCO */
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_Q, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPFRZ_I, 1, unit);

			/* Release  DCO Reset*/	
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_Q, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_DCOC, AI_DCOC_LPRST_I, 0, unit);
			BDBG_MSG(("DCOC Frozen"));
		}
	}
	BDBG_MSG(("BTNR_7125_P_Set_Tuner_DCOC_Mode() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_Set_BB_LPF_Filter()	This routine it to set the BB filter in the tuner	
 *******************************************************************************************/
void BTNR_7125_P_Tuner_Set_BB_LPF_Filter(BTNR_7125_Handle h)
{
	/*local variables*/
	uint8_t IFLPF_BWR_Sel;
	unsigned unit = h->settings.unitNumber;

	/*lookup the IFLPF_BWR_Sel and FGA_RC_CNTL_Sel values, they are different for each bandwidth 5,6,7,8 MHz and Cable/Terrestrial*/
	switch (h->pTnrModeData->TunerBw)
	{
	case BTNR_7125_Bandwidth_e8MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_8MHz;*/	/*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_8MHz;*/
		IFLPF_BWR_Sel = DS_IFLPF_BWR_SEL_8MHz;
		break;
	case BTNR_7125_Bandwidth_e7MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_7MHz;*/	/*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_7MHz;*/
		IFLPF_BWR_Sel = DS_IFLPF_BWR_SEL_7MHz;
		break;
	case BTNR_7125_Bandwidth_e6MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_6MHz;*/ /*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_6MHz;*/
		IFLPF_BWR_Sel = DS_IFLPF_BWR_SEL_6MHz;
		break;
	case BTNR_7125_Bandwidth_e5MHz : 	/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_5MHz;*/ /*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_5MHz;*/
		IFLPF_BWR_Sel = DS_IFLPF_BWR_SEL_5MHz;
		break;
	default :
		IFLPF_BWR_Sel=0;
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected in BTNR_7125_P_Tuner_Set_BB_LPF_Filter() , Value is %d",h->pTnrModeData->TunerBw));
		break;
	}

	/*write the bandwidth values*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_BWR, IFLPF_BWR_Sel, unit);
	/* RC is now set in RC_Sel called by AlignAGC2() */
	/*the h->pTnrModeData->TunerBw is an enum with 8 MHz = 1, 7 MHz = 2, 6 MHz = 2, and 5_MHz = 4*/
	BDBG_MSG(("%d MHz Channel BW selected LPF Selections is IFLPF_BWR=%d ",(9-(uint8_t)h->pTnrModeData->TunerBw),IFLPF_BWR_Sel));

	BDBG_MSG(("BTNR_7125_P_Tuner_Set_BB_LPF_Filter() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_AlignAGC2()	This routine optimizes signal path gain settings for Cable
 *This function is passed the current value of the LNA Gain
 *******************************************************************************************/
static const int RC_bonded[64][8] =
{
    { 0,3,13,17,29,34,44,48 },
    { 7,11,19,23,34,38,47,51 },
    { 13,17,25,28,38,42,50,53 },
    { 19,22,29,32,41,44,52,55 },
    { 23,26,32,35,44,47,53,56 },
    { 27,30,36,38,46,49,55,58 },
    { 30,33,38,41,48,50,56,59 },
    { 33,35,41,43,49,52,57,60 },
    { 35,37,43,45,51,53,58,60 },
    { 38,40,44,46,52,54,59,61 },
    { 39,41,46,48,53,55,60,62 },
    { 41,43,47,49,54,56,60,62 },
    { 43,44,48,50,55,57,61,63 },
    { 44,46,50,51,56,58,61,63 },
    { 45,47,51,52,57,58,62,122 },
    { 47,48,52,53,57,59,62,122 },
    { 48,49,52,54,58,59,63,122 },
    { 49,50,53,55,59,60,63,123 },
    { 50,51,54,55,59,60,122,123 },
    { 50,52,55,56,60,61,122,123 },
    { 51,52,55,56,60,61,122,123 },
    { 52,53,56,57,60,62,122,124 },
    { 53,54,56,58,61,62,123,124 },
    { 53,54,57,58,61,62,123,124 },
    { 54,55,57,58,62,63,123,124 },
    { 54,55,58,59,62,63,123,124 },
    { 55,56,58,59,62,63,124,125 },
    { 55,56,59,60,62,63,124,125 },
    { 56,57,59,60,63,122,124,125 },
    { 56,57,59,60,63,122,124,125 },
    { 57,58,60,61,63,122,124,125 },
    { 57,58,60,61,63,122,124,125 },
    { 57,58,60,61,122,123,125,125 },
    { 58,59,61,61,122,123,125,126 },
    { 58,59,61,62,122,123,125,126 },
    { 59,59,61,62,122,123,125,126 },
    { 59,60,61,62,122,123,125,126 },
    { 59,60,62,62,123,123,125,126 },
    { 59,60,62,63,123,123,125,126 },
    { 60,60,62,63,123,124,125,126 },
    { 60,61,62,63,123,124,125,126 },
    { 60,61,63,63,123,124,126,126 },
    { 60,61,63,63,123,124,126,126 },
    { 61,61,63,122,123,124,126,126 },
    { 61,62,63,122,124,124,126,126 },
    { 61,62,63,122,124,124,126,127 },
    { 61,62,63,122,124,124,126,127 },
    { 62,62,122,122,124,125,126,127 },
    { 62,62,122,122,124,125,126,127 },
    { 62,62,122,122,124,125,126,127 },
    { 62,63,122,123,124,125,126,127 },
    { 62,63,122,123,124,125,126,127 },
    { 62,63,122,123,124,125,126,127 },
    { 63,63,122,123,125,125,126,127 },
    { 63,63,123,123,125,125,126,127 },
    { 63,63,123,123,125,125,127,127 },
    { 63,122,123,123,125,125,127,127 },
    { 63,122,123,123,125,125,127,127 },
    { 63,122,123,124,125,125,127,127 },
    { 63,122,123,124,125,126,127,127 },
    { 122,122,123,124,125,126,127,127 },
    { 122,122,123,124,125,126,127,127 },
    { 122,122,123,124,125,126,127,127 },
    { 122,122,123,124,125,126,127,127 },
};
static const int RC_single[64][8] =
{
    { 0,0,0,0,3,10,27,34 },
    { 0,0,0,0,11,17,32,38 },
    { 0,0,0,1,17,23,36,42 },
    { 0,0,3,8,22,27,39,44 },
    { 0,0,8,13,26,31,42,47 },
    { 0,3,13,18,30,34,44,49 },
    { 4,8,18,22,33,37,46,50 },
    { 9,13,21,25,36,39,48,52 },
    { 13,16,24,28,38,41,49,53 },
    { 16,20,27,31,40,43,51,54 },
    { 19,22,30,33,42,45,52,55 },
    { 22,25,32,35,43,46,53,56 },
    { 25,27,34,37,45,47,54,57 },
    { 27,30,36,38,46,49,55,58 },
    { 29,32,37,40,47,50,56,58 },
    { 31,33,39,41,48,51,56,59 },
    { 32,35,40,43,49,51,57,59 },
    { 34,36,42,44,50,52,58,60 },
    { 36,38,43,45,51,53,58,60 },
    { 37,39,44,46,52,54,59,61 },
    { 38,40,45,47,52,54,59,61 },
    { 39,41,46,48,53,55,60,62 },
    { 40,42,47,48,54,56,60,62 },
    { 41,43,47,49,54,56,60,62 },
    { 42,44,48,50,55,57,61,63 },
    { 43,45,49,51,55,57,61,63 },
    { 44,46,50,51,56,58,61,63 },
    { 45,47,50,52,56,58,62,63 },
    { 46,47,51,52,57,58,62,122 },
    { 46,48,51,53,57,59,62,122 },
    { 47,49,52,53,58,59,63,122 },
    { 48,49,52,54,58,59,63,122 },
    { 48,50,53,54,58,60,63,122 },
    { 49,50,53,55,59,60,63,123 },
    { 49,51,54,55,59,60,63,123 },
    { 50,51,54,56,59,61,122,123 },
    { 50,52,55,56,60,61,122,123 },
    { 51,52,55,56,60,61,122,123 },
    { 51,53,55,57,60,61,122,123 },
    { 52,53,56,57,60,62,122,124 },
    { 52,53,56,57,61,62,123,124 },
    { 53,54,56,58,61,62,123,124 },
    { 53,54,57,58,61,62,123,124 },
    { 53,55,57,58,61,62,123,124 },
    { 54,55,57,58,61,63,123,124 },
    { 54,55,58,59,62,63,123,124 },
    { 54,56,58,59,62,63,123,124 },
    { 55,56,58,59,62,63,124,125 },
    { 55,56,58,59,62,63,124,125 },
    { 55,56,59,60,62,63,124,125 },
    { 56,57,59,60,63,122,124,125 },
    { 56,57,59,60,63,122,124,125 },
    { 56,57,59,60,63,122,124,125 },
    { 57,57,60,60,63,122,124,125 },
    { 57,58,60,61,63,122,124,125 },
    { 57,58,60,61,63,122,124,125 },
    { 57,58,60,61,63,122,124,125 },
    { 57,58,60,61,122,122,125,125 },
    { 58,59,60,61,122,123,125,125 },
    { 58,59,61,62,122,123,125,126 },
    { 58,59,61,62,122,123,125,126 },
    { 58,59,61,62,122,123,125,126 },
    { 59,59,61,62,122,123,125,126 },
    { 59,60,61,62,122,123,125,126 },
};

static void RC_Sel( BTNR_7125_Handle h )
{
    int RcVal;
    int FGA_Val;
    int LPF_BW;
	unsigned unit = h->settings.unitNumber;

    /* Determine the FGA Gain Setting */
	FGA_Val = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);

    /* Determine the LPF BW Setting */
	LPF_BW  = (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_BWR, unit) & 0x3f);

    /* Look up the RC Control for the FGA gain setting */
    if ( (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_BWR, unit) & 0x40 ) == 0x40 )
    {
        RcVal  = RC_bonded[LPF_BW][FGA_Val];
    }
    else
    {
        RcVal  = RC_single[LPF_BW][FGA_Val];
    }
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_RC_CNTL, RcVal, unit);
	BDBG_MSG(("RC_Sel: Set FgaRcCntl = %d", RcVal ));
}


unsigned long Roof[4][16] =
{
    { 1150000000, 577500000, 386400000, 295300000, 229600000, 195880000, 170500000, 151500000, 
	   128300000, 117900000, 109000000, 101400000,  93900000,  88300000,  83300000,  79000000 },
    { 1240000000, 700600000, 483400000, 373400000, 294400000, 251500000, 219300000, 195400000, 
	   166800000, 153200000, 141500000, 131600000, 121600000, 114400000, 108000000, 102400000 },
    { 1380000000, 803600000, 558000000, 431000000, 342700000, 292000000, 254300000, 226400000, 
	   195500000, 179000000, 165000000, 153300000, 141300000, 132900000, 125400000, 118800000 },
    { 1650000000,1020000000, 710500000, 544800000, 442700000, 373200000, 323400000, 287100000,
	   257100000, 233000000, 213000000, 197000000, 180400000, 169400000, 159400000, 150800000 }
};

/*******************************************************************************************
 * RoofSel() This routine selects the proper roof filter BW depending upon RFTRK1_Gain
 *******************************************************************************************/

static void RoofSel( BTNR_7125_Handle h )
{
    unsigned long Roof_F3db;
    int   TRK_Gain;
    int   j;
	unsigned unit = h->settings.unitNumber;
	unsigned long rfFreq =  h->settings.rfFreq;

    Roof_F3db = ((rfFreq*5)/2);

	TRK_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);
	/* Sanity check */
	if( TRK_Gain > 3 )
	{
		BDBG_ERR(("RoofSel() ERRROR: Array idx is > 3, just return"));
		return;
	}

    for ( j = 0; j < 15; j++ )
    {
        if ( Roof[TRK_Gain][j] <= Roof_F3db )
            break;
    }

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, j, unit);

	BDBG_MSG(("RoofSel: Roof[Rftrk1Gain=%d][set Rftrk1Roof=%d]=%uHz <= (2*%u*1.25)Hz", TRK_Gain, j, Roof[TRK_Gain][j], rfFreq ));
}

/*******************************************************************************************
 * AutoRangeVGA() This routine autoranges the VGA
 *******************************************************************************************/
static void AutoRangeVGA(BTNR_7125_Handle h)
{
    signed int mvarAGCII;
	uint32_t vga_atten;
    uint32_t looplimit = 20;
	unsigned unit = h->settings.unitNumber;
 
    BKNI_Sleep( 5 );
	mvarAGCII = (signed int) BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
	vga_atten = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);
	/* VGA gain range optimization */
    while ( (mvarAGCII < h->pTnrModeData->Min_AGC_Threshold || mvarAGCII > h->pTnrModeData->Max_AGC_Threshold) && looplimit )	    
    {
		/* high input power. Loop until the tuner AGC is above minimum gain */
        if (mvarAGCII < h->pTnrModeData->Min_AGC_Threshold ) 
        {
            /* Increase VGA gain range, i.e add more pads in front of the VGA */
            if (vga_atten<3) 
               vga_atten++;
            else 
                break;
        }
		/* low input signal. Loop until the tuner AGC is below maximum gain */
        else if (mvarAGCII > h->pTnrModeData->Max_AGC_Threshold )    
        {
            /* Decrease VGA gain range, i.e remove more pads in front of the VGA */
            if (vga_atten>0)
                vga_atten--;
            else 
                break;
        }
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, vga_atten, unit);
        BKNI_Sleep( 5 );
		mvarAGCII = (signed int) BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
        looplimit--;
    }

    if( looplimit != 20 )
        BDBG_MSG((">>>AutoRangeVGA(%ldms): Final vga_atten=%ld, AGCII = 0x%08x\n", (20-looplimit)*5, vga_atten, mvarAGCII));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_AlignAGC2() This routine Optimizes the settings of the tuner AGC
 *This function is passed the current value of the LNA Gain
 *******************************************************************************************/

#define F_RFTRK	348000000
void BTNR_7125_P_Tuner_AlignAGC2(BTNR_7125_Handle h, uint8_t LNA_Gain)
{
	uint32_t FGA_Gain = 6;
	uint32_t PGA_Gain = 13;
	signed int mvarAGCII;
	uint32_t vga_atten = 2;
	uint32_t RF_ATT, RFTRK_Gain;
	unsigned unit = h->settings.unitNumber;
	unsigned long rfFreq =  h->settings.rfFreq;
	uint32_t looplimit = 30;

								  
	/*-----------------------------------------------------------------------------------------------------------------
	/ INITIALIZING THE GAINS
	/-----------------------------------------------------------------------------------------------------------------*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
	RC_Sel(h);	/* Use this instead HRW 02022010 */
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_I_PGASET, PGA_Gain, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, PGA_Gain, unit);
	/* make sure the agc bandwidth is wide */
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC2, AGCIFBW, 0x0d, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, vga_atten, unit);

	/*-----------------------------------------------------------------------------------------------------------------
	/ SETTING UP THE FIRST ORDER GAIN VALUES IN THE TUNER FRONT END VIA A LOOK UP TABLE
	/-----------------------------------------------------------------------------------------------------------------*/

	RF_ATT = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_01,RFFE_ATTDB, unit);

	if( LNA_Gain == 31 )
		RFTRK_Gain = 1;
	else
		RFTRK_Gain = 0;
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, RFTRK_Gain, unit);

	/*-----------------------------------------------------------------------------------------------------------------
	/ SETTING UP THE VGA GAIN RANGE
	/-----------------------------------------------------------------------------------------------------------------*/
	BDBG_MSG(("\n->AlignAGC2.."));
	BDBG_MSG(("LNA Gain = 0x%x" , LNA_Gain));
	BDBG_MSG(("ch0 RF_SPLIT_ATT = 0x%x" , BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 0)));
	BDBG_MSG(("RFFE_ATTN = 0x%x" , BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit)));
	BDBG_MSG(("RFTRK1_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit)));
	BDBG_MSG(("RFTRK1_BW = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_BW, unit)));
	BDBG_MSG(("RFTRK1_ROOF = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, unit)));
	BDBG_MSG(("FGA_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit)));
	BDBG_MSG(("VGA_Attenuator = 0x%x",  BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit)));
	BDBG_MSG(("PGA_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, unit)));

	/* HRW 02022010: RELEASE IF AGC */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, 0x28000000, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, AGCIFRZ, 0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC1, AGCIZ, 0, unit);

	BKNI_Sleep(10);

	mvarAGCII = (signed int) BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
	BDBG_MSG(("DS_AGCII = 0x%08x (min=%08x, max=%08x)\n", BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit),
			  h->pTnrModeData->Min_AGC_Threshold, h->pTnrModeData->Max_AGC_Threshold ));
		
    /* VGA gain range optimization */
	while( ((mvarAGCII < h->pTnrModeData->Min_AGC_Threshold) || (mvarAGCII > h->pTnrModeData->Max_AGC_Threshold)) && looplimit  )
	{
		if( mvarAGCII < h->pTnrModeData->Min_AGC_Threshold )		/* high input signal */
		{
			if ( vga_atten < 3 )
				vga_atten++;		/* Increase VGA gain range, i.e add more pads in front of the VGA */
			else if ( FGA_Gain > 4 )
				FGA_Gain = 4;
			else if ( FGA_Gain > 0 )
				FGA_Gain = 0;
			else if ( rfFreq >= F_RFTRK )
			{
			   if ( RF_ATT < 3 )	/* check if RF_ATT can go upto 4 */
				   RF_ATT++;		/* increasing RF attenuator to handle the higher power */
			   else
				   break;
			}
			else /* tuner_freq < F_RFTRK */
			{
			   if ( RFTRK_Gain > 0 )
				  RFTRK_Gain--;		/* reducing RFTRK gain to handle the higher power */
			   else 
				   break;	
			}
		}
		else if( mvarAGCII > h->pTnrModeData->Max_AGC_Threshold )	/* low input signal */
		{
			if ( FGA_Gain < 4 )
				FGA_Gain = 4;
			else if ( FGA_Gain < 6 )
				FGA_Gain = 6;
			else if ( vga_atten > 0 )
			{
				vga_atten--;		/* Decrease VGA gain range, i.e add more pads in front of the VGA */
				if ( (rfFreq < F_RFTRK) && (vga_atten==0) && (RFTRK_Gain==0) )     /* LN: 7019 phy7 mod */
					RFTRK_Gain=1;
			}
			else if ( rfFreq >= F_RFTRK )
			{
			   if ( RF_ATT > 0 )	/* check if RF_ATT can go upto 4 */
				   RF_ATT--;		/* increasing RF attenuator to handle the higher power */
			   else
				   break;
			}
			else /* tuner_freq < F_RFTRK */
			{
			   if ( RFTRK_Gain < 3 )
				  RFTRK_Gain++;		/* reducing RFTRK gain to handle the higher power */
			   else 
				   break;	
			}
		}
		
		/* update gain */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
		RC_Sel(h);	/* Use this instead HRW 02022010 */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, RF_ATT, unit);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, RFTRK_Gain, unit);
		RoofSel(h);	/* Must set the Roof Filter */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, vga_atten, unit);
		BKNI_Sleep( 10 );
		mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
		looplimit--;
		BDBG_MSG(("loop%d: mvarAGCII = 0x%08x\n", (30-looplimit), mvarAGCII ));
		BDBG_MSG(("RFFE_ATTN = 0x%x" , BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit)));
		BDBG_MSG(("RFTRK1_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit)));
		BDBG_MSG(("FGA_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit)));
		BDBG_MSG(("VGA_Attenuator = 0x%x",  BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit)));
	}	/*  end while */
	BDBG_MSG((">>>>After VGAgain range_optm(%dms), AGCII = 0x%08x\n\n", (30-looplimit)*10, mvarAGCII ));

	/*-----------------------------------------------------------------------------------------------------------------
	//SETTING UP THE FGA GAIN 
	/-----------------------------------------------------------------------------------------------------------------*/
    /* High Input */
	mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
	vga_atten = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);
	if ( (vga_atten==3)  && (mvarAGCII < (h->pTnrModeData->Min_AGC_Threshold + 2*((h->pTnrModeData->Max_AGC_Threshold -
																				   h->pTnrModeData->Min_AGC_Threshold)/3))) )
	{
		BDBG_MSG((">>vga_atten==3 & set FgaGain to 0x5"));
		if (FGA_Gain>5) {FGA_Gain=5;}
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
		RC_Sel(h); /* Use this instead HRW 02022010 */
		BKNI_Sleep( 5 );
		mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
		if ( mvarAGCII < (h->pTnrModeData->Min_AGC_Threshold + 2*((h->pTnrModeData->Max_AGC_Threshold -
																   h->pTnrModeData->Min_AGC_Threshold)/3)) )
		{
			BDBG_MSG((">>vga_atten==3 & set FgaGain to 0x4"));
			if (FGA_Gain>4) {FGA_Gain=4;}
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
			RC_Sel(h);

			BKNI_Sleep( 5 );
			mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
			if ( mvarAGCII < (h->pTnrModeData->Min_AGC_Threshold + 2*((h->pTnrModeData->Max_AGC_Threshold -
																	   h->pTnrModeData->Min_AGC_Threshold)/3)) )
			{
				BDBG_MSG((">>vga_atten==3 & set FgaGain to 0x2"));
				if (FGA_Gain>2) {FGA_Gain=2;}
				/* FGA_Gain=2; */
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
				RC_Sel(h);
			}

			BKNI_Sleep( 5 );
			mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
			if ( mvarAGCII < (h->pTnrModeData->Min_AGC_Threshold + 2*((h->pTnrModeData->Max_AGC_Threshold -
																	   h->pTnrModeData->Min_AGC_Threshold)/3)) )
			{
				BDBG_MSG((">>vga_atten==3 & set FgaGain to 0x0"));
				if (FGA_Gain>0) {FGA_Gain=0;}
				/*FGA_Gain=0; */
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
				RC_Sel(h);
			}
		}
    }

    /* debug */
    BKNI_Sleep( 5 );
    mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
    BDBG_MSG((">>>>After FGA High Input adj, AGCII = 0x%08x, FGA_Gain = 0x%x\n", mvarAGCII, BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit)));

    /* Setting up RF track gain for Low Input */
    if ( rfFreq < F_RFTRK )
    {
        if ( (vga_atten==0) && (RFTRK_Gain==1) && (FGA_Gain==6) ) 
        {
            BDBG_MSG((">>vga_atten==0 & set RFTRK_Gain to 2"));
            RFTRK_Gain = 2;
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, RFTRK_Gain, unit);
            AutoRangeVGA(h);
        }

		mvarAGCII = (signed int)BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, unit);
        vga_atten = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);
        if ( (vga_atten==0) && (RFTRK_Gain==2) && (mvarAGCII > (h->pTnrModeData->Min_AGC_Threshold + 
																((h->pTnrModeData->Max_AGC_Threshold-h->pTnrModeData->Min_AGC_Threshold)/2))) && (FGA_Gain==6) ) 
        {
            BDBG_MSG((">>vga_atten==0 & set RFTRK_Gain to 3\n"));
            RFTRK_Gain = 3;
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, RFTRK_Gain, unit);
            AutoRangeVGA(h);
        }
    }
    BDBG_MSG((">>>>After FGA Low Input adj, AGCII = 0x%08x\n<-AlignAGC2\n\n", mvarAGCII ));
    BDBG_MSG(("ch0 RF_SPLIT_ATT = 0x%x" , BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 0)));
    BDBG_MSG(("RFFE_ATTN = 0x%x" , BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit)));
    BDBG_MSG(("RFTRK1_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit)));
    BDBG_MSG(("RFTRK1_BW = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_BW, unit)));
    BDBG_MSG(("RFTRK1_ROOF = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFTRK1_ROOF, unit)));
    BDBG_MSG(("FGA_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit)));
    BDBG_MSG(("VGA_Attenuator = 0x%x",  BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit)));
    BDBG_MSG(("PGA_Gain = 0x%x", BTNR_7125_P_ReadField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, unit)));
}
/****************************************************************************************************
 * BTNR_7125_P_Tuner_LoopVGAGainRange()  
 ****************************************************************************************************/
void BTNR_7125_P_Tuner_LoopVGAGainRange(BTNR_7125_Handle h)
{
	/*local variables*/
	uint8_t RF_ATT_Range, RFTRK_Gain_Range, RFTRK_Gain_Range_old, FGA_Gain, VGA_Gain_Range, Filter_sel, HR_en;
	uint8_t VGA_Flag, LoopCounter, MaxLoopCount, PrintCounter;
	int32_t mvarAGCTI;
 	unsigned unit = h->settings.unitNumber;

	/******************************************************************/
	/*Added a loop to allow us to see values during convergence*/
	PrintCounter = 0;
	/*Loop n times to let mvarAGCTI to settle*/
	/*h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell is defined in the btnr_7125_tune.h and can be changed by BBS*/
	while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell))
	{	
		if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
		{
			mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);
			BDBG_MSG(("PrintCounter = %d DelayTime in uS = %d mvarAGCTI = %x",PrintCounter,500*PrintCounter,mvarAGCTI));
		}
		BKNI_Delay(500);
		PrintCounter++;
	}
	/******************************************************************/
	
	/*If (mvarAGCTI < MIN_AGC_THRESHOLD) or (mvarAGCTI > MAX_AGC_THRESHOLD)*/
	/*the loop will execute until the right gain values are found */
	/*3 on VGA_Gain_Range is the highest gain,	0 on VGA_Gain_Range is the lowest gain*/
	/*7 on FGA_Gain is the highest gain,		0 on FGA_Gain is the lowest gain*/
	/*0 RF_ATT_Range is the highest gain,		4 RF_ATT_RANGE is the lowest gain (used in Cable mode only)*/
	/*3 RFTRK_Gain_Range is the highest gain,	0 RFTRK_Gain_Range is the lowest gain (used in Cable mode only)*/

	/*Initial Conditions for Loop*/
	LoopCounter = 0;
	VGA_Flag = 0;
	VGA_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);	/*read the VGA gain range*/
	FGA_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);			/*read the FGA gain*/
	HR_en =  BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, unit);				/*read HR mode*/	
	Filter_sel = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, unit);	    /*check filter selection*/
	RF_ATT_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit);	/*read the RF Atten value*/
	RFTRK_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);/*read the Trakcing Filter gain value*/
	RFTRK_Gain_Range_old = RFTRK_Gain_Range;	/*Keep track of previous setting*/
	mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);


	/*BEGIN VGA gain range optimization LOOP*/
	MaxLoopCount = 17;
	BDBG_MSG(("While Loop Initial mvarAGCTI = %x",mvarAGCTI));
	while (((mvarAGCTI < MIN_AGC_THRESHOLD) || (mvarAGCTI > MAX_AGC_THRESHOLD))	&& LoopCounter<MaxLoopCount)	
	{   
		/*Reset VGA Flag*/
		VGA_Flag = 0;

		if (mvarAGCTI < MIN_AGC_THRESHOLD)										/*Loop until the tuner AGC is above minimum gain*/
		{
			/*First decrease the VGA gain*/
			if (VGA_Gain_Range > 0)		
			{
				VGA_Gain_Range = VGA_Gain_Range - 1;
				VGA_Flag = 1;
			}
			else	
			{
				if (FGA_Gain > 0)	/*VGA gain is minimum: second decrease the FGA gain*/
				{
					FGA_Gain = FGA_Gain - 1;
				}
				else	/*VGA gain is minimum: FGA gain is minimumn: third If Cable adjust either RFTRK_Gain_Range or RFTRK_Gain_Range*/
				{
					if ((HR_en == 1) && (Filter_sel == 2)) /*harmonic rejection is on*/
					{
						if (RFTRK_Gain_Range > 0)
						{
							RFTRK_Gain_Range = RFTRK_Gain_Range - 1;
						}
						else
						{
							LoopCounter = MaxLoopCount-1;      /*CAB SPEEDUP Added Breakout Condition*/
						}
					}
					else				 /*harmonic rejection is off*/
					{
						if (RF_ATT_Range < 3)
						{
							RF_ATT_Range = RF_ATT_Range + 1;
						} 
						else
						{
							LoopCounter = MaxLoopCount-1;      /*CAB SPEEDUP Added Breakout Condition*/
						}
					}
				}
			}
			LoopCounter = LoopCounter + 1;
		}
		else if (mvarAGCTI > MAX_AGC_THRESHOLD)
		{
			/*First increase the FGA gain (for cable only)*/
			if (FGA_Gain < 7)
			{
				FGA_Gain = FGA_Gain + 1;
			}
			else
			{
				if (VGA_Gain_Range < 3)
				{
					VGA_Gain_Range = VGA_Gain_Range + 1;
					VGA_Flag = 1;
				}
				else
				{
					if ((HR_en == 1) && (Filter_sel == 2)) /*harmonic rejection is on*/
					{
						if (RFTRK_Gain_Range < 3)
						{
							RFTRK_Gain_Range = RFTRK_Gain_Range + 1;
						}
						else
						{
							LoopCounter = MaxLoopCount-1;    /*CAB SPEEDUP Added Breakout Condition*/
						}
					}
					else				 /*harmonic rejection is off*/
					{
						if (RF_ATT_Range > 0)
						{
							RF_ATT_Range = RF_ATT_Range - 1;
						} 
						else
						{
							LoopCounter = MaxLoopCount-1;    /*CAB SPEEDUP Added Breakout Condition*/
						}
					}
				}
			}
			LoopCounter = LoopCounter + 1;
		}

		/*Write New Gain Value*/
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, VGA_Gain_Range, unit);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, FGA_Gain, unit);
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, RF_ATT_Range, unit);
		
		/* Change RFTRK_Gain in certain order 1 <-> 2 <-> 3 <-> 0, no 0 <-> 1 transition!!!*/
		if (RFTRK_Gain_Range_old == 0 && RFTRK_Gain_Range == 1)
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x3, unit);
			BKNI_Delay(100);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x2, unit);
			BKNI_Delay(100);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x1, unit);
		}
		else if (RFTRK_Gain_Range_old == 1 && RFTRK_Gain_Range == 0)
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x2, unit);
			BKNI_Delay(100);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x3, unit);
			BKNI_Delay(100);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 0x0, unit);
		}
		else
		{	
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, RFTRK_Gain_Range, unit);
		}

		/*Added a different delay for VGA changes and other changes, the VGA needs more time to settle*/
		/*Also do not delay/write if we broke out of the loop since all things were already max*/
		/*also allow printing to see convergence rate*/
		/***********************************************************************************************/
		if (LoopCounter != MaxLoopCount)
		{
			if (VGA_Flag == 1)
			{
				PrintCounter = 0;
				/*Loop n times to let mvarAGCTI to settle*/
				/*h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell is defined in the btnr_7125_tune.h and can be changed by BBS*/
				while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell))
				{	
					if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
					{
						mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);
						BDBG_MSG(("VGA_FLAG is %d PrintCounter = %d DelayTime in uS = %d mvarAGCTI = %x",VGA_Flag,PrintCounter,500*PrintCounter, mvarAGCTI));
					}
					BKNI_Delay(500);
					PrintCounter++;
				}
			}
			else
			{
				PrintCounter = 0;
				/*Loop n times to let mvarAGCTI to settle*/
				/*h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell is defined in the btnr_7125_tune.h and can be changed by BBS*/
				while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell))
				{	
					if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
					{
						mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);
						BDBG_MSG(("VGA_FLAG is %d PrintCounter = %d DelayTime in uS = %d mvarAGCTI = %x",VGA_Flag,PrintCounter,500*PrintCounter, mvarAGCTI));
					}
					BKNI_Delay(500);
					PrintCounter++;
				}
			}
		/***********************************************************************************************/

			/*Get new values to update the loop*/
			VGA_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, unit);	/*read the vga attenuator value*/	
			FGA_Gain = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);			/*read the FGA gain value*/
			RF_ATT_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, RFFE_ATTDB, unit);	/*read the RF Atten value*/
			RFTRK_Gain_Range = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, unit);  /*read the Trakcing Filter gain value*/
			RFTRK_Gain_Range_old = RFTRK_Gain_Range;																												/*Keep track of previous setting*/
			mvarAGCTI = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCTI, unit);
	
			BDBG_MSG(("mvarAGCTI = %x",mvarAGCTI));
			BDBG_MSG(("VGA_Gain_Range = %d,FGA_Gain = %d, LoopCounter = %d\n",VGA_Gain_Range,FGA_Gain, LoopCounter));
		}
	}

	BDBG_MSG(("VGA_Gain_Range = %d,FGA_Gain = %d",VGA_Gain_Range,FGA_Gain));
	BDBG_MSG(("RF_ATT_Range = %d, RFTRK_Gain_Range = %d\n",RF_ATT_Range,RFTRK_Gain_Range));

	BDBG_MSG(("BTNR_7125_P_Tuner_LoopVGAGainRange() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_FGA_BW_Selection()	  
 *******************************************************************************************/
void BTNR_7125_P_Tuner_FGA_BW_Selection(BTNR_7125_Handle h)
{
	/*local variables*/
	uint32_t  iflpf_bwr;
	uint8_t	  FGA_Gain;
	uint8_t FGA_RC_Sel;
	unsigned unit = h->settings.unitNumber;

	/*The IFLPF_BWR_BREAKPOINTs are used by the BTNR_7125_P_Get_Status_TNR_Core0() function as well*/
	/*Make sure any changes made here get copied to the other function as well*/
	iflpf_bwr = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_BWR, unit);
	FGA_Gain =  BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, unit);
	if (iflpf_bwr <= IFLPF_BWR_BREAKPOINT1) 
	{	
		FGA_RC_Sel = 0; 
	}
	else if ((iflpf_bwr >= IFLPF_BWR_BREAKPOINT2) && (iflpf_bwr <= IFLPF_BWR_BREAKPOINT3))
	{
		FGA_RC_Sel = iflpf_bwr-3; 
	}
	else
	{
		FGA_RC_Sel = 13;              
	}	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_RC_CNTL, FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL, unit);

	BDBG_MSG(("FGA_RC_CNTL = %d", FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL));
	BDBG_MSG(("BTNR_7125_P_Tuner_FGA_BW_Selection() Complete\n"));
}
