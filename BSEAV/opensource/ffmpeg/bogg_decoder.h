/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef _BOGG_DECODER_H_
#define _BOGG_DECODER_H_

#include "nexus_pid_channel.h"
#include "nexus_video_image_input.h"
#include "nexus_stc_channel.h"

#ifdef _cplusplus
extern "C"
{
#endif

typedef struct bogg_decoder_create_settings {
    unsigned decoder_queue;
} bogg_decoder_create_settings;

typedef struct bogg_decoder *bogg_decoder_t;

typedef struct bogg_decoder_start_settings {
    const char *fname;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_StcChannelHandle stcChannel;
} bogg_decoder_start_settings;

typedef struct bogg_decoder_start_information {
    int unused;
} bogg_decoder_start_information;

typedef struct bogg_decoder_status {
    struct {
        NEXUS_PtsType pts_type;
        uint32_t pts;
        unsigned decoder_queue;
    } video;
} bogg_decoder_status;

void bogg_decoder_get_default_create_settings(bogg_decoder_create_settings *settings);
bogg_decoder_t bogg_decoder_create(const bogg_decoder_create_settings *settings);
void bogg_decoder_destroy(bogg_decoder_t flv);
void bogg_decoder_get_status(bogg_decoder_t flv, bogg_decoder_status *status);
void bogg_decoder_get_default_start_settings(bogg_decoder_start_settings *settings);
NEXUS_Error bogg_decoder_start(bogg_decoder_t flv, const bogg_decoder_start_settings *settings, bogg_decoder_start_information *information);
void bogg_decoder_stop(bogg_decoder_t flv);

#ifdef _cplusplus
}
#endif


#endif /* _BOGG_DECODER_H_ */
