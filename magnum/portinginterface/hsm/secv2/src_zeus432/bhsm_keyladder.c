/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_s_keycommon.h"
#include "bhsm_keyladder_priv.h"
#include "bhsm_keyladder.h"

BDBG_MODULE( BHSM );

typedef struct
{
    bool allocated;
    BHSM_SecurityCpuContext owner; /* used to specify whether the keyladder is intended for HOST or SAGE usage. */

    BHSM_Handle hHsm;                       /* HSM handle. */
    bool configured;
    unsigned index;
    BHSM_KeyLadderSettings settings;

}BHSM_P_KeyLadder;


#define MAX_NUM_KEYLADDERS 8 /*TODO ... use BSP define. */


typedef struct BHSM_KeyLadderModule
{
    BHSM_Handle hHsm;                       /* HSM handle. */
    BHSM_P_KeyLadder keyLadders[MAX_NUM_KEYLADDERS]; 		/*TODO ... use define. */
}BHSM_KeyLadderModule;


static BERR_Code  _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static BERR_Code  _GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static BERR_Code _RouteIv( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static BERR_Code _RouteKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );


BERR_Code BHSM_KeyLadder_Init( BHSM_Handle hHsm, BHSM_KeyLadderModuleSettings *pSettings )
{

    BDBG_ENTER( BHSM_KeyLadder_Init );
    BSTD_UNUSED( pSettings );

    hHsm->modules.pKeyLadders = BKNI_Malloc( sizeof(BHSM_KeyLadderModule) );
    if( !hHsm->modules.pKeyLadders ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pKeyLadders, 0, sizeof(BHSM_KeyLadderModule) );

#if 0
    hHsm->modules.pKeyLadders->hHsm = hHsm;
#endif

    BDBG_LEAVE( BHSM_KeyLadder_Init );
    return BERR_SUCCESS;
}


void BHSM_KeyLadder_Uninit( BHSM_Handle hHsm )
{

    BDBG_ENTER( BHSM_KeyLadder_Uninit );
    BSTD_UNUSED( hHsm );

#if 0
    if( !hHsm->modules.pKeyLadders ) {
        BERR_TRACE( BERR_NOT_INITIALIZED );
        return;
    }

    BKNI_Memset( hHsm->modules.pKeyLadders, 0, sizeof(BHSM_KeyLadderModule) );
    BKNI_Free( hHsm->modules.pKeyLadders );
    hHsm->modules.pKeyLadders = NULL;
#endif

    BDBG_LEAVE( BHSM_KeyLadder_Uninit );
    return;
}


/*
    Allocate a KeyLadder Resource
*/
BHSM_KeyLadderHandle BHSM_KeyLadder_Allocate( BHSM_Handle hHsm,
                                              const BHSM_KeyLadderAllocateSettings *pSettings )
{
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pSettings );
#if 0
    BHSM_P_KeyLadder *pKeyLadder = NULL;
    BHSM_KeyLadderModule *pModule = NULL;
    unsigned i;

    BDBG_ENTER( BHSM_KeyLadder_Allocate );

    pModule = hHsm->modules.pKeyLadders;

    if( pSettings->index < MAX_NUM_KEYLADDERS )
    {
        if( pModule->keyLadders[pSettings->index].allocated  )
        {
            BERR_TRACE( BERR_NOT_AVAILABLE );
            goto exit;
        }
        pKeyLadder = &(pModule->keyLadders[pSettings->index]);
        BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
        pKeyLadder->index = pSettings->index;
        pKeyLadder->owner = pSettings->owner;
        pKeyLadder->hHsm = hHsm;
        pKeyLadder->allocated = true;
        goto exit;
    }
    else
    {
        for( i = 0; i < MAX_NUM_KEYLADDERS; i++ )
        {
           if( pModule->keyLadders[i].allocated == false )
           {
               pKeyLadder = &pModule->keyLadders[i];
               BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
               pKeyLadder->index = i;
               pKeyLadder->allocated = true;
               goto exit;
           }
        }

        if( !pKeyLadder ) { BERR_TRACE( BERR_NOT_AVAILABLE ); }
    }

exit:

    BDBG_LEAVE( BHSM_KeyLadder_Allocate );
    return (BHSM_KeyLadderHandle)pKeyLadder;
#endif
    return NULL;
}

/*
    Free a KeyLadder Resource
*/
void BHSM_KeyLadder_Free( BHSM_KeyLadderHandle handle )
{
    BSTD_UNUSED( handle );
#if 0
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_Free );

    if( pkeyLadder->allocated == false )
    {
        BERR_TRACE( BERR_NOT_AVAILABLE );
        return;
    }

    pkeyLadder->allocated = false;

    BDBG_LEAVE( BHSM_KeyLadder_Free );
#endif
    return;
}

/*
    returns the current or default setting for the keyladder.
*/
void BHSM_KeyLadder_GetSettings( BHSM_KeyLadderHandle handle,
                                 BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_GetSettings );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return;  }

    *pSettings = pkeyLadder->settings;

    BDBG_LEAVE( BHSM_KeyLadder_GetSettings );
    return;
}

/*
    Configure the keyladder.
*/
BERR_Code BHSM_KeyLadder_SetSettings( BHSM_KeyLadderHandle handle,
                                      const BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_SetSettings );

    /* TODO, check validity/consistency of (some) setings. */
    pkeyLadder->settings = *pSettings;

    pkeyLadder->configured = true;

    BDBG_LEAVE( BHSM_KeyLadder_SetSettings );
    return BERR_SUCCESS;
}

/*
    Progress the key down the keyladder.
*/
BERR_Code BHSM_KeyLadder_GenerateLevelKey( BHSM_KeyLadderHandle handle,
                                           const BHSM_KeyLadderLevelKey  *pKey )
{

    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_KeyLadder_GenerateLevelKey );

    if( pKey->level > pkeyLadder->settings.numLevels )
    {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if( pKey->level == 3 )
    {
        /* configure the root key. */
        rc =  _GenerateRootKey( handle, pKey );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
	goto exit;
    }

    if( pKey->route.destination == BHSM_KeyLadderDestination_eNone )
    {
        rc =  _GenerateLevelKey( handle, pKey );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
	goto exit;
    }

    if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotKey )
    {
        rc = _RouteKey( handle, pKey );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
	goto exit;
    }

    if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv )
    {
        rc = _RouteIv( handle, pKey );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
	goto exit;
    }

    /* other destination not supported yet!*/
    BERR_TRACE( BERR_NOT_SUPPORTED );

exit:

    BDBG_LEAVE( BHSM_KeyLadder_GenerateLevelKey );
    return rc;
}

BERR_Code BHSM_GetKeyLadderInfo( BHSM_KeyLadderHandle handle, BHSM_KeyLadderInfo *pInfo )
{
    BERR_TRACE( BERR_NOT_SUPPORTED ); /* FUNCTION DEPRECATED . use BHSM_KeyLadder_GetInfo. Function will be removed. */
    return BHSM_KeyLadder_GetInfo( handle, pInfo );
}

BERR_Code BHSM_KeyLadder_GetInfo( BHSM_KeyLadderHandle handle, BHSM_KeyLadderInfo *pInfo )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BDBG_ENTER( BHSM_KeyLadder_GetInfo );

    if( !pInfo ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pInfo->index = pkeyLadder->index;

    BDBG_LEAVE( BHSM_KeyLadder_GetInfo );

    return BERR_SUCCESS;
}

void BHSM_KeyLadder_GetDefaultChallenge( BHSM_KeyLadderChallenge  *pChallenge )
{

    BDBG_ENTER( BHSM_KeyLadder_GetDefaultChallenge );
    BSTD_UNUSED( pChallenge );

    BDBG_LEAVE( BHSM_KeyLadder_GetDefaultChallenge );
    return;
}

/*
    Challange the keyladder. Can be used to authenticate the STB.
*/
BERR_Code BHSM_KeyLadder_Challenge( BHSM_KeyLadderHandle handle,
                                    const BHSM_KeyLadderChallenge  *pChallenge,
                                    BHSM_KeyLadderChallengeResponse *pResponse )
{

    BDBG_ENTER( BHSM_KeyLadder_Challenge );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pChallenge );
    BSTD_UNUSED( pResponse );


    BDBG_LEAVE( BHSM_KeyLadder_Challenge );
    return BERR_SUCCESS;
}

static BERR_Code  _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{

    BSTD_UNUSED( handle );
    BSTD_UNUSED( pKey );
#if 0
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderRootConfig bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( _GenerateRootKey );

    if( !pkeyLadder->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSettings = &pkeyLadder->settings;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.vklId       = pkeyLadder->index;
    bspConfig.in.customerSel = pSettings->mode;

    if( ( pSettings->root.type == BHSM_KeyLadderRootType_eOtpAskm ) ||
        ( pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey ) )
    {
        bspConfig.in.askmSel             = 1;
        bspConfig.in.caVendorId          = (uint16_t)pSettings->root.askm.caVendorId;
        bspConfig.in.caVendorIdExtension = (uint8_t)pSettings->root.askm.caVendorIdScope;
        bspConfig.in.stbOwnerIdSel       = (uint8_t)pSettings->root.askm.stbOwnerSelect;

        if( pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey )
        {
            bspConfig.in.globalKeyIndex = pSettings->root.globalKey.index;

            switch( pSettings->root.globalKey.owner )
            {
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0: { bspConfig.in.globalKeyOwnerIdSelect = 0;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1: { bspConfig.in.globalKeyOwnerIdSelect = 1;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne:  { bspConfig.in.globalKeyOwnerIdSelect = 2;  break; }
                default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
            }
        }

        if( pSettings->root.type == BHSM_KeyLadderRootType_eOtpAskm )
        {

        }
    }

    BKNI_Memcpy( bspConfig.in.procIn, pKey->ladderKey, sizeof( bspConfig.in.procIn ) );

    rc = BHSM_P_KeyLadder_RootConfig( pkeyLadder->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _GenerateRootKey );
#endif
    return BERR_SUCCESS;
}


static BERR_Code  _GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pKey );
#if 0
    BHSM_P_KeyLadder *pKeySlot = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderLayerSet bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( _GenerateLevelKey );

    if( !pKeySlot->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSettings = &pKeySlot->settings;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.vklId = pKeySlot->index;
    switch( pSettings->algorithm ) {
        case BHSM_CryptographicAlgorithm_e3DesAba:  { bspConfig.in.keyLadderType = 0; break; }
        case BHSM_CryptographicAlgorithm_eAes128:   { bspConfig.in.keyLadderType = 1; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    bspConfig.in.destinationKeyLayer = pKey->level;

    switch( pSettings->operation ){
        case BHSM_CryptographicOperation_eEncrypt:    { bspConfig.in.keyLadderOperation = 1; break; }
        case BHSM_CryptographicOperation_eDecrypt:    { bspConfig.in.keyLadderOperation = 0; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    BKNI_Memcpy( bspConfig.in.procIn, pKey->ladderKey, sizeof( bspConfig.in.procIn ) );

    switch( pSettings->keySize ) {
        case 64:   { bspConfig.in.keySize = 0; break; }
        case 128:  { bspConfig.in.keySize = 1; break; }
        case 192:  { bspConfig.in.keySize = 2; break; }
        case 256:  { bspConfig.in.keySize = 3; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    rc = BHSM_P_KeyLadder_LayerSet( pKeySlot->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _GenerateLevelKey );
#endif
    return BERR_SUCCESS;
}



static BERR_Code _RouteIv( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pKey );
#if 0
    BHSM_P_KeyLadder *pKeySlot = NULL;
    BHSM_P_KeyLadderRouteIv bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BHSM_KeyslotInfo keySlotInfo;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( _RouteIv );

    pKeySlot = (BHSM_P_KeyLadder*)handle;
    pSettings = &pKeySlot->settings;

    if( !pKeySlot->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    switch( pKey->route.keySlot.entry )
    {
        case BHSM_KeyslotBlockEntry_eCpdOdd:
        case BHSM_KeyslotBlockEntry_eCpdEven:
        case BHSM_KeyslotBlockEntry_eCpdClear:
        {
            bspConfig.in.blockType = 0 /*Bsp_KeySlotBlockType_eCPDescrambler*/;
            break;
        }
        case BHSM_KeyslotBlockEntry_eCaOdd:
        case BHSM_KeyslotBlockEntry_eCaEven:
        case BHSM_KeyslotBlockEntry_eCaClear:
        {
            bspConfig.in.blockType = 1 /*Bsp_KeySlotBlockType_eCA*/;
            break;
        }
        case BHSM_KeyslotBlockEntry_eCpsOdd:
        case BHSM_KeyslotBlockEntry_eCpsEven:
        case BHSM_KeyslotBlockEntry_eCpsClear:
        {
            bspConfig.in.blockType = 2 /*Bsp_KeySlotBlockType_eCPScrambler*/;
            break;
        }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    switch( pKey->route.keySlot.entry )
    {
        case BHSM_KeyslotBlockEntry_eCpdOdd:
        case BHSM_KeyslotBlockEntry_eCaOdd:
        case BHSM_KeyslotBlockEntry_eCpsOdd:
        {
            bspConfig.in.entryType = 0; /* Bsp_KeySlotEntryType_eOddKey */
            break;
        }
        case BHSM_KeyslotBlockEntry_eCpdEven:
        case BHSM_KeyslotBlockEntry_eCaEven:
        case BHSM_KeyslotBlockEntry_eCpsEven:
        {
            bspConfig.in.entryType = 1; /* Bsp_KeySlotEntryType_eEvenKey */
            break;
        }
        case BHSM_KeyslotBlockEntry_eCpdClear:
        case BHSM_KeyslotBlockEntry_eCaClear:
        case BHSM_KeyslotBlockEntry_eCpsClear:
        {
            bspConfig.in.entryType = 2; /* Bsp_KeySlotEntryType_eClearKey */
            break;
        }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    rc = BHSM_GetKeySlotInfo( pKey->route.keySlot.handle, &keySlotInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    bspConfig.in.keySlotType   = keySlotInfo.type;
    bspConfig.in.keySlotNumber = keySlotInfo.number;
    bspConfig.in.vklId = pKeySlot->index;
    bspConfig.in.keyLayer = pKey->level;

    switch (pKey->route.destination){
        case BHSM_KeyLadderDestination_eKeyslotIv : {bspConfig.in.ivType = 1; break;} /* Bsp_KeySlotIvType_eIv*/
        case BHSM_KeyLadderDestination_eKeyslotIv2: {bspConfig.in.ivType = 2; break;} /* Bsp_KeySlotIvType_eAesShortIv*/
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    rc = BHSM_P_KeyLadder_RouteIv( pKeySlot->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _RouteIv );
#endif
    return BERR_SUCCESS;
}


static BERR_Code  _RouteKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pKey );
#if 0
    BHSM_P_KeyLadder *pKeySlot = NULL;
    BHSM_P_KeyLadderRouteKey bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( _RouteKey );

    pKeySlot = (BHSM_P_KeyLadder*)handle;
    pSettings = &pKeySlot->settings;

    if( !pKeySlot->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
    bspConfig.in.vklId = pKeySlot->index;

    /*BKNI_Memcpy( bspConfig.in.procIn, pKey->ladderKey, sizeof( bspConfig.in.procIn ) );*/

    rc = BHSM_P_KeyLadder_RouteKey( pKeySlot->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _RouteKey );
#endif
    return BERR_SUCCESS;
}
