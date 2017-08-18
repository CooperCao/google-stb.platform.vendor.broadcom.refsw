/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* Description: IP Applib Implementation for TTS.
*
***************************************************************************/

/* PR47230: TTS pacing */
#include "b_os_lib.h"
#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_tts_throttle.h"
#include "nexus_timebase.h"

/**
* TTS Processing module of Playback IP Applib
*/

BDBG_MODULE(b_tts_throttle);

#define PWM_CENTER_FREQUENCY 0x400001

#define TTS_STEP_PERIOD                 1000 /* in msec */

/* slew ~70 Hz (~2.6 ppm) @ 100 Hz/sec,
 * in steps of 10Hz
 */
#define TTS_PWM_STEP_PERIOD             500         /* in msec */
#define TTS_PWM_STEP_SIZE               47          /* !!! 7Hz !!! (Hz/bit = 4386/0x7fff = 0.13385 Hz/bit, ie. 0.00494 ppm/bit) */
#define TTS_PWM_STEPS_PER_SLEW          8

#define TTS_MAX_PPM_VALUE               240       /* default to 60 ppm */
#define TTS_MIN_PPM_VALUE               (-TTS_MAX_PPM_VALUE)

#if PLAYBACK_IP_GCB_SUPPORT
#undef TTS_MAX_PPM_VALUE
#undef TTS_MIN_PPM_VALUE
#define TTS_MAX_PPM_VALUE               200
#define TTS_MIN_PPM_VALUE               (-TTS_MAX_PPM_VALUE)
#endif

/* pwm_value is 0.00494 ppm/bit, eg 200 ppm = 40486, and 1 ppm = approx 202 (actually 202.43)
 * to convert ppm values into pwm values, multiply by 202
 * to convert pwm values into ppm values, divide by 202
 */
#define TTS_PPM_TO_PWM_CONV_FACTOR      202
#define TTS_MAX_CLOCK_MISMATCH          100         /* default to 100 ppm - eg. server/encoder +/-50ppm, STB +/-50ppm */

#define TTS_BUF_DEPTH_AVERAGING_PERIOD  16      /* this must be multiplied by the TTS_STEP_PERIOD/1000 to get the actual period in seconds */
#define TTS_DELAYED_START_PERIOD        TTS_BUF_DEPTH_AVERAGING_PERIOD
#define TTS_TREND_AVERAGING_PERIOD      300
#define TTS_TREND_SAMPLE_PERIOD         120
#define TTS_HALT_TASK_TIMEOUT_MSEC      600

/* the TTS throttle structure */
typedef struct B_PlaybackIp_TtsThrottle {
    NEXUS_PlaypumpHandle playPump;

    B_PlaybackIp_TtsThrottle_Params params;
    B_PlaybackIp_TtsThrottle_Status status;
    B_PlaybackIp_TtsThrottle_State state;
    B_PlaybackIpBufferState bufferState;

    unsigned *bufSamples;
    unsigned bufHead;
    unsigned bufTail;
    int bufSamplesSum;
    uint32_t bufDepthInMsec;

    unsigned *avgSamples;
    unsigned avgHead;
    unsigned avgTail;
    int avgSamplesSum;
    unsigned *trendWindow;
    unsigned trendHead;
    unsigned trendTail;
    bool trendIsUp;
    signed trendDutyCycle;
    bool wideTrackMode;
    unsigned waitCount;

    unsigned periodCount;
    unsigned prevBufDepth;
    unsigned prevMaxBufDepth;
    unsigned prevMinBufDepth;

    long currentPwmValue;
    long maxPwmValue;
    long minPwmValue;
    unsigned int numPwmStepsPending;
    bool forcingPwmSlow;
    B_Time prevTime;

    B_ThreadHandle threadHandle;
    B_EventHandle threadHaltedEvent;
    bool suspendPacing;
} B_PlaybackIp_TtsThrottle;

static void B_PlaybackIp_TtsThrottle_P_SlowAdjust(B_PlaybackIp_TtsThrottleHandle ttsThrottle, unsigned long fifoDepth);
static void B_PlaybackIp_TtsThrottle_P_PwmSlew(B_PlaybackIp_TtsThrottleHandle ttsThrottle);
static void B_PlaybackIp_TtsThrottle_P_PwmSlow(B_PlaybackIp_TtsThrottleHandle ttsThrottle, bool forceSlow);
static void B_PlaybackIp_TtsThrottle_P_TrackModeCheck(B_PlaybackIp_TtsThrottleHandle ttsThrottle, unsigned *trendAvgSample);
static void B_PlaybackIp_TtsThrottle_P_CalcTrendDutyCycle(B_PlaybackIp_TtsThrottleHandle ttsThrottle, unsigned *trendAvgSample);
static void B_PlaybackIp_TtsThrottle_P_BufViolationCheck(B_PlaybackIp_TtsThrottleHandle ttsThrottle);
static void B_PlaybackIp_TtsThrottle_P_BufViolationCallback(void *context, bool isMax);
static B_PlaybackIpError B_PlaybackIp_TtsThrottle_SetCenterFreq(B_PlaybackIp_TtsThrottleHandle ttsThrottle);
static B_PlaybackIpError B_PlaybackIp_TtsThrottle_ManageBuffer(B_PlaybackIp_TtsThrottleHandle ttsThrottle);
static void B_PlaybackIp_TtsThrottle_Step(B_PlaybackIp_TtsThrottleHandle ttsThrottle);

/**
Summary:
**/
B_PlaybackIp_TtsThrottleHandle B_PlaybackIp_TtsThrottle_Open(
    void
    )
{
    B_PlaybackIp_TtsThrottleHandle  ttsThrottle;

    ttsThrottle = BKNI_Malloc(sizeof(*ttsThrottle));
    if (!ttsThrottle) goto error;
    BKNI_Memset(ttsThrottle, 0, sizeof(*ttsThrottle));

    ttsThrottle->bufSamples = BKNI_Malloc(TTS_BUF_DEPTH_AVERAGING_PERIOD * sizeof(unsigned));
    if (!ttsThrottle->bufSamples) {
        BKNI_Free(ttsThrottle);
        goto error;
    }

    ttsThrottle->avgSamples = BKNI_Malloc(TTS_TREND_AVERAGING_PERIOD * sizeof(unsigned));
    if (!ttsThrottle->avgSamples) {
        BKNI_Free(ttsThrottle->bufSamples);
        BKNI_Free(ttsThrottle);
        goto error;
    }
    ttsThrottle->trendWindow = BKNI_Malloc(TTS_TREND_SAMPLE_PERIOD * sizeof(unsigned));
    if (!ttsThrottle->trendWindow) {
        BKNI_Free(ttsThrottle->bufSamples);
        BKNI_Free(ttsThrottle->avgSamples);
        BKNI_Free(ttsThrottle);
        goto error;
    }

    ttsThrottle->threadHaltedEvent = B_Event_Create(NULL);
    if (!ttsThrottle->threadHaltedEvent) {
        BKNI_Free(ttsThrottle->bufSamples);
        BKNI_Free(ttsThrottle->avgSamples);
        BKNI_Free(ttsThrottle->trendWindow);
        BKNI_Free(ttsThrottle);
        goto error;
    }

    ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Opened;
    ttsThrottle->bufferState = B_PlaybackIpBufferState_eInit;

    ttsThrottle->currentPwmValue = 0; /* start at the crystal nominal center frequency */
    ttsThrottle->maxPwmValue = TTS_MAX_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR;
    ttsThrottle->minPwmValue = TTS_MIN_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR;
    ttsThrottle->status.pacingClockAdjustment = 0;

    return ttsThrottle;
error:
    BDBG_ERR(("B_PlaybackIp_TtsThrottle_Open error"));

    return NULL;
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_Close(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    B_PlaybackIp_TtsThrottle_Stop(ttsThrottle);

    if (ttsThrottle->threadHaltedEvent != NULL) B_Event_Destroy(ttsThrottle->threadHaltedEvent);

    BKNI_Free(ttsThrottle->bufSamples);
    BKNI_Free(ttsThrottle->avgSamples);
    BKNI_Free(ttsThrottle->trendWindow);
    BKNI_Free(ttsThrottle);
}

/**
Summary:
**/
/* TODO: change to getDefaultSettings??? */
void B_PlaybackIp_TtsThrottle_ParamsInit(
    B_PlaybackIp_TtsThrottle_Params *params
    )
{
    params->playPump = NULL;
    params->initBufDepth = 500000;
    params->minBufDepth = 0;
    params->maxBufDepth = 0xFFFFFFFF;
    params->bufDepthViolationCallback = B_PlaybackIp_TtsThrottle_P_BufViolationCallback;
    params->bufDepthViolationCbContext = NULL;
    params->maxClockMismatch = TTS_MAX_CLOCK_MISMATCH;
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_GetSettings(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    B_PlaybackIp_TtsThrottle_Params *params
    )
{
    BKNI_Memcpy(params, &ttsThrottle->params, sizeof(*params));
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_SetSettings(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    B_PlaybackIp_TtsThrottle_Params *params
    )
{
    BKNI_Memcpy(&ttsThrottle->params, params, sizeof(*params));

    ttsThrottle->playPump = ttsThrottle->params.playPump;
}

void B_PlaybackIp_TtsThrottle_Task(
    void *data
    )
{
    B_PlaybackIp_TtsThrottleHandle ttsThrottle = (B_PlaybackIp_TtsThrottleHandle)data;

    while(ttsThrottle->state != B_PlaybackIp_TtsThrottle_State_Stopping) {
        if (ttsThrottle->suspendPacing == true) {
            B_Thread_Sleep(TTS_PWM_STEP_PERIOD);
            continue;
        }
        B_PlaybackIp_TtsThrottle_ManageBuffer(ttsThrottle);

        if(ttsThrottle->bufferState == B_PlaybackIpBufferState_ePlaying) {
            B_PlaybackIp_TtsThrottle_Step(ttsThrottle);
            B_Thread_Sleep(TTS_PWM_STEP_PERIOD);
        }
    }

    B_Event_Set(ttsThrottle->threadHaltedEvent);
}

/**
Summary:
**/
B_PlaybackIpError B_PlaybackIp_TtsThrottle_Start(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    B_PlaybackIpError rc;
    B_ThreadSettings threadSettings;

    BDBG_ASSERT(ttsThrottle->playPump);

    ttsThrottle->playPump = ttsThrottle->params.playPump;

    ttsThrottle->status.pacingClockAdjustment = 0;

    ttsThrottle->forcingPwmSlow = true;
    ttsThrottle->numPwmStepsPending = 0;
    ttsThrottle->currentPwmValue = 0; /* start at the crystal nominal center frequency */

    B_PlaybackIp_TtsThrottle_SetCenterFreq(ttsThrottle);

    BDBG_WRN(("initBufDepth: %d, minBufDepth: %d, maxBufDepth: %d, maxClockMismatch: %d",
              ttsThrottle->params.initBufDepth,
              ttsThrottle->params.minBufDepth,
              ttsThrottle->params.maxBufDepth,
              ttsThrottle->params.maxClockMismatch));

    ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Paused;
    ttsThrottle->bufferState = B_PlaybackIpBufferState_eInit;
    ttsThrottle->status.bufDepth = 0;
    ttsThrottle->status.pacingErrorCount = 0;
    ttsThrottle->status.pacingClockRailed = false;

    ttsThrottle->periodCount = 0;
    ttsThrottle->bufHead = 0;
    ttsThrottle->bufTail = 1;
    ttsThrottle->bufSamplesSum = 0;

    ttsThrottle->prevMaxBufDepth = ttsThrottle->prevMinBufDepth = 0;

    ttsThrottle->avgHead = 0;
    ttsThrottle->avgTail = 1;
    ttsThrottle->avgSamplesSum = 0;
    ttsThrottle->trendHead = 0;
    ttsThrottle->trendTail = 1;
    ttsThrottle->trendIsUp = false;
    ttsThrottle->trendDutyCycle = 0;
    ttsThrottle->wideTrackMode = false;
    ttsThrottle->waitCount = 0;

    rc = NEXUS_Playpump_SetPause(ttsThrottle->playPump, true);
    if(rc!=B_ERROR_SUCCESS) { goto error; }
    rc = NEXUS_Playpump_SuspendPacing(ttsThrottle->playPump, true);
    if(rc!=B_ERROR_SUCCESS) { goto error; }

    B_Time_Get(&ttsThrottle->prevTime);

    B_Thread_GetDefaultSettings(&threadSettings);
    ttsThrottle->threadHandle = B_Thread_Create("ttsThrottle",
                                                (B_ThreadFunc)B_PlaybackIp_TtsThrottle_Task,
                                                (void *)ttsThrottle,
                                                &threadSettings);
    if (NULL == ttsThrottle->threadHandle) { goto error; }

    return B_ERROR_SUCCESS;

error:
    BDBG_ERR(("B_PlaybackIp_TtsThrottle_Start error"));
    return rc;
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_Stop(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    B_Error rc;

    switch(ttsThrottle->state) {
    case B_PlaybackIp_TtsThrottle_State_Opened:
        /* do nothing */
        break;
    case B_PlaybackIp_TtsThrottle_State_Running:
/* TODO: add this back when it's supported on the main branch */
#if 0
        NEXUS_Timebase_StopTwoPcrErrorMonitor(NEXUS_Timebase_e0);
#endif
        /* fall through */
    case B_PlaybackIp_TtsThrottle_State_Paused:
        ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Stopping;
        rc = B_Event_Wait(ttsThrottle->threadHaltedEvent, TTS_HALT_TASK_TIMEOUT_MSEC);
        if (rc == B_ERROR_TIMEOUT) {
            BDBG_ERR(("threadHaltedEvent was timed out"));
        } else if (rc != B_ERROR_SUCCESS) {
            BDBG_ERR(("threadHaltedEvent gave error"));
        }

        if(ttsThrottle->threadHandle) {
            B_Thread_Destroy(ttsThrottle->threadHandle);
            ttsThrottle->threadHandle = NULL;
        }
        break;
    case B_PlaybackIp_TtsThrottle_State_Stopping:
        /* do nothing */
        break;
    default:
        /* error */
        break;
    }

    ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Opened;

/*  ip_start_luma_msg = false;
*/
}

/**
Summary:
**/
B_PlaybackIpError B_PlaybackIp_TtsThrottle_GetStatus(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    B_PlaybackIp_TtsThrottle_Status *status
    )
{
    BKNI_Memcpy(status, &ttsThrottle->status, sizeof(*status));
    status->state = ttsThrottle->state;

    return B_ERROR_SUCCESS;
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_Pause(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    bool pause
    )
{
    switch(ttsThrottle->state) {
    case B_PlaybackIp_TtsThrottle_State_Opened:
        /* does nothing */
        break;
    case B_PlaybackIp_TtsThrottle_State_Paused:
        if(!pause) {
            BDBG_WRN(("state: paused -> running"));
/* TODO: add this back when it's supported on the main branch */
#if 0
            NEXUS_Timebase_StartTwoPcrErrorMonitor(NEXUS_Timebase_e0);
#endif
            ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Running;
        }
        break;
    case B_PlaybackIp_TtsThrottle_State_Running:
        if(pause) {
            BDBG_WRN(("state: running -> paused"));
/* TODO: add this back when it's supported on the main branch */
#if 0
            NEXUS_Timebase_StopTwoPcrErrorMonitor(NEXUS_Timebase_e0);
#endif
            ttsThrottle->state = B_PlaybackIp_TtsThrottle_State_Paused;
        }
        break;
    case B_PlaybackIp_TtsThrottle_State_Stopping:
        /* does nothing */
        break;
    default:
        /* error */
        break;
    }

    if (NEXUS_Playpump_SetPause(ttsThrottle->playPump, pause) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to %s", BSTD_FUNCTION, pause ? "set pause":"unset pause"));
    }

}

/**
Summary:
**/
B_PlaybackIpError B_PlaybackIp_TtsThrottle_SuspendPacing(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    bool suspend
    )
{
    if(suspend) {
        BDBG_WRN(("suspending pacing function"));
        ttsThrottle->suspendPacing = true;
    }
    else {
        BDBG_WRN(("unsuspending pacing function"));
        ttsThrottle->suspendPacing = false;
    }
    BDBG_ASSERT(ttsThrottle->playPump);
    return NEXUS_Playpump_SuspendPacing(ttsThrottle->playPump, suspend);
}

/**
Summary:
**/
void B_PlaybackIp_TtsThrottle_Step(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    NEXUS_PlaypumpStatus pumpStatus;
    NEXUS_Error rc;
    B_Time cur_time;
    int diff_time;
    size_t curBufDepth;

    if (ttsThrottle->params.bufDepthInMsec) {
        curBufDepth = ttsThrottle->bufDepthInMsec;
    }
    else {
        curBufDepth = pumpStatus.fifoDepth;
    }

    B_Time_Get(&cur_time);
    diff_time = B_Time_Diff(&cur_time, &ttsThrottle->prevTime);

    if(diff_time > 499) {
            ttsThrottle->prevTime = cur_time;

            if(ttsThrottle->state == B_PlaybackIp_TtsThrottle_State_Running) {
                BDBG_ASSERT(ttsThrottle->playPump);
                rc = NEXUS_Playpump_GetStatus(ttsThrottle->playPump, &pumpStatus);
                if (rc) BDBG_WRN(("NEXUS_Playpump_GetStatus returned error: %d", rc));

                B_PlaybackIp_TtsThrottle_P_SlowAdjust(ttsThrottle, curBufDepth);
                if(ttsThrottle->status.pacingErrorCount != pumpStatus.pacingTsRangeError) {
                    ttsThrottle->status.pacingErrorCount = pumpStatus.pacingTsRangeError;
                    /* make it ding (for debug) */
                    /* (("pacing error %c", 7)); */
                    BDBG_WRN(("pacing error"));
                }

                /* supress early LumaBuffer messages */
#if 0
                if(ttsThrottle->periodCount == TTS_DELAYED_START_PERIOD)
                    ip_start_luma_msg = true;
#endif
        }
    }
    else {
        BDBG_WRN(("TTS throttle step scheduling out-of-range"));
    }
}

/**
Summary:
 A single pass comprises taking of any necessary action with
 respect to altering the TTS pacing clock. ie. if thresholds are exceeded the pacing clock is
 adjusted accordingly.
**/
static void B_PlaybackIp_TtsThrottle_P_SlowAdjust(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    unsigned long fifoDepth
    )
{
    signed init_buf = ttsThrottle->params.initBufDepth;
    unsigned avgBufDepth;
    unsigned trendAvgSample;

    ttsThrottle->status.bufDepth = fifoDepth;

    if(ttsThrottle->periodCount < TTS_TREND_AVERAGING_PERIOD) ttsThrottle->periodCount++;

    /* average the buffer depth values */
    ttsThrottle->bufSamples[ttsThrottle->bufHead] = fifoDepth;
    ttsThrottle->bufSamplesSum += ttsThrottle->bufSamples[ttsThrottle->bufHead];
    if(ttsThrottle->periodCount <= TTS_BUF_DEPTH_AVERAGING_PERIOD) {
        avgBufDepth = ttsThrottle->bufSamplesSum/(long)(ttsThrottle->periodCount);
    }
    else {
        ttsThrottle->bufSamplesSum -= ttsThrottle->bufSamples[ttsThrottle->bufTail];
        avgBufDepth = ttsThrottle->bufSamplesSum/(long)(TTS_BUF_DEPTH_AVERAGING_PERIOD);
    }
    ttsThrottle->bufHead++;
    if(ttsThrottle->bufHead>=TTS_BUF_DEPTH_AVERAGING_PERIOD) ttsThrottle->bufHead=0;
    ttsThrottle->bufTail++;
    if(ttsThrottle->bufTail>=TTS_BUF_DEPTH_AVERAGING_PERIOD) ttsThrottle->bufTail=0;

    ttsThrottle->status.avgBufDepth = avgBufDepth;
    B_PlaybackIp_TtsThrottle_P_TrackModeCheck(ttsThrottle, &trendAvgSample);

    if(ttsThrottle->periodCount == TTS_DELAYED_START_PERIOD-1) {
        init_buf = ttsThrottle->params.initBufDepth = avgBufDepth;
        ttsThrottle->prevMaxBufDepth = ttsThrottle->prevMinBufDepth = avgBufDepth;
    }

    if(ttsThrottle->periodCount >= TTS_DELAYED_START_PERIOD) {
        /* adjust the pacing clock */
        if((signed)avgBufDepth > init_buf) {
            if(avgBufDepth > ttsThrottle->prevMaxBufDepth) {
                ttsThrottle->prevMaxBufDepth = avgBufDepth;
                B_PlaybackIp_TtsThrottle_P_PwmSlow(ttsThrottle, false);
            }
        }
        else {
            if(avgBufDepth < ttsThrottle->prevMinBufDepth) {
                ttsThrottle->prevMinBufDepth = avgBufDepth;
                B_PlaybackIp_TtsThrottle_P_PwmSlow(ttsThrottle, true);
            }
        }
    }

    /* decay the max/min buffer depths used for adjusting the pacing clock */
    ttsThrottle->prevMinBufDepth = init_buf-(((init_buf-ttsThrottle->prevMinBufDepth)*999/1000));
    ttsThrottle->prevMaxBufDepth = init_buf+(((ttsThrottle->prevMaxBufDepth-init_buf)*999/1000));

    B_PlaybackIp_TtsThrottle_P_BufViolationCheck(ttsThrottle);

    B_PlaybackIp_TtsThrottle_P_PwmSlew(ttsThrottle);

/* re-enable the ttsThrottle log (before release) */
#if 1
    {
        NEXUS_PlaypumpStatus pumpStatus;
        NEXUS_Error rc;

        BDBG_ASSERT(ttsThrottle->playPump);
        rc = NEXUS_Playpump_GetStatus(ttsThrottle->playPump, &pumpStatus);
        if (rc) BDBG_WRN(("NEXUS_Playpump_GetStatus returned error: %d", rc));

        BDBG_WRN(("depth: avg, min, max, bytes, clk_adj %u, %u, %u, %zu, %d clk min: %ld, max: %ld, pcr_err: %d, trend_up: %d, duty: %d, mode: %d",
              ttsThrottle->status.avgBufDepth,
              ttsThrottle->prevMinBufDepth,
              ttsThrottle->prevMaxBufDepth,
              pumpStatus.fifoDepth,
              ttsThrottle->status.pacingClockAdjustment,
              ttsThrottle->minPwmValue/TTS_PPM_TO_PWM_CONV_FACTOR,
              ttsThrottle->maxPwmValue/TTS_PPM_TO_PWM_CONV_FACTOR,
/*                pump_status.ndpcr_two_err, */
              pumpStatus.started,   /* TODO: temp until dpcrTwoError is added */
              ttsThrottle->trendIsUp,
              ttsThrottle->trendDutyCycle,
              ttsThrottle->wideTrackMode));
    }
    BDBG_WRN(("tts throttle %p: bufDepth: %u, clock_adj: %d, violation_count: %u, err_count: %u",
              (void *)ttsThrottle, ttsThrottle->status.avgBufDepth,
              ttsThrottle->status.pacingClockAdjustment,
              ttsThrottle->status.bufViolationCount,
              ttsThrottle->status.pacingErrorCount));
#else
#endif
}

/**
Summary:
**/
static void B_PlaybackIp_TtsThrottle_P_PwmSlow(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    bool forceSlow
    )
{
    /* Only trigger a new slew if there is not one currently in progress.
     * This provides some hysteresis and keeps things simple.
     */
    if(ttsThrottle->numPwmStepsPending == 0) {
        if(forceSlow) {
            if(ttsThrottle->currentPwmValue > ttsThrottle->minPwmValue) {
                ttsThrottle->numPwmStepsPending = TTS_PWM_STEPS_PER_SLEW;
                ttsThrottle->forcingPwmSlow = true;
            }
            else {
            }
        }
        else {
            if(ttsThrottle->currentPwmValue < ttsThrottle->maxPwmValue) {
                ttsThrottle->numPwmStepsPending = TTS_PWM_STEPS_PER_SLEW;
                ttsThrottle->forcingPwmSlow = false;
            }
            else {
            }
        }
    }
    else {
        /* BDBG_WRN(("Slew of clock abandoned, ttsThrottle->numPwmStepsPending=%d!", ttsThrottle->numPwmStepsPending)); */
    }

    return;
}

/*
Summary:
    Control the slew rate of the PWM value
Description:
     This function is called from the timer context.
*/
static void B_PlaybackIp_TtsThrottle_P_PwmSlew(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    if(ttsThrottle->numPwmStepsPending > 0) {
        ttsThrottle->numPwmStepsPending--;
        if(ttsThrottle->forcingPwmSlow) {
            ttsThrottle->currentPwmValue -= TTS_PWM_STEP_SIZE;
            if(ttsThrottle->currentPwmValue <= ttsThrottle->minPwmValue) {
                ttsThrottle->currentPwmValue = ttsThrottle->minPwmValue;
                ttsThrottle->numPwmStepsPending = 0;
                ttsThrottle->status.pacingClockRailed = true;
            }
            else {
                ttsThrottle->status.pacingClockRailed = false;
            }
        }
        else {
            ttsThrottle->currentPwmValue += TTS_PWM_STEP_SIZE;
            if(ttsThrottle->currentPwmValue >= ttsThrottle->maxPwmValue) {
                ttsThrottle->currentPwmValue = ttsThrottle->maxPwmValue;
                ttsThrottle->numPwmStepsPending = 0;
                ttsThrottle->status.pacingClockRailed = true;
            }
            else {
                ttsThrottle->status.pacingClockRailed = false;
            }
        }
        B_PlaybackIp_TtsThrottle_SetCenterFreq(ttsThrottle);

        ttsThrottle->status.pacingClockAdjustment = ttsThrottle->currentPwmValue/TTS_PPM_TO_PWM_CONV_FACTOR;
    }
    return;
}

/**
Summary:
Check for transition to/from wide-track mode
**/
static void B_PlaybackIp_TtsThrottle_P_TrackModeCheck(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    unsigned *trendAvgSample
    )
{
    B_PlaybackIp_TtsThrottle_P_CalcTrendDutyCycle(ttsThrottle, trendAvgSample);

    if (ttsThrottle->wideTrackMode) {
        if(ttsThrottle->currentPwmValue > TTS_MIN_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR &&
           ttsThrottle->currentPwmValue < TTS_MAX_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR) {
            /* we're back within the normal range, exit wide-track mode */
            ttsThrottle->wideTrackMode = false;
            ttsThrottle->maxPwmValue = TTS_MAX_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR;
            ttsThrottle->minPwmValue = TTS_MIN_PPM_VALUE*TTS_PPM_TO_PWM_CONV_FACTOR;
            BDBG_WRN(("Exiting wide track range mode, new range: +/- %d ppm", TTS_MAX_PPM_VALUE));
        }
    }
    else {
        if(ttsThrottle->waitCount==0) {
            /* not currently waiting to check */
            if(ttsThrottle->status.pacingClockRailed) {
                ttsThrottle->waitCount = 1;     /* start waiting period */
                ttsThrottle->trendDutyCycle = 0; /* reset duty cycle */
            }
        }
        else {
            /* currently waiting to check if it is time to enter wide-track mode */
            ttsThrottle->waitCount++;
            if(ttsThrottle->waitCount > TTS_TREND_SAMPLE_PERIOD*2) {
                /* waiting period expired, check if we need to transition to wide-track range mode */
                ttsThrottle->waitCount = 0;
                BDBG_MSG(("Checking wide-track mode..."));

                if(ttsThrottle->currentPwmValue==ttsThrottle->minPwmValue && ttsThrottle->trendDutyCycle<0) {
                    ttsThrottle->wideTrackMode = true;
                    ttsThrottle->maxPwmValue = ttsThrottle->params.maxClockMismatch*TTS_PPM_TO_PWM_CONV_FACTOR;
                    ttsThrottle->minPwmValue = -ttsThrottle->params.maxClockMismatch*TTS_PPM_TO_PWM_CONV_FACTOR;
                    BDBG_WRN(("Entering wide track range mode, new range: +/- %d ppm", ttsThrottle->params.maxClockMismatch));
                }
                else if(ttsThrottle->currentPwmValue==ttsThrottle->maxPwmValue && ttsThrottle->trendDutyCycle>0) {
                    ttsThrottle->wideTrackMode = true;
                    ttsThrottle->maxPwmValue = ttsThrottle->params.maxClockMismatch*TTS_PPM_TO_PWM_CONV_FACTOR;
                    ttsThrottle->minPwmValue = -ttsThrottle->params.maxClockMismatch*TTS_PPM_TO_PWM_CONV_FACTOR;
                    BDBG_WRN(("Entering wide track range mode, new range: +/- %d ppm", ttsThrottle->params.maxClockMismatch));
                }
            }
        }
    }
}

/**
Summary:
**/
static void B_PlaybackIp_TtsThrottle_P_CalcTrendDutyCycle(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    unsigned *trendAvgSample
    )
{
    /* trend avg values */
    ttsThrottle->avgSamples[ttsThrottle->avgHead] = ttsThrottle->status.avgBufDepth;
    ttsThrottle->avgSamplesSum += ttsThrottle->avgSamples[ttsThrottle->avgHead];
    if(ttsThrottle->periodCount < TTS_TREND_AVERAGING_PERIOD) {
        *trendAvgSample = ttsThrottle->avgSamplesSum/(long)(ttsThrottle->periodCount);
    }
    else {
        ttsThrottle->avgSamplesSum -= ttsThrottle->avgSamples[ttsThrottle->avgTail];
        *trendAvgSample = ttsThrottle->avgSamplesSum/(long)(TTS_TREND_AVERAGING_PERIOD-1);
    }
    ttsThrottle->avgHead++;
    if(ttsThrottle->avgHead>=TTS_TREND_AVERAGING_PERIOD) ttsThrottle->avgHead=0;
    ttsThrottle->avgTail++;
    if(ttsThrottle->avgTail>=TTS_TREND_AVERAGING_PERIOD) ttsThrottle->avgTail=0;

    /* trend sampling values */
    ttsThrottle->trendWindow[ttsThrottle->trendHead] = *trendAvgSample;

    if(ttsThrottle->periodCount < TTS_TREND_SAMPLE_PERIOD) {
        if(ttsThrottle->trendWindow[ttsThrottle->trendHead] > ttsThrottle->trendWindow[0])
            ttsThrottle->trendIsUp = true;
        else
            ttsThrottle->trendIsUp = false;
    }
    else {
        if(ttsThrottle->trendWindow[ttsThrottle->trendHead] > ttsThrottle->trendWindow[ttsThrottle->trendTail]) {
            ttsThrottle->trendIsUp = true;
            ttsThrottle->trendDutyCycle++;
            if(ttsThrottle->trendDutyCycle > TTS_TREND_SAMPLE_PERIOD)
                ttsThrottle->trendDutyCycle = TTS_TREND_SAMPLE_PERIOD;
        }
        else {
            ttsThrottle->trendIsUp = false;
            ttsThrottle->trendDutyCycle--;
            if(ttsThrottle->trendDutyCycle < -TTS_TREND_SAMPLE_PERIOD)
                ttsThrottle->trendDutyCycle = -TTS_TREND_SAMPLE_PERIOD;
        }
    }

    ttsThrottle->trendHead++;
    if(ttsThrottle->trendHead>=TTS_TREND_SAMPLE_PERIOD) ttsThrottle->trendHead=0;
    ttsThrottle->trendTail++;
    if(ttsThrottle->trendTail>=TTS_TREND_SAMPLE_PERIOD) ttsThrottle->trendTail=0;
}

/**
Summary:
Check max/min buffer thresholds
**/
static void B_PlaybackIp_TtsThrottle_P_BufViolationCheck(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    if(ttsThrottle->params.bufDepthViolationCallback != NULL) {
        if(ttsThrottle->status.avgBufDepth > ttsThrottle->params.maxBufDepth) {
            ttsThrottle->status.bufViolationCount++;
            ttsThrottle->params.bufDepthViolationCallback(ttsThrottle->params.bufDepthViolationCbContext, true);
        }
        else if(ttsThrottle->status.avgBufDepth < ttsThrottle->params.minBufDepth) {
            ttsThrottle->status.bufViolationCount++;
            ttsThrottle->params.bufDepthViolationCallback(ttsThrottle->params.bufDepthViolationCbContext, false);
        }
    }
}

/**
Summary:
Default function callback that will be called if max/min buffer depth is violated
**/
static void B_PlaybackIp_TtsThrottle_P_BufViolationCallback(
    void *context,
    bool isMax
    )
{
    BSTD_UNUSED(context);

    if(isMax)
        BDBG_WRN(("maxBufDepth threshold was violated"));
    else
        BDBG_WRN(("minBufDepth threshold was violated"));
}

/**
Summary:
**/
B_PlaybackIpError B_PlaybackIp_TtsThrottle_SetCenterFreq(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    NEXUS_TimebaseSettings timebaseSettings;
    uint32_t center_freq;
    NEXUS_Timebase timebase;

    if(ttsThrottle->params.timebase == NEXUS_Timebase_eInvalid) { return B_ERROR_INVALID_PARAMETER; }

    timebase = ttsThrottle->params.timebase;

    /* center_freq on 7445 is 0.238 ppm/bit, eg 100 ppm = 420
     * ie. to convert pwm_value to center_freq, divide pwm_value by 20243/420 = 48
     */

    center_freq = (PWM_CENTER_FREQUENCY+(ttsThrottle->currentPwmValue/48));
    BDBG_WRN(("adjust center_freq: %d,%u,%u", ((int32_t)center_freq - (int32_t)0x400000) >> 2, center_freq, ttsThrottle->status.avgBufDepth ));

    NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
    timebaseSettings.sourceSettings.freeRun.centerFrequency = center_freq;

    return NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
}

static B_PlaybackIpError B_PlaybackIp_TtsThrottle_ManageBuffer(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle
    )
{
    NEXUS_PlaypumpStatus pumpStatus;
    NEXUS_Error rc;
    size_t initBufferDepth = ttsThrottle->params.initBufDepth;
    size_t curBufDepth;

    BDBG_ASSERT(ttsThrottle->playPump);
    rc = NEXUS_Playpump_GetStatus(ttsThrottle->playPump, &pumpStatus);
    if (rc) BDBG_WRN(("NEXUS_Playpump_GetStatus returned error: %d", rc));

    if (ttsThrottle->params.bufDepthInMsec) {
        curBufDepth = ttsThrottle->bufDepthInMsec;
    }
    else {
        curBufDepth = pumpStatus.fifoDepth;
    }

    switch(ttsThrottle->bufferState) {
    case B_PlaybackIpBufferState_eInit:
        if(curBufDepth >= initBufferDepth) {
            BDBG_WRN(("tts throttle %p: Playpump buffer fullness established %zu (%zu), going to playing state", (void *)ttsThrottle, pumpStatus.fifoDepth, initBufferDepth));
            B_PlaybackIp_TtsThrottle_Pause(ttsThrottle, false);
            BKNI_Sleep(1); /* this prevents a pacing error at startup */
            NEXUS_Playpump_SuspendPacing(ttsThrottle->playPump, false);
            ttsThrottle->bufferState = B_PlaybackIpBufferState_ePlaying;
        }
        else {
            B_PlaybackIp_TtsThrottle_Pause(ttsThrottle, true);
        }
        break;
    case B_PlaybackIpBufferState_ePreCharging:
        if(curBufDepth >= initBufferDepth) {
            BDBG_WRN(("tts throttle %p: Playpump buffer fullness restored, going to playing state", (void *)ttsThrottle));
            B_PlaybackIp_TtsThrottle_Pause(ttsThrottle, false);
            ttsThrottle->bufferState = B_PlaybackIpBufferState_ePlaying;
        }
        else {
            B_PlaybackIp_TtsThrottle_Pause(ttsThrottle, true);
        }
        break;
    case B_PlaybackIpBufferState_ePlaying:
        if( (pumpStatus.fifoDepth >= pumpStatus.fifoSize-(IP_MAX_PKT_SIZE*PKTS_PER_CHUNK)) && !ttsThrottle->params.scatterGatherPlaypump) {
            BDBG_WRN(("tts throttle: %p, Approaching playback buffer overflow, flushing buffers!", (void *)ttsThrottle));
            NEXUS_Playpump_Flush(ttsThrottle->playPump);
            ttsThrottle->bufferState = B_PlaybackIpBufferState_ePreCharging;
        }
        else if(pumpStatus.fifoDepth==0) {
            BDBG_WRN(("tts throttle: %p, Playpump buffer underflow, avgBufDepthInMsec=%zu", (void *)ttsThrottle, curBufDepth));
            NEXUS_Playpump_SuspendPacing(ttsThrottle->playPump, true);
            NEXUS_Playpump_SuspendPacing(ttsThrottle->playPump, false);
            ttsThrottle->bufferState = B_PlaybackIpBufferState_ePreCharging;
            ttsThrottle->bufDepthInMsec = 0;
        }
        break;
    case B_PlaybackIpBufferState_ePostCharging:
    case B_PlaybackIpBufferState_eOverflowing:
    case B_PlaybackIpBufferState_eUnderflowing:
        /* not used - fall through */
    default:
        break;
    }

    return B_ERROR_SUCCESS;
}
void
B_PlaybackIp_TtsThrottle_UpdateBufferDepth(
    B_PlaybackIp_TtsThrottleHandle ttsThrottle,
    uint32_t bufDepthInMsec
    )
{
    ttsThrottle->bufDepthInMsec = bufDepthInMsec;
}
