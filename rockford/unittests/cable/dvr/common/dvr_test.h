/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ****************************************************************************/
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "bstd.h"
#include "nexus_timebase.h"
#include "nexus_base_types.h"
#include "nexus_frontend.h"
#include "nexus_frontend_qam.h"
#include "nexus_parser_band.h"
#include "nexus_display.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_types.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_output.h"
#include "nexus_audio_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_svideo_output.h"
#include "nexus_audio_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"
#include "b_dvr_manager.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_mediafile.h"
#include "b_dvr_datainjectionservice.h"
#include "CuTest.h"
#include "b_dvr_drmservice.h"

#include "msutil.h"
#include "msdiag.h"
#include "tshdrbuilder.h"



/* TODO: change makefile to enable DRM */
#define ENABLE_DRM
#define HNSTREAM



#define MAX_STATIC_CHANNELS             4
#define MAX_PROGRAMS_PER_CHANNEL        6
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_AUDIO_STREAMS               4
#define MAX_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 (NEXUS_MAX_FRONTENDS-1)
#define MAX_AV_PATH                     2
#define TSB_SERVICE_PREFIX              "streamPath"
#define MAX_TSB_BUFFERS                 5
#define BTST_TS_HEADER_BUF_LENGTH   	189
#define MAX_TSB_TIME                    5*60*1000 /*5mins*/



#if defined(ENABLE_DRM)
#define MAX_DRM_NUM					  50
#define KEY_LENGTH                    16
#endif

#ifdef HNSTREAM
#define BUFFER_LEN                      4096*188
#define BUFFERALIGNMENT                 4096
#define PACKET_LENGTH                   188
#define PACKET_CHUNK                    188*7
#define CHUNK_ELEMENT                   48*4
#define SOCKET_IO_VECTOR_ELEMENT        1
#define IP_ADDR_STRING_LEN              30
#define FrameSymbolRate                 8
#define MultiplePlayback                2
#define STREAM_TSB_SERVICE_PREFIX       "mulstreamPath"
#endif


typedef struct DvrTest_ChannelInfo
{
    NEXUS_FrontendQamAnnex annex; /* To describe Annex A or Annex B format */
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolrate;
    unsigned int  frequency;
    char programTitle[MAX_PROGRAM_TITLE_LENGTH];
    int numAudioStreams;
    int numVideoStreams;
    unsigned int  videoPids[MAX_VIDEO_STREAMS];
    unsigned int  audioPids[MAX_AUDIO_STREAMS];
    unsigned int  pcrPid[MAX_VIDEO_STREAMS];
    NEXUS_VideoCodec videoFormat[MAX_VIDEO_STREAMS];
    NEXUS_AudioCodec audioFormat[MAX_AUDIO_STREAMS];
} DvrTest_ChannelInfo;


#ifdef HNSTREAM
typedef enum hnChunkBufID
{
    HnChunkBufInit,
    HnChunkBufFirstHalf,
    HnChunkBufSecondHalf,
    HnChunkBufEndBuf
}hnChunkBufID;

typedef struct hnPlayScale
{
    int numerator;
    int denominator;
}hnPlayScale;

typedef struct hnstreamingpara
{
    B_DVR_MediaFileDirection Direction;
    B_DVR_MediaFileReadMode  TrickMode;
    B_DVR_MediaFileSeekMode  SeekMode;
    unsigned long            SeekValue;
    char                     targetfilename[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     programName[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned long            streamingmode;
    unsigned long            DRMMode;
    int                      StreamPathIndex;
    int                      PlayScaleSpeed;
    char                     dstip[IP_ADDR_STRING_LEN];
    char                     interfaceName[16];
    char                     portnum[2];
}hnstreamingpara;

typedef struct hnrecordpara
{
    int                      StreamPathIndex;
    char                     programName[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     programSubDir[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     nfsSourceFilePath[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     nfsSourceFileName[B_DVR_MAX_FILE_NAME_LENGTH];
}hnrecordpara;

typedef struct hnmediaplaypara
{
    int                      StreamPathIndex;
    char                     programName[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     programSubDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned long            DRMMode;
    unsigned long            TrickModeSet;
    B_DVR_Operation          TrickValue;
}hnmediaplaypara;

typedef struct hnmulstreams
{
    struct sockaddr_in si_other;
    struct msghdr msg;
    struct iovec  iovec_set[SOCKET_IO_VECTOR_ELEMENT];
    struct ifreq ifr;
    int  s;
    int  bytesWritten, bytesToWrite, bytesRead;
    int  counter;
    char dstip[IP_ADDR_STRING_LEN];
    char RecoredProgName[B_DVR_MAX_FILE_NAME_LENGTH];
    char interfaceName[16];
    char portnum[2];
}hnmulstreams;
#endif

struct DvrTest_StreamPath
{
    DvrTest_ChannelInfo *tunedChannel;
    unsigned currentChannel;
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendQamStatus qamStatus;
    NEXUS_FrontendCapabilities capabilities;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelHandle stcAudioChannel;
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_TSBServiceRequest tsbServiceRequest;
    B_DVR_TranscodeServiceHandle transcodeService;
    B_DVR_TranscodeServiceRequest transcodeServiceRequest;
    BKNI_EventHandle transcodeFinishEvent;
    unsigned transcodeOption;
    B_DVR_PlaybackServiceHandle playbackService;
    NEXUS_PidChannelHandle audioPlaybackPidChannels[MAX_VIDEO_STREAMS];
    NEXUS_PidChannelHandle videoPlaybackPidChannels[MAX_AUDIO_STREAMS];
    NEXUS_PidChannelHandle audioPidChannels[MAX_VIDEO_STREAMS];
    NEXUS_PidChannelHandle videoPidChannels[MAX_AUDIO_STREAMS];
    NEXUS_PlaybackHandle playback;
    B_DVR_PlaybackServiceRequest playbackServiceRequest;
    B_DVR_RecordServiceRequest recordServiceRequest;
    B_DVR_RecordServiceHandle recordService;
    B_DVR_DataInjectionServiceHandle dataInjectionService;
#if defined(ENABLE_DRM)
	B_DVR_DRMServiceHandle drmService[MAX_DRM_NUM];
	B_DVR_DRMServiceRequest drmServiceRequest; 
#endif
    bool liveDecodeStarted;
    bool playbackDecodeStarted;	
#ifdef HNSTREAM
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_MediaFileOpenMode openMode;
    B_DVR_MediaFilePlayOpenSettings openSettings;
    B_DVR_MediaFileHandle mediaFile;
    int fd;
    void *orgbuffer;
    void *buffer;
    char *adjbuf;	
    char nonsegfileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char threadName[B_DVR_MAX_FILE_NAME_LENGTH];
    B_ThreadHandle	threadID;
    ssize_t returnSize;
    ssize_t readSize;
    ssize_t tempSize;		
    off_t readOffset;
    B_DVR_MediaFileStats mediaFileStats;
    int   taskProcessing;		
    hnmulstreams  hnMulStreamPara;	
    struct timeval start, stop;
    struct timeval estStartTime;
    double dt;
    double rate;
    int    ChunkLoopRead;
    int    flag;	
    int    zeroRateFlag;
    ssize_t zeroBackupReturnSize;
	B_SchedulerTimerId	  streamTimer;
	ssize_t 			  bufSize;
	unsigned int		  timerPeriod;
	struct timeval		  startTime;
	struct timeval		  endTime;
	struct timeval		  testTime;
	unsigned int		  dtPeriod;
	hnChunkBufID		  chunkFlag;
#endif
    bool dataInjectionStarted;
    bool tsbConversionStarted;
    uint8_t pat[BTST_TS_HEADER_BUF_LENGTH];
    uint8_t pmt[BTST_TS_HEADER_BUF_LENGTH];
    unsigned patCcValue;
    unsigned pmtCcValue;
    unsigned dataInsertionCount;
    B_SchedulerTimerId timer;
    B_SchedulerEventId tsbConvEventSchedulerID;
    B_EventHandle tsbConvEvent;
    B_DVR_TSBServicePermanentRecordingRequest permRecReq;
	unsigned psiCount;
	B_SchedulerTimerId patMonitorTimer;
    B_SchedulerTimerId dataInjectTimer;
    B_DVR_MediaFileHandle patFile;
    void *patBuffer;
    off_t patSeekOffset;
    unsigned patSegmentCount;
	B_MutexHandle streamPathMutex;
};

typedef struct DvrTest_StreamPath DvrTest_StreamPathHandle;

struct DvrTest
{
    B_DVR_ManagerHandle dvrManager;
    B_DVR_MediaStorageHandle mediaStorage;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageRegisterVolumeSettings mediaStorageRegisterSettings;
    NEXUS_PlatformConfiguration platformconfig;
    DvrTest_StreamPathHandle streamPath[MAX_STREAM_PATH];
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
#endif
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    int maxChannels;
    B_DVR_ServiceCallback dvrTestCallback;
    void *buffer;
    B_DVR_MediaFileHandle mediaFile;
#if defined(ENABLE_DRM)
    B_DVR_DRMServiceSettings drmServiceSettings;
	NEXUS_DmaHandle dma;
	NEXUS_DmaHandle dma2;
#endif
#ifdef HNSTREAM
    int pathIndex;
    B_DVR_DRMServiceHandle  DecryptDrmHandle;
    NEXUS_DmaHandle         decDma;
    B_DVR_MediaFileReadMode frameTypeInTrickMode;
    B_DVR_MediaFileStreamFrameRate      streamFrameRate;
    unsigned long           frameSymbolRate;
    unsigned long           skipStepIPFrame;
    unsigned long           dupNumIPFrame;
    unsigned long           usedNumIPFrame;
    unsigned long           interDumpFlag;
    unsigned long           interDumpPlusDupNumIPFrame;
    hnPlayScale             playScale;
    int                     playbackactived;
    int                     playbackDRMEnabledMode;
    hnstreamingpara         hnStreamPara;
    hnrecordpara            hnRecPara;
    hnmediaplaypara         hnMPPara;	
    unsigned char           multipleStreamFlag;
    B_MutexHandle           glbstreamMutex;
    B_ThreadHandle          threadID;
    char                    threadName[B_DVR_MAX_FILE_NAME_LENGTH];
    B_MutexHandle           PFschedulerMutex;
    B_SchedulerHandle       PFscheduler;
    B_ThreadHandle          PFthread;
#endif
};
typedef struct DvrTest *DvrTestHandle;


#define MEDIA_STORAGE_REGISTRY "./vregistry"

typedef enum DvrTest_Operation
{
    eDvrTest_OperationQuit,
    eDvrTest_OperationChannelChange,
    eDvrTest_OperationChannelUp,
    eDvrTest_OperationChannelDown,
    eDvrTest_OperationTSBServiceStart,
    eDvrTest_OperationTSBServiceStop,
    eDvrTest_OperationTSBServicePlayStart,
    eDvrTest_OperationTSBServicePlayStop,
    eDvrTest_OperationTSBServiceConvertStart,
    eDvrTest_OperationTSBServiceConvertStop,
    eDvrTest_OperationPlaybackServiceStart,
    eDvrTest_OperationPlaybackServiceStop,
    eDvrTest_OperationRecordServiceStart,
    eDvrTest_OperationRecordServiceStop,
    eDvrTest_OperationDataInjectionStart,
    eDvrTest_OperationDataInjectionStop,
    eDvrTest_OperationPlay,
    eDvrTest_OperationPlayGop,
    eDvrTest_OperationPause,
    eDvrTest_OperationRewind,
    eDvrTest_OperationFastForward,
    eDvrTest_OperationSlowRewind,
    eDvrTest_OperationSlowForward,
    eDvrTest_OperationFrameAdvance,
    eDvrTest_OperationFrameReverse,
    eDvrTest_OperationMediaFileRead,
    eDvrTest_OperationMediaFileWrite,
    eDvrTest_OperationListRecordings,
    eDvrTest_OperationSeek,
#if defined(ENABLE_DRM)
	/* DRM support CPS and M2M for ClearKey and Keyladder */

	/* CPS using same key value, sc polarity is eClear for M2M playback */
    eDvrTest_OperationListRecordServiceCpsClearKeyStart,
    eDvrTest_OperationListRecordServiceCpsClearKeyStop,
    eDvrTest_OperationListRecordServiceCpsKeyladderStart,
    eDvrTest_OperationListRecordServiceCpsKeyladderStop,    

    /* M2M record for M3M playback  */
	eDvrTest_OperationListRecordServiceM2mClearKeyStart,
	eDvrTest_OperationListRecordServiceM2mClearKeyStop,    
    eDvrTest_OperationListRecordServiceM2mKeyladderStart,
    eDvrTest_OperationListRecordServiceM2mKeyladderStop,

    /* M2M playback*/
	eDvrTest_OperationListPlaybackServiceM2mClearKeyStart,
	eDvrTest_OperationListPlaybackServiceM2mClearKeyStop,    
    eDvrTest_OperationListPlaybackServiceM2mKeyladderStart,
    eDvrTest_OperationListPlaybackServiceM2mKeyladderStop,

	/* TSB CPS using same key value, sc polarity is eClear for M2M TSB playback */
	eDvrTest_OperationTSBServiceCpsClearKeyStart,
	eDvrTest_OperationTSBServiceCpsClearKeyStop,	
	eDvrTest_OperationTSBServiceCpsKeyladderStart,
	eDvrTest_OperationTSBServiceCpsKeyladderStop,	

	/* TSB CPS using same key value, sc polarity is eOdd for CA TSB playback */
	eDvrTest_OperationTSBServiceCpsClearKeyStartForCa,
	eDvrTest_OperationTSBServiceCpsKeyladderStartForCa,

	/* TSB M2M playback */
	eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStart,
	eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStop, 
	eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStart,
	eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStop,	

	/* CPS using even/odd keys , sc polarity is even/odd for CA playback [ include various keyloading ] */
    eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStart, 
    eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStop,

	/* CA/CPD using even/odd keys */
	eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStart,
	eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStop,

	/* TSB M2M record for M2M playback  */
	eDvrTest_OperationTSBServiceM2mClearKeyStart,
	eDvrTest_OperationTSBServiceM2mClearKeyStop,	
	eDvrTest_OperationTSBServiceM2mKeyladderStart,
	eDvrTest_OperationTSBServiceM2mKeyladderStop,	

    /* TSB CPS using odd keys , sc polarity is odd for CA playback */
	eDvrTest_OperationTSBServiceCaClearKeyPlaybackStart,
	eDvrTest_OperationTSBServiceCaClearKeyPlaybackStop,	
	eDvrTest_OperationTSBServiceCaKeyladderPlaybackStart,
	eDvrTest_OperationTSBServiceCaKeyladderPlaybackStop,	

	/* TSB M2M to M2M Stop*/
	eDvrTest_OperationTSBServiceM2mToM2mClearKeyPlaybackStop,
	eDvrTest_OperationTSBServiceM2mToM2mKeyladderPlaybackStop,

#endif    
    eDvrTest_OperationMax
}DvrTest_Operation;

typedef struct
{
    int pathIndex;
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];
	char subDir [B_DVR_MAX_FILE_NAME_LENGTH];
	unsigned long seekTime;
} DvrTest_Operation_Param;

extern DvrTest_ChannelInfo channelInfo[MAX_STATIC_CHANNELS];

extern DvrTestHandle g_dvrTest;

void Do_DvrTest_Operation(CuTest *tc, DvrTest_Operation dvrTestOp, DvrTest_Operation_Param * param);

#ifdef ENABLE_DRM
extern unsigned char changeToLocalSession[16];

extern unsigned char changeToLocalControlWord[16];

extern unsigned char changeToIv[16];

extern void procInKey3_sessionKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle);

extern void procInKey4_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle);

extern void iv_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle);

#endif
