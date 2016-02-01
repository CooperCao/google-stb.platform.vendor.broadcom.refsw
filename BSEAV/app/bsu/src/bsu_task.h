/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _TASK_H_
#define _TASK_H_

#if UCOS_VERSION==1
#define STACK UBYTE
#else
#define STACK CPU_STK
#endif

/* define task priorities */
#define ROOT_TASK_PRIORITY 20 /*240*/
#ifdef MOCA_SUPPORT
    #define MOCA_MONITOR_TASK_PRIORITY   16 /*201*/
    #define MOCA_INT_TASK_PRIORITY   17 /*202*/
#endif
#define RDWRCMP_TASK_PRIORITY 18 /*203*/
#define XCODE_SYSTEMDATA_TASK_PRIORITY 19 /*204*/

/* define task stack sizes */
#define ROOT_TASK_STACK_SIZE 0x8000
#ifdef MOCA_SUPPORT
    #define MOCA_MONITOR_TASK_STACK_SIZE 0x2000
    #define MOCA_INT_TASK_STACK_SIZE 0x2000
#endif
#define RDWRCMP_TASK_STACK_SIZE 0x2000

#if UCOS_VERSION==3
    extern OS_TCB RootTaskTCB;
    #ifdef MOCA_SUPPORT
        extern OS_TCB MoCAIntTaskTCB;
        extern OS_TCB MoCAMonitorTaskTCB;
    #endif
#endif

/* define task stack */
extern STACK RootTaskStack[ROOT_TASK_STACK_SIZE];
#ifdef MOCA_SUPPORT
    extern STACK MocaMonitorTaskStack[MOCA_MONITOR_TASK_STACK_SIZE];
    extern STACK MocaIntTaskStack[MOCA_INT_TASK_STACK_SIZE];
#endif

/* declare task functions */
//void root_task(void * param);

#endif /* _TASK_H_ */
