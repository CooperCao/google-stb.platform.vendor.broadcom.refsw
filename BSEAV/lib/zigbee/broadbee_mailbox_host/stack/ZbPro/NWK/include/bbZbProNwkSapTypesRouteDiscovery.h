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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkSapTypesRouteDiscovery.h $
*
* DESCRIPTION:
*   Network Layer Management Entity Route Discovery primitive declarations
*
* $Revision: 2385 $
* $Date: 2014-05-14 08:41:03Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H
#define _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkSapTypesNetworkStatus.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request parameters structure, see ZigBee Spec r20, 3.2.2.31.
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;
    ZBPRO_NWK_NwkAddr_t     dstAddr;
    uint8_t                 radius;
    Bool8_t                 noRouteCache;
    Bool8_t                 canBeDelayed; // if true then NWK will waiting for empty discovery entry.
} ZBPRO_NWK_RouteDiscoveryReqParams_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.confirm parameters structure, see ZigBee Spec r20, 3.2.2.32
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryConfParams_t
{
    ZBPRO_NWK_Status_t              status;
    ZBPRO_NWK_NetworkStatusCode_t   networkStatus;
} ZBPRO_NWK_RouteDiscoveryConfParams_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request descriptor data type declaration.
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqDescr_t  ZBPRO_NWK_RouteDiscoveryReqDescr_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.confirm primitive callback function type
 */
typedef void (*ZBPRO_NWK_RouteDiscoveryConfCallback_t)(ZBPRO_NWK_RouteDiscoveryReqDescr_t   *reqDescr,
                                                       ZBPRO_NWK_RouteDiscoveryConfParams_t *conf);

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request descriptor data type.
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqDescr_t
{
    ZbProNwkServiceField_t                  service;
    ZBPRO_NWK_RouteDiscoveryReqParams_t     params;
    ZBPRO_NWK_RouteDiscoveryConfCallback_t  callback;
} ZBPRO_NWK_RouteDiscoveryReqDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief NLME-ROUTE-DISCOVERY.request primitive function.
    \param[in] reqDescr - pointer to the request descriptor.
 ***************************************************************************************/
void ZBPRO_NWK_RouteDiscoveryReq(ZBPRO_NWK_RouteDiscoveryReqDescr_t *reqDescr);

#endif /* _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H */