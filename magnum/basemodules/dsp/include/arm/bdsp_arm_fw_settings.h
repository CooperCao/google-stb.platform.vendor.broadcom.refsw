/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BDSP_ARM_FW_SETTINGS_H_
#define BDSP_ARM_FW_SETTINGS_H_

#include "bdsp_common_fw_settings.h"

typedef struct BDSP_Arm_Audio_DDPEncConfigParams
{   /*
    Input channel routing [Default : 0,1,2,3,4,5,6,7]
                Use this option to designate which input file channels
                should be routed to which encoder inputs. Each value
                in the routing string corresponds to a specific channel:
                L, R, C, LFE, Ls, Rs, Lrs, Rrs
                Use -1 instead of a number to route silence to an input.
                Example: 0,0,0,-1,1,1 routes the first interleaved channel to
                left, center, and right, routes silence to LFE, and routes
                the second interleaved channel to Ls and Rs.
    */
    int32_t     i32ChannelRouting[8];


    /*Encoder mode [Default : 8 = Dolby Digital Plus Consumer Transcoding (bsid 12)]
                8 = Dolby Digital Plus Consumer Transcoding (bsid 12)
                9 = Dolby Digital Consumer Transcoding (bsid 4)
    */
    uint32_t    ui32Mode;

    /*Audio coding mode [Default : 7  = L R C LFE Ls Rs]
                Broadcast Standard channel order (LFE optional):
                    full mix channels (-i)
                    L R C LFE Ls Rs x1  x2
                0  = L R
                1  =     C
                2  = L R
                3  = L R C LFE
                4  = L R   LFE Ls
                5  = L R C LFE Ls
                6  = L R   LFE Ls Rs
                7  = L R C LFE Ls Rs
                19 = L R C LFE Ls Rs Lvh Rvh
                21 = L R C LFE Ls Rs Lrs Rrs
    */
    uint32_t    ui32AudioCodingMode;

    /*
    Center Mix Level [Default : 0 = 0.707 (-3.0 dB)]
                0 = 0.707 (-3.0 dB)
                1 = 0.595 (-4.5 dB)
                2 = 0.500 (-6.0 dB)
                3 = not set */
    uint32_t    ui32CenterMixLevel;

   /*
    Dialog Normalization [Default : 31 = -31dB (quiet input)]
                1  = -1dB (loudest input)
                ...
                27 = -27dB
                31 = -31dB (quiet input)
    */
    uint32_t    ui32DialNorm;

    /* Dialog Normalization channel 2 [Default : 31] */
    uint32_t    ui32DialNormChannel2;

    /*
    Dolby Surround mode [Default : 0 = not indicated]
                0 = not indicated
                1 = Dolby Surround disabled
                2 = Dolby Surround enabled
    */
    uint32_t    ui32DolbySurroundMode;

    /*
    Low frequency effects channel on/off [Default : 1 = LFE on]
                0 = LFE off
                1 = LFE on
    */
    uint32_t    ui32LfeEnable;

    /* Evolution metadata [Default : ] */
    uint32_t    ui32EvoMdAvailable;

    /*
    Surround Mix Level [Default:0 = 0.707 (-3.0 dB)]
                0 = 0.707 (-3.0 dB)
                1 = 0.500 (-6.0 dB)
                2 = 0.000 (-inf dB)
                3 = not set */
    uint32_t    ui32SurroundMixLevel;

    /*
    90 deg phase shift surrounds [Default:1 = enabled]
                0 = disabled
                1 = enabled
    */
    uint32_t    ui32NinetyDegreeFilterEnable;

    /* Datarate (in kilobits per second) [ Default:-dr384] */
    uint32_t    ui32Datarate;

    /*
    Preferred stereo downmix mode [Default:-xa0 = not indicated]
                0 = not indicated
                1 = Pro Logic downmix preferred
                2 = Stereo downmix preferred
                3 = Pro Logic II downmix preferred
    */
    uint32_t    ui32Downmix;

    /*
    Lt/Rt center mix level [Default:-xb4 = 0.707 (-3.0 dB)]
                        0 = 1.414 (+3.0 dB)
                        1 = 1.189 (+1.5 dB)
                        2 = 1.000 (0.0 dB)
                        3 = 0.841 (-1.5 dB)
                        4 = 0.707 (-3.0 dB)
                        5 = 0.595 (-4.5 dB)
                        6 = 0.500 (-6.0 dB)
                        7 = 0.000 (-inf dB)
                        8 = not set */
    uint32_t    ui32LtRtCenterMixLevel;

    /*
    Lt/Rt surround mix level [Default: 4 = 0.707 (-3.0 dB)]
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LtRtSurrMixLevel;

    /*
    Lo/Ro center mix level [Default : 4 = 0.707 (-3.0 dB)]
                0 = 1.414 (+3.0 dB)
                1 = 1.189 (+1.5 dB)
                2 = 1.000 (0.0 dB)
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LoRoCenterMixLevel;

    /*
    Lo/Ro surround mix level [Default: 4 -xe4 = 0.707 (-3.0 dB)]
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LoRoSurrMixLevel;

    /*
    Dolby Surround EX mode [Default: 0 -xf0 = not indicated]
                0 = not indicated
                1 = Dolby Surround EX disabled
                2 = Dolby Surround EX enabled
                */
    uint32_t    ui32DolbyEXMode;

    /*
    Dolby Headphone mode [Default: 0 -xg0 = not indicated]
                0 = not indicated
                1 = Dolby Headphone disabled
                2 = Dolby Headphone enabled
                */
    uint32_t    ui32DolbyHeadPhoneMode;

    /*
    A/D Converter Type [ Default:0 -xh0 = Standard]
                0 = Standard
                1 = HDCD
                */
    uint32_t    ui32ADConvType;

    /*
    Send Audio Production Info [Default: 0 -xi0 = Don't send info]
                0 = Don't send info
                1 = Send info
                */
    uint32_t    ui32SendAudioProductionInfo;

    /*
    Send Audio Production Info 2 [ Default: 0 -xis0 = Don't send info]
                0 = Don't send info
                1 = Send info
                */
    uint32_t    ui32SendAudioProductionInfo2;

    /*
    Audio production mixing level [ Default:105 -xm105 = 105 dB SPL]
                80 =  80 dB SPL
                105 = 105 dB SPL
                111 = 111 dB SPL
                */
    uint32_t    ui32AudioProductionMixLevel;

    /*
    Audio production mixing level 2 [ Default:105 -xm105 = 105 dB SPL]
                80 =  80 dB SPL
                105 = 105 dB SPL
                111 = 111 dB SPL
                */
    uint32_t    ui32AudioProductionMixLevel2;

    /*
    Copyright flag [Default: 1 -xn1 = copyrighted material]
                0 = non-copyrighted material
                1 = copyrighted material
                */
    uint32_t    ui32CopyrightFlag;

    /*
    Original Bitstream flag [Default: 1 -xo1 = not copied]
                0 = copied
                1 = not copied
                */
    uint32_t    ui32OriginalBitstream;

    /*
    Bitstream Mode [ Default: 0 -xp0 = Main audio service: complete main (CM)]
                0 = Main audio service: complete main (CM)
                1 = Main audio service: music and effects (ME)
                2 = Associated audio service: visually impaired (VI)
                3 = Associated audio service: hearing impaired (HI)
                4 = Associated audio service: dialogue (D)
                5 = Associated audio service: commentary (C)
                6 = Associated audio service: emergency (E)
                7 = Associated audio service: voice over (VO)
                */
    uint32_t    ui32BitstreamMode;

    /*
    Audio production room type [ Default: 2 -xr2 = Small room, flat monitor]
                0 = Not Indicated
                1 = Large Room, X curve monitor
                2 = Small room, flat monitor
                */
    uint32_t    ui32AudioProductionRoomType;

    /*
    Audio production room type 2 [ Default:2 -xrs2 = Small room, flat monitor]
                0 = Not Indicated
                1 = Large Room, X curve monitor
                2 = Small room, flat monitor
                */
    uint32_t    ui32AudioProductionRoomType2;

    /* Additional Bitstream Information Flag [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
                */
    uint32_t    ui32AdditionalBsiEnable;

    /* Dolby Certification Flag [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
                */
    uint32_t    ui32DolbyCertificationFlag;

    /* Enable Low Complexity Encoding [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
        This flag will choose the optimal set of tools to make an encoding
                */
    uint32_t    ui32LowComplexity;

    /* Bitmask of payload IDs to try to pass-through */
    int32_t     ui32SupportedEvolutionPayloads;

    /* Enable / Disable SPDIF or HDMI packing [Default : 1 = enabled] */
    uint32_t    ui32SpdifEnable;

    /* Additional Bitstream Information String [Default : 0]
       A maximum of 64 characters can be represented as ASCII converted integers
       Eg: "Hello" maps to {104, 101, 108, 108, 111, 0}
                */
    int32_t     i32AddBsi[64];

} BDSP_Arm_Audio_DDPEncConfigParams;

extern const BDSP_Arm_Audio_DDPEncConfigParams         BDSP_ARM_sDefDDPEncConfigSettings;
extern const BDSP_P_Audio_FrameSyncTsmConfigParams     BDSP_ARM_sDefaultFrameSyncTsmSettings;

#endif /*BDSP_ARM_FW_SETTINGS_H_*/
