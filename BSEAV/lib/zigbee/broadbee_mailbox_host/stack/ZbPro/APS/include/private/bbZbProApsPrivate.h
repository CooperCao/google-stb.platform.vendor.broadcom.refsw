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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsPrivate.h $
 *
 * DESCRIPTION:
 *   This is the general header file for the ZigBee PRO APS component.
 *
 * $Revision: 10430 $
 * $Date: 2016-03-14 17:29:10Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_PRIVATE_APS_H
#define _ZBPRO_PRIVATE_APS_H

/************************* INCLUDES ****************************************************/
#include "bbSysTaskScheduler.h"
#include "bbZbProApsKeywords.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Value which is used to transform a key into a Key-Transport Key
 */
#define ZBPRO_APS_KEYTRANSPORT_KEY_HASH_VALUE       0x00u

/**//**
 * \brief Value which is used to transform a key into a Key-Load Key
 */
#define ZBPRO_APS_KEYLOAD_KEY_HASH_VALUE            0x02u

/************************* TYPES *******************************************************/

/**//**
 * \brief Identifiers of the APS layer handlers.
 */
typedef enum _ZBPRO_APS_HandlerId_t
{
    ZBPRO_APS_RX_ACK_HANDLER_ID,
    ZBPRO_APS_RX_FSM_HANDLER_ID,
    ZBPRO_APS_TX_FSM_PROCEED_HANDLER_ID,
    ZBPRO_APS_TX_FSM_ZDO_HANDLER_ID,
    ZBPRO_APS_HUB_HANDLER_ID,

    ZBPRO_APS_DATA_REQUEST_HANDLER_ID,

    ZBPRO_APS_BINDING_SERVICE_HANDLER_ID,

    ZBPRO_APS_GROUP_MANAGER_HANDLER_ID,

    ZBPRO_APS_GET_SET_HANDLER_ID,

    ZBPRO_APS_ENDPOINT_HANDLER_ID,

    ZBPRO_APS_STARTSTOP_HANDLER_ID,

    ZBPRO_APS_STARTSTOP_DELAYED_STOP_ID,

    ZBPRO_APS_HANDLERS_AMOUNT,

} ZBPRO_APS_HandlerId_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/**//**
 * \brief Posts handling task within ZB PRO APS layer task.
 *
 * \param[in] handlerId - identifier of the APS layer handler to be called.
 */
APS_PRIVATE void zbProApsPostTask(ZBPRO_APS_HandlerId_t handlerId);

/**//**
 * \brief Recalls the specified handling task within ZB PRO APS layer task.
 *
 * \param[in] handlerId - identifier of the APS layer handler to recall.
 ****************************************************************************************/
APS_PRIVATE void zbProApsRecallTask(ZBPRO_APS_HandlerId_t handlerId);

#endif /* _ZBPRO_PRIVATE_APS_H */
