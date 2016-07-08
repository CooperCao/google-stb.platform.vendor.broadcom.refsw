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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZDO Layer State declarations
*
* $Revision: 2385 $
* $Date: 2014-05-14 08:41:03Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_STATE_H
#define _BB_ZBPRO_ZDO_STATE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* TYPES *******************************************************/

/**//**
 * \brief Definitions of ZDO function IDs which maps into appropriate global state bits
 *        along with group IDs which consists of several ZDO function IDs.
 */
typedef enum
{
    ZBPRO_ZDO_RESERVED_STATE            = 0,
    ZBPRO_ZDO_START_SERVICE_STATE       = BIT(0),
    ZBPRO_ZDO_LEAVE_SERVICE_STATE       = BIT(1),
    ZBPRO_ZDO_DEVICE_ANNCE_STATE        = BIT(2),
    ZBPRO_ZDO_PERSISTENT_STATE          = BIT(3),


    /* Composite states */
    ZBPRO_ZDO_AWAITNG_RESET_STATE       = ZBPRO_ZDO_RESERVED_STATE,
    ZBPRO_ZDO_INITIALIZED_STATE         = ZBPRO_ZDO_START_SERVICE_STATE,
    ZBPRO_ZDO_AUTHENTIFICATION_STATE    = ZBPRO_ZDO_LEAVE_SERVICE_STATE,
    ZBPRO_ZDO_HAS_PAN_STATE             = ZBPRO_ZDO_LEAVE_SERVICE_STATE | ZBPRO_ZDO_DEVICE_ANNCE_STATE | ZBPRO_ZDO_PERSISTENT_STATE | ZBPRO_ZDO_START_SERVICE_STATE,

} ZbProZdoStateId_t;


/**//**
 * \brief   Data type for ZDO Global State.
 */
typedef ZbProZdoStateId_t  ZbProZdoStateDescr_t;


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Switches ZDO State and starts corresponding ZDO Managers if necessary.
 * \param[in]   newState        New ZDO Sate to be set.
 */
ZDO_PRIVATE void zbProZdoStateSet(const ZbProZdoStateDescr_t  newState);


/**//**
 * \brief   Check if the specified functionality is enabled.
 * \param[in]   state       State to be tested against the actual current ZDO State.
 * \return  TRUE, if all the specified functionality is currently enabled.
 */
ZDO_PRIVATE bool zbProZdoStateIsOn(const ZbProZdoStateDescr_t  state);


#endif /* _BB_ZBPRO_ZDO_STATE_H */
