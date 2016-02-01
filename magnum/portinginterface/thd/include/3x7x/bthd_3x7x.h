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

#ifndef _BTHD_3x7x_H__
#define _BTHD_3x7x_H__           

#include "bthd.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
	Callback structure to TNR from THD
 ***************************************************************************/

typedef enum BTHD_CallbackMode{	
	BTHD_CallbackMode_eSetMode      = 0,
	BTHD_CallbackMode_eRequestMode  = 1,
	BTHD_CallbackMode_eEnablePower  = 2,	
	BTHD_CallbackMode_eDisablePower = 3,
	BTHD_CallbackMode_eSmartTune    = 4,	    
	BTHD_CallbackMode_eLast
}BTHD_CallbackMode;

typedef struct BTHD_P_ThdCallbackData_s{	
	void*					hTunerChn;	
	uint16_t				Mode;				/*Callback is to send data on eSetMode, Callback is to get data on eRequestMode*/	
	uint32_t				Symbol_Rate;            /*Upper Symbol_Rate currently being scanned on eSetMode: return last value sent on eRequestMode*/
	int32_t		            Freq_Offset;					  /*Front end offset to use on eSetMode: return last value sent on eRequestMode*/
	uint32_t		        RF_Freq;						    /*RF frequency of the tuner on eRequestMode: set to 0 if unknown*/
	int32_t		            Total_Mix_After_ADC;    /*Sum of mixer frequencies after ADC on eRequestMode*/
	int16_t		            PreADC_Gain_x256db ;    /*Gain in db*256 before ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            PostADC_Gain_x256db;    /*Gain in db*256 after ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            External_Gain_x256db;   /*Gain in db*256 external to chip (like external LNA) on eRequestMode: set to 0x8000 if unknown*/
	uint16_t		          SmartTune;              /*SmartTune mode*/
}BTHD_P_ThdCallbackData_t;

/***************************************************************************
 * Summary:
 *   This function returns the default settings for 7550 THD module.
 *
 * Description:
 *   This function is responsible for returns the default setting for 
 *   7550 THD module. The returning default setting should be used when
 *   opening the device.
 *
 * Returns:
 *   TODO:
 *  
 * See Also:
 *   None.
 *   
 ***************************************************************************/
BERR_Code BTHD_3x7x_GetDefaultSettings(BTHD_Settings *);
  
/***************************************************************************
 * Summary:
 *   This function returns the default inbandParams for 7550 THD module.
 *
 * Description:
 *   This function is responsible for returns the default setting for 
 *   7550 THD module. The returning default setting should be used when
 *   opening the device.
 *
 * Returns:
 *   TODO:
 *   
 * See Also:
 *   None.
 *     
 ***************************************************************************/
BERR_Code BTHD_3x7x_GetDefaultInbandParams(BTHD_InbandParams *);
  

#ifdef __cplusplus
}
#endif

#endif /* _BTHD_3x7x_H__ */

