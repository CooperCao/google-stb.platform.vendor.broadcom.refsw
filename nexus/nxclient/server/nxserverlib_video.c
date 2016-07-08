/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#include "nxserverlib_impl.h"

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_mosaic_display.h"
#include "nexus_video_input.h"
#include "nexus_video_decoder_private.h"

BDBG_MODULE(nxserverlib_video);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

/*
video decoders and windows are destroyed when not in use
*/

struct video_decoder_resource {
    bool used; /* if false, it can be destroyed */
    unsigned index;
    NEXUS_VideoDecoderHandle handle[NXCLIENT_MAX_IDS]; /* if >1, mosaic */
    unsigned stcIndex;

    struct b_connect *connect;
    NxClient_ConnectSettings connectSettings; /* copy of settings that were used when created */
    unsigned windowIndex; /* index into connect->client->session->window[] */
    struct b_session *session; /* needed for cleanup after connect/clients are gone */
    struct video_decoder_resource *linked;
};
#define IS_MOSAIC(connect) ((connect)->settings.simpleVideoDecoder[1].id)

#ifndef NEXUS_NUM_SID_VIDEO_DECODERS
#define NEXUS_NUM_SID_VIDEO_DECODERS 0
#endif
#ifndef NEXUS_NUM_DSP_VIDEO_DECODERS
#define NEXUS_NUM_DSP_VIDEO_DECODERS 0
#endif
#ifndef NEXUS_NUM_SOFT_VIDEO_DECODERS
#define NEXUS_NUM_SOFT_VIDEO_DECODERS 0
#endif
#define TOTAL_DECODERS (NEXUS_NUM_VIDEO_DECODERS+NEXUS_NUM_SID_VIDEO_DECODERS+NEXUS_NUM_DSP_VIDEO_DECODERS+NEXUS_NUM_HDMI_INPUTS+NEXUS_NUM_SOFT_VIDEO_DECODERS)

static struct {
    struct video_decoder_resource *r;
} g_decoders[TOTAL_DECODERS];
static void video_decoder_destroy(struct video_decoder_resource *r);

enum b_cap {
    b_cap_no,
    b_cap_possible,
    b_cap_preferred
};

/* convert enum to a comparable integer for a bandwidth equation.
consider 50==60 and 24==25==30. consider i60 == p30. consider 1920x1080==1280x720.
valid comparisons: 4Kp60 > 4Kp30 > 1080p60 > 1080i60/1080p30/720p60 > 480p60 > 480i
*/
static unsigned nxserver_p_video_comparator(unsigned height, bool interlaced, unsigned verticalFreq)
{
    if (verticalFreq >= 50) {
        verticalFreq = interlaced ? 30 : 60;
    }
    else {
        verticalFreq = 30;
    }
    if (height > 1088) { /* UHD */
        return 200 + verticalFreq;
    }
    else if (height >= 1080) { /* 1080p60/30 */
        return 100 + verticalFreq;
    }
    else if (height >= 720) { /* 720p60, which is like 1080p30 */
        return 100 + 30;
    }
    else { /* SD */
        return verticalFreq;
    }
}
static unsigned nxserver_p_videoformat_comparator(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormatInfo info;
    NEXUS_VideoFormat_GetInfo(format, &info);
    return nxserver_p_video_comparator(info.height, info.interlaced, info.verticalFreq/100);
}

static enum b_cap video_decoder_p_meets_decoder_cap(struct b_connect *connect, unsigned index, unsigned assumed_framerate)
{
    nxserver_t server = connect->client->server;
    enum b_cap result = b_cap_preferred;
    unsigned i;

    if (!server->settings.memConfigSettings.videoDecoder[index].maxFormat) {
        /* for non-memconfig platforms, we can't distiguish between decoders */
        return b_cap_preferred;
    }

    if (connect->settings.simpleVideoDecoder[0].windowCapabilities.type == NxClient_VideoWindowType_eNone) {
        if (server->videoDecoder.cap.videoDecoder[index].feeder.index != (unsigned)-1) {
            result = b_cap_possible;
        }
    }
    else {
        if (server->videoDecoder.cap.videoDecoder[index].feeder.index == (unsigned)-1) {
            return b_cap_no;
        }
    }

    if (server->settings.svp == nxserverlib_svp_type_none) {
        /* unless running SVP 2.0 (with -svp or -svp_urr), we must match secureVideo with decoder secure/unsecure capabilities) */
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo) {
            if (server->settings.memConfigSettings.videoDecoder[index].secure == NEXUS_SecureVideo_eUnsecure) {
                return b_cap_no;
            }
        }
        else {
            if (server->settings.memConfigSettings.videoDecoder[index].secure == NEXUS_SecureVideo_eSecure) {
                return b_cap_no;
            }
        }
    }

    if (IS_MOSAIC(connect)) {
        unsigned needed_mem=0, avail_mem;

        /* mosaic requires MFD0..3 for "combo trigger". see SW7445-1608 and SW7445-1710. */
        if (server->platformSettings.videoDecoderModuleSettings.mfdMapping[index] > 3) {
            return b_cap_no;
        }

        /* estimate mosaic memory based on # of pixels.
        TODO: allow mosaic even if memconfig doesn't specify mosaic settings. would need some conversion. */
        for (i=0;i<NXCLIENT_MAX_IDS && connect->settings.simpleVideoDecoder[i].id;i++) {
            needed_mem += connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth * connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight;
        }
        avail_mem = server->settings.memConfigSettings.videoDecoder[index].mosaic.maxNumber *
                    server->settings.memConfigSettings.videoDecoder[index].mosaic.maxWidth *
                    server->settings.memConfigSettings.videoDecoder[index].mosaic.maxHeight;
        if (needed_mem > avail_mem) {
            return b_cap_no;
        }

        /* for platforms with linked decoders, only allow mosaic on decoder 0 */
        if (connect->client->server->videoDecoder.special_mode && index != 0) {
            return b_cap_no;
        }
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxFormat) {
        if (nxserver_p_videoformat_comparator(connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxFormat) >
            nxserver_p_videoformat_comparator(server->settings.memConfigSettings.videoDecoder[index].maxFormat)) return b_cap_no;
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxWidth || connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight) {
        if (nxserver_p_video_comparator(connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight, false, assumed_framerate) >
            nxserver_p_videoformat_comparator(server->settings.memConfigSettings.videoDecoder[index].maxFormat)) return b_cap_no;
    }

    for (i=0;i<NEXUS_VideoCodec_eMax;i++) {
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[i] &&
            !server->videoDecoder.cap.memory[index].supportedCodecs[i]) {
            return b_cap_no;
        }
    }

    /* compare decoder cap with memconfig. if memConfigSettings are zero, then this is a non-memconfig platform,
    so we just provide any decoder. */
    if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.colorDepth && server->videoDecoder.cap.memory[index].colorDepth) {
        /* decoder - required */
        if (IS_MOSAIC(connect)) {
            if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.colorDepth > server->videoDecoder.cap.memory[index].mosaic.colorDepth) {
                return b_cap_no;
            }
        }
        else {
            if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.colorDepth > server->videoDecoder.cap.memory[index].colorDepth) {
                return b_cap_no;
            }
        }

        /* feeder - can be optional or required */
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.feeder.colorDepth && server->videoDecoder.cap.videoDecoder[index].feeder.colorDepth) {
            /* required */
            if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.feeder.colorDepth > server->videoDecoder.cap.videoDecoder[index].feeder.colorDepth) {
                return b_cap_no;
            }
        }
        else {
            if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.colorDepth > server->videoDecoder.cap.videoDecoder[index].feeder.colorDepth) {
                /* a downconvert would happen, but the user does not require it be avoided.
                don't return return because some later condition (added in the future) may result in b_cap_no. */
                result = b_cap_possible;
            }
        }
    }

    return result;
}

/* params: index = current index. start with NEXUS_MAX_VIDEO_DECODERS.
   return: next index, caller terminates with -1. */
static int b_next_decoder_index(struct b_connect *connect, int index)
{
    if (connect->client->server->videoDecoder.special_mode == b_special_mode_linked_decoder2)
    {
        if (connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder) {
            /* encoder uses 2,1 */
            switch (index) {
            case NEXUS_MAX_VIDEO_DECODERS: return 2;
            case 2: return 0;
            default: return -1;
            }
        }
        else {
            /* non-transcode uses 0,2 */
            switch (index) {
            case NEXUS_MAX_VIDEO_DECODERS: return 0;
            case 0: return 2;
            default: return -1;
            }
        }
    }
    else {
        /* a reverse search generally finds the least capable decoder that meets requirements */
        return index-1;
    }
}

static void video_decoder_p_find_decoder(struct b_connect *connect, unsigned assumed_framerate, unsigned *pindex, unsigned *pnum_avail)
{
    int i = NEXUS_MAX_VIDEO_DECODERS;
    *pnum_avail = 0;

    while ((i = b_next_decoder_index(connect, i)) != -1) {
        bool done = false;
        if (!connect->client->server->settings.memConfigSettings.videoDecoder[i].used) {
            continue;
        }
        if (!g_decoders[i].r || !g_decoders[i].r->used) {
            (*pnum_avail)++;
            switch (video_decoder_p_meets_decoder_cap(connect, i, assumed_framerate)) {
            case b_cap_no:
                break;
            case b_cap_possible:
                *pindex = i; /* keep going */
                break;
            case b_cap_preferred:
                *pindex = i;
                done = true;
                break;
            }
            if (done) break;
        }
    }
}

static const char * const eotfStrings[NEXUS_VideoEotf_eMax + 1] =
{
    "SDR",
    "HLG",
    "HDR10",
    "Invalid",
    "unspecified"
};

#if NEXUS_HAS_HDMI_OUTPUT
static void video_decoder_stream_changed(void * context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct video_decoder_resource *r = context;
    NEXUS_VideoDecoderStreamInformation streamInfo;

    BSTD_UNUSED(param);

    rc = NEXUS_VideoDecoder_GetStreamInformation(r->handle[0], &streamInfo);
    if (!rc)
    {
        struct b_session *session = r->session;
        session->hdmi.drm.input.eotf = streamInfo.eotf;
        BKNI_Memcpy(&session->hdmi.drm.input.metadata.typeSettings.type1.contentLightLevel, &streamInfo.contentLightLevel, sizeof(session->hdmi.drm.input.metadata.typeSettings.type1.contentLightLevel));
        BKNI_Memcpy(&session->hdmi.drm.input.metadata.typeSettings.type1.masteringDisplayColorVolume, &streamInfo.masteringDisplayColorVolume, sizeof(session->hdmi.drm.input.metadata.typeSettings.type1.masteringDisplayColorVolume));
        session->hdmi.drm.inputValid = true;
        nxserverlib_p_apply_hdmi_drm(session, NULL, false);
    }

}
#endif

/* Because of RTS dependencies, we don't keep a cached decoder open beyond the next video_decoder_create.
After that create, any cached decoder should be closed.
For instance, if a 4K decoder is cached, it may prevent another 1080p decoder from running. */
void nxserverlib_p_clear_video_cache(void)
{
    unsigned i;
    for (i=0;i<TOTAL_DECODERS;i++) {
        struct video_decoder_resource *r = g_decoders[i].r;
        if (r && !r->used) {
            video_decoder_destroy(r);
        }
    }
}

struct video_decoder_resource *nx_video_decoder_p_malloc(struct b_connect *connect, unsigned index)
{
    struct video_decoder_resource *r;
    r = BKNI_Malloc(sizeof(*r));
    if (!r) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(r, 0, sizeof(*r));
    g_decoders[index].r = r;
    r->index = index;
    r->session = connect->client->session;
    r->connect = connect;
    r->connectSettings = connect->settings;
    r->used = true;
    BDBG_MSG(("video_decoder_create %p: connect %p, index %d", (void*)r, (void*)connect, index));
    return r;
}

#if NEXUS_HAS_SAGE
static bool nxserver_p_urr_on(nxserver_t server)
{
    NEXUS_HeapRuntimeSettings heapSettings;
#ifdef NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP
    if (server->platformConfig.heap[NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
#else
    BSTD_UNUSED(heapSettings);
    BSTD_UNUSED(rc);
#endif
#ifdef NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP
    if (server->platformConfig.heap[NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
#endif
#ifdef NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP
    if (server->platformConfig.heap[NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
#endif
    return false;
}
#endif

static struct video_decoder_resource *video_decoder_create(struct b_connect *connect)
{
    unsigned index = TOTAL_DECODERS;
    int rc;
    struct video_decoder_resource *r;
    nxserver_t server = connect->client->server;
    struct b_session *session = connect->client->session;
    unsigned linked_decoder = 0;
    unsigned total_linked_decoders = 0;
    NEXUS_HeapHandle cdbHeap = NULL;

    /* allow SimpleVideoDecoder to cache VideoDecoder/VideoWindow connections to
    minimize start time */
    NEXUS_SimpleVideoDecoderModule_SetCacheEnabled(session->video.server, true);

    /* window-only is only used with HDMI input */
    if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.connectType == NxClient_VideoDecoderConnectType_eWindowOnly) {
#if NEXUS_HAS_HDMI_INPUT
        index = TOTAL_DECODERS - NEXUS_NUM_HDMI_INPUTS;
        if (g_decoders[index].r) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            return NULL;
        }
        r = nx_video_decoder_p_malloc(connect, index);
        return r;
#else
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
#endif
    }

    /* find unused index.
    first, check for an external index.
    use DSP/SID/SOFT decoders for codecs that require them.
    then, prefer one that's already created but unused.
    finally, create a new one. */
    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder]) {
        rc = server->settings.externalApp.allocIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_video_decoder, &index);
        if (rc) {
            BDBG_WRN(("no externally allocated video_decoder index available"));
            return NULL;
        }
        if (index >= TOTAL_DECODERS || g_decoders[index].r) {
            BDBG_ERR(("externally allocated video_decoder failure on %d", index));
            return NULL;
        }
        /* we have an externally allocated index */
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eMotionJpeg]) {
        if (server->videoDecoder.cap.sidVideoDecoder.useForMotionJpeg) {
            for (index=server->videoDecoder.cap.sidVideoDecoder.baseIndex;index<server->videoDecoder.cap.sidVideoDecoder.baseIndex+server->videoDecoder.cap.sidVideoDecoder.total;index++) {
                if (!g_decoders[index].r) break;
            }
        }
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eVp6]) {
        if (server->videoDecoder.cap.dspVideoDecoder.useForVp6) {
            for (index=server->videoDecoder.cap.dspVideoDecoder.baseIndex;index<server->videoDecoder.cap.dspVideoDecoder.baseIndex+server->videoDecoder.cap.dspVideoDecoder.total;index++) {
                if (!g_decoders[index].r) break;
            }
        }
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eVp9]) {
        if (server->videoDecoder.cap.softVideoDecoder.useForVp9) {
            for (index=server->videoDecoder.cap.softVideoDecoder.baseIndex;index<server->videoDecoder.cap.softVideoDecoder.baseIndex+server->videoDecoder.cap.softVideoDecoder.total;index++) {
                if (!g_decoders[index].r) break;
            }
        }
    }

    if (index == TOTAL_DECODERS) {
        if (server->videoDecoder.special_mode == b_special_mode_linked_decoder2 &&
            connect->settings.simpleVideoDecoder[1].id &&
            connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight >= 720)
        {
            if ((!g_decoders[0].r || !g_decoders[0].r->used) && (!g_decoders[1].r || !g_decoders[1].r->used))
            {
                index = 0;
                linked_decoder = 1;
                total_linked_decoders = 2;
            }
            else {
                BERR_TRACE(NEXUS_NOT_SUPPORTED);
                return NULL;
            }
        }
    }

    if (index == TOTAL_DECODERS) {
        unsigned num_avail = 0;
        unsigned tries = 0;
        /* two passes required: assume p60/50, then don't assume (aka assume p30/25/24). */
        for (tries=0; tries<2; tries++) {
            video_decoder_p_find_decoder(connect, tries?30:60, &index, &num_avail);
            if (index < TOTAL_DECODERS || connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxFormat) break;
        }
        if (index == TOTAL_DECODERS) {
            if (num_avail) {
                BDBG_WRN(("%d video decoder(s) found, but none meet capabilities", num_avail));
            }
            else {
                BDBG_WRN(("no video decoders available"));
            }
            NEXUS_VideoDecoderModule_PrintDecoders();
            /* don't pick from SID/DSP decoders unless going for MJPEG or VP6 */
        }
    }

    if (index >= TOTAL_DECODERS) {
        BDBG_WRN(("no video decoder available"));
        return NULL;
    }

#if NEXUS_HAS_SAGE
    if(connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo) {
        /* Use CRR for video CDB if secure decoder/heaps AND sage toggle on (URR=on) */
        /* Otherwise use GLR for video CDB (CRR will NOT work) */
        /* All secure picture buffer heaps are toggled at once, just need to check one */
        if (nxserver_p_urr_on(server)) {
            cdbHeap = server->settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP];
            if (!cdbHeap) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
        }
    }
#else
    if(connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo) {
        cdbHeap = server->settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP];
        if (!cdbHeap) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
    }
#endif
    BDBG_MSG(("VideoDecoder CDB in %s", cdbHeap ? "CRR" : "GLR"));

    if (g_decoders[index].r) {
        r = g_decoders[index].r;
        if (!r->used) {
            unsigned i;
            NEXUS_VideoDecoderOpenSettings openSettings;
            /* compare decoder capabilities, # of mosaics */
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                if (linked_decoder) break; /* always recreate for linked_decoder configuration */
                if ((connect->settings.simpleVideoDecoder[i].id!=0) != (r->connectSettings.simpleVideoDecoder[i].id!=0)) break;
                if (connect->settings.simpleVideoDecoder[i].id) {
                    if (BKNI_Memcmp(&connect->settings.simpleVideoDecoder[i].decoderCapabilities,
                        &r->connectSettings.simpleVideoDecoder[i].decoderCapabilities,
                        sizeof(connect->settings.simpleVideoDecoder[i].decoderCapabilities))) break;

                        NEXUS_VideoDecoder_GetOpenSettings(r->handle[0], &openSettings);
                        if(openSettings.cdbHeap != cdbHeap)
                            break;
                }
            }
            if (i < NXCLIENT_MAX_IDS) {
                /* need to recreate for mismatch */
                video_decoder_destroy(r);
            }
            else {
                r->used = true;
                r->connect = connect;
                BDBG_MSG(("video_decoder_create %p(recycled): connect %p, index %d", (void*)r, (void*)connect, index));
                nxserverlib_p_clear_video_cache();
                return r;
            }
        }
        else {
            BDBG_WRN(("video decoder %d already in use", index));
            return NULL;
        }
    }

    r = nx_video_decoder_p_malloc(connect, index);
    if (!r) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    if (linked_decoder) {
        if (g_decoders[linked_decoder].r) {
            video_decoder_destroy(g_decoders[linked_decoder].r);
        }
        r->linked = nx_video_decoder_p_malloc(connect, linked_decoder);
        if (!r->linked) {
            BKNI_Free(r);
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc;
        }
    }
    nxserverlib_p_clear_video_cache();

    if (!IS_MOSAIC(connect)) {
        /* regular (non-mosaic) */
        NEXUS_VideoDecoderOpenSettings openSettings;

        NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.fifoSize) {
            /* per connect override */
            openSettings.fifoSize = connect->settings.simpleVideoDecoder[0].decoderCapabilities.fifoSize;
        }
        else if (server->settings.videoDecoder.fifoSize) {
            /* global override */
            openSettings.fifoSize = server->settings.videoDecoder.fifoSize;
        }
        openSettings.avc51Enabled = connect->settings.simpleVideoDecoder[0].decoderCapabilities.avc51Enabled;
        openSettings.enhancementPidChannelSupported = connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc];
        openSettings.secureVideo = connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo;
        openSettings.cdbHeap = cdbHeap;

        r->handle[0] = NEXUS_VideoDecoder_Open(index, &openSettings);
        if (!r->handle[0]) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        /* maxWidth, maxHeight and supportedCodecs may be used in decoder selection,
        but they are also set by simple_decoder once selection is made. Cannot call NEXUS_VideoDecoder_SetSettings here. */

#if NEXUS_HAS_HDMI_OUTPUT
        /* TODO: This is for dynamic range -- not sure how that applies to mosaic. */
        if (session->hdmiOutput)
        {
            NEXUS_VideoDecoderPrivateSettings privateSettings;
            NEXUS_VideoDecoder_GetPrivateSettings(r->handle[0], &privateSettings);
            privateSettings.streamChanged.callback = video_decoder_stream_changed;
            privateSettings.streamChanged.context = r;
            rc = NEXUS_VideoDecoder_SetPrivateSettings(r->handle[0], &privateSettings);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
#endif

        BDBG_MSG(("  videoDecoder[%d]: %p, max %dx%d",
                  index, (void*)r->handle[0],
            connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxWidth,
            connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight));
    }
    else {
        unsigned i;

        for (i=0;i<connect->settings.simpleVideoDecoder[i].id;i++) {
            NEXUS_VideoDecoderOpenMosaicSettings openSettings;
            NEXUS_VideoDecoderSettings settings;
            unsigned parentIndex, mosaicIndex;

            NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openSettings);
            if (connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth &&
                connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight) {
                openSettings.maxWidth = connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth;
                openSettings.maxHeight = connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight;
            }
            if (connect->settings.simpleVideoDecoder[i].decoderCapabilities.fifoSize) {
                openSettings.openSettings.fifoSize = connect->settings.simpleVideoDecoder[i].decoderCapabilities.fifoSize;
            }
            openSettings.openSettings.secureVideo = connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo;
            openSettings.openSettings.cdbHeap = cdbHeap;

            if (r->linked && i >= (total_linked_decoders/2)) {
                openSettings.linkedDevice.enabled = true;
                openSettings.linkedDevice.avdIndex = 0;
                parentIndex = r->linked->index;
                mosaicIndex = i - (total_linked_decoders/2);
            }
            else {
                parentIndex = r->index;
                mosaicIndex = i;
            }
            r->handle[i] = NEXUS_VideoDecoder_OpenMosaic(parentIndex, mosaicIndex, &openSettings);
            if (!r->handle[i]) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
            NEXUS_VideoDecoder_GetSettings(r->handle[i], &settings);
            BDBG_MSG(("  mosaic videoDecoder[%d][%d]: %p, max %dx%d",
                      index, i, (void*)r->handle[i],
                connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth,
                connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight));
        }
    }
    return r;

error:
    video_decoder_destroy(g_decoders[index].r);
    return NULL;

err_malloc:
    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder]) {
        server->settings.externalApp.freeIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_video_decoder, index);
    }
    return NULL;
}

static void video_decoder_release(struct video_decoder_resource *r)
{
    nxserver_t server = r->session->server;
    BDBG_MSG(("video_decoder_release %p: connect %p, index %d", (void*)r, (void*)r->connect, r->index));
    BDBG_OBJECT_ASSERT(r->connect, b_connect);
    if (IS_MOSAIC(r->connect) || server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder] ||
        server->settings.videoDecoder.dynamicPictureBuffers ||
        r->index >= NEXUS_NUM_VIDEO_DECODERS /* SID or DSP decoder */
        )
    {
        video_decoder_destroy(r);
    }
    else {
        /* cached. by not closing, we have faster start time if the same decoder/window connection is used, which is likely. */
        r->used = false;
        r->connect = NULL;
    }
}

static void video_decoder_destroy(struct video_decoder_resource *r)
{
    unsigned i;
    nxserver_t server = r->session->server;

    BDBG_MSG(("video_decoder_destroy %p: connect %p, index %d", (void*)r, (void*)r->connect, r->index));

    if (r->linked) {
        video_decoder_destroy(r->linked);
    }
    BDBG_ASSERT(g_decoders[r->index].r == r);
    g_decoders[r->index].r = NULL;
    for (i=NXCLIENT_MAX_IDS-1;i<=NXCLIENT_MAX_IDS;i--) { /* reverse */
        if (r->handle[i]) {
            NEXUS_SimpleVideoDecoderModule_CheckCache(r->session->video.server, NULL, r->handle[i]);
            NEXUS_VideoDecoder_Close(r->handle[i]);
        }
    }
    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder]) {
        server->settings.externalApp.freeIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_video_decoder, r->index);
    }
    BKNI_Free(r);
}

static unsigned get_video_decoder_req_id(struct b_req *req, unsigned i) { return req->handles.simpleVideoDecoder[i].id; }

bool lacks_video(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_video_decoder];
    return req && req->handles.simpleVideoDecoder[0].id && !req->handles.simpleVideoDecoder[0].r;
}

/* SurfaceClient may come from any request for this client.
do not cache this handle in another request/connect; it may be freed. */
static int nxclient_get_surface_client(struct b_connect *connect, nxclient_t client, unsigned i, NEXUS_SurfaceClientHandle *pSurfaceClient)
{
    struct b_req *req;
    unsigned surfaceClientId;
    const nxclient_t org_client = client;

    if (i >= NXCLIENT_MAX_IDS) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    surfaceClientId = connect->settings.simpleVideoDecoder[i].surfaceClientId;
    if (!surfaceClientId) {
        *pSurfaceClient = NULL;
        return 0;
    }

    /* if client is NULL, this is a top-level call. Otherwise it is recursive. */
    if (!client) client = connect->client;

    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (req->handles.surfaceClient[i].id == surfaceClientId) {
                *pSurfaceClient = req->handles.surfaceClient[i].handle;
                return 0;
            }
            else if (!req->results.surfaceClient[i].id) {
                break;
            }
        }
    }

    /* if eVerified, we allow searching all clients */
    if (!org_client && client->clientSettings.configuration.mode == NEXUS_ClientMode_eVerified) {
        for (client = BLST_D_FIRST(&client->session->clients); client; client = BLST_D_NEXT(client, session_link)) {
            int rc;
            if (client == org_client) continue; /* already searched */
            rc = nxclient_get_surface_client(connect, client, i, pSurfaceClient);
            if (!rc) {
                return rc;
            }
        }
    }

    return NEXUS_INVALID_PARAMETER; /* no BERR_TRACE for recursive search */
}

static bool has_window(struct b_connect *connect)
{
    BDBG_ASSERT(connect->windowIndex < NEXUS_NUM_VIDEO_WINDOWS);
    return (connect->client->session->window[connect->windowIndex].connect == connect);
}

static int resize_full_screen(struct b_session *session, unsigned displayIndex, unsigned windowIndex, bool visible, bool remainVisible)
{
    NEXUS_VideoWindowSettings settings;
    NEXUS_VideoWindowHandle window;
    window = session->display[displayIndex].parentWindow[windowIndex];
    if (!window) {
        window = session->display[displayIndex].window[windowIndex][0];
    }
    if (!window) return 0;
    NEXUS_VideoWindow_GetSettings(window, &settings);
    settings.position.x = 0;
    settings.position.y = 0;
    settings.position.width = session->display[displayIndex].formatInfo.width;
    settings.position.height = session->display[displayIndex].formatInfo.height;
    if (windowIndex) {
        settings.position.width /= 2;
        settings.position.height /= 2;
        settings.position.x = settings.position.width;
    }
    else if (session->nxclient.displaySettings.display3DSettings.orientation == NEXUS_VideoOrientation_e3D_LeftRight) {
        settings.position.width /= 2;
    }
    else if (session->nxclient.displaySettings.display3DSettings.orientation == NEXUS_VideoOrientation_e3D_OverUnder) {
        settings.position.height /= 2;
    }
    if (!remainVisible) {
        settings.visible = visible;
    }
    return NEXUS_VideoWindow_SetSettings(window, &settings);
}

static void release_video_window(struct b_connect *connect, bool remainVisible);
static int acquire_video_window(struct b_connect *connect, bool grab)
{
    struct b_session *session = connect->client->session;
    unsigned index;
    unsigned i, j;
    int rc;

    if (!session->server->settings.grab) {
        grab = false;
    }
    if (has_window(connect)) {
        return 0;
    }
    if (is_transcode_connect(connect)) {
        return 0;
    }

    switch (connect->settings.simpleVideoDecoder[0].windowCapabilities.type) {
    case NxClient_VideoWindowType_eMain: index = 0; break;
#if NEXUS_NUM_VIDEO_WINDOWS > 1
    case NxClient_VideoWindowType_ePip:  index = 1; break;
#endif
    case NxClient_VideoWindowType_eNone: return 0;
    default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (session->window[index].connect) {
        rc = NEXUS_NOT_AVAILABLE;
        if (grab && connect->client->session->window[index].connect) {
            struct b_connect *prev_connect = connect->client->session->window[index].connect;
            BDBG_MSG(("grab video window %d", index));
            release_video_decoders(prev_connect);
            if (session->window[index].connect) {
                rc = NEXUS_NOT_AVAILABLE;
            }
            else {
                release_video_window(prev_connect, false);
                rc = 0;
                /* proceed with acquire */
            }
        }
        if (rc) {
            BDBG_WRN(("video window %d not available", index));
            return rc;
        }
    }

    /* reserve window */
    BDBG_MSG(("acquire_video_window: connect %p %d", (void*)connect, index));
    session->window[index].connect = connect;
    connect->windowIndex = index;

    /* create windows */
    for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
        if (session->display[j].display) {
            /* regular window may already be open */
            if (!session->display[j].window[index][0]) {
                session->display[j].window[index][0] = NEXUS_VideoWindow_Open(session->display[j].display, index);
                if (!session->display[j].window[index][0]) {
                    rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    goto error;
                }
            }

            if (IS_MOSAIC(connect)) {
                /* mosaics are not cached */
                BDBG_ASSERT(!session->display[j].parentWindow[index]);
                session->display[j].parentWindow[index] = session->display[j].window[index][0];
                session->display[j].window[index][0] = NULL;
                NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, session->display[j].parentWindow[index], NULL);
                for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                    if (!connect->settings.simpleVideoDecoder[i].id) break;
                    BDBG_ASSERT(!session->display[j].window[index][i]);
                    session->display[j].window[index][i] = NEXUS_VideoWindow_OpenMosaic(session->display[j].parentWindow[index], i);
                    if (!session->display[j].window[index][i]) {
                        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                        goto error;
                    }
                }
            }
            /* we need to make visible in case app doesn't use SurfaceCompositor for video window and
            it inherits an invisible window. */
            resize_full_screen(session, j, index, true, false);
        }
    }

    /* connect to SurfaceCompositor */
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SurfaceClientHandle surfaceClient;
        NEXUS_SurfaceCompositorClientSettings settings;

        if (!connect->settings.simpleVideoDecoder[i].id) break;

        rc = nxclient_get_surface_client(connect, NULL, i, &surfaceClient);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
        if (!surfaceClient) continue;

        /*
        For NEXUS_SurfaceCompositorClientSettings.display[].window[INDEX], INDEX must be the AcquireVideoWindow index. It is unchanging in a swap.
        For session->display[].window[INDEX], INDEX is the BVN window, 0 = main, 1 = PIP. It changes in a swap.
        */
        NEXUS_SurfaceCompositor_GetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
        for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
            NEXUS_VideoWindowHandle window = session->display[j].window[connect->windowIndex][i];
            if (!window) continue;
            if (nxserver_p_video_only_display(session, j)) {
                continue;
            }
            BDBG_MSG(("  set surfaceClient %p, display[%u].window[%u].window = %p", (void*)surfaceClient, j, connect->settings.simpleVideoDecoder[i].windowId, (void*)window));
            settings.display[j].window[connect->settings.simpleVideoDecoder[i].windowId].window = window;
#if NEXUS_NUM_VIDEO_ENCODERS
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            if(get_transcode_encoder_index(session->server, session->display[j].global_index) != -1) {
                NEXUS_VideoWindowSettings settings;
                BDBG_MSG(("session %d, display %d, display %u is encoded and disable video force capture", session->index, j, session->display[j].global_index));
                NEXUS_VideoWindow_GetSettings(window, &settings);
                if (settings.forceCapture) {
                    settings.forceCapture = false;
                    NEXUS_VideoWindow_SetSettings(window, &settings);
                }
            }
#endif
#endif
        }
        rc = NEXUS_SurfaceCompositor_SetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
        if (rc) { rc = BERR_TRACE(rc); /* continue */ }
    }

    return 0;

error:
    release_video_window(connect, false);
    return rc;
}

static void release_video_window(struct b_connect *connect, bool remainVisible)
{
    struct b_session *session = connect->client->session;
    unsigned i, j;

    BDBG_MSG(("release_video_window: connect %p %d", (void*)connect, connect->windowIndex));
    if (!has_window(connect)) return;

    /* disconnect from SurfaceCompositor */
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SurfaceClientHandle surfaceClient;
        NEXUS_SurfaceCompositorClientSettings settings;
        unsigned j;
        int rc;

        rc = nxclient_get_surface_client(connect, NULL, i, &surfaceClient);
        if (rc || !surfaceClient) continue;

        NEXUS_SurfaceCompositor_GetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
        for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
            settings.display[j].window[connect->settings.simpleVideoDecoder[i].windowId].window = NULL;
        }
        (void)NEXUS_SurfaceCompositor_SetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
    }

    for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
        if (session->display[j].display) {
            /* destroy mosaic windows */
            if (session->display[j].parentWindow[connect->windowIndex]) {
                for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                    if (session->display[j].window[connect->windowIndex][i]) {
                        NEXUS_VideoWindow_Close(session->display[j].window[connect->windowIndex][i]);
                        session->display[j].window[connect->windowIndex][i] = NULL;
                    }
                }

                /* change mosaic parent to regular window */
                session->display[j].window[connect->windowIndex][0] = session->display[j].parentWindow[connect->windowIndex];
                session->display[j].parentWindow[connect->windowIndex] = NULL;
            }
            resize_full_screen(session, j, connect->windowIndex, false, remainVisible);
        }
    }

    /* unreserve window */
    session->window[connect->windowIndex].connect = NULL;
    connect->windowIndex = 0; /* don't care */
}

void uninit_session_video(struct b_session *session)
{
    unsigned i, j, index;
    NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, NULL, NULL);
    for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
        if (session->display[j].display) {
            for (index=0;index<NEXUS_NUM_VIDEO_WINDOWS;index++) {
                for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                    if (session->display[j].window[index][i]) {
                        NEXUS_VideoWindow_Close(session->display[j].window[index][i]);
                        session->display[j].window[index][i] = NULL;
                    }
                }
                if (session->display[j].parentWindow[index]) {
                    NEXUS_VideoWindow_Close(session->display[j].parentWindow[index]);
                    session->display[j].parentWindow[index] = NULL;
                }
            }
        }
    }
}


static NEXUS_Error video_acquire_stc_index(struct b_connect * connect, int *pStcIndex)
{
    struct b_stc_caps stcreq;
    stc_index_request_init(connect, &stcreq);
    /* NRT mode audio and video decoder STCs are different, so video
           STC doesn't need to have audio decoder capability! */
    if (nxserver_p_connect_is_nonrealtime_encode(connect)) {
        stcreq.audio = false;
    }
    *pStcIndex = stc_index_acquire(connect, &stcreq);
    if (*pStcIndex == -1) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BDBG_MSG_TRACE(("connect %p acquired STC%u", connect, *pStcIndex));
    return 0;
}

static void video_release_stc_index(struct b_connect *connect, int stcIndex)
{
    stc_index_release(connect, stcIndex);
    BDBG_MSG(("connect %p released STC%u", (void*)connect, stcIndex));
}

int acquire_video_decoders(struct b_connect *connect, bool grab)
{
    struct b_req *req = connect->req[b_resource_simple_video_decoder];
    unsigned i;
    int rc;
    struct video_decoder_resource *r;
    int stcIndex;
    struct b_session *session = connect->client->session;

    if (!connect->settings.simpleVideoDecoder[0].id) {
        /* no request */
        return 0;
    }

    if (req->handles.simpleVideoDecoder[0].r) {
        BDBG_WRN(("already connected"));
        return 0;
    }

    rc = acquire_video_window(connect, grab);
    if (rc) return rc;

    /* one connect always resolves into one video_decoder_create */
    r = video_decoder_create(connect);
    if (!r) {
        if (grab) {
            for (i=0;i<TOTAL_DECODERS;i++) {
                struct video_decoder_resource *grab_from = g_decoders[i].r;
                if (grab_from) {
                    if (grab_from->connect && grab_from->connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder) {
                        /* never steal from a transcode */
                        continue;
                    }
                    if (grab_from->connect) {
                        if (grab_from->connect->client->session != session) {
                            continue;
                        }
                        release_video_decoders(grab_from->connect);
                        /* may have been destroyed, so need to re-get it */
                        grab_from = g_decoders[i].r;
                    }
                    if (grab_from) {
                        video_decoder_destroy(grab_from);
                    }
                    r = video_decoder_create(connect);
                    break;
                }
            }
        }
        if (!r) {
            BDBG_WRN(("connect %p: no video decoder available", (void*)connect));
            rc = NEXUS_NOT_AVAILABLE; /* no BERR_TRACE */
            goto err_create;
        }
    }

    rc = video_acquire_stc_index(connect, &stcIndex);
    if (rc) { rc = BERR_TRACE(rc); goto err_stcindex; }

    req->handles.simpleVideoDecoder[0].r = r;

    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleVideoDecoderHandle videoDecoder;
        NEXUS_SimpleVideoDecoderServerSettings settings;
        unsigned index;
        int rc;

        if (!connect->settings.simpleVideoDecoder[i].id) break;

        if (get_req_index(req, get_video_decoder_req_id, connect->settings.simpleVideoDecoder[i].id, &index)) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_set;
        }

        videoDecoder = req->handles.simpleVideoDecoder[index].handle;

        /* connect regular decoder to simple decoder */
        NEXUS_SimpleVideoDecoder_GetServerSettings(session->video.server, videoDecoder, &settings);
        settings.videoDecoder = r->handle[i];
        if (has_window(connect)) {
            unsigned j;
            for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                if (j>0 && nxserverlib_p_native_3d_active(session)) {
                    continue;
                }
                settings.window[j] = session->display[j].window[connect->windowIndex][i];
                if (settings.window[j]) {
                    NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, settings.window[j], settings.videoDecoder);
                    settings.display[j] = session->display[j].display; /* closedCaptionRouting */
                }
            }
        }
        settings.stcIndex = stcIndex;
        settings.mosaic = IS_MOSAIC(connect);

        BDBG_MSG(("connect SimpleVideoDecoder %p to decoder %p, window0 %p, stcIndex %u", (void*)videoDecoder, (void*)settings.videoDecoder, (void*)settings.window[0], settings.stcIndex));
        rc = NEXUS_SimpleVideoDecoder_SetServerSettings(session->video.server, videoDecoder, &settings);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_set;
        }
    }
    return 0;

err_set:
    release_video_decoders(connect);
err_stcindex:
    video_decoder_destroy(r);
err_create:
    return rc;
}

void nxserverlib_video_disconnect_sd_display(struct b_session *session)
{
    /* Go through every connection and disconnect video from the SD path. It's not supported with native 3D.
    TODO: There is no code to reconnect the SD path going out of native 3D. For now, app must reconnect. */
    nxclient_t client;
    for (client = BLST_D_FIRST(&session->clients); client; client = BLST_D_NEXT(client, session_link)) {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
            struct b_req *req = connect->req[b_resource_simple_video_decoder];
            unsigned i, j;
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                NEXUS_SimpleVideoDecoderServerSettings settings;
                if (!req || !req->handles.simpleVideoDecoder[i].handle) continue;
                NEXUS_SimpleVideoDecoder_GetServerSettings(session->video.server, req->handles.simpleVideoDecoder[i].handle, &settings);
                /* skip j=0 */
                for (j=1;j<NXCLIENT_MAX_DISPLAYS;j++) {
                    settings.window[j] = NULL;
                    settings.display[j] = NULL;
                }
                NEXUS_SimpleVideoDecoder_SetServerSettings(session->video.server, req->handles.simpleVideoDecoder[i].handle, &settings);
            }
        }
    }
}

void release_video_decoders(struct b_connect *connect)
{
    struct b_req *req;
    bool remainVisible = false;
    struct b_session * session;

    BDBG_OBJECT_ASSERT(connect, b_connect);
    session = connect->client->session;
    req = connect->req[b_resource_simple_video_decoder];

    if (req && req->handles.simpleVideoDecoder[0].r) {
        unsigned i;
        int stcIndex = -1;
        for (i=NXCLIENT_MAX_IDS-1;i<=NXCLIENT_MAX_IDS;i--) { /* reverse */
            NEXUS_SimpleVideoDecoderHandle videoDecoder;
            NEXUS_SimpleVideoDecoderServerSettings settings;
            unsigned index;

            if (!connect->settings.simpleVideoDecoder[i].id) continue;/* ned to continue since for loop reversed */

            if (get_req_index(req, get_video_decoder_req_id, connect->settings.simpleVideoDecoder[i].id, &index)) {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                continue;
            }
            videoDecoder = req->handles.simpleVideoDecoder[index].handle;

            if (i == 0 && videoDecoder) {
                NEXUS_VideoDecoderSettings decoderSettings;
                NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &decoderSettings);
                if (decoderSettings.channelChangeMode != NEXUS_VideoDecoder_ChannelChangeMode_eMute &&
                    decoderSettings.channelChangeMode != NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture) {
                    remainVisible = true;
                }
            }

            /* disconnect regular decoder from simple decoder */
            NEXUS_SimpleVideoDecoder_GetServerSettings(session->video.server, videoDecoder, &settings);
            if (i == 0) {
                stcIndex = settings.stcIndex;
            }
            settings.stcIndex = -1;
            {
                unsigned j;
                BDBG_MSG(("disconnect SimpleVideoDecoder %p from decoder %p, window0 %p, stcIndex %u", (void*)videoDecoder, (void*)settings.videoDecoder, (void*)settings.window[0], settings.stcIndex));
                settings.videoDecoder = NULL;
                for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                    settings.window[j] = NULL;
                    settings.display[j] = NULL;
                }
                (void)NEXUS_SimpleVideoDecoder_SetServerSettings(session->video.server, videoDecoder, &settings);
            }
        }
        video_decoder_release(req->handles.simpleVideoDecoder[0].r);
        if (stcIndex != -1) {
            video_release_stc_index(connect, stcIndex);
        }
        req->handles.simpleVideoDecoder[0].r = NULL;
    }

    release_video_window(connect, remainVisible);
}

int video_init(nxserver_t server)
{
    unsigned index = 0;
    NEXUS_VideoDecoderHandle videoDecoder;
    int rc = NEXUS_SUCCESS;

#if BCHP_CHIP == 7445
    if (server->platformStatus.boxMode == 15) {
        server->videoDecoder.special_mode = b_special_mode_linked_decoder2;
    }
#endif

    NEXUS_GetVideoDecoderCapabilities(&server->videoDecoder.cap);
#if NEXUS_NUM_VIDEO_ENCODERS
    NEXUS_GetVideoEncoderCapabilities(&server->videoEncoder.cap);
#endif

    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder]) {
        rc = server->settings.externalApp.allocIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_video_decoder, &index);
        if (rc) return BERR_TRACE(rc);
    }

    videoDecoder = NEXUS_VideoDecoder_Open(index, NULL);
    if (!videoDecoder) {rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto done;}
    NEXUS_SimpleVideoDecoderModule_LoadDefaultSettings(videoDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);

done:
    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder]) {
        server->settings.externalApp.freeIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_video_decoder, index);
    }
    return rc;
}

void video_uninit(void)
{
}

/* Return the SimpleStcChannel's stcIndex currently in use.
If none in use, return -1. */
int video_get_stc_index(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_video_decoder];
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        unsigned index;
        int stcIndex;

        if (!req || !connect->settings.simpleVideoDecoder[i].id) break;

        if (get_req_index(req, get_video_decoder_req_id, connect->settings.simpleVideoDecoder[i].id, &index)) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            continue;
        }

        /* disconnect regular decoder from simple decoder */
        NEXUS_SimpleVideoDecoder_GetStcIndex(connect->client->session->video.server, req->handles.simpleVideoDecoder[index].handle, &stcIndex);
        if (stcIndex != -1) {
            return stcIndex;
        }
    }
    return -1;
}

int nxserverlib_p_swap_video_windows(struct b_connect *connect1, struct b_connect *connect2)
{
    NEXUS_SurfaceClientHandle surfaceClient1, surfaceClient2;
    int rc;
    struct b_session *session = connect1->client->session;
    bool connect1_has_video = has_window(connect1);
    bool connect2_has_video = has_window(connect2);

    if (session != connect2->client->session) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (IS_MOSAIC(connect1) || IS_MOSAIC(connect2)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!connect1_has_video && !connect2_has_video) {
        /* neither has a window, so it's a no-op */
        return 0;
    }
    if (!connect1_has_video || !connect2_has_video) {
        /* if only one as a window, then either another client has grabbed one away, or they are contending on the same window.
        in either case, don't allow swap. */
        BDBG_ERR(("don't allow video window swap unless connects have a window"));
        return NEXUS_NOT_SUPPORTED;
    }

    /* do the swap */
    {
        struct b_req *req1 = connect1->req[b_resource_simple_video_decoder];
        struct b_req *req2 = connect2->req[b_resource_simple_video_decoder];
        NEXUS_SimpleVideoDecoder_SwapWindows(session->video.server,
            req1->handles.simpleVideoDecoder[0].handle,
            req2->handles.simpleVideoDecoder[0].handle);
    }

    /* swap accounting */
    {
        unsigned temp = connect1->windowIndex;
        connect1->windowIndex = connect2->windowIndex;
        connect2->windowIndex = temp;
        temp = connect2->settings.simpleVideoDecoder[0].windowCapabilities.type;
        connect2->settings.simpleVideoDecoder[0].windowCapabilities.type = connect1->settings.simpleVideoDecoder[0].windowCapabilities.type;
        connect1->settings.simpleVideoDecoder[0].windowCapabilities.type = temp;
    }
    session->window[connect1->windowIndex].connect = connect1;
    session->window[connect2->windowIndex].connect = connect2;

    rc = nxclient_get_surface_client(connect1, NULL, 0, &surfaceClient1);
    if (!rc) {
        rc = nxclient_get_surface_client(connect2, NULL, 0, &surfaceClient2);
    }
    if (!rc) {
        NEXUS_SurfaceCompositor_SwapWindows(session->surfaceCompositor,
            surfaceClient1, connect1->settings.simpleVideoDecoder[0].windowId,
            surfaceClient2, connect2->settings.simpleVideoDecoder[0].windowId);
    }
    return 0;
}

NEXUS_VideoDecoderHandle nxserver_p_get_video_decoder(struct b_connect *connect)
{
    struct b_req *req = connect->req[b_resource_simple_video_decoder];
    if (req && req->handles.simpleVideoDecoder[0].r) {
        return req->handles.simpleVideoDecoder[0].r->handle[0];
    }
    else {
        return NULL;
    }
}

#else
bool lacks_video(struct b_connect *connect) { return true; }
void nxserverlib_video_disconnect_sd_display(struct b_session *session) { }
void release_video_decoders(struct b_connect *connect) { }
int acquire_video_decoders(struct b_connect *connect, bool grab) { return 0; }
int nxserverlib_p_swap_video_windows(struct b_connect *connect1, struct b_connect *connect2) { return 0; }
void nxserverlib_p_clear_video_cache(void){ }
void uninit_session_video(struct b_session *session) { }
int video_init(nxserver_t server) { return 0; }
void video_uninit(void) { }
#endif
