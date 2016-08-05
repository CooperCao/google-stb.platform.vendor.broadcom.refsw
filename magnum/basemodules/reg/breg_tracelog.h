/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

/*= Module Overview ********************************************************
This module supplies the function required to log events with the
TraceLogger. Each software module has fixed number of events which
are mapped to unique sentinel registers, and software module create event
with arbitrary 32-bit value. Then these events could be used with
the TraceLogger tool.

ModuleID  -  is the unique token, currently tokens are shared with the BERR interface

***************************************************************************/

#ifndef BREG_TRACELOG_H
#define BREG_TRACELOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if 0
/***************************************************************************
Summary:
    Registers software module for the trace log interface

Description:
    This macro is used to register software module in the system. It should be called
    at least once. Result of this macro is printout on the debug console  MEMC_SENTINEL
    address range allocated for the module.

Example:
    #include "bstd.h"
    int foo_open(BREG_Handle reg)
    {
        BREG_TRACELOG_REGISTER(reg, FOO);
    }

Input:
    reg - instance of BREG_Handle
    moduleId - unique module ID

Returns:
    <none>
****************************************************************************/
#define BREG_TRACELOG_REGISTER(regHandle, moduleId)

/***************************************************************************
Summary:
    Unregisters software module for the trace log interface

Example:
    #include "bstd.h"
    int foo_close(BREG_Handle reg)
    {
        BREG_TRACELOG_UNREGISTER(reg, FOO);
    }

Input:
    reg - instance of BREG_Handle
    moduleId - unique module ID

Returns:
    <none>
****************************************************************************/
#define BREG_TRACELOG_REGISTER(regHandle, moduleId)

/***************************************************************************
Summary:
    Records unique event

Description:
    This macro is used to record unique event with arbitrary 32-bit data.
    Then these events could be used with the TraceLogger tool.

Example:
    #include "bstd.h"
    int foo_start(BREG_Handle reg)
    {
        BREG_TRACELOG_EVENT(reg, FOO, 3, 1); // event #3 used to notify about start/stop condition. For this event, data '1' means start
    }

    int foo_start(BREG_Handle reg)
    {
        BREG_TRACELOG_EVENT(reg, FOO, 3, 1); // event #3 used to notify about start/stop condition. For this event, data '0' means stop
    }

Input:
    reg - instance of BREG_Handle
    moduleId - unique module ID
    event - event number (currently 0 through 15 are allowed). This number needs to be resolved to the compile time constant
    data - arbitrary number

Returns:
    <none>
****************************************************************************/
#define BREG_TRACELOG_EVENT(regHandle,moduleId,event,data)
#endif /* 0 */

#define BREG_P_TRACELOG_MAX_MODULES 0x200
#define BREG_P_TRACELOG_MAX_EVENTS_PER_DEVICE   16

#define BREG_TRACELOG_EVENT(regHandle,moduleId,event,data) do {BDBG_CASSERT((BERR_##moduleId##_ID)<BREG_P_TRACELOG_MAX_MODULES);BDBG_CASSERT((event)<BREG_P_TRACELOG_MAX_EVENTS_PER_DEVICE);BREG_P_Tracelog_Event_isrsafe(regHandle, (BERR_##moduleId##_ID * BREG_P_TRACELOG_MAX_EVENTS_PER_DEVICE + (event)),(data));} while(0)

#define BREG_TRACELOG_REGISTER(regHandle, moduleId) BREG_P_Tracelog_Register(regHandle, BERR_##moduleId##_ID, #moduleId)
#define BREG_TRACELOG_UNREGISTER(regHandle, moduleId) BREG_P_Tracelog_Register(regHandle, BERR_##moduleId##_ID, NULL)

void BREG_P_Tracelog_Event_isrsafe(BREG_Handle RegHandle, unsigned offset, /* offset from MEMC_SENTINEL_0_0 */ uint32_t data);
void BREG_P_Tracelog_Register(BREG_Handle RegHandle, unsigned moduleId, const char *moduleName);

#ifdef __cplusplus
}
#endif

#endif
