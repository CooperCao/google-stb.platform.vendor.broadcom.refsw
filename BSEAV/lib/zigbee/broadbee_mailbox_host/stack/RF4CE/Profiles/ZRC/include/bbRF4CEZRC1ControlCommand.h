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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRC1ControlCommand.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC 1.1 profile Control Command Handler handler.
 *
 * $Revision: 3368 $
 * $Date: 2014-08-21 16:02:35Z $
 *
 ****************************************************************************************/

#ifndef BBRF4CEZRC1CONTROLCOMMAND_H
#define BBRF4CEZRC1CONTROLCOMMAND_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC Control Command Status.
 */
typedef enum _RF4CE_ZRC_ControlCommandConfStatus_t
{
    RF4CE_ZRC_CC_SUCCESS = 0,
    RF4CE_ZRC_CC_INVALID_PARAMETER,
    RF4CE_ZRC_CC_INVALID_REQUEST,
    RF4CE_ZRC_CC_ERROR_SENDING
} RF4CE_ZRC_ControlCommandConfStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 control command request descriptor parameters.
 */
typedef struct _RF4CE_ZRC1_ControlCommandReqParams_t
{
    uint8_t pairingRef;                 /*!< Pairing reference. */
    uint8_t commandCode;                /*!< The command code. */
    SYS_DataPointer_t payload;          /*!< Possible additional data. */
} RF4CE_ZRC1_ControlCommandReqParams_t;

/**//**
 * \brief RF4CE ZRC control command confirmation parameters.
 */
typedef struct _RF4CE_ZRC_ControlCommandConfParams_t
{
    uint8_t status; /*!< The status of the operation. */
} RF4CE_ZRC_ControlCommandConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 control command request descriptor declaration.
 */
typedef struct _RF4CE_ZRC1_ControlCommandReqDescr_t RF4CE_ZRC1_ControlCommandReqDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 control command request callback.
 */
typedef void (*RF4CE_ZRC1_ControlCommandCallback_t)(RF4CE_ZRC1_ControlCommandReqDescr_t *req, RF4CE_ZRC_ControlCommandConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 control command request descriptor.
 */
struct _RF4CE_ZRC1_ControlCommandReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;           /*!< Service field. */
#endif /* _HOST_ */
    RF4CE_ZRC1_ControlCommandReqParams_t params;  /*!< Request parameters. */
    RF4CE_ZRC1_ControlCommandCallback_t callback; /*!< Request confirmation callback. */
};

/**//**
 * \brief RF4CE ZRC 1.1 frame format.
 */
typedef struct PACKED _RF4CE_ZRC1_ControlCommandFrame_t
{
    uint8_t frameControl;
    uint8_t commandCode;
} RF4CE_ZRC1_ControlCommandFrame_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts sending ZRC 1.1 Control Command.

 \param[in] request - pointer to the ZRC 1.1 Control Command request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_ControlCommandPressedReq(RF4CE_ZRC1_ControlCommandReqDescr_t *request);

/************************************************************************************//**
 \brief Ends sending ZRC 1.1 Control Command.

 \param[in] request - pointer to the ZRC 1.1 Control Command request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_ControlCommandReleasedReq(RF4CE_ZRC1_ControlCommandReqDescr_t *request);

#endif // BBRF4CEZRC1CONTROLCOMMAND_H