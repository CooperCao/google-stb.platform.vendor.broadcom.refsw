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
#ifndef MEDIA_PROBE_H__
#define MEDIA_PROBE_H__

#include "nexus_types.h"
#include "nxapps_crypto_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
simple standalone probe

returns single program
**/
struct probe_results
{
    struct {
        unsigned pid;
        NEXUS_AudioCodec codec;
    } audio[10];
    unsigned num_audio;
    struct {
        unsigned pid;
        NEXUS_VideoCodec codec;
        unsigned width, height, colorDepth; /* initial detected, could vary later on */
        NEXUS_VideoFrameRate frameRate;
        NEXUS_AspectRatio aspectRatio;
        struct {
            unsigned x, y;
        } sampleAspectRatio;
        struct {
            unsigned pid;
            NEXUS_VideoCodec codec;
        } enhancement;
    } video[10];
    unsigned num_video;
    struct {
        unsigned pid;
    } other[10];
    unsigned num_other;
    NEXUS_TransportType transportType;
    NEXUS_TransportTimestampType timestampType;
    bool useStreamAsIndex; /* player should use stream as index because it is required or available */
    bool enableStreamProcessing; /* Copy to NEXUS_PlaybackSettings.enableStreamProcessing, for ES audio */

    struct {
        bool valid;
        NEXUS_VideoEotf eotf;
        NEXUS_ContentLightLevel contentLightLevel;
        NEXUS_MasteringDisplayColorVolume masteringDisplayColorVolume;
    } videoColourMasteringMetadata;
};

struct probe_request
{
    const char *streamname;
    unsigned program;
    bool quiet;
    struct {
        NEXUS_SecurityAlgorithm algo;
        struct {
            uint8_t *data;
            unsigned size;
        } key;
    } decrypt;
};
void probe_media_get_default_request(struct probe_request *request);
int probe_media_request(const struct probe_request *request, struct probe_results *results);

/* simpler version, backward compat
defaults probe_request.quiet = false, .program = 0 */
int probe_media(const char *streamname, struct probe_results *results);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_PROBE_H__ */
