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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ProfileManager/include/bbRF4CEPMPair.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE profile manager pair/unpair handlers.
 *
 * $Revision: 2116 $
 * $Date: 2014-04-08 11:21:36Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_PM_PAIR_H
#define _RF4CE_PM_PAIR_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWK.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* TYPES *******************************************************/
/**//**
 * \brief UNPAIR Request parameters structure.
 */
typedef struct _RF4CE_UnpairReqParams_t
{
    uint8_t pairingRef; /*!< The existing pairing reference */
} RF4CE_UnpairReqParams_t;

/**//**
 * \brief UNPAIR Request confirmation structure.
 */
typedef struct _RF4CE_UnpairConfParams_t
{
    Bool8_t status; /*!< The status of the operation */
} RF4CE_UnpairConfParams_t;

/**//**
 * \brief UNPAIR Request structure declaration.
 */
typedef struct _RF4CE_UnpairReqDescr_t RF4CE_UnpairReqDescr_t;

/**//**
 * \brief UNPAIR Request callback.
 */
typedef void (*RF4CE_UnpairCallback_t)(RF4CE_UnpairReqDescr_t *req, RF4CE_UnpairConfParams_t *conf);

/**//**
 * \brief UNPAIR Request structure.
 */
struct _RF4CE_UnpairReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;
#endif /* _HOST_ */
    RF4CE_UnpairReqParams_t params;  /*!< The request parameters */
    RF4CE_UnpairCallback_t callback; /*!< The request confirmation callback */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief UNPAIR function.

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_UnpairReq(RF4CE_UnpairReqDescr_t *request);

/************************************************************************************//**
    \brief UNPAIR function.
    \param[in] request - pointer to the request structure.
 ****************************************************************************************/
void RF4CE_UnpairSilentlyReq(RF4CE_UnpairReqDescr_t *request);

/************************************************************************************//**
    \brief Checks the presence of a specific Profile Id in the discovery indication.
    \param[in] indication - pointer to the discovery indication structure.
    \param[in] profileId - the searched profile id.
    \return True if the profile id was found.
 ****************************************************************************************/
bool RF4CE_CheckDiscoveryRequestForProfile(RF4CE_NWK_DiscoveryIndParams_t *indication, uint8_t profileId);

/************************************************************************************//**
    \brief Function is called after the unpairing procedure is finished.
 ****************************************************************************************/
void rf4cezrc2UnpairingProcedureFinished(void);

#ifdef __cplusplus
}
#endif

#endif /* _RF4CE_PM_PAIR_H */