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

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION==2)

#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_security.h"
#include "nexus_hash.h"
#include "security_utils.h"
#include "security_test_vectors_sha.h"

#    define SECURITY_SET_SHA(settings, data, dataLn, shaType, shaLn)    \
{                                                                       \
    BKNI_Memset( &settings, 0, sizeof( settings) );                     \
                                                                        \
    settings.hashType  = NEXUS_HashType_e##shaType##_##shaLn;           \
    data.inputText     = sha_##shaLn##_##dataLn##_text;                 \
    data.shaResult     = sha_##shaLn##_##dataLn;                        \
    data.inputLenght   = sha_##shaLn##_##dataLn##_text_lenght;          \
    data.shaResultLenght   = sha_##shaLn##_##dataLn##_lenght;           \
    pTestTitle             = "Sha_"#shaLn" with "#dataLn" byte length data test"; \
    printf("\n---> Test#%d %s %s starts.\n",count, data.appendKey ? "using appended softKey ":"", pTestTitle); \
}

typedef struct security_hashTestData {
    char         *inputText;
    unsigned      inputLenght;
    bool          appendKey;
    uint8_t      *shaResult;
    unsigned      shaResultLenght;
} security_hashTestData;

/* The main example to setup the different types' sha operations */
static int    test_hash_sha_operation(
    NEXUS_HashSettings * pSettings,
    security_hashTestData * pData );

int main(
    int argc,
    char **argv )
{
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    char         *pTestTitle;
    NEXUS_HashSettings settings;
    security_hashTestData data;
    static uint8_t count = 0;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* Followings are the usual data SHA operations
     * The data size is usually larger than one byte length.
     */
    BKNI_Memset( &data, 0, sizeof( data ) );

    SECURITY_SET_SHA( settings, data, usual, 1, 160 );  /* Setup the sha input data, and the result data vectors */
    rc = test_hash_sha_operation( &settings, &data );   /* Nexus APIs to operate NEXUS_HashType_e1_160 on the input data sha_160_usual_text[] */
    SECURITY_CHECK_TEST_RESULT( rc );                   /* Check the sha result is same as sha_160_usual[]. */

    SECURITY_SET_SHA( settings, data, usual, 2, 224 );  /* Setup the sha input data, and the result data vectors */
    rc = test_hash_sha_operation( &settings, &data );   /* Nexus APIs to operate NEXUS_HashType_e2_224 on the input data sha_224_usual_text[] */
    SECURITY_CHECK_TEST_RESULT( rc );                   /* Check the sha result is same as sha_224_usual[]. */

    SECURITY_SET_SHA( settings, data, usual, 2, 256 );  /* Setup the sha input data, and the result data vectors */
    rc = test_hash_sha_operation( &settings, &data );   /* Nexus APIs to operate NEXUS_HashType_e2_256 on the input data sha_256_usual_text[] */
    SECURITY_CHECK_TEST_RESULT( rc );                   /* Check the sha result is same as sha_256_usual[]. */

    /* The followings are the one byte data SHA operations.
     * The data size is exact 1 byte.
     */
    BKNI_Memset( &data, 0, sizeof( data ) );

    SECURITY_SET_SHA( settings, data, one, 1, 160 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    SECURITY_SET_SHA( settings, data, one, 2, 224 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    SECURITY_SET_SHA( settings, data, one, 2, 256 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );
    BKNI_Memset( &data, 0, sizeof( data ) );

    /* The followings are the zero byte data SHA operations.
     * The data size is zero. The data is from for example 'echo -n ""'
     */
    BKNI_Memset( &data, 0, sizeof( data ) );

    SECURITY_SET_SHA( settings, data, zero, 1, 160 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    SECURITY_SET_SHA( settings, data, zero, 2, 224 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    SECURITY_SET_SHA( settings, data, zero, 2, 256 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    /* Followings are the operations with appended soft key.
     * At the end of the data, there is key appended.
     */
    BKNI_Memset( &data, 0, sizeof( data ) );

    data.appendKey = true;      /* Enabling append soft key with data */

    SECURITY_SET_SHA( settings, data, usual, 2, 224 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    SECURITY_SET_SHA( settings, data, usual, 2, 256 );
    rc = test_hash_sha_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    /* Shutdown the Nexus platform. */
    securityUtil_PlatformUnInit(  );

    return rc;
}

/* The main example to setup the different types' sha operations
*/
static int test_hash_sha_operation(
    NEXUS_HashSettings * pSettings,
    security_hashTestData * pData )
{
    NEXUS_HashHandle hashHandle = NULL;
    NEXUS_HashData hashData;
    NEXUS_HashResult result;
    NEXUS_HashSettings settings;
    NEXUS_MemoryAllocationSettings memSetting;
    NEXUS_Addr    offset = 0;
    uint8_t      *addr = 0;
    size_t        dataLen = 0;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    const char    zero[] = "ZERO";

    /* This is the function where the relevent Nexus APIs are called for the Hash operations */

    hashHandle = NEXUS_Hash_Create(  );
    if( !hashHandle ) { return BERR_TRACE( NEXUS_NOT_AVAILABLE ); }

    if( !strstr( ( char * ) pData->inputText, zero ) ) {
        dataLen = pData->inputLenght;
    }

    NEXUS_Hash_GetDefaultSettings( &settings );
    settings.hashType = pSettings->hashType;
    settings.appendKey = pData->appendKey;

    /* If we are handling softKey. */
    if( settings.appendKey && !pSettings->key.keyladder.handle ) {
        /* Use the last eight chars from the usual input data as the soft key. */
        if( dataLen >= 16 ) {
            /* The size is bits size. */
            unsigned      keySizeBytes = 16;

            settings.key.softKeySize = keySizeBytes * 8;    /*bits */
            BKNI_Memcpy( settings.key.softKey, &pData->inputText[dataLen - keySizeBytes], keySizeBytes );
            BDBG_LOG( ( "%s() Setting softKey of %d bytes.\n", BSTD_FUNCTION, keySizeBytes ) );

            /* The bottom part of data in the test vector is taken as appended the key, remove it from the data to be hashed. */
            dataLen -= keySizeBytes;
        }
        else {
            rc = NEXUS_NOT_AVAILABLE;
            SECURITY_CHECK_RC( rc );
        }
    }

    /* Set the Hash type. */
    rc = NEXUS_Hash_SetSettings( hashHandle, &settings );
    SECURITY_CHECK_RC( rc );

    /* If there is data, let's setup its offset. */
    if( dataLen ) {
        NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
        memSetting.alignment = 32;

        rc = NEXUS_Memory_Allocate( dataLen, &memSetting, ( void * ) &addr );
        SECURITY_CHECK_RC( rc );

        BKNI_Memcpy( addr, pData->inputText, dataLen );
        NEXUS_Memory_FlushCache( addr, dataLen );

        /* The operation needs the memory offset */
        offset = NEXUS_AddrToOffset( addr );
    }

    /* Let's submit the data's offset and length. */
    NEXUS_Hash_GetDefaultData( &hashData );

    hashData.dataOffset = offset;
    hashData.dataSize = dataLen;
    hashData.last = true;
    rc = NEXUS_Hash_SubmitData( hashHandle, &hashData, &result );
    SECURITY_CHECK_RC( rc );

    if( pData->shaResultLenght != result.hashLength ) {
        printf( "    Test FAILED, invalid length [%u][%u]\n", pData->shaResultLenght, result.hashLength );
        rc = NEXUS_INVALID_PARAMETER;
        SECURITY_CHECK_RC( rc );
    }

    /* Check the hash result. */
    if( BKNI_Memcmp( result.hash, pData->shaResult, result.hashLength ) ) {
        printf( "    Test FAILED, result is wrong!\n" );
        DEBUG_PRINT_ARRAY( "Data to hash:", hashData.dataSize, addr );
        DEBUG_PRINT_ARRAY( "Test result:", result.hashLength, result.hash );
        DEBUG_PRINT_ARRAY( "Expected   :", pData->shaResultLenght, pData->shaResult );
        rc = NEXUS_INVALID_PARAMETER;
        SECURITY_CHECK_RC( rc );
    }

  exit:

    /* Clear up. */
    if( hashHandle ) NEXUS_Hash_Destroy( hashHandle );
    if( addr )       NEXUS_Memory_Free( addr );

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
