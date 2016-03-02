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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ProfileManager/include/bbRF4CEPMStartReset.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE profile manager initialization/reset methods.
 *
 * $Revision: 2869 $
 * $Date: 2014-07-10 08:15:06Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_PM_START_RESET_H
#define _RF4CE_PM_START_RESET_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKStartReset.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Profiles NWK constants.
 */
#define RF4CE_PM_NWK_DISCOVERY_LQI_THRESHOLD    0x01

/**//**
 * \brief START/RESET result type declaration.
 */
typedef enum _RF4CE_StartReset_Status_t
{
    RF4CE_START_RESET_OK = 0,         /*!< Success */
    RF4CE_START_RESET_ERROR_NETWORK,  /*!< Error happened at the Network level */
    RF4CE_START_RESET_ERROR_PROFILE,  /*!< Error happened at the Profile level */
    RF4CE_START_RESET_UNAVAILABLE,    /*!< Error happened right at the startup level */
    RF4CE_START_RESET_ALREADY_PROCESSING /*!< The request is already being processed */
} RF4CE_StartReset_Status_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief NLME-START confirm primitive's parameters structure declaration.
 */
typedef struct _RF4CE_StartResetConfParams_t
{
    uint8_t status;     /*!< The status of the START/RESET request. One of the RF4CE_StartReset_Status_t values.  */
} RF4CE_StartResetConfParams_t;

/**//**
 * \brief START request structure declaration.
 */
typedef struct _RF4CE_StartReqDescr_t RF4CE_StartReqDescr_t;

/**//**
 * \brief START confirmation function type.
 */
typedef void (*RF4CE_StartConfCallback_t)(RF4CE_StartReqDescr_t *req, RF4CE_StartResetConfParams_t *conf);

/**//**
 * \brief START request structure.
 */
typedef struct _RF4CE_StartReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service; /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_StartConfCallback_t callback; /*!< Callback for confirmation. */
} RF4CE_StartReqDescr_t;

/**//**
 * \brief RESET request structure declaration.
 */
typedef struct _RF4CE_ResetReqDescr_t RF4CE_ResetReqDescr_t;

/**//**
 * \brief RESET confirmation function type.
 */
typedef void (*RF4CE_ResetConfCallback_t)(RF4CE_ResetReqDescr_t *req, RF4CE_StartResetConfParams_t *conf);

/**//**
 * \brief RESET request parameters structure.
 */
typedef struct _RF4CE_ResetReqParams_t
{
    Bool8_t setDefaultPIBNIB;  /*!< Set default values for NIB and PIB. */
} RF4CE_ResetReqParams_t;

/**//**
 * \brief RESET request structure.
 */
struct _RF4CE_ResetReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service; /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ResetReqParams_t params;      /*!< Request parameters. */
    RF4CE_ResetConfCallback_t callback; /*!< Callback for confirmation. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to start the profile layer.

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_StartReq(RF4CE_StartReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to reset the profile layer.

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ResetReq(RF4CE_ResetReqDescr_t *request);

#ifdef __cplusplus
}
#endif

#endif /* _RF4CE_PM_START_RESET_H */