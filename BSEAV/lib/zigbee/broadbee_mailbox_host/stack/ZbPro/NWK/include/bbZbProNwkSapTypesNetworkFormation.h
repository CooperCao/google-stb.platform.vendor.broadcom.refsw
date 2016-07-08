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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkSapTypesNetworkFormation.h $
*
* DESCRIPTION:
*   Network Layer Management Entity Network Formation primitive declarations
*
* $Revision: 2385 $
* $Date: 2014-05-14 08:41:03Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_SAP_TYPES_NETWORK_FORMATION_H
#define _ZBPRO_NWK_SAP_TYPES_NETWORK_FORMATION_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief NLME-NETWORK-FORMATION request parameters structure. See ZigBee Spec r20, 3.2.2.3.
 * \note Not all parameters of the request primitive described in ZigBee Spec r20, 3.2.2.3
 *  are supported. BeaconOrder, SuperframeOrder and BatteryLifeExtension are NOT SUPPORTED.
 */
typedef struct _ZBPRO_NWK_NetworkFormationReqParams_t
{
    /*NOTE: Description for apsUseExtendedPANID say "The 64-bit address of a network to form
    or to join" to support opportunity we shall to extend the NWk Formation primitive by extendedPanId field.
    See ZibBee Spec r20. 2.2.7.2 */
    ZBPRO_NWK_ExtPanId_t    extendedPANId;
    uint32_t                scanChannels;
    uint8_t                 scanDuration;
} ZBPRO_NWK_NetworkFormationReqParams_t;

/**//**
 * \brief NLME-NETWORK-FORMATION confirm parameters structure. See ZigBee Spec r20, 3.2.2.4.
 */
typedef struct _ZBPRO_NWK_NetworkFormationConfParams_t
{
    ZBPRO_NWK_Status_t status;
} ZBPRO_NWK_NetworkFormationConfParams_t;

/**//**
 * \brief NLME-NETWORK-FORMATION.request descriptor data type declaration.
 */
typedef struct _ZBPRO_NWK_NetworkFormationReqDescr_t ZBPRO_NWK_NetworkFormationReqDescr_t;

/**//**
 * \brief NLME-NETWORK-FORMATION confirm primitive callback function type.
 */
typedef void (*ZBPRO_NWK_NetworkFormationConfCallback_t)(ZBPRO_NWK_NetworkFormationReqDescr_t   *const reqDescr,
                                                         ZBPRO_NWK_NetworkFormationConfParams_t *const conf);

/**//**
 * \brief NLME-NETWORK-FORMATION request descriptor definition structure.
 */
typedef struct _ZBPRO_NWK_NetworkFormationReqDescr_t
{
    ZbProNwkServiceField_t                      service;
    ZBPRO_NWK_NetworkFormationReqParams_t       params;
    ZBPRO_NWK_NetworkFormationConfCallback_t    callback;
} ZBPRO_NWK_NetworkFormationReqDescr_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
    \brief NLME-NETWORK-FORMATION request primitive function.
    \param[in] reqDescr - pointer to the request descriptor.
 ***************************************************************************************/
void ZBPRO_NWK_NetworkFormationReq(ZBPRO_NWK_NetworkFormationReqDescr_t *reqDescr);

#endif /* _ZBPRO_NWK_SAP_TYPES_NETWORK_FORMATION_H */