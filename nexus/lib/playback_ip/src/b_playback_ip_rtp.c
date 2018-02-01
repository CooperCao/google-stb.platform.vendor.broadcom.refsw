/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Description: IP Applib Implementation for IP_RTP Protocols in which
*              rtp payload contains MPEG2 TS packets.
*
***************************************************************************/

 /**
 * Note this file is currently only compiled for LINUX platforms. Even though it
 * has VxWorks & WinCE related hash defines, the code needs more porting
 * work for these OS platforms.
 * If the feature isn't compiled in, then the only functions in this .c file
 * should be the public stubs.
 **/
#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_lm_helper.h"
#include "b_playback_ip_psi.h"
#include <sys/ioctl.h>
#include <net/if.h>

BDBG_MODULE(b_playback_ip_rtp);
/**
* RTP Processing module of Playback IP Applib
*/

static int B_PlaybackIp_RtpGetData(
    void *dst,
    void *src,
    unsigned size
    )
{
    memcpy(dst, src, size);
    return 0;
}
#ifdef SPF_SUPPORT
    #define BRTP_DEFAULT_CFG         brtp_spf_default_cfg
    #define BRTP_CREATE              brtp_spf_create
    #define BRTP_DESTROY             brtp_spf_destroy
    #define BRTP_FEED                brtp_spf_feed
    #define BRTP_DEFAULT_SESSION_CFG brtp_spf_default_session_cfg
    #define BRTP_START               brtp_spf_start
    #define BRTP_STOP                brtp_spf_stop
#else
    #define BRTP_DEFAULT_CFG         brtp_default_cfg
    #define BRTP_CREATE              brtp_create
    #define BRTP_DESTROY             brtp_destroy
    #define BRTP_FEED                brtp_feed
    #define BRTP_DEFAULT_SESSION_CFG brtp_default_session_cfg
    #define BRTP_START               brtp_start
    #define BRTP_STOP                brtp_stop
#endif

void B_PlaybackIp_RtpInit(
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef SPF_SUPPORT
    brtp_spf_cfg cfg;
#else
    brtp_cfg cfg;
#endif /* SPF_SUPPORT */
    BDBG_MSG(("%s: \n", BSTD_FUNCTION));
    playback_ip->factory = batom_factory_create(bkni_alloc, 512);
    BDBG_ASSERT(playback_ip->factory);
    playback_ip->pipe_out = batom_pipe_create(playback_ip->factory);
    playback_ip->pipe_in = batom_pipe_create(playback_ip->factory);
    BRTP_DEFAULT_CFG(&cfg);
    cfg.factory = playback_ip->factory;
    cfg.copy_to_user = B_PlaybackIp_RtpGetData;
    playback_ip->rtp = BRTP_CREATE(&cfg);
    BDBG_ASSERT(playback_ip->rtp);
}

void B_PlaybackIp_RtpExit(
    B_PlaybackIpHandle playback_ip
    )
{
    BDBG_MSG(("%s: \n", BSTD_FUNCTION));
    BRTP_DESTROY(playback_ip->rtp);
    batom_pipe_destroy(playback_ip->pipe_in);
    batom_pipe_destroy(playback_ip->pipe_out);
    batom_factory_destroy(playback_ip->factory);
}

B_PlaybackIpError B_PlaybackIp_GetRtpHeaderData(
    B_PlaybackIpHandle playback_ip,
    unsigned int maxDataSize,
    B_PlaybackIp_RtpStatsHeader *rtpHeader,
    unsigned *entry_cnt
    )
{
    unsigned int dataLen;

    BKNI_AcquireMutex(playback_ip->rtp_hdr_data->lock);
    /* Only copy maximum allocated on caller side */
    dataLen = playback_ip->rtp_hdr_data->entry_cnt*sizeof(B_PlaybackIp_RtpStatsHeader);
    if (dataLen > maxDataSize)
        dataLen = maxDataSize;

    *entry_cnt = playback_ip->rtp_hdr_data->entry_cnt;
    memcpy(rtpHeader, &playback_ip->rtp_hdr_data->header, dataLen);
    playback_ip->rtp_hdr_data->entry_cnt = 0;
    BKNI_ReleaseMutex(playback_ip->rtp_hdr_data->lock);
    return B_ERROR_SUCCESS;
}

#ifdef DBG_RTP
int printRtp(B_PlaybackIp_RtpHeader *rtp, B_PlaybackIpHandle playback_ip)
{
    uint16_t word0;
    uint16_t version;
    uint16_t payloadType;
    static int cnt = 0;
    word0 = ntohs(rtp->word0);
    cnt++;
#if 0
    printf("RTP Hdr: Version %d Pad %d Ext %d CRSC count %d Marker %d Type %d ",
            (word0 & VERSION_MASK) >> VERSION_SHIFT,
            (word0 & PADDING_MASK) >> PADDING_SHIFT,
            (word0 & EXTENSION_MASK) >> EXTENSION_SHIFT,
            (word0 & CSRC_MASK) >> CSRC_SHIFT,
            (word0 & MARKER_MASK) >> MARKER_SHIFT,
            (word0 & TYPE_MASK) >> TYPE_SHIFT
          );
    printf("Seq %d, TS %u, SSRC %u\n",
            ntohs(rtp->sequenceNum),
            ntohl(rtp->timestamp),
            ntohl(rtp->ssrc));
#endif
#if 0
    /* if enabling this code, add initDone & prevSeq variables to the B_PlaybackIp structure in b_playback_ip_priv.h */
    if (!playback_ip->initDone) {
        playback_ip->prevSeq = ntohs(rtp->sequenceNum);
        playback_ip->initDone = 1;
    }
    else {
        int next;
        next = (++playback_ip->prevSeq) % 65536;
        if (next != ntohs(rtp->sequenceNum)) {
            printf("BREAK in RTP SEQ: prev %d, expected %d, next %d\n",
                    playback_ip->prevSeq, next, ntohs(rtp->sequenceNum));
            playback_ip->prevSeq = ntohs(rtp->sequenceNum);
        }
    }
#endif
    version = (word0 & VERSION_MASK) >> VERSION_SHIFT;
    payloadType = (word0 & TYPE_MASK) >> TYPE_SHIFT;
    if (version != 2 || payloadType != 33) {
        printf(" *** BAD PKT: ver %d, type %d *** out of %d pkts \n", version, payloadType, cnt);
        return -1;
    }
    return 0;
}
#endif


static void B_PlaybackIp_AtomUserFree(
    batom_t atom,
    void *user
    )
{
    struct B_PlaybackIpItem *item = *(B_PlaybackIpItem **)user;

    BSTD_UNUSED(atom);
    BDBG_MSG_FLOW(("recycling item: %p", (void *)item));

    BLST_Q_REMOVE(&item->playback_ip->active_item, item, active_item_queue);
    if (item->block_end) {
        /* we need to clear the block_end flag as block end can vary per recvfrom */
        /* this can happen when recvfrom doesn't return PKTS_PER_CHUNK pkts in low bitrate scenarios */
        item->block_end = 0;
        BLST_Q_INSERT_TAIL(&item->playback_ip->free_item, item->block_start, item_queue);
        BDBG_MSG_FLOW(("inserting block head item %p with block head index %d for item index %d to freelist\n",
                    (void *)item->block_start, item->block_start->item_index, item->item_index));
    }
}

static const batom_user B_PlaybackIpAtomUser = {
    B_PlaybackIp_AtomUserFree,
    sizeof(void **)
};

static void
B_PlaybackIp_RtpParseHeader(
    B_PlaybackIp_RtpStatsHeader *rtp_header,
    unsigned char *buf
    )
{
    rtp_header->version = (buf[0] & 0xC0) >> 6;
    rtp_header->padding = (buf[0] & 0x20) >> 5;
    rtp_header->extension = (buf[0] & 0x10) >> 4;
    rtp_header->csrc_count = (buf[0] & 0x0F) >> 0;

    rtp_header->marker = (buf[1] & 0x80) >> 7;
    rtp_header->payload_type = (buf[1] & 0x7F) >> 0;

    rtp_header->sequence_num = (buf[2] << 8);
    rtp_header->sequence_num |= buf[3];

    rtp_header->timestamp = (buf[4] << 24);
    rtp_header->timestamp |= (buf[5] << 16);
    rtp_header->timestamp |= (buf[6] << 8);
    rtp_header->timestamp |= (buf[7] << 0);

    rtp_header->ssrc = (buf[8] << 24);
    rtp_header->ssrc |= (buf[9] << 16);
    rtp_header->ssrc |= (buf[10] << 8);
    rtp_header->ssrc |= (buf[11] << 0);

#if 0
    BDBG_MSG(("RTP header: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
              buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]));
#endif
}

/* read PKTS_PER_CHUNK packets & feed them to the RTP processing engine */
int
B_PlaybackIp_ReadRtpPackets(
    B_PlaybackIpHandle playback_ip
    )
{
    int i;
    B_PlaybackIpError rc;
    batom_t atom;
    int result = -1;
    struct B_PlaybackIpItem *item;
    int udpPayloadLength = 0; /* udp payload length, i.e. rtp header + payload length */
    struct udp_hdr *udpHeader;
    B_PlaybackIp_RtpHeader *rtpHeader = NULL;
    int bytesRemaining;
    int totalBytesRecv;
    bool recvTimeout = false;
    bool flushPipeline = false;

    /* get the wrapper structure to hold each read packet */
    item = BLST_Q_FIRST(&playback_ip->free_item);
    if (!item) {
        /* no free items left currently as all are queued up for RTP processing, wait for more until next cycle */
        BDBG_ERR(("%s: out of items\n", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG_FLOW(("%s: item %p, item->data 0x%p\n", BSTD_FUNCTION, (void *)item, (void *)item->data));

    do {
        memset((int *)playback_ip->bytesRecvPerPkt, 0, PKTS_PER_CHUNK * sizeof(int));
        totalBytesRecv = 0;
        /* read next chunk of data from socket */
        rc = B_PlaybackIp_UtilsReadUdpPayloadChunk(
            playback_ip,
            (void *)(item->data),
            PKTS_PER_READ*IP_MAX_PKT_SIZE,  /* how much to read on each recvfrom call: 1pkt worth for std socket */
            PKTS_PER_CHUNK/PKTS_PER_READ,   /* # of iterations of recvfrom calls: 1 for accel socket */
            playback_ip->bytesRecvPerPkt,   /* list storing bytes read during each iteration */
            &totalBytesRecv,
            &recvTimeout
            );
        if (rc != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Network Read Error", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG_FLOW(("%s: read %d bytes from server\n", BSTD_FUNCTION, totalBytesRecv));
        if (recvTimeout) {
            /* network packet loss event, this constitutues a unmarked discontinuity */
            /* current video decoder & display manager is not able to gracefully recover from it */
            /* and we get a lag of couple of sec between Audio & Video (PR47521).*/
            /* Current solution is to flush the complete pipeline & wait until this n/w pkt loss condition clears */
            /* set a flag to do so when we recover from n/w loss condition */
            flushPipeline = true;
            /* let app know about this timeout using the ipTunerUnlockEvent */
            if (!playback_ip->ipTunerUnLockedEventSent && playback_ip->openSettings.eventCallback) {
                playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eIpTunerUnLocked);
                playback_ip->ipTunerUnLockedEventSent = true;
                playback_ip->ipTunerLockedEventSent = false; /* set the locked event flag so that it can be sent again once data starts coming in */
            }
            /* go back & try to read again */
            continue;
        }
        else {
            /* recvTimeout is not set, we just came out of packet loss condition*/
            playback_ip->ipTunerUnLockedEventSent = false; /* reset the TunerUnLockedEvent flag */
            /* flush the AV pipeline, drop this chunk of data, and then resume normal processing */
            if (flushPipeline) {
                if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip) < 0)
                    goto error;
                flushPipeline = false;
                /* drop this chunk and start fresh from the next chunk */
                continue;
            }
        }
    } while (recvTimeout);

    /* invoke ipTunerLocked event for after 1st time or after a network timeout exceeding the defined network jitter */
    if (!playback_ip->ipTunerLockedEventSent && playback_ip->openSettings.eventCallback) {
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eIpTunerLocked);
        playback_ip->ipTunerLockedEventSent = true;
    }

    /* got some data, so remove this item from list */
    BLST_Q_REMOVE_HEAD(&playback_ip->free_item, item_queue);

    /* now assign the read packet to individual atoms & then feed them to rtp processing engine */
    bytesRemaining = totalBytesRecv;
    i = 0;
    do
    {
        struct B_PlaybackIpItem *pitem;
        /* initialize item */
        item[i].playback_ip = playback_ip;
        BLST_Q_INSERT_TAIL(&playback_ip->active_item, &item[i], active_item_queue);
#ifdef B_HAS_NETACCEL
        udpHeader = (struct udp_hdr *) ((char *)(item[0].data) + i*IP_MAX_PKT_SIZE);
        if (ntohs(udpHeader->len) <= 0 ||  ntohs(udpHeader->len) > IP_MAX_PKT_SIZE) {
            BDBG_ERR(("ERROR in %s: pkt# %d, Received incorrect UDP Hdr fields, aborting app, UDP header: sport %d, dport %d, len %d\n",
                BSTD_FUNCTION, i, ntohs(udpHeader->source), ntohs(udpHeader->dest), ntohs(udpHeader->len)));
            goto error;
        }
        udpPayloadLength = ntohs(udpHeader->len) - sizeof(struct udp_hdr);
        rtpHeader = (B_PlaybackIp_RtpHeader *) (udpHeader + 1);
        bytesRemaining -= ntohs(udpHeader->len);
        BDBG_MSG_FLOW(("udp hdr[pkt = %d] sport: %d, dport %d, len %d\n", i, ntohs(udpHeader->source), ntohs(udpHeader->dest), ntohs(udpHeader->len)));
#else
        BSTD_UNUSED(udpHeader);
        if (i == 0)
            rtpHeader = (struct B_PlaybackIp_RtpHeader *) item[0].data;
        else
            rtpHeader = (struct B_PlaybackIp_RtpHeader *) ((char *)(rtpHeader) + playback_ip->bytesRecvPerPkt[i-1]);
        udpPayloadLength = playback_ip->bytesRecvPerPkt[i];
        bytesRemaining -= udpPayloadLength;
#endif
#ifdef DBG_RTP
        if (printRtp(rtpHeader, playback_ip)) {
            BDBG_ERR(("%s: Incorrect RTP header found at index %d", BSTD_FUNCTION, i));
            goto error;
        }
#endif
        /* Cache the RTP header for RTCP RR reports */
        BKNI_AcquireMutex(playback_ip->rtp_hdr_data->lock);
        B_PlaybackIp_RtpParseHeader(
            &playback_ip->rtp_hdr_data->header[playback_ip->rtp_hdr_data->entry_cnt],
            (unsigned char*)(rtpHeader));
        playback_ip->rtp_hdr_data->entry_cnt++;
        if(playback_ip->rtp_hdr_data->entry_cnt>=B_MAX_RTP_HEADERS)
            playback_ip->rtp_hdr_data->entry_cnt = 0;
        BKNI_ReleaseMutex(playback_ip->rtp_hdr_data->lock);

        /* get atom for this item */
        pitem = &item[i];
        atom = batom_from_range(playback_ip->factory, (const void *)(rtpHeader), udpPayloadLength, &B_PlaybackIpAtomUser, &pitem);
        BDBG_ASSERT(atom);
        if (!atom) {
            BDBG_ERR(("ERROR in %s: got NULL ATOM for a RTP Payload, possibly out of memory\n", BSTD_FUNCTION));
            goto error;
        }
        /* feed atom to rtp processing engine */
        batom_pipe_push(playback_ip->pipe_out, atom);

        BDBG_MSG_FLOW(("%s: pipe1, push %d atom, item->data: %p, atom: %p", BSTD_FUNCTION, i, (void *)udpHeader, (void *)atom));
        /* done with current packet, move to next packet */
        i++;
    } while (bytesRemaining);

    /* We may read into more than 1 items at once upto PKTS_PER_CHUNK */
    /* However, for really low bitrate or network error cases, we may not have read a complete PKTS_PER_CHUNK */
    /* we will need to mark the last item in a item block */
    /* this info is used by the free routine to insert only the start item of a block into the free list */
    /* only when the last_item in the block is freed */
    item[i-1].block_end = true;
    if (i != PKTS_PER_CHUNK) BDBG_MSG(("******** %s: # of item %d, idx: block end %d, start %d", BSTD_FUNCTION, i, item[i-1].item_index, item[0].item_index));
    result = 0;
error:
    return (result);
}

int
B_PlaybackIp_FeedRtpPacketsToPlaypump(
    B_PlaybackIpHandle playback_ip
    )
{
    int item_cnt = 0;
    batom_t         atom;
    batom_cursor    cursor;
    bool releaseToPlaypump = false;

    /* read from pipe until no data left */
    for (;;) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping) {
            BDBG_WRN(("%s: breaking out due to state %d change", BSTD_FUNCTION, playback_ip->playback_state));
            break;
        }
        atom = batom_pipe_peek(playback_ip->pipe_in);
        if (!atom || releaseToPlaypump) {
            /* no more atoms available: either all are processed or we have filled the current buffer */
            BDBG_MSG(("######## UDP/RTP processing not done yet, break out ######## \n"));
            /* if no data is yet copied to the playpump buffer (no atoms are yet processed), then break */
            if (playback_ip->byte_count == 0) {
                BDBG_MSG(("Nothing to FEED at this time, item_cnt %d\n", item_cnt));
                break;
            }
            if (playback_ip->startSettings.monitorPsi) {
                B_PlaybackIp_ParseAndProcessPsiState(playback_ip, playback_ip->buffer, playback_ip->byte_count);
            }
            /* enough has been accumulated, release buffer to playback */
            if (NEXUS_Playpump_ReadComplete(playback_ip->nexusHandles.playpump, 0, playback_ip->byte_count)) {
                /* don't treat this as an error: it can occur when the playpump overflows and is subsequently flushed */
                BDBG_WRN(("Returned error from NEXUS_Playpump_ReadComplete()!"));
            }
            BDBG_MSG(("%s: fed %lu byte to Playpump, item cnt %d\n", BSTD_FUNCTION, playback_ip->byte_count, item_cnt));
            playback_ip->totalConsumed += playback_ip->byte_count;
            /* write data to file */
            if (playback_ip->enableRecording && playback_ip->fclear) {
                fwrite(playback_ip->buffer, 1, playback_ip->byte_count, playback_ip->fclear);
                fflush(playback_ip->fclear);
            }
            playback_ip->byte_count = 0;

            /* get an adequately sized buffer from the playpump */
            if (B_PlaybackIp_UtilsGetPlaypumpBuffer(playback_ip, MAX_BUFFER_SIZE) < 0) {
                BDBG_WRN(("Returned error from NEXUS_Playpump_GetBuffer ()!"));
                goto error;
            }

            /* now that we have fed a chunk to playback, break out of here */
            break;
        }

        /* rtp engine finished processed an atom, copy the AV data into playpump buffer */
        batom_cursor_from_atom(&cursor, atom);
        BDBG_MSG_FLOW(("got atom: item cnt %d, byte cnt %lu, vec len %d, PP buffer len %d\n",
            item_cnt, playback_ip->byte_count, cursor.vec->len, playback_ip->buffer_size));
        if ((playback_ip->byte_count+cursor.vec->len) <= playback_ip->buffer_size ) {
            /* playback buffer has space for this item's data, so copy it to playback buffer */
            item_cnt++;
            atom = batom_pipe_pop(playback_ip->pipe_in);
            BDBG_ASSERT(atom);
            batom_cursor_from_atom(&cursor, atom);
            BDBG_MSG_FLOW(("pipe2, pop atom: %p, *cursor.vec->base: 0x%x, cursor.vec->len: %d",
                    (void *)atom, *(uint32_t *)cursor.vec->base, cursor.vec->len));

            /* copy this payload to Playback buffer */
            memcpy((void*)((unsigned long)playback_ip->buffer+playback_ip->byte_count), cursor.vec->base, cursor.vec->len);
            playback_ip->byte_count += cursor.vec->len;

            /* free up this atom */
            batom_release(atom);
            /* continue until no more processed atoms */
        }
        else {
            /* current buffer is almost full, so release it to playpump */
            releaseToPlaypump = true;
        }
    }
    return 0;
error:
    return -1;
}

void B_PlaybackIp_RtpProcessing(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
#ifdef B_HAS_NETACCEL
    STRM_SockRecvParams_t sockRecvParams = STRM_SOCK_RECV_PARAMS_RTP_DEFAULT;
#endif
    char recordFileName[32];
    static int fileNameSuffix = 0;

    BDBG_MSG(("Entered %s()\n", BSTD_FUNCTION));

    if (playback_ip->startSettings.monitorPsi) {
        if ( (playback_ip->pPsiState = B_PlaybackIp_CreatePsiState(playback_ip)) == NULL ) {
            BDBG_ERR(("%s: B_PlaybackIp_CreatePsiState() Failed to allocate memory for psiState", BSTD_FUNCTION));
            goto error;
        }
    }

    /* check if playpump & decoder are setup */
    if (B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup(playback_ip))
        goto error;

    if (playback_ip->enableRecording) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/rtp_playpump_rec%d.ts", fileNameSuffix++);
        playback_ip->fclear = fopen(recordFileName, "w+b");
    }

#ifdef B_HAS_NETACCEL
    memset((char *)&sockRecvParams, 0, sizeof(sockRecvParams));
    sockRecvParams.hdrOffset = 0;
    if(playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip) {
        sockRecvParams.pktsPerRecv = 200;
        sockRecvParams.recvTimeout = 10;
    }
    else {
        sockRecvParams.recvTimeout = 35;
        sockRecvParams.pktsPerRecv = PKTS_PER_READ;
    }
    /* PR48500: Sumitomo would need to encrypt the live IP stream while recording. */
    /* This use of M2M DMA engine for encryption purpose requires synchronization of DMA engine */
    /* also being used by the netaccel driver to user copies purposes. However, this synchronization */
    /* is really hard to achieve if the encryption module (PI) is in user space & netaccel driver in the kernel. */
    /* Alternative solution is to not use M2M engine for user copies in netaccel. This doesn't impose any significant */
        /* write data to file */
    sockRecvParams.useCpuCopy = true;

    /* Tell netaccel driver to copy each packet at a fixed offset. This lets application receive variable size packets. */
    sockRecvParams.pktOffset = IP_MAX_PKT_SIZE;

    if (setsockopt(playback_ip->socketState.fd, SOCK_BRCM_DGRAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)) != 0)
    {
        BDBG_ERR(("%s: setsockopt() ERROR:", BSTD_FUNCTION));
        /* in case of failure (shouldn't happen), read 1 pkt at a time */
    }
    BDBG_ERR(("%s: Modified the default pkts per recvfrom to %d, payload offset %d\n",
                BSTD_FUNCTION, PKTS_PER_READ, sockRecvParams.pktOffset));
#endif

    if (playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip
        || playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip) {
        B_PlaybackIp_TtsThrottle_Params throttleParams;
        NEXUS_PlaypumpSettings playpumpSettings;

        playback_ip->ttsThrottle = B_PlaybackIp_TtsThrottle_Open();

        B_PlaybackIp_TtsThrottle_GetSettings(playback_ip->ttsThrottle, &throttleParams);
        BKNI_Memcpy(&throttleParams, &playback_ip->settings.ttsParams.throttleParams, sizeof(B_PlaybackIp_TtsThrottle_Params));
        throttleParams.playPump = playback_ip->nexusHandles.playpump;
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump, &playpumpSettings);
        throttleParams.timebase = playpumpSettings.timestamp.timebase;
        B_PlaybackIp_TtsThrottle_SetSettings(playback_ip->ttsThrottle, &throttleParams);
        B_PlaybackIp_TtsThrottle_Start(playback_ip->ttsThrottle);

        B_PlaybackIp_UtilsFlushSocket(playback_ip);
#ifdef B_HAS_NETACCEL
        sockRecvParams.pktsPerRecv = PKTS_PER_READ;
        if (setsockopt(playback_ip->socketState.fd, SOCK_BRCM_DGRAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)) != 0)
        {
            BDBG_ERR(("%s: setsockopt() ERROR:", BSTD_FUNCTION));
            /* in case of failure (shouldn't happen), read 1 pkt at a time */
        }
#endif
    }

    /* get an adequately sized buffer from the playpump */
    if (B_PlaybackIp_UtilsGetPlaypumpBuffer(playback_ip, MAX_BUFFER_SIZE) < 0)
        goto error;

    if (B_PlaybackIp_UtilsDiscardBufferedData(playback_ip, playback_ip->socketState.fd, SOCK_DGRAM) == false) {
        BDBG_WRN(("%s: B_PlaybackIp_UtilsDiscardBufferedData failed, but continuing...", BSTD_FUNCTION));
    }

    /* main loop */
    while (playback_ip->playback_state != B_PlaybackIpState_eStopping) {
        /* read udp/rtp data from socket & wrap each packet into a atom */
        if (B_PlaybackIp_ReadRtpPackets(playback_ip))
            goto error;

        /* invoke the rtp engine to do RTP processing on the queued up packets */
        BRTP_FEED(playback_ip->rtp, playback_ip->pipe_out);

        /* get the processed packets from rtp engine & feed them to the playpump */
        if (B_PlaybackIp_FeedRtpPacketsToPlaypump(playback_ip) != 0)
            goto error;
    } /* while */

error:
    if (playback_ip->pPsiState) { B_PlaybackIp_DestroyPsiState(playback_ip->pPsiState); }

    if (playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip
        || playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip) {
        B_PlaybackIp_TtsThrottle_Stop(playback_ip->ttsThrottle);
        B_PlaybackIp_TtsThrottle_Close(playback_ip->ttsThrottle);
    }

    BDBG_MSG(("Exiting %s()\n", BSTD_FUNCTION));
    BKNI_SetEvent(playback_ip->playback_halt_event);
}

void
B_PlaybackIp_RtpSessionClose(
    B_PlaybackIpHandle playback_ip
    )
{
    if (playback_ip->item) {
        BKNI_Free(playback_ip->item);
        playback_ip->item = NULL;
    }
    if (playback_ip->item_mem) {
        BKNI_Free(playback_ip->item_mem);
        playback_ip->item_mem = NULL;
    }
    if (playback_ip->rtp_hdr_data && playback_ip->rtp_hdr_data->lock) {
        BKNI_DestroyMutex(playback_ip->rtp_hdr_data->lock);
        playback_ip->rtp_hdr_data->lock = NULL;
    }
    if (playback_ip->rtp_hdr_data) {
        BKNI_Free(playback_ip->rtp_hdr_data);
        playback_ip->rtp_hdr_data = NULL;
    }
    if (playback_ip->bytesRecvPerPkt) {
        BKNI_Free(playback_ip->bytesRecvPerPkt);
        playback_ip->bytesRecvPerPkt = NULL;
    }
    if (playback_ip->discard_buf) {
        BKNI_Free(playback_ip->discard_buf);
        playback_ip->discard_buf = NULL;
    }
#ifdef LIVEMEDIA_SUPPORT
    if (playback_ip->lm_context) {
        B_PlaybackIp_liveMediaSessionClose(playback_ip->lm_context);
        playback_ip->lm_context = NULL;
    }
#endif

    B_PlaybackIp_UtilsMediaProbeDestroy(playback_ip);

#ifdef EROUTER_SUPPORT
    B_PlaybackIp_UtilsUnsetErouterFilter(playback_ip);
#endif

}

extern B_PlaybackIpError
B_PlaybackIp_UdpSetupSimpleSocket(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSocketState *socketState,
    struct addrinfo *addrInfo
    );

B_PlaybackIpError
B_PlaybackIp_RtpSessionOpen(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSessionOpenStatus *openStatus /* [out] */
    )
{
    int i;
    struct sockaddr_in laddr;
    socklen_t len;
    int block_start_idx = 0;
    B_PlaybackIpSocketState *socketState;
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    char portString[16];
    bool ipAddressIsUnicast = false;

    if (!playback_ip || !openSettings || !openStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, openSettings %p, openStatus %p\n", BSTD_FUNCTION, (void *)playback_ip, (void *)openSettings, (void *)openStatus));
        return B_ERROR_INVALID_PARAMETER;
    }
    socketState = &openStatus->socketState;

    /* use getaddinfo() to determine whether user specified ipAddr is a v4 or v6 address */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString), "%d", openSettings->socketOpenSettings.port);
    if (getaddrinfo(openSettings->socketOpenSettings.ipAddr, portString, &hints, &addrInfo) != 0) {
        BDBG_ERR(("%s: ERROR: getaddrinfo failed: %s, errno %d", BSTD_FUNCTION, openSettings->socketOpenSettings.ipAddr, errno));
        perror("getaddrinfo");
        errorCode = B_ERROR_INVALID_PARAMETER;
        goto error;
    }
    if (addrInfo->ai_family != AF_INET && addrInfo->ai_family != AF_INET6) {
        BDBG_WRN(("%s: ERROR: Unsupported Address family %d\n", BSTD_FUNCTION, addrInfo->ai_family));
        errorCode = B_ERROR_SOCKET_ERROR;
        goto error;
    }

    if (addrInfo->ai_family == AF_INET6 && openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtp) {
        openSettings->socketOpenSettings.protocol = B_PlaybackIpProtocol_eRtpNoRtcp;
        BDBG_WRN(("%s: Only Basic RTP protocol is supported, RTCP is not yet supported by LiveMedia Library for IPv6 Protocol\n", BSTD_FUNCTION));
    }
    else if (addrInfo->ai_family == AF_INET6 && openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtsp) {
        BDBG_WRN(("%s: RTSP protocol is NOT supported over IPv6 as LiveMedia Library doesn't yet support IPv6 Protocol\n", BSTD_FUNCTION));
        errorCode = B_ERROR_NOT_SUPPORTED;
        goto error;
    }

    BDBG_WRN(("Channel Info: socktype %d, proto = %s, port %d, addr %s", addrInfo->ai_socktype,
        (openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtp) ? "RTP":"RTSP",
        ntohs(((struct sockaddr_in *)addrInfo->ai_addr)->sin_port),
        inet_ntoa(((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr)));

    if (addrInfo->ai_family == AF_INET) {
        if (!IN_MULTICAST(ntohl(((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr)))
            ipAddressIsUnicast = true;
    }
    else if (addrInfo->ai_family == AF_INET6) {
        if (!IN6_IS_ADDR_MULTICAST(((struct sockaddr_in6 *)(addrInfo->ai_addr))->sin6_addr.s6_addr))
            ipAddressIsUnicast = true;
    }

    /* allocate the items which are contained in the atoms for RTP processing */
    playback_ip->item = (B_PlaybackIpItem *)BKNI_Malloc(IP_MAX_ITEMS * sizeof(struct B_PlaybackIpItem));
    if (!playback_ip->item) {
        BDBG_ERR(("%s: Failed to allocate memory \n", BSTD_FUNCTION));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    memset(playback_ip->item, 0, IP_MAX_ITEMS * sizeof(struct B_PlaybackIpItem));
    BDBG_MSG(("%s: allocated %d bytes for %d items\n", BSTD_FUNCTION, IP_MAX_ITEMS * (int)sizeof(struct B_PlaybackIpItem), IP_MAX_ITEMS));
    /* setup the item->data pointer: this is where AV data is copied in the recv calls */
    playback_ip->item_mem = (uint8_t *)BKNI_Malloc(IP_MAX_ITEMS * IP_MAX_PKT_SIZE);
    if (!playback_ip->item_mem) {
        BDBG_ERR(("%s: Failed to allocate memory \n", BSTD_FUNCTION));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    BDBG_MSG(("item_mem %p, item size %d, data size %d\n",
              (void *)playback_ip->item_mem, (int)sizeof(struct B_PlaybackIpItem), (IP_MAX_ITEMS * IP_MAX_PKT_SIZE)));
    /* now setup the items data pointer */
    for (i=0; i < IP_MAX_ITEMS; i++) {
        playback_ip->item[i].data = playback_ip->item_mem + i*IP_MAX_PKT_SIZE;
        playback_ip->item[i].item_index = i;
        /* For both regular & accelerated sockets, we try to read upto PKTS_PER_CHUNK items per recvfrom call */
        /* thus there can be upto PKTS_PER_CHUNK items per read block */
        /* we group the items into PKTS_PER_CHUNK blocks and use only 1st item for free & active list management */
        /* every PKTS_PER_CHUNKth item is the starting block for each item in that block */
        if (i % PKTS_PER_CHUNK == 0)
            block_start_idx = i;
        playback_ip->item[i].block_start = &playback_ip->item[block_start_idx];
    }

    /* rtp header initialization: we cache rtp header of each packet and make them available for RTCP stats purposes */
    playback_ip->rtp_hdr_data = (B_PlaybackIp_RtpHeaderData *)BKNI_Malloc(sizeof(B_PlaybackIp_RtpHeaderData));
    if (!playback_ip->rtp_hdr_data) {
        BDBG_ERR(("%s: Failed to allocate memory\n", BSTD_FUNCTION));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    playback_ip->rtp_hdr_data->entry_cnt = 0;
    if (BKNI_CreateMutex(&playback_ip->rtp_hdr_data->lock)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    playback_ip->bytesRecvPerPkt = (int *)BKNI_Malloc(PKTS_PER_CHUNK * sizeof(int));
    playback_ip->discard_buf = (char *)BKNI_Malloc(DISCARD_BUFFER_SIZE * sizeof(char));
    if (playback_ip->bytesRecvPerPkt == NULL || playback_ip->discard_buf == NULL) {
        BDBG_ERR(("%s: BKNI_Malloc failed: recv buff %p, discard_buf %p\n", BSTD_FUNCTION, (void *)playback_ip->bytesRecvPerPkt, (void *)playback_ip->discard_buf));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }

#ifdef LIVEMEDIA_SUPPORT
    if (openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtp) {
        /* we only do the Live Media calls for RTP protocols. This is because this RtpSessionOpen() */
        /* is also called by the RtspSessionOpen which should have already initialized the Live Media directly */

        /* We pass IP Channel info (IP, Port, RTSP URL) to the Live Media library */
        /* Library is responsible for setting up the initial socket information  & returns the socket fd back */
        playback_ip->lm_context = NULL;
        errorCode = B_PlaybackIp_liveMediaSessionOpen(playback_ip, openSettings, &playback_ip->lm_context, &socketState->fd);
        if (errorCode != B_ERROR_SUCCESS)
            goto error;

        BDBG_ASSERT(socketState->fd);
        BDBG_ASSERT(playback_ip->lm_context);
        B_PlaybackIp_liveMediaSessionSetup(playback_ip->lm_context);
    }
#endif
    if (openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtpNoRtcp) {
        if (B_PlaybackIp_UdpSetupSimpleSocket(playback_ip, openSettings, socketState, addrInfo) != B_ERROR_SUCCESS) {
            goto error;
        }
    }

#if !defined(B_HAS_NETACCEL)
    {
        int reuse_flag = 1;
        /* Make socket reusable for PiP and record channels */
        if (setsockopt(socketState->fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
            BDBG_WRN(("Set REUSEADDR failed, PiP may not work"));
        }
    }
    if (fcntl(socketState->fd, F_SETFL, O_NONBLOCK)) {
        BDBG_WRN(("can't set O_NONBLOCK mode"));
    }
#endif

    /*
    * When LiveMedia Library is compiled for supporting RTP & RTSP, data socket is setup by this library.
    * Since we dont want to modify this 3rd party library to use accelerated socket, we will instead setup
    * an accelerated socket here. This will cause all incoming data for both Live RTP & RTSP Playback
    * sessions to come via accelerated sockets. Livemedia's data socket doesn't see any RTP data on it.
    * Instead, IP Applib caches RTP headers of each incoming RTP packets and makes it available to
    * Livemedia via a callback for RTCP stats purposes.
    */

    /* Get local address & port information from the socket created by the LiveMedia library */
    BKNI_Memset(&laddr, 0, sizeof(laddr));
    len = sizeof(laddr);
    if (getsockname(socketState->fd, (struct sockaddr *)&laddr, &len) != 0) {
        BDBG_ERR(("%s: failed to get socket information\n", BSTD_FUNCTION));
        errorCode = B_ERROR_PROTO;
        goto error;
    }

    if (ntohl(laddr.sin_addr.s_addr) == INADDR_ANY) {
        /* LiveMedia library hasn't bind an IP to the socket determine the local IP address as */
        if (openSettings->socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtp) {
            /* For RTP, IP address is the one specified by the app in the Socket Open Settings */
            /* as it can be a live session to a multicast address */
            laddr.sin_addr.s_addr = inet_addr(openSettings->socketOpenSettings.ipAddr);
        }
        else {
#if defined(B_HAS_NETACCEL)
            /* For RTSP, IP address is the address of local interface as RTSP always uses unicast IP */
            struct ifreq ifr;
            /* TODO: Add code to retrieve the interface index instead of hard coding the i/f name */
            strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

            /* now retrieve the IP address associated with the media */
            if (ioctl(socketState->fd, SIOCGIFADDR, &ifr) != 0) {
                /* Try 7118 on-chip eth, via BNM front end */
                strncpy(ifr.ifr_name, "eth0:1", IFNAMSIZ);
                if (ioctl(socketState->fd, SIOCGIFADDR, &ifr) != 0) {
                        BDBG_WRN(("Failed to get Interface Address Information"));
                        errorCode = B_ERROR_SOCKET_ERROR;
                        goto error;
                }
            }
            laddr.sin_addr.s_addr = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr;
#endif
        }
    }
    BDBG_WRN(("%s: Local addr 0x%x, port %d\n", BSTD_FUNCTION, ntohl(laddr.sin_addr.s_addr), ntohs(laddr.sin_port)));

#if defined(B_HAS_NETACCEL)
    /* create accelerated DGRAM socket */
    if ((socketState->fd = socket(AF_INET, SOCK_BRCM_DGRAM, 0)) == -1 ) {
        BDBG_ERR(("%s: Failed to create the socket, errno %d", BSTD_FUNCTION, errno));
        errorCode = B_ERROR_SOCKET_ERROR;
        goto error;
    }

    if (bind(socketState->fd, (struct sockaddr *) &(laddr), sizeof (struct sockaddr)) == -1) {
        BDBG_ERR(("%s: Bind failed, errno %d", BSTD_FUNCTION, errno));
        errorCode = B_ERROR_SOCKET_ERROR;
        goto error;
    }
#endif

#ifdef EROUTER_SUPPORT
    if (ipAddressIsUnicast) {
        if (B_PlaybackIp_UtilsSetErouterFilter(playback_ip, openSettings) != B_ERROR_SUCCESS)
            goto error;
    }
#else
    BSTD_UNUSED(ipAddressIsUnicast);
#endif

    B_PlaybackIp_UtilsTuneNetworkStack(socketState->fd);
    errorCode = B_ERROR_SUCCESS;

    BDBG_MSG(("%s: successfully opened the RTP session (fd %d)", BSTD_FUNCTION, socketState->fd));
    freeaddrinfo(addrInfo);
    return errorCode;

error:
    B_PlaybackIp_RtpSessionClose(playback_ip);
    freeaddrinfo(addrInfo);
    return errorCode;
}

B_PlaybackIpError
B_PlaybackIp_RtpSessionSetup(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionSetupSettings *setupSettings,
    B_PlaybackIpSessionSetupStatus *setupStatus /* [out] */
    )
{
    int rc = -1;
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (!playback_ip || !setupSettings || !setupStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, setupSettings %p, setupStatus %p\n", BSTD_FUNCTION, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        return errorCode;
    }

    /* if SessionSetup is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if SessionSetup is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_MSG(("%s: previously started session setup operation completed, playback_ip %p", BSTD_FUNCTION, (void *)playback_ip));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    /* Neither SessionSetup is in progress nor it is completed, so start setup */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted = false;

    if (setupSettings->u.udp.skipPsiParsing) {
        errorCode = B_ERROR_SUCCESS;
        rc = 0;
        goto error;
    }

    BDBG_MSG(("%s: psiParsingTimeLimit %ld", BSTD_FUNCTION, playback_ip->setupSettings.u.udp.psiParsingTimeLimit));
    if (playback_ip->openSettings.nonBlockingMode) {
        /* do PSI parsing in a thread and return back to app */
        playback_ip->sessionSetupThread = B_Thread_Create("SessionSetupThread", (B_ThreadFunc)B_PlaybackIp_UtilsMediaProbeCreate, (void *)playback_ip, NULL);
        if (NULL == playback_ip->sessionSetupThread) {
            BDBG_ERR(("%s: Failed to create thread for media probe during Session Setup\n", BSTD_FUNCTION));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: Non blocking media probe operation started: playback_ip %p\n", BSTD_FUNCTION, (void *)playback_ip));
#endif
        errorCode = B_ERROR_IN_PROGRESS;
        goto error;
    }
    else {
        B_PlaybackIp_UtilsMediaProbeCreate(playback_ip);
    }

done:
    /* check the success of probing function */
    if (!playback_ip->psi.psiValid) {
        BDBG_ERR(("%s: Failed to acquire PSI info via media probe\n", BSTD_FUNCTION));
        errorCode = B_ERROR_UNKNOWN;
        goto error;
    }
    setupStatus->u.udp.psi = playback_ip->psi;
    setupStatus->u.udp.stream = (void *)playback_ip->stream;
    errorCode = B_ERROR_SUCCESS;
    /* success */
    rc = 0;

error:
    if (errorCode != B_ERROR_IN_PROGRESS) {
        playback_ip->apiInProgress = false;
        playback_ip->apiCompleted = false;
    }
    if (rc < 0) {
        return errorCode;
    }
    else {
        return B_ERROR_SUCCESS;
    }
}

/*
Summary:
    Read callback from playpump
Description:
    We can't do the read and read_complete in the callback, so notify
    the playback_func task
*/

static void
B_PlaybackIp_ReadCallback(void *context, int param)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;
    BKNI_SetEvent(playback_ip->read_callback_event);
    BSTD_UNUSED(param);
}

B_PlaybackIpError
B_PlaybackIp_RtpSessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus /* [out] */
    )
{
    int i;
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    B_ThreadSettings settingsThread;
    char *threadName;
    int transportPacketSize;
#ifdef SPF_SUPPORT
    brtp_spf_session_cfg session_cfg;
#else
    brtp_session_cfg session_cfg;
#endif /* SPF_SUPPORT */
    NEXUS_Error rc = B_ERROR_UNKNOWN;
    NEXUS_PlaypumpSettings nSettings;

    if (!playback_ip || !startSettings || !startStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", BSTD_FUNCTION, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        return errorCode;
    }

    if (startSettings->nexusHandlesValid && startSettings->nexusHandles.playpump) {
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump, &nSettings);
        nSettings.dataCallback.callback = B_PlaybackIp_ReadCallback;
        nSettings.dataCallback.context = playback_ip;
        rc = NEXUS_Playpump_SetSettings(playback_ip->nexusHandles.playpump, &nSettings);
        if (rc) {
            BDBG_ERR(("%s:%d Nexus Error: %d\n", BSTD_FUNCTION, __LINE__, rc));
            goto error;
        }
        playback_ip->pumpparams = nSettings;
    }

    if (playback_ip->pumpparams.timestamp.type != NEXUS_TransportTimestampType_eNone) {
        BDBG_WRN(("%s: Timestamp mode is enabled", BSTD_FUNCTION));
        transportPacketSize = TS_PKT_SIZE + 4; /* 4 byte timestamp per Transport packet */
    }
    else {
        transportPacketSize = TS_PKT_SIZE;
    }

    BDBG_WRN(("Starting Live IP/RTP Session"));
    /* Note: Assumption here is that we are dealing w/ only Fixed Size RTP header of 12 bytes */
#if defined (B_HAS_NETACCEL)
    /* In case of RTP, we receive both UDP & RTP headers from NetAccel */
    playback_ip->payload_size = TS_PKTS_PER_UDP_PKT * transportPacketSize + 20;
#else
    /* In case of Normal sockets for RTP, we receive starting from RTP headers (UDP header is stripped by kernel) */
    playback_ip->payload_size = TS_PKTS_PER_UDP_PKT * transportPacketSize + 12;
#endif
    B_PlaybackIp_RtpInit(playback_ip);
    BRTP_DEFAULT_SESSION_CFG(playback_ip->rtp, &session_cfg);
    session_cfg.pt_mask = 0xFF;
    if (startSettings->rtp.rtpPayloadType != 0)
        session_cfg.pt_value = startSettings->rtp.rtpPayloadType;
    else if (startSettings->mpegType == NEXUS_TransportType_eEs)
        session_cfg.pt_value = RTP_PAYLOAD_TYPE_ES;
    else
        session_cfg.pt_value = RTP_PAYLOAD_TYPE_MP2T;
    BDBG_MSG(("%s: Using Payload type %d", BSTD_FUNCTION, session_cfg.pt_value ));
    BRTP_START(playback_ip->rtp, playback_ip->pipe_in, &session_cfg);

    BLST_Q_INIT(&playback_ip->free_item);
    BLST_Q_INIT(&playback_ip->active_item);
    for (i=0;i<IP_MAX_ITEMS/PKTS_PER_CHUNK;i++) {
        /* since we read PKTS_PER_CHUNK pkts from sockets, we group individual items into PKTS_PER_CHUNK size blocks */
        /* and only insert the 1st item of each block in the free list */
        BLST_Q_INSERT_TAIL(&playback_ip->free_item, &playback_ip->item[i*PKTS_PER_CHUNK], item_queue);
        BDBG_MSG_FLOW(("added item %d addr %p to free list\n", i*PKTS_PER_CHUNK, (void *)&playback_ip->item[i*PKTS_PER_CHUNK]));
    }

    threadName = "PlaybackIpRtp";
    /* create thread to process incoming IP packets & feed them to PB hw */
    B_Thread_GetDefaultSettings(&settingsThread);
    playback_ip->playbackIpThread = B_Thread_Create(threadName, (B_ThreadFunc)B_PlaybackIp_RtpProcessing, (void *)playback_ip, &settingsThread);
    if (NULL == playback_ip->playbackIpThread) {
        BDBG_ERR(("%s: Failed to create the %s thread for RTP media transport protocol\n", BSTD_FUNCTION, threadName));
        goto error;
    }

#ifdef LIVEMEDIA_SUPPORT
    if (playback_ip->protocol == B_PlaybackIpProtocol_eRtp)
        /* LiveMediaSessionStart is already called for RTSP protocol in RtspStart() */
        B_PlaybackIp_liveMediaSessionStart(playback_ip->lm_context);
#endif

    errorCode = B_ERROR_SUCCESS;

error:
#ifdef LIVEMEDIA_SUPPORT
    if (errorCode != B_ERROR_SUCCESS && playback_ip->protocol == B_PlaybackIpProtocol_eRtp && playback_ip->lm_context)
        B_PlaybackIp_liveMediaSessionStop(playback_ip->lm_context);
#endif
    return errorCode;
}

void
B_PlaybackIp_RtpSessionStop(
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef LIVEMEDIA_SUPPORT
    if (playback_ip->lm_context)
        B_PlaybackIp_liveMediaSessionStop(playback_ip->lm_context);
#endif
    if (playback_ip->rtp) {
        BRTP_STOP(playback_ip->rtp);
        B_PlaybackIp_RtpExit(playback_ip);
        BLST_Q_INIT(&playback_ip->free_item);
        BLST_Q_INIT(&playback_ip->active_item);
    }
    if (playback_ip->fclear) {
        fclose(playback_ip->fclear);
    }
}

#endif /* LINUX || VxWorks */
