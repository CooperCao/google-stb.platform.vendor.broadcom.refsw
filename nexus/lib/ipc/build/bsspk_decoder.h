/******************************************************************************
 *    (c)2011 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
#ifndef BSSPK_DECODER_H__
#define BSSPK_DECODER_H__

#include "bipc.h"
#include "nexus_platform_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsspk_decoder *bsspk_decoder_t;


typedef struct bsspk_decoder_video_settings {
    bool closedCaptionPassthrough;
    uint32_t macrovision;
    uint32_t cgms;
    struct {
        uint16_t values[10];
        uint16_t count;
    } errorConcealment;
} bsspk_decoder_video_settings;

typedef enum bsspk_decoder_audio_output_format {
    bsspk_decoder_audio_output_format_stereo,
    bsspk_decoder_audio_output_format_multichannelpcm,
    bsspk_decoder_audio_output_format_compressed
} bsspk_decoder_audio_output_format;

typedef struct bsspk_decoder_audio_settings {
    bool karaoke;
    uint32_t volume;
    int panning;
    bsspk_decoder_audio_output_format output_format;
    struct {
        bool analog;
        bool spdif;
        bool hdmi;
    } output;
    bool dolbyBulletin11;
} bsspk_decoder_audio_settings;

typedef struct bsspk_decoder_resources {
    unsigned video_id;
    unsigned audio_id;
} bsspk_decoder_resources;

bsspk_decoder_t  bsspk_decoder_open(bipc_t ipc);
int bsspk_decoder_get_resources(bsspk_decoder_t decoder, bsspk_decoder_resources *resources);
int bsspk_decoder_get_default_audio_settings(bsspk_decoder_t decoder, bsspk_decoder_audio_settings *settings);
int bsspk_decoder_set_audio_settings(bsspk_decoder_t decoder, unsigned id, const bsspk_decoder_audio_settings *settings);
int bsspk_decoder_get_default_video_settings(bsspk_decoder_t decoder, bsspk_decoder_video_settings *settings);
int bsspk_decoder_set_video_settings(bsspk_decoder_t decoder, unsigned id, const bsspk_decoder_video_settings *settings);
void bsspk_decoder_close(bsspk_decoder_t decoder);
int bsspk_decoder_clock_get(bsspk_decoder_t decoder, uint32_t *clock);
int bsspk_decoder_clock_set(bsspk_decoder_t decoder, uint32_t clock);
int bsspk_decoder_clock_stop(bsspk_decoder_t decoder);
int bsspk_decoder_clock_start(bsspk_decoder_t decoder);

#ifdef __cplusplus
}
#endif

#endif /* BSSPK_DECODER_H__ */

