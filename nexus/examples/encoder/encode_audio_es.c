/******************************************************************************
*    (c)2008-2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
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
*****************************************************************************/
#include "nexus_platform.h"
#include "nexus_platform_features.h"

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_AUDIO_MUX_OUTPUT && NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_audio_dummy_output.h"
#include "nexus_video_encoder_output.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_audio_mux_output.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* set to 0 to show decode only test */
#define BTST_ENABLE_TRANSCODE            1

BDBG_MODULE(encode_audio_es);


/* Encode parameters */
typedef struct EncodeSettings {
	char                        fname[256];
	NEXUS_AudioCodec            eAudioCodec;
    NEXUS_AudioMode             chMode;
    NEXUS_AudioMonoChannelMode  monoMode;
    bool                        realTime;
} EncodeSettings;

/* Input stream parameters */
typedef struct InputSettings {
	char                 fname[256];
	NEXUS_TransportType  eStreamType;          
	NEXUS_AudioCodec     eAudioCodec;    
	int                  iAudioPid;
	int                  iPcrPid;
} InputSettings;

void print_usage(void) {
    printf("Usage:\n");
    printf("nexus encode_audio_es ");
    printf("-ifile <in_file> ");
    printf("-ofile <out_file> ");
    printf("-icodec <in_codec> ");
    printf("-ocodec <out_codec> ");
    printf("-chmode <enc_ch_mode> ");
    printf("-monomode <enc_mono_ch_mode> ");
    printf("-ts [");
    printf("-pid <in_pid> ");
    printf("[-pcr <in_pcr>]] ");
    printf("-nrt ");
    printf("\n");
}

int main(int argc, char **argv) {
	NEXUS_StcChannelHandle stcChannel, stcChannelTranscode;
	NEXUS_StcChannelSettings stcSettings;
	NEXUS_PidChannelHandle audioPidChannel, pcrPidChannel;
	NEXUS_AudioDecoderHandle audioDecoder;
	NEXUS_AudioDecoderStartSettings audioProgram;
	NEXUS_FilePlayHandle file;
	NEXUS_PlaypumpHandle playpump;
	NEXUS_PlaybackHandle playback;
	NEXUS_PlaybackSettings playbackSettings;
	NEXUS_PlaybackPidChannelSettings playbackPidSettings;
	NEXUS_AudioMuxOutputHandle audioMuxOutput;
	NEXUS_AudioMuxOutputStatus audioMuxStatus;
	NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    /*NEXUS_AudioMixerHandle audioMixer;
    NEXUS_AudioMixerSettings audioMixerSettings;*/
    NEXUS_AudioDummyOutputHandle audioDummyOutput;
    NEXUS_AudioDacHandle audioDac;
#if BTST_ENABLE_TRANSCODE 
	NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderCodecSettings encCodecSettings;
	NEXUS_AudioEncoderHandle audioEncoder;
#endif
    bool monoDecOutputRequired;
	size_t bytes;
	FILE *fout, *fdesc;
	InputSettings stInput;
	EncodeSettings stEncode;
    bool menu = false;
    int curarg = 1;
    bool writeMetadata = true;
    void *pDataBuffer;
    int cntlFlag = 0;

    sprintf(stInput.fname,"/mnt/media/MP3/foxandcats.mp3");
    stInput.eStreamType = NEXUS_TransportType_eEs;
    stInput.eAudioCodec = NEXUS_AudioCodec_eMp3;
    stInput.iAudioPid = 1;
    stInput.iPcrPid = 0;

    sprintf(stEncode.fname,"/mnt/media/MP3/encoded/fc_dual.aac");
    stEncode.chMode = NEXUS_AudioMode_e2_0;
    stEncode.monoMode = NEXUS_AudioMonoChannelMode_eMix;
    stEncode.realTime = true;

    while (curarg < argc) {
        if (strcmp(argv[1],"-menu") == 0){
            menu = true;
            curarg = argc;
            break;
        }

        if (!strcmp("--help", argv[curarg]) ||
            !strcmp("-h", argv[curarg]) ||
            !strcmp("-?", argv[curarg])) {
            print_usage();
            return 1;
        }
        else if (!strcmp("-ifile", argv[curarg]) && curarg+1 < argc) {
            sprintf(stInput.fname, argv[++curarg]);
        }
        else if (!strcmp("-ofile", argv[curarg]) && curarg+1 < argc) {
            sprintf(stEncode.fname, argv[++curarg]);
        }
        else if (!strcmp("-icodec", argv[curarg]) && curarg+1 < argc) {
            stInput.eAudioCodec = atoi(argv[++curarg]);
        }
        else if (!strcmp("-ocodec", argv[curarg]) && curarg+1 < argc) {
            stEncode.eAudioCodec = atoi(argv[++curarg]);
        }
        else if (!strcmp("-chmode", argv[curarg]) && curarg+1 < argc) {
            stEncode.chMode = atoi(argv[++curarg]);
        }
        else if (!strcmp("-monomode", argv[curarg]) && curarg+1 < argc) {
            stEncode.monoMode = atoi(argv[++curarg]);
        }
        else if (!strcmp("-ts", argv[curarg])) {
            stInput.eStreamType = NEXUS_TransportType_eTs;
        }
        else if (!strcmp("-pid", argv[curarg]) && curarg+1 < argc) {
            stInput.iAudioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp("-pcr", argv[curarg]) && curarg+1 < argc) {
            stInput.iPcrPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp("-nrt", argv[curarg])) {
            stEncode.realTime = false;
        }
        else {
            printf("Unknown param: %s\n", argv[curarg]);
            return -1;
        }
        curarg++;
    }

    if (menu)
    {
        printf("\n decode stream: ");                                        scanf("%s", stInput.fname);
	    printf("\n decode stream type  (1) Es (2) Ts ");                     scanf("%d", (int32_t*)&stInput.eStreamType);
	    printf("\n encode stream codec (1) Mpg (2) Mp3 (3) AAC (6) AAC+ (7) AC3 ");  scanf("%d", (int32_t*)&stEncode.eAudioCodec);
        if (stInput.eStreamType != NEXUS_TransportType_eEs) {
	        printf("\n Audio pid ");                                             scanf("%d", &stInput.iAudioPid);
	        printf("\n Pcr   pid ");                                             scanf("%d", &stInput.iPcrPid);
        }
        printf("\n encode stream: ");                                        scanf("%s", stEncode.fname);
	    printf("\n encode stream codec (1) Mpg (2) Mp3 (3) AAC (6) AAC+ (7) AC3 (37) ILBC (38) ISAC");  scanf("%d", (int32_t*)&stEncode.eAudioCodec);
        if (stEncode.eAudioCodec == NEXUS_AudioCodec_eMp3 || stEncode.eAudioCodec == NEXUS_AudioCodec_eAac || stEncode.eAudioCodec == NEXUS_AudioCodec_eAacPlusAdts) {
    	    printf("\n encode stereo, mono, or dual mono: (1) mono (2) dual mono (3) stereo ");         scanf("%d", (int*)&stEncode.chMode);
            if (stEncode.chMode == NEXUS_AudioMode_e1_0) {
        	    printf("\n encode mono mode: (0) left (1) right (2) mix "); scanf("%d", (int*)&stEncode.monoMode);
            }
        }
    }

	printf("\n****************************************************************\n");
	printf("\n Input Parameters\n");
	printf("\n Input filename           %s", stInput.fname);
	printf("\n Streamtype %d iencoderAudioCodec %d", stInput.eStreamType, stInput.eAudioCodec);
	printf("\n aPid %4d, PcrPid  %4d", stInput.iAudioPid, stInput.iPcrPid);
	printf("\n\n Encode Parameters\n");
	printf("\n Output Encode filename   %s", stEncode.fname);
	printf("\n Encode Codec  %d", stEncode.eAudioCodec);
    printf("\n Encode channel mode  %s", (stEncode.chMode==NEXUS_AudioMode_e1_0) ? "Mono" : (stEncode.chMode==NEXUS_AudioMode_e1_1) ? "Dual Mono" : "Stereo");
    if (stEncode.chMode == NEXUS_AudioMode_e1_0)
    {
        printf("\n Encode mono mode  %s", (stEncode.monoMode==0) ? "Left" : (stEncode.monoMode==1) ? "Right" : "Mix");
    }
	printf("\n****************************************************************\n");

#if defined(NEXUS_MODE_client)
    NEXUS_Platform_Join(); 
#else
    {
        NEXUS_PlatformSettings platformSettings;
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        platformSettings.openOutputs = false;
        NEXUS_Platform_Init(&platformSettings);
    }
#endif

    audioDummyOutput = NEXUS_AudioDummyOutput_Open(0, NULL);
    audioDac = NEXUS_AudioDac_Open(0, NULL);
    /*NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
    audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);*/

    playpump = NEXUS_Playpump_Open(0, NULL);
	assert(playpump);
	playback = NEXUS_Playback_Create();
	assert(playback);

	file = NEXUS_FilePlay_OpenPosix(stInput.fname, NULL);
	if (!file) {
		fprintf(stderr, "can't open file:%s\n", stInput.fname);
		return -1;
	}

	NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    if (stEncode.realTime)
    {
	    /* encoders/mux require different STC broadcast mode from decoder */
	    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
	    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
	    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    	stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);
    }
    else 
    {
        stcChannelTranscode = stcChannel;
    }

	NEXUS_Playback_GetSettings(playback, &playbackSettings);
	playbackSettings.playpump = playpump;
	/* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
	playbackSettings.playpumpSettings.transportType = stInput.eStreamType;
    playbackSettings.stcChannel = stcChannel;
	NEXUS_Playback_SetSettings(playback, &playbackSettings);

	/* Open the audio decoder */
	audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

	/* Open the audio and pcr pid channel */
	NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
	playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
	playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */

	audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, stInput.iAudioPid, &playbackPidSettings);
	playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
	pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, stInput.iPcrPid, &playbackPidSettings);

	/* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
	NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);

    audioProgram.codec = stInput.eAudioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    audioProgram.nonRealTime = !stEncode.realTime;

    /* NEXUS_AudioMixer_AddInput(audioMixer, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)); */

	/* Connect audio decoders to outputs */
    if (stEncode.realTime)
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDummyOutput_GetConnector(audioDummyOutput),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if 0
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(audioDac),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    }

	/* Open audio mux output */
	audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
	assert(audioMuxOutput);

#if BTST_ENABLE_TRANSCODE 
	/* Open audio encoder */
	NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
	encoderSettings.codec = stEncode.eAudioCodec;
	audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
	assert(audioEncoder);

    NEXUS_AudioEncoder_GetCodecSettings(audioEncoder, encoderSettings.codec, &encCodecSettings);
    monoDecOutputRequired = false;
    switch (encoderSettings.codec)
    {
        case NEXUS_AudioCodec_eMp3:
        case NEXUS_AudioCodec_eMpeg:
            encCodecSettings.codecSettings.mp3.outputMode = stEncode.chMode;
            encCodecSettings.codecSettings.mp3.monoMode = stEncode.monoMode;
            break;
        case NEXUS_AudioCodec_eAac:
            encCodecSettings.codecSettings.aac.outputMode = stEncode.chMode;
            encCodecSettings.codecSettings.aac.monoMode = stEncode.monoMode;
            break;
        case NEXUS_AudioCodec_eAacPlusAdts:
            encCodecSettings.codecSettings.aacPlus.outputMode = stEncode.chMode;
            encCodecSettings.codecSettings.aacPlus.monoMode = stEncode.monoMode;
            break;
        case NEXUS_AudioCodec_eAmrWb:
        case NEXUS_AudioCodec_eAmrNb:
        case NEXUS_AudioCodec_eG711:
        case NEXUS_AudioCodec_eG723_1:
        case NEXUS_AudioCodec_eG729:
        case NEXUS_AudioCodec_eIlbc:
        case NEXUS_AudioCodec_eIsac:
            writeMetadata = false;
            monoDecOutputRequired = true;
            break;
        default:
            break;
    }
    NEXUS_AudioEncoder_SetCodecSettings(audioEncoder, &encCodecSettings);

	/* Connect encoder to decoder */
    if ( monoDecOutputRequired )
    {
    	NEXUS_AudioEncoder_AddInput(audioEncoder,
    		NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMono));
    }
    else
    {
    	NEXUS_AudioEncoder_AddInput(audioEncoder,
    		NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
	/* Connect mux to encoder */
	NEXUS_AudioOutput_AddInput(
		NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
#else    
	/* Connect mux to encoder */
	NEXUS_AudioOutput_AddInput(
		NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput),
		NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif

    /* Start audio mux output */
    NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
    audioMuxStartSettings.stcChannel = stcChannelTranscode;
    audioMuxStartSettings.nonRealTime = !stEncode.realTime;
    NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);

    /* Start decoders */
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &audioMuxStatus);
    NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &pDataBuffer);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    fout = fopen(stEncode.fname,"wb");
    fdesc = fopen("audio_desc.txt","w");
    fprintf(fdesc, "flags(h) origPts(h)         pts(Lh)     escr(h) ticksPerBit(u) shr(d) offset(h) length(h) protocol(u)\n");

	fprintf (stderr,"******************************\n");
	fprintf(stderr,"press 'q' and then 'ENTER' to exit from this application..\n");
	fprintf (stderr,"******************************\n\n");

	/*Register the STDIN file descriptor*/
	cntlFlag = fcntl(STDIN_FILENO, F_GETFL, 0);
    for(;;) {
        size_t size[2];
        const NEXUS_AudioMuxOutputFrame *desc[2];
        unsigned i,j;
        unsigned descs;
		int retval;
		char one_char;
		fcntl(STDIN_FILENO, F_SETFL, cntlFlag|O_NONBLOCK);
		retval = read(STDIN_FILENO, &one_char, 1);
		if(retval != -1)
		{
			if (one_char == 'q')
			{
				fcntl(STDIN_FILENO, F_SETFL, 0); /*Clear all cntlFlag that are set. This will reset/close the STDIN_FILENO*/
				break;
			}
		}
		NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            NEXUS_AudioDecoderStatus astatus;

            NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus);
            fflush(fout);
            fprintf(stderr, "written %lu bytes.... decode:%u\t\r", (unsigned long)bytes, astatus.pts);
            BKNI_Sleep(30);
            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<size[j];i++) {
                bool writeData = true;
                if ( desc[j][i].flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA )
                {
                    if(writeMetadata) {
                        NEXUS_MemoryBlock_Lock(audioMuxStatus.metadataBufferBlock, &pDataBuffer);
                    } else {
                        writeData = false;
                        printf("encode_audio_es - ignoring metadata for this type\n");
                    }
                }
                else
                {
                    NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &pDataBuffer);
                }

                if ( writeData && (desc[j][i].length > 0) )
                {
                    fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                    fprintf(fdesc, "%8x %8x   %08x%08x %8x %5u %5d %8x %lux\n", desc[j][i].flags, desc[j][i].originalPts,
                        (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
                        desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, (unsigned long)desc[j][i].length);
                    bytes+= desc[j][i].length;
                }
            }
        }
        NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, descs);
        fflush(fout);
        fflush(fdesc);
    }

    /* Playback state machine is driven from inside Nexus. */
    /*printf("Press ENTER to quit\n");
    getchar();*/

	/* Bring down system */
	NEXUS_Playback_Stop(playback);
	NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);
#if BTST_ENABLE_TRANSCODE
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
#endif
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(audioDummyOutput));
	NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
    NEXUS_AudioEncoder_Close(audioEncoder);
	NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_AudioDac_Close(audioDac);
    NEXUS_AudioDummyOutput_Close(audioDummyOutput);
	NEXUS_Playback_Destroy(playback);
	NEXUS_Playpump_Close(playpump);
    if (stEncode.realTime)
    {
        NEXUS_StcChannel_Close(stcChannelTranscode);
    }
    NEXUS_StcChannel_Close(stcChannel);

	NEXUS_FilePlay_Close(file);
	NEXUS_Platform_Uninit();

	return 0;
}
#else
#include <stdio.h>
int main(void)
{
	printf("This application is not supported on this platform!\n");
	return 0;
}
#endif

