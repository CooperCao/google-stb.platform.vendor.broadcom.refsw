/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

/* Example to record stream with CPS AES using nexus */

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_security.h"
#include "nexus_recpump.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#define ENABLE_CRYPTO
#define NUM_STATIC_CHANNELS             4
#define MAX_PROGRAM_TITLE_LENGTH       128
#define NUM_AUDIO_STREAMS               4
#define NUM_VIDEO_STREAMS               4
#define MAX_STREAM_PATH				  4

NEXUS_PlatformSettings platformSettings;
NEXUS_FrontendUserParameters userParams;
NEXUS_FrontendHandle frontend=NULL;
NEXUS_FrontendQamSettings qamSettings;
NEXUS_PlatformConfiguration platformConfig;
NEXUS_ParserBand parserBand;
NEXUS_ParserBandSettings parserBandSettings;
NEXUS_StcChannelHandle stcChannel;
NEXUS_StcChannelSettings stcSettings;
NEXUS_FrontendCapabilities capabilities;
NEXUS_FrontendQamStatus qamStatus;
unsigned frontendIndex;

static int currentChannel = 0;

typedef struct g_channelInfo
{
    NEXUS_FrontendQamAnnex annex;
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolrate;
    unsigned int  frequency;
    char programTitle[MAX_PROGRAM_TITLE_LENGTH];
    int numAudioStreams;
    int numVideoStreams;
    unsigned int  videoPids[NUM_VIDEO_STREAMS];
    unsigned int  audioPids[NUM_AUDIO_STREAMS];
    unsigned int  pcrPid[NUM_VIDEO_STREAMS];
    NEXUS_VideoCodec videoFormat[NUM_VIDEO_STREAMS];
    NEXUS_AudioCodec audioFormat[NUM_AUDIO_STREAMS];
} g_channelInfo;

static g_channelInfo channelInfo[NUM_STATIC_CHANNELS] =
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

void show_ChannelMap(void)
{
    int channelCount, esStreamCount;
    for (channelCount = 0; channelCount < NUM_STATIC_CHANNELS; channelCount++)
    {
        printf("channel index (%d)\n", channelCount);
        printf("\tannex (%d)\n",channelInfo[channelCount].annex);
        printf("\tmodulation (%d)\n",channelInfo[channelCount].modulation);
        printf("\tfrequency (%d)\n", channelInfo[channelCount].frequency);
        printf("\tsymbolrate (%d)\n", channelInfo[channelCount].symbolrate);
        printf("\tprogram_title (%s)\n",channelInfo[channelCount].programTitle);
        printf("\tno of audio streams (%d)\n",channelInfo[channelCount].numAudioStreams);
        printf("\tno of video streams (%d)\n",channelInfo[channelCount].numVideoStreams);
        printf("\tvideo_pids/format ");
        for(esStreamCount=0;esStreamCount<channelInfo[channelCount].numVideoStreams;esStreamCount++)
        {
         printf("0x%x/%d ",channelInfo[channelCount].videoPids[esStreamCount],channelInfo[channelCount].videoFormat[esStreamCount]);
        }
        printf("\n");
        printf("\taudioPids/formats " );
        for(esStreamCount=0;esStreamCount<channelInfo[channelCount].numAudioStreams;esStreamCount++)
        {
         printf("0x%x/%d ", channelInfo[channelCount].audioPids[esStreamCount],channelInfo[channelCount].audioFormat[esStreamCount]);
        }
        printf("\n");
        printf("\tpcrPid ");
        for(esStreamCount=0;esStreamCount<channelInfo[channelCount].numVideoStreams;esStreamCount++)
        {
          printf("0x%x ",channelInfo[channelCount].pcrPid[channelCount]);
        }
        printf("\n");
    }
}

void tuneChannel(int channel)
{
	NEXUS_Frontend_GetUserParameters(frontend, &userParams);
	parserBand = (NEXUS_ParserBand)userParams.param1;
	NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
	parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;
	parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
	parserBandSettings.transportType = NEXUS_TransportType_eTs;
	NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = channelInfo[channel].frequency;
	qamSettings.mode = channelInfo[channel].modulation;
	qamSettings.annex = channelInfo[channel].annex;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;

	NEXUS_Frontend_TuneQam(frontend,&qamSettings);
	BKNI_Sleep(2000);
	NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
	printf("receiver lock = %d\n",qamStatus.receiverLock);
	printf("symbol Rate = %d\n",qamStatus.symbolRate);
}


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

static NEXUS_Error ConfigureKeySlotFor3DesCA (NEXUS_KeySlotHandle keyHandle,
											 unsigned char * pKeyEven,
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity);
static NEXUS_Error ConfigureKeySlotForAesCPS (NEXUS_KeySlotHandle keyHandle,
											 unsigned char * pKeyEven,
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity);

int main(void)
{
    FILE *data_file;
    FILE *index_file;
    const char data_fname[] = "videos/decode_aes_cps.mpg";
    const char index_fname[] = "videos/decode_aes_cps.idx";
    BKNI_EventHandle event;

    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    NEXUS_RecpumpHandle recpump;
	unsigned int videoPID, audioPID, counter = 0;
	NEXUS_KeySlotHandle videoKeyHandle = NULL;
	NEXUS_KeySlotHandle audioKeyHandle = NULL;
	NEXUS_KeySlotHandle videoKeyHandle2 = NULL;
	NEXUS_KeySlotHandle audioKeyHandle2 = NULL;
	NEXUS_PidChannelStatus pidStatus;
	NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
	NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;

    unsigned char VidEvenControlWord[] = {0x01,0xB9,0x91,0x67,0xE0,0xE3,0xE9,0xAB,0x07,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB};
    unsigned char VidOddControlWord[] = {0x01,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB,0x07,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB};
    unsigned char AudEvenControlWord[] = {0x01,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB,0x07,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB};
    unsigned char AudOddControlWord[] = {0x01,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB,0x07,0xB9,0x91,0x67, 0xE0,0xE3,0xE9,0xAB};

	unsigned char VidEvenCpsControlWord[] = {0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73};
    unsigned char VidOddCpsControlWord[] = {0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73};
	unsigned char AudEvenCpsControlWord[] = {0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73};
    unsigned char AudOddCpsControlWord[] = {0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73};

	show_ChannelMap();

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    for ( frontendIndex = 0; frontendIndex < MAX_STREAM_PATH; frontendIndex++ )
    {
        frontend = platformConfig.frontend[frontendIndex];

        if (frontend) {
            NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
			printf("[%d] frontend , qam(%d)\n",frontendIndex,capabilities.qam );
            /* Does this frontend support qam? */
            if ( capabilities.qam )
            {
				NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
				tuneChannel(currentChannel);
                break;
            }
        }
    }

    if (NULL == frontend )
    {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        return 0;
    }


    printf("press ENTER to start record\n");
    getchar();

    BKNI_CreateEvent(&event);
    recpump = NEXUS_Recpump_Open(0, NULL);

    data_file = fopen(data_fname, "wb");
    if (!data_file) {
        fprintf(stderr, "can't open data file:%s\n", data_fname);
        goto error;
    }
    index_file = fopen(index_fname, "wb");
    if (!index_file) {
        fprintf(stderr, "can't open index file:%s\n", index_fname);
        goto error;
    }

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = event; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    NEXUS_Recpump_AddPidChannel(recpump,
        videoPidChannel=NEXUS_PidChannel_Open(parserBand, 0x11, NULL), /* open it again */
        &addPidChannelSettings);
    NEXUS_Recpump_AddPidChannel(recpump,
        audioPidChannel=NEXUS_PidChannel_Open(parserBand, 0x14, NULL), /* open it again */
        NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
 /*   videoProgram.stcChannel = stcChannel;*/
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
/*    audioProgram.stcChannel = stcChannel;*/

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	NEXUS_AudioOutput_AddInput(
		  NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
		  NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	/***************************************************************************************
		Config CA descrambler
	***************************************************************************************/
#if defined(ENABLE_CRYPTO)
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp;

	/* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !videoKeyHandle)
	{
		printf("\nAllocate CACP video keyslot failed \n");
		return 1;
	}
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if ( !audioKeyHandle)
	{
		printf("\nAllocate CACP audio keyslot failed \n");
		return 1;
	}


	invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eOddAndEven;
	invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
	invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCa;
	invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
	NEXUS_Security_InvalidateKey(videoKeyHandle, &invSettings);
	NEXUS_Security_InvalidateKey(audioKeyHandle, &invSettings);


	invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eOddAndEven;
	invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
	invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCps;
	invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
	NEXUS_Security_InvalidateKey(videoKeyHandle, &invSettings);
	NEXUS_Security_InvalidateKey(audioKeyHandle, &invSettings);


	invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eOddAndEven;
	invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
	invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCpd;
	invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
	NEXUS_Security_InvalidateKey(videoKeyHandle, &invSettings);
	NEXUS_Security_InvalidateKey(audioKeyHandle, &invSettings);



	/* Config AV algorithms */
	if ( ConfigureKeySlotFor3DesCA (videoKeyHandle,
									VidEvenControlWord,
									VidOddControlWord,
									NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig video CA Algorithm failed for video keyslot 1 \n");
		return 1;
	}


	if ( ConfigureKeySlotFor3DesCA (audioKeyHandle,
									AudEvenControlWord,
									AudOddControlWord,
									NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig audio CA Algorithm failed for audio keyslot 1 \n");
		return 1;
	}

	printf ("\n CPS scrambler\n");

	/***************************************************************************************
		Config CPS (CP scrambler)
	***************************************************************************************/
	if ( ConfigureKeySlotForAesCPS (videoKeyHandle,
									VidEvenCpsControlWord,
									VidOddCpsControlWord,
                                    NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed for video keyslot 1 \n");
		return 1;
	}

	if ( ConfigureKeySlotForAesCPS (audioKeyHandle,
									AudEvenCpsControlWord,
									AudOddCpsControlWord,
                                    NEXUS_SecurityAlgorithmScPolarity_eEven) != 0 )
	{
		printf("\nConfig audio CPS Algorithm failed for audio keyslot 1 \n");
		return 1;
	}

	/* Add video PID channel to keyslot */
	NEXUS_PidChannel_GetStatus (videoPidChannel, &pidStatus);
	videoPID = pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID)!= 0 )
	{
		printf("\nConfigPIDPointerTable failed \n");
		return 1;
	}

	NEXUS_PidChannel_GetStatus (audioPidChannel, &pidStatus);
	audioPID = pidStatus.pidChannelIndex;
	NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID);

	printf ("\nSecurity Config OK\n");
#endif

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

	NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

	NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
	stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
	stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
	NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);


	NEXUS_Frontend_TuneQam(frontend, &qamSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);

    NEXUS_Recpump_Start(recpump);

    while (1) {

        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;

        if (NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size))
            break;
        if (NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size))
            break;
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
		}

			if (data_buffer_size) {
				n = fwrite(data_buffer, 1, data_buffer_size, data_file);
				if (n < 0) goto error;
				NEXUS_Recpump_DataReadComplete(recpump, n);
		}
			if (index_buffer_size) {
				n = fwrite(index_buffer, 1, index_buffer_size, index_file);
				if (n < 0) goto error;
				NEXUS_Recpump_IndexReadComplete(recpump, n);
		}
		/* Switch SC polarity for output packets */
				counter++;
#if defined(ENABLE_CRYPTO )
				/* simply disable keyslots altogether */

				if ( counter <= 10)
				{
					if(counter %2)
					{
							printf("H1\n");
							ConfigureKeySlotForAesCPS (videoKeyHandle,VidEvenCpsControlWord, VidOddCpsControlWord,NEXUS_SecurityAlgorithmScPolarity_eEven);
							ConfigureKeySlotForAesCPS (audioKeyHandle,AudEvenCpsControlWord, AudOddCpsControlWord,NEXUS_SecurityAlgorithmScPolarity_eEven);
							BKNI_Sleep(1000);
					}
					else
					{
							printf("H2\n");
							ConfigureKeySlotForAesCPS (videoKeyHandle,VidEvenCpsControlWord, VidOddCpsControlWord,NEXUS_SecurityAlgorithmScPolarity_eEven);
							ConfigureKeySlotForAesCPS (audioKeyHandle,AudEvenCpsControlWord, AudOddCpsControlWord,NEXUS_SecurityAlgorithmScPolarity_eEven);
							BKNI_Sleep(1000);
					}
				}
				else
				{
						counter = 0;
				}
#endif

	}
	printf("Press enter to tune again\n");
	   getchar();

    /* loops forever */
error:
    return 1;
}


static NEXUS_Error ConfigureKeySlotFor3DesCA (NEXUS_KeySlotHandle keyHandle,
											 unsigned char * pKeyEven,
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity)
{
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm           = NEXUS_SecurityAlgorithm_e3DesAba;
    AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eCipherStealing;
	AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
	AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
	AlgConfig.caVendorID          = 0x1234;
	AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;
	AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
	AlgConfig.key2Select		  = NEXUS_SecurityKey2Select_eReserved1;
	AlgConfig.dest 				  = NEXUS_SecurityAlgorithmConfigDestination_eCa;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = false;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal]	  = false;

#if 0 /* nothing done  here for 40-nm platforms; CPS will modify SC bits. */
	AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] 	  = polarity;
	AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] 		  = polarity;
#endif

	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed \n");
		return 1;
	}

	/* Load AV keys */
	key.keySize = 16;
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCa;
	key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;

    memcpy(key.keyData,pKeyEven,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA EVEN key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,pKeyOdd,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA ODD key failed \n");
		return 1;
	}
	return 0;

}

static NEXUS_Error ConfigureKeySlotForAesCPS (NEXUS_KeySlotHandle keyHandle,
											 unsigned char * pKeyEven,
											 unsigned char * pKeyOdd,
											 NEXUS_SecurityAlgorithmScPolarity polarity)
{
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;

    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm 		  = NEXUS_SecurityAlgorithm_eAes128;
	AlgConfig.algorithmVar 		  = NEXUS_SecurityAlgorithmVariant_eEcb;
	AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eCipherStealing;
	AlgConfig.ivMode			  = NEXUS_SecurityIVMode_eRegular;
	AlgConfig.solitarySelect	  = NEXUS_SecuritySolitarySelect_eClear;
	AlgConfig.caVendorID		  = 0x1234;
	AlgConfig.askmModuleID		  = NEXUS_SecurityAskmModuleID_eModuleID_4;
	AlgConfig.otpId 			  = NEXUS_SecurityOtpId_eOtpVal;
	AlgConfig.key2Select		  = NEXUS_SecurityKey2Select_eReserved1;
	AlgConfig.dest 				  = NEXUS_SecurityAlgorithmConfigDestination_eCps;
	AlgConfig.bRestrictEnable     = false;
	AlgConfig.bEncryptBeforeRave  = false;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
	AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = true;
	AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] = polarity;
	AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal]	    = polarity;

	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CPS Algorithm failed \n");
		return 1;
	}


	/* Load AV keys */
	key.keySize = 16;
	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCps;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;

	if (polarity == NEXUS_SecurityAlgorithmScPolarity_eEven)
		memcpy(key.keyData,pKeyEven,key.keySize);
	else
		memcpy(key.keyData,pKeyOdd,key.keySize);

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CPS EVEN key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;

	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CPS ODD key failed \n");
		return 1;
	}
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
    {
        printf("\nLoad CPS CLEAR key failed \n");
        return 1;
    }
	return 0;

}
