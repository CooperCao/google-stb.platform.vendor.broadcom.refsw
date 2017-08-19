/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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

    BDBG_ENTER(BTNR_31xxIb_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

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

