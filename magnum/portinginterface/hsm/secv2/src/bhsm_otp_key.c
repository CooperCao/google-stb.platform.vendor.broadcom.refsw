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

#include "bhsm_priv.h"
#include "bhsm.h"
#include "bhsm_otp_key.h"
#include "bsp_types.h"
#include "bhsm_p_otpmode0.h"
#include "bhsm_otp_msp.h"
#include "bhsm_otp_msp_indexes.h"

BDBG_MODULE( BHSM );

typedef struct BHSM_OtpKeyModule
{
    BHSM_OtpKeyInfo KeyInfo[Bsp_Otp_KeyType_eMax];
    bool cached[Bsp_Otp_KeyType_eMax];
}BHSM_OtpKeyModule;


static bool _isHashValid( BHSM_Handle hHsm, unsigned index ); /* checks if the hash for the specified key is accessible. */


BERR_Code BHSM_OtpKey_Init( BHSM_Handle hHsm, BHSM_OtpKeyModuleSettings *pSettings )
{

    BDBG_ENTER( BHSM_OtpKey_Init );
    BSTD_UNUSED( pSettings );

    hHsm->modules.pOtpKey = BKNI_Malloc( sizeof(BHSM_OtpKeyModule) );
    if( !hHsm->modules.pOtpKey ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pOtpKey, 0, sizeof(BHSM_OtpKeyModule) );



    BDBG_LEAVE( BHSM_OtpKey_Init );
    return BERR_SUCCESS;
}


void BHSM_OtpKey_Uninit( BHSM_Handle hHsm )
{

    BDBG_ENTER( BHSM_OtpKey_Uninit );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !hHsm->modules.pOtpKey ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return; }

    BKNI_Memset( hHsm->modules.pOtpKey, 0, sizeof(BHSM_OtpKeyModule) );
    BKNI_Free( hHsm->modules.pOtpKey );
    hHsm->modules.pOtpKey = NULL;

    BDBG_LEAVE( BHSM_OtpKey_Uninit );
    return;
}

BERR_Code BHSM_OtpKey_GetInfo( BHSM_Handle hHsm, BHSM_OtpKeyInfo *pKeyInfo )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_OtpMode0KeyMc0Read keyRead;
    BDBG_ENTER( BHSM_OtpKey_GetInfo );

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKeyInfo ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKeyInfo->index >= Bsp_Otp_KeyType_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if(hHsm->modules.pOtpKey->cached[pKeyInfo->index] == false)
    {
        /* KeyHash */
        pKeyInfo->hashValid = _isHashValid( hHsm, pKeyInfo->index );
        if( pKeyInfo->hashValid )
        {
            BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
            keyRead.in.sectionIndex = pKeyInfo->index;
            keyRead.in.field = Bsp_Otp_KeyField_eKeyHash;
            rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
            if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

            rc = BHSM_MemcpySwap( &pKeyInfo->hash[4], &keyRead.out.data[0], 4 );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

            rc = BHSM_MemcpySwap( &pKeyInfo->hash[0], &keyRead.out.data[1], 4 );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
        }

        /* KeyId */
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eKeyID;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        rc = BHSM_MemcpySwap( &pKeyInfo->id[4], &keyRead.out.data[0], 4 );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        rc = BHSM_MemcpySwap( &pKeyInfo->id[0], &keyRead.out.data[1], 4 );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        /* blackBoxId */
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0BlackBoxId;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        rc = BHSM_MemcpySwap( &pKeyInfo->blackBoxId, &keyRead.out.data[0], 4 );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        /* ca Keyladder allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0CaKeyLadderDisallow;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->caKeyLadderAllow = true;

        /* cp Keyladder allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0CpKeyLadderDisallow;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->cpKeyLadderAllow = true;

        /* gp1 Keyladder allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0Gp1KeyLadderDisallow;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->gp1KeyLadderAllow = true;

        /* gp2 Keyladder allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0Gp2KeyLadderDisallow;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->gp2KeyLadderAllow = true;

        /* sage Keyladder allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0SageKeyLadderDisallow ;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->sageKeyLadderAllow = true;

        /* rootKeySwap allowed*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0RootKeySwapDisallow ;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !keyRead.out.data[0] ) pKeyInfo->rootKeySwapAllow = true;

        /* fixedDeobfuscationEnabled */
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0FixedDeobfuscationVariantEnable;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( keyRead.out.data[0] ) pKeyInfo->fixedDeobfuscationEnabled = true;

        /* deobfuscationEnabled */
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0DeobfuscationEnable;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( keyRead.out.data[0] ) pKeyInfo->deobfuscationEnabled = true;

        /* customer Mode*/
        BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
        keyRead.in.sectionIndex = pKeyInfo->index;
        keyRead.in.field = Bsp_Otp_KeyField_eMc0CustomerMode;
        rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        pKeyInfo->customerMode = keyRead.out.data[0];

        hHsm->modules.pOtpKey->cached[pKeyInfo->index] = true;

        BKNI_Memcpy(&hHsm->modules.pOtpKey->KeyInfo[pKeyInfo->index],pKeyInfo, sizeof(*pKeyInfo));
    }
    else
    {
        BKNI_Memcpy(pKeyInfo, &hHsm->modules.pOtpKey->KeyInfo[pKeyInfo->index], sizeof(*pKeyInfo));
    }

BHSM_P_DONE_LABEL:

    if( rc != BERR_SUCCESS )
    {
        hHsm->modules.pOtpKey->cached[pKeyInfo->index] = false;
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
    }

    BDBG_LEAVE( BHSM_OtpKey_GetInfo );
    return rc;
}



BERR_Code BHSM_OtpKey_Program( BHSM_Handle hHsm, const BHSM_OtpKeyProgram *pKeyConfig )
{

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pKeyConfig ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pKeyConfig->index >= Bsp_Otp_KeyType_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    hHsm->modules.pOtpKey->cached[pKeyConfig->index] = false;

    return 0;
}

/* check MSP to see if we can read the key Hash */
static bool _isHashValid( BHSM_Handle hHsm,  unsigned index )
{
    unsigned mspIndex = 0xFFFFF;
    BHSM_OtpMspRead mspRead;
    BERR_Code rc = BERR_UNKNOWN;

    switch( index ){
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_A
        case 0: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_A; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_B
        case 1: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_B; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_C
        case 2: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_C; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_D
        case 3: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_D; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_E
        case 4: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_E; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_F
        case 5: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_F; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_G
        case 6: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_G; break; }
       #endif
       #ifdef BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_H
        case 7: { mspIndex = BHSM_OTPMSP_DESTINATION_DISALLOW_KEY_H; break; }
       #endif
        default: { return false; } /* we don't have access to the key's MSP */
    }

    BKNI_Memset( &mspRead, 0, sizeof(mspRead) );
    mspRead.index = mspIndex;

    rc = BHSM_OtpMsp_Read( hHsm, &mspRead );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return false; }

    #define BHSM_HASH_DISALLOWED_BIT 5

    if( mspRead.valid & (1<<BHSM_HASH_DISALLOWED_BIT) &&
        mspRead.data  & (1<<BHSM_HASH_DISALLOWED_BIT) )
    {
        return false;
    }

    return true;  /* it available. */
}
