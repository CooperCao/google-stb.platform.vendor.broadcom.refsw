/******************************************************************************
 * (c) 2007-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#ifndef BIP_TO_STR_H
#define BIP_TO_STR_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_to_str

BIP_ToStr - BIP Enum to String Conversion Functions

**/


/**
Summary:
Convert an enum value to a NULL-terminated string

Description:
Naming convention is BIP_ToStr_<enum typedef>()

Calling Context:
\code
    if (psi.pMediaInfoTracks[i].type != BIP_MediaInfoTrackType_eVideo)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "trackId %d has incorrect type:%s Should be video"
                   BIP_MSG_PRE_ARG,
                   trackId,
                   BIP_ToStr_BIP_MediaInfoTrackType(psi.pMediaInfoTracks[i].type));
    }
\endcode

**/

/* Prototypes for BIP's native FromStr functions.*/
unsigned BIP_FromStr_BIP_HttpRequestMethod( const char * name);
unsigned BIP_FromStr_BIP_HttpResponseStatus( const char * name);




/* Prototypes for BIP's native ToStr functions.*/
const char * BIP_ToStr_BIP_Arb_State( int value);
const char * BIP_ToStr_BIP_Arb_ThreadOrigin( int value);
const char * BIP_ToStr_BIP_HttpRequestMethod( int value);
const char * BIP_ToStr_BIP_HttpRequestMethod( int value);
const char * BIP_ToStr_BIP_HttpRequest_State( int value);
const char * BIP_ToStr_BIP_HttpResponseStatus( int value);
const char * BIP_ToStr_BIP_HttpResponseStatus( int value);
const char * BIP_ToStr_BIP_HttpResponse_State( int value);
const char * BIP_ToStr_BIP_HttpServerListenerState( int value);
const char * BIP_ToStr_BIP_HttpServerSocketState( int value);
const char * BIP_ToStr_BIP_HttpServerStartState( int value);
const char * BIP_ToStr_BIP_HttpSocketCallbackState( int value);
const char * BIP_ToStr_BIP_HttpSocketConsumerCallbackState( int value);
const char * BIP_ToStr_BIP_HttpSocketShutdownState( int value);
const char * BIP_ToStr_BIP_HttpSocketState( int value);
const char * BIP_ToStr_BIP_HttpStreamerOutputState( int value);
const char * BIP_ToStr_BIP_HttpStreamerProtocol( int value);
const char * BIP_ToStr_BIP_HttpStreamerResponseHeadersState( int value);
const char * BIP_ToStr_BIP_HttpStreamerState( int value);
const char * BIP_ToStr_BIP_HttpVersion( int value);
const char * BIP_ToStr_BIP_IgmpListener_MembershipReportType( int value);
const char * BIP_ToStr_BIP_IoCheckerEvent( int value);
const char * BIP_ToStr_BIP_ListenerCallbackState( int value);
const char * BIP_ToStr_BIP_MediaInfoNavCreated( int value);
const char * BIP_ToStr_BIP_MediaInfoTrackType( int value);
const char * BIP_ToStr_BIP_NetworkAddressType( int value);
const char * BIP_ToStr_BIP_RtspIgmpMemRepStatus( int value);
const char * BIP_ToStr_BIP_RtspLiveMediaStreamingMode( int value);
const char * BIP_ToStr_BIP_RtspRequestMethod( int value);
const char * BIP_ToStr_BIP_RtspResponseStatus( int value);
const char * BIP_ToStr_BIP_RtspSocketState( int value);
const char * BIP_ToStr_BIP_SocketCallbackState( int value);
const char * BIP_ToStr_BIP_SocketType( int value);
const char * BIP_ToStr_BIP_StreamerInputState( int value);
const char * BIP_ToStr_BIP_StreamerInputType( int value);
const char * BIP_ToStr_BIP_StreamerMpeg2TsPatPmtMode( int value);
const char * BIP_ToStr_BIP_StreamerOutputState( int value);
const char * BIP_ToStr_BIP_StreamerProtocol( int value);
const char * BIP_ToStr_BIP_StreamerState( int value);
const char * BIP_ToStr_BIP_TranscodeState( int value);
const char * BIP_ToStr_BIP_UdpStreamerOutputState( int value);
const char * BIP_ToStr_BIP_UdpStreamerProtocol( int value);
const char * BIP_ToStr_BIP_UdpStreamerState( int value);
const char * BIP_ToStr_HttpParserState( int value);
const char * BIP_ToStr_HttpRangeParserState( int value);
const char * BIP_ToStr_LastReadByteStatus( int value);
const char * BIP_ToStr_BIP_PlayerContainerType( int value);
const char * BIP_ToStr_BIP_PlayerState( int value);
const char * BIP_ToStr_BIP_PlayerSubState( int value);
const char * BIP_ToStr_BIP_PlayerClockRecoveryMode( int value);
const char * BIP_ToStr_BIP_DtcpIpClientFactoryAkeEntryState( int value);
const char * BIP_ToStr_BIP_PlayerDataAvailabilityModel( int value);
const char * BIP_ToStr_BIP_PlayerMode( int value);
const char * BIP_ToStr_BIP_MediaInfoType(int value);

/* Conveniently named wrapper functions for NEXUS types. */
const char * BIP_ToStr_NEXUS_VideoFormat(int value);
const char * BIP_ToStr_NEXUS_VideoFrameRate(int value);
const char * BIP_ToStr_NEXUS_TransportType(int value);
const char * BIP_ToStr_NEXUS_VideoCodec(int value);
const char * BIP_ToStr_NEXUS_VideoCodecProfile(int value);
const char * BIP_ToStr_NEXUS_VideoCodecLevel(int value);
const char * BIP_ToStr_NEXUS_AudioCodec(int value);
const char * BIP_ToStr_NEXUS_StcChannelAutoModeBehavior(int value);
const char * BIP_ToStr_NEXUS_PlaybackLoopMode(int value);
const char * BIP_ToStr_NEXUS_TransportTimestampType(int value);
const char * BIP_ToStr_NEXUS_VideoWindowContentMode(int value);
const char * BIP_ToStr_NEXUS_FrontendVsbMode(int value);
const char * BIP_ToStr_NEXUS_FrontendQamMode(int value);
const char * BIP_ToStr_NEXUS_FrontendOfdmMode(int value);
const char * BIP_ToStr_NEXUS_FrontendSatelliteMode(int value);
const char * BIP_ToStr_NEXUS_FrontendDiseqcVoltage(int value);
const char * BIP_ToStr_NEXUS_FrontendSatelliteNetworkSpec(int value);
const char * BIP_ToStr_NEXUS_FrontendSatelliteNyquistFilter(int value);
const char * BIP_ToStr_NEXUS_FrontendDvbt2Profile(int value);
const char * BIP_ToStr_NEXUS_VideoDecoderErrorHandling(int value);
const char * BIP_ToStr_NEXUS_VideoOrientation(int value);
const char * BIP_ToStr_NEXUS_Display3DSourceBuffer(int value);
const char * BIP_ToStr_NEXUS_VideoDecoderSourceOrientation(int value);
const char * BIP_ToStr_NEXUS_DisplayAspectRatio(int value);
const char * BIP_ToStr_NEXUS_SecurityAlgorithm(int value);
const char * BIP_ToStr_NEXUS_PlatformStandbyMode(int value);
const char * BIP_ToStr_NEXUS_ColorSpace(int value);
const char * BIP_ToStr_NEXUS_AudioLoudnessEquivalenceMode(int value);
const char * BIP_ToStr_NEXUS_AudioChannelMode(int value);
const char * BIP_ToStr_NEXUS_AudioDecoderDolbyDrcMode(int value);

#ifdef __cplusplus
}
#endif

#endif /* BIP_TO_STR_H */
