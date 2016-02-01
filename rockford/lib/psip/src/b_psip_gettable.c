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
*    Library routines for PSIP gettable state
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"

/***************************************************************************
Summary:
    State machine state that processes asynchronously received PSIP table
    data. Primary purpose is to clean up any state data and to notify the
    original calling code that the job has completed.

Description:
    State - GetTable
    Substate - None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetTable(
    B_PSIP_Job        * pJob, 
    B_PSIP_StateEvent   event
    )
{
    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    /* we're at the end of the state machine so remove job from list */
    BLST_S_REMOVE(&pJob->apiSettings.siHandle->jobList, pJob, B_PSIP_Job, node);

    /* clean up state data */

    /* stop api timeout timer*/
    B_Scheduler_CancelTimer(pJob->apiSettings.siHandle->scheduler,
                            pJob->apiTimeout);

    /* respond to original calling code */
    switch (event)
    {
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
        pJob->apiSettings.dataReadyCallback( B_ERROR_SUCCESS, pJob->apiSettings.dataReadyCallbackParam);
        break;
    case B_PSIP_eStateEvent_Failure:
        pJob->apiSettings.dataReadyCallback( B_ERROR_UNKNOWN, pJob->apiSettings.dataReadyCallbackParam);
        break;
    case B_PSIP_eStateEvent_TableTimeout:
    case B_PSIP_eStateEvent_Timeout:
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
