/******************************************************************************
 * (c) 2015 Broadcom Corporation
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
 ******************************************************************************/

#include "convert.h"
#include "panel_streaming.h"
#include "channel.h"
#include "time.h"
#include "playlist_db.h"

BDBG_MODULE(panel_streaming);

#define MAX_PROPS          14
#define MENU_WIDTH_VIDEOS  200

#define MENU_WIDTH_PROPS   250
#define PROP_PERCENTAGE    45

CPanelStreaming::CPanelStreaming(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelStreaming", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pStreamingMenu(NULL),
    _pPlaylistsMenu(NULL),
    _pVideosMenu(NULL),
    _pPlaylistPropertiesMenu(NULL),
    _pChannelPropertiesMenu(NULL),
    _bExpandPanel(false),
    _pExpand(NULL),
    _Back(NULL),
    _pPlaylistButtonFocus(NULL),
    _pFocusPlaylist(NULL),
    _pFocusPlaylistButton(NULL)
{
}

CPanelStreaming::~CPanelStreaming()
{
    uninitialize();
}

eRet CPanelStreaming::addProperty(
        CWidgetMenu *                pMenu,
        MAutoList <CStreamingProp> * pPropList,
        const char *                 strName,
        bwin_font_t                  font
        )
{
    uint32_t         colorText       = COLOR_EGGSHELL;
    uint32_t         colorBackground = 0xCC222222;
    CStreamingProp * pProp           = NULL;
    eRet             ret             = eRet_Ok;

    pProp = new CStreamingProp(this, pMenu, strName, font, PROP_PERCENTAGE, fill_eSolid);
    CHECK_PTR_ERROR_GOTO("unable to allocate streaming property", pProp, ret, eRet_OutOfMemory, error);
    pProp->setTag(MString(strName));
    pProp->setValue("");
    pProp->setTextColor(colorText);
    pProp->setBackgroundColor(colorBackground);
    pPropList->add(pProp);

error:
    return(ret);
} /* addProperty */

/* if you need to process any incomming notifications (processNotification()) we will need to do a similar
 * thing and put a processNotificationStreamingMenu() method in here. */

/* returns eRet_Ok if everything gets created...eRet_OutOfMemory otherwise */
eRet CPanelStreaming::initialize(
        CModel *  pModel,
        CConfig * pConfig
        )
{
    eRet        ret            = eRet_Ok;
    CGraphics * pGraphics      = NULL;
    bwin_font_t font10         = NULL;
    bwin_font_t font12         = NULL;
    bwin_font_t font14         = NULL;
    uint32_t    graphicsWidth  = 0;
    uint32_t    graphicsHeight = 0;
    MRect       rectPanel;

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

    graphicsWidth  = GET_INT(_pCfg, GRAPHICS_SURFACE_WIDTH);
    graphicsHeight = GET_INT(_pCfg, GRAPHICS_SURFACE_HEIGHT);

    /* set the size of the panel itself */
    rectPanel.set(50, 50, MENU_WIDTH_VIDEOS, 0);
    setGeometry(rectPanel);

    /* CWidgetMenu's consist of the menu bar, the scrolling list view, and the other stuff that
     * makes it look like a window.   It will internally handle all the scrolling.  you must create buttons
     * outside of the CWidgetMenu and then call addButton() to add it to the widget.  because you create
     * the buttons outside of the CWidgetMenu, we will set them up so when clicked, the callback comes
     * to the CPanelStreaming and not to the CWidgetMenu itself. */
    _pStreamingMenu = new CWidgetMenu("CPanelStreaming::_pStreamingMenu", getEngine(), this, MRect(0, 0, MENU_WIDTH_VIDEOS, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pStreamingMenu, ret, eRet_OutOfMemory, error);
    _pStreamingMenu->showSubTitle(false);
    _pStreamingMenu->showListView(false);
    _pStreamingMenu->showEdit(false);
    _pStreamingMenu->setMenuTitle("Streaming", NULL);
    _pStreamingMenu->show(true);
    {
        _pPlaylistsMenu = new CWidgetMenu("CPanelStreaming::_pPlaylistsMenu", getEngine(), _pStreamingMenu, MRect(0, 0, MENU_WIDTH_VIDEOS, 0), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPlaylistsMenu, ret, eRet_OutOfMemory, error);
        _pPlaylistsMenu->showMenuBar(false);
        _pPlaylistsMenu->showEdit(false);
        _pPlaylistsMenu->setMenuTitle(NULL, "Discovered Playlists");
        _pPlaylistsMenu->setScroll(true);
    }
    {
        _pVideosMenu = new CWidgetMenu("CPanelStreaming::_pVideosMenu", getEngine(), _pStreamingMenu, MRect(0, 0, MENU_WIDTH_VIDEOS, 0), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pVideosMenu, ret, eRet_OutOfMemory, error);
        _pVideosMenu->showMenuBar(false);
        _pVideosMenu->showEdit(false);
        _pVideosMenu->setMenuTitle(NULL, "Playlist");
        _pVideosMenu->setScroll(true);
    }

    /* discovered Playlist item details in expansion panel */
    _pPlaylistPropertiesMenu = new CWidgetMenu("CPanelStreaming::_pPlaylistPropertiesMenu", getEngine(), _pStreamingMenu, MRect(0, 0, MENU_WIDTH_PROPS, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPlaylistPropertiesMenu, ret, eRet_OutOfMemory, error);
    _pPlaylistPropertiesMenu->showMenuBar(false);
    _pPlaylistPropertiesMenu->showEdit(false);
    _pPlaylistPropertiesMenu->setMenuTitle("", "Playlist Properties");
    _pPlaylistPropertiesMenu->setScroll(false);
    _pPlaylistPropertiesMenu->setListViewBackgroundColor(0xFF222222);
    _pPlaylistPropertiesMenu->setListViewTextColor(COLOR_EGGSHELL);
    _pPlaylistPropertiesMenu->setListViewBevel(0);

    /* add properties labels */
    for (int i = 0; i < MAX_PROPS; i++)
    {
        ret = addProperty(_pPlaylistPropertiesMenu, &_propListPlaylist, "", font10);
        CHECK_ERROR_GOTO("unable to create playlist property", ret, error);
    }

    /* channel item details in expansion panel */
    _pChannelPropertiesMenu = new CWidgetMenu("CPanelStreaming::_pChannelPropertiesMenu", getEngine(), _pStreamingMenu, MRect(0, 0, MENU_WIDTH_PROPS, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pChannelPropertiesMenu, ret, eRet_OutOfMemory, error);
    _pChannelPropertiesMenu->showMenuBar(false);
    _pChannelPropertiesMenu->showEdit(false);
    _pChannelPropertiesMenu->setMenuTitle("", "Channel Properties");
    _pChannelPropertiesMenu->setScroll(false);
    _pChannelPropertiesMenu->setListViewBackgroundColor(0xFF222222);
    _pChannelPropertiesMenu->setListViewTextColor(COLOR_EGGSHELL);
    _pChannelPropertiesMenu->setListViewBevel(0);
    _pChannelPropertiesMenu->show(false);

    /* add properties labels */
    for (int i = 0; i < MAX_PROPS; i++)
    {
        ret = addProperty(_pChannelPropertiesMenu, &_propListChannel, "", font10);
        CHECK_ERROR_GOTO("unable to create channel property", ret, error);
    }

    /* expand button */
    _pExpand = new CWidgetButton("CPanelStreaming::_pExpand", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pExpand, ret, eRet_OutOfMemory, error);
    _pStreamingMenu->addExpandButton(_pExpand);
    _bExpandPanel = true;

    /* back button */
    /* the back button is the button in the upper left area of the CWidgetMenu.  similar to
     * the other menu buttons, it is created here and passed in using addBackButton().  it behaves
     * similarly where click callbacks come to this CPanelStreaming. */
    _Back = new CWidgetButton("CPanelStreaming::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
    _Back->setText("Menu");
    _pStreamingMenu->addBackButton(_Back);

    _Back->setFocus();

    refreshPlaylistButtonList();

    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelStreaming::uninitialize()
{
    CGraphics * pGraphics = _pModel->getGraphics();

    BDBG_ASSERT(NULL != pGraphics);

    /* remove buttons from menu and free widgets  */
    _pPlaylistsMenu->clearButtons();
    _pVideosMenu->clearButtons();
    _ButtonsPlaylist.clear();
    _propListPlaylist.clear();
    _propListChannel.clear();

    DEL(_Back);
    DEL(_pExpand);
    DEL(_pPlaylistPropertiesMenu);
    DEL(_pChannelPropertiesMenu);
    DEL(_pPlaylistsMenu);
    DEL(_pVideosMenu);
    DEL(_pStreamingMenu);
} /* uninitialize */

void CPanelStreaming::layout()
{
    MRect    rectMenu(0, 0, MENU_WIDTH_VIDEOS, 0);
    MRect    rectPlaylists = _pPlaylistsMenu->getGeometry();
    MRect    rectProps     = _pPlaylistPropertiesMenu->getGeometry();
    MRect    rectPanel     = getGeometry();
    uint16_t propsMargin   = 7;
    uint16_t topMargin     = 10;
    MRect    rectPlaylistsListView;

    /* layout videos menu */
    rectPlaylists.setX(0);
    rectPlaylists.setY(30);
    rectPlaylists.setWidth(MENU_WIDTH_VIDEOS);
    rectPlaylists.setHeight(300);
    _pPlaylistsMenu->setGeometry(rectPlaylists);
    _pVideosMenu->setGeometry(rectPlaylists);

    rectPlaylistsListView = _pPlaylistsMenu->getListView()->getGeometry();
    rectPlaylistsListView.setHeight(rectPlaylists.height() - topMargin);

    /* layout Playlist/channel properties menu widgets */
    rectProps.setX(rectPlaylists.x() + rectPlaylists.width());
    rectProps.setY(rectPlaylists.y());
    rectProps.setWidth(MENU_WIDTH_PROPS);
    rectProps.setHeight(rectPlaylistsListView.height() / 2);
    _pPlaylistPropertiesMenu->setGeometry(rectProps);
    _pChannelPropertiesMenu->setGeometry(rectProps);

    _pExpand->setText(_bExpandPanel ? "<<" : ">>");

    /* adjust menu to fit */
    rectMenu.setHeight(rectPlaylistsListView.height() + 30);
    if (true == _bExpandPanel)
    {
        rectMenu.setWidth(rectProps.x() + rectProps.width() + propsMargin);
    }
    else
    {
        rectMenu.setWidth(rectPlaylistsListView.x() + rectPlaylistsListView.width() + propsMargin);
    }
    rectMenu.setHeight(rectPlaylists.y() + rectPlaylists.height());
    _pStreamingMenu->setGeometry(rectMenu);

    /* adjust panel size to fit */
    rectPanel.setHeight(rectMenu.height());
    rectPanel.setWidth(rectMenu.width());
    setGeometry(rectPanel);
} /* layout */

eRet CPanelStreaming::updateProperties(CPlaylist * pPlaylist)
{
    eRet             ret   = eRet_Ok;
    CStreamingProp * pProp = NULL;

    MListItr <CStreamingProp> itr(&_propListPlaylist);

    if (false == _bExpandPanel)
    {
        ret = eRet_InvalidState;
        goto done;
    }

    /* update properties based on playlist meta data */
    if (NULL != pPlaylist)
    {
        int nMetadata = pPlaylist->totalMetadata();
        int nPropList = _propListPlaylist.total();
        int i         = 0;

        /* copy metadata to properties list */
        for (i = 0; (i < nMetadata) && (i < nPropList); i++)
        {
            pProp = _propListPlaylist.at(i);

            pProp->setTag(pPlaylist->getMetadataTag(i));
            pProp->setValue(pPlaylist->getMetadataValue(i));
        }

        /* clear out remaining properties list */
        while (nPropList > i)
        {
            pProp = _propListPlaylist.at(i);

            pProp->setTag("");
            pProp->setValue("");

            i++;
        }
    }
    else
    {
        /* no playlist given so clear properties */
        int nPropList = _propListPlaylist.total();
        int i         = 0;

        while (nPropList > i)
        {
            pProp = _propListPlaylist.at(i);

            pProp->setTag(NULL);
            pProp->setValue(NULL);

            i++;
        }
    }

done:
    return(ret);
} /* updateProperties */

eRet CPanelStreaming::updateProperties(CChannel * pChannel)
{
    eRet             ret   = eRet_Ok;
    CStreamingProp * pProp = NULL;

    MListItr <CStreamingProp> itr(&_propListChannel);

    if (false == _bExpandPanel)
    {
        ret = eRet_InvalidState;
        goto done;
    }

    /* update properties based on playlist meta data */
    if (NULL != pChannel)
    {
        int nMetadata = pChannel->totalMetadata();
        int nPropList = _propListChannel.total();
        int i         = 0;

        /* copy metadata to properties list */
        for (i = 0; (i < nMetadata) && (i < nPropList); i++)
        {
            pProp = _propListChannel.at(i);

            pProp->setTag(pChannel->getMetadataTag(i));
            pProp->setValue(pChannel->getMetadataValue(i));
        }

        /* clear out remaining properties list */
        while (nPropList > i)
        {
            pProp = _propListChannel.at(i);

            pProp->setTag(NULL);
            pProp->setValue(NULL);

            i++;
        }
    }
    else
    {
        /* no channel given so clear properties */
        int nPropList = _propListChannel.total();
        int i         = 0;

        while (nPropList > i)
        {
            pProp = _propListChannel.at(i);

            pProp->setTag(NULL);
            pProp->setValue(NULL);

            i++;
        }
    }

done:
    return(ret);
} /* updateProperties */

void CPanelStreaming::show(bool bShow)
{
    CPanel::show(bShow);
} /* show */

void CPanelStreaming::onFocus(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    _pPlaylistButtonFocus = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (pWidget == _Back)
    {
        if (_Back->getText() == "Menu")
        {
            updateProperties((CPlaylist *)NULL);
        }
        else
        {
            updateProperties((CChannel *)NULL);
        }
    }
    else
    {
        /* update properties based on selected video */
        CWidgetButton * pButton;

        for (pButton = _ButtonsPlaylist.first(); pButton; pButton = _ButtonsPlaylist.next())
        {
            CPlaylistDb * pPlaylistDb = _pModel->getPlaylistDb();

            if (pWidget == pButton)
            {
                CPlaylist * pPlaylist = pPlaylistDb->getPlaylist(_ButtonsPlaylist.index(pButton));

                /* save focused plalist button */
                _pPlaylistButtonFocus = pButton;

                /* update displayed Playlist properties based on item in focus */
                BDBG_MSG(("update Playlist properties for %s", pPlaylist->getName().s()));
                updateProperties(pPlaylist);
                break;
            }
        }

        if (NULL == pButton)
        {
            int i = 0;

            for (pButton = _ButtonsVideo.first(); pButton; pButton = _ButtonsVideo.next())
            {
                if (pWidget == pButton)
                {
                    break;
                }

                i++;
            }

            if (NULL != pButton)
            {
                CChannel * pChannel = _pFocusPlaylist->getChannel(i);

                /* update displayed channel properties based on item in focus */
                BDBG_MSG(("update channel properties for %s", pChannel->getDescription().s()));
                updateProperties(pChannel);
            }
        }
    }
} /* onFocus */

void CPanelStreaming::showVideosMenu(bool bShow)
{
    eRet ret = eRet_Ok;

    if (true == bShow)
    {
        _pPlaylistsMenu->show(false);
        _pPlaylistPropertiesMenu->show(false);
        _pVideosMenu->show(true);
        _pChannelPropertiesMenu->show(true);

        _Back->setText("Playlists");
        _Back->setFocus();

        {
            CWidgetButton * pButtonVideo = NULL;
            CChannel *      pChannel     = NULL;
            int             i            = 0;
            CGraphics *     pGraphics    = NULL;
            bwin_font_t     font         = NULL;

            pGraphics = _pModel->getGraphics();
            BDBG_ASSERT(NULL != pGraphics);

            font = pGraphics->getFont();
            BDBG_ASSERT(NULL != font);

            _pVideosMenu->clearButtons();
            _ButtonsVideo.clear();

            if (NULL != _pFocusPlaylist)
            {
                _pVideosMenu->setMenuTitle(NULL, _pFocusPlaylist->getName().s());
            }

            for (pChannel = _pFocusPlaylist->getChannel(i); pChannel; pChannel = _pFocusPlaylist->getChannel(++i))
            {
                pButtonVideo = new CWidgetButton("CPanelStreaming::pButtonVideo", getEngine(), this, MRect(0, 0, 0, 22), font);
                CHECK_PTR_ERROR_GOTO("unable to allocate video button widget", pButtonVideo, ret, eRet_OutOfMemory, error);

                BDBG_MSG(("Allocate Button widget, for video:%s,", pChannel->getDescriptionShort().s()));
                pButtonVideo->setText(pChannel->getDescriptionShort().s(), bwidget_justify_horiz_left);
                _pVideosMenu->addButton(pButtonVideo, pChannel->getDescriptionShort().s());
                _ButtonsVideo.add(pButtonVideo);
            }
        }
    }
    else
    {
        _pVideosMenu->show(false);
        _pChannelPropertiesMenu->show(false);
        _pPlaylistsMenu->show(true);
        _pPlaylistPropertiesMenu->show(true);

        _Back->setText("Menu");

        if (NULL != _pFocusPlaylistButton)
        {
            _pFocusPlaylistButton->setFocus();
        }
    }
error:
    return;
} /* showVideosMenu */

/* only handles button clicks in playback menu.  if fully handled, then it returns eRet_Ok which
 * signifies that the click has been consumed.  otherwise, it returns eRet_NotSupported and the onClick()
 * handler in screen_test.cpp will continue looking for a handler. */
void CPanelStreaming::onClick(bwidget_t widget)
{
    CWidgetBase *   pWidget = NULL;
    CWidgetButton * pButton;

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
        _pStreamingMenu->setMenuTitle(_bExpandPanel ? "Streaming Player" : "Streaming", NULL);
        layout();
    }
    else
    if (_Back == pWidget)
    {
        if (_Back->getText() == "Menu")
        {
            show(false);
            getParent()->show(true);
        }
        else
        {
            showVideosMenu(false);
        }
    }

    /* check for click on Playlist button */
    for (pButton = _ButtonsPlaylist.first(); pButton; pButton = _ButtonsPlaylist.next())
    {
        CPlaylistDb * pPlaylistDb = _pModel->getPlaylistDb();

        if (pWidget == pButton)
        {
            _pFocusPlaylistButton = pButton;
            _pFocusPlaylist       = pPlaylistDb->getPlaylist(_ButtonsPlaylist.index(pButton));
            if (NULL != _pFocusPlaylist)
            {
                showVideosMenu(true);
            }
            break;
        }
    }

    if (NULL == pButton)
    {
        int i = 0;

        /* check for click on video button */
        for (pButton = _ButtonsVideo.first(); pButton; pButton = _ButtonsVideo.next())
        {
            if (pWidget == pButton)
            {
                break;
            }

            i++;
        }

        if (NULL != pButton)
        {
            CChannel * pChannel = _pFocusPlaylist->getChannel(i);

            CChannelData channelData(NULL);
            channelData._pChannel = pChannel;
            notifyObservers(eNotify_Tune, &channelData);
        }
    }

    return;
} /* onClick */

void CPanelStreaming::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_PlaylistAdded:
    {
        CPlaylist * pPlaylist = (CPlaylist *)notification.getData();
        BDBG_ASSERT(pPlaylist);
        refreshPlaylistButtonList();
    }
    break;

    case eNotify_PlaylistRemoved:
    {
        CPlaylist * pPlaylist = (CPlaylist *)notification.getData();
        BDBG_ASSERT(pPlaylist);
        refreshPlaylistButtonList();

        {
            _pFocusPlaylistButton = NULL;
            showVideosMenu(false);
        }
    }
    break;

    default:
        break;
    } /* switch */
}     /* processNotification */

eRet CPanelStreaming::refreshPlaylistButtonList()
{
    /* Clear the list , then recreate it */
    bwin_font_t font;
    CGraphics * pGraphics = NULL;
    eRet        ret       = eRet_Ok;

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
    _pPlaylistsMenu->clearButtons();
    _ButtonsPlaylist.clear();

    /* change playback panel last focus */
    _focusWidget = _Back->getWidget();

    {
        CPlaylist *   pPlaylist   = NULL;
        CPlaylistDb * pPlaylistDb = _pModel->getPlaylistDb();
        int           i           = 0;

        if ((NULL == pPlaylistDb) || (0 == pPlaylistDb->numPlaylists()))
        {
            BDBG_WRN(("No Playlists Found"));
            goto error;
        }

        for (pPlaylist = pPlaylistDb->getPlaylist(i); pPlaylist; pPlaylist = pPlaylistDb->getPlaylist(++i))
        {
            ret = addPlaylist(pPlaylist);
            CHECK_ERROR_GOTO("unable to add playlist", ret, error);
        }
    }

error:
    return(ret);
} /* refreshPlaylistButtonList */

eRet CPanelStreaming::addPlaylist(CPlaylist * pPlaylist)
{
    eRet            ret = eRet_Ok;
    bwin_font_t     font;
    CGraphics *     pGraphics = NULL;
    CWidgetButton * pButton   = NULL;

    BDBG_ASSERT(NULL != pPlaylist);

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    font = pGraphics->getFont();
    BDBG_ASSERT(NULL != font);

    /* todo: verify newly discovered Playlist is not already in list */

    pButton = new CWidgetButton("CPanelStreaming::pButton", getEngine(), this, MRect(0, 0, 0, 22), font);
    BDBG_MSG(("Allocate Button widget, for disovered Playlist:%s,", pPlaylist->getName().s()));
    pButton->setText(pPlaylist->getName().s(), bwidget_justify_horiz_left);
    pButton->setValue((int)pPlaylist);

    _pPlaylistsMenu->addButton(pButton, pPlaylist->getName().s());
    _ButtonsPlaylist.add(pButton);
    _pPlaylistsMenu->layout();

    return(ret);
} /* addPlaylist */

eRet CPanelStreaming::removePlaylist(CPlaylist * pPlaylist)
{
    eRet ret = eRet_NotAvailable;

    CWidgetButton * pButton = NULL;

    for (pButton = _ButtonsPlaylist.first(); pButton; pButton = _ButtonsPlaylist.next())
    {
        if (pButton->getValue() == (int)pPlaylist)
        {
            _pPlaylistsMenu->removeButton(pButton);
            _ButtonsPlaylist.remove(pButton); /* MAutoList so frees pButton memory */
            _pPlaylistsMenu->layout();

            ret = eRet_Ok;
            break;
        }
    }

    return(ret);
} /* removePlaylist */