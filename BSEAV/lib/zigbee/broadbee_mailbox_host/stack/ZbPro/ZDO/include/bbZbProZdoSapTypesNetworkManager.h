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
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoSapTypesNetworkManager.h $
 *
 * DESCRIPTION:
 *   This header describes types and API for the ZDO Network Manager component.
 *
 * $Revision: 2876 $
 * $Date: 2014-07-10 09:58:52Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_ZDO_SAP_TYPES_NETWORK_MANAGER_H
#define _ZBPRO_ZDO_SAP_TYPES_NETWORK_MANAGER_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief ZDO Start network confirm parameters.
 */
typedef struct _ZBPRO_ZDO_StartNetworkConfParams_t
{
    ZBPRO_ZDO_ExtPanId_t    extendedPanId;
    ZBPRO_ZDO_PanId_t       panId;
    ZBPRO_ZDO_NwkAddr_t     networkAddress;
    ZBPRO_ZDO_Channel_t     activeChannel;
    ZBPRO_ZDO_Status_t      status;
} ZBPRO_ZDO_StartNetworkConfParams_t;

/**//**
 * \brief ZDO Start descriptor prototype.
 */
typedef struct _ZBPRO_ZDO_StartNetworkReqDescr_t ZBPRO_ZDO_StartNetworkReqDescr_t;

/**//**
 * \brief Callback function type.
 */
typedef void ZBPRO_ZDO_StartNetworkConfCallback_t(ZBPRO_ZDO_StartNetworkReqDescr_t *const reqDescr,
        ZBPRO_ZDO_StartNetworkConfParams_t *const confParams);

/**//**
 * \brief ZDO Start descriptor data type.
 */
struct _ZBPRO_ZDO_StartNetworkReqDescr_t
{
    struct
    {
        SYS_QueueElement_t queueElement;
    } service;

    ZBPRO_ZDO_StartNetworkConfCallback_t *callback;
};

/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief Initiates the device network joining procedure for the current device.
  \param[in] reqDescr - pointer to the request structure.
*****************************************************************************************/
void ZBPRO_ZDO_StartNetworkReq(ZBPRO_ZDO_StartNetworkReqDescr_t *const reqDescr);

#endif /* _ZBPRO_ZDO_SAP_TYPES_NETWORK_MANAGER_H */