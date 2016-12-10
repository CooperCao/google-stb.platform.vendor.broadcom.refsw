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

#define MAX_NETWORK_WIFI_PROPS  255
#define MAX_CONNECTION_STATUS   16
#define MENU_WIDTH              340
#define MENU_HEIGHT             355

#include "channel.h"
#include "convert.h"
#include "screen_main.h"
#include "panel_network_wifi.h"
#include "panel_keyboard.h"
#include "widget_grid.h"
#include "network.h"
#ifdef WPA_SUPPLICANT_SUPPORT
#include "wifi.h"
#endif

BDBG_MODULE(panel_network_wifi);

#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)

CPanelNetworkWifiProp::CPanelNetworkWifiProp(
        CPanel *      pPanel,
        CWidgetMenu * pMenu,
        const char *  strName,
        bwin_font_t   font
        ) :
    _pCfg(NULL),
    _pContainer(NULL),
    _pContainerBkgnd(NULL),
    _pConnection(NULL),
    _pSsid(NULL),
    _pChannel(NULL),
    _pModeA(NULL),
    _pModeB(NULL),
    _pModeG(NULL),
    _pModeN(NULL),
    _pModeAC(NULL),
    _pSignalLevel(NULL),
    _pSecurity(NULL),
    _mode(eWifiMode_Max),
    _bConnected(false)
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

            _pConnection = new CWidgetLabel(MString(strName)+"Wifi Connection", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pConnection, ret, eRet_OutOfMemory, error);
            _pConnection->setBevel(0);
            _pConnection->loadImage("images/wifi_small.png", bwin_image_render_mode_maximize_down);
            _pConnection->show(false);
        }

        /* ssid */
        {
            percentage = 35;
            rect.setX(end + margin / 2);
            rect.setWidth((width - margin * 2) * percentage / 100);
            rect.setHeight(18);
            end = rect.x() + rect.width();

            _pSsid = new CWidgetButton(MString(strName)+"Ssid", pPanel->getEngine(), pPanel, rect, font, _pContainerBkgnd->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate ssid label widget", _pSsid, ret, eRet_OutOfMemory, error);
            _pSsid->setBevel(0);
            _pSsid->setBackgroundFillMode(fill_eGradient);
            _pContainer->setFocusWidget(_pSsid);
        }

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
        }
    }

    _pContainer->setFocusable(false);

    ret = pMenu->addButton(_pContainer, strName);
    CHECK_ERROR_GOTO("unable to add button to menu", ret, error);

    /* set up wifi mode enum to mode label mapping */
    _pModeToLabel[eWifiMode_A]   = _pModeA;
    _pModeToLabel[eWifiMode_B]   = _pModeB;
    _pModeToLabel[eWifiMode_G]   = _pModeG;
    _pModeToLabel[eWifiMode_N]   = _pModeN;
    _pModeToLabel[eWifiMode_AC]  = _pModeAC;
    _pModeToLabel[eWifiMode_Max] = NULL;

    /* clear wifi mode */
    setMode(eWifiMode_Max);
error:
    return;
}

CPanelNetworkWifiProp::~CPanelNetworkWifiProp()
{
    DEL(_pContainer);
    DEL(_pContainerBkgnd);
    DEL(_pConnection);
    DEL(_pSsid);
    DEL(_pChannel);
    DEL(_pModeA);
    DEL(_pModeB);
    DEL(_pModeG);
    DEL(_pModeN);
    DEL(_pModeAC);
    DEL(_pSignalLevel);
    DEL(_pSecurity);
}

eRet CPanelNetworkWifiProp::initialize(CConfiguration * pCfg)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pCfg);
    _pCfg = pCfg;

    return(ret);
}

void CPanelNetworkWifiProp::setConnected(bool bConnected)
{
    BDBG_ASSERT(NULL != _pConnection);
    _pConnection->show(bConnected);
    _bConnected = bConnected;
}

void CPanelNetworkWifiProp::setSsid(const char * pText)
{
    BDBG_ASSERT(NULL != _pSsid);
    _pSsid->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
}

void CPanelNetworkWifiProp::setChannel(const char * pText)
{
    BDBG_ASSERT(NULL != _pChannel);
    _pChannel->setText(pText, bwidget_justify_horiz_right, bwidget_justify_vert_middle);
}

void CPanelNetworkWifiProp::setMode(eWifiMode mode)
{
    CWidgetLabel * pLabelMode = _pModeToLabel[mode];
    int            i          = 0;

    _mode = mode;

    /* set the given mode widget to enable gradient */
    if (eWifiMode_Max != mode)
    {
        switch (mode)
        {
        case eWifiMode_A:
            pLabelMode->setBackgroundGradient(COLOR_ORANGE_LIGHT, COLOR_ORANGE, COLOR_ORANGE_DARK);
            pLabelMode->setTextColor(COLOR_GREY_DARK);
            break;
        case eWifiMode_B:
            pLabelMode->setBackgroundGradient(COLOR_BLUE_DARK2_LIGHT, COLOR_BLUE_DARK2, COLOR_BLUE_DARK2_DARK);
            pLabelMode->setTextColor(COLOR_EGGSHELL);
            break;
        case eWifiMode_G:
            pLabelMode->setBackgroundGradient(COLOR_LIME_LIGHT, COLOR_LIME, COLOR_LIME_DARK);
            pLabelMode->setTextColor(COLOR_GREY_DARK);
            break;
        case eWifiMode_N:
            pLabelMode->setBackgroundGradient(COLOR_MAGENTA_LIGHT, COLOR_MAGENTA, COLOR_MAGENTA_DARK);
            pLabelMode->setTextColor(COLOR_EGGSHELL);
            break;
        case eWifiMode_AC:
            pLabelMode->setBackgroundGradient(COLOR_RED_LIGHT, COLOR_RED, COLOR_RED_DARK);
            pLabelMode->setTextColor(COLOR_EGGSHELL);
            break;
        default:
            pLabelMode->setBackgroundGradient(COLOR_EGGSHELL + 2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 4 * COLOR_STEP);
            pLabelMode->setTextColor(COLOR_GREY);
            break;
        } /* switch */
    }
    else
    {
        /* set all mode widgets to disable gradient */
        for (i = eWifiMode_A; i < eWifiMode_Max; i++)
        {
            _pModeToLabel[i]->setBackgroundGradient(COLOR_EGGSHELL + 2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 4 * COLOR_STEP);
            _pModeToLabel[i]->setTextColor(COLOR_GREY);
        }
    }
} /* setMode */

void CPanelNetworkWifiProp::setSignalLevel(uint8_t percent)
{
    BDBG_ASSERT(100 >= percent);

    _pSignalLevel->setLevel(PERCENT_TO_UINT16(percent));
    _pSignalLevel->showProgress(true);
    _pSignalLevel->show(true);
}

void CPanelNetworkWifiProp::setSecurity(bool bSecurity)
{
    _pSecurity->show(bSecurity);
}

#ifdef WPA_SUPPLICANT_SUPPORT
void CPanelNetworkWifiProp::update(CNetworkWifi * pNetwork)
{
    uint32_t signalToNoise = MIN(ABS(pNetwork->_signalToNoise) * 100 / 40, 100);

    BDBG_ASSERT(NULL != pNetwork);

    setBssid(pNetwork->_strBSSID);
    setSsid(pNetwork->_strSSID);
    setChannel(pNetwork->getChannelNum());
    setSecurity(pNetwork->hasSecurity());
    setSignalLevel(signalToNoise);

    /* set container text for sorting - we will sort based on
     * signal strength and ssid. lowercase will be forced to allow
     * mixed case alpha sort */
    {
        MString strAlpha;
        char    strNumber[8];

        /* set container name to be signalToNoise combined with SSID.
         * we will subtract the SNR from 100 so that the higher SNR fields
         * are smaller and the lower SNR fields are bigger.  our default
         * sort works from low to high so this will invert it.
         */
        snprintf(strNumber, 5, "%04d", 100-signalToNoise);
        strAlpha  = strNumber;
        strAlpha += getSsid();
        strAlpha.lower();

        getContainer()->setText(strAlpha);
    }

#if 0
    setMode(eWifiMode_Max);
    if (pNetwork->tMode & NETAPP_WIFI_802_11_MODE_A) { setMode(eWifiMode_A); }
    if (pNetwork->tMode & NETAPP_WIFI_802_11_MODE_B) { setMode(eWifiMode_B); }
    if (pNetwork->tMode & NETAPP_WIFI_802_11_MODE_G) { setMode(eWifiMode_G); }
    if (pNetwork->tMode & NETAPP_WIFI_802_11_MODE_N) { setMode(eWifiMode_N); }
    if (pNetwork->tMode & NETAPP_WIFI_802_11_MODE_AC) { setMode(eWifiMode_AC); }
#endif /* if 0 */
} /* update */

/* comparison operator overload */
bool CPanelNetworkWifiProp::operator ==(CNetworkWifi & rhs)
{
    bool bMatch = false;

    bMatch = (getBssid() == rhs._strBSSID);

    return(bMatch);
}

#elif NETAPP_SUPPORT
void CPanelNetworkWifiProp::update(NETAPP_WIFI_AP_INFO * pInfo)
{
    setSsid(pInfo->cSSID);
    setChannel(MString(pInfo->ulChannel));

    /* set container text for sorting - we will sort based on
     * ssid and channel number.  note that channel number must
     * be zero-prefixed for the string based sort comparison
     * to work properly. lowercase will be forced to allow
     * mixed case alpha sort */
    {
        MString strAlpha;
        char    strNumber[8];

        strAlpha = getSsid();
        snprintf(strNumber, 5, "%04d", getChannel());
        strAlpha += strNumber;
        strAlpha.lower();

        getContainer()->setText(strAlpha);
    }

    setMode(eWifiMode_Max);
    if (pInfo->tMode & NETAPP_WIFI_802_11_MODE_A) { setMode(eWifiMode_A); }
    if (pInfo->tMode & NETAPP_WIFI_802_11_MODE_B) { setMode(eWifiMode_B); }
    if (pInfo->tMode & NETAPP_WIFI_802_11_MODE_G) { setMode(eWifiMode_G); }
    if (pInfo->tMode & NETAPP_WIFI_802_11_MODE_N) { setMode(eWifiMode_N); }
    if (pInfo->tMode & NETAPP_WIFI_802_11_MODE_AC) { setMode(eWifiMode_AC); }

    if ((NETAPP_WIFI_SECURITY_NONE == pInfo->tSecurity) ||
        (NETAPP_WIFI_SECURITY_NOT_SUPPORTED == pInfo->tSecurity))
    {
        setSecurity(false);
    }
    else
    {
        setSecurity(true);
    }

    setSignalLevel(25 * pInfo->tRSSI);
} /* update */

/* comparison operator overload */
bool CPanelNetworkWifiProp::operator ==(const NETAPP_WIFI_AP_INFO & rhs)
{
    bool bMatch = false;

    bMatch = ((getSsid() == rhs.cSSID) && (getChannel() == (int)rhs.ulChannel));

    return(bMatch);
}

#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

void CPanelNetworkWifiProp::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("panel_network_wifi", &level);
        BDBG_SetModuleLevel("panel_network_wifi", BDBG_eMsg);
    }

    BDBG_MSG(("Network Wifi Property ssid:%s channel:%d mode:%d secure:%d", getSsid().s(), getChannel(), getMode(), isSecure()));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("panel_network_wifi", level);
    }
} /* dump */

CPanelNetworkWifi::CPanelNetworkWifi(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelNetworkWifi", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pNetworkWifiMenu(NULL),
    _pPropertiesMenu(NULL),
    _pStatusMenu(NULL),
    _bExpandPanel(false),
    _pExpand(NULL),
    _pHeadingProperties(NULL),
    _pGridTitle(NULL),
    _pGrid(NULL),
    _timerCloseMsgBox(pWidgetEngine, this, 3000),
    _timerUpdate(pWidgetEngine, this, 500),
    _timerScan(pWidgetEngine, this, 10000),
    _MsgBoxStatus(NULL),
#ifdef NETAPP_SUPPORT
    _pNetwork(NULL),
#endif
    _pBack(NULL)
{
}

CPanelNetworkWifi::~CPanelNetworkWifi()
{
    uninitialize();
}

eRet CPanelNetworkWifi::initialize(
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

    _pNetworkWifiMenu = new CWidgetMenu("CPanelNetworkWifi::_pNetworkWifiMenu", getEngine(), this, MRect(0, 0, 0, 0), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pNetworkWifiMenu, ret, eRet_OutOfMemory, error);
    _pNetworkWifiMenu->showSubTitle(false);
    _pNetworkWifiMenu->showListView(false);
    _pNetworkWifiMenu->showEdit(false);
    setMenuTitleStatus();
    _pNetworkWifiMenu->show(true);
    {
        _pPropertiesMenu = new CWidgetMenu("CPanelNetworkWifi::_pPropertiesMenu", getEngine(), _pNetworkWifiMenu, MRect(0, 0, MENU_WIDTH, 200), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pPropertiesMenu, ret, eRet_OutOfMemory, error);
        _pPropertiesMenu->showMenuBar(false);
        _pPropertiesMenu->showEdit(false);
        _pPropertiesMenu->setScroll(true);

        _pHeadingProperties = new CWidgetLabel("CPanelNetworkWifi::_pHeadingProperties", getEngine(), _pNetworkWifiMenu, MRect(0, 36, MENU_WIDTH, 20), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate heading widget", _pHeadingProperties, ret, eRet_OutOfMemory, error);
        _pHeadingProperties->setBackgroundColor(_pPropertiesMenu->getBackgroundColor());
        _pHeadingProperties->setTextColor(0xFF80C42F);
        _pHeadingProperties->setBevel(0);
        _pHeadingProperties->setZOrder(2);
        _pHeadingProperties->setText("  SSID                             Ch    Modes         Signal",
                bwidget_justify_horiz_left, bwidget_justify_vert_bottom);

        /* create max number of buttons - we will create/add them once, then
         * activate/deactivate them from the CWidgetMenu as needed. */
        for (int i = 0; i < MAX_NETWORK_WIFI_PROPS; i++)
        {
            CPanelNetworkWifiProp * pProp = NULL;
            pProp = new CPanelNetworkWifiProp(this, _pPropertiesMenu, MString(i).s(), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate NetworkWifi property", pProp, ret, eRet_OutOfMemory, error);

            pProp->initialize(_pCfg);

            /* start buttons as inactive - layout() will activate as needed */
            activateProp(pProp, false);

            /* keep track of all added buttons */
            _propList.add(pProp);
        }

        /* expandable status menu */
        _pStatusMenu = new CWidgetMenu("CPanelNetworkWifi::_pStatusMenu", getEngine(), _pNetworkWifiMenu, MRect(0, 0, MENU_WIDTH, 200), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pStatusMenu, ret, eRet_OutOfMemory, error);
        _pStatusMenu->showMenuBar(false);
        _pStatusMenu->showEdit(false);
        _pStatusMenu->setMenuTitle(NULL, "Connection Status");
        _pStatusMenu->setScroll(true);

        /* create double labels for tag/value in connection status
         * our window is big enough to hold them all so no need to scroll */
        for (int i = 0; i < MAX_CONNECTION_STATUS; i++)
        {
            CPanelNetworkWifiStatus * pStatus = NULL;
            pStatus = new CPanelNetworkWifiStatus(this, _pStatusMenu, MString(i).s(), font12, 35);
            CHECK_PTR_ERROR_GOTO("unable to allocate NetworkWifi status", pStatus, ret, eRet_OutOfMemory, error);

            /* start buttons as inactive - layout() will activate as needed */
            _pStatusMenu->setListButtonActive(pStatus->getContainer(), false);
            /* keep track of all added buttons */
            _statusList.add(pStatus);
        }

        /* expand button */
        _pExpand = new CWidgetButton("CPanelNetworkWifi::_pExpand", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pExpand, ret, eRet_OutOfMemory, error);
        _pNetworkWifiMenu->addExpandButton(_pExpand);
        /* default expand panel open if thumbnail decode is available */
        _bExpandPanel = (MAX_NETWORK_WIFI_PROPS != getConnectedIndex()) ? true : false;

        /* back button */
        _pBack = new CWidgetButton("CPanelNetworkWifi::_pBack", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _pBack, ret, eRet_OutOfMemory, error);
        _pBack->setText("Menu");
        _pNetworkWifiMenu->addBackButton(_pBack);

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

            _MsgBoxStatus = new CWidgetModalMsgBox("CPanelNetworkWifi::_MsgBox", getEngine(), this, geom, font12);
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

void CPanelNetworkWifi::uninitialize()
{
    _timerScan.stop();
    _timerUpdate.stop();
    DEL(_MsgBoxStatus);
    DEL(_pBack);
    DEL(_pHeadingProperties);
    DEL(_pExpand);
    DEL(_pStatusMenu);
    DEL(_pPropertiesMenu);
    DEL(_pNetworkWifiMenu);
}

#define GRID_WIDTH   150
#define GRID_HEIGHT  150

void CPanelNetworkWifi::layout()
{
    MRect rectMenu(0, 0, MENU_WIDTH, MENU_HEIGHT);
    MRect rectPanel   = getGeometry();
    MRect rectProps   = _pPropertiesMenu->getGeometry();
    MRect rectStatus  = _pPropertiesMenu->getGeometry();
    MRect rectHeading = _pHeadingProperties->getGeometry();
    MRect rectNetworkMenu;

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
    _pNetworkWifiMenu->setGeometry(rectMenu);
    setGeometry(rectPanel);

    _pExpand->setText(_bExpandPanel ? "<<" : ">>");
} /* layout */

eRet CPanelNetworkWifi::connect(CPanelNetworkWifiProp * pNetworkProp)
{
    eRet    ret = eRet_Ok;
    MString strSSID;
    MString strPassword;

    BDBG_ASSERT(NULL != pNetworkProp);

    /* save ssid before showing modal keyboard during which time the network prop
     * may update */
    strSSID = pNetworkProp->getSsid();

    if (true == pNetworkProp->isSecure())
    {
        /* get network password from user */
        strPassword = getScreenMain()->showKeyboardModal(pNetworkProp->getSsid(), "Enter Wifi Password");
        BDBG_WRN(("password entry:%s", strPassword.s()));
    }

    {
        CNetworkWifiConnectData dataConnect;
        dataConnect._strSsid     = strSSID;
        dataConnect._strPassword = strPassword;
        notifyObservers(eNotify_NetworkWifiConnect, &dataConnect);
    }

    if (false == _MsgBoxStatus->isVisible())
    {
        _MsgBoxStatus->showModal("Connecting to Wifi Network", -1, NULL, NULL, NULL);
    }

    return(ret);
} /* connect */

#ifdef WPA_SUPPLICANT_SUPPORT
eRet CPanelNetworkWifi::disconnect(CPanelNetworkWifiProp * pNetworkProp)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pNetworkProp);

    {
        CNetworkWifiConnectData dataConnect;
        dataConnect._strSsid = pNetworkProp->getSsid();
        notifyObservers(eNotify_NetworkWifiDisconnect, &dataConnect);
    }
    return(ret);
}

#elif NETAPP_SUPPORT
eRet CPanelNetworkWifi::disconnect(CPanelNetworkWifiProp * pNetworkProp)
{
    eRet    ret = eRet_Ok;
    MString strPassword;

    BDBG_ASSERT(NULL != pNetworkProp);

    {
        CNetworkWifiConnectData dataConnect;
        dataConnect._strSsid     = pNetworkProp->getSsid();
        dataConnect._strPassword = strPassword;
        notifyObservers(eNotify_NetworkWifiDisconnect, &dataConnect);
    }
    return(ret);
}

#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

/* returns MAX_NETWORK_WIFI_PROPS if NOT connected, otherwise returns index of connected property */
int CPanelNetworkWifi::getConnectedIndex()
{
    int i = 0;

    for (i = 0; i < MAX_NETWORK_WIFI_PROPS; i++)
    {
        if (true == _propList[i]->isConnected())
        {
            break;
        }
    }

    return(i);
}

void CPanelNetworkWifi::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    /* look for match to a selected network button */
    for (int i = 0; i < MAX_NETWORK_WIFI_PROPS; i++)
    {
        if (_propList[i]->getSsidButton() == pWidget)
        {
            eRet ret = eRet_Ok;

            if (false == _propList[i]->isConnected())
            {
                ret = connect(_propList[i]);
                CHECK_ERROR_GOTO("unable to connect to wifi network", ret, error);
            }
            else
            {
                ret = disconnect(_propList[i]);
                CHECK_ERROR_GOTO("unable to disconnect from wifi network", ret, error);
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

void CPanelNetworkWifi::expand(bool bExpand)
{
    _bExpandPanel = bExpand;
    _pExpand->setText(_bExpandPanel ? "<<" : ">>");
    layout();
}

void CPanelNetworkWifi::startUpdateTimers(bool bStart)
{
    if (false == isVisible())
    {
        return;
    }

    if (true == bStart)
    {
        _timerUpdate.stop();
        _timerUpdate.start(GET_INT(_pCfg, UI_NETWORK_WIFI_UPDATE_TIMEOUT));
    }
    else
    {
        _timerUpdate.stop();
    }
} /* startUpdateTimers */

void CPanelNetworkWifi::startScanTimers(bool bStart)
{
    if (false == isVisible())
    {
        return;
    }

    if (true == bStart)
    {
        _timerScan.stop();
        _timerScan.start(GET_INT(_pCfg, UI_NETWORK_WIFI_SCAN_TIMEOUT));
    }
    else
    {
        _timerScan.stop();
    }
} /* startScanTimers */

void CPanelNetworkWifi::activateProp(
        CPanelNetworkWifiProp * pProp,
        bool                    bActivate
        )
{
    if (bActivate != isPropActive(pProp))
    {
        _pPropertiesMenu->setListButtonActive(pProp->getContainer(), bActivate);
    }
}

bool CPanelNetworkWifi::isPropActive(CPanelNetworkWifiProp * pProp)
{
    return(_pPropertiesMenu->isListButtonActive(pProp->getContainer()));
}

void CPanelNetworkWifi::activateStatus(
        CPanelNetworkWifiStatus * pStatus,
        bool                      bActivate
        )
{
    if (bActivate != isStatusActive(pStatus))
    {
        BDBG_MSG(("set list button(%s) active(%d)", pStatus->getTag().s(), bActivate));
        _pStatusMenu->setListButtonActive(pStatus->getContainer(), bActivate);
    }
}

bool CPanelNetworkWifi::isStatusActive(CPanelNetworkWifiStatus * pStatus)
{
    return(_pStatusMenu->isListButtonActive(pStatus->getContainer()));
}

void CPanelNetworkWifi::clear()
{
    {
        MListItr <CPanelNetworkWifiProp> itr(&_propList);
        CPanelNetworkWifiProp *          pProp = NULL;

        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            pProp->setSsid("");
            activateProp(pProp, false);
        }
    }
    {
        MListItr <CPanelNetworkWifiStatus> itr(&_statusList);
        CPanelNetworkWifiStatus *          pStatus = NULL;

        for (pStatus = itr.first(); pStatus; pStatus = itr.next())
        {
            activateStatus(pStatus, false);
        }
    }
} /* clear */

void CPanelNetworkWifi::scan(bool bStart)
{
    if (false == isVisible())
    {
        return;
    }

    notifyObservers((true == bStart) ? eNotify_NetworkWifiScanStart : eNotify_NetworkWifiScanStop);
    startUpdateTimers(bStart);
    startScanTimers(bStart);
}

void CPanelNetworkWifi::scanResults()
{
    notifyObservers(eNotify_NetworkWifiScanResultRetrieve);
}

void CPanelNetworkWifi::show(bool bShow)
{
    CPanel::show(bShow);

    if (true == bShow)
    {
        _pBack->setFocus();
        clear();

        updateConnectStatus();
    }

    scan(bShow);
}

void CPanelNetworkWifi::clearConnectionStatus()
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CPanelNetworkWifiProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        pProp->setConnected(false);
    }
}

CPanelNetworkWifiProp * CPanelNetworkWifi::findProp(const char * strBssid)
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CPanelNetworkWifiProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getBssid() == strBssid)
        {
            break;
        }
    }

    return(pProp);
} /* findProp */

CPanelNetworkWifiProp * CPanelNetworkWifi::findProp(
        const char * strSsid,
        const int    nChannel
        )
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CPanelNetworkWifiProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getSsid() == strSsid)
        {
            /* check channel num if required */
            if (pProp->getChannel() == nChannel)
            {
                break;
            }
        }
    }

    return(pProp);
} /* findProp */

void CPanelNetworkWifi::setMenuTitleStatus(const char * pStr)
{
    MString strText = "Wifi Networks";

    if (NULL != pStr)
    {
        strText += " - ";
        strText += pStr;
    }
    _pNetworkWifiMenu->setMenuTitle(strText.s(), NULL);
}

void CPanelNetworkWifi::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerUpdate == pTimer)
        {
            updateConnectStatus();
            /* Let NetApp Scan in the background because if we we set a timeout too short and the background task
             * is in the process of trying to connect() the scan will be timed out, cancelled and then restarted.
             *
             * scan();
             */
        }
        else
        if (&_timerScan == pTimer)
        {
            /* start a new scan */
            scan(true);
        }
        else
        if (&_timerCloseMsgBox == pTimer)
        {
            _MsgBoxStatus->cancelModal("");
        }
    }
    break;

    case eNotify_NetworkWifiRssiResult:
    {
#ifdef NETAPP_SUPPORT
        CNetwork * pNetwork = (CNetwork *)notification.getData();

        /* updated signal strength status */
        updateSignalStrength(pNetwork);
#endif /* ifdef NETAPP_SUPPORT */
    }
    break;

    case eNotify_NetworkWifiScanStarted:
        setMenuTitleStatus("Scanning...");
        break;

    case eNotify_NetworkWifiScanStopped:
        setMenuTitleStatus();
        scanResults();
        break;

    case eNotify_NetworkWifiScanResult:
    {
#ifdef WPA_SUPPLICANT_SUPPORT
        CWifi * pWifi = (CWifi *)notification.getData();
#elif NETAPP_SUPPORT
        CNetwork * pNetwork = (CNetwork *)notification.getData();
#endif

        setMenuTitleStatus();
#ifdef WPA_SUPPLICANT_SUPPORT
        updateNetworkList(pWifi);
#elif NETAPP_SUPPORT
        updateNetworkList(pNetwork);
#endif

        startUpdateTimers(true);
        startScanTimers(true);
    }
    break;

#ifdef WPA_SUPPLICANT_SUPPORT
    case eNotify_NetworkWifiConnected:
    {
        CWifi * pWifi = (CWifi *)notification.getData();

        updateConnectStatus(pWifi);
        updateConnectedIcon(pWifi);

        /* expand panel based on connection status */
        expand(true);
        _MsgBoxStatus->cancelModal("");
    }
    break;

    case eNotify_NetworkWifiConnectFailure:
    {
        CWifi * pWifi = (CWifi *)notification.getData();

        updateConnectStatus(pWifi);
        updateConnectedIcon(pWifi);

        /* expand panel based on connection status */
        expand(false);
        _MsgBoxStatus->setText("Connection Failure");
        _timerCloseMsgBox.start();
    }
    break;

    case eNotify_NetworkWifiDisconnected:
    {
        CWifi * pWifi = (CWifi *)notification.getData();

        updateConnectStatus(pWifi);
        updateConnectedIcon(pWifi);

        /* expand panel based on connection status */
        expand(false);
    }
    break;
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

#ifdef NETAPP_SUPPORT
    case eNotify_NetworkWifiConnectionStatus:
    {
        CNetwork * pNetwork = (CNetwork *)notification.getData();

        setMenuTitleStatus();
        _MsgBoxStatus->cancelModal("");

        clearConnectionStatus();

        BDBG_MSG(("wifi connection status:%d", pNetwork->getConnectedStatus()));
        switch (pNetwork->getConnectedStatus())
        {
        case eConnectStatus_Connecting:
            if (false == _MsgBoxStatus->isVisible())
            {
                _MsgBoxStatus->showModal("Connecting to Wifi Network", -1, NULL, NULL, NULL);
            }
            break;

        case eConnectStatus_Connected:
        {
            CNetwork * pNetwork = (CNetwork *)notification.getData();

            /* connection success */
            CPanelNetworkWifiProp * pProp = NULL;
            eRet                    ret   = eRet_Ok;
            NETAPP_WIFI_AP_INFO     info;

            ret = pNetwork->getConnectedWifiNetwork(&info);
            CHECK_ERROR_GOTO("unable to get connected wifi network info", ret, error);

            /* may have received connected status before we scanned for available networks,
             * so we'll refresh our network list first */
            updateNetworkList(pNetwork);

            pProp = findProp(info.cSSID, info.ulChannel);
            CHECK_PTR_ERROR_GOTO("unable to find property that matches connection info", pProp, ret, eRet_NotAvailable, error);

            pProp->setConnected(true);

            updateConnectStatus(pNetwork);

            /* expand panel based on connection status */
            expand(true);
        }
        break;

        case eConnectStatus_Disconnected:
        {
            CNetwork * pNetwork = (CNetwork *)notification.getData();

            /* connection failed */
            BDBG_WRN(("WIFI connection disconnected"));

            updateConnectStatus(pNetwork);

            /* expand panel based on connection status */
            expand(false);
        }
        break;

        default:
            break;
        }   /* switch */
    }
    break;
#endif /* ifdef NETAPP_SUPPORT */

    default:
        break;
    } /* switch */

error:
    return;
} /* processNotification */

CPanelNetworkWifiProp * CPanelNetworkWifi::findEmptyProp()
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CPanelNetworkWifiProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (pProp->getSsid().isEmpty())
        {
            /* found empty prop in list */
            break;
        }
    }

    return(pProp);
}

void CPanelNetworkWifi::updateConnectStatus()
{
    if (NULL == _pNetwork)
    {
        return;
    }

    return(updateConnectStatus(_pNetwork));
}

#ifdef WPA_SUPPLICANT_SUPPORT
void CPanelNetworkWifi::updateConnectStatus(CWifi * pNetwork)
{
    MStringHash *             pConnectStatus = pNetwork->getConnectedStatus();
    MString                   strBSSID       = pNetwork->getConnectedBSSID();
    CPanelNetworkWifiStatus * pStatusWidget  = NULL;

    MListItr <CPanelNetworkWifiStatus> itr(&_statusList);

    BDBG_MSG(("%s connected network:%s", __FUNCTION__, strBSSID.s()));

    if (0 < strBSSID.length())
    {
        pConnectStatus = pNetwork->getConnectedStatus();
        pStatusWidget  = itr.first();
        for (pConnectStatus->first(); pConnectStatus->current(); pConnectStatus->next())
        {
            if (NULL != pStatusWidget)
            {
                pStatusWidget->setTag(pConnectStatus->name() + ":");
                pStatusWidget->setValue(pConnectStatus->value());

                /* note we do not use the button's show() method here since that is
                 * manipulated by the control itself when scrolling.  the setListButtonActive()
                 * method of the widget menu allows us to hide/show buttons without
                 * removing them from the control and still allows scrolling to work
                 * properly */
                _pStatusMenu->setListButtonActive(pStatusWidget->getContainer(), true);

                pStatusWidget = itr.next();
            }
            else
            {
                BDBG_WRN(("Too many status to display - increase size of network status list"));
            }
        }

        /* save network object */
        _pNetwork = pNetwork;
    }

    if (0 == pNetwork->getConnectedBSSID().length())
    {
        /* disconnected so clear status */
        pStatusWidget = itr.first();

        /* clear network object */
        _pNetwork = NULL;
    }

error:
    updateConnectedIcon(pNetwork);

    /* hide any remaining unused buttons */
    while (NULL != pStatusWidget)
    {
        _pStatusMenu->setListButtonActive(pStatusWidget->getContainer(), false);
        pStatusWidget = itr.next();
    }
} /* updateConnectStatus */

eRet CPanelNetworkWifi::updateSignalStrength(CNetworkWifi * pNetwork)
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CNetworkWifi                     info;
    CPanelNetworkWifiProp *          pProp = NULL;
    eRet ret                               = eRet_Ok;
    BDBG_ASSERT(NULL != pNetwork);

    return(ret);

    /*
     * ret = pNetwork->getConnectedWifiNetwork(&info);
     * CHECK_ERROR_GOTO("unable to get connected wifi network info", ret, error);
     */

    /* search property list for a match */
    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (*pProp == info)
        {
            /* network matches existing property in list (ssid and channel num) so just update data */
            pProp->update(&info);
            break;
        }
    }

error:
    return(ret);
} /* updateSignalStrength */

#elif defined (NETAPP_SUPPORT)
void CPanelNetworkWifi::updateConnectStatus(CNetwork * pNetwork)
{
    eRet ret                          = eRet_Ok;
    CPanelNetworkWifiStatus * pStatus = NULL;
    MStringHash               status;

    MListItr <CPanelNetworkWifiStatus> itr(&_statusList);

    BDBG_ASSERT(NULL != pNetwork);

    if (eConnectStatus_Connected == pNetwork->getConnectedStatus())
    {
        /* connected so get status */
        ret = pNetwork->getStatusList(&status);
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

        /* save network object */
        _pNetwork = pNetwork;
    }
    else
    {
        /* disconnected so clear status */
        pStatus = itr.first();

        /* clear network object */
        _pNetwork = NULL;
    }
error:
    updateConnectedIcon(pNetwork);

    /* hide any remaining unused buttons */
    while (NULL != pStatus)
    {
        _pStatusMenu->setListButtonActive(pStatus->getContainer(), false);
        pStatus = itr.next();
    }
} /* updateConnectStatus */

eRet CPanelNetworkWifi::updateSignalStrength(CNetwork * pNetwork)
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    NETAPP_WIFI_AP_INFO              info;
    CPanelNetworkWifiProp *          pProp = NULL;
    eRet ret                               = eRet_Ok;
    BDBG_ASSERT(NULL != pNetwork);

    ret = pNetwork->getConnectedWifiNetwork(&info);
    CHECK_ERROR_GOTO("unable to get connected wifi network info", ret, error);

    /* search property list for a match */
    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        if (*pProp == info)
        {
            /* network matches existing property in list (ssid and channel num) so just update data */
            pProp->update(&info);
            break;
        }
    }

error:
    return(ret);
} /* updateSignalStrength */

#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
void CPanelNetworkWifi::updateNetworkList(CWifi * pWifi)
{
    int            i        = 0;
    CNetworkWifi * pNetwork = NULL;

    MAutoList<CNetworkWifi> * pScannedNetworksList = NULL;
    CPanelNetworkWifiProp *   pProp                = NULL;
    CPanelNetworkWifiProp *   pPropFocus           = NULL;
    uint32_t                  nIndex               = 0;
    bwidget_t                 widgetFocus          = getFocus();
    eRet                      ret                  = eRet_Ok;

    BDBG_ASSERT(NULL != pWifi);

    pScannedNetworksList = pWifi->getScannedNetworkList();

    /* debug: printout network list */
    for (pNetwork = pScannedNetworksList->first();
         pNetwork;
         pNetwork = pScannedNetworksList->next())
    {
        BDBG_MSG(("Network: %s %s %d %s ssid:%s", pNetwork->_strBSSID.s(), pNetwork->_strFrequency.s(), pNetwork->_signalToNoise, pNetwork->_strFlags.s(), pNetwork->_strSSID.s()));
    }

    /* add/update networks - note that the pScannedNetworksList contains a list of
     * APs.  depending on your networks, there may be several APs per network.
     * we will only show the closest AP (best signal level) and use that to
     * represent the entire network of APs with the same SSID.
     */
    for (pNetwork = pScannedNetworksList->first();
         pNetwork;
         pNetwork = pScannedNetworksList->next())
    {
        /* ignore unnamed networks */
        if (true == pNetwork->_strSSID.isEmpty()) { continue; }

        for (pProp = _propList.first(); pProp; pProp = _propList.next())
        {
            if (*pProp == *pNetwork)
            {
                BDBG_MSG(("found matching network:%s", pProp->getSsid().s()));
                /* network matches existing property in list so just update data */
                pProp->update(pNetwork);
                break;
            }
        }

        if (NULL == pProp)
        {
            /* didn't find match in property list so create a new one */
            CPanelNetworkWifiProp * pPropNew = NULL;

            pPropNew = findEmptyProp();
            if (NULL == pPropNew)
            {
                BDBG_WRN(("unable to add new wifi network - increase MAX_NETWORK_WIFI_PROPS"));
                break;
            }
            else
            {
                /* add new property */
                pPropNew->update(pNetwork);
                BDBG_MSG(("create new prop for newly found network:%s", pPropNew->getBssid().s()));
            }
        }
    }

    for (pPropFocus = _propList.first(); pPropFocus; pPropFocus = _propList.next())
    {
        if (pPropFocus->getSsidButton()->getWidget() == widgetFocus)
        {
            break;
        }
    }
    /* now that we have added/updated the given networks to our property list,
     * we must remove outdated networks that no longer exist */
    for (pProp = _propList.first(); pProp; pProp = _propList.next())
    {
        for (pNetwork = pScannedNetworksList->first();
             pNetwork;
             pNetwork = pScannedNetworksList->next())
        {
            if (pProp->getSsid() == pNetwork->_strSSID)
            {
                /*
                 * found
                 * enable property on screen
                 */
                activateProp(pProp, true);
                break;
            }
        }

        if (NULL == pNetwork)
        {
            if (pProp->getSsidButton()->getWidget() == widgetFocus)
            {
                /* we are removing the currently focused widget so move focus before
                 * deactivating the widget */
                _pBack->setFocus();
                pPropFocus = NULL;
            }

            /* pProp not found in list of networks - deactivate it.
             * this will remove it from the on screen list */
            pProp->setSsid("");
            activateProp(pProp, false);
        }
    }

    /* sorting the list may rearrange our currently focused item so we will pass it to
     * sortListView() so that it can keep the focused item visible
     */
    ret = _pPropertiesMenu->sortListView((NULL != pPropFocus) ? pPropFocus->getContainer() : NULL);
    CHECK_ERROR("Unable to sort properties menu", ret);

    /* get the connected network if any and mark in list */
    updateConnectedIcon(pWifi);

    return;
} /* updateNetworkList */

void CPanelNetworkWifi::updateConnectedIcon(CWifi * pNetwork)
{
    /* NETAPP_WIFI_AP_INFO     info; */
    CPanelNetworkWifiProp * pProp = NULL;
    eRet                    ret   = eRet_Ok;
    MString                 strConnectedBSSID;

    MListItr <CPanelNetworkWifiProp> itr(&_propList);

    clearConnectionStatus();

    strConnectedBSSID = pNetwork->getConnectedBSSID();
    pProp             = findProp(strConnectedBSSID);
    if (NULL != pProp)
    {
        pProp->setConnected(true);
    }

error:
    return;
} /* updateConnectedIcon */

#elif NETAPP_SUPPORT
void CPanelNetworkWifi::updateNetworkList(CNetwork * pNetwork)
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    NETAPP_WIFI_AP_INFO *            pInfo = NULL;
    CPanelNetworkWifiProp *          pProp = NULL;
    uint32_t  nIndex                       = 0;
    bwidget_t widgetFocus                  = getFocus();
    eRet      ret                          = eRet_Ok;

    BDBG_ASSERT(NULL != pNetwork);

    /* add/update networks */
    pInfo = pNetwork->getNetworkInfo(nIndex);
    while (NULL != pInfo)
    {
        if (false == MString(pInfo->cSSID).isEmpty())
        {
            /* search property list for a match */
            for (pProp = itr.first(); pProp; pProp = itr.next())
            {
                if (*pProp == *pInfo)
                {
                    /* network matches existing property in list (ssid and channel num) so just update data */
                    pProp->update(pInfo);
                    break;
                }
            }

            if (NULL == pProp)
            {
                /* didn't find match in property list so create a new one */
                CPanelNetworkWifiProp * pPropNew = NULL;

                pPropNew = findEmptyProp();
                if (NULL == pPropNew)
                {
                    BDBG_WRN(("unable to add new wifi network - increase MAX_NETWORK_WIFI_PROPS"));
                    break;
                }
                else
                {
                    /* add new property */
                    pPropNew->update(pInfo);
                }
            }
        }

        nIndex++;
        pInfo = pNetwork->getNetworkInfo(nIndex);
    }

    /* now that we have added/updated the given networks to our property list,
     * we must remove outdated networks that no longer exist */
    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        nIndex = 0;
        pInfo  = pNetwork->getNetworkInfo(nIndex);

        while (NULL != pInfo)
        {
            if (pProp->getSsid() == pInfo->cSSID)
            {
                /*
                 * found
                 * enable property on screen
                 */
                activateProp(pProp, true);
                break;
            }

            nIndex++;
            pInfo = pNetwork->getNetworkInfo(nIndex);
        }

        if (NULL == pInfo)
        {
            if (pProp->getSsidButton()->getWidget() == widgetFocus)
            {
                /* we are removing the currently focused widget so move focus before
                 * deactivating the widget */
                _pBack->setFocus();
            }

            /* pProp not found in list of networks - deactivate it.
             * this will remove it from the on screen list */
            pProp->setSsid("");
            activateProp(pProp, false);
        }
    }

    ret = _pPropertiesMenu->sortListView();
    CHECK_ERROR("Unable to sort properties menu", ret);

    /* get the connected network if any and mark in list */
    updateConnectedIcon(pNetwork);
} /* updateNetworkList */

void CPanelNetworkWifi::updateConnectedIcon(CNetwork * pNetwork)
{
    NETAPP_WIFI_AP_INFO     info;
    CPanelNetworkWifiProp * pProp = NULL;
    eRet                    ret   = eRet_Ok;

    MListItr <CPanelNetworkWifiProp> itr(&_propList);

    clearConnectionStatus();

    if (eConnectStatus_Connected == pNetwork->getConnectedStatus())
    {
        ret = pNetwork->getConnectedWifiNetwork(&info);
        CHECK_ERROR_GOTO("unable to retrieve connected wifi network", ret, error);

        for (pProp = itr.first(); pProp; pProp = itr.next())
        {
            if (*pProp == info)
            {
                pProp->setConnected(true);
            }
        }
    }

error:
    return;
} /* updateConnectedIcon */

#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

void CPanelNetworkWifi::dump(bool bForce)
{
    MListItr <CPanelNetworkWifiProp> itr(&_propList);
    CPanelNetworkWifiProp *          pProp = NULL;

    for (pProp = itr.first(); pProp; pProp = itr.next())
    {
        pProp->dump(bForce);
    }
}

#endif /* ifdef NETAPP_SUPPORT */