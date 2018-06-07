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

#    include "nexus_security.h"
#    include "nexus_keyslot.h"
#    include "security_utils.h"
#    include "security_test_vectors_clear_key.h"

#    define NUM_PACKETS (1)
#    define TEST_WITH_TIMESTAMP (false)

#    if TEST_WITH_TIMESTAMP
#        define XPT_PACKET_SIZE   (192*NUM_PACKETS)
#    else
#        define XPT_PACKET_SIZE   (188*NUM_PACKETS)
#    endif

int main(
    int argc,
    char **argv )
{
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotKey slotKey;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eMax;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    uint8_t      *pSrc = NULL, *pDest = NULL, *pTemp = NULL;
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    unsigned char cleardeskey[16] =
        { 0xC6, 0x82, 0xEC, 0xB8, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* There are two DMA transfers of a XTP packet: [pSrc] -> CPS -> [pDest] -> CPD -> [pTemp].
       There are two keys for CPS and CPD each.
       We use the same keyslot for the above transfers with invalidating the keyslot's 9 key entry before the 2nd transfer.
       Nevertheless, the keys for CPS and CPD (or CA) can be the same polarity without the two keys being used simultanously
       in the following pipeline. The SC bits are the same between [pSrc] and [pTemp]:
       CPD [clear, odd, even ] -> CA [clear, odd, even ] -> CPS  [clear, odd, even ]
       This is more flexible than that is shown with example security_keyslot_mpeg_packet_crypto.c
    */

    NEXUS_Memory_Allocate( XPT_PACKET_SIZE, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( XPT_PACKET_SIZE, NULL, ( void ** ) &pDest );
    NEXUS_Memory_Allocate( XPT_PACKET_SIZE, NULL, ( void ** ) &pTemp );

    /* Allocate a key slot. */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerSlot;
    keyslotAllocSettings.useWithDma = true;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_ERR( ( "\nError: Can't allocate keyslot\n" ) );
        rc = BERR_TRACE( NEXUS_NOT_AVAILABLE );
        goto exit;
    }

    /* Configure the encryption keyslot. */
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Configure a key entry to encrypt. */
    entry = NEXUS_KeySlotBlockEntry_eCpsEven;
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_e3DesAba;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = false;
    keyslotEntrySettings.gPipeEnable = true;
    keyslotEntrySettings.outputPolarity.specify = true;
    keyslotEntrySettings.outputPolarity.gPipe = NEXUS_KeySlotPolarity_eEven;
    keyslotEntrySettings.outputPolarity.rPipe = NEXUS_KeySlotPolarity_eEven;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    BDBG_LOG( ( "Loads the clear key to key entry #%d.\n", entry ) );
    BKNI_Memcpy( slotKey.key, cleardeskey, sizeof( cleardeskey ) );
    slotKey.size = sizeof( cleardeskey );

    rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Compose a XPT packet to src buffer */
    CompositTSPackets( pSrc, XPT_PACKET_SIZE, 2 /* Even polority key used in Cps */  );

    BDBG_LOG( ( "Encrypt data with the key in key entry #%d.\n", entry ) );
    NEXUS_Memory_FlushCache (pSrc, XPT_PACKET_SIZE);
    /* Encrypt src data to dest */
    rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eMpeg, XPT_PACKET_SIZE, false );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Check the encryption result */
    if( !BKNI_Memcmp( pDest, pSrc, XPT_PACKET_SIZE ) ) {
        BDBG_ERR( ( "    Test FAILED!, The data has not been encrypted. \n" ) );
        goto exit;
    }

    /* Remove the key in CPS */
    NEXUS_KeySlot_Invalidate( keyslotHandle );

    /* Set up decryption */
    entry = NEXUS_KeySlotBlockEntry_eCpdEven; /* encryption output packet has even polority. */
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_e3DesAba;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = false;
    keyslotEntrySettings.gPipeEnable = true;
    /* Since there is no other key, we can use the same polarity as the CPS */
    keyslotEntrySettings.outputPolarity.specify = true;
    keyslotEntrySettings.outputPolarity.gPipe = NEXUS_KeySlotPolarity_eEven;
    keyslotEntrySettings.outputPolarity.rPipe = NEXUS_KeySlotPolarity_eEven;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }


    BDBG_LOG( ( "Loads the same key to key entry #%d.\n", entry ) );

    rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Decrypt the data in dest to temp buffer */
    BDBG_LOG( ( "Decrypt data with key in key entry #%d.\n", entry ) );

    NEXUS_Memory_FlushCache (pDest, XPT_PACKET_SIZE);
    rc = securityUtil_DmaTransfer( keyslotHandle, pDest, pTemp, NEXUS_DmaDataFormat_eMpeg, XPT_PACKET_SIZE, false );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    /* Check the decrypt result is correct, even the packet head to be the same as the plain one. */
    if( BKNI_Memcmp( pTemp, pSrc, XPT_PACKET_SIZE - XPT_TS_PACKET_HEAD_SIZE ) ) {
        BDBG_ERR( ( "    Test FAILED!\n" ) );
        BDBG_ERR( ( "    Expected the following\n" ) );
        PRINT_TS_PACKET(pSrc);
        BDBG_ERR( ( "    Received the following\n" ) );
        PRINT_TS_PACKET(pTemp);
    } else {
        BDBG_LOG( ( "    Test PASSED!\n" ) );
    }

  exit:

    if( pSrc ) NEXUS_Memory_Free( pSrc );
    if( pDest ) NEXUS_Memory_Free( pDest );
    if( pTemp ) NEXUS_Memory_Free( pTemp );
    if( keyslotHandle ) NEXUS_KeySlot_Free( keyslotHandle );

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
