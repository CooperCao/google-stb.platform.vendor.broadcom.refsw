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
 *      This is the header file for the RF4CE Network Layer component indications routines.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_INDICATIONS_H
#define _RF4CE_NWK_INDICATIONS_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysTaskScheduler.h"
#include "bbMacSapForRF4CE.h"

#include "bbRF4CENWKNIBAttributes.h"


/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK incoming data indications queue.
 */
typedef struct _RF4CE_NWK_IncomingFrameBuffer_t
{
    SYS_QueueElement_t queueElement;                    /*!< Internal queue field */

    RF4CE_PairingTableEntry_t *pairEntry;               /*!< Pointer to an appropriate pair entry */
    SYS_DataPointer_t   body;

    MAC_Address_t       srcAddr;                        /*!< Source address */
    MAC_PanId_t         srcPanId;                       /*!< Source PAN ID */
    MAC_AddrMode_t      srcAddrMode;                    /*!< Source address mode */
    Bool8_t             isBroadcast;

    uint32_t            frameCounter;                   /*!< Frame counter from frame header. */
    uint16_t            vendorId;                       /*!< Vendor id from frame header. */
    uint8_t             profileId;                      /*!< Profile id from frame header. */
    uint8_t             frameControl;                   /*!< Frame control. */
    uint8_t             lqi;                            /*!< frame link quality */
    uint8_t             channelDesignatorValue;
} RF4CE_NWK_IncomingFrameBuffer_t;

typedef struct _RF4CE_RxServiceMem_t
{
    SYS_QueueDescriptor_t   freeIndication;
    SYS_QueueDescriptor_t   usedIndication;
    RF4CE_NWK_IncomingFrameBuffer_t frameStorage[RF4CE_NWK_MAX_INCOMING_PACKETS];
    MAC_SetReqDescr_t       macSetReq;
} RF4CE_RxServiceMem_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Reset rf4ce network service to default state.
 ****************************************************************************************/
void rf4ceNwkRxServiceReset(void);

/************************************************************************************//**
 \brief Task handlers for specific commands.
 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UpdateKeyRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_DataRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_StartResetRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_DataRequestEncryptHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_OnMacDataIndication(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_DiscoveryRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_OnDiscoveryDurationTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
#ifdef RF4CE_TARGET
void RF4CE_NWK_AutoDiscoveryRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_AutoDiscoveryRequestHandler2(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void RF4CE_NWK_OnAutoDiscoveryTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
#else
# define RF4CE_NWK_AutoDiscoveryRequestHandler(...)
# define RF4CE_NWK_AutoDiscoveryRequestHandler2(...)
# define RF4CE_NWK_OnAutoDiscoveryTimeout(...)
#endif /* RF4CE_TARGET */

/************************************************************************************//**
 \brief Group of internal command handlers.
 \param[in] packetBuffer - pointer to the incoming packet.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_OnDataIndication(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnDiscoveryResponse(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnPairResponse(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnUnpairRequest(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnKeySeed(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnPingResponse(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
#ifdef RF4CE_TARGET
void RF4CE_NWK_OnDiscoveryRequest(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnPairRequest(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
void RF4CE_NWK_OnPingRequest(RF4CE_NWK_IncomingFrameBuffer_t *const packetBuffer);
#else
# define RF4CE_NWK_OnDiscoveryRequest(...)
# define RF4CE_NWK_OnPairRequest(...)
# define RF4CE_NWK_OnPingRequest(...)
#endif /* RF4CE_TARGET */

#endif /* _RF4CE_NWK_INDICATIONS_H */

/* eof bbRF4CENWKIndications.h */
