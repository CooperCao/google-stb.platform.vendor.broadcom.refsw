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
*    Library routines for PSIP getchlist state
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
#include "ts_pat.h"
#include "ts_pmt.h"
#include "psip_vct.h"
#include "psip_mss.h"
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
    State machine state that gets the PMT and saves found PID information.

Description:
    State - GetChList
    SubState - GetPMT

    This state assumes that the PAT has already been retrieved and is stored
    in the given job's state data.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetChList_GetPidsPSI_PMT(
    B_PSIP_Job        * pJob,
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetChList * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getChListData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            TS_PAT_program pat;
            uint8_t        progIndex = 0;
            BERR_Code      rc;
            for (progIndex = 0;
                 progIndex < TS_PAT_getNumPrograms(pStateData->patBuffer);
                 progIndex++)
            {
                /* we will assume pat table data is valid */
                rc = TS_PAT_getProgram(pStateData->patBuffer,
                                  pStateData->patBufferLen,
                                  progIndex,
                                  &pat);
                if (rc != BERR_SUCCESS)
                {
                    continue;
                }
                if (pat.program_number != pStateData->currentCh.programNum)
                {
                    continue;
                }

                pStateData->pmtBufferLen = PMT_BUF_LEN;
                if (B_ERROR_SUCCESS != B_PSIP_GetPMT_P(&pJob->apiSettings,
                                                     pat.PID,
                                                     pStateData->pmtBuffer,
                                                     &pStateData->pmtBufferLen,
                                                     pJob))
                {
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
                }

                /* already found the correct programNum so quit searching */
                break;
            }

            if (progIndex == TS_PAT_getNumPrograms(pStateData->patBuffer))
            {
                BDBG_WRN(("No matching program number (%d) in PAT - probably means that given stream is available on a different physical channel (%d.%d) - not saved to channel list!",
                          pStateData->currentCh.programNum,
                          pStateData->currentCh.major,
                          pStateData->currentCh.minor));
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
            }
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        /* skip to next channel index and continue searching for channels */
        pJob->stateData.getChListData.channelIndex++;
        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetChData);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        {
            int numAudio = 0;
            int numVideo = 0;
            int p_idx    = 0;

            BDBG_MSG(("%s success!", __FUNCTION__));
            /*
            printPMT(pJob->apiSettings.siHandle, pStateData->pmtBuffer, pStateData->pmtBufferLen);
            */

            pStateData->currentCh.pcrPID =
                TS_PMT_getPcrPid(pStateData->pmtBuffer, pStateData->pmtBufferLen);

            for (p_idx = 0;
                 p_idx < TS_PMT_getNumStreams(pStateData->pmtBuffer, pStateData->pmtBufferLen);
                 p_idx++)
            {
                TS_PMT_stream pmt;
                if (B_ERROR_SUCCESS != TS_PMT_getStream(pStateData->pmtBuffer, pStateData->pmtBufferLen, p_idx, &pmt))
                {
                    BDBG_WRN(("Failure processing PMT %d",p_idx));
                    continue;
                }

                if (pmt.stream_type == 0x02) /* Video */
                {
                    if (MAX_VIDEO_PIDS > numVideo)
                    {
                        BDBG_WRN(("Video PID[%d] = 0x%04x",p_idx,pmt.elementary_PID ));
                        pStateData->currentCh.videoPID[numVideo] = pmt.elementary_PID;
                        numVideo++;
                    }
                    else
                    {
                        BDBG_WRN(("Throwing out Video PID[%d] = 0x%04x", numVideo, pmt.elementary_PID));
                    }
                }
                else
                if (pmt.stream_type == 0x81) /* Audio */
                {
                    if (MAX_AUDIO_PIDS > numAudio)
                    {
                        BDBG_WRN(("Audio PID[%d] = 0x%04x",p_idx,pmt.elementary_PID ));
                        pStateData->currentCh.audioPID[numAudio] = pmt.elementary_PID;
                        numAudio++;
                    }
                    else
                    {
                        BDBG_WRN(("Throwing out Audio PID[%d] = 0x%04x", numAudio, pmt.elementary_PID));
                    }
                }
            }

            if ((0 < numAudio) || (0 < numVideo))
            {
                pStateData->channelsFound++;

                /* got all channel data so add to list of channels (if space is avail)*/
                if (pStateData->channelsSaved < *(pStateData->pLength))
                {
                    BDBG_WRN(("adding channel:%d.%d to channel list",
                              pStateData->currentCh.major,
                              pStateData->currentCh.minor));
                    memcpy(&pStateData->chList[pStateData->channelsSaved],
                           &pStateData->currentCh,
                           sizeof(B_PSIP_Channel));
                    pStateData->channelsSaved++;
                }
                else
                {
                    BDBG_WRN(("not enough space in chList to save channel:%d.%d",
                              pStateData->currentCh.major,
                              pStateData->currentCh.minor));
                }
            }
            else
            {
                /* no pids found so search next channel */
                BDBG_WRN(("no pids found for ch:%d.%d - not saving in channel list",
                          pStateData->currentCh.major,
                          pStateData->currentCh.minor));
            }

            pStateData->channelIndex++;
            CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetChData);
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
    State machine state that gets the PAT and saves it in the given job's
    state data.

Description:
    State - GetChList
    SubState - GetPAT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetChList_GetPidsPSI_PAT(
    B_PSIP_Job        * pJob,
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetChList * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getChListData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->patBufferLen = PAT_BUF_LEN;
        if (B_ERROR_SUCCESS != B_PSIP_GetPAT_P(&pJob->apiSettings,
                                             pStateData->patBuffer,
                                             &pStateData->patBufferLen,
                                             pJob))
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        /* skip to next channel index and continue building ch list */
        pJob->stateData.getChListData.channelIndex++;
        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetChData);
        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));

        if (TS_PAT_validate(pJob->stateData.getChListData.patBuffer,
                            pJob->stateData.getChListData.patBufferLen))
        {
            /*
            printPAT(pJob->apiSettings.siHandle, pStateData->patBuffer, pStateData->patBufferLen);
            */

            CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetPidsPSI_PMT);
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
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
    Final state machine state that notifies original caller that
    acquisition of the current channel list is complete.  This state will
    also cancel timers and free temporary state machine memory associated
    with the given job.

Description:
    State - GetChList
    SubState - Done

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetChList_Done(
    B_PSIP_Job        * pJob,
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetChList * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getChListData;

    /* we're at the end of the state machine so remove job from list */
    BLST_S_REMOVE(&pJob->apiSettings.siHandle->jobList, pJob, B_PSIP_Job, node);

    /* clean up state data */
    if (pStateData->patBuffer)
    {
        B_Os_Free(pStateData->patBuffer);
        pStateData->patBuffer    = NULL;
        pStateData->patBufferLen = 0;
    }
    if (pStateData->pmtBuffer)
    {
        B_Os_Free(pStateData->pmtBuffer);
        pStateData->pmtBuffer    = NULL;
        pStateData->pmtBufferLen = 0;
    }
    if (pStateData->vctBuffer)
    {
        B_Os_Free(pStateData->vctBuffer);
        pStateData->vctBuffer    = NULL;
        pStateData->vctBufferLen = 0;
    }

    if (0 == *(pStateData->pLength))
    {
        /* since our original API call indicated a channel list size of 0, we
           only save the size of the available channel list.  no actual channnel
           list data is saved. */

        /* change original length param to indicate number of found channels */
        *(pStateData->pLength) = pStateData->channelsFound;
    }
    else
    {
        /* change original length param to indicate number of saved channels */
        *(pStateData->pLength) = pStateData->channelsSaved;
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

/***************************************************************************
Summary:
    State machine state that gets the VCT and saves it in the given job's
    state data.

Description:
    State - GetChList
    SubState - GetVCT

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetChList_GetVCT(
    B_PSIP_Job        * pJob,
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetChList * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getChListData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        pStateData->vctBufferLen = VCT_BUF_LEN;
        if (B_PSIP_eTerrestrial == pStateData->type)
        {
            if (B_ERROR_SUCCESS != B_PSIP_GetTVCT_P(&pJob->apiSettings,
                                                  pStateData->sectionIndex,
                                                  pStateData->vctBuffer,
                                                  &pStateData->vctBufferLen,
                                                  pJob))
            {
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
            }
        }
        else
        if (B_PSIP_eCable == pStateData->type)
        {
            if (B_ERROR_SUCCESS != B_PSIP_GetCVCT_P(&pJob->apiSettings,
                                                  pStateData->sectionIndex,
                                                  pStateData->vctBuffer,
                                                  &pStateData->vctBufferLen,
                                                  pJob))
            {
                STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
            }
        }
        else
        {
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Failure);
        }
        break;
    case B_PSIP_eStateEvent_Failure:
    case B_PSIP_eStateEvent_TableTimeout:
        pJob->stateData.getChListData.sectionIndex++;
        if (pJob->stateData.getChListData.sectionIndex < pJob->stateData.getChListData.sectionIndexMax)
        {
            /* more sections to try */
            STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
        }
        else
        {
            /* no more sections to try (or never got section 0 successfully) so give up */
            CHANGE_STATE(pJob,B_PSIP_eState_GetChList_Done);
            STATE_ENGINE(pJob, event);
        }
        break;
    case B_PSIP_eStateEvent_Timeout:
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetChList_Done);
        STATE_ENGINE(pJob, event);
        break;
    case B_PSIP_eStateEvent_TableReceived:
        BDBG_MSG(("%s success!", __FUNCTION__));
        /*
        printVCT(pJob->apiSettings.siHandle,
                 pJob->stateData.getChListData.vctBuffer,
                 pJob->stateData.getChListData.vctBufferLen);
        */
        pJob->stateData.getChListData.sectionIndexMax = pJob->stateData.getChListData.vctBuffer[7];
        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetChData);
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
    Get the time shift services if they exist.

Description:
    Get the time shift services if they exist.  Save the time shift in the
    given channel struct.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_Error B_PSIP_GetTimeShiftServices(
    uint8_t         * buffer,
    uint8_t           index,
    B_PSIP_Channel  * pCh)
{
    TS_PSI_descriptor chDesc;
    int               descIdx      = 0;

    BDBG_ASSERT(NULL != buffer);
    BDBG_ASSERT(NULL != pCh);

    while (NULL != (chDesc = PSIP_VCT_getChannelDescriptor(buffer, index, descIdx)))
    {
        if (chDesc[0] == 0xA2) /* Time Shift descriptor */
        {
            uint8_t           numServices = chDesc[2] & 0x1F;
            uint8_t           i           = 0;
            TS_PSI_descriptor pService    = NULL;

            pCh->numTimeShiftServices = numServices;
            pService = chDesc + 3;
            for (i = 0; i < numServices; i++)
            {
                pCh->timeShift[i].timeShift = (((pService[0] & 0x03) << 8) | pService[1]);
                pCh->timeShift[i].major     = (((pService[2] & 0x0F) << 6) | ((pService[3] & 0xFC) >> 2));
                pCh->timeShift[i].minor     = (((pService[3] & 0x03) << 8) | pService[4]);
                BDBG_MSG(("timeshift:%d major:%d minor:%d",
                          pCh->timeShift[i].timeShift,
                          pCh->timeShift[i].major,
                          pCh->timeShift[i].minor));
                pService += 5;
            }
            break;
        }

        descIdx++;
    }

    return ((0 < pCh->numTimeShiftServices) ? B_ERROR_SUCCESS: B_ERROR_UNKNOWN);
}

/***************************************************************************
Summary:
    Get PIDs and language codes from the Service Location Descriptor.

Description:
    Get PIDs and lang codes for the given index from the Service Location
    Descriptor contained in the given buffer.  Save PIDs in pCh.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_Error B_PSIP_GetPidsSLD(
    uint8_t        * buffer,
    int              index,
    B_PSIP_Channel * pCh
    )
{
    TS_PSI_descriptor chDesc;
    bool              foundPIDs = false;
    int               descIdx      = 0;

    BDBG_ASSERT(NULL != buffer);
    BDBG_ASSERT(index >= 0);
    BDBG_ASSERT(NULL != pCh);

    while (NULL != (chDesc = PSIP_VCT_getChannelDescriptor(buffer, index, descIdx)))
    {
        if (chDesc[0] == 0xA1) /* Service location descriptor */
        {
            PSIP_SLD_header  sldHeader;
            PSIP_SLD_element sldElement;
            int              numAudio = 0;
            int              numVideo = 0;
            int              sldIdx   = 0;


            PSIP_SLD_getHeader(chDesc, &sldHeader);
            foundPIDs = true;

            pCh->pcrPID = sldHeader.PCR_PID;
            BDBG_MSG(("SLD number_elements = %d",sldHeader.number_elements ));
            BDBG_MSG(("found PCR PID = 0x%04x",pCh->pcrPID ));

            for (sldIdx = 0; sldIdx < sldHeader.number_elements; sldIdx++)
            {
                if (B_ERROR_SUCCESS != PSIP_SLD_getElement(chDesc,sldIdx,&sldElement))
                {
                    BDBG_WRN(("Error processing Service Location Descriptor %d of %d",
                              sldIdx,
                              sldHeader.number_elements));
                    break;
                }

                if (0x02 == sldElement.stream_type) /* Video */
                {
                    BDBG_WRN(("found Video PID[%d] = 0x%04x",
                              sldIdx,
                              sldElement.elementary_PID));

                    if (numVideo < MAX_AUDIO_PIDS)
                    {
                        pCh->videoPID[numVideo] = sldElement.elementary_PID;
                        numVideo++;
                    }
                    else
                    {
                        BDBG_WRN(("Throwing out Video PID[%d] = 0x%04x",
                                  sldIdx,
                                  sldElement.elementary_PID));
                    }
                    pCh->numVideoPids = numVideo;
                }
                else
                if (0x81 == sldElement.stream_type) /* Audio */
                {
                    BDBG_MSG(("found Audio PID[%d] = 0x%04x, lang => %c%c%c",
                              sldIdx,
                              sldElement.elementary_PID,
                              sldElement.ISO_639_language_code[0],
                              sldElement.ISO_639_language_code[1],
                              sldElement.ISO_639_language_code[2]));

                    if (numAudio < MAX_AUDIO_PIDS)
                    {
                        pCh->audioPID[numAudio]     = sldElement.elementary_PID;
                        pCh->audioLang[numAudio][0] = sldElement.ISO_639_language_code[0];
                        pCh->audioLang[numAudio][1] = sldElement.ISO_639_language_code[1];
                        pCh->audioLang[numAudio][2] = sldElement.ISO_639_language_code[2];
                        numAudio++;
                    }
                    else
                    {
                        BDBG_WRN(("Throwing out Audio PID[%d] = 0x%04x, lang => %c%c%c",
                                  sldIdx,
                                  sldElement.elementary_PID,
                                  sldElement.ISO_639_language_code[0],
                                  sldElement.ISO_639_language_code[1],
                                  sldElement.ISO_639_language_code[2]));
                    }
                    pCh->numAudioPids = numAudio;
                }
            }
        }

        descIdx++;
    }

    return (foundPIDs ? B_ERROR_SUCCESS: B_ERROR_UNKNOWN);
}

/***************************************************************************
Summary:
    Convert PSIP modulation mode to psip application library modulation mode
    type.

Description:
    Convert PSIP modulation mode to psip application library modulation mode
    type.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_PSIP_ModulationMode Psip2Si_ModulationMode(
    PSIP_VCT_modulation_mode modMode
    )
{
    B_PSIP_ModulationMode retVal = B_PSIP_eModUnknown;

    switch(modMode)
    {
    case PSIP_VCT_analog:
        retVal = B_PSIP_eModAnalog;
        break;
    case PSIP_VCT_SCTE_mode_1:
        retVal = B_PSIP_eModSCTE1;
        break;
    case PSIP_VCT_SCTE_mode_2:
        retVal = B_PSIP_eModSCTE2;
        break;
    case PSIP_VCT_ATSC_8:
        retVal = B_PSIP_eModATSC8;
        break;
    case PSIP_VCT_ATSC_16:
        retVal = B_PSIP_eModATSC16;
        break;
    default:
        retVal = B_PSIP_eModUnknown;
        break;
    }

    return retVal;
}

/***************************************************************************
Summary:
    Convert PSIP service type to psip application library service type.

Description:
    Convert PSIP service type to psip application library service type.

Returns:
    None

See Also:
    None
****************************************************************************/
static B_PSIP_ServiceType Psip2Si_ServiceType(
    PSIP_VCT_service_type serviceType
    )
{
    B_PSIP_ServiceType retVal = B_PSIP_eServiceUnknown;

    switch(serviceType)
    {
    case PSIP_VCT_analog_television:
        retVal = B_PSIP_eServiceAnalog;
        break;
    case PSIP_VCT_ATSC_digital_television:
        retVal = B_PSIP_eServiceAtscDigital;
        break;
    case PSIP_VCT_ATSC_audio:
        retVal = B_PSIP_eServiceAtscAudio;
        break;
    case PSIP_VCT_ATSC_data_only_service:
        retVal = B_PSIP_eServiceAtscData;
        break;
    default:
        retVal = B_PSIP_eServiceUnknown;
        break;
    }

    return retVal;
}

/***************************************************************************
Summary:
    Get the long channel name from the Extended Channel Name Descriptor

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
    State machine state that gets the channel data from the given VCT.

Description:
    State - GetChList
    SubState - GetChData

    Assumes that the VCT has already been saved in the given job's
    state data.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_State_GetChList_GetChData(
    B_PSIP_Job        * pJob,
    B_PSIP_StateEvent   event
    )
{
    B_PSIP_StateData_GetChList * pStateData = NULL;

    BDBG_ASSERT(NULL != pJob);
    BDBG_ASSERT(NULL != pJob->apiSettings.siHandle);

    pStateData = &pJob->stateData.getChListData;

    switch(event)
    {
    case B_PSIP_eStateEvent_Enter:
        {
            bool searchingPSI = false;
            while ((pStateData->channelIndex < PSIP_VCT_getNumChannels(pStateData->vctBuffer)) &&
                   (false == searchingPSI))
            {
                PSIP_VCT_channel vct;
                B_Error err = B_ERROR_SUCCESS;

                err = PSIP_VCT_getChannel(pStateData->vctBuffer,
                                          pStateData->channelIndex,
                                          &vct);

                if (B_ERROR_SUCCESS == err)
                {
                    if (0 == vct.minor_channel_number)
                    {
                        BDBG_MSG(("Skipping analog channel:%d", vct.minor_channel_number));
                        pStateData->channelIndex++;
                        continue;
                    }

                    memset(&pStateData->currentCh, 0, sizeof(B_PSIP_Channel));
                    /* note that for (B_PSIP_eCable == type), major/minor channel numbers
                       must be interpreted properly.  if the major/minor channel numbers
                       are below 1000, then the major/minor pair defines a 2 part channel
                       number.  if the 6 most sig bits of the major channel number are
                       111111, then use the following formula to calculate the 1 part
                       channel number: onePartNumber = (major & 0x00F)<<10 + minor; */
                    pStateData->currentCh.minor        = vct.minor_channel_number;
                    pStateData->currentCh.major        = vct.major_channel_number;
                    pStateData->currentCh.sourceId     = vct.source_id;
                    pStateData->currentCh.programNum   = vct.program_number;
                    pStateData->currentCh.modulation   = Psip2Si_ModulationMode(vct.modulation_mode);
                    pStateData->currentCh.serviceType  = Psip2Si_ServiceType(vct.service_type);
                    memcpy(pStateData->currentCh.chName, vct.short_name, MAX_CH_NAME_CHARS * sizeof(uint16_t));
                    B_PSIP_GetExtChannelNameFromECND(pJob->apiSettings.siHandle,
                                                   pStateData->vctBuffer,
                                                   pStateData->channelIndex,
                                                   &pStateData->currentCh);
                    B_PSIP_GetTimeShiftServices(pStateData->vctBuffer,
                                                pStateData->channelIndex,
                                                &pStateData->currentCh);


                    if (B_PSIP_eCable == pStateData->type)
                    {
                        pStateData->currentCh.pathSelect = vct.path_select;
                        pStateData->currentCh.OOB        = vct.out_of_band;
                    }

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

                    /* check service location descriptor for PIDs */
                    if (B_ERROR_SUCCESS == B_PSIP_GetPidsSLD(pStateData->vctBuffer,
                                                             pStateData->channelIndex,
                                                             &pStateData->currentCh))
                    {
                        pStateData->channelsFound++;

                        /* got all channel data so add to list of channels (if space is avail)*/
                        if (pStateData->channelsSaved < *(pStateData->pLength))
                        {
                            memcpy(&pStateData->chList[pStateData->channelsSaved],
                                   &pStateData->currentCh,
                                   sizeof(B_PSIP_Channel));
                            pStateData->channelsSaved++;
                        }
                        else
                        if (0 < *(pStateData->pLength))
                        {
                            BDBG_WRN(("not enough space in chList to save channel:%d.%d",
                                      pStateData->currentCh.major,
                                      pStateData->currentCh.minor));
                        }
                    }
                    else
                    {
                        /* couldn't find pids so use backup plan - search for PIDs in PSI data */
                        CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetPidsPSI_PAT);
                        STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                        searchingPSI = true;
                    }
                }
                else
                {
                    BDBG_WRN(("PSIP_VCT_getChannel %d failed", pStateData->channelIndex));
                }

                if (false == searchingPSI)
                {
                    pStateData->channelIndex++;
                }
            }

            if (false == searchingPSI)
            {
                pStateData->sectionIndex++;
                pStateData->channelIndex = 0;
                if (pStateData->sectionIndex <= pStateData->sectionIndexMax)
                {
                    /* get next section of vct */
                    CHANGE_STATE(pJob, B_PSIP_eState_GetChList_GetVCT);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
                else
                {
                    /* done looking thru all available vct sections */
                    CHANGE_STATE(pJob, B_PSIP_eState_GetChList_Done);
                    STATE_ENGINE(pJob, B_PSIP_eStateEvent_Enter);
                }
            }
        }
        break;
    case B_PSIP_eStateEvent_Cancel:
        CHANGE_STATE(pJob,B_PSIP_eState_GetChList_Done);
        STATE_ENGINE(pJob, event);
        break;
    default:
        BDBG_ERR(("invalid event:%d received in state:%s:%d",
                  event, __FUNCTION__, __LINE__));
        break;
    }
}

