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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/friend/bbZbProApsEndpointFriend.h $
 *
 * DESCRIPTION:
 *   Friend interface of the APS Endpoint component
 *
 * $Revision: 2999 $
 * $Date: 2014-07-21 13:30:43Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_ENDPOINT_FRIEND_H
#define _ZBPRO_APS_ENDPOINT_FRIEND_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsSapEndpoint.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Invalid Endpoint Id
 */
#define ZBPRO_APS_ENDPOINT_ID_INVALID       0xFFu

/**//**
 * \brief Invalid Endpoint Index
 */
#define ZBPRO_APS_ENDPOINT_INDEX_INVALID    0xFFu

/**************************** TYPES ****************************************************/

/**//**
 * \brief Index of the registered endpoint (order of registering)
 */
typedef uint8_t ZbProApsEndpointIdx_t;

/************************** PROTOTYPES *************************************************/

/************************************************************************************//**
 \brief Returns an Endpoint Id by the specified Endpoint index
 ***************************************************************************************/
APS_PRIVATE ZBPRO_APS_EndpointId_t zbProApsEndpointGetIdByIdx(const ZbProApsEndpointIdx_t endpointIdx);

/************************************************************************************//**
 \brief Returns an Endpoint Index by the specified Endpoint Id
 ***************************************************************************************/
APS_PRIVATE ZbProApsEndpointIdx_t zbProApsEndpointGetIdxById(const ZBPRO_APS_EndpointId_t endpointId);

/**//**
 * \brief Gets the simple descriptor of the next registered endpoint
 */
APS_PRIVATE bool zbProApsEndpointGetNextSimpleDesc(ZBPRO_APS_SimpleDescriptor_t **simpleDesc);

/**//**
 * \brief   Returns pointer to Simple Descriptor object by Local Endpoint identifier.
 * \param[in]   endpoint    Numeric value of the Endpoint Identifier.
 * \return  Pointer to Simple Descriptor object belonging to the specified Endpoint, if
 *  it exists; or NULL.
 *  This function searches for active Local Endpoint having the specified Identifier and
 *  returns pointer to its Simple Descriptor object. If there is no active andpoint on the
 *  local node with the specified identifier, NULL is returned. Values 0x00 (ZDO/ZDP
 *  endpoint) and 0xFF (broadcast endpoint) are prohibited for endpoint identifier; for
 *  such case NULL is returned.
 */
APS_PRIVATE ZBPRO_APS_SimpleDescriptor_t* zbProApsEndpointGetSimpleDescriptorByEndpointId(
                const ZBPRO_APS_EndpointId_t endpoint);

#endif /* _ZBPRO_APS_ENDPOINT_FRIEND_H */