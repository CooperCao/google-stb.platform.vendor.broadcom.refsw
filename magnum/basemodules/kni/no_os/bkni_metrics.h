/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BKNI_METRICS_H__
#define BKNI_METRICS_H__

/*=*************************************************************************
The purpose of the metrics interface is to provide a standard way of
monitoring kernel interface usage. This meets a basic requirement of the Magnum
architecture which is to improve system performance. One simple way of doing this is
to know who is sleeping and blocking and for how long. Grep'ing code can find
these spots, but it can't tell you how frequently and for how long these calls
are being made.

No Magnum code is allowed to call the metrics interface. It is only callable
by system level code.

The metrics interface can be turned off or compiled out. If you don't want the overhead,
you're not required to have it. However, we do require that every platform
should implement the metrics interface so that it can be used if desired.

See bkni.h for a kernel interface overview.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Data structure populated by BKNI_Metrics_Get.
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

  uint32_t totalWaitForEvents;		/* Total number of times BKNI_WaitForEvent was called. */
  uint32_t totalWaitForEventTime;	/* Total time in milliseconds spent blocking in
  										BKNI_WaitForEvent calls. */
  uint32_t maxWaitForEventTime;		/* Largest time in milliseconds spent blocking in
  										BKNI_WaitForEvent calls. */

  uint32_t totalCriticalSections;	/* Total number of times BKNI_AcquireMutex was called. */
  uint32_t totalCriticalSectionTime;/* Total time in milliseconds spent between
  										BKNI_EnterCriticalSection and BKNI_LeaveCriticalSection calls. */
  uint32_t maxCriticalSectionTime;	/* Largest time in milliseconds spent between
  										BKNI_EnterCriticalSection and BKNI_LeaveCriticalSection calls. */

  uint32_t totalMallocs;			/* The total number of times BKNI_Malloc is called. */
  uint32_t totalMemoryAllocated;	/* The total number of bytes allocated by BKNI_Malloc.
  										This total is not decremented by BKNI_Free, therefore
										this number only reflects the amount of system
										memory usage, not the current amount actually allocated. */
} BKNI_Metrics;


/***************************************************************************
Summary:
	Retrieve metrics from kernel interface regarding delay, sleep and mutex usage. The
	metrics are accumulated since the last BKNI_Init() or BKNI_Metrics_Reset() call.
****************************************************************************/
void BKNI_Metrics_Get(BKNI_Metrics *metrics);


/***************************************************************************
Summary:
	Reset all metrics to initial values before kernel interface was used.
****************************************************************************/
void BKNI_Metrics_Reset(void);


/***************************************************************************
Summary:
	Structure to control debug output of kernel interface.
	
	Ideally, this debug output should include the FILE and LINE number of each
	call. This can be implemented by using tagged versions of the kernel interface
	functions.

	Also, the output mechanism may be the debug interface, BKNI_Printf or some other
	means. If it is the debug interface, you must also enable the correct
	debug level for the kernel interface module. The exact implementation of this
	is up to the platform developer.
****************************************************************************/
typedef struct {
  bool printDelays;				/* Print every time BKNI_Delay is called. */
  bool printSleeps;				/* Print every time BKNI_Sleep is called. */
  bool printMutexSections;		/* Print every time BKNI_ReleaseMutex is called and
  									every time BKNI_AcquireMutex is entered and exited. */
  bool printCriticalSections;	/* Print every time BKNI_LeaveCriticalSection is called and
  									every time BKNI_EnterCriticalSection is entered and exited. */
  bool printEvents;				/* Print every time BKNI_SetEvent and BKNI_ResetEvent are called and
  									every time BKNI_WaitForEvent is entered and exited. */
} BKNI_Metrics_LoggingState;


/***************************************************************************
Summary:
	Change the debug output of the kernel interface.
****************************************************************************/
void BKNI_Metrics_GetLoggingState(BKNI_Metrics_LoggingState *logging);


/***************************************************************************
Summary:
	Get the debug output state of the kernel interface.
****************************************************************************/
void BKNI_Metrics_SetLoggingState(const BKNI_Metrics_LoggingState *logging);

#ifdef __cplusplus
}
#endif

#endif /* BKNI_METRICS_H__ */
