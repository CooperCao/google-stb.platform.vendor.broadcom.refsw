/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#include "video_window.h"
#include "display.h"
#include "nexus_video_decoder.h"
#include "nexus_video_adj.h"

BDBG_MODULE(atlas_video_window);

CVideoWindow::CVideoWindow(
        const uint16_t   number,
        CDisplay *       pDisplay,
        CConfiguration * pCfg
        ) :
    _number(number),
    _videoWindow(NULL),
    _pDisplay(pDisplay),
    _pCfg(pCfg),
    _checkedOut(false),
    _type(eWindowType_Main),

    _position(0, 0, GET_INT(pCfg, VIDEO_WINDOW_WIDTH), GET_INT(pCfg, VIDEO_WINDOW_HEIGHT)),
    _zOrder(number),
    _deinterlacer(0 == number && 0 == pDisplay->getNumber()), /* only for 1st window on 1st display */
    _playbackDeinterlacer(false),
    _sharpness(0) /* DTT what should this default be? */
{
}

CVideoWindow::~CVideoWindow()
{
    close();
}

eRet CVideoWindow::open()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != _pDisplay);

    _videoWindow = NEXUS_VideoWindow_Open(_pDisplay->getDisplay(), _number);
    CHECK_PTR_ERROR_GOTO("Nexus video window open failed", _videoWindow, ret, eRet_ExternalError, error);

    {
        NEXUS_VideoWindowSettings settings;
        NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
        settings.contentMode = NEXUS_VideoWindowContentMode_eBox;
        nerror               = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set content mode", ret, nerror, error);
    }

error:
    return(ret);
} /* open */

void CVideoWindow::close()
{
    if (NULL != _videoWindow)
    {
        NEXUS_VideoWindow_RemoveAllInputs(_videoWindow);
        _videoDecodeList.clear();
        NEXUS_VideoWindow_Close(_videoWindow);
        _videoWindow = NULL;
    }
}

eRet CVideoWindow::addInput(CVideoDecode * pVideoDecode)
{
    eRet        ret    = eRet_InvalidParameter;  /* assume error */
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(pVideoDecode);
    BDBG_ASSERT(pVideoDecode->getDecoder());

    nerror = NEXUS_VideoWindow_AddInput(_videoWindow, NEXUS_VideoDecoder_GetConnector(pVideoDecode->getDecoder()));
    CHECK_NEXUS_ERROR_GOTO("Adding decode to video window failed", ret, nerror, error);
    _videoDecodeList.add(pVideoDecode);
    ret = eRet_Ok;

error:
    return(ret);
}

eRet CVideoWindow::setContentMode(NEXUS_VideoWindowContentMode contentMode)
{
    NEXUS_VideoWindowSettings settings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    settings.contentMode = contentMode;
    nerror               = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set content mode", ret, nerror, error);

error:
    return(ret);
}

NEXUS_VideoWindowContentMode CVideoWindow::getContentMode()
{
    NEXUS_VideoWindowSettings settings;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    return(settings.contentMode);
}

eRet CVideoWindow::setDeinterlacer(bool bDeinterlacer)
{
    NEXUS_VideoWindowMadSettings settings;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetMadSettings(getWindow(), &settings);
    settings.deinterlace = bDeinterlacer;
    nerror               = NEXUS_VideoWindow_SetMadSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set deinterlacer", ret, nerror, error);

error:
    return(ret);
}

eRet CVideoWindow::setBoxDetect(bool bBoxDetect)
{
    NEXUS_VideoWindowSettings settings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    settings.letterBoxDetect  = bBoxDetect;
    settings.letterBoxAutoCut = true;
    nerror                    = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set box detect", ret, nerror, error);

error:
    return(ret);
}

eRet CVideoWindow::setPosition(
        MRect    rect,
        uint16_t zorder
        )
{
    NEXUS_VideoWindowSettings settings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    settings.position.x      = rect.x();
    settings.position.y      = rect.y();
    settings.position.width  = rect.width();
    settings.position.height = rect.height();
    settings.zorder          = zorder;
    nerror                   = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set position", ret, nerror, error);

error:
    return(ret);
} /* setPosition */

void CVideoWindow::getPosition(
        MRect *    pRect,
        uint16_t * pZorder
        )
{
    NEXUS_VideoWindowSettings settings;

    BDBG_ASSERT(NULL != pRect);
    BDBG_ASSERT(NULL != pZorder);

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    pRect->setX(settings.position.x);
    pRect->setY(settings.position.y);
    pRect->setWidth(settings.position.width);
    pRect->setHeight(settings.position.height);
    *pZorder = settings.zorder;
} /* getPosition */

eRet CVideoWindow::setZorder(uint16_t zorder)
{
    NEXUS_VideoWindowSettings settings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    settings.zorder = zorder;
    nerror          = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set zorder", ret, nerror, error);

error:
    return(ret);
}

eRet CVideoWindow::setVisible(bool bVisible)
{
    NEXUS_VideoWindowSettings settings;
    eRet                      ret    = eRet_Ok;
    NEXUS_Error               nerror = NEXUS_SUCCESS;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    settings.visible = bVisible;
    nerror           = NEXUS_VideoWindow_SetSettings(getWindow(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set visibiliy", ret, nerror, error);

error:
    return(ret);
}

bool CVideoWindow::isVisible()
{
    NEXUS_VideoWindowSettings settings;

    NEXUS_VideoWindow_GetSettings(getWindow(), &settings);
    return(settings.visible);
}

eRet CVideoWindow::setGeometry(
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
    BDBG_ASSERT(eWindowType_Max != getType());

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
} /* setGeometry */