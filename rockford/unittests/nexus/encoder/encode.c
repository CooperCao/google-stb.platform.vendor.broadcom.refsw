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
* API Description:
*
****************************************************************************/

#include "nexus_platform.h"
#if !NEXUS_HAS_VIDEO_ENCODER
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#else

#if  NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#include "nexus_audio_input.h"
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_video_encoder.h"
#include "nexus_video_encoder_output.h"
#include "nexus_audio_encoder.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_record.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_file_mux.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#include "nexus_core_utils.h"

#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "bfile_stdio.h"

#include "tshdrbuilder.h"
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "tspsimgr.h"

BDBG_MODULE(encode);


/***********************************
 *   PID assignments
 */
#define BTST_MUX_VIDEO_PID      (0x11)
#define BTST_MUX_AUDIO_PID      (0x12)
#define BTST_MUX_PCR_PID        (0x13)
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define BTST_PSI_QUEUE_CNT 4 /* every second insert a PSI, so it takes 4 secs to circle; hopefully its transfer won't be delay that far */

#define ENABLE_VIDEO_ENCODE 1
#define ENABLE_AUDIO_ENCODE 1

#define BTST_INPUT_MAX_WIDTH 416
#define BTST_INPUT_MAX_HEIGHT 224

#if NEXUS_NUM_HDMI_INPUTS
static uint8_t SampleEDID[] =
{
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x74, 0x22, 0x05, 0x01, 0x11, 0x20,
    0x00, 0x14, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
    0x17, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
    0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
    0x58, 0x2C, 0x25, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
    0x43, 0x4D, 0x37, 0x34, 0x32, 0x32, 0x2F, 0x37, 0x34, 0x32, 0x35, 0x0A, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x17, 0x3D, 0x0F, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x89,

    0x02, 0x03, 0x3C, 0x71, 0x7F, 0x03, 0x0C, 0x00, 0x40, 0x00, 0xB8, 0x2D, 0x2F, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE3, 0x05, 0x1F, 0x01, 0x49, 0x90, 0x05, 0x20, 0x04, 0x03, 0x02, 0x07,
    0x06, 0x01, 0x29, 0x09, 0x07, 0x01, 0x11, 0x07, 0x00, 0x15, 0x07, 0x00, 0x01, 0x1D, 0x00, 0x72,
    0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A,
    0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x18,
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x0B, 0x88, 0x21, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D
};
#endif

const char *infile="videos/avatar_AVC_15M.ts";
const char *outfile="videos/stream.mpg";
const char *outnavfile="videos/stream.nav";
const char *mp4File = "videos/stream.mp4";

NEXUS_StreamMuxHandle streamMux;

typedef struct TranscodeContext {
    /* PSI system data */
    void                     *pat[BTST_PSI_QUEUE_CNT];
    void                     *pmt[BTST_PSI_QUEUE_CNT];
    unsigned                  ccValue;
    NEXUS_StreamMuxSystemData psi[2];
    B_MutexHandle             mutexSystemdata;
    B_SchedulerHandle         schedulerSystemdata;
    B_SchedulerTimerId        systemdataTimer;
    B_ThreadHandle            schedulerThread;
    bool                      systemdataTimerIsStarted;
} TranscodeContext;

#define MAX_PROGRAMS_PER_FREQUENCY 24
#define NUM_PID_CHANNELS 4
typedef struct psiCollectionDataType
{
    NEXUS_ParserBand            parserBand;
    struct {
        uint16_t                    num;
        NEXUS_PidChannelHandle      channel;
    } pid[NUM_PID_CHANNELS];
} psiCollectionDataType,*pPsiCollectionDataType;

/* global context
 * function level context in this */
static TranscodeContext xcodeContext[NEXUS_NUM_VIDEO_ENCODERS];

struct opts_t {
    NEXUS_TransportType transportType;
    unsigned short videoPid, pcrPid, audioPid, pmtPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_AudioCodec audioCodec;
};

struct {
    NEXUS_VideoCodec nexus;
    bvideo_codec settop;
} g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_none},
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_unknown},
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eH264_Svc, bvideo_codec_h264_svc},
    {NEXUS_VideoCodec_eH264_Mvc, bvideo_codec_h264_mvc},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eRv40, bvideo_codec_rv40},
    {NEXUS_VideoCodec_eVp6, bvideo_codec_vp6},
    {NEXUS_VideoCodec_eVp8, bvideo_codec_vp8},
    {NEXUS_VideoCodec_eSpark, bvideo_codec_spark},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs}
};

struct {
    NEXUS_AudioCodec nexus;
    baudio_format settop;
} g_audioCodec[] = {
   {NEXUS_AudioCodec_eUnknown, baudio_format_unknown},
   {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
   {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
   {NEXUS_AudioCodec_eAac, baudio_format_aac},
   {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
   {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
   {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
   {NEXUS_AudioCodec_eAc3, baudio_format_ac3},
   {NEXUS_AudioCodec_eAc3Plus, baudio_format_ac3_plus},
   {NEXUS_AudioCodec_eDts, baudio_format_dts},
   {NEXUS_AudioCodec_eLpcmHdDvd, baudio_format_lpcm_hddvd},
   {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
   {NEXUS_AudioCodec_eDtsHd, baudio_format_dts_hd},
   {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
   {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
   {NEXUS_AudioCodec_eLpcmDvd, baudio_format_lpcm_dvd},
   {NEXUS_AudioCodec_eAvs, baudio_format_avs},
   {NEXUS_AudioCodec_eAmr, baudio_format_amr},
   {NEXUS_AudioCodec_eDra, baudio_format_dra},
   {NEXUS_AudioCodec_eCook, baudio_format_cook},
   {NEXUS_AudioCodec_ePcmWav, baudio_format_pcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_adpcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_dvi_adpcm}
};

struct {
    NEXUS_TransportType nexus;
    unsigned settop;
} g_mpegType[] = {
    {NEXUS_TransportType_eTs, bstream_mpeg_type_unknown},
    {NEXUS_TransportType_eEs, bstream_mpeg_type_es},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_bes},
    {NEXUS_TransportType_eMpeg2Pes, bstream_mpeg_type_pes},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_ts},
    {NEXUS_TransportType_eDssEs, bstream_mpeg_type_dss_es},
    {NEXUS_TransportType_eDssPes, bstream_mpeg_type_dss_pes},
    {NEXUS_TransportType_eVob, bstream_mpeg_type_vob},
    {NEXUS_TransportType_eAsf, bstream_mpeg_type_asf},
    {NEXUS_TransportType_eAvi, bstream_mpeg_type_avi},
    {NEXUS_TransportType_eMpeg1Ps, bstream_mpeg_type_mpeg1},
    {NEXUS_TransportType_eMp4, bstream_mpeg_type_mp4},
    {NEXUS_TransportType_eMkv, bstream_mpeg_type_mkv},
    {NEXUS_TransportType_eWav, bstream_mpeg_type_wav},
    {NEXUS_TransportType_eMp4Fragment, bstream_mpeg_type_mp4_fragment},
    {NEXUS_TransportType_eRmff, bstream_mpeg_type_rmff},
    {NEXUS_TransportType_eFlv, bstream_mpeg_type_flv},
    {NEXUS_TransportType_eOgg, bstream_mpeg_type_ogg}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i=0;i<sizeof(g_struct)/sizeof(g_struct[0]);i++) { \
        if (g_struct[i].settop == settop_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", settop_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value)
{
    CONVERT(g_videoCodec);
}

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value)
{
    CONVERT(g_audioCodec);
}

NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value)
{
    CONVERT(g_mpegType);
}

#undef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))

/* struct used to keep track of the si callback we must use, to notify
   the si applib that it's previously requested data is now available */
typedef struct siRequest_t
{
    B_PSIP_DataReadyCallback   callback;
    void                   * callbackParam;
} siRequest_t;

/* we only have one filterHandle (i.e. msg), so we only have to keep track
   of the current request from si (specifically, the si "data ready"
   callback and associated param) */
static siRequest_t g_siRequest;

/* nexus message "data ready" callback - forward notification to si lib */
static void DataReady(void * context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    if (NULL != g_siRequest.callback) {
        /* printf("in DataReady callback - forward notification to psip lib\n");*/
        g_siRequest.callback(g_siRequest.callbackParam);
    }
}

static void SiDataReady(B_Error status, void * context)
{
    /* printf("in APP DataReady callback - psip lib has finished processing data\n");*/
    if (B_ERROR_SUCCESS == status) {
        BKNI_SetEvent((BKNI_EventHandle)context);
    } else {
        BDBG_WRN(("problem receiving data from api call - error code:%d\n", status));
        /* set event so our test app can continue... */
        BKNI_SetEvent((BKNI_EventHandle)context);
    }
}

static void cpyFilter(NEXUS_MessageFilter * pNexusFilter, B_PSIP_Filter * pSiFilter)
{
    int i = 0;

    for (i = 0; i < MIN(B_PSIP_FILTER_SIZE, NEXUS_MESSAGE_FILTER_SIZE); i++)
    {
        if (i == 2) {
            /* WARNING: will not filter message byte 2! */
            continue;
        } else
            if (i >= 2) {
                /* adjust nexus index see NEXUS_MessageFilter in nexus_message.h */
                pNexusFilter->mask[i-1]        = pSiFilter->mask[i];
                pNexusFilter->coefficient[i-1] = pSiFilter->coef[i];
                pNexusFilter->exclusion[i-1]   = pSiFilter->excl[i];
            } else {
                pNexusFilter->mask[i]        = pSiFilter->mask[i];
                pNexusFilter->coefficient[i] = pSiFilter->coef[i];
                pNexusFilter->exclusion[i]   = pSiFilter->excl[i];
            }
    }
}

/* gets a pidchannel - nexus handles pid channel reuse automatically */
NEXUS_PidChannelHandle OpenPidChannel(psiCollectionDataType * pCollectionData, uint16_t pid)
{
    int    i = 0;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidChannelSettings;

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidChannelSettings);

    playpumpPidChannelSettings.pidSettings.requireMessageBuffer = true;
    pidChannelSettings.requireMessageBuffer = true;
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);

    for (i = 0; i < NUM_PID_CHANNELS; i++)
    {
        if (NULL == pCollectionData->pid[i].channel) {
            BDBG_MSG(("open pidChannel for pid:0x%04x\n", pid));
            pCollectionData->pid[i].num = pid;
        pCollectionData->pid[i].channel =
        NEXUS_PidChannel_Open(pCollectionData->parserBand, pid, &pidChannelSettings);
            return pCollectionData->pid[i].channel;
        }
    }

    if (i == NUM_PID_CHANNELS) {
        BDBG_WRN(("failed to open pid channel:0x%04x - not enough storage space in pCollectionData!\n", pid));
    }

    return NULL;
}

/* closes a previously opened pid channel */
void ClosePidChannel(psiCollectionDataType * pCollectionData, uint16_t pid)
{
    int i = 0;
    bool found=false;

    for (i = 0; i < NUM_PID_CHANNELS; i++)
    {
        /* find pid to close */
        if (( pCollectionData->pid[i].num == pid) &&
            ( pCollectionData->pid[i].channel != NULL)) {
            BDBG_MSG(("close pidChannel for pid:0x%04x\n", pid));

        NEXUS_PidChannel_Close(pCollectionData->pid[i].channel);
            pCollectionData->pid[i].channel = NULL;
            pCollectionData->pid[i].num = 0;
            found = true;
            break;
        }
    }
    if (!found)
        BDBG_WRN(("failure closing pid channel:0x%04x - not found in list\n", pid));
}

void StartMessageFilter(psiCollectionDataType * pCollectionData, B_PSIP_CollectionRequest * pRequest, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_MessageHandle          msg;
    NEXUS_MessageStartSettings   msgStartSettings;

    BSTD_UNUSED(pCollectionData);
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    /* printf("StartMessageFilter\n");*/

    NEXUS_Message_GetDefaultStartSettings(msg, &msgStartSettings);
    msgStartSettings.bufferMode = NEXUS_MessageBufferMode_eOneMessage;
    msgStartSettings.pidChannel = pidChannel;
    cpyFilter(&msgStartSettings.filter, &pRequest->filter);

    NEXUS_Message_Start(msg, &msgStartSettings);
    NEXUS_StartCallbacks(msg);
}

void StopMessageFilter(psiCollectionDataType * pCollectionData, B_PSIP_CollectionRequest * pRequest)
{
    NEXUS_MessageHandle          msg;
    BSTD_UNUSED(pCollectionData);

    /* printf("StopMessageFilter\n"); */
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    NEXUS_StopCallbacks(msg);
    NEXUS_Message_Stop(msg);
}

/* callback function called by si applib to collect si data */
static B_Error CollectionFunc(B_PSIP_CollectionRequest * pRequest, void * context)
{

    NEXUS_PidChannelHandle pidChannel = NULL;
    pPsiCollectionDataType  pCollectionData = (pPsiCollectionDataType)context;
    NEXUS_MessageHandle          msg;

    BDBG_ASSERT(NULL != pRequest);
    BDBG_ASSERT(NULL != context);
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    if (NULL == msg) {
        BDBG_WRN(("invalid filterHandle received from SI applib!\n"));
        return B_ERROR_INVALID_PARAMETER;
    }

    switch (pRequest->cmd)
    {
        const uint8_t  * buffer;
        size_t             size;

    case B_PSIP_eCollectStart:
        BDBG_MSG(("-=-=- B_PSIP_eCollectStart -=-=-\n"));

        /*
         * Save off pRequest->callback and pRequest->callbackParam.
         * these need to be called when DataReady() is called.  this will
         * notify the si lib that it can begin processing the received data.
         * Si applib only allows us to call one API at a time (per filterHandle),
         * so we only have to keep track of the latest siRequest.callback
         * and siRequest.callbackParam (for our one and only filterHandle).
         */
        g_siRequest.callback      = pRequest->callback;
        g_siRequest.callbackParam = pRequest->callbackParam;

        pidChannel = OpenPidChannel(pCollectionData, pRequest->pid);
        StartMessageFilter(pCollectionData, pRequest, pidChannel);
        break;

    case B_PSIP_eCollectStop:
        BDBG_MSG(("-=-=- B_PSIP_eCollectStop -=-=-\n"));
        StopMessageFilter(pCollectionData, pRequest);
        ClosePidChannel(pCollectionData, pRequest->pid);
        break;

    case B_PSIP_eCollectGetBuffer:
        BDBG_MSG(("-=-=- B_PSIP_eCollectGetBuffer -=-=-\n"));
        /* fall through for now... */

    case B_PSIP_eCollectCopyBuffer:
        /*printf("-=-=- B_PSIP_eCollectCopyBuffer -=-=-\n");*/
        if (NEXUS_SUCCESS == NEXUS_Message_GetBuffer(msg, (const void **)&buffer, &size)) {
            if (0 < size) {
                BDBG_MSG(("NEXUS_Message_GetBuffer() succeeded! size:"BDBG_UINT64_FMT"\n",BDBG_UINT64_ARG((uint64_t)size)));
                memcpy(pRequest->pBuffer, buffer, *(pRequest->pBufferLength)); /* copy valid data to request buffer */
                *(pRequest->pBufferLength) = MIN(*(pRequest->pBufferLength), size);
                NEXUS_Message_ReadComplete(msg, size);
            } else {
                /* NEXUS_Message_GetBuffer can return size==0 (this is normal
                 * operation).  We will simply wait for the next data ready
                 * notification by returning a B_ERROR_RETRY to the PSIP applib
                 */
                NEXUS_Message_ReadComplete(msg, size);
                return B_ERROR_PSIP_RETRY;
            }
        }
        else {
            BDBG_WRN(("NEXUS_Message_GetBuffer() failed\n"));

            return B_ERROR_UNKNOWN;
        }
        break;

    default:
        BDBG_WRN(("-=-=- invalid Command received:%d -=-=-\n", pRequest->cmd));
        return B_ERROR_INVALID_PARAMETER;
        break;
    }

    return B_ERROR_SUCCESS;
}

static void convertStreamToPsi( TS_PMT_stream *stream, struct opts_t *psi)
{
    psi->transportType = NEXUS_TransportType_eTs;
    switch (stream->stream_type) {
        case TS_PSI_ST_13818_2_Video:
        case TS_PSI_ST_ATSC_Video:
            psi->videoCodec = NEXUS_VideoCodec_eMpeg2;
            psi->videoPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_11172_2_Video:
            psi->videoCodec = NEXUS_VideoCodec_eMpeg1;
            psi->videoPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_14496_10_Video:
            psi->videoCodec = NEXUS_VideoCodec_eH264;
            psi->videoPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_11172_3_Audio:
        case TS_PSI_ST_13818_3_Audio:
            psi->audioCodec = NEXUS_AudioCodec_eMpeg;
            psi->audioPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_ATSC_AC3:
            psi->audioCodec = NEXUS_AudioCodec_eAc3;
            psi->audioPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_ATSC_EAC3:
            psi->audioCodec = NEXUS_AudioCodec_eAc3Plus;
            psi->audioPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_13818_7_AAC:
            psi->audioCodec = NEXUS_AudioCodec_eAac;
            psi->audioPid = stream->elementary_PID;
            break;
        case TS_PSI_ST_14496_3_Audio:
            psi->audioCodec = NEXUS_AudioCodec_eAacPlus;
            psi->audioPid = stream->elementary_PID;
            break;
        default:
            BDBG_WRN(("###### TODO: Unknown stream type: %x #####", stream->stream_type));
    }
}

static void
tsPsi_procProgDescriptors( const uint8_t *p_bfr, unsigned bfrSize, PROGRAM_INFO_T *progInfo )
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

static void
tsPsi_procStreamDescriptors( const uint8_t *p_bfr, unsigned bfrSize, int streamNum, EPID *ePidData )
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
void acquirePsiInfo(pPsiCollectionDataType pCollectionData, struct opts_t *psi, int *numProgramsFound)
{

    B_Error                    errCode;
    NEXUS_MessageSettings      settings;
    NEXUS_MessageHandle        msg = NULL;
    B_PSIP_Handle si = NULL;
    B_PSIP_Settings si_settings;
    B_PSIP_ApiSettings settingsApi;
    TS_PMT_stream stream;
    TS_PAT_program program;
    uint8_t  i,j;
    uint8_t  *buf = NULL;
    uint32_t bufLength = 4096;
    uint8_t  *bufPMT = NULL;
    uint32_t bufPMTLength = 4096;


    BKNI_EventHandle dataReadyEvent = NULL;

    PROGRAM_INFO_T programInfo;
    EPID epid;
    BKNI_Memset(&programInfo, 0, sizeof(programInfo));


    buf = BKNI_Malloc(bufLength);
    bufPMT = BKNI_Malloc(bufPMTLength);
    if (buf == NULL || bufPMT == NULL) { BDBG_ERR(("BKNI_Malloc Failure at %d", __LINE__)); goto error;}
    /* Start stream  */
    *numProgramsFound = 0;


    if (BKNI_CreateEvent(&dataReadyEvent) != BERR_SUCCESS) { BDBG_ERR(("Failed to create PSI dataReadyEvent \n")); goto error; }

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = DataReady;
    settings.dataReady.context = NULL;
    msg = NEXUS_Message_Open(&settings);
    if (!msg) { BDBG_ERR(("PSI - NEXUS_Message_Open failed\n")); goto error; }

    B_PSIP_GetDefaultSettings(&si_settings);
    si_settings.PatTimeout = 500;
    si_settings.PmtTimeout = 500;
    si = B_PSIP_Open(&si_settings, CollectionFunc, pCollectionData);
    if (!si) { BDBG_ERR(("PSI - Unable to open SI applib\n")); goto error; }

    BDBG_MSG(("starting PSI gathering\n"));
    errCode = B_PSIP_Start(si);
    if ( errCode ) { BDBG_ERR(("PSI - Unable to start SI data collection, err %d\n", errCode)); goto error; }


    B_PSIP_GetDefaultApiSettings(&settingsApi);
    settingsApi.siHandle                 = si;
    settingsApi.filterHandle             = msg;
    settingsApi.dataReadyCallback        = SiDataReady;
    settingsApi.dataReadyCallbackParam   = dataReadyEvent;
    settingsApi.timeout                  = 65000;

    memset(buf, 0, bufLength);
    BKNI_ResetEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam);
    if (B_ERROR_SUCCESS != B_PSIP_GetPAT(&settingsApi, buf, &bufLength)) {
        BDBG_ERR(("B_PSIP_GetPAT() failed\n"));
        goto error;
    }

    /* wait for async response from si - wait on dataReadyEvent */
    BKNI_WaitForEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam, BKNI_INFINITE);
    BDBG_MSG(("received response from SI, len = %d!\n", bufLength));

    if (0 < bufLength) {
        BDBG_MSG(("PAT Programs found = %d\n", TS_PAT_getNumPrograms(buf)));
        for (i = 0; i<MAX_PROGRAMS_PER_FREQUENCY && (TS_PAT_getProgram(buf, bufLength, i, &program)==BERR_SUCCESS); i++)
        {
            memset(psi, 0, sizeof(*psi));
            BDBG_MSG(("program_number: %d, i %d, PID: 0x%04X, psi %p, sizeof psi %d\n", program.program_number, i, program.PID, (void*)psi, (unsigned)sizeof(*psi)));
            psi->pmtPid = program.PID;

            BKNI_ResetEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam);
            memset(bufPMT, 0, bufPMTLength);
            if (B_ERROR_SUCCESS != B_PSIP_GetPMT(&settingsApi, program.PID, bufPMT, &bufPMTLength)) {
                BDBG_ERR(("B_PSIP_GetPMT() failed\n"));
                continue;
            }
            /* wait for async response from si - wait on dataReadyEvent */
            BKNI_WaitForEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam, BKNI_INFINITE);
            BDBG_MSG(("received PMT: size:%d, # of streams in this program %d\n", bufPMTLength, TS_PMT_getNumStreams(bufPMT, bufPMTLength)));

            /* find and process Program descriptors */
            tsPsi_procProgDescriptors(bufPMT, bufPMTLength, &programInfo);
            if (programInfo.ca_pid) {
                BDBG_WRN(("Program # %d, pid 0x%x is encrypted, ca_pid 0x%x, ignore it", program.program_number, program.PID, programInfo.ca_pid));
                continue;
            }

            if (0 < bufPMTLength) {
                for (j = 0; j < TS_PMT_getNumStreams(bufPMT, bufPMTLength); j++)
                {
                    memset(&stream, 0, sizeof(stream));
                    TS_PMT_getStream(bufPMT, bufPMTLength, j, &stream);
                    BDBG_MSG(("j %d, stream_type:0x%02X, PID:0x%04X\n", j, stream.stream_type, stream.elementary_PID));
                    convertStreamToPsi( &stream, psi);

                    memset(&epid, 0, sizeof(epid));
                    tsPsi_procStreamDescriptors(bufPMT, bufPMTLength, j, &epid);
                    if (epid.ca_pid) {
                        BDBG_WRN(("program 0x%x has ca pid 0x%x, ignore it", program.PID, epid.ca_pid));
                        break;
                    }

                    psi->pcrPid = TS_PMT_getPcrPid(bufPMT, bufPMTLength);
                    if ((psi->videoPid ) && (psi->audioPid )) {
                        (*numProgramsFound)++;
                        BDBG_WRN(("Found %d program, vpid %d, vcodec %d, apid %d, acodec %d", *numProgramsFound, psi->videoPid, psi->videoCodec, psi->audioPid, psi->audioCodec));
                        psi += 1;
                        break;
                    }
                    else if (j == (TS_PMT_getNumStreams(bufPMT, bufPMTLength)-1)) {
                        /* last stream in the program */
                        if ((psi->videoPid ) || (psi->audioPid )) {
                            (*numProgramsFound)++;
                            BDBG_WRN(("Found %d program, vpid %d or apid %d", *numProgramsFound, psi->videoPid, psi->audioPid));
                            psi += 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    BDBG_MSG(("stopping PSI gathering\n"));



    /* cleanup */
    B_PSIP_Stop(si);
error:
    if (buf)
        BKNI_Free(buf);
    if (bufPMT)
        BKNI_Free(bufPMT);
    if (si)
        B_PSIP_Close(si);
    if (msg)
        NEXUS_Message_Close(msg);
    if (dataReadyEvent)
        BKNI_DestroyEvent(dataReadyEvent);
}

int probe(struct opts_t *opts, const char *filename, const char **indexname)
{
    int rc = 0;

    /* use media probe to set values */
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    FILE *fin;
    char stream_info[512];

    probe = bmedia_probe_create();

    opts->videoCodec = NEXUS_VideoCodec_eUnknown;
    opts->audioCodec = NEXUS_AudioCodec_eUnknown;


    fin = fopen64(filename,"rb");
    if (!fin) {
    printf("can't open media file '%s' for probing\n", filename);
    rc = -1;
    goto done;
    }

    fd = bfile_stdio_read_attach(fin);


    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = filename;
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    bfile_stdio_read_detach(fd);

    fclose(fin);
    if(!stream) {
    printf("media probe can't parse stream '%s'\n", filename);
    rc = -1;
    goto done;
    }

    /* if the user has specified the index, don't override */
    if (indexname && !*indexname) {
    if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required) {
        *indexname = filename;
    }
    }

    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    printf(
    "Media Probe:\n"
    "%s\n\n",
    stream_info);

    opts->transportType = b_mpegtype2nexus(stream->type);


    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
    switch(track->type) {
    case bmedia_track_type_audio:
        if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
        opts->audioPid = track->number;
        opts->audioCodec = b_audiocodec2nexus(track->info.audio.codec);
        foundAudio = true;
        }
        break;
    case bmedia_track_type_video:
        if(track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc) {
        break;
        } else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
        opts->videoPid = track->number;
        opts->videoCodec = b_videocodec2nexus(track->info.video.codec);
        foundVideo = true;
        }
        break;
    case bmedia_track_type_pcr:
        opts->pcrPid = track->number;
        break;
    default:
        break;
    }
    }

done:
    if (probe) {
    if (stream) {
        bmedia_probe_stream_free(probe, stream);
    }
    bmedia_probe_destroy(probe);
    }

    return rc;
}

static void
transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189
void add_pat_pmt(
    TranscodeContext *pContext,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    uint8_t  vidStreamType,
    uint8_t  audStreamType
)
{
    uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;

    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, BTST_MUX_PMT_PID);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if(audPid)
    {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pat[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pat[0] + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pmt[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pmt[0] + buf_used, pmt_pl_buf, pmt_pl_size);

}

static void insertSystemDataTimer(void *context)
{
    TranscodeContext *pContext = context;
    uint8_t ccByte;

    ++pContext->ccValue;/* increment CC synchronously with PAT/PMT */
    ccByte = *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PAT pointer */
    pContext->psi[0].pData = (void*)((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 1);

    ccByte = *((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4);
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PMT pointer */
    pContext->psi[1].pData = (void*)((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 1);

    NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &pContext->psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &pContext->psi[1]);
    BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) & 0xf,
        *((uint8_t*)pContext->pmt[pContext->ccValue  % BTST_PSI_QUEUE_CNT] + 4) & 0xf));
    if(pContext->systemdataTimerIsStarted)
    {
        pContext->systemdataTimer = B_Scheduler_StartTimer(
            pContext->schedulerSystemdata,pContext->mutexSystemdata, 1000, insertSystemDataTimer, pContext);
        if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error %d", NEXUS_OUT_OF_SYSTEM_MEMORY));}
    }
    return;
}

/*******************************
 * Add system data to stream_mux
 */
static void xcode_set_systemdata( TranscodeContext  *pContext, NEXUS_VideoCodec encoderVideoCodec, NEXUS_AudioCodec encoderAudioCodec)
{
    uint8_t vidStreamType=0, audStreamType=0;
    unsigned i;

    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pContext->pat[i]);
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pContext->pmt[i]);
    }

#if ENABLE_VIDEO_ENCODE
    /* decide the stream type to set in PMT */
    switch(encoderVideoCodec)
    {
        case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
        case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
        case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
        case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
        default:
        BDBG_ERR(("Video encoder codec %d is not supported!\n", encoderVideoCodec));
    }
#else
    BSTD_UNUSED(encoderVideoCodec);
#endif

#if ENABLE_AUDIO_ENCODE
    switch(encoderAudioCodec)
    {
        case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
        default:
        BDBG_ERR(("Audio encoder codec %d is not supported!\n", encoderAudioCodec));
    }
#else
    BSTD_UNUSED(encoderAudioCodec);
#endif

    add_pat_pmt(pContext, BTST_MUX_PCR_PID, BTST_MUX_VIDEO_PID, BTST_MUX_AUDIO_PID, vidStreamType, audStreamType);
    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        if(i > 0)
        {
            BKNI_Memcpy(pContext->pat[i], pContext->pat[0], BTST_TS_HEADER_BUF_LENGTH);
            BKNI_Memcpy(pContext->pmt[i], pContext->pmt[0], BTST_TS_HEADER_BUF_LENGTH);
        }
        NEXUS_Memory_FlushCache(pContext->pat[i], BTST_TS_HEADER_BUF_LENGTH);
        NEXUS_Memory_FlushCache(pContext->pmt[i], BTST_TS_HEADER_BUF_LENGTH);
    }
    BKNI_Memset(pContext->psi, 0, sizeof(pContext->psi));
    pContext->psi[0].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[0].pData = (void*)((uint8_t*)pContext->pat[0] + 1);
    pContext->psi[0].timestampDelta = 0;
    pContext->psi[1].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[1].pData = (void*)((uint8_t*)pContext->pmt[0] + 1);
    pContext->psi[1].timestampDelta = 0;
    NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &pContext->psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &pContext->psi[1]);
    BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->pat[0] + 4) & 0xf,
        *((uint8_t*)pContext->pmt[0] + 4) & 0xf));

    /* schedule a periodic timer to insert PAT/PMT */
    {
    B_ThreadSettings settingsThread;
    pContext->mutexSystemdata = B_Mutex_Create(NULL);
    pContext->schedulerSystemdata = B_Scheduler_Create(NULL);
    /* create thread to run scheduler */
    B_Thread_GetDefaultSettings(&settingsThread);
    pContext->schedulerThread = B_Thread_Create("systemdata_Scheduler",
        (B_ThreadFunc)B_Scheduler_Run,
        pContext->schedulerSystemdata,
        &settingsThread);

    if (NULL == pContext->schedulerThread)
    {
        BDBG_ERR(("failed to create scheduler thread"));
    }
    pContext->systemdataTimer = B_Scheduler_StartTimer(
        pContext->schedulerSystemdata,pContext->mutexSystemdata, 1000, insertSystemDataTimer, pContext);
    if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error"));}
    pContext->systemdataTimerIsStarted = true;
    }
}

static void xcode_stop_systemdata( TranscodeContext  *pContext )
{
    /* cancel system data timer */
    if(pContext->systemdataTimerIsStarted)
    {
        B_Scheduler_CancelTimer(pContext->schedulerSystemdata, pContext->systemdataTimer);
        B_Scheduler_Stop(pContext->schedulerSystemdata);
        B_Scheduler_Destroy(pContext->schedulerSystemdata);
        if (pContext->schedulerThread)
        {
            B_Thread_Destroy(pContext->schedulerThread);
            pContext->schedulerThread = NULL;
        }
        B_Mutex_Destroy(pContext->mutexSystemdata);
        pContext->systemdataTimer = NULL;
        pContext->systemdataTimerIsStarted = false;
    }

}

#if  NEXUS_HAS_FRONTEND
typedef struct GenericCallbackContext
{
   void *context;
   void* handle;
} GenericCallbackContext;

static void qam_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)((GenericCallbackContext*)context)->context;
    NEXUS_FrontendQamStatus qamStatus;
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)((GenericCallbackContext*)context)->handle;

    fprintf(stderr, "Lock callback, frontend %p\n", (void*)frontend);

    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)frontend,
            qamStatus.fecLock, qamStatus.receiverLock);

    BKNI_SetEvent(statusEvent);
}

static void sat_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)((GenericCallbackContext*)context)->context;
    NEXUS_FrontendSatelliteStatus status;
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)((GenericCallbackContext*)context)->handle;

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %d\n", status.demodLocked);

    BKNI_SetEvent(statusEvent);
}
#endif

int main(int argc, char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
#if ENABLE_VIDEO_ENCODE
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplaySettings displaySettings;

    NEXUS_DisplayHandle display, displayTranscode;
    NEXUS_VideoWindowHandle window, windowTranscode;
    NEXUS_VideoFormatInfo videoInfo;

    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoWindowSettings windowSettings;

    NEXUS_VideoDecoderHandle videoDecoder;

    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PidChannelHandle pidChannelTranscodeVideo;
    NEXUS_RecordPidChannelSettings recordPidSettings;
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio;
    NEXUS_PidChannelHandle pidChannelTranscodeAudio;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_AudioCodec audioCodec;
#endif
#if NEXUS_NUM_HDMI_INPUTS
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
#endif

#if  NEXUS_HAS_FRONTEND
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    GenericCallbackContext qamLockCallbackContext;
    GenericCallbackContext satLockCallbackContext;
#endif
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;

    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodePcr;
    NEXUS_FileRecordHandle fileTranscode;
    NEXUS_StcChannelHandle stcChannelTranscode;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodePat;
    NEXUS_PidChannelHandle pidChannelTranscodePmt;
#if NEXUS_HAS_SYNC_CHANNEL && ENABLE_AUDIO_ENCODE
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#endif


    NEXUS_FileMuxHandle fileMux;
    NEXUS_MuxFileHandle muxFileOutput;
    NEXUS_FileMuxCreateSettings fileMuxCreateSettings;
    NEXUS_FileMuxStartSettings fileMuxConfig;

    TranscodeContext *pContext = &xcodeContext[0];

    BKNI_EventHandle signalLockedEvent=NULL;
    bool simulDisplay = true;

    struct opts_t opts;

    unsigned width=BTST_INPUT_MAX_WIDTH, height=BTST_INPUT_MAX_HEIGHT, framerate=2997, bitrate=400000;
    unsigned freq=765, qammode=64;
#if NEXUS_HAS_FRONTEND
    unsigned i;
#endif
    bool qam=false, sat=false, hdmi=false, streamer=false, encodeUserData=false, mp4=false;
    int curarg=0;
    bool b720pEncode=false;
    bool xcodeSyncLock = false;

    while (++curarg < argc) {
        if (!strcmp(argv[curarg], "--help")) {
            printf(
        "Usage: nexus encoder [-in] [-width] [-height] [-framerate] [-bitrate] [-noDisplay] \n"
        "\n"
        );
            return 0;
        }
        else if (!strcmp(argv[curarg], "-width") && curarg+1 < argc) {
            width = atoi(argv[++curarg]);
           BDBG_ERR(( "W:%d H:%d\n", width, height));

        }
    else if (!strcmp(argv[curarg], "-height") && curarg+1 < argc) {
            height = atoi(argv[++curarg]);
                BDBG_ERR(( "W:%d H:%d\n", width, height));
        }
    else if (!strcmp(argv[curarg], "-framerate") && curarg+1 < argc) {
            framerate = atoi(argv[++curarg]);
        }
    else if (!strcmp(argv[curarg], "-bitrate") && curarg+1 < argc) {
            bitrate = atoi(argv[++curarg]);
        }
    else if (!strcmp(argv[curarg], "-in") && curarg+1 < argc) {
            infile = argv[++curarg];
        }
    else if (!strcmp(argv[curarg], "-freq") && curarg+1 < argc) {
             freq = atoi(argv[++curarg]);
        }
    else if (!strcmp(argv[curarg], "-mode") && curarg+1 < argc) {
              qammode = atoi(argv[++curarg]);
        }
    else if (!strcmp(argv[curarg], "-qam")) {
        qam = true;
        }
    else if (!strcmp(argv[curarg], "-sat")) {
        sat = true;
        }
    else if (!strcmp(argv[curarg], "-streamer")) {
        streamer = true;
        }
    else if (!strcmp(argv[curarg], "-hdmi")) {
        hdmi = true;
        }
    else if (!strcmp(argv[curarg], "-userdata")) {
        encodeUserData = true;
        }
    else if (!strcmp(argv[curarg], "-mp4")) {
        mp4 = true;
        }
    else if (!strcmp(argv[curarg], "-xsl")) {
        xcodeSyncLock = true;
        }
    else if (!strcmp(argv[curarg], "-noDisplay")) {/* to disable simul display */
        simulDisplay = false;
        }
    }

    B_Os_Init();

#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if(width > 640 && height > 352 ) {
        if(width==1280 && height==720) {
            if(framerate!=10 && framerate!=7493){
                printf("UNSUPPORTED ENCODE FORMAT\n");
                return -1;
            }
            printf("ENABLE 720p ENCODE \n\n\n");
            b720pEncode=true;
        } else {
            printf("UNSUPPORTED ENCODE FORMAT\n");
            return -1;
        }
    } else if(width > BTST_INPUT_MAX_WIDTH && height > BTST_INPUT_MAX_HEIGHT ) {
        if(framerate!=7493 && framerate!=10 && framerate!=1498 && framerate!=15){
            printf("UNSUPPORTED ENCODE FORMAT\n");
            return -1;
        }
    }
#endif

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = (qam || sat)?true:false;
    /* Increase buffer size for 720p capture */
    if(b720pEncode) {
    platformSettings.heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size += 18*1024*1024;
    platformSettings.displayModuleSettings.displayHeapSettings[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].fullHdBuffers.count += 1;
    platformSettings.displayModuleSettings.displayHeapSettings[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].hdBuffers.count += 9;
    }
    if(framerate == 15 || framerate == 30) {
    platformSettings.displayModuleSettings.dropFrame = false;
    }
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

#if NEXUS_HAS_SYNC_CHANNEL && ENABLE_AUDIO_ENCODE
        /* create a sync channel */
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    if(qam) {
#if  NEXUS_HAS_FRONTEND
    printf("Transcoding from Qam Source\n");

    BKNI_CreateEvent(&signalLockedEvent);

    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        NEXUS_FrontendCapabilities capabilities;
        frontend = platformConfig.frontend[i];
        if (frontend) {
        NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
        /* Does this frontend support qam? */
        if ( capabilities.qam )
        {
            break;
        }
        }
    }

    if (NULL == frontend )
    {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        return 0;
    }
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    switch (qammode) {
    default:
    case 64: qamSettings.mode = NEXUS_FrontendQamMode_e64; qamSettings.symbolRate = 5056900; break;
    case 256 : qamSettings.mode = NEXUS_FrontendQamMode_e256; qamSettings.symbolRate = 5360537; break;
    case 1024: qamSettings.mode = NEXUS_FrontendQamMode_e1024; qamSettings.symbolRate = 0; /* TODO */break;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamLockCallbackContext.context = frontend;
    qamLockCallbackContext.handle = signalLockedEvent;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = qam_lock_callback;
    qamSettings.lockCallback.context = &qamLockCallbackContext;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
#else
    BDBG_ERR(("QAM Input not supported"));
    return 0;
#endif
    } else if (sat) {
#if  NEXUS_HAS_FRONTEND
    printf("Transcoding from Sat Source\n");

    BKNI_CreateEvent(&signalLockedEvent);

    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        NEXUS_FrontendCapabilities capabilities;
        frontend = platformConfig.frontend[i];
        if (frontend) {
        NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
        /* Does this frontend support satellite? */
        if ( capabilities.satellite )
        {
            break;
        }
        }
    }

    if (NULL == frontend )
    {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        return 0;
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = 1119000000;
    satSettings.mode = NEXUS_FrontendSatelliteMode_eDvb;
    satLockCallbackContext.context = frontend;
    satLockCallbackContext.handle = signalLockedEvent;
    satSettings.lockCallback.callback = sat_lock_callback;
    satSettings.lockCallback.context = &satLockCallbackContext;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    printf("Input Band is %d\n", userParams.param1);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
    diseqcSettings.toneEnabled = true;
    diseqcSettings.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
    NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
    printf("Set DiseqcSettings\n");
#else
    BDBG_ERR(("SAT Input not supported"));
    return 0;
#endif
    } else if(streamer) {
    printf("Transcoding from Streamer Source\n");
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    } else if(hdmi) {
#if NEXUS_NUM_HDMI_INPUTS
    NEXUS_TimebaseSettings timebaseSettings;

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
    hdmiInputSettings.timebase = NEXUS_Timebase_e0;
    hdmiInputSettings.useInternalEdid = true ;
    hdmiInput = NEXUS_HdmiInput_OpenWithEdid(0, &hdmiInputSettings,
                         &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
    if(!hdmiInput) {
        fprintf(stderr, "Can't get hdmi input\n");
        return -1;
    }
#endif
    }


    /* Acquire PSI info or probe file */
    if(qam || sat || streamer) {
    psiCollectionDataType collectionData;
    int numProgramsFound;
    struct opts_t psi[MAX_PROGRAMS_PER_FREQUENCY];
#if  NEXUS_HAS_FRONTEND
    if(qam) {
        NEXUS_Error rc;

        BKNI_ResetEvent(signalLockedEvent);
        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if (rc) {
        BDBG_WRN(("Unable to lock front end for PSI acquisition"));
        return -1;
        }
        if (BKNI_WaitForEvent(signalLockedEvent, 5000)) {
        BDBG_WRN(("QAM Src failed to lock the signal during PSI acquisition"));
        return -1;
        }
    } else if (sat) {
        NEXUS_Error rc;

        BKNI_ResetEvent(signalLockedEvent);
        rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
        if (rc) {
        BDBG_WRN(("Unable to lock front end for PSI acquisition"));
        return -1;
        }
        if (BKNI_WaitForEvent(signalLockedEvent, 5000)) {
        BDBG_WRN(("SAT Src failed to lock the signal during PSI acquisition"));
        return -1;
        }
    }
#endif
    BKNI_Memset(&collectionData, 0, sizeof(collectionData));
    collectionData.parserBand = parserBand;
    acquirePsiInfo(&collectionData, &psi[0], &numProgramsFound);
    opts = psi[0];
    } else if (hdmi){
    /* No pid setup required */
    } else  {
    probe(&opts, infile, NULL);
    }
    /* Done PSI or file probe */

    if(infile) {
    char *navfile=NULL;
    printf("Transcoding File %s\n", infile);
    if(opts.transportType == NEXUS_TransportType_eMkv ||
       opts.transportType == NEXUS_TransportType_eMp4 ) {
        navfile = (char *)infile;
    }
    file = NEXUS_FilePlay_OpenPosix(infile, navfile);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", infile);
        return -1;
    }
    }

    /* Open STC channel */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    if(hdmi) {
    stcSettings.autoConfigTimebase = false;
    }
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* encoders/mux require different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);


    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = opts.transportType;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

#if ENABLE_VIDEO_ENCODE
    /* Open Transcode Display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    if(b720pEncode) {
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(1, &displaySettings);
    } else {
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);
    }
    assert(display);

    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));


    /* Open Window for Displaying source*/
    window = NEXUS_VideoWindow_Open(display, 0);
    assert(window);

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoInfo);
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.position.x = 0;
    windowSettings.position.y = 0;
    windowSettings.position.width = videoInfo.width;
    windowSettings.position.height = videoInfo.height;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    if(b720pEncode) {
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    displayTranscode = NEXUS_Display_Open(0, &displaySettings);
    } else {
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
#else /* new VIP capture format is purely based on the STG display format full-screen */
    displaySettings.format = NEXUS_VideoFormat_e480p;
    /* STG display has designated encoder timing generator (STG) */
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
#endif
    displayTranscode = NEXUS_Display_Open(1, &displaySettings);
    }
    assert(displayTranscode);

    /* Open Transcode Window */
    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = width;
    windowSettings.position.height = height;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
    /* force the display vsync to 60Hz */
    if(framerate == 15 || framerate == 30) {
    windowSettings.autoMaster = false;
    }
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);

    {
    NEXUS_VideoWindowScalerSettings sclSettings;
    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
    }
#else
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.forceCapture = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
    if(width && height)
    {/* HZ: you could use custom display format to customize capture format */
        NEXUS_DisplayCustomFormatSettings customFormatSettings;
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = width;
        customFormatSettings.height = height;
        customFormatSettings.refreshRate = 59940;
        customFormatSettings.interlaced = false;
        customFormatSettings.aspectRatio = 0;
        customFormatSettings.dropFrameAllowed = true;
        NEXUS_Display_SetCustomFormatSettings(displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
    }
#endif

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    assert(videoDecoder);

    if(hdmi) {
#if NEXUS_NUM_HDMI_INPUTS
    if(simulDisplay) {
        NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    }
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
#endif
    } else {
    if(simulDisplay) {
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }

    /* TODO: BUG? must occur after the addInput, otherwise does not take effect */
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.preferSyncLock = xcodeSyncLock;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);

    /* Open video encoder */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);

#endif

#if ENABLE_AUDIO_ENCODE
    /* Open the audio decoder */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    assert(audioDecoder);

    /* Connect audio decoders to outputs */
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    /* Open audio mux output */
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    assert(audioMuxOutput);

    /* Open audio encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = mp4?NEXUS_AudioCodec_eAacAdts:NEXUS_AudioCodec_eAacPlusLoas;
    audioCodec = encoderSettings.codec;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    assert(audioEncoder);

    /* Connect encoder to decoder */
    NEXUS_AudioEncoder_AddInput(audioEncoder,
                                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
#endif

    if(qam || sat || streamer) {
#if ENABLE_VIDEO_ENCODE
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, opts.videoPid, NULL);
#endif

#if ENABLE_AUDIO_ENCODE
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, opts.audioPid, NULL);
#endif

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
        NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
    } else if (hdmi) {
    /* No audio/video pid channel for hdmi input */
    } else {
#if ENABLE_VIDEO_ENCODE
    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = opts.videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.videoPid, &playbackPidSettings);
#endif

#if ENABLE_AUDIO_ENCODE
    /* Open the audio pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.audioPid, &playbackPidSettings);
#endif
    }

#if ENABLE_VIDEO_ENCODE
    if(!hdmi) {
    /* Set up decoder Start structures now. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    }
#endif

#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    if(hdmi) {
#if NEXUS_NUM_HDMI_INPUTS
    audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);
    audioProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;
#endif
    }else {
    audioProgram.codec = opts.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    }
#endif

#if NEXUS_HAS_SYNC_CHANNEL && ENABLE_AUDIO_ENCODE
        /* connect sync channel */
        NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
        syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        syncChannelSettings.enablePrecisionLipsync = false;/* to support 60->30 frc transcode */
        NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

#if ENABLE_VIDEO_ENCODE
    /* Set video encode settings */
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    switch(framerate) {
    case 7493:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e7_493;
    break;
    case 10:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e10;
    break;
    case 1498:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e14_985;
    break;
    case 15:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e15;
    break;
    case 2997:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    break;
    case 30:
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
    break;
    default:
    printf("Unsupported Frame Rate\n");
    return 0;
    }
    videoEncoderConfig.bitrateMax = bitrate;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    videoEncoderStartConfig.bounds.inputDimension.max.width = width;
    videoEncoderStartConfig.bounds.inputDimension.max.height = height;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
    videoEncoderStartConfig.encodeUserData = encodeUserData;

    /* encode setting and startSetting to be set after end-to-end delay is determined */

    /* get end-to-end delay (Dee) for audio and video encoders;
     * TODO: match AV delay! In other words,
     *   if (aDee > vDee) {
     *       vDee' = aDee' = aDee;
     *   }
     *   else {
     *       vDee' = aDee' = vDee;
     *   }
     */
    {
#if ENABLE_AUDIO_ENCODE
        unsigned Dee;
#endif
        /* NOTE: video encoder delay is in 27MHz ticks */
        NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
        printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

#if ENABLE_AUDIO_ENCODE
        NEXUS_AudioMuxOutput_GetDelayStatus(audioMuxOutput, audioCodec, &audioDelayStatus);
        printf("\tAudio codec %d end-to-end delay = %u ms\n", audioCodec, audioDelayStatus.endToEndDelay);

        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
        if(Dee > videoDelay.min)
        {
            if(Dee > videoDelay.max)
            {
                BDBG_ERR(("\tAudio Dee is way too big! Use video Dee max!"));
                Dee = videoDelay.max;
            }
            else
            {
                printf("\tUse audio Dee %u ms %u ticks@27Mhz!\n", Dee/27000, Dee);
            }
        }
        else
        {
            Dee = videoDelay.min;
            printf("\tUse video Dee %u ms or %u ticks@27Mhz!\n\n", Dee/27000, Dee);
        }
        videoEncoderConfig.encoderDelay = Dee;

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = stcChannelTranscode;
        audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
        NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);
#else
        videoEncoderConfig.encoderDelay = videoDelay.min;
#endif
    }

    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
#endif

#if ENABLE_VIDEO_ENCODE
    /* Start Encoder */
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
#endif

    if(mp4) {
    BKNI_CreateEvent(&finishEvent);
    NEXUS_FileMux_GetDefaultCreateSettings(&fileMuxCreateSettings);
    fileMuxCreateSettings.finished.callback = transcoderFinishCallback;
    fileMuxCreateSettings.finished.context = finishEvent;

    fileMux = NEXUS_FileMux_Create(&fileMuxCreateSettings);
    NEXUS_FileMux_GetDefaultStartSettings(&fileMuxConfig, NEXUS_TransportType_eMp4);

#if ENABLE_VIDEO_ENCODE
    fileMuxConfig.video[0].track = 1;
    fileMuxConfig.video[0].codec = videoEncoderStartConfig.codec;
    fileMuxConfig.video[0].encoder = videoEncoder;
#endif
#if ENABLE_AUDIO_ENCODE
    fileMuxConfig.audio[0].track = 2;
    fileMuxConfig.audio[0].codec = encoderSettings.codec;
    fileMuxConfig.audio[0].muxOutput = audioMuxOutput;
#endif
    snprintf(fileMuxConfig.tempDir, sizeof(fileMuxConfig.tempDir), "videos");
    muxFileOutput = NEXUS_MuxFile_OpenPosix(mp4File);
    if (!muxFileOutput) {
        fprintf(stderr, "can't open file:%s\n", mp4File);
        return -1;
    }

    /* start mux */
    NEXUS_FileMux_Start(fileMux, &fileMuxConfig, muxFileOutput);
    } else {
#if ENABLE_VIDEO_ENCODE
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeVideo = NEXUS_Playpump_Open(1, &playpumpConfig);
    assert(playpumpTranscodeVideo);
#endif

#if ENABLE_AUDIO_ENCODE
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeAudio = NEXUS_Playpump_Open(2, &playpumpConfig);
    assert(playpumpTranscodeAudio);
#endif

    playpumpTranscodePcr = NEXUS_Playpump_Open(3, &playpumpConfig);
    assert(playpumpTranscodePcr);

    BKNI_CreateEvent(&finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = finishEvent;
    streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = stcChannelTranscode;

#if ENABLE_VIDEO_ENCODE
    muxConfig.video[0].pid = BTST_MUX_VIDEO_PID;
    muxConfig.video[0].encoder = videoEncoder;
    muxConfig.video[0].playpump = playpumpTranscodeVideo;
#endif

#if ENABLE_AUDIO_ENCODE
    muxConfig.audio[0].pid = BTST_MUX_AUDIO_PID;
    muxConfig.audio[0].muxOutput = audioMuxOutput;
    muxConfig.audio[0].playpump = playpumpTranscodeAudio;
#endif

    muxConfig.pcr.pid = BTST_MUX_PCR_PID;
    muxConfig.pcr.playpump = playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;


    /* Add system data to stream_mux */
    xcode_set_systemdata(pContext,
#if ENABLE_VIDEO_ENCODE
                 videoEncoderStartConfig.codec,
#else
                 0,
#endif
#if ENABLE_AUDIO_ENCODE
                 encoderSettings.codec
#else
                 0
#endif
        );


    /* open PidChannels */
    pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    assert(pidChannelTranscodePcr);
    pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, BTST_MUX_PMT_PID, NULL);
    assert(pidChannelTranscodePmt);
    pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, BTST_MUX_PAT_PID, NULL);
    assert(pidChannelTranscodePat);

    /* start mux */
    NEXUS_StreamMux_Start(streamMux,&muxConfig, &muxOutput);

#if ENABLE_VIDEO_ENCODE
    pidChannelTranscodeVideo = muxOutput.video[0];
#endif
#if ENABLE_AUDIO_ENCODE
    pidChannelTranscodeAudio = muxOutput.audio[0];
#endif

    recpump = NEXUS_Recpump_Open(0, NULL);
    assert(recpump);

    record = NEXUS_Record_Create();
    assert(record);

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordSettings);

#if ENABLE_VIDEO_ENCODE
    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264 /* opts.videoCodec */;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeVideo, &recordPidSettings);
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeAudio, NULL);
#endif
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePcr, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePat, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePmt, NULL);

    fileTranscode = NEXUS_FileRecord_OpenPosix(outfile, outnavfile);
    assert(fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(record, fileTranscode);
    }

    /* Start Decoders */
#if ENABLE_VIDEO_ENCODE
    if(!hdmi) {
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    }
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
#endif


    if(qam) {
#if  NEXUS_HAS_FRONTEND
    /* Tune Qam */
    NEXUS_Frontend_TuneQam(frontend, &qamSettings);
#endif
    } else if (sat) {
#if  NEXUS_HAS_FRONTEND
    /* Tune Sat */
    NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
#endif
    } else if(streamer) {
    /* Do nothing */
    } else if (hdmi) {
    /* Nothing to stop */
    } else {
    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);
    }


    {
    NEXUS_DisplayStatus displayStatus;
    NEXUS_Display_GetStatus(displayTranscode, &displayStatus);
    printf("\nDISPLAY RATE IS %d Hz", displayStatus.refreshRate);
    }

    printf("\n\nEncoding %s to %s : Width %d Height %d Frame Rate %d Bitrate %d\n\n",
       infile?infile:(qam?"QAM":(sat?"SAT":"HDMI")), mp4?mp4File:outfile, width, height, framerate, bitrate);


    /* Playback state machine is driven from inside Nexus. */

    printf("PRESS ENTER TO STOP ENCODE\n\n");
    getchar();
    /* BKNI_Sleep(3600000); */

    if(!mp4) {
    /* stop system data scheduler */
    xcode_stop_systemdata(pContext);
    }


    /* Bring down system */
    if(qam || sat) {
#if  NEXUS_HAS_FRONTEND
    NEXUS_Frontend_Untune(frontend);
#endif
    } else if(streamer) {
    /* Do nothing */
    } else if (hdmi) {
    /* Nothing to stop */
    } else {
    NEXUS_Playback_Stop(playback);
    }

#if ENABLE_VIDEO_ENCODE
    if(!hdmi) {
    NEXUS_VideoDecoder_Stop(videoDecoder);
    }
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);
#endif
#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);
#endif

#if ENABLE_AUDIO_ENCODE && NEXUS_HAS_SYNC_CHANNEL
    /* disconnect sync channel after decoders stop */
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    if(mp4) {
    NEXUS_FileMux_Finish(fileMux);
    /* wait for encode buffer to be drained; double delay margin */
    while(BKNI_WaitForEvent(finishEvent, 3000)!=BERR_SUCCESS) {
        fprintf(stderr, "File mux unfinished! Wait for another 3 seconds...\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_FileMux_Stop(fileMux);
    NEXUS_MuxFile_Close(muxFileOutput);
    NEXUS_FileMux_Destroy(fileMux);
    } else {
    unsigned jj;
    NEXUS_StreamMux_Finish(streamMux);
    /* For now we just wait for 1 sec. TODO : Configure the encoder delay */
    if(BKNI_WaitForEvent(finishEvent, 3000)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }
    for(jj=0; jj<BTST_PSI_QUEUE_CNT; jj++)
    {
        NEXUS_Memory_Free(pContext->pat[jj]);
        NEXUS_Memory_Free(pContext->pmt[jj]);
    }

    BKNI_DestroyEvent(finishEvent);
    NEXUS_Record_Stop(record);

    /* Note: remove all record PID channels before stream mux stop since streammux would close the A/V PID channels */
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_StreamMux_Stop(streamMux);

    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
    NEXUS_FileRecord_Close(fileTranscode);

    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePcr);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePat);
    NEXUS_Playpump_ClosePidChannel(playpumpTranscodePcr, pidChannelTranscodePmt);

    NEXUS_StreamMux_Destroy(streamMux);

    NEXUS_Playpump_Close(playpumpTranscodePcr);
#if ENABLE_VIDEO_ENCODE
    NEXUS_Playpump_Close(playpumpTranscodeVideo);
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_Playpump_Close(playpumpTranscodeAudio);
#endif
    }

    if(qam || sat || streamer) {
#if ENABLE_VIDEO_ENCODE
    NEXUS_PidChannel_Close(videoPidChannel);
#endif
#if ENABLE_AUDIO_ENCODE
        NEXUS_PidChannel_Close(audioPidChannel);
#endif
    } else if (hdmi) {
    /* Nothing to close */
    } else {
    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_FilePlay_Close(file);
    }


#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoEncoder_Close(videoEncoder);

    if(hdmi){
#if NEXUS_NUM_HDMI_INPUTS
    if(simulDisplay) {
        NEXUS_VideoWindow_RemoveInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    }
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
#endif
    } else {
    if(simulDisplay) {
        NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }
#endif

#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioOutput_RemoveInput(
    NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioEncoder_RemoveInput(audioEncoder,
                   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioEncoder_Close(audioEncoder);
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
#endif

#if NEXUS_NUM_HDMI_INPUTS
    if(hdmi) {
    NEXUS_HdmiInput_Close(hdmiInput);
    }
#endif

    printf("\n\nStarting Playback of Encoded File %s\n\n", mp4?mp4File:outfile);

    probe(&opts, mp4?mp4File:outfile, NULL);

    file = NEXUS_FilePlay_OpenPosix(mp4?mp4File:outfile, mp4?mp4File:outnavfile);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", mp4?mp4File:outfile);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpumpSettings.transportType = opts.transportType;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

#if ENABLE_VIDEO_ENCODE
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
#endif

#if ENABLE_VIDEO_ENCODE
    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = opts.videoCodec;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.videoPid, &playbackPidSettings);
#endif

#if ENABLE_AUDIO_ENCODE
    /* Open the audio pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.audioPid, &playbackPidSettings);
#endif

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* live */
    NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);

    /* Start Decoders */
#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
#endif

#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = opts.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
#endif

#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
#endif

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    printf("PRESS ENTER TO STOP PLAYBACK\n\n");
    getchar();

#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoDecoder_Stop(videoDecoder);
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Stop(audioDecoder);
#endif
    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
#if ENABLE_VIDEO_ENCODE
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(display);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_VideoDecoder_Close(videoDecoder);
#endif
#if ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Close(audioDecoder);
#endif
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);
    if(signalLockedEvent)
    BKNI_DestroyEvent(signalLockedEvent);

    NEXUS_Platform_Uninit();

    B_Os_Uninit();

    return 0;
}

#endif
