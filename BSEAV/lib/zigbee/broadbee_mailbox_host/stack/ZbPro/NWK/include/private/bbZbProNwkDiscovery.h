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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkDiscovery.h $
*
* DESCRIPTION:
*   Network Discovery service private declarations
*
* $Revision: 2564 $
* $Date: 2014-05-30 10:57:34Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_DISCOVERY_H
#define _ZBPRO_NWK_DISCOVERY_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"
#include "bbZbProNwkSapTypesDiscovery.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Beacon filter function type
 */
typedef bool (*ZbProNwkDiscoveryExtBeaconFilter_t)(const ZbProNwkParsedBeaconNotify_t *const beaconNotify);

/**//**
 * \brief Discovery requests service descriptor.
 */
typedef struct _ZbProNwkDiscoveryServiceDescr_t
{
    SYS_FSM_Descriptor_t    fsm;
    SYS_QueueDescriptor_t   queue;
    uint8_t                 networkCnt;
    SYS_DataPointer_t       networkList;
    MAC_ScanReqDescr_t      scanReq;
} ZbProNwkDiscoveryServiceDescr_t;

/**//**
 * \brief Internal descriptor type.
 */
typedef struct _ZbProNwkDiscoveryInternalReqDescr_t
{
    ZbProNwkDiscoveryServiceField_t         service;
    ZbProNwkDiscoveryReqParams_t            params;
    union
    {
        ZBPRO_NWK_NetworkDiscoveryCallback_t callbackDiscovery;
        ZBPRO_NWK_EDScanConfCallback_t       callbackEdScan;
    };
} ZbProNwkDiscoveryInternalReqDescr_t;
/************************* PROTOTYPES **************************************************/
/*************************************************************************************//**
  \brief Resets discovery service.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkDiscoveryReset(void);

/*************************************************************************************//**
  \brief Discovery service task handler.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkDiscoveryHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*************************************************************************************//**
  \brief Helper function. Start discovery procedure with custom beacon filter.
  \param[in] reqDescr - pointer to discovery request descriptor.
  \param[in] beaconFilter - filter function.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkDiscoveryReq(ZbProNwkDiscoveryInternalReqDescr_t *const reqDescr,
                                      ZbProNwkDiscoveryExtBeaconFilter_t beaconFilter);

/*************************************************************************************//**
  \brief Helper function. Default beacon filter function.
  \param[in] eventId - event number.
  \param[in] data - pointer to the data to raise.
*****************************************************************************************/
bool zbProNwkDiscoveryDefaultBeaconFilter(const ZbProNwkParsedBeaconNotify_t *const beaconNotify);

#endif /* _ZBPRO_NWK_DISCOVERY_H */