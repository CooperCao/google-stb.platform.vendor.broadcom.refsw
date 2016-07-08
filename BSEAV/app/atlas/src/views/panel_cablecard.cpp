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

#ifdef MPOD_SUPPORT

#define MAX_CABLECARD_PROPS  20
#define MENU_WIDTH           750

#include "convert.h"
#include "panel_cablecard.h"
#include "widget_grid.h"

BDBG_MODULE(panel_cablecard);

CPanelCableCard::CPanelCableCard(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelCableCard", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pCableCardMenu(NULL),
    _pPropertiesMenu(NULL),
    _timerUpdate(pWidgetEngine, this, 500),
    _counter(0)
{
}

CPanelCableCard::~CPanelCableCard()
{
    uninitialize();
}

eRet CPanelCableCard::initialize(
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
    rectPanel.set(50, 50, MENU_WIDTH, 0);
    setGeometry(rectPanel);

    _pCableCardMenu = new CWidgetMenu("CPanelCableCard::_pCableCardMenu", getEngine(), this, MRect(0, 0, 0, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pCableCardMenu, ret, eRet_OutOfMemory, error);
    _pCableCardMenu->showSubTitle(false);
    _pCableCardMenu->showListView(false);
    _pCableCardMenu->showEdit(false);
    _pCableCardMenu->setMenuTitle("CableCard", NULL);
    _pCableCardMenu->show(true);
    {
        _pPropertiesMenu = new CWidgetMenu("CPanelCableCard::_pPropertiesMenu", getEngine(), _pCableCardMenu, MRect(0, 0, MENU_WIDTH, 133), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPropertiesMenu, ret, eRet_OutOfMemory, error);
        _pPropertiesMenu->showMenuBar(false);
        _pPropertiesMenu->showEdit(false);
        _pPropertiesMenu->setMenuTitle(NULL, "CableCard Info");
        _pPropertiesMenu->setScroll(false);

        /* create max number of buttons - we will create/add them once, then
         * activate/deactivate them from the CWidgetMenu as needed. */
        for (int i = 0; i < MAX_CABLECARD_PROPS; i++)
        {
            CPanelCableCardProp * pProp = NULL;
            pProp = new CPanelCableCardProp(this, _pPropertiesMenu, MString(i).s(), font12, 0);
            CHECK_PTR_ERROR_GOTO("unable to allocate cablecard property", pProp, ret, eRet_OutOfMemory, error);

            /* start buttons as inactive - layout() will activate as needed */
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), false);
            /* keep track of all added buttons */
            _propList.add(pProp);
        }
    }

    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelCableCard::uninitialize()
{
    DEL(_pPropertiesMenu);
    DEL(_pCableCardMenu);
}

void CPanelCableCard::layout()
{
    MRect    rectMenu(0, 0, MENU_WIDTH, 0);
    MRect    rectProps  = _pPropertiesMenu->getGeometry();
    MRect    rectPanel  = getGeometry();
    uint16_t gridMargin = 7;
    MRect    rectPropsListView;

    /* layout properties menu */
    rectProps.setX(0);
    rectProps.setY(30);
    rectProps.setWidth(rectMenu.width());
    _pPropertiesMenu->setGeometry(rectProps);
    rectPropsListView = _pPropertiesMenu->getListView()->getGeometry();

    /* adjust menu to fit */
    rectMenu.setHeight(rectProps.height() + 30);

    {
        rectMenu.setWidth(rectPropsListView.x() + rectPropsListView.width() + gridMargin);
        /* no grid so menu height is based on properties menu widget */
        rectMenu.setHeight(rectProps.y() + rectProps.height());
    }
    _pCableCardMenu->setGeometry(rectMenu);

    /* adjust panel size to fit */
    rectPanel.setHeight(rectMenu.height());
    rectPanel.setWidth(rectMenu.width());
    setGeometry(rectPanel);
} /* layout */

void CPanelCableCard::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }
} /* onClick */

void CPanelCableCard::startUpdateTimers(bool bStart)
{
    if (true == bStart)
    {
        updateStatistics();
        _timerUpdate.start(5000);
    }
    else
    {
        _timerUpdate.stop();
    }
} /* startUpdateTimers */

void CPanelCableCard::show(bool bShow)
{
    CPanel::show(bShow);

    startUpdateTimers(bShow);
}

void CPanelCableCard::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerUpdate == pTimer)
        {
            updateStatistics();
            _timerUpdate.start();
        }
    }
    break;

    case eNotify_CurrentChannel:
        /* kick start update timers */
        show(isVisible());
        break;

    default:
        break;
    } /* switch */
}     /* processNotification */

/* get the list of cablecard statistics, compile them into a list of tags/values and display
 * in the properties menu list widget. */
void CPanelCableCard::updateStatistics()
{
    MListItr <CPanelCableCardProp> itr(&_propList);
    CPanelCableCardProp *          pProp = NULL;
    eRet ret                             = eRet_Ok;

    CCablecard * pCablecard = _pModel->getCableCard();

    MStringHash properties;

    CHECK_PTR_MSG_GOTO("No cablecard - unable to retrieve card statistics", pCablecard, ret, eRet_NotAvailable, error);

    /* retrieve properties from cablecard */
    pCablecard->getProperties(&properties, 1);

    pProp = itr.first();

    /* add properties to list widget */
    for (properties.first(); properties.current(); properties.next())
    {
        if (NULL != pProp)
        {
            BDBG_MSG(("updating %s=%s\n", properties.name().s(), properties.value().s()));
#if 0
            pProp->setTag(properties.name() + ":");

            pProp->setTag(properties.name());
#endif
            pProp->setValue(properties.value());
            /* note we do not use the button's show() method here since that is
             * manipulated by the control itself when scrolling.  the setListButtonActive()
             * method of the widget menu allows us to hide/show buttons without
             * removing them from the control and still allows scrolling to work
             * properly */
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), true);

            pProp = itr.next();
        }
        else
        {
            BDBG_WRN(("Too many properties to display - increase size of cablecard properties list"));
        }
    }

    /* hide any remaining unused buttons */
    while (NULL != pProp)
    {
        _pPropertiesMenu->setListButtonActive(pProp->getContainer(), false);
        pProp = itr.next();
    }

    goto done;

error:
    {
        bool bFirst = true;

        /* cablecard is unavailable - deactivate all menu list buttons */
        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            if (true == bFirst)
            {
                pProp->setTag("Cablecard Properties:");
                pProp->setValue("Unavailable");
                _pPropertiesMenu->setListButtonActive(pProp->getContainer(), true);
                bFirst = false;

                continue;
            }

            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), false);
        }
    }

done:

    layout();
} /* updateStatistics */

#endif /* MPOD_SUPPORT */