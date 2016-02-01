/******************************************************************************
 * (c) 2002-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "bsettop.h"
#include "tspsimgr.h"
#include "bsettop_message.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
BDBG_MODULE(tspsimgr);

#include "bkni.h"
#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset */

#define PSI_BFR_SIZE    TS_PSI_MAX_PSI_TABLE_SIZE

/* Note that the following timeouts are not 400 milliseconds.
It is 400 BKNI_Sleep(10)'s, which will be about 4000 ms (4 seconds) on linux.
Also, the original default of 200 was found to be too short for some customer streams. The downside
of having too large a timeout value is usually app delays when scanning non-existent streamers
or streams with no PSI information. Not critical. */
static int tsPsi_patTimeout = 400;

/* PR 18488 - temporarily increasing timeout for 740x platforms. */
static int tsPsi_pmtTimeout = 600;

static void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, PROGRAM_INFO_T *progInfo );
static void tsPsi_procStreamDescriptors( const uint8_t *p_bfr, unsigned bfrSize, int streamNum, EPID *ePidData );
static bresult tsPsi_getProgramMaps( bband_t band, const void *p_patBfr, unsigned pat_bfrSize,
    CHANNEL_INFO_T *p_chanInfo, bstream_t stream);
static void tsPsi_p_dumpBuffer(const uint8_t *p_bfr, unsigned bfrSize);

#if BDBG_DEBUG_BUILD
#define B_ERROR(ERR) (BDBG_ERR(("%s at line %d", #ERR, __LINE__)), (ERR))
#else
#define B_ERROR(ERR) (ERR)
#endif

int tsPsi_getPAT(bband_t band, void *p_bfr, unsigned bfrSize, bstream_t stream)
{
    bmessage_stream_t patStream;
    bmessage_stream_params params;
    int timeout;
    bresult result;
    unsigned patSize = 0;

    patStream = bmessage_open(bmessage_format_psi);
    if (!patStream) return -1;

    bmessage_stream_params_init(&params, patStream);
    params.band = band;
    params.stream = stream;
    params.pid = 0x0;

    result = bmessage_start(patStream, &params);
    if (result) return -1;

    BDBG_MSG(("Looking for PAT"));
    for( timeout = tsPsi_patTimeout; timeout > 0 && patSize == 0; timeout-- )
    {
        const void *buffer;
        if (bmessage_get_buffer(patStream, &buffer, &patSize))
            return -1;
        if (patSize) {
            if (patSize > bfrSize)
                patSize = bfrSize;
            BKNI_Memcpy(p_bfr, buffer, patSize);
            /* don't call bmessage_read_complete because we're stopping anyway */
        }
        else {
            BKNI_Sleep(10);
        }
    }
    bmessage_close(patStream);

    /* validate before returning it */
    if (patSize && !TS_PAT_validate(p_bfr, patSize)) {
        tsPsi_p_dumpBuffer(p_bfr, patSize);
        BDBG_WRN(("Invalid PAT data detected"));
        return -1;
    }

    return patSize;
}

bresult tsPsi_getChannelInfo(CHANNEL_INFO_T *p_chanInfo, bband_t band, bstream_t stream )
{
    uint8_t     pat[PSI_BFR_SIZE];
    size_t      patSize;
    TS_PSI_header header;

    /* Blocking call to get PAT */
    patSize = tsPsi_getPAT(band, pat, PSI_BFR_SIZE, stream);
    /* If there's no PAT, return but don't print an error because this can happen
    normally. */
    if (patSize <= 0)
        return berr_external_error;

    TS_PSI_getSectionHeader( pat, &header );
    p_chanInfo->version                 = header.version_number;
    p_chanInfo->transport_stream_id     = header.table_id_extension;
    p_chanInfo->num_programs            = 0;

    BDBG_MSG(("Parsing PAT"));
    return tsPsi_getProgramMaps( band, pat, patSize, p_chanInfo, stream );
}

#define ADD_VIDEO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_video_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  vpid[%d] 0x%x, type 0x%x", (P_INFO)->num_video_pids, (PID), (TYPE))); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].pid = (PID); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->video_pids[(P_INFO)->num_video_pids] ); \
    (P_INFO)->num_video_pids++; \
    } \
    } while (0)
#define ADD_AUDIO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_audio_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  apid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].pid = (PID); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->audio_pids[(P_INFO)->num_audio_pids] ); \
    (P_INFO)->num_audio_pids++; \
    } \
    } while (0)
#define ADD_OTHER_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_other_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  opid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].pid = (PID); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->other_pids[(P_INFO)->num_other_pids] ); \
    (P_INFO)->num_other_pids++; \
    } \
    } while (0)

void tsPsi_parsePMT( const void *pmt, unsigned pmtSize, PROGRAM_INFO_T *p_programInfo)
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
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg1, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_Video:   /* ATSC MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_13818_2_Video: /* MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_2_Video: /* MPEG-4 Part 2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg4_part2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_Video: /* H.264/AVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_h264, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_AnnexG_Video: /* H.264/SVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_h264_svc, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_AnnexH_Video: /* H.264/MVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_h264_mvc, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_23008_2_Video: /* H.265/HEVC */
        case TS_PSI_ST_23008_2_Video_brcm: /*deprecated */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_h265, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Video: /* AVS */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_avs, pmt, pmtSize, i);
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
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_vc1, pmt, pmtSize, i);
                            else /* Simple/Main Profile ES */
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_vc1_sm, pmt, pmtSize, i);
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
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_mpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_3_Audio: /* MPEG-2 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_mpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_7_AAC:  /* MPEG-2 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_aac, pmt, pmtSize, i);    /* Note baudio_format_aac = MPEG-2 AAC */
            break;
        case TS_PSI_ST_14496_3_Audio: /* MPEG-4 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_aac_plus, pmt, pmtSize, i);   /* Note baudio_format_aac_plus = MPEG-4 AAC */
            break;
        case TS_PSI_ST_ATSC_AC3:      /* ATSC AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_ac3, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_EAC3:     /* ATSC Enhanced AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_ac3_plus, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_DTS:     /* ASTC ??? DTS audio */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_dts, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Audio:     /* AVS */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_avs, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_DRA_Audio:     /* DRA */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_dra, pmt, pmtSize, i);
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
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg1, pmt, pmtSize, i);
                    else
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_MPEG4_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_mpeg4_part2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVC:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_h264, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVS_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, bvideo_codec_avs, pmt, pmtSize, i);
                    break;

                /* audio formats */
                case TS_PSI_DT_AudioStream:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_mpeg, pmt, pmtSize, i);   /* Same baudio_format for MPEG-1 or MPEG-2 audio */
                    break;
                case TS_PSI_DT_MPEG2_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_aac, pmt, pmtSize, i);   /* Note baudio_format_aac = MPEG-2 AAC */
                    break;
                case TS_PSI_DT_MPEG4_Audio:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_aac_plus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_aac_plus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_ac3, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_EnhancedAC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_ac3_plus, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_DTS:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_dts, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_DRA:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, baudio_format_dra, pmt, pmtSize, i);
                    break;
                default:
                    BDBG_MSG(("Unsupported private descriptor 0x%x",desc[0]));
                    break;
                }
            }
            break;
        default:
            if( p_programInfo->num_other_pids < MAX_PROGRAM_MAP_PIDS )
            {
                ADD_OTHER_PID(p_programInfo, pmt_stream.elementary_PID, pmt_stream.stream_type, pmt, pmtSize, i);
            }
            break;
        }
        /* If we get any data our status is complete! */
    } /* EFOR Program map loop */
}

static void tsPsi_p_addChanInfo(void *context, uint16_t pmt_pid, const void *pmt, unsigned size)
{
    CHANNEL_INFO_T *p_chanInfo = (CHANNEL_INFO_T*)context;
    PROGRAM_INFO_T programInfo;
    int i;

    BKNI_Memset(&programInfo, 0, sizeof(programInfo));

    /* If this isn't true, then the logic at the top of tsPsi_getPMTs has failed,
    or we haven't stopped each msgstream after reading one and only one item. */
    BDBG_ASSERT(p_chanInfo->num_programs < MAX_PROGRAMS_PER_CHANNEL);

    /* The "if" comparision below is present to silence the Coverity from choosing the false path in the above "BDBG_ASSERT" line of code. */
    if(p_chanInfo->num_programs < MAX_PROGRAMS_PER_CHANNEL){
        tsPsi_parsePMT(pmt, size, &programInfo);
        programInfo.map_pid = pmt_pid;

        /* now that we know the program_number, insert it into the array */
        for (i=0;i<p_chanInfo->num_programs;i++) {
            if (programInfo.program_number < p_chanInfo->program_info[i].program_number)
                break;
        }
        /* make room for an insertion */
        if (i < p_chanInfo->num_programs) {
            unsigned char *ptr = (unsigned char *)&p_chanInfo->program_info[i];
            BKNI_Memmove(ptr + sizeof(PROGRAM_INFO_T), ptr, sizeof(PROGRAM_INFO_T) * (p_chanInfo->num_programs - i));
        }
        /* now copy into place */
        BKNI_Memcpy(&p_chanInfo->program_info[i], &programInfo, sizeof(PROGRAM_INFO_T));
        p_chanInfo->num_programs++;
    }
}

static bresult  tsPsi_getProgramMaps( bband_t band, const void *p_patBfr, unsigned pat_bfrSize,
    CHANNEL_INFO_T *p_chanInfo, bstream_t stream )
{
    return tsPsi_getPMTs(band, p_patBfr, pat_bfrSize, tsPsi_p_addChanInfo, p_chanInfo, stream);
}

bresult tsPsi_getPMTs(bband_t band, const void *p_patBfr, unsigned pat_bfrSize,
    tsPsi_PMT_callback callback, void *context, bstream_t stream)
{
    bresult         result = b_ok;
    int             i;
    size_t          bfrSize;
    int             num_programs;
    int             num_programs_to_get;
    int             num_messages_received;
    bool            message_received;
    int             timeout;
    bmessage_stream_t   *pmtStreamArray;
    uint16_t        *pmt_pidArray;
    TS_PAT_program  program;
    int             curProgramNum = 0;

    num_programs = TS_PAT_getNumPrograms(p_patBfr);
    BDBG_MSG(("num_programs %d", num_programs));
    if( num_programs > MAX_PROGRAMS_PER_CHANNEL )
    {
        BDBG_WRN(("Maximum number of programs exceeded in tspsimgr: %d > %d",
            num_programs, MAX_PROGRAMS_PER_CHANNEL));
        num_programs = MAX_PROGRAMS_PER_CHANNEL;
    }

    pmtStreamArray = (bmessage_stream_t *)malloc( sizeof(bmessage_stream_t) * num_programs );
    memset(pmtStreamArray, 0, sizeof(bmessage_stream_t) * num_programs);

    pmt_pidArray = (uint16_t *)malloc( sizeof(uint16_t) * num_programs );
    memset(pmt_pidArray, 0, sizeof(uint16_t) * num_programs);

    result = TS_PAT_getProgram( p_patBfr, pat_bfrSize, curProgramNum, &program );
    curProgramNum++;

    if( result == b_ok )
    {
        while( num_programs )
        {
            /* Always try to read the max number of pids at the same time */
            num_programs_to_get = num_programs;
            num_messages_received = 0;

            for( i = 0; i < num_programs_to_get; i++ )
            {
                if( program.program_number == 0 )
                {
                    /* This is the network PID, so ignore it */
                    num_messages_received++;
                }
                else
                {
                    bmessage_stream_params params;

                    pmtStreamArray[i] = bmessage_open(bmessage_format_psi);
                    if( pmtStreamArray[i] == NULL )
                    {
                        /* Decrease the number of programs to get in this loop due to transport resources */
                        num_programs_to_get = i-1;

                        if( num_programs_to_get == 0 )
                        {
                            /* Abort due to not being able to enable messages */
/*                          BRCM_DBG_MSG(("Unable to enable any messages!")); */
                            num_programs = 0;
                        }
                        break;
                    }

                    bmessage_stream_params_init(&params, pmtStreamArray[i]);
                    params.band = band;
                    params.stream = stream;
                    params.pid = program.PID;
                    /* these fields must match */
                    params.filter.mask[0] = 0x00;
                    params.filter.mask[3] = 0x00;
                    params.filter.mask[4] = 0x00;

                    /* they must match these values */
                    params.filter.coef[0] = 0x02;
                    params.filter.coef[3] = (program.program_number & 0xFF00) >> 8;
                    params.filter.coef[4] = program.program_number & 0xFF;

                    BDBG_MSG(("filter pid %#x, program %#x", params.pid, program.program_number));

                    pmt_pidArray[i] = program.PID;
                    if (bmessage_start(pmtStreamArray[i], &params)) {
                        bmessage_close(pmtStreamArray[i]);
                        pmtStreamArray[i] = NULL;
                        pmt_pidArray[i] = 0;
                    }
                }

                /* We are finished with this program association so go to the next */

                /* TODO: Check for error */
                TS_PAT_getProgram( p_patBfr, pat_bfrSize, curProgramNum, &program );
                curProgramNum++;
            }

            /* Now we have enabled our pid channels, so wait for each one to get some data */
            timeout = tsPsi_pmtTimeout;
            while( num_messages_received != num_programs_to_get && timeout != 0 )
            {
                message_received = 0;
                /* Check each of the pid channels we are waiting for */
                for( i = 0; i < num_programs_to_get; i++ )
                {
                    const void *buffer;
                    if (pmtStreamArray[i] &&
                        !bmessage_get_buffer(pmtStreamArray[i], &buffer, &bfrSize ) &&
                        bfrSize)
                    {
                        /* don't call bmessage_read_complete because we're stopping anyway */

                        message_received = true;
                        num_messages_received++;

                        BDBG_MSG(("PMT: %d %d (%02x %02x %02x)", i, bfrSize,
                            ((unsigned char *)buffer)[0],((unsigned char *)buffer)[1],((unsigned char *)buffer)[2]));

                        if (!TS_PMT_validate(buffer, bfrSize)) {
                            BDBG_WRN(("Invalid PMT data detected: ch %d, bfrSize 0x%x", i, bfrSize));
                            tsPsi_p_dumpBuffer(buffer, bfrSize);
                        }
                        else {
                            /* Give the PMT to the callback */
                            (*callback)(context, pmt_pidArray[i], buffer, bfrSize);
                        }

                        /* cannot stop until after the data has been parsed because
                        we're not copying the data into a local buffer */
                        bmessage_stop(pmtStreamArray[i]);
                    }
                }
                if( !message_received )
                {
                    BKNI_Sleep(10);
                    timeout--;
                }
            }

            /* Now disable our pid channels */
            for( i = 0; i < num_programs_to_get; i++ )
            {
                if (pmtStreamArray[i])
                    bmessage_close(pmtStreamArray[i]);
            }
            num_programs -= num_programs_to_get;
        }
    }

    free( pmtStreamArray );
    free(pmt_pidArray);

    return result;
}

void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, PROGRAM_INFO_T *progInfo )
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

void tsPsi_procStreamDescriptors( const uint8_t *p_bfr, unsigned bfrSize, int streamNum, EPID *ePidData )
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
            ePidData->ca_pid = ((descriptor[4] & 0x1F) << 8) + descriptor[5];
            break;

        default:
            break;
        }
    }
}

void tsPsi_setTimeout( int patTimeout, int pmtTimeout )
{
    tsPsi_patTimeout = patTimeout;
    tsPsi_pmtTimeout = pmtTimeout;
}

void tsPsi_getTimeout( int *patTimeout, int *pmtTimeout )
{
    *patTimeout = tsPsi_patTimeout;
    *pmtTimeout = tsPsi_pmtTimeout;
}

static void tsPsi_p_dumpBuffer(const uint8_t *p_bfr, unsigned bfrSize)
{
    unsigned i;
    for (i=0;i<bfrSize;i++)
        BKNI_Printf("%02X", p_bfr[i]);
    BKNI_Printf("\n");
}
