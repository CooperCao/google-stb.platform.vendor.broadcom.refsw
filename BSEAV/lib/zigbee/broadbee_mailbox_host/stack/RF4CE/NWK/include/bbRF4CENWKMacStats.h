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
 * FILENAME: $Workfile: $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Mac Statistics handler.
 *
 * $Revision: $
 * $Date: $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_MAC_STATS_H
#define _RF4CE_NWK_MAC_STATS_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief MAC Statistics status enumeration.
 */
typedef enum _RF4CE_MAC_STATS_Status_t
{
    RF4CE_MAC_STATS_NOT_SUPPORTED = 0,
    RF4CE_MAC_STATS_SUPPORTED     = 1
} RF4CE_MAC_STATS_Status_t;

/**//**
 * \brief MAC Statistics result enumeration.
 */
typedef enum _RF4CE_MAC_STATS_Result_t
{
    RF4CE_MAC_STATS_OUT_OF_MAC_REQUEST = 0,
    RF4CE_MAC_STATS_IN_MAC_REQUEST     = 1
} RF4CE_MAC_STATS_Result_t;

/**//**
 * \brief MAC Statistics request ID enumeration.
 */
typedef enum _RF4CE_MAC_STATS_RequestId_t
{
    RF4CE_MAC_STATS_REQ_NONE  = 0,
    RF4CE_MAC_STATS_REQ_DATA  = 1,
    RF4CE_MAC_STATS_REQ_GET   = 2,
    RF4CE_MAC_STATS_REQ_RESET = 3,
    RF4CE_MAC_STATS_REQ_SCAN  = 4,
    RF4CE_MAC_STATS_REQ_SET   = 5,
    RF4CE_MAC_STATS_REQ_START = 6
} RF4CE_MAC_STATS_RequestId_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief MAC Statistics request parameters type.
 */
typedef struct _RF4CE_NWK_MacStatsReqParams_t
{
    uint8_t dummy;                              /*!< Just dummy data needed for Mailbox. Always set to 0. */
} RF4CE_NWK_MacStatsReqParams_t;

/**//**
 * \brief MAC Statistics request confirm parameters type.
 */
typedef struct _RF4CE_NWK_MacStatsConfParams_t
{
    uint8_t status;                     /*!< One of the RF4CE_MAC_STATS_Status_t values. */
    uint8_t result;                     /*!< One of the RF4CE_MAC_STATS_Result_t values. */
    uint8_t requestId;                  /*!< One of the RF4CE_MAC_STATS_RequestId_t values. */
    uint8_t attrId;                     /*!< One of the MAC_PibAttributeId_t values for Get/Set requests or 0 otherwise. */
} RF4CE_NWK_MacStatsConfParams_t;

/**//**
 * \brief MAC Statistics request type declaration.
 */
typedef struct _RF4CE_NWK_MacStatsReqDescr_t RF4CE_NWK_MacStatsReqDescr_t;

/**//**
 * \brief MAC Statistics request callback function type.
 */
typedef void (*RF4CE_NWK_MacStatsConfCallback_t)(RF4CE_NWK_MacStatsReqDescr_t *req, RF4CE_NWK_MacStatsConfParams_t *conf);

/**//**
 * \brief MAC Statistics request type.
 */
struct _RF4CE_NWK_MacStatsReqDescr_t
{
    RF4CE_NWK_MacStatsReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_MacStatsConfCallback_t callback;  /*!< Callback to inform sender on the result. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to transmit data.

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_MacStatsReq(RF4CE_NWK_MacStatsReqDescr_t *request);

#endif /* _RF4CE_NWK_MAC_STATS_H */
