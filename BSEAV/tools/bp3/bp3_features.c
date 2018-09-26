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

#include <stdio.h>
#include "bsagelib_types.h"
#include "bkni.h"
#include "bp3_features.h"

bp3featuresStruct bp3_features[] = {
    {"H265 (HEVC)", 1, 19},
    {"Dolby Vision HDR Activation ($)", 1, 65},
    {"Technicolor HDR Activation ($)", 1, 66},
    {"Technicolor ITM Activation ($)", 1, 67},
    {"FE-CATV QAM Activation ($)", 1, 68},
    {"FE-DBS EchoStar Activation ($)", 1, 69},
    {"FE-DBS DIRECTV Activation ($)", 1, 70},
    {"FE-DBS DVB-S2X Activation ($)", 1, 71},
    {"CA Multi2", 1, 81},
    {"CA DVB-CSA3", 1, 82},
    {"AC4 Activation ($)", 1, 99},
    {"TrueHD Activation ($)", 1, 100},
    {"MS12v2 Activation ($)", 1, 103},
    {"MS12v1 Config C Activation", 1, 112},
    {"MS12v1 Config D Activation", 1, 113},
    {"MS12v1 Config B Activation ($)", 1, 114},
    {"MS12v1 Config A Activation ($)", 1, 115},
    {"DAP Content Processing Activation ($)", 1, 116},
    {"DAP Virtualizer Activation ($)", 1, 117},
    {"DAP Device Processing Activation ($)", 1, 118},
    {"Adv Output Ch Activation ($)", 1, 119},
    {"Post Proc: DAP", 2, 0},
    {"Decode Dolby Digital", 2, 1},
    {"Decode Dolby Digital Plus", 2, 2},
    {"Decode AC4", 2, 3},
    {"Decode TrueHD", 2, 4},
    {"MS10/11", 2, 5},
    {"MS12 v1", 2, 6},
    {"MS12 v2", 2, 7},
    {"Decode Dolby Vision", 2, 33},
    {"(ACP) Macrovision", 3, 0},
    {"Prime", 4, 34},
    {"ITM", 4, 35},
    {"DTS TruVolume", 5, 8},
    {"DTS Digital Surround", 5, 9},
    {"DTS-HD (M6)", 5, 10},
    {"DTS-HDMA (M8)", 5, 11},
    {"DTS Headphone:X", 5, 12},
    {"DTS Virtual:X", 5, 13},
    {"DTS:X", 5, 14},
    {NULL, 0, 0},
};
