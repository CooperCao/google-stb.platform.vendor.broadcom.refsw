#include "dvr_test.h"
BDBG_MODULE(hnmediaplay);

static char hnmediaplaytxt[] = "hnmediaplay.txt";
static struct DvrTest multDvrTestHandle[MultiplePlayback];
extern void DvrTest_LiveDecodeStop(CuTest * tc, int index);
extern void DvrTest_LiveDecodeStart(CuTest * tc, int index);

static int playbackDrmEnabledWithClearKey(DvrTestHandle dvrtesthandle,B_DVR_MediaNodeSettings *pOpenSettings)
{
     B_DVR_MediaFileSettings      mediaFileSettings;
     B_DVR_DRMServiceRequest      drmServiceRequest;  
     B_DVR_DRMServiceSettings     drmServiceSettings;
     NEXUS_DmaHandle              decDma;	
     B_DVR_Media mNode;
     unsigned keylength;
     unsigned char key[16];
     int rc = 0;

     B_DVR_Manager_GetMediaNode(dvrtesthandle->dvrManager,pOpenSettings,&mNode);
     B_DVR_Manager_GetKeyBlobLengthPerEsStream(dvrtesthandle->dvrManager,pOpenSettings,&keylength);
     B_DVR_Manager_GetKeyBlobPerEsStream(dvrtesthandle->dvrManager,pOpenSettings,mNode.esStreamInfo[0].pid,key);

     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
     decDma = NEXUS_Dma_Open(0, NULL);	
     dvrtesthandle->decDma = decDma;
     drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
     drmServiceRequest.controlWordKeyLoader = NULL;
     drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
     drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
     drmServiceRequest.keySlot = NULL;
     drmServiceRequest.dma = decDma;
     drmServiceRequest.service = eB_DVR_ServicePlayback;
     dvrtesthandle->DecryptDrmHandle = B_DVR_DRMService_Open(&drmServiceRequest);		

     drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
     drmServiceSettings.keyLength = KEY_LENGTH;
     drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
     drmServiceSettings.keys = key;
     drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
     rc = B_DVR_DRMService_Configure(dvrtesthandle->DecryptDrmHandle,&drmServiceSettings);
     if(rc)
     {
         printf("\n playbackDrmEnabledWithClearKey DRMService configuration failed");
         goto playbackDrmEnabledWithClearKeyError;
     }   
     mediaFileSettings.drmHandle = dvrtesthandle->DecryptDrmHandle;
     mediaFileSettings.fileOperation.playbackOperation.operation = eB_DVR_OperationMax;
     B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
playbackDrmEnabledWithClearKeyError:
     return rc;
}

static int playbackDrmEnabledWithKeyLadder(DvrTestHandle dvrtesthandle,B_DVR_MediaNodeSettings *pOpenSettings)
{
     B_DVR_MediaFileSettings      mediaFileSettings;
     B_DVR_DRMServiceRequest      drmServiceRequest;  
     B_DVR_DRMServiceSettings     drmServiceSettings;
     NEXUS_DmaHandle              decDma;	
     B_DVR_Media mNode;
     unsigned keylength;
     unsigned char key[32];
     unsigned char session[16];
     unsigned char controlWd[16];
     unsigned char iv[16];
     int count;
     int rc = 0;

     B_DVR_Manager_GetMediaNode(dvrtesthandle->dvrManager,pOpenSettings,&mNode);
     B_DVR_Manager_GetKeyBlobLengthPerEsStream(dvrtesthandle->dvrManager,pOpenSettings,&keylength);
     B_DVR_Manager_GetKeyBlobPerEsStream(dvrtesthandle->dvrManager,pOpenSettings,mNode.esStreamInfo[0].pid,key);
     for(count=0;count<(int)(keylength*3);count++)
     {
        BDBG_MSG(("[%d] key[0x%x]",count,key[count]));
        if (count<16) session[count] = key[count];
        else if (count >=16 && count < 32) controlWd[count-keylength] = key[count];
        else if (count >= 32) iv[count - (keylength*2)] = key[count];
     }

     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);
     decDma = NEXUS_Dma_Open(0, NULL);	
     dvrtesthandle->decDma = decDma;
     drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
     drmServiceRequest.controlWordKeyLoader = NULL;
     drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
     drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
     drmServiceRequest.keySlot = NULL;
     drmServiceRequest.dma = decDma;
     drmServiceRequest.service = eB_DVR_ServicePlayback;
     dvrtesthandle->DecryptDrmHandle = B_DVR_DRMService_Open(&drmServiceRequest);		

     drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
     drmServiceSettings.keyLength = KEY_LENGTH;
     drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
     drmServiceSettings.sessionKeys = session;
     drmServiceSettings.keys = controlWd;	
     drmServiceSettings.ivKeys = iv;
     drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
     rc = B_DVR_DRMService_Configure(dvrtesthandle->DecryptDrmHandle,&drmServiceSettings);
     if(rc)
     {
         printf("\n playbackDrmEnabledWithClearKey DRMService configuration failed");
         goto playbackDrmEnabledWithKeyLadderError;
     }   
     mediaFileSettings.drmHandle = dvrtesthandle->DecryptDrmHandle;
     mediaFileSettings.fileOperation.playbackOperation.operation = eB_DVR_OperationMax;
     B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&mediaFileSettings);

playbackDrmEnabledWithKeyLadderError:
     return rc;
}

int playbackopen(DvrTestHandle dvrtesthandle)
{
     char programName[B_DVR_MAX_FILE_NAME_LENGTH];
     B_DVR_MediaFileOpenMode openMode;
     B_DVR_MediaFilePlayOpenSettings openSettings;
     B_DVR_MediaNodeSettings mediaNodeSettings;
     int rc = 0;

     strcpy(openSettings.subDir,dvrtesthandle->hnMPPara.programSubDir);
     strcpy(programName,dvrtesthandle->hnMPPara.programName);
     openMode = eB_DVR_MediaFileOpenModePlayback;
     openSettings.volumeIndex = 0;
     openSettings.playpumpIndex = dvrtesthandle->pathIndex;

     openSettings.activeVideoPidIndex[0] = 0;
     openSettings.activeVideoPidIndex[1] = -1;
     openSettings.activeAudioPidIndex = 1;
     openSettings.stcChannel = dvrtesthandle->streamPath[dvrtesthandle->pathIndex].stcChannel;
     openSettings.videoDecoder[0] = dvrtesthandle->streamPath[dvrtesthandle->pathIndex].videoDecoder;
     openSettings.videoDecoder[1] = NULL;
     openSettings.audioDecoder[0] = dvrtesthandle->streamPath[dvrtesthandle->pathIndex].audioDecoder;
     openSettings.audioDecoder[1] = NULL;

     dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
     if(!dvrtesthandle->mediaFile)
     {
         printf("\n error in opening mediaFile\n");
         rc = -1;
         goto playbackopenerror;
     }
     
     mediaNodeSettings.programName = programName;
     mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
     mediaNodeSettings.volumeIndex = openSettings.volumeIndex;
     mediaNodeSettings.subDir = openSettings.subDir;
     switch(dvrtesthandle->playbackDRMEnabledMode)
     {
         case 0:
              printf("\n Playback DRM is diasbled\n");
              break;
         case 1:
              printf("\n Playback DRM enabled with ClearKey\n");
              rc = playbackDrmEnabledWithClearKey(dvrtesthandle,&mediaNodeSettings);
              break;
         case 2:
              printf("\n Playback DRM enabled with Keyladder\n");
              rc = playbackDrmEnabledWithKeyLadder(dvrtesthandle,&mediaNodeSettings);
              break;
         default:
              printf("\n Playback DRM is diasbled\n");
              break;
     }
playbackopenerror:
     return rc;
}

int playbackstart(DvrTestHandle dvrtesthandle)
{
     B_DVR_MediaFilePlaySettings playSettings;
     int rc = 0;

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n PlaybackService handler is NULL\n");
         rc = -1;
         goto errorplaybackstart;
     }

     if(g_dvrTest->streamPath[dvrtesthandle->pathIndex].liveDecodeStarted)
     {
         printf("\n LiveDecode is stoppping now due to it has been started in testinit module\n");
         if(!dvrtesthandle->pathIndex)
             NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[dvrtesthandle->pathIndex].audioDecoder);
         NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[dvrtesthandle->pathIndex].videoDecoder);	 
     } 
     else
         printf("\n LiveDecode is NOT started yet in testinit module index= %d\n",dvrtesthandle->pathIndex);
	 
     rc = B_DVR_MediaFile_PlayStart(dvrtesthandle->mediaFile,&playSettings);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Start fail!!\n");
     else
     {
        dvrtesthandle->playbackactived = 1;
        dvrtesthandle->streamPath[dvrtesthandle->pathIndex].playbackDecodeStarted=true;
     }

errorplaybackstart:
     return rc;
}

int playbackstop(DvrTestHandle dvrtesthandle)
{
     int rc = 0;

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n PlaybackService handler is NULL\n");
         rc = -1;
         goto errorplaybackstop;
     }
     rc = B_DVR_MediaFile_PlayStop(dvrtesthandle->mediaFile);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Stop fail!!\n");

     if(dvrtesthandle->DecryptDrmHandle)
     {
         B_DVR_DRMService_Close(dvrtesthandle->DecryptDrmHandle);
         printf("\n Doing NEXUS_Dma_Close \n");
         NEXUS_Dma_Close(dvrtesthandle->decDma);
         printf("\n After NEXUS_Dma_Close \n");
         dvrtesthandle->DecryptDrmHandle = NULL;
     }
     rc = B_DVR_MediaFile_Close(dvrtesthandle->mediaFile);
     if(rc != B_DVR_SUCCESS)
         printf("\n Playback Close fail!!\n");
     dvrtesthandle->mediaFile = NULL;
     dvrtesthandle->playbackactived = 0;
     dvrtesthandle->playbackDRMEnabledMode = 0;
     dvrtesthandle->streamPath[dvrtesthandle->pathIndex].playbackDecodeStarted=false;
     if(g_dvrTest->streamPath[dvrtesthandle->pathIndex].liveDecodeStarted)
     {
         printf("\n LiveDecode is restart index= %d\n",dvrtesthandle->pathIndex);
         NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[dvrtesthandle->pathIndex].videoDecoder, &g_dvrTest->streamPath[dvrtesthandle->pathIndex].videoProgram);
         if(!dvrtesthandle->pathIndex)
              NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[dvrtesthandle->pathIndex].audioDecoder,&g_dvrTest->streamPath[dvrtesthandle->pathIndex].audioProgram);
     }	 
errorplaybackstop:
     return rc;
}

int playbackmodeTrickModeSetting(DvrTestHandle dvrtesthandle)
{
     B_DVR_MediaFileSettings Settings;
     int rc = 0;

     if(!dvrtesthandle->playbackactived)
     {
         printf("\n Playback has not been start yet!!\n");
         rc = -1;
         goto errorplaybackmodeTrickModeSetting;
     }
     B_DVR_MediaFile_GetSettings(dvrtesthandle->mediaFile,&Settings);
     switch(dvrtesthandle->hnMPPara.TrickValue)
     {
        case eB_DVR_OperationPlay:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationPlay;
             break;
        case eB_DVR_OperationPause:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationPause;
             break;
        case eB_DVR_OperationFastRewind:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationFastRewind;
             Settings.fileOperation.playbackOperation.operationSpeed = 4;
             break;
        case eB_DVR_OperationFastForward:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationFastForward;
             Settings.fileOperation.playbackOperation.operationSpeed = 4;
             break;
        case eB_DVR_OperationSlowRewind:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationSlowRewind;
             Settings.fileOperation.playbackOperation.operationSpeed = 4;
             break;
        case eB_DVR_OperationSlowForward:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationSlowForward;
             Settings.fileOperation.playbackOperation.operationSpeed = 4;
             break;
        case eB_DVR_OperationForwardFrameAdvance:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationForwardFrameAdvance;
             break;
        case eB_DVR_OperationReverseFrameAdvance:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationReverseFrameAdvance;
             break;
        default:
             Settings.fileOperation.playbackOperation.operation = eB_DVR_OperationPlay;
             break;
     }
     rc = B_DVR_MediaFile_SetSettings(dvrtesthandle->mediaFile,&Settings);
errorplaybackmodeTrickModeSetting:
     return rc;
}

void playbackmodeDRMEnabled(DvrTestHandle dvrtesthandle)
{
     switch(dvrtesthandle->hnMPPara.DRMMode)
     {
         case 0:
              dvrtesthandle->playbackDRMEnabledMode = 0;
              break;
         case 1:
              dvrtesthandle->playbackDRMEnabledMode = 1;
              break;
         case 2:
              dvrtesthandle->playbackDRMEnabledMode = 2;
              break;
         default:
              dvrtesthandle->playbackDRMEnabledMode = 0;
              break;
     }
}

void InitHnMediaplaypara(void)
{
    g_dvrTest->hnMPPara.programSubDir[0] = '\0';
    g_dvrTest->hnMPPara.programName[0] = '\0';
    g_dvrTest->hnMPPara.DRMMode = 0;
    g_dvrTest->hnMPPara.StreamPathIndex = 0;
    g_dvrTest->hnMPPara.TrickModeSet = 0;
    g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationPlay;
}

void CheckMediaPlayName(char *buffer)
{
    char testpara[512];
    char strpara[512];
    int  paravalue = 0;
    int  n = 0, m = 0;

    n = sscanf(buffer, "%s\n", testpara);
    if (n > 0)
    {
       if(strcmp(testpara, "SubFolderName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnMPPara.programSubDir,strpara);
       }
       else if(strcmp(testpara, "ProgramName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnMPPara.programName,strpara);
       }
       else if(strcmp(testpara, "StreamPathIndex") == 0)
       {
          m = sscanf(buffer, "%s %d\n", testpara,&paravalue);
          if(m > 0)
            g_dvrTest->hnMPPara.StreamPathIndex = paravalue;
       }
       else if(strcmp(testpara, "DRMDisable") == 0)
            g_dvrTest->hnMPPara.DRMMode = 0;
       else if(strcmp(testpara, "DRMEnableClearKey") == 0)
            g_dvrTest->hnMPPara.DRMMode = 1;
       else if(strcmp(testpara, "DRMEnableKeyladder") == 0)
            g_dvrTest->hnMPPara.DRMMode = 2;
       else if(strcmp(testpara, "TrickModeDisable") == 0)
            g_dvrTest->hnMPPara.TrickModeSet = 0;
       else if(strcmp(testpara, "TrickModeEnable") == 0)
            g_dvrTest->hnMPPara.TrickModeSet = 1;
       else if(strcmp(testpara, "TrickModeValue") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
              if(strcmp(strpara,"Play") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationPlay;
              else if(strcmp(strpara,"Pause") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationPause;
              else if(strcmp(strpara,"FastRewind") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationFastRewind;
              else if(strcmp(strpara,"FastForward") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationFastForward;
              else if(strcmp(strpara,"SlowRewind") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationSlowRewind;
              else if(strcmp(strpara,"SlowForward") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationSlowForward;
              else if(strcmp(strpara,"FrameAdvance") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationForwardFrameAdvance;
              else if(strcmp(strpara,"FrameReverse") == 0)
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationReverseFrameAdvance;
              else
                  g_dvrTest->hnMPPara.TrickValue = eB_DVR_OperationPlay;
          }
       }
       else
          BDBG_WRN(("Parameter not recognized: %s\n", testpara));
    }
}

int HnMediaPlayLoadTxt(void)
{
    char buffer[512];
    FILE *fd;
    int rc = 0;

    printf("\n Load Hn Mediaplay input txt file \n");
    fd= fopen(hnmediaplaytxt, "r");
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
         CheckMediaPlayName(buffer);
      }
    }
    fclose(fd);
    return rc;
}


int hnMediaPlayTest(CuTest * tc)
{
     int rc = 0;
     int loopcount=6;

     printf("\n Enter hnMediaPlayTest Main function : \n");

     InitHnMediaplaypara();
     rc = HnMediaPlayLoadTxt();
     if (rc != 0)
     {
        printf("\n Missing input file\n");
        goto errormediaplaytest;
     }
     g_dvrTest->pathIndex = g_dvrTest->hnMPPara.StreamPathIndex;
     playbackmodeDRMEnabled(g_dvrTest);
     rc = playbackopen(g_dvrTest);
     if(rc)
     {
        printf("\n Playback open error!!\n");
        goto errormediaplaytest;
     }
     rc = playbackstart(g_dvrTest);
     if(rc)
     {
        printf("\n Playback start error!!\n");
        goto errormediaplaytest;
     }
     BKNI_Sleep(5000);
     rc = playbackmodeTrickModeSetting(g_dvrTest);
     while(loopcount--)
        BKNI_Sleep(5000);
     rc = playbackstop(g_dvrTest);
     if(rc)
     {
        printf("\n Playback stop error!!\n");
     }
errormediaplaytest:
    CuAssertTrue(tc, rc == 0 );
    return rc;
}

int hnMediaPlayTestMultiplePlay(CuTest * tc)
{
     int rc = 0;
     int loopcount=6;
     int mulcnt = 0;

     printf("\n Enter hnMediaPlayTest Main function : \n");

     InitHnMediaplaypara();
     rc = HnMediaPlayLoadTxt();
     if (rc != 0)
     {
        printf("\n Missing input file\n");
        goto errormediaplaytest;
     }
     g_dvrTest->pathIndex = g_dvrTest->hnMPPara.StreamPathIndex;
     playbackmodeDRMEnabled(g_dvrTest);
     for (mulcnt = 0 ; mulcnt < MultiplePlayback; mulcnt++)
        BKNI_Memcpy((void *)(&multDvrTestHandle[mulcnt]),(void *)g_dvrTest,sizeof(*g_dvrTest));    

     for (mulcnt = 0 ; mulcnt < MultiplePlayback; mulcnt++)
     { 
        printf("\n Enter hnMediaPlayTest Main function: Step --00\n");
        BKNI_Memcpy((void *)g_dvrTest,(void *)(&multDvrTestHandle[mulcnt]),sizeof(*g_dvrTest));
        g_dvrTest->pathIndex = mulcnt;
        printf("\n Enter hnMediaPlayTest Main function: Step --11\n");
        rc = playbackopen(g_dvrTest);
        if(rc)
        {
           printf("\n Playback open error!!\n");
           goto errormediaplaytest;
        }
        multDvrTestHandle[mulcnt].pathIndex = g_dvrTest->pathIndex;
        multDvrTestHandle[mulcnt].mediaFile = g_dvrTest->mediaFile;
        printf("\n Enter hnMediaPlayTest Main function: Step --22\n");
        rc = playbackstart(g_dvrTest);
        if(rc)
        {
           printf("\n Playback start error!!\n");
           goto errormediaplaytest;
        }
        BKNI_Sleep(5000);
        rc = playbackmodeTrickModeSetting(g_dvrTest);
        printf("\n Enter hnMediaPlayTest Main function: Step --33\n");
     }

     while(loopcount--)
        BKNI_Sleep(5000);

     for (mulcnt = 0 ; mulcnt < MultiplePlayback; mulcnt++)
     {   
        BKNI_Memcpy((void *)g_dvrTest,(void *)(&multDvrTestHandle[mulcnt]),sizeof(*g_dvrTest));    
        rc = playbackstop(g_dvrTest);
        if(rc)
        {
           printf("\n Playback stop error!!\n");
        }
        printf("\n Enter hnMediaPlayTest Main function: Step --44-- audioPidChannels = %ld\n",(long)(g_dvrTest->streamPath[mulcnt].audioPidChannels[0]));
        BKNI_Sleep(5000);		
     }
errormediaplaytest:
    CuAssertTrue(tc, rc == 0 );
    return rc;
}

