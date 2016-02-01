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
*    Library typedefs for PSIP job
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_JOB_H__
#define B_PSIP_JOB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "b_psip_getproginfo.h"
#include "b_psip_getchlist.h"
#include "b_psip_gettablever.h"
#include "b_psip_gettime.h"

/***************************************************************************
Summary:
Each major path through the state machine may require this temporary data.
Note that this data is on a per job basis.
***************************************************************************/
typedef union B_PSIP_StateData
{
    B_PSIP_StateData_GetProgInfo getProgInfoData;   /* for B_PSIP_GetProgInfo() */
    B_PSIP_StateData_GetChList   getChListData;     /* for B_PSIP_GetChannelList() and B_PSIP_GetNumChannels() */
    B_PSIP_StateData_GetTableVer getTableVerData;   /* for B_PSIP_GetTableVersions() */
    B_PSIP_StateData_GetTable    getTableData;      /* for Table data retrieval APIs */
    B_PSIP_StateData_GetTime     getTimeData;       /* for B_PSIP_GetTime() */
} B_PSIP_StateData;

/***************************************************************************
Summary:
A Job corresponds to a single API call and maintains context data as it 
progresses through the state machine.
***************************************************************************/
typedef struct B_PSIP_Job
{
    BLST_S_ENTRY(B_PSIP_Job)  node;                 /* List node */
    B_PSIP_ApiSettings        apiSettings;          /* Original API settings provided by the caller */
    B_EventHandle             eventHandle;          /* Event handle used to signal the scheduler to begin processing received table data */
    B_SchedulerEventId        schedulerEventId;     /* Scheduler event/callback id */
    B_SchedulerTimerId        tableTimeout;         /* Scheduler table timeout id */
    B_SchedulerTimerId        apiTimeout;           /* Scheduler api timeout id */
    B_PSIP_CollectionRequest  request;              /* Collection request data */
    B_PSIP_State              stateLast;            /* Last state */
    B_PSIP_State              state;                /* Current state */
    B_PSIP_StateData          stateData;            /* Temporary state specific data */
} B_PSIP_Job;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_JOB_H__ */

