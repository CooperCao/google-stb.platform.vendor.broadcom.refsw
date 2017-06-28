/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_scheduler_priv.h"
#include "bdbg.h"
#include "bkni.h"
#include <string.h>

BDBG_MODULE(platform);

PlatformHandle platform_open(const char * appName)
{
    PlatformHandle platform;
    int rc = 0;
    NxClient_JoinSettings joinSettings;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    BKNI_Snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", appName);
    rc = NxClient_Join(&joinSettings);
    if (rc) goto out_join_failure;

    platform = BKNI_Malloc(sizeof(*platform));
    BDBG_ASSERT(platform);
    BKNI_Memset(platform, 0, sizeof(*platform));

    platform->scheduler = platform_scheduler_p_create(platform);
    BDBG_ASSERT(platform->scheduler);

    NxClient_GetDefaultCallbackThreadSettings(&platform->callbackThreadSettings);
    platform->callbackThreadSettings.hdmiOutputHotplug.callback = platform_p_hotplug_handler;
    platform->callbackThreadSettings.hdmiOutputHotplug.context = platform;
    platform->callbackThreadSettings.hdmiOutputHotplug.param = 0;
    rc = NxClient_StartCallbackThread(&platform->callbackThreadSettings);
    BDBG_ASSERT(!rc);

    NEXUS_GetVideoDecoderCapabilities(&platform->videoCaps);

    return platform;

out_join_failure:
    return NULL;
}

void platform_close(PlatformHandle platform)
{
    if (!platform) return;
    NxClient_StopCallbackThread();
    platform_scheduler_p_destroy(platform->scheduler);
    BKNI_Free(platform);
    NxClient_Uninit();
}

static const char * capabilityStrings[] =
{
    "           NO",
    "         YES",
    "    UNKNOWN",
    NULL
};

const char * platform_get_capability_name(PlatformCapability cap)
{
    return capabilityStrings[cap];
}

static const char * dynrngStrings[] =
{
    "AUTO",
    "SDR",
    "HLG",
    "HDR10",
    "DBV",
    "TCH",
    "DISABLED",
    "UNKNOWN",
    "UNSUPPORTED",
    NULL
};

const char * platform_get_dynamic_range_name(PlatformDynamicRange dynrng)
{
    return dynrngStrings[dynrng];
}

PlatformDynamicRange platform_get_dynamic_range_from_path(const char * path)
{
    BDBG_ASSERT(path);
    if (strstr(path, "hdr") || strstr(path, "HDR") || strstr(path, "Hdr"))
    {
        return PlatformDynamicRange_eHdr10;
    }
    else if (strstr(path, "hlg") || strstr(path, "HLG") || strstr(path, "Hlg"))
    {
        return PlatformDynamicRange_eHlg;
    }
    else if (strstr(path, "dvs") || strstr(path, "DVS") || strstr(path, "Dvs") || strstr(path, "dovi") || strstr(path, "DoVi") || strstr(path, "dbv"))
    {
        return PlatformDynamicRange_eDolbyVision;
    }
    else
    {
        return PlatformDynamicRange_eSdr;
    }
}

void platform_p_output_dynamic_range_to_nexus(PlatformDynamicRange dynrng, NEXUS_VideoEotf * pEotf, NEXUS_HdmiOutputDolbyVisionMode * pDolbyVision)
{
    *pDolbyVision = NEXUS_HdmiOutputDolbyVisionMode_eDisabled;

    switch (dynrng)
    {
        case PlatformDynamicRange_eSdr:
            *pEotf = NEXUS_VideoEotf_eSdr;
            break;
        case PlatformDynamicRange_eHlg:
            *pEotf = NEXUS_VideoEotf_eHlg;
            break;
        case PlatformDynamicRange_eHdr10:
            *pEotf = NEXUS_VideoEotf_eHdr10;
            break;
        case PlatformDynamicRange_eDolbyVision:
            *pEotf = NEXUS_VideoEotf_eInvalid;
            *pDolbyVision = NEXUS_HdmiOutputDolbyVisionMode_eEnabled;
            break;
        case PlatformDynamicRange_eInvalid:
            *pEotf = NEXUS_VideoEotf_eInvalid;
            break;
        case PlatformDynamicRange_eAuto:
        case PlatformDynamicRange_eUnknown:
        default:
            *pEotf = NEXUS_VideoEotf_eMax;
            *pDolbyVision = NEXUS_HdmiOutputDolbyVisionMode_eAuto;
            break;
    }
}

PlatformDynamicRange platform_p_input_dynamic_range_from_nexus(NEXUS_VideoEotf nxEotf, NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType)
{
    PlatformDynamicRange dynrng;

    switch (dynamicMetadataType)
    {
        case NEXUS_VideoDecoderDynamicRangeMetadataType_eDolbyVision:
            dynrng = PlatformDynamicRange_eDolbyVision;
            break;
        case NEXUS_VideoDecoderDynamicRangeMetadataType_eTechnicolorPrime:
            dynrng = PlatformDynamicRange_eTechnicolorPrime;
            break;
        default:
        case NEXUS_VideoDecoderDynamicRangeMetadataType_eNone:
            switch (nxEotf)
            {
                case NEXUS_VideoEotf_eSdr:
                    dynrng = PlatformDynamicRange_eSdr;
                    break;
                case NEXUS_VideoEotf_eHlg:
                    dynrng = PlatformDynamicRange_eHlg;
                    break;
                case NEXUS_VideoEotf_eHdr10:
                    dynrng = PlatformDynamicRange_eHdr10;
                    break;
                case NEXUS_VideoEotf_eInvalid:
                    dynrng = PlatformDynamicRange_eInvalid;
                    break;
                case NEXUS_VideoEotf_eMax:
                    dynrng = PlatformDynamicRange_eAuto;
                    break;
                default:
                    dynrng = PlatformDynamicRange_eUnknown;
                    break;
            }
            break;
    }

    return dynrng;
}

PlatformDynamicRange platform_p_output_dynamic_range_from_nexus(NEXUS_VideoEotf nxEotf, NEXUS_HdmiOutputDolbyVisionMode dolbyVision)
{
    PlatformDynamicRange dynrng;

    if (dolbyVision == NEXUS_HdmiOutputDolbyVisionMode_eEnabled)
    {
        dynrng = PlatformDynamicRange_eDolbyVision;
    }
    else
    {
        switch (nxEotf)
        {
            case NEXUS_VideoEotf_eSdr:
                dynrng = PlatformDynamicRange_eSdr;
                break;
            case NEXUS_VideoEotf_eHlg:
                dynrng = PlatformDynamicRange_eHlg;
                break;
            case NEXUS_VideoEotf_eHdr10:
                dynrng = PlatformDynamicRange_eHdr10;
                break;
            case NEXUS_VideoEotf_eInvalid:
                dynrng = PlatformDynamicRange_eInvalid;
                break;
            case NEXUS_VideoEotf_eMax:
                dynrng = PlatformDynamicRange_eAuto;
                break;
            default:
                dynrng = PlatformDynamicRange_eUnknown;
                break;
        }
    }

    return dynrng;
}

static const char * colorimetryStrings[] =
{
    "AUTO",
    "SD/BT601",
    "HD/BT709",
    "UHD/BT2020",
    "UNKNOWN",
    "UNSUPPORTED",
    NULL
};

const char * platform_get_colorimetry_name(PlatformColorimetry colorimetry)
{
    return colorimetryStrings[colorimetry];
}

NEXUS_MatrixCoefficients platform_p_colorimetry_to_nexus(PlatformColorimetry colorimetry)
{
    NEXUS_MatrixCoefficients nxColorimetry;

    switch (colorimetry)
    {
        case PlatformColorimetry_e601:
            nxColorimetry = NEXUS_MatrixCoefficients_eXvYCC_601;
            break;
        case PlatformColorimetry_e709:
            nxColorimetry = NEXUS_MatrixCoefficients_eItu_R_BT_709;
            break;
        case PlatformColorimetry_e2020:
            nxColorimetry = NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL;
            break;
        case PlatformColorimetry_eAuto:
        case PlatformColorimetry_eUnknown:
        default:
            nxColorimetry = NEXUS_MatrixCoefficients_eMax;
            break;
    }

    return nxColorimetry;
}

PlatformColorimetry platform_p_colorimetry_from_nexus(NEXUS_MatrixCoefficients nxColorimetry)
{
    PlatformColorimetry colorimetry;

    switch (nxColorimetry)
    {
        case NEXUS_MatrixCoefficients_eUnknown:
        case NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG:
        case NEXUS_MatrixCoefficients_eSmpte_170M:
        case NEXUS_MatrixCoefficients_eXvYCC_601:
            colorimetry = PlatformColorimetry_e601;
            break;
        case NEXUS_MatrixCoefficients_eSmpte_240M:
        case NEXUS_MatrixCoefficients_eXvYCC_709:
        case NEXUS_MatrixCoefficients_eItu_R_BT_709:
            colorimetry = PlatformColorimetry_e709;
            break;
        case NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL:
        case NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL:
            colorimetry = PlatformColorimetry_e2020;
            break;
        case NEXUS_MatrixCoefficients_eMax:
            colorimetry = PlatformColorimetry_eAuto;
            break;
        default:
            colorimetry = PlatformColorimetry_eUnknown;
            break;
    }

    return colorimetry;
}

static const char * colorSpaceStrings[] =
{
    "AUTO",
    "RGB",
    "YCBCR 420",
    "YCBCR 422",
    "YCBCR 444",
    "INVALID",
    "UNKNOWN",
    "UNSUPPORTED",
    NULL
};

const char * platform_get_color_space_name(PlatformColorSpace colorSpace)
{
    return colorSpaceStrings[colorSpace];
}

NEXUS_ColorSpace platform_p_color_space_to_nexus(PlatformColorSpace colorSpace)
{
    NEXUS_ColorSpace nxColorSpace;

    switch (colorSpace)
    {
        case PlatformColorSpace_eAuto:
            nxColorSpace = NEXUS_ColorSpace_eAuto;
            break;
        case PlatformColorSpace_eRgb:
            nxColorSpace = NEXUS_ColorSpace_eRgb;
            break;
        case PlatformColorSpace_eYCbCr420:
            nxColorSpace = NEXUS_ColorSpace_eYCbCr420;
            break;
        case PlatformColorSpace_eYCbCr422:
            nxColorSpace = NEXUS_ColorSpace_eYCbCr422;
            break;
        case PlatformColorSpace_eYCbCr444:
            nxColorSpace = NEXUS_ColorSpace_eYCbCr444;
            break;
        case PlatformColorSpace_eInvalid:
        case PlatformColorSpace_eUnknown:
        default:
            nxColorSpace = NEXUS_ColorSpace_eMax;
            break;
    }

    return nxColorSpace;
}

PlatformColorSpace platform_p_color_space_from_nexus(NEXUS_ColorSpace nxColorSpace)
{
    PlatformColorSpace colorSpace;

    switch (nxColorSpace)
    {
        case NEXUS_ColorSpace_eAuto:
            colorSpace = PlatformColorSpace_eAuto;
            break;
        case NEXUS_ColorSpace_eRgb:
            colorSpace = PlatformColorSpace_eRgb;
            break;
        case NEXUS_ColorSpace_eYCbCr420:
            colorSpace = PlatformColorSpace_eYCbCr420;
            break;
        case NEXUS_ColorSpace_eYCbCr422:
            colorSpace = PlatformColorSpace_eYCbCr422;
            break;
        case NEXUS_ColorSpace_eYCbCr444:
            colorSpace = PlatformColorSpace_eYCbCr444;
            break;
        case NEXUS_ColorSpace_eMax:
            colorSpace = PlatformColorSpace_eInvalid;
            break;
        default:
            colorSpace = PlatformColorSpace_eUnknown;
            break;
    }

    return colorSpace;
}

static const struct {
    unsigned frequency;
    NEXUS_VideoFrameRate nexusFramerate;
} b_verticalfrequency[NEXUS_VideoFrameRate_eMax] = {
/* array should be sorted by the frequency to facilitate implementations of NEXUS_P_RefreshRate_FromFrameRate_isrsafe and NEXUS_P_FrameRate_FromRefreshRate_isrsafe */
    { 7493, NEXUS_VideoFrameRate_e7_493},
    { 7500, NEXUS_VideoFrameRate_e7_5},
    { 9990, NEXUS_VideoFrameRate_e9_99},
    {10000, NEXUS_VideoFrameRate_e10},
    {11988, NEXUS_VideoFrameRate_e11_988},
    {12000, NEXUS_VideoFrameRate_e12},
    {12500, NEXUS_VideoFrameRate_e12_5},
    {14985, NEXUS_VideoFrameRate_e14_985},
    {15000, NEXUS_VideoFrameRate_e15},
    {19980, NEXUS_VideoFrameRate_e19_98},
    {20000, NEXUS_VideoFrameRate_e20},
    {23976, NEXUS_VideoFrameRate_e23_976},
    {24000, NEXUS_VideoFrameRate_e24},
    {25000, NEXUS_VideoFrameRate_e25},
    {29970, NEXUS_VideoFrameRate_e29_97},
    {30000, NEXUS_VideoFrameRate_e30},
    {50000, NEXUS_VideoFrameRate_e50},
    {59940, NEXUS_VideoFrameRate_e59_94},
    {60000, NEXUS_VideoFrameRate_e60},
    {100000, NEXUS_VideoFrameRate_e100},
    {119880, NEXUS_VideoFrameRate_e119_88},
    {120000, NEXUS_VideoFrameRate_e120}
};

unsigned platform_p_frame_rate_from_nexus(NEXUS_VideoFrameRate frameRate)
{
    unsigned i;
    for(i=0;i<sizeof(b_verticalfrequency)/sizeof(*b_verticalfrequency);i++) {
        if (frameRate == b_verticalfrequency[i].nexusFramerate) {
            return b_verticalfrequency[i].frequency;
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return 0; /* NEXUS_VideoFrameRate_eUnknown */
}

void platform_p_hotplug_handler(void * context, int param)
{
    PlatformHandle platform = context;
    BSTD_UNUSED(param);
    BDBG_ASSERT(platform);
    if (platform->rx)
    {
        platform_receiver_p_hotplug_handler(platform->rx);
    }
}

void platform_get_default_model(PlatformModel * pModel)
{
    unsigned i;
    BKNI_Memset(pModel, 0, sizeof(*pModel));
    for(i=0; i<MAX_MOSAICS; i++) {
        platform_get_default_picture_info(&pModel->vid[i].info);
        pModel->vid[i].plm = PlatformTriState_eMax;
    }
    platform_get_default_picture_info(&pModel->gfx.info);
    platform_get_default_picture_info(&pModel->out.info);
    pModel->rcv.dynrng = PlatformCapability_eUnknown;
    pModel->gfx.plm = PlatformTriState_eMax;
}

void platform_get_default_picture_info(PlatformPictureInfo * pInfo)
{
    BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    pInfo->dynrng = PlatformDynamicRange_eUnknown;
    pInfo->gamut = PlatformColorimetry_eUnknown;
    pInfo->space = PlatformColorSpace_eUnknown;
}

PlatformSchedulerHandle platform_get_scheduler(PlatformHandle platform)
{
    BDBG_ASSERT(platform);
    return platform->scheduler;
}
