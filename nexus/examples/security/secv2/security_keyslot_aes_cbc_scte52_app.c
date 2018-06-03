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

/* The followings are the test vectors for odd lengh data crypto transfer. */

static uint8_t const IV_16[] = {
    0X5E, 0X7B, 0X2D, 0XBD, 0X9E, 0X42, 0X46, 0X3D, 0XCB, 0X15, 0XE2, 0X97, 0X78, 0X9B, 0X55, 0XF2
};

static uint8_t const KEY_16[] = {
    0XB4, 0X73, 0X2C, 0X69, 0X53, 0X0F, 0X6B, 0XA7, 0XBB, 0X1A, 0X2C, 0X77, 0XB9, 0XDA, 0XCC, 0X74
};

static uint8_t const cipherText_182[] = {
    0X5C, 0X86, 0XDD, 0X81, 0XDE, 0XB0, 0X29, 0X92, 0XD4, 0XC3, 0XF8, 0X4B, 0XAE, 0XC9, 0X43, 0X51,
    0XC7, 0X2F, 0X00, 0X25, 0X1F, 0XB7, 0X56, 0X9D, 0X28, 0X8D, 0X79, 0X16, 0X34, 0XF1, 0X9A, 0XA6,
    0X6F, 0X81, 0XE0, 0XC2, 0X31, 0X77, 0X90, 0X91, 0X6B, 0X33, 0XF1, 0X38, 0X10, 0XAE, 0X37, 0X53,
    0XED, 0X1C, 0XE8, 0XC4, 0XA4, 0X40, 0X68, 0X22, 0XD1, 0X2A, 0X65, 0XE1, 0XA1, 0X50, 0X87, 0X9D,
    0X3F, 0XCC, 0XAE, 0X9A, 0XE7, 0XAB, 0X12, 0X26, 0X10, 0XA5, 0X8F, 0XE8, 0XCA, 0XD8, 0XB2, 0X1C,
    0XC5, 0XE1, 0X09, 0XE0, 0X29, 0X2B, 0XE4, 0XE6, 0X78, 0X7A, 0X96, 0XD0, 0X94, 0X39, 0X5A, 0X09,
    0XAC, 0XE4, 0XB1, 0X83, 0XE6, 0X4E, 0XA3, 0XAE, 0XF5, 0XEB, 0XE0, 0X58, 0XF1, 0XCC, 0XBE, 0X54,
    0XCD, 0XBB, 0X7D, 0XEE, 0X33, 0X96, 0XAD, 0XA2, 0X10, 0X5B, 0XF2, 0X82, 0XBE, 0XDD, 0X26, 0X7A,
    0X02, 0X24, 0X2B, 0X3F, 0XB1, 0X02, 0XF2, 0XDA, 0XD3, 0XDE, 0X6A, 0X2D, 0XF8, 0X61, 0XA9, 0X20,
    0XF0, 0XA4, 0XC6, 0X75, 0X3C, 0X31, 0XC2, 0XBE, 0XB6, 0XB4, 0X05, 0X25, 0X22, 0XD1, 0X41, 0X75,
    0X41, 0XC9, 0X93, 0XAE, 0X50, 0X2F, 0X6B, 0XA6, 0X6E, 0XA3, 0XC3, 0XC3, 0XDF, 0X24, 0X3E, 0X50,
    0X54, 0XDD, 0XA4, 0X5F, 0X1A, 0XE4
};

static uint8_t const plainText_182[] = {
    0X97, 0X1D, 0X39, 0XED, 0X4E, 0X46, 0X84, 0X6D, 0X08, 0X3A, 0X24, 0X85, 0XD6, 0X2C, 0X99, 0XC1,
    0X99, 0X60, 0X7C, 0XF1, 0X2E, 0X37, 0X6A, 0X0C, 0X36, 0XE4, 0X93, 0X3C, 0X18, 0X41, 0X1E, 0X73,
    0X02, 0XA7, 0XFD, 0XAB, 0X0A, 0X89, 0XD6, 0X9F, 0XFB, 0X65, 0X6E, 0XA6, 0X0C, 0XF5, 0XB2, 0XA6,
    0XFD, 0X8D, 0X12, 0XAB, 0XA4, 0X61, 0XC9, 0X55, 0X4A, 0X6B, 0X8F, 0X6C, 0X6A, 0XF1, 0XE3, 0X59,
    0X33, 0X13, 0XA5, 0X85, 0XB6, 0X7A, 0X42, 0XDB, 0XF6, 0X15, 0XC2, 0XCB, 0X02, 0X96, 0XCE, 0XC4,
    0X12, 0X3B, 0X7B, 0XA0, 0X2A, 0X79, 0XD6, 0XCA, 0X3D, 0X6B, 0XD6, 0X2F, 0X6F, 0XA9, 0XF9, 0X82,
    0X8F, 0XF3, 0X7E, 0X65, 0X1C, 0XC3, 0X1E, 0X3E, 0XBC, 0XB3, 0X70, 0X69, 0X34, 0XEF, 0X11, 0XC3,
    0XA4, 0X60, 0X0A, 0X36, 0XD7, 0X18, 0X45, 0XC1, 0X11, 0XE9, 0XBA, 0X87, 0X99, 0XB4, 0X3A, 0X27,
    0XFC, 0XB8, 0XE7, 0X8A, 0X47, 0X4B, 0X90, 0X29, 0XB6, 0X91, 0XC6, 0XF8, 0X50, 0XCC, 0X6B, 0XB5,
    0X66, 0X61, 0XF1, 0X60, 0X78, 0X9C, 0XE6, 0X67, 0XC9, 0X75, 0X2C, 0X19, 0XA0, 0X07, 0XED, 0X0B,
    0X16, 0XBA, 0X93, 0X15, 0XBD, 0X8F, 0X05, 0X60, 0XF2, 0X11, 0X79, 0X90, 0X8C, 0X87, 0X98, 0XFF,
    0X74, 0X8C, 0X7F, 0XCB, 0X9B, 0X72
};

static uint8_t const cipherText_18[] = {
    0X51, 0XA5, 0X5B, 0XD1, 0X9D, 0X71, 0XF1, 0X70, 0X55, 0X73, 0XE0, 0X2B, 0X65, 0X85, 0XC7, 0X6D,
    0X93, 0XB9
};

static uint8_t const plainText_18[] = {
    0X48, 0XF9, 0X9B, 0X14, 0XC7, 0XC8, 0X94, 0X27, 0X61, 0X13, 0X54, 0XFC, 0X08, 0XBC, 0X57, 0X9E,
    0X3A, 0X99
};

static uint8_t const cipherText_14[] = {
    0XE1, 0XB5, 0X83, 0X0E, 0XBF, 0X93, 0XE7, 0X50, 0XC8, 0XB3, 0X0F, 0X8B, 0XCF, 0X37
};

static uint8_t const plainText_14[] = {
    0XF3, 0X33, 0XAD, 0XDC, 0XD1, 0XFC, 0XA1, 0XE9, 0X68, 0XF3, 0X45, 0X00, 0X09, 0X66
};

/* The followings are the test vectors for odd lengh data crypto transfer. */

static uint8_t const KEY_16_ODD[] = {
    0X4A, 0XBF, 0XF1, 0X96, 0X40, 0XDC, 0X2F, 0X82, 0X7D, 0X62, 0XD5, 0XFB, 0X4C, 0X9A, 0X51, 0X9A
};

static uint8_t const IV_16_ODD[] = {
    0XC6, 0XBB, 0XD2, 0X8E, 0X34, 0X72, 0XF3, 0X69, 0XEE, 0XD2, 0X4A, 0XDE, 0X41, 0XF4, 0XB3, 0X84
};

static uint8_t const cipherText_181[] = {
    0XC2, 0XD0, 0X48, 0XAE, 0XC9, 0XCC, 0X35, 0XA2, 0XC3, 0X11, 0X45, 0XE5, 0X93, 0X22, 0XD8, 0XF0,
    0XD9, 0X4C, 0X1C, 0X3F, 0X3A, 0X00, 0XE8, 0XAB, 0X54, 0X84, 0X54, 0XA2, 0XA3, 0XFC, 0X48, 0X56,
    0X96, 0X43, 0X54, 0X2E, 0XF6, 0X67, 0XD8, 0X58, 0X5D, 0X56, 0X53, 0X49, 0X13, 0X81, 0X91, 0X87,
    0XBF, 0X25, 0XD7, 0X74, 0XDC, 0XF5, 0X47, 0X97, 0X5E, 0X1A, 0XC0, 0XA1, 0X61, 0X39, 0XB8, 0X3D,
    0X8E, 0XCD, 0X42, 0X13, 0X5A, 0X9B, 0XE5, 0X37, 0X5F, 0XDE, 0X37, 0X0A, 0X78, 0X1C, 0XC0, 0X99,
    0XD2, 0XB6, 0XB4, 0XB2, 0X73, 0X52, 0XBE, 0XFC, 0XA4, 0XA1, 0XD5, 0X2B, 0XE5, 0X00, 0XD4, 0X71,
    0X71, 0XED, 0X0E, 0X98, 0X59, 0X22, 0XE3, 0X62, 0X31, 0X06, 0X97, 0X82, 0X17, 0X84, 0XAB, 0XBB,
    0X32, 0XC3, 0XA9, 0X10, 0X98, 0X3A, 0XBE, 0XFF, 0X26, 0X4C, 0XA8, 0XD4, 0XB2, 0X85, 0X14, 0X29,
    0XE5, 0X53, 0X7B, 0X2F, 0XD7, 0XFE, 0X25, 0X88, 0XED, 0X90, 0X9F, 0X8C, 0X02, 0X11, 0XC5, 0XDE,
    0XD8, 0XB7, 0XB8, 0XFD, 0X2E, 0X23, 0X16, 0X81, 0X38, 0X4C, 0XB9, 0XF0, 0X72, 0XE8, 0X50, 0X4B,
    0XA4, 0X14, 0XDC, 0X06, 0X0F, 0XCC, 0X31, 0X86, 0XDB, 0X19, 0X02, 0X47, 0X22, 0X74, 0X57, 0X31,
    0X4E, 0X5C, 0X37, 0X47, 0XBE
};

static uint8_t const plainText_181[] = {
    0X1F, 0X11, 0X41, 0X98, 0X63, 0XEC, 0XDF, 0XDC, 0X15, 0X15, 0X3D, 0X60, 0X2F, 0X95, 0X90, 0X54,
    0X5A, 0X76, 0X45, 0X1F, 0XB8, 0XCB, 0XDB, 0X28, 0XAB, 0X47, 0XE3, 0X0B, 0X1F, 0XAD, 0XE3, 0XE9,
    0XB7, 0XBD, 0X2B, 0X11, 0X65, 0XA2, 0X39, 0XF3, 0XFA, 0X2E, 0X2C, 0X9A, 0X63, 0X1D, 0X23, 0XC9,
    0XEE, 0XC0, 0XF4, 0XD1, 0X14, 0X62, 0XAC, 0XCC, 0XD8, 0X97, 0XEA, 0X1B, 0X87, 0X0F, 0XCE, 0X51,
    0X51, 0X62, 0X5C, 0X0B, 0X5F, 0X0B, 0X8C, 0X11, 0X20, 0X00, 0X23, 0X53, 0X6B, 0XC9, 0X37, 0X2A,
    0X75, 0X67, 0XEC, 0X1C, 0X0E, 0XA0, 0XFE, 0X1A, 0XE6, 0XFE, 0X57, 0X96, 0X2D, 0X90, 0XF0, 0XF9,
    0X0A, 0X54, 0X9E, 0X7E, 0X0F, 0XE8, 0X56, 0X97, 0XD9, 0X01, 0X31, 0X79, 0X71, 0X7A, 0XBA, 0XFC,
    0XC6, 0X48, 0X1E, 0XC7, 0XA0, 0XED, 0X8D, 0X27, 0X5F, 0X19, 0X35, 0X7B, 0XDE, 0X9A, 0X9E, 0X49,
    0XB5, 0XD2, 0X63, 0XF3, 0XFA, 0X71, 0X03, 0X67, 0XA2, 0XAF, 0XBA, 0X06, 0XB0, 0X45, 0XC4, 0X3E,
    0XF3, 0X1A, 0X1D, 0X36, 0X31, 0X8E, 0X36, 0X11, 0XCB, 0X4D, 0X9F, 0X89, 0X70, 0X88, 0X3D, 0XAE,
    0XA8, 0XE6, 0X6C, 0XFC, 0XBA, 0X55, 0X6C, 0XD8, 0XEF, 0X09, 0X06, 0X41, 0X20, 0X7A, 0XAF, 0X77,
    0X9B, 0XAD, 0X54, 0X1B, 0X6E
};

static uint8_t const cipherText_17[] = {
    0X44, 0XED, 0X0F, 0XB6, 0X9F, 0X97, 0X43, 0X08, 0X4A, 0XA5, 0X4B, 0X75, 0XF9, 0X87, 0XD9, 0X5F,
    0X45
};

static uint8_t const plainText_17[] = {
    0XC1, 0X5B, 0XA8, 0X70, 0X9B, 0X3B, 0XDB, 0XE9, 0X6C, 0X1F, 0X79, 0X3D, 0X22, 0X40, 0X5A, 0X1A,
    0XCB
};

static uint8_t const cipherText_15[] = {
    0XE7, 0X8B, 0XC9, 0X9C, 0X9B, 0X25, 0X22, 0XB0, 0X37, 0X91, 0X9E, 0X44, 0XE8, 0X2F, 0XFF
};

static uint8_t const plainText_15[] = {
    0X5C, 0X30, 0X36, 0XE8, 0XC8, 0XB7, 0XDD, 0X7D, 0X3B, 0XD6, 0X14, 0X4A, 0X16, 0X30, 0XC5
};

/* The main example to allocate and configure keyslot */
static int    security_keyslot_aes_cbc_scte52(
    const uint8_t * plainTxt,
    const uint8_t * cipherTxt,
    const unsigned data_size,
    const uint8_t * key,
    const unsigned key_size,
    const uint8_t * iv,
    const unsigned iv_size );

int main(
    int argc,
    char **argv )
{
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* Even length data DMA crypto transfer. */
    rc = security_keyslot_aes_cbc_scte52( plainText_182, cipherText_182, sizeof( plainText_182 ),
                                          KEY_16, sizeof( KEY_16 ), IV_16, sizeof( IV_16 ) );
    SECURITY_CHECK_RC( rc );

    rc = security_keyslot_aes_cbc_scte52( plainText_18, cipherText_18, sizeof( plainText_18 ),
                                          KEY_16, sizeof( KEY_16 ), IV_16, sizeof( IV_16 ) );
    SECURITY_CHECK_RC( rc );

    rc = security_keyslot_aes_cbc_scte52( plainText_14, cipherText_14, sizeof( plainText_14 ),
                                          KEY_16, sizeof( KEY_16 ), IV_16, sizeof( IV_16 ) );
    SECURITY_CHECK_RC( rc );

    /* Odd length data DMA crypto transfer. */
    rc = security_keyslot_aes_cbc_scte52( plainText_181, cipherText_181, sizeof( plainText_181 ),
                                          KEY_16_ODD, sizeof( KEY_16_ODD ), IV_16_ODD, sizeof( IV_16_ODD ) );
    SECURITY_CHECK_RC( rc );

    rc = security_keyslot_aes_cbc_scte52( plainText_17, cipherText_17, sizeof( plainText_17 ),
                                          KEY_16_ODD, sizeof( KEY_16_ODD ), IV_16_ODD, sizeof( IV_16_ODD ) );
    SECURITY_CHECK_RC( rc );

    rc = security_keyslot_aes_cbc_scte52( plainText_15, cipherText_15, sizeof( plainText_15 ),
                                          KEY_16_ODD, sizeof( KEY_16_ODD ), IV_16_ODD, sizeof( IV_16_ODD ) );
    SECURITY_CHECK_RC( rc );

  exit:
    securityUtil_PlatformUnInit(  );
}

/* The main example to allocate and configure keyslot */
static int security_keyslot_aes_cbc_scte52(
    const uint8_t * plainTxt,
    const uint8_t * cipherTxt,
    const unsigned data_size,
    const uint8_t * key,
    const unsigned key_size,
    const uint8_t * iv,
    const unsigned iv_size )
{

    NEXUS_KeySlotHandle keyslotHandle = NULL;

    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotIv slotIv;
    NEXUS_KeySlotKey slotKey;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eMax;
    uint8_t      *pSrc = NULL, *pDest = NULL;
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    /* Allocate a key slot. */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerEntry;
    keyslotAllocSettings.useWithDma = true;

    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ) {
        BDBG_ERR( ( "\nError: Can't allocate keyslot\n" ) );
        return NEXUS_NOT_AVAILABLE;
    }

    /* Configure the keyslot. */
    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    SECURITY_CHECK_RC( rc );

    /* Configure a key entry to encrypt. */
    entry = NEXUS_KeySlotBlockEntry_eCpsClear;
    NEXUS_KeySlot_GetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eCbc;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eScte52;
    keyslotEntrySettings.solitaryMode = NEXUS_KeySlotTerminationSolitaryMode_eIv1;
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;

    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle, entry, &keyslotEntrySettings );
    SECURITY_CHECK_RC( rc );

    BDBG_LOG( ( "Loads the clear key to key entry #%d.\n", entry ) );

    /* The actual size for the algorithm selected. */
    slotKey.size = key_size;
    BKNI_Memcpy( slotKey.key, key, key_size );

    /* Using clear key as the root key to encrypt the plain text, cipher is pDest. */
    rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle, entry, &slotKey );
    SECURITY_CHECK_RC( rc );

    /* IV is 128 bits for AES, 64 bits for DES. */
    slotIv.size = iv_size;
    BKNI_Memcpy( slotIv.iv, iv, iv_size );

    rc = NEXUS_KeySlot_SetEntryIv( keyslotHandle, entry, &slotIv, NULL );
    SECURITY_CHECK_RC( rc );

    /* DMA transfer to encrypt plainTxt to cipherTxt. */
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pSrc );
    NEXUS_Memory_Allocate( data_size, NULL, ( void ** ) &pDest );

    BKNI_Memcpy( pSrc, plainTxt, data_size );
    BKNI_Memset( pDest, 0, data_size );

    rc = securityUtil_DmaTransfer( keyslotHandle, pSrc, pDest, NEXUS_DmaDataFormat_eBlock, data_size, false );
    SECURITY_CHECK_RC( rc );

    if( BKNI_Memcmp( pDest, cipherTxt, data_size ) ) {
        BDBG_ERR( ( "    Test FAILED with data size %d DMA!\n", data_size ) );
        rc = NEXUS_SUCCESS; /* report the error, while let the app progress further with the remaining cases. */
    }
    else {
        BDBG_LOG( ( "    Test PASSED with data size %d DMA!\n", data_size ) );
    }

  exit:

    if( pSrc ) NEXUS_Memory_Free( pSrc );
    if( pDest ) NEXUS_Memory_Free( pDest );
    if( keyslotHandle ) NEXUS_KeySlot_Free( keyslotHandle );

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
