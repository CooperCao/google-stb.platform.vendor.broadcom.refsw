/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/
#include "bstd.h"
#include "nexus_types.h"
#include "decoder_bitrate.h"


BDBG_MODULE(decoder_bitrate);

void decoder_bitrate_init(struct decoder_bitrate *br)
{
    br->enabled = false;
    br->origin = 0;
    gettimeofday(&br->start, NULL);
    return;
}

#if NEXUS_HAS_AUDIO || NEXUS_HAS_VIDEO_DECODER
static unsigned decoder_bitrate_read_priv(struct decoder_bitrate *br, uint64_t numBytesDecoded)
{
    struct timeval now;
    uint64_t bytes;
    long msec;

    if(!br->enabled) {
        return 0;
    }
    gettimeofday(&now, NULL);
    if(numBytesDecoded <= br->origin) {
        br->origin = numBytesDecoded;
        br->start = now;
        return 0;
    }
    bytes = numBytesDecoded - br->origin;
    msec = 1000*(now.tv_sec - br->start.tv_sec) + (now.tv_usec - br->start.tv_usec)/1000;
    if(msec <= 0) {
        br->origin = numBytesDecoded;
        br->start = now;
    }
    BDBG_MSG(("%u(%u) %u %u", (unsigned)numBytesDecoded,(unsigned)bytes, (unsigned)msec, (unsigned)br->origin));
    return (8*1000*bytes) / msec;
}
#endif /* NEXUS_HAS_AUDIO && NEXUS_HAS_VIDEO_DECODER */


#if NEXUS_HAS_AUDIO
void decoder_bitrate_control_audio(struct decoder_bitrate *br, NEXUS_AudioDecoderHandle audioDecoder,bool enable)
{
    if(enable && !br->enabled) {
        gettimeofday(&br->start, NULL);
        br->origin = 0;
        if(audioDecoder) {
            NEXUS_AudioDecoderStatus astatus;
            if(NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus)==NEXUS_SUCCESS) {
                br->origin = astatus.numBytesDecoded;
            }
        }
    }
    br->enabled = enable;
    return;
}

unsigned decoder_bitrate_audio(struct decoder_bitrate *br, const NEXUS_AudioDecoderStatus *astatus)
{
    return decoder_bitrate_read_priv(br, astatus->numBytesDecoded);
}
#endif /* NEXUS_HAS_AUDIO */

#if NEXUS_HAS_VIDEO_DECODER
void decoder_bitrate_control_video(struct decoder_bitrate *br, NEXUS_VideoDecoderHandle videoDecoder,bool enable)
{
    if(enable && !br->enabled) {
        gettimeofday(&br->start, NULL);
        br->origin = 0;
        if(videoDecoder) {
            NEXUS_VideoDecoderStatus vstatus;
            if(NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus)==NEXUS_SUCCESS) {
                br->origin = vstatus.numBytesDecoded;
            }
        }
    }
    br->enabled = enable;
    return;
}

unsigned decoder_bitrate_video(struct decoder_bitrate *br, const NEXUS_VideoDecoderStatus *vstatus)
{
    return decoder_bitrate_read_priv(br, vstatus->numBytesDecoded);
}
#endif /* NEXUS_HAS_VIDEO_DECODER */
