/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef KEY_LOADER_TL_TYPES_H__
#define KEY_LOADER_TL_TYPES_H__

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_crypto_types.h"

#define KEY_LOADER_INVALID_KEYSLOT_INDEX  (0xFFFF)
#define KEY_LOADER_INVALID_PROFILE_INDEX  (0xFFFF)

typedef struct
{
    uint32_t  profileIndex;
    BSAGElib_Crypto_Engine_e             engine;
    BSAGElib_Crypto_CustomerSubMode_e    custSubMode; /* i.e. CP_128_4*/
    BSAGElib_Crypto_Algorithm_e          algorithm;
    BSAGElib_Crypto_AlgorithmVariant_e   algorithmVar; /* Cipher chain mode for selected cipher */
    BSAGElib_Crypto_TerminationMode_e    terminationMode; /* Termination Type for residual block to be ciphered */
    BSAGElib_Crypto_AesCounterMode_e     aesCounterMode;  /* for AesCounter algorithm    */
    BSAGElib_Crypto_AesCounterSize_e     aesCounterSize;
    BSAGElib_Crypto_SolitaryMode_e       solitarySelect; /* Process selection for short blocks in data packets to be ciphered */
    BSAGElib_Crypto_Operation_e          operation; /*   CA:   only decryption allowed
                                                         M2M:  NEXUS_SecurityOperation_eEncrypt or NEXUS_SecurityOperation_eDecrypt
                                                         CP:   NEXUS_SecurityOperation_eEncrypt or NEXUS_SecurityOperation_eDecrypt */
    BSAGElib_Crypto_KeyType_e            keyType; /* Odd/Even/OddAndEven/Clear */
} KeyLoader_KeySlotConfigSettings;


typedef struct
{
    BSAGElib_Crypto_Algorithm_e          keyladderAlg;
    BSAGElib_Crypto_KeyLadderLevel_e     keyladderDepth;
    BSAGElib_Crypto_KeyType_e            keyType; /* Odd/Even/OddAndEven/Clear */
    uint8_t                              procInForKey3[16];
    uint8_t                              procInForKey4[16];
    uint8_t                              procInForKey5[16];
    uint32_t                             keyLength; /* Key length in bytes */
    uint32_t                             ivLength;  /* IV length in bytes. Set to 0 when IV is not required by algorithm. */
    uint8_t                              iv[16];

} KeyLoader_WrappedKeySettings;

typedef struct
{
    BSAGElib_Crypto_KeyType_e            keyType;   /* Odd/Even/OddAndEven/Clear */
    uint32_t                             ivLength;  /* IV length in bytes        */
    uint8_t                              iv[16];

} KeyLoader_UpdateIvSettings;

#endif /*KEY_LOADER_TL_TYPES_H__*/
