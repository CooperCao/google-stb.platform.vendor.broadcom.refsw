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

#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "ts_priv.h"
#include "ts_packet.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmpeg2ts_psi_probe.h"

BDBG_MODULE(b_playback_ip_psi);

#define BMPEG2TS_PSI_PAT_PID        0x00
#define BMPEG2TS_SYNC_BYTE          0x47
#define BMPEG2TS_PKT_SIZE           188
#define BMPEG2TS_INITIAL_SYNC_COUNT 5
#define STREAM_INFO_SIZE            1024
typedef enum
{
    PBIP_TsPktState_eIdle,
    PBIP_TsPktState_eNew,
    PBIP_TsPktState_eWaitingForInitialSyncByte,
    PBIP_TsPktState_eWaitingForFullPkt,
    PBIP_TsPktState_eDone
} PBIP_TsPktState;

typedef enum
{
    PBIP_PsiState_eIdle,
    PBIP_PsiState_eNew,
    PBIP_PsiState_eWaitingForPat,
    PBIP_PsiState_eProcessPat,          /* transitional state */
    PBIP_PsiState_eWaitingForPmt,
    PBIP_PsiState_eProcessPmt,          /* transitional state */
    PBIP_PsiState_eWaitingForFirstPtsPassed,
    PBIP_PsiState_eWaitingForAvPipePlayout,
    PBIP_PsiState_eWaitingToResumePsiParsing,
    PBIP_PsiState_eResumePsiParsing,    /* transitional state */
    PBIP_PsiState_eDone
} PBIP_PsiState;

typedef struct B_PlaybackIpPsiState
{
    unsigned        cachedTsPktLength;
    uint8_t         cachedTsPkt[BMPEG2TS_PKT_SIZE];
    uint8_t         patPmtBuf[BMPEG2TS_PKT_SIZE*2];
    uint8_t         *pTsPkt;
    unsigned        curPmtPid;
    PBIP_TsPktState tsPktState;
    PBIP_PsiState   psiState;
    off_t             syncByteCount;
    int             consecutiveTsSyncByteCount;
    bool            abort;

    const bmedia_probe_stream *stream;
    char *          pStreamInfo;
    const bmedia_probe_stream *pInitialStream;
    bmedia_probe_t  probe;
    unsigned        pcrPid;
    unsigned        videoPid;
    unsigned        audioPid1;
    unsigned        audioPid2;

    unsigned        totalStreamChanges;
    off_t           totalBytesConsumed;
    off_t           tsPktCnt;
    off_t           tsPatPktCnt;
    off_t           tsNewPmtPktCnt;
    off_t           tsPktWithNoPayload;
    off_t           tsPktWithError;
    off_t           totalInitialTsPktsSkipped;
    off_t           totalProbeErrs;
    off_t           totalPmtValidationErrs;
    off_t           totalProgramParsingErrs;

} B_PlaybackIpPsiState;

struct bfile_io_read_patPmtBuf
{
    struct bfile_io_read self;
    uint8_t *pBuffer;
    off_t offset;
};

static int B_PlaybackIp_PatPmtBufBounds(
    bfile_io_read_t self,
    off_t *first,
    off_t *last
    )
{
    struct bfile_io_read_patPmtBuf *patPmtBfileIo = (struct bfile_io_read_patPmtBuf *) self;
    *first = 0;
    *last = BMPEG2TS_PKT_SIZE*2;
    BDBG_MSG(("%s: patPmtBfileIo=%p bounds returned first=%"PRId64 " last=%"PRId64 , __FUNCTION__, (void *)patPmtBfileIo, *first, *last));
    return 0;
}

static off_t B_PlaybackIp_PatPmtBufSeek(
    bfile_io_read_t self,
    off_t offset,
    int whence
    )
{
    struct bfile_io_read_patPmtBuf *patPmtBfileIo = (struct bfile_io_read_patPmtBuf *) self;
    BSTD_UNUSED(whence);
    off_t first, last;

    first = 0;
    last = 2*BMPEG2TS_PKT_SIZE;

    BDBG_MSG(("%s: patPmtBfileIo=%p offsets: first=%"PRId64 " last=%"PRId64 " cur=%"PRId64 " asked=%"PRId64 " whence=%s", __FUNCTION__,
                (void *)patPmtBfileIo, first, last, patPmtBfileIo->offset, offset, whence==SEEK_SET?"SEEK_SET":whence==SEEK_CUR?"SEEK_CUR":"SEEK_END" ));
    if (whence == SEEK_CUR) {
        offset = patPmtBfileIo->offset + offset;
    }
    else if (whence == SEEK_END) {
        offset = last + offset;
    }

    if (offset < first) {
        offset = first;
    }
    else if (offset > last) {
        offset = last;
    }
    patPmtBfileIo->offset = offset;

    BDBG_MSG(("%s: patPmtBfileIo=%p updated offset=%"PRId64 , __FUNCTION__, (void *)patPmtBfileIo, offset));
    return offset;
}

static ssize_t B_PlaybackIp_PatPmtBufRead(
    bfile_io_read_t self,
    void *buf,
    size_t length
    )
{
    struct bfile_io_read_patPmtBuf *patPmtBfileIo = (struct bfile_io_read_patPmtBuf *) self;
    ssize_t bytesToRead = 0;

    BDBG_MSG(("%s: patPmtBfileIo=%p buf=%p length=%zu", __FUNCTION__, (void *)patPmtBfileIo, (void *)buf, length));

    if (patPmtBfileIo->offset >= 2 * BMPEG2TS_PKT_SIZE)
    {
        BDBG_MSG(("%s: forcing EOF to complate probe faster %zu, offset %"PRId64 , __FUNCTION__, length, patPmtBfileIo->offset));
        return 0;
    }
    if (length > 2 * BMPEG2TS_PKT_SIZE)
    {
        length = 2 * BMPEG2TS_PKT_SIZE; /* forcing the read length to only 1st two MPEG2 TS packets */
        BDBG_MSG(("%s: trimming index read request to complate probe faster, length=%zu offset %"PRId64 , __FUNCTION__, length, patPmtBfileIo->offset));
    }

    if (patPmtBfileIo->offset+length >= BMPEG2TS_PKT_SIZE*2)
        bytesToRead = BMPEG2TS_PKT_SIZE*2 - (size_t)patPmtBfileIo->offset;
    else
        bytesToRead = length;

    BKNI_Memcpy(buf, patPmtBfileIo->pBuffer+patPmtBfileIo->offset, bytesToRead);
    BDBG_MSG(("%s: returning %zd bytes at offst %"PRId64 , __FUNCTION__, bytesToRead, patPmtBfileIo->offset));
    return bytesToRead;
}

static const struct bfile_io_read patPmtBuf_read =
{
    B_PlaybackIp_PatPmtBufRead,
    B_PlaybackIp_PatPmtBufSeek,
    B_PlaybackIp_PatPmtBufBounds,
    BIO_DEFAULT_PRIORITY
};

static const bmedia_probe_stream *
B_PlaybackIp_PatPmtProbe(
    B_PlaybackIpPsiStateHandle pPsi,
    uint8_t *pBuffer
    )
{
    struct bfile_io_read_patPmtBuf  patPmtBfileIo;
    const bmedia_probe_stream *     stream = NULL;
    bmedia_probe_config             probe_config;

    /* Create probe object if it is not already created. */
    if (!pPsi->probe)
    {
        pPsi->probe = bmedia_probe_create();
        if (!pPsi->probe)
        {
            BDBG_ERR(("%s: failed to create the probe object", __FUNCTION__));
            return (NULL);
        }
    }

    bmedia_probe_default_cfg(&probe_config);
    /* Configure probe for a fast MPEG2 TS PSI probe. */
    probe_config.type = bstream_mpeg_type_ts;
    probe_config.file_name = "xxx.ts";
    probe_config.probe_payload = false;
    probe_config.probe_index = false;
    probe_config.parse_index = false;
    probe_config.probe_es = false;
    probe_config.probe_all_formats = false;
    probe_config.probe_duration = false;

    patPmtBfileIo.pBuffer = pBuffer;
    patPmtBfileIo.self = patPmtBuf_read;
    patPmtBfileIo.offset = 0;

    stream = bmedia_probe_parse(pPsi->probe, (bfile_io_read_t)&patPmtBfileIo.self, &probe_config);
    if (!stream)
    {
        /* probe didn't find the PSI info either, return error */
        BDBG_ERR(("%s: media probe didn't find the PSI info, return error", __FUNCTION__));
    }

    return (stream);
}

void B_PlaybackIp_ResumePsiParsing(
    B_PlaybackIpPsiStateHandle pPsi
    )
{
    if (!pPsi) return;
    pPsi->psiState = PBIP_PsiState_eResumePsiParsing;
    BDBG_MSG(("%s: Resumed Psi parsing for pPsi=%p", __FUNCTION__, (void *)pPsi));
}

void B_PlaybackIp_ResetPsiState(
    B_PlaybackIpPsiStateHandle pPsi
    )
{
    if (!pPsi) return;
    pPsi->tsPktState = PBIP_TsPktState_eIdle;
    pPsi->psiState = PBIP_PsiState_eIdle;
    pPsi->abort = true;
    BDBG_MSG(("%s: Re-setting PSI state for Psi=%p", __FUNCTION__, (void *)pPsi));
}

void B_PlaybackIp_GetPsiStreamState(
    B_PlaybackIpPsiStateHandle pPsi,
    const void **ppStream
    )
{
    if (!pPsi) return;
    *ppStream = pPsi->stream;
    BDBG_MSG(("%s: Returned bmedia stream=%p for pPsi=%p", __FUNCTION__, (void *)*ppStream, (void *)pPsi));
}

void B_PlaybackIp_DestroyPsiState(
    B_PlaybackIpPsiStateHandle pPsi
    )
{
    BDBG_ASSERT(pPsi);
    if (!pPsi) return;
    BDBG_WRN(("%s:%p: tsPktCnt=%"PRId64 " totalPat=%"PRId64 ", newPmt=%"PRId64 " totalStreamChanges=%u tsPktWithNoPayload=%"PRId64 " tsPktWithError=%"PRId64 " totalBytesConsumed=%"PRId64 " totalInitialTsPktsSkipped=%"PRId64 " totalProbeErrs=%"PRId64 " totalPmtValidationErrs=%"PRId64 " totalProgramParsingErrs=%"PRId64 ,
                __FUNCTION__, (void *)pPsi,
                pPsi->tsPktCnt,
                pPsi->tsPatPktCnt,
                pPsi->tsNewPmtPktCnt,
                pPsi->totalStreamChanges,
                pPsi->tsPktWithNoPayload,
                pPsi->tsPktWithError,
                pPsi->totalBytesConsumed,
                pPsi->totalInitialTsPktsSkipped,
                pPsi->totalProbeErrs,
                pPsi->totalPmtValidationErrs,
                pPsi->totalProgramParsingErrs
             ));
    /* Free-up probe related objects & reset their pointers. */
    if (pPsi->pStreamInfo) {
        B_Os_Free(pPsi->pStreamInfo);
        pPsi->pStreamInfo = NULL;
    }
    if (pPsi->stream) {
        bmedia_probe_stream_free(pPsi->probe, pPsi->stream);
        pPsi->stream = NULL;
    }
    if (pPsi->probe) {
        bmedia_probe_destroy(pPsi->probe);
        pPsi->probe = NULL;
    }

    B_Os_Free(pPsi);
}

B_PlaybackIpPsiStateHandle B_PlaybackIp_CreatePsiState(B_PlaybackIpHandle playback_ip)
{
    B_PlaybackIpPsiStateHandle hPsi;

    hPsi = B_Os_Calloc( 1, sizeof(B_PlaybackIpPsiState));
    if (hPsi == NULL)
    {
        BDBG_ERR(("%s: B_Os_Calloc failed for %zu bytes of B_PlaybackIpPsiState structure!", __FUNCTION__, sizeof(B_PlaybackIpPsiState) ));
        return NULL;
    }

    if ((hPsi->pStreamInfo = B_Os_Calloc(1, STREAM_INFO_SIZE+1)) == NULL)
    {
        BDBG_ERR(("%s: B_Os_Calloc failed for %d bytes of streamInfo string!", __FUNCTION__, STREAM_INFO_SIZE+1));
        B_Os_Free(hPsi);
        return NULL;
    }
    /* Now cache the initial AV PID values. */
    hPsi->pInitialStream = playback_ip->stream;
    hPsi->pcrPid = playback_ip->psi.pcrPid;
    hPsi->videoPid = playback_ip->psi.videoPid;
    hPsi->audioPid1 = playback_ip->psi.audioPid;
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s:%p: Created: initial AV PID info: pPsi=%p pcr=%u video=%u audio=%u", __FUNCTION__, (void *)playback_ip, (void *)hPsi, hPsi->pcrPid, hPsi->videoPid, hPsi->audioPid1));
#endif
#if defined(LOG_IP_LATENCY)
    B_PlaybackIp_UtilsTrkLatencyInit(playback_ip);
#endif
    return (hPsi);
}


static const bmedia_probe_track *getTrackOfType(
    const bmedia_probe_stream   *pStream,
    bmedia_track_type           trackType,
    int                         trackIndex
    )
{
    const bmedia_probe_track    *pTrack = NULL;
    int                         currentIndex;

    for (   pTrack=BLST_SQ_FIRST(&pStream->tracks), currentIndex=1;
            pTrack;
            pTrack=BLST_SQ_NEXT(pTrack, link)
        )
    {
        if (pTrack->type == trackType)
        {
            if (currentIndex++ == trackIndex) break;
        }
    }
    return (pTrack);
}

static bool hasStreamInfoChanged(
    B_PlaybackIpPsiState *pPsi,
    const bmedia_probe_stream *pNewStream
    )
{
    const bmedia_probe_stream   *pCurStream;
    const bmedia_probe_track    *pCurTrack = NULL;
    const bmedia_probe_track    *pNewTrack = NULL;

    /*
     * Compare the current stream w/ the new one & see if new one has any changes.
     */
    pCurStream = pPsi->stream ? pPsi->stream : pPsi->pInitialStream;

    /* Check for changes in stream level attributes first. */
    if (!pNewStream->ntracks)
    {
        BDBG_MSG(("%s: streamHasChanged=false: New stream doesn't have any valid # of tracks!", __FUNCTION__));
        return (false);
    }
    if (pCurStream->ntracks != pNewStream->ntracks)
    {
        BDBG_WRN(("%s: streamHasChanged=true ntracks cur=%u new=%u", __FUNCTION__, pCurStream->ntracks, pNewStream->ntracks));
        return (true);
    }
    if (pCurStream->type != pNewStream->type)
    {
        BDBG_WRN(("%s: streamHasChanged=true type cur=%u new=%u", __FUNCTION__, pCurStream->type, pNewStream->type));
        return (true);
    }

    /* Then look for track level attributes. */

    /* See if Video track is different in two streams. */
    pCurTrack = getTrackOfType(pCurStream, bmedia_track_type_video, 1/*trackIndex*/); /* 1st video track. */
    pNewTrack = getTrackOfType(pNewStream, bmedia_track_type_video, 1/*trackIndex*/);
    if (pCurTrack && pNewTrack)
    {
        /* Both streams have video tracks, compare their PIDs & Codecs. */
        if (pCurTrack->number != pNewTrack->number)
        {
            BDBG_WRN(("%s: streamHasChanged=true: VIDEO PID cur=0x%x new=0x%x", __FUNCTION__, pCurTrack->number, pNewTrack->number));
            return (true);
        }
        else if (pCurTrack->info.video.codec != pNewTrack->info.video.codec)
        {
            BDBG_WRN(("%s: streamHasChanged=true: VIDEO Codec cur=%u new=%u", __FUNCTION__, pCurTrack->info.video.codec, pNewTrack->info.video.codec));
            return (true);
        }
        else
        {
            BDBG_MSG(("%s: streamHasChanged=false: Video Tracks are same, continue analyzing other stream attributes!", __FUNCTION__));
        }
    }
    else if (pCurTrack || pNewTrack)
    {
        /* Only one stream has video track, so we consider this as a change & return. */
        BDBG_WRN(("%s: streamHasChanged=true: %s", __FUNCTION__, pCurTrack? "current stream has Video Track & new one doesn't":"current stream doesn't have Video Track & new one does!"));
        return (true);
    }
    else
    {
        BDBG_MSG(("%s: streamHasChanged=false: Neither current or new stream has Video Track!", __FUNCTION__));
    }

    /* See if PCR track is different in two streams. */
    pCurTrack = getTrackOfType(pCurStream, bmedia_track_type_pcr, 1/*trackIndex*/);
    pNewTrack = getTrackOfType(pNewStream, bmedia_track_type_pcr, 1/*trackIndex*/);
    if (pCurTrack && pNewTrack)
    {
        /* Both streams have PCR tracks, compare their PIDs. */
        if (pCurTrack->number != pNewTrack->number)
        {
            BDBG_WRN(("%s: streamHasChanged=true: PCR PID cur=0x%x new=0x%x", __FUNCTION__, pCurTrack->number, pNewTrack->number));
            return (true);
        }
        else
        {
            BDBG_MSG(("%s: streamHasChanged=false: PCR Tracks are same, continue analyzing other stream attributes!", __FUNCTION__));
        }
    }
    else
    {
        /* PCR Tracks are missing in either cur or new streams. Continue looking for other differences! */
    }

    /* If we are here, there is no change in Video & PCR Tracks. So lets look for changes in Audio Tracks. */
    pCurTrack = getTrackOfType(pCurStream, bmedia_track_type_audio, 1/*trackIndex*/); /* 1st audio track. */
    pNewTrack = getTrackOfType(pNewStream, bmedia_track_type_audio, 1/*trackIndex*/);
    if (pCurTrack && pNewTrack)
    {
        /* Both streams have audio tracks, compare their PIDs & Codecs. */
        if (pCurTrack->number != pNewTrack->number)
        {
            BDBG_WRN(("%s: streamHasChanged=true: AUDIO PID cur=0x%x new=0x%x", __FUNCTION__, pCurTrack->number, pNewTrack->number));
            return (true);
        }
        else if (pCurTrack->info.audio.codec != pNewTrack->info.audio.codec)
        {
            BDBG_WRN(("%s: streamHasChanged=true: AUDIO Codec cur=%u new=%u", __FUNCTION__, pCurTrack->info.audio.codec, pNewTrack->info.audio.codec));
            return (true);
        }
        else
        {
            BDBG_MSG(("%s: streamHasChanged=false: 1st Audio Tracks are same, continue analyzing other stream attributes!", __FUNCTION__));
        }
    }
    else if (pCurTrack || pNewTrack)
    {
        /* Only one stream has audio track, so we consider this as a change & return. */
        BDBG_WRN(("%s: streamHasChanged=true: %s", __FUNCTION__, pCurTrack? "current stream has Audio Track & new one doesn't":"current stream doesn't have Audio Track & new one does!"));
        return (true);
    }
    else
    {
        BDBG_MSG(("%s: streamHasChanged=false: Neither current or new stream has Audio Track!", __FUNCTION__));
        /* We are done. */
        return (false);
    }

    /* TODO: may need to consider 2nd audio track. */

    /* If we are here, we haven't found any differences in two streams. So return. */
    return (false);
} /* hasStreamInfoChanged */

extern B_PlaybackIpError updateNexusPlaypumpDecodersState( B_PlaybackIpHandle playback_ip, B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
B_PlaybackIpError B_PlaybackIp_ParseAndProcessPsiState(
    B_PlaybackIpHandle playback_ip,
    uint8_t *pAvBuffer,
    unsigned avBufferLength
    )
{
    B_PlaybackIpError       rc = B_ERROR_PROTO;
    B_PlaybackIpPsiState    *pPsi = playback_ip->pPsiState;
    bool                    tsTimestampsEnabled = playback_ip->psi.transportTimeStampEnabled ? true : false;
    TS_packet               tsPkt;
    TS_PAT_program          program;
    TS_PSI_header           sectionHeader;
    unsigned                pointerField = 0;
    const uint8_t           *pSectionBuf = NULL;
    unsigned                numPrograms;
    unsigned                sectionBufLength;
    unsigned                bytesSkipped = 0;
    unsigned                bytesConsumed = 0;
    bool                    unlocked = false;
    const bmedia_probe_stream *pNewStream;

    if (playback_ip->psi.mpegType != NEXUS_TransportType_eTs)
    {
        BDBG_MSG(("%s: Runtime PSI Parsing is not supported for non-TS container formats=%d", __FUNCTION__, playback_ip->psi.mpegType));
        return (B_ERROR_SUCCESS);
    }
    if (!playback_ip->stream)
    {
        BDBG_MSG(("%s: Runtime PSI Parsing is not supported for if initial stream is not known!", __FUNCTION__));
        return (B_ERROR_SUCCESS);
    }

    if (!pAvBuffer || !avBufferLength)
    {
        return (B_ERROR_SUCCESS);
    }

    if (pPsi->psiState == PBIP_PsiState_eIdle)
    {
        pPsi->psiState = PBIP_PsiState_eNew;
        pPsi->tsPktState = PBIP_TsPktState_eNew;
        pPsi->syncByteCount = 0;
        pPsi->abort = false;
        if (!pPsi->curPmtPid )
        {
            pPsi->curPmtPid = playback_ip->psi.pmtPid;
            BDBG_WRN(("playback_ip=%p: Using initial PMT PID=0x%x", (void *)playback_ip, pPsi->curPmtPid));
        }
    }

    while (bytesConsumed < avBufferLength)
    {
        if (unlocked)
        {
            BKNI_AcquireMutex(playback_ip->lock);
            unlocked = false;
        }
        if (breakFromLoopDueToChChg(playback_ip) || pPsi->abort)
        {
            BDBG_MSG(("playback_ip=%p state=%d: breaking out of while loop due to %s", (void *)playback_ip, playback_ip->playback_state, pPsi->abort?"psiStateAbort":"Channel Change"));
            rc = B_ERROR_CHANNEL_CHANGE;
            break;
        }
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode )
        {
            BKNI_ReleaseMutex(playback_ip->lock);
            unlocked = true;
            BDBG_MSG(("playback_ip=%p: wait to come out of %s", (void *)playback_ip, playback_ip->playback_state == B_PlaybackIpState_ePaused ? "Paused state" : "Trickmode API completion"));
            BKNI_Sleep(20);
            continue;
        }
        if (pPsi->psiState == PBIP_PsiState_eNew)
        {
            /* Wait for next PAT */
            pPsi->psiState = PBIP_PsiState_eWaitingForPat;
        }
        if (pPsi->tsPktState == PBIP_TsPktState_eNew)
        {
            if (pPsi->cachedTsPktLength >0)
            {
                pPsi->cachedTsPktLength = 0;
                BKNI_Memset(pPsi->cachedTsPkt, 0, BMPEG2TS_PKT_SIZE);
            }
            /* Wait for start of TS packet: indicated by a sync byte (== 0x47) */
            pPsi->tsPktState = PBIP_TsPktState_eWaitingForInitialSyncByte;
            BKNI_Memset(&tsPkt, 0, sizeof(tsPkt));
        }

        /* Wait until we have a full PAT or PMT TS packet. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForPat || pPsi->psiState == PBIP_PsiState_eWaitingForPmt)
        {
            if (pPsi->tsPktState == PBIP_TsPktState_eWaitingForInitialSyncByte)
            {
                /* Keep consuming until TS Sync byte is found. */
                if (pAvBuffer[bytesConsumed] != BMPEG2TS_SYNC_BYTE)
                {
                    bytesConsumed++;
                    bytesSkipped++;
                    if (pPsi->syncByteCount < BMPEG2TS_INITIAL_SYNC_COUNT) {
                        BDBG_MSG(("Resetting initialSyncByte check logic due to missing sync: syncByteCount=%"PRId64 " BMPEG2TS_INITIAL_SYNC_COUNT=%d total totalInitialTsPktsSkipped=%"PRId64 " ",
                                    pPsi->syncByteCount, BMPEG2TS_INITIAL_SYNC_COUNT, pPsi->totalInitialTsPktsSkipped));
                        pPsi->syncByteCount = 0;
                    }
                    continue;
                }
                else
                {
                    /* Sync byte is found, so now look for Full TS packet. */
                    pPsi->syncByteCount++;
                    pPsi->tsPktState = PBIP_TsPktState_eWaitingForFullPkt;
                    if (bytesSkipped)
                    {
                        BDBG_MSG(("bytes skipped=%u while looking for TS Sync, bytesConsumed=%u avBufferLength=%u", bytesSkipped, bytesConsumed, avBufferLength));
                        bytesSkipped = 0;
                    }
                }
            }

            /* Check if we have a full TS packet. */
            if (pPsi->tsPktState == PBIP_TsPktState_eWaitingForFullPkt)
            {
                if (pPsi->cachedTsPktLength + (avBufferLength-bytesConsumed) < BMPEG2TS_PKT_SIZE)
                {
                    /* cached pkt & remaining bytes length is still than a TS PKT, meaning we dont yet have a full TS packet, so cache these bytes. */
                    BDBG_MSG(("Keep caching a TS packet as cachedTsPktLength=%u & remainingBytes =%u are still < BMPEG2TS_PKT_SIZE", pPsi->cachedTsPktLength, (avBufferLength-bytesConsumed)));
                    BKNI_Memcpy(&pPsi->cachedTsPkt[pPsi->cachedTsPktLength], pAvBuffer+bytesConsumed, avBufferLength-bytesConsumed);
                    pPsi->cachedTsPktLength += avBufferLength-bytesConsumed;
                    bytesConsumed = avBufferLength;
                    /* consumed all bytes from the current buffer, so continue to finish the loop. */
                    continue;
                }
                else if (pPsi->cachedTsPktLength)
                {
                    /* current cached pkt & avBufferLength combined contain atleast 1 full TS pkt, combine the bytes to create the full pkt. */
                    BDBG_MSG(("cachedTsPktLength=%u & avBufferLength=%u contain atleast 1 BMPEG2TS_PKT_SIZE", pPsi->cachedTsPktLength, avBufferLength));
                    BKNI_Memcpy(&pPsi->cachedTsPkt[pPsi->cachedTsPktLength], pAvBuffer, BMPEG2TS_PKT_SIZE-pPsi->cachedTsPktLength);
                    bytesConsumed = BMPEG2TS_PKT_SIZE-pPsi->cachedTsPktLength;
                    if (tsTimestampsEnabled) bytesConsumed += 4;
                    pPsi->pTsPkt = pPsi->cachedTsPkt;
                    pPsi->tsPktState = PBIP_TsPktState_eDone;
                    pPsi->tsPktCnt++;
                }
                else
                {
                    /* nothing is previously cached & avBufferLength contains atleast 1 full TS pkt. */
                    BDBG_MSG_FLOW(("Nothing is previosly cached, avBufferLength=%u bytesConsumed=%d", avBufferLength, bytesConsumed));
                    pPsi->pTsPkt = pAvBuffer+bytesConsumed;
                    bytesConsumed += BMPEG2TS_PKT_SIZE;
                    if (tsTimestampsEnabled) bytesConsumed += 4;
                    pPsi->tsPktState = PBIP_TsPktState_eDone;
                    pPsi->tsPktCnt++;
                }
            }

            /* Check if we have a full TS packet, then determine if it is PAT & if so start its processing. */
            if (pPsi->tsPktState == PBIP_TsPktState_eDone)
            {
                /* Check if we have received enough TS Sync packets. */
                if (pPsi->syncByteCount < BMPEG2TS_INITIAL_SYNC_COUNT)
                {
                    pPsi->totalInitialTsPktsSkipped++;
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    BDBG_MSG(("Skipping TS Pkt: syncByteCount=%"PRId64 " BMPEG2TS_INITIAL_SYNC_COUNT=%d totalInitialTsPktsSkipped=%"PRId64 " ",
                                pPsi->syncByteCount, BMPEG2TS_INITIAL_SYNC_COUNT, pPsi->totalInitialTsPktsSkipped));
                    continue;
                }
                else
                {
                    BDBG_MSG_FLOW(("Correctly detected start of TS SyncByte in the AV stream: syncByteCount=%"PRId64 " BMPEG2TS_INITIAL_SYNC_COUNT=%d totalInitialTsPktsSkipped=%"PRId64 " ",
                                pPsi->syncByteCount, BMPEG2TS_INITIAL_SYNC_COUNT, pPsi->totalInitialTsPktsSkipped));
                }
                TS_parseTsPacket(pPsi->pTsPkt, &tsPkt);
                if (tsPkt.data_size == 0)
                {
                    pPsi->tsPktWithNoPayload++;
                    /* TS packet doesn't contain the payload, skip it & keep looking for PAT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    BDBG_MSG(("TS packet doesn't contain the payload, skipping it: avBufferLength=%u bytesConsumed=%d, tsPkt#=%"PRId64 , avBufferLength, bytesConsumed, pPsi->tsPktCnt));
                    continue;
                }
                if (tsPkt.transport_error_indicator)
                {
                    pPsi->tsPktWithError++;
                    /* TS packet doesn't contain the payload, skip it & keep looking for PAT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    BDBG_MSG(("TS packet has error indicator set, skipping it: avBufferLength=%u bytesConsumed=%d, tsPkt#=%"PRId64 , avBufferLength, bytesConsumed, pPsi->tsPktCnt));
                    continue;
                }
#if defined(LOG_IP_LATENCY)
                if (tsPkt.adaptation_field.PCR_flag || tsPkt.adaptation_field.discontinuity_indicator)
                {
                    B_PlaybackIp_UtilsTrkPcrJitter(playback_ip, &tsPkt);
                }
                if (tsPkt.payload_unit_start_indicator && tsPkt.adaptation_field_control)
                {
                    B_PlaybackIp_UtilsTrkLatencyStreamToDecodePts(playback_ip, &tsPkt);
                }
#endif
                if (tsPkt.payload_unit_start_indicator)
                {
                    pointerField = tsPkt.p_data_byte[0]; /* pointerField essentially provides offset to the PAT or PMT packet. */
                }
                else
                {
                    pointerField = 0;
                }
                if ( (pPsi->psiState == PBIP_PsiState_eWaitingForPat || pPsi->psiState == PBIP_PsiState_eWaitingForPmt) && tsPkt.PID == BMPEG2TS_PSI_PAT_PID)
                {
                    /* PAT packet, advance PSI state such that it can process PAT below. */
                    pPsi->tsPatPktCnt++;
                    pPsi->psiState = PBIP_PsiState_eProcessPat;
                }
                else if (pPsi->psiState == PBIP_PsiState_eWaitingForPmt && tsPkt.PID == pPsi->curPmtPid)
                {
                    /* New PMT packet, advance PSI state such that it can process PMT below. */
                    /* Note: curPmtPid was updated in the ProcessPat state below when PAT contained a new PMT pid. */
                    pPsi->psiState = PBIP_PsiState_eProcessPmt;
                }
                else
                {
                    /* It is not a PAT/PMT packet, keep looking for PAT/PMT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    BDBG_MSG_FLOW(("TS packet is not a PAT/PMT packet, skipping it: avBufferLength=%u bytesConsumed=%d, tsPkt#=%"PRId64 " pid=%u, psiState=%d curPmtPid=%u", avBufferLength, bytesConsumed, pPsi->tsPktCnt, tsPkt.PID, pPsi->psiState, pPsi->curPmtPid ));
                    continue;
                }
            }
        }

        /* If we have the PAT Pkt, then process it. */
        if (pPsi->psiState == PBIP_PsiState_eProcessPat)
        {
            const uint8_t *pPat;

            pSectionBuf = &tsPkt.p_data_byte[pointerField] + 1; /* +1 for the pointer field itself. */
            /* coverity[var_deref_model: FALSE] */
            TS_PSI_getSectionHeader(pSectionBuf, &sectionHeader);
            sectionBufLength = tsPkt.data_size - pointerField -1; /* -1 for the pointer field */
            pPat = pSectionBuf;
            numPrograms = TS_PAT_getNumPrograms(pPat);
            if (TS_PAT_getProgram(pPat, tsPkt.data_size - pointerField -1, 0, &program)!=BERR_SUCCESS)
            {
                BDBG_WRN(("%p: TS_PAT_getProgram() Failed to parse the program section in a PAT", (void *)playback_ip));
                pPsi->totalProgramParsingErrs++;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPat;
                continue;
            }
            /* PAT contains a valid PMT. */
            if (program.PID != pPsi->curPmtPid)
            {
                /* PID of new program (1st one in the PAT) is now different than the currently decoded program, so find info about this Program. */
                BDBG_WRN(("%p: AV Stream has updated PAT: pid=0x0 version=%d tsPkt#=%"PRId64 " numPrograms=%d program_number=%d, program_pid=0x%x sectionBufLength=%d psiState=%d",
                        (void *)playback_ip, sectionHeader.version_number, pPsi->tsPktCnt, numPrograms, program.program_number, program.PID, sectionBufLength, pPsi->psiState));
                pPsi->curPmtPid = program.PID;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPmt;
                BKNI_Memcpy(pPsi->patPmtBuf, pPsi->pTsPkt, BMPEG2TS_PKT_SIZE);
                pPsi->tsNewPmtPktCnt++;
                continue;
            }
            else
            {
                /* PAT contains the same program as program-PID hasn't changed. */
                /* NOTE: Initially, we used to skip this PMT (as it is same as before) & continue to looking for next PAT. */
                /* However, some Gateway server's tend to update PMT w/o updating its version number or PMT PID. */
                /* So for robust player implementation, we now evaluate all PMTs for any AV stream changes below. */

                BDBG_MSG(("%p: AV Stream has same PAT: pid=0x0 tsPkt#=%"PRId64 " numPrograms=%d program_number=%d, program_pid=0x%x sectionBufLength=%d psiState=%d",
                        (void *)playback_ip, pPsi->tsPktCnt, numPrograms, program.program_number, program.PID, sectionBufLength, pPsi->psiState));
                pPsi->curPmtPid = program.PID;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPmt;
                BKNI_Memcpy(pPsi->patPmtBuf, pPsi->pTsPkt, BMPEG2TS_PKT_SIZE);
                continue;
            }
        }

        /* If we have the PMT Pkt, then process it. */
        if (pPsi->psiState == PBIP_PsiState_eProcessPmt)
        {
            BKNI_Memcpy(pPsi->patPmtBuf+BMPEG2TS_PKT_SIZE, pPsi->pTsPkt, BMPEG2TS_PKT_SIZE);
            pSectionBuf = &tsPkt.p_data_byte[pointerField] + 1; /* +1 for the pointer field itself. */
            /* coverity[var_deref_model: FALSE] */
            TS_PSI_getSectionHeader(pSectionBuf, &sectionHeader);
            sectionBufLength = tsPkt.data_size - pointerField -1; /* -1 for the pointer field */
            if (TS_PMT_validate(pSectionBuf, sectionBufLength) != true)
            {
                BDBG_WRN(("%p: TS_PMT_validate() Failed to validate the PMT", (void *)playback_ip));
                pPsi->totalPmtValidationErrs++;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPmt;
                continue;
            }

            /* Run BMedia Probe on patPmt and let it build & update the stream pointer for this new PMT. */
            if ( (pNewStream = B_PlaybackIp_PatPmtProbe(pPsi, pPsi->patPmtBuf)) == NULL )
            {
                BDBG_WRN(("%p: B_PlaybackIp_PatPmtProbe() Failed on next PAT/PMT", (void *)playback_ip));
                pPsi->totalProbeErrs++;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPat;
                continue;
            }

            /* Now compare this new stream object to the current object & determine if there is any change in the AV stream characterstics. */
            if (hasStreamInfoChanged(pPsi, pNewStream) == true)
            {
                pPsi->totalStreamChanges++;
                BKNI_Memset(pPsi->pStreamInfo, 0, STREAM_INFO_SIZE+1);
                bmedia_stream_to_string(pNewStream, pPsi->pStreamInfo, STREAM_INFO_SIZE);
                BDBG_WRN(("%p: AV Stream has updated PMT: pid=0x%x tsPkt#=%"PRId64 " version=%d, totalStreamChanges=%u psiState=%d",
                            (void *)playback_ip, program.PID, pPsi->tsPktCnt, sectionHeader.version_number, pPsi->totalStreamChanges, pPsi->psiState));
                BDBG_WRN(("media stream info: %s", pPsi->pStreamInfo));
                /* Free-up the current stream object & update it w/ the new one. */
                if (pPsi->stream)
                {
                    bmedia_probe_stream_free(pPsi->probe, pPsi->stream);
                }
                pPsi->stream = pNewStream;
            }
            else
            {
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPat;
                BDBG_MSG(("%p: AV Stream has same PMT: tsPkt#=%"PRId64 " psiState=%d", (void *)playback_ip, pPsi->tsPktCnt, pPsi->psiState));
                bmedia_probe_stream_free(pPsi->probe, pNewStream);
                continue;
            }


            /* Now change state to playing out currently buffered program data. */
#if 1
#if 0
            /* Commenting this code out as it can wait forever for TSM if data fed before this point didn't have an i-frame. */
            /* This can happen if we tune to a channel when it is at the tail end of the GOP of the current program. */
            if (0&&!playback_ip->firstPtsPassed)
            {
                BDBG_WRN(("%p: Wait until we get firstPtsPassed!", (void *)playback_ip));
                pPsi->psiState = PBIP_PsiState_eWaitingForFirstPtsPassed;
            }
            else
#endif
            {
                BDBG_WRN(("%p: Wait until currently buffered AV is completedly played out!", (void *)playback_ip));
                pPsi->psiState = PBIP_PsiState_eWaitingForAvPipePlayout;
            }
#else
            pPsi->tsPktState = PBIP_TsPktState_eNew;
            pPsi->psiState = PBIP_PsiState_eWaitingForPat;
            BDBG_WRN(("%p: Skipped setting up new Program PIDs!", (void *)playback_ip));
#endif
        }

        /* Before we can notify BIP about the new program, we will need to ensure that all AV Stream corresponding to current program PID has been decoded & displayed. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForFirstPtsPassed)
        {
            /* Note: this function also sets the firstPtsPassed flag from the AV decoder status. */
            if (!playback_ip->firstPtsPassed)
            {
                BDBG_MSG(("%p: Still waiting for firstPtsPassed", (void *)playback_ip));
                BKNI_Sleep(100);
                continue;
            }
            else
            {
                /* Done, Sleep a bit to playout 1st couple of frames. */
                BKNI_Sleep(50);
                BDBG_WRN(("%p: firstPtsPassed is true, now wait until currently buffered AV is completedly played out!", (void *)playback_ip));
                pPsi->psiState = PBIP_PsiState_eWaitingForAvPipePlayout;
            }
        }

        /* Before we can notify BIP about the new program, we will need to ensure that all AV Stream corresponding to current program PID has been decoded & displayed. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForAvPipePlayout)
        {
            if (B_PlaybackIp_UtilsEndOfStream(playback_ip) == false)
            {
                BDBG_MSG(("%p: Still waiting to playout all AV frames!", (void *)playback_ip));
                BKNI_Sleep(100);
                continue;
            }
            else
            {
                /* Done, Sleep a frametime to playout any last frame. */
                BKNI_Sleep(20);
                BDBG_WRN(("%p: Currently buffered AV stream is played out!", (void *)playback_ip));

                /* Now everything pertaining to the current program has been completely played out. */
                /* Notify App (BIP) about the new program and then wait for App to take an action and notify us to resume PSI parsing. */
                if (playback_ip->playback_state != B_PlaybackIpState_eStopping)
                {
                    pPsi->psiState = PBIP_PsiState_eWaitingToResumePsiParsing;
                    BKNI_ReleaseMutex(playback_ip->lock);
                    unlocked = true;
                    if (playback_ip->openSettings.eventCallback)
                    {
                        BDBG_WRN(("%p: issue newProgram callback to app!", (void *)playback_ip));
                        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eNewProgram);
                        BDBG_WRN(("%p: issued newProgram callback to app!", (void *)playback_ip));
                    }
                    /* We have released the lock before invoking the callback, so need to goto the top to re-acquire the lock. */
                    continue;
                }
                else
                {
                    /* We are in stopping state, goto top of the loop so that we can break out from this loop. */
                    continue;
                }
            }
        }

        if (pPsi->psiState == PBIP_PsiState_eWaitingToResumePsiParsing)
        {
            BDBG_MSG(("%p: Waiting to ResumePsiParsing!", (void *)playback_ip));
            BKNI_Sleep(20);
            continue;
        }

        /* AV Decoder have been re-started, so we can change pids to the new program! */
        if (pPsi->psiState == PBIP_PsiState_eResumePsiParsing)
        {
            BDBG_WRN(("%p: App finished selecting AV tracks from new program & has indicated to resume PSI Parsing!", (void *)playback_ip));

            /* Now reset states to look for next updated PAT TS packet */
            pPsi->tsPktState = PBIP_TsPktState_eNew;
            pPsi->psiState = PBIP_PsiState_eWaitingForPat;

            if (playback_ip->playback_state == B_PlaybackIpState_eTrickMode) {
                if ((rc = updateNexusPlaypumpDecodersState(playback_ip, &playback_ip->ipTrickModeSettings)) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state after new program PSI setup!", __FUNCTION__));
                    continue;
                }
                BDBG_WRN(("%p: Updated Decoder trickmode state!", (void *)playback_ip));
            }
        }
    } /* while */

    pPsi->totalBytesConsumed += bytesConsumed;
    rc = B_ERROR_SUCCESS;

    if (rc != B_ERROR_SUCCESS)
    {
        B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
    }
    BDBG_MSG(("Psi=%p: tsPktCnt=%"PRId64 " totalPat=%"PRId64 ", newPmt=%"PRId64 " totalStreamChanges=%u tsPktWithNoPayload=%"PRId64 " tsPktWithError=%"PRId64 " totalBytesConsumed=%"PRId64 " avBufferLength=%u syncByteCount=%"PRId64 " totalInitialTsPktsSkipped=%"PRId64 ,
                (void *)pPsi,
                pPsi->tsPktCnt,
                pPsi->tsPatPktCnt,
                pPsi->tsNewPmtPktCnt,
                pPsi->totalStreamChanges,
                pPsi->tsPktWithNoPayload,
                pPsi->tsPktWithError,
                pPsi->totalBytesConsumed,
                avBufferLength,
                pPsi->syncByteCount,
                pPsi->totalInitialTsPktsSkipped
             ));
    return (rc);
}

/* APIs added for UNIT Test purposes so that unit test can directly call the Psi related APIs above. */
void B_PlaybackIp_DestroyPsiStateForUnitTest(B_PlaybackIpHandle hPlaybackIp)
{
    if (hPlaybackIp->pPsiState) B_PlaybackIp_DestroyPsiState(hPlaybackIp->pPsiState);
}

/* Custom build a PlaybackIp State structure so that we can then directly call its PSI Parsing APIs. */
B_PlaybackIpError B_PlaybackIp_CreatePsiStateForUnitTest(B_PlaybackIpHandle hPlaybackIp, unsigned currentProgramPid, NEXUS_PlaypumpHandle playpump, NEXUS_PidChannelHandle videoPidChannel, NEXUS_PidChannelHandle audioPidChannel, bool transportTimeStampEnabled)
{
    BDBG_ASSERT(hPlaybackIp);
    hPlaybackIp->playback_state = B_PlaybackIpState_ePlaying;
    hPlaybackIp->psi.pmtPid = currentProgramPid;
    hPlaybackIp->nexusHandles.playpump = playpump;
    hPlaybackIp->psi.transportTimeStampEnabled = transportTimeStampEnabled;
    hPlaybackIp->nexusHandles.videoPidChannel = videoPidChannel;
    hPlaybackIp->nexusHandles.audioPidChannel = audioPidChannel;
    hPlaybackIp->pPsiState = B_PlaybackIp_CreatePsiState(hPlaybackIp);
    BDBG_ASSERT(hPlaybackIp->pPsiState);
    return (B_ERROR_SUCCESS);
}
#endif /* LINUX || VxWorks */
