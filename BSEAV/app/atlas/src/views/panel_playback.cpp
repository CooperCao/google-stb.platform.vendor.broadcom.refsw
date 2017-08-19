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

#include "convert.h"
#include "still_decode.h"
#include "panel_playback.h"
#include "channel.h"
#include "thumb.h"
#include "time.h"

BDBG_MODULE(panel_playback);

#define MAX_PROPS          20
#define MENU_WIDTH_VIDEOS  250

#define MENU_WIDTH_PROPS   300
#define PROP_PERCENTAGE    25

CPanelPlayback::CPanelPlayback(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelPlayback", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pPlaybackMenu(NULL),
    _pVideosMenu(NULL),
    _pPropertiesMenu(NULL),
    _pThumbnailLabel(NULL),
    _maxThumbnailSize(0, 0, 0, 0),
    _thumbnailFramebuffer(NULL),
    _surfaceThumb(NULL),
    _bExpandPanel(false),
    _pExpand(NULL),
    _pThumb(NULL),
    _Back(NULL),
    _pList(NULL),
    _filePropsTimer((CObserver *)this),
    _pVideoButtonFocus(NULL)
{
    _filePropsTimer.setWidgetEngine(pWidgetEngine);
    _filePropsTimer.setTimeout(2000);
}

CPanelPlayback::~CPanelPlayback()
{
    uninitialize();
}

eRet CPanelPlayback::addPlaybackProp(
        const char * strName,
        bwin_font_t  font
        )
{
    uint32_t        colorText       = COLOR_EGGSHELL;
    uint32_t        colorBackground = 0xCC222222;
    CPlaybackProp * pProp           = NULL;
    eRet            ret             = eRet_Ok;

    pProp = new CPlaybackProp(this, _pPropertiesMenu, strName, font, PROP_PERCENTAGE, fill_eSolid);
    CHECK_PTR_ERROR_GOTO("unable to allocate playback property", pProp, ret, eRet_OutOfMemory, error);
    pProp->setTag(MString(strName) + ":");
    pProp->setValue("");
    pProp->setTextColor(colorText);
    pProp->setBackgroundColor(colorBackground);
    _propList.add(pProp);

error:
    return(ret);
} /* addPlaybackProp */

/* if you need to process any incomming notifications (processNotification()) we will need to do a similar
 * thing and put a processNotificationPlaybackMenu() method in here. */

/* returns eRet_Ok if everything gets created...eRet_OutOfMemory otherwise */
eRet CPanelPlayback::initialize(
        CModel *  pModel,
        CConfig * pConfig
        )
{
    eRet           ret            = eRet_Ok;
    CGraphics *    pGraphics      = NULL;
    CStillDecode * pStillDecode   = NULL;
    bwin_font_t    font10         = NULL;
    bwin_font_t    font12         = NULL;
    bwin_font_t    font14         = NULL;
    uint32_t       graphicsWidth  = 0;
    uint32_t       graphicsHeight = 0;
    MRect          rectPanel;

    BDBG_ASSERT(NULL != pModel);
    BDBG_ASSERT(NULL != pConfig);

    setModel(pModel);
    setConfig(pConfig);

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    font10 = pGraphics->getFont(10);
    BDBG_ASSERT(NULL != font10);
    font12 = pGraphics->getFont(12);
    BDBG_ASSERT(NULL != font12);
    font14 = pGraphics->getFont(14);
    BDBG_ASSERT(NULL != font14);

    pStillDecode = _pModel->getStillDecode();

    graphicsWidth  = GET_INT(_pCfg, GRAPHICS_SURFACE_WIDTH);
    graphicsHeight = GET_INT(_pCfg, GRAPHICS_SURFACE_HEIGHT);

    /* set the size of the panel itself */
    rectPanel.set(50, 50, MENU_WIDTH_VIDEOS, 0);
    setGeometry(rectPanel);

    /* CWidgetMenu's consist of the menu bar, the scrolling list view, and the other stuff that
     * makes it look like a window.   It will internally handle all the scrolling.  you must create buttons
     * outside of the CWidgetMenu and then call addButton() to add it to the widget.  because you create
     * the buttons outside of the CWidgetMenu, we will set them up so when clicked, the callback comes
     * to the CPanelPlayback and not to the CWidgetMenu itself. */
    _pPlaybackMenu = new CWidgetMenu("CPanelPlayback::_pPlaybackMenu", getEngine(), this, MRect(0, 0, MENU_WIDTH_VIDEOS, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPlaybackMenu, ret, eRet_OutOfMemory, error);
    _pPlaybackMenu->showSubTitle(false);
    _pPlaybackMenu->showListView(false);
    _pPlaybackMenu->showEdit(false);
    _pPlaybackMenu->setMenuTitle("Playback", NULL);
    _pPlaybackMenu->show(true);
    {
        _pVideosMenu = new CWidgetMenu("CPanelPlayback::_pVideosMenu", getEngine(), _pPlaybackMenu, MRect(0, 0, MENU_WIDTH_VIDEOS, 0), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pVideosMenu, ret, eRet_OutOfMemory, error);
        _pVideosMenu->showMenuBar(false);
        _pVideosMenu->showEdit(false);
        _pVideosMenu->setMenuTitle(NULL, "Videos");
        _pVideosMenu->setScroll(true);
    }

    if (NULL != _pList)
    {
        refreshPlaybackButtonList();
    }

    /* playback item thumbnail in expansion panel */
    _pThumbnailLabel = new CWidgetLabel("CPanelPlayback::_pThumbnailLabel", getEngine(), _pPlaybackMenu, MRect(0, 0, 0, 0), font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pThumbnailLabel, ret, eRet_OutOfMemory, error);
    _pThumbnailLabel->setBackgroundColor(0xFF222222);
    _pThumbnailLabel->setTextColor(COLOR_EGGSHELL);
    _pThumbnailLabel->setBevel(0);
    _pThumbnailLabel->loadImage("images/brcm_chip.png", bwin_image_render_mode_single);

    /* playback item details in expansion panel */
    _pPropertiesMenu = new CWidgetMenu("CPanelPlayback::_pPropertiesMenu", getEngine(), _pPlaybackMenu, MRect(0, 0, MENU_WIDTH_PROPS, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPropertiesMenu, ret, eRet_OutOfMemory, error);
    _pPropertiesMenu->showMenuBar(false);
    _pPropertiesMenu->showEdit(false);
    _pPropertiesMenu->setMenuTitle("", "Properties");
    _pPropertiesMenu->setScroll(false);
    _pPropertiesMenu->setListViewBackgroundColor(0xFF222222);
    _pPropertiesMenu->setListViewTextColor(COLOR_EGGSHELL);
    _pPropertiesMenu->setListViewBevel(0);

    /* add properties labels */
    {
        ret = addPlaybackProp("Filename", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Description", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Date", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Ratings", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Size", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Resolution", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Video Codec", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Video PID", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Audio Codec", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
        ret = addPlaybackProp("Audio PID", font10);
        CHECK_ERROR_GOTO("unable to create playback property", ret, error);
    }

    /* expand button */
    _pExpand = new CWidgetButton("CPanelPlayback::_pExpand", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pExpand, ret, eRet_OutOfMemory, error);
    _pPlaybackMenu->addExpandButton(_pExpand);
    /* default expand panel open if thumbnail decode is available */
    _bExpandPanel = (NULL != pStillDecode) ? true : false;

    /* back button */
    /* the back button is the button in the upper left area of the CWidgetMenu.  similar to
     * the other menu buttons, it is created here and passed in using addBackButton().  it behaves
     * similarly where click callbacks come to this CPanelPlayback. */
    _Back = new CWidgetButton("CPanelPlayback::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
    _Back->setText("Menu");
    _pPlaybackMenu->addBackButton(_Back);

    _Back->setFocus();

    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelPlayback::uninitialize()
{
    CGraphics * pGraphics = _pModel->getGraphics();

    BDBG_ASSERT(NULL != pGraphics);

    /* remove buttons from menu and free widgets  */
    _pVideosMenu->clearButtons();
    _Buttons.clear();
    _propList.clear();

    DEL(_Back);
    DEL(_pExpand);
    DEL(_pPropertiesMenu);
    if (NULL != _thumbnailFramebuffer)
    {
        pGraphics->destroyBwinFramebuffer(_thumbnailFramebuffer);
        _thumbnailFramebuffer = NULL;
    }
    if (NULL != _surfaceThumb)
    {
        NEXUS_Surface_Destroy(_surfaceThumb);
        _surfaceThumb = NULL;
    }
    DEL(_pThumbnailLabel);
    DEL(_pVideosMenu);
    DEL(_pPlaybackMenu);
} /* uninitialize */

/* resize the thumbnail widget based on the given width/height.  the widget
 * will be resized such that it maintains it's original height but adjusts
 * it's width to maintain the given width:height proportions.  the widget
 * will also be centered based on it's original position. */
MRect CPanelPlayback::thumbnailSetGeometryWithAspect(
        unsigned width,
        unsigned height
        )
{
    MRect rectThumbLabel(0, 0, 0, 0);

    if ((0 < width) && (0 < height))
    {
        /* adjust thumbnail label width to maintain proper video aspect ratio */
        {
            uint32_t adjWidth  = _maxThumbnailSize.width();
            uint32_t adjHeight = _maxThumbnailSize.height();

            if ((_maxThumbnailSize.width() * height) > (_maxThumbnailSize.height() * width))
            {
                /* adjust width */
                adjWidth = (_maxThumbnailSize.height() * width + 1) / height;
            }
            else
            {
                /* adjust height */
                adjHeight = (_maxThumbnailSize.width() * height + 1) / width;
            }

            rectThumbLabel.setWidth(adjWidth);
            rectThumbLabel.setHeight(adjHeight);
        }

        /* adjust thumbnail label x to center */
        rectThumbLabel.setX(_maxThumbnailSize.x() + (_maxThumbnailSize.width() - rectThumbLabel.width() + 1) / 2);
        rectThumbLabel.setY(_maxThumbnailSize.y() + (_maxThumbnailSize.height() - rectThumbLabel.height() + 1) / 2);
    }
    else
    {
        /* invalid width/height given so default to original thumbnail dimensions */
        rectThumbLabel = _maxThumbnailSize;
    }

    BDBG_MSG(("aspect corrected thumb x:%d y:%d w:%d h:%d", rectThumbLabel.x(), rectThumbLabel.y(), rectThumbLabel.width(), rectThumbLabel.height()));
    _pThumbnailLabel->setGeometry(rectThumbLabel);

    return(rectThumbLabel);
} /* thumbnailSetGeometryWithAspect */

MRect CPanelPlayback::thumbnailSetGeometryWithAspect(CVideo * pVideo)
{
    unsigned width  = 0;
    unsigned height = 0;

    if (NULL != pVideo)
    {
        width  = pVideo->getWidth();
        height = pVideo->getHeight();
    }
    return(thumbnailSetGeometryWithAspect(width, height));
} /* thumbnailSetGeometryWithAspect */

MRect CPanelPlayback::thumbnailSetGeometryWithAspect(bwin_image_t bwinImage)
{
    unsigned width  = 0;
    unsigned height = 0;

    if (NULL != bwinImage)
    {
        bwin_image_settings settings;
        bwin_image_get_settings(bwinImage, &settings);

        width  = settings.width;
        height = settings.height;
    }

    return(thumbnailSetGeometryWithAspect(width, height));
} /* thumbnailSetGeometryWithAspect */

MRect CPanelPlayback::thumbnailSetGeometryWithAspect(bwin_framebuffer_t bwinFramebuffer)
{
    unsigned width  = 0;
    unsigned height = 0;

    if (NULL != bwinFramebuffer)
    {
        bwin_framebuffer_settings settings;
        bwin_get_framebuffer_settings(bwinFramebuffer, &settings);

        width  = settings.width;
        height = settings.height;
    }

    return(thumbnailSetGeometryWithAspect(width, height));
} /* thumbnailSetGeometryWithAspect */

void CPanelPlayback::layout()
{
    MRect    rectMenu(0, 0, MENU_WIDTH_VIDEOS, 0);
    MRect    rectVideos      = _pVideosMenu->getGeometry();
    MRect    rectProps       = _pPropertiesMenu->getGeometry();
    MRect    rectThumbnail   = _pPropertiesMenu->getGeometry();
    MRect    rectPanel       = getGeometry();
    unsigned propsMargin     = 7;
    unsigned thumbnailMargin = 10;
    MRect    rectVideosListView;

    /* layout videos menu */
    rectVideos.setX(0);
    rectVideos.setY(30);
    rectVideos.setWidth(MENU_WIDTH_VIDEOS);
    rectVideos.setHeight(400);
    _pVideosMenu->setGeometry(rectVideos);

    rectVideosListView = _pVideosMenu->getListView()->getGeometry();
    rectVideosListView.setHeight(rectVideos.height() - thumbnailMargin);

    /* layout video properties menu widget */
    rectProps.setX(rectVideos.x() + rectVideos.width());
    rectProps.setY(rectVideos.y() + (rectVideosListView.height() / 2) - 20);
    rectProps.setWidth(MENU_WIDTH_PROPS);
    rectProps.setHeight(rectVideosListView.height() / 2);
    _pPropertiesMenu->setGeometry(rectProps);

    /* layout thumbnail */
    {
        rectThumbnail.setWidth(MENU_WIDTH_PROPS - (thumbnailMargin * 2));
        rectThumbnail.setHeight(rectThumbnail.width() * 9 / 16);
        rectThumbnail.setX(rectProps.x() + ((MENU_WIDTH_PROPS - rectThumbnail.width()) / 2));
        rectThumbnail.setY(((rectProps.y() - rectVideos.y() - rectThumbnail.height()) / 2) + 35);
        _pThumbnailLabel->setGeometry(rectThumbnail);

        /* save max thumbnail size/position */
        _maxThumbnailSize = rectThumbnail;

        if (NULL == _surfaceThumb)
        {
            /* create thumbnail nexus surface of the same size */
            NEXUS_SurfaceCreateSettings settings;
            NEXUS_Surface_GetDefaultCreateSettings(&settings);
            settings.width  = rectThumbnail.width();
            settings.height = rectThumbnail.height();
            _surfaceThumb   = NEXUS_Surface_Create(&settings);
        }
        else
        {
            /* sanity check: make sure thumbnail surface is equal or larger than thumbnail widget */
            NEXUS_SurfaceCreateSettings settings;
            NEXUS_Surface_GetCreateSettings(_surfaceThumb, &settings);
            BDBG_ASSERT(settings.width >= rectThumbnail.width());
            BDBG_ASSERT(settings.height >= rectThumbnail.height());
        }

        /* clear out any previously loaded framebuffer and
         * set thumbnail widget geometry based on currently loaded framebuffer or image dimensions. */
        if (NULL != _thumbnailFramebuffer)
        {
            _pThumbnailLabel->loadFramebuffer(_thumbnailFramebuffer);
            thumbnailSetGeometryWithAspect(_thumbnailFramebuffer);
        }
        else
        if (NULL != _pThumbnailLabel->getImage())
        {
            _pThumbnailLabel->loadFramebuffer(NULL);
            thumbnailSetGeometryWithAspect(_pThumbnailLabel->getImage());
        }
    }

    _pExpand->setText(_bExpandPanel ? "<<" : ">>");

    /* adjust menu to fit */
    rectMenu.setHeight(rectVideosListView.height() + 30);
    if (true == _bExpandPanel)
    {
        rectMenu.setWidth(rectProps.x() + rectProps.width() + propsMargin);
    }
    else
    {
        rectMenu.setWidth(rectVideosListView.x() + rectVideosListView.width() + propsMargin);
    }
    rectMenu.setHeight(rectVideos.y() + rectVideos.height());
    _pPlaybackMenu->setGeometry(rectMenu);

    /* adjust panel size to fit */
    rectPanel.setHeight(rectMenu.height());
    rectPanel.setWidth(rectMenu.width());
    setGeometry(rectPanel);
} /* layout */

eRet CPanelPlayback::updateProperties(CVideo * pVideo)
{
    eRet            ret   = eRet_Ok;
    CPlaybackProp * pProp = NULL;

    MListItr <CPlaybackProp> itr(&_propList);

    if (false == _bExpandPanel)
    {
        ret = eRet_InvalidState;
        goto done;
    }

    pVideo->updateFileSysInfo();

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (-1 != pProp->getTag().find("Filename"))
        {
            pProp->setValue(pVideo->getVideoName());
        }
        else
        if (-1 != pProp->getTag().find("Description"))
        {
            pProp->setValue(pVideo->getDescription());
        }
        else
        if (-1 != pProp->getTag().find("Date"))
        {
            char        buf[32]   = "Unknown";
            time_t      timeVideo = pVideo->getDate();
            struct tm * timeinfo;

            if (0 < timeVideo)
            {
                timeinfo = localtime(&timeVideo);
                strftime(buf, sizeof(buf), "%b %d %Y %H:%M", timeinfo);
            }
            pProp->setValue(buf);
        }
        else
        if (-1 != pProp->getTag().find("Ratings"))
        {
            pProp->setValue("None");
        }
        else
        if (-1 != pProp->getTag().find("Size"))
        {
            float sizeB  = pVideo->getSize();
            float sizeKB = (0.0 < sizeB)  ? (sizeB  / 1024.0) : 0.0;
            float sizeMB = (0.0 < sizeKB) ? (sizeKB / 1024.0) : 0.0;
            float sizeGB = (0.0 < sizeMB) ? (sizeMB / 1024.0) : 0.0;
            char  str[32];

            if (1.0 < sizeGB)
            {
                snprintf(str, 32, "%.2f Gigabytes", sizeGB);
            }
            else
            if (1.0 < sizeMB)
            {
                snprintf(str, 32, "%.2f Megabytes", sizeMB);
            }
            else
            if (1.0 < sizeKB)
            {
                snprintf(str, 32, "%.2f Kilobytes", sizeKB);
            }
            else
            {
                snprintf(str, 32, "%d Bytes", (int)sizeB);
            }
            pProp->setValue(str);
        }
        else
        if (-1 != pProp->getTag().find("Resolution"))
        {
            int width  = pVideo->getWidth();
            int height = pVideo->getHeight();

            if ((0 == width) || (0 == height))
            {
                pProp->setValue("Unknown");
            }
            else
            {
                pProp->setValue(MString(width) + " x " + MString(height));
            }
        }
        else
        if (-1 != pProp->getTag().find("Video Codec"))
        {
            CPid * pPid = pVideo->getPid(0, ePidType_Video);

            if (NULL == pPid)
            {
                pProp->setValue("None");
            }
            else
            {
                pProp->setValue(videoCodecToString(pPid->getVideoCodec()));
            }
        }
        else
        if (-1 != pProp->getTag().find("Video PID"))
        {
            CPid * pPid = pVideo->getPid(0, ePidType_Video);

            if (NULL == pPid)
            {
                pProp->setValue("None");
            }
            else
            {
                char str[16];
                snprintf(str, 16, "0x%x", pPid->getPid());
                pProp->setValue(str);
            }
        }
        else
        if (-1 != pProp->getTag().find("Audio Codec"))
        {
            CPid * pPid = pVideo->getPid(0, ePidType_Audio);

            if (NULL == pPid)
            {
                pProp->setValue("None");
            }
            else
            {
                pProp->setValue(audioCodecToString(pPid->getAudioCodec()));
            }
        }
        else
        if (-1 != pProp->getTag().find("Audio PID"))
        {
            CPid * pPid = pVideo->getPid(0, ePidType_Audio);

            if (NULL == pPid)
            {
                pProp->setValue("None");
            }
            else
            {
                char str[16];
                snprintf(str, 16, "0x%x", pPid->getPid());
                pProp->setValue(str);
            }
        }
    }

done:
    return(ret);
} /* updateProperties */

eRet CPanelPlayback::updateThumb(
        CVideo * pVideo,
        uint32_t incrementMsec
        )
{
    eRet           ret                   = eRet_Ok;
    CStillDecode * pStillDecode          = getModel()->getStillDecode();
    CGraphics *    pGraphics             = getModel()->getGraphics();
    CThumb *       pThumbExtractor       = getModel()->getThumbExtractor();
    bool           bThumbExtractorOpened = false;
    MRect          rectThumbLabel;

    if (false == _bExpandPanel)
    {
        ret = eRet_InvalidState;
        goto done;
    }

    if ((NULL == pStillDecode) || (NULL == pGraphics) || (NULL == pThumbExtractor))
    {
        ret = eRet_InvalidParameter;
        goto done;
    }

    if (NULL != _thumbnailFramebuffer)
    {
        /* NULL out framebuffer setting in thumbnail widget */
        _pThumbnailLabel->loadFramebuffer(NULL);

        pGraphics->destroyBwinFramebuffer(_thumbnailFramebuffer);
        _thumbnailFramebuffer = NULL;
    }

    if (NULL == pVideo)
    {
        goto error;
    }

    ret = pStillDecode->open();
    CHECK_WARN_GOTO("unable to open still decoder", ret, error);

    ret = pThumbExtractor->open(pVideo, pStillDecode, pGraphics);
    CHECK_WARN_GOTO("unable to open thumb extractor", ret, error);
    bThumbExtractorOpened = true;

    /* resize thumb widget to fit aspect ratio of thumb and center within widget */
    rectThumbLabel = thumbnailSetGeometryWithAspect(pVideo);

    {
        uint32_t timestamp = pVideo->incrementThumbTimestamp(incrementMsec);

        ret = pThumbExtractor->extract(timestamp, _surfaceThumb, rectThumbLabel.width(), rectThumbLabel.height());
        CHECK_WARN_GOTO("unable to extract thumb", ret, error);
    }

    /* valid thumb returned - update thumbnail widget */
    _thumbnailFramebuffer = pGraphics->createBwinFramebuffer(_surfaceThumb, rectThumbLabel.width(), rectThumbLabel.height());
    CHECK_PTR_ERROR_GOTO("unable to create bwin framebuffer", _thumbnailFramebuffer, ret, eRet_OutOfMemory, error);

    ret = _pThumbnailLabel->loadFramebuffer(_thumbnailFramebuffer);
    CHECK_ERROR_GOTO("unable to load framebuffer into label widget", ret, error);

    goto done;
error:
    /* NULL out framebuffer setting in thumbnail widget */
    _pThumbnailLabel->loadFramebuffer(NULL);

    /* thumbnail has default image set - resize widget to match */
    thumbnailSetGeometryWithAspect(_pThumbnailLabel->getImage());

    if (NULL != _thumbnailFramebuffer)
    {
        pGraphics->destroyBwinFramebuffer(_thumbnailFramebuffer);
        _thumbnailFramebuffer = NULL;
    }
done:
    if (true == bThumbExtractorOpened)
    {
        pThumbExtractor->close();
    }

    if (NULL != pStillDecode)
    {
        pStillDecode->close();
    }

    return(ret);
} /* updateThumb */

void CPanelPlayback::show(bool bShow)
{
    CThumb * pThumbExtractor = getModel()->getThumbExtractor();

    if (NULL != pThumbExtractor)
    {
        if (true == bShow)
        {
            pThumbExtractor->create();
            _filePropsTimer.start();
        }
        else
        {
            _filePropsTimer.stop();
            pThumbExtractor->destroy();
        }
    }

    CPanel::show(bShow);
} /* show */

void CPanelPlayback::onFocus(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    _pVideoButtonFocus = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    /* update properties based on selected video */
    MListItr <CWidgetButton> itrButtons(&_Buttons);
    CWidgetButton *          pButton;
    for (pButton = itrButtons.first(); pButton; pButton = itrButtons.next())
    {
        if (pWidget == pButton)
        {
            CVideo * pVideo = _pList->getVideo(_Buttons.index(pButton));

            /* save focused video button */
            _pVideoButtonFocus = pButton;

            /* update displayed properties based on item in focus */
            BDBG_MSG(("update properties/thumb for %s", pVideo->getVideoName().s()));
            updateProperties(pVideo);

            /* update thumbnail image with 5 second increment added to last timestamp */
            updateThumb(pVideo, 5000);
            _filePropsTimer.start();
            break;
        }
    }
} /* onFocus */

/* only handles button clicks in playback menu.  if fully handled, then it returns eRet_Ok which
 * signifies that the click has been consumed.  otherwise, it returns eRet_NotSupported and the onClick()
 * handler in screen_test.cpp will continue looking for a handler. */
void CPanelPlayback::onClick(bwidget_t widget)
{
    MListItr <CWidgetButton> itrButtons(&_Buttons);
    CWidgetBase *            pWidget = NULL;
    CWidgetButton *          pButton;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_pExpand == pWidget)
    {
        _bExpandPanel = _bExpandPanel ? false : true;
        _pExpand->setText(_bExpandPanel ? "<<" : ">>");
        layout();
    }
    else
    if (_Back == pWidget)
    {
        show(false);
        getParent()->show(true);
    }

    for (pButton = itrButtons.first(); pButton; pButton = itrButtons.next())
    {
        if (pWidget == pButton)
        {
            CVideo * video = _pList->getVideo(_Buttons.index(pButton));
            BDBG_MSG(("Info FILE selected calling Correct Playback File %s", video->getInfoName().s()));
            BDBG_MSG(("MPG FILE selected calling Correct Playback File %s", video->getVideoName().s()));
            {
                CPlaybackData playbackData(video->getVideoName().s(), NULL, NULL, video);

                notifyObservers(eNotify_PlaybackStart, &playbackData);
                show(false);
                getParent()->show(true);
            }
            break;
        }
    }

    return;
} /* onClick */

void CPanelPlayback::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_PlaybackListChanged:
    {
        CPlaybackList * pPlaybackList = (CPlaybackList *)notification.getData();
        BDBG_ASSERT(pPlaybackList);
        _pList = pPlaybackList;
        refreshPlaybackButtonList();
        layout();
    }
    break;

    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (pTimer == &_filePropsTimer)
        {
            BDBG_MSG(("file props timer expired"));

            /* update properties/thumb and restart timer if recording */
            if ((NULL != _pVideoButtonFocus) && (true == _pVideoButtonFocus->isVisible()))
            {
                CVideo * pVideo = _pList->getVideo(_Buttons.index(_pVideoButtonFocus));

                if (NULL != pVideo)
                {
                    /* update displayed properties based on item in focus */
                    updateProperties(pVideo);
                    /* update thumbnail image with 5 second increment added to last timestamp */
                    updateThumb(pVideo, 5000);

                    /* update properties/thumb continuously */
                    _filePropsTimer.start();
                }
            }
        }
    }
    break;

    default:
        break;
    } /* switch */
}     /* processNotification */

void CPanelPlayback::refreshPlaybackButtonList()
{
    /* Clear the list , then recreate it */
    MListItr <CWidgetButton> itrButtons(&_Buttons);

    CWidgetButton * pButton;
    MString         mediaName;
    CVideo *        video;
    bwin_font_t     font;
    CGraphics *     pGraphics = NULL;

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    font = pGraphics->getFont();
    BDBG_ASSERT(NULL != font);

    if (true == isVisible())
    {
        /* change focus before deleting buttons */
        _Back->setFocus();
    }

    /* remove buttons from menu and free widgets  */
    _pVideosMenu->clearButtons();
    _Buttons.clear();

    /* change playback panel last focus */
    _focusWidget = _Back->getWidget();

    if (_pList == NULL)
    {
        BDBG_ERR((" No playback List"));
        return;
    }

    {
        uint32_t index = 0;
        while (NULL != (video = _pList->getVideo(index++)))
        {
            mediaName = video->getInfoName();
            pButton   = new CWidgetButton("CPanelPlayback::pButton", getEngine(), this, MRect(0, 0, 0, 22), font);
            BDBG_MSG(("Allocate Button widget, for info file %s,", mediaName.s()));
            mediaName.truncate(mediaName.findRev(".", -1, false));
            pButton->setText((mediaName + MString("_") + MString(video->getProgram())).s(), bwidget_justify_horiz_left);
            _pVideosMenu->addButton(pButton, mediaName);
            _Buttons.add(pButton);
        }
    }
} /* refreshPlaybackButtonList */