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
*   API name: Event
*    Library routines for event management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"

BDBG_MODULE(b_os_event);

/***************************************************************************
Summary:
Get default event settings.
***************************************************************************/
void B_Event_GetDefaultSettings(
    B_EventSettings *pSettings  /* [out] */
    )
{
    /* Empty Structure */
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Create an event
***************************************************************************/
B_EventHandle B_Event_Create(
    const B_EventSettings *pSettings    /* Pass NULL for defaults */
    )
{
    BKNI_EventHandle event;
    BERR_Code errCode;

    BSTD_UNUSED(pSettings);     /* Empty struct for now */

    errCode = BKNI_CreateEvent(&event);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return NULL;
    }

    return event;
}

/***************************************************************************
Summary:
Destroy an event
***************************************************************************/
void B_Event_Destroy(
    B_EventHandle event
    )
{
    BDBG_ASSERT(NULL != event);
    BKNI_DestroyEvent(event);
}

/***************************************************************************
Summary:
Set an event
***************************************************************************/
void B_Event_Set(
    B_EventHandle event
    )
{
    BDBG_ASSERT(NULL != event);
    BKNI_SetEvent(event);
}

/***************************************************************************
Summary:
Wait for an event to be set

Description:
This routine will wait for the specified event to be set.  If the event
was set, B_Error_eSuccess will be returned.  If the event was not set,
B_Error_eTimeout will be returned.
***************************************************************************/
B_Error B_Event_Wait(
    B_EventHandle event, 
    int timeoutMsec         /* Timeout in milliseconds.  Pass B_WAIT_FOREVER to wait 
                               forever or B_WAIT_NONE to avoid blocking */
    )
{
    BDBG_ASSERT(NULL != event);
    return (B_Error)BKNI_WaitForEvent(event, timeoutMsec);
}

/***************************************************************************
Summary:
Clear an event if it has been set
***************************************************************************/
void B_Event_Reset(
    B_EventHandle event
    )
{
    BDBG_ASSERT(NULL != event);
    BKNI_ResetEvent(event);
}

/***************************************************************************
Summary:
Get default event group settings
***************************************************************************/
void B_EventGroup_GetDefaultSettings(
    B_EventGroupSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(pSettings); /* Empty Structure */
}

/***************************************************************************
Summary:
Create an event group
***************************************************************************/
B_EventGroupHandle B_EventGroup_Create(
    const B_EventGroupSettings *pSettings
    )
{
    BERR_Code errCode;
    BKNI_EventGroupHandle group;

    BSTD_UNUSED(pSettings); /* Empty Structure */

    errCode = BKNI_CreateEventGroup(&group);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return NULL;
    }

    return group;
}

/***************************************************************************
Summary:
Destroy an event group
***************************************************************************/
void B_EventGroup_Destroy(
    B_EventGroupHandle group
    )
{
    BDBG_ASSERT(NULL != group);
    BKNI_DestroyEventGroup(group);
}

/***************************************************************************
Summary:
Add an event to the group
***************************************************************************/
B_Error B_EventGroup_AddEvent(
    B_EventGroupHandle group, 
    B_EventHandle event
    )
{
    BERR_Code errCode;

    BDBG_ASSERT(NULL != group);
    BDBG_ASSERT(NULL != event);

    errCode = BKNI_AddEventGroup(group, event);
    if ( errCode )
    {
        return (B_Error)BERR_TRACE(errCode);
    }

    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
Remove an event from the group
***************************************************************************/
B_Error B_EventGroup_RemoveEvent(
    B_EventGroupHandle group, 
    B_EventHandle event
    )
{
    BERR_Code errCode;

    BDBG_ASSERT(NULL != group);
    BDBG_ASSERT(NULL != event);

    errCode = BKNI_RemoveEventGroup(group, event);
    if ( errCode )
    {
        return (B_Error)BERR_TRACE(errCode);
    }

    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
Wait for any event in the group to be set.

Description:
This routine will wait for the specified event to be set.  If the event
was set, B_Error_eSuccess will be returned.  If the event was not set,
B_Error_eTimeout will be returned.
***************************************************************************/
B_Error B_EventGroup_Wait(
    B_EventGroupHandle group, 
    int timeoutMsec,                    /* Timeout in milliseconds.  Pass B_WAIT_FOREVER to wait 
                                           forever or B_WAIT_NONE to avoid blocking */
    B_EventHandle *pTriggeredEvents,    /* Triggered events handles will be copied into this array */
    unsigned maxTriggeredEvents,        /* Size of triggered event array */
    unsigned *pNumTriggeredEvents       /* [out] Number of triggered events */
    )
{
    BDBG_ASSERT(NULL != group);
    BDBG_ASSERT(NULL != pTriggeredEvents);
    BDBG_ASSERT(maxTriggeredEvents > 0);
    BDBG_ASSERT(NULL != pNumTriggeredEvents);

    return (B_Error) BKNI_WaitForGroup(group,
                                       timeoutMsec,
                                       (BKNI_EventHandle *)pTriggeredEvents,
                                       maxTriggeredEvents,
                                       pNumTriggeredEvents);
}

