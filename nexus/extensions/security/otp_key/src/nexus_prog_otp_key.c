/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "nexus_security_module.h"
#include "nexus_prog_otp_key.h"
#include "bhsm_otp_key.h"

BDBG_MODULE ( nexus_prog_otp_key );

NEXUS_Error NEXUS_Security_GetDefaultProgramOtpKey ( NEXUS_ProgramOtpKey * pOtpKey )
{

    BDBG_ENTER ( NEXUS_Security_GetDefaultProgramOtpKey );

    if ( !pOtpKey )
    {
        BDBG_ERR ( ( "NULL parameter pointer." ) );
        return BERR_TRACE ( NEXUS_INVALID_PARAMETER );
    }

    pOtpKey->keyType = NEXUS_OtpKeyType_eA;
    pOtpKey->keyLayer = NEXUS_SecurityKeySource_eKey3;

    /* Setting invalid data for client to fill in the correct values. */
    pOtpKey->virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    pOtpKey->keyDataSize = 0;

    BDBG_LEAVE ( NEXUS_Security_GetDefaultProgramOtpKey );

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_ProgramOtpKey ( const NEXUS_ProgramOtpKey * pOtpKey )
{
    BERR_Code       rc;
    BHSM_Handle     hHsm;
    BHSM_ProgramOtpKey_t progOtpKey;

    BDBG_ENTER ( NEXUS_Security_ProgramOtpKey );

    if ( !pOtpKey )
    {
        BDBG_ERR ( ( "NULL parameter pointer." ) );
        return BERR_TRACE ( NEXUS_INVALID_PARAMETER );
    }

    if ( pOtpKey->keyDataSize > NEXUS_MAX_OTP_KEY_LENGTH )
    {
        BDBG_ERR ( ( "Invalide key data size %d.", pOtpKey->keyDataSize ) );
        return BERR_TRACE ( NEXUS_INVALID_PARAMETER );
    }

    NEXUS_Security_GetHsm_priv ( &hHsm );

    if ( !hHsm )
    {
        BDBG_ERR ( ( "HSM module has not been opened." ) );
        return BERR_TRACE ( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    progOtpKey.keyType = pOtpKey->keyType;
    progOtpKey.keyLayer = pOtpKey->keyLayer;
    progOtpKey.virtualKeyLadderID = pOtpKey->virtualKeyLadderID;
    progOtpKey.keyDataSize = pOtpKey->keyDataSize;

    BKNI_Memcpy ( &progOtpKey.keyData, &pOtpKey->keyData, pOtpKey->keyDataSize );

    rc = BHSM_ProgOTPKey ( hHsm, &progOtpKey );
    if ( rc != BERR_SUCCESS )
    {
        return BERR_TRACE ( NEXUS_INVALID_PARAMETER );
    }

    BDBG_LEAVE ( NEXUS_Security_ProgramOtpKey );

    return NEXUS_SUCCESS;
}
