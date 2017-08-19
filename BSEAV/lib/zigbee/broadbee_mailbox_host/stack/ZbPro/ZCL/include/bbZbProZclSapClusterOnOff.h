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
*       ZCL On/Off cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_ONOFF_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_ONOFF_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of On/Off ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  On/Off ZCL cluster has no attributes provided by Client side.
 * \ingroup ZBPRO_ZCL_OnOffAttr
 * \note
 *  This implementation of On/Off ZCL cluster doesn't provide Server side; and its Client
 *  side isn't able to access any of the optional attributes of the Server side, it's able
 *  to access only the following mandatory attributes of the Server side:
 *  - OnOff.
 *
 *  Client is also able to receive reporting of this OnOff attribute.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.8.2.2, 3.8.2.7, 3.8.3.2, table 3-42.
 */
typedef enum _ZBPRO_ZCL_OnOffServerAttributeId_t
{
    ZBPRO_ZCL_ONOFF_ATTR_ID_ONOFF                   = 0x0000,       /*!< OnOff. */

    ZBPRO_ZCL_ONOFF_ATTR_ID_GLOBAL_SCENE_CONTROL    = 0x4000,       /*!< GlobalSceneControl. */

    ZBPRO_ZCL_ONOFF_ATTR_ID_ON_TIME                 = 0x4001,       /*!< OnTime. */

    ZBPRO_ZCL_ONOFF_ATTR_ID_OFF_WAIT_TIME           = 0x4002,       /*!< OffWaitTime. */

    ZBPRO_ZCL_ONOFF_ATTR_ID_MAX                     = 0xFFFF,       /*!< Introduced only to make the enumeration 16-bit
                                                                        wide. */
} ZBPRO_ZCL_OnOffServerAttributeId_t;


/**//**
 * \brief   Data types of attributes of On/Off ZCL cluster.
 * \details
 *  All the listed attributes are transferred by value.
 * \ingroup ZBPRO_ZCL_OnOffAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.8.2.2, table 3-42.
 */
typedef Bool8_t   ZBPRO_ZCL_OnOffAttrOnOff_t;                   /*!< On/Off. */

/**//**
 * \brief   Data types of attributes of On/Off ZCL cluster.
 * \ingroup ZBPRO_ZCL_OnOffAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.8.2.2, table 3-42.
 */
typedef Bool8_t   ZBPRO_ZCL_OnOffAttrGlobalSceneControl_t;      /*!< GlobalSceneControl. */

/**//**
 * \brief   Data types of attributes of On/Off ZCL cluster.
 * \ingroup ZBPRO_ZCL_OnOffAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.8.2.2, table 3-42.
 */
typedef uint16_t  ZBPRO_ZCL_OnOffAttrOnTime_t;                  /*!< OnTime. */

/**//**
 * \brief   Data types of attributes of On/Off ZCL cluster.
 * \ingroup ZBPRO_ZCL_OnOffAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.8.2.2, table 3-42.
 */
typedef uint16_t  ZBPRO_ZCL_OnOffAttrOffWaitTime_t;             /*!< OffWaitTime. */


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Off, On, or Toggle
 *  command.
 * \details
 *  All the implemented commands - Off, On, Toggle - have the same format and differs only
 *  with the Command Id which is determined by using different primitives.
 * \details
 *  Commands Off, On, Toggle have no custom parameters.
 * \ingroup ZBPRO_ZCL_OnOffReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.8.2.3.1, 3.8.2.3.2, 3.8.2.3.3.
 */
typedef struct _ZBPRO_ZCL_OnOffCmdReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_OnOffCmdReqParams_t;

/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OnOffCmdReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Off,
 *  On, or Toggle command.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \ingroup ZBPRO_ZCL_OnOffConf
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_OnOffCmdConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_OnOffCmdConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OnOffCmdConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Off, On, or Toggle
 *  command.
 * \ingroup ZBPRO_ZCL_OnOffReq
 */
typedef struct _ZBPRO_ZCL_OnOffCmdReqDescr_t  ZBPRO_ZCL_OnOffCmdReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Off, On, and Toggle
 *  command.
 * \ingroup ZBPRO_ZCL_OnOffConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_OnOffCmdConfCallback_t(
                ZBPRO_ZCL_OnOffCmdReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_OnOffCmdConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Off, On, or Toggle
 *  command.
 * \ingroup ZBPRO_ZCL_OnOffReq
 */
struct _ZBPRO_ZCL_OnOffCmdReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_OnOffCmdConfCallback_t     *callback;         /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t  service;          /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_OnOffCmdReqParams_t         params;           /*!< ZCL Request parameters structure. */
};

/*
 * Validate structure of ZCL Local Request Descriptor.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_OnOffCmdReqDescr_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief  Functions accept ZCL Local Requests to issue commands of this cluster.
 * \details
 *  The caller shall specify the following obligatory parameters of request: callback,
 *  remoteApsAddress.addrMode, respWaitTimeout, localEndpoint, disableDefaultResp.
 * \details
 *  For the case when remote (destination) node is bound to this (source) node, one may
 *  set the Remote Addressing Mode to NOT_PRESENT and specify only the Local Endpoint and
 *  Cluster on it. APS layer will then accept Remote node Address (extended or group) and
 *  Endpoint corresponding to the specified Local Endpoint according to the Binding Table.
 *  Otherwise, for direct addressing mode, the caller shall also specify the following
 *  parameters: remoteApsAddress, remoteEndpoint. For the case of Local Response to Local
 *  Indication it's enough just to copy all the obligatory parameters of Indication to
 *  Response byte-to-byte to obtain correct destination addressing values of Response.
 * \details
 *  Following parameters are ignored and reassigned by command handlers: localApsAddress,
 *  clusterId, manufCode, commandId, transSeqNum, overallStatus, direction,
 *  clusterSpecific, manufSpecific, useSpecifiedTsn, nonUnicastRequest, useDefaultResponse.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Off command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OnOffCmdOffReq(
                ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue On command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OnOffCmdOnReq(
                ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Toggle command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OnOffCmdToggleReq(
                ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr);


#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_ONOFF_H */

/* eof bbZbProZclSapClusterOnOff.h */