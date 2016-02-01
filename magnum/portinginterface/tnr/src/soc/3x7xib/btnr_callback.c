/*************************************************************************
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
#ifdef LEAP_BASED_CODE
#include "btnr_api.h"
#else
#include "bmem.h"
#include "btnr_3x7x.h"
#include "bdbg.h"
#include "btnr_priv.h"
#include "btnr_3x7xib_priv.h"
BDBG_MODULE(btnr_callback);
#endif
#include "btnr_tune.h"
#include "btnr_init.h"

/********************************************************************************************/
/*Function to program the RF offset for the ADS channel scan                                */
/********************************************************************************************/
BERR_Code BTNR_P_Set_RF_Offset(BTNR_3x7x_ChnHandle hTnr, int32_t RF_Offset, uint32_t Symbol_Rate)	
{
	BERR_Code retCode = BERR_SUCCESS;

	if (hTnr->pTunerStatus->PowerStatus != BTNR_ePower_On)
  {
		BDBG_ERR(("BTNR_P_Set_RF_Offset: power is still off  "));
		return BERR_NOT_INITIALIZED;
  }

	/*These two parameters come from the BADS_Acquire() to program the front end for 1-7 MBaud and freq offsets*/
	/*RF_Offset is an int32_t*/
	/*Symbol_Rate is an uint32_t*/
	hTnr->pTunerParams->BTNR_Local_Params.RF_Offset   = RF_Offset;    
	  
	BTNR_P_Program_Back_DDFS(hTnr);
	if (Symbol_Rate != 1)
	{
	hTnr->pTunerParams->BTNR_Local_Params.Symbol_Rate = Symbol_Rate;
	BTNR_P_Program_VID_CIC_HB_SAW(hTnr);
	}
	return retCode;
}

/********************************************************************************************/
/*Function to get the RF status for the ADS channel scan                                    */
/********************************************************************************************/
BERR_Code BTNR_P_Get_RF_Status(BTNR_3x7x_ChnHandle hTnr)	
{	
	BERR_Code retCode = BERR_SUCCESS;

   if (hTnr->pTunerStatus->PowerStatus != BTNR_ePower_On)
   {
		BDBG_ERR(("BTNR_P_Get_RF_Status: power is still off  "));
		return BERR_NOT_INITIALIZED;
   }
	/*get LNA status registers*/
#ifdef LEAP_BASED_CODE
#if EXT_LNA_ENABLE
	BLNA_P_Read_Registers(hTnr->hTnr->pLna);
#endif
#endif

	/*These three parameters come from the BTNR_P_TunerStatus()*/
	/*hTnr->pTunerStatus->Tuner_RF_Freq is an uint32_t*/
	/*hTnr->pTunerStatus->Tuner_PreADC_Gain_x256db is an int16_t*/
	/*hTnr->pTunerStatus->External_Gain_x256db is an int16_t*/
	BTNR_P_TunerStatus(hTnr);
	
	/*These four parameters come from the BTNR_P_InitStatus()*/
	/*hTnr->pTunerParams->BTNR_Local_Params.RF_Offset is an int32_t */
	/*hTnr->pTunerParams->BTNR_Local_Params.Symbol_Rate is an uint32_t*/
	/*hTnr->pTunerParams->BTNR_Local_Params.Total_Mix_After_ADC is an int32_t*/
	/*hTnr->pTunerParams->BTNR_Local_Params.PostADC_Gain_x256db is an int16_t*/
	BTNR_P_InitStatus(hTnr);

	return retCode;
}

/********************************************************************************************/
/*Function to set the Tuner power mode                                  */
/********************************************************************************************/
BERR_Code BTNR_P_LNA_AGC_Power(BTNR_3x7x_ChnHandle hTnr, uint16_t Mode)
{
   
   if (hTnr->pTunerStatus->PowerStatus != BTNR_ePower_On)
   {
		BDBG_ERR(("BTNR_P_LNA_AGC_Power: power is still off  "));
		return BERR_NOT_INITIALIZED;
   }
	
   BSTD_UNUSED(Mode);
	/*BDBG_MSG(("BTNR_P_LNA_AGC_Power and the mode is %d", Mode)); */
	
	return BERR_SUCCESS;


}
