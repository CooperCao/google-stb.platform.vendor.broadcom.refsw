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
 *      This is the header file for the RF4CE ZRC component static data declaration.
 *
*******************************************************************************/

#ifndef _RF4CE_ZRC_STATIC_DATA_H
#define _RF4CE_ZRC_STATIC_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"
#include "bbRF4CEPM.h"
#include "bbRF4CEZRCAttributes.h"
#include "bbRF4CEZRC1Attributes.h"
#include "bbRF4CEZRCConstants.h"
#include "bbRF4CEZRCBind.h"
#include "bbRF4CEZRC1CommandDiscovery.h"
#include "bbRF4CEZRCControlCommand.h"
#include "bbRF4CEZRC1Bind.h"
#include "private/bbRF4CEZRCTasks.h"
#include "private/bbRF4CEZRCPrivateAttributes.h"
#include "private/bbRF4CEZRCPrivateBind.h"
#include "bbSysNvmManager.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 Flags values.
 */
#define RF4CE_ZRC2_IN_BIND_REQUEST_FLAG 0x01
#define RF4CE_ZRC2_IN_PROXY_BIND_REQUEST_FLAG 0x02
#define RF4CE_ZRC2_SET_IN_BIND_FLAG(v) ((v) |= RF4CE_ZRC2_IN_BIND_REQUEST_FLAG)
#define RF4CE_ZRC2_CLEAR_IN_BIND_FLAG(v) ((v) &= ~RF4CE_ZRC2_IN_BIND_REQUEST_FLAG)
#define RF4CE_ZRC2_IS_IN_BIND(v) (((v) & RF4CE_ZRC2_IN_BIND_REQUEST_FLAG) != 0)
#define RF4CE_ZRC2_SET_IN_PROXY_BIND_FLAG(v) ((v) |= RF4CE_ZRC2_IN_PROXY_BIND_REQUEST_FLAG)
#define RF4CE_ZRC2_CLEAR_IN_PROXY_BIND_FLAG(v) ((v) &= ~RF4CE_ZRC2_IN_PROXY_BIND_REQUEST_FLAG)
#define RF4CE_ZRC2_IS_IN_PROXY_BIND(v) (((v) & RF4CE_ZRC2_IN_PROXY_BIND_REQUEST_FLAG) != 0)

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC static data indication queue.
 */
typedef struct _RF4CE_NWK_QueueDataIndParams_t
{
    SYS_QueueElement_t queueElement;               /*!< Queue service. */
    RF4CE_NWK_DataIndParams_t ind;                 /*!< Indication parameters. */
} RF4CE_NWK_QueueDataIndParams_t;

/**//**
 * \brief RF4CE ZRC constant data structure.
 */
typedef struct _RF4CE_ZRC_StaticConstData_t
{
    /*!< Currently is empty. */
} RF4CE_ZRC_StaticConstData_t;
extern const RF4CE_ZRC_StaticConstData_t rf4ceZRCStaticConstData;

/**//**
 * \brief RF4CE ZRC static data structure.
 */
typedef struct _RF4CE_ZRC_StaticData_t
{
    RF4CE_ZRC1_Attributes_t attributes;            /*!< ZRC1 Attributes. */
    uint8_t flagsBusy;                             /*!< Flags. */
#ifdef USE_RF4CE_PROFILE_ZRC2
    struct {
        RF4CE_ZRC2_SimpleAttributes_t       simple;
        RF4CE_ZRC2_PollConfigurationValue_t pollConfiguration; /* only peer's are used */
        uint8_t                             explicitPaddingForRamToNvmMapping1;
        uint16_t                            explicitPaddingForRamToNvmMapping2;
        RF4CE_ZRC2_MappableAction_t         mappableActions[RF4CE_ZRC2_ACTION_MAPPINGS_MAX];
    } zrc2Attributes[RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES + 1 /* local */];

    /* Home Automation attribute elements of all peers share the common table */
    Rf4ceZrc2HaRecord_t                     haTable[RF4CE_ZRC2_HA_ELEMENTS_MAX];

    union
    {
        /*!< to interface with NVM */
        RF4CE_ZRC2_Bitmap256_t          tmpBitmap256;
        Rf4ceZrc2ActionMappingBuffer_t  tmpActionMapping;
    };

#endif /* USE_RF4CE_PROFILE_ZRC2 */

    SYS_SchedulerTaskDescriptor_t zrcTasks;        /*!< ZRC1 Tasks. */
    SYS_QueueDescriptor_t commonDescr;             /*!< Common task. */
    SYS_QueueDescriptor_t getsetDescr;             /*!< Get/Set task. */
    SYS_QueueDescriptor_t ccDescr;                 /*!< Control Command task. */
    SYS_QueueDescriptor_t allDescr;                /*!< All? task. */
    SYS_QueueDescriptor_t heartbeatDescr;          /*!< Heartbeat task. */
    SYS_QueueDescriptor_t clientNotificationDescr; /*!< Client notification task. */
    union
    {
        RF4CE_NWK_DiscoveryReqDescr_t nwkDiscoveryRequest; /*!< Discovery Request. */
        RF4CE_NWK_PairReqDescr_t nwkPairRequest;   /*!< Pair Request. */
        RF4CE_UnpairReqDescr_t nwkUnpairRequest;   /*!< Unpair Request. */
        RF4CE_NWK_RXEnableReqDescr_t nwkRXEnableBind; /*!< RxEnable/Bind Request. */
        RF4CE_NWK_SetReqDescr_t nwkSetRequest;     /*!< Set Request. */
#ifdef RF4CE_TARGET
        RF4CE_NWK_AutoDiscoveryReqDescr_t autoDiscoveryRequest;
#endif /* RF4CE_TARGET */
    } nwkRequests;                                 /*!< NWK Requests container. */
    RF4CE_ZRC1_BindConfParams_t zrc1BindConfirm;   /*!< Bind confirm parameters. */
    Bool8_t isZRCStartUp;                          /*!< Is ZRC started up? */
#ifdef RF4CE_TARGET
    SYS_TimeoutTask_t zrc1TargetBindTimeout;
    uint8_t inTBindRequest;
#endif /* RF4CE_TARGET */

#if defined(USE_RF4CE_PROFILE_ZRC2)

    SYS_QueueDescriptor_t privateDescr;
    union
    {
        RF4CE_ZRC2_GetAttributesReqDescr_t getReq;
        RF4CE_ZRC2_SetAttributesReqDescr_t setReq;
        RF4CE_ZRC2_ConfigurationCompleteReqDescr_t confCompleteReq;
    } internalReqs;
    RF4CE_ZRC2_GetAttributesConfParams_t getConfirm;
    RF4CE_ZRC2_PullAttributesReqDescr_t pullReq;

    /* Private Get/Set variables */
    SYS_DataPointer_t payloadGet;
    uint16_t reqPayloadOffset;
    Bool8_t storeSimpleAttributesInNVM;

    RF4CE_ZRC2GDP2_Status_t statusGetSetAttribute;
    RF4CE_ZRC2GDP2_Status_t statusHeartbeat;
    RF4CE_ZRC2GDP2_Status_t statusClientNotification;

    RF4CE_ZRC2_BindingData_t bindingData;

    Rf4cezrc2GenericResponseRxCallback_t genericResponseCallback;

    /* Buffer for incomming indications. */
    RF4CE_ZRC_DataIncommingInd_t incommingDataBuffer[RF4CE_ZRC_MAX_INCOMING_INDICATIONS];
    /* Pool to take a free incomming indication entry. */
    SYS_QueueDescriptor_t incommingDataPool;
    /* Current indications queue to process. */
    SYS_QueueDescriptor_t incommingDataQueue;

    Bool8_t getSetPushPullWasConfirm;
    Bool8_t getSetPushPullWasResponse;
#endif /* defined(USE_RF4CE_PROFILE_ZRC2) */
} RF4CE_ZRC_StaticData_t;

/************************* DEFINITIONS *************************************************/
#define RF4CE_ZRC_STATIC_DATA_VAR_NAME rf4ceZRCStaticData

/**//**
 * \brief Common Stack Static Structure member.
 */
#define RF4CE_ZRC_STATIC_DATA_FIELD()               RF4CE_ZRC_StaticData_t RF4CE_ZRC_StaticDataDataField;

/**//**
 * \brief Common Stack Static Structure member access.
 */
#define GET_RF4CE_ZRC_STATIC_DATA_FIELD()   (&RF4CE_ZRC_STATIC_DATA_VAR_NAME.RF4CE_ZRC_StaticDataDataField)

/**//**
 * \brief Common Stack Static Structure member initialization.
 */
#ifdef RF4CE_CONTROLLER

#define ZRC1_TARGET_STATIC_DATA_INIT

#else /* RF4CE_CONTROLLER */

#define ZRC1_TARGET_STATIC_DATA_INIT \
    .zrc1TargetBindTimeout = \
    { \
        .taskDescriptor = &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrcTasks, \
        .handlerId = RF4CE_ZRC1_TARGET_BIND_TIMEOUT_HANDLER, \
    },

#endif /* RF4CE_CONTROLLER */

#define INIT_RF4CE_ZRC_STATIC_DATA_FIELD() \
.RF4CE_ZRC_StaticDataDataField = \
{ \
    .attributes = INIT_ZRC1_ATTRIBUTES, \
    .commonDescr = \
    { \
        .nextElement = NULL, \
    }, \
    .getsetDescr = \
    { \
        .nextElement = NULL, \
    }, \
    .ccDescr = \
    { \
        .nextElement = NULL, \
    }, \
    .allDescr = \
    { \
        .nextElement = NULL, \
    }, \
    .zrcTasks =  \
    { \
        .qElem = \
        { \
            .nextElement = NULL \
        }, \
        .priority = SYS_SCHEDULER_RF4CE_ZRC_PRIORITY, \
        .handlers = rf4cezrcTaskHandlers, \
        .handlersMask = 0, \
    }, \
    .isZRCStartUp = true, \
    ZRC1_TARGET_STATIC_DATA_INIT \
},

typedef struct _INT_RF4CE_ZRC_StaticData_t
{
    RF4CE_ZRC_STATIC_DATA_FIELD()
} INT_RF4CE_ZRC_StaticData_t;

extern INT_RF4CE_ZRC_StaticData_t RF4CE_ZRC_STATIC_DATA_VAR_NAME;

#endif /* _RF4CE_ZRC_STATIC_DATA_H */

/* eof bbRF4CEZRCStaticData.h */