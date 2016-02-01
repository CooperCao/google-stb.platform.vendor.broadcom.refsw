#include "dvr_test.h"
BDBG_MODULE(hnstreaming);

const char *programName = "perm_000"; 
static char hnstreamingtxt[] = "hnstreaming.txt";

static double difftime1(
    struct timeval *start, 
    struct timeval *stop) 
{
    double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
    return dt;
}

static int HnCalSkipStepForIPFrame(DvrTestHandle dvrtesthandle)
{
   int              rc = 0;
   unsigned long    calValue;

   dvrtesthandle->skipStepIPFrame = 0;
   dvrtesthandle->dupNumIPFrame = 0;
   dvrtesthandle->usedNumIPFrame = 0;
   if ((dvrtesthandle->playScale.denominator == 1) &&
      (dvrtesthandle->playScale.numerator >= dvrtesthandle->playScale.denominator))
   {

      if(dvrtesthandle->streamFrameRate.sfRateNumerator > 0)
      {
         calValue = (dvrtesthandle->playScale.numerator)*(dvrtesthandle->streamFrameRate.sfRateNumerator);
         if(calValue >= (dvrtesthandle->frameSymbolRate))
         {
             calValue -= (dvrtesthandle->frameSymbolRate);
             dvrtesthandle->skipStepIPFrame = calValue%(dvrtesthandle->frameSymbolRate)?
                                            (calValue/(dvrtesthandle->frameSymbolRate)+1):(calValue/(dvrtesthandle->frameSymbolRate));
         }
         else
         {
             if(calValue >= (dvrtesthandle->frameSymbolRate/2))
                dvrtesthandle->dupNumIPFrame = 1;   /*dumplicate one time*/
             else
             {
                if (calValue >= (dvrtesthandle->frameSymbolRate/4))
                   dvrtesthandle->dupNumIPFrame = 3; 
                else
                   dvrtesthandle->dupNumIPFrame = 7; 
             }
             dvrtesthandle->usedNumIPFrame = dvrtesthandle->dupNumIPFrame;
         }
       }
       else
       {
         if (dvrtesthandle->streamFrameRate.sfRateDenominator != 0)
         {
            calValue = dvrtesthandle->frameSymbolRate * (dvrtesthandle->streamFrameRate.sfRateDenominator);
            if(calValue >= (unsigned)dvrtesthandle->playScale.numerator)
            {
               dvrtesthandle->dupNumIPFrame = (calValue/dvrtesthandle->playScale.numerator) - 1;
            }
            else
            {
               dvrtesthandle->skipStepIPFrame = ((dvrtesthandle->playScale.numerator - calValue)%calValue)?
                                             (((dvrtesthandle->playScale.numerator - calValue)/calValue)+1):((dvrtesthandle->playScale.numerator - calValue)/calValue);
            }
            dvrtesthandle->usedNumIPFrame = dvrtesthandle->dupNumIPFrame;
         }
       }
   }
   else
   {
      rc = -1;
   }
   return rc;
}

static int StreamToVLC(DvrTestHandle dvrtesthandle)
{
     struct sockaddr_in si_other;
     struct msghdr msg;
     struct iovec  iovec_set[SOCKET_IO_VECTOR_ELEMENT];
     struct ifreq ifr;
#if 0
     char PlayFileName[B_DVR_MAX_FILE_NAME_LENGTH]="seg001120.ts";
/*     char PlayFileName[B_DVR_MAX_FILE_NAME_LENGTH]="trp_008_spiderman_lotr_oceans11_480i_q64.mpg";*/
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
     unsigned long bufSize = PACKET_CHUNK*CHUNK_ELEMENT+B_DVR_IO_BLOCK_SIZE*2; 
     char *orgbuffer,*adjbuf;
     double rate = 19.4;
     struct timeval start, stop;
     double dt=0, maxrate = 19.4;
     off_t  streambytes=0;
#endif
     int rc = 0;
     unsigned long  skipStep,dumpStep,interDumpValue;

     strcpy(dstip,dvrtesthandle->hnStreamPara.dstip);
     strcpy(interfaceName,dvrtesthandle->hnStreamPara.interfaceName);
     strcpy(portnum,dvrtesthandle->hnStreamPara.portnum);
     if ((s=socket(AF_INET, SOCK_DGRAM, 0))<0)
     {
         printf("\n socket open failed \n");   
         rc = -1;
         goto StreamToVLC_error_open_scoket;
     }
     si_other.sin_family = AF_INET;
     si_other.sin_port = htons(atoi(portnum));
     si_other.sin_addr.s_addr = inet_addr(dstip);
     strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);
     if ((errorcode = setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0 )
     {
        printf("\n Set Bingparater error = %d!\n",errorcode);
        rc = -1;
        goto StreamToVLC_error_set_scoket;
     }
     else
     {
        printf("\n Set Bingparater Success = %d!\n",errorcode);
     }

     if(dvrtesthandle->DecryptDrmHandle)
     {
         NEXUS_Memory_Allocate(bufSize+4, NULL, (void *)&orgbuffer);
         dvrtesthandle->buffer =(void *)((unsigned long)orgbuffer & 0xfffffffc);

     }else{
         orgbuffer  = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
         dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)orgbuffer);
     }
#if 0
     if ((PlayFileID = open(PlayFileName, O_RDONLY,0666)) < 0)
     {
         printf("\n Unable to open %s",PlayFileName);
         goto StreamToVLC_error_set_scoket;
     }
#endif
     gettimeofday(&start, NULL);
     maxrate = 5000;
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

           memset(&msg, 0, sizeof(struct msghdr));
           memset(&(iovec_set[0]), 0, sizeof(struct iovec)*SOCKET_IO_VECTOR_ELEMENT);
           iovec_set[0].iov_base = adjbuf;
           iovec_set[0].iov_len = bytesToWrite; 
           msg.msg_iovlen = SOCKET_IO_VECTOR_ELEMENT;
           msg.msg_name = (struct sockaddr *)&si_other;
           msg.msg_namelen = sizeof(struct sockaddr_in);
           msg.msg_iov = &(iovec_set[0]);
           if ((bytesWritten = sendmsg(s, &msg, 0)) <= 0) 
           {
               printf("\n ERROR writing to socket\n");
               rc = -1;
               goto StreamToVLC_error_sendmsg;
           }
           bytesRead -=bytesToWrite;
           adjbuf+=bytesToWrite;
           counter++;
        }

        usleep(10);
        streambytes += PACKET_CHUNK*CHUNK_ELEMENT;
        gettimeofday(&stop, NULL);
        dt = difftime1(&start, &stop);
        rate = (8.*PACKET_CHUNK*CHUNK_ELEMENT*1000)/dt;
        while (rate > maxrate) {
             usleep(10000);
             gettimeofday(&stop, NULL);
             dt = difftime1(&start,&stop);
             rate = (8.*PACKET_CHUNK*CHUNK_ELEMENT*1000)/dt;
        }
        memcpy(&start,&stop,sizeof(struct timeval));

        if((dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIFrame)||
           (dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIPFrame))
         {
            skipStep = dvrtesthandle->skipStepIPFrame;
            if(dvrtesthandle->interDumpFlag == 1)
		  dumpStep = dvrtesthandle->interDumpPlusDupNumIPFrame;
            else
		  dumpStep = dvrtesthandle->dupNumIPFrame;

            if(dumpStep != 0)
            {
               if(dvrtesthandle->usedNumIPFrame == 0)
               {
                  dumpStep = 0;
                  dvrtesthandle->interDumpFlag = 0;
                  dvrtesthandle->interDumpPlusDupNumIPFrame = 0;
                  dvrtesthandle->usedNumIPFrame = dvrtesthandle->dupNumIPFrame;
               }
               else
                  dvrtesthandle->usedNumIPFrame--;
            }
            printf("\n Skipstep= %ld dumpStep = %ld\n",skipStep,dumpStep);
            rc = B_DVR_MediaFile_TrickModeIPFrameSkip(dvrtesthandle->mediaFile,skipStep,dumpStep,(unsigned *)&interDumpValue);
            if(rc != 0)
            {
                printf("\n Failed on TrickmodeIPFrameSkip\n");
                break;
            }
            if(interDumpValue != 0)
            {
               dvrtesthandle->interDumpPlusDupNumIPFrame = dvrtesthandle->dupNumIPFrame*interDumpValue;
               dvrtesthandle->usedNumIPFrame = dvrtesthandle->interDumpPlusDupNumIPFrame-1;
               dvrtesthandle->interDumpFlag = 1;
            }
         }
     }
#if 0
     close(PlayFileID);
#endif
StreamToVLC_error_sendmsg:
     if(dvrtesthandle->DecryptDrmHandle)
         NEXUS_Memory_Free(orgbuffer);
     else
         BKNI_Free(orgbuffer);
StreamToVLC_error_set_scoket:
     close(s);
StreamToVLC_error_open_scoket:
     return rc;
}

static int StreamToFile(DvrTestHandle dvrtesthandle)
{
     char targetfilename[B_DVR_MAX_FILE_NAME_LENGTH];
     FILE *FileFp = NULL;
     unsigned long bufSize = BUFFER_LEN;
     void *buffer;
     ssize_t returnSize;
     int rc = 0;
     unsigned long  skipStep,dumpStep,interDumpValue;

     strcpy(targetfilename,dvrtesthandle->hnStreamPara.targetfilename);
     if ((FileFp = fopen(targetfilename, "w+" )) == NULL) {
         printf("\n %s: Unable to open a target file %s\n", __FUNCTION__, targetfilename);
         rc = -1;
         goto StreamToFile_fail_open_file;
     }else
         printf("\n %s: Open a target file = %s\n", __FUNCTION__, targetfilename);

     if(dvrtesthandle->DecryptDrmHandle)
     {
         bufSize = 188*128;
         NEXUS_Memory_Allocate(bufSize+4, NULL, (void *)&buffer);
         dvrtesthandle->buffer =(void *)((unsigned long)buffer & 0xfffffffc);
     }else{
         buffer = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
         dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);
     }
     while(1)
     {
         returnSize = B_DVR_MediaFile_Read(dvrtesthandle->mediaFile,dvrtesthandle->buffer,bufSize);
         if(returnSize != 0)
         {
             fwrite(dvrtesthandle->buffer, 1,returnSize,FileFp);                
         }

         if((dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIFrame)||
            (dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIPFrame))
         {
             if (returnSize == 0)
             {
                printf("\n End of file read\n");
                break;
             }
         }
         else
         {      
             if ((returnSize == 0)||((unsigned long)returnSize < bufSize))
             {
                printf("\n End of file read\n");
                break;
             }
         }       
         if((dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIFrame)||
            (dvrtesthandle->frameTypeInTrickMode == eB_DVR_MediaFileReadModeIPFrame))
         {
            skipStep = dvrtesthandle->skipStepIPFrame;
            if(dvrtesthandle->interDumpFlag == 1)
		  dumpStep = dvrtesthandle->interDumpPlusDupNumIPFrame;
            else
		  dumpStep = dvrtesthandle->dupNumIPFrame;

            if(dumpStep != 0)
            {
               if(dvrtesthandle->usedNumIPFrame == 0)
               {
                  dumpStep = 0;
                  dvrtesthandle->interDumpFlag = 0;
                  dvrtesthandle->interDumpPlusDupNumIPFrame = 0;
                  dvrtesthandle->usedNumIPFrame = dvrtesthandle->dupNumIPFrame;
               }
               else
                  dvrtesthandle->usedNumIPFrame--;
            }
            printf("\n Skipstep= %ld dumpStep = %ld\n",skipStep,dumpStep);
            rc = B_DVR_MediaFile_TrickModeIPFrameSkip(dvrtesthandle->mediaFile,skipStep,dumpStep,(unsigned *)&interDumpValue);
            if(rc != 0)
            {
                printf("\n Failed on TrickmodeIPFrameSkip\n");
                break;
            }
            if(interDumpValue != 0)
            {
               dvrtesthandle->interDumpPlusDupNumIPFrame = dvrtesthandle->dupNumIPFrame*interDumpValue;
               dvrtesthandle->usedNumIPFrame = dvrtesthandle->interDumpPlusDupNumIPFrame-1;
               dvrtesthandle->interDumpFlag = 1;
            }
         }
     }
     if(dvrtesthandle->DecryptDrmHandle)
         NEXUS_Memory_Free(buffer);
     else
         BKNI_Free(buffer);

     dvrtesthandle->buffer = NULL;
     if(FileFp)
        fclose(FileFp);

StreamToFile_fail_open_file:
     return rc;
}

static int StreamChunkSet(DvrTestHandle dvrtesthandle)
{
     B_DVR_MediaFileSettings      mediaFileSettings;
     int rc = 0;

     if(!dvrtesthandle->mediaFile)
     {
          printf("\n Streaming has not been opened yet!!\n");
          rc = -1;
          goto StreamChunkSet_error_end;
     }
     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);  
     mediaFileSettings.fileOperation.streamingOperation.direction =  dvrtesthandle->hnStreamPara.Direction;   
     switch(dvrtesthandle->hnStreamPara.Direction)
     {
         case eB_DVR_MediaFileDirectionForward:
              printf("\n Direction--Forward\n");
              break;
         case eB_DVR_MediaFileDirectionReverse:
              printf("\n Direction--Reserve\n");
              break;
         default:
              mediaFileSettings.fileOperation.streamingOperation.direction = eB_DVR_MediaFileDirectionForward;
              break;
     }

     mediaFileSettings.fileOperation.streamingOperation.readMode = dvrtesthandle->hnStreamPara.TrickMode;
     switch(dvrtesthandle->hnStreamPara.TrickMode)
     {
         case eB_DVR_MediaFileReadModeFull:
              printf("\n TrickMode is FULL mode\n");
              break;
         case eB_DVR_MediaFileReadModeIFrame:
              printf("\n TrickMode is I-mode\n");
              break;
         case eB_DVR_MediaFileReadModeIPFrame:
              printf("\n TrickMode is IP-mode\n");
              break;
         default:
              mediaFileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeFull;
              break;
     }
     dvrtesthandle->frameTypeInTrickMode = mediaFileSettings.fileOperation.streamingOperation.readMode;
     dvrtesthandle->playScale.numerator = dvrtesthandle->hnStreamPara.PlayScaleSpeed;
     dvrtesthandle->frameSymbolRate = FrameSymbolRate;
     B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
     if((mediaFileSettings.fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame)||
        (mediaFileSettings.fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIPFrame))
     {
         rc = HnCalSkipStepForIPFrame(dvrtesthandle);
         if(rc !=0)
         {
            printf("\n Error calculate skipstep for trickmode IP Frame \n");
            goto StreamChunkSet_error_end;
         }  
     }
StreamChunkSet_error_end:
     return rc;
}

static int StreamOpen(DvrTestHandle dvrtesthandle)
{
     char programName[B_DVR_MAX_FILE_NAME_LENGTH];
     B_DVR_MediaFileOpenMode openMode;
     B_DVR_MediaFilePlayOpenSettings openSettings;
     B_DVR_MediaFileStats mediaFileStats;
     B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
     int rc = 0;

     strcpy(openSettings.subDir,dvrtesthandle->hnStreamPara.subDir);
     strcpy(programName,dvrtesthandle->hnStreamPara.programName);
     openMode = eB_DVR_MediaFileOpenModeStreaming;
     openSettings.playpumpIndex = 3;
     openSettings.volumeIndex = dvrtesthandle->hnStreamPara.StreamPathIndex;
     dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
     if(!dvrtesthandle->mediaFile)
     {
         printf("\n error in opening mediaFile\n");
         rc = -1;
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
     return rc;
}

static int StreamUDPMode(DvrTestHandle dvrtesthandle)
{
     int rc = 0;

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n Streaming has not been opened yet!!\n");
         rc = -1;
         goto StreamUDPMode_error_end;
     }

     switch(dvrtesthandle->hnStreamPara.streamingmode)
     {
         case 0:
             rc = StreamToFile(dvrtesthandle);
             break;

         case 1:
             rc = StreamToVLC(dvrtesthandle);
             break;
     }
StreamUDPMode_error_end:
     return rc;
}

static void StreamClose(DvrTestHandle dvrtesthandle)
{
     if(dvrtesthandle->DecryptDrmHandle)
     {
         B_DVR_DRMService_Close(dvrtesthandle->DecryptDrmHandle);
         printf("\n Doing NEXUS_Dma_Close \n");
         NEXUS_Dma_Close(dvrtesthandle->decDma);
         printf("\n After NEXUS_Dma_Close \n");
         dvrtesthandle->DecryptDrmHandle = NULL;
     }
     if(dvrtesthandle->mediaFile)
         B_DVR_MediaFile_Close(g_dvrTest->mediaFile);
     dvrtesthandle->mediaFile = NULL;
     return;
}

static int StreamSeekSet(DvrTestHandle dvrtesthandle)
{
     B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
     B_DVR_MediaFileSettings     mediaFileSettings;
     int rc = 0;

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n Medaifile has not been opened yet!!\n");
         goto StreamSeekSet_error;
     }
     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);  
     switch(dvrtesthandle->hnStreamPara.SeekMode)
     {
        case eB_DVR_MediaFileSeekModeByteOffsetBased:
             mediaFileSeekSettings.mediaSeek.offset =(off_t)dvrtesthandle->hnStreamPara.SeekValue;
             mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
             break;
        case eB_DVR_MediaFileSeekModeTimestampBased:
             mediaFileSeekSettings.mediaSeek.time = dvrtesthandle->hnStreamPara.SeekValue;;
             mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeTimestampBased;
             break;
        default:
             break;
     }

     if((mediaFileSettings.fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame) ||
        (mediaFileSettings.fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame))
     {
        rc = B_DVR_MediaFile_Seek(dvrtesthandle->mediaFile,&mediaFileSeekSettings);
        if(rc!= 0)
        {
            printf("\n Error seeking \n");
            goto StreamSeekSet_error;
        }
        rc = B_DVR_MediaFile_StreamIPFrameRate(dvrtesthandle->mediaFile,&dvrtesthandle->streamFrameRate);
        if(rc !=0)
        {
            printf("\n Error on cal B_DVR_MediaFile_StreamIPFrameRate\n");
            goto StreamSeekSet_error;
        }  
        rc = HnCalSkipStepForIPFrame(dvrtesthandle);
        if(rc !=0)
        {
            printf("\n Error calculate skipstep for trickmode IP Frame \n");
            goto StreamSeekSet_error;
        }  
     }
     rc = B_DVR_MediaFile_Seek(dvrtesthandle->mediaFile,&mediaFileSeekSettings);
     if(rc!= 0)
     {
        printf("\n Error seeking \n");
     }
StreamSeekSet_error:
     return rc;
}

static int StreamDecryptionMode(DvrTestHandle dvrtesthandle)
{
     B_DVR_MediaFileSettings      mediaFileSettings;
     B_DVR_DRMServiceRequest      drmServiceRequest;  
     B_DVR_DRMServiceSettings     drmServiceSettings;
     NEXUS_DmaHandle              decDma;	
     int rc = 0;

     printf("\n Enter open HnGetDRMDecHandle\n");
     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
     decDma = NEXUS_Dma_Open(0, NULL);	
     dvrtesthandle->decDma = decDma;
     drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
     drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
     drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
     drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
     drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
     drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
     drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
     drmServiceRequest.service = eB_DVR_ServiceHomeNetworkDRM;
     drmServiceRequest.dma = decDma;					
     dvrtesthandle->DecryptDrmHandle = B_DVR_DRMService_Open(&drmServiceRequest);  
					
     drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
     drmServiceSettings.keyLength = KEY_LENGTH;
     drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
     drmServiceSettings.sessionKeys = changeToLocalSession;
     drmServiceSettings.keys = changeToLocalControlWord;
     drmServiceSettings.ivKeys = changeToIv;				
     drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
     rc = B_DVR_DRMService_Configure(dvrtesthandle->DecryptDrmHandle,&drmServiceSettings);
     if(rc)
     {
         printf("\n DRMService configuration failed");
         goto errorOfDRMDec;
     }   
     mediaFileSettings.drmHandle = dvrtesthandle->DecryptDrmHandle;
     B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
     printf("\n End open HnGetDRMDecHandle drmhandle=%lx\n",(unsigned long)dvrtesthandle->DecryptDrmHandle);

errorOfDRMDec:
     return rc;
}

static int streammodetest(DvrTestHandle dvrtesthandle)
{
     int rc = 0;

     rc = StreamOpen(dvrtesthandle);
     if(rc)
     {
        printf("\n Stream open error!!\n");
        goto errorstreammodetest;
     }
     rc = StreamChunkSet(dvrtesthandle);
     if(rc)
     {
        printf("\n Stream Chunk Set Error!!\n");
        goto errorstreammodetest;
     }
     rc = StreamSeekSet(dvrtesthandle);
     if(rc)
     {
        printf("\n Stream Seek Set Error!!\n");
        goto errorstreammodetest;
     }
     if (dvrtesthandle->hnStreamPara.DRMMode)
     {
         rc = StreamDecryptionMode(dvrtesthandle);
         if(rc)
         {
            printf("\n DRM Mode Set Error!!\n");
            goto errorstreammodetest;
         }
     }
     rc = StreamUDPMode(dvrtesthandle);
     if(rc)
     {
        printf("\n UDP Mode Set Error!!\n");
        goto errorstreammodetest;
     }
     StreamClose(dvrtesthandle);

errorstreammodetest:
     return rc;
}

void InitHnStreamingpara(void)
{
    g_dvrTest->hnStreamPara.Direction = eB_DVR_MediaFileDirectionForward;
    g_dvrTest->hnStreamPara.TrickMode = eB_DVR_MediaFileReadModeFull;
    g_dvrTest->hnStreamPara.SeekMode = eB_DVR_MediaFileSeekModeByteOffsetBased;
    g_dvrTest->hnStreamPara.SeekValue = 0;
    g_dvrTest->hnStreamPara.targetfilename[0] = '\0';
    g_dvrTest->hnStreamPara.subDir[0] = '\0';
    g_dvrTest->hnStreamPara.programName[0] = '\0';
    g_dvrTest->hnStreamPara.streamingmode = 0;
    g_dvrTest->hnStreamPara.DRMMode = 0;
    g_dvrTest->hnStreamPara.StreamPathIndex = 0;
    g_dvrTest->hnStreamPara.PlayScaleSpeed = 1;
    g_dvrTest->hnStreamPara.dstip[0] = '\0';   
    g_dvrTest->hnStreamPara.interfaceName[0] = '\0';    
    g_dvrTest->hnStreamPara.portnum[0] = '\0';    
}

void CheckTestName(char *buffer)
{
    char testpara[512];
    char strpara[512];
    int  paravalue = 0;
    unsigned long paraunsigned = 0;
    int  n = 0, m = 0;

    n = sscanf(buffer, "%s\n", testpara);
    if (n > 0)
    {
       if(strcmp(testpara, "SubFolderName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.subDir,strpara);
       }
       else if(strcmp(testpara, "ProgramName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.programName,strpara);
       }
       else if(strcmp(testpara, "StreamPathIndex") == 0)
       {
          m = sscanf(buffer, "%s %d\n", testpara,&paravalue);
          if(m > 0)
            g_dvrTest->hnStreamPara.StreamPathIndex = paravalue;
       }
       else if(strcmp(testpara, "Forward") == 0)
            g_dvrTest->hnStreamPara.Direction = eB_DVR_MediaFileDirectionForward;
       else if(strcmp(testpara, "Reverse") == 0)
            g_dvrTest->hnStreamPara.Direction = eB_DVR_MediaFileDirectionReverse;
       else if(strcmp(testpara, "PlayScaleSpeed") == 0)
       {
          m = sscanf(buffer, "%s %d\n", testpara,&paravalue);
          if(m > 0)
            g_dvrTest->hnStreamPara.PlayScaleSpeed = paravalue;
       }
       else if(strcmp(testpara, "TrickModeFULL") == 0)
            g_dvrTest->hnStreamPara.TrickMode = eB_DVR_MediaFileReadModeFull;
       else if(strcmp(testpara, "TrickModeI") == 0)
            g_dvrTest->hnStreamPara.TrickMode = eB_DVR_MediaFileReadModeIFrame;
       else if(strcmp(testpara, "TrickModeIP") == 0)
            g_dvrTest->hnStreamPara.TrickMode = eB_DVR_MediaFileReadModeIPFrame;
       else if(strcmp(testpara, "DRMEnable") == 0)
            g_dvrTest->hnStreamPara.DRMMode = 1;
       else if(strcmp(testpara, "DRMDisable") == 0)
            g_dvrTest->hnStreamPara.DRMMode = 0;
       else if(strcmp(testpara, "SeekByteOffset") == 0)
       {
          m = sscanf(buffer, "%s %lu\n", testpara,&paraunsigned);
          if(m > 0)
          {
            g_dvrTest->hnStreamPara.SeekValue = paraunsigned;
            g_dvrTest->hnStreamPara.SeekMode = eB_DVR_MediaFileSeekModeByteOffsetBased;
          }
       }
       else if(strcmp(testpara, "SeekTimeStamp") == 0)
       {
          m = sscanf(buffer, "%s %lu\n", testpara,&paraunsigned);
          if(m > 0)
          {
            g_dvrTest->hnStreamPara.SeekValue = paraunsigned;
            g_dvrTest->hnStreamPara.SeekMode = eB_DVR_MediaFileSeekModeTimestampBased;
          }
       }
       else if(strcmp(testpara, "StreamingToFile") == 0)
            g_dvrTest->hnStreamPara.streamingmode = 0;
       else if(strcmp(testpara, "StreamingToUDP") == 0)
            g_dvrTest->hnStreamPara.streamingmode = 1;
       else if(strcmp(testpara, "TargetFileName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.targetfilename,strpara);
       }
       else if(strcmp(testpara, "DestpIP") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.dstip,strpara);
       }
       else if(strcmp(testpara, "LocalIFName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.interfaceName,strpara);
       }
       else if(strcmp(testpara, "PortNum") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnStreamPara.portnum,strpara);
       }
       else
          BDBG_WRN(("Parameter not recognized: %s\n", testpara));
    }
}

int HnStreamingLoadTxt(void)
{
    char buffer[512];
    FILE *fd;
    int rc = 0;

    printf("\n Load Hn Streaming input txt file \n");
    fd= fopen(hnstreamingtxt, "r");
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
         CheckTestName(buffer);
      }
    }
    fclose(fd);
    return rc;
}

int hnStreamingTest(CuTest * tc)
{
    int returnValue = 0;

    printf("\n Enter hnStreamingTest Main function : \n");
    g_dvrTest->playScale.numerator= 1;
    g_dvrTest->playScale.denominator = 1;
    InitHnStreamingpara();
    returnValue = HnStreamingLoadTxt();
    if (returnValue != 0)
    {
       printf("\n Missing input file\n");
       goto endofhnStreamingTest;
    }
    g_dvrTest->pathIndex = g_dvrTest->hnStreamPara.StreamPathIndex;
    returnValue = streammodetest(g_dvrTest);

endofhnStreamingTest:
    CuAssertTrue(tc, returnValue == 0 );
    return returnValue;
}
