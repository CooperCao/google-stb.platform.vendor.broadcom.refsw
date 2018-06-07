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
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_keyladder_priv.h"
#include "nexus_keyladder.h"
#include "nexus_otp_key.h"
#include "bhsm_otp_key.h"

BDBG_MODULE( nexus_otp_key );

NEXUS_Error NEXUS_OtpKey_GetInfo( unsigned index, NEXUS_OtpKeyInfo *pKeyInfo )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_OtpKeyInfo keyInfo;
    BHSM_Handle hHsm;

    BDBG_ENTER( NEXUS_OtpKey_GetInfo );

    if( !pKeyInfo ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    BKNI_Memset( &keyInfo, 0, sizeof(keyInfo) );
    keyInfo.index = index;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    rc = BHSM_OtpKey_GetInfo( hHsm, &keyInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    BKNI_Memset( pKeyInfo, 0, sizeof(*pKeyInfo) );

    /* copy data back */
    if( keyInfo.hashValid ){
        pKeyInfo->hashValid = true;
        BKNI_Memcpy( pKeyInfo->hash, keyInfo.hash, sizeof(pKeyInfo->hash) );
    }
    BKNI_Memcpy( pKeyInfo->id, keyInfo.id, sizeof(pKeyInfo->id) );
    pKeyInfo->blackBoxId        = keyInfo.blackBoxId;
    pKeyInfo->caKeyLadderAllow  = keyInfo.caKeyLadderAllow;
    pKeyInfo->cpKeyLadderAllow  = keyInfo.cpKeyLadderAllow;
    pKeyInfo->gp1KeyLadderAllow = keyInfo.gp1KeyLadderAllow;
    pKeyInfo->gp2KeyLadderAllow = keyInfo.gp2KeyLadderAllow;
    pKeyInfo->sageKeyLadderAllow = keyInfo.sageKeyLadderAllow;
    pKeyInfo->rootKeySwapAllow   = keyInfo.rootKeySwapAllow;
    pKeyInfo->deobfuscationEnabled = keyInfo.deobfuscationEnabled;
    pKeyInfo->fixedDeobfuscationEnabled = keyInfo.fixedDeobfuscationEnabled;
    pKeyInfo->customerMode      = keyInfo.customerMode;

    BDBG_LEAVE( NEXUS_OtpKey_GetInfo );
    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_OtpKey_Program( unsigned index, const NEXUS_OtpKeyProgram * pKeyConfig )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_OtpKeyProgram hsmKeyConfig;
    NEXUS_KeyLadderInfo_priv keyladderInfo;
    BHSM_Handle hHsm;

    BDBG_ENTER( NEXUS_OtpKey_Program );

    BKNI_Memset( &hsmKeyConfig, 0, sizeof(hsmKeyConfig) );

    BDBG_CASSERT( sizeof(pKeyConfig->keyData) == sizeof(hsmKeyConfig.keyData) );
    BKNI_Memcpy( hsmKeyConfig.keyData, pKeyConfig->keyData, sizeof(hsmKeyConfig.keyData) );

    NEXUS_KeyLadder_GetInfo_priv( pKeyConfig->keyladder.handle, &keyladderInfo );
    hsmKeyConfig.index = index;
    hsmKeyConfig.keyladder.handle = keyladderInfo.hsmKeyLadderHandle;
    hsmKeyConfig.keyladder.level = pKeyConfig->keyladder.level;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    rc = BHSM_OtpKey_Program( hHsm, &hsmKeyConfig );
    if( rc != BERR_SUCCESS ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BDBG_LEAVE( NEXUS_OtpKey_Program );
    return NEXUS_SUCCESS;
}
