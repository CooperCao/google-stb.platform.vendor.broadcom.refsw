/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#if NEXUS_HAS_TRANSPORT
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "tspsimgr3.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "nexus_message.h"

BDBG_MODULE(tspsimgr);

static void tsPsi_parsePMT2( const void *pmt, unsigned pmtSize, tspsimgr_program *p_programInfo);
static void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, tspsimgr_program *progInfo );
static unsigned tsPsi_procStreamDescriptors_getCaPid( const uint8_t *p_bfr, unsigned bfrSize, int streamNum);

#define MAX_MSG (TSPSIMGR_MAX_PROGRAMS+1)
struct tspsimgr
{
    bool started;
    tspsimgr_scan_settings settings;
    tspsimgr_scan_results results;
    BKNI_MutexHandle mutex; /* protects results */
    unsigned pmt_count; /* no internal timeout. app can have its own timeout. */
    NEXUS_PidChannelHandle pidChannel[MAX_MSG];
    NEXUS_MessageHandle msg[MAX_MSG];
    unsigned pid[MAX_MSG];
    bool scan_done;
};

/* continuous PSI scan requires filtering for the version number so you don't process repeat messages */
#define CONTINUOUS 0

static NEXUS_PidChannelHandle message_p_openpid(const tspsimgr_scan_settings *psettings, unsigned pid)
{
#if NEXUS_HAS_TRANSPORT
    if (psettings->playpump) {
        return NEXUS_Playpump_OpenPidChannel(psettings->playpump, pid, NULL);
    }
    else
#endif
    {
        return NEXUS_PidChannel_Open(psettings->parserBand, pid, NULL);
    }
}

static void message_callback(void *context, int index)
{
    tspsimgr_t handle = context;
    NEXUS_Error rc;
    const void *buffer;
    size_t size;
    NEXUS_MessageSettings openSettings;
    NEXUS_MessageStartSettings startSettings;

    if (!handle->msg[index]) {
        BDBG_ERR(("tspsimgr %p: NULL message %d", (void*)handle, index));
        return;
    }
    
    rc = NEXUS_Message_GetBuffer(handle->msg[index], &buffer, &size);
    if (rc || !size) {
        rc = BERR_TRACE(rc);
        return;
    }

    if (index == 0) {
        /* PAT */
        if (TS_PAT_validate(buffer, size)) {
            TS_PAT_program program;
            unsigned i;
            TS_PSI_header header;
            
            TS_PSI_getSectionHeader(buffer, &header);
            BKNI_AcquireMutex(handle->mutex);
            handle->results.version                 = header.version_number;
            handle->results.transport_stream_id     = header.table_id_extension;
            handle->results.num_programs            = 0;
            BKNI_ReleaseMutex(handle->mutex);
            
            handle->pmt_count = TS_PAT_getNumPrograms(buffer);
            BDBG_MSG(("received PAT with %d programs", handle->pmt_count));
            
            for (i=0;i<handle->pmt_count;i++) {
                rc = TS_PAT_getProgram(buffer, size, i,  &program);
                if (!rc) {
                    unsigned msgindex;
                    /* launch filter for PMT. find an open slot first. */
                    for (msgindex = 0; msgindex < MAX_MSG; msgindex++) {
                        if (!handle->pidChannel[msgindex]) break;
                    }
                    if (msgindex == MAX_MSG) {
                        BDBG_WRN(("too many programs. increase MAX_MSG %d", MAX_MSG));
                        handle->pmt_count--;
                        continue;
                    }
                    
                    handle->pid[msgindex] = program.PID;
                    NEXUS_Message_GetDefaultSettings(&openSettings);
                    openSettings.dataReady.callback = message_callback;
                    openSettings.dataReady.context = handle;
                    openSettings.dataReady.param = msgindex;
                    handle->msg[msgindex] = NEXUS_Message_Open(&openSettings);
                    if (!handle->msg[msgindex]) {
                        BDBG_ERR(("unable to open message"));
                        handle->pmt_count--;
                        continue;
                    }
                    
                    handle->pidChannel[msgindex] = message_p_openpid(&handle->settings, program.PID);
                    if (!handle->pidChannel[msgindex]) {
                        BDBG_ERR(("unable to open pid channel %#x", program.PID));
                        NEXUS_Message_Close(handle->msg[msgindex]);
                        handle->msg[msgindex] = NULL;
                        handle->pmt_count--;
                        continue;
                    }
                    
                    BDBG_MSG(("filter PMT %d on pid %#x with slot %d", program.program_number, program.PID, msgindex));
                    NEXUS_Message_GetDefaultStartSettings(handle->msg[msgindex], &startSettings);
                    startSettings.pidChannel             = handle->pidChannel[msgindex];
                    startSettings.filter.mask[0]         = 0x0;
                    startSettings.filter.mask[2]         = 0x0;
                    startSettings.filter.mask[15]        = 0xFF;
                    startSettings.filter.coefficient[0]  = 0x02;
                    startSettings.filter.coefficient[2]  = (program.program_number & 0xFF00) >> 8;
                    startSettings.filter.coefficient[15] = 0xFF;
                    rc = NEXUS_Message_Start(handle->msg[msgindex], &startSettings);
                    if (rc) {
                        BDBG_ERR(("unable to start message pid on %#x", program.PID));
                        NEXUS_Message_Close(handle->msg[msgindex]);
                        handle->msg[msgindex] = NULL;
                        NEXUS_PidChannel_Close(handle->pidChannel[msgindex]);
                        handle->pidChannel[msgindex] = NULL;
                        handle->pmt_count--;
                        continue;
                    }
                    
                    /* we are filtering */
                }
                else {
                    handle->pmt_count--;
                }
            }
        }
#if !CONTINUOUS
        NEXUS_Message_Stop(handle->msg[index]);
#endif
    }
    else {
        /* PMT */
        BDBG_MSG(("PMT: %#x size:%d (%02x %02x %02x)", handle->pid[index], (unsigned)size,
            ((unsigned char *)buffer)[0],((unsigned char *)buffer)[1],((unsigned char *)buffer)[2]));
        if (TS_PMT_validate(buffer, size)) {
            tspsimgr_scan_results *p_chanInfo = &handle->results;
            tspsimgr_program programInfo;
            unsigned i;
        
            BKNI_Memset(&programInfo, 0, sizeof(programInfo));
        
            if (p_chanInfo->num_programs < TSPSIMGR_MAX_PROGRAMS) {
                bool skip = false;
                BKNI_AcquireMutex(handle->mutex);
                tsPsi_parsePMT2(buffer, size, &programInfo);

                if (handle->settings.exclude_no_video_audio) {
                    if (!programInfo.num_video_pids && !programInfo.num_audio_pids) {
                        skip = true;
                    }
                }
                if (!skip && handle->settings.exclude_ca_pid) {
                    if (programInfo.ca_pid) skip = true;
                    for (i=0;!skip && i<programInfo.num_video_pids;i++) {
                        if (programInfo.video_pids[i].ca_pid) skip = true;
                    }
                    for (i=0;!skip && i<programInfo.num_audio_pids;i++) {
                        if (programInfo.audio_pids[i].ca_pid) skip = true;
                    }
                    for (i=0;!skip && i<programInfo.num_other_pids;i++) {
                        if (programInfo.other_pids[i].ca_pid) skip = true;
                    }
                }
                if (!skip) {
                    programInfo.map_pid = handle->pid[index];

                    /* now that we know the program_number, insert it into the array */
                    for (i=0;i<p_chanInfo->num_programs;i++) {
                        if (programInfo.program_number < p_chanInfo->program_info[i].program_number)
                            break;
                    }
                    /* make room for an insertion */
                    if (i < p_chanInfo->num_programs) {
                        unsigned char *ptr = (unsigned char *)&p_chanInfo->program_info[i];
                        BKNI_Memmove(ptr + sizeof(tspsimgr_program), ptr, sizeof(tspsimgr_program) * (p_chanInfo->num_programs - i));
                    }
                    /* now copy into place */
                    BKNI_Memcpy(&p_chanInfo->program_info[i], &programInfo, sizeof(tspsimgr_program));
                    p_chanInfo->num_programs++;
                }
                BKNI_ReleaseMutex(handle->mutex);
            }
            else {
                BDBG_WRN(("PMT overflow"));
            }
        }
        else {
            BDBG_ERR(("invalid PMT %#x", handle->pid[index]));
        }
#if !CONTINUOUS
        NEXUS_Message_Stop(handle->msg[index]);
#endif
        if (handle->pmt_count && --handle->pmt_count == 0) {
            handle->scan_done = true;
            if (handle->settings.scan_done) {
                (handle->settings.scan_done)(handle->settings.context);
            }
        }
    }
    
#if CONTINUOUS
    rc = NEXUS_Message_ReadComplete(handle->msg[index], size);
    if (rc) BERR_TRACE(rc);
#endif
}

#define ADD_VIDEO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_video_pids < TSPSIMGR_MAX_PIDS ) \
    { \
    BDBG_MSG(("  vpid[%d] 0x%x, type 0x%x", (P_INFO)->num_video_pids, (PID), (TYPE))); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].pid = (PID); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].codec = (TYPE); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].ca_pid = tsPsi_procStreamDescriptors_getCaPid((PMT), (PMTSIZE), (INDEX)); \
    (P_INFO)->num_video_pids++; \
    } \
    } while (0)
#define ADD_AUDIO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_audio_pids < TSPSIMGR_MAX_PIDS ) \
    { \
    BDBG_MSG(("  apid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].pid = (PID); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].codec = (TYPE); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].ca_pid = tsPsi_procStreamDescriptors_getCaPid((PMT), (PMTSIZE), (INDEX)); \
    (P_INFO)->num_audio_pids++; \
    } \
    } while (0)
#define ADD_OTHER_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_other_pids < TSPSIMGR_MAX_PIDS ) \
    { \
    BDBG_MSG(("  opid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].pid = (PID); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].stream_type = (TYPE); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].ca_pid = tsPsi_procStreamDescriptors_getCaPid((PMT), (PMTSIZE), (INDEX)); \
    (P_INFO)->num_other_pids++; \
    } \
    } while (0)


static void tsPsi_parsePMT2( const void *pmt, unsigned pmtSize, tspsimgr_program *p_programInfo)
{
    int i;
    TS_PMT_stream pmt_stream;
    TS_PSI_header header;

    TS_PSI_getSectionHeader(pmt, &header );

    /* Store the main information about the program */
    p_programInfo->program_number   = header.table_id_extension;
    p_programInfo->version          = header.version_number;
    p_programInfo->pcr_pid          = TS_PMT_getPcrPid(pmt, pmtSize);

    /* find and process Program descriptors */
    tsPsi_procProgDescriptors(pmt, pmtSize, p_programInfo );

    /* Find the video and audio pids... */
    p_programInfo->num_video_pids   = 0;
    p_programInfo->num_audio_pids   = 0;
    p_programInfo->num_other_pids   = 0;

    for( i = 0; i < TS_PMT_getNumStreams(pmt, pmtSize); i++ )
    {
        int descIdx = 0;
        if (TS_PMT_getStream(pmt, pmtSize, i, &pmt_stream )) {
            BDBG_WRN(("Invalid PMT data detected"));
            continue;
        }

        switch( pmt_stream.stream_type )
        {
        /* video formats */
        case TS_PSI_ST_11172_2_Video:  /* MPEG-1 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg1, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_Video:   /* ATSC MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_13818_2_Video: /* MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_2_Video: /* MPEG-4 Part 2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg4Part2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_Video: /* H.264/AVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_AnnexG_Video: /* H.264/SVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264_Svc, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_AnnexH_Video: /* H.264/MVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264_Mvc, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_23008_2_Video: /* H.265/HEVC */
        case TS_PSI_ST_23008_2_Video_brcm: /*deprecated */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH265, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Video: /* AVS */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eAvs, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_SMPTE_VC1:      /* VC-1 */
            /* need to parse descriptor and then subdescriptor to determine profile */
            for (;;) {
                TS_PSI_descriptor desc = TS_PMT_getStreamDescriptor(pmt, pmtSize, i, descIdx);
                if (desc == NULL) break;
                descIdx++;

                switch(desc[0]) {
                case TS_PSI_DT_Registration:
                    /* calculate and check format_identifier */
                    {
                    uint32_t format_identifier = (desc[2] << 24) + (desc[3] << 16) + (desc[4] << 8) + desc[5];
                    if (format_identifier == 0x56432D31) {
                        /* check that proper sub-descriptor exists */
                        int subdescriptor_tag = desc[6];
                        if (subdescriptor_tag == 0x01) {
                            int profile_level = desc[7];
                            if (profile_level >= 0x90)  /* Advanced Profile ES */
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eVc1, pmt, pmtSize, i);
                            else /* Simple/Main Profile ES */
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eVc1SimpleMain, pmt, pmtSize, i);
                        }
                    }
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        /* audio formats */
        case TS_PSI_ST_11172_3_Audio: /* MPEG-1 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_3_Audio: /* MPEG-2 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_7_AAC:  /* MPEG-2 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAac, pmt, pmtSize, i);    /* Note baudio_format_aac = MPEG-2 AAC */
            break;
        case TS_PSI_ST_14496_3_Audio: /* MPEG-4 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i);   /* Note baudio_format_aac_plus = MPEG-4 AAC */
            break;
        case TS_PSI_ST_ATSC_AC3:      /* ATSC AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_EAC3:     /* ATSC Enhanced AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3Plus, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_DTS_HD:   /* ASTC DTS-UHD audio */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDtsHd, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Audio:     /* AVS */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAvs, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_DRA_Audio:     /* DRA */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDra, pmt, pmtSize, i);
            break;


        /* video or audio */
        case TS_PSI_ST_13818_1_PrivatePES:  /* examine descriptors to handle private data */
            for (;;) {
                TS_PSI_descriptor desc = TS_PMT_getStreamDescriptor(pmt, pmtSize, i, descIdx);
                if (desc == NULL) break;
                descIdx++;

                switch(desc[0]) {
                /* video formats */
                case TS_PSI_DT_VideoStream:
                    /* MPEG_1_only_flag is bit 2 of desc[2], this determines MPEG-1/2 */
                    if ((desc[2] & 0x04) == 1)
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg1, pmt, pmtSize, i);
                    else
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_MPEG4_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg4Part2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVC:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVS_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eAvs, pmt, pmtSize, i);
                    break;

                /* audio formats */
                case TS_PSI_DT_AudioStream:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);   /* Same baudio_format for MPEG-1 or MPEG-2 audio */
                    break;
                case TS_PSI_DT_MPEG2_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAac, pmt, pmtSize, i);   /* Note baudio_format_aac = MPEG-2 AAC */
                    break;
                case TS_PSI_DT_MPEG4_Audio:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_EnhancedAC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3Plus, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_DTS:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDts, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_DRA:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDra, pmt, pmtSize, i);
                    break;
                default:
                    BDBG_MSG(("Unsupported private descriptor 0x%x",desc[0]));
                    break;
                }
            }
            break;
        default:
            if( p_programInfo->num_other_pids < TSPSIMGR_MAX_PIDS )
            {
                ADD_OTHER_PID(p_programInfo, pmt_stream.elementary_PID, pmt_stream.stream_type, pmt, pmtSize, i);
            }
            break;
        }
        /* If we get any data our status is complete! */
    } /* EFOR Program map loop */
}

static void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, tspsimgr_program *progInfo )
{
    int i;
    TS_PSI_descriptor descriptor;

    for( i = 0, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i );
        descriptor != NULL;
        i++, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i ) )
    {
        switch (descriptor[0])
        {
        case TS_PSI_DT_CA:
            progInfo->ca_pid = ((descriptor[4] & 0x1F) << 8) + descriptor[5];
            break;

        default:
            break;
        }
    }
}

static unsigned tsPsi_procStreamDescriptors_getCaPid( const uint8_t *p_bfr, unsigned bfrSize, int streamNum )
{
    int i;
    TS_PSI_descriptor descriptor;

    for( i = 0, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i );
        descriptor != NULL;
        i++, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i ) )
    {
        switch (descriptor[0])
        {
        case TS_PSI_DT_CA:
            return ((descriptor[4] & 0x1F) << 8) + descriptor[5];
            break;

        default:
            break;
        }
    }
    return 0;
}

tspsimgr_t tspsimgr_create(void)
{
    tspsimgr_t handle;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    BKNI_Memset(handle, 0, sizeof(*handle));
    BKNI_CreateMutex(&handle->mutex);
    return handle;
}

void tspsimgr_destroy( tspsimgr_t handle )
{
    tspsimgr_stop_scan(handle);
    BKNI_DestroyMutex(handle->mutex);
    BKNI_Free(handle);
}

void tspsimgr_get_default_start_scan_settings( tspsimgr_scan_settings *psettings )
{
    BKNI_Memset(psettings, 0, sizeof(*psettings));
    psettings->exclude_no_video_audio = true;
    psettings->exclude_ca_pid = true;

}

int tspsimgr_start_scan( tspsimgr_t handle, const tspsimgr_scan_settings *psettings )
{
    NEXUS_MessageStartSettings startSettings;
    NEXUS_MessageSettings openSettings;
    int rc;

    if (handle->started) {
        tspsimgr_stop_scan(handle);
    }
    
    BKNI_AcquireMutex(handle->mutex);
    BKNI_Memset(&handle->results, 0, sizeof(handle->results));
    BKNI_ReleaseMutex(handle->mutex);

    /* start scan for PAT */
    NEXUS_Message_GetDefaultSettings(&openSettings);
    openSettings.dataReady.callback = message_callback;
    openSettings.dataReady.context = handle;
    openSettings.dataReady.param = 0; /* PAT */
    handle->settings = *psettings;
    handle->msg[0] = NEXUS_Message_Open(&openSettings);
    if (!handle->msg[0]) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    /* PAT is on pid channel 0x0 */
    handle->pidChannel[0] = message_p_openpid(psettings, 0x0);
    if (!handle->pidChannel[0]) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    NEXUS_Message_GetDefaultStartSettings(handle->msg[0], &startSettings);
    startSettings.pidChannel = handle->pidChannel[0];
    rc = NEXUS_Message_Start(handle->msg[0], &startSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }

    handle->scan_done = false;
    handle->started = true;
    return 0;

error:
    if (handle->msg[0]) {
        NEXUS_Message_Close(handle->msg[0]);
        handle->msg[0] = NULL;
    }
    if (handle->pidChannel[0]) {
        NEXUS_PidChannel_Close(handle->pidChannel[0]);
        handle->pidChannel[0] = NULL;
    }
    return rc;
}

void tspsimgr_stop_scan( tspsimgr_t handle )
{
    unsigned i;
    
    if (!handle->started) return;
    
    for (i=0;i<MAX_MSG;i++) {
        if (handle->msg[i]) {
            NEXUS_Message_Close(handle->msg[i]);
            handle->msg[i] = NULL;
        }
        if (handle->pidChannel[i]) {
            NEXUS_PidChannel_Close(handle->pidChannel[i]);
            handle->pidChannel[i] = NULL;
        }
    }
    handle->started = false;    
}

int tspsimgr_get_scan_results( tspsimgr_t handle, tspsimgr_scan_results *presults )
{
    int rc;
    BKNI_AcquireMutex(handle->mutex);
    *presults = handle->results;
    rc = handle->scan_done?0:-1;
    BKNI_ReleaseMutex(handle->mutex);
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs transport)!\n");
    return 0;
}
#endif
