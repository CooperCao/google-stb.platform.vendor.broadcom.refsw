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
 *      This header describes types and API for the ZDO ZDP service Mgmt_Bind
 *
*******************************************************************************/

#ifndef _ZBPRO_ZDO_SAP_TYPES_MGMT_BIND_HANDLER_H
#define _ZBPRO_ZDO_SAP_TYPES_MGMT_BIND_HANDLER_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"
#include "bbZbProApsCommon.h"

/************************* TYPES ********************************************************/


/*************************************************************************************//**
  \brief   Structure for parameters of ZDO Local Request to issue ZDP Mgmt_Bind_req
  command. This structure takes its origin from ZDP Mgmt_Bind_req command
  \ingroup ZBPRO_ZDO_MgmtBindReq
  \par  Documentation
  See ZigBee Document 053474r20, subclause 2.4.3.3.4, figure 2.56, table 2.83.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtBindReqParams_t
{
    ZBPRO_ZDO_Address_t                 zdpDstAddress;    /*!< Destination address. It shall be unicast only.*/

    BitField8_t                         startIndex;       /*!< Starting Index for the requested elements of
                                                               the Neighbor Table. */
} ZBPRO_ZDO_MgmtBindReqParams_t;


/*************************************************************************************//**
  \brief   Structure for parameters of ZDO Local Confirmation on ZDP Mgmt_Bind_req
  command. This structure takes its origin from ZDP Mgmt_Bind_rsp command
  \ingroup ZBPRO_ZDO_MgmtBindConf
  \par  Documentation
  See ZigBee Document 053474r20, subclause 2.4.4.3.4, figure 2.97, table 2.130.
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtBindConfParams_t
{
    ZBPRO_ZDO_Status_t  status;                   /*!< The status of the Mgmt_Bind_req command. */

    BitField8_t         bindingTableEntries;      /*!< Total number of Binding Table entries
                                                       within the Remote Device.*/

    BitField8_t         startIndex;               /*!< Starting Index for the requested elements of
                                                       the Neighbor Table. */

    BitField8_t         bindingTableListCount;    /*!< Number of Binding Table entries included
                                                       within BindingTableList. */

    SYS_DataPointer_t   payload;                  /*!< A list of descriptors, beginning with the
                                                       StartIndex element and continuing for
                                                       BindingTableListCount, of the elements in the
                                                       Remote Device's Binding Table
                                                       (see ZBPRO_APS_BindingTableEntity_t from
                                                       bbZbProApsBindingTableInterface.h). */
} ZBPRO_ZDO_MgmtBindConfParams_t;


/*************************************************************************************//**
  \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Bind_req
  command.
  \ingroup ZBPRO_ZDO_MgmtBindReq
*****************************************************************************************/
typedef struct _ZBPRO_ZDO_MgmtBindReqDescr_t  ZBPRO_ZDO_MgmtBindReqDescr_t;


/*************************************************************************************//**
  \brief  Data type for ZDO Local Confirmation callback function of ZDP Mgmt_Bind_req
  command.
  \ingroup ZBPRO_ZDO_MgmtBindConf
  \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
  \param[in]   confParams      Pointer to the confirmation parameters structure.
*****************************************************************************************/
typedef void ZBPRO_ZDO_MgmtBindConfCallback_t(ZBPRO_ZDO_MgmtBindReqDescr_t   *const reqDescr,
                                              ZBPRO_ZDO_MgmtBindConfParams_t *const confParams);


/*************************************************************************************//**
  \brief  Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Bind_req
  command.
  \ingroup ZBPRO_ZDO_MgmtBindReq
*****************************************************************************************/
struct _ZBPRO_ZDO_MgmtBindReqDescr_t
{
    ZBPRO_ZDO_MgmtBindConfCallback_t   *callback;       /*!< ZDO Confirmation callback handler entry point. */

    ZbProZdoLocalRequest_t             service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtBindReqParams_t      params;          /*!< ZDO Request parameters structure. */
};



/************************* PROTOTYPES ***************************************************/


/*************************************************************************************//**
 * \brief  Accepts ZDO Local Request to issue ZDP Mgmt_Bind_req command.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \return Nothing.
*****************************************************************************************/
void ZBPRO_ZDO_MgmtBindReq(ZBPRO_ZDO_MgmtBindReqDescr_t *const  reqDescr);


 #endif

/* eof bbZbProZdoSapTypesMgmtBind.h */