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

#define NUM_KEYS     41
#define MAX_ENTRY    30
#define MENU_WIDTH   245
#define MENU_HEIGHT  240

#include "channel.h"
#include "convert.h"
#include "panel_keyboard.h"
#include "widget_grid.h"
#include "mstringlist.h"

BDBG_MODULE(panel_network_wifi);

static int msg_box_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet             ret = eRet_NotAvailable; /* assume key is not consumed */
    CPanelKeyboard * pPopup;
    blabel_settings  labelSettings;

    blabel_get(widget, &labelSettings);
    pPopup = (CPanelKeyboard *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pPopup);

    ret = pPopup->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CPanelKeyboard::CPanelKeyboard(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelKeyboard", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _bShift(false),
    _bSymbol(false),
    _pKeyboardMenu(NULL),
    _pHeading(NULL),
    _numDigits(0),
    _pBackground(NULL),
    _done(false),
    _Back(NULL)
{
}

CPanelKeyboard::~CPanelKeyboard()
{
    uninitialize();
}

eRet CPanelKeyboard::initialize(
        CModel *  pModel,
        CConfig * pConfig
        )
{
    eRet        ret       = eRet_Ok;
    CGraphics * pGraphics = NULL;
    bwin_font_t font10;
    bwin_font_t font12;
    bwin_font_t font14;
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
    rectPanel.setX(graphicsWidth / 2 - MENU_WIDTH / 2);
    rectPanel.setY(graphicsHeight / 2 - MENU_HEIGHT / 2);
    rectPanel.setWidth(MENU_WIDTH);
    rectPanel.setHeight(MENU_HEIGHT);
    setGeometry(rectPanel);
    setZOrder(getZOrder() + 1);

    _pKeyboardMenu = new CWidgetMenu("CPanelKeyboard::_pKeyboardMenu", getEngine(), this, MRect(0, 0, MENU_WIDTH, MENU_HEIGHT), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pKeyboardMenu, ret, eRet_OutOfMemory, error);
    _pKeyboardMenu->showSubTitle(false);
    _pKeyboardMenu->showListView(false);
    _pKeyboardMenu->showEdit(false);
    _pKeyboardMenu->setMenuTitle("Keyboard", NULL);
    _pKeyboardMenu->show(true);

    {
        _pHeading = new CWidgetLabel("CPanelKeyboard::_pHeading", getEngine(), _pKeyboardMenu, MRect(0, 36, MENU_WIDTH, 20), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate heading widget", _pHeading, ret, eRet_OutOfMemory, error);
        /* _pHeading->setBackgroundColor(_pKeyboardMenu->getBackgroundColor()); */
        _pHeading->setTextColor(COLOR_GREY_DARK);
        _pHeading->setText("Enter", bwidget_justify_horiz_center, bwidget_justify_vert_bottom);
        _pHeading->setBevel(0);
        _pHeading->setZOrder(2);
        _pHeading->show(true);

        _pBackground = new CWidgetLabel("CPanelKeyboard::_pBackground", getEngine(), _pKeyboardMenu, MRect(0, 56, MENU_WIDTH, 200), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate background widget", _pBackground, ret, eRet_OutOfMemory, error);
        _pBackground->setBevel(0);
        _pBackground->setBackgroundFillMode(fill_eGradient);
        _pBackground->setBackgroundGradient(COLOR_GREY - 2 * COLOR_STEP, COLOR_GREY - 8 * COLOR_STEP, COLOR_GREY - 14 * COLOR_STEP);

        for (int i = 0; i < NUM_KEYS; i++)
        {
            CWidgetButton * pButton = NULL;

            /* create keyboard keys */
            pButton = new CWidgetButton("CPanelKeyboard::key", getEngine(), this, MRect(0, 0, 0, 0), font14, _pBackground->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate key button widget", pButton, ret, eRet_OutOfMemory, error);
            pButton->setBevel(0);
            pButton->setBackgroundFillMode(fill_eGradient);
            pButton->setTextColor(COLOR_GREY_DARK);
            pButton->setBackgroundGradient(COLOR_EGGSHELL +  2 * COLOR_STEP, COLOR_EGGSHELL - 1 * COLOR_STEP, COLOR_EGGSHELL - 6 * COLOR_STEP);

            _keyList.add(pButton);
        }
    }

    /* back button */
    _Back = new CWidgetButton("CPanelKeyboard::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
    _Back->setText("Back");
    /*
     * disable back button - done button works for ok/cancel
     *_pKeyboardMenu->addBackButton(_Back);
     */

    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelKeyboard::uninitialize()
{
    DEL(_Back);
    _keyList.clear();
    DEL(_pBackground);
    DEL(_pHeading);
    DEL(_pKeyboardMenu);
}

void CPanelKeyboard::setTitle(
        const char * strTitle,
        const char * strEntryTitle
        )
{
    BDBG_ASSERT(NULL != _pKeyboardMenu);
    BDBG_ASSERT(NULL != _pHeading);

    _pKeyboardMenu->setMenuTitle(strTitle);
    _strEntryTitle = strEntryTitle;
    _pHeading->setText(_strEntryTitle);
}

void CPanelKeyboard::addKey(const char ch)
{
    if (MAX_ENTRY == _numDigits)
    {
        BDBG_WRN(("Max number of entered digits reached"));
        return;
    }

    _strEntry += ch;

    _pHeading->setText(_strEntry);
    _numDigits++;
}

void CPanelKeyboard::delKey()
{
    if (0 < _numDigits)
    {
        _strEntry.truncate(_strEntry.length() - 1);
        _pHeading->setText(_strEntry);
        _numDigits--;
    }

    if (0 == _numDigits)
    {
        _pHeading->setText(_strEntryTitle);
    }
}

void CPanelKeyboard::clear()
{
    _numDigits = 0;
    _bShift    = false;
    _bSymbol   = false;
    _strEntry.clear();
    delKey();
}

#define HSPACE      4
#define VSPACE      10
#define KEY_WIDTH   20
#define KEY_HEIGHT  25

void CPanelKeyboard::layout(eKeyMapping keyMap)
{
    MRect       rectMenu(0, 0, MENU_WIDTH, 0);
    MRect       rectPanel = getGeometry();
    CGraphics * pGraphics = NULL;
    bwin_font_t font12    = NULL;
    bwin_font_t font14    = NULL;
    bwin_font_t font17    = NULL;

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);
    font12 = pGraphics->getFont(12);
    BDBG_ASSERT(NULL != font12);
    font14 = pGraphics->getFont(14);
    BDBG_ASSERT(NULL != font14);
    font17 = pGraphics->getFont(17);
    BDBG_ASSERT(NULL != font17);

    if (eKeyMapping_none == keyMap)
    {
        /* key map not specified so auto determine keyboard mapping */
        if (_bSymbol)
        {
            keyMap = eKeyMapping_qwertySymbol;
        }
        else
        if (_bShift)
        {
            keyMap = eKeyMapping_qwertyShift;
        }
        else
        {
            keyMap = eKeyMapping_qwerty;
        }
    }

    {
        CWidgetButton * pButton    = NULL;
        int             margin     = 5;
        int             row        = 0;
        int             i          = 0;
        int             nextButton = 0;
        int             widthKeys  = (10 * KEY_WIDTH) + (9 * HSPACE);

        /* 1st row number keys */
        {
            MStringList strListRow0;
            row    = 0;
            margin = HSPACE;
            strListRow0.add("1"); strListRow0.add("2"); strListRow0.add("3"); strListRow0.add("4"); strListRow0.add("5");
            strListRow0.add("6"); strListRow0.add("7"); strListRow0.add("8"); strListRow0.add("9"); strListRow0.add("0");
            for (i = 0; i < 10; i++)
            {
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                pButton->setFont(font17);
                pButton->setText(strListRow0[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                nextButton++;
            }
        }

        /* set key mapping and update keys */
        switch (keyMap)
        {
        case eKeyMapping_qwertyShift:
        {
            /* 2nd row keys */
            {
                MStringList strListRow1;
                row    = 1;
                margin = HSPACE;
                strListRow1.add("Q"); strListRow1.add("W"); strListRow1.add("E"); strListRow1.add("R"); strListRow1.add("T");
                strListRow1.add("Y"); strListRow1.add("U"); strListRow1.add("I"); strListRow1.add("O"); strListRow1.add("P");
                for (i = 0; i < 10; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow1[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 3rd row keys */
            {
                MStringList strListRow2;
                row    = 2;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow2.add("A"); strListRow2.add("S"); strListRow2.add("D"); strListRow2.add("F"); strListRow2.add("G");
                strListRow2.add("H"); strListRow2.add("J"); strListRow2.add("K"); strListRow2.add("L");
                for (i = 0; i < 9; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow2[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 4th row keys */
            {
                MStringList strListRow3;
                row    = 3;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow3.add("Z"); strListRow3.add("X"); strListRow3.add("C"); strListRow3.add("V"); strListRow3.add("B");
                strListRow3.add("N"); strListRow3.add("M");
                for (i = 0; i < 7; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + (i + 1) * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow3[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin, row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                if (true == _bShift)
                {
                    pButton->setBackgroundGradient(COLOR_EGGSHELL - 6 * COLOR_STEP, COLOR_EGGSHELL - 1 * COLOR_STEP, COLOR_EGGSHELL + 2 * COLOR_STEP);
                    pButton->setTextColor(COLOR_GREY_DARK);
                }
                else
                {
                    pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                    pButton->setTextColor(COLOR_EGGSHELL);
                }
                pButton->setFont(font17);
                pButton->setText("^", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                nextButton++;

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin + widthKeys - (KEY_WIDTH * 1.3), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                pButton->setFont(font17);
                pButton->setText("<", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                pButton->setTextColor(COLOR_EGGSHELL);
                nextButton++;
            }
        }
        break;

        case eKeyMapping_qwertySymbol:
        {
            /* 2nd row keys */
            {
                MStringList strListRow1;
                row    = 1;
                margin = HSPACE;
                strListRow1.add("+"); strListRow1.add("|"); strListRow1.add("~"); strListRow1.add("<"); strListRow1.add(">");
                strListRow1.add("#"); strListRow1.add("%"); strListRow1.add("^"); strListRow1.add("*"); strListRow1.add("=");
                for (i = 0; i < 10; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow1[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 3rd row keys */
            {
                MStringList strListRow2;
                row    = 2;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow2.add("/"); strListRow2.add(":"); strListRow2.add(";"); strListRow2.add("("); strListRow2.add(")");
                strListRow2.add("$"); strListRow2.add("&"); strListRow2.add("@"); strListRow2.add("\\");
                for (i = 0; i < 9; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow2[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 4th row keys */
            {
                MStringList strListRow3;
                row    = 3;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow3.add("."); strListRow3.add(","); strListRow3.add("?"); strListRow3.add("!"); strListRow3.add("\'");
                strListRow3.add("-"); strListRow3.add("_");
                for (i = 0; i < 7; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + (i + 1) * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow3[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin, row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                if (true == _bShift)
                {
                    pButton->setBackgroundGradient(COLOR_EGGSHELL - 6 * COLOR_STEP, COLOR_EGGSHELL - 1 * COLOR_STEP, COLOR_EGGSHELL + 2 * COLOR_STEP);
                    pButton->setTextColor(COLOR_GREY_DARK);
                }
                else
                {
                    pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                    pButton->setTextColor(COLOR_EGGSHELL);
                }
                pButton->setFont(font17);
                pButton->setText("^", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                nextButton++;

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin + widthKeys - (KEY_WIDTH * 1.3), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                pButton->setFont(font17);
                pButton->setText("<", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                pButton->setTextColor(COLOR_EGGSHELL);
                nextButton++;
            }
        }
        break;

        case eKeyMapping_qwerty:
        {
            /* 2nd row keys */
            {
                MStringList strListRow1;
                row    = 1;
                margin = HSPACE;
                strListRow1.add("q"); strListRow1.add("w"); strListRow1.add("e"); strListRow1.add("r"); strListRow1.add("t");
                strListRow1.add("y"); strListRow1.add("u"); strListRow1.add("i"); strListRow1.add("o"); strListRow1.add("p");
                for (i = 0; i < 10; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow1[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 3rd row keys */
            {
                MStringList strListRow2;
                row    = 2;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow2.add("a"); strListRow2.add("s"); strListRow2.add("d"); strListRow2.add("f"); strListRow2.add("g");
                strListRow2.add("h"); strListRow2.add("j"); strListRow2.add("k"); strListRow2.add("l");
                for (i = 0; i < 9; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + i * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow2[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }
            }

            /* 4th row keys */
            {
                MStringList strListRow3;
                row    = 3;
                margin = HSPACE + (KEY_WIDTH + HSPACE) / 2;
                strListRow3.add("z"); strListRow3.add("x"); strListRow3.add("c"); strListRow3.add("v"); strListRow3.add("b");
                strListRow3.add("n"); strListRow3.add("m");
                for (i = 0; i < 7; i++)
                {
                    pButton = _keyList[nextButton];
                    pButton->setGeometry(MRect(margin + (i + 1) * (KEY_WIDTH + HSPACE), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH, KEY_HEIGHT));
                    pButton->setFont(font17);
                    pButton->setText(strListRow3[i], bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                    nextButton++;
                }

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin, row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                pButton->setFont(font17);
                if (true == _bShift)
                {
                    pButton->setBackgroundGradient(COLOR_EGGSHELL - 6 * COLOR_STEP, COLOR_EGGSHELL - 1 * COLOR_STEP, COLOR_EGGSHELL + 2 * COLOR_STEP);
                    pButton->setTextColor(COLOR_GREY_DARK);
                }
                else
                {
                    pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                    pButton->setTextColor(COLOR_EGGSHELL);
                }
                pButton->setText("^", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                nextButton++;

                margin  = HSPACE;
                pButton = _keyList[nextButton];
                pButton->setGeometry(MRect(margin + widthKeys - (KEY_WIDTH * 1.3), row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 1.3, KEY_HEIGHT));
                pButton->setFont(font17);
                pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
                pButton->setText("<", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
                pButton->setTextColor(COLOR_EGGSHELL);
                nextButton++;
            }
        }
        default:
            break;
        } /* switch */

        /* 5th row keys */
        {
            int nextX = 0;

            row     = 4;
            margin  = HSPACE;
            pButton = _keyList[nextButton];
            pButton->setGeometry(MRect(margin, row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 2.5, KEY_HEIGHT));
            pButton->setBackgroundGradient(COLOR_GREY - 6 * COLOR_STEP, COLOR_GREY - 9 * COLOR_STEP, COLOR_GREY - 12 * COLOR_STEP);
            pButton->setFont(font12);
            if (false == _bSymbol)
            {
                pButton->setText("#+=", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            }
            else
            {
                pButton->setText("ABC", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            }
            pButton->setTextColor(COLOR_EGGSHELL);
            nextX = pButton->getGeometry().x() + pButton->getGeometry().width();
            nextButton++;

            pButton = _keyList[nextButton];
            pButton->setGeometry(MRect(margin + nextX, row * (KEY_HEIGHT + VSPACE) + VSPACE, widthKeys - ((KEY_WIDTH * 5) + (HSPACE * 2)), KEY_HEIGHT));
            pButton->setFont(font14);
            pButton->setText("space", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            pButton->setTextColor(COLOR_GREY - COLOR_STEP * 3);
            nextX = pButton->getGeometry().x() + pButton->getGeometry().width();
            nextButton++;

            pButton = _keyList[nextButton];
            pButton->setGeometry(MRect(margin + nextX, row * (KEY_HEIGHT + VSPACE) + VSPACE, KEY_WIDTH * 2.5, KEY_HEIGHT));
            pButton->setBackgroundGradient(COLOR_GREEN + 2 * COLOR_STEP, COLOR_GREEN, COLOR_GREEN - 4 * COLOR_STEP);
            pButton->setFont(font12);
            pButton->setText("Done", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            pButton->setTextColor(0xFF222222);
            nextX = pButton->getGeometry().x() + pButton->getGeometry().width();
            nextButton++;
        }
    }
} /* layout */

void CPanelKeyboard::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    for (int i = 0; i < NUM_KEYS; i++)
    {
        if (pWidget == _keyList[i])
        {
            CWidgetButton * pButton = NULL;

            pButton = _keyList[i];

            if (pButton->getText() == "^")
            {
                setShift(!_bShift);
                layout();
            }
            else
            if ((pButton->getText() == "#+=") || (pButton->getText() == "ABC"))
            {
                setSymbol(!_bSymbol);
                layout();
            }
            else
            if (pButton->getText() == "<")
            {
                delKey();
            }
            else
            if (pButton->getText() == "space")
            {
                addKey(' ');
            }
            else
            if (pButton->getText() == "Done")
            {
                _done = true;
            }
            else
            {
                addKey(pButton->getText()[0]);
            }

            break;
        }
    }

    if (_Back == pWidget)
    {
        _strEntry.clear();
        _done = true;
    }
} /* onClick */

void CPanelKeyboard::show(bool bShow)
{
    CPanel::show(bShow);

    {
        MRect rectGeom = getGeometry();
        BDBG_WRN(("panel geom x:%d y:%d w:%d h:%d", rectGeom.x(), rectGeom.y(), rectGeom.width(), rectGeom.height()));
    }
    {
        MRect rectGeom = _pKeyboardMenu->getGeometry();
        BDBG_WRN(("keyboard menu geom x:%d y:%d w:%d h:%d", rectGeom.x(), rectGeom.y(), rectGeom.width(), rectGeom.height()));
    }
}

MString CPanelKeyboard::showModal()
{
    MRect           geometry = getGeometry();
    MRect           rectButton;
    blabel_settings labelSettingsOrig;
    blabel_settings labelSettings;
    bwidget_t       widgetFocus;

    {
        /* save currently focused widget */
        widgetFocus = bwidget_get_focus(getEngine());

        /* modal screen so capture key down events (for consuming) */
        blabel_get(getWidget(), &labelSettings);

        /* save original settings */
        labelSettingsOrig = labelSettings;

        labelSettings.widget.key_down = msg_box_bwidget_key_down;
        labelSettings.widget.data     = this;
        blabel_set(getWidget(), &labelSettings);
    }

    clear();
    show(true);
    bwidget_set_modal(getEngine(), getWidget());

    /* set focus to 'g' key which is the centermost key */
    for (int i = 0; i < NUM_KEYS; i++)
    {
        CWidgetButton * pButton = NULL;
        pButton = _keyList[i];

        if (pButton->getText() == "g")
        {
            pButton->setFocus();
        }
    }

    /* enter local bwidget event loop so we can handle all keypresses locally */
    _done = false;
    while ((false == _done) && (!bwidget_process_event(getEngine())))
    {
    }

    /* cancelModal() was called or _pOk or _pCancel button was pressed */

    bwidget_set_modal(getEngine(), NULL);

    {
        /* restore original settings */
        blabel_get(getWidget(), &labelSettings);
        labelSettings = labelSettingsOrig;
        blabel_set(getWidget(), &labelSettings);

        /* restore previously focused widget */
        bwidget_set_focus(widgetFocus);
    }

    show(false);

    return(_strEntry);
} /* showModal */

void CPanelKeyboard::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    default:
        break;
    } /* switch */
}     /* processNotification */