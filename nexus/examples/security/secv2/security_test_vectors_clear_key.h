/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef _SECURITY_TEST_KEYSLOT_DATA__
#define _SECURITY_TEST_KEYSLOT_DATA__

/*
   1. Clear key test data generated from openssl. These following test data is used by
   the nexus security example applications.

   The followings key, iv and plain texts will be used to generate the cipher texts in different
   cryptography algorithms and modes.

*/

#    define clearKey_128_0                  clearKey_128
#    define CipherText_Aes_128_Cbc_0        CipherText_Aes_128_Cbc

uint8_t       clearKey_64[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6
};

uint8_t       clearKey_128[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

uint8_t       clearKey_128_1[] = {
    0x3b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

uint8_t       clearKey_128_2[] = {
    0x4b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

uint8_t       clearKey_192[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

uint8_t       clearKey_256[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

uint8_t       IV_64[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

uint8_t       IV_128[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

uint8_t       IV_192[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

uint8_t       IV_256[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

uint8_t       plainText[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

/*
   1.1.1 AES_128_ECB

    How the test vector data is generated:

    a) set the plain text, key values as env varibles.

    env 'PLAINTEXT=6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710' /bin/bash

    env 'KEY128=2b7e151628aed2a6abf7158809cf4f3c' /bin/bash
    env 'KEY128_1=3b7e151628aed2a6abf7158809cf4f3c' /bin/bash
    env 'KEY128_2=4b7e151628aed2a6abf7158809cf4f3c' /bin/bash

    env 'KEY64=2b7e151628aed2a6' /bin/bash

    env 'KEY192=2b7e151628aed2a6abf7158809cf4f3c1122334455667788' /bin/bash

    env 'IV=000102030405060708090a0b0c0d0e0f' /bin/bash

    or

    export PLAINTEXT='6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710'

    env 'KEYA=efc8c844f0a9339d' /bin/bash
    env 'KEYB=b89f249ca1b8e36a' /bin/bash

    b) pass the plain text value to openssl and generate the cipher text.

    Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

    echo $PLAINTEXT | xxd -r -p | openssl enc -e -aes-128-ecb -K $KEY128 -out aes_ecb_cipher.txt; cat aes_ecb_cipher.txt | xxd -p -u -i -c 16;
    rm aes_ecb_cipher.txt

*/

/*
   Generate clearKey_ABA []
echo $KEYA$KEYB$KEYA | xxd -r -p  | xxd -p -u -i -c 16

*/

/* Keys alignment is [Key A, Key B] to be passed in 3DES_ABA. */
uint8_t       clearKey_ABA[] = {
    0XEF, 0XC8, 0XC8, 0X44, 0XF0, 0XA9, 0X33, 0X9D,
    0XB8, 0X9F, 0X24, 0X9C, 0XA1, 0XB8, 0XE3, 0X6A
};

uint8_t       CipherText_Aes_128_Ecb[] = {
    0X3A, 0XD7, 0X7B, 0XB4, 0X0D, 0X7A, 0X36, 0X60, 0XA8, 0X9E, 0XCA, 0XF3, 0X24, 0X66, 0XEF, 0X97,
    0XF5, 0XD3, 0XD5, 0X85, 0X03, 0XB9, 0X69, 0X9D, 0XE7, 0X85, 0X89, 0X5A, 0X96, 0XFD, 0XBA, 0XAF,
    0X43, 0XB1, 0XCD, 0X7F, 0X59, 0X8E, 0XCE, 0X23, 0X88, 0X1B, 0X00, 0XE3, 0XED, 0X03, 0X06, 0X88,
    0X7B, 0X0C, 0X78, 0X5E, 0X27, 0XE8, 0XAD, 0X3F, 0X82, 0X23, 0X20, 0X71, 0X04, 0X72, 0X5D, 0XD4
};

/*

   1.1.1.1 AES_128_ECB AESVS CAVS 11.1 GFSbox count=0 sample.

   Sample test of NIST AES ECB GFSbox128 count = 0
   COUNT = 0
   KEY = 00000000000000000000000000000000
   PLAINTEXT = f34481ec3cc627bacd5dc3fb08f273e6
   CIPHERTEXT = 0336763e966d92595a567cc9ce537f5e

   Use following encryption script to generate the cipher text and verify the cipher text:

   echo 'f34481ec3cc627bacd5dc3fb08f273e6' | xxd -r -p | openssl enc -e -aes-128-ecb -K 00000000000000000000000000000000 -out aes_ecb_128_GFSbox_cipher.txt; cat aes_ecb_128_GFSbox_cipher.txt | xxd -p -u -i -c 16

*/

/*
   1.1.2 AES_192_ECB

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-192-ecb -K $KEY192 -out aes_ecb_cipher.txt;
   cat aes_ecb_cipher.txt | xxd -p -u -i -c 16;
   rm aes_ecb_cipher.txt

*/

uint8_t       CipherText_Aes_192_Ecb[] = {
    0XC2, 0X48, 0X17, 0XF6, 0XAF, 0XC2, 0X6F, 0XBD, 0X15, 0X39, 0X5B, 0X38, 0XEB, 0X42, 0X87, 0X39,
    0X2F, 0XB4, 0X46, 0X46, 0X69, 0XFE, 0X9C, 0XB8, 0X97, 0XDA, 0X1F, 0X96, 0X55, 0X4C, 0X84, 0XEC,
    0XAD, 0X4D, 0XA9, 0X84, 0X39, 0XF8, 0XD2, 0X19, 0X70, 0X02, 0X5E, 0XF9, 0XB6, 0XBB, 0X12, 0X41,
    0X40, 0XC7, 0X22, 0X8A, 0X96, 0XEE, 0XF0, 0X71, 0X53, 0XD1, 0XC0, 0XBD, 0X1D, 0XB0, 0XA0, 0X89
};

/*
   1.1.2 AES_256_ECB

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-256-ecb -K 2b7e151628aed2a6abf7158809cf4f3c11223344556677881122334455667788 -out aes_ecb_cipher.txt;
   cat aes_ecb_cipher.txt | xxd -p -u -i -c 16

*/

uint8_t       CipherText_Aes_256_Ecb[] = {
    0X45, 0XA6, 0X60, 0XE4, 0X06, 0X8A, 0X34, 0XE0, 0XB9, 0X5A, 0X01, 0X41, 0XEC, 0XE8, 0XC1, 0XB3,
    0X5C, 0X6E, 0X34, 0XEA, 0X41, 0XA1, 0X62, 0X2C, 0XCC, 0X04, 0XF7, 0X51, 0XB6, 0X52, 0X7A, 0X16,
    0XC4, 0X34, 0X8F, 0X96, 0X80, 0X2D, 0X17, 0X5C, 0XF0, 0X43, 0X2E, 0X00, 0XF9, 0XE9, 0X87, 0X03,
    0XC2, 0X43, 0X59, 0X60, 0XD7, 0X3B, 0X0E, 0XCA, 0X1F, 0X20, 0X23, 0X80, 0X0B, 0X47, 0X25, 0XD8
};

/*
   1.2.1 AES_128_CBC

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-128-cbc -K $KEY128 -iv $IV -out aes_cbc_cipher.txt;
   cat aes_cbc_cipher.txt | xxd -p -u -i -c 16
   rm aes_cbc_cipher.txt
*/

uint8_t       CipherText_Aes_128_Cbc[] = {
    0X76, 0X49, 0XAB, 0XAC, 0X81, 0X19, 0XB2, 0X46, 0XCE, 0XE9, 0X8E, 0X9B, 0X12, 0XE9, 0X19, 0X7D,
    0X50, 0X86, 0XCB, 0X9B, 0X50, 0X72, 0X19, 0XEE, 0X95, 0XDB, 0X11, 0X3A, 0X91, 0X76, 0X78, 0XB2,
    0X73, 0XBE, 0XD6, 0XB8, 0XE3, 0XC1, 0X74, 0X3B, 0X71, 0X16, 0XE6, 0X9E, 0X22, 0X22, 0X95, 0X16,
    0X3F, 0XF1, 0XCA, 0XA1, 0X68, 0X1F, 0XAC, 0X09, 0X12, 0X0E, 0XCA, 0X30, 0X75, 0X86, 0XE1, 0XA7
};

/* env 'IV=0001020304050607' /bin/bash */
uint8_t       CipherText_Aes_128_Cbc_IV_64[] = {
    0XD8, 0X53, 0X82, 0XDE, 0X04, 0XA7, 0X6F, 0XE6, 0XE6, 0X06, 0XFA, 0X5E, 0XE0, 0XC7, 0X39, 0XF0,
    0X87, 0XE2, 0X4E, 0XC7, 0X67, 0X69, 0X70, 0X4C, 0X77, 0XD4, 0X81, 0XD7, 0X93, 0X2F, 0XC2, 0XC8,
    0X1D, 0X9B, 0X24, 0X0D, 0XF3, 0XAB, 0X3B, 0XBC, 0X5F, 0X8F, 0X7C, 0XF4, 0XD6, 0X88, 0X9D, 0XE8,
    0XD7, 0X57, 0XA7, 0XE8, 0X20, 0XC6, 0X0F, 0X7E, 0X9A, 0X6D, 0X21, 0X11, 0X45, 0XC0, 0X0B, 0XEB,
};

/*
   1.2.1-1 AES_128_CBC

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-128-cbc -K $KEY128_1 -iv $IV -out aes_cbc_cipher.txt;
   cat aes_cbc_cipher.txt | xxd -p -u -i -c 16
   rm aes_cbc_cipher.txt
*/

uint8_t       CipherText_Aes_128_Cbc_1[] = {
    0XA8, 0X62, 0X73, 0XCA, 0XF5, 0X5F, 0X4C, 0XB2, 0X29, 0XEC, 0X47, 0XF4, 0X3C, 0XA4, 0XFD, 0XF4,
    0X51, 0X9F, 0X9C, 0XFC, 0XCB, 0XD8, 0XE5, 0XEC, 0X55, 0X10, 0X77, 0X18, 0XBD, 0X27, 0X80, 0X59,
    0XCD, 0X59, 0XAA, 0XD9, 0X58, 0XCE, 0X20, 0X72, 0X24, 0X0E, 0X16, 0XE0, 0X48, 0X3F, 0X6F, 0XF8,
    0XFC, 0X9E, 0X86, 0X4F, 0XFD, 0X78, 0X83, 0X01, 0X8D, 0XC8, 0XAB, 0X0F, 0X07, 0X37, 0XAE, 0X1A
};

/*
   1.2.1-2 AES_128_CBC

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-128-cbc -K $KEY128_2 -iv $IV -out aes_cbc_cipher.txt;
   cat aes_cbc_cipher.txt | xxd -p -u -i -c 16
   rm aes_cbc_cipher.txt
*/

uint8_t       CipherText_Aes_128_Cbc_2[] = {
    0X64, 0XB6, 0XB0, 0XD8, 0X4A, 0XE7, 0X7B, 0X9F, 0X67, 0X61, 0X33, 0X4F, 0X3E, 0X75, 0X55, 0XC2,
    0X4E, 0XBE, 0X1A, 0X77, 0XA2, 0X7B, 0X89, 0XA6, 0X2B, 0XC5, 0X9D, 0X95, 0X03, 0X7D, 0X22, 0XDF,
    0XD2, 0XC0, 0XC6, 0XA0, 0XF4, 0XC9, 0X75, 0X9D, 0X9B, 0XB6, 0XBD, 0XB2, 0XED, 0X38, 0XA0, 0X90,
    0X1F, 0XD7, 0XD6, 0X72, 0XEA, 0X83, 0XCA, 0X05, 0X26, 0XD5, 0XB3, 0X9D, 0XEF, 0X18, 0XB9, 0X9E
};

/*

   1.2.1.1 AES_128_CBC AESVS CAVS 11.1 GFSbox count=0 sample.

   Sample test of NIST AES CBCGFSbox128 count = 0
   KEY = 00000000000000000000000000000000
   IV = 000000000000000000000000000000000
   PLAINTEXT = f34481ec3cc627bacd5dc3fb08f273e6
   CIPHERTEXT = 0336763e966d92595a567cc9ce537f5e

   Use following encryption script to generate the cipher text:
   echo 'f34481ec3cc627bacd5dc3fb08f273e6' | xxd -r -p | openssl enc -e -aes-128-cbc -K 00000000000000000000000000000000 -iv 00000000000000000000000000000000 -out aes_cbc_128_GFSbox_cipher.txt; cat aes_cbc_128_GFSbox_cipher.txt | xxd -p -u -i -c 16

   1.2.1.2 AES_128_CBC AESVS CAVS 11.1 GFSbox count=0 sample.

   Sample test of NIST AES CBCGFSbox128 count = 1
   KEY = 00000000000000000000000000000000
   IV = 00000000000000000000000000000000
   PLAINTEXT = 9798c4640bad75c7c3227db910174e72
   CIPHERTEXT = a9a1631bf4996954ebc093957b234589

   Use following encryption script to generate the cipher text:
   echo '9798c4640bad75c7c3227db910174e72' | xxd -r -p | openssl enc -e -aes-128-cbc -K 00000000000000000000000000000000 -iv 00000000000000000000000000000000 -out aes_cbc_128_GFSbox_cipher.txt; cat aes_cbc_128_GFSbox_cipher.txt | xxd -p -u -i -c 16

   1.2.1.3 AES_128_CBC AESVS CAVS 11.1 KeySbox count = 0 sample.

   Sample test of NIST AES CBCKeySbox128 count = 0

   KEY = 10a58869d74be5a374cf867cfb473859
   IV = 00000000000000000000000000000000
   PLAINTEXT = 00000000000000000000000000000000
   CIPHERTEXT = 6d251e6944b051e04eaa6fb4dbf78465

   Use following encryption script to generate the cipher text:
   echo '00000000000000000000000000000000' | xxd -r -p | openssl enc -e -aes-128-cbc -K 10a58869d74be5a374cf867cfb473859 -iv 00000000000000000000000000000000 -out aes_cbc_128_KeySbox_cipher.txt; cat aes_cbc_128_KeySbox_cipher.txt | xxd -p -u -i -c 16

   1.2.1.4 AES_128_CBC AESVS CAVS 11.1 KeySbox count = 1 sample.

   Sample test of NIST AES CBCKeySbox128 count = 1

   KEY = caea65cdbb75e9169ecd22ebe6e54675
   IV = 00000000000000000000000000000000
   PLAINTEXT = 00000000000000000000000000000000
   CIPHERTEXT = 6e29201190152df4ee058139def610bb

   Use following encryption script to generate the cipher text:
   echo '00000000000000000000000000000000' | xxd -r -p | openssl enc -e -aes-128-cbc -K caea65cdbb75e9169ecd22ebe6e54675 -iv 00000000000000000000000000000000 -out aes_cbc_128_KeySbox_cipher.txt; cat aes_cbc_128_KeySbox_cipher.txt | xxd -p -u -i -c 16

*/

/*
   1.2.2 AES_192_CBC

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-192-cbc -K $KEY192 -iv $IV -out aes_cbc_cipher.txt;
   cat aes_cbc_cipher.txt | xxd -p -u -i -c 16;
   rm aes_cbc_cipher.txt

*/

uint8_t       CipherText_Aes_192_Cbc[] = {
    0XA2, 0X09, 0X99, 0XBC, 0XE8, 0X90, 0XE2, 0X90, 0XCE, 0X80, 0X93, 0XC6, 0X48, 0X20, 0XFD, 0X01,
    0X24, 0X12, 0X72, 0X11, 0XBC, 0X2C, 0X04, 0X52, 0X58, 0X64, 0X9A, 0X26, 0XB5, 0X1E, 0X2D, 0XDF,
    0X40, 0XEC, 0X45, 0X53, 0X0C, 0XE5, 0X40, 0XD7, 0X16, 0X5F, 0X9B, 0X6A, 0X30, 0X4A, 0XD8, 0X09,
    0XCD, 0XCD, 0X76, 0XD0, 0X13, 0X5C, 0X91, 0X08, 0X3E, 0XF3, 0XA4, 0XD6, 0XD7, 0X99, 0XBA, 0X6A
};

/*
   1.3.1 AES_128_CTR

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-128-ctr -K $KEY128 -iv $IV -out aes_ctr_cipher.txt;
   cat aes_ctr_cipher.txt | xxd -p -u -i -c 16

   Example to verify with openssl decription:

   1) generate the head file format hex cipher data into a file.

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-128-ctr -K $KEY128 -iv $IV -out aes_ctr_cipher.txt;
   cat aes_ctr_cipher.txt | xxd -p -u -i -c 16 > aes-128-ctr-cipher-head-format.txt

   2) Check the encrypted text by decrypting the cipher text to check the plain text match above plainText[] values. This may not confirm the parameters are set correctly though. Another step would to verify with NIST published test vectors. Using openssl gives us more flexibilities.

   cat aes-128-ctr-cipher-head-format.txt | xxd -r -p |
   openssl enc -d -aes-128-ctr -K $KEY128 -iv $IV -out aes_ctr_plain.txt;
   cat aes_ctr_plain.txt | xxd -p -u -i -c 16;
   rm aes_ctr_plain.txt

*/

uint8_t       CipherText_Aes_128_Ctr[] = {
    0X3B, 0X3F, 0XD9, 0X2E, 0XB7, 0X2D, 0XAD, 0X20, 0X33, 0X34, 0X49, 0XF8, 0XE8, 0X3C, 0XFB, 0X4A,
    0X01, 0X0C, 0X04, 0X19, 0X99, 0XE0, 0X3F, 0X36, 0X44, 0X86, 0X24, 0X48, 0X3E, 0X58, 0X2D, 0X0E,
    0XA6, 0X22, 0X93, 0XCF, 0XA6, 0XDF, 0X74, 0X53, 0X5C, 0X35, 0X41, 0X81, 0X16, 0X87, 0X74, 0XDF,
    0X2D, 0X55, 0XA5, 0X47, 0X06, 0X27, 0X3C, 0X50, 0XD7, 0XB4, 0XF8, 0XA8, 0XCD, 0XDC, 0X6E, 0XD7
};

/*
   1.3.2 AES_192_CTR

   Generate the cipher text using the following script, and record the cipher text the same size as the plain text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -aes-192-ctr -K $KEY192 -iv $IV -out aes_ctr_cipher.txt;
   cat aes_ctr_cipher.txt | xxd -p -u -i -c 16;
   rm aes_ctr_cipher.txt

*/

uint8_t       CipherText_Aes_192_Ctr[] = {
    0XBB, 0X49, 0X3E, 0XB0, 0XEA, 0X07, 0X0F, 0XDE, 0X40, 0XD9, 0X04, 0XD0, 0X85, 0X40, 0X36, 0XF0,
    0X41, 0XAD, 0XDC, 0X23, 0X2B, 0XD9, 0X93, 0XD1, 0X5E, 0XCD, 0XC6, 0XE8, 0X70, 0X02, 0XD5, 0X90,
    0X6C, 0XF8, 0X4E, 0XC0, 0XB5, 0XCE, 0XDE, 0X9E, 0X00, 0X1A, 0X0A, 0X40, 0XFC, 0XE3, 0X1A, 0X95,
    0XF6, 0X06, 0XB0, 0X4E, 0X9B, 0X5A, 0XA7, 0X20, 0XA4, 0X20, 0X2D, 0X6A, 0XE0, 0X23, 0X63, 0X08
};

/*
   2 DES

   Generate the cipher text using the following script, and record the cipher text.
   Please note, the key is 8 bytes with 56 bits used.

   env 'KEY64=2b7e151628aed2a6' /bin/bash

   Encryption ) des_cipher.txt is the value of below CipherText_Des_64_Ecb[] for DES tests.

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -des-ecb -K $KEY64 -out des_ecb_cipher.txt;
   cat des_ecb_cipher.txt | xxd -p -u -i -c 16;
   rm des_ecb_cipher.txt

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -des-cbc -K $KEY64 -iv $IV -out des_cbc_cipher.txt;
   cat des_cbc_cipher.txt | xxd -p -u -i -c 16;
   rm des_cbc_cipher.txt

   Decryption ) des_plain.txt shall has the same values as the above plainText[].

   cat des_ecb_cipher.txt | xxd -p | xxd -p -r |
   openssl enc -d -des-ecb -K $KEY64 -iv $IV -out des_plain.txt;
   cat des_plain.txt | xxd -p -u -i -c 16;
   rm des_plain.txt
*/

uint8_t       CipherText_Des_64_Ecb[] = {
    0X6E, 0XDF, 0XD1, 0XB7, 0XA0, 0X01, 0XCD, 0X17, 0XCD, 0XC5, 0X7F, 0XF7, 0X9C, 0XF8, 0X72, 0XD0,
    0X11, 0X97, 0XA6, 0XD2, 0X13, 0X59, 0X4F, 0X7A, 0X3D, 0X7C, 0X7C, 0XEC, 0XBC, 0XDD, 0XD2, 0X20,
    0X3A, 0X75, 0X8B, 0X06, 0X75, 0X2E, 0X18, 0X0D, 0X55, 0X0F, 0XDD, 0X57, 0X5A, 0XF1, 0X3B, 0X94,
    0X18, 0X3D, 0X4D, 0XA1, 0X1E, 0X14, 0X75, 0X6B, 0X0F, 0XD9, 0XD9, 0X64, 0X16, 0XA0, 0X60, 0X14,
    0X3D, 0X22, 0XD1, 0X3C, 0X3D, 0X4D, 0X5F, 0X1E
};

uint8_t       CipherText_Des_64_Cbc[] = {
    0X39, 0X72, 0X1E, 0XD4, 0X24, 0X6D, 0X18, 0X8F, 0XF8, 0XBA, 0XA6, 0X1A, 0X8E, 0X38, 0X1C, 0X3B,
    0XF7, 0X31, 0X2B, 0X53, 0XC8, 0X54, 0XC0, 0XC6, 0XD9, 0X9B, 0XF1, 0X27, 0X3B, 0X5C, 0X92, 0XCC,
    0X72, 0X27, 0X3D, 0X95, 0X91, 0XC9, 0XC4, 0X72, 0X76, 0X98, 0X2A, 0XF9, 0X46, 0XCB, 0X79, 0XB1,
    0XC2, 0XA2, 0X2E, 0X1D, 0X7F, 0X28, 0X58, 0XC2, 0XD7, 0XB8, 0X7D, 0XDB, 0X94, 0X9F, 0X15, 0X3C,
    0X88, 0X76, 0XC3, 0X00, 0X3B, 0XF5, 0XA9, 0X8E
};

/*
   3. Triple_DES_128_CBC

   Generate the cipher text using the following script, and record the cipher text:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -des-ede3-cbc -K $KEY128 -iv $IV -out 3des_cbc_cipher.txt;
   cat 3des_cbc_cipher.txt | xxd -p -u -i -c 16;
   rm 3des_cbc_cipher.txt

*/

uint8_t       CipherText_3des[] = {
    0X86, 0X45, 0XA6, 0X3F, 0X6A, 0X40, 0X3B, 0X61, 0XE9, 0X77, 0XA8, 0X3B, 0X4F, 0XDE, 0X0C, 0X62,
    0X1D, 0X01, 0XD6, 0X65, 0X51, 0X83, 0X5E, 0XF6, 0XF3, 0X08, 0X18, 0X41, 0X74, 0XFF, 0X89, 0XF3,
    0X77, 0X93, 0X8E, 0XB7, 0X1A, 0XBB, 0X68, 0X8C, 0X82, 0X81, 0X46, 0XCB, 0XFB, 0XC2, 0XE9, 0X5F,
    0X82, 0XC0, 0X73, 0X23, 0X91, 0X58, 0XAF, 0XED, 0X4D, 0X7A, 0X8C, 0X94, 0X64, 0XF9, 0XBD, 0X36,
    0XE8, 0X48, 0XBD, 0X52, 0X3F, 0XCC, 0X28, 0X13
};

/*
   3.2 Triple_DES_128_ECB

   Followings three scripts give the same cipher text results:
   3DES two keys ECB with key AB, the key's length is 56 x 56, as 7 bits each 8 bits is used.

   env 'KEYA=efc8c844f0a9339d' /bin/bash
   env 'KEYB=b89f249ca1b8e36a' /bin/bash

   echo '65a715c1f1a360c8' | xxd -r -p |
   openssl enc -e -des-ede -K $KEYB$KEYA -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

   1) Using Key A (key 1) = efc8c844f0a9339d,  Key B (key 2) = b89f249ca1b8e36a, and K = [Key B: Key A]
   echo '65a715c1f1a360c8' | xxd -r -p |
   openssl enc -e -des-ede -K b89f249ca1b8e36aefc8c844f0a9339d  -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

   The cipher text is [ 0X27, 0X23, 0X18, 0XE9, 0XA2, 0X04, 0XAC, 0X17, 0X59, 0X4D, 0X14, 0X1B, 0X56, 0X74, 0X68, 0X91 ]

   2) Following is to try 3DES encryption with DES(Key A) (Enc) -> DES(Key B) (Dec) -> DES(Key A) (Enc)

   echo '65a715c1f1a360c8' | xxd -r -p |
   openssl enc -e -des-ecb -K efc8c844f0a9339d -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -r -p |
   openssl enc -d -des-ecb -K b89f249ca1b8e36a -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -r -p |
   openssl enc -e -des-ecb -K efc8c844f0a9339d -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

   The cipher text is [ 0X27, 0X23, 0X18, 0XE9, 0XA2, 0X04, 0XAC, 0X17, 0X59, 0X4D, 0X14, 0X1B, 0X56, 0X74, 0X68, 0X91 ]

   3) 3DES three keys ECB with key ABA format, two keys are used. The key length is acutually 56 x 56, the same as the previous one.
   3.1) 3DES BAB
   echo '65a715c1f1a360c8' | xxd -r -p |
   openssl enc -e -des-ede3 -K $KEYB$KEYA$KEYB -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

   echo $KEYA$KEYB$KEYA | xxd -r -p  | xxd -p -u -i -c 16

   3.2) 3DES ABA

   Encryption ) The result from the following settings matches the values from BRCM doc about 3DES-ABA.

   echo '65a715c1f1a360c8' | xxd -r -p |
   openssl enc -e -des-ede3 -K $KEYA$KEYB$KEYA -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

   The cipher text is
   [0X28, 0XEB, 0X90, 0X2E, 0XB2, 0X36, 0X18, 0X88, 0X2F, 0X3D, 0X78, 0X82, 0X6A, 0X9A, 0XDF, 0XCE]

   Decryption )

   cat 3des_ecb_cipher.txt | xxd -p | xxd -p -r |
   openssl enc -d -des-ede3 -K $KEYA$KEYB$KEYA -out 3des_ecb_plain.txt;
   cat 3des_ecb_plain.txt | xxd -p -u -i -c 16

   The plain text is [0X65, 0XA7, 0X15, 0XC1, 0XF1, 0XA3, 0X60, 0XC8]

   Generate the test data for the test framework using the above settings:

   echo $PLAINTEXT | xxd -r -p |
   openssl enc -e -des-ede3 -K $KEYA$KEYB$KEYA -out 3des_ecb_cipher.txt;
   cat 3des_ecb_cipher.txt | xxd -p -u -i -c 16

*/

uint8_t       CipherText_TDes_112[] = {
    0X92, 0XF8, 0XC0, 0X24, 0X9E, 0X3B, 0X3C, 0XEE, 0X8B, 0X3C, 0XEF, 0X9F, 0X59, 0X73, 0XDB, 0X92,
    0X50, 0X70, 0X08, 0X41, 0X08, 0X8F, 0X4E, 0X60, 0XB3, 0XCE, 0XE7, 0X10, 0X27, 0X7D, 0X65, 0XD8,
    0X97, 0X7E, 0XCD, 0X3C, 0XEC, 0X5C, 0X54, 0XA3, 0X1B, 0XDC, 0X43, 0X7E, 0XC8, 0X64, 0X5C, 0XCD,
    0XC3, 0X2E, 0X37, 0X2C, 0X65, 0XEE, 0XF1, 0X39, 0XED, 0XA6, 0X4E, 0X6A, 0X39, 0X15, 0XD0, 0XDF,
    0X2F, 0X3D, 0X78, 0X82, 0X6A, 0X9A, 0XDF, 0XCE
};

#endif /* #ifndef _SECURITY_TEST_KEYSLOT_DATA__ */
