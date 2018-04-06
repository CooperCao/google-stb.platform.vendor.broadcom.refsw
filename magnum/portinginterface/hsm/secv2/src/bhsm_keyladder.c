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
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_keyladder.h"
#include "bhsm_p_keyladder.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_keyladder_priv.h"
#include "bhsm_hwkl.h"
#include "bsp_types.h"

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
#define HWKL_INDEX    MAX_NUM_KEYLADDERS

typedef struct BHSM_KeyLadderModule
{
    BHSM_Handle hHsm;                       /* HSM handle. */
    BHSM_P_KeyLadder keyLadders[MAX_NUM_KEYLADDERS + 1];         /*TODO ... use define. */
}BHSM_KeyLadderModule;


typedef struct
{
    BHSM_KeyslotHandle hKeySlot;   /* the keyslot handle to route to. */
    BHSM_KeyslotBlockEntry entry;  /* the entry within the keyslot to route to. */

    unsigned keyLadderIndex;       /* the vklId */
    unsigned keyLadderLayer;       /* the level within the keyladder to route key from. */
}BHSM_P_KeyslotRouteKey;

typedef struct
{
    BHSM_KeyslotHandle hKeySlot;   /* the keyslot handle to route to. */
    BHSM_KeyslotBlockEntry entry;  /* the entry within the keyslot to route to. */

    unsigned keyLadderIndex;       /* the vklId */
    unsigned keyLadderLayer;       /* the level within the keyladder to route key from. */
    bool     configIv2;           /*true if key is to be routed to IV2 */
}BHSM_P_KeyslotRouteIv;


static BERR_Code  _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static BERR_Code  _GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static uint8_t _MapCustomerSubMode( BHSM_KeyLadderMode mode );
static BERR_Code _RouteEntryKey( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteKey *pConf );
static BERR_Code _RouteEntryIv( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteIv *pConf );

#if 0
static void dumpKeyLadderOwnership( BHSM_Handle hHsm, const char *pFunction  );
#endif

BERR_Code BHSM_KeyLadder_Init( BHSM_Handle hHsm, BHSM_KeyLadderModuleSettings *pSettings )
{

    BDBG_ENTER( BHSM_KeyLadder_Init );
    BSTD_UNUSED( pSettings );

    hHsm->modules.pKeyLadders = BKNI_Malloc( sizeof(BHSM_KeyLadderModule) );
    if( !hHsm->modules.pKeyLadders ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pKeyLadders, 0, sizeof(BHSM_KeyLadderModule) );

    hHsm->modules.pKeyLadders->hHsm = hHsm;

    BDBG_LEAVE( BHSM_KeyLadder_Init );
    return BERR_SUCCESS;
}


void BHSM_KeyLadder_Uninit( BHSM_Handle hHsm )
{

    BDBG_ENTER( BHSM_KeyLadder_Uninit );

    if( !hHsm->modules.pKeyLadders ) {
        BERR_TRACE( BERR_NOT_INITIALIZED );
        return;
    }

    BKNI_Memset( hHsm->modules.pKeyLadders, 0, sizeof(BHSM_KeyLadderModule) );
    BKNI_Free( hHsm->modules.pKeyLadders );
    hHsm->modules.pKeyLadders = NULL;

#if 0
    dumpKeyLadderOwnership( hHsm, BSTD_FUNCTION );
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
    BHSM_P_KeyLadder *pKeyLadder = NULL;
    BHSM_KeyLadderModule *pModule = NULL;
    BHSM_P_KeyLadderFwklInvalidate invFwkl;
    unsigned i;

    BDBG_ENTER( BHSM_KeyLadder_Allocate );

    pModule = hHsm->modules.pKeyLadders;
    if( !pModule ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit;  }

    if( (pSettings->index < MAX_NUM_KEYLADDERS) || (pSettings->index == BHSM_HWKL_ID))
    {
        unsigned index = (pSettings->index == BHSM_HWKL_ID) ? HWKL_INDEX : pSettings->index;

        if( pModule->keyLadders[index].allocated )
        {
            BERR_TRACE( BERR_NOT_AVAILABLE );
            goto exit;
        }
        pKeyLadder = &(pModule->keyLadders[index]);
        if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit;  }

        BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
        pKeyLadder->index = index;
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
               if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit;  }

               BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
               pKeyLadder->index = i;
               pKeyLadder->owner = pSettings->owner;
               pKeyLadder->hHsm = hHsm;
               pKeyLadder->allocated = true;
               goto exit;
           }
        }

        if( !pKeyLadder ) { BERR_TRACE( BERR_NOT_AVAILABLE ); }
    }

exit:

    if (pKeyLadder)
    {
        /* no invalidation for HWKL */
        if(pSettings->index != BHSM_HWKL_ID)
        {
            /* Invalid the FWKL of the vklId being used. */
            BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );
            invFwkl.in.vklId = pKeyLadder->index;
            invFwkl.in.clearAllKeyLayer = 1;
            invFwkl.in.freeOwnership = 1;
            BHSM_P_KeyLadder_FwklInvalidate( pKeyLadder->hHsm, &invFwkl );
        }
    }

    BDBG_LEAVE( BHSM_KeyLadder_Allocate );
    return (BHSM_KeyLadderHandle)pKeyLadder;
}

/*
    Free a KeyLadder Resource
*/
void BHSM_KeyLadder_Free( BHSM_KeyLadderHandle handle )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderFwklInvalidate invFwkl;

    BDBG_ENTER( BHSM_KeyLadder_Free );

    if( pkeyLadder->allocated == false )
    {
        BERR_TRACE( BERR_NOT_AVAILABLE );
        return;
    }

    pkeyLadder->allocated = false;

    /* Invalid the FWKL of the vklId being used. */
    BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );
    invFwkl.in.vklId = pkeyLadder->index;
    invFwkl.in.clearAllKeyLayer = 1;
    invFwkl.in.freeOwnership = 1;
    BHSM_P_KeyLadder_FwklInvalidate( pkeyLadder->hHsm, &invFwkl );

    BDBG_LEAVE( BHSM_KeyLadder_Free );
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

    if( !pSettings )  { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pkeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if ((pSettings->mode == BHSM_KeyLadderMode_eHwlk) && (pkeyLadder->index != HWKL_INDEX))
    {
        return  BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if ((pSettings->mode != BHSM_KeyLadderMode_eHwlk) && (pkeyLadder->index == BHSM_HWKL_ID))
    {
        return  BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( pKey->level == 3 )
    {
        if(pkeyLadder->index == HWKL_INDEX)
        {
            rc =  BHSM_Keyslot_GenerateHwKlRootKey( handle, pKey );  /* configure the root key. */
        }
        else
        {
            rc =  _GenerateRootKey( handle, pKey );  /* configure the root key. */
        }
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        goto exit;
    }

    if(pkeyLadder->index == HWKL_INDEX)
    {
        rc =  BHSM_Keyslot_GenerateHwKlLevelKey( handle, pKey );
    }
    else
    {
        rc =  _GenerateLevelKey( handle, pKey );
    }
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    if( pKey->route.destination == BHSM_KeyLadderDestination_eNone )
    {
        goto exit; /* nothing more to do. */
    }

    /*route the key, as required. */
    if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotKey )
    {

        if(pkeyLadder->index == HWKL_INDEX)
        {
            BHSM_KeyslotRouteKey routeKey;

            BKNI_Memset( &routeKey, 0, sizeof(routeKey) );
            routeKey.keyLadderIndex = pkeyLadder->index;
            routeKey.keyLadderLayer = pKey->level;

            rc = BHSM_Keyslot_RouteHWKlEntryKey( pKey->route.keySlot.handle
                                                 , pKey->route.keySlot.entry
                                                 , &routeKey );
        }
        else
        {
            BHSM_P_KeyslotRouteKey routeKey;

            BKNI_Memset( &routeKey, 0, sizeof(routeKey) );
            routeKey.hKeySlot =  pKey->route.keySlot.handle;
            routeKey.entry = pKey->route.keySlot.entry;
            routeKey.keyLadderIndex = pkeyLadder->index;
            routeKey.keyLadderLayer = pKey->level;

            rc = _RouteEntryKey( handle, &routeKey );
        }
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        goto exit;
    }

    if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv ||
        pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv2 )
    {
        BHSM_P_KeyslotRouteIv routeIv;

        BKNI_Memset( &routeIv, 0, sizeof(routeIv) );
        routeIv.keyLadderIndex = pkeyLadder->index;
        routeIv.keyLadderLayer = pKey->level;
        routeIv.hKeySlot =  pKey->route.keySlot.handle;
        routeIv.entry = pKey->route.keySlot.entry;
        if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv2 ) { routeIv.configIv2 = true; }

        rc = _RouteEntryIv(  handle, &routeIv );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

        goto exit;
    }

    /* other destination not supported yet!*/
    BERR_TRACE( BERR_NOT_SUPPORTED );

exit:

    BDBG_LEAVE( BHSM_KeyLadder_GenerateLevelKey );
    return rc;
}

BERR_Code BHSM_KeyLadder_Invalidate( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderInvalidate  *pInvalidate )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderFwklInvalidate invFwkl;

    BDBG_ENTER( BHSM_KeyLadder_Invalidate );

    if( !pInvalidate ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );

    invFwkl.in.vklId = pKeyLadder->index;
    invFwkl.in.clearAllKeyLayer = 1;
    invFwkl.in.freeOwnership = pInvalidate->clearOwnership?1:0;

    BHSM_P_KeyLadder_FwklInvalidate( pKeyLadder->hHsm, &invFwkl );

    BDBG_LEAVE( BHSM_KeyLadder_Invalidate );

    return BERR_SUCCESS;
}




BERR_Code BHSM_GetKeyLadderInfo( BHSM_KeyLadderHandle handle, BHSM_KeyLadderInfo *pInfo )
{
    BERR_TRACE( BERR_NOT_SUPPORTED ); /*DEPRECATED . use BHSM_KeyLadder_GetInfo */
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

    BERR_TRACE( BERR_NOT_SUPPORTED );

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

    BERR_TRACE( BERR_NOT_SUPPORTED );

    BDBG_LEAVE( BHSM_KeyLadder_Challenge );
    return BERR_SUCCESS;
}

static BERR_Code  _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderRootConfig bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;
    unsigned keyOffset = 0;

    BDBG_ENTER( _GenerateRootKey );

    if( !pkeyLadder->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSettings = &pkeyLadder->settings;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.vklId = pkeyLadder->index;
    switch( pSettings->algorithm ) {
        case BHSM_CryptographicAlgorithm_e3DesAba:  { bspConfig.in.keyLadderType = 0; break; }
        case BHSM_CryptographicAlgorithm_eAes128:   { bspConfig.in.keyLadderType = 1; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    switch( pSettings->root.type ) {
        case BHSM_KeyLadderRootType_eCustomerKey: {
            bspConfig.in.rootKeySrc = 0;  /*Bsp_RootKeySrc_eCusKey*/
            break;
        }
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm: {
            bspConfig.in.rootKeySrc = pSettings->root.otpKeyIndex+1;
            break;
        }
        case BHSM_KeyLadderRootType_eGlobalKey: {
            bspConfig.in.rootKeySrc = 9;  /*Bsp_RootKeySrc_eAskmGlobalKey*/
            break;
        }
        default: {
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
    }
    bspConfig.in.customerSel = _MapCustomerSubMode( pSettings->mode );
    switch( pSettings->operation ){
        case BHSM_CryptographicOperation_eEncrypt:    { bspConfig.in.keyLadderOperation = 1; break; }
        case BHSM_CryptographicOperation_eDecrypt:    { bspConfig.in.keyLadderOperation = 0; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    if( ( pSettings->root.type == BHSM_KeyLadderRootType_eOtpAskm ) ||
        ( pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey ) )
    {
        bspConfig.in.askmSel               = 1;
        bspConfig.in.caVendorId            = (uint16_t)pSettings->root.askm.caVendorId;
        bspConfig.in.askm3DesKlRootKeySwapEnable = pSettings->root.askm.swapKey?1:0;
        bspConfig.in.stbOwnerIdSel         = (uint8_t)pSettings->root.askm.stbOwnerSelect;
        switch( pSettings->root.askm.caVendorIdScope )
        {
            case BHSM_KeyladderCaVendorIdScope_eChipFamily: bspConfig.in.askmMaskKeySel = 0; break;
            case BHSM_KeyladderCaVendorIdScope_eFixed:      bspConfig.in.askmMaskKeySel = 2; break;
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

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
    }

    switch( pKey->ladderKeySize ) {
        case 64:   { bspConfig.in.keySize = 0; break; }
        case 128:  { bspConfig.in.keySize = 1; break; }
        case 192:  { bspConfig.in.keySize = 2; break; }
        case 256:  { bspConfig.in.keySize = 3; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    keyOffset = (8-(2*(bspConfig.in.keySize+1))); /* offset of key in word */
    if( keyOffset*4 >= sizeof(bspConfig.in.procIn) ||
        keyOffset*4 + pKey->ladderKeySize/8 > sizeof(bspConfig.in.procIn) ) {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    BHSM_Mem32cpy( &bspConfig.in.procIn[keyOffset], pKey->ladderKey, (pKey->ladderKeySize/8) );

    rc = BHSM_P_KeyLadder_RootConfig( pkeyLadder->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _GenerateRootKey );
    return BERR_SUCCESS;
}


static BERR_Code  _GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BHSM_P_KeyLadder *pKeySlot = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderLayerSet bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;
    unsigned keyOffset = 0;

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

    switch( pKey->ladderKeySize ) {
        case 64:   { bspConfig.in.keySize = 0; break; }
        case 128:  { bspConfig.in.keySize = 1; break; }
        case 192:  { bspConfig.in.keySize = 2; break; }
        case 256:  { bspConfig.in.keySize = 3; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    keyOffset = (8-(2*(bspConfig.in.keySize+1))); /* offset of key in word */

    if( keyOffset*4 >= sizeof(bspConfig.in.procIn) ||
        keyOffset*4 + pKey->ladderKeySize/8 > sizeof(bspConfig.in.procIn) ) {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    BHSM_Mem32cpy( &bspConfig.in.procIn[keyOffset], pKey->ladderKey, (pKey->ladderKeySize/8) );

    rc = BHSM_P_KeyLadder_LayerSet( pKeySlot->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _GenerateLevelKey );
    return BERR_SUCCESS;
}

/*
Description:
    Route a key to the the keyslot from a Keyladder
*/
BERR_Code _RouteEntryKey ( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteKey *pConf )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeyLadderRouteKey bspConfig;
    BHSM_KeyslotDetails keyslotDetails;

    BDBG_ENTER( _RouteEntryKey );

    rc = BHSM_P_Keyslot_GetDetails( pConf->hKeySlot, pConf->entry, &keyslotDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.blockType = keyslotDetails.blockType ;
    bspConfig.in.entryType = keyslotDetails.polarity;
    bspConfig.in.keySlotType = BHSM_P_ConvertSlotType(keyslotDetails.slotType);
    bspConfig.in.keySlotNumber = keyslotDetails.number;
    bspConfig.in.keyMode = keyslotDetails.keyModeBspMapped;

    bspConfig.in.modeWords[0] = keyslotDetails.ctrlWord0;
    bspConfig.in.modeWords[1] = keyslotDetails.ctrlWord1;
    bspConfig.in.modeWords[2] = keyslotDetails.ctrlWord2;
    bspConfig.in.modeWords[3] = keyslotDetails.ctrlWord3;

    if( keyslotDetails.externalIvValid ) {
        bspConfig.in.extIvPtr = keyslotDetails.externalIvOffset;
    }

    bspConfig.in.vklId = pConf->keyLadderIndex;
    bspConfig.in.keyLayer = pConf->keyLadderLayer;

    rc = BHSM_P_KeyLadder_RouteKey( BHSM_P_KeyLadder_GetHsmHandle(handle), &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _RouteEntryKey );
    return BERR_SUCCESS;
}

/*
Description:
    Route an Iv to the the keyslot from a Keyladder
*/
BERR_Code _RouteEntryIv( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteIv *pConf )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeyLadderRouteIv bspConfig;
    BHSM_KeyslotDetails keyslotDetails;

    BDBG_ENTER( _RouteEntryIv );

    rc = BHSM_P_Keyslot_GetDetails( pConf->hKeySlot, pConf->entry, &keyslotDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.blockType = keyslotDetails.blockType ;
    bspConfig.in.entryType = keyslotDetails.polarity;
    bspConfig.in.keySlotType = BHSM_P_ConvertSlotType(keyslotDetails.slotType);
    bspConfig.in.keySlotNumber = keyslotDetails.number;

    bspConfig.in.vklId = pConf->keyLadderIndex;
    bspConfig.in.keyLayer = pConf->keyLadderLayer;
    if( pConf->configIv2 ){
        bspConfig.in.ivType = 2;
    }
    else{
        bspConfig.in.ivType = 1;
    }

    rc = BHSM_P_KeyLadder_RouteIv( BHSM_P_KeyLadder_GetHsmHandle(handle), &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _RouteEntryIv );
    return BERR_SUCCESS;
}


static uint8_t _MapCustomerSubMode( BHSM_KeyLadderMode mode )
{

    switch(mode){
        case BHSM_KeyLadderMode_eCa_64_4:        { return Bsp_CustomerSubMode_eGeneric_Ca_64_4; }
        case BHSM_KeyLadderMode_eCp_64_4:        { return Bsp_CustomerSubMode_eGeneric_Cp_64_4; }
        case BHSM_KeyLadderMode_eCa_64_5:        { return Bsp_CustomerSubMode_eGeneric_Ca_64_5; }
        case BHSM_KeyLadderMode_eCp_64_5:        { return Bsp_CustomerSubMode_eGeneric_Cp_64_5; }
        case BHSM_KeyLadderMode_eCa_128_4:       { return Bsp_CustomerSubMode_eGeneric_Ca_128_4; }
        case BHSM_KeyLadderMode_eCp_128_4:       { return Bsp_CustomerSubMode_eGeneric_Cp_128_4; }
        case BHSM_KeyLadderMode_eCa_128_5:       { return Bsp_CustomerSubMode_eGeneric_Ca_128_5; }
        case BHSM_KeyLadderMode_eCp_128_5:       { return Bsp_CustomerSubMode_eGeneric_Cp_128_5; }
        case BHSM_KeyLadderMode_eCa_64_7:        { return Bsp_CustomerSubMode_eGeneric_Ca_64_7; }
        case BHSM_KeyLadderMode_eCa_128_7:       { return Bsp_CustomerSubMode_eGeneric_Ca_128_7; }
        case BHSM_KeyLadderMode_eCa64_45:        { return Bsp_CustomerSubMode_eGeneric_Ca_64_45; }
        case BHSM_KeyLadderMode_eCp64_45:        { return Bsp_CustomerSubMode_eGeneric_Cp_64_45; }
        case BHSM_KeyLadderMode_eSageBlDecrypt:  { return Bsp_CustomerSubMode_eSageBlDecrypt; }
        case BHSM_KeyLadderMode_eSage128_5:      { return 0xd; }
        case BHSM_KeyLadderMode_eSage128_4:      { return 0xe; }
        case BHSM_KeyLadderMode_eGeneralPurpose1:{ return Bsp_CustomerSubMode_eGeneralPurpose1; }
        case BHSM_KeyLadderMode_eGeneralPurpose2:{ return Bsp_CustomerSubMode_eGeneralPurpose2; }
        case BHSM_KeyLadderMode_eEtsi_5:         { return Bsp_CustomerSubMode_eEtsi_5; }
        case BHSM_KeyLadderMode_eRpmb:           { return Bsp_CustomerSubMode_eRpmb; }
        default: BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return 0xff;
}


BHSM_Handle BHSM_P_KeyLadder_GetHsmHandle( BHSM_KeyLadderHandle handle )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    if( !handle ) { return NULL; }

    return pkeyLadder->hHsm;
}

bool BHSM_P_KeyLadder_CheckConfigured( BHSM_KeyLadderHandle handle )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    if( !handle ) { return false; }

    return pkeyLadder->configured;
}


uint8_t BHSM_P_KeyLadder_MapRootKeySrc( BHSM_KeyLadderRootType rootKeySrc, unsigned otpIndex )
{

    switch( rootKeySrc )
    {
        case BHSM_KeyLadderRootType_eCustomerKey: { return Bsp_RootKeySrc_eCusKey; }
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm:     { return (Bsp_RootKeySrc_eOtpa + otpIndex); }
        case BHSM_KeyLadderRootType_eGlobalKey:   { return Bsp_RootKeySrc_eAskmGlobalKey; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return 0xFF;  /* invalid type */
}



uint8_t BHSM_P_KeyLadder_MapCaVendorIdScope( BHSM_KeyladderCaVendorIdScope caVendorIdScope )
{
    switch( caVendorIdScope )
    {
        case BHSM_KeyladderCaVendorIdScope_eChipFamily: return 0;
        case BHSM_KeyladderCaVendorIdScope_eFixed:      return 2;
        default: BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return 0xFF;
}

BERR_Code BHSM_P_KeyLadder_GetDetails( BHSM_KeyLadderHandle handle, BHSM_KeyLadderDetails_t *pDetails )
{
    BHSM_P_KeyLadder *pKeySlot = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_P_KeyLadder_GetDetails );

    if( !handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pDetails ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pDetails->hHsm = pKeySlot->hHsm;
    pDetails->configured = pKeySlot->configured;
    pDetails->index = pKeySlot->index;

    BDBG_LEAVE( BHSM_P_KeyLadder_GetDetails );
    return BERR_SUCCESS;
}



#if 0
static void dumpKeyLadderOwnership( BHSM_Handle hHsm, const char *pFunction  )
{
    BERR_Code rc;
    BHSM_P_KeyLadderFwklQuery query;
    unsigned numKeyladders = sizeof(query.out.fwklOwnership) / sizeof(query.out.fwklOwnership[0]);
    unsigned i;

    rc = BHSM_P_KeyLadder_FwklQuery( hHsm, &query );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return; }

    for( i = 0; i < numKeyladders; i++ )
    {
        BDBG_LOG(("KL[%d] ownership [%02d] submode[%02x] [%s] ", i, (unsigned)query.out.fwklOwnership[i], (unsigned)query.out.fwklSubCustomerMode[i], pFunction ));
    }

    return;
}
#endif
