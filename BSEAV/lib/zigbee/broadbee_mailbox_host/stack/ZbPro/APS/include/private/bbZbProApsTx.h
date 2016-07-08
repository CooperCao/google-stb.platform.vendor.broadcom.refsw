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
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsTx.h $
 *
 * DESCRIPTION:
 *   Declaration of the ZigBee PRO APS Tx component.
 *
 * $Revision: 9364 $
 * $Date: 2016-01-04 18:21:51Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_TX_H
#define _ZBPRO_APS_TX_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"
#include "bbSysTimeoutTask.h"

#include "bbZbProSsp.h"

#include "bbZbProNwkSapTypesData.h"

#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"

#include "bbZbProZdoSapTypesDiscoveryManager.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Descriptor of Transmitter variables
 */
typedef struct _ZbProApsTxDesc_t
{
    ZbProApsCounter_t       apsCounter;
    SYS_QueueDescriptor_t   waitAckQueue;
    SYS_QueueDescriptor_t   proceedQueue;
    SYS_QueueDescriptor_t   zdoConfirmQueue;
    SYS_TimeoutSignal_t     timeoutTask;
} ZbProApsTxDesc_t;

/**//**
 * \brief Tx Tx buffer type definition
 */
typedef struct _ZbProApsTxBuffer_t
{
    ZbProSspFrameBuffer_t frameBuffer;

    /* internal data */
    struct
    {
        SYS_QueueElement_t          qElem;
        void                        *req;   /* initiator request */
        ZBPRO_APS_Timestamp_t       txTime;
        ZbProApsCommandId_t         id;
        SYS_FSM_StateId_t           txFsmState;
        ZBPRO_APS_Status_t          status;
        uint8_t                     attemptsRemain;
        ZbProApsCounter_t           apsCounterCopy; /* while NWK payload is encrypted */

        union
        {
            ZBPRO_ZDO_AddrResolvingReqDescr_t   addrResolveReq;
            ZbProSspEncryptReq_t                encReq;
            ZbProSspDecryptReq_t                decReq;
            ZBPRO_NWK_DataReqDescr_t            nwkReq;
            SYS_TimeoutSignal_t                 sendSecondCopyDelayDescr;   /* Performs delay prior to send the second
                                                            (nonencrypted) copy of an APS command (APS:UpdateDevice). */
        };

        ZbProSspKey_t                   actualKey;  /* effective key due to the Transport Key command */
    };

    /* requester's part */
    struct
    {
        ZBPRO_APS_ExtAddr_t         extDstAddr;
        ZBPRO_APS_ShortAddr_t       nwkDstAddr;
        ZBPRO_APS_NwkRadius_t       nwkRadius;
        ZbProSspKeyId_t             keyId;

        struct
        {
            BitField8_t             dstNwkAddrSet   : 1;
            BitField8_t             security        : 1;
            BitField8_t             extNonce        : 1;
            BitField8_t             nwkSecurity     : 1; /* enables NWK security */
            BitField8_t             ackRequest      : 1;
            BitField8_t             keepApsCounter  : 1; /* to keep APS counter for ACK */
            BitField8_t             tunnel          : 1; /* to embed the command into a Tunnel command */
        };
    };
} ZbProApsTxBuffer_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/**//**
 * \brief Requests transmitting using Tx FSM service
 */
APS_PRIVATE void zbProApsTxReq(ZbProApsTxBuffer_t *buf);

/**//**
 * \brief Matches the specified ACK to an appropriate Tx buffer
 */
APS_PRIVATE void zbProApsTxGotAck(ZbProApsCounter_t apsCounter, bool isCommand,
        ZBPRO_APS_ClusterId_t clusterId, ZBPRO_APS_ProfileId_t profileId,
        ZBPRO_APS_EndpointId_t dstEndpoint, ZBPRO_APS_EndpointId_t srcEndpoint);

/**//**
 * \brief FSM handlers
 */
APS_PRIVATE void zbProApsTxFsmZdoHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
APS_PRIVATE void zbProApsTxFsmProceedHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Resets the Tx service
 */
APS_PRIVATE void zbProApsTxReset(void);

#endif /* _ZBPRO_APS_TX_H */
