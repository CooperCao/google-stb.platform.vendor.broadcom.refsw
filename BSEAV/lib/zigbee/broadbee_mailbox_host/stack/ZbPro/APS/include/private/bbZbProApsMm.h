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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsMm.h $
 *
 * DESCRIPTION:
 *   Contains implementation of the interface for ZigBee PRO APS memory objects.
 *
 * $Revision: 2927 $
 * $Date: 2014-07-15 16:04:50Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_APS_MM_H
#define _ZBPRO_APS_MM_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProApsStartStop.h"
#include "private/bbZbProApsState.h"
#include "private/bbZbProApsPrivate.h"
#include "private/bbZbProApsIb.h"
#include "private/bbZbProApsGetSet.h"
#include "private/bbZbProApsHub.h"
#include "private/bbZbProApsDataTx.h"
#include "private/bbZbProApsBindingTableDescr.h"    /* APS Binding Table descriptors. */
#include "private/bbZbProApsProcBindUnbind.h"       /* APSME-(UN)BIND interface.      */
#include "private/bbZbProApsProcRemoveDevice.h"     /* APSME-REMOVE-DEVICE interface. */
#include "private/bbZbProApsProcRequestKey.h"       /* APSME-REQUEST-KEY interface.   */
#include "private/bbZbProApsProcSwitchKey.h"        /* APSME-SWITCH-KEY interface.    */
#include "private/bbZbProApsProcTransportKey.h"     /* APSME-TRANSPORT-KEY interface. */
#include "private/bbZbProApsProcUpdateDevice.h"     /* APSME-UPDATE-DEVICE interface. */
#include "private/bbZbProApsProcTunnel.h"
#include "private/bbZbProApsGroupTablePrivate.h"
#include "private/bbZbProApsGroupManagerPrivate.h"
#include "private/bbZbProApsDuplicate.h"
#include "private/bbZbProApsKeyPair.h"
#include "private/bbZbProApsEndpointPrivate.h"
/************************* DEFINITIONS *************************************************/

/************************* TYPES *******************************************************/
/**//**
 * \brief Type describes the APS buffer type.
 */
typedef enum _ZbProApsBufferType_t
{
    ZBPRO_APS_TX_BUFFER_TYPE,
    ZBPRO_APS_RX_BUFFER_TYPE,
    ZBPRO_APS_BUFFER_TYPE_LAST
} ZbProApsBufferType_t;

/**//**
 * \brief Type describes the structure of APS layer service buffer.
 */
typedef struct _ZbProApsBuffer_t
{
    union
    {
        ZbProApsTxBuffer_t  txBuffer;
        ZbProApsRxBuffer_t  rxBuffer;
    };
    struct {
        uint8_t             busy : 1;
        uint8_t             bufferType : 1; /* NOTE: Be careful, shall be enough for ZbProApsBufferType_t */
    } service;
} ZbProApsBuffer_t;

/**//**
 * \brief Type describes the structure of APS layer descriptor.
 */
typedef struct _ZbProApsDescr_t
{
    SYS_SchedulerTaskDescriptor_t       taskDescriptor;

    ZbProApsStateDescr_t                state;
    ZbProApsIb_t                        aib;
    ZbProApsGetSetDescr_t               getSetDescr;

    ZbProApsTxDesc_t                    txDesc;
    ZbProApsRxDesc_t                    rxDesc;
    ZbProApsHubDesc_t                   hubDesc;

    ZbProApsDuplicateDescriptor_t       duplicateDescr;
    ZbProApsGroupManagerDescr_t         groupManagerDescr;
    ZbProApsBindUnbindDescr_t           bindingServiceDescr;

    ZbProApsDataDesc_t                  dataDesc;
    ZbProApsRemoveDeviceDescr_t         removeDeviceDescr;
    ZbProApsRequestKeyDescr_t           requestKeyDescr;
    ZbProApsSwitchKeyDescr_t            switchKeyDescr;
    ZbProApsTransportKeyDescr_t         transportKeyDescr;
    ZbProApsUpdateDeviceDescr_t         updateDeviceDescr;
    ZbProApsTunnelDescr_t               tunnelDescr;

    ZbProApsBindingTableDescriptor_t    bindingTableDescr;
    ZbProApsBindingTableLinkToGroup_t   bindingTable[ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_GROUPS];
    ZbProApsGroupTableEntry_t           groupTable[ZBPRO_APS_GROUP_TABLE_SIZE];
    ZbProApsKeyPairDescriptor_t         keyPairDescr;
    ZbProApsEndpointDescriptor_t        endpointDescr;

    ZbProApsStartStopDesc_t             startStopDescr;

    ZbProApsBuffer_t                    bufferPool[ZBPRO_APS_BUFFER_POOL_SIZE];
    ZbProApsEndpointTableSaveLoadDescriptor_t    endpointsSaveLoadDescr;
} ZbProApsDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Returns a pointer to the common APS layer descriptor
 ****************************************************************************************/
ZbProApsDescr_t *ZbProApsMmDescrGet(void);

/************************************************************************************//**
 \brief Returns a pointer to the APS Information base.
 ****************************************************************************************/
INLINE ZbProApsIb_t *zbProApsIb(void)
{
    return &ZbProApsMmDescrGet()->aib;
}

/************************************************************************************//**
 \brief Returns a pointer to the APS Key Pair table descriptor.
 ****************************************************************************************/
INLINE ZbProApsKeyPairDescriptor_t *zbProApsKeyPairDescriptorMem(void)
{
    return &ZbProApsMmDescrGet()->keyPairDescr;
}

/************************************************************************************//**
 \brief Returns a pointer to the APS get/set service memory.
 ****************************************************************************************/
INLINE ZbProApsGetSetDescr_t *zbProApsGetSetDescr(void)
{
    return &ZbProApsMmDescrGet()->getSetDescr;
}

/************************************************************************************//**
 \brief Allocates an APS layer service buffer
 ****************************************************************************************/
ZbProApsBuffer_t *ZbProApsMmBufferGet(ZbProApsBufferType_t type);

/************************************************************************************//**
 \brief Frees an APS layer service buffer
 ****************************************************************************************/
void ZbProApsMmBufferFree(void *buffer);

#endif /* _ZBPRO_APS_MM_H */