/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7364ib.h"
#include "btnr_leapib_priv.h"

BDBG_MODULE(btnr_7364);

#define DEV_MAGIC_ID                ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_7364_SETTINGS_I2CADDR  (0x6C)          /* 7bit addr */
#define BTNR_7364_MAX_CHANNELS      1

/*******************************************************************************/
BERR_Code BTNR_7364Ib_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_7364Ib_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_LeapIb_Handle h7364Dev;
    BTNR_P_LeapIb_Settings *pTnrImplData;
    BTNR_Handle hDev;

    BDBG_ENTER(BTNR_7364_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h7364Dev = (BTNR_LeapIb_Handle) BKNI_Malloc( sizeof( BTNR_P_LeapIb_Handle ) );
    if( h7364Dev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7364_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h7364Dev, 0x00, sizeof( BTNR_P_LeapIb_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7364_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h7364Dev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7364Dev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h7364Dev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;

    h7364Dev->channelNo = pSettings->channelNo;

    h7364Dev->hHab = hHab;

    hDev->hDevImpl = (void *) h7364Dev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_LeapIb_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_LeapIb_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_LeapIb_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_LeapIb_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_LeapIb_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_LeapIb_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_LeapIb_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_LeapIb_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_LeapIb_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_LeapIb_SetSettings;

    if(pSettings->channelNo >= pSettings->mxChnNo)
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Only %d TNR channels supported", pSettings->mxChnNo));
    }
    else
        *phDev = hDev;
done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *phDev = NULL;
    }
    BDBG_LEAVE(BTNR_7364_Open);
    return( retCode );
}

BERR_Code BTNR_7364Ib_GetDefaultSettings(
    BTNR_7364Ib_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_7364_SETTINGS_I2CADDR;
    pDefSettings->mxChnNo = BTNR_7364_MAX_CHANNELS;

    return BERR_SUCCESS;
}
