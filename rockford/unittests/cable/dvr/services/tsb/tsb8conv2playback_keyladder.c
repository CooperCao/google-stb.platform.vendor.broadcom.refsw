/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * Module Description:
 * 8 TSB conversion, 2 TSB playback using keyladder and list of recordings
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#include "b_dvr_test.h"

BDBG_MODULE(dvr_test_8tsb_keyladder);

#define MAX_AVPATH 8
#define MAX_AV_DECODER 2
#define KEY_LENGTH     16

#define DURATION1     60000        /* 1 minute */
#define DURATION2    300000        /* 5 minutes */
#define DURATION3    600000      /* 10 minutes */
#define DURATION4   1200000     /* 20 minutes */
#define DURATION5   1800000     /* 30 minutes */

#define DURATION     DURATION4

unsigned char changeToLocalSession[16];
unsigned char changeToLocalControlWord[16];
unsigned char changeToIv[16];

unsigned char ucProcInKey3[16] = { 0x00 ,0x00 ,0x11 ,0x11 ,0x55 ,0x55 ,0x66 ,0x66 ,0x11 ,0x22 ,0x33 ,0x44 ,0x55 ,0x66 ,0x77 ,0x88 };
unsigned char ucProcInKey4[16] = { 0x28 ,0x30 ,0x46 ,0x70 ,0x71 ,0x4B ,0x8C ,0x75 ,0xEB ,0x46 ,0x04 ,0xBB ,0x96 ,0xD0 ,0x48 ,0x88};
unsigned char ivkeys[16] = { 0xad ,0xd6 ,0x9e ,0xa3 ,0x89 ,0xc8 ,0x17 ,0x72 ,0x1e ,0xd4 ,0x0e ,0xab ,0x3d ,0xbc ,0x7a ,0xf2 };

typedef struct {
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned volumeIndex;
} RecordingListThreadParam;

typedef struct {
    int *cmds;
} TSBTrickPlayThreadParam;

static int currentChannel = 2;

B_EventHandle TSBServiceReady;
B_EventHandle TSBConversionComplete;
B_EventHandle TSBRecordListComplete;
unsigned int num_TSBServiceReady = 0;
unsigned int num_TSBConversionComplete = 0;

char recordingList[MAX_AVPATH][B_DVR_MAX_FILE_NAME_LENGTH];
int TrickPlayCmds[5] = {0, 1, 2,3, -1};     /* TSB Trick Play Commands */


void procInKey3_sessionKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalSession[i] = ucProcInKey3[i];   
         BDBG_MSG(("[sessionKey] %d = 0x%x",i,changeToLocalSession[i]));
    }   
}

void procInKey4_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToLocalControlWord[i] = ucProcInKey4[i]);   
        BDBG_MSG(("[procInKey4] %d = 0x%x",i,changeToLocalControlWord[i]));    
    }   

}

void iv_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToIv[i] = ivkeys[i]);   
        BDBG_MSG(("[IV] %d = 0x%x",i,changeToIv[i]));    
    }   

}


void DvrTest_TuneChannel(int index)
{
    NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);
    g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrTest->streamPath[index].userParams.param1;
    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    g_dvrTest->streamPath[index].parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);

    NEXUS_Frontend_GetDefaultQamSettings(&g_dvrTest->streamPath[index].qamSettings);
    g_dvrTest->streamPath[index].qamSettings.frequency =  channelInfo[currentChannel].frequency;
    g_dvrTest->streamPath[index].qamSettings.mode = channelInfo[currentChannel].modulation;
    g_dvrTest->streamPath[index].qamSettings.annex = channelInfo[currentChannel].annex;
    g_dvrTest->streamPath[index].qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;

    rc = NEXUS_Frontend_TuneQam(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(2000);
    rc = NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamStatus);
    BDBG_ASSERT(!rc);
    printf("\n %s: receiver lock = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.receiverLock);
    printf("\n %s: Symbol rate = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.symbolRate);

    printf("\n %s: index %d >>>",__FUNCTION__,index);
    return;
}

void DvrTest_LiveDecodeStart(int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        /* NEXUS_PidChannelSettings settings; */

        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                    channelInfo[currentChannel].videoPids[0], NULL);
    }
    else
    {
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                   channelInfo[currentChannel].videoPids[0], NULL);
    }
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
    g_dvrTest->streamPath[index].videoProgram.codec = channelInfo[currentChannel].videoFormat[0];
    g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;

    NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
    NEXUS_Timebase_GetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);

    g_dvrTest->streamPath[index].stcSettings.autoConfigTimebase = true;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.offsetThreshold = 8;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.maxPcrError = 255;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableTimestampCorrection = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableJitterAdjustment = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */

    NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);
    if(!index)
    {

        if(g_dvrTest->streamPath[index].tsbService)
        {
            /* NEXUS_PidChannelSettings settings; */
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
        }
        else
        { 
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
        }
       NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
       g_dvrTest->streamPath[index].audioProgram.codec = channelInfo[currentChannel].audioFormat[0];
       g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPidChannels[0];
       g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
       NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);

    }
    g_dvrTest->streamPath[index].liveDecodeStarted=true;
    
    printf("\n %s:index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_LiveDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
        NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].audioPidChannels[0]);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].videoPidChannels[0]);
    g_dvrTest->streamPath[index].liveDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void B_DVR_TSB_Encryption_Config(int pathIndex)
{
	int dmaCount=0;
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;	
	
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
	g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;

	g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
	g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
	g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;

	if(pathIndex>=0 && pathIndex<2) /* CPS enc for main and sub*/
	{
		g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;	
		g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
		g_dvrTest->streamPath[pathIndex].drmService[0] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);		
		g_dvrTest->streamPath[pathIndex].drmService[1] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				
		printf("\n[%s]drm Opened\n",(pathIndex>0)? "sub":"main");	

		g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
		g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
		g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
		g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
		g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
		dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[0] ,&g_dvrTest->drmServiceSettings);
		dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[1] ,&g_dvrTest->drmServiceSettings);		
		printf("\n[%s]drm Configured\n",(pathIndex>0)? "sub":"main");	
		
	}
	else /* M2M enc except main and sub */
	{
		dmaCount = pathIndex-2;
		/* g_dvrTest->streamPath[pathIndex].dma = NEXUS_Dma_Open(0, NULL);	*/
		g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
		g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NEXUS_Dma_Open(0, NULL); 
		g_dvrTest->streamPath[pathIndex].drmService[0] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);						
		printf("\n[%d]drm Opened , dmaCount[%d]\n",pathIndex,dmaCount);	

		g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
		g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
		g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
		g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
		dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[0] ,&g_dvrTest->drmServiceSettings);
	}	

}

void B_DVR_TSB_Encryption_Start(int pathIndex, NEXUS_PidChannelHandle *handle)
{
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;
	bool tsbPlayback = false;

	if(pathIndex>=0 && pathIndex<2) 		
	{
		dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],*handle);
		dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],*(handle+1));	
	}
	else if(pathIndex>=2)
	{
		printf("\n %s pathIndex[%d]\n",__FUNCTION__,pathIndex);
		dvrError = B_DVR_TSBService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,
														g_dvrTest->streamPath[pathIndex].drmService[0],tsbPlayback);
	}
}

void B_DVR_TSB_Encryption_Stop(int pathIndex)
{
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;
	NEXUS_PidChannelHandle videoPidchHandle;
	NEXUS_PidChannelHandle audioPidchHandle; 
	bool tsbPlayback=false;
	unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
	if(pathIndex>=0 && pathIndex<2)
	{
	    videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
			                                                  channelInfo[currentChannel].videoPids[0]);

		audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
														      channelInfo[currentChannel].audioPids[0]);
		
		dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
		dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);
		
				dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[1]);		
	}else if(pathIndex>=2)
	{
		NEXUS_Dma_Close(g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma);
		dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);
	}
	
	dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[0]);
	BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
}

void B_DVR_TSB_Decryption_Config(int index , unsigned char *key,unsigned keylength)
{
	unsigned count=0;
	unsigned char session[16];
	unsigned char controlWd[16];
	unsigned char iv[16];
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;

	for(count=0;count<keylength;count++)
	{
		BDBG_MSG(("key[%d] for m2m = 0x%x\n",count,*(key+count)));
		if (count<16) session[count] = *(key+count);
		else if (count >=16 && count < 32) controlWd[count-(keylength/3)] = *(key+count);
		else if (count >= 32) iv[count - (keylength-16)] = *(key+count);		
	}

	g_dvrTest->pbDma[index] = NEXUS_Dma_Open(0, NULL);
	g_dvrTest->drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
	g_dvrTest->drmServiceRequest.controlWordKeyLoader = NULL;
	g_dvrTest->drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
	g_dvrTest->drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
	g_dvrTest->drmServiceRequest.keySlot = NULL;
	g_dvrTest->drmServiceRequest.dma = g_dvrTest->pbDma[index];
    g_dvrTest->drmServiceRequest.service = eB_DVR_ServicePlayback;
	g_dvrTest->drmPbService[index] = B_DVR_DRMService_Open(&g_dvrTest->drmServiceRequest);

	g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
	g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
	g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
	g_dvrTest->drmServiceSettings.sessionKeys = session;
	g_dvrTest->drmServiceSettings.keys = controlWd; 
	g_dvrTest->drmServiceSettings.ivKeys = iv;
	g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
	dvrError = B_DVR_DRMService_Configure(g_dvrTest->drmPbService[index],&g_dvrTest->drmServiceSettings);

}

void B_DVR_TSB_Decryption_Start(int index)
{
	bool tsbPlayback = true;
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;

	if(index>=0 && index<2)
	{
		dvrError = B_DVR_TSBService_AddDrmSettings(g_dvrTest->streamPath[index].tsbService,
														g_dvrTest->drmPbService[index],tsbPlayback);
	}
	else
	{
		printf("\nnot support for %d index playback",index);
	}

}

void B_DVR_TSB_Decryption_Stop(int index)
{
	bool tsbPlayback = true;
	B_DVR_ERROR dvrError = B_DVR_SUCCESS;

	if(index>=0 && index<2)
	{
		NEXUS_Dma_Close(g_dvrTest->pbDma[index]);
		dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[index].tsbService,tsbPlayback);
		dvrError = B_DVR_DRMService_Close(g_dvrTest->drmPbService[index]);
	}
	else
	{
		printf("\nnot support for %d index playback",index);
	}

}


void DvrTest_PlaybackDecodeStart(int index)
{
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_Media media;
    B_DVR_ERROR rc;
	unsigned keyLength;
	unsigned count=0;
	unsigned char key[48];
	
    printf("\n %s:index %d >>>",__FUNCTION__,index);

    if(g_dvrTest->streamPath[index].tsbService || g_dvrTest->streamPath[index].playbackService)
    {
        if(g_dvrTest->streamPath[index].tsbService)
        { 
            mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].tsbServiceRequest.volumeIndex;

            BSTD_UNUSED(playbackServiceSettings);
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            g_dvrTest->streamPath[index].playback = tsbServiceSettings.tsbPlaybackSettings.playback;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);            
        }
        else
        {
            mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].playbackServiceRequest.volumeIndex;
 
            BSTD_UNUSED(tsbServiceSettings);
            B_DVR_PlaybackService_GetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
            g_dvrTest->streamPath[index].playback = playbackServiceSettings.playback;
            playbackServiceSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            playbackServiceSettings.videoDecoder[1]=NULL;
            playbackServiceSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            playbackServiceSettings.audioDecoder[1]=NULL;
            playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_PlaybackService_SetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
        }

        rc = B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n media node not found");
            assert(!rc);
        }

		if(index>=0 && index<2) /* M2M decryption */
		{
			B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keyLength);
			B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,media.esStreamInfo[0].pid,key);
			
			BDBG_MSG(("keyLength = %d\n",keyLength));
			
			for(count=0;count<keyLength;count++){
				BDBG_MSG(("key[%d] = 0x%x\n",count,key[count]));}

			B_DVR_TSB_Decryption_Config(index,key,keyLength);			
		}
		
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = g_dvrTest->streamPath[index].videoDecoder;
        g_dvrTest->streamPath[index].videoPlaybackPidChannels[0] =
             NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[0].pid,&playbackPidSettings);

        if(!index)
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = g_dvrTest->streamPath[index].audioDecoder;
            g_dvrTest->streamPath[index].audioPlaybackPidChannels[0] = 
            NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[1].pid,&playbackPidSettings);
        }

		 B_DVR_TSB_Decryption_Start(index);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
        g_dvrTest->streamPath[index].videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
        g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPlaybackPidChannels[0];
        g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
        NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
            NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);
        }
        g_dvrTest->streamPath[index].playbackDecodeStarted=true;

    }
    else
    {
        printf("\n error!!!!!neither playback or tsbservice opened");
    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}


void DvrTest_PlaybackDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);

	B_DVR_TSB_Decryption_Stop(index);
	
    if(!index)
    {
        NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
    }
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
    g_dvrTest->streamPath[index].playbackDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_AudioVideoPathOpen(int index)
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
                                    NEXUS_AudioDecoderConnectorType_eStereo));
    }
    else
    {
        g_dvrTest->streamPath[index].audioDecoder = NULL;
    }
    
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_AudioVideoPathClose(int index)
{
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    if(!index)
    {
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioDecoder_Close(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_VideoWindow_RemoveAllInputs(g_dvrTest->streamPath[index].window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    NEXUS_VideoWindow_Close(g_dvrTest->streamPath[index].window);
    NEXUS_VideoDecoder_Close(g_dvrTest->streamPath[index].videoDecoder);
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
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


void B_DVR_TSB_Service_Start(int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	NEXUS_PidChannelHandle handle[2];
	B_DVR_MediaNodeSettings mediaNodeSettings;
	unsigned char totalKey[48];
	unsigned index;

    printf("TSB buffering begins\n");
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        if ((pathIndex == 0) || (pathIndex == 1))
            DvrTest_LiveDecodeStart(pathIndex);
    }
    printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
            sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
            "%s%d",TSB_SERVICE_PREFIX,pathIndex);
    printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
    sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
    g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
    B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

    BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

	B_DVR_TSB_Encryption_Config(pathIndex);
	
    BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);
    handle[0] = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,inputEsStream.esStreamInfo.pid);	

    inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);
    handle[1] = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,inputEsStream.esStreamInfo.pid);	

	mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
	mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
	mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
	mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
	BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
															mediaNodeSettings.programName,
															mediaNodeSettings.volumeIndex));
	for(index=0;index<48;index++)
	{
		 if(index < 16) totalKey[index] = changeToLocalSession[index];
		 else if (index >= 16 && index <32) totalKey[index] = changeToLocalControlWord[index-KEY_LENGTH];
		 else if(index >= 32) totalKey[index] = changeToIv[index -(KEY_LENGTH*2)];
	         BDBG_MSG(("[totalKey] %d = 0x%x\n",index,totalKey[index]));
	}				

	dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*3));				
	dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
														&mediaNodeSettings,
														channelInfo[currentChannel].videoPids[0],
														totalKey);
				
	dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*3));				
	dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
														&mediaNodeSettings,
														channelInfo[currentChannel].audioPids[0],
														totalKey);	 
	B_DVR_TSB_Encryption_Start(pathIndex,handle);

    dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
    if(dvrError!=B_DVR_SUCCESS)
    {
        printf("\n Error in starting the tsbService %d",pathIndex);
    }
    else
    {
        printf("tsbService started %d\n",pathIndex);
    }

}

void B_DVR_TSB_Service_Stop(int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
		B_DVR_TSB_Encryption_Stop(pathIndex);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
            DvrTest_PlaybackDecodeStop(pathIndex);
        }
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in stopping the timeshift service %d\n",pathIndex);
        }
        /* temporal workaround */
        BKNI_Sleep(1000);
        /* temporal workaround */
        B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
        dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].tsbService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in closing the timeshift service %d\n",pathIndex);
        }
    }
    else
    {
        printf("\n timeshift srvc not started");
    }

}

void B_DVR_TSB_Service_Play_Start(int pathIndex)
{
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus tsbServiceStatus;

    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        DvrTest_LiveDecodeStop(pathIndex);
    }
    DvrTest_PlaybackDecodeStart(pathIndex);
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
    operationSettings.operation = eB_DVR_OperationPause; 
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
}

void B_DVR_TSB_Service_Play_Stop(int pathIndex)
{
    B_DVR_OperationSettings operationSettings;

    if(!g_dvrTest->streamPath[pathIndex].tsbService)
    {
        printf("\n error TSBPlayStart without TSBServiceStart");
    }
    else
    {
        operationSettings.operation = eB_DVR_OperationTSBPlayStop;
        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
        DvrTest_PlaybackDecodeStop(pathIndex);
        if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
        {
            DvrTest_LiveDecodeStart(pathIndex);
        }
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


static void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};

    printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    BSTD_UNUSED(appContext);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                B_DVR_TSBServiceStatus tsbServiceStatus;

                B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                printf("\n Beginning Of TSB Conversion start time %lu endTime %lu",tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
                ++num_TSBServiceReady;
                if (num_TSBServiceReady == MAX_AVPATH)
                    B_Event_Set(TSBServiceReady);
            }
            else
            {
                printf("\n Beginning of background recording");

            }
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n End Of TSB Conversion");
            }
            else
            {
                printf("\n End of background recording");
            }
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n HD Stream recording in TSB Service");
            }
            else
            {
                printf("\n HD Stream recording in Record Service");
            }
        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n SD Stream recording in TSB Service");
            }
            else
            {
                printf("\n SD Stream recording in Record Service");
            }
        }
        break;
    case eB_DVR_EventTSBConverstionCompleted:
        {
            printf("\n TSB Conversion completed for the index path %d", index);
            ++num_TSBConversionComplete;
            printf("\n Total number of TSB Conversion completed is %d\n", num_TSBConversionComplete);
            if (num_TSBConversionComplete == MAX_AVPATH)
                B_Event_Set(TSBConversionComplete);
        }
        break;
    case eB_DVR_EventOverFlow:
        {
            printf("\n OverFlow");
        }
        break;
    case eB_DVR_EventUnderFlow:
        {
            printf("\nunderFlow");
        }
        break;
        case eB_DVR_EventStartOfPlayback:
            {
                if(service == eB_DVR_ServicePlayback)
                {
                  printf("\n Beginnning of playback stream. Pausing");
                }
                else
                {
                    printf("\n Beginning of TSB. Pausing\n");
                 }
            }
            break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream. Pausing");
            printf("\n Stop either TSBPlayback or Playback to enter live mode");
        }
        break;
    case eB_DVR_EventPlaybackAlarm:
        {
            printf("\n playback alarm event");
        }
        break;
    case eB_DVR_EventAbortPlayback:
        {
            printf("\n abort playback");
        }
        break;
    case eB_DVR_EventAbortRecord:
        {
            printf("\n abort record");
        }
        break;
    case eB_DVR_EventAbortTSBRecord:
        {
            printf("\n abort TSB record");
        }
        break;
    case eB_DVR_EventAbortTSBPlayback:
        {
            printf("\n abort TSB Playback");
        }
        break;
    case eB_DVR_EventDataInjectionCompleted:
        {
            printf("\n data Injection complete");
        }
        break;
    case eB_DVR_EventOutOfMediaStorage:
        {
            printf("\n no media Storage space");
        }
        break;
    case eB_DVR_EventOutOfNavigationStorage:
        {
            printf("\n no navigation Storage space");
        }
        break;
    case eB_DVR_EventOutOfMetaDataStorage:
        {
            printf("\n no metaData Storage space");
        }
        break;
    default:
        {
            printf("\n invalid event");
        }
   }
    printf("\n DvrTest_Callback <<<<<");
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



void B_DVR_TSB_Buffer_Preallocate(int numAVPath)
{
    int index;
    
    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_AllocSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings,
                                                    MAX_TSB_BUFFERS);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in allocating the tsb segments");
        }
    }

    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
}

void B_DVR_TSB_Buffer_Deallocate(int numAVPath)
{
    int index;

    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }
}

void print_media_attributes(unsigned attributeVal)
{
    int attribute = 32;

    do {
        if (attribute & attributeVal)
        {
            switch (attribute)
            {
                case B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM:
                    printf("Segmented_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM:
                    printf("Encrypted_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HD_STREAM:
                    printf("HD_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_AUDIO_ONLY_STREAM:
                    printf("Audio_Only_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HITS_STREAM:
                    printf("HITS_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS:
                    printf("Recording_In_Progress ");
                break;
                default:
                    printf("Unknown_Attribute_Value ");
                break;
            }
        }
        attribute = attribute >> 1;
    } while (attribute > 0);
}



void DVR_Recordings_Info(void *context)
{
    int volIndex = 0;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned recordingCount;
    unsigned index;
    unsigned mediaTime;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned keyLength;
    unsigned count=0;
    unsigned char key[16];
    RecordingListThreadParam *pParam;
    bool quit = false;

    pParam = (RecordingListThreadParam*)context;
    strcpy(subDir, pParam->subDir);
    volIndex= pParam->volumeIndex;

    BKNI_Free(context);

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;
    
    recordingCount = MAX_AVPATH;

    do {

        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

        printf("\n******************************************");
        printf("\nList of Recordings (%s)", subDir);
        printf("\n******************************************");

        for(index=0;index<recordingCount;index++)
        {
            printf("\n------------------------------------------");
            printf("\n program index: %d", index);
            printf("\n program name: %s", recordingList[index]);
            mediaNodeSettings.programName = recordingList[index];
            B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
            printf("\n program metaData file: %s",media.mediaNodeFileName);
            printf("\n media metaData file: %s",media.mediaFileName);
            printf("\n nav metaData file: %s",media.navFileName);
            printf("\n media size: %u",(unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset));
            printf("\n media attributes: ");
            print_media_attributes(media.mediaAttributes);
            printf("\n nav size: %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
            mediaTime = (unsigned)((media.mediaEndTime - media.mediaStartTime)/1000);
            printf("\n media time (seconds): %u", mediaTime);
            if (mediaTime >= DURATION/1000)
            {
                quit = true;
            }
            if (!(media.mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM)) 
            {
                printf("\n media stream is not encrypted");
            }
            else 
            {
                printf("\n media stream is encrypted");
                if (media.drmServiceType == eB_DVR_DRMServiceTypeBroadcastMedia)
                {
                    printf("\n drm service type: Broadcast Media");
                }
                else if (media.drmServiceType == eB_DVR_DRMServiceTypeContainerMedia)
                {
                    printf("\n drm service type: Container Media");
                }
                if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeProtected)
                {
                    printf("\n drm service key type: Protected");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeClear)
                {
                    printf("\n drm service key type: Clear");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeMax)
                {
                    printf("\n drm service key type: Not Available");
                }

                B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keyLength);
                B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,media.esStreamInfo[0].pid,key);

                printf("\n key length = %d\n", keyLength);
                printf(" key = ");
                for(count=0;count<keyLength;count++)
                {
                    printf("0x%x ", key[count]);
                }

            }
            printf("\n------------------------------------------\n");
        }
        BKNI_Sleep(3000);
        if (quit)
            break;
    }while(1);
    B_Event_Set(TSBRecordListComplete);
}


void TSB_Trick_Play(void *context)
{
    B_DVR_TSBServiceStatus tsbServiceStatus;
    B_DVR_OperationSettings operationSettings;
    TSBTrickPlayThreadParam *pParam;
    int *ptr, index;


    pParam = (TSBTrickPlayThreadParam*)context;
    ptr= pParam->cmds;

    BKNI_Free(context);

    do 
    {
        BKNI_Sleep(5000);

        if (*ptr >= 0)
        {
            switch(*ptr)
            {
                case 0:
                    printf("\n****************************\n");
                    printf("Command: TSB playback pause");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++) 
                    {
                        if(g_dvrTest->streamPath[index].tsbService) 
                        {
                            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                            if (!tsbServiceStatus.tsbPlayback)
                            {
                                printf("\nTSB Playback is not started for the index %d", index);
                                B_DVR_TSB_Service_Play_Start(index);
                                printf("\nTSB Playback is starting for the index %d...", index);
                            }
                            else
                            {
                                if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                                {
                                    printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                                }
                                else
                                {
                                    B_DVR_OperationSettings operationSettings;
                                    operationSettings.operation = eB_DVR_OperationPause;
                                    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                                }
                            }
                        }
                        else 
                        {
                            printf("\nTSB Service is not enabled for the index %d", index);
                        }
                    }
                break;
                case 1:
                    printf("\n****************************\n");
                    printf("Command: TSB playback rewind");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++) 
                    {
                        if(g_dvrTest->streamPath[index].tsbService) {
                            if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                            {
                                printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                            }
                            else
                            {
                                operationSettings.operation = eB_DVR_OperationFastRewind;
                                operationSettings.operationSpeed = 4;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                            }
                        }
                        else 
                        {
                            printf("\nTSB Service for the index %d is not enabled", index);
                        }
                    }
                break;
                case 2:
                    printf("\n****************************\n");
                    printf("Command: TSB playback play");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++)
                    {
                        if(g_dvrTest->streamPath[index].tsbService) {
                            if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                            {
                                printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                            }
                            else
                            {
                                operationSettings.operation = eB_DVR_OperationPlay;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                            }
                        }
                        else 
                        {
                            printf("\nTSB Service for the index %d is not enabled", index);
                        }
                    }
                break;
                case 3:
                    printf("\n****************************\n");
                    printf("Command: TSB playback forward");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++)
                    {
                        if(g_dvrTest->streamPath[index].tsbService) {
                            if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                            {
                                printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                            }
                            else
                            {
                                operationSettings.operation = eB_DVR_OperationFastForward;
                                operationSettings.operationSpeed = 4;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                            }
                        }
                        else
                        {
                            printf("\nTSB Service for the index %d is not enabled", index);
                        }
                    }
                break;
                default:
                    printf("Unknown Command\n");
                break;
            }
            ++ptr;
        }
        else
        {
            ptr = ptr-3;
        }
    }while(B_Event_Wait(TSBRecordListComplete, B_WAIT_NONE)!=B_ERROR_SUCCESS);
}


int main(void)
{
    int decoderIndex,frontendIndex;
    unsigned volumeIndex = 0;
    int i, index; 
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_ThreadHandle RecordingListThread;
    RecordingListThreadParam *pRecordingListParam;
    B_ThreadHandle TSBTrickPlayThread;
    TSBTrickPlayThreadParam *pTSBTrickPlayParam;


    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));
    
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(NULL);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

    B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);

    g_dvrTest->maxChannels = MAX_STATIC_CHANNELS;
    for (index=0; index<MAX_AVPATH; index++)
    {
        g_dvrTest->streamPath[index].currentChannel = currentChannel;
    }

    B_Os_Init();

    TSBServiceReady = B_Event_Create(NULL);
    TSBConversionComplete= B_Event_Create(NULL);
    TSBRecordListComplete = B_Event_Create(NULL);

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);

    printf("max stream path = %d\n", MAX_STREAM_PATH);
    printf("configured stream path = %d\n", MAX_AVPATH);

    for (frontendIndex=0,i=0;frontendIndex < MAX_STREAM_PATH; frontendIndex++ )
    {
        g_dvrTest->streamPath[frontendIndex].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
        if (g_dvrTest->streamPath[frontendIndex].frontend)
        {
            NEXUS_Frontend_GetCapabilities(g_dvrTest->streamPath[frontendIndex].frontend, &g_dvrTest->streamPath[frontendIndex].capabilities);
            if (g_dvrTest->streamPath[frontendIndex].capabilities.qam)
            {
                g_dvrTest->streamPath[i].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
                NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[i].frontend, &g_dvrTest->streamPath[i].qamStatus);
                DvrTest_TuneChannel(i);
                g_dvrTest->streamPath[i].liveDecodeStarted= false;
                i++;
            }
        }
    }


    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);



    for(decoderIndex=0;decoderIndex <MAX_AV_DECODER; decoderIndex++)
    {
        DvrTest_AudioVideoPathOpen(decoderIndex);
        DvrTest_LiveDecodeStart(decoderIndex);
        g_dvrTest->streamPath[decoderIndex].liveDecodeStarted = true;
        g_dvrTest->streamPath[decoderIndex].playbackDecodeStarted = false;
    }

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTest_Callback;
    g_dvrTest->dvrManager = B_DVR_Manager_Init(NULL);
    if(!g_dvrTest->dvrManager)
    {
        printf("Error in opening the dvr manager\n");
    }

    B_DVR_Manager_CreateMediaNodeList(g_dvrTest->dvrManager,0);

    B_DVR_TSB_Buffer_Preallocate(MAX_AVPATH);

    for (index = 0; index < MAX_AVPATH; index++)
    {
        if(!g_dvrTest->streamPath[index].tsbService)
        {
            printf("\nTSB Service for the path index %d is not started\n", index);
            B_DVR_TSB_Service_Start(index);
            printf("Starting TSB Service for the path index %d...\n", index);
        }
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

    /* starting TSB conversion */
    for (index = 0; index < MAX_AVPATH; index++)
    {
        printf("\n### TSB index %d is ready\n", index);
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_TSBServiceStatus tsbStatus;
            B_DVR_TSBServicePermanentRecordingRequest recReq;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            sprintf(recReq.programName, "TSBRecord%d", index);
            recReq.recStartTime = tsbStatus.tsbRecStartTime;
            recReq.recEndTime = recReq.recStartTime + DURATION;

            printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
            sprintf(recReq.subDir,"tsbConv");
            rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
            if(rc!=B_DVR_SUCCESS)
            {
                printf("\n TSB Conversion of AV path index % is failed", index);
            }
        }
    }

    /* starting TSB Trick Play */
    pTSBTrickPlayParam = BKNI_Malloc(sizeof(TSBTrickPlayThreadParam));
    if (!pTSBTrickPlayParam)
    {
        printf("Error in creating a TSBTrickPlayThreadParam\n");
    }

    pTSBTrickPlayParam->cmds = &TrickPlayCmds[0];
    
    TSBTrickPlayThread = B_Thread_Create("TSB TrickPlayThread", TSB_Trick_Play, pTSBTrickPlayParam, NULL);
    if (!TSBTrickPlayThread) 
    {
         printf("Error in creating a TSBTrickPlay thread\n");
    }


    /* start displaying of a list of TSB recordings */
    pRecordingListParam = BKNI_Malloc(sizeof(RecordingListThreadParam));
    if (!pRecordingListParam)
    {
        printf("Error in creating a RecordingListThreadParam\n");
    }
    
    sprintf(pRecordingListParam->subDir, "tsbConv");
    pRecordingListParam->volumeIndex = volumeIndex;
    
    RecordingListThread = B_Thread_Create("Recording List Thread", DVR_Recordings_Info, pRecordingListParam, NULL);
    if (!RecordingListThread) 
    {
         printf("Error in creating a RecordingList thread\n");
    }

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    printf("\n Stopping TSB Service Conversion");
    for (index = 0; index < MAX_AVPATH; index++)
    {
        B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[index].tsbService);
    }

    B_Thread_Destroy(TSBTrickPlayThread);
    B_Thread_Destroy(RecordingListThread);

    /* Stopping TSB playback service for the index 0 and 1 */
    for (index=0; index<2; index++)
    {
        B_DVR_TSBServiceStatus tsbStatus;
    
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbStatus);
        if (tsbStatus.tsbPlayback)
        {
            B_DVR_TSB_Service_Play_Stop(index);
        }
    }

    printf("\n Stopping all TSB Services");
    for(index=0;index< MAX_AVPATH; index++)
    {
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_TSBServiceStatus tsbStatus;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            /* tsbConversion is false */
            if(!tsbStatus.tsbCoversion)
            {
                printf("\n Stopping TSB Conversion Service index %d", index);
                B_DVR_TSB_Service_Stop(index);
            }
        }
    }


    printf("\n Closing all the services before stopping the test app");
    for(index=0;index< MAX_STREAM_PATH; index++)
    {
        if(index < MAX_AV_DECODER)
        { 
            if(g_dvrTest->streamPath[index].playbackDecodeStarted)
            {
                DvrTest_PlaybackDecodeStop(index);
            }
            if(g_dvrTest->streamPath[index].tsbService)
            {
                B_DVR_TSBService_Stop(g_dvrTest->streamPath[index].tsbService);
                B_DVR_TSBService_Close(g_dvrTest->streamPath[index].tsbService);
            }
            if(g_dvrTest->streamPath[index].playbackService)
            {
                B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[index].playbackService);
                B_DVR_PlaybackService_Close(g_dvrTest->streamPath[index].playbackService);
            }
            if(g_dvrTest->streamPath[index].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(index);
            }
            DvrTest_AudioVideoPathClose(index);
        }
        else
        {
            if(g_dvrTest->streamPath[index].recordService)
            {
                B_DVR_RecordService_Stop(g_dvrTest->streamPath[index].recordService);
                B_DVR_RecordService_Close(g_dvrTest->streamPath[index].recordService);
            }
        }
    }

    /* removing TSB conversion recordings from the disk */
    for (index=0; index < MAX_AVPATH; index++)
    {
        char recName[B_DVR_MAX_FILE_NAME_LENGTH];
        sprintf(recName, "TSBRecord%d", index);
        B_DVR_Delete_Record(volumeIndex, "tsbConv", recName);
    }

    B_DVR_TSB_Buffer_Deallocate(MAX_AVPATH);

    B_DVR_Manager_DestroyMediaNodeList(g_dvrTest->dvrManager,volumeIndex);

    B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage,volumeIndex);
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();

    return 0;
}
