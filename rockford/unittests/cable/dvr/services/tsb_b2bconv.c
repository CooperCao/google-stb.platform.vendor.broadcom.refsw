#include "dvr_test.h"

#define DURATION1     60000     /* 1 minutes */
#define DURATION2    300000     /* 5 minutes */
#define DURATION3    600000     /* 10 minutes */
#define DURATION4   1200000     /* 20 minutes */
#define DURATION5   1800000     /* 30 minutes */

#define DURATION     DURATION3


BDBG_MODULE(tsb_b2bconv);

extern void DvrTest_LiveDecodeStart(CuTest * tc, int index);
extern void DvrTest_LiveDecodeStop(CuTest * tc, int index);

extern void DvrTest_AudioVideoPathOpen(CuTest * tc, unsigned index);
extern void DvrTest_AudioVideoPathClose(CuTest * tc, int index);

extern void DvrTest_PlaybackDecodeStart(CuTest * tc, int index);
extern void DvrTest_PlaybackDecodeStop(CuTest * tc, int index);

extern void hotplug_callback(void *pParam, int iParam);
extern void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);

extern B_EventHandle TSBServiceReadyEvent;
extern B_EventHandle TSBConversionCompleteEvent;

static int Perm1Deleted = 0;
static int Perm2Deleted = 0;



B_DVR_ERROR B_DVR_TSB_B2B_Convert_Start(unsigned long start, unsigned long end, char *fileName, int pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TSBServiceStatus tsbStatus;
    B_DVR_TSBServicePermanentRecordingRequest recReq;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService, &tsbStatus);
        sprintf(recReq.programName, fileName);
        recReq.recStartTime =start;
        recReq.recEndTime = end;
    
        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
        sprintf(recReq.subDir,"tsbConv");
         rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[pathIndex].tsbService,&recReq);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Failed to do TSB converson of stream path %d", pathIndex);
        }
    }
    else
    {
        printf("TSB Service is not enabled\n");
    }
    return (rc);
}

void B_DVR_TSB_Convert_Stop(int pathIndex)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;

    rc = B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[pathIndex].tsbService);
    if(rc!=B_DVR_SUCCESS)
    {
        printf("\n Failed to stop conversion");
    }
}


B_DVR_ERROR B_DVR_Delete_Record(int volIndex, char *subDir, char *programName)
{

    B_DVR_MediaNodeSettings *mediaNodeSettings;
    B_DVR_ERROR err;
    mediaNodeSettings = malloc(sizeof(B_DVR_MediaNodeSettings));
    
    mediaNodeSettings->subDir = subDir;
    mediaNodeSettings->programName = programName;
    mediaNodeSettings->volumeIndex = volIndex;
    mediaNodeSettings->recordingType = eB_DVR_RecordingPermanent;

    printf("deleting volume: %u directory: %s program: %s\n",mediaNodeSettings->volumeIndex, mediaNodeSettings->subDir, mediaNodeSettings->programName);

    err = B_DVR_Manager_DeleteRecording(B_DVR_Manager_GetHandle(),mediaNodeSettings);
    if (err!=B_DVR_SUCCESS)
    {
        printf("check the file name!\n");
    }
    free(mediaNodeSettings);
    return err;
}

static B_DVR_ERROR dumpMetaDataFile(char *fileName)
{
    B_DVR_SegmentedFileNode fileNode;
    off_t returnOffset, fileNodeOffset;
    ssize_t sizeRead=0;
    unsigned index=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_File dvrFile;
    B_DVR_FileSettings settings;

    settings.fileIO = eB_DVR_FileIORead;
    settings.directIO = false;
    B_DVR_File_Open(&dvrFile,fileName, &settings);

    printf("dumpMetaDataFile %s>>>\n",fileName);
    do
    {
        fileNodeOffset = index*sizeof(fileNode);
        returnOffset = dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            printf("End of metadata file\n");
            break;
        }
         sizeRead = dvrFile.fileInterface.io.readFD.read(
             &dvrFile.fileInterface.io.readFD,&fileNode,sizeof(fileNode));

        if(sizeRead != sizeof(fileNode))
        {
            printf("end of metaDataFile\n");
        }
        else
        {
            printf("------------------------------------------\n");
            printf("File Node Index: %u\n", index);
            printf("File Node Name: %s\n", fileNode.fileName);
            printf("File Node Linear Start Offset: %lu\n", (unsigned long)fileNode.linearStartOffset);
            printf("File Node Size: %lu\n", (unsigned long)fileNode.size);
            printf("File Node Record Type: %d\n", fileNode.recordType);
            printf("File Node Reference Count: %d\n", fileNode.refCount);
            printf("------------------------------------------\n");
        }
        index++;
    }while(sizeRead);

    B_DVR_File_Close(&dvrFile);
    printf("dumpMetaDataFile <<<\n");
    return rc;
}


void DvrTestTsbServiceBack2BackConvert(CuTest * tc)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    B_DVR_TSBServiceStatus tsbStatus;
    DvrTest_Operation_Param param;
    int volumeIndex = 0;


    /* Assign the AV path index to 0 */
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    B_Event_Reset(TSBServiceReadyEvent);
    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param);
    }
    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReadyEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);


    /* Assign the AV path index to 1 */
    memset(&param, 0, sizeof(param));
    param.pathIndex = 1;
    B_Event_Reset(TSBServiceReadyEvent);
    if(!g_dvrTest->streamPath[1].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param);
    }
    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReadyEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);


    B_Event_Reset(TSBConversionCompleteEvent);
    if(g_dvrTest->streamPath[1].tsbService)
    {
        B_DVR_TSBServicePermanentRecordingRequest recReq;

        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[1].tsbService, &tsbStatus);
        sprintf(recReq.programName, "TSBRecord%d", 1);
        recReq.recStartTime = tsbStatus.tsbRecStartTime;
        recReq.recEndTime = recReq.recStartTime + DURATION;

        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
        sprintf(recReq.subDir,"tsbConv");
        rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[1].tsbService,&recReq);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Failed to do TSB conversion of stream path 1");
        }
    }

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionCompleteEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_Event_Reset(TSBConversionCompleteEvent);
    B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[1].tsbService);

    if(g_dvrTest->streamPath[0].tsbService)
    {
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService, &tsbStatus);
        rc = B_DVR_TSB_B2B_Convert_Start(tsbStatus.tsbRecStartTime, tsbStatus.tsbRecEndTime/2, "Perm1", 0);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Back to Back conversion of Perm1 Record is failed");
        }
        rc = B_DVR_TSB_B2B_Convert_Start(tsbStatus.tsbRecEndTime/2, tsbStatus.tsbRecEndTime, "Perm2", 0);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Back to Back conversion of Perm2 Record is failed");
        }
    }

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionCompleteEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_Event_Reset(TSBConversionCompleteEvent);
    B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[0].tsbService);


    dumpMetaDataFile("/mnt/dvr0-metadata/tsbConv/Perm1.ts");
    dumpMetaDataFile("/mnt/dvr0-metadata/tsbConv/Perm2.ts");

    memset(&param, 0, sizeof(param));
    param.pathIndex = 1;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param);

    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param);

    BKNI_Sleep(2000);
    /* automate testing */
    if (!Perm1Deleted)
    {
        printf("\nRemoving Perm1 from the tsbConv directory ....\n");
        rc = B_DVR_Delete_Record(volumeIndex, "tsbConv", "Perm1");
        if (rc == B_DVR_SUCCESS)
        {
            Perm1Deleted = 1;
        }
    }
    
    if (Perm2Deleted)
    {
        printf("Perm2.ts is not available\n");
    }
    else 
    {
        dumpMetaDataFile("/mnt/dvr0-metadata/tsbConv/Perm2.ts");
        printf("\nPlaying Perm2 for about 20 seconds ...");
        sprintf(param.subDir, "tsbConv");
        sprintf(param.programName, "Perm2");
        Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
    }

    /* playback for 20 seconds */
    BKNI_Sleep(20000);

    if (Perm2Deleted)
    {
        printf("Perm2.ts is not available\n");
    }
    else 
    {
        if(!g_dvrTest->streamPath[0].liveDecodeStarted)
        {
            printf("\nStop playing Perm2 ...");
            param.pathIndex = 0;
            Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
        }
        else
        {
            printf("Playback mode is not enabled\n");
        }
    }

    B_DVR_Delete_Record(volumeIndex, "tsbConv", "TSBRecord1");

    /* removing Perm1 & Perm2 and exit program */
    if (!Perm1Deleted)
    {
        rc = B_DVR_Delete_Record(volumeIndex, "tsbConv", "Perm1");
        if (rc == B_DVR_SUCCESS)
        {
            Perm1Deleted = 1;
        }
    }
    if (!Perm2Deleted)
    {
        rc = B_DVR_Delete_Record(volumeIndex, "tsbConv", "Perm2");
        if (rc == B_DVR_SUCCESS)
        {
            Perm2Deleted = 1;
        }
    }
}
