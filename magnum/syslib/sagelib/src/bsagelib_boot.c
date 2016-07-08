/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bsagelib_priv.h"
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


BDBG_MODULE(BSAGElib);


/* Host to Sage communication buffers size */
#define SAGE_HOST_BUF_SIZE (32)

#define SAGE_BL_LENGTH (BCHP_SCPU_LOCALRAM_REG_END - BCHP_SCPU_LOCALRAM_REG_START + 4)

/* Types */
#define SAGE_HEADER_TYPE_BL      (0x5357)
#define SAGE_HEADER_TYPE_FRAMEWORK  (0x424C)
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

    bool sageBlTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageFrameworkTripleSigning;  /* triple-signing scheme or single-signing scheme */
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
    unsigned char ucReserved2;
    unsigned char ucEpochSelect;
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
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
    BCMD_SecondTierKey_t second_tier_key2;  /* triple signing scheme */
    BCMD_SecondTierKey_t second_tier_key3;  /* triple signing scheme */
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
        BREG_Write32(hSAGElib->core_handles.hReg, addr, VAL);                   \
        BootParamDbgPrintf(("%s - Read %s - Addr: 0x%08x Value: 0x%08x", __FUNCTION__, #REGID, addr, BREG_Read32(hSAGElib->core_handles.hReg, (uint32_t)addr))); }


#define _Swap16(PVAL) (((PVAL)[0] << 8) | ((PVAL)[1]))


/****************************************
 * Local Functions
 ***************************************/
static BERR_Code BSAGElib_P_Boot_GetSageOtpMspParams(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGElib_P_Boot_CheckCompatibility(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);
static BERR_Code BSAGElib_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGElib_P_Boot_ParseSageImage(BSAGElib_Handle hSAGElib, BSAGElib_P_BootContext *ctx, uint8_t *pBinary, uint32_t binarySize, BSAGElib_SageImageHolder* holder);
static BERR_Code BSAGElib_P_Boot_SetBootParams(BSAGElib_P_BootContext *ctx, BSAGElib_BootSettings *pBootSettings, BSAGElib_SageImageHolder* frameworkHolder);
static BERR_Code BSAGElib_P_Boot_ResetSage(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *bl_img);
static BERR_Code BSAGElib_P_Boot_CheckFrameworkBFWVersion(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);

static BCMD_SecondTierKey_t * BSAGElib_P_Boot_GetKey(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder* image);
static uint8_t *BSAGElib_P_Boot_GetSignature(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *image);
static void BSAGElib_P_Boot_SetImageInfo(BSAGElib_ImageInfo *pImageInfo, BSAGElib_SageImageHolder* holder, uint32_t header_version, uint32_t type, bool triple_sign);

static BCMD_SecondTierKey_t *
BSAGElib_P_Boot_GetKey(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    BCMD_SecondTierKey_t *ret = NULL;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    if (ctx->sageFrameworkTripleSigning || ctx->sageBlTripleSigning) {
        BSAGElib_SageSecureHeaderTripleSign *triple_sign_header =
            (BSAGElib_SageSecureHeaderTripleSign *)image->header;

        if(hSAGElib->chipInfo.chipType == BSAGElib_ChipType_eCustomer1) {
            if(ctx->otp_market_id == SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_ZERO) {
                ret = &triple_sign_header->second_tier_key;
            }
            else if(ctx->otp_market_id == SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_0xFFEE) {
                ret = &triple_sign_header->second_tier_key2;
            }
            else {
                ret = &triple_sign_header->second_tier_key3;
            }
        }
        else {
            BDBG_ERR(("%s - This chip type does not support triple signing.", __FUNCTION__));
        }

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
    uint32_t index = _Swap16(image->header->ucHeaderIndex);
    uint32_t sig_size;
    bool triple = false;

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
        BSAGElib_Handle hSAGElib = ctx->hSAGElib;
        if(hSAGElib->chipInfo.chipType == BSAGElib_ChipType_eCustomer1) {
            if(ctx->otp_market_id == SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_ZERO) {
                /* Market ID = 0x0 - use first image signature */
                signature = image->signature;
            }
            else if(ctx->otp_market_id == SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_0xFFEE) {
                /* Market ID = 0xFFEE - use second image signature*/
                signature = &(image->signature[sig_size]);
            }
            else {
                /* Use third image signature */
                signature = &(image->signature[2*sig_size]);
            }
        }
        else {
            BDBG_ERR(("%s - This chip type does not support triple signing.", __FUNCTION__));
        }
    }
    else {
        signature = image->signature;
    }
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
BSAGElib_P_Boot_GetSageOtpMspParams(
    BSAGElib_P_BootContext *ctx)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    /* Pull OTPs:
     * BCMD_Otp_CmdMsp_eReserved210 : SAGE decryption enabled
     * BCMD_Otp_CmdMsp_eReserved209 : SAGE verification enabled
     * BCMD_Otp_CmdMsp_eReserved212 : SAGE secure enabled
     * BCMD_Otp_CmdMsp_eMarketId : market Id
     * BCMD_Otp_CmdMsp_eSystemEpoch // BCMD_Otp_CmdMsp_eReserved87 : Epoch */

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved210, &ctx->otp_sage_decrypt_enable, "decrypt_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved209, &ctx->otp_sage_verify_enable, "verify_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved212, &ctx->otp_sage_secure_enable, "secure_enable");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eMarketId, &ctx->otp_market_id, "market id");
    if (rc != BERR_SUCCESS) { goto end; }

#if (ZEUS_VERSION < ZEUS_4_1)
    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eSystemEpoch, &ctx->otp_system_epoch0, "system epoch 0");
#else
    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved87, &ctx->otp_system_epoch0, "epoch");
    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eSystemEpoch3, &ctx->otp_system_epoch3, "system epoch 3");
#endif
    if (rc != BERR_SUCCESS) { goto end; }

    BDBG_MSG(("%s - OTP [SAGE SECURE ENABLE: %d, SAGE DECRYPT_ENABLE: %d, SAGE VERIFY_ENABLE: %d]",
              __FUNCTION__, ctx->otp_sage_secure_enable,
              ctx->otp_sage_decrypt_enable,
              ctx->otp_sage_verify_enable));

    BDBG_MSG(("%s - OTP [MARKET ID: 0x%08x, SYSTEM EPOCH 0: %d, SYSTEM EPOCH 3: %d]",
                  __FUNCTION__, ctx->otp_market_id,
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
BSAGElib_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx)
{
    BERR_Code rc = BERR_SUCCESS;

    if(ctx->sageBlTripleSigning != ctx->sageFrameworkTripleSigning) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s - Both SAGE images must be in triple signing mode", __FUNCTION__));
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
BSAGElib_P_Boot_CheckFrameworkBFWVersion(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SageImageHolder *image)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)image->header;
    BHSM_Capabilities_t hsmCaps;

    BDBG_MSG(("SAGE Framework version major %d",header->ucSageImageVersion[0]));
    if (header->ucSageImageVersion[0] >= SAGE_FRAMEWORK_VERSION_CHECK)
    {
        if( ( rc = BHSM_GetCapabilities(hSAGElib->core_handles.hHsm, &hsmCaps ) ) != BERR_SUCCESS )
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
        subTypeStr = "GENERIC";
        BDBG_MSG(("%s - '%s' Detected [type=%s, header_version=%u, triple_sign=%u]",
                  __FUNCTION__, holder->name, typeStr, header_version, triple_sign));
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
            pImageInfo->THLShortSig = pHeaderSingle->ucThLShortSig[0] | (pHeaderSingle->ucThLShortSig[1] << 8) |
                                      (pHeaderSingle->ucThLShortSig[2] << 16) | (pHeaderSingle->ucThLShortSig[3] << 24);
        }
        BDBG_MSG(("%s - '%s' Detected [type=%s, header_version=%u, triple_sign=%u, THL Short Sig=0x%08x]",
                  __FUNCTION__, holder->name, typeStr, header_version, triple_sign, pImageInfo->THLShortSig));
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

       image_type = index;
       image_signing_scheme = holder->header->ucImageSigningScheme;
       header_version = holder->header->ucHeaderVersion;

       if(image_signing_scheme == SAGE_HEADER_SECURE_IMAGE_TRIPLE_SIGNING_SCHEME_VALUE) {
           triple = true;
           all_sig_size = 3*_SIG_SZ;
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
           BDBG_ERR(("%s - '%s' Image: Invalid header (0x%x)", __FUNCTION__, holder->name, index));
           rc = BERR_NOT_SUPPORTED;
           goto end;
       }
   }
   /* else:? TBD */

   /* get pointer to data, last block, i.e. whats remaining */
   holder->data = raw_ptr;
   holder->data_len = raw_remain;

   BDBG_MSG(("%s - '%s' Header@%p, Signature@%p, Data@%p, Data length=%d", __FUNCTION__,
             holder->name, (void *)holder->header, (void *)holder->signature, (void *)holder->data, holder->data_len));

end:
   if (rc != BERR_SUCCESS) {
       BDBG_ERR(("%s failure for '%s'", __FUNCTION__, holder->name));
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
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    /* SAGE < -- > Host communication buffers */
    offset = hSAGElib->i_memory_map.addr_to_offset(hSAGElib->hsi_buffers);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert HSI buffer address to offset", __FUNCTION__));
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

    /* SAGE Services parameters - resources */
    {
        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        uint32_t sageVklMask = ((1 << BHSM_RemapVklId(hSAGElib->vkl1)) | (1 << BHSM_RemapVklId(hSAGElib->vkl2)));
        _BSAGElib_P_Boot_SetBootParam(SageVklMask, sageVklMask);
    }
    _BSAGElib_P_Boot_SetBootParam(SageDmaChannel, 0);

    /* SAGE Secure Boot */
    offset = hSAGElib->i_memory_map.addr_to_offset(frameworkHolder->data);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert framework address to offset", __FUNCTION__));
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
            offset = hSAGElib->i_memory_map.addr_to_offset(BSAGElib_P_Boot_GetSignature(ctx, frameworkHolder));
            if(offset == 0) {
                BDBG_ERR(("%s - Cannot convert SAGE Framework signature address to offset", __FUNCTION__));
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
            data = (single_sign_header->ucInstrSectionSize[3] << 0) | (single_sign_header->ucInstrSectionSize[2] << 8) | (single_sign_header->ucInstrSectionSize[1] << 16) | (single_sign_header->ucInstrSectionSize[0] << 24);
        }
        _BSAGElib_P_Boot_SetBootParam(TextSectionSize, data);

        offset = hSAGElib->i_memory_map.addr_to_offset(frameworkHolder->header);
        if(offset == 0) {
            BDBG_ERR(("%s - Cannot convert SAGE framework header address to offset", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, offset);
        _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, BSAGElib_P_Boot_GetHeaderSize(ctx, frameworkHolder));
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
    BCMD_VKLID_e vkl_id = BCMD_VKL_eMax;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    /* if OTP_SAGE_VERIFY_ENABLE_BIT: verify SAGE boot loader */
    if(ctx->otp_sage_verify_enable) {
        BHSM_VerifySecondTierKeyIO_t secondTierKey;

        BKNI_Memset(&secondTierKey, 0, sizeof(BHSM_VerifySecondTierKeyIO_t));

        if(header->ucKey0Type == SAGE_HEADER_KEY0_SELECT_KEY0) {
            secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0;
        }
        else {
            secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0Prime;
        }
        secondTierKey.eKeyIdentifier = BCMD_SecondTierKeyId_eKey3;

        secondTierKey.keyAddr = hSAGElib->i_memory_map.addr_to_offset(BSAGElib_P_Boot_GetKey(ctx, blImg));
        if(secondTierKey.keyAddr == 0) {
            BDBG_ERR(("%s - Cannot convert 2nd-tier key address to offset", __FUNCTION__));
            goto end;
        }

        BSAGElib_iLockHsm();
        rc = BHSM_VerifySecondTierKey(hSAGElib->core_handles.hHsm,  &secondTierKey);
        BSAGElib_iUnlockHsm();
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - BHSM_VerifySecondTierKey() fails %d", __FUNCTION__, rc));
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
            BDBG_ERR(("%s - BHSM_AllocateVKL() fails %d", __FUNCTION__, rc));
            goto end;
        }

        vkl_id = allocateVKLIO.allocVKL;
        BDBG_MSG(("%s - SAGE BL VKL ID: %d", __FUNCTION__, vkl_id));
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
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key3 fails %d", __FUNCTION__, rc));
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
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key4 fails %d", __FUNCTION__, rc));
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
                BDBG_ERR(("%s - BHSM_GenerateRouteKey() for key5 fails %d", __FUNCTION__, rc));
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
            BDBG_ERR(("%s - Cannot convert bootloader address to offset", __FUNCTION__));
            goto end;
        }
        verifyIO.region.endAddress = verifyIO.region.startAddress + (blImg->data_len) - 1;

        if (blImg->signature) {
            verifyIO.signature.startAddress = hSAGElib->i_memory_map.addr_to_offset(BSAGElib_P_Boot_GetSignature(ctx, blImg));
            if(verifyIO.signature.startAddress == 0) {
                BDBG_ERR(("%s - Cannot convert bootloader signature address to offset", __FUNCTION__));
                goto end;
            }
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
            BDBG_ERR(("\nBHSM_RegionVerification_Configure fails - region = 0x%x, status = %x\n", 0x18, rc));
            goto end;
        }

        BSAGElib_iLockHsm();
        rc = BHSM_RegionVerification_Enable(hSAGElib->core_handles.hHsm);
        BSAGElib_iUnlockHsm();
        if ( rc != 0) {
            BDBG_ERR(("\nBHSM_RegionVerification_Enable fails - status = %x\n", rc));
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
                    BDBG_ERR(("%s - BHSM_RegionVerification_Status fails - region = 0x%x, status = 0x%x", __FUNCTION__, 0x18, rc));
                    goto end;
                }
                else
                {
                    BDBG_MSG(("%s - BHSM_RegionVerification_Status in progress", __FUNCTION__));
                }
            }
        }while (status.state != BHSM_RegionVerificationStatus_eVerified);

        BDBG_LOG(("%s: SAGE reset completed successfully", __FUNCTION__));
    }
end:
    if(vkl_id != BCMD_VKL_eMax)
    {
        BSAGElib_iLockHsm();
        BHSM_FreeVKL(hSAGElib->core_handles.hHsm, vkl_id);
        BSAGElib_iUnlockHsm();
    }

    return rc;
}

/****************************************
 * Public API
 ***************************************/

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
        (pBootSettings->frameworkSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", __FUNCTION__));
        goto end;
    }

    if (hSAGElib->core_handles.hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", __FUNCTION__));
        goto end;
    }

    ctx = BKNI_Malloc(sizeof(*ctx));
    if (ctx == NULL) {
        BDBG_ERR(("%s - Cannot allocate temporary context for SAGE Boot.", __FUNCTION__));
        goto end;
    }
    BKNI_Memset(ctx, 0, sizeof(*ctx));
    ctx->hSAGElib = hSAGElib;

    /* Get SAGE Secureboot OTP MSPs value */
    rc = BSAGElib_P_Boot_GetSageOtpMspParams(ctx);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_GetSageOtpMspParams() fails", __FUNCTION__));
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
        BDBG_ERR(("***********************************************************************"));
        BDBG_ERR(("BOLT with BFW version 4.1.3 or later must be used with SAGE Framework 3.x."));
        BDBG_ERR(("Please upgrade BOLT."));
        BDBG_ERR(("***********************************************************************"));
        goto end;
    }

    /* Check if both SAGE images have triple signing mode */
    rc = BSAGElib_P_Boot_CheckSigningMode(ctx);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Set SAGE boot parameters information into Global SRAM GP registers */
    rc = BSAGElib_P_Boot_SetBootParams(ctx, pBootSettings, &frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_SetBootParams() fails", __FUNCTION__));
        goto end;
    }

    /* Take SAGE out of reset */
    rc = BSAGElib_P_Boot_ResetSage(ctx, &blHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_ResetSage() fails %d", __FUNCTION__, rc));
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

    BSAGElib_iLockHsm();
    rc = BHSM_InitialiseBypassKeyslots(hSAGElib->core_handles.hHsm);
    BSAGElib_iUnlockHsm();

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BHSM_InitialiseBypassKeyslots() fails %d", __FUNCTION__, rc));
        goto end;
    }

    hSAGElib->bBootPostCalled = true;

end:
    BDBG_LEAVE(BSAGElib_Boot_Post);
    return rc;
}
