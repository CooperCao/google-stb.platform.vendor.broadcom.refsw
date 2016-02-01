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
#include "stbmiscparams.h" /* profiles for parameter callbacks */

extern TRxObjNode CapabilitiesDesc[];
extern TRxObjNode ComponentsDesc[];
extern TRxObjNode AVStreamsDesc[];
extern TRxObjNode AVPlayersDesc[];
extern TRxObjNode ServiceMonitoringDesc[];
extern TRxObjNode ApplicationsDesc[];

/* STBService.AVStreams.AVStream Object */
TRXGFUNC(getAVStreamsAVStreamStatus);
TRXSFUNC(setAVStreamsAVStreamAlias);
TRXGFUNC(getAVStreamsAVStreamAlias);
TRXGFUNC(getAVStreamsAVStreamName);
TRXGFUNC(getAVStreamsAVStreamPVRState);
TRXGFUNC(getAVStreamsAVStreamFrontEnd);
TRXGFUNC(getAVStreamsAVStreamInbound);
TRXGFUNC(getAVStreamsAVStreamOutbound);
TRXGFUNC(getAVStreamsAVStreamAudioDecoder);
TRXGFUNC(getAVStreamsAVStreamVideoDecoder);
TRXGFUNC(getAVStreamsAVStreamCA);
TRXGFUNC(getAVStreamsAVStreamDRM);
TRXSFUNC(setSTBServiceAlias);
TRXGFUNC(getSTBServiceAlias);
TRXSFUNC(setSTBServiceEnable);
TRXGFUNC(getSTBServiceEnable);

/* STBService.AVStreams Object */
TRXGFUNC(getAVStreamsActiveAVStreams);
TRXGFUNC(getAVStreamsAVStreamNumberOfEntries);

/* STBService.AVPlayers.AVPLayer Object */
TRXSFUNC(setAVPlayersAVPlayerEnable);
TRXGFUNC(getAVPlayersAVPlayerEnable);
TRXGFUNC(getAVPlayersAVPlayerStatus);
TRXSFUNC(setAVPlayersAVPlayerAlias);
TRXGFUNC(getAVPlayersAVPlayerAlias);
TRXGFUNC(getAVPlayersAVPlayerName);
TRXGFUNC(getAVPlayersAVPlayerAudioLanguage);
TRXGFUNC(getAVPlayersAVPlayerSubtitlingStatus);
TRXGFUNC(getAVPlayersAVPlayerSubtitlingLanguage);
TRXGFUNC(getAVPlayersAVPlayerAudioOutputs);
TRXGFUNC(getAVPlayersAVPlayerVideoOutputs);
TRXGFUNC(getAVPlayersAVPlayerMainStream);
TRXGFUNC(getAVPlayersAVPlayerPIPStreams);

/* STBService.AVPlayers Object */
TRXGFUNC(getAVPlayersActiveAVPlayers);
TRXGFUNC(getAVPlayersAVPlayerNumberOfEntries);
TRXSFUNC(setAVPlayersPreferredAudioLanguage);
TRXGFUNC(getAVPlayersPreferredAudioLanguage);
TRXSFUNC(setAVPlayersPreferredSubtitlingLanguage);
TRXGFUNC(getAVPlayersPreferredSubtitlingLanguage);
TRXSFUNC(setAVPlayersPreferredBehaviour);
TRXGFUNC(getAVPlayersPreferredBehaviour);
TRXSFUNC(setAVPlayersResetPINCode);
TRXGFUNC(getAVPlayersResetPINCode);

/* STBService.Applications.CDSPull.ContentItem Object */
TRXSFUNC(setApplicationsCDSPullContentItemAlias);
TRXGFUNC(getApplicationsCDSPullContentItemAlias);
TRXGFUNC(getApplicationsCDSPullContentItemContentReferenceId);
TRXGFUNC(getApplicationsCDSPullContentItemVersionNumber);
TRXSFUNC(setApplicationsCDSPullContentItemDeleteItem);
TRXGFUNC(getApplicationsCDSPullContentItemDeleteItem);

/* STBService.Applications.CDSPull Object */
TRXGFUNC(getApplicationsCDSPullReference);
TRXGFUNC(getApplicationsCDSPullContentItemNumberOfEntries);

/* STBService.Applications.CDSPush.ContentItem Object */
TRXSFUNC(setApplicationsCDSPushContentItemAlias);
TRXGFUNC(getApplicationsCDSPushContentItemAlias);
TRXGFUNC(getApplicationsCDSPushContentItemContentReferenceId);
TRXGFUNC(getApplicationsCDSPushContentItemVersionNumber);
TRXSFUNC(setApplicationsCDSPushContentItemDeleteItem);
TRXGFUNC(getApplicationsCDSPushContentItemDeleteItem);

/* STBService.Applications.CDSPush Object */
TRXGFUNC(getApplicationsCDSPushReference);
TRXGFUNC(getApplicationsCDSPushContentItemNumberOfEntries);

/* STBService.Applications.AudienceStats.Channel Object */
TRXSFUNC(setApplicationsAudienceStatsChannelAlias);
TRXGFUNC(getApplicationsAudienceStatsChannelAlias);
TRXGFUNC(getApplicationsAudienceStatsChannelName);
TRXGFUNC(getApplicationsAudienceStatsChannelDuration);

/* STBService.Applications.AudienceStats Object */
TRXSFUNC(setApplicationsAudienceStatsEnable);
TRXGFUNC(getApplicationsAudienceStatsEnable);
TRXSFUNC(setApplicationsAudienceStatsReset);
TRXGFUNC(getApplicationsAudienceStatsReset);
TRXGFUNC(getApplicationsAudienceStatsResetTime);
TRXGFUNC(getApplicationsAudienceStatsChannelNumberOfEntries);

/* STBService.Applications.ServiceProvider Object */
TRXSFUNC(setApplicationsServiceProviderAlias);
TRXGFUNC(getApplicationsServiceProviderAlias);
TRXSFUNC(setApplicationsServiceProviderName);
TRXGFUNC(getApplicationsServiceProviderName);
TRXSFUNC(setApplicationsServiceProviderDomain);
TRXGFUNC(getApplicationsServiceProviderDomain);
TRXSFUNC(setApplicationsServiceProviderServiceDiscoveryServer);
TRXGFUNC(getApplicationsServiceProviderServiceDiscoveryServer);
TRXGFUNC(getApplicationsServiceProviderActiveBCGServers);

/* STBService.Applications Object */
TRXGFUNC(getApplicationsServiceProviderNumberOfEntries);

TRxObjNode AVStreamsAVStreamDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Status, {{tString, 32, 0}}, NULL, getAVStreamsAVStreamStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setAVStreamsAVStreamAlias, getAVStreamsAVStreamAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PVRState, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamPVRState, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FrontEnd, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamFrontEnd, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Inbound, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamInbound, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Outbound, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamOutbound, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AudioDecoder, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamAudioDecoder, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoDecoder, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamVideoDecoder, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CA, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamCA, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DRM, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamDRM, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Status, {{tString, 32, 0}}, NULL, getAVStreamsAVStreamStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setAVStreamsAVStreamAlias, getAVStreamsAVStreamAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamName, NULL, NULL},
    {PVRState, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamPVRState, NULL, NULL},
    {FrontEnd, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamFrontEnd, NULL, NULL},
    {Inbound, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamInbound, NULL, NULL},
    {Outbound, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamOutbound, NULL, NULL},
    {AudioDecoder, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamAudioDecoder, NULL, NULL},
    {VideoDecoder, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamVideoDecoder, NULL, NULL},
    {CA, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamCA, NULL, NULL},
    {DRM, {{tString, 256, 0}}, NULL, getAVStreamsAVStreamDRM, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode AVStreamsAVStreamInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, AVStreamsAVStreamDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, AVStreamsAVStreamDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode AVStreamsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ActiveAVStreams, {{tUnsigned, 0, 0}}, NULL, getAVStreamsActiveAVStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AVStreamNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getAVStreamsAVStreamNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AVStream, {{tInstance, 0, 0}}, NULL,NULL, AVStreamsAVStreamInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {ActiveAVStreams, {{tUnsigned, 0, 0}}, NULL, getAVStreamsActiveAVStreams, NULL, NULL},
    {AVStreamNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getAVStreamsAVStreamNumberOfEntries, NULL, NULL},
    {AVStream, {{tInstance, 0, 0}}, NULL,NULL, AVStreamsAVStreamInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode AVPlayersAVPlayerDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setAVPlayersAVPlayerEnable, getAVPlayersAVPlayerEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getAVPlayersAVPlayerStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setAVPlayersAVPlayerAlias, getAVPlayersAVPlayerAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getAVPlayersAVPlayerName, NULL, NULL, 1, 3, 0, 0, NULL, true},
    {AudioLanguage, {{tString, 64, 0}}, NULL, getAVPlayersAVPlayerAudioLanguage, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SubtitlingStatus, {{tString, 32, 0}}, NULL, getAVPlayersAVPlayerSubtitlingStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SubtitlingLanguage, {{tString, 64, 0}}, NULL, getAVPlayersAVPlayerSubtitlingLanguage, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioOutputs, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerAudioOutputs, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoOutputs, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerVideoOutputs, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MainStream, {{tString, 256, 0}}, NULL, getAVPlayersAVPlayerMainStream, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PIPStreams, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerPIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setAVPlayersAVPlayerEnable, getAVPlayersAVPlayerEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getAVPlayersAVPlayerStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setAVPlayersAVPlayerAlias, getAVPlayersAVPlayerAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getAVPlayersAVPlayerName, NULL, NULL},
    {AudioLanguage, {{tString, 64, 0}}, NULL, getAVPlayersAVPlayerAudioLanguage, NULL, NULL},
    {SubtitlingStatus, {{tString, 32, 0}}, NULL, getAVPlayersAVPlayerSubtitlingStatus, NULL, NULL},
    {SubtitlingLanguage, {{tString, 64, 0}}, NULL, getAVPlayersAVPlayerSubtitlingLanguage, NULL, NULL},
    {AudioOutputs, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerAudioOutputs, NULL, NULL},
    {VideoOutputs, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerVideoOutputs, NULL, NULL},
    {MainStream, {{tString, 256, 0}}, NULL, getAVPlayersAVPlayerMainStream, NULL, NULL},
    {PIPStreams, {{tString, 1024, 0}}, NULL, getAVPlayersAVPlayerPIPStreams, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode AVPlayersAVPlayerInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, AVPlayersAVPlayerDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, AVPlayersAVPlayerDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode AVPlayersDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ActiveAVPlayers, {{tUnsigned, 0, 0}}, NULL, getAVPlayersActiveAVPlayers, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AVPlayerNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getAVPlayersAVPlayerNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PreferredAudioLanguage, {{tString, 64, 0}}, setAVPlayersPreferredAudioLanguage, getAVPlayersPreferredAudioLanguage, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PreferredSubtitlingLanguage, {{tString, 64, 0}}, setAVPlayersPreferredSubtitlingLanguage, getAVPlayersPreferredSubtitlingLanguage, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PreferredBehaviour, {{tString, 64, 0}}, setAVPlayersPreferredBehaviour, getAVPlayersPreferredBehaviour, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {ResetPINCode, {{tBool, 0, 0}}, setAVPlayersResetPINCode, getAVPlayersResetPINCode, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {AVPlayer, {{tInstance, 0, 0}}, NULL, NULL, AVPlayersAVPlayerInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {ActiveAVPlayers, {{tUnsigned, 0, 0}}, NULL, getAVPlayersActiveAVPlayers, NULL, NULL},
    {AVPlayerNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getAVPlayersAVPlayerNumberOfEntries, NULL, NULL},
    {PreferredAudioLanguage, {{tString, 64, 0}}, setAVPlayersPreferredAudioLanguage, getAVPlayersPreferredAudioLanguage, NULL, NULL},
    {PreferredSubtitlingLanguage, {{tString, 64, 0}}, setAVPlayersPreferredSubtitlingLanguage, getAVPlayersPreferredSubtitlingLanguage, NULL, NULL},
	{PreferredBehaviour, {{tString, 64, 0}}, setAVPlayersPreferredBehaviour, getAVPlayersPreferredBehaviour, NULL, NULL},
	{ResetPINCode, {{tBool, 0, 0}}, setAVPlayersResetPINCode, getAVPlayersResetPINCode, NULL, NULL},
    {AVPlayer, {{tInstance, 0, 0}}, NULL, NULL, AVPlayersAVPlayerInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPullContentItemDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 256, 0}}, setApplicationsCDSPullContentItemAlias, getApplicationsCDSPullContentItemAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {ContentReferenceId, {{tString, 32, 0}}, NULL, getApplicationsCDSPullContentItemContentReferenceId, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {VersionNumber, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPullContentItemVersionNumber, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {DeleteItem, {{tBool, 0, 0}}, setApplicationsCDSPullContentItemDeleteItem, getApplicationsCDSPullContentItemDeleteItem, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 256, 0}}, setApplicationsCDSPullContentItemAlias, getApplicationsCDSPullContentItemAlias, NULL, NULL},
	{ContentReferenceId, {{tString, 32, 0}}, NULL, getApplicationsCDSPullContentItemContentReferenceId, NULL, NULL},
	{VersionNumber, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPullContentItemVersionNumber, NULL, NULL},
	{DeleteItem, {{tBool, 0, 0}}, setApplicationsCDSPullContentItemDeleteItem, getApplicationsCDSPullContentItemDeleteItem, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPullContentItemInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsCDSPullContentItemDesc, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsCDSPullContentItemDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPullDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Reference, {{tString, 256, 0}}, NULL, getApplicationsCDSPullReference, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ContentItemNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPullContentItemNumberOfEntries, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ContentItem, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsCDSPullContentItemInstanceDesc, NULL, 1, 1, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Reference, {{tString, 256, 0}}, NULL, getApplicationsCDSPullReference, NULL, NULL},
	{ContentItemNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPullContentItemNumberOfEntries, NULL, NULL},
	{ContentItem, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsCDSPullContentItemInstanceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPushContentItemDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 256, 0}}, setApplicationsCDSPushContentItemAlias, getApplicationsCDSPushContentItemAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {ContentReferenceId, {{tString, 32, 0}}, NULL, getApplicationsCDSPushContentItemContentReferenceId, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {VersionNumber, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPushContentItemVersionNumber, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {DeleteItem, {{tBool, 0, 0}}, setApplicationsCDSPushContentItemDeleteItem, getApplicationsCDSPushContentItemDeleteItem, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 256, 0}}, setApplicationsCDSPushContentItemAlias, getApplicationsCDSPushContentItemAlias, NULL, NULL},
	{ContentReferenceId, {{tString, 32, 0}}, NULL, getApplicationsCDSPushContentItemContentReferenceId, NULL, NULL},
	{VersionNumber, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPushContentItemVersionNumber, NULL, NULL},
	{DeleteItem, {{tBool, 0, 0}}, setApplicationsCDSPushContentItemDeleteItem, getApplicationsCDSPushContentItemDeleteItem, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPushContentItemInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsCDSPushContentItemDesc, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsCDSPushContentItemDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsCDSPushDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Reference, {{tString, 256, 0}}, NULL, getApplicationsCDSPushReference, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ContentItemNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPushContentItemNumberOfEntries, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ContentItem, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsCDSPushContentItemInstanceDesc, NULL, 1, 1, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Reference, {{tString, 256, 0}}, NULL, getApplicationsCDSPushReference, NULL, NULL},
	{ContentItemNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsCDSPushContentItemNumberOfEntries, NULL, NULL},
	{ContentItem, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsCDSPushContentItemInstanceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsAudienceStatsChannelDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setApplicationsAudienceStatsChannelAlias, getApplicationsAudienceStatsChannelAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, NULL, getApplicationsAudienceStatsChannelName, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Duration, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsChannelDuration, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setApplicationsAudienceStatsChannelAlias, getApplicationsAudienceStatsChannelAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getApplicationsAudienceStatsChannelName, NULL, NULL},
    {Duration, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsChannelDuration, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsAudienceStatsChannelInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsChannelDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsChannelDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsAudienceStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setApplicationsAudienceStatsEnable, getApplicationsAudienceStatsEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Reset, {{tBool, 0, 0}}, setApplicationsAudienceStatsReset, getApplicationsAudienceStatsReset, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ResetTime, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsResetTime, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ChannelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsChannelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Channel, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsChannelInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setApplicationsAudienceStatsEnable, getApplicationsAudienceStatsEnable, NULL, NULL},
    {Reset, {{tBool, 0, 0}}, setApplicationsAudienceStatsReset, getApplicationsAudienceStatsReset, NULL, NULL},
    {ResetTime, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsResetTime, NULL, NULL},
    {ChannelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsAudienceStatsChannelNumberOfEntries, NULL, NULL},
    {Channel, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsChannelInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsServiceProviderDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setApplicationsServiceProviderAlias, getApplicationsServiceProviderAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, setApplicationsServiceProviderName, getApplicationsServiceProviderName, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Domain, {{tString, 256, 0}}, setApplicationsServiceProviderDomain, getApplicationsServiceProviderDomain, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ServiceDiscoveryServer, {{tString, 256, 0}}, setApplicationsServiceProviderServiceDiscoveryServer, getApplicationsServiceProviderServiceDiscoveryServer, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {ActiveBCGServers, {{tString, 1024, 0}}, NULL, getApplicationsServiceProviderActiveBCGServers, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setApplicationsServiceProviderAlias, getApplicationsServiceProviderAlias, NULL, NULL},
	{Name, {{tString, 256, 0}}, setApplicationsServiceProviderName, getApplicationsServiceProviderName, NULL, NULL},
	{Domain, {{tString, 256, 0}}, setApplicationsServiceProviderDomain, getApplicationsServiceProviderDomain, NULL, NULL},
	{ServiceDiscoveryServer, {{tString, 256, 0}}, setApplicationsServiceProviderServiceDiscoveryServer, getApplicationsServiceProviderServiceDiscoveryServer, NULL, NULL},
	{ActiveBCGServers, {{tString, 1024, 0}}, NULL, getApplicationsServiceProviderActiveBCGServers, NULL, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsServiceProviderInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsServiceProviderDesc, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ApplicationsServiceProviderDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ApplicationsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ServiceProviderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsServiceProviderNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ServiceProvider, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsServiceProviderInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {AudienceStats, {{tObject, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {CDSPush, {{tObject, 0, 0}}, NULL, NULL, ApplicationsCDSPushDesc, NULL, 1, 0, 0, 0, NULL, false},
    {CDSPull, {{tObject, 0, 0}}, NULL, NULL, ApplicationsCDSPullDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{ServiceProviderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getApplicationsServiceProviderNumberOfEntries, NULL, NULL},
	{ServiceProvider, {{tInstance, 0, 0}}, NULL, NULL, ApplicationsServiceProviderInstanceDesc, NULL},
	{AudienceStats, {{tObject, 0, 0}}, NULL, NULL, ApplicationsAudienceStatsDesc, NULL},
    {CDSPush, {{tObject, 0, 0}}, NULL, NULL, ApplicationsCDSPushDesc, NULL},
	{CDSPull, {{tObject, 0, 0}}, NULL, NULL, ApplicationsCDSPullDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode  STBServiceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setSTBServiceEnable, getSTBServiceEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setSTBServiceAlias, getSTBServiceAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Capabilities, {{tObject, 0, 0}}, NULL, NULL, CapabilitiesDesc, NULL, 1, 0, 0, 0, NULL, true},
    {Components, {{tObject, 0, 0}}, NULL, NULL, ComponentsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AVStreams, {{tObject, 0,0 }}, NULL, NULL, AVStreamsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AVPlayers, {{tObject, 0, 0}}, NULL, NULL, AVPlayersDesc, NULL, 1, 0, 0, 0, NULL, true},
    {ServiceMonitoring, {{tObject, 0, 0}}, NULL, NULL, ServiceMonitoringDesc, NULL, 1, 0, 0, 0, NULL, true},
    {Applications, {{tObject, 0, 0}}, NULL, NULL, ApplicationsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setSTBServiceEnable, getSTBServiceEnable, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setSTBServiceAlias, getSTBServiceAlias, NULL, NULL},
    {Capabilities, {{tObject, 0, 0}}, NULL, NULL, CapabilitiesDesc, NULL},
    {Components, {{tObject, 0, 0}}, NULL, NULL, ComponentsDesc, NULL},
    {AVStreams, {{tObject, 0,0 }}, NULL, NULL, AVStreamsDesc, NULL},
    {AVPlayers, {{tObject, 0, 0}}, NULL, NULL, AVPlayersDesc, NULL},
	{ServiceMonitoring, {{tObject, 0, 0}}, NULL, NULL, ServiceMonitoringDesc, NULL},
    {Applications, {{tObject, 0, 0}}, NULL, NULL, ApplicationsDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Services.STBService. */
TRxObjNode STBServiceInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, STBServiceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, STBServiceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

