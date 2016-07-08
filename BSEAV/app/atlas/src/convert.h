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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef CONVERT_H__
#define CONVERT_H__

#include "atlas.h"
#include "mstring.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_decoder_types.h"
#include "nexus_ir_input.h"
#if NEXUS_HAS_UHF_INPUT
#include "nexus_uhf_input.h"
#endif
#include "nexus_display_vbi.h"
#include "nexus_vbi.h"
#include "remote.h"
#include "bmedia_types.h"
#include "resource.h"
#include "playback.h"
#include "power.h"
#include "videolist.h"
#ifdef PLAYBACK_IP_SUPPORT
#include "b_playback_ip_lib.h"
#endif

#if NEXUS_HAS_SECURITY
#include "nexus_security_datatypes.h"
#endif
#if B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif

#ifdef NETAPP_SUPPORT
#include "netapp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const char * g_audioCodecs[NEXUS_AudioCodec_eMax]; /* index: audio Codecs */
extern const char * g_videoCodecs[NEXUS_VideoCodec_eMax]; /* index: Video Codecs */

NEXUS_AudioCodec    bmediaToAudioCodec(baudio_format audio);
NEXUS_VideoCodec    bmediaToVideoCodec(bvideo_codec video);
NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value);
NEXUS_PixelFormat   bwinToNexusPixelFormat(bwin_pixel_format bwin_value);
NEXUS_Rect          bwinToNexusRect(bwin_rect brect);

/* STRING_TO_ENUM_DECLARE(funcName, enumType) translates to enumType funcName(const char * str) */
STRING_TO_ENUM_DECLARE(stringToAudioCodec, NEXUS_AudioCodec)
STRING_TO_ENUM_DECLARE(stringToVideoCodec, NEXUS_VideoCodec)
STRING_TO_ENUM_DECLARE(stringToIrRemoteType, NEXUS_IrInputMode)
#if NEXUS_HAS_UHF_INPUT
STRING_TO_ENUM_DECLARE(stringToUhfChannel, NEXUS_UhfInputChannel)
#endif
STRING_TO_ENUM_DECLARE(stringToTransportType, NEXUS_TransportType)
STRING_TO_ENUM_DECLARE(stringToVideoFormat, NEXUS_VideoFormat)
STRING_TO_ENUM_DECLARE(stringToVideoFormatSD, NEXUS_VideoFormat)
STRING_TO_ENUM_DECLARE(stringToKey, eKey)
#if NEXUS_HAS_SECURITY
STRING_TO_ENUM_DECLARE(stringToSecurityAlgorithm, NEXUS_SecurityAlgorithm)
#endif

/* ENUM_TO_MSTRING_DECLARE(funcName, enumType) translates to MString funcName(enumType enumValue) */
ENUM_TO_MSTRING_DECLARE(transportTypeToString, NEXUS_TransportType)
ENUM_TO_MSTRING_DECLARE(audioCodecToString, NEXUS_AudioCodec)
ENUM_TO_MSTRING_DECLARE(audioMpegToNumChannelsString, NEXUS_AudioMpegChannelMode)
ENUM_TO_MSTRING_DECLARE(audioDtsToNumChannelsString, NEXUS_AudioDtsAmode)
ENUM_TO_MSTRING_DECLARE(audioWmaProToNumChannelsString, NEXUS_AudioWmaProAcmod)
ENUM_TO_MSTRING_DECLARE(audioDraToNumChannelsString, NEXUS_AudioDraAcmod)
ENUM_TO_MSTRING_DECLARE(audioCodecToNumChannelsString, NEXUS_AudioCodec)
ENUM_TO_MSTRING_DECLARE(videoCodecToString, NEXUS_VideoCodec)
ENUM_TO_MSTRING_DECLARE(videoFrameRateToString, NEXUS_VideoFrameRate)
ENUM_TO_MSTRING_DECLARE(videoFrameRateThousandthsToString, NEXUS_VideoFrameRate)
ENUM_TO_MSTRING_DECLARE(videoContentModeToString, NEXUS_VideoWindowContentMode)
ENUM_TO_MSTRING_DECLARE(videoFormatToVertRes, NEXUS_VideoFormat)
ENUM_TO_MSTRING_DECLARE(videoFormatToHorizRes, NEXUS_VideoFormat)
ENUM_TO_MSTRING_DECLARE(videoFormatToString, NEXUS_VideoFormat)
ENUM_TO_MSTRING_DECLARE(colorSpaceToString, NEXUS_ColorSpace)
ENUM_TO_MSTRING_DECLARE(aspectRatioToString, NEXUS_AspectRatio)
ENUM_TO_MSTRING_DECLARE(displayAspectRatioToString, NEXUS_DisplayAspectRatio)
ENUM_TO_MSTRING_DECLARE(boardResourceToString, eBoardResource)
ENUM_TO_MSTRING_DECLARE(audioAc3ToNumChannelsString, NEXUS_AudioAc3Acmod)
ENUM_TO_MSTRING_DECLARE(audioAc3LfeToNumChannelsString, NEXUS_AudioAc3Acmod)
ENUM_TO_MSTRING_DECLARE(audioAacToNumChannelsString, NEXUS_AudioAacAcmod)
ENUM_TO_MSTRING_DECLARE(audioAacLfeToNumChannelsString, NEXUS_AudioAacAcmod)
#if NEXUS_HAS_SECURITY
ENUM_TO_MSTRING_DECLARE(securityAlgorithmToString, NEXUS_SecurityAlgorithm)
#endif
ENUM_TO_MSTRING_DECLARE(playbackTrickToString, ePlaybackTrick)
ENUM_TO_MSTRING_DECLARE(playbackHostTrickToString, NEXUS_PlaybackHostTrickMode)
ENUM_TO_MSTRING_DECLARE(nielsenAmolToString, NEXUS_AmolType)
ENUM_TO_MSTRING_DECLARE(macrovisionToString, NEXUS_DisplayMacrovisionType)
ENUM_TO_MSTRING_DECLARE(dcsToString, NEXUS_DisplayDcsType)
ENUM_TO_MSTRING_DECLARE(retToString, eRet)
ENUM_TO_MSTRING_DECLARE(powerModeToString, ePowerMode)
#if NEXUS_HAS_FRONTEND
ENUM_TO_MSTRING_DECLARE(satModeToString, NEXUS_FrontendSatelliteMode)
#endif
#ifdef NETAPP_SUPPORT
ENUM_TO_MSTRING_DECLARE(netappCallbackToString, NETAPP_CB_TYPE)
#endif

MString audioDecodeStatusToNumChannelsString(NEXUS_AudioDecoderStatus * pStatus);

#ifdef PLAYBACK_IP_SUPPORT
STRING_TO_ENUM_DECLARE(stringToIpProtocol, B_PlaybackIpProtocol)
ENUM_TO_MSTRING_DECLARE(ipProtocolToString, B_PlaybackIpProtocol)
STRING_TO_ENUM_DECLARE(stringToIpSecurity, B_PlaybackIpSecurityProtocol)
ENUM_TO_MSTRING_DECLARE(ipSecurityToString, B_PlaybackIpSecurityProtocol)
#endif /* ifdef PLAYBACK_IP_SUPPORT */
#if B_HAS_DTCP_IP
STRING_TO_ENUM_DECLARE(stringToDtcpKeyFormat, B_DTCP_KeyFormat_T)
ENUM_TO_MSTRING_DECLARE(dtcpKeyFormatToString, B_DTCP_KeyFormat_T)
#endif
STRING_TO_ENUM_DECLARE(stringToServerIndexState, eServerIndexState)
ENUM_TO_MSTRING_DECLARE(serverIndexStateToString, eServerIndexState)

#ifdef __cplusplus
}
#endif

#endif /* CONVERT_H__ */