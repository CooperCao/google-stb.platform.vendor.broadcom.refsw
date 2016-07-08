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

#include "widget_label.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_label);

static void label_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    blabel_settings * label_settings;
    blabel_settings   labelSettings;
    CWidgetLabel *    pLabel;

    blabel_get(widget, &labelSettings);
    label_settings = &labelSettings;

    pLabel = (CWidgetLabel *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pLabel);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    BDBG_MSG(("draw_label: %p %d,%d,%d,%d (%d,%d,%d,%d)",
              (void *)widget, label_settings->widget.window.x, label_settings->widget.window.y,
              win_settings.rect.width, win_settings.rect.height,
              cliprect->x, cliprect->y, cliprect->width, cliprect->height));

    pLabel->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pLabel->drawBevel(widget, label_settings, win_settings.rect, cliprect);
    pLabel->drawFocus(widget, label_settings, win_settings.rect, cliprect);
    pLabel->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* label_bwidget_draw */

static void focusCallback(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    /* call widget's focus handler */
    pWidget->onFocus(widget);
}

static void blurCallback(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    /* call widget's blur handler */
    pWidget->onBlur(widget);
}

static int keyDownCallback(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet          ret     = eRet_NotAvailable; /* assume key is not consumed */
    CWidgetBase * pWidget = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    /* call widget's key press handler - return code determines
     * whether key is consumed or will be propogated further. */
    ret = pWidget->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CWidgetLabel::CWidgetLabel(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) : /* optional */
    CWidgetBase(strName, engine, pParentWidget),
    _image(NULL),
    _framebuffer(NULL)
{
    blabel_settings labelSettings;

    BDBG_ASSERT((NULL != pParentWidget) || (NULL != parentWin));

    memset(_pText, 0, LABEL_MAX_TEXT);

    blabel_get_default_settings(&labelSettings);
    labelSettings.widget.draw                = label_bwidget_draw;
    labelSettings.widget.window.parent       = (NULL == parentWin) ? pParentWidget->getWin() : parentWin,
    labelSettings.widget.window.x            = geometry.x();
    labelSettings.widget.window.y            = geometry.y();
    labelSettings.widget.window.rect.width   = geometry.width();
    labelSettings.widget.window.rect.height  = geometry.height();
    labelSettings.widget.window.zorder       = (NULL == pParentWidget) ? 0 : pParentWidget->getZOrder();
    labelSettings.font                       = font;
    labelSettings.bevel                      = 3;
    labelSettings.bevel_color[bevel_eTop]    = 0xFFAAAAAA;
    labelSettings.bevel_color[bevel_eRight]  = 0xFFAAAAAA;
    labelSettings.bevel_color[bevel_eBottom] = 0xFFAAAAAA;
    labelSettings.bevel_color[bevel_eLeft]   = 0xFFAAAAAA;
    labelSettings.text                       = _pText;
    labelSettings.text_color                 = 0xFF676767;
    labelSettings.text_justify_horiz         = bwidget_justify_horiz_left;
    labelSettings.text_margin                = 2;
    labelSettings.background_color_disable   = false;
    labelSettings.background_color           = COLOR_EGGSHELL;
    labelSettings.widget.focus               = focusCallback;
    labelSettings.widget.blur                = blurCallback;
    labelSettings.widget.key_down            = keyDownCallback;
    labelSettings.widget.data                = this; /* save ptr to this object */
    _widget = blabel_create(getEngine(), &labelSettings);
}

CWidgetLabel::~CWidgetLabel()
{
    if (NULL != _widget)
    {
        bwidget_destroy(_widget);
        _widget = NULL;
    }

    if (NULL != _framebuffer)
    {
        /* framebuffer was created outside widget so do not delete it here */
        _framebuffer = NULL;
    }

    if (NULL != _image)
    {
        bwin_image_close(_image);
        _image = NULL;
    }
}

MRect CWidgetLabel::getGeometry()
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    return(MRect(labelSettings.widget.window.x,
                   labelSettings.widget.window.y,
                   labelSettings.widget.window.rect.width,
                   labelSettings.widget.window.rect.height));
}

void CWidgetLabel::setGeometry(MRect geometry)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.widget.window.x           = geometry.x();
    labelSettings.widget.window.y           = geometry.y();
    labelSettings.widget.window.rect.width  = geometry.width();
    labelSettings.widget.window.rect.height = geometry.height();
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

uint32_t CWidgetLabel::getBackgroundColor()
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    return(labelSettings.background_color);
}

void CWidgetLabel::setBackgroundColor(uint32_t color)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.background_color = color;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetLabel::setBackgroundFillMode(backgroundFillMode_t mode)
{
    blabel_settings labelSettings;

    CWidgetBase::setBackgroundFillMode(mode);

    blabel_get(getWidget(), &labelSettings);
    labelSettings.background_color_disable = (fill_eNone == mode) ? true : false;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

uint32_t CWidgetLabel::getTextColor()
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    return(labelSettings.text_color);
}

void CWidgetLabel::setTextColor(uint32_t color)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.text_color = color;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetLabel::setText(
        const char *          pText,
        bwidget_justify_horiz justifyHoriz,
        bwidget_justify_vert  justifyVert
        )
{
    blabel_settings labelSettings;
    bool            change = false;

    if (NULL == pText)
    {
        _pText[0] = '\0';
    }
    else
    {
        strncpy(_pText, pText, LABEL_MAX_TEXT);
    }

    blabel_get(getWidget(), &labelSettings);
    if (bwidget_justify_horiz_max != justifyHoriz)
    {
        labelSettings.text_justify_horiz = justifyHoriz;
        change                           = true;
    }
    if (bwidget_justify_vert_max != justifyVert)
    {
        labelSettings.text_justify_vert = justifyVert;
        change                          = true;
    }

    if (true == change)
    {
        blabel_set(getWidget(), &labelSettings);
    }

    bwin_repaint(bwidget_win(getWidget()), NULL);
} /* setText */

int CWidgetLabel::getBevel()
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    return(labelSettings.bevel);
}

void CWidgetLabel::setBevel(int bevel)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.bevel = bevel;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetLabel::setTextMargin(int margin)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.text_margin = margin;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

bwin_font_t CWidgetLabel::getFont()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(buttonSettings.label.font);
}

void CWidgetLabel::setFont(bwin_font_t font)
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.font = font;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

bwin_image_t CWidgetLabel::getImage()
{
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);

    return(labelSettings.image);
} /* getImageSize */

eRet CWidgetLabel::loadImage(
        const char *           filename,
        bwin_image_render_mode renderMode
        )
{
    eRet            ret = eRet_Ok;
    blabel_settings labelSettings;

    BDBG_ASSERT(NULL != filename);

    if (NULL != _image)
    {
        bwin_image_close(_image);
        _image = NULL;
    }

    _image = bwin_image_load(getWinEngine(), filename);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _image, ret, eRet_ExternalError, error);

    blabel_get(getWidget(), &labelSettings);
    labelSettings.image       = _image;
    labelSettings.render_mode = renderMode;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadImage */

eRet CWidgetLabel::loadImage(
        const void *           mem,
        unsigned               length,
        bwin_image_render_mode renderMode
        )
{
    eRet            ret = eRet_Ok;
    blabel_settings labelSettings;

    BDBG_ASSERT(NULL != mem);
    BDBG_ASSERT(0 < length);
    _image = bwin_image_load_memory(getWinEngine(), mem, length);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _image, ret, eRet_ExternalError, error);

    blabel_get(getWidget(), &labelSettings);
    labelSettings.image       = _image;
    labelSettings.render_mode = renderMode;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadImage */

eRet CWidgetLabel::loadFramebuffer(
        bwin_framebuffer_t     framebuffer,
        bwin_image_render_mode renderMode
        )
{
    eRet            ret = eRet_Ok;
    blabel_settings labelSettings;

    blabel_get(getWidget(), &labelSettings);
    labelSettings.framebuffer = framebuffer;
    labelSettings.render_mode = renderMode;
    blabel_set(getWidget(), &labelSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);

    return(ret);
} /* loadFramebuffer */