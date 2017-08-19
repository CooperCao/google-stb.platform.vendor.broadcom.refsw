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
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_312xob_priv.h"

BDBG_MODULE(btnr_312xOb_priv);


#define DEV_MAGIC_ID            ((BERR_TNR_ID<<16) | 0xFACE)

/*******************************************************************************/
BERR_Code BTNR_312xOb_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_312xOb_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hDev->magicId = 0x00;       /* clear it to catch improper use */
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BTNR_312xOb_Close);
    return  retCode;
}

BERR_Code BTNR_312xOb_SetRfFreq(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[13] = HAB_MSG_HDR(BTNR_ACQUIRE_PARAMS_WRITE, 0x8, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t hab1[7] = HAB_MSG_HDR(BTNR_ACQUIRE, 0x2, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BSTD_UNUSED(tunerMode);
    
    BDBG_ENTER(BTNR_312xOb_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core Powered Off"));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {        
        hab[5] = BTNR_CORE_TYPE_TO_FEED;
        hab[8] = (rfFreq >> 24);
        hab[9] = (rfFreq >> 16);
        hab[10] = (rfFreq >> 8);
        hab[11] = rfFreq;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 13, hab, 0, false, true, 13));

        hab1[5] = (BTNR_CORE_TYPE_TO_FEED & 0x0F);

        /* WFE Acquire */  
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab1, 7, hab1, 0, false, true, 7));  
    }

done:   
    BDBG_LEAVE(BTNR_312xOb_SetRfFreq);
    return  retCode;
}
            
BERR_Code BTNR_312xOb_GetRfFreq(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_312xOb_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_312xOb_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_312xOb_GetRfFreq);
    return  retCode;
}

BERR_Code BTNR_P_312xOb_GetAgcRegVal(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    

    BDBG_ENTER(BTNR_P_312xOb_GetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    retCode = BHAB_ReadMbox( hDev->hHab, regOffset, agcVal );

    BDBG_LEAVE(BTNR_P_312xOb_GetAgcRegVal);
    return  retCode;
}

BERR_Code BTNR_312xOb_SetAgcRegVal(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BTNR_312xOb_SetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    retCode = BHAB_WriteMbox( hDev->hHab, regOffset, agcVal );
	
    BDBG_LEAVE(BTNR_312xOb_SetAgcRegVal);
    return  retCode;
}

BERR_Code BTNR_312xOb_GetInfo(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{   
    BTNR_P_312xOb_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_312xOb_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    /* Hard coded here - Vish. */
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x3128; 
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_312xOb_GetInfo);
    return  retCode;
}

BERR_Code BTNR_312xOb_GetPowerSaver(
	BTNR_312xOb_Handle hDev,					/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings 		/* [in] Power saver settings. */
	)
{  
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTNR_POWER_STATUS_READ, 0x0, BTNR_CORE_TYPE, BTNR_CHANNEL_ID); 
    BSTD_UNUSED(hDev);
    
    BDBG_ENTER(BTNR_312xOb_GetPowerSaver);
    
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 9, false, true, 9));  
    
    if(buf[4]){
        pwrSettings->enable = false;
    }
    else {
        pwrSettings->enable = true;
    }

done: 
    BDBG_LEAVE(BTNR_312xOb_GetPowerSaver);
    return retCode;
}

BERR_Code BTNR_312xOb_SetPowerSaver(
	BTNR_312xOb_Handle hDev,					/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings 		/* [in] Power saver settings. */
	)
{   
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BTNR_POWER_ON, 0x0, BTNR_CORE_TYPE, BTNR_CHANNEL_ID); 
    uint8_t buf1[5] = HAB_MSG_HDR(BTNR_POWER_OFF, 0x0, BTNR_CORE_TYPE, BTNR_CHANNEL_ID); 
    BSTD_UNUSED(hDev);

    BDBG_ENTER(BTNR_312xOb_SetPowerSaver);
    
    if(pwrSettings->enable) {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf1, 5, buf1, 0, false, true, 5));
        hDev->bPowerdown = true;
    }
    else {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 0, false, true, 5));
        hDev->bPowerdown = false;     
    }
    
done:    
    BDBG_LEAVE(BTNR_312xOb_SetPowerSaver);
    return retCode;
}


BERR_Code BTNR_312xOb_GetSettings(
    BTNR_312xOb_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    )
{   
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(settings);

    return retCode;
}

BERR_Code BTNR_312xOb_SetSettings(
    BTNR_312xOb_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    )

{  
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(settings);

    return retCode;
}

BERR_Code BTNR_312xOb_GetVersionInfo(
    BTNR_312xOb_Handle hDev,        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[29] = HAB_MSG_HDR(BTNR_SYS_VERSION, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);  

    BDBG_ENTER(BTNR_312xOb_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 29, false, true, 29)); 
    pVersionInfo->majorVersion = (buf[4] << 8) | buf[5];
    pVersionInfo->minorVersion = (buf[6] << 8) | buf[7];
    pVersionInfo->buildType = (buf[8] << 8) | buf[9];
    pVersionInfo->buildId = (buf[10] << 8) | buf[11];
    
done:    
    BDBG_LEAVE(BTNR_312xOb_GetVersionInfo);
    return( retCode );
}
