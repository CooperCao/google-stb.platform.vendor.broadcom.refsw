
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

BDBG_MODULE(mediafile_test_Streamingmode);

#define MAX_STATIC_CHANNELS             4
#define MAX_PROGRAMS_PER_CHANNEL        6
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_AUDIO_STREAMS               4
#define MAX_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 2
#define TSB_SERVICE_PREFIX              "streamPath"
#define BUFFER_LEN                      4096*188
#define BUFFERALIGNMENT                 4096

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

static DvrTest_ChannelInfo channelInfo[MAX_STATIC_CHANNELS] =
{
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,429000000,"hockey",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,435000000,"color beat",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,447000000,"hockey match", 3,3,{0x11,0x21,0x41},
    {0x14,0x24,0x44},{0x11,0x21,0x41},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
    {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},

    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e256,5360537,525000000,"sc_bugs_baywatch",3,3,{0x31,0x11,0x21},
    {0x34,0x14,0x24},{0x31,0x11,0x21},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
    {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},
};

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
    B_DVR_PlaybackServiceRequest playbackServiceRequest;
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
    NEXUS_PlatformSettings platformSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    DvrTest_StreamPathHandle streamPath[MAX_STREAM_PATH];
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    int maxChannels;
    B_DVR_MediaFileHandle   MediaFileHdl;
    void *buffer;

};
typedef struct DvrTest *DvrTestHandle;

DvrTestHandle g_dvrTest;

#define MEDIA_STORAGE_REGISTRY "./vregistry"
const char *programName = "permrecord1"; 

void DvrTest_MediaStorageMenu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t mediaStorage Operations");
    printf("\n ************************************************");
    printf("\n 1: format");
    printf("\n 2: register");
    printf("\n 3: Unregister");
    printf("\n 4: volume status");
    printf("\n 5: mount");
    printf("\n 6: unmount");
    printf("\n 7: get metadata folder path");
    printf("\n 0: exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrTest_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t MedaiFile Play Operations");
    printf("\n ************************************************");
    printf("\n 1: Mediafile Play start");
    printf("\n 2: Medaifile Play Stop");
    printf("\n 3: Medaifile Play Close");
    printf("\n 4: Medaifile Read");
    printf("\n 5: Medaifile Seek");
    printf("\n 0: exit\n");
    printf("\n Select an operation: ");
    return;
}
typedef enum DvrTest_Operation
{
    eDvrTest_OperationQuit,
    eDvrTest_OperationMediaFilePlay,
    eDvrTest_OperationMediaFileStop,
    eDvrTest_OperationMediaFileClose,
    eDvrTest_OperationMediaFileRead,
    eDvrTest_OperationMediaFileSeek,
    eDvrTest_OperationMax
}DvrTest_Operation;

void DvrTest_StreamPathOpen(unsigned index)
{
    printf("\n %s:  index %d >>> ",__FUNCTION__,index);
    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].stcSettings.timebase = index?NEXUS_Timebase_e1:NEXUS_Timebase_e0;
    g_dvrTest->streamPath[index].stcChannel = NEXUS_StcChannel_Open(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].videoProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].audioProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].window = NEXUS_VideoWindow_Open(g_dvrTest->display,index);
    g_dvrTest->streamPath[index].videoDecoder = NEXUS_VideoDecoder_Open(index, NULL);
    NEXUS_VideoWindow_AddInput(g_dvrTest->streamPath[index].window,
                               NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    g_dvrTest->streamPath[index].audioDecoder = NEXUS_AudioDecoder_Open(index, NULL);
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}
void DvrTest_StreamPathClose(int index)
{
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);

    NEXUS_VideoWindow_RemoveAllInputs(g_dvrTest->streamPath[index].window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    NEXUS_VideoWindow_Close(g_dvrTest->streamPath[index].window);
    NEXUS_StcChannel_Close(g_dvrTest->streamPath[index].stcChannel);
    NEXUS_VideoDecoder_Close(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_AudioDecoder_Close(g_dvrTest->streamPath[index].audioDecoder);
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}


static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    printf( "\n HDMI hotplug event: %s", status.connected?"connected":"not connected");
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

int main(void)
{
    int decoderIndex, pathIndex, operation;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    DvrTest_Operation dvrTestOp;
    unsigned volumeIndex = 0;

    B_DVR_MediaFilePlayOpenSettings    TestFilePlay_OpenSetting;
    B_DVR_EsStreamInfo esStreamInfo[2],*PesStreamInfo;
    B_DVR_MediaFileSeekSettings FileSeekSetting;

    unsigned currentChannel;
    unsigned esCount;
    unsigned *buffer;
    ssize_t returnbyte =0;
    B_DVR_MediaFileStats mediaFileStats;
    off_t readOffset,returnOffset;
    ssize_t returnSize;

    char targetfilename[256]="targetfilename";
    FILE *FileFp = NULL;


    printf("\n Enter Main function : \n");
/*    
    buffer = BKNI_Malloc(BUFFER_LEN+512);

    printf("\n Input buffer Address = %x \n",buffer);

    buffer = (unsigned char *)buffer + 512;

    printf("\n Input buffer Address = %x \n",buffer);

    buffer = (unsigned char *)((unsigned int)buffer & 0xfffff200);

    printf("\n Input buffer Address = %x \n",buffer);
    printf("\n Input buffer Address = %x \n",&(buffer[0]));
*/
    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(NULL);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
    printf("Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes);
    printf("Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes);

    sprintf(g_dvrTest->mediaStorageRegisterSettings.device,"/dev/sda");
    g_dvrTest->mediaStorageRegisterSettings.startSec = 0;
    g_dvrTest->mediaStorageRegisterSettings.length = 0;   

   if (B_DVR_MediaStorage_RegisterVolume(g_dvrTest->mediaStorage,&g_dvrTest->mediaStorageRegisterSettings,&volumeIndex) >0)
         printf("\n Register media volume error");
   else
         printf("\n Register media volume success vol:%d",volumeIndex);

   B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);

   B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

/*Test--*/

    if (!mediaStorageStatus.volumeInfo[0].registered)   
    {
       volumeIndex=0;
       printf("\n Enter volume index : ");
       scanf("%u", &volumeIndex);
	   B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);
       B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
       printf("Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes);
       printf("Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes);
    }
    printf("Volume %d ",volumeIndex);
    printf("\n %s", mediaStorageStatus.volumeInfo[volumeIndex].mounted?"Mounted":"Not Mounted");
    printf("\n\tname: %s", mediaStorageStatus.volumeInfo[volumeIndex].name);
    printf("\n\tmedia segment size: %d", mediaStorageStatus.volumeInfo[volumeIndex].mediaSegmentSize);
    printf("\n\tmediaStorageStatus.numMountedVolumes: %d", mediaStorageStatus.numMountedVolumes);
    printf("\n\tB_DVR_MediaStorage_Handle: =%s\n",(char *)g_dvrTest->mediaStorage);

    
    g_dvrTest->streamPath[0].currentChannel = 0;
    if(NEXUS_NUM_VIDEO_DECODERS > 1)
    g_dvrTest->streamPath[1].currentChannel= 1;

    printf("\n Before enter B_OS_INIT\n");

    B_Os_Init();

    printf("\n Finished B_OS_INIT\n");

    NEXUS_Platform_GetDefaultSettings(&(g_dvrTest->platformSettings));
    g_dvrTest->platformSettings.openFrontend = false;

    NEXUS_Platform_Init(&(g_dvrTest->platformSettings));

    printf("\n Finished PLATFORM INIT\n");

    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);

    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));

    printf("\n Before enter HDMIOutput setting\n");

    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);

    for(decoderIndex=0;decoderIndex < 2; decoderIndex++)
    {
        DvrTest_StreamPathOpen(decoderIndex);
    }

    printf("\n Before enter B_DVR_Manager_Init\n");
    g_dvrTest->dvrManager = B_DVR_Manager_Init(NULL);
    if(!g_dvrTest->dvrManager)
    {
        printf("Error in opening the dvr manager\n");
    } 
    printf("\n Done of calling B_DVR_Manager_Init\n");

    
    if ((FileFp = fopen(targetfilename, "w+" )) == NULL) {
        printf("\n %s: Unable to open a target file %s\n", __FUNCTION__, targetfilename);
        goto endofmainfile;
    }

    while(1)
    { 
        DvrTest_Menu();
        fflush(stdin);
        operation=0;
        scanf("%d", &operation);
        dvrTestOp = (DvrTest_Operation)operation; 
        if(dvrTestOp > eDvrTest_OperationQuit && operation < eDvrTest_OperationMax)
        {
            printf("\n enter streamPath Index for %d : ",operation);
            fflush(stdin);
            pathIndex=0;
            scanf("%d",&pathIndex);
         }

        if (!operation)
        {
            break;
        }
        
        switch(dvrTestOp)
        {
        case eDvrTest_OperationMediaFilePlay:

                TestFilePlay_OpenSetting.activeVideoPidIndex[0] = 0;
                TestFilePlay_OpenSetting.activeVideoPidIndex[1] = -1;
                TestFilePlay_OpenSetting.activeAudioPidIndex = 1;
                TestFilePlay_OpenSetting.volumeIndex = 0;
                TestFilePlay_OpenSetting.playpumpIndex = pathIndex;
                TestFilePlay_OpenSetting.stcChannel = g_dvrTest->streamPath[0].stcChannel;
                TestFilePlay_OpenSetting.videoDecoder[0] = g_dvrTest->streamPath[0].videoDecoder;
                TestFilePlay_OpenSetting.videoDecoder[1] = g_dvrTest->streamPath[1].videoDecoder;
                TestFilePlay_OpenSetting.audioDecoder[0] = g_dvrTest->streamPath[0].audioDecoder;
                TestFilePlay_OpenSetting.audioDecoder[1] = g_dvrTest->streamPath[1].audioDecoder;

                printf("\n Before enter B_DVR_MediaFile_Open\n");

                g_dvrTest->MediaFileHdl = B_DVR_MediaFile_Open(programName,eB_DVR_MediaFileOpenModeStreaming,&TestFilePlay_OpenSetting);
                
                printf("\n Done of calling B_DVR_MediaFile_Open Handler =%s\n",(char *)g_dvrTest->MediaFileHdl);
                /*
                 * Change in API should be reflected here 
                 */
                #if 0
                PesStreamInfo = &(esStreamInfo[0]);
                dvrError = B_DVR_MediaFile_GetEsStreamInfo(g_dvrTest->MediaFileHdl,PesStreamInfo,&esCount);
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("\n Error in GetESStreaminfo %d\n",pathIndex);
                }
                else
                {
                    printf("\n GetEsStreamInfo Success %d\n",pathIndex);
                }

                    g_dvrTest->streamPath[pathIndex].videoProgram.codec = PesStreamInfo->codec.videoCodec;
                    g_dvrTest->streamPath[pathIndex].videoProgram.pidChannel = PesStreamInfo->pidChannel;
                    printf("\n playback pid channel %x",(unsigned )(PesStreamInfo->pidChannel));

                currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                PesStreamInfo->pid = channelInfo[currentChannel].videoPids[0];
                PesStreamInfo->pidType = NEXUS_PidType_eVideo;
                PesStreamInfo->codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                
                PesStreamInfo++;
                PesStreamInfo->pid = channelInfo[currentChannel].audioPids[0];
                PesStreamInfo->pidType = NEXUS_PidType_eAudio;
                PesStreamInfo->codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                
                dvrError = B_DVR_MediaFile_SetEsStreamInfo(g_dvrTest->MediaFileHdl,PesStreamInfo,&esCount);
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("\n Error in SetESStreaminfo %d\n",pathIndex);
                }
                else
                {
                    printf("\n SetEsStreamInfo Success %d\n",pathIndex);
                }
                #endif
                  break;
        case  eDvrTest_OperationMediaFileRead:

                buffer = BKNI_Malloc(BUFFER_LEN+BUFFERALIGNMENT);
                g_dvrTest->buffer = (void*)((unsigned long)buffer & 0xffff1000);
                if(!g_dvrTest->buffer)
                {
                    printf("\n error_buf_alloc\n");
                    break;
                }

                printf("\n enter B_DVR_MediaFile_Read\n");

                B_DVR_MediaFile_Stats(g_dvrTest->MediaFileHdl,&mediaFileStats);

                readOffset = FileSeekSetting.mediaSeek.offset = mediaFileStats.startOffset;
                FileSeekSetting.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
                returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->MediaFileHdl,&FileSeekSetting);
                if(returnOffset < 0)
                {
                    BKNI_Free(buffer);
                    printf("\n error seeking\n");
                    break;
                }

                for(;readOffset<mediaFileStats.endOffset;readOffset+=BUFFER_LEN) 
                {
                   returnSize = B_DVR_MediaFile_Read(g_dvrTest->MediaFileHdl,g_dvrTest->buffer,BUFFER_LEN);
                   if(returnSize < 0)
                   {
                        printf("\n Error reading\n");
                        break;
                   }
                   else
                   {
                        returnbyte += returnSize;
                        printf("\n Total read data size = %u",returnSize);
                        fwrite(g_dvrTest->buffer, 1,returnSize,FileFp);
                        if ((returnSize == 0)||(returnSize < BUFFER_LEN))
                            break;
                   }
                 }

        /*returnbyte = B_DVR_MediaFile_Read(g_dvrTest->MediaFileHdl,buffer,len);

        if(returnbyte != len)
            printf("\n Error in MediaFileRead %d\n",returnbyte);
        else
        { i =0;
          while(i<BUFFER_LEN)
           {
             printf("\t%c",buffer[i++]);
             if ((i%20)==0)
            {
            printf("\n");i++;
            }
            }
        }*/     
              break;

        case  eDvrTest_OperationMediaFileSeek:

            FileSeekSetting.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
            FileSeekSetting.mediaSeek.offset = 0;
            /*FileSeekSetting.mediaSeek.offset = (g_dvrTest->MediaFileHdl->currentOffset) + returnbyte;*/
            dvrError =B_DVR_MediaFile_Seek(g_dvrTest->MediaFileHdl,&FileSeekSetting);
            if(dvrError!=B_DVR_SUCCESS)
                printf("\n Error in OperationMediaFileSeek \n");
            else
                printf("\n OperationMediaFileSeek Success \n");
            break;


        case eDvrTest_OperationMediaFileStop:

            dvrError = B_DVR_MediaFile_PlayStop(g_dvrTest->MediaFileHdl);
            if(dvrError!=B_DVR_SUCCESS)
            {
                printf("\n Error in MediaFile_PlayStop %d\n",pathIndex);
            }
            else
            {
                printf("\n MediaFile_PlayStop Success %d\n",pathIndex);
            }
            break;

        case eDvrTest_OperationMediaFileClose:

            dvrError = B_DVR_MediaFile_Close(g_dvrTest->MediaFileHdl);
            if(dvrError!=B_DVR_SUCCESS)
            {
                printf("\n Error in MediaFile_Close %d\n",pathIndex);
            }
            else
            {
                printf("\n MediaFile_Close Success %d\n",pathIndex);
            }
            break;
            
        default:
            printf("Invalid DVR Operation - Select the operation again");
            break;
        }  
    }
    
    printf("\n Closing all the services before stopping the test app");

    if (FileFp) 
        fclose(FileFp);
endofmainfile:
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();
  
  return 0;
}
