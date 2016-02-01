/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BTNR_3x7x_H__
#define BTNR_3x7x_H__

#include "bchp.h"
#include "btnr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned short i2cAddr;             /* 7bit I2C address of Bcm7550 */
	BTMR_Handle hTmr;
	BMEM_Heap_Handle hHeap;
} BTNR_3x7x_Settings;

/***************************************************************************
Summary:
    This function returns the default settings for Bcm3x7x Tuner module.

Description:
    This function is responsible for returns the default setting for 
    BTNR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTNR_3x7x_Open()

****************************************************************************/
BERR_Code BTNR_3x7x_GetDefaultSettings(
    BTNR_3x7x_Settings *pDefSettings  /* [out] Returns default setting */
    );


/***************************************************************************
Summary:
    This function opens Bcm3x7x Tuner module.

Description:
    This function is responsible for opening Bcm3x7x BTNR module. When BTNR is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTNR_Close(), BTNR_3x7x_GetDefaultSettings()

****************************************************************************/

BERR_Code BTNR_3x7x_Open(BTNR_Handle *phDev, 
		BTNR_3x7x_Settings *pSettings, 
		BREG_Handle hRegister);


/***************************************************************************
Summary:
    Function called once the event is sent to upper layer

	BTNR_3x7x_ProcessInterruptEvent
****************************************************************************/
BERR_Code BTNR_3x7x_ProcessInterruptEvent(BTNR_Handle hDev);

/***************************************************************************
Summary:
    Function called by upper to get the inetrrupt handle

	BTNR_3x7x_GetInterruptEventHandle
****************************************************************************/
BERR_Code BTNR_3x7x_GetInterruptEventHandle(BTNR_Handle h, BKNI_EventHandle* hEvent);

/***************************************************************************
Summary: callback for demod to tuner through above layer
	BTNR_3x7x_Set_RF_Offset
****************************************************************************/
BERR_Code BTNR_3x7x_Set_RF_Offset(BTNR_Handle hTnrDev, int32_t RF_Offset, uint32_t Symbol_Rate);


typedef struct
{
	uint32_t		        RF_Freq;				/*RF frequency of the tuner on eRequestMode: set to 0 if unknown*/
	int32_t		            Total_Mix_After_ADC;    /*Sum of mixer frequencies after ADC on eRequestMode*/
	int16_t		            PreADC_Gain_x256db ;    /*Gain in db*256 before ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            PostADC_Gain_x256db;    /*Gain in db*256 after ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            External_Gain_x256db;   /*Gain in db*256 external to chip (like external LNA) on eRequestMode: set to 0x8000 if unknown*/
	uint32_t				Symbol_Rate;
	int32_t					RF_Offset;
} BTNR_3x7x_RfStatus_t;


/******************************************************************************
  BTNR_3x7x_Get_RF_Status()
  callback from Demod to Tuner through above layer
 ******************************************************************************/
BERR_Code BTNR_3x7x_Get_RF_Status(BTNR_Handle hTnrDev, BTNR_3x7x_RfStatus_t *RfCallbackStatus);



/******************************************************************************
  BTNR_3x7x_Set_RF_LoopThrough()
 ******************************************************************************/
BERR_Code BTNR_3x7x_Set_RF_LoopThrough(BTNR_Handle hTnrDev, bool EnableRfLoopThrough);

/******************************************************************************
  BTNR_3x7x_Get_RF_LoopThrough()
 ******************************************************************************/
BERR_Code BTNR_3x7x_Get_RF_LoopThrough(BTNR_Handle hTnrDev, bool EnableRfLoopThrough);




/***************************************************************************
Summary:
    Structure for External Gain Settings

Description:
    Structure for External Gain Settings

See Also:
    BTNR_GetExternalGain(), BTNR_GetInternallGain()
    
****************************************************************************/
typedef struct BTNR_3x7x_ExternalGainSettings
{
    int16_t externalGainBypassable; /* External Gain bypassable in units of 1/100th of a dB */
    int16_t externalGainTotal; /* External Gain Total in units of 1/100th of a dB */
} BTNR_3x7x_ExternalGainSettings;

/***************************************************************************
Summary:
    Structure for Internal Gain Settings

Description:
    Structure for Internal Gain Settings

See Also:
    BTNR_GetInternalGain()
    
****************************************************************************/
typedef struct BTNR_3x7x_InternalGainSettings
{
    uint32_t    frequency;                  /* frequency */ 
    int16_t     internalGainLoopThrough;    /* Internal Gain to Loop Through in units of 1/100th of a dB */
    int16_t     internalGainDaisy;          /* Internal Gain to Daisy in units of 1/100th of a dB */
    bool        externalGainBypassed;       /* This flag indicates whether the external gain into the tuner is bypass-able or not */
} BTNR_3x7x_InternalGainSettings;

/***************************************************************************
Summary:
    Structure for External Gain Settings

Description:
    Structure for External Gain Settings

See Also:
    BTNR_GetInternalGain()

****************************************************************************/
typedef struct BTNR_3x7x_InternalGainInputParams
{
    uint32_t frequency;              /* frequency */ 
} BTNR_3x7x_InternalGainInputParams;

/******************************************************************************
  BTNR_3x7x_SetExternalGain()
Summary:
   This function sets the external gain settings to a demod chip
Description:
  
Returns:
   BERR_Code
******************************************************************************/   

BERR_Code BTNR_3x7x_SetExternalGain(BTNR_Handle hTnrDev, const BTNR_3x7x_ExternalGainSettings *);

/******************************************************************************
  BTNR_3x7x_GetExternalGain()
  Summary:
   This function retreive the external gain settings to a demod chip
Description:
  
Returns:
   BERR_Code
******************************************************************************/  

BERR_Code BTNR_3x7x_GetExternalGain(BTNR_Handle hTnrDev, BTNR_3x7x_ExternalGainSettings *); 

/******************************************************************************
  BTNR_3x7x_GetInternalGain()
Summary:
   This function retrieves the internal gain settings of a demod chip
Description:
  
Returns:
   BERR_Code
******************************************************************************/    
BERR_Code BTNR_3x7x_GetInternalGain(BTNR_Handle hTnrDev, const BTNR_3x7x_InternalGainInputParams *, BTNR_3x7x_InternalGainSettings *);


/***************************************************************************
Summary:
    Enumeration for TNR Input Mode

Description:
    This field specifies the Input mode for TNR.

See Also:

****************************************************************************/
typedef enum BTNR_3x7x_RfInputMode
{
  BTNR_3x7x_RfInputMode_eOff, /* Tuner is off. */
  BTNR_3x7x_RfInputMode_eExternalLna,  /* Tuner Rf input through UHF path. This Rf path does not use internal LNA. */
  BTNR_3x7x_RfInputMode_eInternalLna, /* Tuner Rf input through VHF path. This Rf path uses internal LNA. */
  BTNR_3x7x_RfInputMode_eStandardIf,  /* 44 MHz or 36 MHz */
  BTNR_3x7x_RfInputMode_eLowIf,  /*4 MHz to 5 MHz. */
  BTNR_3x7x_RfInputMode_eBaseband,
  BTNR_3x7x_RfInputMode_eLast
} BTNR_3x7x_RfInputMode;


BERR_Code BTNR_3x7x_GetRfInputMode(BTNR_Handle hTnrDev,  BTNR_3x7x_RfInputMode RfInputMode);
BERR_Code BTNR_3x7x_SetRfInputMode(BTNR_Handle hTnrDev,  BTNR_3x7x_RfInputMode RfInputMode);



/***************************************************************************
Summary:
    Enumeration for Tuner Application

Description:
    This field controls the target application.

See Also:

****************************************************************************/
typedef enum BTNR_3x7x_TunerApplication
{
  BTNR_3x7x_TunerApplication_eCable,
  BTNR_3x7x_TunerApplication_eTerrestrial,
  BTNR_3x7x_TunerApplication_eLast
} BTNR_3x7x_TunerApplication;


BERR_Code BTNR_3x7x_SetTnrApplication(BTNR_Handle hTnrDev,  BTNR_3x7x_TunerApplication TnrApp);
BERR_Code BTNR_3x7x_GetTnrApplication(BTNR_Handle hTnrDev,  BTNR_3x7x_TunerApplication TnrApp);

#ifdef __cplusplus
}
#endif
 
#endif



