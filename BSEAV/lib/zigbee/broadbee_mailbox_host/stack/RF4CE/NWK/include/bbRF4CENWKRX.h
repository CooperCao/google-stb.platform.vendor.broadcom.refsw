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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKRX.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer Receiver handler.
 *
 * $Revision: 2659 $
 * $Date: 2014-06-17 09:43:22Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_RX_H
#define _RF4CE_NWK_RX_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbSysQueue.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK NLME-RX-ENABLE.request parameters type.
 */
typedef struct _RF4CE_NWK_RXEnableReqParams_t
{
    uint32_t rxOnDuration;  /*!< The number of MAC symbols for which the receiver is to be enabled. To activate power
                                 saving mode, this value should correspond to the value of nwkActivePeriod and
                                 nwkActivePeriod should not be equal to 0x000000.
                                 If this parameter is equal to 0x000000, the receiver is to be disabled until further
                                 notice, allowing the node to enter dormant power save mode.
                                 If this parameter is equal to 0xffffff, the receiver is to be enabled until further
                                 notice, allowing the node to come out of power save mode. */
} RF4CE_NWK_RXEnableReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-RX-ENABLE.confirm parameters type.
 */
typedef struct _RF4CE_NWK_RXEnableConfParams_t
{
    uint8_t status;    /*!< The status of the Receiver manipulation attempt. */
} RF4CE_NWK_RXEnableConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-RX-ENABLE.request type declaration.
 */
typedef struct _RF4CE_NWK_RXEnableReqDescr_t RF4CE_NWK_RXEnableReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-RX-ENABLE confirmation function type.
 */
typedef void (*RF4CE_NWK_RXEnableConfCallback_t)(RF4CE_NWK_RXEnableReqDescr_t *req, RF4CE_NWK_RXEnableConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-RX-ENABLE.request type.
 */
struct _RF4CE_NWK_RXEnableReqDescr_t
{
#ifndef _HOST_
    struct
    {
        SYS_QueueElement_t queueElement;
        Bool8_t isPrivate;
    } service;
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_NWK_RXEnableReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_RXEnableConfCallback_t callback;  /*!< Callback to inform sender on the result. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to enable/disable the receiver.

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing
 ****************************************************************************************/
void RF4CE_NWK_RXEnableReq(RF4CE_NWK_RXEnableReqDescr_t *request);

#endif /* _RF4CE_NWK_RX_H */