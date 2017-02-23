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
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "ip_streamer_transcode.h"
#endif

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)  
#endif

#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
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

int
initNexusHdmiSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    NEXUS_Error rc;
    HdmiSrc *hdmiSrc = NULL;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_HdmiInputSettings hdmiInputSettings;

    /* create the per dst list mutex */
    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->hdmiSrcMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }
    for (i=0; i<NEXUS_NUM_HDMI_INPUTS; i++) {
        memset(&ipStreamerGlobalCtx->hdmiSrcList[i], 0, sizeof(HdmiSrc));
        hdmiSrc = &ipStreamerGlobalCtx->hdmiSrcList[i];
        if ((hdmiSrc->timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == NEXUS_Timebase_eInvalid) {
            BDBG_ERR(("%s: ERROR: NEXUS_Timebase_Open Failed to open a free Timebase", __FUNCTION__));
            return -1;
        }
        hdmiSrc->hdmiInputId = i;
        hdmiSrc->inUse = false;
        hdmiSrc->refCount = 0;

        NEXUS_Timebase_GetSettings(hdmiSrc->timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
        rc = NEXUS_Timebase_SetSettings(hdmiSrc->timebase, &timebaseSettings);
        if (rc) {
            BDBG_ERR(("%s: NEXUS_Timebase_SetSettings() Failed, rc %d", __FUNCTION__, rc));
            goto error;
        }

        NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
        hdmiInputSettings.timebase = hdmiSrc->timebase;
        hdmiSrc->hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiSrc->hdmiInputId, &hdmiInputSettings, &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
        if (!hdmiSrc->hdmiInput) {
            BDBG_ERR(("%s: ERROR: Failed to open %d HDMI Input Handle", __FUNCTION__, hdmiSrc->hdmiInputId));
            goto error;
        }
        BDBG_MSG(("%s: NEXUS_HdmiInput_OpenWithEdid() is successful, timebase %d ...", __FUNCTION__, (int)hdmiSrc->timebase));
    }

    BDBG_MSG(("%s: %d HDMI Src Initialized", __FUNCTION__, NEXUS_NUM_HDMI_INPUTS));
    return 0;

error:
    BKNI_DestroyMutex(ipStreamerGlobalCtx->hdmiSrcMutex);
    return -1;
}

void
unInitNexusHdmiSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    HdmiSrc *hdmiSrc = NULL;

    for (i=0; i<NEXUS_NUM_HDMI_INPUTS; i++) {
        hdmiSrc = &ipStreamerGlobalCtx->hdmiSrcList[i];
        NEXUS_HdmiInput_Close(hdmiSrc->hdmiInput);
        if (hdmiSrc->timebase != NEXUS_Timebase_eInvalid)
            NEXUS_Timebase_Close(hdmiSrc->timebase);
        hdmiSrc->timebase = NEXUS_Timebase_eInvalid;

    }
    if (ipStreamerGlobalCtx->hdmiSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->hdmiSrcMutex);
    BDBG_MSG(("%s: HDMI Src Un-Initialized", __FUNCTION__));
}
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */

#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
void
closeNexusHdmiSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    HdmiSrc *hdmiSrc;
    if (!ipStreamerCtx || !ipStreamerCtx->hdmiSrc)
        return;
    hdmiSrc = ipStreamerCtx->hdmiSrc;

    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->hdmiSrcMutex);
    hdmiSrc->refCount--;
    if (hdmiSrc->refCount == 0) {
        /* Close transcoder resources */
        hdmiSrc->inUse = false;
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->hdmiSrcMutex);
    ipStreamerCtx->hdmiSrc = NULL;

    BDBG_MSG(("CTX %p: HDMI Src %p is closed (ref cnt %d)", (void *)ipStreamerCtx, (void *)hdmiSrc, hdmiSrc->refCount));
}
int
openNexusHdmiSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int i;
    int status = -1;
    HdmiSrc *hdmiSrc = NULL;

    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->hdmiSrcMutex);
    for (i = 0; i < NEXUS_NUM_HDMI_INPUTS; i++) {
        if (ipStreamerCtx->globalCtx->hdmiSrcList[i].hdmiInputId == ipStreamerCfg->hdmiInputId) {
            hdmiSrc = &ipStreamerCtx->globalCtx->hdmiSrcList[i];
            BDBG_MSG(("%s: Found Matching HDMI Src entry: idx %d, addr %p, total HDMI Src entries %d", __FUNCTION__, i, (void *)hdmiSrc, NEXUS_NUM_HDMI_INPUTS));
            break;
        }
    }
    if (!hdmiSrc) {
        BDBG_ERR(("%s: ERROR: No Free or matching HDMI Src Entry remaining, current used %d", __FUNCTION__, i));
        goto error;
    }

    if (hdmiSrc->refCount > 0) {
        BDBG_MSG(("%s: HDMI Src id %d is already setup for encoding, cur ref count %d", __FUNCTION__, hdmiSrc->hdmiInputId, hdmiSrc->refCount));
        ipStreamerCtx->hdmiSrc = hdmiSrc;
        ipStreamerCtx->transcoderDst = hdmiSrc->transcoderDst;
        ipStreamerCtx->transcoderDst->refCount++;
        hdmiSrc->refCount++;
    }
    else {
        /* setup the encode pipeline */
        ipStreamerCtx->hdmiSrc = hdmiSrc;
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        hdmiSrc->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (hdmiSrc->transcoderDst == NULL) {
            BDBG_ERR(("%s: Failed to open the transcoder pipe", __FUNCTION__));
            goto error;
        }

        hdmiSrc->inUse = true;
        hdmiSrc->refCount = 1;

    }
    status = 0;

error:
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->hdmiSrcMutex);
    if (status != 0)
        closeNexusHdmiSrc(ipStreamerCtx);
    return status;
}
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */
