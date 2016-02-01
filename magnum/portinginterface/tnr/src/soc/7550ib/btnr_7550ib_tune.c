
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
#include "btnr_75xxib_priv.h"
#include "btnr_7550ib_tune.h"

/*registers needed for this function */
#include "bchp_ds_tuner_digctl_0.h"
#include "bchp_ds_pll.h"
#include "bchp_ds_qdsafe_0.h" 
#include "bchp_ds_qafe_if_0.h" 
#include "bchp_ds_tuner_anactl.h"
#include "bchp_ds_tuner_ref.h"
#include "bchp_ds_tuner.h"

BDBG_MODULE(btnr_7550ib_tune);

/*******************************************************************************************
 * BTNR_7550_P_Init_TNR_Core0()		This routine initializes the tuner and is only run once
 *******************************************************************************************/
BERR_Code BTNR_7550_P_Init_TNR_Core0(BTNR_75xx_Handle h)
{
	BERR_Code retCode;

	/* local variables */
	uint8_t Ref_PLL_Status;
	SignalSource_t InputSource;

	BTNR_7550_P_Tuner_Device_type(h);
	/*Force AGC Source to always come from the DS core*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_TUNER_TOP_CNTL0, 	AGC_SOURCE_SEL, 0);

	/* Initialize the local part (NOT PI PART!!) of the BTNR_P_75xx_AcquireSettings_t Structure*/
	/* This only happens once, the first time BTNR_7550_P_Init_TNR_Core0() is called*/
	if (h->pTnrModeData->DoneLoadFlag == BTNR_OFF) 
	{
		BTNR_7550_P_Tuner_Init_Buffer(h);
		h->pTnrModeData->DoneLoadFlag = BTNR_ON;				/*set DoneLoadFlag*/	
		h->pTnrModeData->DoneFirstTimeFlag = BTNR_ON;		/*set DoneFirstTimeFlag*/	
		BDBG_MSG(("INITIALIZING THE BTNR_P_75xx_AcquireSettings_t STRUCTURE"));
	}

	/*reset the DoneFirstTimeFlag, this will be set at the end of the BTNR_7550_P_Tune_TNR_Core0()*/	
	h->pTnrModeData->DoneFirstTimeFlag = BTNR_OFF;
	
	/*clear the lock status*/
	h->pTnrStatus->Tuner_Ref_Lock_Status = 0;
	h->pTnrStatus->Tuner_Mixer_Lock_Status = 0;

	/* Range check the local Structure*/
	BTNR_7550_P_Range_Check(h);

	/*define local variables*/
	Ref_PLL_Status = 0;
	InputSource =  (SignalSource_t)h->pTnrModeData->BBS_Input_Source;
 
	/********************************************************************************************/
	/********************************************************************************************/
	/********************************************************************************************/
	/*Reset Registers in DS_PLL*/
	BREG_Write32(h->hRegister, BCHP_DS_PLL_DIV1 , 0x004E0102);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_DIV2 , 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_CH1 , 0x00040004);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_CH2 , 0x0004000C);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_CH3 , 0x000C0012);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_MISC1 , 0x00100023);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_MISC2 , 0x38000700);
	BREG_Write32(h->hRegister, BCHP_DS_PLL_MISC3 , 0x00000015);

	/*DVB-T/ISDB-T DS_PLL Setup*/
	/*determine input source*/
	if ((h->pTnrModeData->TunerAcquisitionMode ==  BTNR_7550_Standard_eDVBT) || (h->pTnrModeData->TunerAcquisitionMode ==  BTNR_7550_Standard_eISDBT))
	{
		/*ISDB-T with an external tuner*/
		if ((h->pTnrModeData->TunerAcquisitionMode ==  BTNR_7550_Standard_eISDBT) && (InputSource == ExternalTuner))
		{
		/* Sample-Rate Converter Setup for ISDB-T (Fs=45MHz) only with an external tuner*/
		/* F = (p2div/p1div)*(ndiv_int+ndiv_frac)/mxdiv*54MHz = (4/2)*(10+0)/12*54MHz = 90MHz */
		BDBG_MSG(("Setting-up DS PLL for 45MHz sampling"));
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, NDIV_INT, 10);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P2DIV, 4);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P1DIV, 2);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV2, NDIV_FRAC, 0);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_CH3,  M3_DIV, 12);

		}
		/*DVB-T with an external tuner or DVB-T/ISDB-T with an internal tuner*/
		else
		{
		/* F = (p2div/p1div)*(ndiv_int+ndiv_frac)/mxdiv*54MHz = (4/2)*(10+0)/10*54MHz = 108MHz */
		BDBG_MSG(("Setting-up DS PLL for 54MHz sampling"));
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, NDIV_INT, 10);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P2DIV, 4);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P1DIV, 2);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV2, NDIV_FRAC, 0);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_CH3,  M3_DIV, 10);
		}

	}
	/*QAM Setup DS_PLL Setup*/
	else 
	{
		/*QAM with an external or internal tuner*/
		/* F = (p2div/p1div)*(ndiv_int+ndiv_frac)/mxdiv*54MHz = (1/2)*(78+0)/18*54MHz = 117MHz */
		BDBG_MSG(("Setting-up DS PLL for 58.5MHz sampling"));
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, NDIV_INT, 78);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P2DIV, 1);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV1, P1DIV, 2);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_DIV2, NDIV_FRAC, 0);
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_CH3,  M3_DIV, 18);
	}

		/*Common to Cable and Terrestrial*/
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_CH2,  PWRDN_CH2, 0);										/* PLL CH2 power up */
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_CH3,  PWRDN_CH3, 0);                    /* PLL CH3 power up */
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_MISC1,  DRESET, 0);                     /* PLL deassert digital reset */
		BTNR_7550_P_WriteField(h->hRegister, DS_PLL_MISC1,  ARESET, 0);                     /* PLL deassert analog reset */


	/********************************************************************************************/	
	/******************* RESET REGISTERS IN TUNER, QDSAFE, and QAFE *****************************/
	/********************************************************************************************/
	/*Reset Registers in DS_TUNER_ANACTL*/
	/*DS_TUNER_ANACTL_WDATA_R01 is read only*/
	/*DS_TUNER_ANACTL_WDATA_R02 is read only*/
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_00, 0x0000E000);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_01, 0xE4450C28);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_02, 0x82002A98);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_03, 0x1014D134);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_04, 0x888300D3);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_05, 0x31661A28);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_06, 0x00C62B12);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_07, 0x90230B01);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_13, 0x00038F78);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_16, 0x0008A66D);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_17, 0x00000682);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_18, 0x0000D903);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_19, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_20, 0x7DF34DE5);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_21, 0x82A00000);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_22, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_23, 0x00007009);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_24, 0x00000000);

	/*Reset Registers in DS_TUNER_REF*/
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_08, 0x18002515);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_09, 0x07DFC061);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_10, 0x00980FFF);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_11, 0x00DD1616);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_12, 0x00001604);

	
	/*Reset Registers in DS_QDSAFE_IF_0*/
	/*Terrestrial Mode*/
	if ((h->pTnrModeData->TunerAcquisitionMode == BTNR_7550_Standard_eDVBT) || (h->pTnrModeData->TunerAcquisitionMode ==  BTNR_7550_Standard_eISDBT))
	{	
		/*DVB-T/ISDB-T Internal Tuner*/
		if (InputSource ==InternalTuner)							
		{
					BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL1, 0x803C3BA4);
					BDBG_MSG(("DVB-T/ISDB-T Internal Tuner Selected"));
		}
		/*DVB-T/ISDB-T External Tuner or IF*/
		else
		{
			/*ISDB-T External Tuner or IF*/
			if (h->pTnrModeData->TunerAcquisitionMode == BTNR_7550_Standard_eISDBT)				
			{
					BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL1, 0x813C3BA0);
					BDBG_MSG(("ISDB-T External Tuner or IF Selected"));
			}
			/*DVB-T External Tuner or IF*/			
			else																																				
			{
					BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL1, 0x813C3BA4);
					BDBG_MSG(("DVB-T External Tuner or IF Selected"));
			}
		}
	}
	/*Cable Mode*/
	else
	{
		/*QAM Internal*/
		if (InputSource ==InternalTuner)								
		{
			BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL1, 0xB03C2000);
			BDBG_MSG(("QAM Internal Tuner"));
		}
		/*QAM External Tuner or IF*/
		else																						
		{
			BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL1, 0xB13C2000);
			BDBG_MSG(("QAM External Tuner or IF Selected"));
		}
	}

	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL2, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL3, 0xBF2A8B06);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL4, 0x97C40092);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL5, 0x00000040);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL6, 0x00000002);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL7, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL8, 0x0000000D);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL9, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL10, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL11, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_CTRL12, 0x00000000);
	BREG_Write32(h->hRegister, BCHP_DS_QDSAFE_0_SPARE, 0x00000000);
	/********************************************************************************************/
	/********************************************************************************************/
	/********************************************************************************************/

	/*power on*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, PWRDN_BG_1P2, 0);  
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, PWDN_BG, 0);		 
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, PD_ADC, 0);		

	/* Initialize DPM*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, DPM_RST, 0x1); 
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, DPM_RST, 0x0); 

	/*Initialize the LNA*/
	BTNR_7550_P_Init_LNA(h);
	BTNR_7550_P_Set_LNA_Boost(h);

	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial)
	{
		/* Turn off cable stuff and turn on terrestrial stuff */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, FILTER_SEL, 0x3); 
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x1); 
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_20, 0x19f34de4);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_21, 0x82980000);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_22, 0x00000000);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_23, 0x00007001);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_24, 0x00000000);

		/* Disable Daisy Output (never used in Terrestrial mode)*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_DB0_WDATA_01, DS_PD, 0x1);

			/* Set Clip Detector thresholds */
#if BCHP_VER == BCHP_VER_A0 /* For A0 only since mixer bias cannot be lowered due to powerdown bug*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_IOS_PRG, 0x8);		/* RF detector threshold*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_IOS_PRG, 0x9);		/* BB detector threshold*/
#else /* For A1 and newer */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, PD_RF_IOS_PRG, 0x5);		/* RF detector threshold*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_21, PD_BB_IOS_PRG, 0x7);		/* BB detector threshold*/
#endif
		/* Change LNA 2nd stage gain */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_GAIN_CNTL, 0x1);	/* LNA 2nd stage set to 12dB*/

		/* Reset (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_DIG_RESET, 0x1);		/* reset AGC digital*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_23, AGC_DIG_RESET, 0x0);		/* release reset*/

		/* Misc Settings (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x00004013);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* Threshold (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x1);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x4f344f34);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* AGC Window Length (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x2);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x00001323);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* Loop BW and Coefficients (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x4);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x00000014);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* Initial Accumulator Value for Main AGC Loop (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x5);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x80000000);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* Initial Accumulator Value for Alt AGC Loop (AGC digital) */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x6);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x80000000);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

		/* Put tuner in lower power mode */	
#if BCHP_VER == BCHP_VER_A0 /* For A0 only since mixer bias cannot be lowered due to powerdown bug*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, MIX_ICNTL, 0x6);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_ICNTL, 0x4);
#else /* For A1 and newer */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, MIX_ICNTL, 0x3);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_ICNTL, 0x2);
#endif
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, IFLPF_BIAS, 0x2);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_PWRSAV, 0xf);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_BIAS, 0x2);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2BUF_DVBT_BIAS, 0x3);

		BDBG_MSG(("Tuner Mode set to Terrestrial"));
	}
	else /*cable*/
	{
		/* Turn on cable stuff and turn off terrestrial stuff */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, FILTER_SEL, 0x1); 
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x0); 
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_20, 0x7df34de5);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_21, 0x82a00000);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_22, 0x00000000);
		BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_23, 0x00007009);
		
		if (h->pTnrModeData->TunerAcquisitionMode != BTNR_7550_Standard_eQAM) /*Reduce power for ISDBT through cable mode*/
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, MIX_ICNTL, 0x2);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_ICNTL, 0x2);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, IFLPF_BIAS, 0x2);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_PWRSAV, 0xf);

			/* Disable Daisy Output (never used in Terrestrial mode)*/
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_DB0_WDATA_01, DS_PD, 0x1);
		}
		BDBG_MSG(("Tuner Mode set to Cable"));
	}

	/* Disable Tuner Test Pads*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUT_PAD_EN, 0x00);
 
	/* Set up reference PLL functions in the tuner */
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_08, 0x18782F15);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_08, 0x18282F15);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_08, 0x18202F15);

	BKNI_Sleep(1);
	
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_08, 0x18002F15);
	BREG_Write32(h->hRegister, BCHP_DS_TUNER_REF_WDATA_09, 0x07D7c0e1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_VC_PRB_EN, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_VZ_PRB_EN, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, PDB_PHOUT, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_LODIVBY4EN, 0x0);	

	/* This sets up the reference PLL */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05,BG_CORE_ADJ,0x7);			
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_09,I_QP_ICTRL,0x03);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_VCOBUF_LATCH_ON,0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DIV_FB,0x12);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_10,I_OUTDIV_M0,0x04);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_11,I_OUTDIV_M1,0x16);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_11,I_OUTDIV_M2,0x16);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_11,I_OUTDIV_M3,0xDD);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_11,I_OUTDIV_M4,0x00);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_12,I_OUTDIV_M5,0x04);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_12,I_OUTDIV_M6,0x16);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_DIV2RST,0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_FBDIVRST,0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_DIV4RST,0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_09,I_OUTDIV_RESET_M,0x3F);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_CLFCNT,0x0);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_VCRST,0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_10,I_CLKEN_M,0x60);	 
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05,IP_LD_EN,0x1);		

	/* release feedback divider resets */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_DIV2RST,0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_FBDIVRST,0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08,I_DAC_VCRST,0x0);

	BKNI_Sleep(1);
	
	/* release divider resets */
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08, I_DAC_DIV4RST, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_09, I_OUTDIV_RESET_M, 0x0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_09, I_QP_ICTRL, 0x7);	

	BKNI_Sleep(1);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08, LD_RESET_STRT, 0x0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, BG_CORE_ADJ, 0x04);	

	BKNI_Sleep(1);

	/*Get the lock status*/
	Ref_PLL_Status = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_TUNER_TOP_STATUS, REF_PLL_LOCK_STATUS);
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
 
 	/*Initialize Tuner Gain Stages*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, RFFE_ATTDB, 0x0);	  	
 	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3); 	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, 0x7);	
 	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT, 0x3);
 	
	/*Initialize QDSAFE and QAFE*/
	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_AGC_MISC, RF_OD_CTRL, 0x01);
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, CLK_RESET, 1);
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, CLK_RESET, 0);
	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_MISC, RST_FIFO, 1);
	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_MISC, RST_FIFO, 0);



	/*Select AGCT on IF delta-sigma, AGCT and AGCI Frozen*/
#ifdef IFAGC_OD_MODE
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGC_MISC, 0x000004cc);
	BDBG_MSG(("Open Drain mode for IFAGC Selected"));
#else
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGC_MISC, 0x0000040c);
	BDBG_MSG(("Push Pull mode for IFAGC Selected"));
#endif
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_ARFLT, MIN_TUNER_VGA_LEVEL);	
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_ARFUT, MAX_TUNER_VGA_LEVEL);
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_DCOC, 0x000000CF);		/* BBDCOC BW=2^(-20), rst, freeze*/	 
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_PGA, 0x02000000);		/*PGA in fixed gain mode*/
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGC1, 0x01c00151);		/*freeze AGCI, break interaction, reset AGCT, fixed IF*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial)
	{
		BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGC3, 0x001107B0);		/*AGCT, Set Acquisition BW = 2^-17, Theshold is 12 dB below sine wave clip level*/	
	}
	else
	{
		BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGC3, 0x00110500);		/*AGCT, Set Acquisition BW = 2^-17, Theshold is 14 dB below sine wave clip level*/	
	}

	/*Set Initial BW for AGC's and DCOC*/
	BTNR_7550_P_Set_Tuner_IFVGA_BW(h, IFVGA_BW_eAcquisition);	
	BTNR_7550_P_Set_Tuner_DCOC_Mode(h, DCOC_Mode_eFreezeReset);

	/*Init AGCT Integrator Value Mid-Range*/
	BREG_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI, 0x30000000);

	/*Initialize PGA Starting Values*/
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_I_PGASET, 0xD);		
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_Q_PGASET, 0xD);		

	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_AGC_MISC, AGCTFRZ, 0);		/*RELEASE TUNER AGC*/

	BKNI_Sleep(1);

	/*Initialization Complete*/
	BDBG_MSG((" BTNR_7550_P_Init_TNR_Core0() Complete\n"));

  retCode = BERR_SUCCESS;
  return retCode;
}

/*******************************************************************************************
 * BTNR_7550_P_Get_LockStatus_TNR_Core0()  This routine gets the status of the tuner
 *******************************************************************************************/
BERR_Code BTNR_7550_P_Get_LockStatus_TNR_Core0(BTNR_75xx_Handle h)
{
	BERR_Code retCode;

	/* local variables */
	int32_t mvarAGCTI, max_vga_level;
	uint8_t VGA_Gain_Range, FGA_Gain, HR_en, Filter_sel, RF_ATT_Range, RFTRK_Gain_Range, LNA_Gain; 

	/* Check Reference PLL Lock */
	h->pTnrStatus->Tuner_Ref_Lock_Status = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_TUNER_TOP_STATUS, REF_PLL_LOCK_STATUS);

	/* Check Mixer Lock */
	h->pTnrStatus->Tuner_Mixer_Lock_Status = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_TUNER_TOP_STATUS, T0_MIXER_PLL_LOCK_STATUS);

	/*Determine if IFVGA is out of Range or if the BTNR_7550_P_Tuner_LoopVGAGainRange() would run*/
	/*The RF path has fixed gains, if the IF VGA goes below MIN_AGC_THRESHOLD the the IF VGA is adding attenuation and clipping could be occuring unless*/
	/*all of the Fixed Gains are already at their MIN levels, then the signal is just strong*/
	/*The RF path has fixed gains, if the IF VGA goes above MAX_AGC_THRESHOLD the the IF VGA is adding gain and a better solution could be found unless*/
	/*all of the Fixed Gains are already at their MAX levels, then the signal is just weak*/

	/*The Cable mode only sets the VGA_Gain_Range, FGA_Gain and either RF_ATT_Range or RFTRK_Gain_Range at acquisition*/ 
	/*The Terrestrial mode only sets the VGA_Gain_Range at acquisition, LNA2 and FGA_Gain are always fixed and RF_ATT_Range and RFTRK_Gain_Range are not used*/ 
	
	/*Get Gain Values*/
	LNA_Gain = BTNR_7550_P_Get_LNA_Gain(h);															/*get LNA gain*/
	mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
	VGA_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT);		/*read the VGA gain range*/
	FGA_Gain = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN);						/*read the FGA gain*/
	HR_en =  BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN);								/*read HR mode*/	
	Filter_sel = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, FILTER_SEL);		    /*check filter selection*/
	RF_ATT_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, RFFE_ATTDB);			/*read the RF Atten value*/
	RFTRK_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN);/*read the Tracking Filter gain value*/

	max_vga_level = MAX_TUNER_VGA_LEVEL;

	/*Terrestrial*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial) /*For terrestrial settings*/
	{
			/*IF AGC is in range*/
			h->pTnrStatus->Tuner_AGC_Status = 1;
	}
	else	/*Cable*/
	{
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
	
			if (mvarAGCTI == max_vga_level)		/*cable disconnect condition*/
			{
				if ((HR_en == 1) && (Filter_sel == 2)) /*harmonic rejection is ON VGA_Gain_Range, FGA_Gain, and RFTRK_Gain_Range used: {RFTRK_Gain_Range is 0 MinGain to 3 MaxGain}*/
				{
					if ((LNA_Gain == 0x1F) && (VGA_Gain_Range == 3) && (FGA_Gain == 7) && (RFTRK_Gain_Range == 3))
					{
						h->pTnrStatus->Tuner_AGC_Status = 0;
					}
				}
				else  /*harmonic rejection is OFF, VGA_Gain_Range, FGA_Gain RF_ATT_Range used: {RF_ATT_Range is 3 MinGain to 0 MaxGain}*/
				{
					if ((LNA_Gain == 0x1F) && (VGA_Gain_Range == 3) && (FGA_Gain == 7) && (RF_ATT_Range == 0 ))
					{
						h->pTnrStatus->Tuner_AGC_Status = 0;
					}
				} 
			}
		}
		else
		{
			/*IF AGC is in range*/
			h->pTnrStatus->Tuner_AGC_Status = 1;
		}
	}

	/*Lock Check Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}

/*******************************************************************************************
 * BTNR_7550_P_Tune_TNR_Core0()		This routine tunes the tuner
 *******************************************************************************************/
BERR_Code BTNR_7550_P_Tune_TNR_Core0(BTNR_75xx_Handle h)
{
	BERR_Code retCode;
	static uint8_t NumberofTune = 0;

	/* Range check the local Structure*/
	BTNR_7550_P_Range_Check(h);

	/* this is needed because the Tuner is called from the DS or THD unlocking so nexus will overwrite and BBS values*/ 
	if (h->pTnrModeData->BBS_Force_TunerAcquisitionType != 0)
	{
		switch(h->pTnrModeData->BBS_Force_TunerAcquisitionType)
		{
		case 1:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eInitTune;
			break;
		case 2:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eTune;
			break;
		case 3:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eMiniTune;
			break;
		default:
			h->pTnrModeData->TunerAcquisitionType = BTNR_7550_TunerAcquisitionType_eInitTune;
			BDBG_ERR(("ERROR!!! Invalid Force_TunerAcquisitionType from BBS selected, Value is %d",h->pTnrModeData->BBS_Force_TunerAcquisitionType));
			break;
		}
	}

	/*BBS or the PI wants to Rerun BTNR_7550_P_Init_TNR_Core0()*/
	/*RUN FOR TuneInit, NOT Tune or MiniTune*/
	if ((h->pTnrModeData->BBS_Rerun_Init == BTNR_ON) || (h->pTnrModeData->TunerAcquisitionType == BTNR_7550_TunerAcquisitionType_eInitTune))
	{
		BTNR_7550_P_Init_TNR_Core0(h);
		BTNR_7550_P_Tuner_MXR_DCO_DAC(h);			
		BTNR_7550_P_Tuner_MXR_DCO_LOOP(h);
		BTNR_7550_P_Tuner_VGA_DCO_LOOP(h);
		if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial)
		{
			BTNR_7550_P_Tuner_AGCT_DCO_LOOP(h);
		}
	}

	/*RUN FOR TuneInit and Tune, NOT MiniTune*/
	if (h->pTnrModeData->TunerAcquisitionType != BTNR_7550_TunerAcquisitionType_eMiniTune)
	{
		BTNR_7550_P_Set_Tuner_Freq(h); 	
		BTNR_7550_P_Set_Tuner_DCOC_Mode(h, DCOC_Mode_eFreezeReset);	/*Freeze and Reset DCOC */
		BTNR_7550_P_Tuner_Set_BB_LPF_Filter(h);											/*Set Initial BW for LPF*/
		
		if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
		{
			BTNR_7550_P_Set_LNA_AGC_BW(h, LNA_AGC_BW_eAcquisition);  /* Set LNA AGC to Acquisition Bandwidth */
		}

		/*Set Tilt, and Output enable in LNA*/	
		/*BTNR_7550_P_Set_LNA_Boost(h) Terrestrial is done in Tune init.  Cable is done in miniTune */
		BTNR_7550_P_Set_LNA_Tilt(h);
		BTNR_7550_P_Set_LNA_Output(h);
	}
	
	/*RUN EVERY TIME: TuneInit, Tune, and MiniTune*//*Perform AGC Alignment*/
	/*********************************************************************************************************************************************/
	BTNR_7550_P_Set_Tuner_IFVGA_BW(h, IFVGA_BW_eAcquisition);	 	/*Set AGC to Acquire Bandwidth */

	/*Terrestrial*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial) /*For terrestrial settings*/
	{
		BTNR_7550_P_Tuner_AlignAGCT(h);	       /*Perform Fixed-Gain AGC Alignment for terrestrial*/
		BDBG_MSG(("AGC Alignment finished for terrestrial"));
	}
	else /*For cable settings*/
	{
		BDBG_MSG(("LNA_GAIN before alignagc2 = %x",BTNR_7550_P_Get_LNA_Gain(h)));

		BTNR_7550_P_Set_LNA_AGC_BW(h, LNA_AGC_BW_eAcquisition);  /* Set LNA AGC to Acquisition Bandwidth */
#if 0
		if(BTNR_7550_P_Get_LNA_AGCLock_Status(h) != 1)
		{
			BTNR_7550_P_Wait_For_LNA_AGCLock(h); 
		}
#endif
		if (DS_LNA_AUTO_BOOST_ON == 0)
		{
			BTNR_7550_P_Set_LNA_Boost(h);
		}
		else if (DS_LNA_AUTO_BOOST_ON == 1 && NumberofTune++ == 20) /*Recheck boost mode every 20th tune if in auto boost mode */
		{
			BDBG_MSG(("Re-running LNA Boost Setup"));
			BTNR_7550_P_Set_LNA_Boost(h);
			NumberofTune = 0;
		}
	
		h->pTnrModeData->LNA_Gain = BTNR_7550_P_Get_LNA_Gain(h); /*Get LNA gain*/

		/*Set Daisy gain for cable only after Initialization*/
		if (h->pTnrModeData->DoneFirstTimeFlag == BTNR_OFF)
		{
			/* Set initial gain stages based on LNA gain setting for Daisy operation*/
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_DB0_WDATA_01, DS_GAIN_CTR, Daisy_Gain_Selection_Table[h->pTnrModeData->LNA_Gain].DS_GAIN_CTR);
		}

		BTNR_7550_P_Tuner_AlignAGC2(h, h->pTnrModeData->LNA_Gain);/*Perform Fixed-Gain AGC Alignment for cable*/
		BDBG_MSG(("AGC Alignment finished for cable"));
	}
	
	BTNR_7550_P_Tuner_LoopVGAGainRange(h);	/*Choose correct attenuator setting, FGA setting, RF attenuator setting(if cable)*/
	
	BDBG_MSG(("LNA_GAIN after LoopVGA = %x",BTNR_7550_P_Get_LNA_Gain(h)));

	/*If LNA gain has moved since before alignagc2 routine, input/LNA are still settling.  Re-run alignagc2 and LoopVGA.  Cable mode only*/
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
	{
		if((h->pTnrModeData->LNA_Gain - BTNR_7550_P_Get_LNA_Gain(h) >= 2) || (BTNR_7550_P_Get_LNA_Gain(h) - h->pTnrModeData->LNA_Gain >= 2))
		{
			BDBG_MSG(("Re-running alignagc2 and LoopVGAGainRange Setup"));
			BTNR_7550_P_Wait_For_LNA_AGCLock(h);  
			h->pTnrModeData->LNA_Gain = BTNR_7550_P_Get_LNA_Gain(h); /*Get LNA gain*/
			BTNR_7550_P_Tuner_AlignAGC2(h, h->pTnrModeData->LNA_Gain); /*Re-running alignagc2*/
			BTNR_7550_P_Tuner_LoopVGAGainRange(h);	/*Re-running LoopVGAGainRange*/
		}
	}
	BTNR_7550_P_Set_LNA_AGC_BW(h, LNA_AGC_BW_eTracking);   /* Set LNA AGC to Tracking Bandwidth */
	BTNR_7550_P_Tuner_FGA_BW_Selection(h);					    /*Set FGA BW*/
	BTNR_7550_P_Set_Tuner_IFVGA_BW(h, IFVGA_BW_eTracking);		/* Set AGC to Tracking Bandwidth */

	/*********************************************************************************************************************************************/

	/*RUN FOR TuneInit and Tune, NOT MiniTune*/
	if (h->pTnrModeData->TunerAcquisitionType != BTNR_7550_TunerAcquisitionType_eMiniTune)
	{
		BTNR_7550_P_Set_Tuner_DCOC_Mode(h, DCOC_Mode_eRun);				/* Run DCOC */
	}

	/*clear the DoneFirstTimeFlag flag*/
	h->pTnrModeData->DoneFirstTimeFlag = BTNR_ON;

	BDBG_MSG(("BTNR_7550_P_Tune_TNR_Core0() Complete: TunerAcquisitionType = %d\n",h->pTnrModeData->TunerAcquisitionType));

	/*Tuning Complete*/
	retCode = BERR_SUCCESS;
	return retCode;
}



/*******************************************************************************************
 * BTNR_7550_P_Set_Tuner_Freq() this rooutine set the tuner frequency and calls 4 subroutines
 * BTNR_7550_P_Set_Tuner_DDFS(), BTNR_7550_P_Set_Tuner_VCO(), BTNR_7550_P_Set_Tuner_VHForUHF()
 * and BTNR_7550_P_Set_Tuner_LPF() it is the only function that goes 3 levels deep
 *******************************************************************************************/
void BTNR_7550_P_Set_Tuner_Freq(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t i,j, part60nm;
	uint8_t RFTRK_Gain_Range;
	uint32_t tempReg1, tempReg2;

	part60nm = h->pTnrStatus->Device_Tech_type;

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x0);

	BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_07, 0x90338301);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_DIV16_12B, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_07, DAC_IF_HP, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_07, DAC_ICTL, 0xF);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_07, DAC_GMX, 0x7);

	/* VGA gets its CM from ADC*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, I_ADC_CM_EN, 0x1);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_REF_WDATA_08, I_MUX_PWRUP, 0x0);	
	
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
		BDBG_ERR(("ERROR Tuner_Freq is not in lookup table in BTNR_7550_P_Set_Tuner_Freq()"));
	}

	/*program divider with value from lookup table*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, DIV_SEL, Tuner_Freq_Selection_Table[part60nm][j].div_sel);

	/*set the DDFS*/
	BTNR_7550_P_Set_Tuner_DDFS(h, Tuner_Freq_Selection_Table[part60nm][j].ssval1);

	/*find the optimum IP_CAP_CNTL*/
	/*select tune routine based on freq range*/
	if (Tuner_Freq_Selection_Table[part60nm][j].tunctl == 1)
    {
		/*Program the VCO*/
		BTNR_7550_P_Set_Tuner_VCO(h, Tuner_Freq_Selection_Table[part60nm][j].tunctl); 
		if (BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CAP_CNTL) == 0x3F)
		{
			/*program divider with value from lookup table*/
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, DIV_SEL, Tuner_Freq_Selection_Table[part60nm][j-1].div_sel);
			/*set the DDFS*/
			BTNR_7550_P_Set_Tuner_DDFS(h, Tuner_Freq_Selection_Table[part60nm][j-1].ssval1);
			/*Program the VCO*/
			BTNR_7550_P_Set_Tuner_VCO(h, 2);
		}
	}
	else 
	{
		/*Program the VCO*/
		BTNR_7550_P_Set_Tuner_VCO(h, Tuner_Freq_Selection_Table[part60nm][j].tunctl); 
		if (BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CAP_CNTL) == 0x3F)
		{
			/*Program the VCO*/
			BTNR_7550_P_Set_Tuner_VCO(h, 1); 
		}
	}

	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, RSTCMOSBB, 0x1);	
	BKNI_Delay(200);  

	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, RSTCMOSBB, 0x0);	
	BKNI_Delay(200);   

	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_06, IP_QPBIASCNT, 0x0D);	
	BKNI_Delay(200);  

	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eTerrestrial) /*For terrestrial settings*/
	{
		BTNR_7550_P_Set_Tuner_VHForUHF(h);
	}
	else  /*For cable settings*/
	{
		/* Need to reset RFTRAK gain to a non 0 before switching between HR and non HR mode*/
		/* Also need to be reset in certain order 1 <-> 2 <-> 3 <-> 0, no 0 <-> 1 transition!!!*/
		RFTRK_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN);  
		if (RFTRK_Gain_Range == 0)
		{
			if (RFTRK_Gain_Range == 0)
			{
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
				BKNI_Delay(100);
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
				BKNI_Delay(100);
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
			}
		    else if (RFTRK_Gain_Range == 2)
			{
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
			}
			else if (RFTRK_Gain_Range == 3)
			{
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
				BKNI_Delay(100);
				BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
			}
		}

		if (h->pTnrModeData->TunerFreq < TUNER_F_RFTRK) 	/*select 3rd harmonic rejection filter*/
		{
			tempReg1 = BREG_Read32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_01);
			tempReg2 = BREG_Read32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_02);
			tempReg1 = tempReg1 & 0xE7FFFF9F;
			tempReg1 = tempReg1 | 0x10000040;
			tempReg2 = tempReg2 | 0x00010000;
			tempReg2 = tempReg2 & 0xFFFFF7FF;
			BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_02, tempReg2);
			BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_01, tempReg1);

			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_ROOF, 0x1);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, FILTER_SEL, 0x2);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x1);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFFE_RSW_EN, 0x0);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x0);*/

			/*select the Low Pass Filter*/
			BTNR_7550_P_Set_Tuner_LPF(h);
		}
		else
		{
			tempReg1 = BREG_Read32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_01);
			tempReg2 = BREG_Read32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_02);
			tempReg1 = tempReg1 & 0xE7FFFF9F;
			tempReg1 = tempReg1 | 0x18000020;
			tempReg2 = tempReg2 & 0xFFFEFFFF;
			tempReg2 = tempReg2 & 0xFFFFF7FF;
			BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_01, tempReg1);
			BREG_Write32(h->hRegister, BCHP_DS_TUNER_ANACTL_WDATA_02, tempReg2);

			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_ROOF, 0x1);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x0);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, FILTER_SEL, 0x1);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFFE_RSW_EN, 0x1);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x0);*/
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_BW, 0x3);*/	
			/*BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_IN, 0x7);*/
		}
	}

	BDBG_MSG(("Tuner_Freq = %d",h->pTnrModeData->TunerFreq));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].ssval1 = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].ssval1));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].div_sel = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].div_sel));
	BDBG_MSG(("Tuner_Freq_Selection_Table[%d][%d].tunctl = %d",part60nm,j,Tuner_Freq_Selection_Table[part60nm][j].tunctl));
 
	/*Tuning Complete*/
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_Freq() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Set_Tuner_DDFS()   This routine programs the DDFS in the ADS area
 *******************************************************************************************/
void BTNR_7550_P_Set_Tuner_DDFS(BTNR_75xx_Handle h, uint8_t ssval1)		
{
	/*local variables*/
	uint32_t ulMultA, ulMultB;
	uint32_t ulNrmHi, ulNrmLo, ulDivisor;

	/*check ssval1 is 4,8,16,32, or 64*/
	if ((ssval1 != 4) && (ssval1 !=8) && (ssval1 != 16) && (ssval1 !=32) && (ssval1 !=64))
	{
		BDBG_ERR(("ERROR ssval1 is not in range in BTNR_7550_P_Set_Tuner_DDFS() value received is %d",ssval1));
	}

	/*Reset DDFS FCW and wait to settle*/
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_DIGCTL_0_DIGCTL_40, DDFS_CORE_SRST, 0x01);
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_DIGCTL_0_DIGCTL_40, DDFS_CORE_SRST, 0x00);
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
		BDBG_ERR(("ERROR 64to32 division is not in a 32 bit range in BTNR_7550_P_Set_Tuner_DDFS()"));
	}

	/*Program the DDFS FCW and wait to settle*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_DIGCTL_0F, FCW, ulNrmLo);
	BKNI_Delay(200);  

	/*Tuner DDFS Complete*/
	BDBG_MSG(("DDFS programmed with 0x%x",ulNrmLo));
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_DDFS() Complete\n"));
}


/*******************************************************************************************
 * BTNR_7550_P_Set_Tuner_VCO()    This routine uses VCO to select optimal caps
 *******************************************************************************************/
void BTNR_7550_P_Set_Tuner_VCO(BTNR_75xx_Handle h, uint8_t tunctl)
{	
	/*local variables*/
	int8_t i;
	uint8_t cap_cntl, POPCAP_Status;
	uint8_t ip_qpbiascnt2, ip_rcntl;

	/*Initial value of IP_CAP_CNTL, this must be 32 or the binary search will go outside of its range*/
	cap_cntl = 32;

	/*check tunectl is 1 or 2*/
	if ((tunctl != 1) && (tunctl !=2))
	{
		BDBG_ERR(("ERROR tunctl is not in range in BTNR_7550_P_Tuner_TuneVCO() value received is %d",tunctl));
	}

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_VCREF, 0x15);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_QPBIASCNT, 0x1B);
	
	/*tunctl is used in this write*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_VCO_SEL, tunctl);	
	
	/*Initial value of IP_CAP_CNTL*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CAP_CNTL, cap_cntl);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CMLDIVRST, 0x1);		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_MANRSTCLF, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, RSTCMOSBB, 0x1);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CMLDIVRST, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_MANRSTCLF, 0x0);	

	BKNI_Delay(200);   
		
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, RSTCMOSBB, 0x0);	

	for(i=5;i>=0;i--)		/*for i = 5 to 0 step -1*/
	{
		/*read the current setting*/
		cap_cntl = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_ANACTL_WDATA_05, IP_CAP_CNTL);	

		/* Adjust Rctl and bias based on cap to set loop BW and gain for VCO*/
		/* Lookup the bias settings in the table at the top of the file*/
		ip_qpbiascnt2 = CAPBIAS_Selection_Table[tunctl-1][cap_cntl].ip_qpbiascnt2;
		ip_rcntl = CAPBIAS_Selection_Table[tunctl-1][cap_cntl].ip_rcntl;
			
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_QPBIASCNT2, ip_qpbiascnt2);	
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_06, IP_RCNTL, ip_rcntl);
		
		/*Read the status*/
		POPCAP_Status = BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_TUNER_TOP_STATUS, T0_MIXER_PLL_POPCAP_STATUS);
		
		/*perform a binary search to find the transistion point of POPCAP_Status going 1 to 0*/
		/*this search only works if the initial value of IP_CAP_CNTRL is 0x20 or 32*/
		if (i > 0)
		{
			cap_cntl = (POPCAP_Status == 0) ? cap_cntl+(1<<(i-1)) : cap_cntl-(1<<(i-1));  /*i=5,4,3,2,1: cap_cntl = cap_cntl +-16, +-8, +-4, +-2, +-1*/
		}
		else
		{
			cap_cntl = (POPCAP_Status == 0) ? cap_cntl : cap_cntl-1;											/*i=0: cap_cntl = cap_cntl +0 or -1*/

		}

		/*write the cap_cntl then allow settling time*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_05, IP_CAP_CNTL, cap_cntl);		
		BDBG_MSG(("VCO Cap setting = 0x%x and VCO Tank used =0x%x",cap_cntl,tunctl));

		BKNI_Delay(200);  

	}

	BDBG_MSG(("Final VCO Cap setting = 0x%x and VCO Tank used =0x%x",cap_cntl,tunctl));
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_VCO() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Set_Tuner_VHForUHF() This routine selects VHF/UHF path from VHF_UHF_BREAKPOINT
 *******************************************************************************************/
void BTNR_7550_P_Set_Tuner_VHForUHF(BTNR_75xx_Handle h)
{

	if (h->pTnrModeData->TunerFreq < VHF_UHF_BREAKPOINT) /*signal path set to VHF band*/
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN, 0x1); /*disable UHF RFPGA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_UHF_PWRDN, 0x1); /*disable UHF LNA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2BUF_DVBT_UHF_PWRDN, 0x1); /*disable UHF LNA buffer */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN, 0x0); /*enable VHF RFPGA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_VHF_PWRDN, 0x0); /*enable VHF LNA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2BUF_DVBT_VHF_PWRDN, 0x0); /*enable VHF LNA buffer */
		BDBG_MSG(("VHF Path Choosen, Tuner_Freq= %d, Breakpoint= %d",h->pTnrModeData->TunerFreq, VHF_UHF_BREAKPOINT));
	}
	else /*signal path set to UHF band*/
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_VHF_OUTPUT_PWRDN, 0x1); /*disable VHF RFPGA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_VHF_PWRDN, 0x1); /*disable VHF LNA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2BUF_DVBT_VHF_PWRDN, 0x1); /*disable VHF LNA buffer */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, RFPGA_DVBT_UHF_OUTPUT_PWRDN, 0x0); /*enable UHF RFPGA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2_DVBT_UHF_PWRDN, 0x0); /*enable UHF LNA */
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_20, LNA2BUF_DVBT_UHF_PWRDN, 0x0); /*enable UHF LNA buffer */
		BDBG_MSG(("UHF Path Choosen, Tuner_Freq= %d, Breakpoint= %d",h->pTnrModeData->TunerFreq, VHF_UHF_BREAKPOINT));
	}

	BDBG_MSG(("BTNR_7550_P_Set_Tuner_VHForUHF() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Set_Tuner_LPF()    This routine selects the correct LPF corner frequency
 *******************************************************************************************/
void BTNR_7550_P_Set_Tuner_LPF(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t i,j;
	uint8_t rftrk1_in;

	/*initialize the tracking filter*/
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, PGALPF_PWRDN, 0x0);	
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_02, HR_EN, 0x1);	

	/*optimization of the tracking filter linearity*/
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_ROOF, 0x1);		
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_VDD_CNTL, 0x2);	

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
		BDBG_ERR(("ERROR Tuner_Freq is not in lookup table in BTNR_7550_P_Set_Tuner_LPF()"));
	}
	rftrk1_in = LPF_Selection_Table[j].rftrk1_in;

	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_BW, j);		
	BTNR_7550_P_WriteField(h->hRegister,DS_TUNER_ANACTL_WDATA_01, RFTRK1_IN, rftrk1_in);	
	
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_LPF() Complete\n"));
}

/*******************************************************************************************************************
 * BTNR_7550_P_Set_Tuner_IFVGA_BW()  This routine sets the RF/IF bandwidth in the QAFE
 ******************************************************************************************************************/
void BTNR_7550_P_Set_Tuner_IFVGA_BW(BTNR_75xx_Handle h, IFVGA_BW_t IFVGA_BW)
{
	/*local variables*/
	uint32_t RF_AGC_BW, IF_AGC_BW;

	/*check IFVGA_BW is in range*/
	if ((IFVGA_BW != IFVGA_BW_eAcquisition) && (IFVGA_BW != IFVGA_BW_eTracking))
	{
		BDBG_ERR(("ERROR IFVGA_BW is not in range in BTNR_7550_P_Set_Tuner_IFVGA_BW() value received is %d",IFVGA_BW));
	}

	if (IFVGA_BW == IFVGA_BW_eAcquisition)
	{
		/*Set the AGC Acquisition bandwidths based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T mode from the BTNR_P_75xx_AcquireSettings_t structure*/
		RF_AGC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_RF_AGC_Acq_BW : h->pTnrModeData->BBS_THD_RF_AGC_Acq_BW;
		IF_AGC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_IF_AGC_Acq_BW : h->pTnrModeData->BBS_THD_IF_AGC_Acq_BW;
		BDBG_MSG(("AGC Acquisition BW's set: RF_BW =  0x%x  IF_BW =  0x%x",RF_AGC_BW, IF_AGC_BW));
	}
	else
	{
		/*Set the AGC Tracking bandwidths based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T mode from the BTNR_P_75xx_AcquireSettings_t structure*/
		RF_AGC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_RF_AGC_Trk_BW : h->pTnrModeData->BBS_THD_RF_AGC_Trk_BW;
		IF_AGC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_IF_AGC_Trk_BW : h->pTnrModeData->BBS_THD_IF_AGC_Trk_BW;
		BDBG_MSG(("AGC Tracking BW's set: RF_BW =  0x%x  IF_BW =  0x%x",RF_AGC_BW, IF_AGC_BW));
	}

	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_AGC3, AGCTNBW, RF_AGC_BW);	
	BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_AGC2, AGCIFBW, IF_AGC_BW);	
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_IFVGA_BW() Complete\n"));
}

/*******************************************************************************************************************
 * BTNR_7550_P_Set_Tuner_DCOC_Mode()  This routine sets the DCOC in the QAFE
 ******************************************************************************************************************/
void BTNR_7550_P_Set_Tuner_DCOC_Mode(BTNR_75xx_Handle h, DCOC_Mode_t DCOC_Mode)
{
	/*local variables*/
	BTNR_OFFON_t DCOC_On;
	uint32_t DCOC_BW;

	/*check IFVGA_BW is in range*/
	if ((DCOC_Mode != DCOC_Mode_eFreezeReset) && (DCOC_Mode != DCOC_Mode_eRun))
	{
		BDBG_ERR(("ERROR DCOC_Mode is not in range in BTNR_7550_P_Set_Tuner_DCOC_Mode() value received is %d",DCOC_Mode));
	}

	if (DCOC_Mode == DCOC_Mode_eFreezeReset)
	{
		/*Set the DCO Acquisition BW based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T*/
		DCOC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_DCOC_Acq_BW : h->pTnrModeData->BBS_THD_DCOC_Acq_BW;
		BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_BWC, DCOC_BW);

		/* Freeze DCO */
		BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_Q, 1);	
		BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_I, 1);

		/* Reset DCO */	
		BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_Q, 1);
		BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_I, 1);

	BDBG_MSG(("DCOC is Reset and Frozen with initial BW = 0x%x", DCOC_BW));
	}
	else
	{
		/*Enable the DCO based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T*/
		DCOC_On = (BTNR_OFFON_t)(h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_DCOC_On : h->pTnrModeData->BBS_THD_DCOC_On;
	
		/*Set the DCO bandwidth based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T*/
		DCOC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_DCOC_Acq_BW : h->pTnrModeData->BBS_THD_DCOC_Acq_BW;
		if (DCOC_On == BTNR_ON)			 
		{   
			/* DCO Acquisition BW*/
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_BWC, DCOC_BW);

			/* Freeze DCO */
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_Q, 0);	
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_I, 0);

			/* Reset DCO */	
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_Q, 0);
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_I, 0);
			BDBG_MSG(("DCOC is started with BW = 0x%x", DCOC_BW));

			BKNI_Sleep(2);

			/* DCO Tracking BW*/
			DCOC_BW = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? h->pTnrModeData->BBS_DS_DCOC_Trk_BW : h->pTnrModeData->BBS_THD_DCOC_Trk_BW;
	
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_BWC, DCOC_BW);
			BDBG_MSG(("DCOC Bandwidth changed, BW = 0x%x", DCOC_BW));
		}	
		else
   	{
			/* DCO Acquisition BW*/
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_BWC, DCOC_BW);
	
	  	/* Freeze DCO */
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_Q, 1);	
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPFRZ_I, 1);

			/* Release  DCO Reset*/	
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_Q, 0);
			BTNR_7550_P_WriteField(h->hRegister, DS_QAFE_IF_0_DCOC, AI_DCOC_LPRST_I, 0);
			BDBG_MSG(("DCOC Frozen"));
		}
	}
	BDBG_MSG(("BTNR_7550_P_Set_Tuner_DCOC_Mode() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_Set_BB_LPF_Filter()	This routine it to set the BB filter in the tuner	
 *******************************************************************************************/
void BTNR_7550_P_Tuner_Set_BB_LPF_Filter(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t IFLPF_BWR_Sel, FGA_RC_CNTL_Sel;

	/*lookup the IFLPF_BWR_Sel and FGA_RC_CNTL_Sel values, they are different for each bandwidth 5,6,7,8 MHz and Cable/Terrestrial*/
	switch (h->pTnrModeData->TunerBw)
	{
	case BTNR_7550_Bandwidth_e8MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_8MHz;*/	/*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_8MHz;*/
		IFLPF_BWR_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_IFLPF_BWR_SEL_8MHz : THD_IFLPF_BWR_SEL_8MHz;
		FGA_RC_CNTL_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_FGA_RC_CNTL_SEL_8MHz : THD_FGA_RC_CNTL_SEL_8MHz;
		break;
	case BTNR_7550_Bandwidth_e7MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_7MHz;*/	/*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_7MHz;*/
		IFLPF_BWR_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_IFLPF_BWR_SEL_7MHz : THD_IFLPF_BWR_SEL_7MHz;
		FGA_RC_CNTL_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_FGA_RC_CNTL_SEL_7MHz : THD_FGA_RC_CNTL_SEL_7MHz;
		break;
	case BTNR_7550_Bandwidth_e6MHz : 		/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_6MHz;*/ /*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_6MHz;*/
		IFLPF_BWR_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_IFLPF_BWR_SEL_6MHz : THD_IFLPF_BWR_SEL_6MHz;
		FGA_RC_CNTL_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_FGA_RC_CNTL_SEL_6MHz : THD_FGA_RC_CNTL_SEL_6MHz;
		break;
	case BTNR_7550_Bandwidth_e5MHz : 	/*IFLPF_BWR_Sel = IFLPF_BWR_SEL_5MHz;*/ /*FGA_RC_CNTL_Sel = FGA_RC_CNTL_SEL_5MHz;*/
		IFLPF_BWR_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_IFLPF_BWR_SEL_5MHz : THD_IFLPF_BWR_SEL_5MHz;
		FGA_RC_CNTL_Sel = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? DS_FGA_RC_CNTL_SEL_5MHz : THD_FGA_RC_CNTL_SEL_5MHz;
		break;
	default :
		IFLPF_BWR_Sel=0;
		FGA_RC_CNTL_Sel=0;
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected in BTNR_7550_P_Tuner_Set_BB_LPF_Filter() , Value is %d",h->pTnrModeData->TunerBw));
		break;
	}

		/*write the bandwidth values*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, IFLPF_BWR, IFLPF_BWR_Sel);	   
    BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_RC_CNTL, FGA_RC_CNTL_Sel);	

		/*the h->pTnrModeData->TunerBw is an enum with 8 MHz = 1, 7 MHz = 2, 6 MHz = 2, and 5_MHz = 4*/
		BDBG_MSG(("%d MHz Channel BW selected LPF Selections are IFLPF_BWR=%d   FGA_RC_CNTL=%d",(9-(uint8_t)h->pTnrModeData->TunerBw),IFLPF_BWR_Sel,FGA_RC_CNTL_Sel));

	BDBG_MSG(("BTNR_7550_P_Tuner_Set_BB_LPF_Filter() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_AlignAGCT()	 This routine optimizes signal path gain settings for Terrestrial
 *******************************************************************************************/
void BTNR_7550_P_Tuner_AlignAGCT(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t VGA_Gain_Range, div_sel;

	VGA_Gain_Range = 0;
	div_sel = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, DIV_SEL);

	/* INITIALIZING THE GAINS */
	if (div_sel == 1) /* Mixer in non HR mode */
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, 0x3); 		
	}
	else /* Mixer in HR mode.  Give 1.5dB to FGA */
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, 0x4);
	}

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT, VGA_Gain_Range); 	
		
	/*************************************	
	* Adjust terrestrial AGC parameters and reset
	**************************************/
	
	/*Set initial accumulator value to mid point*/
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x5);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x80000000);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WADDR, 0x6);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_24, AGC_WDATA, 0x80000000);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x0);
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_22, AGC_WLOAD, 0x1);

	BDBG_MSG(("BTNR_7550_P_Tuner_AlignAGCT() Complete\n"));
}



/*******************************************************************************************
 * BTNR_7550_P_Tuner_AlignAGC2()	This routine optimizes signal path gain settings for Cable
 *This function is passed the current value of the LNA Gain
 *******************************************************************************************/
void BTNR_7550_P_Tuner_AlignAGC2(BTNR_75xx_Handle h, uint8_t LNA_Gain)
{
	/*local variables*/
	uint8_t VGA_Gain_Range, div_sel, ADC_PGA;
	uint8_t  RFTRK_Gain_Range, RFTRK_Gain_Range_old;
	
	/*check LNA_Gain is in range or 0 to 0x1F*/
	if (LNA_Gain > 0x1F)
	{
		BDBG_ERR(("ERROR LNA_Gain is not in range in BTNR_7550_P_Tuner_AlignAGC2() value received is %d",LNA_Gain));
	}

	VGA_Gain_Range = 1;	
	ADC_PGA = 0xD;
	div_sel = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, DIV_SEL);	

	/* INITIALIZING THE GAINS */	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01,  RFFE_ATTDB,  AlignAgc2_Gain_Selection_Table[LNA_Gain].RF_ATT_Range);

	RFTRK_Gain_Range_old = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN);	/* Get current gain */
	RFTRK_Gain_Range = AlignAgc2_Gain_Selection_Table[LNA_Gain].RFTRK_Gain_Range;			/* Get new gain */
	
	/* Change RFTRK_Gain in certain order 1 <-> 2 <-> 3 <-> 0, no 0 <-> 1 transition!!!*/
	if (RFTRK_Gain_Range_old == 0 && RFTRK_Gain_Range == 1)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
	}
	else if (RFTRK_Gain_Range_old == 0 && RFTRK_Gain_Range == 2)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);	
	}
	else if (RFTRK_Gain_Range_old == 1 && RFTRK_Gain_Range == 0)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x0);
	}
	else if (RFTRK_Gain_Range_old == 1 && RFTRK_Gain_Range == 3)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
	}
	else if (RFTRK_Gain_Range_old == 2 && RFTRK_Gain_Range == 0)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x0);
	}
	else if (RFTRK_Gain_Range_old == 3 && RFTRK_Gain_Range == 1)
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
		BKNI_Delay(100);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
	}
	else
	{	
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, RFTRK_Gain_Range);
	}

	if (div_sel == 1) /* Mixer in non HR mode */
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, 0x5); 		
	}
	else /* Mixer in HR mode.  Give 1.5dB to FGA */
	{
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, 0x6);
	}	
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT, VGA_Gain_Range); 
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_I_PGASET, ADC_PGA);		
	BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_Q_PGASET, ADC_PGA);	

	BDBG_MSG(("BTNR_7550_P_Tuner_AlignAGC2() Complete\n"));
}

/****************************************************************************************************
 * BTNR_7550_P_Tuner_LoopVGAGainRange()  
 ****************************************************************************************************/
void BTNR_7550_P_Tuner_LoopVGAGainRange(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint8_t RF_ATT_Range, RFTRK_Gain_Range, RFTRK_Gain_Range_old, FGA_Gain, VGA_Gain_Range, Filter_sel, HR_en, ADC_PGA;
	uint8_t VGA_Flag, LoopCounter, MaxLoopCount, PrintCounter;
	int32_t mvarAGCTI;

	/******************************************************************/
	/*Added a loop to allow us to see values during convergence*/
	PrintCounter = 0;
	/*Loop n times to let mvarAGCTI to settle*/
	/*h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell is defined in the btnr_7550_tune.h and can be changed by BBS*/
	while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell))
	{	
		if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
		{
			mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
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
	VGA_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT);		/*read the VGA gain range*/
	FGA_Gain = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN);						/*read the FGA gain*/
	HR_en =  BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_02, HR_EN);								/*read HR mode*/	
	Filter_sel = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, FILTER_SEL);		    /*check filter selection*/
	RF_ATT_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, RFFE_ATTDB);			/*read the RF Atten value*/
	RFTRK_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN);/*read the Trakcing Filter gain value*/
	RFTRK_Gain_Range_old = RFTRK_Gain_Range;	/*Keep track of previous setting*/
	ADC_PGA = BTNR_7550_P_ReadField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_I_PGASET);

	/*BEGIN VGA gain range optimization LOOP*/
	mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
	MaxLoopCount = (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) ? 17 : 4;
	BDBG_MSG(("While Loop Initial mvarAGCTI = %x",mvarAGCTI));
	while (((mvarAGCTI < MIN_AGC_THRESHOLD) || (mvarAGCTI > MAX_AGC_THRESHOLD))	&& LoopCounter<MaxLoopCount)
	{   
		/*Reset VGA Flag*/
		VGA_Flag = 0;

		if (mvarAGCTI < MIN_AGC_THRESHOLD) /*Loop until the tuner AGC is above minimum threshold*/
		{
			/*First decrease the VGA gain.  For cable, decrease FGA if VGA is 1 or 0*/
			if (VGA_Gain_Range > 0)	
			{
				if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable && VGA_Gain_Range == 1 && FGA_Gain > 0) 
				{
					FGA_Gain = FGA_Gain - 1;
				}
				else		
				{
					VGA_Gain_Range = VGA_Gain_Range - 1;
					VGA_Flag = 1;
				}
			}
			else	
			{
				if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) /*ONLY RUN FOR CABLE*/
				{
					if (FGA_Gain > 0)	/*VGA gain is minimum: second decrease the FGA gain*/
					{
						FGA_Gain = FGA_Gain - 1;
					}
					else	/*VGA gain is minimum: FGA gain is minimumn: If Cable adjust either RFTRK_Gain_Range or RFTRK_Gain_Range*/
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

			}
			LoopCounter = LoopCounter + 1;
		}
		else if (mvarAGCTI > MAX_AGC_THRESHOLD) /*Loop until the tuner AGC is below maximum threshold*/
		{
			/*First increase the FGA gain for cable if VGA is not 0.  Increase VGA for terrestrial*/
			if (VGA_Gain_Range < 3)
			{
				if ((h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) && VGA_Gain_Range != 0 && FGA_Gain < 7)
				{
					FGA_Gain = FGA_Gain + 1;
				}
				else
				{
					VGA_Gain_Range = VGA_Gain_Range + 1;
					VGA_Flag = 1;
				}
			}
			else
			{
				if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable) /*ONLY RUN FOR CABLE*/
				{
					if (FGA_Gain > 7)	/*VGA gain is minimum: second increase the FGA gain*/
					{
						FGA_Gain = FGA_Gain + 1;
					}
					else	/*VGA gain is maximum: FGA gain is maximum: If Cable adjust either RFTRK_Gain_Range or RFTRK_Gain_Range*/
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
			}
			LoopCounter = LoopCounter + 1;
		}

		/*Write New Gain Value*/
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT, VGA_Gain_Range);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN, FGA_Gain);
		BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, RFFE_ATTDB, RF_ATT_Range);
		
		/* Change RFTRK_Gain in certain order 1 <-> 2 <-> 3 <-> 0, no 0 <-> 1 transition!!!*/
		if (RFTRK_Gain_Range_old == 0 && RFTRK_Gain_Range == 1)
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
			BKNI_Delay(100);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
			BKNI_Delay(100);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x1);
		}
		else if (RFTRK_Gain_Range_old == 1 && RFTRK_Gain_Range == 0)
		{
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x2);
			BKNI_Delay(100);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x3);
			BKNI_Delay(100);
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, 0x0);
		}
		else
		{	
			BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN, RFTRK_Gain_Range);
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
				/*h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell is defined in the btnr_7550_tune.h and can be changed by BBS*/
				while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell))
				{	
					if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
					{
						mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
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
				/*h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell is defined in the btnr_7550_tune.h and can be changed by BBS*/
				while ((PrintCounter < h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell))
				{	
					if (h->pTnrModeData->BBS_LoopVGAGain_Display==1)
					{
						mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
						BDBG_MSG(("VGA_FLAG is %d PrintCounter = %d DelayTime in uS = %d mvarAGCTI = %x",VGA_Flag,PrintCounter,500*PrintCounter, mvarAGCTI));
					}
					BKNI_Delay(500);
					PrintCounter++;
				}
			}
		/***********************************************************************************************/

			/*Get new values to update the loop*/
			VGA_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, OUTVGA_ATT);	    /*read the vga attenuator value*/	
			FGA_Gain = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_GAIN);							/*read the FGA gain value*/
			RF_ATT_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_01, RFFE_ATTDB);				/*read the RF Atten value*/
			RFTRK_Gain_Range = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_04, RFTRK1_GAIN);  /*read the Trakcing Filter gain value*/
			RFTRK_Gain_Range_old = RFTRK_Gain_Range;																												/*Keep track of previous setting*/
			mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
	
			BDBG_MSG(("mvarAGCTI = %x",mvarAGCTI));
			BDBG_MSG(("VGA_Gain_Range = %d,FGA_Gain = %d, LoopCounter = %d\n",VGA_Gain_Range,FGA_Gain, LoopCounter));
			if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
			{
				BDBG_MSG(("RF_ATT_Range = %d, RFTRK_Gain_Range = %d\n",RF_ATT_Range,RFTRK_Gain_Range));
			}
		}
	}

	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
	{
		if(mvarAGCTI > AVG_MAXMIN_THRESHOLD + (MAX_AGC_THRESHOLD - MIN_AGC_THRESHOLD)/15)
		{
			while (ADC_PGA < 0xF && (mvarAGCTI > AVG_MAXMIN_THRESHOLD))
			{
				ADC_PGA = ADC_PGA + 1;
				BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_I_PGASET, ADC_PGA);
				BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_Q_PGASET, ADC_PGA);
				BKNI_Sleep (2);
				mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
			}
		}
		else if(mvarAGCTI < AVG_MAXMIN_THRESHOLD - (MAX_AGC_THRESHOLD - MIN_AGC_THRESHOLD)/15)			
		{
			while (ADC_PGA > 0xB && (mvarAGCTI < AVG_MAXMIN_THRESHOLD))
			{
				ADC_PGA = ADC_PGA - 1;
				BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_I_PGASET, ADC_PGA);
				BTNR_7550_P_WriteField(h->hRegister, DS_QDSAFE_0_CTRL1, INT_Q_PGASET, ADC_PGA);
				BKNI_Sleep (2);
				mvarAGCTI = BREG_Read32(h->hRegister, BCHP_DS_QAFE_IF_0_AGCTI);
			}
		}
	}

	BDBG_MSG(("mvarAGCTI = %x",mvarAGCTI));
	BDBG_MSG(("VGA_Gain_Range = %d,FGA_Gain = %d",VGA_Gain_Range,FGA_Gain));
	BDBG_MSG(("ADC_PGA = %x",ADC_PGA));
	if (h->pTnrModeData->TunerSetup == BTNR_7550_Setup_eCable)
	{
		BDBG_MSG(("RF_ATT_Range = %d, RFTRK_Gain_Range = %d\n",RF_ATT_Range,RFTRK_Gain_Range));
	}
	BDBG_MSG(("BTNR_7550_P_Tuner_LoopVGAGainRange() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7550_P_Tuner_FGA_BW_Selection()	  
 *******************************************************************************************/
void BTNR_7550_P_Tuner_FGA_BW_Selection(BTNR_75xx_Handle h)
{
	/*local variables*/
	uint32_t  iflpf_bwr;
	uint8_t	  FGA_Gain;
	uint8_t FGA_RC_Sel;

	/*The IFLPF_BWR_BREAKPOINTs are used by the BTNR_7550_P_Get_Status_TNR_Core0() function as well*/
	/*Make sure any changes made here get copied to the other function as well*/
	iflpf_bwr = BTNR_7550_P_ReadField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, IFLPF_BWR);
	FGA_Gain =  BTNR_7550_P_ReadField(h->hRegister,DS_TUNER_ANACTL_WDATA_03, FGA_GAIN);
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
	BTNR_7550_P_WriteField(h->hRegister, DS_TUNER_ANACTL_WDATA_03, FGA_RC_CNTL, FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL);	

	BDBG_MSG(("FGA_RC_CNTL = %d", FGA_RC_Selection_Table[FGA_RC_Sel][FGA_Gain].FGA_RC_CNTL));
	BDBG_MSG(("BTNR_7550_P_Tuner_FGA_BW_Selection() Complete\n"));
}
