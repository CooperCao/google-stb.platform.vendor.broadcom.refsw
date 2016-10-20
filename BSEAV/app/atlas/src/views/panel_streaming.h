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

#ifndef ATLAS_PANEL_STREAMING_H__
#define ATLAS_PANEL_STREAMING_H__

#include "panel.h"
#include "timer.h"
#include "videolist.h"
#include "mlist.h"

#include "widget_label.h"
#include "widget_button.h"
#include "widget_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CStreamingProp is the tag/value button used to display each Streaming file properties
 * in the list widget. */
class CStreamingProp
{
public:
    CStreamingProp(
            CPanel *             pPanel,
            CWidgetMenu *        pMenu,
            const char *         strName,
            bwin_font_t          font,
            uint8_t              labelPercentage = 50,
            backgroundFillMode_t fillMode = fill_eGradient
            )
    {
        eRet ret = eRet_Ok;

        BDBG_ASSERT(NULL != pPanel);
        ret = pPanel->addDualLabelButton(pMenu, strName, &_pContainer, &_pTag, &_pValue, font, labelPercentage, fillMode);
        if (eRet_Ok != ret)
        {
            fprintf(stderr, "unable to allocate double label button");
            goto error;
        }
        _pContainer->setFocusable(false);

error:
        return;
    }

    ~CStreamingProp()
    {
        DEL(_pContainer);
        DEL(_pTag);
        DEL(_pValue);
    }

    CWidgetButton * getContainer(void) { return(_pContainer); }

    MString getTag(void) { return(MString(_pTag->getText())); }

    void setTag(const char * pText)
    {
        if (NULL != _pTag)
        {
            _pTag->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        }
    }

    MString getValue(void) { return(MString(_pValue->getText())); }

    void setValue(const char * pText)
    {
        if (NULL != _pValue)
        {
            _pValue->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        }
    }

    void setTextColor(uint32_t color)
    {
        _pTag->setTextColor(color);
        _pValue->setTextColor(color);
    }

    void setBackgroundColor(uint32_t color)
    {
        _pTag->setBackgroundColor(color);
        _pValue->setBackgroundColor(color);
        _pContainer->setBackgroundColor(color);
    }

protected:
    CWidgetButton * _pContainer;
    CWidgetLabel *  _pTag;
    CWidgetLabel *  _pValue;
};

class CVideo;
class CPlaylist;

#define THUMBNAIL_TIMESTAMP_MSEC  5000

class CPanelStreaming : public CPanel
{
public:
    CPanelStreaming(
            CWidgetEngine * pWidgetEngine,
            CScreenMain *   pScreenMain,
            CWidgetBase *   pParentWidget,
            MRect           geometry,
            bwin_font_t     font,
            bwin_t          parentWin = NULL
            ); /* optional */
    ~CPanelStreaming(void);

    eRet initialize(CModel * pModel, CConfig * pConfig);
    void uninitialize(void);
    eRet addProperty(CWidgetMenu * pMenu, MAutoList <CStreamingProp> * pPropList, const char * strName, bwin_font_t font);
    eRet updateProperties(CPlaylist * pPlaylist);
    eRet updateProperties(CChannel * pChannel);
    void show(bool bShow);
    void onFocus(bwidget_t widget);
    void onClick(bwidget_t widget);
    void processNotification(CNotification & notification);
    eRet refreshPlaylistButtonList(void);
    eRet addPlaylist(CPlaylist * pPlaylist);
    eRet removePlaylist(CPlaylist * pPlaylist);
    void showVideosMenu(bool bShow);

    void layout(void);

protected:
    eRet addStreamingProp(CWidgetMenu * pMenu, const char * strName, bwin_font_t font);

protected:
    CWidgetMenu *              _pStreamingMenu;
    CWidgetMenu *              _pPlaylistsMenu;
    CWidgetMenu *              _pVideosMenu;
    CWidgetMenu *              _pPlaylistPropertiesMenu;
    CWidgetMenu *              _pChannelPropertiesMenu;
    bool                       _bExpandPanel;
    CWidgetButton *            _pExpand;
    CWidgetButton *            _Back;
    MAutoList <CWidgetButton>  _ButtonsPlaylist;
    MAutoList <CWidgetButton>  _ButtonsVideo;
    MAutoList <CStreamingProp> _propListPlaylist;
    MAutoList <CStreamingProp> _propListChannel;
    CWidgetButton *            _pPlaylistButtonFocus;
    CPlaylist *                _pFocusPlaylist;
    CWidgetButton *            _pFocusPlaylistButton;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_STREAMING_H__ */