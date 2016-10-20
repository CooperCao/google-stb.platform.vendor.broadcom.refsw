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
#define SAGE_HEADER_TYPE_BL      (0x53570000)
#define SAGE_HEADER_TYPE_OS_APP  (0x424C0000)
#define SAGE_BINARY_TYPE_ZB      (0x3E3F)
#define SAGE_BINARY_TYPE_ZS      (0x0202)
#define SAGE_BINARY_TYPE_CUST1   (0x0303)

#define SAGE_HEADER_SECURE_IMAGE_TRIPLE_SIGNING_SCHEME_VALUE    (0x1)
#define SAGE_HEADER_SECURE_IMAGE_VERSION_MASK                   (0x0000FF00)
#define SAGE_HEADER_SECURE_IMAGE_SIGNING_SCHEME_MASK            (0x000000FF)
#define SAGE_HEADER_SECURE_IMAGE_TYPE_MASK                      (0xFFFF0000)


#define SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_ZERO   (0x0)
#define SAGE_HEADER_TRIPLE_SIGNING_MARKET_ID_0xFFEE (0xFFEE)

/* Key0 selection */
#define SAGE_HEADER_KEY0_SELECT_KEY0       (0x43) /* Key0 is used -> 'C' for Customer tool */
#define SAGE_HEADER_KEY0_SELECT_KEY0PRIME  (0x49) /* Key0prime is used -> 'I' for BRCM Internal tool */

/* Signature */
#define _SIG_SZ (256)
#define _SIG_SZ_SAGE_OS_APP (2*_SIG_SZ)

typedef struct {
    BSAGElib_Handle hSAGElib;
    uint32_t otp_sage_decrypt_enable;
    uint32_t otp_sage_verify_enable;
    uint32_t otp_sage_secure_enable;
    uint32_t otp_system_epoch0;
    uint32_t otp_system_epoch3;
    uint32_t otp_market_id;

    bool sageBlTripleSigning;  /* triple-signing scheme or single-signing scheme */
    bool sageOsAppTripleSigning;  /* triple-signing scheme or single-signing scheme */
    /* Secure Image version */
    uint8_t sageBlSecureVersion;
    uint8_t sageOsAppSecureVersion;

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
    unsigned char ucHeaderIndex[4]; /* see comments in structures below */
    unsigned char ucSecurityType;
    unsigned char ucImageType;
    unsigned char ucReserved[2];
    unsigned char ucSageSfwVersion[5];
    unsigned char ucSageBlVersion[3];
    unsigned char ucPlatformId[4];
    unsigned char ucCaVendorId[2];
    unsigned char ucStbOwnerIdSelect;
    unsigned char ucSwizzle0aVariant;
    unsigned char ucSwizzle0aVersion;
    unsigned char ucCustKeyVar;
    unsigned char ucKeyVarHi;
    unsigned char ucKeyVarlo;
    unsigned char ucModuleId;
    unsigned char ucReserved1[3];
    unsigned char ucProcIn1[16];
    unsigned char ucProcIn2[16];
    unsigned char ucProcIn3[16];
    BCMD_SecondTierKey_t second_tier_key; /* Zeus 2/3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
    unsigned char ucSize[4];
    unsigned char ucInstrSectionSize[4];
    unsigned char ucDataSectionAddr[4];
    unsigned char ucDataSectionSize[4];
} BSAGElib_SageSecureHeader;

typedef struct
{
    unsigned char ucHeaderIndex[4];     /* [0-1] is used for header type (SAGE Bootloader or SAGE SW), [2] is used for SAGE secure image version, [3] is used to indicate single or triple signing scheme */
    unsigned char ucSecurityType;       /* security type: signature, encryption or both */
    unsigned char ucImageType;          /* data type: SAGE bootloader or SAGE SW */
    unsigned char ucReserved[2];        /* [0] is used to report GlobalOwnerID value, [1] is unused */
    unsigned char ucSageSfwVersion[5];  /* SAGE Bootloader/SW version [0-3] is unused */
    unsigned char ucSageBlVersion[3];   /* [0-3] is unused */
    unsigned char ucPlatformId[4];      /* platform ID: [0-3] is unused */
    unsigned char ucCaVendorId[2];      /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;   /* STB Owwner ID */
    unsigned char ucSwizzle0aVariant;   /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;   /* Swizzle0a Version */
    unsigned char ucCustKeyVar;         /* Cust Key Var */
    unsigned char ucKeyVarHi;           /* Key Var Hi */
    unsigned char ucKeyVarlo;           /* Key Var Low */
    unsigned char ucModuleId;           /* Module ID */
    unsigned char ucReserved1[3];       /* [0] is used for key0 type, [1-2] are used for SCM type and SCM version */
    unsigned char ucProcIn1[16];        /* Proc In 1 */
    unsigned char ucProcIn2[16];        /* Proc In 2 */
    unsigned char ucProcIn3[16];        /* Proc In 3 */
    BCMD_SecondTierKey_t second_tier_key;  /* 528 bytes - Zeus 2/3/4.1 and 532 bytes - Zeus 4.2 */
    BCMD_SecondTierKey_t second_tier_key2;  /* triple signing scheme */
    BCMD_SecondTierKey_t second_tier_key3;  /* triple signing scheme */
    unsigned char ucSize[4];            /* size of the encrypted data */
    unsigned char ucInstrSectionSize[4];/* size of the instruction section of the SAGE SW, [0-3] is unused for SAGE BL */
    unsigned char ucDataSectionAddr[4]; /* offset of the data section of the SAGE SW, [0-3] is unused for SAGE BL */
    unsigned char ucDataSectionSize[4]; /* size of the data section of teh SAGE SW, [0-3] is unused for SAGE BL */
} BSAGElib_SageSecureHeaderTripleSign;


typedef struct /* this structure has the common parts to both previous structures */
{
    unsigned char ucHeaderIndex[4];     /* [0-1] is used for header type (SAGE Bootloader or SAGE SW), [2] is used for SAGE secure image version, [3] is used to indicate single or triple signing scheme */
    unsigned char ucSecurityType;       /* security type: signature, encryption or both */
    unsigned char ucImageType;          /* data type: SAGE bootloader or SAGE SW */
    unsigned char ucReserved[2];        /* [0] is used to report GlobalOwnerID value, [1] is unused */
    unsigned char ucSageSfwVersion[5];  /* SAGE Bootloader/SW version [0-3] is unused */
    unsigned char ucSageBlVersion[3];   /* [0-3] is unused */
    unsigned char ucPlatformId[4];      /* platform ID: [0-3] is unused */
    unsigned char ucCaVendorId[2];      /* CA vendor ID */
    unsigned char ucStbOwnerIdSelect;   /* STB Owwner ID */
    unsigned char ucSwizzle0aVariant;   /* Swizzle0a Variant */
    unsigned char ucSwizzle0aVersion;   /* Swizzle0a Version */
    unsigned char ucCustKeyVar;         /* Cust Key Var */
    unsigned char ucKeyVarHi;           /* Key Var Hi */
    unsigned char ucKeyVarlo;           /* Key Var Low */
    unsigned char ucModuleId;           /* Module ID */
    unsigned char ucReserved1[3];       /* [0] is used for key0 type, [1-2] are used for SCM type and SCM version */
    unsigned char ucProcIn1[16];        /* Proc In 1 */
    unsigned char ucProcIn2[16];        /* Proc In 2 */
    unsigned char ucProcIn3[16];        /* Proc In 3 */
    BCMD_SecondTierKey_t second_tier_key;  /* 528 bytes - Zeus 2/3/4.1 and 532 bytes - Zeus 4.2 */
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
#if 0
#define BootParamDbgPrintf(format) BDBG_MSG(format)
#else
#define BootParamDbgPrintf(format)
#endif

/* TODO: stays as is for the time being */
#define _BSAGElib_P_Boot_SetBootParam(REGID, VAL) {      \
        uint32_t addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_e##REGID); \
        BREG_Write32(hSAGElib->core_handles.hReg, addr, VAL);                   \
        BootParamDbgPrintf(("%s - Read %s - Addr: %p, Value: %08x", __FUNCTION__, #REGID, addr, BREG_Read32(hSAGElib->core_handles.hReg, addr))); }


#define _ChipTypeSwap(PVAL) (((PVAL)[0] << 8) | ((PVAL)[1]))


/****************************************
 * Local Functions
 ***************************************/
static BERR_Code BSAGElib_P_Boot_GetSageOtpMspParams(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGElib_P_Boot_CheckCompatibility(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);
static BERR_Code BSAGElib_P_Boot_CheckSigningMode(BSAGElib_P_BootContext *ctx);
static BERR_Code BSAGElib_P_Boot_ParseSageImage(BSAGElib_Handle hSAGElib, BSAGElib_P_BootContext *ctx, uint8_t *pBinary, uint32_t binarySize, BSAGElib_SageImageHolder* holder);
static BERR_Code BSAGElib_P_Boot_SetBootParams(BSAGElib_P_BootContext *ctx, BSAGElib_BootSettings *pBootSettings, BSAGElib_SageImageHolder* kernelHolder);
static BERR_Code BSAGElib_P_Boot_ResetSage(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *bl_img);
static BERR_Code BSAGElib_P_Boot_CheckOsAppBFWVersion(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *img);

static BCMD_SecondTierKey_t * BSAGElib_P_Boot_GetKey(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder* image);
static uint8_t *BSAGElib_P_Boot_GetSignature(BSAGElib_P_BootContext *ctx, BSAGElib_SageImageHolder *image);
static void BSAGElib_P_Boot_PrintBinaryVersion(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *image);

static BCMD_SecondTierKey_t *
BSAGElib_P_Boot_GetKey(
    BSAGElib_P_BootContext *ctx,
    BSAGElib_SageImageHolder *image)
{
    BCMD_SecondTierKey_t *ret = NULL;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    if (ctx->sageOsAppTripleSigning || ctx->sageBlTripleSigning) {
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
    uint32_t index = _EndianSwap(image->header->ucHeaderIndex);
    uint32_t sig_size;
    bool triple = false;

    if ((index & SAGE_HEADER_SECURE_IMAGE_TYPE_MASK) == SAGE_HEADER_TYPE_BL) {
        sig_size = _SIG_SZ;
        if (ctx->sageBlTripleSigning) {
            triple = true;
        }
    }
    else {
        sig_size = _SIG_SZ_SAGE_OS_APP;
        if (ctx->sageOsAppTripleSigning) {
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

    if(ctx->sageBlTripleSigning != ctx->sageOsAppTripleSigning) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s - Both SAGE images must be in triple signing mode", __FUNCTION__));
        goto end;
    }

end:
    return rc;
}

#define SAGE_OSAPP_VERSION_CHECK 3
#define BFW_VERSION_CHECK_MAJOR 4
#define BFW_VERSION_CHECK_MINOR 0
#define BFW_VERSION_CHECK_SUBMINOR 3

static BERR_Code
BSAGElib_P_Boot_CheckOsAppBFWVersion(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SageImageHolder *image)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)image->header;
    BHSM_Capabilities_t hsmCaps;

    BDBG_MSG(("SAGE OS/APP version major %d",header->ucSageSfwVersion[0]));
    if (header->ucSageSfwVersion[0] >= SAGE_OSAPP_VERSION_CHECK)
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
            if(_ChipTypeSwap(img->header->ucReserved) != SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x).", _ChipTypeSwap(img->header->ucReserved)));
                BDBG_ERR(("* Chipset Type is ZS!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eZB:
            /* return error only if chip is BRCM development part (ZS) */
            if(_ChipTypeSwap(img->header->ucReserved) == SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x) .", _ChipTypeSwap(img->header->ucReserved)));
                BDBG_ERR(("* Chipset Type is ZB!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eCustomer1:
        case BSAGElib_ChipType_eCustomer:
            /* return error only if chip is BRCM development part (ZS) */
            if(_ChipTypeSwap(img->header->ucReserved) == SAGE_BINARY_TYPE_ZS)
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", img->name));
                BDBG_ERR(("* Invalid SAGE binary type (0x%04x).", _ChipTypeSwap(img->header->ucReserved)));
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
 * binary (bootloader or kernel) located in memory into a
 * BSAGElib_SageImageHolder structure.
Raw file in memory:
    +----------+---------------+-------------------------------------------------
    |  header  |   signature   |    binary data (kernel, bootloader)      . . .
    +----------+---------------+-------------------------------------------------
*/
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
       uint32_t image_type, image_signing_scheme, image_version;
       uint32_t all_sig_size;
       uint32_t sec_header_size;
       bool triple;

       holder->header = (BSAGElib_SageSecureHeaderCommon *)raw_ptr;

       index = _EndianSwap(holder->header->ucHeaderIndex);

       image_type = index & SAGE_HEADER_SECURE_IMAGE_TYPE_MASK;
       image_signing_scheme = index & SAGE_HEADER_SECURE_IMAGE_SIGNING_SCHEME_MASK;
       image_version = (index & SAGE_HEADER_SECURE_IMAGE_VERSION_MASK) >> 8;

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
           BDBG_MSG(("%s - '%s' Detected [type=Bootloader, version=%u, sign=%s]",
                     __FUNCTION__, holder->name, image_version, triple? "triple" : "single"));
            ctx->sageBlSecureVersion = image_version;
            ctx->sageBlTripleSigning = triple;
           break;
       case SAGE_HEADER_TYPE_OS_APP:
           BDBG_MSG(("%s - '%s' Detected [type=OS/Application, version=%u, sign=%s]",
                     __FUNCTION__, holder->name, image_version, triple? "triple" : "single"));
           ctx->sageOsAppSecureVersion = image_version;
           ctx->sageOsAppTripleSigning = triple;
           /* Add extra sig for OS/App and set image version */
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
   BSAGElib_P_Boot_PrintBinaryVersion(hSAGElib, holder);

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
    BSAGElib_SageImageHolder* kernelHolder)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t offset = 0;
    BSAGElib_Handle hSAGElib = ctx->hSAGElib;

    /* 7 restricted regions */
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion1,     pBootSettings->SRROffset);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion1Size, pBootSettings->SRRSize);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion2,     pBootSettings->CRROffset);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion2Size, pBootSettings->CRRSize);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion3,     pBootSettings->URR0Offset);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion3Size, pBootSettings->URR0Size);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion4,     pBootSettings->URR1Offset);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion4Size, pBootSettings->URR1Size);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion5,     pBootSettings->URR2Offset);
    _BSAGElib_P_Boot_SetBootParam(RestrictedRegion5Size, pBootSettings->URR2Size);
    _BSAGElib_P_Boot_SetBootParam(SageLogBuffer,     0);
    _BSAGElib_P_Boot_SetBootParam(SageLogBufferSize, 0);
    _BSAGElib_P_Boot_SetBootParam(SageLogWriteCountLSB, 0);
    _BSAGElib_P_Boot_SetBootParam(SageLogWriteCountMSB, 0);

    /* SAGE_FULL_HEAP heap boundaries */
    _BSAGElib_P_Boot_SetBootParam(GeneralHeap,     pBootSettings->GLR0Offset);
    _BSAGElib_P_Boot_SetBootParam(GeneralHeapSize, pBootSettings->GLR0Size);

    /* Secondary 'client' FULL HEAP heap boundaries (optional, offset and size can be 0) */
    _BSAGElib_P_Boot_SetBootParam(ClientHeap,     pBootSettings->GLR1Offset);
    _BSAGElib_P_Boot_SetBootParam(ClientHeapSize, pBootSettings->GLR1Size);

    /* kernel (bootloader) image */
    offset = hSAGElib->i_memory_map.addr_to_offset(kernelHolder->data);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert kernel address to offset", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
    _BSAGElib_P_Boot_SetBootParam(Kernel, offset);
    _BSAGElib_P_Boot_SetBootParam(KernelSize, kernelHolder->data_len);

    /* status */
    _BSAGElib_P_Boot_SetBootParam(LastError, 0);
    _BSAGElib_P_Boot_SetBootParam(BootStatus, BSAGElibBootStatus_eNotStarted);

    /* Vkl */
    {
        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        uint32_t sageReservedVklMask = ((1 << BHSM_RemapVklId(hSAGElib->vkl1)) | (1 << BHSM_RemapVklId(hSAGElib->vkl2)));
        _BSAGElib_P_Boot_SetBootParam(SageReservedVkl, sageReservedVklMask);
    }

    /* Communication buffers */
    offset = hSAGElib->i_memory_map.addr_to_offset(hSAGElib->hsi_buffers);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert HSI buffer address to offset", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

	_BSAGElib_P_Boot_SetBootParam(RequestBuffer,      offset);
    _BSAGElib_P_Boot_SetBootParam(RequestBufferSize,  SAGE_HOST_BUF_SIZE);
    _BSAGElib_P_Boot_SetBootParam(AckBuffer,          offset + SAGE_HOST_BUF_SIZE);
    _BSAGElib_P_Boot_SetBootParam(AckBufferSize,      SAGE_HOST_BUF_SIZE);
    _BSAGElib_P_Boot_SetBootParam(ResponseBuffer,     offset + (2 * SAGE_HOST_BUF_SIZE));
    _BSAGElib_P_Boot_SetBootParam(ResponseBufferSize, SAGE_HOST_BUF_SIZE);

    /* Misc */
    _BSAGElib_P_Boot_SetBootParam(BlVersion, 0);
    _BSAGElib_P_Boot_SetBootParam(SSFWVersion, 0);
    _BSAGElib_P_Boot_SetBootParam(SageLogBuffer, pBootSettings->logBufferOffset);
    _BSAGElib_P_Boot_SetBootParam(SageLogBufferSize, pBootSettings->logBufferSize);

    /* keys */
    if (ctx->otp_sage_verify_enable || ctx->otp_sage_decrypt_enable)
    {
        uint32_t data;

        if (kernelHolder->signature) {
            offset = hSAGElib->i_memory_map.addr_to_offset(BSAGElib_P_Boot_GetSignature(ctx, kernelHolder));
            if(offset == 0) {
                BDBG_ERR(("%s - Cannot convert kernel signature address to offset", __FUNCTION__));
                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            _BSAGElib_P_Boot_SetBootParam(KernelSignature, offset);
        } else {
            _BSAGElib_P_Boot_SetBootParam(KernelSignature, 0);
        }

        /* Key0 select */
        _BSAGElib_P_Boot_SetBootParam(Key0Select, kernelHolder->header->ucReserved1[0]);

       if (ctx->sageOsAppTripleSigning)
        {
            BSAGElib_SageSecureHeaderTripleSign *triple_sign_header = (BSAGElib_SageSecureHeaderTripleSign *)kernelHolder->header;
            data = (triple_sign_header->ucInstrSectionSize[3] << 0) | (triple_sign_header->ucInstrSectionSize[2] << 8) | (triple_sign_header->ucInstrSectionSize[1] << 16) | (triple_sign_header->ucInstrSectionSize[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(InstructionSectionSize, data);

            data = (triple_sign_header->ucDataSectionSize[3] << 0) | (triple_sign_header->ucDataSectionSize[2] << 8) | (triple_sign_header->ucDataSectionSize[1] << 16) | (triple_sign_header->ucDataSectionSize[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(DataSectionSize, data);

            data = (triple_sign_header->ucDataSectionAddr[3] << 0) | (triple_sign_header->ucDataSectionAddr[2] << 8) | (triple_sign_header->ucDataSectionAddr[1] << 16) | (triple_sign_header->ucDataSectionAddr[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(DataSectionOffset, data);
        }
        else
        {
            BSAGElib_SageSecureHeader *single_sign_header = (BSAGElib_SageSecureHeader *)kernelHolder->header;
            data = (single_sign_header->ucInstrSectionSize[3] << 0) | (single_sign_header->ucInstrSectionSize[2] << 8) | (single_sign_header->ucInstrSectionSize[1] << 16) | (single_sign_header->ucInstrSectionSize[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(InstructionSectionSize, data);

            data = (single_sign_header->ucDataSectionSize[3] << 0) | (single_sign_header->ucDataSectionSize[2] << 8) | (single_sign_header->ucDataSectionSize[1] << 16) | (single_sign_header->ucDataSectionSize[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(DataSectionSize, data);

            data = (single_sign_header->ucDataSectionAddr[3] << 0) | (single_sign_header->ucDataSectionAddr[2] << 8) | (single_sign_header->ucDataSectionAddr[1] << 16) | (single_sign_header->ucDataSectionAddr[0] << 24);
            _BSAGElib_P_Boot_SetBootParam(DataSectionOffset, data);
        }

        data =  (kernelHolder->header->ucCustKeyVar<<24) |
                (kernelHolder->header->ucKeyVarHi<<16) |
                (kernelHolder->header->ucKeyVarlo<<8) |
                ((  !(kernelHolder->header->ucCustKeyVar) &&
                    !(kernelHolder->header->ucKeyVarHi) &&
                    !(kernelHolder->header->ucKeyVarlo))? 1 : 0);
        _BSAGElib_P_Boot_SetBootParam(DecryptInfo, data);

        /* proc_in 1 */
        data = _EndianSwap(&kernelHolder->header->ucProcIn1[0]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn1[4]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 1, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn1[8]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 2, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn1[12]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 3, data);

        /* proc_in 2 */
        data = _EndianSwap(&kernelHolder->header->ucProcIn2[0]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn2[4]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 1, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn2[8]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 2, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn2[12]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 3, data);

        /* proc_in 3 */
        data = _EndianSwap(&kernelHolder->header->ucProcIn3[0]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn3[4]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 1, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn3[8]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 2, data);

        data = _EndianSwap(&kernelHolder->header->ucProcIn3[12]);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 3, data);

        data = (kernelHolder->header->ucCaVendorId[1] << 8) | (kernelHolder->header->ucCaVendorId[0]);
        _BSAGElib_P_Boot_SetBootParam(VendorId, data);

        data = (kernelHolder->header->ucStbOwnerIdSelect << 8) | kernelHolder->header->ucModuleId;
        _BSAGElib_P_Boot_SetBootParam(ModuleId_StbOwnerId, data);

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0))
        data = kernelHolder->header->ucSwizzle0aVariant;
#else
        switch(kernelHolder->header->ucSwizzle0aVariant)
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
#endif
        _BSAGElib_P_Boot_SetBootParam(Swizzle0aVariant, data);

        data = kernelHolder->header->ucSwizzle0aVersion;
        _BSAGElib_P_Boot_SetBootParam(Swizzle0aVersion, data);

        /* Second Tier Key */
        offset = hSAGElib->i_memory_map.addr_to_offset(BSAGElib_P_Boot_GetKey(ctx, kernelHolder));
        if(offset == 0) {
            BDBG_ERR(("%s - Cannot convert 2nd-tier key address to offset", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
        _BSAGElib_P_Boot_SetBootParam(SecondTierKey, offset);

        _BSAGElib_P_Boot_SetBootParam(MarketId, 0); /* Fixed by convention */
        _BSAGElib_P_Boot_SetBootParam(MarketIdMask, 0); /* Fixed by convetion */
        if(ctx->sageBlSecureVersion >= 3)
        {
            _BSAGElib_P_Boot_SetBootParam(Epoch, 0); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochMask, 0); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochSelect, 0x3); /* Fixed by convention */
        }
        else if(ctx->sageBlSecureVersion == 2)
        {
            _BSAGElib_P_Boot_SetBootParam(Epoch, 0); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochMask, 0xFC); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochSelect, 0x3); /* Fixed by convention */
        }
        else
        {
            _BSAGElib_P_Boot_SetBootParam(Epoch, 0); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochMask, 0); /* Fixed by convention */
            _BSAGElib_P_Boot_SetBootParam(EpochSelect, 0); /* Fixed by convention */
        }
    }
    else {
        _BSAGElib_P_Boot_SetBootParam(KernelSignature, 0);
        _BSAGElib_P_Boot_SetBootParam(Key0Select, 0);
        _BSAGElib_P_Boot_SetBootParam(SecondTierKey, 0);

        _BSAGElib_P_Boot_SetBootParam(DecryptInfo, 0);

        _BSAGElib_P_Boot_SetBootParam(ProcIn1, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 1, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 2, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn1 + 3, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 1, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 2, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn2 + 3, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 1, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 2, 0);
        _BSAGElib_P_Boot_SetBootParam(ProcIn3 + 3, 0);

        _BSAGElib_P_Boot_SetBootParam(VendorId, 0);
        _BSAGElib_P_Boot_SetBootParam(Swizzle0aVariant, 0);
        _BSAGElib_P_Boot_SetBootParam(Swizzle0aVersion, 0);
        _BSAGElib_P_Boot_SetBootParam(ModuleId_StbOwnerId, 0);

        _BSAGElib_P_Boot_SetBootParam(MarketId, 0);
        _BSAGElib_P_Boot_SetBootParam(MarketIdMask, 0);
        _BSAGElib_P_Boot_SetBootParam(Epoch, 0);
        _BSAGElib_P_Boot_SetBootParam(EpochMask, 0);
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

        if(header->ucReserved1[0] == SAGE_HEADER_KEY0_SELECT_KEY0) {
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
            generateRouteKeyIO.sageCAVendorID              = (header->ucCaVendorId[1] << 8) | (header->ucCaVendorId[0]);
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
        verifyIO.region.endAddress = verifyIO.region.startAddress + SAGE_BL_LENGTH - 1;

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
                    verifyIO.unEpoch |= (1 << header->ucPlatformId[2]) - 1;
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
        _BSAGElib_P_Boot_SetBootParam(SageBlAr, verifyIO.unEpoch);

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
    BSAGElib_SageImageHolder osAppHolder;
    BSAGElib_P_BootContext *ctx = NULL;

    BDBG_ENTER(BSAGElib_Boot_Launch);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pBootSettings);

    /* Validate boot settings */
    if ((pBootSettings->pBootloader == NULL) ||
        (pBootSettings->bootloaderSize == 0) ||
        (pBootSettings->pOsApp == NULL)      ||
        (pBootSettings->osAppSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", __FUNCTION__));
        goto end;
    }

    /* Validate heap offset and size. GLR1, URR0, URR1, URR2 are optional. */
    if ((pBootSettings->GLR0Offset == 0)     ||
        (pBootSettings->GLR0Size == 0)       ||
        (pBootSettings->SRROffset == 0)  ||
        (pBootSettings->SRRSize == 0)    ||
        (pBootSettings->CRROffset == 0) ||
        (pBootSettings->CRRSize == 0)) {
        BDBG_ERR(("%s - Invalid heap information.", __FUNCTION__));
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

    /* Parse SAGE OS/APP */
    osAppHolder.name = "OS/APP image";
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, ctx, pBootSettings->pOsApp, pBootSettings->osAppSize, &osAppHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE OS/APP compatibility with current chipset */
    rc = BSAGElib_P_Boot_CheckCompatibility(hSAGElib, &osAppHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGElib->i_memory_sync.flush(pBootSettings->pOsApp, pBootSettings->osAppSize);

    /* Check SAGE OS/APP compatibility with BFW version */
    rc = BSAGElib_P_Boot_CheckOsAppBFWVersion(hSAGElib, &osAppHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("***********************************************************************"));
        BDBG_ERR(("BOLT with BFW version 4.0.3 or later must be used with SAGE OS/APP 3.x."));
        BDBG_ERR(("Please upgrade BOLT."));
        BDBG_ERR(("***********************************************************************"));
        goto end;
    }

    /* Check if both SAGE images have triple signing mode */
    rc = BSAGElib_P_Boot_CheckSigningMode(ctx);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Set SAGE boot parameters information into Global SRAM GP registers */
    rc = BSAGElib_P_Boot_SetBootParams(ctx, pBootSettings, &osAppHolder);
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
    _BSAGElib_P_Boot_SetBootParam(Kernel, 0);
    _BSAGElib_P_Boot_SetBootParam(KernelSize, 0);
    _BSAGElib_P_Boot_SetBootParam(KernelSignature, 0);
    _BSAGElib_P_Boot_SetBootParam(SecondTierKey, 0);

    BDBG_LEAVE(BSAGElib_Boot_Clean);
    return;
}

static void
BSAGElib_P_Boot_PrintBinaryVersion(BSAGElib_Handle hSAGElib, BSAGElib_SageImageHolder *image) {
    BSAGElib_SageSecureHeader *header = (BSAGElib_SageSecureHeader *)image->header;
    const char *type = NULL;
    const char *subType = NULL;
    uint32_t index = _EndianSwap(header->ucHeaderIndex);
    char *verStr;

    if ((index & SAGE_HEADER_SECURE_IMAGE_TYPE_MASK) == SAGE_HEADER_TYPE_OS_APP) {
        type = "OS/APP";
        verStr = hSAGElib->BootImage_OSVerStr;
        switch (header->ucPlatformId[0]) {
             case 0:
                 subType = "GENERIC";
                 break;
             case 1:
                 subType = "MANUFACTURING TOOL";
                 break;
            case 2:
                 subType = "CUSTOM";
                 break;
            case 3:
                 subType = "SPECIAL";
                 break;
            default:
                 subType = "UNKNOWN";
                 break;
        }
    }
    else {
        verStr = hSAGElib->BootImage_BlVerStr;
        type = "BOOTLOADER";
        subType = "GENERIC";
    }

    if (header->ucSageSfwVersion[0] == 0 && header->ucSageSfwVersion[1] == 0 && header->ucSageSfwVersion[2] == 0) {
        BKNI_Snprintf(verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING,  "SAGE %s does not contain SAGE versioning information", type);
        BDBG_WRN(("%s", verStr));
    }
    else {
        if ((index & SAGE_HEADER_SECURE_IMAGE_TYPE_MASK) == SAGE_HEADER_TYPE_OS_APP) {
           BKNI_Snprintf (verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING, "SAGE %s Version [%s=%d.%d.%d.%d.%d, Secure mode=%d, Signing Tool=%d.%d.%d]",
                type,
                subType,
                header->ucSageSfwVersion[0],
                header->ucSageSfwVersion[1],
                header->ucSageSfwVersion[2],
                header->ucSageSfwVersion[3],
                header->ucSageSfwVersion[4],
                header->ucPlatformId[1],
                header->ucSageBlVersion[0],
                header->ucSageBlVersion[1],
                header->ucSageBlVersion[2]);
        }
        else {
          BKNI_Snprintf (verStr, SIZE_OF_BOOT_IMAGE_VERSION_STRING, "SAGE %s Version [%s=%d.%d.%d.%d.%d, Signing Tool=%d.%d.%d]",
                type,
                subType,
                header->ucSageSfwVersion[0],
                header->ucSageSfwVersion[1],
                header->ucSageSfwVersion[2],
                header->ucSageSfwVersion[3],
                header->ucSageSfwVersion[4],
                header->ucSageBlVersion[0],
                header->ucSageBlVersion[1],
                header->ucSageBlVersion[2]);
        }
        BDBG_LOG(("%s", verStr));
    }

}

void
BSAGElib_Boot_GetBinariesVersion(BSAGElib_Handle hSAGElib, char **ppBLVer, char **ppOSVer) {
    BDBG_ENTER(BSAGElib_Boot_GetBinariesVersion);
    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    if (ppBLVer != NULL)
        *ppBLVer = hSAGElib->BootImage_BlVerStr;

    if (ppOSVer != NULL)
        *ppOSVer = hSAGElib->BootImage_OSVerStr;

    BDBG_LEAVE(BSAGElib_Boot_GetBinariesVersion);
    return;
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
        BDBG_ERR(("%s - BHSM_InitialiseBypassKeysltos() fails %d", __FUNCTION__, rc));
        goto end;
    }

    hSAGElib->bBootPostCalled = true;

end:
    BDBG_LEAVE(BSAGElib_Boot_Post);
    return rc;
}
