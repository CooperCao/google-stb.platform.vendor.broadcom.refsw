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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Security handlers static data.
 *
*******************************************************************************/

#ifndef _BB_SECURITY_DATA_H
#define _BB_SECURITY_DATA_H


/************************* INCLUDES *****************************************************/
#include "bbSecurity.h"            /* Security header. */
#include "bbSysTaskScheduler.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Type of the Security internal data structure.
 */
typedef struct _SYS_SecurityData_t
{
    SYS_SchedulerTaskDescriptor_t descriptor;
    SYS_QueueDescriptor_t queue;
    Security_CCMReq_t *request;
    uint8_t state;
    uint8_t length;
    uint8_t lengthA;
    uint8_t lengthM;
    uint8_t iteration;
    uint8_t buffer[16];
} SYS_SecurityData_t;

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Common Security Data Structure member.
 */
#define SECURITY_DATA_FIELD()       SYS_SecurityData_t SYS_SecurityField;

/**//**
 * \brief Common Security Data Structure member access.
 */
#define GET_SECURITY_DATA_FIELD() (&sysCommonStackData.SYS_SecurityField)

/**//**
 * \brief Common Security Data Structure member initialization.
 */

#define INIT_SECURITY_DATA_FIELD() \
.SYS_SecurityField = \
{ \
    .descriptor = \
    { \
        .qElem = \
        { \
          .nextElement = NULL \
        }, \
        .priority = SYS_SCHEDULER_MAC_PRIORITY, \
        .handlers = NULL, \
        .handlersMask = 0, \
    }, \
    .queue = \
    { \
        .nextElement = NULL \
    }, \
    .request = NULL, \
},

#endif /* _BB_SECURITY_DATA_H */

/* eof bbSecurityData.h */