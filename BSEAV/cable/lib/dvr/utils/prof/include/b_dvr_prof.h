/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *  ANY LIMITED REMEDY..
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Simple profiler utility for DVR extenstion library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#ifndef _B_DVR_PROF_H
#define _B_DVR_PROF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"

#define B_DVR_MAXFUNCNAME   31

typedef unsigned int B_DVR_ProfTimerId;

typedef struct {
    char name[B_DVR_MAXFUNCNAME]; /* set timer name, usually function name */
    unsigned int peakCheck; /* enable peak check if 1 */
} B_DVR_ProfSettings;

typedef struct {
    unsigned totalProfs;           /* total profilers created*/
    char name[B_DVR_MAXFUNCNAME];             /* default name is PROFDATA_[ID] */
    unsigned long calls;    /* number of calls measured */
    struct timespec peakHigh;      /* highest peak time */
    struct timespec peakLow;       /* lowest peak time */
    struct timespec totalTime;     /* total execution time*/
    struct timespec meanTime;      /* mean execution time*/
    struct timespec res;           /* timer resolution */
    unsigned long long totalUserdata;    /* total sum of userdata given */
} B_DVR_ProfStatus;

/**
Summary:
Creates profiler n instances
**/
B_DVR_ERROR B_DVR_Prof_Create(unsigned int n);

/**
Summary:
Destroy profiler instances.
**/
void B_DVR_Prof_Destroy(void);

/**
Summary:
Returns current settings of a profiler
**/
B_DVR_ERROR B_DVR_Prof_GetSettings(B_DVR_ProfTimerId timerId, B_DVR_ProfSettings *settings);

/**
Summary:
Changes configuration of a profiler
**/
B_DVR_ERROR B_DVR_Prof_SetSettings(B_DVR_ProfTimerId timerId, B_DVR_ProfSettings *settings);

/**
Summary:
Returns current status of a profiler
**/
B_DVR_ERROR B_DVR_Prof_GetStatus(B_DVR_ProfTimerId timerId, B_DVR_ProfStatus *status);

/**
Summary:
Start a measurement for a profiler
**/
void B_DVR_Prof_Start(B_DVR_ProfTimerId timerId, struct timespec *start);

/**
Summary:
End a measurement for a profiler
**/
void B_DVR_Prof_Stop(B_DVR_ProfTimerId timerId, struct timespec *start);

/**
Summary:
End a measurement for a profiler with userdata
user data will be accumulated.
**/
void B_DVR_Prof_StopUser(B_DVR_ProfTimerId timerId, struct timespec *start, unsigned long userdata);

/**
Summary:
Reset a profiler
**/
B_DVR_ERROR B_DVR_Prof_Reset(B_DVR_ProfTimerId timerId);

#ifdef __cplusplus
}
#endif

#endif
