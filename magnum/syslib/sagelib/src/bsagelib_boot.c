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


#include "bstd.h"
#include "bkni.h"

#include "bhsm.h"
#include "bhsm_keyladder.h"
#if (BHSM_API_VERSION==1)
#include "bhsm_bseck.h"
#include "bhsm_verify_reg.h"
#include "bhsm_keyladder_enc.h"
#include "bhsm_otpmsp.h"
#include "bhsm_misc.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bsp_s_hw.h"
#include "bsp_s_download.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"
#include "bsp_s_mem_auth.h"
#else
#include "bhsm_keyslot.h"
#include "bhsm_rv_rsa.h"
#include "bhsm_rv_region.h"
#include "bhsm_otp_msp.h"
#endif

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"

#include "bsagelib.h"
#include "bsagelib_boot.h"
#include "bsagelib_priv.h"
#include "priv/bsagelib_shared_types.h"

BDBG_MODULE(BSAGElib);

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
/* OTP names */
#define OTP_SAGE_DECRYPT_ENABLE     BCMD_Otp_CmdMsp_eReserved210
#define OTP_SAGE_VERIFY_ENABLE      BCMD_Otp_CmdMsp_eReserved209
#define OTP_SAGE_SECURE_ENABLE      BCMD_Otp_CmdMsp_eReserved212
#define OTP_MARKET_ID_0             BCMD_Otp_CmdMsp_eMarketId
#define OTP_MARKET_ID_1             BCMD_Otp_CmdMsp_eMarketId1
#define OTP_SYSTEM_EPOCH_0          BCMD_Otp_CmdMsp_eSystemEpoch
#define OTP_SYSTEM_EPOCH_3          BCMD_Otp_CmdMsp_eSystemEpoch3
#else

#define RV_CONTROLLING_PARAMETERS_REVERSED (1)

#if 1 /*TBD these values should come from a header file */
typedef enum Bsp_Otp_CmdMsp_e
{
    Bsp_Otp_CmdMsp_eSageVerifyEnable                               = 164,
    Bsp_Otp_CmdMsp_eSageFsblDecryptionEnable                       = 165,
    Bsp_Otp_CmdMsp_eSageSecureEnable                               = 166,
    Bsp_Otp_CmdMsp_eMarketId0                                      = 313,
    Bsp_Otp_CmdMsp_eMarketId1                                      = 314,
    Bsp_Otp_CmdMsp_eSystemEpoch0                                   = 326,
    Bsp_Otp_CmdMsp_eSystemEpoch3                                   = 329,
    Bsp_Otp_CmdMsp_LIMIT
} Bsp_Otp_CmdMsp_e ;
#endif

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,1))
/* OTP values are deprecated with BW2.10 and should be assumed 1 for any Zeus5 platform */
#define OTP_SAGE_DECRYPT_ENABLE     Bsp_Otp_CmdMsp_eSageFsblDecryptionEnable
#define OTP_SAGE_VERIFY_ENABLE      Bsp_Otp_CmdMsp_eSageVerifyEnable
#endif

#define OTP_SAGE_SECURE_ENABLE      Bsp_Otp_CmdMsp_eSageSecureEnable
#define OTP_MARKET_ID_0             Bsp_Otp_CmdMsp_eMarketId0
#define OTP_MARKET_ID_1             Bsp_Otp_CmdMsp_eMarketId1
#define OTP_SYSTEM_EPOCH_0          Bsp_Otp_CmdMsp_eSystemEpoch0
#define OTP_SYSTEM_EPOCH_3          Bsp_Otp_CmdMsp_eSystemEpoch3

#endif


/* Host to Sage communication buffers size */
#define SAGE_HOST_BUF_SIZE (32)

#define SAGE_BL_LENGTH (BCHP_SCPU_LOCALRAM_REG_END - BCHP_SCPU_LOCALRAM_REG_START + 4)

/* Types */
#define SAGE_HEADER_TYPE_BL      (0x5357)
#define SAGE_HEADER_TYPE_FRAMEWORK  (0x424C)
#define SAGE_HEADER_TYPE_FRAMEWORK_COMBINED  (0x4349)
#define SAGE_BINARY_TYPE_ZB      (0x3E3F)
#define SAGE_BINARY_TYPE_ZS      (0x0202)
#define SAGE_BINARY_TYPE_CUST1   (0x0303)

#define SAGE_HEADER_SECURE_IMAGE_TRIPLE_SIGNING_SCHEME_VALUE    (0x1)

#define SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_ZERO   (0x0)
#define SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_0xFFEE (0xFFEE)

/* Key0 selection */
#define SAGE_HEADER_KEY0_SELECT_KEY0       (0x43) /* Key0 is used -> 'C' for Customer tool */
#define SAGE_HEADER_KEY0_SELECT_KEY0PRIME  (0x49) /* Key0prime is used -> 'I' for BRCM Internal tool */

/* Signature */
#define _SIG_SZ (256)
#define _SIG_SZ_SAGE_FRAMEWORK (2*_SIG_SZ)

typedef struct {
    BSAGElib_Handle hSAGElib;
    uint32_t otp_sage_decrypt_enable;
    uint32_t otp_sage_verify_enable;
    uint32_t otp_sage_secure_enable;
    uint32_t otp_system_epoch0;
    uint32_t otp_system_epoch3;
    uint32_t otp_market_id;
    uint32_t otp_market_id1;
    uint32_t otp_market_id_lock;
    uint32_t otp_market_id1_lock;

    bool sageBlTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageFrameworkTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageProductionKey;  /* triple-signing's 2ndTierKey can be development key or production key. True for prodction key */
    /* Secure Image version */
    uint8_t sageBlSecureVersion;
    uint8_t sageFrameworkSecureVersion;

} BSAGElib_P_BootContext;

/* TODO: use the BHSM_SecondTierKey_t struct in HSM instead of this one */
#define BSAGELIB_2ND_TIER_RSAKEY_SIZE_BYTES (256)
#define BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES (256)

typedef struct
{
    uint8_t ucKeyData[BSAGELIB_2ND_TIER_RSAKEY_SIZE_BYTES];
    uint8_t ucReserved0;
    uint8_t ucPubExponent;        /* 0 => 3; 1 => 64K+1 */
    uint8_t ucReserved1;
    uint8_t ucRights;             /* 0 => MIPS; 2 => AVD, RAPTOR, RAVE; 4 => BSP; 8 => Boot; 10 => SAGE */
    uint8_t ucMarketID[4];        /* NB: ID and Mask are little endian, so no _Swap required ... */
    uint8_t ucMarketIDMask[4];    /* when comparing to value read from OTP */
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    uint8_t ucEpoch;
    uint8_t ucEpochMask;
    uint8_t ucEpochSelect;
    uint8_t ucMarketIDSelect;
    uint8_t ucReserved3[2];
    uint8_t ucSignatureType;
    uint8_t ucSignatureVersion;
#else
    unsigned char ucReserved1[2];
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
#endif
   uint8_t ucSignature[BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES];
} BCMD_Zeus42_SecondTierKey_t;

typedef struct
{
    uint8_t ucKeyData[BSAGELIB_2ND_TIER_RSAKEY_SIZE_BYTES];
    uint8_t ucEpochSelect;          /* [2:0] */
    uint8_t ucMarketIDSelect;       /* [1:0] */
    uint8_t ucSignatureType;
    uint8_t ucSignatureVersion;
    uint8_t ucReserved_0;
    uint8_t ucChipBindingSelect;    /* [5:0] */
    uint8_t ucReserved_1[2];
    uint8_t ucSigningRights;
    uint8_t ucReserved_2[3];
    uint8_t ucMarketID[4];        /* NB: ID and Mask are little endian, so no _Swap required ... */
    uint8_t ucMarketIDMask[4];    /* when comparing to value read from OTP */
    uint8_t ucEpoch[4];
    uint8_t ucEpochMask[4];
    uint8_t ucUpperChipsetBinding[4];
    uint8_t ucLowerChipsetBinding[4];
    uint8_t ucMetadata1[4];
    uint8_t ucMetadata2[4];
    uint8_t ucSignature[BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES];
}BCMD_Zeus50_SecondTierKey_t;

typedef struct
{
    uint8_t ucReserved0[2];
    uint8_t ucCpuType;
    uint8_t ucNoReloc;
    uint8_t ucMarketId[4];
    uint8_t ucMarketIdMask[4];
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    uint8_t ucReserved1;
    uint8_t ucEpochSelect;
    uint8_t ucEpochMask;
    uint8_t ucEpoch;
    uint8_t ucSignatureVersion;
    uint8_t ucSignatureType;
    uint8_t ucReserved2[2];
#else
    unsigned char ucReserved1[2];
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
#endif
}BSAGElib_Zeus42_ControllingParams;   /* 20 bytes */

typedef struct
{
    uint8_t ucEpochSelect;          /* [2:0] */
    uint8_t ucMarketIDSelect;       /* [1:0] */
    uint8_t ucSignatureType;
    uint8_t ucSignatureVersion;
    uint8_t ucReserved_0;
    uint8_t ucChipBindingSelect;    /* [5:0] */
    uint8_t ucReserved_1;
    uint8_t ucCpuType;
    uint8_t ucReserved_2[4];
    uint8_t ucMarketID[4];        /* NB: ID and Mask are little endian, so no _Swap required ... */
    uint8_t ucMarketIDMask[4];    /* when comparing to value read from OTP */
    uint8_t ucEpoch[4];
    uint8_t ucEpochMask[4];
    uint8_t ucUpperChipsetBinding[4];
    uint8_t ucLowerChipsetBinding[4];
    uint8_t ucMetadata1[4];
    uint8_t ucMetadata2[4];
}BSAGElib_Zeus50_ControllingParams;  /* 44 bytes */


#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0))
typedef BCMD_Zeus50_SecondTierKey_t BCMD_SecondTierKey_t;
typedef BSAGElib_Zeus50_ControllingParams BSAGElib_ControllingParams;
#define SBLSSF_HEADER_RESERVED_1_SIZE (4)
#else
typedef BCMD_Zeus42_SecondTierKey_t BCMD_SecondTierKey_t;
typedef BSAGElib_Zeus42_ControllingParams BSAGElib_ControllingParams;
#define SBLSSF_HEADER_RESERVED_1_SIZE (28)
#endif

typedef struct
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion;            /* version of the secure header structure */
    unsigned char ucImageSigningScheme;        /* indicates single-signing or triple signing scheme */
    unsigned char ucSecurityType;               /* indicates whether the image is signed, encrypted or both */
    unsigned char ucImageType;                  /* Image type: SAGE BL or framework */
    unsigned char ucGlobalOwnerId[2];           /* GlobalOwner ID */
    unsigned char ucSageImageVersion[4];        /* SAGE BL or framework version - for Host-side logs */
    unsigned char ucSageSecurebootToolVersion[4];      /* SAGE secureboot tool version - for Host-side logs */
    unsigned char ucSageImageBinaryType;      /* Only used for Framework: 0:Generic, 1: manufacturing tool, 2: custom, 3: special */
    unsigned char ucEpochVersion;             /* EPOCH version (For bootloader from 0x0 to 0x8, for framework from 0x0 to 0x14) */
    unsigned char ucCaVendorId[4];        /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;    /* STB Owner ID */
    unsigned char ucSwizzle0aVariant;    /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;    /* Swizzle0a Version */
    unsigned char ucCustKeyVar;          /* Cust Key Var */
    unsigned char ucKeyVarHi;            /* Key Var Hi */
    unsigned char ucKeyVarlo;            /* Key Var Low */
    unsigned char ucModuleId;            /* Module ID */
    unsigned char ucKey0Type;            /* Signature Root Key type: key0Prime or Key0 */
    unsigned char ucScmType;            /* SCM type, UNUSED for SAGE BL or framework */
    unsigned char ucScmVersion;         /* SCM version, UNUSED for SAGE BL or framework */
    unsigned char ucProcIn1[16];         /* Proc In 1 */
    unsigned char ucProcIn2[16];         /* Proc In 2 */
    unsigned char ucProcIn3[16];         /* Proc In 3 */
    BCMD_SecondTierKey_t second_tier_key;              /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
    unsigned char ucSize[4];                           /* size of the encrypted data */
    unsigned char ucInstrSectionSize[4];               /* size of the instruction section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucDataSectionSize[4];                /* size of the data section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucReserved[4];                       /* Reserved */
    unsigned char ucSignatureInstrSectionShort[128];   /* first 128 bits of SAGE framework instruction section signature */
    unsigned char ucSignatureDataSectionShort[128];    /* first 128 bits of SAGE framework data section signature */
    unsigned char ucSageVersionString[2048];           /* SAGE Framework version info string */
    unsigned char ucThLShortSig[4];                    /* first 4 bytes of Thin-Layer signature */
    unsigned char ucReserved1[SBLSSF_HEADER_RESERVED_1_SIZE];                     /* Reserved for future usage */
    BSAGElib_ControllingParams ucControllingParameters;/* SAGE Image Header controlling Parameters */
    unsigned char ucHeaderSignature[256];              /* Header signature */
} BSAGElib_SageSecureHeader;

typedef struct /* SAGE 3.2 framework image device info section */
{
    unsigned char ucChipType[4];           /* chip type, as 7425B0,7445D0,etc */
    unsigned char ucChipVariant[2];           /* chip Variant, as ZD,ZB/ZS,etc */
    unsigned char ucReserved[2];           /* reserved */
    unsigned char ucDeviceTreeSize[4];            /* the size of device tree */
} BSAGElib_DeviceInfo;

typedef struct /* SAGE 3.2 framework Header */
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion;            /* version of the secure header structure */
    unsigned char ucImageSigningScheme;        /* indicates single-signing or triple signing scheme */
    unsigned char ucSecurityType;               /* indicates whether the image is signed, encrypted or both */
    unsigned char ucImageType;                  /* Image type: SAGE BL or framework */
    unsigned char ucGlobalOwnerId[2];           /* GlobalOwner ID */
    unsigned char ucSageImageVersion[4];        /* SAGE BL or framework version - for Host-side logs */
    unsigned char ucSageSecurebootToolVersion[4];      /* SAGE secureboot tool version - for Host-side logs */
    unsigned char ucSageImageBinaryType;      /* Only used for Framework: 0:Generic, 1: manufacturing tool, 2: custom, 3: special */
    unsigned char ucEpochVersion;             /* EPOCH version (For bootloader from 0x0 to 0x8, for framework from 0x0 to 0x14) */
    unsigned char ucCaVendorId[4];        /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;    /* STB Owner ID */
    unsigned char ucSwizzle0aVariant;    /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;    /* Swizzle0a Version */
    unsigned char ucCustKeyVar;          /* Cust Key Var */
    unsigned char ucKeyVarHi;            /* Key Var Hi */
    unsigned char ucKeyVarlo;            /* Key Var Low */
    unsigned char ucModuleId;            /* Module ID */
    unsigned char ucKey0Type;            /* Signature Root Key type: key0Prime or Key0 */
    unsigned char ucScmType;            /* SCM type, UNUSED for SAGE BL or framework */
    unsigned char ucScmVersion;         /* SCM version, UNUSED for SAGE BL or framework */
    unsigned char ucProcIn1[16];         /* Proc In 1 */
    unsigned char ucProcIn2[16];         /* Proc In 2 */
    unsigned char ucProcIn3[16];         /* Proc In 3 */
    unsigned char ucSize[4];                           /* size of the encrypted data */
    unsigned char ucInstrSectionSize[4];               /* size of the instruction section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucDataSectionSize[4];                /* size of the data section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucReserved[4];                       /* Reserved */
    unsigned char ucSignatureInstrSectionShort[128];   /* first 128 bits of SAGE framework instruction section signature */
    unsigned char ucSignatureDataSectionShort[128];    /* first 128 bits of SAGE framework data section signature */
    unsigned char ucSageVersionString[2048];           /* SAGE Framework version info string */
    unsigned char ucThLShortSig[4];                    /* first 4 bytes of Thin-Layer signature */
    unsigned char ucReserved1[SBLSSF_HEADER_RESERVED_1_SIZE];                     /* Reserved for future usage */
    BSAGElib_ControllingParams ucControllingParameters;/* SAGE Image Header controlling Parameters */
    unsigned char ucHeaderSignature[256];              /* Header signature */
    BCMD_SecondTierKey_t second_tier_key;              /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
    BSAGElib_DeviceInfo deviceInfo;
} BSAGElib_FrameworkHeader; /* SAGE 3.2 framework Header */

typedef struct /* SAGE 3.2 framework Bottom, after device tree section, it's chip specific */
{
    BSAGElib_ControllingParams ucDevicetreeControllingParameters;/* DeviceTree controlling Parameters */
    uint8_t     ucDeviceTreeSignature[256];        /* DeviceTree signature */
    BSAGElib_ControllingParams ucBinaryControllingParameters;/* binary(instruction+RO data, RW data controlling Parameters */
    uint8_t     ucInstructionSignature[256];        /* Binary instruction + RO data signature */
    uint8_t     ucDataSignature[256];        /* RW data signature */
    BSAGElib_ControllingParams ucFullImageControllingParameters;/* full Image controlling Parameters */
    uint8_t     ucFullImageSignature[256];        /* recontructed framework image signature
                                              recontructed framework image: BSAGElib_FrameworkHeader + DeviceTree + ucControllingParameters + ucDeviceTreeSignature + framework Binary (Data+code)*/
} BSAGElib_DeviceBottom; /* SAGE 3.2 framework Bottom, after device tree section, it's chip specific */

typedef struct /* SAGE 3.2 framework device section, after header, and before binary, it's chip specific */
{
    BSAGElib_FrameworkHeader *pHeader;
    uint8_t     *pDeviceTree;
    BSAGElib_DeviceBottom *pBottom;
} BSAGElib_DeviceSection;

typedef struct
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion[2];            /* version of the secure header structure */
    unsigned char ucCombinedImageSize[4];        /* Size of total image */
    unsigned char ucSectionNumber[4];            /* number of sections in the blob */
} BSAGElib_CombineImageHeader; /* SAGE 3.2 framework combine header, it's before framework header */

typedef struct /* SAGE 3.2 bootloader header */
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion;            /* version of the secure header structure */
    unsigned char ucImageSigningScheme;        /* indicates single-signing or triple signing scheme */
    unsigned char ucSecurityType;               /* indicates whether the image is signed, encrypted or both */
    unsigned char ucImageType;                  /* Image type: SAGE BL or framework */
    unsigned char ucGlobalOwnerId[2];           /* GlobalOwner ID */
    unsigned char ucSageImageVersion[4];        /* SAGE BL or framework version - for Host-side logs */
    unsigned char ucSageSecurebootToolVersion[4];      /* SAGE secureboot tool version - for Host-side logs */
    unsigned char ucSageImageBinaryType;      /* Only used for Framework: 0:Generic, 1: manufacturing tool, 2: custom, 3: special */
    unsigned char ucEpochVersion;             /* EPOCH version (For bootloader from 0x0 to 0x8, for framework from 0x0 to 0x14) */
    unsigned char ucCaVendorId[4];        /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;    /* STB Owner ID */
    unsigned char ucSwizzle0aVariant;    /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;    /* Swizzle0a Version */
    unsigned char ucCustKeyVar;          /* Cust Key Var */
    unsigned char ucKeyVarHi;            /* Key Var Hi */
    unsigned char ucKeyVarlo;            /* Key Var Low */
    unsigned char ucModuleId;            /* Module ID */
    unsigned char ucKey0Type;            /* Signature Root Key type: key0Prime or Key0 */
    unsigned char ucScmType;            /* SCM type, UNUSED for SAGE BL or framework */
    unsigned char ucScmVersion;         /* SCM version, UNUSED for SAGE BL or framework */
    unsigned char ucProcIn1[16];         /* Proc In 1 */
    unsigned char ucProcIn2[16];         /* Proc In 2 */
    unsigned char ucProcIn3[16];         /* Proc In 3 */
    unsigned char ucSize[4];                           /* size of the encrypted data */
    unsigned char ucInstrSectionSize[4];               /* size of the instruction section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucDataSectionSize[4];                /* size of the data section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucReserved[4];                       /* Reserved */
    unsigned char ucSignatureInstrSectionShort[128];   /* first 128 bits of SAGE framework instruction section signature */
    unsigned char ucSignatureDataSectionShort[128];    /* first 128 bits of SAGE framework data section signature */
    unsigned char ucSageVersionString[2048];           /* SAGE Framework version info string */
    unsigned char ucThLShortSig[4];                    /* first 4 bytes of Thin-Layer signature */
    unsigned char ucReserved1[SBLSSF_HEADER_RESERVED_1_SIZE];                     /* Reserved for future usage */
    BSAGElib_ControllingParams ucControllingParameters;/* SAGE Image Header controlling Parameters */
    unsigned char ucHeaderSignature[256];              /* Header signature */
    BCMD_SecondTierKey_t second_tier_key;              /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
} BSAGElib_BootloaderHeader;/* SAGE 3.2 bootloader header */

typedef struct
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion;            /* version of the secure header structure */
    unsigned char ucImageSigningScheme;        /* indicates single-signing or triple signing scheme */
    unsigned char ucSecurityType;               /* indicates whether the image is signed, encrypted or both */
    unsigned char ucImageType;                  /* Image type: SAGE BL or framework */
    unsigned char ucGlobalOwnerId[2];           /* GlobalOwner ID */
    unsigned char ucSageImageVersion[4];        /* SAGE BL or framework version - for Host-side logs */
    unsigned char ucSageSecurebootToolVersion[4];      /* SAGE secureboot tool version - for Host-side logs */
    unsigned char ucSageImageBinaryType;      /* Only used for Framework: 0:Generic, 1: manufacturing tool, 2: custom, 3: special */
    unsigned char ucEpochVersion;             /* EPOCH version (For bootloader from 0x0 to 0x8, for framework from 0x0 to 0x14) */
    unsigned char ucCaVendorId[4];        /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;    /* STB Owner ID */
    unsigned char ucSwizzle0aVariant;    /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;    /* Swizzle0a Version */
    unsigned char ucCustKeyVar;          /* Cust Key Var */
    unsigned char ucKeyVarHi;            /* Key Var Hi */
    unsigned char ucKeyVarlo;            /* Key Var Low */
    unsigned char ucModuleId;            /* Module ID */
    unsigned char ucKey0Type;            /* Signature Root Key type: key0Prime or Key0 */
    unsigned char ucScmType;            /* SCM type, UNUSED for SAGE BL or framework */
    unsigned char ucScmVersion;         /* SCM version, UNUSED for SAGE BL or framework */
    unsigned char ucProcIn1[16];         /* Proc In 1 */
    unsigned char ucProcIn2[16];         /* Proc In 2 */
    unsigned char ucProcIn3[16];         /* Proc In 3 */
    unsigned char ucSize[4];                           /* size of the encrypted data */
    unsigned char ucInstrSectionSize[4];               /* size of the instruction section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucDataSectionSize[4];                /* size of the data section of the SAGE SW, UNUSED for SAGE BL */
    unsigned char ucReserved[4];                       /* Reserved */
    unsigned char ucSignatureInstrSectionShort[128];   /* first 128 bits of SAGE framework instruction section signature */
    unsigned char ucSignatureDataSectionShort[128];    /* first 128 bits of SAGE framework data section signature */
    unsigned char ucSageVersionString[2048];           /* SAGE Framework version info string */
    unsigned char ucThLShortSig[4];                    /* first 4 bytes of Thin-Layer signature */
    unsigned char ucReserved1[SBLSSF_HEADER_RESERVED_1_SIZE];                     /* Reserved for future usage */
    BSAGElib_ControllingParams ucControllingParameters;/* SAGE Image Header controlling Parameters */
    unsigned char ucHeaderSignature[256];              /* Header signature */
    unsigned char ucHeaderSignatureP[256];              /* Header signature */
    BCMD_SecondTierKey_t second_tier_key[3];            /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
} BSAGElib_SageSecureHeaderTripleSign;

typedef struct /* this structure has the common parts to both previous structures */
{
    unsigned char ucHeaderIndex[2];             /* header magic number */
    unsigned char ucHeaderVersion;            /* version of the secure header structure */
    unsigned char ucImageSigningScheme;        /* indicates single-signing or triple signing scheme */
    unsigned char ucSecurityType;               /* indicates whether the image is signed, encrypted or both */
    unsigned char ucImageType;                  /* Image type: SAGE BL or framework */
    unsigned char ucGlobalOwnerId[2];           /* GlobalOwner ID */
    unsigned char ucSageImageVersion[4];        /* SAGE BL or framework version - for Host-side logs */
    unsigned char ucSageSecurebootToolVersion[4];      /* SAGE secureboot tool version - for Host-side logs */
    unsigned char ucSageImageBinaryType;      /* Only used for Framework: 0:Generic, 1: manufacturing tool, 2: custom, 3: special */
    unsigned char ucEpochVersion;             /* EPOCH version (For bootloader from 0x0 to 0x8, for framework from 0x0 to 0x14) */
    unsigned char ucCaVendorId[4];        /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;    /* STB Owner ID */
    unsigned char ucSwizzle0aVariant;    /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;    /* Swizzle0a Version */
    unsigned char ucCustKeyVar;          /* Cust Key Var */
    unsigned char ucKeyVarHi;            /* Key Var Hi */
    unsigned char ucKeyVarlo;            /* Key Var Low */
    unsigned char ucModuleId;            /* Module ID */
    unsigned char ucKey0Type;            /* Signature Root Key type: key0Prime or Key0 */
    unsigned char ucScmType;            /* SCM type, UNUSED for SAGE BL or framework */
    unsigned char ucScmVersion;         /* SCM version, UNUSED for SAGE BL or framework */
    unsigned char ucProcIn1[16];         /* Proc In 1 */
    unsigned char ucProcIn2[16];         /* Proc In 2 */
    unsigned char ucProcIn3[16];         /* Proc In 3 */
    BCMD_SecondTierKey_t second_tier_key;              /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
} BSAGElib_SageSecureHeaderCommon;

typedef struct {
    const char *name;         /* printable name */
    BSAGElib_SageSecureHeaderCommon *header;
    uint8_t *signature;
    uint8_t *data;
    uint32_t data_len;
    uint8_t *controlling_parameters;
} BSAGElib_SageImageHolder;

/* build code for writing all _SageGlobalSram_e* register value
 * used in BSAGElib_P_SetBootParams() and BSAGElib_Sage_P_CleanBootVars() */
#if 1
#define BootParamDbgPrintf(format) BDBG_MSG(format)
#else
#define BootParamDbgPrintf(format)
#endif

/* TODO: stays as is for the time being */
#define _BSAGElib_P_Boot_SetBootParam(REGID, VAL) {      \
        uint32_t addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_e##REGID); \
        BREG_Write32(hSAGElib->core_handles.hReg, addr, VAL);                   \
        BootParamDbgPrintf(("%s - Read %s - Addr: 0x%08x Value: 0x%08x", BSTD_FUNCTION, #REGID, addr, BREG_Read32(hSAGElib->core_handles.hReg, (uint32_t)addr))); }
#define _BSAGElib_P_Boot_GetBootParam(REGID, VAR) {      \
        uint32_t addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_e##REGID); \
        VAR=BREG_Read32(hSAGElib->core_handles.hReg, addr); }


#define _Swap16(PVAL) (((PVAL)[0] << 8) | ((PVAL)[1]))
#define _ToDWORD(PVAL) (((PVAL)[3] << 24) | ((PVAL)[2] << 16) | ((PVAL)[1] << 8) |((PVAL)[0] << 0))


/****************************************
 * Local Functions
 ***************************************/
static BERR_Code BSAGElib_P_Boot_GetSageOtpMspParams(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGElib_P_Boot_CheckCompatibility(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);
/*
static BERR_Code BSAGElib_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx);
*/
static BERR_Code BSAGElib_P_Boot_ParseSageImage(BSAGElib_Handle hSAGElib, BSAGElib_P_BootContext *ctx, uint8_t *pBinary, uint32_t binarySize, BSAGElib_SageImageHolder* holder);
static BERR_Code BSAGElib_P_Boot_SetBootParams(BSAGElib_P_BootContext *ctx, BSAGElib_BootSettings *pBootSettings, BSAGElib_SageImageHolder* frameworkHolder);
static BERR_Code BSAGElib_P_Boot_ResetSage(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *bl_img);
static BERR_Code BSAGElib_P_Boot_CheckFrameworkBFWVersion(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);

static BCMD_SecondTierKey_t * BSAGElib_P_Boot_GetKey(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder* image);
static uint8_t *BSAGElib_P_Boot_GetSignature(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *image);
static void BSAGElib_P_Boot_SetImageInfo(BSAGElib_ImageInfo *pImageInfo, BSAGElib_SageImageHolder* holder, uint32_t header_version, uint32_t type, bool triple_sign);

/* in bsagelib_boot_4x.c */
extern BERR_Code BSAGElib_P_Boot_SUIF(BSAGElib_Handle hSAGElib,BSAGElib_BootSettings *pBootSettings);

static BERR_Code BSAGElib_P_Boot_CheckFrameworkKeys(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder* frameworkHolder)
{
    BERR_Code rc = BERR_SUCCESS;

    if(ctx->sageFrameworkTripleSigning !=0) {
        BSAGElib_SageSecureHeaderTripleSign *pTripHeader = (BSAGElib_SageSecureHeaderTripleSign *)frameworkHolder->header;
        BCMD_SecondTierKey_t *pSecondTierKey;
        uint32_t market_id, market_id_mask, otp_market_id, i;

        pSecondTierKey = &(pTripHeader->second_tier_key[0]);

        for(i=0; i<3; i++, pSecondTierKey++) {
            otp_market_id = (pSecondTierKey->ucMarketIDSelect) ? ctx->otp_market_id1 : ctx->otp_market_id;
            market_id = _ToDWORD(pSecondTierKey->ucMarketID);
            market_id_mask = _ToDWORD(pSecondTierKey->ucMarketIDMask);
            BDBG_MSG(("%s comparing key %u ID=0x%08X, mask=0x%08X to OTP=%08X", BSTD_FUNCTION, i, market_id, market_id_mask, otp_market_id));
            /* check SAGE BL b/c */
            if( ((market_id&market_id_mask)==(otp_market_id&market_id_mask)) && (market_id!=otp_market_id)) {
                BDBG_WRN(("%s- Segmentation using marketId=0x%08X, mask=0x%X may not be supported", BSTD_FUNCTION, market_id, market_id_mask));
                break;
            }
        }
    }
    return rc;
}

static BERR_Code BSAGElib_P_Boot_CheckSRR(
        BSAGElib_Handle hSAGElib,
        const BSAGElib_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t srr_offset, srr_size, srr_offset_req, srr_size_req, r_offset, r_size;

    /* fetch current SRR start and end values which are set by BFW */
    _BSAGElib_P_Boot_GetBootParam(SRRStartOffset, r_offset);
    _BSAGElib_P_Boot_GetBootParam(SRREndOffset, r_size);

    if( (r_offset!=0) || (r_size!=0) ){
        /* BFW has already set SRR, which must match current request */
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
        srr_offset_req = pBootSettings->SRROffset;
        srr_size_req = pBootSettings->SRRSize;
#else
        uint32_t i;
        srr_offset_req = srr_size_req = -1;
        for(i=0; i< pBootSettings->regionMapNum; i++) {
            if(pBootSettings->pRegionMap[i].id == BSAGElib_RegionId_Srr){
                srr_offset_req = pBootSettings->pRegionMap[i].offset;
                srr_size_req = pBootSettings->pRegionMap[i].size;
            }
        }
#endif
        /* convert to physical offset and  size */
        srr_offset = RESTRICTED_REGION_OFFSET(r_offset);
        srr_size   = RESTRICTED_REGION_SIZE(r_offset, r_size);

        BDBG_MSG(("%s - Current SRR offset=0x%08X, size=%u. SAGE heap offset=0x%08X, size=%u",
                BSTD_FUNCTION, srr_offset, srr_size, srr_offset_req, srr_size_req));

        if( (srr_offset!= srr_offset_req) || (srr_size != srr_size_req) ) {
            BDBG_ERR(("%s - Current SRR offset=0x%08X, size=%u does not match requested SAGE heap offset=0x%08X, size=%u",
                    BSTD_FUNCTION, srr_offset, srr_size, srr_offset_req, srr_size_req));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
    }
end:

    return rc;
}

static BERR_Code BSAGElib_P_Boot_CheckMarketId(
    BSAGElib_P_BootContext *ctx,
    BCMD_SecondTierKey_t *pKey)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t otp_market_id, key_market_id,key_market_id_mask;

    if(pKey->ucMarketIDSelect == 0)
    {
        otp_market_id = ctx->otp_market_id;
    }else if(pKey->ucMarketIDSelect == 1)
    {
        otp_market_id = ctx->otp_market_id1;
    }else{
        BDBG_WRN(("%s - Key's ucMarketIDSelect %d is wrong.", BSTD_FUNCTION,pKey->ucMarketIDSelect));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    key_market_id = _ToDWORD(pKey->ucMarketID);
    key_market_id_mask = _ToDWORD(pKey->ucMarketIDMask);

    BDBG_MSG(("%s - marketID key-%x otp-%x (key marketID select %d, mask %x, otp_marketID 0-%x, 1-%x", BSTD_FUNCTION,key_market_id,otp_market_id,pKey->ucMarketIDSelect,key_market_id_mask,ctx->otp_market_id,ctx->otp_market_id1));

    if((otp_market_id&key_market_id_mask) != (key_market_id&key_market_id_mask))
    {
        BDBG_WRN(("%s: Key market id %08x does not match chip OTP",
                  BSTD_FUNCTION, key_market_id));
        BDBG_WRN(("%s: (MktIdSel %d, mask %08x, OTP MktId %08x, OTP MktId1 %08x)",
                  BSTD_FUNCTION, pKey->ucMarketIDSelect,
                  key_market_id_mask, ctx->otp_market_id, ctx->otp_market_id1));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
end:
    return rc;
}

static BCMD_SecondTierKey_t *
BSAGElib_P_Boot_GetKey(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    BCMD_SecondTierKey_t *ret = NULL;

    if (/*ctx->sageFrameworkTripleSigning ||*/ ctx->sageBlTripleSigning) {
        BSAGElib_SageSecureHeaderTripleSign *triple_sign_header =
            (BSAGElib_SageSecureHeaderTripleSign *)image->header;

        BDBG_MSG(("%s %d keys offset 0x%x 0x%x 0x%x",BSTD_FUNCTION,__LINE__,
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key[0]) - (uint8_t *)triple_sign_header),
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key[1]) - (uint8_t *)triple_sign_header),
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key[2]) - (uint8_t *)triple_sign_header)));

        ctx->sageProductionKey = false;

        if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key[0]) == BERR_SUCCESS)
            ret = &triple_sign_header->second_tier_key[0];
        else if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key[1]) == BERR_SUCCESS)
            ret = &triple_sign_header->second_tier_key[1];
        else if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key[2]) == BERR_SUCCESS)
        {
            ctx->sageProductionKey = true;
            ret = &triple_sign_header->second_tier_key[2];
        }
        else {
            BDBG_ERR(("%s - Image has no valid second tier key for chip's MarketID1", BSTD_FUNCTION));
        }
    }
    else if(image->header->ucHeaderVersion >= 0x0a && image->header->ucHeaderIndex[0] == 0x53 && image->header->ucHeaderIndex[1] == 0x57)
    {    /* it's SAGE 3.2 bootloader */
        BSAGElib_BootloaderHeader *pBootLoader = (BSAGElib_BootloaderHeader *)image->header;
        if (BSAGElib_P_Boot_CheckMarketId(ctx,&pBootLoader->second_tier_key) == BERR_SUCCESS) {
            ret = &pBootLoader->second_tier_key;
        } else {
            BDBG_ERR(("%s - Image has no valid second tier key for chip's MarketID1", BSTD_FUNCTION));
        }
    }
    else {
        if (BSAGElib_P_Boot_CheckMarketId(ctx,&image->header->second_tier_key) == BERR_SUCCESS) {
            ret = &image->header->second_tier_key;
        } else {
            BDBG_ERR(("%s - Image has no valid second tier key for chip's MarketID1", BSTD_FUNCTION));
        }
    }

    return ret;
}

static uint8_t *
BSAGElib_P_Boot_GetSignature(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    uint8_t *signature = NULL;
    signature = image->signature;

    if(ctx->sageProductionKey) /* for triple signed SBL, 'signature' point to development Key signature, */
        signature += _SIG_SZ;  /* production key signature is right after */

    return signature;
}

static uint32_t
BSAGElib_P_Boot_GetHeaderSize(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    uint32_t index = _Swap16(image->header->ucHeaderIndex);
    bool triple = false;
    uint32_t header_size = 0;

    if (index == SAGE_HEADER_TYPE_BL) {
        if (ctx->sageBlTripleSigning) {
            triple = true;
        }
    }
    else {
        if (ctx->sageFrameworkTripleSigning) {
            triple = true;
        }
    }

    if (triple) {
        header_size = sizeof(BSAGElib_SageSecureHeaderTripleSign);
    }
    else {
        header_size = sizeof(BSAGElib_SageSecureHeader);
    }
    return header_size;
}

/* Check SAGE Secureboot OTP MSPs */
static BERR_Code
BSAGElib_P_Boot_GetSageOtpMspParams(
    BSAGElib_P_BootContext *ctx)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,1))
    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_SAGE_DECRYPT_ENABLE, &ctx->otp_sage_decrypt_enable, NULL, "decrypt_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_SAGE_VERIFY_ENABLE, &ctx->otp_sage_verify_enable, NULL, "verify_enable");
    if (rc != BERR_SUCCESS) { goto end; }
#else
    ctx->otp_sage_decrypt_enable=1;
    ctx->otp_sage_verify_enable=1;
#endif

    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_SAGE_SECURE_ENABLE, &ctx->otp_sage_secure_enable, NULL, "secure_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_MARKET_ID_0, &ctx->otp_market_id, &ctx->otp_market_id_lock, "market id0");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_MARKET_ID_1, &ctx->otp_market_id1, &ctx->otp_market_id1_lock, "market id1");
    if (rc != BERR_SUCCESS) { goto end; }

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,1))
    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eSystemEpoch, &ctx->otp_system_epoch0, NULL, "system epoch 0");
#else
    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_SYSTEM_EPOCH_0, &ctx->otp_system_epoch0, NULL, "epoch");
    if (rc != BERR_SUCCESS) { goto end; }
    rc = BSAGElib_P_GetOtp(hSAGElib, OTP_SYSTEM_EPOCH_3, &ctx->otp_system_epoch3, NULL, "system epoch 3");
#endif
    if (rc != BERR_SUCCESS) { goto end; }

    BDBG_MSG(("%s - OTP [SAGE SECURE ENABLE: %d, SAGE DECRYPT_ENABLE: %d, SAGE VERIFY_ENABLE: %d]",
              BSTD_FUNCTION, ctx->otp_sage_secure_enable,
              ctx->otp_sage_decrypt_enable,
              ctx->otp_sage_verify_enable));

    BDBG_MSG(("%s - OTP [MARKET ID: 0x%08x, SYSTEM EPOCH 0: %d, SYSTEM EPOCH 3: %d]",
                  BSTD_FUNCTION, ctx->otp_market_id,
                  ctx->otp_system_epoch0, ctx->otp_system_epoch3));

    if((ctx->otp_sage_secure_enable == 0) ||
       ((ctx->otp_sage_decrypt_enable == 0) &&
       (ctx->otp_sage_verify_enable == 0)))
    {
        BDBG_ERR(("*******  SAGE ERROR  >>>>>"));
        BDBG_ERR(("* SAGE secureboot NOT enabled."));
        BDBG_ERR(("* SAGE secureboot OTP MSPs to program."));
        BDBG_ERR(("*******  SAGE ERROR  <<<<<"));
        rc = BERR_INVALID_PARAMETER;
    }

    if((hSAGElib->chipInfo.chipType != BSAGElib_ChipType_eZS) &&
       ((ctx->otp_market_id1_lock >> 16) != 0xFFFF)) {
        BDBG_WRN(("    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
        BDBG_WRN(("    !!!                                                !!!"));
        BDBG_WRN(("    !!! MarketID1[bits 31:16] have not been programmed !!!"));
        BDBG_MSG(("    !!!        (unprogrammed=0x%04Xxxxx)               !!!",
                  ctx->otp_market_id1_lock >> 16));
        BDBG_WRN(("    !!!    Run 'program_production_marketid1' app      !!!"));
        BDBG_WRN(("    !!!      to properly program MarketID1             !!!"));
        BDBG_WRN(("    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    }
end:
    return rc;
}

/*
static BERR_Code
BSAGElib_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx)
{
    BERR_Code rc = BERR_SUCCESS;

    if(ctx->sageBlTripleSigning != ctx->sageFrameworkTripleSigning) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s - Both SAGE images must be in triple signing mode", BSTD_FUNCTION));
        goto end;
    }

end:
    return rc;
}
*/

#define SAGE_FRAMEWORK_VERSION_CHECK 3
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
#define BFW_VERSION_CHECK_MAJOR 4
#define BFW_VERSION_CHECK_MINOR 1
#define BFW_VERSION_CHECK_SUBMINOR 3
#else
#define BFW_VERSION_CHECK_MAJOR 2
#define BFW_VERSION_CHECK_MINOR 0
#define BFW_VERSION_CHECK_SUBMINOR 2
#endif
static BERR_Code
BSAGElib_P_Boot_CheckFrameworkBFWVersion(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SageImageHolder *image)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)image->header;

    BDBG_MSG(("SAGE Framework version major %d",header->ucSageImageVersion[0]));
    if (header->ucSageImageVersion[0] >= SAGE_FRAMEWORK_VERSION_CHECK)
    {
#if (BHSM_API_VERSION==1)
        BHSM_Capabilities_t hsmCaps;
        if( ( rc = BHSM_GetCapabilities(hSAGElib->core_handles.hHsm, &hsmCaps ) ) != BERR_SUCCESS )
        {
            BDBG_ERR(("couldn't read BFW version"));
            return rc;
        }
        else
        {
            BDBG_MSG(("BFW %d.%d.%d",hsmCaps.version.firmware.bseck.major, hsmCaps.version.firmware.bseck.minor, hsmCaps.version.firmware.bseck.subMinor));
            if ( BFW_VERSION_CHECK_MAJOR < hsmCaps.version.firmware.bseck.major )
            {
                return rc;
            }
            else if ( BFW_VERSION_CHECK_MAJOR == hsmCaps.version.firmware.bseck.major )
            {
                if ( BFW_VERSION_CHECK_MINOR < hsmCaps.version.firmware.bseck.minor )
                {
                    return rc;
                }
                else if ( BFW_VERSION_CHECK_MINOR == hsmCaps.version.firmware.bseck.minor )
                {
                    if ( BFW_VERSION_CHECK_SUBMINOR <= hsmCaps.version.firmware.bseck.subMinor )
#else
        BHSM_ModuleCapabilities hsmCaps;
        BKNI_Memset( &hsmCaps, 0, sizeof(hsmCaps) );
        BHSM_GetCapabilities(hSAGElib->core_handles.hHsm, &hsmCaps );
        {
            BDBG_MSG(("BFW %d.%d.%d",hsmCaps.version.bfw.major, hsmCaps.version.bfw.minor, hsmCaps.version.bfw.subminor));
            if ( BFW_VERSION_CHECK_MAJOR < hsmCaps.version.bfw.major )
            {
                return BERR_SUCCESS;
            }
            else if ( BFW_VERSION_CHECK_MAJOR == hsmCaps.version.bfw.major )
            {
                if ( BFW_VERSION_CHECK_MINOR < hsmCaps.version.bfw.minor )
                {
                    return BERR_SUCCESS;
                }
                else if ( BFW_VERSION_CHECK_MINOR == hsmCaps.version.bfw.minor )
                {
                    if ( BFW_VERSION_CHECK_SUBMINOR <= hsmCaps.version.bfw.subminor )
#endif
                    {
                        return rc;
                    }
                    else
                    {
                        return BERR_NOT_SUPPORTED;
                    }
                }
            }
            return BERR_NOT_SUPPORTED;
        }
    }
    return rc;
}

static BERR_Code
BSAGElib_P_Boot_CheckCompatibility(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SageImageHolder *img)
{
    BERR_Code rc = BERR_SUCCESS;

    switch(hSAGElib->chipInfo.chipType)
    {
        case BSAGElib_ChipType_eZS:
            if(_Swap16(img->header->ucGlobalOwnerId) != SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x).", _Swap16(img->header->ucGlobalOwnerId)));
                BDBG_ERR(("* Chipset Type is ZS!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eZB:
            /* return error only if chip is BRCM development part (ZS) */
            if(_Swap16(img->header->ucGlobalOwnerId) == SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x) .", _Swap16(img->header->ucGlobalOwnerId)));
                BDBG_ERR(("* Chipset Type is ZB!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eCustomer1:
        case BSAGElib_ChipType_eCustomer:
            /* return error only if chip is BRCM development part (ZS) */
            if(_Swap16(img->header->ucGlobalOwnerId) == SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x).", _Swap16(img->header->ucGlobalOwnerId)));
                BDBG_ERR(("* Chipset Type is ZB or ZS."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;
        default:
            BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
            BDBG_ERR(("* Invalid chipset type."));
            BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

            rc = BERR_INVALID_PARAMETER;
            goto end;
    }

end:
    return rc;
}

/* Reference all needed areas (secure header, data, signature) of the SAGE
 * binary (bootloader or framework) located in memory into a
 * BSAGElib_SageImageHolder structure.
Raw file in memory:
    +----------+---------------+-------------------------------------------------
    |  header  |   signature   |    binary data (framework, bootloader)      . . .
    +----------+---------------+-------------------------------------------------
*/

static void
BSAGElib_P_Boot_SetImageInfo(
    BSAGElib_ImageInfo *pImageInfo,
    BSAGElib_SageImageHolder* holder,
    uint32_t header_version,
    uint32_t type,
    bool triple_sign)
{
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)holder->header;
    const char *typeStr = NULL;
    const char *subTypeStr = NULL;
    char *verStr;
    uint32_t version;

    BKNI_Memcpy(pImageInfo->version, header->ucSageImageVersion, 4);
    BKNI_Memcpy(pImageInfo->signingToolVersion, header->ucSageSecurebootToolVersion, 4);

    verStr = pImageInfo->versionString;
    if (type == SAGE_HEADER_TYPE_BL) {
        typeStr = "Bootloader";
        BDBG_MSG(("%s - '%s' Detected [type=%s, header_version=%u, triple_sign=%u]",
                  BSTD_FUNCTION, holder->name, typeStr, header_version, triple_sign));
        pImageInfo->THLShortSig = 0;
    }
    else {
        typeStr = "Framework ";
        if (triple_sign) {
            BSAGElib_SageSecureHeaderTripleSign *pHeaderTriple = (BSAGElib_SageSecureHeaderTripleSign *)header;
            pImageInfo->THLShortSig = pHeaderTriple->ucThLShortSig[0] | (pHeaderTriple->ucThLShortSig[1] << 8) |
                                      (pHeaderTriple->ucThLShortSig[2] << 16) | (pHeaderTriple->ucThLShortSig[3] << 24);
        }
        else {
            BSAGElib_SageSecureHeader *pHeaderSingle = (BSAGElib_SageSecureHeader *)header;
            BSAGElib_FrameworkHeader  *pSage32Header = (BSAGElib_FrameworkHeader  *)header;
            uint32_t header_version;
            header_version = header->ucHeaderVersion;
            if(header_version >= 0x0a)
            { /* SAGE 3.2 framework */
                pImageInfo->THLShortSig = pSage32Header->ucThLShortSig[0] | (pSage32Header->ucThLShortSig[1] << 8) |
                                          (pSage32Header->ucThLShortSig[2] << 16) | (pSage32Header->ucThLShortSig[3] << 24);
            }else{
              /* SAGE 3.1 framework */
                pImageInfo->THLShortSig = pHeaderSingle->ucThLShortSig[0] | (pHeaderSingle->ucThLShortSig[1] << 8) |
                                          (pHeaderSingle->ucThLShortSig[2] << 16) | (pHeaderSingle->ucThLShortSig[3] << 24);
            }
        }
        BDBG_MSG(("%s - '%s' Detected [type=%s, header_version=%u, triple_sign=%u, THL Short Sig=0x%08x]",
                  BSTD_FUNCTION, holder->name, typeStr, header_version, triple_sign, pImageInfo->THLShortSig));
        {
            char sage_ver_info_str[] = "\nSAGE_VER_INFO";
            if (BKNI_Memcmp(sage_ver_info_str, header->ucSageVersionString, sizeof(sage_ver_info_str)-1) == 0) {
                BDBG_MSG(("SAGE Version String: %s", header->ucSageVersionString));
            }
        }
    }

    switch (header->ucSageImageBinaryType) {
    case 0:
        subTypeStr = "RELEASE";
        break;
    case 1:
        subTypeStr = "MANUFACTURING TOOL";
        break;
    case 2:
        subTypeStr = "CUSTOM";
        break;
    case 3:
        subTypeStr = "DevOnly";
        break;
    default:
        subTypeStr = "UNKNOWN";
        break;
    }

    version = pImageInfo->version[0] | (pImageInfo->version[1] << 8) |
             (pImageInfo->version[2] << 16) | (pImageInfo->version[3] << 24);
    if (version == 0) {
        BKNI_Snprintf(verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING,
                      "SAGE %s does not contain SAGE versioning information", typeStr);
        BDBG_WRN(("%s", verStr));
    }
    else {
        char revstr[9];
        if ((pImageInfo->version[2] == 0) && (pImageInfo->version[3] != 0)) {
            BKNI_Snprintf(revstr, sizeof(revstr)-1, "Alpha%u", pImageInfo->version[3]);
        } else {
            BKNI_Snprintf(revstr, sizeof(revstr)-1, "%u.%u",
                          pImageInfo->version[2], pImageInfo->version[3]);
        }
        BKNI_Snprintf (verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING,
                       "SAGE %s Version [%s=%u.%u.%s, Signing Tool=%u.%u.%u.%u]",
                       typeStr,
                       subTypeStr,
                       pImageInfo->version[0],
                       pImageInfo->version[1],
                       revstr,
                       pImageInfo->signingToolVersion[0],
                       pImageInfo->signingToolVersion[1],
                       pImageInfo->signingToolVersion[2],
                       pImageInfo->signingToolVersion[3]);

        BDBG_LOG(("%s", verStr));
    }
}
/* getDeviceSectionSize() scan through the SAGE 3.2 framework headers, and adds up all the device sections length
   there could be mutiply chip specific device sections */
static int getDeviceSectionSize(
    BSAGElib_Handle hSAGElib,
    BSAGElib_CombineImageHeader *pBlob,
    BSAGElib_FrameworkHeader **ppFrameworkHeader
    )
{
    unsigned productId,familyId,rev,chipId; /* hex id. for example 0x7252. */
    int sectionNum;
    int count = -1,i;
    BSAGElib_FrameworkHeader *pFramework = NULL;
    uint32_t ulChipId,ulDeviceTreeSize;
    BSAGElib_ChipType_e chipType;

    if(pBlob == NULL)
    {
        BDBG_ERR(("%s %d wrong parameter pHeader NULL",BSTD_FUNCTION,__LINE__));
        goto err;
    }

    *ppFrameworkHeader = NULL;

    familyId = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    productId = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
    if (!productId) {
        productId = familyId;
    }
    rev = productId &0xff;

    if (familyId & 0xF0000000) {
        /* 4 digit + rev */
        chipId = ((familyId & 0xffff0000) >> 8) + rev + 0xa0;
    }
    else {
        /* 5 digit */
        chipId = (familyId & 0x0fffff00) + rev + 0xa0;
    }

    sectionNum = pBlob->ucSectionNumber[0]<<24 | pBlob->ucSectionNumber[1]<<16 | pBlob->ucSectionNumber[2]<<8 |pBlob->ucSectionNumber[3];

    BDBG_MSG(("%s %d Device Blob sectionNum %d chipId %x (productId %x familyId %x) chipType %d",BSTD_FUNCTION,__LINE__,sectionNum,chipId,productId,familyId,hSAGElib->chipInfo.chipType));
    if(sectionNum < 0 || sectionNum > 100)
    {
        BDBG_ERR(("%s %d wrong parameter Device Blob sectionNum %d",BSTD_FUNCTION,__LINE__,sectionNum));
        goto err;
    }

    count = sizeof(BSAGElib_CombineImageHeader);

    for( i = 0;i<sectionNum;i++)
    {
        pFramework = (BSAGElib_FrameworkHeader *)((uint8_t*)pBlob + count);
        ulChipId = pFramework->deviceInfo.ucChipType[0]<<24 | pFramework->deviceInfo.ucChipType[1]<<16 | pFramework->deviceInfo.ucChipType[2]<<8 | pFramework->deviceInfo.ucChipType[3];
        ulDeviceTreeSize = pFramework->deviceInfo.ucDeviceTreeSize[0]<<24 | pFramework->deviceInfo.ucDeviceTreeSize[1]<<16 | pFramework->deviceInfo.ucDeviceTreeSize[2]<<8 |pFramework->deviceInfo.ucDeviceTreeSize[3];
        if(pFramework->deviceInfo.ucChipVariant[0] == 'Z' && pFramework->deviceInfo.ucChipVariant[1] == 'D')
            chipType = BSAGElib_ChipType_eZS;
        else
            chipType = BSAGElib_ChipType_eZB;

        BDBG_MSG(("%s %d i %d count %d, ulDeviceTreeSize %x ChipID %x part %c%c type %d",
                  BSTD_FUNCTION,__LINE__,i,count,ulDeviceTreeSize,ulChipId,
                  pFramework->deviceInfo.ucChipVariant[0], pFramework->deviceInfo.ucChipVariant[1],
                  chipType));
        if(/*ulChipId == chipId && chipType == hSAGElib->chipInfo.chipType &&*/ *ppFrameworkHeader == NULL)
        {
            *ppFrameworkHeader = pFramework;
        }
        count += ulDeviceTreeSize + sizeof( BSAGElib_FrameworkHeader) + sizeof(BSAGElib_DeviceBottom);
    }
err:
    return count;
}

static BERR_Code
BSAGElib_P_Boot_ParseSageImage(
    BSAGElib_Handle hSAGElib,
    BSAGElib_P_BootContext *ctx,
    uint8_t *pBinary,
    uint32_t binarySize,
    BSAGElib_SageImageHolder* holder)
{
    BERR_Code rc = BERR_SUCCESS;
    uint8_t *raw_ptr;
    size_t raw_remain;

    if (pBinary == NULL || binarySize == 0) {
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    /* save pointer to walk on buffer then get references on header, signature and data */
   raw_ptr = pBinary;
   raw_remain = binarySize;

   /* If secure boot is enabled, get pointer to buffer header */
   if(ctx->otp_sage_verify_enable || ctx->otp_sage_decrypt_enable) {
       uint32_t index;
       uint32_t image_type, image_signing_scheme, header_version;
       uint32_t all_sig_size;
       uint32_t sec_header_size;
       bool triple;

       holder->header = (BSAGElib_SageSecureHeaderCommon *)raw_ptr;

       index = _Swap16(holder->header->ucHeaderIndex);

       if(index == SAGE_HEADER_TYPE_FRAMEWORK_COMBINED)
       { /* this is a SAGE 3.2 framework image */
           int deviceBlobSize,ulDeviceTreeSize;
           uint8_t *bin_ptr;
           BSAGElib_FrameworkHeader *pFramework;
           BSAGElib_DeviceBottom *pFrameworkBottom;
           BSAGElib_CombineImageHeader *pBlob = (BSAGElib_CombineImageHeader *)raw_ptr;
           void *dst;
           const void *src;
           size_t len;
           BSAGElib_DeviceBottom  *pBottom;

           /* calculate the device Blob section size, include all the chip specific blobs, and find the current chip's section */
           deviceBlobSize = getDeviceSectionSize(hSAGElib,pBlob,&pFramework);
           BDBG_MSG(("deviceBlobSize %d pFramework %p",deviceBlobSize,(void *)pFramework));

           if(pFramework == NULL)
           {
               BDBG_ERR(("%s - Did not find framework header", BSTD_FUNCTION));
               rc = BERR_NOT_SUPPORTED;
               goto end;
           }

           /*  get current chips section info */
           ulDeviceTreeSize = pFramework->deviceInfo.ucDeviceTreeSize[0]<<24 | pFramework->deviceInfo.ucDeviceTreeSize[1]<<16 | pFramework->deviceInfo.ucDeviceTreeSize[2]<<8 |pFramework->deviceInfo.ucDeviceTreeSize[3];
           pFrameworkBottom = (BSAGElib_DeviceBottom *)((uint8_t *)pFramework + ulDeviceTreeSize + sizeof( BSAGElib_FrameworkHeader));
           bin_ptr = (uint8_t *)&(pFrameworkBottom->ucBinaryControllingParameters);

           raw_ptr += deviceBlobSize;
           raw_remain -= deviceBlobSize;

           /* move binary up, overwrite unused, other chip's section */
           pBottom = BKNI_Malloc(sizeof(BSAGElib_DeviceBottom));
           if(pBottom == NULL)
           {
               BDBG_ERR(("%s - Failure to allocate pBottom", BSTD_FUNCTION));
               rc = BERR_OUT_OF_DEVICE_MEMORY;
               goto end;
           }
           BKNI_Memcpy(pBottom,pFrameworkBottom,sizeof(BSAGElib_DeviceBottom));
           BKNI_Memmove(bin_ptr,raw_ptr,raw_remain);

           /* put signatures after binary, leave out the device tree controling parameter and device tree signature, they're infront of the binary */
           dst = (void *)(bin_ptr+raw_remain);
           src = &pBottom->ucBinaryControllingParameters;
           len = sizeof(BSAGElib_DeviceBottom) - sizeof(BSAGElib_ControllingParams) - 256;
           BKNI_Memcpy(dst,src,len);
           BKNI_Free(pBottom);

           holder->header = (BSAGElib_SageSecureHeaderCommon *)pFramework;

           holder->data = bin_ptr;
           holder->data_len = raw_remain;

           holder->signature = bin_ptr + holder->data_len + sizeof(BSAGElib_ControllingParams);

           index = _Swap16(holder->header->ucHeaderIndex);
           image_type = index;

           image_signing_scheme = holder->header->ucImageSigningScheme;
           header_version = holder->header->ucHeaderVersion;
           triple = false;
           BSAGElib_P_Boot_SetImageInfo(&hSAGElib->frameworkInfo,
                                        holder, header_version, image_type, triple);
           ctx->sageFrameworkSecureVersion = header_version;
           ctx->sageFrameworkTripleSigning = triple;
           /* Add extra sig for Framework and set image version */
           goto end;
       }

       image_type = index;
       image_signing_scheme = holder->header->ucImageSigningScheme;
       header_version = holder->header->ucHeaderVersion;

       if(image_signing_scheme == SAGE_HEADER_SECURE_IMAGE_TRIPLE_SIGNING_SCHEME_VALUE) {
           triple = true;
           all_sig_size = 2*_SIG_SZ;
           sec_header_size = sizeof(BSAGElib_SageSecureHeaderTripleSign);
       }
       else {
           triple = false;
           all_sig_size = _SIG_SZ;
           sec_header_size = sizeof(BSAGElib_SageSecureHeader);
       }

       /* eat-up header */
       raw_ptr += sec_header_size;
       raw_remain -= sec_header_size;

       if(header_version >= 0x0a && image_type == SAGE_HEADER_TYPE_BL)
       { /* SAGE 3.2 boot loader */
            if(header_version > 0x0a){
                raw_remain -= all_sig_size+sizeof(BSAGElib_ControllingParams);
            }
           holder->data = raw_ptr;
           holder->data_len = raw_remain - all_sig_size - sizeof(BSAGElib_ControllingParams);
           raw_ptr += holder->data_len ;
           holder->controlling_parameters = raw_ptr;
           raw_ptr += sizeof(BSAGElib_ControllingParams);
           raw_remain = all_sig_size;
           holder->signature = raw_ptr;
           BSAGElib_P_Boot_SetImageInfo(&hSAGElib->bootloaderInfo,
                                        holder, header_version, image_type, triple);
            ctx->sageBlSecureVersion = header_version;
            ctx->sageBlTripleSigning = triple;
       }else{
           /* get signature */
           holder->signature = raw_ptr;
           raw_ptr += all_sig_size;
           raw_remain -= all_sig_size;

           switch (image_type) {
           case SAGE_HEADER_TYPE_BL:
               BSAGElib_P_Boot_SetImageInfo(&hSAGElib->bootloaderInfo,
                                            holder, header_version, image_type, triple);
                ctx->sageBlSecureVersion = header_version;
                ctx->sageBlTripleSigning = triple;
               break;
           case SAGE_HEADER_TYPE_FRAMEWORK:
               BSAGElib_P_Boot_SetImageInfo(&hSAGElib->frameworkInfo,
                                            holder, header_version, image_type, triple);
               ctx->sageFrameworkSecureVersion = header_version;
               ctx->sageFrameworkTripleSigning = triple;
               /* Add extra sig for Framework and set image version */
               raw_ptr += all_sig_size;
               raw_remain -= all_sig_size;
               break;
           default:
               BDBG_ERR(("%s - '%s' Image: Invalid header (0x%x)", BSTD_FUNCTION, holder->name, index));
               rc = BERR_NOT_SUPPORTED;
               goto end;
           }

           /* get pointer to data, last block, i.e. whats remaining */
           holder->data = raw_ptr;
           holder->data_len = raw_remain;
       }
   }
   BDBG_MSG(("%s - '%s' Header@%p, Signature@%p, Data@%p, Data length=%d", BSTD_FUNCTION,
             holder->name, (void *)holder->header, (void *)holder->signature, (void *)holder->data, holder->data_len));

end:
   if (rc != BERR_SUCCESS) {
       BDBG_ERR(("%s failure for '%s'", BSTD_FUNCTION, holder->name));
   }
    return rc;
}

/* Write SAGE boot parameters information into Global SRAM GP registers */
static BERR_Code
BSAGElib_P_Boot_SetBootParams(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_BootSettings *pBootSettings,
    BSAGElib_SageImageHolder* frameworkHolder)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t offset = 0;
    uint32_t header_size;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    /* SAGE < -- > Host communication buffers */
    offset = hSAGElib->i_memory_map.addr_to_offset(hSAGElib->hsi_buffers);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert HSI buffer address to offset", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    _BSAGElib_P_Boot_SetBootParam(HostSageBuffers,     offset);
    _BSAGElib_P_Boot_SetBootParam(HostSageBuffersSize, SAGE_HOST_BUF_SIZE*4);

    /* Regions configuration */
    _BSAGElib_P_Boot_SetBootParam(RegionMapOffset, hSAGElib->i_memory_map.addr_to_offset(pBootSettings->pRegionMap));
    _BSAGElib_P_Boot_SetBootParam(RegionMapSize, pBootSettings->regionMapNum * sizeof(*pBootSettings->pRegionMap));

    /* SAGE Secure Logging */
    _BSAGElib_P_Boot_SetBootParam(SageLogBufferOffset, pBootSettings->logBufferOffset);
    _BSAGElib_P_Boot_SetBootParam(SageLogBufferSize, pBootSettings->logBufferSize);
    _BSAGElib_P_Boot_SetBootParam(SageLogWriteCountLSB, 0);
    _BSAGElib_P_Boot_SetBootParam(SageLogWriteCountMSB, 0);

    /* SAGE Anti Rollback */
    _BSAGElib_P_Boot_SetBootParam(SageBootloaderEpochVersion, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkEpochVersion, 0);

    /* Control SAGE life cycle */
    _BSAGElib_P_Boot_SetBootParam(Reset, 0);
    _BSAGElib_P_Boot_SetBootParam(Suspend, 0);

    /* Sage Status and versioning */
    _BSAGElib_P_Boot_SetBootParam(LastError, 0);
    _BSAGElib_P_Boot_SetBootParam(BootStatus, BSAGElibBootStatus_eNotStarted);
    _BSAGElib_P_Boot_SetBootParam(SageBootloaderVersion, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkVersion, 0);

    /* Misc */
    _BSAGElib_P_Boot_SetBootParam(SageStatusFlags, 0);
#if (BHSM_API_VERSION==1)
    /* SAGE Services parameters - resources */
    {
        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        uint32_t sageVklMask = ((1 << BHSM_RemapVklId(hSAGElib->vkl1)) | (1 << BHSM_RemapVklId(hSAGElib->vkl2)));
        _BSAGElib_P_Boot_SetBootParam(SageVklMask, sageVklMask);
    }
#else
     /* SAGE Services parameters - resources */
    {
        uint32_t sageVklMask;
        BHSM_KeyLadderInfo info;

        info.index = 32; /* shift past uint32 if no return value */
        BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle1, &info);
        sageVklMask = 1<<info.index;

        info.index = 32;
        BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle2, &info);
        sageVklMask |= 1<<info.index;

        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        _BSAGElib_P_Boot_SetBootParam(SageVklMask, sageVklMask);
         BDBG_LOG(("%s:%u BSAGElib_P_Boot_SetBootParam(SageVklMask,0x%08X)", BSTD_FUNCTION,__LINE__,sageVklMask));
   }
#endif
    _BSAGElib_P_Boot_SetBootParam(SageDmaChannel, 0);

    /* SAGE Secure Boot */
    offset = hSAGElib->i_memory_map.addr_to_offset(frameworkHolder->data);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert framework address to offset", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBin, offset);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSize, frameworkHolder->data_len);

    if (ctx->otp_sage_verify_enable || ctx->otp_sage_decrypt_enable)
    {
        uint32_t data;

        /* Framework Signature */
        if (frameworkHolder->signature) {
            uint8_t *signature;
            signature = BSAGElib_P_Boot_GetSignature(ctx, frameworkHolder);
            offset = hSAGElib->i_memory_map.addr_to_offset(signature);
            if(offset == 0) {
                BDBG_ERR(("%s - Cannot convert SAGE Framework signature address to offset", BSTD_FUNCTION));
                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            hSAGElib->i_memory_sync.flush(signature, 256);
            _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSignature, offset);
        } else {
            _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSignature, 0);
        }

        /* pull text section size */
        if (ctx->sageFrameworkTripleSigning)
        {
            BSAGElib_SageSecureHeaderTripleSign *triple_sign_header = (BSAGElib_SageSecureHeaderTripleSign *)frameworkHolder->header;
            data = (triple_sign_header->ucInstrSectionSize[3] << 0) | (triple_sign_header->ucInstrSectionSize[2] << 8) | (triple_sign_header->ucInstrSectionSize[1] << 16) | (triple_sign_header->ucInstrSectionSize[0] << 24);
        }
        else
        {
            BSAGElib_SageSecureHeader *single_sign_header = (BSAGElib_SageSecureHeader *)frameworkHolder->header;
            BSAGElib_FrameworkHeader  *pSage32Header =      (BSAGElib_FrameworkHeader  *)frameworkHolder->header;
            uint32_t header_version;
            header_version = single_sign_header->ucHeaderVersion;
            if(header_version >= 0x0a)
            { /* SAGE 3.2 framework */
                data = (pSage32Header->ucInstrSectionSize[3] << 0) | (pSage32Header->ucInstrSectionSize[2] << 8) | (pSage32Header->ucInstrSectionSize[1] << 16) | (pSage32Header->ucInstrSectionSize[0] << 24);
            }else{
              /* SAGE 3.1 framework */
                data = (single_sign_header->ucInstrSectionSize[3] << 0) | (single_sign_header->ucInstrSectionSize[2] << 8) | (single_sign_header->ucInstrSectionSize[1] << 16) | (single_sign_header->ucInstrSectionSize[0] << 24);
            }
        }
        _BSAGElib_P_Boot_SetBootParam(TextSectionSize, data);

        offset = hSAGElib->i_memory_map.addr_to_offset(frameworkHolder->header);
        if(offset == 0) {
            BDBG_ERR(("%s - Cannot convert SAGE framework header address to offset", BSTD_FUNCTION));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
        header_size = BSAGElib_P_Boot_GetHeaderSize(ctx, frameworkHolder);
        hSAGElib->i_memory_sync.flush(frameworkHolder->header, header_size);
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, offset);
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, header_size);
    }
end:
    return rc;
}

static BERR_Code
BSAGElib_P_Boot_ResetSage(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *blImg)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeaderCommon *header = blImg->header;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;
#if (BHSM_API_VERSION==1)
    BCMD_VKLID_e vkl_id = BCMD_VKL_eMax;

    /* if OTP_SAGE_VERIFY_ENABLE_BIT: verify SAGE boot loader */
    if(ctx->otp_sage_verify_enable) {
        BHSM_VerifySecondTierKeyIO_t secondTierKey;
        BCMD_SecondTierKey_t *pSecondTierKey;

        BKNI_Memset(&secondTierKey, 0, sizeof(BHSM_VerifySecondTierKeyIO_t));

        if(header->ucKey0Type == SAGE_HEADER_KEY0_SELECT_KEY0) {
            secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0;
        }
        else {
            secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0Prime;
        }
        secondTierKey.eKeyIdentifier = BCMD_SecondTierKeyId_eKey3;

        pSecondTierKey = BSAGElib_P_Boot_GetKey(ctx, blImg);
        secondTierKey.keyAddr = hSAGElib->i_memory_map.addr_to_offset(pSecondTierKey);
        if(secondTierKey.keyAddr == 0) {
            rc = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s - Cannot convert 2nd-tier key address to offset", BSTD_FUNCTION));
            goto end;
        }
        hSAGElib->i_memory_sync.flush(pSecondTierKey, sizeof(*pSecondTierKey));
        BSAGElib_iLockHsm();
        rc = BHSM_VerifySecondTierKey(hSAGElib->core_handles.hHsm,  &secondTierKey);
        BSAGElib_iUnlockHsm();
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - BHSM_VerifySecondTierKey() failed on key from SAGE Bootloader image [0x%x]", BSTD_FUNCTION, rc));
            goto end;
        }
    }

    { /* prepare a Vkl for SAGE BL */
        BHSM_AllocateVKLIO_t allocateVKLIO;

        BKNI_Memset(&allocateVKLIO, 0, sizeof(BHSM_AllocateVKLIO_t));
        allocateVKLIO.client                  = BHSM_ClientType_eHost;
        allocateVKLIO.customerSubMode         = BCMD_CustomerSubMode_eGeneric_CA_64_4;
        allocateVKLIO.bNewVKLCustSubModeAssoc = false;

        BSAGElib_iLockHsm();
        rc = BHSM_AllocateVKL(hSAGElib->core_handles.hHsm, &allocateVKLIO);
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS )
        {
            BDBG_ERR(("%s - BHSM_AllocateVKL() fails %d", BSTD_FUNCTION, rc));
            goto end;
        }

        vkl_id = allocateVKLIO.allocVKL;
        BDBG_MSG(("%s - SAGE BL VKL ID: %d", BSTD_FUNCTION, vkl_id));
    }

    /* If OTP_SAGE_DECRYPT_ENABLE_BIT: decrypt the SAGE BL  */
    if(ctx->otp_sage_decrypt_enable) {
        {
            BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
            uint32_t data;

            /* GRK 1 */
            BKNI_Memset(&generateRouteKeyIO, 0, sizeof(BHSM_GenerateRouteKeyIO_t));
            generateRouteKeyIO.keyLadderSelect              = BCMD_eFWKL;
            generateRouteKeyIO.bASKM3DesKLRootKeySwapEnable = false;
            generateRouteKeyIO.keyGenMode                   = BCMD_KeyGenFlag_eGen;
            /* For Hardware Key Ladder */
            generateRouteKeyIO.hwklLength                   = BCMD_HWKL_LEN0;
            generateRouteKeyIO.hwklDestAlg                  = 0;
            /* HDDTA support */
            generateRouteKeyIO.bResetPKSM      = false;
            generateRouteKeyIO.PKSMInitSize    = 0;
            generateRouteKeyIO.PKSMcycle       = 0;
            generateRouteKeyIO.client                 = BHSM_ClientType_eHost;
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0))
            generateRouteKeyIO.cusKeySwizzle0aVariant = header->ucSwizzle0aVariant;
            BSTD_UNUSED(data);
#else
            switch(header->ucSwizzle0aVariant)
            {
                case 0:
                    data = 3; /* BCMD_OwnerIDSelect_eUse1 = 3 */
                    break;
                case 1:
                    data = BCMD_OwnerIDSelect_eMSP0;
                    break;
                case 2:
                    data = BCMD_OwnerIDSelect_eMSP1;
                    break;
                case 3:
                default:
                    /* Invalid parameter */
                    data = 4; /* BCMD_OwnerIDSelect_eMax */
                    break;
            }
            generateRouteKeyIO.cusKeySwizzle0aVariant = data;
#endif
            generateRouteKeyIO.cusKeySwizzle0aVersion      = header->ucSwizzle0aVersion;
            generateRouteKeyIO.sageAskmConfigurationEnable = true;
            generateRouteKeyIO.cusKeySwizzle0aEnable       = true;
            generateRouteKeyIO.sageModuleID                = header->ucModuleId;
            generateRouteKeyIO.sageSTBOwnerID              = BCMD_STBOwnerID_eOneVal;
            generateRouteKeyIO.sageCAVendorID              = (header->ucCaVendorId[3] << 24) |
                                                             (header->ucCaVendorId[2] << 16) |
                                                             (header->ucCaVendorId[1] << 8) |
                                                             (header->ucCaVendorId[0]);
            generateRouteKeyIO.sageMaskKeySelect           = BCMD_ASKM_MaskKeySel_eRealMaskKey;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0))
            generateRouteKeyIO.keyTweak              = BCMD_KeyTweak_eNoTweak;
#endif

            /* HWKL CWC setting */
            generateRouteKeyIO.bHWKLVistaKeyGenEnable = 0;
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.customerSubMode    = BCMD_CustomerSubMode_eSAGE_BL_DECRYPT;
            generateRouteKeyIO.keyMode            = BCMD_KeyMode_eRegular;
            generateRouteKeyIO.ucKeyDataLen       = 16;
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, header->ucProcIn1, 16);
            generateRouteKeyIO.unKeySlotNum     = 0;
            generateRouteKeyIO.caKeySlotType    = BCMD_XptSecKeySlot_eType3;
            generateRouteKeyIO.bASKMModeEnabled = true;
            generateRouteKeyIO.bSwapAesKey      = false;
            generateRouteKeyIO.key3Op           = BCMD_Key3Op_eKey3NoProcess;
            generateRouteKeyIO.keyDestIVType    = BCMD_KeyDestIVType_eNoIV;
          /*  generateRouteKeyIO.subCmdID         = BCMD_VKLAssociationQueryFlag_e;*/
            generateRouteKeyIO.keyLadderType    = BCMD_KeyLadderType_eAES128;
            generateRouteKeyIO.rootKeySrc       = BCMD_RootKeySrc_eCusKey;
            generateRouteKeyIO.bUseCustKeyLowDecrypt = false;
            generateRouteKeyIO.ucCustKeyLow     = header->ucCustKeyVar;
            generateRouteKeyIO.bUseCustKeyHighDecrypt = false;
            generateRouteKeyIO.ucCustKeyHigh    = header->ucCustKeyVar;
            generateRouteKeyIO.ucKeyVarLow      = header->ucKeyVarlo;
            generateRouteKeyIO.ucKeyVarHigh     = header->ucKeyVarHi;
            generateRouteKeyIO.keySize          = BCMD_KeySize_e128;
            generateRouteKeyIO.bIsRouteKeyRequired = false;
            generateRouteKeyIO.keyDestBlckType  = BCMD_KeyDestBlockType_eCPDescrambler;
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0))
            generateRouteKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
#else
            generateRouteKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
#endif
            generateRouteKeyIO.RpipeSC01Val     = 0;
            generateRouteKeyIO.GpipeSC01Val     = 0;
            generateRouteKeyIO.SC01ModeMapping     = BCMD_SC01ModeWordMapping_eClear;
            generateRouteKeyIO.keyLayer         = BCMD_KeyRamBuf_eKey3;
            generateRouteKeyIO.swizzleType      = BCMD_SwizzleType_eSwizzle0;

            BSAGElib_iLockHsm();
            rc = BHSM_GenerateRouteKey (hSAGElib->core_handles.hHsm, &generateRouteKeyIO);
            BSAGElib_iUnlockHsm();
            if (rc != BERR_SUCCESS) {
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key3 fails %d", BSTD_FUNCTION, rc));
                goto end;
            }

            /* GRK 4 */
            BKNI_Memset(&generateRouteKeyIO, 0, sizeof(BHSM_GenerateRouteKeyIO_t));
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.keySize            = BCMD_KeySize_e128;
            generateRouteKeyIO.ucKeyDataLen       = 16;
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, header->ucProcIn2, 16);
            generateRouteKeyIO.bIsRouteKeyRequired = false;
            generateRouteKeyIO.keyDestBlckType  = BCMD_KeyDestBlockType_eCPDescrambler;
            generateRouteKeyIO.keyLayer         = BCMD_KeyRamBuf_eKey4;
            generateRouteKeyIO.bSwapAesKey      = false;
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.keyLadderType    = BCMD_KeyLadderType_eAES128;
            generateRouteKeyIO.RpipeSC01Val     = 0;
            generateRouteKeyIO.GpipeSC01Val     = 0;
            generateRouteKeyIO.SC01ModeMapping  = BCMD_SC01ModeWordMapping_eClear;
            generateRouteKeyIO.keyGenMode       = BCMD_KeyGenFlag_eGen;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0))
            generateRouteKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
#endif
            generateRouteKeyIO.client           = BHSM_ClientType_eHost;
            generateRouteKeyIO.keyLadderSelect  = BCMD_eFWKL;
            generateRouteKeyIO.caKeySlotType    = BCMD_XptSecKeySlot_eType3;

            BSAGElib_iLockHsm();
            rc = BHSM_GenerateRouteKey (hSAGElib->core_handles.hHsm, &generateRouteKeyIO);
            BSAGElib_iUnlockHsm();
            if (rc != BERR_SUCCESS) {
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key4 fails %d", BSTD_FUNCTION, rc));
                goto end;
            }

            /* GRK 5 */
            BKNI_Memset(&generateRouteKeyIO, 0, sizeof(BHSM_GenerateRouteKeyIO_t));
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.keySize          = BCMD_KeySize_e128;
            generateRouteKeyIO.ucKeyDataLen       = 16;
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, header->ucProcIn3, 16);
            generateRouteKeyIO.bIsRouteKeyRequired = false;
            generateRouteKeyIO.keyDestBlckType  = BCMD_KeyDestBlockType_eCPDescrambler;
            generateRouteKeyIO.keyLayer         = BCMD_KeyRamBuf_eKey5;
            generateRouteKeyIO.bSwapAesKey      = false;
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.keyLadderType    = BCMD_KeyLadderType_eAES128;
            generateRouteKeyIO.RpipeSC01Val     = 0;
            generateRouteKeyIO.GpipeSC01Val     = 0;
            generateRouteKeyIO.SC01ModeMapping  = BCMD_SC01ModeWordMapping_eClear;
            generateRouteKeyIO.keyGenMode       = BCMD_KeyGenFlag_eGen;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0))
            generateRouteKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
#endif
            generateRouteKeyIO.client                 = BHSM_ClientType_eHost;
            generateRouteKeyIO.keyLadderSelect              = BCMD_eFWKL;
            generateRouteKeyIO.caKeySlotType    = BCMD_XptSecKeySlot_eType3;

            BSAGElib_iLockHsm();
            rc = BHSM_GenerateRouteKey (hSAGElib->core_handles.hHsm, &generateRouteKeyIO);
            BSAGElib_iUnlockHsm();
            if (rc != BERR_SUCCESS) {
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key5 fails %d", BSTD_FUNCTION, rc));
                goto end;
            }
        }
    }

    { /* setup and verify region */
        BHSM_RegionConfiguration_t  verifyIO;
        BHSM_RegionStatus_t status;

        BKNI_Memset(&verifyIO, 0, sizeof(BHSM_RegionConfiguration_t));
        verifyIO.cpuType = BCMD_MemAuth_CpuType_eScpu;
        verifyIO.ucIntervalCheckBw = 0x10;
        verifyIO.SCBBurstSize = BCMD_ScbBurstSize_e64;
        verifyIO.unRSAKeyID = 0x03;

        verifyIO.region.startAddress = hSAGElib->i_memory_map.addr_to_offset(blImg->data);
        if(verifyIO.region.startAddress == 0) {
            BDBG_ERR(("%s - Cannot convert bootloader address to offset", BSTD_FUNCTION));
            goto end;
        }
        verifyIO.region.endAddress = verifyIO.region.startAddress + (blImg->data_len) - 1;

        if (blImg->signature) {
            uint8_t *signature;
            signature = BSAGElib_P_Boot_GetSignature(ctx, blImg);
            verifyIO.signature.startAddress = hSAGElib->i_memory_map.addr_to_offset(signature);
            if(verifyIO.signature.startAddress == 0) {
                BDBG_ERR(("%s - Cannot convert bootloader signature address to offset", BSTD_FUNCTION));
                goto end;
            }
            hSAGElib->i_memory_sync.flush(signature, 256);
        }
        else {
            verifyIO.signature.startAddress = 0;
        }
        verifyIO.signature.endAddress = verifyIO.signature.startAddress + 256 - 1;

        if(ctx->otp_sage_verify_enable || ctx->otp_sage_decrypt_enable) {
            verifyIO.bgCheck = 0x0D;
            verifyIO.instrChecker = 0x0C;

            if(ctx->otp_sage_decrypt_enable) {
                verifyIO.vkl = vkl_id;
                verifyIO.keyLayer = BCMD_KeyRamBuf_eKey5;
            }

            if(ctx->otp_sage_verify_enable) {
                verifyIO.codeRelocatable = 0;
                verifyIO.unEpoch = 0;
                verifyIO.unMarketID = 0;
                verifyIO.unMarketIDMask = 0;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
                verifyIO.ucSigVersion = 0x01;
                verifyIO.ucSigType = 0x04;
                if(ctx->sageBlSecureVersion >= 2) {
                    verifyIO.ucEpochSel = 0x3;
                }
                else {
                    verifyIO.ucEpochSel = 0;
                }

                if(ctx->sageBlSecureVersion >= 3) {
                    /* set the bits based on EPOCH version */
                    verifyIO.unEpoch |= (1 << header->ucEpochVersion) - 1;
                }
#endif
                if(ctx->sageBlSecureVersion == 2) {
                    verifyIO.unEpochMask = 0x03;
                }
                else if(ctx->sageBlSecureVersion >= 3) {
                    verifyIO.unEpochMask = 0xFF;
                }
                else {
                    verifyIO.unEpochMask = 0;
                }
            }
        }

        /* SAGE BL AR */
        /* Share the SAGE BL AR version through SAGE Global SRAM */
        _BSAGElib_P_Boot_SetBootParam(SageBootloaderEpochVersion, header->ucEpochVersion);

        BSAGElib_iLockHsm();
        rc = BHSM_RegionVerification_Configure(hSAGElib->core_handles.hHsm, 0x18, &verifyIO);
        BSAGElib_iUnlockHsm();
        if ( rc != 0 ) {
            BDBG_ERR(("BHSM_RegionVerification_Configure fails - region = 0x%x, status = %x", 0x18, rc));
            goto end;
        }

        BSAGElib_iLockHsm();
        rc = BHSM_RegionVerification_Enable(hSAGElib->core_handles.hHsm);
        BSAGElib_iUnlockHsm();
        if ( rc != 0) {
            BDBG_ERR(("BHSM_RegionVerification_Enable fails - status = %x", rc));
            goto end;
        }

        do
        {
            BSAGElib_iLockHsm();
            rc = BHSM_RegionVerification_Status(hSAGElib->core_handles.hHsm, 0x18, &status);
            BSAGElib_iUnlockHsm();
            if(rc != 0)
            {
                /*  Only return BERR_SUCCESS for verification in-progress status */
                if(status.state != BSP_STATUS_eRegionVerifyInProgress)
                {
                    rc = BERR_INVALID_PARAMETER;
                    BDBG_ERR(("%s - BHSM_RegionVerification_Status fails - region = 0x%x, status = 0x%x", BSTD_FUNCTION, 0x18, rc));
                    goto end;
                }
                else
                {
                    BDBG_MSG(("%s - BHSM_RegionVerification_Status in progress", BSTD_FUNCTION));
                }
            }
        }while (status.state != BHSM_RegionVerificationStatus_eVerified);

        BDBG_LOG(("%s: SAGE reset completed successfully", BSTD_FUNCTION));
    }
end:
    if(vkl_id != BCMD_VKL_eMax)
    {
        BSAGElib_iLockHsm();
        BHSM_FreeVKL(hSAGElib->core_handles.hHsm, vkl_id);
        BSAGElib_iUnlockHsm();
    }

#else
    BHSM_RvRsaHandle hRvRsa = NULL;
    BHSM_RvRsaAllocateSettings rsaAllocConf;
    BHSM_RvRsaSettings rsaConf;
    BHSM_KeyLadderHandle hKeyLadder = NULL;
    BHSM_KeyLadderAllocateSettings ladderAllocConf;
    BHSM_KeyLadderSettings ladderConf;
    BHSM_KeyLadderLevelKey ladderLevelKey;
    BCMD_SecondTierKey_t *pRsaKey;   /* BCMD_* is not a good name for this type  */
    BHSM_RvRegionHandle hRv = NULL;
    BHSM_RvRegionAllocateSettings rvAllocConf;
    BHSM_RvRegionSettings rvConf;
    uint8_t *pSignature;
    BMMA_DeviceOffset startAddress;
    BHSM_RvRegionStatus regionStatus;
    unsigned count;
    uint8_t *controlling_parameters=NULL;

    /* Share the SAGE BL AR version through SAGE Global SRAM */
    _BSAGElib_P_Boot_SetBootParam( SageBootloaderEpochVersion, header->ucEpochVersion );

    if( ctx->otp_sage_verify_enable )
    {
        /* allocate RSA Key slot. */
        BKNI_Memset( &rsaAllocConf, 0, sizeof(rsaAllocConf) );
        rsaAllocConf.rsaKeyId = BHSM_ANY_ID;
        BSAGElib_iLockHsm();
        hRvRsa = BHSM_RvRsa_Allocate( hSAGElib->core_handles.hHsm, &rsaAllocConf );
        BSAGElib_iUnlockHsm();
        if( !hRvRsa ){ rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto end; }

        /* Configure the RSA Key.*/
        BKNI_Memset( &rsaConf, 0, sizeof(rsaConf) );
        if( header->ucKey0Type == SAGE_HEADER_KEY0_SELECT_KEY0 ) {
            rsaConf.rootKey = BHSM_RvRsaRootKey_e0;
        }
        else {
            rsaConf.rootKey = BHSM_RvRsaRootKey_e0Prime;
        }

        pRsaKey = BSAGElib_P_Boot_GetKey( ctx, blImg );
        rsaConf.keyOffset = hSAGElib->i_memory_map.addr_to_offset( pRsaKey );
        if( rsaConf.keyOffset == 0 ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }

        BSAGElib_iLockHsm();
        rc = BHSM_RvRsa_SetSettings( hRvRsa,  &rsaConf );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }
    }

    if( ctx->otp_sage_decrypt_enable )
    {
        /* alloc keyladder */
        BKNI_Memset( &ladderAllocConf, 0, sizeof(ladderAllocConf) );
        ladderAllocConf.owner = BHSM_SecurityCpuContext_eHost;
        ladderAllocConf.index = BHSM_ANY_ID;
        BSAGElib_iLockHsm();
        hKeyLadder = BHSM_KeyLadder_Allocate( hSAGElib->core_handles.hHsm, &ladderAllocConf );
        BSAGElib_iUnlockHsm();
        if( !hKeyLadder ){ rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto end; }

        /* configure keyladder root key */
        BKNI_Memset( &ladderConf, 0, sizeof(ladderConf) );
        ladderConf.algorithm = BHSM_CryptographicAlgorithm_eAes128;
        ladderConf.operation = BHSM_CryptographicOperation_eDecrypt;
        ladderConf.mode = BHSM_KeyLadderMode_eSageBlDecrypt;
        ladderConf.numLevels = 5;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0))
        ladderConf.root.askm.caVendorIdScope    = BHSM_KeyladderCaVendorIdScope_eFixed;
        ladderConf.root.askm.stbOwnerSelect     = header->ucStbOwnerIdSelect;
        ladderConf.root.askm.caVendorId         = (header->ucCaVendorId[3] << 24) |
                                                    (header->ucCaVendorId[2] << 16) |
                                                    (header->ucCaVendorId[1] << 8) |
                                                    (header->ucCaVendorId[0]);

        ladderConf.root.type = BHSM_KeyLadderRootType_eGlobalKey;
        ladderConf.root.globalKey.index = 0x3B;
#else
        ladderConf.root.askm.stbOwnerSelect     = header->ucStbOwnerIdSelect;
        ladderConf.root.askm.caVendorId         = (header->ucCaVendorId[3] << 24) |
                                                  (header->ucCaVendorId[2] << 16) |
                                                  (header->ucCaVendorId[1] << 8) |
                                                  (header->ucCaVendorId[0]);
        ladderConf.root.customerKey.type = BHSM_SwizzelType_e0;
        ladderConf.root.customerKey.swizzle1IndexSel = 0;
        ladderConf.root.customerKey.enableSwizzle0a = true;
        ladderConf.root.customerKey.low.keyIndex = header->ucCustKeyVar;
        ladderConf.root.customerKey.low.keyVar = header->ucKeyVarlo;
        ladderConf.root.customerKey.low.decrypt = false;
        ladderConf.root.customerKey.high.keyIndex = header->ucCustKeyVar;
        ladderConf.root.customerKey.high.keyVar = header->ucKeyVarHi;
        ladderConf.root.customerKey.high.decrypt = false;
        ladderConf.root.type = BHSM_KeyLadderRootType_eCustomerKey;
        ladderConf.root.customerKey.cusKeySwizzle0aVersion = header->ucSwizzle0aVersion;
#endif

        switch(header->ucSwizzle0aVariant) {
            case 0:
                ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne;
                ladderConf.root.customerKey.cusKeySwizzle0aVariant = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne;
                break;
            case 1:
                ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0;
                ladderConf.root.customerKey.cusKeySwizzle0aVariant = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0;
                break;
            case 2:
                ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1;
                ladderConf.root.customerKey.cusKeySwizzle0aVariant = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1;
                break;
            case 3:
                ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne;
                ladderConf.root.customerKey.cusKeySwizzle0aVariant = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMax;
                break;
            default:
                /* Invalid parameter */
                 ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMax;
                 ladderConf.root.customerKey.cusKeySwizzle0aVariant = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMax;
               break;
        }

        BSAGElib_iLockHsm();
        rc = BHSM_KeyLadder_SetSettings( hKeyLadder, &ladderConf );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

        /* set level 3 key */
        BKNI_Memset( &ladderLevelKey, 0, sizeof(ladderLevelKey) );

        ladderLevelKey.level = 3;
        BKNI_Memcpy( ladderLevelKey.ladderKey, header->ucProcIn1, sizeof(header->ucProcIn1) );
        ladderLevelKey.ladderKeySize = 128;
        ladderLevelKey.route.destination = BHSM_KeyLadderDestination_eNone;

        BSAGElib_iLockHsm();
        rc = BHSM_KeyLadder_GenerateLevelKey( hKeyLadder, &ladderLevelKey );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

        /* set level 4 key */
        ladderLevelKey.level = 4;
        BKNI_Memcpy( ladderLevelKey.ladderKey, header->ucProcIn2, sizeof(header->ucProcIn2) );
        ladderLevelKey.ladderKeySize = 128;

        BSAGElib_iLockHsm();
        rc = BHSM_KeyLadder_GenerateLevelKey( hKeyLadder, &ladderLevelKey );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

        /* set level 5 key */
        ladderLevelKey.level = 5;
        BKNI_Memcpy( ladderLevelKey.ladderKey, header->ucProcIn3, sizeof(header->ucProcIn3) );
        ladderLevelKey.ladderKeySize = 128;

        BSAGElib_iLockHsm();
        rc = BHSM_KeyLadder_GenerateLevelKey( hKeyLadder, &ladderLevelKey );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }
    }
    /* allocate Region Verification instance. */
    BKNI_Memset( &rvAllocConf, 0, sizeof(rvAllocConf) );
#if 0
    rvAllocConf.regionType = BHSM_RvRegionType_eScpu;
    rvAllocConf.regionSubType.scpu = BHSM_RvRegionSubTypeScpu_eFsbl;
#else
    rvAllocConf.regionId = BHSM_RegionId_eRedacted_0x18; /* TBD this should be publicly named Fsbl */
#endif
    BSAGElib_iLockHsm();
    hRv = BHSM_RvRegion_Allocate( hSAGElib->core_handles.hHsm, &rvAllocConf );
    BSAGElib_iUnlockHsm();
    if( !hRv ){ rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto end; }

    pSignature = BSAGElib_P_Boot_GetSignature( ctx, blImg );
    if( !pSignature ){ rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }
    startAddress = hSAGElib->i_memory_map.addr_to_offset(blImg->data);
    if( startAddress == 0 ){ rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }

    /* configure region verification. */
    BKNI_Memset( &rvConf, 0, sizeof(rvConf) );
    rvConf.intervalCheckBandwidth = 0x10;
#if 0
    rvConf.scbBurstSize = 64;
    rvConf.range[0].startAddress = startAddress;  /* TODO SHOUD BE LOCAL */
    rvConf.range[0].size = blImg->data_len;
    BKNI_Memcpy( &rvConf.signature, pSignature, sizeof(rvConf.signature) );
#else
    rvConf.range[0].address = startAddress;
    rvConf.range[0].size = blImg->data_len;
    rvConf.signature.address = hSAGElib->i_memory_map.addr_to_offset(pSignature);
    rvConf.signature.size = BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES;
#endif

#if RV_CONTROLLING_PARAMETERS_REVERSED
    controlling_parameters = hSAGElib->i_memory_alloc.malloc(sizeof(BSAGElib_ControllingParams));
    {
        uint32_t k; uint8_t* s = blImg->controlling_parameters;
        for(k=0; k<sizeof(BSAGElib_ControllingParams); k+=4){
            controlling_parameters[k  ]= s[k+3];
            controlling_parameters[k+1]= s[k+2];
            controlling_parameters[k+2]= s[k+1];
            controlling_parameters[k+3]= s[k  ];
        }
    }
    hSAGElib->i_memory_sync.flush(controlling_parameters, sizeof(BSAGElib_ControllingParams));
#else
    controlling_parameters = blImg->controlling_parameters;
#endif
    rvConf.parameters.address = hSAGElib->i_memory_map.addr_to_offset(controlling_parameters);

    rvConf.rvRsaHandle = hRvRsa;
    rvConf.keyLadderHandle = hKeyLadder; /* may be  NULL.*/
    rvConf.keyLadderLayer = 5;
#if 0
    rvConf.epoch = 0;
    rvConf.epochMask = 0;
#endif
    /* InstChk managed by BFW based on region ID */

    if(ctx->otp_sage_verify_enable || ctx->otp_sage_decrypt_enable)
    {
        rvConf.backgroundCheck = true;

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
        rvConf.epoch = 0;
        rvConf.marketId = 0;
        rvConf.marketIdMask = 0;
        rvConf.codeRelocatable = 1;
        rvConf.signatureType = BHSM_RvSignatureType_eCode;
        rvConf.signatureVersion = 0x01;
        if(ctx->sageBlSecureVersion >= 2)
        {
            rvConf.epochSelect = 0x3;
        }
        else
        {
            rvConf.epochSelect = 0;
        }

        if(ctx->sageBlSecureVersion >= 3) {
            /* set the bits based on EPOCH version */
            rvConf.epoch |= (1 << header->ucEpochVersion) - 1;
        }

        if(ctx->sageBlSecureVersion == 2) {
            rvConf.epochMask = 0x03;
        }
        else if(ctx->sageBlSecureVersion >= 3) {
            rvConf.epochMask = 0xFF;
        }
        else {
            rvConf.epochMask = 0;
        }
#endif
    }

    rvConf.allowRegionDisable = true;
    rvConf.enforceAuth = true;

#if ((BCHP_CHIP == 7278 && BCHP_VER < BCHP_VER_B0))
    /* work-around for DMA bug BKNI_Memcpy( LocalRAM, blImg->data, blImg->data_len);*/
    {
        uint32_t *pData = (uint32_t *)blImg->data;
        uint32_t word_count = blImg->data_len/4;
        uint32_t ix;
        BDBG_MSG(("%s:%u copying %u bytes of SBL to LocalRAM",BSTD_FUNCTION,__LINE__, blImg->data_len));
        for (ix=0; ix<word_count; ix++) {
            BREG_Write32(hSAGElib->core_handles.hReg,BCHP_SCPU_LOCALRAM_REG_START+4*ix, pData[ix]);
        }
    }
#endif

    BSAGElib_iLockHsm();
    rc = BHSM_RvRegion_SetSettings( hRv, &rvConf );
    BSAGElib_iUnlockHsm();
    if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

    /* enable region verification */
    BSAGElib_iLockHsm();
    rc = BHSM_RvRegion_Enable( hRv );
    BSAGElib_iUnlockHsm();
    if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

    count = 200;
    do{
        BKNI_Sleep( 10 );

        BSAGElib_iLockHsm();
        rc = BHSM_RvRegion_GetStatus( hRv, &regionStatus );
        BSAGElib_iUnlockHsm();
        if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

        BDBG_LOG(("[%s] SAGE BL region status = 0x%08X", BSTD_FUNCTION, regionStatus.status));

    } while( !(regionStatus.status&BHSM_RV_REGION_STATUS_FAST_CHECK_FINISHED) && --count );

    if( regionStatus.status & BHSM_RV_REGION_STATUS_FAST_CHECK_FAILED ) {
        rc = BERR_TRACE(BHSM_STATUS_REGION_VERIFICATION_FAILED); goto end;
    }

    if( !count ) {
        rc = BERR_TRACE(BERR_TIMEOUT); goto end;
    }

end:
#if RV_CONTROLLING_PARAMETERS_REVERSED
    if(controlling_parameters) {
        hSAGElib->i_memory_alloc.free(controlling_parameters);
    }
#endif
    if( hRv ) BHSM_RvRegion_Free( hRv );
    if( hRvRsa ) BHSM_RvRsa_Free( hRvRsa );
    if( hKeyLadder ) BHSM_KeyLadder_Free( hKeyLadder );
#endif
    BDBG_LOG((" LEAVING -----------------  [%s]", BSTD_FUNCTION));
    return rc;
}

/****************************************
 * Public API
 ***************************************/

/* Host (nexus) has restarted... check/reset connection w/ SAGE */
/* To be called prior to any attempts to communicate w/ SAGE */
BERR_Code
BSAGElib_Boot_HostReset(
    BSAGElib_Handle hSAGElib,
    const BSAGElib_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_INVALID_PARAMETER;
    uint32_t offset = 0;
    uint32_t val;
    BSAGElib_SageImageHolder frameworkHolder;
    BSAGElib_P_BootContext *ctx = NULL;
    BSAGElib_RegionInfo local_region[3];
    unsigned i;

    BDBG_ENTER(BSAGElib_Boot_HostReset);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pBootSettings);

    /* Validate boot settings */
    if ((pBootSettings->pFramework == NULL)      ||
        (pBootSettings->frameworkSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", BSTD_FUNCTION));
        goto end;
    }

    if (hSAGElib->core_handles.hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", BSTD_FUNCTION));
        goto end;
    }

    ctx = BKNI_Malloc(sizeof(*ctx));
    if (ctx == NULL) {
        BDBG_ERR(("%s - Cannot allocate temporary context for SAGE Boot.", BSTD_FUNCTION));
        goto end;
    }
    BKNI_Memset(ctx, 0, sizeof(*ctx));
    ctx->hSAGElib = hSAGElib;

    /* Get SAGE Secureboot OTP MSPs value */
    rc = BSAGElib_P_Boot_GetSageOtpMspParams(ctx);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_GetSageOtpMspParams() fails", BSTD_FUNCTION));
        goto end;
    }

    rc = BERR_INVALID_PARAMETER;

    offset = hSAGElib->i_memory_map.addr_to_offset(hSAGElib->hsi_buffers);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert HSI buffer address to offset", BSTD_FUNCTION));
        goto end;
    }

    /* HSI will not work w/o HSI buffers, and GLR region */
    _BSAGElib_P_Boot_GetBootParam(HostSageBuffers, val);
    if(val!=offset)
    {
        BDBG_ERR(("%s - Host/Sage buffer address cannot change", BSTD_FUNCTION));
        goto end;
    }

    _BSAGElib_P_Boot_GetBootParam(HostSageBuffersSize, val);
    if(val!=(SAGE_HOST_BUF_SIZE*4))
    {
        BDBG_ERR(("%s - Host/Sage buffer size cannot change", BSTD_FUNCTION));
        goto end;
    }

    /* On "cleanup", SAGE will store last GLR region in re-used GSRAM */
    /* Store for later check w/ CRR/SRR */
    _BSAGElib_P_Boot_GetBootParam(LastRegionGlrOffset, local_region[0].offset);
    _BSAGElib_P_Boot_GetBootParam(LastRegionGlrSize, local_region[0].size);
    local_region[0].id=BSAGElib_RegionId_Glr;

    _BSAGElib_P_Boot_GetBootParam(SageLogBufferOffset, val);
    if(val!=pBootSettings->logBufferOffset)
    {
        BDBG_ERR(("%s - log buffer offset cannot change", BSTD_FUNCTION));
        goto end;
    }
    _BSAGElib_P_Boot_GetBootParam(SageLogBufferSize, val);
    if(val!=pBootSettings->logBufferSize)
    {
        BDBG_ERR(("%s - log buffer size cannot change", BSTD_FUNCTION));
        goto end;
    }

    _BSAGElib_P_Boot_GetBootParam(SageVklMask, val);
#if (BHSM_API_VERSION==1)
    if(val!=(uint32_t)((1 << BHSM_RemapVklId(hSAGElib->vkl1)) | (1 << BHSM_RemapVklId(hSAGElib->vkl2))))
    {
        BDBG_ERR(("%s - VKL info cannot change", BSTD_FUNCTION));
        goto end;
    }
#else
     /* SAGE Services parameters - resources */
    {
        uint32_t sageVklMask;
        BHSM_KeyLadderInfo info;

        info.index = 32; /* shift past uint32 if no return value */
        BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle1, &info);
        sageVklMask = 1<<info.index;

        info.index = 32;
        BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle2, &info);
        sageVklMask |= 1<<info.index;

        if(val != sageVklMask)
        {
            BDBG_ERR(("%s - VKL info cannot change", BSTD_FUNCTION));
            goto end;
        }
   }
#endif
    _BSAGElib_P_Boot_GetBootParam(SageDmaChannel, val);
    if(val!=0)
    {
        BDBG_ERR(("%s - dma channel cannot change", BSTD_FUNCTION));
        goto end;
    }

    /* Can check some region settings */
    _BSAGElib_P_Boot_GetBootParam(CRRStartOffset, local_region[1].offset);
    _BSAGElib_P_Boot_GetBootParam(CRREndOffset, local_region[1].size);

    /* translate size and offset per Zeus architecture */
    local_region[1].size = RESTRICTED_REGION_SIZE(local_region[1].offset,local_region[1].size);
    local_region[1].offset = RESTRICTED_REGION_OFFSET(local_region[1].offset);
    local_region[1].id=BSAGElib_RegionId_Crr;

    _BSAGElib_P_Boot_GetBootParam(SRRStartOffset, local_region[2].offset);
    _BSAGElib_P_Boot_GetBootParam(SRREndOffset, local_region[2].size);

    /* translate size and offset per Zeus architecture */
    local_region[2].size = RESTRICTED_REGION_SIZE(local_region[2].offset,local_region[2].size);
    local_region[2].offset = RESTRICTED_REGION_OFFSET(local_region[2].offset);
    local_region[2].id=BSAGElib_RegionId_Srr;

    for(i=0;i<pBootSettings->regionMapNum;i++)
    {
        switch(pBootSettings->pRegionMap[i].id)
        {
            case BSAGElib_RegionId_Glr:
                if(BKNI_Memcmp(&pBootSettings->pRegionMap[i], &local_region[0], sizeof(local_region[0]))!=0)
                {
                    BDBG_ERR(("%s - GLR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            case BSAGElib_RegionId_Crr:
               if(BKNI_Memcmp(&pBootSettings->pRegionMap[i], &local_region[1], sizeof(local_region[1]))!=0)
                {
                    BDBG_ERR(("%s - CRR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            case BSAGElib_RegionId_Srr:
               if(BKNI_Memcmp(&pBootSettings->pRegionMap[i], &local_region[2], sizeof(local_region[2]))!=0)
                {
                    BDBG_ERR(("%s - SRR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            default:
                break;
        }
    }

    /* Parse SAGE Framework */
    /* Needed for THL check */
    frameworkHolder.name = "Framework image";
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, ctx, pBootSettings->pFramework, pBootSettings->frameworkSize, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* If here, then things are ok */
    rc = BERR_SUCCESS;

end:
    if (ctx != NULL) {
        BKNI_Free(ctx);
    }

    BDBG_LEAVE(BSAGElib_Boot_HostReset);

    return rc;
}

void
BSAGElib_Boot_GetDefaultSettings(
    BSAGElib_BootSettings *pBootSettings /* [in/out] */)
{
    BDBG_ENTER(BSAGElib_Boot_GetDefaultSettings);

    BDBG_ASSERT(pBootSettings);
    BKNI_Memset(pBootSettings, 0, sizeof(*pBootSettings));

    BDBG_LEAVE(BSAGElib_Boot_GetDefaultSettings);
}

BERR_Code
BSAGElib_Boot_Launch(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageImageHolder blHolder;
    BSAGElib_SageImageHolder frameworkHolder;
    BSAGElib_P_BootContext *ctx = NULL;

    BDBG_ENTER(BSAGElib_Boot_Launch);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pBootSettings);

    /* Validate boot settings */
    if ((pBootSettings->pBootloader == NULL) ||
        (pBootSettings->bootloaderSize == 0) ||
        (pBootSettings->pFramework == NULL)      ||
        (pBootSettings->frameworkSize == 0)   ||
        ((pBootSettings-> pRegionMap == NULL)  &&  (pBootSettings-> regionMapNum != 0)) ) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", BSTD_FUNCTION));
        goto end;
    }

    if (hSAGElib->core_handles.hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", BSTD_FUNCTION));
        goto end;
    }

    /* Check SRR location  */
    rc = BSAGElib_P_Boot_CheckSRR(hSAGElib, pBootSettings);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("***********************************************************************"));
        BDBG_ERR(("BOLT and Nexus do not agree on SAGE heap location"));
        BDBG_ERR(("SAGE load can not continue"));
        BDBG_ERR(("***********************************************************************"));
        goto end;
    }

    rc = BSAGElib_P_Boot_SUIF(hSAGElib,pBootSettings);
    if(rc != BERR_NOT_SUPPORTED)
    {
        /* when a BERR_NOT_SUPPORTED is returned, it mean the SBL or SSF is not SUIF format,
         * we should keep on going to boot non-SUIF image,
         * or we successfully booted SAGE, or in error, we can return in these cases
         */
        goto end;
    }

    ctx = BKNI_Malloc(sizeof(*ctx));
    if (ctx == NULL) {
        BDBG_ERR(("%s - Cannot allocate temporary context for SAGE Boot.", BSTD_FUNCTION));
        goto end;
    }
    BKNI_Memset(ctx, 0, sizeof(*ctx));
    ctx->hSAGElib = hSAGElib;

    /* Get SAGE Secureboot OTP MSPs value */
    rc = BSAGElib_P_Boot_GetSageOtpMspParams(ctx);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_GetSageOtpMspParams() fails", BSTD_FUNCTION));
        goto end;
    }

    /* Parse SAGE bootloader */
    blHolder.name = "Bootloader";
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, ctx, pBootSettings->pBootloader,
                                        pBootSettings->bootloaderSize, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE bootloader compatibility with current chipset */
    rc = BSAGElib_P_Boot_CheckCompatibility(hSAGElib, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGElib->i_memory_sync.flush(pBootSettings->pBootloader, pBootSettings->bootloaderSize);

    /* Parse SAGE Framework */
    frameworkHolder.name = "Framework image";
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, ctx, pBootSettings->pFramework, pBootSettings->frameworkSize, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE Framework compatibility with current chipset */
    rc = BSAGElib_P_Boot_CheckCompatibility(hSAGElib, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGElib->i_memory_sync.flush(pBootSettings->pFramework, pBootSettings->frameworkSize);

    /* Check SAGE Framework compatibility with BFW version */
    rc = BSAGElib_P_Boot_CheckFrameworkBFWVersion(hSAGElib, &frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("***************************************************************************"));
        BDBG_ERR(("BOLT with BFW version %d.%d.%d or later must be used with SAGE Framework %d.x",
            BFW_VERSION_CHECK_MAJOR, BFW_VERSION_CHECK_MINOR, BFW_VERSION_CHECK_SUBMINOR,
            frameworkHolder.header->ucSageImageVersion[0]));
        BDBG_ERR(("Please upgrade BOLT."));
        BDBG_ERR(("***************************************************************************"));
        goto end;
    }

    /* Check if both SAGE images have triple signing mode
    rc = BSAGElib_P_Boot_CheckSigningMode(ctx);
    if(rc != BERR_SUCCESS) { goto end; } */

    /* Check SBL B/C compatability */
    BSAGElib_P_Boot_CheckFrameworkKeys(ctx, &frameworkHolder);

    /* push region map to DRAM */
    if(pBootSettings->regionMapNum != 0) {
        hSAGElib->i_memory_sync.flush(pBootSettings-> pRegionMap, pBootSettings->regionMapNum * sizeof(BSAGElib_RegionInfo));
    }

    /* Set SAGE boot parameters information into Global SRAM GP registers */
    rc = BSAGElib_P_Boot_SetBootParams(ctx, pBootSettings, &frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_SetBootParams() fails", BSTD_FUNCTION));
        goto end;
    }

    /* Take SAGE out of reset */
    rc = BSAGElib_P_Boot_ResetSage(ctx, &blHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_ResetSage() failed to launch SAGE Bootloader [0x%x]", BSTD_FUNCTION, rc));
        goto end;
    }

end:

    if (ctx != NULL) {
        BKNI_Free(ctx);
    }

    BDBG_LEAVE(BSAGElib_Boot_Launch);
    return rc;
}

void
BSAGElib_Boot_GetState(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootState *pState /* [out] */)
{
    uint32_t addr;

    BDBG_ENTER(BSAGElib_Boot_GetState);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pState);

    /* Retrieve status */
    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBootStatus);
    pState->status = BREG_Read32(hSAGElib->core_handles.hReg, addr);

    /* Retrieve last error */
    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eLastError);
    pState->lastError = BREG_Read32(hSAGElib->core_handles.hReg, addr);

    BDBG_LEAVE(BSAGElib_Boot_GetState);
    return;
}

void
BSAGElib_Boot_Clean(
    BSAGElib_Handle hSAGElib)
{
    BDBG_ENTER(BSAGElib_Boot_Clean);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    /* wipe associated registers */
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBin, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSize, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSignature, 0);
    _BSAGElib_P_Boot_SetBootParam(TextSectionSize, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, 0);

    BDBG_LEAVE(BSAGElib_Boot_Clean);
    return;
}

void
BSAGElib_Boot_GetBinariesInfo(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ImageInfo *pBootloaderInfo,
    BSAGElib_ImageInfo *pFrameworkInfo)
{
    BDBG_ENTER(BSAGElib_Boot_GetBinariesInfo);
    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    if (pBootloaderInfo != NULL) {
        *pBootloaderInfo = hSAGElib->bootloaderInfo;
    }

    if (pFrameworkInfo != NULL) {
        *pFrameworkInfo = hSAGElib->frameworkInfo;
    }

    BDBG_LEAVE(BSAGElib_Boot_GetBinariesInfo);
}

BERR_Code
BSAGElib_Boot_Post(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc;

    BDBG_ENTER(BSAGElib_Boot_Post);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
#if (BHSM_API_VERSION==1)
    BSAGElib_iLockHsm();
    rc = BHSM_InitialiseBypassKeyslots(hSAGElib->core_handles.hHsm);
    BSAGElib_iUnlockHsm();
#else
    rc = BERR_SUCCESS;
    BDBG_LOG(("%s TBD BHSM_API_VERSION=2 post boot actions", BSTD_FUNCTION));
#endif
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BHSM_InitialiseBypassKeyslots() fails %d", BSTD_FUNCTION, rc));
        goto end;
    }

    hSAGElib->bBootPostCalled = true;

end:
    BDBG_LEAVE(BSAGElib_Boot_Post);
    return rc;

}
