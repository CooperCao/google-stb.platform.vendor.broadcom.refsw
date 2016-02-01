/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BKNI_METRICS_H
#define BKNI_METRICS_H

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
