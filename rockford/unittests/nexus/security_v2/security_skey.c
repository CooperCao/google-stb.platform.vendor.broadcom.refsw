/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#if NEXUS_HAS_SECURITY
#include "nexus_security_datatypes.h"
#include "nexus_platform.h"
#include "nexus_platform.h"
#include "nexus_recpump.h"
#include "nexus_playpump.h"
#include "nexus_dma.h"

#include "nexus_memory.h"
#include "nexus_security.h"
#include "nexus_keyslot.h"
#include "security_util.h"
#include "security_main.h"
#include "security_test_vectors_clear_key.h"

BDBG_MODULE( security_skey );

#    define SECURITY_CHECK_TEST_RETURN(rc)  {                               \
    char* op;                                                           \
    op = (data.entry ==NEXUS_KeySlotBlockEntry_eCpsClear) ? "Encryption" : "Decryption" ;              \
    if (rc) {                                                           \
        BDBG_ERR( ( "<--- Failed: %s() Test#%d %s with %s.", __FUNCTION__, count++, op, pTestTitle));   \
        return rc;                                                      \
    } else  {                                                           \
        verboseLog("<--- Success: %s() Test#%d %s with %s.",  __FUNCTION__, count++, op, pTestTitle); \
    }                                                                   \
}

#    define SECURITY_SET_CIPHER(settings, data, algo, keyLn, algoMode, termMode)    \
{                                                                               \
    BKNI_Memset( &settings, 0, sizeof( settings) );                             \
    settings.algorithm          = NEXUS_CryptographicAlgorithm_e##algo##keyLn;  \
    settings.algorithmMode      = NEXUS_CryptographicAlgorithmMode_e##algoMode; \
    settings.terminationMode    = NEXUS_KeySlotTerminationMode_e##termMode;     \
    settings.external.key       = data.entrySettings.external.key;              \
    settings.external.iv        = data.entrySettings.external.iv;               \
    data.clearKey               = clearKey_##keyLn;                             \
    if ( keyLn > 128 && NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 5) {               \
      data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock256;          \
    }                                                                           \
    data.IV                     = IV_128;                                       \
    data.plainText              = plainText;                                    \
    data.cipherText             = CipherText_##algo##_##keyLn##_##algoMode;     \
    pTestTitle                  = "Algorithm "#algo"_"#keyLn", mode "#algoMode", Termination "#termMode; \
    verboseLog( "---> Test#%d %s starts.", count, pTestTitle);       \
}

#    define SECURITY_SET_EXT_CIPHER(settings, data, algo, keyLn, algoMode, termMode, keyNo)    \
{                                                                               \
    BKNI_Memset( &settings, 0, sizeof( settings) );                             \
    settings.algorithm          = NEXUS_CryptographicAlgorithm_e##algo##keyLn;  \
    settings.algorithmMode      = NEXUS_CryptographicAlgorithmMode_e##algoMode; \
    settings.terminationMode    = NEXUS_KeySlotTerminationMode_e##termMode;     \
    settings.external.key       = data.entrySettings.external.key;              \
    settings.external.iv        = data.entrySettings.external.iv;               \
    data.clearKey               = clearKey_##keyLn##_##keyNo;                   \
    data.IV                     = IV_128;                                       \
    data.plainText              = plainText;                                    \
    data.cipherText             = CipherText_##algo##_##keyLn##_##algoMode##_##keyNo;     \
    pTestTitle                  = "Algorithm "#algo"_"#keyLn", mode "#algoMode", Termination "#termMode; \
    verboseLog( "---> Test#%d %s starts.", count, pTestTitle);                 \
}

typedef struct security_keyslotTestData {
    NEXUS_KeySlotBlockEntry entry;
    NEXUS_KeySlotEntrySettings entrySettings;
    NEXUS_KeySlotAllocateSettings allocteSetting;
    NEXUS_KeySlotHandle keyslotHandle;
    bool          keepKeyslot;
    uint8_t      *clearKey;
    uint8_t      *IV;
    uint8_t      *plainText;
    uint8_t      *cipherText;
} security_keyslotTestData;

typedef struct security_keyslotListNode {
    BLST_S_ENTRY(
    security_keyslotListNode ) next;
    NEXUS_KeySlotHandle keyslotHandle;
} security_keyslotListNode;

typedef struct {
    unsigned      slotIndex;                               /* Index into external key-slot table.  */
    struct {
        bool          valid;                               /* Indicates that the associated offset is valid.  */
        unsigned      offset;                              /* offset into external key slot. */
        unsigned      size;
        uint8_t      *pData;
    } key        ,
                  iv;
} external_key_data_t;

static uint8_t xptTSPackets[XPT_TS_PACKET_SIZE] = {
    0x47, 0x09, 0x0c, 0x19,
};

#if 0
static int    securityTest_Keyslot_open_close_single(
    int argc,
    char **argv );
static int    securityTest_Keyslot_block_size(
    int argc,
    char **argv );
static int    securityTest_Keyslot_termination_mode(
    int argc,
    char **argv );
static int    securityTest_Keyslot_block_type(
    int argc,
    char **argv );
static int    securityTest_Keyslot_block_polarity(
    int argc,
    char **argv );
#endif
static int    securityTest_Keyslot_crypto_algorithm(
    NEXUS_KeySlotEntrySettings * pSettings,
    security_keyslotTestData * pData );
static int    securityTest_Keyslot_SC_bits_DMA(
    NEXUS_KeySlotPolarity inputPolarity,
    NEXUS_KeySlotBlockEntry keyEntry,
    NEXUS_KeySlotBlockType block,
    NEXUS_KeySlotPolarity outputPolarity );
static NEXUS_Error securityTest_Keyslot_cases_AES(
    security_keyslotTestData Data );

static NEXUS_KeySlotBlockEntry ScValueToBlockEntry(
    NEXUS_KeySlotPolarity polarity,
    NEXUS_KeySlotBlockType block )
{
    if( polarity >= NEXUS_KeySlotPolarity_eMax || block >= NEXUS_KeySlotBlockType_eMax ) {
        return NEXUS_KeySlotBlockEntry_eMax;
    }

    return block * NEXUS_KeySlotBlockType_eMax + polarity;
}

#if 0
static NEXUS_KeySlotPolarity ScValueToKeyslotPolarity(
    unsigned int scValue )
{
    NEXUS_KeySlotPolarity ScPloarity;

    switch ( scValue ) {
    case 0:
        ScPloarity = NEXUS_KeySlotPolarity_eClear;
        break;
    case 1:
        ScPloarity = NEXUS_KeySlotPolarity_eMax;
        break;
    case 2:
        ScPloarity = NEXUS_KeySlotPolarity_eEven;
        break;
    case 3:
        ScPloarity = NEXUS_KeySlotPolarity_eOdd;
        break;
    default:
        ScPloarity = NEXUS_KeySlotPolarity_eMax;
        break;
    }
    return ScPloarity;
}
#endif

static unsigned int KeyslotPolarityToSC(
    NEXUS_KeySlotPolarity ksPloarity )
{
    unsigned int  ScPloarity = 1;

    switch ( ksPloarity ) {
    case NEXUS_KeySlotPolarity_eClear:
        ScPloarity = 0;
        break;
    case NEXUS_KeySlotPolarity_eMax:
        ScPloarity = 1;
        break;
    case NEXUS_KeySlotPolarity_eEven:
        ScPloarity = 2;
        break;
    case NEXUS_KeySlotPolarity_eOdd:
        ScPloarity = 3;
        break;
    default:
        ScPloarity = 1;         /* Marked as No Key is used */
        break;
    }
    return ScPloarity;
}


#if 0
static char  *PrintScValue(
    unsigned int scValue )
{
    switch ( scValue ) {
    case 0:
        return "Clear";
    case 2:
        return "Even";
    case 3:
        return "Odd";

    case 1:
    default:
        return "No Key";
    }
    return "No Key";
}
#endif

static void compileBtp(
    uint8_t * pBtp,
    external_key_data_t * pBtpData )
{
    unsigned char *p = pBtp;
    unsigned      x = 0;
    unsigned      len = 0;
    unsigned char templateBtp[] = { /* ( 0) */ 0x47,
        /* ( 1) */ 0x00,
        /* ( 2) */ 0x21,
        /* ( 3) */ 0x20,
        /* ( 4) */ 0xb7,
        /* ( 5) */ 0x82,
        /* ( 6) */ 0x45,
        /* ( 7) */ 0x00,
        /* ( 8) */ 0x42,
        /*'B' */
        /* ( 9) */ 0x52,
        /*'R' */
        /* (10) */ 0x43,
        /*'C' */
        /* (11) */ 0x4d,
        /*'M' */
        /* (12) */ 0x00,
        /* (13) */ 0x00,
        /* (14) */ 0x00,
        /* (15) */ 0x1a
            /* security BTP */
    };

    assert( pBtp );
    assert( pBtpData );
    assert( sizeof( templateBtp ) <= XPT_TS_PACKET_SIZE );

    memset( pBtp, 0, XPT_TS_PACKET_SIZE );
    memcpy( pBtp, templateBtp, sizeof( templateBtp ) );

    /* Location of external  keyslot in external keyslot table  */
    pBtp[18] = ( pBtpData->slotIndex >> 8 ) & 0xFF;
    pBtp[19] = pBtpData->slotIndex & 0xFF;

    verboseLog( "\n Slot offset [%d] \n", pBtpData->slotIndex );

    /*pack key into BTP */
    verboseLog( "KEY valid[%d] offset[%d] size[%d]\n", pBtpData->key.valid, pBtpData->key.offset, pBtpData->key.size );
    if( pBtpData->key.valid ) {
        x = 0;

        p = &pBtp[20];          /* start of BTP data section . */
        p += ( pBtpData->key.offset * 16 ); /* locate where to write the key within the BTP data section. */

        len = pBtpData->key.size;

        DEBUG_PRINT_ARRAY( "KEY", len, pBtpData->key.pData );

        pBtpData->key.pData += ( len - 8 ); /*  write the data into BTP in reversed 64bit chunks !! */

        while( len ) {
            memcpy( p, pBtpData->key.pData, MIN( len, 8 ) );    /* set Key   */
            memset( ( p + 8 ), 0xFF, MIN( len, 8 ) );   /* set Mask */
            p += 16;            /* 8 bytes for data, 8 for mask */
            len -= MIN( len, 8 );
            pBtpData->key.pData -= MIN( len, 8 );
        }
    }

    /* pack iv into BTP */
    verboseLog( "IV valid[%d] offset[%d] size[%d]\n", pBtpData->iv.valid, pBtpData->iv.offset, pBtpData->iv.size );
    if( pBtpData->iv.valid ) {
        x = 0;

        p = &pBtp[20];          /* start of BTP data section . */
        p += ( pBtpData->iv.offset * 16 );  /* move to offset withtin BTP for IV */

        len = pBtpData->iv.size;

        DEBUG_PRINT_ARRAY( "IV", len, pBtpData->iv.pData );

        pBtpData->iv.pData += ( len - 8 );  /*  write the data into BTP in reverse!! */

        while( len ) {
            memcpy( p, pBtpData->iv.pData, MIN( len, 8 ) ); /* set IV    */
            memset( p + 8, 0xFF, MIN( len, 8 ) );   /* set Mask */
            p += 16;
            len -= MIN( len, 8 );
            pBtpData->iv.pData -= MIN( len, 8 );
        }
    }

    DEBUG_PRINT_ARRAY( "BTP  0- 19", 20, pBtp );
    DEBUG_PRINT_ARRAY( "BTP 20-188", ( XPT_TS_PACKET_SIZE - 20 ), ( pBtp + 20 ) );

    return;
}

static size_t securityTest_Get_AlogrithmKeySize(
    NEXUS_CryptographicAlgorithm algorithm )
{
    size_t        key_size = 0;

    switch ( algorithm ) {
    case NEXUS_CryptographicAlgorithm_eDes:
        key_size = 8;
        break;
    case NEXUS_CryptographicAlgorithm_e3DesAba:
        key_size = 8 * 2;       /* Actually 8 * 2 */
        break;
    case NEXUS_CryptographicAlgorithm_e3DesAbc:
        key_size = 8 * 3;
        break;
    case NEXUS_CryptographicAlgorithm_eAes128:
        key_size = 128 / 8;
        break;
    case NEXUS_CryptographicAlgorithm_eAes192:
        key_size = 192 / 8;
        break;
    case NEXUS_CryptographicAlgorithm_eAes256:
        key_size = 256 / 8;
        break;
    default:
        /* Invalid */
        key_size = 0;
        BDBG_ERR( ( "Can't get algorithm %d's key size", algorithm ) );
        break;
    }

    return key_size;
}

#if 0
static int securityTest_Keyslot_open_close_single(
    int argc,
    char **argv )
{
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerBlock;
    keyslotAllocSettings.useWithDma = true;
    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_LOG( ( "Can't allocate keyslot\n" ) );
        return -1;
    }

    NEXUS_KeySlot_Free( keyslotHandle );

    return NEXUS_SUCCESS;
}

static int securityTest_Keyslot_block_size(
    int argc,
    char **argv )
{
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    return NEXUS_SUCCESS;
}
static int securityTest_Keyslot_termination_mode(
    int argc,
    char **argv )
{
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    return NEXUS_SUCCESS;
}



static int securityTest_Keyslot_block_type(
    int argc,
    char **argv )
{
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    return NEXUS_SUCCESS;
}
static int securityTest_Keyslot_block_polarity(
    int argc,
    char **argv )
{
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    return NEXUS_SUCCESS;
}
#endif

static void RecPumpDataReadyCallback(
    void *context,
    int param )
{
    BSTD_UNUSED( param );
    BKNI_SetEvent( ( BKNI_EventHandle ) context );
}

static int securityTest_Keyslot_SC_bits_playback(
    NEXUS_KeySlotPolarity inputPolarity,
    NEXUS_KeySlotBlockEntry keyEntry,
    NEXUS_KeySlotBlockType block,
    NEXUS_KeySlotPolarity outputPolarity )
{
    NEXUS_KeySlotEntrySettings settings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    NEXUS_RecpumpHandle recpump;
    BKNI_EventHandle recPumpDataReadyEvent;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_PidChannelHandle cryptoPidChannel;
    security_keyslotTestData data;
    bool          injectTSPackets = true;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    char         *pTestTitle;
    const void   *data_buffer;
    size_t        data_buffer_size = 0;
    char          empty[XPT_TS_PACKET_SIZE - XPT_TS_PACKET_HEAD_SIZE];
    static uint32_t count = 1;

    /* Testing AES 128 CBC SW key encryption. */
    BKNI_Memset( &data, 0, sizeof( data ) );
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Ecb, Clear );
    settings.outputPolarity.specify = true;
    settings.outputPolarity.gPipe = outputPolarity;
    settings.outputPolarity.rPipe = outputPolarity;
    data.entry = keyEntry;
    data.keepKeyslot = true;
    data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
    data.allocteSetting.useWithDma = true;

    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    if( !data.keyslotHandle ) {
        printf( "\nError: NULL keyslot handler.\n" );
        return 1;
    }

    verboseLog( "\nComposite a single XPT packet.\n" );
    CompositTSPackets( xptTSPackets, KeyslotPolarityToSC( inputPolarity ) );

    /* Configure transport input */
    playpump = NEXUS_Playpump_Open( 0, NULL );
    assert( playpump );
    NEXUS_Playpump_GetSettings( playpump, &playpumpSettings );
    playpumpSettings.transportType = IN_XPT_TYPE;
    NEXUS_Playpump_SetSettings( playpump, &playpumpSettings );

    cryptoPidChannel = NEXUS_Playpump_OpenPidChannel( playpump, VIDEO_PID, NULL );

    /* Add the security to the playback. */
    NEXUS_KeySlot_AddPidChannel( data.keyslotHandle, cryptoPidChannel );

    /* Configure transport output */
    BKNI_CreateEvent( &recPumpDataReadyEvent );

    NEXUS_Recpump_GetDefaultOpenSettings( &recpumpOpenSettings );
    /* Configrations for passing a single packet. */
    recpumpOpenSettings.data.atomSize = 0;
    recpumpOpenSettings.data.dataReadyThreshold = 1;
    recpump = NEXUS_Recpump_Open( 0, &recpumpOpenSettings );

    NEXUS_Recpump_GetSettings( recpump, &recpumpSettings );
    recpumpSettings.data.dataReady.callback = RecPumpDataReadyCallback;
    recpumpSettings.data.dataReady.context = recPumpDataReadyEvent;
    recpumpSettings.index.dataReady.callback = NULL;
    recpumpSettings.index.dataReady.context = NULL;
    recpumpSettings.data.overflow.callback = NULL;
    recpumpSettings.data.overflow.context = NULL;
    recpumpSettings.index.overflow.callback = NULL;
    recpumpSettings.index.overflow.context = NULL;
    recpumpSettings.outputTransportType = OUT_XPT_TYPE;
    NEXUS_Recpump_SetSettings( recpump, &recpumpSettings );

    NEXUS_Recpump_GetDefaultAddPidChannelSettings( &addPidChannelSettings );
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    NEXUS_Recpump_AddPidChannel( recpump, cryptoPidChannel, &addPidChannelSettings );

    rc = NEXUS_Playpump_Start( playpump );
    BKNI_Sleep( 7 );
    rc = NEXUS_Recpump_Start( recpump );

    verboseLog( "\nComposite a single XPT packet.\n" );
    CompositTSPackets( xptTSPackets, KeyslotPolarityToSC( outputPolarity ) );
    PRINT_TS_PACKET( xptTSPackets );

    /* Inject the packet to XPT block to be processed by CPD->CA->CPS. */
    while( injectTSPackets ) {
        size_t        size = 0;
        void         *buffer;

        rc = NEXUS_Playpump_GetBuffer( playpump, &buffer, &size );
        assert( !rc );
        if( !size ) {
            BKNI_Sleep( 10 );
            continue;
        }
        if( injectTSPackets ) {
            assert( size > sizeof( xptTSPackets ) );
            BKNI_Memcpy( buffer, xptTSPackets, sizeof( xptTSPackets ) );
            size = sizeof( xptTSPackets );
            injectTSPackets = false;
        }

        rc = NEXUS_Playpump_ReadComplete( playpump, 0, size );
        assert( !rc );
    }

    /* Check the processed packet. */
    /* Checking the output packets' SC value after the packet has gone through cpd-> ca -> cps. */

    if( !NEXUS_Recpump_GetDataBuffer( recpump, &data_buffer, &data_buffer_size ) ) {
        const uint8_t *buffer = ( uint8_t * ) data_buffer;

        if( !data_buffer_size ) {
            rc = BKNI_WaitForEvent( recPumpDataReadyEvent, 3000 );
            if( rc ) {
                printf( "\nError: No data received!\n" );
                goto exit;
            }
        }

        verboseLog( "From recpump buffer (buffer: %p, size: %ld)!\n", data_buffer, ( unsigned long ) data_buffer_size );
        verboseLog( "\nFollowing is the output transport stream packet:\n" );
        PRINT_TS_PACKET( buffer );

        BKNI_Memset( empty, 0, sizeof( empty ) );
        if( !BKNI_Memcmp( &buffer[XPT_TS_PACKET_HEAD_SIZE], empty, sizeof( empty ) ) ) {
            printf( "\nError: the packet coming out of block #%d payload are all 0s, security configs may be wrong.\n",
                    block );
            rc = -1;
        }
        else if( !BKNI_Memcmp
                 ( &buffer[XPT_TS_PACKET_HEAD_SIZE], &xptTSPackets[XPT_TS_PACKET_HEAD_SIZE],
                   XPT_TS_PACKET_SIZE - XPT_TS_PACKET_HEAD_SIZE - 1 ) ) {
            printf
                ( "\nError: the packet coming out of block #%d payload are not encrypted or decrpyted, security configs may be wrong.\n",
                  block );
            rc = -1;
        }
        else if( ( ( buffer[3] & 0xC0 ) >> 6 ) == KeyslotPolarityToSC( outputPolarity ) ) {
            verboseLog( "\nThe packet coming out of block #%d has the correct output SC value %d.\n", block,
                    KeyslotPolarityToSC( outputPolarity ) );
        }
        else {
            BDBG_ERR(( "\nError: The packet coming out of block #%d has the INCORRECT output SC value %d, expected is %d.\n",
                            block,
                            ( buffer[3] & 0xC0 ) >> 6,
                            KeyslotPolarityToSC( outputPolarity ) ));
        }
    }

  exit:
    NEXUS_Recpump_Stop( recpump );
    NEXUS_Recpump_RemoveAllPidChannels( recpump );
    NEXUS_Recpump_Close( recpump );
    NEXUS_Playpump_Stop( playpump );
    if( data.keepKeyslot && data.keyslotHandle ) {
        NEXUS_KeySlot_RemovePidChannel( data.keyslotHandle, cryptoPidChannel );
        NEXUS_KeySlot_Invalidate( data.keyslotHandle );
        NEXUS_KeySlot_Free( data.keyslotHandle );
    }
    NEXUS_Playpump_ClosePidChannel( playpump, cryptoPidChannel );
    NEXUS_Playpump_Close( playpump );
    BKNI_DestroyEvent( recPumpDataReadyEvent );
    count++;
    return rc;
}

static int securityTest_Keyslot_SC_bits_DMA(
    NEXUS_KeySlotPolarity inputPolarity,
    NEXUS_KeySlotBlockEntry keyEntry,
    NEXUS_KeySlotBlockType block,
    NEXUS_KeySlotPolarity outputPolarity )
{
    NEXUS_KeySlotEntrySettings settings;
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    char         *pTestTitle;
    uint8_t      *data_buffer;
    uint8_t      *packet;
    const uint8_t *buffer;
    static uint32_t count = 1;

    /* Testing AES 128 CBC SW key encryption. */
    BKNI_Memset( &data, 0, sizeof( data ) );
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Ecb, Clear );
    settings.outputPolarity.specify = true;
    settings.outputPolarity.gPipe = outputPolarity;
    settings.outputPolarity.rPipe = outputPolarity;
    data.entry = keyEntry;
    data.keepKeyslot = true;
    data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
    data.allocteSetting.useWithDma = true;

    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    if( !data.keyslotHandle ) {
        printf( "\nError: NULL keyslot handler.\n" );
        return 1;
    }

    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &packet );
    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &data_buffer );

    verboseLog( "\nComposite a single XPT packet.\n" );
    CompositTSPackets( xptTSPackets, KeyslotPolarityToSC( inputPolarity ) );

    /* PRINT_TS_PACKET( xptTSPackets ); */

    BKNI_Memcpy( packet, xptTSPackets, sizeof( xptTSPackets ) );

    rc = securityUtil_DmaTransfer( data.keyslotHandle, packet, data_buffer, NEXUS_DmaDataFormat_eMpeg,
                                   XPT_TS_PACKET_SIZE, false );
    if( rc != NEXUS_SUCCESS ) {
        BERR_TRACE( rc );
    }

    if( !BKNI_Memcmp( packet, data_buffer, XPT_TS_PACKET_SIZE ) ) {
        verboseLog
            ( "\n---- Error, packet data has not been encrypted or decrypted: input RC %d, keyBlock %d, keyEntry %d, output RC %d.\n",
              inputPolarity, block, keyEntry, outputPolarity );
        return -1;
    }

    /* PRINT_TS_PACKET( data_buffer ); */

    buffer = ( uint8_t * ) data_buffer;
    if( ( ( buffer[3] & 0xC0 ) >> 6 ) == KeyslotPolarityToSC( outputPolarity ) ) {
        verboseLog( "\nThe packet coming out of block #%d has the correct output SC value %d.\n", block,
                KeyslotPolarityToSC( outputPolarity ) );
    }
    else {
        verboseLog
            ( "\nError: The packet coming out of block #%d has the INCORRECT output SC value %d, expected is %d.\n",
              block, ( buffer[3] & 0xC0 ) >> 6, KeyslotPolarityToSC( outputPolarity ) );
    }

    if( data.keepKeyslot && data.keyslotHandle ) {
        NEXUS_KeySlot_Invalidate( data.keyslotHandle );
        NEXUS_KeySlot_Free( data.keyslotHandle );
    }

    if( data_buffer ) {
        NEXUS_Memory_Free( data_buffer );
    }
    if( packet ) {
        NEXUS_Memory_Free( packet );
    }
    return rc;
}

static int securityTest_Keyslot_crypto_algorithm(
    NEXUS_KeySlotEntrySettings * pSettings,
    security_keyslotTestData * pData )
{
    NEXUS_Error   rc;
    uint8_t      *pSrc,
                 *pDest,
                 *pExternalKey,
                 *pExternalKeyShaddow,
                 *pExpected;
    unsigned      data_size,
                  key_size,
                  iv_size;

    external_key_data_t btp;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotKey slotKey;
    NEXUS_KeySlotIv slotIv;
    NEXUS_KeySlotExternalKeyData extKeyData;
    NEXUS_KeySlotBlockEntry entry = pData->entry;

    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = pData->allocteSetting.slotType;
    keyslotAllocSettings.useWithDma = pData->allocteSetting.useWithDma;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_LOG( ( "Can't allocate keyslot\n" ) );
        return -1;
    }

    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );

    /* keyslotSettings.encryptBeforeRave = true; */

    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = pSettings->algorithm;
    keyslotEntrySettings.algorithmMode = pSettings->algorithmMode;
    keyslotEntrySettings.terminationMode = pSettings->terminationMode;

    if( pSettings->algorithmMode == NEXUS_CryptographicAlgorithmMode_eCounter ) {
        keyslotEntrySettings.counterMode = pSettings->counterMode;
        keyslotEntrySettings.counterSize = pSettings->counterSize;
    }

    data_size = sizeof( plainText );

    /* The actual key size for the algorithm selected. */
    key_size = slotKey.size = securityTest_Get_AlogrithmKeySize( pSettings->algorithm );

    /* IV is 128 bits for AES, 64 bits for DES. */
    iv_size = 128 / 8;

    keyslotEntrySettings.external.iv = pSettings->external.iv;
    keyslotEntrySettings.external.key = pSettings->external.key;

    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;
    keyslotEntrySettings.outputPolarity.specify = pSettings->outputPolarity.specify;
    keyslotEntrySettings.outputPolarity.gPipe = pSettings->outputPolarity.gPipe;
    keyslotEntrySettings.outputPolarity.rPipe = pSettings->outputPolarity.rPipe;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    pSrc = pDest = pExternalKey = pExternalKeyShaddow = NULL;

    /* Config key and IV. */

    if( pSettings->external.iv || pSettings->external.key ) {

        /* key or IV from BTP. */
        rc = NEXUS_KeySlot_GetEntryExternalKeySettings( keyslotHandle, entry, &extKeyData );
        if( rc != NEXUS_SUCCESS ) {
            return BERR_TRACE( rc );
        }

        BKNI_Memset( &btp, 0, sizeof( btp ) );
        btp.slotIndex = extKeyData.slotIndex;

        if( extKeyData.key.valid ) {
            btp.key.valid = true;
            btp.key.offset = extKeyData.key.offset;
            btp.key.size = key_size;
            btp.key.pData = pData->clearKey;
        }

        if( extKeyData.iv.valid ) {
            btp.iv.valid = true;
            btp.iv.offset = extKeyData.iv.offset;
            btp.iv.size = iv_size;
            btp.iv.pData = pData->IV;
        }

        /* Zeus 4 ...  external Key and IV are encapsulated within BTP and inserted in front of DMA tansfer */
        NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKey );
        NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKeyShaddow );
        memset( pExternalKey, 0, XPT_TS_PACKET_SIZE );
        memset( pExternalKeyShaddow, 0, XPT_TS_PACKET_SIZE );

        /* compile a Broadcom Transport Packet into pExternalKey */
        compileBtp( pExternalKey, &btp );

        verboseLog( "\n---- %s() loads the clear key and IV from BTP to key entry #%d.\n", __FUNCTION__, entry );
        rc = securityUtil_DmaTransfer( keyslotHandle, pExternalKey, pExternalKeyShaddow, NEXUS_DmaDataFormat_eBlock,
                                       XPT_TS_PACKET_SIZE, true );
        if( rc != NEXUS_SUCCESS ) {
            BERR_TRACE( rc );
        }
    }
    else {

        /* key or IV from application. */

        verboseLog( "\n---- %s() loads the clear key to key entry #%d.\n", __FUNCTION__, entry );
        BKNI_Memcpy( slotKey.key, pData->clearKey, key_size );
        rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );
        if( rc != NEXUS_SUCCESS ) {
            return BERR_TRACE( rc );
        }

        if( pData->IV && ( pSettings->algorithmMode != NEXUS_CryptographicAlgorithmMode_eEcb ) ) {
            verboseLog( "\n---- %s() loads IV for the key entry #%d.\n", __FUNCTION__, entry );
            slotIv.size = iv_size;
            BKNI_Memcpy( slotIv.iv, pData->IV, iv_size );
            rc = NEXUS_KeySlot_SetEntryIv( keyslotHandle, entry, &slotIv, NULL );
            if( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE( rc );
            }
        }
    }

    if( ( !pSettings->outputPolarity.specify ) && pData->allocteSetting.useWithDma ) {

        NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
        NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

        if( entry == NEXUS_KeySlotBlockEntry_eCpsClear ) {
            BKNI_Memcpy( pSrc, plainText, data_size );
            pExpected = pData->cipherText;
        }
        else if( entry == NEXUS_KeySlotBlockEntry_eCpdClear ) {
            BKNI_Memcpy( pSrc, pData->cipherText, data_size );
            pExpected = plainText;
        }

        BKNI_Memset( pDest, 0, data_size );
        rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
        if( rc != NEXUS_SUCCESS ) {
            BERR_TRACE( rc );
        }
    }

    if( pData->keepKeyslot ) {
        pData->keyslotHandle = keyslotHandle;
    }
    else {
        NEXUS_KeySlot_Invalidate( keyslotHandle );
        NEXUS_KeySlot_Free( keyslotHandle );
    }

    if( !pSettings->outputPolarity.specify ) {
        if( BKNI_Memcmp( pDest, pExpected, data_size ) ) {
            verboseLog( ( "    Test FAILED!\n" ) );
            DEBUG_PRINT_ARRAY( "Key:", key_size, pData->clearKey );
            DEBUG_PRINT_ARRAY( "Iv:", iv_size, pData->IV );
            DEBUG_PRINT_ARRAY( "Source:", data_size, pSrc );
            DEBUG_PRINT_ARRAY( "Destination", data_size, pDest );
            DEBUG_PRINT_ARRAY( "Expected result", data_size, pExpected );
            rc = NEXUS_INVALID_PARAMETER;
        }
        else {
            verboseLog( "    Test PASSED!\n" );
            rc = NEXUS_SUCCESS;
        }
    }

    if( pSrc )
        NEXUS_Memory_Free( pSrc );
    if( pDest )
        NEXUS_Memory_Free( pDest );
    if( pExternalKey )
        NEXUS_Memory_Free( pExternalKey );
    if( pExternalKeyShaddow )
        NEXUS_Memory_Free( pExternalKeyShaddow );

    return rc;
}

/* The tests of the same AES key length use the same keys. */
static NEXUS_Error securityTest_Keyslot_cases_AES(
    security_keyslotTestData data )
{
    NEXUS_KeySlotEntrySettings settings;
    char         *pTestTitle;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    static uint32_t count = 1;

    /* Testing AES 128 Cbc encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Cbc, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 192 Ecb encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 192, Ecb, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

#    if (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 5)
    /* Testing AES 256 Ecb encryption and decryption.
     * It has not been supported for Zeus 4.
     */
    SECURITY_SET_CIPHER( settings, data, Aes, 256, Ecb, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );
#    endif

    /* Testing AES 128 CBC encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Cbc, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 192 ECB encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 192, Ecb, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 128 ECB encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Ecb, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 128 CBC encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 128, Cbc, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 192 ECB encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 192, Ecb, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    /* Testing AES 192 CBC encryption and decryption. */
    SECURITY_SET_CIPHER( settings, data, Aes, 192, Cbc, Clear );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    return rc;
}

/* The tests of the same AES key length use three different alternated keys. */
static NEXUS_Error securityTest_Keyslot_cases_AES_different_keys(
    security_keyslotTestData data )
{
    NEXUS_KeySlotEntrySettings settings;
    char         *pTestTitle;
    NEXUS_SecurityCapabilities securityCapabilities;
    uint8_t       numKeySlotsForType,
                  keySlotType;
    security_keyslotListNode *keyslotListNode,
                 *last;
    static uint32_t count = 1;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BLST_S_HEAD( securityTest_KeySlotList_t, security_keyslotListNode ) keyslotList;
    BLST_S_INIT( &keyslotList );

    NEXUS_GetSecurityCapabilities( &securityCapabilities );

    /* Allocate all the large keyslots in one go, BHSM_KeyslotType_eMulti2 is not handled, so use eMax -1. */
    for( keySlotType = 0; keySlotType < NEXUS_KeySlotType_eMax; keySlotType++ ) {
        for( numKeySlotsForType = 0; numKeySlotsForType < securityCapabilities.numKeySlotsForType[keySlotType];
             numKeySlotsForType++ ) {

            /* Testing AES 128 Cbc encryption and decryption with three alternated keys. */
            switch ( count % 3 ) {
            case 0:
                SECURITY_SET_EXT_CIPHER( settings, data, Aes, 128, Cbc, Clear, 0 );
                break;
            case 1:
                SECURITY_SET_EXT_CIPHER( settings, data, Aes, 128, Cbc, Clear, 1 );
                break;
            case 2:
                SECURITY_SET_EXT_CIPHER( settings, data, Aes, 128, Cbc, Clear, 2 );
                break;
            default:
                BDBG_ERR( ( "Can't set the clear test and cipher test for keyslot type %d, count %d, slot number %d",
                            keySlotType, count, numKeySlotsForType ) );
            }

            data.allocteSetting.slotType = keySlotType;
            data.allocteSetting.useWithDma = true;
            data.keepKeyslot = true;

            rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
            SECURITY_CHECK_TEST_RETURN( rc );

            keyslotListNode = BKNI_Malloc( sizeof( *keyslotListNode ) );
            keyslotListNode->keyslotHandle = data.keyslotHandle;
            verboseLog( "Allocated keyslot %p for type [%d], slot number [%d] on the HOST.\n",
                    ( void * ) data.keyslotHandle, keySlotType, numKeySlotsForType );

            BLST_S_INSERT_HEAD( &keyslotList, keyslotListNode, next );
        }
    }

    /* Free all the allocated keyslots. */
    verboseLog( "\n\n Freeing all the allocated keyslots.\n\n" );
    keyslotListNode = BLST_S_FIRST( &keyslotList );

    while( keyslotListNode && keyslotListNode->keyslotHandle ) {
        verboseLog( "\nFreeing keyslot %p", ( void * ) keyslotListNode->keyslotHandle );
        NEXUS_KeySlot_Free( keyslotListNode->keyslotHandle );
        last = keyslotListNode;
        keyslotListNode = BLST_S_NEXT( keyslotListNode, next );
        BKNI_Free( last );
    }

    /* Multi2KeySlots is not handled . */

    return rc;
}

int securityTest_Keyslot_open_close_all(
    int argc,
    char **argv )
{
    NEXUS_SecurityCapabilities securityCapabilities;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    uint8_t       numKeySlotsForType,
                  keySlotType;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    security_keyslotListNode *keyslotListNode,
                 *last;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BLST_S_HEAD( securityTest_KeySlotList_t, security_keyslotListNode ) keyslotList;
    BLST_S_INIT( &keyslotList );

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    NEXUS_GetSecurityCapabilities( &securityCapabilities );

    /* Allocate all the large keyslots in one go, BHSM_KeyslotType_eMulti2 is not handled, so use eMax -1. */
    for( keySlotType = 0; keySlotType < NEXUS_KeySlotType_eMax; keySlotType++ ) {
        for( numKeySlotsForType = 0; numKeySlotsForType < securityCapabilities.numKeySlotsForType[keySlotType];
             numKeySlotsForType++ ) {
            NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
            keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
            keyslotAllocSettings.slotType = keySlotType;
            keyslotAllocSettings.useWithDma = true;
            keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
            if( !keyslotHandle ) {
                BDBG_ERR( ( "Can't allocate keyslot for type [%d], slot number [%d] on the HOST.\n", keySlotType,
                            numKeySlotsForType ) );

                rc = NEXUS_INVALID_PARAMETER;
                goto exit;
            }
            keyslotListNode = BKNI_Malloc( sizeof( *keyslotListNode ) );

            keyslotListNode->keyslotHandle = keyslotHandle;
            verboseLog( "Allocated keyslot %p for type [%d], slot number [%d] on the HOST.\n", ( void * ) keyslotHandle,
                    keySlotType, numKeySlotsForType );

            BLST_S_INSERT_HEAD( &keyslotList, keyslotListNode, next );
        }
    }

  exit:

    /* Free all the allocated keyslots. */
    verboseLog( "\n\n Freeing all the allocated keyslots.\n\n" );
    keyslotListNode = BLST_S_FIRST( &keyslotList );

    while( keyslotListNode && keyslotListNode->keyslotHandle ) {
        verboseLog( "Freeing keyslot %p\n", ( void * ) keyslotListNode->keyslotHandle );
        NEXUS_KeySlot_Free( keyslotListNode->keyslotHandle );
        last = keyslotListNode;
        keyslotListNode = BLST_S_NEXT( keyslotListNode, next );
        BKNI_Free( last );
    }

    /* Multi2KeySlots is not handled at the moment. */

    return rc;
}

int securityTest_Keyslot_crypto_DES(
    int argc,
    char **argv )
{
    NEXUS_KeySlotEntrySettings settings;
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    char         *pTestTitle;
    static uint32_t count = 1;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    /* Add the new tests here */

    BKNI_Memset( &settings, 0, sizeof( settings ) );
    BKNI_Memset( &data, 0, sizeof( data ) );
    if( strcmp( "encryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    }
    else if( strcmp( "decryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    }

    settings.algorithm = NEXUS_CryptographicAlgorithm_eDes;
    settings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    settings.solitaryMode = NEXUS_KeySlotTerminationSolitaryMode_eClear;

    data.clearKey = clearKey_64;
    data.plainText = plainText;
    data.cipherText = CipherText_Des_64_Ecb;
    data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
    data.allocteSetting.useWithDma = true;
    pTestTitle = "Algorithm DES";
    verboseLog( "---> Test#%d %s %s starts.", count, pTestTitle, argv[0] );

    /* Testing DES Ecb encryption and decryption. */
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    return rc;
}

int securityTest_Keyslot_crypto_TDES(
    int argc,
    char **argv )
{
    NEXUS_KeySlotEntrySettings settings;
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;
    char         *pTestTitle;
    static uint32_t count = 1;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    /* Add the new tests here */

    BKNI_Memset( &settings, 0, sizeof( settings ) );
    BKNI_Memset( &data, 0, sizeof( data ) );
    if( strcmp( "encryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    }
    else if( strcmp( "decryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    }

    settings.algorithm = NEXUS_CryptographicAlgorithm_e3DesAba;
    settings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    data.clearKey = clearKey_ABA;
    data.IV = IV_128;
    data.plainText = plainText;
    data.cipherText = CipherText_TDes_112;
    data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
    data.allocteSetting.useWithDma = true;
    pTestTitle = "Algorithm TDES";

    verboseLog( "---> Test#%d %s starts.", count, pTestTitle );
    rc = securityTest_Keyslot_crypto_algorithm( &settings, &data );
    SECURITY_CHECK_TEST_RETURN( rc );

    return rc;
}

int securityTest_Keyslot_crypto_AES(
    int argc,
    char **argv )
{
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    BKNI_Memset( &data, 0, sizeof( data ) );
    if( strcmp( "encryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    }
    else if( strcmp( "decryption", argv[0] ) == 0 ) {
        data.entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    }

    data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
    data.allocteSetting.useWithDma = true;

    rc = securityTest_Keyslot_cases_AES( data );

    return rc;
}

int securityTest_Keyslot_external_iv_key(
    int argc,
    char **argv )
{
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    if( strcmp( "AES_CBC", argv[0] ) == 0 ) {

        BKNI_Memset( &data, 0, sizeof( data ) );

        data.allocteSetting.slotType = NEXUS_KeySlotType_eIvPerBlock;
        data.allocteSetting.useWithDma = true;

        data.entrySettings.external.iv = true;
        data.entrySettings.external.key = true;

        /* Testing AES 128 CBC SW key decryption. */
        data.entry = NEXUS_KeySlotBlockEntry_eCpdClear;

        rc = securityTest_Keyslot_cases_AES( data );

        /* Testing AES 128 CBC SW key encryption. */
        data.entry = NEXUS_KeySlotBlockEntry_eCpsClear;

        rc = securityTest_Keyslot_cases_AES( data );

    }
    return rc;
}

int securityTest_Keyslot_all_external_iv_keys(
    int argc,
    char **argv )
{
    security_keyslotTestData data;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    if( strcmp( "AES_CBC", argv[0] ) == 0 ) {

        BKNI_Memset( &data, 0, sizeof( data ) );

        data.entrySettings.external.iv = true;
        data.entrySettings.external.key = true;

        /* Testing AES 128 CBC SW key decryption. */
        data.entry = NEXUS_KeySlotBlockEntry_eCpdClear;

        rc = securityTest_Keyslot_cases_AES_different_keys( data );
        if( rc ) {
            printf( "\nFAILED: decryption operations with different external keys and IV." );
        }
        /* Testing AES 128 CBC SW key encryption. */ data.entry = NEXUS_KeySlotBlockEntry_eCpsClear;

        rc = securityTest_Keyslot_cases_AES_different_keys( data );
        if( rc ) {
            printf( "\nFAILED: encryption operations with different external keys and IV." );
        }

    }

    return rc;
}

int securityTest_Keyslot_SC_bits_rec_playback(
    int argc,
    char **argv )
{
    NEXUS_KeySlotPolarity CpdInputPacketSCValue = NEXUS_KeySlotPolarity_eOdd;   /* The SC value of the packet going into XPT */
    NEXUS_KeySlotPolarity CpsOutputPacketSCValue = NEXUS_KeySlotPolarity_eOdd;  /* The SC value of the packet going out of Cps */
    NEXUS_KeySlotPolarity CpdOutputPacketSCValue = NEXUS_KeySlotPolarity_eClear;    /* The SC value of the packet going out of Cpd */
    NEXUS_KeySlotPolarity CaOutputPacketSCValue = NEXUS_KeySlotPolarity_eEven;  /* The SC value of the packet going out of Ca */
    NEXUS_KeySlotBlockEntry cpdKeyEntry,
                  caKeyEntry,
                  cpsKeyEntry;
    int           count = 0;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    cpdKeyEntry = ScValueToBlockEntry( CpdInputPacketSCValue, NEXUS_KeySlotBlockType_eCpd );
    caKeyEntry = ScValueToBlockEntry( CpdOutputPacketSCValue, NEXUS_KeySlotBlockType_eCa );
    cpsKeyEntry = ScValueToBlockEntry( CaOutputPacketSCValue, NEXUS_KeySlotBlockType_eCps );

    verboseLog( "\nCPDKeyEntry=%d, CAKeyEntry=%d, CPSKeyEntry=%d.\n", cpdKeyEntry, caKeyEntry, cpsKeyEntry );

    rc = securityTest_Keyslot_SC_bits_playback( CpdInputPacketSCValue,
                                                cpdKeyEntry, NEXUS_KeySlotBlockType_eCpd, CpdOutputPacketSCValue );
    if( rc == NEXUS_SUCCESS ) {
        verboseLog
            ( "\n--- Test #%d: XPT packet with SC[%d] goes to Cpd, key entry[%d], output SC[%d] passed.\n",
              count++, CpdInputPacketSCValue, cpdKeyEntry, CpdOutputPacketSCValue );
    }
    else {
        verboseLog
            ( "\n--- Test #%d: XPT packet with SC[%d] goes to Cpd, key entry[%d], output SC[%d] failed.\n",
              count++, CpdInputPacketSCValue, cpdKeyEntry, CpdOutputPacketSCValue );
        return rc;
    }

    rc = securityTest_Keyslot_SC_bits_playback( CpdOutputPacketSCValue,
                                                caKeyEntry, NEXUS_KeySlotBlockType_eCa, CaOutputPacketSCValue );
    if( rc == NEXUS_SUCCESS ) {
        verboseLog
            ( "\n--- Test #%d: XPT packet from Cpd with SC[%d] goes to Ca, key entry[%d], output SC[%d] passed.\n",
              count++, CpdOutputPacketSCValue, caKeyEntry, CaOutputPacketSCValue );
    }
    else {
        verboseLog
            ( "\n--- Test #%d: XPT packet from Cpd with SC[%d] goes to Ca, key entry[%d], output SC[%d] failed.\n",
              count++, CpdOutputPacketSCValue, caKeyEntry, CaOutputPacketSCValue );
        return rc;
    }

    rc = securityTest_Keyslot_SC_bits_playback( CaOutputPacketSCValue,
                                                cpsKeyEntry, NEXUS_KeySlotBlockType_eCps, CpsOutputPacketSCValue );
    if( rc == NEXUS_SUCCESS ) {
        verboseLog
            ( "\n--- Test #%d: XPT packet from Ca with SC[%d] goes to Cps, key entry[%d], output SC[%d] passed.\n",
              count++, CaOutputPacketSCValue, cpsKeyEntry, CpsOutputPacketSCValue );
    }
    else {
        verboseLog
            ( "\n--- Test #%d: XPT packet from Ca with SC[%d] goes to Cps, key entry[%d], output SC[%d] passed.\n",
              count++, CaOutputPacketSCValue, cpsKeyEntry, CpsOutputPacketSCValue );
        return rc;
    }

    return NEXUS_SUCCESS;
}

int securityTest_Keyslot_SC_bits(
    int argc,
    char **argv )
{
    NEXUS_Error   rc = NEXUS_SUCCESS;
    NEXUS_KeySlotPolarity CpdInputPacketSCValue;           /* The SC value of the packet going into XPT */
    NEXUS_KeySlotPolarity CpdOutputPacketSCValue;          /* The SC value of the packet going out of Cpd */
    NEXUS_KeySlotPolarity CaOutputPacketSCValue;           /* The SC value of the packet going out of Ca */
    NEXUS_KeySlotPolarity CpsOutputPacketSCValue;          /* The SC value of the packet going out of Cps */
    NEXUS_KeySlotBlockEntry cpdKeyEntry,
                  caKeyEntry,
                  cpsKeyEntry;
    uint16_t      count = 0;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    for( CpdInputPacketSCValue = NEXUS_KeySlotPolarity_eOdd;
         CpdInputPacketSCValue < NEXUS_KeySlotPolarity_eMax; CpdInputPacketSCValue++ ) {

        for( CpdOutputPacketSCValue = NEXUS_KeySlotPolarity_eOdd; CpdOutputPacketSCValue < NEXUS_KeySlotPolarity_eMax;
             CpdOutputPacketSCValue++ ) {

            for( CaOutputPacketSCValue = NEXUS_KeySlotPolarity_eOdd; CaOutputPacketSCValue < NEXUS_KeySlotPolarity_eMax;
                 CaOutputPacketSCValue++ ) {

                for( CpsOutputPacketSCValue = NEXUS_KeySlotPolarity_eOdd;
                     CpsOutputPacketSCValue < NEXUS_KeySlotPolarity_eMax; CpsOutputPacketSCValue++ ) {

                    cpdKeyEntry = ScValueToBlockEntry( CpdInputPacketSCValue, NEXUS_KeySlotBlockType_eCpd );
                    caKeyEntry = ScValueToBlockEntry( CpdOutputPacketSCValue, NEXUS_KeySlotBlockType_eCa );
                    cpsKeyEntry = ScValueToBlockEntry( CaOutputPacketSCValue, NEXUS_KeySlotBlockType_eCps );

                    verboseLog( "\nCPDKeyEntry=%d, CAKeyEntry=%d, CPSKeyEntry=%d.\n",
                            cpdKeyEntry, caKeyEntry, cpsKeyEntry );

                    rc = securityTest_Keyslot_SC_bits_DMA( CpdInputPacketSCValue,
                                                           cpdKeyEntry,
                                                           NEXUS_KeySlotBlockType_eCpd, CpdOutputPacketSCValue );
                    if( rc == NEXUS_SUCCESS ) {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet with SC[%d] goes to Cpd, key entry[%d], output SC[%d] passed.\n",
                              count++, CpdInputPacketSCValue, cpdKeyEntry, CpdOutputPacketSCValue );
                    }
                    else {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet with SC[%d] goes to Cpd, key entry[%d], output SC[%d] failed.\n",
                              count++, CpdInputPacketSCValue, cpdKeyEntry, CpdOutputPacketSCValue );
                        return rc;
                    }

                    rc = securityTest_Keyslot_SC_bits_DMA( CpdOutputPacketSCValue,
                                                           caKeyEntry,
                                                           NEXUS_KeySlotBlockType_eCa, CaOutputPacketSCValue );
                    if( rc == NEXUS_SUCCESS ) {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet from Cpd with SC[%d] goes to Ca, key entry[%d], output SC[%d] passed.\n",
                              count++, CpdOutputPacketSCValue, caKeyEntry, CaOutputPacketSCValue );
                    }
                    else {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet from Cpd with SC[%d] goes to Ca, key entry[%d], output SC[%d] failed.\n",
                              count++, CpdOutputPacketSCValue, caKeyEntry, CaOutputPacketSCValue );
                        return rc;
                    }

                    rc = securityTest_Keyslot_SC_bits_DMA( CaOutputPacketSCValue,
                                                           cpsKeyEntry,
                                                           NEXUS_KeySlotBlockType_eCps, CpsOutputPacketSCValue );
                    if( rc == NEXUS_SUCCESS ) {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet from Ca with SC[%d] goes to Cps, key entry[%d], output SC[%d] passed.\n",
                              count++, CaOutputPacketSCValue, cpsKeyEntry, CpsOutputPacketSCValue );
                    }
                    else {
                        verboseLog
                            ( "\n--- Test #%d: XPT packet from Ca with SC[%d] goes to Cps, key entry[%d], output SC[%d] passed.\n",
                              count++, CaOutputPacketSCValue, cpsKeyEntry, CpsOutputPacketSCValue );
                        return rc;
                    }

                }
            }
        }
    }

    return NEXUS_SUCCESS;
}

#endif /* #if NEXUS_HAS_SECURITY */
