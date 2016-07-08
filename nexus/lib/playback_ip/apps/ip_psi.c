/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *  ip psi test file
 *
 ******************************************************************************/
#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"

#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_message.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "ip_psi.h"
#include "b_playback_ip_lib.h"
#include "b_os_lib.h"
#include "b_os_lib.h"

#include "b_psip_lib.h"
#include "nexus_core_utils.h"

#ifdef STREAMER_CABLECARD_SUPPORT
#include "ip_strm_cablecard.h"
#else
#include <tspsimgr.h>
#endif

BDBG_MODULE(ip_psi);

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

/* message "data ready" callback which is called by si when our requested data
   has been processed and is ready for retrieval */
static void SiDataReady(B_Error status, void * context)
{
    /* printf("in APP DataReady callback - psip lib has finished processing data\n");*/
    if (B_ERROR_SUCCESS == status) {
        BKNI_SetEvent((BKNI_EventHandle)context);
    } else {
        BDBG_MSG(("problem receiving data from api call - error code:%d\n", status));
        /* set event so our test app can continue... */
        BKNI_SetEvent((BKNI_EventHandle)context);
    }
}

/* utility function to convert si applib message filter to nexus message filter
   assumes pNexusFilter has already been properly initialized with default values */
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
            if (pCollectionData->srcType == IpStreamerSrc_eIp) {
                pCollectionData->pid[i].channel =
                    NEXUS_Playpump_OpenPidChannel(pCollectionData->playpump, pid, &playpumpPidChannelSettings);
            }
            else {
                pCollectionData->pid[i].channel =
                    NEXUS_PidChannel_Open(pCollectionData->parserBand, pid, &pidChannelSettings);
            }
            return pCollectionData->pid[i].channel;
        }
    }

    if (i == NUM_PID_CHANNELS) {
        BDBG_ERR(("failed to open pid channel:0x%04x - not enough storage space in pCollectionData!\n", pid));
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
            if (pCollectionData->srcType == IpStreamerSrc_eIp)
                NEXUS_Playpump_ClosePidChannel(pCollectionData->playpump, pCollectionData->pid[i].channel);
            else
                NEXUS_PidChannel_Close(pCollectionData->pid[i].channel);
            pCollectionData->pid[i].channel = NULL;
            pCollectionData->pid[i].num = 0;
            found = true;
            break;
        }
    }
    if (!found)
        BDBG_ERR(("failure closing pid channel:0x%04x - not found in list\n", pid));
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
        BDBG_ERR(("invalid filterHandle received from SI applib!\n"));
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
            BDBG_ERR(("NEXUS_Message_GetBuffer() failed\n"));

            return B_ERROR_UNKNOWN;
        }
        break;

    default:
        BDBG_ERR(("-=-=- invalid Command received:%d -=-=-\n", pRequest->cmd));
        return B_ERROR_INVALID_PARAMETER;
        break;
    }

    return B_ERROR_SUCCESS;
}

/* convertStreamToPsi                            */
/* Fills in B_PlaybackIpPsiInfo based on PMT info */
static void convertStreamToPsi( TS_PMT_stream *stream, B_PlaybackIpPsiInfo *psi)
{
    psi->mpegType = NEXUS_TransportType_eTs;
    switch (stream->stream_type) {
        case TS_PSI_ST_13818_2_Video:
        case TS_PSI_ST_ATSC_Video:
            psi->videoCodec = NEXUS_VideoCodec_eMpeg2;
            psi->videoPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_11172_2_Video:
            psi->videoCodec = NEXUS_VideoCodec_eMpeg1;
            psi->videoPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_14496_10_Video:
            psi->videoCodec = NEXUS_VideoCodec_eH264;
            psi->videoPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case 0x27:
        case 0x6:
        case 0x24:
            psi->videoCodec = NEXUS_VideoCodec_eH265;
            psi->videoPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_11172_3_Audio:
        case TS_PSI_ST_13818_3_Audio:
            psi->audioCodec = NEXUS_AudioCodec_eMpeg;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_ATSC_AC3:
            psi->audioCodec = NEXUS_AudioCodec_eAc3;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_ATSC_EAC3:
            psi->audioCodec = NEXUS_AudioCodec_eAc3Plus;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_13818_7_AAC:
            psi->audioCodec = NEXUS_AudioCodec_eAac;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case TS_PSI_ST_14496_3_Audio:
            psi->audioCodec = NEXUS_AudioCodec_eAacPlus;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        case 0x88: /*For DTS-UHD/DTS-HD audio*/
            psi->audioCodec = NEXUS_AudioCodec_eDtsHd;
            psi->audioPid = stream->elementary_PID;
            psi->psiValid = true;
            break;
        default:
            BDBG_MSG(("###### TODO: Unknown stream type: %x #####", stream->stream_type));
    }
}

#ifndef STREAMER_CABLECARD_SUPPORT
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
#endif


/*                                                         */
/* This function returns first stream (video and/or audio) */
/* To return multiple streams, remove stream found and     */
/* pass a ptr to array of psi structures.                  */
/*                                                         */
void acquirePsiInfo(pPsiCollectionDataType pCollectionData, B_PlaybackIpPsiInfo *psi, int *numProgramsFound)
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
#define PMT_BUF_LENGTH 4096
    uint32_t bufPMTLength = PMT_BUF_LENGTH;
    NEXUS_Error rc;
    B_PlaybackIpSessionStartSettings ipStartSettings;
    B_PlaybackIpSessionStartStatus ipStartStatus;
    BKNI_EventHandle dataReadyEvent = NULL;
#ifndef STREAMER_CABLECARD_SUPPORT
    PROGRAM_INFO_T programInfo;
    BKNI_Memset(&programInfo, 0, sizeof(programInfo));
    EPID epid;
#endif

    buf = BKNI_Malloc(bufLength);
    bufPMT = BKNI_Malloc(bufPMTLength);
    if (buf == NULL || bufPMT == NULL) { BDBG_ERR(("BKNI_Malloc Failure at %d", __LINE__)); goto error;}
    /* Start stream  */
    *numProgramsFound = 0;
    if (pCollectionData->srcType == IpStreamerSrc_eIp) {
        /* start playpump */
        rc = NEXUS_Playpump_Start(pCollectionData->playpump);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d \n", rc, __LINE__)); goto error;}
        /* let IP Applib go ... */
        memset(&ipStartSettings, 0, sizeof(ipStartSettings));
        /* Since PAT/PMT based PSI discovery is only done for Live UDP/RTP/RTSP sessions, so we only need to set the playpump handle */
        ipStartSettings.nexusHandles.playpump = pCollectionData->playpump;
        ipStartSettings.nexusHandlesValid = true;
        ipStartSettings.u.rtsp.mediaTransportProtocol = B_PlaybackIpProtocol_eRtp;  /* protocol used to carry actual media */
        rc = B_PlaybackIp_SessionStart(pCollectionData->playbackIp, &ipStartSettings, &ipStartStatus);
        while (rc == B_ERROR_IN_PROGRESS) {
            /* app can do some useful work while SessionSetup is in progress and resume when callback sends the completion event */
            BDBG_MSG(("Session Start call in progress, sleeping for now..."));
            BKNI_Sleep(100);
            rc = B_PlaybackIp_SessionStart(pCollectionData->playbackIp, &ipStartSettings, &ipStartStatus);
        }
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
    }
#if NEXUS_HAS_FRONTEND
    else if (pCollectionData->srcType == IpStreamerSrc_eQam) {
        BKNI_ResetEvent(pCollectionData->signalLockedEvent);
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneQam(pCollectionData->frontend, pCollectionData->qamSettings);
        if (rc) {
            BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__));
            goto error;
        }
        /* now wait for scan to complete */
#define QAM_LOCK_TIMEOUT 5000
        if (BKNI_WaitForEvent(pCollectionData->signalLockedEvent, QAM_LOCK_TIMEOUT)) {
            BDBG_ERR(("%s: QAM Src failed to lock the signal during PSI acquisition...", __FUNCTION__));
            goto error;
        }
        BDBG_MSG(("%s: QAM Src (frontend %p) locked the signal during PSI acquisition...", __FUNCTION__, (void *)pCollectionData->frontend));
        /* continue below */
    }
    else if (pCollectionData->srcType == IpStreamerSrc_eSat) {
        BDBG_MSG(("%s: Sat Freq %d, symbolRate %d, mode %d", __FUNCTION__,
                    pCollectionData->satSettings->frequency, pCollectionData->satSettings->symbolRate, pCollectionData->satSettings->mode));
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneSatellite(pCollectionData->frontend, pCollectionData->satSettings);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
        BDBG_MSG(("%s: Sat Src locked the signal during PSI acquisition...", __FUNCTION__));
        if (BKNI_WaitForEvent(pCollectionData->signalLockedEvent, 10000)) {
            BDBG_ERR(("Sat Src failed to lock the signal during PSI acquisition...\n"));
            goto error;
        }
    }
    else if (pCollectionData->srcType == IpStreamerSrc_eOfdm) {
        BDBG_MSG(("%s: Ofdm Freq %d, bandwidth %d, mode %d", __FUNCTION__,
                    pCollectionData->ofdmSettings->frequency, pCollectionData->ofdmSettings->bandwidth, pCollectionData->ofdmSettings->mode));
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneOfdm(pCollectionData->frontend, pCollectionData->ofdmSettings);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
        BDBG_MSG(("%s: Ofdm Src tuned during PSI acquisition...", __FUNCTION__));
        if (BKNI_WaitForEvent(pCollectionData->signalLockedEvent, 10000)) {
            BDBG_ERR(("Ofdm Src failed to lock the signal during PSI acquisition...\n"));
            goto error;
        }
    }
    else if (pCollectionData->srcType == IpStreamerSrc_eVsb) {
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneVsb(pCollectionData->frontend, pCollectionData->vsbSettings);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
        if (BKNI_WaitForEvent(pCollectionData->signalLockedEvent, 5000)) {
            BDBG_ERR(("failed to lock the signal during PSI acquisition...\n"));
            goto error;
        }
    }
#endif
    else if (pCollectionData->srcType == IpStreamerSrc_eStreamer) {
        BDBG_MSG(("%s: nothing to be setup for this %d src type", __FUNCTION__, pCollectionData->srcType));
        goto out;
    }
    else {
        BDBG_ERR(("%s: doesn't support PSI discovery for this %d src type", __FUNCTION__, pCollectionData->srcType));
        goto error;
    }

out:
    if (BKNI_CreateEvent(&dataReadyEvent) != BERR_SUCCESS) { BDBG_ERR(("Failed to create PSI dataReadyEvent \n")); goto error; }

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = DataReady;
    settings.dataReady.context = NULL;
    msg = NEXUS_Message_Open(&settings);
    if (!msg) { BDBG_ERR(("PSI - NEXUS_Message_Open failed\n")); goto error; }

    B_PSIP_GetDefaultSettings(&si_settings);
    si_settings.PatTimeout = 800;
    si_settings.PmtTimeout = 800;
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
    if (BKNI_WaitForEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam, si_settings.PatTimeout)) {
        BDBG_ERR(("Failed to find PAT table ...\n"));
        goto error;
    }
    BDBG_MSG(("received response from SI, len = %d!\n", bufLength));
#if 0
    printf("\n"); for (i=0;i<bufLength;i++) printf("%02x ",buf[i]); printf("\n");
#endif

    if (0 < bufLength) {
        BDBG_MSG(("PAT Programs found = %d\n", TS_PAT_getNumPrograms(buf)));
        for (i = 0; i<MAX_PROGRAMS_PER_FREQUENCY && (TS_PAT_getProgram(buf, bufLength, i, &program)==BERR_SUCCESS); i++)
        {
            memset(psi, 0, sizeof(*psi));
            BDBG_MSG(("program_number: %d, i %d, PID: 0x%04X, psi %p, sizeof psi %d\n", program.program_number, i, program.PID, (void *)psi, sizeof(*psi)));
            psi->pmtPid = program.PID;

            BKNI_ResetEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam);
            bufPMTLength = PMT_BUF_LENGTH;
            memset(bufPMT, 0, bufPMTLength);
            if (B_ERROR_SUCCESS != B_PSIP_GetPMT(&settingsApi, program.PID, bufPMT, &bufPMTLength)) {
                BDBG_ERR(("B_PSIP_GetPMT() failed\n"));
                continue;
            }
            /* wait for async response from si - wait on dataReadyEvent */
            if (BKNI_WaitForEvent((BKNI_EventHandle)settingsApi.dataReadyCallbackParam, si_settings.PmtTimeout)) {
                BDBG_ERR(("Failed to find PMT table ...\n"));
                goto error;
            }
            BDBG_MSG(("received PMT: size:%d, # of streams in this program %d\n", bufPMTLength, TS_PMT_getNumStreams(bufPMT, bufPMTLength)));
#ifndef STREAMER_CABLECARD_SUPPORT
            /* find and process Program descriptors */
            tsPsi_procProgDescriptors(bufPMT, bufPMTLength, &programInfo);
            if (programInfo.ca_pid) {
                /* we are no longer ignoring the programs w/ ca_pid set as it was causing us to skip over even the clear channels */
                /* looks like this descriptor is set even for clear streams */
                BDBG_MSG(("Even though Program # %d, pid 0x%x seems encrypted, ca_pid 0x%x, we are not skipping it", program.program_number, program.PID, programInfo.ca_pid));
            }
#endif
            if (0 < bufPMTLength) {
                for (j = 0; j < TS_PMT_getNumStreams(bufPMT, bufPMTLength); j++)
                {
                    memset(&stream, 0, sizeof(stream));
                    TS_PMT_getStream(bufPMT, bufPMTLength, j, &stream);
                    BDBG_MSG(("j %d, stream_type:0x%02X, PID:0x%04X\n", j, stream.stream_type, stream.elementary_PID));
                    convertStreamToPsi( &stream, psi);
#ifdef STREAMER_CABLECARD_SUPPORT
                    memcpy(psi->pmtBuffer, bufPMT, bufPMTLength);
                    psi->pmtBufferSize = bufPMTLength;
#else
                    memset(&epid, 0, sizeof(epid));
                    tsPsi_procStreamDescriptors(bufPMT, bufPMTLength, j, &epid);
                    if (epid.ca_pid) {
                        BDBG_MSG(("even program 0x%x has ca pid 0x%x, we are not ignoring it", program.PID, epid.ca_pid));
                        break;
                    }
#endif
                    psi->pcrPid = TS_PMT_getPcrPid(bufPMT, bufPMTLength);
                    if ((psi->videoPid ) && (psi->audioPid )) {
                        (*numProgramsFound)++;
                        BDBG_MSG(("Found %d program, vpid %d, vcodec %d, apid %d, acodec %d", *numProgramsFound, psi->videoPid, psi->videoCodec, psi->audioPid, psi->audioCodec));
                        psi += 1;
                        break;
                    }
                    else if (j == (TS_PMT_getNumStreams(bufPMT, bufPMTLength)-1)) {
                        /* last stream in the program */
                        if ((psi->videoPid ) || (psi->audioPid )) {
                            (*numProgramsFound)++;
                            BDBG_MSG(("Found %d program, vpid %d or apid %d", *numProgramsFound, psi->videoPid, psi->audioPid));
                            psi += 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    BDBG_MSG(("stopping PSI gathering\n"));

    if (pCollectionData->srcType == IpStreamerSrc_eIp) {
        B_PlaybackIp_SessionStop(pCollectionData->playbackIp);
        NEXUS_Playpump_Stop(pCollectionData->playpump);
    }
#ifdef NEXUS_HAS_FRONTEND
    else if (pCollectionData->srcType == IpStreamerSrc_eQam || pCollectionData->srcType == IpStreamerSrc_eVsb || pCollectionData->srcType == IpStreamerSrc_eSat || pCollectionData->srcType == IpStreamerSrc_eOfdm) {
        NEXUS_StopCallbacks(pCollectionData->frontend);
        NEXUS_Frontend_Untune(pCollectionData->frontend);
    }
#endif

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
