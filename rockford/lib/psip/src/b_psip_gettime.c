#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"
#include <string.h>
#include <sys/time.h>
/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
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
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: PSIP
*    Library routines for PSIP gettime state
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include <time.h>

#include "ts_psi.h"
#include "psip_stt.h"
#include "psip_descriptor.h"

#include "tsid_table.h"

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"
#include "b_psip_stateengine.h"
#include "b_psip_table.h"

/***************************************************************************
Summary:
    Final state machine state that notifies original caller that 
    acquisition of the current time is complete.  This state will
    also cancel timers and free temporary state machine memory associated
    with the given job.

Description:
    State - GetTime
    SubState - Done

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetTime_Done(
    B_PSIP_Job * pJob, 
    B_PSIP_StateEvent event
    )
{
    B_PSIP_StateData_GetTime * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getTimeData;

    /* we're at the end of the state machine so remove job from list */
    BLST_S_REMOVE(&pJob->apiSettings.siHandle->jobList, pJob, B_PSIP_Job, node);

    /* clean up state data */
    if (pStateData->sttBuffer)
    {
        B_Os_Free(pStateData->sttBuffer);
        pStateData->sttBuffer    = NULL;
        pStateData->sttBufferLen = 0;
    }

    /* stop api timeout timer*/
    B_Scheduler_CancelTimer(pJob->apiSettings.siHandle->scheduler,
                            pJob->apiTimeout);
    
    /* respond to original calling code */
    switch (event)
    {
    case B_PSIP_eStateEvent_Enter:
        pJob->apiSettings.dataReadyCallback( B_ERROR_SUCCESS, pJob->apiSettings.dataReadyCallbackParam);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_ERR(("shouldn't get a table received event in this state:%s", __FUNCTION__));
        break;
    case B_PSIP_eStateEvent_Failure:
        pJob->apiSettings.dataReadyCallback( B_ERROR_UNKNOWN, pJob->apiSettings.dataReadyCallbackParam);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_TableTimeout:
        pJob->apiSettings.dataReadyCallback( B_ERROR_TIMEOUT, pJob->apiSettings.dataReadyCallbackParam);
        break;
    case B_PSIP_eStateEvent_Cancel:
        pJob->apiSettings.dataReadyCallback( B_ERROR_PSIP_CANCEL, pJob->apiSettings.dataReadyCallbackParam);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", event, __FUNCTION__, __LINE__));
        break;
    }

    /* clean up job - no need to change state since job itself is being deleted */
    B_Os_Free(pJob);
    pJob = NULL;
}


/* Jan 6, 1980 => Sunday */
#define START_DAY   0

/* Days/month for non-leap year */
const unsigned char s_days_per_mon[] = 
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

#define IS_LEAP_YEAR(y) ( !((y) % 4) && ( ((y) % 100) || !((y) % 400) ) )

static void utcTime(unsigned int secs,struct tm *p_tm)
{
    unsigned int yday,mon,t_val;
    unsigned char *p_dpm = (unsigned char*)s_days_per_mon;

    memset(p_tm,0,sizeof(struct tm));

    /* seconds */
    p_tm->tm_sec = secs % 60;
    t_val = secs / 60;  /* minutes */

    /* minutes */
    p_tm->tm_min = t_val % 60;
    t_val /= 60; /* hours */

    /* hours */
    p_tm->tm_hour = t_val % 24;
    t_val /= 24;

    /* day of week */
    p_tm->tm_wday = t_val % 7 - START_DAY;

    /* offset value to start of the year */
    t_val += 5;

    /* year */
    p_tm->tm_year = (t_val * 100) / 36524;

    /* year day */
    p_tm->tm_yday = ((t_val * 100) - (p_tm->tm_year * 36524))/100;

    if (IS_LEAP_YEAR(p_tm->tm_year))
        p_dpm += 12;


    yday = p_tm->tm_yday + 1;
    mon = 0;
    while(yday >= s_days_per_mon[mon])
    {
        yday -= s_days_per_mon[mon];
        mon++;
    }

    /* month */
    p_tm->tm_mon = mon;

    /* day of month */
    p_tm->tm_mday = yday;

}

/***************************************************************************
Summary:
    State machine state that gets the STT and saves it in the given job's
    state data.

Description:
    State - GetChList
    SubState - GetVCT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetTime_GetSTT(
    B_PSIP_Job * pJob, 
    B_PSIP_StateEvent event
    )
{
    B_PSIP_StateData_GetTime * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getTimeData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->sttBufferLen = STT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetSTT_P(&pJob->apiSettings, 
                                               pStateData->sttBuffer,
                                               &pStateData->sttBufferLen,
                                               pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob, B_PSIP_eState_GetTime_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        {
            PSIP_STT_header sttHeader;
            struct tm       utcTimeParsed;
            BDBG_MSG(("%s success!", __FUNCTION__));

            /*
            printSTT(pStateData->sttBuffer, pStateData->sttBufferLen);
            */

            PSIP_STT_getHeader(pStateData->sttBuffer, &sttHeader);

            *(pStateData->pUtcSecs)           = sttHeader.system_time - sttHeader.GPS_UTC_offset;
            *(pStateData->pDst)               = sttHeader.daylight_savings.DS_status;
            *(pStateData->pLocalOffsetSource) = pJob->apiSettings.siHandle->localTimeSource;

            *(pStateData->pLocalOffset) = 0;
            if (B_PSIP_eTS_TSID == pJob->apiSettings.siHandle->localTimeSource)
            {
                /* use local time offset calculated from TSID */
                *(pStateData->pLocalOffset)  = pJob->apiSettings.siHandle->localTimeOffset;
            }

            BDBG_MSG(("local time offset:%d, source:%d", 
                      *(pStateData->pLocalOffset),
                      pJob->apiSettings.siHandle->localTimeSource));

            utcTime(*(pStateData->pUtcSecs), &utcTimeParsed);
            BDBG_MSG(("STT UTC = %2d/%2d/%4d %2d:%2d-%s(%2d %2d)",
                      utcTimeParsed.tm_mon + 1,
                      utcTimeParsed.tm_mday,
                      utcTimeParsed.tm_year + 1980, 
                      utcTimeParsed.tm_hour,
                      utcTimeParsed.tm_min,
                      (sttHeader.daylight_savings.DS_status == 0) ? "STD" : "DST",
                      sttHeader.daylight_savings.DS_day_of_month,
                      sttHeader.daylight_savings.DS_hour ));

            CHANGE_STATE(pJob, B_PSIP_eState_GetTime_Done);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}
