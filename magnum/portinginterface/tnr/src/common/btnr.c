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
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"

BDBG_MODULE(btnr);


#define DEV_MAGIC_ID            ((BERR_TNR_ID<<16) | 0xFACE)

#define CHK_RETCODE( rc, func )     \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)


/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/



/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/



/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/



/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/



/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_SetTunerRfFreq(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t freq,                      /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_SetTunerRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pSetRfFreq != NULL )
    {
        CHK_RETCODE( retCode, hDev->pSetRfFreq( hDev->hDevImpl, freq, tunerMode ) );
    }

done:
    BDBG_LEAVE(BTNR_SetTunerRfFreq);
    return( retCode );
}

BERR_Code BTNR_GetTunerRfFreq(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t *freq,                     /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_GetTunerRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->pGetRfFreq != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetRfFreq( hDev->hDevImpl, freq, tunerMode ) );
    }
    else
    {
        *freq = 0;
        *tunerMode = BTNR_TunerMode_eDigital;
    }

done:
    BDBG_LEAVE(BTNR_GetTunerRfFreq);
    return( retCode );
}

BERR_Code BTNR_GetInfo(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BKNI_Memset( tnrInfo, 0x00, sizeof( BTNR_TunerInfo ) );
    if( hDev->pGetInfo != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetInfo( hDev->hDevImpl, tnrInfo ) );
    }

done:
    BDBG_LEAVE(BTNR_GetInfo);
    return( retCode );
}

BERR_Code BTNR_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->pClose != NULL )
    {
        CHK_RETCODE( retCode, hDev->pClose( hDev ) );
    }

done:
    BDBG_LEAVE(BTNR_Close);
    return( retCode );
}

BERR_Code BTNR_P_GetTunerAgcRegVal(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_P_GetTunerAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->pGetAgcRegVal != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetAgcRegVal( hDev->hDevImpl, regOffset, agcVal ) );
    }
    else
    {
        *agcVal = 0;
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_P_GetTunerAgcRegVal);
    return( retCode );
}

BERR_Code BTNR_SetTunerAgcRegVal(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTNR_SetTunerAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->pSetAgcRegVal != NULL )
    {
        CHK_RETCODE( retCode, hDev->pSetAgcRegVal( hDev->hDevImpl, regOffset, agcVal ) );
    }
    else
    {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_P_GetTunerAgcRegVal);
    return( retCode );
}

BERR_Code BTNR_GetPowerSaver(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [out] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pGetPowerSaver != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetPowerSaver( hDev->hDevImpl, pwrSettings ) );
    }
    else {
        retCode = BERR_INVALID_PARAMETER;
    }

done:
    BDBG_LEAVE(BTNR_GetPowerSaver);
    return( retCode );
}

BERR_Code BTNR_SetPowerSaver(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_SetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pSetPowerSaver != NULL )
    {
        CHK_RETCODE( retCode, hDev->pSetPowerSaver( hDev->hDevImpl, pwrSettings ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_SetPowerSaver);
    return( retCode );
}

BERR_Code BTNR_GetSettings(
    BTNR_Handle hDev,           /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pGetSettings != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetSettings( hDev->hDevImpl, settings ) );
    }
    else {
        retCode = BERR_INVALID_PARAMETER;
    }

done:
    BDBG_LEAVE(BTNR_GetSettings);
    return( retCode );
}

BERR_Code BTNR_SetSettings(
    BTNR_Handle hDev,           /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_SetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pSetSettings != NULL )
    {
        CHK_RETCODE( retCode, hDev->pSetSettings( hDev->hDevImpl, settings ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_SetSettings);
    return( retCode );
}

BERR_Code BTNR_RequestSpectrumAnalyzerData(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumSettings *pSettings /* [in] spectrum settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_RequestSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pRequestSpectrumAnalyzerData != NULL )
    {
        CHK_RETCODE( retCode, hDev->pRequestSpectrumAnalyzerData( hDev->hDevImpl, pSettings ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_RequestSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BTNR_GetSpectrumAnalyzerData(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pGetSpectrumAnalyzerData != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetSpectrumAnalyzerData( hDev->hDevImpl, pSpectrumData ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_GetSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BTNR_InstallCallback(
    BTNR_Handle hDev,     /* [in] Device handle */
    BTNR_Callback callbackType, /* [in] Type of callback */
    BTNR_CallbackFunc pCallback_isr, /* [in] Function Ptr to callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_InstallCallback);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pInstallCallback != NULL )
    {
        CHK_RETCODE( retCode, hDev->pInstallCallback( hDev->hDevImpl, callbackType, pCallback_isr, pParam ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_InstallCallback);
    return( retCode );
}

BERR_Code BTNR_GetVersionInfo(
    BTNR_Handle hDev,              /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pVersionInfo != NULL )
    {
        CHK_RETCODE( retCode, hDev->pVersionInfo( hDev->hDevImpl, pVersionInfo ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_GetVersionInfo);
    return( retCode );
}    
  
BERR_Code BTNR_TuneIfDac(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_IfDacSettings *pSettings       /* [in] IF DAC Settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_TuneIfDac);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
      
    if( hDev->pTuneIfDac != NULL )
    {    
        CHK_RETCODE( retCode, hDev->pTuneIfDac( hDev->hDevImpl, pSettings ) );
    }
    else {     
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_TuneIfDac);
    return( retCode );
}   

BERR_Code BTNR_ResetIfDacStatus(
    BTNR_Handle hDev        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_ResetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pResetIfDacStatus != NULL )
    {
        CHK_RETCODE( retCode, hDev->pResetIfDacStatus( hDev->hDevImpl ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_ResetIfDacStatus);
    return( retCode );
}   

BERR_Code BTNR_RequestIfDacStatus(
    BTNR_Handle hDev        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pRequestIfDacStatus != NULL )
    {
        CHK_RETCODE( retCode, hDev->pRequestIfDacStatus( hDev->hDevImpl ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_RequestIfDacStatus);
    return( retCode );
}   

BERR_Code BTNR_GetIfDacStatus(
    BTNR_Handle hDev,               /* [in] Device handle */
    BTNR_IfDacStatus *pStatus       /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_GetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if( hDev->pGetIfDacStatus != NULL )
    {
        CHK_RETCODE( retCode, hDev->pGetIfDacStatus( hDev->hDevImpl, pStatus ) );
    }
    else {
        retCode = BERR_NOT_SUPPORTED;
    }

done:
    BDBG_LEAVE(BTNR_GetIfDacStatus);
    return( retCode );
}

