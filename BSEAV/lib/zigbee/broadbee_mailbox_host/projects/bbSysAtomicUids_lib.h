/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

/*****************************************************************************************
*
* DESCRIPTION:
*       Atomic sections UIDs enumeration.
*
*****************************************************************************************/

#ifndef _BB_SYS_ATOMIC_UIDS_H
#define _BB_SYS_ATOMIC_UIDS_H


/************************* INCLUDES *****************************************************/
#include "bbSysOptions.h"           /* Compiler and SYS options setup. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Atomic sections UIDs enumeration.
 */
typedef enum _SysAtomicSectionUid_t
{
    /* Reserved UIDs - from 0x0000'0001 to 0x0000'FFFF. */

    SYS_ATOMIC_DEFAULT_UID                                                  = 0x00000000,

    /* These values are generated automatically. Do not edit them manually! */

    /* bbMl507Usart.c */
    UART_ENABLE                                                                                            = 0x00010000,
    UART_DISABLE                                                                                           = 0x00010001,

    /* bbSocPhyControl.c */
    HAL_calibrationOnIdle                                                                                  = 0x00010002,

    /* bbSocUart.c */
    SoC_UART_ENABLE                                                                                        = 0x00010003,
    SoC_UART_DISABLE                                                                                       = 0x00010004,

    /* bbHalMailboxUsart.c */
    UARTMAILBOX_WRITE2                                                                                     = 0x00010005,
    UARTMAILBOX_WRITE                                                                                      = 0x00010006,

    /* bbHalUsart.c */
    UART_WRITE2                                                                                            = 0x00010007,
    UART_WRITE                                                                                             = 0x00010008,

    /* bbSecurity.c */
    SYS_SECURITY_INIT_ATOMIC                                                                               = 0x00010009,

    /* bbSysPriorityQueue.h */
    PRIORITYQUEUE_PUT                                                                                      = 0x0001000a,
    PRIORITYQUEUE_REMOVE                                                                                   = 0x0001000b,
    PRIORITYQUEUE_EXTRACT                                                                                  = 0x0001000c,

    /* bbSysFsm.c */
    SYS_FSM_START_EVENT_HANDLING                                                                           = 0x0001000d,
    SYS_FSM_FINISH_EVENT_HANDLING                                                                          = 0x0001000e,

    /* bbSysTaskScheduler.c */
    SYS_SCHEDULER_INIT_0                                                                                   = 0x0001000f,
    SYS_SCHEDULER_RUN_TASK_0                                                                               = 0x00010010,
    SYS_SCHEDULER_RUN_TASK_1                                                                               = 0x00010011,
    SYS_SCHEDULER_RUN_TASK_2                                                                               = 0x00010012,
    SYS_SCHEDULER_POST_TASK_0                                                                              = 0x00010013,
    SYS_SCHEDULER_RECALL_TASK_0                                                                            = 0x00010014,

    /* bbMacPibApi.h */
    ATM_macPibApiSetPromiscuousMode                                                                        = 0x00010015,
    ATM_macPibApiSetRxOnWhenIdle                                                                           = 0x00010016,

    /* bbMacFeReqProcRxEnable.c */
    ATM_macFeReqProcRxEnableIssueConf                                                                      = 0x00010017,

    /* bbMacMemory.c */
    ATM_macMemoryPibReset                                                                                  = 0x00010018,

    /* bbPhyPibApi.h */
    ATM_phyPibApiSetTransmitPower                                                                          = 0x00010019,
    ATM_phyPibApiSetCcaMode                                                                                = 0x0001001a,
    ATM_phyPibApiSetCurrentChannelOnPage                                                                   = 0x0001001b,

} SysAtomicSectionUid_t;


#endif /* _BB_SYS_ATOMIC_UIDS_H */

/* eof bbSysAtomicUids.h */