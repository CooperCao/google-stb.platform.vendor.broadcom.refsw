/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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
#include "baob.h"
#include "baob_priv.h"
#include "bhab.h"



BDBG_MODULE(baob);


#define sizeInLong(x)   (sizeof(x)/sizeof(uint32_t))
#define CHK_RETCODE( rc, func )             \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)


#define DEV_MAGIC_ID            ((BERR_AOB_ID<<16) | 0xFACE)


/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BAOB_Settings defDevSettings =
{
    BHAB_DevId_eAOB0,                   
    NULL,                       /* Hab handle, must be provided by application*/
    BAOB_SETTINGS_XTALFREQ,
    BAOB_SETTINGS_IFFREQ,
    false,
    BAOB_SpectrumMode_eNoInverted,
    true,                       /* Default is open drain mode. */
    true                        /* Default is serial data. */ 
};


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
BERR_Code BAOB_31xx_p_EventCallback_isr(
    void * pParam1, int param2
    )
{
    BAOB_Handle hDev = (BAOB_Handle) pParam1;
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_InterruptType event = (BHAB_InterruptType) param2;

    BDBG_ENTER(BAOB_31xx_p_EventCallback_isr);
    
    BDBG_ASSERT( hDev );
    switch (event) {
        case BHAB_Interrupt_eLockChange:
            if( hDev->pCallback[BAOB_Callback_eLockChange] != NULL )
            {
                (hDev->pCallback[BAOB_Callback_eLockChange])(hDev->pCallbackParam[BAOB_Callback_eLockChange] );
            }
        break;
        case BHAB_Interrupt_eOobAsyncStatusReady:
            {
                if( hDev->pCallback[BAOB_Callback_eAsyncStatusReady] != NULL )
                {
                    (hDev->pCallback[BAOB_Callback_eAsyncStatusReady])(hDev->pCallbackParam[BAOB_Callback_eAsyncStatusReady] );
                }
            }
            break;
        default:
            BDBG_WRN((" unknown event code from 31xx"));
        break;
    }

    BDBG_LEAVE(BAOB_31xx_p_EventCallback_isr);
    return( retCode );
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/

BERR_Code BAOB_GetDefaultSettings(
    BAOB_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);
    BDBG_ENTER(BAOB_GetDefaultSettings);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BAOB_GetDefaultSettings);
    return( retCode );
}



BERR_Code BAOB_Open(
    BAOB_Handle *pAob,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BAOB_Settings *pDefSettings   /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BAOB_Handle hDev;
    unsigned int event=0;
    uint8_t buf[5];

    BDBG_ENTER(BAOB_Open);

    /* Alloc memory from the system heap */
    hDev = (BAOB_Handle) BKNI_Malloc( sizeof( BAOB_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BAOB_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BAOB_P_Handle ) );

    hDev->magicId = DEV_MAGIC_ID;
    hDev->hChip = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->hHab = pDefSettings->hGeneric;
    hDev->devId = pDefSettings->devId;
    hDev->enableFEC = pDefSettings->enableFEC;
    hDev->spectrum = pDefSettings->spectrum;
    hDev->ifFreq = pDefSettings->ifFreq;
    
    CHK_RETCODE(retCode, BKNI_CreateMutex(&hDev->mutex));
    BHAB_InstallInterruptCallback( hDev->hHab,  hDev->devId, BAOB_31xx_p_EventCallback_isr , (void *) hDev, event);

    buf[0] = 0x12;
    buf[1] = 0;
    buf[2] = 0x1a;
    buf[3] = 0x04;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 4, buf, 8, true, true, 8));       

    buf[0] = 0x0b;

    if(pDefSettings->serialData) {
        buf[1] = (buf[4] & 0xC);
    }
    else {
        buf[1] = 0xC;       
    }

    if(pDefSettings->isOpenDrain ) {
        buf[1] |= 0x1;
    }
    
    buf[1] |= (buf[4] & 0x2);
    
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 1, true, true, 5));
    
    *pAob = hDev;

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *pAob = NULL;
    }

    BDBG_LEAVE(BAOB_Open);
    return( retCode );
}

BERR_Code BAOB_Close(
    BAOB_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BAOB_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BKNI_DestroyMutex(hDev->mutex);

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BAOB_Close);
    return( retCode );
}

BERR_Code BAOB_Acquire(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_AcquireParam *obParams         /* [in] Out of band acquire parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[4];
    uint8_t ifBuf[8];
    uint8_t buf2[2];

    BDBG_ENTER(BAOB_Acquire);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BKNI_AcquireMutex(hDev->mutex);
    hDev->isLock = false;
    BKNI_ReleaseMutex(hDev->mutex);

    buf[0] = 0x22;
    buf[1] = 0x10;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));

    BHAB_CHK_RETCODE(BHAB_EnableLockInterrupt(hDev->hHab, hDev->devId, false)); 

    ifBuf[0] = 0x13;
    ifBuf[1] = 0x00;
    ifBuf[2] = 0x0f;
    ifBuf[3] = 0x04;
    ifBuf[4] = hDev->ifFreq >> 24;
    ifBuf[5] = hDev->ifFreq >> 16;
    ifBuf[6] = hDev->ifFreq >> 8;
    ifBuf[7] = hDev->ifFreq;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, ifBuf, 8, ifBuf, 1, true, true, 8));

    buf[0] = 0x0c;
    buf[1] = ((obParams->autoAcquire) ? 0x20 : 0x00);
    buf[3] = 0x00;

    switch (obParams->modType) {
    case BAOB_ModulationType_eDvs167Qpsk:
        if(obParams->symbolRate == 772000) {
            buf[2] = 0x1a;
        }
        else if(obParams->symbolRate == 1544000){
            buf[2] = 0x2a;
        }
        break;
    case BAOB_ModulationType_eDvs178Qpsk:
        buf[2] = 0x2;
        break;
    case BAOB_ModulationType_ePod_Dvs167Qpsk:
        if(obParams->symbolRate == 772000) {
            buf[2] = 0x9a;
        }
        else if(obParams->symbolRate == 1544000){
            buf[2] = 0xaa;
        }
        break;
    case BAOB_ModulationType_ePod_Dvs178Qpsk:
        buf[2] = 0x82;
        break;
    default:
        BDBG_ERR(("Unsupported modulation type"));
    }

    if(hDev->spectrum == BAOB_SpectrumMode_eAuto) {
        buf[1] |= 0x50;
    }
    else {
        buf[1] |= 0x40;

        if(hDev->spectrum == BAOB_SpectrumMode_eInverted) {
            buf2[0] = 0x1c;
            buf2[1] = 0x1;
            CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf2, 2, buf2, 1, true, true, 2));
        }
        else {
            buf2[0] = 0x1c;
            buf2[1] = 0x0;
            CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf2, 2, buf2, 1, true, true, 2));
        }
    }

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 4, buf, 1, true, true, 4));
    BHAB_CHK_RETCODE(BHAB_EnableLockInterrupt(hDev->hHab, hDev->devId, true));  

done:
    BDBG_LEAVE(BAOB_Acquire);
    return( retCode );
}

BERR_Code BAOB_GetStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[64];

    BDBG_ENTER(BAOB_GetStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );
    
    buf[0] = 0x0e;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 1, buf, 64, true, true, 64));
    /* Populate the status structure here. */
    pStatus->modType = (buf[2] & 0x8) ? BAOB_ModulationType_eDvs167Qpsk : BAOB_ModulationType_eDvs178Qpsk;
    pStatus->isFecLock = buf[16] & 0x4;
    pStatus->isQamLock = buf[16] & 0x8;
    pStatus->ifFreq = (int32_t)((buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0a] << 8) + buf[0x0b]);
    pStatus->loFreq = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
    
    pStatus->symbolRate = (int32_t)((buf[0x0c] << 24) + (buf[0x0d] << 16) + (buf[0x0e] << 8) + buf[0x0f]);;
    pStatus->sysXtalFreq = 54000000;    
    pStatus->snrEstimate = (buf[0x11] << 8) + buf[0x12];
    pStatus->agcIntLevel = (((buf[0x1d] << 8) + buf[0x1e]) * 1000) / 65535;
    pStatus->agcExtLevel = (((buf[0x1b] << 8) + buf[0x1c]) * 1000) / 65535;
    pStatus->carrierFreqOffset = (int32_t)((buf[0x15] << 24) + (buf[0x16] << 16) + (buf[0x17] << 8) + buf[0x18]);
    pStatus->carrierPhaseOffset = 0;
    pStatus->uncorrectedCount = (int32_t)((buf[0x27] << 24) + (buf[0x28] << 16) + (buf[0x29] << 8) + buf[0x2a]);
    pStatus->correctedCount = (int32_t)((buf[0x23] << 24) + (buf[0x24] << 16) + (buf[0x25] << 8) + buf[0x26]);
    pStatus->berErrorCount = (int32_t)((buf[0x1f] << 24) + (buf[0x20] << 16) + (buf[0x21] << 8) + buf[0x22]);

    BDBG_ERR(("After Sync Status: modType = %d  symbolRate = %d loFreq = %d\n", pStatus->modType, pStatus->symbolRate, pStatus->loFreq));

done:
    BDBG_LEAVE(BAOB_GetStatus);
    return retCode; 
}

BERR_Code BAOB_GetAsyncStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[64];

    BDBG_ENTER(BAOB_GetAsyncStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BDBG_ERR(("Before Async Status: modType = %d  symbolRate = %d loFreq = %d\n", pStatus->modType, pStatus->symbolRate, pStatus->loFreq));

    buf[0] = 0x29;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 1, buf, 64, true, true, 64));
    /* Populate the status structure here. */
    pStatus->modType = (buf[2] & 0x8) ? BAOB_ModulationType_eDvs167Qpsk : BAOB_ModulationType_eDvs178Qpsk;
    pStatus->isFecLock = buf[16] & 0x4;
    pStatus->isQamLock = buf[16] & 0x8;
    pStatus->ifFreq = (int32_t)((buf[0x08] << 24) + (buf[0x09] << 16) + (buf[0x0a] << 8) + buf[0x0b]);
    pStatus->loFreq = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
    
    pStatus->symbolRate = (int32_t)((buf[0x0c] << 24) + (buf[0x0d] << 16) + (buf[0x0e] << 8) + buf[0x0f]);;
    pStatus->sysXtalFreq = 54000000;    
    pStatus->snrEstimate = (buf[0x11] << 8) + buf[0x12];
    pStatus->agcIntLevel = (((buf[0x1d] << 8) + buf[0x1e]) * 1000) / 65535;
    pStatus->agcExtLevel = (((buf[0x1b] << 8) + buf[0x1c]) * 1000) / 65535;
    pStatus->carrierFreqOffset = (int32_t)((buf[0x15] << 24) + (buf[0x16] << 16) + (buf[0x17] << 8) + buf[0x18]);
    pStatus->carrierPhaseOffset = 0;
    pStatus->uncorrectedCount = (int32_t)((buf[0x27] << 24) + (buf[0x28] << 16) + (buf[0x29] << 8) + buf[0x2a]);
    pStatus->correctedCount = (int32_t)((buf[0x23] << 24) + (buf[0x24] << 16) + (buf[0x25] << 8) + buf[0x26]);
    pStatus->berErrorCount = (int32_t)((buf[0x1f] << 24) + (buf[0x20] << 16) + (buf[0x21] << 8) + buf[0x22]);

    BDBG_ERR(("After Async Status: modType = %d  symbolRate = %d loFreq = %d\n", pStatus->modType, pStatus->symbolRate, pStatus->loFreq));

done:
    BDBG_LEAVE(BAOB_GetAsyncStatus);
    return retCode; 
}

BERR_Code BAOB_RequestAsyncStatus(
    BAOB_Handle hDev                   /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf;

    BDBG_ENTER(BAOB_RequestAsyncStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf = 0x28;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, &buf, 1, &buf, 1, true, true, 1));
    
done:
    BDBG_LEAVE(BAOB_RequestAsyncStatus);
    return retCode; 
}

BERR_Code BAOB_GetLockStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    bool *isLock                        /* [out] Returns lock status, 0=not lock, 1=locked */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BAOB_GetLockStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    *isLock = false;
    BKNI_AcquireMutex(hDev->mutex);
    *isLock = hDev->isLock;
    BKNI_ReleaseMutex(hDev->mutex);
    

    BDBG_LEAVE(BAOB_GetLockStatus);
    return( retCode );
}

BERR_Code BAOB_GetSoftDecision(
    BAOB_Handle hDev,                   /* [in] Device handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BERR_Code retCode = BERR_NOT_SUPPORTED;

    BDBG_ENTER(BAOB_GetSoftDecision);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BSTD_UNUSED(nbrToGet);
    BSTD_UNUSED(iVal);
    BSTD_UNUSED(qVal);
    BSTD_UNUSED(nbrGotten);
#if !(BDBG_DEBUG_BUILD)
    BSTD_UNUSED(hDev);
#endif
    BDBG_LEAVE(BAOB_GetSoftDecision);
    return( retCode );
}

BERR_Code BAOB_EnablePowerSaver(
    BAOB_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t   buf[2];
    
    BDBG_ENTER(BAOB_EnablePowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BDBG_ERR(("BADS_31xx_EnablePowerSaver called."));
    
    buf[0] = 0x21;
    buf[1] = 0x10;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
done:
    BDBG_LEAVE(BAOB_EnablePowerSaver);
    return( retCode );
}

BERR_Code BAOB_DisablePowerSaver(
    BAOB_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t   buf[2];
    
    BDBG_ENTER(BAOB_DisablePowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    BDBG_ERR(("BAOB_DisablePowerSaver called."));
    
    buf[0] = 0x22;
    buf[1] = 0x10;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
done:
    BDBG_LEAVE(BAOB_DisablePowerSaver);
    return( retCode );
}

BERR_Code BAOB_Get_Spectrum(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_SpectrumMode *spectrum         /* [out]current spectrum setting*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BAOB_Get_Spectrum);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    *spectrum = hDev->spectrum;

    BDBG_LEAVE(BAOB_Get_Spectrum);
    return( retCode );
}


BERR_Code BAOB_Set_Spectrum(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_SpectrumMode spectrum          /* [in] spectrum setting to be set*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t   buf[2];

    BDBG_ENTER(BAOB_Set_Spectrum);
    BSTD_UNUSED(spectrum);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    if(spectrum == BAOB_SpectrumMode_eInverted) {
        buf[0] = 0x1c;
        buf[1] = 0x1;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
    }
    else {
        buf[0] = 0x1c;
        buf[1] = 0x0;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
    }
    
    hDev->spectrum = spectrum;

done:   
    BDBG_LEAVE(BAOB_Set_Spectrum);
    return( retCode );

}


BERR_Code BAOB_ProcessNotification(
    BAOB_Handle hDev,                       /* [in] Device handle */
    unsigned int event                      /* [in] Event code and event data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    bool isFECLock, isQAMLock, isLock;
    unsigned int event_code;
    
    BDBG_ENTER(BAOB_ProcessNotification);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    event_code = event>>16;

    switch (event_code) {
        /*Set the event - Vish.*/
        case 1:
            isQAMLock = 1;
            isFECLock = 1;
            if (hDev->enableFEC){
                isLock = isQAMLock && isFECLock;
            } else {
                isLock = isQAMLock;
            }
            {
                if( hDev->pCallback[BAOB_Callback_eLockChange] != NULL )
                {
                    (hDev->pCallback[BAOB_Callback_eLockChange])(hDev->pCallbackParam[BAOB_Callback_eLockChange] );
                }
            }
            break;
        default:
            BDBG_WRN((" unknown event code from 31xx %x",event_code ));
            break;
    }

    BDBG_LEAVE(BAOB_ProcessNotification);
    return( retCode );
}

BERR_Code BAOB_InstallCallback(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Callback callbackType,         /* [in] Type of callback */
    BAOB_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BAOB_InstallCallback);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    switch( callbackType )
    {
        case BAOB_Callback_eLockChange:
            hDev->pCallback[callbackType] = pCallback;
            hDev->pCallbackParam[callbackType] = pParam;
            break;
        case BAOB_Callback_eAsyncStatusReady:
            hDev->pCallback[callbackType] = pCallback;
            hDev->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BAOB_InstallCallback);
    return( retCode );
}

BERR_Code BAOB_ResetStatus(
    BAOB_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t   buf[1];

    BDBG_ENTER(BAOB_Set_Spectrum);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf[0] = 0x1D;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 1, buf, 1, true, true, 1));

done:   
    BDBG_LEAVE(BAOB_Set_Spectrum);
    return( retCode );
}

