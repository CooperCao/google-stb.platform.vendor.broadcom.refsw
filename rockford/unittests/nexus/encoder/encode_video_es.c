/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
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
* Module Description:
*
*****************************************************************************/
/* Nexus unittest app: encode video from file/qam/hdmi source */

#include "nexus_platform.h"
#if !NEXUS_HAS_VIDEO_ENCODER
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#else

#include "nexus_video_decoder.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_adj.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#endif
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include <sys/select.h>

#if  NEXUS_HAS_FRONTEND
#define BTST_SUPPORT_FRONTEND    1
#else
#define BTST_SUPPORT_FRONTEND    0
#endif

#define BTST_RESOURCE_FILE       0
#define BTST_RESOURCE_HDMI       1
#define BTST_RESOURCE_QAM        2


BDBG_MODULE(encode_video_es);
#if  BTST_SUPPORT_FRONTEND
static void lock_callback(void *context, int param)
{
	NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
	NEXUS_FrontendQamStatus qamStatus;

	BSTD_UNUSED(param);

	fprintf(stderr, "Lock callback, frontend %p\n", (void*)frontend);

	NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
	fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)frontend,
		qamStatus.fecLock, qamStatus.receiverLock);
}
#endif

typedef struct EncodeSettings {
	char                    fname[256];
	char                    indexfname[256];
	NEXUS_VideoFormat       displayFormat;
	NEXUS_VideoFrameRate    encoderFrameRate;
	int                     encoderBitrate;
	int                     encoderGopStructureFramesP;
	int                     encoderGopStructureFramesB;
	NEXUS_VideoCodec        encoderVideoCodec;
	NEXUS_VideoCodecProfile encoderProfile;
	NEXUS_VideoCodecLevel   encoderLevel;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;
	bool                    bCustom;
}  EncodeSettings;

typedef struct InputSettings{
	int                   resource;
	char                 fname[256];
	NEXUS_TransportType  eStreamType;
	NEXUS_VideoCodec     encoderVideoCodec;
	int                  iVideoPid;
	int                   iPcrPid;
	int                   freq;
#if  BTST_SUPPORT_FRONTEND
	NEXUS_FrontendQamMode qamMode;
#endif
} InputSettings;

static int kbhit(void)
{
	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);

	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
	return 0;

	if(FD_ISSET(0,&read_fd))
	return 1;

	return 0;

}

#if  BTST_SUPPORT_FRONTEND
int transcode_qam(EncodeSettings  *pEncodeSettings,
				  InputSettings     *pInputSettings)
{

	NEXUS_FrontendUserParameters userParams;
	NEXUS_FrontendHandle frontend=NULL;
	NEXUS_FrontendQamSettings qamSettings;
	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_ParserBand parserBand;
	NEXUS_ParserBandSettings parserBandSettings;
	NEXUS_StcChannelHandle stcChannel, stcChannelEncoder;
	NEXUS_StcChannelSettings stcSettings;
	NEXUS_DisplayHandle display, displayTranscode;
	NEXUS_DisplaySettings displaySettings;
	NEXUS_VideoWindowHandle window, windowTranscode;
	NEXUS_VideoDecoderHandle videoDecoder;

	NEXUS_VideoEncoderHandle videoEncoder;
	NEXUS_VideoEncoderSettings videoEncoderConfig;
	NEXUS_VideoEncoderDelayRange videoDelay;
	NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
	NEXUS_VideoEncoderStatus videoEncoderStatus;
	NEXUS_PidChannelHandle videoPidChannel;
	NEXUS_VideoDecoderStartSettings videoProgram;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;
	NEXUS_Error rc;

	unsigned i;
	void *pDataBuffer;
	FILE   *fout, *fdesc;
	size_t bytes;

	fdesc = fopen(pEncodeSettings->indexfname, "w");
	assert(fdesc);
	fprintf(fdesc, "flags(h) origPts(h) pts(Lh) 	escr(h) ticksPerBit(u) shr(d) offset(h) length(h)\n");

	NEXUS_Platform_Init(NULL);
	NEXUS_Platform_GetConfiguration(&platformConfig);

	for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
	{
		NEXUS_FrontendCapabilities capabilities;
		frontend = platformConfig.frontend[i];
		if (frontend) {
			NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
			/* Does this frontend support qam? */
			if ( capabilities.qam )
			{
				break;
			}
		}
	}



	if (NULL == frontend )
	{
		fprintf(stderr, "Unable to find QAM-capable frontend\n");
		return 0;
	}

	NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
	qamSettings.frequency = pInputSettings->freq* 1000000;
	qamSettings.mode = pInputSettings->qamMode;
	switch (pInputSettings->qamMode) {
		default:
		case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
		case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
		case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; /* TODO */ break;
	}
	qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
	qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
	qamSettings.lockCallback.callback = lock_callback;
	qamSettings.lockCallback.context = frontend;

	NEXUS_Frontend_GetUserParameters(frontend, &userParams);

	/* Map a parser band to the demod's input band. */
	parserBand = NEXUS_ParserBand_e0;
	NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
	parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
	parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
	parserBandSettings.transportType = pInputSettings->eStreamType;
	NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
	videoPidChannel = NEXUS_PidChannel_Open(parserBand, pInputSettings->iVideoPid, NULL);

	NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_ePcr;
	stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
	stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.format = NEXUS_VideoFormat_eNtsc;
	display = NEXUS_Display_Open(0, &displaySettings);



#if NEXUS_NUM_COMPONENT_OUTPUTS
	if(platformConfig.outputs.component[0]){
		NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
	}
#endif

	window = NEXUS_VideoWindow_Open(display, 0);

	/* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
	* which might cause encoder display GISB error since encoder display would
	* trigger RDC to program mailbox registers in ViCE2;
	*/
	videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
	assert(videoEncoder);


	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;

	if(!pEncodeSettings->bCustom)
	{
		displaySettings.format = (NEXUS_VideoFormat)pEncodeSettings->displayFormat;
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);
	}
	else
	{
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);

		NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
		customFormatSettings.width = pEncodeSettings->customFormatSettings.width;
		customFormatSettings.height = pEncodeSettings->customFormatSettings.height;
		customFormatSettings.refreshRate = pEncodeSettings->customFormatSettings.refreshRate;
		customFormatSettings.interlaced = false;
		customFormatSettings.aspectRatio = pEncodeSettings->customFormatSettings.aspectRatio;
		customFormatSettings.dropFrameAllowed = true;
		rc = NEXUS_Display_SetCustomFormatSettings(displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
		assert(!rc);
	}

	windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);

	videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
	NEXUS_VideoWindow_AddInput(window,          NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));

	NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
	videoProgram.codec = pInputSettings->encoderVideoCodec;
	videoProgram.pidChannel = videoPidChannel;
	videoProgram.stcChannel = stcChannel;

	NEXUS_Frontend_TuneQam(frontend, &qamSettings);

	NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

	/* encoder requires different STC broadcast mode from decoder */
	NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
	stcSettings.autoConfigTimebase = false; /* don't let encoder stc to auto config timebase */
	stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

	NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
	videoEncoderConfig.variableFrameRate = true;
	videoEncoderConfig.frameRate = pEncodeSettings->encoderFrameRate;
	videoEncoderConfig.bitrateMax = pEncodeSettings->encoderBitrate;
	videoEncoderConfig.streamStructure.framesP = pEncodeSettings->encoderGopStructureFramesP; /* IPP GOP size = 30 */
	videoEncoderConfig.streamStructure.framesB = pEncodeSettings->encoderGopStructureFramesB;

	NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
	videoEncoderStartConfig.codec = pEncodeSettings->encoderVideoCodec;
	videoEncoderStartConfig.profile = pEncodeSettings->encoderProfile;
	videoEncoderStartConfig.level = pEncodeSettings->encoderLevel;
	videoEncoderStartConfig.input = displayTranscode;
	videoEncoderStartConfig.stcChannel = stcChannelEncoder;

	/* NOTE: video encoder delay is in 27MHz ticks */
	NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
	BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
	videoEncoderConfig.encoderDelay = videoDelay.min;

	/* note the Dee is set by SetSettings */
	NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
	NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
	NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
	BDBG_ASSERT(videoEncoderStatus.bufferBlock);
	NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);
	fprintf(stderr, "videoEncoderStatus.bufferBlock: 0x%p (0x%p)\n", (void*)videoEncoderStatus.bufferBlock, (void*)pDataBuffer);

	fout = fopen(pEncodeSettings->fname,"wb");
	assert(fout);

	for(bytes=0;;) {
		size_t size[2];
		const NEXUS_VideoEncoderDescriptor *desc[2];
		unsigned i,j;
		unsigned descs;


		NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
		if(size[0]==0 && size[1]==0) {
			NEXUS_VideoDecoderStatus vstatus;

			NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
			fflush(fout);
			fprintf(stderr, "written "BDBG_UINT64_FMT" bytes.... decode:%u\t\r", BDBG_UINT64_ARG((uint64_t)bytes), vstatus.pts);
			BKNI_Sleep(30);
			continue;
		}
		for(descs=0,j=0;j<2;j++) {
			descs+=size[j];
			for(i=0;i<size[j];i++) {
				if(desc[j][i].length > 0)
				{
					if((desc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
						fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
					}
					fprintf(fdesc, "%8x %8x   %x%08x %08x	  %5u	%5d   %8x "BDBG_UINT64_FMT"\n", desc[j][i].flags, desc[j][i].originalPts,
						(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
						desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, BDBG_UINT64_ARG((uint64_t)desc[j][i].length));
					bytes+= desc[j][i].length;
					if(desc[j][i].length > 0x100000)
					{
						BDBG_ERR(("++++ desc[%d][%d] length = "BDBG_UINT64_FMT", offset=0x%x", j,i, BDBG_UINT64_ARG((uint64_t)desc[j][i].length), desc[j][i].offset));
					}

				}
			}
		}

		if(kbhit()) break;
		NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
	}
	NEXUS_MemoryBlock_Unlock(videoEncoderStatus.bufferBlock);
	/* Bring down system */
	NEXUS_VideoEncoder_Stop(videoEncoder, NULL);/* stop encoder first since file capture already stopped to avoid VCE timeout */

	NEXUS_VideoDecoder_Stop(videoDecoder);

	NEXUS_VideoWindow_RemoveInput(window,          NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_Close(window);
	NEXUS_VideoWindow_Close(windowTranscode);
	NEXUS_Display_Close(displayTranscode);
	NEXUS_Display_Close(display);

	NEXUS_PidChannel_Close(videoPidChannel);
	NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoDecoder_Close(videoDecoder);

	NEXUS_VideoEncoder_Close(videoEncoder);
	NEXUS_StcChannel_Close(stcChannelEncoder);
	NEXUS_StcChannel_Close(stcChannel);
	fclose(fout);
	fclose(fdesc);

	NEXUS_Platform_Uninit();
	return 0;
}
#endif

int transcode_hdmi(EncodeSettings* pEncodeSettings)
{
#if NEXUS_NUM_HDMI_INPUTS
	NEXUS_PlatformSettings platformSettings;
	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_StcChannelHandle stcChannel,stcChannelEncoder;
	NEXUS_StcChannelSettings stcSettings;
	NEXUS_DisplayHandle display;
	NEXUS_VideoWindowHandle window;

	NEXUS_DisplayHandle displayTranscode;
	NEXUS_VideoWindowHandle windowTranscode;
	NEXUS_VideoEncoderHandle videoEncoder;
	NEXUS_VideoEncoderSettings videoEncoderConfig;
	NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
	NEXUS_VideoEncoderDelayRange videoDelay;
	NEXUS_VideoEncoderStatus videoEncoderStatus;
	void *pDataBuffer;

	NEXUS_HdmiInputHandle hdmiInput;
	NEXUS_HdmiInputSettings hdmiInputSettings;


	NEXUS_DisplaySettings displaySettings;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;

	FILE *fout;
	FILE *fdesc;
	size_t bytes;
	NEXUS_Error rc;

	fdesc = fopen(pEncodeSettings->indexfname, "w");
	fprintf(fdesc, "flags(h) origPts(h) pts(Lh) 	escr(h) ticksPerBit(u) shr(d) offset(h) length(h)\n");
	assert(fdesc);

	NEXUS_Platform_GetDefaultSettings(&platformSettings);
	platformSettings.openFrontend = false;
	NEXUS_Platform_Init(&platformSettings);
	NEXUS_Platform_GetConfiguration(&platformConfig);


	NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
	hdmiInputSettings.timebase = NEXUS_Timebase_e0;
	hdmiInput = NEXUS_HdmiInput_Open(0, &hdmiInputSettings);
	if(!hdmiInput) {
		fprintf(stderr, "Can't get hdmi input\n");
		return -1;
	}


	NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

	/* Bring up video display and outputs */
	display = NEXUS_Display_Open(0, NULL);
#if NEXUS_NUM_COMPONENT_OUTPUTS
	if(platformConfig.outputs.component[0]){
		NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
	}
#endif

	window = NEXUS_VideoWindow_Open(display, 0);

	/* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
	* which might cause encoder display GISB error since encoder display would
	* trigger RDC to program mailbox registers in ViCE2;
	*/
	videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
	assert(videoEncoder);

	/* Bring up video encoder display */
	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;

	if(!pEncodeSettings->bCustom)
	{
		displaySettings.format = (NEXUS_VideoFormat)pEncodeSettings->displayFormat;
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);
	}
	else
	{
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);

		NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
		customFormatSettings.width = pEncodeSettings->customFormatSettings.width;
		customFormatSettings.height = pEncodeSettings->customFormatSettings.height;
		customFormatSettings.refreshRate = pEncodeSettings->customFormatSettings.refreshRate;
		customFormatSettings.interlaced = false;
		customFormatSettings.aspectRatio = pEncodeSettings->customFormatSettings.aspectRatio;
		customFormatSettings.dropFrameAllowed = true;
		rc = NEXUS_Display_SetCustomFormatSettings(displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
		assert(!rc);
	}

	windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
	assert(windowTranscode);

	/* connect same decoder to the encoder display;
	* NOTE: simul display + transcode mode might have limitation in audio pathre;
	* here is for video transcode bringup purpose;
	*/
	NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
	NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));

	/* encoder requires different STC broadcast mode from decoder */
	NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
	stcSettings.autoConfigTimebase = false; /* don't let encoder stc to auto config timebase */
	stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

	NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
	videoEncoderConfig.variableFrameRate = true;
	videoEncoderConfig.frameRate = pEncodeSettings->encoderFrameRate;
	videoEncoderConfig.bitrateMax = pEncodeSettings->encoderBitrate;
	videoEncoderConfig.streamStructure.framesP = pEncodeSettings->encoderGopStructureFramesP;
	videoEncoderConfig.streamStructure.framesB = pEncodeSettings->encoderGopStructureFramesB;
	NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);

	NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
	videoEncoderStartConfig.codec   = pEncodeSettings->encoderVideoCodec;
	videoEncoderStartConfig.profile = pEncodeSettings->encoderProfile;
	videoEncoderStartConfig.level   = pEncodeSettings->encoderLevel;
	videoEncoderStartConfig.input = displayTranscode;
	videoEncoderStartConfig.stcChannel = stcChannelEncoder;

	/* NOTE: video encoder delay is in 27MHz ticks */
	NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
	BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
	videoEncoderConfig.encoderDelay = videoDelay.min;
	NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

	NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
	NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);

	fout = fopen(pEncodeSettings->fname,"wb");
	assert(fout);

	for(bytes=0;;) {
		size_t size[2];
		const NEXUS_VideoEncoderDescriptor *desc[2];
		unsigned i,j;
		unsigned descs;


		NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
		if(size[0]==0 && size[1]==0) {
			fflush(fout);
			fprintf(stderr, "written "BDBG_UINT64_FMT" bytes.... \t\r", BDBG_UINT64_ARG((uint64_t)bytes));
			BKNI_Sleep(30);
			continue;
		}
		for(descs=0,j=0;j<2;j++) {
			descs+=size[j];
			for(i=0;i<size[j];i++) {
				if(desc[j][i].length > 0)
				{
					if((desc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
						fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
					}
					fprintf(fdesc, "%8x %8x   %x%08x %08x	  %5u	%5d   %8x "BDBG_UINT64_FMT"\n", desc[j][i].flags, desc[j][i].originalPts,
						(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
						desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, BDBG_UINT64_ARG((uint64_t)desc[j][i].length));
					bytes+= desc[j][i].length;
					if(desc[j][i].length > 0x100000)
					{
						BDBG_ERR(("++++ desc[%d][%d] length = "BDBG_UINT64_FMT", offset=0x%x", j,i, BDBG_UINT64_ARG((uint64_t)desc[j][i].length), desc[j][i].offset));
					}

				}
			}
		}
		NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
		if(kbhit()) break;
	}

	/* Bring down system */
	NEXUS_VideoEncoder_Stop(videoEncoder, NULL);

	NEXUS_VideoWindow_RemoveInput(window,          NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
	NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
	NEXUS_VideoWindow_Close(window);
	NEXUS_VideoWindow_Close(windowTranscode);
	NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
	NEXUS_HdmiInput_Close(hdmiInput);
	NEXUS_Display_Close(display);
	NEXUS_StcChannel_Close(stcChannel);

	NEXUS_Display_Close(displayTranscode);
	NEXUS_VideoEncoder_Close(videoEncoder);
	NEXUS_StcChannel_Close(stcChannelEncoder);
	fclose(fout);
	fclose(fdesc);
	NEXUS_Platform_Uninit();
	return (0);
#else
	printf("\n No HDMI input supported\n\n");
	BSTD_UNUSED(pEncodeSettings);
	return (0);
#endif
}

int transcode_file(EncodeSettings* pEncodeSettings,
				   InputSettings*     pInputSettings)
{

	NEXUS_PlatformSettings platformSettings;
	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_StcChannelHandle stcChannel, stcChannelEncoder;
	NEXUS_StcChannelSettings stcSettings;
	NEXUS_PidChannelHandle videoPidChannel, pcrPidChannel;
	NEXUS_VideoDecoderHandle videoDecoder;
	NEXUS_VideoDecoderStartSettings videoProgram;
	NEXUS_FilePlayHandle file;
	NEXUS_PlaypumpHandle playpump;
	NEXUS_PlaybackHandle playback;
	NEXUS_PlaybackSettings playbackSettings;
	NEXUS_PlaybackPidChannelSettings playbackPidSettings;

	NEXUS_DisplayHandle display, displayTranscode;
	NEXUS_VideoWindowHandle window, windowTranscode;
	NEXUS_VideoEncoderHandle videoEncoder;
	NEXUS_VideoEncoderSettings videoEncoderConfig;
	NEXUS_VideoEncoderDelayRange videoDelay;
	NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
	NEXUS_VideoEncoderStatus videoEncoderStatus;
	NEXUS_DisplaySettings displaySettings;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;
	NEXUS_VideoWindowCoefficientIndexSettings coeffIndexSettings;
	const char *customCoeffs = NULL;
	bool bCustomCoeffs = false;
	NEXUS_VideoWindowScalerSettings sclSettings;
	NEXUS_Error rc;
	void *pDataBuffer;

	FILE *fout;
	FILE *fdesc;
	size_t bytes;

	assert(pInputSettings);
	assert(pEncodeSettings);

	fdesc = fopen(pEncodeSettings->indexfname, "w");
	fprintf(fdesc, "flags(h) origPts(h) pts(Lh) escr(h) ticksPerBit(u) shr(d) offset(h) length(h)\n");
	assert(fdesc);

	NEXUS_Platform_GetDefaultSettings(&platformSettings);
	platformSettings.openFrontend = false;
	NEXUS_Platform_Init(&platformSettings);
	NEXUS_Platform_GetConfiguration(&platformConfig);

	playpump = NEXUS_Playpump_Open(0, NULL);
	assert(playpump);
	playback = NEXUS_Playback_Create();
	assert(playback);

	file = NEXUS_FilePlay_OpenPosix(pInputSettings->fname, NULL);

	if (!file) {
		fprintf(stderr, "can't open file:%s\n", pInputSettings->fname);
		return -1;
	}

	NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

	NEXUS_Playback_GetSettings(playback, &playbackSettings);
	playbackSettings.playpump = playpump;
	/* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
	playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)pInputSettings->eStreamType;
	playbackSettings.stcChannel = stcChannel;
	NEXUS_Playback_SetSettings(playback, &playbackSettings);


	/* bring up decoder and connect to local display */
	videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

	/* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
	* which might cause encoder display GISB error since encoder display would
	* trigger RDC to program mailbox registers in ViCE2;
	*/
	videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
	assert(videoEncoder);

	/* Bring up video display and outputs */
	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.format = NEXUS_VideoFormat_e480p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
   if(platformConfig.outputs.component[0]){
    	NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
   }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
   if(platformConfig.outputs.hdmi[0]){
    	NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
   }
#endif

    window = NEXUS_VideoWindow_Open(display, 0);

	/* Bring up video encoder display */
	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
	if(!pEncodeSettings->bCustom)
	{
		displaySettings.format = (NEXUS_VideoFormat)pEncodeSettings->displayFormat;
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);
		windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
		assert(windowTranscode);

	}
	else
	{
		displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
		assert(displayTranscode);
		windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
		assert(windowTranscode);


		NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
		customFormatSettings.width = pEncodeSettings->customFormatSettings.width;
		customFormatSettings.height = pEncodeSettings->customFormatSettings.height;
		customFormatSettings.refreshRate = pEncodeSettings->customFormatSettings.refreshRate;
		customFormatSettings.interlaced = false;
		customFormatSettings.aspectRatio = pEncodeSettings->customFormatSettings.aspectRatio;
		customFormatSettings.dropFrameAllowed = true;
		rc = NEXUS_Display_SetCustomFormatSettings(displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
		assert(!rc);
	}

	/* connect same decoder to encoder display
	* This simul mode is for video encoder bringup only; audio path may have limitation
	* for simul display+transcode mode;
	*/
	NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_AddInput(window,          NEXUS_VideoDecoder_GetConnector(videoDecoder));


	/* override window scaler coeffs */
	NEXUS_VideoWindow_GetCoefficientIndexSettings(windowTranscode, &coeffIndexSettings);
	customCoeffs = NEXUS_GetEnv("scl_coeff_luma_h");
	if(customCoeffs) {
		coeffIndexSettings.sclHorzLuma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("scl_coeff_luma_v");
	if(customCoeffs) {
		coeffIndexSettings.sclVertLuma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("scl_coeff_chroma_h");
	if(customCoeffs) {
		coeffIndexSettings.sclHorzChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("scl_coeff_chroma_v");
	if(customCoeffs) {
		coeffIndexSettings.sclVertChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("hscl_coeff_chroma");
	if(customCoeffs) {
		coeffIndexSettings.hsclHorzChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("hscl_coeff_luma");
	if(customCoeffs) {
		coeffIndexSettings.hsclHorzChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("mad_coeff_chroma");
	if(customCoeffs) {
		coeffIndexSettings.madHorzChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}
	customCoeffs = NEXUS_GetEnv("mad_coeff_luma");
	if(customCoeffs) {
		coeffIndexSettings.madHorzChroma = strtoul(customCoeffs, NULL, 0);
		bCustomCoeffs = true;
	}

	if(bCustomCoeffs) {
		NEXUS_VideoWindow_SetCoefficientIndexSettings(windowTranscode, &coeffIndexSettings);
	}

	/* dejag */
	NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
	customCoeffs = NEXUS_GetEnv("dejag");
	if(customCoeffs) {
		sclSettings.verticalDejagging = strtoul(customCoeffs, NULL, 0);
		BDBG_WRN(("dejag? %d", sclSettings.verticalDejagging));
		NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
	}

	/* Open the video pid channel */
	NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
	playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
	playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pInputSettings->encoderVideoCodec; /* must be told codec for correct handling */
	playbackPidSettings.pidTypeSettings.video.index = true;
	playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
	videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, pInputSettings->iVideoPid, &playbackPidSettings);
	if(pInputSettings->iVideoPid != pInputSettings->iPcrPid)
	{
		playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
		pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, pInputSettings->iPcrPid, &playbackPidSettings);
	}

	/* Set up decoder Start structures now. We need to know the audio codec to properly set up
	the audio outputs. */
	NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
	videoProgram.codec = (NEXUS_VideoCodec)pInputSettings->encoderVideoCodec;
	videoProgram.pidChannel = videoPidChannel;
	videoProgram.stcChannel = stcChannel;

	NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
	videoEncoderConfig.variableFrameRate = true;
	videoEncoderConfig.frameRate = pEncodeSettings->encoderFrameRate;
	videoEncoderConfig.bitrateMax = pEncodeSettings->encoderBitrate;
	videoEncoderConfig.streamStructure.framesP = pEncodeSettings->encoderGopStructureFramesP; /* IPP GOP size = 30 */
	videoEncoderConfig.streamStructure.framesB = pEncodeSettings->encoderGopStructureFramesB;

	/* Start decoders */
	NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

	/* Start playback */
	NEXUS_Playback_Start(playback, file, NULL);

	BKNI_Sleep(1000);

	/* encoder requires different STC broadcast mode from decoder */
	NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
	stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

	NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
	videoEncoderStartConfig.codec = pEncodeSettings->encoderVideoCodec;
	videoEncoderStartConfig.profile = pEncodeSettings->encoderProfile;
	videoEncoderStartConfig.level = pEncodeSettings->encoderLevel;
	videoEncoderStartConfig.input = displayTranscode;
	videoEncoderStartConfig.stcChannel = stcChannelEncoder;

	/* NOTE: video encoder delay is in 27MHz ticks */
	NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
	BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
	videoEncoderConfig.encoderDelay = videoDelay.min;

	/* note the Dee is set by SetSettings */
	NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
	NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
	NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
	BDBG_ASSERT(videoEncoderStatus.bufferBlock);
	NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);

	fout = fopen(pEncodeSettings->fname,"wb");
	assert(fout);

	printf("\nEnter Any Key to Quit\n\n");
	for(bytes=0;;) {
		size_t size[2];
		const NEXUS_VideoEncoderDescriptor *desc[2];
		unsigned i,j;
		unsigned descs;


		NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
		if(size[0]==0 && size[1]==0) {
			NEXUS_VideoDecoderStatus vstatus;

			NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
			fflush(fout);
			fprintf(stderr, "written "BDBG_UINT64_FMT" bytes.... decode:%u\t\r", BDBG_UINT64_ARG((uint64_t)bytes), vstatus.pts);
			BKNI_Sleep(30);
			continue;
		}
		for(descs=0,j=0;j<2;j++) {
			descs+=size[j];
			for(i=0;i<size[j];i++) {
				if(desc[j][i].length > 0)
				{
					if((desc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
						fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
					}
					fprintf(fdesc, "%8x %8x   %x%08x %08x	  %5u	%5d   %8x "BDBG_UINT64_FMT"\n", desc[j][i].flags, desc[j][i].originalPts,
						(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
						desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, BDBG_UINT64_ARG((uint64_t)desc[j][i].length));
					bytes+= desc[j][i].length;
					if(desc[j][i].length > 0x100000)
					{
						BDBG_ERR(("++++ desc[%d][%d] length = "BDBG_UINT64_FMT", offset=0x%x", j,i, BDBG_UINT64_ARG((uint64_t)desc[j][i].length), desc[j][i].offset));
					}

				}
			}
		}
		NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
		if(kbhit()) break;
	}
	NEXUS_MemoryBlock_Unlock(videoEncoderStatus.bufferBlock);

	/* Bring down system */
	/**************************************************
	* NOTE: stop sequence should be in front->back order
	*/

	NEXUS_VideoEncoder_Stop(videoEncoder, NULL);
	NEXUS_Playback_Stop(playback);
	NEXUS_VideoDecoder_Stop(videoDecoder);
	NEXUS_VideoWindow_RemoveInput(window,          NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoWindow_Close(window);
	NEXUS_VideoWindow_Close(windowTranscode);
	NEXUS_Display_Close(display);
	NEXUS_Display_Close(displayTranscode);

	NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
	NEXUS_FilePlay_Close(file);

	NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
	NEXUS_VideoDecoder_Close(videoDecoder);

	NEXUS_Playback_Destroy(playback);
	NEXUS_Playpump_Close(playpump);

	NEXUS_VideoEncoder_Close(videoEncoder);
	NEXUS_StcChannel_Close(stcChannelEncoder);

	fclose(fout);
	fclose(fdesc);

	NEXUS_Platform_Uninit();

	return (0);

}

int main(void)  {

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER

	InputSettings stInput;
	EncodeSettings stEncode;
	char *tmpPtr;



	printf("\n choose resource (0) File playback (1) HDMI (2) QAM      ");scanf("%d", (int32_t*)&stInput.resource);

	switch (stInput.resource)
	{
	case (BTST_RESOURCE_HDMI):
		break;
	case (BTST_RESOURCE_QAM):
		{
#if  BTST_SUPPORT_FRONTEND
			printf("\n Front End QAM freq (MHz) ");                          scanf("%d", (int32_t*)&stInput.freq);
			printf("\n Front End QAM Mode (%d) 64 (%d) 256 (%d) 1024 ", NEXUS_FrontendQamMode_e64, NEXUS_FrontendQamMode_e256, NEXUS_FrontendQamMode_e1024);
			scanf("%d", (int32_t*)&stInput.qamMode);
			printf("\n source stream type (%d) Ts           ", NEXUS_TransportType_eTs); stInput.eStreamType = NEXUS_TransportType_eTs;
			printf("\n source stream codec (%d) Mpeg2 (%d) H264    ", NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264);
			scanf("%d", (int32_t*)&stInput.encoderVideoCodec);
			printf("\n Video pid										   ");	   scanf("%d", &stInput.iVideoPid);
			printf("\n Pcr	 pid										   ");	   scanf("%d", &stInput.iPcrPid);
			break;
#else
			printf("This platform doesn't support frontend.\n");
			return -1;
#endif
		}
	case (BTST_RESOURCE_FILE):
	default:
		{
			printf("\n source stream file: ");
			scanf("%s", stInput.fname);
			printf("\n source stream type (%d) Es (%d) Ts            ", NEXUS_TransportType_eEs, NEXUS_TransportType_eTs);
			scanf("%d", (int32_t*)&stInput.eStreamType);
			printf("\n source stream codec (%d) Mpeg2 (%d) H264    ", NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264);
			scanf("%d", (int32_t*)&stInput.encoderVideoCodec);
			printf("\n Video pid										   ");	   scanf("%d", &stInput.iVideoPid);
			printf("\n Pcr	 pid										   ");	   scanf("%d", &stInput.iPcrPid);
		}
		stInput.resource = BTST_RESOURCE_FILE;
	}





	printf("\n****************************************************************\n");
	printf("\n Input Parameters\n");
	printf("\n Source   %d", stInput.resource);
	if(stInput.resource != BTST_RESOURCE_HDMI)
	{
	if(stInput.resource == BTST_RESOURCE_FILE)
		printf("\n filename           %s", stInput.fname);

#if  BTST_SUPPORT_FRONTEND
	if(stInput.resource == BTST_RESOURCE_QAM)
		printf("\n freq %d qam %d", stInput.freq, stInput.qamMode);
#endif

	printf("\n Streamtype %d iencoderVideoCodec %d", stInput.eStreamType, stInput.encoderVideoCodec);
	printf("\n vPid %4d, PcrPid  %4d", stInput.iVideoPid, stInput.iPcrPid);
	}
	printf("\n****************************************************************\n");


	printf("\n\n Encode Settings");
	printf("\n encode stream                                                                        "); scanf("%s", stEncode.fname);
	printf("\n custom format (0) No (1) Yes                                                         "); scanf("%d", (int32_t*)&stEncode.bCustom);
	if(!stEncode.bCustom)
	{
		printf("\n enocder displayFormat:\n"
			" (%2d) 480i\n"
			" (%2d) 576i\n"
			" (%2d) 480p\n"
			" (%2d) 576p\n"
			" (%2d) 1080i60\n"
			" (%2d) 1080i50\n"
			" (%2d) 1080p24\n"
			" (%2d) 1080p25\n"
			" (%2d) 1080p30\n"
			" (%2d) 1080p50\n"
			" (%2d) 1080p60\n"
			" (%2d)  720p60\n"
			" (%2d)  720p50\n"
			" (%2d) 720p24\n"
			" (%2d) 720p25\n"
			" (%2d) 720p30Hz:\n",
			(NEXUS_VideoFormat_eNtsc),
			(NEXUS_VideoFormat_ePal),
			(NEXUS_VideoFormat_e480p),
			(NEXUS_VideoFormat_e576p),
			(NEXUS_VideoFormat_e1080i),
			(NEXUS_VideoFormat_e1080i50hz),
			(NEXUS_VideoFormat_e1080p24hz),
			(NEXUS_VideoFormat_e1080p25hz),
			(NEXUS_VideoFormat_e1080p30hz),
			(NEXUS_VideoFormat_e1080p50hz),
			(NEXUS_VideoFormat_e1080p60hz),
			(NEXUS_VideoFormat_e720p),
			(NEXUS_VideoFormat_e720p50hz),
			(NEXUS_VideoFormat_e720p24hz),
			(NEXUS_VideoFormat_e720p25hz),
			(NEXUS_VideoFormat_e720p30hz));

		scanf("%d", (int32_t*)&stEncode.displayFormat);
	}
	else
	{
		printf("\n Resolution width & height                                                        ");
		scanf("%d %d", (int32_t*)&stEncode.customFormatSettings.width, (int32_t*)&stEncode.customFormatSettings.height);

		printf("\n refresh rate                                                                     "); scanf("%d", (int32_t*)&stEncode.customFormatSettings.refreshRate);
		printf("\n Aspect Ratio (0) Auto (1) 4x3 (2) 16x9                                           "); scanf("%d", (int32_t*)&stEncode.customFormatSettings.aspectRatio);
	}

	printf("\n Frame rate:\n"
			" (%d) 23.976\n"
			" (%d) 24\n"
			" (%d) 25\n"
			" (%d) 29.97Hz\n"
			" (%d) 30 Hz\n"
			" (%d) 50 Hz\n"
			" (%d) 59.94\n"
			" (%d) 60\n"
			" (%d) 14.985\n",
			NEXUS_VideoFrameRate_e23_976,
			NEXUS_VideoFrameRate_e24,
			NEXUS_VideoFrameRate_e25,
			NEXUS_VideoFrameRate_e29_97,
			NEXUS_VideoFrameRate_e30,
			NEXUS_VideoFrameRate_e50,
			NEXUS_VideoFrameRate_e59_94,
			NEXUS_VideoFrameRate_e60,
			NEXUS_VideoFrameRate_e14_985);
	scanf("%d", (int32_t*)&stEncode.encoderFrameRate);

	printf("\n Bitrate (bps)                                                                        "); scanf("%d", (int32_t*)&stEncode.encoderBitrate);
	printf("\n P frame                                                                              "); scanf("%d", (int32_t*)&stEncode.encoderGopStructureFramesP);
	printf("\n B frame                                                                              "); scanf("%d", (int32_t*)&stEncode.encoderGopStructureFramesB);
	printf("\n Encode Video Codec: (%d) MPEG2 (%d) MPEG4 Part2 (%d) H264 ", NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eMpeg4Part2, NEXUS_VideoCodec_eH264);
	scanf("%d", (int32_t*)&stEncode.encoderVideoCodec);
	printf("\n Profile: (%d) Simple (%d) Main (%d) High (%d) Baseline                               ",
			NEXUS_VideoCodecProfile_eSimple, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecProfile_eHigh, NEXUS_VideoCodecProfile_eBaseline);
	scanf("%d", (int32_t*)&stEncode.encoderProfile);

	printf("\n Level:\n"
		" (%2d) 1.0\n"
		" (%2d) 2.0\n"
		" (%2d) 2.1\n"
		" (%2d) 2.2\n"
		" (%2d) 3.0\n"
		" (%2d) 3.1\n"
		" (%2d) 3.2\n"
		" (%2d) 4.0\n"
		" (%2d) 4.1\n"
		" (%2d) 4.2\n"
		" (%2d) 5.0\n"
		" (%2d) Low\n"
		" (%2d) Main\n"
		" (%2d) High\n",
		NEXUS_VideoCodecLevel_e10, NEXUS_VideoCodecLevel_e20, NEXUS_VideoCodecLevel_e21, NEXUS_VideoCodecLevel_e22, NEXUS_VideoCodecLevel_e30,
		NEXUS_VideoCodecLevel_e31, NEXUS_VideoCodecLevel_e32, NEXUS_VideoCodecLevel_e40, NEXUS_VideoCodecLevel_e41,
		NEXUS_VideoCodecLevel_e42, NEXUS_VideoCodecLevel_e50, NEXUS_VideoCodecLevel_eLow, NEXUS_VideoCodecLevel_eMain, NEXUS_VideoCodecLevel_eHigh);

	scanf("%d", (int32_t*)&stEncode.encoderLevel);


	printf("\n****************************************************************\n");
	printf("\n Encode Parameters\n");
	printf("\n filename           %s", stEncode.fname);
	printf("\n display format     %d", stEncode.displayFormat);
	printf("\n Frame Rate %d Bit Rate %d", stEncode.encoderFrameRate, stEncode.encoderBitrate);
	printf("\n P frame %d B frame %d", stEncode.encoderGopStructureFramesP, stEncode.encoderGopStructureFramesB);
	printf("\n Video Codec %4d, Profile %4d Level %4d", stEncode.encoderVideoCodec, stEncode.encoderProfile,stEncode.encoderLevel );
	printf("\n****************************************************************\n");
	strncpy(stEncode.indexfname, stEncode.fname, strlen(stEncode.fname));
	tmpPtr = strstr(stEncode.indexfname, ".");
	if (tmpPtr) {
	    strncpy(tmpPtr+1, "nav", 3);
	}
	else {
	    strncat(stEncode.indexfname, ".nav", sizeof(stEncode.indexfname) - 1);
	}
	BDBG_MSG(("Record file name: %s, descriptor file name %s", stEncode.fname, stEncode.indexfname));

	switch (stInput.resource)
	{
	case (BTST_RESOURCE_HDMI):
		{
			transcode_hdmi(&stEncode);
			break;
		}
#if  BTST_SUPPORT_FRONTEND
	case (BTST_RESOURCE_QAM):
		{
			transcode_qam(&stEncode, &stInput);
			break;
		}
#endif
	case (BTST_RESOURCE_FILE):
	default:
		{
			transcode_file(&stEncode, &stInput);
			break;
		}
	}


#endif
	return 0;
}

#endif
