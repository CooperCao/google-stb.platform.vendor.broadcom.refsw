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
 *      This is the header file for the RF4CE Network Layer component pair/unpair handlers.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_PAIR_H
#define _RF4CE_NWK_PAIR_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKDiscovery.h"
#include "bbSysTaskScheduler.h"
#include "bbRF4CENWKRequestService.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK NLME-PAIR.request parameters type.
 * \ingroup RF4CE_NWK_PairReq
 */
typedef struct _RF4CE_NWK_PairReqParams_t
{
    uint8_t logicalChannel;         /*!< The logical channel of the device with which to pair. */
    uint16_t dstPanId;              /*!< The PAN identifier of the device with which to pair. */
    uint64_t dstIeeeAddr;           /*!< The IEEE address of the device with which to pair. */
    RF4CE_NWK_ApplicationCapabilities_t orgAppCapabilities; /*!< The application capabilities of the node issuing this
                                                                 request. */
    uint8_t keyExTransferCount;     /*!< The number of transfers the target should use to exchange the link key with the
                                         pairing originator. */
    SYS_DataPointer_t payload;      /*!< The list of device types supported by the node issuing this request. */
                                    /*!< The list of profile identifiers supported by the node issuing this request. */
} RF4CE_NWK_PairReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR.confirm parameters type.
 * \ingroup RF4CE_NWK_PairConf
 */
typedef struct _RF4CE_NWK_PairConfParams_t
{
    uint8_t status;                     /*!< The status of the pair attempt. */
    uint8_t pairingRef;                 /*!< The pairing table reference for this pairing link. If the Status parameter
                                             is not equal to SUCCESS, this value will be equal to 0xff. */
    uint16_t recVendorId;               /*!< The vendor identifier of the originator of the pair response. */
    uint8_t recVendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< The vendor string of the originator of the pair
                                                                  response. */
    RF4CE_NWK_ApplicationCapabilities_t recAppCapabilities;  /*!< The application capabilities of the originator of the
                                                                  pair response. */
    SYS_DataPointer_t payload;           /*!< User string. Variable part of the entry */
                                         /*!< List of the supported devices. Variable part of the entry */
                                         /*!< List of the supported profiles. Variable part of the entry */
} RF4CE_NWK_PairConfParams_t;

/**//**
 * \brief RF4CE NWK pair request declaration.
 * \ingroup RF4CE_NWK_PairReq
 */
typedef struct _RF4CE_NWK_PairReqDescr_t RF4CE_NWK_PairReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR confirmation function type.
 * \ingroup RF4CE_NWK_PairConf
 */
typedef void (*RF4CE_NWK_PairConfCallback_t)(RF4CE_NWK_PairReqDescr_t *req, RF4CE_NWK_PairConfParams_t *conf);

/**//**
 * \brief RF4CE NWK pair request
 * \ingroup RF4CE_NWK_PairReq
 */
typedef struct _RF4CE_NWK_PairReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;     /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_PairReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_PairConfCallback_t callback;  /*!< Callback to inform sender on the result. */
} RF4CE_NWK_PairReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR.indication parameters type.
 * \ingroup RF4CE_NWK_PairInd
 */
typedef struct _RF4CE_NWK_PairIndParams_t
{
    uint8_t status;                     /*!< The status of the provisional pairing. */
    uint16_t srcPanId;                  /*!< The PAN identifier of the device requesting the pair. */
    uint64_t srcIeeeAddr;               /*!< The IEEE address of the device requesting the pair. */
    RF4CE_NWK_NodeCapabilities_t orgNodeCapabilities; /*!< The capabilities of the originator of the pair request. */
    uint16_t orgVendorId;               /*!< The vendor identifier of the originator of the pair request. */
    uint8_t orgVendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< The vendor string of the originator of the pair
                                                                  request. */
    RF4CE_NWK_ApplicationCapabilities_t orgAppCapabilities;  /*!< The application capabilities of the originator of the
                                                                  pair response. */
    uint8_t keyExTransferCount;         /*!< The number of transfers being requested to exchange the link key with the
                                             pairing originator. */
    uint8_t provPairingRef;             /*!< The pairing reference that will be used if this pairing request is
                                             successful or 0xff if there is no further capacity in the pairing table. */
    SYS_DataPointer_t payload;          /*!< The user defined identification string of the originator of the pair
                                             response. */
                                        /*!< The list of device types supported by the originator of the pair
                                             response. */
                                        /*!< The list of profile identifiers supported by the originator of the pair
                                             response. */
} RF4CE_NWK_PairIndParams_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR.response parameters type.
 * \ingroup RF4CE_NWK_PairResp
 */
typedef struct _RF4CE_NWK_PairRespParams_t
{
    uint8_t status;                     /*!< The status of the pairing request. */
    uint16_t dstPanId;                  /*!< The PAN identifier of the device requesting the pair. */
    uint64_t dstIeeeAddr;               /*!< The IEEE address of the device requesting the pair. */
    RF4CE_NWK_ApplicationCapabilities_t recAppCapabilities; /*!< The application capabilities of the node issuing this
                                                                 primitive. */
    uint8_t provPairingRef;             /*!< The reference to the provisional pairing entry if the pair was accepted or
                                             0xff otherwise. */
    SYS_DataPointer_t payload;          /*!< The list of device types supported by the node issuing this primitive. */
                                        /*!< The list of profile identifiers supported by the node issuing this
                                             primitive. */
} RF4CE_NWK_PairRespParams_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR.response descriptor type.
 * \ingroup RF4CE_NWK_PairResp
 */
typedef struct _RF4CE_NWK_PairRespDescr_t
{
    RF4CE_NWK_PairRespParams_t params; /*!< Response data filled by the sender. */
} RF4CE_NWK_PairRespDescr_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.request parameters type.
 * \ingroup RF4CE_NWK_UnpairReq
 */
typedef struct _RF4CE_NWK_UnpairReqParams_t
{
    uint8_t pairingRef;         /*!< The reference into the local pairing table of the entry that is to be removed. */
} RF4CE_NWK_UnpairReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.confirm parameters type.
 * \ingroup RF4CE_NWK_UnpairConf
 */
typedef struct _RF4CE_NWK_UnpairConfParams_t
{
    uint8_t status;             /*!< The status of the unpair attempt. */
    uint8_t pairingRef;         /*!< The pairing table reference for this pairing link. */
} RF4CE_NWK_UnpairConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR request type declaration
 * \ingroup RF4CE_NWK_UnpairReq
 */
typedef struct _RF4CE_NWK_UnpairReqDescr_t RF4CE_NWK_UnpairReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR confirmation function type.
 * \ingroup RF4CE_NWK_UnpairConf
 */
typedef void (*RF4CE_NWK_UnpairConfCallback_t)(RF4CE_NWK_UnpairReqDescr_t *req, RF4CE_NWK_UnpairConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-UNPAIR request type
 * \ingroup RF4CE_NWK_UnpairReq
 */
typedef struct _RF4CE_NWK_UnpairReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;       /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_UnpairReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_UnpairConfCallback_t callback;  /*!< Callback to inform sender on the result. */
} RF4CE_NWK_UnpairReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.indication parameters type.
 * \ingroup RF4CE_NWK_UnpairInd
 */
typedef RF4CE_NWK_UnpairReqParams_t RF4CE_NWK_UnpairIndParams_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.response parameters type.
 * \ingroup RF4CE_NWK_UnpairResp
 */
typedef RF4CE_NWK_UnpairReqParams_t RF4CE_NWK_UnpairRespParams_t;

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.response descriptor type.
 * \ingroup RF4CE_NWK_UnpairResp
 */
typedef struct _RF4CE_NWK_UnpairRespDescr_t
{
    RF4CE_NWK_UnpairRespParams_t params;    /*!< Request data filled by the sender. */
} RF4CE_NWK_UnpairRespDescr_t;

/**//**
 * \brief RF4CE NWK NLME-PAIR.indication callback function type.
 * \ingroup RF4CE_NWK_PairInd
 */
typedef void (RF4CE_NWK_PairIndCallback_t)(RF4CE_NWK_PairIndParams_t *indication);

/**//**
 * \brief RF4CE NWK NLME-UNPAIR.indication callback function type.
 * \ingroup RF4CE_NWK_UnpairInd
 */
typedef void (RF4CE_NWK_UnpairIndCallback_t)(RF4CE_NWK_UnpairIndParams_t *indication);

/**//**
 * \brief RF4CE NWK Key Seed start parameters type.
 * \ingroup RF4CE_NWK_KeySeedStartInd
 * \note Used for NWK certification tests only
 */
typedef struct _RF4CE_NWK_KeySeedStartIndParams_t
{
    uint8_t pairingRef;              /*!< The pairing table reference for this pairing link. */
} RF4CE_NWK_KeySeedStartIndParams_t;

#ifdef ENABLE_GU_KEY_SEED_IND

/**//**
 * \brief RF4CE NWK Key Seed start indication function type.
 * \ingroup RF4CE_NWK_KeySeedStartInd
 */
typedef void (RF4CE_NWK_KeySeedStartIndCallback_t)(RF4CE_NWK_KeySeedStartIndParams_t *indication);

/************************************************************************************//**
 \brief RF4CE NWK Key Seed Start indication prototype. For more information see RF4CE_NWK_KeySeedStartIndCallback_t type.
 \ingroup RF4CE_NWK_Functions
 ****************************************************************************************/
extern RF4CE_NWK_KeySeedStartIndCallback_t RF4CE_NWK_KeySeedStartInd;

#endif /* ENABLE_GU_KEY_SEED_IND */

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to pair with the target.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairReq(RF4CE_NWK_PairReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to send response to the subsequent pair request.
 \ingroup RF4CE_NWK_Functions

 \param[in] response - pointer to the structure that contains a pointer to the response structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairResp(RF4CE_NWK_PairRespDescr_t *response);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to unpair with the target.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UnpairReq(RF4CE_NWK_UnpairReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to silently unpair with the target.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UnpairSilentlyReq(RF4CE_NWK_UnpairReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to send response to the subsequent unpair request.
 \ingroup RF4CE_NWK_Functions

 \param[in] response - pointer to the structure that contains a pointer to the response structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UnpairResp(RF4CE_NWK_UnpairRespDescr_t *response);

/************************************************************************************//**
 \brief RF4CE NWK NLME-PAIR.indication prototype. For more information see RF4CE_NWK_PairIndCallback_t type.
 \ingroup RF4CE_NWK_Functions
 ****************************************************************************************/
extern RF4CE_NWK_PairIndCallback_t RF4CE_NWK_PairInd;

/************************************************************************************//**
 \brief RF4CE NWK NLME-UNPAIR.indication prototype. For more information see RF4CE_NWK_UnpairIndCallback_t type.
 \ingroup RF4CE_NWK_Functions
 ****************************************************************************************/
extern RF4CE_NWK_UnpairIndCallback_t RF4CE_NWK_UnpairInd;

#endif /* _RF4CE_NWK_PAIR_H */

/* eof bbRF4CENWKPair.h */