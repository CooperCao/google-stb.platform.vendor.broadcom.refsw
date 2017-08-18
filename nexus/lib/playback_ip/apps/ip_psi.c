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

#include "ts_psi.h"
#ifdef STREAMER_CABLECARD_SUPPORT
#include "ip_strm_cablecard.h"
#else
#include "tspsimgr2.h"
#endif

#include "nexus_core_utils.h"

BDBG_MODULE(ip_psi);

#undef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))

typedef enum IP_PSI_TrackType
{
   IP_PSI_TrackType_eVideo,
   IP_PSI_TrackType_eAudio,
   IP_PSI_TrackType_eOther,
   IP_PSI_TrackType_eMax,
}IP_PSI_TrackType;

/* Fills in B_PlaybackIpPsiInfo based on PMT info */
static void convertEPIDToPsi( B_PlaybackIpPsiInfo *psi, EPID *pEpid, IP_PSI_TrackType type)
{
   if (type == IP_PSI_TrackType_eVideo) {
      psi->mpegType = NEXUS_TransportType_eTs;
      psi->videoCodec = pEpid->streamType;
      psi->videoPid = pEpid->pid;
      psi->psiValid = true;
   }
   else if (type == IP_PSI_TrackType_eAudio ) {
      psi->mpegType = NEXUS_TransportType_eTs;
      psi->audioCodec = pEpid->streamType;
      psi->audioPid = pEpid->pid;
      psi->psiValid = true;
   }
   else
   {
      BDBG_MSG(("###### TODO: Unknown type #####"));
   }
}

#define IP_PSI_MEDIAINFO_PAT_TIMEOUT 800
#define IP_PSI_MEDIAINFO_PMT_TIMEOUT 800
/*                                                         */
/* This function returns first stream (video and/or audio) */
/* To return multiple streams, remove stream found and     */
/* pass a ptr to array of psi structures.                  */
/*                                                         */
void acquirePsiInfo(pPsiCollectionDataType pCollectionData, B_PlaybackIpPsiInfo *psi, int *numProgramsFound)
{

    B_Error errCode;
    uint8_t i,j;
    NEXUS_Error rc;
    B_PlaybackIpSessionStartSettings ipStartSettings;
    B_PlaybackIpSessionStartStatus ipStartStatus;
    BKNI_EventHandle dataReadyEvent = NULL;
    int patTimeout = IP_PSI_MEDIAINFO_PAT_TIMEOUT;
    int patTimeoutOrig = 0;
    int pmtTimeout = IP_PSI_MEDIAINFO_PMT_TIMEOUT;
    int pmtTimeoutOrig = 0;
    CHANNEL_INFO_T *pChanInfo = NULL;
    bool nitFound = 0;

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
            BDBG_ERR(("%s: QAM Src failed to lock the signal during PSI acquisition...", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: QAM Src (frontend %p) locked the signal during PSI acquisition...", BSTD_FUNCTION, (void *)pCollectionData->frontend));
        /* continue below */
    }
    else if (pCollectionData->srcType == IpStreamerSrc_eSat) {
        BDBG_MSG(("%s: Sat Freq %d, symbolRate %d, mode %d", BSTD_FUNCTION,
                    pCollectionData->satSettings->frequency, pCollectionData->satSettings->symbolRate, pCollectionData->satSettings->mode));
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneSatellite(pCollectionData->frontend, pCollectionData->satSettings);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
        BDBG_MSG(("%s: Sat Src locked the signal during PSI acquisition...", BSTD_FUNCTION));
        if (BKNI_WaitForEvent(pCollectionData->signalLockedEvent, 10000)) {
            BDBG_ERR(("Sat Src failed to lock the signal during PSI acquisition...\n"));
            goto error;
        }
    }
    else if (pCollectionData->srcType == IpStreamerSrc_eOfdm) {
        BDBG_MSG(("%s: Ofdm Freq %d, bandwidth %d, mode %d", BSTD_FUNCTION,
                    pCollectionData->ofdmSettings->frequency, pCollectionData->ofdmSettings->bandwidth, pCollectionData->ofdmSettings->mode));
        NEXUS_StartCallbacks(pCollectionData->frontend);
        rc = NEXUS_Frontend_TuneOfdm(pCollectionData->frontend, pCollectionData->ofdmSettings);
        if (rc) {BDBG_ERR(("PSI - NEXUS Error (%d) at %d\n", rc, __LINE__)); goto error;}
        BDBG_MSG(("%s: Ofdm Src tuned during PSI acquisition...", BSTD_FUNCTION));
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
        BDBG_MSG(("%s: nothing to be setup for this %d src type", BSTD_FUNCTION, pCollectionData->srcType));
        goto out;
    }
    else {
        BDBG_ERR(("%s: doesn't support PSI discovery for this %d src type", BSTD_FUNCTION, pCollectionData->srcType));
        goto error;
    }

out:

    {
        pChanInfo = BKNI_Malloc( sizeof(CHANNEL_INFO_T));
        if (pChanInfo == NULL)
        {
            BDBG_ERR(("B_Os_Malloc() failed"));
            goto error;
        }

        /* adjust pat/pmt timeouts for faster scanning */
        tsPsi_getTimeout(&patTimeoutOrig, &pmtTimeoutOrig);
        tsPsi_setTimeout(patTimeout, pmtTimeout);

        rc = tsPsi_getChannelInfo(pChanInfo, pCollectionData->parserBand);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("tsPsi_getChannelInfo failed\\n"));
            goto error;
        }

        /* restore default pat/pmt timeouts */
        tsPsi_setTimeout(patTimeoutOrig, pmtTimeoutOrig);

        (*numProgramsFound) = pChanInfo->num_programs;
        if ( (*numProgramsFound) > MAX_PROGRAMS_PER_FREQUENCY)
        {
            (*numProgramsFound) = MAX_PROGRAMS_PER_FREQUENCY;
        }

        for (i = 0; i < pChanInfo->num_programs && i < MAX_PROGRAMS_PER_FREQUENCY; i++)
        {
            PROGRAM_INFO_T *pProgramInfo = NULL;

            pProgramInfo = &pChanInfo->program_info[i];

            if (pProgramInfo->program_number == 0)
            {
                nitFound = 1;
                /* Right now we don't use nit information, so continue */
                continue;
            }

            psi->pmtPid = pProgramInfo->map_pid ;/* this is pmt_pid */
            psi->pcrPid = pProgramInfo->pcr_pid ;

            /* Current interface only expose one video and audio track per program, BIP handles it properly.*/
            if(pProgramInfo->num_video_pids >= 1)
            {
                convertEPIDToPsi(psi,&pProgramInfo->video_pids[i], IP_PSI_TrackType_eVideo);
            }

            if(pProgramInfo->num_audio_pids >= 1)
            {
                convertEPIDToPsi(psi,&pProgramInfo->audio_pids[i], IP_PSI_TrackType_eAudio);
            }
            psi += 1;
        }
    }
error:
    if (pChanInfo)
    {
        BKNI_Free(pChanInfo);
    }
}
