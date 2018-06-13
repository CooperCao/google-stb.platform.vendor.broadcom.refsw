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
#include "bhsm_otp_msp_indexes.h"
#endif

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"

#include "bsagelib.h"
#include "bsagelib_boot.h"
#include "bsagelib_priv.h"
#include "priv/bsagelib_shared_types.h"
#include "priv/suif_sbl.h"

BDBG_MODULE(BSAGElib_boot_4x);

#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0))
#define RV_CONTROLLING_PARAMETERS_REVERSED (1)
#endif

/* Host to Sage communication buffers size */
#define SAGE_HOST_BUF_SIZE (32)

/* TODO: use the BHSM_SecondTierKey_t struct in HSM instead of this one */
#define BSAGELIB_2ND_TIER_RSAKEY_SIZE_BYTES (256)
#define BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES (256)

typedef struct {
    SUIF_PackageHeader  *header;
    uint32_t binarySize;
} BSAGElib_Sage_ImageHolder;

#define SAGEBL_TIMEOUT_MS 10000

#define SAGEBL_STATUS_OK    0x00000000

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

/****************************************
 * Local Functions
 ***************************************/
#define SAGE_FRAMEWORK_VERSION_CHECK 3
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
#define BFW_VERSION_CHECK_MAJOR 4
#define BFW_VERSION_CHECK_MINOR 1
#define BFW_VERSION_CHECK_SUBMINOR 3
#else
#define BFW_VERSION_CHECK_MAJOR 2
#define BFW_VERSION_CHECK_MINOR 1
#define BFW_VERSION_CHECK_SUBMINOR 3
#endif

static BERR_Code
BSAGElib_P_Boot_CheckFrameworkBFWVersion(
    BSAGElib_Handle hSAGElib,
    BSAGElib_Sage_ImageHolder *image)
{
    BERR_Code rc = BERR_UNKNOWN;
    SUIF_CommonHeader *pCommonHeader;

    if (hSAGElib == NULL || image == 0)
    {
       BDBG_ERR(("%s hSAGElib %p or image %p NULL", BSTD_FUNCTION, (void *)hSAGElib,(void *)image));
       rc = BERR_INVALID_PARAMETER;
       goto end;
    }

    pCommonHeader = &(image->header->imageHeader.common);

    BDBG_MSG(("SAGE Framework version major %d",pCommonHeader->imageVersion.major));
    if (pCommonHeader->imageVersion.major >= SAGE_FRAMEWORK_VERSION_CHECK)
    {
#if (BHSM_API_VERSION==1)
        BHSM_Capabilities_t hsmCaps;
        if( ( rc = BHSM_GetCapabilities(hSAGElib->core_handles.hHsm, &hsmCaps ) ) != BERR_SUCCESS )
        {
            BDBG_ERR(("couldn't read BFW version"));
            goto end;
        }
        else
        {
            BDBG_MSG(("BFW %d.%d.%d",hsmCaps.version.firmware.bseck.major, hsmCaps.version.firmware.bseck.minor, hsmCaps.version.firmware.bseck.subMinor));
            if ( BFW_VERSION_CHECK_MAJOR < hsmCaps.version.firmware.bseck.major )
            {
                /* major version is later than required version, return success */
                rc = BERR_SUCCESS;
                goto end;
            }
            else if ( BFW_VERSION_CHECK_MAJOR == hsmCaps.version.firmware.bseck.major )
            {
                if ( BFW_VERSION_CHECK_MINOR < hsmCaps.version.firmware.bseck.minor )
                {
                    /* minor version is later than required version, return success */
                    rc = BERR_SUCCESS;
                    goto end;
                }
                else if ( BFW_VERSION_CHECK_MINOR == hsmCaps.version.firmware.bseck.minor )
                {
                    if ( BFW_VERSION_CHECK_SUBMINOR <= hsmCaps.version.firmware.bseck.subMinor )
#else
        BHSM_ModuleCapabilities hsmCaps;
        BKNI_Memset( &hsmCaps, 0, sizeof(hsmCaps) );
        if ( ( rc = BHSM_GetCapabilities(hSAGElib->core_handles.hHsm, &hsmCaps ) ) != BERR_SUCCESS )
        {
            BDBG_ERR(("couldn't read BFW version"));
            goto end;
        }
        else
        {
            BDBG_MSG(("BFW %d.%d.%d",hsmCaps.version.bfw.major, hsmCaps.version.bfw.minor, hsmCaps.version.bfw.subminor));
            if ( BFW_VERSION_CHECK_MAJOR < hsmCaps.version.bfw.major )
            {
                /* major version is later than required version, return success */
                rc = BERR_SUCCESS;
                goto end;
            }
            else if ( BFW_VERSION_CHECK_MAJOR == hsmCaps.version.bfw.major )
            {
                if ( BFW_VERSION_CHECK_MINOR < hsmCaps.version.bfw.minor )
                {
                    /* minor version is later than required version, return success */
                    rc = BERR_SUCCESS;
                    goto end;
                }
                else if ( BFW_VERSION_CHECK_MINOR == hsmCaps.version.bfw.minor )
                {
                    if ( BFW_VERSION_CHECK_SUBMINOR <= hsmCaps.version.bfw.subminor )
#endif
                    {
                        /* subminor version is later than or equal to required version, return success */
                        rc = BERR_SUCCESS;
                        goto end;
                    }
                }
            }

            /* BFW version doesn't meet the requirement of SAGE */
            rc =  BERR_NOT_SUPPORTED;
        }
    }

end:
    return rc;
}

static BERR_Code
BSAGElib_P_Boot_CheckCompatibility(
    BSAGElib_Handle hSAGElib,
    BSAGElib_Sage_ImageHolder *img)
{
    BERR_Code rc = BERR_SUCCESS;
    const char *name = NULL;
    SUIF_CommonHeader *pCommonHeader = &(img->header->imageHeader.common);

    if(pCommonHeader->imageType == SUIF_ImageType_eSBL)
    {
        name = "Bootloader";
    }else if(pCommonHeader->imageType == SUIF_ImageType_eSSF)
    {
        name = "Framework ";
    }else
    {
        BDBG_ERR(("%s wrong image type %d in image file, must be SBL(%d) or SSF(%d) !", BSTD_FUNCTION,
            SUIF_ImageType_eSBL,SUIF_ImageType_eSSF,pCommonHeader->imageType));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    switch(hSAGElib->chipInfo.chipType)
    {
        case BSAGElib_ChipType_eZS:
            if(pCommonHeader->signingProfileName[0] != 'Z' && pCommonHeader->signingProfileName[1] != 'D')
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", name));
                BDBG_ERR(("* Invalid SAGE binary type %c%c.", pCommonHeader->signingProfileName[0],pCommonHeader->signingProfileName[1]));
                BDBG_ERR(("* Chipset Type is ZS!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eZB:
            /* return error only if chip is BRCM development part (ZS) */
            if(pCommonHeader->signingProfileName[0] != 'Z' && pCommonHeader->signingProfileName[1] != 'B')
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", name));
                BDBG_ERR(("* Invalid SAGE binary type %c%c.", pCommonHeader->signingProfileName[0],pCommonHeader->signingProfileName[1]));
                BDBG_ERR(("* Chipset Type is ZB!."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;

        case BSAGElib_ChipType_eCustomer1:
        case BSAGElib_ChipType_eCustomer:
            /* return error only if chip is BRCM development part (ZS) */
            if(pCommonHeader->signingProfileName[0] == 'Z' && pCommonHeader->signingProfileName[1] == 'D')
            {
                BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", name));
                BDBG_ERR(("* Invalid SAGE binary type %c%c.", pCommonHeader->signingProfileName[0],pCommonHeader->signingProfileName[1]));
                BDBG_ERR(("* Chipset Type is Customer."));
                BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

                rc = BERR_INVALID_PARAMETER;
                goto end;
            }
            break;
        default:
            BDBG_ERR(("*******  SAGE ERROR for '%s'  >>>>>", name));
            BDBG_ERR(("* Invalid chipset type."));
            BDBG_ERR(("*******  SAGE ERROR  <<<<<"));

            rc = BERR_INVALID_PARAMETER;
            goto end;
    }

end:
    return rc;
}

static BERR_Code
BSAGElib_P_Boot_SetmageInfo(
    BSAGElib_ImageInfo *pImageInfo,
    SUIF_CommonHeader *pCommonHeader)
{
    BERR_Code rc = BERR_SUCCESS;
    const char *typeStr = NULL;
    const char *subTypeStr = NULL;
    char *verStr;
    uint32_t version;

    if (pImageInfo == NULL || pCommonHeader == 0)
    {
       BDBG_ERR(("%s pImageInfo %p or pCommonHeader %p NULL", BSTD_FUNCTION, (void *)pImageInfo,(void *)pCommonHeader));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    pImageInfo->version[0] = pCommonHeader->imageVersion.major;
    pImageInfo->version[1] = pCommonHeader->imageVersion.minor;
    pImageInfo->version[2] = pCommonHeader->imageVersion.revision;
    pImageInfo->version[3] = pCommonHeader->imageVersion.branch;

    pImageInfo->signingToolVersion[0] = pCommonHeader->signingToolVersion.major;
    pImageInfo->signingToolVersion[1] = pCommonHeader->signingToolVersion.minor;
    pImageInfo->signingToolVersion[2] = pCommonHeader->signingToolVersion.revision;
    pImageInfo->signingToolVersion[3] = pCommonHeader->signingToolVersion.branch;

    pImageInfo->THLShortSig = 0;

    verStr = pImageInfo->versionString;

    if(pCommonHeader->imageType == SUIF_ImageType_eSBL)
    {
        typeStr = "Bootloader";
    }else if(pCommonHeader->imageType == SUIF_ImageType_eSSF)
    {
        typeStr = "Framework ";
    }else
    {
        BDBG_ERR(("%s wrong image type %d in image file, must be SBL(%d) or SSF(%d) !", BSTD_FUNCTION,
            SUIF_ImageType_eSBL,SUIF_ImageType_eSSF,pCommonHeader->imageType));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }


    switch (pCommonHeader->releaseType)
    {
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

end:
    return rc;
}
static BERR_Code BSAGElib_P_Boot_CheckTierKey(
    BSAGElib_Handle hSAGElib,
    SUIF_TierKeyPackage *pKey)
{
    BERR_Code rc = BERR_SUCCESS;
    BCMD_Otp_CmdMsp_e otp_market_id_index;
    uint32_t *pMarketId_LE,*pMarketIdMask_LE;
    uint32_t otp_market_id, otp_market_id_lock, key_market_id,key_market_id_mask;

#if BHSM_ZEUS_VER_MAJOR == 4
    if(pKey->controllingParams.marketIdSelect == 0)
    {
        otp_market_id_index = BCMD_Otp_CmdMsp_eMarketId;
    }else if(pKey->controllingParams.marketIdSelect == 1)
    {
        otp_market_id_index = BCMD_Otp_CmdMsp_eMarketId1;
    }
#elif BHSM_ZEUS_VER_MAJOR >= 5
    if(pKey->controllingParams.marketIdSelect == 0)
    {
        otp_market_id_index = BHSM_OTPMSP_MARKET_ID0;
    }else if(pKey->controllingParams.marketIdSelect == 1)
    {
        otp_market_id_index = BHSM_OTPMSP_MARKET_ID1;
    }
#else
#error unsupported Zeus version
#endif
    else{
        BDBG_WRN(("%s - Tier Key's marketIdSelect %d is wrong.", BSTD_FUNCTION,pKey->controllingParams.marketIdSelect));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    rc = BSAGElib_P_GetOtp(hSAGElib, otp_market_id_index, &otp_market_id, &otp_market_id_lock, "market id");
    if (rc != BERR_SUCCESS) { goto end; }

    pMarketId_LE = (uint32_t *) pKey->controllingParams.marketId_LE;
    pMarketIdMask_LE = (uint32_t *) pKey->controllingParams.marketIdMask_LE;

    key_market_id = SUIF_Get32(pMarketId,LE);
    key_market_id_mask = SUIF_Get32(pMarketIdMask,LE);

    BDBG_MSG(("%s - marketID key-%x(mask %x) otp-%x(lock %x) (key marketID select %d)", BSTD_FUNCTION,key_market_id,key_market_id_mask,otp_market_id,otp_market_id_lock,pKey->controllingParams.marketIdSelect));

    if((otp_market_id&key_market_id_mask) != (key_market_id&key_market_id_mask))
    {
        BDBG_WRN(("%s: Key market id 0x%08x(mask 0x%08x) does not match chip OTP 0x%08x",
                  BSTD_FUNCTION, key_market_id,key_market_id_mask,otp_market_id));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
end:
    return rc;
}

static SUIF_TierKeyPackage *
BSAGElib_P_Boot_GetTierKey(
    BSAGElib_Handle hSAGElib,
    SUIF_TierKeyPackage *pTierKey_First)
{
    SUIF_TierKeyPackage *tierkey = NULL;
    SUIF_TierKeyPackage *currentkey = NULL;
    int i=0;

    if(pTierKey_First == NULL)
    {
        BDBG_ERR(("%s Invalid pTierKey_First %p",BSTD_FUNCTION,(void *)pTierKey_First));
        goto end;
    }

    for(currentkey=pTierKey_First,i=0; i< SUIF_TIER_KEY_PACKAGES_NUM; i++,currentkey++) /* 3 Tier keys availabe from pTierKey_First */
    {
        BDBG_MSG(("%s - Check Tier Key %d@%p", BSTD_FUNCTION,i,(void *)currentkey));
        if(BSAGElib_P_Boot_CheckTierKey(hSAGElib,currentkey) == BERR_SUCCESS)
        {
            tierkey = currentkey;
            break;
        }
    }
end:
    BDBG_MSG(("%s - found Tier Key (%d)@%p", BSTD_FUNCTION,i,(void *)tierkey));
    if(tierkey == NULL)
        BDBG_ERR(("%s NULL Tier Key, pTierKey_First %p",BSTD_FUNCTION,(void *)pTierKey_First));
    return tierkey;
}
static uint8_t *
BSAGElib_P_Boot_GetSignature(
    uint8_t *signature_start,
    SUIF_TierKeyPackage *pTierKey_First,
    SUIF_TierKeyPackage *pTierKey)
{
    uint8_t *signature = NULL;

    if(signature_start == NULL
     ||pTierKey_First == NULL
     ||pTierKey == NULL)
    {
        BDBG_ERR(("%s Invalid parameter! signature_start %p pTierKey_First %p,pTierKey %p",
            BSTD_FUNCTION,signature_start,(void *)pTierKey_First,(void *)pTierKey));
        goto end;
    }

    if(pTierKey == pTierKey_First
     ||pTierKey == pTierKey_First + 1)
    {
        signature = signature_start;
        goto end;
    }

    if(pTierKey == pTierKey_First + 2)
    {
        signature = signature_start + SUIF_TIER_RSAKEY_SIG_SIZE_BYTES;
        goto end;
    }

end:
    if(signature == NULL)
        BDBG_ERR(("%s NULL signature! signature_start %p pTierKey_First %p,"
                  " pTierKey %p tierKeyPackageSize %u",BSTD_FUNCTION,
                  (void *)signature_start,(void *)pTierKey_First,
                  (void *)pTierKey,(unsigned int)sizeof(SUIF_TierKeyPackage)));
    return signature;
}
/* parse image according image type given(SBL or SSF)
 *
 * hSAGElib,      --[in] SAGElib handle
 * imageType,     --[in] the tpye of this image, SBL or SSF
 * pBinary,       --[in] the pointer of image
 * binarySize,    --[in] image file size
 * holder         --[out] the information of image(the pointer and size)
 * return error if image if not the type given, or image wrong.
 * return BERR_NOT_SUPPORTED if the image tpye is not SUIF
 */
static BERR_Code
BSAGElib_P_Boot_ParseSageImage(
    BSAGElib_Handle hSAGElib,
    SUIF_ImageType_e imageType,
    uint8_t *pBinary,
    uint32_t binarySize,
    BSAGElib_Sage_ImageHolder* holder)
{
    BERR_Code rc = BERR_SUCCESS;
    SUIF_CommonHeader *pCommonHeader = (SUIF_CommonHeader *)pBinary;
    uint32_t magicNumber, *pMagicNumber_BE;

    if (pBinary == NULL || binarySize == 0 || holder == NULL)
    {
        BDBG_ERR(("%s pBinary %p or holder %p can not be NULL,binarySize %d can not be 0 ",
            BSTD_FUNCTION, (void *)pBinary,(void *)holder,binarySize));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    pMagicNumber_BE = (uint32_t *)pCommonHeader->magicNumber_BE;
    magicNumber = SUIF_Get32(pMagicNumber,BE);
    switch(imageType)
    {
    case SUIF_ImageType_eSBL:
        if(magicNumber != SUIF_MagicNumber_eSBL)
        {
            BDBG_MSG(("%s Not SUIF Magic number 0x%x for SBL",BSTD_FUNCTION,magicNumber));
            rc = BERR_NOT_SUPPORTED;
            goto end;
        }
        rc = BSAGElib_P_Boot_SetmageInfo(&hSAGElib->bootloaderInfo,pCommonHeader);
        break;
    case SUIF_ImageType_eSSF:
        if(magicNumber != SUIF_MagicNumber_eSSF)
        {
            BDBG_ERR(("%s Not SUIF Magic number 0x%x for SSF",BSTD_FUNCTION,magicNumber));
            rc = BERR_NOT_SUPPORTED;
            goto end;
        }
        rc = BSAGElib_P_Boot_SetmageInfo(&hSAGElib->frameworkInfo,pCommonHeader);
        break;
    default:
        BDBG_ERR(("%s wrong image type %d to parse, must be SBL(%d) or SSF(%d) !", BSTD_FUNCTION,
            SUIF_ImageType_eSBL,SUIF_ImageType_eSSF,imageType));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    holder->header = (SUIF_PackageHeader *)pBinary;
    holder->binarySize = binarySize;

end:
    return rc;
}

static BERR_Code BSAGElib_P_WaitSBL(BSAGElib_Handle hSAGElib)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t val,i;

    for (i=0; i<SAGEBL_TIMEOUT_MS; i++)
    {
        _BSAGElib_P_Boot_GetBootParam(HostSageComControl,val);

        if ((val & HOSTSAGE_CONTROL_SRdy_MASK)>> HOSTSAGE_CONTROL_SRdy_SHIFT == 1 )
        {
            break;
        } else {
            BKNI_Sleep(1);
        }
        if (((i+1) % 1000) == 0) {
            BDBG_LOG(("* SBL state(0x%x) waiting to be ready",val));
        }
    }

    if ((val & HOSTSAGE_CONTROL_SRdy_MASK)>> HOSTSAGE_CONTROL_SRdy_SHIFT == 0 )
    {
        BDBG_ERR(("%s - SAGEBL boot timeout, status 0x%x", BSTD_FUNCTION,val));
        rc = BERR_TRACE(BERR_TIMEOUT);
    }

    return rc;
}

static BERR_Code
BSAGElib_P_Boot_Framework(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootSettings *pBootSettings,
    BSAGElib_Sage_ImageHolder* frameworkHolder)
{
    BERR_Code rc = BERR_SUCCESS;
    SUIF_SBLImageHeader *header = (SUIF_SBLImageHeader *)frameworkHolder->header;
    uint16_t *pHeader_length_BE  = (uint16_t *)(header->common.headerLength_BE);
    uint32_t header_length  = SUIF_Get16(pHeader_length,BE);
    uint32_t offset = 0,val = 0,i = 0;
    BSAGElib_InOutContainer *io_container = NULL;

    BDBG_ENTER(BSAGElib_P_Boot_Framework);

    rc = BSAGElib_P_WaitSBL(hSAGElib);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SBL not ready", BSTD_FUNCTION));
        rc = BERR_TIMEOUT;
        goto end;
    }

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

    _BSAGElib_P_Boot_SetBootParam(LastError, 0);
    _BSAGElib_P_Boot_SetBootParam(BootStatus, BSAGElibBootStatus_eNotStarted);

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
        uint32_t sageVklMask = 0;
        BHSM_KeyLadderInfo info;
        BERR_Code rc_local;

        BKNI_Memset(&info, 0, sizeof(info));
        rc_local = BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle1, &info);
        if(rc_local != BERR_SUCCESS) { rc = BERR_TRACE( rc_local ); goto end; }

        if(info.index < 32)
        {
            sageVklMask = 1<<info.index;
        }

        rc_local = BHSM_KeyLadder_GetInfo(hSAGElib->vklHandle2, &info);
        if(rc_local != BERR_SUCCESS) { rc = BERR_TRACE( rc_local ); goto end; }

        if(info.index < 32)
        {
            sageVklMask |= 1<<info.index;
        }

        /* HSM internally remaps VKL ID. We need to remap VKL ID back to actual VKL ID before to send to SAGE. */
        _BSAGElib_P_Boot_SetBootParam(SageVklMask, sageVklMask);
         BDBG_LOG(("%s:%u BSAGElib_P_Boot_SetBootParam(SageVklMask,0x%08X)", BSTD_FUNCTION,__LINE__,sageVklMask));
   }
#endif

    offset = hSAGElib->i_memory_map.addr_to_offset(header);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert SAGE framework header address to offset", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    hSAGElib->i_memory_sync.flush(header, frameworkHolder->binarySize);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeader, offset);
    _BSAGElib_P_Boot_SetBootParam(SageFrameworkHeaderSize, header_length);

    /* prepare the coomand to SBL */
    io_container = hSAGElib->i_memory_alloc.malloc(sizeof(*io_container));
    if(io_container == NULL)
    {
        BDBG_ERR(("%s - Cannot space for sage bl io container", BSTD_FUNCTION));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }

    BKNI_Memset(io_container, 0, sizeof(*io_container));
    io_container->basicIn[0] = Hsc_LoadSSF;
    io_container->basicIn[1] = 0;   /* reserved */
    io_container->basicIn[2] = offset;
    io_container->basicIn[3] = frameworkHolder->binarySize;

    offset = hSAGElib->i_memory_map.addr_to_offset((void *)io_container);
    if(offset == 0) {
        BDBG_ERR(("%s - Cannot convert sage io container address to offset", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
    hSAGElib->i_memory_sync.flush((void *)io_container, sizeof(*io_container));
    _BSAGElib_P_Boot_SetBootParam(HostSageComBuffer, offset);

    _BSAGElib_P_Boot_GetBootParam(HostSageComControl,val);

    /* Check the SBL status is fine */
    if (((val & HOSTSAGE_CONTROL_SRdy_MASK)>> HOSTSAGE_CONTROL_SRdy_SHIFT == 0) /* sagebl should be ready, or return error */
      ||((val & HOSTSAGE_CONTROL_ORdy_MASK)>> HOSTSAGE_CONTROL_ORdy_SHIFT != 0) /* sagebl out should not be ready, or return error */
      ||((val & HOSTSAGE_CONTROL_IRdy_MASK)>> HOSTSAGE_CONTROL_IRdy_SHIFT != 0))/* sagebl In should not be ready, or return error */
    {
        BDBG_ERR(("%s - SAGEBL status 0x%x wrong", BSTD_FUNCTION,val));
        rc = BERR_UNKNOWN;
        goto end;
    }

    /* Tell SBL input is ready */
    _BSAGElib_P_Boot_GetBootParam(HostSageComControl,val);
    val &= ~HOSTSAGE_CONTROL_IRdy_MASK;
    val |= 0x01<<HOSTSAGE_CONTROL_IRdy_SHIFT;
    _BSAGElib_P_Boot_SetBootParam(HostSageComControl, val);

    /* Wait for it to be done */
    for (i=0; i<SAGEBL_TIMEOUT_MS; i++)
    {
        _BSAGElib_P_Boot_GetBootParam(HostSageComControl,val);

        if (((val & HOSTSAGE_CONTROL_SRdy_MASK)>> HOSTSAGE_CONTROL_SRdy_SHIFT == 0) /* sage ready bit changed, break; */
          ||((val & HOSTSAGE_CONTROL_ORdy_MASK)>> HOSTSAGE_CONTROL_ORdy_SHIFT != 0))/* sage out ready now, break; */
        {
            break;
        } else {
            BKNI_Sleep(1);
        }
        if (((i+1) % 1000) == 0) {
            BDBG_LOG(("*"));
        }
    }

    if (i >= 1000)
        BDBG_LOG((" "));

    if (((val & HOSTSAGE_CONTROL_SRdy_MASK)>> HOSTSAGE_CONTROL_SRdy_SHIFT == 1) /* sage ready */
      &&((val & HOSTSAGE_CONTROL_ORdy_MASK)>> HOSTSAGE_CONTROL_ORdy_SHIFT == 1) /* sage out ready */
      &&(io_container->basicOut[0] == SAGEBL_STATUS_OK)) /* boot status,sucess; */
    {
        rc = BERR_SUCCESS;
    } else {
        BDBG_ERR(("%s - boot framework ready 0x%x, status 0x%x wrong", BSTD_FUNCTION,val,io_container->basicOut[0]));
        rc = BERR_UNKNOWN;
    }

    /* Acknowledge output processed */
    _BSAGElib_P_Boot_GetBootParam(HostSageComControl,val);
    val &= ~HOSTSAGE_CONTROL_ORdy_MASK;
    val |= 0x00<<HOSTSAGE_CONTROL_ORdy_SHIFT;
    _BSAGElib_P_Boot_SetBootParam(HostSageComControl, val);
end:
    if(io_container){
        BKNI_Memset((void *)io_container,0,sizeof(*io_container));
        hSAGElib->i_memory_alloc.free((void *)io_container);
    }

    BDBG_LEAVE(BSAGElib_P_Boot_Framework);
    return rc;
}

static BERR_Code
BSAGElib_P_Boot_ResetSage(
    BSAGElib_Handle hSAGElib,
    BSAGElib_Sage_ImageHolder *blImg)
{
    BERR_Code rc = BERR_SUCCESS;
    SUIF_CommonHeader *pCommon = (SUIF_CommonHeader *)blImg->header;
    const SUIF_SBLSpecificHeader *pSBL = &(((SUIF_SBLImageHeader *)blImg->header)->sbl);

    uint32_t *pDataSize_BE = (uint32_t *)(pCommon->dataSize_BE);
    uint32_t *pTextSize_BE = (uint32_t *)(pCommon->textSize_BE);
    uint32_t text_data_len = SUIF_Get32(pTextSize,BE) + SUIF_Get32(pDataSize,BE);

    uint32_t *pMeta_data_size_BE  = (uint32_t *)(pCommon->metadataSize_BE);
    uint32_t meta_data_size  = SUIF_Get32(pMeta_data_size,BE);

    uint16_t *pPkg_pad_size_BE = (uint16_t *)(pCommon->packagePadSize_BE);
    uint32_t pkg_pad_size    = SUIF_Get16(pPkg_pad_size,BE);

    SUIF_PackageFooter *footer = (SUIF_PackageFooter *)
                ((uint8_t *)blImg->header + sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size);

    SUIF_TierKeyPackage *pTierKey_First = (blImg->header)->tierKeyPackages;
    uint8_t *text = (uint8_t *)blImg->header + sizeof(SUIF_PackageHeader);
    uint8_t *image_controlling_parameters = (uint8_t *)&(footer->textVerification.controllingParameters);
    uint8_t *signature_start = footer->textVerification.signature0;

#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
    SUIF_RegionVerificationControllingParameters *pControlling_parameters = (SUIF_RegionVerificationControllingParameters *)image_controlling_parameters;
#endif

    uint16_t *pHeader_length_BE  = (uint16_t *)(pCommon->headerLength_BE);
    uint32_t header_length  = SUIF_Get16(pHeader_length,BE);

    uint16_t *pCntrl_params_sz_BE = (uint16_t *)(pCommon->controllingParamsSize_BE);
    uint32_t cntrl_params_sz = SUIF_Get16(pCntrl_params_sz,BE);

    uint16_t *pTierKeyPackageSize_BE = (uint16_t *)(pCommon->tierKeyPackageSize_BE);
    uint16_t tierKeyPackageSize = SUIF_Get16(pTierKeyPackageSize,BE);

    uint32_t *pGlobalOwnerIdSelect_BE = (uint32_t *)(pSBL->keyladder.globalOwnerIdSelect_BE);
    uint32_t globalOwnerIdSelect = SUIF_Get32(pGlobalOwnerIdSelect,BE);

    uint32_t *pStbOwnerId_BE     = (uint32_t *)(pSBL->keyladder.stbOwnerIdSelect_BE);
    uint32_t *pCaVendorId_BE     = (uint32_t *)(pSBL->keyladder.caVendorId_BE);
    uint32_t *pGlobalKeyIndex_BE = (uint32_t *)(pSBL->keyladder.globalKeyIndex_BE);

    uint8_t *signature;
    SUIF_TierKeyPackage *pTierKey;

    if(header_length != sizeof(SUIF_DummyImageHeader))
    {
        BDBG_ERR(("%s - image header length (%u)wrong should be %u bytes", BSTD_FUNCTION,
            header_length,(unsigned int)sizeof(SUIF_DummyImageHeader)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(tierKeyPackageSize != sizeof(SUIF_TierKeyPackage))
    {
        BDBG_ERR(("%s - tierKeyPackageSize (%u)wrong should be %u bytes", BSTD_FUNCTION,
            tierKeyPackageSize,(unsigned int)sizeof(SUIF_TierKeyPackage)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(cntrl_params_sz != sizeof(SUIF_RegionVerificationControllingParameters))
    {
        BDBG_ERR(("%s - cntrl_params_sz (%u)wrong should be %u bytes", BSTD_FUNCTION,
            cntrl_params_sz,(unsigned int)sizeof(SUIF_RegionVerificationControllingParameters)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(blImg->binarySize != sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + sizeof(SUIF_PackageFooter))
    {
        BDBG_ERR(("%s - Image size (%u)wrong should be %u bytes(PKG header %u, Text+Data %u, mata Data %u, Pad %u, Footer %u)",
            BSTD_FUNCTION, blImg->binarySize,
            (unsigned int)sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + (unsigned int)sizeof(SUIF_PackageFooter),
            (unsigned int)sizeof(SUIF_PackageHeader),  text_data_len,  meta_data_size,  pkg_pad_size,  (unsigned int)sizeof(SUIF_PackageFooter)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(pCommon->signingScheme == SUIF_SigningScheme_eTriple)
    {
        pTierKey =  BSAGElib_P_Boot_GetTierKey(hSAGElib,pTierKey_First);
        signature = BSAGElib_P_Boot_GetSignature(signature_start,pTierKey_First,pTierKey);
    }else
    {
        pTierKey =  pTierKey_First;
        signature = signature_start;
        if(BSAGElib_P_Boot_CheckTierKey(hSAGElib,pTierKey) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Tier key market ID wrong !", BSTD_FUNCTION));
        }
    }

    BDBG_MSG((" pTierKey index %d (%p,start %p pkg size %u) signature index %d (%p,start %p,size 0x%x)",
        (int)(pTierKey-pTierKey_First),(void *)pTierKey,(void *)pTierKey_First,(unsigned int)sizeof(SUIF_TierKeyPackage),
        (int)(signature-signature_start)/SUIF_TIER_RSAKEY_SIG_SIZE_BYTES,(void *)signature,(void *)signature_start,SUIF_TIER_RSAKEY_SIG_SIZE_BYTES));

    { /* start BHSM_API_VERSION specific code */
#if (BHSM_API_VERSION==1)
        BCMD_VKLID_e vkl_id = BCMD_VKL_eMax;

        /* if OTP_SAGE_VERIFY_ENABLE_BIT: verify SAGE boot loader */
        {
            BHSM_VerifySecondTierKeyIO_t secondTierKey;
            SUIF_TierKeyControllingParameters *pSecondTierKey;

            BKNI_Memset(&secondTierKey, 0, sizeof(BHSM_VerifySecondTierKeyIO_t));

            if(pCommon->key0Select == SUIF_Key0Select_eIndex )
            {
                secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0;
            }
            else {
                secondTierKey.eFirstTierRootKeySrc = BCMD_FirstTierKeyId_eKey0Prime;
            }
            secondTierKey.eKeyIdentifier = BCMD_SecondTierKeyId_eKey3;

            pSecondTierKey = (SUIF_TierKeyControllingParameters *)pTierKey;
            secondTierKey.keyAddr = hSAGElib->i_memory_map.addr_to_offset(pSecondTierKey);
            if(secondTierKey.keyAddr == 0) {
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
        {
            uint32_t *pModuleId_BE       = (uint32_t *)(pSBL->keyladder.moduleId_BE);
            BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;

            /* GRK 1 */
            BKNI_Memset(&generateRouteKeyIO, 0, sizeof(BHSM_GenerateRouteKeyIO_t));
            generateRouteKeyIO.keyLadderSelect              = BCMD_eFWKL;
            generateRouteKeyIO.bASKM3DesKLRootKeySwapEnable = false;
            generateRouteKeyIO.client                 = BHSM_ClientType_eHost;
            if(globalOwnerIdSelect != 3)
            {
                BDBG_ERR(("%s - globalOwnerIdSelects 0x%x wrong in image,should be:3", BSTD_FUNCTION,globalOwnerIdSelect));
                rc = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto end;
            }
            generateRouteKeyIO.globalKeyOwnerId = BHSM_OwnerIDSelect_eUse1; /* translate to 3 */

            generateRouteKeyIO.cusKeySwizzle0aEnable       = false;
            generateRouteKeyIO.askm.configurationEnable    = true;
            generateRouteKeyIO.askm.moduleId               = SUIF_Get32(pModuleId,BE);
            generateRouteKeyIO.askm.stbOwnerId     = SUIF_Get32(pStbOwnerId,BE);
            generateRouteKeyIO.askm.caVendorId     = SUIF_Get32(pCaVendorId,BE);
            generateRouteKeyIO.askm.maskKeySelect  = BCMD_ASKM_MaskKeySel_eFixedMaskKey;
            generateRouteKeyIO.keyTweak             = BCMD_KeyTweak_eNoTweak;
            generateRouteKeyIO.virtualKeyLadderID = vkl_id;
            generateRouteKeyIO.customerSubMode    = BCMD_CustomerSubMode_eSAGE_BL_DECRYPT;
            generateRouteKeyIO.keyMode            = BCMD_KeyMode_eRegular;
            generateRouteKeyIO.ucKeyDataLen       = 16;
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSBL->keyladder.procIn3, 16 );
            generateRouteKeyIO.unKeySlotNum     = 0;
            generateRouteKeyIO.caKeySlotType    = BCMD_XptSecKeySlot_eType3;
            generateRouteKeyIO.bASKMModeEnabled = true;
            generateRouteKeyIO.bSwapAesKey      = false;
            generateRouteKeyIO.key3Op           = BCMD_Key3Op_eKey3NoProcess;
            generateRouteKeyIO.keyDestIVType    = BCMD_KeyDestIVType_eNoIV;
            generateRouteKeyIO.keyLadderType    = BCMD_KeyLadderType_eAES128;
            generateRouteKeyIO.rootKeySrc       = BCMD_RootKeySrc_eASKMGlobalKey;
            generateRouteKeyIO.AskmGlobalKeyIndex = SUIF_Get32(pGlobalKeyIndex,BE);
            generateRouteKeyIO.globalKeyVersion = 0;
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
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSBL->keyladder.procIn4, 16);
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
            BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSBL->keyladder.procIn5, 16);
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

        { /* setup and verify region */
            BHSM_RegionConfiguration_t  verifyIO;
            BHSM_RegionStatus_t status;

            BKNI_Memset(&verifyIO, 0, sizeof(BHSM_RegionConfiguration_t));
            verifyIO.cpuType = BCMD_MemAuth_CpuType_eScpu;
            verifyIO.ucIntervalCheckBw = 0x10;
            verifyIO.SCBBurstSize = BCMD_ScbBurstSize_e64;
            verifyIO.unRSAKeyID = 0x03;

            verifyIO.region.startAddress = hSAGElib->i_memory_map.addr_to_offset(text);
            if(verifyIO.region.startAddress == 0) {
                BDBG_ERR(("%s - Cannot convert bootloader address to offset", BSTD_FUNCTION));
                goto end;
            }
            verifyIO.region.endAddress = verifyIO.region.startAddress + (text_data_len) - 1;

            {
                verifyIO.signature.startAddress = hSAGElib->i_memory_map.addr_to_offset(signature);
                if(verifyIO.signature.startAddress == 0) {
                    BDBG_ERR(("%s - Cannot convert bootloader signature address to offset", BSTD_FUNCTION));
                    goto end;
                }
                hSAGElib->i_memory_sync.flush(signature, 256);
            }
            verifyIO.signature.endAddress = verifyIO.signature.startAddress + 256 - 1;

            {
                verifyIO.bgCheck = 0x0D;
                verifyIO.instrChecker = 0x0C;

                {
                    verifyIO.vkl = vkl_id;
                    verifyIO.keyLayer = BCMD_KeyRamBuf_eKey5;
                }

                {
                    verifyIO.codeRelocatable = 0;
                    verifyIO.unEpoch = 0;
                    verifyIO.unMarketID = 0;
                    verifyIO.unMarketIDMask = 0;
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
                    verifyIO.ucSigVersion = 0x01;
                    verifyIO.ucSigType = 0x04;
                    {
                        verifyIO.ucEpochSel = 0x3;
                    }

                    {
                        /* set the bits based on EPOCH version */
                        verifyIO.unEpoch |= (1 << pControlling_parameters->epoch) - 1;
                    }
#endif
                    {
                        verifyIO.unEpochMask = 0xFF;
                    }
                }
            }

            /* SAGE BL AR */
            /* Share the SAGE BL AR version through SAGE Global SRAM */
            _BSAGElib_P_Boot_SetBootParam( SageBootloaderEpochVersion, pCommon->epochVersion );

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

        rc = BSAGElib_P_WaitSBL(hSAGElib);
end:
        if(vkl_id != BCMD_VKL_eMax)
        {
            BSAGElib_iLockHsm();
            BHSM_FreeVKL(hSAGElib->core_handles.hHsm, vkl_id);
            BSAGElib_iUnlockHsm();
        }

#else /* #if (BHSM_API_VERSION==1) */
        BHSM_RvRsaHandle hRvRsa = NULL;
        BHSM_RvRsaAllocateSettings rsaAllocConf;
        BHSM_RvRsaSettings rsaConf;
        BHSM_KeyLadderHandle hKeyLadder = NULL;
        BHSM_KeyLadderAllocateSettings ladderAllocConf;
        BHSM_KeyLadderSettings ladderConf;
        BHSM_KeyLadderLevelKey ladderLevelKey;
        SUIF_TierKeyControllingParameters *pRsaKey;   /* BCMD_* is not a good name for this type  */
        BHSM_RvRegionHandle hRv = NULL;
        BHSM_RvRegionAllocateSettings rvAllocConf;
        BHSM_RvRegionSettings rvConf;
        uint8_t *pSignature;
        BMMA_DeviceOffset startAddress;
        BHSM_RvRegionStatus regionStatus;
        unsigned count;
        uint8_t *controlling_parameters=NULL;

        _BSAGElib_P_Boot_SetBootParam(HostSageComControl, 0);

        /* Share the SAGE BL AR version through SAGE Global SRAM */
        _BSAGElib_P_Boot_SetBootParam( SageBootloaderEpochVersion, pCommon->epochVersion );

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
            if( pCommon->key0Select == SUIF_Key0Select_eIndex ) {
                rsaConf.rootKey = BHSM_RvRsaRootKey_e0;
            }
            else {
                rsaConf.rootKey = BHSM_RvRsaRootKey_e0Prime;
            }

            pRsaKey = (SUIF_TierKeyControllingParameters *)pTierKey;
            rsaConf.keyOffset = hSAGElib->i_memory_map.addr_to_offset( pRsaKey );
            if( rsaConf.keyOffset == 0 ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }

            BSAGElib_iLockHsm();
            rc = BHSM_RvRsa_SetSettings( hRvRsa,  &rsaConf );
            BSAGElib_iUnlockHsm();
            if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }
        }

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
            ladderConf.root.askm.caVendorIdScope    = BHSM_KeyLadderCaVendorIdScope_eFixed;
            ladderConf.root.askm.stbOwnerSelect     = SUIF_Get32(pStbOwnerId,BE);
            ladderConf.root.askm.caVendorId         = SUIF_Get32(pCaVendorId,BE);

            ladderConf.root.type = BHSM_KeyLadderRootType_eGlobalKey;
            ladderConf.root.globalKey.index = SUIF_Get32(pGlobalKeyIndex,BE);

            BDBG_MSG(("globalOwnerIdSelect %d",globalOwnerIdSelect));
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
            if(globalOwnerIdSelect != 3) /* BCMD_OwnerIDSelect_eUse1 = 3, Zeus 4*/
            {
                BDBG_ERR(("%s - globalOwnerIdSelects 0x%x wrong in image,should be:3", BSTD_FUNCTION,globalOwnerIdSelect));
#else
            if(globalOwnerIdSelect != 2) /* bspConfig.in.globalKeyOwnerIdSelect = 2,zeus 5 */
            {
                BDBG_ERR(("%s - globalOwnerIdSelects 0x%x wrong in image,should be:2", BSTD_FUNCTION,globalOwnerIdSelect));
#endif
                rc = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto end;
            }
            /* BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne will map to BSP 2 in Zeus 5, BSP 3 in Zeus 4 */
            ladderConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne;

            BSAGElib_iLockHsm();
            rc = BHSM_KeyLadder_SetSettings( hKeyLadder, &ladderConf );
            BSAGElib_iUnlockHsm();
            if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

            /* set level 3 key */
            BKNI_Memset( &ladderLevelKey, 0, sizeof(ladderLevelKey) );

            ladderLevelKey.level = 3;
            BKNI_Memcpy( ladderLevelKey.ladderKey, pSBL->keyladder.procIn3, SUIF_AES_PROCIN_SIZE_BYTES );
            ladderLevelKey.ladderKeySize = 128;
            ladderLevelKey.route.destination = BHSM_KeyLadderDestination_eNone;

            BSAGElib_iLockHsm();
            rc = BHSM_KeyLadder_GenerateLevelKey( hKeyLadder, &ladderLevelKey );
            BSAGElib_iUnlockHsm();
            if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

            /* set level 4 key */
            ladderLevelKey.level = 4;
            BKNI_Memcpy( ladderLevelKey.ladderKey, pSBL->keyladder.procIn4, SUIF_AES_PROCIN_SIZE_BYTES );
            ladderLevelKey.ladderKeySize = 128;

            BSAGElib_iLockHsm();
            rc = BHSM_KeyLadder_GenerateLevelKey( hKeyLadder, &ladderLevelKey );
            BSAGElib_iUnlockHsm();
            if( rc != BERR_SUCCESS ){ BERR_TRACE(rc); goto end; }

            /* set level 5 key */
            ladderLevelKey.level = 5;
            BKNI_Memcpy( ladderLevelKey.ladderKey, pSBL->keyladder.procIn5, SUIF_AES_PROCIN_SIZE_BYTES );
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

        pSignature = signature;
        if( !pSignature ){ rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }
        startAddress = hSAGElib->i_memory_map.addr_to_offset(text);
        if( startAddress == 0 ){ rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto end; }

        /* configure region verification. */
        BKNI_Memset( &rvConf, 0, sizeof(rvConf) );
        rvConf.intervalCheckBandwidth = 0x10;
#if 0
        rvConf.scbBurstSize = 64;
        rvConf.range[0].startAddress = startAddress;  /* TODO SHOUD BE LOCAL */
        rvConf.range[0].size = blImg->text_data_len;
        BKNI_Memcpy( &rvConf.signature, pSignature, sizeof(rvConf.signature) );
#else
        rvConf.range[0].address = startAddress;
        rvConf.range[0].size = text_data_len;
        rvConf.signature.address = hSAGElib->i_memory_map.addr_to_offset(pSignature);
        rvConf.signature.size = BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES;
#endif
        hSAGElib->i_memory_sync.flush(text, text_data_len);
        hSAGElib->i_memory_sync.flush(signature, BSAGELIB_2ND_TIER_RSAKEY_SIG_SIZE_BYTES);

#if RV_CONTROLLING_PARAMETERS_REVERSED
        controlling_parameters = hSAGElib->i_memory_alloc.malloc(sizeof(SUIF_RegionVerificationControllingParameters));
        {
            uint32_t k; uint8_t* s = image_controlling_parameters;
            for(k=0; k<sizeof(SUIF_RegionVerificationControllingParameters); k+=4){
                controlling_parameters[k  ]= s[k+3];
                controlling_parameters[k+1]= s[k+2];
                controlling_parameters[k+2]= s[k+1];
                controlling_parameters[k+3]= s[k  ];
            }
        }
        hSAGElib->i_memory_sync.flush(controlling_parameters, sizeof(SUIF_RegionVerificationControllingParameters));
#else
        controlling_parameters = image_controlling_parameters;
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

        {
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
            BHSM_RvSignatureType signatureType;
            uint32_t *pMarketId_BE = (uint32_t *)(pControlling_parameters->marketId_BE);
            uint32_t *pMarketIdMask_BE = (uint32_t *)(pControlling_parameters->marketIdMask_BE);

            switch(pControlling_parameters->signatureType)
            {
            case 1: /* BCMD_SigType_eKeys */
                signatureType = BHSM_RvSignatureType_eKeys;
                break;
            case 4: /* BCMD_SigType_eCode*/
                signatureType = BHSM_RvSignatureType_eCode;
                break;
            case 6: /* BCMD_SigType_ePCIEWinSize:*/
                signatureType = BHSM_RvSignatureType_ePciEWinSize;
                break;
            default: /* BCMD_SigType_eAssymUnlock = 5, BCMD_SigType_eBootCode = 2,BCMD_SigType_eBootParams = 3,*/
                signatureType = BHSM_RvSignatureType_eMax;
                break;
            }
            rvConf.epoch = pControlling_parameters->epoch;
            rvConf.marketId = SUIF_Get32(pMarketId,BE);
            rvConf.marketIdMask = SUIF_Get32(pMarketIdMask,BE);
            rvConf.codeRelocatable = !pControlling_parameters->noReloc;
            rvConf.signatureType = signatureType;
            rvConf.signatureVersion = pControlling_parameters->signatureVersion;
            rvConf.epochSelect = pControlling_parameters->epochSelect;
            rvConf.epochMask = pControlling_parameters->epochMask;
#endif
            rvConf.backgroundCheck = true;
        }

        rvConf.allowRegionDisable = true;
        rvConf.enforceAuth = true;

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

        rc = BSAGElib_P_WaitSBL(hSAGElib);
end:
#if RV_CONTROLLING_PARAMETERS_REVERSED
        if(controlling_parameters) {
            hSAGElib->i_memory_alloc.free(controlling_parameters);
        }
#endif
        if( hRv ) BHSM_RvRegion_Free( hRv );
        if( hRvRsa ) BHSM_RvRsa_Free( hRvRsa );
        if( hKeyLadder ) BHSM_KeyLadder_Free( hKeyLadder );
#endif /* #if (BHSM_API_VERSION==1) */
    } /* end of BHSM_API_VERSION specific code */
out:
    BDBG_LOG((" LEAVING -----------------  [%s]", BSTD_FUNCTION));
    return rc;
}

BERR_Code
BSAGElib_Check_SUIF_Image(
    BSAGElib_Handle hSAGElib,
    SUIF_PackageHeader  *header,
    uint32_t binarySize)
{
    BERR_Code rc = BERR_SUCCESS;

    SUIF_CommonHeader *pCommon = (SUIF_CommonHeader *)header;

    uint32_t *pDataSize_BE = (uint32_t *)(pCommon->dataSize_BE);
    uint32_t *pTextSize_BE = (uint32_t *)(pCommon->textSize_BE);
    uint32_t text_data_len = SUIF_Get32(pTextSize,BE) + SUIF_Get32(pDataSize,BE);

    uint32_t *pMeta_data_size_BE  = (uint32_t *)(pCommon->metadataSize_BE);
    uint32_t meta_data_size  = SUIF_Get32(pMeta_data_size,BE);

    uint16_t *pPkg_pad_size_BE = (uint16_t *)(pCommon->packagePadSize_BE);
    uint32_t pkg_pad_size    = SUIF_Get16(pPkg_pad_size,BE);

    SUIF_PackageFooter *footer = (SUIF_PackageFooter *)
                ((uint8_t *)header + sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size);

    SUIF_TierKeyPackage *pTierKey_First = header->tierKeyPackages;

    uint16_t *pTierKeyPackageSize_BE = (uint16_t *)(pCommon->tierKeyPackageSize_BE);
    uint16_t tierKeyPackageSize = SUIF_Get16(pTierKeyPackageSize,BE);

    BDBG_ENTER(BSAGElib_Check_Sage_Image);

    if(tierKeyPackageSize != sizeof(SUIF_TierKeyPackage))
    {
        BDBG_ERR(("%s - tierKeyPackageSize (%u)wrong should be %u bytes", BSTD_FUNCTION,
            tierKeyPackageSize,(unsigned int)sizeof(SUIF_TierKeyPackage)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(binarySize != sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + sizeof(SUIF_PackageFooter))
    {
        BDBG_ERR(("%s - Image size (%u)wrong should be %u bytes(PKG header %u, Text+Data %u, mata Data %u, Pad %u, Footer %u)",
            BSTD_FUNCTION, binarySize,
            (unsigned int)sizeof(SUIF_PackageHeader) + text_data_len + meta_data_size + pkg_pad_size + (unsigned int)sizeof(SUIF_PackageFooter),
            (unsigned int)sizeof(SUIF_PackageHeader),  text_data_len,  meta_data_size,  pkg_pad_size,  (unsigned int)sizeof(SUIF_PackageFooter)));
        rc = BERR_INVALID_PARAMETER;
        goto out;
    }

    if(header->imageHeader.common.signingScheme == SUIF_SigningScheme_eTriple)
    {
        SUIF_TierKeyPackage *pTierKey;
        int tierkeyIndex=-1;

        pTierKey =  BSAGElib_P_Boot_GetTierKey(hSAGElib,pTierKey_First);

        tierkeyIndex = pTierKey-pTierKey_First;
        BDBG_MSG((" pTierKey index %d (%p,start %p pkg size %u)",
            tierkeyIndex,(void *)pTierKey,(void *)pTierKey_First,(unsigned int)sizeof(SUIF_TierKeyPackage)));

        if(tierkeyIndex == 0 || tierkeyIndex == 1)
        {
            /* test key, zero-fill production key and production signature */
            SUIF_TierKeyPackage *pTierKey_Production = &(header->tierKeyPackages[2]);
            uint8_t *text_signature_production = footer->textVerification.signature1;
            uint8_t *data_signature_production = footer->dataVerification.signature1;

            BDBG_MSG((" Test Tier Key, zero-fill production Tier Key and signatures"));

            BKNI_Memset( pTierKey_Production, 0, sizeof(SUIF_TierKeyPackage) );
            BKNI_Memset( text_signature_production, 0, SUIF_TIER_RSAKEY_SIG_SIZE_BYTES );
            BKNI_Memset( data_signature_production, 0, SUIF_TIER_RSAKEY_SIG_SIZE_BYTES );
        }
    }
out:
    BDBG_LEAVE(BSAGElib_Check_Sage_Image);
    return rc;
}

/* lauch SAGE Bootlader and Framework with SUIF image format
 * both SAGE Bootlader and Framework images should be in SUIF format
 * if launch successfuly, it will return BERR_SUCCESS
 * if SBL or SSF is not SUIF format, it will return BERR_NOT_SUPPORTED
 * (BSAGElib_P_Boot_ParseSageImage() check the image is SUIF or not, and return the coressponding error code )
 * others will return errors
 */
BERR_Code
BSAGElib_P_Boot_SUIF(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Sage_ImageHolder blHolder;
    BSAGElib_Sage_ImageHolder frameworkHolder;

    BDBG_ENTER(BSAGElib_P_Boot_SUIF);

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

    /* Parse SAGE bootloader */
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, SUIF_ImageType_eSBL,pBootSettings->pBootloader,
                                        pBootSettings->bootloaderSize, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }

    /* Check SAGE bootloader compatibility with current chipset */
    rc = BSAGElib_P_Boot_CheckCompatibility(hSAGElib, &blHolder);
    if(rc != BERR_SUCCESS) { goto end; }
    hSAGElib->i_memory_sync.flush(pBootSettings->pBootloader, pBootSettings->bootloaderSize);

    /* Parse SAGE Framework */
    rc = BSAGElib_P_Boot_ParseSageImage(hSAGElib, SUIF_ImageType_eSSF,pBootSettings->pFramework,
                                        pBootSettings->frameworkSize, &frameworkHolder);
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
            hSAGElib->frameworkInfo.version[0]));
        BDBG_ERR(("Please upgrade BOLT."));
        BDBG_ERR(("***************************************************************************"));
        goto end;
    }

    /* push region map to DRAM */
    if(pBootSettings->regionMapNum != 0) {
        hSAGElib->i_memory_sync.flush(pBootSettings-> pRegionMap, pBootSettings->regionMapNum * sizeof(BSAGElib_RegionInfo));
    }

    if(BCHP_SAGE_GetStatus(hSAGElib->core_handles.hReg) == BSAGElibBootStatus_eNotStarted)
    {    /* Take SAGE out of reset */
        _BSAGElib_P_Boot_SetBootParam(HostSageComControl, 0);/* clear Host Sage communication state */
        rc = BSAGElib_P_Boot_ResetSage(hSAGElib, &blHolder);
        if(rc != BERR_SUCCESS) {
            BDBG_ERR(("%s - BSAGElib_P_Boot_ResetSage() failed to launch SAGE Bootloader [0x%x]", BSTD_FUNCTION, rc));
            goto end;
        }
    }

    rc = BSAGElib_Check_SUIF_Image(hSAGElib,frameworkHolder.header,frameworkHolder.binarySize);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_Check_Sage_Image() fails", BSTD_FUNCTION));
        goto end;
    }

    /* Boot frameworks  */
    rc = BSAGElib_P_Boot_Framework(hSAGElib,pBootSettings, &frameworkHolder);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGElib_P_Boot_Framework() fails", BSTD_FUNCTION));
        goto end;
    }
end:

    BDBG_LEAVE(BSAGElib_P_Boot_SUIF);
    return rc;
}
