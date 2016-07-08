/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkDispatcher.h $
*
* DESCRIPTION:
*   Contains interface for network dispatcher module.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_DISPATCHER_H
#define _ZBPRO_NWK_DISPATCHER_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkServices.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Dispatcher descriptor.
 */
typedef struct _ZbProNwkDispatcherDescr_t
{
    BITMAP_DECLARE(requestedCommands, ZBPRO_NWK_MAX_SID);
    SYS_QueueDescriptor_t   transitQueue;
} ZbProNwkDispatcherDescr_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
    \brief Initialize procedure of the sending command.
    \param[in] sId - service type
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherReq(const ZbProNwkServiceId_t sId);

/************************************************************************************//**
    \brief Initiates transmission of transit packet.
    \param[in] inBuf - pointer to the buffer with transit packet.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherSendTransit(ZbProNwkInputBuffer_t *const inBuf);

/************************************************************************************//**
    \brief Tries to allocate network buffer and memory for header and/or payload if required.
    \param[in] sId - service type
    \param[in] memorySize - memory size required to place header and/or payload.
****************************************************************************************/
NWK_PRIVATE ZbProNwkOutputBuffer_t *const zbProNwkDispatcherAllocMem(const ZbProNwkServiceId_t sId,
        uint8_t memorySize);

/************************************************************************************//**
    \brief Calls appropriate service handler to fill the buffer parameters.
    \param[in] buffer - a pointer to the output packet buffer.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherFill(ZbProNwkOutputBuffer_t *const buffer);

/************************************************************************************//**
    \brief Receives confirmation of sending a packet.
    \param[in] buffer - a pointer to the output packet buffer.
    \param[in] status - operation status.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherConf(ZbProNwkOutputBuffer_t *const buffer,
                                        const ZBPRO_NWK_Status_t status);

/************************************************************************************//**
    \brief Attempts to send a request to the TX Fsm.
    \param[in] taskDescriptor - a pointer to the network task descriptor.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Redirects indication to the appropriate service.
    \param[in] buffer - pointer to the incoming packet.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherInd(ZbProNwkInputBuffer_t *buffer);

/************************************************************************************//**
    \brief Sets internal variables to the default state.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDispatcherReset(void);

/************************************************************************************//**
    \brief Tries to allocate network buffer and memory for header and/or payload if required.
****************************************************************************************/
NWK_PRIVATE ZbProNwkInputBuffer_t *const zbProNwkDispatcherAllocInputBuffer(ZbProNwkPacketType_t type);

#endif /* _ZBPRO_NWK_DISPATCHER_H */