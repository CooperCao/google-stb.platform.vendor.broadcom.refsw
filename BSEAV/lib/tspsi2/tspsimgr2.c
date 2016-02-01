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
#include "tspsimgr2.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
BDBG_MODULE(tspsimgr);

#include "bkni.h"
#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset */

#include "b_os_lib.h"

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
static BERR_Code tsPsi_getProgramMaps(NEXUS_ParserBand   band,
                                      const void       * pPatBuffer,
                                      unsigned           patBufferSize,
                                      CHANNEL_INFO_T   * pChanInfo);
static void tsPsi_p_dumpBuffer(const uint8_t *p_bfr, unsigned bfrSize);

#if BDBG_DEBUG_BUILD
#define B_ERROR(ERR) (BDBG_ERR(("%s at line %d", #ERR, __LINE__)), (ERR))
#else
#define B_ERROR(ERR) (ERR)
#endif

void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    B_Event_Set((B_EventHandle)context);
}

#include "nexus_message.h"
int tsPsi_getPAT2(NEXUS_ParserBand band, void * pPatBuffer, unsigned patBufferSize)
{
    NEXUS_MessageHandle           msg                 = NULL;
    NEXUS_MessageSettings         openSettings;
    NEXUS_MessageStartSettings    startSettings;
    NEXUS_PidChannelHandle        pidChannel          = NULL;
    NEXUS_PidChannelSettings      pidChannelSettings;
    B_EventHandle                 event               = NULL;
    const uint8_t               * buffer              = NULL;
    size_t                        bufferSize          = 0;
    int                           patSize             = -1;            /* assume failure */
    NEXUS_Error                   rc                  = NEXUS_SUCCESS; /* assume success */

    BDBG_ASSERT(NULL != pPatBuffer);

    event = B_Event_Create(NULL);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    /* PAT is on pid channel 0x0 */
    pidChannel = NEXUS_PidChannel_Open(band, 0x0, &pidChannelSettings);

    NEXUS_Message_GetDefaultSettings(&openSettings);
    openSettings.dataReady.callback       = message_callback;
    openSettings.dataReady.context        = event;
    openSettings.maxContiguousMessageSize = 4096;

    msg = NEXUS_Message_Open(&openSettings);
    BDBG_ASSERT(NULL != msg);

    NEXUS_Message_GetDefaultStartSettings(msg, &startSettings);
    startSettings.pidChannel = pidChannel;

    rc = NEXUS_Message_Start(msg, &startSettings);
    if (NEXUS_SUCCESS != rc)
    {
        BDBG_WRN(("Nexus message start failure"));
        goto done;
    }

    do
    {
        rc = NEXUS_Message_GetBuffer(msg, (const void **)&buffer, &bufferSize);
        BDBG_ASSERT(!rc);

        if (0 == bufferSize)
        {
            rc = B_Event_Wait(event, tsPsi_patTimeout * 10);
            if (NEXUS_SUCCESS != rc)
            {
                BDBG_WRN(("Timeout waiting for PAT data"));
                goto done;
            }

            /* PAT data now available, try reading buffer again */
            continue;
        }
    }
    while (0 == bufferSize);

    /* validate PAT before returning it */
    if (true == TS_PAT_validate(buffer, bufferSize))
    {
        patSize = bufferSize;

        /* copy PAT into caller supplied buffer */
        if (patSize > (int)patBufferSize)
        {
            patSize = (int)patBufferSize;
        }
        BKNI_Memcpy(pPatBuffer, buffer, patSize);
    }
    else
    {
        tsPsi_p_dumpBuffer(buffer, bufferSize);
        BDBG_WRN(("Invalid PAT data detected"));
    }

done:
    if (bufferSize)
    {
        NEXUS_Message_ReadComplete(msg, bufferSize);
    }

    if (NULL != msg)
    {
        NEXUS_Message_Stop(msg);
        NEXUS_Message_Close(msg);
    }

    if (NULL != pidChannel)
    {
        NEXUS_PidChannel_Close(pidChannel);
    }

    if (NULL != event)
    {
        B_Event_Destroy(event);
    }

    return patSize;
}


BERR_Code tsPsi_getChannelInfo2(CHANNEL_INFO_T * pChanInfo, NEXUS_ParserBand band)
{
    uint8_t       pat[PSI_BFR_SIZE];
    int           patSize;
    TS_PSI_header header;

    /* Blocking call to get PAT */
    patSize = tsPsi_getPAT2(band, pat, PSI_BFR_SIZE);
    /* If there's no PAT, return but don't print an error because this can happen
    normally. */
    if (patSize <= 0)
        return berr_external_error;

    TS_PSI_getSectionHeader(pat, &header);
    pChanInfo->version             = header.version_number;
    pChanInfo->transport_stream_id = header.table_id_extension;
    pChanInfo->num_programs        = 0;

    BDBG_MSG(("Parsing PAT"));
    return tsPsi_getProgramMaps(band, pat, patSize, pChanInfo);
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


void tsPsi_parsePMT2( const void *pmt, unsigned pmtSize, PROGRAM_INFO_T *p_programInfo)
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
        case TS_PSI_ST_ATSC_DTS:     /* ASTC ??? DTS audio */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDts, pmt, pmtSize, i);
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
                    if ((desc[2] & 0x04) == 0x04)
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
        tsPsi_parsePMT2(pmt, size, &programInfo);
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

typedef struct tsPsi_PmtData
{
    NEXUS_MessageHandle    msg;
    NEXUS_PidChannelHandle pidChannel;
    TS_PAT_program         program;
} tsPsi_PmtData;

static void clearPmtData(tsPsi_PmtData * data)
{
    if ((NULL == data) || (NULL == data->msg))
    {
        /* invalid or already cleared */
        return;
    }

    if (NULL != data->pidChannel)
    {
        NEXUS_PidChannel_Close(data->pidChannel);
    }

    if (NULL != data->msg)
    {
        NEXUS_Message_Stop(data->msg);
        NEXUS_Message_Close(data->msg);
    }

    memset(data, 0, sizeof(tsPsi_PmtData));
}

static BERR_Code tsPsi_getProgramMaps(NEXUS_ParserBand   band,
                                      const void       * pPatBuffer,
                                      unsigned           patBufferSize,
                                      CHANNEL_INFO_T   * pChanInfo)
{
    return tsPsi_getPMTs2(band, pPatBuffer, patBufferSize, tsPsi_p_addChanInfo, pChanInfo);
}

BERR_Code tsPsi_getPMTs2(NEXUS_ParserBand     band,
                        const void         * pPatBuffer,
                        unsigned             patBufferSize,
                        tsPsi_PMT_callback   callback,
                        void               * context)
{
    BERR_Code        result                  = BERR_SUCCESS;
    int              numProgramsTotal        = 0;
    int              numProgramsToGet        = 0;
    int              numProgramsReceived     = 0;
    int              curProgramNum           = 0;
    TS_PAT_program   program;
    tsPsi_PmtData  * pmtDataArray            = NULL;

    int             timeout;

    numProgramsTotal = TS_PAT_getNumPrograms(pPatBuffer);
    BDBG_MSG(("numProgramsTotal %d", numProgramsTotal));
    if( numProgramsTotal > MAX_PROGRAMS_PER_CHANNEL )
    {
        BDBG_WRN(("Maximum number of programs exceeded in tspsimgr: %d > %d",
                   numProgramsTotal, MAX_PROGRAMS_PER_CHANNEL));
        numProgramsTotal = MAX_PROGRAMS_PER_CHANNEL;
    }

    pmtDataArray = (tsPsi_PmtData *)malloc(sizeof(tsPsi_PmtData) * numProgramsTotal);
    memset(pmtDataArray, 0, sizeof(tsPsi_PmtData) * numProgramsTotal);

    /* if the number of total programs exceeds the number of available pid channels and
       message filters, we may have to acquire the program data in multiple tries */
    while (numProgramsTotal)
    {
        int i = 0;

        result = TS_PAT_getProgram(pPatBuffer, patBufferSize, curProgramNum, &program );
        if (BERR_SUCCESS != result)
        {
            BDBG_WRN(("Error getting PAT program num:%d", curProgramNum));
            break;
        }

        /* Always try to read the max number of pids at the same time */
        numProgramsToGet    = numProgramsTotal;
        numProgramsReceived = 0;

        /* open pid channels and message filters for all programs listed in given PAT.
           note that we may break out early if we exhaust the available pid channels and
           message filters...the outer while loop will handle this case */
        for( i = 0; i < numProgramsToGet; i++ )
        {
            /* We are finished with this program association so go to the next */
            result = TS_PAT_getProgram(pPatBuffer, patBufferSize, curProgramNum, &program);
            if(BERR_SUCCESS != result)
            {
                BDBG_WRN(("Error getting PAT program num:%d", curProgramNum));
                numProgramsTotal = 0;
                break;
            }

            curProgramNum++;
            pmtDataArray[i].program = program;

            if( program.program_number == 0 )
            {
                /* This is the network PID, so ignore it */
                numProgramsReceived++;
            }
            else
            {
                NEXUS_PidChannelSettings      pidChannelSettings;
                NEXUS_MessageSettings         openSettings;
                NEXUS_MessageStartSettings    startSettings;
                NEXUS_Error                   nerror              = NEXUS_SUCCESS;

                NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
                pmtDataArray[i].pidChannel = NEXUS_PidChannel_Open(band, program.PID, &pidChannelSettings);

                NEXUS_Message_GetDefaultSettings(&openSettings);
                openSettings.maxContiguousMessageSize = 4096;
                pmtDataArray[i].msg = NEXUS_Message_Open(&openSettings);

                /* check for pid channel open error or message open error */
                if ((NULL == pmtDataArray[i].pidChannel) || (NULL == pmtDataArray[i].msg))
                {
                    /* possibly out of pid/message resources so break out and continue
                       processing what we've opened so far...we'll try again the next
                       time through the while loop */
                    BDBG_WRN(("Error opening message capture"));
                    /* failure, so make sure pidchannel and message filter is closed */
                    clearPmtData(&pmtDataArray[i]);

                    /* Decrease the number of programs to get due to transport resources */
                    numProgramsToGet = i - 1;

                    if (numProgramsToGet == 0)
                    {
                        /* Abort due to not being able to enable messages */
                        numProgramsTotal = 0;
                    }
                    break;
                }

                BDBG_MSG(("filter pid %#x, program %#x", pmtDataArray[i].program.PID, pmtDataArray[i].program.program_number));

                NEXUS_Message_GetDefaultStartSettings(pmtDataArray[i].msg, &startSettings);
                startSettings.pidChannel             = pmtDataArray[i].pidChannel;
                startSettings.filter.mask[0]         = 0x0;
                startSettings.filter.mask[2]         = 0x0;
                /*startSettings.filter.mask[4]         = 0x0;*/
                startSettings.filter.mask[15]        = 0xFF;
                startSettings.filter.coefficient[0]  = 0x02;
                startSettings.filter.coefficient[2]  = (program.program_number & 0xFF00) >> 8;
                /*startSettings.filter.coefficient[4] = program.program_number & 0xFF;*/
                startSettings.filter.coefficient[15] = 0xFF;
                nerror = NEXUS_Message_Start(pmtDataArray[i].msg, &startSettings);
                if (NEXUS_SUCCESS != nerror)
                {
                    BDBG_WRN(("Error starting message capture - skipping this program (num:%d pid:%d",
                              pmtDataArray[i].program.program_number, pmtDataArray[i].program.PID));
                    /* failure, so make sure pidchannel and message filter is closed */
                    clearPmtData(&pmtDataArray[i]);
                    /* keep going - we will not attempt to get data for this program */
                }
            }
        }

        /* Now we have enabled our pid channels and message filters, so wait for each one to get some data */
        timeout = tsPsi_pmtTimeout;
        while ((numProgramsReceived < numProgramsToGet) && (timeout != 0))
        {
            bool messageReceived;

            messageReceived = false;

            /* Check each of the pid channels we are waiting for */
            for (i = 0; i < numProgramsToGet; i++)
            {
                const void * buffer     = NULL;
                size_t       bufferSize = 0;

                /* see if message data is available */
                if ((NULL != pmtDataArray[i].msg) &&
                    (NEXUS_SUCCESS == NEXUS_Message_GetBuffer(pmtDataArray[i].msg, &buffer, &bufferSize)) &&
                    (0 < bufferSize))
                {
                    /* PMT data received */
                    messageReceived = true;
                    numProgramsReceived++;

                    BDBG_MSG(("PMT: %d bufferSize:%d (%02x %02x %02x)", i, bufferSize,
                        ((unsigned char *)buffer)[0],((unsigned char *)buffer)[1],((unsigned char *)buffer)[2]));

                    if (false == TS_PMT_validate(buffer, bufferSize))
                    {
                        BDBG_WRN(("Invalid PMT data detected: ch %d, bufferSize 0x%x", i, bufferSize));
                        tsPsi_p_dumpBuffer(buffer, bufferSize);
                    }
                    else
                    {
                        /* Give the PMT to the callback */
                        (*callback)(context, pmtDataArray[i].program.PID, buffer, bufferSize);
                    }

                    /* cannot stop until after the data has been parsed because
                    we're not copying the data into a local buffer.  callback is complete
                    so assume data has been copied/processed so we can clear it now. */
                    clearPmtData(&pmtDataArray[i]);
                }
            }

            if (false == messageReceived)
            {
                BKNI_Sleep(10);
                timeout--;
            }
        }

        /* all done for this loop of "programs to get" so clear out pmt data */
        for( i = 0; i < numProgramsToGet; i++ )
        {
            clearPmtData(&pmtDataArray[i]);
        }

        numProgramsTotal -= numProgramsToGet;
    }

    free(pmtDataArray);

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

void tsPsi_setTimeout2( int patTimeout, int pmtTimeout )
{
    tsPsi_patTimeout = patTimeout;
    tsPsi_pmtTimeout = pmtTimeout;
}

void tsPsi_getTimeout2( int *patTimeout, int *pmtTimeout )
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
