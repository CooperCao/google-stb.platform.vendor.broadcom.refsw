/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#include "standby.h"
#include "util.h"

#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "bfile_stdio.h"

#include "b_psip_lib.h"
#include "tspsimgr.h"

BDBG_MODULE(standby_priv);

B_StandbyNexusHandles g_StandbyNexusHandles;
B_DeviceState g_DeviceState;

pthread_t gfx2d_thread;

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
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs},
    {NEXUS_VideoCodec_eH265, bvideo_codec_h265}
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
                BDBG_MSG(("NEXUS_Message_GetBuffer() succeeded! size:%d\n",size));
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
            BDBG_MSG(("program_number: %d, i %d, PID: 0x%04X, psi %p, sizeof psi %d\n", program.program_number, i, program.PID, psi, sizeof(*psi)));
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

#if PLAYBACK_IP_SUPPORT
static bmedia_probe_stream * playback_ip_start(unsigned id)
{
    bmedia_probe_stream *stream=NULL;
    /* URL contains http src info, setup IP playback */
    B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
    B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
    B_PlaybackIpSessionSetupSettings ipSessionSetupSettings;
    B_PlaybackIpSessionSetupStatus ipSessionSetupStatus;
    NEXUS_Error rc;

    if (!strcasecmp(g_DeviceState.url.scheme, "http") || !strcasecmp(g_DeviceState.url.scheme, "https")) {
        /* Setup socket setting structure used in the IP Session Open */
        B_PlaybackIp_GetDefaultSessionOpenSettings(&ipSessionOpenSettings);
        ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
        strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, g_DeviceState.url.domain, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
        ipSessionOpenSettings.socketOpenSettings.port = g_DeviceState.url.port;
        ipSessionOpenSettings.socketOpenSettings.url = g_DeviceState.url.path;
        BDBG_WRN(("parsed url is http://%s:%d%s", ipSessionOpenSettings.socketOpenSettings.ipAddr, ipSessionOpenSettings.socketOpenSettings.port, ipSessionOpenSettings.socketOpenSettings.url));
        ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePull;

        rc = B_PlaybackIp_SessionOpen(g_StandbyNexusHandles.playbackIp[id], &ipSessionOpenSettings, &ipSessionOpenStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG (("Session Open call succeeded, HTTP status code %d", ipSessionOpenStatus.u.http.statusCode));

        /* now do session setup */
        B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
        /* if app needs to play multiple formats (such as a DLNA DMP/DMR) (e.g. TS, VOB/PES, MP4, ASF, etc.), then set this option to do deep payload inspection */
        ipSessionSetupSettings.u.http.enablePayloadScanning = true;
        /* set a limit on how long the psi parsing should continue before returning */
        ipSessionSetupSettings.u.http.psiParsingTimeLimit = 30000; /* 30sec */
        rc = B_PlaybackIp_SessionSetup(g_StandbyNexusHandles.playbackIp[id], &ipSessionSetupSettings, &ipSessionSetupStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG (("Session Setup call succeeded, file handle %p", ipSessionSetupStatus.u.http.file));
        stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.http.stream);
        g_StandbyNexusHandles.filePlay[id] = ipSessionSetupStatus.u.http.file;
        if (ipSessionSetupStatus.u.http.psi.liveChannel)
            g_DeviceState.playbackIpLiveMode[id] = true;
        else
            g_DeviceState.playbackIpLiveMode[id] = false;
        g_DeviceState.playbackIpPsi[id] = ipSessionSetupStatus.u.http.psi;
        BDBG_MSG(("file %p, stream %p, psi %p, psi valid %d", ipSessionSetupStatus.u.http.file, ipSessionSetupStatus.u.http.stream, ipSessionSetupStatus.u.http.psi, ipSessionSetupStatus.u.http.psi.psiValid));
    } else if (!strcasecmp(g_DeviceState.url.scheme, "udp") || !strcasecmp(g_DeviceState.url.scheme, "rtp")) {
        /* Setup socket setting structure used in the IP Session Open */
        B_PlaybackIp_GetDefaultSessionOpenSettings(&ipSessionOpenSettings);
        ipSessionOpenSettings.maxNetworkJitter = 300;
        ipSessionOpenSettings.networkTimeout = 1;  /* timeout in 1 sec during network outage events */
        if (!strcasecmp(g_DeviceState.url.scheme, "rtp")) {
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eRtp;
        }
        else {
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eUdp;
        }
        strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, g_DeviceState.url.domain, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
        ipSessionOpenSettings.socketOpenSettings.port = g_DeviceState.url.port;
#if 0
        /* needed for RTSP */
        ipSessionOpenSettings.socketOpenSettings.url = g_DeviceState.url.path;
#endif
        BDBG_MSG(("parsed url is udp://%s:%d%s", ipSessionOpenSettings.socketOpenSettings.ipAddr, ipSessionOpenSettings.socketOpenSettings.port, ipSessionOpenSettings.socketOpenSettings.url));
        ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;

        rc = B_PlaybackIp_SessionOpen(g_StandbyNexusHandles.playbackIp[id], &ipSessionOpenSettings, &ipSessionOpenStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG(("Session Open call succeeded"));

        /* now do session setup */
        B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
        /* set a limit on how long the psi parsing should continue before returning */
        ipSessionSetupSettings.u.udp.psiParsingTimeLimit = 3000; /* 3sec */
        rc = B_PlaybackIp_SessionSetup(g_StandbyNexusHandles.playbackIp[id], &ipSessionSetupSettings, &ipSessionSetupStatus);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        BDBG_MSG(("Session Setup call succeeded"));
        stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.udp.stream);
        g_StandbyNexusHandles.filePlay[id] = NULL;
        g_DeviceState.playbackIpLiveMode[id] = true;
        g_DeviceState.playbackIpPsi[id] = ipSessionSetupStatus.u.udp.psi;
    }

    BDBG_MSG(("Video Pid %d, Video Codec %d, Audio Pid %d, Audio Codec %d, Pcr Pid %d, Transport Type %d",
                g_DeviceState.playbackIpPsi[id].videoPid, g_DeviceState.playbackIpPsi[id].videoCodec, g_DeviceState.playbackIpPsi[id].audioPid, g_DeviceState.playbackIpPsi[id].audioCodec, g_DeviceState.playbackIpPsi[id].pcrPid, g_DeviceState.playbackIpPsi[id].mpegType));

error:
    return stream;
}

static void playback_ip_stop(unsigned id)
{
    if (g_DeviceState.playbackIpActive[id]) {
        B_PlaybackIp_SessionStop(g_StandbyNexusHandles.playbackIp[id]);
        B_PlaybackIp_SessionClose(g_StandbyNexusHandles.playbackIp[id]);
    }
}
#endif

#undef min
#define min(A,B) ((A)<(B)?(A):(B))

/* parse_url()

example: http://player.vimeo.com:80/play_redirect?quality=hd&codecs=h264&clip_id=638324

scheme=http
domain=player.vimeo.com
port=80
path=/play_redirect?quality=hd&codecs=h264&clip_id=638324

example: file://videos/cnnticker.mpg or videos/cnnticker.mpg

scheme=file
domain=
port=0
path=videos/cnnticker.mpg

 */
#if PLAYBACK_IP_SUPPORT
static void parse_url(const char *s, struct url_t *url)
{
    const char *server, *file;

    memset(url, 0, sizeof(*url));

    server = strstr(s, "://");
    if (!server) {
        strcpy(url->scheme, "file");
        server = s;
    }
    else {
        strncpy(url->scheme, s, server-s);
        server += strlen("://"); /* points to the start of server name */
    }

    if (!strcmp(url->scheme, "file")) {
        strncpy(url->path, server, sizeof(url->path)-1);
    }
    else {
        char *port;
        file = strstr(server, "/"); /* should point to start of file name */
        if (file) {
            strncpy(url->domain, server, min(sizeof(url->domain)-1, (unsigned)(file-server)));
            strncpy(url->path, file, sizeof(url->path)-1);
        }
        else {
            strncpy(url->domain, server, sizeof(url->domain)-1);
        }

        /* now server string is null terminated, look for explicit port number in the format of server:port */
        port = strstr(url->domain, ":");
        if (port) {
            *port = 0;
            url->port = atoi(port+1);
        }
        else {
            url->port = 80; /* default port */
        }
    }
}
#endif

int probe(unsigned id)
{
    int rc = 0;
    struct opts_t *opts = &g_DeviceState.opts[id];
    const char *filename = g_DeviceState.playfile[id];
    const char **indexname = NULL;

    /* use media probe to set values */
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    FILE *fin;
    char stream_info[512];

    opts->videoCodec = NEXUS_VideoCodec_eUnknown;
    opts->audioCodec = NEXUS_AudioCodec_eUnknown;

#if PLAYBACK_IP_SUPPORT
    g_DeviceState.playbackIpActive[id] = false;

    parse_url(filename, &g_DeviceState.url);

    if (!strcasecmp(g_DeviceState.url.scheme, "http")  ||
            !strcasecmp(g_DeviceState.url.scheme, "https") ||
            !strcasecmp(g_DeviceState.url.scheme, "udp")   ||
            !strcasecmp(g_DeviceState.url.scheme, "rtp")) {
        stream = playback_ip_start(id);

        g_DeviceState.playbackIpActive[id] = true;
        opts->transportType = g_DeviceState.playbackIpPsi[id].mpegType;
    }
    else
#endif
    {
        probe = bmedia_probe_create();

        fin = fopen64(filename,"rb");
        if (!fin) {
            printf("\n\n/*************************************/\n");
            printf("Can't open media file '%s' for probing\n", filename);
            printf("/*************************************/\n\n");
            rc = -1;
            goto error;
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
            goto error;
        }

        /* if the user has specified the index, don't override */
        if (indexname && !*indexname) {
            if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required) {
                *indexname = filename;
            }
        }
    }

    if(stream) {
        bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
        printf(
                "Media Probe:\n"
                "%s\n\n",
                stream_info);
        opts->transportType = b_mpegtype2nexus(stream->type);
    }

#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id] && (g_DeviceState.playbackIpLiveMode[id] || g_DeviceState.playbackIpPsi[id].hlsSessionEnabled || g_DeviceState.playbackIpPsi[id].mpegDashSessionEnabled || g_DeviceState.playbackIpPsi[id].numPlaySpeedEntries)) {
        opts->videoPid = g_DeviceState.playbackIpPsi[id].videoPid;
        opts->videoCodec = g_DeviceState.playbackIpPsi[id].videoCodec;
        opts->audioPid = g_DeviceState.playbackIpPsi[id].audioPid;
        opts->audioCodec = g_DeviceState.playbackIpPsi[id].audioCodec;
        opts->pcrPid  = g_DeviceState.playbackIpPsi[id].pcrPid;
    } else
#endif
    {
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
                        opts->width = track->info.video.width;
                        opts->height = track->info.video.height;
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
    }

error:
#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id]) {
        playback_ip_stop(id);
    }
#endif
    if (probe) {
        if (stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }

    return rc;
}


void irCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;     /*  */
    bool overflow;
    NEXUS_IrInputHandle irHandle = *(NEXUS_IrInputHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_IrInputEvent irEvent;
        rc = NEXUS_IrInput_GetEvents(irHandle, &irEvent, 1, &numEvents, &overflow);
        if (numEvents && !irEvent.repeat) {
            printf("Received IR event\n");
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
            switch(irEvent.code) {
                case S1_IR_CODE:
                    g_DeviceState.power_mode = ePowerModeS1;
                    break;
                case S2_IR_CODE:
                    g_DeviceState.power_mode = ePowerModeS2;
                    break;
                case S3_IR_CODE:
                    g_DeviceState.power_mode = ePowerModeS3;
                    break;
                case S5_IR_CODE:
                    g_DeviceState.power_mode = ePowerModeS5;
                    break;
                case EXIT_IR_CODE:
                    g_DeviceState.exit_app = true;
                    break;
                default:
                    BDBG_MSG(("Unknown IR event (%x)\n", irEvent.code));
                    return;
            }
            if(g_StandbyNexusHandles.event)
                BKNI_SetEvent(g_StandbyNexusHandles.event);
        }
    }
}

#if NEXUS_HAS_UHF_INPUT
void uhfCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_UhfInputHandle uhfHandle = *(NEXUS_UhfInputHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_UhfInputEvent uhfEvent;
        rc = NEXUS_UhfInput_GetEvents(uhfHandle, &uhfEvent, 1, &numEvents, &overflow);
        if (numEvents && !uhfEvent.repeat) {
            printf("Received UHF event %08x\n", uhfEvent.code);
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}
#endif

#if NEXUS_HAS_KEYPAD
void keypadCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_KeypadHandle  keypadHandle = *(NEXUS_KeypadHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_KeypadEvent keypadEvent;
        rc = NEXUS_Keypad_GetEvents(keypadHandle, &keypadEvent,1, &numEvents, &overflow);
        if (numEvents && !keypadEvent.repeat) {
            printf("Received Keypad event %08x\n", keypadEvent.code);
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}
#endif

#if NEXUS_HAS_CEC && NEXUS_HAS_HDMI_OUTPUT
void cecDeviceReady_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_WRN(("BCM%d Logical Address <%d> Acquired",
                BCHP_CHIP,
                status.logicalAddress)) ;

    BDBG_WRN(("BCM%d Physical Address: %X.%X.%X.%X",
                BCHP_CHIP,
                (status.physicalAddress[0] & 0xF0) >> 4,
                (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4,
                (status.physicalAddress[1] & 0x0F))) ;

    if ((status.physicalAddress[0] = 0xFF)
            && (status.physicalAddress[1] = 0xFF))
    {
        BDBG_WRN(("CEC Device Ready!")) ;
        g_DeviceState.cecDeviceReady = true ;
    }
}

void cecMsgReceived_callback(void *context, int param)
{
    NEXUS_CecStatus status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error rc ;
    uint8_t i, j ;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_MSG(("Message Received: %s", status.messageReceived ? "Yes" : "No")) ;

    rc = NEXUS_Cec_ReceiveMessage(g_StandbyNexusHandles.hCec, &receivedMessage);
    BDBG_ASSERT(!rc);

    /* For debugging purposes */
    for (i = 0, j = 0; i < receivedMessage.data.length ; i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
                receivedMessage.data.buffer[i]) ;
    }
    BDBG_MSG(("CEC Message Length %d Received: %s",
                receivedMessage.data.length, msgBuffer)) ;

    BDBG_MSG(("Msg Recd Status from Phys/Logical Addrs: %X.%X.%X.%X / %d",
                (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
                status.logicalAddress)) ;

    if((receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x40) || /* User Control Pressed : Power */
       (receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x6b) || /* User Control Pressed : Power Toggle */
       (receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x6d) || /* User Control Pressed : Power On */
       (receivedMessage.data.buffer[0] == 0x86 && receivedMessage.data.buffer[1] == 0x0 && receivedMessage.data.buffer[2] == 0x0)   /* Set Stream Path */
      ) {
        if(g_DeviceState.power_mode == ePowerModeS1 && g_StandbyNexusHandles.s1Event) {
            BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}

void cecMsgTransmitted_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_WRN(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d",
                (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
                status.logicalAddress)) ;

    BDBG_WRN(("Xmit Msg Acknowledged: %s",
                status.transmitMessageAcknowledged ? "Yes" : "No")) ;
    BDBG_WRN(("Xmit Msg Pending: %s",
                status.messageTransmitPending ? "Yes" : "No")) ;
}

void cec_setup(void)
{
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
    NEXUS_HdmiOutputStatus status;
    unsigned loops;

    g_StandbyNexusHandles.hCec = g_StandbyNexusHandles.platformConfig.outputs.cec[0];

    NEXUS_HdmiOutput_GetStatus(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &status);

    NEXUS_Cec_GetSettings(g_StandbyNexusHandles.hCec, &cecSettings);
    cecSettings.messageReceivedCallback.callback = cecMsgReceived_callback ;
    cecSettings.messageReceivedCallback.context = g_StandbyNexusHandles.event;

    cecSettings.messageTransmittedCallback.callback = cecMsgTransmitted_callback;
    cecSettings.messageTransmittedCallback.context = g_StandbyNexusHandles.event;

    cecSettings.logicalAddressAcquiredCallback.callback = cecDeviceReady_callback ;
    cecSettings.logicalAddressAcquiredCallback.context = g_StandbyNexusHandles.event;

    cecSettings.physicalAddress[0]= (status.physicalAddressA << 4)
        | status.physicalAddressB;
    cecSettings.physicalAddress[1]= (status.physicalAddressC << 4)
        | status.physicalAddressD;

    NEXUS_Cec_SetSettings(g_StandbyNexusHandles.hCec, &cecSettings);

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(g_StandbyNexusHandles.hCec, &cecSettings);
    cecSettings.enabled = true;
    NEXUS_Cec_SetSettings(g_StandbyNexusHandles.hCec, &cecSettings);

    printf("Wait for logical address before starting test...\n");
    for (loops = 0; loops < 20; loops++) {
        if (g_DeviceState.cecDeviceReady)
            break;
        BKNI_Sleep(100);
    }

    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &cecStatus);
    if (cecStatus.logicalAddress == 0xFF)
    {
        printf("No CEC capable device found on HDMI output\n");
    }
}
#endif

#if NEXUS_HAS_FRONTEND
void untune_frontend(unsigned id)
{
    if(!g_DeviceState.frontend_tuned[id])
        return;

    NEXUS_Frontend_Release(g_StandbyNexusHandles.frontend[id]);

    g_DeviceState.frontend_tuned[id] = false;
}

static void qam_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)param;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend 0x%08x\n", (unsigned)frontend);

    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("QAM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked){
        printf("QAM Lock callback: Fast lock status = Locked.\n");
        BKNI_SetEvent(statusEvent);
    }
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("QAM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("QAM Lock callback: Fast lock status = Unknown.\n");
}

int tune_qam(unsigned id)
{
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
    if (!g_StandbyNexusHandles.frontend[id]) {
        BDBG_ERR(("Unable to find QAM-capable frontend"));
        rc = NEXUS_NOT_AVAILABLE;
        return rc;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    switch (g_DeviceState.frontend[id].qammode) {
        default:
        case 64: qamSettings.mode = NEXUS_FrontendQamMode_e64; qamSettings.symbolRate = 5056900; break;
        case 256 : qamSettings.mode = NEXUS_FrontendQamMode_e256; qamSettings.symbolRate = 5360537; break;
        case 1024: qamSettings.mode = NEXUS_FrontendQamMode_e1024; qamSettings.symbolRate = 0; /* TODO */break;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = qam_lock_callback;
    qamSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];
    qamSettings.lockCallback.param = (int)g_StandbyNexusHandles.signalLockedEvent;

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]);
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    rc = NEXUS_Frontend_TuneQam(g_StandbyNexusHandles.frontend[id], &qamSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_qam(unsigned id)
{
   untune_frontend(id);
}

static void ofdm_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)param;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend 0x%08x\n", (unsigned)frontend);

    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("OFDM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked){
        printf("OFDM Lock callback: Fast lock status = Locked.\n");
        BKNI_SetEvent(statusEvent);
    }
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("OFDM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("OFDM Lock callback: Fast lock status = Unknown.\n");
}


int tune_ofdm(unsigned id)
{
    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    if(!g_StandbyNexusHandles.frontend[id]){
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.ofdm = true;
        g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
        if (!g_StandbyNexusHandles.frontend[id]) {
            fprintf(stderr, "Unable to find OFDM-capable frontend\n");
            rc = NEXUS_NOT_AVAILABLE;
            return rc;
        }
    }

    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);

    ofdmSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;

    switch (g_DeviceState.frontend[id].ofdmmode) {
        default:
        case 0:
            ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbt;
            ofdmSettings.bandwidth = 8000000;
            ofdmSettings.manualTpsSettings = false;
            ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
            ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;
            ofdmSettings.terrestrial = true;
            break;
        case 1:
            ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbt2;
            ofdmSettings.bandwidth = 8000000;
            ofdmSettings.dvbt2Settings.plpMode = true;
            ofdmSettings.dvbt2Settings.plpId = 0;
            ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
            ofdmSettings.terrestrial = true;
            break;
        case 2: ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbc2; break;
        case 3: ofdmSettings.mode = NEXUS_FrontendOfdmMode_eIsdbt; ofdmSettings.bandwidth = 6000000; break;
    }

    ofdmSettings.lockCallback.callback = ofdm_lock_callback;
    ofdmSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];
    ofdmSettings.lockCallback.param = (int)g_StandbyNexusHandles.signalLockedEvent;

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    rc = NEXUS_Frontend_TuneOfdm(g_StandbyNexusHandles.frontend[id], &ofdmSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_ofdm(unsigned id)
{
    untune_frontend(id);
}

static void sat_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)param;

    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %d\n", status.demodLocked);

    BKNI_SetEvent(statusEvent);
}

int tune_sat(unsigned id)
{
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;
    g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
    if (!g_StandbyNexusHandles.frontend[id]) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        rc = NEXUS_NOT_AVAILABLE;
        return rc;
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    satSettings.mode = g_DeviceState.frontend[id].satmode;
    satSettings.lockCallback.callback = sat_lock_callback;
    satSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];
    satSettings.lockCallback.param = (int)g_StandbyNexusHandles.signalLockedEvent;

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    /* Map a parser band to the demod's input band. */
    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]);
    } else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    NEXUS_Frontend_GetDiseqcSettings(g_StandbyNexusHandles.frontend[id], &diseqcSettings);
    diseqcSettings.toneEnabled = true;
    diseqcSettings.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
    NEXUS_Frontend_SetDiseqcSettings(g_StandbyNexusHandles.frontend[id], &diseqcSettings);
    printf("Set DiseqcSettings\n");

    if (g_DeviceState.frontend[id].adc != 0) {
        NEXUS_FrontendSatelliteRuntimeSettings settings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(g_StandbyNexusHandles.frontend[id], &settings);
        settings.selectedAdc = g_DeviceState.frontend[id].adc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(g_StandbyNexusHandles.frontend[id], &settings);
    }

    rc = NEXUS_Frontend_TuneSatellite(g_StandbyNexusHandles.frontend[id], &satSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_sat(unsigned id)
{
    untune_frontend(id);
}
#endif

int streamer_start(unsigned id)
{
    NEXUS_ParserBandSettings parserBandSettings;

    if(g_DeviceState.frontend_tuned[id])
        return 0;

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    return 0;
}

void streamer_stop(unsigned id)
{
    BSTD_UNUSED(id);
    /* Nothing to untune for streamer */
    return;
}

void *graphics2d_thread(void *context)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_SurfaceCreateSettings surfaceHdSettings, surfaceSdSettings;
    NEXUS_Error rc;
    unsigned i;

    BSTD_UNUSED(context);

    if(g_StandbyNexusHandles.offscreenHD)
        NEXUS_Surface_GetCreateSettings(g_StandbyNexusHandles.offscreenHD, &surfaceHdSettings);
    if(g_StandbyNexusHandles.offscreenSD)
        NEXUS_Surface_GetCreateSettings(g_StandbyNexusHandles.offscreenSD, &surfaceSdSettings);

    for(i=0;;i++) {

        if(g_DeviceState.stop_graphics2d) {
            rc = NEXUS_Graphics2D_Checkpoint(g_StandbyNexusHandles.gfx2d, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(g_StandbyNexusHandles.checkpointEvent, BKNI_INFINITE);
            }
            return NULL;
        }

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);

        if(g_StandbyNexusHandles.offscreenHD) {
            fillSettings.surface = g_StandbyNexusHandles.offscreenHD;
            fillSettings.rect.x = rand() % (surfaceHdSettings.width - 20);
            fillSettings.rect.y = rand() % (surfaceHdSettings.height - 20);
            fillSettings.rect.width = 20;
            fillSettings.rect.height = 20;
            fillSettings.color = (0xFF << 24) | rand();

            while (1) {
                rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                    BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                } else {
                    break;
                }
            }
        }

        if(g_StandbyNexusHandles.offscreenSD) {
            fillSettings.surface = g_StandbyNexusHandles.offscreenSD;
            fillSettings.rect.x = rand() % (surfaceSdSettings.width - 8);
            fillSettings.rect.y = rand() % (surfaceSdSettings.height - 8);
            fillSettings.rect.width = 8;
            fillSettings.rect.height = 8;
            fillSettings.color = (0xFF << 24) | rand();

            while (1) {
                rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                    BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                } else {
                    break;
                }
            }
        }

        if (i && i%100==0) {

            if(g_DeviceState.decode_started[0]) {
                if(g_StandbyNexusHandles.offscreenHD) {
                    fillSettings.surface = g_StandbyNexusHandles.offscreenHD;
                    fillSettings.rect.x = 0;
                    fillSettings.rect.y = 0;
                    fillSettings.rect.width = surfaceHdSettings.width/2;
                    fillSettings.rect.height = surfaceHdSettings.height/2;
                    fillSettings.color = 0x00cacaca;
                    while (1) {
                        rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                            BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                        } else {
                            break;
                        }
                    }
                }

                if(g_StandbyNexusHandles.offscreenSD) {
                    fillSettings.surface = g_StandbyNexusHandles.offscreenSD;
                    fillSettings.rect.x = 0;
                    fillSettings.rect.y = 0;
                    fillSettings.rect.width = surfaceSdSettings.width/2;
                    fillSettings.rect.height = surfaceSdSettings.height/2;
                    fillSettings.color = 0x00cacaca;
                    while (1) {
                        rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                            BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                        } else {
                            break;
                        }
                    }
                }
            }
#if NEXUS_NUM_VIDEO_WINDOWS > 1
            if(g_DeviceState.decode_started[1]) {
                if(g_StandbyNexusHandles.offscreenHD) {
                    fillSettings.surface = g_StandbyNexusHandles.offscreenHD;
                    fillSettings.rect.x = surfaceHdSettings.width/2;
                    fillSettings.rect.y = surfaceHdSettings.height/2;
                    fillSettings.rect.width = surfaceHdSettings.width/2;
                    fillSettings.rect.height = surfaceHdSettings.height/2;
                    fillSettings.color = 0x00cacaca;
                    while (1) {
                        rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                            BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                        } else {
                            break;
                        }
                    }
                }

                if(g_StandbyNexusHandles.offscreenSD) {
                    fillSettings.surface = g_StandbyNexusHandles.offscreenSD;
                    fillSettings.rect.x = surfaceSdSettings.width/2;
                    fillSettings.rect.y = surfaceSdSettings.height/2;
                    fillSettings.rect.width = surfaceSdSettings.width/2;
                    fillSettings.rect.height = surfaceSdSettings.height/2;
                    fillSettings.color = 0x00cacaca;
                    while (1) {
                        rc = NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                            BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                        } else {
                            break;
                        }
                    }
                }
            }
#endif
            if(g_StandbyNexusHandles.offscreenHD) {
                NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
                blitSettings.source.surface = g_StandbyNexusHandles.offscreenHD;
                blitSettings.output.surface = g_StandbyNexusHandles.framebufferHD;
                blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
                blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
                while (1) {
                    rc = NEXUS_Graphics2D_Blit(g_StandbyNexusHandles.gfx2d, &blitSettings);
                    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                        BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                    } else {
                        break;
                    }
                }
            }

            if(g_StandbyNexusHandles.offscreenSD) {
                NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
                blitSettings.source.surface = g_StandbyNexusHandles.offscreenSD;
                blitSettings.output.surface = g_StandbyNexusHandles.framebufferSD;
                blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
                blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
                while (1) {
                    rc = NEXUS_Graphics2D_Blit(g_StandbyNexusHandles.gfx2d, &blitSettings);
                    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
                        BKNI_WaitForEvent(g_StandbyNexusHandles.spaceAvailableEvent, BKNI_INFINITE);
                    } else {
                        break;
                    }
                }
            }

            rc = NEXUS_Graphics2D_Checkpoint(g_StandbyNexusHandles.gfx2d, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(g_StandbyNexusHandles.checkpointEvent, BKNI_INFINITE);
            }
            BDBG_ASSERT(!rc);
        }
    }
}

int playback_start(unsigned id)
{
    NEXUS_Error rc=0;
    char *navfile=NULL;

    if(g_DeviceState.playback_started[id])
        return rc;

#if PLAYBACK_IP_SUPPORT
    if(g_DeviceState.playbackIpActive[id] ) {
        unsigned cnt = 300;
        while(!isIpReachable(g_DeviceState.url.domain, g_DeviceState.url.port) && cnt) {
            BKNI_Sleep(100);
            cnt--;
        }
        if(!cnt)
            BDBG_WRN(("Timed out waiting for network"));
        playback_ip_start(id);
    } else
#endif
    {
        if(g_DeviceState.opts[id].transportType == NEXUS_TransportType_eMkv ||
                g_DeviceState.opts[id].transportType == NEXUS_TransportType_eMp4 ) {
            navfile = (char *)g_DeviceState.playfile[id];
        }
        g_StandbyNexusHandles.filePlay[id] = NEXUS_FilePlay_OpenPosix(g_DeviceState.playfile[id], navfile);
        if (!g_StandbyNexusHandles.filePlay[id]) {
            fprintf(stderr, "CANNOT OPEN PLAYBACK FILE %s\n", g_DeviceState.playfile[id]);
            goto err;
        }
    }

#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id] && (g_DeviceState.playbackIpLiveMode[id] || g_DeviceState.playbackIpPsi[id].hlsSessionEnabled || g_DeviceState.playbackIpPsi[id].mpegDashSessionEnabled || g_DeviceState.playbackIpPsi[id].numPlaySpeedEntries)) {
        rc = NEXUS_Playpump_Start(g_StandbyNexusHandles.playpump[id]);
        if (rc) { BERR_TRACE(rc); goto err;}
    } else
#endif
    {
        /* Start playback */
        rc = NEXUS_Playback_Start(g_StandbyNexusHandles.playback[id], g_StandbyNexusHandles.filePlay[id], NULL);
        if (rc) { BERR_TRACE(rc); goto err;}
    }

#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id]) {
        B_PlaybackIpSessionStartSettings ipSessionStartSettings;
        B_PlaybackIpSessionStartStatus ipSessionStartStatus;
        /*B_PlaybackIp_GetDefaultSessionStartSettings(&ipSessionStartSettings);*/
        memset(&ipSessionStartSettings, 0, sizeof(ipSessionStartSettings));
        memset(&ipSessionStartStatus, 0, sizeof(ipSessionStartStatus));
        /* set Nexus handles */
        ipSessionStartSettings.nexusHandles.playpump = g_StandbyNexusHandles.playpump[id];
        ipSessionStartSettings.nexusHandles.playback = g_StandbyNexusHandles.playback[id];
        ipSessionStartSettings.nexusHandles.videoDecoder = g_StandbyNexusHandles.videoDecoder[id];
        ipSessionStartSettings.nexusHandles.stcChannel = g_StandbyNexusHandles.stcChannel[id];
        ipSessionStartSettings.nexusHandles.primaryAudioDecoder = g_StandbyNexusHandles.audioDecoder[id];
        ipSessionStartSettings.nexusHandles.videoPidChannel = g_StandbyNexusHandles.videoPidChannel[id];
        /* ipSessionStartSettings.nexusHandles.extraVideoPidChannel = player->videoProgram.settings.enhancementPidChannel; */
        ipSessionStartSettings.nexusHandles.audioPidChannel = g_StandbyNexusHandles.audioPidChannel[id];
        ipSessionStartSettings.nexusHandles.pcrPidChannel = g_StandbyNexusHandles.videoPidChannel[id];
        ipSessionStartSettings.nexusHandlesValid = true;
        ipSessionStartSettings.mpegType = g_DeviceState.opts[id].transportType;
        rc = B_PlaybackIp_SessionStart(g_StandbyNexusHandles.playbackIp[id], &ipSessionStartSettings, &ipSessionStartStatus);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }
#endif

    g_DeviceState.playback_started[id] = true;

err:
    return rc;
}

void playback_stop(unsigned id)
{
    if(!g_DeviceState.playback_started[id])
        return;

#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id] && (g_DeviceState.playbackIpLiveMode[id] || g_DeviceState.playbackIpPsi[id].hlsSessionEnabled || g_DeviceState.playbackIpPsi[id].mpegDashSessionEnabled || g_DeviceState.playbackIpPsi[id].numPlaySpeedEntries)) {
        NEXUS_Playpump_Stop(g_StandbyNexusHandles.playpump[id]);
    } else
#endif
    {
        /*Stop playback */
        NEXUS_Playback_Stop(g_StandbyNexusHandles.playback[id]);
    }

#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id]) {
        playback_ip_stop(id);
    } else
#endif
        /* Close File. Required for umount */
        NEXUS_FilePlay_Close(g_StandbyNexusHandles.filePlay[id]);

    g_DeviceState.playback_started[id] = false;
}

int record_start(unsigned id)
{
    NEXUS_Error rc=0;

#if NEXUS_HAS_RECORD
    NEXUS_RecordPidChannelSettings pidSettings;
    char *mpgfile = id==0?"videos/record0.mpg":"videos/record1.mpg";
    char *navfile = id==0?"videos/record0.nav":"videos/record1.nav";

    if(g_DeviceState.record_started[id])
        return rc;

    g_StandbyNexusHandles.fileRec[id] = NEXUS_FileRecord_OpenPosix(mpgfile, navfile);
    if (!g_StandbyNexusHandles.fileRec[id]) {
        fprintf(stderr, "CANNOT OPEN RECORD FILE %d\n", id);
        goto err;
    }

    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = g_DeviceState.opts[id].videoCodec;
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.videoPidChannel[id], &pidSettings);

    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.audioPidChannel[id], NULL);

    rc = NEXUS_Record_Start(g_StandbyNexusHandles.record[id], g_StandbyNexusHandles.fileRec[id]);
    if (rc) { BERR_TRACE(rc); goto err;}

    g_DeviceState.record_started[id] = true;

err:
#endif

    return rc;
}

void record_stop(unsigned id)
{
#if NEXUS_HAS_RECORD
    if(!g_DeviceState.record_started[id])
        return;

    NEXUS_Record_Stop(g_StandbyNexusHandles.record[id]);
    NEXUS_Record_RemoveAllPidChannels(g_StandbyNexusHandles.record[id]);
    NEXUS_FileRecord_Close(g_StandbyNexusHandles.fileRec[id]);

    g_DeviceState.record_started[id] = false;
#endif
}


int decode_start(unsigned id)
{
    NEXUS_Error rc=0;

    if(g_DeviceState.decode_started[id]) {
        BDBG_WRN(("Video Decoder %u already started", id));
        return rc;
    }

    /* Start decoders */
    if(g_StandbyNexusHandles.videoDecoder[id] && g_DeviceState.opts[id].videoPid) {
        rc = NEXUS_VideoDecoder_Start(g_StandbyNexusHandles.videoDecoder[id], &g_StandbyNexusHandles.videoProgram[id]);
        if (rc) { BERR_TRACE(rc); goto video_err;}
    }

    if(g_StandbyNexusHandles.audioDecoder[id] && g_DeviceState.opts[id].audioPid) {
        rc = NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);
        if (rc) { BERR_TRACE(rc); goto audio_err;}
    }
    g_DeviceState.decode_started[id] = true;

    return rc;

audio_err:
    NEXUS_VideoDecoder_Stop(g_StandbyNexusHandles.videoDecoder[id]);
video_err:
    return rc;
}

void decode_stop(unsigned id)
{
    if(!g_DeviceState.decode_started[id])
        return;

    /* Stop decoders */
    if(g_StandbyNexusHandles.videoDecoder[id] && g_DeviceState.opts[id].videoPid)
        NEXUS_VideoDecoder_Stop(g_StandbyNexusHandles.videoDecoder[id]);
    if(g_StandbyNexusHandles.audioDecoder[id] && g_DeviceState.opts[id].audioPid)
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

    g_DeviceState.decode_started[id] = false;
}

int picture_decode_start(void)
{
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_SurfaceCreateSettings imgSurfaceSettings;
    void *buffer;
    size_t size;
    int rc;

    if(!g_StandbyNexusHandles.pictureDecoder)
        return -1;

    NEXUS_PictureDecoder_GetBuffer(g_StandbyNexusHandles.pictureDecoder, &buffer, &size);
    rc = fread(buffer, 1, size, g_StandbyNexusHandles.picFileHandle);
    if(rc<0) {
        return -1;
    }
    NEXUS_PictureDecoder_ReadComplete(g_StandbyNexusHandles.pictureDecoder, 0, rc);

    NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
    pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    NEXUS_PictureDecoder_Start(g_StandbyNexusHandles.pictureDecoder, &pictureSettings);

    do {
        NEXUS_PictureDecoder_GetStatus(g_StandbyNexusHandles.pictureDecoder, &g_StandbyNexusHandles.imgStatus);
        if( g_StandbyNexusHandles.imgStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            return -1;
        }
        usleep(1000);
    } while(!g_StandbyNexusHandles.imgStatus.headerValid); /* wait for picture dimensions */

    NEXUS_Surface_GetDefaultCreateSettings(&imgSurfaceSettings);
    imgSurfaceSettings.pixelFormat = g_StandbyNexusHandles.imgStatus.header.format;
    imgSurfaceSettings.width       = g_StandbyNexusHandles.imgStatus.header.surface.width;
    imgSurfaceSettings.height      = g_StandbyNexusHandles.imgStatus.header.surface.height;
    imgSurfaceSettings.alignment = 2;

    switch (imgSurfaceSettings.pixelFormat) {
        case NEXUS_PixelFormat_ePalette8:
        case NEXUS_PixelFormat_eA8:
            imgSurfaceSettings.pitch = imgSurfaceSettings.width;
            break;
        case NEXUS_PixelFormat_ePalette4:
            imgSurfaceSettings.pitch = (imgSurfaceSettings.width + 1) >> 1;
            break;
        case NEXUS_PixelFormat_ePalette2:
            imgSurfaceSettings.pitch = (imgSurfaceSettings.width + 3) >> 2;
            break;
        case NEXUS_PixelFormat_ePalette1:
            imgSurfaceSettings.pitch = (imgSurfaceSettings.width + 7) >> 3;
            break;
        case NEXUS_PixelFormat_eA8_Palette8:
            imgSurfaceSettings.pitch = imgSurfaceSettings.width << 1;
            break;
        default:
            break;
    }
    imgSurfaceSettings.pitch = ((imgSurfaceSettings.pitch + 3) & ~3);
    BDBG_MSG(("creating surface: format %d, %dx%d, pitch %d", imgSurfaceSettings.pixelFormat, imgSurfaceSettings.width, imgSurfaceSettings.height, imgSurfaceSettings.pitch));

    g_StandbyNexusHandles.picSurface = NEXUS_Surface_Create(&imgSurfaceSettings);

    /* if uses a palette, copy it */
    if (NEXUS_PIXEL_FORMAT_IS_PALETTE(imgSurfaceSettings.pixelFormat)) {
        NEXUS_PictureDecoderPalette sidPalette;
        NEXUS_SurfaceMemory mem;

        rc = NEXUS_PictureDecoder_GetPalette(g_StandbyNexusHandles.pictureDecoder, &sidPalette);
        BDBG_ASSERT(!rc);
        NEXUS_Surface_GetMemory(g_StandbyNexusHandles.picSurface, &mem),
            BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
        NEXUS_Surface_Flush(g_StandbyNexusHandles.picSurface);
    }

    /* start decoding */
    NEXUS_PictureDecoder_DecodeSegment(g_StandbyNexusHandles.pictureDecoder, g_StandbyNexusHandles.picSurface, NULL);

    do {
        NEXUS_PictureDecoder_GetStatus(g_StandbyNexusHandles.pictureDecoder, &g_StandbyNexusHandles.imgStatus);
        if( g_StandbyNexusHandles.imgStatus.state == NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            return -1;
        }
        usleep(1000);
    } while( g_StandbyNexusHandles.imgStatus.state!=NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */
    NEXUS_PictureDecoder_Stop(g_StandbyNexusHandles.pictureDecoder);
#endif

    return 0;
}

void picture_decode_stop(void)
{
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_GraphicsSettings       graphicsSettings;

    NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
    graphicsSettings.enabled = false;
    NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);

    if(g_StandbyNexusHandles.picSurface)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.picSurface);
#endif
}

int picture_decode_display(void)
{
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_GraphicsSettings       graphicsSettings;
    NEXUS_VideoFormatInfo        videoFormatInfo;
    NEXUS_DisplaySettings        displaySettings;
    int rc=0;

    if(!g_StandbyNexusHandles.gfx2d) {
        printf("Graphics 2D not Opened. Cannot display picture\n");
        return -1;
    }
    if(!g_StandbyNexusHandles.picSurface) {
        printf("Invalid picture Surface\n");
        return -1;
    }
    if(!g_StandbyNexusHandles.picFrameBuffer) {
        printf("Invalid Picture Frame Buffer\n");
        return -1;
    }

    NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface     = g_StandbyNexusHandles.picSurface;
    blitSettings.source.rect.x      = 0;
    blitSettings.source.rect.y      = 0;
    blitSettings.source.rect.width  = g_StandbyNexusHandles.imgStatus.header.width;
    blitSettings.source.rect.height = g_StandbyNexusHandles.imgStatus.header.height;
    blitSettings.output.surface     = g_StandbyNexusHandles.picFrameBuffer;
    blitSettings.output.rect.width  = videoFormatInfo.width; /* fill to fit entire screen */
    blitSettings.output.rect.height = videoFormatInfo.height;
    NEXUS_Graphics2D_Blit(g_StandbyNexusHandles.gfx2d, &blitSettings);

    rc = NEXUS_Graphics2D_Checkpoint(g_StandbyNexusHandles.gfx2d, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(g_StandbyNexusHandles.checkpointEvent, BKNI_INFINITE);
        rc = 0;
    }

    NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(g_StandbyNexusHandles.displayHD, g_StandbyNexusHandles.picFrameBuffer);

    return rc;
#else
    return 0;
#endif
}

int encode_start(unsigned id)
{
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_Error rc=0;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;

    BSTD_UNUSED(id);

    if(g_DeviceState.encode_started)
        return rc;

    NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowTranscode, NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[0]));

    NEXUS_VideoEncoder_GetSettings(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = true; /* encoder can detect film content and follow CET */
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e23_976;
    videoEncoderConfig.bitrateMax = 6*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 23;
    videoEncoderConfig.streamStructure.framesB = 0;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = g_StandbyNexusHandles.displayTranscode;
    videoEncoderStartConfig.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;

    /******************************************
     * add configurable delay to video path
     */
    /* NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
     * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
    videoEncoderConfig.enableFieldPairing = true;

    /* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
    videoEncoderStartConfig.rateBufferDelay = 0;

    /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
     * Note: lower minimum framerate means longer encode delay */
    videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

    /* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
    videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
    videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

    /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
    videoEncoderStartConfig.bounds.inputDimension.max.width = 1280;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 720;

    {
        unsigned Dee;

        /* NOTE: video encoder delay is in 27MHz ticks */
        NEXUS_VideoEncoder_GetDelayRange(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
        printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

        NEXUS_AudioMuxOutput_GetDelayStatus(g_StandbyNexusHandles.audioMuxOutput, NEXUS_AudioCodec_eAac, &audioDelayStatus);
        printf("\tAudio codec AAC end-to-end delay = %u ms\n", audioDelayStatus.endToEndDelay);

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
        audioMuxStartSettings.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;
        audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
        NEXUS_AudioMuxOutput_Start(g_StandbyNexusHandles.audioMuxOutput, &audioMuxStartSettings);
    }

    NEXUS_VideoEncoder_SetSettings(g_StandbyNexusHandles.videoEncoder, &videoEncoderConfig);

    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = g_StandbyNexusHandles.stcChannelTranscode;

    muxConfig.video[0].pid = 0x11;
    muxConfig.video[0].encoder = g_StandbyNexusHandles.videoEncoder;
    muxConfig.video[0].playpump = g_StandbyNexusHandles.playpumpTranscodeVideo;

    muxConfig.audio[0].pid = 0x12;
    muxConfig.audio[0].muxOutput = g_StandbyNexusHandles.audioMuxOutput;
    muxConfig.audio[0].playpump = g_StandbyNexusHandles.playpumpTranscodeAudio;

    muxConfig.pcr.pid = 0x13;
    muxConfig.pcr.playpump = g_StandbyNexusHandles.playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;

    g_StandbyNexusHandles.pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(g_StandbyNexusHandles.playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.pidChannelTranscodePcr);

    NEXUS_StreamMux_Start(g_StandbyNexusHandles.streamMux, &muxConfig, &muxOutput);

    g_StandbyNexusHandles.pidChannelTranscodeVideo = muxOutput.video[0];
    g_StandbyNexusHandles.pidChannelTranscodeAudio = muxOutput.audio[0];

    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodeVideo, &recordPidSettings);
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodeAudio, NULL);
    NEXUS_Record_AddPidChannel(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.pidChannelTranscodePcr, NULL);

    g_StandbyNexusHandles.fileTranscode = NEXUS_FileRecord_OpenPosix("videos/encode.mpg", "videos/encode.nav");
    BDBG_ASSERT(g_StandbyNexusHandles.fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(g_StandbyNexusHandles.recordTranscode, g_StandbyNexusHandles.fileTranscode);

    NEXUS_VideoEncoder_Start(g_StandbyNexusHandles.videoEncoder, &videoEncoderStartConfig);

    g_DeviceState.encode_started = true;

    return rc;
#else
    BSTD_UNUSED(id);
    return 0;
#endif
}

void encode_stop(unsigned id)
{
    BSTD_UNUSED(id);

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if(!g_DeviceState.encode_started)
        return;

    NEXUS_AudioMuxOutput_Stop(g_StandbyNexusHandles.audioMuxOutput);
    NEXUS_VideoEncoder_Stop(g_StandbyNexusHandles.videoEncoder, NULL);

    NEXUS_StreamMux_Finish(g_StandbyNexusHandles.streamMux);
    if(BKNI_WaitForEvent(g_StandbyNexusHandles.finishEvent, 2000)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }

    NEXUS_Record_Stop(g_StandbyNexusHandles.recordTranscode);
    NEXUS_Record_RemoveAllPidChannels(g_StandbyNexusHandles.recordTranscode);
    NEXUS_StreamMux_Stop(g_StandbyNexusHandles.streamMux);
    NEXUS_FileRecord_Close(g_StandbyNexusHandles.fileTranscode);

    NEXUS_Playpump_CloseAllPidChannels(g_StandbyNexusHandles.playpumpTranscodePcr);

    NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowTranscode, NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[0]));

    g_DeviceState.encode_started = false;
#endif
}

int graphics2d_start(void)
{
    if(g_DeviceState.graphics2d_started)
        return 0;

    g_DeviceState.stop_graphics2d=false;
    pthread_create(&gfx2d_thread, NULL, graphics2d_thread, NULL);

    g_DeviceState.graphics2d_started = true;

    return 0;
}

void graphics2d_stop(void)
{
    if(!g_DeviceState.graphics2d_started)
        return;

    g_DeviceState.stop_graphics2d=true;
    pthread_join(gfx2d_thread, NULL);

    g_DeviceState.graphics2d_started = false;
}

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

void ir_open(void)
{
    NEXUS_IrInputSettings irSettings;
    NEXUS_IrInputDataFilter irPattern;

    if(!g_StandbyNexusHandles.irHandle) {
        NEXUS_IrInput_GetDefaultSettings(&irSettings);
        irSettings.mode = NEXUS_IrInputMode_eRemoteA;
        irSettings.dataReady.callback = irCallback;
        irSettings.dataReady.context = &g_StandbyNexusHandles.irHandle;
        g_StandbyNexusHandles.irHandle = NEXUS_IrInput_Open(0, &irSettings);
        BDBG_ASSERT(g_StandbyNexusHandles.irHandle);

        NEXUS_IrInput_GetDefaultDataFilter(&irPattern );
        irPattern.filterWord[0].patternWord = S0_IR_CODE; /*power*/
        irPattern.filterWord[0].enabled = true;
        irPattern.filterWord[0].mask = ~0xFFFF;
#if 0
	/* enable this to verify that can use two different IR keys to wake up. */
	/* use 'ir_last_key' to get the key that was pressed to wake-up. */
        irPattern.filterWord[1].patternWord = SO_IR_CODE; /*ok*/
        irPattern.filterWord[1].enabled = true;
        irPattern.filterWord[1].mask = ~0xFFFF;
#endif
        NEXUS_IrInput_SetDataFilter(g_StandbyNexusHandles.irHandle, &irPattern);
    }
}

void ir_close(void)
{
    if(g_StandbyNexusHandles.irHandle)
        NEXUS_IrInput_Close(g_StandbyNexusHandles.irHandle);
    g_StandbyNexusHandles.irHandle = NULL;
}

void ir_last_key(uint32_t *code, uint32_t *codeHigh)
{
    NEXUS_IrInputEvent pEvent;
    NEXUS_IrInput_ReadEvent(g_StandbyNexusHandles.irHandle, &pEvent);
    *code = pEvent.code;
    *codeHigh = pEvent.codeHigh;
}

void uhf_open(void)
{
#if NEXUS_HAS_UHF_INPUT
    NEXUS_UhfInputSettings uhfSettings;

    if(!g_StandbyNexusHandles.uhfHandle) {
        NEXUS_UhfInput_GetDefaultSettings(&uhfSettings);
        uhfSettings.channel = NEXUS_UhfInputMode_eChannel1;
        uhfSettings.dataReady.callback = uhfCallback;
        uhfSettings.dataReady.context = &g_StandbyNexusHandles.uhfHandle;
        g_StandbyNexusHandles.uhfHandle = NEXUS_UhfInput_Open(0, &uhfSettings);
        BDBG_ASSERT(g_StandbyNexusHandles.uhfHandle);
    }
#endif
}

void uhf_close(void)
{
#if NEXUS_HAS_UHF_INPUT
    if(g_StandbyNexusHandles.uhfHandle)
        NEXUS_UhfInput_Close(g_StandbyNexusHandles.uhfHandle);
    g_StandbyNexusHandles.uhfHandle = NULL;
#endif
}

void keypad_open(void)
{
#if NEXUS_HAS_KEYPAD
    NEXUS_KeypadSettings keypadSettings;

    if(!g_StandbyNexusHandles.keypadHandle) {
        NEXUS_Keypad_GetDefaultSettings(&keypadSettings);
        keypadSettings.dataReady.callback = keypadCallback;
        keypadSettings.dataReady.context = &g_StandbyNexusHandles.keypadHandle;
        g_StandbyNexusHandles.keypadHandle = NEXUS_Keypad_Open(0, &keypadSettings);
    }
#endif
}

void keypad_close(void)
{
#if NEXUS_HAS_KEYPAD
    if(g_StandbyNexusHandles.keypadHandle)
        NEXUS_Keypad_Close(g_StandbyNexusHandles.keypadHandle);
    g_StandbyNexusHandles.keypadHandle = NULL;
#endif
}

void display_open(unsigned id)
{
    NEXUS_DisplaySettings displaySettings;

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    switch(id) {
        case 0:
            if(!g_StandbyNexusHandles.displayHD ) {
#if NEXUS_HAS_HDMI_OUTPUT
                NEXUS_HdmiOutputStatus status;
                NEXUS_DisplayCapabilities displayCap;
                NEXUS_HdmiOutput_GetStatus(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &status);
                NEXUS_GetDisplayCapabilities(&displayCap);
                if (status.connected && displayCap.displayFormatSupported[status.preferredVideoFormat]) {
                    displaySettings.format = status.preferredVideoFormat;
                }
#else
                displaySettings.format = NEXUS_VideoFormat_e1080i;
#endif
                g_StandbyNexusHandles.displayHD = NEXUS_Display_Open(id, &displaySettings);
                BDBG_ASSERT(g_StandbyNexusHandles.displayHD);
            }
            break;
        case 1:
            if(!g_StandbyNexusHandles.displaySD ) {
                displaySettings.format = NEXUS_VideoFormat_eNtsc;
                g_StandbyNexusHandles.displaySD = NEXUS_Display_Open(id, &displaySettings);
                BDBG_ASSERT(g_StandbyNexusHandles.displaySD);
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void display_close(unsigned id)
{
    switch(id) {
        case 0:
            if(g_StandbyNexusHandles.displayHD) {
                NEXUS_Display_Close(g_StandbyNexusHandles.displayHD);
                g_StandbyNexusHandles.displayHD = NULL;
            }
            break;
        case 1:
            if(g_StandbyNexusHandles.displaySD) {
                NEXUS_Display_Close(g_StandbyNexusHandles.displaySD);
                g_StandbyNexusHandles.displaySD = NULL;
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void window_open(unsigned window_id, unsigned display_id)
{
    BDBG_ASSERT(window_id <= MAX_CONTEXTS);

    switch(display_id) {
        case 0:
            if(g_StandbyNexusHandles.displayHD) {
                g_StandbyNexusHandles.windowHD[window_id] = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displayHD, window_id);
                BDBG_ASSERT(g_StandbyNexusHandles.windowHD[window_id]);
            } else
                BDBG_WRN(("HD Display not opened"));
            break;
        case 1:
            if(g_StandbyNexusHandles.displaySD) {
                g_StandbyNexusHandles.windowSD[window_id] = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displaySD, window_id);
                BDBG_ASSERT(g_StandbyNexusHandles.windowSD[window_id]);
            } else
                BDBG_WRN(("SD Display not opened"));
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void window_close(unsigned window_id, unsigned display_id)
{
    BDBG_ASSERT(window_id <= MAX_CONTEXTS);

    switch(display_id) {
        case 0:
            if(g_StandbyNexusHandles.windowHD[window_id]) {
                NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowHD[window_id]);
                g_StandbyNexusHandles.windowHD[window_id] = NULL;
            }
            break;
        case 1:
            if(g_StandbyNexusHandles.windowSD[window_id]) {
                NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowSD[window_id]);
                g_StandbyNexusHandles.windowSD[window_id] = NULL;
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

#if NEXUS_NUM_HDMI_OUTPUTS
static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings    ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( !status.videoFormatSupported[displaySettings.format] )
    {
        BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d", status.preferredVideoFormat));
        displaySettings.format = status.preferredVideoFormat;
        NEXUS_Display_SetSettings(display, &displaySettings);
    }

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings);
    NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings);
}

static void mhl_standby_callback(void *pParam, int iParam)
{
    BSTD_UNUSED(pParam);
    BSTD_UNUSED(iParam);
    BDBG_MSG(("MHL standby callback"));

    g_DeviceState.power_mode = ePowerModeS3;
    if(g_StandbyNexusHandles.event)
        BKNI_SetEvent(g_StandbyNexusHandles.event);
}
#endif

void add_hdmi_output(void)
{
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;

    if (g_DeviceState.hdmi_connected)
        return;

    if (g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        NEXUS_Display_AddOutput(g_StandbyNexusHandles.displayHD, NEXUS_HdmiOutput_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]));

        NEXUS_HdmiOutput_GetSettings(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context = g_StandbyNexusHandles.platformConfig.outputs.hdmi[0];
        hdmiSettings.hotplugCallback.param = (int)g_StandbyNexusHandles.displayHD;

        /* MHL support */
        hdmiSettings.mhlStandbyCallback.callback = mhl_standby_callback;
        hdmiSettings.mhlStandbyCallback.context = g_StandbyNexusHandles.platformConfig.outputs.hdmi[0];
        hdmiSettings.mhlStandbyCallback.param = (int)g_StandbyNexusHandles.displayHD;

        NEXUS_HdmiOutput_SetSettings(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &hdmiSettings);


        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_HdmiOutput_GetAudioConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

        g_DeviceState.hdmi_connected = true;
    }

#endif
}

void remove_hdmi_output(void)
{
#if NEXUS_NUM_HDMI_OUTPUTS
    if (!g_DeviceState.hdmi_connected)
        return;

    if (g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displayHD, NEXUS_HdmiOutput_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]));

        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_HdmiOutput_GetAudioConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

        g_DeviceState.hdmi_connected = false;
    }
#endif
}

void add_component_output(void)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.component_connected)
        return;

    if(g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.component[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displayHD, NEXUS_ComponentOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.component[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.component_connected = true;
    }
#endif
}

void remove_component_output(void)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.component_connected)
        return;

    if(g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.component[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displayHD, NEXUS_ComponentOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.component[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.component_connected = false;
    }
#endif
}

void add_composite_output(void)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.composite_connected)
        return;

    if(g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.composite[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displaySD, NEXUS_CompositeOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.composite[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.composite_connected = true;
    }
#endif
}

void remove_composite_output(void)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.composite_connected)
        return;

    if(g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.composite[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displaySD, NEXUS_CompositeOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.composite[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.composite_connected = false;
    }
#endif
}

void add_rfm_output(void)
{
#if NEXUS_NUM_RFM_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.rfm_connected)
        return;

    if (g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.rfm[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displaySD, NEXUS_Rfm_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.rfm[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.rfm_connected = true;
    }
#endif
}

void remove_rfm_output(void)
{
#if NEXUS_NUM_RFM_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.rfm_connected)
        return;

    if (g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.rfm[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displaySD, NEXUS_Rfm_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.rfm[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.rfm_connected = false;
    }
#endif
}

void add_dac_output(unsigned id)
{
#if NEXUS_NUM_AUDIO_DACS
    if (g_DeviceState.dac_connected[id])
        return;

    if(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.dac_connected[id] = true;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void remove_dac_output(unsigned id)
{
#if NEXUS_NUM_AUDIO_DACS
    if (!g_DeviceState.dac_connected[id])
        return;

    if(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.dac_connected[id] = false;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void add_spdif_output(unsigned id)
{
#if NEXUS_NUM_SPDIF_OUTPUTS
    if (g_DeviceState.spdif_connected[id])
        return;

    if(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.spdif_connected[id] = true;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void remove_spdif_output(unsigned id)
{
#if NEXUS_NUM_SPDIF_OUTPUTS
    if (!g_DeviceState.spdif_connected[id])
        return;

    if(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.spdif_connected[id] = false;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void playback_open(unsigned id)
{
    NEXUS_PlaybackSettings playbackSettings;

    if(g_StandbyNexusHandles.playback[id])
        return;

    g_StandbyNexusHandles.playpump[id] = NEXUS_Playpump_Open(id, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.playpump[id]);
    g_StandbyNexusHandles.playback[id] = NEXUS_Playback_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.playback[id]);

    NEXUS_Playback_GetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);
    playbackSettings.playpump = g_StandbyNexusHandles.playpump[id];
    playbackSettings.stcChannel = g_StandbyNexusHandles.stcChannel[id];
    NEXUS_Playback_SetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);

#if PLAYBACK_IP_SUPPORT
    g_StandbyNexusHandles.playbackIp[id] = B_PlaybackIp_Open(NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.playbackIp[id]);
#endif
}

void playback_close(unsigned id)
{
    if(g_StandbyNexusHandles.playback[id])
        NEXUS_Playback_Destroy(g_StandbyNexusHandles.playback[id]);
    g_StandbyNexusHandles.playback[id] = NULL;
    if(g_StandbyNexusHandles.playpump[id])
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpump[id]);
    g_StandbyNexusHandles.playpump[id] = NULL;

#if PLAYBACK_IP_SUPPORT
    if (g_StandbyNexusHandles.playbackIp[id])
        B_PlaybackIp_Close(g_StandbyNexusHandles.playbackIp[id]);
    g_StandbyNexusHandles.playbackIp[id] = NULL;
#endif
}

void record_open(unsigned id)
{
#if NEXUS_HAS_RECORD
    NEXUS_RecordSettings recordSettings;

    if(g_StandbyNexusHandles.record[id])
        return;

    g_StandbyNexusHandles.recpump[id] = NEXUS_Recpump_Open(id, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.recpump[id]);
    g_StandbyNexusHandles.record[id] = NEXUS_Record_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.record[id]);

    NEXUS_Record_GetSettings(g_StandbyNexusHandles.record[id], &recordSettings);
    recordSettings.recpump = g_StandbyNexusHandles.recpump[id];
    NEXUS_Record_SetSettings(g_StandbyNexusHandles.record[id], &recordSettings);
#endif
}

void record_close(unsigned id)
{
#if NEXUS_HAS_RECORD
    if(g_StandbyNexusHandles.record[id])
        NEXUS_Record_Destroy(g_StandbyNexusHandles.record[id]);
    g_StandbyNexusHandles.record[id] = NULL;
    if(g_StandbyNexusHandles.recpump[id])
        NEXUS_Recpump_Close(g_StandbyNexusHandles.recpump[id]);
    g_StandbyNexusHandles.recpump[id] = NULL;
#endif
}

void stc_channel_open(unsigned id)
{
    NEXUS_StcChannelSettings stcSettings;

    if(g_StandbyNexusHandles.stcChannel[id])
        return;

    NEXUS_StcChannel_GetDefaultSettings(id, &stcSettings);
    stcSettings.timebase = (id==0)?NEXUS_Timebase_e0:NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    g_StandbyNexusHandles.stcChannel[id] = NEXUS_StcChannel_Open(id, &stcSettings);
}

void stc_channel_close(unsigned id)
{
    if(g_StandbyNexusHandles.stcChannel[id])
        NEXUS_StcChannel_Close(g_StandbyNexusHandles.stcChannel[id]);
    g_StandbyNexusHandles.stcChannel[id] = NULL;
}

void graphics2d_open(void)
{
    NEXUS_Graphics2DSettings gfxSettings;

    if(g_StandbyNexusHandles.gfx2d)
        return;

    BKNI_CreateEvent(&g_StandbyNexusHandles.checkpointEvent);
    BKNI_CreateEvent(&g_StandbyNexusHandles.spaceAvailableEvent);

    g_StandbyNexusHandles.gfx2d = NEXUS_Graphics2D_Open(0, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.gfx2d);
    NEXUS_Graphics2D_GetSettings(g_StandbyNexusHandles.gfx2d, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = g_StandbyNexusHandles.checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = g_StandbyNexusHandles.spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(g_StandbyNexusHandles.gfx2d, &gfxSettings);
}

void graphics2d_close(void)
{
    if(g_StandbyNexusHandles.gfx2d)
        NEXUS_Graphics2D_Close(g_StandbyNexusHandles.gfx2d);
    g_StandbyNexusHandles.gfx2d = NULL;
    if(g_StandbyNexusHandles.checkpointEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.checkpointEvent);
    g_StandbyNexusHandles.checkpointEvent = NULL;
    if(g_StandbyNexusHandles.spaceAvailableEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.spaceAvailableEvent);
    g_StandbyNexusHandles.spaceAvailableEvent = NULL;
}

void graphics2d_setup(void)
{
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_DisplayCapabilities displayCap;

    NEXUS_GetDisplayCapabilities(&displayCap);

    if(displayCap.display[0].graphics.width &&
       displayCap.display[0].graphics.height &&
       g_StandbyNexusHandles.displayHD) {
        NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        if(g_StandbyNexusHandles.windowHD[0]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowHD[0], &windowSettings);
            windowSettings.position.x = 0;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowHD[0], &windowSettings);
        }
        if(g_StandbyNexusHandles.windowHD[1]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowHD[1], &windowSettings);
            windowSettings.position.x = videoFormatInfo.width/2;
            windowSettings.position.y = videoFormatInfo.height/2;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowHD[1], &windowSettings);
        }

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = displayCap.display[0].graphics.width;
        createSettings.height = displayCap.display[0].graphics.height;
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
        g_StandbyNexusHandles.framebufferHD = NEXUS_Surface_Create(&createSettings);
        g_StandbyNexusHandles.offscreenHD = NEXUS_Surface_Create(&createSettings);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.framebufferHD;
        fillSettings.rect.width = createSettings.width;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = 0xFF0000FF;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.offscreenHD;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);

        NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.position.width = videoFormatInfo.width;
        graphicsSettings.position.height = videoFormatInfo.height;
        graphicsSettings.clip.width = createSettings.width;
        graphicsSettings.clip.height = createSettings.height;
        NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(g_StandbyNexusHandles.displayHD, g_StandbyNexusHandles.framebufferHD);
    }

    if(displayCap.display[1].graphics.width &&
       displayCap.display[1].graphics.height &&
       g_StandbyNexusHandles.displaySD) {
        NEXUS_Display_GetSettings(g_StandbyNexusHandles.displaySD, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        if(g_StandbyNexusHandles.windowSD[0]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowSD[0], &windowSettings);
            windowSettings.position.x = 0;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowSD[0], &windowSettings);
        }
        if(g_StandbyNexusHandles.windowSD[1]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowSD[1], &windowSettings);
            windowSettings.position.x = videoFormatInfo.width/2;
            windowSettings.position.y = videoFormatInfo.height/2;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowSD[1], &windowSettings);
        }

        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width =  displayCap.display[1].graphics.width;
        createSettings.height = displayCap.display[1].graphics.height;
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(1);
        g_StandbyNexusHandles.framebufferSD = NEXUS_Surface_Create(&createSettings);
        g_StandbyNexusHandles.offscreenSD = NEXUS_Surface_Create(&createSettings);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.framebufferSD;
        fillSettings.rect.width = createSettings.width;
        fillSettings.rect.height = createSettings.height;
        fillSettings.color = 0xFF0000FF;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);
        fillSettings.surface = g_StandbyNexusHandles.offscreenSD;
        NEXUS_Graphics2D_Fill(g_StandbyNexusHandles.gfx2d, &fillSettings);

        NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displaySD, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.position.width = videoFormatInfo.width;
        graphicsSettings.position.height = videoFormatInfo.height;
        graphicsSettings.clip.width = createSettings.width;
        graphicsSettings.clip.height = createSettings.height;
        NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displaySD, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(g_StandbyNexusHandles.displaySD, g_StandbyNexusHandles.framebufferSD);
    }
}

void graphics2d_destroy(void)
{
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_VideoWindowSettings windowSettings;

    if(g_StandbyNexusHandles.displayHD) {
        NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        if(g_StandbyNexusHandles.windowHD[0]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowHD[0], &windowSettings);
            windowSettings.position.x = 0;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width;
            windowSettings.position.height = videoFormatInfo.height;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowHD[0], &windowSettings);
        }
        if(g_StandbyNexusHandles.windowHD[1]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowHD[1], &windowSettings);
            windowSettings.position.x = videoFormatInfo.width/2;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowHD[1], &windowSettings);
        }
        NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
        graphicsSettings.enabled = false;
        NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displayHD, &graphicsSettings);
    }
    if(g_StandbyNexusHandles.displaySD) {
        NEXUS_Display_GetSettings(g_StandbyNexusHandles.displaySD, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        if(g_StandbyNexusHandles.windowSD[0]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowSD[0], &windowSettings);
            windowSettings.position.x = 0;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width;
            windowSettings.position.height = videoFormatInfo.height;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowSD[0], &windowSettings);
        }
        if(g_StandbyNexusHandles.windowSD[1]) {
            NEXUS_VideoWindow_GetSettings(g_StandbyNexusHandles.windowSD[1], &windowSettings);
            windowSettings.position.x = videoFormatInfo.width/2;
            windowSettings.position.y = 0;
            windowSettings.position.width = videoFormatInfo.width/2;
            windowSettings.position.height = videoFormatInfo.height/2;
            windowSettings.alpha = 0xFF;
            NEXUS_VideoWindow_SetSettings(g_StandbyNexusHandles.windowSD[1], &windowSettings);
        }
        NEXUS_Display_GetGraphicsSettings(g_StandbyNexusHandles.displaySD, &graphicsSettings);
        graphicsSettings.enabled = false;
        NEXUS_Display_SetGraphicsSettings(g_StandbyNexusHandles.displaySD, &graphicsSettings);
    }

    if(g_StandbyNexusHandles.offscreenHD)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.offscreenHD);
    g_StandbyNexusHandles.offscreenHD = NULL;
    if(g_StandbyNexusHandles.framebufferHD)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.framebufferHD);
    g_StandbyNexusHandles.framebufferHD = NULL;
    if(g_StandbyNexusHandles.offscreenSD)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.offscreenSD);
    g_StandbyNexusHandles.offscreenSD = NULL;
    if(g_StandbyNexusHandles.framebufferSD)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.framebufferSD);
    g_StandbyNexusHandles.framebufferSD = NULL;
}

void graphics3d_open(void)
{

}

void graphics3d_close(void)
{

}

void decoder_open(unsigned id)
{
    /* bring up video decoder */
    g_StandbyNexusHandles.videoDecoder[id] = NEXUS_VideoDecoder_Open(id, NULL); /* take default capabilities */
    if(!g_StandbyNexusHandles.videoDecoder[id]) {
        BDBG_WRN(("Video Decoder %u not supported", id));
    }

    /* Bring up audio decoders */
    g_StandbyNexusHandles.audioDecoder[id] = NEXUS_AudioDecoder_Open(id, NULL);
    if(g_StandbyNexusHandles.audioDecoder[id]) {
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    /* Add dummy output, so that decoder can be started without an actual output */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDummyOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDummy[id]),
            NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    } else {
        BDBG_WRN(("Audio Decoder %u not supported", id));
    }

}

void decoder_close(unsigned id)
{
    if(g_StandbyNexusHandles.videoDecoder[id]) {
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[id]));
        NEXUS_VideoDecoder_Close(g_StandbyNexusHandles.videoDecoder[id]);
    }
    g_StandbyNexusHandles.videoDecoder[id] = NULL;

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]));
#endif
    if(g_StandbyNexusHandles.audioDecoder[id]) {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioDecoder_Close(g_StandbyNexusHandles.audioDecoder[id]);
    }
    g_StandbyNexusHandles.audioDecoder[id] = NULL;

}

void set_max_decode_size(unsigned id)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    if(!g_StandbyNexusHandles.videoDecoder[id])
        return;

    NEXUS_VideoDecoder_GetSettings(g_StandbyNexusHandles.videoDecoder[id], &videoDecoderSettings);
    if(g_DeviceState.opts[id].width > videoDecoderSettings.maxWidth ||
       g_DeviceState.opts[id].height > videoDecoderSettings.maxHeight) {
        videoDecoderSettings.maxWidth = g_DeviceState.opts[id].width ;
        videoDecoderSettings.maxHeight = g_DeviceState.opts[id].height;
        rc = NEXUS_VideoDecoder_SetSettings(g_StandbyNexusHandles.videoDecoder[id], &videoDecoderSettings);
        BDBG_ASSERT(!rc);
    }
}

void picture_decoder_open(void)
{
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplaySettings        displaySettings;
    NEXUS_VideoFormatInfo        videoFormatInfo;

    if(g_DeviceState.picfile == NULL) {
        printf("Input file not specified\n");
    }

    g_StandbyNexusHandles.picFileHandle = fopen(g_DeviceState.picfile, "rb");
    if(!g_StandbyNexusHandles.picFileHandle) {
        BDBG_ERR(("File %s not found", g_DeviceState.picfile));
        goto file_err;
    }
    fseek(g_StandbyNexusHandles.picFileHandle, 0, SEEK_END); /* seek to end of file so we can get filesize */

    NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
    decoderSettings.multiScanBufferSize = 128*1024;
    decoderSettings.bufferSize = ftell(g_StandbyNexusHandles.picFileHandle);
    g_StandbyNexusHandles.pictureDecoder = NEXUS_PictureDecoder_Open(NEXUS_ANY_ID, &decoderSettings);
    if (!g_StandbyNexusHandles.pictureDecoder) {
        BDBG_ERR(("Failed to open picture decoder !!"));
        goto picture_err;
    }
    fseek(g_StandbyNexusHandles.picFileHandle, 0, SEEK_SET);

    NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = videoFormatInfo.width;
    createSettings.height = videoFormatInfo.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    g_StandbyNexusHandles.picFrameBuffer = NEXUS_Surface_Create(&createSettings);
    if(!g_StandbyNexusHandles.picFrameBuffer) {
        BDBG_ERR(("Failed to create picture framebuffer"));
        goto surface_err;
    }

    return;

surface_err:
    if(g_StandbyNexusHandles.pictureDecoder)
        NEXUS_PictureDecoder_Close(g_StandbyNexusHandles.pictureDecoder);
picture_err:
    if(g_StandbyNexusHandles.picFileHandle)
        fclose(g_StandbyNexusHandles.picFileHandle);
file_err:
    return;
#endif
}

void picture_decoder_close(void)
{
#if NEXUS_HAS_PICTURE_DECODER
    if(g_StandbyNexusHandles.picFrameBuffer)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.picFrameBuffer);
    if(g_StandbyNexusHandles.pictureDecoder)
        NEXUS_PictureDecoder_Close(g_StandbyNexusHandles.pictureDecoder);
    if(g_StandbyNexusHandles.picFileHandle)
        fclose(g_StandbyNexusHandles.picFileHandle);
#endif
}

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
static void transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}
#endif

void encoder_open(unsigned id)
{
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_RecordSettings recordSettings;

    BSTD_UNUSED(id);

    NEXUS_StcChannel_GetDefaultSettings(2, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    g_StandbyNexusHandles.stcChannelTranscode = NEXUS_StcChannel_Open(2, &stcSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.stcChannelTranscode);

    g_StandbyNexusHandles.audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.audioMuxOutput);

    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAac;
    g_StandbyNexusHandles.audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.audioEncoder);

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

    NEXUS_AudioEncoder_AddInput(g_StandbyNexusHandles.audioEncoder,
            NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput),
            NEXUS_AudioEncoder_GetConnector(g_StandbyNexusHandles.audioEncoder));

    if(g_DeviceState.decode_started[id])
        NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

    g_StandbyNexusHandles.videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.videoEncoder);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p;
    g_StandbyNexusHandles.displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);/* cmp3 for transcoder */
    BDBG_ASSERT(g_StandbyNexusHandles.displayTranscode);

    g_StandbyNexusHandles.windowTranscode = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displayTranscode, 0);
    BDBG_ASSERT(g_StandbyNexusHandles.windowTranscode);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    g_StandbyNexusHandles.playpumpTranscodeVideo = NEXUS_Playpump_Open(2, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodeVideo);
    g_StandbyNexusHandles.playpumpTranscodeAudio = NEXUS_Playpump_Open(3, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodeAudio);
    g_StandbyNexusHandles.playpumpTranscodePcr = NEXUS_Playpump_Open(4, &playpumpConfig);
    BDBG_ASSERT(g_StandbyNexusHandles.playpumpTranscodePcr);

    g_StandbyNexusHandles.recpumpTranscode = NEXUS_Recpump_Open(2, NULL);
    BDBG_ASSERT(g_StandbyNexusHandles.recpumpTranscode);
    g_StandbyNexusHandles.recordTranscode = NEXUS_Record_Create();
    BDBG_ASSERT(g_StandbyNexusHandles.recordTranscode);
    NEXUS_Record_GetSettings(g_StandbyNexusHandles.recordTranscode, &recordSettings);
    recordSettings.recpump = g_StandbyNexusHandles.recpumpTranscode;
    NEXUS_Record_SetSettings(g_StandbyNexusHandles.recordTranscode, &recordSettings);

    BKNI_CreateEvent(&g_StandbyNexusHandles.finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = g_StandbyNexusHandles.finishEvent;
    g_StandbyNexusHandles.streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    BDBG_ASSERT(g_StandbyNexusHandles.streamMux);
#else
    BSTD_UNUSED(id);
#endif
}

void encoder_close(unsigned id)
{
    BSTD_UNUSED(id);

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if(g_StandbyNexusHandles.streamMux)
        NEXUS_StreamMux_Destroy(g_StandbyNexusHandles.streamMux);
    g_StandbyNexusHandles.streamMux = NULL;

    if(g_StandbyNexusHandles.finishEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.finishEvent);
    g_StandbyNexusHandles.finishEvent = NULL;

    if(g_StandbyNexusHandles.recordTranscode)
        NEXUS_Record_Destroy(g_StandbyNexusHandles.recordTranscode);
    g_StandbyNexusHandles.recordTranscode = NULL;
    if(g_StandbyNexusHandles.recpumpTranscode)
        NEXUS_Recpump_Close(g_StandbyNexusHandles.recpumpTranscode);
    g_StandbyNexusHandles.recpumpTranscode = NULL;

    if(g_StandbyNexusHandles.playpumpTranscodePcr)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodePcr);
    g_StandbyNexusHandles.playpumpTranscodePcr = NULL;
    if(g_StandbyNexusHandles.playpumpTranscodeAudio)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodeAudio);
    g_StandbyNexusHandles.playpumpTranscodeAudio = NULL;
    if(g_StandbyNexusHandles.playpumpTranscodeVideo)
        NEXUS_Playpump_Close(g_StandbyNexusHandles.playpumpTranscodeVideo);
    g_StandbyNexusHandles.playpumpTranscodeVideo = NULL;

    if(g_StandbyNexusHandles.windowTranscode)
        NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowTranscode);
    g_StandbyNexusHandles.windowTranscode = NULL;
    if(g_StandbyNexusHandles.displayTranscode)
        NEXUS_Display_Close(g_StandbyNexusHandles.displayTranscode);
    g_StandbyNexusHandles.displayTranscode = NULL;

    if(g_StandbyNexusHandles.videoEncoder)
        NEXUS_VideoEncoder_Close(g_StandbyNexusHandles.videoEncoder);
    g_StandbyNexusHandles.videoEncoder = NULL;

    if(g_StandbyNexusHandles.audioMuxOutput) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput));
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(g_StandbyNexusHandles.audioMuxOutput));
        NEXUS_AudioMuxOutput_Destroy(g_StandbyNexusHandles.audioMuxOutput);
    }
    g_StandbyNexusHandles.audioMuxOutput = NULL;

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

    if(g_StandbyNexusHandles.audioEncoder) {
        NEXUS_AudioEncoder_RemoveAllInputs(g_StandbyNexusHandles.audioEncoder);
        NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(g_StandbyNexusHandles.audioEncoder));
        NEXUS_AudioEncoder_Close(g_StandbyNexusHandles.audioEncoder);
    }
    g_StandbyNexusHandles.audioEncoder = NULL;

    if(g_DeviceState.decode_started[0])
        NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[0]);

    if(g_StandbyNexusHandles.stcChannelTranscode)
        NEXUS_StcChannel_Close(g_StandbyNexusHandles.stcChannelTranscode);
    g_StandbyNexusHandles.stcChannelTranscode = NULL;
#endif
}

void add_window_input(unsigned decoder_id)
{
    BDBG_ASSERT(decoder_id <= MAX_CONTEXTS);

    if(!g_StandbyNexusHandles.videoDecoder[decoder_id])
        return;

    if(g_DeviceState.decoder_connected[decoder_id])
        return;

    if(g_StandbyNexusHandles.windowHD[decoder_id])
        NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowHD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));
    if(g_StandbyNexusHandles.windowSD[decoder_id])
        NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowSD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));

    g_DeviceState.decoder_connected[decoder_id] = true;
}

void remove_window_input(unsigned decoder_id)
{
    BDBG_ASSERT(decoder_id <= MAX_CONTEXTS);

    if(!g_StandbyNexusHandles.videoDecoder[decoder_id])
        return;

    if(!g_DeviceState.decoder_connected[decoder_id])
        return;

    if(g_StandbyNexusHandles.windowHD[decoder_id])
        NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowHD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));
    if(g_StandbyNexusHandles.windowSD[decoder_id])
        NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowSD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));

    g_DeviceState.decoder_connected[decoder_id] = false;
}

void decoder_p_setup(unsigned id)
{
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_StandbyNexusHandles.videoProgram[id]);
    g_StandbyNexusHandles.videoProgram[id].codec = g_DeviceState.opts[id].videoCodec;
    g_StandbyNexusHandles.videoProgram[id].pidChannel = g_StandbyNexusHandles.videoPidChannel[id];
    g_StandbyNexusHandles.videoProgram[id].stcChannel = g_StandbyNexusHandles.stcChannel[id];
    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_StandbyNexusHandles.audioProgram[id]);
    g_StandbyNexusHandles.audioProgram[id].codec = g_DeviceState.opts[id].audioCodec;
    g_StandbyNexusHandles.audioProgram[id].pidChannel = g_StandbyNexusHandles.audioPidChannel[id];
    g_StandbyNexusHandles.audioProgram[id].stcChannel = g_StandbyNexusHandles.stcChannel[id];
}

void live_p_setup(unsigned id)
{
    NEXUS_StcChannelSettings stcSettings;

    g_StandbyNexusHandles.videoPidChannel[id] = NEXUS_PidChannel_Open(g_StandbyNexusHandles.parserBand[id], g_DeviceState.opts[id].videoPid, NULL);
    g_StandbyNexusHandles.audioPidChannel[id] = NEXUS_PidChannel_Open(g_StandbyNexusHandles.parserBand[id], g_DeviceState.opts[id].audioPid, NULL);

    NEXUS_StcChannel_GetSettings(g_StandbyNexusHandles.stcChannel[id], &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = g_StandbyNexusHandles.videoPidChannel[id];
    NEXUS_StcChannel_SetSettings(g_StandbyNexusHandles.stcChannel[id], &stcSettings);

    if(g_DeviceState.power_mode == ePowerModeS0)
        decoder_p_setup(id);
}

void playback_p_setup(unsigned id)
{
#if PLAYBACK_IP_SUPPORT
    if (g_DeviceState.playbackIpActive[id] && (g_DeviceState.playbackIpLiveMode[id] || g_DeviceState.playbackIpPsi[id].hlsSessionEnabled || g_DeviceState.playbackIpPsi[id].mpegDashSessionEnabled || g_DeviceState.playbackIpPsi[id].numPlaySpeedEntries)) {
        NEXUS_PlaypumpSettings playpumpSettings;
        B_PlaybackIpSettings playbackIpSettings;
        NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;

        NEXUS_Playpump_GetSettings(g_StandbyNexusHandles.playpump[id], &playpumpSettings);
        playpumpSettings.transportType = g_DeviceState.opts[id].transportType;
        NEXUS_Playpump_SetSettings(g_StandbyNexusHandles.playpump[id], &playpumpSettings);

        B_PlaybackIp_GetSettings(g_StandbyNexusHandles.playbackIp[id], &playbackIpSettings);
        playbackIpSettings.useNexusPlaypump = true;
        B_PlaybackIp_SetSettings(g_StandbyNexusHandles.playbackIp[id], &playbackIpSettings);

        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eVideo;
        g_StandbyNexusHandles.videoPidChannel[id] = NEXUS_Playpump_OpenPidChannel(g_StandbyNexusHandles.playpump[id], g_DeviceState.opts[id].videoPid, &pidChannelSettings);

        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eAudio;
        g_StandbyNexusHandles.audioPidChannel[id] = NEXUS_Playpump_OpenPidChannel(g_StandbyNexusHandles.playpump[id], g_DeviceState.opts[id].audioPid, &pidChannelSettings);
    } else
#endif
    {
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        NEXUS_Playback_GetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);
        playbackSettings.playpumpSettings.transportType = g_DeviceState.opts[id].transportType;
        NEXUS_Playback_SetSettings(g_StandbyNexusHandles.playback[id], &playbackSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = g_DeviceState.opts[id].videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = g_StandbyNexusHandles.videoDecoder[id];
        g_StandbyNexusHandles.videoPidChannel[id] = NEXUS_Playback_OpenPidChannel(g_StandbyNexusHandles.playback[id], g_DeviceState.opts[id].videoPid, &playbackPidSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = g_StandbyNexusHandles.audioDecoder[id];
        g_StandbyNexusHandles.audioPidChannel[id] = NEXUS_Playback_OpenPidChannel(g_StandbyNexusHandles.playback[id], g_DeviceState.opts[id].audioPid, &playbackPidSettings);
    }

    if(g_DeviceState.power_mode == ePowerModeS0)
        decoder_p_setup(id);
}

int live_setup(unsigned id)
{
    psiCollectionDataType collectionData;
    int numProgramsFound;
    struct opts_t psi[MAX_PROGRAMS_PER_FREQUENCY];
    NEXUS_Error rc;

    switch(g_DeviceState.source[id]) {
#if NEXUS_HAS_FRONTEND
        case eInputSourceQam:
            BKNI_ResetEvent(g_StandbyNexusHandles.signalLockedEvent);
            rc = tune_qam(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("%s: Frontend failed to lock QAM signal during PSI acquisition"));
                BERR_TRACE(rc); goto err;
            }
            break;
        case eInputSourceSat:
            rc = tune_sat(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("%s: Frontend failed to lock SAT signal during PSI acquisition"));
                BERR_TRACE(rc); goto err;
            }
            break;
        case eInputSourceOfdm:
            rc = tune_ofdm(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("%s: Frontend failed to lock OFDM signal during PSI acquisition"));
                BERR_TRACE(rc); goto err;
            }
            break;
#endif
        case eInputSourceStreamer:
            rc = streamer_start(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            break;
        default:
            BDBG_WRN(("Unknown Live Source\n"));
            rc = NEXUS_UNKNOWN; rc = BERR_TRACE(rc); goto err;
    }

    /* Acquire PSI info */
    BKNI_Memset(&collectionData, 0, sizeof(collectionData));
    BKNI_Memset(psi, 0, sizeof(psi));
    collectionData.parserBand = g_StandbyNexusHandles.parserBand[id];
    acquirePsiInfo(&collectionData, psi, &numProgramsFound);
    g_DeviceState.opts[id] = psi[id];

    live_p_setup(id);

err:
    return rc;
}

int playback_setup(unsigned id)
{
    NEXUS_Error rc;

    /* Probe file */
    rc = probe(id);
    if(rc) { rc = BERR_TRACE(rc); goto err; }

    playback_p_setup(id);

err:
    return rc;
}

int start_live_context(unsigned id)
{
    NEXUS_Error rc;

    if(!g_DeviceState.openfe && g_DeviceState.source[id] != eInputSourceStreamer) {
        printf("Initializing Frontend ...\n");
        rc = NEXUS_Platform_InitFrontend();
        if(rc) { rc = BERR_TRACE(rc); goto live_err; }
        g_DeviceState.openfe = true;
    }
    rc = live_setup(id);
    if(rc) { rc = BERR_TRACE(rc); goto live_err; }

    if(g_DeviceState.power_mode == ePowerModeS0) {
        add_window_input(id);
        rc = decode_start(id);
        if(rc) { rc = BERR_TRACE(rc); goto decode_err; }
    } else {
        /* We might be in S1 mode and cannot start decoder upon resume
           Set the input source to none, so app does not attempt to star decode */
        g_DeviceState.source[id] = eInputSourceNone;
    }

    return rc;

decode_err:
    stop_live_context(id);
live_err:
    return rc;
}

void stop_live_context(unsigned id)
{
#if NEXUS_HAS_FRONTEND
    untune_frontend(id);
#endif

    if(g_DeviceState.power_mode == ePowerModeS0) {
        decode_stop(id);
        remove_window_input(id);
    }
    g_DeviceState.source[id] = eInputSourceNone;
}

int start_play_context(unsigned id)
{
    NEXUS_Error rc;

    if(g_DeviceState.playfile[id] == NULL) {
        BDBG_WRN(("No input file specified\n"));
        rc = NEXUS_UNKNOWN; rc = BERR_TRACE(rc); goto playback_err;
    }

    printf("Playing file %s\n", g_DeviceState.playfile[id]);

    rc = playback_setup(id);
    if(rc) { rc = BERR_TRACE(rc); goto playback_err; }

    rc = playback_start(id);
    if(rc) { rc = BERR_TRACE(rc); goto playback_err; }

    if(g_DeviceState.power_mode == ePowerModeS0) {
        set_max_decode_size(id);
        add_window_input(id);
        rc = decode_start(id);
        if(rc) { rc = BERR_TRACE(rc); goto decode_err; }
    } else {
        /* We might be in S1 mode and cannot start decoder upon resume
           Set the input source to none, so app does not attempt to star decode */
        g_DeviceState.source[id] = eInputSourceNone;
    }

    return rc;

decode_err:
    stop_play_context(id);
playback_err:
    return rc;
}

void stop_play_context(unsigned id)
{
    playback_stop(id);
    if(g_DeviceState.power_mode == ePowerModeS0) {
        decode_stop(id);
        remove_window_input(id);
    }
    g_DeviceState.source[id] = eInputSourceNone;
    g_DeviceState.playfile[id] = NULL;
}

void stop_decodes()
{
    unsigned i;

    for(i=0; i<MAX_CONTEXTS; i++) {
        stop_play_context(i);
        stop_live_context(i);
    }
}

void add_outputs(void)
{
    add_hdmi_output();
    add_component_output();
    add_composite_output();
    add_rfm_output();

    add_dac_output(0);
    add_spdif_output(0);
}

void remove_outputs(void)
{
    remove_hdmi_output();
    remove_component_output();
    remove_composite_output();
    remove_rfm_output();

    remove_dac_output(0);
    remove_spdif_output(0);
}

int start_app(void)
{
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_VideoDecoderCapabilities videoCap;
    unsigned i;

    NEXUS_Platform_GetConfiguration(&g_StandbyNexusHandles.platformConfig);

    BKNI_CreateEvent(&g_StandbyNexusHandles.event);
    BKNI_CreateEvent(&g_StandbyNexusHandles.s1Event);
    BKNI_CreateEvent(&g_StandbyNexusHandles.signalLockedEvent);

    ir_open();
    uhf_open();
    /*keypad_open();*/

    NEXUS_GetDisplayCapabilities(&displayCap);
    if(displayCap.display[0].numVideoWindows)
        display_open(0);
    if(displayCap.display[1].numVideoWindows)
        display_open(1);

    NEXUS_GetVideoDecoderCapabilities(&videoCap);
    g_DeviceState.num_contexts = videoCap.numVideoDecoders<=MAX_CONTEXTS?videoCap.numVideoDecoders:MAX_CONTEXTS;

    for(i=0; i<g_DeviceState.num_contexts ; i++) {
        if(i < displayCap.display[0].numVideoWindows)
            window_open(i, 0); /* window on display 0 */
        if(i < displayCap.display[1].numVideoWindows)
            window_open(i, 1); /* window on display 1 */

        if(i < videoCap.numVideoDecoders)
            decoder_open(i);

        stc_channel_open(i);

        playback_open(i);
        record_open(i);

        g_StandbyNexusHandles.parserBand[i] = (i==0)?NEXUS_ParserBand_e0:NEXUS_ParserBand_e1;
    }

#if NEXUS_HAS_CEC && NEXUS_HAS_HDMI_OUTPUT
    cec_setup();
#endif

    return NEXUS_SUCCESS;
}

void stop_app(void)
{
    unsigned i;

    /* Bring down system */
    graphics2d_stop();
    graphics2d_destroy();
    graphics2d_close();

    encode_stop(0);

    for(i=0; i<g_DeviceState.num_contexts; i++) {
        decode_stop(i);

#if NEXUS_HAS_FRONTEND
        untune_frontend(i);
#endif

        playback_stop(i);
        record_stop(i);

        playback_close(i);
        record_close(i);

        stc_channel_close(i);

        decoder_close(i);

        encoder_close(i);

        window_close(i, 0); /* window on display 0 */
        window_close(i, 1); /* window on display 1 */
    }

    display_close(0);
    display_close(1);

    /*keypad_close();*/
    uhf_close();
    ir_close();

    if(g_StandbyNexusHandles.event)
        BKNI_DestroyEvent(g_StandbyNexusHandles.event);

    if(g_StandbyNexusHandles.s1Event)
        BKNI_DestroyEvent(g_StandbyNexusHandles.s1Event);

    if(g_StandbyNexusHandles.signalLockedEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.signalLockedEvent);
}

