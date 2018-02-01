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
#include "bhsm_bsp_msg.h"
#include "bsp_s_keycommon.h"
#include "bhsm_keyladder.h"
#include "bsp_s_commands.h"
#include "bsp_s_keycommon.h"
#include "bhsm_keyslot_priv.h"

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


#define MAX_NUM_KEYLADDERS BCMD_VKL_KeyRam_eMax
#define GEN_ROUTE_KEY_DATA_LEN (((BCMD_GenKey_InCmd_eKeySize/4)*4) - ((BCMD_GenKey_InCmd_eProcIn/4)*4))
#define KEYLADDER_GENERATE_KEY BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY
#define KEYLADDER_INVALIDATE BCMD_cmdType_eSESSION_INVALIDATE_KEY
#define KEYLADDER_GET_OWNER BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY




typedef struct BHSM_KeyLadderModule
{
    BHSM_Handle hHsm;  /* HSM handle. */
    BHSM_P_KeyLadder keyLadders[MAX_NUM_KEYLADDERS];
}BHSM_KeyLadderModule;


static BERR_Code _GetKeyLadderOwner( BHSM_Handle hHsm, unsigned index, BHSM_SecurityCpuContext *pOwner );
static uint8_t _MapKeyLadderType( BHSM_CryptographicAlgorithm algorithm );
static uint8_t  _MapHwKeyLadderType( BHSM_CryptographicAlgorithm algorithm );
static uint8_t _MapRootKeySource( BHSM_KeyLadderRootType type, unsigned otpKeyIndex );
static uint8_t _MapKeyLadderMode( BHSM_KeyLadderMode mode ); /* Customer Sub mode */
static uint8_t _MapGlobalKeyOwnerIdSelect( BHSM_KeyLadderGlobalKeyOwnerIdSelect owner );
static uint8_t _MapKeySize( unsigned sizeInBits  );
static uint8_t _MapTestKeySel( BHSM_KeyladderCaVendorIdScope  scope );
static uint8_t _MapKeyMode( BHSM_KeyLadderKeyMode keyMode );
static uint8_t _MapSc01Mode( BHSM_KeyslotPolarity useEntry );
static uint8_t _MapPolarity( BHSM_KeyslotPolarity polarity );
static uint8_t  _MapModuleId( BHSM_KeyLadderMode mode ); /* Get moduleId from ladder mode. */
static uint8_t _MapDestinationType( BHSM_KeyLadderDestination destination );


BERR_Code BHSM_KeyLadder_Init( BHSM_Handle hHsm, BHSM_KeyLadderModuleSettings *pSettings )
{
    BHSM_KeyLadderModule *pModule;
    BDBG_ENTER( BHSM_KeyLadder_Init );
    BSTD_UNUSED( pSettings );

    pModule  = BKNI_Malloc( sizeof(BHSM_KeyLadderModule) );
    if( !pModule ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( pModule, 0, sizeof(BHSM_KeyLadderModule) );

    pModule->hHsm = hHsm;

    hHsm->modules.pKeyLadders = (void*)pModule;

    BDBG_LEAVE( BHSM_KeyLadder_Init );
    return BERR_SUCCESS;
}


void BHSM_KeyLadder_Uninit( BHSM_Handle hHsm )
{
    BHSM_KeyLadderModule *pModule;
    BDBG_ENTER( BHSM_KeyLadder_Uninit );
    BSTD_UNUSED( hHsm );

    pModule = (BHSM_KeyLadderModule*)hHsm->modules.pKeyLadders;

    if( !pModule ){ BERR_TRACE( BERR_NOT_INITIALIZED ); return; }

    BKNI_Memset( pModule, 0, sizeof(BHSM_KeyLadderModule) );
    BKNI_Free( hHsm->modules.pKeyLadders );
    hHsm->modules.pKeyLadders = NULL;

    BDBG_LEAVE( BHSM_KeyLadder_Uninit );
    return;
}


/* Allocate a KeyLadder Resource */
BHSM_KeyLadderHandle BHSM_KeyLadder_Allocate( BHSM_Handle hHsm, const BHSM_KeyLadderAllocateSettings *pSettings )
{
    BHSM_P_KeyLadder *pKeyLadder = NULL;
    BHSM_KeyLadderModule *pModule = NULL;
    unsigned i;
    BHSM_SecurityCpuContext owner;
    BHSM_KeyLadderInvalidate invalidate;
    BERR_Code rc;

    BDBG_ENTER( BHSM_KeyLadder_Allocate );

    pModule = (BHSM_KeyLadderModule*)hHsm->modules.pKeyLadders;
    if( !pModule ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto _exit;  }

    if( pSettings->index < MAX_NUM_KEYLADDERS )
    {
        if( pModule->keyLadders[pSettings->index].allocated  )
        {
            BERR_TRACE( BERR_NOT_AVAILABLE );
            return NULL;
        }

        if( pSettings->owner == BHSM_SecurityCpuContext_eHost )
        {   /* Make sure that is not a sage keyladder */
            rc = _GetKeyLadderOwner( hHsm, pSettings->index, &owner );
            if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return NULL; }

            if( owner == BHSM_SecurityCpuContext_eSage ){ BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }
        }

        pKeyLadder = &(pModule->keyLadders[pSettings->index]);
        if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto _exit;  }

        BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
        pKeyLadder->index = pSettings->index;
        pKeyLadder->owner = pSettings->owner;
        pKeyLadder->hHsm = hHsm;
        pKeyLadder->allocated = true;
    }
    else if ( pSettings->index == BHSM_ANY_ID || (pSettings->index == BHSM_HWKL_ID ) )
    {
        for( i = 0; i < MAX_NUM_KEYLADDERS; i++ )
        {
            if( pModule->keyLadders[i].allocated == false )
            {
                if( pSettings->owner == BHSM_SecurityCpuContext_eHost )
                {   /* Make sure that is not a sage keyladder */

                    rc = _GetKeyLadderOwner( hHsm, i, &owner );
                    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto _exit; }

                    if( owner == BHSM_SecurityCpuContext_eSage ){ continue; } /* can't use this keyladder */
                }

                pKeyLadder = &pModule->keyLadders[i];
                if( !pKeyLadder ) { BERR_TRACE( BERR_INVALID_PARAMETER ); goto _exit;  }

                BKNI_Memset( pKeyLadder, 0, sizeof(*pKeyLadder) );
                pKeyLadder->index = i;
                pKeyLadder->owner = pSettings->owner;
                pKeyLadder->hHsm = hHsm;
                pKeyLadder->allocated = true;
                break; /*out of for loop*/
            }
        }

        if( !pKeyLadder ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }
    }
    else
    {
        BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL;
    }

    /* invalidate the keyladder. */
    BKNI_Memset( &invalidate, 0, sizeof(invalidate) );
    invalidate.clearOwnership = true;
    rc = BHSM_KeyLadder_Invalidate( (BHSM_KeyLadderHandle)pKeyLadder, &invalidate );
    if( rc != BERR_SUCCESS )
    {
         pKeyLadder->allocated = false;
         BERR_TRACE( rc );
         return NULL;
    }

_exit:
    BDBG_LEAVE( BHSM_KeyLadder_Allocate );
    return (BHSM_KeyLadderHandle)pKeyLadder;
}

/* Free a KeyLadder Resource */
void BHSM_KeyLadder_Free( BHSM_KeyLadderHandle handle )
{
    BERR_Code rc;
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_KeyLadderInvalidate invalidate;

    BDBG_ENTER( BHSM_KeyLadder_Free );

    if( pkeyLadder->allocated == false ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return; }

    /* invalidate the keyladder. */
    BKNI_Memset( &invalidate, 0, sizeof(invalidate) );
#ifdef BHSM_BUILD_HSM_FOR_SAGE
    invalidate.clearOwnership = true;
#endif
    rc = BHSM_KeyLadder_Invalidate( handle, &invalidate);
    if (rc != BERR_SUCCESS) {BERR_TRACE( rc ); goto _exit;}

    pkeyLadder->allocated = false;

_exit:
    BDBG_LEAVE( BHSM_KeyLadder_Free );
    return;
}

/* returns the current or default setting for the keyladder.  */
void BHSM_KeyLadder_GetSettings( BHSM_KeyLadderHandle handle, BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_GetSettings );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return;  }

    *pSettings = pkeyLadder->settings;

    BDBG_LEAVE( BHSM_KeyLadder_GetSettings );
    return;
}

/* Configure the keyladder. */
BERR_Code BHSM_KeyLadder_SetSettings( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderSettings *pSettings )
{
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;

    BDBG_ENTER( BHSM_KeyLadder_SetSettings );

    if( !pSettings )  { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pkeyLadder ) { return  BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pkeyLadder->settings = *pSettings;

    pkeyLadder->configured = true;

    BDBG_LEAVE( BHSM_KeyLadder_SetSettings );
    return BERR_SUCCESS;
}

/* Progress the key down the keyladder. */
BERR_Code BHSM_KeyLadder_GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey  *pKey )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeyLadder *pkeyLadder = (BHSM_P_KeyLadder*)handle;
    BHSM_KeyLadderSettings *pSettings;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_KeyslotDetails keySlot;
    unsigned cmd_ladderKey = 0;
    uint8_t status = 0;
    unsigned moduleId = 0;

    BDBG_ENTER( BHSM_KeyLadder_GenerateLevelKey );

    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pkeyLadder->configured ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKey->level < BCMD_KeyRamBuf_eKey3 ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKey->level >= BCMD_KeyRamBuf_eMax ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKey->ladderKeySize > (BHSM_KEYLADDER_LADDER_KEY_SIZE*8)  ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKey->ladderKeySize % 8 ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    pSettings = &pkeyLadder->settings;

    rc = BHSM_BspMsg_Create( pkeyLadder->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, KEYLADDER_GENERATE_KEY, &header );

    switch(pSettings->mode)
    {
    case BHSM_KeyLadderMode_eHwlk:
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderSelection, BCMD_eHWKL );
        break;
    default:
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderSelection, BCMD_eFWKL );
        break;
    }
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKM3DesKLRootKeySwapEnable, pSettings->root.askm.swapKey ? 1 : 0 );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, _MapKeyLadderType(pSettings->algorithm) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRootKeySrc, _MapRootKeySource(pSettings->root.type, pSettings->root.otpKeyIndex) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCustomerSel, _MapKeyLadderMode(pSettings->mode) );

    moduleId =   (pSettings->mode == BHSM_KeyLadderMode_eHwlk) ? pSettings->hwkl.moduleId : _MapModuleId( pSettings->mode );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID, moduleId );

    if( pSettings->root.type == BHSM_KeyLadderRootType_eOtpAskm ||
        pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMSel, 1 );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, (uint8_t)pSettings->root.askm.stbOwnerSelect );
        BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, (uint16_t)pSettings->root.askm.caVendorId );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCAVendorIDExtension, (uint8_t)pSettings->root.askm.caVendorIdExtension );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eTestKeySel, _MapTestKeySel(pSettings->root.askm.caVendorIdScope) );
#ifdef BHSM_BUILD_HSM_FOR_SAGE
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID,   pSettings->root.askm.sageModuleId ? pSettings->root.askm.sageModuleId : moduleId );
#endif

        if( pSettings->root.type == BHSM_KeyLadderRootType_eGlobalKey )
        {
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMGlobalKeyIndex, (uint8_t)pSettings->root.globalKey.index );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, _MapGlobalKeyOwnerIdSelect(pSettings->root.globalKey.owner) );
        }
    }

    if (pKey->level == 3) {
        if( pSettings->root.type == BHSM_KeyLadderRootType_eCustomerKey )
        {
            uint8_t cusKeySelect;

            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzle1IndexSel, (pSettings->root.customerKey.swizzle1IndexSel & 0x1F) );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzleType, pSettings->root.customerKey.type );

            cusKeySelect =  (pSettings->root.customerKey.low.keyIndex & 0x3F);
            cusKeySelect |= (pSettings->root.customerKey.low.decrypt?1:0) << 6;
            cusKeySelect |= (pSettings->root.customerKey.enableSwizzle0a?1:0) << 7;
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelL, cusKeySelect );

            if (pSettings->root.customerKey.enableSwizzle0a) {
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, _MapGlobalKeyOwnerIdSelect(pSettings->root.customerKey.cusKeySwizzle0aVariant) );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKey2GenVersion, (uint8_t)pSettings->root.customerKey.cusKeySwizzle0aVersion );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, (uint8_t)pSettings->root.askm.stbOwnerSelect );
                BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, (uint16_t)pSettings->root.askm.caVendorId );
            }

            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarL, pSettings->root.customerKey.low.keyVar );
            cusKeySelect =  (pSettings->root.customerKey.high.keyIndex & 0x3F);
            cusKeySelect |= (pSettings->root.customerKey.high.decrypt?1:0) << 6;
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelH, cusKeySelect );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarH, pSettings->root.customerKey.high.keyVar );
        }
    }
    else {
        if( pSettings->root.type == BHSM_KeyLadderRootType_eCustomerKey )
        {
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzle1IndexSel, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzleType, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelL, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarL, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKey2GenVersion, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelH, 0 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarH, 0 );
        }
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, pkeyLadder->index );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, pKey->level  );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyTweak, BCMD_KeyTweak_eNoTweak );/*fixed*/
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderOpera, (uint8_t)pSettings->operation?0:1 );

    cmd_ladderKey = BCMD_GenKey_InCmd_eProcIn + GEN_ROUTE_KEY_DATA_LEN - (pKey->ladderKeySize/8);
    BHSM_BspMsg_PackArray( hMsg, cmd_ladderKey, pKey->ladderKey, (pKey->ladderKeySize /8) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySize, _MapKeySize(pKey->ladderKeySize) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyMode, _MapKeyMode(pSettings->keyMode) );

    if( pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotKey ||
        pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv  ||
        pKey->route.destination == BHSM_KeyLadderDestination_eKeyslotIv2 )
    {

        if( !pKey->route.keySlot.handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

        BKNI_Memset( &keySlot, 0, sizeof(keySlot) );
        rc = BHSM_P_Keyslot_GetDetails( pKey->route.keySlot.handle, pKey->route.keySlot.entry, &keySlot );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSC01ModeWordMapping, _MapSc01Mode(keySlot.sc01.useEntry) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eGPipeSC01Value, _MapPolarity(keySlot.sc01.outputPolarity.gPipe) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRPipeSC01Value, _MapPolarity(keySlot.sc01.outputPolarity.rPipe) );

        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord0, keySlot.ctrlWord0 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord1, keySlot.ctrlWord1 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord2, keySlot.ctrlWord2 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord3, keySlot.ctrlWord3 );

        if( keySlot.externalIvValid )
        {
            BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eExtIVPtr, (uint16_t)keySlot.externalIvOffset );
        }

        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySlotType, BHSM_P_MapKeySlotType(keySlot.slotType) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySlotNumber, keySlot.number );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eEntryType, BHSM_P_MapKeySlotEntry(pKey->route.keySlot.entry) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eBlkType, BHSM_P_MapKeySlotBlock(pKey->route.keySlot.entry) );

        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eIVType, _MapDestinationType( pKey->route.destination )  );

        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRouteKeyFlag, 1 );
    }

    if(pSettings->mode == BHSM_KeyLadderMode_eHwlk)
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eHwKlLength, pSettings->hwkl.numlevels );
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eHwKlDestinationAlg, _MapHwKeyLadderType(pSettings->hwkl.algorithm));
    }

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto _exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto _exit;
    }

_exit:

   (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_KeyLadder_GenerateLevelKey );
    return rc;
}


BERR_Code BHSM_KeyLadder_Invalidate( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderInvalidate  *pInvalidate )
{
    BHSM_P_KeyLadder *pKeyLadder = (BHSM_P_KeyLadder*)handle;
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;

    BDBG_ENTER( BHSM_KeyLadder_Invalidate );

    if( !pInvalidate ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pKeyLadder->index >= BCMD_VKL_KeyRam_eMax ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    rc = BHSM_BspMsg_Create( pKeyLadder->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    #define HSM_INVALIDATE_VKL (BCMD_cmdType_eSESSION_INVALIDATE_KEY)
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, HSM_INVALIDATE_VKL, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eVKLID, pKeyLadder->index );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyFlag, BCMD_InvalidateKey_Flag_eSrcKeyOnly );

    if( pInvalidate->clearOwnership ) {
         /* clear all key layers and ownership */
        BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eFreeVKLOwnerShip, 1 );
        BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eAllKeyLayer, 1 );
    }
    else {
        /* clear all key layers */
        BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eAllKeyLayer, 1 );
    }

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP error status[0x%02X]. index[%d] )", BSTD_FUNCTION, status, pKeyLadder->index ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    if( /*pKeyLadder->hHsm->firmwareVersion.bseck.major >= 4 && */ pInvalidate->clearOwnership )
    {
        pKeyLadder->owner = BHSM_SecurityCpuContext_eNone;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_KeyLadder_Invalidate );

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


static BERR_Code _GetKeyLadderOwner( BHSM_Handle hHsm, unsigned index, BHSM_SecurityCpuContext *pOwner )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t allocation = 0;
    uint8_t association = 0;
    uint8_t status;

    BDBG_ENTER( _GetKeyLadderOwner );
    if( index >= MAX_NUM_KEYLADDERS ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY, &header );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLAssociationQuery, BCMD_VKLAssociationQueryFlag_eQuery );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, BCMD_KeyLadderType_e3DESABA );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eNoKeyGenFlag, BCMD_KeyGenFlag_eNoGen );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, index );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, BCMD_KeyRamBuf_eKey3 );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto _exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto _exit;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_GenKey_OutCmd_eVKLAssociation+(index*4), &association );

    if( association == 0xFF )
    {
        *pOwner = BHSM_SecurityCpuContext_eNone;
    }
    else
    {
        BHSM_BspMsg_Get8( hMsg, BCMD_GenKey_OutCmd_eVKLAllocation+(index*4), &allocation );

        switch( allocation ) {
            case 0: { *pOwner = BHSM_SecurityCpuContext_eHost; break; }
            case 1: { *pOwner = BHSM_SecurityCpuContext_eSage; break; }
            default: { BERR_TRACE(BHSM_STATUS_BSP_ERROR); }
        }
    }

    BDBG_MSG(("%s index[%d] association[0x%X] allocation[0x%X]", BSTD_FUNCTION, index, association, allocation ));

_exit:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _GetKeyLadderOwner );
    return BERR_SUCCESS;
}

void BHSM_KeyLadder_GetDefaultChallenge( BHSM_KeyLadderChallenge  *pChallenge )
{
    BDBG_ENTER( BHSM_KeyLadder_GetDefaultChallenge );
    BSTD_UNUSED( pChallenge );
    BDBG_LEAVE( BHSM_KeyLadder_GetDefaultChallenge );
    return;
}

/* Challange the keyladder. Can be used to authenticate the STB. */
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



static uint8_t  _MapKeyLadderType( BHSM_CryptographicAlgorithm algorithm )
{
    switch( algorithm )
    {
        case BHSM_CryptographicAlgorithm_eDes:     { return BCMD_KeyLadderType_e1DES; }
        case BHSM_CryptographicAlgorithm_e3DesAba: { return BCMD_KeyLadderType_e3DESABA; }
        case BHSM_CryptographicAlgorithm_eAes128:  { return BCMD_KeyLadderType_eAES128; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    return BCMD_KeyLadderType_eAES128; /*default to AES */
}


static uint8_t  _MapHwKeyLadderType( BHSM_CryptographicAlgorithm algorithm )
{
    switch( algorithm )
    {
    case BHSM_CryptographicAlgorithm_eDvbCsa2:   { return 0x0; }
    case BHSM_CryptographicAlgorithm_eMulti2:    { return 0x1; }
    case BHSM_CryptographicAlgorithm_eDes:       { return 0x2; }
    case BHSM_CryptographicAlgorithm_e3DesAba:   { return 0x3; }
    case BHSM_CryptographicAlgorithm_e3DesAbc:   { return 0x4; }
    case BHSM_CryptographicAlgorithm_eDvbCsa3:   { return 0x5; }
    case BHSM_CryptographicAlgorithm_eAes128:    { return 0x6; }
    case BHSM_CryptographicAlgorithm_eAes192:    { return 0x7; }
    case BHSM_CryptographicAlgorithm_eC2:       { return 0x9; }
    case BHSM_CryptographicAlgorithm_eM6Ke:     { return 0xb; }
    case BHSM_CryptographicAlgorithm_eM6S:      { return 0xc; }
    case BHSM_CryptographicAlgorithm_eRc4:      { return 0xd; }
    case BHSM_CryptographicAlgorithm_eMsMultiSwapMac: { return 0xe; }
    case BHSM_CryptographicAlgorithm_eWmDrmPd:  { return 0xf; }
    case BHSM_CryptographicAlgorithm_eAes128G:  { return 0x10; }
    case BHSM_CryptographicAlgorithm_eHdDvd:    { return 0x11; }
    case BHSM_CryptographicAlgorithm_eBrDvd:    { return 0x12; }
    case BHSM_CryptographicAlgorithm_eReserved19:{ return 0x13; }
    default:BERR_TRACE( BERR_INVALID_PARAMETER ); /* unsupported algorithm. */
    }
    return 6; /*default to AES */
}

static uint8_t  _MapRootKeySource( BHSM_KeyLadderRootType type, unsigned otpKeyIndex )
{
    switch( type )
    {
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm:
        {
            if( (BCMD_RootKeySrc_eOTPKeya + otpKeyIndex) > BCMD_RootKeySrc_eOTPKeyMax )
            {
                BERR_TRACE( BERR_INVALID_PARAMETER );
                return BCMD_RootKeySrc_eOTPKeya; /* return a random source. */
            }
            return  BCMD_RootKeySrc_eOTPKeya + otpKeyIndex;
            break;
        }
        case BHSM_KeyLadderRootType_eGlobalKey: { return BCMD_RootKeySrc_eASKMGlobalKey; }
        case BHSM_KeyLadderRootType_eCustomerKey: { return BCMD_RootKeySrc_eCusKey; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return BCMD_RootKeySrc_eASKMGlobalKey; /*default to global.*/
}



static uint8_t  _MapKeyLadderMode( BHSM_KeyLadderMode mode ) /* Customer Sub mode */
{
    switch( mode )
    {
        case BHSM_KeyLadderMode_eCa_64_4:          { return BCMD_CustomerSubMode_eGeneric_CA_64_4; }
        case BHSM_KeyLadderMode_eCp_64_4:          { return BCMD_CustomerSubMode_eGeneric_CP_64_4; }
        case BHSM_KeyLadderMode_eCa_64_5:          { return BCMD_CustomerSubMode_eGeneric_CA_64_5; }
        case BHSM_KeyLadderMode_eCp_64_5:          { return BCMD_CustomerSubMode_eGeneric_CP_64_5; }
        case BHSM_KeyLadderMode_eCa_128_4:         { return BCMD_CustomerSubMode_eGeneric_CA_128_4; }
        case BHSM_KeyLadderMode_eCp_128_4:         { return BCMD_CustomerSubMode_eGeneric_CP_128_4; }
        case BHSM_KeyLadderMode_eCa_128_5:         { return BCMD_CustomerSubMode_eGeneric_CA_128_5; }
        case BHSM_KeyLadderMode_eCp_128_5:         { return BCMD_CustomerSubMode_eGeneric_CP_128_5; }
        case BHSM_KeyLadderMode_eCa_64_7:          { return BCMD_CustomerSubMode_eGeneric_CA_64_7; }
        case BHSM_KeyLadderMode_eCa_128_7:         { return BCMD_CustomerSubMode_eGeneric_CA_128_7; }
        case BHSM_KeyLadderMode_eCa64_45:          { return BCMD_CustomerSubMode_eGeneric_CA_64_45; }
        case BHSM_KeyLadderMode_eCp64_45:          { return BCMD_CustomerSubMode_eGeneric_CP_64_45; }
        case BHSM_KeyLadderMode_eSage128_5:        { return BCMD_CustomerSubMode_eReserved12;}
        case BHSM_KeyLadderMode_eSage128_4:        { return BCMD_CustomerSubMode_eReserved30;}
        case BHSM_KeyLadderMode_eSageBlDecrypt:    { return BCMD_CustomerSubMode_eSAGE_BL_DECRYPT; }
        case BHSM_KeyLadderMode_eGeneralPurpose1:  { return BCMD_CustomerSubMode_eGeneralPurpose1; }
        case BHSM_KeyLadderMode_eGeneralPurpose2:  { return BCMD_CustomerSubMode_eGeneralPurpose2; }
        case BHSM_KeyLadderMode_eEtsi_5:           { return BCMD_CustomerSubMode_eETSI_5; }
        case BHSM_KeyLadderMode_eScte52Ca_5:       { return BCMD_CustomerSubMode_eSCTE52_CA_5; }
        case BHSM_KeyLadderMode_eHwlk:             { return BCMD_CustomerSubMode_eHWKL; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return (uint8_t)mode; /* pass through what was passed in.  */
}

static uint8_t  _MapModuleId( BHSM_KeyLadderMode mode ) /* Get moduleId from ladder mode. */
{
    switch( mode )
    {
        case BHSM_KeyLadderMode_eCa_64_4:          { return 3; }
        case BHSM_KeyLadderMode_eCp_64_4:          { return 4; }
        case BHSM_KeyLadderMode_eCa_64_5:          { return 5; }
        case BHSM_KeyLadderMode_eCp_64_5:          { return 6; }
        case BHSM_KeyLadderMode_eCa_128_4:         { return 7; }
        case BHSM_KeyLadderMode_eCp_128_4:         { return 8; }
        case BHSM_KeyLadderMode_eCa_128_5:         { return 9; }
        case BHSM_KeyLadderMode_eCp_128_5:         { return 10; }
        case BHSM_KeyLadderMode_eCa_64_7:          { return 11; }
        case BHSM_KeyLadderMode_eCa_128_7:         { return 12; }
        case BHSM_KeyLadderMode_eCa64_45:          { return 17; }
        case BHSM_KeyLadderMode_eCp64_45:          { return 18; }
        case BHSM_KeyLadderMode_eSage128_5:        { return 13; }
        case BHSM_KeyLadderMode_eSage128_4:        { return 29; }
        case BHSM_KeyLadderMode_eSageBlDecrypt:    { return 14; }
        case BHSM_KeyLadderMode_eGeneralPurpose1:  { return 15; }
        case BHSM_KeyLadderMode_eGeneralPurpose2:  { return 16; }
        case BHSM_KeyLadderMode_eEtsi_5:           { return 19; }
        case BHSM_KeyLadderMode_eScte52Ca_5:       { return 24; }
        case BHSM_KeyLadderMode_eHwlk:             { return 0xFF; } /* don't care. */
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return (uint8_t)mode; /* pass through what was passed in. */
}




static uint8_t  _MapGlobalKeyOwnerIdSelect( BHSM_KeyLadderGlobalKeyOwnerIdSelect owner )
{
    switch( owner )
    {
        case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0: { return BCMD_OwnerIDSelect_eMSP0; }
        case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1: { return BCMD_OwnerIDSelect_eMSP1; }
        case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne:  { return BCMD_OwnerIDSelect_eUse1; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    return owner;
}

static uint8_t  _MapKeySize( unsigned sizeInBits  )
{
  switch( sizeInBits ) {
        case 64:   { return 0; }
        case 128:  { return 1; }
        case 192:  { return 2; }
        case 256:  { return 3; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    return (uint8_t)sizeInBits;
}


static uint8_t  _MapTestKeySel( BHSM_KeyladderCaVendorIdScope  scope )
{
    switch( scope )
    {
        case BHSM_KeyladderCaVendorIdScope_eChipFamily: { return 0; }
        case BHSM_KeyladderCaVendorIdScope_eFixed:      { return 2; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return (uint8_t)scope;
}


static uint8_t _MapKeyMode( BHSM_KeyLadderKeyMode keyMode )
{
    switch( keyMode )
    {
        case BHSM_KeyLadderKeyMode_eRegular:        { return 0; }
        case BHSM_KeyLadderKeyMode_eDes56:          { return 1; }
        case BHSM_KeyLadderKeyMode_eDvbConformance: { return 4; }
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return (uint8_t)keyMode;
}


static uint8_t _MapSc01Mode( BHSM_KeyslotPolarity useEntry )
{
    switch( useEntry )
    {
        case BHSM_KeyslotPolarity_eClear: { return 0;}
        case BHSM_KeyslotPolarity_eOdd:   { return 1;}
        case BHSM_KeyslotPolarity_eEven:  { return 2;}
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    return (uint8_t)useEntry;
}

static uint8_t _MapPolarity( BHSM_KeyslotPolarity polarity )
{
    switch( polarity )
    {
        case BHSM_KeyslotPolarity_eClear:     { return 0;}
        case BHSM_KeyslotPolarity_eReserved:  { return 1;}
        case BHSM_KeyslotPolarity_eEven:      { return 2;}
        case BHSM_KeyslotPolarity_eOdd:       { return 3;}
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }
    return (uint8_t)polarity;
}

static uint8_t _MapDestinationType( BHSM_KeyLadderDestination destination )
{
    switch( destination )
    {
        case BHSM_KeyLadderDestination_eKeyslotKey: return 0;
        case BHSM_KeyLadderDestination_eKeyslotIv: return 1;
        case BHSM_KeyLadderDestination_eKeyslotIv2: return 2;
        case BHSM_KeyLadderDestination_eNone:
        default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    return 0;
}
