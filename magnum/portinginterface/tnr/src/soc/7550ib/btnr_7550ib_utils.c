
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

#include "btnr_75xxib_priv.h"
#include "btnr_7550ib_tune.h"
#include "bchp_sun_top_ctrl.h"

/*registers needed for this function */
#include "bchp_ds_tuner_anactl.h"
#include "bchp_ds_tuner_digctl_0.h"



BDBG_MODULE(btnr_7550ib_utils);

/***************************************************************************
 * PowerDn_TNR_Core0()
 ***************************************************************************/
BERR_Code BTNR_7550_P_PowerDn_TNR_Core0(BTNR_75xx_Handle h)
{
  BERR_Code retCode;
  BSTD_UNUSED(h); /*what am I*/
  
	/*put tuner power down code here*/
	BDBG_WRN(("WARNING!!!!!   BTNR_7550_P_PowerDn_TNR_Core0() UNSUPPORTED \n"));
		
	
	BDBG_MSG(("BTNR_7550_P_PowerDn_TNR_Core0() Complete\n"));
	
	retCode = BERR_SUCCESS;
  return retCode;
}

/***************************************************************************
 * PowerUp_TNR_Core0()
 ***************************************************************************/
BERR_Code BTNR_7550_P_PowerUp_TNR_Core0(BTNR_75xx_Handle h)
{
  BERR_Code retCode;
  BSTD_UNUSED(h); /*what am I*/

  /*put tuner power up code here*/
	BDBG_WRN(("WARNING!!!!!   BTNR_7550_P_PowerUp_TNR_Core0() UNSUPPORTED \n"));


	BDBG_MSG(("BTNR_7550_P_PowerUp_TNR_Core0() Complete\n"));
	retCode = BERR_SUCCESS;
  return retCode;
}

/*******************************************************************************************
 * BTNR_7550_P_Range_Check()	 This routine checks that parameters from the PI are in range
 *******************************************************************************************/
void BTNR_7550_P_Range_Check(BTNR_75xx_Handle h)
{

	/* Range check the inputs */

	/*Check/Print Tuner Tuner Acquisition Type*/
	switch (h->pTnrModeData->TunerAcquisitionType)
	{
	case BTNR_7550_TunerAcquisitionType_eInitTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is Tune/Init"));
		break;
	case BTNR_7550_TunerAcquisitionType_eTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is Tune"));
		break;
	case BTNR_7550_TunerAcquisitionType_eMiniTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is MiniTune"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid TunerAcquisitionType selected, Value is %d",h->pTnrModeData->TunerAcquisitionType));
		break;
	}

	/*Check/Print Tuner Bandwidth*/
	switch (h->pTnrModeData->TunerBw)
	{
	case BTNR_7550_Bandwidth_e8MHz : 	
		BDBG_MSG(("Range_Check 8 MHz Channel BW selected"));
		break;
	case BTNR_7550_Bandwidth_e7MHz : 	
		BDBG_MSG(("Range_Check 7 MHz Channel BW selected"));
		break;
	case BTNR_7550_Bandwidth_e6MHz : 	
		BDBG_MSG(("Range_Check 6 MHz Channel BW selected"));
		break;
	case BTNR_7550_Bandwidth_e5MHz : 	
		BDBG_MSG(("Range_Check 5 MHz Channel BW selected"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected, Value is %d",h->pTnrModeData->TunerBw));
		break;
	}

	/*Check/Print Tuner Acquisition Mode*/
	switch (h->pTnrModeData->TunerAcquisitionMode)
	{
	case BTNR_7550_Standard_eDVBT : 	
		BDBG_MSG(("Range_Check DVB-T Tuner_Mode selected"));
		break;
	case BTNR_7550_Standard_eISDBT : 	
		BDBG_MSG(("Range_Check ISDB-T Tuner_Mode selected"));
		break;
	case BTNR_7550_Standard_eQAM : 	
		BDBG_MSG(("Range_Check QAM Tuner_Mode selected"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid Tuner_Mode selected Value is %d",h->pTnrModeData->TunerAcquisitionMode));
		break;
	}

	/*Check/Print Tuner Setup Mode*/
	switch (h->pTnrModeData->TunerSetup)
	{
	case BTNR_7550_Setup_eCable : 	
		BDBG_MSG(("Range_Check Cable TunerSetup selected"));
		break;
	case BTNR_7550_Setup_eTerrestrial : 	
		BDBG_MSG(("Range_Check Terrestrial TunerSetup selected"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid Tuner Setup selected Value is %d",h->pTnrModeData->TunerSetup));
		break;
	}

	/*Check/Print Tuner Frequency*/
	if ((h->pTnrModeData->TunerFreq < MIN_TUNER_FREQ) || (h->pTnrModeData->TunerFreq > MAX_TUNER_FREQ)) 
	{
		BDBG_ERR(("ERROR UNSUPPORTED Tuner_Freq PASSED TO BTNR_7550_P_Init_TNR_Core0()"));
	}

	BDBG_MSG(("BTNR_7550_P_Range_Check() Complete\n"));
}

/*************************************************************************************************************
* BTNR_7550_P_Tuner_Init_Buffer()	 This routine loads the BTNR_P_75xx_AcquireSettings_t with default values*
* The only values not initialized are the ones passed from the PI											 *
**************************************************************************************************************/
void BTNR_7550_P_Tuner_Init_Buffer(BTNR_75xx_Handle h)
{
	/*Initialize BTNR_P_75xx_AcquireSettings_t Structure*/
	/*PI parameters are not initialized and are never written to*/
	/*these parameters are used locally and for BBS to sent parameters into the function*/
	h->pTnrModeData->BBS_AcqWord0 =									(uint32_t)ACQWORD0;								/*Beginning of 37 words of memory for BBS*/ 
	h->pTnrModeData->BBS_AcqWord1 =									(uint32_t)ACQWORD1;
	h->pTnrModeData->BBS_AcqWord2 =									(uint32_t)ACQWORD2;
	h->pTnrModeData->BBS_AcqWord3 =									(uint32_t)ACQWORD3;
	h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell =		(uint32_t)LOOPVGAGAIN_INIT_DWELL;				 
	h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell =		(uint32_t)LOOPVGAGAIN_VGA_DWELL;	 
	h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell =	(uint32_t)LOOPVGAGAIN_NOTVGA_DWELL;
	h->pTnrModeData->BBS_LoopVGAGain_Display =			(uint32_t)LOOPVGAGAIN_DISPLAY;
	h->pTnrModeData->BBS_Input_Source =							(uint32_t)INPUT_SOURCE;
	h->pTnrModeData->BBS_Rerun_Init =								(uint32_t)RERUN_INIT;
	h->pTnrModeData->BBS_THD_RF_AGC_Acq_BW =				(uint32_t)THD_RF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_THD_IF_AGC_Acq_BW =				(uint32_t)THD_IF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_THD_RF_AGC_Trk_BW =				(uint32_t)THD_RF_AGC_TRK_BW;
	h->pTnrModeData->BBS_THD_IF_AGC_Trk_BW =				(uint32_t)THD_IF_AGC_TRK_BW;
	h->pTnrModeData->BBS_THD_DCOC_On =							(uint32_t)THD_DCOC_ON;
	h->pTnrModeData->BBS_THD_DCOC_Acq_BW =					(uint32_t)THD_DCOC_ACQ_BW;
	h->pTnrModeData->BBS_THD_DCOC_Trk_BW =					(uint32_t)THD_DCOC_TRK_BW;
	h->pTnrModeData->BBS_THD_BCM3410_Auto =					(uint32_t)THD_BCM3410_AUTO;
	h->pTnrModeData->BBS_THD_BCM3410_Gain_State =		(uint32_t)THD_BCM3410_GAIN_STATE;
	h->pTnrModeData->BBS_THD_LNA_Boost_On =					(uint32_t)THD_LNA_BOOST_ON;
	h->pTnrModeData->BBS_THD_LNA_Tilt_On =					(uint32_t)THD_LNA_TILT_ON;
	h->pTnrModeData->BBS_THD_LNA_Tilt_Freq =				(uint32_t)THD_LNA_TILT_FREQ;
	h->pTnrModeData->BBS_DS_RF_AGC_Acq_BW =					(uint32_t)DS_RF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_DS_IF_AGC_Acq_BW =					(uint32_t)DS_IF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_DS_RF_AGC_Trk_BW =					(uint32_t)DS_RF_AGC_TRK_BW;
	h->pTnrModeData->BBS_DS_IF_AGC_Trk_BW =					(uint32_t)DS_IF_AGC_TRK_BW;
	h->pTnrModeData->BBS_DS_DCOC_On =								(uint32_t)DS_DCOC_ON;
	h->pTnrModeData->BBS_DS_DCOC_Acq_BW =						(uint32_t)DS_DCOC_ACQ_BW;
	h->pTnrModeData->BBS_DS_DCOC_Trk_BW =						(uint32_t)DS_DCOC_TRK_BW;
	h->pTnrModeData->BBS_DS_BCM3410_Auto =					(uint32_t)DS_BCM3410_AUTO;
	h->pTnrModeData->BBS_DS_BCM3410_Gain_State =		(uint32_t)DS_BCM3410_GAIN_STATE;
	h->pTnrModeData->BBS_DS_LNA_Boost_On =					(uint32_t)DS_LNA_BOOST_ON;
	h->pTnrModeData->BBS_DS_LNA_Tilt_On =						(uint32_t)DS_LNA_TILT_ON;
	h->pTnrModeData->BBS_DS_LNA_Tilt_Freq =					(uint32_t)DS_LNA_TILT_FREQ;	
	h->pTnrModeData->BBS_Force_TunerAcquisitionType =(uint32_t)FORCE_TUNERACQUISITIONTYPE;/*End of 37 words of memory for BBS*/ 
	h->pTnrModeData->DoneLoadFlag =									BTNR_ON;
	h->pTnrModeData->DoneFirstTimeFlag =						BTNR_OFF;
	h->pTnrModeData->ReRunInit =										BTNR_OFF;
	h->pTnrModeData->Ref_PLL_Status =								0;
	h->pTnrModeData->Mixer_PLL_Status =							0;
	h->pTnrModeData->LNA_Gain =											0;
	h->pTnrModeData->LNA_ChipID =										BTNR_LNAID_3410;

	BDBG_MSG(("BTNR_7550_P_Tuner_Init_Buffer() Complete\n"));
}

/***************************************************************************
* BTNR_7550_P_2560log10() This function calculated 256*log10(x)
***************************************************************************/
uint32_t BTNR_7550_P_2560log10(uint32_t x)
{
  int32_t  x1;
  int32_t  x2;
  int32_t  x3;
  int32_t  x4;

  if (x == 0)
    return 0;

  x1 = 30;
  while (!((x >> x1) & 1))
    x1 = x1-1;
  x1 = x1+1;

  x2 = (int32_t)((x << 8) >> x1);
  x3 = -24381739 + x2*(62348 + (x2 << 7));
  x4 = 5907991 + (x2 << 16);

  return (unsigned)((770*(x3/(x4 >> 8) + (x1 << 8))) >> 8);
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_MXR_DCO_DAC()	This routine adjusts the mixer DCO DAC	
 *******************************************************************************************/
void BTNR_7550_P_Tuner_MXR_DCO_DAC(BTNR_75xx_Handle h)
{
	
	/*local variables*/
	uint8_t HR_Mode, VHF_pwrdn, UHF_pwrdn;
	uint8_t i;
	uint8_t DCO_Data_i, dco_comp_val_i, DCO_Data_q, dco_comp_val_q;

	/* Hit the mixer DCO resets*/
 	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_GMICNTL, 0x0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, I_DCO_MIX_RESET, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_Q_QP_RST, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_I_RST, 0x1);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXR_DCO_LSB_1P2, 0x4);

	/* make sure it is not powered down*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXR_DCODAC_PWRDN_1P2, 0);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, DCO_PRE_IQ_PWRDN, 0);	
	

	HR_Mode=0;
	VHF_pwrdn=0;
	UHF_pwrdn=0;
	/* disable input*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial) /*For terrestrial*/
	{
		/*disable input and store the state of input path*/
		VHF_pwrdn = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN);
		UHF_pwrdn = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN, 0x1);	/*Kill VHF input*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN, 0x1);	/*Kill UHF input*/
	}
	else	/*For cable */               	
	{
		/*disable input and store the state of HR path*/
		HR_Mode = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN);	
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x1);		/*Force HR path*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x1);		
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, FILTER_SEL, 2);		
	}
	
	BKNI_Sleep(1);
	
	/*init values for I*/	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_I_1P2, 0x00);	
	DCO_Data_i = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_I_1P2);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_COMP_EN_1P2, 0x1);	
	
	BKNI_Sleep(1);
	
	dco_comp_val_i = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_ANACTL_WDATA_00, DCO_PRE_I_COMP_OUT);	

	/*init values for Q*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_Q_1P2, 0x00);	
	DCO_Data_q = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_Q_1P2);	
	
	BKNI_Sleep(1);
	
	dco_comp_val_q = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_00, DCO_PRE_Q_COMP_OUT);	
	

	/*Loop until ((dco_comp_val_i == 1) && (dco_comp_val_q == 1)) or loop is executed 31 times*/
	i = 0;
	while (i < 31)
	{
		dco_comp_val_i = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_00, DCO_PRE_I_COMP_OUT);	
		dco_comp_val_q = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_00, DCO_PRE_Q_COMP_OUT);	
		
		if(dco_comp_val_i != 1) 
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_I_1P2, DCO_Data_i);	
		}

		if(dco_comp_val_q != 1) 
		{		
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_Q_1P2, DCO_Data_q);	
		}

		BKNI_Sleep(2);

		DCO_Data_i = (BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_I_1P2) + 1);	/*DCO_Data_i = DS_TUNER_ANACTL.WDATA_13.MXRDCO_DATA_I_1P2 + 1*/
		DCO_Data_q = (BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, MXRDCO_DATA_Q_1P2) + 1);	/*DCO_Data_q = DS_TUNER_ANACTL.WDATA_13.MXRDCO_DATA_Q_1P2 + 1*/

		i++;
		
		if (((dco_comp_val_i == 1) && (dco_comp_val_q == 1)) || (i == 31))
		{
			BDBG_MSG(("dco_comp_val_i=%d, dco_comp_val_q=%d, after %d passes out of 31 through the loop",dco_comp_val_i,dco_comp_val_q, i));
			i = 31; /*terminate loop*/
		}
	}
	
	BKNI_Sleep(1);
	
	/*re-enable input*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial) /*For terrestrial*/
	{
		if (VHF_pwrdn == 0)
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20,  RFPGA_DVBT_VHF_OUTPUT_PWRDN, 0x0);
		}
		if (UHF_pwrdn == 0)
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20,  RFPGA_DVBT_UHF_OUTPUT_PWRDN, 0x0);
		}
	}
	else	/*For cable*/
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x0);	
		if (HR_Mode == 0)
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x0);
		}
	}
	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, DCO_PRE_IQ_PWRDN, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, I_DCO_MIX_RESET, 0x1);	

	BDBG_MSG(("DCO_Data_i=0x%x,DCO_Data_q=0x%x",DCO_Data_i,DCO_Data_q));
	BDBG_MSG(("BTNR_7550_P_Tuner_MXR_DCO_DAC() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_MXR_DCO_LOOP()	This routine turns on the mixer DCO loop	
 *******************************************************************************************/
void BTNR_7550_P_Tuner_MXR_DCO_LOOP(BTNR_75xx_Handle h)
{
	/*power down VGA DCO*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_IDCO_PWRDN, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_QDCO_PWRDN, 0x1);

	/*freeze VGA DCO*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_17, VGA_DCO_FRZ, 0x1);

	/*power up mxr_DCO_loop*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, DCO_PRE_IQ_PWRDN, 0);

	/*Reset QP*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_I_RST, 0x1);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_Q_QP_RST, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_I_RST, 0x0);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_13, DCO_MIX_Q_QP_RST, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, DCO_PRE_QP_I, 0x7);	    /* set QP current setting = 320nA*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, I_DCO_MIX_RESET, 0);		
	
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_24, 0x0);	/* Clear LSB integrator I[31:0]  */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_25, 0x0);	/* Clear MSB integrator I[54:23] */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_26, 0x0);	/* Clear LSB integrator Q[31:0]  */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_27, 0x0);	/* Clear MSB integrator Q[54:23] */

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_OUT_FRZ, 0x1);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_IN_EDGE_SEL, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_OUT_EDGE_SEL, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_LIN_COEF, 0x10);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_INT_COEF, 0xB);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_OUT_FRZ, 0x0);		
		
	BKNI_Sleep(5);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, DCO_PRE_QP_I, 0x5);		/* set QP current setting = 200nA */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_LIN_COEF, 0xE);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_20, MXR_DCO_INT_COEF, 0x8);

	BKNI_Sleep(2);

	BDBG_MSG(("BTNR_7550_P_Tuner_MXR_DCO_LOOP() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_VGA_DCO_LOOP()	This routine turns on the VGA DCO loop	
 *******************************************************************************************/
void BTNR_7550_P_Tuner_VGA_DCO_LOOP(BTNR_75xx_Handle h)
{
	/* power up VGA DCO */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_IDCO_PWRDN, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_QDCO_PWRDN, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_IDCO_LBS, 0x4);	/* LSB range = 4 */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, VGA_QDCO_LBS, 0x4);	/* LSB range = 4 */
	
	/* set VGA DCO */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_17, 0x00006093); /* BW=2^-17, out={int[31], int[24:21]}, reset, freeze */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_17, 0x00006090); /* Unreset, unfreeze */

	BKNI_Sleep(2);

	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_17, 0x000060b0); /* reduce BW=2^-19 */
	
	BKNI_Sleep(10);

	BREG_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_DIGCTL_17, 0x000060b1); /* freeze it */

	BDBG_MSG(("BTNR_7550_P_Tuner_VGA_DCO_LOOP() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_AGCT_DCO_LOOP()	 This routine turns on the Terrestrial AGC DCO loop
 *******************************************************************************************/
void BTNR_7550_P_Tuner_AGCT_DCO_LOOP(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t VHF_pwrdn, UHF_pwrdn, RF_IOS, BB_IOS, RF_DCO_setting, BB_DCO_setting;
	
	/*disable input and store the state of input path*/
	VHF_pwrdn = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN);
	UHF_pwrdn = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN, 0x1);				/*Kill VHF input*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN, 0x1);				/*Kill UHF input*/

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_CAL_EN, 0x1);											/*Enable RF DCO mode*/
	RF_IOS = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_IOS_PRG);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_IOS_PRG, 0x0);											/*Reset RF offset current*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_RF_DCO_START, 0x0);									/*Start RF DCO*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_RF_DCO_START, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_RF_DCO_START, 0x0);

	BKNI_Sleep(1);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_IOS_PRG, RF_IOS);									/*Restore RF offset current*/

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_CAL_EN, 0x1);											/*Enable BB DCO mode*/
	BB_IOS = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_IOS_PRG);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_IOS_PRG, 0x0);											/*Reset BB offset current*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_BB_DCO_START, 0x0);									/*Start BB DCO*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_BB_DCO_START, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_BB_DCO_START, 0x0);

	BKNI_Sleep(1);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_IOS_PRG, BB_IOS);									/*Restore BB offset current*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN, VHF_pwrdn);	/*restore VHF input*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN, UHF_pwrdn);	/*restore UHF input*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RADDR, 8);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RLOAD, 0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RLOAD, 1);
	RF_DCO_setting = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_R02, AGC_RDATA);
	BDBG_MSG(("RF AGC DCO setting = %d",RF_DCO_setting));

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RADDR, 9);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RLOAD, 0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_RLOAD, 1);
	BB_DCO_setting = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_R02, AGC_RDATA);
	BDBG_MSG(("BB AGC DCO setting = %d",BB_DCO_setting));

	BDBG_MSG(("BTNR_7550_P_Tuner_AGCT_DCO_LOOP() Complete\n"));
}


/*******************************************************************************************
 * BTNR_7550_P_Tuner_Device_type()	 This routine read the otp to see if it is 60 or 65nm
 *******************************************************************************************/
void BTNR_7550_P_Tuner_Device_type(BTNR_75xx_Handle h)
{
	
	uint32_t temp, bit_mask, bit_value;
	
	/* Set the jtag_otp_cpu_mode bit high to provide a free running clock to the OTP module */
	BTNR_7550_P_WriteField(h->hRegister, SUN_TOP_CTRL_GENERAL_CTRL_1, jtag_otp_cpu_mode, 1);
	temp = BREG_Read32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_STATUS_1);
    temp = BREG_Read32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_1); 
	BREG_Write32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_1, (temp | (0x65 << 16)));  /* Read OTP CPU STATUS */
	BREG_Write32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0, 0); /* set otp_prog_en high and Make sure Start = 0 */
    temp = BREG_Read32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0);
    BREG_Write32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0, (temp | 0x00000001));
    temp = BREG_Read32(h->hRegister, BCHP_SUN_TOP_CTRL_GENERAL_STATUS_0);
    bit_mask = 0x1 << 5;
    bit_value = (temp & bit_mask) >> 5;
    BDBG_MSG(("BTNR_7550_P_Tuner_Device_type() : %d \n", bit_value));
    h->pTnrStatus->Device_Tech_type = bit_value;
}