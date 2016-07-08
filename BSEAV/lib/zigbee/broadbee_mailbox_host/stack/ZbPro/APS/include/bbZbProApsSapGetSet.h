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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapGetSet.h $
 *
 * DESCRIPTION:
 *   APS-AIB Public Access. APSME-GET, APSME-SET API declarations.
 *
 * $Revision: 2437 $
 * $Date: 2014-05-19 13:56:29Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_APS_SAP_GET_SET_H
#define _ZBPRO_APS_SAP_GET_SET_H

/************************* INCLUDES *****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsSapIb.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief Service request type.
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
 */
typedef struct _ZbProApsGetSetServiceField_t
{
    SYS_QueueElement_t              queueElement;
    ZbProApsGetSetReqType_t         reqType;
} ZbProApsGetSetServiceField_t;

/**//**
 * \brief APSME-GET.request parameters structure.
 */
typedef struct _ZBPRO_APS_GetReqParams_t
{
    uint8_t                         id;
} ZBPRO_APS_GetReqParams_t;

/**//**
 * \brief APSME-GET.confirm parameters structure.
 */
typedef struct _ZBPRO_APS_GetConfParams_t
{
    SYS_DataPointer_t               payload;
    ZBPRO_APS_IbAttributeValue_t    value;
    uint8_t                         id;
    ZBPRO_APS_Status_t              status;
} ZBPRO_APS_GetConfParams_t;

/**//**
 * \brief APSME-GET.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_GetReqDescr_t  ZBPRO_APS_GetReqDescr_t;

/**//**
 * \brief APSME-GET.confirm primitive callback function type.
 */
typedef void (*ZBPRO_APS_GetConfCallback_t)(ZBPRO_APS_GetReqDescr_t   *const reqDescr,
        ZBPRO_APS_GetConfParams_t *const confParams);

/**//**
 * \brief APSME-GET.request primitive structure.
 */
typedef struct _ZBPRO_APS_GetReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;
    ZBPRO_APS_GetReqParams_t        params;
    ZBPRO_APS_GetConfCallback_t     callback;
} ZBPRO_APS_GetReqDescr_t;

/**//**
 * \brief APSME-SET.request parameters structure.
 */
typedef struct _ZBPRO_APS_SetReqParams_t
{
    SYS_DataPointer_t               payload;
    ZBPRO_APS_IbAttributeValue_t    value;
    uint8_t                         id;
} ZBPRO_APS_SetReqParams_t;

/**//**
 * \brief APSME-SET.confirm parameters structure.
 */
typedef struct _ZBPRO_APS_SetConfParams_t
{
    uint8_t                         id;
    ZBPRO_APS_Status_t              status;
} ZBPRO_APS_SetConfParams_t;

/**//**
 * \brief APSME-SET.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_SetReqDescr_t  ZBPRO_APS_SetReqDescr_t;

/**//**
 * \brief APSME-SET.confirm primitive callback function type.
 */
typedef void (*ZBPRO_APS_SetConfCallback_t)(ZBPRO_APS_SetReqDescr_t   *const reqDescr,
        ZBPRO_APS_SetConfParams_t *const confParams);

/**//**
 * \brief APSME-SET.request primitive structure.
 */
struct _ZBPRO_APS_SetReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;
    ZBPRO_APS_SetReqParams_t        params;
    ZBPRO_APS_SetConfCallback_t     callback;
};

typedef struct _ZBPRO_APS_GetKeyReqParams_t
{
    ZBPRO_APS_ExtAddr_t             deviceAddr;
} ZBPRO_APS_GetKeyReqParams_t;

typedef struct _ZBPRO_APS_GetKeyConfParams_t
{
    ZbProSspKey_t                   key;
    ZBPRO_APS_Status_t              status;
} ZBPRO_APS_GetKeyConfParams_t;

typedef struct _ZBPRO_APS_GetKeyReqDescr_t  ZBPRO_APS_GetKeyReqDescr_t;

typedef void (*ZBPRO_APS_GetKeyConfCallback_t)(ZBPRO_APS_GetKeyReqDescr_t   *const reqDescr,
        ZBPRO_APS_GetKeyConfParams_t *const confParams);
typedef struct _ZBPRO_APS_GetKeyReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;
    ZBPRO_APS_GetKeyReqParams_t     params;
    ZBPRO_APS_GetKeyConfCallback_t  callback;
} ZBPRO_APS_GetKeyReqDescr_t;

typedef struct _ZBPRO_APS_SetKeyReqParams_t
{
    ZbProSspKey_t                   newKeyValue;
    ZBPRO_APS_ExtAddr_t             deviceAddr;
} ZBPRO_APS_SetKeyReqParams_t;

typedef struct _ZBPRO_APS_SetKeyConfParams_t
{
    ZBPRO_APS_Status_t              status;
} ZBPRO_APS_SetKeyConfParams_t;
typedef struct _ZBPRO_APS_SetKeyReqDescr_t  ZBPRO_APS_SetKeyReqDescr_t;
typedef void (*ZBPRO_APS_SetKeyConfCallback_t)(ZBPRO_APS_SetKeyReqDescr_t   *const reqDescr,
        ZBPRO_APS_SetKeyConfParams_t *const confParams);
typedef struct _ZBPRO_APS_SetKeyReqDescr_t
{
    ZbProApsGetSetServiceField_t    service;
    ZBPRO_APS_SetKeyReqParams_t     params;
    ZBPRO_APS_SetKeyConfCallback_t  callback;
} ZBPRO_APS_SetKeyReqDescr_t;


/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief APSME-GET.request primitive function.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void ZBPRO_APS_GetReq(ZBPRO_APS_GetReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-SET.request primitive function.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void ZBPRO_APS_SetReq(ZBPRO_APS_SetReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-GET(aps key).request primitive function.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void ZBPRO_APS_GetKeyReq(ZBPRO_APS_GetKeyReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief APSME-SET(aps key).request primitive function.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void ZBPRO_APS_SetKeyReq(ZBPRO_APS_SetKeyReqDescr_t *const reqDescr);

#endif /* _ZBPRO_APS_SAP_GET_SET_H */
