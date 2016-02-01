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
*    Library typedefs for PSIP state machine states and events.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef B_PSIP_STATES_H__
#define B_PSIP_STATES_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
All available states used in state machine 
***************************************************************************/
typedef enum B_PSIP_State
{
    B_PSIP_eState_Idle,                         /* state: Idle */
    B_PSIP_eState_GetTable,                     /* state: Get PSIP table */
    B_PSIP_eState_GetChList_GetVCT,             /* state: Get channel list, substate: get VCT table*/
    B_PSIP_eState_GetChList_GetChData,          /* state: Get channel list, substate: get channel data from VCT */
    B_PSIP_eState_GetChList_GetPidsPSI_PAT,     /* state: Get channel list, substate: alt. method of finding PIDs if VCT is unavailable */
    B_PSIP_eState_GetChList_GetPidsPSI_PMT,     /* state: Get channel list, substate: alt. method of finding PIDs if VCT is unavailable */
    B_PSIP_eState_GetChList_Done,               /* state: Get channel list, substate: clean up state data and respond to original caller */
    B_PSIP_eState_GetProgInfo_GetSTT,           /* state: Get program info, substate: get STT table */
    B_PSIP_eState_GetProgInfo_GetVCT,           /* state: Get program info, substate: get VCT table */
    B_PSIP_eState_GetProgInfo_GetSourceId,      /* state: Get program info, substate: get source id */
    B_PSIP_eState_GetProgInfo_GetMGT,           /* state: Get program info, substate: get MGT table */
    B_PSIP_eState_GetProgInfo_SearchMGT4EIT,    /* state: Get program info, substate: search for EIT tables in MGT */
    B_PSIP_eState_GetProgInfo_GetEIT,           /* state: Get program info, substate: get EIT table */
    B_PSIP_eState_GetProgInfo_SearchEIT,        /* state: Get program info, substate: find event(s) matching time range in EIT table */
    B_PSIP_eState_GetProgInfo_SearchMGT4ETT,    /* state: Get program info, substate: search for ETT tables in MGT */
    B_PSIP_eState_GetProgInfo_GetETT,           /* state: Get program info, substate: get ETT table */
    B_PSIP_eState_GetProgInfo_SaveProgram,      /* state: Get program info, substate: save program */
    B_PSIP_eState_GetProgInfo_Done,             /* state: Get program info, substate: clean up state data and respond to original caller */
    B_PSIP_eState_GetTableVer_GetMGT,           /* state: Get table versions, substate: get MGT table */
    B_PSIP_eState_GetTableVer_Done,             /* state: Get table versions, substate: clean up state data and respond */
    B_PSIP_eState_GetTime_GetSTT,               /* state: Get time, substate: get STT table */
    B_PSIP_eState_GetTime_Done,                 /* state: Get time, substate: clean up time data and respond */
    B_PSIP_eState_MaxStates                     /* not a valid state */
} B_PSIP_State;
/* !!! changes to this enum must also have a corresponding change to B_PSIP_StateFunction in b_psip_stateengine.c !!! */


/***************************************************************************
Summary:
Events that drive state transistions within the PSIP state machine 
***************************************************************************/
typedef enum B_PSIP_StateEvent
{
    B_PSIP_eStateEvent_Enter,           /* enter */
    B_PSIP_eStateEvent_Cancel,          /* cancel */
    B_PSIP_eStateEvent_Timeout,         /* timeout */
    B_PSIP_eStateEvent_TableTimeout,    /* table timeout */
    B_PSIP_eStateEvent_TableReceived,   /* previously requested PSIP table data is available */
    B_PSIP_eStateEvent_Failure          /* failure */
} B_PSIP_StateEvent;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_STATES_H__ */

