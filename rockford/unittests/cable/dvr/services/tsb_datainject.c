#include "dvr_test.h"

BDBG_MODULE(tsb_datainject);

extern void DvrTest_TuneChannel(CuTest * tc, int index);

extern void DvrTest_LiveDecodeStart(CuTest * tc, int index);
extern void DvrTest_LiveDecodeStop(CuTest * tc, int index);

extern void DvrTest_AudioVideoPathOpen(CuTest * tc, unsigned index);
extern void DvrTest_AudioVideoPathClose(CuTest * tc, int index);

extern void DvrTest_PlaybackDecodeStart(CuTest * tc, int index);
extern void DvrTest_PlaybackDecodeStop(CuTest * tc, int index);

extern void hotplug_callback(void *pParam, int iParam);
extern void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);


extern B_EventHandle TSBServiceReadyEvent;
extern B_EventHandle TSBDataInjectionCompleteEvent;
extern B_EventHandle TSBConversionCompleteEvent;



static B_DVR_ERROR B_DVR_TSB_Convert_Start(int pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TSBServiceStatus tsbStatus;
    B_DVR_TSBServicePermanentRecordingRequest recReq;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService, &tsbStatus);
        sprintf(recReq.programName, "DataInject");
        recReq.recStartTime = tsbStatus.tsbRecStartTime;
        recReq.recEndTime = tsbStatus.tsbRecEndTime;
    
        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
        sprintf(recReq.subDir,"tsbConv");
         rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[pathIndex].tsbService,&recReq);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Invalid paramters");
        }
    }
    else
    {
        printf("TSB Service is not enabled\n");
    }
    return (rc);
}

static void B_DVR_TSB_Convert_Stop(int pathIndex)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;

    rc = B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[pathIndex].tsbService);
    if(rc!=B_DVR_SUCCESS)
    {
        printf("\n Failed to stop conversion");
    }
}



B_DVR_ERROR B_DVR_TSB_Data_Injection_Start(unsigned injectionType, unsigned pid, int pathIndex)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings settings;
    char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    uint8_t pkt[188*30];
    int dataFileID;
    int readSize;



    if (pid == 0x0)
        sprintf(dataFileName, "/mnt/nfs/PAT.TS");
    else
        sprintf(dataFileName, "/mnt/nfs/PMT.TS");
    
    if ((dataFileID = open(dataFileName, O_RDONLY,0666)) < 0)
    {
        printf("\n unable to open %s",dataFileName);
    }
    
    BKNI_Memset((void *)&pkt[0],0,188*30);
    
    readSize = (int) read(dataFileID,&pkt[0],188*30);
    
    if(readSize < 0)
    {
        printf("\n Data file is empty or not available");
    }
    
    B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,
        g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

    if ((injectionType != eB_DVR_DataInjectionServiceTypePSI) && 
         (injectionType!= eB_DVR_DataInjectionServiceTypeRawData))
         return B_DVR_INVALID_PARAMETER;

    settings.dataInjectionServiceType = injectionType;
    settings.pid = pid;
    B_DVR_DataInjectionService_SetSettings(g_dvrTest->streamPath[pathIndex].dataInjectionService,&settings);
    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,&pkt[0],readSize);
    }
    else
    {
        printf("TSB Service is not available\n");
    }
    return (rc);
}

static void B_DVR_TSB_Data_Injection_Stop(int pathIndex)
{
        if(g_dvrTest->streamPath[pathIndex].tsbService)
        {
            B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
        }
        else
        {
            printf("TSB Service is not available\n");
        }
}


static B_DVR_ERROR B_DVR_Delete_Record(int volIndex, char *subDir, char *programName)
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



B_DVR_ERROR CopyMediaFromMetaData(char *fileName)
{
    B_DVR_SegmentedFileNode fileNode;
    off_t returnOffset, fileNodeOffset;
    ssize_t sizeRead=0;
    unsigned index=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_File dvrFile;
    B_DVR_FileSettings settings;
    char temp[512];

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
            printf("file node-%u %s %lu %lu %d\n",
                      index,
                      fileNode.fileName,
                      (unsigned long)fileNode.linearStartOffset,
                      (unsigned long)fileNode.size,
                      fileNode.recordType);

            sprintf(temp, "cp /mnt/dvr0-media/record/%s /mnt/nfs/", fileNode.fileName);
            printf("command: %s\n", temp);
            system(temp);
        }
        index++;
    }while(sizeRead);


    B_DVR_File_Close(&dvrFile);
    printf("dumpMetaDataFile <<<\n");
    return rc;
}


void DvrTestTsbServiceDataInjection(CuTest * tc)
{
    int i;
    int injectionType;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    DvrTest_Operation_Param param;


    printf("\nStarting DvrTestTsbServiceDataInjection...\n");

    /* Assign the AV path index to 0 */
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

    B_Event_Reset(TSBServiceReadyEvent);
    B_Event_Reset(TSBDataInjectionCompleteEvent);
    B_Event_Reset(TSBConversionCompleteEvent);

    /* channel should go up to match data injection data */
    Do_DvrTest_Operation(tc, eDvrTest_OperationChannelUp, &param);


    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param);
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReadyEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);


    injectionType = eB_DVR_DataInjectionServiceTypeRawData;

    for (i=0; i < 10; i++) {

        /* Data Injection of PAT */
        B_DVR_TSB_Data_Injection_Start(injectionType, 0x0, 0);
        /* wait until TSB Data Injection is completed */
        while(B_Event_Wait(TSBDataInjectionCompleteEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        B_Event_Reset(TSBDataInjectionCompleteEvent);
        B_DVR_TSB_Data_Injection_Stop(0);

        /* Data Injection of PMT */
        B_DVR_TSB_Data_Injection_Start(injectionType, 0x3f, 0);
        /* wait until TSB Data Injection is completed */
        while(B_Event_Wait(TSBDataInjectionCompleteEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        B_Event_Reset(TSBDataInjectionCompleteEvent);

        B_DVR_TSB_Data_Injection_Stop(0);
        /* wait for 100 milisec */
        BKNI_Sleep(100);
    }

    B_DVR_TSB_Convert_Start(0);
    /* wait until TSB Data Injection is completed */
    while(B_Event_Wait(TSBConversionCompleteEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_Event_Reset(TSBConversionCompleteEvent);
    B_DVR_TSB_Convert_Stop(0);

    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param);

    CopyMediaFromMetaData("/mnt/dvr0-metadata/tsbConv/DataInject.ts");
    BKNI_Sleep(1000);

    rc = B_DVR_Delete_Record(0, "tsbConv", "DataInject");

    if (rc == B_DVR_SUCCESS)
        printf("DataInjected File is removed successfully\n");
    else
        printf("DataInjected File removal is failed\n");

    /* channel goes down to the original state */
    Do_DvrTest_Operation(tc, eDvrTest_OperationChannelDown, &param);

}
