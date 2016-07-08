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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ProfileManager/include/bbRF4CEPMExternalIndications.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Profiles Manager Layer component:
 *   the list of indications that MUST be implemented on the HOST.
 *
 * $Revision: 2869 $
 * $Date: 2014-07-10 08:15:06Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_PM_EXTERNAL_INDICATIONS_H
#define _RF4CE_PM_EXTERNAL_INDICATIONS_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"
#include "bbRF4CENWK.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE Pairing Reference Indication statuses enumeration.
 */
#ifdef RF4CE_NO_SHORT_ENUM
typedef uint8_t RF4CE_PairingIndStatus_t;
#else
typedef enum _RF4CE_PairingIndStatus_t
{
    RF4CE_PAIRING_REFERENCE_IND_STATUS_CREATED,
    RF4CE_PAIRING_REFERENCE_IND_STATUS_UPDATED
} RF4CE_PairingIndStatus_t;
#endif

/**//**
 * \brief RF4CE Pairing Indication parameters.
 */
typedef struct _RF4CE_PairingIndParams_t
{
    RF4CE_PairingIndStatus_t status;    /*!< The pairing status */
    uint8_t pairingRef;                          /*!< The pairing reference */
} RF4CE_PairingIndParams_t;

/**//**
 * \brief RF4CE Pairing Reference Indication parameters.
 */
typedef struct _RF4CE_PairingReferenceIndParams_t
{
    uint8_t pairingRef; /*!< The pairing reference */
} RF4CE_PairingReferenceIndParams_t;

/**//**
 * \brief RF4CE Pairing Reference Indication with profile ID parameters.
 */
typedef struct _RF4CE_PairingReferenceProfileIdIndParams_t
{
    uint8_t pairingRef; /*!< The pairing reference */
    uint8_t profileId;  /*!< The profile ID */
} RF4CE_PairingReferenceProfileIdIndParams_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Indication to the HOST on Counter Expired error.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_CounterExpiredInd(RF4CE_PairingReferenceIndParams_t *indication);

/************************************************************************************//**
 \brief Indication to the HOST on Unpair indication data.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_UnpairInd(RF4CE_PairingReferenceIndParams_t *indication);

#ifdef RF4CE_TARGET

/************************************************************************************//**
 \brief Indication to the HOST on Pair indication data.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_PairInd(RF4CE_PairingIndParams_t *indication);

/************************************************************************************//**
 \brief Indication to the HOST on Pair indication data.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_PairInd(RF4CE_PairingIndParams_t *indication, uint8_t protocolId);

#endif /* RF4CE_TARGET */

#endif /* _RF4CE_PM_EXTERNAL_INDICATIONS_H */
