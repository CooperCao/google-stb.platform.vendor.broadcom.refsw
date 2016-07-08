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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkAddrConflict.h $
*
* DESCRIPTION:
*   Contains declarations for the Network Layer address conflict resolution.
*
* $Revision: 2797 $
* $Date: 2014-07-03 10:59:52Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_ADDR_CONFLICT_H
#define _ZBPRO_NWK_ADDR_CONFLICT_H

/************************* INCLUDES ****************************************************/
#include "bbMacSapForZBPRO.h"

#include "bbZbProNwkSapTypesGetSet.h"
#include "private/bbZbProNwkStatus.h"
#include "private/bbZbProNwkRejoinResp.h"

/************************* DEFINITIONS *************************************************/

/************************* TYPES *******************************************************/
/**//**
 * \brief Network address conflict status states enumeration.
 */
typedef enum _ZbProNwkAddrConflictStatus_t
{
    ZBPRO_NWK_ADDRESS_CONFLICT_IDLE_STATE,
    ZBPRO_NWK_ADDRESS_CONFLICT_READY_TO_START_STATE,
    ZBPRO_NWK_ADDRESS_CONFLICT_IN_PROGRESS_STATE,
} ZbProNwkAddrConflictStatus_t;


/**//**
 * \brief Network address conflict service descriptor.
 */
typedef struct _ZbProNwkAddrConflictServiceDescr_t
{
    ZBPRO_NWK_NwkAddr_t conflictedAddr;
    ZbProNwkAddrConflictStatus_t status;
    Bool8_t isQuiet;
    union
    {
        ZbProNwkStatusReqDescr_t            nwkStatus;
        ZBPRO_NWK_SetReqDescr_t             nwkSetReq;
        ZbProNwkRejoinResponseDescriptor_t  rejoinResp;
    } req;

} ZbProNwkAddrConflictServiceDescr_t;

/*********************** FUNCTION PROTOTYPES *******************************************/
/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE void zbProNwkAddrConflictReset(void);

/************************************************************************************//**
    \brief Checks address pair on conflict.
    \param[in] nwkAddr - short address.
    \param[in] extAddr - extended address.
    \return true, if an address conflict is NOT detected.
****************************************************************************************/
NWK_PRIVATE bool zbProNwkAddrConflictVerifyAddrPair(const ZBPRO_NWK_NwkAddr_t nwkAddr, const ZBPRO_NWK_ExtAddr_t *const extAddr);

/**************************************************************************//**
  \brief Raises address conflict.
  \param[in] conflictedAddr - source address of conflict.
  \param[in] keepSilence - if 'true' then nwkStatus will be sent after the resolving procedure.
 ******************************************************************************/
NWK_PRIVATE void zbProNwkAddrConflictRaise(const ZBPRO_NWK_NwkAddr_t conflictedAddr, const Bool8_t keepSilence);

/**************************************************************************//**
  \brief Starts address resolving procedure.
  \param[in] taskDescriptor - network task descriptor manager.
 ******************************************************************************/
NWK_PRIVATE void zbProNwkAddrConflictHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**************************************************************************//**
  \brief Resolves address conflict.
  \param[in] nwkAddr - the duplicated network address.
 ******************************************************************************/
NWK_PRIVATE void zbProNwkAddressConflictResolve(void);


#endif /* _ZBPRO_NWK_ADDR_CONFLICT_H */