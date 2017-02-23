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
#ifndef _BSOFT_DECODER_H_
#define _BSOFT_DECODER_H_

#include "nexus_pid_channel.h"
#include "nexus_video_image_input.h"
#include "nexus_stc_channel.h"
#include "blst_squeue.h"

#ifdef _cplusplus
extern "C"
{
#endif

typedef struct bsoft_decoder *bsoft_decoder_t;

typedef struct bsoft_decoder_config {
    int unused;
#if 0
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_StcChannelHandle stcChannel;
#endif
} bsoft_decoder_config;

struct bsoft_decoder_frame {
    BLST_SQ_ENTRY(bsoft_decoder_frame) link;
    uint32_t timestamp;
    unsigned width;
    unsigned height;
    unsigned qscale;
    NEXUS_SurfaceHandle surface;
};

void bsoft_decoder_default_config(bsoft_decoder_config *config);
bsoft_decoder_t bsoft_decoder_create(const bsoft_decoder_config *config);
void bsoft_decoder_destroy(bsoft_decoder_t decoder);
const struct bsoft_decoder_frame *bsoft_decoder_get_mature_frame(bsoft_decoder_t decoder, uint32_t stc, NEXUS_VideoImageInputSettings *imageInputSettings);
const struct bsoft_decoder_frame *bsoft_decoder_get_display_frame(bsoft_decoder_t decoder, NEXUS_VideoImageInputSettings *imageInputSettings);
NEXUS_Error bsoft_decoder_frame_alloc(struct bsoft_decoder_frame *frame, unsigned width, unsigned height);
struct bsoft_decoder_frame *bsoft_decoder_pop_free_frame(bsoft_decoder_t decoder);
void bsoft_decoder_push_decoded_frame(bsoft_decoder_t sd, struct bsoft_decoder_frame *frame);

#ifdef _cplusplus
}
#endif


#endif /* _BSOFT_DECODER_H_ */
