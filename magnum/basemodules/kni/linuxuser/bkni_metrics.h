/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#ifndef BKNI_METRICS_H
#define BKNI_METRICS_H

void BKNI_P_Stats_Print(void);
/***************************************************************************
Summary:
Data structure used by BKNI_GetMetrics to return kernel interface metrics.
****************************************************************************/
typedef struct {
  uint32_t totalDelays;				/* Total number of times BKNI_Delay was called. */
  uint32_t totalDelayTime;			/* Acculated microsec's passed into BKNI_Delay. */
  uint32_t maxDelayTime;			/* Largest microsec value passed into BKNI_Delay. */

  uint32_t totalSleeps;				/* Total number of times BKNI_Sleep was called. */
  uint32_t totalSleepTime;			/* Acculated millisec's passed into BKNI_Sleep. */
  uint32_t maxSleepTime;			/* Largest millisec value passed into BKNI_Sleep. */

  uint32_t totalMutexSections;		/* Total number of times BKNI_AcquireMutex was called. */
  uint32_t totalMutexSectionTime;	/* Total time in milliseconds spent between matching
  										BKNI_AcquireMutex and BKNI_ReleaseMutex calls. */
  uint32_t maxMutexSectionTime;		/* Largest time in milliseconds spent between matching
  										BKNI_AcquireMutex and BKNI_ReleaseMutex calls. */

  uint32_t totalCriticalSections;	/* Total number of times BKNI_AcquireMutex was called. */
  uint32_t totalCriticalSectionTime;/* Total time in milliseconds spent between
  										BKNI_EnterCriticalSection and BKNI_LeaveCriticalSection calls. */
  uint32_t maxCriticalSectionTime;	/* Largest time in milliseconds spent between
  										BKNI_EnterCriticalSection and BKNI_LeaveCriticalSection calls. */

  uint32_t totalMemoryAllocated;
} BKNI_Metrics;


/***************************************************************************
Summary:
	Retrieve metrics from kernel interface regarding delay, sleep and mutex usage. The
	metrics are accumulated since the last BKNI_Init() or BKNI_ResetMetrics() call.
****************************************************************************/
void BKNI_GetMetrics(BKNI_Metrics *metrics);


/***************************************************************************
Summary:
	Reset all metrics to initial values before kernel interface was used.
****************************************************************************/
void BKNI_ResetMetrics(void);


/***************************************************************************
Summary:
	Structure to control debug output of kernel interface.
****************************************************************************/
typedef struct {
  bool printDelays;				/* Print every time BKNI_Delay is called. */
  bool printSleeps;				/* Print every time BKNI_Sleep is called. */
  bool printMutexSections;		/* Print every time BKNI_AcquireMutex and BKNI_ReleaseMutex are called. */
  bool printCriticalSections;	/* Print every time BKNI_EnterCriticalSection and
  									BKNI_LeaveCriticalSection are called. */
  bool printEvents;				/* Print every time BKNI_SetEvent and 
  									BKNI_WaitForEvent are called. */
} BKNI_MetricsLoggingState;


/***************************************************************************
Summary:
	Change the debug output of the kernel interface.
****************************************************************************/
void BKNI_GetMetricsLoggingState(BKNI_MetricsLoggingState *logging);


/***************************************************************************
Summary:
	Get the debug output state of the kernel interface.
****************************************************************************/
void BKNI_SetMetricsLoggingState(const BKNI_MetricsLoggingState *logging);

#endif /* BKNI_METRICS_H */
