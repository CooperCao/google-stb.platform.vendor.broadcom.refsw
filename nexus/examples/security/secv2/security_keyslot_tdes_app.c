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

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION==2)

#include "nexus_security.h"
#include "nexus_keyslot.h"
#include "security_utils.h"
#include "security_test_vectors_clear_key.h"

int main(
    int argc,
    char **argv )
{
    NEXUS_KeySlotHandle keyslotHandle = NULL;

    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotKey slotKey;
    unsigned      data_size, key_size;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eMax;
    uint8_t      *pSrc = NULL, *pDest = NULL;
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* Allocate a key slot. */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerEntry;
    keyslotAllocSettings.useWithDma = true;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_ERR(( "\nError: Can't allocate keyslot\n" ));
        return -1;
    }

    /* Configure the keyslot. */
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    /* Configure a key entry to encrypt. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_e3DesAba;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb; /* simple Ecb mode. */
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BDBG_LOG( ( "Loads the clear key to key entry #%d.\n", entry ) );

    /* The actual size for the algorithm selected. */
    key_size = securityGetAlogrithmKeySize( keyslotEntrySettings.algorithm );
    slotKey.size = key_size;
    BKNI_Memcpy( slotKey.key, clearKey_ABA, key_size );

    rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );
    if( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    /* Using clear key as the root key to encrypt the plain text, cipher is pDest. */
    data_size = sizeof( plainText );

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }
    else {
        if( BKNI_Memcmp( pDest, CipherText_TDes_112, data_size ) ) {
            BDBG_ERR( ( "    Test FAILED!\n" ) );
        }
        else {
            BDBG_LOG( ( "    Test PASSED!\n" ) );
        }
    }

    if( pSrc )
        NEXUS_Memory_Free( pSrc );
    if( pDest )
        NEXUS_Memory_Free( pDest );

    securityUtil_PlatformUnInit( );

    return rc;
}

#else /* NEXUS_HAS_SECURITY */

#    include <stdio.h>
int main(
    void )
{
    printf( "This application is not supported on this platform!\n" );
    return -1;
}

#endif
