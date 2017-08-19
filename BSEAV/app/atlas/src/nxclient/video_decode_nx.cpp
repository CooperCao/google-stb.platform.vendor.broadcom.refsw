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

#include "graphics_nx.h"
#include "video_decode_nx.h"
#include "model.h"
#include "nxclient.h"

#define CALLBACK_VDECODE_SOURCE_CHANGED  "callbackVDecodeSourceChanged"
#define CALLBACK_VDECODE_STREAM_CHANGED  "callbackVDecodeStreamChanged"

BDBG_MODULE(atlas_video_decode);

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinSimpleVideoDecodeSourceChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    pSimpleVideoDecode->videoDecodeSourceChangedCallback();
} /* bwinSimpleVideoDecodeSourceChangedCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinSimpleVideoDecodeStreamChangedCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CSimpleVideoDecode * pSimpleVideoDecode = (CSimpleVideoDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pSimpleVideoDecode);

    pSimpleVideoDecode->videoDecodeStreamChangedCallback();
} /* bwinSimpleVideoDecodeStreamChangedCallback */

/* nexus callback that is executed when the video source has changed */
static void NexusVideoDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CSimpleVideoDecode * pVideoDecode = (CSimpleVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pVideoDecode);

    CWidgetEngine * pWidgetEngine = pVideoDecode->getWidgetEngine();

    pVideoDecode->setSourceChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pVideoDecode, CALLBACK_VDECODE_SOURCE_CHANGED);
    }
} /* NexusVideoDecodeSourceChangedCallback */

/* nexus callback that is executed when the video stream has changed */
static void NexusVideoDecodeStreamChangedCallback(
        void * context,
        int    param
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pVideoDecode);

    CWidgetEngine * pWidgetEngine = pVideoDecode->getWidgetEngine();

    pVideoDecode->setStreamChanged(true);

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pVideoDecode, CALLBACK_VDECODE_STREAM_CHANGED);
    }
} /* NexusVideoDecodeStreamChangedCallback */

CSimpleVideoDecodeNx::CSimpleVideoDecodeNx(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CSimpleVideoDecode(name, number, pCfg),
    _surfaceClientVideoWin(NULL),
    _videoWindowType(NxClient_VideoWindowType_eMain)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(eRet_Ok == ret);
}

CSimpleVideoDecodeNx::~CSimpleVideoDecodeNx()
{
    close();
}

eRet CSimpleVideoDecodeNx::open(
        CWidgetEngine * pWidgetEngine,
        CStc *          pStc
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pStc);
    BDBG_ASSERT(NULL != _pModel);

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Simple Video decoder is already opened.", ret, error);
    }

    if (NULL == _pBoardResources)
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Set board resources before opening.", ret, error);
    }

    if (NULL != pWidgetEngine)
    {
        _pWidgetEngine = pWidgetEngine;
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED, bwinSimpleVideoDecodeSourceChangedCallback);
        _pWidgetEngine->addCallback(this, CALLBACK_VDECODE_STREAM_CHANGED, bwinSimpleVideoDecodeStreamChangedCallback);
    }
    else
    {
        BDBG_MSG(("Opening CSimpleVideoDecode num:%d without providing widgetEngine - decoder callbacks will be ignored", getNumber()));
    }

    _simpleDecoder = NEXUS_SimpleVideoDecoder_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire simple video decoder", _simpleDecoder, ret, eRet_NotAvailable, error);

    {
        CGraphicsNx *      pGraphics      = (CGraphicsNx *)_pModel->getGraphics();
        CSurfaceClientNx * pSurfaceClient = NULL;

        if (NULL != pGraphics)
        {
            pSurfaceClient = (CSurfaceClientNx *)pGraphics->getSurfaceClientDesktop();
            _surfaceClientVideoWin = NEXUS_SurfaceClient_AcquireVideoWindow(pSurfaceClient->getSurfaceClient(), getWindowType());
        }

        CHECK_PTR_ERROR_GOTO("unable to acquire video window", _surfaceClientVideoWin, ret, eRet_NotAvailable, error);
    }

    ret = setStc(pStc);
    CHECK_ERROR_GOTO("SetStcChannel simple video decoder failed!", ret, error);

    /* set video decoder settings */
    {
        NEXUS_Error                nError = NEXUS_SUCCESS;
        NEXUS_VideoDecoderSettings videoDecoderSettings;

        NEXUS_SimpleVideoDecoder_GetSettings(_simpleDecoder, &videoDecoderSettings);
        videoDecoderSettings.sourceChanged.callback = NexusVideoDecodeSourceChangedCallback;
        videoDecoderSettings.sourceChanged.context  = this;
        videoDecoderSettings.sourceChanged.param    = 0;
        videoDecoderSettings.streamChanged.callback = NexusVideoDecodeStreamChangedCallback;
        videoDecoderSettings.streamChanged.context  = this;
        videoDecoderSettings.streamChanged.param    = 0;
        videoDecoderSettings.dropFieldMode          = GET_BOOL(_pCfg, DROP_FIELD_MODE_ENABLED);

        if ((0 < _maxWidth) && (0 < _maxHeight))
        {
            videoDecoderSettings.maxWidth  = _maxWidth;
            videoDecoderSettings.maxHeight = _maxHeight;
        }
        nError = NEXUS_SimpleVideoDecoder_SetSettings(_simpleDecoder, &videoDecoderSettings);
        CHECK_NEXUS_ERROR_GOTO("Error applying simple video decoder settings.", ret, nError, error);
    }

error:
    return(ret);
} /* open */

CStc * CSimpleVideoDecodeNx::close()
{
    CStc * pStc = _pStc;

    if (NULL != _simpleDecoder)
    {
        /* Disconnect the STC */
        setStc(NULL);

        NEXUS_SimpleVideoDecoder_Release(_simpleDecoder);
        _simpleDecoder = NULL;
    }

    if (NULL != _surfaceClientVideoWin)
    {
        NEXUS_SurfaceClient_ReleaseVideoWindow(_surfaceClientVideoWin);
        _surfaceClientVideoWin = NULL;
    }
    _videoWindowList.clear();

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_STREAM_CHANGED);
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

eRet CSimpleVideoDecodeNx::setPosition(
        MRect    rect,
        unsigned zorder
        )
{
    NEXUS_SurfaceClientSettings settings;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    NEXUS_SurfaceClient_GetSettings(_surfaceClientVideoWin, &settings);

    /* only update settings if different from requested geometry */
    if ((settings.composition.position.x != (int)rect.x()) ||
        (settings.composition.position.y != (int)rect.y()) ||
        (settings.composition.position.width != rect.width()) ||
        (settings.composition.position.height != rect.height())
       )
    {
        settings.composition.position.x      = (int)rect.x();
        settings.composition.position.y      = (int)rect.y();
        settings.composition.position.width  = rect.width();
        settings.composition.position.height = rect.height();
        settings.composition.zorder          = zorder;
        nerror                               = NEXUS_SurfaceClient_SetSettings(_surfaceClientVideoWin, &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set position", ret, nerror, error);
    }

error:
    return(ret);
} /* setPosition */

/* percent, border, and *pRectPercent are expressed in terms: 0-1000 = 0-100.0% */
eRet CSimpleVideoDecodeNx::setGeometryVideoWindow(
        MRect    rect,
        unsigned  percent,
        eWinArea area,
        unsigned  border,
        unsigned zorder,
        MRect *  pRectPercent
        )
{
    eRet     ret       = eRet_Ok;
    unsigned borderGap = 0;
    MRect    rectScaled;

    BDBG_ASSERT(1000 >= percent);

    borderGap = border * rect.height() / 1000;

    rectScaled.setWidth(rect.width() * percent / 1000);
    rectScaled.setHeight(rect.height() * percent / 1000);

    if (pRectPercent)
    {
        pRectPercent->setWidth(percent);
        pRectPercent->setHeight(percent);
    }

    switch (area)
    {
    case eWinArea_LowerLeft:
        rectScaled.setX(borderGap);
        rectScaled.setY(rect.height() - borderGap - rectScaled.height());

        if (pRectPercent)
        {
            pRectPercent->setX(border);
            pRectPercent->setY(rectScaled.y() * 1000 / rect.height());
        }
        break;

    case eWinArea_LowerRight:
        rectScaled.setX(rect.width() - borderGap - rectScaled.width());
        rectScaled.setY(rect.height() - borderGap - rectScaled.height());

        if (pRectPercent)
        {
            pRectPercent->setX(rectScaled.x() * 1000 / rect.width());
            pRectPercent->setY(rectScaled.y() * 1000 / rect.height());
        }
        break;

    case eWinArea_UpperLeft:
        rectScaled.setX(borderGap);
        rectScaled.setY(borderGap);

        if (pRectPercent)
        {
            pRectPercent->setX(border);
            pRectPercent->setY(border);
        }
        break;

    case eWinArea_UpperRight:
    default:
        rectScaled.setX(rect.width() - borderGap - rectScaled.width());
        rectScaled.setY(borderGap);

        if (pRectPercent)
        {
            pRectPercent->setX(rectScaled.x() * 1000 / rect.width());
            pRectPercent->setY(border);
        }
        break;
    } /* switch */

    BDBG_MSG(("Set video window type:%d geometry x:%d y:%d w:%d h:%d", getType(), rectScaled.x(), rectScaled.y(), rectScaled.width(), rectScaled.height()));
    ret = setPosition(rectScaled, zorder);

    return(ret);
} /* setGeometryVideoWindow */

NEXUS_VideoFormat CSimpleVideoDecodeNx::getFormat()
{
    NxClient_DisplaySettings settings;

    NxClient_GetDisplaySettings(&settings);

    return(settings.format);
}

/* given rect values are in percent where 0-1000 == 0%-100.0%.
   if rectGeom.isNull() is true, then default video window sizes will be used */
eRet CSimpleVideoDecodeNx::setVideoWindowGeometryPercent(MRect * pRectGeomPercent)
{
    eRet          ret       = eRet_Ok;
    NEXUS_Error   nerror    = NEXUS_SUCCESS;
    CGraphicsNx * pGraphics = (CGraphicsNx *)_pModel->getGraphics();
    MRect         rectVideoFormat;

    BDBG_ASSERT(NULL != _surfaceClientVideoWin);
    BDBG_ASSERT(NULL != pGraphics);
    BDBG_ASSERT(NULL != pRectGeomPercent);

    {
        NEXUS_SurfaceComposition settingsSurfaceClient;
        NxClient_GetSurfaceClientComposition(
                pGraphics->getSurfaceClientDesktop()->getNumber(),
                &settingsSurfaceClient);

        /* get size of required video window - we will base main/pip on this size */
        rectVideoFormat.setX(settingsSurfaceClient.position.x);
        rectVideoFormat.setY(settingsSurfaceClient.position.y);
        rectVideoFormat.setWidth(settingsSurfaceClient.position.width);
        rectVideoFormat.setHeight(settingsSurfaceClient.position.height);
    }

    /* disable close captioning routing for mosaic decodes */
    if ((eWindowType_Mosaic1 <= getWindowType()) && (eWindowType_Max > getWindowType()))
    {
        NEXUS_SimpleVideoDecoderClientSettings videoDecoderClientSettings;

        NEXUS_SimpleVideoDecoder_GetClientSettings(getSimpleDecoder(), &videoDecoderClientSettings);
        videoDecoderClientSettings.closedCaptionRouting = false;
        nerror = NEXUS_SimpleVideoDecoder_SetClientSettings(getSimpleDecoder(), &videoDecoderClientSettings);
        CHECK_NEXUS_ERROR("unable to disable closed caption routing", nerror);
    }

    if (true == pRectGeomPercent->isNull())
    {
        /* invalid geometry given so use some default video */

        if (eWindowType_Mosaic1 == getWindowType())
        {
            BDBG_MSG(("---------------------------------- resizing mosaic1 video window"));
            ret = setGeometryVideoWindow(rectVideoFormat, 500, eWinArea_UpperLeft, 0, 1, pRectGeomPercent);
            *pRectGeomPercent = MRect(0, 0, 500, 500);
        }
        else
        if (eWindowType_Mosaic2 == getWindowType())
        {
            BDBG_MSG(("---------------------------------- resizing mosaic2 video window"));
            ret = setGeometryVideoWindow(rectVideoFormat, 500, eWinArea_UpperRight, 0, 1, pRectGeomPercent);
            *pRectGeomPercent = MRect(500, 0, 500, 500);
        }
        else
        if (eWindowType_Mosaic3 == getWindowType())
        {
            BDBG_MSG(("---------------------------------- resizing mosaic3 video window"));
            ret = setGeometryVideoWindow(rectVideoFormat, 500, eWinArea_LowerLeft, 0, 1, pRectGeomPercent);
            *pRectGeomPercent = MRect(0, 500, 500, 500);
        }
        else
        if (eWindowType_Mosaic4 == getWindowType())
        {
            BDBG_MSG(("---------------------------------- resizing mosaic4 video window"));
            ret = setGeometryVideoWindow(rectVideoFormat, 500, eWinArea_LowerRight, 0, 1, pRectGeomPercent);
            *pRectGeomPercent = MRect(500, 500, 500, 500);
        }
        else
        if (getWindowType() != _pModel->getFullScreenWindowType())
        {
            BDBG_MSG(("---------------------------------- resizing decimate video window"));
            ret = setGeometryVideoWindow(
                    rectVideoFormat,
                    GET_INT(_pCfg, PIP_PERCENTAGE),
                    (eWinArea)GET_INT(_pCfg, PIP_POSITION),
                    GET_INT(_pCfg, PIP_BORDER_PERCENTAGE),
                    1,
                    pRectGeomPercent);
            CHECK_ERROR_GOTO("unable to set default video window geometry", ret, error);
        }
        else
        {
            BDBG_MSG(("---------------------------------- resizing full screen window x:%d y:%d w:%d h:%d",
                      rectVideoFormat.x(), rectVideoFormat.y(), rectVideoFormat.width(), rectVideoFormat.height()));
            setPosition(rectVideoFormat, 0);
            *pRectGeomPercent = MRect(0, 0, 1000, 1000);
        }
    }
    else
    {
        /* valid geometry given so use it */
        MRect rectGeomAbsolute = SCALE_RECT_PERCENT(rectVideoFormat, (*pRectGeomPercent));

        BDBG_MSG(("---------------------------------- resizing full screen window x:%d y:%d w:%d h:%d",
                  rectVideoFormat.x(), rectVideoFormat.y(), rectVideoFormat.width(), rectVideoFormat.height()));
        BDBG_MSG(("---------------------------------- resizing window x:%d y:%d w:%d h:%d",
                  rectGeomAbsolute.x(), rectGeomAbsolute.y(), rectGeomAbsolute.width(), rectGeomAbsolute.height()));

        BDBG_MSG(("Set video window type:%d geometry x:%d y:%d w:%d h:%d", getType(), rectGeomAbsolute.x(), rectGeomAbsolute.y(), rectGeomAbsolute.width(), rectGeomAbsolute.height()));
        ret = setPosition(rectGeomAbsolute, 1);
        CHECK_ERROR_GOTO("unable to set video window geometry", ret, error);
    }


error:
    return(ret);
}

eRet CSimpleVideoDecodeNx::updateConnectSettings(
        NxClient_ConnectSettings * pSettings,
        int                        index
        )
{
    eRet          ret       = eRet_Ok;
    CGraphicsNx * pGraphics = (CGraphicsNx *)_pModel->getGraphics();

    pSettings->simpleVideoDecoder[index].id = getNumber();

    if (eWindowType_Pip == _pModel->getFullScreenWindowType())
    {
        /* pip is fullscreen (swap occurred) */
        pSettings->simpleVideoDecoder[index].windowCapabilities.type = /* NxClient_VideoWindowType_eMain; */
                                                                       (eWindowType_Pip == getWindowType()) ? NxClient_VideoWindowType_eMain : NxClient_VideoWindowType_ePip;
    }
    else
    {
        /* main is fullscreen (no swap) */
        pSettings->simpleVideoDecoder[index].windowCapabilities.type =
            (eWindowType_Pip == getWindowType()) ? NxClient_VideoWindowType_ePip : NxClient_VideoWindowType_eMain;
    }

    if (NULL != pGraphics)
    {
        pSettings->simpleVideoDecoder[index].surfaceClientId = pGraphics->getSurfaceClientDesktop()->getNumber();
        pSettings->simpleVideoDecoder[index].windowId        = getWindowType(); /* (eWindowType_Main == getVideoWindowType()) ? 0 : 1; */
    }

    if ((0 < _maxWidth) && (0 < _maxHeight))
    {
        pSettings->simpleVideoDecoder[index].decoderCapabilities.maxWidth  = _maxWidth;
        pSettings->simpleVideoDecoder[index].decoderCapabilities.maxHeight = _maxHeight;
        /* TTTTTTTTTTTTTTTT TODO: support 10bit color depth
         * pSettings->simpleVideoDecoder[index].decoderCapabilities.colorDepth =
         */
    }

    return(ret);
} /* updateConnectSettings */
