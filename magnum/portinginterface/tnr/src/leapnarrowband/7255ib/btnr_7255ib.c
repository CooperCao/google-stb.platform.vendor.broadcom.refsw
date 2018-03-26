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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7255ib.h"
#include "btnr_7255ib_priv.h"

BDBG_MODULE(btnr_7255);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_7255_SETTINGS_I2CADDR    (0x66)          /* 7bit addr */

/*******************************************************************************/
BERR_Code BTNR_7255_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_7255_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7255_Handle h7255Dev;
    BTNR_P_7255_Settings *pTnrImplData;
    BTNR_Handle hDev;


    BDBG_ENTER(BTNR_7255_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h7255Dev = (BTNR_7255_Handle) BKNI_Malloc( sizeof( BTNR_P_7255_Handle ) );
    if( h7255Dev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7255_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset( h7255Dev, 0x00, sizeof( BTNR_P_7255_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7255_Open: BKNI_malloc() failed"));
        BKNI_Free( h7255Dev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7255Dev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h7255Dev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
    h7255Dev->channelNo = pSettings->channelNo;
    h7255Dev->hHab = hHab;
    hDev->hDevImpl = (void *) h7255Dev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_7255_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_7255_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_7255_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_7255_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_7255_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_7255_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_7255_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_7255_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_7255_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_7255_SetSettings;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_7255_GetVersionInfo;

done:
    *phDev = hDev;
    BDBG_LEAVE(BTNR_7255_Open);
    return( retCode );
}

BERR_Code BTNR_7255_GetDefaultSettings(
    BTNR_7255_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_7255_SETTINGS_I2CADDR;

    return BERR_SUCCESS;
}