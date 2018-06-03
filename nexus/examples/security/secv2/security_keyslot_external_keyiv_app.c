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

#include "nexus_security.h"
#include "nexus_keyslot.h"
#include "security_utils.h"
#include "security_test_vectors_clear_key.h"

typedef struct {
    unsigned      slotIndex;                               /* Index into external key-slot table.  */
    struct {
        bool          valid;                               /* Indicates that the associated offset is valid.  */
        unsigned      offset;                              /* offset into external key slot. */
        unsigned      size;
        uint8_t      *pData;
    } key, iv;
} external_key_data_t;

static void compileBtp(
    uint8_t * pBtp,
    external_key_data_t * pBtpData )
{
    unsigned char *p = pBtp;
    unsigned      len = 0;
    unsigned char templateBtp[] = {
        /* ( 0) */ 0x47,
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

    BDBG_ASSERT( pBtp );
    BDBG_ASSERT( pBtpData );
    BDBG_ASSERT( sizeof( templateBtp ) <= XPT_TS_PACKET_SIZE );

    BKNI_Memset( pBtp, 0, XPT_TS_PACKET_SIZE );
    BKNI_Memcpy( pBtp, templateBtp, sizeof( templateBtp ) );

    /* Location of external  keyslot in external keyslot table  */
    pBtp[18] = ( pBtpData->slotIndex >> 8 ) & 0xFF;
    pBtp[19] = pBtpData->slotIndex & 0xFF;

    BDBG_LOG( ( "\n Slot offset [%d] \n", pBtpData->slotIndex ) );

    /* Pack key into BTP */
    BDBG_LOG( ( "KEY valid[%d] offset[%d] size[%d]\n", pBtpData->key.valid, pBtpData->key.offset,
                pBtpData->key.size ) );
    if( pBtpData->key.valid ) {

        p = &pBtp[20];          /* start of BTP data section . */
        p += ( pBtpData->key.offset * 16 ); /* locate where to write the key within the BTP data section. */

        len = pBtpData->key.size;

        DEBUG_PRINT_ARRAY( "KEY", len, pBtpData->key.pData );

        pBtpData->key.pData += ( len - 8 ); /*  write the data into BTP in reversed 64bit chunks !! */

        while( len ) {
            BKNI_Memcpy( p, pBtpData->key.pData, MIN( len, 8 ) );   /* set Key   */
            BKNI_Memset( ( p + 8 ), 0xFF, MIN( len, 8 ) );  /* set Mask */
            p += 16;            /* 8 bytes for data, 8 for mask */
            len -= MIN( len, 8 );
            pBtpData->key.pData -= MIN( len, 8 );
        }
    }

    /* pack iv into BTP */
    BDBG_LOG( ( "IV valid[%d] offset[%d] size[%d]\n", pBtpData->iv.valid, pBtpData->iv.offset, pBtpData->iv.size ) );
    if( pBtpData->iv.valid ) {

        p = &pBtp[20];          /* start of BTP data section . */
        p += ( pBtpData->iv.offset * 16 );  /* move to offset withtin BTP for IV */

        len = pBtpData->iv.size;

        DEBUG_PRINT_ARRAY( "IV", len, pBtpData->iv.pData );

        pBtpData->iv.pData += ( len - 8 );  /*  write the data into BTP in reverse!! */

        while( len ) {
            BKNI_Memcpy( p, pBtpData->iv.pData, MIN( len, 8 ) );    /* set IV    */
            BKNI_Memset( p + 8, 0xFF, MIN( len, 8 ) );  /* set Mask */
            p += 16;
            len -= MIN( len, 8 );
            pBtpData->iv.pData -= MIN( len, 8 );
        }
    }

    DEBUG_PRINT_ARRAY( "BTP 0 - 19", 20, pBtp );
    DEBUG_PRINT_ARRAY( "BTP 20-188", ( XPT_TS_PACKET_SIZE - 20 ), ( pBtp + 20 ) );

    return;
}

int main(
    int argc,
    char **argv )
{
    NEXUS_KeySlotHandle keyslotHandle = NULL;

    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eMax;
    unsigned      data_size, key_size, iv_size;
    external_key_data_t btp;
    NEXUS_KeySlotExternalKeyData extKeyData;
    uint8_t      *pSrc = NULL, *pDest = NULL, *pExternalKey = NULL, *pExternalKeyShaddow = NULL;
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* Allocate a key slot. */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerBlock;
    keyslotAllocSettings.useWithDma = true;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_ERR( ( "\nError: Can't allocate keyslot\n" ) );
        return NEXUS_NOT_AVAILABLE;
    }

    /* Configure the keyslot. */
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Configure a key entry for encryption. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    /* Specify to use external IV and key from BTP. */
    keyslotEntrySettings.external.iv = true;
    keyslotEntrySettings.external.key = true;

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eCbc;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    pSrc = pDest = pExternalKey = pExternalKeyShaddow = NULL;

    /* Following steps is for loading the key or IV from BTP. */
    rc = NEXUS_KeySlot_GetEntryExternalKeySettings( keyslotHandle, entry, &extKeyData );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* The actual key size for the algorithm selected. */
    key_size = securityGetAlogrithmKeySize( NEXUS_CryptographicAlgorithm_eAes128 );

    /* IV is 128 bits for AES, 64 bits for DES. */
    iv_size = 128 / 8;

    BKNI_Memset( &btp, 0, sizeof( btp ) );
    btp.slotIndex = extKeyData.slotIndex;

    if( extKeyData.key.valid ) {
        btp.key.valid = true;
        btp.key.offset = extKeyData.key.offset;
        btp.key.size = key_size;
        btp.key.pData = clearKey_128;
    }

    if( extKeyData.iv.valid ) {
        btp.iv.valid = true;
        btp.iv.offset = extKeyData.iv.offset;
        btp.iv.size = iv_size;
        btp.iv.pData = IV_128;
    }

    /* Zeus 4 ...  external Key and IV are encapsulated within BTP and inserted in front of DMA tansfer */
    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKey );
    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKeyShaddow );
    BKNI_Memset( pExternalKey, 0, XPT_TS_PACKET_SIZE );
    BKNI_Memset( pExternalKeyShaddow, 0, XPT_TS_PACKET_SIZE );

    /* Compile a Broadcom Transport Packet into pExternalKey */
    compileBtp( pExternalKey, &btp );

    /* Load the loaded key and IV from BTP to the key entry specified. */
    rc = securityUtil_DmaTransfer( keyslotHandle, pExternalKey, pExternalKeyShaddow, NEXUS_DmaDataFormat_eBlock,
                                   XPT_TS_PACKET_SIZE, true );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Using loaded clear key as the root key to encrypt the plain text, cipher is pDest. */
    data_size = sizeof( plainText );

    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    BKNI_Memcpy( pSrc, plainText, data_size );
    BKNI_Memset( pDest, 0, data_size );

    /* Encrypt the plainText to the cipherText */
    rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }
    else {
        /* Check the encryption result. */
        if( BKNI_Memcmp( pDest, CipherText_Aes_128_Cbc, data_size ) ) {
            BDBG_ERR( ( "    Test FAILED!\n" ) );
        }
        else {
            BDBG_LOG( ( "    Test PASSED!\n" ) );
        }
    }

exit:

    if( pSrc ) NEXUS_Memory_Free( pSrc );
    if( pDest ) NEXUS_Memory_Free( pDest );
    if( pExternalKey ) NEXUS_Memory_Free( pExternalKey );
    if( pExternalKeyShaddow ) NEXUS_Memory_Free( pExternalKeyShaddow );
    if (keyslotHandle) NEXUS_KeySlot_Free( keyslotHandle );

    securityUtil_PlatformUnInit(  );

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
