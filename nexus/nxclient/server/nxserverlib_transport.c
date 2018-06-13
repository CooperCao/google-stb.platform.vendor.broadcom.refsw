/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nxserverlib_impl.h"
#if NEXUS_HAS_TRANSPORT
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#endif
#include "nexus_transport_capabilities.h"

BDBG_MODULE(nxserverlib_transport);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

NEXUS_Error stc_pool_init(nxserver_t server)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TransportCapabilities xptCaps;
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderCapabilities videoCaps;
#endif
#if NEXUS_HAS_AUDIO
    NEXUS_AudioCapabilities audioCaps;
#endif
    struct b_stc * pStc, * prev = NULL;
    unsigned i;

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_GetVideoDecoderCapabilities(&videoCaps);
#endif
#if NEXUS_HAS_AUDIO
    NEXUS_GetAudioCapabilities(&audioCaps);
#endif
    NEXUS_GetTransportCapabilities(&xptCaps);

    /* nxclient assumes timebase0 is reserved for display and audio. */
    nxserver_p_reserve_timebase(NEXUS_Timebase_e0);
    BDBG_MSG(("numStc: xpt=%d, vid=%d, aud=%d",
        xptCaps.numStcs, videoCaps.numStcs, audioCaps.numStcs));

    for (i = 0; i < xptCaps.numStcs; i++)
    {
        pStc = BKNI_Malloc(sizeof(struct b_stc));
        if (!pStc) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
        BKNI_Memset(pStc, 0, sizeof(struct b_stc));
        pStc->index = i;

        /*
         * according VICE team, encoder stc count always matches xpt stc count
         * if this changes, we can add API to GetVideoEncoderCaps
         */
        pStc->caps.encode = true;
        pStc->score++;
#if NEXUS_HAS_VIDEO_DECODER
        if (i < videoCaps.numStcs)
        {
            pStc->caps.video = true;
            pStc->score++;
        }
#endif
#if NEXUS_HAS_AUDIO
        if (i < audioCaps.numStcs)
        {
            pStc->caps.audio = true;
            pStc->score++;
        }
#endif
        if (!prev) {
            BLST_D_INSERT_HEAD(&server->transport.stcs, pStc, link);
        }
        else {
            BLST_D_INSERT_AFTER(&server->transport.stcs, prev, pStc, link);
        }
        prev = pStc;
    }

end:
    return rc;

error:
    goto end;
}

void stc_pool_uninit(nxserver_t server)
{
    struct b_stc * pStc;

    for (pStc = BLST_D_FIRST(&server->transport.stcs); pStc; pStc = BLST_D_FIRST(&server->transport.stcs))
    {
        if (pStc->refcnt)
        {
            BDBG_WRN(("STC pool uninitialized with referenced stcs outstanding: %p:%u", (void*)pStc, pStc->refcnt));
            BERR_TRACE(NEXUS_LEAKED_RESOURCE);
        }
        BLST_D_REMOVE_HEAD(&server->transport.stcs, link);
        BKNI_Free(pStc);
    }
}

static struct b_stc * stc_find_by_index(nxserver_t server, unsigned index)
{
    struct b_stc * pStc = NULL;

    for (pStc = BLST_D_FIRST(&server->transport.stcs); pStc; pStc = BLST_D_NEXT(pStc, link))
    {
        if (pStc->index == index)
        {
            break;
        }
    }

    return pStc;
}

static bool stc_caps_meets_request(const struct b_stc_caps * pReq, const struct b_stc_caps * pCaps)
{
    bool met = true;

    if
    (
        pReq
        &&
        (
            (pReq->audio && !pCaps->audio)
            ||
            (pReq->video && !pCaps->video)
            ||
            (pReq->encode && !pCaps->encode)
        )
    )
    {
        met = false;
    }

    return met;
}

static struct b_stc * stc_find_where_unused_and_caps_meets_request_and_min_score(nxserver_t server, const struct b_stc_caps * pReq)
{
    struct b_stc * pStc = NULL;
    struct b_stc * pMinScoreStc = NULL;
    unsigned minScore = 0xffffffff;

    BDBG_MSG_TRACE(("stc find where: %c%c%c",
        pReq->video ? 'V' : 'X',
        pReq->audio ? 'A' : 'X',
        pReq->encode ? 'E' : 'X'
    ));

    for (pStc = BLST_D_FIRST(&server->transport.stcs); pStc; pStc = BLST_D_NEXT(pStc, link))
    {
        if (!pStc->refcnt && stc_caps_meets_request(pReq, &pStc->caps) && (pStc->score < minScore))
        {
            minScore = pStc->score;
            pMinScoreStc = pStc;
        }
    }

    return pMinScoreStc;
}

static struct b_stc * stc_acquire(nxserver_t server, const struct b_stc_caps * pReq)
{
    struct b_stc * pStc = NULL;

    BDBG_ASSERT(server);

    /* TODO: need to atomicize check and reserve */
    pStc = stc_find_where_unused_and_caps_meets_request_and_min_score(server, pReq);
    if (pStc)
    {
        BDBG_MSG(("STC%u acquired", pStc->index));
        pStc->refcnt++;
    }
    else
    {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    return pStc;
}

static void stc_release(struct b_stc * pStc)
{
    if (pStc)
    {
        if (pStc->refcnt == 1)
        {
            pStc->refcnt = 0;
            BDBG_MSG(("STC%u released", pStc->index));
        }
        else if (pStc->refcnt > 1)
        {
            BDBG_WRN(("STC released when still in use - refcnt:%u", pStc->refcnt));
        }
    }
}

void stc_index_request_init(struct b_connect * connect, struct b_stc_caps * pStcReq)
{
    BKNI_Memset(pStcReq, 0, sizeof(struct b_stc_caps));
    pStcReq->video = connect->settings.simpleVideoDecoder[0].id != 0;
    pStcReq->audio = connect->settings.simpleAudioDecoder.id != 0;
    if (!nxserver_p_connect_is_nonrealtime_encode(connect) && (pStcReq->audio || pStcReq->video))
    {
        /*
         * For non-NRT usages, the connect/set/start of audio comes separately
         * from that for video. We need to thus assume av sessions when a or v
         * is requested.  This prevents us from using an audio-only or
         * video-only stc index, but we can still use encoder-only stc indices.
         * In the NRT case, we still assign audio or video separately from
         * stcs whose capabilities meet those respective requests.
         */
        pStcReq->video = pStcReq->audio = true;
    }
}

int stc_index_acquire(struct b_connect *connect, const struct b_stc_caps * pStcReq)
{
    struct b_stc * pStc = NULL;
    int stcIndex = -1;
    bool nonRealTime;
    nxserver_t server = connect->client->server;

    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_stc_index]) {
        unsigned index;
        int rc = server->settings.externalApp.allocIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_stc_index, &index);
        if (rc) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            return -1;
        }
        return index;
    }

    nonRealTime = nxserver_p_connect_is_nonrealtime_encode(connect);

    if (pStcReq->video && pStcReq->audio && !pStcReq->encode && !nonRealTime)
    {
        int audioStcIndex;
        int videoStcIndex;
        /*
         * AV only stc request -> we can reuse other AV stc indices
         * The first two conditions here ensure that all audios and videos in the
         * same connect request will get the same STC index.  This includes mosaics,
         * but not primers, as primers don't use Connect.
         */

        if ((audioStcIndex = audio_get_stc_index(connect)) >= 0)
        {
            BDBG_MSG(("selecting stc index %d because of connected audio decoder", audioStcIndex));
            pStc = stc_find_by_index(connect->client->server, (unsigned)audioStcIndex);
            if (!pStc) { BDBG_ERR(("Audio has invalid stc index %d", audioStcIndex)); goto error; }
        }
        else if ((videoStcIndex = video_get_stc_index(connect)) >= 0)
        {
            BDBG_MSG(("selecting stc index %d because of connected video decoder", videoStcIndex));
            pStc = stc_find_by_index(connect->client->server, (unsigned)videoStcIndex);
            if (!pStc) { BDBG_ERR(("Video has invalid stc index %d", videoStcIndex)); goto error; }
        }
    }

    /*
     * for encodes, video-only decodes, audio-only decodes, or some combo of
     * these without an STC already, we allocate a new STC index
     */
    if (!pStc)
    {
        /* allocate new stc index */
        pStc = stc_acquire(connect->client->server, pStcReq);
        if (!pStc) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error; }
        BDBG_MSG_TRACE(("allocated new stc index %u", pStc->index));
    }

    /* all ways of getting an stc index above will inc the refcnt here */
    if (pStc)
    {
        pStc->refcnt++;
        stcIndex = (int)pStc->index;
        BDBG_MSG_TRACE(("stc%u req:%c%c%c refs:%u->%u", pStc->index,
            pStcReq->video ? 'V' : 'X',
            pStcReq->audio ? 'A' : 'X',
            pStcReq->encode ? 'E' : 'X',
            pStc->refcnt-1,
            pStc->refcnt));
    }

error:
    return stcIndex;
}

void stc_index_release(struct b_connect *connect, int stcIndex)
{
    struct b_stc * pStc;
    nxserver_t server = connect->client->server;

    if (server->settings.externalApp.enableAllocIndex[nxserverlib_index_type_stc_index]) {
        server->settings.externalApp.freeIndex(server->settings.externalApp.callback_context, nxserverlib_index_type_stc_index, stcIndex);
        return;
    }

    if (stcIndex >= 0)
    {
        pStc = stc_find_by_index(connect->client->server, (unsigned)stcIndex);
        if (pStc)
        {
            if (pStc->refcnt > 1)
            {
                pStc->refcnt--;

                BDBG_MSG_TRACE(("stc%u refs:%u->%u", pStc->index, pStc->refcnt+1, pStc->refcnt));

            }
            else
            {
                BDBG_ERR(("stc_index_release called on stc that has already been released"));
                BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }

            /* internally, acquire also incs refcnt to mark as used */
            if (pStc->refcnt == 1)
            {
                stc_release(pStc);
            }
        }
    }
}
#else
NEXUS_Error stc_pool_init(nxserver_t server) { return 0; }
void stc_pool_uninit(nxserver_t server) { }
#endif

void nxserver_p_reserve_timebase(NEXUS_Timebase timebase)
{
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
    NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
}
