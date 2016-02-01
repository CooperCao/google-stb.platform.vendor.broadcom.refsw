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
#include "btc2.h"
#include "btc2_priv.h"


BDBG_MODULE(btc2);



#define CHK_RETCODE( rc, func )             \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)


#define DEV_MAGIC_ID            ((BERR_TC2_ID<<16) | 0xFACE)




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
BERR_Code BTC2_Open(
    BTC2_Handle *pTc2,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BTC2_Settings *pDefSettings   /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_Open);
    
    if( pDefSettings->funcPtr.Open != NULL )
    {
        retCode = pDefSettings->funcPtr.Open(pTc2, hChip, hRegister, hInterrupt, pDefSettings );
    }
    else
    {
        *pTc2 = NULL;
        BDBG_WRN(("BTC2_Open: Funtion Ptr is NULL"));
    }

    return( retCode );
}

BERR_Code BTC2_Close(
    BTC2_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.Close != NULL )
    {
        retCode = hDev->settings.funcPtr.Close( hDev );
    }
    else
    {
        BDBG_WRN(("BTC2_Close: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_Close);
    return( retCode );
}

BERR_Code BTC2_Init(
    BTC2_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_Init);
    BDBG_ASSERT( hDev );
    
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.Init != NULL )
    {
        retCode = hDev->settings.funcPtr.Init( hDev );
    }
    else
    {
        BDBG_WRN(("BTC2_Init: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_Init);
    return( retCode );
}

BERR_Code BTC2_GetVersion(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.GetVersion != NULL )
    {
        retCode = hDev->settings.funcPtr.GetVersion( hDev, pVersion );
    }
    else
    {
        BDBG_WRN(("BTC2_GetVersion: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetVersion);
    return( retCode );
}

BERR_Code BTC2_GetVersionInfo(
    BTC2_Handle hDev,              /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.GetVersionInfo != NULL )
    {
        retCode = hDev->settings.funcPtr.GetVersionInfo( hDev, pVersionInfo );
    }
    else
    {
        BDBG_WRN(("BTC2_GetVersionInfo: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetVersionInfo);
    return( retCode );
}
    
    
BERR_Code BTC2_GetTotalChannels(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.GetTotalChannels != NULL )
    {
        retCode = hDev->settings.funcPtr.GetTotalChannels( hDev, totalChannels );
    }
    else
    {
        BDBG_WRN(("BTC2_GetTotalChannels: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetTotalChannels);
    return( retCode );
}

BERR_Code BTC2_GetChannelDefaultSettings(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BTC2_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if( hDev->settings.funcPtr.GetChannelDefaultSettings != NULL )
    {
        retCode = hDev->settings.funcPtr.GetChannelDefaultSettings( hDev, channelNo, pChnDefSettings );
    }
    else
    {
        BDBG_WRN(("BTC2_GetChannelDefaultSettings: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetChannelDefaultSettings);
    return( retCode );
}

BERR_Code BTC2_OpenChannel(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const BTC2_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_OpenChannel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *phChn = NULL;
    if( hDev->settings.funcPtr.OpenChannel != NULL )
    {
        retCode = hDev->settings.funcPtr.OpenChannel( hDev, phChn, channelNo, pChnDefSettings );
    }
    else
    {
        BDBG_WRN(("BTC2_OpenChannel: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_OpenChannel);
    return( retCode );
}

BERR_Code BTC2_CloseChannel(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    

    BDBG_ENTER(BTC2_CloseChannel);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.CloseChannel != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.CloseChannel( hChn );
    }
    else
    {
        BDBG_WRN(("BTC2_CloseChannel: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_CloseChannel);
    return( retCode );
}

BERR_Code BTC2_GetDevice(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetDevice);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = NULL;
    if( hChn->hTc2->settings.funcPtr.GetDevice != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetDevice( hChn, phDev );
    }
    else
    {
        BDBG_WRN(("BTC2_GetDevice: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetDevice);
    return( retCode );
}

BERR_Code BTC2_GetDefaultAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [out] default Inband Parameters */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetDefaultAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.GetDevice != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetDefaultAcquireParams( ibParams );
    }
    else
    {
        BDBG_WRN(("BTC2_GetDefaultAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetDefaultAcquireParams);
    return( retCode );
}

BERR_Code BTC2_SetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_SetAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.SetAcquireParams != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.SetAcquireParams( hChn, ibParams );
    }
    else
    {
        BDBG_WRN(("BTC2_SetAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_SetAcquireParams);
    return( retCode );
}


BERR_Code BTC2_GetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [out] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.GetAcquireParams != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetAcquireParams( hChn, ibParams );
    }
    else
    {
        BDBG_WRN(("BTC2_GetAcquireParams: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetAcquireParams);
    return( retCode );
}


BERR_Code BTC2_Acquire(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_InbandParam *ibParams           /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_Acquire);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.Acquire != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.Acquire( hChn, ibParams );
    }
    else
    {
        BDBG_WRN(("BTC2_Acquire: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_Acquire);
    return( retCode );
}

BERR_Code BTC2_GetStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.GetStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetStatus( hChn, pStatus);
    }
    else
    {
        BDBG_WRN(("BTC2_Status: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetStatus);
    return( retCode );
}

BERR_Code BTC2_RequestAsyncStatus(
    BTC2_ChannelHandle hChn            /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_RequestAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.RequestAsyncStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.RequestAsyncStatus( hChn);
    }
    else
    {
        BDBG_WRN(("BTC2_RequestAsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_RequestAsyncStatus);
    return( retCode );
}

BERR_Code BTC2_GetAsyncStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */   
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.GetAsyncStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetAsyncStatus( hChn, pStatus);
    }
    else
    {
        BDBG_WRN(("BTC2_AsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetAsyncStatus);
    return( retCode );
}

BERR_Code BTC2_GetLockStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.GetLockStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetLockStatus( hChn, pLockStatus);
    }
    else
    {
        BDBG_WRN(("BTC2_GetLockStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetLockStatus);
    return( retCode );
}

BERR_Code BTC2_GetSoftDecision(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_GetSoftDecision);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.GetSoftDecision != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetSoftDecision( hChn, nbrToGet, iVal, qVal, nbrGotten );
    }
    else
    {
        BDBG_WRN(("BTC2_GetSoftDecision: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetSoftDecision);
    return( retCode );
}

BERR_Code BTC2_EnablePowerSaver(
    BTC2_ChannelHandle hChn         /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.EnablePowerSaver != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.EnablePowerSaver( hChn );
    }
    else
    {
        BDBG_WRN(("BTC2_EnablePowerSaver: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_EnablePowerSaver);
    return( retCode );
}


BERR_Code BTC2_DisablePowerSaver(
    BTC2_ChannelHandle hChn         /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_DisablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.DisablePowerSaver != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.DisablePowerSaver( hChn );
    }
    else
    {
        BDBG_WRN(("BTC2_DisablePowerSaver: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_DisablePowerSaver);
    return( retCode );
}


BERR_Code BTC2_ProcessNotification(
    BTC2_ChannelHandle hChn,                /* [in] Device channel handle */
    unsigned int event                      /* [in] Event code and event data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_ProcessNotification);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    if( hChn->hTc2->settings.funcPtr.ProcessNotification != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.ProcessNotification( hChn, event);
    }
    else
    {
        BDBG_WRN(("BTC2_ProcessNotification: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_ProcessNotification);
    return( retCode );
}

BERR_Code BTC2_InstallCallback(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Callback callbackType,         /* [in] Type of callback */
    BTC2_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BTC2_InstallCallback);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.InstallCallback != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.InstallCallback( hChn, callbackType, pCallback, pParam );
    }
    else
    {
        BDBG_WRN(("BTC2_InstallCallback: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_InstallCallback);
    return( retCode );
}


BERR_Code BTC2_ResetStatus(
    BTC2_ChannelHandle hChn     /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_ResetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.ResetStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.ResetStatus( hChn);
    }
    else
    {
        BDBG_WRN(("BTC2_ResetStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetStatus);
    return( retCode );
}

BERR_Code BTC2_ReadSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [out] ptr to the data that we will read from the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to read? one to four*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_ReadSlave);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.ReadSlave != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.ReadSlave( hChn, chipAddr, subAddr, subAddrLen, data, dataLen );
    }
    else
    {
        BDBG_WRN(("BTC2_ReadSlave: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_ReadSlave);
    return( retCode );
}

BERR_Code BTC2_WriteSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [in] ptr to the data that we will write to the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to write? one to four*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_WriteSlave);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.WriteSlave != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.WriteSlave(  hChn, chipAddr, subAddr, subAddrLen, data, dataLen );
    }
    else
    {
        BDBG_WRN(("BTC2_WriteSlave: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_WriteSlave);
    return( retCode );
}

BERR_Code BTC2_RequestSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_RequestSelectiveAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.RequestSelectiveAsyncStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.RequestSelectiveAsyncStatus(  hChn, type );
    }
    else
    {
        BDBG_WRN(("BTC2_RequestSelectiveAsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_RequestSelectiveAsyncStatus);
    return( retCode );
}

BERR_Code BTC2_GetSelectiveAsyncStatusReadyType(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusReady *ready
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_GetSelectiveAsyncStatusReadyType);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.GetSelectiveAsyncStatusReadyType != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetSelectiveAsyncStatusReadyType(  hChn, ready );
    }
    else
    {
        BDBG_WRN(("BTC2_GetSelectiveAsyncStatusReadyType: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetSelectiveAsyncStatusReadyType);
    return( retCode );
}    

BERR_Code BTC2_GetSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type,
    BTC2_SelectiveStatus *pStatus   /* [out] */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTC2_GetSelectiveAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    
    if( hChn->hTc2->settings.funcPtr.GetSelectiveAsyncStatus != NULL )
    {
        retCode = hChn->hTc2->settings.funcPtr.GetSelectiveAsyncStatus(  hChn, type, pStatus );
    }
    else
    {
        BDBG_WRN(("BTC2_GetSelectiveAsyncStatus: Funtion Ptr is NULL"));
    }

    BDBG_LEAVE(BTC2_GetSelectiveAsyncStatus);
    return( retCode );
}   

