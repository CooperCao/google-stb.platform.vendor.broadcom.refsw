/***************************************************************************
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
#include "dvr_test.h"

BDBG_MODULE(tsb_services);

extern void DvrTest_LiveDecodeStart(CuTest * tc, int index);
extern void DvrTest_LiveDecodeStop(CuTest * tc, int index);

extern void DvrTest_AudioVideoPathOpen(CuTest * tc, unsigned index);
extern void DvrTest_AudioVideoPathClose(CuTest * tc, int index);

extern void DvrTest_PlaybackDecodeStart(CuTest * tc, int index);
extern void DvrTest_PlaybackDecodeStop(CuTest * tc, int index);

extern void hotplug_callback(void *pParam, int iParam);
extern void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);


extern B_EventHandle TSBServiceReadyEvent;



static int B_DVR_Generate_Random(int limit)
{
    int r;
    time_t t;
    unsigned int seed;

    t = time(NULL);
    seed = (unsigned int)t;
    srand(seed);
    r = rand() % limit;

    return (r);
}



#if defined(ENABLE_DRM)
void DvrTestTsbServiceCpsClearKeyTest(CuTest * tc)
{

    DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsClearKeyStart, &param); 
    sleep(10);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsClearKeyStop, &param); 
}

void DvrTestTsbServiceCpsKeyladderTest(CuTest * tc)
{

    DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsKeyladderStart, &param); 
    sleep(10);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsKeyladderStop, &param); 
}


void DvrTestTsbServiceM2mClearKeyTest(CuTest * tc)
{

    DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyStart, &param); 
    sleep(10);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyStop, &param); 
}

void DvrTestTsbServiceM2mKeyladderTest(CuTest * tc)
{

    DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderStart, &param); 
    sleep(10);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderStop, &param); 
}

void DvrTestTsbServiceCpsEncM2mDecClearKeyPlaybackTest(CuTest * tc)
{
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceCpsEncM2mDecClearKeyPlaybackTest ******* \n");
	printf(" ****** [TSB] CPS ENC & M2M DEC USING CLEARKEY [START] ********** \n");
	printf(" **************************************************************** \n");
	
    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsClearKeyStart, &param); 
    }
	
	BKNI_Sleep(10000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStart, &param); 
	BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStop, &param);

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceCpsEncM2mDecClearKeyPlaybackTest ******* \n");
	printf(" ****** [TSB] CPS ENC & M2M DEC USING CLEARKEY [END] ************ \n");
	printf(" **************************************************************** \n");
	
	
}

void DvrTestTsbServiceCpsEncM2mDecKeyladderPlaybackTest(CuTest * tc)
{
    DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceCpsEncM2mDecKeyladderPlaybackTest ****** \n");
	printf(" ****** [TSB] CPS ENC & M2M DEC USING KEYLADDER [START] ********* \n");
	printf(" **************************************************************** \n");

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsKeyladderStart, &param); 
    }
	
	BKNI_Sleep(10000);	
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStart, &param); 
    BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStop, &param);

	 printf("\n \n \n **************************************************************** \n");
	 printf(" ****** DvrTestTsbServiceCpsEncM2mDecKeyladderPlaybackTest ****** \n");
	 printf(" ****** [TSB] CPS ENC & M2M DEC USING KEYLADDER [END] ************ \n");
	 printf(" **************************************************************** \n");
	
 }

void DvrTestTsbServiceM2mEncM2mDecClearKeyPlaybackTest(CuTest * tc)
 {
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
	
	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceM2mEncM2mDecClearKeyPlaybackTest ******* \n");
	printf(" ****** [TSB] M2M ENC & M2M DEC USING CLEARKEY [START] ********** \n");
	printf(" **************************************************************** \n");	

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyStart, &param); 
    }
	
	BKNI_Sleep(10000);	
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStart, &param); 
    BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mToM2mClearKeyPlaybackStop, &param);

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceM2mEncM2mDecClearKeyPlaybackTest ******* \n");
	printf(" ****** [TSB] M2M ENC & M2M DEC USING CLEARKEY [END] ************ \n");
	printf(" **************************************************************** \n");
	
}

void DvrTestTsbServiceM2mEncM2mDecKeyladderPlaybackTest(CuTest * tc)
{
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceM2mEncM2mDecKeyladderPlaybackTest ****** \n");
	printf(" ****** [TSB] M2M ENC & M2M DEC USING KEYLADDER [START] ********* \n");
	printf(" **************************************************************** \n");	
	
    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderStart, &param); 
    }
	
	BKNI_Sleep(10000);	
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStart, &param); 
    BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceM2mToM2mKeyladderPlaybackStop, &param);

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceM2mEncM2mDecKeyladderPlaybackTest ****** \n");
	printf(" ****** [TSB] M2M ENC & M2M DEC USING KEYLADDER [END] *********** \n");
	printf(" **************************************************************** \n");	
}

void DvrTestTsbServiceCpsEncCaDecClearKeyPlaybackTest(CuTest * tc)
{
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceCpsEncCaDecClearKeyPlaybackTest ******** \n");
	printf(" ****** [TSB] CPS ENC & CA DEC USING CLEARKEY [START] *********** \n");
	printf(" **************************************************************** \n");	
	
    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsClearKeyStartForCa, &param);     
    }
	
	BKNI_Sleep(10000);		
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCaClearKeyPlaybackStart, &param); 
    BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCaClearKeyPlaybackStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestTsbServiceCpsEncCaDecClearKeyPlaybackTest ******** \n");
	printf(" ****** [TSB] CPS ENC & CA DEC USING CLEARKEY [END] ************* \n");
	printf(" **************************************************************** \n"); 
	
}

void DvrTestTsbServiceCpsEncCaDecKeyladderPlaybackTest(CuTest * tc)
{
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;
    if(!g_dvrTest->streamPath[0].tsbService)
    {
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCpsKeyladderStartForCa, &param);     
    }
	
	BKNI_Sleep(10000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCaKeyladderPlaybackStart, &param); 
    BKNI_Sleep(8000);
    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceCaKeyladderPlaybackStop, &param); 
}


#endif
void DvrTestTsbServiceTest(CuTest * tc)
{
    /* Once TSB playback has started, it performs rewind and fastforward 
         repeatedly. The purpose is to check whether any deadlock happens.
    */
    
    int commands[19] = {0, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1,3, 1, 3, 5, -1};
    int *ptr;

    B_DVR_TSBServiceStatus tsbServiceStatus;
    B_DVR_OperationSettings operationSettings;
    DvrTest_Operation_Param param;

    /* Assign the AV path index to 0 */
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

    B_Event_Reset(TSBServiceReadyEvent);

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        printf("\nTSB Service is not started\n");
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param); 
        printf("Starting TSB Service...\n");
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReadyEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);


    ptr = &commands[0];
    while (*ptr >= 0) {

        BKNI_Sleep(3000);

        switch(*ptr)
        {
            case 0:
                    printf("\n****************************\n");
                    printf("Command: TSB playback pause");
                    printf("\n****************************\n");
                    if(g_dvrTest->streamPath[0].tsbService) {
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                        if (!tsbServiceStatus.tsbPlayback)
                        {
                            printf("TSB Playback is not started\n");
                            Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServicePlayStart, &param); 
                            printf("TSB Playback is starting...\n");
                        }
                        else
                        {
                            if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                            {
                                printf("TSB playback is not started. Start TSB playback\n");
                            }
                            else
                            {
                                B_DVR_OperationSettings operationSettings;
                                operationSettings.operation = eB_DVR_OperationPause;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                            }
                        }
                    }
                    else 
                    {
                        printf("TSB Service is not enabled\n");
                    }
            break;
            case 1:
                printf("\n****************************\n");
                printf("Command: TSB playback rewind");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastRewind;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 2:
                printf("\n****************************\n");
                printf("Command: TSB playback play");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 3:
                printf("\n****************************\n");
                printf("Command: TSB playback forward");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastForward;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 4:
                printf("\n****************************\n");
                printf("Command: Convert to Record");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService)
                {
                    B_DVR_ERROR rc = B_DVR_SUCCESS;
                    B_DVR_TSBServiceStatus tsbStatus;
                    B_DVR_TSBServicePermanentRecordingRequest recReq;
                    unsigned fileSuffix;

                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService, &tsbStatus);
                    fileSuffix = B_DVR_Generate_Random(100);
                    sprintf(recReq.programName, "PermRecord%d", fileSuffix);
                    recReq.recStartTime = tsbStatus.tsbRecStartTime;
                    recReq.recEndTime = tsbStatus.tsbRecEndTime;

                    printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
                    sprintf(recReq.subDir,"tsbConv");
                     rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[0].tsbService,&recReq);
                    if(rc!=B_DVR_SUCCESS)
                    {
                        printf("\n Invalid paramters");
                    }
                }
                else
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 5:
                printf("\n****************************\n");
                printf("Command: Switch to Live mode");
                printf("\n****************************\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                    printf("It is already in the Live mode!!!\n");
                else {
                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                    if (tsbServiceStatus.tsbPlayback)
                    {
                        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServicePlayStop, &param); 
                    }
                    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param); 
                }
            break;
            case 6:
                printf("\n****************************\n");
                printf("Command: Switch to TSB mode");
                printf("\n****************************\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                {
                    printf("Starting TSB Service...");
                    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param); 
                }
                else
                {
                    printf("TSB Service is already started\n");
                }
            break;
            default:
                printf("Unknown Command\n");
                break;
        }
        ++ptr;
    }
}



void DvrTestTsbServiceLongevity(CuTest * tc)
{

    int i;
    int cmd;

    B_DVR_TSBServiceStatus tsbServiceStatus;
    B_DVR_OperationSettings operationSettings;
    DvrTest_Operation_Param param;


    /* Assign the AV path index to 0 */
    memset(&param, 0, sizeof(param));
    param.pathIndex = 0;

    B_Event_Reset(TSBServiceReadyEvent);

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        printf("\nTSB Service is not started\n");
        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param); 
        printf("Starting TSB Service...\n");
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReadyEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);


    for (i = 0; i < 100; i++) {
        BKNI_Sleep(5000);

        cmd = B_DVR_Generate_Random(7);

        switch(cmd) {

            case 0:
                    printf("\n****************************\n");
                    printf("Command: TSB playback pause");
                    printf("\n****************************\n");
                    if(g_dvrTest->streamPath[0].tsbService) {
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                        if (!tsbServiceStatus.tsbPlayback)
                        {
                            printf("TSB Playback is not started\n");
                            Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServicePlayStart, &param); 
                            printf("TSB Playback is starting...\n");
                        }
                        else
                        {
                            if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                            {
                                printf("TSB playback is not started. Start TSB playback\n");
                            }
                            else
                            {
                                B_DVR_OperationSettings operationSettings;
                                operationSettings.operation = eB_DVR_OperationPause;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                            }
                        }
                    }
                    else 
                    {
                        printf("TSB Service is not enabled\n");
                    }
            break;
            case 1:
                printf("\n****************************\n");
                printf("Command: TSB playback rewind");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastRewind;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 2:
                printf("\n****************************\n");
                printf("Command: TSB playback play");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 3:
                printf("\n****************************\n");
                printf("Command: TSB playback forward");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastForward;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 4:
                printf("\n****************************\n");
                printf("Command: Convert to Record");
                printf("\n****************************\n");
                if(g_dvrTest->streamPath[0].tsbService)
                {
                    B_DVR_ERROR rc = B_DVR_SUCCESS;
                    B_DVR_TSBServiceStatus tsbStatus;
                    B_DVR_TSBServicePermanentRecordingRequest recReq;
                    unsigned fileSuffix;

                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService, &tsbStatus);
                    fileSuffix = B_DVR_Generate_Random(100);
                    sprintf(recReq.programName, "PermRecord%d", fileSuffix);
                    recReq.recStartTime = tsbStatus.tsbRecStartTime;
                    recReq.recEndTime = tsbStatus.tsbRecEndTime;

                    printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
                    sprintf(recReq.subDir,"tsbConv");
                     rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[0].tsbService,&recReq);
                    if(rc!=B_DVR_SUCCESS)
                    {
                        printf("\n Invalid paramters");
                    }
                }
                else
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case 5:
                printf("\n****************************\n");
                printf("Command: Switch to Live mode");
                printf("\n****************************\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                    printf("It is already in the Live mode!!!\n");
                else {
                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                    if (tsbServiceStatus.tsbPlayback)
                    {
                        Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServicePlayStop, &param); 
                    }
                    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param); 
                }
            break;
            case 6:
                printf("\n****************************\n");
                printf("Command: Switch to TSB mode");
                printf("\n****************************\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                {
                    printf("Starting TSB Service...");
                    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStart, &param); 
                }
                else
                {
                    printf("TSB Service is already started\n");
                }
            break;
            default:
                printf("Unknown Command\n");
                break;
        }
    }

    Do_DvrTest_Operation(tc, eDvrTest_OperationTSBServiceStop, &param);


}
