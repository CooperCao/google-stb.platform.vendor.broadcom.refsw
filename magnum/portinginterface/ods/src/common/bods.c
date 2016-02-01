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
#include "bods.h"
#include "bods_priv.h"

BDBG_MODULE(bods);
BDBG_OBJECT_ID(BODS);
BDBG_OBJECT_ID(BODS_CHANNEL);

#define CHK_RETCODE( rc, func )             \
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
BERR_Code BODS_Open(
    BODS_Handle *pOds,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BODS_Settings *pDefSettings   /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_Open);
    
    if( pDefSettings->funcPtr.Open != NULL )
    {
        retCode = pDefSettings->funcPtr.Open(pOds, hChip, hRegister, hInterrupt, pDefSettings );
    }
    else
    {
        *pOds = NULL;
        BDBG_WRN(("BODS_Open: Funtion Ptr is NULL"));
    }

    return( retCode );
}

BERR_Code BODS_Close(
    BODS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_Close);
    BDBG_OBJECT_ASSERT( hDev,BODS );
    
    if( hDev->settings.funcPtr.Close != NULL )
    {
        retCode = hDev->settings.funcPtr.Close( hDev );
    }
    else
    {
        BDBG_WRN(("BODS_Close: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_Close);
    return( retCode );
}

BERR_Code BODS_Init(
    BODS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_Init);
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.Init != NULL )
    {
        retCode = hDev->settings.funcPtr.Init( hDev );
    }
    else
    {
        BDBG_WRN(("BODS_Init: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_Init);
    return( retCode );
}

BERR_Code BODS_GetVersion(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.GetVersion != NULL )
    {
        retCode = hDev->settings.funcPtr.GetVersion( hDev, pVersion );
    }
    else
    {
        BDBG_WRN(("BODS_GetVersion: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetVersion);
    return( retCode );
}

BERR_Code BODS_GetVersionInfo(
    BODS_Handle hDev,                        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.GetVersionInfo != NULL )
    {
        retCode = hDev->settings.funcPtr.GetVersionInfo( hDev, pVersionInfo );
    }
    else
    {
        BDBG_WRN(("BODS_GetVersionInfo: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetVersionInfo);
    return( retCode );
}

BERR_Code BODS_GetTotalChannels(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Standard standard,             /* [in] Device handle */    
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.GetTotalChannels != NULL )
    {
        retCode = hDev->settings.funcPtr.GetTotalChannels( hDev, standard, totalChannels );
    }
    else
    {
        BDBG_WRN(("BODS_GetTotalChannels: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetTotalChannels);
    return( retCode );
}

BERR_Code BODS_GetChannelDefaultSettings(
    BODS_Handle hDev,                       /* [in] Device handle */
    BODS_Standard standard,                 /* [in] Device handle */
    BODS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.GetChannelDefaultSettings != NULL )
    {
        retCode = hDev->settings.funcPtr.GetChannelDefaultSettings( hDev, standard, pChnDefSettings );
    }
    else
    {
        BDBG_WRN(("BODS_GetChannelDefaultSettings: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetChannelDefaultSettings);
    return( retCode );
}

BERR_Code BODS_OpenChannel(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_ChannelHandle *phChn,          /* [out] Returns channel handle */
    const BODS_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_OpenChannel);
    BDBG_OBJECT_ASSERT( hDev,BODS );

    *phChn = NULL;
    if( hDev->settings.funcPtr.OpenChannel != NULL )
    {
        retCode = hDev->settings.funcPtr.OpenChannel( hDev, phChn, pChnDefSettings );
    }
    else
    {
        BDBG_WRN(("BODS_OpenChannel: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_OpenChannel);
    return( retCode );
}

BERR_Code BODS_CloseChannel(
    BODS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    

    BDBG_ENTER(BODS_CloseChannel);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.CloseChannel != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.CloseChannel( hChn );
    }
    else
    {
        BDBG_WRN(("BODS_CloseChannel: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_CloseChannel);
    return( retCode );
}

BERR_Code BODS_GetDevice(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetDevice);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    *phDev = NULL;
    if( hChn->hOds->settings.funcPtr.GetDevice != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetDevice( hChn, phDev );
    }
    else
    {
        BDBG_WRN(("BODS_GetDevice: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetDevice);
    return( retCode );
}


BERR_Code BODS_GetCapabilities(
    BODS_Handle hDev,                           /* [in] Device handle */
    BODS_FrontendCapabilities *pCapabilities    /* [out] */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetCapabilities);
    BDBG_OBJECT_ASSERT( hDev,BODS );

    if( hDev->settings.funcPtr.GetCapabilities != NULL )
    {
        retCode = hDev->settings.funcPtr.GetCapabilities( hDev, pCapabilities );
    }
    else
    {
        BDBG_WRN(("BODS_GetCapabilities: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetCapabilities);
    return( retCode );
}

BERR_Code BODS_GetLockStatus(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetLockStatus);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.GetLockStatus != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetLockStatus( hChn, pLockStatus);
    }
    else
    {
        BDBG_WRN(("BODS_GetLockStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetLockStatus);
    return( retCode );
}

BERR_Code BODS_GetSoftDecision(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetSoftDecision);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.GetSoftDecision != NULL )
    {
        *nbrGotten = 0;
        retCode = hChn->hOds->settings.funcPtr.GetSoftDecision( hChn, nbrToGet, iVal, qVal, nbrGotten );
    }
    else
    {
        BDBG_WRN(("BODS_GetSoftDecision: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetSoftDecision);
    return( retCode );
}

BERR_Code BODS_InstallCallback(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Callback callbackType,         /* [in] Type of callback */
    BODS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_InstallCallback);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.InstallCallback != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.InstallCallback( hChn, callbackType, pCallback, pParam );
    }
    else
    {
        BDBG_WRN(("BODS_InstallCallback: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_InstallCallback);
    return( retCode );
}

BERR_Code BODS_GetDefaultAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [out] default Acquire Parameters */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetDefaultAcquireParams);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.GetDevice != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetDefaultAcquireParams( hChn, acquireParams );
    }
    else
    {
        BDBG_WRN(("BODS_GetDefaultAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetDefaultAcquireParams);
    return( retCode );
}

BERR_Code BODS_SetAcquireParams(
    BODS_ChannelHandle hChn ,                   /* [in] Device channel handle */
    const BODS_AcquireParams *acquireParams     /* [in] Acquire Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_SetAcquireParams);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.SetAcquireParams != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.SetAcquireParams( hChn, acquireParams );
    }
    else
    {
        BDBG_WRN(("BODS_SetAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_SetAcquireParams);
    return( retCode );
}


BERR_Code BODS_GetAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [out] Acquire Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_GetAcquireParams);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.GetAcquireParams != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetAcquireParams( hChn, acquireParams );
    }
    else
    {
        BDBG_WRN(("BODS_GetAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetAcquireParams);
    return( retCode );
}


BERR_Code BODS_Acquire(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [in] Acquire Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_Acquire);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.Acquire != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.Acquire( hChn, acquireParams );
    }
    else
    {
        BDBG_WRN(("BODS_Acquire: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_Acquire);
    return( retCode );
}

BERR_Code BODS_EnablePowerSaver(
    BODS_ChannelHandle hChn,             /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings /* [in] Power saver settings */ 
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_EnablePowerSaver);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.EnablePowerSaver != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.EnablePowerSaver( hChn, pwrSettings );
    }
    else
    {
        BDBG_WRN(("BODS_EnablePowerSaver: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_EnablePowerSaver);
    return( retCode );
}


BERR_Code BODS_DisablePowerSaver(
    BODS_ChannelHandle hChn,             /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings /* [in] Power saver settings */ 
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BODS_DisablePowerSaver);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );

    if( hChn->hOds->settings.funcPtr.DisablePowerSaver != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.DisablePowerSaver( hChn, pwrSettings );
    }
    else
    {
        BDBG_WRN(("BODS_DisablePowerSaver: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_DisablePowerSaver);
    return( retCode );
}

BERR_Code BODS_ResetStatus(
    BODS_ChannelHandle hChn     /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BODS_ResetStatus);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.ResetStatus != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.ResetStatus( hChn);
    }
    else
    {
        BDBG_WRN(("BODS_ResetStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetStatus);
    return( retCode );
}

BERR_Code BODS_RequestSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_SelectiveAsyncStatusType type  /* Async Status Type */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BODS_RequestSelectiveAsyncStatus);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.RequestSelectiveAsyncStatus != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.RequestSelectiveAsyncStatus(  hChn, type );
    }
    else
    {
        BDBG_WRN(("BODS_RequestSelectiveAsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_RequestSelectiveAsyncStatus);
    return( retCode );
}

BERR_Code BODS_GetSelectiveAsyncStatusReadyType(
    BODS_ChannelHandle hChn,                    /* [in] Device channel handle */
    BODS_SelectiveAsyncStatusReadyType *ready   /* [out] Status Ready Type returned */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BODS_GetSelectiveAsyncStatusReadyType);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.GetSelectiveAsyncStatusReadyType != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetSelectiveAsyncStatusReadyType(  hChn, ready );
    }
    else
    {
        BDBG_WRN(("BODS_GetSelectiveAsyncStatusReadyType: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetSelectiveAsyncStatusReadyType);
    return( retCode );
}    

BERR_Code BODS_GetSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_SelectiveAsyncStatusType type, /* [in] Async Status Type */
    BODS_SelectiveStatus *pStatus       /* [out] */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BODS_GetSelectiveAsyncStatus);
	BDBG_OBJECT_ASSERT( hChn,BODS_CHANNEL );
    
    if( hChn->hOds->settings.funcPtr.GetSelectiveAsyncStatus != NULL )
    {
        retCode = hChn->hOds->settings.funcPtr.GetSelectiveAsyncStatus(  hChn, type, pStatus );
    }
    else
    {
        BDBG_WRN(("BODS_GetSelectiveAsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BODS_GetSelectiveAsyncStatus);
    return( retCode );
}   
