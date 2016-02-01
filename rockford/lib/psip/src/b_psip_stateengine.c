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
*    Library routines for the PSIP applib state machine
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
#include <sys/time.h>
#include <time.h>

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"
#include "b_psip_utils.h"
#include "b_psip_states_decl.h"

/***************************************************************************
Summary:
Array of all possible state functions.
***************************************************************************/
static void (* const B_PSIP_StateFunction[B_PSIP_eState_MaxStates])(B_PSIP_Job * pJob, B_PSIP_StateEvent event) =
{
    NULL,                                   /* B_PSIP_eState_Idle */
    B_PSIP_State_GetTable,                  /* B_PSIP_eState_GetTable */
    B_PSIP_State_GetChList_GetVCT,          /* B_PSIP_eState_GetChList_GetVCT */
    B_PSIP_State_GetChList_GetChData,       /* B_PSIP_eState_GetChList_GetChData */
    B_PSIP_State_GetChList_GetPidsPSI_PAT,  /* B_PSIP_eState_GetChList_GetPidsPSI_PAT */
    B_PSIP_State_GetChList_GetPidsPSI_PMT,  /* B_PSIP_eState_GetChList_GetPidsPSI_PMT */
    B_PSIP_State_GetChList_Done,            /* B_PSIP_eState_GetChList_Done */
    B_PSIP_State_GetProgInfo_GetSTT,        /* B_PSIP_eState_GetProgInfo_GetSTT */
    B_PSIP_State_GetProgInfo_GetVCT,        /* B_PSIP_eState_GetProgInfo_GetVCT */
    B_PSIP_State_GetProgInfo_GetSourceId,   /* B_PSIP_eState_GetProgInfo_GetSourceId */
    B_PSIP_State_GetProgInfo_GetMGT,        /* B_PSIP_eState_GetProgInfo_GetMGT */
    B_PSIP_State_GetProgInfo_SearchMGT4EIT, /* B_PSIP_eState_GetProgInfo_SearchMGT4EIT */
    B_PSIP_State_GetProgInfo_GetEIT,        /* B_PSIP_eState_GetProgInfo_GetEIT */
    B_PSIP_State_GetProgInfo_SearchEIT,     /* B_PSIP_eState_GetProgInfo_SearchEIT */
    B_PSIP_State_GetProgInfo_SearchMGT4ETT, /* B_PSIP_eState_GetProgInfo_SearchMGT4ETT */
    B_PSIP_State_GetProgInfo_GetETT,        /* B_PSIP_eState_GetProgInfo_GetETT */
    B_PSIP_State_GetProgInfo_SaveProgram,   /* B_PSIP_eState_GetProgInfo_SaveProgram */
    B_PSIP_State_GetProgInfo_Done,          /* B_PSIP_eState_GetProgInfo_Done */
    B_PSIP_State_GetTableVer_GetMGT,        /* B_PSIP_eState_GetTableVer_GetMGT */
    B_PSIP_State_GetTableVer_Done,          /* B_PSIP_eState_GetTableVer_Done */
    B_PSIP_State_GetTime_GetSTT,            /* B_PSIP_eState_GetTime_GetSTT */
    B_PSIP_State_GetTime_Done,              /* B_PSIP_eState_GetTime_Done */
};
/* !!! changes to this array must also have a corresponding change to the B_PSIP_State enum in b_psip_states.h !!! */

/***************************************************************************
Summary:
    Advance state machine with new event

Description:

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_StateEngine(
	B_PSIP_Job * pJob, 
	B_PSIP_StateEvent event, 
	const char * func, 
	const uint32_t line)
{
    BDBG_ASSERT(NULL != pJob);
    BDBG_MSG(("job:%p ---> %s:%d - sending event:%d to state:%d",
              (pJob), func, line, event, pJob->state));

    if (B_PSIP_StateFunction[pJob->state])
    {
        B_PSIP_StateFunction[pJob->state](pJob, event);
    }
    else
    {
        BDBG_ERR(("job:%p ---> NULL state encountered - ignoring!", (pJob), func, line));
    }
}
