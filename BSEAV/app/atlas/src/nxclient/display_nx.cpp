/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "atlas.h"
#include "display_nx.h"
#include "video_decode_nx.h"
#include "graphics_nx.h"
#include "nxclient.h"
#include "convert.h"

BDBG_MODULE(atlas_display);

CDisplayNx::CDisplayNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CDisplay(name, number, pCfg)
{
}

CDisplayNx::~CDisplayNx()
{
    close();
}

eRet CDisplayNx::open()
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);
    settings.format = _defaultFormat;
    nerror          = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set default output format", ret, nerror, error);

error:
    return(ret);
} /* open */

void CDisplayNx::close()
{
} /* close */

CDisplayVbiData CDisplayNx::getVbiSettings()
{
    CDisplayVbiData      vbiData;
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode();

    if (NULL != pVideoDecode)
    {
        NEXUS_SimpleVideoDecoderClientSettings settings;

        NEXUS_SimpleVideoDecoder_GetClientSettings(pVideoDecode->getSimpleDecoder(), &settings);
        vbiData.bClosedCaptions = settings.closedCaptionRouting;
    }

    return(vbiData);
}

eRet CDisplayNx::setVbiSettings(CDisplayVbiData * pSettings)
{
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode();
    eRet                 ret          = eRet_Ok;
    NEXUS_Error          nerror       = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pSettings);

    if (NULL != pVideoDecode)
    {
        NEXUS_SimpleVideoDecoderClientSettings settings;

        NEXUS_SimpleVideoDecoder_GetClientSettings(pVideoDecode->getSimpleDecoder(), &settings);
        settings.closedCaptionRouting = pSettings->bClosedCaptions;
        nerror                        = NEXUS_SimpleVideoDecoder_SetClientSettings(pVideoDecode->getSimpleDecoder(), &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set vbi nxclient settings", ret, nerror, error);
    }

    goto done;
error:
    notifyObservers(eNotify_ErrorVbi, &ret);
    goto done;
done:
    notifyObservers(eNotify_VbiSettingsChanged, this);
    return(ret);
} /* setVbiSettings */

MRect CDisplayNx::getGraphicsGeometry()
{
    BDBG_WRN(("unable to get graphics geometry in NxClient mode"));
    return(MRect(0, 0, 0, 0));
}

eRet CDisplayNx::updateGraphicsGeometry()
{
    eRet ret = eRet_Ok;

    BDBG_WRN(("unable to update graphics geometry in NxClient mode"));
    return(ret);
} /* updateGraphicsGeometry */

eRet CDisplayNx::setContentMode(NEXUS_VideoWindowContentMode contentMode)
{
    eRet                        ret          = eRet_Ok;
    NEXUS_Error                 nerror       = NEXUS_SUCCESS;
    CSimpleVideoDecodeNx *      pVideoDecode = NULL;
    NEXUS_SurfaceClientHandle   scVideoWin   = NULL;
    NEXUS_SurfaceClientSettings settings;

    pVideoDecode = (CSimpleVideoDecodeNx *)_pModel->getSimpleVideoDecode();
    CHECK_PTR_ERROR_GOTO("unable to set content mode because video decode is unavailable", pVideoDecode, ret, eRet_NotAvailable, error);

    scVideoWin = pVideoDecode->getDesktopClientVideoWin();
    CHECK_PTR_ERROR_GOTO("unable to set content mode because video window is unavailable", scVideoWin, ret, eRet_NotAvailable, error);

    NEXUS_SurfaceClient_GetSettings(scVideoWin, &settings);
    settings.composition.contentMode = contentMode;
    nerror                           = NEXUS_SurfaceClient_SetSettings(scVideoWin, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set content mode", ret, nerror, error);

error:
    return(ret);
} /* setContentMode */

eRet CDisplayNx::setColorDepth(uint8_t * pColorDepth)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;
    eNotification            notification = eNotify_Invalid;

    BDBG_ASSERT(NULL != pColorDepth);

    NxClient_GetDisplaySettings(&settings);

    if (settings.hdmiPreferences.colorDepth == *pColorDepth)
    {
        /* nothing changed - done */
        return(ret);
    }

    settings.hdmiPreferences.colorDepth = *pColorDepth;
    nerror                              = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set color depth", ret, nerror, error);

error:
    /* color depth takes a few display vsyncs to complete */
    BKNI_Sleep(50);
    NxClient_GetDisplaySettings(&settings);
    if (*pColorDepth != settings.hdmiPreferences.colorDepth)
    {
        ret = eRet_NotSupported;
    }

    *pColorDepth = settings.hdmiPreferences.colorDepth;

    notification = (ret == eRet_Ok ? eNotify_ColorDepthChanged : eNotify_ColorDepthFailure);
    notifyObservers(notification, &(settings.hdmiPreferences.colorDepth));

    return(ret);
} /* setColorDepth */

eRet CDisplayNx::setColorSpace(NEXUS_ColorSpace * pColorSpace)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;
    eNotification            notification = eNotify_Invalid;

    BDBG_ASSERT(NULL != pColorSpace);

    NxClient_GetDisplaySettings(&settings);

    if (settings.hdmiPreferences.colorSpace == *pColorSpace)
    {
        /* nothing changed - done */
        return(ret);
    }

    settings.hdmiPreferences.colorSpace = *pColorSpace;
    nerror                              = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set color space", ret, nerror, error);

error:
    /* color space takes a few display vsyncs to complete */
    BKNI_Sleep(50);
    NxClient_GetDisplaySettings(&settings);
    if (*pColorSpace != settings.hdmiPreferences.colorSpace)
    {
        ret = eRet_NotSupported;
    }

    *pColorSpace = settings.hdmiPreferences.colorSpace;

    notification = (ret == eRet_Ok ? eNotify_ColorSpaceChanged : eNotify_ColorSpaceFailure);
    notifyObservers(notification, &(settings.hdmiPreferences.colorSpace));

    return(ret);
} /* setColorSpace */

/* set preferred colorspace for ouputs that support it */
eRet CDisplayNx::setPreferredColorSpace(NEXUS_VideoFormat format)
{
    eRet             ret        = eRet_Ok;
    NEXUS_ColorSpace colorSpace = NEXUS_ColorSpace_eAuto;

    BSTD_UNUSED(format);

    ret = setColorSpace(&colorSpace);
    CHECK_ERROR_GOTO("unable to set color space", ret, error);

error:
    return(ret);
} /* setPreferredColorSpace */

eRet CDisplayNx::setDeinterlacer(bool bDeinterlacer)
{
    eRet                 ret          = eRet_Ok;
    NEXUS_Error          nerror       = NEXUS_SUCCESS;
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode();
    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;

    BDBG_MSG(("setting deinterlacer nx:%s", bDeinterlacer ? "true" : "false"));

    BDBG_ASSERT(NULL != pVideoDecode);

    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(pVideoDecode->getSimpleDecoder(), &settings);
    settings.mad.deinterlace = bDeinterlacer;
    nerror                   = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(pVideoDecode->getSimpleDecoder(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set deinterlacer settings", ret, nerror, error);

    notifyObservers(eNotify_DeinterlacerChanged, &bDeinterlacer);
error:
    return(ret);
} /* setDeinterlacer */

eRet CDisplayNx::setMpaaDecimation(bool bMpaaDecimation)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;

    BDBG_MSG(("setting Mpaa Decimation nx:%s", bMpaaDecimation ? "true" : "false"));

    NxClient_GetDisplaySettings(&settings);
    settings.componentPreferences.mpaaDecimationEnabled = bMpaaDecimation;
    nerror = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set mpaa decimation setting", ret, nerror, error);

    notifyObservers(eNotify_MpaaDecimationChanged, &bMpaaDecimation);

error:
    return(ret);
} /* setMpaaDecimation */

eRet CDisplayNx::setBoxDetect(bool bBoxDetect)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bBoxDetect);

    BDBG_WRN(("unable to set Box Detect in NxClient mode"));
    return(ret);
} /* setBoxDetect */

NEXUS_DisplayAspectRatio CDisplayNx::getAspectRatio()
{
    NxClient_DisplaySettings settings;
    NEXUS_DisplayAspectRatio aspectRatio = NEXUS_DisplayAspectRatio_eMax;

    NxClient_GetDisplaySettings(&settings);
    if (0 < getNumber())
    {
        aspectRatio = settings.slaveDisplay[getNumber() - 1].aspectRatio;
    }
    else
    {
        aspectRatio = settings.aspectRatio;
    }

    return(aspectRatio);
} /* getAspectRatio */

eRet CDisplayNx::setAspectRatio(NEXUS_DisplayAspectRatio aspectRatio)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);
    if (0 < getNumber())
    {
        BDBG_MSG(("%s for slave display - do nothing", __FUNCTION__));
        goto error;
    }
    else
    {
        settings.aspectRatio = aspectRatio;
        BDBG_MSG(("%s display:%d aspect ratio:%d", __FUNCTION__, getNumber(), aspectRatio));
    }
    nerror = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set aspect ratio", ret, nerror, error);

    notifyObservers(eNotify_AspectRatioChanged, &aspectRatio);

error:
    return(ret);
} /* setAspectRatio */

eRet CDisplayNx::setFormat(
        NEXUS_VideoFormat format,
        CGraphics *       pGraphics,
        bool              bNotify
        )
{
    eRet                     ret         = eRet_Ok;
    NEXUS_Error              nerror      = NEXUS_SUCCESS;
    NEXUS_VideoFormat        formatOrig  = NEXUS_VideoFormat_eUnknown;
    NEXUS_VideoFormat        formatValid = format;
    NxClient_DisplaySettings settings;

    BSTD_UNUSED(pGraphics);

    BDBG_ASSERT(NEXUS_VideoFormat_eUnknown != format);
    BDBG_ASSERT(NEXUS_VideoFormat_eMax != format);
    BDBG_ASSERT(NULL != _pModel);

    formatOrig = getFormat();

    if (getFormat() == formatValid)
    {
        /* no change needed */
        goto done;
    }

    /* set display format */
    BDBG_MSG(("set display %d format:%s(%d)", getNumber(), videoFormatToString(format).s(), format));
    NxClient_GetDisplaySettings(&settings);
    settings.format = format;
    nerror          = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set video format", ret, nerror, error);

done:
    if (true == bNotify)
    {
        notifyObservers(eNotify_VideoFormatChanged, &formatValid);
    }
    goto finish;
error:
    notifyObservers(eNotify_VideoFormatChanged, &formatOrig);
finish:
    return(ret);
} /* setFormat */

NEXUS_VideoFormat CDisplayNx::getFormat()
{
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);
    return(settings.format);
} /* getFormat */

eRet CDisplayNx::setUpdateMode(bool bAuto)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bAuto);

    BDBG_WRN(("Update mode is not supported in NxClient mode"));

    return(ret);
} /* setUpdateMode */

bool CDisplayNx::isStandardDef()
{
    bool  bSD          = false;
    MRect rectGraphics = getMaxGraphicsGeometry();

    if (MString(videoFormatToHorizRes(NEXUS_VideoFormat_eNtsc)).toInt() == (int)rectGraphics.width())
    {
        bSD = true;
    }

    return(bSD);
}

eRet CDisplayNx::addOutput(COutput * pOutput)
{
    eRet ret = eRet_InvalidParameter; /* assume error */

    BDBG_ASSERT(NULL != pOutput);

    if (pOutput->isVideoOutput())
    {
        _outputList.add(pOutput);
        _bOutputsEnabled = true;
        ret              = eRet_Ok;
    }

    return(ret);
} /* addOutput */

eRet CDisplayNx::removeOutput(COutput * pOutput)
{
    eRet ret = eRet_InvalidParameter; /* assume error */

    if (NULL == pOutput)
    {
        return(ret);
    }

    if (pOutput->isVideoOutput())
    {
        _outputList.remove(pOutput);
        if (0 == _outputList.total())
        {
            _bOutputsEnabled = false;
        }
        ret = eRet_Ok;
    }

    return(ret);
} /* removeOutput */

eRet CDisplayNx::enableOutputs(bool bEnable )
{
    eRet ret = eRet_Ok; /* does nothing */

    if (_bOutputsEnabled == bEnable)
    {
        /* already in requested enable state */
        return(ret);
    }

    _bOutputsEnabled = bEnable;
    return(ret);
} /* removeOutput */

eDynamicRange CDisplayNx::getOutputDynamicRange()
{
    eDynamicRange            dynamicRange = eDynamicRange_Unknown;
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);
    if (NEXUS_HdmiOutputDolbyVisionMode_eDisabled != settings.hdmiPreferences.dolbyVision.outputMode)
    {
        dynamicRange = eDynamicRange_DolbyVision;
    }
    else
    if (NEXUS_VideoEotf_eHdr10 == settings.hdmiPreferences.drmInfoFrame.eotf)
    {
        dynamicRange = eDynamicRange_HDR10;
    }
    else
    if (NEXUS_VideoEotf_eHlg == settings.hdmiPreferences.drmInfoFrame.eotf)
    {
        dynamicRange = eDynamicRange_HLG;
    }
    else
    {
        dynamicRange = eDynamicRange_SDR;
    }

    return(dynamicRange);
} /* getOutputDynamicRange() */

#define SMD_TO_SMPTE_ST2086(X) ((X)/0.00002)

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_ZERO =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { 0, 0 }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_BT709 =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.64), SMD_TO_SMPTE_ST2086(0.33) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.30), SMD_TO_SMPTE_ST2086(0.60) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.15), SMD_TO_SMPTE_ST2086(0.06) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_BT2020 =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.708), SMD_TO_SMPTE_ST2086(0.292) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.170), SMD_TO_SMPTE_ST2086(0.797) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.131), SMD_TO_SMPTE_ST2086(0.046) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

eRet CDisplayNx::setOutputDynamicRange(eDynamicRange dynamicRange)
{
    eRet                     ret       = eRet_Ok;
    NEXUS_Error              nerror    = NEXUS_SUCCESS;
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);

    settings.hdmiPreferences.dolbyVision.outputMode =
        (dynamicRange == eDynamicRange_DolbyVision) ? NEXUS_HdmiOutputDolbyVisionMode_eAuto : NEXUS_HdmiOutputDolbyVisionMode_eDisabled;

    switch(dynamicRange)
    {
    case eDynamicRange_HDR10:
        settings.hdmiPreferences.drmInfoFrame.eotf = NEXUS_VideoEotf_eHdr10;
        BKNI_Memcpy(&settings.hdmiPreferences.drmInfoFrame.metadata,
                    &SMD_BT2020,
                    sizeof(settings.hdmiPreferences.drmInfoFrame.metadata));
        break;

    case eDynamicRange_HLG:
        settings.hdmiPreferences.drmInfoFrame.eotf = NEXUS_VideoEotf_eHlg;
        BKNI_Memcpy(&settings.hdmiPreferences.drmInfoFrame.metadata,
                    &SMD_ZERO,
                    sizeof(settings.hdmiPreferences.drmInfoFrame.metadata));
        break;

    case eDynamicRange_DolbyVision:
        settings.hdmiPreferences.drmInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
        BKNI_Memcpy(&settings.hdmiPreferences.drmInfoFrame.metadata,
                    &SMD_ZERO,
                    sizeof(settings.hdmiPreferences.drmInfoFrame.metadata));
        break;

    case eDynamicRange_SDR:
        settings.hdmiPreferences.drmInfoFrame.eotf = NEXUS_VideoEotf_eSdr;
        BKNI_Memcpy(&settings.hdmiPreferences.drmInfoFrame.metadata,
                    &SMD_ZERO,
                    sizeof(settings.hdmiPreferences.drmInfoFrame.metadata));
        break;

    default:
        settings.hdmiPreferences.drmInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
        BKNI_Memcpy(&settings.hdmiPreferences.drmInfoFrame.metadata,
                    &SMD_ZERO,
                    sizeof(settings.hdmiPreferences.drmInfoFrame.metadata));
        break;
    }

    nerror = NxClient_SetDisplaySettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set display settings", ret, nerror, error);
    waitForDisplaySettingsApply();

    _pModel->setLastDynamicRange(dynamicRange);
error:
    return(ret);
} /* setOutputDynamicRange() */