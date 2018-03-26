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

#ifndef ATLAS_PANEL_NETWORK_WIFI_H__
#define ATLAS_PANEL_NETWORK_WIFI_H__

#include "panel.h"
#include "timer.h"
#include "widget_progress.h"
class CWidgetMenu;
class CWidgetButton;
class CWidgetLabel;
class CWidgetGrid;
class CPanelKeyboard;
class CNetworkWifi;
#ifdef WPA_SUPPLICANT_SUPPORT
class CPanelWps;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
#define PERCENT_TO_UINT16(percent)  ((percent) * 65535 / 100)

#ifdef NETAPP_SUPPORT
typedef enum eWifiMode
{
    eWifiMode_A,
    eWifiMode_B,
    eWifiMode_G,
    eWifiMode_N,
    eWifiMode_AC,
    eWifiMode_Max
} eWifiMode;
#endif /* ifdef NETAPP_SUPPORT */

/* CPanelNetworkWifiProp is the tag/value button used to display each network wifi property
 * in the list widget. */
class CPanelNetworkWifiProp
{
public:
    CPanelNetworkWifiProp(
            CPanel *      pPanel,
            CWidgetMenu * pMenu,
            const char *  strName,
            bwin_font_t   font
            );
    ~CPanelNetworkWifiProp();

    eRet            initialize(CConfiguration * pCfg);
    CWidgetButton * getContainer(void) { return(_pContainer); }
    void            setConnected(bool bConnected);
    bool            isConnected(void)            { return(_bConnected); }
    void            setBssid(const char * pText) { _strBssid = pText; }
    MString         getBssid(void)               { return(_strBssid); }
    void            setSsid(const char * pText);
    MString         getSsid(void)       { return(_pSsid->getText()); }
    CWidgetButton * getSsidButton(void) { return(_pSsid); }
    void            setChannel(const char * pText);
    int             getChannel(void) { return(MString(_pChannel->getText()).toInt()); }
#ifdef NETAPP_SUPPORT
    void      setMode(eWifiMode mode);
    eWifiMode getMode(void) { return(_mode); }
#endif
    void setSignalLevel(uint8_t percent);
    void setSecurity(bool bSecurity);
    bool isSecure(void) { return(_pSecurity->isVisible()); }
    void dump(bool bForce = false);
#ifdef WPA_SUPPLICANT_SUPPORT
    void update(CNetworkWifi * pInfo);

    /* comparison operator overload */
    bool operator ==(CNetworkWifi & rhs);
#elif NETAPP_SUPPORT
    void update(NETAPP_WIFI_AP_INFO * pInfo);

    /* comparison operator overload */
    bool operator ==(const NETAPP_WIFI_AP_INFO & rhs);
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

protected:
    CConfiguration *           _pCfg;
    CWidgetListViewContainer * _pContainer;
    CWidgetLabel *             _pContainerBkgnd;
    CWidgetLabel *             _pConnection;
    MString                    _strBssid;
    CWidgetButton *            _pSsid;
    CWidgetLabel *             _pChannel;
#ifdef NETAPP_SUPPORT
    CWidgetLabel * _pModeA;
    CWidgetLabel * _pModeB;
    CWidgetLabel * _pModeG;
    CWidgetLabel * _pModeN;
    CWidgetLabel * _pModeAC;
#endif /* ifdef NETAPP_SUPPORT */
    CWidgetProgress * _pSignalLevel;
    CWidgetLabel *    _pSecurity;
#ifdef NETAPP_SUPPORT
    CWidgetLabel * _pModeToLabel[eWifiMode_Max + 1];
    eWifiMode      _mode;
#endif
    bool _bConnected;
};

/* CPanelNetworkWifiStatus is the tag/value button used to display each network property
 * in the list widget. */
class CPanelNetworkWifiStatus
{
public:
    CPanelNetworkWifiStatus(
            CPanel *      pPanel,
            CWidgetMenu * pMenu,
            const char *  strName,
            bwin_font_t   font,
            uint8_t       labelPercentage = 50
            )
    {
        uint32_t colorText       = COLOR_EGGSHELL;
        uint32_t colorBackground = 0xCC222222;
        eRet     ret             = eRet_Ok;

        BDBG_ASSERT(NULL != pPanel);
        ret = pMenu->addDualLabelButton(pPanel, strName, &_pContainer, &_pTag, &_pValue, font, labelPercentage, fill_eSolid);
        if (eRet_Ok != ret)
        {
            fprintf(stderr, "unable to allocate double label button");
            goto error;
        }
        _pContainer->setFocusable(false);

        _pTag->setTextColor(colorText);
        _pValue->setTextColor(colorText);

        _pTag->setBackgroundColor(colorBackground);
        _pValue->setBackgroundColor(colorBackground);
        _pContainer->setBackgroundColor(colorBackground);

error:
        return;
    }

    ~CPanelNetworkWifiStatus()
    {
        DEL(_pContainer);
        DEL(_pTag);
        DEL(_pValue);
    }

    CWidgetButton * getContainer(void) { return(_pContainer); }

    MString getTag(void) { return(MString(_pTag->getText())); }
    void    setTag(const char * pText)
    {
        if (NULL != _pTag)
        {
            _pTag->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        }
    }

    MString getValue(void) { return(MString(_pValue->getText())); }
    void    setValue(const char * pText)
    {
        if (NULL != _pValue)
        {
            _pValue->setText(pText, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        }
    }

protected:
    CWidgetButton * _pContainer;
    CWidgetLabel *  _pTag;
    CWidgetLabel *  _pValue;
};

class CPanelNetworkWifi : public CPanel
{
public:
    CPanelNetworkWifi(
            CWidgetEngine * pWidgetEngine,
            CScreenMain *   pScreenMain,
            CWidgetBase *   pParentWidget,
            MRect           geometry,
            bwin_font_t     font,
            bwin_t          parentWin = NULL
            ); /* optional */
    ~CPanelNetworkWifi(void);

    eRet                    initialize(CModel * pModel, CConfig * pConfig);
    void                    uninitialize(void);
    int                     getConnectedIndex(void);
    void                    onClick(bwidget_t widget);
    void                    show(bool bShow);
    void                    scan(bool bStart = true);
    void                    scanResults(void);
    void                    clear(void);
    void                    clearConnectionStatus(void);
    CPanelNetworkWifiProp * findProp(const char * strBssid);
    CPanelNetworkWifiProp * findProp(const char * strSsid, const int nChannel);
    void                    setMenuTitleStatus(const char * pStr = NULL);
    void                    processNotification(CNotification & notification);
    eRet                    connect(CPanelNetworkWifiProp * pNetworkProp);
    eRet                    disconnect(CPanelNetworkWifiProp * pNetworkProp);
    CPanelNetworkWifiProp * findEmptyProp(void);
    void                    updateConnectStatus(void);
#ifdef WPA_SUPPLICANT_SUPPORT
    void updateConnectStatus(CWifi * pNetwork);
    eRet updateSignalStrength(CNetworkWifi * pNetwork);
#elif NETAPP_SUPPORT
    void updateConnectStatus(CNetwork * pNetwork);
    eRet updateSignalStrength(CNetwork * pNetwork);
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT
    void updateNetworkList(CWifi * pWifi);
    void updateConnectedIcon(CWifi * pNetwork);
#elif NETAPP_SUPPORT
    void updateNetworkList(CNetwork * pNetwork);
    void updateConnectedIcon(CNetwork * pNetwork);
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */
    void expand(bool bExpand = true);
    void startUpdateTimers(bool bStart = true);
    void startScanTimers(bool bStart = true);
    void activateProp(CPanelNetworkWifiProp * pProp, bool bActivate = true);
    bool isPropActive(CPanelNetworkWifiProp * pProp);
    void activateStatus(CPanelNetworkWifiStatus * pStatus, bool bActivate = true);
    bool isStatusActive(CPanelNetworkWifiStatus * pStatus);
    void layout(void);
    void dump(bool bForce = false);

protected:
    CWidgetMenu * _pNetworkWifiMenu;
    CWidgetMenu * _pPropertiesMenu;
#ifdef WPA_SUPPLICANT_SUPPORT
    CWidgetButton * _pWpsButton;
    CWidgetLabel *  _pWpsLabel;
#endif
    CWidgetMenu *   _pStatusMenu;
    bool            _bExpandPanel;
    CWidgetButton * _pExpand;
#ifdef NETAPP_SUPPORT
    CWidgetLabel * _pHeadingProperties;
#endif
#ifdef WPA_SUPPLICANT_SUPPORT
    CPanelWps * _pPanelWps;
#endif
    CTimer               _timerCloseMsgBox;
    CTimer               _timerUpdate;
    CTimer               _timerScan;
    CWidgetModalMsgBox * _MsgBoxStatus;
    CWidgetModalMsgBox * _MsgBoxError;
#ifdef WPA_SUPPLICANT_SUPPORT
    CWifi * _pNetwork;
#elif defined NETAPP_SUPPORT
    CNetwork * _pNetwork;
#else /* placeholder */
    void * _pNetwork;
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */
    CWidgetButton * _pBack;
    MAutoList <CPanelNetworkWifiProp>   _propList;
    MAutoList <CPanelNetworkWifiStatus> _statusList;
};

#endif /* ifdef NETAPP_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_NETWORK_WIFI_H__ */