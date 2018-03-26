/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "screen.h"
#include "widget_button.h"
#include "widget_base.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_button);

static void button_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    blabel_settings * label_settings;
    bbutton_settings  buttonSettings;
    CWidgetButton *   pButton;

    bbutton_get(widget, &buttonSettings);
    label_settings = &(buttonSettings.label);

    pButton = (CWidgetButton *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pButton);

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

    pButton->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawBevel(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawFocus(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* button_bwidget_draw */

static void focusCallback(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CWidgetBase * pParent = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    if (true == pWidget->isVisible())
    {
        pParent = pWidget->getParent();
        if (NULL != pParent)
        {
            /* widget is visible, so pass on focus callback to the parent widget */
            pParent->onFocus(widget);
        }
    }

    /* call widget's focus handler */
} /* focusCallback */

static void blurCallback(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CWidgetBase * pParent = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    if (true == pWidget->isVisible())
    {
        pParent = pWidget->getParent();
        if (NULL != pParent)
        {
            /* widget is visible, so pass on blur callback to the parent widget */
            pWidget->onBlur(widget);
        }
    }
} /* blurCallback */

static void clickCallback(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CWidgetBase * pParent = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetBase *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    if (true == pWidget->isVisible())
    {
        pParent = pWidget->getParent();
        if (NULL != pParent)
        {
            /* widget is visible, so pass on click to the parent widget */
            pParent->onClick(widget);
        }
    }
} /* clickCallback */

CWidgetButton::CWidgetButton(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) : /* optional */
    CWidgetBase(strName, engine, pParentWidget),
    _image(NULL)
{
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != pParentWidget);

    memset(_pText, 0, BUTTON_MAX_TEXT);

    bbutton_get_default_settings(&buttonSettings);
    buttonSettings.label.widget.draw                = button_bwidget_draw;
    buttonSettings.label.widget.window.parent       = (NULL == parentWin) ? pParentWidget->getWin() : parentWin;
    buttonSettings.label.widget.window.x            = geometry.x();
    buttonSettings.label.widget.window.y            = geometry.y();
    buttonSettings.label.widget.window.rect.width   = geometry.width();
    buttonSettings.label.widget.window.rect.height  = geometry.height();
    buttonSettings.label.widget.window.zorder       = (NULL == pParentWidget) ? 0 : pParentWidget->getZOrder();
    buttonSettings.label.font                       = font;
    buttonSettings.label.bevel                      = 3;
    buttonSettings.label.bevel_color[bevel_eTop]    = 0xFFAAAAAA;
    buttonSettings.label.bevel_color[bevel_eRight]  = 0xFFDCDCDC;
    buttonSettings.label.bevel_color[bevel_eBottom] = 0xFFAAAAAA;
    buttonSettings.label.bevel_color[bevel_eLeft]   = 0xFFDCDCDC;
    buttonSettings.label.text                       = _pText;
    buttonSettings.label.text_color                 = 0xFF676767; /*0xFF82C345;*/
    buttonSettings.label.text_justify_horiz         = bwidget_justify_horiz_center;
    buttonSettings.label.text_justify_vert          = bwidget_justify_vert_middle;
    buttonSettings.label.text_margin                = 2;
    buttonSettings.label.background_color_disable   = false;
    buttonSettings.label.background_color           = COLOR_EGGSHELL; /*0xFF222222;*/
    buttonSettings.label.widget.focus               = focusCallback;
    buttonSettings.label.widget.blur                = blurCallback;
    buttonSettings.click                            = clickCallback;
    buttonSettings.label.widget.data                = this; /* save ptr to this object */
    _widget = bbutton_create(getEngine(), &buttonSettings);

    setBackgroundFillMode(fill_eGradient);
}

CWidgetButton::~CWidgetButton()
{
    if (NULL != _widget)
    {
        bwidget_destroy(_widget);
        _widget = NULL;
    }

    if (NULL != _image)
    {
        bwin_image_close(_image);
        _image = NULL;
    }
}

MRect CWidgetButton::getGeometry()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(MRect(buttonSettings.label.widget.window.x,
                   buttonSettings.label.widget.window.y,
                   buttonSettings.label.widget.window.rect.width,
                   buttonSettings.label.widget.window.rect.height));
}

void CWidgetButton::setGeometry(MRect geometry)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.widget.window.x           = geometry.x();
    buttonSettings.label.widget.window.y           = geometry.y();
    buttonSettings.label.widget.window.rect.width  = geometry.width();
    buttonSettings.label.widget.window.rect.height = geometry.height();
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

uint32_t CWidgetButton::getBackgroundColor()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(buttonSettings.label.background_color);
}

void CWidgetButton::setBackgroundColor(uint32_t color)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.background_color = color;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetButton::setBackgroundFillMode(backgroundFillMode_t mode)
{
    bbutton_settings buttonSettings;

    CWidgetBase::setBackgroundFillMode(mode);

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.background_color_disable = (fill_eNone == mode) ? true : false;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

uint32_t CWidgetButton::getTextColor()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(buttonSettings.label.text_color);
}

void CWidgetButton::setTextColor(uint32_t color)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.text_color = color;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetButton::setText(
        const char *          pText,
        bwidget_justify_horiz justifyHoriz,
        bwidget_justify_vert  justifyVert
        )
{
    bbutton_settings buttonSettings;
    bool             change = false;

    if (NULL == pText)
    {
        _pText[0] = '\0';
    }
    else
    {
        strncpy(_pText, pText, BUTTON_MAX_TEXT);
    }

    bbutton_get(getWidget(), &buttonSettings);
    if (bwidget_justify_horiz_max != justifyHoriz)
    {
        buttonSettings.label.text_justify_horiz = justifyHoriz;
        change = true;
    }
    if (bwidget_justify_vert_max != justifyVert)
    {
        buttonSettings.label.text_justify_vert = justifyVert;
        change = true;
    }

    if (true == change)
    {
        bbutton_set(getWidget(), &buttonSettings);
    }

    bwin_repaint(bwidget_win(getWidget()), NULL);
} /* setText */

bwidget_justify_horiz CWidgetButton::getTextJustifyHoriz()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);

    return(buttonSettings.label.text_justify_horiz);
}

bwidget_justify_vert CWidgetButton::getTextJustifyVert()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);

    return(buttonSettings.label.text_justify_vert);
}

bwin_font_t CWidgetButton::getFont()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(buttonSettings.label.font);
}

void CWidgetButton::setFont(bwin_font_t font)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.font = font;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetButton::setBevel(int bevel)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.bevel = bevel;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetButton::setTextMargin(int margin)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.text_margin = margin;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

eRet CWidgetButton::loadImage(
        const char *           filename,
        bwin_image_render_mode renderMode
        )
{
    eRet             ret = eRet_Ok;
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != filename);

    if (NULL != _image)
    {
        bwin_image_close(_image);
    }

    _image = bwin_image_load(getWinEngine(), filename);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _image, ret, eRet_ExternalError, error);

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.image       = _image;
    buttonSettings.label.render_mode = renderMode;
    bbutton_set(getWidget(), &buttonSettings);

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadImage */