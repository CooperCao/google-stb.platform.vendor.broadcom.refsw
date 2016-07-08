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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkTxStatus.h $
*
* DESCRIPTION:
*   Network TX Status definitions and declaration.
*
* $Revision: 10322 $
* $Date: 2016-03-04 12:37:23Z $
*
*****************************************************************************************/


#ifndef _BB_ZB_PRO_NWK_TX_STATUS_H
#define _BB_ZB_PRO_NWK_TX_STATUS_H

/************************* INCLUDES *****************************************************/
#include "bbMacSapForZBPRO.h"
#include "bbZbProNwkCommon.h"
#include "private/bbZbProNwkPassiveAck.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Tx timeout task period. Granularity for all tx delays.
 */
#define ZBPRO_TX_TIMEOUT_TASK_PERIOD_MS     10

/**//**
 * \brief Converts octets duration to tx timeout task ticks.
 */
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_TX_OCTETS_TO_TIMEOUT_TASK_TICKS(octets) CEIL(ZBPRO_NWK_OCTETS_TO_MILLISECONDS(octets), ZBPRO_TX_TIMEOUT_TASK_PERIOD_MS)

/**//**
 * \brief NWK TX action enumeration.
 */
typedef enum _ZbProNwkTxAction_t {
    NWK_TX_NOW,
    NWK_TX_INDIRECT,
    NWK_DELAY_IS_REQUIRED,
    NWK_TX_DONE,
} ZbProNwkTxAction_t;

/**//**
 * \brief NWK TX Type enumeration.
 */
typedef enum _ZbProNwkTxType_t {
    NWK_UNICAST_TX_TYPE,
    NWK_MULTICAST_TX_TYPE,
    NWK_BROADCAST_TX_TYPE,
    NWK_ALL_DEVICES_BROADCAST_TX_TYPE,
    NWK_INDIRECT_BROADCAST_TX_TYPE,
    NWK_LINK_STATUS_TX_TYPE,
    NWK_INITIAL_RREQ_TX_TYPE,
    NWK_RREQ_TX_TYPE,
    NWK_FINISHED_TX_TYPE
} ZbProNwkTxType_t;

/**//**
 * \brief NWK TX status structure.
 */
typedef struct _ZbProNwkTxStatus_t
{
    ZbProNwkTxType_t            type;
    uint8_t                     attemptsToTx;
    uint16_t                    delay;
    MAC_Status_t                macStatus;
    ZbProNwkPassiveAckEntry_t   passiveAck;
} ZbProNwkTxStatus_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/************************************************************************************//**
    \brief Initializes the TX Status structure with default values.
    \param[in]  txStatus - pointer to TX Status structure.
    \param[in]  sId - network service identifier.
    \param[in]  parsedHeader - pointer to the parsed header structure.
    \param[in]  prevHopAddr - address of the previous hop if packet is transit.
 ***************************************************************************************/
void zbProNwkInitTxStatus(ZbProNwkTxStatus_t *const txStatus,
                          const ZbProNwkServiceId_t sId,
                          const ZbProNwkParsedHeader_t *const parsedHeader,
                          const ZBPRO_NWK_NwkAddr_t prevHopAddr);

/************************************************************************************//**
    \brief Updates the TX Status structure on the base of previous transmission attempt
           result.
    \param[in]  txStatus - pointer to TX Status structure.
    \param[in]  macStatus - the result of previous transmission attempt.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateTxStatus(ZbProNwkTxStatus_t *const txStatus,
                                        const MAC_Status_t macStatus);

/************************************************************************************//**
    \brief Generates NWK TX Action by the TX Status structure.
    \param[in]  txStatus - pointer to TX Status structure.
    \return NWK TX Action identifier.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkTxAction_t zbProNwkCheckTxStatus(
                                        ZbProNwkTxStatus_t const *const txStatus);

/************************************************************************************//**
    \brief Receives confirm from Passive Ack component which means that Passive
           Acknowledgment has been received.
    \param[in]  entry - pointer to the Passive Ack structure which is a member
                of output buffer structure.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkPassiveAckConf(ZbProNwkPassiveAckEntry_t *const entry);

/************************************************************************************//**
    \brief Frees any memory occupied by the txStatus structure.
    \param[in]  txStatus - pointer to TX Status structure.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkFreeTxStatus(ZbProNwkTxStatus_t *const txStatus);

/************************* INLINE FUNCTIONS ********************************************/
/************************************************************************************//**
    \brief Performs a decrement of delay counter if its value greater than zero.
    \param[in]  txStatus - pointer to TX Status structure.
 ***************************************************************************************/
INLINE ZbProNwkTxAction_t zbProNwkTxDelayTick(ZbProNwkTxStatus_t *const txStatus)
{
    SYS_DbgAssert(NULL != txStatus, ZBPRONWKTXSTATUS_TXDELAYTICK_DA0);

    if (txStatus->delay)
        txStatus->delay--;

    return (txStatus->delay) ? NWK_DELAY_IS_REQUIRED : zbProNwkCheckTxStatus(txStatus);
}

/************************************************************************************//**
    \brief Returns result status.
 ***************************************************************************************/
INLINE ZBPRO_NWK_Status_t zbProNwkGetResultTxStatus(ZbProNwkTxStatus_t *const txStatus)
{
    SYS_DbgAssert(NULL != txStatus, ZBPRONWKTXSTATUS_GETRESULTTXSTATUS_DA0);

    return (ZBPRO_NWK_Status_t)txStatus->macStatus;
}

/************************************************************************************//**
    \brief Marks the TX Status structure as finished.
    \param[in]  txStatus - pointer to TX Status structure.
 ***************************************************************************************/
INLINE void zbProNwkFinishTx(ZbProNwkTxStatus_t *const txStatus)
{
    txStatus->type = NWK_FINISHED_TX_TYPE;
}

INLINE void zbProNwkIterateTxStatus(ZbProNwkTxStatus_t *const txStatus)
{
    SYS_DbgAssert(0 != txStatus->attemptsToTx, BBZBPRONWKTXSTATUS_UPDATEBROADCASTTXSTATUS_DA0);
    txStatus->attemptsToTx--;
    txStatus->macStatus = (MAC_Status_t)ZBPRO_NWK_FORBIDDEN_STATUS_VALUE;
}

#endif /* _BB_ZB_PRO_NWK_TX_STATUS_H */
