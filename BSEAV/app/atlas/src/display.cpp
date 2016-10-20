/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "convert.h"
#include "display.h"
#include "convert.h"
#include "nexus_display_vbi.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif

BDBG_MODULE(atlas_display);

CDisplay::CDisplay(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_display, pCfg),
    _display(NULL),
    _bAutoFormat(false),
    _defaultFormat(NEXUS_VideoFormat_eNtsc),
    _pModel(NULL),
    _rectMaxGraphicsGeometry(0, 0, 1920, 1080),
    _bOutputsEnabled(false)
{
}

CDisplay::~CDisplay()
{
    close();
}

void CDisplay::dump()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();

    BDBG_MSG(("%s:%d (video windows:%d)",
              _name.s(),
              _number,
              pPlatformConfig->getNumWindowsPerDisplay()));
}

eRet CDisplay::open()
{
    NEXUS_DisplaySettings settings;
    eRet                  ret             = eRet_Ok;
    NEXUS_Error           nerror          = NEXUS_SUCCESS;
    uint16_t              maxVideoWindows = 0;
    CPlatform *           pPlatformConfig = NULL;

    pPlatformConfig = _pCfg->getPlatformConfig();
    BDBG_ASSERT(NULL != pPlatformConfig);

    /* create list of video windows associated with this display */
    maxVideoWindows = pPlatformConfig->getNumWindowsPerDisplay();

    for (uint16_t i = 0; maxVideoWindows > i; i++)
    {
        if (true == pPlatformConfig->isSupportedVideoWindow(getNumber(), i))
        {
            _videoWindowList.add(new CVideoWindow(i, this, _pCfg));
        }
    }

    NEXUS_Display_GetDefaultSettings(&settings);
    settings.format = _defaultFormat;

    _display = NEXUS_Display_Open(_number, &settings);
    CHECK_PTR_ERROR_GOTO("nexus display open failed!", _display, ret, eRet_ExternalError, error);

    {
        NEXUS_VideoFormatInfo  videoFormatInfo;
        NEXUS_GraphicsSettings graphicsSettings;

        NEXUS_VideoFormat_GetInfo(settings.format, &videoFormatInfo);

        NEXUS_Display_GetGraphicsSettings(getDisplay(), &graphicsSettings);
        graphicsSettings.enabled     = true;
        graphicsSettings.clip.width  = videoFormatInfo.width;
        graphicsSettings.clip.height = videoFormatInfo.height;
        nerror                       = NEXUS_Display_SetGraphicsSettings(getDisplay(), &graphicsSettings);
        CHECK_NEXUS_ERROR_GOTO("Error setting graphics settings", ret, nerror, error);
    }

    return(ret);

error:
    if (_display)
    {
        NEXUS_Display_Close(_display);
        _display = NULL;
    }

    return(ret);
} /* open */

void CDisplay::close()
{
    CVideoWindow * pVideoWindow = NULL;

    MListItr <CVideoWindow> itrWindows(&_videoWindowList);

    if (NULL != _display)
    {
        NEXUS_Display_RemoveAllOutputs(_display);
        _outputList.clear();
    }

    /* close any open video windows */
    for (pVideoWindow = itrWindows.first(); pVideoWindow; pVideoWindow = itrWindows.next())
    {
        if (true == pVideoWindow->isCheckedOut())
        {
            pVideoWindow->setCheckedOut(false);
        }

        pVideoWindow->close();
    }
    _videoWindowList.clear();

    if (NULL != _display)
    {
        NEXUS_Display_Close(_display);
        _display = NULL;
    }

    _pModel = NULL;
} /* close */

CVideoWindow * CDisplay::checkoutVideoWindow()
{
    CVideoWindow * pVideoWindow           = NULL;
    CVideoWindow * pCheckedOutVideoWindow = NULL;

    MListItr <CVideoWindow> itr(&_videoWindowList);

    for (pVideoWindow = itr.first(); pVideoWindow; pVideoWindow = itr.next())
    {
        if (false == pVideoWindow->isCheckedOut())
        {
            pCheckedOutVideoWindow = pVideoWindow;
            pCheckedOutVideoWindow->setCheckedOut(true);
            break;
        }
    }

    return(pCheckedOutVideoWindow);
} /* checkoutVideoWindow */

eRet CDisplay::checkinVideoWindow(CVideoWindow * pWindow)
{
    if (NULL == pWindow)
    {
        CVideoWindow *          pVideoWindow = NULL;
        MListItr <CVideoWindow> itr(&_videoWindowList);

        /* checkin all video windows */
        for (pVideoWindow = itr.first(); pVideoWindow; pVideoWindow = itr.next())
        {
            if (true == pVideoWindow->isCheckedOut())
            {
                pVideoWindow->close();
                pVideoWindow->setCheckedOut(false);
            }
        }
    }
    else
    {
        /* checkin given video window */
        if (pWindow->isCheckedOut())
        {
            pWindow->close();
            pWindow->setCheckedOut(false);
        }
    }

    return(eRet_Ok);
} /* checkinVideoWindow */

eRet CDisplay::addOutput(COutput * pOutput)
{
    eRet ret = eRet_InvalidParameter; /* assume error */

    BDBG_ASSERT(NULL != pOutput);

    if (pOutput->isVideoOutput())
    {
        NEXUS_Error nerror = NEXUS_SUCCESS;

        nerror = NEXUS_Display_AddOutput(_display, pOutput->getConnectorV());
        CHECK_NEXUS_ERROR_GOTO("failure to add output", ret, nerror, error);
        _outputList.add(pOutput);
        _bOutputsEnabled = true;
        ret              = eRet_Ok;
    }

error:
    return(ret);
} /* addOutput */

eRet CDisplay::removeOutput(COutput * pOutput)
{
    eRet ret = eRet_InvalidParameter; /* assume error */

    if (NULL == pOutput)
    {
        return(ret);
    }

    if (pOutput->isVideoOutput())
    {
        NEXUS_Error nerror = NEXUS_SUCCESS;

        nerror = NEXUS_Display_RemoveOutput(_display, pOutput->getConnectorV());
        CHECK_NEXUS_ERROR_GOTO("failure to remove output", ret, nerror, error);
        _outputList.remove(pOutput);
        if (0 == _outputList.total())
        {
            _bOutputsEnabled = false;
        }
        ret = eRet_Ok;
    }

error:
    return(ret);
} /* removeOutput */

COutput * CDisplay::getOutput(eBoardResource outputType)
{
    COutput * pOutput = NULL;

    for (pOutput = _outputList.first(); pOutput; pOutput = _outputList.next())
    {
        if (outputType == pOutput->getType())
        {
            break;
        }
    }

    return(pOutput);
}

eRet CDisplay::enableOutputs(bool bEnable)
{
    NEXUS_Error nerror  = NEXUS_SUCCESS;
    eRet        ret     = eRet_Ok;
    COutput *   pOutput = NULL;

    if (_bOutputsEnabled == bEnable)
    {
        /* already in requested enable state */
        return(ret);
    }

    MListItr <COutput> itr(&_outputList);

    for (pOutput = itr.first(); pOutput; pOutput = itr.next())
    {
        if (pOutput->isVideoOutput())
        {
            if (true == bEnable)
            {
                nerror = NEXUS_Display_AddOutput(_display, pOutput->getConnectorV());
                CHECK_NEXUS_ERROR_GOTO("failure to add output", ret, nerror, error);

                /*
                 * do not add to output list since we are only enabling existing outputs
                 * _outputList.add(pOutput);
                 */
            }
            else
            {
                nerror = NEXUS_Display_RemoveOutput(_display, pOutput->getConnectorV());
                CHECK_NEXUS_ERROR_GOTO("failure to remove output", ret, nerror, error);

                /*
                 * do not remove from output list since we are only disabling outputs
                 * _outputList.remove(pOutput);
                 */
            }
        }
    }

    _bOutputsEnabled = bEnable;

error:
    return(ret);
} /* disableOutputs */

eRet CDisplay::setFramebuffer(NEXUS_SurfaceHandle surface)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    BDBG_ASSERT(NULL != surface);

    nerror = NEXUS_Display_SetGraphicsFramebuffer(getDisplay(), surface);
    CHECK_NEXUS_ERROR_GOTO("Unable to set graphics framebuffer.", ret, nerror, error);

error:
    return(ret);
}

bool CDisplay::isMacrovisionCompatible(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormat myFormat    = format;
    bool              bCompatible = false;

    if (NEXUS_VideoFormat_eUnknown == format)
    {
        myFormat = getFormat();
    }

    switch (myFormat)
    {
    case NEXUS_VideoFormat_eNtsc:      /* 480i, NTSC-M for North America */
    case NEXUS_VideoFormat_eNtsc443:   /* NTSC encoding with the PAL color carrier frequency. */
    case NEXUS_VideoFormat_eNtscJapan: /* Japan NTSC, no pedestal level */
    case NEXUS_VideoFormat_ePalM:      /* PAL Brazil */
    case NEXUS_VideoFormat_ePalN:      /* PAL_N */
    case NEXUS_VideoFormat_ePalNc:     /* PAL_N, Argentina */
    case NEXUS_VideoFormat_ePalB:      /* Australia */
    case NEXUS_VideoFormat_ePalB1:     /* Hungary */
    case NEXUS_VideoFormat_ePalD:      /* China */
    case NEXUS_VideoFormat_ePalD1:     /* Poland */
    case NEXUS_VideoFormat_ePalDK2:    /* Eastern Europe */
    case NEXUS_VideoFormat_ePalDK3:    /* Eastern Europe */
    case NEXUS_VideoFormat_ePalG:      /* Europe. Same as NEXUS_VideoFormat_ePal. */
    case NEXUS_VideoFormat_ePalH:      /* Europe */
    case NEXUS_VideoFormat_ePalK:      /* Europe */
    case NEXUS_VideoFormat_ePalI:      /* U.K. */
    case NEXUS_VideoFormat_ePal60hz:   /* 60Hz PAL */
    case NEXUS_VideoFormat_e480p:      /* NTSC Progressive (27Mhz) */
    case NEXUS_VideoFormat_e576p:      /* HD PAL Progressive 50hz for Australia */
        bCompatible = true;
        break;
    default:
        break;
    } /* switch */

    return(bCompatible);
} /* isMacrovisionCompatible */

void CDisplay::validateMacrovision(NEXUS_VideoFormat format)
{
    eRet            ret         = eRet_Ok;
    CDisplayVbiData vbiDataOrig = _vbiSettings;
    CDisplayVbiData vbiData     = getVbiSettings();

    if (true == isMacrovisionCompatible(format))
    {
        if (_vbiSettings.macrovisionType != vbiData.macrovisionType)
        {
            /* video format is compatible with macrovision so
             * set based on local saved settings */
            BDBG_MSG(("restore macrovision type:%s", macrovisionToString(_vbiSettings.macrovisionType).s()));

            ret = setVbiSettings(&_vbiSettings);
            CHECK_ERROR_GOTO("error setting macrovision", ret, error);
        }
    }
    else
    {
        if (NEXUS_DisplayMacrovisionType_eNone != _vbiSettings.macrovisionType)
        {
            /* video format is NOT compatible with macrovision so
             * disable and save original macrovision setting */
            BDBG_MSG(("disable macrovision setting based on video format:%s", videoFormatToString(format).s()));

            vbiData.macrovisionType = NEXUS_DisplayMacrovisionType_eNone;
            ret                     = setVbiSettings(&vbiData);
            CHECK_ERROR_GOTO("error setting macrovision", ret, error);

            /* restore local copy of original vbi settings */
            _vbiSettings = vbiDataOrig;
        }
    }

error:
    return;
} /* validateMacrovision */

eRet CDisplay::setVbiSettings(CDisplayVbiData * pSettings)
{
    eRet                     ret           = eRet_Ok;
    NEXUS_Error              nerror        = NEXUS_SUCCESS;
    CSimpleVideoDecode *     pSimpleDecode = NULL;
    CVideoDecode *           pVideoDecode  = NULL;
    NEXUS_DisplayVbiSettings displayVbiSettings;

    pSimpleDecode = _pModel->getSimpleVideoDecode();
    CHECK_PTR_ERROR_GOTO("unable to set VBI settings- simple video decoder", pSimpleDecode, ret, eRet_NotAvailable, errorVbi);
    pVideoDecode = pSimpleDecode->getVideoDecoder();
    CHECK_PTR_ERROR_GOTO("unable to set VBI settings - video decoder", pVideoDecode, ret, eRet_NotAvailable, errorVbi);

    NEXUS_Display_GetVbiSettings(getDisplay(), &displayVbiSettings);
    displayVbiSettings.vbiSource            = pVideoDecode->getConnector();
    displayVbiSettings.closedCaptionEnabled = pSettings->bClosedCaptions;
    displayVbiSettings.closedCaptionRouting = pSettings->bClosedCaptions;
    displayVbiSettings.teletextEnabled      = pSettings->bTeletext;
    displayVbiSettings.wssEnabled           = pSettings->bWss;
    displayVbiSettings.cgmsEnabled          = pSettings->bCgms;
    displayVbiSettings.vpsEnabled           = pSettings->bVps;
    displayVbiSettings.gemStarEnabled       = pSettings->bGemstar;
    displayVbiSettings.amolEnabled          = (NEXUS_AmolType_eMax == pSettings->amolType) ? false : true;
    displayVbiSettings.amol.type            = pSettings->amolType;
    nerror = NEXUS_Display_SetVbiSettings(getDisplay(), &displayVbiSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set vbi settings", ret, nerror, errorVbi);

    if ((_vbiSettings.macrovisionType != pSettings->macrovisionType) && (true == isMacrovisionCompatible()))
    {
        BDBG_WRN(("setting Macrovision type:%s", macrovisionToString(pSettings->macrovisionType).s()));
        nerror = NEXUS_Display_SetMacrovision(getDisplay(), pSettings->macrovisionType, NULL);
        CHECK_NEXUS_ERROR_GOTO("unable to set Macrovision type", ret, nerror, errorMacrovision);
    }

    if (_vbiSettings.dcsType != pSettings->dcsType)
    {
        BDBG_WRN(("setting DCS type:%s", dcsToString(pSettings->dcsType).s()));
        nerror = NEXUS_Display_SetDcs(getDisplay(), pSettings->dcsType);
        CHECK_NEXUS_ERROR_GOTO("unable to set DCS type", ret, nerror, errorDcs);
    }

    /* save vbi settings locally */
    _vbiSettings = *pSettings;

    goto done;
errorVbi:
    notifyObservers(eNotify_ErrorVbi, &ret);
    goto done;
errorMacrovision:
    notifyObservers(eNotify_ErrorVbiMacrovision, &ret);
    goto done;
errorDcs:
    notifyObservers(eNotify_ErrorVbiDcs, &ret);
    goto done;
done:
    BDBG_WRN(("%s bCC:%d bTeletext:%d bWss:%d bCgms:%d bVps:%d bGemstar:%d bAmol:%d",
              __FUNCTION__, pSettings->bClosedCaptions, pSettings->bTeletext, pSettings->bWss, pSettings->bCgms,
              pSettings->bVps, pSettings->bGemstar,
              (NEXUS_AmolType_eMax == pSettings->amolType) ? false : true));
    notifyObservers(eNotify_VbiSettingsChanged, this);
    return(ret);
} /* setVbiSettings */

CDisplayVbiData CDisplay::getVbiSettings()
{
    CDisplayVbiData          settings;
    NEXUS_DisplayVbiSettings displayVbiSettings;

    NEXUS_Display_GetVbiSettings(getDisplay(), &displayVbiSettings);
    settings.bClosedCaptions = _vbiSettings.bClosedCaptions;
    settings.bTeletext       = displayVbiSettings.teletextEnabled;
    settings.bWss            = displayVbiSettings.wssEnabled;
    settings.bCgms           = displayVbiSettings.cgmsEnabled;
    settings.bVps            = displayVbiSettings.vpsEnabled;
    settings.bGemstar        = displayVbiSettings.gemStarEnabled;
    settings.amolType        = displayVbiSettings.amolEnabled ? displayVbiSettings.amol.type : NEXUS_AmolType_eMax;
    settings.macrovisionType = _vbiSettings.macrovisionType;
    settings.dcsType         = _vbiSettings.dcsType;

    return(settings);
} /* getVbiSettings */

MRect CDisplay::getGraphicsGeometry()
{
    NEXUS_GraphicsSettings graphicsSettings;
    MRect                  rect;

    NEXUS_Display_GetGraphicsSettings(getDisplay(), &graphicsSettings);
    rect.set(graphicsSettings.clip.x,
            graphicsSettings.clip.y,
            graphicsSettings.clip.width,
            graphicsSettings.clip.height);

    return(rect);
}

eRet CDisplay::updateGraphicsGeometry()
{
    NEXUS_DisplaySettings  settings;
    NEXUS_VideoFormatInfo  videoFormatInfo;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    eRet                   ret    = eRet_Ok;

    NEXUS_Display_GetSettings(getDisplay(), &settings);
    NEXUS_VideoFormat_GetInfo(settings.format, &videoFormatInfo);

    NEXUS_Display_GetGraphicsSettings(getDisplay(), &graphicsSettings);
    graphicsSettings.enabled     = true;
    graphicsSettings.clip.width  = videoFormatInfo.width;
    graphicsSettings.clip.height = videoFormatInfo.height;
    nerror                       = NEXUS_Display_SetGraphicsSettings(getDisplay(), &graphicsSettings);
    CHECK_NEXUS_ERROR_GOTO("Error setting display graphics settings", ret, nerror, error);
error:
    return(ret);
} /* updateGraphicsGeometry */

eRet CDisplay::setContentMode(NEXUS_VideoWindowContentMode contentMode)
{
    eRet           ret          = eRet_Ok;
    CVideoWindow * pVideoWindow = NULL;

    MListItr <CVideoWindow> itr(&_videoWindowList);

    /* only set deinterlacing on primary video window */
    pVideoWindow = itr.first();
    CHECK_PTR_ERROR_GOTO("No video window available", pVideoWindow, ret, eRet_NotAvailable, error);

    ret = pVideoWindow->setContentMode(contentMode);
    CHECK_ERROR_GOTO("unable to set video window content mode", ret, error);

    notifyObservers(eNotify_ContentModeChanged, &contentMode);

error:
    return(ret);
} /* setContentMode */

NEXUS_VideoWindowContentMode CDisplay::getContentMode(void)
{
    CVideoWindow *               pVideoWindow = NULL;
    NEXUS_VideoWindowContentMode contentMode  = NEXUS_VideoWindowContentMode_eMax;

    MListItr <CVideoWindow> itr(&_videoWindowList);

    /* only set deinterlacing on primary video window */
    pVideoWindow = itr.first();
    if (NULL != pVideoWindow)
    {
        contentMode = pVideoWindow->getContentMode();
    }

    return(contentMode);
}

eRet CDisplay::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    eRet      ret     = eRet_Ok;
    COutput * pOutput = NULL;

    BDBG_ASSERT(NEXUS_ColorSpace_eMax != colorSpace);
    MListItr <COutput> itr(&_outputList);

    for (pOutput = itr.first(); pOutput; pOutput = itr.next())
    {
        ret = pOutput->setColorSpace(colorSpace);
        CHECK_ERROR_GOTO("unable to set color space", ret, error);
    }

    notifyObservers(eNotify_ColorSpaceChanged, &colorSpace);

error:
    return(ret);
} /* setColorSpace */

/* set preferred colorspace for ouputs that support it */
eRet CDisplay::setPreferredColorSpace(NEXUS_VideoFormat format)
{
    eRet             ret        = eRet_Ok;
    COutput *        pOutput    = NULL;
    NEXUS_ColorSpace colorSpace = NEXUS_ColorSpace_eMax;

    MListItr <COutput> itr(&_outputList);

    for (pOutput = itr.first(); pOutput; pOutput = itr.next())
    {
        colorSpace = pOutput->getPreferredColorSpace(format);
        if (NEXUS_ColorSpace_eMax != colorSpace)
        {
            ret = pOutput->setColorSpace(colorSpace);
            CHECK_ERROR_GOTO("unable to set color space", ret, error);

            notifyObservers(eNotify_ColorSpaceChanged, &colorSpace);
        }
    }

error:
    return(ret);
} /* setPreferredColorSpace */

eRet CDisplay::setMpaaDecimation(bool bMpaaDecimation)
{
    eRet      ret     = eRet_Ok;
    COutput * pOutput = NULL;

    MListItr <COutput> itr(&_outputList);

    for (pOutput = itr.first(); pOutput; pOutput = itr.next())
    {
        ret = pOutput->setMpaaDecimation(bMpaaDecimation);
        CHECK_ERROR_GOTO("unable to set mpaa decimation", ret, error);
    }

    notifyObservers(eNotify_MpaaDecimationChanged, &bMpaaDecimation);

error:
    return(ret);
} /* setMpaaDecimation */

eRet CDisplay::setDeinterlacer(bool bDeinterlacer)
{
    eRet           ret          = eRet_Ok;
    CVideoWindow * pVideoWindow = NULL;

    MListItr <CVideoWindow> itr(&_videoWindowList);

    /* only set deinterlacing on primary video window */
    pVideoWindow = itr.first();
    CHECK_PTR_ERROR_GOTO("unable to set deinterlacer - no valid video window", pVideoWindow, ret, eRet_NotAvailable, error);

    ret = pVideoWindow->setDeinterlacer(bDeinterlacer);
    CHECK_ERROR_GOTO("unable to set Deinterlacer", ret, error);

    notifyObservers(eNotify_DeinterlacerChanged, &bDeinterlacer);

error:
    return(ret);
} /* setDeinterlacer */

eRet CDisplay::setBoxDetect(bool bBoxDetect)
{
    eRet           ret          = eRet_InvalidState;
    CVideoWindow * pVideoWindow = NULL;

    MListItr <CVideoWindow> itr(&_videoWindowList);

    for (pVideoWindow = itr.first(); pVideoWindow; pVideoWindow = itr.next())
    {
        if (NULL != pVideoWindow->getWindow())
        {
            ret = pVideoWindow->setBoxDetect(bBoxDetect);
            CHECK_ERROR_GOTO("unable to set box detect", ret, error);
        }
    }

    CHECK_ERROR_GOTO("unable to set box detect", ret, error);
    notifyObservers(eNotify_BoxDetectChanged, &bBoxDetect);

error:
    return(ret);
} /* setBoxDetect */

NEXUS_DisplayAspectRatio CDisplay::getAspectRatio()
{
    NEXUS_DisplaySettings settings;

    NEXUS_Display_GetSettings(getDisplay(), &settings);

    return(settings.aspectRatio);
}

eRet CDisplay::setAspectRatio(NEXUS_DisplayAspectRatio aspectRatio)
{
    eRet                  ret    = eRet_Ok;
    NEXUS_Error           nerror = NEXUS_SUCCESS;
    NEXUS_DisplaySettings settings;

    NEXUS_Display_GetSettings(getDisplay(), &settings);

    /*
     *  we could check for HD and default to 16x9, but for now we'll just
     *  do whatever the caller requests.
     *
     *  NEXUS_VideoFormatInfo     formatInfo;
     *  NEXUS_VideoFormat_GetInfo(settings.format, &formatInfo);
     *
     *  if ((formatInfo.height > 576) || (false == formatInfo.interlaced))
     *  {
     *      if ((settings.format != NEXUS_VideoFormat_e480p) ||
     *          (settings.format != NEXUS_VideoFormat_e576p))
     *      {
     *      }
     *  }
     */

    settings.aspectRatio = aspectRatio;
    nerror               = NEXUS_Display_SetSettings(getDisplay(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set aspect ratio", ret, nerror, error);

    notifyObservers(eNotify_AspectRatioChanged, &aspectRatio);

error:
    return(ret);
} /* setAspectRatio */

NEXUS_VideoFormat CDisplay::validateFormat(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormat formatValid      = format;
    uint16_t          vertResFormatMax = videoFormatToVertRes(getMaxFormat()).toInt();
    uint16_t          vertResFormat    = videoFormatToVertRes(format).toInt();

    if (720 >= vertResFormatMax)
    {
        /* this display's max format is SD, then modify given video format */
        MString strVideoFormat = videoFormatToString(format);
        formatValid = stringToVideoFormatSD(strVideoFormat);

        if (NEXUS_VideoFormat_ePal == formatValid)
        {
            formatValid = stringToVideoFormat(GET_STR(_pCfg, PREFERRED_PAL));
        }
        if (NEXUS_VideoFormat_eNtsc == formatValid)
        {
            formatValid = stringToVideoFormat(GET_STR(_pCfg, PREFERRED_NTSC));
        }
        BDBG_MSG(("Display%d validate format: use SD formats only - format:%s", getNumber(), videoFormatToString(formatValid).s()));
    }
    else
    if (vertResFormatMax < vertResFormat)
    {
        NEXUS_VideoFormat formatPreferred        = stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_HD));
        uint16_t          vertResFormatPreferred = videoFormatToVertRes(formatPreferred).toInt();

        if (vertResFormatMax >= vertResFormatPreferred)
        {
            /* use preferred HD format */
            formatValid = formatPreferred;
            BDBG_MSG(("Display%d validate format: use preferred format:%s", getNumber(), videoFormatToString(formatValid).s()));
        }
        else
        {
            /* default to max format */
            formatValid = getMaxFormat();
            BDBG_MSG(("Display%d validate format: use max display format:%s", getNumber(), videoFormatToString(formatValid).s()));
        }
    }

    return(formatValid);
} /* validateFormat */

eRet CDisplay::setFormat(
        NEXUS_VideoFormat format,
        CGraphics *       pGraphics,
        bool              bNotify
        )
{
    eRet                  ret         = eRet_Ok;
    NEXUS_Error           nerror      = NEXUS_SUCCESS;
    NEXUS_VideoFormat     formatOrig  = NEXUS_VideoFormat_eUnknown;
    NEXUS_VideoFormat     formatValid = format;
    NEXUS_DisplaySettings settings;

    BDBG_ASSERT(NEXUS_VideoFormat_eUnknown != format);
    BDBG_ASSERT(NEXUS_VideoFormat_eMax != format);
    BDBG_ASSERT(NULL != _pModel);
    BDBG_ASSERT(NULL != pGraphics);

    /* deactivate graphics for display format change */
    pGraphics->setActive(false);

    formatOrig  = getFormat();
    formatValid = validateFormat(format);

    if (getFormat() == formatValid)
    {
        /* no change needed */
        goto done;
    }

    /* set display preferred colorspace if available */
    ret = setPreferredColorSpace(formatValid);
    CHECK_ERROR("unable to set colorspace", ret);

    /* adjust macrovision setting based on video format */
    validateMacrovision(formatValid);

    /* set display format */
    BDBG_MSG(("set display %d format:%s(%d)", getNumber(), videoFormatToString(formatValid).s(), formatValid));
    NEXUS_Display_GetSettings(getDisplay(), &settings);
    settings.format = formatValid;
    nerror          = NEXUS_Display_SetSettings(getDisplay(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set video format", ret, nerror, error);

    /* adjust video windows based on new display format */
    ret = updateVideoWindowGeometry();
    CHECK_ERROR("unable to update video window geometry", ret);

    pGraphics->setFramebufferSize(this);
    pGraphics->setActive(true);

done:
    if (true == bNotify)
    {
        notifyObservers(eNotify_VideoFormatChanged, &formatValid);
    }
    goto finish;
error:
finish:
    pGraphics->setActive(true);
    return(ret);
} /* setFormat */

eRet CDisplay::updateVideoWindowGeometry()
{
    eRet           ret          = eRet_Ok;
    CVideoWindow * pVideoWindow = NULL;

    MListItr <CVideoWindow> itr(&_videoWindowList);
    MRect                   rectVideoFormat;

    BDBG_ASSERT(NULL != _pModel);

    /* get size of required video window - we will base main/pip on this size */
    rectVideoFormat.setX(0);
    rectVideoFormat.setY(0);
    rectVideoFormat.setWidth(videoFormatToHorizRes(getFormat()).toInt());
    rectVideoFormat.setHeight(videoFormatToVertRes(getFormat()).toInt());

    setUpdateMode(false);

    /* pip swap does not acually swap the underlying video windows.  the windows
     * are kept the same but the content that passes to each video window is swapped.
     * most of atlas operates on the assumption that the video windows actually
     * change, but this function, which resizes the actual video windows, must
     * be aware of the fact that the main/pip video windows do not change
     * position.  because of this, we will use winTypeFull and winTypePip to
     * represent the actual underlaying window types for main and pip. */
    eWindowType winTypeFull = _pModel->getFullScreenWindowType();
    eWindowType winTypePip  = _pModel->getPipScreenWindowType();
    if (true == _pModel->getPipSwapState())
    {
        winTypeFull = (eWindowType_Main == winTypeFull) ? eWindowType_Pip : eWindowType_Main;
        winTypePip  = (eWindowType_Main == winTypePip) ? eWindowType_Pip : eWindowType_Main;
    }

    for (pVideoWindow = _videoWindowList.first(); pVideoWindow && pVideoWindow->getWindow(); pVideoWindow = _videoWindowList.next())
    {
        if (winTypeFull == pVideoWindow->getType())
        {
            ret = pVideoWindow->setGeometry(rectVideoFormat);
            CHECK_ERROR_GOTO("unable to set video window geometry", ret, error);
        }
        else
        if (winTypePip == pVideoWindow->getType())
        {
            ret = pVideoWindow->setGeometry(rectVideoFormat,
                    GET_INT(_pCfg, PIP_PERCENTAGE),
                    (eWinArea)GET_INT(_pCfg, PIP_POSITION),
                    GET_INT(_pCfg, PIP_BORDER_PERCENTAGE),
                    1);
            CHECK_ERROR_GOTO("unable to set video window geometry", ret, error);
        }
        else
        {
            BDBG_WRN(("unable to set window geometry - invalid window type"));
        }
    }

error:
    setUpdateMode(true);
    return(ret);
} /* updateVideoWindowGeometry */

NEXUS_VideoFormat CDisplay::getFormat()
{
    NEXUS_DisplaySettings settings;

    BDBG_ASSERT(NULL != getDisplay());

    NEXUS_Display_GetSettings(getDisplay(), &settings);
    return(settings.format);
}

eRet CDisplay::setUpdateMode(bool bAuto)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = NEXUS_DisplayModule_SetUpdateMode(bAuto ? NEXUS_DisplayUpdateMode_eAuto : NEXUS_DisplayUpdateMode_eManual);
    CHECK_NEXUS_ERROR_GOTO("unable to change display update mode", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoFormat CDisplay::getMaxFormat(void)
{
    CPlatform * pPlatform = _pCfg->getPlatformConfig();

    return(pPlatform->getDisplayMaxVideoFormat(getNumber()));
}

uint16_t CDisplay::getNumVideoWindows()
{
    NEXUS_DisplayCapabilities capabilities;

    NEXUS_GetDisplayCapabilities(&capabilities);

    return(capabilities.display[getNumber()].numVideoWindows);
}

bool CDisplay::hasGraphics()
{
    NEXUS_DisplayCapabilities capabilities;

    NEXUS_GetDisplayCapabilities(&capabilities);

    return(((capabilities.display[getNumber()].graphics.width > 0) &&
            (capabilities.display[getNumber()].graphics.height > 0)) ? true : false);
}

bool CDisplay::isSupported()
{
#if !NXCLIENT_SUPPORT
    NEXUS_PlatformCapabilities capabilities;

    NEXUS_GetPlatformCapabilities(&capabilities);

    return(capabilities.display[getNumber()].supported);

#else /* if !NXCLIENT_SUPPORT */
    return(true);

#endif /* if !NXCLIENT_SUPPORT */
}

bool CDisplay::isReservedForEncoder()
{
#if (NEXUS_HAS_VIDEO_ENCODER && !NXCLIENT_SUPPORT)
    NEXUS_PlatformCapabilities capabilities;

    NEXUS_GetPlatformCapabilities(&capabilities);

    return(capabilities.display[getNumber()].encoder);

#else /* if (NEXUS_HAS_VIDEO_ENCODER && !NXCLIENT_SUPPORT) */
    return(false);

#endif /* if (NEXUS_HAS_VIDEO_ENCODER && !NXCLIENT_SUPPORT) */
}

bool CDisplay::isStandardDef()
{
    bool    bSD       = false;
    MString strFormat = videoFormatToString(getMaxFormat());

    if ((-1 < strFormat.find("NTSC", 0, false)) || (-1 < strFormat.find("PAL", 0, false)))
    {
        bSD = true;
    }

    return(bSD);
}