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
 *      This is the header file for the MSO RF4CE Profile
 *      User control commands support.
 *
*******************************************************************************/

#ifndef _RF4CE_MSO_USER_CONTROL_H
#define _RF4CE_MSO_USER_CONTROL_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysQueue.h"
#include "bbRF4CEMSOConstants.h"
#include "bbSysPayload.h"
#include "bbRF4CENWKRequestService.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO User Control request types.
 */
typedef enum _RF4CE_MSO_UserControlReqType_t
{
    RF4CE_MSO_USER_CONTROL_PRESSED = MSO_CONTROL_PRESSED,
    RF4CE_MSO_USER_CONTROL_REPEATED = MSO_CONTROL_REPEATED,
    RF4CE_MSO_USER_CONTROL_RELEASED = MSO_CONTROL_RELEASED
} RF4CE_MSO_UserControlReqType_t;

/**//**
 * \brief RF4CE MSO User Control status codes.
 */
typedef enum _RF4CE_MSO_UserControlStatusCodes_t
{
    RF4CE_MSO_USER_CONTROL_SUCCESS = 0,
    RF4CE_MSO_USER_CONTROL_NO_MEMORY,
    RF4CE_MSO_USER_CONTROL_ERROR_SEND,
    RF4CE_MSO_USER_CONTROL_NO_MORE_DATA,
    RF4CE_MSO_USER_CONTROL_INVALID_PARAMETER
} RF4CE_MSO_UserControlStatusCodes_t;

/**//**
 * \brief RF4CE MSO User Control bitmask codes.
 */
#define MSO_IS_IN_PRESSED_STATE(v) (0 != ((v) & 0x8000))
#define MSO_SET_IN_PRESSED_STATE(v) v |= 0x8000
#define MSO_CLEAR_IN_PRESSED_STATE(v) v &= ~0x8000
#define MSO_IS_IN_TASK_REQURED_STATE(v) (0 != ((v) & 0x4000))
#define MSO_SET_IN_TASK_REQURED_STATE(v) v |= 0x4000
#define MSO_CLEAR_IN_TASK_REQURED_STATE(v) v &= ~0x4000
#define MSO_IS_IN_NEED_CALLBACK_STATE(v) (0 != ((v) & 0x2000))
#define MSO_SET_NEED_CALLBACK_STATE(v) v |= 0x2000
#define MSO_CLEAR_NEED_CALLBACK_STATE(v) v &= ~0x2000

#define MSO_IS_PRESSED_STATE_USED(v) (0 != ((v) & (0x01 << 6)))
#define MSO_SET_PRESSED_STATE_USED(v) v |= (0x01 << 6)
#define MSO_CLEAR_PRESSED_STATE_USED(v) v &= ~(0x01 << 6)
#define MSO_IS_REPEATED_STATE_USED(v) (0 != ((v) & (0x02 << 6)))
#define MSO_SET_REPEATED_STATE_USED(v) v |= (0x02 << 6)
#define MSO_CLEAR_REPEATED_STATE_USED(v) v &= ~(0x02 << 6)
#define MSO_IS_RELEASED_STATE_USED(v) (0 != ((v) & (0x04 << 6)))
#define MSO_SET_RELEASED_STATE_USED(v) v |= (0x04 << 6)
#define MSO_CLEAR_RELEASED_STATE_USED(v) v &= ~(0x04 << 6)
#define MSO_SET_STATES_USED(v, pressed, repeated, released) v = (v & ~(0x07 << 6)) | \
    ((pressed) ? (0x01 << 6) : 0) | \
    ((repeated) ? (0x02 << 6) : 0) | \
    ((released) ? (0x04 << 6) : 0)

#define MSO_IS_PRESSED_STATE_KEEP_PRESSING(v) (0 != ((v) & 0x01))
#define MSO_SET_PRESSED_STATE_KEEP_PRESSING(v) v |= 0x01
#define MSO_CLEAR_PRESSED_STATE_KEEP_PRESSING(v) v &= ~0x01
#define MSO_IS_REPEATED_STATE_KEEP_PRESSING(v) (0 != ((v) & 0x02))
#define MSO_SET_REPEATED_STATE_KEEP_PRESSING(v) v |= 0x02
#define MSO_CLEAR_REPEATED_STATE_KEEP_PRESSING(v) v &= ~0x02
#define MSO_IS_RELEASED_STATE_KEEP_PRESSING(v) (0 != ((v) & 0x04))
#define MSO_SET_RELEASED_STATE_KEEP_PRESSING(v) v |= 0x04
#define MSO_CLEAR_RELEASED_STATE_KEEP_PRESSING(v) v &= ~0x04
#define MSO_SET_STATES_KEEP_PRESSING(v, pressed, repeated, released) v = (v & ~0x07) | \
    ((pressed) ? 0x01 : 0) | \
    ((repeated) ? 0x02 : 0) | \
    ((released) ? 0x04 : 0)

#define MSO_IS_PRESSED_STATE_SHORT_RETRY(v) (0 != ((v) & (0x01 << 3)))
#define MSO_SET_PRESSED_STATE_SHORT_RETRY(v) v |= (0x01 << 3)
#define MSO_CLEAR_PRESSED_STATE_SHORT_RETRY(v) v &= ~(0x01 << 3)
#define MSO_IS_REPEATED_STATE_SHORT_RETRY(v) (0 != ((v) & (0x02 << 3)))
#define MSO_SET_REPEATED_STATE_SHORT_RETRY(v) v |= (0x02 << 3)
#define MSO_CLEAR_REPEATED_STATE_SHORT_RETRY(v) v &= ~(0x02 << 3)
#define MSO_IS_RELEASED_STATE_SHORT_RETRY(v) (0 != ((v) & (0x04 << 3)))
#define MSO_SET_RELEASED_STATE_SHORT_RETRY(v) v |= (0x04 << 3)
#define MSO_CLEAR_RELEASED_STATE_SHORT_RETRY(v) v &= ~(0x04 << 3)
#define MSO_HAS_STATE_SHORT_RETRY(v) (0 != ((v) & (0x07 << 3)))
#define MSO_SET_STATES_SHORT_RETRY(v, pressed, repeated, released) v = (v & ~(0x07 << 3)) | \
    ((pressed) ? (0x01 << 3) : 0) | \
    ((repeated) ? (0x02 << 3) : 0) | \
    ((released) ? (0x04 << 3) : 0)

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO User Control request parameters.
 * \ingroup RF4CE_MSO_UserControlReq
 */
typedef struct _RF4CE_MSO_UserControlReqParams_t
{
    uint8_t pairingRef;        /*!< The pairing reference value on the successful binding */
    uint8_t keyCode;           /*!< Key code */
    Bool8_t isKeyMappable;     /*!< Is key mappable? */
} RF4CE_MSO_UserControlReqParams_t;

/**//**
 * \brief RF4CE MSO User Control confirmation parameters.
 * \ingroup RF4CE_MSO_UserControlConf
 */
typedef struct _RF4CE_MSO_UserControlConfParams_t
{
    uint8_t status;            /*!< One of the RF4CE_MSO_UserControlStatusCodes_t values */
} RF4CE_MSO_UserControlConfParams_t;

/**//**
 * \brief RF4CE MSO User Control request declaration.
 * \ingroup RF4CE_MSO_UserControlReq
 */
typedef struct _RF4CE_MSO_UserControlReqDescr_t RF4CE_MSO_UserControlReqDescr_t;

/**//**
 * \brief RF4CE MSO User Control request callback.
 * \ingroup RF4CE_MSO_UserControlConf
 */
typedef void (*RF4CE_MSO_UserControlCallback_t)(RF4CE_MSO_UserControlReqDescr_t *req, RF4CE_MSO_UserControlConfParams_t *conf);

/**//**
 * \brief RF4CE MSO User Control request.
 * \ingroup RF4CE_MSO_UserControlReq
 */
struct _RF4CE_MSO_UserControlReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;       /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_MSO_UserControlReqParams_t params;  /*!< Parameters. */
    RF4CE_MSO_UserControlCallback_t callback; /*!< Request callback. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates MSO User Control Pressed request.
 \ingroup RF4CE_MSO_Functions

 \param[in] request - pointer to the request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UserControlPressedReq(RF4CE_MSO_UserControlReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates MSO User Control Released request.
 \ingroup RF4CE_MSO_Functions

 \param[in] request - pointer to the request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UserControlReleasedReq(RF4CE_MSO_UserControlReqDescr_t *request);

#endif /* _RF4CE_MSO_USER_CONTROL_H */

/* eof bbRF4CEMSOUserControl.h */