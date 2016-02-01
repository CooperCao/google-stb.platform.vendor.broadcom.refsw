#ifndef _B_DVR_TEST_H
#define _B_DVR_TEST_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
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

#define MAX_STATIC_CHANNELS             4
#define MAX_PROGRAMS_PER_CHANNEL        6
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_AUDIO_STREAMS               4
#define MAX_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 (NEXUS_MAX_FRONTENDS-1)
#define MAX_AV_PATH                     1
#define TSB_SERVICE_PREFIX              "streamPath"
#define MAX_TSB_BUFFERS                 5

#define MEDIA_STORAGE_REGISTRY "./vregistry"

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
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_TSBServiceRequest tsbServiceRequest;
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
    bool liveDecodeStarted;
    bool playbackDecodeStarted;
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
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    int maxChannels;
    B_DVR_ServiceCallback dvrTestCallback;
    void *buffer;
    B_DVR_MediaFileHandle mediaFile;
};

typedef struct DvrTest *DvrTestHandle;
DvrTestHandle g_dvrTest;


#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_TEST_H */

