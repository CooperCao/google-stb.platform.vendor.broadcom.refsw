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


#ifndef BHSM_SECURITY_COMMON_H__
#define BHSM_SECURITY_COMMON_H__

typedef enum BHSM_SecurityCpuContext
{
    BHSM_SecurityCpuContext_eHost,         /* host CPU. */
    BHSM_SecurityCpuContext_eSage,         /* SAGE CPU. */

    BHSM_SecurityCpuContext_eMax
}BHSM_SecurityCpuContext;

typedef enum BHSM_KeyslotType
{
    BHSM_KeyslotType_eIvPerSlot,     /* SMALL keyslot. One IV for the whole keyslot. Suitable for most CA decryption. (128bit)*/
    BHSM_KeyslotType_eIvPerBlock,    /* MEDIUM keyslot. One IV per Block (CPD/CA/CPS). Suitable for most block mode decryption (128bit). */
    BHSM_KeyslotType_eIvPerEntry,    /* LARGE keyslot. One IV per Entry. Required to support a small number of scenarios (128bit). */
    BHSM_KeyslotType_eIvPerBlock256, /* MEDIUM keyslot. One IV per Block (CPD/CA/CPS). Suitable for most block mode decryption. */
    BHSM_KeyslotType_eIvPerEntry256, /* LARGE keyslot. One IV per Entry. Required to support a small number of scenarios. */
    BHSM_KeyslotType_eMulti2,        /* Multi2 keyslot is specially handled by different BSP command. */
    BHSM_KeyslotType_eMax
} BHSM_KeyslotType;

typedef enum BHSM_SecurityRegion
{
    BHSM_SecurityRegion_eGlr,    /* Global region */
    BHSM_SecurityRegion_eCrr,    /* Compressed Data Restricted Region */
    BHSM_SecurityRegion_eUrr,    /* User Restricted region */
    BHSM_SecurityRegion_eXrr,    /* eXport Restricted Region */

    BHSM_SecurityRegion_eMax
}BHSM_SecurityRegion;


/*
    Used to indicate wheather to encrypt of decrypt.
*/
typedef enum BHSM_CryptographicOperation
{
    BHSM_CryptographicOperation_eEncrypt,
    BHSM_CryptographicOperation_eDecrypt,
    BHSM_CryptographicOperation_eMax
}BHSM_CryptographicOperation;


/*
    Enumerate all cryptographic encryption algorithms.
*/
typedef enum BHSM_CryptographicAlgorithm
{
    BHSM_CryptographicAlgorithm_eDvbCsa2,
    BHSM_CryptographicAlgorithm_eDvbCsa3,
    BHSM_CryptographicAlgorithm_eMulti2,
    BHSM_CryptographicAlgorithm_eDes,
    BHSM_CryptographicAlgorithm_e3DesAba,
    BHSM_CryptographicAlgorithm_e3DesAbc,
    BHSM_CryptographicAlgorithm_eAes128,
    BHSM_CryptographicAlgorithm_eAes192,
    BHSM_CryptographicAlgorithm_eAes256,
    BHSM_CryptographicAlgorithm_eCam128,
    BHSM_CryptographicAlgorithm_eCam192,
    BHSM_CryptographicAlgorithm_eCam256,
    BHSM_CryptographicAlgorithm_eGhash,
    BHSM_CryptographicAlgorithm_eC2,             /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eCss,            /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eM6Ke,           /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eM6S,            /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eRc4,            /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eMsMultiSwapMac, /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eWmDrmPd,        /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eAes128G,        /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eHdDvd,          /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eBrDvd,          /* pre Zeus5 */
    BHSM_CryptographicAlgorithm_eReserved19,

    BHSM_CryptographicAlgorithm_eMax
} BHSM_CryptographicAlgorithm;


typedef enum BHSM_CryptographicAlgorithmMode
{
    BHSM_CryptographicAlgorithmMode_eEcb,        /* Electronic Code Book */
    BHSM_CryptographicAlgorithmMode_eCbc,        /* Cipher Block Chaining */
    BHSM_CryptographicAlgorithmMode_eCounter,    /* Counter */
    BHSM_CryptographicAlgorithmMode_eRcbc,
    BHSM_CryptographicAlgorithmMode_eMax
} BHSM_CryptographicAlgorithmMode;

typedef enum BHSM_CounterMode
{
    BHSM_CounterMode_e0,  /* Generic Mode for full blocks only. Partial block will not be processed. */
    BHSM_CounterMode_e1,  /* Generic Mode for full blocks and partial blocks. */
    BHSM_CounterMode_e2,  /* Full blocks only. Parse packets generated by playback for IV. */
    BHSM_CounterMode_e3,  /* Partial block continuous to beginning of next packet. */
    BHSM_CounterMode_e4,  /* Full blocks and partial blocks. Skip PES header. */
    BHSM_CounterMode_eMax
} BHSM_CounterMode;


typedef enum
{
    BHSM_HashType_e1_160,   /* SHA-1, 160 bit digest. */
    BHSM_HashType_e2_256,   /* SHA-2, 256 bit digest. */
    BHSM_HashType_e2_512,   /* SHA-2, 512 bit digest. */

    BHSM_HashType_eMax
}BHSM_HashType;

#endif
