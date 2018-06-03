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
#include "priv/nexus_core.h"
#include "priv/nexus_security_priv.h"
#include "bhsm_signed_command.h"

BDBG_MODULE(nexus_signed_command);

#define _COMMAND_SIZE      (364)            /* (96 - 5 ) * 4 */
#define _METADATA_OFFSET   (_COMMAND_SIZE)
#define _METADATA_SIZE     (44)             /* 11 * 4 */
#define _SIGNATURE_OFFSET  (_METADATA_OFFSET+_METADATA_SIZE)
#define _SIGNATURE_SIZE    (256)
#define _RSA_CONFIG_OFFSET (_SIGNATURE_OFFSET + _SIGNATURE_SIZE)


void NEXUS_GetDefaultSignedCommand( NEXUS_SignedCommand *pSignedCommand )
{
    BDBG_ENTER( NEXUS_GetDefaultSignedCommand );

    if ( !pSignedCommand ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pSignedCommand, 0, sizeof(*pSignedCommand) );

    BDBG_LEAVE( NEXUS_GetDefaultSignedCommand );
    return;
}

/*
    This function:
        - loads the RSA key via the NEXUS_RegVerRsa_* interface.
        - submits the signed command for verification against the loaded RSA key.
            - here the command signature is copied in to allocated device memory where it can be
              accessed from the BFW.
*/

NEXUS_Error NEXUS_SetSignedCommand( const NEXUS_SignedCommand *pSignedCommand )
{
   #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(5,0)
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BERR_Code rcHsm = BERR_UNKNOWN;
    BHSM_Handle hHsm;
    BHSM_SignedCommand hsmSignedCommand;
    NEXUS_MemoryAllocationSettings memSetting;
    NEXUS_RegVerRsaHandle rsaHandle = NULL;
    NEXUS_RegVerRsaAllocateSettings rsaAllocSettings;
    NEXUS_RegVerRsaSettings rsaSettings;
    uint8_t *pMetadataAndSignature = NULL;   /* The command Metadata and signature is allocated from NEXUS/device memory. */
    const unsigned rsaKeyId = 2;  /* the RSA keyslot to use. */

    BDBG_ENTER( NEXUS_SetSignedCommand );

    if ( !pSignedCommand ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    /* number check! */
    BDBG_CASSERT( NEXUS_SIGNED_COMMAND_SIZE == (_RSA_CONFIG_OFFSET + NEXUS_REGVER_RSA_KEY_SIZE) );

    NEXUS_RegVerRsa_GetDefaultAllocateSettings( &rsaAllocSettings );
    rsaAllocSettings.rsaKeyId = rsaKeyId;
    rsaHandle = NEXUS_RegVerRsa_Allocate( &rsaAllocSettings );
    if( !rsaHandle ) { BERR_TRACE( NEXUS_NOT_AVAILABLE ); goto error; }

    BKNI_Memset( &rsaSettings, 0, sizeof(rsaSettings) );
    rsaSettings.rootKey = pSignedCommand->signingAuthority;
    rsaSettings.multiTier = false;
    BKNI_Memcpy( rsaSettings.rsaKey,
                 &pSignedCommand->signedCommand[_RSA_CONFIG_OFFSET],
                 NEXUS_REGVER_RSA_KEY_SIZE );

    rc = NEXUS_RegVerRsa_SetSettings( rsaHandle, &rsaSettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto error; }

    /* allocate memory for signature. */
    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
    memSetting.alignment = 32;
    memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( (_METADATA_SIZE + _SIGNATURE_SIZE),
                                &memSetting,
                                (void**)&pMetadataAndSignature );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY ); goto error; }

    BKNI_Memcpy( pMetadataAndSignature,
                 &pSignedCommand->signedCommand[_METADATA_OFFSET],
                 (_METADATA_SIZE + _SIGNATURE_SIZE) );

    NEXUS_FlushCache( (const void*)pMetadataAndSignature, (_METADATA_SIZE + _SIGNATURE_SIZE) );

    BKNI_Memset( &hsmSignedCommand, 0, sizeof(hsmSignedCommand) );
    hsmSignedCommand.rsaKeyId = rsaKeyId;
    hsmSignedCommand.signatureOffset = NEXUS_AddrToOffset( pMetadataAndSignature );
    hsmSignedCommand.pCommand = &pSignedCommand->signedCommand[0];

    rcHsm = BHSM_SignedCommand_Set( hHsm, &hsmSignedCommand );
    if( rcHsm != BERR_SUCCESS ) { rc = BERR_TRACE( rcHsm ); goto error; }

error:

    if( rsaHandle ) { NEXUS_RegVerRsa_Free( rsaHandle ); }
    if( pMetadataAndSignature ) { NEXUS_Memory_Free( pMetadataAndSignature ); }

    BDBG_LEAVE( NEXUS_SetSignedCommand );
    return rc;
   #else
    BSTD_UNUSED( pSignedCommand );
    return NEXUS_NOT_SUPPORTED;
   #endif
}
