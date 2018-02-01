/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "priv/nexus_security_priv.h"
#include "nexus_security_common.h"
#include "nexus_keyslot_priv.h"
#include "nexus_keyladder.h"
#include "priv/nexus_keyladder_priv.h"
#include "bhsm_keyladder.h"

BDBG_MODULE(nexus_keyladder);

struct NEXUS_KeyLadder
{
    NEXUS_OBJECT(NEXUS_KeyLadder);

    BHSM_KeyLadderHandle hsmKeyLadderHandle;
    NEXUS_KeyLadderSettings settings;

    unsigned reserved;
};

BHSM_KeyLadderMode  _MapKeyMode( NEXUS_KeyLadderMode mode );

void NEXUS_KeyLadder_GetDefaultAllocateSettings( NEXUS_KeyLadderAllocateSettings *pSettings )
{
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    return;
}

/*
    Allocate a KeyLadder Resource
*/
NEXUS_KeyLadderHandle NEXUS_KeyLadder_Allocate( unsigned index,
                                                const NEXUS_KeyLadderAllocateSettings *pSettings )
{
    BHSM_KeyLadderAllocateSettings hsmSettings;
    NEXUS_KeyLadderHandle handle;
    BHSM_Handle hHsm;

    BDBG_ENTER(NEXUS_KeyLadder_Allocate);

    BDBG_CASSERT( NEXUS_ANY_ID == BHSM_ANY_ID );

    handle = BKNI_Malloc( sizeof(struct NEXUS_KeyLadder) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }

    NEXUS_OBJECT_INIT(NEXUS_KeyLadder, handle);

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return NULL; }

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );
    hsmSettings.owner = pSettings->owner;
    hsmSettings.index = index;

    handle->hsmKeyLadderHandle = BHSM_KeyLadder_Allocate( hHsm, &hsmSettings );
    if( handle->hsmKeyLadderHandle == NULL ) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error; }

    return handle;

error:

    if( handle ) {
        if( handle->hsmKeyLadderHandle ) {
            BHSM_KeyLadder_Free( handle->hsmKeyLadderHandle );
            handle->hsmKeyLadderHandle = NULL;
        }
        BKNI_Free( handle );
    }

    BDBG_LEAVE(NEXUS_KeyLadder_Allocate);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_KeyLadder, NEXUS_KeyLadder_Free );

/*
    Free a KeyLadder Resource
*/
void NEXUS_KeyLadder_P_Finalizer( NEXUS_KeyLadderHandle handle )
{

    BDBG_ENTER(NEXUS_KeyLadder_P_Finalizer);

    BHSM_KeyLadder_Free( handle->hsmKeyLadderHandle );

    NEXUS_OBJECT_DESTROY(NEXUS_KeyLadder, handle);
    BKNI_Free( handle );

    BDBG_LEAVE(NEXUS_KeyLadder_P_Finalizer);
    return;
}


void NEXUS_KeyLadder_GetSettings( NEXUS_KeyLadderHandle handle,
                                  NEXUS_KeyLadderSettings *pSettings )
{
    BDBG_ENTER(NEXUS_KeyLadder_GetSettings);
    NEXUS_OBJECT_ASSERT( NEXUS_KeyLadder, handle );

    *pSettings = handle->settings;

    BDBG_LEAVE(NEXUS_KeyLadder_GetSettings);
    return;
}


NEXUS_Error NEXUS_KeyLadder_SetSettings( NEXUS_KeyLadderHandle handle,
                                         const NEXUS_KeyLadderSettings *pSettings )
{
    BHSM_KeyLadderSettings hsmSettings;
    BERR_Code hsmRc = BERR_SUCCESS;

    BDBG_ENTER(NEXUS_KeyLadder_SetSettings);
    NEXUS_OBJECT_ASSERT( NEXUS_KeyLadder, handle );

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );

    hsmSettings.algorithm = pSettings->algorithm;
    hsmSettings.operation = pSettings->operation;
    hsmSettings.mode = _MapKeyMode(  pSettings->mode );
    hsmSettings.keyMode = pSettings->keyMode;

    hsmSettings.root.type = pSettings->root.type;
    hsmSettings.root.otpKeyIndex = pSettings->root.otpKeyIndex;

    hsmSettings.root.askm.caVendorId = pSettings->root.askm.caVendorId ;
    hsmSettings.root.askm.caVendorIdScope = pSettings->root.askm.caVendorIdScope ;
    hsmSettings.root.askm.stbOwnerSelect = pSettings->root.askm.stbOwnerSelect ;
    hsmSettings.root.askm.swapKey = pSettings->root.askm.swapKey;

    hsmSettings.root.globalKey.owner = pSettings->root.globalKey.owner ;
    hsmSettings.root.globalKey.index = pSettings->root.globalKey.index ;

    hsmSettings.root.customerKey.type = pSettings->root.customerKey.type;
    hsmSettings.root.customerKey.swizzle1IndexSel = pSettings->root.customerKey.swizzle1IndexSel;
    hsmSettings.root.customerKey.enableSwizzle0a = pSettings->root.customerKey.enableSwizzle0a;
    hsmSettings.root.customerKey.low.keyVar = pSettings->root.customerKey.low.keyVar;
    hsmSettings.root.customerKey.low.keyIndex = pSettings->root.customerKey.low.keyIndex;
    hsmSettings.root.customerKey.low.decrypt = pSettings->root.customerKey.low.decrypt;
    hsmSettings.root.customerKey.high.keyVar = pSettings->root.customerKey.high.keyVar;
    hsmSettings.root.customerKey.high.keyIndex = pSettings->root.customerKey.high.keyIndex;
    hsmSettings.root.customerKey.high.decrypt = pSettings->root.customerKey.high.decrypt;

    hsmSettings.hwkl.numlevels = pSettings->hwkl.numlevels;
    hsmSettings.hwkl.algorithm = pSettings->hwkl.algorithm;
    hsmSettings.hwkl.moduleId = pSettings->hwkl.moduleId;

    hsmRc = BHSM_KeyLadder_SetSettings( handle->hsmKeyLadderHandle, &hsmSettings );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE(hsmRc); }

    handle->settings = *pSettings;

    BDBG_LEAVE(NEXUS_KeyLadder_SetSettings);
    return NEXUS_SUCCESS;
}

void  NEXUS_KeyLadder_GetLevelKeyDefault( NEXUS_KeyLadderLevelKey *pKey )
{
    BDBG_ENTER(NEXUS_KeyLadder_GetLevelKeyDefault);

    if( !pKey ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pKey, 0, sizeof(*pKey) );

    BDBG_LEAVE(NEXUS_KeyLadder_GetLevelKeyDefault);
    return;
}


NEXUS_Error NEXUS_KeyLadder_GenerateLevelKey( NEXUS_KeyLadderHandle handle,
                                               const NEXUS_KeyLadderLevelKey *pKey )
{
    BHSM_KeyLadderLevelKey hsmLevelKey;
    BERR_Code hsmRc;
    NEXUS_Error rc;

    BDBG_ENTER(NEXUS_KeyLadder_GenerateLevelKey);
    NEXUS_OBJECT_ASSERT( NEXUS_KeyLadder, handle );

    if( !pKey ){ return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( &hsmLevelKey, 0, sizeof(hsmLevelKey) );

    hsmLevelKey.level = pKey->level;
    BKNI_Memcpy( hsmLevelKey.ladderKey, pKey->ladderKey, sizeof(hsmLevelKey.ladderKey) );
    hsmLevelKey.ladderKeySize = pKey->ladderKeySize;

    if( pKey->route.destination != NEXUS_KeyLadderDestination_eNone )
    {
        hsmLevelKey.route.destination = pKey->route.destination;

        hsmLevelKey.route.keySlot.entry = pKey->route.keySlot.entry;

        rc = NEXUS_KeySlot_GetHsmHandle_priv( pKey->route.keySlot.handle, &hsmLevelKey.route.keySlot.handle );
        if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }
    }

    hsmRc = BHSM_KeyLadder_GenerateLevelKey( handle->hsmKeyLadderHandle, &hsmLevelKey );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE(hsmRc); }

    BDBG_LEAVE(NEXUS_KeyLadder_GenerateLevelKey);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_KeyLadder_GetInfo( NEXUS_KeyLadderHandle handle, NEXUS_KeyLadderInfo *pInfo )
{
    BERR_Code hsmRc;
    BHSM_KeyLadderInfo keyLadderInfo;

    BDBG_ENTER(NEXUS_KeyLadder_GetInfo);

    if( !pInfo ){ return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( pInfo, 0, sizeof(*pInfo) );

    hsmRc = BHSM_KeyLadder_GetInfo( handle->hsmKeyLadderHandle, &keyLadderInfo );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE(hsmRc); }

    pInfo->index = keyLadderInfo.index;

    BDBG_LEAVE(NEXUS_KeyLadder_GetInfo);
    return NEXUS_SUCCESS;
}


void NEXUS_KeyLadder_GetDefaultChallenge( NEXUS_KeyLadderChallenge *pChallenge )
{
    BDBG_ENTER(NEXUS_KeyLadder_GetDefaultChallenge);

    BSTD_UNUSED(pChallenge);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);

    BDBG_LEAVE(NEXUS_KeyLadder_GetDefaultChallenge);
    return;
}

NEXUS_Error NEXUS_KeyLadder_Challenge( NEXUS_KeyLadderHandle handle,
                                       const NEXUS_KeyLadderChallenge  *pChallenge,
                                       NEXUS_KeyLadderChallengeResponse *pResponse )
{
    BDBG_ENTER(NEXUS_KeyLadder_Challenge);
    NEXUS_OBJECT_ASSERT( NEXUS_KeyLadder, handle );

    BSTD_UNUSED(pChallenge);
    BSTD_UNUSED(pResponse);

    BDBG_LEAVE(NEXUS_KeyLadder_Challenge);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}



void NEXUS_KeyLadder_GetInfo_priv( NEXUS_KeyLadderHandle handle, NEXUS_KeyLadderInfo_priv *pInfo )
{
    if( !handle ){ BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }
    if( !pInfo ){ BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    pInfo->hsmKeyLadderHandle = handle->hsmKeyLadderHandle;

    return;
}



BHSM_KeyLadderMode  _MapKeyMode( NEXUS_KeyLadderMode mode )
{
    switch( mode )
    {
        case NEXUS_KeyLadderMode_eCa_64_4:                { return  BHSM_KeyLadderMode_eCa_64_4; }
        case NEXUS_KeyLadderMode_eCp_64_4:              { return  BHSM_KeyLadderMode_eCp_64_4; }
        case NEXUS_KeyLadderMode_eCa_64_5:              { return  BHSM_KeyLadderMode_eCa_64_5; }
        case NEXUS_KeyLadderMode_eCp_64_5:              { return  BHSM_KeyLadderMode_eCp_64_5; }
        case NEXUS_KeyLadderMode_eCa_128_4:             { return  BHSM_KeyLadderMode_eCa_128_4; }
        case NEXUS_KeyLadderMode_eCp_128_4:             { return  BHSM_KeyLadderMode_eCp_128_4; }
        case NEXUS_KeyLadderMode_eCa_128_5:             { return  BHSM_KeyLadderMode_eCa_128_5; }
        case NEXUS_KeyLadderMode_eCp_128_5:             { return  BHSM_KeyLadderMode_eCp_128_5; }
        case NEXUS_KeyLadderMode_eCa_64_7:              { return  BHSM_KeyLadderMode_eCa_64_7; }
        case NEXUS_KeyLadderMode_eCa_128_7:             { return  BHSM_KeyLadderMode_eCa_128_7; }
        case NEXUS_KeyLadderMode_eGeneralPurpose1:      { return  BHSM_KeyLadderMode_eGeneralPurpose1; }
        case NEXUS_KeyLadderMode_eGeneralPurpose2:      { return  BHSM_KeyLadderMode_eGeneralPurpose2; }
        case NEXUS_KeyLadderMode_eCa64_45:              { return  BHSM_KeyLadderMode_eCa64_45; }
        case NEXUS_KeyLadderMode_eCp64_45:              { return  BHSM_KeyLadderMode_eCp64_45; }
        case NEXUS_KeyLadderMode_eHwlk:                 { return  BHSM_KeyLadderMode_eHwlk; }
        case NEXUS_KeyLadderMode_eEtsi_5:               { return  BHSM_KeyLadderMode_eEtsi_5; }
        case NEXUS_KeyLadderMode_eScte52Ca_5:           { return  BHSM_KeyLadderMode_eScte52Ca_5; }
        case NEXUS_KeyLadderMode_eSageBlDecrypt:        { return  BHSM_KeyLadderMode_eSageBlDecrypt; }
        case NEXUS_KeyLadderMode_eSage128_5:            { return  BHSM_KeyLadderMode_eSage128_5; }
        case NEXUS_KeyLadderMode_eSage128_4:            { return  BHSM_KeyLadderMode_eSage128_4; }
        default: { BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    }

    return (BHSM_KeyLadderMode)0xFF;
}
