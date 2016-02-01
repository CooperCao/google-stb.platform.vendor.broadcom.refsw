/***************************************************************************
 *	   Copyright (c) 2003-2007, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

#ifndef BINT_STATS_H
#define BINT_STATS_H

#include "bstd.h"
#include "bint.h"
#include "btmr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BDBG_DEBUG_BUILD
#define BINT_STATS_ENABLE
#endif /* BDBG_DEBUG_BUILD */

/***************************************************************************
Summary:
	List of errors unique to INT
****************************************************************************/
#define BINT_STATS_ERR_ALREADY_ENABLED           BERR_MAKE_CODE(BERR_INT_ID, 0)
#define BINT_STATS_ERR_ALREADY_DISABLED          BERR_MAKE_CODE(BERR_INT_ID, 1)

#define BINT_P_STATS_BIN_MAX 10 /* Number of maximum bins. */
#define BINT_P_STATS_SAMPLE_MAX 10000
#define BINT_P_STATS_RECENT_CB_HIT_COUNT 20 /* Number of hits used to track hit frequency */
#define BINT_P_STATS_EXECUTION_TIME_MAX_THRESHOLD 5000 /* Maxmim time for callback execution */
#define BINT_P_STATS_AVG_PERIOD_MIN_THRESHOLD 1000 /* Minimum avg time between callback hits */

/*
This structure defines a callback statistics bin.  It has a minimum and maximum range in
microseconds. Each time the callback associated with it executes and completes within the
minimum and maximum range of the bin, ulBinHitCount of the bin increments.
*/
typedef struct BINT_Stats_CallbackBin
{
	uint32_t ulBinRangeMin; /* minimum bin range, in microseconds. */
	uint32_t ulBinRangeMax; /* maximum bin range, in microseconds. */
	uint32_t ulBinHitCount; /* count of times callback executed within this bin's range. */
}BINT_Stats_CallbackBin;

/*
This structure keeps track of a callback's statistics.  It is updated every time the
callback executes, and can be obtained by callling BINT_Stats_Get().  It may be
necessary to lock this structure down when reading from it to ensure that it is
not updated in mid-read.
*/
typedef struct BINT_Stats_CallbackStats
{
	uint32_t ulTimeMin;    /* Minimum execution time. */
	uint32_t ulTimeMax;    /* Maximum execution time. */
	uint32_t ulTimeAvg;    /* Average execution time.  Average is over ulCbHitCount
						      samples, if ulCbHitCount is less than BINT_P_STATS_SAMPLE_MAX.

						      If ulCbHitCount is greater than BINT_P_STATS_SAMPLE_MAX,
						      Average is roughly equivalent to BINT_P_STATS_SAMPLE_MAX
						      samples */
	uint32_t ulCbHitCount; /* Number of times callback has executed. */
	bool bDefaultBins;     /* whether default bins or user bins are being used */
	uint32_t ulActiveBins; /* Number of active bins */

	BINT_Stats_CallbackBin aBinInfo[BINT_P_STATS_BIN_MAX];   /* array storing stat bins */
	uint32_t aulTimeStamp[BINT_P_STATS_RECENT_CB_HIT_COUNT]; /* circular array storing start times of
														        last BINT_P_STATS_RECENT_CB_HIT_COUNT
															    hits to track hit frequency */
	uint32_t ulTimeStampStartIdx;                            /* start index of above array */

}BINT_Stats_CallbackStats;

/*
Summary:
Enables statistics tracking of callbacks.

Description:
Enables tracking of statistics for the interrupt module.  Must be called for
statistics to be tracked.  If enabled, a call to BINT_Stats_Disable is
required before the module is closed with BINT_Close.

Access to a shared freerun timer from the TMR module will be acquired when
stats tracking is enabled.
*/
BERR_Code BINT_Stats_Enable(
							BINT_Handle intHandle, /* [in] Interrupt handle */
							BTMR_Handle hTmrHandle /* [in] Timer module handle */
							);

/*
Summary:
Disables statistics tracking of callbacks.

Description:
Disables tracking of statistics for the interrupt module.  Must be called before
the module is closed with BINT_Close if stats tracking was enabled with
BINT_Stats_Enable.

Disabling stats tracking also relinquishes access to the shared freerun timer
acquired when tracking was enabled.
*/
BERR_Code BINT_Stats_Disable(
							BINT_Handle intHandle /* [in] Interrupt handle */
							);

/*
Summary:
Adds a statistics bin to a callback.

Description:
Adds a bin with a minimum and maximum time range in microseconds.  The ulBinHitCount
field will be incremented in this bin everytime the execution of the callback falls
within the time range.  Returns an error if maximum number of bins is exceeded or
if the max and min ranges overlap or fall within an existing bin's range.  No two
bins can have overlapping ranges.

Adding a bin removes the default bins for a callback.
*/
BERR_Code BINT_Stats_AddBin(
							BINT_CallbackHandle cbHandle, /* [in] Callback handle returned by BINT_CreateCallback() */
							uint32_t ulRangeMin, /* [in] Minimum time range, in microseconds */
							uint32_t ulRangeMax  /* [in] Maximum time range, in microseconds */
							);

/*
Summary:
Destroys existing stats bins of a callback.

Description:
Destroys and clears out the created stats bins of a given callback.
*/
BERR_Code BINT_Stats_DestroyBins(
							BINT_CallbackHandle cbHandle /* [in] Callback handle returned by BINT_CreateCallback() */
							);


/*
Summary:
Gets the statistics of a callback.

Description:
Gets the statistics of a specific callback.  It returns the pointer to the BINT_CallbackStats
structure associated with the callback.  The BINT_CallbackStats structure is constantly being
updated, each time the callback executes.  It may be necessary to use critical sections when
reading from the callback structure to make sure the stats don't update in mid-read.

#define BINT_STATS_ENABLE to enable stats tracking.  Stats tracking functions and datastructures
are disabled by default.
*/
BERR_Code BINT_Stats_Get(
						 BINT_CallbackHandle cbHandle,
						 BINT_Stats_CallbackStats **ppCbStats
						 );

/*
Summary:
Resets the statistics of a callback.

Description:
Resets the statistics of a specific callback.  All stats for the callback and its bins are
reset when this function is called.
*/
BERR_Code BINT_Stats_Reset(
						 BINT_CallbackHandle cbHandle
						 );

#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */



