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
    unsigned      data_size, key_size;
    uint8_t      *pSrc = NULL, *pDest = NULL;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotBlockEntry entry;
    NEXUS_KeySlotKey slotKey;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    BDBG_LOG( ( "Step 1: Allocate a key slot." ) );
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerEntry;
    keyslotAllocSettings.useWithDma = true;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) { BDBG_ERR(( "\nError: Can't allocate keyslot\n" )); return NEXUS_NOT_AVAILABLE; }

    BDBG_LOG( ( "Step2: Configure the allocated keyslot." ) );
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );
    NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );

    BDBG_LOG( ( "Step3: Configure a key entry of the allocated keyslot for data encryption." ) );
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eDes;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb; /* simple Ecb mode. */
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    BDBG_LOG( ( "Step4: Load a clear key to the configured key entry #%d.\n", entry ) );
    key_size = securityGetAlogrithmKeySize( keyslotEntrySettings.algorithm );
    slotKey.size = key_size;
    BKNI_Memcpy( slotKey.key, clearKey_64, key_size );

    NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );

    BDBG_LOG( ( "Step5: Use the clear key as the root key to encrypt a plain text, the cipher is pDest.") );
    data_size = sizeof( plainText );

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );

    BDBG_LOG( ( "Use the crypto DMA to encrypt the plain text, cipher is pDest.") );
    securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );

    if( pSrc ) NEXUS_Memory_Free( pSrc );
    if( pDest ) NEXUS_Memory_Free( pDest );
    if (keyslotHandle) NEXUS_KeySlot_Free( keyslotHandle );

    /* Shutdown NEXUS. */
    securityUtil_PlatformUnInit( );

    return NEXUS_SUCCESS;
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
