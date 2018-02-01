/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "tspsimgr3.h"

BDBG_MODULE(live);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

static void live_p_setup(unsigned id)
{
    NEXUS_StcChannelSettings stcSettings;

    g_StandbyNexusHandles.videoPidChannel[id] = NEXUS_PidChannel_Open(g_StandbyNexusHandles.parserBand[id], g_DeviceState.opts[id].videoPid, NULL);
    g_StandbyNexusHandles.audioPidChannel[id] = NEXUS_PidChannel_Open(g_StandbyNexusHandles.parserBand[id], g_DeviceState.opts[id].audioPid, NULL);

    NEXUS_StcChannel_GetSettings(g_StandbyNexusHandles.stcChannel[id], &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = g_StandbyNexusHandles.videoPidChannel[id];
    NEXUS_StcChannel_SetSettings(g_StandbyNexusHandles.stcChannel[id], &stcSettings);

    if(g_DeviceState.power_mode == ePowerModeS0)
        decoder_setup(id);
}

static void scan_complete(void *data)
{
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static NEXUS_Error scan_program(tspsimgr_scan_results *scan_results, NEXUS_ParserBand parserBand)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    tspsimgr_t tspsimgr;
    tspsimgr_scan_settings scan_settings;
    BKNI_EventHandle feEvent;

    BKNI_CreateEvent(&feEvent);

    tspsimgr = tspsimgr_create();
    tspsimgr_get_default_start_scan_settings(&scan_settings);
    scan_settings.parserBand = parserBand;
    scan_settings.scan_done = scan_complete;
    scan_settings.context = feEvent;
    rc = tspsimgr_start_scan(tspsimgr, &scan_settings);
    if (rc != NEXUS_SUCCESS) goto error;

    BKNI_WaitForEvent(feEvent, 3000);
    /* even on timeout, work with partial scan results */

    tspsimgr_get_scan_results(tspsimgr, scan_results);
    tspsimgr_destroy(tspsimgr);

error:
    BKNI_DestroyEvent(feEvent);
    return rc;
}

int live_setup(unsigned id)
{
    tspsimgr_scan_results scan_results;
    NEXUS_Error rc;

    switch(g_DeviceState.source[id]) {
#if NEXUS_HAS_FRONTEND
        case eInputSourceQam:
            BKNI_ResetEvent(g_StandbyNexusHandles.signalLockedEvent);
            rc = tune_qam(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("Frontend failed to lock QAM signal during PSI acquisition"));
                BERR_TRACE(rc); goto err;
            }
            break;
        case eInputSourceSat:
            rc = tune_sat(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("Frontend failed to lock SAT signal during PSI acquisition"));
                BERR_TRACE(rc); goto err;
            }
            break;
        case eInputSourceOfdm:
            rc = tune_ofdm(id);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            rc = BKNI_WaitForEvent(g_StandbyNexusHandles.signalLockedEvent, 5000);
            if(rc) {
                BDBG_WRN(("Frontend failed to lock OFDM signal during PSI acquisition"));
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
    scan_program(&scan_results, g_StandbyNexusHandles.parserBand[id]);
    g_DeviceState.opts[id].videoPid = scan_results.program_info[id].video_pids[0].pid;
    g_DeviceState.opts[id].videoCodec = scan_results.program_info[id].video_pids[0].codec;
    g_DeviceState.opts[id].audioPid = scan_results.program_info[id].audio_pids[0].pid;
    g_DeviceState.opts[id].audioCodec = scan_results.program_info[id].audio_pids[0].codec;
    g_DeviceState.opts[id].pcrPid = scan_results.program_info[id].pcr_pid;

    live_p_setup(id);

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
