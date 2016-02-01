/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: 
 *  stream from vsb source
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#include "b_psip_lib.h"                                                                                                  
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
#ifndef DMS_CROSS_PLATFORMS
#include "ip_psi.h"
#endif

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)  
#endif

#ifdef NEXUS_HAS_FRONTEND
/* build a list of all available qam sources */
/* Note: for VMS configuration, local tuners (aka sources) == # of decoders are left for local decoding app */
int
initNexusVsbSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i, j;
    int vsbTunerCount = 0;
    QamSrc *vsbSrc;
    NEXUS_FrontendHandle frontendHandle=NULL;
    NEXUS_PlatformConfiguration platformConfig;

    /* global reset */
    memset(ipStreamerGlobalCtx->vsbSrcList, 0, sizeof(ipStreamerGlobalCtx->vsbSrcList));

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->vsbSrcMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i = 0, j = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        NEXUS_FrontendCapabilities capabilities;
        frontendHandle = platformConfig.frontend[i];
        if (!frontendHandle) {
            BDBG_MSG(("%s: NULL Frontend (# %d)" ,__FUNCTION__, i));
            continue;
        }
        NEXUS_Frontend_GetCapabilities(frontendHandle, &capabilities);
        if (capabilities.vsb) {
            vsbTunerCount++;
            /* Found a vsb source */
            vsbSrc = &ipStreamerGlobalCtx->vsbSrcList[j];
            memset(vsbSrc, 0, sizeof(QamSrc));
            vsbSrc->frontendHandle = frontendHandle;
            if (BKNI_CreateEvent(&vsbSrc->psiAcquiredEvent)) {
                BDBG_ERR(("Failed to create psiAcquired event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateEvent(&vsbSrc->signalLockedEvent)) {
                BDBG_ERR(("Failed to create signalLocked event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateMutex(&vsbSrc->lock) != 0) {
                BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
                return -1;
            }
            /* successfully setup a vsb frontend src */
            j++;
        }
        else {
            /* skip other tuner types */
            continue;
        }
    }

    if (!j) {
        BDBG_MSG(("NO vsb Front End founds, check Nexus platform configuration"));
        return 0;
    }
    BDBG_MSG(("Initialized %d VSB Frontend Sources", j));
    return 0;
}

void
unInitNexusVsbSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    QamSrc *vsbSrc;
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ipStreamerGlobalCtx->vsbSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        vsbSrc = &ipStreamerGlobalCtx->vsbSrcList[i];
        if (vsbSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(vsbSrc->psiAcquiredEvent);
        if (vsbSrc->signalLockedEvent)
            BKNI_DestroyEvent(vsbSrc->signalLockedEvent);
        if (vsbSrc->lock)
            BKNI_DestroyMutex(vsbSrc->lock);
    }
    /* global reset */
    memset(ipStreamerGlobalCtx->vsbSrcList, 0, sizeof(ipStreamerGlobalCtx->vsbSrcList));

    if (ipStreamerGlobalCtx->vsbSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->vsbSrcMutex);
    BDBG_MSG(("VSB Frontend Sources are uninitialized"));
}

static void
vsbLockCallback(void *context, int param)
{
    QamSrc *vsbSrc = (QamSrc *)context;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_FrontendVsbStatus vsbStatus;

    BSTD_UNUSED(param);
    BDBG_ASSERT(vsbSrc);
    frontendHandle = vsbSrc->frontendHandle;

    BKNI_Memset(&vsbStatus, 0, sizeof(vsbStatus));
    if (NEXUS_Frontend_GetVsbStatus(frontendHandle, &vsbStatus) != NEXUS_SUCCESS) 
        return;
    BDBG_MSG(("VSB Lock callback, frontend 0x%08x - receiverLock=%d, fecLock=%d, opllLock=%d",
                vsbStatus.receiverLock, vsbStatus.fecLock, vsbStatus.opllLock));

}

/* select a qam src based on the requested QAM frequency */
int
openNexusVsbSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    bool matchFound = false;
    QamSrc *vsbSrc = NULL;
    QamSrc *vsbSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;

    vsbSrcList = &ipStreamerCtx->globalCtx->vsbSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->vsbSrcMutex);
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!vsbSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        if (vsbSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            vsbSrc = &vsbSrcList[i];
            /* set a flag to indicate skipping PSI acquisition as it would have already been acquired during first tuning to this frequency */
            if (vsbSrc->numProgramsFound)
                ipStreamerCtx->skipPsiAcquisition = true;
            matchFound = true;
            break;
        }
        if (!vsbSrcList[i].refCount && !vsbSrc)
            /* save a pointer if we haven't yet found a free unused tuner */
            vsbSrc = &vsbSrcList[i];
    }
    if (vsbSrc) {
        BKNI_AcquireMutex(vsbSrc->lock);
        vsbSrc->refCount++;
        vsbSrc->frequency = ipStreamerCfg->frequency;
        if (!matchFound) {
            /* force PSI acquisition as this src is being used to receive a new IP channel */
            ipStreamerCtx->skipPsiAcquisition = false;
            vsbSrc->numProgramsFound = 0;
        }
        BKNI_ReleaseMutex(vsbSrc->lock);
        BDBG_MSG(("Found a free VSB Frontend %p, use it for %dMhz frequency, refCount %d", vsbSrc, ipStreamerCfg->frequency, vsbSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->vsbSrcMutex);
    if (!vsbSrc) {
        BDBG_MSG(("No Free tuner available for this %d frequency all turners (# %d) busy", ipStreamerCfg->frequency, i));
        return -1;
    }
    ipStreamerCtx->vsbSrc = vsbSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultVsbSettings(&ipStreamerCtx->vsbSettings);
    ipStreamerCtx->vsbSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->vsbSettings.mode = ipStreamerCfg->vsbMode;
    ipStreamerCtx->vsbSettings.lockCallback.callback = vsbLockCallback;
    ipStreamerCtx->vsbSettings.lockCallback.context = vsbSrc;
    ipStreamerCtx->vsbSettings.lockCallback.param = ipStreamerCfg->srcType;

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    for (i = 0; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
        /* pick an unused parser band */
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
        if (ipStreamerCtx->globalCtx->parserBandList[i].frequency == ipStreamerCfg->frequency &&
            ipStreamerCtx->globalCtx->parserBandList[i].subChannel == ipStreamerCfg->subChannel) {
            /* this parser band is being used for same frequency and sub-channel (pids), so reuse it */
            ipStreamerCtx->parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
            break;
        }
        if (!ipStreamerCtx->parserBandPtr && ipStreamerCtx->globalCtx->parserBandList[i].refCount == 0) {
            /* found 1st unused entry */
            ipStreamerCtx->parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
        }
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (ipStreamerCtx->parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(vsbSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(ipStreamerCtx->parserBandPtr->lock);
    if (ipStreamerCtx->parserBandPtr->refCount == 0) {
        ipStreamerCtx->parserBandPtr->frequency = ipStreamerCfg->frequency;
        ipStreamerCtx->parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(ipStreamerCtx->parserBandPtr->parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(vsbSrc->frontendHandle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        rc = NEXUS_ParserBand_SetSettings(ipStreamerCtx->parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", ipStreamerCtx->parserBandPtr->parserBand));
            BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
            ipStreamerCtx->parserBandPtr = NULL;
            goto error;
        }
    }
    ipStreamerCtx->parserBandPtr->refCount++;
    BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
    BDBG_MSG(("CTX %p: VSB Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                ipStreamerCtx, vsbSrc, vsbSrc->refCount, userParams.param1, ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
    return 0;

error:
    BKNI_AcquireMutex(vsbSrc->lock);
    vsbSrc->refCount--;
    BKNI_ReleaseMutex(vsbSrc->lock);
    return -1;
}

void
closeNexusVsbSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    QamSrc *vsbSrc = ipStreamerCtx->vsbSrc;

    /* free up the parser band */
    if (ipStreamerCtx->parserBandPtr) {
        BKNI_AcquireMutex(ipStreamerCtx->parserBandPtr->lock);
        ipStreamerCtx->parserBandPtr->refCount--;
        BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
        BDBG_MSG(("CTX %p: Closed a parser band src %p, refCount %d", ipStreamerCtx, ipStreamerCtx->parserBandPtr, ipStreamerCtx->parserBandPtr->refCount));
    }

    if (!vsbSrc || !vsbSrc->refCount)
        return;

    ipStreamerCtx->vsbSrc = NULL;
    BKNI_AcquireMutex(vsbSrc->lock);
    vsbSrc->refCount--;
    BKNI_ReleaseMutex(vsbSrc->lock);
    BDBG_MSG(("CTX %p: Closed a VSB src %p, refCount %d", ipStreamerCtx, vsbSrc, vsbSrc->refCount));
}

int
setupAndAcquirePsiInfoVsbSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    QamSrc *vsbSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    collectionData.parserBand = ipStreamerCtx->parserBandPtr->parserBand;
    collectionData.vsbSettings = &ipStreamerCtx->vsbSettings;
    collectionData.frontend = ipStreamerCtx->vsbSrc->frontendHandle;
    collectionData.signalLockedEvent = ipStreamerCtx->vsbSrc->signalLockedEvent;
    vsbSrc = ipStreamerCtx->vsbSrc;

    if (vsbSrc->numProgramsFound == 0) {
        /* PSI hasn't yet been acquired, check if any other thread is not in the process of acquiring it */
        BKNI_AcquireMutex(vsbSrc->lock);
        if (!vsbSrc->psiAcquiring) {
            /* no other thread is acquiring PSI */
            BDBG_MSG(("CTX %p: Acquire Psi Info...", ipStreamerCtx));
            /* set flag to indicate this thread is getting PSI */
            vsbSrc->psiAcquiring = true;
            BKNI_ReleaseMutex(vsbSrc->lock);

            /* now get the PSI, this can take several seconds ... */
            acquirePsiInfo(&collectionData, &vsbSrc->psi[0], &vsbSrc->numProgramsFound);

            /* tell any other waiting thread that we are done acquiring PSI */
            BKNI_AcquireMutex(vsbSrc->lock);
            vsbSrc->psiAcquiring = false;
            BKNI_ReleaseMutex(vsbSrc->lock);
            BKNI_SetEvent(vsbSrc->psiAcquiredEvent);
        }
        else {
            /* other thread is acquiring PSI, so wait for its completion */
            BKNI_ReleaseMutex(vsbSrc->lock);
            BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", ipStreamerCtx));
            if (BKNI_WaitForEvent(vsbSrc->psiAcquiredEvent, 60000)) {
                BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", __FUNCTION__));
                return -1;
            }
        }
    }

    if (vsbSrc->numProgramsFound == 0) {
        BDBG_MSG(("Unable to parse PSI!!"));
        return -1;
    }

    if (ipStreamerCfg->subChannel > vsbSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, vsbSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", ipStreamerCtx, ipStreamerCfg->subChannel, vsbSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &vsbSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    BKNI_ResetEvent(vsbSrc->signalLockedEvent);
    return 0;
}

int
startNexusVsbSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    QamSrc *vsbSrc = ipStreamerCtx->vsbSrc;

    BKNI_AcquireMutex(vsbSrc->lock);
    if (vsbSrc->started) {
        BKNI_ReleaseMutex(vsbSrc->lock);
        BDBG_MSG(("CTX %p: VSB Src %p is already started, refCount %d", ipStreamerCtx, vsbSrc, vsbSrc->refCount));
        return 0;
    }

    /* start playpump */
    BKNI_ResetEvent(vsbSrc->signalLockedEvent);
    rc = NEXUS_Frontend_TuneVsb(vsbSrc->frontendHandle, &ipStreamerCtx->vsbSettings);
    if (rc) {
        BKNI_ReleaseMutex(vsbSrc->lock);
        BDBG_ERR(("Failed to Tune to vsb Frontend"));
        return -1;
    }

    if (BKNI_WaitForEvent(vsbSrc->signalLockedEvent, 5000)) {
        BKNI_ReleaseMutex(vsbSrc->lock);
        BDBG_MSG(("CTX %p: VSB Src %p failed to lock the signal ...", ipStreamerCtx, vsbSrc));
        return -1;
    }

    vsbSrc->started = true;
    BKNI_ReleaseMutex(vsbSrc->lock);
    BDBG_MSG(("CTX %p: VSB Src %p is started ...", ipStreamerCtx, vsbSrc));
    return 0;
}

void
stopNexusVsbSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    QamSrc *vsbSrc = ipStreamerCtx->vsbSrc;
    int refCount;

    BKNI_AcquireMutex(vsbSrc->lock);
    refCount = vsbSrc->refCount;
    BKNI_ReleaseMutex(vsbSrc->lock);

    if (refCount == 1) {
        NEXUS_Frontend_Untune(vsbSrc->frontendHandle);
        vsbSrc->started = false;
        BDBG_MSG(("CTX %p: vsb Frontend Src is stopped ...", ipStreamerCtx));
    }
    else {
        BDBG_MSG(("CTX %p: vsb Frontend Src %p is not stopped, ref count %d ...", ipStreamerCtx, vsbSrc, refCount));
    }
}
#else /* !NEXUS_HAS_FRONTEND */
/* stub functions */
int initNexusVsbSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    /* note: we let the init call succeed, but we will fail the open call */
    /* this way app wont fail during startup, but will fail a client request to open/start a recording */
    return 0; 
}
void unInitNexusVsbSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int openNexusVsbSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1; 
}
void closeNexusVsbSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int setupAndAcquirePsiInfoVsbSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut)
{
    BSTD_UNUSED(psiOut);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1; 
}
int startNexusVsbSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
int stopNexusVsbSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from VSB Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
#endif /* NEXUS_HAS_FRONTEND */
