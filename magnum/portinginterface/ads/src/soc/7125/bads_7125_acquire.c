/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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
#include "bchp_ds_tuner_0.h"
#include "bchp_ds_tuner_1.h"
#include "bchp_ds_tuner_chnz_0.h"
#include "bchp_ds_tuner_chnz_1.h"
#include "bchp_ds_tuner_core_gen_0_0.h"
#include "bchp_ds_tuner_core_gen_0_1.h"
#include "bchp_ds_qafe_if_0_0.h" 
#include "bchp_ds_qdsafe_0_0.h"

BDBG_MODULE(bads_7125_acquire);


/***********************************************************************************************************************/
/*the Baud Rates are broken up into 16 regions, 0 is (1000000 MBaud <= Fb < 1130000 MBaud),  15 is (Fb > 6254270 MBaud)*/
/*The table was created using a baud spacing of 13%, nominal values are within +-6.5%                                  */ 
/*This is so all of the loops can be lookuptables																	   */
/***********************************************************************************************************************/
static uint32_t BaudRates_TBL[NUM_BAUD_RATES] =
{
1000000,	/*1000000 < Fb <= 1130000*/
1130000,	/*1130000 < Fb <= 1276900*/
1276900,	/*1276900 < Fb <= 1442897*/ 
1442897,	/*1442897 < Fb <= 1630474*/ 
1630474,	/*1630474 < Fb <= 1842435*/ 
1842435,	/*1842435 < Fb <= 2081952*/
2081952,	/*2081952 < Fb <= 2352605*/ 
2352605,	/*2352605 < Fb <= 2658444*/
2658444,	/*2658444 < Fb <= 3004042*/ 
3004042,	/*3004042 < Fb <= 3394567*/
3394567,	/*3394567 < Fb <= 3835861*/
3835861,	/*3835861 < Fb <= 4334523*/ 
4334523,	/*4334523 < Fb <= 4898011*/ 
4898011,	/*4898011 < Fb <= 5534753*/
5534753,	/*5534753 < Fb <= 6254270*/ 
6254270		/*6254270 < Fb <= 7300000*/
};

static uint32_t AnnexA_Filter_TBL[18] = 
{
 0x000001B1, 0x000000F2, 0x0000002A, 0x00000FB4, 0x00000FA9, 0x00000FDF,
 0x00000016, 0x00000026, 0x00000015, 0x00000FFC, 0x00000FF1, 0x00000FF6,
 0x00000FFF, 0x00000004, 0x00000004, 0x00000001, 0x00000FFF, 0x00000200
};

static uint32_t Q64AnnexB_Filter_TBL[18] = 
{
 0x000001CD, 0x00000148, 0x000000A2, 0x00000013, 0x00000FBF, 0x00000FAD,
 0x00000FC9, 0x00000FF4, 0x00000012, 0x0000001B, 0x00000013, 0x00000005,
 0x00000FFC, 0x00000FF9, 0x00000FFB, 0x00000FFE, 0x00000001, 0x00000200
};

static uint32_t Q256AnnexB_Filter_TBL[18] = 
{
 0x000001C9, 0x0000013C, 0x0000008F, 0x00000000, 0x00000FB5, 0x00000FAF,
 0x00000FD4, 0x00000000, 0x00000019, 0x0000001B, 0x0000000E, 0x00000000,
 0x00000FF9, 0x00000FF9, 0x00000FFD, 0x00000000, 0x00000002, 0x00000200
};

/*******************************************************************************************
 * BADS_7125_P_Init_ADS()		This routine initializes the ADS and is only run once
 *******************************************************************************************/
BERR_Code BADS_7125_P_Init(BADS_ChannelHandle hChannel)
{
	BERR_Code retCode;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);

	/* Initialize the local part (NOT PI PART!!) of the BADS_7125_P_AcquireParams_t Structure*/
	/* This only happens once, the first time BADS_7125_P_Init_ADS() is called*/
	if (h->pAcquireParam->DoneLoadFlag == BADS_OffOn_eOff) 
	{
		BADS_7125_P_ADS_Init_Buffer(h);
		h->pAcquireParam->DoneLoadFlag = BADS_OffOn_eOn;			/*set DoneLoadFlag*/	
 		BDBG_MSG(("INITIALIZING THE BTNR_P_7125_AcquireSettings_t STRUCTURE"));
 
	}

	/*reset the DoneFirstTimeFlag, this will be set by the BADS_7125_P_Acquire()*/	
	h->pAcquireParam->DoneFirstTimeFlag = BADS_OffOn_eOff;

	/*Initialization Complete*/
 	BDBG_MSG((" BADS_7125_P_Init_ADS_Core0() Complete\n"));
 
  retCode = BERR_SUCCESS;
  return retCode;
}

/*******************************************************************************************
 * BADS_7125_P_Get_LockStatus_ADS_Core0()		This routine gets the lock status of the ADS
 *******************************************************************************************/
BERR_Code BADS_7125_P_Get_LockStatus(BADS_ChannelHandle hChannel)
{
	BERR_Code retCode;
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;

	/*local variables*/
	BADS_QAMStatus_t QamStatus;
	BADS_FECStatus_t FECStatus;
	BADS_FECStatus_t CurrentFECStatus;
	uint8_t ReSyncFlag;
	uint32_t HighThresh, LowThresh, SNR;
	uint32_t BMPG1, CBERC1, UERC1, NBERC1; 

	uint32_t ReadReg;

	/***************************/
	/*Determine QAM Lock Status*/
	/***************************/
	HighThresh = BADS_7125_P_ReadField(hImplDev->hRegister, DS_EQ_0_SNRHT, SNRHTHRESH, chn);		 
	LowThresh = BADS_7125_P_ReadField(hImplDev->hRegister, DS_EQ_0_SNRLT, SNRLTHRESH, chn);
	SNR = BADS_7125_P_ReadField(hImplDev->hRegister, DS_EQ_0_SNR, ERRVAL, chn);

   if ((SNR < HighThresh) && (SNR > LowThresh)) 
   {
		QamStatus = BADS_QAMStatus_eLocked;
   }
   else
   {
		QamStatus = BADS_QAMStatus_eNotLocked;
   }
	
	 /*assign values to the status structure to be read by BBS*/
	h->pStatus->QamStatus = (uint32_t)QamStatus;
	
	/***************************************/
	/*Determine SNR                        */
	/*BBS will divide by 256 to get dB     */
	/***************************************/
	/*SNR in dB is different for each QAM Mode*/
  /*16   QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 32113-5120*log(ERRVAL)*/
  /*32   QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 31342-5120*log(ERRVAL)*/
	/*64   QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 32167-5120*log(ERRVAL)*/
	/*128  QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 31370-5120*log(ERRVAL)*/    
	/*256  QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 32180-5120*log(ERRVAL)*/
	/*512  QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 31376-5120*log(ERRVAL)*/    
	/*1024 QAM  125.4405 dB - 20*log(ERRVAL) or if scaled by 256: 32183-5120*log(ERRVAL)*/
			
	/*SNR*/
	ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
	ReadReg = (ReadReg & 0x000000F0)>>4;

	switch (ReadReg)
	{
		case 3: h->pStatus->SNR = 32113 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*16 QAM Mode*/
		case 4: h->pStatus->SNR = 31342 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*32 QAM Mode*/
		case 5: h->pStatus->SNR = 32167 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*64 QAM Mode*/
		case 6: h->pStatus->SNR = 31370 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*128 QAM Mode*/
		case 7:	h->pStatus->SNR = 32180 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*256 QAM Mode*/
		case 8:	h->pStatus->SNR = 31376 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*512 QAM Mode*/
		case 9:	h->pStatus->SNR = 32183 - (BADS_7125_P_2560log10(SNR)<<1); break;		/*1024 QAM Mode*/
		default: BDBG_WRN(("WARNING!!! UNSUPPORTED OR UNDEFINED QAM_MODE (%u) chn %u",ReadReg,chn)); break;
	}

	/*SNR AVERAGE*/
	/*BBS will divide by 256 to get dB*/
	h->pStatus->SNRAVG = ((h->pStatus->SNR*(h->pAcquireParam->BBS_AcqWord0)) + (16384-(h->pAcquireParam->BBS_AcqWord0))*h->pStatus->SNRAVG)/16384;


	/***************************/
	/*Determine FEC Lock Status*/
	/***************************/

	/*clear the MPEG bad block resync flag */
	ReSyncFlag = 0;

	/*Get the currect lock condition*/
	CurrentFECStatus = (BADS_FECStatus_t)h->pStatus->FecStatus;

	/*Get the latest block error counter values*/	
	BMPG1  = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_BMPG1, BMPGCNTVAL, chn);
	CBERC1 = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_CBERC1, CBERCCNTVAL, chn);
	UERC1  = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_UERC1, UERCCNTVAL, chn);
	NBERC1 = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_NBERC1, NBERCCNTVAL, chn);

	/*Check for at least one good block since the last call which is 25 mS*/
	if ((NBERC1 - h->pAcquireParam->Old_NBERC1) >= NUM_CLEAN_BLOCKS_TO_LOCK)
	{
		/*If Annex B then check for MPEG Checksum False Sync*/
		/*This is and implementation from Jorge's BCM3250/7015/7100 App note*/
		/*Document 3250/7015/7100-An01-R*/
		if ((BMPG1 > 10) && (h->pAcquireParam->Annex == BADS_Annex_eAnnexB))
		{
			if ((10*UERC1 < 4*BMPG1) || (3*UERC1 > 5*BMPG1))
			{
				/*resync FEC, false MPEG Lock detected*/
				BDBG_MSG(("QAM AnnexB FEC detected an MPEG Checksum False Sync, staying locked but Resyncing FEC"));
				ReSyncFlag = 1;
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_RST, FECRST, 1, chn);	
				h->pStatus->ReSyncCount++;
			}
		}
		/*Reset Bad Block Counter*/
		h->pAcquireParam->BadBlockCount = 0;
		
		/*Declare Lock*/
		FECStatus = BADS_FECStatus_eLocked;

		BDBG_MSG(("LOCK1 chn %u", chn));
	}
	else
	{
		/*Wait for NUM_BAD_BLOCK_TO_UNLOCK bad blocks without a good one to declare unlock*/
		if((UERC1 - h->pAcquireParam->Old_UERC1 + h->pAcquireParam->BadBlockCount) > NUM_BAD_BLOCK_TO_UNLOCK)	
		{
	
			/*Reset Bad Block Counter*/
			h->pAcquireParam->BadBlockCount = 0;
			
			/*Declare UnLock*/
			FECStatus = BADS_FECStatus_eNotLocked;
			
			BDBG_MSG(("          UNLOCK1 chn %u", chn));
		}
		else
		{
			if (CurrentFECStatus == BADS_FECStatus_eNotLocked) 
			{
				/*Declare UnLock*/
				FECStatus = BADS_FECStatus_eNotLocked;
			}
			else
			{
				BDBG_MSG(("          UNLOCK2 chn %u", chn));
				/*NUM_BAD_BLOCK_TO_UNLOCK bad blocks did not come along without a good one so wait until next call*/
				/*increment bad block counter*/
				h->pAcquireParam->BadBlockCount += (UERC1 - h->pAcquireParam->Old_UERC1);
			
				/*Declare Lock*/
				FECStatus = BADS_FECStatus_eLocked;
				BDBG_MSG(("                    Keep LOCK2 chn %u", chn));
			}
		}
	}
 
	/*Check for stuck FEC condition, all counters did not move since the last call to BADS_7125_P_Get_LockStatus()*/
	if ((CBERC1 - h->pAcquireParam->Old_CBERC1 == 0) && (UERC1 - h->pAcquireParam->Old_UERC1 == 0) && 
			(NBERC1 - h->pAcquireParam->Old_NBERC1 == 0))
	{
		if ((h->pAcquireParam->StuckFECCount == 0) || (CurrentFECStatus == BADS_FECStatus_eNotLocked))
		{
		  /*Reset Counters*/
			h->pAcquireParam->StuckFECCount = 0;
			h->pAcquireParam->BadBlockCount = 0;

			/*Declare UnLock*/
 			FECStatus = BADS_FECStatus_eNotLocked;
			BDBG_MSG(("QAM%u FEC is stuck, declare an unlock condition", chn));
		}	
		else
		{
			h->pAcquireParam->StuckFECCount--;
			BDBG_MSG(("QAM%u  FEC is stuck, Count = %d", chn, h->pAcquireParam->StuckFECCount));
		}
	}
 
 
	/*store counter values for the next call*/
	h->pAcquireParam->Old_CBERC1 = CBERC1;
	h->pAcquireParam->Old_UERC1 = UERC1;
	h->pAcquireParam->Old_NBERC1 = NBERC1;

	/*clear FEC counters to prevent overflow*/
	/*Resync The FEC for bad MPEG packet count*/
	/*Reset FEC Counters*/
	if ((BMPG1 > POWER2_31) || (CBERC1 > POWER2_31) || (UERC1 > POWER2_31) || (NBERC1 > POWER2_31) || (ReSyncFlag == 1)) 
	{			
		BDBG_MSG(("                                                  RESYNCING chn %u", chn));
		ReSyncFlag = 0;
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TPFEC, 0x000F8000, chn);
		h->pAcquireParam->Old_UERC1 = 0;
		h->pAcquireParam->Old_NBERC1 = 0;
		h->pAcquireParam->Old_CBERC1 = 0;
	}

	/*assign values to the status structure to be read by BBS AND used by the PI*/
	h->pStatus->FecStatus = (uint32_t)FECStatus;

	h->pStatus->TimingStatus = (uint32_t)h->pAcquireParam->TimingCheck;
	h->pStatus->SpectrumStatus  = (uint32_t)h->pAcquireParam->FECSpectrum;

	/* increment poll counter*/
	h->pStatus->pCount++;
 
	/*LockStatus Complete*/
  retCode = BERR_SUCCESS;
  return retCode;
}

/***************************************************************************
 * BADS_7125_P_Acquire()
 ***************************************************************************/
BERR_Code BADS_7125_P_Acquire(BADS_ChannelHandle hChannel)
{
	BERR_Code retCode;
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;

	/* Local Variables */
	
	uint32_t	SampleRate;
	uint8_t		FilterFactor;
	uint8_t		ARFFactor;
	uint8_t		FE_Index;
	uint32_t  VID_SampleRate;
	uint32_t	ReadReg;
	int32_t   VIDTimingErrorOrig, VIDTimingError;
 	BADS_QAMStatus_t QamStatus;
	BADS_FECStatus_t FECStatus;
	MVAR_t MVAR;

	/*initial value for the FS counter to allow the acquisition time to be profiled*/
	/*the profile timing MUST be done with the printing to monitor turned off*/
	uint32_t FscntInit=0x00ffffff;
	BADS_OffOn_t CounterMaskBit; 
	uint32_t TimeMs[8];
	uint32_t TotalTimeMs[8];
	
 	BDBG_MSG(("BADS_7125_P_Acquire() chn %u",chn));

	/*Begin by being unlocked*/
	QamStatus = BADS_QAMStatus_eLocked; 
	FECStatus = BADS_FECStatus_eNotLocked;
	h->pStatus->QamStatus = (uint32_t)QamStatus;
	h->pStatus->FecStatus = (uint32_t)FECStatus;

	h->pStatus->ReAcquireCount++;
	
	/*Acquisition Timing Profile*/
	/*Get the current state of the interrupt mask*/
	/*Acquisition Time Profile is beginning, restore the interrupt mask at the end of acquisition*/
	/*Turn off the interrupt mask for the duration of the acquisition*/
	/*set the counter to 0 to freeze it*/
	/*clear the interrupt status register*/
	/*start counter*/
	/*Begin Timing Event [0]*/
	CounterMaskBit = BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_IRQMSK2, FSCNT1_IMSK, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_IRQMSET2, FSCNT1_IMSET, 1, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_IRQCLR2, FSCNT1_ICLR, 1, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);

	/*check if this is the first time to call acquire*/
	if ((h->pAcquireParam->DoneFirstTimeFlag == BADS_OffOn_eOff) || (h->pAcquireParam->DoneLoadFlag == BADS_OffOn_eOff) || ((BADS_OffOn_t)h->pAcquireParam->BBS_Rerun_Init == BADS_OffOn_eOn))
	{
		BADS_7125_P_Init(hChannel);
	}
	
	/*set the DoneFirstTimeFlag*/
	h->pAcquireParam->DoneFirstTimeFlag = BADS_OffOn_eOn;

	/*Range Check the Inputs*/
	/*Map the PI QAM/FEC combined structure BADS_ModulationType*/
	/*to the local structures BADS_AnnexMode_t and BADS_QAM_t*/
	BADS_7125_P_Range_Check(h);

	/*Reset the ADS Core*/ 
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_RST, 0x007FBFF9, chn);		/*MCW DOCSIS asserts SFTRST in DS_GBL, then enables DS to MAC */ 

	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AFECLK, 0x00000000, chn);	/*AFE Clock Phase Alignment Control*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_CLK, 0x00000F00, chn);		/*turn on PS clock and ADC clock to US*/ 
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_ICB_CTL, BUS_ERR_EN, 0, chn);	/*Internal Configuration Bus Control and Status and Stops bus errors*/

	BKNI_Sleep(2);
	/*The Initialization routine is run, the Structures loaded with default values*/
	/*DoneFirstTimeFlag is set, DoneLoadFlag is set*/
	/*Check Timing Event [0], Begin Timing Event [1]*/
	TimeMs[0] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[0] = TimeMs[0];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);

	/*Reset FEC Counters*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_RST, FECRST, 1, chn);			/*Reset FEC*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TPFEC, 0x000F9F00, chn);
	h->pAcquireParam->Old_CBERC1 = 0;
	h->pAcquireParam->Old_UERC1 = 0;
	h->pAcquireParam->Old_NBERC1 = 0;
	h->pStatus->ReSyncCount = 0;
	h->pAcquireParam->StuckFECCount = STUCK_FEC_RESET_COUNT;

	/*************************************************************************************************************/
	/*BEGIN INITIALIZATION*/
	/*************************************************************************************************************/

	/* Configure Downstream for Operation */
	/* add alternate resets */
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_QDSAFE_0_0_CTRL1, CLK_RESET, 0x1, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_QDSAFE_0_0_CTRL1, CLK_RESET, 0x0, chn);

	/*start with loops frozen*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FRZ, 0x1F7FE7FE, chn);

	/*  FEC NCO programming: use byte clock, serial output mode, invert clock */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_OI_CTL, 0x00000011, chn);

	/*Program the FEC and FEC NCO */ 
	BADS_7125_P_ProgramFEC(hChannel);

	/*Configuration of DS Front End*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CHNZ_0_FE, 0x00089801, chn);		    /* enable front end signature analyzers, thirdband (odd length), data from regular input  */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CHNZ_0_CFLOS, 0x000007FF, chn);		/* enable 1st fixed mixer, mix with (1+0*j)*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_CFL, CFLRST, 1, chn);						/* Reset CFL loop*/

	/*Look up the Carrier Phase Loop Control Initial Parameters and Reset Integrator*/ 
	/*The Carrier Phase Loop Initial Settings are in a Lookup Table in acquire.h*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPL, DS_EQ_CPL_TBL[h->pAcquireParam->QAM], chn);	

	if (h->pAcquireParam->Annex == BADS_Annex_eAnnexA)
	{
		/* Annex A 64/256/512/1024QAM Alpha 15%, bypass BBDCOC */
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_BR, ALPHA, 1, chn);
	}
    else if (h->pAcquireParam->Annex == BADS_Annex_eAnnexC)
    {
        /* Annex A 64/256QAM Alpha 12%, bypass BBDCOC */
        BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_BR, ALPHA, 0, chn);
    }
	else
	{
		if (h->pAcquireParam->QAM == BADS_QAM_eQam64)
		{
			/* Annex B, 64QAM, Alpha 18%, bypass BBDCOC */
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_BR, ALPHA, 0x2, chn);
		}
		else
		{
			/* Annex B, 256/512/1024QAM Alpha 12%, bypass BBDCOC */
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_BR, ALPHA, 0x0, chn);
		}
	}
    /* bypass BBDCOC */
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_BR, BDCBYP, 0x1, chn);

	/*Call the Function to program the Front End Filter that replaces Filter C*/ 
	BADS_7125_P_ProgramFilters(hChannel);

	/*Get the Sample Rate*/
	SampleRate = BADS_7125_P_Get_SampleRate(hChannel);

	/*Set up timing & sweep parameters*/
	MVAR = BADS_7500_P_Get_MVAR(hChannel);
	
	 

	/****************************************************************************************************/
	/*Set the Timing Loop Frequency Control Word a 24 bit 2's complement number*/
	/*The NCO value depends on the Symbol Rate, SampleRate, ARF, FilterFactor, and Timing Loop NCO value*/
	/*TLFOS = 2^25*FilterFactor*SymbolRate/VID_SampleRate: VID_SampleRate=SampleRate/(ARF*FilterFactor)*/
	/****************************************************************************************************/
	ARFFactor = BADS_7125_P_Get_ARFFactor(hChannel);
	FilterFactor = BADS_7125_P_Get_FilterFactor(hChannel);
	FE_Index = ((ARFFactor>>1) + (FilterFactor>>1)); /*Front End Decimation Rate for the VID Coeff Lookup Tables*/
	
	/*This is an integer division, ARFFactor = 1,2 and FilterFactor is 1,2,4*/
	/*Check for division by 0                                               */
	if (ARFFactor*FilterFactor == 0)
	{
		BDBG_ERR(("divide by 0 in BADS_7125_P_Acquire() while calculating VID_SampleRate"));
	}	
	VID_SampleRate = SampleRate/(ARFFactor*FilterFactor);

	/*Set the timing loop frequency control word*/
	BADS_7125_P_Set_TimingFrequency(hChannel, MVAR.mvarDownSymbolRate, SampleRate, FilterFactor, ARFFactor);
	
	/****************************************************************************************************/
	/*Calculate the Carrier Loop Frequency Control Word a 16 bit 2's complement number*/
	/*The NCO value depends on the SampleRate, IF Frequency a 2's complement number passed as uint32_t*/
	/*CFLFOS = 2^16*IF/SampleRate*/
	/****************************************************************************************************/
	
	/*Set the carrier loop frequency control word*/
	BADS_7125_P_Set_IfFrequency(hChannel, h->pAcquireParam->IfFrequency, SampleRate);

	/*Set up CWC, all four taps identical, AFC mode, 0 Hz*/
	/*Reset/Freeze ALL CWC integrators, set all to PLL_MODE=AFC mode, set all mu=2^-8, Freeze ALL CWC Tap Freeze, set to LENGTH=1 Tap*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CWC, 0xFF0492F1, chn);									
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FSBAUD, FCW_BAUD_SEL, 1, chn);					/*use integrator of TL*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FSBAUD, FSBAUD_Hz, (uint32_t)(7312500), chn);	/*use pre-computed value*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FSCARR, FCW_CARR_SEL, 1, chn);					/*use integrator of CFL*/	
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FSCARR, FSCARR_Hz, (uint32_t)(29250000), chn);	/*use pre-computed value*/	  
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CWC_LFC1, 0x00000000, chn);
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CWC_LFC2, 0x00000000, chn);
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CWC_LFC3, 0x00000000, chn);
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CWC_LFC4, 0x00000000, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FIN1, FIN1_HZ, (int32_t)h->pAcquireParam->BBS_CWC1_Freq, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FIN2, FIN2_HZ, (int32_t)h->pAcquireParam->BBS_CWC2_Freq, chn);		 
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FIN3, FIN3_HZ, (int32_t)h->pAcquireParam->BBS_CWC3_Freq, chn);		 
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC_FIN4, FIN4_HZ, (int32_t)h->pAcquireParam->BBS_CWC4_Freq, chn);		 

	/*Setup IQ Imb */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CORE_GEN_0_0_IQAMP, 0x00000063, chn);/*IQ AMP BW=2^(-16), freeze,rst, freeze   //LN072409 .. note reset here only clears the ampl register*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CORE_GEN_0_0_IQPHS, 0x00000067, chn);/*IQ PHS BW=2^(-16), freeze,rst,bypass*/

    /*Setup AGFI: frozen by DS_FRZ*/      
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CHNZ_0_AGF , 0x0C080001, chn);		/*BW=2^-14, TH=0.75, Reset AGF*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_TUNER_CHNZ_0_AGFI , 0x00080000, chn);		/*Init AGFI Integrator*/
											
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FRZ, 0x077FE7FC, chn);					/*Release AGF*/ 	 	

	/*Setup AGCBI_IR: frozen by DS_FRZ*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCB, 0x0a001201, chn);					/*BW=2^-13, TH=1.0, Reset AGCB*/ 	 	
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCBI, 0x03F00000, chn);					/*Init AGCBI Integrator*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCB_IR, 0x0a001201, chn);				/*BW=2^-13, TH=1.0, Reset AGCBI_IR*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_AGCBI_IR , 0x03F00000, chn);				/*Init AGCBI_IR Integrator*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FRZ, 0x073FE7FC, chn);					/*Release AGCB*/ 	 	

	BKNI_Sleep(2);
	/*Setup Timing Loop: frozen by DS_FRZ*/
	
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TL , 0x00000089, chn);		/* Full precision PD, Enable */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TLAGCI , 0x0100000, chn);	/* Set Timing Loop AGC to Gain 8x */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TLC , 0x00003839, chn);		/* Set timing loop coefficients */
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FRZ, 0x033FE6FC, chn);		/* Release Timing loop */ 	 	

	/*Look up the Carrier Phase Loop Control Initial Parameters and Reset Integrator*/ 
	/*Set CPL Linear and Integrator coefficients to Acquisition Bandwidth*/
	/*The Carrier Phase Loop Initial Settings are in a Lookup Table in acquire.h*/
	/*The Carrier Phase Loop Coefficient are in a Lookup Table in acquire.h*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPL, DS_EQ_CPL_TBL[h->pAcquireParam->QAM], chn);	
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CPLC, COMBO_COEFFS, PhaseLoopAcqCoeffs_TBL[MVAR.BaudRateIndex], chn);	
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPLI, 0, chn);
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPLSWP, 0, chn);
	
	/*Setup hum-AGC Gain offset, leakage, reset loop integrator*/
    if ((BADS_OffOn_t)h->pAcquireParam->BBS_DDAGC_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGC, 0x00400001, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGCC, 0x00a00050, chn);	/*AGCLCOEFF = 160*2^-15, AGCICOEFF = 80*2^-22*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGCPA, 0x00000000, chn);
	}

	/*Setup FFE/DFE/IMC, hum-AGC in FN mode, derot frozen, CWC with loop frozen, start CMA fast*/
    /*TLS 20091020  override values for FN modulus to fix non-square modes when DD AGC is on*/
	/* DS_EQ.FN = &h40000000&	'//override value for FN modulus, Scaler = FnErr/2^4*/
	/*The Equalizer FN Modulus Settings are in a Lookup Table in acquire.h*/
	 BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_FN, DS_EQ_FN_TBL[h->pAcquireParam->QAM], chn); 

	/*Setup override value for CMA modulus*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CMA, 0x00000000, chn);	

	/*Setup Equalizer and Hum_AGC enable*/
	/*The Equalizer Initial Settings are in a Lookup Table in acquire.h*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CTL, DS_EQ_CTL_TBL[h->pAcquireParam->QAM], chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, HUM_EN, 0, chn);

	/*Initalize and reset FFE: frozen by DS_FRZ*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_FFE, 0x14101119, chn);	/*Main tap at tap 20, length = 36 taps, main mu = 2^-6, other mu = 2^-8, update at symbol rate*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_FFEU20, 0x30000000, chn);	/*Initialize Main Tap 20*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_LEAK, 0x00000000, chn);	/*no leakage*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_FMTHR, 0x00000020, chn);	/*set main tap threshhold to gen IRQ*/
	
  /*Initailize and reset DFE: frozen by DS_FRZ*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_DFE, 0x00001829, chn);	/*DFE length = 36 taps, mu = 2^-8, reset taps*/

  /*Initailize and reset IMC: frozen by DS_FRZ*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, IMC_EN, 0, chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_IMC, RESET, 1, chn);

	/*Program the FEC and FEC NCO */ 
	BADS_7125_P_ProgramFEC(hChannel);

	/*Check Timing Event [1], Begin Timing Event [2]*/
	TimeMs[1] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[1] = TimeMs[1]+TotalTimeMs[0];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);
	
	/*************************************************************************************************************/
	/*INITIALIZATION FINISHED: BEGIN ACQUISITION*/
	/*************************************************************************************************************/
 
	/*Release DPM_DC_Second_Order/AGF/AGCB/AGCB_IR loops*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	DPMK2DCFRZ, 0, chn);		/*Release DPM DC canceller Second Order*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	AGCBFRZ_IR, 0, chn);		/*Release AGCB Freeze for IMC path*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	AGFFRZ, 0, chn);			/*Release Fine AGC*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	AGCBFRZ, 0, chn);			/*Release AGCB*/
 
	/*wait for DPM_DC/AGF/AGCB/AGCB_IR to settle*/
	BKNI_Sleep(2);

	/*Reduce AGF/AGCB/AGCB_IR bandwidth to tracking values*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CHNZ_0_AGF, AGFBW, 0x4, chn);	/*Reduce AGF BW 2^-22*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_AGCB, AGCBBW, 0xf, chn);			/*Reduce AGCB BW 2^-16*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_AGCB_IR, AGCBBW_IR, 0xf, chn);		/*Reduce AGCB_IR BW 2^-16*/

	/*Release IQAMP & IQPHS after AGCs have settled using initial bandwidths*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_IQIMB_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQAMP, iqamp_byp, 0, chn);		/*unbypass IQAMP*/
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQAMP, iqamp_freeze, 0, chn);		/*unfreeze IQAMP*/
        if ((BADS_OffOn_t)h->pAcquireParam->BBS_IQPHS_on == BADS_OffOn_eOn)	/*'LN102509 this is added because of the IQ saturation issue */		
      	{
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQPHS, iqphs_byp, 0, chn);	/*unbypass IQPHS*/
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQPHS, iqphs_freeze, 0, chn);	/*unfreeze IQPHS*/
      	} 
	}
												
	/*Release DPM_DC/Timing loops*/
	
	/*I think this is a mistake and DPM should not be released CAB 5/5/2010*/
	/*BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	DPMDCFRZ, 0, chn);*/		/*Release DPM DC canceller*/
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	TLFRZ, 0, chn);			/*Release Timing Loop*/

	/*wait for timing loop to converge*/
	BKNI_Sleep(3);										/*re-added timing dwell for large sym offsets [TLS 20091109])*/
	
	/*Check Timing Lock*/ 
	/*************************************************************************************************************/
	/*This can check for timing lock as well as try 64/256 QAM in Annex B mode*/
	/*Set Timing Status flag*/
	h->pAcquireParam->TimingCheck = BADS_Timing_eNotChecked;
	/*The acquisition will end early if timing is out of range after sweep*/ 
	if (h->pAcquireParam->fastAcquire == BADS_eSlowAcquire)
	{
		/*The Timing Sweep will push the loop in the positive direction*/
 		VIDTimingErrorOrig = BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, SampleRate);
		VIDTimingError = VIDTimingErrorOrig;
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLSWP,	TLSWP, 0x0000A000, chn);	/*Start Timing Sweep*/
		/*Wait for timing to be pushed out UP TO 10 ms*/
		BADS_7500_P_SLEEP(hChannel, 10, VID_SampleRate);
		while ((BADS_7500_P_SLEEP(hChannel, 0, 0) != 0) && (VIDTimingError >= (-1*MAX_TIMING_OFFSET_FOR_LOCK)) && (VIDTimingError <= MAX_TIMING_OFFSET_FOR_LOCK))
			{
 				VIDTimingError = BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, SampleRate) - VIDTimingErrorOrig;
			}
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLSWP,	TLSWP, 0, chn);				/*End Timing Sweep*/
	
		/*BKNI_Sleep(10);*/
		/*BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLSWP,	TLSWP, 0, chn);	*/			/*End Timing Sweep*/
 		/*VIDTimingError = BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, SampleRate) - VIDTimingError;*/
		 
		if ((VIDTimingError < (-1*MAX_TIMING_OFFSET_FOR_LOCK)) || (VIDTimingError > MAX_TIMING_OFFSET_FOR_LOCK))
		{
			/*Try Other Flavor*/
 			if ((h->pAcquireParam->fastAcquire == BADS_eSlowAcquire) && (h->pAcquireParam->Annex == BADS_Annex_eAnnexB) && ((h->pAcquireParam->QAM == BADS_QAM_eQam64) || (h->pAcquireParam->QAM == BADS_QAM_eQam256)))
			{
				
				if (h->pAcquireParam->QAM == BADS_QAM_eQam256)
				{
					h->pAcquireParam->QAM = BADS_QAM_eQam64;
				}
				else
				{
					h->pAcquireParam->QAM = BADS_QAM_eQam256;
				}

				/*********************************************************************************/
				/*To flip the 64/256 QAM AutoFlip, the front end must be reset to original values*/
				/*********************************************************************************/
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	TLFRZ, 1, chn);
				BADS_7125_P_ProgramFilters(hChannel);
				MVAR = BADS_7500_P_Get_MVAR(hChannel);
				BADS_7125_P_Set_TimingFrequency(hChannel, MVAR.mvarDownSymbolRate, SampleRate, FilterFactor, ARFFactor);
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_CFLC, COMBO_COEFFS, FrequencyLoopCoeffs_TBL[MVAR.BaudRateIndex], chn);
				BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPL, DS_EQ_CPL_TBL[h->pAcquireParam->QAM], chn);	
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CPLC, COMBO_COEFFS, PhaseLoopAcqCoeffs_TBL[MVAR.BaudRateIndex], chn);	
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLC, COMBO_COEFFS, TimingLoopAcqCoeffs_TBL[MVAR.BaudRateIndex][FE_Index], chn);
				BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_FN, DS_EQ_FN_TBL[h->pAcquireParam->QAM], chn); 
				BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CTL, DS_EQ_CTL_TBL[h->pAcquireParam->QAM], chn);
				BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TLAGCI , 0x0100000, chn);
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLI,	TLVAL, 0, chn);	
				BADS_7125_P_ProgramFEC(hChannel);
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	TLFRZ, 0, chn);
				/*********************************************************************************/
				/*********************************************************************************/
				
				BKNI_Sleep(3);
				/*The Timing Sweep will push the loop in the positive direction*/
 				VIDTimingErrorOrig = BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, SampleRate);
				VIDTimingError = VIDTimingErrorOrig;
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLSWP,	TLSWP, 0x0000A000, chn);	/*Start Timing Sweep*/
				/*Wait for timing to be pushed out UP TO 10 ms*/
				BADS_7500_P_SLEEP(hChannel, 10, VID_SampleRate);
				while ((BADS_7500_P_SLEEP(hChannel, 0, 0) != 0) && (VIDTimingError >= (-1*MAX_TIMING_OFFSET_FOR_LOCK)) && (VIDTimingError <= MAX_TIMING_OFFSET_FOR_LOCK))
				{
 					VIDTimingError = BADS_7125_P_Get_VIDTimingError(hChannel, ARFFactor, FilterFactor, SampleRate) - VIDTimingErrorOrig;
				}
				BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLSWP,	TLSWP, 0, chn);				/*End Timing Sweep*/
 		}
						
			if ((VIDTimingError < (-1*MAX_TIMING_OFFSET_FOR_LOCK)) || (VIDTimingError > MAX_TIMING_OFFSET_FOR_LOCK))
			{
				h->pAcquireParam->TimingCheck = BADS_Timing_eUnLocked;
				/*Leaving Early*/
				/*Check Timing Event [2], Begin Timing Event [3]*/
				TimeMs[2] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
				TotalTimeMs[2] = TimeMs[2]+TotalTimeMs[1];
				if(h->pAcquireParam->BBS_AcqWord1 == 1)
				{
					BDBG_WRN(("Timing Not Locked, Exit Early, error is %d  Acquisition time is %d msec",VIDTimingError, TotalTimeMs[2]/29250));
				}
				else
				{
					BDBG_MSG(("Timing Not Locked, Exit Early, error is %d",VIDTimingError));

				}
				retCode = BERR_SUCCESS;
				return retCode;
			}
			else
			{
				h->pAcquireParam->TimingCheck = BADS_Timing_eLockedAlternateQam;
				BDBG_MSG(("Timing Locked Alternate QAM, error is %d",VIDTimingError));

			}
		}
		else
		{
			h->pAcquireParam->TimingCheck = BADS_Timing_eLocked;
			BDBG_MSG(("Timing Locked, error is %d",VIDTimingError));
		}
	}
	/*************************************************************************************************************/

	/*Reduce timing loop coefficients*/
	/*The Timing Loop coefficient are in a Lookup Table in acquire.h*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLC, COMBO_COEFFS, TimingLoopTrk1Coeffs_TBL[MVAR.BaudRateIndex][FE_Index], chn);	
	BKNI_Sleep(1);

	/*Reduce timing loop coefficients to tracking bandwidths*/
	/*The Timing Loop coefficient are in a Lookup Table in acquire.h*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TLC, COMBO_COEFFS, TimingLoopTrk2Coeffs_TBL[MVAR.BaudRateIndex][FE_Index], chn);	
	BKNI_Sleep(1);

	/*Timing Loop Should Be Locked*/
	/*Check Timing Event [2], Begin Timing Event [3]*/
	TimeMs[2] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[2] = TimeMs[2]+TotalTimeMs[1];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);
	
	/*Setup hum-AGC Gain offset, leakage, reset loop integrator*/
    if ((BADS_OffOn_t)h->pAcquireParam->BBS_DDAGC_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGC, 0x00400001, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGCC, 0x00a00050, chn);/*AGCLCOEFF = 160*2^-15, AGCICOEFF = 80*2^-22*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGCPA, 0x00000000, chn);
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, HUM_EN, 1, chn);
	}

	/*Start IMC*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_IMC_on == BADS_OffOn_eOn)
	{ 
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, IMC_EN, 1, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_IMC, 0x0c081119, chn);	/*36 IMC taps, main tap is 8 left of FFE main, mu = 2^-6, full LMS, update sym, leak off, coarse mode off*/
  }	
	
	/*Reduce IQAMP & IQPHS to tracking bandwidths*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_IQIMB_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQAMP, iqamp_shift, 0x7, chn);	/*IQ AMP BW=2^(-17)*/    
		if ((BADS_OffOn_t)h->pAcquireParam->BBS_IQPHS_on == BADS_OffOn_eOn)									/*'LN102509 this is added because of the IQ saturation issue */		
      	{
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_TUNER_CORE_GEN_0_0_IQPHS, iqphs_shift, 0x8, chn);/*IQ PHS BW=2^(-18)*/    
      	} 
	}

	/*Release EQ-DD-AGC/FFE (not main Tap)*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	DDAGCFRZ, 0, chn);			/*Release Equalizer DD-AGC Loop*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	FFEFRZ, 0, chn);			/*Release FFE*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_DDAGC_on == BADS_OffOn_eOff)
	{	
		/*Release FFE Main Tap Real*/
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	FFRZMR, 0, chn);		/*Release FFE Main Tap Real*/
	}
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	COMBO_DFEFRZ, 0, chn);		/*Release All DFE taps*/

	/*Wait for FFE/DFE to open eye*/
	BKNI_Sleep(3);  /* from 2 to 4 TLS 20091020*/
	
	/*reduce FFE Mus*/				
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_FFE, MAINSTEP, 0x2, chn);		/*main mu = 2^-8*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_FFE, STEP, 0x3, chn);			/*other mu = 2^-10*/

	/*Wait for FFE and DFE to open eye more*/
	BKNI_Sleep(3);     /* 1 to 4 TLS 20091020*/

	/*Start Carrier Phase Loop*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPLI, (uint32_t)MVAR.mvarSwp_Start, chn);	/*Set up CPL Sweep*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPLSWP, (uint32_t)MVAR.mvarSwp_Rate, chn);/*Set sweep rate [TLS 20091109] global variable added*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	CPLFRZ, 0, chn);				/*Release Carrier Phase Loop Freeze*/
	BKNI_Sleep((uint32_t)MVAR.mvarSwp_Delay);											
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CPLSWP, SWEEP, 0, chn);				/*Stop Sweeping*/

	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	IMCFRZ, 0, chn);				/*Release Image Canceller*/
 
	/*Reduce Carrier Phase Loop coefficient to Tracking Bandwidth*/
	/*The Carrier Phase Loop coefficient are in a Lookup Table in acquire.h*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CPLC, COMBO_COEFFS, PhaseLoopTrkCoeffs_TBL[MVAR.BaudRateIndex], chn);	

  if ((BADS_OffOn_t)h->pAcquireParam->BBS_CFL_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	CFLFRZ, 0, chn);			/*Release Carrier Frequency Loop Freeze*/
	}

	/*Carrier Loop Should Be Locked*/
	/*Check Timing Event [3], Begin Timing Event [4]*/
	TimeMs[3] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[3] = TimeMs[3]+TotalTimeMs[2];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);

	/*Start CWC after derotator locked*/
	/*Length is set in initialization*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_CWC_Enable == BADS_OffOn_eOn)
	{													   			
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC, LF_FRZ, 0xE, chn);	/*Unfreeze 1 CWC integrator*/
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CWC, FREEZE, 0xE, chn);	/*Unfreeze 1 CWC Tap Freeze*/

	}
	
	/*Set DDAGC BW for Tracking Mode*/
	/*This register makes more sense to treat as a 32 bit write*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_DDAGC_on == BADS_OffOn_eOn)
	{ 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_AGCC, 0x00400010, chn);	/*AGCLCOEFF = 64*2^-15, AGCICOEFF = 16*2^-22*/	
	}

	/*Switch EQ to LMS mode*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, CMAEN, 0, chn);
	BKNI_Sleep(1);

	/*EQ is in LMS mode*/
	/*Check Timing Event [4], Begin Timing Event [5]*/
	TimeMs[4] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[4] = TimeMs[4]+TotalTimeMs[3];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);

	/*Reduce IMC Bandwidth*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_IMC_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_IMC, STEP, 2, chn);		/*Set IMC Step Size = 2^-8*/
	}

	/*extra time seems to be needed before switching the DD-AGC in 512/1024 QAM*/
	if ((h->pAcquireParam->QAM == BADS_QAM_eQam512) || (h->pAcquireParam->QAM == BADS_QAM_eQam1024))
	{
		BKNI_Sleep(15);
	}


	/*Switch DDAGC to DD mode*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_DDAGC_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_CTL, FNEN, 0, chn);		 
	} 

	/*Start Post FFE Leak*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_LEAK, FFE_LEAK_POST, 0xb, chn);	/* leak at sym_rate, (2^4)*/			

	/*Set the final Phase Loop parameters*/
	/*This register makes more sense to treat as a 32 bit write*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPL, 0x00000000, chn);		/*Don't leak the CPL when not using the CFL, use all points*/

	/*Unfreeze Front loop and start leak in carrier phase loop*/
	if ((BADS_OffOn_t)h->pAcquireParam->BBS_CFL_on == BADS_OffOn_eOn)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FRZ,	CFLFRZ, 0, chn);		
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPL, 0x00001700, chn);	/*Turn on CPL int. leak: Fixed leakage, 2^-20, use all points*/
	}

	/*BKNI_Sleep(20);*/

	/*program burst registers used in the Annex A burst detection*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BND, 0x04000000, chn); 
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BND_THR, 0x00000000, chn); 
	
	/*Configure SNR performance Monitoring*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_SNRLT, SNRLTHRESH, SNRLTHRESH_TBL[h->pAcquireParam->QAM], chn);
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_EQ_0_SNRHT, SNRHTHRESH, SNRHTHRESH_TBL[h->pAcquireParam->QAM], chn);

	/*Set the Burst Mode parameters for Annex A*/
	/*These are values that overwrite the standard acquisition values and are only valid at high baud rates*/
 	if (((BADS_OffOn_t)h->pAcquireParam->BBS_Burst_mode == BADS_OffOn_eOn) && (h->pAcquireParam->Annex == BADS_Annex_eAnnexA))  
	{
		/*dgm - 4/21/09, added burst noise enhancement code from TLS*/
		/*change tracking bandwidths*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_CPLC, 0xFF00FF80, chn);	/*Fn=30k, Damp=0.44 for 6.9Mbaud*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_CFLC, 0x00002521, chn);		/*Fn=1k,  Damp=0.70 for 6.9Mbaud @ Fs=54.0M*/

		/*program burst registers*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BND, 0x08000000, chn); 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BND_THR, 0x00072B02, chn); 

		/*turn on erasures in FEC*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECU, (AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECU | 0x0000000C), chn); 
	}
 
	/*************************************************************************************************************/
	/*ACQUISITION FINISHED: BEGIN LOCK CHECKING/SPECTRAL INVERSION/AUTOINVERSION*/
	/*************************************************************************************************************/

	/*Set the FEC spectrum status Flag*/
	h->pAcquireParam->FECSpectrum = BADS_FECSpectrum_eNotInverted;
	
	/*Check if the spectrum is to be inverted from the default setting*/
	if (h->pAcquireParam->InvertSpectrum == BADS_InvertSpectrum_eInverted)
	{
		ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
		ReadReg = ReadReg ^ 0x00000008;												/*Perform Spectral Inversion in the FEC*/
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECL, ReadReg, chn);
		h->pAcquireParam->FECSpectrum = BADS_FECSpectrum_eInverted;
	}

	/*Wait for FEC to sync and output data UP TO 15 ms*/
	BADS_7500_P_SLEEP(hChannel, 15, VID_SampleRate);
	while ((BADS_7500_P_SLEEP(hChannel, 0, 0) != 0) && (BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_NBERC1, NBERCCNTVAL, chn) == 0))
	{
	}
	BADS_7125_P_Get_LockStatus(hChannel);

	/*Check to see if spectrum AutoInversion is activated*/
	if (h->pAcquireParam->spectrum == BADS_SpectrumMode_eAuto)
	{
		/*If FEC is not locked try inverting the spectrum*/
		if ((BADS_FECStatus_t)h->pStatus->FecStatus == BADS_FECStatus_eNotLocked)
		{
			/*Invert Spectrum if FEC is not locked*/
			ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_0_FECL, chn);
			ReadReg = ReadReg ^ 0x00000008;											/*Perform Spectral Inversion in the FEC*/
			BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECL, ReadReg, chn);
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_RST, FECRST, 1, chn);	/*Reset FEC*/
			BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_TPFEC, 0x000F9F00, chn);
			h->pAcquireParam->Old_CBERC1 = 0;
			h->pAcquireParam->Old_UERC1 = 0;
			h->pAcquireParam->Old_NBERC1 = 0;
			h->pStatus->ReSyncCount = 0;
			h->pAcquireParam->StuckFECCount = STUCK_FEC_RESET_COUNT;
			if (h->pAcquireParam->InvertSpectrum == BADS_InvertSpectrum_eInverted)
			{
				h->pAcquireParam->FECSpectrum = BADS_FECSpectrum_eInvertedAutoInvert;
			}
			else
			{
				h->pAcquireParam->FECSpectrum = BADS_FECSpectrum_eNotInvertedAutoInvert;
			}

			/*Wait for FEC to sync and output data UP TO 15 ms*/
			BADS_7500_P_SLEEP(hChannel, 15, VID_SampleRate);
			while ((BADS_7500_P_SLEEP(hChannel, 0, 0) != 0) && (BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_NBERC1, NBERCCNTVAL, chn) == 0))
			{
			}
			BADS_7125_P_Get_LockStatus(hChannel);
		}
	}

	/*Clear FEC lost lock interrupt*/
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_IRQCLR1, FEC_LOCK_DET_ICLR , 1, chn);
	
	/*Reset SNR and Output Interface*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_EQ_0_SNR , 0x0000000F, chn);	/*Reset ERRVAL, Force update, long averaging*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_OI_VCO , 0x00000004, chn);	/*Reset OI control and data paths*/
	BKNI_Sleep(1);
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_OI_CTL , 0x00020011, chn);	/*use byte clock, serial output mode, invert clock*/
	BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_OI_OUT, 0, chn);				/*in B0 this register muxes the transport data */
 
	/*Acquisition Complete*/	

	/*Check Timing Event [5], Begin Timing Event [6]*/
	TimeMs[5] =  FscntInit - BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, chn);
	TotalTimeMs[5] = TimeMs[5]+TotalTimeMs[4];
	BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT1, COUNTVAL, 0x00ffffff, chn);
	

	/*************************************************************************************************************/
	/*LOCK CHECKING/SPECTRAL INVERSION/AUTOINVERSION FINISHED*/
	/*************************************************************************************************************/

	/*Display Acquisition Time Profile*/
	if(h->pAcquireParam->BBS_AcqWord1 == 1)
	{
		/*BDBG_WRN(("Load/Init     Done: TimeMs[0] = %d ms  TotalTimeMs[0] = %d ms",TimeMs[0]/29250,TotalTimeMs[0]/29250));
		BDBG_WRN(("ADS Init      Done: TimeMs[1] = %d ms  TotalTimeMs[1] = %d ms",TimeMs[1]/29250,TotalTimeMs[1]/29250));
		BDBG_WRN(("Timing        Done: TimeMs[2] = %d ms  TotalTimeMs[2] = %d ms",TimeMs[2]/29250,TotalTimeMs[2]/29250));	
		BDBG_WRN(("Carrier       Done: TimeMs[3] = %d ms  TotalTimeMs[3] = %d ms",TimeMs[3]/29250,TotalTimeMs[3]/29250));	
		BDBG_WRN(("EQ-LMS        Done: TimeMs[4] = %d ms  TotalTimeMs[4] = %d ms",TimeMs[4]/29250,TotalTimeMs[4]/29250));*/
		BDBG_WRN(("Acquisition   Done: TimeMs[5] = %d ms  TotalTimeMs[5] = %d ms",TimeMs[5]/29250,TotalTimeMs[5]/29250));
	}

	/*Acquisition Time Profile is finished, restore the interrupt mask*/
	if (CounterMaskBit == BADS_OffOn_eOff)
	{
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_IRQMCLR2, FSCNT1_IMCLR, 1, chn);
	} 
	else
	{
			BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_IRQMSET2, FSCNT1_IMSET, 1, chn);
	}

		BKNI_Sleep(1);

		BADS_7125_P_Write32(hImplDev->hRegister,BCHP_DS_0_OI_CTL, 0x00020011, chn);		/*use byte clock, serial output mode, invert clock	//LN072409 for B0 change to DS.OI_CTL=&h00000010& */	
		BADS_7125_P_Write32(hImplDev->hRegister,BCHP_DS_0_OI_OUT, 0x00000000, chn);		/*in B0 this register allows direct control of the transport PS outputs*/

	retCode = BERR_SUCCESS;
	return retCode;
}

/********************************************************************************************
*Function to program the channel filter, I think the Annex A should be be based on BaudRate
********************************************************************************************/
void BADS_7125_P_ProgramFilters(BADS_ChannelHandle hChannel)	
{
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;
	
	uint32_t *FilterData;
	uint8_t i=0;
	uint8_t j=0;

    /*Program FilterC*/
	/*ALL ANNEX A FILTERS ARE THE SAME*/
	/*ALL ANNEX B FILTERS ARE THE SAME EXCEPT 64 QAM*/
	/*The Filter Coefficient are in a Lookup Table in acquire.h*/
    if ((h->pAcquireParam->Annex == BADS_Annex_eAnnexA) || 
        (h->pAcquireParam->Annex == BADS_Annex_eAnnexC))
	{
		FilterData = AnnexA_Filter_TBL;
	}  	
    else
	{ 
		if (h->pAcquireParam->QAM == BADS_QAM_eQam64)
		{
			FilterData = Q64AnnexB_Filter_TBL;
		}
		else
		{
			FilterData = Q256AnnexB_Filter_TBL;
		}
	}
	
	/*Access the data in the array and skip 3,7,11,15,and 19*/
	for (i=0;i<23;i++)
	{
		if((i != 3) && (i != 7) && (i != 11) && (i != 15) && (i != 19))
		{
			BADS_7125_P_Write32(hImplDev->hRegister,BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_ARRAY_BASE+(i*4), FilterData[j], chn);
			j=j+1; /*j will skip the 3,7,11,15,and 19 values*/
		}
	}	
}

/********************************************************************************************
*Function to program the FEC                                          
********************************************************************************************/
void BADS_7125_P_ProgramFEC(BADS_ChannelHandle hChannel)	
{
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;
		
	/*The FEC values are in a Lookup Table in bads_7125_acquire.h*/
	if ((h->pAcquireParam->Annex == BADS_Annex_eAnnexA) || (h->pAcquireParam->Annex == BADS_Annex_eAnnexC))
	{
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECU, AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECU, chn); 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECM, AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECM, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECL, AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECL, chn); 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECOUT_NCON, AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECOUT_NCON, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECOUT_NCODL, AnnexA_FEC_TBL[h->pAcquireParam->QAM].DS_FECOUT_NCODL, chn);
	}
	else
	{
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECU, AnnexB_FEC_TBL[h->pAcquireParam->QAM].DS_FECU, chn); 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECM, AnnexB_FEC_TBL[h->pAcquireParam->QAM].DS_FECM, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECL, AnnexB_FEC_TBL[h->pAcquireParam->QAM].DS_FECL, chn); 
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECOUT_NCON, AnnexB_FEC_TBL[h->pAcquireParam->QAM].DS_FECOUT_NCON, chn);
		BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_FECOUT_NCODL, AnnexB_FEC_TBL[h->pAcquireParam->QAM].DS_FECOUT_NCODL, chn);
	}

}

/********************************************************************************************
*Function to set up Symbol time dependent parameters, this is for the Timing Lock Check                                          
********************************************************************************************/
MVAR_t BADS_7500_P_Get_MVAR(BADS_ChannelHandle hChannel)	
{
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;

	/*Local Variables*/
	uint8_t LoopCount;
	MVAR_t MVAR;
			
	/****************************************************************************************************/
	/*Lookup the BaudRateIndex, Set the Nyquist Filter, and Lookup the Carrier Sweep Parameters*/
	/*For AnnexA there are 16 different baud rate settings*/
	/*For AnnexB there are only 2 settings, 64Qam and 256/1024Qam*/
	/*BaudRates, PhaseLoopSweepAnnexB and PhaseLoopSweepAnnexB parameters are in tables in acquire.h*/
	/****************************************************************************************************/
	MVAR.BaudRateIndex = 0;
		
	if ((h->pAcquireParam->Annex == BADS_Annex_eAnnexA) ||			/*16/32/64/128/256/512/1024 QAM Annex A, Symbol Rate = MIN_BAUD_RATE to MAX_BAUD_RATE*/
        (h->pAcquireParam->Annex == BADS_Annex_eAnnexC))
	{
		/*Lookup SymbolRateIndex which divides the symbol rate into 16 sections*/
		/*The RangeCheck() checks that the MIN_BAUD_RATE<h->pAcquireParam->SymbolRate<MAX_BAUD_RATE*/
		/*Make sure that the BaudRates[0]>=MIN_BAUD_RATE and BaudRates[LoopCount]<=MAX_BAUD_RATE*/
		if ((BaudRates_TBL[0]<MIN_BAUD_RATE) ||(BaudRates_TBL[NUM_BAUD_RATES-1]>MAX_BAUD_RATE))
		{
			BDBG_WRN(("AnnexA LookupTable has entries that are out of Range"));
		}

		for (LoopCount=0;LoopCount<NUM_BAUD_RATES;LoopCount++)
		{
			if (h->pAcquireParam->SymbolRate >= BaudRates_TBL[LoopCount])
			{
				MVAR.BaudRateIndex = LoopCount;
			}
		}
		/*Annex A Symbol times and sweep parameters*/
        if (h->pAcquireParam->Annex == BADS_Annex_eAnnexA)
        {
            BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BR, 0x00000102, chn);	/*Annex A 64/256/512/1024QAM Alpha 15%, bypass BBDCOC*/
        }
        else
        {
            BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BR, 0x00000002, chn);	/*Annex C 64/256QAM Alpha 12%, bypass BBDCOC*/
        }
		MVAR.mvarDownSymbolRate= h->pAcquireParam->SymbolRate;
		MVAR.mvarSwp_Delay = PhaseLoopSweepAnnexA_TBL[MVAR.BaudRateIndex].SweepTime;
		if ((BADS_SweepDirection_t)h->pAcquireParam->BBS_sweep_dir == BADS_SweepDirection_eNeg2Pos)
		{
			MVAR.mvarSwp_Rate =  PhaseLoopSweepAnnexA_TBL[MVAR.BaudRateIndex].PosSweepRate;
			MVAR.mvarSwp_Start = PhaseLoopSweepAnnexA_TBL[MVAR.BaudRateIndex].PosSweepStart;
		}
		else
		{
			MVAR.mvarSwp_Rate  = PhaseLoopSweepAnnexA_TBL[MVAR.BaudRateIndex].NegSweepRate;
			MVAR.mvarSwp_Start = PhaseLoopSweepAnnexA_TBL[MVAR.BaudRateIndex].NegSweepStart;
		}

	}
	else
	{
		if (h->pAcquireParam->QAM == BADS_QAM_eQam64)  /*64 QAM Annex B, Symbol Rate = 5056941, BaudRateIndex is 13*/
		{
			for (LoopCount=0;LoopCount<NUM_BAUD_RATES;LoopCount++)
			{
				if (Q64_ANNEXB_SYMBOL_RATE >= BaudRates_TBL[LoopCount])
				{
					MVAR.BaudRateIndex = LoopCount;
				}
			}
			BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BR, 0x00000202, chn);	/*Annex B, 64QAM, Alpha 18%, bypass BBDCOC*/
			MVAR.mvarDownSymbolRate = Q64_ANNEXB_SYMBOL_RATE;
			MVAR.mvarSwp_Delay = PhaseLoopSweepAnnexB_TBL[0].SweepTime;
			if ((BADS_SweepDirection_t)h->pAcquireParam->BBS_sweep_dir == BADS_SweepDirection_eNeg2Pos)
			{
				MVAR.mvarSwp_Rate =  PhaseLoopSweepAnnexB_TBL[0].PosSweepRate;
				MVAR.mvarSwp_Start = PhaseLoopSweepAnnexB_TBL[0].PosSweepStart;
			}
			else
			{
				MVAR.mvarSwp_Rate  = PhaseLoopSweepAnnexB_TBL[0].NegSweepRate;
				MVAR.mvarSwp_Start = PhaseLoopSweepAnnexB_TBL[0].NegSweepStart;
			}
		}
		else										/*256/1024 QAM Annex B, Symbol Rate = 5360537*/
		{
			for (LoopCount=0;LoopCount<NUM_BAUD_RATES;LoopCount++)
			{
				if (Q256_Q1024_ANNEXB_SYMBOL_RATE >= BaudRates_TBL[LoopCount])
				{
					MVAR.BaudRateIndex = LoopCount;
				}
			}
			BADS_7125_P_Write32(hImplDev->hRegister, BCHP_DS_0_BR, 0x00000002, chn);	/*Annex B, 256/1024QAM Alpha 12%, bypass BBDCOC*/
			MVAR.mvarDownSymbolRate = Q256_Q1024_ANNEXB_SYMBOL_RATE;
			MVAR.mvarSwp_Delay = PhaseLoopSweepAnnexB_TBL[1].SweepTime;
			if ((BADS_SweepDirection_t)h->pAcquireParam->BBS_sweep_dir == BADS_SweepDirection_eNeg2Pos)
			{
				MVAR.mvarSwp_Rate =  PhaseLoopSweepAnnexB_TBL[1].PosSweepRate;
				MVAR.mvarSwp_Start = PhaseLoopSweepAnnexB_TBL[1].PosSweepStart;
			}
			else
			{
				MVAR.mvarSwp_Rate  = PhaseLoopSweepAnnexB_TBL[1].NegSweepRate;
				MVAR.mvarSwp_Start = PhaseLoopSweepAnnexB_TBL[1].NegSweepStart;
			}
		}	 
	}
	/**********************************************************************/
		return MVAR;
}

/********************************************************************************************
*Function to set up internal timer using FSCNT2                                          
********************************************************************************************/
uint32_t BADS_7500_P_SLEEP(BADS_ChannelHandle hChannel, uint8_t DelayMs, uint32_t VID_SampleRate)	
{
	BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
	BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
	BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
	unsigned int chn = h->chnNo;

	/*Set counter if VID_SAMPLE RATE is nonzero, else do not set rate so the counter can be read*/
	if (VID_SampleRate != 0)
	{
		BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_FSCNT2, COUNTVAL, (DelayMs*VID_SampleRate)/1000, chn);
	}
	return BADS_7125_P_ReadField(hImplDev->hRegister, DS_0_FSCNT2, COUNTVAL, chn);
}
