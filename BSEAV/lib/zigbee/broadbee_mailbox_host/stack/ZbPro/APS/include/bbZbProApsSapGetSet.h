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
 *      APS-AIB Public Access. APSME-GET, APSME-SET API declarations.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_SAP_GET_SET_H
#define _ZBPRO_APS_SAP_GET_SET_H

/************************* INCLUDES *****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsSapIb.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief Service request type.
 * \ingroup ZBPRO_APS_GetSetReq
 */
typedef enum
{
    ZBPRO_APS_GET       = 0,
    ZBPRO_APS_SET       = 1,
    ZBPRO_APS_GET_KEY   = 2,
    ZBPRO_APS_SET_KEY   = 3,
} ZbProApsGetSetReqType_t;

/**//**
 * \brief Service structure type.
 * \ingroup ZBPRO_APS_GetSetReq
 */
typedef struct _ZbProApsGetSetServiceField_t
{
    SYS_QueueElement_t              queueElement;  /*!< Service queue field. */
    ZbProApsGetSetReqType_t         reqType;       /*!< Request type. */
} ZbProApsGetSetServiceField_t;

/**//**
 * \brief APSME-GET.request parameters structure.
 * \ingroup ZBPRO_APS_GetReq
 */
typedef struct _ZBPRO_APS_GetReqParams_t
{
    uint8_t                         id;            /*!< Get ID. */
} ZBPRO_APS_GetReqParams_t;

/**//**
 * \brief APSME-GET.confirm parameters structure.
 * \ingroup ZBPRO_APS_GetConf
 */
typedef struct _ZBPRO_APS_GetConfParams_t
{
    SYS_DataPointer_t               payload;       /*!< Confirmation payload. */
    ZBPRO_APS_IbAttributeValue_t    value;         /*!< Attribute value. */
    uint8_t                         id;            /*!< Attribute ID. */
    ZBPRO_APS_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_APS_GetConfParams_t;

/**//**
 * \brief APSME-GET.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_GetReq
 */
typedef struct _ZBPRO_APS_GetReqDescr_t  ZBPRO_APS_GetReqDescr_t;

/**//**
 * \brief APSME-GET.confirm primitive callback function type.
 * \ingroup ZBPRO_APS_GetConf
 */
typedef void (*ZBPRO_APS_GetConfCallback_t)(ZBPRO_APS_GetReqDescr_t   *const reqDescr,
        ZBPRO_APS_GetConfParams_t *const confParams);

/**//**
 * \brief APSME-GET.request primitive structure.
 * \ingroup ZBPRO_APS_GetReq
 */
typedef struct _ZBPRO_APS_GetReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;       /*!< Service field. */
    ZBPRO_APS_GetReqParams_t        params;        /*!< Request parameters. */
    ZBPRO_APS_GetConfCallback_t     callback;      /*!< Request callback. */
} ZBPRO_APS_GetReqDescr_t;

/**//**
 * \brief APSME-SET.request parameters structure.
 * \ingroup ZBPRO_APS_SetReq
 */
typedef struct _ZBPRO_APS_SetReqParams_t
{
    SYS_DataPointer_t               payload;       /*!< Request payload. */
    ZBPRO_APS_IbAttributeValue_t    value;         /*!< Attibute new value. */
    uint8_t                         id;            /*!< Attribute ID. */
} ZBPRO_APS_SetReqParams_t;

/**//**
 * \brief APSME-SET.confirm parameters structure.
 * \ingroup ZBPRO_APS_SetConf
 */
typedef struct _ZBPRO_APS_SetConfParams_t
{
    uint8_t                         id;            /*!< Attribute ID. */
    ZBPRO_APS_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_APS_SetConfParams_t;

/**//**
 * \brief APSME-SET.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_SetReq
 */
typedef struct _ZBPRO_APS_SetReqDescr_t  ZBPRO_APS_SetReqDescr_t;

/**//**
 * \brief APSME-SET.confirm primitive callback function type.
 * \ingroup ZBPRO_APS_SetConf
 */
typedef void (*ZBPRO_APS_SetConfCallback_t)(ZBPRO_APS_SetReqDescr_t   *const reqDescr,
        ZBPRO_APS_SetConfParams_t *const confParams);

/**//**
 * \brief APSME-SET.request primitive structure.
 * \ingroup ZBPRO_APS_SetReq
 */
struct _ZBPRO_APS_SetReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_APS_SetReqParams_t        params;        /*!< Request parameters. */
    ZBPRO_APS_SetConfCallback_t     callback;      /*!< Confirmation callback. */
};

/**//**
 * \brief APSME-GET(aps key).request parameters structure.
 * \ingroup ZBPRO_APS_GetKeyReq
 */
typedef struct _ZBPRO_APS_GetKeyReqParams_t
{
    ZBPRO_APS_ExtAddr_t             deviceAddr;    /*!< Device extended address. */
} ZBPRO_APS_GetKeyReqParams_t;

/**//**
 * \brief APSME-GET(aps key).confirm parameters structure.
 * \ingroup ZBPRO_APS_GetKeyConf
 */
typedef struct _ZBPRO_APS_GetKeyConfParams_t
{
    ZbProSspKey_t                   key;           /*!< Key descriptor. */
    ZBPRO_APS_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_APS_GetKeyConfParams_t;

/**//**
 * \brief APSME-GET(aps key).request descriptor data type declaration.
 * \ingroup ZBPRO_APS_GetKeyReq
 */
typedef struct _ZBPRO_APS_GetKeyReqDescr_t  ZBPRO_APS_GetKeyReqDescr_t;

/**//**
 * \brief APSME-GET(aps key).confirm primitive callback function type.
 * \ingroup ZBPRO_APS_GetKeyConf
 */
typedef void (*ZBPRO_APS_GetKeyConfCallback_t)(ZBPRO_APS_GetKeyReqDescr_t   *const reqDescr,
        ZBPRO_APS_GetKeyConfParams_t *const confParams);

/**//**
 * \brief APSME-GET(aps key).request primitive structure.
 * \ingroup ZBPRO_APS_GetKeyReq
 */
typedef struct _ZBPRO_APS_GetKeyReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_APS_GetKeyReqParams_t     params;        /*!< Request parameters. */
    ZBPRO_APS_GetKeyConfCallback_t  callback;      /*!< Confirmation callback. */
} ZBPRO_APS_GetKeyReqDescr_t;

/**//**
 * \brief APSME-SET(aps key).request parameters structure.
 * \ingroup ZBPRO_APS_SetKeyReq
 */
typedef struct _ZBPRO_APS_SetKeyReqParams_t
{
    ZbProSspKey_t                   newKeyValue;   /*!< New key value. */
    ZBPRO_APS_ExtAddr_t             deviceAddr;    /*!< Device extended address. */
} ZBPRO_APS_SetKeyReqParams_t;

/**//**
 * \brief APSME-SET(aps key).confirm parameters structure.
 * \ingroup ZBPRO_APS_SetKeyConf
 */
typedef struct _ZBPRO_APS_SetKeyConfParams_t
{
    ZBPRO_APS_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_APS_SetKeyConfParams_t;

/**//**
 * \brief APSME-SET(aps key).request descriptor data type declaration.
 * \ingroup ZBPRO_APS_SetKeyReq
 */
typedef struct _ZBPRO_APS_SetKeyReqDescr_t  ZBPRO_APS_SetKeyReqDescr_t;

/**//**
 * \brief APSME-SET(aps key).confirm primitive callback function type.
 * \ingroup ZBPRO_APS_SetKeyConf
 */
typedef void (*ZBPRO_APS_SetKeyConfCallback_t)(ZBPRO_APS_SetKeyReqDescr_t   *const reqDescr,
        ZBPRO_APS_SetKeyConfParams_t *const confParams);

/**//**
 * \brief APSME-SET(aps key).request primitive structure.
 * \ingroup ZBPRO_APS_SetKeyReq
 */
typedef struct _ZBPRO_APS_SetKeyReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_APS_SetKeyReqParams_t     params;        /*!< Request parameters. */
    ZBPRO_APS_SetKeyConfCallback_t  callback;      /*!< Confirmation callback. */
} ZBPRO_APS_SetKeyReqDescr_t;


/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief APSME-GET.request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_GetReq(ZBPRO_APS_GetReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-SET.request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_SetReq(ZBPRO_APS_SetReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-GET(aps key).request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_GetKeyReq(ZBPRO_APS_GetKeyReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-SET(aps key).request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_SetKeyReq(ZBPRO_APS_SetKeyReqDescr_t *const reqDescr);

#endif /* _ZBPRO_APS_SAP_GET_SET_H */

/* eof bbZbProApsSapGetSet.h */