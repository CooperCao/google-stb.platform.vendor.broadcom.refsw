/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7255ib_priv.h"

BDBG_MODULE(btnr_7255_priv);

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
BERR_Code BTNR_7255_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7255_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BTNR_7255_Close);
    return retCode;
}

BERR_Code BTNR_7255_SetRfFreq(
    BTNR_7255_Handle hDev,            /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7255_Settings *pTnrImplData;

    uint8_t hab[17] = HAB_MSG_HDR(BTNR_ACQUIRE_PARAMS_WRITE, 0xC, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t bandwidth = 0;
    BSTD_UNUSED(tunerMode);

    BDBG_ENTER(BTNR_7255_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    pTnrImplData = &hDev->settings;

    hab[4] = pTnrImplData->std + 1;

    switch (pTnrImplData->bandwidth)
    {
        case 8000000:
            bandwidth = 1;
            break;
        case 7000000:
            bandwidth = 2;
            break;
        case 6000000:
            bandwidth = 3;
            break;
        case 5000000:
            bandwidth = 4;
            break;
        case 1700000:
            bandwidth = 5;
            break;
        default:
            BDBG_ERR(("Invalid bandwidth, valid bandwidths are 8Mhz, 7Mhz, 6Mhz, 5Mhz and 1.7Mhz"));
    }

    hab[3] = hDev->channelNo;
    hab[4] |= (bandwidth << 3);
    if (pTnrImplData->std == BTNR_Standard_eQam)
        hab[4] |= (BTNR_TunerApplication_eCable + 1) << 6;
    else
        hab[4] |= (BTNR_TunerApplication_eTerrestrial + 1) << 6;
    hab[5] = 0x80;
    hab[12] = (rfFreq >> 24);
    hab[13] = (rfFreq >> 16);
    hab[14] = (rfFreq >> 8);
    hab[15] = rfFreq;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 17, hab, 0, false, true, 17));

done:
    BDBG_LEAVE(BTNR_7255_SetRfFreq);
    return retCode;
}

BERR_Code BTNR_7255_GetRfFreq(
    BTNR_7255_Handle hDev,            /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7255_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_7255_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_7255_GetRfFreq);
    return retCode;
}

BERR_Code BTNR_P_7255_GetAgcRegVal(
    BTNR_7255_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);

    return retCode;
}

BERR_Code BTNR_7255_SetAgcRegVal(
    BTNR_7255_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);

    return retCode;
}

BERR_Code BTNR_7255_GetInfo(
    BTNR_7255_Handle hDev,            /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{
    BTNR_P_7255_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7255_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x7255;
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_7255_GetInfo);
    return retCode;
}

BERR_Code BTNR_7255_GetPowerSaver(
    BTNR_7255_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings        /* [in] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[9] = HAB_MSG_HDR(BTNR_POWER_CTRL_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7255_GetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hab[3] = hDev->channelNo;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 5, hab, 9, false, true, 9 ));

    if(hab[4] == 0xFF)
        pwrSettings->enable = 0;
    else
        pwrSettings->enable = 1;

done:
    return retCode;
}

BERR_Code BTNR_7255_SetPowerSaver(
    BTNR_7255_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[5] = HAB_MSG_HDR(BTNR_POWER_CTRL_ON, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t hab1[5] = HAB_MSG_HDR(BTNR_POWER_CTRL_OFF, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7255_SetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hab[3] = hDev->channelNo;
    hab1[3] = hDev->channelNo;
    if(pwrSettings->enable)
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab1, 5, hab1, 0, false, true, 5 ));
    else
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 5, hab, 0, false, true, 5 ));

done:
    BDBG_LEAVE(BTNR_7255_SetPowerSaver);
    return retCode;
}

BERR_Code BTNR_7255_GetSettings(
    BTNR_7255_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    )
{
    BTNR_P_7255_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7255_GetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    settings->std = pTnrImplData->std;
    settings->bandwidth = pTnrImplData->bandwidth;

    BDBG_LEAVE(BTNR_7255_GetSettings);
    return retCode;
}

BERR_Code BTNR_7255_SetSettings(
    BTNR_7255_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    )

{
    BTNR_P_7255_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7255_SetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    pTnrImplData->std = settings->std;
    pTnrImplData->bandwidth = settings->bandwidth;

    BDBG_LEAVE(BTNR_7255_SetSettings);
    return retCode;
}

BERR_Code BTNR_7255_GetVersionInfo(
    BTNR_7255_Handle hDev,          /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[29] = HAB_MSG_HDR(BTNR_SYS_VERSION, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7255_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 29, false, true, 29));
    pVersionInfo->majorVersion = (buf[4] << 8) | buf[5];
    pVersionInfo->minorVersion = (buf[6] << 8) | buf[7];
    pVersionInfo->buildType = (buf[8] << 8) | buf[9];
    pVersionInfo->buildId = (buf[10] << 8) | buf[11];

done:
    BDBG_LEAVE(BTNR_7255_GetVersionInfo);
    return( retCode );
}
