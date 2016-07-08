/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#if NEXUS_HAS_SECURITY  && defined(HSM_TEST_TO_BE_INTEGRATED)

/* Nexus example app: single live a/v descrambling and decode from an input band */
/* IVs are different between EVEN/ODD key */
/* EVEN packets are decrypted inband */
/* ODD packets are decrypted by RMX loopback */
/* Input band -> Parser(0) -> M2M -> playback -> Decode -> Display */
/* Test stream is spiderman_aescbc_diff_iv.ts */

/* Nexus example app: playpump and decode */

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_playpump.h"
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_dma.h"
#include "nexus_memory.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_security.h"

#define MAX_READ (188*1024*4)
#define MAX_FIFO_SIZE (MAX_READ*8)
#define BUFFER_SIZE (188*102400)
#define LIVE_PCR

typedef struct
{
	unsigned char * pBuffer;
	unsigned int bufferSize;
	unsigned int startPos;
	unsigned int size;
    NEXUS_DmaHandle dma;
    NEXUS_KeySlotHandle EvenKeyHandle;
	NEXUS_KeySlotHandle OddKeyHandle;
	BKNI_MutexHandle mutex;
} Decryption_Buffer;

typedef struct 
{
	bool * pRunning;
	BKNI_EventHandle event_record;
	NEXUS_RecpumpHandle recpump;
	BKNI_EventHandle event;
	NEXUS_PlaypumpHandle playpump;
    NEXUS_VideoDecoderHandle videoDecoder;
	Decryption_Buffer buffer;

} Playback_Context;

static void PlaybackThread(void *);
static void RecordThread(void *);

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void ConfigureKeys(NEXUS_KeySlotHandle EvenKeyHandle, NEXUS_KeySlotHandle OddKeyHandle, 
						  unsigned char * EvenControlWord, unsigned char * OddControlWord,
						  unsigned char * iv_even, unsigned char * iv_odd)
{
    NEXUS_SecurityAlgorithmSettings algConfig;
    NEXUS_SecurityClearKey key;  

	/* Configure Algorithm */ 
    NEXUS_Security_GetDefaultAlgorithmSettings(&algConfig);
    algConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    algConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    algConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    algConfig.operation = NEXUS_SecurityOperation_eDecrypt;
	algConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = false;
    if ( NEXUS_Security_ConfigAlgorithm(EvenKeyHandle, &algConfig)!=0 )
	{
        printf("\nConfigure Algorithn failed for EVEN key failed \n");
    }    
	algConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = true;
	if ( NEXUS_Security_ConfigAlgorithm(OddKeyHandle, &algConfig)!=0 )
	{
        printf("\nConfigure Algorithm failed for ODD key failed \n");
    }

	/* Load Key and IV  for EVEN */
	NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keySize = 16;
    BKNI_Memcpy(key.keyData, EvenControlWord, key.keySize);
    if (NEXUS_Security_LoadClearKey(EvenKeyHandle, &key) != 0) 
	{
        printf("\nLoad EVEN key failed \n");
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    key.keySize = 16;
    BKNI_Memcpy(key.keyData, iv_even, key.keySize);
    if (NEXUS_Security_LoadClearKey(EvenKeyHandle, &key) != 0) 
	{
        printf("\nLoad EVEN IV failed \n");
    }

	/* Load Key and IV  for ODD */
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keySize = 16;
    BKNI_Memcpy(key.keyData, OddControlWord, key.keySize);
    if (NEXUS_Security_LoadClearKey(OddKeyHandle, &key) != 0) 
	{
        printf("\nLoad ODD key failed \n");
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    key.keySize = 16;
    BKNI_Memcpy(key.keyData, iv_odd, key.keySize);
    if (NEXUS_Security_LoadClearKey(OddKeyHandle, &key) != 0) 
	{
        printf("\nLoad ODD IV failed \n");
    }

}

static void ConfigureVideoKeys(NEXUS_KeySlotHandle EvenKeyHandle, NEXUS_KeySlotHandle OddKeyHandle)
{
	unsigned char EvenControlWord[] = { 
		                                    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
		                                    0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };   
    unsigned char OddControlWord[] = { 
		                                    0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef,
											0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, };
                                            
    unsigned char iv_even[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned char iv_odd[]={0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1};

	ConfigureKeys (EvenKeyHandle, OddKeyHandle, EvenControlWord, OddControlWord, iv_even, iv_odd);

}

static void ConfigureAudioKeys(NEXUS_KeySlotHandle EvenKeyHandle, NEXUS_KeySlotHandle OddKeyHandle)
{
	/* Audio keys happen to be the same for this test.  But they can be different */
	unsigned char EvenControlWord[] = { 
		                                    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
		                                    0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };   
    unsigned char OddControlWord[] = { 
		                                    0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef,
											0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, };
                                            
    unsigned char iv_even[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned char iv_odd[]={0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1};

	ConfigureKeys (EvenKeyHandle, OddKeyHandle, EvenControlWord, OddControlWord, iv_even, iv_odd);

}
static void DMATransfer (unsigned char * pIn, unsigned char * pOut, unsigned int size, NEXUS_DmaHandle hDma, NEXUS_KeySlotHandle hKey)
{
	NEXUS_DmaJobSettings  dmaJobSetting;
	NEXUS_DmaSettings dmaSettings;
	NEXUS_DmaJobBlockSettings blockSettings;
	NEXUS_DmaJobHandle hDmaJob;
    NEXUS_DmaJobStatus jobStatus;

	dmaSettings.endianMode = NEXUS_DmaEndianMode_eLittle;
	dmaSettings.swapMode = NEXUS_DmaSwapMode_eNone;
	NEXUS_Dma_SetSettings (hDma, &dmaSettings);

	NEXUS_DmaJob_GetDefaultSettings (&dmaJobSetting);
	dmaJobSetting.numBlocks = 1;
	dmaJobSetting.completionCallback.callback = NULL;
	dmaJobSetting.dataFormat = NEXUS_DmaDataFormat_eMpeg;
	dmaJobSetting.keySlot = hKey;
	if ( (hDmaJob = NEXUS_DmaJob_Create (hDma, &dmaJobSetting)) == NULL )
		return;
    
	NEXUS_DmaJob_GetDefaultBlockSettings (&blockSettings);
    blockSettings.pSrcAddr = pIn;    
    blockSettings.pDestAddr = pOut;    
    blockSettings.blockSize = size;  
    blockSettings.resetCrypto = true;              
    blockSettings.scatterGatherCryptoStart = true; 
    blockSettings.scatterGatherCryptoEnd = true;   
	NEXUS_DmaJob_ProcessBlocks(hDmaJob, &blockSettings, 1);
	
    jobStatus.currentState = NEXUS_DmaJobState_eUnknown;
    while (jobStatus.currentState != NEXUS_DmaJobState_eComplete)
    {
		/*BDBG_ERR(("Waiting... status=%d\n", jobStatus.currentState));*/
       NEXUS_DmaJob_GetStatus(hDmaJob, &jobStatus);
	   if ( jobStatus.currentState == NEXUS_DmaJobState_eComplete )
		   break;
		BKNI_Sleep(1);
    } 

	NEXUS_DmaJob_Destroy (hDmaJob);
}



static unsigned int WriteToBuffer (Decryption_Buffer * pBuffer, unsigned char * pIn, unsigned int size)
{
	unsigned int data_write;
	unsigned char * pData; 
	unsigned int buffer_size = pBuffer->bufferSize - pBuffer->size;

	BKNI_AcquireMutex (pBuffer->mutex);

	if ( buffer_size < size )
		data_write = buffer_size;
	else
		data_write = size;

	if ( data_write )
	{
		if ((pBuffer->startPos+pBuffer->size)>= pBuffer->bufferSize )
		{
			/* wrap around */
			pData = pBuffer->pBuffer + (pBuffer->startPos+pBuffer->size - pBuffer->bufferSize);
			DMATransfer (pIn,pData,data_write, pBuffer->dma, NULL);
		}
		else
		{
			unsigned int remainning = pBuffer->bufferSize-pBuffer->startPos-pBuffer->size;
			pData = pBuffer->pBuffer + (pBuffer->startPos+pBuffer->size);
			if ( remainning < data_write )
			{
				/* wrap around */
				DMATransfer (pIn,pData,remainning, pBuffer->dma, NULL);
				DMATransfer (pIn+remainning, pBuffer->pBuffer, data_write-remainning, pBuffer->dma, NULL);
			}
			else
			{
				DMATransfer (pIn,pData,data_write, pBuffer->dma, NULL);
			}
		}

		pBuffer->size += data_write;
	}

	BKNI_ReleaseMutex (pBuffer->mutex);
	return data_write;
}

static unsigned int ReadFromBuffer (Decryption_Buffer * pBuffer, unsigned char * pOut, unsigned int size)
{
	unsigned int data_read;
	unsigned char * pData; 

	BKNI_AcquireMutex (pBuffer->mutex);
	if ( pBuffer->size < size )
		data_read = pBuffer->size;
	else
		data_read = size;

	if (data_read)
	{
		pData = pBuffer->pBuffer + pBuffer->startPos; 
		if ((pBuffer->startPos+data_read)> pBuffer->bufferSize )
		{
			/* Wrap around */
			unsigned int remainning = pBuffer->bufferSize-pBuffer->startPos;
			DMATransfer (pData,pOut,remainning, pBuffer->dma, NULL);
			DMATransfer (pBuffer->pBuffer,pOut+remainning, data_read-remainning, pBuffer->dma, NULL);

			pBuffer->startPos = data_read-remainning;
		}
		else
		{
			DMATransfer (pData,pOut,data_read, pBuffer->dma, NULL);
			pBuffer->startPos += data_read;
			if ( pBuffer->startPos==pBuffer->bufferSize )
				pBuffer->startPos = 0;
		}

		pBuffer->size -= data_read;
	}

	BKNI_ReleaseMutex (pBuffer->mutex);
	return data_read;

}

int main(void)  
{
	NEXUS_Error rc;
    NEXUS_DmaHandle dma;

    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump, playpump_audio;
    BKNI_EventHandle event, event_audio;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
	Playback_Context playback_context, playback_context_audio;
    pthread_t playback_thread, playback_thread_audio;
    pthread_t record_thread, record_thread_audio;

    BKNI_EventHandle event_record, event_record_audio;
    NEXUS_InputBand inputBand;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
	NEXUS_RecpumpOpenSettings openSettings;
    NEXUS_RecpumpHandle recpump, recpump_audio;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle pidChannel[2];
	bool bRunning = true;

    NEXUS_KeySlotHandle EvenKeyHandle, OddKeyHandle;
    NEXUS_KeySlotHandle EvenKeyHandle_audio, OddKeyHandle_audio;
    NEXUS_SecurityKeySlotSettings keySettings;
	NEXUS_PidChannelStatus pidStatus;
	unsigned int videoPID, audioPID;


	NEXUS_Platform_GetDefaultSettings(&platformSettings);
    /*platformSettings.openFrontend = false;*/
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

	dma = NEXUS_Dma_Open(0, NULL);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    BKNI_CreateEvent(&event_record);
    BKNI_CreateEvent(&event_record_audio);

    recpump = NEXUS_Recpump_Open(0, NULL);

	NEXUS_Recpump_GetDefaultOpenSettings(&openSettings);
	openSettings.data.bufferSize = ((188/4)*4096) + 68;
	openSettings.data.dataReadyThreshold = ((188/4)*32);
    recpump_audio = NEXUS_Recpump_Open(1, &openSettings);


    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event_record;
    recpumpSettings.index.dataReady.callback = NULL; /* same callback */
    recpumpSettings.index.dataReady.context = NULL; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = NULL;
    recpumpSettings.index.overflow.context = NULL;
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    recpumpSettings.data.dataReady.context = event_record_audio;
    NEXUS_Recpump_SetSettings(recpump_audio, &recpumpSettings);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
	/*addPidChannelSettings.pidTypeSettings.video.index = true;*/
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    pidChannel[0] = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[0], &addPidChannelSettings);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eAudio;
    pidChannel[1] = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    NEXUS_Recpump_AddPidChannel(recpump_audio, pidChannel[1], NULL);


	/* Playback set up */
    BKNI_CreateEvent(&event);
    BKNI_CreateEvent(&event_audio);
	playpump = NEXUS_Playpump_Open(0, NULL);
	playpump_audio = NEXUS_Playpump_Open(1, NULL);

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.context = event_audio;
    NEXUS_Playpump_SetSettings(playpump_audio, &playpumpSettings);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 0x11, NULL);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump_audio, 0x14, NULL);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
#ifdef LIVE_PCR
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
	stcSettings.modeSettings.pcr.pidChannel = pidChannel[0];
#else
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcSettings.modeSettings.pcr.pidChannel = videoPidChannel;
#endif
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display.*/
    display = NEXUS_Display_Open(0, NULL);
    #if NEXUS_NUM_COMPOSITE_OUTPUTS
    if ( platformConfig.outputs.composite[0] ) {
        NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
    #endif    
	
	window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

	NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
	NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Bring up audio decoders and outputs */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif

	NEXUS_AudioDecoder_GetSettings(pcmDecoder, &audioDecoderSettings);
	NEXUS_AudioDecoder_SetSettings(pcmDecoder, &audioDecoderSettings);

    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
		NEXUS_AudioDecoder_GetSettings(compressedDecoder, &audioDecoderSettings);
		NEXUS_AudioDecoder_SetSettings(compressedDecoder, &audioDecoderSettings);
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    EvenKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!EvenKeyHandle) {
        printf("\nAllocate EVEN keyslot failed \n");
        return 1;
    }
    OddKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!OddKeyHandle) {
        printf("\nAllocate ODD keyslot failed \n");
        return 1;
    }
    EvenKeyHandle_audio = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!EvenKeyHandle_audio) {
        printf("\nAllocate EVEN keyslot failed (Audio)\n");
        return 1;
    }
    OddKeyHandle_audio = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!OddKeyHandle_audio) {
        printf("\nAllocate ODD keyslot failed (Audio)\n");
        return 1;
    }
	
	/* Configure video keys */
	ConfigureVideoKeys (EvenKeyHandle, OddKeyHandle);

	/* Configure audio keys */
	ConfigureAudioKeys (EvenKeyHandle_audio, OddKeyHandle_audio);

	/* Add video PID channel to EVEN keyslot */
	NEXUS_PidChannel_GetStatus (pidChannel[0], &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(EvenKeyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	}

	/* Add audio PID channel to EVEN keyslot */
	NEXUS_PidChannel_GetStatus (pidChannel[1], &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(EvenKeyHandle_audio, audioPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	} 

	NEXUS_PidChannel_GetStatus (videoPidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(OddKeyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	}

	/* Add audio PID channel to EVEN keyslot */
	NEXUS_PidChannel_GetStatus (audioPidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(OddKeyHandle_audio, audioPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
	} 

	/* Start Playback thread video */
	playback_context.pRunning = &bRunning;
	playback_context.event_record = event_record;
	playback_context.recpump = recpump;
	playback_context.pRunning = &bRunning;
	playback_context.event = event;
	playback_context.playpump = playpump;
	playback_context.videoDecoder = videoDecoder;
	NEXUS_Memory_Allocate(BUFFER_SIZE, NULL, (void **)&playback_context.buffer.pBuffer);
	playback_context.buffer.bufferSize = BUFFER_SIZE;
	playback_context.buffer.startPos = 0;
	playback_context.buffer.size = 0;
	playback_context.buffer.dma = dma;
	playback_context.buffer.EvenKeyHandle = EvenKeyHandle;
	playback_context.buffer.OddKeyHandle = OddKeyHandle;
	BKNI_CreateMutex(&playback_context.buffer.mutex);

    rc = pthread_create(&playback_thread, NULL, PlaybackThread, &playback_context);
    BDBG_ASSERT(!rc);
    rc = pthread_create(&record_thread, NULL, RecordThread, &playback_context);
    BDBG_ASSERT(!rc);

	/* Start Playback thread audio */
	playback_context_audio.pRunning = &bRunning;
	playback_context_audio.event_record = event_record_audio;
	playback_context_audio.recpump = recpump_audio;
	playback_context_audio.pRunning = &bRunning;
	playback_context_audio.event = event_audio;
	playback_context_audio.playpump = playpump_audio;
	playback_context_audio.videoDecoder = videoDecoder;
	playback_context_audio.buffer.bufferSize = BUFFER_SIZE>>8;
	NEXUS_Memory_Allocate(playback_context_audio.buffer.bufferSize, NULL, (void **)&playback_context_audio.buffer.pBuffer);
	playback_context_audio.buffer.startPos = 0;
	playback_context_audio.buffer.size = 0;
	playback_context_audio.buffer.dma = dma;
	playback_context_audio.buffer.EvenKeyHandle = EvenKeyHandle_audio;
	playback_context_audio.buffer.OddKeyHandle = OddKeyHandle_audio;
	/*BKNI_CreateMutex(&playback_context_audio.buffer.mutex);*/
	playback_context_audio.buffer.mutex = playback_context.buffer.mutex;

    rc = pthread_create(&playback_thread_audio, NULL, PlaybackThread, &playback_context_audio);
    BDBG_ASSERT(!rc);
    rc = pthread_create(&record_thread_audio, NULL, RecordThread, &playback_context_audio);
    BDBG_ASSERT(!rc);
#if 1
	/* Loop forever */
	while (1)
	{
		BKNI_Sleep (1000);
	}
#else
    printf("Press ENTER to stop decode\n");
    getchar();

	bRunning = false;

	/* Add cleanup code later */
#endif
    return 0;

    return 1;

}

#define TIME_OUT BKNI_INFINITE


static void RecordThread(void * pParam)
{
	Playback_Context *pContext = (Playback_Context *)pParam;
	volatile bool * pRunning;
	BKNI_EventHandle event_record;
	NEXUS_RecpumpHandle recpump;
	Decryption_Buffer * pBuffer;

	printf("\nRecord Thread started...\n");
	pRunning = pContext->pRunning;
	event_record = pContext->event_record;
	recpump = pContext->recpump;
	pBuffer = &pContext->buffer;

    NEXUS_Recpump_Start(recpump);

	while (*pRunning) 
	{
		const void *buffer_in;
        size_t buffer_in_size, n;
        NEXUS_Error rc;

        if (NEXUS_Recpump_GetDataBuffer(recpump, &buffer_in, &buffer_in_size))
            break;
        if (buffer_in_size == 0 ) {
            BKNI_WaitForEvent(event_record, TIME_OUT);
				continue;
        }


		n = WriteToBuffer (pBuffer, (unsigned char *)buffer_in, buffer_in_size);	
		if (n!=0)
		{
			/*printf("Wrote %d bytes\n", n);*/
			rc =NEXUS_Recpump_DataReadComplete(recpump, n);
			BDBG_ASSERT(!rc);
		}

    }

	return;
}

static void PlaybackThread(void * pParam)
{
	Playback_Context *pContext = (Playback_Context *)pParam;
	volatile bool * pRunning;
	BKNI_EventHandle event;
	NEXUS_PlaypumpHandle playpump;
    NEXUS_VideoDecoderHandle videoDecoder;
	Decryption_Buffer * pBuffer;

	printf("\nPlayback Thread started...\n");
	pRunning = pContext->pRunning;
	event = pContext->event;
	playpump = pContext->playpump;
	videoDecoder = pContext->videoDecoder;
	pBuffer = &pContext->buffer;

    NEXUS_Playpump_Start(playpump);

	while (*pRunning) {
        void *buffer;
        size_t buffer_size;
        size_t n;
        NEXUS_Error rc;

		if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size))
            break;
		if (buffer_size == 0) {
            BKNI_WaitForEvent(event, TIME_OUT);
				continue;
        }



        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

		n = ReadFromBuffer (pBuffer, buffer, buffer_size);
		if (n != 0 )
		{
			/*printf("Read %d bytes\n", n);*/

			rc = NEXUS_Playpump_WriteComplete(playpump, 0, n);
			BDBG_ASSERT(!rc);
		}
    }
	return;
}


#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
