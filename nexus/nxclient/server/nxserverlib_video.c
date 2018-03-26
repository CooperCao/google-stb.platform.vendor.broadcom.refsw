/******************************************************************************
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
    struct b_connect *mosaic_connect[NXCLIENT_MAX_IDS];
    NxClient_ConnectSettings connectSettings; /* copy of settings that were used when created */
    unsigned windowIndex; /* index into connect->client->session->window[] */
    struct b_session *session; /* needed for cleanup after connect/clients are gone */
};
static unsigned video_decoder_id(const NxClient_ConnectSettings *pSettings, unsigned i) {return pSettings->simpleVideoDecoder[i].id != NXCLIENT_CONNECT_BACKEND_MOSAIC ? pSettings->simpleVideoDecoder[i].id : 0;}
#define IS_VIRTUALIZED_MOSAIC(connect) ((connect)->settings.simpleVideoDecoder[0].decoderCapabilities.virtualized)
#define IS_MOSAIC_DECODER(connect) (video_decoder_id(&(connect)->settings, 1) || IS_VIRTUALIZED_MOSAIC(connect))
#define IS_MOSAIC_WINDOW(connect) ((connect)->settings.simpleVideoDecoder[1].id || IS_VIRTUALIZED_MOSAIC(connect))

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
static void video_decoder_destroy_for_connect(struct video_decoder_resource *r, struct b_connect *connect);

enum b_cap {
    b_cap_no,
    b_cap_possible,
    b_cap_preferred
};

/* convert enum to a comparable integer for a bandwidth equation.
consider 50==60 and 24==25==30. consider i60 == p30. consider 1920x1080==1280x720.
valid comparisons: 4Kp60 > 4Kp30 > 1080p60 > 1080i60/1080p30/720p60 > 480p60 > 480i
*/
static unsigned nxserver_p_video_comparator(unsigned width, unsigned height, bool interlaced, unsigned verticalFreq)
{
    if (verticalFreq >= 50) {
        verticalFreq = interlaced ? 30 : 60;
    }
    else {
        verticalFreq = 30;
    }
    if (width > 1920 || height > 1088) { /* UHD */
        return 200 + verticalFreq;
    }
    else if (width > 1280 || height > 720) { /* 1080p60/30 */
        return 100 + verticalFreq;
    }
    else if (width > 720 || height > 576) { /* 720p60, which is like 1080p30 */
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
    return nxserver_p_video_comparator(info.width, info.height, info.interlaced, info.verticalFreq/100);
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
            if (connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder) {
                if (server->settings.memConfigSettings.videoDecoder[index].secureTranscode != NEXUS_SecureVideo_eSecure) return b_cap_no;
            }
            else {
                switch(server->settings.memConfigSettings.videoDecoder[index].secure) {
                case NEXUS_SecureVideo_eUnsecure:
                case NEXUS_SecureVideo_eNone: /* Intentional fall through */
                    return b_cap_no;
                default:
                    break;
                }
            }
        }
        else {
            switch(server->settings.memConfigSettings.videoDecoder[index].secure) {
            case NEXUS_SecureVideo_eSecure:
            case NEXUS_SecureVideo_eNone: /* Intentional fall through */
                return b_cap_no;
            default:
                break;
            }
        }
    }

    if (IS_MOSAIC_DECODER(connect)) {
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
        if (!avail_mem || needed_mem > avail_mem) {
            return b_cap_no;
        }
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxFormat) {
        if (nxserver_p_videoformat_comparator(connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxFormat) >
            nxserver_p_videoformat_comparator(server->settings.memConfigSettings.videoDecoder[index].maxFormat)) return b_cap_no;
    }
    else if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxWidth || connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight) {
        if (nxserver_p_video_comparator(connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxWidth, connect->settings.simpleVideoDecoder[0].decoderCapabilities.maxHeight, false, assumed_framerate) >
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
        if (IS_MOSAIC_DECODER(connect)) {
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

static void video_decoder_p_find_decoder(struct b_connect *connect, unsigned assumed_framerate, unsigned *pindex, unsigned *pnum_avail)
{
    int i = NEXUS_MAX_VIDEO_DECODERS;
    *pnum_avail = 0;

    /* a reverse search generally finds the least capable decoder that meets requirements */
    while (--i != -1) {
        bool done = false;
        if (!connect->client->server->settings.memConfigSettings.videoDecoder[i].used) {
            continue;
        }
        if (!g_decoders[i].r || !g_decoders[i].r->used || IS_VIRTUALIZED_MOSAIC(connect)) {
            if (g_decoders[i].r && g_decoders[i].r->used && IS_VIRTUALIZED_MOSAIC(connect)) {
                /* used decoder must be on same window */
                if (g_decoders[i].r->connect->settings.simpleVideoDecoder[0].windowCapabilities.type != connect->settings.simpleVideoDecoder[0].windowCapabilities.type) {
                    continue;
                }
            }
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
        BKNI_AcquireMutex(session->server->settings.lock);
        session->hdmi.drm.input.eotf = streamInfo.eotf;
        session->hdmi.drm.eotfValid = true;
        nxserverlib_p_apply_hdmi_drm(session, NULL, false);
        BKNI_ReleaseMutex(session->server->settings.lock);
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
    if (server->platformConfig.heap[NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
    if (server->platformConfig.heap[NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
    if (server->platformConfig.heap[NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP]) {
        NEXUS_Platform_GetHeapRuntimeSettings(server->platformConfig.heap[NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP], &heapSettings);
        return heapSettings.secure;
    }
    return false;
}
#else
static bool nxserver_p_urr_on(nxserver_t server)
{
    /* counter-intuitive, but we want to use non-secure CRR in this case for test. */
    BSTD_UNUSED(server);
    return true;
}
#endif

static struct video_decoder_resource *video_decoder_create(struct b_connect *connect)
{
    unsigned index = TOTAL_DECODERS;
    int rc;
    struct video_decoder_resource *r = NULL;
    nxserver_t server = connect->client->server;
    struct b_session *session = connect->client->session;
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

    if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo || (server->settings.svp == nxserverlib_svp_type_cdb)) {
        if (connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder) {
            cdbHeap = server->platformConfig.heap[NEXUS_CRRT_HEAP];
            if (!cdbHeap) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
        }
        else if (nxserver_p_urr_on(server)) {
            /* Use CRR for video CDB if secure decoder/heaps AND sage toggle on (URR=on) */
            /* Otherwise use GLR for video CDB (CRR will NOT work) */
            /* All secure picture buffer heaps are toggled at once, just need to check one */
            cdbHeap = server->platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            if (!cdbHeap) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
        }
    }

    if (g_decoders[index].r) {
        r = g_decoders[index].r;
        if (r->connect && IS_VIRTUALIZED_MOSAIC(r->connect) && IS_VIRTUALIZED_MOSAIC(connect)) {
            /* allow virtualized mosaic to be extended */
        }
        else if (!r->used) {
            unsigned i;
            /* compare decoder capabilities that are only set at Open, # of mosaics */
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                if (IS_MOSAIC_DECODER(connect)) break; /* don't know if decoder is mosaic, so must destroy */
                if ((connect->settings.simpleVideoDecoder[i].id!=0) != (r->connectSettings.simpleVideoDecoder[i].id!=0)) break;
                if (connect->settings.simpleVideoDecoder[i].id) {
                    NEXUS_VideoDecoderOpenSettings openSettings;
                    const NxClient_VideoDecoderCapabilities *cap = &connect->settings.simpleVideoDecoder[i].decoderCapabilities;
                    if (!r->handle[i]) break;
                    NEXUS_VideoDecoder_GetOpenSettings(r->handle[i], &openSettings);
                    if ((cap->fifoSize && cap->fifoSize != openSettings.fifoSize) ||
                        (cap->itbFifoSize && cap->itbFifoSize != openSettings.itbFifoSize) ||
                        (cap->avc51Enabled != openSettings.avc51Enabled) ||
                        (cap->secureVideo != openSettings.secureVideo) ||
                        (cap->userDataBufferSize > openSettings.userDataBufferSize) ||
                        (cdbHeap != openSettings.cdbHeap)) break;
                }
            }
            if (i < NXCLIENT_MAX_IDS) {
                /* need to recreate for mismatch */
                video_decoder_destroy(r);
                r = NULL;
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

    if (!r) {
        r = nx_video_decoder_p_malloc(connect, index);
        if (!r) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc;
        }
        nxserverlib_p_clear_video_cache();
    }

    if (!IS_MOSAIC_DECODER(connect)) {
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
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.itbFifoSize) {
            openSettings.itbFifoSize = connect->settings.simpleVideoDecoder[0].decoderCapabilities.itbFifoSize;
        }
        openSettings.avc51Enabled = connect->settings.simpleVideoDecoder[0].decoderCapabilities.avc51Enabled;
        openSettings.enhancementPidChannelSupported = connect->settings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc];
        if (connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo) {
            if (connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder) {
                openSettings.secureVideo = NEXUS_VideoDecoderSecureType_eSecureTranscode;
            }
            else {
                openSettings.secureVideo = NEXUS_VideoDecoderSecureType_eSecure;
            }
        }
        openSettings.userDataBufferSize = connect->settings.simpleVideoDecoder[0].decoderCapabilities.userDataBufferSize;
        openSettings.cdbHeap = cdbHeap;
        /* if you modify openSettings here, search this file for NEXUS_VideoDecoder_GetOpenSettings where we test the cache for OpenSettings. */

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
        for (i=0;i<NXCLIENT_MAX_IDS && connect->settings.simpleVideoDecoder[i].id;i++) {
            NEXUS_VideoDecoderOpenMosaicSettings openSettings;
            NEXUS_VideoDecoderSettings settings;
            unsigned parentIndex, mosaicIndex;
            for (mosaicIndex=0;mosaicIndex<NXCLIENT_MAX_IDS;mosaicIndex++) {
                if (!r->handle[mosaicIndex]) break;
            }
            if (mosaicIndex == NXCLIENT_MAX_IDS) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }

            NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openSettings);
            if (connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth &&
                connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight) {
                openSettings.maxWidth = connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxWidth;
                openSettings.maxHeight = connect->settings.simpleVideoDecoder[i].decoderCapabilities.maxHeight;
            }
            if (connect->settings.simpleVideoDecoder[i].decoderCapabilities.fifoSize) {
                openSettings.openSettings.fifoSize = connect->settings.simpleVideoDecoder[i].decoderCapabilities.fifoSize;
            }
            if (connect->settings.simpleVideoDecoder[i].decoderCapabilities.itbFifoSize) {
                openSettings.openSettings.itbFifoSize = connect->settings.simpleVideoDecoder[i].decoderCapabilities.itbFifoSize;
            }
            openSettings.openSettings.secureVideo = connect->settings.simpleVideoDecoder[0].decoderCapabilities.secureVideo;
            openSettings.openSettings.cdbHeap = cdbHeap;
            parentIndex = r->index;
            r->handle[mosaicIndex] = NEXUS_VideoDecoder_OpenMosaic(parentIndex, mosaicIndex, &openSettings);
            if (!r->handle[mosaicIndex]) {
                rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                goto error;
            }
            r->mosaic_connect[mosaicIndex] = connect;

            NEXUS_VideoDecoder_GetSettings(r->handle[mosaicIndex], &settings);
            BDBG_MSG(("  mosaic videoDecoder[%d][%d]: %p, max %dx%d",
                      parentIndex, mosaicIndex, (void*)r->handle[mosaicIndex],
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

static void video_decoder_release(struct video_decoder_resource *r, struct b_connect *connect)
{
    nxserver_t server = r->session->server;
    BDBG_MSG(("video_decoder_release %p: connect %p, index %d", (void*)r, (void*)r->connect, r->index));
    BDBG_OBJECT_ASSERT(r->connect, b_connect);
    if (IS_MOSAIC_DECODER(r->connect) || server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder] ||
        server->settings.videoDecoder.dynamicPictureBuffers ||
        r->index >= NEXUS_NUM_VIDEO_DECODERS /* SID or DSP decoder */
        )
    {
        video_decoder_destroy_for_connect(r, connect);
    }
    else {
        /* cached. by not closing, we have faster start time if the same decoder/window connection is used, which is likely. */
        r->used = false;
        r->connect = NULL;
    }
}

static void video_decoder_destroy(struct video_decoder_resource *r)
{
    video_decoder_destroy_for_connect(r, NULL);
}

static void video_decoder_destroy_for_connect(struct video_decoder_resource *r, struct b_connect *connect)
{
    unsigned i;
    nxserver_t server = r->session->server;
    struct b_connect *remaining_connect = NULL;

    BDBG_MSG(("video_decoder_destroy %p: connect %p, index %d", (void*)r, (void*)r->connect, r->index));
    BDBG_ASSERT(g_decoders[r->index].r == r);

    if (connect) {
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (r->mosaic_connect[i] == connect) {
                NEXUS_SimpleVideoDecoderModule_CheckCache(r->session->video.server, NULL, r->handle[i]);
                NEXUS_VideoDecoder_Close(r->handle[i]);
                r->handle[i] = NULL;
                r->mosaic_connect[i] = NULL;
            }
            else if (r->mosaic_connect[i]) {
                remaining_connect = r->mosaic_connect[i];
            }
        }
        if (remaining_connect) {
            r->connect = remaining_connect;
            return;
        }
    }

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
    if (connect->client->session->window[connect->windowIndex].connect == connect) {
        return true;
    }
    if (connect->client->session->display[0].mosaic_connect[0]) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (connect->client->session->display[0].mosaic_connect[connect->windowIndex][i] == connect) return true;
        }
    }
    return false;
}

static int resize_full_screen(struct b_session *session, unsigned displayIndex, unsigned windowIndex, bool mosaic)
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
    /* size window to RTS max */
    settings.position.width = session->display[displayIndex].formatInfo.width * session->server->display.cap.display[displayIndex].window[windowIndex].maxWidthPercentage / 100;
    settings.position.height = session->display[displayIndex].formatInfo.height * session->server->display.cap.display[displayIndex].window[windowIndex].maxHeightPercentage / 100;
    /* if not mosaic, don't size PIP greater than quarter screen */
    if (!mosaic && windowIndex) {
        if (session->server->display.cap.display[displayIndex].window[windowIndex].maxWidthPercentage > 50) {
            settings.position.width = session->display[displayIndex].formatInfo.width / 2;
        }
        if (session->server->display.cap.display[displayIndex].window[windowIndex].maxHeightPercentage > 50) {
            settings.position.height = session->display[displayIndex].formatInfo.height / 2;
        }
        settings.position.x = session->display[displayIndex].formatInfo.width - settings.position.width;
    }
    else if (session->nxclient.displaySettings.display3DSettings.orientation == NEXUS_VideoOrientation_e3D_LeftRight) {
        settings.position.width /= 2;
    }
    else if (session->nxclient.displaySettings.display3DSettings.orientation == NEXUS_VideoOrientation_e3D_OverUnder) {
        settings.position.height /= 2;
    }
    settings.visible = true;
    return NEXUS_VideoWindow_SetSettings(window, &settings);
}

static void release_video_window(struct b_connect *connect);
static int acquire_video_window(struct b_connect *connect, bool grab)
{
    struct b_session *session = connect->client->session;
    unsigned index;
    unsigned i, j;
    int rc;
    unsigned window_index;

    if (!nxserver_p_allow_grab(connect->client)) {
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
    if (session->window[index].connect && !(IS_VIRTUALIZED_MOSAIC(session->window[index].connect) && IS_VIRTUALIZED_MOSAIC(connect))) {
        rc = NEXUS_NOT_AVAILABLE;
        if (grab && connect->client->session->window[index].connect && !IS_VIRTUALIZED_MOSAIC(connect->client->session->window[index].connect)) {
            struct b_connect *prev_connect = connect->client->session->window[index].connect;
            BDBG_MSG(("grab video window %d", index));
            release_video_decoders(prev_connect);
            if (session->window[index].connect) {
                rc = NEXUS_NOT_AVAILABLE;
            }
            else {
                release_video_window(prev_connect);
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
        if (session->display[j].display && !session->display[j].graphicsOnly) {
            /* regular window may already be open */
            if (!session->display[j].window[index][0]) {
                session->display[j].window[index][0] = NEXUS_VideoWindow_Open(session->display[j].display, index);
                if (!session->display[j].window[index][0]) {
                    rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    goto error;
                }
                if (j == 0 && index == 0) {
                    NEXUS_VideoWindowSettings settings;
                    /* by preferring synclock on main, if PIP is recreated first, it will be destroyed and
                    recreated when main is created. */
                    NEXUS_VideoWindow_GetSettings(session->display[j].window[index][0], &settings);
                    settings.preferSyncLock = true;
                    NEXUS_VideoWindow_SetSettings(session->display[j].window[index][0], &settings);
                }
            }

            if (IS_MOSAIC_WINDOW(connect)) {
                /* mosaics are not cached */
                if (!session->display[j].parentWindow[index]) {
                    session->display[j].parentWindow[index] = session->display[j].window[index][0];
                    session->display[j].window[index][0] = NULL;
                    NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, session->display[j].parentWindow[index], NULL);
                }
                for (i=0;i<NXCLIENT_MAX_IDS && connect->settings.simpleVideoDecoder[i].id;i++) {
                    unsigned window_index;
                    for (window_index=0;window_index<NXCLIENT_MAX_IDS;window_index++) {
                        if (!session->display[j].window[index][window_index]) break;
                    }
                    if (window_index == NXCLIENT_MAX_IDS) {
                        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                        goto error;
                    }
                    session->display[j].window[index][window_index] = NEXUS_VideoWindow_OpenMosaic(session->display[j].parentWindow[index], window_index);
                    if (!session->display[j].window[index][window_index]) {
                        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                        goto error;
                    }
                    session->display[j].mosaic_connect[index][window_index] = connect;
                }
            }
            if (!connect->settings.simpleVideoDecoder[0].surfaceClientId || IS_MOSAIC_WINDOW(connect)) {
                /* we need to make full screen and visible if app doesn't use SurfaceCompositor for video window.
                also, if we're going into mosaic mode, surface compositor assumes the parent window is full screen and visible. */
                resize_full_screen(session, j, index, IS_MOSAIC_WINDOW(connect));
            }
        }
    }

    /* connect to SurfaceCompositor */
    window_index = 0;
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

        if (session->display[0].mosaic_connect[connect->windowIndex][0]) {
            while (window_index<NXCLIENT_MAX_IDS && session->display[0].mosaic_connect[connect->windowIndex][window_index] != connect) window_index++;
            if (window_index == NXCLIENT_MAX_IDS) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto error;
            }
        }

        /*
        For NEXUS_SurfaceCompositorClientSettings.display[].window[INDEX], INDEX must be the AcquireVideoWindow index. It is unchanging in a swap.
        For session->display[].window[INDEX], INDEX is the BVN window, 0 = main, 1 = PIP. It changes in a swap.
        */
        NEXUS_SurfaceCompositor_GetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
        for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
            NEXUS_VideoWindowHandle window;

            window = session->display[j].window[connect->windowIndex][window_index];
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
        settings.backendMosaic = (connect->settings.simpleVideoDecoder[1].id == NXCLIENT_CONNECT_BACKEND_MOSAIC);
        rc = NEXUS_SurfaceCompositor_SetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
        if (rc) { rc = BERR_TRACE(rc); /* continue */ }

        window_index++;
    }

    return 0;

error:
    release_video_window(connect);
    return rc;
}

static void release_video_window(struct b_connect *connect)
{
    struct b_session *session = connect->client->session;
    unsigned i, j;
    struct b_connect *remaining_connect = NULL;

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
                    if (session->display[j].mosaic_connect[connect->windowIndex][i] == connect) {
                        NEXUS_VideoWindow_Close(session->display[j].window[connect->windowIndex][i]);
                        session->display[j].window[connect->windowIndex][i] = NULL;
                        session->display[j].mosaic_connect[connect->windowIndex][i] = NULL;
                    }
                    else if (session->display[j].mosaic_connect[connect->windowIndex][i]) {
                        remaining_connect = session->display[j].mosaic_connect[connect->windowIndex][i];
                    }
                }

                if (!remaining_connect) {
                    /* if no more mosaics, change mosaic parent to regular window */
                    session->display[j].window[connect->windowIndex][0] = session->display[j].parentWindow[connect->windowIndex];
                    session->display[j].parentWindow[connect->windowIndex] = NULL;
                }
            }
        }
    }

    /* unreserve or throw ownership of window */
    session->window[connect->windowIndex].connect = remaining_connect;
    connect->windowIndex = 0; /* don't care */
}

void nxserverlib_video_close_windows(struct b_session *session, unsigned local_display_index)
{
    unsigned i, index;
    for (index=0;index<NEXUS_NUM_VIDEO_WINDOWS;index++) {
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (session->display[local_display_index].window[index][i]) {
                NEXUS_VideoWindow_Close(session->display[local_display_index].window[index][i]);
                session->display[local_display_index].window[index][i] = NULL;
                session->display[local_display_index].mosaic_connect[index][i] = NULL;
            }
        }
        if (session->display[local_display_index].parentWindow[index]) {
            NEXUS_VideoWindow_Close(session->display[local_display_index].parentWindow[index]);
            session->display[local_display_index].parentWindow[index] = NULL;
        }
    }
}

void uninit_session_video(struct b_session *session)
{
    unsigned j;
    NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, NULL, NULL);
    for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
        if (session->display[j].display) {
            nxserverlib_video_close_windows(session, j);
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
    unsigned decoder_index, window_index;

    if (!connect->settings.simpleVideoDecoder[0].id) {
        /* no request */
        return 0;
    }

    if (req->handles.simpleVideoDecoder[0].r) {
        if (req->handles.simpleVideoDecoder[0].r->connect == connect) {
            BDBG_WRN(("already connected"));
            return 0;
        }
        else {
            BDBG_ERR(("already connected to %p", (void*)req->handles.simpleVideoDecoder[0].r->connect));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }

    rc = acquire_video_window(connect, grab);
    if (rc) return rc;

    /* one connect always resolves into one video_decoder_create */
    r = video_decoder_create(connect);
    if (!r) {
        if (grab) {
            for (i=0;i<TOTAL_DECODERS;i++) {
                struct video_decoder_resource *grab_from = g_decoders[i].r;
                if (grab_from && (!grab_from->connect || !IS_VIRTUALIZED_MOSAIC(grab_from->connect))) {
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

    decoder_index = 0;
    window_index = 0;
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleVideoDecoderHandle videoDecoder;
        NEXUS_SimpleVideoDecoderServerSettings settings;
        unsigned index;
        int rc;

        if (!video_decoder_id(&connect->settings, i)) break;

        if (get_req_index(req, get_video_decoder_req_id, connect->settings.simpleVideoDecoder[i].id, &index)) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_set;
        }
        videoDecoder = req->handles.simpleVideoDecoder[index].handle;

        if (r->mosaic_connect[0]) {
            /* find next mosaic decoder/window for this connect */
            while (decoder_index<NXCLIENT_MAX_IDS && r->mosaic_connect[decoder_index] != connect) decoder_index++;
            if (decoder_index == NXCLIENT_MAX_IDS) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_set;
            }

            while (window_index<NXCLIENT_MAX_IDS && session->display[0].mosaic_connect[connect->windowIndex][window_index] != connect) window_index++;
            if (window_index == NXCLIENT_MAX_IDS) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_set;
            }
        }

        /* connect regular decoder to simple decoder */
        NEXUS_SimpleVideoDecoder_GetServerSettings(session->video.server, videoDecoder, &settings);
        settings.videoDecoder = r->handle[decoder_index];
        if (has_window(connect)) {
            unsigned j;
            for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                if (j>0 && (nxserverlib_p_native_3d_active(session) || nxserverlib_p_dolby_vision_active(session))) {
                    continue;
                }
                settings.window[j] = session->display[j].window[connect->windowIndex][window_index];
                if (settings.window[j]) {
                    NEXUS_SimpleVideoDecoderModule_CheckCache(session->video.server, settings.window[j], settings.videoDecoder);
                    settings.display[j] = session->display[j].display; /* closedCaptionRouting */
                }
            }
#if NEXUS_NUM_MOSAIC_DECODES
            if (connect->settings.simpleVideoDecoder[1].id == NXCLIENT_CONNECT_BACKEND_MOSAIC) {
                unsigned k;
                for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                    BDBG_CASSERT(NEXUS_NUM_MOSAIC_DECODES <= NXCLIENT_MAX_IDS);
                    for (k=0;k<NEXUS_NUM_MOSAIC_DECODES;k++) {
                        settings.backendMosaic.display[j].window[k] = session->display[j].window[connect->windowIndex][k];
                    }
                }
            }
#endif
        }
        settings.stcIndex = stcIndex;
        settings.mainWindow = (connect->settings.simpleVideoDecoder[0].windowCapabilities.type == NxClient_VideoWindowType_eMain);

        BDBG_MSG(("connect SimpleVideoDecoder %p to decoder %p, window0 %p, stcIndex %u", (void*)videoDecoder, (void*)settings.videoDecoder, (void*)settings.window[0], settings.stcIndex));
        rc = NEXUS_SimpleVideoDecoder_SetServerSettings(session->video.server, videoDecoder, &settings);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_set;
        }

        decoder_index++;
        window_index++;
    }
    return 0;

err_set:
    release_video_decoders(connect);
    return rc;
err_stcindex:
    video_decoder_destroy_for_connect(r, connect);
err_create:
    return rc;
}

void nxserverlib_video_disconnect_display(nxserver_t server, NEXUS_DisplayHandle display)
{
    nxclient_t client;
    for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
            struct b_req *req = connect->req[b_resource_simple_video_decoder];
            unsigned i, j;
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                NEXUS_SimpleVideoDecoderServerSettings settings;
                bool change = false;
                if (!req || !req->handles.simpleVideoDecoder[i].handle) continue;
                NEXUS_SimpleVideoDecoder_GetServerSettings(client->session->video.server, req->handles.simpleVideoDecoder[i].handle, &settings);
                for (j=0;j<NXCLIENT_MAX_DISPLAYS;j++) {
                    if (settings.display[j] == display) {
                        settings.window[j] = NULL;
                        settings.display[j] = NULL;
                        BKNI_Memset(&settings.backendMosaic.display[j], 0, sizeof(settings.backendMosaic.display[j]));
                        change = true;
                    }
                }
                if (change) {
                    NEXUS_SimpleVideoDecoder_SetServerSettings(client->session->video.server, req->handles.simpleVideoDecoder[i].handle, &settings);
                }
            }
        }
    }
}

void release_video_decoders(struct b_connect *connect)
{
    struct b_req *req;
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

            if (!video_decoder_id(&connect->settings, i)) continue;/* need to continue since for loop reversed */

            if (get_req_index(req, get_video_decoder_req_id, connect->settings.simpleVideoDecoder[i].id, &index)) {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                continue;
            }
            videoDecoder = req->handles.simpleVideoDecoder[index].handle;

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
                BKNI_Memset(&settings.backendMosaic, 0, sizeof(settings.backendMosaic));
                (void)NEXUS_SimpleVideoDecoder_SetServerSettings(session->video.server, videoDecoder, &settings);
            }
        }
        video_decoder_release(req->handles.simpleVideoDecoder[0].r, connect);
        if (stcIndex != -1) {
            video_release_stc_index(connect, stcIndex);
        }
        req->handles.simpleVideoDecoder[0].r = NULL;
    }

    release_video_window(connect);
}

int video_init(nxserver_t server)
{
    NEXUS_GetVideoDecoderCapabilities(&server->videoDecoder.cap);
#if NEXUS_NUM_VIDEO_ENCODERS
    NEXUS_GetVideoEncoderCapabilities(&server->videoEncoder.cap);
#endif

    return NEXUS_SUCCESS;
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

        if (!req || !video_decoder_id(&connect->settings, i)) break;

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
    if (IS_MOSAIC_WINDOW(connect1) || IS_MOSAIC_WINDOW(connect2)) {
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
bool lacks_video(struct b_connect *connect) { BSTD_UNUSED(connect); return true; }
void release_video_decoders(struct b_connect *connect) { BSTD_UNUSED(connect); }
int acquire_video_decoders(struct b_connect *connect, bool grab) { BSTD_UNUSED(connect); BSTD_UNUSED(grab); return 0; }
int nxserverlib_p_swap_video_windows(struct b_connect *connect1, struct b_connect *connect2) { BSTD_UNUSED(connect1);BSTD_UNUSED(connect2);return 0; }
void nxserverlib_p_clear_video_cache(void){ }
void uninit_session_video(struct b_session *session) {BSTD_UNUSED(session);}
int video_init(nxserver_t server) { BSTD_UNUSED(server);return 0; }
void video_uninit(void) { }
void nxserverlib_video_disconnect_display(nxserver_t server, NEXUS_DisplayHandle display) {BSTD_UNUSED(server);BSTD_UNUSED(display);}
void nxserverlib_video_close_windows(struct b_session *session, unsigned local_display_index) {BSTD_UNUSED(session);BSTD_UNUSED(local_display_index);}
int video_get_stc_index(struct b_connect *connect) {BSTD_UNUSED(connect);return 0;}
#endif
