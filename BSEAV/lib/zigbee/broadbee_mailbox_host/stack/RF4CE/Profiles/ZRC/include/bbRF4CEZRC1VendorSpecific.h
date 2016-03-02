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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRC1VendorSpecific.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC 1.1 profile vendor specific frame.
 *
 * $Revision: 3368 $
 * $Date: 2014-08-21 16:02:35Z $
 *
 ****************************************************************************************/

#ifndef BBRF4CEZRC1VENDORSPECIFIC_H
#define BBRF4CEZRC1VENDORSPECIFIC_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific status.
 */
typedef RF4CE_NLDE_DATA_Status_t RF4CE_ZRC1_VendorSpecificStatus_t;

/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific request parameters.
 */
#ifdef _PHY_TEST_HOST_INTERFACE_
typedef struct _RF4CE_ZRC1_VendorSpecificIndParams_t
{
    uint8_t pairingRef;         /*!< The pairing reference. */
    uint8_t profileID;
    uint16_t vendorID;
    uint32_t nsduLength;
    uint8_t linkQuality;
    uint8_t rxFlags;
    SYS_DataPointer_t payload;  /*!< The payload to be sent. */
} RF4CE_ZRC1_VendorSpecificIndParams_t;

typedef struct _RF4CE_ZRC1_VendorSpecificReqParams_t
{
    uint8_t pairingRef;         /*!< The pairing reference. */
    uint8_t profileID;
    uint16_t vendorID;
    uint32_t nsduLength;
    uint8_t txFlags;
    uint8_t reserve;
    SYS_DataPointer_t payload;  /*!< The payload to be sent. */
} RF4CE_ZRC1_VendorSpecificReqParams_t;

#else

typedef struct _RF4CE_ZRC1_VendorSpecificReqParams_t
{
    uint8_t pairingRef;         /*!< The pairing reference. */
    uint16_t vendorID;          /*!< The Vendor ID. */
    SYS_DataPointer_t payload;  /*!< The payload to be sent. */
} RF4CE_ZRC1_VendorSpecificReqParams_t;
#endif

/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific confirmation parameters.
 */
typedef struct _RF4CE_ZRC1_VendorSpecificConfParams_t
{
	RF4CE_ZRC1_VendorSpecificStatus_t status;     /*!< The status of sending. */
} RF4CE_ZRC1_VendorSpecificConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific request declaration.
 */
typedef struct _RF4CE_ZRC1_VendorSpecificReqDescr_t RF4CE_ZRC1_VendorSpecificReqDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific callback.
 */
typedef void (*RF4CE_ZRC1_VendorSpecificCallback_t)(RF4CE_ZRC1_VendorSpecificReqDescr_t *req, RF4CE_ZRC1_VendorSpecificConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific request.
 */
struct _RF4CE_ZRC1_VendorSpecificReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;           /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ZRC1_VendorSpecificReqParams_t params;  /*!< Request params. */
    RF4CE_ZRC1_VendorSpecificCallback_t callback; /*!< Callback on request completion. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts Vendor Specific sending.

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_VendorSpecificReq(RF4CE_ZRC1_VendorSpecificReqDescr_t *request);

#endif /* BBRF4CEZRC1VENDORSPECIFIC_H */
