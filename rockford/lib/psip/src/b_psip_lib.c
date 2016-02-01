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
*    Library routines for PSIP abstraction
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"
#include <string.h>

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"
#include "b_psip_states_decl.h"
#include "b_psip_stateengine.h"
#include "b_psip_table.h"
#include "b_psip_utils.h"

/***************************************************************************
Summary:
Timeout for API calls.

Description:
    Callback function called when an API data request times out.  param 
    contains a pointer to the original request.

Note: 
    This is a scheduler based callback so the si mutex is automatically
    set on entry and reset on exit.
***************************************************************************/
static void B_PSIP_TimeoutAPI(void * param)
{
    B_PSIP_CollectionRequest * pRequest = (B_PSIP_CollectionRequest *)param;
    B_PSIP_Handle              si       = pRequest->si;

    B_PSIP_Job * pJob = B_PSIP_FindJob(si, pRequest->filterHandle);
    if (pJob)
    {
        BDBG_MSG(("#### API TIMEOUT! eventHandle:0x%04x stateCurrent:%d stateLast:%d",
                  pJob->eventHandle,
                  pJob->state,
                  pJob->stateLast));

        if (B_PSIP_eCollectStop != pRequest->cmd)
        {
            /* stop data collection */
            pRequest->cmd = B_PSIP_eCollectStop;
            si->CollectionFunc(pRequest, si->CollectionContext);
        }

        /* cancel table timeout */
        B_Scheduler_CancelTimer(si->scheduler, pJob->tableTimeout);

        if (pJob->eventHandle)
        {
            /* clean up */
            B_Scheduler_UnregisterEvent(si->scheduler, pJob->schedulerEventId);
            B_Event_Destroy(pJob->eventHandle);
            pJob->eventHandle = NULL;
        }

        /* advance state engine */
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Timeout);
    }
    else
    {
        BDBG_WRN(("### api timeout received, but no corresponding job exists in list!"));
    }
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Program Allocation Table (PAT)

Description:
    Request the Program Allocation Table (PAT).  This call will return before
    the PAT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of PAT. 
    B_ERROR_TIMEOUT         On failed acquisition of PAT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetPAT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetPAT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.PatTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Program Allocation Table (PAT)

Description:
    Request the Program Map Table (PMT).  This call will return before
    the PMT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of PMT. 
    B_ERROR_TIMEOUT         On failed acquisition of PMT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetPMT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetPMT_P(pApiSettings, pid, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.PmtTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Master Guide Table (MGT)

Description:
    Request the Master Guide Table (MGT).  This call will return before
    the PMT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of MGT. 
    B_ERROR_TIMEOUT         On failed acquisition of MGT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetMGT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal   = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob     = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetMGT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.MgtTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }
    
    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Terrestrial Virtual Channel Table 
    (TVCT)

Description:
    Request the Terrestrial Virtual Channel Table (TVCT).  This call will 
    return before the TVCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the TVCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of TVCT. 
    B_ERROR_TIMEOUT         On failed acquisition of TVCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetTVCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetTVCT_P(pApiSettings, section, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.TvctTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Cable Virtual Channel Table (CVCT)

Description:
    Request the Cable Virtual Channel Table (CVCT).  This call will 
    return before the CVCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the CVCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of CVCT. 
    B_ERROR_TIMEOUT         On failed acquisition of CVCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetCVCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetCVCT_P(pApiSettings, section, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.CvctTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the System Time Table (STT)

Description:
    Request the System Time Table (STT).  This call will return before the 
    STT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the STT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of STT. 
    B_ERROR_TIMEOUT         On failed acquisition of STT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetSTT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetSTT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.SttTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Event Information Table (EIT)

Description:
    Request the Event Information Table (EIT).  This call will return 
    before the EIT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the EIT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pid                     PID to seach for event information
    sourceId                Indentifies the program information to look for.
    section                 Sections to acquire (EIT can span multiple sections)                 
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of STT. 
    B_ERROR_TIMEOUT         On failed acquisition of STT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetEIT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{                       
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    retVal = B_PSIP_GetEIT_P(pApiSettings, pid, sourceId, section, pBuffer, pBufferLength, NULL);

    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.EitTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Extended Text Table (ETT)

Description:
    Request the Extended Text Table (ETT).  This call will return 
    before the ETT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the ETT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pid                     PID to seach for extended text information
    sourceId                Indentifies the program information to look for.
    section                 Sections to acquire (ETT can span multiple sections)                 
    etmType                 Channel or Event ETM.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of ETT. 
    B_ERROR_TIMEOUT         On failed acquisition of ETT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetETT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint16_t                section,
    B_PSIP_EtmIdType        etmType,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    retVal = B_PSIP_GetETT_P(pApiSettings, pid, sourceId, section, etmType, pBuffer, pBufferLength, NULL);

    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.EttTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Region Rating Table (RTT)

Description:
    Request the Region Rating Table (RTT).  This call will return 
    before the RTT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the RTT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of RTT. 
    B_ERROR_TIMEOUT         On failed acquisition of RTT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetRRT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetRRT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.RrtTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Directed Channel Change Table 
    (DCCT)

Description:
    Request the Directed Channel Change Table (DCCT).  This call will return 
    before the DCCT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the DCCT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of DCCT. 
    B_ERROR_TIMEOUT         On failed acquisition of DCCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetDCCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetDCCT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.DcctTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Directed Channel Change 
    Selection Code Table (DCCSCT)

Description:
    Request the Directed Channel Change Selection Code Table (DCCSCT).  
    This call will return before the DCCSCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the DCCSCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of DCCSCT. 
    B_ERROR_TIMEOUT         On failed acquisition of DCCSCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetDCCSCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    )
{
    B_Error        retVal = B_ERROR_SUCCESS;
    B_PSIP_Job   * pJob   = NULL;

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    retVal = B_PSIP_GetDCCSCT_P(pApiSettings, pBuffer, pBufferLength, NULL);
    pJob = B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle);
    if (pJob)
    {
        if (0 == pApiSettings->timeout)
        {
            pApiSettings->timeout = pApiSettings->siHandle->settings.DccsctTimeout;
        }

        /* start api timeout */
        pJob->apiTimeout =
            B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                   pApiSettings->siHandle->mutex, 
                                   pApiSettings->timeout,
                                   B_PSIP_TimeoutAPI,
                                   &pJob->request);

        CHANGE_STATE(pJob, B_PSIP_eState_GetTable);
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Get the default settings for PSIP instantiation (see B_PSIP_Open())

Description:
    Initialize PSIP settings structure with default values.  In most cases
    these values can be used without modification.

Input:
    None

Output:
    pSettings               Struct containing default PSIP applib settings 

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultSettings(
    B_PSIP_Settings * pSettings 
    )
{
    BDBG_ASSERT(NULL != pSettings);

    memset( pSettings, 0, sizeof(B_PSIP_Settings) );

    pSettings->PatTimeout    = B_PSIP_PAT_TIMEOUT;
    pSettings->PmtTimeout    = B_PSIP_PMT_TIMEOUT;
    pSettings->MgtTimeout    = B_PSIP_MGT_TIMEOUT;
    pSettings->TvctTimeout   = B_PSIP_TVCT_TIMEOUT;
    pSettings->CvctTimeout   = B_PSIP_CVCT_TIMEOUT;
    pSettings->RrtTimeout    = B_PSIP_RRT_TIMEOUT;
    pSettings->EitTimeout    = B_PSIP_EIT_TIMEOUT;
    pSettings->EttTimeout    = B_PSIP_ETT_TIMEOUT;
    pSettings->SttTimeout    = B_PSIP_STT_TIMEOUT;
    pSettings->DcctTimeout   = B_PSIP_DCCT_TIMEOUT;
    pSettings->DccsctTimeout = B_PSIP_DCCSCT_TIMEOUT;

    /* default rating region to USA */
    pSettings->RatingRegion = 1;

    /* default language to english */
    pSettings->ISO_639_language_code[0][0] = 'e';
    pSettings->ISO_639_language_code[0][1] = 'n';
    pSettings->ISO_639_language_code[0][2] = 'g';
}

/***************************************************************************
Summary:
    Get the default settings for the API settings structure

Description:
    Initialize PSIP API settings structure with default values.  

Input:
    None

Output:
    pSettings               Struct containing default PSIP API applib settings 

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultApiSettings(
    B_PSIP_ApiSettings * pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    memset(pSettings, 0, sizeof(B_PSIP_ApiSettings));
}

/***************************************************************************
Summary:
    Create a PSIP instance

Description:
    Creates and initializes a PSIP instance

Input:
    pSettings               Struct containing default PSIP applib settings 
    CollectionFunc          Function to be called to retrieve SI data and 
                            save in a given buffer 
    CollectionContext       Parameter to be passed to CollectionFunc() 

Output:
    None

Returns:
    B_PSIP_Handle           Handle to a new instance of PSIP

See Also:
    None
****************************************************************************/
B_PSIP_Handle B_PSIP_Open(
    B_PSIP_Settings           * pSettings, 
    B_PSIP_CollectionCallback   CollectionFunc,    
    void                      * CollectionContext  
    )
{
    B_PSIP * pSI = NULL;

    BDBG_ASSERT(NULL != CollectionFunc);

    B_Os_Init();

    pSI = B_Os_Calloc(1, sizeof(B_PSIP));
    BDBG_ASSERT(NULL != pSI);

    memcpy(&pSI->settings, pSettings, sizeof(B_PSIP_Settings));

    pSI->CollectionFunc    = CollectionFunc;
    pSI->CollectionContext = CollectionContext;
    pSI->mutex             = B_Mutex_Create(NULL);

    BLST_S_INIT(&pSI->jobList);
    return pSI;
}

/***************************************************************************
Summary:
    Destroy a PSIP instance

Description:
    Destroy a PSIP instance

Input:
    si                      Handle to PSIP instance to destroy.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Close(
    B_PSIP_Handle si 
    )
{
    BDBG_ASSERT(NULL != si);

    if (si->schedulerThread)
    {
        B_PSIP_Stop(si);
    }

    B_Mutex_Destroy(si->mutex);

    B_Os_Free(si);
    B_Os_Uninit();
}

/***************************************************************************
Summary:
    Get the settings for the given PSIP instance.

Description:
    Get the settings for the given PSIP instance.

Input:
    si                      Handle to PSIP instance 

Output:
    pSettings               Settings structure containing current PSIP 
                            settings.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetSettings(
    B_PSIP_Handle     si,       
    B_PSIP_Settings * pSettings
    )
{
    BDBG_ASSERT(NULL != si);

    B_Mutex_Lock(si->mutex);
    memcpy(pSettings, &si->settings, sizeof(B_PSIP_Settings));
    B_Mutex_Unlock(si->mutex);
}

/***************************************************************************
Summary:
    Set the settings for the given PSIP instance.

Description:
    Set the settings for the given PSIP instance.

Input:
    si                      Handle to PSIP instance 
    pSettings               Settings structure data to use.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_SetSettings(
    B_PSIP_Handle     si,
    B_PSIP_Settings * pSettings
    )
{
    BDBG_ASSERT(NULL != si);

    B_Mutex_Lock(si->mutex);
    memcpy(&si->settings, pSettings, sizeof(B_PSIP_Settings));
    B_Mutex_Unlock(si->mutex);
}

/***************************************************************************
Summary:
    This API must be called before the PSIP applib will service any
    incomming PSIP requests.  

Description:
    Start the PSIP applib internal worker thread.

Input:
    si                      Handle to PSIP instance 

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
B_Error B_PSIP_Start(
    B_PSIP_Handle si
    )
{
    B_ThreadSettings settingsThread;

    BDBG_ASSERT(NULL != si);

    B_Mutex_Lock(si->mutex);

    /* create the scheduler */
    si->scheduler = B_Scheduler_Create(NULL);

    if (NULL == si->scheduler)
    {
        return B_ERROR_UNKNOWN;
    }

    /* create thread to run scheduler */
    B_Thread_GetDefaultSettings(&settingsThread);
    si->schedulerThread = B_Thread_Create("SI_Scheduler", 
                                          (B_ThreadFunc)B_Scheduler_Run, 
                                          si->scheduler, 
                                          &settingsThread);

    if (NULL == si->schedulerThread)
    {
        return B_ERROR_UNKNOWN;
    }
    
    B_Mutex_Unlock(si->mutex);
    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
    Cancels the given PSIP applib job.

Description:
    Cancels the given PSIP applib job.
    The original calling code will be notified of the job cancellation.

Input:
    si                      Handle to PSIP instance 

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_CancelJob(
    B_PSIP_Handle        si,
    B_PSIP_Job         * pJob
    )
{
    B_PSIP_CollectionRequest * pRequest = NULL;

    BDBG_ASSERT(NULL != si);
    BDBG_ASSERT(NULL != pJob);

    /* cancel table timeout if set */
    pRequest = &pJob->request;

    if (B_PSIP_eCollectStop != pRequest->cmd)
    {
        /* stop data collection */
        pRequest->cmd = B_PSIP_eCollectStop;
        si->CollectionFunc(pRequest, si->CollectionContext);
    }

    if (NULL != pJob->tableTimeout)
    {
        B_Scheduler_CancelTimer(si->scheduler, pJob->tableTimeout);
    }

    if (NULL != pJob->eventHandle)
    {
        /* clean up */
        B_Scheduler_UnregisterEvent(si->scheduler, pJob->schedulerEventId);
        B_Event_Destroy(pJob->eventHandle);
        pJob->eventHandle = NULL;
    }

    /* api timeout will be cancelled and pJob will be freed automatically as the 
       B_PSIP_eStateEvent_Cancel propogates thru the state machine */
    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Cancel);

    BDBG_MSG(("!!!!! PSIP Job cancelled (job:%x filterhandle:%x) !!!!!", 
              pJob, pJob->apiSettings.filterHandle));
}

/***************************************************************************
Summary:
    Stops the PSIP applib from servicing any new incomming PSIP requests.

Description:
    Stop the PSIP applib internal worker thread.  This will also cancel any
    pending API jobs.

Input:
    si                      Handle to PSIP instance 

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Stop(
    B_PSIP_Handle si 
    )
{
    B_PSIP_Job * pJob = NULL;

    BDBG_ASSERT(NULL != si);
    BDBG_ASSERT(NULL != si->schedulerThread);
    BDBG_ASSERT(NULL != si->scheduler);

    B_Mutex_Lock(si->mutex);

    /* order of function calls very important */

    while (NULL != (pJob = BLST_S_FIRST(&si->jobList))) 
    {
        B_PSIP_CancelJob(si, pJob);
    }
    
    /* stop scheduler and exit associated thread */
    B_Scheduler_Stop(si->scheduler);

    if (si->scheduler)
    {
    B_Scheduler_Destroy(si->scheduler);
    si->scheduler = NULL;
    }

    if (si->schedulerThread)
    {
        B_Thread_Destroy(si->schedulerThread);
        si->schedulerThread = NULL;
    }

    B_Mutex_Unlock(si->mutex);
}

/***************************************************************************
Summary:
    Cancel the currently executing job.

Description:
    Cancel the currently executing job associated with the given 
    filterHandle.  If the job has already completed, then this API will do
    nothing.
    

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
                            Note: the siHandle and filterHandle are the only
                            2 struct values that are necessary.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Cancel(
    B_PSIP_ApiSettings * pApiSettings
    )
{
    B_PSIP_Handle              si   = NULL;
    B_PSIP_Job               * pJob = NULL;

    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);

    si = pApiSettings->siHandle;

    B_Mutex_Lock(si->mutex);

    pJob = B_PSIP_FindJob(si, pApiSettings->filterHandle);

    if (NULL != pJob) 
    {
        B_PSIP_CancelJob(si, pJob);
    }

    B_Mutex_Unlock(si->mutex);
}

/***************************************************************************
Summary:
    Asynchronous API to get the current UTC time, local offset, 
    local offset source, and Daylight Savings Time indicator.

Description:
    Request the current UTC time, local offset, local offset source, and
    Daylight Savings Time indicator. This call will return before the data
    is actually retrieved.  pApiSettings->dataReadyCallback() will be called 
    when the requested data has been received.
    

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pUtcSecs                Storage for Current UTC time in seconds.
    pLocalOffset            Storage for Local time offset from UTC time.
    pLocalOffsetSource      Storage for Local offset source storage.
    pDst                    Storage for Daylight savings time flag.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If time requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetTime(
    B_PSIP_ApiSettings      * pApiSettings,
    time_t                  * pUtcSecs,            
    int                     * pLocalOffset,       
    B_PSIP_TimeSource       * pLocalOffsetSource,
    bool                    * pDst              
    )
{
    B_PSIP_Handle              si       = NULL;
    B_Error                    retVal   = B_ERROR_SUCCESS;
    B_PSIP_Job               * pJob     = NULL;
    bool                       jobNew   = false;

    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pUtcSecs);
    BDBG_ASSERT(NULL != pLocalOffset);
    BDBG_ASSERT(NULL != pDst);

    si = pApiSettings->siHandle;

    B_Mutex_Lock(si->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    pJob = B_Os_Calloc(1, sizeof(B_PSIP_Job));
    if (NULL == pJob)
    {
        BDBG_WRN(("Out of memory (%s:%d)!", __FUNCTION__, __LINE__));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    if (B_ERROR_SUCCESS == retVal)
    {
        memset(pJob, 0, sizeof(B_PSIP_Job));
        pJob->eventHandle = B_Event_Create(NULL);
        memcpy(&pJob->apiSettings, pApiSettings, sizeof(B_PSIP_ApiSettings));

        if (NULL != pJob->eventHandle)
        {
            pJob->stateData.getTimeData.sttBuffer          = B_Os_Calloc(MGT_BUF_LEN, sizeof(uint8_t));
            pJob->stateData.getTimeData.sttBufferLen       = VCT_BUF_LEN;
            pJob->stateData.getTimeData.pUtcSecs           = pUtcSecs;
            pJob->stateData.getTimeData.pLocalOffset       = pLocalOffset;
            pJob->stateData.getTimeData.pLocalOffsetSource = pLocalOffsetSource;
            pJob->stateData.getTimeData.pDst               = pDst;

            if (NULL != pJob->stateData.getTimeData.sttBuffer) 
            {
                /* add job to list of all pending jobs */
                BLST_S_INSERT_HEAD(&pApiSettings->siHandle->jobList, pJob, node);
                jobNew = true;

                if (0 == pApiSettings->timeout)
                {
                    pApiSettings->timeout = B_PSIP_GETTIME_TIMEOUT;
                }

                /* start api timeout */
                pJob->apiTimeout =
                    B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                           pApiSettings->siHandle->mutex, 
                                           pApiSettings->timeout,
                                           B_PSIP_TimeoutAPI,
                                           &pJob->request);

                /* advance state machine */
                CHANGE_STATE(pJob, B_PSIP_eState_GetTime_GetSTT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                BDBG_WRN(("unable to allocate a stt buffer"));
                retVal = B_ERROR_OUT_OF_MEMORY;
            }
        }
        else
        {
            BDBG_WRN(("unable to create an event"));
            retVal = B_ERROR_UNKNOWN;
        }
    }
    else
    {
        BDBG_WRN(("unable to create job"));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    /* clean up memory allocations on any error condition */
    if (B_ERROR_SUCCESS != retVal)
    {
        if (jobNew) 
            BLST_S_REMOVE_HEAD(&pApiSettings->siHandle->jobList, node);

        if (pJob)
        {
	    if (pJob->eventHandle)                          B_Event_Destroy(pJob->eventHandle);
	    if (pJob->stateData.getTimeData.sttBuffer)      B_Os_Free(pJob->stateData.getTimeData.sttBuffer);

            B_Os_Free(pJob);
            pJob = NULL;
        }

    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API to get the versioning info for the currently available
    PSIP tables.

Description:
    Get the versioning info for the currently available PSIP tables.  
    This can be used to quickly determine which PSIP tables have changed.
    pLength should indicate the maximum size of the verList (for 
    terrestrial this may range from 6 to 370.  for cable this may range
    from 2 to 370. Calling code must also fill in the verList[].tableType 
    field for each requested table version to be retrieved.  This allows 
    full control over which exact table versions to compare.  The 
    verList[].changed field will indicate if the associated tableType 
    has changed in the MGT.  

    This call will return before the version info is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the version
    info has been received.

    This API can also be called to retrieve the current pids and version for
    each requested table type.  The calling code must first set the 
    table types it is interested in, and set each pid to 0. The table 
    version data, and pid will be copied from the current MGT and marked 
    as changed.

    A typical use-case would be to call this API the first time with each
    node in the list having a pid of 0.  That same verList can be passed in
    on subsequent calls to obtain table change information.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    verList                 List of PSIP tables to monitor for version
                            changes.
    length                  Length of verList

Output:
    None

Returns:
	B_ERROR_SUCCESS         If table versions requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetTableVersions(
    B_PSIP_ApiSettings      * pApiSettings,
    B_PSIP_TableVersion     * verList,
    uint16_t                  length)
{
    B_Error                    retVal   = B_ERROR_SUCCESS;
    B_PSIP_Job               * pJob     = NULL;
    bool                       jobNew   = false;
    int                        i        = 0;

    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != verList);
    BDBG_ASSERT(0     < length);

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    for (i = 0; i < length; i++)
    {
        verList[i].changed = false;
    }

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    pJob = B_Os_Calloc(1, sizeof(B_PSIP_Job));
    if (NULL == pJob)
    {
        BDBG_WRN(("Out of memory (%s:%d)!", __FUNCTION__, __LINE__));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    if (B_ERROR_SUCCESS == retVal)
    {
        memset(pJob, 0, sizeof(B_PSIP_Job));
        pJob->eventHandle = B_Event_Create(NULL);
        memcpy(&pJob->apiSettings, pApiSettings, sizeof(B_PSIP_ApiSettings));

        if (NULL != pJob->eventHandle)
        {
            pJob->stateData.getTableVerData.mgtBuffer    = B_Os_Calloc(MGT_BUF_LEN, sizeof(uint8_t));
            pJob->stateData.getTableVerData.mgtBufferLen = VCT_BUF_LEN;
            pJob->stateData.getTableVerData.verList      = verList;
            pJob->stateData.getTableVerData.length       = length;

            if (NULL != pJob->stateData.getTableVerData.mgtBuffer) 
            {
                /* add job to list of all pending jobs */
                BLST_S_INSERT_HEAD(&pApiSettings->siHandle->jobList, pJob, node);
                jobNew = true;

                if (0 == pApiSettings->timeout)
                {
                    pApiSettings->timeout = B_PSIP_GETTABLEVERSIONS_TIMEOUT;
                }

                /* start api timeout */
                pJob->apiTimeout =
                    B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                           pApiSettings->siHandle->mutex, 
                                           pApiSettings->timeout,
                                           B_PSIP_TimeoutAPI,
                                           &pJob->request);

                /* advance state machine */
                CHANGE_STATE(pJob, B_PSIP_eState_GetTableVer_GetMGT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                BDBG_WRN(("unable to allocate a mgt buffer"));
                retVal = B_ERROR_OUT_OF_MEMORY;
            }
        }
        else
        {
            BDBG_WRN(("unable to create an event"));
            retVal = B_ERROR_UNKNOWN;
        }
    }
    else
    {
        BDBG_WRN(("unable to create job"));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    /* clean up memory allocations on any error condition */
    if (B_ERROR_SUCCESS != retVal)
    {
        if (jobNew) 
            BLST_S_REMOVE_HEAD(&pApiSettings->siHandle->jobList, node);

        if (pJob)
        {
            if (pJob->eventHandle)                          B_Event_Destroy(pJob->eventHandle);
            if (pJob->stateData.getTableVerData.mgtBuffer)  B_Os_Free(pJob->stateData.getTableVerData.mgtBuffer);

            B_Os_Free(pJob);
            pJob = NULL;
        }
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
    return retVal;
}


/***************************************************************************
Summary:
PRIVATE API to get the current channel list.  pLength should indicate the 
    maximum size of the chList.  Upon return pLength will indicate the 
    number of channels found and stored to chList.  if *pLength == 0
    then the chList is NOT populated with data - only pLength is 
    updated.
***************************************************************************/
static B_Error B_PSIP_GetChannelList_P(
    B_PSIP_ApiSettings     * pApiSettings,
    B_PSIP_Channel         * chList,
    uint16_t               * pLength,
    B_PSIP_ChannelListType   type
    )
{
    B_Error                    retVal   = B_ERROR_SUCCESS;
    B_PSIP_Job               * pJob     = NULL;
    bool                       jobNew   = false;

    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != chList);
    BDBG_ASSERT(NULL != pLength);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    pJob = B_Os_Calloc(1, sizeof(B_PSIP_Job));
    if (NULL == pJob)
    {
        BDBG_WRN(("Out of memory (%s:%d)!", __FUNCTION__, __LINE__));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    if (B_ERROR_SUCCESS == retVal)
    {
        memset(pJob, 0, sizeof(B_PSIP_Job));
        pJob->eventHandle = B_Event_Create(NULL);
        memcpy(&pJob->apiSettings, pApiSettings, sizeof(B_PSIP_ApiSettings));

        if (NULL != pJob->eventHandle)
        {
            pJob->stateData.getChListData.vctBuffer    = B_Os_Calloc(VCT_BUF_LEN, sizeof(uint8_t));
            pJob->stateData.getChListData.vctBufferLen = VCT_BUF_LEN;
            pJob->stateData.getChListData.patBuffer    = B_Os_Calloc(PAT_BUF_LEN, sizeof(uint8_t));
            pJob->stateData.getChListData.patBufferLen = PAT_BUF_LEN;
            pJob->stateData.getChListData.pmtBuffer    = B_Os_Calloc(PMT_BUF_LEN, sizeof(uint8_t));
            pJob->stateData.getChListData.pmtBufferLen = PMT_BUF_LEN;
            pJob->stateData.getChListData.chList       = chList;
            pJob->stateData.getChListData.pLength      = pLength;
            pJob->stateData.getChListData.type         = type;

            if ((NULL != pJob->stateData.getChListData.vctBuffer) &&
                (NULL != pJob->stateData.getChListData.patBuffer) &&
                (NULL != pJob->stateData.getChListData.pmtBuffer))
            {
                /* add job to list of all pending jobs */
                BLST_S_INSERT_HEAD(&pApiSettings->siHandle->jobList, pJob, node);
                jobNew = true;

                if (0 == pApiSettings->timeout)
                {
                    pApiSettings->timeout = B_PSIP_GETCHANNELLIST_TIMEOUT;
                }

                /* start api timeout */
                pJob->apiTimeout =
                    B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                           pApiSettings->siHandle->mutex, 
                                           pApiSettings->timeout,
                                           B_PSIP_TimeoutAPI,
                                           &pJob->request);

                /* advance state machine */
                CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetVCT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                BDBG_WRN(("unable to allocate a vct/pat/pmt buffer"));
                retVal = B_ERROR_OUT_OF_MEMORY;
            }
        }
        else
        {
            BDBG_WRN(("unable to create an event"));
            retVal = B_ERROR_UNKNOWN;
        }
    }
    else
    {
        BDBG_WRN(("unable to create job"));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    /* clean up memory allocations on any error condition */
    if (B_ERROR_SUCCESS != retVal)
    {
        if (jobNew)
            BLST_S_REMOVE_HEAD(&pApiSettings->siHandle->jobList, node);

        if (pJob)
        {
            if (pJob->eventHandle)                        B_Event_Destroy(pJob->eventHandle);
            if (pJob->stateData.getChListData.vctBuffer)  B_Os_Free(pJob->stateData.getChListData.vctBuffer);
            if (pJob->stateData.getChListData.patBuffer)  B_Os_Free(pJob->stateData.getChListData.patBuffer);
            if (pJob->stateData.getChListData.pmtBuffer)  B_Os_Free(pJob->stateData.getChListData.pmtBuffer);

            B_Os_Free(pJob);
            pJob = NULL;
        }
    }

    return retVal;
}

/***************************************************************************
Summary:
    Asynchronous API to get the number of channels in the current channel 
    list.

Description:
    Counts the number of channels in the current channel list.  Channel 
    data such as PIDs, names, etc, are not saved.

    This call will return before the number of channels are actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the number of channels has been counted.

    This function is not very efficient when combined with 
    B_PSIP_GetChannelList() because a VCT scan is performed twice.  
    The only way to determine the number of channels is to traverse all 
    sections  of the VCT.  In nearly all cases, it is better to simply 
    allocate enough memory to hold a large channel list and just call 
    B_PSIP_GetChannelList().

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pNumChannels            Storage for total number of channels
    type                    Terrestrial or Cable channel list.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If number of channels requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
void B_PSIP_GetNumChannels(
    B_PSIP_ApiSettings     * pApiSettings,
    uint16_t               * pNumChannels,
    B_PSIP_ChannelListType   type
    )
{
    B_PSIP_Channel ch;  /* dummy channel */

    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pNumChannels);

    B_Mutex_Lock(pApiSettings->siHandle->mutex);
    /* force list size to 0 so no channel data will be captured. only count 
       number of valid channels */
    *pNumChannels = 0;
    B_PSIP_GetChannelList_P(pApiSettings, &ch, pNumChannels, type);
    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
}

/***************************************************************************
Summary:
    Asynchronous API to get the current channel list.

Description:
    Retrieves the current channel list.  

    This call will return before the channel list is actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the channel list has been counted.

    ***Important Note for Cable Channel Lists 
    For (B_PSIP_eCable == type), major/minor channel numbers
    must be interpreted properly.  if the major/minor channel numbers 
    are below 1000, then the major/minor pair defines a 2 part channel
    number.  if the 6 most sig bits of the major channel number are
    111111, then use the following formula to calculate the 1 part
    channel number: onePartNumber = (major & 0x00F)<<10 + minor; 

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    chList                  Storage for list of channels.
    pLength                 Size of chList.
    type                    Terrestrial or Cable channel list.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If number of channels requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
void B_PSIP_GetChannelList(
    B_PSIP_ApiSettings     * pApiSettings,
    B_PSIP_Channel         * chList,
    uint16_t               * pLength,
    B_PSIP_ChannelListType   type
    )
{
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != chList);
    BDBG_ASSERT(NULL != pLength);

    B_Mutex_Lock(pApiSettings->siHandle->mutex);
    B_PSIP_GetChannelList_P(pApiSettings, chList, pLength, type);
    B_Mutex_Unlock(pApiSettings->siHandle->mutex);
}

/***************************************************************************
Summary:
    Asynchronous API to get the program information associated with the 
    given channel and time.

Description:
    Retrieves the program information associated with the given channel
    and time.

    This call will return before the program data is actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the program data has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pChannel                Channel associated with desired program data.
    pProgram                Storage for program data
    pLength                 Number of programs that can be stored in pProgram.
                            Variable will contain the number of programs saved
                            saved in pProgram when API call completes.
    timeUTCStart            Start time associated with desired program data.
                            If timeUTC == 0, then current time will be used.
    timeUTCEnd              End time associated with desired program data.
                            If timeUTC == 0, then current time will be used.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If program information requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetProgram(
    B_PSIP_ApiSettings * pApiSettings,
    B_PSIP_Channel     * pChannel,
    B_PSIP_Program     * pProgram,
    uint16_t           * pLength,
    time_t               timeUTCStart,
    time_t               timeUTCEnd
    )
{
    B_Error                        retVal   = B_ERROR_SUCCESS;
    B_PSIP_Job                   * pJob     = NULL;
    bool                           jobNew   = false;
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL !=  pApiSettings);
    BDBG_ASSERT(NULL !=  pApiSettings->siHandle);
    BDBG_ASSERT(NULL !=  pApiSettings->filterHandle);
    BDBG_ASSERT(NULL !=  pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL !=  pChannel);
    BDBG_ASSERT(NULL !=  pProgram);
    BDBG_ASSERT(0    <  *pLength);

    B_Mutex_Lock(pApiSettings->siHandle->mutex);

    /* search for given filterHandle in current list of pending jobs */
    if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
    {
        BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
        retVal = B_ERROR_PSIP_BUSY;
    }

    pJob = B_Os_Calloc(1, sizeof(B_PSIP_Job));
    if (NULL == pJob)
    {
        BDBG_WRN(("Out of memory (%s:%d)!", __FUNCTION__, __LINE__));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    if (B_ERROR_SUCCESS == retVal)
    {
        pStateData = &pJob->stateData.getProgInfoData;

        memset(pJob, 0, sizeof(B_PSIP_Job));
        pJob->eventHandle = B_Event_Create(NULL);
        memcpy(&pJob->apiSettings, pApiSettings, sizeof(B_PSIP_ApiSettings));

        if (NULL != pJob->eventHandle)
        {
            pStateData->sttBuffer    = B_Os_Calloc(STT_BUF_LEN, sizeof(uint8_t));
            pStateData->sttBufferLen = STT_BUF_LEN;
            pStateData->vctBuffer    = B_Os_Calloc(VCT_BUF_LEN, sizeof(uint8_t));
            pStateData->vctBufferLen = VCT_BUF_LEN;
            pStateData->eitBuffer    = B_Os_Calloc(EIT_BUF_LEN, sizeof(uint8_t));
            pStateData->eitBufferLen = EIT_BUF_LEN;
            pStateData->ettBuffer    = B_Os_Calloc(ETT_BUF_LEN, sizeof(uint8_t));
            pStateData->ettBufferLen = ETT_BUF_LEN;
            pStateData->mgtBuffer    = B_Os_Calloc(MGT_BUF_LEN, sizeof(uint8_t));
            pStateData->mgtBufferLen = MGT_BUF_LEN;
            pStateData->progList     = pProgram;
            pStateData->pLength      = pLength;
            pStateData->timeUTCStart = timeUTCStart;
            pStateData->timeUTCEnd   = timeUTCEnd;
            memcpy(&pStateData->currentCh, pChannel, sizeof(B_PSIP_Channel));

            if ((NULL != pStateData->sttBuffer) &&
                (NULL != pStateData->vctBuffer) &&
                (NULL != pStateData->eitBuffer) &&
                (NULL != pStateData->ettBuffer) &&
                (NULL != pStateData->mgtBuffer))
            {
                /* add job to list of all pending jobs */
                BLST_S_INSERT_HEAD(&pApiSettings->siHandle->jobList, pJob, node);
                jobNew = true;

                if (0 == pApiSettings->timeout)
                {
                    pApiSettings->timeout = B_PSIP_GETPROGRAM_TIMEOUT;
                }

                /* start api timeout */
                pJob->apiTimeout =
                    B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                           pApiSettings->siHandle->mutex, 
                                           pApiSettings->timeout,
                                           B_PSIP_TimeoutAPI,
                                           &pJob->request);

                /* advance state machine */
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetSTT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                BDBG_WRN(("unable to allocate a vct/eit/ett/mgt buffer"));
                retVal = B_ERROR_OUT_OF_MEMORY;
            }
        }
        else
        {
            BDBG_WRN(("unable to create an event"));
            retVal = B_ERROR_UNKNOWN;
        }
    }
    else
    {
        BDBG_WRN(("unable to create job"));
        retVal = B_ERROR_OUT_OF_MEMORY;
    }

    /* clean up memory allocations on any error condition */
    if (B_ERROR_SUCCESS != retVal)
    {
        if (jobNew)                 
            BLST_S_REMOVE_HEAD(&pApiSettings->siHandle->jobList, node);

        if (pJob)
        {
            if (pJob->eventHandle)                              B_Event_Destroy(pJob->eventHandle);
            if (pStateData != NULL)
            {
                if (pJob->stateData.getProgInfoData.vctBuffer)  B_Os_Free(pStateData->vctBuffer);
                if (pJob->stateData.getProgInfoData.eitBuffer)  B_Os_Free(pStateData->eitBuffer);
                if (pJob->stateData.getProgInfoData.ettBuffer)  B_Os_Free(pStateData->ettBuffer);
                if (pJob->stateData.getProgInfoData.mgtBuffer)  B_Os_Free(pStateData->mgtBuffer);
            }
            B_Os_Free(pJob);
            pJob = NULL;
        }
    }

    B_Mutex_Unlock(pApiSettings->siHandle->mutex);

    return retVal;
}

