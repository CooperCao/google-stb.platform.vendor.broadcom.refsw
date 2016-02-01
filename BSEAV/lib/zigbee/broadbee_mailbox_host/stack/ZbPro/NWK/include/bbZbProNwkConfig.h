/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkConfig.h $
*
* DESCRIPTION:
*   Contains ZigBee PRO NWK layer configuration.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_CONFIG_H
#define _ZBPRO_NWK_CONFIG_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief The total amount of network buffers.
 */
#define ZBPRO_NWK_MAX_PACKET_AMOUNT             20U

/**//**
 * \brief Number of neighbor table entries supported by the network layer.
 *        Minimum for ZigBee PRO Coordinator: Max number of child end devices plus 16
 *        Minimum for ZigBee PRO Router: Max number of child end devices plus 16
 */
#define ZBPRO_NWK_NEIGHBOR_TABLE_SIZE           18U

/**//**
 * \brief Maximum amount of stored address pairs.
 */
#define ZBPRO_NWK_ADDRESS_MAP_ENTRY_AMOUNT      10U

/**//**
 * \brief The maximum amount of join requests in one time.
 */
#define ZBPRO_NWK_JOIN_BUFFERS_AMOUNT           5U

/**//**
 * \brief The number of entries in routing table. Minimum for ZigBee PRO: 10.
 */
#define ZBPRO_NWK_ROUTING_TABLE_SIZE            10U

/**//**
 * \brief The number of entries in source routing table. Optional for ZigBee PRO.
 */
#define ZBPRO_NWK_SRC_ROUTING_TABLE_SIZE        5U
#define ZBPRO_NWK_SRC_ROUTING_TABLE_SIZE_IS_ENOUGH_FOR_CACHE true

/**//**
 * \brief The number of entries in route discovery table. Minimum for ZigBee PRO: 4.
 */
#define ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_SIZE    4U

/**//**
 * \brief The number of entries in broadcast transaction table.
 */
#define ZBPRO_NWK_BTT_SIZE                      32U

/**//**
 * \brief Duration of searching other networks on same channel.
 */
#define ZBPRO_NWK_SEARCH_NETWORK_DURATION       8U /* ~4sec - 2.4MHz */

/**//**
 * \brief The number of security keys supported.
 */
#define ZBPRO_NWK_SECURITY_KEYS_AMOUNT          2U


/**//**
 * \brief Enhanced route discovery procedure, if flag was defined the route
 *      discovery procedure shall call a callback function after the first successful route reply.
 */
#ifdef ZBPRO_NWK_ROUTE_DISCOVERY_DISABLE_FAST_RESULT
# undef ZBPRO_NWK_ROUTE_DISCOVERY_DISABLE_FAST_RESULT
# define ZBPRO_NWK_ROUTE_DISCOVERY_DISABLE_FAST_RESULT 1
#else
# define ZBPRO_NWK_ROUTE_DISCOVERY_DISABLE_FAST_RESULT 0
#endif

#endif /* _ZBPRO_NWK_CONFIG_H */