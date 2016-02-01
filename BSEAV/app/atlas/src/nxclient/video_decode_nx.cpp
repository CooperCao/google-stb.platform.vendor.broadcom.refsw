/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "graphics_nx.h"
#include "video_decode_nx.h"
#include "model.h"
#include "nxclient.h"

#define CALLBACK_VDECODE_SOURCE_CHANGED  "callbackVDecodeSourceChanged"

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

    pSimpleVideoDecode->videoDecodeCallback();
} /* bwinSimpleVideoDecodeSourceChangedCallback */

/* nexus callback that is executed when the video source has changed */
static void NexusVideoDecodeSourceChangedCallback(
        void * context,
        int    param
        )
{
    CVideoDecode * pVideoDecode = (CVideoDecode *)context;

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

CSimpleVideoDecodeNx::CSimpleVideoDecodeNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CSimpleVideoDecode(name, number, pCfg),
    _connectId(0),
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
        }

        _surfaceClientVideoWin = NEXUS_SurfaceClient_AcquireVideoWindow(pSurfaceClient->getSurfaceClient(), getNumber());
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
        _pWidgetEngine->removeCallback(this, CALLBACK_VDECODE_SOURCE_CHANGED);
        _pWidgetEngine = NULL;
    }

    return(pStc);
} /* close */

eRet CSimpleVideoDecodeNx::setPosition(
        MRect    rect,
        uint16_t zorder
        )
{
    NEXUS_SurfaceClientSettings settings;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    NEXUS_SurfaceClient_GetSettings(_surfaceClientVideoWin, &settings);
    settings.composition.position.x      = (int16_t)rect.x();
    settings.composition.position.y      = (int16_t)rect.y();
    settings.composition.position.width  = rect.width();
    settings.composition.position.height = rect.height();
    settings.composition.zorder          = zorder;
    nerror                               = NEXUS_SurfaceClient_SetSettings(_surfaceClientVideoWin, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set position", ret, nerror, error);

error:
    return(ret);
} /* setPosition */

eRet CSimpleVideoDecodeNx::setGeometryVideoWindow(
        MRect    rect,
        uint8_t  percent,
        eWinArea area,
        uint8_t  border,
        uint16_t zorder
        )
{
    eRet     ret       = eRet_Ok;
    uint16_t borderGap = 0;
    MRect    rectScaled;

    BDBG_ASSERT(100 >= percent);

    borderGap = border * rect.height() / 100;

    rectScaled.setWidth(rect.width() * percent / 100);
    rectScaled.setHeight(rect.height() * percent / 100);

    switch (area)
    {
    case eWinArea_LowerLeft:
        rectScaled.setX(borderGap);
        rectScaled.setY(rect.height() - borderGap - rectScaled.height());
        break;

    case eWinArea_LowerRight:
        rectScaled.setX(rect.width() - borderGap - rectScaled.width());
        rectScaled.setY(rect.height() - borderGap - rectScaled.height());
        break;

    case eWinArea_UpperLeft:
        rectScaled.setX(borderGap);
        rectScaled.setY(borderGap);
        break;

    case eWinArea_UpperRight:
    default:
        rectScaled.setX(rect.width() - borderGap - rectScaled.width());
        rectScaled.setY(borderGap);
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

eRet CSimpleVideoDecodeNx::start(
        CPid * pPid,
        CStc * pStc
        )
{
    CGraphicsNx *            pGraphics = (CGraphicsNx *)_pModel->getGraphics();
    eRet                     ret       = eRet_Ok;
    NEXUS_Error              nerror    = NEXUS_SUCCESS;
    NxClient_ConnectSettings settings;

    if (0 != _connectId)
    {
        BDBG_ERR(("video decoder connect id is invalid - possibly trying to connect more than once?!"));
    }

    NxClient_GetDefaultConnectSettings(&settings);
    settings.simpleVideoDecoder[0].id                      = getNumber();
    settings.simpleVideoDecoder[0].windowCapabilities.type = getVideoWindowType();

    if ((videoFormatToHorizRes(NEXUS_VideoFormat_e1080p).toInt() < pPid->getWidth()) ||
        (videoFormatToVertRes(NEXUS_VideoFormat_e1080p).toInt() < pPid->getHeight()))
    {
        /* set max width/height based on pPid width/height if bigger than 1080p */
        _maxWidth  = pPid->getWidth();
        _maxHeight = pPid->getHeight();
    }
    else
    if (NEXUS_VideoCodec_eH265 == pPid->getVideoCodec())
    {
        /* default to 4k max size for HEVC video codec */
        _maxWidth  = videoFormatToHorizRes(NEXUS_VideoFormat_e3840x2160p24hz).toInt();
        _maxHeight = videoFormatToVertRes(NEXUS_VideoFormat_e3840x2160p24hz).toInt();
    }

    if (NULL != pGraphics)
    {
        settings.simpleVideoDecoder[0].surfaceClientId               = pGraphics->getSurfaceClientDesktop()->getNumber();
        settings.simpleVideoDecoder[0].windowId                      = getNumber();
        settings.simpleVideoDecoder[0].decoderCapabilities.maxWidth  = _maxWidth;
        settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = _maxHeight;
        /* TTTTTTTTTTTTTTTT TODO: support 10bit color depth
         * settings.simpleVideoDecoder[0].decoderCapabilities.colorDepth =
         */
    }

    nerror = NxClient_Connect(&settings, &_connectId);
    CHECK_NEXUS_ERROR_GOTO("unable to connect to simple video decoder", nerror, ret, error);

    if ((NxClient_VideoWindowType_ePip == getVideoWindowType()) && (NULL != _surfaceClientVideoWin))
    {
        MRect rectVideoFormat;

        /* get size of required video window - we will base main/pip on this size */
        rectVideoFormat.setX(0);
        rectVideoFormat.setY(0);
        rectVideoFormat.setWidth(videoFormatToHorizRes(getFormat()).toInt());
        rectVideoFormat.setHeight(videoFormatToVertRes(getFormat()).toInt());

        ret = setGeometryVideoWindow(
                rectVideoFormat,
                GET_INT(_pCfg, PIP_PERCENTAGE),
                (eWinArea)GET_INT(_pCfg, PIP_POSITION),
                GET_INT(_pCfg, PIP_BORDER_PERCENTAGE),
                1);
        CHECK_ERROR_GOTO("unable to set video window geometry", ret, error);
    }

    return(CSimpleVideoDecode::start(pPid, pStc));

error:
    return(ret);
} /* start */

/* stop */

CPid * CSimpleVideoDecodeNx::stop()
{
    if (0 < _connectId)
    {
        NxClient_Disconnect(_connectId);
        _connectId = 0;
    }
    else
    {
        BDBG_ERR(("unable to disconnect video decoder!"));
    }

    return(CSimpleVideoDecode::stop());
} /* stop */