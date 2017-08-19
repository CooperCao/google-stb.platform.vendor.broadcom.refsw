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

#include "bsagelib.h"
#include "bsagelib_boot.h"
#include "bsage.h"
#include "bsage_priv.h"
#include "priv/bsagelib_shared_types.h"

#include "bhsm.h"
#include "bhsm_keyladder.h"
#include "bhsm_keyladder_enc.h"
#include "bhsm_bseck.h"
#include "bhsm_verify_reg.h"
#include "bhsm_otpmsp.h"
#include "bhsm_misc.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bsp_s_hw.h"
#include "bsp_s_download.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"
#include "bsp_s_mem_auth.h"

#include "bchp_common.h"
#include "bchp_bsp_glb_control.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(BSAGE);

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
    BCMD_VKLID_e vklBoot;
    uint32_t otp_sage_decrypt_enable;
    uint32_t otp_sage_verify_enable;
    uint32_t otp_sage_secure_enable;
    uint32_t otp_system_epoch0;
    uint32_t otp_system_epoch3;
    uint32_t otp_market_id;
    uint32_t otp_market_id1;

    bool sageBlTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageFrameworkTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageProductionKey;  /* triple-signing's 2ndTierKey can be development key or production key. True for prodction key */
    /* Secure Image version */
    uint8_t sageBlSecureVersion;
    uint8_t sageFrameworkSecureVersion;

} BSAGElib_P_BootContext;

/* TODO: use the BHSM_SecondTierKey_t struct in HSM instead of this one */
typedef struct
{
    uint8_t     ucKeyData[256];
    uint8_t     ucRights;       /* 0 => MIPS; 2 => AVD, RAPTOR, RAVE; 4 => BSP; 8 => Boot; 10 => SAGE */
    uint8_t     ucReserved0;
    uint8_t     ucPubExponent;  /* 0 => 3; 1 => 64K+1 */
    uint8_t     ucReserved1;
    uint8_t     ucMarketID[4];
    uint8_t     ucMarketIDMask[4];
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    unsigned char ucEpoch;
    unsigned char ucEpochMask;
    unsigned char ucEpochSelect;
    unsigned char ucMarketIDSelect;
    unsigned char ucSignatureVersion;
    unsigned char ucSignatureType;
    unsigned char ucReserved3[2];
#else
    unsigned char usReserved2[2];
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
#endif
    uint8_t     ucSignature[256];
} BCMD_SecondTierKey_t;


typedef struct
{
    unsigned char ucReserved0[2];
    unsigned char ucCpuType;
    unsigned char ucNoReloc;
    unsigned char ucMarketId[4];
    unsigned char ucMarketIdMask[4];
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    unsigned char ucReserved1;
    unsigned char ucEpochSelect;
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
    unsigned char ucSignatureVersion;
    unsigned char ucSignatureType;
    unsigned char ucReserved2[2];
#else
    unsigned char ucReserved1[2];
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
#endif
}BSAGElib_ControllingParams;


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
    unsigned char ucReserved1[28];                     /* Reserved for future usage */
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
    unsigned char ucReserved1[28];                     /* Reserved for future usage */
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
    unsigned char ucReserved1[28];                     /* Reserved for future usage */
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
    unsigned char ucReserved1[28];                     /* Reserved for future usage */
    BSAGElib_ControllingParams ucControllingParameters;/* SAGE Image Header controlling Parameters */
    unsigned char ucHeaderSignature[256];              /* Header signature */
    unsigned char ucHeaderSignatureP[256];              /* Header signature */
    BCMD_SecondTierKey_t second_tier_key;              /* Zeus 3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
    BCMD_SecondTierKey_t second_tier_key2;  /* triple signing scheme */
    BCMD_SecondTierKey_t second_tier_key3;  /* triple signing scheme */
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
        BREG_Write32(hSAGE->hReg, addr, VAL);                   \
        BootParamDbgPrintf(("%s - Read %s - Addr: 0x%08x Value: 0x%08x", BSTD_FUNCTION, #REGID, addr, BREG_Read32(hSAGE->hReg, (uint32_t)addr))); }
#define _BSAGElib_P_Boot_GetBootParam(REGID, VAR) {      \
        uint32_t addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_e##REGID); \
        VAR=BREG_Read32(hSAGE->hReg, addr); }


#define _Swap16(PVAL) (((PVAL)[0] << 8) | ((PVAL)[1]))
#define _ToDWORD(PVAL) (((PVAL)[3] << 24) | ((PVAL)[2] << 16) | ((PVAL)[1] << 8) |((PVAL)[0] << 0))

#define OTP_SWIZZLE0A_MSP0_VALUE_ZS (0x02)
#define OTP_SWIZZLE0A_MSP1_VALUE_ZS (0x02)
#define OTP_SWIZZLE0A_MSP0_VALUE_ZB (0x3E)
#define OTP_SWIZZLE0A_MSP1_VALUE_ZB (0x3F)
#define OTP_SWIZZLE0A_MSP0_VALUE_CUST1 (0x03)
#define OTP_SWIZZLE0A_MSP1_VALUE_CUST1 (OTP_SWIZZLE0A_MSP0_VALUE_CUST1)

/* SAGE boot timing */
#define SAGE_MAX_BOOT_TIME_US (15 * 1000 * 1000)
#define SAGE_STEP_BOOT_TIME_US (50 * 1000)

/* SAGE ARC Prescreen error codes */
#define SAGE_SECUREBOOT_ARC_PRESCREEN_NUMBER_BAD_BIT_EXCEEDED  (0x60D)
#define SAGE_SECUREBOOT_ARC_PRESCREEN_MSP_PROG_FAILURE         (0x60E)

/****************************************
 * Local Functions
 ***************************************/
static BERR_Code BSAGE_P_MonitorBoot(BSAGE_Handle hSAGE,BHSM_Handle hHsm);
static BERR_Code BSAGE_P_Boot_GetSageOtpMspParams(BSAGE_Handle hSAGE,BHSM_Handle hHsm,BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGE_P_Boot_CheckCompatibility(BSAGE_Handle hSAGE, BSAGElib_SageImageHolder *img);
static BERR_Code BSAGE_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGE_P_Boot_ParseSageImage(BSAGE_Handle hSAGE, BSAGElib_P_BootContext *ctx, uint8_t *pBinary, uint32_t binarySize, BSAGElib_SageImageHolder* holder);
static BERR_Code BSAGE_P_Boot_SetBootParams(BSAGElib_P_BootContext *ctx, BSAGE_BootSettings *pBootSettings, BSAGElib_SageImageHolder* frameworkHolder);
static BERR_Code BSAGE_P_Boot_ResetSage(BSAGE_Handle hSAGE,BHSM_Handle hHsm,BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *bl_img);
static BERR_Code BSAGE_P_Boot_CheckFrameworkBFWVersion(BHSM_Handle hHsm,BSAGElib_SageImageHolder *img);

static BCMD_SecondTierKey_t * BSAGElib_P_Boot_GetKey(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder* image);
static uint8_t *BSAGElib_P_Boot_GetSignature(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *image);
static void BSAGElib_P_Boot_SetImageInfo(BSAGElib_ImageInfo *pImageInfo, BSAGElib_SageImageHolder* holder, uint32_t header_version, uint32_t type, bool triple_sign);

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

    if(otp_market_id != key_market_id)
    {
        BDBG_WRN(("%s - Key's market id %x do not match with otp market id %x (key market id select %d, mask %x, otp_market id %x, otp market id1 %x", BSTD_FUNCTION,key_market_id,otp_market_id,pKey->ucMarketIDSelect,key_market_id_mask,ctx->otp_market_id,ctx->otp_market_id1));
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
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key) - (uint8_t *)triple_sign_header),
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key2) - (uint8_t *)triple_sign_header),
            (uint32_t)((uint8_t *)(&triple_sign_header->second_tier_key3) - (uint8_t *)triple_sign_header)));

        ctx->sageProductionKey = false;

        if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key) == BERR_SUCCESS)
            ret = &triple_sign_header->second_tier_key;
        else if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key2) == BERR_SUCCESS)
            ret = &triple_sign_header->second_tier_key2;
        else if(BSAGElib_P_Boot_CheckMarketId(ctx,&triple_sign_header->second_tier_key3) == BERR_SUCCESS)
        {
            ctx->sageProductionKey = true; /* this is production key */
            ret = &triple_sign_header->second_tier_key3;
        }
        else {
            BDBG_ERR(("%s - This chip type does not have valid second tier key.", BSTD_FUNCTION));
        }

    }
    else if(image->header->ucHeaderVersion == 0x0a && image->header->ucHeaderIndex[0] == 0x53 && image->header->ucHeaderIndex[1] == 0x57)
    {    /* it's SAGE 3.2 bootloader */
		BSAGElib_BootloaderHeader *pBootLoader = (BSAGElib_BootloaderHeader *)image->header;
        ret = &pBootLoader->second_tier_key;
	}
    else {
        ret = &image->header->second_tier_key;
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

    if(ctx->sageProductionKey) /* for triple signed SBL, 'signature' point to develoment Key signature, */
        signature += _SIG_SZ;  /* production key signature is right after */

    return signature;
}

static uint32_t
BSAGElib_P_Boot_GetHeaderSize(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    uint32_t index = _Swap16(image->header->ucHeaderIndex);
    uint32_t sig_size;
    bool triple = false;
    uint32_t header_size = 0;

    if (index == SAGE_HEADER_TYPE_BL) {
        sig_size = _SIG_SZ;
        if (ctx->sageBlTripleSigning) {
            triple = true;
        }
    }
    else {
        sig_size = _SIG_SZ_SAGE_FRAMEWORK;
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
BSAGE_P_Boot_GetSageOtpMspParams(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm,
    BSAGElib_P_BootContext *ctx)
{
    BERR_Code rc = BERR_SUCCESS;

    /* Pull OTPs:
     * BCMD_Otp_CmdMsp_eReserved210 : SAGE decryption enabled
     * BCMD_Otp_CmdMsp_eReserved209 : SAGE verification enabled
     * BCMD_Otp_CmdMsp_eReserved212 : SAGE secure enabled
     * BCMD_Otp_CmdMsp_eMarketId : market Id
     * BCMD_Otp_CmdMsp_eSystemEpoch // BCMD_Otp_CmdMsp_eReserved87 : Epoch */

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved210, &ctx->otp_sage_decrypt_enable, "decrypt_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved209, &ctx->otp_sage_verify_enable, "verify_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved212, &ctx->otp_sage_secure_enable, "secure_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eMarketId, &ctx->otp_market_id, "market id");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_GetOtp(hSAGE, hHsm, BCMD_Otp_CmdMsp_eMarketId1, &ctx->otp_market_id1, "market id1");
    if (rc != BERR_SUCCESS) { goto end; }

#if (ZEUS_VERSION < ZEUS_4_1)
    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eSystemEpoch, &ctx->otp_system_epoch0, "system epoch 0");
#else
    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved87, &ctx->otp_system_epoch0, "epoch");
    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eSystemEpoch3, &ctx->otp_system_epoch3, "system epoch 3");
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

end:
    return rc;
}

static BERR_Code
BSAGE_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx)
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

#define SAGE_FRAMEWORK_VERSION_CHECK 3
#define BFW_VERSION_CHECK_MAJOR 4
#define BFW_VERSION_CHECK_MINOR 1
#define BFW_VERSION_CHECK_SUBMINOR 3

static BERR_Code
BSAGE_P_Boot_CheckFrameworkBFWVersion(
    BHSM_Handle hHsm,
    BSAGElib_SageImageHolder *image)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)image->header;
    BHSM_Capabilities_t hsmCaps;

    BDBG_MSG(("SAGE Framework version major %d",header->ucSageImageVersion[0]));
    if (header->ucSageImageVersion[0] >= SAGE_FRAMEWORK_VERSION_CHECK)
    {
        if( ( rc = BHSM_GetCapabilities(hHsm, &hsmCaps ) ) != BERR_SUCCESS )
        {
            BDBG_ERR(("couldn't read BFW version"));
            return rc;
        }
        else
        {
            BDBG_MSG(("BFW %d %d %d",hsmCaps.version.firmware.bseck.major, hsmCaps.version.firmware.bseck.minor, hsmCaps.version.firmware.bseck.subMinor));
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
BSAGE_P_Boot_CheckCompatibility(
    BSAGE_Handle hSAGE,
    BSAGElib_SageImageHolder *img)
{
    BERR_Code rc = BERR_SUCCESS;

    switch(hSAGE->chipInfo.chipType)
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
        subTypeStr = "GENERIC";
        BDBG_MSG(("%s - '%s' Detected [type=%s, header_version=%u, triple_sign=%u]",
                  BSTD_FUNCTION, holder->name, typeStr, header_version, triple_sign));
        pImageInfo->THLShortSig = 0;
    }
    else {
        typeStr = "Framework";
        switch (header->ucSageImageBinaryType) {
             case 0:
                 subTypeStr = "GENERIC";
                 break;
             case 1:
                 subTypeStr = "MANUFACTURING TOOL";
                 break;
            case 2:
                 subTypeStr = "CUSTOM";
                 break;
            case 3:
                 subTypeStr = "SPECIAL";
                 break;
            default:
                 subTypeStr = "UNKNOWN";
                 break;
        }
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
            if(header_version == 0x0a)
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

    version = pImageInfo->version[0] | (pImageInfo->version[1] << 8) |
             (pImageInfo->version[2] << 16) | (pImageInfo->version[3] << 24);
    if (version == 0) {
        BKNI_Snprintf(verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING,
                      "SAGE %s does not contain SAGE versioning information", typeStr);
        BDBG_WRN(("%s", verStr));
    }
    else {
        BKNI_Snprintf (verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING,
                       "SAGE %s Version [%s=%u.%u.%u.%u, Signing Tool=%u.%u.%u.%u]",
                       typeStr,
                       subTypeStr,
                       pImageInfo->version[0],
                       pImageInfo->version[1],
                       pImageInfo->version[2],
                       pImageInfo->version[3],
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
    BSAGE_Handle hSAGE,
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

    familyId = BREG_Read32(hSAGE->hReg, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    productId = BREG_Read32(hSAGE->hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
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

    BDBG_MSG(("%s %d Device Blob sectionNum %d chipId %x (productId %x familyId %x) chipType %d",BSTD_FUNCTION,__LINE__,sectionNum,chipId,productId,familyId,hSAGE->chipInfo.chipType));

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

        BDBG_MSG(("%s %d i %d count %d, ulDeviceTreeSize %x ChipID %x type %d",BSTD_FUNCTION,__LINE__,i,count,ulDeviceTreeSize,ulChipId,chipType));

        if(/*ulChipId == chipId && chipType == hSAGE->chipInfo.chipType &&*/ *ppFrameworkHeader == NULL)
        {
            *ppFrameworkHeader = pFramework;
        }
        count += ulDeviceTreeSize + sizeof( BSAGElib_FrameworkHeader) + sizeof(BSAGElib_DeviceBottom);
    }
err:
    return count;
}

static BERR_Code
BSAGE_P_Boot_ParseSageImage(
    BSAGE_Handle hSAGE,
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
           deviceBlobSize = getDeviceSectionSize(hSAGE,pBlob,&pFramework);
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

           /* move binary up, overwrite unused, other chips' section */
           pBottom = BKNI_Malloc(sizeof(BSAGElib_DeviceBottom));
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
           BSAGElib_P_Boot_SetImageInfo(&hSAGE->frameworkInfo,
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

       if(header_version == 0x0a && image_type == SAGE_HEADER_TYPE_BL)
       { /* SAGE 3.2 boot loader */
           holder->data = raw_ptr;
           holder->data_len = raw_remain - all_sig_size - 20;
           raw_ptr += holder->data_len + 20;
           raw_remain = all_sig_size;
           holder->signature = raw_ptr;
           BSAGElib_P_Boot_SetImageInfo(&hSAGE->bootloaderInfo,
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
               BSAGElib_P_Boot_SetImageInfo(&hSAGE->bootloaderInfo,
                                            holder, header_version, image_type, triple);
                ctx->sageBlSecureVersion = header_version;
                ctx->sageBlTripleSigning = triple;
               break;
           case SAGE_HEADER_TYPE_FRAMEWORK:
               BSAGElib_P_Boot_SetImageInfo(&hSAGE->frameworkInfo,
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
BSAGE_P_Boot_SetBootParams(
    BSAGElib_P_BootContext *ctx,
    BSAGE_BootSettings *pBootSettings,
    BSAGElib_SageImageHolder* frameworkHolder)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t offset = 0;
    uint32_t header_size;
    BSAGE_Handle hSAGE = hSAGE_Global;

    /* SAGE < -- > Host communication buffers */
    if(pBootSettings->HSIBufferOffset != 0)
        offset = pBootSettings->HSIBufferOffset;
    else
        offset = hSAGE->i_memory_map.addr_to_offset(hSAGE->hsi_buffers);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert HSI buffer address to offset", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    _BSAGElib_P_Boot_SetBootParam(HostSageBuffers,     offset);
    _BSAGElib_P_Boot_SetBootParam(HostSageBuffersSize, SAGE_HOST_BUF_SIZE*4);

    /* Regions configuration */
    _BSAGElib_P_Boot_SetBootParam(RegionMapOffset, pBootSettings->regionMapOffset);
    _BSAGElib_P_Boot_SetBootParam(RegionMapSize, pBootSettings->regionMapNum * sizeof(BSAGElib_RegionInfo));

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

    /* SAGE Services parameters - resources */
    {
        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        uint32_t sageVklMask = ((1 << BHSM_RemapVklId(hSAGE->vkl1)) | (1 << BHSM_RemapVklId(hSAGE->vkl2)));
        _BSAGElib_P_Boot_SetBootParam(SageVklMask, sageVklMask);
    }
    _BSAGElib_P_Boot_SetBootParam(SageDmaChannel, 0);

    /* SAGE Secure Boot */
    offset = hSAGE->i_memory_map.addr_to_offset(frameworkHolder->data);
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
            offset = hSAGE->i_memory_map.addr_to_offset(signature);
            if(offset == 0) {
                BDBG_ERR(("%s - Cannot convert SAGE Framework signature address to offset", BSTD_FUNCTION));
                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
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
            if(header_version == 0x0a)
            { /* SAGE 3.2 framework */
                data = (pSage32Header->ucInstrSectionSize[3] << 0) | (pSage32Header->ucInstrSectionSize[2] << 8) | (pSage32Header->ucInstrSectionSize[1] << 16) | (pSage32Header->ucInstrSectionSize[0] << 24);
            }else{
              /* SAGE 3.1 framework */
                data = (single_sign_header->ucInstrSectionSize[3] << 0) | (single_sign_header->ucInstrSectionSize[2] << 8) | (single_sign_header->ucInstrSectionSize[1] << 16) | (single_sign_header->ucInstrSectionSize[0] << 24);
            }
        }
        _BSAGElib_P_Boot_SetBootParam(TextSectionSize, data);

        offset = hSAGE->i_memory_map.addr_to_offset(frameworkHolder->header);
        if(offset == 0) {
            BDBG_ERR(("%s - Cannot convert SAGE framework header address to offset", BSTD_FUNCTION));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
        header_size = BSAGElib_P_Boot_GetHeaderSize(ctx, frameworkHolder);
        hSAGE->i_memory_sync.flush(frameworkHolder->header, header_size);
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, offset);
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, header_size);
    }
end:
    return rc;
}

static BERR_Code
BSAGE_P_Boot_ResetSage(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm,
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *blImg)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeaderCommon *header = blImg->header;
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
        secondTierKey.keyAddr = hSAGE->i_memory_map.addr_to_offset(pSecondTierKey);
        if(secondTierKey.keyAddr == 0) {
            BDBG_ERR(("%s - Cannot convert 2nd-tier key address to offset", BSTD_FUNCTION));
            goto end;
        }
        hSAGE->i_memory_sync.flush(pSecondTierKey, sizeof(*pSecondTierKey));
        hSAGE->i_sync_hsm.lock();
        rc = BHSM_VerifySecondTierKey(hHsm,  &secondTierKey);
        hSAGE->i_sync_hsm.unlock();
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - BHSM_VerifySecondTierKey() fails %d", BSTD_FUNCTION, rc));
            goto end;
        }
    }

    vkl_id = ctx->vklBoot;
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
            generateRouteKeyIO.askm.configurationEnable    = true;
            generateRouteKeyIO.cusKeySwizzle0aEnable       = true;
            generateRouteKeyIO.askm.moduleId                = header->ucModuleId;
            generateRouteKeyIO.askm.stbOwnerId              = BCMD_STBOwnerID_eOneVal;
            generateRouteKeyIO.askm.caVendorId              = (header->ucCaVendorId[3] << 24) |
                                                             (header->ucCaVendorId[2] << 16) |
                                                             (header->ucCaVendorId[1] << 8) |
                                                             (header->ucCaVendorId[0]);
            generateRouteKeyIO.askm.maskKeySelect           = BCMD_ASKM_MaskKeySel_eRealMaskKey;
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

            hSAGE->i_sync_hsm.lock();
            rc = BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
            hSAGE->i_sync_hsm.unlock();
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

            hSAGE->i_sync_hsm.lock();
            rc = BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
            hSAGE->i_sync_hsm.unlock();
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

            hSAGE->i_sync_hsm.lock();
            rc = BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
            hSAGE->i_sync_hsm.unlock();
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

        verifyIO.region.startAddress = hSAGE->i_memory_map.addr_to_offset(blImg->data);
        if(verifyIO.region.startAddress == 0) {
            BDBG_ERR(("%s - Cannot convert bootloader address to offset", BSTD_FUNCTION));
            goto end;
        }
        verifyIO.region.endAddress = verifyIO.region.startAddress + (blImg->data_len) - 1;

        if (blImg->signature) {
            uint8_t *signature;
            signature = BSAGElib_P_Boot_GetSignature(ctx, blImg);
            verifyIO.signature.startAddress = hSAGE->i_memory_map.addr_to_offset(signature);
            if(verifyIO.signature.startAddress == 0) {
                BDBG_ERR(("%s - Cannot convert bootloader signature address to offset", BSTD_FUNCTION));
                goto end;
            }
            hSAGE->i_memory_sync.flush(signature, 256);
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

        hSAGE->i_sync_hsm.lock();
        rc = BHSM_RegionVerification_Configure(hHsm, 0x18, &verifyIO);
        hSAGE->i_sync_hsm.unlock();
        if ( rc != 0 ) {
            BDBG_ERR(("\nBHSM_RegionVerification_Configure fails - region = 0x%x, status = %x\n", 0x18, rc));
            goto end;
        }

        hSAGE->i_sync_hsm.lock();
        rc = BHSM_RegionVerification_Enable(hHsm);
        hSAGE->i_sync_hsm.unlock();
        if ( rc != 0) {
            BDBG_ERR(("\nBHSM_RegionVerification_Enable fails - status = %x\n", rc));
            goto end;
        }

        do
        {
            hSAGE->i_sync_hsm.lock();
            rc = BHSM_RegionVerification_Status(hHsm, 0x18, &status);
            hSAGE->i_sync_hsm.unlock();
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

    return rc;
}

static BERR_Code
BSAGE_P_GetChipsetType(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t otp_swizzle0a_msp0;
    uint32_t otp_swizzle0a_msp1;

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved233, &otp_swizzle0a_msp0, "msp0");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_GetOtp(hSAGE,hHsm, BCMD_Otp_CmdMsp_eReserved234, &otp_swizzle0a_msp1, "msp1");
    if (rc != BERR_SUCCESS) { goto end; }

    BDBG_MSG(("%s - OTP [MSP0: %d, MSP1: %d]",
              BSTD_FUNCTION, otp_swizzle0a_msp0, otp_swizzle0a_msp1));

    if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_ZS) &&
       (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_ZS)) {
        BDBG_LOG(("%s - Chip Type: ZS", BSTD_FUNCTION));
        hSAGE->chipInfo.chipType = BSAGElib_ChipType_eZS;
    }
    else if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_ZB) &&
             (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_ZB)) {
        BDBG_LOG(("%s - Chip Type: ZB", BSTD_FUNCTION));
        hSAGE->chipInfo.chipType = BSAGElib_ChipType_eZB;
    }
    else if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_CUST1) &&
             (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_CUST1)) {
        BDBG_LOG(("%s - Chip Type: Customer1", BSTD_FUNCTION));
        hSAGE->chipInfo.chipType = BSAGElib_ChipType_eCustomer1;
    }
    else
    {
        BDBG_LOG(("%s - Chip Type: Customer specific chip", BSTD_FUNCTION));
        hSAGE->chipInfo.chipType = BSAGElib_ChipType_eCustomer;
    }

end:
    return rc;
}

/* Host (nexus) has restarted... check/reset connection w/ SAGE */
/* To be called prior to any attempts to communicate w/ SAGE */
BERR_Code
BSAGE_P_Boot_HostReset(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm,
    const BSAGE_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_INVALID_PARAMETER;
    uint32_t offset = 0;
    uint32_t val;
    BSAGElib_SageImageHolder frameworkHolder;
    BSAGElib_P_BootContext *ctx = NULL;
    BSAGElib_RegionInfo local_region[3];
    uint8_t *pFramework;
    uint32_t frameworkSize;
    unsigned i;

    BDBG_ENTER(BSAGE_Boot_HostReset);

    if (!hSAGE) {
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", BSTD_FUNCTION));
        goto end;
    }

    BDBG_ASSERT(pBootSettings);

    /* Validate boot settings */
    if ((pBootSettings->bootloaderOffset == 0) ||
        (pBootSettings->bootloaderSize == 0) ||
        (pBootSettings->frameworkOffset == 0)  ||
        (pBootSettings->frameworkSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", BSTD_FUNCTION));
        goto end;
    }

    if (hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", BSTD_FUNCTION));
        goto end;
    }

    rc = BSAGE_Rpc_Init(pBootSettings->HSIBufferOffset);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGE_Rpc_Init failed.", BSTD_FUNCTION));
        goto end;
    }

    hSAGE->vkl1 = pBootSettings->vkl1;
    hSAGE->vkl2 = pBootSettings->vkl2;

    BSAGE_P_GetChipsetType(hSAGE,hHsm);

    ctx = BKNI_Malloc(sizeof(*ctx));
    if (ctx == NULL) {
        BDBG_ERR(("%s - Cannot allocate temporary context for SAGE Boot.", BSTD_FUNCTION));
        goto end;
    }
    BKNI_Memset(ctx, 0, sizeof(*ctx));

    /* Get SAGE Secureboot OTP MSPs value */
    rc = BSAGE_P_Boot_GetSageOtpMspParams(hSAGE,hHsm,ctx);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_GetSageOtpMspParams() fails", BSTD_FUNCTION));
        goto end;
    }

    rc = BERR_INVALID_PARAMETER;

    offset = hSAGE->i_memory_map.addr_to_offset(hSAGE->hsi_buffers);
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
    if(val!=(uint32_t)((1 << BHSM_RemapVklId(hSAGE->vkl1)) | (1 << BHSM_RemapVklId(hSAGE->vkl2))))
    {
        BDBG_ERR(("%s - VKL info cannot change", BSTD_FUNCTION));
        goto end;
    }

    _BSAGElib_P_Boot_GetBootParam(SageDmaChannel, val);
    if(val!=0)
    {
        BDBG_ERR(("%s - dma channel cannot change", BSTD_FUNCTION));
        goto end;
    }

    /* Can check some region settings */
    _BSAGElib_P_Boot_GetBootParam(CRRStartOffset, local_region[1].offset);
    _BSAGElib_P_Boot_GetBootParam(CRREndOffset, local_region[1].size);
    local_region[1].size=local_region[1].size-local_region[1].offset+0x8; /* HW uses 8byte alignment */;
    local_region[1].id=BSAGElib_RegionId_Crr;

    _BSAGElib_P_Boot_GetBootParam(SRRStartOffset, local_region[2].offset);
    _BSAGElib_P_Boot_GetBootParam(SRREndOffset, local_region[2].size);
    local_region[2].size=local_region[2].size-local_region[2].offset+0x8; /* HW uses 8byte alignment */
    local_region[2].id=BSAGElib_RegionId_Srr;

    for(i=0;i<pBootSettings->regionMapNum;i++)
    {
        BSAGElib_RegionInfo *pRegionMap = hSAGE->i_memory_map.offset_to_addr(pBootSettings-> regionMapOffset);
        switch(pRegionMap[i].id)
        {
            case BSAGElib_RegionId_Glr:
                if(BKNI_Memcmp(&pRegionMap[i], &local_region[0], sizeof(local_region[0]))!=0)
                {
                    BDBG_ERR(("%s - GLR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            case BSAGElib_RegionId_Crr:
                if(BKNI_Memcmp(&pRegionMap[i], &local_region[1], sizeof(local_region[1]))!=0)
                {
                    BDBG_ERR(("%s - CRR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            case BSAGElib_RegionId_Srr:
               if(BKNI_Memcmp(&pRegionMap[i], &local_region[2], sizeof(local_region[2]))!=0)
                {
                    BDBG_ERR(("%s - SRR region cannot change", BSTD_FUNCTION));
                    goto end;
                }
                break;
            default:
                break;
        }
    }

    if (hSAGE->chipInfo.chipType == BSAGElib_ChipType_eZS) {
        pFramework = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->frameworkDevOffset);
        frameworkSize = pBootSettings->frameworkDevSize;
    }else{
        pFramework = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->frameworkOffset);
        frameworkSize = pBootSettings->frameworkSize;
    }

    /* Parse SAGE Framework */
    /* Needed for THL check */
    frameworkHolder.name = "Framework image";
    rc = BSAGE_P_Boot_ParseSageImage(hSAGE, ctx, pFramework, frameworkSize, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* If here, then things are ok */
    rc = BERR_SUCCESS;

end:
    if (ctx != NULL) {
        BKNI_Free(ctx);
    }

    BDBG_LEAVE(BSAGE_Boot_HostReset);

    return rc;
}

BERR_Code
BSAGE_Boot_Launch(
    BHSM_Handle hHsm,
    BSAGE_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageImageHolder blHolder;
    BSAGElib_SageImageHolder frameworkHolder;
    uint8_t *pBootloader,*pFramework;
    uint32_t bootloaderSize,frameworkSize;
    BSAGElib_P_BootContext *ctx = NULL;
    BSAGE_Handle hSAGE = hSAGE_Global;

    BDBG_ENTER(BSAGE_Boot_Launch);

    if (!hSAGE) {
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", BSTD_FUNCTION));
        goto end;
    }

    BDBG_ASSERT(pBootSettings);

    /* Validate boot settings */
    if ((pBootSettings->bootloaderOffset == 0) ||
        (pBootSettings->bootloaderSize == 0) ||
        (pBootSettings->frameworkOffset == 0)  ||
        (pBootSettings->frameworkSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", BSTD_FUNCTION));
        goto end;
    }

    if (hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", BSTD_FUNCTION));
        goto end;
    }

    hSAGE->vkl1 = pBootSettings->vkl1;
    hSAGE->vkl2 = pBootSettings->vkl2;

    BSAGE_P_GetChipsetType(hSAGE,hHsm);

    if(!(BREG_Read32(hSAGE->hReg, BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT)
        & BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT_SCPU_SW_INIT_MASK))
    {
        rc = BSAGE_P_Boot_HostReset(hSAGE,hHsm,pBootSettings);
        goto end;
    }

    ctx = BKNI_Malloc(sizeof(*ctx));
    if (ctx == NULL) {
        BDBG_ERR(("%s - Cannot allocate temporary context for SAGE Boot.", BSTD_FUNCTION));
        goto end;
    }
    BKNI_Memset(ctx, 0, sizeof(*ctx));

    /* Get SAGE Secureboot OTP MSPs value */
    rc = BSAGE_P_Boot_GetSageOtpMspParams(hSAGE,hHsm,ctx);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_GetSageOtpMspParams() fails", BSTD_FUNCTION));
        goto end;
    }

    if (hSAGE->chipInfo.chipType == BSAGElib_ChipType_eZS) {
        pBootloader = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->bootloaderDevOffset);
        pFramework = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->frameworkDevOffset);
        bootloaderSize = pBootSettings->bootloaderDevSize;
        frameworkSize = pBootSettings->frameworkDevSize;
    }else{
        pBootloader = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->bootloaderOffset);
        pFramework = (uint8_t *)hSAGE->i_memory_map.offset_to_addr(pBootSettings->frameworkOffset);
        bootloaderSize = pBootSettings->bootloaderSize;
        frameworkSize = pBootSettings->frameworkSize;
    }

    /* Parse SAGE bootloader */
    blHolder.name = "Bootloader";
    rc = BSAGE_P_Boot_ParseSageImage(hSAGE, ctx, pBootloader,
                                        bootloaderSize, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE bootloader compatibility with current chipset */
    rc = BSAGE_P_Boot_CheckCompatibility(hSAGE, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGE->i_memory_sync.flush(pBootloader, bootloaderSize);

    /* Parse SAGE Framework */
    frameworkHolder.name = "Framework image";
    rc = BSAGE_P_Boot_ParseSageImage(hSAGE, ctx, pFramework, frameworkSize, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE Framework compatibility with current chipset */
    rc = BSAGE_P_Boot_CheckCompatibility(hSAGE, &frameworkHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGE->i_memory_sync.flush(pFramework, frameworkSize);

    /* Check SAGE Framework compatibility with BFW version */
    rc = BSAGE_P_Boot_CheckFrameworkBFWVersion(hHsm,&frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("***********************************************************************"));
        BDBG_ERR(("BOLT with BFW version 4.1.3 or later must be used with SAGE Framework 3.x."));
        BDBG_ERR(("Please upgrade BOLT."));
        BDBG_ERR(("***********************************************************************"));
        goto end;
    }

    /* Check if both SAGE images have triple signing mode
    rc = BSAGE_P_Boot_CheckSigningMode(ctx);
    if(rc != BERR_SUCCESS) { goto end; } */

    /* push region map to DRAM */
    if(pBootSettings->regionMapNum != 0) {
        BSAGElib_RegionInfo *pRegionMap = hSAGE->i_memory_map.offset_to_addr(pBootSettings-> regionMapOffset);
        hSAGE->i_memory_sync.flush(pRegionMap, pBootSettings->regionMapNum * sizeof(BSAGElib_RegionInfo));
    }

    /* Set SAGE boot parameters information into Global SRAM GP registers */
    rc = BSAGE_P_Boot_SetBootParams(ctx, pBootSettings, &frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGE_P_Boot_SetBootParams() fails", BSTD_FUNCTION));
        goto end;
    }

    ctx->vklBoot = pBootSettings->vklBoot;

    /* Take SAGE out of reset */
    rc = BSAGE_P_Boot_ResetSage(hSAGE,hHsm,ctx, &blHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGE_P_Boot_ResetSage() fails %d", BSTD_FUNCTION, rc));
        goto end;
    }

    rc = BSAGE_P_MonitorBoot(hSAGE,hHsm);

end:

    if (ctx != NULL) {
        BKNI_Free(ctx);
    }

    BDBG_LEAVE(BSAGE_Boot_Launch);
    return rc;
}

static void
BSAGE_Boot_GetState(
    BSAGE_Handle hSAGE,
    BSAGElib_BootState *pState /* [out] */)
{
    uint32_t addr;

    BDBG_ENTER(BSAGE_Boot_GetState);

    BDBG_ASSERT(pState);

    /* Retrieve status */
    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBootStatus);
    pState->status = BREG_Read32(hSAGE->hReg, addr);

    /* Retrieve last error */
    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eLastError);
    pState->lastError = BREG_Read32(hSAGE->hReg, addr);

    BDBG_LEAVE(BSAGE_Boot_GetState);
    return;
}

static void
BSAGE_Boot_Clean(
    BSAGE_Handle hSAGE)
{
    BDBG_ENTER(BSAGE_Boot_Clean);

    /* wipe associated registers */
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBin, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSize, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkBinSignature, 0);
    _BSAGElib_P_Boot_SetBootParam(TextSectionSize, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, 0);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, 0);

    BDBG_LEAVE(BSAGE_Boot_Clean);
    return;
}

BERR_Code
BSAGE_Boot_GetInfo(
    BSAGElib_ChipInfo *pChipInfo,
    BSAGElib_ImageInfo *pBootloaderInfo,
    BSAGElib_ImageInfo *pFrameworkInfo)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_Handle hSAGE = hSAGE_Global;

    BDBG_ENTER(BSAGE_Boot_GetInfo);

    if (!hSAGE) {
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", BSTD_FUNCTION));
        goto err;
    }

    if(pChipInfo != NULL)
    {
        *pChipInfo = hSAGE->chipInfo;
    }

    if (pBootloaderInfo != NULL) {
        *pBootloaderInfo = hSAGE->bootloaderInfo;
    }

    if (pFrameworkInfo != NULL) {
        *pFrameworkInfo = hSAGE->frameworkInfo;
    }

err:
    BDBG_LEAVE(BSAGE_Boot_GetInfo);
    return rc;
}

static BERR_Code
BSAGE_Boot_Post(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm)
{
    BERR_Code rc;

    BDBG_ENTER(BSAGE_Boot_Post);

    BSAGE_iLockHsm();
    rc = BHSM_InitialiseBypassKeyslots(hHsm);
    BSAGE_iUnlockHsm();

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BHSM_InitialiseBypassKeyslots() fails %d", BSTD_FUNCTION, rc));
        goto end;
    }

    hSAGE->bBootPostCalled = true;

end:
    BDBG_LEAVE(BSAGE_Boot_Post);
    return rc;
}

static BERR_Code
BSAGE_P_MonitorBoot(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm)
{
    BERR_Code rc = BERR_SUCCESS;
    int totalBootTimeUs;
    int alreadyConsumedBootTimeUs;
    int overheadUs;
    uint32_t lastStatus=0x42;
    BSAGElib_BootState bootState;

    alreadyConsumedBootTimeUs = totalBootTimeUs = 0;

    do {
        BSAGE_Boot_GetState(hSAGE,&bootState);
        if(bootState.status == BSAGElibBootStatus_eStarted) {
            break;
        }
        else if (bootState.status != lastStatus) {
            BDBG_MSG(("%s - last_status=%u,new_status=%u,total=%d us,overhead=%d us",
                      BSTD_FUNCTION, lastStatus,bootState.status,totalBootTimeUs,
                      totalBootTimeUs - alreadyConsumedBootTimeUs));
            lastStatus = bootState.status;
        }
        if (bootState.status == BSAGElibBootStatus_eError) {
            if (bootState.lastError) {
                rc = BERR_UNKNOWN;
                BDBG_ERR(("%s -  SAGE boot detected error", BSTD_FUNCTION));
                goto err;
            }
        }

        if (totalBootTimeUs > SAGE_MAX_BOOT_TIME_US) {
            rc = BERR_TIMEOUT;
            BDBG_ERR(("%s - SAGE boot exceeds %d ms. Failure!!!!!!", BSTD_FUNCTION, SAGE_MAX_BOOT_TIME_US));
            goto err;
        }
        BKNI_Sleep(SAGE_STEP_BOOT_TIME_US/1000);
        totalBootTimeUs += SAGE_STEP_BOOT_TIME_US;
    } while (1);

    overheadUs = totalBootTimeUs - alreadyConsumedBootTimeUs;
    if (overheadUs > 0) {
        BDBG_MSG(("%s - SAGE boot tooks %d us, overhead is %d us",
                  BSTD_FUNCTION, totalBootTimeUs, overheadUs));
    }
    else {
        BDBG_MSG(("%s - SAGE booted, boot time is unknown", BSTD_FUNCTION));
    }

    /* Initialize bypass keyslot */
    {
        rc = BSAGE_Boot_Post(hSAGE,hHsm);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Boot_Post() fails %d.", BSTD_FUNCTION, rc));
            goto err;
        }
    }

    BDBG_MSG(("SAGE Boot done"));

err:
    if (bootState.status != BSAGElibBootStatus_eStarted) {
        char *_flavor = (hSAGE->chipInfo.chipType == BSAGElib_ChipType_eZS) ? "_dev" : "";
        BDBG_ERR(("*******  SAGE ERROR  >>>>>"));
        BDBG_ERR(("* The SAGE side software cannot boot."));
        if (lastStatus == BSAGElibBootStatus_eBlStarted) {
            BDBG_ERR(("* SAGE Framework hangs"));
            BDBG_ERR(("* Please check your sage_framework%s.bin", _flavor));
        }
        else if (lastStatus == BSAGElibBootStatus_eError) {
            BDBG_ERR(("* SAGE Bootloader error: 0x%08x", bootState.lastError));
            switch(bootState.lastError)
            {
                case SAGE_SECUREBOOT_ARC_PRESCREEN_NUMBER_BAD_BIT_EXCEEDED:
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    BDBG_ERR(("* SAGE ARC Bad Bit Management: More than 2 SAGE ARC Bad bits"));
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    break;
                case SAGE_SECUREBOOT_ARC_PRESCREEN_MSP_PROG_FAILURE:
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    BDBG_ERR(("* SAGE ARC Bad Bit Management: MSP OTP programming error"));
                    BDBG_ERR(("* DO NOT USE THIS CHIP!!"));
                    break;
                default:
                    BDBG_ERR(("* Please check your sage_bl%s.bin and sage_framework%s.bin", _flavor, _flavor));
            }

        }
        else {
            BDBG_ERR(("* Reboot the system and try again."));
            BDBG_ERR(("* SAGE Bootloader status: 0x%08x", lastStatus));
            BDBG_ERR(("* SAGE Bootloader hangs"));
            BDBG_ERR(("* Please check your sage_bl%s.bin", _flavor));
        }
        BDBG_ERR(("*******  SAGE ERROR  <<<<<"));
        BKNI_Sleep(200);
        BDBG_ASSERT(false && "SAGE CANNOT BOOT");
    }
    BSAGE_Boot_Clean(hSAGE);
    return rc;
}
