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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapTunnel.h $
*
* DESCRIPTION:
*   APSME-TUNNEL security service public interface.
*
* $Revision: 2999 $
* $Date: 2014-07-21 13:30:43Z $
*
*****************************************************************************************/

#ifndef _BB_ZBPRO_APS_SAP_TUNNEL_H
#define _BB_ZBPRO_APS_SAP_TUNNEL_H

/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapSecurityTypes.h"

/***************************** TYPES ****************************************************/

/**//**
 * \brief APSME-TUNNEL.request parameters data structure.
 */
typedef struct _ZBPRO_APS_TunnelReqParams_t
{
    ZBPRO_APS_ShortAddr_t       gateNwkAddr;    /* Where tunnel ends */
    ZBPRO_APS_ShortAddr_t       dstNwkAddr;     /* Where the innerCommand has to be delivered.
                                                 * There has to be the appropriate association in the Address Map.
                                                 */
    SYS_DataPointer_t           innerCommand;
} ZBPRO_APS_TunnelReqParams_t;

/**//**
 * \brief APSME-TUNNEL.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_TunnelReqDescr_t  ZBPRO_APS_TunnelReqDescr_t;

/**//**
 * \brief APSME-TUNNEL.confirm callback function data type.
 */
typedef void ZBPRO_APS_TunnelConfCallback_t(ZBPRO_APS_TunnelReqDescr_t       *const reqDescr,
                                                  ZBPRO_APS_SecurityServicesConfParams_t *const confParams);

/**//**
 * \brief APSME-TUNNEL.request descriptor data type.
 */
typedef struct _ZBPRO_APS_TunnelReqDescr_t
{
    /* Fields are arranged to minimize paddings */
    ZBPRO_APS_TunnelConfCallback_t      *callback;      /*!< Confirm callback function.  */

    ZBPRO_APS_TunnelReqParams_t         params;         /*!< Request parameters set.     */

    struct
    {
        SYS_QueueElement_t              queueElement;
    } service;
} ZBPRO_APS_TunnelReqDescr_t;

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief
    Accepts APSME-TUNNEL.requests
  \param    reqDescr
    Pointer to the request descriptor data structure.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_TunnelReq(ZBPRO_APS_TunnelReqDescr_t *reqDescr);

/*************************************************************************************//**
    \brief Receives notification from APS layer that tunneled command has been
           sent to the child.
    \param[in] extAddr - child extended address.
*****************************************************************************************/
void ZBPRO_APS_TunneledTransitCommandNtfy(const ZBPRO_NWK_ExtAddr_t *const extAddr);

#endif /* _BB_ZBPRO_APS_SAP_TUNNEL_H */