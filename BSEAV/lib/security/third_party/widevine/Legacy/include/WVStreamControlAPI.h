//
// Declarations for Widevine Adaptive Streaming API
//
// Copyright 2011 Widevine Technologies, Inc., All Rights Reserved
//

#ifndef __WV_STREAM_CONTROL_API_H__
#define __WV_STREAM_CONTROL_API_H__

#include <string>
#include <vector>
#include "WVStatus.h"
#include "WVTypes.h"
#include "WVControlSettings.h"

#if OS_WIN
#ifdef CLIENTAPI_EXPORTS
#define CLIENT_API __declspec(dllexport)
#elif !defined(CLIENT_API_STATIC)
#define CLIENT_API __declspec(dllimport)
#else
#define CLIENT_API
#endif
#else
#define CLIENT_API
#endif

typedef enum WVLogging {
    WV_Logging_HTTP = 1,
    WV_Logging_ParseOutput = 2,
} WVLogging;

typedef enum WVOutputFormat {
    WV_OutputFormat_PS = 0,         // Generic program stream
    WV_OutputFormat_DVD_Video = 1,  // DVD-Video program stream
    WV_OutputFormat_TS = 2,         // Transport Stream
    WV_OutputFormat_ES = 3,         // Elementary Streams
    WV_OutputFormat_DVD_Video_No_EAC3_Framing = 4       // DVD-Video without Dolby Digital+ audio framing
} WVOutputFormat;


// ES selectors for WV_GetEsData
typedef enum WVEsSelector {
    WV_EsSelector_Audio = 0,    // Audio ES data
    WV_EsSelector_Video = 1     // Video ES data
} WVEsSelector;


#define UNSPECIFIED_TRACK_ID WVUnsignedInt(0)


class WVSession;


// Provide info on socket open/close operations for bandwidth attribution
// status: {0=open, 1=close}
typedef void (*WVSocketInfoCallback)(int fd, int status, void * context);

//
// CALLBACK: WVDecryptCallback
//
// Gets called to decrypt individual crypto units
//
// Parameters:
//     [in] es_type - Type of elementary stream, indicating audio or video
//     [in] input -   Pointer to encrypted crypto unit
//     [in] output -  Pointer or handle to receive decrypted crypto unit
//     [in] length -  Length of crypto unit
//     [in] key -     ID of key to use for decryption
//     [in] dts -     Decode timestamp, 90 KHz clock
//     [in] pts -     Presentation timestamp, 90 KHz clock
//     [in] au_end -  Indicates end of Access Unit (last CU in frame)
//     [in] context - Client context established from WV_Setup
//
// Returns:
//     - WV_Status_OK:               Decryption succeeded
//     - WV_Status_Warning_Need_Key: Indicates key not yet available, will
//                                   be retried
//     - Error code indicating problem.  Causes playback to halt.
//
typedef WVStatus (*WVDecryptCallback)(WVEsSelector es_type, void* input, void* output, size_t length,
                                      int key, unsigned long long dts, unsigned long long pts,
                                      bool au_end, void *context);

enum WVDecryptCallbackMode {
    WVDecryptCallbackMode_MultiCU =  0,      // WVDecryptCallback called repeatedly until buffer exhausted
    WVDecryptCallbackMode_SingleCU = 1,      // WVDecryptCallback called once
    WVDecryptCallbackMode_WholeAU = 2,       // WVDecryptCallback called repeatedly until AU complete
};


class WVCallbacks {
 public:
 WVCallbacks() :
    pushData(NULL),
    response(NULL),
    allocBuffer(NULL),
    freeBuffer(NULL),
    getDeviceID(NULL),
    getKeyboxPath(NULL),
    decrypt(NULL),
    decryptMode(WVDecryptCallbackMode_MultiCU),
    socketInfo(NULL)
    {}

    void (*pushData)(unsigned short port, void *buffer, size_t length);
    void (*response)(WVSession *session, const std::string &response);
    void *(*allocBuffer)(size_t);
    void (*freeBuffer)(void *);

    // Get unique device ID.  Return number of bytes inserted into buffer not to exceed size
    int (*getDeviceID)(char *buffer, size_t size);

    int (*getKeyboxPath)(char* path, size_t size);

    WVDecryptCallback decrypt;
    WVDecryptCallbackMode decryptMode;

    // publish info about descriptors used for streaming for accounting purposes
    WVSocketInfoCallback socketInfo;
};


//
// METHOD: WV_Initialize
//
// Initialize Widevine stream control API.  Must be called before any other
// WV API functions.
//
// Parameters:
//     [in] callbacks - Specifies callback functions in client
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_Initialize(const WVCallbacks *callbacks);

//
// METHOD: WV_Terminate
//
// Terminate Widevine stream control API and release all resources.  No other
// WV API functions may be called after calling WV_Terminate.
//
// Parameters:
//     N/A
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_Terminate();


//
// METHOD: WV_SetWarningToErrorMS
//
// Set time to return warnings before returning error
//
// Parameters:
//     [in] warningToErrorMS - number of milliseconds after which warning
//                             returns from WV_GetData turn to errors
//
// Returns:
//     N/A
//
CLIENT_API void
WV_SetWarningToErrorMS(long warningToErrorMS);

//
// METHOD: WV_SetLogging
//
// Set various run time logging and checking options
//
//
// Parameters:
//     [in] flags - facilities to enable. See WVLogging for details
//
//
// Returns:
//     N/A
//
CLIENT_API void
WV_SetLogging(unsigned long flags);


// METHOD: WV_AddMediaFilter
//
// Add filtering options to specify which kind of media formats the device is able to play,
// or should be able to play based on other considerations.  This function should
// be called after WV_Initialize, but before WV_Setup.  This function may be called multiple
// times to specify multiple filters.
//
// Parameters:
//    [in] selector - Specifies the encoding parameter to which the filter applies.  The same
//                    selector may be used in multiple calls to WV_AddMediaFilter, in which case
//                    media matching any of the filters for that selector will pass.  If a particular
//                    selector is not used in any calls to WV_AddMediaFilter, this is akin to an
//                    pass for all values for that selector.
//
//    [in] operator - Specifies the comparison operator which will be applied to the value for
//                    the specified selector.
//
//    [in] value -    Specifies the value for comparsion.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

typedef enum WVMediaFilterSelector {
    WV_MediaFilterSelector_Null = 0,              // NULL selector
    WV_MediaFilterSelector_Bandwidth = 1,         // Any operator for comparison. Value is bits/sec
    WV_MediaFilterSelector_VideoType = 2,         // Only Operator_EQ for comparison. Value is WVVideoType
    WV_MediaFilterSelector_VideoWidth = 3,        // Any operator for comparison. Value is width, in pixels
    WV_MediaFilterSelector_VideoHeight = 4,       // Any operator for comparison. Value is height, in lines
    WV_MediaFilterSelector_VideoAspectRatio = 5,  // Any operator for comparison.  Value is width/height, adjusted for pixel aspect ratio
    WV_MediaFilterSelector_AudioType = 6,         // Only Operator_EQ for comparsion. Value is WVAudioType
    WV_MediaFilterSelector_AudioChannels = 7,     // Any operator for comparison. Value is number of audio channels
    WV_MediaFilterSelector_H264Profile = 8,       // Any operator for comparison. Value is profile_idc
    WV_MediaFilterSelector_H264Level = 9,         // Any operator for comparison. Value is level_idc
    WV_MediaFilterSelector_AACProfile = 10        // Any operator for comparison. Value is Object Profile ID
} WVMediaFilterSelector;

typedef enum WVMediaFilterOperator {
    WV_MediaFilterOperator_Null = 0,              // NULL operator
    WV_MediaFilterOperator_LT = 1,                // Media value is Less Than specified value
    WV_MediaFilterOperator_LE = 2,                // Media value is Less than or Equal to specified value
    WV_MediaFilterOperator_EQ = 3,                // Media value EQuals specified value
    WV_MediaFilterOperator_GE = 4,                // Media value is Greater than or Equal to specified value
    WV_MediaFilterOperator_GT = 5                 // Media value is Greater Than specified value
} WVMediaFilterOperator;

CLIENT_API WVStatus
WV_AddMediaFilter(WVMediaFilterSelector sel, WVMediaFilterOperator op, double value);


// METHOD: WV_ResetMediaFilter
//
// Resets media filters set up with WV_AddMediaFilter
//
// Parameters:
//    none
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_ResetMediaFilter();


//
// METHOD: WV_StartBandwidthCheck
//
// Initiate a bandwidth check on the network connection.  This function starts
// a background thread that will perform the bandwidth check and returns
// immediately. The bandwidth measurement is performed by downloading a portion
// of the test file specified by the URL parameter. WV_StartBandwidthCheck may be
// used prior to WV_Setup/WV_Play to determine if the connection has sufficient
// minimum bandwidth for the service.  A bandwidth check should be performed
// prior to any WV_Setup requests playback to allow an appropriate initial
// asset bitrate to be selected. It is an error to call WV_StartBandwidthCheck
// while a check is already in progress. If WV_Setup, WV_Play are called before
// the bandwidth check completes, playback will start up at the lowest bitrate
// and there may not be sufficient bandwidth for playback, so it is recommended
// that the bandwidth check is allowed to complete before starting playback.
// WV_Terminate will cancel a bandwidth check that is in progress.
//
// Parameters:
//    [in] url - The URL of a test file.  The test file should be located on the
//               same server/CDN as the actual content to be streamed.
//
//    [in] proxy - A structure of proxy settings if a proxy server is to be used
//
// Returns:
//    WV_Status_OK on success, otherwise one of the WVStatus values
//    indicating the specific error.
//

// version without proxy
CLIENT_API WVStatus
WV_StartBandwidthCheck(const std::string &url);

// version with proxy
CLIENT_API WVStatus
WV_StartBandwidthCheck(const std::string &url, WVProxySettings &proxy);

//
// METHOD: WV_GetBandwidthCheckStatus
//
// Test the status of a bandwidth check that was initiated by a previous call to
// WV_StartBandwidthCheck.   The status value returned from this function will
// indicate one of the following cases: (1) Bandwidth check in progress  the
// check has not yet completed, no bandwidth value is returned and the return
// status is WV_Status_Checking_Bandwidth (2) An error occurred during bandwidth
// checking.  The return status is the error code representing the error condition
// (3)  Bandwidth check complete.  In this case, *bandwidth is set to the detected
// connection bandwidth and WV_Status_OK is returned.  Once a bandwidth check has
// completed, subsequent calls to WV_GetBandwidthCheckStatus will return the
// previously calculated bandwidth value until a new bandwidth check is initiated
// with WV_StartBandwidthCheck().
//
CLIENT_API WVStatus
WV_GetBandwidthCheckStatus(unsigned long *bandwidth);


//
// METHOD: WV_CheckBandwidth
//
// Perform a bandwidth check on the network connection.  The bandwidth measurement
// is performed by downloading a portion of the test file specified by the URL
// parameter. WV_CheckBandwidth may be used prior to WV_Setup/WV_Play to determine
// if the connection has sufficient minimum bandwidth for the service.
//
// Parameters:
//    [in] url - The URL of a test file.  The test file should be located on the
//               same server/CDN as the actual content to be streamed.
//
//    [out] bandwidth - The connection bandwidth in bits per second
//
//
//    [in] proxy - A structure of proxy settings if a proxy server is to be used
//
// Returns:
//    WV_Status_OK on success, otherwise one of the WVStatus values
//    indicating the specific error.
//

// version without proxy
CLIENT_API WVStatus
WV_CheckBandwidth(const std::string &url, unsigned long *bandwidth);

//version with proxy
CLIENT_API WVStatus
WV_CheckBandwidth(const std::string &url, unsigned long *bandwidth, const WVProxySettings& proxy);


//
// METHOD: WV_Setup
//
// Initiate a session between the Widevine stream control API and a client.
// Allocates resources for a stream.
//
// Parameters:
//    [out] session - The supplied pointer to session parameter will be assigned
//                    a newly constructed session object.  The caller needs to
//                    dispose of this object with WV_Teardown
//
//    [in] url - The URL of the media asset to setup
//
//    [in] transport - Indicates which transport protocol is to be used and
//         configures its parameters such as destination address and destination
//         port for a single stream.
//
//    [in] credentials - A structure of information that is used to authenticate
//         the user's stream request.
//
//         The syntax for the transport specifier is
//
//         transport/profile/lower-transport.
//
//    [in] proxy - A structure of proxy settings if a proxy server is to be used
//
//    [in] outputFormat - Indicates the format of the output to be fed into the
//         decoder.
//
//   Below are the allowed configuration parameters associated with transport:
//
//   destination:
//          The destination to which a stream will be sent.  For the direct
//          function call data pull model, use "destination=getdata".
//          For the direct function call data push model use
//          "destination=callback"
//
//   client_port:
//          This parameter provides the unicast RTP port on which the client
//          has chosen to receive media data. Note that RTCP is not used in this
//          localhost implementation, so the client_port takes a single value not
//          a range.  When using the direct function call data push model, you
//          should specify a port value that will be returned with the callback
//          data such that the client can associate the data with a specified
//          session.
//
//   Examples:
//          RTP/AVP/UDP;unicast;client_port=3456
//          RAW/RAW/UDP;unicast;client_port=3456
//          RAW/RAW/RAW;destination=callback;client_port=3456
//          RAW/RAW/RAW;destination=getdata
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

// Setup without a proxy (backward compatible).
//This form is deprecated and may be removed in a future release
CLIENT_API WVStatus
WV_Setup(WVSession *&session, const std::string &url,
        const std::string &transport, WVCredentials &credentials,
        WVOutputFormat outputFormat = WV_OutputFormat_PS,
        unsigned long bufferSize = 10 * 1024 * 1024, void *context = NULL);

// Setup using a proxy
CLIENT_API WVStatus
WV_Setup(WVSession *&session, const std::string &url,
        const std::string &transport, WVCredentials &credentials,
	    WVProxySettings &proxy, WVOutputFormat outputFormat = WV_OutputFormat_PS,
	    unsigned long bufferSize = 10 * 1024 * 1024, void *context = NULL);

//
// An alternate form of WV_Setup that receives data from a file source instead of accessing
// directly via a URI.  This method of accessing the source data has limitations, it should
// only be used in specific cases where the URI is not available.
//

class WVFileSource {
public:
    virtual unsigned long long GetSize() = 0;
    virtual unsigned long long GetOffset() = 0;

    virtual void Seek(unsigned long long offset) = 0;
    virtual size_t Read(size_t amount, unsigned char *buffer) = 0;
    virtual ~WVFileSource() {}
};

CLIENT_API WVStatus
WV_Setup(WVSession *&session, WVFileSource *source, const std::string &transport,
        WVCredentials &credentials, WVOutputFormat outputFormat = WV_OutputFormat_PS,
         unsigned long bufferSize = 10 * 1024 * 1024, void *context = NULL);

//
// METHOD: WV_IsWidevineMedia
//
// Given a buffer of data, determine if the media in the buffer is Widevine
// content (.wvm format)
//
// Parameters:
//    [in] buffer - a buffer containing the leading segment of the media
//
//    [in] length - the number of bytes in the buffer
//
// Returns:
//     true if content is widevine encrypted, false otherwise
//
CLIENT_API bool
WV_IsWidevineMedia(const char *buffer, size_t length);

//
// METHOD: WV_ConfigureHeartbeat
//
// Provides a method of configuring the heartbeat after WV_Setup has been
// called.
//
//
// Parameters:
//     [in] heartbeatURL - the URL of the heartbeat server
//     [in] heartbeatPeriod - the heartbeat interval in seconds
//     [in] assetId - the asset ID
//     [in] deviceId - the device ID
//     [in] streamId - the streamID
//     [in] userData - user data
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_ConfigureHeartbeat(WVSession *session, std::string &serverUrl, unsigned int period,
                      unsigned long assetId, std::string &deviceId,
                      std::string &streamId, std::string &userData);

//
// METHOD: WV_Teardown
//
// Teardown a session and release all associated resources.  The session
// object will be deleted and the pointer set to NULL.
//
// Parameters:
//    [in] session - The session to be torn down.  The object pointed to
//                   by session will be deleted.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_Teardown(WVSession *&session);

//
// METHOD: WV_Play
//
// Initiate playback for a session.
//
// The WV_Play method tells the server to start sending data via the
// mechanism specified in WV_Setup. A client MUST NOT issue a WV_Play
// request until any outstanding WV_Setup requests have been
// acknowledged as successful.
//
// The WV_Play method positions the normal play time to the beginning
// of the range specified and delivers stream data until the end of the
// range is reached.
//
// Parameters:
//    [in] session - The session for which to initiate playback
//
//    [in] scale_requested - A scale value of 1 indicates normal play or record
//         at the normal forward viewing rate. If not 1, the value corresponds
//         to the rate with respect to normal viewing rate. For example, a
//         ratio of 2 indicates twice the normal viewing rate ("fast forward")
//         and a ratio of 0.5 indicates half the normal viewing rate. In other
//         words, a ratio of 2 has normal play time increase at twice the
//         wallclock rate. For every second of elapsed (wallclock) time, 2
//         seconds of content will be delivered. A negative value indicates
//         reverse direction.
//
//    [out] scale_used -  The server will try to approximate the requested
//         scale in scale_requested, but may restrict the range of scale
//         values that it supports. On exit, the scale actually used will be
//         returned in the variable pointed to by this parameter.
//
//    [in] range - The time range that playback is to start/stop.  Valid time
//         values are according to the RTSP npt-range header, e.g.:
//
//    npt-range    =   ( npt-time "-" [ npt-time ] ) | ( "-" npt-time )
//    npt-time     =   "now" | npt-sec | npt-hhmmss
//    npt-sec      =   1*DIGIT [ "." *DIGIT ]
//    npt-hhmmss   =   npt-hh ":" npt-mm ":" npt-ss [ "." *DIGIT ]
//    npt-hh       =   1*DIGIT     ; any positive number
//    npt-mm       =   1*2DIGIT    ; 0-59
//    npt-ss       =   1*2DIGIT    ; 0-59
//
//    Examples:
//      npt=123.45-125
//      npt=12:05:35.3-
//      npt=now-
//
//      The syntax conforms to ISO 8601. The npt-sec notation is optimized
//      for automatic generation, the ntp-hhmmss notation for consumption
//      by human readers. The "now" constant allows clients to request to
//      receive the live feed rather than the stored or time-delayed
//      version. This is needed since neither absolute time nor zero time
//      are appropriate for this case.
//
//    [in] audioTrack - The ID of the audio track to be used. This value can be
//         from the audio track dictionaries obtained via WV_Info_GetAudioTracks.
//         This value is optional.  If UNSPECIFIED_TRACK_ID is used, the default
//         audio track, which is the first encoded into the media package, or
//         the last selected audio track is used.
//
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_Play(WVSession *session, float scale_requested, float *scale_used,
	const std::string &range, WVUnsignedInt audioTrack = UNSPECIFIED_TRACK_ID);

//
// METHOD: WV_Pause
//
// The WV_Pause method causes the stream delivery to be interrupted
// (halted) temporarily. Any server resources are kept.
//
//   The PAUSE request may contain a time parameter specifying when the
//   stream is to be halted. We refer to this point as the "pause point".
//   The time must contain exactly one value rather than a time range. The
//   normal play time for the stream is set to the pause point. The pause
//   request becomes effective the first time the server is encountering
//   the time point specified in any of the currently pending PLAY requests.
//   If a time outside the currently pending PLAY request is specified, the
//   error "457 Invalid Range" is returned. If a media unit (such as an audio
//   or video frame) starts presentation at exactly the pause point, it is
//   not played. If the time parameter is the empty string "", stream
//   delivery is interrupted immediately on receipt of the message and the
//   pause point is set to the current normal play time.
//
//   A subsequent WV_Play method with "now-" range value resumes from the
//   pause point.
//
// Parameters:
//    [in] session - The session to pause
//
//    [in] time - The pause point, as described above.  See WV_Play for a
//                description of time formats.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_Pause(WVSession *session, const std::string &time);

//
// METHOD: WV_Info_GetVersion
//
// Query the version number of the Widevine library
//
// Parameters:
//     void
//
// Returns:
//     Widevine library build version string
//
CLIENT_API const char *
WV_Info_GetVersion();

//
// METHOD: WV_Info_GetTime
//
// The WV_Info_GetTime method returns the current play time of the specified
// session.
//
// Parameters:
//    [in] session - The session to query
//
//    [in] format - "npt" or "smpte".  npt is the default.
//
// Returns:
//     The current time as a std::string in the requested format
//
//
CLIENT_API std::string
WV_Info_GetTime(WVSession *session, const std::string &format = "npt");


// METHOD: WV_TimestampToMediaTime
//
// This method is used to determine media timestamps (SCR or PTS) from media time,
// automatically compenstaing for trick play.
//
// Parameters:
//    [in] session - The session from which data is requested
//
//    [in] timestamp    - Media timestamp (SCR or PTS). Up to
//                        300 seconds (real time) in the past
//
//    [in] type         - The type of the first paramter (SCR or PTS).
//
//    [in] format - "npt" or "smpte".  npt is the default.
//
// Returns:
//     The media time corresponding to the timestamp if available, or an empty
//     string otherwise
//
typedef enum {
    SCR = 0,
    PTS = 1
} TimestampType;
CLIENT_API std::string
WV_TimestampToMediaTime(WVSession* session, unsigned long long timestamp,
        TimestampType type = PTS, const std::string &format = "npt");


//
// METHOD: WV_GetData
//
// This method is used when the client chooses to receive multiplexed stream data directly via
// function call using a pull data model.  In this case, the transport parameter to the
// WV_Setup method should include destination configuration of "destination=getdata"
//
// Parameters:
//    [in] session - The session from which data is requested
//
//    [in] buffer - A pointer to the memory buffer to receive the data
//
//    [in] request_size - Number of bytes requested.  It is the caller's responsibility to
//                        make sure the buffer is large enough to receive this many bytes.
//
//    [out] return_size - On return, set to the number of bytes written into the buffer
//
//    [in] timeout_sec - Unused, set to 0
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_GetData(WVSession *session, unsigned char *buffer, size_t request_size,
           size_t *return_size, unsigned long timeout_sec);


//
// METHOD: WV_GetEsData
//
// This method is used when the client chooses to receive elementary stream data directly via
// function call using a pull data model.  This method always returns data corresponding to
// at most one single access unit.
//
// Parameters:
//    [in] session - The session from which data is requested
//
//    [in] es_selector - Selector specifying which elementary stream data to receive.
//
//    [in] buffer - A pointer to the memory buffer to receive the data
//
//    [in] request_size - Number of bytes requested.  It is the caller's responsibility to
//                        make sure the buffer is large enough to receive this many bytes.
//
//    [out] return_size - On return, set to the number of bytes written into the buffer
//
//    [out] dts - On return, set to MPEG-2 access unit decoding timestamp (DTS)
//
//    [out] pts - On return, set to MPEG-2 access unit presentation timestamp (PTS)
//
//    [out] sync_frame - On return, indicates whether the data belongs to a sync frame
//                       (video key frame, or audio frame).
//
//    [out] au_end - On return, indicates whether the data belongs to the end of the access unit.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_GetEsData(WVSession *session, WVEsSelector es_selector,
	     unsigned char *buffer, size_t request_size, size_t& return_size,
	     unsigned long long& dts, unsigned long long& pts, bool& sync_frame, bool& au_end);


//
// METHOD: WV_GetNextEsDataBufferSize
//
// This method returns the optimal size of the buffer size that should be used in the next
// call to WV_GetEsData.  Calculation is dependent on whether a decryption callback is used
// and if so, the decrypt callback mode (WVDecryptCallbackMode), returning the total size of
// remaining crypto units in the current access unit (no decrypt callback or
// WVDecryptCallbackMode_MultiCU specified), the size of the next crypto unit
// (decrypt callback and WVDecryptCallbackMode_SingleCU specified), or the size of the next
// access unit (decrypt callback and WVDecryptCallbackMode_WholeAU specified)
//
// Parameters:
//    [in] session - The session from which data is requested
//
//    [in] es_selector - Selector specifying the elementary stream targetted.
//
//    [out] buffer_size - The size of the buffer
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus
WV_GetNextEsDataBufferSize(WVSession* session, WVEsSelector es_selector, size_t &buffer_size);


//
// METHOD: WV_Info_GetDuration
//
// The WV_Info_GetDuration method returns the length of the current media file
// that was set up for the session.  The length is returned in either npt or smpte
// time format, as desired.
//
// Parameters:
//    [in] session - The session to query
//
//    [in] format - "npt" or "smpte".  npt is the default.
//
// Returns:
//     The media file duration as a std::string in the requested format
//     If the duration of the asset is not yet available, the empty string "" is returned.
//
//
CLIENT_API std::string
WV_Info_GetDuration(WVSession *session, const std::string &format = "npt");

//
// METHOD: WV_Info_GetSystemType
//
// The WV_Info_GetSystemType method returns the container format type
// for the specified session.
//
// Parameters:
//    [in] session - The session to query
//
// Returns:
//     The container format of the media stream
//
typedef enum WVSystemType {
    WV_System_Unknown      = 0x00000000,
    WV_System_MPEG1_System = 0x00010000,
    WV_System_MPEG2_TS     = 0x00020000,
    WV_System_MPEG2_PS     = 0x00030000,
    WV_System_MP4          = 0x00040000,
    WV_System_AVI          = 0x00050000,
    WV_System_GIF          = 0x00060000,
    WV_System_JPG          = 0x00070000,
    WV_System_MP3          = 0x00080000,
    WV_System_DAT          = 0x00090000,
    WV_System_SVCD         = 0x000a0000,
    WV_System_VOB          = 0x000b0000,
    WV_System_None         = 0x00ff0000
} WVSystemType;

CLIENT_API WVSystemType
WV_Info_GetSystemType(WVSession *session);


//
// METHOD: WV_Info_GetAudioConfiguration
//
//   The WV_Info_GetAudioConfiguration method returns information pertinent to the audio stream
//   within the current asset.
//
// Parameters:
//   [in] session - the session to query
//   [out] streamType - The type of audio stream (NULL if not queried)
//   [out] streamID - The stream PES stream_id (NULL if not queried)
//   [out] profile - The audio encoding profile, if applicable (NULL if not queried)
//   [out] numChannels - The audio channel configuration (NULL if not queried)
//   [out] sampleFrequency - The audio sampling frequency (NULL if not queried)
//   [out] bitRate - The audio bit rate (NULL if not queried)
//
// Returns:
//   WV_Status_OK on success, otherwise one of the WVStatus values indicating the specific error.
//

typedef enum WVAudioType {
    WV_AudioType_None      = 0x00000000,
    WV_AudioType_AAC       = 0x00000001,
    WV_AudioType_EAC3      = 0x00000002,
    WV_AudioType_DTS       = 0x00000003,
    WV_AudioType_Unknown   = 0x000000ff
} WVAudioType;

CLIENT_API WVStatus
WV_Info_GetAudioConfiguration(WVSession *session,
        WVAudioType *streamType,
        unsigned short *streamID,
        unsigned short *profile,
        unsigned short *numChannels,
        unsigned long *sampleFrequency,
        unsigned long *bitRate);


//
// METHOD: WV_Info_GetVideoConfiguration
//
// The WV_Info_GetVideoConfiguration method returns information pertinent
// to the video stream within the current asset.
//
// Parameters:
//   [out] streamType - The type of video stream (NULL if not queried)
//   [out] streamID - The stream PES stream_id (NULL if not queried)
//   [out] profile - The video encoding profile, if applicable (NULL if not queried)
//   [out] level - The video encoding profile level, if applicable (NULL if not queried)
//   [out] width - The video horizontal resolution (NULL if not queried)
//   [out] height - The video vertical resolution (NULL ifnot queried)
//   [out] pixelAspectRatio - The Aspect ratio (w/h) for the video samples (NULL if not required)
//         May be 0 if not available
//   [out] frameRate - The video frame rate (NULL if not queried)
//   [out] bitRate - The audio bit rate (NULL if not queried)
//
// Returns:
//   WV_Status_OK on success, otherwise one of the WVStatus values indicating the specific error.
//

typedef enum WVVideoType {
    WV_VideoType_None      = 0x000000000,
    WV_VideoType_H264      = 0x000000001,
    WV_VideoType_Unknown   = 0x0000000ff
} WVVideoType;

CLIENT_API WVStatus
WV_Info_GetVideoConfiguration(WVSession *session,
        WVVideoType *streamType,
        unsigned short *streamID,
        unsigned short *profile,
        unsigned short *level,
        unsigned short *width,
        unsigned short *height,
        float *pixelAspectRatio,
        float *frameRate,
        unsigned long *bitRate);


//
// METHOD: WV_Info_GetCodecConfig
//
// The WV_Info_GetCodecConfig method returns the 'avcC' or 'esds' configuration data.
//
// Parameters:
//   [in] configType - Either WV_CodecConfigType_AVCC or WV_CodecConfigType_ESDS
//   [out] config - A reference to a pointer that will receive the address of the data
//   [out] size - The number of bytes of data in config, or 0 if no data is available
//
// Returns:
//   WV_Status_OK on success, otherwise one of the WVStatus values indicating the specific error.
//   If the requested codec configuration data is not available, WV_Status_Warning_Not_Available
//   is returned.
//

typedef enum WVCodecConfigType {
    WV_CodecConfigType_AVCC =           	0x00000000,
    WV_CodecConfigType_ESDS =           	0x00000001,
    WV_CodecConfigType_EC3SpecificData = 	0x00000002,
    WV_CodecConfigType_DTSSpecificData =	0x00000003
} WVCodecConfigType;

CLIENT_API WVStatus
WV_Info_GetCodecConfig(WVSession *session, WVCodecConfigType configType,
                       unsigned char const *& config, unsigned long &size);



//
// METHOD: WV_Info_GetNumberOfStreams
//
// Note:  This method is now obsolete.  Please use WV_Info_GetAudioConfiguration and
// WV_Info_GetVideoConfiguration instead.
//
// The WV_Info_GetNumberOfStreams method returns the number of streams
// (audio or video) in the multiplex that was set up for the specified
// session.
//
// Parameters:
//    [in] session - The session to query
//
// Returns:
//     The number of streams
//
CLIENT_API unsigned short
WV_Info_GetNumberOfStreams(WVSession *session);

//
// METHOD: WV_Info_GetStreamInfo
//
// Note:  This method is now obsolete.  Please use WV_Info_GetAudioConfiguration and
// WV_Info_GetVideoConfiguration instead.
//
// The WV_Info_GetStreamInfo method retrieves the type and stream ID of
// a selected stream in the multiplex that was set up for the specified
// session.
//
// Parameters:
//    [in] session - The session to query
//
//    [in] streamNumber - The number of the stream.  Valid stream numbers
//         range from 0..WV_Info_GetNumberOfStreams() - 1
//
//    [out] streamType - A pointer to a variable that will be set to the
//                       type of the stream selected by streamNumber
//
//    [out] streamID - A pointer to a variable that will be set to the
//                     stream ID selected by streamNumber
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

//
// Note:  WVStreamType is now obsolete.  Please use WVAudioType and WVVideoType instead.
//
typedef enum WVStreamType {
    WV_StreamType_Unknown         = 0x00000000,
    WV_StreamType_Audio_MPEG1     = 0x00000001,
    WV_StreamType_Audio_MPEG2     = 0x00000002,
    WV_StreamType_Audio_AAC       = 0x00000003,
    WV_StreamType_Audio_AC3       = 0x00000004,
    WV_StreamType_Audio_ADS       = 0x00000005,
    WV_StreamType_Audio_MPEG2_AAC = 0x00000006,
    WV_StreamType_Audio_MPEG4_AAC = 0x00000007,
    WV_StreamType_Audio_DTS       = 0x00000008,
    WV_StreamType_Audio_EAC3      = 0x00000009,
    WV_StreamType_Audio_None      = 0x000000ff,

    WV_StreamType_Video_Unknown   = 0x00000000,
    WV_StreamType_Video_MPEG1     = 0x00000100,
    WV_StreamType_Video_MPEG2     = 0x00000200,
    WV_StreamType_Video_MPEG4     = 0x00000300,
    WV_StreamType_Video_H264      = 0x00000400,
    WV_StreamType_Video_VC1       = 0x00000500,
    WV_StreamType_Video_None      = 0x0000ff00
} WVStreamType;

CLIENT_API WVStatus
WV_Info_GetStreamInfo(WVSession *session, unsigned short streamNumber,
                      WVStreamType *streamType, unsigned short *streamID);

//
// METHOD: WV_Info_GetAudioSampleRate
//
// Note:  This method is now obsolete.  Please use WV_Info_GetAudioConfiguration instead.
//
// The WV_Info_GetAudioSampleRate method retrieves the audio sample rate
// of the current stream in the multiplex that was set up for the specified
// session.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] sampleRate The audio sample rate for the specified stream, in Hz
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetAudioSampleRate(WVSession *session, unsigned long *sampleRate);


//
// METHOD: WV_Info_GetAudioBitrate
//
// Note:  This method is now obsolete.  Please use WV_Info_GetAudioConfiguration instead.
//
// The WV_Info_GetAudioBitRate method retrieves the average audio bit rate
// of the current stream in the multiplex that was set up for the specified
// session.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] bitRate - The average encoded audio bit rate for the specified
//          stream, in bits per second
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetAudioBitrate(WVSession *session, unsigned long *bitRate);


//
// METHOD: WV_Info_GetCurrentBandwidth
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

CLIENT_API WVStatus
WV_Info_CurrentBandwidth(WVSession *session, unsigned long *bandwidth);

//
// METHOD: WV_Info_TimeBuffered
//
// This method returns an approximate duration for the media buffered
// inside the Widevine adaptive client.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] secondsBuffered - Duration of the media currently
//                            buffered, in seconds
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_TimeBuffered(WVSession *session, float *secondsBuffered);


//
// METHOD: WV_Info_GetStats
//
// This method is used to query current stats from the Widevine
// adaptive client.
//
// Parameters:
//    [in] session - The session to query
//
//    [in,out] stats - Dictionary containing placeholders for the
//                     stats to be queried.  On return the available
//                     stats will be filled in.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetStats(WVSession *session, WVDictionary& stats);


//
// METHOD: WV_Info_GetAdaptiveBitrates
//
// This method provides adaptive streaming metrics for the media stream
// that was set up on the specified session.  This information may be used, for
// example, to display bit rate metering information on the UI to indicate
// quality of service.
//
// Example: if the multi-rate file  contains low, mid and high rate encoded
// streams at 700kb, 1.5Mb and 2.5Mb respectively and the lowest bit rate file
// is currently being streamed, then on return, encodedRates[] = {700000,
// 1500000, 2500000}, ratesReturned = 3 and currentIndex = 0.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] encodedRates - The bit rate of each separate encoding of the asset
//          in the multi-rate encoded file, in bits per second.
//
//    [in] ratesRequested - The maximum number of encoded rates that may be
//         returned in encodedRates, as determined by the number of elements
//         allocated in the array.
//
//    [out] ratesReturned  The number of encoded rates written to the encodedRates
//          array.
//
//    [out] currentIndex  The index of the rate in encodedRates that is currently
//          streaming.   If none, currentIndex == ratesReturned
//
//   Returns:
//      WV_Status_OK on success, otherwise one of the WVStatus values
//      indicating the specific error.
//
CLIENT_API WVStatus
WV_Info_GetAdaptiveBitrates(WVSession *session,
        unsigned long *encodedRates,
        size_t ratesRequested,
        size_t *ratesReturned,
        size_t *currentIndex);

//
// METHOD: WV_Info_GetCopyProtection
//
// This method retrieves information about the current copy protection settings
// as determined from the DRM license for the content.  The copy protection
// in the decoder (APS/Macrovision and/or HDCP) MUST be configured based on the
// information returned from this function.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] macrovisionLevel - The requested macrovision settings
//
//    [out] enableHDCP - If true, HDCP should be enabled on the HDMI output
//
//    [out] CIT : If true, Image Constraint is required.
//
//  GetCopyProtection without CGMS-A (backward compatible).  This form is
//  deprecated and may be removed in a future release
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

typedef enum WVMacrovision {
    WV_MacroVision_Type_None = 0,
    WV_MacroVision_Type_AGC_Only = 1,
    WV_MacroVision_Type_AGC_2Lines = 2,
    WV_MacroVision_Type_AGC_4Lines = 3
} WVMacrovision;

CLIENT_API WVStatus
WV_Info_GetCopyProtection(WVSession *session, WVMacrovision *macrovision, bool *enableHDCP, bool *CIT);

//
// METHOD: WV_Info_GetCopyProtection
//
// This method retrieves information about the current copy protection settings
// as determined from the DRM license for the content.  The copy protection
// in the decoder (APS/Macrovision and/or HDCP) MUST be configured based on the
// information returned from this function.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] macrovisionLevel - The requested macrovision settings
//
//    [out] emi - The requested EMI settings (use for digital or
//          analog copy generation control, e.g. CGMS-A, CGMS-D, etc.)
//
//    [out] enableHDCP - If true, HDCP should be enabled on the HDMI output
//
//    [out] CIT : If true, Image Constraint is required.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
typedef enum WVEMI {
    WV_EMI_Copy_Freely = 0,
    WV_EMI_Copy_No_More = 1,
    WV_EMI_Copy_Once = 2,
    WV_EMI_Copy_Prohibited = 3
} WVEMI;

CLIENT_API WVStatus
WV_Info_GetCopyProtection(WVSession *session, WVMacrovision *macrovision,
        WVEMI *emi, bool *enableHDCP, bool *CIT);


//
// METHOD: WV_Info_GetNumChapters
//
// This method retrieves the number of chapters encoded into the media.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] numChapters - The number of chapters encoded in the media
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetNumChapters(WVSession *session, unsigned long& numChapters);



//
// METHOD: WV_Info_GetChapterData
//
// This method retrieves information about chapters encoded in the media.
//
// Parameters:
//    [in] session - The session to query
//
//    [in] startSeqNum - The sequence number of the first chapter to query (0 start)
//
//    [in] endSeqNum - The sequence number of the last chapter to query (0 start)
//
//    [out] chapterData - An STL vector containing information about each chapter
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

struct WVChapterData {
    unsigned long seqNum;                           // Sequence number for this chapter
    std::string timeIndex;                   // The time index for the chapter in npt format
    std::string title;                       // The title for the chapter, if any
    std::vector<unsigned char> thumbnail;    // The JPEG thumnail data for the chapter
};

CLIENT_API WVStatus
WV_Info_GetChapterData(WVSession *session, unsigned long startSeqNum,
        unsigned long endSeqNum, std::vector<WVChapterData>& chapterData);


//
// METHOD: WV_Info_GetChapterSeqNum
//
// This method retrieves the sequence number of the chapter containing a specified
// time index.
//
// Parameters:
//    [in] session - The session to query
//
//    [in] timeIndex - The time index for the query, in npt format
//
//    [out] chapterSeqNum - The sequence number (0 start) of the chapter
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//


CLIENT_API WVStatus
WV_Info_GetChapterSeqNum(WVSession *session, std::string currentTime,
        unsigned long& chapterSeqNum);


//
// METHOD: WV_Info_Unique_ID
//
// This method retrieves the device's persistent unique identifier.
// For CE devices, this will be from the Widevine Keybox.
// For Desktop platforms, it will be a persistent GUID generated by Widevine.
// For iPhone, it will be the unit's serial number.
//
// Parameters:
//    [in] buffer - empty string buffer to hold the unique ID string.
//                  buffer must be caller-allocated, 64 bytes.
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//


CLIENT_API WVStatus
WV_Info_Unique_ID(char* buffer);

//
// METHOD: WV_SkipChapters
//
// This method causes the stream to skip a number of chapters in any direction
//
// Parameters:
//    [in] session - The session to control
//
//    [in] timeIndex - The current playback time, in npt format
//
//    [in] chapters - The number of chapters to skip.  Negative to skip to previous chapters
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//


CLIENT_API WVStatus
WV_SkipChapters(WVSession *session, std::string currentTime, long chapters);


//
// METHOD: WV_GoToChapter
//
// This method causes the stream to skip to a specified chapter.
//
// Parameters:
//    [in] session - The session to control
//
//    [in] chapterSeqNum - The sequence number (0 based) of the target to skip to
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//


CLIENT_API WVStatus
WV_GoToChapter(WVSession *session, unsigned long chapterSeqNum);


//
// METHOD: WV_Info_GetVideoTracks
//
// This method populates a WVTypedValueArray containing a WVDictionary entry
// for each adaptive video track in the media.  These dicionaries contain all
// the information available for the video, as well as the track ID.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] tracks - WVTypedValueArray which on return will contain an array
//                   of video track info WVDictionary(s).
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetVideoTracks(WVSession *session, WVTypedValueArray& tracks);


//
// METHOD: WV_Info_GetAudioTracks
//
// This method populates a WVTypedValueArray containing a WVDictionary entry
// for each separate audio track in the media.  These dicionaries contain all
// the information available for the audio, as well as the track ID.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] tracks - WVTypedValueArray which on return will contain an array
//                   of audio track info WVDictionary(s).
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetAudioTracks(WVSession *session, WVTypedValueArray& tracks);



//
// METHOD: WV_Info_GetSubtitleTracks
//
// This method populates a WVTypedValueArray containing a WVDictionary entry
// for each separate subtitle track in the media.  These dicionaries contain all
// the information available for subtitle.
//
// NOTE: Subtitle tracks cannot be selected.  They are meant to be rendered by
//       the device media player.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] tracks - WVTypedValueArray which on return will contain an array
//                   of subtile track info WVDictionary(s).
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//


CLIENT_API WVStatus
WV_Info_GetSubtitleTracks(WVSession *session, WVTypedValueArray& tracks);


//
// METHOD: WV_Info_GetCurrentVideoTrack
//
// This method populates a WVDicionary all the information available for the
// current adaptive video track, as well as its track ID.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] track - WVDictionary which on return will contain the information
//                  available for the current adaptive video track
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//

CLIENT_API WVStatus
WV_Info_GetCurrentVideoTrack(WVSession *session, WVDictionary& track);


//
// METHOD: WV_Info_GetCurrentAudioTrack
//
// This method populates a WVDicionary all the information available for the
// current separate audio track, as well as its track ID.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] track - WVDictionary which on return will contain the information
//                  available for the current adaptive video track
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.  This function returns the status
//     WV_Status_Warning_Not_Available if separate audio is not available.
//

CLIENT_API WVStatus
WV_Info_GetCurrentAudioTrack(WVSession *session, WVDictionary& track);


//
// METHOD: WV_SelectTrack
//
// This method is used to explicitly select an adaptive video track or
// separate audio track.
//
// Parameters:
//    [in] session - The session to control
//
//    [out] trackId - The track ID for the track to be selected.
//                    If an adaptive video track ID is specified, then a
//                    video track change will be effected.  Specifying
//                    UNSPECIFIED_TRACK_ID will resume normal adaptive
//                    video behavior.
//                    If a separate audio track ID is specified, then
//                    an audio track change will be effected.
////
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
CLIENT_API WVStatus WV_SelectTrack(WVSession* session, WVUnsignedInt trackId);


#endif // __WV_STREAM_CONTROL_API_H__
