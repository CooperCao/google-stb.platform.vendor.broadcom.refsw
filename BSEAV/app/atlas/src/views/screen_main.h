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

#ifndef ATLAS_PANEL_MAIN_H__
#define ATLAS_PANEL_MAIN_H__

#include "screen.h"
#include "timer.h"
#include "mlist.h"
#include "widget_label.h"
#include "widget_button.h"
#include "widget_check_button.h"
#include "widget_menu.h"
#include "widget_progress.h"
#include "widget_meter.h"
#include "widget_edit.h"
#include "widget_popup_list.h"
#include "widget_modalmsgbox.h"

#include "panel_display.h"
#include "panel_decode.h"
#include "panel_audio.h"
#include "panel_playback.h"
#include "panel_streaming.h"
#include "panel_keyboard.h"
#if NEXUS_HAS_FRONTEND
#include "panel_tuner.h"
#include "panel_scan_qam.h"
#include "panel_scan_vsb.h"
#include "panel_scan_sat.h"
#include "panel_scan_ofdm.h"
#endif /* if NEXUS_HAS_FRONTEND */
#include "panel_vbi.h"
#ifdef MPOD_SUPPORT
#include "panel_cablecard.h"
#endif
#include "panel_record.h"
#include "panel_buffers.h"
#include "panel_power.h"
#include "panel_network_wifi.h"
#include "panel_bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

class CChannel;

typedef enum eMenu
{
    eMenu_Main,
    eMenu_Decode,
    eMenu_Display,
    eMenu_Audio,
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    eMenu_Network,
#endif
#ifdef NETAPP_SUPPORT
    eMenu_Bluetooth,
#endif
    eMenu_Playback,
    eMenu_Streaming,
    eMenu_Tuner,
    eMenu_ScanQam,
    eMenu_ScanVsb,
    eMenu_ScanSat,
    eMenu_ScanOfdm,
    eMenu_Vbi,
    eMenu_Power,
    eMenu_CableCard,
    eMenu_Max
} eMenu;

class CConnectionStatus
{
public:
    CConnectionStatus(
            bool         bConnected,
            const char * strText,
            uint32_t     timeout
            ) :
        _bConnected(bConnected),
        _strText(strText),
        _timeout(timeout) {}

public:
    bool     _bConnected;
    MString  _strText;
    uint32_t _timeout;
};

class CScreenMain : public CScreen
{
public:
    CScreenMain(
            CWidgetEngine * pWidgetEngine,
            CConfig *       pConfig,
            CWidgetBase *   pParent
            );
    ~CScreenMain(void);

    virtual void show(bool bShow);
    virtual bool isVisible(void);

    eRet    initialize(CModel * pModel);
    void    uninitialize();
    void    onClick(bwidget_t widget);
    eRet    onKeyDown(bwidget_t widget, bwidget_key key);
    void    processNotification(CNotification & notification);
    void    setPlaybackStatus(CPlayback * pPlayback);
    void    setChannelNum(const char * str1 = NULL, const char * str2 = NULL);
    void    setChannelNum(CChannel * pChannel);
    void    setChannelStatus(eMode mode, const char * str1 = NULL, const char * str2 = NULL, const char * strPlaybackTitle = NULL, CChannel * pChannel = NULL);
    eRet    addRecordEncodeIndicator(CChannel * pChannel);
    void    setChannelIndicator(CChannelMgr * pChannelMgr, CPlaylistDb * pPlaylistDb);
    void    setVolumeProgress(uint8_t progress, bool bMute = false);
    eRet    registerObserver(CObserver * pObserver, eNotification notification = eNotify_All);
    MRect   getPipGraphicsGeometry(CSimpleVideoDecode * pVideoDecode);
    eRet    showConnectionStatus(bool bConnected, const char * strText, uint32_t timeout = 3000);
    eRet    showPlaybackTitle(bool bShow = true);
    eRet    setPlaybackTitle(const char * str);
    eRet    showDebugTitle(bool bShow = true);
    eRet    setDebugTitle(const char * str);
    eRet    updatePlaybackProgress(void);
    eRet    updateChannelProgress(void);
    eRet    updatePip(void);
    void    showMenu(eMenu menu);
    MString showKeyboardModal(const char * strTitle, const char * strEntryTitle);
#if DVR_LIB_SUPPORT
    void setTsbStatus(CTsb * pTsb);
    eRet updateTsbProgress(void);
#endif
#if NEXUS_HAS_FRONTEND
    void setScanProgress(CTuner * pTuner, uint8_t progress);
#endif

protected:
    bool isPlaybackActive(void);
    void showMenu(CWidgetMenu * pMenu);

protected:
    CWidgetProgress * _pLabelScan;
    CWidgetProgress * _pLabelVolume;
    MRect             _rectChannelNum;
    CWidgetLabel *    _pLabelPip;
    CWidgetLabel *    _pLabelChannelNum;
    CWidgetLabel *    _pLabelChannelNumText;
    CWidgetLabel *    _pLabelChannelType;
    CWidgetProgress * _pProgressPlayback;
    CWidgetLabel *    _pLabelPlaybackState;
    CWidgetLabel *    _pLabelPlaybackLength;
    CWidgetLabel *    _pLabelConnectionType;
    CWidgetLabel *    _pLabelConnectionStatus;
    CWidgetLabel *    _pLabelPlaybackName;
    CWidgetLabel *    _pLabelPlaybackNameShadow;
    CWidgetLabel *    _pLabelDebugName;
    CWidgetLabel *    _pLabelDebugNameShadow;
    CWidgetMenu *     _pMainMenu;
    CWidgetButton *   _Display;
    CWidgetButton *   _Decode;
    CWidgetButton *   _Playback;
    CWidgetButton *   _Audio;
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    CWidgetButton * _Network;
#endif
#ifdef NETAPP_SUPPORT
    CWidgetButton * _Bluetooth;
#endif
    CWidgetButton *      _Streaming;
    MRect                _rectChText;
    MRect                _rectChType;
    CWidgetCheckButton * _Buffers;
    CWidgetButton *      _Back;
    CPanelDecode *       _pDecodeMenu;
    CPanelDisplay *      _pDisplayMenu;
    CPanelAudio *        _pAudioMenu;
    CPanelPlayback *     _pPlaybackMenu;
    CPanelRecord *       _pRecordMenu;
    CPanelBuffers *      _pBuffersMenu;
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    CPanelNetworkWifi * _pNetworkMenu;
#endif
#ifdef NETAPP_SUPPORT
    CPanelBluetooth * _pBluetoothMenu;
#endif
#ifdef PLAYBACK_IP_SUPPORT
    CPanelStreaming * _pStreamingMenu;
#endif
    MAutoList <CWidgetLabel>  _indicatorList;
    MList <CPanel>            _panelList;
    MList <CConnectionStatus> _connectionStatusList;
    CTimer                    _timerConnectionStatus;
    CTimer                    _timerVolume;
    CTimer                    _timerChannel;
    CTimer                    _timerBuffersUpdate;
    uint16_t                  _channelsFound;
    CWidgetModalMsgBox *      _MsgBox;
    CPanelPower *             _pPowerMenu;
    CPanelKeyboard *          _pKeyboard;
    CPanelVbi *               _pVbiMenu;
    CTimer                    _timerMsgBox;
#if NEXUS_HAS_FRONTEND
    CWidgetButton *  _Tuner;
    CWidgetButton *  _ScanQam;
    CWidgetButton *  _ScanVsb;
    CWidgetButton *  _ScanSat;
    CWidgetButton *  _ScanOfdm;
    CPanelTuner *    _pTunerMenu;
    CPanelScanQam *  _pScanQamMenu;
    CPanelScanVsb *  _pScanVsbMenu;
    CPanelScanSat *  _pScanSatMenu;
    CPanelScanOfdm * _pScanOfdmMenu;
#endif /* if NEXUS_HAS_FRONTEND */
#ifdef MPOD_SUPPORT
    CWidgetButton *   _CableCard;
    CPanelCableCard * _pCableCardMenu;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_MAIN_H__ */