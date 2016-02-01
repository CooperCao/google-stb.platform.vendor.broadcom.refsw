/******************************************************************************
*(c) 2014 Broadcom Corporation
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
******************************************************************************/
/******************************************************************************
 *
 * WVPlaybackAPI.h
 *
 * Declarations for Widevine Playback API Abstract Interface
 *****************************************************************************/

#ifndef __WV_PLAYBACK_API_H__
#define __WV_PLAYBACK_API_H__

#include <string>
#include <vector>
#include "WVTypes.h"
#include "WVStatus.h"
#include "WVControlSettings.h"

class WVSession;

class WVPlaybackAPI {
public:
    enum WVEventType {
	WV_Event_Playing,
	WV_Event_FastForward,
	WV_Event_Rewind,
	WV_Event_Paused,
	WV_Event_Stopped,
	WV_Event_Seek,
	WV_Event_Underflow,
	WV_Event_EndOfStream
    };

    //
    // Callback Function Datatypes
    //
    typedef void (*WVQualityCallback)(size_t numLevels, size_t currentLevel, bool isHD, void *userData);
    typedef void (*WVBandwidthCallback)(unsigned long bytesPerSecond, void *userData);
    typedef void (*WVEventCallback)(WVEventType eventType,
				    const std::string &time, void *userData);
    typedef void (*WVErrorCallback)(WVStatus errorCode, void *userData);
    typedef void (*WVTimeCallback)(const std::string &time, void *userData);
    typedef void (*WVBufferLevelCallback)(float bufferLevelSecs, bool isLow, bool isNormal, void *userData);

    //
    // Object Initialization Methods
    //
    static WVPlaybackAPI *Instance();

    virtual WVStatus Initialize(void *platformSpecific = NULL) = 0;
    void Terminate();

    //
    // Configuration Methods
    //
    virtual WVStatus ConfigureCredentials(const WVCredentials &credentials) = 0;
    virtual WVStatus ConfigureBandwidthCheck(const std::string &url,
                        unsigned long intervalSeconds) = 0;
    virtual WVStatus ConfigureBufferSize(unsigned long newSize) = 0;
    virtual WVStatus ConfigureProxy(const WVProxySettings &proxySettings) = 0;

    virtual WVStatus ConfigureQualityCallback(WVQualityCallback callback, void *userData) = 0;
    virtual WVStatus ConfigureBandwidthCallback(WVBandwidthCallback callback, void *userData) = 0;
    virtual WVStatus ConfigureEventCallback(WVEventCallback callback, void *userData) = 0;
    virtual WVStatus ConfigureErrorCallback(WVErrorCallback callback,
                        unsigned long warningTimeout, void *userData) = 0;
    virtual WVStatus ConfigureTimeCallback(WVTimeCallback callback,
                        const std::string &format, void *userData) = 0;
    virtual WVStatus ConfigureBufferLevelCallback(WVBufferLevelCallback callback,
						  float backgroundReportingSecs,
						  float deltaReportingSecs,
						  float lowThreshold,
						  float normalThreshold,
						  void *userData) = 0;

    //
    //  Session Establishment Methods
    //
    virtual WVStatus OpenURL(WVSession *&session, const std::string &url) = 0;
    virtual WVStatus Close(WVSession *&session) = 0;

    //
    //  Transport Control Methods
    //
    virtual WVStatus Play(WVSession *session, float speedRequested, float *speedUsed) = 0;
    virtual WVStatus Pause(WVSession *session) = 0;
    virtual WVStatus Stop(WVSession *session) = 0;
    virtual WVStatus Seek(WVSession *session, const std::string &time) = 0;

    //
    // Multi-Track Methods
    //

    //
    // METHOD: SelectTrack
    //
    // This method is used to explicitly select an adaptive video track or
    // separate audio track.
    //
    // Parameters:
    //    [in] session - The session to control
    //
    //    [in] trackId - The track ID for the track to be selected.
    //                   If an adaptive video track ID is specified, then a
    //                   video track change will be effected.  Specifying
    //                   UNSPECIFIED_TRACK_ID will resume normal adaptive
    //                   video behavior.
    //                   If a separate audio track ID is specified, then
    //                   an audio track change will be effected.
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus SelectTrack(WVSession* session, WVUnsignedInt trackId);

    //
    // METHOD: GetCurrentAudioTrack
    //
    // This method populates a WVDicionary all the information available for the
    // current separate audio track, as well as its track ID.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] track - WVDictionary which on return will contain the information
    //                  available for the current adaptive video track
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.  This function returns the status
    //     WV_Status_Warning_Not_Available if separate audio is not available.
    //
    WVStatus GetCurrentAudioTrack(WVSession *session, WVDictionary& track);

    //
    // METHOD: GetCurrentVideoTrack
    //
    // This method populates a WVDicionary all the information available for the
    // current adaptive video track, as well as its track ID.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] track - WVDictionary which on return will contain the information
    //                  available for the current adaptive video track
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus GetCurrentVideoTrack(WVSession *session, WVDictionary& track);

    //
    // METHOD: GetSubtitleTracks
    //
    // This method populates a WVTypedValueArray containing a WVDictionary entry
    // for each separate subtitle track in the media.  These dicionaries contain all
    // the information available for subtitle.
    //
    // NOTE: Subtitle tracks cannot be selected.  They are meant to be rendered by
    //       the device media player.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] tracks - WVTypedValueArray which on return will contain an array
    //                   of subtile track info WVDictionary(s).
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus GetSubtitleTracks(WVSession *session, WVTypedValueArray& tracks);

    //
    // METHOD: GetAudioTracks
    //
    // This method populates a WVTypedValueArray containing a WVDictionary entry
    // for each separate audio track in the media.  These dicionaries contain all
    // the information available for the audio, as well as the track ID.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] tracks - WVTypedValueArray which on return will contain an array
    //                   of audio track info WVDictionary(s).
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus GetAudioTracks(WVSession *session, WVTypedValueArray& tracks);

    //
    // METHOD: GetVideoTracks
    //
    // This method populates a WVTypedValueArray containing a WVDictionary entry
    // for each adaptive video track in the media.  These dicionaries contain all
    // the information available for the video, as well as the track ID.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] tracks - WVTypedValueArray which on return will contain an array
    //                   of video track info WVDictionary(s).
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus GetVideoTracks(WVSession *session, WVTypedValueArray& tracks);

    //
    // Information Methods
    //

    //
    // METHOD: GetVersion
    //
    // Query the version number of the Widevine library
    //
    // Parameters:
    //     void
    //
    // Returns:
    //     Widevine library build version string
    //
    static std::string GetVersion();

    //
    // METHOD: UniqueID
    //
    // Get the Widevine unique identifier for this device
    //
    // For CE devices, this will be from the Widevine Keybox.
    // For Desktop platforms, it will be a persistent GUID
    //     generated by Widevine.
    // For iPhone, it will be the unit's serial number.
    //
    // Parameters:
    //     None
    //
    // Returns:
    //     On success, string containing the unique ID of the device
    //     On error, empty string is returned
    //
    static std::string UniqueID();

    // format can be "npt" for times of the form "hh:mm:ss.msec"
    // or "sec" for times of the form "seconds.msec"
    virtual std::string GetDuration(WVSession *session, const std::string &format) = 0;


    // METHOD: GetCurrentBandwidth
    //
    // This method retrieves information about the adaptive streaming bandwidth.
    // for the media stream that was setup on the specified session.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] bandwidth - The currently observered average network throughput
    //                      in bits per second
    //
    // Returns:
    //     WV_Status_OK on success, otherwise one of the WVStatus values
    //     indicating the specific error.
    //
    WVStatus GetCurrentBandwidth(WVSession *session, unsigned long& bandwidth);


    // METHOD: GetAdaptiveBitrateInfo
    //
    // This method provides adaptive streaming metrics for the media stream
    // that was set up on the specified session.  This information may be used,
    // for example, to display bit rate metering information on the UI to
    // indicate quality of service.
    //
    // Example: if the multi-rate file  contains low, mid and high rate encoded
    // streams at 700kb, 1.5Mb and 2.5Mb respectively and the lowest bit rate
    // file is currently being streamed, then on return, encodedRates =
    // {700000, 1500000, 2500000}, ratesReturned = 3 and currentIndex = 0.
    //
    // Parameters:
    //    [in] session - The session to query
    //
    //    [out] encodedRates - The bit rate of each separate encoding of the
    //          asset in the multi-rate encoded file, in bits per second.
    //
    //    [out] currentIndex  The index of the rate in encodedRates that is
    //          currently streaming. If none, currentIndex == ratesReturned
    //
    //   Returns:
    //      WV_Status_OK on success, otherwise one of the WVStatus values
    //      indicating the specific error.
    //
    WVStatus GetAdaptiveBitrateInfo(WVSession *session, std::vector<unsigned long>& encodedRates, size_t& currentIndex);

    //
    // METHOD: GetPlatformSpecificData
    //
    // Retrieves any platform-specific data that the app needs from the
    // hardware, such as device handles or pointers to session objects.  The
    // data returned by this function is platform-specific, and this function
    // may not be relevant on all platforms.
    //
    // Parameters:
    //     [out] platformSpecific - Pointer to set to the platform-specific
    //                              data.  How this pointer is used is
    //                              platform-specific.
    //
    // Returns:
    //     * WV_Status_Not_Implemented if this function is not supported on the
    //       current platform.
    //     * WV_Status_OK on success.
    //     * Otherwise one of the WVStatus values indicating the specific
    //       error.
    //
    virtual WVStatus GetPlatformSpecificData(void *platformSpecific) = 0;

protected:
    // Singleton constructor/destructor are protected
    WVPlaybackAPI() {}
    virtual ~WVPlaybackAPI() {}

private:
    static WVPlaybackAPI *sInstance;
};

#endif
