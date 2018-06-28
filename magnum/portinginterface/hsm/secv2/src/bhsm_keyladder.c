/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bhsm.h"
#include "bsp_p_hw.h"
#include "bhsm_priv.h"
#include "bhsm_keyladder.h"
#include "bhsm_p_keyladder.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_keyladder_priv.h"
#include "bhsm_hwkl.h"
#include "bsp_types.h"

BDBG_MODULE( BHSM );


#define _MAX_NUM_KEYLADDERS_FW          8
#define _MAX_NUM_KEYLADDERS_HW          1
#define _MAX_NUM_KEYLADDERS             (_MAX_NUM_KEYLADDERS_FW + _MAX_NUM_KEYLADDERS_HW)
#define _HW_KEYLADDER_INDEX              _MAX_NUM_KEYLADDERS_FW  /* the last keyladder. */
#define _DEBUG_DUMP_LADDER_OWNERSHIP    0

BDBG_OBJECT_ID(BHSM_P_KeyLadder);

typedef struct BHSM_KeyLadderModule
{
    BHSM_Handle hHsm;              /* HSM handle. */
    BHSM_P_KeyLadder keyLadders[_MAX_NUM_KEYLADDERS];

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
    bool     configIv2;            /* true to routed to IV2 */

}BHSM_P_KeyslotRouteIv;


static BERR_Code _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static BERR_Code _GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey );
static uint8_t   _MapCustomerSubMode( BHSM_KeyLadderMode mode );
static BERR_Code _RouteEntryKey( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteKey *pConf );
static BERR_Code _RouteEntryIv( BHSM_KeyLadderHandle handle, const BHSM_P_KeyslotRouteIv *pConf );

#if _DEBUG_DUMP_LADDER_OWNERSHIP
static void dumpKeyLadderOwnership( BHSM_Handle hHsm, const char *pFunction  );
#endif

BERR_Code BHSM_KeyLadder_Init( BHSM_Handle hHsm, BHSM_KeyLadderModuleSettings *pSettings )
{

    BDBG_ENTER( BHSM_KeyLadder_Init );

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
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

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !hHsm->modules.pKeyLadders ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return; }

    BKNI_Memset( hHsm->modules.pKeyLadders, 0, sizeof(BHSM_KeyLadderModule) );
    BKNI_Free( hHsm->modules.pKeyLadders );
    hHsm->modules.pKeyLadders = NULL;

   #if _DEBUG_DUMP_LADDER_OWNERSHIP
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
    unsigned i;

    BDBG_ENTER( BHSM_KeyLadder_Allocate );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );
    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    pModule = hHsm->modules.pKeyLadders;
    if( !pModule ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL;  }

    if( (pSettings->index < _MAX_NUM_KEYLADDERS_FW) || (pSettings->index == BHSM_HWKL_ID))
    {
        unsigned index = (pSettings->index == BHSM_HWKL_ID) ? _HW_KEYLADDER_INDEX : pSettings->index;

        if( pModule->keyLadders[index].allocated ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }

        pKeyLadder = &(pModule->keyLadders[index]);

        BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
        pKeyLadder->index = index;
        pKeyLadder->owner = pSettings->owner;
        pKeyLadder->hHsm = hHsm;

    }
    else if( pSettings->index == BHSM_ANY_ID )
    {
        /* search for a free one */
        for( i = 0; i < _MAX_NUM_KEYLADDERS_FW; i++ )
        {
           if( pModule->keyLadders[i].allocated ) { continue; /* try next */ }

           pKeyLadder = &pModule->keyLadders[i];

           BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
           pKeyLadder->index = i;
           pKeyLadder->owner = pSettings->owner;
           pKeyLadder->hHsm = hHsm;
           break; /* slot found,  */
        }

        if( !pKeyLadder ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }

    }
    else
    {
        BERR_TRACE( BERR_INVALID_PARAMETER ); /* invalid index. */
        return NULL;
    }

    BDBG_ASSERT( pKeyLadder ); /* will not get here without a handle. */

    /* no invalidation for HWKL */
    if( pSettings->index != BHSM_HWKL_ID )
    {
        BHSM_P_KeyLadderFwklInvalidate invFwkl;
        BERR_Code rc = BERR_UNKNOWN;

        /* Invalid the FWKL of the vklId being used. */
        BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );
        invFwkl.in.vklId = pKeyLadder->index;
        invFwkl.in.clearAllKeyLayer = 1;
        invFwkl.in.freeOwnership = 1;
        rc = BHSM_P_KeyLadder_FwklInvalidate( hHsm, &invFwkl );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return NULL; }
    }

    BDBG_OBJECT_SET( pKeyLadder, BHSM_P_KeyLadder );
    pKeyLadder->allocated = true;

    BDBG_LEAVE( BHSM_KeyLadder_Allocate );
    return (BHSM_KeyLadderHandle)pKeyLadder;
}

/*
    Free a KeyLadder Resource
*/
void BHSM_KeyLadder_Free( BHSM_KeyLadderHandle handle )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderFwklInvalidate invFwkl;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_KeyLadder_Free );

    if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pKeyLadder->allocated ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return; }

    if( pKeyLadder->index < _MAX_NUM_KEYLADDERS_FW )
    {
        /* Invalid the FWKL of the vklId being used. */
        BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );
        invFwkl.in.vklId = pKeyLadder->index;
        invFwkl.in.clearAllKeyLayer = 1;
        invFwkl.in.freeOwnership = 1;
        rc = BHSM_P_KeyLadder_FwklInvalidate( pKeyLadder->hHsm, &invFwkl );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); }
    }

    pKeyLadder->allocated = false;
    BDBG_OBJECT_UNSET( pKeyLadder, BHSM_P_KeyLadder );

    BDBG_LEAVE( BHSM_KeyLadder_Free );
    return;
}

/*
    returns the current or default setting for the keyladder.
*/
void BHSM_KeyLadder_GetSettings( BHSM_KeyLadderHandle handle,
                                 BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_GetSettings );

    if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return;  }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return;  }

    *pSettings = pKeyLadder->settings;

    BDBG_LEAVE( BHSM_KeyLadder_GetSettings );
    return;
}

/*
    Configure the keyladder.
*/
BERR_Code BHSM_KeyLadder_SetSettings( BHSM_KeyLadderHandle handle,
                                      const BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_SetSettings );

    if( !pKeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pSettings ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if((pSettings->mode == BHSM_KeyLadderMode_eHwlk) && (pKeyLadder->index != _HW_KEYLADDER_INDEX))
    {
        return  BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if((pSettings->mode != BHSM_KeyLadderMode_eHwlk) && (pKeyLadder->index == BHSM_HWKL_ID))
    {
        return  BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    pKeyLadder->settings = *pSettings;
    pKeyLadder->configured = true;

    BDBG_LEAVE( BHSM_KeyLadder_SetSettings );
    return BERR_SUCCESS;
}

/*
    Progress the key down the keyladder.
*/
BERR_Code BHSM_KeyLadder_GenerateLevelKey( BHSM_KeyLadderHandle handle,
                                           const BHSM_KeyLadderLevelKey  *pKey )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_KeyLadder_GenerateLevelKey );

    if( !pKeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pKey )  { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pKey->level == 3 )
    {
        if(pKeyLadder->index == _HW_KEYLADDER_INDEX)
        {
            rc =  BHSM_Keyslot_GenerateHwKlRootKey( handle, pKey );  /* configure the root key. */
            if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
        }
        else
        {
            rc =  _GenerateRootKey( handle, pKey );  /* configure the root key. */
            if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
        }
        goto exit; /* nothing more to do on level 3. */
    }

    if( pKeyLadder->index == _HW_KEYLADDER_INDEX )
    {
        rc =  BHSM_Keyslot_GenerateHwKlLevelKey( handle, pKey );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
    }
    else
    {
        rc =  _GenerateLevelKey( handle, pKey );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
    }

    /* do we need to route the generated key? */
    switch( pKey->route.destination )
    {
        case BHSM_KeyLadderDestination_eNone:
        {
             /* nothing to do. */
            break;
        }
        case BHSM_KeyLadderDestination_eKeyslotKey:
        {
            if( pKeyLadder->index == _HW_KEYLADDER_INDEX )
            {
                BHSM_KeyslotRouteKey routeKey;

                BKNI_Memset( &routeKey, 0, sizeof(routeKey) );
                routeKey.keyLadderIndex = pKeyLadder->index;
                routeKey.keyLadderLayer = pKey->level;

                rc = BHSM_Keyslot_RouteHWKlEntryKey( pKey->route.keySlot.handle
                                                     , pKey->route.keySlot.entry
                                                     , &routeKey );
                if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
            }
            else
            {
                BHSM_P_KeyslotRouteKey routeKey;

                BKNI_Memset( &routeKey, 0, sizeof(routeKey) );
                routeKey.hKeySlot =  pKey->route.keySlot.handle;
                routeKey.entry = pKey->route.keySlot.entry;
                routeKey.keyLadderIndex = pKeyLadder->index;
                routeKey.keyLadderLayer = pKey->level;

                rc = _RouteEntryKey( handle, &routeKey );
                if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
            }
            break;
        }
        case BHSM_KeyLadderDestination_eKeyslotIv:
        case BHSM_KeyLadderDestination_eKeyslotIv2:
        {
            BHSM_P_KeyslotRouteIv routeIv;

            BKNI_Memset( &routeIv, 0, sizeof(routeIv) );
            routeIv.keyLadderIndex = pKeyLadder->index;
            routeIv.keyLadderLayer = pKey->level;
            routeIv.hKeySlot =  pKey->route.keySlot.handle;
            routeIv.entry = pKey->route.keySlot.entry;
            if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv2 ) { routeIv.configIv2 = true; }

            rc = _RouteEntryIv(  handle, &routeIv );
            if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

            break;
        }
        default:
        {
            /* other destination not supported yet!*/
            rc = BERR_TRACE( BERR_NOT_SUPPORTED );
            break;
        }
    }

exit:

    BDBG_LEAVE( BHSM_KeyLadder_GenerateLevelKey );
    return rc;
}

BERR_Code BHSM_KeyLadder_Invalidate( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderInvalidate  *pInvalidate )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderFwklInvalidate invFwkl;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_KeyLadder_Invalidate );

    if( !pKeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pInvalidate ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKeyLadder->index >= _MAX_NUM_KEYLADDERS_FW ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &invFwkl, 0, sizeof(invFwkl) );

    invFwkl.in.vklId = pKeyLadder->index;
    invFwkl.in.clearAllKeyLayer = 1;
    invFwkl.in.freeOwnership = pInvalidate->clearOwnership?1:0;

    rc = BHSM_P_KeyLadder_FwklInvalidate( pKeyLadder->hHsm, &invFwkl );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

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
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_GetInfo );

    if( !pKeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( pKeyLadder, BHSM_P_KeyLadder );
    if( !pInfo ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pInfo->index = pKeyLadder->index;

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

    BDBG_OBJECT_ASSERT( handle, BHSM_P_KeyLadder );

    BERR_TRACE( BERR_NOT_SUPPORTED );

    BDBG_LEAVE( BHSM_KeyLadder_Challenge );
    return BERR_SUCCESS;
}

static BERR_Code  _GenerateRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_P_KeyLadderRootConfig bspConfig;
    BHSM_KeyLadderSettings *pSettings;
    BERR_Code rc = BERR_SUCCESS;
    unsigned keyWordOffset = 0;
    unsigned keySize = 0; /* key size in bytes. */

    BDBG_ENTER( _GenerateRootKey );

    if( !pKeyLadder ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKeyLadder->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSettings = &pKeyLadder->settings;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.vklId = pKeyLadder->index;
    switch( pSettings->algorithm ) {
        case BHSM_CryptographicAlgorithm_e3DesAba:  { bspConfig.in.keyLadderType = 0; break; }
        case BHSM_CryptographicAlgorithm_eAes128:   { bspConfig.in.keyLadderType = 1; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    bspConfig.in.rootKeySrc = BHSM_P_KeyLadder_MapRootKeySrc( pSettings->root.type, pSettings->root.otpKeyIndex );
    bspConfig.in.customerSel = _MapCustomerSubMode( pSettings->mode );
    switch( pSettings->operation ){
        case BHSM_CryptographicOperation_eEncrypt:    { bspConfig.in.keyLadderOperation = 1; break; }
        case BHSM_CryptographicOperation_eDecrypt:    { bspConfig.in.keyLadderOperation = 0; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    if( ( pSettings->root.type == BHSM_KeyLadderRootType_eOtpAskm ) ||
        ( pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey ) )
    {
        bspConfig.in.askmSel        = 1;
        bspConfig.in.caVendorId     = (uint16_t)pSettings->root.askm.caVendorId;
        bspConfig.in.askm3DesKlRootKeySwapEnable = pSettings->root.askm.swapKey?1:0;
        bspConfig.in.stbOwnerIdSel  = (uint8_t)pSettings->root.askm.stbOwnerSelect;
        bspConfig.in.askmMaskKeySel = BHSM_P_KeyLadder_MapCaVendorIdScope( pSettings->root.askm.caVendorIdScope );

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

    keyWordOffset = (8-(2*(bspConfig.in.keySize+1))); /* offset of key in words. BFW API specified
                                                         algorithm that puts the key at the end
                                                         of the space available. */
    keySize = pKey->ladderKeySize/8;                  /* bit to byte */

    if( keyWordOffset*4 >= sizeof(bspConfig.in.procIn) ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( keyWordOffset*4 + keySize > sizeof(bspConfig.in.procIn) ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_MemcpySwap( &bspConfig.in.procIn[keyWordOffset], pKey->ladderKey, keySize );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    rc = BHSM_P_KeyLadder_RootConfig( pKeyLadder->hHsm, &bspConfig );
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
    unsigned keyWordOffset = 0;
    unsigned keySize = 0; /* key size in bytes. */

    BDBG_ENTER( _GenerateLevelKey );

    if( !pKeySlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKeySlot->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

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

    keyWordOffset = (8-(2*(bspConfig.in.keySize+1))); /* offset of key in word. BFW API specified
                                                         algorithm that puts the key at the end
                                                         of the space available. */
    keySize = pKey->ladderKeySize/8;                  /* bit to byte */

    if( keyWordOffset*4 >= sizeof(bspConfig.in.procIn) ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( keyWordOffset*4 + keySize > sizeof(bspConfig.in.procIn) ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_MemcpySwap( &bspConfig.in.procIn[keyWordOffset], pKey->ladderKey, keySize );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

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
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeyLadderRouteKey bspConfig;
    BHSM_KeyslotSlotDetails slotDetails;
    BHSM_KeyslotEntryDetails entryDetails;

    BDBG_ENTER( _RouteEntryKey );

    if( !pKeyLadder ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pConf ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_P_Keyslot_GetSlotDetails( pConf->hKeySlot, &slotDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    rc = BHSM_P_Keyslot_GetEntryDetails( pConf->hKeySlot, pConf->entry, &entryDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.blockType = entryDetails.blockType;
    bspConfig.in.entryType = entryDetails.polarity;
    bspConfig.in.keySlotType = BHSM_P_ConvertSlotType(slotDetails.slotType);
    bspConfig.in.keySlotNumber = slotDetails.number;
    bspConfig.in.keyMode = entryDetails.bspMapped.keyMode;

    bspConfig.in.modeWords[0] = entryDetails.bspMapped.ctrlWord0;
    bspConfig.in.modeWords[1] = entryDetails.bspMapped.ctrlWord1;
    bspConfig.in.modeWords[2] = entryDetails.bspMapped.ctrlWord2;
    bspConfig.in.modeWords[3] = entryDetails.bspMapped.ctrlWord3;

    if( entryDetails.bspMapped.externalIvValid ) {
        bspConfig.in.extIvPtr = entryDetails.bspMapped.externalIvOffset;
    }

    bspConfig.in.vklId = pConf->keyLadderIndex;
    bspConfig.in.keyLayer = pConf->keyLadderLayer;

    rc = BHSM_P_KeyLadder_RouteKey( pKeyLadder->hHsm, &bspConfig );
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
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeyLadderRouteIv bspConfig;
    BHSM_KeyslotSlotDetails slotDetails;
    BHSM_KeyslotEntryDetails entryDetails;

    BDBG_ENTER( _RouteEntryIv );

    if( !pKeyLadder ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pConf ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_P_Keyslot_GetSlotDetails( pConf->hKeySlot, &slotDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    rc = BHSM_P_Keyslot_GetEntryDetails( pConf->hKeySlot, pConf->entry, &entryDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.blockType = entryDetails.blockType;
    bspConfig.in.entryType = entryDetails.polarity;
    bspConfig.in.keySlotType = BHSM_P_ConvertSlotType(slotDetails.slotType);
    bspConfig.in.keySlotNumber = slotDetails.number;

    bspConfig.in.vklId = pConf->keyLadderIndex;
    bspConfig.in.keyLayer = pConf->keyLadderLayer;
    if( pConf->configIv2 ){
        bspConfig.in.ivType = 2;
    }
    else{
        bspConfig.in.ivType = 1;
    }

    rc = BHSM_P_KeyLadder_RouteIv( pKeyLadder->hHsm, &bspConfig );
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

uint8_t BHSM_P_KeyLadder_MapRootKeySrc( BHSM_KeyLadderRootType rootKeySrc, unsigned otpIndex )
{
    uint8_t bspRootKeySrc = Bsp_RootKeySrc_eMax;

    switch( rootKeySrc )
    {
        case BHSM_KeyLadderRootType_eCustomerKey: { bspRootKeySrc = Bsp_RootKeySrc_eCusKey; break; }
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm: {
            if( otpIndex >= BSP_NUM_OTP_ROOT_KEYS ) {
                BERR_TRACE( BERR_INVALID_PARAMETER );
                break;
            }
            bspRootKeySrc = (Bsp_RootKeySrc_eOtpa + otpIndex);
            break;
        }
        case BHSM_KeyLadderRootType_eGlobalKey: { bspRootKeySrc = Bsp_RootKeySrc_eAskmGlobalKey; break; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); break; }
    }

    return bspRootKeySrc;
}

uint8_t BHSM_P_KeyLadder_MapCaVendorIdScope( BHSM_KeyLadderCaVendorIdScope caVendorIdScope )
{
    switch( caVendorIdScope )
    {
        case BHSM_KeyLadderCaVendorIdScope_eChipFamily: return Bsp_Askm_MaskKeySel_eRealMaskKey;
        case BHSM_KeyLadderCaVendorIdScope_eFixed:      return Bsp_Askm_MaskKeySel_eFixedMaskKey;
        default: BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return Bsp_Askm_MaskKeySel_eMax;
}

BERR_Code BHSM_P_KeyLadder_GetDetails( BHSM_KeyLadderHandle handle, BHSM_KeyLadderDetails_t *pDetails )
{
    BHSM_P_KeyLadder *pKeySlot = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_P_KeyLadder_GetDetails );

    if( !handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( handle, BHSM_P_KeyLadder );
    if( !pDetails ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pDetails->hHsm = pKeySlot->hHsm;
    pDetails->configured = pKeySlot->configured;
    pDetails->index = pKeySlot->index;

    BDBG_LEAVE( BHSM_P_KeyLadder_GetDetails );
    return BERR_SUCCESS;
}

#if _DEBUG_DUMP_LADDER_OWNERSHIP
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
        BDBG_LOG(("KL[%d] ownership [%02d] submode[%02x] [%s] ", i
                                                               , (unsigned)query.out.fwklOwnership[i]
                                                               , (unsigned)query.out.fwklSubCustomerMode[i]
                                                               , pFunction ));
    }

    return;
}
#endif
