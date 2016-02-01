/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#include "baus.h"
#include "baus_priv.h"

BDBG_MODULE(baus);

#define CHK_RETCODE( rc, func )             \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)


#define DEV_MAGIC_ID            ((BERR_AUS_ID<<16) | 0xFACE)

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BAUS_Settings defDevSettings =
{
    BHAB_DevId_eAUS0,                   /* TODO: Is this required? */
    NULL,                               /* HAB handle, must be provided by application*/
    BAUS_SETTINGS_XTALFREQ,
    BAUS_SETTINGS_OPERATIONMODE,
    { 0, 0, 0, 1, 0, 1 }, /* External clk, Falling Edgeclk, Active low data, MSB transmitted first, CLAV based on ATM cell, s2p Enabled */
};

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BAUS_Open(
    BAUS_Handle *pAus,                      /* [out] Returns handle */
    BCHP_Handle hChip,                      /* [in] Chip handle */
    BREG_Handle hRegister,                  /* [in] Register handle */
    const BAUS_Settings *pDefSettings       /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BAUS_Handle hDev;
    uint8_t buf[2];

    BDBG_ENTER(BAUS_Open);

    BSTD_UNUSED( hChip );
    BSTD_UNUSED( hRegister );

    /* Alloc memory from the system heap */
    hDev = (BAUS_Handle) BKNI_Malloc( sizeof( BAUS_P_Handle ) );
    if( hDev == NULL )
    {
        *pAus = NULL;
        retCode = BERR_OUT_OF_SYSTEM_MEMORY;
        BDBG_ERR(("BAUS_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BAUS_P_Handle ) );

    hDev->magicId = DEV_MAGIC_ID;
    hDev->hChip = hChip;
    hDev->hRegister = hRegister;
    hDev->hHab = pDefSettings->hGeneric;
    hDev->devId = pDefSettings->devId;

    hDev->settings.xtalFreq = pDefSettings->xtalFreq;
    hDev->settings.operationMode = pDefSettings->operationMode;
    hDev->settings.isTransmitterEnabled = false;
    hDev->settings.premablePatternSz = 0;
    hDev->settings.rfFreq = 0;
    hDev->settings.powerLevel = 0;
    hDev->settings.symbolRate = 0;

    buf[0] = 0x1E;
    buf[1] = 0;

    if(pDefSettings->s2PSettings.clockSource)
        buf[1] |= 0x1;
    if(pDefSettings->s2PSettings.clockPolarity)
        buf[1] |= 0x2;
    if(pDefSettings->s2PSettings.dataValidPolarity)
        buf[1] |= 0x4;
    if(pDefSettings->s2PSettings.bitOrder)
        buf[1] |= 0x8;
    if(pDefSettings->s2PSettings.cellAvailable)
        buf[1] |= 0x10;
    if(pDefSettings->s2PSettings.enableS2P)
        buf[1] |= 0x20;    
    
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));

    *pAus = hDev;

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( (void *) hDev );
            *pAus = NULL;
        }
    }
    BDBG_LEAVE(BAUS_Open);
    return( retCode );
}

BERR_Code BAUS_Close(
    BAUS_Handle hDev                        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BAUS_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BAUS_Close);
    return( retCode );
}

BERR_Code BAUS_GetDefaultSettings(
    BAUS_Settings *pDefSettings,            /* [out] Returns default setting */
    BCHP_Handle hChip                       /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);

    BDBG_ENTER(BAUS_GetDefaultSettings);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BAUS_GetDefaultSettings);
    return( retCode );
}

BERR_Code BAUS_SetOperationMode(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_OperationMode operationMode    /* [in] Requested operation mode */    
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[2];

    BDBG_ENTER(BAUS_SetOperationMode);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf[0] = 0x16;
    buf[1] = 0x0;

    switch( operationMode )
    {
        case BAUS_OperationMode_eStarvue:
            buf[1] = 0x00;
            break;
        case BAUS_OperationMode_eDavic:
            buf[1] = 0x01;
            break;
        case BAUS_OperationMode_ePodStarvue:
            buf[1] = 0x02;
            break;
        case BAUS_OperationMode_ePodDavic:
            buf[1] = 0x03;
            break;
        case BAUS_OperationMode_eTestSingleCw:
            buf[1] = 0x04;
            break;
        case BAUS_OperationMode_eTestDoubleCw:
            buf[1] = 0x05;
            break;                      
        case BAUS_OperationMode_eTestPn23:
            buf[1] = 0x06;
            break;
        case BAUS_OperationMode_eDocsis:        
        default:
            retCode = BERR_INVALID_PARAMETER;
            return( retCode );
            break;
    }

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));

    hDev->settings.operationMode = operationMode;

done:

    BDBG_LEAVE(BAUS_SetOperationMode);
    return( retCode );
}

BERR_Code BAUS_GetOperationMode(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_OperationMode *operationMode   /* [out] Current operation mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BAUS_GetOperationMode);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *operationMode = hDev->settings.operationMode;

    BDBG_LEAVE(BAUS_GetOperationMode);
    return( retCode );
}

BERR_Code BAUS_SetSymbolRate(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long symbolRate                /* [in] Requested symbol rate, in baud */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[6];

    BDBG_ENTER(BAUS_SetSymbolRate);
    BDBG_MSG(("%s: symbolRate=%d", __FUNCTION__, symbolRate));
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf[0] = 0x18;
    buf[1] = 0x01;
    buf[2] = symbolRate >> 24;
    buf[3] = symbolRate >> 16;
    buf[4] = symbolRate >> 8;
    buf[5] = symbolRate;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 6, buf, 1, true, true, 6));

    hDev->settings.symbolRate = symbolRate;

done:
    BDBG_LEAVE(BAUS_SetSymbolRate);
    return( retCode );
}

BERR_Code BAUS_GetSymbolRate(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long *symbolRate               /* [out] Current symbol rate, in baud */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BAUS_GetSymbolRate);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *symbolRate = hDev->settings.symbolRate;

    BDBG_LEAVE(BAUS_GetSymbolRate);
    return( retCode );
}

BERR_Code BAUS_SetRfFreq(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long rfFreq                    /* [in] Requested RF frequency, in Hertz */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[6];

    BDBG_ENTER(BAUS_SetRfFreq);
    BDBG_MSG(("%s: rfFreq=%d", __FUNCTION__, rfFreq));
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf[0] = 0x18;
    buf[1] = 0x02;
    buf[2] = rfFreq >> 24;
    buf[3] = rfFreq >> 16;
    buf[4] = rfFreq >> 8;
    buf[5] = rfFreq;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 6, buf, 1, true, true, 6));

    hDev->settings.rfFreq = rfFreq;

done:
    BDBG_LEAVE(BAUS_SetRfFreq);
    return( retCode );
}

BERR_Code BAUS_GetRfFreq(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long *rfFreq                   /* [out] Current RF frequency, in Hertz */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BAUS_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *rfFreq = hDev->settings.rfFreq;

    BDBG_LEAVE(BAUS_GetRfFreq);
    return( retCode );
}

BERR_Code BAUS_SetPowerLevel(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned int powerLevel                 /* [in] Requested power level, in hundredth of dBmV */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[4];

    BDBG_ENTER(BAUS_SetPowerLevel);
    BDBG_MSG(("%s: powerLevel=%d", __FUNCTION__, powerLevel));
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );

    buf[0] = 0x18;
    buf[1] = 0x05;
    buf[2] = powerLevel >> 8;
    buf[3] = powerLevel;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 4, buf, 1, true, true, 4));

    hDev->settings.powerLevel = powerLevel;

done:
    BDBG_LEAVE(BAUS_SetPowerLevel);
    return( retCode );
}

BERR_Code BAUS_GetPowerLevel(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned int *powerLevel                /* [out] Current power level, in hundredth of dBmV */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ENTER(BAUS_GetPowerLevel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *powerLevel = hDev->settings.powerLevel;

    BDBG_LEAVE(BAUS_GetPowerLevel);
    return( retCode );
}


BERR_Code BAUS_GetStatus(
    BAUS_Handle hDev,                       /* [in] Device handle */
    BAUS_Status *pStatus                    /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BAUS_GetStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pStatus->operationMode = hDev->settings.operationMode;
    pStatus->symbolRate = hDev->settings.symbolRate;
    pStatus->rfFreq = hDev->settings.rfFreq;
    pStatus->powerLevel = hDev->settings.powerLevel;
    pStatus->isPowerSaverEnabled = hDev->settings.isTransmitterEnabled;
    pStatus->sysXtalFreq = hDev->settings.xtalFreq;
    BDBG_LEAVE(BAUS_GetStatus);

    return( retCode );
}


BERR_Code BAUS_TransmitStarvuePkt(
    BAUS_Handle hDev,                       /* [in] Device handle */
    uint8_t *ucXmitBuffer,                  /* [in] Buffer to transmit */
    unsigned int size                       /* [in] Size of Buffer (in bytes) */
    )
{
    BERR_Code retCode = BERR_NOT_SUPPORTED;

    BDBG_ENTER(BAUS_TransmitStarvuePkt);
    BSTD_UNUSED( size );
    BDBG_ASSERT(ucXmitBuffer);
    BDBG_ENTER(BAUS_GetStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* Not implemented as of now. */
    BDBG_LEAVE(BAUS_TransmitStarvuePkt);
    return( retCode );
}


BERR_Code BAUS_EnableTransmitter(
    BAUS_Handle hDev                        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_NOT_SUPPORTED;
    uint8_t   buf[2];

    BDBG_ENTER(BAUS_EnableTransmitter);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    buf[0] = 0x22;
    buf[1] = 0x60;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
done:
    BDBG_LEAVE(BAUS_EnableTransmitter);
    return( retCode );
}

BERR_Code BAUS_DisableTransmitter(
    BAUS_Handle hDev                        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_NOT_SUPPORTED;
    uint8_t   buf[2];

    BDBG_ENTER(BAUS_DisableTransmitter);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    buf[0] = 0x21;
    buf[1] = 0x60;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));
done:
    BDBG_LEAVE(BAUS_DisableTransmitter);
    return( retCode );
}

BERR_Code BAUS_SetTestModeParams(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_TestModeSettings *testModeSettings  /* [in] Requested operation mode settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[2];
    uint8_t buf2[6];

    BDBG_ENTER(BAUS_SetOperationMode);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( hDev->hHab );
    
    buf[1]=0;

    switch( testModeSettings->operationMode )
    {
        case BAUS_OperationMode_eTestSingleCw:
            buf[0] = 0x1f;
            buf[1] |= testModeSettings->bypassFilters;
            if(testModeSettings->nyquistAlphaValue)
                buf[1] |= 0x2;
            break;
        case BAUS_OperationMode_eTestDoubleCw:
            buf[0] = 0x1f;
            buf[1] |= testModeSettings->bypassFilters;
            if(testModeSettings->nyquistAlphaValue)
                buf[1] |= 0x2;

            buf2[0] = 0x18;
            buf2[1] = 0x03;
            buf2[2] = testModeSettings->rfFreq2 >> 24;
            buf2[3] = testModeSettings->rfFreq2 >> 16;
            buf2[4] = testModeSettings->rfFreq2 >> 8;
            buf2[5] = testModeSettings->rfFreq2;
            BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf2, 6, buf2, 1, true, true, 6));
            
            break;                      
        case BAUS_OperationMode_eTestPn23:
            buf[0] = 0x20;
            buf[1] = (testModeSettings->qamMode & 0x7);
            if(testModeSettings->nyquistAlphaValue)
                buf[1] |= 0x10;
            if(testModeSettings->spectrumInverted)
                buf[1] |= 0x20;
            break;
        case BAUS_OperationMode_eDocsis:
        case BAUS_OperationMode_eStarvue:
        case BAUS_OperationMode_eDavic:
        case BAUS_OperationMode_ePodStarvue:
        case BAUS_OperationMode_ePodDavic:
        default:
            retCode = BERR_INVALID_PARAMETER;
            return( retCode );
            break;
    }

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(hDev->hHab, buf, 2, buf, 1, true, true, 2));

done:

    BDBG_LEAVE(BAUS_SetOperationMode);
    return( retCode );
}

