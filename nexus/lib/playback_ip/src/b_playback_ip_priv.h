/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef __B_PLAYBACK_IP_PRIV__
#define __B_PLAYBACK_IP_PRIV__

#include <fcntl.h>

#ifndef _WIN32_WCE

#ifdef __vxworks
  #include <sysLib.h>
  #include <string.h>
  #include <stdlib.h>
  #include <logLib.h>
  #include <sys/times.h>
  #include <selectLib.h>
  #include <taskLib.h>
  #include <semLib.h>
  #include <sys/ioctl.h>
  #include <hostLib.h>
#else
  #include <memory.h>
  #include <pthread.h>
  #include <sys/time.h>
#endif

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#else /* _WIN32_WCE */
#include <windows.h>
    typedef HANDLE pthread_t;
    #define close(x) closesocket(x)
    unsigned long errno = 0;
    #define EINTR -1
#endif /* _WIN32_WCE */

#include <inttypes.h>
#include "b_os_lib.h"
#include "bdbg.h"
#include "bkni.h"
#include "berr.h"
#include "bkni_multi.h"
#include "bpool.h"
#include "barena.h"
#include "bioatom.h"
#include "blst_queue.h"
#ifdef SPF_SUPPORT
#include "brtp.h"
#endif /* SPF_SUPPORT */
#include "brtp_spf.h"
#include "bfile_io.h"
#include "bfile_buffered.h"
#include "nexus_types.h"
#include "nexus_file.h"
#include "nexus_file_pvr.h"
#include "bmedia_probe.h"
#include "bmedia_player.h"
#include "bmedia_util.h"
#include "b_playback_ip_tts_throttle.h"
#include "b_playback_ip_xml_helper.h"

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"
#define ENCRYPTION_PADDING  DTCP_CONTENT_PACKET_HEADER_SIZE + HTTP_AES_BLOCK_SIZE-1
#endif
#define STREAMING_BUF_SIZE (DIO_BLK_SIZE*188*4)
#define STREAMING_BUF_MULTIPLE 32 /* can't be less than this as read size has to be DIO aligned */
#define HTTP_AES_BLOCK_SIZE 16
#define HTTP_DEFAULT_PORT 80

#ifdef B_HAS_NETACCEL
#include "bnetaccel_info.h"
#endif

#define RTP_PAYLOAD_TYPE_MP2T           33          /* MPEG2 TS stream */
#define RTP_PAYLOAD_TYPE_ES             97          /* ES streams */
#define MAX_READ                        160
#define TEST_BUFFER_SIZE                65536
#define IP_MAX_PWM_VALUE                0x7FFF
#define IP_MIN_PWM_VALUE                (-IP_MAX_PWM_VALUE)
#define IP_POSTCHARGE_PERIOD_MSEC       5000
#define IP_MAX_DECODE_POLLS             10
#define IP_UNDERFLOW_PERIOD_MSEC        100

/* slew 270 Hz (10 ppm) @ 100 Hz/sec,
 * in steps of 10Hz
 */
#define IP_PWM_STEP_PERIOD              100         /* 0.1 second */
#define IP_PWM_STEP_SIZE                75          /* 10Hz (Hz/bit = 4386/0x7fff = 0.13385 Hz/bit, ie. 0.00494 ppm/bit) */
#define IP_PWM_STEPS_PER_VIOLATION      27

#if defined (LINUX)
  #define STATUS_REPORTING_CONSTANT     1
  #define NEW_RMEM_MAX                  (1*256*1024)
#else
  #define STATUS_REPORTING_CONSTANT     0
#endif

#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

/* #define B_RECORD_IP */
#ifdef B_RECORD_IP
  #include <stdio.h>
#endif

#ifdef __vxworks
#if !defined (USE_PTHREAD)
    typedef int pthread_t;
#endif
#endif

#define PBIP_CHECK_GOTO(                                                                    \
    expression,     /* Condition expected to be true for success.               */         \
    errorMessage,   /* If expression false, print this with BDBG_ERR.           */         \
    errorLabel,     /* If expression false, goto this label.                    */         \
    errorValue,     /* If expression false, set errorVariable to this value.    */         \
    errorVariable   /* If expression false, set this to errorValue              */         \
    )                                                                                      \
    do {                                                                                   \
        if (!(expression)) {                                                               \
                BDBG_ERR(errorMessage);                                                    \
                errorVariable = BERR_TRACE(errorValue);                                    \
          goto errorLabel;                                                                 \
        }                                                                                  \
    } while(0)

/**
* Playback IP module can receive IP encapsulated AV data from
* network using following 3 compile options:
*       -use normal sockets (true when B_HAS_NETACCEL &
*        B_HAS_PLAYPUMP_IP is not defined)
*       -use legacy IP Playpump (true when B_HAS_PLAYPUMP_IP is
*        defined), this i/f is being deprecated)
*       -use recommended accelerated sockets (which uses
*        Broadcom's Accelerated Sockets i/f) (true when
*        B_HAS_NETACCEL is defined & B_HAS_PLAYPUMP_IP is not
*        defined).
*/
#define PKTS_PER_CHUNK 8        /* how many packets to process at one time */
#if !defined(B_HAS_NETACCEL) || defined(B_HAS_PLAYPUMP_IP)
#define PKTS_PER_READ 1                 /* For normal sockets, UDP allows only 1pkt/recvfrom call*/
#else
#define PKTS_PER_READ PKTS_PER_CHUNK    /* Accelerated Sockets allows packet aggregation */
#endif
#define IP_MAX_ITEMS    (PKTS_PER_CHUNK * 128)/* max number of packets that can be pending in the filter */
#define IP_MAX_PKT_SIZE 1518
#define TMP_BUF_SIZE    1518
#define DISCARD_BUFFER_SIZE IP_MAX_PKT_SIZE*200

#define TS_PKT_SIZE                 188
#define TS_PKTS_PER_UDP_PKT         7
#define UDP_PAYLOAD_SIZE            TS_PKT_SIZE * TS_PKTS_PER_UDP_PKT
#define RTP_PAYLOAD_SIZE            UDP_PAYLOAD_SIZE
#define IP_HALT_TASK_TIMEOUT_MSEC   5000
#define IP_RECV_TIMEOUT_USEC        250000
#define IP_RECV_BURST_USEC          (IP_RECV_TIMEOUT_USEC/5)

#define abs(x) (x>0 ? x : -x)
#define MAX_BUFFER_SIZE (IP_MAX_PKT_SIZE * PKTS_PER_CHUNK)
/* Note: we used to maintain two separate data caches for supporting MP4 files where AV can be in far offsets */
/* However, such a support required double the memory. Also, since most common MP4 will have AV interleved */
/* within few seconds, so we may not need the 2nd cache. This also saves about 12MB of memory used by 2nd cache */
#define HTTP_MAX_DATA_CACHES 1
#define HTTP_SELECT_TIMEOUT 60  /* DLNA requirement */
#define HTTP_DATA_CACHE_CHUNK_SIZE 96000
#define HTTP_INDEX_CACHE_CHUNK_SIZE 16384
/* during runtime pre-charging, we keep a certain amount of previous data in the cache as for MP4 player can request some of the previous data */
#define HTTP_CACHE_DEPTH_FUDGE_FACTOR (10 * HTTP_DATA_CACHE_CHUNK_SIZE)

/* default cache sizes */
#define HTTP_INDEX_CACHE_SIZE (HTTP_DATA_CACHE_CHUNK_SIZE * 20)
#define HTTP_DATA_CACHE_SIZE (HTTP_DATA_CACHE_CHUNK_SIZE * 20)

/* timeout for app thread to wait for acknowledgement from HTTP thread of starting or stopping pre-charging */
#define HTTP_PRE_CHARGE_EVENT_TIMEOUT 6000

#define B_PLAYBACK_IP_INFO_FILE_EXTENSION ".info"
#define B_PLAYBACK_IP_INDEX_FILE_EXTENSION ".nav"
#define DIO_BLK_SIZE        (512)
#define DIO_MASK            (DIO_BLK_SIZE - 1)
#define DIO_ALIGN(x) (unsigned char *)(((unsigned long) x + DIO_MASK) & ~DIO_MASK)

#ifndef timeAdd
    #define timeAdd(_a, _b, _res)              \
  do {                          \
    (_res)->tv_usec = (_a)->tv_usec + (_b)->tv_usec;    \
    (_res)->tv_sec = (_a)->tv_sec + (_b)->tv_sec;   \
    if ((_res)->tv_usec >= 1000000)         \
      {                         \
        (_res)->tv_usec -= 1000000;         \
        (_res)->tv_sec++;               \
      }                         \
  } while (0)
#endif

#ifndef timeSub
    #define timeSub(_a, _b, _res)              \
  do {                          \
    (_res)->tv_usec = (_a)->tv_usec - (_b)->tv_usec;    \
    (_res)->tv_sec = (_a)->tv_sec - (_b)->tv_sec;   \
    if ((_res)->tv_usec < 0) {              \
      (_res)->tv_usec += 1000000;           \
      (_res)->tv_sec--;                 \
    }                           \
  } while (0)
#endif

#ifdef EROUTER_SUPPORT
#define IOCTL_ADD_DELETE_WANUNICAST_PACKET  816
#define WAN_PORT_DELETE                     0
#define WAN_PORT_ADD                        1
typedef struct ioctl_if_list_portnums {
    unsigned short add;         // 0 - delete, 1 add
    unsigned short portNumb;
} IOCTL_IF_LIST_PORTNUMS;
#endif

typedef struct B_PlaybackIp_GlobalContext{
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}B_PlaybackIp_GlobalContext;


/**
Summary:
This struct defines the fields of the RTP header
 **/
/* # of rtp header cached for RTCP stats purpose */
#define B_MAX_RTP_HEADERS              4000
typedef struct B_PlaybackIp_RtpStatsHeader {
    uint8_t version;
    uint8_t padding;
    bool extension;
    uint8_t csrc_count;
    uint8_t marker;
    uint8_t payload_type;
    uint16_t sequence_num;
    uint32_t timestamp;
    uint32_t ssrc;

    struct {
        uint16_t defined_by_profile;
        uint16_t length;
    } extension_header;
} B_PlaybackIp_RtpStatsHeader;

typedef struct B_PlaybackIp_RtpHeaderData {
    B_PlaybackIp_RtpStatsHeader header[B_MAX_RTP_HEADERS];
    unsigned entry_cnt;
    BKNI_MutexHandle lock;
} B_PlaybackIp_RtpHeaderData;

#define RTP_FIXED_HEADER_SIZE 12

typedef enum B_PlaybackIp_RtspCtrlCmdType {
    B_PlaybackIp_RtspCtrlCmdType_Play,
    B_PlaybackIp_RtspCtrlCmdType_Pause,
    B_PlaybackIp_RtspCtrlCmdType_Resume
} B_PlaybackIp_RtspCtrlCmdType;

typedef struct B_PlaybackIp_RtspCtrlParams {
    long start;
    long end;
    long scale;
} B_PlaybackIp_RtspCtrlParams;

typedef struct B_PlaybackIpRtspCtrlCmd {
    B_PlaybackIp_RtspCtrlCmdType type;
    B_PlaybackIp_RtspCtrlParams params;
} B_PlaybackIp_RtspCtrlCmd;

/**
* Summary:
* State of the software dejitter buffer
* (used only for software managed TSM case)
**/
typedef enum B_PlaybackIpBufferState {
    B_PlaybackIpBufferState_eInit,
    B_PlaybackIpBufferState_ePreCharging,
    B_PlaybackIpBufferState_ePostCharging,
    B_PlaybackIpBufferState_ePlaying,
    B_PlaybackIpBufferState_eOverflowing,
    B_PlaybackIpBufferState_eUnderflowing
} B_PlaybackIpBufferState;

/**
* Summary:
* structure to hold the incoming packets
* (contains either one packet (for standard socket or multiple
* packets for accelerated sockets)
**/
struct B_PlaybackIpItem;    /* forward declaration */
BLST_Q_HEAD(B_PlaybackIpItemQueue, B_PlaybackIpItem);
typedef struct B_PlaybackIpItem
{
    BLST_Q_ENTRY(B_PlaybackIpItem) item_queue;
    BLST_Q_ENTRY(B_PlaybackIpItem) active_item_queue;
    uint8_t *data;          /* Netaccel aggregates incoming packets, malloc buffer for multiple pkts */
    B_PlaybackIpHandle playback_ip;
    int item_index;
    bool block_end;
    struct B_PlaybackIpItem *block_start; /* points to the 1st item in a item block */
                                        /* For Accelerated socket, item block logically groups PKTS_PER_CHUNK items into one block */
                                        /* as accelerated sockets need a contiguous buffer for reading multiple pkts at once*/
}B_PlaybackIpItem;

/* implementation of bfile_io_read */
struct bfile_io_read_net {
    struct bfile_io_read self;
    B_PlaybackIpHandle playback_ip;
    int fd;
    off_t offset;
    bool socketError;   /* set for any of these conditions: 1) n/w error (EPIPE), 2) server closed, 3) bytes read != bytes asked 4) EOF */
};

/* implementation of NEXUS_FilePlay */
typedef struct bfile_in_net {
    struct NEXUS_FilePlay self;
    struct bfile_io_read_net data;
    struct bfile_io_read_net index;
} bfile_in_net;

/* implementation of bfile_io_write */
struct bfile_io_write_net {
    struct bfile_io_write self;
    B_PlaybackIpHandle playback_ip;
    int fd;
    int writeQueueSize; /* socket write queue size */
    int selectTimeouts; /* number of consecutive select timeouts indicating socket write q full event */
    int writeQueueFullTimeouts; /* number of consecutive times we were unable to stream due to write q being full */
    bool liveStreaming; /* true if we are streaming a live channel */
    B_PlaybackIpProtocol streamingProtocol; /* HTTP, RTP, or UDP */
    struct sockaddr_in streamingSockAddr;   /* socket address info of streaming destination */
    struct sockaddr_in streamingSockAddrLocal;   /* socket address info of streaming destination (local client) */
    int streamingFdLocal;   /* socket fd info of streaming destination (local client) */
    int rtpPayloadType; /* value of payload type field of the outgoing RTP header */
    unsigned short nextSeq; /* next seq # for RTP protocol */
    struct iovec *iovec;
    const char *interfaceName;
#ifdef B_HAS_DTCP_IP
    void *dtcpCtx;
    void *dtcpStreamHandle;
    void *dtcpAkeHandle;
    unsigned char *encryptedBuf;
    unsigned int encryptedBufSize;
    unsigned char *pcpHeader;
    unsigned int pcpHeaderSize;
    int residualBytesLength;    /* # of bytes that were not encrypted in the previous encrypt operation due to AES block size limitation of 16 bytes */
    char residualBytesToEncrypt[ENCRYPTION_PADDING]; /* actual bytes to encrypt */
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    NEXUS_KeySlotHandle pvrDecKeyHandle; /* handle for pvr decryption */
    NEXUS_DmaHandle dmaHandle;
    NEXUS_DmaJobHandle dmaJobHandle;
    BKNI_EventHandle event;
#endif
    uint64_t totalBytesConsumed;
    struct timeval totalTime;
    struct timeval minTime;
    struct timeval maxTime;
    uint32_t totalIterations;
    bool stopStreaming; /* this flag is set when app wants streaming thread to stop streaming */
    NEXUS_HeapHandle heapHandle;
    unsigned ipTtl; /* Time To Live (TTL) for IPv4 */
};

/* implementation of NEXUS_FileRecord */
typedef struct bfile_out_net {
    struct NEXUS_FileRecord self;
    struct bfile_io_write_net data;
    struct bfile_io_write_net index;
} bfile_out_net;

typedef struct B_PlaybackIpNetIo {
    int (*read)(void *securityHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len); /* method to read data */
    int (*write)(void *securityHandle, volatile B_PlaybackIpState *playbackIpState, int sd, char *wbuf, int wbuf_len); /* method to write data */
    void (*suspend)(void *securityHandle); /* method to suspend session. Ip socket closed, but security session intact */
    void (*close)(void *securityHandle); /* method to close security  session. */
    void (*shutdown)(void *securityHandle, unsigned int flag); /* method to shutdown security  session. */
} B_PlaybackIpNetIo;

typedef struct B_PlaybackIpAvCacheStats {
    unsigned int hits;
    unsigned int partialHits;
    unsigned int cacheable;
    unsigned int miss;
}B_PlaybackIpAvCacheStats;

/* Cache structure used to hold index & data */
typedef struct B_PlaybackIpAvCache {
    char *cache; /* pointer to buffer where actual data is cached */
    bool inUse;  /* true if data cache is being used */
    volatile unsigned int depth; /* how many bytes are currently cached */
    volatile unsigned int size; /* max bytes that can be cached */
    off_t startOffset; /* linear offset of current first byte in data cache */
    off_t endOffset;   /* linear offset of current last byte in data cache */
    off_t firstOffset;   /* 1st data offset asked by the player, used to detect rewind condition */
    unsigned int writeIndex; /* byte offset in data cache where next byte (from n/w) can be written into */
    unsigned int readIndex; /* byte offset in data cache where startOffset starts */
    B_Time lastUsedTime; /* time in msec when this cache was last used */
    B_PlaybackIpAvCacheStats stats; /* cache performance stats */
} B_PlaybackIpAvCache;


#if defined (B_HAS_HLS_PROTOCOL_SUPPORT) || defined (B_HAS_MPEG_DASH_PROTOCOL_SUPPORT)

typedef struct EncryptionInfo {
    bool useMediaSegSeqAsIv;
    union {
        struct {
           bool none;
        } none;
        struct {
            unsigned char key[16]; /* 128 bit key for AES-128 */
            unsigned char iv[16]; /* 128 bit IV for AES-128 */
        } aes128;
    }encryptionMethod;
} EncryptionInfo;

/*struct MediaFileSegmentInfo;*/    /* forward declaration */
typedef struct MediaFileSegmentInfo{
    BLST_Q_ENTRY(MediaFileSegmentInfo) next;
    char *absoluteUri; /* complete URI string of the media segment */
    char *uri; /* file name part of the media file segment uri */
    char *server; /* pointer to the server name of the media segment uri */
    unsigned port; /* port number of the media segment uri */
    B_PlaybackIpProtocol protocol;
    B_PlaybackIpSecurityProtocol securityProtocol;  /* security protocol being used */
    NEXUS_PlaybackPosition duration; /* duration of media file segment in milli-seconds */
    char *title; /* media segment title */
    char *date; /* date & time associated with a URI */
    char *time;
    int mediaSequence; /* number of this uri in the media stream presentation */
    bool markedDiscontinuity; /* need to reset the decoders when starting playback from this point */
    /* security *sec; */ /* security keys associated with this uri */
    EncryptionInfo encryptionInfo; /* stores info for decrypting the media */
    unsigned segmentLength; /* length of segment in bytes, defined via the BYTERANGE tag */
    off_t segmentStartByteOffset; /* offset of the starting byte of this segment */
    bool hasAudio;
    bool hasVideo;
} MediaFileSegmentInfo;
BLST_Q_HEAD(MediaFileSegmentInfoQueue, MediaFileSegmentInfo);

typedef enum HlsSegmentCodecType
{
    HlsSegmentCodecType_eUnknown,
    HlsSegmentCodecType_eAudioVideo,
    HlsSegmentCodecType_eAudioOnly,
    HlsSegmentCodecType_eVideoOnly,
    HlsSegmentCodecType_eMax
} HlsSegmentCodecType;

/* list of playlist files built from a variant playlist file */
typedef struct PlaylistFileInfo{
    BLST_Q_ENTRY(PlaylistFileInfo) next;    /* next playlist file */
    MediaFileSegmentInfo *currentMediaFileSegment; /* pointer to the current segment being downloaded */
    struct MediaFileSegmentInfoQueue mediaFileSegmentInfoQueueHead; /* queue of media segments belonging to this playlist file */
    /* playlist file attributes */
    char *absoluteUri; /* complete URI string of the playlist file */
    char *uri;  /* file name part of the playlist file uri */
    char *server; /* pointer to the playlist server name uri */
    unsigned port; /* port number of the playlist server uri */
    B_PlaybackIpProtocol protocol;
    short version; /* number (1 by default, 2 w/ AES) */
    short programId; /* number that uniquely identifies a particular presentation within the scope of a playlist file */
    unsigned bandwidth; /* in bps (0 if the variant playlist file doesn't exist) */
    NEXUS_PlaybackPosition totalDuration; /* playlist duration in milli-seconds, sum of durations of media files within a playlist */
    NEXUS_PlaybackPosition maxMediaSegmentDuration; /* max duration of any media stream segment: in msec */
    int mediaSegmentBaseSequence; /* base sequence number of the media sequence in the current playlist */
    bool allowCaching; /* if set, media stream can be cached (i.e. recorded) */
    int initialMinimumReloadDelay; /* duration of the last media file in playlist: used the interval for re-downloading the unbounded playlist */
    /*
       -codecs: in RFC4281 format
       -resolution:HxV
    */
    /* state needed during playback */
    bool bounded; /* (set if end tag is present) */
    int lastUsedUriIndex; /* index of the last URI being downloaded */
    int numMediaSegments; /* number of media segments in a playlist */
    bool playlistHasChanged; /* set when playlist re-download has refreshed the playlist */
    HlsSegmentCodecType segmentCodecType;
    bool altAudioRenditionsEnabled;
    char *altAudioRenditionGroupId;
} PlaylistFileInfo;
BLST_Q_HEAD(PlaylistFileInfoQueue, PlaylistFileInfo);

typedef enum AltRenditionType {
    HlsAltRenditionType_eAudio,
    HlsAltRenditionType_eVideo,
    HlsAltRenditionType_eSubtitles,
    HlsAltRenditionType_eClosedCaptions,
    HlsAltRenditionType_eMax
}AltRenditionType;

/* list of playlist files built from a variant playlist file */
typedef struct AltRenditionInfo {
    BLST_Q_ENTRY(AltRenditionInfo) next;    /* next altRendition entry */

    /* Required Alternate Rendition attributes */
    AltRenditionType type;
    char *groupId; /* group to which this rendition entry belogs. */
    char *name; /* description for the rendition. */

    /* Optional Alternate Rendition attributes */
    char *language; /* language tag: RFC 5646. */
    char *assocLanguage; /* associated language tag: RFC 5646. */
    bool defaultRendition; /* if true, this rendition should be played if player doesn't choose one. */
    char *absoluteUri; /* complete URI string if provided. */
    char *uri;  /* file name part URI string. */
    char *server; /* pointer to the playlist server name uri */
    unsigned port; /* port number of the playlist server uri */
    B_PlaybackIpProtocol protocol;

    /* Not yet used attributes. */
    bool autoSelect; /* TODO: not yet sure on the exact usage of this tag!. */
    bool forced; /* TODO: not yet sure on the exact usage of this tag!. */
    char *inStreamId; /* for Closed captions. */

} AltRenditionInfo;
BLST_Q_HEAD(AltRenditionInfoQueue, AltRenditionInfo);

#endif  /*  defined (B_HAS_HLS_PROTOCOL_SUPPORT) || defined (B_HAS_MPEG_DASH_PROTOCOL_SUPPORT) */

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT

#define HLS_PES_AUDIO_ES_ID 0xC0
#define HLS_NUM_SEGMENT_BUFFERS 6
#define HLS_NUM_SEGMENT_BUFFER_SIZE (2*1024*1024)
typedef struct HlsSegmentBuffer {
    char *buffer;
    char *bufferOrig;
    int bufferSize;
    int bufferDepth;
    bool filled;    /* set when the buffer is filled w/ media segment */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to this structure by HLS download & playback threads */
    bool markedDiscontinuity;
    bool containsEndOfSegment; /* set if this buffer contains the end of current segment. */
}HlsSegmentBuffer;

typedef struct B_PlaybackIpMediaProbeState
{
    bmedia_probe_config probe_config;
    char *stream_info;
    struct bfile_io_read_net index;
    const bmedia_probe_stream *stream;
    bmedia_probe_t probe;
    B_PlaybackIpPsiInfo psi;
    HlsSegmentBuffer *segmentBuffer;
    bool quickMediaProbe; /* if set, quick media  probe is carried out: for HLS, only first few Ts packets are examined for PSI info  */
}B_PlaybackIpMediaProbeState;

typedef struct HlsNetworkBandwidthContext
{
    unsigned long       sampleLimit;    /* number of samples to average together */
    unsigned long       accumulatedTimeInMs;
    unsigned long       accumulatedBytes;
    unsigned long       accumulatedTimeWeightedKBps;
    unsigned long       accumulatedByteWeightedKBps;
    unsigned long       accumulatedSamples;
    unsigned long       filteredBandwidthInKbitsPerSecond;
}HlsNetworkBandwidthContext;
#define HLS_BANDWIDTHCONTEXT_TO_PRINTF_ARGS(pObj)     \
            "BwContext: sampleLimit: %lu accumTimeInMs: %lu accumBytes: %lu accumSamples: %lu accumTmWeightedKBps: %lu accumByteWeightedKBps: %lu bwInKbPerSec: %lu", \
            (pObj)->sampleLimit,                              \
            (pObj)->accumulatedTimeInMs,                      \
            (pObj)->accumulatedBytes,                         \
            (pObj)->accumulatedSamples,                       \
            (pObj)->accumulatedTimeWeightedKBps,              \
            (pObj)->accumulatedByteWeightedKBps,              \
            (pObj)->filteredBandwidthInKbitsPerSecond

typedef struct HlsSessionState {
    struct PlaylistFileInfoQueue playlistFileInfoQueueHead;
    struct AltRenditionInfoQueue altRenditionInfoQueueHead;
    struct PlaylistFileInfoQueue iFramePlaylistFileInfoQueueHead;
    PlaylistFileInfo *currentPlaylistFile;
    PlaylistFileInfo *currentIFramePlaylistFile;
    char *playlistBuffer;
    int playlistBufferSize;
    int playlistBufferDepth;
    unsigned int playlistBufferReadIndex;
    B_ThreadHandle hlsSegmentDownloadThread;
    bool hlsSegmentDownloadThreadDone; /* set when download thread either gets an error or is done downloading all media segments in a playlist */
    bool hlsPlaybackThreadDone;
    B_ThreadHandle hlsPlaylistReDownloadThread;
    bool hlsPlaylistReDownloadThreadDone;
    HlsSegmentBuffer segmentBuffer[HLS_NUM_SEGMENT_BUFFERS];
    BKNI_EventHandle bufferFilledEvent;
    BKNI_EventHandle bufferEmptiedEvent;
    BKNI_EventHandle reDownloadPlaylistEvent;
    BKNI_EventHandle segDownloadThreadPausedEvent;
    BKNI_EventHandle playlistReDownloadThreadPausedEvent;
    BKNI_EventHandle playbackThreadPausedEvent;
    BKNI_EventHandle segDownloadThreadPauseDoneEvent;
    BKNI_EventHandle playbackThreadPauseDoneEvent;
    BKNI_EventHandle playlistReDownloadThreadPauseDoneEvent;
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to this structure by Nexus IO, App, Http threads */
    int networkBandwidth; /* network b/w that is achieved while downloading a media segment */
    int playlistSocketFd;   /* socket file descriptor of the playlist file */
    bool downloadedAllSegmentsInCurrentPlaylist; /* set when all media segments in the current playlist have been downloaded: used to detect when to switch playlists in case base seq # wraps around */
    bool resetPlaylist; /* set to reset the playlist so that we start download from the 1st segment of the playlist */
    unsigned maxNetworkBandwidth; /* max network bandwidth set via max_nw_bw */
    unsigned currentPlaylistBandwidth; /* b/w of the playlist currently being used for playback */
    bool seekOperationStarted; /* set when HLS specific seek work is in progress */
    NEXUS_PlaybackPosition lastSeekPosition;
    HlsNetworkBandwidthContext bandwidthContext;
    B_PlaybackIpMediaProbeState mediaProbe; /* media probe state info, we do probe when we switch from media segment of another variant */
    B_Time lastSegmentDownloadStartTime; /* time when we started the last segment download */
    unsigned lastSegmentDownloadTime; /* time, in msec, to download the last full segment */
    unsigned lastPartialSegmentDownloadTime; /* time to download the part of last segment as we didn't have big enough buffer to hold the full segment */
    unsigned lastSegmentBitrate;
    unsigned lastSegmentDuration;
    char *lastSegmentUrl;
    unsigned lastSegmentSequence;
    unsigned playlistBandwidthToUseInTrickmode;
    bool useLowestBitRateSegmentOnly; /* Set if we are doing trickmodes for HLS protocol where we use simulated STC mode to drive it faster and need to feed segments as fast as we can. */
    unsigned segmentCounter; /* counter that increments by 1 for every segment that we download. */
    bool restartPlaylistRampUpByBandwidth; /* set during seek, enables logic to re-start the next segment from the lowest bandwidth & go up from there! */
    unsigned dropEveryNthSegment; /* which segment should get dropped at the higher speeds. */
    unsigned segmentCount; /* how many segments have been downloaded & fed during a particular trickmode speed. */
    NEXUS_PlaybackPosition segmentsRemovedDuration; /* set to the duration of segments removed when a live playlist is updated, needed to calculate the next segment position. */
    bmedia_pes_info pesInfo;
    bool useIFrameTrickmodes;
}HlsSessionState;
#endif

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT


/* Define data structures for bandwidth accumulation and
 * averaging. */

typedef struct MpegDashBandwidthContext
{
    unsigned long       sampleLimit;    /* number of samples to average together */

    unsigned long       accumulatedTimeInMs;
    unsigned long       accumulatedBytes;
    unsigned long       accumulatedTimeWeightedKBps;
    unsigned long       accumulatedByteWeightedKBps;
    unsigned long       accumulatedSamples;
    unsigned long       filteredBandwidthInKbitsPerSecond;
} MpegDashBandwidthContext;
#define MPEG_DASH_BANDWIDTHCONTEXT_TO_PRINTF_ARGS(pObj)     \
            "BwContext: sampleLimit: %lu accumTimeInMs: %lu accumBytes: %lu accumSamples: %lu accumTmWeightedKBps: %lu accumByteWeightedKBps: %lu bwInKbPerSec: %lu", \
            (pObj)->sampleLimit,                              \
            (pObj)->accumulatedTimeInMs,                      \
            (pObj)->accumulatedBytes,                         \
            (pObj)->accumulatedSamples,                       \
            (pObj)->accumulatedTimeWeightedKBps,              \
            (pObj)->accumulatedByteWeightedKBps,              \
            (pObj)->filteredBandwidthInKbitsPerSecond

/* Define data structures for downloading segments... shared
 * by B_PlaybackIp_MpegDashSetupHttpSessionToServer() and
 * B_PlaybackIp_MpegDashDownloadMediaSegment().  */


#define MPEG_DASH_NUM_SEGMENT_BUFFERS 2
typedef struct MpegDashSegmentBuffer {
    char *buffer;
    ssize_t bufferSize;
    ssize_t bufferDepth;
    int mp4BoxPrefixSize;
    bool allocated; /* buffer is in use and being filled or may actually be full. */
    bool filled;    /* set when the buffer is filled w/ media segment */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to this structure by download & playback threads */
    bool markedDiscontinuity;
    bool hasAudio;
    bool hasVideo;
} MpegDashSegmentBuffer;


typedef struct MpegDashDownloadContext
{
    MediaFileSegmentInfo  * mediaFileSegmentInfo;
    int                     socketFd;
    MpegDashSegmentBuffer * segmentBuffer;

    B_Time          beginTime;

    ssize_t          totalBytesExpected;
    ssize_t          totalBytesReadSoFar;
    unsigned long   elapsedTimeInMs;
    unsigned        networkBandwidth;

    bool            serverClosed;
} MpegDashDownloadContext;


/* Define internal representation for DateTime datatype */
typedef struct MpegDashDateTime
{
    bool            valid;      /* true => struct contains a DateTime, false => empty */
    time_t          utc_time_t; /* seconds since 00:00:00 on January 1, 1970, UTC */
    unsigned long   nsecs;      /* nanoseconds */
    long            tz_secs;    /* timezone expressed in seconds (used to  */
                                /* ...convert utc_time_t to local time).   */
} MpegDashDateTime;

#define MPEG_DASH_DATETIME_TO_PRINTF_ARGS(pObj)     \
            "DateTime: valid:%u utc_time_t:%lu nsecs:%lu tz_secs:%ld", \
            (pObj)->valid,                            \
            (pObj)->utc_time_t,                       \
            (pObj)->nsecs,                            \
            (pObj)->tz_secs

/* Define internal representation for Duration datatype */
typedef struct MpegDashDuration
{
    bool        valid;      /* true => struct contains a Duration, false => empty */
    bool        minusSign;  /* true => duration is negative */
    int         nsec;       /* nanoseconds */
    int         sec;
    int         min;
    int         hour;
    int         mday;
    int         mon;
    int         year;
} MpegDashDuration;

#define MPEG_DASH_DURATION_TO_PRINTF_ARGS(pObj)  \
            "Duration: valid:%u minusSign:%u year:%d "      \
            "mon:%d mday:%d hour:%d min:%d sec:%d nsec:%d", \
            (pObj)->valid,                         \
            (pObj)->minusSign,                     \
            (pObj)->year,                          \
            (pObj)->mon,                           \
            (pObj)->mday,                          \
            (pObj)->hour,                          \
            (pObj)->min,                           \
            (pObj)->sec,                           \
            (pObj)->nsec


/* Information from the "AnyText" element type.
 * This is a generalized form of the "anyURI" type from the MPEG-DASH spec.
 * It can hold any type that is represented by a single text string.
 */
typedef struct MpegDashAnyTextType {
    B_PlaybackIp_XmlElement   myXmlElem;  /* pointer to the MXmlElement corresponding to this struct */

    const char      * text;

    BLST_Q_ENTRY(MpegDashAnyTextType) nextAnyText;    /* used for list of AnyTextType */
} MpegDashAnyTextType;


/* Information from the BaseURL element type. */
typedef struct MpegDashBaseUrlInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    const char      * childData;
    const char      * serviceLocation;  /* type="xs:string" */
    const char      * byteRange;        /* type="xs:string" */

    BLST_Q_ENTRY(MpegDashBaseUrlInfo) nextBaseUrlInfo;    /* used for list of BaseUrlInfo */
} MpegDashBaseUrlInfo;


typedef struct MpegDashUrlInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from URLType of the MPEG-DASH XML schema */

        const char * sourceURL;  /* type="xs:anyURI" */
        const char * range;      /* type="xs:string" */

    /* End of data from URLType of the MPEG-DASH XML schema */

} MpegDashUrlInfo;


typedef struct MpegDashSegmentUrlInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from SegmentUrlType of the MPEG-DASH XML schema */
        const char * media;         /*  type="xs:anyURI" */
        const char * mediaRange;    /*  type="xs:string" */
        const char * index;         /*  type="xs:anyURI" */
        const char * indexRange;    /*  type="xs:string" */

    /* End of data from SegmentUrlType of the MPEG-DASH XML schema */

    BLST_Q_ENTRY(MpegDashSegmentUrlInfo) nextSegmentUrl;    /* used for list of SegmentUrlInfo */

} MpegDashSegmentUrlInfo;


/* Information from the RepresentationBase type. */
typedef struct MpegDashRepresentationBaseInfo {
    B_PlaybackIp_XmlElement   myXmlElem;  /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from RepresentationBaseType of the MPEG-DASH XML schema */
     /* BLST_Q_HEAD(mpegDashFramePackingInfoHead,              MpegDashFramePackingInfo)              mpegDashFramePackingInfoHead;             */
     /* BLST_Q_HEAD(mpegDashAudioChannelConfigurationInfoHead, MpegDashAudioChannelConfigurationInfo) mpegDashAudioChannelConfigurationInfoHead;*/
     /* BLST_Q_HEAD(mpegDashContentProtectionInfoHead,         MpegDashContentProtectionInfo)         mpegDashContentProtectionInfoHead;        */

        const char * profiles;
        unsigned width;
        unsigned height;
        const char * sar;
        const char * frameRate;
        const char * audioSamplingRate;
        const char * mimeType;
        const char * segmentProfiles;
        const char * codecs;
        const char * maximumSAPPeriod; /* double */
        unsigned startWithSAP;         /* SAP Types defined in ISO/IEC 14496-12-2012 Annex I (0 => Unknown) */
        const char * maxPlayoutRate;   /* double */
        unsigned codingDependency;     /* boolean */
        const char * scanType;
    /* End of data from RepresentationBaseType of the MPEG-DASH XML schema */

} MpegDashRepresentationBaseInfo;


typedef struct MpegDashSegmentBaseInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from SegmentBaseType of the MPEG-DASH XML schema */
        MpegDashUrlInfo     *initialization;         /* type="URLType" */
        MpegDashUrlInfo     *representationIndex;    /* type="URLType" */

        const char * timescale;                /* type="xs:unsignedInt"  (but leave as char * so we can tell if it was there or not)  */
        const char * presentationTimeOffset;   /* type="xs:unsignedInt"  (but leave as char * so we can tell if it was there or not)  */
        const char * indexRange;               /* type="xs:string"        */
        const char * indexRangeExact;          /* type="xs:boolean" default="false" (but leave as char * so we can tell if it was there or not)  */
    /* End of data from SegmentBaseType of the MPEG-DASH XML schema */

    BLST_Q_ENTRY(MpegDashSegmentBaseInfo) nextSegmentBaseInfo;    /* used for list of SegmentBaseInfo */
} MpegDashSegmentBaseInfo;


typedef struct MpegDashSegmentTimelineElemInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from the list elements of the SegmentTimelineType of the MPEG-DASH XML schema */

        /* Segment's "earliest_presentation_time" in units of
         * "SegmentBaseType.timescale". */
        const char * t;     /* type="xs:unsignedInt" */

        /* Segment's duration in units of "SegmentBaseType.timescale". */
        const char * d;     /* type="xs:unsignedInt" */

        /* Repeat count in addition to the first. */
        const char * r;     /* type="xs:unsignedInt" */

    /* End of data from MultipleSegmentBaseType of the MPEG-DASH XML schema */

    BLST_Q_ENTRY(MpegDashSegmentTimelineElemInfo) nextSegmentTimelineElemInfo;    /* used for list of SegmentTimelineElemInfo */

    /* Start of locally defined data */
    struct {    /* Some of the fields converted to internal format. */
        unsigned int t;     /* type="xs:unsignedInt" */
        unsigned int d;     /* type="xs:unsignedInt" */
        unsigned int r;     /* type="xs:unsignedInt" */
    } cooked;


    /* If this SegmentTimelineElem has a repeat count, then the
     * following fields will pertain to the first segment. */
    bool            parsed;     /* false => the following fields are not yet valid. */
    uint64_t        segmentTimeInTimescaleUnits; /* segment's starting Period time       */
    unsigned long   segmentEptInTimescaleUnits;  /* segment's earliest_presentation_time */
    unsigned long   segmentNumber;

} MpegDashSegmentTimelineElemInfo;
#define MPEG_DASH_SEGMENTTIMELINEELEM_TO_PRINTF_ARGS(pObj)      \
            "SegmentTimelineElem:: t:%s (%lu)  d:%s (%lu)  r:%s (%lu) parsed: %lu segTimeInTsUnits: %" PRIu64 " segEptInTsUnits: %lu, segNbr: %lu",   \
            (pObj)->t, (pObj)->cooked.t,                     \
            (pObj)->d, (pObj)->cooked.d,                     \
            (pObj)->r, (pObj)->cooked.r,                     \
            (pObj)->parsed,                                  \
            (pObj)->segmentTimeInTimescaleUnits,             \
            (pObj)->segmentEptInTimescaleUnits,              \
            (pObj)->segmentNumber


typedef struct MpegDashSegmentTimelineInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from SegmentTimelineType of the MPEG-DASH XML schema */

        BLST_Q_HEAD(mpegDashSegmentTimelineElemInfoHead,       MpegDashSegmentTimelineElemInfo)          mpegDashSegmentTimelineElemInfoHead;

    /* End of data from SegmentTimelineType of the MPEG-DASH XML schema */

} MpegDashSegmentTimelineInfo;



typedef struct MpegDashMultipleSegmentBaseInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from MultipleSegmentBaseType of the MPEG-DASH XML schema */

        MpegDashSegmentBaseInfo     * baseTypeSegmentBase;  /* Include base class */

        MpegDashSegmentTimelineInfo * segmentTimeline;
        MpegDashUrlInfo             * bitstreamSwitching;

        const char *  duration;     /*  type="xs:unsignedInt" (but leave as char * so we can tell if it was there or not) */
        const char *  startNumber;  /*  type="xs:unsignedInt" (but leave as char * so we can tell if it was there or not) */

    /* End of data from MultipleSegmentBaseType of the MPEG-DASH XML schema */

} MpegDashMultipleSegmentBaseInfo;


typedef struct MpegDashSegmentListInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from SegmentListType of the MPEG-DASH XML schema */
        MpegDashMultipleSegmentBaseInfo  *baseTypeMultipleSegmentBase;  /* Include base class */

        BLST_Q_HEAD(mpegDashSegListSegmentUrlInfoHead,       MpegDashSegmentUrlInfo)          mpegDashSegmentUrlInfoHead;

        const char * href;
        const char * actuate;
    /* End of data from SegmentListType of the MPEG-DASH XML schema */
} MpegDashSegmentListInfo;



typedef struct MpegDashSegmentTemplateInfo {
    B_PlaybackIp_XmlElement   myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from SegmentTemplateType of the MPEG-DASH XML schema */
        MpegDashMultipleSegmentBaseInfo  *baseTypeMultipleSegmentBase;  /* Include base class */

        const char * media;              /* type="xs:string" */
        const char * index;              /* type="xs:string" */
        const char * initialization;     /* type="xs:string" */
        const char * bitstreamSwitching; /* type="xs:string" */

    /* End of data from SegmentListType of the MPEG-DASH XML schema */
} MpegDashSegmentTemplateInfo;


/* Information from the Representation element. */
typedef struct MpegDashRepresentationInfo {
    B_PlaybackIp_XmlElement              myXmlElem;  /* pointer to the MXmlElement corresponding to this struct */
    struct MpegDashAdaptationSetInfo   * parentObj;

    /* Start of data from RepresentationType of the MPEG-DASH XML schema */
        MpegDashRepresentationBaseInfo   * baseTypeRepresentationBase;  /* Include base class */

        BLST_Q_HEAD(mpegDashRepBaseUrlInfoHead,         MpegDashBaseUrlInfo)            mpegDashBaseUrlInfoHead;
    /*  BLST_Q_HEAD(mpegDashSubRepresentationInfoHead,  MpegDashSubRepresentationInfo)  mpegDashSubRepresentationInfoHead; */

        MpegDashSegmentBaseInfo        * segmentBase;
        MpegDashSegmentListInfo        * segmentList;
        MpegDashSegmentTemplateInfo    * segmentTemplate;

        const char *      id;                       /* type="StringNoWhitespaceType" use="required" */
        const char *      bandwidth;                /* type="xs:unsignedInt" use="required"         */
        unsigned int      bandwidthNumeric;         /* bandwidth converted to unsigned int          */
        unsigned int      qualityRanking;           /* type="xs:unsignedInt"                        */
        const char *      dependencyId;             /* type="StringVectorType"                      */
        const char *      mediaStreamStructureId;   /* type="StringVectorType"                      */

    /* End of data from RepresentationType of the MPEG-DASH XML schema */

    const char *pBaseUrl;
    BLST_Q_ENTRY(MpegDashRepresentationInfo) nextRepresentationInfo;    /* used for list of RepresentationInfo */

    bool                    probeInfoIsValid;
    B_PlaybackIpPsiInfo     * psi;

    unsigned char          * codecSpecificAddrForAudio;    /* BKNI_Malloc'd codec-specific data */
    size_t                   codecSpecificSizeForAudio;    /* Size of codecSpecific data for audio */
    uint32_t                 fourccForAudio;               /* Formatted as with BMEDIA_FOURCC() */
    unsigned long            trackIdForAudio;

    /* GARYWASHERE: Combine these audio/video properties into a structure */
    unsigned long         mdhdTimescaleForAudio;
    unsigned long         trexDefaultSampleDescriptionIndexForAudio;
    unsigned long         trexDefaultSampleDurationForAudio;
    unsigned long         trexDefaultSampleFlagsForAudio;
    unsigned long         trexDefaultSampleSizeForAudio;

    unsigned char          * codecSpecificAddrForVideo;    /* BKNI_Malloc'd codec-specific data */
    size_t                   codecSpecificSizeForVideo;    /* Size of codecSpecific data for video */
    uint32_t                 fourccForVideo;               /* Formatted as with BMEDIA_FOURCC() */
    unsigned long            trackIdForVideo;

    unsigned long         mdhdTimescaleForVideo;
    unsigned long         trexDefaultSampleDescriptionIndexForVideo;
    unsigned long         trexDefaultSampleDurationForVideo;
    unsigned long         trexDefaultSampleFlagsForVideo;
    unsigned long         trexDefaultSampleSizeForVideo;

    /* Some fields inherited from the SegmentBase. */
    unsigned int inheritedTimescale;    /* timescale value inherited from MultipleSegmentBase (from SegmentTemplate or SegmentList)   */
    unsigned int inheritedPresentationTimeOffset;
    unsigned int inheritedIndexRangeExact;

    /* Some fields inherited from the MultipleSegmentBase. */
    unsigned int inheritedStartNumber;  /* startNumber value inherited from MultipleSegmentBase (from SegmentTemplate or SegmentList)   */
    unsigned int inheritedDuration;     /* duration value inherited from MultipleSegmentBase (from SegmentTemplate or SegmentList)   */


    /************ Things below this line will probably go away. ****************/


    BLST_Q_ENTRY(MpegDashRepresentationInfo) next;    /* next playlist file */ /* GARYWASHERE: This needs to go away!!! */

    MediaFileSegmentInfo *currentMediaFileSegment; /* pointer to the current segment being downloaded */
    struct MediaFileSegmentInfoQueue mediaFileSegmentInfoQueueHead; /* queue of media segments belonging to this playlist file */
    short version; /* number (1 by default, 2 w/ AES) */
    short programId; /* number that uniquely identifies a particular presentation within the scope of a playlist file */
    NEXUS_PlaybackPosition totalDuration; /* playlist duration in milli-seconds, sum of durations of media files within a playlist */
    NEXUS_PlaybackPosition maxMediaSegmentDuration; /* max duration of any media stream segment: in msec */
    int mediaSegmentBaseSequence; /* base sequence number of the media sequence in the current playlist */
    bool allowCaching; /* if set, media stream can be cached (i.e. recorded) */
    int initialMinimumReloadDelay; /* duration of the last media file in playlist: used the interval for re-downloading the unbounded playlist */
    /*
       -codecs: in RFC4281 format
       -resolution:HxV
    */
    /* state needed during playback */
    bool bounded; /* (set if end tag is present) */
    int lastUsedUriIndex; /* index of the last URI being downloaded */
    int numMediaSegments; /* number of media segments in a playlist */
    bool representationHasChanged; /* set when playlist re-download has refreshed the playlist */
} MpegDashRepresentationInfo;

#define MPEG_DASH_REPRESENTATION_TO_PRINTF_ARGS(pObj)                                        \
            "RepresentationType: id:%s bandwidth:%s (%u) qualityRanking:%u "                 \
            "dependencyId:%s mediaStreamStructureId:%s",                                     \
            (pObj)->id,                                                                        \
            (pObj)->bandwidth,                                                                 \
            (pObj)->bandwidthNumeric,                                                          \
            (pObj)->qualityRanking,                                                            \
            (pObj)->dependencyId,                                                              \
            (pObj)->mediaStreamStructureId


typedef struct MpegDashContentComponentInfo {
    B_PlaybackIp_XmlElement       myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from ContentComponentType of the MPEG-DASH XML schema */

    /*  BLST_Q_HEAD(mpegDashAccessibilityInfoHead,    MpegDashAccessibilityInfo)    mpegDashAccessibilityInfoHead;  */
    /*  BLST_Q_HEAD(mpegDashRoleInfoHead,             MpegDashRoleInfo)             mpegDashRoleInfoHead;           */
    /*  BLST_Q_HEAD(mpegDashRatingInfoHead,           MpegDashRatingInfo)           mpegDashRatingInfoHead;         */
    /*  BLST_Q_HEAD(mpegDashViewpointInfoHead,        MpegDashViewpointInfo)        mpegDashViewpointInfoHead;      */

        const char * id;
        const char * lang;
        const char * contentType;
        const char * par;

        BLST_Q_ENTRY(MpegDashContentComponentInfo) nextContentComponentInfo;    /* used for list of ContentComponentInfo */
    /* End of data from ContentComponentType of the MPEG-DASH XML schema */

} MpegDashContentComponentInfo;

#define MPEG_DASH_CONTENTCOMPONENT_TO_PRINTF_ARGS(pObj)                       \
            "ContentComponentType: id:%s lang::%s contentType:%s par:%s",     \
            (pObj)->id,                                                         \
            (pObj)->lang,                                                       \
            (pObj)->contentType,                                                \
            (pObj)->par



typedef struct MpegDashAdaptationSetInfo {
    B_PlaybackIp_XmlElement       myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */
    struct MpegDashPeriodInfo   * parentObj;

    /* Start of data from AdaptationSetType of the MPEG-DASH XML schema */

        MpegDashRepresentationBaseInfo  * baseTypeRepresentationBase;  /* Include base class */

    /*  BLST_Q_HEAD(mpegDashAccessibilityInfoHead,    MpegDashAccessibilityInfo)    mpegDashAccessibilityInfoHead;  */
    /*  BLST_Q_HEAD(mpegDashRoleInfoHead,             MpegDashRoleInfo)             mpegDashRoleInfoHead;           */
    /*  BLST_Q_HEAD(mpegDashRatingInfoHead,           MpegDashRatingInfo)           mpegDashRatingInfoHead;         */
    /*  BLST_Q_HEAD(mpegDashViewpointInfoHead,        MpegDashViewpointInfo)        mpegDashViewpointInfoHead;      */
        BLST_Q_HEAD(mpegDashContentComponentInfoHead, MpegDashContentComponentInfo) mpegDashContentComponentInfoHead;
        BLST_Q_HEAD(mpegDashBaseUrlInfoHead,          MpegDashBaseUrlInfo)          mpegDashBaseUrlInfoHead;

        MpegDashSegmentBaseInfo        * segmentBase;
        MpegDashSegmentListInfo        * segmentList;
        MpegDashSegmentTemplateInfo    * segmentTemplate;

        BLST_Q_HEAD(mpegDashRepresentationInfoHead,   MpegDashRepresentationInfo)   mpegDashRepresentationInfoHead;

        const char * href;
        const char * actuate;
        const char * id;
        unsigned int group;
        const char * lang;
        const char * contentType;
        const char * par;
        unsigned int minBandwidth;
        unsigned int maxBandwidth;
        unsigned int minWidth;
        unsigned int maxWidth;
        unsigned int minHeight;
        unsigned int maxHeight;
        const char * minFrameRate;
        const char * maxFrameRate;
        unsigned int segmentAlignment;      /* boolean */
        unsigned int subsegmentAlignment;   /* boolean */
        unsigned int subsegmentStartsWithSAP;
        unsigned int bitstreamSwitching;    /* boolean */

        BLST_Q_ENTRY(MpegDashAdaptationSetInfo) nextAdaptationSetInfo;    /* used for list of AdaptationSetInfo */
    /* End of data from AdaptationSetType of the MPEG-DASH XML schema */

    bool hasVideo;
    bool hasAudio;

    MpegDashRepresentationInfo   *currentRepresentation;

} MpegDashAdaptationSetInfo;
#define MPEG_DASH_ADAPTATIONSET_TO_PRINTF_ARGS(pObj)                                         \
            "AdaptationSetType: href:%s actuate:%s id:%s group:%u "                \
            "lang:%s contentType:%s par:%s "                                                 \
            "minBW:%u maxBW:%u minWidth:%u maxWidth:%u "                                     \
            "minHeight:%u maxHeight:%u minFrameRate:%s maxFrameRate:%s "                     \
            "segAlign:%u subSegAlign:%u subSegStartsWithSAP:%u bsSwitching:%u "              \
            "hasAudio:%u hasVideo:%u",                                                       \
            (pObj)->href,                                                                      \
            (pObj)->actuate,                                                                   \
            (pObj)->id,                                                                        \
            (pObj)->group,                                                                     \
            (pObj)->lang,                                                                      \
            (pObj)->contentType,                                                               \
            (pObj)->par,                                                                       \
            (pObj)->minBandwidth,                                                              \
            (pObj)->maxBandwidth,                                                              \
            (pObj)->minWidth,                                                                  \
            (pObj)->maxWidth,                                                                  \
            (pObj)->minHeight,                                                                 \
            (pObj)->maxHeight,                                                                 \
            (pObj)->minFrameRate,                                                              \
            (pObj)->maxFrameRate,                                                              \
            (pObj)->segmentAlignment,                                                          \
            (pObj)->subsegmentAlignment,                                                       \
            (pObj)->subsegmentStartsWithSAP,                                                   \
            (pObj)->bitstreamSwitching,                                                        \
            (pObj)->hasAudio,                                                                  \
            (pObj)->hasVideo


typedef struct MpegDashPeriodInfo {
    B_PlaybackIp_XmlElement    myXmlElem;          /* pointer to the MXmlElement corresponding to this struct */
    struct MpegDashMpdInfo   * parentObj;

    /* Start of data from PeriodType of the MPEG-DASH XML schema */
        BLST_Q_HEAD(mpegDashPeriodBaseUrlInfoHead,   MpegDashBaseUrlInfo)         mpegDashBaseUrlInfoHead;
        MpegDashSegmentBaseInfo        * segmentBase;
        MpegDashSegmentListInfo        * segmentList;
        MpegDashSegmentTemplateInfo    * segmentTemplate;
        BLST_Q_HEAD(mpegDashAdaptationSetInfoHead,   MpegDashAdaptationSetInfo)   mpegDashAdaptationSetInfoHead;
    /*  BLST_Q_HEAD(mpegDashSubsetInfoHead,          MpegDashSubsetInfo)          mpegDashSubsetInfoHead;       */

        const char * href;
        const char * actuate;
        const char * id;
        const char * start;
        const char * duration;
        const char * bitstreamSwitching;

        BLST_Q_ENTRY(MpegDashPeriodInfo) nextPeriodInfo;    /* used for list of PeriodInfo */
    /* End of data from PeriodType of the MPEG-DASH XML schema */

    /* Start of locally defined data */
    struct {
        MpegDashDuration  start;
        MpegDashDuration  duration;
    } cooked;

    uint64_t    periodStartInMs;    /* Period start time relative to start of the MPD's time. */
    uint64_t    periodDurationInMs; /* Period duration in milliseconds. */

    MpegDashAdaptationSetInfo   *currentVideoAdaptationSet;
    MpegDashAdaptationSetInfo   *currentAudioAdaptationSet;

    /* GARYWASHERE: will need to support separate Representations for audio and video. */
    B_PlaybackIp_XmlElement   xmlElemRepresentation;   /* pointer to the active Representation XML element */

} MpegDashPeriodInfo;
#define MPEG_DASH_PERIOD_TO_PRINTF_ARGS(pObj)                                                     \
            "PeriodType: href:%s actuate:%s id:%s start:%s duration:%s bsSwitching:%s", \
            (pObj)->href,                                                                           \
            (pObj)->actuate,                                                                        \
            (pObj)->id,                                                                             \
            (pObj)->start,                                                                          \
            (pObj)->duration,                                                                       \
            (pObj)->bitstreamSwitching

/* Information from the MPD element. */
typedef struct MpegDashMpdInfo {
    B_PlaybackIp_XmlElement   myXmlElem;     /* pointer to the MXmlElement corresponding to this struct */

    /* Start of data from MPDType of the MPEG-DASH XML schema */
    /*  BLST_Q_HEAD(mpegDashProgramInformationInfoHead, MpegDashProgramInformationInfo) mpegDashProgramInformationInfoHead; */
        BLST_Q_HEAD(mpegDashMpdBaseUrlInfoHead,  MpegDashBaseUrlInfo)  mpegDashBaseUrlInfoHead;
        BLST_Q_HEAD(mpegDashLocationHead,        MpegDashAnyTextType)  mpegDashLocationInfoHead;
        BLST_Q_HEAD(mpegDashPeriodInfoHead,      MpegDashPeriodInfo)   mpegDashPeriodInfoHead;
    /*  BLST_Q_HEAD(mpegDashMetricsInfoHead,     MpegDashMetricsInfo)  mpegDashMetricsInfoHead; */

        const char * id;
        const char * profiles;
        const char * type; /* "static" or "dynamic" */
        const char * availabilityStartTime;
        const char * availabilityEndTime;
        const char * mediaPresentationDuration;
        const char * minimumUpdatePeriod;
        const char * minBufferTime;
        const char * timeShiftBufferDepth;
        const char * suggestedPresentationDelay;
        const char * maxSegmentDuration;
        const char * maxSubsegmentDuration;
    /* End of data from MPDType of the MPEG-DASH XML schema */

    /* Start of locally defined data */
    struct {
        MpegDashDateTime  availabilityStartTime;
        MpegDashDateTime  availabilityEndTime;

        MpegDashDuration  mediaPresentationDuration;
        MpegDashDuration  minimumUpdatePeriod;
        MpegDashDuration  minBufferTime;
        MpegDashDuration  timeShiftBufferDepth;
        MpegDashDuration  suggestedPresentationDelay;
        MpegDashDuration  maxSegmentDuration;
        MpegDashDuration  maxSubsegmentDuration;
    } cooked;

    MpegDashDateTime     fetchTime;  /* Time that MPD was downloaded. */
    MpegDashDateTime     checkTime;  /* Time to check for updated MPD. */
    bool                            isDynamic;  /* true=> MpegDashMpdInfo.type == "dynamic" */

    const char              * pBaseUrl;
    MpegDashPeriodInfo      * currentPeriod;
    MpegDashBaseUrlInfo     * currentBaseUrl;

} MpegDashMpdInfo;

#define MPEG_DASH_MPD_TO_PRINTF_ARGS(pObj)                                 \
            "MpdType: id:%s profiles:%s type:%s StartTime:%s "   \
            "EndTime:%s Duration:%s UpdatePeriod:%s minBufferTime:%s",     \
            (pObj)->id,                                                      \
            (pObj)->profiles,                                                \
            (pObj)->type,                                                    \
            (pObj)->availabilityStartTime,                                   \
            (pObj)->availabilityEndTime,                                     \
            (pObj)->mediaPresentationDuration,                               \
            (pObj)->minimumUpdatePeriod,                                     \
            (pObj)->minBufferTime




typedef enum B_PlaybackMpegDashSeekStatus {
    B_PlaybackMpegDashSeekStatus_eNotInUse,     /* This RepresentationSeekContext is not being used (like when audio+video use same representation. */
    B_PlaybackMpegDashSeekStatus_eInvalid,
    B_PlaybackMpegDashSeekStatus_eValid,
    B_PlaybackMpegDashSeekStatus_eDownloaded
} B_PlaybackMpegDashSeekStatus;



typedef struct MpegDashRepresentationSeekContext {

    B_PlaybackMpegDashSeekStatus  status;

    uint64_t                      periodTimeInTimescaleUnits;   /* Number of timescale units since start of Period. */
    unsigned long                 durationInTimescaleUnits;     /* Segment duration in timescale units.  */
    unsigned long                 timescale;                    /* Number of timescale units per second. */

    uint64_t                      periodTimeInMs;   /* time relative to start of Period (in milliseconds). Not accurate due to rounding errors.  */
    bool                          isAtEof;          /* true => last seek tried to go past end of Representation. */

    MpegDashRepresentationInfo  * representationInfo;
    const char                  * representationId;
    unsigned                      byteOffset;           /* Byte offset within segment. */
    unsigned long                 segmentTimelineTime;  /* "t" field of SegmentTimeline element list. */

    /* Fields used for "SegmentBase" type of Representations. */
    MpegDashSegmentBaseInfo     * segmentBase;

    /* Fields used for "SegmentList" type of Representations. */
    MpegDashSegmentListInfo     * segmentList;
    MpegDashSegmentUrlInfo      * segmentUrlInfo;  /* For segmentList based segments. */

    /* Fields used for "SegmentTemplate" type of Representations. */
    MpegDashSegmentTemplateInfo * segmentTemplate;
    unsigned                      segmentNumber;    /* For template-based segments. */


} MpegDashRepresentationSeekContext;

#define MPEG_DASH_REPRSEEK_TO_PRINTF_ARGS(pObj)                                 \
            "ReprSeekCtx: PerTimeInTs:%" PRIu64 " DurInTs:%lu Ts:%lu PerTimeInMs:%" PRIu64 " Eof:%u ReprID:%s Repr:%p segmentBase:%p segmentList:%p segmentTemplate:%p",   \
            (pObj)->periodTimeInTimescaleUnits,                              \
            (pObj)->durationInTimescaleUnits,                                \
            (pObj)->timescale,                                               \
            (pObj)->periodTimeInMs,                                          \
            (pObj)->isAtEof,                                                 \
            (pObj)->representationId,                                        \
            (pObj)->representationInfo,                                      \
            (pObj)->segmentBase,                                             \
            (pObj)->segmentList,                                             \
            (pObj)->segmentTemplate

typedef struct MpegDashSeekContext {
    uint64_t    mpdSeekTimeInMs;        /* time relative to start of MPD (either MPD.availabilityStartTime or zero). */
    uint64_t    periodSeekTimeInMs;     /* time relative to start of current Period. */

    /* When a stream has audio and video in separate Representations, the following
       currentRepresentation will indicate which one the seek is at.*/
    MpegDashRepresentationSeekContext   * currentRepresentationSeekContext;

    MpegDashRepresentationSeekContext   video;
    MpegDashRepresentationSeekContext   audio;
} MpegDashSeekContext;



typedef struct MpegDashSlavePidEntry {
    BLST_Q_ENTRY(MpegDashSlavePidEntry) nextSlavePidEntry;
    NEXUS_PidChannelHandle      pidChannelHandle;
    uint16_t                    pid;
} MpegDashSlavePidEntry;


typedef struct MpegDashSessionState {
    BLST_Q_HEAD(mpegDashSessionRepresentationInfoHead, MpegDashRepresentationInfo) mpegDashRepresentationInfoHead;
    MpegDashRepresentationInfo *currentRepresentation;
    char *mpdParseBuffer;
    int mpdParseBufferSize;
    int mpdParseBufferDepth;
    B_ThreadHandle mpegDashSegmentDownloadThread;
    bool mpegDashSegmentDownloadThreadDone; /* set when download thread either gets an error or is done downloading all media segments in a playlist */
    bool mpegDashPlaybackThreadDone;
    B_ThreadHandle mpegDashMpdReDownloadThread;
    bool mpegDashMpdReDownloadThreadDone;
    MpegDashSeekContext requestSeekCtx;
    MpegDashDownloadContext downloadContext;
    MpegDashSegmentBuffer segmentBuffer[MPEG_DASH_NUM_SEGMENT_BUFFERS];
    BKNI_EventHandle playbackThreadWakeEvent;
    BKNI_EventHandle downloadThreadWakeEvent;
    BKNI_EventHandle reDownloadMpdEvent;
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to this structure by Nexus IO, App, Http threads */
    int networkBandwidth; /* network b/w that is achieved while downloading a media segment */
    int mpdSocketFd;   /* socket file descriptor of the playlist file */
    bool downloadedAllSegmentsInCurrentRepresentation; /* set when all media segments in the current playlist have been downloaded: used to detect when to switch playlists in case base seq # wraps around */
    bool resetRepresentation; /* set to reset the playlist so that we start download from the 1st segment of the playlist */
    unsigned maxNetworkBandwidth; /* max network bandwidth set via max_nw_bw */

    uint16_t    audioMasterPid;
    uint16_t    videoMasterPid;

    NEXUS_PlaypumpHandle *pAudioPlaypump;
    NEXUS_PlaypumpHandle *pVideoPlaypump;

    NEXUS_TransportType  mpegType;

    BLST_Q_HEAD(mpegDashSlavePidHead,    MpegDashSlavePidEntry) audioSlavePidHead, videoSlavePidHead;

    MpegDashBandwidthContext     bandwidthContext;   /* Data structures for bandwidth averaging. */

    BKNI_EventHandle segDownloadThreadPausedEvent;
    BKNI_EventHandle playbackThreadPausedEvent;
    BKNI_EventHandle segDownloadThreadPauseDoneEvent;
    BKNI_EventHandle playbackThreadPauseDoneEvent;

    batom_factory_t factory;

    /* Attributes of the initial MPD file URL. */
    char *absoluteUri; /* complete URI string of the playlist file */
    char *uri;  /* file name part of the playlist file uri */
    char *server; /* pointer to the playlist server name uri */
    unsigned port; /* port number of the playlist server uri */
    B_PlaybackIpProtocol protocol;

    B_PlaybackIp_XmlElement       xmlElemRoot;   /* pointer to the root XML element */
    MpegDashMpdInfo             * mpdInfo;

} MpegDashSessionState;

#endif

typedef struct brtp_channel *brtp_channel_t;

typedef struct B_PlaybackIpPsiState *B_PlaybackIpPsiStateHandle;
typedef struct B_PlaybackIp
{
    B_PlaybackIpSettings settings;
    B_PlaybackIpSessionOpenSettings openSettings;
    B_PlaybackIpSessionOpenStatus openStatus;
    B_PlaybackIpError sessionOpenStatus;
    B_PlaybackIpSessionSetupSettings setupSettings;
    B_PlaybackIpSessionSetupStatus setupStatus;
    bool sessionSetupCompleted; /* set when session setup is successfully completed */
    B_PlaybackIpSessionStartSettings startSettings;
    B_PlaybackIpSessionStartStatus startStatus;
    char *responseMessage; /* cached response message string */
    char *requestMessage; /* buffer to build the outgoing request message */
    B_PlaybackIpSocketState socketState;
    B_PlaybackIpPsiInfo psi;
    NEXUS_TransportType http_content_type;
    bool apiInProgress; /* if set, an API call is in progress, cleared when api is completed */
    bool apiCompleted; /* if set, an API call is completed and results can be returned in next API call */
    BKNI_EventHandle liveMediaSyncEvent; /* event to wait for completion of a command submitted to live media library */
    BKNI_EventHandle playback_halt_event;
    BKNI_EventHandle preChargeBufferEvent;
    BKNI_EventHandle newJobEvent;  /* event to indicate new tasks to the worker thread (e.g. used to wake up http thread to do either buffering or trick play operations for RVU) */
    BKNI_EventHandle read_callback_event;
    B_PlaybackIpState playback_state;
#ifndef DMS_CROSS_PLATFORMS
    B_PlaybackIpNexusHandles nexusHandles;
    NEXUS_PlaypumpSettings pumpparams;
#endif /* DMS_CROSS_PLATFORMS */
    NEXUS_FilePlayHandle nexusFileHandle;
    bfile_io_read_t bufferedFdIndex;
    /* TODO: is this needed bplayback_ip_buffer_scheme  ip_buffer_scheme;*/
#if 0
    bstream_t                   stream;
    b_timer_t                   pwm_slew_timer;
    long                        current_pwm_value;
    unsigned int                num_pwm_steps_pending;
    bool                        forcing_pwm_slow;
    bool                        buffer_fill_period_expired;
    bool                        ip_recording;
#endif
    B_PlaybackIpProtocol protocol;
    unsigned int payload_size;
    B_PlaybackIp_TtsThrottleHandle ttsThrottle;
    unsigned numRecvTimeouts;                   /* count of the number of receive timeouts since start of IP playback */
#ifdef LIVEMEDIA_SUPPORT
    void *lm_context;
#endif
    B_ThreadHandle playbackIpThread;
    B_ThreadHandle sessionSetupThread;
    B_ThreadHandle sessionOpenThread;
#if TODO
    B_PlaybackIpHandlerickmode_params trickmode_param;
    bdecode_status decode_status;
#endif
#ifdef EROUTER_SUPPORT
    int bcmrgDrvFd;
#endif
    char temp_buf[TMP_BUF_SIZE+1];  /* extra 1 byte to allow '\0' to be placed at end for string operations */
    int tempBytesToRead;    /* used for handling timeout cases during chunk header processing */
    int tempBytesRead;
    char *discard_buf;
    void *buffer;
    size_t buffer_size;
    struct B_PlaybackIpItem     *item;
    uint8_t *item_mem;
    struct B_PlaybackIpItemQueue active_item; /* items pending in filter */
    struct B_PlaybackIpItemQueue free_item; /* items ready for use */
    unsigned long byte_count;
    batom_factory_t factory;
    batom_pipe_t pipe_out;  /* pipe data out from playback to filter */
    batom_pipe_t pipe_in;   /* pipe data into playback from filter */
#ifdef SPF_SUPPORT
    brtp_spf_t rtp;         /* the RTP filter */
#else
    brtp_t rtp;         /* the RTP filter */
#endif /* SPF_SUPPORT */
    bool socket_flush;  /* Set by the controller to request socket evacuate */
    int initial_data_len;
    unsigned int firstPts;  /* Cached First PTS value sent by the server */
    bool firstPtsPassed;  /* Cached First PTS value sent by the server */
    unsigned int streamDurationUntilLastDiscontinuity; /* cumulative stream duration until the last PTS discontinuity */
    unsigned int lastUsedPts;   /* PTS used during last trickplay operation */
    unsigned int ptsDuringLastDiscontinuity;    /* pts of the displayed frame before the last dicontinuity */
    unsigned int firstPtsBeforePrevDiscontinuity;   /* first pts of the contigous block before the previous discontinuity */
    bool getNewPtsAfterDiscontinuity;   /* set during the ptsErrorCallback to indicate that we need to get new currentPts when next get position is called */
    unsigned int speed;     /* new speed value passed by the application */
    off_t contentLength;      /* length of the content being played via HTTP */
    B_Time start_time;
    B_PlaybackIp_RtpHeaderData *rtp_hdr_data;
    struct bfile_in_net *file;
    struct bfile_out_net *fileOut;
    bfile_buffered_t fileBuffered;
    bool useNexusPlaypump;  /* set if app has indicated to use Nexus Playpump to feed IP session data (instead of default Nexus Playback) */
    char *indexCache;
    volatile unsigned int indexCacheDepth;
    volatile unsigned int indexCacheSize;
    off_t indexCacheStartOffset; /* linear offset of currnet first byte in index cache */
    off_t indexCacheEndOffset;   /* linear offset of currnet last byte in index cache */
    unsigned int indexCacheWriteIndex;  /* byte offset in index cache where next byte (from n/w) can be written into */
    unsigned int indexCacheReadIndex;  /* byte offset in index cache where indexCacheStartOffset starts */
    int indexCacheInit;  /* set after data cache is initialized */
    int indexSeekCount;  /* # of times seek has been called for index */
    bool indexAtEnd; /* true if index towards the end of the file */
    B_PlaybackIpAvCache dataCache[HTTP_MAX_DATA_CACHES];   /* we maintain two data caches for serving two different offset ranges (when AV data can be far apart) */
    int lastUsedCacheIndex; /* index of the cache that was used in the last read callback from Nexus */
    int cacheIndexUsingSocket; /* index of the cache that last read from the network socket */
    off_t lastSeekOffset;    /* offset into file that was used in the last seek callback from playback io threads */
    bool preChargeBuffer;   /* if set, HTTP thread starts pre-charging n/w buffer (data cache) by reading data from socket */
    int networkTimeout; /* timeout for network calls like select, read, etc. */
    bool selectTimeout; /* true when select call times out */
    bool rewind; /* set when file has been rewind */
    int *bytesRecvPerPkt; /* array of values representing the number of bytes received in each packet */
    bool chunkEncoding; /* if true, server is sending content using HTTP Chunk Transfer Encoding */
    bool adjustBytesToRead; /* if true, adjust the bytesToRead variable if DTCP PCP header is found */
    off_t chunkSize; /* current chunk size */
    off_t bytesLeftInCurChunk; /* current chunk size */
    off_t totalRead; /* total bytes Read */
    off_t totalConsumed; /* total bytes consumed by the Media player */
    char *chunkPlayloadStartPtr;  /* points to the start of payload read along w/ the next chunk header */
    int chunkPayloadLength;  /* length of the payload read along with the next chunk header */
    int num_chunks_received;  /* how many chunks have been completely received */
    bool serverClosed; /* true if server has closed the connection */
    bool reOpenSocket; /* true if current socket needs to be closed & reopened due to server closed or cache change case */
    bool reOpenIndexSocket; /* true if current socket needs to be reopened due to socket being owned for data */
    B_PlaybackIpNetIo netIo; /* interface for network IO, implemented by each security module */
    void *securityHandle;   /* opaque handle of the security module for the currently active connection */
    bool printedOnce;   /* used to throttle the error message prints */
    bool readCallBackInProgress;   /* set when nexus io thread has invoked the data read callback */
    int cacheDepthFudgeFactor;   /* 0 during initial buffering, set during runtime buffering to HTTP_CACHE_DEPTH_FUDGE_FACTOR */
    unsigned int byteRangeOffset; /* byte range offset calculated in security session open. miliseconds */
    int statusCode;         /* http status code */
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    bool streamingFormat;   /* set for TS, MP3, VoB, ES formats, false for MP4 & ASF (where index is present & are always cached) */
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_PlaybackPosition pausedPosition; /* position in msec where playback is currently paused */
    NEXUS_PlaybackPosition seekPosition; /* position in msec where playback needs to seek to */
    bool useSeekPosition; /* use seekPosition variable instead of current stream position while resuming the play */
    NEXUS_PlaybackPosition lastSeekPosition; /* position in msec where playback needs to start either after seek or trickmode */
    bool lastSeekPositionSet; /* true when a seek operation is carried out but firstPts callback hasn't yet come */
#endif /* DMS_CROSS_PLATFORMS */
    off_t pausedByteOffset;    /* paused position in bytes from where to begin playback upon play */
    bool useNewOffset;  /* force next NetRangeReq call to use the pausedByteOffset even if it is 0 */
    bool socketClosed; /* set when socket has been closed: mainly used during pause using disconnect/seek method */
    bool firstDataReadCallbackDone; /* set when 1st read callback is done */
    bool waveHeaderInserted; /* set when wave header is inserted */
    bool dtcpPcpHeaderFound; /* set when DTCP/IP wrapper layer lowers the chunksize to account for PCP header */
    B_PlaybackIpProtocol mediaTransportProtocol; /* applicable for RTSP sessions only: transport protocol for carrying AV */
    bool disableLiveMode; /* if set, live mode is disabled (i.e. dont timeout select call on a live ip socket) */
    bool sendNextTimeSeekReq; /* set during RVU trick play we need to send the next time seek req to server */
    double initialTimeSeekRangeBegin; /* set upon each trick play command */
    double timeSeekRangeInterval; /* interval for the time seek range request */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to this structure by Nexus IO, App, Http threads */
    B_Time mediaStartTime;
    bool mediaStartTimeNoted;
    bool mediaEndTimeNoted;
    NEXUS_PlaybackPosition lastPosition; /* playback position when it was last queried. Also, updated to current position when media paused or seeked */
    const bmedia_probe_stream *stream;
    bmedia_probe_t probe;
    bool hlsSessionEnabled; /* set if HTTP Live Streaming (HLS) is being used for the current session */
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    HlsSessionState *hlsSessionState;
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    bool mpegDashSessionEnabled; /* set if MPEG-DASH is being used for the current session */
    MpegDashSessionState *mpegDashSessionState;
#endif
    unsigned int prevPlayed;
    bool hwStcTrickMode;
    bool simulatedStcTrickMode;
    unsigned int stcRate;
    bool mediaProbeStartTimeNoted;
    B_Time mediaProbeStartTime; /* time when media probe is started */
    B_PlaybackIpAppHeader appHeader;
    bool ipTunerLockedEventSent; /* set when lock event is sent */
    bool ipTunerUnLockedEventSent; /* set when unlock event is sent */
    bool ipVerboseLog; /* if set, detailed logging is enabled on console log */
    bool enableRecording; /* if set, downloaded media stream is captured into a test file */
    NEXUS_VideoFrameRate frameRate; /* nexus video frame rate */
    char *cookieFoundViaHttpRedirect; /* caches pointer to the cookie header found in the HTTP redirect response */
    bool netRangeFunctionInvoked; /* set if media probe was carried out for this session */
    int readTimeout; /* read returns either the asked amount of data or whatever is available in these msec */
    FILE *fclear;
    B_ThreadHandle trickModeThread;
    BKNI_EventHandle newTrickModeJobEvent;
    B_PlaybackIpError trickModeStatus;
    bool trickModeThreadDone;
    brtp_channel_t channels;
    void *lm;
    void *lmSession;
    char lmStop;
    bool isTtsStream;
    unsigned rtpHeaderLength;
    NEXUS_CallbackDesc appSourceChanged;
    NEXUS_CallbackDesc appFirstPts;
    NEXUS_CallbackDesc streamStatusAvailable;
    NEXUS_CallbackDesc appFirstPtsPassed;
    NEXUS_CallbackDesc appPtsError;
    char encryptedBuf[HTTP_AES_BLOCK_SIZE]; /* buffer to cache the encrypted bytes from the dtcp lib that couldn't be decrypted as they were followed by HTTP Chunk header that had to be removed */
    int encryptedBufLength; /* length of encrypted bytes */
    bool quickMediaProbe; /* if set, quick media  probe is carried out: for HLS, only first few Ts packets are examined for PSI info  */
    int speedNumerator;
    int speedDenominator;
    unsigned int lastPlayed; /* contains the count of frames played until the last play position sampling interval */
    unsigned int originalFirstPts; /* very first PTS that never gets overwritten (even during stream PTS discontinuities). */
    unsigned int lastPtsExtrapolated; /* extrapaloated value of last PTS using first PTS + stream duration * 45K */
    BKNI_EventHandle sessionOpenRetryEventHandle;
    bool isPausedFromTrickMode;
    bool beginningOfStreamCallbackIssued; /* set when rewind has reached to the start of the stream and we have issued the callback. This avoids any further callbacks until next rewind operation. */
    bool endOfClientBufferCallbackIssued; /* set when fwd has reached to the end of the current stream buffer and we have issued the callback. This avoids any further callbacks until next fwd operation. */
    B_PlaybackIpHandle playbackIp2;
    B_PlaybackIpState *parentPlaybackIpState;
    bool trickmodeApiActive;
    B_PlaybackIp_EventCallback savedUserCallback;
    void *savedUserContext;
    bool rtspLmStartDone;
    bool rtspPsiSetupDone;
    FILE *fclearIndex;
    char playSpeedString[8];
    bool frameAdvanceApiActive;
    bool playApiActive;
    bool forward;
    bool frameRewindCalled;
    B_PlaybackIpPsiStateHandle pPsiState;
    uint32_t fifoMarker;
    B_Time fifoMarkerUpdateTime;
} B_PlaybackIp;

/* Output structure used in security session opens */
typedef struct B_PlaybackIpSecurityOpenOutputParams {
    B_PlaybackIpNetIo *netIoPtr; /* interface for network IO, implemented by each security module */
    void **securityHandle;
    unsigned int byteRangeOffset; /* byte range offset calculated from seekTimeOffset */
} B_PlaybackIpSecurityOpenOutputParams;

typedef enum {
    HTTP_URL_IS_DIRECT, /* e.g. www.mydeo.com/portugal.mpg */
    HTTP_URL_IS_ASX,    /* e.g. www.mydeo.com/portugal.asp */
    HTTP_URL_IS_REDIRECT, /* e.g. www.mydeo.com/portugal.asp */
    HTTP_URL_IS_PLS    /* a playlist file format used by some radio stations */
}http_url_type_t;
extern int http_parse_url(char *server, unsigned *portPtr, char **uri, char *url);
extern int http_parse_redirect(char *server, unsigned *portPtr, B_PlaybackIpProtocol *protocol, char **urlPtr, char **cookie, char *http_hdr);

struct udp_hdr {
    unsigned short   source;
    unsigned short   dest;
    unsigned short   len;
    unsigned short   checksum;
};


typedef struct B_PlaybackIp_RtpHeader {
    uint16_t word0;
#define VERSION_MASK 0xc000
#define VERSION_SHIFT 14
#define PADDING_MASK 0x2000
#define PADDING_SHIFT 13
#define EXTENSION_MASK 0x1000
#define EXTENSION_SHIFT 12
#define CSRC_MASK 0x0f00
#define CSRC_SHIFT 8
#define MARKER_MASK 0x0080
#define MARKER_SHIFT 7
#define TYPE_MASK 0x007f
#define TYPE_SHIFT 0
    uint16_t sequenceNum;
    uint32_t timestamp;
    uint32_t ssrc;
} B_PlaybackIp_RtpHeader;

#define breakFromLoopDueToChChg(playback_ip) \
    (\
     (playback_ip)->playback_state == B_PlaybackIpState_eStopping || \
     (playback_ip)->playback_state == B_PlaybackIpState_eStopped \
    )\

#define breakFromLoop(playback_ip) \
    (           \
      ((playback_ip)->parentPlaybackIpState && \
           (\
            *(playback_ip)->parentPlaybackIpState == B_PlaybackIpState_eStopping || \
            *(playback_ip)->parentPlaybackIpState == B_PlaybackIpState_eStopped || \
            *(playback_ip)->parentPlaybackIpState == B_PlaybackIpState_eWaitingToEnterTrickMode || \
            (playback_ip)->playback_state == B_PlaybackIpState_eStopping || \
            (playback_ip)->playback_state == B_PlaybackIpState_eStopped || \
            (playback_ip)->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode \
           )\
      ) || \
      (!(playback_ip)->parentPlaybackIpState && \
           (\
            (playback_ip)->playback_state == B_PlaybackIpState_eStopping || \
            (playback_ip)->playback_state == B_PlaybackIpState_eStopped || \
            (playback_ip)->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode \
           )\
      )  \
    ) \

#define playbackIpState(playback_ip) \
      ((playback_ip)->parentPlaybackIpState ? *(playback_ip)->parentPlaybackIpState:(playback_ip)->playback_state) \

#ifdef __cplusplus
extern "C" {
#endif
B_PlaybackIpError B_PlaybackIp_GetRtpHeaderData(
    B_PlaybackIpHandle p,
    unsigned int maxDataSize,
    B_PlaybackIp_RtpStatsHeader *rtpHeaderData,
    unsigned *entry_cnt
    );
#ifdef __cplusplus
}
#endif

#endif /* __B_PLAYBACK_IP_PRIV__*/
