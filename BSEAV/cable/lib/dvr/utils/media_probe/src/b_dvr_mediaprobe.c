/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 ****************************************************************************/

#include "nexus_platform.h"
#include "b_dvr_manager.h"
#include "b_dvr_mediaprobe.h"
#include "b_dvr_file.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_drmservice.h"
#include "bmpeg2ts_parser.h"
#include "bmpeg2pes_parser.h"
#include "bmedia_probe_es.h"
#include "bmpeg_video_probe.h"
#include "bmpeg_audio_probe.h"
#include "bavc_video_probe.h"
#include "bhevc_video_probe.h"
#include "baac_adts_probe.h"
#include "baac_loas_probe.h"
#include "bac3_probe.h"
#include "bmedia_probe_impl.h"
#include "b_dvr_manager_priv.h"


BDBG_MODULE(b_dvr_mediaprobe);
BDBG_OBJECT_ID(B_DVR_MediaProbe);
typedef struct videoProbeMap
{
    NEXUS_VideoCodec videoCodec;
    bmedia_probe_es_desc *videoProbe;
}videoProbeMap;

typedef struct audioProbeMap
{
    NEXUS_AudioCodec audioCodec;
    bmedia_probe_es_desc *audioProbe;
}audioProbeMap;

static videoProbeMap g_videoProbeMap[] =
{
    {NEXUS_VideoCodec_eUnknown,NULL},                      /* unknown/not supported video codec */
    {NEXUS_VideoCodec_eNone,NULL},                         /* unknown/not supported video codec */
    {NEXUS_VideoCodec_eMpeg1,NULL},                        /* MPEG-1 Video (ISO/IEC 11172-2) */
    {NEXUS_VideoCodec_eMpeg2,(bmedia_probe_es_desc*)&bmpeg_video_probe},           /* MPEG-2 Video (ISO/IEC 13818-2) */
    {NEXUS_VideoCodec_eMpeg4Part2,NULL},                   /* MPEG-4 Part 2 Video */
    {NEXUS_VideoCodec_eH263,NULL},                         /* H.263 Video. The value of the enum is not based on PSI standards. */
    {NEXUS_VideoCodec_eH264,(bmedia_probe_es_desc *)&bavc_video_probe},             /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC */
    {NEXUS_VideoCodec_eH265,(bmedia_probe_es_desc *)&bhevc_video_probe},             /* HEVC */
    {NEXUS_VideoCodec_eH264_Svc,NULL},                     /* Scalable Video Codec extension of H.264 */
    {NEXUS_VideoCodec_eH264_Mvc,NULL},                     /* Multi View Coding extension of H.264 */
    {NEXUS_VideoCodec_eVc1,NULL},                          /* VC-1 Advanced Profile */
    {NEXUS_VideoCodec_eVc1SimpleMain,NULL},                /* VC-1 Simple & Main Profile */
    {NEXUS_VideoCodec_eDivx311,NULL},                      /* DivX 3.11 coded video */
    {NEXUS_VideoCodec_eAvs,NULL},                          /* AVS coded video */
    {NEXUS_VideoCodec_eRv40,NULL},                         /* RV 4.0 coded video */
    {NEXUS_VideoCodec_eVp6,NULL},                          /* VP6 coded video */
    {NEXUS_VideoCodec_eVp7,NULL},                          /* VP7 coded video */
    {NEXUS_VideoCodec_eVp8,NULL},                          /* VP8 coded video */
    {NEXUS_VideoCodec_eSpark,NULL},                        /* H.263 Sorenson Spark coded video */
    {NEXUS_VideoCodec_eMax,NULL}
};

static audioProbeMap g_audioProbeMap[] =
{
    {NEXUS_AudioCodec_eUnknown,NULL},                         /* unknown/not supported audio format */
    {NEXUS_AudioCodec_eMpeg,(bmedia_probe_es_desc *)&bmpeg_audio_probe},              /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
    {NEXUS_AudioCodec_eMp3,NULL},                             /* MPEG1/2, layer 3. */
    {NEXUS_AudioCodec_eAac,(bmedia_probe_es_desc *)&baac_adts_probe},                 /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    {NEXUS_AudioCodec_eAacAdts,(bmedia_probe_es_desc *)&baac_adts_probe},             /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    {NEXUS_AudioCodec_eAacLoas,(bmedia_probe_es_desc *)&baac_loas_probe},             /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    {NEXUS_AudioCodec_eAacPlus,(bmedia_probe_es_desc *)&baac_adts_probe},             /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    {NEXUS_AudioCodec_eAacPlusLoas,(bmedia_probe_es_desc *)&baac_loas_probe},         /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    {NEXUS_AudioCodec_eAacPlusAdts,(bmedia_probe_es_desc *)&baac_adts_probe},         /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with ADTS (Audio Data Transport Format) sync */
    {NEXUS_AudioCodec_eAc3,(bmedia_probe_es_desc *)&bac3_probe},                      /* Dolby Digital AC3 audio */
    {NEXUS_AudioCodec_eAc3Plus,(bmedia_probe_es_desc *)&bac3_probe},                  /* Dolby Digital Plus (AC3+ or DDP) audio */
    {NEXUS_AudioCodec_eDts,NULL},                             /* Digital Digital Surround sound, uses non-legacy frame-sync */
    {NEXUS_AudioCodec_eLpcmDvd,NULL},                         /* LPCM, DVD mode */
    {NEXUS_AudioCodec_eLpcmHdDvd,NULL},                       /* LPCM, HD-DVD mode */
    {NEXUS_AudioCodec_eLpcmBluRay,NULL},                      /* LPCM, Blu-Ray mode */
    {NEXUS_AudioCodec_eDtsHd,NULL},                           /* Digital Digital Surround sound, HD, uses non-legacy frame-sync, decodes only DTS part of DTS-HD stream */
    {NEXUS_AudioCodec_eWmaStd,NULL},                          /* WMA Standard */
    {NEXUS_AudioCodec_eWmaStdTs,NULL},                        /* WMA Standard with a 24-byte extended header */
    {NEXUS_AudioCodec_eWmaPro,NULL},                          /* WMA Professional */
    {NEXUS_AudioCodec_eAvs,NULL},                             /* AVS */
    {NEXUS_AudioCodec_ePcm,NULL},                             /* PCM audio - Generally used only with inputs such as SPDIF or HDMI. */
    {NEXUS_AudioCodec_ePcmWav,NULL},                          /* PCM audio with Wave header - Used with streams containing PCM audio */
    {NEXUS_AudioCodec_eAmr,NULL},                             /* Adaptive Multi-Rate compression (typically used w/3GPP) */
    {NEXUS_AudioCodec_eDra,NULL},                             /* Dynamic Resolution Adaptation.  Used in Blu-Ray and China Broadcasts. */
    {NEXUS_AudioCodec_eCook,NULL},                            /* Real Audio 8 LBR */
    {NEXUS_AudioCodec_eAdpcm,NULL},                           /* MS ADPCM audio format */
    {NEXUS_AudioCodec_eSbc,NULL},                             /* Sub Band Codec used in Bluetooth A2DP audio */
    {NEXUS_AudioCodec_eDtsLegacy,NULL},                       /* Digital Digital Surround sound, legacy mode (14 bit), uses legacy frame-sync */
    {NEXUS_AudioCodec_eVorbis,NULL},                          /* Vorbis audio codec.  Typically used with OGG or WebM container formats. */
    {NEXUS_AudioCodec_eMax,NULL}
};
typedef struct B_DVR_MediaProbe_PayLoad
{
    bmpeg2ts_parser_pid ts; /* should be first */
    bmpeg2pes_parser pes;
    const bmedia_probe_es_desc *probe_desc;
    bmedia_probe_base_es_t probe;
    bmedia_probe_track *track;
    batom_accum_t pes_accum;
    bool active;
} B_DVR_MediaProbe_PayLoad;

typedef struct B_DVR_MediaProbe_PayLoadFeed
{
    batom_factory_t factory;
    batom_accum_t accum;
} B_DVR_MediaProbe_PayLoadFeed;

typedef struct B_DVR_MediaProbe
{
    BDBG_OBJECT(B_DVR_MediaProbe)
    B_DVR_MediaProbe_PayLoad *payLoad;
    B_DVR_MediaProbe_PayLoadFeed payLoadFeed;
    B_DVR_MediaNode mediaNode;
    NEXUS_FilePlayHandle nexusFileProbe;
    uint8_t *payLoadBuffer;
    ssize_t payLoadBufSize;
    unsigned probeCount;
    NEXUS_FileRecordHandle nexusFileRecord;
    B_MutexHandle mediaProbeMutex;
    B_Time startTime;
}B_DVR_MediaProbe;

static bmedia_probe_es_desc * B_DVR_MediaProbe_P_GetAudioProbe(NEXUS_AudioCodec audioCodec)
{
    unsigned index=0;
    bmedia_probe_es_desc *audioProbe=NULL;
    BDBG_MSG(("B_DVR_MediaProbe_P_GetAudioProbe numProbesAvail %u",sizeof(g_audioProbeMap)/sizeof(g_audioProbeMap[0])));
    for (index=0;index < sizeof(g_audioProbeMap)/sizeof(g_audioProbeMap[0]);index++)
    {
        if(g_audioProbeMap[index].audioCodec == audioCodec)
        {
            audioProbe = g_audioProbeMap[index].audioProbe;
            break;
        }
    }
    return audioProbe;
}

static bmedia_probe_es_desc * B_DVR_MediaProbe_P_GetVideoProbe(NEXUS_VideoCodec videoCodec)
{
    unsigned index=0;
    bmedia_probe_es_desc *videoProbe=NULL;
    BDBG_MSG(("B_DVR_MediaProbe_P_GetVideoProbe numProbesAvail %u",sizeof(g_videoProbeMap)/sizeof(g_videoProbeMap[0])));
    for (index=0;index < sizeof(g_videoProbeMap)/sizeof(g_videoProbeMap[0]);index++)
    {
        if(g_videoProbeMap[index].videoCodec == videoCodec)
        {
            videoProbe = g_videoProbeMap[index].videoProbe;
            break;
        }
    }
    return videoProbe;
}

static void
B_DVR_MediaProbe_PayLoadPrintTrack(B_DVR_MediaProbe_PayLoad *payLoad)
{
    BDBG_MSG(("B_DVR_MediaProbe_PayLoadPrintTrack >>>"));
    BDBG_ASSERT(payLoad);
    if(payLoad->track) {
        bmedia_probe_stream stream;
        char str[128];
        BKNI_Memset(&stream,0,sizeof(bmedia_probe_stream));
        bmedia_probe_stream_init(&stream, bstream_mpeg_type_es);
        bmedia_probe_add_track(&stream, payLoad->track);
        bmedia_stream_to_string(&stream, str, sizeof(str));
        BDBG_MSG(("pid %u track %#lx",payLoad->ts.pid,(unsigned long)payLoad->track));
        BDBG_LOG(("%s", str));
    }
    else
    {
        BDBG_MSG(("track is null for pid %u active %s",payLoad->ts.pid,payLoad->active?"true":"false"));
    }
    BDBG_MSG(("B_DVR_MediaProbe_PayLoadPrintTrack <<<"));
    return;
}

static void B_DVR_MediaProbe_Last(B_DVR_MediaProbe_PayLoad *payLoad)
{
    unsigned probability = 0;
    BDBG_ASSERT(payLoad);
    BDBG_MSG(("B_DVR_MediaProbe_Last  pid %u track %#lx >>>",payLoad->ts.pid,(unsigned long)payLoad->track));
    payLoad->track =  payLoad->probe_desc->last(payLoad->probe, &probability);
    B_DVR_MediaProbe_PayLoadPrintTrack(payLoad);
    BDBG_MSG(("B_DVR_MediaProbe_Last pid %u track %#lx <<<",payLoad->ts.pid,(unsigned long)payLoad->track));
    return;
}

static bmpeg2ts_parser_action
B_DVR_MediaProbe_TsData(bmpeg2ts_parser_pid *pid, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
    B_DVR_MediaProbe_PayLoad *payLoad = (B_DVR_MediaProbe_PayLoad*)pid;
    /*BDBG_MSG(("B_DVR_MediaProbe_TsData: %#lx pid:%#x %u bytes", payLoad, payLoad->ts.pid, len));*/
    if(payLoad->active) {
        return bmpeg2pes_parser_feed(&payLoad->pes, flags, src, cursor, len);
    }
    return bmpeg2ts_parser_action_skip;
}

static void
B_DVR_MediaProbe_PesData(void *packet_cnxt, batom_accum_t src, batom_cursor *payload, size_t len, const bmpeg2pes_atom_info *info)
{
    B_DVR_MediaProbe_PayLoad *payLoad = packet_cnxt;
    size_t accum_len;
    batom_t packet = NULL;
    batom_cursor payload_start;

   /* BDBG_MSG(("B_DVR_MediaProbe_PesData: %#lx stream %#x:%#x pes data %u:%u", (unsigned long)payLoad, (unsigned)payLoad->ts.pid, (unsigned)info->pes_id, info->data_offset, len));*/

    if(!payLoad->active || payLoad->track) {
        return;
    }
    accum_len = batom_accum_len(payLoad->pes_accum);
    if( accum_len>(8*1024) || (accum_len>0 && info->data_offset == 0)) {
        packet = batom_from_accum(payLoad->pes_accum, NULL, NULL);
    }
    BATOM_CLONE(&payload_start, payload);
    batom_cursor_skip(payload, len);
    batom_accum_append(payLoad->pes_accum, src, &payload_start, payload);
    if(packet) {
        bool done = false;
        payLoad->track = payLoad->probe_desc->feed(payLoad->probe, packet, &done);
        if(done||payLoad->track) {
            B_DVR_MediaProbe_PayLoadPrintTrack(payLoad);
            if(payLoad->track)
            {
                payLoad->active = false;
            }
            BDBG_MSG(("B_DVR_MediaProbe_PesData : done %s track %ld",done?"true":"false",(unsigned long)payLoad->track));
        }
        batom_release(packet);
    }
    return;
}

static void B_DVR_MediaProbe_P_PayLoadInit(B_DVR_MediaProbe_PayLoad *payLoad,const B_DVR_MediaProbe_PayLoadFeed *payLoadFeed,unsigned pid, const bmedia_probe_es_desc *probe_desc)
{

    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadInit >>>"));
    bmpeg2ts_parser_pid_init(&payLoad->ts, pid);
    bmpeg2pes_parser_init(payLoadFeed->factory,
                          &payLoad->pes,
                          BMPEG2PES_ID_ANY);
    payLoad->ts.payload = B_DVR_MediaProbe_TsData;
    payLoad->pes.packet = B_DVR_MediaProbe_PesData;
    payLoad->pes.packet_cnxt = payLoad;
    payLoad->probe_desc = probe_desc;
    payLoad->probe =  probe_desc->create(payLoadFeed->factory);
    BDBG_ASSERT(payLoad->probe);
    payLoad->pes_accum = batom_accum_create(payLoadFeed->factory);
    BDBG_ASSERT(payLoad->pes_accum);
    payLoad->active = true;
    payLoad->track = NULL;
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadInit <<<"));
    return;
}

static void B_DVR_MediaProbe_P_PayLoadUnInit(B_DVR_MediaProbe_PayLoad *payLoad)
{
    BDBG_MSG(("B_DVR_MediaProbe_PayLoadUnInit >>>"));
    bmpeg2pes_parser_shutdown(&payLoad->pes);
    payLoad->probe_desc->destroy(payLoad->probe);
    batom_accum_destroy(payLoad->pes_accum);
    BDBG_MSG(("B_DVR_MediaProbe_PayLoadUnInit <<<"));
    return;
}

static void B_DVR_MediaProbe_P_PayLoadReset(B_DVR_MediaProbe_PayLoad *payLoad)
{
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadReset >>>"));
    payLoad->probe_desc->reset(payLoad->probe);
    bmpeg2pes_parser_reset(&payLoad->pes);
    batom_accum_clear(payLoad->pes_accum);
    if(payLoad->track)
    {
        BKNI_Free(payLoad->track);
        payLoad->track= NULL;
    }
    payLoad->active = true;
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadReset <<<"));
    return;
}

static void B_DVR_MediaProbe_P_PayLoadFeedInit(B_DVR_MediaProbe_PayLoadFeed *payLoadFeed)
{
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeedInit >>>"));
    payLoadFeed->factory = batom_factory_create(bkni_alloc, 256);
    BDBG_ASSERT(payLoadFeed->factory);
    payLoadFeed->accum = batom_accum_create(payLoadFeed->factory);
    BDBG_ASSERT(payLoadFeed->accum);
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeedInit <<<"));
    return;
}

static void B_DVR_MediaProbe_P_PayLoadFeedUnInit(B_DVR_MediaProbe_PayLoadFeed *payLoadFeed)
{
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeedUninit >>>"));
    batom_accum_destroy(payLoadFeed->accum);
    batom_factory_destroy(payLoadFeed->factory);
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeedUninit <<<"));
    return;
}

static void
b_atom_free_media(batom_t atom, void *user)
{
    void *block = *(void **)user;
    BDBG_MSG(("free_media %p(%p)", block, atom));
    BSTD_UNUSED(atom);
    BKNI_Free(block);
    return;
}

static const batom_user b_atom_media = {
    b_atom_free_media,
    sizeof(void *)
};

static bool B_DVR_MediaProbe_P_Succeeded(B_DVR_MediaProbeHandle mediaProbe)
{
    unsigned i=0;
    bool probeSuccess=false;
    BDBG_MSG(("B_DVR_MediaProbe_P_Succeeded >>>"));
    BDBG_OBJECT_ASSERT(mediaProbe,B_DVR_MediaProbe);
    for(i=0;i<mediaProbe->probeCount;i++)
    {
        if(mediaProbe->payLoad[i].active)
        {
           BDBG_MSG(("pid %u still active, npackets %d",mediaProbe->payLoad[i].ts.pid, mediaProbe->payLoad[i].ts.npackets));
           break;
        }
    }
    if(i==mediaProbe->probeCount)
    {
        probeSuccess=true;
        BDBG_MSG(("all pids parsed"));
    }
    BDBG_MSG(("B_DVR_MediaProbe_P_Succeeded probeSuccess:%s <<<",probeSuccess?"true":"false"));
    return probeSuccess;
}

static void B_DVR_MediaProbe_P_PayLoadFeed(B_DVR_MediaProbeHandle mediaProbe,
                                         unsigned nprobes,
                                         const void *data,
                                         size_t data_len)
{
    batom_t atom;
    void *buf;
    unsigned i, pid;
    batom_cursor cursor;
    BDBG_OBJECT_ASSERT(mediaProbe,B_DVR_MediaProbe);
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeed nprobe %u buffer size %u >>>",nprobes,data_len));
    for(i=0;i<nprobes;i++) {
        if(mediaProbe->payLoad[i].active) {
            break;
        }
    }
    if(i>=nprobes) {
        return; /* nothing to do */
    }

    buf = BKNI_Malloc(data_len);
    BDBG_ASSERT(buf);
    BKNI_Memcpy(buf, data, data_len);
    atom = batom_from_range(mediaProbe->payLoadFeed.factory, buf, data_len, &b_atom_media, &buf);
    BDBG_ASSERT(atom);
    batom_accum_add_atom(mediaProbe->payLoadFeed.accum, atom);
    batom_release(atom);

    batom_cursor_from_accum(&cursor, mediaProbe->payLoadFeed.accum);
    for(pid=i;;)
    {
        int rc;
        if(!mediaProbe->payLoad[pid].active)
        {
            for(i=0;i<nprobes;i++)
            {
                if(mediaProbe->payLoad[i].active)
                {
                    BDBG_MSG(("active pid %u probe index %u, npackets %d",mediaProbe->payLoad[i].ts.pid,i,mediaProbe->payLoad[i].ts.npackets));
                    break;
                }
            }
            if(i>=nprobes)
            {
                BDBG_MSG(("all the pids' info extracted"));
                break; /* nothing to do */
            }
            pid = i;
        }
        rc = bmpeg2ts_parser_pid_feed(&mediaProbe->payLoad[pid].ts, mediaProbe->payLoadFeed.accum, &cursor);
        if(mediaProbe->payLoad[pid].track)
        {
            BDBG_MSG(("track %p for pid %#x", mediaProbe->payLoad[pid].track, mediaProbe->payLoad[pid].ts.pid));
        }
        if(rc<0)
        {
            BDBG_MSG(("rc %d from bmpeg2ts_parser_pid_feed",rc));
            break;
        }

        batom_cursor_skip(&cursor, BMPEG2TS_PKT_LEN);
    }
    batom_accum_clear(mediaProbe->payLoadFeed.accum);
    BDBG_MSG(("B_DVR_MediaProbe_P_PayLoadFeed <<<"));
    return;
}

B_DVR_MediaProbeHandle B_DVR_MediaProbe_Open(B_DVR_MediaNode mediaNode,
                                             NEXUS_FileRecordHandle nexusFileRecord,
                                             unsigned volumeIndex,
                                             unsigned serviceIndex)
{
    unsigned index;
    B_DVR_SegmentedFileSettings segmentedFileSettings;
    bmedia_probe_es_desc *videoProbe=NULL;
    bmedia_probe_es_desc *audioProbe=NULL;
    B_DVR_MediaProbeHandle mediaProbe=NULL;
    NEXUS_MemoryAllocationSettings memorySettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();

    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    BDBG_MSG(("B_DVR_MediaProbe_Open >>>"));

    mediaProbe = BKNI_Malloc(sizeof(B_DVR_MediaProbe));
    if(!mediaProbe)
    {
        BDBG_ERR(("mediaProbe alloc failure"));
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaProbe),
                                            true, __FUNCTION__,__LINE__);
    BKNI_Memset((void *)mediaProbe,0,sizeof(B_DVR_MediaProbe));
    BDBG_OBJECT_SET(mediaProbe,B_DVR_MediaProbe);
    mediaProbe->mediaProbeMutex = B_Mutex_Create(NULL);
    if(!mediaProbe->mediaProbeMutex)
    {
        BDBG_ERR(("media Probe mutex create failed"));
        goto error_mutex;
    }

    mediaProbe->mediaNode = mediaNode;
    mediaProbe->payLoadBufSize = B_DVR_MEDIA_PROBE_CHUNK_SIZE;
    mediaProbe->payLoad = BKNI_Malloc(sizeof(B_DVR_MediaProbe_PayLoad)*(mediaNode->esStreamCount));
    if(!mediaProbe->payLoad)
    {
        BDBG_ERR(("payLoad probe alloc failure"));
        goto error_payLoad;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaProbe->payLoad),
                                            true, __FUNCTION__,__LINE__);
    BKNI_Memset((void*)mediaProbe->payLoad,0,sizeof(B_DVR_MediaProbe_PayLoad)*(mediaNode->esStreamCount));
    memorySettings.alignment = B_DVR_IO_BLOCK_SIZE;
    memorySettings.heap = NULL;
    NEXUS_Memory_Allocate(mediaProbe->payLoadBufSize,&memorySettings, (void *)&mediaProbe->payLoadBuffer);
    if(!mediaProbe->payLoadBuffer)
    {
        BDBG_ERR(("payLoadBuffer alloc failure"));
        goto error_payLoadBuffer;
    }
    B_DVR_MediaProbe_P_PayLoadFeedInit(&mediaProbe->payLoadFeed);

    for(index=0;index<mediaNode->esStreamCount;index++)
    {
        if(mediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeVideo)
        {
            videoProbe = B_DVR_MediaProbe_P_GetVideoProbe(mediaNode->esStreamInfo[index].codec.videoCodec);
            if(videoProbe)
            {
                BDBG_MSG(("video pid %u added for media probing",mediaNode->esStreamInfo[index].pid));
                B_DVR_MediaProbe_P_PayLoadInit(&mediaProbe->payLoad[mediaProbe->probeCount],
                                               &mediaProbe->payLoadFeed,
                                               mediaNode->esStreamInfo[index].pid,
                                               videoProbe);
                mediaProbe->probeCount++;
            }
        }

        if(mediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeAudio)
        {
            audioProbe = B_DVR_MediaProbe_P_GetAudioProbe(mediaNode->esStreamInfo[index].codec.audioCodec);
            if(audioProbe)
            {
                BDBG_MSG(("audio pid %u added for media probing",mediaNode->esStreamInfo[index].pid));
                B_DVR_MediaProbe_P_PayLoadInit(&mediaProbe->payLoad[mediaProbe->probeCount],
                                               &mediaProbe->payLoadFeed,
                                               mediaNode->esStreamInfo[index].pid,
                                               audioProbe);
                mediaProbe->probeCount++;
            }
        }
    }

    segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
    segmentedFileSettings.maxSegmentCount = 0;
    segmentedFileSettings.registeredCallback = NULL;
    segmentedFileSettings.volumeIndex = volumeIndex;
    segmentedFileSettings.mediaSegmentSize = 0;
    segmentedFileSettings.serviceIndex = serviceIndex;
    if(mediaNode->recording == eB_DVR_RecordingTSB)
    {
        segmentedFileSettings.service = eB_DVR_ServiceTSB;
    }
    else
    {
        segmentedFileSettings.service = eB_DVR_ServiceRecord;
    }
    segmentedFileSettings.event =  NULL;
    segmentedFileSettings.itbThreshhold = 0;
    if(mediaNode->mediaNodeSubDir[0]=='\0')
    {
        segmentedFileSettings.metaDataSubDir = NULL;
    }
    else
    {
        segmentedFileSettings.metaDataSubDir = (char *)mediaNode->mediaNodeSubDir;
    }
    mediaProbe->nexusFileProbe = B_DVR_SegmentedFilePlay_Open(mediaNode->mediaFileName,
                                                              mediaNode->navFileName,
                                                              &segmentedFileSettings,
                                                              nexusFileRecord);
    if(mediaProbe->nexusFileProbe == NULL)
    {
        BDBG_ERR(("nexusFileProbe open failed %s",mediaNode->mediaFileName));
        goto error_nexusFileProbe;
    }

    B_Time_Get(&mediaProbe->startTime);

    BDBG_MSG(("number of esStreams to be probed %u",mediaProbe->probeCount));
    BDBG_MSG(("Program to be probed %s media %s nav %s",
              mediaNode->programName,mediaNode->mediaFileName,mediaNode->navFileName));
    BDBG_MSG(("B_DVR_MediaProbe_Open <<<"));
    return mediaProbe;
error_nexusFileProbe:
     NEXUS_Memory_Free(mediaProbe->payLoadBuffer);
error_payLoadBuffer:
     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                             sizeof(*mediaProbe->payLoad),
                                             false, __FUNCTION__,__LINE__);
     BKNI_Free(mediaProbe->payLoad);
error_payLoad:
    B_Mutex_Destroy(mediaProbe->mediaProbeMutex);
error_mutex:
    BDBG_OBJECT_DESTROY(mediaProbe,B_DVR_MediaProbe);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaProbe),
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(mediaProbe);
error_alloc:
    return NULL;
}

bool B_DVR_MediaProbe_Parse(B_DVR_MediaProbeHandle mediaProbe,
                            B_DVR_DRMServiceHandle drmService,
                            off_t *probeStartOffset)
{

    NEXUS_FilePlayHandle segmentedFile;
    unsigned i=0;
    ssize_t returnSize=0;
    off_t returnOffset;
    B_DVR_DRMServiceStreamBufferInfo drmBufInfo;
    bool probeSuccess=false;
    off_t probeCurrentOffset;
    BDBG_OBJECT_ASSERT(mediaProbe,B_DVR_MediaProbe);
    BDBG_OBJECT_ASSERT(mediaProbe->mediaNode,B_DVR_Media);
    if(drmService)
    {
        BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    }
    BDBG_MSG(("B_DVR_MediaProbe_Parse %s %lld >>>",mediaProbe->mediaNode->programName,*probeStartOffset));
    B_Mutex_Lock(mediaProbe->mediaProbeMutex);
    segmentedFile = mediaProbe->nexusFileProbe;
    probeCurrentOffset = *probeStartOffset;
    do{
        returnOffset = (segmentedFile->file.data)->seek(segmentedFile->file.data,probeCurrentOffset,SEEK_SET);
        if(returnOffset!=probeCurrentOffset)
        {
            BDBG_MSG(("B_DVR_MediaProbe_Parse seek failed"));
            break;
        }
        returnSize = (segmentedFile->file.data)->read(segmentedFile->file.data,mediaProbe->payLoadBuffer,mediaProbe->payLoadBufSize);

        if(returnSize < mediaProbe->payLoadBufSize)
        {
            BDBG_MSG(("desired readSize %u actual readSize %u",mediaProbe->payLoadBufSize,returnSize));
            returnSize -=(returnSize%BMPEG2TS_PKT_LEN);
        }

        drmBufInfo.streamBuf = mediaProbe->payLoadBuffer;
        drmBufInfo.streamBufLen = returnSize;
        if(drmService)
        {
            B_DVR_DRMService_DecryptData(drmService,&drmBufInfo);
        }
        B_DVR_MediaProbe_P_PayLoadFeed(mediaProbe,mediaProbe->probeCount,mediaProbe->payLoadBuffer,returnSize);

        probeSuccess = B_DVR_MediaProbe_P_Succeeded(mediaProbe);
        if(probeSuccess)
        {
            break;
        }
        probeCurrentOffset += mediaProbe->payLoadBufSize;
        BDBG_MSG(("probeCurrentOffset %lld readSize %u",probeCurrentOffset,returnSize));
    }while(probeCurrentOffset < mediaProbe->mediaNode->mediaLinearEndOffset);

    if(probeCurrentOffset >= mediaProbe->payLoadBufSize)
    {
        probeCurrentOffset -= mediaProbe->payLoadBufSize;
    }

    if(!probeSuccess)
    {
        BDBG_MSG(("Attempt B_DVR_MediaProbe_Last"));
        for(i=0;i<mediaProbe->probeCount;i++)
        {
            if(mediaProbe->payLoad[i].active)
            {
                B_DVR_MediaProbe_Last(&mediaProbe->payLoad[i]);
                if(mediaProbe->payLoad[i].track)
                {
                    mediaProbe->payLoad[i].active = false;
                    BDBG_MSG(("Using B_DVR_MediaProbe_Last, track info extracted for index %u pid %u track %ld active %s",
                              i,mediaProbe->payLoad[i].ts.pid,(unsigned long)mediaProbe->payLoad[i].track,mediaProbe->payLoad[i].active?"true":"false"));
                }
                else if(mediaProbe->payLoad[i].ts.npackets == 0)
                {
                    // If no packet is comming in 30 secs or probing didn't finish after certain number of bytes, stop it
                    B_Time current;
                    B_Time_Get(&current);
                    if( (B_Time_Diff(&current, &mediaProbe->startTime) > B_DVR_MEDIA_PROBE_TIMEOUT) ||
                        ((probeCurrentOffset - mediaProbe->mediaNode->mediaLinearStartOffset) > B_DVR_MEDIA_SEGMENT_SIZE))
                    {
                        mediaProbe->payLoad[i].active = false;
                        BDBG_WRN(("Stop probing. index %u pid %u track %ld active %s. No TS packets in %d secs",
                              i,mediaProbe->payLoad[i].ts.pid,(unsigned long)mediaProbe->payLoad[i].track,mediaProbe->payLoad[i].active?"true":"false", B_DVR_MEDIA_PROBE_TIMEOUT/1000));
                    }
                }
            }

        }
        probeSuccess = B_DVR_MediaProbe_P_Succeeded(mediaProbe);
    }
    else
    {
        for(i=0;i<mediaProbe->probeCount;i++)
        {
            BDBG_MSG(("pid %u active %s track %#lx",
                      mediaProbe->payLoad[i].ts.pid,
                      mediaProbe->payLoad[i].active?"true":"false",
                      mediaProbe->payLoad[i].track));
        }
    }

    *probeStartOffset=probeCurrentOffset;
    if(probeSuccess)
    {
        for(i=0;i<mediaProbe->probeCount;i++)
        {
            unsigned j=0;
            for (j=0;j<mediaProbe->mediaNode->esStreamCount;j++)
            {
                if(!mediaProbe->payLoad[i].active &&
                   mediaProbe->payLoad[i].ts.pid == mediaProbe->mediaNode->esStreamInfo[j].pid)
                {
                    if(!mediaProbe->payLoad[i].track)
                    {
                        BDBG_WRN(("!!! track == NULL !!!pid %u active %s ",mediaProbe->payLoad[i].ts.pid,
                                  mediaProbe->payLoad[i].active?"true":"false"));
                        continue;
                    }
                    if(mediaProbe->mediaNode->esStreamInfo[j].pidType == eB_DVR_PidTypeAudio)
                    {
                        BDBG_ASSERT(mediaProbe->payLoad[i].track);
                        mediaProbe->mediaNode->esStreamInfo[j].audioChannelCount =
                            mediaProbe->payLoad[i].track->info.audio.channel_count;

                        mediaProbe->mediaNode->esStreamInfo[j].audioSampleRate =
                            mediaProbe->payLoad[i].track->info.audio.sample_rate;

                        mediaProbe->mediaNode->esStreamInfo[j].audioSampleSize =
                            mediaProbe->payLoad[i].track->info.audio.sample_size;

                        mediaProbe->mediaNode->esStreamInfo[j].bitRate =
                            mediaProbe->payLoad[i].track->info.audio.bitrate*1000;

                        BDBG_MSG(("Audio PID %u",mediaProbe->mediaNode->esStreamInfo[j].pid));
                        BDBG_MSG(("ChannelCount %u",mediaProbe->mediaNode->esStreamInfo[j].audioChannelCount));
                        BDBG_MSG(("SampleRate %u",mediaProbe->mediaNode->esStreamInfo[j].audioSampleRate));
                        BDBG_MSG(("SampleSize %u",mediaProbe->mediaNode->esStreamInfo[j].audioSampleSize));
                        BDBG_MSG(("BitRate %u",mediaProbe->mediaNode->esStreamInfo[j].bitRate));
                    }

                    if(mediaProbe->mediaNode->esStreamInfo[j].pidType == eB_DVR_PidTypeVideo)
                    {
                        BDBG_ASSERT(mediaProbe->payLoad[i].track);
                        mediaProbe->mediaNode->esStreamInfo[j].videoHeight =
                            mediaProbe->payLoad[i].track->info.video.height;

                        mediaProbe->mediaNode->esStreamInfo[j].videoWidth =
                            mediaProbe->payLoad[i].track->info.video.width;

                        mediaProbe->mediaNode->esStreamInfo[j].bitRate =
                            mediaProbe->payLoad[i].track->info.video.bitrate*1000;

                        if(mediaProbe->mediaNode->esStreamInfo[j].codec.videoCodec == NEXUS_VideoCodec_eMpeg2)
                        {
                            mediaProbe->mediaNode->esStreamInfo[j].videoFrameRate =
                                ((bmedia_probe_mpeg_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->framerate;
                        }
                        BDBG_MSG(("Video PID %u",mediaProbe->mediaNode->esStreamInfo[j].pid));
                        BDBG_MSG(("Height %u",mediaProbe->mediaNode->esStreamInfo[j].videoHeight));
                        BDBG_MSG(("Width %u",mediaProbe->mediaNode->esStreamInfo[j].videoWidth));
                        BDBG_MSG(("frameRate %u",mediaProbe->mediaNode->esStreamInfo[j].videoFrameRate));
                        BDBG_MSG(("BitRate %u",mediaProbe->mediaNode->esStreamInfo[j].bitRate));
                        if(mediaProbe->mediaNode->esStreamInfo[j].videoHeight >= 720)
                        {
                            mediaProbe->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_HD_STREAM;
                        }
                        else
                        {
                            mediaProbe->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_HD_STREAM);
                        }
                        if(mediaProbe->mediaNode->esStreamInfo[j].codec.videoCodec == NEXUS_VideoCodec_eMpeg2)
                        {
                            mediaProbe->mediaNode->esStreamInfo[j].profile =
                                ((bmedia_probe_mpeg_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->profile;
                            mediaProbe->mediaNode->esStreamInfo[j].level =
                                ((bmedia_probe_mpeg_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->level;
                            if(((bmedia_probe_mpeg_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->progressive_sequence)
                            {
                                mediaProbe->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE;
                                BDBG_MSG(("MPEG2 stream progressive"));
                            }
                            else
                            {
                                mediaProbe->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE);
                                BDBG_MSG(("MPEG2 stream interlaced"));
                            }
                            if(((bmedia_probe_mpeg_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->low_delay)
                            {
                                mediaProbe->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_LOW_DELAY_STREAM;
                                BDBG_WRN(("MPEG2 low delay stream"));
                            }
                            else
                            {
                                mediaProbe->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_LOW_DELAY_STREAM);
                                BDBG_MSG(("MPEG2 non low delay stream"));
                            }
                            BDBG_MSG(("MPEG2 profile %u",mediaProbe->mediaNode->esStreamInfo[j].profile));
                            BDBG_MSG(("MPEG2 level %u",mediaProbe->mediaNode->esStreamInfo[j].level));
                        }

                        if(mediaProbe->mediaNode->esStreamInfo[j].codec.videoCodec == NEXUS_VideoCodec_eH264)
                        {
                            mediaProbe->mediaNode->esStreamInfo[j].profile =
                                ((bmedia_probe_h264_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->sps.profile;
                            mediaProbe->mediaNode->esStreamInfo[j].level =
                                ((bmedia_probe_h264_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->sps.level;
                            if(((bmedia_probe_h264_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->frame_mbs_only)
                            {
                                mediaProbe->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE;
                                BDBG_MSG(("H264 Stream: progressive picture scan type"));
                            }
                            else
                            {
                                mediaProbe->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE);
                                BDBG_MSG(("H264 Stream: interlaced picture scan type"));
                            }
                            BDBG_MSG(("H264 profile %u",mediaProbe->mediaNode->esStreamInfo[j].profile));
                            BDBG_MSG(("H264 level %u",mediaProbe->mediaNode->esStreamInfo[j].level));
                        }
                        else
                        {
                            if(mediaProbe->mediaNode->esStreamInfo[j].codec.videoCodec == NEXUS_VideoCodec_eH265)
                            {

                                mediaProbe->mediaNode->esStreamInfo[j].profile =
                                ((bmedia_probe_h265_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->sps.general_profile_idc;
                            mediaProbe->mediaNode->esStreamInfo[j].level =
                                ((bmedia_probe_h265_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->sps.general_level_idc;
                            if(((bmedia_probe_h265_video*)(&mediaProbe->payLoad[i].track->info.video.codec_specific))->frame_mbs_only)
                            {
                                mediaProbe->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE;
                                BDBG_MSG(("H265 Stream: progressive picture scan type"));
                            }
                            else
                            {
                                mediaProbe->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE);
                                BDBG_MSG(("H265 Stream: interlaced picture scan type"));
                            }
                            BDBG_MSG(("H265 profile %u",mediaProbe->mediaNode->esStreamInfo[j].profile));
                            BDBG_MSG(("H265 level %u",mediaProbe->mediaNode->esStreamInfo[j].level));

                            }
                        }

                    }

                }
            }
        }
        for(i=0;i<mediaProbe->probeCount;i++)
        {
            B_DVR_MediaProbe_P_PayLoadReset(&mediaProbe->payLoad[i]);
        }
    }
    BDBG_MSG(("B_DVR_MediaProbe_Parse <<<"));
    B_Mutex_Unlock(mediaProbe->mediaProbeMutex);
    return probeSuccess;
}

B_DVR_ERROR B_DVR_MediaProbe_Close(B_DVR_MediaProbeHandle mediaProbe)
{
    unsigned index;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaProbe,B_DVR_MediaProbe);
    BDBG_MSG(("B_DVR_MediaProbe_Close >>>"));
    B_Mutex_Lock(mediaProbe->mediaProbeMutex);

    for(index=0;index<mediaProbe->probeCount;index++)
    {
        B_DVR_MediaProbe_P_PayLoadReset(&mediaProbe->payLoad[index]);
    }

    for(index=0;index<mediaProbe->probeCount;index++)
    {
        B_DVR_MediaProbe_P_PayLoadUnInit(&mediaProbe->payLoad[index]);
    }

    B_DVR_MediaProbe_P_PayLoadFeedUnInit(&mediaProbe->payLoadFeed);
    B_DVR_SegmentedFilePlay_Close(mediaProbe->nexusFileProbe);
    NEXUS_Memory_Free(mediaProbe->payLoadBuffer);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaProbe->payLoad),
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(mediaProbe->payLoad);
    B_Mutex_Unlock(mediaProbe->mediaProbeMutex);
    B_Mutex_Destroy(mediaProbe->mediaProbeMutex);
    BDBG_OBJECT_DESTROY(mediaProbe,B_DVR_MediaProbe);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaProbe), false,
                                            __FUNCTION__,__LINE__);
    BKNI_Free(mediaProbe);
    BDBG_MSG(("B_DVR_MediaProbe_Close <<<"));
    return rc;
}

