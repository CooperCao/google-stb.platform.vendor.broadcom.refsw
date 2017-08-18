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

/******************************************************************************
*
* DESCRIPTION:
*       Network Layer Management Entity Route Discovery primitive declarations
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H
#define _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkSapTypesNetworkStatus.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request parameters structure, see ZigBee Spec r20, 3.2.2.31.
 * \ingroup ZBPRO_NWK_RouteDiscoveryReq
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;              /*!< The kind of destination address */
    ZBPRO_NWK_NwkAddr_t     dstAddr;                  /*!< The destination of the route discovery */
    uint8_t                 radius;                   /*!< Number of hops that the route request will travel
                                                           through the network */
    Bool8_t                 noRouteCache;             /*!< TRUE = no route record table should be established */
    Bool8_t                 canBeDelayed;             /*!< If TRUE, then NWK will wait for empty discovery entry */
} ZBPRO_NWK_RouteDiscoveryReqParams_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.confirm parameters structure, see ZigBee Spec r20, 3.2.2.32
 * \ingroup ZBPRO_NWK_RouteDiscoveryConf
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryConfParams_t
{
    ZBPRO_NWK_Status_t              status;           /*!< The status of an attempt to initiate route discovery */
    ZBPRO_NWK_NetworkStatusCode_t   networkStatus;    /*!< In the case where the Status parameter has a value of
                                                           ROUTE_ERROR, this code gives further information about
                                                           the kind of error that occurred. Otherwise, it should be
                                                           ignored */
} ZBPRO_NWK_RouteDiscoveryConfParams_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_RouteDiscoveryReq
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqDescr_t  ZBPRO_NWK_RouteDiscoveryReqDescr_t;

/**//**
 * \brief NLME-ROUTE-DISCOVERY.confirm primitive callback function type
 * \ingroup ZBPRO_NWK_RouteDiscoveryConf
 */
typedef void (*ZBPRO_NWK_RouteDiscoveryConfCallback_t)(ZBPRO_NWK_RouteDiscoveryReqDescr_t   *reqDescr,
                                                       const ZBPRO_NWK_RouteDiscoveryConfParams_t *conf);

/**//**
 * \brief NLME-ROUTE-DISCOVERY.request descriptor data type.
 * \ingroup ZBPRO_NWK_RouteDiscoveryReq
 */
typedef struct _ZBPRO_NWK_RouteDiscoveryReqDescr_t
{
    ZbProNwkServiceField_t                  service;  /*!< Service field */
    ZBPRO_NWK_RouteDiscoveryReqParams_t     params;   /*!< Request parameters */
    ZBPRO_NWK_RouteDiscoveryConfCallback_t  callback; /*!< Confirmation callback */
} ZBPRO_NWK_RouteDiscoveryReqDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief NLME-ROUTE-DISCOVERY.request primitive function.
    \ingroup ZBPRO_NWK_Functions

    \param[in] reqDescr - pointer to the request descriptor.
    \return Nothing.
 ***************************************************************************************/
void ZBPRO_NWK_RouteDiscoveryReq(ZBPRO_NWK_RouteDiscoveryReqDescr_t *reqDescr);

#endif /* _ZBPRO_NWK_SAP_TYPES_ROUTE_DISCOVERY_H */

/* eof bbZbProNwkSapTypesRouteDiscovery.h */