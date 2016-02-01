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
#define PACKET_LENGTH                   188
#define PACKET_CHUNK            188*2
#define CHUNK_ELEMENT                   48
#define SOCKET_IO_VECTOR_ELEMENT        2
#define IP_ADDR_STRING_LEN              30

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
    B_DVR_MediaStorageVolumeFormatSettings mediaStorageFormatSettings;
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
    void *buffer;
    B_DVR_MediaFileHandle mediaFile;
    int pathIndex;
};
typedef struct DvrTest *DvrTestHandle;

DvrTestHandle g_dvrTest;

#define MEDIA_STORAGE_REGISTRY "./vregistry"
const char *programName = "perm_000"; 

typedef enum PLAYBACKTEST_Operation{
    PLAYBACKTEST_OperationQuit,
    PLAYBACKTEST_START,
    PLAYBACKTEST_STOP,
    PLAYBACKTEST_OperationMax
}PLAYBACKTEST_Operation;

typedef enum DvrTrickMode_Operation{
    eDvrHNTrickMode_TrickModeExit,
    eDvrHNTrickMode_TrickModeFULL,
    eDvrHNTrickMode_TrickModeI,
    eDvrHNTrickMode_TrickModeIP,
    eDvrHNTrickMode_TrickModeMax
}DvrTrickMode_Operation;

typedef enum DvrHNTest_Operation
{
    eDvrHNTest_OperationQuit,
    eDvrHNTest_Streamingmode,
    eDvrHNTest_Playbackmode,
    eDvrHNTest_Recordingmode,
    eDvrHNTest_OperationMax
}DvrHNTest_Operation;

typedef enum HNUDPMode_Operation
{
    eDvrHNUDPMode_OperationQuit,
    eDvrHNUDPMode_Vlc,
    eDvrHNUDPMode_File,
    eDvrHNUDPMode_OperationMax
}HNUDPMode_Operation;

typedef enum HNSTREAMING_Operation
{
    eDvrHNStream_OperationQuit,
    eDvrHNStream_Open,
    eDvrHNStream_HN_Chunk,
    eDvrHNStream_UDP_Mode_Opt,
    eDvrHNStream_Close,
    eDvrHNStream_HN_Status,
    eDvrHNStream_OperationMax
}HNSTREAMING_Operation;

typedef enum HNRECORD_Operation
{
    eDvrHNRrcord_OperationQuit,
    eDvrHNRrcord_TS_Format,
    eDvrHNRrcord_Container_Format,
    eDvrHNRrcord_OperationMax
}HNRECORD_Operation;

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

void DvrHNTest_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t HomeNetwork MediaFile I/F Operations");
    printf("\n ************************************************");
    printf("\n 1: Streaming Mode");
    printf("\n 2: Playback Mode");
    printf("\n 3: Recording Mode");
    printf("\n 0: Exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrHNUDP_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t UDP Mode Operations");
    printf("\n ************************************************");
    printf("\n 1: UDP streaming with remote pc running VLC");
    printf("\n 2: UDP streaming into local file");
    printf("\n 0: Exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrHNStreaming_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t Streaming Mode Operations");
    printf("\n ************************************************");
    printf("\n 1: Hn  Streaming open");
    printf("\n 2: Hn  Chunk mode setting(Normal, I, IP frame)");
    printf("\n 3: UDP streaming ");
    printf("\n 4: Hn  Streaming close");
    printf("\n 5: Hn  Streaming status query");
    printf("\n 0: Exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrHNRecord_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t Homenetwork Record Operations");
    printf("\n ************************************************");
    printf("\n 1: Record with TS format");
    printf("\n 2: Record with container format");
    printf("\n 0: Exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrHNTrickMode_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t TRICK MODE SELECTION");
    printf("\n ************************************************");
    printf("\n 1. TrickMode--FULL (Default Mode)");
    printf("\n 2. TrickMode--I");
    printf("\n 3. TrickMode--IP");
    printf("\n Select an operation: ");
    return;
}

void DvrPlaybacktest_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t PLAYBACK TEST SELECTION");
    printf("\n ************************************************");
    printf("\n 1. Playbacktest Start");
    printf("\n 2. Playbacktest Stop");
    printf("\n 0: Exit\n");
    printf("\n Select an operation: ");
    return;
}

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

    if(!index)
    {
        g_dvrTest->streamPath[index].audioDecoder = NEXUS_AudioDecoder_Open(index, NULL);
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                    NEXUS_AudioDecoderConnectorType_eStereo)
                                   );
    }
    else
    {
        g_dvrTest->streamPath[index].audioDecoder = NULL;
    }
  
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_StreamPathClose(int index)
{
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
    }
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

static void recordmodetest(DvrTestHandle dvrtesthandle)
{
     HNRECORD_Operation  HnRecordOp;
     int opt;

     while(1)
     {
         DvrHNRecord_Menu();
         fflush(stdin);
         opt=0;
         scanf("%d", &opt);
         HnRecordOp = (HNRECORD_Operation)opt; 

         if (!opt)
         {
             break;
         }

         switch(HnRecordOp)
         {
             case eDvrHNRrcord_TS_Format:
                 {
/*                 char programName[B_DVR_MAX_FILE_NAME_LENGTH]="brcmseg";
                   char nfsFileName[B_DVR_MAX_FILE_NAME_LENGTH]="trp_008_spiderman_lotr_oceans11_480i_q64.mpg";*/
                   char programName[B_DVR_MAX_FILE_NAME_LENGTH];
                   char nfsFileName[B_DVR_MAX_FILE_NAME_LENGTH];
                   B_DVR_MediaFileOpenMode openMode;
                   B_DVR_MediaFilePlayOpenSettings openSettings;
                   int nfsFileID;
                   unsigned long bufSize;
                   void *buffer;

                   fflush(stdin);
                   printf("\n Enter the NFS source file:");
                   scanf("%s",nfsFileName);
                   if ((nfsFileID = open(nfsFileName, O_RDONLY,0666)) < 0)
                   {
                       printf("\n Unable to open %s",nfsFileName);
                       break;
                   }
                   printf("\n Enter a program name for network recording:");
                   scanf("%s",programName);
                   sprintf(openSettings.subDir,"netRecording");
                   openMode = eB_DVR_MediaFileOpenModeRecord;
                   openSettings.playpumpIndex = 0;
                   openSettings.volumeIndex = 0;
                   bufSize = BUFFER_LEN;
                   buffer  = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
                   if(!buffer)
                   {
                      printf("\n unable to allocate source read buffer");
                      close(nfsFileID);
                      break;
                   }
                   dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);

                   dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);

                   if(!dvrtesthandle->mediaFile)
                   {
                       printf("\n unable to open dvr mediaFile %s",programName);
                       free(buffer);
                       close(nfsFileID);
                       break;
                   }
                
                   while (read(nfsFileID,dvrtesthandle->buffer,bufSize)>0)
                   {
                       B_DVR_MediaFile_Write(dvrtesthandle->mediaFile,dvrtesthandle->buffer,bufSize);
                   }
                   printf("\n writing the nfs file to multiple file segments");
                   B_DVR_MediaFile_Close(dvrtesthandle->mediaFile);
                   close(nfsFileID);
                   BKNI_Free(buffer);
                 }
                 break;

             case eDvrHNRrcord_Container_Format:
                 break;

             default:
                 break;
         }
     }
     return;
}

static void StreamToVLC(DvrTestHandle dvrtesthandle)
{
     struct sockaddr_in si_other;
     struct msghdr msg;
     struct iovec  iovec_set[SOCKET_IO_VECTOR_ELEMENT];
     struct ifreq ifr;
#if 0
     char PlayFileName[B_DVR_MAX_FILE_NAME_LENGTH]="trp_008_spiderman_lotr_oceans11_480i_q64.mpg";
     int  PlayFileID;
#endif
     int  s;
     int  bytesWritten, bytesToWrite, bytesRead;
     int  errorcode = 0;
     int  counter = 0;
     char dstip[IP_ADDR_STRING_LEN];
     char interfaceName[16];
     char portnum[2];
#if 0
     void *buffer;
     unsigned long bufSize = 188*4096;
#else
     unsigned long bufSize = PACKET_CHUNK*CHUNK_ELEMENT+B_DVR_IO_BLOCK_SIZE; 
     char *orgbuffer,*adjbuf;
#endif

     fflush(stdin);
     printf("\n Enter Destnation IP Addr:");
     scanf("%s", dstip);
     printf("\n Enter interface name:");
     scanf("%s", interfaceName);
     printf("\n Enter PORT Number (Default 5000):");
     scanf("%s", portnum);

     if ((s=socket(AF_INET, SOCK_DGRAM, 0))<0)
     {
         printf("\n socket open failed \n");   
         goto StreamToVLC_error_open_scoket;
     }
     si_other.sin_family = AF_INET;
     si_other.sin_port = htons(atoi(portnum));
     si_other.sin_addr.s_addr = inet_addr(dstip);
     strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);
     if ((errorcode = setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0 )
     {
        printf("\n Set Bingparater error = %d!\n",errorcode);
        goto StreamToVLC_error_set_scoket;
     }
     else
     {
        printf("\n Set Bingparater Success = %d!\n",errorcode);
     }

     orgbuffer  = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
     dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)orgbuffer);
#if 0
     if ((PlayFileID = open(PlayFileName, O_RDONLY,0666)) < 0)
     {
         printf("\n Unable to open %s",PlayFileName);
         goto StreamToVLC_error_set_scoket;
     }
#endif
#if 0
     while ((bytesRead = read(PlayFileID,dvrtesthandle->buffer,PACKET_CHUNK*CHUNK_ELEMENT))>0)
#else
     while ((bytesRead = B_DVR_MediaFile_Read(dvrtesthandle->mediaFile,dvrtesthandle->buffer,PACKET_CHUNK*CHUNK_ELEMENT))>0)
#endif
     {
        adjbuf = dvrtesthandle->buffer;
        if(bytesRead != PACKET_CHUNK*CHUNK_ELEMENT )
        {
            printf("\n Byteread != PACKET_CHUNK*CHUNK_ELEMENT =%d\n",bytesRead);
        }
        while(bytesRead!= 0)
        {
           if (bytesRead <= PACKET_CHUNK)
              bytesToWrite = bytesRead;
           else
              bytesToWrite = PACKET_CHUNK;

           memset(&(iovec_set[0]), 0, sizeof(struct iovec)*SOCKET_IO_VECTOR_ELEMENT);
           if(bytesToWrite >= PACKET_CHUNK)
           {
               iovec_set[0].iov_base = adjbuf;
               iovec_set[0].iov_len = PACKET_LENGTH; 
               iovec_set[1].iov_base = (adjbuf+PACKET_LENGTH); 
               iovec_set[1].iov_len = PACKET_LENGTH; 
           }
           else
           {
               iovec_set[0].iov_base = adjbuf;
               iovec_set[0].iov_len = PACKET_LENGTH; 
               iovec_set[1].iov_base = (adjbuf+PACKET_LENGTH); 
               if(bytesToWrite >= PACKET_LENGTH)
               {
                  iovec_set[1].iov_len = (bytesToWrite - PACKET_LENGTH); 
               }
               else
               {
                  iovec_set[1].iov_len = 0;
               }
           }
           memset(&msg, 0, sizeof(struct msghdr));
           msg.msg_name = (struct sockaddr *)&si_other;
           msg.msg_namelen = sizeof(struct sockaddr_in);
           msg.msg_iov = &(iovec_set[0]);
           msg.msg_iovlen = SOCKET_IO_VECTOR_ELEMENT;
           if ((bytesWritten = sendmsg(s, &msg, 0)) <= 0) 
           {
               printf("\n ERROR writing to socket\n");
           }
           bytesRead -=bytesToWrite;
           adjbuf+=bytesToWrite;
           counter++;
        }
        printf("\n Finished one time loop counter = %d\n",counter);
     }
#if 0
     close(PlayFileID);
#endif
     BKNI_Free(orgbuffer);
StreamToVLC_error_set_scoket:
     close(s);
StreamToVLC_error_open_scoket:
     return;
}

static void StreamToFile(DvrTestHandle dvrtesthandle)
{
     char targetfilename[B_DVR_MAX_FILE_NAME_LENGTH];
/*     char targetfilename[B_DVR_MAX_FILE_NAME_LENGTH]="targetfilename";*/
     FILE *FileFp = NULL;
     unsigned long bufSize = BUFFER_LEN;
     void *buffer;
     ssize_t returnSize;

     fflush(stdin);
     printf("\n Enter file name(targetfilename):");
     scanf("%s", targetfilename);
     if ((FileFp = fopen(targetfilename, "w+" )) == NULL) {
         printf("\n %s: Unable to open a target file %s\n", __FUNCTION__, targetfilename);
     goto StreamToFile_fail_open_file;
     }else
         printf("\n %s: Open a target file = %s\n", __FUNCTION__, targetfilename);

     buffer = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
     dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);

     while(1)
     {
         returnSize = B_DVR_MediaFile_Read(dvrtesthandle->mediaFile,dvrtesthandle->buffer,bufSize);
         if(returnSize != 0)
         {
             fwrite(dvrtesthandle->buffer, 1,returnSize,FileFp);                
         }
       
         if ((returnSize == 0)||((unsigned long)returnSize < bufSize))
         {
         printf("\n End of file read\n");
             break;
         }       
     }

     BKNI_Free(buffer);
     dvrtesthandle->buffer = NULL;
     if(FileFp)
        fclose(FileFp);

StreamToFile_fail_open_file:

     return;
}

static void StreamChunkSet(DvrTestHandle dvrtesthandle)
{

     B_DVR_MediaFileSettings      mediaFileSettings;
     int trickmodeopt;

     if(!dvrtesthandle->mediaFile)
     {
          printf("\n Streaming has not been opened yet!!\n");
          goto StreamChunkSet_error_end;
     }
    
     DvrHNTrickMode_Menu();
     trickmodeopt = 1;
     fflush(stdin);
     scanf("%d", &trickmodeopt);
     mediaFileSettings.fileOperation.streamingOperation.direction = eB_DVR_MediaFileDirectionForward;
     switch((DvrTrickMode_Operation)trickmodeopt)
     {
         case eDvrHNTrickMode_TrickModeFULL:
             mediaFileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeFull;
             break;
         case eDvrHNTrickMode_TrickModeI:
             mediaFileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeIFrame;
             break;
         case eDvrHNTrickMode_TrickModeIP:
             mediaFileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeIPFrame;
             break;
         default:
             mediaFileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeFull;
             break;
     }

     B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);

StreamChunkSet_error_end:
     return;
}

static void StreamOpen(DvrTestHandle dvrtesthandle)
{
     char programName[B_DVR_MAX_FILE_NAME_LENGTH];
     B_DVR_MediaFileOpenMode openMode;
     B_DVR_MediaFilePlayOpenSettings openSettings;
     B_DVR_MediaFileStats mediaFileStats;
     B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
     int rc;

     fflush(stdin);
     printf("\n Enter sub folder name:");
     scanf("%s",openSettings.subDir);
     printf("\n Enter programName:");
     scanf("%s",programName);

     openMode = eB_DVR_MediaFileOpenModeStreaming;
     openSettings.playpumpIndex = 3;
     openSettings.volumeIndex = 0;
     dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
     if(!dvrtesthandle->mediaFile)
     {
         printf("\n error in opening mediaFile\n");
         goto StreamOpen_error_mediaFile_open;
     }
     B_DVR_MediaFile_Stats(dvrtesthandle->mediaFile,&mediaFileStats);
     mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
     mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
     rc = B_DVR_MediaFile_Seek(dvrtesthandle->mediaFile,&mediaFileSeekSettings);
     if(rc!= 0)
     {
          printf("\n Error seeking \n");
     }

StreamOpen_error_mediaFile_open:
     return;
}

static void playbackstart(DvrTestHandle dvrtesthandle)
{
     char programName[B_DVR_MAX_FILE_NAME_LENGTH];
     B_DVR_MediaFileOpenMode openMode;
     B_DVR_MediaFilePlayOpenSettings openSettings;
     B_DVR_MediaFilePlaySettings playSettings;
     int rc;

     fflush(stdin);
     printf("\n Enter the sub folder name for recording to be playedback:");
     scanf("%s",openSettings.subDir);
     printf("\n Enter the recording name:");
     scanf("%s",programName);

     openMode = eB_DVR_MediaFileOpenModePlayback;
     openSettings.volumeIndex = 0;
     openSettings.playpumpIndex = dvrtesthandle->pathIndex;

     openSettings.activeVideoPidIndex[0] = 0;
     openSettings.activeVideoPidIndex[1] = -1;
     openSettings.activeAudioPidIndex = 1;
     openSettings.stcChannel = dvrtesthandle->streamPath[dvrtesthandle->pathIndex].stcChannel;
     openSettings.videoDecoder[0] = dvrtesthandle->streamPath[0].videoDecoder;
     openSettings.videoDecoder[1] = dvrtesthandle->streamPath[1].videoDecoder;
     openSettings.audioDecoder[0] = dvrtesthandle->streamPath[0].audioDecoder;
     openSettings.audioDecoder[1] = dvrtesthandle->streamPath[1].audioDecoder;

     dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
     if(!dvrtesthandle->mediaFile)
     {
         printf("\n error in opening mediaFile\n");
         goto playbacktest_error_mediaFile_open;
     }
     rc = B_DVR_MediaFile_PlayStart(dvrtesthandle->mediaFile,&playSettings);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Start fail!!\n");

playbacktest_error_mediaFile_open:
     return;
}

static void playbackstop(DvrTestHandle dvrtesthandle)
{
     int rc;

     rc = B_DVR_MediaFile_PlayStop(dvrtesthandle->mediaFile);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Stop fail!!\n");
     rc = B_DVR_MediaFile_Close(dvrtesthandle->mediaFile);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Close fail!!\n");
     return;
}

static void playbacktest(DvrTestHandle dvrtesthandle)
{
     PLAYBACKTEST_Operation  PlaybacktestOp;
     int opt;

     while(1)
     {
         DvrPlaybacktest_Menu();
         fflush(stdin);
         opt=0;
         scanf("%d", &opt);
         PlaybacktestOp = (PLAYBACKTEST_Operation)opt; 

         if (!opt)
         {
             break;
         }

         switch(PlaybacktestOp)
         {
             case PLAYBACKTEST_START:
                  playbackstart(dvrtesthandle);
                  break;

             case PLAYBACKTEST_STOP:
                  playbackstop(dvrtesthandle);
                  break;

             default:
                  break;
         }
     }
     return;
}

static void StreamUDPMode(DvrTestHandle dvrtesthandle)
{
     HNUDPMode_Operation  UDPModeOp;
     int opt;

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n Streaming has not been opened yet!!\n");
         goto StreamUDPMode_error_end;
     }

     while(1)
     {
         DvrHNUDP_Menu();
         fflush(stdin);
         opt=0;
         scanf("%d", &opt);
         UDPModeOp = (HNUDPMode_Operation)opt; 

         if (!opt)
         {
             break;
         }

         switch(UDPModeOp)
         {
             case eDvrHNUDPMode_Vlc:
                 StreamToVLC(dvrtesthandle);
                 break;

             case eDvrHNUDPMode_File:
                 StreamToFile(dvrtesthandle);
                 break;

             default:
                 break;
         }
     }

StreamUDPMode_error_end:
     return;
}

static void StreamClose(DvrTestHandle dvrtesthandle)
{
     if(dvrtesthandle->mediaFile)
         B_DVR_MediaFile_Close(g_dvrTest->mediaFile);
     dvrtesthandle->mediaFile = NULL;

     return;
}

static void StreamStatus(DvrTestHandle dvrtesthandle)
{
     BSTD_UNUSED(dvrtesthandle);
     return;
}

static void streammodetest(DvrTestHandle dvrtesthandle)
{
     HNSTREAMING_Operation  HnStreamOp;
     int opt;

     while(1)
     {
         DvrHNStreaming_Menu();
         fflush(stdin);
         opt=0;
         scanf("%d", &opt);
         HnStreamOp = (HNSTREAMING_Operation)opt; 

         if (!opt)
         {
             break;
         }

         switch(HnStreamOp)
         {
             case eDvrHNStream_Open:
                 StreamOpen(dvrtesthandle);
                 break;

             case eDvrHNStream_HN_Chunk:
                 StreamChunkSet(dvrtesthandle);
                 break;

             case eDvrHNStream_UDP_Mode_Opt:
                 StreamUDPMode(dvrtesthandle);
                 break;

             case eDvrHNStream_Close:
                 StreamClose(dvrtesthandle);
                 break;

             case eDvrHNStream_HN_Status:
                 StreamStatus(dvrtesthandle);
                 break;
             default:
                 break;
         } 
     }

     return;
}


int main(void)
{
    int decoderIndex, pathIndex, operation;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    DvrHNTest_Operation  HnTestOp;
    unsigned volumeIndex = 0;

    printf("\n Enter Main function : \n");

    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    g_dvrTest->mediaStorageOpenSettings.registryfile = MEDIA_STORAGE_REGISTRY;
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(&g_dvrTest->mediaStorageOpenSettings);

    if(!g_dvrTest->mediaStorage) 
    {
        printf("\n check registry %s",g_dvrTest->mediaStorageOpenSettings.registryfile);
        return -1;
    }

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
    printf("Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes);
    printf("Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes);

    if (B_DVR_MediaStorage_UnregisterVolume(g_dvrTest->mediaStorage,volumeIndex) >0)
         printf("Unregister error\n");
    else
         printf("Unregister success\n");

    sprintf(g_dvrTest->mediaStorageRegisterSettings.volName,"bcmvol0");
    sprintf(g_dvrTest->mediaStorageRegisterSettings.device,"/dev/sda");
    sprintf(g_dvrTest->mediaStorageRegisterSettings.media,"/dev/sda1");
    sprintf(g_dvrTest->mediaStorageRegisterSettings.navigation,"/dev/sda2");
    sprintf(g_dvrTest->mediaStorageRegisterSettings.metadata,"/dev/sda3");
    if (B_DVR_MediaStorage_RegisterVolume(g_dvrTest->mediaStorage,&g_dvrTest->mediaStorageRegisterSettings,&volumeIndex) >0)
         printf("\n Register media volume error\n");
    else
         printf("\n Register media volume success vol:%d\n",volumeIndex);

    B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex, "/mnt/");
    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

    if (!mediaStorageStatus.volumeInfo[0].registered)   
    {
        volumeIndex=0;
        printf("\n Enter volume index : ");
        scanf("%u", &volumeIndex);
        B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex, "/mnt/");
        B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
        printf("\n Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes);
        printf("\n Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes);
    }
    printf("\n Volumeindex = %d",volumeIndex);
    printf("\n %s", mediaStorageStatus.volumeInfo[volumeIndex].mounted?"Mounted":"Not Mounted");
    printf("\n\tname: %s", mediaStorageStatus.volumeInfo[volumeIndex].name);
    printf("\n\tmedia: dev %s", mediaStorageStatus.volumeInfo[volumeIndex].media.devName);
    printf("\n\tnavigation dev: %s", mediaStorageStatus.volumeInfo[volumeIndex].navigation.devName);
    printf("\n\tmetadata dev: %s", mediaStorageStatus.volumeInfo[volumeIndex].metadata.devName);
    printf("\n\tmedia segment size: %d", mediaStorageStatus.volumeInfo[volumeIndex].mediaSegmentSize);
    printf("\n\tmediaStorageStatus.numMountedVolumes: %d", mediaStorageStatus.numMountedVolumes);
    printf("\n\tB_DVR_MediaStorage_Handle: =%s\n",(char *)g_dvrTest->mediaStorage);

    
    g_dvrTest->streamPath[0].currentChannel = 0;
    if(NEXUS_NUM_VIDEO_DECODERS > 1)
    g_dvrTest->streamPath[1].currentChannel= 1;

    B_Os_Init();

    NEXUS_Platform_GetDefaultSettings(&(g_dvrTest->platformSettings));
    g_dvrTest->platformSettings.openFrontend = false;

    NEXUS_Platform_Init(&(g_dvrTest->platformSettings));

    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);

    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));

    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);

    for(decoderIndex=0;decoderIndex < 2; decoderIndex++)
    {
        DvrTest_StreamPathOpen(decoderIndex);
    }

    g_dvrTest->dvrManager = B_DVR_Manager_Init(NULL);
    if(!g_dvrTest->dvrManager)
    {
        printf("\n Error in opening the dvr manager\n");
    } 
    
    B_DVR_Manager_CreateMediaNodeList(g_dvrTest->dvrManager,0);

    while(1)
    { 
        DvrHNTest_Menu();
        fflush(stdin);
        operation=0;
        scanf("%d", &operation);
        HnTestOp = (DvrHNTest_Operation)operation; 
        if(HnTestOp > eDvrHNTest_OperationQuit && HnTestOp < eDvrHNTest_OperationMax)
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
        
        g_dvrTest->pathIndex = pathIndex;
        switch(HnTestOp)
        {
           case  eDvrHNTest_Streamingmode:
              streammodetest(g_dvrTest);
              break;

           case  eDvrHNTest_Playbackmode:
              playbacktest(g_dvrTest);
              break;

           case  eDvrHNTest_Recordingmode:
              recordmodetest(g_dvrTest);
              break;
          
           default:
              printf("Invalid DVR Operation - Select the operation again");
              break;
        }  
    }
    
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();
    return 0;
}
