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
*    Library routines for PSIP getproginfo state
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

#include "ts_psi.h"
#include "psip_mgt.h"
#include "psip_vct.h"
#include "psip_eit.h"
#include "psip_ett.h"
#include "psip_mss.h"
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
    Get the content advisory descriptors if they exist.

Description:
    Get the content advisory descriptors if they exist. Save the ratings 
    in the given program struct.  

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessContentAdvisoryDescriptor(
    B_PSIP_Job            * pJob,
    TS_PSI_descriptor       eitDesc,
    B_PSIP_Program        * pProgram
    )
{
    PSIP_CAD_rating_region         region;
    PSIP_CAD_rating_dimension      dim;
    B_PSIP_Handle                  si         = NULL;
    int                            regionIdx  = 0;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != eitDesc);
    BDBG_ASSERT(NULL != pProgram);

    si = pJob->apiSettings.siHandle;

    for (regionIdx = 0; regionIdx < PSIP_CAD_getRatingRegionCount(eitDesc); ++regionIdx)
    {
        uint16_t length       = MAX_RATING_STR;
        int      numValidDims = 0;
        int      dimIdx       = 0;

        if (B_ERROR_SUCCESS != PSIP_CAD_getRatingRegion(eitDesc, regionIdx, &region))
        {
            continue;
        }

        BDBG_MSG(("found rating region:%d", region.rating_region));
        if (region.rating_region != si->settings.RatingRegion) 
        {
            continue;
        }

        /* save rating region */
        pProgram->rating.region    = region.rating_region;

        length = MAX_RATING_STR;
        memset(pProgram->rating.description, 0, MAX_RATING_STR);
        if (B_ERROR_SUCCESS == B_PSIP_GetStringFromMSS(si, region.p_rating_description_text, pProgram->rating.description, &length))
        {
            BDBG_MSG(("CAD[%d] %s\n", regionIdx, pProgram->rating.description));
        }

        numValidDims = 0;
        dimIdx       = 0;
        /* save all rating dim/value pairs in pProgram */
        for (dimIdx = 0; dimIdx < region.rated_dimensions; ++dimIdx)
        {
            if (B_ERROR_SUCCESS != PSIP_CAD_getRatingDimension(eitDesc, regionIdx, dimIdx, &dim))
            {
                continue;
            }

            BDBG_WRN(("*** CAD_RD[%d, %d] dim:%d val:%d", regionIdx, dimIdx, dim.rating_dimension_j, dim.rating_value));
            pProgram->rating.dim[numValidDims].ratingDim = dim.rating_dimension_j;
            pProgram->rating.dim[numValidDims].ratingVal = dim.rating_value;
            numValidDims++;
        }

        pProgram->rating.numDims = numValidDims;

        /* already found all ratings for our given region so quit */
        break;
    }
}

/***************************************************************************
Summary:
    Get the AC3 audio descriptors if they exist.

Description:
    Get the AC3 audio descriptors if they exist. Save the descriptors 
    in the given program struct.  

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessAC3AudioDescriptor(
    B_PSIP_Job            * pJob,
    TS_PSI_descriptor       eitDesc,
    B_PSIP_Program        * pProgram
    )
{
    BSTD_UNUSED(pJob);
    BSTD_UNUSED(eitDesc);
    BSTD_UNUSED(pProgram);

    if (pProgram->numAC3AudioDesc < MAX_AC3_DESC)
    {
        /* not sure how useful this descriptor is to applications, so
           just save last received descriptor for given program */
        memcpy(pProgram->AC3Audio[pProgram->numAC3AudioDesc].descriptor, 
               eitDesc, 
               MAX_AC3_AUDIO_DESC_LEN);
        pProgram->numAC3AudioDesc++;
    }
}

/***************************************************************************
Summary:
    Get the closed captioning descriptors if they exist.

Description:
    Get the closed captioning descriptors if they exist. Save the caption
    data in the given program struct.  

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessCaptionServiceDescriptor(
    B_PSIP_Job            * pJob,
    TS_PSI_descriptor       eitDesc,
    B_PSIP_Program        * pProgram
    )
{
    PSIP_CSD_service service;
    int              numValidServices = 0;
    int              serviceIdx       = 0;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != eitDesc);
    BDBG_ASSERT(NULL != pProgram);

    BDBG_MSG(("number of caption services:%d", PSIP_CSD_getNumServices(eitDesc)));
    for (serviceIdx = 0; serviceIdx < PSIP_CSD_getNumServices(eitDesc); serviceIdx++)
    {
        if (B_ERROR_SUCCESS != PSIP_CSD_getService(eitDesc, serviceIdx, &service))
        {
            continue;
        }

        pProgram->captions.captionService[numValidServices].language[0] = 
            service.language[0];
        pProgram->captions.captionService[numValidServices].language[1] = 
            service.language[1];
        pProgram->captions.captionService[numValidServices].language[2] = 
            service.language[2];
        BDBG_MSG(("%d caption lang:%c%c%c", 
                  numValidServices,
                  pProgram->captions.captionService[numValidServices].language[0],
                  pProgram->captions.captionService[numValidServices].language[1],
                  pProgram->captions.captionService[numValidServices].language[2]));

        pProgram->captions.captionService[numValidServices].ccType = 
            service.cc_type;

        if (service.cc_type)
        {
            pProgram->captions.captionService[numValidServices].cc.captionServiceNumber = 
                service.cc.caption_service_number;
            BDBG_MSG(("%d caption ServiceNumber:0x%02x",
                      numValidServices,
                      pProgram->captions.captionService[numValidServices].cc.captionServiceNumber));
        }
        else
        {
            pProgram->captions.captionService[numValidServices].cc.line21Field = 
                service.cc.line21_field;
            BDBG_MSG(("%d caption line21Field:%d",
                      numValidServices,
                      pProgram->captions.captionService[numValidServices].cc.line21Field));
        }

        pProgram->captions.captionService[numValidServices].easyReader = 
            service.easy_reader;
        pProgram->captions.captionService[numValidServices].wideAspectRatio = 
            service.wide_aspect_ratio;
        BDBG_MSG(("%d caption easyReader:%d wideAspectRatio:%d",
                  numValidServices,
                  pProgram->captions.captionService[numValidServices].easyReader,
                  pProgram->captions.captionService[numValidServices].wideAspectRatio));

        numValidServices++;
    }


    pProgram->captions.numCaptions = numValidServices;
}

/***************************************************************************
Summary:
    Get the redistribution control descriptor if it exists.

Description:
    Get the redistribution control descriptor if it exists. Save the
    descriptor in the given program struct.  

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessRedistributionControlDescriptor(
    B_PSIP_Job            * pJob,
    TS_PSI_descriptor       eitDesc,
    B_PSIP_Program        * pProgram
    )
{
    B_PSIP_Handle   si        = NULL;
    uint8_t         numBytes  = 0;
    const uint8_t * pRcdData  = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != eitDesc);
    BDBG_ASSERT(NULL != pProgram);

    si = pJob->apiSettings.siHandle;

    if (NULL == si->settings.RedistributionControlData)
    {
        BDBG_WRN(("No redistribution control data buffer specified - ignoring descriptor"));
        return;
    }

    PSIP_RCD_getRcInformation(eitDesc, &numBytes, &pRcdData);

    memcpy(si->settings.RedistributionControlData, pRcdData, numBytes);
}

/***************************************************************************
Summary:
    Get the genre descriptor if it exists.

Description:
    Get the genre descriptor if it exists. Save the genres in the given 
    program struct.  

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessGenreDescriptor(
    B_PSIP_Job            * pJob,
    TS_PSI_descriptor       eitDesc,
    B_PSIP_Program        * pProgram
    )
{
    int i = 0;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != eitDesc);
    BDBG_ASSERT(NULL != pProgram);

    pProgram->genres.numGenres = PSIP_GD_getNumGenres(eitDesc);
    if (pProgram->genres.numGenres > MAX_GENRES)
    {
        pProgram->genres.numGenres = MAX_GENRES;
    }

    memset(pProgram->genres.genre, 0, sizeof(uint8_t) * MAX_GENRES);
    for (i = 0; i < pProgram->genres.numGenres; i++)
    {
        PSIP_GD_getGenre(eitDesc, i, &pProgram->genres.genre[i]);
        BDBG_MSG(("genre:0x%02x", pProgram->genres.genre[i]));
    }
}

/***************************************************************************
Summary:
    Process the descriptors contained in the EIT. 

Description:
    Process the descriptors contained in the EIT stored in the getProgInfo
    state data.  Assumes that the EIT has already been saved in the 
    getProgInfo state data.

Returns:
    None

See Also:
    None
****************************************************************************/
static void B_PSIP_ProcessDescriptors(
    B_PSIP_Job      * pJob,
    B_PSIP_Program  * pProgram,
    uint8_t           eventIdx
    )
{                            
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;
    TS_PSI_descriptor              eitDesc;
    int                            eitDescIdx = 0;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pProgram);

    pStateData = &pJob->stateData.getProgInfoData;

    /* Process EIT entry descriptors */
    while ((eitDesc = PSIP_EIT_getEventDescriptor(pStateData->eitBuffer, eventIdx, eitDescIdx++)) != NULL)
    {
        switch (eitDesc[0])
        {
        case 0x87: /* Content advisory descriptor */
            BDBG_MSG(("********** Content Advisory descriptor found! [0x%02x]", eitDesc[0]));
            B_PSIP_ProcessContentAdvisoryDescriptor(pJob, eitDesc, pProgram);
            break;
        case 0x86: /* Caption Service descriptor */
            BDBG_MSG(("********** Caption Service descriptor found! [0x%02x]", eitDesc[0]));
            B_PSIP_ProcessCaptionServiceDescriptor(pJob, eitDesc, pProgram);
            break;
        case 0x81: /* AC-3 Audio descriptor */
            BDBG_MSG(("********** AC-3 Audio descriptor found! [0x%02x]", eitDesc[0]));
            B_PSIP_ProcessAC3AudioDescriptor(pJob, eitDesc, pProgram);
            break;
        case 0xAA: /* Redistribution control descriptor */
            BDBG_MSG(("********** Redistribution Control descriptor found! [0x%02x]", eitDesc[0]));
            B_PSIP_ProcessRedistributionControlDescriptor(pJob, eitDesc, pProgram);
            break;
        case 0xB6: /* Content identifier descriptor */
            BDBG_MSG(("********** Content Identifier descriptor found! [0x%02x]", eitDesc[0]));
            break;
        case 0xAB: /* Genre descriptor */
            BDBG_MSG(("********** Genre descriptor found! [0x%02x]", eitDesc[0]));
            B_PSIP_ProcessGenreDescriptor(pJob, eitDesc, pProgram);
            break;
        default:
            BDBG_WRN(("eitDesc[0x%02x] ignored",eitDesc[0]));
            break;
        }
    }
}

/***************************************************************************
Summary:
    Final state machine state that notifies original caller that 
    acquisition of the program info is complete.  This state will
    also cancel timers and free temporary state machine memory associated
    with the given job.

Description:
    State - GetProgInfo
    SubState - Done

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_Done(
    B_PSIP_Job        * pJob, 
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    /* save number of saved programs */
    *pStateData->pLength = pStateData->progsFound;

    if ((B_PSIP_eStateEvent_Failure == event) && (0 < pStateData->progsFound))
    {
        /* when searching for programs within a time RANGE, it is possible to arrive
           at this point with a failure event, so we'll change it to success
           if we have found at least one program.  this happens when searching
           the MGT for programs that match a time range that extends past the
           currently available programs in the stream.  while we may have found
           some programs during our search, we will eventually arrive here with 
           an error when searching for a program time that simply does not exist. 
           this is a signal that our time range search is over - not that an 
           error has occurred */
        event = B_PSIP_eStateEvent_Enter;
    }

    /* we're at the end of the state machine so remove job from list */
    BLST_S_REMOVE(&pJob->apiSettings.siHandle->jobList, pJob, B_PSIP_Job, node);

    /* clean up state data */
    if (pStateData->sttBuffer)
    {
        B_Os_Free(pStateData->sttBuffer);
        pStateData->sttBuffer    = NULL;
        pStateData->sttBufferLen = 0;
    }
    if (pStateData->eitBuffer)
    {
        B_Os_Free(pStateData->eitBuffer);
        pStateData->eitBuffer    = NULL;
        pStateData->eitBufferLen = 0;
    }
    if (pStateData->ettBuffer)
    {
        B_Os_Free(pStateData->ettBuffer);
        pStateData->ettBuffer    = NULL;
        pStateData->ettBufferLen = 0;
    }
    if (pStateData->mgtBuffer)
    {
        B_Os_Free(pStateData->mgtBuffer);
        pStateData->mgtBuffer    = NULL;
        pStateData->mgtBufferLen = 0;
    }
    if (pStateData->vctBuffer)
    {
        B_Os_Free(pStateData->vctBuffer);
        pStateData->vctBuffer    = NULL;
        pStateData->vctBufferLen = 0;
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
        BDBG_WRN(("shouldn't get a table received event in this state:%s", __FUNCTION__));
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


/***************************************************************************
Summary:
    State machine state that saves the currently acquired program data to
    the calling code supplied program list and then determines if the entire
    time range has been searched for available program data.  If more 
    searching is required, we transition back to the SearchMGT4EIT state.
    Otherwise transition to the Done state.

Description:
    State - GetProgInfo
    SubState - SaveProgram

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_SaveProgram(
    B_PSIP_Job        * pJob, 
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;
    
    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);
    BSTD_UNUSED(event);

    pStateData = &pJob->stateData.getProgInfoData;
    
    /* successfully retreived program data */

    if (*pStateData->pLength > pStateData->progsFound)
    {
        pStateData->progsFound++;

        /* copy acquired program data to user's struct */
        memcpy(&pStateData->progList[pStateData->progsFound-1], 
               &pStateData->currentProg, 
               sizeof(B_PSIP_Program));

        /* set start of search time range just past then end time of the 
           current program */
        pStateData->timeUTCStart = pStateData->currentProg.startTime + 
                                   pStateData->currentProg.length +
                                   1;

        if (pStateData->timeUTCStart < pStateData->timeUTCEnd)
        {
            /* continue searching for more programs */
            pStateData->tableIndex = 0;
            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4EIT);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            BDBG_MSG(("new start time:%ul end time:%ul", pStateData->timeUTCStart, pStateData->timeUTCEnd));
            /* done searching for more programs */
            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_Done);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
    }
    else
    {
        BDBG_WRN(("Not enough space to save program data #%d", pStateData->progsFound));
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
    }
}

/***************************************************************************
Summary:
    State machine state that gets the ETT and saves the extended text
    string to given job's state data.

Description:
    State - GetProgInfo
    SubState - GetETT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetETT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->eitBufferLen = ETT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetETT_P(&pJob->apiSettings, 
                                               pStateData->pidETT,
                                               pStateData->currentCh.sourceId,
                                               pStateData->currentProg.eventId,
                                               B_PSIP_eEventETM,
                                               pStateData->ettBuffer,
                                               &pStateData->ettBufferLen,
                                               pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        {
            /* no more sections to try (or never got section 0 successfully) so keep searching for more ETTs in MGT */
            pStateData->tableIndex++;
            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4ETT);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_SaveProgram);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        {
            PSIP_ETT_header ettHeader;
            uint16_t        descMaxLen = MAX_DESC_CHARS;

            BDBG_MSG(("%s success!", __FUNCTION__));
            
            PSIP_ETT_getHeader(pStateData->ettBuffer, &ettHeader);
            B_PSIP_GetStringFromMSS(pJob->apiSettings.siHandle, 
                                    ettHeader.p_extended_text_message, 
                                    pStateData->currentProg.description,
                                    &descMaxLen);

            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SaveProgram);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that searches the current MGT for an ETT associated
    with the given event id. 

Description:
    State - GetProgInfo
    SubState - SearchMGT4ETT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_SearchMGT4ETT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            bool foundETT = false;

            /* get time difference between current time (from stt) and 
               desired program time.  use that diff to calculate the eit
               number that will contain the program info. search that eit
               only. (note: 3hrs of programming per EIT) */
            int      timeDiff        = 0;
            uint32_t timeCurrentUTC  = 0;
            uint16_t targetTableType = 0x200;
            PSIP_STT_header header;

            PSIP_STT_getHeader(pStateData->sttBuffer, &header);
            timeCurrentUTC = header.system_time - header.GPS_UTC_offset;
            BDBG_MSG(("timeCurrentUTC:%lu", timeCurrentUTC));

            timeDiff = pStateData->timeUTCStart - timeCurrentUTC;
            if ((0 <= timeDiff) && (0 < (timeDiff / 60 / 60 / 3)))
            {
                uint8_t tableIndexSkipETT = 0;

                tableIndexSkipETT  = (timeDiff / 60 / 60 / 3);
                targetTableType   += tableIndexSkipETT;
                BDBG_MSG(("+++++++++++++++++ %s:%d timeDiff in hours:%d skipping ETTs:%d targetTableType:0x%04x", 
                          __FUNCTION__, __LINE__, timeDiff / 60 / 60, tableIndexSkipETT, targetTableType));
            }

            /* search MGT for requested ETT */
            BDBG_MSG(("SEARCHING for ETT table: 0x%04x", targetTableType)); 
            while (pStateData->tableIndex < PSIP_MGT_getTablesDefined(pStateData->mgtBuffer))
            {
                PSIP_MGT_table table;
                PSIP_MGT_getTable(pStateData->mgtBuffer, pStateData->tableIndex, &table);
                if (table.table_type != targetTableType)
                {
                    /* skip non-matching tables */
                    pStateData->tableIndex++;
                    continue;
                }

                BDBG_MSG(("ETT table (type:0x%04x) found - search for extended text for given event id:0x%04x", 
                          table.table_type, pStateData->currentProg.eventId));

                pStateData->pidETT = table.table_type_PID;
                foundETT = true;
                break;
            }

            if (true == foundETT)
            {
                /* found ETT table so get/process it */
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetETT); 
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                /* done searching mgt for ETT tables */
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SaveProgram);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_TableTimeout:
        /* never got section 0 successfully so give up */
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SaveProgram);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that searches the given EIT for an event matching
    the request UTC time.  If found, save program data and process 
    descriptors.

Description:
    State - GetProgInfo
    SubState - SearchEIT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_SearchEIT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            bool           foundProgram        = false;
            PSIP_EIT_event eitEvent;

            while (pStateData->eventIndex < PSIP_EIT_getNumEvents(pStateData->eitBuffer))
            {
                uint16_t titleMaxLen  = MAX_TITLE_CHARS;

                BDBG_MSG(("get EIT[%d]", pStateData->eventIndex));
                if (B_ERROR_SUCCESS != PSIP_EIT_getEvent(pStateData->eitBuffer, pStateData->eventIndex, &eitEvent ))
                {
                    BDBG_WRN(("Failure processing EIT event:%d", pStateData->eventIndex));
                    break;
                }

                BDBG_MSG(("eitEvent.start_time:%lu eitEvent.end_time:%lu requested time:%lu", 
                          eitEvent.start_time, eitEvent.start_time + eitEvent.length_in_seconds,
                          pStateData->timeUTCStart));

                if ((pStateData->timeUTCStart < eitEvent.start_time) || 
                    (pStateData->timeUTCStart >= (eitEvent.start_time + eitEvent.length_in_seconds)))
                {
                    /* given time is outside of the current EIT event's time range */
                    pStateData->eventIndex++;
                    continue;
                }

                BDBG_MSG(("********** FOUND requested program event in EIT!"));

                memset(&pStateData->currentProg, 0, sizeof(B_PSIP_Program)); 
                pStateData->currentProg.startTime   = eitEvent.start_time;
                pStateData->currentProg.length      = eitEvent.length_in_seconds;
                pStateData->currentProg.eventId     = eitEvent.event_id;

                titleMaxLen  = MAX_TITLE_CHARS;
                B_PSIP_GetStringFromMSS(pJob->apiSettings.siHandle, 
                                        eitEvent.p_title_text, 
                                        pStateData->currentProg.title, 
                                        &titleMaxLen);

                B_PSIP_ProcessDescriptors(pJob, &pStateData->currentProg, pStateData->eventIndex);

                if (PSIP_ETM == eitEvent.ETM_location)
                {
                    /* program description is available - get it from ETT later */
                    pStateData->currentProg.ettAvailable = true;
                }

                foundProgram = true;
                break;
            }

            if (true == foundProgram)
            {
                if (true == pStateData->currentProg.ettAvailable)
                {
                    pStateData->tableIndex = 0;
                    CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4ETT);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
                else
                {
                    CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SaveProgram);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
            }
            else
            {
                pStateData->sectionIndexEIT++;
                if (pStateData->sectionIndexEIT <= pStateData->sectionIndexMaxEIT) 
                {
                    /* get next section of eit */
                    CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetEIT);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
                else
                {
                    /* done looking thru all available eit sections */
                    pStateData->tableIndex++;
                    CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4EIT);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
            }
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        pJob->stateData.getProgInfoData.sectionIndexEIT++;
        if (pJob->stateData.getProgInfoData.sectionIndexEIT < pJob->stateData.getProgInfoData.sectionIndexMaxEIT)
        {
            /* more sections to try */
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            /* no more sections to try (or never got section 0 successfully) so give up */
            CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
            STATE_ENGINE(pJob, event);
        }
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
        /*
        printEIT(pJob->apiSettings.siHandle, 
                 pJob->stateData.getProgInfoData.eitBuffer, 
                 pJob->stateData.getProgInfoData.eitBufferLen);
        */
        pJob->stateData.getProgInfoData.sectionIndexMaxEIT = pJob->stateData.getProgInfoData.eitBuffer[7];
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchEIT);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that gets the given EIT section and saves it in the
    job's state data.

Description:
    State - GetProgInfo
    SubState - GetEIT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetEIT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->eitBufferLen = EIT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetEIT_P(&pJob->apiSettings, 
                                               pStateData->pidEIT,
                                               pStateData->currentCh.sourceId,
                                               pStateData->sectionIndexEIT, 
                                               pStateData->eitBuffer,
                                               &pStateData->eitBufferLen,
                                               pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        pStateData->sectionIndexEIT++;
        if (pStateData->sectionIndexEIT < pStateData->sectionIndexMaxEIT)
        {
            /* more sections to try */
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            /* no more sections to try (or never got section 0 successfully) so keep searching for more EITs in MGT */
            pStateData->tableIndex++;
            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4EIT);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
       /* 
        printEIT(pJob->apiSettings.siHandle, 
                 pJob->stateData.getProgInfoData.eitBuffer, 
                 pJob->stateData.getProgInfoData.eitBufferLen);
        */
        pStateData->sectionIndexMaxEIT = pStateData->eitBuffer[7];
        pStateData->eventIndex         = 0;
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchEIT);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that searches the current MGT for an EIT associated
    with the given event id. 

Description:
    State - GetProgInfo
    SubState - SearchMGT4EIT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_SearchMGT4EIT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            bool foundEIT           = false;
            bool alreadyReceivedEIT = false;

            /* get time difference between current time (from stt) and 
               desired program time.  use that diff to calculate the eit
               number that will contain the program info. search that eit
               only. */
            int      timeDiff        = 0;
            uint32_t timeCurrentUTC  = 0;
            uint16_t targetTableType = 0x100;
            PSIP_STT_header header;

            PSIP_STT_getHeader(pStateData->sttBuffer, &header);
            timeCurrentUTC = header.system_time - header.GPS_UTC_offset;
            BDBG_MSG(("timeCurrentUTC:%lu", timeCurrentUTC));

            timeDiff = pStateData->timeUTCStart - timeCurrentUTC;
            if ((0 <= timeDiff) && (0 < (timeDiff / 60 / 60 / 3)))
            {
                uint8_t tableIndexSkipEIT = 0;

                tableIndexSkipEIT  = (timeDiff / 60 / 60 / 3);
                targetTableType   += tableIndexSkipEIT;
                BDBG_MSG(("+++++++++++++++++ %s:%d timeDiff in hours:%d skipping EITs:%d targetTableType:0x%04x", 
                          __FUNCTION__, __LINE__, timeDiff / 60 / 60, tableIndexSkipEIT, targetTableType));
            }

            /* search MGT for requested EIT */
            while (pStateData->tableIndex < PSIP_MGT_getTablesDefined(pStateData->mgtBuffer))
            {
                PSIP_MGT_table table;
                PSIP_MGT_getTable(pStateData->mgtBuffer, pStateData->tableIndex, &table);
                if (table.table_type != targetTableType)
                {
                    /* skip non-matching tables */
                    pStateData->tableIndex++;
                    continue;
                }

                BDBG_MSG(("EIT table (type:0x%04x) found - search for program event for given time:%lu", 
                          table.table_type, pStateData->timeUTCStart));

                if (pStateData->pidEIT == table.table_type_PID)
                {
                    alreadyReceivedEIT = true;
                }

                pStateData->pidEIT = table.table_type_PID;
                foundEIT = true;
                break;
            }

            if (true == foundEIT)
            {
                /* found eit table so get/process it */
                pStateData->sectionIndexEIT = 0;
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetEIT); 

                if (true == alreadyReceivedEIT)
                {
                    /* our EIT buffer already contains the correct EIT section,
                       so just process (do not reacquire) */
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_TableReceived);
                }
                else
                {
                    /* reqcquire proper EIT section */
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
            }
            else
            {
                /* done searching mgt for eit tables - requested eit not found! */
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
            }
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        /* never got section 0 successfully so give up */
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that gets the current MGT and saves it in the given
    job's state data.

Description:
    State - GetProgInfo
    SubState - GetMGT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetMGT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

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
        /* never got section 0 successfully so give up */
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
        /*
        printMGT(pJob->apiSettings.siHandle, 
                 pJob->stateData.getProgInfoData.mgtBuffer, 
                 pJob->stateData.getProgInfoData.mgtBufferLen);
        */
        pStateData->tableIndex        = 0;
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_SearchMGT4EIT);  
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    Get the long channel name for the given index from the Extended Channel
    Name Descriptor contained in the given buffer.  

Description:
    Get the long channel name for the given index from the Extended Channel
    Name Descriptor contained in the given buffer.  Save name in pCh.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_Error B_PSIP_GetExtChannelNameFromECND(
    B_PSIP_Handle    si, 
    uint8_t        * buffer,
    uint8_t          index,
    B_PSIP_Channel * pCh
    )
{
    TS_PSI_descriptor chDesc;
    int               descIdx      = 0;

    BDBG_ASSERT(NULL != buffer);
    BDBG_ASSERT(NULL != pCh);

    while (NULL != (chDesc = PSIP_VCT_getChannelDescriptor(buffer, index, descIdx)))
    {
        if (chDesc[0] == 0xA0) /* Extended Channel Name descriptor */
        {
            uint16_t length = MAX_LONG_NAME_CHARS;
            PSIP_MSS_string lstr = PSIP_ECND_getLongChannelName(chDesc);
            memset(pCh->extChName, 0, MAX_LONG_NAME_CHARS);
            B_PSIP_GetStringFromMSS(si, lstr, pCh->extChName, &length);
            BDBG_MSG(("----------------------------------- ext channel name:%s", pCh->extChName));
            break;
        }

        descIdx++;
    }

    return (pCh->extChName[0] ? B_ERROR_SUCCESS: B_ERROR_UNKNOWN);
}

/***************************************************************************
Summary:
    State machine state that searches the VCT for the source id associated
    with the given minor channel number.

Description:
    State - GetProgInfo
    SubState - GetSourceId

    Assumes that the VCT has already been acquired and saved in GetProgInfo
    state data.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetSourceId(
    B_PSIP_Job        * pJob, 
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            bool foundSourceId = false;

            while (pStateData->channelIndex < PSIP_VCT_getNumChannels(pStateData->vctBuffer))
            {
                PSIP_VCT_channel vct;
                B_Error err = B_ERROR_SUCCESS;

                err = PSIP_VCT_getChannel(pStateData->vctBuffer, 
                                          pStateData->channelIndex, 
                                          &vct);

                if (B_ERROR_SUCCESS == err)
                {
                    if (vct.minor_channel_number != pStateData->currentCh.minor)
                    {
                        pStateData->channelIndex++;
                        continue;
                    }

                    memset(&pStateData->currentCh, 0, sizeof(B_PSIP_Channel));
                    pStateData->currentCh.minor        = vct.minor_channel_number;
                    pStateData->currentCh.major        = vct.major_channel_number;
                    pStateData->currentCh.sourceId     = vct.source_id;
                    pStateData->currentCh.programNum   = vct.program_number;
                    memcpy(pStateData->currentCh.chName, vct.short_name, MAX_CH_NAME_CHARS * sizeof(uint16_t));
                    B_PSIP_GetExtChannelNameFromECND(pJob->apiSettings.siHandle, 
                                                   pStateData->vctBuffer, 
                                                   pStateData->channelIndex, 
                                                   &pStateData->currentCh);

                    BDBG_MSG(("found requested channel (%d.%d) in VCT - sourceId:0x%04x!", 
                              pStateData->currentCh.major, 
                              pStateData->currentCh.minor, 
                              pStateData->currentCh.sourceId));

                    /* set local time offset */
                    if (B_PSIP_eTS_NONE == pJob->apiSettings.siHandle->localTimeSource)
                    {
                        int  local_offset = 0;
                        char dst          = 0;

                        if ((tsid_to_utc_offset(vct.channel_TSID, &local_offset, &dst) == 0))
                        {
                            pJob->apiSettings.siHandle->localTimeSource = B_PSIP_eTS_TSID;
                            pJob->apiSettings.siHandle->localTimeOffset = local_offset;
                            pJob->apiSettings.siHandle->localTimeDstObs = dst;
                            BDBG_WRN(("### Local time %d from TSID 0x%04x ###",
                                      local_offset,
                                      vct.channel_TSID));
                        }
                    }

                    foundSourceId = true;
                    break;
                }
                else
                {
                    BDBG_WRN(("PSIP_VCT_getChannel %d failed", pStateData->channelIndex));
                }

                pStateData->channelIndex++;
            }

            pStateData->sectionIndexVCT++;
            if ((pStateData->sectionIndexVCT <= pStateData->sectionIndexMaxVCT) &&
                (false == foundSourceId))
            {
                /* get next section of vct */
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetVCT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
            else
            {
                /* done looking thru all available vct sections */
                CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetMGT);
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
            }
        }
        break;
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that gets the VCT for the given section index.

Description:
    State - GetProgInfo
    SubState - GetVCT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetVCT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->vctBufferLen = VCT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetTVCT_P(&pJob->apiSettings, 
                                              pStateData->sectionIndexVCT, 
                                              pStateData->vctBuffer,
                                              &pStateData->vctBufferLen,
                                              pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        pStateData->sectionIndexVCT++;
        if (pStateData->sectionIndexVCT < pStateData->sectionIndexMaxVCT)
        {
            /* more sections to try */
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            /* no more sections to try (or never got section 0 successfully) so give up */
            CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
            STATE_ENGINE(pJob, event);
        }
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
        /*
        printVCT(pJob->apiSettings.siHandle, 
                 pJob->stateData.getProgInfoData.vctBuffer, 
                 pJob->stateData.getProgInfoData.vctBufferLen);
        */
        pStateData->sectionIndexMaxVCT = pStateData->vctBuffer[7];
        pStateData->channelIndex = 0;
        CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetSourceId);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

/***************************************************************************
Summary:
    State machine state that gets the STT. 

Description:
    State - GetProgInfo
    SubState - GetSTT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetProgInfo_GetSTT(
    B_PSIP_Job          * pJob, 
    B_PSIP_StateEvent     event
    )
{
    B_PSIP_StateData_GetProgInfo * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getProgInfoData;

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
        /* no more sections to try (or never got section 0 successfully) so give up */
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetProgInfo_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        {
            PSIP_STT_header header;
            uint32_t        currentTime = 0;

            BDBG_MSG(("%s success!", __FUNCTION__));
            
            /*
            printSTT(pStateData->sttBuffer, 
                     pStateData->sttBufferLen);
            */
           
            PSIP_STT_getHeader(pStateData->sttBuffer, &header);
            currentTime = header.system_time - header.GPS_UTC_offset;

            if ((0 == pStateData->timeUTCStart) || (currentTime > pStateData->timeUTCStart))
            {
                /* default requested start time to current time if value is 0 
                   or if requested start time is less than current time */
                pStateData->timeUTCStart = currentTime;
            }

            if (0 == pStateData->timeUTCEnd)
            {
                /* default requested end time to current time if value is 0 */
                pStateData->timeUTCEnd   = header.system_time - header.GPS_UTC_offset;
            }

            CHANGE_STATE(pJob, B_PSIP_eState_GetProgInfo_GetVCT);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d", 
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

