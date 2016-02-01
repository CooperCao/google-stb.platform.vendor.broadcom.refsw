/******************************************************************************
 *    (c)2011-2012 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/


#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "btmr.h"
#include "bmth.h"
#ifdef LEAP_BASED_CODE
#include "baob_api.h"
#include "bwfe_global_clk.h"
#else
#include "baob.h"
BDBG_MODULE(baob_status);
#endif
#include "baob_struct.h"
#include "baob_acquire.h"
#include "baob_status.h"
#include "baob_utils.h"
#include "baob_priv.h"
#include "bchp_oob.h"

#define LOG10_OB_PREADC 0


/************************************************************************************************
BCM3128A0 : CTRL3 and CTRL6 have a bug --- need to read/write all 32 bits.
 ************************************************************************************************/

/************************************************************************************************
 * BAOB_P_Reset_Status()	 This routine resets the status parameters 
 ************************************************************************************************/
BERR_Code BAOB_P_Reset_Status(BAOB_3x7x_Handle h)
{
	uint32_t ReadReg;

	BERR_Code retCode = BERR_SUCCESS;
	BDBG_MSG(("BAOB_P_Reset_Status() "));

	h->pStatus->Auto = BAOB_Status_eDisable;
	h->pStatus->AI = BAOB_Status_eDisable;
	h->pStatus->BYP = BAOB_Status_eDisable;
	h->pStatus->BPS = BAOB_Status_BPS_eDVS_178;
	h->pStatus->AA = BAOB_Status_BPS_eDVS178;
	h->pStatus->PLBW = BAOB_Status_PLBW_eLow; 
   /*	h->pStatus->BERSRC = BAOB_Status_BERT_Source_eI; */
	h->pStatus->SI = BAOB_Status_eNotInverted;
	h->pStatus->BERSYN = BAOB_Status_eUnlock;
	h->pStatus->Reack_Count = 0;
	h->pStatus->SNR = 0;
	h->pStatus->Sample_Rate = 0; 
	h->pStatus->Symbol_Rate = 0; 
	h->pStatus->Frequency_Error = 0;
	h->pStatus->Symbol_Error = 0;			  
	h->pStatus->BERT = 0;             
	h->pStatus->FEC_Corr_RS_Blocks = 0;  
	h->pStatus->FEC_UCorr_RS_Blocks = 0; 
	h->pStatus->ATM_Total_Count = 0;      
	h->pStatus->ATM_Loss_Count = 0;       

	/*clear RS correction counter--*/
  	BREG_Write32(h->hRegister, BCHP_OOB_STRSB, 0x00000000);

	/*Clear BERT PRBS counter by reading it*/
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDBERTC); 
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDATMC);
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDATML);
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDHEC);


return retCode;
}


/************************************************************************************************
 * BAOB_P_GetLock_Status()	 This routine return the lock status 
 ************************************************************************************************/
BERR_Code BAOB_P_GetLock_Status(BAOB_3x7x_Handle h)
{
	uint32_t ReadReg;

	/*Receiver Lock*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	SNR_IS);
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	SNR_IS);
	if (ReadReg == 1)
	{
		h->pStatus->RLK = BAOB_Status_eLock;
	}
	else
	{
		h->pStatus->RLK = BAOB_Status_eUnlock;
	}
	
	/*FEC Lock*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	FEC_IS);
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	FEC_IS);
	if (ReadReg == 1)
	{
		h->pStatus->FLK = BAOB_Status_eLock;
	}
	else
	{
		h->pStatus->FLK = BAOB_Status_eUnlock;
	}

	return BERR_SUCCESS;
}
/************************************************************************************************
 * BAOB_P_Get_Status()	 This routine gets the status parameters 
 ************************************************************************************************/
BERR_Code BAOB_P_Get_Status(BAOB_3x7x_Handle h)
{
	BERR_Code retCode = BERR_SUCCESS;

	uint32_t ReadReg;
#ifdef LEAP_BASED_CODE
	BAOB_P_AobCallbackData_t CallbackFrontend;
#endif
	/*BDBG_MSG(("BAOB_P_Get_Status() "));*/

	/*Auto Acquire: Get from BAOB_Acquire_Params_t structure since this is a software flag*/
	if (h->pAcqParam->BAOB_Acquire_Params.Auto == BAOB_Acquire_Params_eEnable)
	{
		h->pStatus->Auto = BAOB_Status_eEnable;
	}
	else
	{
		h->pStatus->Auto = BAOB_Status_eDisable;
	}

	/*Auto Invert Spectrum: Get from BAOB_Acquire_Params_t structure since this is a software flag*/
	if (h->pAcqParam->BAOB_Acquire_Params.AI == BAOB_Acquire_Params_eEnable)
	{
		h->pStatus->AI = BAOB_Status_eEnable;
	}
	else
	{
		h->pStatus->AI = BAOB_Status_eDisable;
	}

	/*FEC Bypass*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_CTRL5, FECBYP);
	if (ReadReg == 1)
	{
		h->pStatus->BYP = BAOB_Status_eEnable;
	}
	else
	{
		h->pStatus->BYP = BAOB_Status_eDisable;
	}

	/*BPS: Get from BAOB_Acquire_Params_t structure since this is a software flag*/
   	switch (h->pAcqParam->BAOB_Acquire_Params.BPS)
	{
		case BAOB_Acquire_Params_BPS_eDVS_178:
			h->pStatus->BPS = BAOB_Status_BPS_eDVS_178;
			break;
		case BAOB_Acquire_Params_BPS_eDVS_167_GradeA:
			h->pStatus->BPS = BAOB_Status_BPS_eDVS_167_GradeA;	
			break;
		case BAOB_Acquire_Params_BPS_eDVS_167_GradeB:
			h->pStatus->BPS = BAOB_Status_BPS_eDVS_167_GradeB;
			break;
		case BAOB_Acquire_Params_BPS_eBERT_TEST_MODE:
			h->pStatus->BPS = BAOB_Status_BPS_eBERT_TEST_MODE;
			break;
		default:
			h->pStatus->BPS = BAOB_Status_BPS_eDVS_178;
			BDBG_ERR(("UNKNOWN BAOB_Acquire_Params.BPS in BAOB_P_Get_Status()"));
	}
		/*FEC Bypass*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_LDFECH, OOB_LDFECH_0);
	if ((ReadReg & 0x4) == 0)
	{
		h->pStatus->AA = BAOB_Status_BPS_eDVS167;
	}
	else
	{
		h->pStatus->AA = BAOB_Status_BPS_eDVS178;
	}

	/*PLBW: Get from BAOB_Acquire_Params_t structure since this is a software flag*/
	switch (h->pAcqParam->BAOB_Acquire_Params.PLBW)
	{
		case BAOB_Acquire_Params_PLBW_eLow:
			h->pStatus->PLBW = BAOB_Status_PLBW_eLow;
			break;
		case BAOB_Acquire_Params_PLBW_eMed:
			h->pStatus->PLBW = BAOB_Status_PLBW_eMed; 
			break;
		case BAOB_Acquire_Params_PLBW_eHigh:
			h->pStatus->PLBW = BAOB_Status_PLBW_eHigh; 
			break;
		default:
			h->pStatus->PLBW = BAOB_Status_PLBW_eLow;
			BDBG_ERR(("UNKNOWN BAOB_Acquire_Params.BPS in BAOB_P_Get_Status()"));
	}

	/*Get BERT Source*/
	/*Need to add a shadow  hack since this register can not be read back*/
	/*BDBG_MSG(( "STATUS: CTRL3 %x, BERSRC bits = %x", h->pAcqParam->BAOB_Local_Params.CTRL3,(h->pAcqParam->BAOB_Local_Params.CTRL3>>28) & 0x3));*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_CTRL3,BERSRC);
	switch (ReadReg & 0x3)
	{
	case 0x0:
		h->pStatus->BERSRC = BAOB_Status_BERT_Source_eI;
		break;
	case 0x1:
		h->pStatus->BERSRC = BAOB_Status_BERT_Source_eQ; 
		break;
	case 0x2:
		h->pStatus->BERSRC = BAOB_Status_BERT_Source_eIQ; 
		break;
	case 0x3:
		h->pStatus->BERSRC = BAOB_Status_BERT_Source_eFEC; 
		break;
	default :
		h->pStatus->BERSRC = BAOB_Status_BERT_Source_eI;
		BDBG_ERR(("UNKNOWN BERT_Source in BAOB_P_Get_Status()"));
	}

	/*Receiver Lock*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	SNR_IS);
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	SNR_IS);
	if (ReadReg == 1)
	{
		h->pStatus->RLK = BAOB_Status_eLock;
	}
	else
	{
		h->pStatus->RLK = BAOB_Status_eUnlock;
	}
	
	/*FEC Lock*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	FEC_IS);
	ReadReg = BREG_ReadField(h->hRegister, OOB_IRQST,	FEC_IS);
	if (ReadReg == 1)
	{
		h->pStatus->FLK = BAOB_Status_eLock;
	}
	else
	{
		h->pStatus->FLK = BAOB_Status_eUnlock;
	}

	/*Spectral Inversion Lock*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_CTRL2, OFMT);
	if (h->pAcqParam->BAOB_Acquire_Params.IS == BAOB_Acquire_Params_eDisable) /*OFMT = 0 initially*/
	{
		h->pStatus->SI = (ReadReg == 0) ? BAOB_Status_eNotInverted : BAOB_Status_eInverted; 
	}
	else                                                                       /*OFMT = 1 initially*/
	{
		h->pStatus->SI = (ReadReg == 1) ? BAOB_Status_eNotInverted : BAOB_Status_eInverted; 
	}

	/*BERT Sync*/
	/*Bug in A0 chip prevents reading so set to unlock at all times*/
	ReadReg = BREG_ReadField(h->hRegister, OOB_CTRL3,	BERSYN);
	/*ReadReg = h->pAcqParam->BAOB_Local_Params.CTRL3 & 0x00080000;*/
	if (ReadReg == 1)
	{
		h->pStatus->BERSYN = BAOB_Status_eLock;
	}
	else
	{
		h->pStatus->BERSYN = BAOB_Status_eUnlock;
	}
  
	/*set in the baob_acquire()*/
	/*h->pStatus->Reack_Count = 0;	*/

	/***************************************/
	/*Determine SNR                        */
	/*BBS will divide by 256 to get dB     */
	/***************************************/
	/*SNR in dB is different for each QAM Mode*/
  /*QPSK  124.4714 dB - 20*log(ERRVAL) or if scaled by 256: 31865-5120*log(ERRVAL)*/
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDSNRE);
	ReadReg = ReadReg/256; 
	h->pStatus->SNR = 31865 - (BMTH_2560log10(ReadReg)<<1);
	
	/*Sample Rate*/

	h->pStatus->Sample_Rate = F_HS; 						 
   
	/*Symbol Rate*/
	h->pStatus->Symbol_Rate = BAOB_P_Get_SymbolRate(h);
    
	/*Frequency Error*/
	h->pStatus->Frequency_Error = BAOB_P_Get_CFL_FrequencyError(h); 

	/*Timing Error*/
	h->pStatus->Symbol_Error = BAOB_P_Get_TL_FrequencyError(h)-h->pStatus->Symbol_Rate;
	/*BDBG_MSG(("symbol err=%d",h->pStatus->Symbol_Error ));*/

	/*BERT Count*/
	ReadReg=BREG_Read32(h->hRegister, BCHP_OOB_LDBERTC);
	h->pStatus->BERT = h->pStatus->BERT + ReadReg;
 	
	/*RS Corrected Bits Count*/
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDRSB); 
	ReadReg = ReadReg/65536;
	h->pStatus->FEC_Corr_RS_Blocks = h->pStatus->FEC_Corr_RS_Blocks+ReadReg;   

 	/*RS Uncorrected Blocks Count*/
	h->pStatus->FEC_UCorr_RS_Blocks = h->pStatus->FEC_UCorr_RS_Blocks+BREG_Read32(h->hRegister, BCHP_OOB_LDUERC);   

	/*ATM Total Cell Count*/
	h->pStatus->ATM_Total_Count = h->pStatus->ATM_Total_Count+BREG_Read32(h->hRegister, BCHP_OOB_LDATMC);   

	/*ATM Cell Loss Count*/
	ReadReg = BREG_Read32(h->hRegister, BCHP_OOB_LDATML); 
	ReadReg = ReadReg/65536;
	h->pStatus->ATM_Loss_Count = h->pStatus->ATM_Loss_Count+ReadReg;
#ifdef LEAP_BASED_CODE
 	/*Channel Power*/
	/*Get front end status*/
	
	CallbackFrontend.hTunerChn = (h->pCallbackParam[BAOB_Callback_eTuner]);
	CallbackFrontend.Mode = BAOB_CallbackMode_eRequestMode;
	BKNI_EnterCriticalSection();
	(h->pCallback[BAOB_Callback_eTuner])(&CallbackFrontend);	
	BKNI_LeaveCriticalSection();

	ReadReg=0;

#if OB_MODE == OB_AD 	
	h->pStatus->FrontEndGain_db = (int32_t)(CallbackFrontend.PostADC_Gain_x256db);
#else
	/*Gain before the WB A/D*/
	ReadReg=CallbackFrontend.PreADC_Gain_x256db;
	/*AGCF in WFE Gain*/
	if ((CallbackFrontend.PreADC_Gain_x256db == -32768) || (CallbackFrontend.PostADC_Gain_x256db == -32768))   
	{
		h->pStatus->FrontEndGain_db = -32768;
	}
	else
	{
		h->pStatus->FrontEndGain_db = (int32_t)( ReadReg + CallbackFrontend.PostADC_Gain_x256db);
	} 
#endif 

	/*AGCF*/
   /*	BDBG_MSG(("OOB FrontendGain(dB) = %d",(int32_t)( ReadReg + CallbackFrontend.PostADC_Gain_x256db)));	  */

	/*DAGC Gain*/
	/*DAGC_LPINT.INTGTR is 32-bit 8.24 unsigned number*/
	/*Reset value is 0x00000000*/
	/*  db = 20*log10(DAGC_LPINT/2^24)*/
	/*  256*db = 5120*log10(AGCBVAL/2^16)-5120*log10(2^8)*/ 
	ReadReg = BREG_ReadField(h->hRegister,OOB_DAGC_LPINT, INTGTR);
	ReadReg = 2*BMTH_2560log10(ReadReg/POWER2_16) - LOG10_POWER2_8_X5120;
	h->pStatus->DAGC_Gain_db = (int32_t)ReadReg;  /*BBS will divide by 256 to get dB*/
	/*BDBG_MSG(("OOB DAGC(dB) = %d",h->pStatus->DAGC_Gain_db));*/

	/*EQ Gain (real part of the main tap)*/
	/*LDF1.LDF1_0 and LDF1_1 together is a 12-bit 4.8 signed number*/
	/*LDF1_0 are bits [31:24], LDF1_1 are bits [19:16]*/
	/*EQ_Gain db = 20*log10( (LDF1_0/2^20)+((LDF1_1/2^16) && 0x000f))/2^12)*/
  	/*EQ 256*db = 5120*log10((LDF1_0/2^20)+((LDF1_1/2^16) && 0x000f))-5120*log10(2^12)*/
  
	ReadReg = BREG_ReadField(h->hRegister, OOB_LDF1, OOB_LDF1_0);
	ReadReg = 2*BMTH_2560log10(BREG_ReadField(h->hRegister, OOB_LDF1, OOB_LDF1_1)+ReadReg*POWER2_4);
	ReadReg = (ReadReg) - LOG10_POWER2_8_X5120;
  	h->pStatus->EQ_Gain_db = (int32_t)ReadReg;  /*BBS will divide by 256 to get dB*/
	/*BDBG_MSG(("OOB EQ(dB) = %d",h->pStatus->EQ_Gain_db));*/

	/*check for unknown power level sent from front end, -32768*/
	if (CallbackFrontend.External_Gain_x256db == -32768)
	{
		h->pStatus->EstChannelPower_dbm = -32768;
	}
	else
	{

   	switch (h->pAcqParam->BAOB_Acquire_Params.Nyq)
	{
		case BAOB_Acquire_Params_Nyq_ehalf_raised:
 			 ReadReg = 0;
 			break;
		case BAOB_Acquire_Params_Nyq_ehalf_rtraised:
			ReadReg = 3*256;	
			break;
		case 	BAOB_Acquire_Params_Nyq_ethird_rtraised:
			ReadReg = 3*256;
			break;
		default:
			ReadReg = 0;
			BDBG_ERR(("UNKNOWN BAOB_Acquire_Params.Nyq in BAOB_P_Get_Status()"));
	}

#if OB_MODE==OB_AD 	
		h->pStatus->EstChannelPower_dbm = OB_GAIN_OFFSET + ReadReg - h->pStatus->EQ_Gain_db - h->pStatus->FrontEndGain_db - h->pStatus->DAGC_Gain_db - (int32_t)CallbackFrontend.External_Gain_x256db;
 	    /*BDBG_MSG(("OB_AD : OOB EstChannelPwr(dB) = %d %d",h->pStatus->EstChannelPower_dbm, OB_GAIN_OFFSET));*/
#else
		h->pStatus->EstChannelPower_dbm = OB_GAIN_OFFSET_WB + ReadReg - h->pStatus->EQ_Gain_db - h->pStatus->FrontEndGain_db - h->pStatus->DAGC_Gain_db - (int32_t)CallbackFrontend.External_Gain_x256db;
 		/*BDBG_MSG(("WB_AD : OOB EstChannelPwr(dB) = %d %d",h->pStatus->EstChannelPower_dbm, OB_GAIN_OFFSET_WB));*/
#endif
		ReadReg = (h->pStatus->EQ_Gain_db + h->pStatus->FrontEndGain_db + h->pStatus->DAGC_Gain_db + (int32_t)CallbackFrontend.External_Gain_x256db);
	}
#endif

   	return retCode;
}
