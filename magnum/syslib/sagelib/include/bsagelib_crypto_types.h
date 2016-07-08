/***************************************************************************
*     (c)2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile:  $
* $brcm_Revision:  $
* $brcm_Date:  $
*
* Module Description: BSAGE Crypto type definitions.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef BSAGELIB_CRYPTO_TYPES_H__
#define BSAGELIB_CRYPTO_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif



/**
Summary: This enum defines the crypto engines.
**/
typedef enum BSAGElib_Crypto_Engine_e
{
    BSAGElib_Crypto_Engine_eCa,
    BSAGElib_Crypto_Engine_eM2m,
    BSAGElib_Crypto_Engine_eCp,
    BSAGElib_Crypto_Engine_eCaCp,
    BSAGElib_Crypto_Engine_eRmx,
    BSAGElib_Crypto_Engine_eGeneric, /* Specifies that the key slot will be used  */
                                        /* only to pass a drm context to the         */
                                        /* playpump. */
    BSAGElib_Crypto_Engine_eHdcp22,
    BSAGElib_Crypto_Engine_eMax
} BSAGElib_Crypto_Engine_e;

/**
Summary:
This enum defines the supported key entry types.
**/
typedef enum BSAGElib_Crypto_KeyType_e
{
    BSAGElib_Crypto_KeyType_eOdd,      /* ODD key */
    BSAGElib_Crypto_KeyType_eEven,     /* EVEN key */
    BSAGElib_Crypto_KeyType_eClear,    /* CLEAR key */
    BSAGElib_Crypto_KeyType_eIv,       /* Initial Vector (for chips which do not support per-key IVs) */
    BSAGElib_Crypto_KeyType_eOddAndEven,/* For configuring a single algorithm for both ODD and EVEN types at the same time */
    BSAGElib_Crypto_KeyType_eMax
} BSAGElib_Crypto_KeyType_e;


/**
Summary:
This enum defines the supported Customer SubMode.
**/
typedef enum BSAGElib_Crypto_CustomerSubMode_e
{
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_64_4  = 0x0,  /* Key Ladder for Generic CA with 64 bit keys using Key4 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_64_4  = 0x1,  /* Key Ladder for Generic CP with 64 bit keys using Key4 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_64_5  = 0x2,  /* Key Ladder for Generic CA with 64 bit keys using Key5 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_64_5  = 0x3,  /* Key Ladder for Generic CP with 64 bit keys using Key5 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_128_4 = 0x4,  /* Key Ladder for Generic CA with 128 bit keys using Key4 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_128_4 = 0x5,  /* Key Ladder for Generic CP with 128 bit keys using Key4 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_128_5 = 0x6,  /* Key Ladder for Generic CA with 128 bit keys using Key5 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_128_5 = 0x7,  /* Key Ladder for Generic CP with 128 bit keys using Key5 */
    BSAGElib_Crypto_CustomerSubMode_eReserved8        = 0x8,
    BSAGElib_Crypto_CustomerSubMode_eReserved9        = 0x9,
    BSAGElib_Crypto_CustomerSubMode_eReserved10       = 0xA,
    BSAGElib_Crypto_CustomerSubMode_eReserved11       = 0xB,
    BSAGElib_Crypto_CustomerSubMode_eSAGE_128_5       = 0xC,
    BSAGElib_Crypto_CustomerSubMode_eReserved13       = 0xD,
    BSAGElib_Crypto_CustomerSubMode_eGeneralPurpose1  = 0xE,  /* Key Ladder for HDMI, IV and Signed Commands */
    BSAGElib_Crypto_CustomerSubMode_eGeneralPurpose2  = 0xF,  /* Key Ladder for User Hmac */
    BSAGElib_Crypto_CustomerSubMode_eReserved16       = 0x10,
    BSAGElib_Crypto_CustomerSubMode_eReserved17       = 0x11,
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_64_45 = 0x12, /* Key Ladder for Generic CA with 64 bit keys using Key4 and Key5 */
    BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_64_45 = 0x13, /* Key Ladder for Generic CP with 64 bit keys using Key4 and Key5 */
    BSAGElib_Crypto_CustomerSubMode_eReserved20        = 0x14,
    BSAGElib_Crypto_CustomerSubMode_eReserved21        = 0x15,
    BSAGElib_Crypto_CustomerSubMode_eSecureRSA2       = 0x16,
    BSAGElib_Crypto_CustomerSubMode_eETSI_5           = 0x17,
    BSAGElib_Crypto_CustomerSubMode_eDTA_M_CA         = 0x18,
    BSAGElib_Crypto_CustomerSubMode_eDTA_M_CP         = 0x19,
    BSAGElib_Crypto_CustomerSubMode_eDTA_C_CA         = 0x1A,
    BSAGElib_Crypto_CustomerSubMode_eDTA_C_CP         = 0x1B,
    BSAGElib_Crypto_CustomerSubMode_eSCTE52_CA_5      = 0x1C,
    BSAGElib_Crypto_CustomerSubMode_eSAGE_BL_DECRYPT  = 0x1D,  /* Key Ladder for SAGE FSBL decryption and SAGE EJTAG CR*/
    BSAGElib_Crypto_CustomerSubMode_eReserved18       = 0x1E,
    BSAGElib_Crypto_CustomerSubMode_eVISTA_CWC        = 0x1F,

    BSAGElib_Crypto_CustomerSubMode_eMax
}   BSAGElib_Crypto_CustomerSubMode_e;


/**
Summary:
This enum defines the supported Virtual Key Ladder ID for 7420B0.
**/
typedef enum BSAGElib_Crypto_VirtualKeyladderID_e
{
    BSAGElib_Crypto_VirtualKeyladderID_eVKL0,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL1,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL2,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL3,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL4,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL5,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL6,
    BSAGElib_Crypto_VirtualKeyladderID_eVKL7,
    BSAGElib_Crypto_VirtualKeyladderID_eVKLDummy,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved9,
    BSAGElib_Crypto_VirtualKeyladderID_eSWKey,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved11,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved12,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved13,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved14,
    BSAGElib_Crypto_VirtualKeyladderID_eReserved15,
    BSAGElib_Crypto_VirtualKeyladderID_eMax
} BSAGElib_Crypto_VirtualKeyladderID_e;



/**
Summary:
This enum defines crypto operations.
**/
typedef enum BSAGElib_Crypto_Operation_e
{
    BSAGElib_Crypto_Operation_eEncrypt,
    BSAGElib_Crypto_Operation_eDecrypt,
    BSAGElib_Crypto_Operation_ePassThrough,
    BSAGElib_Crypto_Operation_eSign,
    BSAGElib_Crypto_Operation_eVerify,
    BSAGElib_Crypto_Operation_eDecryptHdcp22, /* Special operation used in SAGE_Crypto_UserRsa() for HDCP 2.2. */
    BSAGElib_Crypto_Operation_eMax
}  BSAGElib_Crypto_Operation_e;




/**
Summary: This enum defines the keyladder level
**/
typedef enum BSAGElib_Crypto_KeyLadderLevel_e
{
    /* The code assumes that the keys indexes match the key numbers (eKey3 == 3, eKey4 == 4, ...). */
    /* Do not modify the values.                                                                   */
    BSAGElib_Crypto_KeyLadderLevel_eKey3               = 3, /* key3 */
    BSAGElib_Crypto_KeyLadderLevel_eKey4               = 4, /* key4 */
    BSAGElib_Crypto_KeyLadderLevel_eKey5               = 5, /* key5 */

    BSAGElib_Crypto_KeyLadderLevel_eMax

}BSAGElib_Crypto_KeyLadderLevel_e;


/**
Summary:
This enum defines the supported crypto alogrithms.
**/
typedef enum BSAGElib_Crypto_Algorithm_e
{
    BSAGElib_Crypto_Algorithm_eDvb,    /* DVB will only be used by CA descrambler */
    BSAGElib_Crypto_Algorithm_eDvbCsa2 = BSAGElib_Crypto_Algorithm_eDvb,
    BSAGElib_Crypto_Algorithm_eMulti2,
    BSAGElib_Crypto_Algorithm_eDes,
    BSAGElib_Crypto_Algorithm_e3DesAba,
    BSAGElib_Crypto_Algorithm_e3DesAbc,
    BSAGElib_Crypto_Algorithm_eAes,
    BSAGElib_Crypto_Algorithm_eAes128 = BSAGElib_Crypto_Algorithm_eAes,
    BSAGElib_Crypto_Algorithm_eAes192,
    BSAGElib_Crypto_Algorithm_eTivo,

    /* The 4 algorithms below are only supported for M2M */
    BSAGElib_Crypto_Algorithm_eC2,
    BSAGElib_Crypto_Algorithm_eCss,
    BSAGElib_Crypto_Algorithm_eM6Ke,
    BSAGElib_Crypto_Algorithm_eM6,

    /* added WMDRM_PD */
    BSAGElib_Crypto_Algorithm_eRc4,
    BSAGElib_Crypto_Algorithm_eCbcMac,
    BSAGElib_Crypto_Algorithm_eWMDrmPd,

    BSAGElib_Crypto_Algorithm_eAes128G,
    BSAGElib_Crypto_Algorithm_eHdDVD,
    BSAGElib_Crypto_Algorithm_eBrDVD,

    BSAGElib_Crypto_Algorithm_eDvbCsa3,
    BSAGElib_Crypto_Algorithm_eAsf,
    BSAGElib_Crypto_Algorithm_eAesCounter = BSAGElib_Crypto_Algorithm_eAsf,
    BSAGElib_Crypto_Algorithm_eMSMultiSwapMac,
    BSAGElib_Crypto_Algorithm_eAsa,

    BSAGElib_Crypto_Algorithm_eMax
} BSAGElib_Crypto_Algorithm_e;


/**
Summary:
This enum defines the supported crypto alogrithm variants.
**/
typedef enum BSAGElib_Crypto_AlgorithmVariant_e
{
    BSAGElib_Crypto_AlgorithmVariant_eEcb,
    BSAGElib_Crypto_AlgorithmVariant_eXpt = BSAGElib_Crypto_AlgorithmVariant_eEcb,  /* for BSAGElib_Crypto_Algorithm_eDvb, scramble level */
    BSAGElib_Crypto_AlgorithmVariant_eCbc,
    BSAGElib_Crypto_AlgorithmVariant_ePes = BSAGElib_Crypto_AlgorithmVariant_eCbc,  /* for BSAGElib_Crypto_Algorithm_eDvb, scramble level */
    BSAGElib_Crypto_AlgorithmVariant_eCounter,
    BSAGElib_Crypto_AlgorithmVariant_eRCbc,
    BSAGElib_Crypto_AlgorithmVariant_eMax
} BSAGElib_Crypto_AlgorithmVariant_e;


/**
Summary:
This enum defines the supported termination modes.
For pairs of Enums, first one for ASKM; second for regular
**/
typedef enum BSAGElib_Crypto_TerminationMode_e
{
    BSAGElib_Crypto_TerminationMode_eClear,
    BSAGElib_Crypto_TerminationMode_eCbcResidual,
    BSAGElib_Crypto_TerminationMode_eBlock = BSAGElib_Crypto_TerminationMode_eCbcResidual,
    BSAGElib_Crypto_TerminationMode_eReserved2,
    BSAGElib_Crypto_TerminationMode_eCipherStealing = BSAGElib_Crypto_TerminationMode_eReserved2,
    BSAGElib_Crypto_TerminationMode_eCtsCpcm,
    BSAGElib_Crypto_TerminationMode_eCipherStealingComcast = BSAGElib_Crypto_TerminationMode_eCtsCpcm,
    BSAGElib_Crypto_TerminationMode_eReserved4,
    BSAGElib_Crypto_TerminationMode_eReserved5,
    BSAGElib_Crypto_TerminationMode_eReserved6,
    BSAGElib_Crypto_TerminationMode_eMax
} BSAGElib_Crypto_TerminationMode_e;


/**
Summary:
This enum defines the supported AES counter modes.
**/
typedef enum BSAGElib_Crypto_AesCounterMode_e
{
    BSAGElib_Crypto_AesCounterMode_eGenericFullBlocks,
    BSAGElib_Crypto_AesCounterMode_eGenericAllBlocks,
    BSAGElib_Crypto_AesCounterMode_eIvPlayBackFullBlocks,
    BSAGElib_Crypto_AesCounterMode_ePartialBlockInNextPacket,
    BSAGElib_Crypto_AesCounterMode_eSkipPesHeaderAllBlocks,
    BSAGElib_Crypto_AesCounterMode_eMax
}BSAGElib_Crypto_AesCounterMode_e;


/**
Summary:
This enum defines the supported AES counter sizes.
**/
typedef enum BSAGElib_Crypto_AesCounterSize_e
{
    BSAGElib_Crypto_AesCounterSize_e32,
    BSAGElib_Crypto_AesCounterSize_e64,
    BSAGElib_Crypto_AesCounterSize_e96,
    BSAGElib_Crypto_AesCounterSize_e128,
    BSAGElib_Crypto_AesCounterSize_eMax
}BSAGElib_Crypto_AesCounterSize_e;


typedef enum BSAGElib_Crypto_SolitaryMode_e
{
    BSAGElib_Crypto_SolitaryMode_eClear,
    BSAGElib_Crypto_SolitaryMode_eSa,
    BSAGElib_Crypto_SolitaryMode_eCbcXorIv,
    BSAGElib_Crypto_SolitaryMode_eIv1 = BSAGElib_Crypto_SolitaryMode_eCbcXorIv,
    BSAGElib_Crypto_SolitaryMode_eXorIv,
    BSAGElib_Crypto_SolitaryMode_eReserved4,
    BSAGElib_Crypto_SolitaryMode_eMax

} BSAGElib_Crypto_SolitaryMode_e;


/**
Summary:
This enum defines the supported root key sources.
**/
typedef enum BSAGElib_Crypto_RootKeySrc_e
{
    BSAGElib_Crypto_RootKeySrc_eCuskey,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyA,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyB,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyC,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyD,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyE,
    BSAGElib_Crypto_RootKeySrc_eOtpKeyF,
    BSAGElib_Crypto_RootKeySrc_eReserved0,
    BSAGElib_Crypto_RootKeySrc_eReserved1,
    BSAGElib_Crypto_RootKeySrc_eReserved2,
    BSAGElib_Crypto_RootKeySrc_eClearKey,
    BSAGElib_Crypto_RootKeySrc_eReserved3,
    BSAGElib_Crypto_RootKeySrc_eGlobalKey,

    /* Add new key entry type definition before this line */
    BSAGElib_Crypto_RootKeySrc_eMax
}   BSAGElib_Crypto_RootKeySrc_e;


/**
Summary:
This enum defines the permission flags for a key slot.
**/
typedef enum BSAGElib_Crypto_Region_e
{
    BSAGElib_Crypto_Region_eNone    = 0x0,
    BSAGElib_Crypto_Region_eGLR     = 0x1, /* global memory region     */
    BSAGElib_Crypto_Region_eCRR     = 0x2, /* restricted memory region */
    BSAGElib_Crypto_Region_eXRR     = 0x4, /* restricted memory region */

}   BSAGElib_Crypto_Region_e;


/**
Summary:
This enum defines the SHA variants
**/
typedef enum BSAGElib_Crypto_ShaVariant_e
{
    BSAGElib_Crypto_ShaVariant_eNone,
    BSAGElib_Crypto_ShaVariant_eSha1,
    BSAGElib_Crypto_ShaVariant_eSha256,
    BSAGElib_Crypto_ShaVariant_eSha384,
    BSAGElib_Crypto_ShaVariant_eMax
}  BSAGElib_Crypto_ShaVariant_e;

/**
Summary:
This enum defines the module ID
**/
typedef enum BSAGElib_Crypto_ModuleID_e
{
    BSAGElib_Crypto_ModuleID_eGeneric_CA_64_4    = 0x03,
    BSAGElib_Crypto_ModuleID_eGeneric_CP_64_4    = 0x04,
    BSAGElib_Crypto_ModuleID_eGeneric_CA_64_5    = 0x05,
    BSAGElib_Crypto_ModuleID_eGeneric_CP_64_5    = 0x06,

    BSAGElib_Crypto_ModuleID_eGeneric_CA_128_4   = 0x07,
    BSAGElib_Crypto_ModuleID_eGeneric_CP_128_4   = 0x08,
    BSAGElib_Crypto_ModuleID_eGeneric_CA_128_5   = 0x09,
    BSAGElib_Crypto_ModuleID_eGeneric_CP_128_5   = 0x0a,

    BSAGElib_Crypto_ModuleID_eGeneric_SAGE_128_5 = 0x0d,

    BSAGElib_Crypto_ModuleID_eGeneralPurpose1    = 0x1a

}BSAGElib_Crypto_ModuleID_e;




/**
Summary:
This enum defines the STB Ownwer ID.
**/
typedef enum BSAGElib_Crypto_STBOwnerID_e
{
    BSAGElib_Crypto_STBOwnerID_eOTPVal                 = 0, /* Use OTP value as STB Owner ID */
    BSAGElib_Crypto_STBOwnerID_eOneVal                 = 1, /* Use value 1 as STB Owner ID */
    BSAGElib_Crypto_STBOwnerID_eZeroVal                = 2, /* Use value 0 as STB Owner ID */
    BSAGElib_Crypto_STBOwnerID_eMax,                        /* The max enum is 0x3 */
    BSAGElib_Crypto_STBOwnerID_eMask                   = 3  /* For ASKM, FW assume max enum is 0-3*/

}BSAGElib_Crypto_STBOwnerID_e;


typedef enum BSAGElib_Crypto_Swizzle0aVariant_e
{
    BSAGElib_Crypto_Swizzle0aVariant_eNoMSP = 0,
    BSAGElib_Crypto_Swizzle0aVariant_eMSP0  = 1,
    BSAGElib_Crypto_Swizzle0aVariant_eMSP1  = 2,

    BSAGElib_Crypto_Swizzle0aVariant_eMax
}BSAGElib_Crypto_Swizzle0aVariant_e;


typedef enum BSAGElib_Crypto_GlobalKeyOwnerIDSelect_e
{
    BSAGElib_Crypto_GlobalKeyOwnerIDSelect_eMSP0        = 0,
    BSAGElib_Crypto_GlobalKeyOwnerIDSelect_eMSP1        = 1,
    BSAGElib_Crypto_GlobalKeyOwnerIDSelect_eReserved    = 2,
    BSAGElib_Crypto_GlobalKeyOwnerIDSelect_eUse1        = 3,
    BSAGElib_Crypto_GlobalKeyOwnerIDSelect_eMax
}BSAGElib_Crypto_GlobalKeyOwnerIDSelect_e;



/* select otp key type for the field to be read */
typedef enum BSAGElib_Crypto_OtpKey_e
{
    BSAGElib_Crypto_OtpKey_eA,
    BSAGElib_Crypto_OtpKey_eB,
    BSAGElib_Crypto_OtpKey_eC,
    BSAGElib_Crypto_OtpKey_eD,
    BSAGElib_Crypto_OtpKey_eE,
    BSAGElib_Crypto_OtpKey_eF,

    BSAGElib_Crypto_OtpKey_eMax

} BSAGElib_Crypto_OtpKey_e;



#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSAGELIB_CRYPTO_TYPES_H__ */
