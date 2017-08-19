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
 *  main test app for ip_streamer
 *
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#ifndef DMS_CROSS_PLATFORMS                                                                                              
#include "nexus_platform.h"                                                                                              
#if NEXUS_HAS_FRONTEND                                                                                                   
#include "nexus_frontend.h"                                                                                              
#endif                                                                                                                   
#include "nexus_parser_band.h"                                                                                           
#include "nexus_pid_channel.h"                                                                                           
#include "nexus_playpump.h"                                                                                              
#include "nexus_message.h"                                                                                               
#include "nexus_timebase.h"                                                                                              
#include "nexus_recpump.h"                                                                                               
#ifdef NEXUS_HAS_RECORD                                                                                                  
#include "nexus_record.h"                                                                                                
#endif                                                                                                                   
#include "nexus_file_fifo.h"                                                                                             
#include "nexus_core_utils.h"                                                                                            
#endif /* DMS_CROSS_PLATFORMS */                                                                                         
#include "b_playback_ip_lib.h"                                                                                           

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"
#endif

#include "nexus_core_utils.h"
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_http.h"
#include "b_os_lib.h"

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)  
#endif

#define QAM_STATUS_RUNNING 1
#define SPECA_RUNNING 2
int _diagModeRunning = 0; /* flag to indicate if specA or const/status running (only 1 can run at a time) */
#define QAM_STATUS_RUNNING 1
#define QAM_STATUS_RUNNING 1
#define SPECA_RUNNING 2

#ifdef NEXUS_HAS_FRONTEND
int B_IpStreamer_GetFrontendStatus(void * dlnaGlobalCtx, int frontendNo, IpStreamerSrc *src, IpStreamerStatus *status)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)dlnaGlobalCtx;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_FrontendQamStatus cable_status;
    NEXUS_FrontendSatelliteStatus sat_status;
    /*NEXUS_FrontendTerrestrialStatus terr_status; - not ready */
    NEXUS_Error rc;
    QamSrc *qamSrc;
    SatSrc *satSrc;
    unsigned int i;

    BDBG_MSG_FLOW(("%s: frontendNo=%d, NEXUS_MAX_FRONTENDS=%d\n", BSTD_FUNCTION, frontendNo, NEXUS_MAX_FRONTENDS));

    if (ipStreamerGlobalCtx == NULL)
        return 1;   /* not successful */

    if (frontendNo < 0)
        return 1;   /* invalid frontend */

    if (frontendNo > NEXUS_MAX_FRONTENDS-1)
        return 1;   /* exceeded max frontends */

    i = frontendNo;

    if (ipStreamerGlobalCtx->qamSrcList[i].frontendHandle != NULL)
    {
        BKNI_AcquireMutex(ipStreamerGlobalCtx->qamSrcMutex);
        qamSrc = &ipStreamerGlobalCtx->qamSrcList[i];
        BDBG_MSG_FLOW(("%s: frontendHandle[%d]=%d, refCount=%d, started %d", BSTD_FUNCTION, i, ipStreamerGlobalCtx->qamSrcList[i].frontendHandle, qamSrc->refCount, qamSrc->started));
        if (qamSrc->refCount != 0 && qamSrc->started)
        {
            frontendHandle = qamSrc->frontendHandle;
            *src = IpStreamerSrc_eQam;
            if(_diagModeRunning == 0 || _diagModeRunning == QAM_STATUS_RUNNING)
            {
                _diagModeRunning = QAM_STATUS_RUNNING;
                BDBG_MSG_FLOW(("%s: Remote Diag (%d) is started", BSTD_FUNCTION, _diagModeRunning));
            }
            else
            {
                BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
                BDBG_MSG_FLOW(("%s: Remote Diags (%d) are already running", BSTD_FUNCTION, _diagModeRunning));
                return 1;
            }
        }
        BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
    }
    else if (ipStreamerGlobalCtx->satSrcList[i].frontendHandle != NULL)
    {
        if (ipStreamerGlobalCtx->satSrcList[i].refCount != 0)
        {
            satSrc = &ipStreamerGlobalCtx->satSrcList[i];
            frontendHandle = satSrc->frontendHandle;
            *src = IpStreamerSrc_eSat;
        }
    }

    if (frontendHandle != NULL)
    {
        if (*src == IpStreamerSrc_eQam)
        {
            BDBG_MSG_FLOW(("%s: Requesting Qam Stats for frontendHandle[%d]=%d, refCount=%d\n", BSTD_FUNCTION, i, frontendHandle, qamSrc->refCount));
            rc = NEXUS_Frontend_GetQamStatus(frontendHandle, &cable_status);
            if (rc != NEXUS_SUCCESS) {BDBG_ERR(("%s: NEXUS_Frontend_GetQamStatus Failed!!!!!!!!!!!!!!", BSTD_FUNCTION)); goto out;}
            BDBG_MSG_FLOW(("%s: Got Qam Stats for frontendHandle[%d]=%d, refCount=%d\n", BSTD_FUNCTION, i, frontendHandle, qamSrc->refCount));

            status->receiverLock = cable_status.receiverLock;
            status->fecLock = cable_status.fecLock;
            status->symbolRate = cable_status.symbolRate;
            status->snrEstimate = cable_status.snrEstimate;
            status->reacquireCount = cable_status.reacquireCount;
            status->frequency = cable_status.settings.frequency;
            status->power = cable_status.dsChannelPower;

            switch (cable_status.settings.mode)
            {
                case NEXUS_FrontendQamMode_e16:
                    status->mode = IpStreamer_FrontendMode_e16;
                    break;
                case NEXUS_FrontendQamMode_e32:
                    status->mode = IpStreamer_FrontendMode_e32;
                    break;
                case NEXUS_FrontendQamMode_e64:
                    status->mode = IpStreamer_FrontendMode_e64;
                    break;
                case NEXUS_FrontendQamMode_e128:
                    status->mode = IpStreamer_FrontendMode_e128;
                    break;
                case NEXUS_FrontendQamMode_e256:
                    status->mode = IpStreamer_FrontendMode_e256;
                    break;
                case NEXUS_FrontendQamMode_e512:
                    status->mode = IpStreamer_FrontendMode_e512;
                    break;
                case NEXUS_FrontendQamMode_e1024:
                    status->mode = IpStreamer_FrontendMode_e1024;
                    break;
                case NEXUS_FrontendQamMode_e2048:
                    status->mode = IpStreamer_FrontendMode_e2048;
                    break;
                case NEXUS_FrontendQamMode_e4096:
                    status->mode = IpStreamer_FrontendMode_e4096;
                    break;
                case NEXUS_FrontendQamMode_eAuto_64_256:     /* Automatically scan both QAM-64 and QAM-256.
                                                                Not available on all chipsets. */
                    status->mode = IpStreamer_FrontendMode_eAuto_64_256;
                    break;
                default:
                    status->mode = IpStreamer_FrontendMode_eAuto_64_256;
                    break;
            }

            switch (cable_status.settings.annex)
            {
                case NEXUS_FrontendQamAnnex_eA:
                    status->annex = IpStreamer_FrontendQamAnnex_eA;
                    break;
                case NEXUS_FrontendQamAnnex_eB:
                    status->annex = IpStreamer_FrontendQamAnnex_eB;
                    break;
                default:
                    status->annex = IpStreamer_FrontendQamAnnex_eB;
                    break;
            }
out:
            if (_diagModeRunning == QAM_STATUS_RUNNING)
            {
                _diagModeRunning = 0;
                BDBG_MSG_FLOW(("%s: Remote Diag (%d) is reset", BSTD_FUNCTION, _diagModeRunning));
            }
            return 0;   /* success */
        }
        else if (*src == IpStreamerSrc_eSat)
        {
            rc = NEXUS_Frontend_GetSatelliteStatus(frontendHandle, &sat_status);
            
            status->tunerLocked = sat_status.tunerLocked;
            status->demodLocked = sat_status.demodLocked;
            status->frequency = sat_status.frequency;
            status->symbolRate = sat_status.symbolRate;
            status->snrEstimate = sat_status.snrEstimate;
            status->carrierOffset = sat_status.carrierOffset;
            status->carrierError = sat_status.carrierError;
            status->sampleClock = sat_status.sampleClock;
            status->timeElapsed = sat_status.timeElapsed;
            status->outputBitRate = sat_status.outputBitRate;
            status->bertLocked = sat_status.bertLocked;
            status->mpegCount  = sat_status.mpegCount;
            status->mpegErrors = sat_status.mpegErrors;
            status->berEstimate = sat_status.berEstimate;
            status->channel = sat_status.channel;
            status->symbolRate = sat_status.symbolRate;
            status->symbolRateError = sat_status.symbolRateError;
            status->reacquireCount = sat_status.reacquireCount;
            status->ifAgc = sat_status.ifAgc;
            status->rfAgc = sat_status.rfAgc;
            status->agf = sat_status.agf;
            status->fecCorrected = sat_status.fecCorrected;
            status->fecUncorrected = sat_status.fecUncorrected;
            status->fecClean = sat_status.fecClean;
            status->fecPhase = sat_status.fecPhase;
            status->codeRateNumerator = sat_status.codeRate.numerator;
            status->codeRateDenominator = sat_status.codeRate.denominator;

            status->chipId = sat_status.version.chipId;
            status->chipVersion = sat_status.version.chipVersion;
            status->bondingOption = sat_status.version.bondingOption;
            status->apMicrocodeVersion = sat_status.version.apMicrocodeVersion;
            status->hostConfigurationVersion = sat_status.version.hostConfigurationVersion;

            switch (sat_status.mode)
            {
                case NEXUS_FrontendSatelliteMode_eDvb:
                    status->mode = IpStreamer_FrontendMode_eDvb;
                    break;
                    
                case NEXUS_FrontendSatelliteMode_eDss:
                    status->mode = IpStreamer_FrontendMode_eDss;
                    break;

                case NEXUS_FrontendSatelliteMode_eDcii:
                    status->mode = IpStreamer_FrontendMode_eDcii;
                    break;

                case NEXUS_FrontendSatelliteMode_eTurboQpsk:
                    status->mode = IpStreamer_FrontendMode_eTurboQpsk;
                    break;

                case NEXUS_FrontendSatelliteMode_eTurbo8psk:
                    status->mode = IpStreamer_FrontendMode_eTurbo8psk;
                    break;
    
                case NEXUS_FrontendSatelliteMode_eTurbo:
                    status->mode = IpStreamer_FrontendMode_eTurbo;
                    break;

                case NEXUS_FrontendSatelliteMode_eQpskLdpc:
                    status->mode = IpStreamer_FrontendMode_eQpskLdpc;
                    break;

                case NEXUS_FrontendSatelliteMode_e8pskLdpc:
                    status->mode = IpStreamer_FrontendMode_e8pskLdpc;
                    break;
    
                case NEXUS_FrontendSatelliteMode_eLdpc:
                    status->mode = IpStreamer_FrontendMode_eLdpc;
                    break;
    
                case NEXUS_FrontendSatelliteMode_eBlindAcquisition:
                    status->mode = IpStreamer_FrontendMode_eBlindAcquisition;
                    break;
                    
                default:
                    status->mode = IpStreamer_FrontendMode_eDvb;
                    break;
            }
            return 0;   /* success */
        }
        else
        {
            /* terrestrial modes - not ready
            switch (terr_status.mode)
            {
                case NEXUS_FrontendSatelliteMode_eDvbt:
                    status->mode = IpStreamer_FrontendMode_eDvbt;
                    break;

                case NEXUS_FrontendSatelliteMode_eDvbt2:
                    status->mode = IpStreamer_FrontendMode_eDvbt2;
                    break;

                case IpStreamer_FrontendMode_eIsdbt:
                    status->mode = IpStreamer_FrontendMode_eIsdbt;
                    break;
                default:
                    status->mode = IpStreamer_FrontendMode_eDvbt;
                    break;
            }
            */
            status->mode = IpStreamer_FrontendMode_eDvbt;
            return 0;   /* success */
        }
    }

    return 1;   /* not successful */
}
#else
int B_IpStreamer_GetFrontendStatus(void * dlnaGlobalCtx, int frontendNo, IpStreamerSrc *src, IpStreamerStatus *status)
{
    BSTD_UNUSED(dlnaGlobalCtx);
    BSTD_UNUSED(frontendNo);
    BSTD_UNUSED(src);
    BSTD_UNUSED(status);
    BDBG_ERR(("%s: Frontend status not supported on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    return 1;
}
#endif

#ifdef REMOTE_DIAG_SUPPORT
int B_IpStreamer_GetFrontendSoftDecision(void * dlnaGlobalCtx, int frontendNo, IpStreamerSoftDecision *softDecision, int length)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)dlnaGlobalCtx;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_FrontendSoftDecision decisions[100];
    NEXUS_FrontendSoftDecision *pDecisions = &decisions[0];
    NEXUS_Error rc;
    QamSrc *qamSrc;
    SatSrc *satSrc;
    int i;

    BDBG_MSG_FLOW(("B_IpStreamer_GetFrontendSoftDecision, frontendNo=%d, NEXUS_MAX_FRONTENDS=%d\n", frontendNo, NEXUS_MAX_FRONTENDS));

    if (ipStreamerGlobalCtx == NULL)
        return 1;   /* not successful */

    if (length > 64)
        return 1;   /* exceeded max length */

    if (frontendNo > NEXUS_MAX_FRONTENDS)
        return 1;   /* exceeded max frontends */

    i = frontendNo;

    if (ipStreamerGlobalCtx->qamSrcList[i].frontendHandle != NULL)
    {
        BDBG_MSG_FLOW(("frontendHandle[%d]=%d, refCount=%d\n", i, ipStreamerGlobalCtx->qamSrcList[i].frontendHandle, ipStreamerGlobalCtx->qamSrcList[i].refCount));
        if (ipStreamerGlobalCtx->qamSrcList[i].refCount != 0)
        {
            qamSrc = &ipStreamerGlobalCtx->qamSrcList[i];
            frontendHandle = qamSrc->frontendHandle;
            BKNI_AcquireMutex(ipStreamerGlobalCtx->qamSrcMutex);
            if(_diagModeRunning == 0 || _diagModeRunning == QAM_STATUS_RUNNING)
            {
                _diagModeRunning = QAM_STATUS_RUNNING;
                BDBG_MSG_FLOW(("%s: Remote Diag (%d) is started", BSTD_FUNCTION, _diagModeRunning));
            }
            else
            {
                BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
                BDBG_MSG_FLOW(("%s: Remote Diags (%d) are already running", BSTD_FUNCTION, _diagModeRunning));
                return 1;
            }
            BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
        }
    }
    else if (ipStreamerGlobalCtx->satSrcList[i].frontendHandle != NULL)
    {
        if (ipStreamerGlobalCtx->satSrcList[i].refCount != 0)
        {
            satSrc = &ipStreamerGlobalCtx->satSrcList[i];
            frontendHandle = satSrc->frontendHandle;
        }
    }

    if (frontendHandle != NULL)
    {
        rc = NEXUS_Frontend_GetSoftDecisions(frontendHandle, pDecisions, length);
        for (i=0; i<length; i++)
        {
            softDecision[i].i = pDecisions[i].i;
            softDecision[i].q = pDecisions[i].q;
        }
        if(_diagModeRunning == QAM_STATUS_RUNNING)
        {
            _diagModeRunning = 0;
            BDBG_MSG_FLOW(("%s: Remote Diag (%d) is reset", BSTD_FUNCTION, _diagModeRunning));
        }
        return 0;   /* success */
    }

    return 1;   /* not successful */
}

static void spectrum_data_ready_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend;
    QamSrc *qamSrc = (QamSrc *)context;
    
    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != context);
    frontend = qamSrc->frontendHandle;
    
    if(frontend != NULL)
    {
        BKNI_SetEvent(qamSrc->signalSpecADataRdy);
    }
    else
    {
        BDBG_ERR(("%s: frontend handle is NULL!!", BSTD_FUNCTION));
    }
}

int B_IpStreamer_SpecAOpen(void * dlnaGlobalCtx, int frontendNo, int* selectedFrontendNo)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)dlnaGlobalCtx;
    QamSrc *qamSrc = NULL;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_FrontendQamMode qamMode = NEXUS_FrontendQamMode_e256;
    int rc = 0;
    int i;

    if (ipStreamerGlobalCtx == NULL || frontendNo > NEXUS_MAX_FRONTENDS)
    {
        return 1;   /* not successful */
    }

    BKNI_AcquireMutex(ipStreamerGlobalCtx->qamSrcMutex);
    if(_diagModeRunning == 0)
    {
        _diagModeRunning = SPECA_RUNNING;
        BDBG_MSG_FLOW(("%s: Remote Diag (%d) is started", BSTD_FUNCTION, _diagModeRunning));
        if (ipStreamerGlobalCtx->qamSrcList[frontendNo].refCount == 0)
        {
            i = frontendNo;
        }
        else
        {
            /* if the desired core (frontendNo) is unavailable, let the server
               find and select one that is available */
            for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) 
            {
                if (ipStreamerGlobalCtx->qamSrcList[i].frontendHandle &&                                                     
                        ipStreamerGlobalCtx->qamSrcList[i].refCount == 0)
                {
                    break;
                }   
            }  
        }

        if (i >= NEXUS_MAX_FRONTENDS) 
        {     
            /* no available frontend cores */
            rc = 1; 
        }  
        else
        {
            qamSrc = &ipStreamerGlobalCtx->qamSrcList[i];
            frontendHandle = qamSrc->frontendHandle;
            ipStreamerGlobalCtx->qamSrcList[i].refCount = 1;
            *selectedFrontendNo = i;
            rc = 0;
        }
        BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
    
        if(frontendHandle != NULL)
        {
            if (BKNI_CreateEvent(&qamSrc->signalSpecADataRdy)) {
                BDBG_ERR(("Failed to signalSpecADataRdy event at %d", __LINE__));
                return -1;
            }
        }
    }
    else
    {
        BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
        BDBG_MSG_FLOW(("%s: Remote Diags (%d) are already running", BSTD_FUNCTION, _diagModeRunning));
        rc = 1;
    }
    return rc;
}

int B_IpStreamer_SpecAClose(void * dlnaGlobalCtx, int frontendNo)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)dlnaGlobalCtx;
    QamSrc *qamSrc = NULL;
    NEXUS_FrontendHandle frontendHandle = NULL;
    int rc = 0;
    
    if (ipStreamerGlobalCtx == NULL || frontendNo > NEXUS_MAX_FRONTENDS)
    {
        return 1;   /* not successful */
    }

    BKNI_AcquireMutex(ipStreamerGlobalCtx->qamSrcMutex);
    if (ipStreamerGlobalCtx->qamSrcList[frontendNo].refCount >= 0)
    {
        qamSrc = &ipStreamerGlobalCtx->qamSrcList[frontendNo];
        frontendHandle = qamSrc->frontendHandle;
        ipStreamerGlobalCtx->qamSrcList[frontendNo].refCount = 0;
        rc = 0;
    }
    else
    {
        rc = 1;
    }
    if(_diagModeRunning == SPECA_RUNNING)
    {
        _diagModeRunning = 0;
        BDBG_MSG_FLOW(("%s: Remote Diag (%d) is reset", BSTD_FUNCTION, _diagModeRunning));
    }
    BKNI_ReleaseMutex(ipStreamerGlobalCtx->qamSrcMutex);
    
    if(frontendHandle != NULL)
    {
        if (qamSrc->signalSpecADataRdy)
        {
            BKNI_DestroyEvent(qamSrc->signalSpecADataRdy);
        }
#if 0
        /* uncommenting untune here for now to debug the issue where specA untune causes further tuning to fail */
        NEXUS_Frontend_Untune(frontendHandle);
#endif
    }
    else
    {
        rc = 1;
    }
    return rc;
}

int B_IpStreamer_GetFrontendSpecAData(void * dlnaGlobalCtx, int frontendNo, IpStreamerSpecAParams *specAParams)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)dlnaGlobalCtx;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_FrontendSpectrumSettings spectrumSettings;
    NEXUS_Error rc;
    QamSrc *qamSrc;
    int channel;
    
    BDBG_MSG_FLOW(("B_IpStreamer_GetFrontendSpecAData, frontendNo=%d, NEXUS_MAX_FRONTENDS=%d\n", frontendNo, NEXUS_MAX_FRONTENDS));
    
    if (ipStreamerGlobalCtx == NULL)
    {
        return 1;   /* not successful */
    }
    if (frontendNo > NEXUS_MAX_FRONTENDS)
    {
        return 1;   /* exceeded max frontends */
    }
    channel = frontendNo;
    if (ipStreamerGlobalCtx->qamSrcList[channel].frontendHandle != NULL)
    {
        BDBG_MSG_FLOW(("frontendHandle[%d]=%d, refCount=%d\n", channel, ipStreamerGlobalCtx->qamSrcList[channel].frontendHandle, ipStreamerGlobalCtx->qamSrcList[channel].refCount));
        if (ipStreamerGlobalCtx->qamSrcList[channel].refCount != 0)
        {
            qamSrc = &ipStreamerGlobalCtx->qamSrcList[channel];
            frontendHandle = qamSrc->frontendHandle;
        }
    }
    
    if (frontendHandle != NULL)
    {
        BKNI_ResetEvent(qamSrc->signalSpecADataRdy);
        
        qamSrc->totalFftSamples = specAParams->numOfSamples;
        qamSrc->fftData = specAParams->data;
        
        spectrumSettings.dataReadyCallback.callback = spectrum_data_ready_callback;
        spectrumSettings.dataReadyCallback.context = qamSrc;
        spectrumSettings.data = specAParams->data;
        spectrumSettings.binAverage = specAParams->numOfAvgBins;
        spectrumSettings.numSamples = specAParams->numOfSamples;
        spectrumSettings.startFrequency = specAParams->fStart;
        spectrumSettings.stopFrequency = specAParams->fStop;
        spectrumSettings.fftSize = specAParams->fftSize;
    
        rc = NEXUS_Frontend_RequestSpectrumData(frontendHandle, &spectrumSettings);
        rc = BKNI_WaitForEvent(qamSrc->signalSpecADataRdy, 5000);
        if(rc == NEXUS_TIMEOUT) {
            BDBG_ERR(("%s ERROR: Spectrum data retreival timed out.", BSTD_FUNCTION));
        }
        return 0;   /* success */
    }
    BDBG_ERR(("%s: Error, frontendHandle is NULL!", BSTD_FUNCTION));
    return 1;   /* not successful */
}
#endif /* REMOTE_DIAG_SUPPORT */
