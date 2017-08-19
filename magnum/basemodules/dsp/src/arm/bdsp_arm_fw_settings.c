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

#include "bdsp_arm_fw_settings.h"

const BDSP_Arm_Audio_DDPEncConfigParams BDSP_ARM_sDefDDPEncConfigSettings =
{
    {0,1,2,3,4,5,6,7},  /* Input channel routing */
    8,                  /* Encoder Mode */
    7,                  /* Audio Coding Mode */
    3,                  /* Center Mix Level, not set */
    31,                 /* Dialnorm */
    31,                 /* Dialnorm Channel 2 */
    0,                  /* Dolby Surround Mode */
    0,                  /* LFE Enable*/
    0,                  /* Evolution Metadata Availability Flag (Mode 8 DDP Only) */
    3,                  /* Surround Mix Level, not set  */
    0,                  /* 90 Degree Phase Shift Filter */
    640,                /* Datarate */
    0,                  /* Preferred Stereo Downmix Mode (Mode 8 DDP Only) */
    8,                  /* LtRt Center Mix Level (Mode 8 DDP Only), not set  */
    8,                  /* LtRt Surround Mix Level (Mode 8 DDP Only), not set  */
    8,                  /* LoRo Center Mix Level, not set  */
    8,                  /* LoRo Surround Mix Level, not set  */
    0,                  /* Dolby EX Mode (Mode 8 DDP Only) */
    0,                  /* Dolby Headphone Mode (Mode 8 DDP Only) */
    0,                  /* A/D Converter Type (Mode 8 DDP Only) */
    0,                  /* Send Audio Production Info */
    0,                  /* Send Audio Production Info Channel 2 */
    105,                /* Audio production mixing level */
    105,                /* Audio production mixing level Channel 2 */
    1,                  /* Copyright flag */
    1,                  /* Original Bitstream flag */
    0,                  /* Bitstream Mode */
    2,                  /* Audio production room type */
    2,                  /* Audio production room type Channel 2 */
    0,                  /* Additional Bitstream Information */
    1,                  /* Dolby Certification Flag */
    0,                  /* Enable Low Complexity Encoding */
    33,                 /* Intelligent loudness payloads only passed through by default */
    1,                  /* SPDIF Packing */
    {0}                 /* Additional Bitstream Information String */
};

const  BDSP_P_Audio_FrameSyncTsmConfigParams   BDSP_ARM_sDefaultFrameSyncTsmSettings =
{
    {                                       /* sFrameSyncConfigParams */
        0,
        0,
        {
            48000
        },
            1,
            {{
            0,
                0
            }},
        0,                                           /* eForceCompleteFirstFrame */
		BDSP_Raaga_Audio_DatasyncType_eNone
    },
    {                                       /* sTsmConfigParams */
        90,                                 /* i32TSMSmoothThreshold */
            0,                                  /* i32TSMSyncLimitThreshold */
            360,                                /* i32TSMGrossThreshold */
            135000,                             /* i32TSMDiscardThreshold */
            0,                                  /* i32TsmTransitionThreshold */
#if 0
#ifdef BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE
            BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE ),/* ui32STCAddr */
#else
            BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE ),/* ui32STCAddr */
#endif
#else
            0, /* ui32STCAddr */
#endif
            0,                                  /* ui32AVOffset */
            0,                                  /* ui32SwSTCOffset */
            5760,                               /* ui32AudioOffset */
            0,           /* eEnableTSMErrorRecovery */
            0,          /* eSTCValid */
            0,           /* ePlayBackOn */
            0,          /* eTsmEnable */
            0,           /* eTsmLogEnable */
            0,           /* eASTMEnable */
    }
};
