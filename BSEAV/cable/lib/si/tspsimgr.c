/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#include "tspsimgr.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
BDBG_MODULE(tspsimgr);

#include "bkni.h"
#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset */

#ifdef SOFTWARE_RS_MPOD
#include "mpod_rate_smooth.h"
#endif

#define PSI_BFR_SIZE    TS_PSI_MAX_PSI_TABLE_SIZE

static void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, PROGRAM_INFO_T *progInfo );
static void tsPsi_procStreamDescriptors( const uint8_t *p_bfr, unsigned bfrSize, int streamNum, EPID *ePidData );
static int  tsPsi_getProgramMaps( NEXUS_ParserBand parserBand, const void *p_patBfr, unsigned pat_bfrSize,
    CHANNEL_INFO_T *p_chanInfo);
static void tsPsi_p_dumpBuffer(const uint8_t *p_bfr, unsigned bfrSize);

#if BDBG_DEBUG_BUILD
#define B_ERROR(ERR) (BDBG_ERR(("%s at line %d", #ERR, __LINE__)), (ERR))
#else
#define B_ERROR(ERR) (ERR)
#endif

static int tsPsi_patTimeout = 400;
static int tsPsi_pmtTimeout = 600;
static int patTimeoutCnt = 0;
static int pmtTimeoutCnt = 0;


NEXUS_PidChannelHandle open_msg_pidchannel(NEXUS_ParserBand parseBand, unsigned int pid)
{
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_PlaypumpOpenPidChannelSettings playpump_pidChannelSettings;
    NEXUS_PlaybackPidChannelSettings playback_pidChannelSettings;
	NEXUS_PidChannelHandle pidchannel;

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
	pidChannelSettings.requireMessageBuffer = true;
#if BCHP_CHIP == 7445
	pidChannelSettings.pidChannelIndex= NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;
#endif

#ifdef SOFTWARE_RS_MPOD
    pidchannel = B_Mpod_PidChannelOpen(parseBand, pid, &pidChannelSettings);
#else
    pidchannel = NEXUS_PidChannel_Open(parseBand, pid, &pidChannelSettings);
#endif

	return pidchannel;
}

void  close_msg_pidchannel(NEXUS_PidChannelHandle pidchannel)
{
#ifdef SOFTWARE_RS_MPOD
    B_Mpod_PidChannelClose(pidchannel);
#else
    NEXUS_PidChannel_Close(pidchannel);
#endif
    return;
}


int tsPsi_getPAT(NEXUS_ParserBand parserBand, void *p_bfr, unsigned bfrSize)
{
	int timeout;
    unsigned patSize = 0;
	NEXUS_Error rc;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;

	pidChannel = open_msg_pidchannel(parserBand, 0);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.maxContiguousMessageSize = 4096;
    msg = NEXUS_Message_Open(&settings);
    BDBG_ASSERT(msg);

    NEXUS_Message_GetDefaultStartSettings(msg, &startSettings);
    startSettings.pidChannel = pidChannel;
    /* use the default filter for any data */
	rc = NEXUS_Message_Start(msg, &startSettings);
    if (rc) { patSize = -1 ;goto end;}

    BDBG_MSG(("Looking for PAT"));
    for( timeout = tsPsi_patTimeout; timeout > 0 && patSize == 0; timeout-- )
    {
		const uint8_t *buffer;
		rc = NEXUS_Message_GetBuffer(msg, (const void **)&buffer, (size_t *)&patSize);
        if (rc) { patSize = -1; BDBG_ERR(("PAT error"));goto end;}
        if (patSize) {
            if (patSize > bfrSize)
                patSize = bfrSize;
            BKNI_Memcpy(p_bfr, buffer, patSize);
			break;
        } else {
        	BKNI_Sleep(1);
        }
    }
	if (timeout == 0)
	{
		patTimeoutCnt ++;
	}
	if (!TS_PAT_validate(p_bfr, patSize)) {  	/* validate before returning it */
		tsPsi_p_dumpBuffer(p_bfr, patSize);
		BDBG_WRN(("Invalid PAT data detected"));
	}

end:
    NEXUS_Message_Stop(msg);
    NEXUS_Message_Close(msg);
    close_msg_pidchannel(pidChannel);
    return patSize;

}

static void tsPsi_p_addProgramInfo(void *context, uint16_t pmt_pid, const void *pmt, unsigned size)
{
    PROGRAM_INFO_T *programInfo = (PROGRAM_INFO_T*)context;
    int i;

    BKNI_Memset(programInfo, 0, sizeof(programInfo));

    tsPsi_parsePMT(pmt, size, programInfo);
    programInfo->map_pid = pmt_pid;
}

int  tsPsi_getProgramInfo(PROGRAM_INFO_T *p_pgInfo, unsigned pg_number, NEXUS_ParserBand parserBand,  unsigned char *pmt, unsigned int *pmt_size )
{
    uint8_t     pat[PSI_BFR_SIZE];
    size_t      patSize;
    TS_PSI_header header;

    /* Blocking call to get PAT */
    patSize = tsPsi_getPAT(parserBand, pat, PSI_BFR_SIZE);
    /* If there's no PAT, return but don't print an error because this can happen
    normally. */
    if (patSize <= 0) return -1;
	if (!TS_PAT_validate(pat, patSize)) {
		return -1;
	}
    return tsPsi_getPMT(parserBand, pat, patSize, pg_number, pmt ,pmt_size, tsPsi_p_addProgramInfo, p_pgInfo );

}

int  tsPsi_getChannelInfo(CHANNEL_INFO_T *p_chanInfo, NEXUS_ParserBand parserBand )
{
    uint8_t     pat[PSI_BFR_SIZE];
    size_t      patSize;
    TS_PSI_header header;

    /* Blocking call to get PAT */
    patSize = tsPsi_getPAT(parserBand, pat, PSI_BFR_SIZE);
    /* If there's no PAT, return but don't print an error because this can happen
    normally. */
    if (patSize <= 0) return -1;

    TS_PSI_getSectionHeader( pat, &header );
    p_chanInfo->version                 = header.version_number;
    p_chanInfo->transport_stream_id     = header.table_id_extension;
    p_chanInfo->num_programs            = 0;

    BDBG_MSG(("Parsing PAT"));
    return tsPsi_getProgramMaps(parserBand, pat, patSize, p_chanInfo);
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
        case TS_PSI_ST_AVS_Audio:     /* AVS */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAvs, pmt, pmtSize, i);
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
                default:
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

static int  tsPsi_getProgramMaps( NEXUS_ParserBand parserBand, const void *p_patBfr, unsigned pat_bfrSize,
    CHANNEL_INFO_T *p_chanInfo )
{
    return tsPsi_getPMTs(parserBand, p_patBfr, pat_bfrSize, tsPsi_p_addChanInfo, p_chanInfo );
}

int tsPsi_getPMTs(NEXUS_ParserBand parserBand, const void *p_patBfr, unsigned pat_bfrSize,
    tsPsi_PMT_callback callback, void *context)
{
    int             i;
    size_t          bfrSize;
    int             num_programs;
    int             num_programs_to_get;
    int             num_messages_received;
    bool            message_received;
    int             timeout;
    NEXUS_MessageHandle   *pmtStreamArray;
    uint16_t        *pmt_pidArray;
    TS_PAT_program  program;
    int             curProgramNum = 0;
	BERR_Code		ret;
    NEXUS_PidChannelHandle *pidChannelArray;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;

    num_programs = TS_PAT_getNumPrograms(p_patBfr);
    BDBG_MSG(("num_programs %d", num_programs));
    if( num_programs > MAX_PROGRAMS_PER_CHANNEL )
    {
        BDBG_WRN(("Maximum number of programs exceeded in tspsimgr: %d > %d",
            num_programs, MAX_PROGRAMS_PER_CHANNEL));
        num_programs = MAX_PROGRAMS_PER_CHANNEL;
    }

    pmtStreamArray = (NEXUS_MessageHandle *)malloc( sizeof(NEXUS_MessageHandle) * num_programs );
    memset(pmtStreamArray, 0, sizeof(NEXUS_MessageHandle) * num_programs);

    pidChannelArray = (NEXUS_PidChannelHandle *)malloc( sizeof(NEXUS_PidChannelHandle) * num_programs );
    memset(pidChannelArray, 0, sizeof(NEXUS_PidChannelHandle) * num_programs);


    pmt_pidArray = (uint16_t *)malloc( sizeof(uint16_t) * num_programs );
    memset(pmt_pidArray, 0, sizeof(uint16_t) * num_programs);

    ret = TS_PAT_getProgram( p_patBfr, pat_bfrSize, curProgramNum, &program );
    curProgramNum++;

    if( ret == BERR_SUCCESS)
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

				    NEXUS_Message_GetDefaultSettings(&settings);
					/* use polling for PMT*/
#if 0
					settings.dataReady.callback = message_callback;
				    settings.dataReady.context = event;
#endif
					settings.maxContiguousMessageSize = 4096;
                    pmtStreamArray[i] = NEXUS_Message_Open(&settings);

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

					pidChannelArray[i] = open_msg_pidchannel(parserBand, program.PID);

				    NEXUS_Message_GetDefaultStartSettings(pmtStreamArray[i], &startSettings);
				    startSettings.pidChannel = pidChannelArray[i];

					startSettings.filter.mask[0] = 0x0;
					startSettings.filter.mask[2] = 0x0;
					startSettings.filter.mask[3] = 0x0;
					startSettings.filter.coefficient[0] = 0x2;
					startSettings.filter.coefficient[2] = (program.program_number & 0xFF00) >> 8;
					startSettings.filter.coefficient[3] = program.program_number & 0xFF;
					startSettings.filter.coefficient[15] = 0xff;
                    BDBG_MSG(("filter pid %#x, program %#x", program.PID, program.program_number));

                    pmt_pidArray[i] = program.PID;

                    if (NEXUS_Message_Start(pmtStreamArray[i], &startSettings)) {
						close_msg_pidchannel(pidChannelArray[i]);
                        NEXUS_Message_Close(pmtStreamArray[i]);
                        pmtStreamArray[i] = NULL;
						pidChannelArray[i] = NULL;
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
                        !NEXUS_Message_GetBuffer(pmtStreamArray[i], &buffer, &bfrSize ) &&
                        bfrSize)
                    {
                        /* don't call NEXUS_Message_ReadComplete() because we're stopping anyway */

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
                        NEXUS_Message_Stop(pmtStreamArray[i]);
                    }
                }
                if( !message_received )
                {
                    BKNI_Sleep(1);
                    timeout--;
                }
            }
			if (timeout == 0)
			{
				pmtTimeoutCnt ++;
			}

            /* Now disable our pid channels */
            for( i = 0; i < num_programs_to_get; i++ )
            {
                if (pmtStreamArray[i])
                    NEXUS_Message_Close(pmtStreamArray[i]);
				if (pidChannelArray[i])
					close_msg_pidchannel(pidChannelArray[i]);
            }
            num_programs -= num_programs_to_get;
        }
    }

    free( pmtStreamArray );
	free (pidChannelArray);
    free(pmt_pidArray);

    return ret;
}


int tsPsi_getPMT(NEXUS_ParserBand parserBand, const void *p_patBfr, unsigned pat_bfrSize,
    unsigned program_number,  unsigned char *pmt, unsigned int *pmt_size, tsPsi_PMT_callback callback,void *context)
{
    int             i;
    size_t          bfrSize;
    int             num_programs;
    int             timeout;
    NEXUS_MessageHandle   pmtStreamArray;
    uint16_t        pmt_pidArray;
    TS_PAT_program  program;
	BERR_Code		ret;
    NEXUS_PidChannelHandle pidChannelArray;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;

    num_programs = TS_PAT_getNumPrograms(p_patBfr);
    BDBG_MSG(("num_programs %d", num_programs));

	for (i=0;i<num_programs;i++ )
	{
    	ret = TS_PAT_getProgram( p_patBfr, pat_bfrSize, i, &program );
		if (ret == BERR_SUCCESS)
		{
			if (program_number==0
				|| program.program_number == program_number) break;
		}
	}

	if (ret == BERR_SUCCESS && i<num_programs)
	{
	    NEXUS_Message_GetDefaultSettings(&settings);
		/* use polling for PMT*/
		settings.maxContiguousMessageSize = 4096;
        pmtStreamArray = NEXUS_Message_Open(&settings);

		pidChannelArray = open_msg_pidchannel( parserBand, program.PID);

	    NEXUS_Message_GetDefaultStartSettings(pmtStreamArray, &startSettings);
	    startSettings.pidChannel = pidChannelArray;

		startSettings.filter.mask[0] = 0x0;
		startSettings.filter.mask[2] = 0x0;
		startSettings.filter.mask[3] = 0x0;
		startSettings.filter.coefficient[0] = 0x2;
		startSettings.filter.coefficient[2] = (program.program_number & 0xFF00) >> 8;
		startSettings.filter.coefficient[3] = program.program_number & 0xFF;
		startSettings.filter.coefficient[15] = 0xff;
        BDBG_MSG(("filter pid %#x, program %#x", program.PID, program.program_number));

        pmt_pidArray = program.PID;

        if (NEXUS_Message_Start(pmtStreamArray, &startSettings)) {
            close_msg_pidchannel(pidChannelArray);
			NEXUS_PidChannel_Close(pidChannelArray);
            NEXUS_Message_Close(pmtStreamArray);
			return -1;
        }

        /* Now we have enabled our pid channels, so wait for each one to get some data */
        timeout = tsPsi_pmtTimeout;
        while( timeout != 0 )
        {
            /* Check each of the pid channels we are waiting for */
            const void *buffer;
            if (pmtStreamArray &&
                !NEXUS_Message_GetBuffer(pmtStreamArray, &buffer, &bfrSize ) && bfrSize)
            {
                /* don't call NEXUS_Message_ReadComplete() because we're stopping anyway */
                BDBG_MSG(("PMT: %d %d (%02x %02x %02x)", i, bfrSize,
                    ((unsigned char *)buffer)[0],((unsigned char *)buffer)[1],((unsigned char *)buffer)[2]));

                if (!TS_PMT_validate(buffer, bfrSize)) {
                    BDBG_WRN(("Invalid PMT data detected: ch %d, bfrSize 0x%x", i, bfrSize));
                    tsPsi_p_dumpBuffer(buffer, bfrSize);
                }
                else {
					if (pmt && pmt_size)
					{
						*pmt_size = bfrSize;
						if (bfrSize >= 4096)
							BDBG_ERR((" PMT size exceeds 4096 byte!"));
						else
							memcpy(pmt, buffer, bfrSize);
					}
                    /* Give the PMT to the callback */
                    (*callback)(context, pmt_pidArray, buffer, bfrSize);
                }

				break;
            } else
        	{
                BKNI_Sleep(2);
                timeout--;
            }
        }
        NEXUS_Message_Stop(pmtStreamArray);
        NEXUS_Message_Close(pmtStreamArray);
		close_msg_pidchannel(pidChannelArray);

		return BERR_SUCCESS;
    }

    return BERR_INVALID_PARAMETER;
}

static char atsc_id[4] = "GA94";
static char scte_id[4] = "SCTE";
void tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, PROGRAM_INFO_T *progInfo )
{
    int i,j;
	char *cc_ptr;
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
        case TS_PSI_DT_Registration:
			if (memcmp(atsc_id, &descriptor[2], 4) ==0)
				BDBG_WRN((" ATSC MPEG2 registration descriptor"));
			else if (memcmp(scte_id, &descriptor[2], 4) == 0)
				BDBG_WRN((" SCTE MPEG2 registration descriptor"));
			else
				BDBG_WRN((" unknown MPEG2 registration descriptor"));
            break;
		case TS_PSI_DT_ATSC_RedistributionControl:
			progInfo->broadcast_flag = true;
			break;
		case TS_PSI_DT_ATSC_CaptionService:
			progInfo->num_cc = descriptor[2]&0x1f;
			cc_ptr = (char*)&descriptor[3];
			for (j=0;j<progInfo->num_cc;j++)
			{
				memcpy(progInfo->cc[j].iso639,cc_ptr,3);
				cc_ptr +=3;
				progInfo->cc[j].ccType = *cc_ptr>>7;
				progInfo->cc[j].ccService = *cc_ptr&0x3f;
				cc_ptr += 3;
			}
			break;
		case TS_PSI_DT_ATSC_ContentAdvisory:
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
		case TS_PSI_DT_ATSC_ComponentName:
			break;
		case TS_PSI_DT_ATSC_AC3_Audio:
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

int tsPsi_getTimeoutCnt( int *patTimeoutCount, int *pmtTimeoutCount )
{
	*patTimeoutCount = patTimeoutCnt;
	*pmtTimeoutCount = pmtTimeoutCnt;
}

void tsPsi_resetTimeoutCnt( void )
{
	patTimeoutCnt = 0;
	pmtTimeoutCnt = 0;
}

static void tsPsi_p_dumpBuffer(const uint8_t *p_bfr, unsigned bfrSize)
{
    unsigned i;
    for (i=0;i<bfrSize;i++)
        BKNI_Printf("%02X", p_bfr[i]);
    BKNI_Printf("\n");
}
