/***************************************************************************
*     (c)2008-2012 Broadcom Corporation
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
/* this file should not #include nexus_core_modules.h so that it can be used locally.
this means it cannot have public API's. */
#include "nexus_base.h"
#include "nexus_types.h"
#include "priv/nexus_core.h"
#include "bfmt.h"

BDBG_MODULE(nexus_core_convert);

static const struct {
    NEXUS_TransportType transportType;
    BAVC_StreamType streamType;
} g_NEXUS_transportTypes[] = {
    {NEXUS_TransportType_eEs, BAVC_StreamType_eEs},
    {NEXUS_TransportType_eTs, BAVC_StreamType_eTsMpeg},
    {NEXUS_TransportType_eMpeg2Pes, BAVC_StreamType_ePes},
    {NEXUS_TransportType_eVob, BAVC_StreamType_ePes},
    {NEXUS_TransportType_eMpeg1Ps, BAVC_StreamType_eMpeg1System}, /* ? */
    {NEXUS_TransportType_eDssEs, BAVC_StreamType_eDssEs},
    {NEXUS_TransportType_eDssPes, BAVC_StreamType_eDssPes},
    /* {NEXUS_TransportType_eAsf, xxx}, - ASF is not supported in Magnum */
    /* {NEXUS_TransportType_eFlv, xxx}, - Flv is not supported in Magnum */
    {NEXUS_TransportType_eAvi, BAVC_StreamType_eAVI},
    {NEXUS_TransportType_eMp4, BAVC_StreamType_eMPEG4},
    {NEXUS_TransportType_eMkv, BAVC_StreamType_eMKV}
};

NEXUS_Error
NEXUS_P_TransportType_ToMagnum_isrsafe(NEXUS_TransportType transportType, BAVC_StreamType *streamType)
{
    unsigned i;
    for (i=0;i<sizeof(g_NEXUS_transportTypes)/sizeof(g_NEXUS_transportTypes[0]);i++) {
        if (g_NEXUS_transportTypes[i].transportType == transportType) {
            *streamType = g_NEXUS_transportTypes[i].streamType;
            return NEXUS_SUCCESS;
        }
    }
    BDBG_WRN(("unknown transport type nexus:%u ",transportType));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

static const struct {
    BFMT_VideoFmt mformat;
    NEXUS_VideoFormat nformat;
} b_formats[] = {
    {BFMT_VideoFmt_eNTSC                      , NEXUS_VideoFormat_eNtsc},
    {BFMT_VideoFmt_eNTSC_J                    , NEXUS_VideoFormat_eNtscJapan},
    {BFMT_VideoFmt_eNTSC_443                  , NEXUS_VideoFormat_eNtsc443},
    {BFMT_VideoFmt_ePAL_60                    , NEXUS_VideoFormat_ePal60hz},
    {BFMT_VideoFmt_ePAL_G                     , NEXUS_VideoFormat_ePal},
    {BFMT_VideoFmt_ePAL_N                     , NEXUS_VideoFormat_ePalN},
    {BFMT_VideoFmt_ePAL_M                     , NEXUS_VideoFormat_ePalM},
    {BFMT_VideoFmt_ePAL_NC                    , NEXUS_VideoFormat_ePalNc},
    {BFMT_VideoFmt_ePAL_B                     , NEXUS_VideoFormat_ePalB},
    {BFMT_VideoFmt_ePAL_B1                    , NEXUS_VideoFormat_ePalB1},
    {BFMT_VideoFmt_ePAL_D                     , NEXUS_VideoFormat_ePalD},
    {BFMT_VideoFmt_ePAL_D1                    , NEXUS_VideoFormat_ePalD1},
    {BFMT_VideoFmt_ePAL_D                     , NEXUS_VideoFormat_ePalDK2},
    {BFMT_VideoFmt_ePAL_D                     , NEXUS_VideoFormat_ePalDK3},
    {BFMT_VideoFmt_ePAL_G                     , NEXUS_VideoFormat_ePalG},
    {BFMT_VideoFmt_ePAL_H                     , NEXUS_VideoFormat_ePalH},
    {BFMT_VideoFmt_ePAL_K                     , NEXUS_VideoFormat_ePalK},
    {BFMT_VideoFmt_ePAL_I                     , NEXUS_VideoFormat_ePalI},
    {BFMT_VideoFmt_eSECAM_L                   , NEXUS_VideoFormat_eSecamL},
    {BFMT_VideoFmt_eSECAM_B                   , NEXUS_VideoFormat_eSecamB},
    {BFMT_VideoFmt_eSECAM_G                   , NEXUS_VideoFormat_eSecamG},
    {BFMT_VideoFmt_eSECAM_D                   , NEXUS_VideoFormat_eSecamD},
    {BFMT_VideoFmt_eSECAM_K                   , NEXUS_VideoFormat_eSecamK},
    {BFMT_VideoFmt_eSECAM_H                   , NEXUS_VideoFormat_eSecamH},
    {BFMT_VideoFmt_e1080i                     , NEXUS_VideoFormat_e1080i},
    {BFMT_VideoFmt_e1080p_24Hz                , NEXUS_VideoFormat_e1080p24hz},
    {BFMT_VideoFmt_e1080p_25Hz                , NEXUS_VideoFormat_e1080p25hz},
    {BFMT_VideoFmt_e1080p_30Hz                , NEXUS_VideoFormat_e1080p30hz},
    {BFMT_VideoFmt_e1080i_50Hz                , NEXUS_VideoFormat_e1080i50hz},
    {BFMT_VideoFmt_e1080p                     , NEXUS_VideoFormat_e1080p60hz},
    {BFMT_VideoFmt_e1080p_100Hz               , NEXUS_VideoFormat_e1080p100hz},
    {BFMT_VideoFmt_e1080p_120Hz               , NEXUS_VideoFormat_e1080p120hz},
    {BFMT_VideoFmt_e720p                      , NEXUS_VideoFormat_e720p},
    {BFMT_VideoFmt_e720p_24Hz                 , NEXUS_VideoFormat_e720p24hz},
    {BFMT_VideoFmt_e720p_25Hz                 , NEXUS_VideoFormat_e720p25hz},
    {BFMT_VideoFmt_e720p_30Hz                 , NEXUS_VideoFormat_e720p30hz},
    {BFMT_VideoFmt_e720p_50Hz                 , NEXUS_VideoFormat_e720p50hz},
    {BFMT_VideoFmt_e1250i_50Hz                , NEXUS_VideoFormat_e1250i50hz},
    {BFMT_VideoFmt_e480p                      , NEXUS_VideoFormat_e480p},
    {BFMT_VideoFmt_e576p_50Hz                 , NEXUS_VideoFormat_e576p},
    {BFMT_VideoFmt_e3840x2160p_24Hz           , NEXUS_VideoFormat_e3840x2160p24hz},
    {BFMT_VideoFmt_e3840x2160p_25Hz           , NEXUS_VideoFormat_e3840x2160p25hz},
    {BFMT_VideoFmt_e3840x2160p_30Hz           , NEXUS_VideoFormat_e3840x2160p30hz},
    {BFMT_VideoFmt_e3840x2160p_50Hz           , NEXUS_VideoFormat_e3840x2160p50hz},
    {BFMT_VideoFmt_e3840x2160p_60Hz           , NEXUS_VideoFormat_e3840x2160p60hz},
    {BFMT_VideoFmt_e4096x2160p_24Hz           , NEXUS_VideoFormat_e4096x2160p24hz},
    {BFMT_VideoFmt_e4096x2160p_25Hz           , NEXUS_VideoFormat_e4096x2160p25hz},
    {BFMT_VideoFmt_e4096x2160p_30Hz           , NEXUS_VideoFormat_e4096x2160p30hz},
    {BFMT_VideoFmt_e4096x2160p_50Hz           , NEXUS_VideoFormat_e4096x2160p50hz},
    {BFMT_VideoFmt_e4096x2160p_60Hz           , NEXUS_VideoFormat_e4096x2160p60hz},
    {BFMT_VideoFmt_eCUSTOM_1440x240p_60Hz     , NEXUS_VideoFormat_eCustomer1440x240p60hz},
    {BFMT_VideoFmt_eCUSTOM_1440x288p_50Hz     , NEXUS_VideoFormat_eCustomer1440x288p50hz},
    {BFMT_VideoFmt_eCUSTOM_1366x768p          , NEXUS_VideoFormat_eCustomer1366x768p60hz},
    {BFMT_VideoFmt_eCUSTOM_1366x768p_50Hz     , NEXUS_VideoFormat_eCustomer1366x768p50hz},
    {BFMT_VideoFmt_eDVI_640x480p              , NEXUS_VideoFormat_eVesa640x480p60hz},
    {BFMT_VideoFmt_eDVI_800x600p              , NEXUS_VideoFormat_eVesa800x600p60hz},
    {BFMT_VideoFmt_eDVI_1024x768p             , NEXUS_VideoFormat_eVesa1024x768p60hz},
    {BFMT_VideoFmt_eDVI_1280x768p             , NEXUS_VideoFormat_eVesa1280x768p60hz},
    {BFMT_VideoFmt_e1080p_50Hz                , NEXUS_VideoFormat_e1080p50hz},
    {BFMT_VideoFmt_e240p_60Hz                 , NEXUS_VideoFormat_e240p60hz},
    {BFMT_VideoFmt_e288p_50Hz                 , NEXUS_VideoFormat_e288p50hz},
    {BFMT_VideoFmt_e1440x480p_60Hz            , NEXUS_VideoFormat_e1440x480p60hz},
    {BFMT_VideoFmt_e1440x576p_50Hz            , NEXUS_VideoFormat_e1440x576p50hz},
    {BFMT_VideoFmt_eDVI_640x350p_60Hz         , NEXUS_VideoFormat_eVesa640x350p60hz},
    {BFMT_VideoFmt_eDVI_640x350p_70Hz         , NEXUS_VideoFormat_eVesa640x350p70hz},
    {BFMT_VideoFmt_eDVI_640x350p_72Hz         , NEXUS_VideoFormat_eVesa640x350p72hz},
    {BFMT_VideoFmt_eDVI_640x350p_75Hz         , NEXUS_VideoFormat_eVesa640x350p75hz},
    {BFMT_VideoFmt_eDVI_640x350p_85Hz         , NEXUS_VideoFormat_eVesa640x350p85hz},
    {BFMT_VideoFmt_eDVI_640x400p_60Hz         , NEXUS_VideoFormat_eVesa640x400p60hz},
    {BFMT_VideoFmt_eDVI_640x400p_70Hz         , NEXUS_VideoFormat_eVesa640x400p70hz},
    {BFMT_VideoFmt_eDVI_640x400p_72Hz         , NEXUS_VideoFormat_eVesa640x400p72hz},
    {BFMT_VideoFmt_eDVI_640x400p_75Hz         , NEXUS_VideoFormat_eVesa640x400p75hz},
    {BFMT_VideoFmt_eDVI_640x400p_85Hz         , NEXUS_VideoFormat_eVesa640x400p85hz},
    {BFMT_VideoFmt_eDVI_640x480p_66Hz         , NEXUS_VideoFormat_eVesa640x480p66hz},
    {BFMT_VideoFmt_eDVI_640x480p_70Hz         , NEXUS_VideoFormat_eVesa640x480p70hz},
    {BFMT_VideoFmt_eDVI_640x480p_72Hz         , NEXUS_VideoFormat_eVesa640x480p72hz},
    {BFMT_VideoFmt_eDVI_640x480p_75Hz         , NEXUS_VideoFormat_eVesa640x480p75hz},
    {BFMT_VideoFmt_eDVI_640x480p_85Hz         , NEXUS_VideoFormat_eVesa640x480p85hz},
    {BFMT_VideoFmt_eDVI_720x400p_60Hz         , NEXUS_VideoFormat_eVesa720x400p60hz},
    {BFMT_VideoFmt_eDVI_720x400p_70Hz         , NEXUS_VideoFormat_eVesa720x400p70hz},
    {BFMT_VideoFmt_eDVI_720x400p_72Hz         , NEXUS_VideoFormat_eVesa720x400p72hz},
    {BFMT_VideoFmt_eDVI_720x400p_75Hz         , NEXUS_VideoFormat_eVesa720x400p75hz},
    {BFMT_VideoFmt_eDVI_720x400p_85Hz         , NEXUS_VideoFormat_eVesa720x400p85hz},
    {BFMT_VideoFmt_eDVI_800x600p_56Hz         , NEXUS_VideoFormat_eVesa800x600p56hz},
    {BFMT_VideoFmt_eDVI_800x600p_59Hz_Red     , NEXUS_VideoFormat_eVesa800x600p59hzRed},
    {BFMT_VideoFmt_eDVI_800x600p_70Hz         , NEXUS_VideoFormat_eVesa800x600p70hz},
    {BFMT_VideoFmt_eDVI_800x600p_72Hz         , NEXUS_VideoFormat_eVesa800x600p72hz},
    {BFMT_VideoFmt_eDVI_800x600p_75Hz         , NEXUS_VideoFormat_eVesa800x600p75hz},
    {BFMT_VideoFmt_eDVI_800x600p_85Hz         , NEXUS_VideoFormat_eVesa800x600p85hz},
    {BFMT_VideoFmt_eDVI_848x480p_60Hz         , NEXUS_VideoFormat_eVesa848x480p60hz},
    {BFMT_VideoFmt_eDVI_1024x768p_66Hz        , NEXUS_VideoFormat_eVesa1024x768p66hz},
    {BFMT_VideoFmt_eDVI_1024x768p_70Hz        , NEXUS_VideoFormat_eVesa1024x768p70hz},
    {BFMT_VideoFmt_eDVI_1024x768p_72Hz        , NEXUS_VideoFormat_eVesa1024x768p72hz},
    {BFMT_VideoFmt_eDVI_1024x768p_75Hz        , NEXUS_VideoFormat_eVesa1024x768p75hz},
    {BFMT_VideoFmt_eDVI_1024x768p_85Hz        , NEXUS_VideoFormat_eVesa1024x768p85hz},
    {BFMT_VideoFmt_eDVI_1280x720p_50Hz        , NEXUS_VideoFormat_eVesa1280x720p50hz},
    {BFMT_VideoFmt_eDVI_1280x720p             , NEXUS_VideoFormat_eVesa1280x720p60hz},
    {BFMT_VideoFmt_eDVI_1280x720p_ReducedBlank, NEXUS_VideoFormat_eVesa1280x720pReducedBlank},
    {BFMT_VideoFmt_eDVI_1280x720p_70Hz        , NEXUS_VideoFormat_eVesa1280x720p70hz},
    {BFMT_VideoFmt_eDVI_1280x720p_72Hz        , NEXUS_VideoFormat_eVesa1280x720p72hz},
    {BFMT_VideoFmt_eDVI_1280x720p_75Hz        , NEXUS_VideoFormat_eVesa1280x720p75hz},
    {BFMT_VideoFmt_eDVI_1280x720p_85Hz        , NEXUS_VideoFormat_eVesa1280x720p85hz},
    {BFMT_VideoFmt_eDVI_1280x768p_Red         , NEXUS_VideoFormat_eVesa1280x768p60hzRed},
    {BFMT_VideoFmt_eDVI_1064x600p_60Hz        , NEXUS_VideoFormat_eVesa1064x600p60hz},
    {BFMT_VideoFmt_eDVI_1024x768i_87Hz        , NEXUS_VideoFormat_eVesa1024x768i87hz},
    {BFMT_VideoFmt_eDVI_1152x864p_75Hz        , NEXUS_VideoFormat_eVesa1152x864p75hz},
    {BFMT_VideoFmt_eDVI_1280x768p_75Hz        , NEXUS_VideoFormat_eVesa1280x768p75hz},
    {BFMT_VideoFmt_eDVI_1280x768p_85Hz        , NEXUS_VideoFormat_eVesa1280x768p85hz},
    {BFMT_VideoFmt_eDVI_1280x960p_60Hz        , NEXUS_VideoFormat_eVesa1280x960p60hz},
    {BFMT_VideoFmt_eDVI_1280x960p_85Hz        , NEXUS_VideoFormat_eVesa1280x960p85hz},
    {BFMT_VideoFmt_eDVI_1280x1024p_60Hz       , NEXUS_VideoFormat_eVesa1280x1024p60hz},
    {BFMT_VideoFmt_eDVI_1280x1024p_69Hz       , NEXUS_VideoFormat_eVesa1280x1024p69hz},
    {BFMT_VideoFmt_eDVI_1280x1024p_75Hz       , NEXUS_VideoFormat_eVesa1280x1024p75hz},
    {BFMT_VideoFmt_eDVI_1280x1024p_85Hz       , NEXUS_VideoFormat_eVesa1280x1024p85hz},
    {BFMT_VideoFmt_eDVI_832x624p_75Hz         , NEXUS_VideoFormat_eVesa832x624p75hz},
    {BFMT_VideoFmt_eDVI_1360x768p_60Hz        , NEXUS_VideoFormat_eVesa1360x768p60hz},
    {BFMT_VideoFmt_eDVI_1366x768p_60Hz        , NEXUS_VideoFormat_eVesa1366x768p60hz},
    {BFMT_VideoFmt_eDVI_1400x1050p_60Hz       , NEXUS_VideoFormat_eVesa1400x1050p60hz},
    {BFMT_VideoFmt_eDVI_1400x1050p_60Hz_Red   , NEXUS_VideoFormat_eVesa1400x1050p60hzReducedBlank},
    {BFMT_VideoFmt_eDVI_1400x1050p_75Hz       , NEXUS_VideoFormat_eVesa1400x1050p75hz},
    {BFMT_VideoFmt_eDVI_1440x900p_60Hz        , NEXUS_VideoFormat_eVesa1440x900p60hz},
    {BFMT_VideoFmt_eDVI_1600x1200p_60Hz       , NEXUS_VideoFormat_eVesa1600x1200p60hz},
    {BFMT_VideoFmt_eDVI_1920x1080p_60Hz_Red   , NEXUS_VideoFormat_eVesa1920x1080p60hzReducedBlank},
    {BFMT_VideoFmt_e720p_60Hz_3DOU_AS         , NEXUS_VideoFormat_e720p_3DOU_AS},
    {BFMT_VideoFmt_e720p_50Hz_3DOU_AS         , NEXUS_VideoFormat_e720p50hz_3DOU_AS},
    {BFMT_VideoFmt_e1080p_24Hz_3DOU_AS        , NEXUS_VideoFormat_e1080p24hz_3DOU_AS},
    {BFMT_VideoFmt_e1080p_30Hz_3DOU_AS        , NEXUS_VideoFormat_e1080p30hz_3DOU_AS},
    {BFMT_VideoFmt_e720x482_NTSC              , NEXUS_VideoFormat_e720x482_NTSC},
    {BFMT_VideoFmt_e720x482_NTSC_J            , NEXUS_VideoFormat_e720x482_NTSC_J},
    {BFMT_VideoFmt_e720x483p                  , NEXUS_VideoFormat_e720x483p},
    {BFMT_VideoFmt_eCustom2                   , NEXUS_VideoFormat_eCustom2}
};

NEXUS_Error
NEXUS_P_VideoFormat_ToMagnum_isrsafe(NEXUS_VideoFormat format, BFMT_VideoFmt *mformat)
{
    unsigned i;
    for(i=0;i<sizeof(b_formats)/sizeof(*b_formats);i++) {
        if (format==b_formats[i].nformat) {
            *mformat = b_formats[i].mformat;
            return BERR_SUCCESS;
        }
    }
    /* fail silently. this is a utility function and querying an unsupported format is not necessarily an ERR. */
    return BERR_INVALID_PARAMETER;
}

NEXUS_VideoFormat
NEXUS_P_VideoFormat_FromMagnum_isrsafe(BFMT_VideoFmt format)
{
    unsigned i;
    for(i=0;i<sizeof(b_formats)/sizeof(*b_formats);i++) {
        if (format==b_formats[i].mformat) {
            return b_formats[i].nformat;
        }
    }
    return NEXUS_VideoFormat_eUnknown;
}


NEXUS_AspectRatio
NEXUS_P_AspectRatio_FromMagnum_isrsafe(BFMT_AspectRatio eAspectRatio)
{
    BDBG_CASSERT(BFMT_AspectRatio_eSAR == (BFMT_AspectRatio)NEXUS_AspectRatio_eSar);
    if (eAspectRatio <= BFMT_AspectRatio_eSAR)
        return (NEXUS_AspectRatio)eAspectRatio;
    else
        return NEXUS_AspectRatio_eUnknown;
}

BFMT_AspectRatio
NEXUS_P_AspectRatio_ToMagnum_isrsafe(NEXUS_AspectRatio aspectRatio)
{
    if (aspectRatio <= NEXUS_AspectRatio_eSar)
        return (BFMT_AspectRatio)aspectRatio;
    else
        return BFMT_AspectRatio_eUnknown;
}


NEXUS_VideoFrameRate
NEXUS_P_FrameRate_FromMagnum_isrsafe(BAVC_FrameRateCode magnumFramerate)
{
    BDBG_CASSERT(NEXUS_VideoFrameRate_eUnknown == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_eUnknown);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e23_976 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e23_976);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e24 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e24);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e25 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e25);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e29_97 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e29_97);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e30 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e30);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e50 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e50);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e59_94 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e59_94);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e60 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e60);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e14_985 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e14_985);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e7_493 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e7_493);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e10 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e10);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e15 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e15);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e20 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e20);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e12_5 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e12_5);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e100 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e100);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e119_88 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e119_88);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e120 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e120);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e19_98 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e19_98);
    BDBG_CWARNING(NEXUS_VideoFrameRate_eMax == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_eMax);

    return (NEXUS_VideoFrameRate)magnumFramerate;
}

NEXUS_Error NEXUS_P_FrameRate_ToMagnum_isrsafe(NEXUS_VideoFrameRate frameRate, BAVC_FrameRateCode *pMagnumFramerate)
{
    if((unsigned)frameRate < NEXUS_VideoFrameRate_eMax) {
        *pMagnumFramerate = (BAVC_FrameRateCode)frameRate;
        return NEXUS_SUCCESS;
    } else {
        *pMagnumFramerate = BAVC_FrameRateCode_eUnknown;
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

static const struct {
    unsigned frequency;
    NEXUS_VideoFrameRate nexusFramerate;
} b_verticalfrequency[NEXUS_VideoFrameRate_eMax] = {
/* array should be sorted by the frequency to facilitate implementations of NEXUS_P_RefreshRate_FromFrameRate_isrsafe and NEXUS_P_FrameRate_FromRefreshRate_isrsafe */
    { 7493, NEXUS_VideoFrameRate_e7_493},
    {10000, NEXUS_VideoFrameRate_e10},
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

unsigned NEXUS_P_RefreshRate_FromFrameRate_isrsafe(NEXUS_VideoFrameRate frameRate)
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

void
NEXUS_P_FrameRate_FromRefreshRate_isrsafe(unsigned frameRateInteger, NEXUS_VideoFrameRate *pNexusFrameRate)
{
    unsigned i;
    /* 1. use precise match */
    for(i=0;i<sizeof(b_verticalfrequency)/sizeof(*b_verticalfrequency);i++) {
        if (frameRateInteger == b_verticalfrequency[i].frequency) {
            *pNexusFrameRate = b_verticalfrequency[i].nexusFramerate;
            return;
        }
    }
    /* 2. use 1% fudge factor */
    for(i=0;i<sizeof(b_verticalfrequency)/sizeof(*b_verticalfrequency);i++) {
        if ((frameRateInteger*258)/256>= b_verticalfrequency[i].frequency && (frameRateInteger*254)/256<=b_verticalfrequency[i].frequency) {
            *pNexusFrameRate = b_verticalfrequency[i].nexusFramerate;
            return;
        }
    }
    /* 3. use 10% fudge factor */
    for(i=0;i<sizeof(b_verticalfrequency)/sizeof(*b_verticalfrequency);i++) {
        if ((frameRateInteger*276)/256>= b_verticalfrequency[i].frequency && (frameRateInteger*236)/256<=b_verticalfrequency[i].frequency) {
            *pNexusFrameRate = b_verticalfrequency[i].nexusFramerate;
            return;
        }
    }
    *pNexusFrameRate = NEXUS_VideoFrameRate_eUnknown;
    return;
}

static const NEXUS_PixelFormatConvertInfo g_pixelFormatInfo[NEXUS_PixelFormat_eMax] = {
    {{false, false, false, 0}, BPXL_INVALID, NEXUS_PixelFormat_eUnknown},
    {{true, false, false, 16}, BPXL_eR5_G6_B5, NEXUS_PixelFormat_eR5_G6_B5},
    {{true, false, false, 16}, BPXL_eB5_G6_R5, NEXUS_PixelFormat_eB5_G6_R5},

    {{true, false, true,  16}, BPXL_eA1_R5_G5_B5, NEXUS_PixelFormat_eA1_R5_G5_B5},
    {{true, false, false, 16}, BPXL_eX1_R5_G5_B5, NEXUS_PixelFormat_eX1_R5_G5_B5},
    {{true, false, true,  16}, BPXL_eA1_B5_G5_R5, NEXUS_PixelFormat_eA1_B5_G5_R5},
    {{true, false, false, 16}, BPXL_eX1_B5_G5_R5, NEXUS_PixelFormat_eX1_B5_G5_R5},
    {{true, false, true,  16}, BPXL_eR5_G5_B5_A1, NEXUS_PixelFormat_eR5_G5_B5_A1},
    {{true, false, false, 16}, BPXL_eR5_G5_B5_X1, NEXUS_PixelFormat_eR5_G5_B5_X1},
    {{true, false, true,  16}, BPXL_eB5_G5_R5_A1, NEXUS_PixelFormat_eB5_G5_R5_A1},
    {{true, false, false, 16}, BPXL_eB5_G5_R5_X1, NEXUS_PixelFormat_eB5_G5_R5_X1},

    {{true, false, true,  16}, BPXL_eA4_R4_G4_B4, NEXUS_PixelFormat_eA4_R4_G4_B4},
    {{true, false, false, 16}, BPXL_eX4_R4_G4_B4, NEXUS_PixelFormat_eX4_R4_G4_B4},
    {{true, false, true,  16}, BPXL_eA4_B4_G4_R4, NEXUS_PixelFormat_eA4_B4_G4_R4},
    {{true, false, false, 16}, BPXL_eX4_B4_G4_R4, NEXUS_PixelFormat_eX4_B4_G4_R4},
    {{true, false, true,  16}, BPXL_eR4_G4_B4_A4, NEXUS_PixelFormat_eR4_G4_B4_A4},
    {{true, false, false, 16}, BPXL_eR4_G4_B4_X4, NEXUS_PixelFormat_eR4_G4_B4_X4},
    {{true, false, true,  16}, BPXL_eB4_G4_R4_A4, NEXUS_PixelFormat_eB4_G4_R4_A4},
    {{true, false, false, 16}, BPXL_eB4_G4_R4_X4, NEXUS_PixelFormat_eB4_G4_R4_X4},

    {{true, false, true,  32}, BPXL_eA8_R8_G8_B8, NEXUS_PixelFormat_eA8_R8_G8_B8},
    {{true, false, false, 32}, BPXL_eX8_R8_G8_B8, NEXUS_PixelFormat_eX8_R8_G8_B8},
    {{true, false, true,  32}, BPXL_eA8_B8_G8_R8, NEXUS_PixelFormat_eA8_B8_G8_R8},
    {{true, false, false, 32}, BPXL_eX8_B8_G8_R8, NEXUS_PixelFormat_eX8_B8_G8_R8},
    {{true, false, true,  32}, BPXL_eR8_G8_B8_A8, NEXUS_PixelFormat_eR8_G8_B8_A8},
    {{true, false, false, 32}, BPXL_eR8_G8_B8_X8, NEXUS_PixelFormat_eR8_G8_B8_X8},
    {{true, false, true,  32}, BPXL_eB8_G8_R8_A8, NEXUS_PixelFormat_eB8_G8_R8_A8},
    {{true, false, false, 32}, BPXL_eB8_G8_R8_X8, NEXUS_PixelFormat_eB8_G8_R8_X8},

    {{true, false, true, 8}, BPXL_eA8, NEXUS_PixelFormat_eA8},
    {{true, false, true, 4}, BPXL_eA4, NEXUS_PixelFormat_eA4},
    {{true, false, true, 2}, BPXL_eA2, NEXUS_PixelFormat_eA2},
    {{true, false, true, 1}, BPXL_eA1, NEXUS_PixelFormat_eA1},

    {{true, false, true, 1}, BPXL_eW1, NEXUS_PixelFormat_eW1},

    {{true, true, true, 16}, BPXL_eA8_P8, NEXUS_PixelFormat_eA8_Palette8},
    {{true, true, true, 8},  BPXL_eP8, NEXUS_PixelFormat_ePalette8},
    {{true, true, true, 4},  BPXL_eP4, NEXUS_PixelFormat_ePalette4},
    {{true, true, true, 2},  BPXL_eP2, NEXUS_PixelFormat_ePalette2},
    {{true, true, true, 1},  BPXL_eP1, NEXUS_PixelFormat_ePalette1},

    {{true,  true, true,   16}, BPXL_eY8_P8, NEXUS_PixelFormat_eY8_Palette8},
    {{false, false, true,  8},  BPXL_eA8_Y8, NEXUS_PixelFormat_eA8_Y8},

    {{false, false, false, 16}, BPXL_eCb8, NEXUS_PixelFormat_eCb8},
    {{false, false, false, 16}, BPXL_eCr8, NEXUS_PixelFormat_eCr8},

    {{false, false, false, 8},  BPXL_eY8,      NEXUS_PixelFormat_eY8},
    {{false, false, false, 16}, BPXL_eCb8_Cr8, NEXUS_PixelFormat_eCb8_Cr8},
    {{false, false, false, 16}, BPXL_eCr8_Cb8, NEXUS_PixelFormat_eCr8_Cb8},

    {{false, false, false, 10}, BPXL_eY10,       NEXUS_PixelFormat_eY10},
    {{false, false, false, 20}, BPXL_eCb10_Cr10, NEXUS_PixelFormat_eCb10_Cr10},
    {{false, false, false, 20}, BPXL_eCr10_Cb10, NEXUS_PixelFormat_eCr10_Cb10},

    {{false, false, false, 16}, BPXL_eY08_Cb8_Y18_Cr8, NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8},
    {{false, false, false, 16}, BPXL_eY08_Cr8_Y18_Cb8, NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8},
    {{false, false, false, 16}, BPXL_eY18_Cb8_Y08_Cr8, NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8},
    {{false, false, false, 16}, BPXL_eY18_Cr8_Y08_Cb8, NEXUS_PixelFormat_eY18_Cr8_Y08_Cb8},
    {{false, false, false, 16}, BPXL_eCb8_Y08_Cr8_Y18, NEXUS_PixelFormat_eCb8_Y08_Cr8_Y18},
    {{false, false, false, 16}, BPXL_eCb8_Y18_Cr8_Y08, NEXUS_PixelFormat_eCb8_Y18_Cr8_Y08},
    {{false, false, false, 16}, BPXL_eCr8_Y18_Cb8_Y08, NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08},
    {{false, false, false, 16}, BPXL_eCr8_Y08_Cb8_Y18, NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18},

    {{false, false, false, 32}, BPXL_eX2_Cr10_Y10_Cb10, NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10},

    {{false, false, true,  32}, BPXL_eA8_Y8_Cb8_Cr8, NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8},
    {{false, false, true,  32}, BPXL_eA8_Cr8_Cb8_Y8, NEXUS_PixelFormat_eA8_Cr8_Cb8_Y8},
    {{false, false, true,  32}, BPXL_eCr8_Cb8_Y8_A8, NEXUS_PixelFormat_eCr8_Cb8_Y8_A8},
    {{false, false, true,  32}, BPXL_eY8_Cb8_Cr8_A8, NEXUS_PixelFormat_eY8_Cb8_Cr8_A8},

    /* bpp for packed 10 bits 4:2:2 format is 20 + some less significant number due to alignment */
    {{false, false, false, 32}, BPXL_eY010_Cb10_Y110_Cr10, NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10},
    {{false, false, false, 32}, BPXL_eY010_Cr10_Y110_Cb10, NEXUS_PixelFormat_eY010_Cr10_Y110_Cb10},
    {{false, false, false, 32}, BPXL_eY110_Cb10_Y010_Cr10, NEXUS_PixelFormat_eY110_Cb10_Y010_Cr10},
    {{false, false, false, 32}, BPXL_eY110_Cr10_Y010_Cb10, NEXUS_PixelFormat_eY110_Cr10_Y010_Cb10},
    {{false, false, false, 32}, BPXL_eCb10_Y010_Cr10_Y110, NEXUS_PixelFormat_eCb10_Y010_Cr10_Y110},
    {{false, false, false, 32}, BPXL_eCb10_Y110_Cr10_Y010, NEXUS_PixelFormat_eCb10_Y110_Cr10_Y010},
    {{false, false, false, 32}, BPXL_eCr10_Y110_Cb10_Y010, NEXUS_PixelFormat_eCr10_Y110_Cb10_Y010},
    {{false, false, false, 32}, BPXL_eCr10_Y010_Cb10_Y110, NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110},

    {{false, false, false, 8},  BPXL_eL8, NEXUS_PixelFormat_eL8},
    {{false, false, true,  8},  BPXL_eL4_A4, NEXUS_PixelFormat_eL4_A4},
    {{false, false, true,  16}, BPXL_eL8_A8, NEXUS_PixelFormat_eL8_A8},
    {{false, false, true,  8},  BPXL_eL15_L05_A6, NEXUS_PixelFormat_eL15_L05_A6},

    /* bpp for compressed ARGB8888 is 16 with BSTC (or 16.5 with old DCEG) + some less significant number due to alignment */
    {{true,  false, true,  32}, BPXL_eCompressed_A8_R8_G8_B8, NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8},

    {{true, false, false, 24}, BPXL_eR8_G8_B8, NEXUS_PixelFormat_eR8_G8_B8},
    {{false, false, false, 32}, BPXL_eX2_Y010_Cb10_Y110_X2_Cr10_Y010_Cb10_X2_Y110_Cr10_Y010_X2_Cb10_Y110_Cr10, NEXUS_PixelFormat_eYCbCr422_10bit}
};


NEXUS_PixelFormat
NEXUS_P_PixelFormat_FromMagnum_isrsafe(BPXL_Format magnumPixelFormat)
{
    unsigned i;
    for(i=0;i<NEXUS_PixelFormat_eMax;i++) {
#if 0
        /* enable this if things get off */
        if (g_pixelFormatInfo[i].nexus != i) {BDBG_ERR(("NEXUS_PixelFormat %d has a wrong lookup in g_pixelFormatInfo[].", i));}
#endif
        if(g_pixelFormatInfo[i].magnum == magnumPixelFormat) {
            BDBG_ASSERT(g_pixelFormatInfo[i].nexus == i);
            return i;
        }
    }
    return NEXUS_PixelFormat_eUnknown;
}

NEXUS_Error
NEXUS_P_PixelFormat_ToMagnum_isrsafe(NEXUS_PixelFormat nexusPixelFormat, BPXL_Format *magnumPixelFormat)
{
    if (nexusPixelFormat < NEXUS_PixelFormat_eMax) {
        BDBG_ASSERT(g_pixelFormatInfo[nexusPixelFormat].nexus == nexusPixelFormat);
        *magnumPixelFormat = g_pixelFormatInfo[nexusPixelFormat].magnum;
        return NEXUS_SUCCESS;
    }
    return BERR_INVALID_PARAMETER;
}

const NEXUS_PixelFormatConvertInfo *NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(NEXUS_PixelFormat pixelFormat)
{
    if (pixelFormat < NEXUS_PixelFormat_eMax) {
        return &g_pixelFormatInfo[pixelFormat];
    }
    BDBG_ERR(("Invalid NEXUS_PixelFormat %d", pixelFormat));
    return NULL;
}

static const struct {
    BAVC_MatrixCoefficients m_matrix;
    NEXUS_MatrixCoefficients n_matrix;
} b_matrices[] = {
    {BAVC_MatrixCoefficients_eHdmi_RGB,              NEXUS_MatrixCoefficients_eHdmi_RGB},
    {BAVC_MatrixCoefficients_eItu_R_BT_709,          NEXUS_MatrixCoefficients_eItu_R_BT_709},
    {BAVC_MatrixCoefficients_eUnknown,               NEXUS_MatrixCoefficients_eUnknown},
    {BAVC_MatrixCoefficients_eDvi_Full_Range_RGB,    NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB},
    {BAVC_MatrixCoefficients_eFCC,                   NEXUS_MatrixCoefficients_eFCC},
    {BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG,     NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG},
    {BAVC_MatrixCoefficients_eSmpte_170M,            NEXUS_MatrixCoefficients_eSmpte_170M},
    {BAVC_MatrixCoefficients_eSmpte_240M,            NEXUS_MatrixCoefficients_eSmpte_240M},
    {BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL,     NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL},
    {BAVC_MatrixCoefficients_eItu_R_BT_2020_CL,      NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL},
    {BAVC_MatrixCoefficients_eXvYCC_709,             NEXUS_MatrixCoefficients_eXvYCC_709},
    {BAVC_MatrixCoefficients_eXvYCC_601,             NEXUS_MatrixCoefficients_eXvYCC_601},
    {BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr, NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr}
};

BAVC_MatrixCoefficients
NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(NEXUS_MatrixCoefficients n_matrix)
{
    unsigned i;
    for(i=0;i<sizeof(b_matrices)/sizeof(*b_matrices);i++) {
        if (n_matrix==b_matrices[i].n_matrix) {
            return b_matrices[i].m_matrix;
        }
    }
    return BAVC_MatrixCoefficients_eUnknown;
}

NEXUS_MatrixCoefficients
NEXUS_P_MatrixCoefficients_FromMagnum_isrsafe(BAVC_MatrixCoefficients m_matrix)
{
    unsigned i;
    for(i=0;i<sizeof(b_matrices)/sizeof(*b_matrices);i++) {
        if (m_matrix==b_matrices[i].m_matrix) {
            return b_matrices[i].n_matrix;
        }
    }
    return NEXUS_MatrixCoefficients_eUnknown;
}

static const struct {
    BAVC_ColorPrimaries m_color_primary;
    NEXUS_ColorPrimaries n_color_primary;
} b_color_primaries[] = {
    {BAVC_ColorPrimaries_eItu_R_BT_709,       NEXUS_ColorPrimaries_eItu_R_BT_709},
    {BAVC_ColorPrimaries_eUnknown,            NEXUS_ColorPrimaries_eUnknown},
    {BAVC_ColorPrimaries_eItu_R_BT_470_2_M,   NEXUS_ColorPrimaries_eItu_R_BT_470_2_M},
    {BAVC_ColorPrimaries_eItu_R_BT_470_2_BG,  NEXUS_ColorPrimaries_eItu_R_BT_470_2_BG},
    {BAVC_ColorPrimaries_eSmpte_170M,         NEXUS_ColorPrimaries_eSmpte_170M},
    {BAVC_ColorPrimaries_eSmpte_240M,         NEXUS_ColorPrimaries_eSmpte_240M},
    {BAVC_ColorPrimaries_eGenericFilm,        NEXUS_ColorPrimaries_eGenericFilm},
    {BAVC_ColorPrimaries_eItu_R_BT_2020,      NEXUS_ColorPrimaries_eItu_R_BT_2020}
};

BAVC_ColorPrimaries
NEXUS_P_ColorPrimaries_ToMagnum_isrsafe(NEXUS_ColorPrimaries n_color_primary)
{
    unsigned i;
    for(i=0;i<sizeof(b_color_primaries)/sizeof(*b_color_primaries);i++) {
        if (n_color_primary==b_color_primaries[i].n_color_primary) {
            return b_color_primaries[i].m_color_primary;
        }
    }
    return BAVC_ColorPrimaries_eUnknown;
}

NEXUS_ColorPrimaries
NEXUS_P_ColorPrimaries_FromMagnum_isrsafe(BAVC_ColorPrimaries m_color_primary)
{
    unsigned i;
    for(i=0;i<sizeof(b_color_primaries)/sizeof(*b_color_primaries);i++) {
        if (m_color_primary==b_color_primaries[i].m_color_primary) {
            return b_color_primaries[i].n_color_primary;
        }
    }
    return NEXUS_ColorPrimaries_eUnknown;
}

static const struct {
    BAVC_TransferCharacteristics m_transfer;
    NEXUS_TransferCharacteristics n_transfer;
} b_transfers[] = {
    {BAVC_TransferCharacteristics_eItu_R_BT_709,        NEXUS_TransferCharacteristics_eItu_R_BT_709},
    {BAVC_TransferCharacteristics_eUnknown,             NEXUS_TransferCharacteristics_eUnknown},
    {BAVC_TransferCharacteristics_eItu_R_BT_470_2_M,    NEXUS_TransferCharacteristics_eItu_R_BT_470_2_M},
    {BAVC_TransferCharacteristics_eItu_R_BT_470_2_BG,   NEXUS_TransferCharacteristics_eItu_R_BT_470_2_BG},
    {BAVC_TransferCharacteristics_eSmpte_170M,          NEXUS_TransferCharacteristics_eSmpte_170M},
    {BAVC_TransferCharacteristics_eSmpte_240M,          NEXUS_TransferCharacteristics_eSmpte_240M},
    {BAVC_TransferCharacteristics_eLinear,              NEXUS_TransferCharacteristics_eLinear},
    {BAVC_TransferCharacteristics_eIec_61966_2_4,       NEXUS_TransferCharacteristics_eIec_61966_2_4},
    {BAVC_TransferCharacteristics_eItu_R_BT_2020_10bit, NEXUS_TransferCharacteristics_eItu_R_BT_2020_10bit},
    {BAVC_TransferCharacteristics_eItu_R_BT_2020_12bit, NEXUS_TransferCharacteristics_eItu_R_BT_2020_12bit},
    {BAVC_TransferCharacteristics_eSmpte_ST_2084,       NEXUS_TransferCharacteristics_eSmpte_ST_2084},
    {BAVC_TransferCharacteristics_eArib_STD_B67,        NEXUS_TransferCharacteristics_eArib_STD_B67},
};

BAVC_TransferCharacteristics
NEXUS_P_TransferCharacteristics_ToMagnum_isrsafe(NEXUS_TransferCharacteristics n_transfer)
{
    unsigned i;
    for(i=0;i<sizeof(b_transfers)/sizeof(*b_transfers);i++) {
        if (n_transfer==b_transfers[i].n_transfer) {
            return b_transfers[i].m_transfer;
        }
    }
    return BAVC_TransferCharacteristics_eUnknown;
}

NEXUS_TransferCharacteristics
NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(BAVC_TransferCharacteristics m_transfer)
{
    unsigned i;
    for(i=0;i<sizeof(b_transfers)/sizeof(*b_transfers);i++) {
        if (m_transfer==b_transfers[i].m_transfer) {
            return b_transfers[i].n_transfer;
        }
    }
    return NEXUS_TransferCharacteristics_eUnknown;
}

NEXUS_ColorSpace NEXUS_P_ColorSpace_FromMagnum_isrsafe(BAVC_Colorspace colorSpace)
{
    switch (colorSpace) {
    case BAVC_Colorspace_eRGB: return NEXUS_ColorSpace_eRgb;
    case BAVC_Colorspace_eYCbCr444: return NEXUS_ColorSpace_eYCbCr444;
    case BAVC_Colorspace_eYCbCr422: return NEXUS_ColorSpace_eYCbCr422;
    case BAVC_Colorspace_eYCbCr420: return NEXUS_ColorSpace_eYCbCr420;
    default: return NEXUS_ColorSpace_eMax;
    }
}

BAVC_Colorspace NEXUS_P_ColorSpace_ToMagnum_isrsafe(NEXUS_ColorSpace colorSpace)
{
    switch (colorSpace) {
    case NEXUS_ColorSpace_eRgb: return BAVC_Colorspace_eRGB;
    case NEXUS_ColorSpace_eYCbCr444: return BAVC_Colorspace_eYCbCr444;
    case NEXUS_ColorSpace_eYCbCr422: return BAVC_Colorspace_eYCbCr422;
    case NEXUS_ColorSpace_eYCbCr420: return BAVC_Colorspace_eYCbCr420;
    case NEXUS_ColorSpace_eAuto: return BAVC_Colorspace_eYCbCr444;
    default: return BAVC_Colorspace_eFuture;
    }
}

BAVC_HDMI_DRM_EOTF NEXUS_P_VideoEotf_ToMagnum_isrsafe(NEXUS_VideoEotf eotf)
{
    switch (eotf) {
    case NEXUS_VideoEotf_eSdr:         return BAVC_HDMI_DRM_EOTF_eSDR;
    case NEXUS_VideoEotf_eHdr:         return BAVC_HDMI_DRM_EOTF_eHDR;
    case NEXUS_VideoEotf_eSmpteSt2084: return BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084;
    case NEXUS_VideoEotf_eFuture:      return BAVC_HDMI_DRM_EOTF_eFuture;
    default: return BAVC_HDMI_DRM_EOTF_eMax;
    }
}
NEXUS_ColorRange NEXUS_P_ColorRange_FromMagnum_isrsafe(BAVC_ColorRange colorRange)
{
    switch (colorRange) {
    case BAVC_ColorRange_eLimited: return NEXUS_ColorRange_eLimited;
    case BAVC_ColorRange_eFull:    return NEXUS_ColorRange_eFull;
    default:                       return NEXUS_ColorRange_eLimited;
    }
}

BAVC_ColorRange NEXUS_P_ColorRange_ToMagnum_isrsafe(NEXUS_ColorRange colorRange)
{
    switch (colorRange) {
    case NEXUS_ColorRange_eLimited: return BAVC_ColorRange_eLimited;
    case NEXUS_ColorRange_eFull:    return BAVC_ColorRange_eFull;
    default:                        return BAVC_ColorRange_eLimited;
    }
}

BAVC_HDMI_BitsPerPixel NEXUS_P_HdmiColorDepth_ToMagnum_isrsafe(NEXUS_HdmiColorDepth colorDepth)
{
    switch (colorDepth) {
    default: return BAVC_HDMI_BitsPerPixel_e24bit;
    case 10: return BAVC_HDMI_BitsPerPixel_e30bit;
    case 12: return BAVC_HDMI_BitsPerPixel_e36bit;
    case 16: return BAVC_HDMI_BitsPerPixel_e48bit;
    }
}

bool
NEXUS_P_VideoFormat_IsSd(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormatInfo info;
    NEXUS_VideoFormat_GetInfo(format, &info);
    return info.digitalHeight <= BFMT_PAL_HEIGHT && info.interlaced;
}

bool
NEXUS_P_VideoFormat_IsNotAnalogOutput(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormatInfo info;
    if (format == NEXUS_VideoFormat_eUnknown) {
        return false;
    }
    NEXUS_VideoFormat_GetInfo(format, &info);
    return (info.pixelFreq > 21600); /* must disable analog output for pixel freq beyond 216MHz */
}

bool
NEXUS_P_VideoFormat_IsInterlaced(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormatInfo info;
    NEXUS_VideoFormat_GetInfo(format, &info);
    return info.interlaced;
}

/* This function does not promise an exact match. Instead, it returns a NEXUS_VideoFormat which most closely matches the given info.
This allows applications to make general configuration decisions.
Always make sure there's no 50/60 Hz or interlaced/progressive mixup. */
NEXUS_VideoFormat NEXUS_P_VideoFormat_FromInfo_isrsafe(unsigned height, unsigned frameRate, bool interlaced)
{
    bool is50 = (frameRate == NEXUS_VideoFrameRate_e50) || (frameRate == NEXUS_VideoFrameRate_e25);

    if (height > 1088) {
        switch (frameRate) {
        case NEXUS_VideoFrameRate_e23_976:
        case NEXUS_VideoFrameRate_e24:
            return NEXUS_VideoFormat_e3840x2160p24hz;
        case NEXUS_VideoFrameRate_e25: return NEXUS_VideoFormat_e3840x2160p25hz;
        case NEXUS_VideoFrameRate_e29_97: return NEXUS_VideoFormat_e3840x2160p30hz;
        case NEXUS_VideoFrameRate_e30: return NEXUS_VideoFormat_e3840x2160p30hz;
        case NEXUS_VideoFrameRate_e50: return NEXUS_VideoFormat_e3840x2160p50hz;
        default: return NEXUS_VideoFormat_e3840x2160p60hz; /* 60hz */
        }
    }

    if (height > BFMT_720P_HEIGHT) {
        if (interlaced) {
            return is50 ? NEXUS_VideoFormat_e1080i50hz : NEXUS_VideoFormat_e1080i;
        }
        else {
            switch (frameRate) {
            case NEXUS_VideoFrameRate_e23_976:
            case NEXUS_VideoFrameRate_e24:
                return NEXUS_VideoFormat_e1080p24hz;
            case NEXUS_VideoFrameRate_e25: return NEXUS_VideoFormat_e1080p25hz;
            case NEXUS_VideoFrameRate_e29_97: return NEXUS_VideoFormat_e1080p30hz;
            case NEXUS_VideoFrameRate_e30: return NEXUS_VideoFormat_e1080p30hz;
            case NEXUS_VideoFrameRate_e50: return NEXUS_VideoFormat_e1080p50hz;
            default: return NEXUS_VideoFormat_e1080p; /* 60hz */
            }
        }
    }

    /* NOTE: the analog height of 480p is 483 */
    if (((is50 && height > BFMT_576P_HEIGHT) || (!is50 && height > 483)) && !interlaced) {
        switch (frameRate) {
        case NEXUS_VideoFrameRate_e23_976:
        case NEXUS_VideoFrameRate_e24:
            return NEXUS_VideoFormat_e720p24hz;
        case NEXUS_VideoFrameRate_e25:
        case NEXUS_VideoFrameRate_e50:
            return NEXUS_VideoFormat_e720p50hz;
        default:
            return NEXUS_VideoFormat_e720p; /* 60hz */
        }
    }

    if (is50) {
        if (height == 288 && !interlaced) {
            /* handle one specific type */
            return NEXUS_VideoFormat_e288p50hz;
        }

        /* the catch all for 50Hz is 576i/576p */
       return interlaced ? NEXUS_VideoFormat_ePal : NEXUS_VideoFormat_e576p;
    }
    else {
        if (height == 240 && !interlaced) {
            /* handle one specific type */
            return NEXUS_VideoFormat_e240p60hz;
        }

        /* the catch all, if not 50Hz, is 480i/480p */
        return interlaced ? NEXUS_VideoFormat_eNtsc : NEXUS_VideoFormat_e480p;
    }
}

BFMT_Orientation
NEXUS_P_VideoOrientation_ToMagnum_isrsafe(NEXUS_VideoOrientation orientation)
{
    return (BFMT_Orientation)orientation;
}

BAVC_StripeWidth NEXUS_P_StripeWidth_ToMagnum_isrsafe(unsigned stripeWidth)
{
    switch (stripeWidth) {
    default:
    case 128: return BAVC_StripeWidth_e128Byte;
    case 256: return BAVC_StripeWidth_e256Byte;
    case  64: return BAVC_StripeWidth_e64Byte;
    }
}

unsigned NEXUS_P_StripeWidth_FromMagnum_isrsafe(BAVC_StripeWidth stripeWidth)
{
    switch (stripeWidth) {
    default:
    case BAVC_StripeWidth_e128Byte: return 128;
    case BAVC_StripeWidth_e256Byte: return 256;
    case BAVC_StripeWidth_e64Byte:  return 64;
    }
}
