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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/bbRF4CEMSORIB.h $
 *
 * DESCRIPTION:
 *   This is the header file for the MSO RF4CE Profile
 *   Remote Information Base component.
 *
 * $Revision: 1814 $
 * $Date: 2014-03-14 13:05:14Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_RIB_H
#define _RF4CE_MSO_RIB_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CENWKConstants.h"
#include "bbSysQueue.h"
#include "bbSysPayload.h"
#include "bbRF4CENWKRequestService.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO RIB attributes IDs.
 */
typedef enum _RF4CE_MSO_RIBAttributeID_t
{
    RF4CE_MSO_RIB_PEREFERAL_IDS            = 0x00, /*!< RW. Identifiers of the Peripherals */
    RF4CE_MSO_RIB_RF_STATISTICS            = 0x01, /*!< RW. RF Statistics */
    RF4CE_MSO_RIB_VERSIONING               = 0x02, /*!< RW. Versions of different parts of the device */
    RF4CE_MSO_RIB_BATTERY_STATUS           = 0x03, /*!< RW. Controller battery status information */
    RF4CE_MSO_RIB_SHORT_RF_RETRY_PERIOD    = 0x04, /*!< RO. The maximum time in us a unicast acknowledged multichannel
                                                        transmission shall be retried in case the Short RF Retry
                                                        configuration is set. */
    RF4CE_MSO_RIB_IRRF_DATABASE            = 0xDB, /*!< RO. IR and RF codes for different keys */
    RF4CE_MSO_RIB_VALIDATION_CONFIGURATION = 0xDC, /*!< RO. Configurable properties of the validation procedure */
    RF4CE_MSO_RIB_GENERAL_PURPOSE          = 0xFF  /*!< RW. General purpose remote storage */
} RF4CE_MSO_RIBAttributeID_t;

/**//**
 * \brief RF4CE MSO RIB attribute status codes.
 */
typedef enum _RF4CE_MSO_RIBAttributeStatus_t
{
    RF4CE_MSO_RIB_SUCCESS = 0x00,
    RF4CE_MSO_RIB_INVALID_PARAMETER,
    RF4CE_MSO_RIB_UNSUPPORTED_ATTRIBUTE,
    RF4CE_MSO_RIB_INVALID_INDEX,
    RF4CE_MSO_RIB_UNSUPPORTED_REQUEST,
    RF4CE_MSO_RIB_NO_MEMORY,
    RF4CE_MSO_RIB_ERROR_RX_ON,
    RF4CE_MSO_RIB_ERROR_DATA_SEND,
    RF4CE_MSO_RIB_ERROR_TIMEOUT
} RF4CE_MSO_RIBAttributeStatus_t;

/**//**
 * \brief RF4CE MSO RIB attribute Versioning Indexes for access IDs.
 */
typedef enum _RF4CE_MSO_RIB_Versioning_t
{
    RF4CE_MSO_RIB_VERSIONING_SW = 0x00,
    RF4CE_MSO_RIB_VERSIONING_HW = 0x01,
    RF4CE_MSO_RIB_VERSIONING_IRDB = 0x02
} RF4CE_MSO_RIB_Versioning_t;

/**//**
 * \brief RF4CE MSO RIB attribute RF Statistics data size.
 */
#define RF4CE_MSO_RF_STATISTICS_DATA_SIZE 16

/**//**
 * \brief RF4CE MSO RIB attribute General Purpose Row size.
 */
#define RF4CE_MSO_GENERAL_PURPOSE_ROW_SIZE 16

/**//**
 * \brief RF4CE MSO RIB Software Version access macros.
 */
#define RF4CE_MSO_SW_GET_MAJOR(version) ((version) & 0xFF)
#define RF4CE_MSO_SW_SET_MAJOR(version, value) (((version) & 0xFFFFFF00) | ((value) & 0xFF))
#define RF4CE_MSO_SW_GET_MINOR(version) (((version) >> 8) & 0xFF)
#define RF4CE_MSO_SW_SET_MINOR(version, value) (((version) & 0xFFFF00FF) | (((value) & 0xFF) << 8))
#define RF4CE_MSO_SW_GET_REVISION(version) (((version) >> 16) & 0xFF)
#define RF4CE_MSO_SW_SET_REVISION(version, value) (((version) & 0xFF00FFFF) | (((value) & 0xFF) << 16))
#define RF4CE_MSO_SW_GET_PATCH(version) (((version) >> 24) & 0xFF)
#define RF4CE_MSO_SW_SET_PATCH(version, value) (((version) & 0xFFFFFF) | (((value) & 0xFF) << 24))

/**//**
 * \brief RF4CE MSO RIB Hardware Version access macros.
 */
#define RF4CE_MSO_HW_GET_MODEL(version) ((version) & 0x0F)
#define RF4CE_MSO_HW_SET_MODEL(version, value) (((version) & 0xFFFFFFF0) | ((value) & 0x0F))
#define RF4CE_MSO_SW_GET_MANUFACTURER(version) (((version) >> 4) & 0x0F)
#define RF4CE_MSO_SW_SET_MANUFACTURER(version, value) (((version) & 0xFFFFFF0F) | (((value) & 0x0F) << 4))
#define RF4CE_MSO_SW_GET_HW_REVISION(version) (((version) >> 8) & 0xFF)
#define RF4CE_MSO_SW_SET_HW_REVISION(version, value) (((version) & 0xFFFF00FF) | (((value) & 0xFF) << 8))
#define RF4CE_MSO_SW_GET_LOT_CODE(version) (((((version) >> 16) & 0x0F) << 8) | (((version) >> 24) & 0xFF))
#define RF4CE_MSO_SW_SET_LOT_CODE(version, value) (((version) & 0x00F0FFFF) | (((value) & 0x0F00) << 8) | (((value) & 0xFF) << 24))

/**//**
 * \brief RF4CE MSO RIB IRDB Version access macros.
 */
#define RF4CE_MSO_IRDB_GET_MAJOR(version) ((version) & 0xFF)
#define RF4CE_MSO_IRDB_SET_MAJOR(version, value) (((version) & 0xFFFFFF00) | ((value) & 0xFF))
#define RF4CE_MSO_IRDB_GET_MINOR(version) (((version) >> 8) & 0xFF)
#define RF4CE_MSO_IRDB_SET_MINOR(version, value) (((version) & 0xFFFF00FF) | (((value) & 0xFF) << 8))
#define RF4CE_MSO_IRDB_GET_REVISION(version) (((version) >> 16) & 0xFF)
#define RF4CE_MSO_IRDB_SET_REVISION(version, value) (((version) & 0xFF00FFFF) | (((value) & 0xFF) << 16))
#define RF4CE_MSO_IRDB_GET_PATCH(version) (((version) >> 24) & 0xFF)
#define RF4CE_MSO_IRDB_SET_PATCH(version, value) (((version) & 0xFFFFFF) | (((value) & 0xFF) << 24))

/**//**
 * \brief RF4CE MSO RIB Battery Status Flags access macros.
 */
#define RF4CE_MSO_BAT_GET_REPLACEMENT(flags) ((flags) & 0x01)
#define RF4CE_MSO_BAT_SET_REPLACEMENT(flags, value) (((flags) & 0xFE) | ((value) ? 0x01 : 0))
#define RF4CE_MSO_BAT_GET_CHARGING(flags) (((flags) >> 1) & 0x01)
#define RF4CE_MSO_BAT_SET_CHARGING(flags, value) (((flags) & 0xFD) | ((value) ? 0x02 : 0))
#define RF4CE_MSO_BAT_GET_IMPENDING_DOOM(flags) (((flags) >> 2) & 0x01)
#define RF4CE_MSO_BAT_SET_IMPENDING_DOOM(flags, value) (((flags) & 0xFB) | ((value) ? 0x04 : 0))

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database RF descriptor rfConfig field access macros.
 */
#define RF4CE_MSO_RFD_GET_MIN_TRANSMISSIONS(config) ((config) & 0x0F)
#define RF4CE_MSO_RFD_SET_MIN_TRANSMISSIONS(config, value) (((config) & 0xF0) | ((value) & 0x0F))
#define RF4CE_MSO_RFD_GET_KEEP_TRANSMITTING(config) (((config) >> 4) & 0x01)
#define RF4CE_MSO_RFD_SET_KEEP_TRANSMITTING(config, value) (((config) & 0xEF) | ((value) ? 0x10 : 0))
#define RF4CE_MSO_RFD_GET_SHORT_RETRY(config) (((config) >> 5) & 0x01)
#define RF4CE_MSO_RFD_SET_SHORT_RETRY(config, value) (((config) & 0xDF) | ((value) ? 0x20 : 0))

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database IR descriptor irConfig field access macros.
 */
#define RF4CE_MSO_IRD_GET_MIN_TRANSMISSIONS(config) ((config) & 0x0F)
#define RF4CE_MSO_IRD_SET_MIN_TRANSMISSIONS(config, value) (((config) & 0xF0) | ((value) & 0x0F))
#define RF4CE_MSO_IRD_GET_KEEP_TRANSMITTING(config) (((config) >> 4) & 0x01)
#define RF4CE_MSO_IRD_SET_KEEP_TRANSMITTING(config, value) (((config) & 0xEF) | ((value) ? 0x10 : 0))
#define RF4CE_MSO_IRD_GET_TWEAK_DB(config) (((config) >> 6) & 0x01)
#define RF4CE_MSO_IRD_SET_TWEAK_DB(config, value) (((config) & 0xBF) | ((value) ? 0x40 : 0))

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database flags field access macros.
 */
#define RF4CE_MSO_IRRFDB_FLAGS_GET_PRESSED(flags) ((flags) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_PRESSED(flags, value) (((flags) & 0xFE) | ((value) ? 0x01 : 0))
#define RF4CE_MSO_IRRFDB_FLAGS_GET_REPEATED(flags) (((flags) >> 1) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_REPEATED(flags, value) (((flags) & 0xFD) | ((value) ? 0x02 : 0))
#define RF4CE_MSO_IRRFDB_FLAGS_GET_RELEASED(flags) (((flags) >> 2) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_RELEASED(flags, value) (((flags) & 0xFB) | ((value) ? 0x04 : 0))
#define RF4CE_MSO_IRRFDB_FLAGS_GET_IR(flags) (((flags) >> 3) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_IR(flags, value) (((flags) & 0xF7) | ((value) ? 0x08 : 0))
#define RF4CE_MSO_IRRFDB_FLAGS_GET_USE_DEFAULT(flags) (((flags) >> 6) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_USE_DEFAULT(flags, value) (((flags) & 0xBF) | ((value) ? 0x40 : 0))
#define RF4CE_MSO_IRRFDB_FLAGS_GET_PERMANENT(flags) (((flags) >> 7) & 0x01)
#define RF4CE_MSO_IRRFDB_FLAGS_SET_PERMANENT(flags, value) (((flags) & 0x7F) | ((value) ? 0x80 : 0))

/**//**
 * \brief RF4CE MSO base file index for RIB attributes
 */
#define RF4CE_MSO_RIB_BASE_FILE_INDEX_ID       (1)

/**//**
 * \brief Gets the base file index of the specified MSO RIB attribute
 */
#define RF4CE_MSO_RIB_BASE_FILE_INDEX(attrId) \
    (RF4CE_MSO_RIB_BASE_FILE_INDEX_ID + (attrId == 0xFF ? 0xFE : attrId))

/**//**
 * \brief Gets the file index of the specified MSO RIB attribute for the specified pair
 *  and the specified index of element
 */
#define RF4CE_MSO_RIB_FILE_INDEX(pair, attrId, index) \
        ((RF4CE_MSO_RIB_BASE_FILE_INDEX(attrId) | \
         (((RF4CE_NWK_INVALID_PAIRING_REF == pair) ? RF4CE_MSO_RIB_BASE_FILE_INDEX_ID : pair) << 8) | \
         (index << 16)) & 0xFFFFFF)

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO RIB attribute Peripheral ID type.
 */
typedef uint32_t RF4CE_MSO_RIB_PeripheralID_t;   /*!< User defined peripheral ID */

/**//**
 * \brief RF4CE MSO RIB attribute RF Statistics type.
 */
typedef struct _RF4CE_MSO_RIB_RFStatistics_t
{
    uint8_t rfStatistics[RF4CE_MSO_RF_STATISTICS_DATA_SIZE];   /*!< RF Statistics data */
} RF4CE_MSO_RIB_RFStatistics_t;

/**//**
 * \brief RF4CE MSO RIB attribute Versioning SW Version type.
 *        Use RF4CE_MSO_SW_GET_XXX/RF4CE_MSO_SW_SET_XXX macros for access
 */
typedef uint32_t RF4CE_MSO_RIB_VersioningSW_t;

/**//**
 * \brief RF4CE MSO RIB attribute Versioning HW Version type.
 *        Use RF4CE_MSO_HW_GET_XXX/RF4CE_MSO_HW_SET_XXX macros for access
 */
typedef uint32_t RF4CE_MSO_RIB_VersioningHW_t;

/**//**
 * \brief RF4CE MSO RIB attribute Versioning IRDB Version type.
 *        Use RF4CE_MSO_IRDB_GET_XXX/RF4CE_MSO_IRDB_SET_XXX macros for access
 */
typedef uint32_t RF4CE_MSO_RIB_VersioningIRDB_t;

/**//**
 * \brief RF4CE MSO RIB attribute Battery Status type.
 */
typedef struct PACKED _RF4CE_MSO_RIB_BatteryStatus_t
{
    uint8_t flags;                          /*!< Flags. Use RF4CE_MSO_BAT_GET_XXX/RF4CE_MSO_BAT_SET_XXX macros for access */
    uint8_t loadedVoltageLevel;             /*!< The voltage level reported by the controller under a load of 25 mA.
                                                 Linearly interpolated between 0 V (0x00) and 4 V (0xFF). */
    uint32_t numRFCodesTransmitted;         /*!< Number of RF codes transmitted since the last battery replacement. */
    uint32_t numIRCodesTransmitted;         /*!< Number of IR codes transmitted since the last battery replacement. */
    uint8_t unloadedVoltageLevel;           /*!< The voltage level reported by the controller under a load of 5 mA.
                                                 Linearly interpolated between 0 V (0x00) and 4 V (0xFF). */
} RF4CE_MSO_RIB_BatteryStatus_t;

/**//**
 * \brief RF4CE MSO RIB attribute ShortRFRetryPeriod type.
 */
typedef uint32_t RF4CE_MSO_RIB_ShortRFRetryPeriod_t;

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database RF descriptor type.
 */
typedef struct _RF4CE_MSO_RIB_IRRFDB_RFDescriptor_t
{
    uint8_t rfConfig;             /*!< RF Config. Use RF4CE_MSO_RFD_GET_XXX/RF4CE_MSO_RFD_SET_XXX macros for access */
    uint8_t txOptions;            /*!< RF4CE Tx Options */
    uint8_t payloadLength;        /*!< Payload Length */
    uint8_t data[1];              /*!< Variable size RF4CE-NWK Payload */
} RF4CE_MSO_RIB_IRRFDB_RFDescriptor_t;

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database IR descriptor type.
 */
typedef struct _RF4CE_MSO_RIB_IRRFDB_IRDescriptor_t
{
    uint8_t irConfig;                       /*!< IR Config. Use RF4CE_MSO_IRD_GET_XXX/RF4CE_MSO_IRD_SET_XXX macros for access */
    uint8_t irCodeLength;                   /*!< IR Code Length */
    uint8_t data[1];                        /*!< Vriable size IR Code */
} RF4CE_MSO_RIB_IRRFDB_IRDescriptor_t;

/**//**
 * \brief RF4CE MSO RIB attribute IR-RF Database type.
 */
typedef struct _RF4CE_Cable_RIB_IRRFDBEntry_t
{
    uint8_t flags;                           /*!< Flags. Use RF4CE_MSO_IRRFDB_FLAGS_GET_XXX/RF4CE_MSO_IRRFDB_FLAGS_SET_XXX macros for access */
    RF4CE_MSO_RIB_IRRFDB_RFDescriptor_t rfPressedDescriptor;     /*!< Optional RF Pressed Descriptor */
    RF4CE_MSO_RIB_IRRFDB_RFDescriptor_t rfRepeatedDescriptor;    /*!< Optional RF Repeated Descriptor */
    RF4CE_MSO_RIB_IRRFDB_RFDescriptor_t rfReleasedDescriptor;    /*!< Optional RF Released Descriptor */
    RF4CE_MSO_RIB_IRRFDB_IRDescriptor_t irDescriptor;            /*!< Optional IR Descriptor */
} RF4CE_MSO_RIB_IRRFDBEntry_t;

/**//**
 * \brief RF4CE MSO RIB attribute Validation Configuration type.
 */
typedef struct PACKED _RF4CE_MSO_RIB_ValidationConfiguration_t
{
    uint16_t autoCheckValidationPeriod;     /*!< Auto Check Validation Period in milliseconds. */
    uint16_t linkLostWaitTime;              /*!< Link Lost Wait Time in milliseconds. */
} RF4CE_MSO_RIB_ValidationConfiguration_t;

/**//**
 * \brief RF4CE MSO RIB attribute General Purpose type.
 */
typedef struct _RF4CE_MSO_RIB_GeneralPurposeEntry_t
{
    uint8_t row[RF4CE_MSO_GENERAL_PURPOSE_ROW_SIZE];    /*!< General Purpose Memory single row. */
} RF4CE_MSO_RIB_GeneralPurposeEntry_t;

/**//**
 * \brief RF4CE MSO RIB attributes Set Request Parameters type.
 */
typedef struct _RF4CE_MSO_SetRIBAttributeReqParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference for the request. */
    uint8_t attributeID;       /*!< Requested attribute ID. */
    uint8_t attributeIndex;    /*!< Requested attribute Index. */
    SYS_DataPointer_t payload; /*!< The requested attribute value. */
} RF4CE_MSO_SetRIBAttributeReqParams_t;

/**//**
 * \brief RF4CE MSO RIB attributes Set Response (Confirmation) Parameters type.
 */
typedef struct _RF4CE_MSO_SetRIBAttributeConfParams_t
{
    uint8_t status;         /*!< Requested attribute status: one of the RF4CE_MSO_RIBAttributeStatus_t codes */
    uint8_t attributeId;    /*!< Requested attribute ID. */
    uint8_t attributeIndex; /*!< Requested attribute Index. */
} RF4CE_MSO_SetRIBAttributeConfParams_t;

/**//**
 * \brief RF4CE MSO RIB attributes Set Request Descriptor type declaration.
 */
typedef struct _RF4CE_MSO_SetRIBAttributeReqDescr_t RF4CE_MSO_SetRIBAttributeReqDescr_t;

/**//**
 * \brief RF4CE MSO RIB attributes Set Request callback.
 */
typedef void (*RF4CE_MSO_SetRIBAttributeCallback_t)(RF4CE_MSO_SetRIBAttributeReqDescr_t *req, RF4CE_MSO_SetRIBAttributeConfParams_t *conf);

/**//**
 * \brief RF4CE MSO RIB attributes Set Request Descriptor type.
 */
struct _RF4CE_MSO_SetRIBAttributeReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;           /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_MSO_SetRIBAttributeReqParams_t params;  /*!< Request parameters. */
    RF4CE_MSO_SetRIBAttributeCallback_t callback; /*!< Request callback. */
};

/**//**
 * \brief RF4CE MSO RIB attributes Get Request Parameters type.
 */
typedef struct _RF4CE_MSO_GetRIBAttributeReqParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference for the request. */
    uint8_t attributeID;       /*!< Requested attribute ID. */
    uint8_t attributeIndex;    /*!< Requested attribute Index. */
    uint8_t valueLength;       /*!< Requested attribute length in bytes. */
} RF4CE_MSO_GetRIBAttributeReqParams_t;

/**//**
 * \brief RF4CE MSO RIB attributes Get Response (Confirmation) Parameters type.
 */
typedef struct _RF4CE_MSO_GetRIBAttributeConfParams_t
{
    uint8_t status;            /*!< Requested attribute status: one of the RF4CE_MSO_RIBAttributeStatus_t codes */
    uint8_t attributeId;       /*!< Requested attribute ID. */
    uint8_t attributeIndex;    /*!< Requested attribute Index. */
    SYS_DataPointer_t payload; /*!< The requested attribute value. */
} RF4CE_MSO_GetRIBAttributeConfParams_t;

/**//**
 * \brief RF4CE MSO RIB attributes Get Request Descriptor type declaration.
 */
typedef struct _RF4CE_MSO_GetRIBAttributeReqDescr_t RF4CE_MSO_GetRIBAttributeReqDescr_t;

/**//**
 * \brief RF4CE MSO RIB attributes Get Request callback.
 */
typedef void (*RF4CE_MSO_GetRIBAttributeCallback_t)(RF4CE_MSO_GetRIBAttributeReqDescr_t *req, RF4CE_MSO_GetRIBAttributeConfParams_t *conf);

/**//**
 * \brief RF4CE MSO RIB attributes Get Request Descriptor type.
 */
struct _RF4CE_MSO_GetRIBAttributeReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;           /*!< Service field. */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_MSO_GetRIBAttributeReqParams_t params;  /*!< Request parameters. */
    RF4CE_MSO_GetRIBAttributeCallback_t callback; /*!< Request callback. */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initializes MSO RIB Attribute Set request.

 \param[in] request - pointer to the request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_SetRIBAttributeReq(RF4CE_MSO_SetRIBAttributeReqDescr_t *request);

/************************************************************************************//**
 \brief Initializes MSO RIB Attribute Get request.

 \param[in] request - pointer to the request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_GetRIBAttributeReq(RF4CE_MSO_GetRIBAttributeReqDescr_t *request);

#endif /* _RF4CE_MSO_RIB_H */