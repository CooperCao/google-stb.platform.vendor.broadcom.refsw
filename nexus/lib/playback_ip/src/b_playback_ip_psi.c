/***************************************************************************
*     (c)2003-2016 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* Description: This file contains logic to detect PAT/PMT changes in a buffer
* containing clear AV stream.
*
***************************************************************************/

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
    PBIP_PsiState_eWaitingForNewPmt,
    PBIP_PsiState_eProcessPmt,          /* transitional state */
    PBIP_PsiState_eWaitingForPlaypumpFifoPlayout,
    PBIP_PsiState_eWaitingForAvPipePlayout,
    PBIP_PsiState_eChangePids,          /* transitional state */
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
    int             syncByteCount;
    int             consecutiveTsSyncByteCount;
    bool            abort;

    const bmedia_probe_stream *stream;
    bmedia_probe_t  probe;

    off_t           totalBytesConsumed;
    off_t           tsPktCnt;
    off_t           tsPatPktCnt;
    off_t           tsNewPmtPktCnt;
    off_t           tsPktWithNoPayload;
    off_t           tsPktWithError;
    off_t           totalInitialTsPktsSkipped;

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
    BDBG_MSG(("%s: patPmtBfileIo=%p bounds returned first=%lld last=%lld", __FUNCTION__, patPmtBfileIo, *first, *last));
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

    BDBG_MSG(("%s: patPmtBfileIo=%p offsets: first=%lld last=%lld cur=%lld asked=%lld whence=%s", __FUNCTION__,
                patPmtBfileIo, first, last, patPmtBfileIo->offset, offset, whence==SEEK_SET?"SEEK_SET":whence==SEEK_CUR?"SEEK_CUR":"SEEK_END" ));
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

    BDBG_MSG(("%s: patPmtBfileIo=%p updated offset=%lld", __FUNCTION__, patPmtBfileIo, offset));
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

    BDBG_MSG(("%s: patPmtBfileIo=%p buf=%p length=%u", __FUNCTION__, patPmtBfileIo, buf, length));

    if (patPmtBfileIo->offset >= 2 * BMPEG2TS_PKT_SIZE)
    {
        BDBG_MSG(("%s: forcing EOF to complate probe faster for HLS %d, offset %lld", __FUNCTION__, length, patPmtBfileIo->offset));
        return 0;
    }
    if (length > 2 * BMPEG2TS_PKT_SIZE)
    {
        length = 2 * BMPEG2TS_PKT_SIZE; /* forcing the read length to only 1st two MPEG2 TS packets */
        BDBG_MSG(("%s: trimming index read request to complate probe faster for HLS, offset %lld", __FUNCTION__, length, patPmtBfileIo->offset));
    }

    if (patPmtBfileIo->offset+length >= BMPEG2TS_PKT_SIZE*2)
        bytesToRead = BMPEG2TS_PKT_SIZE*2 - (size_t)patPmtBfileIo->offset;
    else
        bytesToRead = length;

    BKNI_Memcpy(buf, patPmtBfileIo->pBuffer+patPmtBfileIo->offset, bytesToRead);
    BDBG_MSG(("%s: returning %d bytes at offst %lld", __FUNCTION__, bytesToRead, patPmtBfileIo->offset));
    return bytesToRead;
}

static const struct bfile_io_read patPmtBuf_read =
{
    B_PlaybackIp_PatPmtBufRead,
    B_PlaybackIp_PatPmtBufSeek,
    B_PlaybackIp_PatPmtBufBounds,
    BIO_DEFAULT_PRIORITY
};

static int
B_PlaybackIp_PatPmtProbe(
    B_PlaybackIpPsiStateHandle pPsi,
    uint8_t *pBuffer
    )
{
    int                             rc = -1;
    char *                          stream_info = NULL;
    struct bfile_io_read_patPmtBuf  patPmtBfileIo;
    const bmedia_probe_stream *     stream = NULL;
    bmedia_probe_config             probe_config;

    if ((stream_info = BKNI_Malloc(STREAM_INFO_SIZE+1)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure for stream_info structure", __FUNCTION__));
        return -1;
    }
    BKNI_Memset(stream_info, 0, STREAM_INFO_SIZE+1);

    /* Create probe object if it is not already created. */
    if (!pPsi->probe)
    {
        pPsi->probe = bmedia_probe_create();
        if (!pPsi->probe)
        {
            BDBG_ERR(("%s: failed to create the probe object", __FUNCTION__));
            goto error;
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
    if (stream)
    {
        /* probe succeeded in finding a stream */
        bmedia_stream_to_string(stream, stream_info, STREAM_INFO_SIZE);
        BDBG_WRN(("media stream info: %s", stream_info));
        /* Free-up the current stream object & update it w/ the latest one. */
        if (pPsi->stream) {
            bmedia_probe_stream_free(pPsi->probe, pPsi->stream);
        }
        pPsi->stream = stream;
        rc = 0;
    }
    else
    {
        /* probe didn't find the PSI info either, return error */
        BDBG_ERR(("%s: media probe didn't find the PSI info, return error", __FUNCTION__));
        rc = -1;
    }
    BDBG_MSG(("%s: done", __FUNCTION__));

error:
    if (stream_info) { BKNI_Free( stream_info); }
    BDBG_MSG(("%s: Done, rc %d", __FUNCTION__, rc));

    return (rc);
}

void B_PlaybackIp_ResetPsiState(
    B_PlaybackIpPsiStateHandle pPsi
    )
{
    if (!pPsi) return;
    pPsi->tsPktState = PBIP_TsPktState_eIdle;
    pPsi->psiState = PBIP_PsiState_eIdle;
    pPsi->abort = true;
    BDBG_MSG(("%s: Re-setting PSI state for Psi=%p", __FUNCTION__, pPsi));
}

void B_PlaybackIp_DestroyPsiState(
    B_PlaybackIpPsiStateHandle pPsi
    )
{
    BDBG_ASSERT(pPsi);
    if (!pPsi) return;
    BDBG_WRN(("%s: Freeing up Psi=%p: stats: tsPktCnt=%lld totalPat=%lld, newPmt=%lld tsPktWithNoPayload=%lld tsPktWithError=%lld totalBytesConsumed=%lld totalInitialTsPktsSkipped=%lld",
                __FUNCTION__, pPsi,
                pPsi->tsPktCnt,
                pPsi->tsPatPktCnt,
                pPsi->tsNewPmtPktCnt,
                pPsi->tsPktWithNoPayload,
                pPsi->tsPktWithError,
                pPsi->totalBytesConsumed,
                pPsi->totalInitialTsPktsSkipped
             ));
    /* Free-up probe related objects & reset their pointers. */
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

B_PlaybackIpPsiStateHandle B_PlaybackIp_CreatePsiState(void)
{
    BDBG_MSG(("%s: allocating %d bytes for PsiState", __FUNCTION__, sizeof(B_PlaybackIpPsiState) ));
    return ( B_Os_Calloc( 1, sizeof(B_PlaybackIpPsiState)) );
}

static void stopVideoDecoder(
    B_PlaybackIpNexusHandles *pNexusHandles
    )
{
    if (pNexusHandles->videoDecoder)
    {
        NEXUS_VideoDecoder_Stop(pNexusHandles->videoDecoder);
        BDBG_MSG(("pNexusHandles=%p: non-simple VideoDecoder=%p is stopped", pNexusHandles, pNexusHandles->videoDecoder ));
    }
    else if (pNexusHandles->simpleVideoDecoder)
    {
        NEXUS_SimpleVideoDecoder_Stop(pNexusHandles->simpleVideoDecoder);
        BDBG_MSG(("pNexusHandles=%p: simple VideoDecoder=%p is stopped", pNexusHandles, pNexusHandles->simpleVideoDecoder ));
    }
} /* stopVideoDecoder */

static void stopAudioDecoder(
    B_PlaybackIpNexusHandles *pNexusHandles
    )
{
    if (pNexusHandles->primaryAudioDecoder)
    {
        NEXUS_AudioDecoder_Stop(pNexusHandles->primaryAudioDecoder);
        BDBG_MSG(("pNexusHandles=%p: non-simple AudioDecoder=%p is stopped", pNexusHandles, pNexusHandles->primaryAudioDecoder ));
    }
    else if (pNexusHandles->simpleAudioDecoder)
    {
        NEXUS_SimpleAudioDecoder_Stop(pNexusHandles->simpleAudioDecoder);
        BDBG_MSG(("pNexusHandles=%p: simple AudioDecoder=%p is stopped", pNexusHandles, pNexusHandles->simpleAudioDecoder ));
    }
} /* stopAudioDecoder */

static B_PlaybackIpError startVideoDecoder(
    B_PlaybackIpNexusHandles *pNexusHandles
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    if (pNexusHandles->videoDecoder)
    {
        nrc = NEXUS_VideoDecoder_Start( pNexusHandles->videoDecoder, &pNexusHandles->videoStartSettings );
        if (nrc!=NEXUS_SUCCESS) { BDBG_ERR(("NEXUS_VideoDecoder_Start() Failed during Video Track Change")); goto error; }
        BDBG_MSG(("pNexusHandles=%p: non-simple VideoDecoder=%p is re-started", pNexusHandles, pNexusHandles->videoDecoder ));
    }
    else if (pNexusHandles->simpleVideoDecoder)
    {
        nrc = NEXUS_SimpleVideoDecoder_Start( pNexusHandles->simpleVideoDecoder, &pNexusHandles->simpleVideoStartSettings );
        if (nrc!=NEXUS_SUCCESS) { BDBG_ERR(("NEXUS_SimpleVideoDecoder_Start() Failed during Video Track Change")); goto error; }
        BDBG_MSG(("pNexusHandles=%p: simple VideoDecoder=%p is re-started", pNexusHandles, pNexusHandles->simpleVideoDecoder ));
    }

error:
    if (nrc == NEXUS_SUCCESS)
        return (B_ERROR_SUCCESS);
    else
        return (B_ERROR_PROTO);

} /* startVideoDecoder */

static B_PlaybackIpError startAudioDecoder(
    B_PlaybackIpNexusHandles *pNexusHandles
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    if (pNexusHandles->primaryAudioDecoder)
    {
        nrc = NEXUS_AudioDecoder_Start( pNexusHandles->primaryAudioDecoder, &pNexusHandles->audioStartSettings );
        if (nrc!=NEXUS_SUCCESS) { BDBG_ERR(("NEXUS_AudioDecoder_Start() Failed during Audio Track Change")); goto error; }
        BDBG_MSG(("pNexusHandles=%p: non-simple AudioDecoder=%p is re-started", pNexusHandles, pNexusHandles->primaryAudioDecoder ));
    }
    else if (pNexusHandles->simpleAudioDecoder)
    {
        nrc = NEXUS_SimpleAudioDecoder_Start( pNexusHandles->simpleAudioDecoder, &pNexusHandles->simpleAudioStartSettings );
        if (nrc!=NEXUS_SUCCESS) { BDBG_ERR(("NEXUS_SimpleAudioDecoder_Start() Failed during Audio Track Change")); goto error; }
        BDBG_MSG(("pNexusHandles=%p: simple AudioDecoder=%p is re-started", pNexusHandles, pNexusHandles->simpleAudioDecoder ));
    }

error:
    if (nrc == NEXUS_SUCCESS)
        return (B_ERROR_SUCCESS);
    else
        return (B_ERROR_PROTO);
} /* startAudioDecoder */

NEXUS_StcChannelAutoModeBehavior nexusStcChannelModeBehavior(
    B_PlaybackIpNexusHandles *pNexusHandles
    )
{
    NEXUS_StcChannelAutoModeBehavior modeBehavior = NEXUS_StcChannelAutoModeBehavior_eMax;

    if (pNexusHandles->stcChannel) {
        NEXUS_StcChannelSettings settings;
        NEXUS_StcChannel_GetSettings(pNexusHandles->stcChannel, &settings);
        if (settings.mode == NEXUS_StcChannelMode_eAuto) {
            modeBehavior = settings.modeSettings.Auto.behavior;
        }
    }
    else if (pNexusHandles->simpleStcChannel) {
        NEXUS_SimpleStcChannelSettings settings;
        NEXUS_SimpleStcChannel_GetSettings( pNexusHandles->simpleStcChannel, &settings );
        if (settings.mode == NEXUS_StcChannelMode_eAuto) {
            modeBehavior = settings.modeSettings.Auto.behavior;
        }
    }
    BDBG_MSG(("modeBehavior=%d", modeBehavior));
    return (modeBehavior);
}

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
    unsigned                lastPlayedFrameCount = 0;
    unsigned                pcrPid=UINT_MAX, videoPid=UINT_MAX, audioPid1=UINT_MAX, audioPid2=UINT_MAX;

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
            BDBG_WRN(("playback_ip=%p: Using initial PMT PID=0x%x", playback_ip, pPsi->curPmtPid));
        }
    }
    BKNI_Memset(&tsPkt, 0, sizeof(tsPkt));

    while (bytesConsumed < avBufferLength)
    {
        if (unlocked)
        {
            BKNI_AcquireMutex(playback_ip->lock);
            unlocked = false;
        }
        if (breakFromLoopDueToChChg(playback_ip) || pPsi->abort)
        {
            BDBG_MSG(("playback_ip=%p state=%d: breaking out of while loop due to %s", playback_ip, playback_ip->playback_state, pPsi->abort?"psiStateAbort":"Channel Change"));
            rc = B_ERROR_CHANNEL_CHANGE;
            break;
        }
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_ePaused)
        {
            BKNI_ReleaseMutex(playback_ip->lock);
            unlocked = true;
            BDBG_MSG(("playback_ip=%p: wait to come out of %s", playback_ip, playback_ip->playback_state == B_PlaybackIpState_ePaused ? "Paused state" : "Trickmode API completion"));
            BKNI_Sleep(100);
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
        }

        /* Wait until we have a full PAT or PMT TS packet. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForPat || pPsi->psiState == PBIP_PsiState_eWaitingForNewPmt)
        {
            if (pPsi->tsPktState == PBIP_TsPktState_eWaitingForInitialSyncByte)
            {
                /* Keep consuming until TS Sync byte is found. */
                if (pAvBuffer[bytesConsumed] != BMPEG2TS_SYNC_BYTE)
                {
                    bytesConsumed++;
                    bytesSkipped++;
                    if (pPsi->syncByteCount < BMPEG2TS_INITIAL_SYNC_COUNT) {
                        BDBG_MSG(("Resetting initialSyncByte check logic due to missing sync: syncByteCount=%d BMPEG2TS_INITIAL_SYNC_COUNT=%d total totalInitialTsPktsSkipped=%lld ",
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
                }
                else
                {
                    /* nothing is previously cached & avBufferLength contains atleast 1 full TS pkt. */
                    BDBG_MSG_FLOW(("Nothing is previosly cached, avBufferLength=%u bytesConsumed=%d", avBufferLength, bytesConsumed));
                    pPsi->pTsPkt = pAvBuffer+bytesConsumed;
                    bytesConsumed += BMPEG2TS_PKT_SIZE;
                    if (tsTimestampsEnabled) bytesConsumed += 4;
                    pPsi->tsPktState = PBIP_TsPktState_eDone;
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
                    BDBG_MSG(("Skipping TS Pkt: syncByteCount=%d BMPEG2TS_INITIAL_SYNC_COUNT=%d totalInitialTsPktsSkipped=%lld ",
                                pPsi->syncByteCount, BMPEG2TS_INITIAL_SYNC_COUNT, pPsi->totalInitialTsPktsSkipped));
                    continue;
                }
                else
                {
                    BDBG_MSG_FLOW(("Correctly detected start of TS SyncByte in the AV stream: syncByteCount=%d BMPEG2TS_INITIAL_SYNC_COUNT=%d totalInitialTsPktsSkipped=%lld ",
                                pPsi->syncByteCount, pPsi->totalInitialTsPktsSkipped, BMPEG2TS_INITIAL_SYNC_COUNT));
                }
                TS_parseTsPacket(pPsi->pTsPkt, &tsPkt);
                if (tsPkt.data_size == 0)
                {
                    pPsi->tsPktWithNoPayload++;
                    /* TS packet doesn't contain the payload, skip it & keep looking for PAT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    continue;
                }
                if (tsPkt.transport_error_indicator)
                {
                    pPsi->tsPktWithError++;
                    /* TS packet doesn't contain the payload, skip it & keep looking for PAT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
                    continue;
                }
                if (tsPkt.payload_unit_start_indicator)
                {
                    pointerField = tsPkt.p_data_byte[0]; /* pointerField essentially provides offset to the PAT or PMT packet. */
                }
                else
                {
                    pointerField = 0;
                }
                pPsi->tsPktCnt++;
                if (pPsi->psiState == PBIP_PsiState_eWaitingForPat && tsPkt.PID == BMPEG2TS_PSI_PAT_PID)
                {
                    /* PAT packet, advance PSI state such that it can process PAT below. */
                    pPsi->tsPatPktCnt++;
                    pPsi->psiState = PBIP_PsiState_eProcessPat;
                }
                else if (pPsi->psiState == PBIP_PsiState_eWaitingForNewPmt && tsPkt.PID == pPsi->curPmtPid)
                {
                    /* New PMT packet, advance PSI state such that it can process PMT below. */
                    /* Note: curPmtPid was updated in the ProcessPat state below when PAT contained a new PMT pid. */
                    pPsi->tsNewPmtPktCnt++;
                    pPsi->psiState = PBIP_PsiState_eProcessPmt;
                }
                else
                {
                    /* It is not a PAT packet, keep looking for PAT packet. */
                    pPsi->tsPktState = PBIP_TsPktState_eNew;
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
                BDBG_WRN(("%p: TS_PAT_getProgram() Failed to parse the program section in a PAT", playback_ip));
                break;
            }
            if (program.PID != pPsi->curPmtPid)
            {
                /* PID of new program (1st one in the PAT) is now different than the currently decoded program, so find info about this Program. */
                BDBG_WRN(("%p: AV Stream has updated PAT: pid=0x0 tsPkt#=%lld numPrograms=%d program_number=%d, program_pid=0x%x sectionBufLength=%d",
                        playback_ip, pPsi->tsPktCnt, numPrograms, program.program_number, program.PID, sectionBufLength));
                pPsi->curPmtPid = program.PID;
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForNewPmt;
                BKNI_Memcpy(pPsi->patPmtBuf, pPsi->pTsPkt, BMPEG2TS_PKT_SIZE);
                continue;
            }
            else
            {
                /* PAT contains the same program as program PID hasn't changed, continue to looking for next PAT. */
                pPsi->tsPktState = PBIP_TsPktState_eNew;
                pPsi->psiState = PBIP_PsiState_eWaitingForPat;
                continue;
            }
        }

        /* If we have the PMT Pkt, then process it. */
        if (pPsi->psiState == PBIP_PsiState_eProcessPmt)
        {
            unsigned numStreams;
            TS_PMT_stream pmtStream;
            unsigned i;

            BKNI_Memcpy(pPsi->patPmtBuf+BMPEG2TS_PKT_SIZE, pPsi->pTsPkt, BMPEG2TS_PKT_SIZE);
            pSectionBuf = &tsPkt.p_data_byte[pointerField] + 1; /* +1 for the pointer field itself. */
            /* coverity[var_deref_model: FALSE] */
            TS_PSI_getSectionHeader(pSectionBuf, &sectionHeader);
            sectionBufLength = tsPkt.data_size - pointerField -1; /* -1 for the pointer field */
            if (TS_PMT_validate(pSectionBuf, sectionBufLength) != true)
            {
                BDBG_WRN(("%p: TS_PMT_validate() Failed to validate the PMT", playback_ip));
                break;
            }
            pcrPid = TS_PMT_getPcrPid(pSectionBuf, sectionBufLength);
            for ( numStreams = TS_PMT_getNumStreams(pSectionBuf, sectionBufLength), i = 0; i < numStreams; i++ )
            {
                if (TS_PMT_getStream(pSectionBuf, sectionBufLength, i, &pmtStream) != BERR_SUCCESS)
                {
                    BDBG_WRN(("%p: TS_PMT_getStream() Failed to get the stream section info in a PMT", playback_ip));
                    break;
                }
                TS_PMT_getStream(pSectionBuf, sectionBufLength, i, &pmtStream);
                if (pmtStream.stream_type == 0x81 && audioPid1 == UINT_MAX)
                    audioPid1 = pmtStream.elementary_PID;
                else if (pmtStream.stream_type == 0x81 && audioPid2 == UINT_MAX)
                    audioPid2 = pmtStream.elementary_PID;
                else if (pmtStream.stream_type == 0x2 && videoPid == UINT_MAX)
                    videoPid = pmtStream.elementary_PID;
            }
            BDBG_WRN(("%p: AV Stream has updated PMT: pid=0x%x tsPkt#=%lld pcr_pid=0x%x videoPid=0x%x audioPid=0x%x audioPid=0x%x",
                        playback_ip, program.PID, pPsi->tsPktCnt, pcrPid, videoPid, audioPid1, audioPid2 ));

            /* Run BMedia Probe on patPmt and let it build & update the stream pointer for this new PMT. */
            B_PlaybackIp_PatPmtProbe(pPsi, pPsi->patPmtBuf);

            /* Now change state to playing out currently buffered program data from the playpump fifo before we can change to the new program pids. */
            pPsi->psiState = PBIP_PsiState_eWaitingForPlaypumpFifoPlayout;
        }

        /* Before we can change to the new AV PIDs, we will need to ensure that all TS pkts corresponding to current program PID has been consumed by the XPT. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForPlaypumpFifoPlayout)
        {
            NEXUS_Error nrc;
            NEXUS_PlaypumpStatus status;

            nrc = NEXUS_Playpump_GetStatus(playback_ip->nexusHandles.playpump, &status);
            if (nrc != NEXUS_SUCCESS)
            {
                BDBG_WRN(("%p: NEXUS_Playpump_GetStatus() failed, nrc=%d", playback_ip, nrc));
                break;
            }
            if (status.fifoDepth > 8192)
            {
                BDBG_MSG(("Still %d bytes in the FIFO, sleeping...", status.fifoDepth));
                BKNI_Sleep(10);
                continue;
            }
            else
            {
                /* Sleep a bit for letting XPT consume any pkts still in its h/w pipe. */
                BKNI_Sleep(30);
                BDBG_WRN(("%p: Playpump FIFO is played out: depth=%d", playback_ip, status.fifoDepth));
                /* Now see if we need to drain the AV decoder pipe as well. */
                if (nexusStcChannelModeBehavior(&playback_ip->nexusHandles) == NEXUS_StcChannelAutoModeBehavior_eAudioMaster)
                {
                    pPsi->psiState = PBIP_PsiState_eChangePids;
                    BDBG_MSG(("%p: Playpump FIFO is played out & we are audio master mode, so no need to wait for AV Pipe Playout as video is usually behind audio in live cases", playback_ip));
                }
                else
                {
                    BDBG_MSG(("%p: Playpump FIFO is played out & we are not in audio master mode, so need to wait for AV Pipe Playout otherwise we can loose some initial audio of new program", playback_ip));
                    pPsi->psiState = PBIP_PsiState_eWaitingForAvPipePlayout;
                }
            }
        }

        /* Before we can change to the new AV PIDs, we will need to ensure that all AV Stream corresponding to current program PID has been decode & displayed. */
        if (pPsi->psiState == PBIP_PsiState_eWaitingForAvPipePlayout)
        {
            unsigned currentlyPlayedFrameCount;

            currentlyPlayedFrameCount = B_PlaybackIp_UtilsPlayedCount(playback_ip);
            if (lastPlayedFrameCount != currentlyPlayedFrameCount)
            {
                lastPlayedFrameCount = currentlyPlayedFrameCount;
                BKNI_Sleep(20);
                continue;
            }
            else
            {
                /* Sleep a frame time to playout any last frame. */
                BKNI_Sleep(20);
                BDBG_WRN(("%p: AV Decode pipe is played out", playback_ip));

                /* Now everything pertaining to the current program has been completely played out. */
                /* So we stop & start the AV decoders as this enables us to seemlessly play the new AV stream w/o loosing any initial Audio since this applies only to the video master mode. */
                stopAudioDecoder(&playback_ip->nexusHandles);
                stopVideoDecoder(&playback_ip->nexusHandles);

                if (startVideoDecoder(&playback_ip->nexusHandles) != B_ERROR_SUCCESS)
                {
                    BDBG_WRN(("%p: Failed to start the video decoder..", playback_ip));
                    break;
                }
                if (startAudioDecoder(&playback_ip->nexusHandles) != B_ERROR_SUCCESS)
                {
                    BDBG_WRN(("%p: Failed to start the audio decoder..", playback_ip));
                    break;
                }
                /* AV Decoder have been re-started, so we can change pids to the new program! */
                pPsi->psiState = PBIP_PsiState_eChangePids;
            }
        }

        /* AV Decoder have been re-started, so we can change pids to the new program! */
        if (pPsi->psiState == PBIP_PsiState_eChangePids)
        {
            if (videoPid != UINT_MAX && playback_ip->nexusHandles.videoPidChannel)
            {
                NEXUS_Error nrc;
                nrc = NEXUS_PidChannel_ChangePid(playback_ip->nexusHandles.videoPidChannel, videoPid);
                if (nrc != NEXUS_SUCCESS)
                {
                    BDBG_WRN(("%p: NEXUS_PidChannel_ChangePid() failed, nrc=%d", playback_ip, nrc));
                    break;
                }
                BDBG_WRN(("%p: Changed Video to pid =0x%x", playback_ip, videoPid));
            }
            /* TODO: need to select audio track based on the audioLanguage if provided/known/available else choose the 1st audio track. */
            /* currently, we are just selecting the 1st audio track in the new PMT. */
            if (audioPid1 != UINT_MAX && playback_ip->nexusHandles.audioPidChannel)
            {
                NEXUS_Error nrc;
                nrc = NEXUS_PidChannel_ChangePid(playback_ip->nexusHandles.audioPidChannel, audioPid1);
                if (nrc != NEXUS_SUCCESS)
                {
                    BDBG_WRN(("%p: NEXUS_PidChannel_ChangePid() failed for audioPid=%d nrc=%d", playback_ip, audioPid1, nrc));
                    break;
                }
                BDBG_WRN(("%p: Changed Audio to pid =0x%x", playback_ip, audioPid1));
            }
            BDBG_WRN(("%p: New Program PIDs are setup!", playback_ip));

            /* Now reset states to look for next updated PAT TS packet */
            pPsi->tsPktState = PBIP_TsPktState_eNew;
            pPsi->psiState = PBIP_PsiState_eWaitingForPat;
        }
    } /* while */
    pPsi->totalBytesConsumed += bytesConsumed;
    rc = B_ERROR_SUCCESS;

    if (rc != B_ERROR_SUCCESS)
    {
        B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
    }
    BDBG_MSG(("Psi=%p: stats: tsPktCnt=%lld totalPat=%lld, newPmt=%lld tsPktWithNoPayload=%lld tsPktWithError=%lld totalBytesConsumed=%lld syncByteCount=%lld totalInitialTsPktsSkipped=%lld",
                pPsi,
                pPsi->tsPktCnt,
                pPsi->tsPatPktCnt,
                pPsi->tsNewPmtPktCnt,
                pPsi->tsPktWithNoPayload,
                pPsi->tsPktWithError,
                pPsi->totalBytesConsumed,
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
    hPlaybackIp->pPsiState = B_PlaybackIp_CreatePsiState();
    BDBG_ASSERT(hPlaybackIp->pPsiState);
    return (B_ERROR_SUCCESS);
}
#endif /* LINUX || VxWorks */
