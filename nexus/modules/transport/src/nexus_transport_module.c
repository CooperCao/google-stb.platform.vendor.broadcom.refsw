/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_recpump_impl.h"
#include "priv/nexus_core.h"
#include "priv/nexus_tsmf_priv.h"
#if !BXPT_HAS_MESG_BUFFERS && !NEXUS_USE_SW_FILTER
#error chips without message filtering require NEXUS_USE_SW_FILTER
#endif
#if !NEXUS_USE_SW_FILTER
    #if BXPT_HAS_MESG_L2
        #include "bchp_int_id_xpt_msg_dat_err_intr_l2.h"
        #include "bchp_xpt_msg.h"
    #else
        #include "bchp_int_id_xpt_msg.h"
    #endif
#endif
#include "nexus_power_management.h"
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
#include "bchp_int_id_xpt_wakeup.h"
#endif

#include "bxpt_rave_ihex.h"

BDBG_MODULE(nexus_transport);

/* global instances */
struct NEXUS_Transport_P_State g_NEXUS_Transport_P_State;
#if BXPT_NUM_REMULTIPLEXORS
extern NEXUS_RemuxHandle g_remux[BXPT_NUM_REMULTIPLEXORS];
#endif

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
static NEXUS_ThreadHandle dpcr_integrator_workaround_thread;
static bool dpcr_integrator_workaround_thread_exit;

static BERR_Code do_dpcr_integrator_workaround (void)
{
    return BXPT_PCR_P_Integrator( pTransport->xpt );
}

static void dpcr_integrator_workaround_thread_entry(void *arg)
{
    NEXUS_Time prev;
    unsigned count=0;
    long max = 0;

    BSTD_UNUSED(arg);

    BDBG_WRN(("DPCR Thread: started"));
    NEXUS_Time_Get(&prev);
    for(;;) {
        NEXUS_Time now;
        long diff;
        BERR_Code rc = BERR_SUCCESS;

        BKNI_Sleep(2);  /* 2 mS */
        NEXUS_LockModule();
        if (!pTransport->standby.postResumePending) {
            if(dpcr_integrator_workaround_thread_exit) {
                NEXUS_UnlockModule();
                break;
            }
            rc = do_dpcr_integrator_workaround();
            if(rc!=BERR_SUCCESS) {
                NEXUS_UnlockModule();
                break;
            }
        }
        NEXUS_UnlockModule();
        NEXUS_Time_Get(&now);
        diff = NEXUS_Time_Diff(&now, &prev);
        prev = now;
        max = diff>max ? diff:max;
        count++;
    }
    BDBG_WRN(("DPCR Thread: Called %u times, max delay %ldms", count, max));
    return;
}

static void dpcr_integrator_workaround_start(void)
{
    NEXUS_ThreadSettings settings;
    NEXUS_Thread_GetDefaultSettings(&settings);
    dpcr_integrator_workaround_thread_exit = false;
    dpcr_integrator_workaround_thread = NEXUS_Thread_Create("dpcr_integrator_workaround",
        dpcr_integrator_workaround_thread_entry, NULL, &settings);
    return;
}

static void dpcr_integrator_workaround_stop(void)
{
    dpcr_integrator_workaround_thread_exit = true;
    if (dpcr_integrator_workaround_thread) {
        NEXUS_UnlockModule();
        NEXUS_Thread_Destroy(dpcr_integrator_workaround_thread);
        NEXUS_LockModule();
        dpcr_integrator_workaround_thread = NULL;
    }
    return;
}
#else /* BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND */
#define dpcr_integrator_workaround_start()
#define dpcr_integrator_workaround_stop()
#endif /* BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND */

#if BXPT_SW7425_1323_WORKAROUND
static NEXUS_ThreadHandle timer_thread;
static bool timer_thread_exit;

static BERR_Code do_flow_control(BXPT_Rave_Handle hRave)
{
    return BXPT_Rave_P_FlowControl( hRave );
}

static void timer_thread_entry(void *arg)
{
    NEXUS_Time prev;
    unsigned count=0;
    long max = 0;

    BSTD_UNUSED(arg);

    BDBG_WRN(("Timer: started"));
    NEXUS_Time_Get(&prev);
    for(;;) {
        NEXUS_Time now;
        long diff;
        unsigned i;
        BERR_Code rc = BERR_SUCCESS;

        BKNI_Sleep(1);
        NEXUS_LockModule();
        if (!pTransport->standby.postResumePending) {
            if(timer_thread_exit) {
                NEXUS_UnlockModule();
                break;
            }
            for(i=0;i<BXPT_NUM_RAVE_CHANNELS;i++) {
                rc = do_flow_control(pTransport->rave[i].channel);
                if(rc!=BERR_SUCCESS) {
                    break;
                }
            }
        }
        NEXUS_UnlockModule();
        NEXUS_Time_Get(&now);
        diff = NEXUS_Time_Diff(&now, &prev);
        prev = now;
        max = diff>max ? diff:max;
        count ++;
        if(rc!=BERR_SUCCESS) {
            break;
        }
    }
    BDBG_WRN(("Timer: Called %u times, max delay %ldms", count, max));
    return;
}

static void timer_start(void)
{
    NEXUS_ThreadSettings settings;
    NEXUS_Thread_GetDefaultSettings(&settings);
    timer_thread_exit = false;
    timer_thread = NEXUS_Thread_Create("playback_pacing", timer_thread_entry, NULL, &settings);
    return;
}

static void timer_stop(void)
{
    timer_thread_exit = true;
    if (timer_thread) {
        NEXUS_UnlockModule();
        NEXUS_Thread_Destroy(timer_thread);
        NEXUS_LockModule();
        timer_thread = NULL;
    }
    return;
}
#else /* BXPT_SW7425_1323_WORKAROUND */
#define timer_start()
#define timer_stop()
#endif /* BXPT_SW7425_1323_WORKAROUND */

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
static NEXUS_TimerHandle pipeline_error_checker_timer;
static void pipeline_checker_timer(void *arg)
{
    int rc;
    BXPT_PipelineErrors errors;
    unsigned i;
    uint32_t flags;
    uint32_t *count;
    BSTD_UNUSED(arg);

    pipeline_error_checker_timer = NULL;
    if (pTransport->standby.postResumePending) {
        rc = 0;
        goto resched;
    }
    BDBG_MSG(("check pipeline"));
    rc = BXPT_CheckPipelineErrors( pTransport->xpt, &errors );

    if (rc == BERR_SUCCESS) {
        if (errors.overflow.RsbuffMpodIbp) {
            flags = errors.overflow.RsbuffMpodIbp;
            count = pTransport->overflow.rsbuff.mpodIbp;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
        if (errors.overflow.RsbuffIbp) {
            flags = errors.overflow.RsbuffIbp;
            count = pTransport->overflow.rsbuff.ibp;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
        if (errors.overflow.XcbuffRaveIbp) {
            flags = errors.overflow.XcbuffRaveIbp;
            count = pTransport->overflow.xcbuff.ibp2rave;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
        if (errors.overflow.XcbuffMsgIbp) {
            flags = errors.overflow.XcbuffMsgIbp;
            count = pTransport->overflow.xcbuff.ibp2msg;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
        if (errors.overflow.XcbuffRmx0Ibp) {
            flags = errors.overflow.XcbuffRmx0Ibp;
            count = pTransport->overflow.xcbuff.ibp2rmx0;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
        if (errors.overflow.XcbuffRmx1Ibp) {
            flags = errors.overflow.XcbuffRmx1Ibp;
            count = pTransport->overflow.xcbuff.ibp2rmx1;
            for (i=0; i<32; i++) { if ((flags >> i) & 1) { count[i]++; } }
        }
    }

resched:
    if (rc != BERR_NOT_SUPPORTED) {
        pipeline_error_checker_timer = NEXUS_ScheduleTimer(2000, pipeline_checker_timer, NULL);
    }
}

static void pipeline_checker_start(void)
{
    BDBG_ASSERT(!pipeline_error_checker_timer);
    pipeline_error_checker_timer = NEXUS_ScheduleTimer(2000, pipeline_checker_timer, NULL);
}

static void pipeline_checker_stop(void)
{
    if (pipeline_error_checker_timer) {
        NEXUS_CancelTimer(pipeline_error_checker_timer);
        pipeline_error_checker_timer = NULL;
    }
}
#else /* BXPT_HAS_PIPELINE_ERROR_REPORTING */
#define pipeline_checker_start()
#define pipeline_checker_stop()
#endif /* BXPT_HAS_PIPELINE_ERROR_REPORTING */

#if BXPT_HAS_WAKEUP_PKT_SUPPORT
static void NEXUS_Transport_P_PacketFound_isr(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BXPT_Wakeup_ClearInterruptToPMU_isr(pTransport->xpt);
}
#endif
void NEXUS_TransportModule_GetDefaultInternalSettings(NEXUS_TransportModuleInternalSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void NEXUS_TransportModule_GetDefaultSettings(NEXUS_TransportModuleSettings *pSettings)
{
    BXPT_DefaultSettings xptsettings;
    unsigned i, j;

    BSTD_UNUSED(i); /* in case not used */
    BSTD_UNUSED(j); /* in case not used */
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);

    pSettings->initVcxos = true;

    /* we don't have a chp handle here if called from NEXUS_Platform_GetDefaultSettings.
       BXPT_GetDefaultSettings doesn't use the chp handle so pass in a bogus one */
    BXPT_GetDefaultSettings(&xptsettings, (BCHP_Handle)1);

#if (BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG)
    #if NEXUS_NUM_PARSER_BANDS
    for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
        pSettings->maxDataRate.parserBand[i] = xptsettings.BandwidthConfig.MaxInputRate[i];
        pSettings->clientEnabled.parserBand[i].rave = xptsettings.BandwidthConfig.IbParserClients[i].ToRave;
        pSettings->clientEnabled.parserBand[i].message = xptsettings.BandwidthConfig.IbParserClients[i].ToMsg;
        #if BXPT_NUM_REMULTIPLEXORS
        for( j = 0; j < BXPT_NUM_REMULTIPLEXORS; j++ )
        {
            pSettings->clientEnabled.parserBand[i].remux[ j ] = xptsettings.BandwidthConfig.IbParserClients[i].ToRmx[ j ];
        }
        #endif
        pSettings->clientEnabled.parserBand[i].mpodRs = xptsettings.BandwidthConfig.IbParserClients[i].ToMpodRs;
    }
    #endif

    #if NEXUS_NUM_PLAYPUMPS && BXPT_HAS_PLAYBACK_PARSERS
    for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++)
    {
        pSettings->clientEnabled.playback[i].rave = xptsettings.BandwidthConfig.PlaybackParserClients[i].ToRave;
        pSettings->clientEnabled.playback[i].message = xptsettings.BandwidthConfig.PlaybackParserClients[i].ToMsg;
        #if BXPT_NUM_REMULTIPLEXORS
        for( j = 0; j < BXPT_NUM_REMULTIPLEXORS; j++ )
        {
            pSettings->clientEnabled.playback[i].remux[ j ] = xptsettings.BandwidthConfig.PlaybackParserClients[i].ToRmx[ j ];
        }
        #endif
    }
    #endif
#endif

#if BXPT_NUM_MTSIF
    for (i=0;i<BXPT_NUM_MTSIF;i++) {
        pSettings->mtsif[i].enabled = true; /* XPT defaults to disabled, but nexus defaults to enabled if BXPT_NUM_MTSIF>0*/
        pSettings->mtsif[i].rxInterfaceWidth = xptsettings.MtsifConfig[i].RxInterfaceWidth;
    }
#endif

#if NEXUS_NUM_PLAYPUMPS && BXPT_HAS_PLAYBACK_PARSERS
    for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++) {
        pSettings->maxDataRate.playback[i] = xptsettings.BandwidthConfig.MaxPlaybackRate[i];
    }
#endif
    pSettings->syncInCount = xptsettings.syncInCount;
    pSettings->syncOutCount = xptsettings.syncOutCount;

    return;
}

void NEXUS_InputBand_P_GetDefaultSettings(NEXUS_InputBand inputBand, NEXUS_InputBandSettings *pSettings)
{
#if NEXUS_MAX_INPUT_BANDS
    BXPT_InputBandConfig cfg;
    BERR_Code rc;
#else
    BSTD_UNUSED(inputBand);
#endif

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* Defaults taken from RDB defaults for now -- needs a BXPT default function */
    pSettings->clockActiveHigh = true;
    pSettings->dataLsbFirst = false;
    pSettings->dataActiveHigh = true;
    pSettings->validEnabled = false;
    pSettings->validActiveHigh = true;
    pSettings->useInternalSync = true;   /* internal sync detect should be on for MPEG TS by default */
    pSettings->syncActiveHigh = true;
    pSettings->errorEnabled = false;
    pSettings->errorActiveHigh = true;
    pSettings->packetLength = 188;

#if NEXUS_MAX_INPUT_BANDS
    rc = BXPT_GetInputBandConfig(pTransport->xpt, inputBand, &cfg);
    if (!rc) {
        pSettings->parallelInput = cfg.ParallelInputSel;
    }
    else {
        pSettings->parallelInput = false;
    }
#endif
}

#if BXPT_NUM_PCRS
static NEXUS_Error NEXUS_Timebase_P_Init(unsigned index)
{
    BERR_Code  rc = BERR_SUCCESS;
    BXPT_PCR_DefSettings pcr_cfg;
    NEXUS_TimebaseHandle timebase;

    timebase = BKNI_Malloc(sizeof(*timebase));
    if (!timebase) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(timebase, 0, sizeof(*timebase));
    BDBG_OBJECT_SET(timebase, NEXUS_Timebase);
    pTransport->timebase[index] = timebase;

    rc = BXPT_PCR_GetChannelDefSettings(pTransport->xpt, index, &pcr_cfg);
    if (rc) {rc=BERR_TRACE(rc); goto done;}
    rc = BXPT_PCR_Open(pTransport->xpt, index, &pcr_cfg, &timebase->pcr);
    if (rc) {rc=BERR_TRACE(rc); goto done;}

    NEXUS_Timebase_GetDefaultSettings(&timebase->settings);

    timebase->hwIndex = index;
    timebase->status.timebase = index;
    timebase->enumTimebase = NEXUS_Timebase_e0 + index; /* assumes enums are continuous */
    timebase->monitorCallback = NEXUS_IsrCallback_Create(timebase, NULL);
    timebase->pcrErrorCallback = NEXUS_IsrCallback_Create(timebase, NULL);
#if NEXUS_HAS_ASTM
    timebase->astm.permitted = false;
    BKNI_Memset(&timebase->astm.settings, 0, sizeof(NEXUS_TimebaseAstmSettings));
#endif

    NEXUS_Timebase_P_StartMonitor(timebase);
done:
    return rc;
}

static void NEXUS_Timebase_P_Uninit(NEXUS_TimebaseHandle timebase)
{
    BERR_Code rc = BERR_SUCCESS;
    if (timebase->settings.sourceType == NEXUS_TimebaseSourceType_ePcr) {
        /* release the pcr pid channel */
        NEXUS_TimebaseSettings settings = timebase->settings;
        settings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        NEXUS_Timebase_P_SetSettings(timebase, &settings);
    }
    NEXUS_Timebase_P_StopMonitor(timebase);
    NEXUS_IsrCallback_Destroy(timebase->monitorCallback);
    NEXUS_IsrCallback_Destroy(timebase->pcrErrorCallback);
    rc = BXPT_PCR_Close(timebase->pcr);
    if (rc) rc=BERR_TRACE(rc); /* keep going */
    pTransport->timebase[timebase->hwIndex] = NULL;
    BDBG_OBJECT_DESTROY(timebase, NEXUS_Timebase);
    BKNI_Free(timebase);
}
#endif /* BXPT_NUM_PCRS */

static void NEXUS_TransportModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;
    NEXUS_P_HwPidChannel *pidChannel;
    BDBG_LOG(("transport"));
    #if NEXUS_MAX_INPUT_BANDS
    for (i=0; i<NEXUS_MAX_INPUT_BANDS; i++) {
        if (pTransport->inputBand[i].enabled) {
            BDBG_LOG(("input band %d: enabled pktsize=%u", i, pTransport->inputBand[i].settings.packetLength));
        }
    }
    #endif

    #if NEXUS_NUM_PARSER_BANDS
    for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
        NEXUS_ParserBandHandle pb = pTransport->parserBand[i];
        if (pb) {
            const char *sourceTypeStr;
            unsigned long sourceIndex;
            switch (pb->settings.sourceType) {
            case NEXUS_ParserBandSourceType_eInputBand:
                sourceTypeStr = "IB";
                sourceIndex = pb->settings.sourceTypeSettings.inputBand;
                break;
            case NEXUS_ParserBandSourceType_eRemux:
                sourceTypeStr = "RMX";
                sourceIndex = pb->settings.sourceTypeSettings.remux->index;
                break;
            case NEXUS_ParserBandSourceType_eMtsif:
                sourceTypeStr = "MTSIF";
                sourceIndex = (unsigned long)pb->settings.sourceTypeSettings.mtsif;
                break;
            #if NEXUS_TRANSPORT_EXTENSION_TSMF
            case NEXUS_ParserBandSourceType_eTsmf:
                sourceTypeStr = "TSMF";
                sourceIndex = (unsigned long)pb->settings.sourceTypeSettings.tsmf;
                break;
            #endif
            default:
                sourceTypeStr = "error";
                sourceIndex = pb->settings.sourceType;
                break;
            }
            BDBG_LOG(("parser band %d: source %s %#lx, enabled %c, pid channels %d, cc errors %d, tei errors %d, length errors %d",
                pb->hwIndex, sourceTypeStr, sourceIndex,
                pb->settings.sourceType == NEXUS_ParserBandSourceType_eMtsif?'-':pb->enabled?'y':'n',
                pb->pidChannels, pb->ccErrorCount, pb->teiErrorCount, pb->lengthErrorCount));
        }
    }
    #endif
    #if NEXUS_NUM_PLAYPUMPS
    for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++) {
        NEXUS_PlaypumpHandle p = pTransport->playpump[i].playpump;
        if (p) {
            NEXUS_PlaypumpStatus status;
            if (NEXUS_Playpump_GetStatus(p, &status)) continue;
            BDBG_LOG(("playpump %d: %s, fifo %u/%u(%u%%)", status.index,
                status.started?"started":"stopped",
                (unsigned)status.fifoDepth, (unsigned)status.fifoSize, status.fifoSize?(unsigned)(status.fifoDepth*100/status.fifoSize):0));
        }
    }
    #endif
    for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
        if (pTransport->recpump[i]) {
            NEXUS_RecpumpStatus status;
            NEXUS_Recpump_P_PidChannel *pid;
            if (NEXUS_Recpump_GetStatus(pTransport->recpump[i], &status)) continue;
            BDBG_LOG(("recpump %d: %s, RAVE %u, CDB fifo %u/%u(%u%%), ITB fifo %u/%u(%u%%)", status.rave.index,
                status.started?"started":"stopped", status.rave.index,
                (unsigned)status.data.fifoDepth, (unsigned)status.data.fifoSize, status.data.fifoSize?(unsigned)(status.data.fifoDepth*100/status.data.fifoSize):0,
                (unsigned)status.index.fifoDepth, (unsigned)status.index.fifoSize, status.index.fifoSize?(unsigned)(status.index.fifoDepth*100/status.index.fifoSize):0));

            for (pid=BLST_S_FIRST(&pTransport->recpump[i]->pid_list); pid; pid=BLST_S_NEXT(pid, link)) {
                BDBG_LOG(("  added_pidchannel %u", pid->pidChn->hwPidChannel->status.pidChannelIndex));
            }
        }
    }

    for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
        NEXUS_PidChannelStatus status;
        NEXUS_PidChannelHandle swPidChannel;
        if (NEXUS_P_HwPidChannel_GetStatus(pidChannel, &status)) continue;

        for (swPidChannel = BLST_S_FIRST(&pidChannel->swPidChannels); swPidChannel; swPidChannel = BLST_S_NEXT(swPidChannel, link)) {
            BDBG_LOG(("pidchannel %p: ch %d, %s %u, pid %#x", (void *)swPidChannel, status.pidChannelIndex, status.playback?"playback":"parser",
                status.playback ? status.playbackIndex : status.parserBand, status.pid));
        }
    }


    for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
        struct NEXUS_Rave* rave = pTransport->rave[0].context[i];
        if (rave && rave->enabled) {
            unsigned cdbDepth, cdbSize;
            NEXUS_PidChannelStatus status;
            BKNI_EnterCriticalSection();
            NEXUS_Rave_GetCdbBufferInfo_isr(rave, &cdbDepth, &cdbSize);
            BKNI_LeaveCriticalSection();
            if (NEXUS_P_HwPidChannel_GetStatus(rave->pidChannel, &status)) continue;

            BDBG_LOG(("RAVE%d: type=%s, pidch=%u, type=%u:%u, CDB fifo %d/%d(%d%%)",
                rave->index, "decode",
                status.pidChannelIndex, status.originalTransportType, status.transportType,
                cdbDepth, cdbSize, cdbDepth*100/cdbSize
                ));
        }
    }

    for (i=0;i<BXPT_NUM_PCR_OFFSET_CHANNELS;i++) {
        NEXUS_StcChannelHandle stcChannel = pTransport->stcChannel[i];
        if (stcChannel) {
            uint32_t stc;
            NEXUS_StcChannel_GetStc(stcChannel, &stc);
            switch (stcChannel->settings.mode) {
            case NEXUS_StcChannelMode_ePcr:
                BDBG_LOG(("stcChannel %p: pcr mode, pcrPidChannel %p, stc %d, timebase %d, value %#x, offset %#x, %s",
                    (void *)stcChannel, (void *)stcChannel->settings.modeSettings.pcr.pidChannel, stcChannel->stcIndex, stcChannel->timebase->hwIndex, stc, BXPT_PcrOffset_GetOffset(stcChannel->pcrOffset), BXPT_PcrOffset_IsOffsetValid(stcChannel->pcrOffset)?"valid":"invalid"));
                break;
            case NEXUS_StcChannelMode_eAuto:
                {
                static const char *g_master[NEXUS_StcChannelAutoModeBehavior_eMax] = {"first avail","video","audio"};
                BDBG_LOG(("stcChannel %p: auto mode, %s master, stc %d, timebase %d, value %#x, %s",
                    (void *)stcChannel, g_master[stcChannel->settings.modeSettings.Auto.behavior], stcChannel->stcIndex, stcChannel->timebase->hwIndex, stc, stcChannel->stcValid?"valid":"invalid"));
                }
                break;
            case NEXUS_StcChannelMode_eHost:
                BDBG_LOG(("stcChannel %p: host mode, stc %d, timebase %d, value %#x, %s",
                    (void *)stcChannel, stcChannel->stcIndex, stcChannel->timebase->hwIndex, stc, stcChannel->stcValid?"valid":"invalid"));
                break;
            default:
                break;
            }
        }
    }
    for (i=0;i<BXPT_NUM_PCRS;i++) {
        NEXUS_TimebaseHandle timebase = pTransport->timebase[i];
        if (timebase->acquired) {
            char str[64];
            unsigned n;
            static const char *g_source[NEXUS_TimebaseSourceType_eMax] = {"pcr","freerun","analog","hddvi","ccir656","i2s","spdif"};
            n = BKNI_Snprintf(str, sizeof(str), "timebase %d: %s", timebase->hwIndex, g_source[timebase->settings.sourceType]);
            if (timebase->settings.sourceType == NEXUS_TimebaseSourceType_ePcr) {
                BKNI_Snprintf(&str[n], sizeof(str)-n, ", pidChannel %p", (void *)timebase->settings.sourceSettings.pcr.pidChannel);
            }
            BDBG_LOG(("%s", str));
        }
    }
#endif
}

NEXUS_ModuleHandle NEXUS_TransportModule_PreInit( const NEXUS_TransportModuleInternalSettings *pModuleSettings, const NEXUS_TransportModuleSettings  *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;

    if(!pSettings || !pModuleSettings) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    /* init global module data */
    BKNI_Memset(&g_NEXUS_Transport_P_State, 0, sizeof(g_NEXUS_Transport_P_State));
#if NEXUS_TRANSPORT_EXTENSION_TSMF
    BKNI_Memset(&g_NEXUS_Tsmf_P_State, 0, sizeof(g_NEXUS_Tsmf_P_State));
#endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eHigh, &pSettings->common); /* transport interface is fast */
    moduleSettings.dbgPrint = NEXUS_TransportModule_P_Print;
    moduleSettings.dbgModules = "nexus_transport";
    g_NEXUS_Transport_P_State.transportModule = NEXUS_Module_Create("transport", &moduleSettings);
    if (!g_NEXUS_Transport_P_State.transportModule) {
          return NULL;
    }
    pTransport->settings = *pSettings;
    pTransport->moduleSettings = *pModuleSettings;

    if (!pModuleSettings->postInitCalledBySecurity) {
        int rc;
        NEXUS_LockModule();
        rc = NEXUS_TransportModule_PostInit_priv(NULL);
        NEXUS_UnlockModule();
        if (rc) {
            NEXUS_Module_Destroy(g_NEXUS_Transport_P_State.transportModule);
            return NULL;
        }
    }

    return g_NEXUS_Transport_P_State.transportModule;
}

NEXUS_Error NEXUS_TransportModule_PostInit_priv(RaveChannelOpenCB rave_regver)
{
    BERR_Code rc;
    unsigned i, j;
    BXPT_DefaultSettings *pXptSettings = NULL;
    BXPT_Handle xpt; /* shorthand */
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    BXPT_Wakeup_Settings wakeupSettings;
#endif
    const NEXUS_TransportModuleSettings *pSettings = &pTransport->settings;

    pXptSettings = BKNI_Malloc(sizeof(*pXptSettings));
    if (!pXptSettings) { return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); }

    BSTD_UNUSED(j); /* in case not used */

    BXPT_GetDefaultSettings(pXptSettings, g_pCoreHandles->chp);

    pXptSettings->memHeap = g_pCoreHandles->heap[pTransport->moduleSettings.mainHeapIndex].mem;
    if (pTransport->moduleSettings.secureHeap) {
        pXptSettings->mmaRHeap = NEXUS_Heap_GetMmaHandle(pTransport->moduleSettings.secureHeap);
        pXptSettings->memRHeap = NEXUS_Heap_GetMemHandle(pTransport->moduleSettings.secureHeap);
    }

    /* verify API macros aren't below nexus_platform_features.h */
    BDBG_CASSERT(NEXUS_MAX_PARSER_BANDS >= NEXUS_NUM_PARSER_BANDS);
    BDBG_CASSERT(NEXUS_MAX_PLAYPUMPS >= NEXUS_NUM_PLAYPUMPS);
#if BXPT_NUM_REMULTIPLEXORS
    BDBG_CASSERT(NEXUS_MAX_REMUX >= BXPT_NUM_REMULTIPLEXORS);
#endif
    BDBG_CASSERT(NEXUS_NUM_PARSER_BANDS <= BXPT_NUM_PID_PARSERS);
    BDBG_CASSERT(NEXUS_NUM_PLAYPUMPS <= BXPT_NUM_PLAYBACKS);

#if (BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG)

    #if NEXUS_NUM_PARSER_BANDS
    for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
        pXptSettings->BandwidthConfig.MaxInputRate[i] = pSettings->maxDataRate.parserBand[i];
        pXptSettings->BandwidthConfig.IbParserClients[i].ToRave = pSettings->clientEnabled.parserBand[i].rave;
        pXptSettings->BandwidthConfig.IbParserClients[i].ToMsg = pSettings->clientEnabled.parserBand[i].message;
        #if BXPT_NUM_REMULTIPLEXORS
        for(j=0;j<BXPT_NUM_REMULTIPLEXORS;j++)
        {
            pXptSettings->BandwidthConfig.IbParserClients[i].ToRmx[ j ] = pSettings->clientEnabled.parserBand[i].remux[ j ];
        }
        #endif
        pXptSettings->BandwidthConfig.IbParserClients[i].ToMpodRs = pSettings->clientEnabled.parserBand[i].mpodRs;
    }
    #endif

#if NEXUS_NUM_PARSER_BANDS < BXPT_NUM_PID_PARSERS
    for (i=NEXUS_NUM_PARSER_BANDS;i<BXPT_NUM_PID_PARSERS;i++) {
        pXptSettings->BandwidthConfig.MaxInputRate[i] = 0;
        pXptSettings->BandwidthConfig.IbParserClients[i].ToRave = false;
        pXptSettings->BandwidthConfig.IbParserClients[i].ToMsg = false;
        #if BXPT_NUM_REMULTIPLEXORS
        for (j=0;j<BXPT_NUM_REMULTIPLEXORS;j++)
        {
            pXptSettings->BandwidthConfig.IbParserClients[i].ToRmx[ j ] = false;
        }
        #endif
        pXptSettings->BandwidthConfig.IbParserClients[i].ToMpodRs = false;
    }
#endif

#if NEXUS_NUM_PLAYPUMPS
    for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++) {
        pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToRave = pSettings->clientEnabled.playback[i].rave;
        pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToMsg = pSettings->clientEnabled.playback[i].message;
        #if BXPT_NUM_REMULTIPLEXORS
        for(j=0;j<BXPT_NUM_REMULTIPLEXORS;j++)
        {
            pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToRmx[ j ] = pSettings->clientEnabled.playback[i].remux[ j ];
        }
        #endif
    }
    for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++) {
        pXptSettings->BandwidthConfig.MaxPlaybackRate[i] = pSettings->maxDataRate.playback[i];
    }
#endif

#if NEXUS_NUM_PLAYPUMPS && NEXUS_NUM_PLAYPUMPS < BXPT_NUM_PLAYBACKS
    for (i=NEXUS_NUM_PLAYPUMPS;i<BXPT_NUM_PLAYBACKS;i++) {
        pXptSettings->BandwidthConfig.MaxPlaybackRate[i] = 0;
        pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToRave = false;
        pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToMsg = false;
        #if BXPT_NUM_REMULTIPLEXORS
        for (j=0;j<BXPT_NUM_REMULTIPLEXORS;j++)
        {
            pXptSettings->BandwidthConfig.PlaybackParserClients[i].ToRmx[ j ] = false;
        }
        #endif
    }
#endif

#else /* NEXUS_HAS_LEGACY_XPT */

    /* reduce XPT memory if user has limited capabilities */
#if NEXUS_NUM_PARSER_BANDS < BXPT_NUM_PID_PARSERS
    for (i=NEXUS_NUM_PARSER_BANDS;i<BXPT_NUM_PID_PARSERS;i++) {
        pXptSettings->DramBuffers.IbParserRsSize[i] = 0;
        pXptSettings->DramBuffers.RaveXcCfg.IbParserXcSize[i] = 0;
        pXptSettings->DramBuffers.MesgBufXcCfg.IbParserXcSize[i] = 0;
    }
#endif
#if NEXUS_NUM_PLAYPUMPS < BXPT_NUM_PLAYBACKS
    for (i=NEXUS_NUM_PLAYPUMPS;i<BXPT_NUM_PLAYBACKS;i++) {
        pXptSettings->DramBuffers.PbParserRsSize[i] = 0;
        pXptSettings->DramBuffers.RaveXcCfg.PbParserXcSize[i] = 0;
        pXptSettings->DramBuffers.MesgBufXcCfg.PbParserXcSize[i] = 0;
    }
#endif
#endif

#if BXPT_NUM_MTSIF
    for (i=0;i<BXPT_NUM_MTSIF;i++) {
        pXptSettings->MtsifConfig[i].Enable = pSettings->mtsif[i].enabled;
        pXptSettings->MtsifConfig[i].RxInterfaceWidth = pSettings->mtsif[i].rxInterfaceWidth;
    }
#endif
    pXptSettings->syncInCount = pSettings->syncInCount;
    pXptSettings->syncOutCount = pSettings->syncOutCount;

    rc = BXPT_Open(&xpt, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->heap[pTransport->moduleSettings.mainHeapIndex].mma, g_pCoreHandles->bint, pXptSettings);
    if (rc) {rc=BERR_TRACE(rc); goto done;}

    pTransport->xpt = xpt;

#if B_REFSW_DSS_SUPPORT && !NEXUS_USE_SW_FILTER
    BXPT_DirecTv_SaveMptFlag(pTransport->xpt, pSettings->dssSaveMptFlag);
#endif

#if BXPT_NUM_PCRS
    BDBG_CASSERT(NEXUS_Timebase_eMax >= BXPT_NUM_PCRS); /* make sure no platform has gone beyond the enums */
    for (i=0;i<BXPT_NUM_PCRS;i++) {
        rc = NEXUS_Timebase_P_Init(i);
        if (rc) {rc=BERR_TRACE(rc); goto done;}
    }
#endif

#if BXPT_NUM_RAVE_CHANNELS
    for (i=0;i<BXPT_NUM_RAVE_CHANNELS;i++) {
        BXPT_Rave_ChannelSettings rave_channel_settings;

        rc = BXPT_Rave_GetChannelDefaultSettings(xpt, i, &rave_channel_settings);
        if (rc) {rc=BERR_TRACE(rc); goto done;}

        rave_channel_settings.chanOpenCB = rave_regver;
        rave_channel_settings.ThresholdGranularityInBytes = NEXUS_RAVE_THRESHOLD_UNITS;
        /* for nexus naming, I kept the NEXUS_TransportModuleInternalSettings and NEXUS_RecpumpSettings names in sync. this means the naming
        doesn't match magnum exactly. */
        rave_channel_settings.TpitEventTimeout = pSettings->tpit.idleEventTimeout;
        rave_channel_settings.TpitPacketTimeout = pSettings->tpit.recordEventTimeout;
        rc = BXPT_Rave_OpenChannel(xpt, &pTransport->rave[i].channel, i, &rave_channel_settings);
        if (rc) {rc=BERR_TRACE(rc); goto done;}

        /* contexts are opened with the RAVE interface */
        BKNI_Memset(pTransport->rave[i].context, 0, sizeof(pTransport->rave[i].context));
    }
#endif

#if NEXUS_MAX_INPUT_BANDS
    for (i=0;i<NEXUS_MAX_INPUT_BANDS;i++) {
        NEXUS_InputBandSettings settings;
        /* Set defaults */
        pTransport->inputBand[i].enabled = false;
        if (!BXPT_P_InputBandIsSupported(i)) continue;

        NEXUS_InputBand_P_GetDefaultSettings(i, &settings);
        NEXUS_InputBand_SetSettings(i, &settings);
    }
#endif

    rc = BPCRlib_Open(&pTransport->pcrlib, g_pCoreHandles->chp);
    if (rc) {rc=BERR_TRACE(rc); goto done;}

    /* required to be called once before use. */
#if NEXUS_HAS_LEGACY_XPT
#if !NEXUS_USE_SW_FILTER
    BXPT_Mesg_SetPid2Buff(pTransport->xpt, true);
#endif
#endif

#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    BXPT_Wakeup_GetDefaults(&wakeupSettings );
    pTransport->wakeup.settings = BKNI_Malloc(sizeof(*pTransport->wakeup.settings));
    if (!pTransport->wakeup.settings) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto done; }
    pTransport->wakeup.settings->inputBand = wakeupSettings.InputBand;
    pTransport->wakeup.settings->packetLength = wakeupSettings.PacketLength;
    pTransport->wakeup.settings->errorIgnore = wakeupSettings.ErrorInputIgnore;
    /* Create a callback for packet wakeup interrupt */
    rc = BINT_CreateCallback(&pTransport->wakeup.intPacketFoundCallback, g_pCoreHandles->bint, BCHP_INT_ID_PKT_DETECT, NEXUS_Transport_P_PacketFound_isr, NULL, 0);
    if (rc) {rc = BERR_TRACE(rc); goto done;}
#endif

    timer_start();
    pipeline_checker_start();
    dpcr_integrator_workaround_start();

    NEXUS_Vcxo_Init();

done:
    if (pXptSettings) {
        BKNI_Free(pXptSettings);
    }
    if (rc) {
        /* TODO: unwind Init? */
        return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_TransportModule_GetRaveFirmware_isrsafe( void **ppBxptRaveInitData,
                                                    unsigned *pBxptRaveInitDataSize  /* size of firmware in bytes. */ )
{
    *ppBxptRaveInitData    = (void*)BxptRaveInitData;
    *pBxptRaveInitDataSize = (unsigned)BxptRaveInitDataSize*sizeof(uint32_t);
    return;
}


void NEXUS_TransportModule_Uninit(void)
{
    unsigned i;
    BERR_Code rc;
    NEXUS_P_HwPidChannelHandle pidChannel;
    NEXUS_P_HwPidChannelHandle lastPidChannel;

    if (!pTransport->xpt) {
        goto destroy_module;
    }

    NEXUS_LockModule();
    BDBG_ASSERT(pTransport->transportModule);

    timer_stop();
    pipeline_checker_stop();
    dpcr_integrator_workaround_stop();

#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    BINT_DestroyCallback(pTransport->wakeup.intPacketFoundCallback);
    BKNI_Free(pTransport->wakeup.settings);
#endif

    for(i=0; i<BXPT_NUM_RAVE_CONTEXTS; i++) {
        NEXUS_RecpumpHandle recpump = pTransport->recpump[i];
        if (recpump) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_Recpump, recpump, Destroy);
            NEXUS_Recpump_Close(recpump);
        }
    }
#if BXPT_NUM_RAVE_CHANNELS
    for (i=0;i<BXPT_NUM_RAVE_CHANNELS;i++) {
        rc = BXPT_Rave_CloseChannel(pTransport->rave[i].channel);
        if (rc) rc=BERR_TRACE(rc); /* keep going */
    }
#endif
#if BXPT_NUM_PCR_OFFSET_CHANNELS
    for (i=0;i<BXPT_NUM_PCR_OFFSET_CHANNELS;i++) {
        NEXUS_StcChannelHandle stcChannel = pTransport->stcChannel[i];
        if (stcChannel) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_StcChannel, stcChannel, Destroy);
            NEXUS_StcChannel_Close(stcChannel);
        }
    }
#endif
    BPCRlib_Close(pTransport->pcrlib);
#if BXPT_NUM_PCRS
    for (i=0;i<BXPT_NUM_PCRS;i++) {
        NEXUS_Timebase_P_Uninit(pTransport->timebase[i]);
    }
#endif
    lastPidChannel = NULL;
    while ((pidChannel = BLST_S_FIRST(&pTransport->pidChannels))) {
        /*
         * pid channels, being managed resources, may not fully get closed
         * here if something is still holding onto one in a higher module/layer.
         * We need to avoid infinite loop if we can't close them all.
         */
        if (pidChannel == lastPidChannel) {
            BDBG_ERR(("hwPidChannel %p left open", (void *)pidChannel));
            break;
        }
        NEXUS_P_HwPidChannel_CloseAll(pidChannel);
        lastPidChannel = pidChannel;
    }
    /* HW pid channel refcnt ought to be at zero now */
    for (i=0;i<NEXUS_NUM_PID_CHANNELS;i++) {
        if (pTransport->hwPidChannelRefCnt[i]) {
            BDBG_ERR(("hwPidChannelRefCnt[%d] = %d", i, pTransport->hwPidChannelRefCnt[i]));
        }
    }

#if NEXUS_NUM_PARSER_BANDS
    NEXUS_ParserBand_P_UninitAll();
#endif

#if NEXUS_HAS_XPT_DMA
    {
        NEXUS_DmaHandle dma;
        for (i=0; i<NEXUS_NUM_DMA_CHANNELS; i++) {
            while ( (dma = BLST_S_FIRST(&pTransport->dmaChannel[i].dmaHandles)) ) {
                BDBG_WRN(("Automatically closing DMA %p", (void *)dma));
                NEXUS_OBJECT_UNREGISTER(NEXUS_Dma, dma, Destroy);
                NEXUS_Dma_Close(dma);
            }
        }
    }
#endif

    NEXUS_Vcxo_Uninit();
    BXPT_Close(pTransport->xpt);
    NEXUS_UnlockModule();

destroy_module:
    NEXUS_Module_Destroy(pTransport->transportModule);
    pTransport->transportModule = NULL;
    return;
}

/****
The following code handles XPT interrupts and dispatches based on a looked-up pid channel
****/

#if !NEXUS_USE_SW_FILTER
static void NEXUS_Transport_P_FireInterrupt_isr(unsigned pidChannelIndex, NEXUS_XptDataInterrupt xptDataInterrupt)
{
    unsigned i;
    for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
        NEXUS_MessageHandle msg = pTransport->message.handle[i];
        if (msg) {
            NEXUS_Message_P_FireInterrupt_isr(msg, pidChannelIndex, xptDataInterrupt);
        }
    }
}

static void NEXUS_Transport_P_MessageError_isr(void *context, int param)
{
    unsigned pid_channel_group;

    BSTD_UNUSED(context);
    for (pid_channel_group=0;pid_channel_group<4;pid_channel_group++) {
        unsigned i;
        uint32_t addr = BCHP_XPT_MSG_BUF_ERR_00_31 + sizeof(uint32_t)*pid_channel_group;
        uint32_t status = BREG_Read32(g_pCoreHandles->reg, addr);
        BREG_Write32(g_pCoreHandles->reg, addr, 0);
        for (i=0;i<32;i++) {
            if (status & (1<<i)) {
                unsigned pidChannelIndex = pid_channel_group * 32 + i;
                NEXUS_Transport_P_FireInterrupt_isr(pidChannelIndex, param);
            }
        }
    }
}
#endif /* !NEXUS_USE_SW_FILTER */

void NEXUS_Transport_P_SetInterrupts(void)
{
#if !NEXUS_USE_SW_FILTER
    BERR_Code rc;
    NEXUS_XptDataInterrupt xptDataInterrupt;

    for (xptDataInterrupt=0;xptDataInterrupt<NEXUS_XptDataInterrupt_eMax;xptDataInterrupt++) {
        unsigned i;
        bool hasCallback = false;
        for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
            NEXUS_MessageHandle msg = pTransport->message.handle[i];
            if (msg) {
                hasCallback = NEXUS_Message_P_HasCallback(msg, xptDataInterrupt);
                if (hasCallback)
                    break;
            }
        }

        if (hasCallback) {
            if (!pTransport->message.xptDataInterrupt[xptDataInterrupt]) {
                unsigned intId;
                switch (xptDataInterrupt) {
#if BXPT_HAS_MESG_L2
                case NEXUS_XptDataInterrupt_ePsiLengthError: intId = BCHP_INT_ID_PSI_ERROR_INTR; break;
                case NEXUS_XptDataInterrupt_eCrcError: intId = BCHP_INT_ID_CRC_ERROR_INTR; break;
                case NEXUS_XptDataInterrupt_ePesLengthError: intId = BCHP_INT_ID_PES_ERROR_INTR; break;
                case NEXUS_XptDataInterrupt_ePesStartCodeError: intId = BCHP_INT_ID_PSC_ERROR_INTR; break;
#else
                case NEXUS_XptDataInterrupt_ePsiLengthError: intId = BCHP_INT_ID_PSI_ERROR; break;
                case NEXUS_XptDataInterrupt_eCrcError: intId = BCHP_INT_ID_CRC_ERROR; break;
                case NEXUS_XptDataInterrupt_ePesLengthError: intId = BCHP_INT_ID_PES_ERROR; break;
                case NEXUS_XptDataInterrupt_ePesStartCodeError: intId = BCHP_INT_ID_PSC_ERROR; break;
#endif
                /* coverity[dead_error_begin] */
                default: continue;
                }
                rc = BINT_CreateCallback(&pTransport->message.xptDataInterrupt[xptDataInterrupt], g_pCoreHandles->bint, intId, NEXUS_Transport_P_MessageError_isr, NULL, xptDataInterrupt);
                if (rc) {rc = BERR_TRACE(rc);}
                rc = BINT_EnableCallback(pTransport->message.xptDataInterrupt[xptDataInterrupt]);
                if (rc) {rc = BERR_TRACE(rc);}
            }
        }
        else if (pTransport->message.xptDataInterrupt[xptDataInterrupt]) {
            BINT_DisableCallback(pTransport->message.xptDataInterrupt[xptDataInterrupt]);
            BINT_DestroyCallback(pTransport->message.xptDataInterrupt[xptDataInterrupt]);
            pTransport->message.xptDataInterrupt[xptDataInterrupt] = NULL;
        }
    }
#endif
}

bool NEXUS_TransportModule_P_IsMtsifEncrypted(void)
{
#if BXPT_NUM_MTSIF
    return BXPT_IsMtsifDecryptionEnabled(pTransport->xpt, 0); /* assume that either all MTSIF channels are encrypted or all clear */
#else
    return false;
#endif
}

NEXUS_Error NEXUS_TransportModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;
    unsigned i;

    BSTD_UNUSED(pSettings);

    if (enabled)
    {
        BXPT_StandbySettings standbySettings;
        BXPT_GetDefaultStandbySettings(&standbySettings);

#if NEXUS_NUM_PLAYPUMPS
        for(i=0; i<NEXUS_NUM_PLAYPUMPS; i++) {
            NEXUS_PlaypumpStatus status;
            if(pTransport->playpump[i].playpump) {
                NEXUS_Playpump_GetStatus(pTransport->playpump[i].playpump, &status);
                /* Force playpump to stop; if not already stopped */
                if(status.started) {
                    BDBG_WRN(("Forcing Playpump %d Stop for Standby", i));
                    NEXUS_Playpump_Stop(pTransport->playpump[i].playpump);
                }
            }
        }
#endif

        if(pSettings->mode == NEXUS_StandbyMode_ePassive || pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
            for(i=0; i<BXPT_NUM_RAVE_CONTEXTS; i++) {
                NEXUS_RecpumpStatus status;
                if(pTransport->recpump[i]) {
                   NEXUS_Recpump_GetStatus(pTransport->recpump[i], &status);
                    /* Force recpump to stop; if not already stopped */
                    if(status.started) {
                        BDBG_WRN(("Forcing Recpump %d Stop for Standby", i));
                        NEXUS_Recpump_Stop(pTransport->recpump[i]);
                    }
                }
            }

#if BXPT_NUM_REMULTIPLEXORS
            for(i=0; i<BXPT_NUM_REMULTIPLEXORS; i++) {
                if(g_remux[i] && g_remux[i]->started) {
                    BDBG_WRN(("Forcing Remux %d Stop for Standby", i));
                    NEXUS_Remux_Stop(g_remux[i]);
                }
            }
#endif

#ifndef NEXUS_USE_SW_FILTER
            for(i=0; i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES; i++) {
                if(pTransport->message.handle[i] && pTransport->message.handle[i]->started) {
                    BDBG_WRN(("Forcing Message %d Stop for Standby", i));
                    NEXUS_Message_Stop(pTransport->message.handle[i]);
                }
            }
#endif
            NEXUS_Vcxo_Uninit();
        }

#if NEXUS_NUM_PARSER_BANDS
        /* disable all parsers before BXPT_Standby and restore them later.
           this removes the requirement for apps to close/re-open pid channels */
        for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
            pTransport->standby.parserEnabled[i] = pTransport->parserBand[i] && pTransport->parserBand[i]->refcnt;
            if (pTransport->standby.parserEnabled[i]) {
                BXPT_SetParserEnable(pTransport->xpt, pTransport->parserBand[i]->hwIndex, false);
            }
        }
#endif

        standbySettings.S3Standby = (pSettings->mode == NEXUS_StandbyMode_eDeepSleep); /* S3Standby is only avail on 40nm platforms */


#if BXPT_HAS_WAKEUP_PKT_SUPPORT
        /* need to disable the standard L2 callback */
        if (pTransport->wakeup.settings->enabled) {
            BINT_DisableCallback(pTransport->wakeup.intPacketFoundCallback);
        }
        standbySettings.UseWakeupPacket = pSettings->wakeupSettings.transport;
#endif
        rc = BXPT_Standby(pTransport->xpt, &standbySettings);
    }
    else
    {
        if (!pTransport->moduleSettings.postInitCalledBySecurity) {
            rc = NEXUS_TransportModule_PostResume_priv();
        }
        else {
            /* security module will call */
            pTransport->standby.postResumePending = true;
            rc = NEXUS_SUCCESS;
        }
    }

    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_TransportModule_PostResume_priv(void)
{
    int rc;

    rc = BXPT_Resume(pTransport->xpt);
    if (rc) return BERR_TRACE(rc);

#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    if (pTransport->wakeup.settings->enabled) {
        rc = BINT_EnableCallback(pTransport->wakeup.intPacketFoundCallback);
    }
#endif

#if NEXUS_NUM_PARSER_BANDS
    {
        unsigned i;
        for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
            if (pTransport->standby.parserEnabled[i] && pTransport->parserBand[i]) {
                BXPT_SetParserEnable(pTransport->xpt, pTransport->parserBand[i]->hwIndex, true);
            }
        }
    }
#endif

    NEXUS_Vcxo_Init();

    pTransport->standby.postResumePending = false;

    return NEXUS_SUCCESS;
}

void NEXUS_Transport_GetXpt_priv( BXPT_Handle *pXpt )
{
    NEXUS_ASSERT_MODULE();
    *pXpt = pTransport->xpt;
}

void NEXUS_TransportWakeup_GetSettings( NEXUS_TransportWakeupSettings * pSettings)
{
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    *pSettings = *pTransport->wakeup.settings;
#else
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

NEXUS_Error NEXUS_TransportWakeup_SetSettings(const NEXUS_TransportWakeupSettings * pSettings)
{
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    BERR_Code rc;
    BXPT_Wakeup_Settings wakeupSettings;

    if (pSettings->enabled) {
        unsigned i,j;
        BXPT_Wakeup_PacketFilter *Filter = BKNI_Malloc(sizeof(BXPT_Wakeup_PacketFilter)*NEXUS_WAKEUP_PACKET_SIZE);
        if (!Filter) {return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}

        for (i=0; i<NEXUS_MAX_WAKEUP_PACKETS; i++) {
            for (j=0; j<pSettings->packetLength; j++) {
                Filter[j].CompareByte = pSettings->filter[i].packet[j].comparePattern;
                Filter[j].Mask = pSettings->filter[i].packet[j].compareMask;
                Filter[j].MaskType = pSettings->filter[i].packet[j].maskType;
            }
            rc = BXPT_Wakeup_SetPacketFilterBytes(pTransport->xpt, i, Filter);
            if (rc) {return BERR_TRACE(rc);}
        }

        BXPT_Wakeup_GetDefaults(&wakeupSettings);
        wakeupSettings.InputBand = pSettings->inputBand;
        wakeupSettings.PacketLength = pSettings->packetLength;
        wakeupSettings.ErrorInputIgnore = pSettings->errorIgnore;
        rc = BXPT_Wakeup_SetSettings(pTransport->xpt , &wakeupSettings);
        if (rc) {return BERR_TRACE(rc);}

        rc = BINT_EnableCallback(pTransport->wakeup.intPacketFoundCallback);
        if (rc) {return BERR_TRACE(rc);}
        BXPT_Wakeup_Armed(pTransport->xpt, true);
        BKNI_Free(Filter);
    } else {
        BXPT_Wakeup_Armed(pTransport->xpt, false);
        BINT_DisableCallback(pTransport->wakeup.intPacketFoundCallback);
    }

    *pTransport->wakeup.settings = *pSettings;
#else
    BSTD_UNUSED(pSettings);
#endif

    return 0;
}
