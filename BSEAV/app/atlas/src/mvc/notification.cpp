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

#include "notification.h"
#include "atlas_os.h"
#include "mstringhash.h"

BDBG_MODULE(atlas_notification);

ENUM_TO_MSTRING_INIT_C(notificationToString, eNotification)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(MString(eNotify_All).s(), "eNotify_All")
ENUM_TO_MSTRING_ENTRY(MString(eNotify_KeyUp).s(), "eNotify_KeyUp")                   /* keypress - key pressed up */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_KeyDown).s(), "eNotify_KeyDown")               /* keypress - key pressed down */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VirtualKeyDown).s(), "eNotify_VirtualKeyDown") /* keypress - virtual key pressed down */

ENUM_TO_MSTRING_ENTRY(MString(eNotify_Tune).s(), "eNotify_Tune")                               /* command  - tune */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChUp).s(), "eNotify_ChUp")                               /* command  - change channel up */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChDown).s(), "eNotify_ChDown")                           /* command  - change channel down */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ScanStart).s(), "eNotify_ScanStart")                     /* command  - start channel scan */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ScanStop).s(), "eNotify_ScanStop")                       /* command  - stop channel scan */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackListDump).s(), "eNotify_PlaybackListDump")       /* command  - dump playback list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackStart).s(), "eNotify_PlaybackStart")             /* command  - start a playback */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackStop).s(), "eNotify_PlaybackStop")               /* command  - stop a playback */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackTrickMode).s(), "eNotify_PlaybackTrickMode")     /* command  - do trickmode playback */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetVolume).s(), "eNotify_SetVolume")                     /* command  - change volume level */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetMute).s(), "eNotify_SetMute")                         /* command  - change mute state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_RecordStart).s(), "eNotify_RecordStart")                 /* command  - start a record */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_RecordStop).s(), "eNotify_RecordStop")                   /* command  - stop a record */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_EncodeStart).s(), "eNotify_EncodeStart")                 /* command  - start an encode */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_EncodeStop).s(), "eNotify_EncodeStop")                   /* command  - stop an encode */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_RefreshPlaybackList).s(), "eNotify_RefreshPlaybackList") /* command  - stop a playback */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelListLoad).s(), "eNotify_ChannelListLoad")         /* command  - load channel list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelListSave).s(), "eNotify_ChannelListSave")         /* command  - save channel list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelListDump).s(), "eNotify_ChannelListDump")         /* command  - dump channel list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAudioProgram).s(), "eNotify_SetAudioProgram")         /* command  - change current audio pid */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAudioProcessing).s(), "eNotify_SetAudioProcessing")   /* command  - change audio processing type*/
#ifdef CPUTEST_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetCpuTestLevel).s(), "eNotify_SetCpuTestLevel") /* command  - change cpu test level */
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetSpdifInput).s(), "eNotify_SetSpdifInput")                       /* command  - change spdif input type */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetHdmiAudioInput).s(), "eNotify_SetHdmiAudioInput")               /* command  - change hdmi audio input type */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAudioDownmix).s(), "eNotify_SetAudioDownmix")                   /* command  - change audio downmix */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAudioDualMono).s(), "eNotify_SetAudioDualMono")                 /* command  - change audio dual mono */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetDolbyDRC).s(), "eNotify_SetDolbyDRC")                           /* command  - change dolby dynamic range control */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetDolbyDialogNorm).s(), "eNotify_SetDolbyDialogNorm")             /* command  - change dolby dialog normalization */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetVideoFormat).s(), "eNotify_SetVideoFormat")                     /* command  - change video format and graphics */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetContentMode).s(), "eNotify_SetContentMode")                     /* command  - change video content mode */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetColorSpace).s(), "eNotify_SetColorSpace")                       /* command  - change video colorspace */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetColorDepth).s(), "eNotify_SetColorDepth")                       /* command  - change video color depth for main decoder */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetMpaaDecimation).s(), "eNotify_SetMpaaDecimation")               /* command  - change mpaa decimation state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetDeinterlacer).s(), "eNotify_SetDeinterlacer")                   /* command  - change MAD deinterlacer state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetBoxDetect).s(), "eNotify_SetBoxDetect")                         /* command  - change box detect state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAspectRatio).s(), "eNotify_SetAspectRatio")                     /* command  - change aspect ratio */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetAutoVideoFormat).s(), "eNotify_SetAutoVideoFormat")             /* command  - change auto video format setting */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetVbiSettings).s(), "eNotify_SetVbiSettings")                     /* command  - change VBI settings */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetPowerMode).s(), "eNotify_SetPowerMode")                         /* command  - change power mode */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ShowPip).s(), "eNotify_ShowPip")                                   /* command  - show/hide pip window */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SwapPip).s(), "eNotify_SwapPip")                                   /* command  - swap pip/main windows */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ClosedCaptionEnable).s(), "eNotify_ClosedCaptionEnable")           /* command  - closed caption has been enabled/disabled */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ClosedCaptionMode).s(), "eNotify_ClosedCaptionMode")               /* command  - set 608/708 closed caption mode */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ipClientTranscodeEnable).s(), "eNotify_ipClientTranscodeEnable")   /* command  - BIP transcoding has been enabled/disabled for a given client */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ipClientTranscodeProfile).s(), "eNotify_ipClientTranscodeProfile") /* command - BIP transcode profile setting for a given client */
#ifdef WPA_SUPPLICANT_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiList).s(), "eNotify_NetworkWifiList") /* command  - list currently known wifi networks*/
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanStart).s(), "eNotify_NetworkWifiScanStart")                           /* command  - start wifi network scan */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanStop).s(), "eNotify_NetworkWifiScanStop")                             /* command  - stop wifi network scan */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanResultRetrieve).s(), "eNotify_NetworkWifiScanResultRetrieve")         /* command  - retrieve results of wifi network scan */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectedNetworkStatus).s(), "eNotify_NetworkWifiConnectedNetworkStatus") /* command  - retrieve status of the currently connected network */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnect).s(), "eNotify_NetworkWifiConnect")                               /* command  - connect to given wifi network */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiGetConnectState).s(), "eNotify_NetworkWifiGetConnectState")               /* command  - get the wifi connection state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiDisconnect).s(), "eNotify_NetworkWifiDisconnect")                         /* command  - disconnect from current wifi network */
#ifdef NETAPP_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothDiscoveryStart).s(), "eNotify_BluetoothDiscoveryStart") /* command  - start bluetooth disovery */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothDiscoveryStop).s(), "eNotify_BluetoothDiscoveryStop")   /* command  - stop bluetooth disovery */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothConnect).s(), "eNotify_BluetoothConnect")               /* command  - connect to given bluetooth device */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothDisconnect).s(), "eNotify_BluetoothDisconnect")         /* command  - disconnect from current bluetooth device */

ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothGetSavedBtListInfo).s(), "eNotify_BluetoothGetSavedBtListInfo")                     /* command  - get Saved Bt List */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothGetDiscBtListInfo).s(), "eNotify_BluetoothGetDiscBtListInfo")                       /* command  - get Discovery Bt list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothGetConnBtListInfo).s(), "eNotify_BluetoothGetConnBtListInfo")                       /* command  - get Connected Bt list */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothConnectBluetoothFromDiscList).s(), "eNotify_BluetoothConnectBluetoothFromDiscList") /* command  - connect to given bluetooth device  */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothA2DPStart).s(), "eNotify_BluetoothA2DPStart")                                       /* command  - start bluetooth A2DP  */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothA2DPStop).s(), "eNotify_BluetoothA2DPStop")                                         /* command  - stop bluetooth A2DP  */
#endif /* ifdef NETAPP_SUPPORT */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ShowDiscoveredPlaylists).s(), "eNotify_ShowDiscoveredPlaylists") /* command  - show list of playlists discovered by atlas server */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ShowPlaylist).s(), "eNotify_ShowPlaylist")                       /* command  - show contents of given playlist */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_StreamChannel).s(), "eNotify_StreamChannel")                     /* command  - stream IP channel from remote server */
#if HAS_VID_NL_LUMA_RANGE_ADJ
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetPlmVideo).s(), "eNotify_SetPlmVideo") /* command  - set programmable luma mapping for video */
#endif
#if HAS_GFX_NL_LUMA_RANGE_ADJ
ENUM_TO_MSTRING_ENTRY(MString(eNotify_SetPlmGraphics).s(), "eNotify_SetPlmGraphics") /* command  - set programmable luma mapping for graphics */
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Debug).s(), "eNotify_Debug") /* command  - show debug message */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Exit).s(), "eNotify_Exit")   /* command  - exit atlas */

ENUM_TO_MSTRING_ENTRY(MString(eNotify_Error).s(), "eNotify_Error")                               /* status   - error occurred */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ErrorVbi).s(), "eNotify_ErrorVbi")                         /* status   - VBI error occurred */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ErrorVbiDcs).s(), "eNotify_ErrorVbiDcs")                   /* status   - VBI DCS error occurred */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ErrorVbiMacrovision).s(), "eNotify_ErrorVbiMacrovision")   /* status   - VBI Macrovision error occurred */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelListChanged).s(), "eNotify_ChannelListChanged")     /* status   - channel list has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelListVersion).s(), "eNotify_ChannelListVersion")     /* status   - channel list is incompatible version */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CurrentChannel).s(), "eNotify_CurrentChannel")             /* status   - current channel has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CurrentChannelNull).s(), "eNotify_CurrentChannelNull")     /* status   - current channel has been untuned */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelStart).s(), "eNotify_ChannelStart")                 /* status   - channel has started decoding */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelFinish).s(), "eNotify_ChannelFinish")               /* status   - channel has untuned */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelStateChanged).s(), "eNotify_ChannelStateChanged")   /* status   - channel state has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DeferredChannel).s(), "eNotify_DeferredChannel")           /* status   - deferred channel has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ScanStarted).s(), "eNotify_ScanStarted")                   /* status   - scan has started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ScanStopped).s(), "eNotify_ScanStopped")                   /* status   - scan has stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ScanProgress).s(), "eNotify_ScanProgress")                 /* status   - scan percent complete */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackListChanged).s(), "eNotify_PlaybackListChanged")   /* status   - playback list updated */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaybackStateChanged).s(), "eNotify_PlaybackStateChanged") /* status   - playback state has changed */
#if DVR_LIB_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_TsbStateChanged).s(), "eNotify_TsbStateChanged") /* status   - tsb state has changed */
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VolumeChanged).s(), "eNotify_VolumeChanged")           /* status   - volume level has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_MuteChanged).s(), "eNotify_MuteChanged")               /* status   - mute state has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PipStateChanged).s(), "eNotify_PipStateChanged")       /* status   - PIP state has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_RecordStarted).s(), "eNotify_RecordStarted")           /* status   - record has started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_RecordStopped).s(), "eNotify_RecordStopped")           /* status   - record has stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_EncodeStarted).s(), "eNotify_EncodeStarted")           /* status   - encode has started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_EncodeStopped).s(), "eNotify_EncodeStopped")           /* status   - encode has stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoSourceChanged).s(), "eNotify_VideoSourceChanged") /* status   - video source has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoStreamChanged).s(), "eNotify_VideoStreamChanged") /* status   - video source has changed */
#if HAS_VID_NL_LUMA_RANGE_ADJ
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoPlmChanged).s(), "eNotify_VideoPlmChanged") /* status   - video programmable luma mapping has changed */
#endif
#if HAS_GFX_NL_LUMA_RANGE_ADJ
ENUM_TO_MSTRING_ENTRY(MString(eNotify_GraphicsPlmChanged).s(), "eNotify_GraphicsPlmChanged") /* status   - graphics programmable luma mapping has changed */
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoDecodeStarted).s(), "eNotify_VideoDecodeStarted")                   /* status   - video decode started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoDecodeStopped).s(), "eNotify_VideoDecodeStopped")                   /* status   - video decode stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_AudioSourceChanged).s(), "eNotify_AudioSourceChanged")                   /* status   - audio source has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_AudioDecodeStarted).s(), "eNotify_AudioDecodeStarted")                   /* status   - audio decode started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_AudioDecodeStopped).s(), "eNotify_AudioDecodeStopped")                   /* status   - audio decode stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VideoFormatChanged).s(), "eNotify_VideoFormatChanged")                   /* status   - video format has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ContentModeChanged).s(), "eNotify_ContentModeChanged")                   /* status   - video content mode has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ColorSpaceChanged).s(), "eNotify_ColorSpaceChanged")                     /* status   - video colorspace has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ColorSpaceFailure).s(), "eNotify_ColorSpaceFailure")                     /* status   - failure changing video colorspace */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ColorDepthChanged).s(), "eNotify_ColorDepthChanged")                     /* status   - video color depth has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ColorDepthFailure).s(), "eNotify_ColorDepthFailure")                     /* status   - failure changing video color depth */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_MpaaDecimationChanged).s(), "eNotify_MpaaDecimationChanged")             /* status   - video component mpaa decimation has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DeinterlacerChanged).s(), "eNotify_DeinterlacerChanged")                 /* status   - video deinterlacer setting has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BoxDetectChanged).s(), "eNotify_BoxDetectChanged")                       /* status   - video box detect setting has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_AspectRatioChanged).s(), "eNotify_AspectRatioChanged")                   /* status   - video aspect ratio setting has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_AutoVideoFormatChanged).s(), "eNotify_AutoVideoFormatChanged")           /* status   - auto video format setting has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_VbiSettingsChanged).s(), "eNotify_VbiSettingsChanged")                   /* status   - display VBI settings have changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PowerModeChanged).s(), "eNotify_PowerModeChanged")                       /* status   - power mode settings have changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_HdmiHotplugEvent).s(), "eNotify_HdmiHotplugEvent")                       /* status   - HDMI hotplug event has occurred */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Timeout).s(), "eNotify_Timeout")                                         /* status   - timer expired */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CableCardIn).s(), "eNotify_CableCardIn")                                 /* status   - cablecard has been plugged in */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CableCardOut).s(), "eNotify_CableCardOut")                               /* status   - cablecard has been plugged out */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_ChannelMapUpdate).s(), "eNotify_ChannelMapUpdate")                       /* status   - cablecard has sent updated channel map via DSG or OOB */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DigitalClosedCaptionChanged).s(), "eNotify_DigitalClosedCaptionChanged") /* status   - digital closed caption display setting has changed  */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Capabilities).s(), "eNotify_Capabilities")                               /* status   - nexus capabilites */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_TunerLockStatus).s(), "eNotify_TunerLockStatus")                         /* status   - tuner lock status has changed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NonTunerLockStatus).s(), "eNotify_NonTunerLockStatus")                   /* status   - non tuner lock status has changed like IP or Streamer */
#ifdef CPUTEST_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CpuTestStarted).s(), "eNotify_CpuTestStarted") /* status   - cpu stress test started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_CpuTestStopped).s(), "eNotify_CpuTestStopped") /* status   - cpu stress test stopped */
#endif
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiRssiResult).s(), "eNotify_NetworkWifiRssiResult")                                       /* status   - wifi network RSSI status */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanStarted).s(), "eNotify_NetworkWifiScanStarted")                                     /* status   - wifi network scan started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanStopped).s(), "eNotify_NetworkWifiScanStopped")                                     /* status   - wifi network scan stopped */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanFailure).s(), "eNotify_NetworkWifiScanFailure")                                     /* status   - wifi network scan failed */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiScanResult).s(), "eNotify_NetworkWifiScanResult")                                       /* status   - results of wifi network scan are available */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectionStatus).s(), "eNotify_NetworkWifiConnectionStatus")                           /* status   - wifi network connection status is available */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectState).s(), "eNotify_NetworkWifiConnectState")                                   /* status   - wifi network connection state */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnected).s(), "eNotify_NetworkWifiConnected")                                         /* status   - wifi network connected */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectAssocStart).s(), "eNotify_NetworkWifiConnectAssocStart")                         /* status   - wifi network connection association with AP started */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectFailure).s(), "eNotify_NetworkWifiConnectFailure")                               /* status   - wifi network connection attempt failure */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectFailureWrongKey).s(), "eNotify_NetworkWifiConnectFailureWrongKey")               /* status   - wifi network connection attempt failture because of incorrect key */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectFailureNetworkNotFound).s(), "eNotify_NetworkWifiConnectFailureNetworkNotFound") /* status   - wifi network connection attempt failture because of missing network */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiConnectFailureAssocReject).s(), "eNotify_NetworkWifiConnectFailureAssocReject")         /* status   - wifi network connection attempt failture because of a rejected association with AP */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiDisconnected).s(), "eNotify_NetworkWifiDisconnected")                                   /* status   - wifi network disconnected */
#ifdef WPA_SUPPLICANT_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_NetworkWifiListUpdated).s(), "eNotify_NetworkWifiListUpdated") /* status   - updated list currently known wifi networks*/
#endif
#ifdef NETAPP_SUPPORT
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothDiscoveryStarted).s(), "eNotify_BluetoothDiscoveryStarted") /* status  - started bluetooth disovery */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothDiscoveryResult).s(), "eNotify_BluetoothDiscoveryStarted")  /* status   - discovery done and results of bluetooth discovery are available */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothConnectionStatus).s(), "eNotify_BluetoothDiscoveryStarted") /* status   - bluetooth connection status is available */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothConnectionDone).s(), "eNotify_BluetoothDiscoveryStarted")   /* status   - connection done */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_BluetoothListStatusDone).s(), "eNotify_BluetoothDiscoveryStarted")   /* status   - Bt list status done */
#endif /* ifdef NETAPP_SUPPORT */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaylistAdded).s(), "eNotify_PlaylistAdded")                       /* status   - a playlist was added to the playlist database */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaylistRemoved).s(), "eNotify_PlaylistRemoved")                   /* status   - a playlist was removed from the playlist database */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DecodeStarted).s(), "eNotify_DecodeStarted")                       /* status   - a video or audio decode has started decoding */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DecodeStopped).s(), "eNotify_DecodeStopped")                       /* status   - a video or audio decode has stopped decoding */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_DiscoveredPlaylistsShown).s(), "eNotify_DiscoveredPlaylistsShown") /* status   - a discovered playlist has been displayed on console */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_PlaylistShown).s(), "eNotify_PlaylistShown")                       /* status   - a playlist contents has been displayed on console */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_EnableRemoteIr).s(), "eNotify_EnableRemoteIr")                     /* status   - enable/disable ir remote handling */
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Invalid).s(), "eNotify_Invalid")
ENUM_TO_MSTRING_ENTRY(MString(eNotify_Max).s(), "eNotify_Max")
ENUM_TO_MSTRING_END()

CNotification::CNotification(
        eNotification id,
        void *        data
        ) :
    _id(id),
    _data(data)
{
}

eNotification CNotification::getId()
{
    return(_id);
}

void CNotification::setData(void * data)
{
    _data = data;
}

void * CNotification::getData()
{
    return(_data);
}

CNotification::~CNotification(void)
{
}

CObserver::CObserver(
        const char *    strName,
        NOTIFY_CALLBACK notifyCallback,
        CObserver *     context
        ) :
    _strName(strName),
    _notifyCallback(notifyCallback),
    _context(context ? context : this)
{
}

eRet CObserver::notify(CNotification & notification)
{
    eRet ret = eRet_Ok;

    if (_notifyCallback)
    {
        _notifyCallback(_context, notification);
    }
    else
    {
        /* error: no notify callback */
        ret = eRet_NotAvailable;
    }

    return(ret);
}

void CObserver::setNotifyCallback(NOTIFY_CALLBACK notifyCallback)
{
    _notifyCallback = notifyCallback;
}

void CObserver::setNotifyContext(CObserver * context)
{
    _context = context;
}

void CObserver::processNotification(CNotification & notification)
{
    BSTD_UNUSED(notification);
}

CSubject::CSubject(const char * strName) :
    _strName(strName)
{
    _mutex = B_Mutex_Create(NULL);
    BDBG_ASSERT(_mutex);

    /* set this environment variable to help debug memory leaks all CSubject derived classes */
    if (getenv("ATLAS_MEMORY_LEAK_DETECT"))
    {
        BDBG_WRN(("==> Allocated mutex (%p) for:%s (%p)", (void *)_mutex, strName, (void *)this));
    }

    clearAllObservers();
}

eRet CSubject::registerObserver(
        CObserver *   pObserver,
        eNotification notification
        )
{
    eRet ret = eRet_Ok;

    MListItr <CObserverListElem> itr(&_observerList);
    CObserverListElem *          ptr = NULL;

    if (eNotify_Invalid == notification)
    {
        return(eRet_InvalidParameter);
    }

    /* serialized access to _observerList */
    CScopedMutex observerListMutex(_mutex);

    BDBG_ASSERT(NULL != pObserver);

    /* see if observer/notification pair already in list */
    for (ptr = itr.first(); ptr; ptr = itr.next())
    {
        CObserverListElem listElem(pObserver, notification);
        if (*ptr == listElem)
        {
            break;
        }
    }

    if (ptr)
    {
        /* duplicate observer/notification pair - do nothing */
        ret = eRet_InvalidParameter;
    }
    else
    {
        CObserverListElem * pObserverListElem = NULL;
        pObserverListElem = new CObserverListElem(pObserver, notification);
        BDBG_ASSERT(pObserverListElem);
        _observerList.add(pObserverListElem);
    }

    return(ret);
} /* registerObserver */

eRet CSubject::unregisterObserver(
        CObserver *   pObserver,
        eNotification notification
        )
{
    eRet ret = eRet_Ok;

    CObserverListElem * ptr = NULL;

    /* serialized access to _observerList */
    CScopedMutex observerListMutex(_mutex);

    BDBG_ASSERT(NULL != pObserver);

    /* assume given observer/notification not found */
    ret = eRet_InvalidParameter;

    /* search list for matching observer/notification pair */
    for (ptr = _observerList.first(); ptr; ptr = _observerList.next())
    {
        if (ptr->getObserver() == pObserver)
        {
            if ((eNotify_All == notification) || (ptr->getNotification() == notification))
            {
                ptr = _observerList.remove(ptr);
                BDBG_ASSERT(ptr);
                delete ptr;
                ret = eRet_Ok;
            }
        }
    }

    return(ret);
} /* unregisterObserver */

/* register the list of observers/notifications in 'this' with
 * the given pSubject.  this is a way to copy the observer/notifications
 * list from one CSubject to another. */
eRet CSubject::registerObserverList(CSubject * pSubject)
{
    eRet ret = eRet_Ok;

    MListItr <CObserverListElem> itr(&_observerList);
    CObserverListElem *          ptr = NULL;

    /* serialized access to _observerList */
    CScopedMutex observerListMutex(_mutex);

    BDBG_ASSERT(NULL != pSubject);

    /* copy observers/notifications from 'this' to given pSubject */
    for (ptr = itr.first(); ptr; ptr = itr.next())
    {
        pSubject->registerObserver(ptr->getObserver(), ptr->getNotification());
    }

    return(ret);
} /* registerObserverList */

/* unregister observers/notifications from pSubject.  use the list of
 * observers/notifications in 'this' CSubject as the list to remove.
 * the observer list in 'this' remains unchanged. */
eRet CSubject::unregisterObserverList(CSubject * pSubject)
{
    eRet ret = eRet_Ok;

    MListItr <CObserverListElem> itr(&_observerList);
    CObserverListElem *          ptr = NULL;

    /* serialized access to _observerList */
    CScopedMutex observerListMutex(_mutex);

    BDBG_ASSERT(NULL != pSubject);

    /* remove corresponding observers/notifications from pSubject */
    for (ptr = itr.first(); ptr; ptr = itr.next())
    {
        pSubject->unregisterObserver(ptr->getObserver(), ptr->getNotification());
    }

    return(ret);
} /* unregisterObserver */

eRet CSubject::notifyObservers(
        eNotification id,
        void *        data
        )
{
    eRet ret = eRet_Ok;

    MList <CObserverListElem> foundObserverList;

    if (eNotify_Invalid == id)
    {
        return(eRet_InvalidParameter);
    }

    {
        /* serialized access to _observerList */
        CScopedMutex observerListMutex(_mutex);

        /* find all observers with matching notifications in observer list and save in foundObserverList.
         * this is done so that the real observer list is not mutex locked while notifying
         * all the registered observers. */
        if (0 < _observerList.total())
        {
            CObserverListElem *          ptr = NULL;
            MListItr <CObserverListElem> itr(&_observerList);

            for (ptr = itr.first(); ptr; ptr = itr.next())
            {
                if ((eNotify_All == ptr->getNotification()) || (id == ptr->getNotification()))
                {
                    foundObserverList.add(ptr);
                }
            }
        }
    }

    /* notify all found matching observers */
    if (0 < foundObserverList.total())
    {
        CObserverListElem *          ptrFound = NULL;
        MListItr <CObserverListElem> itrFound(&foundObserverList);

        for (ptrFound = itrFound.first(); ptrFound; ptrFound = itrFound.next())
        {
            CNotification notification(id, data);

            if ((eNotify_Timeout != id) || (getenv("ATLAS_SHOW_TIMER_NOTIFICATION_DEBUG")))
            {
                /* only display "timeout" debug messages if SHOW_TIMER_NOTIFICATION_DEBUG is set.
                 * note that we use an env var here instead of atlas.cfg to keep CSubject free of
                 * application specific constructs. */
                BDBG_MSG(("%s notify(%s)-> %s", getName().s(), notificationToString(id).s(), ptrFound->getObserver()->getName().s()));
            }
            ptrFound->getObserver()->notify(notification);
        }
    }

    return(ret);
} /* notifyObservers */

eRet CSubject::clearAllObservers(void)
{
    eRet ret = eRet_Ok;

    CObserverListElem * ptr = NULL;

    /* serialized access to _observerList */
    CScopedMutex observerListMutex(_mutex);

    /* clear all observers from list */
    if (0 < _observerList.total())
    {
        for (ptr = _observerList.first(); ptr; ptr = _observerList.next())
        {
            CObserverListElem * pElem = NULL;
            pElem = _observerList.remove(0);
            BDBG_ASSERT(pElem);
            delete pElem;
        }
    }

    return(ret);
} /* clearAllObservers */

eRet CSubject::copyObservers(CSubject * pSubject)
{
    CObserverListElem * pElem = NULL;
    eRet                ret   = eRet_Ok;

    BDBG_ASSERT(NULL != pSubject);

    /* serialized access to _observerLists */
    CScopedMutex observerListMutex(pSubject->_mutex);

    for (pElem = pSubject->_observerList.first(); pElem; pElem = pSubject->_observerList.next())
    {
        CScopedMutex observerListMutex(_mutex);

        CObserverListElem * pElemNew = new CObserverListElem(pElem);
        BDBG_ASSERT(NULL != pElemNew);
        _observerList.add(pElemNew);
    }

    return(ret);
} /* copyObservers */

CSubject::~CSubject(void)
{
    clearAllObservers();

    if (_mutex)
    {
        B_Mutex_Destroy(_mutex);
        _mutex = NULL;
    }
}