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

#include "nexus_security_module.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bhsm_bseck.h"

BDBG_MODULE(nexus_security);


void NEXUS_Security_GetDefaultVerifySecondTierKeySettings(
    NEXUS_SecurityVerifySecondTierKeySettings  *pSettings
    )
{
    if( pSettings == NULL )
    {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_SecurityVerifySecondTierKeySettings));

    pSettings->firstTierRootKeySource  = NEXUS_SecurityFirstTierKeyID_eKey0Prime;
    pSettings->secondTierRootKeySource = NEXUS_SecuritySecondTierKeyID_eKey2;
    pSettings->keyDestination          = NEXUS_SecuritySecondTierKeyID_eKey3;

    return;
}

NEXUS_Error NEXUS_Security_VerifySecondTierKey(
    const NEXUS_SecurityVerifySecondTierKeySettings  *p2ndTierKeySettings
    )

{
    BHSM_Handle hHsm;
    BHSM_VerifySecondTierKeyIO_t verifyIO;

    NEXUS_Security_GetHsm_priv (&hHsm);
    if( hHsm == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset(&verifyIO, 0, sizeof(BHSM_VerifySecondTierKeyIO_t));

    if( p2ndTierKeySettings->secondTierRootKeySource == NEXUS_SecuritySecondTierKeyID_eNone )
    {
        verifyIO.bMultiTierRootKeySrc  = false;
    }
    else
    {
        verifyIO.bMultiTierRootKeySrc  = false;
        verifyIO.eSecondTierRootKeySrc = (BCMD_SecondTierKeyId_e)(p2ndTierKeySettings->secondTierRootKeySource - 1);
    }
    verifyIO.eFirstTierRootKeySrc  = (BCMD_FirstTierKeyId_e)p2ndTierKeySettings->firstTierRootKeySource;
    verifyIO.eKeyIdentifier        = (BCMD_SecondTierKeyId_e)p2ndTierKeySettings->keyDestination;
    verifyIO.keyAddr               = (unsigned)p2ndTierKeySettings->keyAddress;

    if( BHSM_VerifySecondTierKey( hHsm, &verifyIO ) != BERR_SUCCESS )
    {
        return  BERR_TRACE( NEXUS_NOT_SUPPORTED );
    }

    return NEXUS_SUCCESS;
}


void NEXUS_Security_GetDefaultVerifySecondStageCodeSettings(
    NEXUS_SecurityVerifySecondStageCodeSettings  *pSettings
    )
{
    if( pSettings == NULL )
    {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    BKNI_Memset( pSettings, 0, sizeof(NEXUS_SecurityVerifySecondStageCodeSettings) );

    return;
}


NEXUS_Error NEXUS_Security_VerifySecondStageCode(
    const NEXUS_SecurityVerifySecondStageCodeSettings  *pCodeVerifySettings
    )
{
    BHSM_Handle hHsm;
    BHSM_VerifySecondStageCodeLoadIO_t verifyIO;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( hHsm == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &verifyIO, 0, sizeof(BHSM_VerifySecondStageCodeLoadIO_t) );

    verifyIO.codeLocation   = (BHSM_2ndStageCodeLocation_e)pCodeVerifySettings->codeLocation;
    verifyIO.codePtr        = (unsigned)pCodeVerifySettings->codeAddress;
    verifyIO.codeSigPtr     = (unsigned)pCodeVerifySettings->signatureAddress;
    verifyIO.codeVerifyOpt  = (BHSM_2ndStageCodeVerify_e)pCodeVerifySettings->verifierMode;
    verifyIO.eKeySelect     = (BCMD_SecondTierKeyId_e)pCodeVerifySettings->verifierKey;

    if( BHSM_VerifySecondStageCodeLoad( hHsm, &verifyIO ) != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_NOT_SUPPORTED );
    }

    return NEXUS_SUCCESS;
}
