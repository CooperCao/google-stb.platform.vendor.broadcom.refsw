/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __IMEDIA_H__
#define __IMEDIA_H__

#include <stdint.h>
#include <string>
#include <vector>
#include "Debug.h"

namespace Broadcom {
namespace Media {

typedef uint64_t TIME45k;

class BME_SO_EXPORT IMedia
{
 public:
    static const std::string FILE_URI_PREFIX;
    static const std::string NETWORK_HTTP_URI_PREFIX;
    static const std::string NETWORK_HTTPS_URI_PREFIX;
    static const std::string NETWORK_UDP_URI_PREFIX;
    static const std::string NETWORK_RTP_URI_PREFIX;
    static const std::string NETWORK_RTSP_URI_PREFIX;
    static const std::string TV_URI_PREFIX;
    static const std::string TIMESHIFT_URI_PREFIX;
    static const std::string PUSH_ES_URI_PREFIX;
    static const std::string PUSH_YTMS_URI_PREFIX;
    static const std::string PUSH_TS_URI_PREFIX;
    static const std::string HDMI_URI_PREFIX;

    /**
     * @define PlaybackOperation
     * Enumeration for playback that has pause , seek and trickmode
     */
    enum PlaybackOperation {
        OperationUnknown,
        OperationForward,
        OperationRewind,
        OperationSlowForward,
        OperationSlowRewind,
        OperationEnd
    };

    enum PlaybackSeekTime {
        SeekTimeUnknown,
        SeekTimeAbsolute,
        SeekTimeRelative,
        SeekTimeEnd
    };

    /**
     * @define internal playback state
     * Enumeration for the playback state applied to both source/sink
     */
    enum State {
        IdleState,
        PreparingState,
        StoppedState,
        StartedState,
        PausedState
    };

    /**
     * @define EncryptionType
     * Enumeration for the encryption algorithm applied to the
     *        media being feed to the MediaPlayer/
     *        Note: All encryption types may not be supported by a given implementation
     */
    enum EncryptionType {
        UnknownEncryptionType,
        CssEncryptionType,
        CppmEncryptionType,
        TdesEncryptionType,
        DesEncryptionType,
        AesEcbEncryptionType,
        AacsEncryptionType,
        CprmEncryptionType,
        WmdrmEncryptionType,
        DivxEncryptionType,
        DtcpIpEncryptionType,
        YouTubeWidevineEncryptionType,
        YouTubeClearKeyEncryptionType,
        SslEncryptionType,
        PlayReady25EncryptionType,
        PlayReady33EncryptionType,
        YouTubePlayReadyEncryptionType = PlayReady25EncryptionType,
        NoEncryptionType
    };


    enum ErrorType {
        MEDIA_SUCCESS = 0,
        /* Unspecified media player error. */
        MEDIA_ERROR_UNKNOWN = 1,
        MEDIA_INVALD_PARAM_ERROR = 2,
        MEDIA_ERROR_INVALID_STATE,
        MEDIA_ERROR_ARG_OUT_OF_RANGE,
        MEDIA_ERROR_NOT_SUPPORTED,
        /* Media server died. In this case, the application must release
           the MediaPlayer object and instantiate a new one. */
        MEDIA_ERROR_SERVER_DIED = 100,
        /* The video is streamed and its container is not valid
           for progressive playback i.e the video's index
           (e.g moov atom) is not at the start of the file. */
        MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200,
        MEDIA_ERROR_IO = 300,
        MEDIA_ERROR_UNSUPPORTED_STREAM,
        MEDIA_ERROR_UNSUPPORTED_CODEC,
        MEDIA_ERROR_UNSUPPORTED_RATE,
        /* Tuner */
        MEDIA_ERROR_TUNER_NOT_LOCKED = 400,
        MEDIA_ERROR_TUNER_NO_SIGNAL,
        MEDIA_ERROR_TUNER_INVALID_PARAM,
        MEDIA_ERROR_TIMEOUT,
        MEDIA_ERROR_SOURCE_INVALID_PARAM,
        MEDIA_ERROR_SOURCE_INVALID_HANDLE,
        /* Recorder */
        MEDIA_ERROR_RECORDER_INVALID_PARAM = 500,
        MEDIA_ERROR_RECORDER_INVALID_HANDLE,
        MEDIA_ERROR_RECORDER_OVERFLOW,     // buffer overflow detected
        MEDIA_ERROR_RECORDER_DISK_FULL,     // no more space left on disk
        MEDIA_ERROR_RECORDER_MAX_FILE_SIZE, // file has reach maximum size allowed by file system
        MEDIA_ERROR_RECORDER_NO_DATA_TIMEOUT,
        /* Storage */
        MEDIA_ERROR_FROMAT_FAIL = 600,
        MEDIA_ERROR_FROMAT_INVALID_INDEX,
        MEDIA_ERROR_FROMAT_NOT_REGISTERED,
        MEDIA_ERROR_FROMAT_VOLUME_ALREADY_MODUTED,
        MEDIA_ERROR_FROMAT_SYSTEM_ERR,
        MEDIA_ERROR_MOUNT_FAIL,
        MEDIA_ERROR_MOUNT_INVALID_INDEX,
        MEDIA_ERROR_MOUNT_NOT_REGISTERED,
        MEDIA_ERROR_MOUNT_VOLUME_ALREADY_MOUNTED,
        MEDIA_ERROR_MOUNT_NOT_FORMATTED,
        MEDIA_ERROR_MOUNT_SYSTEM_ERR,
        MEDIA_ERROR_MOUNT_WRONG_LABEL,
        /* Streamer error */
        MEDIA_ERROR_OUT_OF_STREAMING_RESOURCES = 700,
        MEDIA_ERROR_STREAMER_SETTINGS,
        MEDIA_ERROR_STREAMER_INVALID_STATE,
        /* File */
        MEDIA_ERROR_FILE_NOT_FOUND = 800,
        /* DVR Playback */
        MEDIA_ERROR_PLAYBACK_ABORT = 900,
        MEDIA_ERROR_PLAYBACK_INVALID_HANDLE,
        /* Security */
        MEDIA_ERROR_SECURITY_LOAD_KEY_FAILED = 1000,
        MEDIA_ERROR_SECURITY_INVALID_HANDLE,
    };

    /**
     * @define PlaybackEvent
     * Enumeration of the events supported by the IMediaPlayer
     */
    enum InfoType {
        MEDIA_INFO_PTS_UPDATE = 902, /* A new set of PTS is available. */
        MEDIA_INFO_LASTPUMP_UPDATE = 903,  /* For one play pump*/
        MEDIA_INFO_LASTVPUMP_UPDATE = 904, /* Last Video play pump level. */
        MEDIA_INFO_LASTAPUMP_UPDATE = 905, /* Last Audio play pump level. */
        MEDIA_INFO_LASTV3DPUMP_UPDATE = 906, /* Last Video 3D play pump level. */
        MEDIA_INFO_FIRST_PTS = 907, /*First PTS*/
        MEDIA_INFO_DATA_EXHAUSTED = 908, /*Data exhausted*/
        MEDIA_INFO_SOURCE_ENDED = 909,     /*Media Source end*/
        MEDIA_INFO_SOURCE_OPENED = 910,   /*Media Source opened*/
        MEDIA_INFO_METADATA_UPDATE = 911, /*Media metadata updated*/
        MEDIA_INFO_SOURCE_PENDING = 912,     /*Media Source pending*/
        MEDIA_INFO_SOURCE_DURATION_CHANGED = 913, /*Media Source duration changed*/
        MEDIA_INFO_VIDEO3D_PTS_CALLBACK = 914, /*3D video PTS sync*/
        MEDIA_INFO_VPUMPFULL_UPDATE = 915,  /* video playpump full status*/
        MEDIA_INFO_APUMPFULL_UPDATE = 916,  /* audio playpump full status*/
        MEDIA_INFO_V3DPUMPFULL_UPDATE = 917,  /* video3D playpump full status*/
        /* Tuner */
        MEDIA_INFO_TUNER_NOT_LOCKED = 400,
        MEDIA_INFO_TUNER_NO_SIGNAL = 401,
        MEDIA_INFO_TUNER_INVALID_PARAM = 402,
        MEDIA_INFO_TUNER_TIMEOUT = 403,
        /* DVR Playback*/
        MEDIA_INFO_DVR_START_POINT = 500, /* When DVR playback start or reached to start point while rewind*/
        MEDIA_INFO_DVR_TSB_CONVERSION_COMPLETED = 501, /*When TSB conversion from TSB beuufer to permanent was done*/
        MEDIA_INFO_DVR_TSB_PLAYBACK_START = 502, /*When TSB playback start*/
        MEDIA_INFO_DVR_TSB_PLAYBACK_STOP = 503 /*When TSB playback stop*/
    };

    enum DrmKeyErrorType {
        DrmKeyUnknownError = 1  // unknown error occurr during processing of encryption keys
    };

    enum VideoCodecProfile {
        UnknownProfile,
        SimpleProfile,
        MainProfile,
        HighProfile,
        AdvancedProfile,
        JizhunProfile,
        SnrScalableProfile,
        SpatiallyScalableProfile,
        AdvancedSimpleProfile,
        BaselineProfile
    };

    enum VideoCodecLevel {
       eUnknown,
       e00,
       e10,
       e1B,
       e11,
       e12,
       e13,
       e20,
       e21,
       e22,
       e30,
       e31,
       e32,
       e40,
       e41,
       e42,
       e50,
       e51,
       e60,
       e62,
       eLow,
       eMain,
       eHigh,
       eHigh1440,
       eMax
    };

    /**
     * @define AudioCodec
     * Enumeration of the various audio codecs
     *        Note: All codecs may not be supported by a given implementation
     */
    enum AudioCodec {
        UnknownAudioCodec,     /* unknown/not supported audio format */
        MpegAudioCodec,        /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
        Mp3AudioCodec,         /* MPEG1/2, layer 3. */
        AacAdtsAudioCodec,     /* Advanced audio coding. Part of MPEG-4, in Audio Data Transport (ADTS) Format */
        AacLoasAudioCodec,     /* Advanced audio coding. Part of MPEG-4, in in Low Overhead Audio Stream (LOAS) Format */
        AacPlusLoasAudioCodec, /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), in Low Overhead Audio Stream (LOAS) Format */
        AacPlusAdtsAudioCodec, /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), in Audio Data Transport (ADTS) Format */
        Ac3AudioCodec,         /* Dolby Digital AC3 audio */
        Ac3PlusAudioCodec,     /* Dolby Digital Plus (AC3+ or DDP) audio */
        DtsAudioCodec,         /* Digital Digital Surround sound, uses non-legacy frame-sync */
        LpcmDvdAudioCodec,     /* LPCM, DVD mode */
        LpcmHdDvdAudioCodec,   /* LPCM, HD-DVD mode */
        LpcmBluRayAudioCodec,  /* LPCM, Blu-Ray mode */
        /* Digital Digital Surround sound, HD, uses non-legacy frame-sync, decodes only DTS part of DTS-HD stream */
        DtsHdAudioCodec,
        WmaStdAudioCodec,      /* WMA Standard */
        WmaStdTsAudioCodec,    /* WMA Standard with a 24-byte extended header */
        WmaProAudioCodec,      /* WMA Professional */
        AvsAudioCodec,         /* AVS */
        PcmAudioCodec,         /* PCM audio - headerless PCM, characteristics must be specified through AudioPlaybackParameters */
        PcmWavAudioCodec,      /* PCM audio with Wave header - Used with streams containing PCM audio */
        AmrAudioCodec,         /* Adaptive Multi-Rate compression (typically used w/3GPP) */
        DraAudioCodec,         /* Dynamic Resolution Adaptation.  Used in Blu-Ray and China Broadcasts. */
        CookAudioCodec,        /* Real Audio 8 LBR */
        AdpcmAudioCodec,       /* MS ADPCM audio format */
        DviAdpcmAudioCodec,    /* DVI Adpcm */
        SbcAudioCodec,         /* Sub Band Codec used in Bluetooth A2DP audio */
        DtsLegacyAudioCodec,   /* Digital Digital Surround sound, legacy mode (14 bit), uses legacy frame-sync */
        DtsExpressAudioCodec,
        VorbisAudioCodec,
        Lpcm1394AudioCodec,
        G711AudioCodec,
        G723_1AudioCodec,
        G726AudioCodec,
        G729AudioCodec,
        FlacAudioCodec,
        MlpAudioCodec,
        ApeAudioCodec,
        IlbcAudioCodec,
        IsacAudioCodec,
        OpusAudioCodec,
        AlsAudioCodec,
        AlsLoasAudioCodec,
        Ac4AudioCodec
    };

    /**
     * @define VideoCodec
     * Enumeration for the various video codecs
     *        Note: All codecs may not be supported by a given implementation
     */
    enum VideoCodec {
        UnknownVideoCodec = 0, /* unknown/not supported video codec */
        Mpeg1VideoCodec,       /* MPEG-1 Video (ISO/IEC 11172-2) */
        Mpeg2VideoCodec,       /* MPEG-2 Video (ISO/IEC 13818-2) */
        Mpeg4Part2VideoCodec,  /* MPEG-4 Part 2 Video */
        H263VideoCodec,        /* H.263 Video. The value of the enum is not based on PSI standards. */
        H264VideoCodec,        /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC */
        H264SvcVideoCodec,     /* Scalable Video Codec extension of H.264 */
        H264MvcVideoCodec,     /* Multi View Coding extension of H.264 */
        Vc1VideoCodec,         /* VC-1 Advanced Profile */
        Vc1SimpleMainVideoCodec, /* VC-1 Simple & Main Profile */
        Divx311VideoCodec,     /* DivX 3.11 coded video */
        AvsVideoCodec,         /* AVS coded video */
        Rv40VideoCodec,        /* RV 4.0 coded video */
        Vp6VideoCodec,         /* VP6 coded video */
        Vp7VideoCodec,         /* VP7 coded video */
        Vp8VideoCodec,         /* VP8 coded video */
        Vp9VideoCodec,         /* VP9 coded video */
        SparkVideoCodec,       /*Spark video codec*/
        MotionJpeg,
        H265VideoCodec
    };

    /**
     * @define TrackType
     * Enumeration of the various track formats
     *        Note: All codecs may not be supported by a given implementation
     */
    enum TrackType {
        VideoTrackType,  /* video track */
        AudioTrackType, /* audio track */
        PcrTrackType,     /* track with PCR information */
        OtherTrackType /* track type other than listed above, it could be video or audio track with unknown codec type */
    };

    /**
     * @define StreamType
     * Enumeration of the various stream formats
     *        Note: All codecs may not be supported by a given implementation
     */
    enum StreamType {
        UnknownStreamType,     /* Unknown stream format */
        EsStreamType,          /* Elementary stream. No container or muxing. */
        TsStreamType,          /* MPEG2 transport stream */
        Mpeg2PesStreamType,    /* MPEG2 packetized elementary stream, this includes MPEG2 Program Stream  streams */
        /* DVD VOB, this is subset of MPEG2 Program Stream, special processing is applied for VOB streams */
        VobStreamType,
        Mpeg1PsStreamType,     /* MPEG1 program stream */
        DssEsStreamType,       /* DSS with ES payload (used for SD) */
        DssPesStreamType,      /* DSS with PES payload (used for HD) */
        AsfStreamType,         /* Advanced Systems Format */
        AviStreamType,         /* Audio Video Interleave */
        Mp4StreamType,         /* MP4 (MPEG-4 Part12) container */
        FlvStreamType,         /* Flash video container */
        MkvStreamType,         /* Matroska container */
        WavStreamType,         /* WAVE audio container */
        Mp4FragmentStreamType, /* separate 'moof' boxes from the MP4 (MPEG-4 Part12) container */
        RmffStreamType,        /* RMFF container */
        OggStreamType,         /* OGG container */
        Mp3StreamType,         /* MP3 container */
        AutoStreamType         /* Auto-detection */
    };



    /**
    * @define VideoFrameRate
    * Enumeration representing rate measured in frames per second.
    */
    enum VideoFrameRate {
        VideoFrameRateUnknown,
        VideoFrameRate_23_976,
        VideoFrameRate_24,
        VideoFrameRate_25,
        VideoFrameRate_29_97,
        VideoFrameRate_30,
        VideoFrameRate_50,
        VideoFrameRate_59_94,
        VideoFrameRate_60,
        VideoFrameRate_14_985,
        VideoFrameRate_7_493,
        VideoFrameRate_Max
    };

    /**
     * @define VideoAspectRatio
     * Enumeration representing video aspect ratio
     */
    enum VideoAspectRatio {
        VideoAspectRatioUnknown,
        VideoAspectRatio_SquarePixel,
        VideoAspectRatio_4x3,    /* 4:3 */
        VideoAspectRatio_16x9,   /* 16:9 */
        VideoAspectRatio_221x1,  /* 2.21:1 */
        VideoAspectRatio_15x9,   /* 15:9 */
        /* Sample aspect ratio - aspect ratio of the source calculated
           as the ratio of two numbers reported by the decoder. */
        VideoAspectRatio_Sar,
        VideoAspectRatio_Max
    };

    enum class DecoderType {
        Main,
        Pip,
        Mosaic,
        Mosaic_HD
    };

    enum class TransportSegmentFormat {
        Psi, /* Filter for Program Specific Information (PSI) from an MPEG2 Transport stream */
        Pes, /* PES data capture. Captures PES packets with respect to PES packet length. */
        PesSaveAll, /* PES data capture. Captures entire payload without respect to PES packet length. */
        Ts,  /* MPEG2 Transport data capture */
        Max
    };

    class DataBuffer
    {
    public:
        DataBuffer() {}
        virtual ~DataBuffer() {}

    private:
        intptr_t _data;
        uint32_t _size;

    public:
        intptr_t getData() const {
            return _data;
        }
        void setData(const intptr_t data) {
            _data = data;
        }
        uint32_t getSize() const {
            return _size;
        }
        void setSize(const uint32_t size) {
            _size = size;
        }
    };

    struct Rect {
        int16_t x;
        int16_t y;
        uint16_t width;
        uint16_t height;
    };

    struct VideoParameters {
        uint16_t streamId;      // videoPid
        uint16_t substreamId;   // pcrPid
        IMedia::VideoCodec videoCodec;
        IMedia::VideoCodec extraVideoCodec;
        uint16_t maxWidth;
        uint16_t maxHeight;
        uint16_t colorDepth;
        IMedia::VideoAspectRatio videoAspectRatio;
    };

    struct AudioParameters {
        uint16_t streamId;      // audioPid
        uint16_t substreamId;
        IMedia::AudioCodec audioCodec;
        uint32_t samplesPerSecond;
        uint32_t bitsPerSample;
        uint8_t numChannels;
    };

    struct PmtParameters {
        uint16_t streamId;
    };

    typedef std::vector<AudioParameters> AudioParametersList;
    typedef std::vector<PmtParameters> PmtParametersList;

    class StreamMetadata {
    public:
        StreamMetadata();
        ~StreamMetadata();
        void operator=(const StreamMetadata& metadata);
        void reset();
        IMedia::StreamType streamType;
        IMedia::EncryptionType encryptionType;
        IMedia::VideoParameters videoParam;
        IMedia::AudioParametersList audioParamList;
        IMedia::PmtParametersList pmtParamList;
        uint32_t parserBand;
        bool timeStampEnabled;
        uint32_t duration;
        int getAudioParamIndex(uint16_t pid) const;
    };

    class MediaResource
    {
    public:
        MediaResource() {}

    private:
        std::string _uri;
        uint32_t _duration;
        uint64_t _size;
        std::string _protocolInfo;
        std::string _resolution;
        std::string _metaData;
        std::string _channelList;

        /* Mp3 meta data */
        std::string _id;
        std::string _title;
        std::string _artist;
        std::string _album;
        std::string _year;
        std::string _genre;
        std::string _startTime;
        std::string _endTime;
        std::string _comment;

    public:
        std::string getUri() const {
            return _uri;
        }
        void setUri(const std::string& uri) {
            _uri = uri;
        }
        uint32_t getDuration() const {
            return _duration;
        }
        void setDuration(const uint32_t duration) {
            _duration = duration;
        }
        uint64_t getSize() const {
            return _size;
        }
        void setSize(const uint64_t size) {
            _size = size;
        }
        std::string getProtocolInfo() {
            return _protocolInfo;
        }
        void setProtocolInfo(std::string protocolInfo) {
            _protocolInfo = protocolInfo;
        }
        std::string getresolution()  {
            return _resolution;
        }
        void  setResolution(std::string resolution)  {
            _resolution = resolution;
        }
        std::string getMetaData()    {
            return _metaData;
        }
        void setMetaData(std::string metaData)    {
            _metaData = metaData;
        }
        std::string getChannelList()  {
            return _channelList;
        }
        void setChannelList(std::string channelList)  {
            _channelList = channelList;
        }
        void setId(std::string id) {
            _id = id;
        }

        /* Mp3 meta data */
        void  setTitle(std::string title) {
            _title = title;
        }
        std::string getTitle() {
            return _title;
        }
        void  setArtist(std::string artist) {
            _artist = artist;
        }
        std::string getArtist() {
            return _artist;
        }
        void  setAlbum(std::string album) {
            _album= album;
        }
        std::string getAlbum() {
            return _album;
        }
        void  setYear(std::string year) {
            _year = year;
        }
        std::string getYear() {
            return _year;
        }
        void  setGenre(std::string genre) {
            _genre = genre;
        }
        std::string getGenre() {
            return _genre;
        }
        void  setStartTime(std::string startTime) {
            _startTime = startTime;
        }
        std::string getStartTime() {
            return _startTime;
        }
        void  setEndTime(std::string endTime) {
            _endTime = endTime;
        }
        std::string getEndTime() {
            return _endTime;
        }
        void  setComment(std::string comment)  {
            _comment = comment;
        }
        std::string getComment() {
            return _comment;
        }
    };

    class MediaObject
    {
    public:
        MediaObject() {}

    private:
        uint32_t _type;
        uint32_t _totalCount;
        std::string _id;
        std::string _title;
        std::string _uPnpClass;
        std::string _poster;

    public:
        uint32_t getType() const {
            return _type;
        }
        void  setType(uint32_t type) {
            _type = type;
        }
        uint32_t getTotalCount() const {
            return _totalCount;
        }
        void setTotalCount(uint32_t totalCount) {
            _totalCount = totalCount;
        }
        std::string getId() const {
            return _id;
        }
        void setId(std::string id) {
            _id = id;
        }
        std::string getTitle() const {
            return _title;
        }
        void setTitle(std::string title) {
            _title = title;
        }
        std::string getUPnPClass() const {
            return _uPnpClass;
        }
        void setUPnPClass(std::string uPnpClass) {
            _uPnpClass = uPnpClass;
        }
        std::string getPoster() const {
            return _poster;
        }
        void setPoster(std::string poster) {
            _poster = poster;
        }
    };

    struct MediaPlayerOptions {
        DecoderType decoderType;
        bool primingMode;
    };

    typedef std::vector<int16_t> AvailablePlaybackRateList;
    typedef std::vector<MediaResource> MediaResourceList;
    typedef std::vector<MediaObject> MediaObjectList;

    struct TimeInfo {
        uint64_t currentPosition;
        uint64_t startTime;
        uint64_t endTime;
    };
};

}  // namespace Media
}  // namespace Broadcom

#endif  //  __IMEDIA_H__
