/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_bsp_msg.h"

BDBG_MODULE(BHSM);


BHSM_Handle BHSM_Open( const BHSM_ModuleSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_Handle *pHandle = NULL;
    BHSM_KeyslotModuleSettings keyslotSettings;
    BHSM_BspMsgConfigure_t bspMsgConfig;
    unsigned i = 0;

    BDBG_ENTER( BHSM_Open );
    BSTD_UNUSED( pSettings );

    pHandle = BKNI_Malloc( sizeof(*pHandle) );
    if ( pHandle == NULL ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL; }
    BKNI_Memset( pHandle, 0, sizeof(*pHandle) );

    pHandle->regHandle = pSettings->hReg;
    pHandle->chipHandle = pSettings->hChip;
    pHandle->interruptHandle = pSettings->hInterrupt;

    BKNI_Memset( &bspMsgConfig, 0, sizeof(bspMsgConfig) );
    rc = BHSM_BspMsg_Init( pHandle, &bspMsgConfig );
    if( rc != BERR_SUCCESS ) { (void)BERR_TRACE( rc ); goto error; }

    BKNI_Memset( &keyslotSettings, 0, sizeof(keyslotSettings) );
    for( i = 0; i < BHSM_KeyslotType_eMax; i++ )
    {
        keyslotSettings.numKeySlotsForType[i] = pSettings->numKeySlotsForType[i];
    }
    rc = BHSM_Keyslot_Init( pHandle, &keyslotSettings );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    BDBG_LEAVE( BHSM_Open );
    return pHandle;

error:
    if( pHandle ) BKNI_Free( pHandle );

    return NULL;
}


BERR_Code BHSM_Close( BHSM_Handle hHsm )
{
    BHSM_P_Handle *pHandle = hHsm;

    BDBG_ENTER( BHSM_Close );

    if( pHandle == NULL ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BHSM_Keyslot_Uninit( hHsm );
    BHSM_BspMsg_Uninit( hHsm );

    BKNI_Free( pHandle );

    BDBG_LEAVE( BHSM_Close );
    return BERR_SUCCESS;
}


void BHSM_GetCapabilities( BHSM_Handle hHsm, BHSM_ModuleCapabilities *pCaps )
{
    BDBG_ENTER( BHSM_GetCapabilities );

    BSTD_UNUSED( hHsm );

    BKNI_Memset( pCaps, 0, sizeof(*pCaps) );

    BDBG_LEAVE( BHSM_GetCapabilities );
    return;
}
