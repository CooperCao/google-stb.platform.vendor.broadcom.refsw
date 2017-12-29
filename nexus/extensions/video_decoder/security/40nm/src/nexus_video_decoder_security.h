/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_DECODER_SECURITY_H__
#define NEXUS_VIDEO_DECODER_SECURITY_H__

#include "nexus_video_decoder.h"

#include "bchp_common.h"
#undef NEXUS_NUM_XVD_DEVICES
#if defined BCHP_HVD_INTR2_2_REG_START
#define NEXUS_NUM_XVD_DEVICES 3
#elif defined BCHP_HVD_INTR2_1_REG_START || defined BCHP_AVD_INTR2_1_REG_START
#define NEXUS_NUM_XVD_DEVICES 2
#else
#define NEXUS_NUM_XVD_DEVICES 1
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define VIDEODECODER_MAX_ARCH 3  /*todo .. this must already be defiend somewhere!! */

typedef struct NEXUS_VideoDecoderSecureBootContext
{
    bool set; /* one shot */
    BAFL_BootInfo avdBootInfo;
    BAFL_FirmwareInfo astFirmwareInfo[3]; /* [3] hardcoded to match bxvd_platform_revn0.c code */
    unsigned deviceID;
    bool bVichRegSet;

    struct{
        bool verifed;
    }arch[VIDEODECODER_MAX_ARCH];
}   NEXUS_VideoDecoderSecureBootContext;

/* retrieve function pointers to enable region verification and register protection */
void NEXUS_VideoDecoder_P_GetSecurityCallbacks(
    BXVD_Settings *pSettings, /*in/out*/
    unsigned deviceId
    );

/* disable region verification for the video decoder firmwares associated with the specified device id */
void NEXUS_VideoDecoder_P_DisableFwVerification(
    unsigned deviceId
    );

NEXUS_Error NEXUS_VideoDecoder_P_BootDecoders(void);

#ifdef __cplusplus
}
#endif

#endif
