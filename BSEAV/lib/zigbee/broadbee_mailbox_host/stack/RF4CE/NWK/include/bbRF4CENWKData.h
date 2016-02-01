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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKData.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer Data transfer handler.
 *
 * $Revision: 3111 $
 * $Date: 2014-07-30 09:40:47Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_DATA_H
#define _RF4CE_NWK_DATA_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKNIBAttributes.h"
#include "bbSysQueue.h"
#include "bbSysMemMan.h"
#include "bbMacSapForRF4CE.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE NWK NLME-DATA.request TX Options Flags.
 */
#define RF4CE_NWK_DATA_TX_BROADCAST             0x01
#define RF4CE_NWK_DATA_TX_IEEEADDRESS           0x02
#define RF4CE_NWK_DATA_TX_ACKNOWLEDGED          0x04
#define RF4CE_NWK_DATA_TX_SECURITY              0x08
#define RF4CE_NWK_DATA_TX_SINGLE_CHANNEL        0x10
#define RF4CE_NWK_DATA_TX_CHANNEL_DESIGNATOR    0x20
#define RF4CE_NWK_DATA_TX_VENDOR_SPECIFIC       0x40
#define RF4CE_NWK_DATA_USE_SHORT_RETRY_PERIOD   0x80

/**//**
 * \brief RF4CE NWK NLME-DATA.indication RX Options Flags.
 */
#define RF4CE_NWK_DATA_RX_BROADCAST             0x01
#define RF4CE_NWK_DATA_RX_SECURITY              0x02
#define RF4CE_NWK_DATA_RX_VENDOR_SPECIFIC       0x04

/**//**
 * \brief RF4CE NWK Outgoing packet TX flags.
 */
#define RF4CE_NWK_OP_TX_FIRST_ATTEMPT           0x01
#define RF4CE_NWK_OP_TX_START_TIMEOUT           0x02

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK NLME-DATA.request parameters type.
 */
typedef struct _RF4CE_NWK_DataReqParams_t
{
    uint8_t pairingRef;                         /*!< Reference into the pairing table which contains the information
                                                     required to transmit the NSDU. This parameter is ignored if the
                                                     TxOptions parameter specifies a broadcast transmission. */
    uint8_t profileId;                          /*!< The identifier of the profile indicating the format of the
                                                     transmitted data. */
    uint16_t vendorId;                          /*!< If the TxOptions parameter specifies that the data is
                                                     vendor-specific, this parameter specifies the vendor identifier.
                                                     If this parameter is equal to 0x0000, the vendor identifier should
                                                     be set to nwkcVendorIdentifier. If the TxOptions parameter
                                                     specifies that the data is not vendor-specific this parameter
                                                     is ignored. */
    SYS_DataPointer_t payload;                  /*!< The set of octets forming the NSDU to be transmitted by the NLDE. */
    uint8_t txOptions;                          /*!< Transmission options for this NSDU. Combination of
                                                     RF4CE_NWK_DATA_TX_xxx constants. */
} RF4CE_NWK_DataReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-DATA.indication parameters type.
 */
typedef struct _RF4CE_NWK_DataIndParams_t
{
    uint8_t pairingRef;                         /*!< Reference into the pairing table which matched the information
                                                     contained in the received NPDU. */
    uint8_t profileId;                          /*!< The identifier of the profile indicating the format of the received
                                                     data. */
    uint16_t vendorId;                          /*!< If the RxFlags parameter specifies that the data is vendor-specific,
                                                     this parameter specifies the vendor identifier. If the RxFlags
                                                     parameter specifies that the data is not vendor-specific this
                                                     parameter is ignored. */
    SYS_DataPointer_t payload;                  /*!< The set of octets forming the NSDU received by the NLDE. */
    uint8_t rxLinkQuality;                      /*!< LQI value measured during the reception of the NPDU. Lower values
                                                     represent lower LQI. */
    uint8_t rxFlags;                            /*!< Reception indication flags for this NSDU. Combination of
                                                     RF4CE_NWK_DATA_RX_xxx constants. */
} RF4CE_NWK_DataIndParams_t;

/**//**
 * \brief RF4CE NWK NLME-DATA.confirm parameters type.
 */
typedef struct _RF4CE_NWK_DataConfParams_t
{
    uint8_t status;                     /*!< The status of the NSDU transmission. */
    uint8_t pairingRef;                 /*!< The pairing table reference for the NSDU being confirmed. */
} RF4CE_NWK_DataConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-DATA.request type declaration.
 */
typedef struct _RF4CE_NWK_DataReqDescr_t RF4CE_NWK_DataReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-DATA confirmation function type.
 */
typedef void (*RF4CE_NWK_DataConfCallback_t)(RF4CE_NWK_DataReqDescr_t *req, RF4CE_NWK_DataConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-DATA.request type.
 */
struct _RF4CE_NWK_DataReqDescr_t
{
#ifndef _HOST_
    SYS_QueueElement_t queueElement;        /*!< Service field */
#endif /* _HOST_ */
    RF4CE_NWK_DataReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_DataConfCallback_t callback;  /*!< Callback to inform sender on the result. */
};

/**//**
 * \brief RF4CE NWK NLME-DATA.indication callback function type.
 */
typedef void (RF4CE_NWK_DataIndCallback_t)(RF4CE_NWK_DataIndParams_t *indication);

/**//**
 * \brief RF4CE NWK incoming packet type.
 */
typedef struct _RF4CE_NWK_IncomingPacket_t
{
    SYS_QueueElement_t queueElement;        /*!< Service field */
    MAC_AddrMode_t srcAddressMode;          /*!< Source address mode */
    MAC_Address_t srcAddress;               /*!< Source address */
    MAC_PanId_t srcPanId;                   /*!< Source PAN ID */
    SYS_DataPointer_t data;                 /*!< Data transferred */
    uint8_t dataLength;                     /*!< Length of the data transferred */
    uint8_t lqi;                            /*!< Packet's LQI */
    Bool8_t isBroadcast;                    /*!< true if the packet is broadcast */
} RF4CE_NWK_IncomingPacket_t;

/**//**
 * \brief RF4CE NWK outgoing packet type.
 */
typedef struct _RF4CE_NWK_OutgoingPacket_t
{
    SYS_QueueElement_t queueElement;        /*!< Service field */
    RF4CE_NWK_DataReqDescr_t *request;      /*!< Pointer to the data request structure */
    RF4CE_PairingTableEntry_t *pair;        /*!< Pointer to the pairing table entry */
    MAC_DataReqDescr_t macData;             /*!< MAC data request */
    MAC_SetReqDescr_t macSet;               /*!< MAC set request */
    uint8_t txFlags;                        /*!< Transmission flags. A combination of the RF4CE_NWK_OP_TX_xxx values;
                                                 Useful for unicast multiple channel acknowledged transmission */
} RF4CE_NWK_OutgoingPacket_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to transmit data.

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DataReq(RF4CE_NWK_DataReqDescr_t *request);

/************************************************************************************//**
 \brief RF4CE NWK NLME-DATA.indication prototype. For more information see RF4CE_NWK_DataIndCallback_t type.
 ****************************************************************************************/
extern RF4CE_NWK_DataIndCallback_t RF4CE_NWK_DataInd;

#endif /* _RF4CE_NWK_DATA_H */