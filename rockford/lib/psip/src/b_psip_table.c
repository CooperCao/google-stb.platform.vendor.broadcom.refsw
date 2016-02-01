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
*    Library routines for getting PSIP tables.
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
#include "b_psip_utils.h"
#include "b_psip_stateengine.h"

/***************************************************************************
Summary:
Table specific settings for table data retrieval.
***************************************************************************/
typedef struct B_PSIP_TableSettings
{
    uint16_t                   pid;                     /* PID to collection data on */
    B_PSIP_Filter              filter;                  /* Filter for received section data */
    B_EventCallback            processingCallback;      /* Scheduler will call this when the requested data arrives */
    B_PSIP_DataReadyCallback   dataReadyCallback;       /* This will be called when section filter data is ready to be retrieved */
    B_TimerCallback            tableTimeoutCallback;    /* Scheduler will call this when the table request timeout expires */
	uint8_t                  * pBuffer;                 /* Buffer for PSIP table data */
    uint32_t                 * pBufferLength;           /* Size of pBuffer data.  Will contains the size of the retrieved data on return */
    uint32_t                   timeout;                 /* Timeout for table acquisition */
} B_PSIP_TableSettings;

/***************************************************************************
Summary:
    Callback which is called when previously requested section filter data
    is available for retrieval.

Description:
    Callback which is called when previously requested section filter data
    is available for retrieval.  This routine should do a minimum amount of
    work.

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_DataReady(void * context)
{
    BDBG_ASSERT(NULL != context);
    /* set event to trigger PSIP scheduler to begin processing data */
    B_Event_Set((B_EventHandle)context);
}

/***************************************************************************
Summary:
    This callback function is called when table data times out.  
    param contains a pointer to the original request.

Description:
    Table timeout callback.  Table timeouts differ from API timeouts in 
    that they only apply to acquisition of a particular table.  
    API timeouts apply to all the tables that are necessary to complete 
    the API request.

    This is a scheduler based callback so the si mutex is automatically
    set on entry and reset on exit.

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_TimeoutTable(void * param)
{
    B_PSIP_CollectionRequest * pRequest = (B_PSIP_CollectionRequest *)param;
    B_PSIP_Handle              si       = pRequest->si;

    /* search list for job (even tho we already have a pointer to it) */
    B_PSIP_Job * pJob = B_PSIP_FindJob(si, pRequest->filterHandle);
    if (pJob)
    {
        BDBG_MSG(("#### TABLE TIMEOUT! pid:0x%04x table_id:0x%02x", 
                  pRequest->pid,
                  pRequest->filter.coef[0]));

        if (B_PSIP_eCollectStop != pRequest->cmd)
        {
            /* stop data collection */
            pRequest->cmd = B_PSIP_eCollectStop;
            si->CollectionFunc(pRequest, si->CollectionContext);
        }

        pRequest->pBufferLength = 0;

        /* api timeout is automatically cancelled by state machine "done" states */

        if (pJob->eventHandle)
        {
            /* clean up */
            B_Scheduler_UnregisterEvent(si->scheduler, pJob->schedulerEventId);
            B_Event_Destroy(pJob->eventHandle);
            pJob->eventHandle = NULL;
        }

        /* advance state engine */
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_TableTimeout);
    }
    else
    {
        BDBG_WRN(("### table timeout received, but no corresponding job exists in list!"));
    }
}

/***************************************************************************
Summary:
    This callback function is called when table data is received.  param 
    contains a pointer to the original request.

Description:
    This callback function is called when table data is received.  param 
    contains a pointer to the original request.

    This is a scheduler based callback so the si mutex is automatically
    set on entry and reset on exit.

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessTable(void * param)
{
    B_PSIP_CollectionRequest * pRequest = NULL;
    B_PSIP_Handle              si       = NULL;
    B_Error                    status   = B_ERROR_SUCCESS;
    B_PSIP_Job               * pJob     = NULL;

    BDBG_ASSERT(NULL != param);

    pRequest = (B_PSIP_CollectionRequest *)param;
    si       = pRequest->si;

    pJob = B_PSIP_FindJob(si, pRequest->filterHandle);

    if (NULL == pJob)
    {
        /* if there is no matching job, we can't do anything so ignore it */
        BDBG_WRN(("Received data ready callback from application code for an unknown filterhandle - ignoring"));
        return;
    }

    /* copy received data to buffer */
    pRequest->cmd = B_PSIP_eCollectCopyBuffer;
    status        = si->CollectionFunc(pRequest, si->CollectionContext);
    if (B_ERROR_PSIP_RETRY == status)
    {
        /* problem getting data buffer - app will retry later */
#if 0
        BDBG_MSG(("%s: Error collecting data from application - RETRYING", __FUNCTION__));
#endif
        return;
    }
    else
    if (B_ERROR_SUCCESS != status)
    {
        /* problem getting data buffer so we'll call it a table timeout */
        BDBG_MSG(("%s: Error collecting data from application - TIMEOUT", __FUNCTION__));
        B_PSIP_TimeoutTable(param);
    }
    else
    {
        /* success getting data from app! */

        /* cancel table timeout */
        B_Scheduler_CancelTimer(si->scheduler, pJob->tableTimeout);

        if (B_PSIP_eCollectStop != pRequest->cmd)
        {
            /* stop data collection */
            pRequest->cmd = B_PSIP_eCollectStop;
            status        = si->CollectionFunc(pRequest, si->CollectionContext);
        }

        if (B_ERROR_SUCCESS != status)
        {
            BDBG_ERR(("Error stopping data collection!"));
        }

        if (pJob->eventHandle)
        {
            /* clean up */
            B_Scheduler_UnregisterEvent(si->scheduler, pJob->schedulerEventId);
            B_Event_Destroy(pJob->eventHandle);
            pJob->eventHandle = NULL;
        }

        STATE_ENGINE(pJob, B_PSIP_eStateEvent_TableReceived);
    }
}

/***************************************************************************
Summary:
    Generic function to retrieve a PSIP table.

Description:
    Generic function to retrieve a PSIP table.  This function provide an
    interface to request any PSIP table.  It is called by the 
    B_PSIP_GetXXX_P() functions in this file.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_Error B_PSIP_GetTable(
    B_PSIP_ApiSettings    * pApiSettings,
    B_PSIP_TableSettings  * pTableSettings,
    B_PSIP_Job            * pJob
    )
{
    B_Error                    retVal   = B_ERROR_SUCCESS;
    bool                       jobNew   = false;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pTableSettings);
    BDBG_ASSERT(NULL != pTableSettings->processingCallback);
    BDBG_ASSERT(NULL != pTableSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pTableSettings->tableTimeoutCallback);
    BDBG_ASSERT(NULL != pTableSettings->pBuffer);
    BDBG_ASSERT(NULL != pTableSettings->pBufferLength);
    BDBG_ASSERT(0     < *(pTableSettings->pBufferLength));

    /* if not given an existing pJob, then create a new one */
    if (NULL == pJob)
    {
        BDBG_MSG(("*** creating a new job!"));
        /* search for given filterHandle in current list of pending jobs */
        if (NULL != B_PSIP_FindJob(pApiSettings->siHandle, pApiSettings->filterHandle))
        {
            BDBG_WRN(("You can only call one data retrieval API at a time for each provided filterHandle."));
            retVal = B_ERROR_PSIP_BUSY;
        }

        pJob = B_Os_Calloc(1, sizeof(B_PSIP_Job));
        if (pJob)
        {
            jobNew = true;
            memset(pJob, 0, sizeof(B_PSIP_Job));

            pJob->stateData.getTableData.pBuffer    = pTableSettings->pBuffer;
            pJob->stateData.getTableData.pBufferLen = pTableSettings->pBufferLength;

            /* add job to list of all pending jobs */
            BLST_S_INSERT_HEAD(&pApiSettings->siHandle->jobList, pJob, node);
        }
        else
        {
            BDBG_WRN(("Out of memory (%s:%d)!", __FUNCTION__, __LINE__));
            retVal = B_ERROR_OUT_OF_MEMORY;
        }
    }

    if (B_ERROR_SUCCESS == retVal)
    {
        B_PSIP_GetDefaultCollectionData(&pJob->request);
        pJob->request.si             = pApiSettings->siHandle;
        pJob->request.pid            = pTableSettings->pid;
        pJob->request.filterHandle   = pApiSettings->filterHandle;
        pJob->request.pBuffer        = pTableSettings->pBuffer;
        pJob->request.pBufferLength  = pTableSettings->pBufferLength;
        memcpy(&pJob->request.filter, &pTableSettings->filter, sizeof(B_PSIP_Filter));

        pJob->eventHandle = B_Event_Create(NULL);
        BDBG_MSG(("NEW event handle:%p", pJob->eventHandle));
        memcpy(&pJob->apiSettings, pApiSettings, sizeof(B_PSIP_ApiSettings));

        if (NULL != pJob->eventHandle)
        {
            /* register event/callback with scheduler 
               when pJob->eventHandle is set, call processingCallback with the
               pJob as a param.  note that the calling code's 
               "data ready" callback and param have been saved to the job list - 
               this callback will be called once the current state machine 
               reaches it's "done" state.
             */
            pJob->schedulerEventId =            
                B_Scheduler_RegisterEvent(pApiSettings->siHandle->scheduler, 
                                          pApiSettings->siHandle->mutex, 
                                          pJob->eventHandle, 
                                          pTableSettings->processingCallback,
                                          &pJob->request);


            if (NULL != pJob->schedulerEventId)
            {
                pJob->request.cmd           = B_PSIP_eCollectStart;
                pJob->request.callback      = pTableSettings->dataReadyCallback; /* B_PSIP_DataReady; */
                pJob->request.callbackParam = pJob->eventHandle;

                /* start collection of data */
                pApiSettings->siHandle->CollectionFunc(&pJob->request, pApiSettings->siHandle->CollectionContext);

                /* set table timeout */
                pJob->tableTimeout = 
                    B_Scheduler_StartTimer(pApiSettings->siHandle->scheduler,
                                           pApiSettings->siHandle->mutex, 
                                           pTableSettings->timeout,
                                           pTableSettings->tableTimeoutCallback, /* B_PSIP_TimeoutTable, */
                                           &pJob->request);
            }
            else
            {
                BDBG_WRN(("unable to register event with scheduler"));
                retVal = B_ERROR_UNKNOWN;
            }
        }
        else
        {
            BDBG_WRN(("unable to create an event"));
            retVal = B_ERROR_UNKNOWN;
        }
    }
    
    /* clean up memory allocations on any error condition */
    if (B_ERROR_SUCCESS != retVal)
    {
        if (pJob)
        {
            if (pJob->schedulerEventId)
            {
                B_Scheduler_UnregisterEvent(pApiSettings->siHandle->scheduler, pJob->schedulerEventId);
                pJob->schedulerEventId = NULL;
            }

            if (pJob->eventHandle)
            {
                B_Event_Destroy(pJob->eventHandle);
                pJob->eventHandle = NULL;
            }

            if (jobNew)
            {
                /* only delete the job if it was newly created */
                BLST_S_REMOVE_HEAD(&pApiSettings->siHandle->jobList, node);
                B_Os_Free(pJob);
                pJob = NULL;
            }
        }
    }

    return retVal;
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Program Allocation
    Table (PAT).  Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetPAT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    BDBG_ENTER(B_PSIP_GetPAT_P);
    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x0;    /* PAT PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
	tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.PatTimeout;

    BDBG_LEAVE(B_PSIP_GetPAT_P);
    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve a Program Map Table (PMT) 
    associated with the given pid. Adds option to acquire table data using 
    an existing job. If pJob is NULL, then a new job will be automatically 
    created.  Note that unlike the public API, this function assumes the 
    si mutex is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetPMT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = pid;    /* PMT PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0x02;   /* PMT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.PmtTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Master Guide Table (MGT)
    Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetMGT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xC7;   /* MGT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;   /* protocol version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.MgtTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the PSIP Terrestrial 
    Virtual Channel Table (TVCT).  Adds option to acquire table data 
    using an existing job. If pJob is NULL, then a new job will be 
    automatically created.  Note that unlike the public API, this 
    function assumes the si mutex is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetTVCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xC8;    /* TVCT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[5]     = 0x01;    /* current table only */
    tableSettings.filter.mask[5]     = 0xFE;
    tableSettings.filter.excl[5]     = 0xFF;
    tableSettings.filter.coef[6]     = section;
    tableSettings.filter.mask[6]     = 0x00;
    tableSettings.filter.excl[6]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;    /* protocol_version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.TvctTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the PSIP Cable Virtual Channel 
    Table (CVCT).  Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetCVCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xC9;    /* CVCT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[5]     = 0x01;    /* current table only */
    tableSettings.filter.mask[5]     = 0xFE;
    tableSettings.filter.excl[5]     = 0xFF;
    tableSettings.filter.coef[6]     = section;
    tableSettings.filter.mask[6]     = 0x00;
    tableSettings.filter.excl[6]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;    /* protocol_version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.CvctTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the System Time Table (STT).
    Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetSTT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xCD;   /* STT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;   /* protocol version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.SttTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Event Information 
    Table (EIT) Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetEIT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = pid;     /* EIT PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xCB;    /* Table ID for EIT */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[3]     = (uint8_t)((sourceId >> 8) & 0xFF);    /* source id */
    tableSettings.filter.mask[3]     = 0x00;
    tableSettings.filter.excl[3]     = 0xff;
    tableSettings.filter.coef[4]     = (uint8_t)(sourceId & 0xFF);    /* source id */
    tableSettings.filter.mask[4]     = 0x00;
    tableSettings.filter.excl[4]     = 0xff;
    tableSettings.filter.coef[6]     = section;
    tableSettings.filter.mask[6]     = 0x00;
    tableSettings.filter.excl[6]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;    /* protocol_version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xff;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.EitTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Extended Text Table (ETT)
    Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetETT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint16_t                eventId,
    B_PSIP_EtmIdType        etmType,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = pid;     /* ETT PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xCC;   /* ETT */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;    /* protocol_version */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;
    /* search using ETM id (see spec for actual format) */
    tableSettings.filter.coef[9]     = (uint8_t)((sourceId >> 8) & 0xFF);
    tableSettings.filter.mask[9]     = 0x00;
    tableSettings.filter.excl[9]     = 0xFF;
    tableSettings.filter.coef[10]    = (uint8_t)(sourceId & 0xFF);
    tableSettings.filter.mask[10]    = 0x00;
    tableSettings.filter.excl[10]    = 0xFF;

    if (B_PSIP_eEventETM == etmType)
    {
        tableSettings.filter.coef[11] = (uint8_t)((eventId >> 6) & 0xFF);
        tableSettings.filter.mask[11] = 0x00;
        tableSettings.filter.excl[11] = 0xFF;
        tableSettings.filter.coef[12] = (uint8_t)((((eventId << 2) & 0xFFFC) | 2) & 0xFF);
        tableSettings.filter.mask[12] = 0x00;   
        tableSettings.filter.excl[12] = 0xFF;
        /*tableSettings.filter.coef[12] = 0x02;*/ /* all event ETMs */
        /*tableSettings.filter.mask[12] = 0xFC;*/
        /*tableSettings.filter.excl[12] = 0xFF;*/
    }

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.EttTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Region Rating Table (RRT)
    Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  Note that
    unlike the public API, this function assumes the si mutex is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetRRT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);
    
    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* RRT PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xCA;   /* RRT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;   /* protocol version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.RrtTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Directed Channel Change 
    Table (DCCT). Adds option to acquire table data using an existing job.
    If pJob is NULL, then a new job will be automatically created.  
    Note that unlike the public API, this function assumes the si mutex 
    is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetDCCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xD3;   /* DCCT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;   /* protocol version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.DcctTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}

/***************************************************************************
Summary:
    PRIVATE Asynchronous API used to retrieve the Directed Channel Change 
    Selection Code Table (DCCSCT). Adds option to acquire table data using 
    an existing job. If pJob is NULL, then a new job will be automatically 
    created.  Note that unlike the public API, this function assumes the 
    si mutex is already locked.

Description:
    None

Returns:
    None

See Also:
    None
***************************************************************************/
B_Error B_PSIP_GetDCCSCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    )
{
    B_PSIP_TableSettings tableSettings;

    /* sanity check required parameters */
    BDBG_ASSERT(NULL != pApiSettings);
    BDBG_ASSERT(NULL != pApiSettings->siHandle);
    BDBG_ASSERT(NULL != pApiSettings->filterHandle);
    BDBG_ASSERT(NULL != pApiSettings->dataReadyCallback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferLength);
    BDBG_ASSERT(0     < *pBufferLength);

    memset(&tableSettings, 0, sizeof(B_PSIP_TableSettings));

    tableSettings.pid                = 0x1FFB; /* base PID */

    B_PSIP_GetDefaultCollectionDataFilter(&tableSettings.filter);
    tableSettings.filter.coef[0]     = 0xD4;   /* DCCSCT table id */
    tableSettings.filter.mask[0]     = 0x00;
    tableSettings.filter.excl[0]     = 0xFF;
    tableSettings.filter.coef[8]     = 0x00;   /* protocol version must be 0 for PSIP */
    tableSettings.filter.mask[8]     = 0x00;
    tableSettings.filter.excl[8]     = 0xFF;

    tableSettings.processingCallback   = B_PSIP_ProcessTable;
    tableSettings.dataReadyCallback    = B_PSIP_DataReady;
    tableSettings.tableTimeoutCallback = B_PSIP_TimeoutTable;
    tableSettings.pBuffer              = pBuffer;
    tableSettings.pBufferLength        = pBufferLength;
    tableSettings.timeout              = pApiSettings->siHandle->settings.DccsctTimeout;

    return B_PSIP_GetTable(pApiSettings, &tableSettings, pJob);
}
