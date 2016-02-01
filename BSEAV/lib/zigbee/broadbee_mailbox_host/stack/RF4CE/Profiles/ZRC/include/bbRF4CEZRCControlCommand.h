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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCControlCommand.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC profile Control Command Handler handler.
 *
 * $Revision: 3272 $
 * $Date: 2014-08-15 09:13:13Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_CONTROL_COMMAND_H
#define _RF4CE_ZRC_CONTROL_COMMAND_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief RF4CE_ZRC2_ControlCommandReqParams_t::payload maximum length
 */
#define RF4CE_ZRC2_CONTROLCOMMAND_PAYLOAD_MAX           32

/**//**
 * \brief RF4CE ZRC 2.0 Actions.
 */
#define RF4CE_ZRC2_ACTION_RESERVED   0x00
#define RF4CE_ZRC2_ACTION_START      0x01
#define RF4CE_ZRC2_ACTION_REPEAT     0x02
#define RF4CE_ZRC2_ACTION_ATOMIC     0x03
#define RF4CE_ZRC2_ACTION_GET_TYPE(action) ((action) & 0x03)
#define RF4CE_ZRC2_ACTION_SET_TYPE(action, value) (((action) & 0xFC) | ((value) & 0x03))
#define RF4CE_ZRC2_ACTION_IGNORE_TYPE(action) ((action) & ~0x03)
#define RF4CE_ZRC2_ACTION_GET_GUI_MODIFIER(action) (((action) >> 4) & 0x01)
#define RF4CE_ZRC2_ACTION_SET_GUI_MODIFIER(action, value) (((action) & 0xEF) | ((value) ? 0x10 : 0))
#define RF4CE_ZRC2_ACTION_GET_ALT_MODIFIER(action) (((action) >> 5) & 0x01)
#define RF4CE_ZRC2_ACTION_SET_ALT_MODIFIER(action, value) (((action) & 0xDF) | ((value) ? 0x20 : 0))
#define RF4CE_ZRC2_ACTION_GET_SHIFT_MODIFIER(action) (((action) >> 6) & 0x01)
#define RF4CE_ZRC2_ACTION_SET_SHIFT_MODIFIER(action, value) (((action) & 0xBF) | ((value) ? 0x40 : 0))
#define RF4CE_ZRC2_ACTION_GET_CTRL_MODIFIER(action) (((action) >> 7) & 0x01)
#define RF4CE_ZRC2_ACTION_SET_CTRL_MODIFIER(action, value) (((action) & 0x7F) | ((value) ? 0x80 : 0))
#define RF4CE_ZRC2_ACTION_IS_VENDOR_FIELD_INCLUDED(actionBank) ((actionBank & 0xE0) == 0xE0)

/**//**
 * \brief RF4CE ZRC 2.0 Control Command Status.
 */
typedef RF4CE_NLDE_DATA_Status_t RF4CE_ZRC2_ControlCommandConfStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 header of action structure (fits into 32 bits as packed)
 */
#define RF4CE_ZRC2_ACTION_HEADER                                                                                    \
    uint8_t actionControl;              /*!< Use RF4CE_ZRC2_ACTION_GET_xxx/RF4CE_ZRC2_ACTION_SET_xxx */             \
                                        /*!< to specify RF4CE_ZRC2_ACTION_ATOMIC */                                 \
    uint8_t payloadLength;              /*!< Action payload length. If this field is 0 then */                      \
                                        /*!< next record starts right after this record */                          \
    uint8_t bank;                       /*!< Action bank. */                                                        \
    uint8_t code;                       /*!< Action code. */

/**//**
 * \brief RF4CE ZRC 2.0 single action structure without Vendor ID support.
 */
typedef struct _RF4CE_ZRC2_Action_t
{
    RF4CE_ZRC2_ACTION_HEADER
                                        /*!< Action payload follow right after the field above (as packed).
                                             Actual length is set by the payloadLength field */
} RF4CE_ZRC2_Action_t;

/**//**
 * \brief RF4CE ZRC 2.0 single action structure with Vendor ID support.
 */
typedef struct _RF4CE_ZRC2_ActionVendor_t
{
    RF4CE_ZRC2_ACTION_HEADER
    uint16_t vendorId;                  /*!< Action Vendor ID. */
                                        /*!< Action payload follow right after the field above (as packed).
                                             Actual length is set by the payloadLength field */
} RF4CE_ZRC2_ActionVendor_t;

/**//**
 * \brief RF4CE ZRC 2.0 control command request descriptor parameters.
 */
typedef struct _RF4CE_ZRC2_ControlCommandReqParams_t
{
    uint8_t pairingRef;                 /*!< Pairing reference. */
    SYS_DataPointer_t payload;          /*!< Supplied payload consisting of one or more RF4CE_ZRC2_Action_t and/or
                                             RF4CE_ZRC2_ActionVendor_t structures. */
} RF4CE_ZRC2_ControlCommandReqParams_t;
typedef RF4CE_ZRC2_ControlCommandReqParams_t RF4CE_ZRC2_ControlCommandIndParams_t;
/**//**
 * \brief RF4CE ZRC 2.0 control command confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_ControlCommandConfParams_t
{
    RF4CE_ZRC2_ControlCommandConfStatus_t status; /*!< The status of the operation. */
} RF4CE_ZRC2_ControlCommandConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 control command request descriptor declaration.
 */
typedef struct _RF4CE_ZRC2_ControlCommandReqDescr_t RF4CE_ZRC2_ControlCommandReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 control command request callback.
 */
typedef void (*RF4CE_ZRC2_ControlCommandCallback_t)(RF4CE_ZRC2_ControlCommandReqDescr_t *req, RF4CE_ZRC2_ControlCommandConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 control command request descriptor.
 */
struct _RF4CE_ZRC2_ControlCommandReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;             /*!< Service field. */
#endif /* _HOST_ */
    RF4CE_ZRC2_ControlCommandReqParams_t params;    /*!< Request parameters. */
    RF4CE_ZRC2_ControlCommandCallback_t callback;   /*!< Request confirmation callback. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts sending ZRC 2.0 Control Command Pressed action.

 \param[in] request - pointer to the ZRC 2.0 Control Command request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_ControlCommandPressedReq(RF4CE_ZRC2_ControlCommandReqDescr_t *request);

/************************************************************************************//**
 \brief Starts sending ZRC 2.0 Control Command Released action.

 \param[in] request - pointer to the ZRC 2.0 Control Command request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_ControlCommandReleasedReq(RF4CE_ZRC2_ControlCommandReqDescr_t *request);

#endif /* _RF4CE_ZRC_CONTROL_COMMAND_H */
