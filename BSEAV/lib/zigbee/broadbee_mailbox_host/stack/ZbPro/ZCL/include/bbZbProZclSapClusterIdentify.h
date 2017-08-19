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
*       ZCL Identify cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_IDENTIFY_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_IDENTIFY_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Identify ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Identify ZCL cluster has no attributes provided by Client side.
 * \ingroup ZBPRO_ZCL_IdentifyAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.5.2.2, 3.5.3.2, table 3-28.
 */
typedef enum _ZBPRO_ZCL_IdentifyServerAttributeId_t
{
    ZBPRO_ZCL_IDENTIFY_ATTR_ID_IDENTIFY_TIME    = 0x0000,       /*!< IdentifyTime. */

    ZBPRO_ZCL_IDENTIFY_ATTR_ID_MAX              = 0xFFFF,       /*!< Introduced only to make the enumeration 16-bit
                                                                        wide. */
} ZBPRO_ZCL_IdentifyServerAttributeId_t;


/**//**
 * \brief   Data type shared by attributes and command parameters of Identify cluster.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.5.2.2, 3.5.2.3, 3.5.2.4, tables 3-28,
 *  3-32, figures 3-6, 3-8.
 */
typedef uint16_t  ZBPRO_ZCL_IdentifyParamIdentifyTime_t;        /*!< Shared data type for IdentifyTime parameter, in
                                                                    seconds. */


/**//**
 * \brief   Special values shared by attributes and command parameters of Identify
 *  cluster.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.2.1.
 */
#define ZBPRO_ZCL_IDENTIFY_TERMINATE_IDENTIFICATION     0x0000      /*!< When assigned to IdentifyTime attribute, or
                                                                        when this attribute reaches this value itself
                                                                        (by decrementing its current value), it makes
                                                                        device terminate its identification
                                                                        procedure. */


/**//**
 * \brief   Data types of attributes of Identify ZCL cluster.
 * \details
 *  All the listed attributes are transferred by value.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.2, table 3-28.
 */
typedef ZBPRO_ZCL_IdentifyParamIdentifyTime_t  ZBPRO_ZCL_IdentifyAttrIdentifyTime_t;        /*!< Identify Time. */


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 * \note
 *  There is an error in Document 075123r05, subclause 3.5.2.3.1, figure 3-6: both
 *  subclause and figure refer to the Identify Query Response command instead of the
 *  Identify Command. The same error persists also in two previous revisions of this
 *  document: 075123r04 and 075123r03. Nevertheless, it is assumed that the format of the
 *  Identify command, as given at the figure 3-6, is correct, because there is also figure
 *  3-8 which defines slightly different format of the Identify Query Response command.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.3.1, figure 3-6.
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    ZBPRO_ZCL_IdentifyParamIdentifyTime_t   identifyTime;           /*!< IdentifyTime, in seconds. */

} ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Default Response
 *  command with application status on received Identify command.
 * \details
 *  Application shall report its status in \p zclObligatoryPart.overallStatus.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \ingroup ZBPRO_ZCL_IdentifyResponseReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Identify Query command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.3.2.
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Identify Query Response
 *  or Default Response command.
 * \details
 *  The Identify Query Response command is issued if \c timeout parameter is not equal to
 *  zero; inversely, the Default Response command is issued.
 * \ingroup ZBPRO_ZCL_IdentifyQueryResponseReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.4.1, figure 3-8.
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    ZBPRO_ZCL_IdentifyParamIdentifyTime_t   timeout;                /*!< Timeout, in seconds. */

} ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t;


/*
 * Validate structures of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue
 *  this cluster commands.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \ingroup ZBPRO_ZCL_IdentifyConf
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_IdentifyCmdConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IdentifyCmdConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t  ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Default Response
 *  command with application status on received Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyResponseReq
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t  ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify Query command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryReq
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t  ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify Query Response
 *  or Default Response command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryResponseReq
 */
typedef struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t
        ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IdentifyCmdIdentifyConfCallback_t(
                ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_IdentifyCmdConfParams_t       *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Default Response
 *  command sent with application status on received Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyResponseConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IdentifyCmdIdentifyResponseConfCallback_t(
                ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_IdentifyCmdConfParams_t               *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Identify Query
 *  command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IdentifyCmdIdentifyQueryConfCallback_t(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_IdentifyCmdConfParams_t            *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Identify Query
 *  Response or Default Response command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryResponseConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseConfCallback_t(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_IdentifyCmdConfParams_t                    *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyReq
 */
struct _ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IdentifyCmdIdentifyConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t     params;        /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Default Response
 *  command with application status on received Identify command.
 * \ingroup ZBPRO_ZCL_IdentifyResponseReq
 */
struct _ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IdentifyCmdIdentifyResponseConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                 service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t     params;        /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify Query command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryReq
 */
struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryConfCallback_t *callback;     /*!< ZCL Confirmation callback handler entry
                                                                        point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t              service;      /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t     params;       /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Identify Query Response
 *  or Default Response command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryResponseReq
 */
struct _ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseConfCallback_t *callback;     /*!< ZCL Confirmation callback handler entry
                                                                                point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                      service;      /*!< ZCL Request Descriptor service
                                                                                field. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t     params;       /*!< ZCL Request parameters structure. */
};


/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Indication of received Identify command.
 * \details
 *  This structure coincides with parameters of ZCL Local Request to issue Identify
 *  command.
 * \ingroup ZBPRO_ZCL_IdentifyInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.3.1, figure 3-6.
 */
typedef ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t  ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Indication of received Identify Query
 *  command.
 * \details
 *  This structure coincides with parameters of ZCL Local Request to issue Identify Query
 *  command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.3.2.
 */
typedef ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t  ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Indication of received Identify Query
 *  Response command.
 * \details
 *  This structure coincides with parameters of ZCL Local Request to issue Identify Query
 *  Response command.
 * \details
 *  Command Identify Query, if sent by Client, allows multiple responses from addressed
 *  Servers. Due to this reason each Identify Query Response is indicated to the Client
 *  application individually, but not with confirmation on original Identify Query; while
 *  original Identify Query is confirmed right on confirmation from APS layer on request
 *  to transmit this command.
 * \ingroup ZBPRO_ZCL_IdentifyQueryResponseInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.4.1, figure 3-8.
 */
typedef ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t  ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t;


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
 *  For the case of request to send Local Response command the caller shall additionally
 *  specify the overallStatus and copy the following parameters from corresponding Local
 *  Indication: transSeqNum, nonUnicastRequest. For the case of request to send Local
 *  Request command these parameters are ignored.
 * \details
 *  Following parameters are ignored and reassigned by command handlers: localApsAddress,
 *  clusterId, manufCode, commandId, direction, clusterSpecific, manufSpecific,
 *  useSpecifiedTsn, useDefaultResponse.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Identify command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyReq(
                ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Default Response command with application
 *  status on received Identify command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq(
                ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Identify Query command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Identify Query Response or Default Response
 *  command.
 * \details
 *  The Identify Query Response command is issued if \c timeout parameter is not equal to
 *  zero; inversely, the Default Response command is issued.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief  Functions handle ZCL Local Indication on reception of commands of this cluster.
 * \details
 *  The higher-level layer (application) or one of ZHA Managers shall define these
 *  callback handler functions.
 */

/**//**
 * \brief   Handles ZCL Local Indication on reception of Identify command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyInd(
                ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t *const  indParams);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Identify Query command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t *const  indParams);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Identify Query Response command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Identify Query Response command is sent in response to Identify Query command but
 *  it is processed by the client unsolicitedly (because multiple response is allowed).
 *  Due to this reason client side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseInd(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t *const  indParams);


/**//**
 * \brief   Test version of Identify Query Response command Indication.
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Identify Query Response command is sent in response to Identify Query command but
 *  it is processed by the client unsolicitedly (because multiple response is allowed).
 *  Due to this reason client side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB(
                ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_IDENTIFY_H */

/* eof bbZbProZclSapClusterIdentify.h */