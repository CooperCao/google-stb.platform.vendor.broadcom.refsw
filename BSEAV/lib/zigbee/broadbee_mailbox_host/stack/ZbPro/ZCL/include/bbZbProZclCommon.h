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
*
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclCommon.h $
*
* DESCRIPTION:
*   ZCL common definitions.
*
* $Revision: 9364 $
* $Date: 2016-01-04 18:21:51Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_COMMON_H
#define _BB_ZBPRO_ZCL_COMMON_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclConfig.h"
#include "bbZbProApsCommon.h"
#include "bbRpc.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of ZCL Statuses.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.5.3, table 2-10.
 */
typedef enum _ZBPRO_ZCL_Status_t
{
    ZBPRO_ZCL_SUCCESS                       = 0x00,     /*!< Operation was successful. */

    ZBPRO_ZCL_FAILURE                       = 0x01,     /*!< Operation was not successful. */

    /*                               0x02 ... 0x7D         < reserved. */

    ZBPRO_ZCL_NOT_AUTHORIZED                = 0x7E,     /*!< The sender of the command does not have authorization to
                                                            carry out this command. */

    ZBPRO_ZCL_RESERVED_FIELD_NOT_ZERO       = 0x7F,     /*!< A reserved field/subfield/bit contains a non-zero value. */

    ZBPRO_ZCL_MALFORMED_COMMAND             = 0x80,     /*!< The command appears to contain the wrong fields, as
                                                            detected either by the presence of one or more invalid field
                                                            entries or by there being missing fields. Command not
                                                            carried out. Implementer has discretion as to whether to
                                                            return this error or INVALID_FIELD. */

    ZBPRO_ZCL_UNSUP_CLUSTER_COMMAND         = 0x81,     /*!< The specified cluster command is not supported on the
                                                            device. Command not carried out. */

    ZBPRO_ZCL_UNSUP_GENERAL_COMMAND         = 0x82,     /*!< The specified general ZCL command is not sup-ported on the
                                                            device. */

    ZBPRO_ZCL_UNSUP_MANUF_CLUSTER_COMMAND   = 0x83,     /*!< A manufacturer specific unicast, cluster specific command
                                                            was received with an unknown manufacturer code, or the
                                                            manufacturer code was recognized but the command is not
                                                            supported. */

    ZBPRO_ZCL_UNSUP_MANUF_GENERAL_COMMAND   = 0x84,     /*!< A manufacturer specific unicast, ZCL specific command was
                                                            received with an unknown manufacturer code, or the
                                                            manufacturer code was recognized but the command is not
                                                            supported. */

    ZBPRO_ZCL_INVALID_FIELD                 = 0x85,     /*!< At least one field of the command contains an incorrect
                                                            value, according to the specification the device is
                                                            implemented to. */

    ZBPRO_ZCL_UNSUPPORTED_ATTRIBUTE         = 0x86,     /*!< The specified attribute does not exist on the device. */

    ZBPRO_ZCL_INVALID_VALUE                 = 0x87,     /*!< Out of range error, or set to a reserved value. Attribute
                                                            keeps its old value. Note that an attribute value may be out
                                                            of range if an attribute is related to another, e.g., with
                                                            minimum and maximum attributes. See the individual attribute
                                                            descriptions for specific details. */

    ZBPRO_ZCL_READ_ONLY                     = 0x88,     /*!< Attempt to write a read only attribute. */

    ZBPRO_ZCL_INSUFFICIENT_SPACE            = 0x89,     /*!< An operation (e.g., an attempt to create an entry in a
                                                            table) failed due to an insufficient amount of free space
                                                            available. */

    ZBPRO_ZCL_DUPLICATE_EXISTS              = 0x8A,     /*!< An attempt to create an entry in a table failed due to a
                                                            duplicate entry already being present in the table. */

    ZBPRO_ZCL_NOT_FOUND                     = 0x8B,     /*!< The requested information (e.g., table entry) could not be
                                                            found. */

    ZBPRO_ZCL_UNREPORTABLE_ATTRIBUTE        = 0x8C,     /*!< Periodic reports cannot be issued for this attribute. */

    ZBPRO_ZCL_INVALID_DATA_TYPE             = 0x8D,     /*!< The data type given for an attribute is incorrect. Command
                                                            not carried out. */

    ZBPRO_ZCL_INVALID_SELECTOR              = 0x8E,     /*!< The selector for an attribute is incorrect. */

    ZBPRO_ZCL_WRITE_ONLY                    = 0x8F,     /*!< A request has been made to read an attribute that the
                                                            requestor is not authorized to read. No action taken. */

    ZBPRO_ZCL_INCONSISTENT_STARTUP_STATE    = 0x90,     /*!< Setting the requested values would put the device in an
                                                            inconsistent state on startup. No action taken. */

    ZBPRO_ZCL_DEFINED_OUT_OF_BAND           = 0x91,     /*!< An attempt has been made to write an attribute that is
                                                            present but is defined using an out-of-band method and not
                                                            over the air. */

    ZBPRO_ZCL_INCONSISTENT                  = 0x92,     /*!< The supplied values (e.g., contents of table cells) are
                                                            inconsistent. */

    ZBPRO_ZCL_ACTION_DENIED                 = 0x93,     /*!< The credentials presented by the device sending the command
                                                            are not sufficient to perform this action. */

    ZBPRO_ZCL_TIMEOUT                       = 0x94,     /*!< The exchange was aborted due to excessive response time. */

    ZBPRO_ZCL_ABORT                         = 0x95,     /*!< Failed case when a client or a server decides to abort the
                                                            upgrade process. */

    ZBPRO_ZCL_INVALID_IMAGE                 = 0x96,     /*!< Invalid OTA upgrade image (ex. failed signature validation
                                                            or signer information check or CRC check). */

    ZBPRO_ZCL_WAIT_FOR_DATA                 = 0x97,     /*!< Server does not have data block available yet. */

    ZBPRO_ZCL_NO_IMAGE_AVAILABLE            = 0x98,     /*!< No OTA upgrade image available for a particular client. */

    ZBPRO_ZCL_REQUIRE_MORE_IMAGE            = 0x99,     /*!< The client still requires more OTA upgrade image files in
                                                            order to successfully upgrade. */

    ZBPRO_ZCL_NOTIFICATION_PENDING          = 0x9A,     /*!< The command has been received and is being processed. */

    /*                               0x9B ... 0xBF         < reserved. */

    ZBPRO_ZCL_HARDWARE_FAILURE              = 0xC0,     /*!< An operation was unsuccessful due to a hardware failure. */

    ZBPRO_ZCL_SOFTWARE_FAILURE              = 0xC1,     /*!< An operation was unsuccessful due to a software failure. */

    ZBPRO_ZCL_CALIBRATION_ERROR             = 0xC2,     /*!< An error occurred during calibration. */

    /*                               0xC3 ... 0xFF         < reserved. */

} ZBPRO_ZCL_Status_t;

/*
 * Validate size of ZCL Status data type.
 */
SYS_DbgAssertStatic(1 == sizeof(ZBPRO_ZCL_Status_t));


/**//**
 * \brief   Enumeration of ZCL Clusters.
 * \note
 *  Take into account that identifiers are listed in the original order according to the
 *  official specification; they are not sorted by their numeric values.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.1.2, 4.1.2, 5.1.2, 6.1.2, 7.1.2, 8.1.2,
 *  9.1.2, 10.1.2, 11.2.2, 12.1.2, 13.1.2, 14.1.2, 15.1.2, tables 3-1, 3-2, 3-3, 3-4, 3-5,
 *  3-6, 4-1, 4-2, 4-3, 4-4, 5-1, 6-1, 7-1, 8-1, 9-1, 10-1, 11-1, 12-1, 13-1, 14-1, 15-1.
 */
typedef enum _ZBPRO_ZCL_ClusterId_t
{
    /* General. Device Configuration and Installation. */

    ZBPRO_ZCL_CLUSTER_ID_BASIC                              = 0x0000,       /*!< Basic. */

    ZBPRO_ZCL_CLUSTER_ID_POWER_CONFIGURATION                = 0x0001,       /*!< Power configuration. */

    ZBPRO_ZCL_CLUSTER_ID_DEVICE_TEMPERATURE_CONFIGURATION   = 0x0002,       /*!< Device temperature configuration. */

    ZBPRO_ZCL_CLUSTER_ID_IDENTIFY                           = 0x0003,       /*!< Identify. */


    /* General. Groups and Scenes. */

    ZBPRO_ZCL_CLUSTER_ID_GROUPS                             = 0x0004,       /*!< Groups. */

    ZBPRO_ZCL_CLUSTER_ID_SCENES                             = 0x0005,       /*!< Scenes. */


    /* General. On/Off and Level Control. */

    ZBPRO_ZCL_CLUSTER_ID_ONOFF                              = 0x0006,       /*!< On/off. */

    ZBPRO_ZCL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION         = 0x0007,       /*!< On/off switch configuration. */

    ZBPRO_ZCL_CLUSTER_ID_LEVEL_CONTROL                      = 0x0008,       /*!< Level control. */


    /* General. Alarms. */

    ZBPRO_ZCL_CLUSTER_ID_ALARMS                             = 0x0009,       /*!< Alarms. */


    /* General. Other. */

    ZBPRO_ZCL_CLUSTER_ID_TIME                               = 0x000A,       /*!< Time. */

    ZBPRO_ZCL_CLUSTER_ID_RSSI_LOCATION                      = 0x000B,       /*!< RSSI Location. */

    ZBPRO_ZCL_CLUSTER_ID_DIAGNOSTICS                        = 0x0B05,       /*!< Diagnostics. */

    ZBPRO_ZCL_CLUSTER_ID_POLL_CONTROL                       = 0x0020,       /*!< Poll Control. */

    ZBPRO_ZCL_CLUSTER_ID_POWER_PROFILE                      = 0x001A,       /*!< Power Profile. */

    ZBPRO_ZCL_CLUSTER_ID_METER_IDENTIFICATION               = 0x0B01,       /*!< Meter Identification. */


    /* General. Generic. */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_INPUT_BASIC                 = 0x000C,       /*!< Analog input (basic). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_OUTPUT_BASIC                = 0x000D,       /*!< Analog output (basic). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_VALUE_BASIC                 = 0x000E,       /*!< Analog value (basic). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_INPUT_BASIC                 = 0x000F,       /*!< Binary input (basic). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_OUTPUT_BASIC                = 0x0010,       /*!< Binary output (basic). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_VALUE_BASIC                 = 0x0011,       /*!< Binary value (basic). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_INPUT_BASIC             = 0x0012,       /*!< Multistate input (basic). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_OUTPUT_BASIC            = 0x0013,       /*!< Multistate output (basic). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_VALUE_BASIC             = 0x0014,       /*!< Multistate value (basic). */


    /* Measurement and Sensing. Illuminance Measurement and Level Sensing. */

    ZBPRO_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT            = 0x0400,       /*!< Illuminance measurement. */

    ZBPRO_ZCL_CLUSTER_ID_ILLUMINANCE_LEVEL_SENSING          = 0x0401,       /*!< Illuminance level sensing. */


    /* Measurement and Sensing. Pressure and Flow Measurement. */

    ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT            = 0x0402,       /*!< Temperature measurement. */

    ZBPRO_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT               = 0x0403,       /*!< Pressure measurement. */

    ZBPRO_ZCL_CLUSTER_ID_FLOW_MEASUREMENT                   = 0x0404,       /*!< Flow measurement. */

    ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT      = 0x0405,       /*!< Relative Humidity measurement. */

    /* Measurement and Sensing. Occupancy Sensing. */

    ZBPRO_ZCL_CLUSTER_ID_OCCUPANCY_SENSING                  = 0x0406,       /*!< Occupancy sensing. */


    /* Measurement and Sensing. Electrical Measurement. */

    ZBPRO_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT             = 0x0B04,       /*!< Electrical Measurement. */


    /* Lighting. */

    ZBPRO_ZCL_CLUSTER_ID_COLOR_CONTROL                      = 0x0300,       /*!< Color Control. */

    ZBPRO_ZCL_CLUSTER_ID_BALLAST_CONFIGURATION              = 0x0301,       /*!< Ballast Configuration. */


    /* HVAC. */

    ZBPRO_ZCL_CLUSTER_ID_PUMP_CONFIGURATION_AND_CONTROL     = 0x0200,       /*!< Pump Configuration and
                                                                                        Control. */

    ZBPRO_ZCL_CLUSTER_ID_PUMP_CONFIGURATION_AND_CONTROL_MANUF_SPEC  =  0xfc01,   /*!< Pump Configuration and
                                                                                        Control manuf specific. */

    ZBPRO_ZCL_CLUSTER_ID_THERMOSTAT                         = 0x0201,       /*!< Thermostat. */

    ZBPRO_ZCL_CLUSTER_ID_FAN_CONTROL                        = 0x0202,       /*!< Fan Control. */

    ZBPRO_ZCL_CLUSTER_ID_DEHUMIDIFICATION_CONTROL           = 0x0203,       /*!< Dehumidification Control. */

    ZBPRO_ZCL_CLUSTER_ID_THERMOSTAT_USER_INTERFACE_CONFIG   = 0x0204,       /*!< Thermostat User Interface
                                                                                Configuration. */

    /* Closures. */

    ZBPRO_ZCL_CLUSTER_ID_SHADE_CONFIGURATION                = 0x0100,       /*!< Shade Configuration. */

    ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK                          = 0x0101,       /*!< Door Lock. */

    ZBPRO_ZCL_CLUSTER_ID_WINDOW_COVERING                    = 0x0102,       /*!< Window Covering. */


    /* Security and Safety. */

    ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE                           = 0x0500,       /*!< IAS Zone. */

    ZBPRO_ZCL_CLUSTER_ID_IAS_ACE                            = 0x0501,       /*!< IAS ACE. */

    ZBPRO_ZCL_CLUSTER_ID_IAS_WD                             = 0x0502,       /*!< IAS WD. */


    /* Protocol Interfaces. */

    ZBPRO_ZCL_CLUSTER_ID_PARTITION                          = 0x0016,       /*!< Partition. */

    ZBPRO_ZCL_CLUSTER_ID_GENERIC_TUNNEL                     = 0x0600,       /*!< Generic tunnel. */

    ZBPRO_ZCL_CLUSTER_ID_BACNET_PROTOCOL_TUNNEL             = 0x0601,       /*!< BACnet protocol tunnel. */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_INPUT_BACNET_REGULAR        = 0x0602,       /*!< Analog input (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_INPUT_BACNET_EXTENDED       = 0x0603,       /*!< Analog input (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_OUTPUT_BACNET_REGULAR       = 0x0604,       /*!< Analog output (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_OUTPUT_BACNET_EXTENDED      = 0x0605,       /*!< Analog output (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_VALUE_BACNET_REGULAR        = 0x0606,       /*!< Analog value (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_ANALOG_VALUE_BACNET_EXTENDED       = 0x0607,       /*!< Analog value (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_INPUT_BACNET_REGULAR        = 0x0608,       /*!< Binary input (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_INPUT_BACNET_EXTENDED       = 0x0609,       /*!< Binary input (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_OUTPUT_BACNET_REGULAR       = 0x060A,       /*!< Binary output (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_OUTPUT_BACNET_EXTENDED      = 0x060B,       /*!< Binary output (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_VALUE_BACNET_REGULAR        = 0x060C,       /*!< Binary value (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_BINARY_VALUE_BACNET_EXTENDED       = 0x060D,       /*!< Binary value (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_INPUT_BACNET_REGULAR    = 0x060E,       /*!< Multistate input (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_INPUT_BACNET_EXTENDED   = 0x060F,       /*!< Multistate input (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_OUTPUT_BACNET_REGULAR   = 0x0610,       /*!< Multistate output (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_OUTPUT_BACNET_EXTENDED  = 0x0611,       /*!< Multistate output (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_VALUE_BACNET_REGULAR    = 0x0612,       /*!< Multistate value (BACnet regular). */

    ZBPRO_ZCL_CLUSTER_ID_MULTISTATE_VALUE_BACNET_EXTENDED   = 0x0613,       /*!< Multistate value (BACnet extended). */

    ZBPRO_ZCL_CLUSTER_ID_11073_PROTOCOL_TUNNEL              = 0x0614,       /*!< 11073 Protocol Tunnel. */

    ZBPRO_ZCL_CLUSTER_ID_ISO7816_TUNNEL                     = 0x0615,       /*!< ISO7816 Tunnel. */


    /* Smart Energy. */

    ZBPRO_ZCL_CLUSTER_ID_PRICE                              = 0x0700,       /*!< Price. */

    ZBPRO_ZCL_CLUSTER_ID_DEMAND_RESPONSE_AND_LOAD_CONTROL   = 0x0701,       /*!< Demand Response and Load Control. */

    ZBPRO_ZCL_CLUSTER_ID_METERING                           = 0x0702,       /*!< Metering. */

    ZBPRO_ZCL_CLUSTER_ID_MESSAGING                          = 0x0703,       /*!< Messaging. */

    ZBPRO_ZCL_CLUSTER_ID_TUNNELING                          = 0x0704,       /*!< Tunneling. */

    ZBPRO_ZCL_CLUSTER_ID_KEY_ESTABLISHMENT                  = 0x0800,       /*!< Key Establishment. */


    /* Over-The-Air. */

    ZBPRO_ZCL_CLUSTER_ID_OTA_UPGRADE                        = 0x0019,       /*!< OTA Upgrade. */


    /* Telecom Applications. */

    ZBPRO_ZCL_CLUSTER_ID_INFORMATION                        = 0x0900,       /*!< Information. */

    ZBPRO_ZCL_CLUSTER_ID_CHATTING                           = 0x0905,       /*!< Chatting. */

    ZBPRO_ZCL_CLUSTER_ID_VOICE_OVER_ZIGBEE                  = 0x0904,       /*!< Voice Over ZigBee. */


    /* Commissioning. */

    ZBPRO_ZCL_CLUSTER_ID_COMMISSIONING                      = 0x0015,       /*!< Commissioning. */

    ZBPRO_ZCL_CLUSTER_ID_TOUCHLINK                          = 0x1000,       /*!< Touchlink. */


    /* Retail. */

    ZBPRO_ZCL_CLUSTER_ID_RETAIL_TUNNEL_CLUSTER              = 0x0617,       /*!< Retail Tunnel Cluster. */

    ZBPRO_ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIG_CLUSTER       = 0x0022,       /*!< Mobile Device Configuration Cluster. */

    ZBPRO_ZCL_CLUSTER_ID_NEIGHBOR_CLEANING_CLUSTER          = 0x0023,       /*!< Neighbor Cleaning Cluster. */

    ZBPRO_ZCL_CLUSTER_ID_NEAREST_GATEWAY_CLUSTER            = 0x0024,       /*!< Nearest Gateway Cluster. */


    /* Appliance Management. */

    ZBPRO_ZCL_CLUSTER_ID_EN50523_APPLIANCE_CONTROL          = 0x001B,       /*!< EN50523 Appliance Control. */

    ZBPRO_ZCL_CLUSTER_ID_EN50523_APPLIANCE_IDENTIFICATION   = 0x0B00,       /*!< EN50523 Appliance Identification. */

    ZBPRO_ZCL_CLUSTER_ID_EN50523_APPLIANCE_EVENTS_ALERTS    = 0x0B02,       /*!< EN50523 Appliance Events and Alerts. */

    ZBPRO_ZCL_CLUSTER_ID_EN50523_APPLIANCE_STATISTICS       = 0x0B03,       /*!< EN50523 Appliance Statistics. */

} ZBPRO_ZCL_ClusterId_t;

/*
 * Validate size of ZCL Cluster Id data type.
 */
SYS_DbgAssertStatic(2 == sizeof(ZBPRO_ZCL_ClusterId_t));


/**//**
 * \brief   Enumeration of ZCL Frame Types.
 * \note
 *  First two items (i.e., profile-wide and cluster-specific) are used as binary (1-bit
 *  width) values.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1.1, figures 2-3, 2-4.
 */
typedef enum _ZBPRO_ZCL_FrameType_t
{
    ZBPRO_ZCL_FRAME_TYPE_PROFILE_WIDE_COMMAND       = 0x0,      /*!< Command acts across the entire profile. Such a
                                                                    command may be (potentially) used in relation to any
                                                                    of ZCL Clusters; and in all cases it will act in the
                                                                    same way having common profile-specific behavior. */

    ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND   = 0x1,      /*!< Command is specific to a cluster. Such a command is
                                                                    defined in relation only to its native cluster. */

    ZBPRO_ZCL_FRAME_TYPE_RESERVED_2                 = 0x2,      /*!< Reserved code. */

    ZBPRO_ZCL_FRAME_TYPE_RESERVED_3                 = 0x3,      /*!< Reserved code. */

} ZBPRO_ZCL_FrameType_t;


/**//**
 * \brief   Enumeration of ZCL Frame Domains: ZCL Standard or Manufacturer Specific.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1.2, figure 2-3.
 */
typedef enum _ZBPRO_ZCL_FrameDomain_t
{
    ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD     = 0x0,      /*!< Command is standardized within ZCL. All such command are
                                                            defined by the official ZCL Specification. */

    ZBPRO_ZCL_FRAME_DOMAIN_MANUF_SPECIFIC   = 0x1,      /*!< Command refers to a manufacturer specific extension to a
                                                            profile. For the case of cluster-specific commands which are
                                                            at the same time manufacturer-specific, they may perform
                                                            arbitrary activities on discretion of the manufacturer. For
                                                            the case of profile-wide commands which are at the same time
                                                            manufacturer-specific, they still have ZCL-standardized
                                                            behavior but in relation to particular manufacturer-specific
                                                            extension of the cluster to which or from which such a
                                                            command is addressed. */
} ZBPRO_ZCL_FrameDomain_t;


/**//**
 * \brief   Enumeration of ZCL Frame Directions.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1.3, figure 2-3.
 */
typedef enum _ZBPRO_ZCL_FrameDirection_t
{
    ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER  = 0x0,      /*!< Command is being sent from the client side of a cluster
                                                                to the server side of a cluster. */

    ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT  = 0x1,      /*!< Command is being sent from the server side of a cluster
                                                                to the client side of a cluster. */
} ZBPRO_ZCL_FrameDirection_t;


/**//**
 * \brief   Enumeration of ZCL Cluster Sides.
 * \details
 *  This enumeration takes its origin from the frame Direction field of ZCL frame.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1.3, figure 2-3.
 */
typedef enum _ZBPRO_ZCL_ClusterSide_t
{
    ZBPRO_ZCL_CLUSTER_SIDE_CLIENT = ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,      /*!< Client side of cluster. */

    ZBPRO_ZCL_CLUSTER_SIDE_SERVER = ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,      /*!< Server side of cluster. */

} ZBPRO_ZCL_ClusterSide_t;


/**//**
 * \brief   Enumeration of ZCL Default Response Disabling (masking) statuses.
 * \details
 *  If not disabled, the Default Response frame is sent back in response on each frame
 *  that mets all of the following criteria simultaneously:
 *  - this command itself is not the Default Response,
 *  - this command was sent unicastly,
 *  - no other command was sent in response on this command (using the same Transaction
 *      sequence number),
 *  - there are no rules specific to this particular command that prohibits the Default
 *      Response on it.
 *
 *  If disabled, the next condition is added to the previous list:
 *  - status returned on this command is not SUCCESS (i.e., one of error results).
 *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1.4, figure 2-3.
 */
typedef enum _ZBPRO_ZCL_DisableDefaultResp_t
{
    ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED    = 0x0,      /*!< Default Response frame will be returned on this frame
                                                                regardless of the status. */

    ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_DISABLED   = 0x1,      /*!< Default Response frame will be returned on this frame
                                                                only in the case of error. */
} ZBPRO_ZCL_DisableDefaultResp_t;


/**//**
 * \brief   Enumeration of ZCL Response Types: Specific and Default.
 */
typedef enum _ZBPRO_ZCL_ResponseType_t
{
    ZBPRO_ZCL_RESPONSE_TYPE_SPECIFIC    = FALSE,        /*!< Use specific response command. */

    ZBPRO_ZCL_RESPONSE_TYPE_DEFAULT     = TRUE,         /*!< Use ZCL Default Response command instead of specific. */

} ZBPRO_ZCL_ResponseType_t;


/**//**
 * \brief   Data type for ZCL Manufacturer Code.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, 2.3.1.2, figure 2-2.
 */
typedef ZBPRO_APS_ManufacturerCode_t  ZBPRO_ZCL_ManufCode_t;

/*
 * Validate size of ZCL Manufacturer Code Field data type.
 */
SYS_DbgAssertStatic(2 == sizeof(ZBPRO_ZCL_ManufCode_t));


/**//**
 * \brief   Data type for ZCL Transaction Sequence Number.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, 2.3.1.3, figure 2-2.
 */
typedef uint8_t  ZBPRO_ZCL_TransSeqNum_t;


/**//**
 * \brief   Data type for ZCL Command identifier.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, 2.3.1.4, figure 2-2.
 */
typedef uint8_t  ZBPRO_ZCL_CommandId_t;


/**//**
 * \brief   Data type for ZCL response waiting Timeout, in seconds.
 * \details
 *  For each requested transmission of a command, application specifies timeout of waiting
 *  for response (either specified or default). If no response is received during the
 *  specified timeout, the TIMEOUT status is confirmed.
 * \details
 *  Special value 0xFFFF instructs ZCL Dispatcher to use the default timeout.
 * \details
 *  Special value 0x0000 instructs ZCL Dispatcher not to wait for response. If APS layer
 *  just has succeeded to transmit, then SUCCESS status is confirmed immediately. This
 *  value used only internally by ZCL Dispatcher. If timeout is assigned with 0x0000 from
 *  the outside of ZCL Dispatcher, it's replaced with 0xFFFF value - consequently, 0x0000
 *  when specified externally also stands for using the default timeout.
 * \note
 *  For some specific commands there may be implemented different logic of waiting for
 *  response, especially for the case of multiple responses (when each response is
 *  delivered to application with separate indication).
 */
typedef uint16_t  ZBPRO_ZCL_Timeout_t;


/**//**
 * \brief   Special value for ZCL Timeout when ZCL Dispatcher shall use the default
 *  response waiting timeout.
 */
#define ZBPRO_ZCL_TIMEOUT_USE_DEFAULT       0xFFFF


/**//**
 * \brief   Structure for obligatory parameters of all ZCL public interface primitives.
 * \details
 *  This structure defines the obligatory set of ZCL public primitive parameters and there
 *  relative order to each other.
 * \details
 *  This structure is used as a complex parameter supplied by local node application
 *  acting as the command originator within ZCL Local Request/Response parameters, and
 *  inversely by ZCL layer of recipient node within ZCL Local Indication/Confirm
 *  parameters when notifying local application.
 * \note
 *  Take into account that the Profile Identifier (as well as the Instance Identifier) is
 *  not included into this structure. Profile (and Instance also) is determined by APS
 *  layer automatically from the \c localEndpoint identifier (on both sides, originator
 *  and recipient as well).
 */
typedef struct _ZbProZclLocalPrimitiveObligatoryPart_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_APS_Address_t             remoteApsAddress;
            /*!< Addressing mode and Address of the remote node. For the case of outgoing commands generated by this
                node, this field specifies destination address of the recipient node or group. For the case of incoming
                commands received by this node, this field specifies source address of the command originator; it may be
                then used as the destination address for the response command. */

    ZBPRO_APS_Address_t             localApsAddress;
            /*!< Addressing mode and Address of the local node. This field is used only for the case of incoming frames.
                For the case of incoming commands received by this node, this field specifies the original destination
                addressing mode and address (or group identifier) which this command was sent to; it may be used by the
                local node to distinguish unicast and nonunicast incoming commands. This field is not used for the case
                of outgoing commands; in this case APS layer automatically specifies the local address of this
                (originating) node. */

    /* 16-bit data. */

    ZBPRO_ZCL_ClusterId_t           clusterId;
            /*!< ZCL Cluster identifier. This field must be assigned with the identifier of the cluster to which this
                command is to be applied. Cluster-specific commands may be applied only to their parent cluster, so this
                parameter may be omitted for them when called by the higher layer (it will be assigned automatically);
                while Profile-wide commands may be applied to arbitrary ZCL-based cluster and in this case this
                parameter must be specified. */

    ZBPRO_ZCL_ManufCode_t           manufCode;
            /*!< Manufacturer Code for manufacturer specific frames. This parameter is valid only if \c manufSpecific is
                assigned with TRUE; otherwise it's ignored. */

    ZBPRO_ZCL_Timeout_t             respWaitTimeout;
            /*!< Timeout of waiting for response, in seconds. This field is used only in ZCL Local Request and Response
                primitives. It instructs ZCL layer how long it shall wait for specific or default response on outgoing
                command commenced with this Request or Response, in seconds. Value 0xFFFF instructs ZCL Dispatcher to
                use default ZCL Timeout (note that such default value may be different for different commands). For
                particular cases ZCL Dispatcher may override user assignments with preprogrammed values. Value 0x0000 is
                not used and substituted with 0xFFFF if specified externally in parameters of new Local Request (notice
                that internal usage of this value by ZCL Dispatcher is different: it stands for instruction not to wait
                for response; this instruction may not be given to ZCL Dispatcher externally with this parameter but it
                is generated internally by ZCL Dispatcher to itself in particular cases ). */

    /* 8-bit data. */

    ZBPRO_APS_EndpointId_t          remoteEndpoint;
            /*!< Identifier of Endpoint on the remote node. This field is equivalent to destination endpoint for
                outgoing commands and to source endpoint for incoming commands. */

    ZBPRO_APS_EndpointId_t          localEndpoint;
            /*!< Identifier of Endpoint on the local node. This field is equivalent to source endpoint for outgoing
                commands and to destination endpoint for incoming commands. For the case when incoming command has
                broadcast DstEndpoint (0xFF) or Indirect (0x0) or Groupcast (0x1) DstAddrMode, this parameter is
                assigned by APS layer with identifier of concrete local endpoint to which this frame is indicated now,
                in the range from 0x01 to 0xFE; and in the case of unicast DstEndpoint (from 0x01 to 0xFE) with Short
                (0x2) or Extended (0x3) DstAddrMode this parameter is assigned with value of the DstEndpoint of the
                received frame. */

    ZBPRO_ZCL_CommandId_t           commandId;
            /*!< Command identifier 8-bit value local to particular cluster, side and manufacturer of the command to be
                issued, in the case of Local Request, or of the command received, in the cases of Local Confirm and
                Local Indication. When Default Response is to be issued or is received, this field contains the
                parameter \c CommandId of the Default Response command, i.e. identifier of the command to which this
                Default Response is related, but not the Default Response command own identifier (0x0B). */

    ZBPRO_ZCL_TransSeqNum_t         transSeqNum;
            /*!< ZCL layer transaction 8-bit sequence number. This field is used in all ZCL Local primitives except the
                Request (when request is generated the transaction sequence number doesn't exist yet). Transaction
                sequence number is reported by local ZCL layer of command recipient node (mostly ZCL Server) to its
                application in ZCL Local Indication primitive; if application should respond, it issues ZCL Local
                Response to its ZCL layer and specifies this transaction sequence number. Transaction sequence number is
                reported also by local ZCL layer of command originator node to its application in ZCL Local Confirm
                primitive; by this way application (mostly ZCL Client) is informed with the identifier of transaction
                started by its ZCL layer; it may be used for filtering incoming responses if they are delivered to this
                client application with individual Local Indications (for the case of multiple response operations, for
                example); note that response still may arrive prior to the APS-ACK and due to this reason response may
                be indicated earlier than the corresponding request transmission is confirmed. */

    ZBPRO_ZCL_Status_t              overallStatus;
            /*!< The overall status of operation execution. This field is used only in ZCL Local Response and Confirm
                primitives. With this field, in Local Response, the server node application specifies status of the
                requested operation execution; this status then is reported (if needed/allowed) by the server node ZCL
                layer via the media to the client node. And in Local Confirm, the local client ZCL layer reports to its
                application the overall status of the requested operation. */

    /* 8-bit data / 1-bit flags. */

    ZBPRO_ZCL_FrameDirection_t      direction;
            /*!< Command direction; either Client-to-Server (0x0) or Server-to-Client (0x1). For the case of new Local
                Request this field defines direction of the requested command. But for the case of Local Indication this
                field defines direction opposite to that one of the received command; it is done to simplify process of
                assigning parameters of Local Response on such Indication - i.e., this field may be simply copied from
                the Indication parameters to the Response parameters without need to be inverted by the higher layer. */

    ZBPRO_ZCL_FrameType_t           clusterSpecific;
            /*!< Frame type; either Profile-Wide command (0x0) or Cluster Specific command (0x1). */

    ZBPRO_ZCL_FrameDomain_t         manufSpecific;
            /*!< Frame domain; either ZCL Standard (0x0) or Manufacturer Specific (0x1). */

    Bool8_t                         useSpecifiedTsn;
            /*!< TRUE instigates ZCL layer to use the Transaction Sequence Number (TSN) specified with \c transSeqNum
                parameter instead of automatically generated one (and avoid generating of it). This flag is set to TRUE
                in parameters of ZCL Response, to instruct ZCL Dispatcher to use TSN of the received command (that was
                reported with ZCL Indication). For the case of ZCL Indication and ZCL Confirm this parameter is
                automatically assigned to TRUE by the stack. FALSE instigates ZCL Dispatcher to automatically generate
                new TSN; it is the case of new ZCL Request. */

    ZBPRO_ZCL_DisableDefaultResp_t  disableDefaultResp;
            /*!< Disable Default Response field; either Default Response is Disabled (0x1) for successful status or
                Enabled (0x0). This field is used only for outgoing commands. By default it's set to Enabled (0x0). */

    ZBPRO_ZCL_ResponseType_t        useDefaultResponse;
            /*!< For outgoing commands: TRUE if the Default Response command must be issued instead of Specific
                Response. For incoming commands: TRUE if Default Response was received. In both cases \c CommandId holds
                not the Default Response command identifier (0x0B) but the value of CommandId field of such Default
                Response (i.e., the identifier of a command on which this Default Response is issued). */

    Bool8_t                         indNonUnicastRequest;
            /*!< TRUE if received command (produced this Local Indication or Local Confirm) was sent to nonunicast
                destination address or broadcast destination endpoint. Destination address is considered to be unicast
                if DstAddrMode is Short (0x2) or Extended (0x3), and DstAddress is one of unicast addresses (i.e., under
                0xFFF8 for the short addressing and under 0xFFFFFFFFFFFFFFFB for extended addressing). For all other
                cases (including groupcast mode) destination address is considered to be nonunicast. Note that there are
                no means to distinguish actually nonunicast ZCL transmission performed as series of distinct unicast APS
                transmissions (each with its own different APS S/N) via multiple-destination binding, for example.
                Broadcast destination endpoint has identifier 0xFF. This field is assigned for Local Indication and
                Confirm parameters according to the received frame parameters; it's used only for outgoing Default
                Responses (i.e., when \c useDefaultResponse equals to TRUE) to instruct ZCL Dispatcher to abort issuing
                of the Default Response on command if it was sent on nonunicast destination address. */

    Bool8_t                         indDisableDefaultResp;
            /*!< Stored value of the Disable Default Response field of the received ZCL command frame; either Default
                Response was Disabled (0x1) for successful response status or Enabled (0x0). This field is assigned for
                Local Indication and Confirm parameters according to the received frame parameters; it's used only for
                outgoing Default Responses (i.e., when \c useDefaultResponse equals to TRUE) to instruct ZCL Dispatcher
                to abort issuing of the Default Response with SUCCESS status. */

} ZbProZclLocalPrimitiveObligatoryPart_t;


/**//**
 * \brief   Structure for Prototype of Parameters of ZCL Local Primitive of any type:
 *  Request, Confirm, Indication, Response.
 * \details
 *  This structure defines the obligatory set of ZCL Local Primitive parameters and their
 *  relative order to each other and to other custom parameters of particular primitive.
 */
typedef struct _ZbProZclLocalPrimitiveParamsPrototype_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */

    /* In specific requests there custom parameters must follow the obligatory ones. */

} ZbProZclLocalPrimitiveParamsPrototype_t;


/**//**
 * \brief   Validates presence and offset of the obligatory parameters within custom ZCL
 *  Local Primitive Parameters structure.
 * \param[in]   paramsStruct    Literal identifier of ZCL Local primitive parameters
 *  structure to be validated.
 * \details
 *  If the specified structure is not valid, this macro produces a compile-time error.
 */
#define ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(paramsStruct)\
        SYS_DbgAssertStatic(\
                offsetof(paramsStruct, zclObligatoryPart) ==\
                offsetof(ZbProZclLocalPrimitiveParamsPrototype_t, zclObligatoryPart))


/**//**
 * \brief   Structure for Prototype of Descriptor of ZCL Local Primitive.
 * \details
 *  This structure defines standard offset (the relative position to the origin) of
 *  the obligatory \c params.zclObligatoryPart subfield within ZCL Local Primitive
 *  Descriptor structure for all primitives compatible with this ZCL Dispatcher.
 * \note
 *  ZCL Responses issued by the local application to its ZCL layer via the mechanism of
 *  request-confirms (but not responses indeed). Consequently, all the so-called ZCL
 *  Responses also use this prototype as well as ZCL Requests.
 */
typedef struct _ZbProZclLocalPrimitiveDescrPrototype_t  ZbProZclLocalPrimitiveDescrPrototype_t;


/**//**
 * \brief   Data type for Prototype of ZCL Local Confirmation callback function.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 * \note
 *  ZCL Responses issued by the local application to its ZCL layer via the mechanism of
 *  request-confirms (but not responses indeed). Consequently, all the so-called ZCL
 *  Responses are also confirmed as well as ZCL Requests.
 */
typedef void ZbProZclLocalPrimitiveCallbackPrototype_t(
                ZbProZclLocalPrimitiveDescrPrototype_t  *const  reqDescr,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  confParams);


/**//**
 * \brief   Structure for the Service Field of ZCL Local Primitive Descriptor object.
 * \details
 *  This structure shall be embedded as a service field into each particular ZCL Local
 *  Primitive Descriptor structure.
 * \details
 *  ZCL Responses issued by the local application to its ZCL layer via the mechanism of
 *  request-confirms (but not responses indeed). Consequently, all the so-called ZCL
 *  Responses also use this type as well as ZCL Requests.
 */
typedef RpcLocalRequest_t  ZbProZclLocalPrimitiveDescrService_t;


/**//**
 * \brief   Structure for Prototype of Descriptor of ZCL Local Primitive.
 * \details
 *  This structure defines standard offset (the relative position to the origin) of
 *  the obligatory \c params.zclObligatoryPart subfield within ZCL Local Primitive
 *  Descriptor structure for all primitives compatible with this ZCL Dispatcher.
 * \note
 *  ZCL Responses issued by the local application to its ZCL layer via the mechanism of
 *  request-confirms (but not responses indeed). Consequently, all the so-called ZCL
 *  Responses also use this prototype as well as ZCL Requests.
 */
struct _ZbProZclLocalPrimitiveDescrPrototype_t
{
    /* 32-bit data. */

    ZbProZclLocalPrimitiveCallbackPrototype_t *callback;        /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t       service;         /*!< ZCL Primitive Descriptor service field. */

    ZbProZclLocalPrimitiveParamsPrototype_t    params;          /*!< ZCL Primitive Parameters structure prototype. */
};


/**//**
 * \brief   Validates presence and offset of the obligatory parameters within custom ZCL
 *  Local Primitive Descriptor structure.
 * \param[in]   descrStruct     Literal identifier of ZCL Local Primitive Descriptor
 *  structure to be validated.
 * \details
 *  If the specified structure is not valid, this macro produces a compile-time error.
 */
#define ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(descrStruct)\
        SYS_DbgAssertStatic(\
                offsetof(descrStruct, params.zclObligatoryPart) ==\
                offsetof(ZbProZclLocalPrimitiveDescrPrototype_t, params.zclObligatoryPart))


#endif /* _BB_ZBPRO_ZCL_COMMON_H */
