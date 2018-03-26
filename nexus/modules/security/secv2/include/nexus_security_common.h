/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#ifndef NEXUS_SECURITY_COMMON_H__
#define NEXUS_SECURITY_COMMON_H__

typedef enum NEXUS_SecurityCpuContext
{
    NEXUS_SecurityCpuContext_eHost,         /* host CPU. */
    NEXUS_SecurityCpuContext_eSage,         /* SAGE CPU. */

    NEXUS_SecurityCpuContext_eMax
}NEXUS_SecurityCpuContext;

typedef enum NEXUS_SigningAuthority
{
    NEXUS_SigningAuthority_eBroadcom,
    NEXUS_SigningAuthority_eCaVendor,

    NEXUS_SigningAuthority_eMax
}NEXUS_SigningAuthority;



typedef enum NEXUS_SecurityRegion
{
    NEXUS_SecurityRegion_eGlr,    /* Global region */
    NEXUS_SecurityRegion_eCrr,    /* Compressed Data Restricted Region */
    NEXUS_SecurityRegion_eUrr,    /* User Restricted region */
    NEXUS_SecurityRegion_eXrr,    /* eXport Restricted Region */

    NEXUS_SecurityRegion_eMax
}NEXUS_SecurityRegion;


/*
    Used to indicate wheather to encrypt of decrypt.
*/
typedef enum NEXUS_CryptographicOperation
{
    NEXUS_CryptographicOperation_eEncrypt,
    NEXUS_CryptographicOperation_eDecrypt,
    NEXUS_CryptographicOperation_eMax
}NEXUS_CryptographicOperation;


/*
    Enumerate all cryptographic encryption algorithms.
*/
typedef enum NEXUS_CryptographicAlgorithm
{
    NEXUS_CryptographicAlgorithm_eDvbCsa2,
    NEXUS_CryptographicAlgorithm_eDvbCsa3,
    NEXUS_CryptographicAlgorithm_eMulti2,
    NEXUS_CryptographicAlgorithm_eDes,
    NEXUS_CryptographicAlgorithm_e3DesAba,
    NEXUS_CryptographicAlgorithm_e3DesAbc,
    NEXUS_CryptographicAlgorithm_eAes128,
    NEXUS_CryptographicAlgorithm_eAes192,
    NEXUS_CryptographicAlgorithm_eAes256,
    NEXUS_CryptographicAlgorithm_eCam128,
    NEXUS_CryptographicAlgorithm_eCam192,
    NEXUS_CryptographicAlgorithm_eCam256,
    NEXUS_CryptographicAlgorithm_eGhash,
    NEXUS_CryptographicAlgorithm_eC2,             /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eM6Ke,           /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eM6S,            /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eRc4,            /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eMsMultiSwapMac, /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eWmDrmPd,        /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eAes128G,        /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eHdDvd,          /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eBrDvd,          /* pre Zeus5 */
    NEXUS_CryptographicAlgorithm_eReserved19,     /* pre Zeus5 */

    NEXUS_CryptographicAlgorithm_eMax
} NEXUS_CryptographicAlgorithm;


typedef enum NEXUS_CryptographicAlgorithmMode
{
    NEXUS_CryptographicAlgorithmMode_eEcb,        /* Electronic Code Book */
    NEXUS_CryptographicAlgorithmMode_eCbc,        /* Cipher Block Chaining */
    NEXUS_CryptographicAlgorithmMode_eCounter,    /* Counter */
    NEXUS_CryptographicAlgorithmMode_eMax
} NEXUS_CryptographicAlgorithmMode;

typedef enum NEXUS_CounterMode
{
    NEXUS_CounterMode_e0,  /* Generic Mode for full blocks only. Partial block will not be processed. */
    NEXUS_CounterMode_e1,  /* Generic Mode for full blocks and partial blocks. */
    NEXUS_CounterMode_e2,  /* Full blocks only. Parse packets generated by playback for IV. */
    NEXUS_CounterMode_e3,  /* Partial block continuous to beginning of next packet. */
    NEXUS_CounterMode_e4,  /* Full blocks and partial blocks. Skip PES header. */
    NEXUS_CounterMode_eMax
} NEXUS_CounterMode;




typedef enum
{
    NEXUS_HashType_e1_160,   /* SHA-1, 160 bit digest. */
    NEXUS_HashType_e2_224,   /* SHA-2, 224 bit digest. */
    NEXUS_HashType_e2_256,   /* SHA-2, 256 bit digest. */

    NEXUS_HashType_eMax
}NEXUS_HashType;

#endif
