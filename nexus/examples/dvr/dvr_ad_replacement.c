/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_playpump.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_primer.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(dvr_ad_replacement);

#define MAX_TEST_CASES 22
#define MAX_ADS   8
#define MAX_READ (188*1024)
#define AUDIO_ENABLE 1
#define AUDIO_SPLICE_TOLERANCE (videoMpeg2?4500:9000)
#define VIDEO_SPLICE_TOLERANCE (videoMpeg2?4500:9000)
#define AUDIO_SPLICE_LAG       (videoMpeg2?4500:9000)
bool videoMpeg2 = false;

typedef struct adInformation
{
    unsigned mainSpliceOutFrameNo;
    uint32_t mainSpliceOutPts;
    long mainSpliceOutOffset;
    unsigned mainSpliceInFrameNo;
    uint32_t mainSpliceInPts;
    long mainSpliceInOffset;
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
    bool startWithAd;
    int numAdSlots;
    adInformation adInfo[MAX_ADS];
}streamInfo;

adInformation *adInfo;
int numAdSlots;
unsigned testCase;

/* Stream base definitions for easy relocation of streams to different mount
   point and/or local hard disk. Just move your stream directory and change
   STREAM_BASE(x) to point to a new location */
/* stream location stbsjc-aut-2.sjs.broadcom.net:/local/home/spothana/streams */
#define STREAM_BASE1 "/mnt/hd/streams/"
/* stream location stbsjc-aut-2.sjs.broadcom.net:/local/home/jxliu/streams */
#define STREAM_BASE2 "/mnt/hd/streams/"

static streamInfo test_cases[MAX_TEST_CASES] =
{
   {                            /* 1 */
        STREAM_BASE1 "adIns/big_buck_bunny_60hz_playback.ts",
        0x1001,
        0x1101,
        0x1001,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eAac,
        false,
        8,
        {
            {   3628,  0x002a34b0,  0x4D090B4,  5068, 0x003aaf70,   0x579715C, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad1_playback.ts", NULL},
            {   8781,  0x00652D5E,  0x9B10D30, 10221, 0x0075a81e,   0xA59C574, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad2_playback.ts", NULL},
            {  13973,  0x00A0984E,  0xE9FEEC4, 15413, 0x00b1130e,   0xF48E864, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad3_playback.ts", NULL},
            {  19106,  0x00DB5664, 0x139F2178, 20546, 0x00ebd124,  0x144808BC, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad4_playback.ts", NULL},
            {  24001, 0x001135B36, 0x189641A4, 25441, 0x0123d5f6,  0x193EEA7C, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad5_playback.ts", NULL},
            {  30164, 0x00159E2E0, 0x1F5BE518, 31604, 0x016a5da0,  0x2004E6CC, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad6_playback.ts", NULL},
            {  36401, 0x001A14356, 0x23587604, 37841, 0x01b1be16,  0x24011550, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad7_playback.ts", NULL},
            {  39761, 0x001C7B716, 0x2641BE1C, 41201, 0x01d831d6,  0x26EA59BC, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_60hz_ad7_playback.ts", NULL}
        }

    },
    {                           /* 2 */
        STREAM_BASE1 "adIns/big_buck_bunny_hevc_60hz_playback.ts",
        0x1001,
        0x1101,
        0x1001,
        NEXUS_VideoCodec_eH265,
        NEXUS_AudioCodec_eAac,
        false,
        8,
        {
            {   3628,  0x002a34b0, 0x00ce2720,  5068, 0x003aaf70,  0x00f444c4, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad1_playback.ts", NULL},
            {   8781,  0x00652D5E, 0x01a481D4, 10221, 0x0075a81e,  0x01c9dfcc, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad2_playback.ts", NULL},
            {  13973,  0x00A0984E, 0x027ed42c, 15413, 0x00b1130e,  0x02a40034, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad3_playback.ts", NULL},
            {  19106,  0x00DB5664, 0x03541714, 20546, 0x00ebd124,  0x0379984c, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad4_playback.ts", NULL},
            {  24001, 0x001135B36, 0x042cc328, 25441, 0x0123d5f6,  0x0451f2dc, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad5_playback.ts", NULL},
            {  30164, 0x00159E2E0, 0x05674228, 31604, 0x016a5da0,  0x058D0C14, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad6_playback.ts", NULL},
            {  36401, 0x001A14356, 0x06b98574, 37841, 0x01b1be16,  0x06dec31c, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad7_playback.ts", NULL},
            {  39761, 0x001C7B716, 0x073cf450, 41201, 0x01d831d6,  0x07624c68, 0xafc8, 0x0011279a, 0x31, 0x32,STREAM_BASE1 "adIns/brcm_ltd_hevc_60hz_ad7_playback.ts", NULL}
        }
    },
    {                           /* 3 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG2_1Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x00128996,  0x00042e2b0, 1158, 0x00204536, 0x000766810, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00204536,  0x000766810, 1658, 0x002e00d6, 0x000a9ee2c, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00497816,  0x00110fecc, 3158, 0x005733b6, 0x001448080, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005733b6,  0x001448080, 3908, 0x006bcd26, 0x00191c94c, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bcd26,  0x00191c94c, 4658, 0x00806696, 0x001df1a2c, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x00806696,  0x001df1a2c, 5408, 0x00950006, 0x0022c8aa0, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 4 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG2_1.5Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x00128997,  0x0005c309c, 1158, 0x00204537, 0x000a33588, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00204537,  0x000a33588, 1658, 0x002e00d7, 0x000ea3494, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00497817,  0x00178444c, 3158, 0x005733b7, 0x001bf3e34, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005733b7,  0x001bf3e34, 3908, 0x006bcd27, 0x00229c18c, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bcd27,  0x00229c18c, 4658, 0x00806697, 0x00294465c, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x00806697,  0x00294465c, 5408, 0x00950007, 0x002fed630, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 5 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG2_3Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x001277ac,  0x000a9d768, 1158, 0x0020334c, 0x0012bab04, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x0020334c,  0x0012bab04, 1658, 0x002deeec, 0x001aca6b8, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x0049662c,  0x002b016dc, 3158, 0x005721cc, 0x003315e34, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x005721cc,  0x003315e34, 3908, 0x006bbb3c, 0x003f38fb8, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006bbb3c,  0x003f38fb8, 4658, 0x008054ac, 0x004b5b404, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x008054ac,  0x004b5b404, 5408, 0x0094ee1c, 0x005781600, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 6 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG2_6Mbps.ts",
        0x65,
        0x1001,
        0x65,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x001248a1,  0x00143ce0c, 1158, 0x00200441, 0x0023a3530, 0x7706, 0x0e2b9e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_1Mbps.ts", NULL},
            {   1158,  0x00200441,  0x0023a3530, 1658, 0x002dbfe1, 0x0033087c4, 0x651c, 0x0e19b4, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_3Mbps.ts", NULL},
            {   2658,  0x00493721,  0x0051df3f0, 3158, 0x0056f2c1, 0x00613e70c, 0x3611, 0x0deaa9, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_MPEG_6Mbps.ts", NULL},
            {   3158,  0x0056f2c1,  0x00613e70c, 3908, 0x006b8c31, 0x00785a850, 0x7706, 0x15096e, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_1Mbps.ts", NULL},
            {   3908,  0x006b8c31,  0x00785a850, 4658, 0x008025a1, 0x008f729b0, 0x651c, 0x14f784, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_3Mbps.ts", NULL},
            {   4658,  0x008025a1,  0x008f729b0, 5408, 0x0094bf11, 0x00a689ae8, 0x3611, 0x14c879, 0x66, 0x1000, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_MPEG_6Mbps.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 7 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_720x576i25.00_16x9_2Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   1316,  0x0012c960,  0x0007d8784, 2316,  0x00208500, 0x000dce514, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  0x000dce514, 3316,  0x002e40a0, 0x0013db7b4, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  0x001fb517c, 6316,  0x00577380, 0x0025a5d88, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  0x0025a5d88, 7816,  0x006c0cf0, 0x002e9821c, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  0x002e9821c, 9316,  0x0080a660, 0x00378fa68, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  0x00378fa68, 10816, 0x00953fd0, 0x0040784c4, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 8 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_720x576i25.00_16x9_2.5Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   1316,  0x0012c960,  0x00096a208, 2316, 0x00208500, 0x001091330, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  0x001091330, 3316, 0x002e40a0, 0x0017c5fec, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  0x002609898, 6316, 0x00577380, 0x002d2d714, 0x16698, 0x0f1b30, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  0x002d2d714, 7816, 0x006c0cf0, 0x0037e8014, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  0x0037e8014, 9316, 0x0080a660, 0x0042a95c4, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  0x0042a95c4, 10816,0x00953fd0, 0x004d5d2d0, 0x16698, 0x15f900, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 9 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_720x576i25.00_16x9_4Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   1316,  0x0012c960,  0x000e52000, 2316, 0x00208500, 0x00190c9bc, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  0x00190c9bc, 3316, 0x002e40a0, 0x0023c72bc, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  0x00393f99c, 6316, 0x00577380, 0x0043f6f34, 0x16698, 0x0de602, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  0x0043f6f34, 7816, 0x006c0cf0, 0x00540ecb4, 0x16698, 0x14fb5e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  0x00540ecb4, 9316, 0x0080a660, 0x00642ae80, 0x16698, 0x14e974, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  0x00642ae80, 10816, 0x00953fd0,0x00743e92c, 0x16698, 0x14ba69, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,  0x0,                                        "", NULL}
       }
    },
    {                           /* 10 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_720x576i25.00_16x9_6Mbps.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   1316,  0x0012c960,  0x0014d8a84, 2316, 0x00208500, 0x002457fb0, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   2316,  0x00208500,  0x002457fb0, 3316, 0x002e40a0, 0x0033d7598, 0x16698, 0x0e1d8e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   5316,  0x0049b7e0,  0x0052d639c, 6316, 0x00577380, 0x00625522c, 0x16698, 0x0de602, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_500_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {   6316,  0x00577380,  0x00625522c, 7816, 0x006c0cf0, 0x0079940d4, 0x16698, 0x14fb5e, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_1000kbps_16x9.ts", NULL},
            {   7816,  0x006c0cf0,  0x0079940d4, 9316, 0x0080a660, 0x0090d674c, 0x16698, 0x14e974, 0x811, 0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_3000kbps_16x9.ts", NULL},
            {   9316,  0x0080a660,  0x0090d674c, 10816, 0x00953fd0, 0x00a811e24, 0x16698, 0x14ba69, 0x811,0x815, STREAM_BASE1 "vm_adIns/Ad2_Org_750_SD_H264_720_576i_25fps_6000kbps_16x9.ts", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,   0x0,                                        "", NULL},
            {    0x0,        0x0,           0x0,  0x0,        0x0,         0x0,    0x0,      0x0,    0x0,   0x0,                                        "", NULL}
       }
    },
    {                           /* 11 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG_1920_1080i_25fps_16000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x0012779E, (0x00342c335 - 0x7d),  1158, 0x0020333E, (0x005c0a2d5 - 0x7d), 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x0020333E, (0x005c0a2d5 - 0x7d),  1658, 0x002DEEDE, (0x0083c1f75 - 0x7d), 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x0049661E, (0x00d341d69 - 0x7d),  3158, 0x005721BE, (0x00fb0fa89 - 0x7d), 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005721BE, (0x00fb0fa89 - 0x7d),  3908, 0x006BBB2E, (0x0136b0491 - 0x7d), 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006BBB2E, (0x0136b0491 - 0x7d),  4658, 0x0080549E, (0x017246205 - 0x7d), 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x0080549E, (0x017246205 - 0x7d),  5408, 0x0094EE0E, (0x01adff435 - 0x7d), 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 12 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG_1920_1080i_25fps_12000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x00128996,  (0x00274195d - 0x7d), 1158, 0x00204536, (0x0045606e5 - 0x7d), 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x00204536,  (0x0045606e5 - 0x7d), 1658, 0x002e00d6, (0x00635046d - 0x7d), 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x00497816,  (0x009f5337d - 0x7d), 3158, 0x005733b6, (0x00bd5c665 - 0x7d), 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005733b6,  (0x00bd5c665 - 0x7d), 3908, 0x006bcd26, (0x00ea66399 - 0x7d), 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006bcd26,  (0x00ea66399 - 0x7d), 4658, 0x00806696, (0x01175e265 - 0x7d), 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x00806696,  (0x01175e265 - 0x7d), 5408, 0x00950006, (0x01446f16d - 0x7d), 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 13 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_MPEG_1920_1080i_25fps_8000kbps_16x9.ts",
        0x1000,
        0x1001,
        0x1000,
        NEXUS_VideoCodec_eMpeg2,
        NEXUS_AudioCodec_eMpeg,
        false,
        6,
        {
            {   658,   0x00128996,  (0x001a7ec39 - 0x7d), 1158, 0x00204536, (0x002ec2aa1 - 0x7d), 0x7706, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   1158,  0x00204536,  (0x002ec2aa1 - 0x7d), 1658, 0x002e00d6, (0x0042f5e75 - 0x7d), 0x650E, 0x0e129e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   2658,  0x00497816,  (0x006b6f79d - 0x7d), 3158, 0x005733b6, (0x007fb0705 - 0x7d), 0x7701, 0x0e2496, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {   3158,  0x005733b6,  (0x007fb0705 - 0x7d), 3908, 0x006bcd26, (0x009e1d095 - 0x7d), 0x7706, 0x14fb5e, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_12000kbps_16x9.ts", NULL},
            {   3908,  0x006bcd26,  (0x009e1d095 - 0x7d), 4658, 0x00806696, (0x00bc7ef09 - 0x7d), 0x650E, 0x14e966, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_16000kbps_16x9.ts", NULL},
            {   4658,  0x00806696,  (0x00bc7ef09 - 0x7d), 5408, 0x00950006, (0x00dae7dd9 - 0x7d), 0x7706, 0x150266, 0x1000, 0x1001, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_MPEG_1920_1080i_25fps_8000kbps_16x9.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 14 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        false,
        3,
        {
            {   6316,  0x00582348,  (0x0042d3fa2 - 0x5e),  7816, 0x006CBCB8, (0x00528ca0a - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  (0x00528ca0a - 0x5e),  9316, 0x00815628, (0x00624aa5e - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628,  (0x00624aa5e - 0x5e), 10816, 0x0095EF98, (0x00721482a - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,          0x0,                 0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,          0x0,                 0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,          0x0,                 0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,          0x0,                 0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,          0x0,                 0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 15 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_6000kbps_16x9_AC3.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        false,
        3,
        {
            {   6316,  0x00582348,  (0x00a5c410e - 0x5e),  7816, 0x006CBCB8, (0x00cd004be - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  (0x00cd004be - 0x5e),  9316, 0x00815628, (0x00f457d62 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628,  (0x00f457d62 - 0x5e), 10816, 0x0095EF98, (0x011bc2ede - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {    0x0,        0x0,                   0x0,    0x0,                 0x0,         0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,    0x0,                 0x0,         0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,    0x0,                 0x0,         0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,    0x0,                 0x0,         0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,    0x0,                 0x0,         0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}

       }
    },
    {                           /* 16 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_8000kbps_16x9_AC3.ts",
        0x1e1,
        0x1e2,
        0x1e1,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        false,
        3,
        {
            {   6316,  0x00582348,  (0x00a5b9b16 - 0x5e),  7816, 0x006CBCB8, (0x00ccfd502 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_AC3.ts", NULL},
            {   7816,  0x006CBCB8,  (0x00ccfd502 - 0x5e),  9316, 0x00815628, (0x00f456df6 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_AC3.ts", NULL},
            {   9316,  0x00815628,  (0x00f456df6 - 0x5e), 10816, 0x0095EF98, (0x011bb8f82 - 0x5e), 0x16698,  0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_AC3.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 17 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        false,
        6,
        {
            {   1316,  0x001375A4,  (0x000ec02de - 0x5e),  2316, 0x00213144, (0x00198d77e - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x00213144,  (0x00198d77e - 0x5e),  3316, 0x002EECE4, (0x00246fd32 - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A6424,  (0x003a0492e - 0x5e),  6316, 0x00581FC4, (0x0044c6d8e - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00581FC4,  (0x0044c6d8e - 0x5e),  7816, 0x006CB934, (0x0054f82a2 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CB934,  (0x0054f82a2 - 0x5e),  9316, 0x008152A4, (0x00652d042 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x008152A4,  (0x00652d042 - 0x5e), 10816, 0x0095EC14, (0x00756da9e - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}

       }
    },
    {                           /* 18 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_6000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        false,
        6,
        {
            {   1316,  0x00137928,  (0x001567e22 - 0x5e),  2316, 0x002134C8, (0x002514d46 - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  (0x002514d46 - 0x5e),  3316, 0x002EF068, (0x0034d2fce - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  (0x005427b1a - 0x5e),  6316, 0x00582348, (0x0063d8966 - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  (0x0063d8966 - 0x5e),  7816, 0x006CBCB8, (0x007b4c262 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  (0x007b4c262 - 0x5e),  9316, 0x00815628, (0x0092d03be - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628,  (0x0092d03be - 0x5e), 10816, 0x0095EF98, (0x00aa5d15e - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}

       }
    },
    {                           /* 19 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        false,
        6,
        {
            {   1316,  0x00137928,  (0x002960a5e - 0x5e),  2316, 0x002134C8, (0x0047aaf5a - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  (0x0047aaf5a - 0x5e),  3316, 0x002EF068, (0x00663f59e - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  (0x00a29b0f6 - 0x5e),  6316, 0x00582348, (0x00c0e5e06 - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  (0x00c0e5e06 - 0x5e),  7816, 0x006CBCB8, (0x00ee443f2 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  (0x00ee443f2 - 0x5e),  9316, 0x00815628, (0x011bbe27e - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628,  (0x011bbe27e - 0x5e), 10816, 0x0095EF98, (0x01491967a - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}

       }
    },
    {                           /* 20 */
        STREAM_BASE1 "vm_adIns/keith_itv_single_Lossless_H264_1920_1080i_25fps_18000kbps_16x9_HEAAC96.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        false,
        6,
        {
            {   1316,  0x00137928,  (0x003d57b6e - 0x5e),  2316, 0x002134C8, (0x006a3fd9a - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   2316,  0x002134C8,  (0x006a3fd9a - 0x5e),  3316, 0x002EF068, (0x00977e0ba - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   5316,  0x004A67A8,  (0x00f0fdfea - 0x5e),  6316, 0x00582348, (0x011ded9ca - 0x5e), 0x16698, 0x0F17AC, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_500_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {   6316,  0x00582348,  (0x011ded9ca - 0x5e),  7816, 0x006CBCB8, (0x01613c4c6 - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_4000kbps_16x9_HEAAC96.ts", NULL},
            {   7816,  0x006CBCB8,  (0x01613c4c6 - 0x5e),  9316, 0x00815628, (0x01a4a206a - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_12000kbps_16x9_HEAAC96.ts", NULL},
            {   9316,  0x00815628,  (0x01a4a206a - 0x5e), 10816, 0x0095EF98, (0x01e7f4e3a - 0x5e), 0x16698, 0x15F57C, 0x810, 0x814, STREAM_BASE1 "vm_adIns/Ad2_Org_750_HD_H264_1920_1080i_25fps_16000kbps_16x9_HEAAC96.ts", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL},
            {    0x0,        0x0,                   0x0,  0x0,        0x0,                   0x0,    0x0,      0x0,    0x0,    0x0,                                        "", NULL}
       }
    },
    {                           /* 21 */
        STREAM_BASE1 "adIns/lipsync/main_playback.ts",
        0x810,
        0x814,
        0x810,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAac,
        false,
        3,
        {
            {   1536,  0x15C7C8, (0x0043b110c - 0x4c),  3072, 0x2ADfc8, (0x00873e150 - 0x4c), 0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad1_playback.ts", NULL},
            {   4608,  0x3FF7C8, (0x00cb45304 - 0x4c),  6144, 0x550fc8, (0x010eccd5c - 0x4c), 0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad2_playback.ts", NULL},
            {   7680,  0x6A27C8, (0x015291404 - 0x4c),  9216, 0x7F3fc8, (0x019616bd8 - 0x4c), 0xafc8, 0x15C0C0, 0x810, 0x814, STREAM_BASE1 "adIns/lipsync/ad3_playback.ts", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL}

       }
    },
    {                           /* 22 */
        STREAM_BASE2 "ad/Mux3_new_short.ts",
        0x100,
        0x101,
        0x100,
        NEXUS_VideoCodec_eH264,
        NEXUS_AudioCodec_eAc3,
        false,
        2,
        {
            {0, 0x1b68399, 0, 0, 0x1c43f39, 0, 0x16698, 0xf2238, 0x810, 0x814, STREAM_BASE2 "ad/20E_TC_H264_1920_1080i_25fps_10000kbps_16x9.ts", NULL},
            {0, 0x1c43f39, 0, 0, 0x1d1fad9, 32483768, 0x16698, 0xf2238, 0x810, 0x814, STREAM_BASE2 "ad/20E_TC_H264_1920_1080i_25fps_10000kbps_16x9.ts", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL},
            {   0x0,        0x0,         0x0,   0x0,      0x0,         0x0,    0x0,      0x0,   0x0,   0x0,                                      "", NULL}
        }
    }
};

typedef enum btp_cmd {
    btp_cmd_splice_start_marker,
    btp_cmd_splice_stop_marker,
    btp_cmd_splice_pcr_offset_marker,
    btp_cmd_splice_transition_marker,
    btp_cmd_splice_stc_offset_marker,
    btp_cmd_splice_inline_flush,
    btp_cmd_splice_pts_offset_marker
}btp_cmd;

typedef enum PtsMarkerControl {
    PtsMarkerControl_eDisable = 0,
    PtsMarkerControl_eBtpOffset,
    PtsMarkerControl_eAutoOffset
} PtsMarkerControl;

typedef struct btp_input {
    btp_cmd cmd;
    uint32_t sub_cmd;
    uint32_t sub_cmd1;
    uint8_t * pBuffer;
    size_t length;
    uint16_t pid;
}btp_input;

typedef struct adContext
{
    btp_input btp_in;
    int adIndex;
}adContext;

typedef struct dvrContext
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle stcPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle playpumpEvent;
    BKNI_EventHandle spliceInEvent,spliceOutEvent,dvrResume;
    BKNI_MutexHandle dataFeedMutex;
    BKNI_MutexHandle mutex;
    bool dvrPlay;
    bool audioSplice;
    bool videoSplice;
    adContext adCtx;
    FILE *file;
}dvrContext;


char adInsertionStateText[][20] =
{
    "eNone",
    "eWaitForStopPts",
    "eFoundStopPts",
    "eWaitForStartPts",
    "eFoundStartPts",
    "eMax"
};


static void playpump_data_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#if AUDIO_ENABLE
static void audio_insertion_point_callback(void *context, int param)
{
    NEXUS_AudioDecoderSpliceStatus spliceStatus;
    dvrContext *dvrCtx = (dvrContext *)context;
    BSTD_UNUSED(param);
    NEXUS_AudioDecoder_GetSpliceStatus(dvrCtx->audioDecoder,&spliceStatus);
    BDBG_WRN(("%s: insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[spliceStatus.state]));
    switch (spliceStatus.state) {
    case NEXUS_DecoderSpliceState_eFoundStopPts:
        /* we found main stop pts and signalling to start the ad */
        BKNI_AcquireMutex(dvrCtx->mutex);
        dvrCtx->audioSplice = true;
        if(dvrCtx->videoSplice) {
            dvrCtx->dvrPlay = false;
            dvrCtx->audioSplice = false;
            dvrCtx->videoSplice = false;
            BKNI_SetEvent(dvrCtx->spliceOutEvent);
        }
        BKNI_ReleaseMutex(dvrCtx->mutex);
        break;
    case NEXUS_DecoderSpliceState_eFoundStartPts:
        /* we found main start pts and signalling to cleanup or set the next splice point */
        BKNI_AcquireMutex(dvrCtx->mutex);
        dvrCtx->audioSplice = true;
        if(dvrCtx->videoSplice)
        {
            dvrCtx->audioSplice = false;
            dvrCtx->videoSplice = false;
            BKNI_SetEvent(dvrCtx->spliceInEvent);
        }
        BKNI_ReleaseMutex(dvrCtx->mutex);
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
    dvrContext *dvrCtx = (dvrContext *)context;
    BSTD_UNUSED(param);
    NEXUS_VideoDecoder_GetSpliceStatus(dvrCtx->videoDecoder,&spliceStatus);
    BDBG_ERR(("%s: insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[spliceStatus.state]));
    switch (spliceStatus.state) {
    case NEXUS_DecoderSpliceState_eFoundStopPts:
        /* we found main stop pts and signalling to start the ad */
        BKNI_AcquireMutex(dvrCtx->mutex);
        dvrCtx->videoSplice = true;
        #if AUDIO_ENABLE
        if(dvrCtx->audioSplice)
        #endif
        {
            dvrCtx->dvrPlay = false;
            dvrCtx->videoSplice = false;
            dvrCtx->audioSplice = false;
            BKNI_SetEvent(dvrCtx->spliceOutEvent);
        }
        BKNI_ReleaseMutex(dvrCtx->mutex);
        break;
    case NEXUS_DecoderSpliceState_eFoundStartPts:
        /* we found main start pts and signalling to cleanup or set the next splice point */
        BKNI_AcquireMutex(dvrCtx->mutex);
        dvrCtx->videoSplice = true;
        #if AUDIO_ENABLE
        if(dvrCtx->audioSplice)
        #endif
        {
            dvrCtx->videoSplice = false;
            dvrCtx->audioSplice = false;
            BKNI_SetEvent(dvrCtx->spliceInEvent);
        }
        BKNI_ReleaseMutex(dvrCtx->mutex);
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
    BKNI_Memset(params, 0, sizeof(params));
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
         * transistion types 1. DiskToLive 2. LiveToDisk 3. DiskToDisk
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
    case btp_cmd_splice_pts_offset_marker:
        /* pts offset adds provided offset to every PTS after this BTP */
        params[0] = 0x1D;
        params[1] = input->sub_cmd1;
        params[9] = input->sub_cmd;
        break;
    default:
        BDBG_ERR(("invalid BTP packet type"));
        return -1;
        break;
    }
    params[8] = 188; /* packet length */
    prepare_btp(input->pBuffer, input->pid, params);
    return 0;

}

int send_splice_btp(dvrContext *dvrCtx,btp_input *btp_in)
{
    void *buffer;
    size_t buffer_size;
    NEXUS_Error rc;
getBuffer:
    BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
    rc = NEXUS_Playpump_GetBuffer(dvrCtx->playpump, &buffer, &buffer_size);
    BDBG_ASSERT(!rc);
    if(!buffer_size) {
        BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
        BKNI_WaitForEvent(dvrCtx->playpumpEvent, 0xffffffff);
        goto getBuffer;
    }
    BKNI_Memcpy(buffer,btp_in->pBuffer,188);
    rc = NEXUS_Playpump_ReadComplete(dvrCtx->playpump, 0, 188);
    BDBG_ASSERT(!rc);
    BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
    return 0;

}
int pumpAd(dvrContext *dvrCtx)
{
    void *buffer;
    size_t buffer_size;
    int n;
    NEXUS_Error rc;
    adContext *adCtx = &dvrCtx->adCtx;
    BDBG_WRN(("start ad playback : %s",adInfo[adCtx->adIndex].adFileName));
    while (1) {
getBuffer:
        BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
        rc = NEXUS_Playpump_GetBuffer(dvrCtx->playpump, &buffer, &buffer_size);
        BDBG_ASSERT(!rc);
        if (buffer_size == 0)
        {
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            BKNI_WaitForEvent(dvrCtx->playpumpEvent, 0xffffffff);
            goto getBuffer;
        }
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;
        n = fread(buffer, 1, buffer_size, adInfo[adCtx->adIndex].file);
        BDBG_ASSERT(n >= 0);
        if (n == 0) {
            BDBG_WRN(("reached end of ad playback : %s",adInfo[adCtx->adIndex].adFileName));
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            return 0;
        }
        else {
            rc = NEXUS_Playpump_ReadComplete(dvrCtx->playpump, 0, n);
            BDBG_ASSERT(!rc);
        }
        BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
    }
}

int pumpDvr(dvrContext *dvrCtx)
{
    BDBG_WRN(("start dvr playback"));
    while (1)
    {
        void *buffer;
        size_t buffer_size;
        int n;
        NEXUS_Error rc;
getBuffer:
        BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
        if (!dvrCtx->dvrPlay)
        {
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            BKNI_WaitForEvent(dvrCtx->dvrResume, 0xffffffff);
            BDBG_WRN(("resuming DVR playback"));
            BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
        }
        rc = NEXUS_Playpump_GetBuffer(dvrCtx->playpump, &buffer, &buffer_size);
        BDBG_ASSERT(!rc);
        if (buffer_size == 0) {
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            BKNI_WaitForEvent(dvrCtx->playpumpEvent, 0xffffffff);
            goto getBuffer;
        }

        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;
        n = fread(buffer, 1, buffer_size, dvrCtx->file);
        BDBG_ASSERT(n >= 0);
        if (n == 0) {
            BDBG_WRN(("reached end of DVR playback:%s",test_cases[testCase].streamName));
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            return 0;
        }
        else {
            rc = NEXUS_Playpump_ReadComplete(dvrCtx->playpump, 0, n);
            BDBG_ASSERT(!rc);
        }

        BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
    }
}

static void * spliceThread(void *ctx)
{
    dvrContext *dvrCtx = (dvrContext *)ctx;
    adContext *adCtx = (adContext *) &dvrCtx->adCtx;
    NEXUS_VideoDecoderSpliceSettings videoSpliceSettings;
    NEXUS_AudioDecoderSpliceSettings audioSpliceSettings;
    NEXUS_VideoDecoderSpliceStatus videoSpliceStatus;
    NEXUS_AudioDecoderSpliceStatus audioSpliceStatus;
    NEXUS_PlaypumpStatus playpumpStatus;
    NEXUS_VideoDecoderStatus videoDecoderStatus;
    NEXUS_AudioDecoderStatus audioDecoderStatus;
    NEXUS_Error rc;
    #if AUDIO_ENABLE
    uint32_t avPtsDiff=0;
    #endif
    while(1)
    {
        BKNI_WaitForEvent(dvrCtx->spliceOutEvent, 0xffffffff);
        BKNI_ResetEvent(dvrCtx->spliceOutEvent);
        BDBG_WRN(("spliced out of dvr %x ",adInfo[adCtx->adIndex].mainSpliceOutPts));

        NEXUS_VideoDecoder_GetStatus(dvrCtx->videoDecoder, &videoDecoderStatus);
        BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));
        NEXUS_VideoDecoder_GetSpliceStatus(dvrCtx->videoDecoder,&videoSpliceStatus);
        BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));
        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetStatus(dvrCtx->audioDecoder,&audioDecoderStatus);
        NEXUS_AudioDecoder_GetSpliceStatus(dvrCtx->audioDecoder,&audioSpliceStatus);
        BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
        BDBG_WRN(("avPtsDiff %08x", avPtsDiff));
        #endif
        if (!test_cases[testCase].startWithAd) {
            rc = NEXUS_Playpump_Flush(dvrCtx->playpump);
            BDBG_ASSERT(!rc);
            rc = NEXUS_PidChannel_ChangePid(dvrCtx->videoPidChannel,adInfo[adCtx->adIndex].vPid);
            BDBG_ASSERT(!rc);
            rc = NEXUS_PidChannel_ChangePid(dvrCtx->audioPidChannel,adInfo[adCtx->adIndex].aPid);
            BDBG_ASSERT(!rc);
        }
     /* dvrCtx->stcSettings.mode = NEXUS_StcChannelMode_eHost;
        rc = NEXUS_StcChannel_SetSettings(dvrCtx->stcChannel, &(dvrCtx->stcSettings));
        BDBG_ASSERT(!rc); */

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x2;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
        adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eBtpOffset;
        adCtx->btp_in.sub_cmd = (adInfo[adCtx->adIndex].mainSpliceOutPts - adInfo[adCtx->adIndex].adPtsStart);
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_start_marker;
        adCtx->btp_in.sub_cmd = 0x0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        #if AUDIO_ENABLE
        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x2;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
        adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eBtpOffset;
        adCtx->btp_in.sub_cmd = (adInfo[adCtx->adIndex].mainSpliceOutPts - adInfo[adCtx->adIndex].adPtsStart) + avPtsDiff;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_start_marker;
        adCtx->btp_in.sub_cmd = 0x0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);
        #endif
        adInfo[adCtx->adIndex].file = fopen(adInfo[adCtx->adIndex].adFileName, "rb");
        BDBG_ASSERT((adInfo[adCtx->adIndex].file));
        pumpAd(dvrCtx);
        #if 0
        {
            unsigned stcIncrement,stcPreScale;
            unsigned speedNumerator = 1, speedDenominator= 8;
            stcIncrement = (NEXUS_NORMAL_PLAY_SPEED * speedNumerator) /speedDenominator;
            stcPreScale = NEXUS_NORMAL_PLAY_SPEED-1;
            NEXUS_StcChannel_SetRate(dvrCtx->stcChannel,stcIncrement,stcPreScale);
        }
        #endif
        fclose(adInfo[adCtx->adIndex].file);

checkBacktoBackAd:
        if (((adCtx->adIndex + 1) <= (numAdSlots-1)) && adInfo[adCtx->adIndex].mainSpliceInPts == adInfo[adCtx->adIndex + 1].mainSpliceOutPts)
        {

            adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
            adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eDisable;
            adCtx->btp_in.sub_cmd = 0;
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(dvrCtx,&adCtx->btp_in);

            #if AUDIO_ENABLE
            adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
            adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eDisable;
            adCtx->btp_in.sub_cmd = 0;
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(dvrCtx,&adCtx->btp_in);
            #endif

            NEXUS_Playpump_GetStatus(dvrCtx->playpump,&playpumpStatus);
            while(playpumpStatus.fifoDepth)
            {
                NEXUS_Playpump_GetStatus(dvrCtx->playpump,&playpumpStatus);
                BKNI_Sleep(5);
            }
            NEXUS_VideoDecoder_GetStatus(dvrCtx->videoDecoder, &videoDecoderStatus);
            BDBG_WRN(("videoDecoderStatus.ptsStcDifference %d",videoDecoderStatus.ptsStcDifference));
            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetStatus(dvrCtx->audioDecoder,&audioDecoderStatus);
            BDBG_WRN(("audioDecoder.ptsStcDifference %d",audioDecoderStatus.ptsStcDifference));
            avPtsDiff = audioDecoderStatus.pts - videoDecoderStatus.pts;
            BDBG_WRN(("avPtsDiff %08x",avPtsDiff));
            #endif

            #if AUDIO_ENABLE
            if (adInfo[adCtx->adIndex].aPid != adInfo[adCtx->adIndex + 1].aPid)
            {
                rc = NEXUS_PidChannel_ChangePid(dvrCtx->audioPidChannel,adInfo[adCtx->adIndex+1].aPid);
                BDBG_ASSERT(!rc);
            }
            #endif
            if (adInfo[adCtx->adIndex].vPid != adInfo[adCtx->adIndex + 1].vPid)
            {
                rc = NEXUS_PidChannel_ChangePid(dvrCtx->videoPidChannel,adInfo[adCtx->adIndex+1].vPid);
                BDBG_ASSERT(!rc);
            }
            adCtx->adIndex+=1;

            adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
            adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eBtpOffset;
            adCtx->btp_in.sub_cmd = (adInfo[adCtx->adIndex].mainSpliceOutPts - adInfo[adCtx->adIndex].adPtsStart);
            BDBG_WRN(("stc offset %x", adCtx->btp_in.sub_cmd));
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(dvrCtx,&adCtx->btp_in);

            #if AUDIO_ENABLE
            adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
            adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eBtpOffset;
            adCtx->btp_in.sub_cmd = (adInfo[adCtx->adIndex].mainSpliceOutPts - adInfo[adCtx->adIndex].adPtsStart) - avPtsDiff;
            adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
            prepare_splice_btp(&adCtx->btp_in);
            send_splice_btp(dvrCtx,&adCtx->btp_in);
            #endif

            BDBG_WRN(("starting back to back ad : %s",adInfo[adCtx->adIndex].adFileName));
            adInfo[adCtx->adIndex].file = fopen(adInfo[adCtx->adIndex].adFileName, "rb");
            BDBG_ASSERT((adInfo[adCtx->adIndex].file));
            pumpAd(dvrCtx);
            fclose(adInfo[adCtx->adIndex].file);
            goto checkBacktoBackAd;
        }


        adCtx->btp_in.cmd = btp_cmd_splice_stop_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x2;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
        adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eDisable;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].vPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_stop_marker;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_transition_marker;
        adCtx->btp_in.sub_cmd = 0x2;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);

        adCtx->btp_in.cmd = btp_cmd_splice_pts_offset_marker;
        adCtx->btp_in.sub_cmd1 = PtsMarkerControl_eDisable;
        adCtx->btp_in.sub_cmd = 0;
        adCtx->btp_in.pid = adInfo[adCtx->adIndex].aPid;
        prepare_splice_btp(&adCtx->btp_in);
        send_splice_btp(dvrCtx,&adCtx->btp_in);


        fseek(dvrCtx->file,adInfo[adCtx->adIndex].mainSpliceInOffset,SEEK_SET);

        BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
        rc = NEXUS_Playpump_GetStatus(dvrCtx->playpump,&playpumpStatus);
        BDBG_ASSERT(!rc);
        BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
        while(playpumpStatus.fifoDepth) {
            BKNI_AcquireMutex(dvrCtx->dataFeedMutex);
            NEXUS_Playpump_GetStatus(dvrCtx->playpump,&playpumpStatus);
            BKNI_ReleaseMutex(dvrCtx->dataFeedMutex);
            BKNI_Sleep(5);
        }
        NEXUS_VideoDecoder_GetSpliceSettings(dvrCtx->videoDecoder, &videoSpliceSettings);
        videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStartAtPts;
        videoSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceInPts;
        videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
        videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
        videoSpliceSettings.splicePoint.context = (void *)dvrCtx;
        rc = NEXUS_VideoDecoder_SetSpliceSettings(dvrCtx->videoDecoder, &videoSpliceSettings);
        BDBG_ASSERT(!rc);

        #if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetSpliceSettings(dvrCtx->audioDecoder, &audioSpliceSettings);
        audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStartAtPts;
        audioSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceInPts - AUDIO_SPLICE_LAG;
        audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
        audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
        audioSpliceSettings.splicePoint.context = (void *)dvrCtx;
        rc = NEXUS_AudioDecoder_SetSpliceSettings(dvrCtx->audioDecoder, &audioSpliceSettings);
        BDBG_ASSERT(!rc);
        #endif

        if (test_cases[testCase].startWithAd) {
            NEXUS_VideoDecoder_SpliceStopFlow(dvrCtx->videoDecoder);
            NEXUS_VideoDecoder_GetSpliceStatus(dvrCtx->videoDecoder,&videoSpliceStatus);
            BDBG_WRN(("%s: video spliceStatus %s pts %08x",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state], videoSpliceStatus.pts));

            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetStatus(dvrCtx->audioDecoder,&audioDecoderStatus);
            NEXUS_AudioDecoder_SpliceStopFlow(dvrCtx->audioDecoder);
            NEXUS_AudioDecoder_GetSpliceStatus(dvrCtx->audioDecoder,&audioSpliceStatus);
            BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
            #endif
        }
        rc = NEXUS_PidChannel_ChangePid(dvrCtx->videoPidChannel,test_cases[testCase].videoPid);
        BDBG_ASSERT(!rc);

        if (test_cases[testCase].startWithAd) {
            NEXUS_VideoDecoderSpliceStartFlowSettings videoStartFlowSettings;
            NEXUS_VideoDecoder_GetDefaultSpliceStartFlowSettings(&videoStartFlowSettings);
            videoStartFlowSettings.pidChannel = dvrCtx->videoPidChannel;
            NEXUS_VideoDecoder_SpliceStartFlow(dvrCtx->videoDecoder, &videoStartFlowSettings);
            NEXUS_VideoDecoder_GetSpliceStatus(dvrCtx->videoDecoder,&videoSpliceStatus);
            BDBG_WRN(("%s: video insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[videoSpliceStatus.state]));
        }
        #if AUDIO_ENABLE
        rc = NEXUS_PidChannel_ChangePid(dvrCtx->audioPidChannel,test_cases[testCase].audioPid);
        BDBG_ASSERT(!rc);
        if (test_cases[testCase].startWithAd) {
            NEXUS_AudioDecoderSpliceStartFlowSettings audioStartFlowSettings;
            NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings(&audioStartFlowSettings);
            audioStartFlowSettings.pidChannel = dvrCtx->audioPidChannel;
            rc = NEXUS_AudioDecoder_SpliceStartFlow(dvrCtx->audioDecoder, &audioStartFlowSettings);
            NEXUS_AudioDecoder_GetSpliceStatus(dvrCtx->audioDecoder,&audioSpliceStatus);
            BDBG_WRN(("%s: audio insertionStatus %s",BSTD_FUNCTION,adInsertionStateText[audioSpliceStatus.state]));
        }
        #endif

        BKNI_AcquireMutex(dvrCtx->mutex);
        dvrCtx->dvrPlay = true;
        BKNI_ReleaseMutex(dvrCtx->mutex);
        BKNI_SetEvent(dvrCtx->dvrResume);

        BKNI_WaitForEvent(dvrCtx->spliceInEvent, 0xffffffff);
        BKNI_ResetEvent(dvrCtx->spliceInEvent);
        BDBG_WRN(("spliced back into dvr %x ",adInfo[adCtx->adIndex].mainSpliceInPts));
        if (adCtx->adIndex >= (numAdSlots-1)) {
            NEXUS_VideoDecoder_GetSpliceSettings(dvrCtx->videoDecoder, &videoSpliceSettings);
            videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
            rc = NEXUS_VideoDecoder_SetSpliceSettings(dvrCtx->videoDecoder, &videoSpliceSettings);
            BDBG_ASSERT(!rc);
            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetSpliceSettings(dvrCtx->audioDecoder, &audioSpliceSettings);
            audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eDisabled;
            rc = NEXUS_AudioDecoder_SetSpliceSettings(dvrCtx->audioDecoder,&audioSpliceSettings);
            BDBG_ASSERT(!rc);
            #endif
            break;
        }
        else
        {
            adCtx->adIndex++;
            NEXUS_VideoDecoder_GetSpliceSettings(dvrCtx->videoDecoder, &videoSpliceSettings);
            videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
            videoSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceOutPts;
            videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
            videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
            videoSpliceSettings.splicePoint.context = (void *) dvrCtx;
            rc = NEXUS_VideoDecoder_SetSpliceSettings(dvrCtx->videoDecoder,&videoSpliceSettings);
            BDBG_ASSERT(!rc);
            #if AUDIO_ENABLE
            NEXUS_AudioDecoder_GetSpliceSettings(dvrCtx->audioDecoder, &audioSpliceSettings);
            audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
            audioSpliceSettings.pts = adInfo[adCtx->adIndex].mainSpliceOutPts - AUDIO_SPLICE_LAG;
            audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
            audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
            audioSpliceSettings.splicePoint.context = (void *) dvrCtx;
            rc = NEXUS_AudioDecoder_SetSpliceSettings(dvrCtx->audioDecoder,&audioSpliceSettings);
            BDBG_ASSERT(!rc);
            #endif
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    #if AUDIO_ENABLE
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    #endif
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    dvrContext dvrCtx;
    NEXUS_VideoDecoderSpliceSettings videoSpliceSettings;
    NEXUS_AudioDecoderSpliceSettings audioSpliceSettings;
    pthread_t spliceThreadId;
    unsigned int videoCodec;
    unsigned int audioCodec;
    int videoPid, audioPid, pcrPid;
    NEXUS_Error rc;

    if (argc < 2) {
        BDBG_WRN(("usage: dvr_ad_replacement #test_case (range 1..22)"));
        return 0;
    }
    testCase = atoi(argv[1])-1;
    if (testCase > MAX_TEST_CASES) {
        BDBG_WRN(("Test case range is 1...22)"));
        return 0;
    }
    else
    {
        BDBG_WRN(("DVR stream %s",test_cases[testCase].streamName));
        videoPid = test_cases[testCase].videoPid;
        audioPid = test_cases[testCase].audioPid;
        pcrPid = test_cases[testCase].pcrPid;
        videoCodec = test_cases[testCase].videoCodec;
        videoMpeg2 = (videoCodec == NEXUS_VideoCodec_eMpeg2)?true:false;
        audioCodec = test_cases[testCase].audioCodec;
        adInfo = test_cases[testCase].adInfo;
        numAdSlots = test_cases[testCase].numAdSlots;
    }

    BKNI_Memset(&dvrCtx,0,sizeof(dvrCtx));

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&dvrCtx.playpumpEvent);
    BKNI_CreateEvent(&dvrCtx.spliceOutEvent);
    BKNI_CreateEvent(&dvrCtx.spliceInEvent);
    BKNI_CreateEvent(&dvrCtx.dvrResume);
    BKNI_CreateMutex(&dvrCtx.mutex);
    BKNI_CreateMutex(&dvrCtx.dataFeedMutex);

    dvrCtx.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Playpump_GetSettings(dvrCtx.playpump, &dvrCtx.playpumpSettings);
    dvrCtx.playpumpSettings.dataCallback.callback = playpump_data_callback;
    dvrCtx.playpumpSettings.dataCallback.context = dvrCtx.playpumpEvent;
    NEXUS_Playpump_SetSettings(dvrCtx.playpump, &dvrCtx.playpumpSettings);
    if (test_cases[testCase].startWithAd) {
        videoPid = adInfo[dvrCtx.adCtx.adIndex].vPid;
        audioPid = adInfo[dvrCtx.adCtx.adIndex].aPid;
    }
    dvrCtx.videoPidChannel = NEXUS_Playpump_OpenPidChannel(dvrCtx.playpump, videoPid, NULL);
    dvrCtx.stcPidChannel  = NEXUS_Playpump_OpenPidChannel(dvrCtx.playpump, pcrPid, NULL);
    dvrCtx.audioPidChannel = NEXUS_Playpump_OpenPidChannel(dvrCtx.playpump, audioPid, NULL);


    /* decoders and display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.spliceEnabled = true;
    dvrCtx.videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(dvrCtx.videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.spliceEnabled = true;
    dvrCtx.audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(dvrCtx.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(dvrCtx.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_StcChannel_GetDefaultSettings(0, &dvrCtx.stcSettings);
    dvrCtx.stcSettings.timebase = NEXUS_Timebase_e0;
    dvrCtx.stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    dvrCtx.stcSettings.modeSettings.pcr.pidChannel = dvrCtx.stcPidChannel;
    dvrCtx.stcSettings.stcIndex = 0;
    dvrCtx.stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    dvrCtx.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &dvrCtx.stcSettings);


    NEXUS_Playpump_Start(dvrCtx.playpump);


    #if AUDIO_ENABLE
    NEXUS_VideoDecoder_GetSettings(dvrCtx.videoDecoder, &videoDecoderSettings);
    BDBG_WRN(("default video discard threshold %d",videoDecoderSettings.discardThreshold));
     /*videoDecoderSettings.ptsOffset = 1800;*/
    videoDecoderSettings.discardThreshold = 4*45000; /* 4 seconds*/
    NEXUS_VideoDecoder_SetSettings(dvrCtx.videoDecoder, &videoDecoderSettings);
    NEXUS_AudioDecoder_GetSettings(dvrCtx.audioDecoder, &audioDecoderSettings);
    BDBG_WRN(("default audio discard threshold %d wideGaThreshold %s gaThreshold %d",audioDecoderSettings.discardThreshold,audioDecoderSettings.wideGaThreshold?"true":"false",audioDecoderSettings.gaThreshold));
    audioDecoderSettings.discardThreshold = 0; /* 6 seconds */
    NEXUS_AudioDecoder_SetSettings(dvrCtx.audioDecoder, &audioDecoderSettings);
    BDBG_WRN(("audio discard threshold %d wideGaThreshold %s gaThreshold %d",audioDecoderSettings.discardThreshold,audioDecoderSettings.wideGaThreshold?"true":"false",audioDecoderSettings.gaThreshold));
    BDBG_WRN(("video discard threshold %d",videoDecoderSettings.discardThreshold));
    #endif
    NEXUS_VideoDecoder_GetDefaultStartSettings(&dvrCtx.videoProgram);
    dvrCtx.videoProgram.codec = videoCodec;
    dvrCtx.videoProgram.pidChannel = dvrCtx.videoPidChannel;
    dvrCtx.videoProgram.stcChannel = dvrCtx.stcChannel;
    rc = NEXUS_VideoDecoder_Start(dvrCtx.videoDecoder, &dvrCtx.videoProgram);
    BDBG_ASSERT(!rc);


    #if AUDIO_ENABLE
    NEXUS_AudioDecoder_GetDefaultStartSettings(&dvrCtx.audioProgram);
    dvrCtx.audioProgram.codec = audioCodec;
    dvrCtx.audioProgram.pidChannel = dvrCtx.audioPidChannel;
    dvrCtx.audioProgram.stcChannel = dvrCtx.stcChannel;
    rc = NEXUS_AudioDecoder_Start(dvrCtx.audioDecoder, &dvrCtx.audioProgram);
    BDBG_ASSERT(!rc);
    #endif

    NEXUS_VideoDecoder_GetSettings(dvrCtx.videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
    rc = NEXUS_VideoDecoder_SetSettings(dvrCtx.videoDecoder,&videoDecoderSettings);
    BDBG_ASSERT(!rc);

    BKNI_Memset(&dvrCtx.adCtx.btp_in,0,sizeof(dvrCtx.adCtx.btp_in));
    dvrCtx.adCtx.btp_in.pBuffer = (uint8_t *)BKNI_Malloc(188);

    pthread_create(&spliceThreadId, NULL, spliceThread, (void *)&dvrCtx);
    BDBG_ASSERT(spliceThreadId);
    if (!test_cases[testCase].startWithAd) {
        /*
         * Set stop PTS insertion point in the dvr playback
         */
        NEXUS_VideoDecoder_GetSpliceSettings(dvrCtx.videoDecoder, &videoSpliceSettings);
        videoSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
        videoSpliceSettings.pts = adInfo[dvrCtx.adCtx.adIndex].mainSpliceOutPts;
        videoSpliceSettings.ptsThreshold = VIDEO_SPLICE_TOLERANCE;
        videoSpliceSettings.splicePoint.callback = video_insertion_point_callback;
        videoSpliceSettings.splicePoint.context = (void *) &dvrCtx;
        rc = NEXUS_VideoDecoder_SetSpliceSettings(dvrCtx.videoDecoder,&videoSpliceSettings);
        BDBG_ASSERT(!rc);

#if AUDIO_ENABLE
        NEXUS_AudioDecoder_GetSpliceSettings(dvrCtx.audioDecoder, &audioSpliceSettings);
        audioSpliceSettings.mode = NEXUS_DecoderSpliceMode_eStopAtPts;
        audioSpliceSettings.pts = adInfo[dvrCtx.adCtx.adIndex].mainSpliceOutPts-AUDIO_SPLICE_LAG;
        audioSpliceSettings.ptsThreshold = AUDIO_SPLICE_TOLERANCE;
        audioSpliceSettings.splicePoint.callback = audio_insertion_point_callback;
        audioSpliceSettings.splicePoint.context = (void *) &dvrCtx;
        rc = NEXUS_AudioDecoder_SetSpliceSettings(dvrCtx.audioDecoder,&audioSpliceSettings);
        BDBG_ASSERT(!rc);
#endif
    }
    dvrCtx.dvrPlay = true;
    #if AUDIO_ENABLE
    dvrCtx.audioSplice = false;
    #endif
    dvrCtx.videoSplice = false;

    dvrCtx.file = fopen(test_cases[testCase].streamName, "rb");
    BDBG_ASSERT((dvrCtx.file));

    if (test_cases[testCase].startWithAd) {
        dvrCtx.dvrPlay = false; /* stop dvr playback before it is started. */
        BKNI_SetEvent(dvrCtx.spliceOutEvent); /* trigger splice thread. */
    }
    pumpDvr(&dvrCtx);
    fclose(dvrCtx.file);

    NEXUS_VideoDecoder_Stop(dvrCtx.videoDecoder);
    #if AUDIO_ENABLE
    NEXUS_AudioDecoder_Stop(dvrCtx.audioDecoder);
    #endif
    NEXUS_Playpump_Stop(dvrCtx.playpump);

    BDBG_WRN(("press ENTER to exit"));
    getchar();
    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(dvrCtx.videoDecoder);
    NEXUS_AudioDecoder_Close(dvrCtx.audioDecoder);
    NEXUS_Playpump_CloseAllPidChannels(dvrCtx.playpump);
    NEXUS_Playpump_Close(dvrCtx.playpump);
    BKNI_DestroyEvent(dvrCtx.playpumpEvent);
    BKNI_DestroyEvent(dvrCtx.spliceInEvent);
    BKNI_DestroyEvent(dvrCtx.spliceOutEvent);
    BKNI_DestroyEvent(dvrCtx.dvrResume);
    BKNI_DestroyMutex(dvrCtx.dataFeedMutex);
    BKNI_DestroyMutex(dvrCtx.mutex);
    BKNI_Free(dvrCtx.adCtx.btp_in.pBuffer);
    pthread_join(spliceThreadId,NULL);
    NEXUS_Platform_Uninit();
    return 0;
}
