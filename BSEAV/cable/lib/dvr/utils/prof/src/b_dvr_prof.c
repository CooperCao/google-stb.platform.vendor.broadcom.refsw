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

#include "b_dvr_prof.h"

typedef struct B_DVR_ProfData {
    char name[B_DVR_MAXFUNCNAME];    /* usually function name */
    unsigned long calls;    /* number of calls measured */
    struct timespec totalTime;     /* total execution time */
    struct timespec peakHigh;      /* highest peak time */
    struct timespec peakLow;       /* lowest peak time */
    struct timespec res;           /* timer resolution */
    unsigned int peakCheck; /* peak check enable */
    unsigned long long totalUserdata;    /* sum of userdata's given */
} B_DVR_ProfData;

static unsigned gProfNum = 0;
static B_DVR_ProfData *gProfData=NULL;

static void B_DVR_P_ProfUpdate(B_DVR_ProfTimerId timerId, struct timespec start, struct timespec end, unsigned long userdata);
static struct timespec B_DVR_P_ProfDiff(struct timespec start, struct timespec end);

BDBG_MODULE(b_dvr_prof);

B_DVR_ERROR B_DVR_Prof_Create(unsigned int n)
{
    struct timespec res;
    unsigned int i;
    
    gProfData = BKNI_Malloc(sizeof(B_DVR_ProfData)*n);
    if (!gProfData) {
        BDBG_ERR(("malloc failed"));
        return B_DVR_OUT_OF_SYSTEM_MEMORY;
    }

    gProfNum = n;
    
    BKNI_Memset(gProfData,0,sizeof(B_DVR_ProfData)*n);

    if (clock_getres(CLOCK_MONOTONIC,&res)<0) {
        BDBG_ERR(("clock getres failed: %s",strerror(errno)));
        return B_DVR_SYSTEM_ERR;
    }

    for (i=0; i<n; i++) {
        snprintf(gProfData[i].name,B_DVR_MAXFUNCNAME-1,"PROFDATA_%03u",i);  /* set default name */
        gProfData[i].peakCheck = 1; /* enable peakCheck by default */
        gProfData[i].res = res;
    }

    return B_DVR_SUCCESS;
}

void B_DVR_Prof_Destroy(void)
{
    if (gProfData)
        BKNI_Free(gProfData);
}

B_DVR_ERROR B_DVR_Prof_SetSettings(B_DVR_ProfTimerId timerId, B_DVR_ProfSettings *settings)
{
    B_DVR_ProfData *profData;

    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return B_DVR_NOT_INITIALIZED; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return  B_DVR_INVALID_INDEX;
    }

    profData = gProfData+timerId;

    snprintf(profData->name,B_DVR_MAXFUNCNAME-1,"%s",settings->name);
    profData->peakCheck = settings->peakCheck;

    return B_DVR_SUCCESS;
}

B_DVR_ERROR B_DVR_Prof_GetSettings(B_DVR_ProfTimerId timerId, B_DVR_ProfSettings *settings)
{
    B_DVR_ProfData *profData;

    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return B_DVR_NOT_INITIALIZED; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return  B_DVR_INVALID_INDEX;
    }

    profData = gProfData+timerId;

    snprintf(settings->name,B_DVR_MAXFUNCNAME-1,"%s",profData->name);
    settings->peakCheck = profData->peakCheck;

    return B_DVR_SUCCESS;
}

B_DVR_ERROR B_DVR_Prof_Reset(B_DVR_ProfTimerId timerId)
{
    B_DVR_ProfData *profData;

    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return B_DVR_NOT_INITIALIZED; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return  B_DVR_INVALID_INDEX;
    }

    profData = gProfData+timerId;

    profData->calls=0;
    profData->peakHigh.tv_sec=0;
    profData->peakHigh.tv_nsec=0;
    profData->peakLow.tv_nsec=0;
    profData->peakLow.tv_sec=0;
    profData->totalTime.tv_sec=0;
    profData->totalTime.tv_nsec=0;   
    profData->totalUserdata = 0;

    return B_DVR_SUCCESS;
}

static struct timespec calc_mean(struct timespec time, unsigned long n)
{
    unsigned long long nsec;
    struct timespec mean;

    if (n==0) return time;

    nsec = (unsigned long long)time.tv_sec*1000000000 + (unsigned long long)time.tv_nsec;
    nsec /= n;

    mean.tv_sec = (time_t)(nsec/1000000000);
    mean.tv_nsec = (long)(nsec%1000000000);

    return mean;
}

B_DVR_ERROR B_DVR_Prof_GetStatus(B_DVR_ProfTimerId timerId, B_DVR_ProfStatus *status)
{
    B_DVR_ProfData *profData;

    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return B_DVR_NOT_INITIALIZED; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return  B_DVR_INVALID_INDEX;
    }

    profData = gProfData+timerId;

    snprintf(status->name,B_DVR_MAXFUNCNAME-1,"%s",profData->name);
    status->res = profData->res;
    status->calls = profData->calls;
    status->meanTime = calc_mean(profData->totalTime, profData->calls);
    status->totalTime = profData->totalTime;
    status->peakHigh = profData->peakHigh;
    status->peakLow = profData->peakLow;
    status->totalProfs = gProfNum;
    status->totalUserdata = profData->totalUserdata;
    return B_DVR_SUCCESS;
}

void B_DVR_Prof_Start(B_DVR_ProfTimerId timerId, struct timespec *start)
{
    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, start);
}

void B_DVR_Prof_Stop(B_DVR_ProfTimerId timerId, struct timespec *start)
{
    struct timespec end;
    
    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return; 
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    B_DVR_P_ProfUpdate(timerId,*start, end, 0);
}

void B_DVR_Prof_StopUser(B_DVR_ProfTimerId timerId, struct timespec *start, unsigned long userdata)
{
    struct timespec end;

    if (!gProfData) { /* profiler not initilaized */
        BDBG_ERR(("profiler not initialized"));
        return; 
    }

    if (timerId>=gProfNum) { /* invaid timerId */
        BDBG_ERR(("invalid timer id : %d", timerId));
        return; 
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    B_DVR_P_ProfUpdate(timerId,*start, end, userdata);
}

static void B_DVR_P_ProfUpdate(B_DVR_ProfTimerId timerId, struct timespec start, struct timespec end, unsigned long userdata)
{
	struct timespec diff; /* exe time for this run */
	struct timespec temp; 
    B_DVR_ProfData *profData;

    profData = gProfData+timerId;

    /* update calls */
    profData->calls++;
    
    /* update userdata */
    profData->totalUserdata += userdata;
    
    /* update total time */
    diff = B_DVR_P_ProfDiff(start,end);

    profData->totalTime.tv_nsec += diff.tv_nsec;
    profData->totalTime.tv_sec += diff.tv_sec;

    if (profData->totalTime.tv_nsec>=1000000000) {
        profData->totalTime.tv_nsec -= 1000000000;
        profData->totalTime.tv_sec++;
    }

    /* update high/low peak values */
    if (profData->peakCheck) {
        if (profData->peakLow.tv_sec==0 && profData->peakLow.tv_nsec==0) {
            profData->peakLow = diff;
        } 
        if (profData->peakHigh.tv_sec==0 && profData->peakHigh.tv_nsec==0) {
            profData->peakHigh = diff;
        } else {        
            temp = B_DVR_P_ProfDiff(profData->peakHigh,diff);
            if (temp.tv_sec>0) {
                profData->peakHigh = temp;
            } else if (temp.tv_sec==0) {
                if (temp.tv_nsec>0)
                    profData->peakHigh = temp;
            } else {
                temp = B_DVR_P_ProfDiff(profData->peakLow,diff);
                if (temp.tv_sec<0) {
                    profData->peakLow = diff;
                } else if (temp.tv_sec==0) {
                    if (temp.tv_nsec<0)
                        profData->peakLow = diff;
                }   
            } 
        }
    } /* peackcheck */    
}

static struct timespec B_DVR_P_ProfDiff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

