/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "namevalue.h"
#include "nexus_types.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_stc_channel.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_types.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output_extra.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder_types.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_video_window.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#if NEXUS_HAS_SECURITY
#include "nexus_security_datatypes.h"
#endif
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_stc_channel.h"
#endif
#include "nexus_platform_standby.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const namevalue_t g_videoFormatStrs[] = {
    {"ntsc",      NEXUS_VideoFormat_eNtsc},
    {"480i",      NEXUS_VideoFormat_eNtsc},
    {"pal",       NEXUS_VideoFormat_ePal},
    {"pal_m",     NEXUS_VideoFormat_ePalM},
    {"pal_n",     NEXUS_VideoFormat_ePalN},
    {"pal_nc",    NEXUS_VideoFormat_ePalNc},
    {"576i",      NEXUS_VideoFormat_ePal},
    {"1080i",     NEXUS_VideoFormat_e1080i},
    {"1080i50",   NEXUS_VideoFormat_e1080i50hz},
    {"720p",      NEXUS_VideoFormat_e720p},
    {"720p24",    NEXUS_VideoFormat_e720p24hz},
    {"720p25",    NEXUS_VideoFormat_e720p25hz},
    {"720p30",    NEXUS_VideoFormat_e720p30hz},
    {"720p50",    NEXUS_VideoFormat_e720p50hz},
    {"480p",      NEXUS_VideoFormat_e480p},
    {"576p",      NEXUS_VideoFormat_e576p},
    {"1080p",     NEXUS_VideoFormat_e1080p},
    {"1080p24",   NEXUS_VideoFormat_e1080p24hz},
    {"1080p25",   NEXUS_VideoFormat_e1080p25hz},
    {"1080p30",   NEXUS_VideoFormat_e1080p30hz},
    {"1080p50",   NEXUS_VideoFormat_e1080p50hz},
    {"1080p60",   NEXUS_VideoFormat_e1080p},
    {"1080p100",  NEXUS_VideoFormat_e1080p100hz},
    {"1080p120",  NEXUS_VideoFormat_e1080p120hz},
    {"720p3D",    NEXUS_VideoFormat_e3D_720p},
    {"1080p3D",   NEXUS_VideoFormat_e3D_1080p_24Hz},
    {"3840x2160p24", NEXUS_VideoFormat_e3840x2160p24hz},
    {"3840x2160p25", NEXUS_VideoFormat_e3840x2160p25hz},
    {"3840x2160p30", NEXUS_VideoFormat_e3840x2160p30hz},
    {"3840x2160p50", NEXUS_VideoFormat_e3840x2160p50hz},
    {"3840x2160p60", NEXUS_VideoFormat_e3840x2160p60hz},
    {"4096x2160p24", NEXUS_VideoFormat_e4096x2160p24hz},
    {"4096x2160p25", NEXUS_VideoFormat_e4096x2160p25hz},
    {"4096x2160p30", NEXUS_VideoFormat_e4096x2160p30hz},
    {"4096x2160p50", NEXUS_VideoFormat_e4096x2160p50hz},
    {"4096x2160p60", NEXUS_VideoFormat_e4096x2160p60hz},
    {NULL, 0}
};

const namevalue_t g_videoFrameRateStrs[] = {
    {"0",      NEXUS_VideoFrameRate_eUnknown},
    {"7.493",  NEXUS_VideoFrameRate_e7_493},
    {"7.5",    NEXUS_VideoFrameRate_e7_5},
    {"9.99",   NEXUS_VideoFrameRate_e9_99},
    {"10",     NEXUS_VideoFrameRate_e10},
    {"11.988",  NEXUS_VideoFrameRate_e11_988},
    {"12",     NEXUS_VideoFrameRate_e12},
    {"12.5",   NEXUS_VideoFrameRate_e12_5},
    {"14.985", NEXUS_VideoFrameRate_e14_985},
    {"15",     NEXUS_VideoFrameRate_e15},
    {"19.98",  NEXUS_VideoFrameRate_e19_98},
    {"20",     NEXUS_VideoFrameRate_e20},
    {"23.976", NEXUS_VideoFrameRate_e23_976},
    {"24",     NEXUS_VideoFrameRate_e24},
    {"25",     NEXUS_VideoFrameRate_e25},
    {"29.97",  NEXUS_VideoFrameRate_e29_97},
    {"30",     NEXUS_VideoFrameRate_e30},
    {"50",     NEXUS_VideoFrameRate_e50},
    {"59.94",  NEXUS_VideoFrameRate_e59_94},
    {"60",     NEXUS_VideoFrameRate_e60},
    {"100",    NEXUS_VideoFrameRate_e100},
    {"119.98", NEXUS_VideoFrameRate_e119_88},
    {"120",    NEXUS_VideoFrameRate_e120},
    {NULL, 0}
};

const namevalue_t g_transportTypeStrs[] = {
    {"ts",  NEXUS_TransportType_eTs},
    {"pes", NEXUS_TransportType_eMpeg2Pes},
    {"es",  NEXUS_TransportType_eEs},
    {"vob", NEXUS_TransportType_eVob},
    {"mp4", NEXUS_TransportType_eMp4},
    {"mkv", NEXUS_TransportType_eMkv},
    {"avi", NEXUS_TransportType_eAvi},
    {"asf", NEXUS_TransportType_eAsf},
    {"wav", NEXUS_TransportType_eWav},
    {"aiff", NEXUS_TransportType_eAiff},
    {"mp4f", NEXUS_TransportType_eMp4Fragment},
    {"rmff", NEXUS_TransportType_eRmff},
    {"flv", NEXUS_TransportType_eFlv},
    {"ogg", NEXUS_TransportType_eOgg},
    {"dss", NEXUS_TransportType_eDssEs},
    {"dss_es", NEXUS_TransportType_eDssEs}, /* alias. used for SD. */
    {"dss_pes", NEXUS_TransportType_eDssPes}, /* used for HD */
    {"mp1", NEXUS_TransportType_eMpeg1Ps},
    {"amr", NEXUS_TransportType_eAmr},
    {NULL, 0}
};

const namevalue_t g_videoCodecStrs[] = {
    {"mpeg2", NEXUS_VideoCodec_eMpeg2},
    {"mpeg", NEXUS_VideoCodec_eMpeg2},
    {"mpeg1", NEXUS_VideoCodec_eMpeg1},
    {"avc", NEXUS_VideoCodec_eH264},
    {"h264", NEXUS_VideoCodec_eH264},
    {"svc", NEXUS_VideoCodec_eH264_Svc},
    {"mvc", NEXUS_VideoCodec_eH264_Mvc},
    {"h263", NEXUS_VideoCodec_eH263},
    {"hevc", NEXUS_VideoCodec_eH265},
    {"h265", NEXUS_VideoCodec_eH265},
    {"avs", NEXUS_VideoCodec_eAvs},
    {"vc1", NEXUS_VideoCodec_eVc1},
    {"vc1sm", NEXUS_VideoCodec_eVc1SimpleMain},
    {"divx", NEXUS_VideoCodec_eMpeg4Part2},
    {"mpeg4", NEXUS_VideoCodec_eMpeg4Part2},
    {"divx311", NEXUS_VideoCodec_eDivx311},
    {"divx3", NEXUS_VideoCodec_eDivx311},
    {"rv40", NEXUS_VideoCodec_eRv40},
    {"vp6", NEXUS_VideoCodec_eVp6},
    {"vp7", NEXUS_VideoCodec_eVp7},
    {"vp8", NEXUS_VideoCodec_eVp8},
    {"vp9", NEXUS_VideoCodec_eVp9},
    {"spark", NEXUS_VideoCodec_eSpark},
    {"mjpeg", NEXUS_VideoCodec_eMotionJpeg},
    {"none", NEXUS_VideoCodec_eUnknown},
    {NULL, 0}
};

const namevalue_t g_videoCodecProfileStrs[] = {
#if NEXUS_HAS_VIDEO_ENCODER
    {"simple", NEXUS_VideoCodecProfile_eSimple},
    {"main", NEXUS_VideoCodecProfile_eMain},
    {"high", NEXUS_VideoCodecProfile_eHigh},
    {"advanced", NEXUS_VideoCodecProfile_eAdvanced},
    {"jizhun", NEXUS_VideoCodecProfile_eJizhun},
    {"snrscl", NEXUS_VideoCodecProfile_eSnrScalable},
    {"spatialscl", NEXUS_VideoCodecProfile_eSpatiallyScalable},
    {"advsimple", NEXUS_VideoCodecProfile_eAdvancedSimple},
    {"baseline", NEXUS_VideoCodecProfile_eBaseline},
#endif
    {NULL, 0}
};

const namevalue_t g_videoCodecLevelStrs[] = {
#if NEXUS_HAS_VIDEO_ENCODER
    {"0.0", NEXUS_VideoCodecLevel_e00},
    {"1.0", NEXUS_VideoCodecLevel_e10},
    {"1.b", NEXUS_VideoCodecLevel_e1B},
    {"1.1", NEXUS_VideoCodecLevel_e11},
    {"1.2", NEXUS_VideoCodecLevel_e12},
    {"1.3", NEXUS_VideoCodecLevel_e13},
    {"2.0", NEXUS_VideoCodecLevel_e20},
    {"2.1", NEXUS_VideoCodecLevel_e21},
    {"2.2", NEXUS_VideoCodecLevel_e22},
    {"3.0", NEXUS_VideoCodecLevel_e30},
    {"3.1", NEXUS_VideoCodecLevel_e31},
    {"3.2", NEXUS_VideoCodecLevel_e32},
    {"4.0", NEXUS_VideoCodecLevel_e40},
    {"4.1", NEXUS_VideoCodecLevel_e41},
    {"4.2", NEXUS_VideoCodecLevel_e42},
    {"5.0", NEXUS_VideoCodecLevel_e50},
    {"5.1", NEXUS_VideoCodecLevel_e51},
    {"6.0", NEXUS_VideoCodecLevel_e60},
    {"6.2", NEXUS_VideoCodecLevel_e62},
    {"low", NEXUS_VideoCodecLevel_eLow},
    {"main", NEXUS_VideoCodecLevel_eMain},
    {"high", NEXUS_VideoCodecLevel_eHigh},
    {"high1440", NEXUS_VideoCodecLevel_eHigh1440},
#endif
    {NULL, 0}
};

const namevalue_t g_audioCodecStrs[] = {
    {"mpeg", NEXUS_AudioCodec_eMpeg},
    {"mp3", NEXUS_AudioCodec_eMp3},
    {"ac3", NEXUS_AudioCodec_eAc3},
    {"ac3plus", NEXUS_AudioCodec_eAc3Plus},
    {"ac4", NEXUS_AudioCodec_eAc4},
    {"aac", NEXUS_AudioCodec_eAac},
    {"aac_loas", NEXUS_AudioCodec_eAacLoas},
    {"aacplus", NEXUS_AudioCodec_eAacPlus},
    {"aacplus_adts", NEXUS_AudioCodec_eAacPlusAdts},
    {"wma", NEXUS_AudioCodec_eWmaStd},
    {"wmastd", NEXUS_AudioCodec_eWmaStd},
    {"wmats", NEXUS_AudioCodec_eWmaStdTs},
    {"wmapro", NEXUS_AudioCodec_eWmaPro},
    {"pcm", NEXUS_AudioCodec_ePcm},
    {"pcm_wav", NEXUS_AudioCodec_ePcmWav},
    {"dra", NEXUS_AudioCodec_eDra},
    {"dts_legacy", NEXUS_AudioCodec_eDtsLegacy},
    {"dts", NEXUS_AudioCodec_eDts},
    {"dtshd", NEXUS_AudioCodec_eDtsHd},
    {"dtscd", NEXUS_AudioCodec_eDtsCd},
    {"dtslbr", NEXUS_AudioCodec_eDtsExpress},
    {"dts_express", NEXUS_AudioCodec_eDtsExpress},
    {"cook", NEXUS_AudioCodec_eCook},
    {"amrnb", NEXUS_AudioCodec_eAmr},
    {"amrwb", NEXUS_AudioCodec_eAmrWb},
    {"amr", NEXUS_AudioCodec_eAmr},
    {"adpcm", NEXUS_AudioCodec_eAdpcm},
    {"vorbis", NEXUS_AudioCodec_eVorbis},
    {"lpcm_1394", NEXUS_AudioCodec_eLpcm1394},
    {"lpcm_bd", NEXUS_AudioCodec_eLpcmBluRay},
    {"g711", NEXUS_AudioCodec_eG711},
    {"g726", NEXUS_AudioCodec_eG726},
    {"g729", NEXUS_AudioCodec_eG729},
    {"g723.1", NEXUS_AudioCodec_eG723_1},
    {"flac", NEXUS_AudioCodec_eFlac},
    {"mlp", NEXUS_AudioCodec_eMlp},
    {"ape", NEXUS_AudioCodec_eApe},
    {"ilbc", NEXUS_AudioCodec_eIlbc},
    {"isac", NEXUS_AudioCodec_eIsac},
    {"opus", NEXUS_AudioCodec_eOpus},
    {"als", NEXUS_AudioCodec_eAls},
    {"als_loas", NEXUS_AudioCodec_eAlsLoas},
    {"none", NEXUS_AudioCodec_eUnknown},
    {NULL, 0}
};

const namevalue_t g_stcChannelMasterStrs[] = {
#if NEXUS_HAS_TRANSPORT
    {"first", NEXUS_StcChannelAutoModeBehavior_eFirstAvailable},
    {"video", NEXUS_StcChannelAutoModeBehavior_eVideoMaster},
    {"audio", NEXUS_StcChannelAutoModeBehavior_eAudioMaster},
#endif
    {NULL, 0}
};

const namevalue_t g_endOfStreamActionStrs[] = {
#if NEXUS_HAS_PLAYBACK
    {"loop",  NEXUS_PlaybackLoopMode_eLoop},
    {"pause", NEXUS_PlaybackLoopMode_ePause},
    {"play",  NEXUS_PlaybackLoopMode_ePlay},
#endif
    {NULL, 0}
};

const namevalue_t g_tsTimestampType[] = {
    {"none", NEXUS_TransportTimestampType_eNone},
    {"mod300", NEXUS_TransportTimestampType_eMod300},
    {"binary", NEXUS_TransportTimestampType_eBinary},
    {NULL, 0}
};

const namevalue_t g_contentModeStrs[] = {
#if NEXUS_HAS_DISPLAY
    {"zoom", NEXUS_VideoWindowContentMode_eZoom},
    {"box", NEXUS_VideoWindowContentMode_eBox},
    {"panscan", NEXUS_VideoWindowContentMode_ePanScan},
    {"full", NEXUS_VideoWindowContentMode_eFull},
    {"nonlinear", NEXUS_VideoWindowContentMode_eFullNonLinear},
    /* {"panscan_no_cor", NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection}, */
#endif
    {NULL, 0}
};

const namevalue_t g_vsbModeStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"8", NEXUS_FrontendVsbMode_e8},
    {"16", NEXUS_FrontendVsbMode_e16},
#endif
    {NULL, 0}
};

const namevalue_t g_qamModeStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"16", NEXUS_FrontendQamMode_e16},
    {"32", NEXUS_FrontendQamMode_e32},
    {"64", NEXUS_FrontendQamMode_e64},
    {"128", NEXUS_FrontendQamMode_e128},
    {"256", NEXUS_FrontendQamMode_e256},
    {"512", NEXUS_FrontendQamMode_e512},
    {"1024", NEXUS_FrontendQamMode_e1024},
    {"2048", NEXUS_FrontendQamMode_e2048},
    {"4096", NEXUS_FrontendQamMode_e4096},
    {"Auto_64_256", NEXUS_FrontendQamMode_eAuto_64_256},
#endif
    {NULL, 0}
};

const namevalue_t g_ofdmModeStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"dvbt", NEXUS_FrontendOfdmMode_eDvbt},
    {"dvbt2", NEXUS_FrontendOfdmMode_eDvbt2},
    {"dvbc2", NEXUS_FrontendOfdmMode_eDvbc2},
    {"isdbt", NEXUS_FrontendOfdmMode_eIsdbt},
#endif
    {NULL, 0}
};

const namevalue_t g_satModeStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"dvb", NEXUS_FrontendSatelliteMode_eDvb},
    {"dss", NEXUS_FrontendSatelliteMode_eDss},
    {"turboqpsk", NEXUS_FrontendSatelliteMode_eTurboQpsk},
    {"turbo8psk", NEXUS_FrontendSatelliteMode_eTurbo8psk},
    {"qpskldpc", NEXUS_FrontendSatelliteMode_eQpskLdpc},
    {"8pskldpc", NEXUS_FrontendSatelliteMode_e8pskLdpc},
    {"dvbs2qpsk", NEXUS_FrontendSatelliteMode_eDvbs2Qpsk},
    {"dvbs28psk", NEXUS_FrontendSatelliteMode_eDvbs28psk},
    {"dvbs216apsk", NEXUS_FrontendSatelliteMode_eDvbs216apsk},
    {"dvbs232apsk", NEXUS_FrontendSatelliteMode_eDvbs232apsk},
    {"dvbs2", NEXUS_FrontendSatelliteMode_eDvbs2},
    {"dvbs2xqpsk", NEXUS_FrontendSatelliteMode_eDvbs2xQpsk},
    {"dvbs2x8psk", NEXUS_FrontendSatelliteMode_eDvbs2x8psk},
    {"dvbs2x8apsk", NEXUS_FrontendSatelliteMode_eDvbs2x8apsk},
    {"dvbs2x16apsk", NEXUS_FrontendSatelliteMode_eDvbs2x16apsk},
    {"dvbs2x32apsk", NEXUS_FrontendSatelliteMode_eDvbs2x32apsk},
    {"dvbs2x", NEXUS_FrontendSatelliteMode_eDvbs2x},
#endif
    {NULL, 0}
};

const namevalue_t g_diseqcVoltageStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"13V", NEXUS_FrontendDiseqcVoltage_e13v},
    {"18V", NEXUS_FrontendDiseqcVoltage_e18v},
    {"V", NEXUS_FrontendDiseqcVoltage_e13v},
    {"H", NEXUS_FrontendDiseqcVoltage_e18v},
#endif
    {NULL, 0}
};

const namevalue_t g_diseqcToneEnabledStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"on", 1 },
    {"off", 0 },
#endif
    {NULL, 0}
};

const namevalue_t g_satNetworkSpecStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"default", NEXUS_FrontendSatelliteNetworkSpec_eDefault },
    {"custom1", NEXUS_FrontendSatelliteNetworkSpec_eCustom1 },
    {"euro", NEXUS_FrontendSatelliteNetworkSpec_eEuro },
#endif
    {NULL, 0}
};

const namevalue_t g_dvbt2ProfileStrs[] = {
#if NEXUS_HAS_FRONTEND
    {"base", NEXUS_FrontendDvbt2Profile_eBase },
    {"lite", NEXUS_FrontendDvbt2Profile_eLite },
#endif
    {NULL, 0}
};

const namevalue_t g_videoErrorHandling[] = {
#if NEXUS_HAS_VIDEO_DECODER
    {"none", NEXUS_VideoDecoderErrorHandling_eNone},
    {"picture", NEXUS_VideoDecoderErrorHandling_ePicture},
    {"prognostic", NEXUS_VideoDecoderErrorHandling_ePrognostic},
#endif
    {NULL, 0}
};

const namevalue_t g_videoOrientation[] = {
    {"2D", NEXUS_VideoOrientation_e2D},
    {"LeftRight", NEXUS_VideoOrientation_e3D_LeftRight},
    {"3D", NEXUS_VideoOrientation_e3D_LeftRight},
    {"OverUnder", NEXUS_VideoOrientation_e3D_OverUnder},
    {NULL, 0}
};

const namevalue_t g_videoSourceBuffer[] = {
#if NEXUS_HAS_DISPLAY
    {"Default", NEXUS_Display3DSourceBuffer_eDefault},
    {"Left", NEXUS_Display3DSourceBuffer_eLeft},
    {"Right", NEXUS_Display3DSourceBuffer_eRight},
#endif
    {NULL, 0}
};

const namevalue_t g_sourceOrientation[] = {
#if NEXUS_HAS_VIDEO_DECODER
    {"2D", NEXUS_VideoDecoderSourceOrientation_e2D},
    {"LeftRight", NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight},
    {"OverUnder", NEXUS_VideoDecoderSourceOrientation_e3D_OverUnder},
    {"LeftRightFull", NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame},
    {"RightLeftFull", NEXUS_VideoDecoderSourceOrientation_e3D_RightLeftFullFrame},
    {"LeftRightEnhanced", NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame},
#endif
    {NULL, 0}
};

const namevalue_t g_channelChangeMode[] = {
#if NEXUS_HAS_VIDEO_DECODER
    {"mute",         NEXUS_VideoDecoder_ChannelChangeMode_eMute},
    {"holdUntilTsm", NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock},
    {"muteUntilPic", NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture},
    {"holdUntilPic", NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture},
#endif
    {NULL, 0}
};

const namevalue_t g_displayAspectRatioStrs[] = {
#if NEXUS_HAS_DISPLAY
    {"auto", NEXUS_DisplayAspectRatio_eAuto},
    {"4x3", NEXUS_DisplayAspectRatio_e4x3},
    {"16x9", NEXUS_DisplayAspectRatio_e16x9},
    {"sar", NEXUS_DisplayAspectRatio_eSar},
#endif
    {NULL, 0}
};

const namevalue_t g_macrovisionStrs[] = {
#if NEXUS_HAS_DISPLAY
    {"none",        NEXUS_DisplayMacrovisionType_eNone},
    {"agcOnly",     NEXUS_DisplayMacrovisionType_eAgcOnly},
    {"agc2Lines",   NEXUS_DisplayMacrovisionType_eAgc2Lines},
    {"agc4Lines",   NEXUS_DisplayMacrovisionType_eAgc4Lines},
    {"custom",      NEXUS_DisplayMacrovisionType_eCustom},
    {"agcOnlyRgb",  NEXUS_DisplayMacrovisionType_eAgcOnlyRgb},
    {"agc2LinesRgb",NEXUS_DisplayMacrovisionType_eAgc2LinesRgb},
    {"agc4LinesRgb",NEXUS_DisplayMacrovisionType_eAgc4LinesRgb},
    {"test01",      NEXUS_DisplayMacrovisionType_eTest01},
    {"test02",      NEXUS_DisplayMacrovisionType_eTest02},
#endif
    {NULL, 0}
};


const namevalue_t g_securityAlgoStrs[] = {
#if NEXUS_HAS_SECURITY
    {"3des",NEXUS_SecurityAlgorithm_e3DesAba},
    {"aes",NEXUS_SecurityAlgorithm_eAes128},
#endif
    {NULL, 0}
};

const namevalue_t g_platformStandbyModeStrs[] = {
    {"On", NEXUS_PlatformStandbyMode_eOn},
    {"Active", NEXUS_PlatformStandbyMode_eActive},
    {"Passive", NEXUS_PlatformStandbyMode_ePassive},
    {"DeepSleep", NEXUS_PlatformStandbyMode_eDeepSleep},
    {NULL, 0}
};

const namevalue_t g_colorSpaceStrs[] = {
    {"auto", NEXUS_ColorSpace_eAuto},
    {"rgb", NEXUS_ColorSpace_eRgb},
    {"422", NEXUS_ColorSpace_eYCbCr422},
    {"444", NEXUS_ColorSpace_eYCbCr444},
    {"420", NEXUS_ColorSpace_eYCbCr420},
    {NULL, 0}
};

const namevalue_t g_matrixCoeffStrs[] = {
    {"709", NEXUS_MatrixCoefficients_eItu_R_BT_709},
    {"bt709", NEXUS_MatrixCoefficients_eItu_R_BT_709},
    {"2020", NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL},
    {"bt2020", NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL},
    {"2020ncl", NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL},
    {"2020cl", NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL},
    {"input", NEXUS_MatrixCoefficients_eMax},
    {NULL, 0}
};

const namevalue_t g_audioLoudnessStrs[] = {
	{"none", NEXUS_AudioLoudnessEquivalenceMode_eNone},
	{"atsc", NEXUS_AudioLoudnessEquivalenceMode_eAtscA85},
	{"ebu", NEXUS_AudioLoudnessEquivalenceMode_eEbuR128},
	{NULL, 0}
};

const namevalue_t g_videoEotfStrs[] = {
	{"sdr", NEXUS_VideoEotf_eSdr},
	{"hdr10", NEXUS_VideoEotf_eHdr10},
	{"pqhdr", NEXUS_VideoEotf_eHdr10},
	{"smpte2084", NEXUS_VideoEotf_eHdr10},
	{"smpte", NEXUS_VideoEotf_eHdr10},
	{"2084", NEXUS_VideoEotf_eHdr10},
	{"hlg", NEXUS_VideoEotf_eHlg},
	{"aribb67", NEXUS_VideoEotf_eHlg},
	{"arib", NEXUS_VideoEotf_eHlg},
	{"b67", NEXUS_VideoEotf_eHlg},
	{"input", NEXUS_VideoEotf_eMax},
	{NULL, 0}
};

const namevalue_t g_dbvPriorityModeStrs[] = {
    {"auto", NEXUS_HdmiOutputDolbyVisionPriorityMode_eAuto},
    {"video", NEXUS_HdmiOutputDolbyVisionPriorityMode_eVideo},
    {"gfx", NEXUS_HdmiOutputDolbyVisionPriorityMode_eGraphics},
    {NULL, 0}
};

const namevalue_t g_dolbyVisionModeStrs[] = {
    {"auto", NEXUS_HdmiOutputDolbyVisionMode_eAuto},
    {"on", NEXUS_HdmiOutputDolbyVisionMode_eEnabled},
    {"off", NEXUS_HdmiOutputDolbyVisionMode_eDisabled},
    {NULL, 0}
};

const namevalue_t g_syncModeStrs[] = {
#if NEXUS_HAS_SIMPLE_DECODER
	{"none", NEXUS_SimpleStcChannelSyncMode_eOff},
	{"off", NEXUS_SimpleStcChannelSyncMode_eOff},
	{"default", NEXUS_SimpleStcChannelSyncMode_eDefaultAdjustmentConcealment},
	{"defaultAdjustmentConcealment", NEXUS_SimpleStcChannelSyncMode_eDefaultAdjustmentConcealment},
	{"muteAdjustmentConcealment", NEXUS_SimpleStcChannelSyncMode_eDefaultAdjustmentConcealment},
	{"noAdjustmentConcealment", NEXUS_SimpleStcChannelSyncMode_eNoAdjustmentConcealment},
	{"audioAdjustmentConcealment", NEXUS_SimpleStcChannelSyncMode_eAudioAdjustmentConcealment},
#endif
	{NULL, 0}
};

const char *lookup_name(const namevalue_t *table, int value)
{
    unsigned i;
    for (i=0;table[i].name;i++) {
        if (table[i].value == value) {
            return table[i].name;
        }
    }
    return NULL;
}

unsigned lookup(const namevalue_t *table, const char *name)
{
    unsigned i;
    unsigned value;
    char *endptr;
    const char *valueName;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    value = strtol(name, &endptr, 0);
    if(!endptr || *endptr) { /* if valid, *endptr = '\0' */
        value = table[0].value;
    }
    valueName = lookup_name(table, value);
    printf("Unknown cmdline param '%s', using %u as value ('%s')\n", name, value, valueName?valueName:"unknown");
    return value;
}

void print_list(const namevalue_t *table)
{
    unsigned i;
    const char *sep=" {";
    for (i=0;table[i].name;i++) {
        /* skip aliases */
        if (i > 0 && table[i].value == table[i-1].value) continue;
        printf("%s%s",sep,table[i].name);
        sep = ",";
    }
    printf("}");
}

void print_list_option(const char *name, const namevalue_t *list)
{
    /* allow caller to prefix it */
    if (name[0] == ' ' || name[0] == '-') {
        printf("%s", name);
    }
    else {
        printf("  -%s", name);
    }
    print_list(list);
    printf("\n");
}

struct {
    NEXUS_VideoCodec nexus;
    bvideo_codec settop;
} g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_none},
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_unknown},
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eH264_Svc, bvideo_codec_h264_svc},
    {NEXUS_VideoCodec_eH264_Mvc, bvideo_codec_h264_mvc},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eRv40, bvideo_codec_rv40},
    {NEXUS_VideoCodec_eVp6, bvideo_codec_vp6},
    {NEXUS_VideoCodec_eVp8, bvideo_codec_vp8},
    {NEXUS_VideoCodec_eVp9, bvideo_codec_vp9},
    {NEXUS_VideoCodec_eSpark, bvideo_codec_spark},
    {NEXUS_VideoCodec_eMotionJpeg, bvideo_codec_mjpeg},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs},
    {NEXUS_VideoCodec_eH265, bvideo_codec_h265}
};

struct {
    NEXUS_AudioCodec nexus;
    baudio_format settop;
} g_audioCodec[] = {
   {NEXUS_AudioCodec_eUnknown, baudio_format_unknown},
   {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
   {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
   {NEXUS_AudioCodec_eAac, baudio_format_aac},
   {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
   {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
   {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
   {NEXUS_AudioCodec_eAc3, baudio_format_ac3},
   {NEXUS_AudioCodec_eAc3Plus, baudio_format_ac3_plus},
   {NEXUS_AudioCodec_eAc4, baudio_format_ac4},
   {NEXUS_AudioCodec_eDts, baudio_format_dts},
   {NEXUS_AudioCodec_eDtsHd, baudio_format_dts_hd},
   {NEXUS_AudioCodec_eDtsLegacy, baudio_format_dts_cd},
   {NEXUS_AudioCodec_eDtsExpress, baudio_format_dts_lbr},
   {NEXUS_AudioCodec_eLpcmHdDvd, baudio_format_lpcm_hddvd},
   {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
   {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
   {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
   {NEXUS_AudioCodec_eLpcmDvd, baudio_format_lpcm_dvd},
   {NEXUS_AudioCodec_eAvs, baudio_format_avs},
   {NEXUS_AudioCodec_eAmrWb, baudio_format_amr_wb},
   {NEXUS_AudioCodec_eAmrNb, baudio_format_amr_nb},
   {NEXUS_AudioCodec_eDra, baudio_format_dra},
   {NEXUS_AudioCodec_eCook, baudio_format_cook},
   {NEXUS_AudioCodec_ePcmWav, baudio_format_pcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_adpcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_dvi_adpcm},
   {NEXUS_AudioCodec_eVorbis, baudio_format_vorbis},
   {NEXUS_AudioCodec_eLpcm1394, baudio_format_lpcm_1394},
   {NEXUS_AudioCodec_eFlac, baudio_format_flac},
   {NEXUS_AudioCodec_eApe, baudio_format_ape},
   {NEXUS_AudioCodec_eMlp, baudio_format_mlp},
   {NEXUS_AudioCodec_eG711, baudio_format_g711},
   {NEXUS_AudioCodec_eOpus, baudio_format_opus},
   {NEXUS_AudioCodec_eAls, baudio_format_als},
   {NEXUS_AudioCodec_eAlsLoas, baudio_format_als_loas},
};

const namevalue_t g_audioChannelModeStrs[] = {
    {"stereo",  NEXUS_AudioChannelMode_eStereo},
    {"left", NEXUS_AudioChannelMode_eLeft},
    {"right",  NEXUS_AudioChannelMode_eRight},
    {"swapped",  NEXUS_AudioChannelMode_eSwapped},
    {NULL, 0}
};

const namevalue_t g_dolbyDrcModeStrs[] = {
#if NEXUS_HAS_AUDIO
    {"line",     NEXUS_AudioDecoderDolbyDrcMode_eLine},
    {"rf",       NEXUS_AudioDecoderDolbyDrcMode_eRf},
    {"custom_a", NEXUS_AudioDecoderDolbyDrcMode_eCustomA},
    {"custom_d", NEXUS_AudioDecoderDolbyDrcMode_eCustomD},
    {"off",      NEXUS_AudioDecoderDolbyDrcMode_eOff},
#endif
    {NULL, 0}
};

const namevalue_t g_audioDdreCodecs[] = {
#if NEXUS_HAS_AUDIO
    {"ac3",      NEXUS_AudioCodec_eAc3},
    {"ac3Plus",  NEXUS_AudioCodec_eAc3Plus},
#endif
    {NULL, 0}
};

struct {
    NEXUS_TransportType nexus;
    unsigned settop;
} g_mpegType[] = {
    {NEXUS_TransportType_eTs, bstream_mpeg_type_unknown},
    {NEXUS_TransportType_eEs, bstream_mpeg_type_es},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_bes},
    {NEXUS_TransportType_eMpeg2Pes, bstream_mpeg_type_pes},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_ts},
    {NEXUS_TransportType_eDssEs, bstream_mpeg_type_dss_es},
    {NEXUS_TransportType_eDssPes, bstream_mpeg_type_dss_pes},
    {NEXUS_TransportType_eVob, bstream_mpeg_type_vob},
    {NEXUS_TransportType_eAsf, bstream_mpeg_type_asf},
    {NEXUS_TransportType_eAvi, bstream_mpeg_type_avi},
    {NEXUS_TransportType_eMpeg1Ps, bstream_mpeg_type_mpeg1},
    {NEXUS_TransportType_eMp4, bstream_mpeg_type_mp4},
    {NEXUS_TransportType_eMkv, bstream_mpeg_type_mkv},
    {NEXUS_TransportType_eWav, bstream_mpeg_type_wav},
    {NEXUS_TransportType_eAiff, bstream_mpeg_type_aiff},
    {NEXUS_TransportType_eMp4Fragment, bstream_mpeg_type_mp4_fragment},
    {NEXUS_TransportType_eRmff, bstream_mpeg_type_rmff},
    {NEXUS_TransportType_eFlv, bstream_mpeg_type_flv},
    {NEXUS_TransportType_eOgg, bstream_mpeg_type_ogg},
    {NEXUS_TransportType_eFlac, bstream_mpeg_type_flac},
    {NEXUS_TransportType_eAmr, bstream_mpeg_type_amr},
    {NEXUS_TransportType_eApe, bstream_mpeg_type_ape}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i=0;i<sizeof(g_struct)/sizeof(g_struct[0]);i++) { \
        if (g_struct[i].settop == settop_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", settop_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value)
{
    CONVERT(g_videoCodec);
}

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value)
{
    CONVERT(g_audioCodec);
}

NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value)
{
    CONVERT(g_mpegType);
}
