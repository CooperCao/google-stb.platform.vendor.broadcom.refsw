/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* Description:
*   API name: PSIP
*    Library routines for PSIP gettablever state
*
***************************************************************************/
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"

#include "ts_psi.h"
#include "psip_mgt.h"
#include "psip_descriptor.h"

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
    acquisition of the table version list is complete.  This state will
    also cancel timers and free temporary state machine memory associated
    with the given job.

Description:
    State - GetTableVer
    SubState - Done

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetTableVer_Done(
    B_PSIP_Job * pJob, 
    B_PSIP_StateEvent event
    )
{
    B_PSIP_StateData_GetTableVer * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getTableVerData;

    /* we're at the end of the state machine so remove job from list */
    BLST_S_REMOVE(&pJob->apiSettings.siHandle->jobList, pJob, B_PSIP_Job, node);

    /* clean up state data */
    if (pStateData->mgtBuffer)
    {
        B_Os_Free(pStateData->mgtBuffer);
        pStateData->mgtBuffer    = NULL;
        pStateData->mgtBufferLen = 0;
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
        BDBG_ERR(("shouldn't get a table received event in this state:%s", BSTD_FUNCTION));
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
        BDBG_ERR(("invalid event:%d received in state:%s:%d", event, BSTD_FUNCTION, __LINE__));
        break;
    }

    /* clean up job - no need to change state since job itself is being deleted */
    B_Os_Free(pJob);
    pJob = NULL;
}

/***************************************************************************
Summary:
    State machine state that gets the MGT and saves it in the given job's
    state data.  Also does table version comparisons.

Description:
    State - GetTableVer
    SubState - GetMGT
    
    Go through MGT table list and compare tableType/pid/version to 
    given verList. Set/clear the changed bit where appropriate.  EIT/ETT 
    must be handled in a slightly different manner than the other tables 
    because tableType/PID pairs can change while table versions do not.

    PIDs are saved if they did not already exist in the version list.
    
Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetTableVer_GetMGT(
    B_PSIP_Job * pJob, 
    B_PSIP_StateEvent event
    )
{
    B_PSIP_StateData_GetTableVer * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getTableVerData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->mgtBufferLen = MGT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetMGT_P(&pJob->apiSettings, 
                                               pStateData->mgtBuffer,
                                               &pStateData->mgtBufferLen,
                                               pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob, B_PSIP_eState_GetTableVer_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        {
            int i = 0;
            int j = 0;
            BDBG_MSG(("%s success!", BSTD_FUNCTION));

            /*
            printMGT(pJob->apiSettings.siHandle, pStateData->mgtBuffer, pStateData->mgtBufferLen);
            */

            /* go thru mgt table list and compare tableType/pid/version to given verList.
               set/clear the changed bit where appropriate.  EIT/ETT must be handled
               in a slightly different manner than the other tables because 
               tableType/PID pairs can change while table versions do not. */
            for (i = 0; i < pStateData->length; i++)
            {
                bool mgtComparisonDone = false;

                /* initialize table change value to 'invalid' */
                pStateData->verList[i].changed = B_PSIP_eTableStatusInvalid;

                mgtComparisonDone = false;
                for (j = 0; 
                     (j < PSIP_MGT_getTablesDefined(pStateData->mgtBuffer)) && (false == mgtComparisonDone); 
                     j++)
                {
                    PSIP_MGT_table table;
                    PSIP_MGT_getTable(pStateData->mgtBuffer, j, &table);

                    if ((0 == pStateData->verList[i].pid) &&
                        (pStateData->verList[i].tableType == table.table_type))
                    {
                        /********************************************/
                        /* no pid given so just copy MGT table data */
                        /********************************************/
                        BDBG_MSG(("Table copy from MGT! verList index:%d table:0x%04x pid:0x%04x old version:%d new version:%d", 
                                  i,
                                  pStateData->verList[i].tableType,
                                  table.table_type_PID,
                                  pStateData->verList[i].version,
                                  table.table_type_version_number));
                        pStateData->verList[i].pid     = table.table_type_PID;
                        pStateData->verList[i].version = table.table_type_version_number;
                        pStateData->verList[i].changed = B_PSIP_eTableStatusPidChanged | 
                                                         B_PSIP_eTableStatusVersionChanged;
                        mgtComparisonDone = true;
                    }
                    else 
                    if (((0x0100 <= pStateData->verList[i].tableType) && (0x017F >= pStateData->verList[i].tableType)) ||
                        ((0x0200 <= pStateData->verList[i].tableType) && (0x027F >= pStateData->verList[i].tableType)))
                    {
                        /************************************************/
                        /* if EIT or ETT table so do version comparison */
                        /************************************************/
                        if (pStateData->verList[i].tableType != table.table_type)
                        {
                            continue; /* j loop */
                        }
                        
                        /* for EIT/ETT the pid may change without the table version changing. 
                           this occurs when EIT-1 becomes EIT-0. we will first update pid/version 
                           if necessary */
                        if (pStateData->verList[i].pid != table.table_type_PID)
                        {
                            int k = 0;

                            /* find table_type_PID in verList if it exists */
                            for (k = 0; k < pStateData->length; k++)
                            {
                                if (pStateData->verList[k].pid == table.table_type_PID)
                                {
                                    break; /* k loop */
                                }
                            }

                            if (k == pStateData->length)
                            {
                                BDBG_MSG(("Table copy from MGT! verList index:%d table:0x%04x pid:0x%04x old version:%d new version:%d", 
                                          i,
                                          pStateData->verList[i].tableType,
                                          table.table_type_PID,
                                          pStateData->verList[i].version,
                                          table.table_type_version_number));
                                /* didn't find table_type_PID in verList so we'll just copy
                                   it from the MGT table data */
                                pStateData->verList[i].pid     = table.table_type_PID;
                                pStateData->verList[i].version = table.table_type_version_number;
                                pStateData->verList[i].changed = B_PSIP_eTableStatusPidChanged | 
                                                                 B_PSIP_eTableStatusVersionChanged;
                                mgtComparisonDone = true;
                            }
                            else
                            {
                                BDBG_MSG(("Update table PID/version! verList index:%d table:0x%04x old pid:0x%04x new pid:0x%04x old version:%d new version:%d", 
                                          i,
                                          pStateData->verList[i].tableType,
                                          pStateData->verList[i].pid,
                                          pStateData->verList[k].pid,
                                          pStateData->verList[i].version,
                                          pStateData->verList[k].version));
                                /* found table_type_PID in verList - means that we already
                                   have a pid/version for this eit/ett to compare against
                                   the table version in the mgt. copy it to our current 
                                   position in verList */
                                pStateData->verList[i].pid     = pStateData->verList[k].pid;
                                pStateData->verList[i].version = pStateData->verList[k].version;
                            }
                        }

                        if (false == mgtComparisonDone)
                        {
                            /* at this point we know that both tableType and pid match so 
                               we can compare versions */
                            if (pStateData->verList[i].version != table.table_type_version_number)
                            {
                                BDBG_MSG(("Table changed! verList index:%d table:0x%04x pid:0x%04x old version:%d new version:%d", 
                                          i,
                                          pStateData->verList[i].tableType,
                                          pStateData->verList[i].pid,
                                          pStateData->verList[i].version,
                                          table.table_type_version_number));
                                pStateData->verList[i].version = table.table_type_version_number;
                                pStateData->verList[i].changed = B_PSIP_eTableStatusVersionChanged;
                            }
                            else
                            {
                                pStateData->verList[i].changed = B_PSIP_eTableStatusNoChange;
                            }

                            mgtComparisonDone = true;
                        }
                    }
                    else 
                    if (pStateData->verList[i].tableType == table.table_type)
                    {
                        /*************************************************************/
                        /* TVCT, CVCT, DCCSCT, or RRT table so do version comparison */
                        /*************************************************************/

                        if ((pStateData->verList[i].pid     == table.table_type_PID) &&
                            (pStateData->verList[i].version != table.table_type_version_number))
                        {
                            BDBG_MSG(("Table changed! verList index:%d table:0x%04x pid:0x%04x old version:%d new version:%d", 
                                      i,
                                      pStateData->verList[i].tableType,
                                      pStateData->verList[i].pid,
                                      pStateData->verList[i].version,
                                      table.table_type_version_number));
                            pStateData->verList[i].version = table.table_type_version_number;
                            pStateData->verList[i].changed = B_PSIP_eTableStatusVersionChanged;
                        }
                        else
                        {
                            pStateData->verList[i].changed = B_PSIP_eTableStatusNoChange;
                        }

                        mgtComparisonDone = true;
                    }
                }
            }

            CHANGE_STATE(pJob, B_PSIP_eState_GetTableVer_Done);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, BSTD_FUNCTION, __LINE__));
        break;
    }
}
