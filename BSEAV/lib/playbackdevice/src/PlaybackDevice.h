/***************************************************************************
*    (c)2005-2011 Broadcom Corporation * 
* This program is the proprietary software of Broadcom Corporation and/or its licensors, 
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 ***************************************************************************/
/**
 * @file CARD_PlaybackDevice.h
 *
 */

#ifndef CARD_PLAYBACKDEVICE_H
#define CARD_PLAYBACKDEVICE_H

#include <signal.h>
#include <memory>
#include <vector>
#include <b_os_lib.h>
#include "NexusIncludes.h"

//enable counting of memory mallocs and frees
//#define CARD_PLAYBACK_MEMCHECK

#define VIDEO_PTS_DELTA_MIN 500 // min number of milliseconds which must pass 
                                // before a time event is inserted into the event queue
                                
#define CDB_SETTLE_TICKS 60*10 // Do not check for underflow during the first 
                               // CDB_SETTLE_TICKS of playback 

#define CARD_PLAYRATE_PAUSE  0
#define CARD_PLAYRATE_NORMAL 1000

#define MAX_INDEX 256

#define TIME45k uint32_t

/**
 * @define CARD_PlaybackStatus
 * @brief Enumeration representing the return status of various PlaybackDevice
 *        functions
 */
enum CARD_PlaybackStatus 
{
    CARD_PlaybackSuccess = 0, 
    CARD_PlaybackFailure, 
    CARD_PlaybackUninitialized, 
    CARD_PlaybackInvalidArg, 
    CARD_PlaybackInvalidState, 
    CARD_PlaybackFull, 
    CARD_PlaybackEmpty, 
    CARD_PlaybackUnsupportedStreamType, 
    CARD_PlaybackNotSupported, 

    CARD_PlaybackStatusLast /* always the last entry */
};

/**
 * @define CARD_PlaybackEventID
 * @brief Enumeration of the events supported by the PlaybackDevice
 */
enum CARD_PlaybackEventID
{
    CARD_PlaybackEvent_PTS_UPDATE,      /* a PTS event with associated value */
    CARD_PlaybackEvent_DATA_EXHAUSTED,  /* no data is available; decoders will stall (no value) */
    CARD_PlaybackEvent_END_OF_STREAM,   /* end of stream has been detected (no value) */
    CARD_PlaybackEvent_FAILURE,         /* a error condition with associated value*/
    CARD_PlaybackEvent_LAST             /* always the last entry */
};

/**
 * @define CARD_OutputSecurity
 * @brief Enumeration for Output Security to be applied for this stream
 */
enum CARD_OutputSecurity
{
    CARD_OutputSecurity_NONE,
    CARD_OutputSecurity_CSS,
    CARD_OutputSecurity_CPPM,
    CARD_OutputSecurity_TDES,
    CARD_OutputSecurity_DES,
    CARD_OutputSecurity_AES_ECB,
    CARD_OutputSecurity_AACS,
    CARD_OutputSecurity_CPRM,
    CARD_OutputSecurity_NETFLIX,
    CARD_OutputSecurity_HDCP,
    CARD_OutputSecurity_LAST         /* always the last entry */
};




/**
 * @define CARD_EncryptionType
 * @brief Enumeration for the encryption algorithm applied to the 
 *        media being feed to the playback device
 *        Note: All encryption types may not be supported by a given implementation
 */
enum CARD_EncryptionType
{
    CARD_EncryptType_NONE,
    CARD_EncryptType_CSS,
    CARD_EncryptType_CPPM,
    CARD_EncryptType_TDES,
    CARD_EncryptType_DES,
    CARD_EncryptType_AES_ECB,
    CARD_EncryptType_AACS,
    CARD_EncryptType_CPRM,
    CARD_EncryptType_WMDRM,
    CARD_EncryptType_DIVX,
	CARD_EncryptType_WV_VOD,
    CARD_EncryptType_LAST        /* always the last entry */
};

enum CARD_EncryptKeyType
{
CARD_EncryptKeyType_NONE,
CARD_EncryptKeyType_EVEN,
CARD_EncryptKeyType_ODD,
CARD_EncryptKeyType_LAST
};



/**
 * @define CARD_VideoCodec
 * @brief Enumeration for the various video codes
 *        Note: All codecs may not be supported by a given implementation
 */
enum CARD_VideoCodec
{
    CARD_VideoCodec_UNKNOWN,
    CARD_VideoCodec_MPEG1  , /* MPEG-1 Video (ISO/IEC 11172-2)           */
    CARD_VideoCodec_MPEG2  , /* MPEG-2 Video (ISO/IEC 13818-2)           */
    CARD_VideoCodec_MPEG4P2, /* MPEG-4 Part2                             */
    CARD_VideoCodec_DIVX311, /* Divx 311 codec.                          */
    CARD_VideoCodec_H264   , /* H.264(ITU-T) or
                              * ISO/IEC14496-10/MPEG-4 AVC               */
    CARD_VideoCodec_VC1    , /* SMPTE VC-1 Video                         */
    CARD_VideoCodec_VC1_SM , /* VC-1 Video Simple&Main Profile           */
    CARD_VideoCodec_LAST     /* always the last entry                    */
};

/**
 * @define CARD_AudioCodec
 * @brief Enumeration for the various Audio codes
 *        Note: All codecs may not be supported by a given implementation
 */
enum CARD_AudioCodec
{
   CARD_AudioCodec_UNKNOWN,      /* Audio Format is unknown */
   CARD_AudioCodec_MPEG,         /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
   CARD_AudioCodec_MP3,          /* MPEG1/2, layer 3. */
   CARD_AudioCodec_WMA,          /* WMA Standard */
   CARD_AudioCodec_AAC,          /* Advanced audio coding. Part of MPEG-4 */
   CARD_AudioCodec_AAC_LOAS,        /* AAC with LOAS (Low Overhead Audio Stream) */
   CARD_AudioCodec_AAC_ADTS,        /* AAC with ADTS (Audio Data Transport Format) */
   CARD_AudioCodec_AAC_HE_LOAS,     /* AAC PLUS with LOAS (Low Overhead Audio Stream) */
   CARD_AudioCodec_AAC_HE_ADTS,     /* AAC PLUS with ADTS (Audio Data Transport Format) */
   CARD_AudioCodec_AC3,          /* Dolby Digital AC3 audio */
   CARD_AudioCodec_AC3_PLUS,     /* Dolby Digital Plus (AC3+ or DDP) audio */
   CARD_AudioCodec_AC3_LOSSLESS, /* Dolby Digital Plus Lossless audio */
   CARD_AudioCodec_DTS,          /* Digital Digital Surround sound. */
   CARD_AudioCodec_LPCM_HDDVD,   /* LPCM, HD-DVD mode */
   CARD_AudioCodec_LPCM_BLURAY,  /* LPCM, Blu-Ray mode */
   CARD_AudioCodec_DTS_HD,       /* DTS-HD */
   CARD_AudioCodec_LPCM_DVD,     /* LPCM, DVD mode */
   CARD_AudioCodec_MLP,          /* MLP */
   CARD_AudioCodec_DTS_HD_XLL,   /* DTS-HD XLL */
   CARD_AudioCodec_DTS_LBR,      /* DTS-HD LBR */
   CARD_AudioCodec_AC3_PLUS_7_1, /* Dolby Digital Plus 7.1 */
   CARD_AudioCodec_DTS_CD,       /* DTS-CD */
   CARD_AudioCodec_DTS_CA,       /* DTS CA */
   CARD_AudioCodec_MPEG2_MC,     /* MPEG2 Multichannel */
   CARD_AudioCodec_PCM,          /* PCM with a wav header. Currently only little endian is supported */
   CARD_AudioCodec_AC3_PLUS_ONLINE, /* Dolby Digital Plus Online streaming audio */
   CARD_AudioCodec_LAST          /* always the last entry */
};

/**
 * @define CARD_StreamType
 * @brief Enumeration for media stream types supported without repackaging
 *        
 */
enum CARD_StreamType
{
    CARD_StreamType_UNKNOWN = 0, /* unknown format */
    CARD_StreamType_MPEG2_TS,    /* MPEG2 Transport Stream */
    CARD_StreamType_MPEG2_PS,    /* MPEG2 Program Stream */
    CARD_StreamType_LAST         /* always the last entry */
};

/**
 * @define CARD_PlaybackVideoParams 
 * @brief  structure defining video stream parameters
 */
struct CARD_PlaybackVideoParams 
{
    uint16_t        pid;          /* Video PID, if Transport Stream */
    uint8_t         pes_id;       /* Video Stream ID, if Program Stream */
    uint8_t         substream_id; /* Video Sub stream ID; optional for PS */
    CARD_VideoCodec codec;        /* MPEG-2, AVC, VC-1, etc. */
};

/**
 * @define CARD_PlaybackAudioParams 
 * @brief  structure defining Audio stream parameters
 */
struct CARD_PlaybackAudioParams 
{
    uint16_t        pid;          /* Audio PID, if Transport Stream */
    uint8_t         pes_id;       /* Audio Stream ID, if Program Stream */
    uint8_t         substream_id; /* Audio Sub stream ID; optional for PS */
    uint32_t        sample_rate;  /* Audio Sample rate */
    uint8_t         num_channels; /* Number of Audio Channels */
    CARD_AudioCodec codec;        /* Dolby AC-3, L-PCM, etc. */
};

/**
 * @define CARD_PlaybackSecurity
 * @brief  structure defining medias output security requirements
 */
struct CARD_PlaybackSecurity
{
    uint8_t             macrovision;       /* analog protection system */
                                           /* 0 - no security */
                                           /* 1 - Macrovision type 1: AGC only */
                                           /* 3 - Macrovision type 3: AGC and 4line colorstripe */
    uint8_t             copyManagement;    /* copy management */
                                           /* 0 - no security */
                                           /* 3 - Never Copy  */
    bool                dot;               /* AACS digital only token */
    bool                ict;               /* AACS image constraint token */
    bool                preventUpscaling;  /* upscaling prohibited on analog outputs by CSS */
    CARD_OutputSecurity securityMode;      /* output security mode of required by current media stream*/
};

typedef struct tag_CARD_DecodeSettings 
{
   NEXUS_ParserBand		parserBand;
   NEXUS_TransportType	transportType;
   NEXUS_PlaypumpHandle	videoPlaypump; /* if set, open playpump pid channels */
   NEXUS_PlaypumpHandle	audioPlaypump; /* if set, open playpump pid channels */    
   uint16_t				videoPid;
   NEXUS_VideoCodec		videoCodec;
   uint16_t				audioPid;
   NEXUS_AudioCodec		audioCodec;
   uint16_t				pcrPid;
} CARD_DecodeSettings;

typedef struct tag_CARD_PidChannelStatus
{
	NEXUS_PidChannelStatus pidStatus;
	
}CARD_PidChannelStatus;

/**
 * @brief Interface for encapsulation of device playback functionalities.
 */
class CARD_PlaybackDevice
{
public:

    class EventQueue;

    // thread priority
    static int VIDEO_DSP_PRIO;

     // event queue size
    static uint32_t EVENT_QUEUE_SIZE;

    // the speed value for normal speed
    static const uint32_t NORMAL_SPEED     = (1000);

    // the speed value for paused speed
    static const uint32_t PAUSED_SPEED     = (0);

    // maximum video streams
    static const int MAX_VID_STREAMS = 16;

    // invalid time stamp
    static const uint64_t INVALID_TIMESTAMP = (-1);

    // the target begin value of audio/video PTS in a stream.
    static const int64_t  PTS_START_VALUE   = (100000000);

    struct VideoAttributes
    {
       uint32_t streamNumber;
	// The FourCC code
       uint32_t compressionID;
       uint32_t imageWidth;
       uint32_t imageHeight;
       uint32_t bitsPerPixel;
       uint32_t pixelAspectRatioX;
       uint32_t pixelAspectRatioY;
       std::vector<unsigned char> codecSpecificData;
    };

    struct AudioAttributes
    {
       uint32_t streamNumber;
	// wFormatTag of WAVEFORMATEX
       uint32_t codecID;
       uint32_t numberofChannels;
       uint32_t samplesPerSecond;
       uint32_t bytesPerSecond;
       uint32_t blockAlignment;
       uint32_t bitsPerSample;
       std::vector<unsigned char> codecSpecificData;
    };

    struct StreamAttributes
    {
        int64_t          ptsOffset;
        AudioAttributes* audioAttributes;
        VideoAttributes* videoAttributes;
    };

    /**
     * the following set of functions define the playback devices public
     */

    /* this constructor can be used in when no implementation
     * specific object is required
     */
    CARD_PlaybackDevice();

    /* this constructor can be used if an implementation specific object
     * needs to be passed to the playback device
     */
    CARD_PlaybackDevice(const void *pObject);

    /* Destructor
     * closes the device if required 
     */
    virtual ~CARD_PlaybackDevice();

    /* this flavor of the open command is used when the details of the media
     * stream are known... for most streaming clients, this is likely to be
     * true
     *
     * If the function call is successful, it will return CARD_PlaybackSuccess
     * other wise CARD_PlaybackFailure will be returned
     *
     * @param[in] streamType     currently only Mpeg2 Transport or Program Streams 
     * @param[in] encryptType    encryption algorithm applied to the media being 
     *                           feed to the playback device
     * @param[in] videoParams    list of video stream parameters.  If there is no
     *                           video stream, this list will be empty.  Currently
     *                           only the first video stream will be played.
     * @param[in] audioParams    list of audio stream parameters.  If there is no
     *                           audio stream, this list will be empty.  Currently
     *                           only the first audio stream will be played.
     * @param[in] bufferDepth    amount of data buffered by the playback device
     * @param[in] security       details of output security protection required 
     * @param[in] bufferCallback pointer function called when a provided databuffer
     *                           is released
     */
    virtual CARD_PlaybackStatus open(CARD_StreamType                        streamType,
                                     CARD_EncryptionType                    encryptType,
                                     std::vector<CARD_PlaybackVideoParams> &videoParams,
                                     std::vector<CARD_PlaybackAudioParams> &audioParams,
                                     uint32_t                               bufferDepth,
                                     CARD_PlaybackSecurity                 &security,
                                     void                                  (*bufferCallback)(void*));

    /* this flavor of the open command can be used if we want to the media
     * parser to identify the type of stream
     *
     * If the function call is successful, it will return CARD_PlaybackSuccess
     * other wise CARD_PlaybackFailure will be returned
     *
     * @param[in] encryptType    encryption algorithm applied to the media being 
     *                           feed to the playback device
     * @param[in] bufferDepth    amount of data buffered by the playback device
     * @param[in] security       details of output security protection required 
     * @param[in] bufferCallback pointer function called when a provided databuffer
     *                           is released
     */
    virtual CARD_PlaybackStatus open(CARD_EncryptionType          encryptType,
                                     uint32_t                     bufferDepth,
                                     CARD_PlaybackSecurity       &security,
                                     void                        (*bufferCallback)(void*));

    /* the close function resets the playback device to its initial state
     * used when the playback device is reused for a new stream, or to start
     * playback at a different location in a clip
     *
     * @param[in] hold_picture   true if the caller wants the last picture displayed
     *                           to held;  false if the caller wants the displayed blank
     */
    virtual void close(bool hold_picture);

    /**
     * Old version maintained for compatibility
     */
    virtual void close(void);

    /* Delivers audio/video data to the playback device. 
     *
     * if the bufferSize is 0 or dataBuffer is NULL,
     * CARD_PlaybackInvalidArg will be returned.
     *
     * if feedData is called before successfully calling the open()
     * functions or after endOfStream() has been called 
     * CARD_PlaybackInvalidState will be returned.
     *
     * If the playback device's internal pipe-line does not have enough 
     * space to accept the data, the CARD_PlaybackFull status will be 
     * returned and the caller should * wait and re-attempt delivery later. 
     *
     * If feedData is successful, CARD_PlaybackSuccess will be returned
     *
     * @param[in] dataBuffer the buffer which contains the data to be
     *            delivered.
     * @param[in] bufferSize number of bytes in dataBuffer
     */
    virtual CARD_PlaybackStatus feedData(void *dataBuffer, uint32_t  bufferSize);
	
	/*same as above but feeds encrypted data*/

	virtual CARD_PlaybackStatus feedData(void *dataBuffer, uint32_t  bufferSize, CARD_EncryptKeyType keyslot_type );

    /**
     * Set the playback speed of device.
     *
     * For normal speed playback speed should equal CARD_PLAYRATE_NORMAL 
     * For playback to be paused speed should equal CARD_PLAYRATE_PAUSE 
     * If speed is set to any other value CARD_PlaybackInvalidArg will be 
     * returned.
     *
     * If setSpeed is called before open() or feedData()
     * has been called CARD_PlaybackInvalidState will be returned.
     *
     * If setSpeed is called after endOfStream() is called
     * CARD_PlaybackInvalidState will be returned.
     *
     * Otherwise CARD_PlaybackSuccess will be returned
     *
     * @param[in] speed CARD_PLAYRATE_NORMAL or CARD_PLAYRATE_PAUSE 
     */
    virtual CARD_PlaybackStatus setSpeed(int32_t speed);

    /**
     * Notify the playback device that the bit-stream data feeding has
     * reached the end of stream.
     *
     * If endOfStream is called before open() or feedData()
     * has been called CARD_PlaybackInvalidState will be returned.
     *
     * Otherwise CARD_PlaybackSuccess will be returned
     */
    virtual CARD_PlaybackStatus endOfStream();

    /**
     * Get event (if any) from the device event queue (first posted
     * event gets retrieved first).
     *
     * @param[out] eventID the event identifier for the type of posted
     *             event.
     * @param[out] eventParam1 event parameter 1, actual meaning
     *             depends on the eventID.
     *
     * @return CARD_PlaybackSuccess if an event is found,
     *         otherwise CARD_PlaybackEmpty (event
     *         queue is empty)
     */
    virtual CARD_PlaybackStatus getEvent(CARD_PlaybackEventID& eventID, uint64_t &eventParam1);

    /**
     * Get the current PTS value from the playback device
     *
     * If getPts is called before playback is started this function
     * will return CARD_PlaybackFailure (pts set to zero).
     *
     * Otherwise it will return success with the current playback PTS value
     * (45kH)
     *
     * @param[out] pts value returned;
     *
     */
    virtual CARD_PlaybackStatus getPts(TIME45k &pts);

    /**
     * Flush the playback device, which includes stop
     * decoding/rendering audio/video immediately and discard buffered
     * data in or between all decoding/rendering stages.
     *
     * After flush, the playback speed is reset to paused, and the
     * end-of-stream state is cleared.
     *
     * flush will have no effect if no data has been feed, but will still
     * return success.
     *
     * If flush is called before open() is called, CARD_PlaybackInvalidState 
     * will be returned.
     *
     * Otherwise CARD_PlaybackSuccess will be returned
     *
     * @param[in] hold_picture   true if the caller wants the last picture displayed
     *                           to held;  false if the caller wants the displayed blank
     *
     */
    virtual CARD_PlaybackStatus flush(bool hold_picture);

    /**
     * Old version maintained for compatibility
     */
    virtual CARD_PlaybackStatus flush(void);

    /**
     * return an array of the supported video codecs
     *
     * @param[out] count - number of video codecs in the array;
     *
     */
    virtual const CARD_VideoCodec *getVideoCodecs(uint32_t &count);

    /**
     * return an array of the supported audio codecs
     *
     * @param[out] count - number of audio codecs in the array;
     *
     */
    virtual const CARD_AudioCodec *getAudioCodecs(uint32_t &count);

    /**
     * return an array of the supported media encryption types
     *
     * @param[out] count - number of audio codecs in the array;
     *
     */
    virtual const CARD_EncryptionType *getEncryptionTypes(uint32_t &count);

    /* this function is used to set the output security after the playback
     * device has been opened.
     *
     * If the function call is successful, it will return CARD_PlaybackSuccess
     * otherwise a failure status will be returned
     *
     * @param[in] security       details of output security protection required 
     */
    virtual CARD_PlaybackStatus setOutputSecurity(CARD_PlaybackSecurity &security);

    virtual void getDefaultSettings(CARD_DecodeSettings *pSettings);	
	virtual void getVideoPidChannelStatus(CARD_PidChannelStatus *pidStatus);
	virtual void getAudioPidChannelStatus(CARD_PidChannelStatus *pidStatus);

    bool m_playing;

    void (*m_bufferCallback)(void*);

private:

    /**
     * starting playback
     *
     * @param[out] pSettings - pointer to default settings;
     *
     */
    NEXUS_Error startPlayback(CARD_DecodeSettings *pSettings);

    /**
     * stopping playback
     *
     * @param[out] - none
     *
     */
    void stopPlayback(void);

    /**
     * pausing playback
     *
     * @param[out] - none
     *
     */
    CARD_PlaybackStatus pausePlayback(void);
	
    /**
     * initializing playback
     *
     * @param[out] - none
     *
     */
    NEXUS_Error initPlayback(void);

    /**
     * first PTS callback
     *
     * @param[out] context - context pointer;
     * @param[out] param - unused;
     *
     */
    static void firstPTS(void *context, int param);

    /**
     * source pending callback
     *
     * @param[out] context - context pointer;
     * @param[out] param - unused;
     *
     */
    static void sourcePendingCallback(void *pContext, int param);

    /**
     * data exhausted callback
     *
     * @param[out] context - context pointer;
     * @param[out] param - unused;
     *
     */
    static void dataExhausted(void *context, int param);

    /**
     * video play callback
     *
     * @param[out] context - context pointer;
     * @param[out] param - unused;
     *
     */
    static void playpumpDataCallback(void *context, int param);

    /**
     * get current pts
     *
     * @param[out] - none 
     *
     */
    uint64_t getCurrentPTSin_ms(void);

    /**
     * pausing video decoder
     *
     * @param[out] - none 
     *
     */
    CARD_PlaybackStatus Pause(void);

    // Tasks

    /**
     * display display task
     *
     * @param[in] context - device context
     *
     */
    static void videoDisplayTask(void *context);

    // Helpers

    /**
     * post playback events
     *
     * @param[out] - none
     *
     */
    void postPlaybackEvents(void);

    void convertParams(CARD_StreamType   streamType, 
                                  CARD_PlaybackVideoParams *VideoParams,
                                  CARD_PlaybackAudioParams *AudioParams);

    BKNI_MutexHandle    m_mutex;

    //Device specific Resources Handles
    NEXUS_AudioDecoderHandle   m_audioPassthrough,
                                                m_audioDecoder;
    NEXUS_VideoDecoderHandle   m_videoDecoder;
    NEXUS_StcChannelHandle       m_stcChannel;
    NEXUS_AudioDacHandle          m_audioDac;
    NEXUS_SpdifOutputHandle      m_spdif;
    NEXUS_PidChannelHandle       m_videoPidChannel,
                                                m_audioPidChannel,
                                                m_pcrPidChannel;
    NEXUS_VideoWindowHandle    m_videoWindow;
    NEXUS_PlaypumpHandle         m_Playpump;
    NEXUS_DisplayHandle            m_display;

    NEXUS_SurfaceHandle      m_frameBufferSurface; // nexus frame buffer surface;

    const BcmNexus_StreamPlayer_Resources  *m_resources;
    BcmNexus_StreamPlayer_Resources_Config m_resConfig;

    NEXUS_AudioDecoderStartSettings  m_audioProgram;
    NEXUS_VideoDecoderStartSettings   m_videoProgram;
    NEXUS_PlatformConfiguration          m_platformConfig;


    bool        m_paused;
    bool        m_firstPts;
    bool        m_started;
    bool        m_initialized;
    bool        m_dataExhausted;
    bool        m_lastPicture;
	bool		m_encrypt;
	CARD_EncryptionType m_encryptType;
	uint8_t * m_dmaBuf_Src;

    int           m_lastPictureCount;

    bool    volatile m_endOfStream;
    bool    volatile m_videoFrameStepping;
    bool    volatile m_endOfStreamEventPosted;
    bool    volatile m_dataExhaustedEventPosted;
    bool    volatile m_runningDeviceTasks;

    int64_t volatile m_lastPostedPresentationTimestamp;

    uint64_t   m_playTime;	

    uint32_t   m_videoStreamNumber;
    uint32_t   m_audioStreamNumber;
    uint32_t   m_prevStreamIndex;
    uint32_t   m_preroll;

    BKNI_EventHandle   m_videoEvent;

    std::auto_ptr<EventQueue>      m_eventQueue;

    // use B_Thread_xxxx here
    B_ThreadHandle    m_videoDisplayThread;

    NEXUS_VideoCodec m_supportedVideoCodecs[NEXUS_VideoCodec_eMax];
    NEXUS_AudioCodec m_supportedAudioCodecs[NEXUS_AudioCodec_eMax];
	//uint8_t * m_dmaBuf_Src;
};

#endif // CARD_PLAYBACKDEVICE_H
