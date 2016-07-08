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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/private/bbRF4CENWKIndications.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer component indications routines.
 *
 * $Revision: 3142 $
 * $Date: 2014-08-04 10:47:40Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_INDICATIONS_H
#define _RF4CE_NWK_INDICATIONS_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysTaskScheduler.h"
#include "bbMacSapTypesData.h"

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Called upon unpair request receiption.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnUnpairRequest(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Update Key Request Handler Task.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UpdateKeyRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Pair Request Task Handler. Handles Request queue and dispatches calls.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairUnpairRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Key Seed Wait timeout expires.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnKeySeedWaitTimeoutInitiator(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Response Wait timeout expires.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnResponseWaitTimeoutInitiator(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Key Seed Wait timeout expires.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnKeySeedWaitTimeoutResponder(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Response Wait timeout expires.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnResponseWaitTimeoutResponder(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief DATA Request Task Handler.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DataRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief DATA Indication Task Handler.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DataIndicationHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Start/Reset request Handler Task.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_StartResetRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief DATA encryption Handler Task.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DataRequestEncryptHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Data indication Handler Task.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnDataIndication(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Discovery Request Handler Task. Controller only.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DiscoveryRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Discovery Duration timeout expires. Controller only.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnDiscoveryDurationTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Called upon discovery response receiption. Controller only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnDiscoveryResponse(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Called upon pair response receiption. Controller only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnPairResponse(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Called upon key seed receiption. Controller only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnKeySeed(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Called upon ping response receiption. Controller only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnPingResponse(MAC_DataIndParams_t *indication, uint8_t dataSize);

#ifdef RF4CE_TARGET

/************************************************************************************//**
 \brief Called upon discovery request receiption. Target only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnDiscoveryRequest(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Called upon pair request receiption. Target only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnPairRequest(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Called upon ping request receiption. Target only.

 \param[in] indication - pointer to the incoming data.
 \param[in] dataSize - real data size pointed by indication->msdu.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnPingRequest(MAC_DataIndParams_t *indication, uint8_t dataSize);

/************************************************************************************//**
 \brief Auto-Discovery Request Handler Task. Target only.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_AutoDiscoveryRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Auto-Discovery Request Handler Task. Target only.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_AutoDiscoveryRequestHandler2(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief The callback is called after Auto Discovery timeout expires. Target only.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnAutoDiscoveryTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

#endif /* RF4CE_TARGET */

#endif /* _RF4CE_NWK_INDICATIONS_H */