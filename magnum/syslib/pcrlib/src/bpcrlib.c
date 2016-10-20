/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bpcrlib.h"
#include "blst_list.h"
#include "bkni.h"

BDBG_MODULE(pcrlib);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

enum BPCRlib_Decoder_State {
    BPCRlib_Decoder_State_eNotLocked,
    BPCRlib_Decoder_State_eLocked,
    BPCRlib_Decoder_State_eLocking,
    BPCRlib_Decoder_State_eWaitingSTC,
    BPCRlib_Decoder_State_eInvalidated
};

#include "bxpt_pcr_offset.h"

#define BPCRlib_P_PCR_FIFO  32

typedef struct {
    uint32_t last_pcr;
    bool    last_pcr_valid;
    unsigned index;
    unsigned count;
    int32_t pcr_fifo[BPCRlib_P_PCR_FIFO];
    bool    pcr_offset_valid;
    uint32_t pcr_offset;
}BPCRlib_P_PCROffset;

struct BPCRlib_P_Channel {
    BLST_D_ENTRY(BPCRlib_P_Channel) link;
    BPCRlib_Handle parent;
    BXPT_PCR_Handle pcr;
    BPCRlib_ChannelSettings settings;
    enum BPCRlib_Decoder_State video_state;
    enum BPCRlib_Decoder_State audio_state;
    BPCRlib_Config cfg;

    BPCRlib_P_PCROffset pcr_offset;

    struct {
        enum {delayed_stc_eInvalid, delayed_stc_eLocked, delayed_stc_eWaiting} state;
        uint32_t stc; /* STC to be applied to the decoder */
        uint32_t old_stc; /* snapshot of STC from the running counter */
    } delayed_stc;

    /* added to see if we get consecutive pts errors */
    struct
    {
        uint32_t uiLastDroppedFrameCount;
        uint32_t uiLastDecodedFrameCount;
        uint32_t uiConsecutivePtsErrorCount;
    } sVideo;
};

/* threshold to detect discontinuity in the streame */
#define BPCRLIB_DISC_TRESHOLD   ((3003*30*5)/4)

struct BPCRlib_P_Handle {
    BLST_D_HEAD(BPCRlib_P_Handle_list, BPCRlib_P_Channel) list;
};

static void BPCRlib_P_InvalidatePCRCache_isr(BPCRlib_Channel channel);

BERR_Code
BPCRlib_Open(BPCRlib_Handle *pHandle, BCHP_Handle hChip)
{
    BPCRlib_Handle handle;

    BSTD_UNUSED(hChip);

    BDBG_ASSERT(BPCRlib_StcDiff_isrsafe(false, 16349, 4294746617U)==118514);
    BDBG_ASSERT(BPCRlib_StcDiff_isrsafe(false, 4294746617U, 16349)==-118514);
    BDBG_ASSERT(BPCRlib_StcDiff_isrsafe(false, 16349, 0)==8174);
    BDBG_ASSERT(BPCRlib_StcDiff_isrsafe(false, 0, 16349)==-8174);

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BLST_D_INIT(&handle->list);
    *pHandle = handle;

    return BERR_SUCCESS;
}

void
BPCRlib_Close(BPCRlib_Handle handle)
{
    BPCRlib_Channel pcr;

    while((pcr=BLST_D_FIRST(&handle->list))!=NULL) {
        BLST_D_REMOVE_HEAD(&handle->list, link);
        BKNI_Free(pcr);
    }
    BKNI_Free(handle);
    return;
}

static const BPCRlib_Config pcr_cfg = {
    false,
    BAVC_StreamType_eTsMpeg,
    NULL, /* primary audio */
    NULL,
    NULL, /* video */
    NULL,
    NULL, /* secondary audio */
    NULL,
    NULL, /* tertiary audio */
    NULL,
    NULL, /* aux transport */
    96 * 1024, /* video CDB level */
    ((3003*4)/2), /* video PTS offset, 120ms */
    (3003/2)*30, /* video STC discard threshold, 1 sec */
    4 * 1024,  /* audio CDB level */
    ((3003*2)/2), /* audio PTS offset, 60ms */
    (3003/2)*120, /* audio STC discard threshold, 4 sec */
    (3003/2)*2, /* pcr_offset 60 ms delay */
    (3003/2)*30*10, /* pcr_discard 10 sec delay */
    BPCRlib_Mode_eAutoPts,
    false, /* is_playback_ip */
    BPCRlib_TsmMode_eSTCMaster, /* tsm_mode */
    5000, /* sync_limit */
    NULL, /* dec flush event handle */
    8, /* consecutive pts error limit */
    true, /* refresh stc on invalidate */
    false, /* non realtime mode */
    false /* paired with another channel */
};

BERR_Code
BPCRlib_Channel_GetChannelDefSettings(BPCRlib_Handle handle, BPCRlib_ChannelSettings *config)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(config, 0, sizeof(*config));
    return BERR_SUCCESS;
}

BERR_Code
BPCRlib_Channel_Create(BPCRlib_Handle handle, BXPT_PCR_Handle pcr, BPCRlib_Channel *pChannel, const BPCRlib_ChannelSettings *settings )
{
    BPCRlib_Channel channel;

    channel = BKNI_Malloc(sizeof(*channel));
    if (!channel) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_EnterCriticalSection();
    channel->settings = *settings;
    channel->pcr = pcr;
    channel->cfg = pcr_cfg;
    channel->parent=handle;
    channel->audio_state = BPCRlib_Decoder_State_eWaitingSTC;
    channel->video_state = BPCRlib_Decoder_State_eWaitingSTC;
    channel->delayed_stc.state = delayed_stc_eInvalid;

    BPCRlib_P_InvalidatePCRCache_isr(channel);

    BLST_D_INSERT_HEAD(&handle->list, channel, link);
    BKNI_LeaveCriticalSection();

    *pChannel = channel;

    return BERR_SUCCESS;
}

void
BPCRlib_Channel_Destroy(BPCRlib_Channel channel)
{
    BKNI_EnterCriticalSection();
    BLST_D_REMOVE(&channel->parent->list, channel, link);
    BKNI_LeaveCriticalSection();
    BKNI_Free(channel);
    return;
}

void
BPCRlib_Channel_GetConfig(BPCRlib_Channel channel, BPCRlib_Config *config)
{
    *config = channel->cfg;
    return;
}

BERR_Code
BPCRlib_Channel_SetConfig( BPCRlib_Channel channel, const BPCRlib_Config *config)
{
    BPCRlib_Channel ch;

    if (config == NULL)
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    if (config->audio!=NULL && config->audio == config->secondary_audio)
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    if (config->audio!=NULL && config->audio == config->tertiary_audio)
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    /* coverity[var_compare_op: FALSE] */
    if ((config->audio_iface==NULL && config->audio!=NULL) ||
        (config->video_iface==NULL && config->video!=NULL) ||
        (config->secondary_audio_iface==NULL && config->secondary_audio!=NULL) ||
        (config->tertiary_audio_iface==NULL && config->tertiary_audio!=NULL))
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    /* coverity[var_deref_op: FALSE] */
    if (config->aux_transport==NULL &&
            ((config->audio && config->audio_iface->useAuxTrp) ||
             (config->secondary_audio && config->secondary_audio_iface->useAuxTrp) ||
             (config->tertiary_audio && config->tertiary_audio_iface->useAuxTrp) ||
             (config->video && config->video_iface->useAuxTrp)))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BDBG_MSG(("pcr old config %p: video %p audio %p,%p,%p", (void *)channel, (void *)channel->cfg.video, (void *)channel->cfg.audio, (void *)channel->cfg.secondary_audio, (void *)channel->cfg.tertiary_audio));
    BDBG_MSG(("pcr new config %p: video %p audio %p,%p,%p", (void *)channel, (void *)config->video, (void *)config->audio, (void *)config->secondary_audio, (void *)config->tertiary_audio));
    BKNI_EnterCriticalSection();
    for(ch=BLST_D_FIRST(&channel->parent->list);ch;ch=BLST_D_NEXT(ch,link)) {
        if (ch==channel) {
            /* skip a current channel */
            continue;
        }
        BDBG_MSG(("pcr %p: video %p audio %p,%p,%p", (void *)ch, (void *)ch->cfg.video, (void *)ch->cfg.audio, (void *)ch->cfg.secondary_audio, (void *)ch->cfg.tertiary_audio));
        /* coverity[copy_paste_error] */
        if (
               (config->video && (config->video == ch->cfg.video))
            || (config->audio && ((config->audio == ch->cfg.audio) || (config->audio == ch->cfg.secondary_audio) || (config->audio == ch->cfg.tertiary_audio)))
            || (config->secondary_audio && ((config->secondary_audio == ch->cfg.audio) || (config->secondary_audio == ch->cfg.secondary_audio) || (config->secondary_audio == ch->cfg.tertiary_audio)))
            || (config->tertiary_audio && ((config->tertiary_audio == ch->cfg.audio) || (config->tertiary_audio == ch->cfg.secondary_audio) || (config->tertiary_audio == ch->cfg.tertiary_audio)))
            ) {
            BKNI_LeaveCriticalSection();
            BDBG_ERR(("Selected audio or video channel currently assigned to other pcrlib channel"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    if (channel->cfg.playback!= config->playback) {
        BPCRlib_P_InvalidatePCRCache_isr(channel);
    }
    /* when decoders changes they go to unlocked state */
    if (channel->cfg.video != config->video) {
        channel->video_state = BPCRlib_Decoder_State_eWaitingSTC;
        BPCRlib_P_InvalidatePCRCache_isr(channel);
    }
    if (channel->cfg.audio != config->audio) {
        channel->audio_state = BPCRlib_Decoder_State_eWaitingSTC;
        BPCRlib_P_InvalidatePCRCache_isr(channel);
    }
    if (channel->cfg.video != config->video || channel->cfg.audio != config->audio || channel->cfg.playback!= config->playback) {
        channel->delayed_stc.state= delayed_stc_eInvalid;
    }
    channel->cfg = *config;
    BKNI_LeaveCriticalSection();
    /* Note, we don't reconfigure audio/video decoders at pcrlib */
    return BERR_SUCCESS;
}

static BPCRlib_Channel
find_by_audio_isr(BPCRlib_Handle handle, void *audio)
{
    BPCRlib_Channel chn;
    BDBG_ASSERT(audio);
    for(chn=BLST_D_FIRST(&handle->list);chn;chn=BLST_D_NEXT(chn,link)) {
        if (chn->cfg.audio==audio || chn->cfg.secondary_audio==audio || chn->cfg.tertiary_audio==audio) {
            /* we exploit fact that audio and secondary audio can't be shared */
            goto found;
        }
    }
    BDBG_WRN(("Unknown audio channel %p", (void *)audio));
found:
    return chn;
}

static BPCRlib_Channel
find_by_video_isr(BPCRlib_Handle handle, void *video)
{
    BPCRlib_Channel chn;

    BDBG_ASSERT(video);
    for(chn=BLST_D_FIRST(&handle->list);chn;chn=BLST_D_NEXT(chn,link)) {
        if (chn->cfg.video==video) {
            goto found;
        }
    }
    BDBG_WRN(("Unknown video channel %p", (void *)video));
found:
    return chn;
}



/*
* this function takes STC in the 45KHz domain (MPEG) or 27MHz domain (DSS)
* as 32 bit unsigned and returns delta in the 22.5KHz domain, 32 bit signed
*/
int32_t
BPCRlib_StcDiff_isrsafe(bool dss, uint32_t stc1, uint32_t stc2)
{
    int32_t delta, modulo;

    /* 1. convert to the 22.5KHz domain */
    if (dss) {
        stc1 /= 1200;
        stc2 /= 1200;
        modulo = (1<<30)/600;
    } else {
        stc1 /= 2;
        stc2 /= 2;
        modulo = (1<<30);
    }

    delta = stc1 - stc2;
    /* 2. Handle wraparond cases */
    if (stc1 < stc2 && delta < -modulo) {
        delta += modulo*2;
    } else if (stc1 > stc2 && delta > modulo) {
        delta -= modulo*2;
    }
    return delta;
}

static bool
b_is_dss_isrsafe(const BPCRlib_Config *cfg)
{
    return cfg->stream == BAVC_StreamType_eDssPes || cfg->stream == BAVC_StreamType_eDssEs;
}

/* 20060717 bandrews - need update for send_stc to match update_stc for requestor behavior */
static BERR_Code
b_really_send_stc_isr(const BPCRlib_Channel chn, uint32_t stc)
{
    BERR_Code rc=BERR_SUCCESS;
    bool dss = b_is_dss_isrsafe(&chn->cfg);
    uint32_t old_stc;

    if (chn->cfg.mode == BPCRlib_Mode_eConstantDelay) {
        rc = BXPT_PcrOffset_SetStc_isr(chn->cfg.aux_transport, stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("BXPT_PcrOffset_SetStc_isr returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.video && chn->cfg.video_iface->setStc) {
        BDBG_MSG(("updating video:STC %#x", stc));
        rc = chn->cfg.video_iface->getStc(chn->cfg.aux_transport, chn->cfg.video, &old_stc);
        rc = chn->cfg.video_iface->setStc(chn->cfg.video_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.video, dss,stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("video setStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.audio && chn->cfg.audio_iface->setStc) {
        BDBG_MSG(("updating audio: STC %#x", stc));
        rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, &old_stc);
        rc = chn->cfg.audio_iface->setStc(chn->cfg.audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.audio, dss, stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio setStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.secondary_audio && chn->cfg.secondary_audio_iface->setStc) {
        BDBG_MSG(("updating secondary_audio: STC %#x", stc));
        rc = chn->cfg.secondary_audio_iface->setStc(chn->cfg.secondary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.secondary_audio, dss, stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("secondary audio setStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.tertiary_audio && chn->cfg.tertiary_audio_iface->setStc) {
        BDBG_MSG(("updating tertiary_audio: STC %#x", stc));
        rc = chn->cfg.tertiary_audio_iface->setStc(chn->cfg.tertiary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.tertiary_audio, dss, stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("tertiary audio setStc returned error %#x, ignored", rc)); }
    }
    return rc;
}

static BERR_Code
b_send_stc_isr(const BPCRlib_Channel chn, uint32_t stc, void * requestor)
{
    BERR_Code rc=BERR_SUCCESS;
    bool dss = b_is_dss_isrsafe(&chn->cfg);

    /*
        20060717 bandrews -
        if video requests -> set video, primary audio, and secondary audio
        if primary audio requests -> set video, primary audio, and secondary audio
        if secondary audio requests -> set secondary audio only
    */
    if (requestor == chn->cfg.video || requestor == chn->cfg.audio)
    {
        rc = b_really_send_stc_isr(chn, stc);
    }
    else /* secondary or tertiary audio */
    {
        if ( requestor == chn->cfg.secondary_audio )
        {
            if (chn->cfg.secondary_audio && chn->cfg.secondary_audio_iface->setStc) {
                BDBG_MSG(("updating secondary_audio: STC %#x", stc));
                rc = chn->cfg.secondary_audio_iface->setStc(chn->cfg.secondary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.secondary_audio, dss, stc);
                if (rc!=BERR_SUCCESS) { BDBG_ERR(("secondary audio setStc returned error %#x, ignored", rc)); }
            }
        }
        else /* tertiary */
        {
            if (chn->cfg.tertiary_audio && chn->cfg.tertiary_audio_iface->setStc) {
                BDBG_MSG(("updating tertiary_audio: STC %#x", stc));
                rc = chn->cfg.tertiary_audio_iface->setStc(chn->cfg.tertiary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, chn->cfg.tertiary_audio, dss, stc);
                if (rc!=BERR_SUCCESS) { BDBG_ERR(("tertiary audio setStc returned error %#x, ignored", rc)); }
            }
        }
    }

    return rc;
}

static bool
b_read_stc_isr(const BPCRlib_Channel chn, uint32_t *stc)
{
    BERR_Code rc;

    if (chn->cfg.audio && chn->audio_state!=BPCRlib_Decoder_State_eWaitingSTC && chn->cfg.audio_iface->getStc ) {
      rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, stc);
      if (rc==BERR_SUCCESS) { return true; }
    }
    if (chn->cfg.video && chn->video_state!=BPCRlib_Decoder_State_eWaitingSTC && chn->cfg.video_iface->getStc) {
      rc = chn->cfg.video_iface->getStc(chn->cfg.aux_transport, chn->cfg.video, stc);
      if (rc==BERR_SUCCESS) { return true; }
    }
    return false;
}

/* 20060630 bandrews - this is going to get ugly... more hacks */
static BERR_Code
b_really_update_stc_isr(const BPCRlib_Channel chn, bool is_request_stc)
{
    BERR_Code rc=BERR_SUCCESS;

    if (chn->cfg.video && chn->cfg.video_iface->updateStc) {
        rc = chn->cfg.video_iface->updateStc(chn->cfg.video_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("video updateStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.audio && chn->cfg.audio_iface->updateStc) {
        rc = chn->cfg.audio_iface->updateStc(chn->cfg.audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio updateStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.secondary_audio && chn->cfg.secondary_audio_iface->updateStc) {
        rc = chn->cfg.secondary_audio_iface->updateStc(chn->cfg.secondary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio updateStc returned error %#x, ignored", rc)); }
    }
    if (chn->cfg.tertiary_audio && chn->cfg.tertiary_audio_iface->updateStc) {
        rc = chn->cfg.tertiary_audio_iface->updateStc(chn->cfg.tertiary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
        if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio updateStc returned error %#x, ignored", rc)); }
    }

    return rc;
}
static BERR_Code
b_update_stc_isr(const BPCRlib_Channel chn, void * requestor, bool is_request_stc)
{
    BERR_Code rc=BERR_SUCCESS;

    /*
        20060630 bandrews -
        if video requests -> update video, primary audio, and secondary audio
        if primary audio requests -> update video, primary audio, and secondary audio
        if secondary audio requests -> update secondary audio only
    */
    if (requestor == chn->cfg.video || requestor == chn->cfg.audio)
    {
        rc = b_really_update_stc_isr(chn, is_request_stc);
    }
    else /* secondary or tertiary audio */
    {
        if ( requestor == chn->cfg.secondary_audio )
        {
            if (chn->cfg.secondary_audio && chn->cfg.secondary_audio_iface->updateStc) {
                rc = chn->cfg.secondary_audio_iface->updateStc(chn->cfg.secondary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
                if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio updateStc returned error %#x, ignored", rc)); }
            }
        }
        else /* tertiary */
        {
            if (chn->cfg.tertiary_audio && chn->cfg.tertiary_audio_iface->updateStc) {
                rc = chn->cfg.tertiary_audio_iface->updateStc(chn->cfg.tertiary_audio_iface->useAuxTrp?chn->cfg.aux_transport:chn->pcr, is_request_stc);
                if (rc!=BERR_SUCCESS) { BDBG_ERR(("audio updateStc returned error %#x, ignored", rc)); }
            }
        }
    }

    return rc;
}

static BERR_Code
b_load_delayed_stc_isr(BPCRlib_Channel chn, void *requestor)
{
    BERR_Code rc=BERR_SUCCESS;
    uint32_t cur_stc;
    uint32_t stc;
    BSTD_UNUSED(requestor);

    switch(chn->delayed_stc.state) {
    case delayed_stc_eInvalid:
        BDBG_WRN(("b_load_delayed_stc_isr: %#lx(%#lx) ignored, STC is not avaliable yet", (unsigned long)chn, (unsigned long)requestor));
        break;
    case delayed_stc_eLocked:
    case delayed_stc_eWaiting:
        cur_stc = BXPT_PcrOffset_GetStc_isr(chn->cfg.aux_transport) + BXPT_PcrOffset_GetOffset_isr(chn->cfg.aux_transport);
        stc = chn->delayed_stc.stc + (cur_stc - chn->delayed_stc.old_stc);
        BDBG_WRN(("b_load_delayed_stc_isr: %#lx(%#lx) loading STC %#x (%u + %d)", (unsigned long)chn, (unsigned long)requestor, (unsigned)stc, (unsigned)chn->delayed_stc.stc, (int)(cur_stc - chn->delayed_stc.old_stc)));
        rc = b_send_stc_isr(chn, stc, requestor);
        /* update delayed_stc, so following  call to b_load_delayed_stc_isr would not use stale values */
        chn->delayed_stc.stc = stc;
        chn->delayed_stc.old_stc = cur_stc;
        chn->delayed_stc.state = delayed_stc_eLocked;
        break;
    }
    return rc;
}



static bool
BPCRlib_TestAudioStc_isr(BPCRlib_Channel chn, uint32_t new_stc, uint32_t pts)
{
    int32_t delta;
    bool dss = b_is_dss_isrsafe(&chn->cfg);

    delta = BPCRlib_StcDiff_isrsafe(dss, new_stc, pts);

    if (
        (delta >= 0 && delta < (2*chn->cfg.audio_pts_offset + (3003 * 5)/(2*2))) ||
        (delta < 0 && delta > -(2*chn->cfg.audio_pts_offset + (3003 * 60 )/(2*2)))
    ) /* video STC could be  0.15 sec ahead or 2.0 sec behind of PTS */
    {
        BDBG_MSG(("TestAudioStc: STC %#x PTS %#x delta %d (%s)", (unsigned)new_stc, (unsigned)pts, (int)delta, "good"));
        return true;
    } else {
        BDBG_MSG(("TestAudioStc: STC %#x PTS %#x delta %d (%s)", (unsigned)new_stc, (unsigned)pts, (int)delta, "bad"));
        return false;
    }
}

static bool
BPCRlib_P_GetStcFromPcr_isr(BPCRlib_Channel chn, uint32_t pts, uint32_t *new_stc)
{
    int32_t delta;
    int32_t offset;
    unsigned i;
    bool dss = b_is_dss_isrsafe(&chn->cfg);

    if (!chn->pcr_offset.last_pcr_valid) {
        return false;
    }
    if (chn->pcr_offset.count==0) {
        delta = BPCRlib_StcDiff_isrsafe(dss, pts, chn->pcr_offset.last_pcr);
        BDBG_WRN(("[pcr init]%s use PCR:  %#x %#x %d", delta<0?" can't":"", (unsigned)pts, (unsigned)chn->pcr_offset.last_pcr, (int)delta));
        if (delta<0) {
            return false;
        }
        *new_stc = chn->pcr_offset.last_pcr - chn->cfg.pcr_offset;
        chn->pcr_offset.last_pcr_valid = false; /* use own PCR not more than one time */
        return true;
    }
    if(chn->pcr_offset.pcr_offset_valid) {
        bool stc_valid;
        uint32_t stc;

        chn->pcr_offset.pcr_offset_valid=false;
        stc_valid = b_read_stc_isr(chn, &stc);
        if (stc_valid) {
            *new_stc = stc + chn->pcr_offset.pcr_offset;
            BDBG_WRN(("use PCR offset %d: PTS %#x PCR %#x old_STC %#x new_STC %#x", (int)chn->pcr_offset.pcr_offset, (unsigned)pts, (unsigned)chn->pcr_offset.last_pcr, (unsigned)stc, (unsigned)*new_stc));
            return true;
        }
    }
    for(offset=0,i=0;i<chn->pcr_offset.count;i++) {
        offset += chn->pcr_offset.pcr_fifo[i];
    }
    offset /= chn->pcr_offset.count;
    BDBG_MSG(("[pcr offset] median offset:  %d", (int)offset));
    if (2*offset > chn->cfg.pcr_offset) {
        offset -= (chn->cfg.pcr_offset/4); /* decresed buffer fullness a little bit */
    }
    *new_stc = chn->pcr_offset.last_pcr - 2 * offset;
    delta = BPCRlib_StcDiff_isrsafe(dss, pts, *new_stc);
    BDBG_WRN(("[pcr offset %d]%s use PCR: PTS %#x PCR %#x STC %#x %d(%d)", 2*offset, (delta*2<chn->cfg.pcr_offset)?""/*can't"*/:"", (unsigned)pts, (unsigned)chn->pcr_offset.last_pcr, (unsigned)*new_stc, (int)delta*2, (int)chn->cfg.pcr_offset));
#if 0
    if (delta*2<chn->cfg.pcr_offset) {
        return false;
    }
#endif
    return true;
}

static bool
BPCRlib_TestVideoStc_isr(BPCRlib_Channel chn, uint32_t new_stc, uint32_t pts)
{
    int32_t delta;
    bool dss = b_is_dss_isrsafe(&chn->cfg);

    delta = BPCRlib_StcDiff_isrsafe(dss, new_stc, pts);

    if (
        (delta >= 0 && delta < (chn->cfg.video_pts_offset + (3003 * 5)/(2*2))) ||
        (delta < 0 && delta > -(chn->cfg.video_pts_offset + (3003 * 60)/(2*2)))
    ) /* audio STC could be  0.15 sec ahead or 2.0 sec behind of PTS */
    {
        BDBG_MSG(("TestVideoStc: STC %#x PTS %#x delta %d (%s)", (unsigned)new_stc, (unsigned)pts, (int)delta, "good"));
        return true;
    } else {
        BDBG_MSG(("TestVideoStc: STC %#x PTS %#x delta %d (%s)", (unsigned)new_stc, (unsigned)pts, (int)delta, "bad"));
        return false;
    }
}

static bool
BPCRlib_IsAudioLocked_isr(BPCRlib_Channel chn, uint32_t *locked_stc)
{
    BERR_Code rc;
    BAVC_PTSInfo pts;

    if (!chn->cfg.audio) {
        return false;
    }
    BDBG_ASSERT(chn->cfg.audio_iface->getStc);
    rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, locked_stc);
    if (rc!=BERR_SUCCESS) {
        return false;
    }

    BDBG_ASSERT(chn->cfg.audio_iface->getPts);
    rc = chn->cfg.audio_iface->getPts(chn->cfg.audio, &pts);
    if (rc!=BERR_SUCCESS) {
        return false;
    }
    return BPCRlib_TestAudioStc_isr(chn, *locked_stc, pts.ui32CurrentPTS);
}

static bool
BPCRlib_IsVideoLocked_isr(BPCRlib_Channel chn, uint32_t *locked_stc)
{
    BAVC_PTSInfo pts;
    BERR_Code rc;

    if (!chn->cfg.video) {
        return false;
    }
    BDBG_ASSERT(chn->cfg.video_iface->getPts);
    rc = chn->cfg.video_iface->getPts(chn->cfg.video, &pts);
    if (rc!=BERR_SUCCESS) {
        return false;
    }
    if (pts.ePTSType == BAVC_PTSType_eInterpolatedFromInvalidPTS) {
        return false;
    }
    BDBG_ASSERT(chn->cfg.video_iface->getStc);
    rc = chn->cfg.video_iface->getStc(chn->cfg.aux_transport, chn->cfg.video, locked_stc);
    if (rc!=BERR_SUCCESS) {
        return false;
    }
    return BPCRlib_TestVideoStc_isr(chn, *locked_stc, pts.ui32CurrentPTS);
}

static bool
BPCRlib_IsVideoBufferLevel_isr(BPCRlib_Channel chn)
{
    unsigned cdb_level;
    BERR_Code rc;

    BDBG_ASSERT(chn->cfg.video_iface->getCdbLevel);
    rc = chn->cfg.video_iface->getCdbLevel(chn->cfg.video, &cdb_level);
    if (rc!=BERR_SUCCESS) {
        BDBG_WRN(("Video buffer level request has returned error %d", cdb_level));
        return false;
    }
    BDBG_MSG(("video VBV %u(%d)", cdb_level, chn->cfg.video_cdb_level));
    return cdb_level > 96 * 1024; /* if there is more then 96KBytes in the VBV bufffer, video is just fine with STC */
}

#if 0
static bool
BPCRlib_IsAudioBufferLevel_isr(BPCRlib_Channel chn)
{
    unsigned cdb_level;
    BERR_Code rc;

    rc = chn->cfg.audio_iface->getCdbLevel(chn->cfg.audio, &cdb_level);
    if (rc!=BERR_SUCCESS) {
        BDBG_WRN(("Audio buffer level request has returned error %d", cdb_level));
        return false;
    }
    BDBG_MSG(("audio CDB %u(%d)", cdb_level, chn->cfg.audio_cdb_level));
    return cdb_level > 1 * 1024; /* if there is more then 1KBytes in the CDB bufffer, audio is just fine with STC */
}
#endif

static BPCRlib_TsmMode BPCRlib_Channel_GetTsmMode_isrsafe(BPCRlib_Channel chn)
{
    switch (chn->cfg.tsm_mode) {
    case BPCRlib_TsmMode_eVideoMaster:
        if (!chn->cfg.paired && !chn->cfg.video) {
            return BPCRlib_TsmMode_eSTCMaster;
        }
        break;
    case BPCRlib_TsmMode_eAudioMaster:
        if (!chn->cfg.paired && !chn->cfg.audio && !chn->cfg.secondary_audio && !chn->cfg.tertiary_audio) {
            return BPCRlib_TsmMode_eSTCMaster;
        }
        break;
    default:
        break;
    }
    return chn->cfg.tsm_mode;
}

BERR_Code
BPCRlib_Channel_AudioRequestStc_isr(BPCRlib_Handle handle, void *audio, uint32_t audio_pts)
{
    BPCRlib_Channel chn = find_by_audio_isr(handle, audio);
    BERR_Code rc = BERR_SUCCESS;
    bool dss = false;

    if (!chn) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("(%p) AudioRequestStc(%p) PTS %#x %s", (void *)chn, (void *)audio, (unsigned)audio_pts, chn->cfg.playback?"Playback":""));
    dss = b_is_dss_isrsafe(&chn->cfg);
    if (chn->cfg.playback) {
        if (chn->cfg.mode == BPCRlib_Mode_eConstantDelay) {
            return b_load_delayed_stc_isr(chn, audio);
        }

        switch (BPCRlib_Channel_GetTsmMode_isrsafe(chn))
        {
            case BPCRlib_TsmMode_eVideoMaster:

                if (chn->video_state != BPCRlib_Decoder_State_eWaitingSTC && chn->video_state != BPCRlib_Decoder_State_eInvalidated && chn->cfg.video)
                {
                    uint32_t new_stc = 0;

                    if (chn->video_state != BPCRlib_Decoder_State_eNotLocked)
                    {
                        if (BPCRlib_IsVideoLocked_isr(chn, &new_stc))
                        {
                            BDBG_MSG(("AudioRequestStc(%p): Video master mode -> using video STC %#x as new STC", (void *)audio, (unsigned)new_stc));
                        }
                        else
                        {
                            BDBG_MSG(("AudioRequestStc(%p): [video not locked] Video master mode -> awaiting video request", (void *)audio));
                            goto no_seed;
                        }
                    }
                    else if (BPCRlib_IsVideoLocked_isr(chn, &new_stc) && BPCRlib_IsVideoBufferLevel_isr(chn))
                    {
                        BDBG_MSG(("AudioRequestStc(%p): [VBV/Fifo] Video master mode -> using video STC %#x as new STC", (void *)audio, (unsigned)new_stc));
                    }
                    else
                    {
                        BDBG_MSG(("AudioRequestStc(%p): [video not locked] Video master mode -> awaiting video request", (void *)audio));
                        goto no_seed;
                    }

                    rc = b_send_stc_isr(chn, new_stc, audio);
                }
                else
                {
                    BDBG_MSG(("AudioRequestStc(%p): [video not locked] Video master mode -> awaiting video request", (void *)audio));
                }
no_seed:
                break;

            case BPCRlib_TsmMode_eAudioMaster:

                if (chn->cfg.secondary_audio == audio || chn->cfg.tertiary_audio == audio)
                {
                    uint32_t new_stc;
#if BDBG_DEBUG_BUILD
                    const char *channel_name = (chn->cfg.secondary_audio == audio)?"secondary":"tertiary";
#endif

                    if (chn->cfg.audio == NULL)
                    {
                        BDBG_ERR(("(%p) AudioRequestStc called in audio master mode for %s channel when primary is not set",(void *)audio, channel_name));
                        return BERR_TRACE(BERR_INVALID_PARAMETER);
                    }

                    rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, &new_stc);
                    if (rc != BERR_SUCCESS)
                    {
                        return rc;
                    }

                    /* if primary audio is locked, use its STC, otherwise, wait for primary audio to lock and load its STC */
                    if (chn->audio_state != BPCRlib_Decoder_State_eWaitingSTC && chn->cfg.audio)
                    {
                        if (chn->audio_state != BPCRlib_Decoder_State_eNotLocked)
                        {
                            if (BPCRlib_IsAudioLocked_isr(chn, &new_stc))
                            {
                                BDBG_MSG(("AudioRequestStc(%p): Audio master mode -> %s channel, using primary audio STC %#x as new STC", (void *)audio, channel_name, (unsigned)new_stc));
                                rc = b_send_stc_isr(chn, new_stc, audio);
                            }
                        }
                        else if (BPCRlib_IsAudioLocked_isr(chn, &new_stc))
                        {
                            BDBG_MSG(("AudioRequestStc(%p): [CDB/Fifo] Audio master mode -> %s channel, using primary audio STC %#x as new STC", (void *)audio, channel_name, (unsigned)new_stc));
                            rc = b_send_stc_isr(chn, new_stc, audio);
                        }
                    }
                    else
                    {
                        BDBG_MSG(("AudioRequestStc(%p): Audio master mode -> %s channel request before primary channel request, ignored", (void *)audio, channel_name));
                    }
                }
                else
                {
                    uint32_t new_stc = 0;
                    new_stc = audio_pts;
                    BDBG_MSG(("AudioRequestStc(%p): Audio master mode -> using audio PTS %#x as new STC %#x", (void *)audio, (unsigned)audio_pts, (unsigned)new_stc));
                    BPCRlib_P_InvalidatePCRCache_isr(chn);
                    chn->audio_state = BPCRlib_Decoder_State_eLocked;
                    rc = b_send_stc_isr(chn, new_stc, audio);
                }

                break;

            case BPCRlib_TsmMode_eSTCMaster:
            default:

                if (chn->cfg.secondary_audio == audio || chn->cfg.tertiary_audio == audio)
                {
                    uint32_t new_stc = 0;
#if BDBG_DEBUG_BUILD
                    const char *channel_name = (chn->cfg.secondary_audio == audio)?"secondary":"tertiary";
#endif

                    if (chn->cfg.audio==NULL)
                    {
                        BDBG_ERR(("(%p) AudioRequestStc called for %s channel when primary is not set", (void *)chn, channel_name));
                        return BERR_TRACE(BERR_INVALID_PARAMETER);
                    }

                    rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, &new_stc);
                    if (rc!=BERR_SUCCESS)
                    {
                        BDBG_ERR(("AudioRequestStc(%p): error retrieving STC", (void *)audio));
                        return rc;
                    }

                    /* if primary audio is locked, use its STC, otherwise, wait for primary audio to lock and load its STC */
                    if (chn->audio_state != BPCRlib_Decoder_State_eWaitingSTC && chn->cfg.audio)
                    {
                        if (chn->audio_state != BPCRlib_Decoder_State_eNotLocked)
                        {
                            /* PR45126 20081022 bandrews - Don't check if primary audio is locked if pcrlib state says it is */
                            BDBG_MSG(("AudioRequestStc(%p): %s channel, using primary audio STC %#x as new STC", (void *)audio, channel_name, (unsigned)new_stc));
                            rc = b_send_stc_isr(chn, new_stc, audio);
                        }
                        else if (BPCRlib_IsAudioLocked_isr(chn, &new_stc))
                        {
                            BDBG_MSG(("AudioRequestStc(%p): [CDB/Fifo] %s channel, using primary audio STC %#x as new STC", (void *)audio, channel_name, (unsigned)new_stc));
                            b_send_stc_isr(chn, new_stc, audio);
                        }
                        else
                        {
                            BDBG_MSG(("AudioRequestStc(%p): secondary audio channel, primary audio state eNotLocked and IsAudioLocked returned false", (void *)audio));
                        }
                    }
                    else
                    {
                        BDBG_MSG(("AudioRequestStc(%p): %s channel request before primary channel request, ignored", (void *)audio, channel_name));
                    }
                }
                else
                {
                    uint32_t new_stc = 0;

                    if(b_read_stc_isr(chn, &new_stc) && BPCRlib_TestAudioStc_isr(chn, new_stc, audio_pts))
                    {
                        BDBG_MSG(("AudioRequestStc(%p): primary audio reusing old STC %#x as new STC", (void *)audio, (unsigned)new_stc));
                        goto set_stc;
                    }
                    if (chn->video_state != BPCRlib_Decoder_State_eWaitingSTC && chn->cfg.video)
                    {
                        if (chn->video_state != BPCRlib_Decoder_State_eNotLocked)
                        {
                            if (BPCRlib_IsVideoLocked_isr(chn, &new_stc))
                            {
                                BDBG_MSG(("AudioRequestStc(%p): primary audio Using video STC %#x as new STC", (void *)audio, (unsigned)new_stc));
                                goto set_stc;
                            }
                        }
                        else if (BPCRlib_IsVideoBufferLevel_isr(chn) && BPCRlib_IsVideoLocked_isr(chn, &new_stc))
                        {
                            BDBG_MSG(("AudioRequestStc(%p): primary audio [VBV/Fifo] Using video STC %#x as new STC", (void *)audio, (unsigned)new_stc));
                            goto set_stc;
                        }
                    }
                    if (BPCRlib_P_GetStcFromPcr_isr(chn, audio_pts, &new_stc) && BPCRlib_TestAudioStc_isr(chn, new_stc, audio_pts))
                    {
                        BDBG_MSG(("AudioRequestStc(%p): primary audio Using PCR %#x as new STC %#x", (void *)audio, (unsigned)chn->pcr_offset.last_pcr, (unsigned)new_stc));
                        BPCRlib_P_InvalidatePCRCache_isr(chn);
                        goto set_stc;
                    }
                    if (chn->cfg.non_real_time)
                    {
                        new_stc = audio_pts;
                    }
                    else
                    {
                        new_stc = audio_pts - chn->cfg.audio_pts_offset;
                    }
                    BDBG_MSG(("AudioRequestStc(%p): primary audio Using audio PTS %#x as new STC %#x", (void *)audio, (unsigned)audio_pts, (unsigned)new_stc));
                    BPCRlib_P_InvalidatePCRCache_isr(chn);
set_stc:
                    chn->audio_state = BPCRlib_Decoder_State_eLocked;
                    rc = b_send_stc_isr(chn, new_stc, audio);
                }
                break;
        } /* switch */
    }
    else /* live */
    {
        return b_update_stc_isr(chn, audio, true);
    }

    return rc;
}

/* delta is in 22.5 KHz domain */
#define BPCRLIB_DELTA_TO_MILLISECONDS(X) (((X) * 2) / 45)
#define BPCRLIB_MILLISECONDS_TO_DELTA(X) (((X) * 45) / 2)

BERR_Code
BPCRlib_Channel_VideoRequestStc_isr(BPCRlib_Handle handle, void *video, const BAVC_PTSInfo *video_pts)
{
    BPCRlib_Channel chn = find_by_video_isr(handle, video);
    BERR_Code rc = BERR_SUCCESS;
    bool dss = false;

    BDBG_ASSERT(video_pts);
    if (!chn) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("(%p) VideoRequestStc(%p) PTS %#x(%d) %s", (void *)chn, (void *)video, (unsigned)video_pts->ui32CurrentPTS, (int)video_pts->ePTSType, chn->cfg.playback?"Playback":""));
    dss = b_is_dss_isrsafe(&chn->cfg);
    if (chn->cfg.playback)
    {
        uint32_t new_stc = 0;

        chn->sVideo.uiConsecutivePtsErrorCount = 0;
        chn->sVideo.uiLastDecodedFrameCount = 0;
        chn->sVideo.uiLastDroppedFrameCount = 0;

        if (chn->cfg.mode == BPCRlib_Mode_eConstantDelay) {
            return b_load_delayed_stc_isr(chn, video);
        }

        switch (BPCRlib_Channel_GetTsmMode_isrsafe(chn))
        {
            case BPCRlib_TsmMode_eAudioMaster:

                if (BPCRlib_IsAudioLocked_isr(chn, &new_stc) && chn->audio_state == BPCRlib_Decoder_State_eLocked)
                {
                    BDBG_MSG(("VideoRequestStc(%p): Audio master mode -> using audio STC %#x as new STC", (void *)video, (unsigned)new_stc));
                    chn->video_state = BPCRlib_Decoder_State_eLocked;
                    rc = b_send_stc_isr(chn, new_stc, video);
                }
                else
                {
                    BDBG_MSG(("VideoRequestStc(%p): [audio not locked] Audio master mode -> awaiting audio request", (void *)video));
                }
                break;

            case BPCRlib_TsmMode_eVideoMaster:

                if (video_pts->ePTSType == BAVC_PTSType_eInterpolatedFromInvalidPTS) {
                    BDBG_MSG(("VideoRequestStc(%p): Video master mode -> interpolated PTS ignored, STC not updated", (void *)video));
                    chn->video_state = BPCRlib_Decoder_State_eWaitingSTC;
                    return BERR_SUCCESS;
                }

                if (chn->cfg.non_real_time)
                {
                    new_stc = video_pts->ui32CurrentPTS;
                }
                else
                {
                    new_stc = video_pts->ui32CurrentPTS - chn->cfg.video_pts_offset;
                }
                BDBG_MSG(("VideoRequestStc(%p): Video master mode -> using video PTS %#x as new STC", (void *)video, (unsigned)new_stc));
                BPCRlib_P_InvalidatePCRCache_isr(chn);

                chn->video_state = BPCRlib_Decoder_State_eLocked;
                rc = b_send_stc_isr(chn, new_stc, video);
                break;

            case BPCRlib_TsmMode_eSTCMaster:
            default:

                if (BPCRlib_IsAudioLocked_isr(chn, &new_stc) && chn->audio_state==BPCRlib_Decoder_State_eLocked) {
                    int32_t delta;

                    delta = BPCRlib_StcDiff_isrsafe(dss, new_stc, video_pts->ui32CurrentPTS);
                    /* if audio is more 2 seconds behind video, use video PTS
                     * delta = STC - vPTS to match decoder TSM numbers
                     * audio behind video above means:
                     * vPTS - STC > 2 s, which is -delta > 2 s, or delta < -2 s
                     */
                    if (delta < -(BPCRLIB_MILLISECONDS_TO_DELTA(2000)))
                    {
                        goto set_video_stc;
                    }
                    BDBG_MSG(("VideoRequestStc(%p): Using audio STC %#x as new STC", (void *)video, (unsigned)new_stc));
                    goto set_stc;
                }

                if (video_pts->ePTSType == BAVC_PTSType_eInterpolatedFromInvalidPTS) {
                    if (chn->cfg.audio) {
                        rc = chn->cfg.audio_iface->getStc(chn->cfg.aux_transport, chn->cfg.audio, &new_stc);
                        if (rc==BERR_SUCCESS) {
                            BDBG_MSG(("VideoRequestStc(%p): Interpolated PTS ignored, using audio STC %#x", (void *)video, (unsigned)new_stc));
                            goto set_stc;
                        }
                    }
                    BDBG_MSG(("VideoRequestStc(%p): Interpolated PTS ignored, STC not updated", (void *)video));
                    chn->video_state = BPCRlib_Decoder_State_eWaitingSTC;
                    return BERR_SUCCESS;
                }
                if(b_read_stc_isr(chn, &new_stc) && BPCRlib_TestVideoStc_isr(chn, new_stc, video_pts->ui32CurrentPTS)) {
                    BDBG_MSG(("VideoRequestStc(%p): reusing old STC %#x as new STC", (void *)video, (unsigned)new_stc));
                    goto set_stc;
                }
                if (BPCRlib_P_GetStcFromPcr_isr(chn, video_pts->ui32CurrentPTS, &new_stc)
                        /* && BPCRlib_TestVideoStc_isr(chn, new_stc, video_pts->ui32CurrentPTS)*/
                    ) {
                    BDBG_MSG(("VideoRequestStc(%p): Using PCR %#x as new STC %#x", (void *)video, (unsigned)chn->pcr_offset.last_pcr, (unsigned)new_stc));
                    BPCRlib_P_InvalidatePCRCache_isr(chn);
                    goto set_stc;
                }
set_video_stc:
                if (chn->cfg.non_real_time)
                {
                    new_stc = video_pts->ui32CurrentPTS;
                }
                else
                {
                    new_stc = video_pts->ui32CurrentPTS - chn->cfg.video_pts_offset;
                }
                BDBG_MSG(("VideoRequestStc(%p): Using video PTS %#x as new STC", (void*)video, (unsigned)new_stc));
                BPCRlib_P_InvalidatePCRCache_isr(chn);
set_stc:
                chn->video_state = BPCRlib_Decoder_State_eLocked;
                rc = b_send_stc_isr(chn, new_stc, video);
                break;

        } /* switch */
    }
    else
    {
        return b_update_stc_isr(chn, video, true);
    }
    return rc;
}

BERR_Code
BPCRlib_Channel_VideoPtsError_isr( BPCRlib_Handle handle, void *video, const BAVC_PTSInfo *video_pts, uint32_t video_stc)
{
    BPCRlib_Channel chn = find_by_video_isr(handle, video);
    BERR_Code rc = BERR_SUCCESS;
    unsigned cdb_level;
    int pts_offset;
    bool dss = false;

    BDBG_ASSERT(video_pts);

    if (!chn) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    dss = b_is_dss_isrsafe(&chn->cfg);
    BDBG_MSG(("(%p) VideoPtsError(%p) PTS %#x(%d) STC %#x %d %s %u", (void *)chn, (void *)video, (unsigned)video_pts->ui32CurrentPTS, (int)video_pts->ePTSType, (unsigned)video_stc,  2*BPCRlib_StcDiff_isrsafe(dss, video_stc, video_pts->ui32CurrentPTS), chn->cfg.playback?"Playback":"", chn->cfg.video_iface->getCdbLevel?chn->cfg.video_iface->getCdbLevel(chn->cfg.video, &cdb_level),cdb_level:0));
    if (chn->cfg.playback) {
        uint32_t new_stc;

        if (chn->cfg.mode == BPCRlib_Mode_eConstantDelay) {
            if(chn->delayed_stc.state==delayed_stc_eWaiting) {
                return b_load_delayed_stc_isr(chn, video);
            } else {
                return BERR_SUCCESS;
            }
        }

        if (video_pts->ePTSType == BAVC_PTSType_eInterpolatedFromInvalidPTS) {
            BDBG_MSG(("Interpolated PTS, ignored"));
            return BERR_SUCCESS;
        }

        switch (BPCRlib_Channel_GetTsmMode_isrsafe(chn))
        {
            case BPCRlib_TsmMode_eOutputMaster:
            case BPCRlib_TsmMode_eAudioMaster:

                return rc;
                break;

            case BPCRlib_TsmMode_eVideoMaster:

                pts_offset = chn->cfg.video_pts_offset;
                if (chn->cfg.video_iface->getCdbLevel && chn->cfg.video_iface->getCdbLevel(chn->cfg.video, &cdb_level)==BERR_SUCCESS && cdb_level > chn->cfg.video_cdb_level) {
                    pts_offset = 0;
                }
                new_stc = video_pts->ui32CurrentPTS - pts_offset;
                BDBG_MSG(("VideoPtsError(%p): Using video PTS %#x[%d] as new STC %#x (old %#x)", (void *)video, (unsigned)video_pts->ui32CurrentPTS, (int)pts_offset, (unsigned)new_stc, (unsigned)video_stc));

                rc = b_send_stc_isr(chn, new_stc, video);
                chn->video_state = BPCRlib_Decoder_State_eLocked;

                break;

            case BPCRlib_TsmMode_eSTCMaster:
            default:

                /* 20070920 bandrews - added to detect DM-unrecoverable video error condition */
                if ((signed)video_pts->ui32CurrentPTS - (signed)video_stc > chn->cfg.video_stc_discard)
                {
                    BDBG_MSG(("Video STC/PTS difference outside of discard threshold"));
                    BDBG_MSG(("Last decoded frame count: %d; Current decoded frame count: %d", chn->sVideo.uiLastDecodedFrameCount, video_pts->uiDecodedFrameCount));
                    if (chn->sVideo.uiLastDecodedFrameCount && (video_pts->uiDecodedFrameCount > chn->sVideo.uiLastDecodedFrameCount))
                    {
                        BDBG_MSG(("Last dropped frame count: %d; Current dropped frame count: %d", chn->sVideo.uiLastDroppedFrameCount, video_pts->uiDroppedFrameCount));
                        if (video_pts->uiDroppedFrameCount > chn->sVideo.uiLastDroppedFrameCount)
                        {
                            unsigned deltaDrop = video_pts->uiDroppedFrameCount - chn->sVideo.uiLastDroppedFrameCount;
                            unsigned deltaDecode = video_pts->uiDecodedFrameCount - chn->sVideo.uiLastDecodedFrameCount;

                            if (deltaDrop == deltaDecode)
                            {
                                /* all frames have been dropped by this channel between errors */
                                BDBG_MSG(("All decoded frames have been dropped since last PTS error; incrementing consecutive error count"));
                                chn->sVideo.uiConsecutivePtsErrorCount += deltaDrop;
                            }
                            else
                            {
                                /* we didn't drop them all, so reset consecutive count */
                                BDBG_MSG(("Some frames have passed TSM since last error; resetting consecutive error count"));
                                chn->sVideo.uiConsecutivePtsErrorCount = 1;
                        }
                    }
                    else
                    {
                            /* decode count incremented, but drop count didn't -> reset consecutive count */
                            BDBG_MSG(("Some frames have passed TSM since last error; resetting consecutive error count"));
                        chn->sVideo.uiConsecutivePtsErrorCount = 1;
                    }
                    }

                    BDBG_MSG(("Consecutive PTS errors detected: %d", chn->sVideo.uiConsecutivePtsErrorCount));

                    /* update the frame counters */
                    chn->sVideo.uiLastDecodedFrameCount = video_pts->uiDecodedFrameCount;
                    chn->sVideo.uiLastDroppedFrameCount = video_pts->uiDroppedFrameCount;

                    if (chn->sVideo.uiConsecutivePtsErrorCount >= chn->cfg.consecutive_pts_error_limit)
                    {
                        BDBG_MSG(("Consecutive PTS error count reached"));
                        /* clear the error counter */
                        chn->sVideo.uiConsecutivePtsErrorCount = 0;
                        /* we have reached a DM-unrecoverable error, reseed STC with video PTS */
                        goto set_stc_from_pts;
                    }
                }

                /* first test if PTS-STC is inside tracking range */
                if (BPCRlib_TestVideoStc_isr(chn, video_stc, video_pts->ui32CurrentPTS)) {
                    int32_t stc_delta;

                    stc_delta=BPCRlib_StcDiff_isrsafe(dss, video_stc, video_pts->ui32CurrentPTS);
                    if (stc_delta>0 && chn->cfg.video_iface->getCdbLevel) {
                        /* STC is ahead of STC, so decoder would drop a frame */
                        rc = chn->cfg.video_iface->getCdbLevel(chn->cfg.video, &cdb_level);
                        if (rc==BERR_SUCCESS && cdb_level<chn->cfg.video_cdb_level) {
                            /* decoder compressed buffer is too shallow, force STC reload to pause decoder and let it accumulate some data */
                            BDBG_MSG(("video decoder is underflowed %d,%u:%u", (int)stc_delta, cdb_level, chn->cfg.video_cdb_level));
                            goto set_stc_from_pts;
                        }
                    }
                    BDBG_MSG(("Video is still in range, ignored"));
                    chn->video_state = BPCRlib_Decoder_State_eLocking;
                    return BERR_SUCCESS;
                }
                else
                {
                    /* likely discontinuity in the stream, however if audio is just happy with STC, let it go */
                    if (BPCRlib_IsAudioLocked_isr(chn, &new_stc))
                    {
                        BDBG_MSG(("Video discontinuity detected, but audio is still locked, ignored"));
                        chn->video_state = BPCRlib_Decoder_State_eNotLocked;
                        return BERR_SUCCESS;
                    }
                    if (BPCRlib_P_GetStcFromPcr_isr(chn, video_pts->ui32CurrentPTS, &new_stc)
                            /* && BPCRlib_TestVideoStc_isr(chn, new_stc, video_pts->ui32CurrentPTS) */
                    )
                    {
                        BDBG_MSG(("VideoPtsError(%p): Using PCR %#x as new STC %#x (old %#x)", (void *)video, (unsigned)chn->pcr_offset.last_pcr, (unsigned)new_stc, (unsigned)video_stc));
                        BPCRlib_P_InvalidatePCRCache_isr(chn);
                        goto set_stc;
                    }
                    goto set_stc_from_pts;
                }
set_stc_from_pts:
                pts_offset = chn->cfg.video_pts_offset;
                if (chn->cfg.video_iface->getCdbLevel && chn->cfg.video_iface->getCdbLevel(chn->cfg.video, &cdb_level)==BERR_SUCCESS && cdb_level > chn->cfg.video_cdb_level)
                {
                    pts_offset = 0;
                }
                new_stc = video_pts->ui32CurrentPTS - pts_offset;
                BDBG_MSG(("VideoPtsError(%p): Using video PTS %#x[%d] as new STC %#x (old %#x)", (void *)video, (unsigned)video_pts->ui32CurrentPTS, (int)pts_offset, (unsigned)new_stc, (unsigned)video_stc));
set_stc:
                rc = b_send_stc_isr(chn, new_stc, video);
                chn->video_state = BPCRlib_Decoder_State_eLocked;

                break;
        }
    } else {
        return b_update_stc_isr(chn, video, false);
    }
    return rc;
}

#define BPCRLIB_MILLISECONDS_TO_PTS_TICKS(CFG, X) (b_is_dss_isrsafe(CFG) ? (X) * 27000 : (X) * 45)

BERR_Code
BPCRlib_Channel_AudioPtsError_isr(BPCRlib_Handle handle, void *audio, const BAVC_PTSInfo *audio_pts, uint32_t audio_stc)
{
    BPCRlib_Channel chn = find_by_audio_isr(handle, audio);
    BERR_Code rc = BERR_SUCCESS;
    unsigned cdb_level;
    int pts_offset;
    bool dss = false;

    BDBG_ASSERT(audio_pts);

    if (!chn) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    dss = b_is_dss_isrsafe(&chn->cfg);
    BDBG_MSG(("(%p) AudioPtsError(%p) PTS %#x(%d) STC %#x %d %s %u", (void *)chn, (void *)audio, (unsigned)audio_pts->ui32CurrentPTS, (int)audio_pts->ePTSType, (unsigned)audio_stc, 2*BPCRlib_StcDiff_isrsafe(dss, audio_stc, audio_pts->ui32CurrentPTS), chn->cfg.playback?"Playback":"", chn->cfg.audio_iface->getCdbLevel?chn->cfg.audio_iface->getCdbLevel(chn->cfg.audio, &cdb_level),cdb_level:0));
    if (chn->cfg.playback) {
        uint32_t new_stc;
        int32_t stc_delta;

        if (chn->cfg.secondary_audio == audio) {
            BDBG_MSG(("secondary audio ignored"));
            return BERR_SUCCESS;
        }

        if (chn->cfg.tertiary_audio == audio) {
            BDBG_MSG(("tertiary audio ignored"));
            return BERR_SUCCESS;
        }

        if (chn->cfg.mode == BPCRlib_Mode_eConstantDelay) {
            if(chn->delayed_stc.state==delayed_stc_eWaiting) {
                return b_load_delayed_stc_isr(chn, audio);
            } else {
                return BERR_SUCCESS;
            }
        }

        if (audio_pts->ePTSType == BAVC_PTSType_eInterpolatedFromInvalidPTS) {
            BDBG_MSG(("Interpolated PTS ignored"));
            return BERR_SUCCESS;
        }

        switch (BPCRlib_Channel_GetTsmMode_isrsafe(chn))
        {
            case BPCRlib_TsmMode_eOutputMaster:
            case BPCRlib_TsmMode_eVideoMaster:

                return rc;
                break;

            case BPCRlib_TsmMode_eAudioMaster:

                pts_offset = chn->cfg.audio_pts_offset;
                stc_delta = (signed)audio_stc - (signed)audio_pts->ui32CurrentPTS;

                if (stc_delta > BPCRLIB_MILLISECONDS_TO_PTS_TICKS(&chn->cfg, (signed)chn->cfg.sync_limit)
                    || stc_delta < - BPCRLIB_MILLISECONDS_TO_PTS_TICKS(&chn->cfg, (signed)chn->cfg.sync_limit)) /* sync limit in PTS ticks*/
                {
                    BDBG_MSG(("Outside of sync limit, requesting audio CDB flush"));
                    new_stc = audio_pts->ui32CurrentPTS - pts_offset;
                    if (chn->cfg.flush)
                    {
                        BKNI_SetEvent_isr(chn->cfg.flush);
                    }
                }
                else
                {
                    new_stc = audio_pts->ui32CurrentPTS;
                }
                BDBG_MSG(("AudioPtsError(%p): Using audio PTS %#x as new STC %#x (old %#x)", (void *)audio, (unsigned)audio_pts->ui32CurrentPTS, (unsigned)new_stc, (unsigned)audio_stc));

                rc = b_send_stc_isr(chn, new_stc, audio);
                chn->audio_state = BPCRlib_Decoder_State_eLocked;
                return BERR_SUCCESS;

                break;

            case BPCRlib_TsmMode_eSTCMaster:
            default:

                stc_delta=BPCRlib_StcDiff_isrsafe(dss, audio_stc, audio_pts->ui32CurrentPTS);
                /* first test if PTS-STC is inside tracking range */
                if ( (stc_delta>0 && stc_delta<chn->cfg.audio_stc_discard) && chn->cfg.audio_iface->getCdbLevel) {
                    /* STC is ahead of STC, so decoder would drop a frame */
                    rc = chn->cfg.audio_iface->getCdbLevel(chn->cfg.audio, &cdb_level);
                    if (rc==BERR_SUCCESS && cdb_level<chn->cfg.audio_cdb_level) {
                        /* decoder compressed buffer is too shallow, force STC reload to pause decoder and let it accumulate some data */
                        BDBG_MSG(("audio decoder is underflowed %d,%u:%u", (int)stc_delta, cdb_level, chn->cfg.audio_cdb_level));
                        goto set_stc_from_pts;
                    }
                }
                if (BPCRlib_TestAudioStc_isr(chn, audio_stc, audio_pts->ui32CurrentPTS)) {
                    goto ignored;
                }
                /* likely discontinuity in the stream, however if vudei is just happy with STC, let it go */
                if (BPCRlib_IsVideoLocked_isr(chn, &new_stc)) {
                    BDBG_MSG(("Audio discontinuity detected, but video is still locked, ignored"));
                    chn->audio_state = BPCRlib_Decoder_State_eNotLocked;
                    return BERR_SUCCESS;
                }
                if (BPCRlib_P_GetStcFromPcr_isr(chn, audio_pts->ui32CurrentPTS, &new_stc)
                        /* && BPCRlib_TestAudioStc_isr(chn, new_stc, audio_pts->ui32CurrentPTS) */
                    ) {
                    BDBG_MSG(("AudioPtsError(%p): Using PCR %#x as new STC %#x (old %#x)", (void *)audio, (unsigned)chn->pcr_offset.last_pcr, (unsigned)new_stc, (unsigned)audio_stc));
                    BPCRlib_P_InvalidatePCRCache_isr(chn);
                    goto set_stc;
                }
                goto set_stc_from_pts;
set_stc_from_pts:
                pts_offset = chn->cfg.audio_pts_offset;
                if (chn->cfg.audio_iface->getCdbLevel && chn->cfg.audio_iface->getCdbLevel(chn->cfg.audio, &cdb_level)==BERR_SUCCESS && cdb_level > chn->cfg.audio_cdb_level) {
                    pts_offset = 0;
                }
                new_stc = audio_pts->ui32CurrentPTS - pts_offset;
                BDBG_MSG(("AudioPtsError(%p): Using audio PTS %#x[%d] as new STC %#x (old %#x)", (void *)audio, (unsigned)audio_pts->ui32CurrentPTS, (int)pts_offset, (unsigned)new_stc, (unsigned)audio_stc));
set_stc:
                rc = b_send_stc_isr(chn, new_stc, audio);
                chn->audio_state = BPCRlib_Decoder_State_eLocked;
                return BERR_SUCCESS;
ignored:
                BDBG_MSG(("Audio is still in range, ignored"));
                chn->audio_state = BPCRlib_Decoder_State_eLocking;
                return BERR_SUCCESS;

                break;
        }
    }
    else
    {
        return b_update_stc_isr(chn, audio, false);
    }
    return rc;

}

static void
BPCRlib_P_InvalidatePCRCache_isr(BPCRlib_Channel chn)
{
    chn->pcr_offset.count = 0;
    chn->pcr_offset.index = 0;
    chn->pcr_offset.last_pcr_valid=false;
    chn->pcr_offset.pcr_offset_valid=false;
    return;
}

bool BPCRlib_P_GetAudioPts_isr(BPCRlib_Channel chn, uint32_t * pts)
{
    bool valid = false;
    BAVC_PTSInfo audio_pts;
    BERR_Code rc = BERR_SUCCESS;

    if (chn->cfg.audio)
    {
        BDBG_ASSERT(chn->cfg.audio_iface->getPts);
        rc = chn->cfg.audio_iface->getPts(chn->cfg.audio, &audio_pts);
        if (rc!=BERR_SUCCESS) goto end; /* 20110516 bandrews - do not trace, failure is normal and handled */

        if (audio_pts.ePTSType == BAVC_PTSType_eInterpolatedFromValidPTS
            || audio_pts.ePTSType == BAVC_PTSType_eCoded)
        {
            BDBG_MSG(("Audio returned valid (%u) PTS of %#x", audio_pts.ePTSType, audio_pts.ui32CurrentPTS));
            *pts = audio_pts.ui32CurrentPTS;
            valid = true;
            goto end;
        }
    }

end:
    return valid;
}

bool BPCRlib_P_GetVideoPts_isr(BPCRlib_Channel chn, uint32_t * pts)
{
    bool valid = false;
    BAVC_PTSInfo video_pts;
    BERR_Code rc = BERR_SUCCESS;

    if (chn->cfg.video)
    {
        BDBG_ASSERT(chn->cfg.video_iface->getPts);
        rc = chn->cfg.video_iface->getPts(chn->cfg.video, &video_pts);
        if (rc!=BERR_SUCCESS) {
            BERR_TRACE(rc);
            goto end;
        }

        if (video_pts.ePTSType == BAVC_PTSType_eInterpolatedFromValidPTS
            || video_pts.ePTSType == BAVC_PTSType_eCoded)
        {
            BDBG_MSG(("Video returned valid (%u) PTS of %#x", video_pts.ePTSType, video_pts.ui32CurrentPTS));
            *pts = video_pts.ui32CurrentPTS;
            valid = true;
            goto end;
        }
    }

end:
    return valid;
}

BERR_Code
BPCRlib_Channel_Invalidate(BPCRlib_Channel chn)
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ASSERT(chn);
    BDBG_MSG(("Invalidate: %p", (void *)chn));
    chn->audio_state = BPCRlib_Decoder_State_eWaitingSTC;
    chn->video_state = BPCRlib_Decoder_State_eWaitingSTC;
    chn->delayed_stc.state = delayed_stc_eInvalid;

    BKNI_EnterCriticalSection();

    BPCRlib_P_InvalidatePCRCache_isr(chn);

    /* handle invalidate call as part of playback resume, try to get lowest
    PTS to seed STC before errors start coming */
    /* 20111117 SW7346-544 bandrews - include playback flag, as some clients are calling this fn in live */
    if (chn->cfg.playback && chn->cfg.refresh_stc_on_invalidate && !chn->cfg.non_real_time)
    {
        uint32_t audio_pts = 0;
        uint32_t video_pts = 0;
        int32_t pts_diff;
        uint32_t new_stc = 0;
        bool audio_valid;
        bool video_valid;

        audio_valid = BPCRlib_P_GetAudioPts_isr(chn, &audio_pts);
        video_valid = BPCRlib_P_GetVideoPts_isr(chn, &video_pts);

        if (audio_valid && video_valid)
        {
            pts_diff = (int32_t)(video_pts - audio_pts);
            if (pts_diff < 0)
            {
                new_stc = video_pts - chn->cfg.video_pts_offset;
            }
            else
            {
                new_stc = audio_pts - chn->cfg.audio_pts_offset;
            }

            BDBG_MSG(("Invalidate: using lowest PTS %#x as new STC", new_stc));
        }
        else if (audio_valid)
        {
            new_stc = audio_pts - chn->cfg.audio_pts_offset;
            BDBG_MSG(("Invalidate: [video invalid] using audio PTS %#x as new STC", new_stc));
        }
        else if (video_valid)
        {
            new_stc = video_pts - chn->cfg.video_pts_offset;
            BDBG_MSG(("Invalidate: [audio invalid] using video PTS %#x as new STC", new_stc));
        }
        else
        {
            BDBG_MSG(("Invalidate: [video, audio invalid] no change in STC"));
            goto end;
        }

        chn->video_state = BPCRlib_Decoder_State_eLocked;
        chn->audio_state = BPCRlib_Decoder_State_eLocked;
        rc = b_really_send_stc_isr(chn, new_stc);
    }

end:
    BKNI_LeaveCriticalSection();
    return rc;
}

BERR_Code
BPCRlib_Channel_GetStc(BPCRlib_Channel chn, uint32_t *stc)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t stc_low;

    BDBG_ASSERT(chn);

    BKNI_EnterCriticalSection();
    /* Systems with PCROFFSET require the aux_transport setting and do not use the pcr setting. */
    *stc = BXPT_PcrOffset_GetStc_isr(chn->cfg.aux_transport) + BXPT_PcrOffset_GetOffset_isr(chn->cfg.aux_transport);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto done;
    }
    if (!chn->cfg.playback || chn->cfg.mode!=BPCRlib_Mode_eConstantDelay) {
        goto done;
    }
    if (chn->delayed_stc.state == delayed_stc_eInvalid) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto done;
    }
    stc_low = chn->delayed_stc.stc + (*stc - chn->delayed_stc.old_stc);
    BDBG_MSG_TRACE(("BPCRlib_Channel_GetStc: %#lx returning STC %#x (%u+%d)", (unsigned long)chn, (unsigned)stc_low, (unsigned)chn->delayed_stc.stc, (int)(*stc - chn->delayed_stc.old_stc)));
    *stc = stc_low;
done:
    BKNI_LeaveCriticalSection();
    return rc;
}


BERR_Code
BPCRlib_Channel_PcrUpdate(BPCRlib_Channel chn, uint32_t pcr)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t stc=0;
    bool stc_valid;
    int32_t delta=0;
    bool send_video_stc=false;
    bool send_audio_stc=false;

    if (!chn || !chn->cfg.playback) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (chn->cfg.mode==BPCRlib_Mode_eConstantDelay) {
        BKNI_EnterCriticalSection();

        switch(chn->delayed_stc.state) {
        case delayed_stc_eLocked:
        case delayed_stc_eWaiting:
#if 0
            rc = BXPT_PCR_GetStc_isr(chn->pcr, &chn->delayed_stc.old_stc, &chn->delayed_stc.stc /* not used */ );
            chn->delayed_stc.stc = pcr;
            BDBG_MSG(("BPCRlib_Channel_PcrUpdate: %#lx saving STC %u (%d)", (unsigned long)chn, (unsigned)pcr, (int)(pcr - chn->delayed_stc.old_stc)));
            chn->delayed_stc.state = delayed_stc_eWaiting; /* don't load STC wait for decoder to ask */
            break;
#endif
        case delayed_stc_eInvalid:
            BDBG_WRN(("BPCRlib_Channel_PcrUpdate: %#lx loading new STC %#x", (unsigned long)chn, (unsigned)pcr));
            rc = b_really_send_stc_isr(chn, pcr);
            chn->delayed_stc.stc = pcr;
            chn->delayed_stc.old_stc = pcr;
            chn->delayed_stc.state = delayed_stc_eLocked;
            break;
        }
        /* update delayed_stc, so following  call to b_load_delayed_stc_isr would not use stale values */
        BKNI_LeaveCriticalSection();
        return rc;
    }


    BKNI_EnterCriticalSection();
    if (chn->cfg.video && chn->video_state==BPCRlib_Decoder_State_eWaitingSTC) {
        send_video_stc = true;
    }
    if (chn->cfg.audio && chn->audio_state==BPCRlib_Decoder_State_eWaitingSTC) {
        send_audio_stc = true;
    }
    if (send_audio_stc && send_video_stc) {
        BDBG_WRN(("%p preloading stc %#x from pcr %#x", (void *)chn, pcr-chn->cfg.pcr_offset, pcr));
        b_send_stc_isr(chn, pcr-chn->cfg.pcr_offset, chn->cfg.video);
        b_send_stc_isr(chn, pcr-chn->cfg.pcr_offset, chn->cfg.audio);
        if (send_audio_stc) {
            chn->audio_state=BPCRlib_Decoder_State_eNotLocked;
        }
        if (send_video_stc) {
            chn->video_state=BPCRlib_Decoder_State_eNotLocked;
        }
    }
    chn->pcr_offset.last_pcr_valid=true;
    chn->pcr_offset.last_pcr=pcr;
    stc_valid = b_read_stc_isr(chn, &stc);
#if 0
    BDBG_MSG(("(%#x) PcrUpdate %u %u %d[%u:%u]", (unsigned)chn, (unsigned)pcr, stc, delta, chn->pcr_offset.index, chn->pcr_offset.count));
#endif
    if (stc_valid) {
        delta = BPCRlib_StcDiff_isrsafe(b_is_dss_isrsafe(&chn->cfg), pcr, stc);
        if (delta <= chn->cfg.pcr_discard/2 && delta >= -chn->cfg.pcr_discard/2) {
            chn->pcr_offset.pcr_fifo[chn->pcr_offset.index] = delta;
            chn->pcr_offset.index++;
            if (chn->pcr_offset.index >= BPCRlib_P_PCR_FIFO) {
                chn->pcr_offset.index = 0;
            }
            if (chn->pcr_offset.count < BPCRlib_P_PCR_FIFO) {
                chn->pcr_offset.count++;
            }
            chn->pcr_offset.pcr_offset_valid = false;
        } else {
            chn->pcr_offset.pcr_offset = pcr-stc;
            chn->pcr_offset.pcr_offset_valid = true;
            BDBG_MSG(("(%p) PcrOffset %#x %#x (%d/%d/%d)", (void *)chn, (unsigned)pcr, stc, (int)delta*2, (int)chn->pcr_offset.pcr_offset,(int)chn->cfg.pcr_discard));
        }
    }
    BKNI_LeaveCriticalSection();

    return rc;
}
