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
 *      This is the header file for the RF4CE ZRC 1.1 profile Binding functionality.
 *
*******************************************************************************/

#ifndef BBRF4CEZRC1BIND_H
#define BBRF4CEZRC1BIND_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Bind result status.
 * \ingroup RF4CE_ZRC1_BindConf
 */
typedef enum _RF4CE_ZRC1_BindStatus_t
{
    RF4CE_ZRC1_BOUND = 0,
    RF4CE_ZRC1_BIND_ERROR_DISCOVERY,
    RF4CE_ZRC1_BIND_ERROR_PAIRING,
    RF4CE_ZRC1_BIND_ERROR_NO_PAIRING_ENTRY,
    RF4CE_ZRC1_BIND_ERROR_TIMEOUT,
    RF4CE_ZRC1_BIND_ERROR_NOT_SUPPORTED,
    RF4CE_ZRC1_BIND_ERROR_DISCOVERY_TIMEOUT
} RF4CE_ZRC1_BindStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Bind confirmation parameters.
 * \ingroup RF4CE_ZRC1_BindConf
 */
typedef struct _RF4CE_ZRC1_BindConfParams_t
{
    uint8_t status;     /*!< The status of binding. See RF4CE_ZRC1_BindStatus_t. */
    uint8_t pairingRef; /*!< The pairing reference on successful binding. */
} RF4CE_ZRC1_BindConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Bind request declaration.
 * \ingroup RF4CE_ZRC1_BindReq
 */
typedef struct _RF4CE_ZRC1_BindReqDescr_t RF4CE_ZRC1_BindReqDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 Bind callback.
 * \ingroup RF4CE_ZRC1_BindConf
 */
typedef void (*RF4CE_ZRC1_BindCallback_t)(RF4CE_ZRC1_BindReqDescr_t *req, RF4CE_ZRC1_BindConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 Bind request.
 * \ingroup RF4CE_ZRC1_BindReq
 */
struct _RF4CE_ZRC1_BindReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service; /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_ZRC1_BindCallback_t callback; /*!< Callback on request completion. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts Controller side Binding Procedure.
 \ingroup RF4CE_ZRC_Functions

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_ControllerBindReq(RF4CE_ZRC1_BindReqDescr_t *request);

/************************************************************************************//**
 \brief Starts Target side Binding Procedure.
 \ingroup RF4CE_ZRC_Functions

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_TargetBindReq(RF4CE_ZRC1_BindReqDescr_t *request);

#ifdef __cplusplus
}
#endif

#endif // BBRF4CEZRC1BIND_H

/* eof bbRF4CEZRC1Bind.h */