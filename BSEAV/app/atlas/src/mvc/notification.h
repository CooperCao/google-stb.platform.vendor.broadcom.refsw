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

#ifndef NOTIFICATION_H__
#define NOTIFICATION_H__

#include "atlas.h"
#include "atlas_os.h"
#include "mlist.h"
#include "mstring.h"

#ifdef __cplusplus
extern "C" {
#endif

/* changes to this enum requires a corresponding change to notificationToString() */
typedef enum eNotification
{
    eNotify_All,
    eNotify_KeyUp,          /* keypress - key pressed up */
    eNotify_KeyDown,        /* keypress - key pressed down */
    eNotify_VirtualKeyDown, /* keypress - fake remote key pressed down */

    eNotify_Tune,                /* command  - tune */
    eNotify_ChUp,                /* command  - change channel up */
    eNotify_ChDown,              /* command  - change channel down */
    eNotify_ScanStart,           /* command  - start channel scan */
    eNotify_ScanStop,            /* command  - stop channel scan */
    eNotify_PlaybackListDump,    /* command  - dump playback list */
    eNotify_PlaybackStart,       /* command  - start a playback */
    eNotify_PlaybackStop,        /* command  - stop a playback */
    eNotify_PlaybackTrickMode,   /* command  - do trickmode playback */
    eNotify_SetVolume,           /* command  - change volume level */
    eNotify_SetMute,             /* command  - change mute state */
    eNotify_RecordStart,         /* command  - start a record */
    eNotify_RecordStop,          /* command  - stop a record */
    eNotify_EncodeStart,         /* command  - start an encode */
    eNotify_EncodeStop,          /* command  - stop an encode */
    eNotify_RefreshPlaybackList, /* command  - stop a playback */
    eNotify_ChannelListLoad,     /* command  - load channel list */
    eNotify_ChannelListSave,     /* command  - save channel list */
    eNotify_ChannelListDump,     /* command  - dump channel list */
    eNotify_GetChannelStats,     /* command  - get Channel Stats */
    eNotify_SetAudioProgram,     /* command  - change current audio pid */
    eNotify_SetAudioProcessing,  /* command  - change audio processing type */
#ifdef CPUTEST_SUPPORT
    eNotify_SetCpuTestLevel, /* command  - change cpu test level */
#endif
    eNotify_SetSpdifInput,            /* command  - change spdif input type */
    eNotify_SetHdmiAudioInput,        /* command  - change hdmi audio input type */
    eNotify_SetAudioDownmix,          /* command  - change audio downmix */
    eNotify_SetAudioDualMono,         /* command  - change audio dual mono */
    eNotify_SetDolbyDRC,              /* command  - change dolby dynamic range control */
    eNotify_SetDolbyDialogNorm,       /* command  - change dolby dialog normalization */
    eNotify_SetVideoFormat,           /* command  - change video format and graphics */
    eNotify_SetContentMode,           /* command  - change video content mode */
    eNotify_SetColorSpace,            /* command  - change video colorspace */
    eNotify_SetColorDepth,            /* command  - change video color depth for main video decoder */
    eNotify_SetMpaaDecimation,        /* command  - change mpaa decimation state */
    eNotify_SetDeinterlacer,          /* command  - change MAD deinterlacer state */
    eNotify_SetBoxDetect,             /* command  - change box detect state */
    eNotify_SetAspectRatio,           /* command  - change aspect ratio */
    eNotify_SetAutoVideoFormat,       /* command  - change auto video format setting */
    eNotify_SetVbiSettings,           /* command  - change VBI based setting(s) */
    eNotify_SetPowerMode,             /* command  - set power mode */
    eNotify_ShowPip,                  /* command  - show/hide pip window */
    eNotify_SwapPip,                  /* command  - swap pip/main windows */
    eNotify_ClosedCaptionEnable,      /* command  - enable/disable closed caption */
    eNotify_ClosedCaptionMode,        /* command  - set 608/708 closed caption mode */
    eNotify_ipClientTranscodeEnable,  /* command  - enable/disable transcoding in BIP streaming for a given client */
    eNotify_ipClientTranscodeProfile, /* command  - set the IP transcode profile in BIP streaming for a given client */
#ifdef WPA_SUPPLICANT_SUPPORT
    eNotify_NetworkWifiList, /* command  - list currently known wifi networks */
#endif
    eNotify_NetworkWifiScanStart,              /* command  - start wifi network scan */
    eNotify_NetworkWifiScanStop,               /* command  - stop wifi network scan */
    eNotify_NetworkWifiScanResultRetrieve,     /* command  - retrieve results of wifi network scan */
    eNotify_NetworkWifiConnectedNetworkStatus, /* command  - retrieve status of the currently connected network */
    eNotify_NetworkWifiConnect,                /* command  - connect to given wifi network */
    eNotify_NetworkWifiGetConnectState,        /* command  - get connection state of current wifi network */
    eNotify_NetworkWifiDisconnect,             /* command  - disconnect from current wifi network */
#ifdef NETAPP_SUPPORT
    eNotify_BluetoothDiscoveryStart,   /* command  - start bluetooth discovery */
    eNotify_BluetoothDiscoveryStop,    /* command  - stop bluetooth discovery */
    eNotify_BluetoothDiscoveryStopped, /* command  - stopped bluetooth discovery */
    eNotify_BluetoothConnect,          /* command  - connect to given bluetooth device */
    eNotify_BluetoothDisconnect,       /* command  - disconnect from given bluetooth device */

    eNotify_BluetoothGetSavedBtListInfo,           /* command  - get Info from the Saved BT list. Remebered when connected */
    eNotify_BluetoothGetDiscBtListInfo,            /* command  - getInfo from the Discovery BT list. List genereted from a discovery Start*/
    eNotify_BluetoothGetConnBtListInfo,            /* command  - getInfo from the Connected BT list. List genereted from a get Connected Devices*/
    eNotify_BluetoothConnectBluetoothFromDiscList, /* command  - connect to one index in BT Discovery list */

    eNotify_BluetoothA2DPStart, /* command  - start bluetooth A2DP */
    eNotify_BluetoothA2DPStop,  /* command  - stop bluetooth A2DP */
#endif /* ifdef NETAPP_SUPPORT */
    eNotify_ShowDiscoveredPlaylists, /* command  - show list of playlists discovered by atlas server */
    eNotify_ShowPlaylist,            /* command  - show contents of given playlist */
    eNotify_StreamChannel,           /* command  - stream IP channel from remote server */
#if HAS_VID_NL_LUMA_RANGE_ADJ
    eNotify_SetPlmVideo, /* command  - set video programmable luma mapping enable/disable */
#endif
#if HAS_GFX_NL_LUMA_RANGE_ADJ
    eNotify_SetPlmGraphics, /* command  - set graphics programmable luma mapping enable/disable */
#endif
    eNotify_Debug, /* command  - show debug message */
    eNotify_Exit,  /* command  - exit atlas */

    eNotify_Error,                /* status   - error occurred */
    eNotify_ErrorVbi,             /* status   - VBI error occurred */
    eNotify_ErrorVbiDcs,          /* status   - VBI DCS error occurred */
    eNotify_ErrorVbiMacrovision,  /* status   - VBI Macrovision error occurred */
    eNotify_ChannelListChanged,   /* status   - channel list has changed */
    eNotify_ChannelListVersion,   /* status   - channel list is incompatible version */
    eNotify_CurrentChannel,       /* status   - current channel has changed */
    eNotify_CurrentChannelNull,   /* status   - current channel has been untuned */
    eNotify_ChannelStart,         /* status   - channel has started decoding */
    eNotify_ChannelFinish,        /* status   - channel has untuned */
    eNotify_ChannelStateChanged,  /* status   - channel state has changed */
    eNotify_DeferredChannel,      /* status   - deferred channel has changed */
    eNotify_ScanStarted,          /* status   - scan has started */
    eNotify_ScanStopped,          /* status   - scan has stopped */
    eNotify_ScanProgress,         /* status   - scan percent complete */
    eNotify_PlaybackListChanged,  /* status   - playback list updated */
    eNotify_PlaybackStateChanged, /* status   - playback state has changed */
#if DVR_LIB_SUPPORT
    eNotify_TsbStateChanged, /* status   - tsb state has changed */
#endif
    eNotify_VolumeChanged,      /* status   - volume level has changed */
    eNotify_MuteChanged,        /* status   - mute state has changed */
    eNotify_PipStateChanged,    /* status   - PIP state has changed */
    eNotify_RecordStarted,      /* status   - record has started */
    eNotify_RecordStopped,      /* status   - record has stopped */
    eNotify_EncodeStarted,      /* status   - encode has started */
    eNotify_EncodeStopped,      /* status   - encode has stopped */
    eNotify_VideoSourceChanged, /* status   - video source has changed */
    eNotify_VideoStreamChanged, /* status   - video stream has changed */
#if HAS_VID_NL_LUMA_RANGE_ADJ
    eNotify_VideoPlmChanged, /* status   - video programmable luma mapping (PLM) changed */
#endif
#if HAS_GFX_NL_LUMA_RANGE_ADJ
    eNotify_GraphicsPlmChanged, /* status   - graphics programmable luma mapping (PLM) changed */
#endif
    eNotify_VideoDecodeStarted,          /* status   - video decode started */
    eNotify_VideoDecodeStopped,          /* status   - video decode stopped */
    eNotify_AudioSourceChanged,          /* status   - audio source has changed */
    eNotify_AudioDecodeStarted,          /* status   - audio decode started */
    eNotify_AudioDecodeStopped,          /* status   - audio decode stopped */
    eNotify_VideoFormatChanged,          /* status   - video format has changed */
    eNotify_ContentModeChanged,          /* status   - video content mode has changed */
    eNotify_ColorSpaceChanged,           /* status   - video colorspace has changed */
    eNotify_ColorSpaceFailure,           /* status   - failure changing video colorspace */
    eNotify_ColorDepthChanged,           /* status   - video color depth has changed */
    eNotify_ColorDepthFailure,           /* status   - failure changing video color depth */
    eNotify_MpaaDecimationChanged,       /* status   - video component mpaa decimation has changed */
    eNotify_DeinterlacerChanged,         /* status   - video deinterlacer setting has changed */
    eNotify_BoxDetectChanged,            /* status   - video box detect setting has changed */
    eNotify_AspectRatioChanged,          /* status   - video aspect ratio setting has changed */
    eNotify_AutoVideoFormatChanged,      /* status   - auto video format setting has changed */
    eNotify_VbiSettingsChanged,          /* status   - VBI based setting(s) has changed */
    eNotify_PowerModeChanged,            /* status   - power mode has changed */
    eNotify_HdmiHotplugEvent,            /* status   - HDMI hotplug event has occurred */
    eNotify_Timeout,                     /* status   - timer expired */
    eNotify_CableCardIn,                 /* status   - cablecard has been plugged in */
    eNotify_CableCardOut,                /* status   - cablecard has been plugged out */
    eNotify_ChannelMapUpdate,            /* status   - cablecard has sent updated channel map via DSG or OOB */
    eNotify_DigitalClosedCaptionChanged, /* status   - digital closed caption display setting has changed * */
    eNotify_Capabilities,                /* status   - nexus capabilities */
    eNotify_TunerLockStatus,             /* status   - tuner lock status has changed */
    eNotify_NonTunerLockStatus,          /* status   - non tuner lock status has changed like IP or Streamer */
#ifdef CPUTEST_SUPPORT
    eNotify_CpuTestStarted, /* status   - cpu stress test started */
    eNotify_CpuTestStopped, /* status   - cpu stress test stopped */
#endif
    eNotify_NetworkWifiRssiResult,                    /* status   - wifi network RSSI status */
    eNotify_NetworkWifiScanStarted,                   /* status   - wifi network scan started */
    eNotify_NetworkWifiScanStopped,                   /* status   - wifi network scan stopped */
    eNotify_NetworkWifiScanFailure,                   /* status   - wifi network scan failed */
    eNotify_NetworkWifiScanResult,                    /* status   - results of wifi network scan are available */
    eNotify_NetworkWifiConnectionStatus,              /* status   - wifi network connection status is available */
    eNotify_NetworkWifiConnectState,                  /* status   - wifi network connection state */
    eNotify_NetworkWifiConnected,                     /* status   - wifi network has been connected */
    eNotify_NetworkWifiConnectAssocStart,             /* status   - wifi network connection association with AP started */
    eNotify_NetworkWifiConnectFailure,                /* status   - wifi network connection attempt failture */
    eNotify_NetworkWifiConnectFailureWrongKey,        /* status   - wifi network connection attempt failture because of incorrect key */
    eNotify_NetworkWifiConnectFailureNetworkNotFound, /* status   - wifi network connection attempt failture because of missing network */
    eNotify_NetworkWifiConnectFailureAssocReject,     /* status   - wifi network connection attempt failture because of a rejected association with AP */
    eNotify_NetworkWifiDisconnected,                  /* status   - wifi network has been disconnected */
#ifdef WPA_SUPPLICANT_SUPPORT
    eNotify_NetworkWifiListUpdated, /* status   - updated list currently known wifi networks */
#endif
#ifdef NETAPP_SUPPORT
    eNotify_BluetoothDiscoveryStarted, /* status   - bluetooth discovery started */
    eNotify_BluetoothDiscoveryResult,  /* status   - discovery done and results of bluetooth discovery are available */
    eNotify_BluetoothConnectionStatus, /* status   - bluetooth connection status is available */
    eNotify_BluetoothConnectionDone,   /* status   - connection done */
    eNotify_BluetoothListStatusDone,   /* status   - Bt list status done */
    eNotify_BluetoothA2DPStarted,      /* status   - bluetooth A2DP started */
    eNotify_BluetoothA2DPStopped,      /* status   - bluetooth A2DP stopped */
#endif /* ifdef NETAPP_SUPPORT */
    eNotify_PlaylistAdded,            /* status   - a playlist was added to the playlist database */
    eNotify_PlaylistRemoved,          /* status   - a playlist was removed from the playlist database */
    eNotify_DecodeStarted,            /* status   - a video or audio decode has started */
    eNotify_DecodeStopped,            /* status   - a video or audio decode has stopped */
    eNotify_DiscoveredPlaylistsShown, /* status   - a discovered playlist has been displayed on console */
    eNotify_PlaylistShown,            /* status   - a playlist contents has been displayed on console */
    eNotify_ChannelStatsShown,        /* status   - channel stats has been displayed on console */
#if RF4CE_SUPPORT
    eNotify_AddRf4ceRemote,
    eNotify_DisplayRf4ceRemotes,
    eNotify_RemoveRf4ceRemote,
#endif
    eNotify_EnableRemoteIr, /* status - enable/disable ir remote handling */
    eNotify_Invalid,
    eNotify_Max
} eNotification;

ENUM_TO_MSTRING_DECLARE(notificationToString, eNotification)

/*
 *  CNotification is passed between CSubjects and CObservers.  It contains
 *  an id indicating the notification type and an optional data pointer.
 */
class CNotification
{
public:
    CNotification(
            eNotification id,
            void *        data = NULL
            );
    ~CNotification(void);

    virtual eNotification getId(void);
    virtual void          setData(void * data);
    virtual void *        getData(void);
protected:
    eNotification _id;
    void *        _data;
};

/*
 *  CObserver objects are able to monitor notifications from CSubject objects.
 */
class CObserver;
typedef void (* NOTIFY_CALLBACK)(CObserver * context, CNotification & notification);

class CObserver
{
public:
    CObserver(
            const char *    strName,
            NOTIFY_CALLBACK notifyCallback,
            CObserver *     context = NULL
            );

    virtual ~CObserver(void) {}
    virtual eRet notify(CNotification & notification);
    virtual void setNotifyCallback(NOTIFY_CALLBACK notifyCallback);
    virtual void setNotifyContext(CObserver * context);
    virtual void processNotification(CNotification & notification);

    bool operator ==(const CObserver & other) const
    {
        return((_notifyCallback == other._notifyCallback) &&
               (_context == other._context));
    }

    MString getName() { return(_strName); }
protected:
    MString         _strName;
    NOTIFY_CALLBACK _notifyCallback;
    CObserver *     _context;
};

/*
 *  CSubject objects can be monitored by CObserver objects.  Changes to
 *  the subject can be monitored by one or more CObserver objects.
 */
class CSubject
{
public:
    CSubject(const char * strName);
    virtual ~CSubject(void);
    virtual eRet registerObserver(CObserver * observer, eNotification notification = eNotify_All);
    virtual eRet unregisterObserver(CObserver * observer, eNotification notification = eNotify_All);
    virtual eRet registerObserverList(CSubject * pSubject);
    virtual eRet unregisterObserverList(CSubject * pSubject);
    virtual eRet notifyObservers(eNotification notification, void * data = NULL);
    virtual eRet clearAllObservers(void);
    virtual eRet copyObservers(CSubject * pSubject);
    virtual void timerCallback(void * pTimer) { BSTD_UNUSED(pTimer); return; }

    MString getName()                     { return(_strName); }
    void    setName(const char * strName) { _strName = strName; }

protected:
    class CObserverListElem
    {
public:
        CObserverListElem(
                CObserver *   pObserver,
                eNotification notification
                ) :
            _pObserver(pObserver),
            _notification(notification) {}

        CObserverListElem(CObserverListElem * pElem) :
            _pObserver(pElem->_pObserver),
            _notification(pElem->_notification) {}

        CObserver *   getObserver()     { return(_pObserver); }
        eNotification getNotification() { return(_notification); }
        bool operator ==(const CObserverListElem & other) const
        {
            return((_pObserver == other._pObserver) &&
                   (_notification == other._notification));
        }

protected:
        CObserver *   _pObserver;
        eNotification _notification;
    };

    MString _strName;
private:
    MList <CObserverListElem> _observerList;
    B_MutexHandle             _mutex;
};

#ifdef __cplusplus
}
#endif

#endif /* NOTIFICATION_H__ */