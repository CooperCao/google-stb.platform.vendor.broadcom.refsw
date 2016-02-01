/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkRouteRecord.h $
*
* DESCRIPTION:
*   Contains declarations related to Route Record command handler.
*
* $Revision: 3816 $
* $Date: 2014-10-02 07:46:11Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_ROUTE_RECORD_H
#define _ZBPRO_NWK_ROUTE_RECORD_H

/************************* INCLUDES ****************************************************/
#include "bbSysQueue.h"
#include "bbSysTimeoutTask.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Initializer for Route Record service descriptor.
 */
#define NWK_ROUTE_RECORD_SERVICE_DESCRIPTOR { \
    .payloadSize    = zbProNwkRouteRecordSize, \
    .fill           = zbProNwkRouteRecordFill, \
    .conf           = zbProNwkRouteRecordConf, \
    .ind            = zbProNwkRouteRecordInd, \
    .reset          = zbProNwkRouteRecordReset, \
}

/**//**
 * \brief Initializer for Transiting of Route Record service descriptor.
 */
#define NWK_ROUTE_RECORD_TRANSIT_SERVICE_DESCRIPTOR { \
    .payloadSize    = NULL, \
    .fill           = NULL, \
    .conf           = NULL, \
    .ind            = zbProNwkRouteRecordTransitInd, \
    .reset          = NULL, \
}

/**//**
 * \brief Length of base part of Route record command payload. ZigBee Spec r20, 3.4.5.
 */
#define ZBPRO_NWK_ROUTE_RECORD_BASE_PAYLOAD_LENGTH    \
    ( /* Command Id */ 1 +                            \
      /* Relay count */ 1)

/**//**
 * \brief Calculates the exact length of Route record command payload. ZigBee Spec r20, 3.4.5.
 */
#define ZBPRO_NWK_ROUTE_RECORD_PAYLOAD_LENGTH(relaysAmount) \
    (ZBPRO_NWK_ROUTE_RECORD_BASE_PAYLOAD_LENGTH + \
     relaysAmount * sizeof(ZbProNwkRouteRecordRelayListEntity_t))

/**//**
 * \brief Determines if the output buffer contains route record command.
 */
#define ZBPRO_NWK_IS_NOT_ROUTE_RECORD(outputBuffer) \
    (ZBPRO_NWK_ROUTE_RECORD_SID != (outputBuffer)->sId && \
     ZBPRO_NWK_ROUTE_RECORD_TRANSIT_SID != (outputBuffer)->sId)

/**//**
 * \brief This field contains the number of relays in the relay list field of
 *        the route record command. ZigBee spec r20, 3.4.5.3.1.
 */
typedef uint8_t ZbProNwkRouteRecordRelayCount_t;

/**//**
 * \brief The relay list field is a list of the 16-bit network addresses of the nodes
 *        that have relayed the packet. ZigBee spec r18, 3.4.5.3.2.
 */
typedef ZBPRO_NWK_NwkAddr_t ZbProNwkRouteRecordRelayListEntity_t;

/**//**
 * \brief Internal states of the route record handler component.
 */
typedef enum _ZbProNwkRouteRecordState_t
{
    NWK_ROUTE_RECORD_IDLE_STATE = 0x2E,
    NWK_ROUTE_RECORD_FIRST_STATE = 0xF6,
    NWK_ROUTE_RECORD_BEGIN_STATE = NWK_ROUTE_RECORD_FIRST_STATE,
    NWK_ROUTE_RECORD_PREPARE_STATE = 0xF7,
    NWK_ROUTE_RECORD_SEND_STATE = 0xF8,
    NWK_ROUTE_RECORD_AWAITING_DELIVERY_STATE = 0xF9,
    NWK_ROUTE_RECORD_CONFIRM_STATE = 0xFA,
    NWK_ROUTE_RECORD_LAST_STATE
} ZbProNwkRouteRecordState_t;

/**//**
 * \brief Internal memory of Route Record handler.
 */
typedef struct _ZbProNwkRouteRecordDescr_t
{
    SYS_QueueDescriptor_t       queue;      /*!< Queue of requests from other NWK components. */
    ZbProNwkRouteRecordState_t  state;      /*!< finite-state machine. */
    SYS_TimeoutSignal_t         timer;
} ZbProNwkRouteRecordDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
  \brief Returns memory size required for route request header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t zbProNwkRouteRecordSize;

/************************************************************************************//**
    \brief Initiates route record transmission.
    \param[in] outputBuffer - pointer to the output buffer.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkRouteRecordReq(ZbProNwkOutputBuffer_t *const outputBuffer);

/************************************************************************************//**
    \brief Route Record task handler.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkRouteRecordHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Resets the Route Record command handler.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkRouteRecordReset;

/************************************************************************************//**
    \brief Fills route record header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkRouteRecordFill;

/************************************************************************************//**
    \brief This function invoked when route record frame has been received.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRouteRecordInd;

/************************************************************************************//**
    \brief Receives a transmission confirm for Route Record Command.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkRouteRecordConf;

NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRouteRecordTransitInd;

#endif /* _ZBPRO_NWK_ROUTE_RECORD_H */
