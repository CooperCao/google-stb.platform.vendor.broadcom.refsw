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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/bbRF4CEMSOBind.h $
 *
 * DESCRIPTION:
 *   This is the header file for the MSO RF4CE Profile
 *   Binding component.
 *
 * $Revision: 2579 $
 * $Date: 2014-06-02 07:16:01Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_BIND_H
#define _RF4CE_MSO_BIND_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CENWKConstants.h"
#include "bbSysQueue.h"
#include "bbRF4CENWKRequestService.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO Bind result status.
 */
typedef enum _RF4CE_MSO_BindStatus_t
{
    RF4CE_MSO_BOUND = 0,
    RF4CE_MSO_BIND_ERROR_DISCOVERY,
    RF4CE_MSO_BIND_ERROR_PAIRING,
    RF4CE_MSO_BIND_ERROR_VALIDATION,
    RF4CE_MSO_BIND_ERROR_TIMEOUT,
    RF4CE_MSO_BIND_ERROR_FULL_ABORT,
    RF4CE_MSO_BIND_ERROR_NOT_SUPPORTED
} RF4CE_MSO_BindStatus_t;

/**//**
 * \brief RF4CE MSO Bind Target side Primary Class index.
 */
#define RF4CE_MSO_PRIMARY_CLASS_INDEX 12

/**//**
 * \brief RF4CE MSO Bind Target side Secondary Class index.
 */
#define RF4CE_MSO_SECONDARY_CLASS_INDEX 11

/**//**
 * \brief RF4CE MSO Bind Target side Tertiary Class index.
 */
#define RF4CE_MSO_TERTIARY_CLASS_INDEX 10

/**//**
 * \brief RF4CE MSO Bind Target side Strict LQI Threshold index.
 */
#define RF4CE_MSO_STRICT_LQI_THRESHOLD_INDEX 13

/**//**
 * \brief RF4CE MSO Bind Target side Basic LQI Threshold index.
 */
#define RF4CE_MSO_BASIC_LQI_THRESHOLD_INDEX 14

/**//**
 * \brief RF4CE MSO Bind Target side class management macros.
 */
#define RF4CE_MSO_GET_CLASS_NUMBER(cls) ((cls) & 0xF)
#define RF4CE_MSO_SET_CLASS_NUMBER(cls, value) (((cls) & 0xF0) | ((value) & 0xF))
#define RF4CE_MSO_GET_DUPLICATE_CLASS_HANDLING(cls) (((cls) >> 4) & 0x3)
#define RF4CE_MSO_SET_DUPLICATE_CLASS_HANDLING(cls, value) (((cls) & 0xCF) | (((value) << 4) & 0x30))
#define RF4CE_MSO_GET_APPLY_STRICT_LQI_THRESHOLD(cls) (((cls) >> 6) & 0x1)
#define RF4CE_MSO_SET_APPLY_STRICT_LQI_THRESHOLD(cls, value) (((cls) & 0xBF) | ((value) ? 0x40 : 0x00))
#define RF4CE_MSO_GET_ENABLE_REQUEST_AUTO_VALIDATION(cls) (((cls) >> 7) & 0x1)
#define RF4CE_MSO_SET_ENABLE_REQUEST_AUTO_VALIDATION(cls, value) (((cls) & 0x7F) | ((value) ? 0x80 : 0x00))

/**//**
 * \brief RF4CE MSO Bind Target side duplicate class holding.
 */
typedef enum _RF4CE_MSO_DuplicateClassHolding_t
{
    RF4CE_MSO_USE_AS_IS = 0,
    RF4CE_MSO_REMOVE_NODE_DESCRIPTOR = 1,
    RF4CE_MSO_RECLASSIFY_NODE_DESCRIPTOR = 2,
    RF4CE_MSO_ABORT_BINDING = 3
} RF4CE_MSO_DuplicateClassHolding_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO Bind confirmation structure.
 */
typedef struct _RF4CE_MSO_BindConfParams_t
{
    uint8_t status;     /*!< One of the RF4CE_MSO_BindStatus_t values */
    uint8_t pairingRef; /*!< The pairing reference value on the successful binding */
} RF4CE_MSO_BindConfParams_t;

/**//**
 * \brief RF4CE MSO Bind request declaration.
 */
typedef struct _RF4CE_MSO_BindReqDescr_t RF4CE_MSO_BindReqDescr_t;

/**//**
 * \brief RF4CE MSO Bind callback.
 */
typedef void (*RF4CE_MSO_BindCallback_t)(RF4CE_MSO_BindReqDescr_t *req, RF4CE_MSO_BindConfParams_t *conf);

/**//**
 * \brief RF4CE MSO Bind request.
 */
struct _RF4CE_MSO_BindReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service; /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_MSO_BindCallback_t callback;  /*!< Callback on request completion. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts asynchronous Binding Procedure.

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindReq(RF4CE_MSO_BindReqDescr_t *request);

#endif /* _RF4CE_MSO_BIND_H */