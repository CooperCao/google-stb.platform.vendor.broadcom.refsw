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

/******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the MSO RF4CE Profile
 *      Static Data.
 *
*******************************************************************************/

#ifndef _RF4CE_MSO_STATIC_DATA_H
#define _RF4CE_MSO_STATIC_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysTaskScheduler.h"
#include "bbSysMemMan.h"
#include "bbSysTimeoutTask.h"
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CEMSOProfileAttributes.h"
#include "bbRF4CENWKData.h"
#include "bbRF4CEPMStartReset.h"
#include "bbRF4CENWKRX.h"
#include "bbRF4CEMSORIB.h"
#include "bbRF4CEMSOValidation.h"
#include "bbRF4CEMSOBind.h"
#include "bbRF4CENWKDiscovery.h"
#include "bbRF4CENWKPair.h"
#include "bbRF4CEMSOUserControl.h"
#include "bbSysNvmManager.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO static data structure.
 */
typedef struct _RF4CE_MSO_StaticData_t
{
    RF4CE_MSO_ProfileAttributes_t attributes;      /*!< MSO Attributes */
    SYS_SchedulerTaskDescriptor_t tasks;           /*!< Existing profile tasks */
    SYS_QueueDescriptor_t startResetBindQueue;     /*!< Binding queue */
    SYS_QueueDescriptor_t startGetSetPAQueue;      /*!< Profile attribute Get/Set queue */
    SYS_QueueDescriptor_t startGetSetRIBQueue;     /*!< RIB attribute Get/Set queue */
    SYS_QueueDescriptor_t ucQueue;                 /*!< User Control Queue */
#ifdef RF4CE_TARGET
    uint8_t ribIRDBMask[32];
    NVM_ReadFileIndDescr_t readNVM;
    NVM_WriteFileIndDescr_t writeNVM;
#endif /* RF4CE_TARGET */
    uint8_t tasksBusy;                             /*!< Busy tasks */
#ifdef RF4CE_CONTROLLER
    RF4CE_NWK_RXEnableReqDescr_t rxOnOffBind;
    RF4CE_MSO_GetRIBAttributeConfParams_t confirmGetRIB;
    SYS_DataPointer_t bindData;
    RF4CE_MSO_BindConfParams_t confirmBind;
    SYS_TimeoutTask_t bindTimeout;
#endif /* RF4CE_CONTROLLER */

    union
    {
#ifdef RF4CE_CONTROLLER
        RF4CE_NWK_DiscoveryReqDescr_t discoveryRequest;
        RF4CE_NWK_PairReqDescr_t pairRequest;
#endif /* RF4CE_CONTROLLER */
        RF4CE_NWK_UnpairReqDescr_t unpairRequest;  /*!< Unpair request */
    } nwk;                                         /*!< NWK request container */
#ifdef RF4CE_TARGET
    uint8_t cvpRef;
#endif /* RF4CE_TARGET */
    uint8_t nBindData;                             /*!< Internal Bind Data */
    uint8_t iPairing;                              /*!< Internal Pairing Data */
} RF4CE_MSO_StaticData_t;

/************************* DEFINITIONS *************************************************/
#define RF4CE_MSO_STATIC_DATA_VAR_NAME rf4ceMSOStaticData

/**//**
 * \brief Common Stack Static Structure member.
 */
#define RF4CE_MSO_STATIC_DATA_FIELD()               RF4CE_MSO_StaticData_t RF4CE_MSO_StaticDataDataField;

/**//**
 * \brief Common Stack Static Structure member access.
 */
#define GET_RF4CE_MSO_STATIC_DATA_FIELD()   (&RF4CE_MSO_STATIC_DATA_VAR_NAME.RF4CE_MSO_StaticDataDataField)

/**//**
 * \brief Common Stack Static Structure member initialization.
 */
extern const SYS_SchedulerTaskHandler_t rf4ceMSOTaskHandlers[];
#define INIT_RF4CE_MSO_STATIC_DATA_FIELD() \
.RF4CE_MSO_StaticDataDataField = \
{ \
    .attributes = \
    { \
        RF4CE_MSO_INIT_PROFILE_ATTRIBUTES \
    }, \
    .tasks = \
    { \
        .qElem = \
        { \
            .nextElement = NULL, \
        }, \
        .priority = SYS_SCHEDULER_RF4CE_MSO_PRIORITY, \
        .handlers = rf4ceMSOTaskHandlers, \
        .handlersMask = 0, \
    }, \
},

typedef struct _INT_RF4CE_MSO_StaticData_t
{
    RF4CE_MSO_STATIC_DATA_FIELD()
} INT_RF4CE_MSO_StaticData_t;

extern INT_RF4CE_MSO_StaticData_t RF4CE_MSO_STATIC_DATA_VAR_NAME;

#endif /* _RF4CE_MSO_STATIC_DATA_H */

/* eof bbRF4CEMSOStaticData.h */