/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_NETWORKSOURCE_H_
#define LIB_MEDIA_NETWORKSOURCE_H_

#include "b_playback_ip_lib.h"
#include "BaseSource.h"

namespace Broadcom
{
namespace Media {

typedef struct tagNetworkSourceContext NetworkSourceContext;
#define MAX_DTCP_HOST_LEN 512
#define MAX_PATH 1024
#define MATCH_PREFIX(a, b) (strncmp((a), (b), strlen(b)) == 0)
#define ORG_FLAG_HTTP_STALLING (1<<21)
#define ORG_FLAG_DLNA_V15 (1<<20)
#define ORG_FLAG_LINK_PROTECTED (1<<16)
#define HTTP_DEFAULT_PORT 80
#define HTTPS_DEFAULT_PORT 443

class BME_SO_EXPORT NetworkSource
    : public BaseSource
{
    public:
        NetworkSource();
        NetworkSource(const NetworkSource& rhs);
        virtual ~NetworkSource();

        /* BaseSource */
        virtual void start();
        virtual void stop(bool holdLastFrame = false);
        virtual void pause();
        virtual IMedia::ErrorType prepare();
        virtual void prepareAsync();
        virtual void setDataSource(MediaStream *mediaStream);
        virtual void reset();
        virtual void release();
        virtual std::string getType();
        virtual bool checkUrlSupport(const std::string& url);
        virtual uint32_t setConnector(SourceConnector* connector);
        virtual IMedia::StreamMetadata getStreamMetadata();
        virtual IMedia::ErrorType seekTo(const uint32_t& milliseconds,
                IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
                IMedia::PlaybackSeekTime  playSeekTime = IMedia::SeekTimeAbsolute);
        virtual void setPlaybackRate(const std::string& rate);
        virtual std::string getAvailablePlaybackRate();
        virtual int getPlaybackRate();
        virtual IMedia::PlaybackOperation getPlaybackOperation();
        virtual uint64_t getDuration();
        virtual uint32_t getCurrentPosition();
        virtual uint64_t getBytesDecoded();
        virtual const IMedia::TimeInfo getTimeInfo();
        virtual void setLooping(bool looping);
        virtual Connector connect(const ConnectSettings& settings);
        virtual void disconnect(const Connector& connector);
        virtual int getScid();

        NetworkSourceContext* getContext() {
            return _context;
        }
        void onPrepared();
        void onError(const IMedia::ErrorType& errorType);
        void onCompletion();
        void onBeginning();
        static void stopInternalThread(void* data);
        static void beginInternalThread(void* data);
        static void errorInternalThread(void* data);
        static void sessionSetupThread(void* data);
        void updateMetadata();
        IMedia::ErrorType probe(bool async);
        void parseResponseHeader();

    private:
        static char _userAgent[];
        static char _additionalHeaders[];
        static const char _rateFlag[];
        static const char _channelsFlag[];
        static const char _certFile[];
        static const uint32_t _networkBufferSize;
        static bool _dtcpIPInitialized;
        static uint16_t _httpNetworkTimeout;
        static uint16_t _rtpNetworkTimeout;
        static void endOfStreamCallback(void* context, int param);
        static void beginningOfStreamCallback(void* context, int param);
        static void errorCallback(void* context, int param);

    private:
        void setOpenSessionParam(B_PlaybackIpSessionOpenSettings* sessionOpenSettings,
                bool async);
        void setSetupSessionParam(B_PlaybackIpSessionSetupSettings *sessionSetupSettings);
        void parseContentFeatures(const char* contentFeature, const char* contentType);
        bool checkServerIsSelf(B_PlaybackIpSessionOpenStatus* sessionOpenStatus);
        void setupDtcpIp(B_PlaybackIpSessionOpenSettings* sessionOpenSettings);
        ReturnCode performDtcpAke(const char* dtcpHost, int dtcpPort);
        char* stristr(const char *str, const char *subStr);
        void onPlaybackIpEventCallback(B_PlaybackIpEventIds eventId);
        static void playbackIpEventCallback(void *context, B_PlaybackIpEventIds param);
        void setDefaultNetworkSettings();
        void getLpcmInfo(const char* mime);
        bool useLiveMode();
        void acquireResources();
        void releaseResources();

        void setupNexusPlaypump();
        void setupNexusPlayback();
        void startIpSession();
        IMedia::ErrorType doPrepare(bool async);
        void init();
        void uninit();
        void updatePsiInfo();
        SourceConnector* getConnector();
        void parseScidFlag(const char* scid);
        void completeSessionSetup();
        void getPsiFromPlaybackIp(const B_PlaybackIpSessionSetupStatus& sessionSetupStatus);

        // add this to satisfy the base class
        /*
           const std::string& getObjectType() const {}
           SerialObject * clone() const {}
           void serialize(Serializer& serializer) const {}

           ITypeManager* getTypeManager() {}
         */

    private:
        NetworkSourceContext* _context;
        std::string _dataSource;
        std::string _url;
        uint64_t _contentLength;
        uint32_t _duration;
        IMedia::EncryptionType _encryptType;
        bool _supportsStalling;
        uint32_t _initialSeekTimeMSec;
        IMedia::StreamMetadata _metadata;
        IMedia::State _state;
        bool _looping;
        bool _resourceAcquired;
        bool _serverTrickMode;
        std::string _playbackRates;
        int _scid;
        uint16_t _maxHeight;
        uint16_t _maxWidth;
};

}  // namespace Media
}  // namespace Broadcom
#endif  // LIB_MEDIA_NETWORKSOURCE_H_
