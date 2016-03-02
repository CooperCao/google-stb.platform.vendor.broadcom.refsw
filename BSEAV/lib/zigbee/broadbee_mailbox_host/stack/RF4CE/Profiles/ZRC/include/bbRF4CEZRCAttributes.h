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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCAttributes.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC profile attributes handler.
 *
 * $Revision: 8795 $
 * $Date: 2015-11-07 00:57:03Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_ATTRIBUTES_H
#define _RF4CE_ZRC_ATTRIBUTES_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"
#include "bbSysMemMan.h"
#include "bbRF4CEZRCConstants.h"
#include "private/bbRF4CEZRCProfileData.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief RF4CE GDP 2.0 capabilities flags.
 */
#define RF4CE_GDP_CAPS_SUPPORT_EXTENDED_VALIDATION            0x01
#define RF4CE_GDP_CAPS_SUPPORT_POLL_SERVER                    0x02
#define RF4CE_GDP_CAPS_SUPPORT_POLL_CLIENT                    0x04
#define RF4CE_GDP_CAPS_SUPPORT_ID_SERVER                      0x08
#define RF4CE_GDP_CAPS_SUPPORT_ID_CLIENT                      0x10
#define RF4CE_GDP_CAPS_SUPPORT_ENHANCED_SECURITY              0x20
#define RF4CE_GDP_CAPS_SUPPORT_SHARED_SECRET_OF_LOCAL_VENDOR  0x40
#define RF4CE_GDP_CAPS_SUPPORT_SHARED_SECRET_OF_REMOTE_VENDOR 0x80

/**//**
 * \brief RF4CE GDP 2.0 power flags.
 */
#define RF4CE_GDP_POWER_CRITICAL                              0x20
#define RF4CE_GDP_POWER_CHARGING                              0x40
#define RF4CE_GDP_POWER_IMPENDING_DOOM                        0x80
#define RF4CE_GDP_GET_POWER_METER(v) ((v) & 0x0F)
#define RF4CE_GDP_SET_POWER_METER(v, value) (((v) & 0xF0) | ((value) & 0x0F))
#define RF4CE_GDP_IS_POWER_CRITICAL(v) (0 != ((v) & RF4CE_GDP_POWER_CRITICAL))
#define RF4CE_GDP_SET_POWER_CRITICAL(v, value) (((v) & ~RF4CE_GDP_POWER_CRITICAL) | ((value) ? RF4CE_GDP_POWER_CRITICAL : 0))
#define RF4CE_GDP_IS_POWER_CHARGING(v) (0 != ((v) & RF4CE_GDP_POWER_CHARGING))
#define RF4CE_GDP_SET_POWER_CHARGING(v, value) (((v) & ~RF4CE_GDP_POWER_CHARGING) | ((value) ? RF4CE_GDP_POWER_CHARGING : 0))
#define RF4CE_GDP_IS_POWER_IMPENDING_DOOM(v) (0 != ((v) & RF4CE_GDP_POWER_IMPENDING_DOOM))
#define RF4CE_GDP_SET_POWER_IMPENDING_DOOM(v, value) (((v) & ~RF4CE_GDP_POWER_IMPENDING_DOOM) | ((value) ? RF4CE_GDP_POWER_IMPENDING_DOOM : 0))

/**//**
 * \brief RF4CE GDP 2.0 polling trigger flags.
 */
#define RF4CE_GDP_POLLING_TRIGGER_TIME_BASED                 0x01
#define RF4CE_GDP_POLLING_TRIGGER_ON_KEY_PRESS               0x02
#define RF4CE_GDP_POLLING_TRIGGER_ON_PICK_UP                 0x04
#define RF4CE_GDP_POLLING_TRIGGER_ON_RESET                   0x08
#define RF4CE_GDP_POLLING_TRIGGER_ON_MIC_ACTIVITY            0x10
#define RF4CE_GDP_POLLING_TRIGGER_ON_OTHER_USER_ACTIVITY     0x20

/**//**
 * \brief RF4CE GDP 2.0 identification capabilities flags.
 */
#define RF4CE_GDP_ID_CAPS_SUPPORTS_FLASH_LIGHT               0x02
#define RF4CE_GDP_ID_CAPS_SUPPORTS_SHORT_SOUND               0x04
#define RF4CE_GDP_ID_CAPS_SUPPORTS_VIBRATE                   0x08

/**//**
 * \brief RF4CE ZRC 2.0 capabilities flags.
 */
#define RF4CE_ZRC_CAPS_ACTIONS_ORIGINATOR                    0x01
#define RF4CE_ZRC_CAPS_ACTIONS_RECIPIENT                     0x02
#define RF4CE_ZRC_CAPS_HA_ACTIONS_ORIGINATOR                 0x04
#define RF4CE_ZRC_CAPS_HA_ACTIONS_RECIPIENT                  0x08
#define RF4CE_ZRC_CAPS_ACTIONS_MAPPING_CLIENT                0x10
#define RF4CE_ZRC_CAPS_ACTIONS_MAPPING_SERVER                0x20
#define RF4CE_ZRC_CAPS_SUPPORT_VENDOR_IRDB_FORMATS           0x40
#define RF4CE_ZRC_CAPS_INFORM_ABOUT_SUPPORTED_ACTIONS        0x80

/**//**
 * \brief Default values of the RF4CE_ZRC2_PROFILE_CAPABILITIES Attribute
 */
#ifdef RF4CE_CONTROLLER
#   define RF4CE_ZRC2_PROFILE_CAPABILITIES_DEFAULT                                  \
    (RF4CE_ZRC_CAPS_ACTIONS_ORIGINATOR | RF4CE_ZRC_CAPS_INFORM_ABOUT_SUPPORTED_ACTIONS)
#else
#   define RF4CE_ZRC2_PROFILE_CAPABILITIES_DEFAULT                                  \
    (RF4CE_ZRC_CAPS_ACTIONS_ORIGINATOR | RF4CE_ZRC_CAPS_ACTIONS_RECIPIENT | RF4CE_ZRC_CAPS_INFORM_ABOUT_SUPPORTED_ACTIONS)
#endif
/**//**
 * \brief Declarations of Simple ZRC2.0 and GDP2.0 Attributes
 * \note: use RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(attrId, rootOfName, type, defaultValue)
 */
/* attributes are defined in order to minimize memory dump */
#define RF4CE_ZRC2_SIMPLE_ATTR_DECLARATIONS                                         \
    /* GDP2 Attributes */                                                           \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x80, GDP2_VERSION, uint16_t, 0x0200)            \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x82, KEY_EXCHANGE_TRANSFER_COUNT, uint8_t, 3)   \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x83, POWER_STATUS, uint8_t, 0x00)               \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x81, GDP2_CAPABILITIES, uint32_t, RF4CE_GDP_CAPS_SUPPORT_ENHANCED_SECURITY) \
    /* aplPollConstraints and aplPollConfiguration are not simple attributes */     \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x86, MAX_BINDING_CANDIDATES, uint8_t, 3)        \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x8b, IDENTIFICATION_CAPABILITIES, uint8_t, 0)   \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x87, AUTO_CHECK_VALIDATION_PERIOD, uint16_t, 500 /* ms */)              \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x88, RECIPIENT_VALIDATION_WAIT_TIME, uint16_t, 10000 /* ms */)          \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x89, ORIGINATOR_VALIDATION_WAIT_TIME, uint16_t, 15000 /* ms */)         \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0x8A, LINK_LOST_WAIT_TIME, uint16_t, 0x1388 /* 5000ms */)                \
                                                                                                            \
    /* ZRC2 Attributes */                                                                                   \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA0, PROFILE_VERSION, uint16_t, 0x0200)         \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA3, ACTION_REPEAT_WAIT_TIME, uint16_t, RF4CE_ZRC_APLC_MAX_ACTION_REPEAT_TRIGGER_INTERVAL << 1) \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA1, PROFILE_CAPABILITIES, uint32_t, RF4CE_ZRC2_PROFILE_CAPABILITIES_DEFAULT)   \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA4, ACTION_BANKS_SUPPORTED_RX, RF4CE_ZRC2_Bitmap256_t, 0)              \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA5, ACTION_BANKS_SUPPORTED_TX, RF4CE_ZRC2_Bitmap256_t, 0)              \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA7, ACTION_BANKS_VERSION,  uint16_t, 0x0100)   \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xA2, ACTION_REPEAT_TRIGGER_INTERVAL, uint8_t, RF4CE_ZRC_APLC_MAX_ACTION_REPEAT_TRIGGER_INTERVAL >> 1) \
                                                                                                                                          \
    /* Additional simple attributes */                                                                                                    \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE0, BO_VENDOR_ID_FILTER, uint16_t, 0xFFFF)         \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE1, BO_MIN_MAX_CLASS_FILTER, uint8_t, 0xE0)        \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE2, BO_MIN_LQI_FILTER, uint8_t, 0x00)              \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE3, BR_PRIMARY_CLASS_DESCR, uint8_t, 0x0e)         \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE4, BR_SECONDARY_CLASS_DESCR, uint8_t, 0x18)       \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE5, BR_TERTIARY_CLASS_DESCR, uint8_t, 0x15)        \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE6, BR_DISC_LQI_THRESHOLD, uint8_t, 0x00)          \
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(0xE7, INITIAL_PAIR_KEY, SecurityKey_t, 0)

/**//**
 * \brief Attribute IDs enumeration
 */
#define RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(attrId, name, type, defaultValue)    \
        RF4CE_ZRC2_##name = attrId,
typedef enum _RF4CE_ZRC2GDP2_AttributesID_t
{
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARATIONS

    /* not simple attributes */
    RF4CE_ZRC2_POLL_CONSTRAINTS                = 0x84, /*!< Polling Methods supported. */
    RF4CE_ZRC2_POLL_CONFIGURATION              = 0x85, /*!< Current node's polling configuration. */
    RF4CE_ZRC2_IRDB_VENDOR_SUPPORT             = 0xa6, /* not supported */

    /* complex attributes */
    RF4CE_ZRC2_ACTION_CODES_SUPPORTED_RX       = 0xc0, /*!< Specifies which Actions codes are supported for the
                                                                corresponding Action bank. For RX. */
    RF4CE_ZRC2_ACTION_CODES_SUPPORTED_TX       = 0xc1, /*!< Specifies which Actions codes are supported for the
                                                                corresponding Action bank. For TX. */
    RF4CE_ZRC2_MAPPABLE_ACTIONS                = 0xc2, /*!< A vector of Action mappings that corresponds to the
                                                                aplMappableActions attribute pushed by the controller. */
    RF4CE_ZRC2_ACTION_MAPPINGS                 = 0xc3, /*!< A one dimensional array of entries, where each entry corresponds
                                                                to a description of the RF and IR code associated with a given action. */
    RF4CE_ZRC2_HOME_AUTOMATION                 = 0xc4, /*!< A two dimensional array of entries, indexed by the instance
                                                                ID and the HA Attribute ID. */
    RF4CE_ZRC2_HOME_AUTOMATION_SUPPORTED       = 0xc5, /*!< A one dimensional array of entries, each is 256-bits array. */
} RF4CE_ZRC2GDP2_AttributesID_t;
#undef RF4CE_ZRC2_SIMPLE_ATTR_DECLARE

/**//**
 * \brief RF4CE ZRC 2.0 simple attributes size macros
 */
#define RF4CE_ZRC2_GDP2_VERSION_SIZE                     sizeof(uint16_t)
#define RF4CE_ZRC2_KEY_EXCHANGE_TRANSFER_COUNT_SIZE      sizeof(uint8_t)
#define RF4CE_ZRC2_POWER_STATUS_SIZE                     sizeof(uint8_t)
#define RF4CE_ZRC2_GDP2_CAPABILITIES_SIZE                sizeof(uint32_t)
    /* aplPollConstraints and aplPollConfiguration are not simple attributes */
#define RF4CE_ZRC2_MAX_BINDING_CANDIDATES_SIZE           sizeof(uint8_t)
#define RF4CE_ZRC2_IDENTIFICATION_CAPABILITIES_SIZE      sizeof(uint8_t)
#define RF4CE_ZRC2_AUTO_CHECK_VALIDATION_PERIOD_SIZE     sizeof(uint16_t)
#define RF4CE_ZRC2_RECIPIENT_VALIDATION_WAIT_TIME_SIZE   sizeof(uint16_t)
#define RF4CE_ZRC2_ORIGINATOR_VALIDATION_WAIT_TIME_SIZE  sizeof(uint16_t)
#define RF4CE_ZRC2_LINK_LOST_WAIT_TIME_SIZE              sizeof(uint16_t)

    /* ZRC2 Attributes */
#define RF4CE_ZRC2_PROFILE_VERSION_SIZE                  sizeof(uint16_t)
#define RF4CE_ZRC2_ACTION_REPEAT_WAIT_TIME_SIZE          sizeof(uint16_t)
#define RF4CE_ZRC2_PROFILE_CAPABILITIES_SIZE             sizeof(uint32_t)
#define RF4CE_ZRC2_ACTION_BANKS_SUPPORTED_RX_SIZE        sizeof(RF4CE_ZRC2_Bitmap256_t)
#define RF4CE_ZRC2_ACTION_BANKS_SUPPORTED_TX_SIZE        sizeof(RF4CE_ZRC2_Bitmap256_t)
#define RF4CE_ZRC2_ACTION_BANKS_VERSION_SIZE             sizeof(uint16_t)
#define RF4CE_ZRC2_ACTION_REPEAT_TRIGGER_INTERVAL_SIZE   sizeof(uint8_t)

    /* Additional simple attributes */
#define RF4CE_ZRC2_BO_VENDOR_ID_FILTER_SIZE              sizeof(uint16_t)
#define RF4CE_ZRC2_BO_MIN_MAX_CLASS_FILTER_SIZE          sizeof(uint8_t)
#define RF4CE_ZRC2_BO_MIN_LQI_FILTER_SIZE                sizeof(uint8_t)
#define RF4CE_ZRC2_BR_PRIMARY_CLASS_DESCR_SIZE           sizeof(uint8_t)
#define RF4CE_ZRC2_BR_SECONDARY_CLASS_DESCR_SIZE         sizeof(uint8_t)
#define RF4CE_ZRC2_BR_TERTIARY_CLASS_DESCR_SIZE          sizeof(uint8_t)
#define RF4CE_ZRC2_BR_DISC_LQI_THRESHOLD_SIZE            sizeof(uint8_t)

/**//**
 * \brief RF4CE ZRC 2.0 RF4CE_ZRC2 ACTION BANKS maximum number
 */
#define RF4CE_ZRC2_ACTION_BANKS_MAX                 256

/**//**
 * \brief RF4CE ZRC 2.0 ACTION MAPPING supported per peer
 * \note: The value has to be a multiple of 4 (for valid RAM to NVM mapping)
 */
#define RF4CE_ZRC2_ACTION_MAPPINGS_MAX              8

/**//**
 * \brief RF4CE ZRC 2.0 ACTION_MAPPINGS entry supported maximum size
 */
#define RF4CE_ZRC2_ACTION_MAPPINGS_ENTRY_MAX_SIZE   87

/**//**
 * \brief The Spec says about MappableAction:
 *      A ZRC Recipient shall interpret a wildcard device type of 0xff in the Action device type field as invalid.
 */
#define RF4CE_ZRC2_ACTION_MAPPING_DEVICETYPE_INVALID    0xFF

/**//**
 * \brief Reserved device type value of the MappableAction element
 * which is used as an INTERNAL absence flag for the initialization purpose
 */
#define RF4CE_ZRC2_ACTION_MAPPING_DEVICETYPE_ABSENT     0x00

/**//**
 * \brief RF4CE ZRC 2.0 HA entities number by the Spec
 */
#define RF4CE_ZRC2_HA_ENTITIES_MAX                  32

/**//**
 * \brief Number of RF4CE ZRC 2.0 HA Attribute elements which are simultaneously available
 */
#define RF4CE_ZRC2_HA_ELEMENTS_MAX                  16

/**//**
 * \brief True if the specified Attribute Id supported
 */
#define RF4CE_ZRC2_IS_ATTR_SUPPORTED(attrId)                                                                    \
    ((RF4CE_ZRC2_GDP2_VERSION <= (attrId) && (attrId) <= RF4CE_ZRC2_IDENTIFICATION_CAPABILITIES) ||             \
     (RF4CE_ZRC2_PROFILE_VERSION <= (attrId) && (attrId) <= RF4CE_ZRC2_ACTION_BANKS_VERSION &&                  \
            RF4CE_ZRC2_IRDB_VENDOR_SUPPORT != (attrId)) ||                                                      \
     (RF4CE_ZRC2_ACTION_CODES_SUPPORTED_RX <= (attrId) && (attrId) <= RF4CE_ZRC2_HOME_AUTOMATION_SUPPORTED))

/**//**
 * \brief True if the specified Attribute Id belongs to ZRC2
 * Note: Shall be used with RF4CE_ZRC2_IS_ATTR_SUPPORTED(attrId)
 */
#define RF4CE_ZRC2_IS_ATTR_ZRC2(attrId) (RF4CE_ZRC2_PROFILE_VERSION <= (attrId))

/**//**
 * \brief True if the specified Attribute Id belongs to an attribute of scalar type
 */
#define RF4CE_ZRC2_IS_SCALAR_ATTR(attrId) ((attrId) < RF4CE_ZRC2_ACTION_CODES_SUPPORTED_RX)

/**//**
 * \brief True if the specified Attribute Id belongs to an additional (not specified in spec) attribute of scalar type
 */
#define RF4CE_ZRC2_IS_ADDITIONAL_SCALAR_ATTR(attrId) ((attrId) == RF4CE_ZRC2_BO_VENDOR_ID_FILTER ||          \
                                                      (attrId) == RF4CE_ZRC2_BO_MIN_MAX_CLASS_FILTER ||      \
                                                      (attrId) == RF4CE_ZRC2_BO_MIN_LQI_FILTER ||            \
                                                      (attrId) == RF4CE_ZRC2_BR_PRIMARY_CLASS_DESCR ||       \
                                                      (attrId) == RF4CE_ZRC2_BR_SECONDARY_CLASS_DESCR ||     \
                                                      (attrId) == RF4CE_ZRC2_BR_TERTIARY_CLASS_DESCR ||      \
                                                      (attrId) == RF4CE_ZRC2_BR_DISC_LQI_THRESHOLD ||        \
                                                      (attrId) == RF4CE_ZRC2_INITIAL_PAIR_KEY)

/**//**
 * \brief True if the specified Attribute Id belongs to an attribute of simple type
 */
#define RF4CE_ZRC2_IS_SIMPLE_ATTR(attrId)                                                                           \
    ((RF4CE_ZRC2_IS_SCALAR_ATTR(attrId) ||                                                                          \
      RF4CE_ZRC2_IS_ADDITIONAL_SCALAR_ATTR(attrId)) &&                                                              \
     (attrId) != RF4CE_ZRC2_POLL_CONFIGURATION &&                                                                   \
     (attrId) != RF4CE_ZRC2_POLL_CONSTRAINTS &&                                                                     \
     (attrId) != RF4CE_ZRC2_IRDB_VENDOR_SUPPORT)

/**//**
 * \brief   Report the type of attribute: either arrayed or scalar.
 * \param[in]   attrId      Identifier of the attribute.
 * \return  TRUE if an attribute is of the arrayed type; FALSE if it's of the scalar type.
 * \details Arrayed attributes have identifiers in ranges 0x90-0x9F and 0xC0-0xDF. All
 *  other identifiers correspond to scalar attributes.
 */
INLINE bool RF4CE_ZRC2_IsArrayedAttribute(const uint8_t attrId)
{
    uint8_t  attrIdH =          /* High byte of the attribute identifier. */
            attrId & 0xF0;

    return (0x90 == attrIdH) || (0xC0 == attrIdH) || (0xD0 == attrIdH);
}

/**//**
 * \brief RF4CE ZRC 2.0 status values.
 */
typedef enum _RF4CE_ZRC2GDP2_Status_t
{
    RF4CE_ZRC2GDP2_SUCCESS                          = 0x00,
    RF4CE_ZRC2GDP2_UNSUPPORTED_REQUEST              = 0x01,
    RF4CE_ZRC2GDP2_UNSUPPORTED_ATTRIBUTE            = 0x01,
    RF4CE_ZRC2GDP2_INVALID_PARAMETER                = 0x02,
    RF4CE_ZRC2GDP2_CONFIGURATION_FAILURE            = 0x03,

    RF4CE_ZRC2GDP2_SYSTEM_IS_BUSY                   = 0x40,
    RF4CE_ZRC2GDP2_NO_MEMORY                        = 0x41,
    RF4CE_ZRC2GDP2_SET_ATTRIBUTE_ERROR_RX_ENABLE    = 0x42,
    RF4CE_ZRC2GDP2_SET_ATTRIBUTE_ERROR_SEND         = 0x43,
    RF4CE_ZRC2GDP2_SET_ATTRIBUTE_ERROR_TIMEOUT      = 0x44,

    RF4CE_ZRC2GDP2_RX_ENABLE_FAILURE                = 0x45,
    RF4CE_ZRC2GDP2_COMMAND_SEND_FAILURE             = 0x46,
    RF4CE_ZRC2GDP2_NO_PAIRING                       = 0x47,
    RF4CE_ZRC2GDP2_NOT_BOUND                        = 0x48,
    RF4CE_ZRC2GDP2_NOT_GDP2_PAIR                    = 0x49,
    RF4CE_ZRC2GDP2_POLL_NEGOTIATION_IN_PROGRESS     = 0x4A,
    RF4CE_ZRC2GDP2_POLLING_DISABLED                 = 0x4B,
    RF4CE_ZRC2GDP2_POLLING_METHOD_DISABLED          = 0x4C,
    RF4CE_ZRC2GDP2_POLLING_TRIGGER_DISABLED         = 0x4D,
    RF4CE_ZRC2GDP2_POLLING_RESPONSE_TIMEOUT         = 0x4E,
    RF4CE_ZRC2GDP2_CLIENT_NOTIFICATION_TIMEOUT      = 0x4F,
    RF4CE_ZRC2GDP2_IDENTIFY_NOT_CAPABLE             = 0x50,

    RF4CE_ZRC2GDP2_KEY_EXCHANGE_FAILURE             = 0x60,
    RF4CE_ZRC2GDP2_KEY_EXCHANGE_TIMEOUT             = 0x61,
    RF4CE_ZRC2GDP2_KEY_EXCHANGE_UNSUP_SECRET        = 0x62,

    RF4CE_ZRC2GDP2_BIND_ERROR_ABORTED               = 0x65,
    RF4CE_ZRC2GDP2_BIND_ERROR_PAIRING               = 0x66,
    RF4CE_ZRC2GDP2_BIND_ERROR_CONFIGURATION         = 0x67,
    RF4CE_ZRC2GDP2_BIND_ERROR_TIMEOUT               = 0x68,
    RF4CE_ZRC2GDP2_BIND_ERROR_NO_PAIRING_ENTRY      = 0x69,
    RF4CE_ZRC2GDP2_BIND_ERROR_NO_MEMORY             = 0x6A,
    RF4CE_ZRC2GDP2_BIND_ERROR_PRECONDITIONS         = 0x6B,
    RF4CE_ZRC2GDP2_BIND_ERROR_INVALID_REQUEST       = 0x6C,
    RF4CE_ZRC2GDP2_BIND_ERROR_VALIDATION_TIMEOUT    = 0x6D,
    RF4CE_ZRC2GDP2_BIND_ERROR_VALIDATION_LINK_LOST  = 0x6E,
    RF4CE_ZRC2GDP2_BIND_ERROR_VALIDATION_FAILED     = 0x6F

} RF4CE_ZRC2GDP2_Status_t;

/**//**
 * \brief RF4CE ZRC 2.0 Get Attribute status values.
 */
typedef enum _RF4CE_ZRC2GDP2_GetAttributeStatus_t
{
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_SUCCESS = 0,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_UNSUPPORTED_ATTRIBUTE = 1,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_ILLEGAL_REQUEST = 2,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_INVALID_ENTRY = 3,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_ERROR_RX_ENABLE = 0x40,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_ERROR_SEND,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_ERROR_TIMEOUT,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_ERROR_NO_MEMORY,
    RF4CE_ZRC2GDP2_GET_ATTRIBUTE_CASH_MISS,
} RF4CE_ZRC2GDP2_GetAttributeStatus_t;

/************************* TYPES *******************************************************/

/**//**
 * \brief RF4CE ZRC 2.0 bitmap of 256 bits.
 */
typedef uint8_t RF4CE_ZRC2_Bitmap256_t[256 >> 3];

/**//**
 * \brief RF4CE GDP 2.0 PollConfiguration value
 */
typedef uint8_t RF4CE_ZRC2_PollConfigurationValue_t[9];

/**//**
 * \brief RF4CE ZRC 2.0 aplMappableActions attribute entry type declaration
 */
typedef struct _RF4CE_ZRC2_MappableAction_t
{
    uint8_t deviceType;     /* set to RF4CE_ZRC2_ACTION_MAPPING_DEVICETYPE_INVALID to not use mapping */
    uint8_t bank;
    uint8_t code;
} RF4CE_ZRC2_MappableAction_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Id type.
 */
typedef struct _RF4CE_ZRC2_AttributeId_t
{
    uint8_t attributeId; /*!< The attribute ID - one of the RF4CE_ZRC2GDP2_AttributesID_t values */
    union
    {
        struct
        {
            uint8_t lo;
            uint8_t hi;
        } indexByte;
        uint16_t index;
    } index;             /*!< The attribute index for arrayed attributes */
} RF4CE_ZRC2_AttributeId_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get requests parameters.
 */
typedef struct _RF4CE_ZRC2_GetAttributesReqParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference. If it is equal to RF4CE_NWK_INVALID_PAIRING_REF then local attributes are retrieved */
    SYS_DataPointer_t payload; /*!< The payload must contain an array of RF4CE_ZRC2_AttributeId_t records */
} RF4CE_ZRC2_GetAttributesReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get requests confirmation identifier.
 */
typedef struct _RF4CE_ZRC2_GetAttributesConfId_t
{
    RF4CE_ZRC2_AttributeId_t id; /*!< The attribute's id */
    uint8_t status;              /*!< One of the RF4CE_ZRC2GDP2_GetAttributeStatus_t values */
    uint8_t size;                /*!< The attribute's value size - the real size of the value field */
    uint8_t value[1];            /*!< The attribute's value. The real size is indicated by size field */
} RF4CE_ZRC2_GetAttributesConfId_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get requests confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_GetAttributesConfParams_t
{
    uint8_t status;            /*!< Overall status of the operation. One of the RF4CE_ZRC2GDP2_GetAttributeStatus_t values */
    SYS_DataPointer_t payload; /*!< The payload must contain an array of aligned RF4CE_ZRC2_GetAttributesConfId_t records */
} RF4CE_ZRC2_GetAttributesConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get request descriptor definition.
 */
typedef struct _RF4CE_ZRC2_GetAttributesReqDescr_t RF4CE_ZRC2_GetAttributesReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get request callback.
 */
typedef void (*RF4CE_ZRC2_GetAttributesReqCallback_t)(RF4CE_ZRC2_GetAttributesReqDescr_t *req, RF4CE_ZRC2_GetAttributesConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Get request descriptor.
 */
struct _RF4CE_ZRC2_GetAttributesReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;             /*!< Service field */
    uint8_t pairingRef;                             /*!< Service field */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ZRC2_GetAttributesReqParams_t params;     /*!< Request parameters */
    RF4CE_ZRC2_GetAttributesReqCallback_t callback; /*!< Request callback */
};

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request attribute identifier.
 */
typedef struct PACKED _RF4CE_ZRC2_SetAttributeId_t
{
    RF4CE_ZRC2_AttributeId_t id; /*!< The attribute's id */
    uint8_t size;                /*!< The attribute's value size - the real size of the value field */
    uint8_t value[1];            /*!< The attribute's value. The real size is shown by the size field */
} RF4CE_ZRC2_SetAttributeId_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request parameters.
 */
typedef struct _RF4CE_ZRC2_SetAttributesReqParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference. If it is equal to RF4CE_NWK_INVALID_PAIRING_REF then local attributes are retrieved */
    SYS_DataPointer_t payload; /*!< The payload must contain an array of aligned RF4CE_ZRC2_SetAttributeId_t records */
} RF4CE_ZRC2_SetAttributesReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_SetAttributesConfParams_t
{
    uint8_t status;     /*!< The status of the operation: one of the RF4CE_ZRC2GDP2_Status_t values */
} RF4CE_ZRC2_SetAttributesConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request descriptor definition.
 */
typedef struct _RF4CE_ZRC2_SetAttributesReqDescr_t RF4CE_ZRC2_SetAttributesReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request callback.
 */
typedef void (*RF4CE_ZRC2_SetAttributesReqCallback_t)(RF4CE_ZRC2_SetAttributesReqDescr_t *req, RF4CE_ZRC2_SetAttributesConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Set request descriptor.
 */
struct _RF4CE_ZRC2_SetAttributesReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;             /*!< Service field */
    uint8_t pairingRef;                             /*!< Service field */
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ZRC2_SetAttributesReqParams_t params;     /*!< Request parameters */
    RF4CE_ZRC2_SetAttributesReqCallback_t callback; /*!< Request callback */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts asynchronous ZRC 2.0 Get Attributes Request.

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_GetAttributesReq(RF4CE_ZRC2_GetAttributesReqDescr_t *request);

/************************************************************************************//**
 \brief Starts asynchronous ZRC 2.0 Set Attributes Request.

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_SetAttributesReq(RF4CE_ZRC2_SetAttributesReqDescr_t *request);

/**//**
 * \brief   Indicates to the Host the received Get Attributes Response.
 * \param[in]   indParams   Pointer to the indication parameters.
 */
void RF4CE_ZRC2_GetAttrRespInd(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams);

/**//**
 * \brief   Indicates to the Host the received Push Attributes Request.
 * \param[in]   indParams   Pointer to the indication parameters.
 */
void RF4CE_ZRC2_PushAttrReqInd(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams);

#endif /* _RF4CE_ZRC_ATTRIBUTES_H */