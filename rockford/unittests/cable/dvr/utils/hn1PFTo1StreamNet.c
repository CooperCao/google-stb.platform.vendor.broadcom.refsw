#include "dvr_test.h"
BDBG_MODULE(hn1PFTo1StreamNet);

#define MAX_TSBPATH 1
static int pathkey[MAX_TSBPATH] = {0};
static char hnpfstreamingtxt[] = "hnpfstreaming.txt";

B_EventHandle MultipleStreamReady;

static void PFInitHnMulStreamingpara(void);
static void PFCheckMulStreamTestName(char *buffer);
static int  PFHnMulStreamingLoadTxt(void);
static int  PFInitMulStreamUDPSocket(void);
static void PF_stream_UDP(int pathIndex);
static void PF_stream_MulThread_Created(int pathIndex);
static void PF_stream_MulThread_Active(int pathIndex);
static void PF_stream_MulThread_Destroy(int pathIndex);

void PFInitHnMulStreamingpara(void)
{
    int  pathIndex;
   
    for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
    {   
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.dstip[0] = '\0';   
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName[0] = '\0';    
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum[0] = '\0';    
    }
}

void PFCheckMulStreamTestName(char *buffer)
{
    char testpara[512];
    char strpara[512];
    int  paravalue = 0;
    int  n = 0, m = 0;
    int  pathIndex;

    n = sscanf(buffer, "%s\n", testpara);
    if (n > 0)
    {
       if(strncmp(testpara, "DestpIP",7) == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             paravalue = atoi(&(testpara[7]));
             strcpy(g_dvrTest->streamPath[paravalue].hnMulStreamPara.dstip,strpara);
             printf("hnMulStreamPara.dstip = %s index=%d \n",g_dvrTest->streamPath[paravalue].hnMulStreamPara.dstip,paravalue);
          }
       }
       else if(strcmp(testpara, "LocalIFName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
             {
                strcpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName,strpara);
                printf("hnMulStreamPara.interfaceName = %s index=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName,pathIndex);
             }
          }
       }
       else if(strcmp(testpara, "PortNum") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
             {
                paravalue = atoi(strpara);
                if(pathIndex != 0)
                   paravalue += 2;
                BKNI_Snprintf(strpara,sizeof(strpara),"%d",paravalue);
                strcpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum,strpara);
                printf("hnMulStreamPara.portnum = %s index=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum,pathIndex);
             }
          }
       }
       else if(strcmp(testpara, "RecoredProgName0") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[0].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s!!\n",g_dvrTest->streamPath[0].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName1") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[1].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=1\n",g_dvrTest->streamPath[1].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName2") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[2].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=2\n",g_dvrTest->streamPath[2].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName3") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[3].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=3\n",g_dvrTest->streamPath[3].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName4") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[4].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=4\n",g_dvrTest->streamPath[4].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName5") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[5].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=5\n",g_dvrTest->streamPath[5].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName6") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[6].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=6\n",g_dvrTest->streamPath[6].hnMulStreamPara.RecoredProgName);
          }
       }
       else if(strcmp(testpara, "RecoredProgName7") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             strcpy(g_dvrTest->streamPath[7].hnMulStreamPara.RecoredProgName,strpara);
             printf("hnMulStreamPara.RecoredProgName = %s index=7\n",g_dvrTest->streamPath[7].hnMulStreamPara.RecoredProgName);
          }
       }
       else
          BDBG_WRN(("Parameter not recognized: %s\n", testpara));
    }
}

int PFHnMulStreamingLoadTxt(void)
{
    char buffer[512];
    FILE *fd;
    int rc = 0;

    printf("\n Load Hn Streaming input txt file \n");
    fd= fopen(hnpfstreamingtxt, "r");
    if(fd == NULL)
    {
        BDBG_MSG(("No input file found.\n"));
        rc = -1;
        return rc;
    }
	
    while (fgets(buffer, 512, fd) != NULL)
    {
      if (buffer[0] != '#')
      {
         PFCheckMulStreamTestName(buffer);
      }
    }
    fclose(fd);
    return rc;
}

int PFInitMulStreamUDPSocket(void)
{
   int pathIndex,rc=0;
   int  errorcode = 0;

   printf("\n Enter InitUDPSocket>>>> \n");  
   for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
   {
   
     if ((g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s=socket(AF_INET, SOCK_DGRAM, 0))<0)
     {
         printf(" socket open failed \n");   
         rc = -1;
         goto EndofInitUDPSocket;
     }
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_family = AF_INET;
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_port = htons(atoi(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum));
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_addr.s_addr = inet_addr(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.dstip);
     strncpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr.ifr_name, g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName, 
             sizeof(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr.ifr_name)-1);

     if ((errorcode = setsockopt(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr), 
          sizeof(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr))) < 0 )
     {
        printf("Set Bingparater error = %d!\n",errorcode);
        rc = -1;
        goto EndofInitUDPSocket;
     }
     else
     {
        printf("Set Bingparater Success = %d!\n",errorcode);
     }
   }
EndofInitUDPSocket:
   printf("\n Exit InitUDPSocket<<<<\n");  
   return rc;
}

void PF_stream_MulThread_Created(int pathIndex)
{
    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
    off_t returnOffset;
    B_DVR_MediaFileOpenMode openMode;
    B_DVR_MediaFilePlayOpenSettings openSettings;
    B_DVR_MediaFileSettings fileSettings,orgFileSettings;
    B_DVR_MediaFileStreamFrameResource frameResource;
    B_DVR_MediaFileStreamFrameRate frameRate;
    int rc =0;
 

    printf("Enter g_dvrTest->multipleStreamCreated>>>> pathindex=%d\n",pathIndex);

    openMode = eB_DVR_MediaFileOpenModeStreaming;
    openSettings.playpumpIndex = pathIndex;
    openSettings.volumeIndex = 0;
    sprintf(openSettings.subDir,"tsbConv");
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].programName,
            sizeof(g_dvrTest->streamPath[pathIndex].programName),
            "%s",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.RecoredProgName);
    printf("ProgramName:%s\n",g_dvrTest->streamPath[pathIndex].programName);

    g_dvrTest->streamPath[pathIndex].mediaFile = B_DVR_MediaFile_Open(g_dvrTest->streamPath[pathIndex].programName,openMode,&openSettings);
    if(!g_dvrTest->streamPath[pathIndex].mediaFile)
    {
        printf("Error in opening mediaFile pathindex= %d",pathIndex);
    }

    B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));
    B_DVR_MediaFile_GetSettings(g_dvrTest->streamPath[pathIndex].mediaFile,&orgFileSettings);
    BKNI_Memcpy((void *)&fileSettings,(void *)&orgFileSettings,sizeof(B_DVR_MediaFileSettings));

    fileSettings.fileOperation.streamingOperation.direction = eB_DVR_FileReadDirectionForward;
    fileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeIFrame;
    B_DVR_MediaFile_SetSettings(g_dvrTest->streamPath[pathIndex].mediaFile,&fileSettings);

    g_dvrTest->streamPath[pathIndex].readOffset = mediaFileSeekSettings.mediaSeek.offset = g_dvrTest->streamPath[pathIndex].mediaFileStats.startOffset;
    mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
    returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->streamPath[pathIndex].mediaFile,&mediaFileSeekSettings);
    if(returnOffset < 0)
    {
        printf("Error seeking \n");
    }

    rc = B_DVR_MediaFile_StreamIPFrameRate(g_dvrTest->streamPath[pathIndex].mediaFile,&frameRate);
    if(rc != 0)
        printf("Error to get streamFrameRate \n");

    B_DVR_MediaFile_SetSettings(g_dvrTest->streamPath[pathIndex].mediaFile,&orgFileSettings);

    B_DVR_MediaFile_GetStreamInfo(g_dvrTest->streamPath[pathIndex].mediaFile,&frameResource);
    g_dvrTest->streamPath[pathIndex].timerPeriod = frameResource.FrameRateptsdelta;

    g_dvrTest->streamPath[pathIndex].bufSize = ((frameResource.FramesGOPBuf)%(PACKET_CHUNK))?((PACKET_CHUNK)*(frameResource.FramesGOPBuf/(PACKET_CHUNK)+1)):(frameResource.FramesGOPBuf);
    g_dvrTest->streamPath[pathIndex].orgbuffer = BKNI_Malloc(g_dvrTest->streamPath[pathIndex].bufSize + B_DVR_IO_BLOCK_SIZE);
    g_dvrTest->streamPath[pathIndex].buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)g_dvrTest->streamPath[pathIndex].orgbuffer);

    if(!g_dvrTest->streamPath[pathIndex].orgbuffer)
    {
        printf("Error_buf_alloc\n");
    }
    if((g_dvrTest->streamPath[pathIndex].bufSize/1024) > 1450)
    {
        printf("Buffer size > 1.4MB!!! \n");
    }

    printf("g_dvrTest->multipleStreamCreated:: timerPeriod=%u(ms)\n",g_dvrTest->streamPath[pathIndex].timerPeriod);
    printf("g_dvrTest->multipleStreamCreated:: FramesGOPBuf=%lx(bytes)\n",frameResource.FramesGOPBuf);
    printf("g_dvrTest->multipleStreamCreated:: bufSize=%d\n",g_dvrTest->streamPath[pathIndex].bufSize);

    g_dvrTest->streamPath[pathIndex].readOffset = mediaFileSeekSettings.mediaSeek.offset = g_dvrTest->streamPath[pathIndex].mediaFileStats.startOffset;
    mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
    returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->streamPath[pathIndex].mediaFile,&mediaFileSeekSettings);
    if(returnOffset < 0)
    {
        printf("Error seeking \n");
    }

    printf("Exit g_dvrTest->multipleStreamFCreate<<<\n");
}


void PF_stream_MulThread_Destroy(int pathIndex)
{
    BKNI_Free(g_dvrTest->streamPath[pathIndex].orgbuffer);
    B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].mediaFile);
    B_Scheduler_CancelTimer(g_dvrTest->PFscheduler,g_dvrTest->streamPath[pathIndex].streamTimer);
}

void PF_stream_UDP(int pathIndex)
{

/*    printf("Enter stream_UDP>>> index=%d\n",pathIndex);*/

    if (g_dvrTest->streamPath[pathIndex].chunkFlag == HnChunkBufFirstHalf)
    {   
       g_dvrTest->streamPath[pathIndex].adjbuf = g_dvrTest->streamPath[pathIndex].buffer;
       g_dvrTest->streamPath[pathIndex].chunkFlag = HnChunkBufSecondHalf;
    }else{
       g_dvrTest->streamPath[pathIndex].adjbuf = g_dvrTest->streamPath[pathIndex].buffer;
       g_dvrTest->streamPath[pathIndex].adjbuf += (g_dvrTest->streamPath[pathIndex].returnSize)/2;
       g_dvrTest->streamPath[pathIndex].chunkFlag = HnChunkBufEndBuf;
    }

    if(g_dvrTest->streamPath[pathIndex].returnSize != g_dvrTest->streamPath[pathIndex].bufSize )
    {
        printf("\n Byteread != PACKET_CHUNK*CHUNK_ELEMENT =%d\n",g_dvrTest->streamPath[pathIndex].returnSize);
    }

    g_dvrTest->streamPath[pathIndex].tempSize = g_dvrTest->streamPath[pathIndex].returnSize/2;

    while(g_dvrTest->streamPath[pathIndex].tempSize!= 0)
    {
       if (g_dvrTest->streamPath[pathIndex].tempSize <= PACKET_CHUNK)
           g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite = g_dvrTest->streamPath[pathIndex].tempSize;
       else
           g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite = PACKET_CHUNK;

       memset(&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0]), 0, sizeof(struct iovec)*SOCKET_IO_VECTOR_ELEMENT);
       memset(&g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg, 0, sizeof(struct msghdr));
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0].iov_base = g_dvrTest->streamPath[pathIndex].adjbuf;
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0].iov_len = g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite; 

       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_name = (struct sockaddr *)&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_namelen = sizeof(struct sockaddr_in);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_iov = &(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0]);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_iovlen = SOCKET_IO_VECTOR_ELEMENT;
       if ((g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten = sendmsg(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s, 
            &(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg), 0)) <= 0) 
       {
           printf("\n ERROR writing to socket\n");
           g_dvrTest->streamPath[pathIndex].returnSize -= g_dvrTest->streamPath[pathIndex].tempSize;
           goto StreamToVLC_error_sendmsg;
       }
       if(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten != g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite)
       {
           printf("\n !!!!!ERROR writing to socket bytewritten =%d, bytesToWrite=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten,
                   g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite);
       }
       g_dvrTest->streamPath[pathIndex].tempSize -=g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite;
       g_dvrTest->streamPath[pathIndex].adjbuf+=g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite;
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.counter++;
    }
StreamToVLC_error_sendmsg:
/*    printf("Exit stream_UDP<<< index=%d\n",pathIndex);*/

    return;
}

void PFStreamAction(void *PathContext)
{
    int pathIndex=*((int*)PathContext);
    unsigned int timer = 0;
/*    printf("\nEnter PFStreamAction pathIndex = %d\n",pathIndex);*/
    gettimeofday(&(g_dvrTest->streamPath[pathIndex].startTime), NULL);
    g_dvrTest->streamPath[pathIndex].dtPeriod = (unsigned int)((1000*(g_dvrTest->streamPath[pathIndex].startTime.tv_sec - g_dvrTest->streamPath[pathIndex].testTime.tv_sec)) +
                                                                   (g_dvrTest->streamPath[pathIndex].startTime.tv_usec - g_dvrTest->streamPath[pathIndex].testTime.tv_usec)/1000);

/*    printf("g_dvrTest->streamPath[pathIndex].TimerExperiodTime =%u\n",g_dvrTest->streamPath[pathIndex].dtPeriod);*/
    PF_stream_UDP(pathIndex);

    if(g_dvrTest->streamPath[pathIndex].chunkFlag == HnChunkBufEndBuf)
    {
       g_dvrTest->streamPath[pathIndex].readSize += g_dvrTest->streamPath[pathIndex].returnSize;
       g_dvrTest->streamPath[pathIndex].readOffset+=g_dvrTest->streamPath[pathIndex].returnSize;
    }

    if((g_dvrTest->streamPath[pathIndex].readOffset + g_dvrTest->streamPath[pathIndex].bufSize) < g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset)
    {
        if(g_dvrTest->streamPath[pathIndex].chunkFlag == HnChunkBufEndBuf)
        {
            g_dvrTest->streamPath[pathIndex].returnSize = B_DVR_MediaFile_Read(g_dvrTest->streamPath[pathIndex].mediaFile,g_dvrTest->streamPath[pathIndex].buffer,g_dvrTest->streamPath[pathIndex].bufSize);

            if(g_dvrTest->streamPath[pathIndex].returnSize <= 0)
            {
                printf("readsize <=0, skip start timer and close file !\n");
                g_dvrTest->streamPath[pathIndex].returnSize = 0;
                goto endofPFStreamAction;
            }
            g_dvrTest->streamPath[pathIndex].chunkFlag = HnChunkBufFirstHalf;
        }

        gettimeofday(&(g_dvrTest->streamPath[pathIndex].endTime), NULL);
        g_dvrTest->streamPath[pathIndex].dtPeriod = (unsigned int)((1000*(g_dvrTest->streamPath[pathIndex].endTime.tv_sec - g_dvrTest->streamPath[pathIndex].startTime.tv_sec)) +
                                                                   (g_dvrTest->streamPath[pathIndex].endTime.tv_usec - g_dvrTest->streamPath[pathIndex].startTime.tv_usec)/1000);

/*        printf("g_dvrTest->streamPath[pathIndex].dtPeriod =%u\n",g_dvrTest->streamPath[pathIndex].dtPeriod);*/
        timer = (g_dvrTest->streamPath[pathIndex].timerPeriod/2 - g_dvrTest->streamPath[pathIndex].dtPeriod);
        g_dvrTest->streamPath[pathIndex].streamTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
                                                                              g_dvrTest->PFschedulerMutex,
                                                                              timer,
                                                                              PFStreamAction,
                                                                              (void*)PathContext);
        gettimeofday(&(g_dvrTest->streamPath[pathIndex].testTime), NULL);
    }
    else
    {
        close(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s);
        B_Mutex_Lock(g_dvrTest->glbstreamMutex);
        g_dvrTest->multipleStreamFlag &= ~(1<<pathIndex); 
        printf("End of PFStreamAction index= %d multipleStreamFlag= %x\n\n",pathIndex,g_dvrTest->multipleStreamFlag);
        B_Event_Set(MultipleStreamReady);
        B_Mutex_Unlock(g_dvrTest->glbstreamMutex);
    }
endofPFStreamAction:    
/*    printf("Exit PFStreamAction pathIndex = %d\n",pathIndex);*/
    return;
}

void PF_stream_MulThread_Active(int pathIndex)
{
    printf("Enter g_dvrTest->MulThread_Active >>>>MultipleThreadFlag = %x\n",g_dvrTest->multipleStreamFlag);

    if((g_dvrTest->streamPath[pathIndex].readOffset + g_dvrTest->streamPath[pathIndex].bufSize) < g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset)
    {
        g_dvrTest->streamPath[pathIndex].returnSize = B_DVR_MediaFile_Read(g_dvrTest->streamPath[pathIndex].mediaFile,g_dvrTest->streamPath[pathIndex].buffer,g_dvrTest->streamPath[pathIndex].bufSize);

        if(g_dvrTest->streamPath[pathIndex].returnSize <= 0)
        {
             printf("readsize <=0, skip start timer and close file !\n");
             goto endofMulThreadActive;
        }
        g_dvrTest->streamPath[pathIndex].streamTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
                                                                              g_dvrTest->PFschedulerMutex,
                                                                              g_dvrTest->streamPath[pathIndex].timerPeriod,
                                                                              PFStreamAction,
                                                                              (void*)&pathIndex);
        g_dvrTest->streamPath[pathIndex].chunkFlag = HnChunkBufFirstHalf;
        g_dvrTest->multipleStreamFlag |= (1<<pathIndex); 
    }
    else
    {
         printf("Readsize is out of file size range!!\n");
         close(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s);
         B_Mutex_Lock(g_dvrTest->glbstreamMutex);
         g_dvrTest->multipleStreamFlag &= ~(1<<pathIndex); 
         printf("Exit stream_Action<<<< index= %d multipleStreamFlag= %x\n\n",pathIndex,g_dvrTest->multipleStreamFlag);
         B_Event_Set(MultipleStreamReady);
         B_Mutex_Unlock(g_dvrTest->glbstreamMutex);
    }

endofMulThreadActive:
    printf("Exit g_dvrTest->MulThread_Active <<<<<MultipleThreadFlag = %x\n",g_dvrTest->multipleStreamFlag);

}

static void PF_Test_Scheduler(void *PathContext)
{
    BSTD_UNUSED(PathContext);
    printf("Starting Scheduler 0x%08x\n", (unsigned)g_dvrTest->PFscheduler);
    B_Scheduler_Run(g_dvrTest->PFscheduler);
    return;
}

void hn1PFTo1StreamNetTest(CuTest * tc)
{
    int index,returnValue; 

    PFInitHnMulStreamingpara();
    returnValue = PFHnMulStreamingLoadTxt();
    if (returnValue != 0)
    {
        printf("\n Missing input file!!!\n");
        return;
    }

    returnValue = PFInitMulStreamUDPSocket();
    if (returnValue != 0)
    {
        printf("\n Init UDP Socket Error!!!\n");
        return;
    }

    g_dvrTest->PFschedulerMutex = B_Mutex_Create(NULL);
    if( !g_dvrTest->PFschedulerMutex)
    {
        printf("\n PFschedulerMutex create error");
        return;
    }

    g_dvrTest->PFscheduler = B_Scheduler_Create(NULL);
    if(!g_dvrTest->PFscheduler)
    {
        printf("\n PFscheduler create error");
        B_Mutex_Destroy(g_dvrTest->PFschedulerMutex);
        return;
    }

    printf("Before g_dvrTest->PFscheduler\n");

    g_dvrTest->PFthread = B_Thread_Create("PF_Test Scheduler", PF_Test_Scheduler,NULL, NULL);
    if(!g_dvrTest->PFthread)
    {
        printf("\n PFscheduler PFthread create error");
        B_Mutex_Destroy(g_dvrTest->PFschedulerMutex);
        B_Scheduler_Destroy(g_dvrTest->PFscheduler);
        return;
    }

    printf("After g_dvrTest->PFscheduler\n");

    g_dvrTest->multipleStreamFlag = 0;

    MultipleStreamReady = B_Event_Create(NULL);

    printf("configured stream path = %d\n", MAX_TSBPATH);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("Create Mutiple thread index = %d \n", index);
        PF_stream_MulThread_Created(pathkey[index]);
    }

    g_dvrTest->glbstreamMutex = B_Mutex_Create(NULL);

    B_Mutex_Lock(g_dvrTest->glbstreamMutex);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("Active Mutiple thread index = %d \n", index);
        PF_stream_MulThread_Active(pathkey[index]);
    }

    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

cntWaitEvent:
    while(B_Event_Wait(MultipleStreamReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    if(g_dvrTest->multipleStreamFlag != 0)
    {
        printf("Need to reset MultipleStreamReady --mutiple thread flag=%x\n",g_dvrTest->multipleStreamFlag);
        B_Event_Reset(MultipleStreamReady);
        goto cntWaitEvent;
    }

    printf("End of mutiple thread flag=%d\n",g_dvrTest->multipleStreamFlag);
    B_Mutex_Lock(g_dvrTest->glbstreamMutex);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("Destroy Mutiple thread index = %d \n", index);
        PF_stream_MulThread_Destroy(pathkey[index]);
    }

    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

    B_Mutex_Destroy(g_dvrTest->glbstreamMutex);

    B_Event_Destroy(MultipleStreamReady);
    
    printf("B_Scheduler_Stop \n");
    B_Scheduler_Stop(g_dvrTest->PFscheduler);

    printf("B_Scheduler_Destroy \n");
    B_Scheduler_Destroy(g_dvrTest->PFscheduler);

    printf("B_Scheduler_PFschedulerMutex \n");
    B_Mutex_Destroy(g_dvrTest->PFschedulerMutex);

    B_Thread_Destroy(g_dvrTest->PFthread);

    CuAssertTrue(tc, returnValue==B_DVR_SUCCESS);

    printf("\n End of hn1PFTo1StreamNetTest");

}

