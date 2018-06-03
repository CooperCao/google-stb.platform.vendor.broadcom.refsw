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


#ifndef BHSM_BSECK_H__
#define BHSM_BSECK_H__

#include "bhsm.h"
#include "bhsm_keyladder.h"

#include "bsp_s_download.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BHSM_MAX_2NDTIER_KEY_SIZE         256

typedef enum BHSM_2ndStageCodeVerify_e
{
    BHSM_2ndStageCodeVerify_eRSASignature,
    BHSM_2ndStageCodeVerify_eOTPHash2,

    BHSM_2ndStageCodeVerify_eMax
} BHSM_2ndStageCodeVerify_e;


typedef enum BHSM_2ndStageCodeLocation_e
{
    BHSM_2ndStageCodeLocation_eFlash,
    BHSM_2ndStageCodeLocation_eDRAM,

    BHSM_2ndStageCodeLocation_eMax
} BHSM_2ndStageCodeLocation_e;

typedef struct BHSM_VerifySecondTierKeyIO
{
    /*In: select key0. Zeus42 BFW 443 and later. */
    unsigned key0Index;

    /* In: second-tier key ID, where the key is to be installed */
    BCMD_SecondTierKeyId_e        eKeyIdentifier;

    /* In: first tier root key source */
    BCMD_FirstTierKeyId_e        eFirstTierRootKeySrc;

    /* In: Second Tier root key source */
    BCMD_SecondTierKeyId_e        eSecondTierRootKeySrc;

    /* In: If true, RootKeySource is a multi-tier key, else it's a first-tier key. */
    bool                        bMultiTierRootKeySrc;

    /* In: If true, perform chip reset upon failure */
    bool                        bChipResetOnFail;

    /* In: 32-bit address in Flash where 2nd-tier key, parameters and signature are stored */
    /* key - 256 bytes   parameters - 4 bytes  signature - 256 bytes  --first 256M is currently supported */
    uint32_t                     keyAddr;

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;

}BHSM_VerifySecondTierKeyIO_t;


typedef struct BHSM_VerifySecondStageCodeLoadIO
{
    uint32_t                    codePtr;        /* In:  Flash address of second stage code */
    uint32_t                    codeSigPtr;     /* In:  Flash address of second stage code signature */
    BHSM_2ndStageCodeVerify_e   codeVerifyOpt;  /* In:  How to verify the second stage code  */
    BHSM_2ndStageCodeLocation_e codeLocation;   /* In:  Specify where the second stage code is located */
    BCMD_SecondTierKeyId_e      eKeySelect;     /* In:  second-tier key ID, to be used for verifying the second stage code */
    uint32_t                    unStatus;       /* Out: DEPRECATED */

}BHSM_VerifySecondStageCodeLoadIO_t;


/**********************************************************************************************
                         BSECK Second Tier Key structure
**********************************************************************************************/

typedef enum BHSM_SecondTierKeyRight_e
{
    BHSM_SecondTierKeyRight_eMIPS,
    BHSM_SecondTierKeyRight_eAVProcessors,
    BHSM_SecondTierKeyRight_eBSP,
    BHSM_SecondTierKeyRight_eBoot,
    BHSM_SecondTierKeyRight_eSAGE,

    BHSM_SecondTierKeyRight_eMax

} BHSM_SecondTierKeyRight_e;


typedef enum BHSM_SecondTierKeyExp_e
{
    BHSM_SecondTierKeyExp_eExp0,
    BHSM_SecondTierKeyExp_eExp64KPlus1,

    BHSM_SecondTierKeyExp_eMax

} BHSM_SecondTierKeyExp_e;


typedef struct BHSM_SecondTierKey_t
{
    unsigned char             keyData[BHSM_MAX_2NDTIER_KEY_SIZE];
    BHSM_SecondTierKeyRight_e keyRight;
    unsigned char             reservedByte0;
    BHSM_SecondTierKeyExp_e   keyExp;
    unsigned char             reservedByte1;
    unsigned int              MarketId;
    unsigned int              MarketIdMask;
    unsigned short            reservedShort0;
    unsigned char             epochMask;
    unsigned char             epoch;
    unsigned char             keySignature[BHSM_MAX_2NDTIER_KEY_SIZE];

} BHSM_SecondTierKey_t;


typedef struct BHSM_SecondTierKeyInvalidate_t
{
    BCMD_SecondTierKeyId_e  eKeyIdentifier;   /* second-tier key ID*/

} BHSM_SecondTierKeyInvalidate_t;


/**********************************************************************************************
                         BSECK public interface functions to upper layers
**********************************************************************************************/
BERR_Code BHSM_VerifySecondTierKey (
        BHSM_Handle hHsm,
        BHSM_VerifySecondTierKeyIO_t *pSecondTierKey
);



BERR_Code BHSM_SecondTierKeyInvalidate(
        BHSM_Handle hHsm,
        BHSM_SecondTierKeyInvalidate_t *pSecondTierKey
);



BERR_Code BHSM_VerifySecondStageCodeLoad (
        BHSM_Handle hHsm,
        BHSM_VerifySecondStageCodeLoadIO_t *pConfig
);




#ifdef __cplusplus
}
#endif

#endif /* BHSM_BSECK_H__ */
