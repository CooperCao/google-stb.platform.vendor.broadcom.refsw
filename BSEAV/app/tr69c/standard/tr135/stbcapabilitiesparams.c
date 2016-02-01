/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "sharedparams.h"
#include "stbcapabilitiesparams.h" /* profiles for parameter callbacks */

/* STBService.Capabilities.AudienceStats Object */
TRXGFUNC(getCapASMaxAudienceStatsChannels);

/* STBService.Capabilities.ServiceMonitoring Object */
TRXGFUNC(getCapSMServiceTypes);
TRXGFUNC(getCapSMMaxEventsPerSampleInterval);
TRXGFUNC(getCapSMMaxActiveMainStreams);
TRXGFUNC(getCapSMMinSampleInterval);
TRXGFUNC(getCapSMMaxReportSamples);
TRXGFUNC(getCapSMHighLevelMetricNames);

/* STBService.Capabilities.DRM Object */
TRXGFUNC(getCapDRMDRMSystems);

/* STBService.Capabilities.CA Object */
TRXGFUNC(getCapCACASystems);

/* STBService.Capabilities.CDS Object */
TRXGFUNC(getCapCDSPushCapable);
TRXGFUNC(getCapCDSPullCapable);

/* STBService.Capabilities.HDMI Object */
TRXGFUNC(getCapHDMISupportedResolutions);
TRXGFUNC(getCapHDMICECSupport);
TRXGFUNC(getCapHDMIHDMI3D);

/* STBService.Capabilities.VideoOutput Object */
TRXGFUNC(getCapVideoOutputCompositeVideoStandards);
TRXGFUNC(getCapVideoOutputVideoFormats);
TRXGFUNC(getCapVideoOutputMacrovision);
TRXGFUNC(getCapVideoOutputHDCP);
TRXGFUNC(getCapVideoOutputDislayFormats);

/* STBService.Capabilities.AudioOutput Object */
TRXGFUNC(getCapAudioOutputAudioFormats);

/* STBService.Capabilities.VideoDecoder.SMPTEVC1.ProfileLevel Object */
TRXSFUNC(setCapVideoDecoderSMPTEVC1PLAlias);
TRXGFUNC(getCapVideoDecoderSMPTEVC1PLAlias);
TRXGFUNC(getCapVideoDecoderSMPTEVC1PLProfile);
TRXGFUNC(getCapVideoDecoderSMPTEVC1PLLevel);
TRXGFUNC(getCapVideoDecoderSMPTEVC1PLMaximumDecodingCapability);

/* STBService.Capabilities.VideoDecoder.SMPTEVC1 Object */
TRXGFUNC(getCapVideoDecoderSMPTEVC1AudioStandards);
TRXGFUNC(getCapVideoDecoderSMPTEVC1ProfileLevelNumberOfEntries);

/* STBService.Capabilities.VideoDecoder.MPEG4Part10.ProfileLevel Object */
TRXSFUNC(setCapVideoDecoderMPEG4Part10PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG4Part10PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG4Part10PLProfile);
TRXGFUNC(getCapVideoDecoderMPEG4Part10PLLevel);
TRXGFUNC(getCapVideoDecoderMPEG4Part10PLMaximumDecodingCapability);

/* STBService.Capabilities.VideoDecoder.MPEG4Part10 Object */
TRXGFUNC(getCapVideoDecoderMPEG4Part10AudioStandards);
TRXGFUNC(getCapVideoDecoderMPEG4Part10ProfileLevelNumberOfEntries);

/* STBService.Capabilities.VideoDecoder.MPEG4Part2.ProfileLevel Object */
TRXSFUNC(setCapVideoDecoderMPEG4Part2PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG4Part2PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG4Part2PLProfile);
TRXGFUNC(getCapVideoDecoderMPEG4Part2PLLevel);
TRXGFUNC(getCapVideoDecoderMPEG4Part2PLMaximumDecodingCapability);

/* STBService.Capabilities.VideoDecoder.MPEG4Part2 Object */
TRXGFUNC(getCapVideoDecoderMPEG4Part2AudioStandards);
TRXGFUNC(getCapVideoDecoderMPEG4Part2ProfileLevelNumberOfEntries);


/* STBService.Capabilities.VideoDecoder.MPEG2Part2.ProfileLevel Object */
TRXSFUNC(setCapVideoDecoderMPEG2Part2PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG2Part2PLAlias);
TRXGFUNC(getCapVideoDecoderMPEG2Part2PLProfile);
TRXGFUNC(getCapVideoDecoderMPEG2Part2PLLevel);
TRXGFUNC(getCapVideoDecoderMPEG2Part2PLMaximumDecodingCapability);

/* STBService.Capabilities.VideoDecoder.MPEG2Part2 Object */
TRXGFUNC(getCapVideoDecoderMPEG2Part2AudioStandards);
TRXGFUNC(getCapVideoDecoderMPEG2Part2ProfileLevelNumberOfEntries);

/* STBService.Capabilities.VideoDecoder Object */
TRXGFUNC(getCapVideoDecoderVideoStandards);

/* STBService.Capabilities.AudioDecoder Object */
TRXGFUNC(getCapAudioDecoderAudioStandards);

/* STBService.Capabilities.PVR Object */
TRXGFUNC(getCapPVRMaxIOStreams);
TRXGFUNC(getCapPVRMaxRecordingStreams);
TRXGFUNC(getCapPVRMaxPlaybackStreams);
TRXGFUNC(getCapPVRMaxTimeDelay);

/* STBService.Capabilities.FrontEnd.IP Object */
TRXGFUNC(getCapFrontEndIPMaxActiveIPStreams);
TRXGFUNC(getCapFrontEndIPMaxActiveInboundIPStreams);
TRXGFUNC(getCapFrontEndIPMaxActiveOutboundIPStreams);
TRXGFUNC(getCapFrontEndIPStreamingControlProtocols);
TRXGFUNC(getCapFrontEndIPStreamingTransportProtocols);
TRXGFUNC(getCapFrontEndIPStreamingTransportControlProtocols);
TRXGFUNC(getCapFrontEndIPDownloadTransportProtocols);
TRXGFUNC(getCapFrontEndIPMultiplexTypes);
TRXGFUNC(getCapFrontEndIPMaxDejitteringBufferSize);

/* STBService.Capabilities.FrontEnd.DVBT Object */
TRXGFUNC(getCapFrontEndDVBTMaxActiveDVBTStreams);
TRXGFUNC(getCapFrontEndDVBTMaxLogicalChannels);

/* STBService.Capabilities Object */
TRXGFUNC(getCapMaxActiveAVStreams);
TRXGFUNC(getCapMaxActiveAVPlayers);

TRXGFUNC(getCapFrontEndQAMMaxActiveQAMStreams);
TRXGFUNC(getCapFrontEndQAMMaxLogicalChannels);

TRxObjNode CapAudienceStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxAudienceStatsChannels, {{tInt, 0, 0}}, NULL, getCapASMaxAudienceStatsChannels, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxAudienceStatsChannels, {{tInt, 0, 0}}, NULL, getCapASMaxAudienceStatsChannels, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapServiceMonitoringDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ServiceTypes, {{tString, 64, 0}}, NULL, getCapSMServiceTypes, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxEventsPerSampleInterval, {{tInt, 0 ,0}}, NULL, getCapSMMaxEventsPerSampleInterval, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxActiveMainStreams, {{tInt, 0, 0}}, NULL, getCapSMMaxActiveMainStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MinSampleInterval, {{tInt, 0, 0}}, NULL, getCapSMMinSampleInterval, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxReportSamples, {{tInt, 0, 0}}, NULL, getCapSMMaxReportSamples, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HighLevelMetricNames, {{tString, 1024, 0}}, NULL, getCapSMHighLevelMetricNames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {ServiceTypes, {{tString, 64, 0}}, NULL, getCapSMServiceTypes, NULL, NULL},
	{MaxEventsPerSampleInterval, {{tInt, 0 ,0}}, NULL, getCapSMMaxEventsPerSampleInterval, NULL, NULL},
    {MaxActiveMainStreams, {{tInt, 0, 0}}, NULL, getCapSMMaxActiveMainStreams, NULL, NULL},
    {MinSampleInterval, {{tInt, 0, 0}}, NULL, getCapSMMinSampleInterval, NULL, NULL},
    {MaxReportSamples, {{tInt, 0, 0}}, NULL, getCapSMMaxReportSamples, NULL, NULL},
    {HighLevelMetricNames, {{tString, 1024, 0}}, NULL, getCapSMHighLevelMetricNames, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapDRMDesc[] = {
#ifdef XML_DOC_SUPPORT
    {DRMSystems, {{tString, 1024, 0}}, NULL, getCapDRMDRMSystems, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {DRMSystems, {{tString, 1024, 0}}, NULL, getCapDRMDRMSystems, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapCADesc[] = {
#ifdef XML_DOC_SUPPORT
    {CASystems, {{tString, 1024, 0}}, NULL, getCapCACASystems, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {CASystems, {{tString, 1024, 0}}, NULL, getCapCACASystems, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapCDSDesc[] = {
#ifdef XML_DOC_SUPPORT
    {PushCapable, {{tBool, 0, 0}}, NULL, getCapCDSPushCapable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PullCapable, {{tBool, 0, 0}}, NULL, getCapCDSPullCapable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {PushCapable, {{tBool, 0, 0}}, NULL, getCapCDSPushCapable, NULL, NULL},
	{PullCapable, {{tBool, 0, 0}}, NULL, getCapCDSPullCapable, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapHDMIDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SupportedResolutions, {{tString, 32, 0}}, NULL, getCapHDMISupportedResolutions, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {CECSupport, {{tBool, 0, 0}}, NULL, getCapHDMICECSupport, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {HDMI3D, {{tBool, 0, 0}}, NULL, getCapHDMIHDMI3D, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {SupportedResolutions, {{tString, 32, 0}}, NULL, getCapHDMISupportedResolutions, NULL, NULL},
	{CECSupport, {{tBool, 0, 0}}, NULL, getCapHDMICECSupport, NULL, NULL},
	{HDMI3D, {{tBool, 0, 0}}, NULL, getCapHDMIHDMI3D, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoOutputDesc[] = {
#ifdef XML_DOC_SUPPORT
    {CompositeVideoStandards, {{tString, 32, 0}}, NULL, getCapVideoOutputCompositeVideoStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoFormats, {{tString, 32, 0}}, NULL, getCapVideoOutputVideoFormats, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Macrovision, {{tString, 32, 0}}, NULL, getCapVideoOutputMacrovision, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HDCP, {{tString, 32, 0}}, NULL, getCapVideoOutputHDCP, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DisplayFormats, {{tString, 32, 0}}, NULL, getCapVideoOutputDislayFormats, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {CompositeVideoStandards, {{tString, 32, 0}}, NULL, getCapVideoOutputCompositeVideoStandards, NULL, NULL},
    {VideoFormats, {{tString, 32, 0}}, NULL, getCapVideoOutputVideoFormats, NULL, NULL},
    {Macrovision, {{tString, 32, 0}}, NULL, getCapVideoOutputMacrovision, NULL, NULL},
    {HDCP, {{tString, 32, 0}}, NULL, getCapVideoOutputHDCP, NULL, NULL},
	{DisplayFormats, {{tString, 32, 0}}, NULL, getCapVideoOutputDislayFormats, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapAudioOutputDesc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioFormats, {{tString, 32, 0}}, NULL, getCapAudioOutputAudioFormats, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioFormats, {{tString, 32, 0}}, NULL, getCapAudioOutputAudioFormats, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderSMPTEVC1ProfileLevelDesc[] = {
#ifdef XML_DOC_SUPPORT
	{Alias, {{tString, 64, 0}}, setCapVideoDecoderSMPTEVC1PLAlias, getCapVideoDecoderSMPTEVC1PLAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLProfile, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLMaximumDecodingCapability, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setCapVideoDecoderSMPTEVC1PLAlias, getCapVideoDecoderSMPTEVC1PLAlias, NULL, NULL},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLProfile, NULL, NULL},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLLevel, NULL, NULL},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderSMPTEVC1PLMaximumDecodingCapability, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderSMPTEVC1ProfileLevelInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1ProfileLevelDesc, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1ProfileLevelDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderSMPTEVC1Desc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1AudioStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderSMPTEVC1ProfileLevelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1ProfileLevelInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderSMPTEVC1AudioStandards, NULL, NULL},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderSMPTEVC1ProfileLevelNumberOfEntries, NULL, NULL},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1ProfileLevelInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part10ProfileLevelDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG4Part10PLAlias, getCapVideoDecoderMPEG4Part10PLAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Profile, {{tString,32, 0}}, NULL, getCapVideoDecoderMPEG4Part10PLProfile, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part10PLLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL,getCapVideoDecoderMPEG4Part10PLMaximumDecodingCapability, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG4Part10PLAlias, getCapVideoDecoderMPEG4Part10PLAlias, NULL, NULL},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part10PLProfile, NULL, NULL},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part10PLLevel, NULL, NULL},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL,getCapVideoDecoderMPEG4Part10PLMaximumDecodingCapability, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part10ProfileLevelInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part10ProfileLevelDesc, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part10ProfileLevelDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part10Desc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part10AudioStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part10ProfileLevelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL,NULL, CapVideoDecoderMPEG4Part10ProfileLevelInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part10AudioStandards, NULL, NULL},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part10ProfileLevelNumberOfEntries, NULL, NULL},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL,NULL, CapVideoDecoderMPEG4Part10ProfileLevelInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part2ProfileLevelDesc[] = {
#ifdef XML_DOC_SUPPORT
	{Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG4Part2PLAlias, getCapVideoDecoderMPEG4Part2PLAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLProfile, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLMaximumDecodingCapability, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG4Part2PLAlias, getCapVideoDecoderMPEG4Part2PLAlias, NULL, NULL},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLProfile, NULL, NULL},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLLevel, NULL, NULL},
    {MaximumDecodingCapability, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part2PLMaximumDecodingCapability, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part2ProfileLevelInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2ProfileLevelDesc, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2ProfileLevelDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG4Part2Desc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioStandards, {{tString,32,0}}, NULL, getCapVideoDecoderMPEG4Part2AudioStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part2ProfileLevelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevel, {{tInstance,0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2ProfileLevelInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioStandards, {{tString,32,0}}, NULL, getCapVideoDecoderMPEG4Part2AudioStandards, NULL, NULL},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG4Part2ProfileLevelNumberOfEntries, NULL, NULL},
    {ProfileLevel, {{tInstance,0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2ProfileLevelInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG2Part2ProfileLevelDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG2Part2PLAlias, getCapVideoDecoderMPEG2Part2PLAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLProfile, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaximumDecodingCapability,{{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLMaximumDecodingCapability, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setCapVideoDecoderMPEG2Part2PLAlias, getCapVideoDecoderMPEG2Part2PLAlias, NULL, NULL},
    {Profile, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLProfile, NULL, NULL},
    {Level, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLLevel, NULL, NULL},
    {MaximumDecodingCapability,{{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG2Part2PLMaximumDecodingCapability, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG2Part2ProfileLevelInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
	{instanceIDMASK,{{0,0,0}}, NULL, NULL, CapVideoDecoderMPEG2Part2ProfileLevelDesc, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0,0,0}}, NULL, NULL, CapVideoDecoderMPEG2Part2ProfileLevelDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderMPEG2Part2Desc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2AudioStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG2Part2ProfileLevelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG2Part2ProfileLevelInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderMPEG2Part2AudioStandards, NULL, NULL},
    {ProfileLevelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCapVideoDecoderMPEG2Part2ProfileLevelNumberOfEntries, NULL, NULL},
    {ProfileLevel, {{tInstance, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG2Part2ProfileLevelInstanceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapVideoDecoderDesc[] = {
#ifdef XML_DOC_SUPPORT
    {VideoStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderVideoStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG2Part2, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG2Part2Desc, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG4Part2, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2Desc, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG4Part10, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part10Desc, NULL, 1, 0, 0, 0, NULL, true},
    {SMPTEVC1, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1Desc, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {VideoStandards, {{tString, 32, 0}}, NULL, getCapVideoDecoderVideoStandards, NULL, NULL},
    {MPEG2Part2, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG2Part2Desc, NULL},
    {MPEG4Part2, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part2Desc, NULL},
    {MPEG4Part10, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderMPEG4Part10Desc, NULL},
    {SMPTEVC1, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderSMPTEVC1Desc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapAudioDecoderDesc[] = {
#ifdef XML_DOC_SUPPORT
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapAudioDecoderAudioStandards, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {AudioStandards, {{tString, 32, 0}}, NULL, getCapAudioDecoderAudioStandards, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapPVRDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxIOStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxIOStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxRecordingStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxRecordingStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxPlaybackStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxPlaybackStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxTimeDelay, {{tInt, 0, 0}}, NULL, getCapPVRMaxTimeDelay, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxIOStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxIOStreams, NULL, NULL},
    {MaxRecordingStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxRecordingStreams, NULL, NULL},
    {MaxPlaybackStreams, {{tInt, 0, 0}}, NULL, getCapPVRMaxPlaybackStreams, NULL, NULL},
    {MaxTimeDelay, {{tInt, 0, 0}}, NULL, getCapPVRMaxTimeDelay, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapFrontEndIPDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxActiveIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxActiveInboundIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveInboundIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxActiveOutboundIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveOutboundIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {StreamingControlProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingControlProtocols, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {StreamingTransportProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingTransportProtocols, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {StreamingTransportControlProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingTransportControlProtocols, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DownloadTransportProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPDownloadTransportProtocols, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MultiplexTypes, {{tString, 32, 0}}, NULL, getCapFrontEndIPMultiplexTypes, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxDejitteringBufferSize, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxDejitteringBufferSize, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxActiveIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveIPStreams, NULL, NULL},
    {MaxActiveInboundIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveInboundIPStreams, NULL, NULL},
    {MaxActiveOutboundIPStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxActiveOutboundIPStreams, NULL, NULL},
    {StreamingControlProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingControlProtocols, NULL, NULL},
    {StreamingTransportProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingTransportProtocols, NULL, NULL},
    {StreamingTransportControlProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPStreamingTransportControlProtocols, NULL, NULL},
    {DownloadTransportProtocols, {{tString, 32, 0}}, NULL, getCapFrontEndIPDownloadTransportProtocols, NULL, NULL},
    {MultiplexTypes, {{tString, 32, 0}}, NULL, getCapFrontEndIPMultiplexTypes, NULL, NULL},
    {MaxDejitteringBufferSize, {{tInt, 0, 0}}, NULL, getCapFrontEndIPMaxDejitteringBufferSize, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapFrontEndDVBTDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxActiveDVBTStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndDVBTMaxActiveDVBTStreams, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaxLogicalChannels, {{tInt, 0, 0}}, NULL, getCapFrontEndDVBTMaxLogicalChannels, NULL, NULL, 1, 0, 0, 0, NULL, false},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxActiveDVBTStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndDVBTMaxActiveDVBTStreams, NULL, NULL},
    {MaxLogicalChannels, {{tInt, 0, 0}}, NULL, getCapFrontEndDVBTMaxLogicalChannels, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapFrontEndQAMDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxActiveQAMStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndQAMMaxActiveQAMStreams, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaxLogicalQAMChannels, {{tInt, 0, 0}}, NULL, getCapFrontEndQAMMaxLogicalChannels, NULL, NULL, 1, 0, 0, 0, NULL, false},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxActiveQAMStreams, {{tInt, 0, 0}}, NULL, getCapFrontEndQAMMaxActiveQAMStreams, NULL, NULL},
    {MaxLogicalQAMChannels, {{tInt, 0, 0}}, NULL, getCapFrontEndQAMMaxLogicalChannels, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};
TRxObjNode CapFrontEndDesc[] = {
#ifdef XML_DOC_SUPPORT
    {DVBT, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndDVBTDesc, NULL, 1, 0, 0, 0, NULL, false},
    {IP, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndIPDesc, NULL, 1, 0, 0, 0, NULL, true},
    {X_BROADCOM_COM_QAM, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndQAMDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {DVBT, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndDVBTDesc, NULL},
    {IP, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndIPDesc, NULL},
    {X_BROADCOM_COM_QAM, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndQAMDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CapabilitiesDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MaxActiveAVStreams, {{tInt, 0, 0}}, NULL, getCapMaxActiveAVStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MaxActiveAVPlayers, {{tInt, 0, 0}}, NULL, getCapMaxActiveAVPlayers, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FrontEnd, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndDesc, NULL, 1, 0, 0, 0, NULL, true},
    {PVR, {{tObject, 0, 0}}, NULL, NULL, CapPVRDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudioDecoder, {{tObject, 0, 0}}, NULL, NULL, CapAudioDecoderDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoDecoder, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudioOutput, {{tObject, 0, 0}}, NULL, NULL, CapAudioOutputDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoOutput, {{tObject, 0, 0}}, NULL, NULL, CapVideoOutputDesc, NULL, 1, 0, 0, 0, NULL, true},
	{HDMI, {{tObject, 0, 0}}, NULL, NULL, CapHDMIDesc, NULL, 1, 0, 0, 0, NULL, true},
	{CDS, {{tObject, 0, 0}}, NULL, NULL, CapCDSDesc, NULL, 1, 0, 0, 0, NULL, false},
	{CA, {{tObject, 0, 0}}, NULL, NULL, CapCADesc, NULL, 1, 0, 0, 0, NULL, false},
    {DRM, {{tObject, 0, 0}}, NULL, NULL, CapDRMDesc, NULL, 1, 0, 0, 0, NULL, false},
    {ServiceMonitoring, {{tObject, 0, 0}}, NULL, NULL, CapServiceMonitoringDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudienceStats, {{tObject, 0, 0}}, NULL, NULL, CapAudienceStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {MaxActiveAVStreams, {{tInt, 0, 0}}, NULL, getCapMaxActiveAVStreams, NULL, NULL},
    {MaxActiveAVPlayers, {{tInt, 0, 0}}, NULL, getCapMaxActiveAVPlayers, NULL, NULL},
    {FrontEnd, {{tObject, 0, 0}}, NULL, NULL, CapFrontEndDesc, NULL},
    {PVR, {{tObject, 0, 0}}, NULL, NULL, CapPVRDesc, NULL},
    {AudioDecoder, {{tObject, 0, 0}}, NULL, NULL, CapAudioDecoderDesc, NULL},
    {VideoDecoder, {{tObject, 0, 0}}, NULL, NULL, CapVideoDecoderDesc, NULL},
    {AudioOutput, {{tObject, 0, 0}}, NULL, NULL, CapAudioOutputDesc, NULL},
    {VideoOutput, {{tObject, 0, 0}}, NULL, NULL, CapVideoOutputDesc, NULL},
    {HDMI, {{tObject, 0, 0}}, NULL, NULL, CapHDMIDesc, NULL},
    {CDS, {{tObject, 0, 0}}, NULL, NULL, CapCDSDesc, NULL},
    {CA, {{tObject, 0, 0}}, NULL, NULL, CapCADesc, NULL},
    {DRM, {{tObject, 0, 0}}, NULL, NULL, CapDRMDesc, NULL},
    {ServiceMonitoring, {{tObject, 0, 0}}, NULL, NULL, CapServiceMonitoringDesc, NULL},
    {AudienceStats, {{tObject, 0, 0}}, NULL, NULL, CapAudienceStatsDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};


