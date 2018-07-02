/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "nexus_file.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(live_ad_replacement);

#define MAX_TEST_CASES 26
#define MAX_ADS   8
#define MAX_READ (188*1024)
#define MAX_ACQUIRE_TIME 20000
#define AUDIO_ENABLE 1
#define AUDIO_SPLICE_TOLERANCE 9000
#define VIDEO_SPLICE_TOLERANCE 9000
#define AUDIO_SPLICE_LAG  9000
#define START_IN_AD 0

bool videoMpeg2 = false;
typedef struct adInformation
{
    unsigned mainSpliceOutFrameNo;
    uint32_t mainSpliceOutPts;
    unsigned mainSpliceInFrameNo;
    uint32_t mainSpliceInPts;
    uint32_t adPtsStart;
    uint32_t adPtsEnd;
    uint32_t vPid;
    uint32_t aPid;
    char adFileName[256];
    FILE *file;
}adInformation;

typedef struct streamInfo{
    char streamName[256];
    uint16_t videoPid;
    uint16_t audioPid;
    uint16_t pcrPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_AudioCodec audioCodec;
    int numAdSlots;
    adInformation adInfo[MAX_ADS];
}streamInfo;
adInformation *adInfo;
int numAdSlots;


/* Stream base definitions for easy relocation of streams to different mount
   point and/or local hard disk. Just move your stream directory and change
   STREAM_BASE(x) to point to a new location */
/* stream location stbsjc-aut-2.sjs.broadcom.net:/local/home/spothana/streams */
#define STREAM_BASE1 "/mnt/hd/streams/"
/* stream location stbsjc-aut-2.sjs.broadcom.net:/local/home/jxliu/streams */
#define STREAM_BASE2 "/mnt/hd/streams/"

static streamInfo test_cases[MAX_TEST_CASES] =
{
    {                           /* 1 */
        "big_buck_bunny_60hz_streamer.v4.ts",
        0x1001,
        0x1101,
        0x1001,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eAac,
        8,
        {
           {   3628,  0x002a34b0,  5068, 0x003aaf70, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad1_playback.ts", NULL},
           {   8761,  0x0064f2c6, 10221, 0x0075a81e, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad2_playback.ts", NULL},
           {  13973,  0x00A0984E, 15413, 0x00b1130e, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad3_playback.ts", NULL},
           {  19106,  0x00DB5664, 20546, 0x00ebd124, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad4_playback.ts", NULL},
           {  24001,  0x01135B36, 25441, 0x0123d5f6, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad5_playback.ts", NULL},
           {  30164,  0x0159E2E0, 31604, 0x016a5da0, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad6_playback.ts", NULL},
           {  36401,  0x01A14356, 37841, 0x01b1be16, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad7_playback.ts", NULL},
           {  39761,  0x01C7B716, 41201, 0x01d831d6, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad7_playback.ts", NULL}
       }
    },
    {                           /* 2 */
        "big_buck_bunny_hevc_60hz_streamer.ts",
        0x1001,
        0x1101,
        0x1001,
        NEXUS_VideoCodec_eH265,
        NEXUS_AudioCodec_eAac,
        8,
        {
            {   3628,  0x002a34b0,  5068, 0x003aaf70, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad1_playback.ts", NULL},
            {   8781,  0x00652D5E, 10221, 0x0075a81e, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad2_playback.ts", NULL},
            {  13973,  0x00A0984E, 15413, 0x00b1130e, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad3_playback.ts", NULL},
            {  19106,  0x00DB5664, 20546, 0x00ebd124, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad4_playback.ts", NULL},
            {  24001, 0x001135B36, 25441, 0x0123d5f6, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad5_playback.ts", NULL},
            {  30164, 0x00159E2E0, 31604, 0x016a5da0, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad6_playback.ts", NULL},
            {  36401, 0x001A14356, 37841, 0x01b1be16, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad7_playback.ts", NULL},
            {  39761, 0x001C7B716, 41201, 0x01d831d6, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad7_playback.ts", NULL}
       }
    },
    {                           /* 3 */
        "QAM_ITV_SCTE35_TSIS102_SDMPEG2_188.ts",
        0x101,
        0x111,
        0x101,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        2,
        {
           {   845,  0x08F99E6E,  1595, 0x090E37DE, 0x3611, 0x14ba69, 0x65, 0x1001,STREAM_BASE1 "vm_adIns/750_SD_MPEG2_6m.ts", NULL},
           {  1595,  0x090E37DE,  2345, 0x0922D14E, 0x7706, 0x15096e, 0x65, 0x1001,STREAM_BASE1 "vm_adIns/750_SD_MPEG2_1m.ts", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
           {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 4 */
        "QAM_ITV_SCTE35_TSIS102_HDMPEG2_188.ts",
        0x101,
        0x111,
        0x101,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        2,
        {
            {   887,  0x0D9C50A6,  1637, 0x0DB0EA16, 0x5635, 0x14e89d, 0x1000, 0x1001,STREAM_BASE1 "vm_adIns/750_HD_MPEG2_19m.ts", NULL},
            {  1637,  0x0DB0EA16,  2387, 0x0DC58386, 0x7706, 0x15096e, 0x1000, 0x1001,STREAM_BASE1 "vm_adIns/750_HD_MPEG2_6m.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
        }
    },
    {                           /* 5 */
        "QAM_ITV_SCTE35_TSIS102_SDMPEG4_188.ts",
        0x101,
        0x111,
        0x101,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        2,
        {
            {   1710,  0x0A4325C9,  3210, 0xA57BF39, 0x16698, 0x15fc84, 0x810, 0x814,STREAM_BASE1 "vm_adIns/750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   3210,  0x0A57BF39,  5710, 0xA6C58A9, 0x16698, 0x15fc84, 0x810, 0x814,STREAM_BASE1 "vm_adIns/750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
        }
    },
    {                           /* 6 */
        "QAM_ITV_SCTE35_TSIS102_HDMPEG4_188.ts",
        0x101,
        0x111,
        0x101,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        2,
        {
            {   1680,  0x0F42D899,  3180, 0x0F577209, 0x16698, 0x15f57c, 0x810, 0x814,STREAM_BASE1 "vm_adIns/750_HD_H264_1920_1088i_25fps_2000kbps_16x9.ts", NULL},
            {   3180,  0x0F577209,  4680, 0xf6c0b79,  0x16698, 0x15f57c, 0x810, 0x814,STREAM_BASE1 "vm_adIns/750_HD_H264_1920_1088i_25fps_15000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 7 */
        "keith_itv_single_Lossless_MPEG2_1Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x00128996,  1158, 0x00204536, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00204536,  1658, 0x002e00d6, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00497816,  3158, 0x005733b6, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005733b6,  3908, 0x006bcd26, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bcd26,  4658, 0x00806696, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x00806696,  5408, 0x00950006, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 8 */
        "keith_itv_single_Lossless_MPEG2_1.5Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x00128997,  1158, 0x00204537, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00204537,  1658, 0x002e00d7, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00497817,  3158, 0x005733b7, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005733b7,  3908, 0x006bcd27, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bcd27,  4658, 0x00806697, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x00806697,  5408, 0x00950007, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 9 */
        "keith_itv_single_Lossless_MPEG2_3Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x001277ac,  1158, 0x0020334c, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x0020334c,  1658, 0x002deeec, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x0049662c,  3158, 0x005721cc, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005721cc,  3908, 0x006bbb3c, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bbb3c,  4658, 0x008054ac, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x008054ac,  5408, 0x0094ee1c, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 10 */
        "keith_itv_single_Lossless_MPEG2_6Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x001248a1,  1158, 0x00200441, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00200441,  1658, 0x002dbfe1, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00493721,  3158, 0x0056f2c1, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x0056f2c1,  3908, 0x006b8c31, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006b8c31,  4658, 0x008025a1, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x008025a1,  5408, 0x0094bf11, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 11 */
        "keith_itv_single_Lossless_H264_720x576i25.00_16x9_2Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   1316,  0x0012c960,  2316, 0x00208500, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  3316, 0x002e40a0, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  6316, 0x00577380, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  7816, 0x006c0cf0, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  9316, 0x0080a660, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  10816, 0x00953fd0,0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 12 */
        "keith_itv_single_Lossless_H264_720x576i25.00_16x9_2.5Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   1316,  0x0012c960,  2316, 0x00208500, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  3316, 0x002e40a0, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  6316, 0x00577380, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  7816, 0x006c0cf0, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  9316, 0x0080a660, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  10816, 0x00953fd0,0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 13 */
        "keith_itv_single_Lossless_H264_720x576i25.00_16x9_4Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   1316,  0x0012c960,  2316, 0x00208500, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  3316, 0x002e40a0, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  6316, 0x00577380, 0x16698, 0x0de602, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  7816, 0x006c0cf0, 0x16698, 0x14fb5e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  9316, 0x0080a660, 0x16698, 0x14e974, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  10816, 0x00953fd0,0x16698, 0x14ba69, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 14 */
        "keith_itv_single_Lossless_H264_720x576i25.00_16x9_6Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   1316,  0x0012c960,  2316, 0x00208500, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  3316, 0x002e40a0, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  6316, 0x00577380, 0x16698, 0x0de602, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  7816, 0x006c0cf0, 0x16698, 0x14fb5e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  9316, 0x0080a660, 0x16698, 0x14e974, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  10816, 0x00953fd0,0x16698, 0x14ba69, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 15 */
        "keith_itv_single_Lossless_MPEG_1920_1080i_25fps_16000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x0012779E,  1158, 0x0020333E, 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x0020333E,  1658, 0x002DEEDE, 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x0049661E,  3158, 0x005721BE, 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005721BE,  3908, 0x006BBB2E, 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006BBB2E,  4658, 0x0080549E, 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x0080549E,  5408, 0x0094EE0E, 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 16 */
        "keith_itv_single_Lossless_MPEG_1920_1080i_25fps_12000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x00128996,  1158, 0x00204536, 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x00204536,  1658, 0x002e00d6, 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x00497816,  3158, 0x005733b6, 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005733b6,  3908, 0x006bcd26, 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006bcd26,  4658, 0x00806696, 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x00806696,  5408, 0x00950006, 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 17 */
        "keith_itv_single_Lossless_MPEG_1920_1080i_25fps_8000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        6,
        {
            {   658,   0x00128996,  1158, 0x00204536, 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x00204536,  1658, 0x002e00d6, 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x00497816,  3158, 0x005733b6, 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005733b6,  3908, 0x006bcd26, 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006bcd26,  4658, 0x00806696, 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x00806696,  5408, 0x00950006, 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 18 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        3,
        {
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 19 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_6000kbps_16x9_AC3.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        3,
        {
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 20 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_8000kbps_16x9_AC3.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        3,
        {
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698,  0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 21 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        3,
        {
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}
       }
    },
    {                           /* 22 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        6,
        {
            {   1316,  0x001375A4,  2316, 0x00213144, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x00213144,  3316, 0x002EECE4, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A6424,  6316, 0x00581FC4, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00581FC4,  7816, 0x006CB934, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CB934,  9316, 0x008152A4, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x008152A4, 10816, 0x0095EC14, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}

       }
    },
    {                           /* 23 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_6000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        6,
        {
            {   1316,  0x00137928,  2316, 0x002134C8, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  3316, 0x002EF068, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  6316, 0x00582348, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}

       }
    },
    {                           /* 24 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        6,
        {
            {   1316,  0x00137928,  2316, 0x002134C8, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  3316, 0x002EF068, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  6316, 0x00582348, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}

       }
    },
    {                           /* 25 */
        "keith_itv_single_Lossless_H264_1920_1080i_25fps_18000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        6,
        {
            {   1316,  0x00137928,  2316, 0x002134C8, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  3316, 0x002EF068, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  6316, 0x00582348, 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  7816, 0x006CBCB8, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  9316, 0x00815628, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628, 10816, 0x0095EF98, 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL},
            {   0x0,        0x0,    0x0,        0x0, 0x0,      0x0,  0x0,    0x0,                                            "", NULL}

       }
    },
    {                           /* 26 */
        "main_live.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        3,
        {
            {   1536,  0x15C7C8,   3072, 0x2ADfc8,  0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad1_playback.ts", NULL},
            {   4608,  0x3FF7C8,   6144, 0x550fc8,  0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad2_playback.ts", NULL},
            {   7680,  0x6A27C8,   9216, 0x7F3fc8,  0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad3_playback.ts", NULL},
            {   0x0,        0x0,    0x0,      0x0,     0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,    0x0,      0x0,     0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,    0x0,      0x0,     0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,    0x0,      0x0,     0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,    0x0,      0x0,     0x0,      0x0,   0x0,   0x0,                                      "", NULL}


       }
    }
};

typedef enum btp_cmd {
    btp_cmd_splice_start_marker,
    btp_cmd_splice_stop_marker,
    btp_cmd_splice_pcr_offset_marker,
    btp_cmd_splice_transition_marker,
    btp_cmd_splice_stc_offset_marker,
    btp_cmd_splice_inline_flush
}btp_cmd;

typedef struct btp_input {
    btp_cmd cmd;
    uint32_t sub_cmd;
    uint8_t * pBuffer;
    size_t length;
    uint16_t pid;
}btp_input;




typedef struct adContext
{
    btp_input btp_in;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle playpumpEvent;
    int adIndex;
    off_t fileOffset;
}adContext;

typedef struct liveContext
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    BKNI_MutexHandle mutex;
    BKNI_EventHandle spliceInEvent,spliceOutEvent,demoDone;
    bool audioSplice;
    bool videoSplice;
    adContext adCtx;
}liveContext;

char adInsertionStateText[][20] =
{
    "eNone",
    "eWaitForStopPts",
    "eFoundStopPts",
    "eWaitForStartPts",
    "eFoundStartPts",
    "eMax"
};

typedef struct adPosition {
    off_t fileOffset;
} adPosition;

#if (START_IN_AD == 1)
static NEXUS_Error computeAdOffset (liveContext * liveCtx, uint32_t livepts, adPosition * pos)
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    adContext *adCtx = &liveCtx->adCtx;

    if (NULL == pos) {
        rc = NEXUS_INVALID_PARAMETER;
        return rc;
    }

    uint32_t ptsOffset = livepts - adInfo[adCtx->adIndex].mainSpliceOutPts;
    uint32_t adTime = ptsOffset/45;
    /*Open file and find position*/
    NEXUS_FilePlayHandle file;
    char * source_fname, source_navname[256];
    source_fname = adInfo[adCtx->adIndex].adFileName;
    strncpy(source_navname, source_fname, 256);
    char * name = basename(source_navname);
    char * ext = strrchr(name, '.');
    strncpy(ext, ".nav", 4);
    file = NEXUS_FilePlay_OpenPosix(source_fname, source_navname);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", source_fname);
        return rc;
    }

    NEXUS_FilePosition first, last, start;
    rc = NEXUS_FilePlay_GetBounds(file, &first, &last);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);
    fprintf(stderr, "first:%u,%u,%u last:%u,%u,%u\n",
		(unsigned)first.timestamp, (unsigned)first.mpegFileOffset, (unsigned)first.indexOffset,
		(unsigned)last.timestamp, (unsigned)last.mpegFileOffset, (unsigned)last.indexOffset);

    rc = NEXUS_FilePlay_GetLocation(file, first.timestamp + adTime, &start);
    fprintf(stderr, "pos:%u:%u:[%u,%u,%u]\n",
            (unsigned)livepts, adTime, (unsigned)start.timestamp, (unsigned)start.mpegFileOffset, (unsigned)start.indexOffset);

    NEXUS_FilePlay_Close(file);
    pos->fileOffset = start.mpegFileOffset;
    return rc;
}
#endif

static void playpump_data_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#if (AUDIO_ENABLE == 1)
static void audio_insertion_point_callback(void *context, int param)
{
    NEXUS_AudioDecoderSpliceStatus spliceStatus;
    liveContext *liveCtx = (liveContext *)context;

    BSTD_UNUSED(param);
    NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&spliceStatus);
    BDBG_WRN(("%s: insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[spliceStatus.state]));
    switch (spliceStatus.state) {
    case NEXUS_DecoderSpliceState_eFoundStopPts:
        /* we found main stop pts and signalling to start the ad */
        BKNI_AcquireMutex(liveCtx->mutex);
        liveCtx->audioSplice = true;
        if(liveCtx->videoSplice) {
            liveCtx->audioSplice = false;
            liveCtx->videoSplice = false;
            BKNI_SetEvent(liveCtx->spliceOutEvent);
        }
        BKNI_ReleaseMutex(liveCtx->mutex);
        break;
    case NEXUS_DecoderSpliceState_eFoundStartPts:
        /* we found main start pts and signalling to cleanup or set the next splice point */
        BKNI_AcquireMutex(liveCtx->mutex);
        liveCtx->audioSplice = true;
        if(liveCtx->videoSplice)
        {
            liveCtx->audioSplice = false;
            liveCtx->videoSplice = false;
            BKNI_SetEvent(liveCtx->spliceInEvent);
        }
        BKNI_ReleaseMutex(liveCtx->mutex);
        break;
    default:
        break;
    }
    return;
}
#endif

static void video_insertion_point_callback(void *context, int param)
{
    NEXUS_VideoDecoderSpliceStatus spliceStatus;
    liveContext *liveCtx = (liveContext *)context;

    BSTD_UNUSED(param);
    NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&spliceStatus);
    BDBG_WRN(("%s: insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[spliceStatus.state]));
    switch (spliceStatus.state) {
    case NEXUS_DecoderSpliceState_eFoundStopPts:
        /* we found main stop pts and signalling to start the ad */
        BKNI_AcquireMutex(liveCtx->mutex);
        liveCtx->videoSplice = true;
#if AUDIO_ENABLE
        if(liveCtx->audioSplice)
#endif
        {
            liveCtx->videoSplice = false;
            liveCtx->audioSplice = false;
            BKNI_SetEvent(liveCtx->spliceOutEvent);
        }
        BKNI_ReleaseMutex(liveCtx->mutex);
        break;
    case NEXUS_DecoderSpliceState_eFoundStartPts:
        /* we found main start pts and signalling to cleanup or set the next splice point */
        BKNI_AcquireMutex(liveCtx->mutex);
        liveCtx->videoSplice = true;
#if AUDIO_ENABLE
        if(liveCtx->audioSplice)
#endif
        {
            liveCtx->videoSplice = false;
            liveCtx->audioSplice = false;
            BKNI_SetEvent(liveCtx->spliceInEvent);
        }
        BKNI_ReleaseMutex(liveCtx->mutex);
        break;
    default:
        break;
    }
    return;
}

/*
 *  prepares a generic BTP
 */
void prepare_btp (uint8_t *buf, uint16_t pid, const uint32_t * params)
{
    int             i;
    buf[0] = 0x47;
    buf[1] = (pid >> 8) & 0x1f;
    buf[2] = pid & 0xff;        /* PID */
    buf[3] = 0x20;              /* not scrambled, adaptation_field and no payload, 0 continuity counter */
    buf[4] = 0xb7;              /* adaptation field length is 183 - the remainder of the packet */
    buf[5] = 0x82;
    buf[6] = 45;                /* Number of relevant bytes */
    buf[7] = 0x00;              /* Align byte */
    buf[8] = 0x42;              /* B */
    buf[9] = 0x52;              /* R */
    buf[10] = 0x43;             /* C */
    buf[11] = 0x4d;             /* M */
    for (i = 0; i < 10; ++i)
    {
        int             base = 12 + i * 4;
        buf[base] = (uint8_t) ((params[i] & 0xff000000) >> 24);
        buf[base + 1] = (uint8_t) ((params[i] & 0x00ff0000) >> 16);
        buf[base + 2] = (uint8_t) ((params[i] & 0x0000ff00) >> 8);
        buf[base + 3] = (uint8_t) (params[i] & 0x000000ff);
    }
    for (i = 52; i < 188; ++i)
    {
        buf[i] = 0x00;
    }
    return;
}

/*
 * Prepares splice specific BTP
 */
int prepare_splice_btp(btp_input *input)
{
    uint32_t params[10];
    switch (input->cmd){
    case btp_cmd_splice_stc_offset_marker:
        params[0] = 0x0B;
        params[1] = 0;
        params[9] = input->sub_cmd;
        break;
    case btp_cmd_splice_start_marker:
        params[0] = 0x12;
        params[1] = 0;
        params[9] = 0;
        break;
    case btp_cmd_splice_stop_marker:
        params[0] = 0x13;
        params[1] = 0;
        params[9] = 0;
        break;
    case btp_cmd_splice_transition_marker:
        /*
         * transistion types 1. liveToDisk 2. DiskToLive 3. DiskToDisk
         */
        params[0] = 0x14;
        params[1] = input->sub_cmd;
        params[9] = 0;
        break;
    case btp_cmd_splice_pcr_offset_marker:
        params[0] = 0x15;
        params[1] = 0;
        params[9] = input->sub_cmd;
        break;
    case btp_cmd_splice_inline_flush:
        /*
         * Inline flush during a playback to live case.
         * Decoder is forced to start from next available I frame
         */
        params[0] = 0x0A;
        params[1] = 0;
        params[9] = 0;
        break;
    default:
        BDBG_WRN(("invalid BTP packet type"));
        return -1;
        break;
    }
    params[2] = 0;
    params[3] = 0;
    params[4] = 0;
    params[5] = 0;
    params[6] = 0;
    params[7] = 0;
    params[8] = 188; /* packet length */
    prepare_btp(input->pBuffer, input->pid, params);
    return 0;

}

int send_splice_btp(adContext *adCtx,btp_input *btp_in)
{
    void *buffer;
    size_t buffer_size;
    NEXUS_Error rc;
getBuffer:
    rc = NEXUS_Playpump_GetBuffer(adCtx->playpump, &buffer, &buffer_size);
    BDBG_ASSERT(!rc);
    if(!buffer_size) {
        BKNI_WaitForEvent(adCtx->playpumpEvent, 0xffffffff);
        goto getBuffer;
    }
    BKNI_Memcpy(buffer,btp_in->pBuffer,188);
    rc = NEXUS_Playpump_ReadComplete(adCtx->playpump, 0, 188);
    BDBG_ASSERT(!rc);
    return 0;

}

int pumpAd(liveContext *liveCtx)
{
    void *buffer;
    size_t buffer_size;
    int n;
    NEXUS_Error rc;
    adContext *adCtx  = &liveCtx->adCtx;
    while (1)
    {
getBuffer:
        rc = NEXUS_Playpump_GetBuffer(adCtx->playpump, &buffer, &buffer_size);
        BDBG_ASSERT(!rc);
        if (buffer_size == 0) {
          BKNI_WaitForEvent(adCtx->playpumpEvent, 0xffffffff);
          goto getBuffer;
        }

        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        n = fread(buffer, 1, buffer_size, adInfo[adCtx->adIndex].file);
        BDBG_ASSERT(n >= 0);
        if (n == 0) {
            BDBG_MSG(("reached end of %s playback",adInfo[adCtx->adIndex].adFileName));
            return 0;
        }
        else {
            rc = NEXUS_Playpump_ReadComplete(adCtx->playpump, 0, n);
            BDBG_ASSERT(!rc);
        }
    }
}

static void * spliceThread(void *ctx)
{
    liveContext *liveCtx = (liveContext *)ctx;
    adContext *adCtx = (adContext *) &liveCtx->adCtx;
    NEXUS_VideoDecoderSpliceStatus videoSpliceStatus;
    NEXUS_PlaypumpStatus playpumpStatus;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    uint32_t stc;
    #if AUDIO_ENABLE
    uint32_t avPtsDiff=0;
    #endif
    NEXUS_Error rc;
    NEXUS_VideoDecoderSpliceStartFlowSettings videoStartFlowSettings;
    NEXUS_VideoDecoderSpliceSettings videoSpliceSettings;
    #if AUDIO_ENABLE
    NEXUS_AudioDecoderSpliceStatus audioSpliceStatus;
    NEXUS_AudioDecoderStatus audioDecoderStatus;
    NEXUS_AudioDecoderSpliceStartFlowSettings audioStartFlowSettings;
    NEXUS_AudioDecoderSpliceSettings audioSpliceSettings;
    #endif

    while(1)
    {
        BKNI_WaitForEvent(liveCtx->spliceOutEvent, 0xffffffff);
        BKNI_ResetEvent(liveCtx->spliceOutEvent);
        NEXUS_StcChannel_GetStc(liveCtx->stcChannel,&stc);
        BDBG_WRN(("stc %x",stc));
        BDBG_WRN(("spliced out of live %x ",adInfo[adCtx->adIndex].mainSpliceOutPts));

        NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder, &videoDecoderStatus);
        BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));

        NEXUS_VideoDecoder_SpliceStopFlow(liveCtx->videoDecoder);
        NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&videoSpliceStatus);
        BDBG_WRN(("%s: video spliceStatus %s pts %08x",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state], videoSpliceStatus.pts));

        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetStatus(liveCtx->audioDecoder,&audioDecoderStatus);
        NEXUS_AudioDecoder_SpliceStopFlow(liveCtx->audioDecoder);
        NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        BSTD_UNUSED(avPtsDiff); /* suppress compiler warning */
        avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
        BDBG_WRN((" avPtsDiff %d",avPtsDiff));
        #endif

        NEXUS_Playpump_Start(adCtx->playpump);
        liveCtx->videoProgram.pidChannel = adCtx->videoPidChannel;
#if (START_IN_AD == 1)
        NEXUS_VideoDecoder_Flush(liveCtx->videoDecoder);

        NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder, &videoDecoderStatus);
        BDBG_WRN(("Fifo %d",videoDecoderStatus.fifoDepth));
#endif
        NEXUS_VideoDecoder_GetDefaultSpliceStartFlowSettings(&videoStartFlowSettings);
        videoStartFlowSettings.pidChannel = adCtx->videoPidChannel;
        rc = NEXUS_VideoDecoder_SpliceStartFlow(liveCtx->videoDecoder, &videoStartFlowSettings);
        BDBG_ASSERT(!rc);

        NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&videoSpliceStatus);
        BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));

        #if AUDIO_ENABLE
#if (START_IN_AD == 1)
        NEXUS_AudioDecoder_Flush(liveCtx->audioDecoder);
#endif
        liveCtx->audioProgram.pidChannel = adCtx->audioPidChannel;
        NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings(&audioStartFlowSettings);
        audioStartFlowSettings.pidChannel = adCtx->audioPidChannel;
        rc = NEXUS_AudioDecoder_SpliceStartFlow(liveCtx->audioDecoder, &audioStartFlowSettings);
        NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        #endif

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x1;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
        adCtx->btp_in.sub_cmd = adInfo[adCtx->adIndex].adPtsStart - (adInfo[adCtx->adIndex].mainSpliceOutPts);
        BDBG_WRN(("pcr offset %x", adCtx->btp_in.sub_cmd));
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_start_marker;
        adCtx->btp_in.sub_cmd = 0x0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        #if AUDIO_ENABLE
        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x3;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
        adCtx->btp_in.sub_cmd = adInfo[adCtx->adIndex].adPtsStart - (adInfo[adCtx->adIndex].mainSpliceOutPts);
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_start_marker;
        adCtx->btp_in.sub_cmd = 0x0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);
        #endif
        BDBG_WRN(("starting ad : %s",adInfo[adCtx->adIndex].adFileName));
        adInfo[adCtx->adIndex].file = fopen(adInfo[adCtx->adIndex].adFileName, "rb");
        BDBG_ASSERT((adInfo[adCtx->adIndex].file));
#if (START_IN_AD == 1)
        adPosition adPos;
        rc = computeAdOffset(liveCtx, stc , &adPos);
        BDBG_ASSERT(!rc);
        liveCtx->adCtx.fileOffset = adPos.fileOffset;
        if (adCtx->fileOffset) {
            int ret = fseek(adInfo[adCtx->adIndex].file, adCtx->fileOffset, SEEK_SET);
            BDBG_ASSERT(0 == ret);
            adCtx->fileOffset = 0;
        }
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_VideoDecoder_GetSettings(liveCtx->videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.freeze = false;
        NEXUS_VideoDecoder_SetSettings(liveCtx->videoDecoder, &videoDecoderSettings);

        #if AUDIO_ENABLE
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(liveCtx->audioDecoder, &audioDecoderSettings);
        audioDecoderSettings.muted = false;
        rc = NEXUS_AudioDecoder_SetSettings(liveCtx->audioDecoder, &audioDecoderSettings);
        BDBG_ASSERT(!rc);
        #endif
#endif
        pumpAd(liveCtx);
        fclose(adInfo[adCtx->adIndex].file);

    checkBacktoBackAd:
        if (((adCtx->adIndex + 1) <= (numAdSlots-1)) && adInfo[adCtx->adIndex].mainSpliceInPts == adInfo[adCtx->adIndex + 1].mainSpliceOutPts)
        {

            adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
            adCtx->btp_in.sub_cmd = 0;
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(adCtx,&adCtx->btp_in);

            #if AUDIO_ENABLE
            adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
            adCtx->btp_in.sub_cmd = 0;
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(adCtx,&adCtx->btp_in);
            #endif

            NEXUS_Playpump_GetStatus(adCtx->playpump,&playpumpStatus);
            while(playpumpStatus.fifoDepth)
            {
                NEXUS_Playpump_GetStatus(adCtx->playpump,&playpumpStatus);
                BKNI_Sleep(5);
            }
            NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder, &videoDecoderStatus);
            BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));
            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetStatus(liveCtx->audioDecoder,&audioDecoderStatus);
            BDBG_WRN(("audioDecoder.ptsStcDifference %d",audioDecoderStatus.ptsStcDifference));
            avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
            BDBG_WRN(("avPtsDiff %d",avPtsDiff));
            #endif

            #if AUDIO_ENABLE
            if (adInfo[adCtx->adIndex].aPid != adInfo[adCtx->adIndex + 1].aPid)
            {
                rc = NEXUS_PidChannel_ChangePid(liveCtx->audioPidChannel,adInfo[adCtx->adIndex+1].aPid);
                BDBG_ASSERT(!rc);
            }
            #endif
            if (adInfo[adCtx->adIndex].vPid != adInfo[adCtx->adIndex + 1].vPid)
            {
                rc = NEXUS_PidChannel_ChangePid(liveCtx->videoPidChannel,adInfo[adCtx->adIndex+1].vPid);
                BDBG_ASSERT(!rc);
            }
            adCtx->adIndex+=1;

            adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
            adCtx->btp_in.sub_cmd = adInfo[adCtx->adIndex].adPtsStart - (adInfo[adCtx->adIndex].mainSpliceOutPts);
            BDBG_WRN(("pcr offset %x", adCtx->btp_in.sub_cmd));
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(adCtx,&adCtx->btp_in);

            #if AUDIO_ENABLE
            adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
            adCtx->btp_in.sub_cmd = adInfo[adCtx->adIndex].adPtsStart - (adInfo[adCtx->adIndex].mainSpliceOutPts);
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(adCtx,&adCtx->btp_in);
            #endif

            BDBG_WRN(("starting back to back ad : %s",adInfo[adCtx->adIndex].adFileName));
            adInfo[adCtx->adIndex].file = fopen(adInfo[adCtx->adIndex].adFileName, "rb");
            BDBG_ASSERT((adInfo[adCtx->adIndex].file));
            pumpAd(liveCtx);
            fclose(adInfo[adCtx->adIndex].file);
            goto checkBacktoBackAd;
        }

        adCtx->btp_in.cmd = btp_cmd_splice_stop_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x3;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_inline_flush;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        #if AUDIO_ENABLE
        adCtx->btp_in.cmd = btp_cmd_splice_stop_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x3;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pcr_offset_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(adCtx,&adCtx->btp_in);
        #endif

        NEXUS_Playpump_GetStatus(adCtx->playpump,&playpumpStatus);
        while(playpumpStatus.fifoDepth)
        {
            NEXUS_Playpump_GetStatus(adCtx->playpump,&playpumpStatus);
            BKNI_Sleep(5);
        }
checkVideoDepth:
        NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder, &videoDecoderStatus);
        if (videoDecoderStatus.fifoDepth*100/videoDecoderStatus.fifoSize > 10) {
            BKNI_Sleep(10);
            goto checkVideoDepth;
        }
#if AUDIO_ENABLE
checkAudioDepth:
       NEXUS_AudioDecoder_GetStatus(liveCtx->audioDecoder,&audioDecoderStatus);
       if (audioDecoderStatus.fifoDepth*100/audioDecoderStatus.fifoSize > 10) {
            BKNI_Sleep(10);
            goto checkAudioDepth;
       }
#endif

        BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));
        #if AUDIO_ENABLE
        BDBG_WRN(("audioDecoder.ptsStcDifference %d",audioDecoderStatus.ptsStcDifference));
        avPtsDiff = audioDecoderStatus.pts-videoDecoderStatus.pts;
        BDBG_WRN(("avPtsDiff %d",avPtsDiff));
        #endif
        NEXUS_VideoDecoder_SpliceStopFlow(liveCtx->videoDecoder);
        NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&videoSpliceStatus);
        BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));
        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_SpliceStopFlow(liveCtx->audioDecoder);
        NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        #endif

        NEXUS_VideoDecoder_GetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
        videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStartAtPts;
        videoSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceInPts;
        videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
        videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
        videoSpliceSettings.splicePoint.context = (void*)liveCtx;
        rc = NEXUS_VideoDecoder_SetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
        BDBG_ASSERT(!rc);
        NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder, &videoSpliceStatus);
        BDBG_ASSERT(!rc);
        BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));

        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
        audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStartAtPts;
        audioSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceInPts - AUDIO_SPLICE_LAG;
        audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
        audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
        audioSpliceSettings.splicePoint.context = (void*)liveCtx;
        NEXUS_AudioDecoder_SetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
        BDBG_ASSERT(!rc);
        NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder, &audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        #endif

        liveCtx->videoProgram.pidChannel = liveCtx->videoPidChannel;
        NEXUS_VideoDecoder_GetDefaultSpliceStartFlowSettings(&videoStartFlowSettings);
        videoStartFlowSettings.pidChannel = liveCtx->videoPidChannel;
        rc = NEXUS_VideoDecoder_SpliceStartFlow(liveCtx->videoDecoder, &videoStartFlowSettings);
        BDBG_ASSERT(!rc);

        #if AUDIO_ENABLE
        liveCtx->audioProgram.pidChannel = liveCtx->audioPidChannel;
        NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings(&audioStartFlowSettings);
        audioStartFlowSettings.pidChannel = liveCtx->audioPidChannel;
        rc = NEXUS_AudioDecoder_SpliceStartFlow(liveCtx->audioDecoder, &audioStartFlowSettings);
        BDBG_ASSERT(!rc);
        #endif

        NEXUS_Playpump_Stop(adCtx->playpump);
        BKNI_WaitForEvent(liveCtx->spliceInEvent, 0xffffffff);
        BKNI_ResetEvent(liveCtx->spliceInEvent);
        BDBG_WRN(("spliced back into live %x ",adInfo[adCtx->adIndex].mainSpliceInPts));
        NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder, &videoDecoderStatus);
        BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));
        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetStatus(liveCtx->audioDecoder,&audioDecoderStatus);
        BDBG_WRN(("audioDecoder.ptsStcDifference %d",audioDecoderStatus.ptsStcDifference));
        avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
        BDBG_WRN(("avPtsDiff %d",avPtsDiff));
        #endif
#if (START_IN_AD == 1)
        videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
        rc = NEXUS_VideoDecoder_SetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
        BDBG_ASSERT(!rc);
        NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder, &videoSpliceStatus);
        BDBG_WRN(("%s: video insertionStatus %s", BSTD_FUNCTION, adInsertionStateText[videoSpliceStatus.state]));

#if AUDIO_ENABLE
        audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
        rc = NEXUS_AudioDecoder_SetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
        BDBG_ASSERT(!rc);
        NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
#endif
        BDBG_WRN(("Sending demoDone event"));
        BKNI_AcquireMutex(liveCtx->mutex);
        BKNI_SetEvent(liveCtx->demoDone);
        BKNI_ReleaseMutex(liveCtx->mutex);
        break;
#else
        if (adCtx->adIndex >= (numAdSlots-1)) {
            videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
            rc = NEXUS_VideoDecoder_SetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
            BDBG_ASSERT(!rc);
            NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder, &videoSpliceStatus);
            BDBG_WRN(("%s: video insertionStatus %s", BSTD_FUNCTION, adInsertionStateText[videoSpliceStatus.state]));

            #if AUDIO_ENABLE
            audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
            rc = NEXUS_AudioDecoder_SetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
            BDBG_ASSERT(!rc);
            NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
            BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
            #endif
            BDBG_WRN(("Sending demoDone event"));
            BKNI_AcquireMutex(liveCtx->mutex);
            BKNI_SetEvent(liveCtx->demoDone);
            BKNI_ReleaseMutex(liveCtx->mutex);
            break;

        }
        else
        {
            adCtx->adIndex++;
            NEXUS_VideoDecoder_GetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
            videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
            videoSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceOutPts;
            videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
            videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
            videoSpliceSettings.splicePoint.context = (void*)liveCtx;
            rc = NEXUS_VideoDecoder_SetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
            BDBG_ASSERT(!rc);
            NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&videoSpliceStatus);
            BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));

            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
            audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
            audioSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceOutPts - AUDIO_SPLICE_LAG;
            audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
            audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
            audioSpliceSettings.splicePoint.context = (void*)liveCtx;
            rc = NEXUS_AudioDecoder_SetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
            BDBG_ASSERT(!rc);
            NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder,&audioSpliceStatus);
            BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
            #endif
        }
#endif
    }


    return NULL;
}


static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_complete_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    BDBG_WRN(("cppm_complete_callback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_callback(void *context, int param)
{
    NEXUS_Error rc;
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle) context;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    BSTD_UNUSED(param);

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        BDBG_WRN(("Unable to retrieve frontend device handle.\n"));
    }

    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = true;
    calibrateSettings.cppm.threshold = 250;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) {
        BDBG_WRN(("NEXUS_FrontendDevice_Recalibrate exited with rc = %d", rc));
    }
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#if 0
static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent);

static void WaitForAd(liveContext *liveCtx, uint32_t wstc)
{
    while (true) {
        uint32_t stc;
        NEXUS_StcChannel_GetStc(liveCtx->stcChannel, &stc);
        BDBG_WRN(("STC %08x", stc));
        if (stc >= wstc) {
            break;
        } else {
            BKNI_Sleep(30);
        }
    }
}
#endif

static NEXUS_Error StartMainProgram(liveContext *liveCtx)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    NEXUS_VideoDecoderSpliceSettings videoSpliceSettings;
    NEXUS_VideoDecoderSpliceStatus videoSpliceStatus;
#if AUDIO_ENABLE
    uint32_t avPtsDiff;
    NEXUS_AudioDecoderStatus audioDecoderStatus;
    NEXUS_AudioDecoderSpliceSettings audioSpliceSettings;
    NEXUS_AudioDecoderSpliceStatus audioSpliceStatus;
#endif

    NEXUS_VideoDecoder_GetSettings(liveCtx->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.mute = false;
    NEXUS_VideoDecoder_SetSettings(liveCtx->videoDecoder, &videoDecoderSettings);
    rc = NEXUS_VideoDecoder_Start(liveCtx->videoDecoder, &liveCtx->videoProgram);
    if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
#if AUDIO_ENABLE
    rc = NEXUS_AudioDecoder_Start(liveCtx->audioDecoder, &liveCtx->audioProgram);
    if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
#endif

    if (!NEXUS_GetEnv("force_vsync"))
    {
    checkPts:
        liveCtx->adCtx.adIndex = 0;
        NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder,&videoDecoderStatus);
        if (videoDecoderStatus.firstPtsPassed && videoDecoderStatus.ptsType == NEXUS_PtsType_eCoded)
        {
            int i=0;
            BDBG_WRN(("videoDecoderStatus.pts %x",videoDecoderStatus.pts));
            for (i=0;i<numAdSlots;i++) {
                if (videoDecoderStatus.pts <= adInfo[i].mainSpliceOutPts)
                {
                    liveCtx->adCtx.adIndex = i;
                    break;
                }
            }

            if (i == numAdSlots)
            {
                BDBG_WRN(("no ad replacement .. exit"));
                rc = NEXUS_NOT_SUPPORTED;
                goto ExitFunc;
            }
            else
            {
                BDBG_WRN(("Start ad Index %u",liveCtx->adCtx.adIndex));
            }
        }
        else
        {
            BKNI_Sleep(10);
            goto checkPts;
        }
    }

    BDBG_ERR(("Live start"));
#if (START_IN_AD == 1)
    fprintf(stderr, "Press enter to start ad\n");
    getchar();

    NEXUS_VideoDecoder_Stop(liveCtx->videoDecoder);
#if AUDIO_ENABLE
    NEXUS_AudioDecoder_Stop(liveCtx->audioDecoder);
#endif
    BKNI_Sleep(1000);
    BDBG_ERR(("Ad start"));
    rc = NEXUS_VideoDecoder_Start(liveCtx->videoDecoder, &liveCtx->videoProgram);
    if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
#if AUDIO_ENABLE
    rc = NEXUS_AudioDecoder_Start(liveCtx->audioDecoder, &liveCtx->audioProgram);
    if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
#endif

    NEXUS_VideoDecoder_GetSettings(liveCtx->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.freeze = true;
    rc = NEXUS_VideoDecoder_SetSettings(liveCtx->videoDecoder, &videoDecoderSettings);
    if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
#if AUDIO_ENABLE
    {
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(liveCtx->audioDecoder, &audioDecoderSettings);
        audioDecoderSettings.muted = true;
        rc = NEXUS_AudioDecoder_SetSettings(liveCtx->audioDecoder, &audioDecoderSettings);
        if(rc){rc = BERR_TRACE(rc); goto ExitFunc;}
    }
#endif
#endif

#if AUDIO_ENABLE
    NEXUS_VideoDecoder_GetStatus(liveCtx->videoDecoder,&videoDecoderStatus);
    NEXUS_AudioDecoder_GetStatus(liveCtx->audioDecoder,&audioDecoderStatus);
    BDBG_WRN(("audioDecoder.ptsStcDifference %d",audioDecoderStatus.ptsStcDifference));
    BSTD_UNUSED(avPtsDiff);     /* suppress compiler warning */
    avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
    BDBG_WRN(("avPtsDiff %d, pts %08x", avPtsDiff, audioDecoderStatus.pts));
#endif

    NEXUS_VideoDecoder_GetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
    videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
#if (START_IN_AD == 0)
    videoSpliceSettings.pts = adInfo[liveCtx->adCtx.adIndex].mainSpliceOutPts;
    videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
#else
    videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtFirstPts;
    videoSpliceSettings.pts = 1;
    videoSpliceSettings.ptsThreshold = 0xFFFFFFFE;
#endif
    videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
    videoSpliceSettings.splicePoint.context = (void*)liveCtx;
    rc = NEXUS_VideoDecoder_SetSpliceSettings(liveCtx->videoDecoder, &videoSpliceSettings);
    BDBG_ASSERT(!rc);
    NEXUS_VideoDecoder_GetSpliceStatus(liveCtx->videoDecoder,&videoSpliceStatus);
    BDBG_WRN(("%s: video spliceStatus  %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));

#if AUDIO_ENABLE
    NEXUS_AudioDecoder_GetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
    audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
#if (START_IN_AD == 0)
    audioSpliceSettings.pts = adInfo[liveCtx->adCtx.adIndex].mainSpliceOutPts - AUDIO_SPLICE_LAG;
    audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
#else
    audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtFirstPts;
    audioSpliceSettings.pts = 1;
    audioSpliceSettings.ptsThreshold = 0xFFFFFFFE;
#endif
    audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
    audioSpliceSettings.splicePoint.context = (void*)liveCtx;
    rc = NEXUS_AudioDecoder_SetSpliceSettings(liveCtx->audioDecoder, &audioSpliceSettings);
    BDBG_ASSERT(!rc);
    NEXUS_AudioDecoder_GetSpliceStatus(liveCtx->audioDecoder, &audioSpliceStatus);
    BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
#endif
ExitFunc:
    return rc;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window = NULL;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    NEXUS_Error rc;
    unsigned freq = 576000000;
    BKNI_EventHandle statusEvent, lockChangedEvent, cppmEvent;
    bool cppm = false;
    bool waitForCppm = false;
    unsigned  mode = 256;
    unsigned int videoCodec;
    unsigned int audioCodec;
    int videoPid, audioPid, pcrPid;
    unsigned maxAcquireTime = MAX_ACQUIRE_TIME;
    bool acquired = false;
    bool firstTune = true;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    liveContext liveCtx;
    pthread_t spliceThreadId;
    int  adIndex;
    unsigned testCase;

    if (argc < 2) {
        BDBG_WRN(("usage: live_ad_replacement #test_case (range 1..25)"));
        return 0;
    }
    testCase = atoi(argv[1])-1;
    if (testCase > MAX_TEST_CASES) {
        BDBG_WRN(("Test case range is 1...25)"));
        return 0;
    }
    else
    {
/*        BDBG_WRN(("start the QAM streaming of %s and press enter",test_cases[testCase].streamName));
          getchar(); */
        videoPid = test_cases[testCase].videoPid;
        audioPid = test_cases[testCase].audioPid;
        pcrPid = test_cases[testCase].pcrPid;
        videoCodec = test_cases[testCase].videoCodec;
        videoMpeg2 = (videoCodec == NEXUS_VideoCodec_eMpeg2)?true:false;
        audioCodec = test_cases[testCase].audioCodec;
        adInfo = test_cases[testCase].adInfo;
        numAdSlots = test_cases[testCase].numAdSlots;
    }
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.videoDecoderModuleSettings.deferInit = true;
#if NEXUS_HAS_VIDEO_ENCODER
    platformSettings.videoEncoderSettings.deferInit = true;
#endif
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&liveCtx.spliceOutEvent);
    BKNI_CreateEvent(&liveCtx.spliceInEvent);
    BKNI_CreateEvent(&liveCtx.demoDone);
    BKNI_CreateEvent(&liveCtx.adCtx.playpumpEvent);
    BKNI_CreateMutex(&liveCtx.mutex);

    liveCtx.adCtx.adIndex = 0;
    adIndex = liveCtx.adCtx.adIndex;
    liveCtx.adCtx.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Playpump_GetSettings(liveCtx.adCtx.playpump, &liveCtx.adCtx.playpumpSettings);
    liveCtx.adCtx.playpumpSettings.dataCallback.callback = playpump_data_callback;
    liveCtx.adCtx.playpumpSettings.dataCallback.context = liveCtx.adCtx.playpumpEvent;
    NEXUS_Playpump_SetSettings(liveCtx.adCtx.playpump, &liveCtx.adCtx.playpumpSettings);
    liveCtx.adCtx.videoPidChannel = NEXUS_Playpump_OpenPidChannel(liveCtx.adCtx.playpump, adInfo[adIndex].vPid, NULL);
    liveCtx.adCtx.audioPidChannel = NEXUS_Playpump_OpenPidChannel(liveCtx.adCtx.playpump, adInfo[adIndex].aPid, NULL);


    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if 0 /*NEXUS_NUM_COMPONENT_OUTPUTS*/
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    rc = NEXUS_Platform_InitFrontend();
    if(rc){rc = BERR_TRACE(rc); goto done;}

    {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_WRN(("Unable to find QAM-capable frontend"));
            return -1;
        }
    }

    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockChangedEvent);
    BKNI_CreateEvent(&cppmEvent);
    NEXUS_StcChannel_GetDefaultSettings(0, &liveCtx.stcSettings);
    liveCtx.stcSettings.timebase = NEXUS_Timebase_e0;
    liveCtx.stcChannel = NEXUS_StcChannel_Open(0, &liveCtx.stcSettings);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.spliceEnabled = true;
    liveCtx.audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(liveCtx.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(liveCtx.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(liveCtx.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.spliceEnabled = true;
    liveCtx.videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(liveCtx.videoDecoder));

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    /* Demonstrate the use of recalibration using cppm (triggered by frontend in case of signal change). */
    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = false;
    calibrateSettings.cppm.threshold = 260;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    calibrateSettings.cppm.calibrationComplete.callback = cppm_complete_callback;
    calibrateSettings.cppm.calibrationComplete.context = cppmEvent;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq;
    switch (mode)
    {
        default:
            BDBG_WRN(("Incorrect mode %d specified. Defaulting to 64(NEXUS_FrontendQamMode_e64)", mode));
        case 64:
            qamSettings.mode = NEXUS_FrontendQamMode_e64;
            qamSettings.symbolRate = 5056900;
            break;
        case 256:
            qamSettings.mode = NEXUS_FrontendQamMode_e256;
            qamSettings.symbolRate = 5360537;
            break;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_changed_callback;
    qamSettings.lockCallback.context = lockChangedEvent;
    qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    qamSettings.asyncStatusReadyCallback.context = statusEvent;
    qamSettings.autoAcquire = true;
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif)
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    liveCtx.videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
    liveCtx.audioPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
    liveCtx.pcrPidChannel = NEXUS_PidChannel_Open(parserBand, pcrPid, NULL);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&liveCtx.videoProgram);
    liveCtx.videoProgram.codec = videoCodec;
    liveCtx.videoProgram.pidChannel = liveCtx.videoPidChannel;
    liveCtx.videoProgram.stcChannel = liveCtx.stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&liveCtx.audioProgram);
    liveCtx.audioProgram.codec = audioCodec;
    liveCtx.audioProgram.pidChannel = liveCtx.audioPidChannel;
    liveCtx.audioProgram.stcChannel = liveCtx.stcChannel;
    NEXUS_StcChannel_GetSettings(liveCtx.stcChannel, &liveCtx.stcSettings);
    liveCtx.stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    liveCtx.stcSettings.modeSettings.pcr.pidChannel = liveCtx.pcrPidChannel;
    liveCtx.stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    rc = NEXUS_StcChannel_SetSettings(liveCtx.stcChannel, &liveCtx.stcSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    BKNI_ResetEvent(lockChangedEvent);

tune:
    BDBG_WRN(("tuning %d MHz... mode = %d", freq/1000000, qamSettings.mode));
    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    {
        unsigned start_time = b_get_time();
        while (1)
        {
            unsigned current_time = b_get_time();
            NEXUS_FrontendFastStatus status;
            if (current_time - start_time > maxAcquireTime)
            {
                BDBG_WRN(("application timeout. cannot acquire."));
                break;
            }
            rc = BKNI_WaitForEvent(lockChangedEvent, maxAcquireTime - (current_time - start_time));
            if (rc == BERR_TIMEOUT)
            {
                BDBG_WRN(("application timeout. cannot acquire."));
                break;
            }
            BDBG_ASSERT(!rc);
            rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
            if (rc == NEXUS_SUCCESS)
            {
                if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
                {
                    BDBG_WRN(("frontend locked"));
                    acquired = true;
                    break;
                }
                else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
                {
                    BDBG_WRN(("frontend unlocked (acquireInProgress=%d)", status.acquireInProgress));
                     /* Wait for maxAcquireTime when unlocked*/
                }
                else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
                {
                    if(!qamSettings.autoAcquire)
                    {
                        BDBG_WRN(("No signal at the tuned frequency."));
                        break;
                    }
                    else
                    {
                        BDBG_WRN(("No signal at the tuned frequency. Waiting till the application timesout to allow auto acquire to try again."));
                    }
                }
                else
                {
                    BDBG_WRN(("unknown status: %d", status.lockStatus));
                }
            }
            else if (rc == NEXUS_NOT_SUPPORTED)
            {
                NEXUS_FrontendQamStatus qamStatus;
                rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
                if (rc) {rc = BERR_TRACE(rc); return false;}
                BDBG_WRN(("frontend %s (fecLock=%d)", qamStatus.receiverLock?"locked":"unlocked", qamStatus.fecLock));
                if (qamStatus.receiverLock)
                {
                    acquired = true;
                    break;
                }
                    /* we can't conclude no lock until application timeout */
            }
            else
            {
                BERR_TRACE(rc);
            }
        }
    }

    if (acquired)
    {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
#if AUDIO_ENABLE
        NEXUS_AudioDecoderSettings audioDecoderSettings;
#endif
        NEXUS_VideoDecoder_GetSettings(liveCtx.videoDecoder, &videoDecoderSettings);
        BDBG_WRN(("default video discard threshold %d",videoDecoderSettings.discardThreshold));
        /*videoDecoderSettings.ptsOffset = 1800;*/
        videoDecoderSettings.discardThreshold = 4*45000; /* 4 seconds*/
        NEXUS_VideoDecoder_SetSettings(liveCtx.videoDecoder, &videoDecoderSettings);
        BDBG_WRN(("video discard threshold %d",videoDecoderSettings.discardThreshold));
#if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetSettings(liveCtx.audioDecoder, &audioDecoderSettings);
        BDBG_WRN(("default audio discard threshold %d wideGaThreshold %s gaThreshold %d",audioDecoderSettings.discardThreshold,audioDecoderSettings.wideGaThreshold?"true":"false",audioDecoderSettings.gaThreshold));
        audioDecoderSettings.discardThreshold = 6000; /* 6 seconds */
        /*audioDecoderSettings.ptsOffset = 1800;*/
        NEXUS_AudioDecoder_SetSettings(liveCtx.audioDecoder, &audioDecoderSettings);
        BDBG_WRN(("audio discard threshold %d wideGaThreshold %s gaThreshold %d",audioDecoderSettings.discardThreshold,audioDecoderSettings.wideGaThreshold?"true":"false",audioDecoderSettings.gaThreshold));
#endif
        BKNI_Memset(&liveCtx.adCtx.btp_in.pBuffer,0,sizeof(liveCtx.adCtx.btp_in.pBuffer));
        liveCtx.adCtx.btp_in.pBuffer = (uint8_t *)BKNI_Malloc(188);
        rc = pthread_create(&spliceThreadId, NULL, spliceThread, (void *)&liveCtx);

        rc = StartMainProgram(&liveCtx);
        if (NEXUS_SUCCESS != rc) {
            rc = BERR_TRACE(rc);
            goto noAdReplacement;
        }

#if AUDIO_ENABLE
        liveCtx.audioSplice = false;
#endif
        liveCtx.videoSplice = false;
    }
    else
    {
        if (firstTune)
        {
            /* trigger CPPM due to lack of signal */
            firstTune = false;
            if (cppm)
            {
                calibrateSettings.cppm.enabled = true;
                rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
                if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);
                if (cppm && waitForCppm)
                {
                   BDBG_WRN(("Waiting for CPPM to complete..."));
                   BKNI_WaitForEvent(cppmEvent, 5000);
                }
                    goto tune;
            }
        }
        BDBG_WRN(("not starting decode because frontend not acquired"));
        goto done;
    }

    BDBG_WRN(("waiting for demoDone event"));
    BKNI_WaitForEvent(liveCtx.demoDone, 0xffffffff);
    BDBG_WRN(("Press enter to quit the live decode after ad replacement"));
    getchar();
noAdReplacement:
    BKNI_Free(liveCtx.adCtx.btp_in.pBuffer);
    pthread_join(spliceThreadId,NULL);
    #if AUDIO_ENABLE
    NEXUS_AudioDecoder_Stop(liveCtx.audioDecoder);
    #endif
    NEXUS_VideoDecoder_Stop(liveCtx.videoDecoder);
    NEXUS_PidChannel_Close(liveCtx.videoPidChannel);
    NEXUS_PidChannel_Close(liveCtx.audioPidChannel);
    NEXUS_PidChannel_Close(liveCtx.pcrPidChannel);

done:
    if(window)NEXUS_VideoWindow_RemoveAllInputs(window);
    if(window)NEXUS_VideoWindow_Close(window);
    if(display)NEXUS_Display_Close(display);
    if(liveCtx.videoDecoder)NEXUS_VideoDecoder_Close(liveCtx.videoDecoder);
    if(liveCtx.audioDecoder)NEXUS_AudioDecoder_Close(liveCtx.audioDecoder);
    if(statusEvent)BKNI_DestroyEvent(statusEvent);
    if(lockChangedEvent)BKNI_DestroyEvent(lockChangedEvent);
    if(cppmEvent)BKNI_DestroyEvent(cppmEvent);
    BKNI_DestroyEvent(liveCtx.spliceOutEvent);
    BKNI_DestroyEvent(liveCtx.spliceInEvent);
    BKNI_DestroyEvent(liveCtx.demoDone);
    BKNI_DestroyEvent(liveCtx.adCtx.playpumpEvent);
    BKNI_DestroyMutex(liveCtx.mutex);
    NEXUS_Playpump_CloseAllPidChannels(liveCtx.adCtx.playpump);
    NEXUS_Playpump_Close(liveCtx.adCtx.playpump);
    NEXUS_Platform_Uninit();
    return 0;
}

#if 0
static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent)
{
    NEXUS_FrontendQamStatus qamStatus;
    int rc;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceStatus deviceStatus;

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    rc = NEXUS_Frontend_RequestQamAsyncStatus(frontend);
    if(rc == NEXUS_SUCCESS){
        rc = BKNI_WaitForEvent(statusEvent, 1000);
        if (rc) {
            printf("Status not returned\n");
            return BERR_TRACE(rc);
        }
        NEXUS_Frontend_GetQamAsyncStatus(frontend , &qamStatus);

        if (deviceHandle) {
            rc = NEXUS_FrontendDevice_GetStatus(deviceHandle, &deviceStatus);
            if(rc) return BERR_TRACE(rc);
        }
    }
    else if(rc == NEXUS_NOT_SUPPORTED){
        rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        return BERR_TRACE(rc);
    }

    printf("\nDownstream lock = %d\n", qamStatus.fecLock);
    printf("Frequency = %d\n", qamStatus.settings.frequency);
    if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e64)
        printf("Mode = NEXUS_FrontendQamMode_e64\n");
    else if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e256)
        printf("Mode  = NEXUS_FrontendQamMode_e256\n");
    else
        printf("Mode = %d\n", qamStatus.settings.mode);
    if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eA)
        printf("Annex = NEXUS_FrontendQamAnnex_eA\n");
    else if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eB)
        printf("Annex  = NEXUS_FrontendQamAnnex_eB\n");
    else
        printf("Annex = %d\n", qamStatus.settings.annex);
    printf("Symbol rate = %d\n", qamStatus.symbolRate);
    printf("Snr estimate = %d\n", qamStatus.snrEstimate/100 );
    printf("FecCorrected = %d\n", qamStatus.fecCorrected);
    printf("FecUncorrected = %d\n", qamStatus.fecUncorrected);
    printf("DsChannelPower in dBmV = %d\n", qamStatus.dsChannelPower/10);
    printf("DsChannelPower in dBm = %d\n", qamStatus.dsChannelPower/10 - 48);
    if (deviceHandle) {
        printf("AVS enabled = %d\n", deviceStatus.avs.enabled);
        printf("AVS voltage = %d\n", deviceStatus.avs.voltage);
        printf("Device temperature = %d\n", deviceStatus.temperature);
    }
    return 0;
}
#endif
#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
