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

#if (NEXUS_SECURITY_API_VERSION==2)

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_security.h"

#include "common_crypto.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

BDBG_MODULE(common_crypto);
#define VERBOSE 0
#ifdef VERBOSE
#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr)                                         \
                            do                                                                    \
                            {                                                                     \
                                int _x_offset_;                                                   \
                                printf("[%s][%d] ", description_txt, in_size);                                 \
                                for( _x_offset_ = 0; _x_offset_ < in_size; _x_offset_++ )         \
                                {                                                                 \
                                    if( (_x_offset_ != 0) && (_x_offset_%16 == 0) ) printf("\n"); \
                                                                                                  \
                                    printf ("%02X ", in_ptr[_x_offset_] );                        \
                                }                                                                 \
                                printf("\n");                                                     \
                            } while(0)
#else
#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) do {} while(0)
#endif





CommonCryptoKeyLadderSettings  custKeySettings = { 0x00,/* cust key*/
                                                       0xFF, /* key var high */
                                                       0x00, /* key var low */
                                                       {0xbf, 0x98, 0x38, 0xe1, 0x74, 0x78, 0xe8, 0x5e, 0xf4, 0x96, 0x25, 0xce, 0xb8, 0x63, 0xce, 0x76}, /* proc_in for key 3 */
                                                       16,
                                                       {0x3f, 0xd5, 0xb3, 0x3e, 0x60, 0x10, 0x86, 0xb4, 0x08, 0xc8, 0xae, 0x1d, 0x49, 0xc1, 0x73, 0x8c}, /* proc_in for key 4 */
                                                       16,
                                                       false,
                                                       {NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicOperation_eDecrypt},
                                                       false,
                                                       NEXUS_CryptographicAlgorithm_e3DesAba,
                                                       NEXUS_ANY_ID,
                                                       NEXUS_KeyLadderMode_eCp_128_4,
                                                       NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eOne,
                                                       0,
                                                       false,
                                                       false
                                                     };

/* ****************************************************************************
 * AES tests vectors
 ******************************************************************************/

CommonCryptoKeyIvSettings aes_ecb_key = {
                                                {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
                                                16,
                                                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                0
                                         };
/* Decrypted buffer */
uint8_t aes_plaintext[] =   {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                             0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
/* AES ECB */
/* Buffer encrypted with user key */
uint8_t aes_ecb_usr_cipher_text[] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                                     0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
uint8_t aes_ecb_cust_cipher_text[16]; /* Buffer encrypted with cust key */
uint8_t aes_ecb_otp_cipher_text[16];  /* Buffer encrypted with otp key */

/* AES CBC */
CommonCryptoKeyIvSettings aes_cbc_key = {
                                                {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
                                                16,
                                                {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
                                                16
                                            };

/* aes_plaintext Buffer encrypted with user key */
uint8_t aes_cbc_usr_cipher_text[] = {0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
                                     0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d};
uint8_t aes_cbc_cust_cipher_text[16]; /* Buffer encrypted with cust key */
uint8_t aes_cbc_otp_cipher_text[16]; /* Buffer encrypted with otp key */

/* AES CTR */
CommonCryptoKeyIvSettings aes_ctr_key = {
                                                {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
                                                16,
                                                {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff},
                                                16
                                            };

/* aes_plaintext Buffer encrypted with user key */
uint8_t aes_ctr_usr_cipher_text[] = {0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26,
                                       0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce};

uint8_t aes_ctr_cust_cipher_text[16]; /* Buffer encrypted with cust key */
uint8_t aes_ctr_otp_cipher_text[16];  /* Buffer encrypted with otp key */

typedef struct test_case_clear_key{
    CommonCryptoKeyIvSettings *pKeyIvSettings;
    bool use_CommonCryptoSetupKey;				/* true = uses CommonCrypto_SetupKey() to load and config the key
                                                    false = uses CommonCrypto_LoadClearKeyIv() and CommonCrypto_LoadKeyConfig() */
    NEXUS_CryptographicOperation opType;         /* Operation to perfrom */
    NEXUS_CryptographicAlgorithm algType;        /* Crypto algorithm */
    NEXUS_CryptographicAlgorithmMode algVariant; /* Cipher chain mode for selected cipher */
    NEXUS_KeySlotPolarity keySlotType;           /* Key destination entry type */
    NEXUS_KeySlotTerminationMode termMode;       /* Termination Type for residual block to be ciphered */


    bool enableExtKey;
    bool enableExtIv;
    unsigned aesCounterSize;                     /* For algorithm modes predicated on a counter, this parameter spcifies
                                                    the size of the counter in bits. Supported values are 32, 64, 96 and 128 bits.*/
    NEXUS_CounterMode aesCounterMode;


    uint8_t *pClear;
    uint8_t *pCiphered;
}test_case_clear_key;

test_case_clear_key tests_clear_key[] = {

    {&aes_ecb_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0,aes_plaintext, aes_ecb_usr_cipher_text},
    {&aes_ecb_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},

#if  !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_ecb_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},
    {&aes_ecb_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},
#endif

    {&aes_cbc_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
    {&aes_cbc_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},

#if !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_cbc_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
    {&aes_cbc_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
#endif

    {&aes_ctr_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
    {&aes_ctr_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},

#if !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_ctr_key, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
    {&aes_ctr_key, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
#endif

    {&aes_ecb_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},
    {&aes_ecb_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},

#if !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_ecb_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},
    {&aes_ecb_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_usr_cipher_text},
#endif

    {&aes_cbc_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
    {&aes_cbc_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},

#if !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_cbc_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
    {&aes_cbc_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_usr_cipher_text},
#endif

    {&aes_ctr_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
    {&aes_ctr_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},

#if !defined DISABLE_EXTERNAL_KEY_IV_TESTS
    {&aes_ctr_key, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
    {&aes_ctr_key, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, true, true, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_usr_cipher_text},
#endif



};


typedef struct test_case_key_ladder{
    CommonCryptoKeyIvSettings *pIvSettings;
    CommonCryptoKeySrc   keySrc;
    CommonCryptoKeyLadderSettings *pKeyLadderSettings;
    bool use_CommonCryptoSetupKey;               /* true = uses CommonCrypto_SetupKey() to load and config the key
                                                    false = uses CommonCrypto_LoadClearKeyIv() and CommonCrypto_LoadKeyConfig() */
    NEXUS_CryptographicOperation opType;         /* Operation to perfrom */
    NEXUS_CryptographicAlgorithm algType;        /* Crypto algorithm */
    NEXUS_CryptographicAlgorithmMode algVariant; /* Cipher chain mode for selected cipher */
    NEXUS_KeySlotPolarity keySlotType;           /* Key destination entry type */
    NEXUS_KeySlotTerminationMode termMode;       /* Termination Type for residual block to be ciphered */
    bool enableExtKey;
    bool enableExtIv;
    unsigned aesCounterSize;                     /* For algorithm modes predicated on a counter, this parameter spcifies
                                                    the size of the counter in bits. Supported values are 32, 64, 96 and 128 bits.*/
    NEXUS_CounterMode aesCounterMode;

    uint8_t *pClear;
    uint8_t *pCiphered;

}test_case_key_ladder;


test_case_key_ladder tests_key_ladder[] = {

    {NULL,         CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_cust_cipher_text},
    {NULL,         CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_cust_cipher_text},
    {&aes_cbc_key, CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e3, aes_plaintext, aes_cbc_cust_cipher_text},
    {&aes_cbc_key, CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e3, aes_plaintext, aes_cbc_cust_cipher_text},

    {&aes_ctr_key, CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_cust_cipher_text},
    {&aes_ctr_key, CommonCrypto_eGlobalKey, &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_cust_cipher_text},

    {NULL,         CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_otp_cipher_text},


    {NULL,         CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_otp_cipher_text},

    {&aes_cbc_key, CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_otp_cipher_text},
    {&aes_cbc_key, CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_otp_cipher_text},

    {&aes_ctr_key, CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_otp_cipher_text},
    {&aes_ctr_key, CommonCrypto_eOtpAskm,  &custKeySettings, false, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_otp_cipher_text},

    {NULL,         CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_cust_cipher_text},
    {NULL,         CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_ecb_cust_cipher_text},

    {&aes_cbc_key, CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_cust_cipher_text},
    {&aes_cbc_key, CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e0, aes_plaintext, aes_cbc_cust_cipher_text},


    {&aes_ctr_key, CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_cust_cipher_text},
    {&aes_ctr_key, CommonCrypto_eGlobalKey, &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_cust_cipher_text},

    {NULL,         CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_otp_cipher_text},
    {NULL,         CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_ecb_otp_cipher_text},

    {&aes_cbc_key, CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_cbc_otp_cipher_text},
    {&aes_cbc_key, CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 32, NEXUS_CounterMode_e3, aes_plaintext, aes_cbc_otp_cipher_text},

    {&aes_ctr_key, CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eDecrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_otp_cipher_text},
    {&aes_ctr_key, CommonCrypto_eOtpAskm,  &custKeySettings, true, NEXUS_CryptographicOperation_eEncrypt, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, NEXUS_KeySlotPolarity_eOdd, NEXUS_KeySlotTerminationMode_eClear, false, false, 128, NEXUS_CounterMode_e0, aes_plaintext, aes_ctr_otp_cipher_text},


};


void generate_test_data(
    CommonCryptoHandle handle,

    NEXUS_CryptographicAlgorithm algType,
    NEXUS_CryptographicAlgorithmMode algVariant,

    uint8_t *pClearText,
    uint32_t size,
    uint8_t *pCiphered,
    CommonCryptoKeyLadderSettings *pCustKeySettings,
    CommonCryptoKeySrc keySrc,
    CommonCryptoKeyIvSettings *pIvSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommonCryptoClearKeySettings ivSettings;
    CommonCryptoKeyConfigSettings  configSettings;
    CommonCryptoCipheredKeySettings cipheredKeySettings;
    CommonCryptoJobSettings jobSettings;
    NEXUS_KeySlotHandle      keySlot = 0;


   NEXUS_KeySlotAllocateSettings keySlotSettings;


    void *pBuf = NULL;
    NEXUS_DmaJobBlockSettings blk;

    /* Allocate a key slot for the test */

    NEXUS_KeySlot_GetDefaultAllocateSettings(&keySlotSettings);


    keySlotSettings.useWithDma = true;
    keySlotSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keySlotSettings.slotType =NEXUS_KeySlotType_eIvPerBlock;
    keySlot =  NEXUS_KeySlot_Allocate(&keySlotSettings);

    if(!keySlot)
    {
        BDBG_ERR(("%s - NEXUS_KeySlot_Allocate failed\n", BSTD_FUNCTION));
        goto errorExit;
    }


    CommonCrypto_GetDefaultKeyConfigSettings(&configSettings);
    configSettings.keySlot = keySlot;
    configSettings.settings.opType = NEXUS_CryptographicOperation_eEncrypt;
    configSettings.settings.algType = algType;
    configSettings.settings.algVariant = algVariant;
    configSettings.settings.termMode = NEXUS_KeySlotTerminationMode_eClear;
    configSettings.settings.aesCounterSize = 128;
    configSettings.settings.aesCounterMode = NEXUS_CounterMode_e0;

    rc = CommonCrypto_LoadKeyConfig(handle,  &configSettings);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_LoadKeyConfig failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    CommonCrypto_GetDefaultCipheredKeySettings(&cipheredKeySettings);


    cipheredKeySettings.keySlot = keySlot;
    cipheredKeySettings.keySrc = keySrc;

    cipheredKeySettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
    BDBG_MSG(("%s:Keyslotentry =%d", BSTD_FUNCTION, cipheredKeySettings.keySlotEntryType ));
    BKNI_Memcpy(&cipheredKeySettings.settings, pCustKeySettings, sizeof(CommonCryptoKeyLadderSettings));

    rc = CommonCrypto_LoadCipheredKey(handle, &cipheredKeySettings);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_LoadCipheredKey failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    if(algVariant == NEXUS_CryptographicAlgorithmMode_eCbc
        || algVariant ==  NEXUS_CryptographicAlgorithmMode_eCounter)
    {
        if(pIvSettings == NULL){
            BDBG_ERR(("%s - Invalid parameter\n", BSTD_FUNCTION));
            goto errorExit;
        }

        CommonCrypto_GetDefaultClearKeySettings(&ivSettings);
        ivSettings.keySlot = keySlot;
        BKNI_Memcpy(ivSettings.settings.iv, pIvSettings->iv, 16);
        ivSettings.settings.ivSize = 16;

        ivSettings.keySlotEntryType= NEXUS_KeySlotBlockEntry_eCpsClear;


        rc =  CommonCrypto_LoadClearKeyIv(handle, &ivSettings);
        if(rc != NEXUS_SUCCESS){
            BDBG_ERR(("%s - CommonCrypto_LoadClearKeyIv failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }
    }

    rc = NEXUS_Memory_Allocate(size, NULL, &pBuf);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    BKNI_Memcpy(pBuf, pClearText, size);
    NEXUS_DmaJob_GetDefaultBlockSettings(&blk);

    blk.pSrcAddr =  pBuf;
    blk.pDestAddr = pBuf;
    blk.blockSize = size;
    blk.scatterGatherCryptoStart = true;
    blk.scatterGatherCryptoEnd = true;
    blk.resetCrypto = true;


    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = keySlot;

    rc = CommonCrypto_DmaXfer(handle, &jobSettings, &blk, 1);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_DmaXfer failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    BKNI_Memcpy(pCiphered, pBuf, size);

errorExit:
    if(keySlot) NEXUS_KeySlot_Free(keySlot);

    if(pBuf) NEXUS_Memory_Free(pBuf);
}


void init_test_vectors(CommonCryptoHandle handle)
{
    DEBUG_PRINT_ARRAY("aes_plaintext", 16, aes_plaintext);

    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb,     aes_plaintext, 16, aes_ecb_cust_cipher_text, &custKeySettings, CommonCrypto_eGlobalKey, NULL);
    DEBUG_PRINT_ARRAY("aes_ecb_glob_cipher_text", 16, aes_ecb_cust_cipher_text);

    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc,     aes_plaintext, 16, aes_cbc_cust_cipher_text, &custKeySettings, CommonCrypto_eGlobalKey, &aes_cbc_key);
    DEBUG_PRINT_ARRAY("aes_cbc_glob_cipher_text", 16, aes_cbc_cust_cipher_text);

    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, aes_plaintext, 16, aes_ctr_cust_cipher_text, &custKeySettings, CommonCrypto_eGlobalKey, &aes_ctr_key);
    DEBUG_PRINT_ARRAY("aes_ctr_glob_cipher_text", 16, aes_ctr_cust_cipher_text);


    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eEcb,     aes_plaintext, 16, aes_ecb_otp_cipher_text, &custKeySettings, CommonCrypto_eOtpAskm, NULL);
    DEBUG_PRINT_ARRAY("aes_ecb_otp_cipher_text", 16, aes_ecb_otp_cipher_text);

    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCbc,     aes_plaintext, 16, aes_cbc_otp_cipher_text, &custKeySettings, CommonCrypto_eOtpAskm, &aes_cbc_key);
    DEBUG_PRINT_ARRAY("aes_cbc_otp_cipher_text", 16, aes_cbc_otp_cipher_text);

    generate_test_data(handle, NEXUS_CryptographicAlgorithm_eAes128, NEXUS_CryptographicAlgorithmMode_eCounter, aes_plaintext, 16, aes_ctr_otp_cipher_text, &custKeySettings, CommonCrypto_eOtpAskm, &aes_ctr_key);
    DEBUG_PRINT_ARRAY("aes_ctr_otp_cipher_text", 16, aes_ctr_otp_cipher_text);

}





NEXUS_Error run_test_keyladder(CommonCryptoHandle handle, test_case_key_ladder *pTest)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_KeySlotHandle      keySlot;
    uint8_t *pBuf = NULL;
    uint8_t *pIv = NULL;

    uint32_t nb_blks = 0;
    NEXUS_DmaJobBlockSettings blks[2];
    NEXUS_KeySlotAllocateSettings keySlotSettings;
    CommonCryptoClearKeySettings ivSettings;
    CommonCryptoJobSettings jobSettings;
    uint8_t *pExpectedResult;
    int ii;

    /* Allocate a key slot for the test */
    NEXUS_KeySlot_GetDefaultAllocateSettings(&keySlotSettings);


    keySlotSettings.useWithDma = true;
    keySlotSettings.owner      = NEXUS_SecurityCpuContext_eHost;
    keySlotSettings.slotType   = NEXUS_KeySlotType_eIvPerBlock;
    keySlot =  NEXUS_KeySlot_Allocate(&keySlotSettings);


    if(!keySlot)
    {
        BDBG_ERR(("%s - NEXUS_KeySlot_Allocate failed\n", BSTD_FUNCTION));
        return NEXUS_NOT_AVAILABLE;
    }

    if(pTest->use_CommonCryptoSetupKey){


        CommonCryptoKeySettings keySettings;

        CommonCrypto_GetDefaultKeySettings(&keySettings, pTest->keySrc);
        keySettings.keySrc = pTest->keySrc;
        keySettings.keySlot = keySlot;
        if((!pTest->enableExtKey)&&(!pTest->enableExtIv))
            BKNI_Memcpy(&keySettings.src.keyLadderInfo,  pTest->pKeyLadderSettings, sizeof(CommonCryptoKeyLadderSettings));
        keySettings.alg.opType = pTest->opType;
        keySettings.alg.algType =  pTest->algType;
        keySettings.alg.algVariant = pTest->algVariant;
        keySettings.alg.keySlotType = pTest->keySlotType;
        keySettings.alg.termMode = pTest->termMode;
        keySettings.alg.enableExtKey = pTest->enableExtKey;
        keySettings.alg.enableExtIv = pTest->enableExtIv;
        keySettings.alg.aesCounterSize = pTest->aesCounterSize;
        keySettings.alg.aesCounterMode = pTest->aesCounterMode;

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
            keySettings.alg.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
        if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
            keySettings.alg.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;

        rc = CommonCrypto_SetupKey(handle, &keySettings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - CommonCrypto_SetupKey failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }

    }
    else {
        CommonCryptoKeyConfigSettings configSettings;
        CommonCryptoCipheredKeySettings cipheredKeySettings;

        CommonCrypto_GetDefaultKeyConfigSettings(&configSettings);

        configSettings.keySlot = keySlot;
        BDBG_MSG(("%s: configSettings.keySlot=%p", BSTD_FUNCTION, configSettings.keySlot));

        configSettings.settings.opType         = pTest->opType;
        configSettings.settings.algType        = pTest->algType;
        configSettings.settings.algVariant     = pTest->algVariant;
        configSettings.settings.keySlotType    = pTest->keySlotType;
        configSettings.settings.termMode       = pTest->termMode;
        configSettings.settings.enableExtKey   = pTest->enableExtKey;
        configSettings.settings.enableExtIv    = pTest->enableExtIv;
        configSettings.settings.aesCounterSize = pTest->aesCounterSize;
        configSettings.settings.aesCounterMode = pTest->aesCounterMode;

        rc = CommonCrypto_LoadKeyConfig(handle, &configSettings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - CommonCrypto_LoadKeyConfig failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }

        CommonCrypto_GetDefaultCipheredKeySettings(&cipheredKeySettings);
        cipheredKeySettings.keySlot = keySlot;

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
           cipheredKeySettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
        if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
            cipheredKeySettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;

        cipheredKeySettings.keySrc = pTest->keySrc;

        BKNI_Memcpy(&cipheredKeySettings.settings, pTest->pKeyLadderSettings, sizeof(CommonCryptoKeyLadderSettings));

        rc = CommonCrypto_LoadCipheredKey(handle, &cipheredKeySettings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - CommonCrypto_LoadCipheredKey failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }
    }

    rc = NEXUS_Memory_Allocate(16, NULL, (void *)&pBuf);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    if(pTest->opType == NEXUS_CryptographicOperation_eEncrypt) {
        BKNI_Memcpy(pBuf, pTest->pClear, 16);
        pExpectedResult = pTest->pCiphered;
    }
    else {
        /* Assume we are decryption */
        BKNI_Memcpy(pBuf, pTest->pCiphered, 16);
        pExpectedResult = pTest->pClear;
    }

    for(ii = 0; ii < 2; ii++) {
        NEXUS_DmaJob_GetDefaultBlockSettings(&blks[ii]);
        blks[ii].cached = true;
    }

    blks[0].scatterGatherCryptoStart = true;
    blks[0].resetCrypto = true;

    if(pTest->algVariant == NEXUS_CryptographicAlgorithmMode_eCbc
        || pTest->algVariant == NEXUS_CryptographicAlgorithmMode_eCounter)
    {
        if(!pTest->enableExtIv){
            CommonCrypto_GetDefaultClearKeySettings(&ivSettings);
            BKNI_Memcpy(ivSettings.settings.iv, pTest->pIvSettings->iv, 16);
            ivSettings.settings.ivSize = 16;
            ivSettings.keySlot = keySlot;

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
            ivSettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
        if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
            ivSettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;

            rc =  CommonCrypto_LoadClearKeyIv(handle, &ivSettings);
            if(rc != NEXUS_SUCCESS){
                BDBG_ERR(("%s - CommonCrypto_LoadClearKeyIv failed, rc = %d\n", BSTD_FUNCTION, rc));
                goto errorExit;
            }
        }
        else {
            rc = NEXUS_Memory_Allocate(16, NULL, (void *)&pIv);
            if(rc != NEXUS_SUCCESS){
                BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
                goto errorExit;
            }
            printf("%s - %d\n", BSTD_FUNCTION, __LINE__);

            /*BKNI_Memcpy(pIv, pTest->pIvSettings->iv, 16);*/
             /* Copy IV.  H and L need to be swapped */
            BKNI_Memcpy(pIv,  pTest->pIvSettings->iv + 8, 8);
            BKNI_Memcpy(pIv + 8, pTest->pIvSettings->iv, 8);

            /*  Set up block*/
            blks[nb_blks].pSrcAddr =  pIv;
            blks[nb_blks].pDestAddr = pIv;
            blks[nb_blks].blockSize = 16;
            nb_blks++;
        }
    }
    blks[nb_blks].pSrcAddr =  pBuf;
    blks[nb_blks].pDestAddr = pBuf;
    blks[nb_blks].blockSize = 16;
    blks[nb_blks].scatterGatherCryptoEnd = true;
    nb_blks++;

    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = keySlot;

    DEBUG_PRINT_ARRAY("BEFORE pBuf", 16, pBuf );
    if( pIv ) DEBUG_PRINT_ARRAY("pIv", 16, pIv );

    rc = CommonCrypto_DmaXfer(handle, &jobSettings, blks, nb_blks);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_DmaXfer failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    DEBUG_PRINT_ARRAY("AFTER pBuf", 16, pBuf );
    DEBUG_PRINT_ARRAY("AFTER pExpectedResult", 16, pExpectedResult );

    if(BKNI_Memcmp(pBuf, pExpectedResult, 16) != 0) {
        BDBG_ERR(("%s - DMA operation failed\n", BSTD_FUNCTION));
        rc = NEXUS_UNKNOWN;
    }

errorExit:
    if(keySlot) NEXUS_KeySlot_Free(keySlot);
    if(pBuf) NEXUS_Memory_Free(pBuf);
    if(pIv) NEXUS_Memory_Free(pIv);

    return rc;
}


NEXUS_Error run_test_clear_key(CommonCryptoHandle handle, test_case_clear_key *pTest)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_KeySlotHandle      keySlot;
    uint8_t *pBuf = NULL;
    uint8_t *pKey = NULL;
    uint8_t *pIv = NULL;
    uint32_t nb_blks = 0;
    NEXUS_DmaJobBlockSettings blks[3];
    uint8_t      *pSrc = NULL, *pDest = NULL, *pExternalKey =NULL, *pExternalKeyShaddow =NULL;
    NEXUS_KeySlotBlockEntry entry;
    NEXUS_KeySlotAllocateSettings keySlotSettings;

    CommonCryptoClearKeySettings keyIvSettings;
    CommonCryptoJobSettings jobSettings;
    uint8_t *pExpectedResult = NULL;
    int ii;

    /* Allocate a key slot for the test */
    NEXUS_KeySlot_GetDefaultAllocateSettings(&keySlotSettings);

    keySlotSettings.useWithDma = true;
    //keySlotSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keySlotSettings.slotType =NEXUS_KeySlotType_eIvPerBlock;
    keySlot =  NEXUS_KeySlot_Allocate(&keySlotSettings);

    if(!keySlot)
    {
        BDBG_ERR(("%s - NEXUS_KeySlot_Allocate failed\n", BSTD_FUNCTION));
        return NEXUS_NOT_AVAILABLE;
    }

    BDBG_MSG(("%s: Allocated keyslot=%p", BSTD_FUNCTION, keySlot));

    if(pTest->use_CommonCryptoSetupKey){
        CommonCryptoKeySettings keySettings;
        CommonCrypto_GetDefaultKeySettings(&keySettings, CommonCrypto_eClearKey);
        keySettings.keySrc = CommonCrypto_eClearKey;
        keySettings.keySlot = keySlot;
        if((!pTest->enableExtKey)&&(!pTest->enableExtIv))
            BKNI_Memcpy(&keySettings.src.keyIvInfo,  pTest->pKeyIvSettings, sizeof(CommonCryptoKeyIvSettings));
        keySettings.alg.keySlotType = pTest->keySlotType;
        keySettings.alg.opType = pTest->opType;
        keySettings.alg.algType =  pTest->algType;
        keySettings.alg.algVariant = pTest->algVariant;
        keySettings.alg.termMode = pTest->termMode;
        keySettings.alg.enableExtKey = pTest->enableExtKey;
        keySettings.alg.enableExtIv = pTest->enableExtIv;
        keySettings.alg.aesCounterSize = pTest->aesCounterSize;
        keySettings.alg.aesCounterMode = pTest->aesCounterMode;

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
                 keySettings.alg.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
        if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
                 keySettings.alg.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;

        rc = CommonCrypto_SetupKey(handle, &keySettings);
        if(rc != NEXUS_SUCCESS){
            BDBG_ERR(("%s - CommonCrypto_SetupKey failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }

    }
    else {
        BDBG_LOG(("%s: CommonCrypto_LoadKeyConfig in secv2 ...", BSTD_FUNCTION));
        CommonCryptoKeyConfigSettings configSettings;

        CommonCrypto_GetDefaultKeyConfigSettings(&configSettings);
        configSettings.keySlot = keySlot;

        BDBG_MSG(("%s: configSettings.keySlot=%p", BSTD_FUNCTION, configSettings.keySlot));
        configSettings.settings.opType = pTest->opType;
        configSettings.settings.algType =  pTest->algType;
        configSettings.settings.algVariant = pTest->algVariant;
        configSettings.settings.keySlotType = pTest->keySlotType;
        configSettings.settings.termMode = pTest->termMode;
        configSettings.settings.enableExtKey = pTest->enableExtKey;
        configSettings.settings.enableExtIv = pTest->enableExtIv;
        configSettings.settings.aesCounterSize = pTest->aesCounterSize;
        configSettings.settings.aesCounterMode = pTest->aesCounterMode;
        configSettings.settings.termMode = pTest->termMode;

        rc = CommonCrypto_LoadKeyConfig(handle, &configSettings);
        if(rc != NEXUS_SUCCESS){
            BDBG_ERR(("%s - CommonCrypto_LoadKeyConfig failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
        }

    }
        CommonCrypto_GetDefaultClearKeySettings(&keyIvSettings);
        keyIvSettings.keySlot = keySlot;

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
                keyIvSettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
        if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
            keyIvSettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;

        /* Handle external key/iv cases */
        if(!pTest->enableExtKey){
            BKNI_Memcpy(keyIvSettings.settings.key, pTest->pKeyIvSettings->key, pTest->pKeyIvSettings->keySize);
            keyIvSettings.settings.keySize = pTest->pKeyIvSettings->keySize;
        }
        if(!pTest->enableExtIv){
            BKNI_Memcpy(keyIvSettings.settings.iv, pTest->pKeyIvSettings->iv,  pTest->pKeyIvSettings->ivSize);
            keyIvSettings.settings.ivSize = pTest->pKeyIvSettings->ivSize;
        }
        /* Load key and or IV */
        BDBG_MSG(("%s:keyIvSettings.settings.keySize %x, keyIvSettings.settings.ivSize %x\n", BSTD_FUNCTION,keyIvSettings.settings.keySize, keyIvSettings.settings.ivSize));
        if(keyIvSettings.settings.keySize != 0 || keyIvSettings.settings.ivSize != 0) {
            BDBG_MSG(("%s: calling CommonCrypto_LoadClearKeyIv ...", BSTD_FUNCTION));
            rc =  CommonCrypto_LoadClearKeyIv(handle, &keyIvSettings);
            if(rc != NEXUS_SUCCESS){
                BDBG_ERR(("%s - CommonCrypto_LoadClearKeyIv failed, rc = %d\n", BSTD_FUNCTION, rc));
                goto errorExit;
            }
        }


    rc = NEXUS_Memory_Allocate(16, NULL, (void *)&pBuf);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }


    if(pTest->opType == NEXUS_CryptographicOperation_eEncrypt){
        BKNI_Memcpy(pBuf, pTest->pClear, 16);
        pExpectedResult = pTest->pCiphered;
    }
    else {
        /* Assume we are decryptin */
        BKNI_Memcpy(pBuf, pTest->pCiphered, 16);
        pExpectedResult = pTest->pClear;
    }

    for(ii = 0; ii < 3; ii++) {
        NEXUS_DmaJob_GetDefaultBlockSettings(&blks[ii]);
        blks[ii].cached = true;
    }

    blks[0].scatterGatherCryptoStart = true;
    blks[0].resetCrypto = true;
    BDBG_MSG(("%s:pTest->enableExtKey =%d, pTest->enableExtIv=%d", BSTD_FUNCTION, pTest->enableExtKey,pTest->enableExtIv));

    if(pTest->enableExtKey){


        CommonCryptoExternalKeyData btp;

        pSrc = pDest = pExternalKey = pExternalKeyShaddow = NULL;
        NEXUS_KeySlotExternalKeyData extKeyData;


         /* Configure a key entry for encryption/decryption. */

        if(pTest->opType==NEXUS_CryptographicOperation_eEncrypt)
                 entry = NEXUS_KeySlotBlockEntry_eCpsClear;
            if(pTest->opType==NEXUS_CryptographicOperation_eDecrypt)
                 entry = NEXUS_KeySlotBlockEntry_eCpdClear;
            BDBG_MSG(("%s: Allocated keyslot=%p, entry=%d", BSTD_FUNCTION,keySlot,entry));
        rc = NEXUS_KeySlot_GetEntryExternalKeySettings( keySlot, entry, &extKeyData );
        if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto errorExit; }

        BDBG_MSG(("%s: extKeyData.slotIndex=%d,extKeyData.key.valid =%d,extKeyData.iv.valid=%d", BSTD_FUNCTION, extKeyData.slotIndex, extKeyData.key.valid, extKeyData.iv.valid ));


        /* The actual key size for the algorithm selected. */
        //key_size = CommonCryptoGetAlogrithmKeySize(pTest->algType );
        BDBG_MSG(("%s: keysize = %d, iv size =%d", BSTD_FUNCTION, pTest->pKeyIvSettings->keySize, pTest->pKeyIvSettings->ivSize));
        /* IV is 128 bits for AES, 64 bits for DES. */
        //iv_size = 128 / 8;
        BDBG_MSG(("%s: Allocate memeory for btp packet", BSTD_FUNCTION));
        BKNI_Memset( &btp, 0, sizeof( btp ) );
        btp.slotIndex = extKeyData.slotIndex;

        if( extKeyData.key.valid ) {

            rc = NEXUS_Memory_Allocate(pTest->pKeyIvSettings->keySize, NULL, (void *)&pKey);
            if(rc != NEXUS_SUCCESS){
                    BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
            goto errorExit;
            }
             /* Copy key.   */
            BKNI_Memcpy(pKey , pTest->pKeyIvSettings->key, pTest->pKeyIvSettings->keySize);

            btp.key.valid = true;
            btp.key.offset = extKeyData.key.offset;
            btp.key.size = pTest->pKeyIvSettings->keySize;
            btp.key.pData = pKey;
        }

        if( extKeyData.iv.valid ) {

            rc = NEXUS_Memory_Allocate(pTest->pKeyIvSettings->ivSize, NULL, (void *)&pIv);
            if(rc != NEXUS_SUCCESS){
                BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc));
                goto errorExit;
            }
            /* Copy IV.  */
            BKNI_Memcpy(pIv,  pTest->pKeyIvSettings->iv ,pTest->pKeyIvSettings->ivSize);
            btp.iv.valid = true;
            btp.iv.offset = extKeyData.iv.offset;
            btp.iv.pData = pIv;
            btp.iv.size = pTest->pKeyIvSettings->ivSize;
        }
        NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKey );
        NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &pExternalKeyShaddow );
        BKNI_Memset( pExternalKey, 0, XPT_TS_PACKET_SIZE );
        BKNI_Memset( pExternalKeyShaddow, 0, XPT_TS_PACKET_SIZE );

        BDBG_MSG(("%s: compile btp packet ", BSTD_FUNCTION));
        /* Compile a Broadcom Transport Packet into pExternalKey */
        CommonCrypto_CompileBtp( pExternalKey, &btp );

        /* Load the loaded key and IV from BTP to the key entry specified. */
        BDBG_LOG(("%s: Set up block with BTP pkt at blk %d ", BSTD_FUNCTION,nb_blks));

    /*  Set up block with key*/
        blks[nb_blks].pSrcAddr =  pExternalKey;
        blks[nb_blks].pDestAddr = pExternalKeyShaddow;
        blks[nb_blks].blockSize = XPT_TS_PACKET_SIZE;

        blks[nb_blks].securityBtp = true;

         blks[nb_blks].scatterGatherCryptoEnd = true;
        nb_blks++;
        CommonCrypto_GetDefaultJobSettings(&jobSettings);
        jobSettings.keySlot = keySlot;

   }

    BDBG_LOG(("%s: Set up block with data pkt at blk %d", BSTD_FUNCTION,nb_blks));
    blks[nb_blks].pSrcAddr =  pBuf;
    blks[nb_blks].pDestAddr = pBuf;
    blks[nb_blks].blockSize = 16;
    blks[nb_blks].scatterGatherCryptoEnd = true;
    blks[nb_blks].securityBtp = false;

        blks[nb_blks].scatterGatherCryptoStart = true;
        blks[nb_blks].resetCrypto = true;

    nb_blks++;

    DEBUG_PRINT_ARRAY("BEFORE pBuf", 16, pBuf);

    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = keySlot;

    rc = CommonCrypto_DmaXfer(handle, &jobSettings, blks, nb_blks);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_DmaXfer failed, rc = %d\n", BSTD_FUNCTION, rc));
        goto errorExit;
    }

    DEBUG_PRINT_ARRAY("AFTER pBuf", 16, pBuf);
    DEBUG_PRINT_ARRAY("AFTER pExpectedResult", 16, pExpectedResult);
    /*if(pExternalKey)
    DEBUG_PRINT_ARRAY( "pExternalKey", XPT_TS_PACKET_SIZE, pExternalKey );
    if(pExternalKeyShaddow)
    DEBUG_PRINT_ARRAY( "pExternalKeyShaddow", XPT_TS_PACKET_SIZE, pExternalKeyShaddow );*/

    if(BKNI_Memcmp(pBuf, pExpectedResult, 16) != 0){
        BDBG_ERR(("%s -  DMA operation failed\n", BSTD_FUNCTION));
        rc = NEXUS_UNKNOWN;
    }

errorExit:


    if(keySlot) NEXUS_KeySlot_Free(keySlot);
    if(pBuf) NEXUS_Memory_Free(pBuf);
    if(pKey) NEXUS_Memory_Free(pKey);
    if(pIv) NEXUS_Memory_Free(pIv);
    if(pExternalKey) NEXUS_Memory_Free(pExternalKey);
    if(pExternalKeyShaddow) NEXUS_Memory_Free(pExternalKeyShaddow);
    if (pDest) NEXUS_Memory_Free(pDest);
    return rc;
}

int main(void)
{
    CommonCryptoHandle handle;
    CommonCryptoSettings  cryptoSettings;
    int nb_tests= 0;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int ii;
    int global_error_count = 0;

#ifdef NXCLIENT_SUPPORT

    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;
    NxClient_AllocResults allocResults;

    printf("\tBringing up nxclient\n\n");
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "test_common_crypto");
    joinSettings.ignoreStandbyRequest = true;

    rc = NxClient_Join(&joinSettings);
    if (rc)
    {
        global_error_count++;
        printf("\tnxclient failed to join\n\n");
        goto handle_error;
    }
    NxClient_GetDefaultAllocSettings(&nxAllocSettings);
    rc = NxClient_Alloc(&nxAllocSettings, &allocResults);
    if (rc)
    {
        printf("\tNxClient_Alloc failed\n\n");
        goto handle_error;
    }
    printf("\tnxclient has joined successfully\n");
#else
    NEXUS_PlatformSettings platformSettings;
    printf("\tBringing up all Nexus modules for platform using default settings\n\n");

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if(rc != NEXUS_SUCCESS)
    {
        global_error_count++;
        printf("%s - Failure to initialize NEXUS platform.\n", BSTD_FUNCTION);
        goto handle_error;
    }
#endif

    CommonCrypto_GetDefaultSettings(&cryptoSettings);
#ifdef NEXUS_VIDEO_SECURE_HEAP
    cryptoSettings.videoSecureHeapIndex = NEXUS_VIDEO_SECURE_HEAP;
#endif
    handle = CommonCrypto_Open(&cryptoSettings);
    if(handle == NULL)
    {
        global_error_count++;
        printf("%s - Failure to open Common Crypto.\n", BSTD_FUNCTION);
        goto handle_error;
    }
     printf("\n%s - initializing test vectors for cipheredkey operations \n", BSTD_FUNCTION);
    init_test_vectors(handle);

    nb_tests = sizeof(tests_clear_key) / sizeof(tests_clear_key[0]);


    printf("\n%s - Running clear key tests\n", BSTD_FUNCTION);
    for(ii = 0; ii < nb_tests; ii++)
    {
        printf("Clear key test no %2d:", ii);
        rc = run_test_clear_key(handle, &tests_clear_key[ii]);
        if(rc != NEXUS_SUCCESS)
        {
            printf("  clear key test FAILED (%u)\n", global_error_count);
            global_error_count++;
        }
        else{
            printf("  Succeeded\n");
        }
    }



    printf("\n%s - Running key ladder tests\n", BSTD_FUNCTION);
    nb_tests = sizeof(tests_key_ladder) / sizeof(tests_key_ladder[0]);
    for(ii = 0; ii < nb_tests; ii++)
    {
        printf("key ladder test no %2d:", ii);
        rc = run_test_keyladder(handle, &tests_key_ladder[ii]);
        if(rc != NEXUS_SUCCESS)
        {
            printf("  key ladder test FAILED (%u)\n", global_error_count);
            global_error_count++;
        }
        else{
            printf(" Succeeded\n");
        }
    }

    printf("\n");

handle_error:

    printf("************************************************************\n");
    if(global_error_count == 0)
    {
        printf("* Common Crypto Test Application completed successfully.\n");
    }
    else
    {
        printf("* Common Crypto Test Application completed with %u error%s.\n", global_error_count, ((global_error_count > 1) ? "s" : ""));
    }
    printf("************************************************************\n\n");

    if(handle != NULL)
    {
        CommonCrypto_Close(handle);
        handle = NULL;
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_Free(&allocResults);
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif

    return global_error_count;
}
#else
BDBG_ERR(("%s: Nexus Secv1 is unsupported on this platform", BSTD_FUNCTION));
#endif
