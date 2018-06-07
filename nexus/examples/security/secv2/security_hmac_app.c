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
#include "nexus_hmac.h"
#include "security_utils.h"
#include "security_test_vectors_hmac.h"

#    define SECURITY_SET_SHA_HMAC(settings, data, dataLn, shaType, shaLn)   \
{                                                                           \
    BKNI_Memset( &settings, 0, sizeof( settings) );                         \
    settings.key.source   = NEXUS_HmacKeySource_eSoftwareKey;               \
    settings.hashType     = NEXUS_HashType_e##shaType##_##shaLn;            \
    data.pInputText       = sha_##shaLn##_hmac_##dataLn##_text;             \
    data.pHmacResult      = sha_##shaLn##_hmac_##dataLn;                    \
    data.inputLenght      = sha_##shaLn##_hmac_##dataLn##_text_lenght;      \
    data.hmacResultLenght = sha_##shaLn##_hmac_##dataLn##_lenght;           \
    data.pHmacKey         = sha_##shaLn##_hmac_##dataLn##_key;              \
    data.keyLenght        = sizeof( sha_##shaLn##_hmac_##dataLn##_key );    \
    pTestTitle            = "HMAC SHA_"#shaLn" with "#dataLn" byte length data test";  \
    BDBG_LOG(("---> Test#%d %s starts.\n",count, pTestTitle));   \
}

typedef struct {
    char         *pInputText;
    char         *pHmacKey;
    uint8_t      *pHmacResult;
    unsigned      inputLenght;
    unsigned      hmacResultLenght;
    unsigned      keyLenght;
} security_hmacTestData;

/* The main example to setup the different types' HMAC operations with soft key.
*/
static int    test_hmac_operation(
    NEXUS_HmacSettings * pSettings,
    security_hmacTestData * pData );

int main(
    int argc,
    char **argv )
{
    NEXUS_Error   rc = NEXUS_UNKNOWN;
    char         *pTestTitle;
    NEXUS_HmacSettings settings;
    security_hmacTestData data;
    static uint8_t count = 0;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* Followings are the usual data SHA operations
     * The data size is usually larger than one byte length.
     */

    BKNI_Memset( &data, 0, sizeof( data ) );

    /* HAMC SHA1 */
    /* Setup the HMAC input data sha_160_hmac_usual_text[],
       key sha_160_hmac_usual_key[],
       and the result data sha_160_hmac_usual[] vectors */
    SECURITY_SET_SHA_HMAC( settings, data, usual, 1, 160 );
    rc = test_hmac_operation( &settings, &data );  /* Nexus APIs to operate HMAC NEXUS_HashType_e1_160 with soft key on the input */
    SECURITY_CHECK_TEST_RESULT( rc );              /* Check the sha result is same as sha_160_hmac_usual[]. */

    /* HAMC SHA224 */
    /* Setup the HMAC input data sha_224_hmac_usual_text[],
       key sha_224_hmac_usual_key[],
       and the result data sha_224_hmac_usual[] vectors */
    SECURITY_SET_SHA_HMAC( settings, data, usual, 2, 224 );
    rc = test_hmac_operation( &settings, &data ); /* Nexus APIs to operate HMAC NEXUS_HashType_e2_224 with soft key on the input */
    SECURITY_CHECK_TEST_RESULT( rc );             /* Check the sha result is same as sha_224_hmac_usual[]. */

    /* HAMC SHA256 */
    /* Setup the HMAC input data sha_256_hmac_usual_text[],
       key sha_256_hmac_usual_key[],
       and the result data sha_256_hmac_usual[] vectors */
    SECURITY_SET_SHA_HMAC( settings, data, usual, 2, 256 );
    rc = test_hmac_operation( &settings, &data ); /* Nexus APIs to operate HMAC NEXUS_HashType_e2_256 with soft key on the input */
    SECURITY_CHECK_TEST_RESULT( rc );             /* Check the sha result is same as sha_256_hmac_usual[]. */


    /* The followings are the one byte data HMAC operations.
     * The data size is exact 1 byte.
     */

    /* HMAC SHA1 operation on one byte input data. */
    SECURITY_SET_SHA_HMAC( settings, data, one, 1, 160 );
    rc = test_hmac_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    /* HMAC SHA2_224 operation on one byte input data.*/
    SECURITY_SET_SHA_HMAC( settings, data, one, 2, 224 );
    rc = test_hmac_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    /* HMAC SHA2_256 operation on one byte input data.*/
    SECURITY_SET_SHA_HMAC( settings, data, one, 2, 256 );
    rc = test_hmac_operation( &settings, &data );
    SECURITY_CHECK_TEST_RESULT( rc );

    /* Shutdown the Nexus platform. */
    securityUtil_PlatformUnInit(  );

    return rc;
}

/* The main example to setup the different types' HMAC operations
*/

static int test_hmac_operation(
    NEXUS_HmacSettings * pSettings,
    security_hmacTestData * pData )
{
    NEXUS_HmacHandle hmacHandle = NULL;
    NEXUS_HmacData hmacData;
    NEXUS_HmacResult result;
    NEXUS_MemoryAllocationSettings memSetting;
    NEXUS_HmacSettings settings;
    NEXUS_Addr    offset = 0;
    uint8_t      *addr = 0;
    size_t        dataLen = 0, keySize = 0;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( !pSettings ) { return BERR_TRACE( NEXUS_NOT_AVAILABLE ); }
    if( !pData )     { return BERR_TRACE( NEXUS_NOT_AVAILABLE ); }

    hmacHandle = NEXUS_Hmac_Create(  );
    if( !hmacHandle ) { return BERR_TRACE( NEXUS_NOT_AVAILABLE ); }

    NEXUS_Hmac_GetDefaultSettings( &settings );

    settings.hashType = pSettings->hashType;
    settings.key.source = pSettings->key.source;

    if( settings.key.source == NEXUS_HmacKeySource_eSoftwareKey ) {
        switch ( settings.hashType ) {
            case NEXUS_HashType_e1_160:
                keySize = 20;
                break;
            case NEXUS_HashType_e2_224:
                keySize = 28;
                break;
            case NEXUS_HashType_e2_256:
                keySize = 32;
                break;
            default:
                rc = NEXUS_NOT_AVAILABLE;
                break;
        }
        SECURITY_CHECK_RC( rc );

        /* The size is byte size. */
        keySize = pData->keyLenght;

        /* Setup the soft key. */
        BKNI_Memcpy( &settings.key.softKey, pData->pHmacKey, keySize );
    }
    else {
        rc = NEXUS_NOT_AVAILABLE;
        SECURITY_CHECK_RC( rc );
    }

    rc = NEXUS_Hmac_SetSettings( hmacHandle, &settings );
    SECURITY_CHECK_RC( rc );

    dataLen = pData->inputLenght;

    /* It needs the offset of the input data. */
    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );

    memSetting.alignment = 32;
    rc = NEXUS_Memory_Allocate( dataLen, &memSetting, ( void * ) &addr );
    SECURITY_CHECK_RC( rc );

    BKNI_Memcpy( addr, pData->pInputText, dataLen );
    NEXUS_Memory_FlushCache( addr, dataLen );

    offset = NEXUS_AddrToOffset( addr );

    NEXUS_Hmac_GetDefaultData( &hmacData );

    hmacData.dataOffset = offset;
    hmacData.dataSize = dataLen;
    hmacData.last = true;

    rc = NEXUS_Hmac_SubmitData( hmacHandle, &hmacData, &result );
    SECURITY_CHECK_RC( rc );

    if( pData->hmacResultLenght != result.hmacLength ||
        BKNI_Memcmp( result.hmac, pData->pHmacResult, result.hmacLength ) ) {
        BDBG_ERR( ( "    Test FAILED, either result's data lengh, or data is wrong!\n" ) );
        DEBUG_PRINT_ARRAY( "Data       :", dataLen, pData->pInputText );
        DEBUG_PRINT_ARRAY( "Key        :", keySize, pData->pHmacKey );
        DEBUG_PRINT_ARRAY( "Expected   :", pData->hmacResultLenght, pData->pHmacResult );
        DEBUG_PRINT_ARRAY( "test result:", result.hmacLength, result.hmac );
        rc = BERR_TRACE( NEXUS_INVALID_PARAMETER );
        SECURITY_CHECK_RC( rc );
    }

  exit:
    if( hmacHandle ) NEXUS_Hmac_Destroy( hmacHandle );
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
