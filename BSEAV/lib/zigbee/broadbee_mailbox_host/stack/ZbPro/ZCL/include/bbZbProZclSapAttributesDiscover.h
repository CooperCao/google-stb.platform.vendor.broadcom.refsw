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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZCL Attributes Discover SAP interface.
*
* $Revision: $
* $Date: $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_ATTRIBUTES_DISCOVER_H
#define _BB_ZBPRO_ZCL_SAP_ATTRIBUTES_DISCOVER_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"
#include "bbZbProZclSapProfileWideAttributes.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Discover Attributes
 *  request.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    ZBPRO_ZCL_AttributeId_t                 minAttributeId;             /*!< Minimum attribute ID value. */
    uint8_t                                 maxAttributes;              /*!< Maximum amount of attributes to return. */

} ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t;

typedef ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t ZBPRO_ZCL_ProfileWideCmdDiscoverAttrIndParams_t;

/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on Attributes Discover request.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    Bool8_t isDiscoveryComplete;                                        /*!< true if discovery is complete. */
    SYS_DataPointer_t payload;                                          /*!< The discovered attributes values. */

} ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue the Discover Attributes Request.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqDescr_t  ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Discover Attributes request.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Discover Attributes request.
 */
struct _ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfCallback_t   *callback;  /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;   /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t         params;  /*!< ZCL Request parameters structure. */
};

/*
 * Validate structure of ZCL Local Request Descriptor.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t);

/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Function accepts ZCL Local Request to issue the Discover Attributes request.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq(
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t *const  reqDescr);

/**//**
 * \brief   Handles ZCL Local Indication on reception of Discover Attribute profile-wide
 *  command.
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 */
void ZBPRO_ZCL_ProfileWideCmdDiscoverAttributesInd(
                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZCL_SAP_ATTRIBUTES_DISCOVER_H */
