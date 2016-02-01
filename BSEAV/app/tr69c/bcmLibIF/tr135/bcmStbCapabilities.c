 /******************************************************************************
  *    (c)2010-2013 Broadcom Corporation
  *
  * This program is the proprietary software of Broadcom Corporation and/or its licensors,
  * and may only be used, duplicated, modified or distributed pursuant to the terms and
  * conditions of a separate, written license agreement executed between you and Broadcom
  * (an "Authorized License").	Except as set forth in an Authorized License, Broadcom grants
  * no license (express or implied), right to use, or waiver of any kind with respect to the
  * Software, and Broadcom expressly reserves all rights in and to the Software and all
  * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
  * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
  * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  *
  * Except as expressly set forth in the Authorized License,
  *
  * 1.	   This program, including its structure, sequence and organization, constitutes the valuable trade
  * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
  * and to use this information only in connection with your use of Broadcom integrated circuit products.
  *
  * 2.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
  * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
  * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
  * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
  * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
  * USE OR PERFORMANCE OF THE SOFTWARE.
  *
  * 3.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
  *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "../main/utils.h"
#include "../SOAPParser/CPEframework.h"

#include "../main/types.h"
#include "syscall.h"

#include "bcmTR135Objs.h"
#include "nexus_platform_features.h"

#define MAX_VideoCodecStandard	18
#define MAX_AudioCodecStandard	33

/****************************************************/
/* STBService.Capabilities.AudienceStats Object     */
/****************************************************/
TRX_STATUS getCapASMaxAudienceStatsChannels(char **value)
{
	uint32 	channel=1;

	*value = strdup(itoa(channel));
	return TRX_OK;
}

/****************************************************/
/* STBService.Capabilities.ServiceMonitoring Object */
/****************************************************/
TRX_STATUS getCapSMServiceTypes(char **value)
{
	*value = strdup("IPTV, PVR");
	return TRX_OK;
}

TRX_STATUS getCapSMMaxEventsPerSampleInterval(char **value)
{
	uint32 max_event_per_sample_interval=1;
	*value = strdup(itoa(max_event_per_sample_interval));
	return TRX_OK;
}

TRX_STATUS getCapSMMaxActiveMainStreams(char **value)
{
	uint32 max_active_streams=1;

	*value = strdup(itoa(max_active_streams));
	return TRX_OK;
}

TRX_STATUS getCapSMMinSampleInterval(char **value)
{
	uint32 min_sample_interval=1;

	*value = strdup(itoa(min_sample_interval));
	return TRX_OK;
}

TRX_STATUS getCapSMMaxReportSamples(char **value)
{
	uint32 max_report_samples=1;
	*value = strdup(itoa(max_report_samples));
	return TRX_OK;
}

TRX_STATUS getCapSMHighLevelMetricNames(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

/****************************************************/
/* STBService.Capabilities.DRM Object                */
/****************************************************/
TRX_STATUS getCapDRMDRMSystems(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

/****************************************************/
/* STBService.Capabilities.CA Object                */
/****************************************************/
TRX_STATUS getCapCACASystems(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}


/****************************************************/
/* STBService.Capabilities.CDS Object                */
/****************************************************/
TRX_STATUS getCapCDSPushCapable(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getCapCDSPullCapable(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}


/****************************************************/
/* STBService.Capabilities.HDMI Object                */
/****************************************************/
TRX_STATUS getCapHDMISupportedResolutions(char **value)
{
	char hdmiFormats[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;hdmiVideoFormat[j].name;j++) {
		strncat(hdmiFormats, hdmiVideoFormat[j].name, strlen(hdmiVideoFormat[j].name));
		if (hdmiVideoFormat[j+1].name){
			strncat(hdmiFormats, sep, strlen(sep));
		}
	}

	*value = strdup(hdmiFormats);
	return TRX_OK;
}

TRX_STATUS getCapHDMICECSupport(char **value)
{
	*value = strdup("Supported");
	return TRX_OK;
}

TRX_STATUS getCapHDMIHDMI3D(char **value)
{
	*value = strdup("Supported");
	return TRX_OK;
}


/****************************************************/
/* STBService.Capabilities.VideoOutput Object       */
/****************************************************/
TRX_STATUS getCapVideoOutputCompositeVideoStandards(char **value)
{
	char compFormats[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;compositeVideoStandard[j].name;j++) {
		strncat(compFormats, compositeVideoStandard[j].name, strlen(compositeVideoStandard[j].name));
		if (compositeVideoStandard[j+1].name){
			strncat(compFormats, sep, strlen(sep));
		}
	}

	*value = strdup(compFormats);
	return TRX_OK;
}

TRX_STATUS getCapVideoOutputVideoFormats(char **value)
{
	char outputFormat[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;videoOutputFormats[j].name;j++) {
		strncat(outputFormat, videoOutputFormats[j].name, strlen(videoOutputFormats[j].name));
		if (videoOutputFormats[j+1].name){
			strncat(outputFormat, sep, strlen(sep));
		}
	}

	*value = strdup(outputFormat);
	return TRX_OK;
}

TRX_STATUS getCapVideoOutputMacrovision(char **value)
{
	*value = strdup("ROVI-REV7.1.L1, ROVI-REV1.4");
	return TRX_OK;
}

TRX_STATUS getCapVideoOutputHDCP(char **value)
{
#if HDCP_SUPPORT_ON_HDMI_OUTPUT
	*value = strdup("HDCP 1.4");
#else
	*value = strdup("Not supported");
#endif
	return TRX_OK;
}

TRX_STATUS getCapVideoOutputDislayFormats(char **value)
{
	char displayFormat[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;videoOutputDisplayFormat[j].name;j++) {
		strncat(displayFormat, videoOutputDisplayFormat[j].name, strlen(videoOutputDisplayFormat[j].name));
		if (videoOutputDisplayFormat[j+1].name){
			strncat(displayFormat, sep, strlen(sep));
		}
	}

	*value = strdup(displayFormat);
	return TRX_OK;
}

/****************************************************/
/* STBService.Capabilities.AudioOutput Object       */
/****************************************************/
TRX_STATUS getCapAudioOutputAudioFormats(char **value)
{
	char audioFormat[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;audioOutputFormat[j].name;j++) {
		strncat(audioFormat, audioOutputFormat[j].name, strlen(audioOutputFormat[j].name));
		if (audioOutputFormat[j+1].name){
			strncat(audioFormat, sep, strlen(sep));
		}
	}

	*value = strdup(audioFormat);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder.SMPTEVC1.ProfileLevel Object */
/*********************************************************************/
TRX_STATUS setCapVideoDecoderSMPTEVC1PLAlias(char *value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	strcpy(pProfileLevel[index-1].alias, value);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderSMPTEVC1PLAlias(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	*value = strdup(pProfileLevel[index-1].alias);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderSMPTEVC1PLProfile(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	*value = strdup(enumToString(videoProfileStrs, pProfileLevel[index-1].profile));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderSMPTEVC1PLLevel(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	*value = strdup(enumToString(videoLevelStrs, pProfileLevel[index-1].level));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderSMPTEVC1PLMaximumDecodingCapability(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[15];

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	snprintf(tmp, 10, "%f", pProfileLevel[index-1].max_decode);
	*value = strdup(tmp);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder.SMPTEVC1 Object              */
/*********************************************************************/
TRX_STATUS getCapVideoDecoderSMPTEVC1AudioStandards(char **value)
{
	*value = strdup("MP3-Surround, MPEG2-AAC-LC, MPEG4-AAC-LC");
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderSMPTEVC1ProfileLevelNumberOfEntries(char **value)
{
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[5];

	getProfileLevel(&pProfileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
	snprintf(tmp, 3, "%d", numEntries);
	*value = strdup(tmp);
	return TRX_OK;
}

/************************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG4Part10.ProfileLevel Object */
/************************************************************************/
TRX_STATUS setCapVideoDecoderMPEG4Part10PLAlias(char *value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	strcpy(pProfileLevel[index-1].alias, value);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part10PLAlias(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	*value = strdup(pProfileLevel[index-1].alias);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part10PLProfile(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	*value = strdup(enumToString(videoProfileStrs, pProfileLevel[index-1].profile));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part10PLLevel(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	*value = strdup(enumToString(videoLevelStrs, pProfileLevel[index-1].level));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part10PLMaximumDecodingCapability(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[15];

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	snprintf(tmp, 10, "%f", pProfileLevel[index-1].max_decode);
	*value = strdup(tmp);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG4Part10 Object           */
/*********************************************************************/
TRX_STATUS getCapVideoDecoderMPEG4Part10AudioStandards(char **value)
{
	*value = strdup("MP3-Surround, MPEG2-AAC-LC, MPEG4-AAC-LC");
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part10ProfileLevelNumberOfEntries(char **value)
{
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[5];

	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
	snprintf(tmp, 3, "%d", numEntries);
	*value = strdup(tmp);
	return TRX_OK;
}

/************************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG4Part2.ProfileLevel Object  */
/************************************************************************/
TRX_STATUS setCapVideoDecoderMPEG4Part2PLAlias(char *value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	strcpy(pProfileLevel[index-1].alias, value);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part2PLAlias(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	*value = strdup(pProfileLevel[index-1].alias);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part2PLProfile(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	*value = strdup(enumToString(videoProfileStrs, pProfileLevel[index-1].profile));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part2PLLevel(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	*value = strdup(enumToString(videoLevelStrs, pProfileLevel[index-1].level));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part2PLMaximumDecodingCapability(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[15];

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	snprintf(tmp, 10, "%f", pProfileLevel[index-1].max_decode);
	*value = strdup(tmp);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG4Part2 Object            */
/*********************************************************************/
TRX_STATUS getCapVideoDecoderMPEG4Part2AudioStandards(char **value)
{
	*value = strdup("MP3-Surround, MPEG2-AAC-LC, MPEG4-AAC-LC");
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG4Part2ProfileLevelNumberOfEntries(char **value)
{
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[5];

	getProfileLevel(&pProfileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
	snprintf(tmp, 3, "%d", numEntries);
	*value = strdup(tmp);
	return TRX_OK;
}

/************************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG2Part2.ProfileLevel Object  */
/************************************************************************/
TRX_STATUS setCapVideoDecoderMPEG2Part2PLAlias(char *value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	if ( value == NULL ) return TRX_ERR;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	strcpy(pProfileLevel[index-1].alias, value);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG2Part2PLAlias(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	*value = strdup(pProfileLevel[index-1].alias);
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG2Part2PLProfile(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	*value = strdup(enumToString(videoProfileStrs, pProfileLevel[index-1].profile));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG2Part2PLLevel(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	*value = strdup(enumToString(videoLevelStrs, pProfileLevel[index-1].level));
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG2Part2PLMaximumDecodingCapability(char **value)
{
	InstanceDesc *idp;
	uint32 index;
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[15];

	idp = getCurrentInstanceDesc();
	index = idp->instanceID;
	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	snprintf(tmp, 10, "%f", pProfileLevel[index-1].max_decode);
	*value = strdup(tmp);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder.MPEG2Part2 Object            */
/*********************************************************************/
TRX_STATUS getCapVideoDecoderMPEG2Part2AudioStandards(char **value)
{
	*value = strdup("MP3-Surround, MPEG2-AAC-LC, MPEG2-AAC-LC");
	return TRX_OK;
}

TRX_STATUS getCapVideoDecoderMPEG2Part2ProfileLevelNumberOfEntries(char **value)
{
	profileLevel *pProfileLevel;
	uint8_t numEntries;
	char tmp[5];

	getProfileLevel(&pProfileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
	snprintf(tmp, 3, "%d", numEntries);
	*value = strdup(tmp);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.VideoDecoder Object                       */
/*********************************************************************/
TRX_STATUS getCapVideoDecoderVideoStandards(char **value)
{
	char videoCodecs[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;videoCodecStandard[j].name;j++) {
		strncat(videoCodecs, videoCodecStandard[j].name, strlen(videoCodecStandard[j].name));
		if (videoCodecStandard[j+1].name){
			strncat(videoCodecs, sep, strlen(sep));
		}
	}

	*value = strdup(videoCodecs);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.AudioDecoder Object                       */
/*********************************************************************/
TRX_STATUS getCapAudioDecoderAudioStandards(char **value)
{
	char audioCodecs[1024] = "\0";
	char *sep = ", ";
	int j;

	for (j=0;audioCodecStandard[j].name;j++) {
		strncat(audioCodecs, audioCodecStandard[j].name, strlen(audioCodecStandard[j].name));
		if (audioCodecStandard[j+1].name){
			strncat(audioCodecs, sep, strlen(sep));
		}
	}

	*value = strdup(audioCodecs);
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.PVR Object                                */
/*********************************************************************/
TRX_STATUS getCapPVRMaxIOStreams(char **value)
{
	NEXUS_TransportCapabilities capabilities;
	uint16_t numRecPumps = 0;
	uint16_t numPlayPumps = 0;

    NEXUS_GetTransportCapabilities(&capabilities);
	numRecPumps = capabilities.numRecpumps;
	numPlayPumps = capabilities.numPlaypumps;

	*value = strdup(itoa(numRecPumps + numPlayPumps));
	return TRX_OK;
}

TRX_STATUS getCapPVRMaxRecordingStreams(char **value)
{
	NEXUS_TransportCapabilities capabilities;
	uint16_t numRecPumps = 0;

    NEXUS_GetTransportCapabilities(&capabilities);
	numRecPumps = capabilities.numRecpumps;

	*value = strdup(itoa(numRecPumps));
	return TRX_OK;
}

TRX_STATUS getCapPVRMaxPlaybackStreams(char **value)
{
	NEXUS_TransportCapabilities capabilities;
	uint16_t numPlayPumps = 0;

    NEXUS_GetTransportCapabilities(&capabilities);
	numPlayPumps = capabilities.numPlaypumps;

	*value = strdup(itoa(numPlayPumps));
	return TRX_OK;
}

TRX_STATUS getCapPVRMaxTimeDelay(char **value)
{
	int max_time_delay=1;
	*value = strdup(itoa(max_time_delay));
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.FrontEnd.IP Object                        */
/*********************************************************************/
TRX_STATUS getCapFrontEndIPMaxActiveIPStreams(char **value)
{
	int max_active_ip_streams=1;
	*value = strdup(itoa(max_active_ip_streams));
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPMaxActiveInboundIPStreams(char **value)
{
	int max_active_inbound_ip_streams=1;
	*value = strdup(itoa(max_active_inbound_ip_streams));
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPMaxActiveOutboundIPStreams(char **value)
{
	int max_active_outbound_ip_streams=1;
	*value = strdup(itoa(max_active_outbound_ip_streams));
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPStreamingControlProtocols(char **value)
{
	*value = strdup("IGMP");
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPStreamingTransportProtocols(char **value)
{
	*value = strdup("TCP");
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPStreamingTransportControlProtocols(char **value)
{
	*value = strdup("RTCP");
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPDownloadTransportProtocols(char **value)
{
	*value = strdup("HTTP");
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPMultiplexTypes(char **value)
{
	*value = strdup("MPEG2-PS");
	return TRX_OK;
}

TRX_STATUS getCapFrontEndIPMaxDejitteringBufferSize(char **value)
{
	int max_buf_size=1;
	*value = strdup(itoa(max_buf_size));
	return TRX_OK;
}

/*********************************************************************/
/* STBService.Capabilities.FrontEnd.DVBT Object                        */
/*********************************************************************/
TRX_STATUS getCapFrontEndDVBTMaxActiveDVBTStreams(char **value)
{
	int max_active_dvbt_streams=1;
	*value = strdup(itoa(max_active_dvbt_streams));
	return TRX_OK;
}

TRX_STATUS getCapFrontEndDVBTMaxLogicalChannels(char **value)
{
	int max_logical_channels=1;
	*value = strdup(itoa(max_logical_channels));
	return TRX_OK;
}

/*********************************************************************/
/* STBService.CapabilitiesFrontEnd.QAM Object					                                  */
/*********************************************************************/

TRX_STATUS getCapFrontEndQAMMaxActiveQAMStreams(char **value)
{
	int max_active_QAM_streams = 1;
	*value = strdup(itoa(max_active_QAM_streams));
	return TRX_OK;
}
TRX_STATUS getCapFrontEndQAMMaxLogicalChannels(char **value)
{
	int max_logical_channels = 1;
	*value = strdup(itoa(max_logical_channels));
	return TRX_OK;
}


/*********************************************************************/
/* STBService.Capabilities Object                                    */
/*********************************************************************/
TRX_STATUS getCapMaxActiveAVStreams(char **value)
{
	int max_active_av_streams=-1;
	*value = strdup(itoa(max_active_av_streams));
	return TRX_OK;
}

TRX_STATUS getCapMaxActiveAVPlayers(char **value)
{
	int max_active_av_players=-1;
	*value = strdup(itoa(max_active_av_players));
	return TRX_OK;
}
