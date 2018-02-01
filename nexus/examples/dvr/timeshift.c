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
 *****************************************************************************/
/* Nexus example app: timeshifting with FileFifo */

#include "nexus_platform.h"
#include <stdio.h>
#include <stdlib.h>
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_AUDIO && NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_file_fifo.h"
#include "nexus_file_fifo_chunk.h"
#include "nexus_record.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <glob.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(timeshift);

/* the following define the input file and its characteristics -- these will vary by input file */
/* don't use .mpg and .nav because a fifo file is not a regular TS or NAV file */
#define DATA_FILE_NAME "videos/stream.fifompg"
#define INDEX_FILE_NAME "videos/stream.fifonav"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x31
#define AUDIO_PID 0x34

#define PERMANENT_CHUNK_FILENAME "videos/chunk_stream.mpg"
#define PERMANENT_CHUNK_INDEXNAME "videos/chunk_stream.nav"

/* global app state makes the callbacks easier to implement. in a real app, this state
should be passed into the callbacks by reference. */
typedef struct b_app_context {
    enum {
        decode_state_stopped,
        decode_state_live,
        decode_state_playback
    } decode_state;
    int rate;
    unsigned fifoInterval;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackLoopMode beginningOfStreamAction;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
    NEXUS_FilePlayHandle playbackfile;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_RecordHandle record;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_ParserBand parserBand;
    uint32_t startAtTimestamp;
    NEXUS_FifoRecordHandle fifofile;
    NEXUS_ChunkedFifoRecordHandle chunkedFifofile;
    unsigned chunksize;
} b_app_context;

void jump_to_beginning(b_app_context *app)
{
    unsigned pos;
    NEXUS_Error rc;
    NEXUS_PlaybackStatus playbackStatus;

    rc = NEXUS_Playback_GetStatus(app->playback, &playbackStatus);
    BDBG_ASSERT(!rc);
    pos = playbackStatus.first;
    if (playbackStatus.last - playbackStatus.first > app->fifoInterval * 1000 / 2) {
        /* once we've reached half of the timeshift buffer, we should not jump to the absolute beginning.
        the beginning of the file is truncated in blocks.
        if we jump to the absolute beginning, we might get an undesired beginningOfStream action when that truncation occurs. */
        pos += 5000;
    }
    printf("Jump to beginning %u, %u...%u\n", pos, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
    (void)NEXUS_Playback_Seek(app->playback, pos);
}

static void stop_decode(b_app_context *app);

static void start_live_decode(b_app_context *app)
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    int rc;
    
    if (app->decode_state == decode_state_live) {
        return;
    }
    else if (app->decode_state != decode_state_stopped) {
        stop_decode(app);
    }
    
    app->videoPidChannel = NEXUS_PidChannel_Open(app->parserBand, VIDEO_PID, NULL);
    app->audioPidChannel = NEXUS_PidChannel_Open(app->parserBand, AUDIO_PID, NULL);

    NEXUS_StcChannel_GetSettings(app->stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = app->pcrPidChannel ? app->pcrPidChannel : app->videoPidChannel;
    rc = NEXUS_StcChannel_SetSettings(app->stcChannel, &stcSettings);
    BDBG_ASSERT(!rc);
    
    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = app->videoPidChannel;
    videoProgram.stcChannel = app->stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = app->audioPidChannel;
    audioProgram.stcChannel = app->stcChannel;

    /* Start decoders */
    rc = NEXUS_VideoDecoder_Start(app->videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_AudioDecoder_Start(app->audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);
    
    app->decode_state = decode_state_live;
}

static void start_playback_decode(b_app_context *app)
{
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    int rc;
    
    if (app->decode_state == decode_state_playback) {
        return;
    }
    else if (app->decode_state != decode_state_stopped) {
        stop_decode(app);
    }
    
    NEXUS_StcChannel_GetSettings(app->stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_StcChannel_SetSettings(app->stcChannel, &stcSettings);
    BDBG_ASSERT(!rc);
    
    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = app->videoDecoder;
    app->videoPidChannel = NEXUS_Playback_OpenPidChannel(app->playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = app->audioDecoder;
    app->audioPidChannel = NEXUS_Playback_OpenPidChannel(app->playback, AUDIO_PID, &playbackPidSettings);
    
    NEXUS_Playback_GetSettings(app->playback, &playbackSettings);
    playbackSettings.startPaused = (app->rate == 0);
    rc = NEXUS_Playback_SetSettings(app->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = app->videoPidChannel;
    videoProgram.stcChannel = app->stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = app->audioPidChannel;
    audioProgram.stcChannel = app->stcChannel;

    /* Start decoders */
    rc = NEXUS_VideoDecoder_Start(app->videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_AudioDecoder_Start(app->audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);

    if (app->record) {
        /* Linking Playback to Record allows Playback to sleep until Record writes data. Avoids a busyloop near live. */
        rc = NEXUS_Record_AddPlayback(app->record, app->playback);
        BDBG_ASSERT(!rc);
    }

    rc = NEXUS_Playback_Start(app->playback, app->playbackfile, NULL);
    BDBG_ASSERT(!rc);
    
    app->decode_state = decode_state_playback;
    
    if (app->startAtTimestamp) {
        /* we are starting from a stopped live decode state, so we need to seek */
        BDBG_MSG(("started at %#x", app->startAtTimestamp));
        NEXUS_Playback_Seek(app->playback, app->startAtTimestamp);
        app->startAtTimestamp = 0;
    }
}

static void stop_decode(b_app_context *app)
{
    if (app->decode_state == decode_state_stopped) {
        return;
    }
    
    if (app->decode_state == decode_state_playback) {
        NEXUS_Playback_Stop(app->playback);
        if (app->record) {
            NEXUS_Record_RemovePlayback(app->record, app->playback);
        }
    }
    else if (app->record) {
        NEXUS_RecordStatus recordStatus;
        NEXUS_Record_GetStatus(app->record, &recordStatus);
        app->startAtTimestamp = recordStatus.lastTimestamp;
        BDBG_MSG(("stopped live at %#x", app->startAtTimestamp));
    }
    
    NEXUS_VideoDecoder_Stop(app->videoDecoder);
    NEXUS_AudioDecoder_Stop(app->audioDecoder);
    
    if (app->decode_state == decode_state_playback) {
        NEXUS_Playback_ClosePidChannel(app->playback, app->videoPidChannel);
        NEXUS_Playback_ClosePidChannel(app->playback, app->audioPidChannel);
    }
    else {
        NEXUS_PidChannel_Close(app->videoPidChannel);
        NEXUS_PidChannel_Close(app->audioPidChannel);
    }
    
    app->decode_state = decode_state_stopped;
}

void beginning_of_stream(void *context, int param)
{
    b_app_context *app = context;
    BSTD_UNUSED(param);
    printf("beginning_of_stream: rate %d\n", app->rate);

    if (app->beginningOfStreamAction == NEXUS_PlaybackLoopMode_ePause) {
        if (app->rate >= 1) {
            /* if you get a bos action when going forward, you have just fallen off the beginning.
            use jump_to_beginning to gap more, then keep going. */
            start_playback_decode(app);
            jump_to_beginning(app);
            app->rate = 1;
            NEXUS_Playback_Play(app->playback);
        }
        else {
            app->rate = 0;
        }
    }
    else {
        app->rate = 1;
    }
}

void end_of_stream(void *context, int param)
{
    b_app_context *app = context;
    BSTD_UNUSED(param);
    printf("end_of_stream\n");
    if (app->rate) {
        app->rate = 1;
        start_live_decode(app);
    }
}

static void print_usage(void)
{
    printf(
    "timeshift\n"
    "-chunk SIZE       Use ChunkedFileFifo. SIZE in MB. If 0, use FileFifo.\n"
    "-interval MIN     Time for loop around. Default is 1 minute.\n"
    "-play             Do playback only from previously recorded fifo.\n"
    );
}

static void set_interval(b_app_context *app, unsigned interval)
{
    int rc;
    if (app->chunkedFifofile) {
        NEXUS_ChunkedFifoRecordSettings fifoRecordSettings;
        NEXUS_ChunkedFifoRecord_GetSettings(app->chunkedFifofile, &fifoRecordSettings);
        fifoRecordSettings.interval = interval;
        fifoRecordSettings.data.chunkSize = app->chunksize * 1024*1024;
        rc = NEXUS_ChunkedFifoRecord_SetSettings(app->chunkedFifofile, &fifoRecordSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        NEXUS_FifoRecordSettings fifoRecordSettings;
        NEXUS_FifoRecord_GetSettings(app->fifofile, &fifoRecordSettings);
        fifoRecordSettings.interval = interval;
        rc = NEXUS_FifoRecord_SetSettings(app->fifofile, &fifoRecordSettings);
        BDBG_ASSERT(!rc);
    }
    app->fifoInterval = interval;
}

static int glob_errfunc(const char *epath, int eerrno)
{
    BDBG_ERR(("glob_errfunc %s %d", epath, eerrno));
    return -1;
}

static void delete_chunked_file(const char *filename, const char *indexname)
{
    char path[64];
    int rc;
    glob_t glb;
    unsigned i;

    snprintf(path, sizeof(path), "%s_[0-9]*", filename);
    rc = glob(path, 0, glob_errfunc, &glb);
    if (rc) {
        /* could be no files, so no BER_TRACE */
        return;
    }
    for (i=0;i<glb.gl_pathc;i++) {
        BDBG_WRN(("deleting %s", glb.gl_pathv[i]));
        unlink(glb.gl_pathv[i]);

    }
    BDBG_WRN(("deleting %s", indexname));
    unlink(indexname);
    globfree(&glb);
}

static void query_chunk_filenames(char *filename, unsigned filenamesize, char *indexname, unsigned indexnamesize)
{
    printf("filename [%s] : ", PERMANENT_CHUNK_FILENAME);
    fgets(filename, filenamesize-1, stdin);
    filename[strlen(filename)-1] = 0;
    if (!filename[0]) strcpy(filename, PERMANENT_CHUNK_FILENAME);

    printf("indexname [%s] : ", PERMANENT_CHUNK_INDEXNAME);
    fgets(indexname, indexnamesize-1, stdin);
    indexname[strlen(indexname)-1] = 0;
    if (!indexname[0]) strcpy(indexname, PERMANENT_CHUNK_INDEXNAME);
}

static NEXUS_ChunkedFifoRecordExportHandle g_exportHandle;

static void stop_export_chunkedfifo(void)
{
    NEXUS_ChunkedFifoRecordExportStatus status;
    while (1) {
        NEXUS_ChunkedFifoRecord_GetExportStatus(g_exportHandle, &status);
        if (status.state >= NEXUS_ChunkedFifoRecordExportState_eFailed) break;
        BKNI_Sleep(250);
    }
    NEXUS_ChunkedFifoRecord_StopExport(g_exportHandle);
    g_exportHandle = NULL;
    if (status.state == NEXUS_ChunkedFifoRecordExportState_eFailed) {
        BDBG_ERR(("export failed"));
    }
    else {
        BDBG_WRN(("exported: timestamp %u..%u, chunk %u..%u, offset " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT,
            (unsigned)status.first.timestamp, (unsigned)status.last.timestamp,
            status.first.chunkNumber, status.last.chunkNumber,
            BDBG_UINT64_ARG(status.first.offset), BDBG_UINT64_ARG(status.last.offset)
            ));
    }
}

static void start_export_chunkedfifo(NEXUS_ChunkedFifoRecordHandle chunkedFifofile)
{
    char filename[128], indexname[128], timerange[64];
    NEXUS_ChunkedFifoRecordExportSettings settings;
    char *s;

    if (g_exportHandle) {
        stop_export_chunkedfifo();
    }

    NEXUS_ChunkedFifoRecord_GetDefaultExportSettings(&settings);

    query_chunk_filenames(filename, sizeof(filename), indexname, sizeof(indexname));
    settings.filename = filename;
    settings.indexname = indexname;

    settings.first.timestamp = 0 * 1000;
    settings.last.timestamp = 60 * 1000;
    printf("time in seconds [%u,%u] : ", settings.first.timestamp/1000, settings.last.timestamp/1000);
    fgets(timerange, sizeof(timerange), stdin);
    timerange[strlen(timerange)-1] = 0;
    s = strchr(timerange,',');
    if (s) {
        *s++ = 0;
        settings.first.timestamp = atoi(timerange) * 1000;
        settings.last.timestamp = atoi(s) * 1000;
    }

    /* deleting old chunks allows the playback_chunk autodetect of first chunk/chunk size to be reliable */
    delete_chunked_file(filename, indexname);

    /* NOTE: if chunkTemplate contains a subdir, app is responsible to mkdir first:
        strcpy(settings.chunkTemplate, "%s/chunk.%u%03u");
        mkdir(settings.filename);
    */
    strcpy(settings.chunkTemplate, "%s_%04u");
    g_exportHandle = NEXUS_ChunkedFifoRecord_StartExport(chunkedFifofile, &settings);
    if (g_exportHandle) {
        BDBG_WRN(("exporting to %s", filename));
    }
    else {
        BDBG_ERR(("unable to export"));
    }
}

int main(int argc, char **argv)
{
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_FileRecordHandle recordfile=NULL;
    NEXUS_RecpumpHandle recpump=NULL;
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannel[2];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_Error rc;
    const char *datafilename = DATA_FILE_NAME;
    const char *indexfilename = INDEX_FILE_NAME;
    b_app_context appinstance, *app = &appinstance;
    int curarg = 1;
    unsigned interval = 1;
    bool playbackOnly = false;
    bool exportOnly = false;
    bool initialExport = false;

    memset(app, 0, sizeof(*app));
    app->rate = 1;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-chunk") && curarg+1<argc) {
            app->chunksize = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-interval") && curarg+1<argc) {
            interval = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-play")) {
            playbackOnly = true;
        }
        else if (!strcmp(argv[curarg], "-export")) {
            exportOnly = true;
        }
        else if (!strcmp(argv[curarg], "-initial_export")) {
            initialExport = true;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }
    interval *= 60; /* convert to seconds */

    rc = NEXUS_Platform_Init(NULL);
    if (rc) return -1;
    
    if (exportOnly) {
        if (app->chunksize) {
            app->chunkedFifofile = NEXUS_ChunkedFifoRecord_ReOpenForExport(datafilename, indexfilename);
            if (app->chunkedFifofile) {
                set_interval(app, interval);
                start_export_chunkedfifo(app->chunkedFifofile);
                stop_export_chunkedfifo();
                NEXUS_FileRecord_Close(NEXUS_ChunkedFifoRecord_GetFile(app->chunkedFifofile));
                rc = 0;
            }
            else {
                rc = BERR_TRACE(-1);
            }
        }
        else {
            rc = BERR_TRACE(-1);
        }
        NEXUS_Platform_Uninit();
        return rc;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    /******************************
    * start record
    **/
    if (!playbackOnly) {
        app->parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(app->parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
        parserBandSettings.transportType = TRANSPORT_TYPE;
        NEXUS_ParserBand_SetSettings(app->parserBand, &parserBandSettings);

        recpump = NEXUS_Recpump_Open(0, NULL);
        app->record = NEXUS_Record_Create();
        NEXUS_Record_GetSettings(app->record, &recordSettings);
        recordSettings.recpump = recpump;
        NEXUS_Record_SetSettings(app->record, &recordSettings);

        if (app->chunksize) {
            app->chunkedFifofile = NEXUS_ChunkedFifoRecord_Create(datafilename, indexfilename);
            recordfile = NEXUS_ChunkedFifoRecord_GetFile(app->chunkedFifofile);
        }
        else {
            app->fifofile = NEXUS_FifoRecord_Create(datafilename, indexfilename);
            recordfile = NEXUS_FifoRecord_GetFile(app->fifofile);
        }
        set_interval(app, interval);
        if (initialExport) {
            start_export_chunkedfifo(app->chunkedFifofile);
            /* don't stop */
        }

        NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
        pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
        pidSettings.recpumpSettings.pidTypeSettings.video.codec = VIDEO_CODEC;

        pidChannel[0] = NEXUS_PidChannel_Open(app->parserBand, VIDEO_PID, NULL);
        rc = NEXUS_Record_AddPidChannel(app->record, pidChannel[0], &pidSettings);
        BDBG_ASSERT(!rc);

        pidChannel[1] = NEXUS_PidChannel_Open(app->parserBand, AUDIO_PID, NULL);
        rc = NEXUS_Record_AddPidChannel(app->record, pidChannel[1], NULL);
        BDBG_ASSERT(!rc);

        BDBG_WRN(("start record"));
        rc = NEXUS_Record_Start(app->record, recordfile);
        BDBG_ASSERT(!rc);
    }
    else {
        pidChannel[0] = pidChannel[1] = NULL;
    }

    /******************************
    * open playback, but don't start
    **/

    app->playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(app->playpump);
    app->playback = NEXUS_Playback_Create();
    BDBG_ASSERT(app->playback);

    if (app->chunksize) {
        app->playbackfile = NEXUS_ChunkedFifoPlay_Open(datafilename, indexfilename, app->chunkedFifofile);
    }
    else {
        app->playbackfile = NEXUS_FifoPlay_Open(datafilename, indexfilename, app->fifofile);
    }
    if (!app->playbackfile) {
        BDBG_ERR(("can't open files:%s %s", datafilename, indexfilename));
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.autoConfigTimebase = false; /* by default, NEXUS_StcChannelMode_eAuto would put the timebase into eFreeRun mode.
                                               autoConfigTimebase = false allows the app to configure the DPCR to track the PCR's rate, yet still do PVR TSM. */
    app->stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* PCR pid channel is from live parser band, even when timeshifting */
    app->pcrPidChannel = pidChannel[0];
    
    NEXUS_Timebase_GetSettings(stcSettings.timebase, &timebaseSettings);
    if (app->pcrPidChannel) {
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.sourceSettings.pcr.pidChannel = app->pcrPidChannel;
    }
    else {
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    }
    rc = NEXUS_Timebase_SetSettings(stcSettings.timebase, &timebaseSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetSettings(app->playback, &playbackSettings);
    playbackSettings.playpump = app->playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.beginningOfStreamCallback.callback = beginning_of_stream;
    playbackSettings.beginningOfStreamCallback.context = app;
#if 1
    /* switching to pause at the beginning is the more likely scenario, but it's a little harder to code.
    the app is responsible to gap the beginning correctly. */
    playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
#else
    /* when play hits the beginning, switch back to play. this is easier to implement in the app. */
    playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
#endif
    app->beginningOfStreamAction = playbackSettings.beginningOfStreamAction;
    playbackSettings.endOfStreamCallback.callback = end_of_stream;
    playbackSettings.endOfStreamCallback.context = app;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
    playbackSettings.timeshifting = true;
    playbackSettings.stcChannel = app->stcChannel;
    playbackSettings.stcTrick = true;
    rc = NEXUS_Playback_SetSettings(app->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    /* Bring up audio decoders and outputs */
    app->audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0]) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(app->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(app->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    app->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoDecoder_GetSettings(app->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
    NEXUS_VideoDecoder_SetSettings(app->videoDecoder, &videoDecoderSettings);
    
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(app->videoDecoder));

    if (playbackOnly) {
        start_playback_decode(app);
    }
    else {
        start_live_decode(app);
    }

#if 1
    for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        NEXUS_VideoDecoderStatus videoStatus;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;
        char cmd[16];
        unsigned i;
        bool quit=false;
        NEXUS_PlaybackTrickModeSettings trickmode_settings;

        printf("timeshift>");
        if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL) {
            break;
        }
        for(i=0;cmd[i]!='\0';i++) {
            switch(cmd[i]) {
            case '?':
                printf(
                "? - this help\n"
                "f - Fast Forward\n"
                "r - Rewind\n"
                "p - Play and Pause toggle\n"
                "a - frame Advance\n"
                "w - Wait 30msec\n"
                "+ - Jump forward 5 seconds\n"
                "- - Jump backward 5 seconds\n"
                "0 - Jump to the beginning\n"
                "9 - Jump to the end (catchup to live)\n"
                "x - Extend interval to MAX to stop looping\n"
                );
                if (app->chunksize) {
                printf(
                "s - Save permanent chunked file\n"
                "d - Delete permanent chunked file\n"
                );
                }
                printf(
                "q - Quit\n"
                );
                break;
            case 'p':
                if (app->rate == 0) {
                    bool stopped_live = (app->decode_state == decode_state_stopped);
                    printf( "play\n" );
                    app->rate = 1;
                    start_playback_decode(app);
                    if (!stopped_live) {
                        NEXUS_Playback_Play(app->playback);
                    }
                }
                else {
                    printf( "pause\n" );
                    app->rate = 0;
                    if (app->decode_state == decode_state_live) {
                        /* if live, just stop decode and keep the last picture visible. next operation will start playback. */
                        stop_decode(app);
                    }
                    else {
                        start_playback_decode(app);
                        NEXUS_Playback_Pause(app->playback);
                    }
                }
                break;
            case 'a':
                printf( "frame advance\n" );
                start_playback_decode(app);
                NEXUS_Playback_FrameAdvance(app->playback, app->rate >= 0);
                break;
            case 'f':
                start_playback_decode(app);
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(app->rate<=0) {
                    app->rate = 1;
                }
                app->rate *=2;
                trickmode_settings.rate = app->rate*NEXUS_NORMAL_PLAY_SPEED;
                printf( "fastward %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode(app->playback, &trickmode_settings );
                break;
            case 'r':
                start_playback_decode(app);
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(app->rate>=0) {
                    app->rate = -1;
                }
                else {
                    app->rate *=2;
                }
                trickmode_settings.rate = app->rate*NEXUS_NORMAL_PLAY_SPEED;
                printf( "rewind %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode(app->playback, &trickmode_settings );
                break;
            case 'w':
                BKNI_Sleep(30);
                break;
            case '-':
                start_playback_decode(app);
                rc = NEXUS_Playback_GetStatus(app->playback, &playbackStatus);
                BDBG_ASSERT(!rc);
                if (playbackStatus.position >= 5*1000) {
                    playbackStatus.position -= 5*1000;
                    /* it's normal for a Seek to fail if it goes past the beginning */
                    printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                    (void)NEXUS_Playback_Seek(app->playback, playbackStatus.position);
                }
                break;
            case '+':
                start_playback_decode(app);
                rc = NEXUS_Playback_GetStatus(app->playback, &playbackStatus);
                BDBG_ASSERT(!rc);
                /* it's normal for a Seek to fail if it goes past the end */
                playbackStatus.position += 5*1000;
                if (playbackStatus.position > playbackStatus.last) {
                    playbackStatus.position = playbackStatus.last;
                }
                printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                (void)NEXUS_Playback_Seek(app->playback, playbackStatus.position);
                break;
            case '0':
                {
                    bool startNormalPlay = (app->rate != 1);
                    app->rate = 1;
                    start_playback_decode(app);
                    jump_to_beginning(app);
                    if (startNormalPlay) {
                        NEXUS_Playback_Play(app->playback);
                    }
                }
                break;
            case '9':
                if (playbackOnly) {
                    /* TODO */
                }
                else {
                    app->rate = 1;
                    start_live_decode(app);
                }
                break;
            case '\n':
            case ' ':
                NEXUS_StcChannel_GetStc(app->stcChannel, &stc);
                NEXUS_VideoDecoder_GetStatus(app->videoDecoder, &videoStatus);
                NEXUS_AudioDecoder_GetStatus(app->audioDecoder, &audioStatus);

                printf("video %u%% %ux%u (%#x:%#x:%d), audio %u%% %uHz (%#x:%#x:%d)\n",
                    videoStatus.fifoSize ? (videoStatus.fifoDepth * 100) / videoStatus.fifoSize : 0,
                    videoStatus.source.width, videoStatus.source.height, videoStatus.pts, stc, videoStatus.ptsStcDifference,
                    audioStatus.fifoSize ? (audioStatus.fifoDepth * 100) / audioStatus.fifoSize : 0,
                    audioStatus.sampleRate, audioStatus.pts, stc, audioStatus.ptsStcDifference
                    );
                if (app->decode_state == decode_state_playback) {
                    rc = NEXUS_Playback_GetStatus(app->playback, &playbackStatus);
                    BDBG_ASSERT(!rc);
                    printf("playback rate=%u fifo=%u%% first=%u last=%u position=%u sec\n",
                        playbackStatus.trickModeSettings.rate,
                        playbackStatus.fifoSize ? (playbackStatus.fifoDepth * 100) / playbackStatus.fifoSize : 0,
                        (unsigned)playbackStatus.first/1000, (unsigned)playbackStatus.last/1000,
                        (unsigned)playbackStatus.position/1000);
                }
                if (app->record) {
                    NEXUS_FilePosition first, last;
                    if (app->chunksize) {
                        NEXUS_ChunkedFifoRecord_GetPosition(app->chunkedFifofile, &first, &last);
                    }
                    else {
                        NEXUS_FifoRecord_GetPosition(app->fifofile, &first, &last);
                    }
                    printf("record file %u:%u\n",
                        (unsigned)first.mpegFileOffset,  (unsigned)last.mpegFileOffset
                        );
                }
                break;
            case 'q':
                quit = true;
                break;
            case 'x':
                set_interval(app, 0xFFFFFFFF);
                break;
            case 's':
                if (app->chunksize) {
                    start_export_chunkedfifo(app->chunkedFifofile);
                    stop_export_chunkedfifo();
                }
                else {
                    BDBG_ERR(("save only supported with chunked fifo"));
                }
                break;
            case 'd':
                if (app->chunksize) {
                    char filename[128], indexname[128];
                    query_chunk_filenames(filename, sizeof(filename), indexname, sizeof(indexname));
                    delete_chunked_file(filename, indexname);
                }
                else {
                    BDBG_ERR(("save only supported with chunked fifo"));
                }
                break;
            default:
                break;
            }
        }
        if(quit)  {
            break;
        }


    }
#else
    printf("timeshifting playback started. press ENTER to stop.\n");
    getchar();
#endif

    /* stop decode & playback */
    stop_decode(app);
    if (app->record) {
        /* stop record */
        NEXUS_Record_Stop(app->record);
        NEXUS_Record_RemoveAllPidChannels(app->record);
        NEXUS_PidChannel_Close(pidChannel[0]);
        NEXUS_PidChannel_Close(pidChannel[1]);
    }
    NEXUS_FilePlay_Close(app->playbackfile);
    NEXUS_Playback_Destroy(app->playback);
    NEXUS_Playpump_Close(app->playpump);
    if (app->record) {
        NEXUS_FileRecord_Close(recordfile);
        NEXUS_Record_Destroy(app->record);
        NEXUS_Recpump_Close(recpump);
    }
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(app->videoDecoder));
    NEXUS_VideoDecoder_Close(app->videoDecoder);
    NEXUS_AudioDecoder_Close(app->audioDecoder);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
