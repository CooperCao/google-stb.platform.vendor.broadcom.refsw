/******************************************************************************
 * (c) 2016 Broadcom Corporation
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

 #include "bip_priv.h"

 BDBG_MODULE(bip_str_to_value);

int BIP_StrTo_BIP_MediaInfoTrackType( const char *str)
{
    const namevalue_t myStrings[] = {
        {"Video", BIP_MediaInfoTrackType_eVideo},
        {"Audio", BIP_MediaInfoTrackType_eAudio},
        {"Pcr"  , BIP_MediaInfoTrackType_ePcr},
        {"Pmt"  , BIP_MediaInfoTrackType_ePmt},
        {"Other", BIP_MediaInfoTrackType_eOther},
        {"Max"  , BIP_MediaInfoTrackType_eMax},
        {NULL, 0}
    };
    return (lookup(myStrings, (str)));
}

int BIP_StrTo_BIP_MediaInfoType(const char *str)
{
    const namevalue_t myStrings[] = {
        { "stream"  ,  BIP_MediaInfoType_eStream },
        { "unknown" ,  BIP_MediaInfoType_eUnknown},
    };
    return (lookup(myStrings, (str)));
}

/* Here are some BIP_StrTo_ wrappers for the NEXUS enums. */
unsigned BIP_StrTo_NEXUS_VideoFormat(const char *str)                     { return( lookup(g_videoFormatStrs, (str)         )); }
unsigned BIP_StrTo_NEXUS_VideoFrameRate(const char *str)                  { return( lookup(g_videoFrameRateStrs, (str)      )); }
unsigned BIP_StrTo_NEXUS_TransportType(const char *str)                   { return( lookup(g_transportTypeStrs, (str)       )); }
unsigned BIP_StrTo_NEXUS_VideoCodec(const char *str)                      { return( lookup(g_videoCodecStrs, (str)          )); }
unsigned BIP_StrTo_NEXUS_VideoCodecProfile(const char *str)               { return( lookup(g_videoCodecProfileStrs, (str)   )); }
unsigned BIP_StrTo_NEXUS_VideoCodecLevel(const char *str)                 { return( lookup(g_videoCodecLevelStrs, (str)     )); }
unsigned BIP_StrTo_NEXUS_AudioCodec(const char *str)                      { return( lookup(g_audioCodecStrs, (str)          )); }
unsigned BIP_StrTo_NEXUS_StcChannelAutoModeBehavior(const char *str)      { return( lookup(g_stcChannelMasterStrs, (str)    )); }
unsigned BIP_StrTo_NEXUS_PlaybackLoopMode(const char *str)                { return( lookup(g_endOfStreamActionStrs, (str)   )); }
unsigned BIP_StrTo_NEXUS_TransportTimestampType(const char *str)          { return( lookup(g_tsTimestampType, (str)         )); }
unsigned BIP_StrTo_NEXUS_VideoWindowContentMode(const char *str)          { return( lookup(g_contentModeStrs, (str)         )); }
unsigned BIP_StrTo_NEXUS_FrontendVsbMode(const char *str)                 { return( lookup(g_vsbModeStrs, (str)             )); }
unsigned BIP_StrTo_NEXUS_FrontendQamMode(const char *str)                 { return( lookup(g_qamModeStrs, (str)             )); }
unsigned BIP_StrTo_NEXUS_FrontendOfdmMode(const char *str)                { return( lookup(g_ofdmModeStrs, (str)            )); }
unsigned BIP_StrTo_NEXUS_FrontendSatelliteMode(const char *str)           { return( lookup(g_satModeStrs, (str)             )); }
unsigned BIP_StrTo_NEXUS_FrontendDiseqcVoltage(const char *str)           { return( lookup(g_diseqcVoltageStrs, (str)       )); }
unsigned BIP_StrTo_NEXUS_FrontendSatelliteNetworkSpec(const char *str)    { return( lookup(g_satNetworkSpecStrs, (str)      )); }
unsigned BIP_StrTo_NEXUS_FrontendSatelliteNyquistFilter(const char *str)  { return( lookup(g_satNetworkSpecStrs, (str)      )); }
unsigned BIP_StrTo_NEXUS_FrontendDvbt2Profile(const char *str)            { return( lookup(g_dvbt2ProfileStrs, (str)        )); }
unsigned BIP_StrTo_NEXUS_VideoDecoderErrorHandling(const char *str)       { return( lookup(g_videoErrorHandling, (str)      )); }
unsigned BIP_StrTo_NEXUS_VideoOrientation(const char *str)                { return( lookup(g_videoOrientation, (str)        )); }
unsigned BIP_StrTo_NEXUS_Display3DSourceBuffer(const char *str)           { return( lookup(g_videoSourceBuffer, (str)       )); }
unsigned BIP_StrTo_NEXUS_VideoDecoderSourceOrientation(const char *str)   { return( lookup(g_sourceOrientation, (str)       )); }
unsigned BIP_StrTo_NEXUS_DisplayAspectRatio(const char *str)              { return( lookup(g_displayAspectRatioStrs, (str)  )); }
unsigned BIP_StrTo_NEXUS_SecurityAlgorithm(const char *str)               { return( lookup(g_securityAlgoStrs, (str)        )); }
unsigned BIP_StrTo_NEXUS_PlatformStandbyMode(const char *str)             { return( lookup(g_platformStandbyModeStrs, (str) )); }
unsigned BIP_StrTo_NEXUS_ColorSpace(const char *str)                      { return( lookup(g_colorSpaceStrs, (str)          )); }
unsigned BIP_StrTo_NEXUS_AudioLoudnessEquivalenceMode(const char *str)    { return( lookup(g_audioLoudnessStrs, (str)       )); }
unsigned BIP_StrTo_NEXUS_AudioChannelMode(const char *str)                { return( lookup(g_audioChannelModeStrs, (str)    )); }
unsigned BIP_StrTo_NEXUS_AudioDecoderDolbyDrcMode(const char *str)        { return( lookup(g_dolbyDrcModeStrs, (str)        )); }
