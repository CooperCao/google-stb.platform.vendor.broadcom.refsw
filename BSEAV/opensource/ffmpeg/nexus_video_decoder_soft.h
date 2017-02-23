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
#ifndef _NEXUS_VIDEO_DECODER_SOFT_H_
#define _NEXUS_VIDEO_DECODER_SOFT_H_

#include "nexus_base_types.h"
#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct b_avcodec_compressed_frame {
    const void *buf;
    size_t size;
    bool pts_valid;
    uint32_t pts;
} b_avcodec_compressed_frame;

typedef struct b_avcodec_plane {
    void *buf;
    size_t stride;
} b_avcodec_plane;

typedef struct b_avcodec_frame {
    b_avcodec_plane y,u,v;
    size_t width;
    size_t height;
    unsigned picture_count;
    bool pts_valid;
    bool random_access_point;
    bool sample_aspect_ratio_valid;
    uint32_t pts;
    unsigned picture_type;
    struct {
        unsigned numerator,denominator;
    } sample_aspect_ratio;
} b_avcodec_frame;

typedef struct b_video_softdecode *b_video_softdecode_t;
typedef struct b_video_softdecode b_video_softdecode;
typedef struct b_video_softdecode_methods {
    NEXUS_Error (*start)(b_video_softdecode_t decode, NEXUS_VideoCodec codec);
    void (*stop)(b_video_softdecode_t decode);
    void (*destroy)(b_video_softdecode_t decode);
    NEXUS_Error (*decode)(b_video_softdecode_t decode, const b_avcodec_compressed_frame *compressed, b_avcodec_frame *frame);
} b_video_softdecode_methods;

typedef struct b_video_softdecode_memory_token {
    void *ptr;
    void *opaque;
} b_video_softdecode_memory_token;

typedef struct b_video_sofdecode_memory_methods {
    NEXUS_Error (*alloc)(void *self, size_t size, b_video_softdecode_memory_token *token);
    void (*free)(void *self, const b_video_softdecode_memory_token *token);
} b_video_sofdecode_memory_methods;

struct b_video_softdecode {
    const b_video_softdecode_methods *methods;
};


#ifdef __cplusplus
}
#endif

#endif /* _NEXUS_VIDEO_DECODER_SOFT_H_ */
