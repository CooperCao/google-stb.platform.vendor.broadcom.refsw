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
*       ZCL Attributes Discover handler private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_HANDLER_ATTRIBUTES_DISCOVER_H
#define _BB_ZBPRO_ZCL_HANDLER_ATTRIBUTES_DISCOVER_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapAttributesDiscover.h"
#include "private/bbZbProZclHandlerProfileWideCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Discover Attributes
 *  response request.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    Bool8_t                                 isDiscoveryComplete;        /*!< Is Discovery Complete value. */
    SYS_DataPointer_t                       payload;                    /*!< The discovered attributes values. */

} ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqParams_t;

/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqParams_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on Attributes Discover response.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
} ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfParams_t);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue the Discover Attributes Response.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqDescr_t  ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Discover Attributes response.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Discover Attributes response.
 */
struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespConfCallback_t   *callback;  /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;   /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespReqParams_t         params;  /*!< ZCL Request parameters structure. */
};

/*
 * Validate structure of ZCL Local Request Descriptor.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t);


/************************* PROTOTYPES ***************************************************/
/*
 * Accepts ZCL Local Request to issue Discover Attributes Response profile-wide command.
 */
void ZBPRO_ZCL_ProfileWideCmdDiscoverAttributesResponseReq(
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrRespDescr_t *const  reqDescr);

/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Discover Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.13.1, figure 2-26.
 */
bool zbProZclHandlerProfileWideComposeDiscoverAttr(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Discover Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.14.1, figures 2-27, 2-28.
 */
bool zbProZclHandlerProfileWideComposeDiscoverAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Discover Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.13.1, figures 2-26.
 */
void zbProZclHandlerProfileWideParseDiscoverAttr(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Discover Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      confParams          Pointer to ZCL Local Confirm Parameters Prototype.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.14.1, figures 2-27, 2-28.
 */
void zbProZclHandlerProfileWideParseDiscoverAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  confParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

#endif /* _BB_ZBPRO_ZCL_HANDLER_ATTRIBUTES_DISCOVER_H */

/* eof bbZbProZclHandlerAttributesDiscover.h */