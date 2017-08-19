/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_SECURITY

#    include "nexus_platform.h"
#    include "nexus_memory.h"
#    include <stdio.h>
#    include <stdlib.h>
#    include <unistd.h>
#    include <string.h>
#    include <assert.h>

#    include "nexus_security_datatypes.h"
#    include "nexus_security.h"
#    include "nexus_keyladder.h"
#    include "security_main.h"
#    include "security_util.h"

#    define MAX_NUM_KEYLADDERS  (8)

typedef struct security_keyladderTestData {
    NEXUS_KeyLadderLevelKey levelKey;
    NEXUS_KeyLadderSettings settings;
    NEXUS_KeyLadderHandle handle;
    NEXUS_CryptographicAlgorithmMode algorithmMode;
    unsigned      data_size;
    uint8_t      *pSrc;
    uint8_t      *pDest;
} security_keyladderTestData;

typedef struct security_keyladderListNode {
    BLST_S_ENTRY(
    security_keyladderListNode ) next;
    NEXUS_KeyLadderHandle keyladderHandle;
} security_keyladderListNode;

static uint8_t plainText[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

static int test_keyladder_GenerateLevelKeyIv(
    NEXUS_KeySlotHandle keyslotHandle,
    NEXUS_KeySlotBlockEntry slotEntry,
    NEXUS_CryptographicAlgorithm algorithm,
    NEXUS_KeyLadderMode mode,
    NEXUS_KeyLadderDestination destination,
    NEXUS_KeyLadderHandle * keyLadderhandle )
{
    NEXUS_KeyLadderHandle handle;
    NEXUS_KeyLadderAllocateSettings ladderAllocSettings;
    NEXUS_CryptographicOperation operation;
    NEXUS_KeyLadderSettings ladderSettings;
    NEXUS_KeyLadderLevelKey levelKey;
    uint8_t       procIn[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    NEXUS_Error   rc;

    if( !keyLadderhandle ) {
        printf( "Error, NULL pointer of keyladder handle\n" );
        return -1;
    }

    /* allocate a new key ladder */
    if( !( *keyLadderhandle ) ) {
        NEXUS_KeyLadderInfo keyLadderInfo;

        NEXUS_KeyLadder_GetDefaultAllocateSettings( &ladderAllocSettings );
        ladderAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;

        handle = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &ladderAllocSettings );
        if( handle == NULL ) { printf( "ERROR, NEXUS_KeyLadder_Allocate\n" ); return -1; }

        rc = NEXUS_KeyLadder_GetInfo( handle, &keyLadderInfo );
        if( rc != NEXUS_SUCCESS ) { printf( "ERROR, NEXUS_KeyLadder_GetInfo\n" ); return -1; }

        verboseLog( "Keyladder allocated. Index[%d].\n", keyLadderInfo.index );

        *keyLadderhandle = handle;
    }
    else {
        handle = *keyLadderhandle;
    }

    operation = NEXUS_CryptographicOperation_eDecrypt;

    NEXUS_KeyLadder_GetSettings( handle, &ladderSettings );
    ladderSettings.algorithm = algorithm;
    ladderSettings.keySize = 128;
    ladderSettings.operation = operation;
    if( destination == NEXUS_KeyLadderDestination_eKeyslotIv || destination == NEXUS_KeyLadderDestination_eKeyslotIv2 ) {
        ladderSettings.mode = NEXUS_KeyLadderMode_eGeneralPurpose1;
    }
    else {
        ladderSettings.mode = mode;
    }

    ladderSettings.numLevels = 4;

    ladderSettings.root.type = NEXUS_KeyLadderRootType_eOtpAskm;
    ladderSettings.root.otpKeyIndex = 1;
    ladderSettings.root.askm.caVendorId = 0x1234;
    ladderSettings.root.askm.caVendorIdScope = NEXUS_KeyladderCaVendorIdScope_eChipFamily;
    ladderSettings.root.askm.stbOwnerSelect = NEXUS_KeyLadderStbOwnerIdSelect_eOne;

    rc = NEXUS_KeyLadder_SetSettings( handle, &ladderSettings );
    if( handle == NULL ) {
        printf( "ERROR, NEXUS_KeyLadder_SetSettings" );
        return -1;
    };

    /* Set the keyladder level keys */
    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 3;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey level 3\n" );
        return -1;
    };

    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 4;
    levelKey.route.destination = destination;
    levelKey.route.keySlot.handle = keyslotHandle;
    levelKey.route.keySlot.entry = slotEntry;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey level 4\n" );
        return -1;
    };

    return 0;
}

static int test_keyladder_SetKeyslot(
    NEXUS_KeySlotBlockEntry slotEntry,
    NEXUS_CryptographicAlgorithm algorithm,
    NEXUS_CryptographicAlgorithmMode algorithmMode,
    NEXUS_KeySlotHandle * pKeyslotHandle )
{
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotHandle keyslotHandle;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_Error   rc;

    /* allocate keyslot */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerBlock;
    keyslotAllocSettings.useWithDma = true;
    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        printf( "ERROR, NEXUS_KeySlot_Allocate failed" );
        return NEXUS_NOT_AVAILABLE;
    };

    /*configure keyslot parameters */
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );
    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeySlot_SetSettings failed" );
        goto exit;
    };

    /*configure keyslot entry parameters */
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, slotEntry, &keyslotEntrySettings );
    keyslotEntrySettings.algorithm = algorithm;
    keyslotEntrySettings.algorithmMode = algorithmMode;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;
    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, slotEntry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeySlot_SetEntrySettings failed" );
        goto exit;
    };

  exit:
    *pKeyslotHandle = keyslotHandle;

    return rc;
}

static int test_keyladder_DMATransferUsingLevelKey(
    security_keyladderTestData * config )
{
    NEXUS_Error   rc;

    rc = test_keyladder_SetKeyslot( config->levelKey.route.keySlot.entry,
                                    config->settings.algorithm,
                                    config->algorithmMode, &config->levelKey.route.keySlot.handle );
    if( rc != NEXUS_SUCCESS || !config->levelKey.route.keySlot.handle ) {
        printf( "ERROR, can not allocate keyslot" );
        return -1;
    }

    rc = test_keyladder_GenerateLevelKeyIv( config->levelKey.route.keySlot.handle,
                                            config->levelKey.route.keySlot.entry,
                                            config->settings.algorithm,
                                            config->settings.mode,
                                            config->levelKey.route.destination, &config->handle );
    if( rc != NEXUS_SUCCESS || !config->handle ) {
        BERR_TRACE( rc );
    }

    rc = securityUtil_DmaTransfer( config->levelKey.route.keySlot.handle, config->pSrc, config->pDest,
                                   NEXUS_DmaDataFormat_eBlock, config->data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    NEXUS_KeySlot_Free( config->levelKey.route.keySlot.handle );
    config->levelKey.route.keySlot.handle = 0;

    NEXUS_KeyLadder_Free( config->handle );
    config->handle = 0;

    return 0;
}

int test_keyladder_touch(
    securityTestConfig * pArgs )
{
    NEXUS_KeyLadderHandle handle;
    NEXUS_KeyLadderAllocateSettings keyladderSettings;
    NEXUS_KeyLadderInfo keyLadderInfo;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 1 );
        SECURITY_FRAMEWORK_SET_NAME( "keyladder touch" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Open and close a keyladder instance" );
        return 1;
    }

    NEXUS_KeyLadder_GetDefaultAllocateSettings( &keyladderSettings );

    handle = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &keyladderSettings );
    if( handle == NULL ) { printf( "ERROR, NEXUS_KeyLadder_Allocate" ); return -1; };

    rc = NEXUS_KeyLadder_GetInfo( handle, &keyLadderInfo );
    if( rc != NEXUS_SUCCESS ) { printf( "ERROR, NEXUS_KeyLadder_GetInfo\n" ); return -1; }

    verboseLog( "Keyladder allocated. Index[%d].\n", keyLadderInfo.index );

    NEXUS_KeyLadder_Free( handle );

    return 0;
}

int test_keyladder_global_conf(
    securityTestConfig * pArgs )
{
    NEXUS_KeyLadderHandle handle;
    NEXUS_KeyLadderAllocateSettings ladderAllocSettings;
    NEXUS_KeyLadderSettings ladderSettings;
    NEXUS_KeyLadderLevelKey levelKey;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_KeySlotBlockEntry slotEntry = NEXUS_KeySlotBlockEntry_eCpsClear;
    uint8_t       procIn[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    NEXUS_Error   rc;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 3 );
        SECURITY_FRAMEWORK_SET_NAME( "Global root configure." );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Configure global key." );
        return 1;
    }

    /* allocate keyslot */

    rc = test_keyladder_SetKeyslot( slotEntry,
                                    NEXUS_CryptographicAlgorithm_eAes128,
                                    NEXUS_CryptographicAlgorithmMode_eEcb, &keyslotHandle );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, allocate keyslot failed." );
        return -1;
    };

    /* allocate the key ladder */
    NEXUS_KeyLadder_GetDefaultAllocateSettings( &ladderAllocSettings );
    ladderAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    handle = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &ladderAllocSettings );
    if( handle == NULL ) {
        printf( "ERROR, NEXUS_KeyLadder_Allocate" );
        return -1;
    };

    /* configure the root key  */
    NEXUS_KeyLadder_GetSettings( handle, &ladderSettings );
    ladderSettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    ladderSettings.keySize = 128;
    ladderSettings.operation = NEXUS_CryptographicOperation_eEncrypt;
    ladderSettings.mode = NEXUS_KeyLadderMode_eCp_128_5;
    ladderSettings.numLevels = 5;
    ladderSettings.root.type = NEXUS_KeyLadderRootType_eGlobalKey;
    ladderSettings.root.askm.caVendorId = 0x1234;
    ladderSettings.root.askm.caVendorIdScope = NEXUS_KeyladderCaVendorIdScope_eChipFamily;
    ladderSettings.root.askm.stbOwnerSelect = NEXUS_KeyLadderStbOwnerIdSelect_eOne;
    ladderSettings.root.globalKey.owner = NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eOne;
    ladderSettings.root.globalKey.index = 0x0;  /*?? */
    rc = NEXUS_KeyLadder_SetSettings( handle, &ladderSettings );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_SetSettings" );
        return -1;
    };

    /* set the keyladder level keys */
    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 3;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey level 3\n" );
        return -1;
    };

    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 4;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey level 4\n" );
        return -1;
    };

    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 5;
    levelKey.route.destination = NEXUS_KeyLadderDestination_eKeyslotKey;
    levelKey.route.keySlot.handle = keyslotHandle;
    levelKey.route.keySlot.entry = slotEntry;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey level 5\n" );
        return -1;
    };

    NEXUS_KeySlot_Free( keyslotHandle );
    NEXUS_KeyLadder_Free( handle );

    return 0;
}

int test_keyladder_otp_conf(
    securityTestConfig * pArgs )
{
    NEXUS_KeyLadderHandle handle;
    NEXUS_KeyLadderAllocateSettings ladderAllocSettings;
    NEXUS_KeyLadderSettings ladderSettings;
    NEXUS_KeyLadderLevelKey levelKey;
    uint8_t       procIn[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    NEXUS_Error   rc;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 4 );
        SECURITY_FRAMEWORK_SET_NAME( "OTP ASKM root configure." );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Configure OTP ASKM key." );
        return 1;
    }

    NEXUS_KeyLadder_GetDefaultAllocateSettings( &ladderAllocSettings );
    ladderAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;

    handle = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &ladderAllocSettings );
    if( handle == NULL ) {
        printf( "ERROR, NEXUS_KeyLadder_Allocate" );
        return -1;
    };

    verboseLog( "Keyladder allocated. Configuring\n" );

    NEXUS_KeyLadder_GetSettings( handle, &ladderSettings );
    ladderSettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    ladderSettings.keySize = 128;
    ladderSettings.operation = NEXUS_CryptographicOperation_eEncrypt;
    ladderSettings.mode = NEXUS_KeyLadderMode_eCp_128_4;
    ladderSettings.numLevels = 3;

    ladderSettings.root.type = NEXUS_KeyLadderRootType_eOtpAskm;
    ladderSettings.root.otpKeyIndex = 1;
    ladderSettings.root.askm.caVendorId = 0x1234;
    ladderSettings.root.askm.caVendorIdScope = NEXUS_KeyladderCaVendorIdScope_eChipFamily;
    ladderSettings.root.askm.stbOwnerSelect = NEXUS_KeyLadderStbOwnerIdSelect_eOne;

    rc = NEXUS_KeyLadder_SetSettings( handle, &ladderSettings );
    if( handle == NULL ) {
        printf( "ERROR, NEXUS_KeyLadder_SetSettings" );
        return -1;
    };

    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 3;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof( levelKey.ladderKey ) );

    rc = NEXUS_KeyLadder_GenerateLevelKey( handle, &levelKey );
    if( handle == NULL ) {
        printf( "ERROR, NEXUS_KeyLadder_GenerateLevelKey" );
        return -1;
    };

    NEXUS_KeyLadder_Free( handle );

    return 0;
}

int test_keyladder_otp_operation(
    NEXUS_KeySlotBlockEntry slotEntry,
    unsigned data_size,
    uint8_t * pSrc,
    uint8_t * pDest )
{
    NEXUS_KeyLadderHandle handle = NULL;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_Error   rc;

    rc = test_keyladder_SetKeyslot( slotEntry,
                                    NEXUS_CryptographicAlgorithm_eAes128,
                                    NEXUS_CryptographicAlgorithmMode_eEcb, &keyslotHandle );
    if( rc != NEXUS_SUCCESS || !keyslotHandle ) {
        printf( "ERROR, can not allocate keyslot" );
        return -1;
    }

    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle,
                                            slotEntry,
                                            NEXUS_CryptographicAlgorithm_eAes128,
                                            NEXUS_KeyLadderMode_eCp_128_4,
                                            NEXUS_KeyLadderDestination_eKeyslotKey, &handle );
    if( rc != NEXUS_SUCCESS || !handle ) {
        BERR_TRACE( rc );
    }

    rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    NEXUS_KeySlot_Free( keyslotHandle );
    NEXUS_KeyLadder_Free( handle );

    return rc;
}

int test_keyladder_otp_enc_dec(
    securityTestConfig * pArgs )
{
    uint8_t      *pSrc = NULL,
        *pDest = NULL;
    NEXUS_KeySlotBlockEntry entry;
    NEXUS_Error   rc;
    unsigned      data_size = sizeof( plainText );

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 5 );
        SECURITY_FRAMEWORK_SET_NAME( "OTP ASKM root key operations." );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "OTP key as root key to decrypt or encrypt data with single keyladder." );
        return 1;
    }

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    /* Using Otp key as the root key to encrypt the plain text. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );
    rc = test_keyladder_otp_operation( entry, data_size, pSrc, pDest );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR with encryption of data\n" );
    }

    if( !BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "    Test FAILED! plainText has not been encrypted.\n" );
    }

    /* Using Otp key as the root key to decrypt the cipher text. */
    entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    BKNI_Memcpy( pSrc, pDest, data_size );
    BKNI_Memset( pDest, 0, data_size );
    rc = test_keyladder_otp_operation( entry, data_size, pSrc, pDest );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR with decryption of data\n" );
    }

    if( !BKNI_Memcmp( pDest, pSrc, data_size ) ) {
        printf( "    Test FAILED! Cipher Text has not been decrypted.\n" );
    }

    /* Check the decryption of the cipher is the same as the plain text. */
    if( BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "Test FAILED! The final result is not the expected values.\n" );
        DEBUG_PRINT_ARRAY( "PlainText:", data_size, plainText );
        DEBUG_PRINT_ARRAY( "Destination", data_size, pDest );
        rc = NEXUS_INVALID_PARAMETER;
    }
    else {
        verboseLog( "    Test PASSED!\n" );
        rc = NEXUS_SUCCESS;
    }

    if( pSrc )
        NEXUS_Memory_Free( pSrc );
    if( pDest )
        NEXUS_Memory_Free( pDest );

    return rc;
}

int test_keyladder_open_close_all(
    securityTestConfig * pArgs )
{
    int           i;
    NEXUS_KeyLadderHandle keyladderHandle;
    NEXUS_KeyLadderAllocateSettings keyladderSettings;

    BLST_S_HEAD( securityTest_KeyLadderList_t, security_keyladderListNode ) keyladderList;
    security_keyladderListNode *keyladderListNode,
                 *last;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 2 );
        SECURITY_FRAMEWORK_SET_NAME( "Allocate all keyladder instances" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Open and close all the keyladder instances." );
        return 1;
    }

    BLST_S_INIT( &keyladderList );

    NEXUS_KeyLadder_GetDefaultAllocateSettings( &keyladderSettings );

    for( i = 0; i < MAX_NUM_KEYLADDERS; i++ ) {
        keyladderHandle = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &keyladderSettings );
        if( !keyladderHandle ) {
            printf( "ERROR, NEXUS_KeyLadder_Allocate" );
            return -1;
        };

        keyladderListNode = BKNI_Malloc( sizeof( *keyladderListNode ) );
        keyladderListNode->keyladderHandle = keyladderHandle;
        verboseLog( "Allocated keyladder 0x%p\n", ( void * ) keyladderHandle );

        BLST_S_INSERT_HEAD( &keyladderList, keyladderListNode, next );
    }

    verboseLog( "\n\n Freeing all the allocated keyladders.\n\n" );
    keyladderListNode = BLST_S_FIRST( &keyladderList );

    while( keyladderListNode && keyladderListNode->keyladderHandle ) {
        verboseLog( "Freeing keyladder %p\n", ( void * ) keyladderListNode->keyladderHandle );
        NEXUS_KeyLadder_Free( keyladderListNode->keyladderHandle );
        last = keyladderListNode;
        keyladderListNode = BLST_S_NEXT( keyladderListNode, next );
        BKNI_Free( last );
    }

    return 0;
}

int test_keyladder_AES_ECB(
    securityTestConfig * pArgs )
{
    uint8_t      *pSrc = NULL,
        *pDest = NULL;
    NEXUS_KeySlotBlockEntry entry;
    NEXUS_Error   rc;
    unsigned      data_size = sizeof( plainText );
    NEXUS_KeySlotHandle keyslotHandle_enc,
                  keyslotHandle_dec;
    NEXUS_KeyLadderHandle keyladderHandle_enc = 0,
        keyladderHandle_dec = 0;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 6 );
        SECURITY_FRAMEWORK_SET_NAME( "Two OTP key keyladders AES_ECB" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION
            ( "Use keyladders and two keyslots to route keys for encryption and decryption seperately." );
        return 1;
    }

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    /* Using Otp key as the root key to encrypt the plain text. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = test_keyladder_SetKeyslot( entry,
                                    NEXUS_CryptographicAlgorithm_eAes128,
                                    NEXUS_CryptographicAlgorithmMode_eEcb, &keyslotHandle_enc );
    if( rc != NEXUS_SUCCESS || !keyslotHandle_enc ) {
        printf( "ERROR, can not allocate encryption keyslot.\n" );
        return -1;
    };

    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_enc,
                                            entry,
                                            NEXUS_CryptographicAlgorithm_eAes128,
                                            NEXUS_KeyLadderMode_eCp_128_4,
                                            NEXUS_KeyLadderDestination_eKeyslotKey, &keyladderHandle_enc );
    if( rc != NEXUS_SUCCESS || !keyladderHandle_enc ) {
        BERR_TRACE( rc );
    }

    rc = securityUtil_DmaTransfer( keyslotHandle_enc, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    if( !BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "    Test FAILED! plainText has not been encrypted.\n" );
    }

    /* Using Otp key as the root key to decrypt the cipher text. */
    entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    BKNI_Memcpy( pSrc, pDest, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = test_keyladder_SetKeyslot( entry,
                                    NEXUS_CryptographicAlgorithm_eAes128,
                                    NEXUS_CryptographicAlgorithmMode_eEcb, &keyslotHandle_dec );
    if( rc != NEXUS_SUCCESS || !keyslotHandle_dec ) {
        printf( "ERROR, can not allocate decryption keyslot.\n" );
        return -1;
    };

    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_dec,
                                            entry,
                                            NEXUS_CryptographicAlgorithm_eAes128,
                                            NEXUS_KeyLadderMode_eCp_128_4,
                                            NEXUS_KeyLadderDestination_eKeyslotKey, &keyladderHandle_dec );
    if( rc != NEXUS_SUCCESS || !keyladderHandle_dec ) {
        printf( "ERROR, can not allocate keyladder.\n" );
        BERR_TRACE( rc );
    }

    rc = securityUtil_DmaTransfer( keyslotHandle_dec, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    if( !BKNI_Memcmp( pDest, pSrc, data_size ) ) {
        printf( "    Test FAILED! Cipher Text has not been decrypted.\n" );
    }

    /* Check the decryption of the cipher is the same as the plain text. */
    if( BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "Test FAILED! The final result is not the expected values.\n" );
        DEBUG_PRINT_ARRAY( "PlainText:", data_size, plainText );
        DEBUG_PRINT_ARRAY( "Destination", data_size, pDest );
        rc = NEXUS_INVALID_PARAMETER;
    }
    else {
        verboseLog( "    Test PASSED!\n" );
        rc = NEXUS_SUCCESS;
    }

    if( pSrc )
        NEXUS_Memory_Free( pSrc );
    if( pDest )
        NEXUS_Memory_Free( pDest );

    NEXUS_KeyLadder_Free( keyladderHandle_enc );
    NEXUS_KeyLadder_Free( keyladderHandle_dec );
    NEXUS_KeySlot_Free( keyslotHandle_enc );
    NEXUS_KeySlot_Free( keyslotHandle_dec );

    return rc;
}

int test_keyladder_AES_CBC(
    securityTestConfig * pArgs )
{
    uint8_t      *pSrc = NULL,
        *pDest = NULL;
    unsigned      data_size = sizeof( plainText );
    NEXUS_Error   rc;

    const NEXUS_CryptographicAlgorithm algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    const NEXUS_CryptographicAlgorithmMode algorithmMode = NEXUS_CryptographicAlgorithmMode_eCbc;
    const NEXUS_KeyLadderMode keyladderMode = NEXUS_KeyLadderMode_eCp_128_4;

    NEXUS_KeySlotBlockEntry entry;
    NEXUS_KeySlotHandle keyslotHandle_enc = 0,
        keyslotHandle_dec = 0;
    NEXUS_KeyLadderHandle keyladderHandle_enc = 0,
        keyladderHandle_enc_iv = 0,
        keyladderHandle_dec_iv = 0,
        keyladderHandle_dec = 0;


    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 7 );
        SECURITY_FRAMEWORK_SET_NAME( "Two OTP key keyladders AES_CBC" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION
            ( "Use four keyladders and two keyslots to route keys and IVs for encryption and decryption seperately." );
        return 1;
    }

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    /* Using Otp key as the root key to encrypt the plain text. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = test_keyladder_SetKeyslot( entry, algorithm, algorithmMode, &keyslotHandle_enc );
    if( rc != NEXUS_SUCCESS || !keyslotHandle_enc ) {
        printf( "ERROR, can not allocate keyslot.\n" );
        BERR_TRACE( rc );
        goto exit;
    };

    /* Route key */
    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_enc,
                                            entry,
                                            algorithm,
                                            keyladderMode,
                                            NEXUS_KeyLadderDestination_eKeyslotKey, &keyladderHandle_enc );
    if( rc != NEXUS_SUCCESS || !keyladderHandle_enc ) {
        BERR_TRACE( rc );
        goto exit;
    }

    /* Route IV */
    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_enc,
                                            entry,
                                            algorithm,
                                            keyladderMode,
                                            NEXUS_KeyLadderDestination_eKeyslotIv, &keyladderHandle_enc_iv );
    if( rc != NEXUS_SUCCESS ) {
        printf( "ERROR, can not route IV for encryption.\n" );
        BERR_TRACE( rc );
        goto exit;
    }

    /* Using Otp key as the root key to encrypt the plain text, cipher is pDest. */
    rc = securityUtil_DmaTransfer( keyslotHandle_enc, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
        goto exit;
    }

    if( !BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "    Test FAILED! plainText has not been encrypted.\n" );
    }

    /* Using Otp key as the root key to decrypt the cipher text. */
    entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    BKNI_Memcpy( pSrc, pDest, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = test_keyladder_SetKeyslot( entry, algorithm, algorithmMode, &keyslotHandle_dec );
    if( rc != NEXUS_SUCCESS || !keyslotHandle_dec ) {
        printf( "ERROR, can not allocate keyladder for decryption.\n" );
        BERR_TRACE( rc );
        goto exit;
    };

    /* Route key */
    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_dec,
                                            entry,
                                            algorithm,
                                            keyladderMode,
                                            NEXUS_KeyLadderDestination_eKeyslotKey, &keyladderHandle_dec );
    if( rc != NEXUS_SUCCESS || !keyladderHandle_dec ) {
        printf( "ERROR, can not route key for decryption.\n" );
        BERR_TRACE( rc );
        goto exit;
    }

    /* Route IV */
    rc = test_keyladder_GenerateLevelKeyIv( keyslotHandle_dec,
                                            entry,
                                            algorithm,
                                            keyladderMode,
                                            NEXUS_KeyLadderDestination_eKeyslotIv, &keyladderHandle_dec_iv );
    if( rc != NEXUS_SUCCESS || !keyladderHandle_dec_iv ) {
        printf( "ERROR, can not route IV for decryption.\n" );
        BERR_TRACE( rc );
        goto exit;
    }

    /* Using Otp key as the root key to decrypt the cipher text, plain text is pDest. */
    rc = securityUtil_DmaTransfer( keyslotHandle_dec, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    if( !BKNI_Memcmp( pDest, pSrc, data_size ) ) {
        printf( "    Test FAILED! Cipher Text has not been decrypted.\n" );
    }

    /* Check the decryption of the cipher is the same as the plain text. */
    if( BKNI_Memcmp( pDest, plainText, data_size ) ) {
        printf( "Test FAILED! The final result is not the expected values.\n" );
        DEBUG_PRINT_ARRAY( "PlainText:", data_size, plainText );
        DEBUG_PRINT_ARRAY( "Destination", data_size, pDest );
        rc = NEXUS_INVALID_PARAMETER;
    }
    else {
        verboseLog( "    Test PASSED!\n" );
        rc = NEXUS_SUCCESS;
    }

  exit:

    if( pSrc )
        NEXUS_Memory_Free( pSrc );
    if( pDest )
        NEXUS_Memory_Free( pDest );

    if( keyladderHandle_enc )
        NEXUS_KeyLadder_Free( keyladderHandle_enc );
    if( keyladderHandle_enc_iv )
        NEXUS_KeyLadder_Free( keyladderHandle_enc_iv );
    if( keyslotHandle_enc )
        NEXUS_KeySlot_Free( keyslotHandle_enc );

    if( keyladderHandle_dec )
        NEXUS_KeyLadder_Free( keyladderHandle_dec );
    if( keyladderHandle_dec_iv )
        NEXUS_KeyLadder_Free( keyladderHandle_dec_iv );
    if( keyslotHandle_dec )
        NEXUS_KeySlot_Free( keyslotHandle_dec );

    return rc;
}

int test_keyladder_otpAskm_rootKey(
    securityTestConfig * pArgs )
{
    security_keyladderTestData config;
    NEXUS_CryptographicAlgorithm algorithm[] = {
        NEXUS_CryptographicAlgorithm_e3DesAba,
        NEXUS_CryptographicAlgorithm_eAes128
    };

    NEXUS_KeyLadderMode mode[] = {
        NEXUS_KeyLadderMode_eCp_64_4,
        NEXUS_KeyLadderMode_eCp_128_4
    };

    unsigned      a,
                  m;

    NEXUS_Error   rc;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyladder );
        SECURITY_FRAMEWORK_SET_NUMBER( 8 );
        SECURITY_FRAMEWORK_SET_NAME( "OTP ASKM root key keyladder operations." );
        SECURITY_FRAMEWORK_SET_DESCRIPTION
            ( "ASKM root key as root key to decrypt or encrypt data with single keyladder with different configs." );
        return 1;
    }

    BKNI_Memset( &config, 0, sizeof( config ) );
    config.data_size = sizeof( plainText );

    NEXUS_Memory_Allocate( config.data_size, NULL, ( void ** ) &config.pSrc );
    NEXUS_Memory_Allocate( config.data_size, NULL, ( void ** ) &config.pDest );

    /* TODO: Iterate all the supported sub modes. */
    for( a = 0; a < sizeof( algorithm ) / sizeof( NEXUS_CryptographicAlgorithm ); a++ ) {
        for( m = 0; m < sizeof( mode ) / sizeof( NEXUS_KeyLadderMode ); m++ ) {
            config.settings.algorithm = algorithm[a];
            config.settings.mode = mode[m];

            /* AES Ecb mode */
            config.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;

            /* Using Otp key as the root key to encrypt the plain text. */
            config.levelKey.route.keySlot.entry = NEXUS_KeySlotBlockEntry_eCpsClear;
            config.levelKey.route.destination = NEXUS_KeyLadderDestination_eKeyslotKey;

            BKNI_Memcpy( config.pSrc, plainText, config.data_size );
            BKNI_Memset( config.pDest, 0, config.data_size );

            rc = test_keyladder_DMATransferUsingLevelKey( &config );
            if( rc != NEXUS_SUCCESS ) {
                printf( "ERROR with encryption of data\n" );
                goto exit;
            }

            if( !BKNI_Memcmp( config.pDest, plainText, config.data_size ) ) {
                printf( "    Test FAILED! plainText has not been encrypted.\n" );
                return -1;
            }

            /* Using Otp key as the root key to decrypt the cipher text. */
            config.levelKey.route.keySlot.entry = NEXUS_KeySlotBlockEntry_eCpdClear;
            BKNI_Memcpy( config.pSrc, config.pDest, config.data_size );
            BKNI_Memset( config.pDest, 0, config.data_size );
            rc = test_keyladder_DMATransferUsingLevelKey( &config );
            if( rc != NEXUS_SUCCESS ) {
                printf( "ERROR with decryption of data\n" );
                goto exit;
            }

            if( !BKNI_Memcmp( config.pDest, config.pSrc, config.data_size ) ) {
                printf( "    Test FAILED! cipher text has not been decrypted.\n" );
                rc = -1;
                goto exit;
            }

            /* Check the decryption of the cipher is the same as the plain text. */
            if( BKNI_Memcmp( config.pDest, plainText, config.data_size ) ) {
                printf( "Test FAILED! The final result is not the expected values.\n" );
                DEBUG_PRINT_ARRAY( "PlainText:", config.data_size, plainText );
                DEBUG_PRINT_ARRAY( "Destination", config.data_size, config.pDest );
                rc = NEXUS_INVALID_PARAMETER;
                goto exit;
            }
            else {
                verboseLog( "    Test PASSED with algorithm [%d], keyladder mode [%d]!\n", config.settings.algorithm,
                        config.settings.mode );
                rc = NEXUS_SUCCESS;
            }
        }
    }

  exit:

    if( config.pSrc )
        NEXUS_Memory_Free( config.pSrc );
    if( config.pDest )
        NEXUS_Memory_Free( config.pDest );

    return rc;
}

/* initalise the keyslot tests. */
void initTestGroup_keyladder(
    void )
{
    /* register group */
    securityFramework_registerGroup( securityTestGroup_keyladder, "KEYLADDER" );

    /*register keyladder tests*/
    securityFramework_registerTest( test_keyladder_touch );
    securityFramework_registerTest( test_keyladder_open_close_all );
    securityFramework_registerTest( test_keyladder_global_conf );
    securityFramework_registerTest( test_keyladder_otp_conf );
    securityFramework_registerTest( test_keyladder_otp_enc_dec );
    securityFramework_registerTest( test_keyladder_AES_ECB );
    securityFramework_registerTest( test_keyladder_AES_CBC );
    securityFramework_registerTest( test_keyladder_otpAskm_rootKey );
    return;
}

#endif
