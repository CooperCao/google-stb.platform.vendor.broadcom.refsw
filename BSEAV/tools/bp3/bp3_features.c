/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
#include <stdio.h>
#include "bsagelib_types.h"
#include "bkni.h"
#include "bp3_features.h"

bp3featuresStruct bp3_features[BP3_FEATURES_NUM] = {
    {"H264/AVC", 1, 0},
    {"MPEG-2", 1, 1},
    {"H263", 1, 3},
    {"VC1", 1, 4},
    {"MPEG1", 1, 5},
    {"MPEG2DTV", 1, 6},
    {"MPEG-4 Part2/Divx", 1, 8},
    {"AVS", 1, 9},
    {"MPEG2 DSS PES", 1, 10},
    {"H264/SVC", 1, 11},
    {"H264/MVC", 1, 13},
    {"VP6", 1, 14},
    {"WebM/VP8", 1, 16},
    {"RV9", 1, 17},
    {"SPARK", 1, 18},
    {"H265 (HEVC)", 1, 19},
    {"VP9", 1, 20},
    {"HD Decode", 1, 31},
    {"10-bit", 1, 32},
    {"4Kp30", 1, 33},
    {"4Kp60", 1, 34},
    {"Macrovision", 1, 64},
    {"Dolby Vision HDR", 1, 65},
    {"Technicolor HDR", 1, 66},
    {"Technicolor ITM", 1, 67},
    {"DPA", 1, 80},
    {"CA Multi2", 1, 81},
    {"CA DVB-CSA3", 1, 82},
    {"DAP", 1, 96},
    {"Dolby Digital", 1, 97},
    {"Dolby Digital Plus", 1, 98},
    {"Dolby AC4", 1, 99},
    {"Dolby TrueHD", 1, 100},
    {"Dolby MS10/11", 1, 101},
    {"Dolby MS12v1", 1, 102},
    {"Dolby MS12v2", 1, 103},
    {"DTS TruVolume", 1, 104},
    {"DTS Digital Surround", 1, 105},
    {"DTS-HD (M6)", 1, 106},
    {"DTS-HDMA (M8)", 1, 107},
    {"DTS Headphone:X", 1, 108},
    {"DTS Virtual:X", 1, 109},
    {"DTS:X", 1, 110},
    {"Post Proc: DAP", 2, 0},
    {"Decode Dolby Digital", 2, 1},
    {"Decode Dolby Digital Plus", 2, 1},
    {"Decode AC4", 2, 3},
    {"Decode TrueHD", 2, 4},
    {"MS10/11", 2, 5},
    {"MS12 v1", 2, 6},
    {"MS12 v2", 2, 7},
    {"Decode Dolby Vision", 2, 33},
    {"Macrovision", 3, 0},
    {"Technicolor Prime", 4, 34},
    {"Technicolor ITM", 4, 35},
    {"DTS TruVolume", 5, 8},
    {"DTS Digital Surround", 5, 9},
    {"DTS-HD (M6)", 5, 10},
    {"DTS-HDMA (M8)", 5, 11},
    {"DTS Headphone:X", 5, 12},
    {"DTS Virtual:X", 5, 13},
    {"DTS:X", 5, 14}
};
