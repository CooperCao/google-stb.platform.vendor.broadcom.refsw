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

#if NEXUS_HAS_FRONTEND

#define MAX_TUNER_PROPS  20
#define MENU_WIDTH       260

#include "channel.h"
#include "convert.h"
#include "panel_tuner.h"
#include "widget_grid.h"

BDBG_MODULE(panel_tuner);

CPanelTuner::CPanelTuner(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelTuner", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pTunerMenu(NULL),
    _pPropertiesMenu(NULL),
    _pGridTitle(NULL),
    _pGrid(NULL),
    _timerUpdate(pWidgetEngine, this, 500),
    _timerGrid(pWidgetEngine, this, 500),
    _numPoints(0),
    _counter(0),
    _bExpandPanel(false),
    _bStatisticsAvail(false),
    _pSoftDecisions(NULL),
    _pExpand(NULL),
    _pBack(NULL)
{
}

CPanelTuner::~CPanelTuner()
{
    uninitialize();
}

eRet CPanelTuner::initialize(
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

    _numPoints = GET_INT(_pCfg, GRID_POINT_MAX);

    _pSoftDecisions = (NEXUS_FrontendSoftDecision *) BKNI_Malloc(_numPoints*sizeof(NEXUS_FrontendSoftDecision));
    CHECK_PTR_ERROR_GOTO("unable to allocate soft decision", _pSoftDecisions, ret, eRet_OutOfMemory, error);

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

    _pTunerMenu = new CWidgetMenu("CPanelTuner::_pTunerMenu", getEngine(), this, MRect(0, 0, 0, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pTunerMenu, ret, eRet_OutOfMemory, error);
    _pTunerMenu->showSubTitle(false);
    _pTunerMenu->showListView(false);
    _pTunerMenu->showEdit(false);
    _pTunerMenu->setMenuTitle("Tuner", NULL);
    _pTunerMenu->show(true);
    {
        _pPropertiesMenu = new CWidgetMenu("CPanelTuner::_pPropertiesMenu", getEngine(), _pTunerMenu, MRect(0, 0, MENU_WIDTH, 133), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPropertiesMenu, ret, eRet_OutOfMemory, error);
        _pPropertiesMenu->showMenuBar(false);
        _pPropertiesMenu->showEdit(false);
        _pPropertiesMenu->setMenuTitle(NULL, "Properties");
        _pPropertiesMenu->setScroll(false);

        /* create max number of buttons - we will create/add them once, then
         * activate/deactivate them from the CWidgetMenu as needed. */
        for (int i = 0; i < MAX_TUNER_PROPS; i++)
        {
            CPanelTunerProp * pProp = NULL;
            pProp = new CPanelTunerProp(this, _pPropertiesMenu, MString(i).s(), font12, 55);
            CHECK_PTR_ERROR_GOTO("unable to allocate tuner property", pProp, ret, eRet_OutOfMemory, error);

            /* start buttons as inactive - layout() will activate as needed */
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), false);
            /* keep track of all added buttons */
            _propList.add(pProp);
        }

        /* constellation title */
        _pGridTitle = new CWidgetLabel("CWidgetMenu::_pGridTitle", getEngine(), _pTunerMenu, MRect(0, 0, 0, 0), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pGridTitle, ret, eRet_OutOfMemory, error);
        _pGridTitle->setBevel(0);
        _pGridTitle->setBackgroundColor(0xCC222222);
        _pGridTitle->setText("Constellation", bwidget_justify_horiz_left, bwidget_justify_vert_top);
        _pGridTitle->setTextColor(0xFF80C42F);

        /* add constellation grid - initially hidden */
        _pGrid = new CWidgetGrid("CPanelTuner::_pGrid", getEngine(), _pTunerMenu, MRect(0, 0, 0, 0), font12);
        _pGrid->setNumPoints(_numPoints);
        _pGrid->setBackgroundFillMode(fill_eGradient);
        _pGrid->setBackgroundGradient(COLOR_EGGSHELL + 2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 4 * COLOR_STEP);

        /* expand button */
        _pExpand = new CWidgetButton("CPanelTuner::_pExpand", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pExpand, ret, eRet_OutOfMemory, error);
        _pExpand->setText(">>");
        _pTunerMenu->addExpandButton(_pExpand);

        /* back button */
        _pBack = new CWidgetButton("CPanelTuner::_pBack", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pBack, ret, eRet_OutOfMemory, error);
        _pBack->setText("Menu");
        _pTunerMenu->addBackButton(_pBack);

        _pBack->setFocus();
    }

    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelTuner::uninitialize()
{
    DEL(_pBack);
    DEL(_pExpand);
    DEL(_pGrid);
    DEL(_pGridTitle);
    DEL(_pPropertiesMenu);
    DEL(_pTunerMenu);
    if (_pSoftDecisions)
    {
        BKNI_Free(_pSoftDecisions);
    }
}

#define GRID_WIDTH   150
#define GRID_HEIGHT  150

void CPanelTuner::layout()
{
    MRect    rectMenu(0, 0, MENU_WIDTH, 0);
    MRect    rectProps     = _pPropertiesMenu->getGeometry();
    MRect    rectGridTitle = _pGridTitle->getGeometry();
    MRect    rectGrid      = _pGrid->getGeometry();
    MRect    rectPanel     = getGeometry();
    uint16_t gridMargin    = 7;
    MRect    rectPropsListView;

    /* layout properties menu */
    rectProps.setX(0);
    rectProps.setY(30);
    rectProps.setWidth(rectMenu.width());
    _pPropertiesMenu->setGeometry(rectProps);
    rectPropsListView = _pPropertiesMenu->getListView()->getGeometry();

    /* layout constellation */
    {
        rectGridTitle.setX(rectProps.x() + rectProps.width());
        rectGridTitle.setY(rectProps.y());
        rectGridTitle.setWidth(MENU_WIDTH - (gridMargin * 2));
        rectGridTitle.setHeight(25);
        _pGridTitle->setGeometry(rectGridTitle);

        rectGrid.setX(rectPropsListView.x() + rectPropsListView.width() + gridMargin);
        rectGrid.setY(rectGridTitle.y() + rectGridTitle.height());
        rectGrid.setWidth(GRID_WIDTH);
        rectGrid.setHeight(GRID_HEIGHT);
        _pGrid->setGeometry(rectGrid);
    }

    /* adjust menu to fit */
    rectMenu.setHeight(rectProps.height() + 30);
    if (true == _bExpandPanel)
    {
        rectMenu.setWidth(rectGrid.x() + rectGrid.width() + gridMargin);
        /* menu height is the taller of the grid or the listview widgets */
        rectMenu.setHeight(MAX(rectProps.y() + rectProps.height(),
                        rectGrid.y() + rectGrid.height() + gridMargin));
    }
    else
    {
        rectMenu.setWidth(rectPropsListView.x() + rectPropsListView.width() + gridMargin);
        /* no grid so menu height is based on properties menu widget */
        rectMenu.setHeight(rectProps.y() + rectProps.height());
    }
    _pTunerMenu->setGeometry(rectMenu);

    /* adjust panel size to fit */
    rectPanel.setHeight(rectMenu.height());
    rectPanel.setWidth(rectMenu.width());
    setGeometry(rectPanel);
} /* layout */

void CPanelTuner::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

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
        startUpdateTimers();
    }
    else
    if (_pBack == pWidget)
    {
        show(false);
        getParent()->show(true);
    }
} /* onClick */

void CPanelTuner::startUpdateTimers(bool bStart)
{
    if (true == bStart)
    {
        updateStatistics();
        _timerUpdate.start(GET_INT(_pCfg, UI_TUNER_UPDATE_TIMEOUT));

        if (true == _bExpandPanel)
        {
            resetGridData();

            if (true == _bStatisticsAvail)
            {
                updateGridData();
                _timerGrid.start(GET_INT(_pCfg, UI_GRID_UPDATE_TIMEOUT));
            }
        }
    }
    else
    {
        _timerUpdate.stop();
        _timerGrid.stop();
    }
} /* startUpdateTimers */

void CPanelTuner::show(bool bShow)
{
    CPanel::show(bShow);

    startUpdateTimers(bShow);
}

void CPanelTuner::processNotification(CNotification & notification)
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
        else
        if (&_timerGrid == pTimer)
        {
            updateGridData();

            if (true == _bExpandPanel)
            {
                _timerGrid.start();
            }
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

/* get the list of current tuner statistics, compile them into a list of tags/values and display
 * in the properties menu list widget. we will also have to re-layou the panel since different
 * tuners have different number of properties so the panel size may change based on the tuner
 * type we are currently using. */
void CPanelTuner::updateStatistics()
{
    MListItr <CPanelTunerProp> itr(&_propList);
    CPanelTunerProp *          pProp = NULL;
    eRet ret                         = eRet_Ok;

    CChannel * pChannel = _pModel->getCurrentChannel();
    CHECK_PTR_MSG_GOTO("No current channel - unable to retrieve tuner statistics", pChannel, ret, eRet_NotAvailable, error);
    {
        CTuner *    pTuner = pChannel->getTuner();
        MStringHash properties;

        _bStatisticsAvail = false;
        CHECK_PTR_MSG_GOTO("No tuner - unable to retrieve tuner statistics", pTuner, ret, eRet_NotAvailable, error);

        /* retrieve properties from tuner */
        pTuner->getProperties(&properties);
        _bStatisticsAvail = properties.first();

        pProp = itr.first();
        BDBG_ASSERT(NULL != pProp);

        /* add tuner type and number to list widget */
        if (NULL != pProp)
        {
            pProp->setTag("Name:");
            pProp->setValue(pTuner->getName() + MString(pTuner->getNumber()));
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), true);
            pProp = itr.next();
        }
        /* add tuner id and version to list widget */
        if (NULL != pProp)
        {
            pProp->setTag("ID:");
            pProp->setValue(pTuner->getFrontendId());
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), true);
            pProp = itr.next();
        }

        /* add properties to list widget */
        for (properties.first(); properties.current(); properties.next())
        {
            if (NULL != pProp)
            {
                BDBG_MSG(("updating %s=%s\n", properties.name().s(), properties.value().s()));
                pProp->setTag(properties.name() + ":");
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
                BDBG_WRN(("Too many properties to display - increase size of tuner properties list"));
            }
        }

        /* hide any remaining unused buttons */
        while (NULL != pProp)
        {
            _pPropertiesMenu->setListButtonActive(pProp->getContainer(), false);
            pProp = itr.next();
        }
    }
    goto done;

error:
    {
        bool bFirst = true;

        /* tuner is unavailable - deactivate all menu list buttons */
        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            if (true == bFirst)
            {
                pProp->setTag("Tuner Properties:");
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

void CPanelTuner::resetGridData()
{
    /* Clear old data */
    _pGrid->clearGrid();
    _counter = 0;
}

eRet CPanelTuner::updateGridData()
{
    eRet        ret      = eRet_Ok;
    CChannel *  pChannel = NULL;
    CTuner *    pTuner   = NULL;
    NEXUS_Error nerror   = NEXUS_SUCCESS;
    uint32_t    i        = 0;

    pChannel = _pModel->getCurrentChannel();
    CHECK_PTR_MSG_GOTO("No current channel - unable to retrieve constellation data", pChannel, ret, eRet_NotAvailable, error);

    if (_counter > 10)
    {
        resetGridData();
    }
    _counter++;
    BKNI_Memset(_pSoftDecisions, 0, _numPoints*sizeof(NEXUS_FrontendSoftDecision));

    /* Must be a valid Tuner */
    pTuner = pChannel->getTuner();
    CHECK_PTR_MSG_GOTO("No tuner available - unable to retrieve tuner constellation data", pTuner, ret, eRet_NotAvailable, error);

    /* Get New Data*/
    nerror = pTuner->getSoftDecisions(_pSoftDecisions, _numPoints);
    CHECK_NEXUS_ERROR_GOTO("unable to get soft decisions", ret, nerror, error);

    for (i = 0; i < _numPoints; i++)
    {
        BDBG_MSG(("printing out coordinates %d x %d", _pSoftDecisions[i].i, _pSoftDecisions[i].q));
        _pGrid->addCoordinate(_pSoftDecisions[i].i, _pSoftDecisions[i].q);
    }

    goto done;
error:
    resetGridData();
done:
    return(ret);
} /* updateGridData */

#endif /* NEXUS_HAS_FRONTEND */