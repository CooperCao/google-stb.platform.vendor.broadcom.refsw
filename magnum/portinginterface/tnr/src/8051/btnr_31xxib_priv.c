/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_31xxib_priv.h"

BDBG_MODULE(btnr_31xx_priv);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)

#define CHK_RETCODE( rc, func )         \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/
BERR_Code BTNR_31xxIb_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_31xxIb_Handle btnr_31xxib_handle;
    BTNR_P_31xxIb_Settings btnr_31xxib_settings;


    BDBG_ENTER(BTNR_31xxIb_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* verify the handle is good before using it */
    btnr_31xxib_handle = (BTNR_31xxIb_Handle) hDev->hDevImpl;
    btnr_31xxib_settings = btnr_31xxib_handle ->settings;

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BTNR_31xxIb_Close);
    return( retCode );
}

BERR_Code BTNR_31xxIb_SetRfFreq(
    BTNR_31xxIb_Handle hDev,            /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_31xxIb_Settings *pTnrImplData;
    uint8_t hab[5];
    
    BDBG_ENTER(BTNR_31xxIb_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    hab[0] = 0x22;
    hab[1] = 0x01;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 2, hab, 1, true, true, 2));

    /* 31xx is a digital only tuner */
    if(tunerMode == BTNR_TunerMode_eDigital)
    {
        BKNI_Memset(hab, 0x00, sizeof(hab));
        hab[0]= 0x11;
        hab[1]= rfFreq >> 24;
        hab[2]= rfFreq >> 16 ;
        hab[3]= rfFreq >> 8;
        hab[4]= rfFreq;
        
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 5, hab, 1, true, true, 5 ));

        pTnrImplData->rfFreq = rfFreq;
        pTnrImplData->tunerMode = tunerMode;
    }
    else
    {
        retCode = BERR_INVALID_PARAMETER;
    }

done:
    BDBG_LEAVE(BTNR_31xxIb_SetRfFreq);
    return( retCode );
}
            
BERR_Code BTNR_31xxIb_GetRfFreq(
    BTNR_31xxIb_Handle hDev,            /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_31xxIb_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_31xxIb_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_31xxIb_GetRfFreq);
    return( retCode );
}

BERR_Code BTNR_P_31xxIb_GetAgcRegVal(
    BTNR_31xxIb_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTNR_P_31xxIb_GetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    retCode = BHAB_ReadMbox( hDev->hHab, regOffset, agcVal );

    BDBG_LEAVE(BTNR_P_31xxIb_GetAgcRegVal);
    return( retCode );
}

BERR_Code BTNR_31xxIb_SetAgcRegVal(
    BTNR_31xxIb_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTNR_31xxIb_SetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    retCode = BHAB_WriteMbox( hDev->hHab, regOffset, agcVal );
    
    BDBG_LEAVE(BTNR_31xxIb_SetAgcRegVal);
    return( retCode );
}

BERR_Code BTNR_31xxIb_GetInfo(
    BTNR_31xxIb_Handle hDev,            /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{   
    BTNR_P_31xxIb_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_31xxIb_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x3117;
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_31xxIb_GetInfo);
    return( retCode );
}

BERR_Code BTNR_31xxIb_GetPowerSaver(
    BTNR_31xxIb_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings        /* [in] Power saver settings. */
    )
{   
    BTNR_P_31xxIb_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_31xxIb_GetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    pwrSettings->enable = pTnrImplData->powerSaver;
    
    BDBG_LEAVE(BTNR_31xxIb_GetPowerSaver);
    return( retCode );
}

BERR_Code BTNR_31xxIb_SetPowerSaver(
    BTNR_31xxIb_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    )
{   
    BTNR_P_31xxIb_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[2];

    BDBG_ENTER(BTNR_31xxIb_SetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    if(pwrSettings->enable) {
        hab[0]= 0x21;
    }
    else {
        hab[0]= 0x22;
    }   
    hab[1]= 0x01;

    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 2, hab, 1, true, true, 2));

    pTnrImplData->powerSaver = pwrSettings->enable;

done:   
    BDBG_LEAVE(BTNR_31xxIb_SetPowerSaver);
    return( retCode );
}

