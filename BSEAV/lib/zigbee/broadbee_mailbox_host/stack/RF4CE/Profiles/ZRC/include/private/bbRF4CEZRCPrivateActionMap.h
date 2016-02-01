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
 *   This is the header file for the private Action Mapping Functionality.
 *
 * $Revision: 4621 $
 * $Date: 2014-11-21 17:07:35Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_PRIVATE_ACTION_MAP_H
#define _RF4CE_ZRC_PRIVATE_ACTION_MAP_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CENWKRequestService.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief RF4CE ZRC 2.0 Action map private request descriptor parameters.
 */
typedef struct _rf4ceZrc2ActionMapReqParams_t
{
    uint8_t pairingRef;                 /*!< Pairing reference. */
    uint8_t bank;                       /*!< Action bank. */
    uint8_t code;                       /*!< Action code. */
} rf4ceZrc2ActionMapReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Action map private request confirmation parameters.
 */
typedef struct _rf4ceZrc2ActionMapConfParams_t
{
    SYS_DataPointer_t action;       /*!< If the action was mapped
                                        there is either RF4CE_ZRC2_Action_t or RF4CE_ZRC2_ActionVendor_t structure.
                                        If the action was not mapped there is shall be an empty payload */
} rf4ceZrc2ActionMapConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Action map private request descriptor declaration.
 */
typedef struct _rf4ceZrc2ActionMapReqDescr_t rf4ceZrc2ActionMapReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 Action map private request callback.
 */
typedef void (*rf4ceZrc2ActionMapCallback_t)(rf4ceZrc2ActionMapReqDescr_t *req, rf4ceZrc2ActionMapConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 Action map private request descriptor.
 */
typedef struct _rf4ceZrc2ActionMapReqDescr_t
{
    RF4CE_NWK_RequestService_t      service;        /*!< Service field. */
    rf4ceZrc2ActionMapReqParams_t   params;         /*!< Request parameters. */
    rf4ceZrc2ActionMapCallback_t    callback;       /*!< Request confirmation callback. */
} rf4ceZrc2ActionMapReqDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief requests ZRC 2.0 Action Mapping

 \param[in] request - pointer to the ZRC 2.0 Action map private request descriptor structure.
 ****************************************************************************************/
void rf4ceZrc2ActionMapReq(rf4ceZrc2ActionMapReqDescr_t *request);

/************************************************************************************//**
 \brief ZRC2 Action Mapping task handler.

 \param[in] queueElement - pointer to the queue element structure.
****************************************************************************************/
void rf4ceZrc2ActionMappingHandler(SYS_QueueElement_t *queueElement);

#endif /* _RF4CE_ZRC_PRIVATE_ACTION_MAP_H */
