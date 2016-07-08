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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/bbRF4CEMSOExternalIndications.h $
 *
 * DESCRIPTION:
 *   This is the header file for the MSO RF4CE Profile
 *   External Indications definition.
 *
 * $Revision: 1615 $
 * $Date: 2014-02-26 13:29:56Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_EXTERNAL_INDICATIONS_H
#define _RF4CE_MSO_EXTERNAL_INDICATIONS_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEMSOValidation.h"
#include "bbRF4CEPMExternalIndications.h"
#include "bbRF4CEMSOUserControl.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO User Control request parameters.
 */
typedef struct _RF4CE_MSO_UserControlIndParams_t
{
    uint8_t pairingRef;
    RF4CE_MSO_UserControlReqType_t indType;
    SYS_DataPointer_t payload;
} RF4CE_MSO_UserControlIndParams_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief MSO Start Validation indication to HOST. Controller only.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_StartValidationInd(RF4CE_PairingReferenceIndParams_t *indication);

/************************************************************************************//**
 \brief MSO Check Validation indication to HOST.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_CheckValidationInd(RF4CE_MSO_CheckValidationIndParams_t *indication);

/************************************************************************************//**
 \brief MSO User Control command indication to HOST.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UserControlInd(RF4CE_MSO_UserControlIndParams_t *indication);

#endif /* _RF4CE_MSO_EXTERNAL_INDICATIONS_H */