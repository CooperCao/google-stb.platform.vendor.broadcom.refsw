/***************************************************************************
*     (c)2012-2013 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_BASE_STATISTICS_H
#define NEXUS_BASE_STATISTICS_H

#include "nexus_base_stackcheck.h"

#if !B_REFSW_SYSTEM_MODE_CLIENT
#if BDBG_DEBUG_BUILD
#ifndef NEXUS_P_CALLBACK_STATS
#define NEXUS_P_CALLBACK_STATS  1
#endif
#ifndef NEXUS_P_MODULE_STATS
#define NEXUS_P_MODULE_STATS    1
#endif
#ifndef NEXUS_P_API_STATS
#define NEXUS_P_API_STATS       1
#endif
#endif
#endif
/* this would need to use some stack (default 64 bytes) for bookkeeping purposes, e.g. it increases stack utilization  and on each test it would read twice entire stack */
#define NEXUS_P_STACKDEPTH_STATS 0

typedef struct NEXUS_P_Base_Stats_Cookie {
    NEXUS_Time timeStart;
#if NEXUS_P_STACKDEPTH_STATS
    NEXUS_P_Base_StackCheck_Frame stackFrame;
#endif
} NEXUS_P_Base_Stats_Cookie;

NEXUS_Error NEXUS_P_Base_Stats_Init(void);
void NEXUS_P_Base_Stats_GetCookie(NEXUS_P_Base_Stats_Cookie *cookie);
void NEXUS_P_Base_Stats_Uninit(void);
void NEXUS_P_Base_Stats_CallbackEvent(const NEXUS_P_Base_Stats_Cookie *cookie, NEXUS_ModulePriority priority, const char *kind, void *callback, void *destination, const char *file, unsigned lineNo);
void NEXUS_P_Base_Stats_ModuleEvent(const NEXUS_Time *timeStart, NEXUS_ModuleHandle module, const char *pFileName, unsigned lineNumber);
void NEXUS_P_Base_Stats_ApiEvent(const NEXUS_P_Base_Stats_Cookie *cookie, const char *api,NEXUS_ModuleHandle module);
void NEXUS_P_Base_Stats_Report(void);


#if NEXUS_P_STACKDEPTH_STATS
#define NEXUS_P_STATS_GETCOOKIE(cookie) NEXUS_P_Base_Stats_GetCookie(cookie)
#else
#define NEXUS_P_STATS_GETCOOKIE(cookie) NEXUS_Time_Get(&(cookie)->timeStart)
#endif

#if NEXUS_P_CALLBACK_STATS
#define NEXUS_P_CALLBACK_STATS_STATE()  NEXUS_P_Base_Stats_Cookie stats_Cookie
#define NEXUS_P_CALLBACK_STATS_START()  NEXUS_P_STATS_GETCOOKIE(&stats_Cookie)
#define NEXUS_P_CALLBACK_STATS_STOP(kind,callback,destination,file,lineNo)  NEXUS_P_Base_Stats_CallbackEvent(&stats_Cookie, scheduler->priority, #kind, callback, (void *)(unsigned long)destination, file, lineNo)
#else
#define NEXUS_P_CALLBACK_STATS_STATE()
#define NEXUS_P_CALLBACK_STATS_START()
#define NEXUS_P_CALLBACK_STATS_STOP(kind,callback,destination,file,lineNo)
#endif

#if NEXUS_P_MODULE_STATS
#define NEXUS_P_MODULE_STATS_STATE()    NEXUS_Time stats_TimeStart
#define NEXUS_P_MODULE_STATS_START()    NEXUS_Time_Get(&stats_TimeStart)
#define NEXUS_P_MODULE_STATS_STOP()     NEXUS_P_Base_Stats_ModuleEvent(&stats_TimeStart, module, pFileName, lineNumber)
#else
#define NEXUS_P_MODULE_STATS_STATE()
#define NEXUS_P_MODULE_STATS_START()
#define NEXUS_P_MODULE_STATS_STOP()
#endif

#if NEXUS_P_API_STATS
#define NEXUS_P_API_STATS_STATE()    NEXUS_P_Base_Stats_Cookie stats_Cookie
#define NEXUS_P_API_STATS_START()    NEXUS_P_STATS_GETCOOKIE(&stats_Cookie)
#define NEXUS_P_API_STATS_STOP(api,module)     NEXUS_P_Base_Stats_ApiEvent(&stats_Cookie, api, module)
#else
#define NEXUS_P_API_STATS_STATE()
#define NEXUS_P_API_STATS_START()
#define NEXUS_P_API_STATS_STOP(api,module)
#endif

typedef struct NEXUS_P_Base_Statistics_StackAggregate {
    unsigned max;
    unsigned count;
    unsigned total;
} NEXUS_P_Base_Statistics_StackAggregate;

void NEXUS_P_Base_Statistics_StackAggregate_Update(NEXUS_P_Base_Statistics_StackAggregate *aggregate, const NEXUS_P_Base_StackCheck_Frame *frame);
#if NEXUS_P_STACKDEPTH_STATS 
#define NEXUS_P_STACKDEPTH_AGGREGATE() static NEXUS_P_Base_Statistics_StackAggregate g_stackAggregate
#define NEXUS_P_STACKDEPTH_STATE()  NEXUS_P_Base_StackCheck_Frame stackFrame
#define NEXUS_P_STACKDEPTH_START()  NEXUS_P_Base_StackCheck_MakeFrame(&stackFrame, &stackFrame)
#define NEXUS_P_STACKDEPTH_STOP()  NEXUS_P_Base_Statistics_StackAggregate_Update(&g_stackAggregate, &stackFrame)
#define NEXUS_P_STACKDEPTH_RESET()  do { g_stackAggregate.max = 0; g_stackAggregate.count = 0; } while(0)
#else
#define NEXUS_P_STACKDEPTH_AGGREGATE() extern NEXUS_P_Base_Statistics_StackAggregate _g_stackAggregate
#define NEXUS_P_STACKDEPTH_STATE()  
#define NEXUS_P_STACKDEPTH_START() 
#define NEXUS_P_STACKDEPTH_STOP()  
#define NEXUS_P_STACKDEPTH_RESET() 
#endif



#endif /* !defined NEXUS_BASE_STATISTICS_H */

