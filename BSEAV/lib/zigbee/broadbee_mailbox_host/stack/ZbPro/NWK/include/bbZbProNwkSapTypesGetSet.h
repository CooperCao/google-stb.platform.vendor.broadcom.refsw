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
 *      NWK-NIB Public Access. NLME-GET, NLME-SET API declarations.
 *
*******************************************************************************/

#ifndef _ZBPRO_NWK_GET_SET_H
#define _ZBPRO_NWK_GET_SET_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"
#include "bbZbProNwkSapTypesIb.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Service request type.
 * \ingroup ZBPRO_NWK_GetSetReq
 */
typedef enum _ZbProNwkGetSetReqType_t
{
    ZBPRO_NWK_GET           = 0,
    ZBPRO_NWK_SET           = 1,
    ZBPRO_NWK_GET_KEY       = 2,
    ZBPRO_NWK_SET_KEY       = 3,
} ZbProNwkGetSetReqType_t;

/**//**
 * \brief Service structure type.
 * \ingroup ZBPRO_NWK_GetSetReq
 */
typedef struct _ZbProNwkGetSetServiceField_t
{
    SYS_QueueElement_t              queueElement;  /*!< Service queue field. */
    ZbProNwkGetSetReqType_t         reqType;       /*!< Request type. */
} ZbProNwkGetSetServiceField_t;

/**//**
 * \brief NLME-GET.request parameters structure, see ZigBee Specification r20, 3.2.2.26.
 * \ingroup ZBPRO_NWK_GetReq
 */
typedef struct _ZBPRO_NWK_GetReqParams_t
{
    uint8_t                         id;            /*!< Get ID. */
} ZBPRO_NWK_GetReqParams_t;

/**//**
 * \brief NLME-GET.confirm parameters structure, see ZigBee Specification r20, 3.2.2.27.
 * \ingroup ZBPRO_NWK_GetConf
 */
typedef struct _ZBPRO_NWK_GetConfParams_t
{
    SYS_DataPointer_t               payload;       /*!< Confirmation payload. */
    ZBPRO_NWK_NibAttributeValue_t   value;         /*!< Attribute value. */
    uint8_t                         id;            /*!< Attribute ID. */
    ZBPRO_NWK_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_NWK_GetConfParams_t;

/**//**
 * \brief NLME-GET.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_GetReq
 */
typedef struct _ZBPRO_NWK_GetReqDescr_t  ZBPRO_NWK_GetReqDescr_t;

/**//**
 * \brief NLME-GET.confirm primitive callback function type.
 * \ingroup ZBPRO_NWK_GetConf
 */
typedef void (*ZBPRO_NWK_GetConfCallback_t)(ZBPRO_NWK_GetReqDescr_t *const reqDescr,
        ZBPRO_NWK_GetConfParams_t *const confParams);

/**//**
 * \brief NLME-GET.request primitive structure.
 * \ingroup ZBPRO_NWK_GetReq
 */
typedef struct _ZBPRO_NWK_GetReqDescr_t
{
    ZbProNwkGetSetServiceField_t    service;       /*!< Service field. */
    ZBPRO_NWK_GetReqParams_t        params;        /*!< Request parameters. */
    ZBPRO_NWK_GetConfCallback_t     callback;      /*!< Request callback. */
} ZBPRO_NWK_GetReqDescr_t;

/**//**
 * \brief NLME-SET.request parameters structure, see ZigBee Specification r20, 3.2.2.28.
 * \ingroup ZBPRO_NWK_SetReq
 */
typedef struct _ZBPRO_NWK_SetReqParams_t
{
    SYS_DataPointer_t               payload;       /*!< Request payload. */
    ZBPRO_NWK_NibAttributeValue_t   newValue;      /*!< Attibute value. */
    uint8_t                         id;            /*!< Attribute ID. */
} ZBPRO_NWK_SetReqParams_t;

/**//**
 * \brief NLME-SET.confirm parameters structure, see ZigBee Specification r20, 3.2.2.29.
 * \ingroup ZBPRO_NWK_SetConf
 */
typedef struct _ZBPRO_NWK_SetConfParams_t
{
    uint8_t                         id;            /*!< Attribute ID. */
    ZBPRO_NWK_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_NWK_SetConfParams_t;

/**//**
 * \brief NLME-SET.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_SetReq
 */
typedef struct _ZBPRO_NWK_SetReqDescr_t  ZBPRO_NWK_SetReqDescr_t;

/**//**
 * \brief NLME-SET.confirm primitive callback function type.
 * \ingroup ZBPRO_NWK_SetConf
 */
typedef void (*ZBPRO_NWK_SetConfCallback_t)(ZBPRO_NWK_SetReqDescr_t *const reqDescr,
        ZBPRO_NWK_SetConfParams_t *const confParams);

/**//**
 * \brief NLME-SET.request primitive structure.
 * \ingroup ZBPRO_NWK_SetReq
 */
typedef struct _ZBPRO_NWK_SetReqDescr_t
{
    ZbProNwkGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_NWK_SetReqParams_t        params;        /*!< Request parameters. */
    ZBPRO_NWK_SetConfCallback_t     callback;      /*!< Confirmation callback. */
} ZBPRO_NWK_SetReqDescr_t;

/**//**
 * \brief Get key request parameters.
 * \ingroup ZBPRO_NWK_GetKeyReq
 */
typedef struct _ZBPRO_NWK_GetKeyReqParams_t
{
    ZbProSspNwkKeySeqNum_t          keyCounter;    /*!< Key number. */
} ZBPRO_NWK_GetKeyReqParams_t;

/**//**
 * \brief Get key confirmation parameters.
 * \ingroup ZBPRO_NWK_GetKeyConf
 */
typedef struct _ZBPRO_NWK_GetKeyConfParams_t
{
    ZbProSspKey_t                   key;           /*!< Key descriptor. */
    ZbProSspNwkKeySeqNum_t          keyCounter;    /*!< Key number. */
    ZBPRO_NWK_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_NWK_GetKeyConfParams_t;

/**//**
 * \brief Get key request parameters descriptor type.
 * \ingroup ZBPRO_NWK_GetKeyReq
 */
typedef struct _ZBPRO_NWK_GetKeyReqDescr_t ZBPRO_NWK_GetKeyReqDescr_t;

/**//**
 * \brief Get key confirmation callback.
 * \ingroup ZBPRO_NWK_GetKeyConf
 */
typedef void (*ZBPRO_NWK_GetKeyCallback_t)(ZBPRO_NWK_GetKeyReqDescr_t *const reqDescr,
        ZBPRO_NWK_GetKeyConfParams_t *const confParams);

/**//**
 * \brief Get key request parameters descriptor struct.
 * \ingroup ZBPRO_NWK_GetKeyReq
 */
typedef struct _ZBPRO_NWK_GetKeyReqDescr_t
{
    ZbProNwkGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_NWK_GetKeyReqParams_t     params;        /*!< Request parameters. */
    ZBPRO_NWK_GetKeyCallback_t      callback;      /*!< Confirmation callback. */
} ZBPRO_NWK_GetKeyReqDescr_t;

/**//**
 * \brief Set key request parameters.
 * \ingroup ZBPRO_NWK_SetKeyReq
 */
typedef struct _ZBPRO_NWK_SetKeyReqParams_t
{
    ZbProSspKey_t                   key;           /*!< Key descriptor. */
    ZbProSspNwkKeySeqNum_t          keyCounter;    /*!< Key number. */
} ZBPRO_NWK_SetKeyReqParams_t;

/**//**
 * \brief Set key confirmation parameters.
 * \ingroup ZBPRO_NWK_SetKeyConf
 */
typedef struct _ZBPRO_NWK_SetKeyConfParams_t
{
    ZbProSspNwkKeySeqNum_t          keyCounter;    /*!< Key number. */
    ZBPRO_NWK_Status_t              status;        /*!< Confirmation status. */
} ZBPRO_NWK_SetKeyConfParams_t;

/**//**
 * \brief Set key request parameters descriptor type.
 * \ingroup ZBPRO_NWK_SetKeyReq
 */
typedef struct _ZBPRO_NWK_SetKeyReqDescr_t ZBPRO_NWK_SetKeyReqDescr_t;

/**//**
 * \brief Set key confirmation callback.
 * \ingroup ZBPRO_NWK_SetKeyConf
 */
typedef void (*ZBPRO_NWK_SetKeyCallback_t)(ZBPRO_NWK_SetKeyReqDescr_t *const reqDescr,
        ZBPRO_NWK_SetKeyConfParams_t *const confParams);

/**//**
 * \brief Set key request parameters descriptor struct.
 * \ingroup ZBPRO_NWK_SetKeyReq
 */
typedef struct _ZBPRO_NWK_SetKeyReqDescr_t
{
    ZbProNwkGetSetServiceField_t    service;       /*!< Request service field. */
    ZBPRO_NWK_SetKeyReqParams_t     params;        /*!< Request parameters. */
    ZBPRO_NWK_SetKeyCallback_t      callback;      /*!< Confirmation callback. */
} ZBPRO_NWK_SetKeyReqDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief NLME-GET.request primitive function.
  \ingroup ZBPRO_NWK_Functions

  \param[in] req - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_NWK_GetReq(ZBPRO_NWK_GetReqDescr_t *req);

/************************************************************************************//**
  \brief NLME-SET.request primitive function.
  \ingroup ZBPRO_NWK_Functions

  \param[in] req - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_NWK_SetReq(ZBPRO_NWK_SetReqDescr_t *req);

/************************************************************************************//**
  \brief Function gets key from the security material storage.
  \ingroup ZBPRO_NWK_Functions

  \param[in] req - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_NWK_GetKeyReq(ZBPRO_NWK_GetKeyReqDescr_t *req);

/************************************************************************************//**
  \brief Function add/update key to the security material storage.
  \ingroup ZBPRO_NWK_Functions

  \param[in] req - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_NWK_SetKeyReq(ZBPRO_NWK_SetKeyReqDescr_t *req);

#endif /* _ZBPRO_NWK_GET_SET_H */

/* eof bbZbProNwkSapTypesGetSet.h */