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

#define MAX_BLUETOOTH_PROPS    100
#define MAX_CONNECTION_STATUS  16
#define MENU_WIDTH             340
#define MENU_HEIGHT            355

#include "channel.h"
#include "convert.h"
#include "screen_main.h"
#include "panel_bluetooth.h"
#include "panel_keyboard.h"
#include "widget_grid.h"
#include "bluetooth.h"

BDBG_MODULE(panel_bluetooth);

#ifdef NETAPP_SUPPORT
CPanelBluetoothProp::CPanelBluetoothProp(
        CPanel *      pPanel,
        CWidgetMenu * pMenu,
        const char *  strName,
        bwin_font_t   font
        ) :
    _pCfg(NULL),
    _pContainer(NULL),
    _pContainerBkgnd(NULL),
    _pConnection(NULL),
    _pName(NULL),
    _pChannel(NULL),
    _pModeA(NULL),
    _pModeB(NULL),
    _pModeG(NULL),
    _pModeN(NULL),
    _pModeAC(NULL),
    _pSignalLevel(NULL),
    _pSecurity(NULL),
    _mode(eWifiMode_Max),
    _bConnected(false),
    _index(-1),
    _pBtDevInfoDiscList(NULL)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pPanel);

    MRect rectMenu = pMenu->getGeometry();
    int   width    = rectMenu.width();
    int   height   = 18;

    _pContainer = new CWidgetListViewContainer(MString(strName)+"Container", pPanel->getEngine(), pPanel, MRect(0, 0, width, height), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pContainer, ret, eRet_OutOfMemory, error);
    _pContainer->setBackgroundFillMode(fill_eGradient);

    _pContainerBkgnd = new CWidgetLabel(MString(strName)+"ContainerBkgnd", pPanel->getEngine(), pPanel, _pContainer->getGeometry(), font, _pContainer->getWin());
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pContainerBkgnd, ret, eRet_OutOfMemory, error);
    _pContainerBkgnd->setBackgroundFillMode(fill_eGradient);
    _pContainerBkgnd->setBevel(0);

    {
        MRect rect          = _pContainerBkgnd->getGeometry();
        MRect rectContainer = _pContainerBkgnd->getGeometry();
        int   margin        = 5;
        int   percentage    = 0;
        int   end           = 0;

        /* connection icon */
        {
            rect.setX(margin / 2);
            rect.setY(0);
            rect.setWidth(20);
            rect.setHeight(20);
            end = rect.x() + rect.width();

            _pConnection = new CWidgetLabel(MString(strName)+"Bluetooth Connection", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pConnection, ret, eRet_OutOfMemory, error);
            _pConnection->setBevel(0);
            _pConnection->loadImage("images/bluetooth-icon_small.png", bwin_image_render_mode_maximize_down);
            _pConnection->show(false);
        }

        /* paired icon */
        {
            rect.setX(margin / 2);
            rect.setY(0);
            rect.setWidth(20);
            rect.setHeight(20);
            end = rect.x() + rect.width();

            _pPaired = new CWidgetLabel(MString(strName)+"Bluetooth Paired", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pConnection, ret, eRet_OutOfMemory, error);
            _pPaired->setBevel(0);
            _pPaired->loadImage("images/bluetooth-icon_transp_small.png", bwin_image_render_mode_maximize_down);
            _pPaired->show(false);
        }
        /*Name */
        {
            percentage = 35;
            rect.setX(end + margin / 2);
            rect.setWidth(((width - margin * 2) * percentage / 100)+66); /* add 66 as i got rid of the modes */
            rect.setHeight(18);
            end = rect.x() + rect.width();

            _pName = new CWidgetButton(MString(strName)+"Name", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate name label widget", _pName, ret, eRet_OutOfMemory, error);
            _pName->setBevel(0);
            _pName->setBackgroundFillMode(fill_eGradient);
            _pContainer->setFocusWidget(_pName);
        }
#if 0

        /* channel */
        {
            percentage = 9;
            rect.setX(end + margin);
            rect.setWidth((width - margin * 2) * percentage / 100);
            end = rect.x() + rect.width();

            _pChannel = new CWidgetLabel(MString(strName)+"Channel", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pChannel, ret, eRet_OutOfMemory, error);
            _pChannel->setBevel(0);
            _pChannel->setBackgroundFillMode(fill_eGradient);
        }

        /* mode A/B/G/N/AC */
        {
            rect.setY(rect.y() + 1);
            rect.setHeight(rect.height() - 2);

            rect.setX(end + margin);
            rect.setWidth(12);
            end = rect.x() + rect.width();

            _pModeA = new CWidgetLabel(MString(strName)+"ModeA", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pModeA, ret, eRet_OutOfMemory, error);
            _pModeA->setBevel(0);
            _pModeA->setText("a", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _pModeA->setTextColor(COLOR_GREY);
            _pModeA->setBackgroundFillMode(fill_eGradient);

            rect.setX(end);
            rect.setWidth(12);
            end = rect.x() + rect.width();

            _pModeB = new CWidgetLabel(MString(strName)+"ModeB", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pModeB, ret, eRet_OutOfMemory, error);
            _pModeB->setBevel(0);
            _pModeB->setText("b", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _pModeB->setTextColor(COLOR_GREY);
            _pModeB->setBackgroundFillMode(fill_eGradient);

            rect.setX(end);
            rect.setWidth(12);
            end = rect.x() + rect.width();

            _pModeG = new CWidgetLabel(MString(strName)+"ModeG", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pModeG, ret, eRet_OutOfMemory, error);
            _pModeG->setBevel(0);
            _pModeG->setText("g", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _pModeG->setTextColor(COLOR_GREY);
            _pModeG->setBackgroundFillMode(fill_eGradient);

            rect.setX(end);
            rect.setWidth(12);
            end = rect.x() + rect.width();

            _pModeN = new CWidgetLabel(MString(strName)+"ModeN", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widet", _pModeN, ret, eRet_OutOfMemory, error);
            _pModeN->setBevel(0);
            _pModeN->setText("n", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _pModeN->setTextColor(COLOR_GREY);
            _pModeN->setBackgroundFillMode(fill_eGradient);

            rect.setX(end);
            rect.setWidth(18);
            end = rect.x() + rect.width();

            _pModeAC = new CWidgetLabel(MString(strName)+"ModeAC", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pModeAC, ret, eRet_OutOfMemory, error);
            _pModeAC->setBevel(0);
            _pModeAC->setText("ac", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _pModeAC->setTextColor(COLOR_GREY);
            _pModeAC->setBackgroundFillMode(fill_eGradient);
        }
#endif /* if 0 */
        /* signal level */
        {
            rect.setY(rect.y() + 5);
            rect.setHeight(rect.height() - 10);

            rect.setX(end + 2 * margin);
            rect.setWidth(40);
            end = rect.x() + rect.width();

            _pSignalLevel = new CWidgetProgress(MString(strName)+"WifiSignalLevel", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pSignalLevel, ret, eRet_OutOfMemory, error);
            _pSignalLevel->setLevel(0);
            _pSignalLevel->setBevel(0);
            _pSignalLevel->setBackgroundFillMode(fill_eGradient);
            _pSignalLevel->setBackgroundGradient(0xFF676767 - 3 * COLOR_STEP, 0xFF676767, 0xFF676767 + 2 * COLOR_STEP);
            _pSignalLevel->showText(false);
            _pSignalLevel->showProgress(true);
        }

        /* security */
        {
            rect.setY(2);
            rect.setHeight(13);

            rect.setX(end + margin);
            rect.setWidth(12);
            end = rect.x() + rect.width();

            _pSecurity = new CWidgetLabel(MString(strName)+"Security", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pSecurity, ret, eRet_OutOfMemory, error);
            _pSecurity->setBevel(0);
            _pSecurity->loadImage("images/lock_small.png", bwin_image_render_mode_maximize_down);
            _pSecurity->show(false);
        }
    }

    _pContainer->setFocusable(false);

    ret = pMenu->addButton(_pContainer, strName);
    CHECK_ERROR_GOTO("unable to add button to menu", ret, error);

error:
    return;
}

CPanelBluetoothProp::~CPanelBluetoothProp()
{
    DEL(_pContainer);
    DEL(_pContainerBkgnd);
    DEL(_pConnection);
    DEL(_pPaired);
    DEL(_pName);
    DEL(_pChannel);
    DEL(_pModeA);
    DEL(_pModeB);
    DEL(_pModeG);
    DEL(_pModeN);
    DEL(_pModeAC);
    DEL(_pSignalLevel);
    DEL(_pSecurity);
}

eRet CPanelBluetoothProp::initialize(CConfiguration * pCfg)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pCfg);
    _pCfg = pCfg;

    return(ret);
}

void CPanelBluetoothProp::setConnected(bool bConnected)
{
    BDBG_ASSERT(NULL != _pConnection);
    _pConnection->show(bConnected);
    _bConnected = bConnected;
}

void CPanelBluetoothProp::setPaired(bool bPaired)
{
    BDBG_ASSERT(NULL != _pSecurity);
    _pPaired->show(bPaired);
    _bPaired = bPaired;
}

void CPanelBluetoothProp::setName(const char * pText)
{
    BDBG_ASSERT(NULL != _pName);
    _pName->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
}

void CPanelBluetoothProp::setAddress(const char * pText)
{
    BDBG_ASSERT(NULL != _pName);
    _pAddress = pText;
}

void CPanelBluetoothProp::setSignalLevel(uint8_t percent)
{
    BDBG_ASSERT(100 >= percent);

    _pSignalLevel->setLevel(PERCENT_TO_UINT16(percent));
    _pSignalLevel->showProgress(true);
    _pSignalLevel->show(true);
}

void CPanelBluetoothProp::setSecurity(bool bSecurity)
{
    _pSecurity->show(bSecurity);
}

/* Updates the Individual  Bluetooth Property */
void CPanelBluetoothProp::update(CBluetoothDevice * pInfo)
{
    {
        MString strAlpha;

        strAlpha = getName();
        strAlpha.lower();

        getContainer()->setText(strAlpha);
    }

    setName(pInfo->getName());
    setIndex(pInfo->getIndex());
    setAddress(pInfo->getAddress());
    if (pInfo->isConnected())
    {
        setPaired(false);
        setConnected(true);
    }
    else
    if (pInfo->isPaired())
    {
        setConnected(false);
        setPaired(true);
    }
    else
    {
        setPaired(false);
        setConnected(false);
    }

    setBtInfoList(pInfo->getBtDevInfoList());
    setBtDevice(pInfo);
    /*setSecurity(pInfo->isPaired()); */

    if (pInfo->getSignalStrength() > -40)
    {
        setSignalLevel(100);
    }
    else
    if (pInfo->getSignalStrength() > -50)
    {
        setSignalLevel(75);
    }
    else
    if (pInfo->getSignalStrength() > -60)
    {
        setSignalLevel(50);
    }
    else
    if (pInfo->getSignalStrength() > -70)
    {
        setSignalLevel(25);
    }
    else
    {
        setSignalLevel(0);
    }
} /* update */

void CPanelBluetoothProp::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("panel_bluetooth", &level);
        BDBG_SetModuleLevel("panel_bluetooth", BDBG_eMsg);
    }

    BDBG_MSG(("Bluetooth Property name:%s index:%d paired:%d Conn:%d", getName().s(), getIndex(), isPaired(), isConnected()));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("panel_bluetooth", level);
    }
} /* dump */

/* comparison operator overload */
bool CPanelBluetoothProp::operator ==(const NETAPP_BT_DEV_INFO & rhs)
{
    bool bMatch = false;

    bMatch = (getName() == rhs.cName && getAddress() == rhs.cAddr);

    return(bMatch);
}

bool CPanelBluetoothProp::operator ==(CBluetoothDevice & rhs)
{
    bool bMatch = false;

    bMatch = (getName() == rhs.getName() && getAddress() == rhs.getAddress());

    return(bMatch);
}

CPanelBluetooth::CPanelBluetooth(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelBluetooth", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pBluetoothMenu(NULL),
    _pPropertiesMenu(NULL),
    _pStatusMenu(NULL),
    _bExpandPanel(false),
    _pExpand(NULL),
    _pHeadingProperties(NULL),
    _pGridTitle(NULL),
    _pGrid(NULL),
    _timerUpdate(pWidgetEngine, this, 500),
    _MsgBoxStatus(NULL),
    _pBluetooth(NULL),
    _pBack(NULL)
{
}

CPanelBluetooth::~CPanelBluetooth()
{
    uninitialize();
}

eRet CPanelBluetooth::initialize(
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

    _pBluetoothMenu = new CWidgetMenu("CPanelBluetooth::_pNetworkWifiMenu", getEngine(), this, MRect(0, 0, 0, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pBluetoothMenu, ret, eRet_OutOfMemory, error);
    _pBluetoothMenu->showSubTitle(false);
    _pBluetoothMenu->showListView(false);
    _pBluetoothMenu->showEdit(false);
    setMenuTitleStatus();
    _pBluetoothMenu->show(true);
    {
        _pPropertiesMenu = new CWidgetMenu("CPanelBluetooth::_pPropertiesMenu", getEngine(), _pBluetoothMenu, MRect(0, 0, MENU_WIDTH, 200), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPropertiesMenu, ret, eRet_OutOfMemory, error);
        _pPropertiesMenu->showMenuBar(false);
        _pPropertiesMenu->showEdit(false);
        _pPropertiesMenu->setScroll(true);

        _pHeadingProperties = new CWidgetLabel("CPanelBluetooth::_pHeadingProperties", getEngine(), _pBluetoothMenu, MRect(0, 36, MENU_WIDTH, 20), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate heading widget", _pHeadingProperties, ret, eRet_OutOfMemory, error);
        _pHeadingProperties->setBackgroundColor(_pPropertiesMenu->getBackgroundColor());
        _pHeadingProperties->setTextColor(0xFF80C42F);
        _pHeadingProperties->setBevel(0);
        _pHeadingProperties->setZOrder(2);
        _pHeadingProperties->setText("  NAME                             Ch    Modes         Signal",
                bwidget_justify_horiz_left, bwidget_justify_vert_bottom);

        /* create max number of buttons - we will create/add them once, then
         * activate/deactivate them from the CWidgetMenu as needed. */
        for (int i = 0; i < MAX_BLUETOOTH_PROPS; i++)
        {
            CPanelBluetoothProp * pProp = NULL;
            pProp = new CPanelBluetoothProp(this, _pPropertiesMenu, MString(i).s(), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate Bluetooth property", pProp, ret, eRet_OutOfMemory, error);

            pProp->initialize(_pCfg);

            /* start buttons as inactive - layout() will activate as needed */
            activateProp(pProp, false);

            /* keep track of all added buttons */
            _propList.add(pProp);
        }

        /* expandable status menu */
        _pStatusMenu = new CWidgetMenu("CPanelBluetooth::_pStatusMenu", getEngine(), _pBluetoothMenu, MRect(0, 0, MENU_WIDTH, 200), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pStatusMenu, ret, eRet_OutOfMemory, error);
        _pStatusMenu->showMenuBar(false);
        _pStatusMenu->showEdit(false);
        _pStatusMenu->setMenuTitle(NULL, "Device Status");
        _pStatusMenu->setScroll(true);

        /* create double labels for tag/value in connection status
         *        our window is big enough to hold them all so no need to scroll */
        for (int i = 0; i < MAX_CONNECTION_STATUS; i++)
        {
            CPanelBluetoothStatus * pStatus = NULL;
            pStatus = new CPanelBluetoothStatus(this, _pStatusMenu, MString(i).s(), font12, 35);
            CHECK_PTR_ERROR_GOTO("unable to allocate Bluetooth status", pStatus, ret, eRet_OutOfMemory, error);

            /* start buttons as inactive - layout() will activate as needed */
            _pStatusMenu->setListButtonActive(pStatus->getContainer(), false);
            /* keep track of all added buttons */
            _statusList.add(pStatus);
        }

        /* expand button */
        _pExpand = new CWidgetButton("CPanelBluetooth::_pExpand", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pExpand, ret, eRet_OutOfMemory, error);
        _pBluetoothMenu->addExpandButton(_pExpand);
        /* default expand panel open if thumbnail decode is available */
        _bExpandPanel = (MAX_BLUETOOTH_PROPS != getConnectedIndex()) ? true : false;

        /* back button */
        _pBack = new CWidgetButton("CPanelBluetooth::_pBack", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pBack, ret, eRet_OutOfMemory, error);
        _pBack->setText("Menu");
        _pBluetoothMenu->addBackButton(_pBack);

        {
            MRect    geom;
            uint32_t msgBoxWidth  = 340;
            uint32_t msgBoxHeight = 100;

            /* center msg box on screen - note that we will set the msg box bwin parent to be the framebuffer
             * so it can draw outside the borders of it's parent widget (this) */
            geom.setX(graphicsWidth / 2 - msgBoxWidth / 2);
            geom.setY(graphicsHeight / 2 - msgBoxHeight / 2);
            geom.setWidth(msgBoxWidth);
            geom.setHeight(msgBoxHeight);

            _MsgBoxStatus = new CWidgetModalMsgBox("CPanelBluetooth::_MsgBox", getEngine(), this, geom, font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate MsgBox widget", _MsgBoxStatus, ret, eRet_OutOfMemory, error);
            _MsgBoxStatus->setText("Sample Text", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _MsgBoxStatus->show(false);
        }

        _pBack->setFocus();
    }

    clear();
    layout();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelBluetooth::uninitialize()
{
    DEL(_MsgBoxStatus);
    DEL(_pBack);
    DEL(_pHeadingProperties);
    DEL(_pExpand);
    DEL(_pStatusMenu);
    DEL(_pPropertiesMenu);
    DEL(_pBluetoothMenu);
}

#define GRID_WIDTH   150
#define GRID_HEIGHT  150

void CPanelBluetooth::layout()
{
    MRect rectMenu(0, 0, MENU_WIDTH, MENU_HEIGHT);
    MRect rectPanel   = getGeometry();
    MRect rectProps   = _pPropertiesMenu->getGeometry();
    MRect rectStatus  = _pPropertiesMenu->getGeometry();
    MRect rectHeading = _pHeadingProperties->getGeometry();

    /* layout properties menu */
    rectProps.setX(0);
    rectProps.setY(30);
    rectProps.setWidth(rectMenu.width());
    rectProps.setHeight((rectMenu.height() - 30));
    _pPropertiesMenu->setGeometry(rectProps);

    /* layout connection status menu */
    rectStatus = rectProps;
    rectStatus.setX(rectProps.x() + rectProps.width());
    rectStatus.setWidth(rectProps.width() * 4 / 5); /* 4/5 size of properties menu */
    _pStatusMenu->setGeometry(rectStatus);

    /* combine properties and status menus to determine panel size */
    rectMenu.setX(0);
    rectMenu.setY(0);

    /* adjust panel size to fit */
    rectPanel.setHeight(MENU_HEIGHT);
    if (true == _bExpandPanel)
    {
        rectMenu.setWidth(rectStatus.x() + rectStatus.width());
        rectPanel.setWidth(rectStatus.x() + rectStatus.width());
    }
    else
    {
        rectMenu.setWidth(MENU_WIDTH);
        rectPanel.setWidth(MENU_WIDTH);
    }
    _pBluetoothMenu->setGeometry(rectMenu);
    setGeometry(rectPanel);

    _pExpand->setText(_bExpandPanel ? "<<" : ">>");
} /* layout */

eRet CPanelBluetooth::connect(CPanelBluetoothProp * pBluetoothProp)
{
    eRet    ret = eRet_Ok;
    MString strPassword;

    BDBG_ASSERT(NULL != pBluetoothProp);

    if (true == pBluetoothProp->isSecure())
    {
        /* get bluetooth  password from user */
#if 0
        strPassword = getScreenMain()->showKeyboardModal(pBluetoothProp->getName(), "Enter Bluetooth Password");
        BDBG_WRN(("TTTTTTTTTTTTTTTTTTTTT password entry:%s", strPassword.s()));
#endif
    }

    {
        CBluetoothConnectionData dataConnect;
        BDBG_MSG(("%s():connecting and the index_to_connect %d", BSTD_FUNCTION, pBluetoothProp->getIndex()));
        dataConnect.index            = pBluetoothProp->getIndex();
        dataConnect.pDevInfoList     = pBluetoothProp->getBtInfoList();
        dataConnect.pBluetoothDevice = pBluetoothProp->getBtDevice();
        notifyObservers(eNotify_BluetoothConnect, &dataConnect);
    }
    return(ret);
} /* connect */

eRet CPanelBluetooth::disconnect(CPanelBluetoothProp * pBluetoothProp)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pBluetoothProp);

    {
        CBluetoothConnectionData dataConnect;
        dataConnect.index            = pBluetoothProp->getIndex();
        dataConnect.pDevInfoList     = pBluetoothProp->getBtInfoList();
        dataConnect.pBluetoothDevice = pBluetoothProp->getBtDevice();
        notifyObservers(eNotify_BluetoothDisconnect, &dataConnect);
    }
    return(ret);
}

/* returns MAX_BLUETOOTH_PROPS if NOT connected, otherwise returns index of connected property */
int CPanelBluetooth::getConnectedIndex()
{
    int i = 0;

    for (i = 0; i < MAX_BLUETOOTH_PROPS; i++)
    {
        if (true == _propList[i]->isConnected())
        {
            break;
        }
    }

    return(i);
}

void CPanelBluetooth::onFocus(bwidget_t widget)
{
    MListItr <CPanelBluetoothProp> itr_prop(&_propList);
    CPanelBluetoothProp *          pProp = NULL;
    for (pProp = itr_prop.first(); pProp; pProp = itr_prop.next())
    {
        if (pProp->getNameButton()->getWidget() == widget)
        {
            updateStatus(pProp->getBtDevice());
            break;
        }
    }

    /* not on a property button, clear status window */
    if (!pProp)
    {
        updateStatus(NULL);
    }
} /* onFocus */

void CPanelBluetooth::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    /* look for match to a selected bluetooth button */
    for (int i = 0; i < MAX_BLUETOOTH_PROPS; i++)
    {
        if (_propList[i]->getNameButton() == pWidget)
        {
            eRet ret = eRet_Ok;

            if ((false == _propList[i]->isConnected()) && (false == _propList[i]->isPaired()))
            {
                BDBG_MSG(("%s():On click connect name %s conencted %d  paired %d index  %d ", BSTD_FUNCTION,
                          _propList[i]->getName().s(), _propList[i]->isConnected(), _propList[i]->isPaired(), _propList[i]->getIndex()));

                ret = connect(_propList[i]);
                CHECK_ERROR_GOTO("unable to connect to bluetooth device", ret, error);
            }
            else
            {
                /* change to unpair  todo */
                BDBG_MSG(("%s():On click disconnect name %s connected %d  paired %d ", BSTD_FUNCTION,
                          _propList[i]->getName().s(), _propList[i]->isConnected(), _propList[i]->isPaired()));
                if (false == _MsgBoxStatus->isVisible())
                {
                    _MsgBoxStatus->showModal("Are you sure you want to unpair Bluetooth device", 1, "Ok", "Cancel", NULL);
                    if (_MsgBoxStatus->getResult() == "Ok")
                    {
                        _pBack->setFocus();
                        ret = disconnect(_propList[i]);
                        CHECK_ERROR_GOTO("unable to disconnect from bluetooth device", ret, error);
                    }
                    else
                    {
                        _propList[i]->getNameButton()->setFocus();
                    }
                }
            }
            goto done;
        }
    }

    if (_pExpand == pWidget)
    {
        expand(!_bExpandPanel);
    }
    else
    if (_pBack == pWidget)
    {
        show(false);
        getParent()->show(true);
    }

error:
done:
    return;
} /* onClick */

void CPanelBluetooth::expand(bool bExpand)
{
    _bExpandPanel = bExpand;
    _pExpand->setText(_bExpandPanel ? "<<" : ">>");
    layout();
}

void CPanelBluetooth::startUpdateTimers(bool bStart)
{
    if (true == bStart)
    {
        _timerUpdate.stop();
        _timerUpdate.start(GET_INT(_pCfg, UI_BLUETOOTH_UPDATE_TIMEOUT));
    }
    else
    {
        _timerUpdate.stop();
    }
} /* startUpdateTimers */

void CPanelBluetooth::activateProp(
        CPanelBluetoothProp * pProp,
        bool                  bActivate
        )
{
    if (bActivate != isPropActive(pProp))
    {
        BDBG_MSG(("set list button(%s) active(%d)", pProp->getName().s(), bActivate));
        _pPropertiesMenu->setListButtonActive(pProp->getContainer(), bActivate);
    }
}

bool CPanelBluetooth::isPropActive(CPanelBluetoothProp * pProp)
{
    return(_pPropertiesMenu->isListButtonActive(pProp->getContainer()));
}

void CPanelBluetooth::activateStatus(
        CPanelBluetoothStatus * pStatus,
        bool                    bActivate
        )
{
    if (bActivate != isStatusActive(pStatus))
    {
        BDBG_MSG(("set list button(%s) active(%d)", pStatus->getTag().s(), bActivate));
        _pStatusMenu->setListButtonActive(pStatus->getContainer(), bActivate);
    }
}

bool CPanelBluetooth::isStatusActive(CPanelBluetoothStatus * pStatus)
{
    return(_pStatusMenu->isListButtonActive(pStatus->getContainer()));
}

void CPanelBluetooth::clear()
{
    {
        MListItr <CPanelBluetoothProp> itr(&_propList);
        CPanelBluetoothProp *          pProp = NULL;

        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            pProp->setName("");
            activateProp(pProp, false);
        }
    }
    {
        MListItr <CPanelBluetoothStatus> itr(&_statusList);
        CPanelBluetoothStatus *          pStatus = NULL;

        for (pStatus = itr.first(); pStatus; pStatus = itr.next())
        {
            activateStatus(pStatus, false);
        }
    }
} /* clear */

void CPanelBluetooth::scan(bool bStart)
{
    notifyObservers((true == bStart) ? eNotify_BluetoothDiscoveryStart : eNotify_BluetoothDiscoveryStop);
}

void CPanelBluetooth::show(bool bShow)
{
    CPanel::show(bShow);

    if (true == bShow)
    {
        _pBack->setFocus();
        clear();
    }

    scan(bShow);
}

void CPanelBluetooth::clearConnectionStatus()
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    CPanelBluetoothProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        pProp->setConnected(false);
    }
}

CPanelBluetoothProp * CPanelBluetooth::findProp(const char * strName)
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    CPanelBluetoothProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getName() == strName)
        {
            break;
        }
    }

    return(pProp);
}

void CPanelBluetooth::setMenuTitleStatus(const char * pStr)
{
    MString strText = "Bluetooth Devices";

    if (NULL != pStr)
    {
        strText += " - ";
        strText += pStr;
    }
    _pBluetoothMenu->setMenuTitle(strText.s(), NULL);
}

void CPanelBluetooth::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerUpdate == pTimer)
        {
            BDBG_MSG(("%s():bluetooth timeout:!!!", BSTD_FUNCTION));
            scan();
        }
    }
    break;

    case eNotify_BluetoothDiscoveryStarted:
    {
        CBluetooth * pBluetooth = (CBluetooth *)notification.getData();
        BDBG_MSG(("%s(): bluetooth Discovery Started", BSTD_FUNCTION));
        setMenuTitleStatus("Discovering...");
        updatePropertyList(pBluetooth);
    }
    break;

    case eNotify_BluetoothDiscoveryStopped:
        setMenuTitleStatus();
        break;

    case eNotify_BluetoothDiscoveryResult:
    {
        CBluetooth * pBluetooth = (CBluetooth *)notification.getData();

        BDBG_MSG(("%s(): bluetooth Discovery Result", BSTD_FUNCTION));
        setMenuTitleStatus();
        updatePropertyList(pBluetooth);
        if (true == isVisible())
        {
            startUpdateTimers(true);
        }
    }
    break;

    case eNotify_BluetoothConnectionDone:
    {
        CBluetooth * pBluetooth = (CBluetooth *)notification.getData();
        BDBG_MSG(("%s():bluetooth Connection Done:", BSTD_FUNCTION));

        setMenuTitleStatus();
        updatePropertyList(pBluetooth);

        if (true == isVisible())
        {
            startUpdateTimers(true);
        }
    }
    break;

    case eNotify_BluetoothConnectionStatus:
    {
        CBluetooth * pBluetooth = (CBluetooth *)notification.getData();

        setMenuTitleStatus();
        _MsgBoxStatus->cancelModal("");

        /* Case where errors out cause you are connecting to invalid bt device */
        if (pBluetooth->getConnectStatus() == eConnectStatus_Disconnecting)
        {
            if (pBluetooth->getConnectedCount())
            {
                pBluetooth->setConnectStatus(eConnectStatus_Connected);
            }
            _MsgBoxStatus->showModal("Can't connect to this type of Device", 0, "Ok", NULL, NULL);
            _pExpand->setFocus();
        }

        /* Check if there  are no  connected devices  */
        if (pBluetooth->getConnectedCount() == 0)
        {
            clearConnectionStatus();
        }

        BDBG_MSG(("bluetooth connection status:%d", pBluetooth->getConnectStatus()));
        switch (pBluetooth->getConnectStatus())
        {
        case eConnectStatus_Connecting:
            BDBG_MSG(("%s():bluetooth Connection status: ConnectING!!!", BSTD_FUNCTION));

            if (false == _MsgBoxStatus->isVisible())
            {
                _MsgBoxStatus->showModal("Connecting to Bluetooth Device", -1, NULL, NULL, NULL);
            }
            break;

        case eConnectStatus_Connected:
        {
            CBluetooth * pBluetooth = (CBluetooth *)notification.getData();
            BDBG_MSG(("%s():bluetooth Connection status: ConnectED!!!", BSTD_FUNCTION));

            /* Sync up the Bluetooth Device list with the Properties */
            updatePropertyList(pBluetooth); /* should update the ones that are connected */
        }
        break;

        case eConnectStatus_Disconnected:
        {
            CBluetooth * pBluetooth = (CBluetooth *)notification.getData();
            /* connection failed */
            BDBG_MSG(("bluetooth connection disconnected"));

            updatePropertyList(pBluetooth); /* should update the ones that are disconnected/unpaired */

            /* expand panel based on connection status */
            expand(false);
        }
        break;

        default:
            break;
        }   /* switch */
    }
    break;

    default:
        break;
    } /* switch */

    return;
} /* processNotification */

CPanelBluetoothProp * CPanelBluetooth::findEmptyProp()
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    CPanelBluetoothProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getName().isEmpty())
        {
            /* found empty prop in list */
            break;
        }
    }

    return(pProp);
}

/* Input BluetoothDevice  and outputs its stats onto the device status panel */
void CPanelBluetooth::updateStatus(CBluetoothDevice * pBluetoothDevice)
{
    eRet                    ret = eRet_Ok;
    MStringHash             status;
    CPanelBluetoothStatus * pStatus = NULL;

    MListItr <CPanelBluetoothStatus> itr(&_statusList);

    /* find the matchind bluetood device */
    if (pBluetoothDevice != NULL)
    {
        /*   update device status */
        ret = pBluetoothDevice->getStatusList(&status);
        CHECK_ERROR_GOTO("unable to get status list", ret, error);

        if (true == status.first())
        {
            pStatus = itr.first();
            for (status.first(); status.current(); status.next())
            {
                if (NULL != pStatus)
                {
                    pStatus->setTag(status.name() + ":");
                    pStatus->setValue(status.value());

                    /* note we do not use the button's show() method here since that is
                     * manipulated by the control itself when scrolling.  the setListButtonActive()
                     * method of the widget menu allows us to hide/show buttons without
                     * removing them from the control and still allows scrolling to work
                     * properly */
                    _pStatusMenu->setListButtonActive(pStatus->getContainer(), true);

                    pStatus = itr.next();
                }
                else
                {
                    BDBG_WRN(("Too many status to display - increase size of network status list"));
                }
            }
        }
    }
    else
    {
        /* disconnected so clear status */
        pStatus = itr.first();
    }

error:
    /* hide any remaining unused buttons */
    while (NULL != pStatus)
    {
        _pStatusMenu->setListButtonActive(pStatus->getContainer(), false);
        pStatus = itr.next();
    }
} /* updateStatus */

/* hook what is in the bluetooth device list to the properties */
void CPanelBluetooth::updatePropertyList(CBluetooth * pBluetooth)
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    bool                  foundMatch  = false;
    CPanelBluetoothProp * pProp       = NULL;
    bwidget_t             widgetFocus = getFocus();

    BDBG_ASSERT(NULL != pBluetooth);

    MListItr <CBluetoothDevice> itr_device(pBluetooth->getBluetoothDeviceList());
    CBluetoothDevice *          pBluetoothDevice = NULL;

    for (pBluetoothDevice = itr_device.first(); pBluetoothDevice; pBluetoothDevice = itr_device.next())
    {
        /* search property list for a match */
        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            if (*pProp == *pBluetoothDevice)
            {
                /* network matches existing property in list (Name and) so just update data */
                BDBG_MSG(("updating property: name(%s) index (%d) paired (%d)  connected (%d)", pBluetoothDevice->getName(), pBluetoothDevice->getIndex(),
                          pBluetoothDevice->isPaired(), pBluetoothDevice->isConnected()
                          ));
                pProp->update(pBluetoothDevice);
                break;
            }
        }

        if (NULL == pProp)
        {
            /* didn't find match in property list so create a new one */
            CPanelBluetoothProp * pPropNew = NULL;

            pPropNew = findEmptyProp();
            if (NULL == pPropNew)
            {
                BDBG_WRN(("unable to add new bluetooth network - increase MAX_NETWORK_WIFI_PROPS"));
                break;
            }
            else
            {
                /* add new property */
                BDBG_MSG(("adding new property: name(%s) index (%d) paired (%d)  connected (%d)", pBluetoothDevice->getName(), pBluetoothDevice->getIndex(),
                          pBluetoothDevice->isPaired(), pBluetoothDevice->isConnected()
                          ));
                pPropNew->update(pBluetoothDevice);
            }
        }
    }

    /*  go through and delete any stale properties  */
    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getAddress() == "")
        {
            continue;
        }

        foundMatch = false;
        /* Search through discovery list for this property */
        for (pBluetoothDevice = itr_device.first(); pBluetoothDevice; pBluetoothDevice = itr_device.next())
        {
            BDBG_MSG((" Prop %s(%s)  BluetoothDevice %s(%s)", pProp->getName().s(), pProp->getAddress().s(),
                      pBluetoothDevice->getName(), pBluetoothDevice->getAddress()));

            if (*pProp == *pBluetoothDevice)
            {
                /* enable property on screen */
                BDBG_MSG(("Activate Prop %s", pProp->getName().s()));
                activateProp(pProp, true);
                foundMatch = true;

                break;
            }
        }

        /* didn't find match else you did and go to next property */
        if (foundMatch == false)
        {
            if (pProp->getNameButton()->getWidget() == widgetFocus)
            {
                /* we are removing the currently focused widget so move focus before
                 *              deactivating the widget */
                _pBack->setFocus();
            }

            /* pProp not found in list of networks - deactivate it.
             *           this will remove it from the on screen list */
            pProp->setName("");
            pProp->setAddress("");
            pProp->setPaired(false);
            pProp->setConnected(false);
            activateProp(pProp, false);
        }
    }
    /* Already sorted in by Saved then Discovery, then by RSSI strength
     *  ret = _pPropertiesMenu->sortListView();
     *  CHECK_ERROR("Unable to sort properties menu", ret);
     */
    /*dump(true); */

    return;
} /* updatePropertyList */

void CPanelBluetooth::dump(bool bForce)
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    CPanelBluetoothProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        pProp->dump(bForce);
    }
}

void CPanelBluetooth::fakeList()
{
    MListItr <CPanelBluetoothProp> itr(&_propList);
    CPanelBluetoothProp *          pProp = NULL;

    pProp = itr.first();
    BDBG_ASSERT(NULL != pProp);

    /* add properties to list widget */
    {
        pProp->setName("my network is cool");
        pProp->setSignalLevel(75);
        pProp->setSecurity(true);

        /* note we do not use the button's show() method here since that is
         * manipulated by the control itself when scrolling. */
        activateProp(pProp, true);

        pProp = itr.next();
    }
    {
        pProp->setConnected(true);
        pProp->setName("my second network");
        pProp->setSignalLevel(50);
        pProp->setSecurity(true);

        /* note we do not use the button's show() method here since that is
         * manipulated by the control itself when scrolling. */
        activateProp(pProp, true);

        pProp = itr.next();
    }
    {
        pProp->setName("my last network");
        pProp->setSignalLevel(25);
        pProp->setSecurity(false);

        /* note we do not use the button's show() method here since that is
         * manipulated by the control itself when scrolling. */
        activateProp(pProp, true);

        pProp = itr.next();
    }

    /* hide any remaining unused buttons */
    while (NULL != pProp)
    {
        activateProp(pProp, false);
        pProp = itr.next();
    }

    /*
     * TTTTTTTTTTTTTTTTTTTTTTTT not sure if we have to layout() every time
     * layout();
     */
} /* layout */

#endif /* ifdef NETAPP_SUPPORT */