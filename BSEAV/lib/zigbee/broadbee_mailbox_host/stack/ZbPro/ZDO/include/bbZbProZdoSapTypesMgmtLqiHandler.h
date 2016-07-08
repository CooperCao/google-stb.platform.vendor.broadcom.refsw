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
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoSapTypesMgmtLqiHandler.h $
 *
 * DESCRIPTION:
 *   This header describes types and API for the ZDO ZDP service Mgmt_Lqi
 *
 * $Revision: 10263 $
 * $Date: 2016-02-29 18:03:06Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_ZDO_SAP_TYPES_GET_LQI_HANDLER_H
#define _ZBPRO_ZDO_SAP_TYPES_GET_LQI_HANDLER_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"
#include "bbZbProNwkNeighbor.h"

/************************* TYPES ********************************************************/

/*************************************************************************************//**
  \brief   Structure for parameters of ZDO Local Request to issue ZDP Mgmt_Lqi_req
  command. This structure takes its origin from ZDP Mgmt_Lql_req command
  \par  Documentation
  See ZigBee Document 053474r20, subclause 2.4.3.3.2, figure 2.54, table 2.81.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtLqiReqParams_t
{
    ZBPRO_ZDO_Address_t                 zdpDstAddress;    /*!< Destination address. It shall be unicast only.*/

    BitField8_t                         startIndex;       /*!< Starting Index for the requested elements of
                                                               the Neighbor Table. */
} ZBPRO_ZDO_MgmtLqiReqParams_t;


/*************************************************************************************//**
  \brief   Structure for parameters of ZDO Local Confirmation on ZDP Mgmt_Lqi_req
  command. This structure takes its origin from ZDP Mgmt_Lql_rsp command
  \par  Documentation
  See ZigBee Document 053474r20, subclause 2.4.4.3.2, figure 2.95, table 2.126.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtLqiConfParams_t
{
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t  status;                   /*!< The status of the Mgmt_Lqi_req command. */

    BitField8_t         neighborTableEntries;     /*!< Total number of Neighbor Table entries
                                                       within the Remote Device. */

    BitField8_t         startIndex;               /*!< Starting index within the Neighbor Table
                                                       to begin reporting for the NeighborTableList. */

    BitField8_t         neighborTableListCount;   /*!< Number of Neighbor Table entries included
                                                       within NeighborTableList. */

    SYS_DataPointer_t   payload;                  /*!< A list of descriptors, beginning with the
                                                       StartIndex element and continuing for
                                                       NeighborTableListCount, of the elements in the
                                                       Remote Device's Neighbor Table including the device
                                                       address and associated LQI (see Table 2.127 for
                                                       details from ZigBee Document 053474r20). */
} ZBPRO_ZDO_MgmtLqiConfParams_t;


/*************************************************************************************//**
  \brief   Structure for element of NeighborTableList Record.
  \par  Documentation
  See ZigBee Document 053474r20, subclause 2.4.4.3.2, figure 2.95, table 2.127.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtLqiConfNeighborTableRecord_t
{
    ZBPRO_ZDO_ExtPanId_t       extendedPANId;    /*!< The 64-bit extended PAN identifier
                                                      of the neighboring device. */

    ZBPRO_ZDO_ExtAddr_t        extendedAddress;  /*!< 64-bit IEEE address that is unique to
                                                      every device. If this value is
                                                      unknown at the time of the request,
                                                      this field shall be set to
                                                      0xffffffffffffffff. */

    ZBPRO_ZDO_NwkAddr_t        networkAddress;   /*!< The 16-bit network address of the
                                                      neighboring device. */

    ZBPRO_NWK_DeviceType_t     deviceType;       /*!< The type of neighbor device. */

    ZBPRO_NWK_Relationship_t   relationship;     /*!< Relationship with neighbor device. */

    Bool8_t                    rxOnWhenIdle;     /*!< Indicates if neighbor's receiver
                                                      is enabled during idle periods. */

    Bool8_t                    permitJoining;    /*!< An indication of whether the device
                                                      is accepting joining requests. */

    ZBPRO_NWK_Depth_t          depth;            /*!< The tree depth of the neighbor
                                                      device. A value of 0x00 indicates
                                                      that the device is the ZigBee
                                                      coordinator for the network. */

    PHY_LQI_t                  LQI;              /*!< The estimated link quality for RF
                                                      transmissions from this device. */
} ZBPRO_ZDO_MgmtLqiConfNeighborTableRecord_t;


/*************************************************************************************//**
  \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Lqi_req
  command.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtLqiReqDescr_t  ZBPRO_ZDO_MgmtLqiReqDescr_t;


/*************************************************************************************//**
  \brief  Data type for ZDO Local Confirmation callback function of ZDP Mgmt_Lqi_req
  command.
  \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
  \param[in]   confParams      Pointer to the confirmation parameters structure.
*****************************************************************************************/
typedef void ZBPRO_ZDO_MgmtLqiConfCallback_t(ZBPRO_ZDO_MgmtLqiReqDescr_t   *const reqDescr,
                                             ZBPRO_ZDO_MgmtLqiConfParams_t *const confParams);


/*************************************************************************************//**
  \brief  Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Lqi_req
  command.
*****************************************************************************************/
struct _ZBPRO_ZDO_MgmtLqiReqDescr_t
{
    ZBPRO_ZDO_MgmtLqiConfCallback_t   *callback;        /*!< ZDO Confirmation callback handler entry point. */

    ZbProZdoLocalRequest_t             service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtLqiReqParams_t       params;          /*!< ZDO Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/


/*************************************************************************************//**
  \brief  Accepts ZDO Local Request to issue ZDP Mgmt_Lqi_req command.

  \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
*****************************************************************************************/
void ZBPRO_ZDO_MgmtLqiReq(ZBPRO_ZDO_MgmtLqiReqDescr_t *const  reqDescr);


#endif
