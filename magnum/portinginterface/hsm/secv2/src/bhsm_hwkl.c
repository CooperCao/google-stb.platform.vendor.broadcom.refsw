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
#include "bhsm_keyslot.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_keyladder_priv.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_types.h"
#include "bhsm_p_keyslot.h"
#include "bhsm_p_hwkl.h"
#include "bhsm_hwkl.h"

BDBG_MODULE(BHSM);

/*
Description:
    Route a key to the the keyslot from HW Keyladder
*/
BERR_Code BHSM_Keyslot_RouteHWKlEntryKey ( BHSM_KeyslotHandle handle,
                                      BHSM_KeyslotBlockEntry entry,  /* block (cps/ca/cpd) and entry (odd/even/clear) */
                                      const BHSM_KeyslotRouteKey *pKey )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_HwklRouteKey bspConfig;
    BHSM_KeyslotDetails details;

    BDBG_ENTER( BHSM_Keyslot_RouteHWKlEntryKey );

    if( !handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    rc = BHSM_P_Keyslot_GetDetails( handle, entry, &details );
    if(rc != BERR_SUCCESS) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    bspConfig.in.blockType = details.blockType;
    bspConfig.in.entryType = details.polarity;

    bspConfig.in.keySlotType = BHSM_P_ConvertSlotType(details.slotType);
    bspConfig.in.keySlotNumber = details.number;

    bspConfig.in.keyMode = 0 /*Bsp_KeyMode_eRegular */;

    bspConfig.in.modeWords[0] = details.ctrlWord0;
    bspConfig.in.modeWords[1] = details.ctrlWord1;
    bspConfig.in.modeWords[2] = details.ctrlWord2;
    bspConfig.in.modeWords[3] = details.ctrlWord3;

    if(details.externalIvValid)
    {
        bspConfig.in.extIvPtr = details.externalIvOffset;
    }

    bspConfig.in.keyLayer = pKey->keyLadderLayer;

    rc = BHSM_P_Hwkl_RouteKey( details.hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Keyslot_RouteHWKlEntryKey );
    return BERR_SUCCESS;
}

BERR_Code  BHSM_Keyslot_GenerateHwKlRootKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BHSM_P_HwklRootConfig bspConfig;
    BHSM_KeyLadderSettings settings;
    BHSM_KeyLadderDetails_t keyLadderDetails;
    BERR_Code rc = BERR_SUCCESS;
    unsigned keyOffset = 0;

    BDBG_ENTER( BHSM_Keyslot_GenerateHwKlRootKey );

    if( !handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_P_KeyLadder_GetDetails( handle, &keyLadderDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    if( !keyLadderDetails.configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BHSM_KeyLadder_GetSettings( handle, &settings );

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.moduleId =  settings.hwkl.moduleId;
    bspConfig.in.hwklLength =  settings.hwkl.numlevels;
    bspConfig.in.hwklDestinationAlg = BHSM_P_Map2KeySlotCryptoAlg( settings.hwkl.algorithm );

    switch( settings.root.type ) {
        case BHSM_KeyLadderRootType_eCustomerKey: {
            bspConfig.in.rootKeySrc = 0;  /*Bsp_RootKeySrc_eCusKey*/
            break;
        }
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm: {
            bspConfig.in.rootKeySrc = settings.root.otpKeyIndex+1;
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

    if( ( settings.root.type == BHSM_KeyLadderRootType_eOtpAskm ) ||
        ( settings.root.type == BHSM_KeyLadderRootType_eGlobalKey ) )
    {
        bspConfig.in.caVendorId            = (uint16_t)settings.root.askm.caVendorId;
        bspConfig.in.askmTdesKlRootKeySwapEnable = settings.root.askm.swapKey?1:0;
        bspConfig.in.stbOwnerIdSel         = (uint8_t)settings.root.askm.stbOwnerSelect;
        switch( settings.root.askm.caVendorIdScope )
        {
            case BHSM_KeyLadderCaVendorIdScope_eChipFamily: bspConfig.in.askmMaskKeySel = 0; break;
            case BHSM_KeyLadderCaVendorIdScope_eFixed:      bspConfig.in.askmMaskKeySel = 2; break;
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

        if( settings.root.type == BHSM_KeyLadderRootType_eGlobalKey )
        {
            bspConfig.in.globalKeyIndex = settings.root.globalKey.index;

            switch( settings.root.globalKey.owner )
            {
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0: { bspConfig.in.globalKeyOwnerIdSelect = 0;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1: { bspConfig.in.globalKeyOwnerIdSelect = 1;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne:  { bspConfig.in.globalKeyOwnerIdSelect = 2;  break; }
                default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
            }
        }
    }

    keyOffset = (sizeof(bspConfig.in.procIn)-(pKey->ladderKeySize/8))/sizeof(uint32_t);

    if( keyOffset*4 >= sizeof(bspConfig.in.procIn) ||
         keyOffset*4 + pKey->ladderKeySize/8 > sizeof(bspConfig.in.procIn) ) {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    rc = BHSM_MemcpySwap( &bspConfig.in.procIn[keyOffset], pKey->ladderKey, pKey->ladderKeySize/8 );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    rc = BHSM_P_Hwkl_RootConfig( keyLadderDetails.hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Keyslot_GenerateHwKlRootKey );
    return BERR_SUCCESS;
}

BERR_Code  BHSM_Keyslot_GenerateHwKlLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey *pKey )
{
    BHSM_P_HwklLayerSet bspConfig;
    BHSM_KeyLadderSettings settings;
    BHSM_KeyLadderDetails_t keyLadderDetails;
    BERR_Code rc = BERR_SUCCESS;
    unsigned keyOffset = 0;

    BDBG_ENTER( BHSM_Keyslot_GenerateHwKlLevelKey );

    if( !handle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKey ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_P_KeyLadder_GetDetails( handle, &keyLadderDetails );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    if( !keyLadderDetails.configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BHSM_KeyLadder_GetSettings( handle, &settings );

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.destinationKeyLayer = pKey->level;

    keyOffset = (sizeof(bspConfig.in.procIn)-(pKey->ladderKeySize/8))/sizeof(uint32_t);
    if( keyOffset*4 >= sizeof(bspConfig.in.procIn) ||
             keyOffset*4 + pKey->ladderKeySize/8 > sizeof(bspConfig.in.procIn) ) {
             return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    rc = BHSM_MemcpySwap( &bspConfig.in.procIn[keyOffset], pKey->ladderKey, pKey->ladderKeySize/8 );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    rc = BHSM_P_Hwkl_LayerSet( keyLadderDetails.hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Keyslot_GenerateHwKlLevelKey );
    return BERR_SUCCESS;
}
