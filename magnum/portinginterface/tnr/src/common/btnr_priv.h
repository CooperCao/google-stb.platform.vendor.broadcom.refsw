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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BTNR_PRIV_H__
#define BTNR_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Set Rf Freq. function.

Description:
    This is pointer to 'Custom' part of tuner module for setting the Rf Freq.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_SetRfFreqFunc)(
    void *hDevImp,                      /* Device handle */
    uint32_t freq,                      /* Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* Requested tuner mode */
    );

/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Get Rf Freq. function.

Description:
    This is pointer to 'Custom' part of tuner module for getting the
    current Rf Freq. setting.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_GetRfFreqFunc)(
    void *hDevImp,                      /* Device handle */
    uint32_t *freq,                     /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    );

/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Close function.

Description:
    This is pointer to 'Custom' part of tuner module for closing tuner module.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_CloseFunc)(
    BTNR_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Get AGC register value.

Description:
    This is pointer to 'Custom' part of tuner module for getting the
    current AGC register value.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_GetAgcRegValFunc)(
    void *hDevImp,                      /* Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [output] Returns AGC register value */
    );

/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Set AGC register value.

Description:
    This is pointer to 'Custom' part of tuner module for setting the
    current AGC register value.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_SetAgcRegValFunc)(
    void *hDevImp,                      /* Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [input] Sets AGC register value */
    );

/***************************************************************************
Summary:
    This typedef defines the pointer to Tuner Get Info.

Description:
    This is pointer to tuner module for getting the tuner Info.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_GetInfoFunc)(
    void *hDevImp,                      /* Device handle */ 
     BTNR_TunerInfo *tnrInfo             /* [output] Tuner information */
    );

/***************************************************************************
Summary:
    This function gets the power-saver mode.

Description:
    This function returns the current power-saver mode.

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_GetPowerSaverFunc)(
    void *hDevImp,                          /* Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* Tuner power saver information */
    );

/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling/disabling the downstream tuner
    power-saver mode.  When the power-saver mode is enabled, the
    Qam In-Band Downstream tuner is shutdown. 


See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_SetPowerSaverFunc)(
    void *hDevImp,                          /* Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* Tuner power saver information */
    );

/***************************************************************************
Summary:
    This function gets the TNR settings

Description:
    This function gets the TNR settings

See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_GetSettingsFunc)(
    BTNR_Handle hDev,           /* Device handle */
    BTNR_Settings *settings     /* TNR settings. */
    );

/***************************************************************************
Summary:
    This function sets the TNR settings

Description:
    This function sets the TNR settings


See Also:
    BTNR_P_Handle

****************************************************************************/
typedef BERR_Code (*BTNR_SetSettingsFunc)(
    BTNR_Handle hDev,           /* Device handle */
    BTNR_Settings *settings     /* TNR settings. */
    );

    
/******************************************************************************
Summary:
   This function sends request for spectrum analyzer data to the LEAP.
Description:
  
See Also:
    BTNR_P_Handle
******************************************************************************/
typedef BERR_Code (*BTNR_RequestSpectrumAnalyzerDataFunc)(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumSettings *pSettings /* [in] spectrum settings */
    );

/******************************************************************************
Summary:
   This function gets spectrum analyzer data from the LEAP.
Description:
  
See Also:
    BTNR_P_Handle
******************************************************************************/    
typedef BERR_Code (*BTNR_GetSpectrumAnalyzerDataFunc)(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    );

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BTNR_InstallCallbackFunc)(
    BTNR_Handle hDev,     /* [in] Device handle */
    BTNR_Callback callbackType, /* [in] Type of callback */
    BTNR_CallbackFunc pCallback_isr, /* [in] Function Ptr to callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    );
    
/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the core driver version 
    information. It return the majorVersion and minorVersion of the core
    driver.
Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BTNR_GetVersionInfoFunc)(
    BTNR_Handle hDev,               /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    );
 
/***************************************************************************
Summary:
    This function tunes IF DAC to the requested frequency.

Description:
    This function is responsible for tuning the IF DAC to the requested 
    frequency.
    
Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BTNR_TuneIfDacFunc)(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_IfDacSettings *pSettings  /* [in] IF DAC Settings */
    );

/***************************************************************************
Summary:
    This function resets the status of the IF DAC.

Description:
    This function is responsible for resetting If DAC status.

Returns:
    TODO:

See Also:

****************************************************************************/    
typedef BERR_Code (*BTNR_ResetIfDacStatusFunc)(
    BTNR_Handle hDev        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for requesting the status to be calculated 
    asynchronously for an IF DAC channel.

Returns:
    TODO:

See Also:

****************************************************************************/    
typedef BERR_Code (*BTNR_RequestIfDacStatusFunc)(
    BTNR_Handle hDev        /* [in] Device handle */
    );
    
/***************************************************************************
Summary:
    This function gets the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for asynchronously getting the complete status
    of the IF DAC.

Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BTNR_GetIfDacStatusFunc)(
    BTNR_Handle hDev,               /* [in] Device handle */
    BTNR_IfDacStatus *pStatus  /* [in] IF DAC Status */
    );
    
/***************************************************************************
Summary:
    The private handle for Tuner module

Description:
    A private handle for BTNR device, not exposed any modules outside of
    BTNR.

See Also:

****************************************************************************/
typedef struct BTNR_P_Handle
{
    void *hDevImpl;                         /* Device Implementation handle */
    uint32_t magicId;                       /* Used to check if structure is corrupt */
    BTNR_SetRfFreqFunc pSetRfFreq;          /* ptr to tune tuner function */
    BTNR_GetRfFreqFunc pGetRfFreq;          /* ptr to get tuned frequency function */
    BTNR_GetAgcRegValFunc pGetAgcRegVal;    /* ptr to get AGC register value */
    BTNR_GetInfoFunc pGetInfo;              /* ptr to get Tuner information */
    BTNR_CloseFunc pClose;                  /* ptr to close tuner module function */
    BTNR_GetPowerSaverFunc pGetPowerSaver;  /* ptr to Get tuner power saver module function */
    BTNR_SetPowerSaverFunc pSetPowerSaver;  /* ptr to set tuner power saver module function */
    BTNR_SetAgcRegValFunc pSetAgcRegVal;    /* ptr to set AGC register value */
    BTNR_GetSettingsFunc pGetSettings;  /* ptr to Get tuner power saver module function */
    BTNR_SetSettingsFunc pSetSettings;  /* ptr to set tuner power saver module function */    
    BTNR_RequestSpectrumAnalyzerDataFunc pRequestSpectrumAnalyzerData;  /* ptr to request spectrum analyzer data function */    
    BTNR_GetSpectrumAnalyzerDataFunc pGetSpectrumAnalyzerData;  /* ptr to get spectrum analyzer data function */    
    BTNR_InstallCallbackFunc pInstallCallback;  /* ptr to Install Callback function */    
    BTNR_GetVersionInfoFunc pVersionInfo;  /* ptr to Version Info function */  
    BTNR_TuneIfDacFunc pTuneIfDac;  /* ptr to Tune IfDac function */ 
    BTNR_ResetIfDacStatusFunc pResetIfDacStatus;    /* ptr to reset IfDac function */
    BTNR_RequestIfDacStatusFunc pRequestIfDacStatus;    /* ptr to request IfDac function */
    BTNR_GetIfDacStatusFunc pGetIfDacStatus;    /* ptr to get IfDac function */
} BTNR_P_Handle;


#ifdef __cplusplus
}
#endif
 
#endif



