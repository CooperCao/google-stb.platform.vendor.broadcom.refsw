 /******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/

#ifndef BIP_STR_TO_VALUE_H
#define BIP_STR_TO_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif


int BIP_StrTo_BIP_MediaInfoTrackType( const char *str);
int BIP_StrTo_BIP_MediaInfoType(const char *str);
int BIP_StrTo_BIP_MediaInfoCaptionType(const char *str);

/* Conveniently named wrapper functions to convert string to NEXUS types. */
unsigned BIP_StrTo_NEXUS_VideoFormat(const char *str);
unsigned BIP_StrTo_NEXUS_VideoFrameRate(const char *str);
unsigned BIP_StrTo_NEXUS_TransportType(const char *str);
unsigned BIP_StrTo_NEXUS_VideoCodec(const char *str);
unsigned BIP_StrTo_NEXUS_VideoCodecProfile(const char *str);
unsigned BIP_StrTo_NEXUS_VideoCodecLevel(const char *str);
unsigned BIP_StrTo_NEXUS_AudioCodec(const char *str);
unsigned BIP_StrTo_NEXUS_StcChannelAutoModeBehavior(const char *str);
unsigned BIP_StrTo_NEXUS_PlaybackLoopMode(const char *str);
unsigned BIP_StrTo_NEXUS_TransportTimestampType(const char *str);
unsigned BIP_StrTo_NEXUS_VideoWindowContentMode(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendVsbMode(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendQamMode(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendOfdmMode(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendSatelliteMode(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendDiseqcVoltage(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendSatelliteNetworkSpec(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendSatelliteNyquistFilter(const char *str);
unsigned BIP_StrTo_NEXUS_FrontendDvbt2Profile(const char *str);
unsigned BIP_StrTo_NEXUS_VideoDecoderErrorHandling(const char *str);
unsigned BIP_StrTo_NEXUS_VideoOrientation(const char *str);
unsigned BIP_StrTo_NEXUS_Display3DSourceBuffer(const char *str);
unsigned BIP_StrTo_NEXUS_VideoDecoderSourceOrientation(const char *str);
unsigned BIP_StrTo_NEXUS_DisplayAspectRatio(const char *str);
unsigned BIP_StrTo_NEXUS_SecurityAlgorithm(const char *str);
unsigned BIP_StrTo_NEXUS_PlatformStandbyMode(const char *str);
unsigned BIP_StrTo_NEXUS_ColorSpace(const char *str);
unsigned BIP_StrTo_NEXUS_AudioLoudnessEquivalenceMode(const char *str);
unsigned BIP_StrTo_NEXUS_AudioChannelMode(const char *str);
unsigned BIP_StrTo_NEXUS_AudioDecoderDolbyDrcMode(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* BIP_STR_TO_VALUE_H */
