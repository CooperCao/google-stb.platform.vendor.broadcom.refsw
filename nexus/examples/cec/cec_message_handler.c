/***************************************************************************
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
***************************************************************************/

#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if NEXUS_HAS_AUDIO && NEXUS_HAS_HDMI_OUTPUT && NEXUS_HAS_CEC && NEXUS_NUM_CEC
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_parser_band.h"
#include "nexus_transport_wakeup.h"

#include "nexus_hdmi_output.h"
#include "nexus_cec.h"

BDBG_MODULE(cec_message_handler);

NEXUS_DisplayHandle display0, display1=NULL;
NEXUS_PlatformSettings platformSettings;
NEXUS_PlatformConfiguration platformConfig;
NEXUS_StcChannelHandle stcChannel;
NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
NEXUS_VideoWindowHandle window0, window1=NULL;
NEXUS_VideoDecoderHandle videoDecoder;
NEXUS_VideoDecoderStartSettings videoProgram;
NEXUS_AudioDecoderHandle audioDecoder;
NEXUS_AudioDecoderStartSettings audioProgram;
NEXUS_FilePlayHandle file;
NEXUS_PlaypumpHandle playpump;
NEXUS_PlaybackHandle playback;
NEXUS_ParserBand parserBand;
bool playback_started=false;

NEXUS_HdmiOutputHandle hdmiOutput;
NEXUS_CecHandle hCec;
bool deviceReady = false, standby_start = false;

#define FILENAME "videos/cnnticker.mpg"

/***********************************************
***********************************************/
static void playback_start(void)
{
	int rc;
	if(playback_started)
		return;

	file = NEXUS_FilePlay_OpenPosix(FILENAME, NULL);
	if (!file) {
		BDBG_ERR(("can't open file %s", FILENAME));
		return;
	}

	/* Start playback */
	rc = NEXUS_Playback_Start(playback, file, NULL);
	if (rc) {
		NEXUS_FilePlay_Close(file);
		return;
	}

	playback_started = true;
}

void playback_stop(void)
{
	if(!playback_started)
		return;

	/*Stop playback */
	NEXUS_Playback_Stop(playback);

	/* Close File. Required for umount */
	NEXUS_FilePlay_Close(file);

	playback_started = false;
}

void decode_start(void)
{
	/* Start decoders */
	NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
	NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
}

void decode_stop(void)
{
	/* Stop decoders */
	NEXUS_VideoDecoder_Stop(videoDecoder);
	NEXUS_AudioDecoder_Stop(audioDecoder);
}


/**********************************************
******** CEC Handler functions ********************
***********************************************/
typedef void (*getParameter) (uint8_t *content, unsigned *length);

#define NUM_MAX_PARAMETERS 3

typedef struct opCodeSupportList
{
	uint8_t opCodeReceived;
	bool sendResponse;
	uint8_t opCodeResponse;
	uint8_t responseAddress;
	getParameter getParamFunc[NUM_MAX_PARAMETERS];
}opCodeSupportList;

void param_getPhysicalAddress( uint8_t *content, unsigned *length )
{
	NEXUS_CecStatus status;
	NEXUS_Cec_GetStatus( hCec, &status );

	content[0] = status.physicalAddress[0];
	content[1] = status.physicalAddress[1];
	*length = 2;
}

void param_getDeviceType( uint8_t *content, unsigned *length )
{
	NEXUS_CecStatus status;
	NEXUS_Cec_GetStatus( hCec, &status );
	switch(status.deviceType)
	{
		case NEXUS_CecDeviceType_eTv: content[0] = 0; break;
		case NEXUS_CecDeviceType_eRecordingDevice: content[0] = 1; break;
		case NEXUS_CecDeviceType_eReserved: content[0] = 2; break;
		case NEXUS_CecDeviceType_eTuner: content[0] = 3; break;
		case NEXUS_CecDeviceType_ePlaybackDevice: content[0] = 4; break;
		case NEXUS_CecDeviceType_eAudioSystem: content[0] = 5; break;
		case NEXUS_CecDeviceType_ePureCecSwitch: content[0] = 6; break;
		case NEXUS_CecDeviceType_eVideoProcessor: content[0] = 7; break;
		default: BDBG_WRN(("Unknown Device Type!!!")); break;
	}
	*length = 1;
}

void param_getCecVersion( uint8_t *content, unsigned *length )
{
	NEXUS_CecStatus status;
	NEXUS_Cec_GetStatus( hCec, &status );
	BDBG_WRN(("CEC Version Requested -> Version: %#x",status.cecVersion));
	content[0] = status.cecVersion;
	*length = 1;
}

void param_getOsdName ( uint8_t *content, unsigned *length )
{
#define OSDNAME_SIZE 7 /* Size no larger than 15 */
	char osdName[OSDNAME_SIZE]= "BRCMSTB";
	unsigned j;
	for( j = 0; j < OSDNAME_SIZE ; j++ )
	{
		content[j] = (int)osdName[j];
	}
	*length = OSDNAME_SIZE;
}

void param_getDeviceVendorID( uint8_t *content, unsigned *length )
{
#define VendorID_SIZE 4 /* Size no larger than 15 */
	char vendorID[VendorID_SIZE]= "BRCM";
	unsigned j;
	for( j = 0; j < VendorID_SIZE ; j++ )
	{
		content[j] = (int)vendorID[j];
	}
	*length = VendorID_SIZE;
}

void func_enterStandby ( uint8_t *content, unsigned *length )
{
	NEXUS_Error rc;
	unsigned timeout = 30;
	NEXUS_PlatformStandbySettings nexusStandbySettings;
	NEXUS_PlatformStandbyStatus nexusStandbyStatus;
	NEXUS_CecMessage transmitMessage;


	BDBG_WRN(("TV STANDBY RECEIVED: STB WILL NOW ENTER STANDBY..."));

	if (playback_started)
	{
        /* Entering Standby, stop playback first */
        playback_stop();
        decode_stop();
	}

    printf("Entering S2 Mode\n");

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_ePassive;
    nexusStandbySettings.wakeupSettings.cec = true;
    nexusStandbySettings.wakeupSettings.ir = true;
    nexusStandbySettings.wakeupSettings.timeout = timeout;
    rc = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
    BDBG_ASSERT(!rc);

	/* wake up from standby */
	system("echo standby > /sys/power/state");
    NEXUS_Platform_GetStandbyStatus(&nexusStandbyStatus);

    /* Printing out wake-up status */
    printf("S2 Wake up Status\n"
           "Standby Status : \n"
           "IR      : %d\n"
           "CEC     : %d\n"
           "Timeout : %d\n"
           "\n",
           nexusStandbyStatus.wakeupStatus.ir,
           nexusStandbyStatus.wakeupStatus.cec,
           nexusStandbyStatus.wakeupStatus.timeout);


    /* RESUME from Standby */
    BDBG_WRN(("Resuming Normal Mode\n"));
    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eOn;
    rc = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
    BDBG_ASSERT(!rc);

    /* Turn the TV on if TV was not woke up manually */
    if(nexusStandbyStatus.wakeupStatus.timeout|| nexusStandbyStatus.wakeupStatus.ir)
    {
        BDBG_WRN(("*************************")) ;
        BDBG_WRN(("No CEC Message received from TV to wake up"));
        BDBG_WRN(("S2 Resume because of %s wake up", nexusStandbyStatus.wakeupStatus.timeout? "Timeout":"IR"));
        BDBG_WRN(("Make sure TV is turned back ON"));
        BDBG_WRN(("Send <Image View On> message")); /* this is needed in case TV was already powered off */
        BDBG_WRN(("*************************")) ;

        transmitMessage.destinationAddr = 0;
        transmitMessage.length = 1;
        transmitMessage.buffer[0] = 0x04;
        rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);
    }

 	if (!playback_started)
 	{
		/* Start Decoder and Playback */
		decode_start();
		playback_start();

		BDBG_WRN(("************************************")) ;
		BDBG_WRN(("Power off the TV "));
		BDBG_WRN(("Platform will go to standby")); /* this is needed in case TV was already powered off */
		BDBG_WRN(("Use TV or IR or wait 30 seconds to"));
		BDBG_WRN(("wake up platform"));
		BDBG_WRN(("************************************")) ;
	}

	BSTD_UNUSED(content);
	BSTD_UNUSED(length);
}

const opCodeSupportList opCodeList[] =
{
	{ NEXUS_CEC_OpGivePhysicalAddress, true, NEXUS_CEC_OpReportPhysicalAddress, 0xF,
		  { param_getPhysicalAddress,
			param_getDeviceType,
			NULL } },

	{ NEXUS_CEC_OpGiveOSDName, true, NEXUS_CEC_OpSetOSDName, 0xD,
		  { param_getOsdName,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpGiveDeviceVendorID, true, NEXUS_CEC_OpDeviceVendorID, 0xD,
		  { param_getDeviceVendorID,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpRequestActiveSource, true, NEXUS_CEC_OpActiveSource, 0xD,
		  { param_getPhysicalAddress,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpSetStreamPath, true, NEXUS_CEC_OpActiveSource, 0xD,
		  { param_getPhysicalAddress,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpStandby, false, 0, 0xD,
		  { func_enterStandby,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpGetCECVersion, true, NEXUS_CEC_OpCECVersion, 0xD,
		  { param_getCecVersion,
			NULL,
			NULL } },

	{ NEXUS_CEC_OpAbort, true, NEXUS_CEC_OpFeatureAbort, 0xD,
		  { NULL,
			NULL,
			NULL } },

	{ 0, false, 0, 0xD, {NULL, NULL, NULL} }
};

void responseLookUp( const uint8_t opCode, const uint8_t address )
{
	NEXUS_CecMessage message;
	int i, index = 0;
	unsigned j, length = 0, param_index = 0;
	uint8_t tmp[NEXUS_CEC_MESSAGE_DATA_SIZE];

	for( i=0; opCodeList[i].opCodeReceived; i++ )
	{
		if (opCode == opCodeList[i].opCodeReceived)
		{
			BDBG_MSG((">>>>>>>>>>>>>>>>>>>>> Found support for opcode:%#x <<<<<<<<<<<<<<<<<<<<<<", opCode));

			/* if free use, use the address passed in */
			if (opCodeList[i].responseAddress == 0xD)
				message.destinationAddr = address;
			else
				message.destinationAddr = opCodeList[i].responseAddress;

			/* Assign Designated Response Op Code */
			message.buffer[0] = opCodeList[i].opCodeResponse;
			message.length = 1;

			/* If require Parameters, then add to message. Skip getParam function if NULL */
			while ( param_index < NUM_MAX_PARAMETERS && opCodeList[i].getParamFunc[param_index] )
			{
				opCodeList[i].getParamFunc[param_index]( tmp, &length );
				BDBG_MSG(("GETTING PARAMETER %d, LENGTH: %d", param_index, length));

				if ( message.length+length > NEXUS_CEC_MESSAGE_DATA_SIZE)
				{
					BDBG_WRN(("This parameter has reached over size limit! Last Parameter Length: %d", length));
					return;
				}
				message.length += length;
				for( j = 0 ; j < length ; j ++ )
				{
					message.buffer[++index]= tmp[j];
					BDBG_MSG(("Message Buffer[%d]: %#x", index, tmp[j]));
				}
				param_index++;
			}

			/* Only response if a response CEC message is required */
			if (opCodeList[i].sendResponse)
			{
				BDBG_MSG((">>>>>>>>>>>>>>>>>>>>> Transmit Response :%#x <<<<<<<<<<<<<<<<<<<<<<", message.buffer[0]));
				NEXUS_Cec_TransmitMessage( hCec, &message );
			}
		}
	}
}

void deviceReady_callback(void *context, int param)
{
	NEXUS_CecStatus status;

	BSTD_UNUSED(param);
	BSTD_UNUSED(context);
	NEXUS_Cec_GetStatus(hCec, &status);

	BDBG_WRN(("BCM%d Logical Address <%d> Acquired",
		BCHP_CHIP,
		status.logicalAddress)) ;

	BDBG_WRN(("BCM%d Physical Address: %X.%X.%X.%X",
		BCHP_CHIP,
		(status.physicalAddress[0] & 0xF0) >> 4,
		(status.physicalAddress[0] & 0x0F),
		(status.physicalAddress[1] & 0xF0) >> 4,
		(status.physicalAddress[1] & 0x0F))) ;

	if ((status.physicalAddress[0] = 0xFF)
	&& (status.physicalAddress[1] = 0xFF))
	{
		BDBG_WRN(("CEC Device Ready!")) ;
		   deviceReady = true ;
	}
}

void msgReceived_callback(void *context, int param)
{
	NEXUS_CecStatus status;
	NEXUS_CecReceivedMessage receivedMessage;
	NEXUS_Error rc ;
	uint8_t i, j ;
	char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

	BSTD_UNUSED(param);
	BSTD_UNUSED(context);
	NEXUS_Cec_GetStatus(hCec, &status);

	BDBG_WRN(("Message Received: %s", status.messageReceived ? "Yes" : "No")) ;

	rc = NEXUS_Cec_ReceiveMessage(hCec, &receivedMessage);
	BDBG_ASSERT(!rc);

	/* For debugging purposes */
	for (i = 0, j = 0; i <= receivedMessage.data.length && j<(sizeof(msgBuffer)-1); i++)
	{
		j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
		receivedMessage.data.buffer[i]) ;
	}

	BDBG_WRN(("CEC Message Length %d Received: %s, %d",
		receivedMessage.data.length, msgBuffer, atoi(msgBuffer))) ;

	BDBG_WRN(("Msg Recd Status from Phys/Logical Addrs: %X.%X.%X.%X / %d",
		(status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
		(status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
		status.logicalAddress)) ;

	/* Look up support opcode list */
	responseLookUp( receivedMessage.data.buffer[0], receivedMessage.data.initiatorAddr);

}

void msgTransmitted_callback(void *context, int param)
{
	NEXUS_CecStatus status;

	BSTD_UNUSED(param);
	BSTD_UNUSED(context);
	NEXUS_Cec_GetStatus(hCec, &status);

	BDBG_WRN(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d",
		(status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
		(status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
		status.logicalAddress)) ;

	BDBG_WRN(("Device Type is : %d", status.deviceType)) ;

	BDBG_WRN(("Xmit Msg Acknowledged: %s",
		status.transmitMessageAcknowledged ? "Yes" : "No")) ;
	BDBG_WRN(("Xmit Msg Pending: %s",
		status.messageTransmitPending ? "Yes" : "No")) ;
}



/******************************************/
void start_app(void)
{
    NEXUS_DisplaySettings displaySettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplayCapabilities displayCapabilities;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display0 = NEXUS_Display_Open(0, &displaySettings);

    NEXUS_GetDisplayCapabilities(&displayCapabilities);

    if (displayCapabilities.numDisplays > 1) {
        displaySettings.format = NEXUS_VideoFormat_eNtsc;
        display1 = NEXUS_Display_Open(1, &displaySettings);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
        if(platformConfig.outputs.composite[0]) {
            NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
        }
#endif
#if NEXUS_NUM_RFM_OUTPUTS
        if (platformConfig.outputs.rfm[0]) {
            NEXUS_Display_AddOutput(display1, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));
        }
#endif
    }

#if NEXUS_NUM_HDMI_OUTPUTS
    if (platformConfig.outputs.hdmi[0]) {
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    }
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if(platformConfig.outputs.component[0]) {
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
#endif

    window0 = NEXUS_VideoWindow_Open(display0, 0);

    if (displayCapabilities.numDisplays > 1 && displayCapabilities.display[1].numVideoWindows) {
        window1 = NEXUS_VideoWindow_Open(display1, 0);
    }

    playpump = NEXUS_Playpump_Open(0, NULL);
    playback = NEXUS_Playback_Create();

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);


    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numOutputs.dac > 0) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

    if (audioCapabilities.numOutputs.spdif > 0) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

    NEXUS_AudioOutput_AddInput(
    NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
    NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));


    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (displayCapabilities.numDisplays > 1 && displayCapabilities.display[1].numVideoWindows) {
        NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x21, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x22, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
       the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eMpeg;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

}

void stop_app(void)
{
    /* Bring down system */
    playback_stop();
    decode_stop();
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoWindow_RemoveInput(window0, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (window1) {
        NEXUS_VideoWindow_RemoveInput(window1, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        NEXUS_VideoWindow_Close(window1);
    }
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_Close(window0);
    NEXUS_Display_Close(display0);
    if (display1) {
        NEXUS_Display_Close(display1);
    }
    NEXUS_VideoDecoder_Close(videoDecoder);
    if (platformConfig.outputs.audioDacs[0] != NULL) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    }
    if (platformConfig.outputs.spdif[0] != NULL) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    }
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_StcChannel_Close(stcChannel);

}


int main(int argc, char **argv)
{
    NEXUS_Error rc;
    bool exit = false;
    unsigned loops;

    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
    NEXUS_CecMessage transmitMessage;

	BSTD_UNUSED(argc);
	BSTD_UNUSED(argv);


    /* Bring up all modules for a platform in a default configuration for this platform */
	NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    hdmiOutput = platformConfig.outputs.hdmi[0];
    hCec = platformConfig.outputs.cec[0];

    rc = NEXUS_Cec_SetHdmiOutput(hCec, hdmiOutput);
    BDBG_ASSERT(!rc);

    NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.messageReceivedCallback.callback = msgReceived_callback ;
        cecSettings.messageTransmittedCallback.callback = msgTransmitted_callback;
        cecSettings.logicalAddressAcquiredCallback.callback = deviceReady_callback ;
    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);


    /* Enable CEC core */
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
    cecSettings.enabled = true;
    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);


    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Wait for logical address before starting test..."));
    BDBG_WRN(("*************************\n\n")) ;
    for (loops = 0; loops < 10; loops++) {
        if (deviceReady)
            break;
        BKNI_Sleep(1000);
    }

    if (cecStatus.logicalAddress == 0xFF)
    {
        BDBG_ERR(("No CEC capable device found on HDMI output."));
        goto done ;
    }

    transmitMessage.destinationAddr = 0;
    transmitMessage.length = 1;


    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Make sure TV is turned ON"));
    BDBG_WRN(("Send <Image View On> message")); /* this is needed in case TV was already powered off */
    BDBG_WRN(("*************************")) ;
    transmitMessage.buffer[0] = 0x04;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);


    start_app();
	while (!exit)
	{
		int tmp;

		/* Display Menu */
		printf("************************************\n");
		printf("*******   Main Menu         ********\n");
		printf("*** 1) Play video\n");
		printf("*** 0) Exit\n");
		printf("*** Enter Selection: \n");
		scanf("%d", &tmp);
		switch ( tmp )
		{
		case 1:
			/* Start Decoder and Playback */
			decode_start();
			playback_start();

			BDBG_WRN(("************************************")) ;
			BDBG_WRN(("Power off the TV "));
			BDBG_WRN(("Platform will go to standby")); /* this is needed in case TV was already powered off */
			BDBG_WRN(("Use TV or IR or wait 30 seconds to"));
			BDBG_WRN(("wake up platform"));
			BDBG_WRN(("************************************")) ;

			break;

		case 0:
			exit = true;
			break;
		}
	}

done:
    stop_app();
    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
