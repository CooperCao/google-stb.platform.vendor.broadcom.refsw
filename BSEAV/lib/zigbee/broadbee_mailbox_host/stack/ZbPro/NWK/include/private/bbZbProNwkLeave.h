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
*       Contains common declaration of leave service.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_LEAVE_COMMON_H
#define _ZBPRO_NWK_LEAVE_COMMON_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkSapTypesLeave.h"
#include "bbZbProNwkNeighbor.h" // TODO: should be a private
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Initializer for Leave service descriptor.
 */
#define NWK_LEAVE_SERVICE_DESCRIPTOR { \
    .payloadSize    = zbProNwkLeaveMemSize, \
    .fill           = zbProNwkLeaveFill, \
    .conf           = zbProNwkLeaveConf, \
    .ind            = zbProNwkLeaveInd, \
    .reset          = zbProNwkLeaveReset, \
}

/**//**
 * \brief Leave command payload length. ZigBee Spec r20, 3.4.4.
 */
#define ZBPRO_NWK_LEAVE_PAYLOAD_LENGTH  \
    ( /* Command Id */ 1 +              \
      /* Command options */ 1)


/************************* TYPES *******************************************************/
/**//**
 * \brief Leave command options enumeration. (ZigBee spec r20 3.4.4.3.1 p.328)
 */
typedef enum _ZbProNwkLeaveOptions_t
{
    /* The Rejoin sub-field is a single-bit field. If the value of this sub-field is 1, the
    device that is leaving from its current parent will rejoin the network. If the value
    of this sub-field is 0, the device will not rejoin the network. */
    ZBPRO_NWK_LEAVE_REJOIN    = 0x20,

    /* The request sub-field is a single-bit field. If the value of this sub-field is 1, then
    the leave command frame is a request for another device to leave the network. If
    the value of this sub-field is 0, then the leave command frame is an indication that
    the sending device plans to leave the network. */
    ZBPRO_NWK_LEAVE_REQUEST   = 0x40,

    /* The remove children sub-field is a single-bit field. If this sub-field has a value of
    1, then the children of the device that is leaving the network will also be removed.
    If this sub-field has a value of 0, then the children of the device leaving the
    network will not be removed. */
    ZBPRO_NWK_LEAVE_REMOVE_CHILDREN = 0x80
} ZbProNwkLeaveOptions_t;

/**//**
 * \brief Leave command payload type. (ZigBee spec r20 3.4.4 p.326)
 */
typedef struct PACKED _ZbProNwkLeavePayload_t
{
    uint8_t leaveOptions;
} ZbProNwkLeavePayload_t;

/**//**
 * \brief Leave service descriptor.
 */
typedef struct _ZbProNwkLeaveServiceDescr_t
{
    SYS_QueueDescriptor_t   reqQueue;
    SYS_QueueDescriptor_t   indQueue;

    /* Parameters required for Leave request handling. */
    struct {
        MAC_ResetReqDescr_t     macReset;
        ZBPRO_NWK_Status_t      resultTxStatus;
        ZBPRO_NWK_Neighbor_t    *neighbor;
    };

    /* Parameters required for Leave command indication handling. */
    struct {
        ZBPRO_NWK_LeaveReqDescr_t   leaveReq;
        ZBPRO_NWK_ExtAddr_t         srcExtAddr;
        ZBPRO_NWK_NwkAddr_t         srcNwkAddr;
        uint8_t                     leaveOptions;
    };

} ZbProNwkLeaveServiceDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
    \brief Initialize request handler.
****************************************************************************************/
NWK_PRIVATE void zbProNwkLeaveReqReset(void);

/************************************************************************************//**
    \brief Initialize indication handler.
****************************************************************************************/
NWK_PRIVATE void zbProNwkLeaveIndReset(void);

/************************************************************************************//**
    \brief Leave service task handler.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkLeaveTaskHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkLeaveReset;

/************************************************************************************//**
    \brief Returns memory size required for route request header and payload.
    \param[in] outputBuffer - buffer pointer.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkLeaveMemSize;

/************************************************************************************//**
    \brief Puts data from the pending request to the output buffer. This function invoked
        by the network dispatcher when buffer has been allocated.
    \param[in] outputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkLeaveFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkLeaveConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkLeaveInd;

/************************************************************************************//**
    \brief Raises a Leave indication to the upper layer.
    \param[in] deviceExtAddress - extended address of device which left the network.
    \param[in] deviceNwkAddress - network address of device which left the network.
    \param[in] rejoin - value of rejoin subfield from received Leave frame.
****************************************************************************************/
NWK_PRIVATE void zbProNwkLeaveIndRaiseIndication(const ZBPRO_NWK_ExtAddr_t deviceExtAddress,
                                                 const ZBPRO_NWK_NwkAddr_t deviceNwkAddress,
                                                 const bool rejoin);

#endif /* _ZBPRO_NWK_LEAVE_COMMON_H */

/* eof bbZbProNwkLeave.h */
