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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclSapClusterBasic.h $
*
* DESCRIPTION:
*   ZCL Basic cluster SAP interface.
*
* $Revision: 7434 $
* $Date: 2015-07-10 10:06:56Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_BASIC_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_BASIC_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Basic ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Basic ZCL cluster has no attributes provided by Client side.
 * \note
 *  This implementation of Basic ZCL cluster doesn't provide Client side; and on the
 *  Server side it doesn't implement any of the optional attributes, only the following
 *  mandatory attributes are implemented:
 *  - ZCLVersion,
 *  - PowerSource.
 *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.2.2.2, 3.2.3, table 3-7.
 */
typedef enum _ZBPRO_ZCL_SapBasicServerAttributeId_t
{
    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_ZCL_VERSION             = 0x0000,       /*!< ZCLVersion. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_APPLICATION_VERSION     = 0x0001,       /*!< ApplicationVersion. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_STACK_VERSION           = 0x0002,       /*!< StackVersion. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_HW_VERSION              = 0x0003,       /*!< HWVersion. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_MANUFACTURER_NAME       = 0x0004,       /*!< ManufacturerName. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_MODEL_IDENTIFIER        = 0x0005,       /*!< ModelIdentifier. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_DATE_CODE               = 0x0006,       /*!< DateCode. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_POWER_SOURCE            = 0x0007,       /*!< PowerSource. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_LOCATION_DESCRIPTION    = 0x0010,       /*!< LocationDescription. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_PHYSICAL_ENVIRONMENT    = 0x0011,       /*!< PhysicalEnvironment. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_DEVICE_ENABLED          = 0x0012,       /*!< DeviceEnabled. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_ALARM_MASK              = 0x0013,       /*!< AlarmMask. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_DISABLE_LOCAL_CONFIG    = 0x0014,       /*!< DisableLocalConfig. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_SW_BUILD_ID             = 0x4000,       /*!< SWBuildID. */

    ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_MAX                     = 0xFFFF,       /*!< Maximum supported value for attribute
                                                                            identifier. Defined only to make the
                                                                            enumeration 16-bit wide. Do not delete. */
} ZBPRO_ZCL_SapBasicServerAttributeId_t;


/**//**
 * \name    Data types of attributes of Basic ZCL cluster.
 * \details
 *  All the listed attributes are transferred by value.
 * \details
 *  The following attributes may be used with corresponding enumerations for their values:
 *  - PowerSource,
 *  - PhysicalEnvironment,
 *  - DeviceEnabled.
 *
 * \details
 *  The following attributes are bitmasks and may be used with corresponding enumerations
 *  for their particular bits and their values:
 *  - AlarmMask,
 *  - DisableLocalConfig.
 *
 * \details
 *  Following attributes having variable size are transferred by means of payload in
 *  dynamic or static memory:
 *  - ManufacturerName,
 *  - ModelIdentifier,
 *  - DateCode,
 *  - LocationDescription,
 *  - SWBuildID.
 *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2, table 3-7.
 */

/**@{*/

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrZclVersion_t;                /*!< ZCLVersion. */

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrApplicationVersion_t;        /*!< ApplicationVersion. */

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrStackVersion_t;              /*!< StackVersion. */

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrHwVersion_t;                 /*!< HWVersion. */

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrPowerSource_t;               /*!< PowerSource. */

typedef uint8_t      ZBPRO_ZCL_SapBasicAttrPhysicalEnvironment_t;       /*!< PhysicalEnvironment. */

typedef Bool8_t      ZBPRO_ZCL_SapBasicAttrDeviceEnabled_t;             /*!< DeviceEnabled. */

typedef BitField8_t  ZBPRO_ZCL_SapBasicAttrAlarmMask_t;                 /*!< AlarmMask. */

typedef BitField8_t  ZBPRO_ZCL_SapBasicAttrDisableLocalConfig_t;        /*!< DisableLocalConfig. */

/**@}*/


/**//**
 * \brief   Enumeration of values of PowerSource attribute of Basic ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.8, table 3-8.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrPowerSourceId_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_UNKNOWN                               = 0x00,     /*!< Unknown. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_MAINS_SINGLE_PHASE                    = 0x01,     /*!< Mains (single
                                                                                                phase). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_MAINS_THREE_PHASE                     = 0x02,     /*!< Mains (3 phase). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_BATTERY                               = 0x03,     /*!< Battery. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_DC_SOURCE                             = 0x04,     /*!< DC source. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_EMERGENCY_MAINS_CONSTANTLY_POWERED    = 0x05,     /*!< Emergency mains
                                                                                                constantly powered. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_POWER_SOURCE_EMERGENCY_MAINS_AND_TRANSFER_SWITCH   = 0x06,     /*!< Emergency mains and
                                                                                                transfer switch. */
} ZBPRO_ZCL_SapBasicAttrPowerSourceId_t;


/**//**
 * \brief   Enumeration of values of PhysicalEnvironment attribute of Basic ZCL cluster.
 * \note
 *  Identifiers for Office (0x24) and LivingRoom (0x39) are renamed to Office_2 and
 *  LivingRoom_2 respectively in order to distinguish them from the same identifiers but
 *  with different values Office (0x0B) and LivingRoom (0x2E).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.10, table 3-9.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrPhysicalEnvironmentId_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_UNSPECIFIED_ENVIRONMENT    = 0x00,        /*!< Unspecified
                                                                                                environment. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MIRROR                     = 0x01,        /*!< Mirror (ZSE
                                                                                                Profile). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ATRIUM                     = 0x01,        /*!< Atrium. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BAR                        = 0x02,        /*!< Bar. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_COURTYARD                  = 0x03,        /*!< Courtyard. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BATHROOM                   = 0x04,        /*!< Bathroom. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BEDROOM                    = 0x05,        /*!< Bedroom. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BILLIARD_ROOM              = 0x06,        /*!< Billiard Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_UTILITY_ROOM               = 0x07,        /*!< Utility Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_CELLAR                     = 0x08,        /*!< Cellar. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_STORAGE_CLOSET             = 0x09,        /*!< Storage Closet. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_THEATER                    = 0x0A,        /*!< Theater. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OFFICE                     = 0x0B,        /*!< Office. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DECK                       = 0x0C,        /*!< Deck. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DEN                        = 0x0D,        /*!< Den. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DINING_ROOM                = 0x0E,        /*!< Dining Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ELECTRICAL_ROOM            = 0x0F,        /*!< Electrical Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ELEVATOR                   = 0x10,        /*!< Elevator. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ENTRY                      = 0x11,        /*!< Entry. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FAMILY_ROOM                = 0x12,        /*!< Family Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MAIN_FLOOR                 = 0x13,        /*!< Main Floor. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_UPSTAIRS                   = 0x14,        /*!< Upstairs. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DOWNSTAIRS                 = 0x15,        /*!< Downstairs. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BASEMENT_LOWER_LEVEL       = 0x16,        /*!< Basement/Lower
                                                                                                Level. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GALLERY                    = 0x17,        /*!< Gallery. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GAME_ROOM                  = 0x18,        /*!< Game Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GARAGE                     = 0x19,        /*!< Garage. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GYM                        = 0x1A,        /*!< Gym. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_HALLWAY                    = 0x1B,        /*!< Hallway. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_HOUSE                      = 0x1C,        /*!< House. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_KITCHEN                    = 0x1D,        /*!< Kitchen. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_LAUNDRY_ROOM               = 0x1E,        /*!< Laundry Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_LIBRARY                    = 0x1F,        /*!< Library. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MASTER_BEDROOM             = 0x20,        /*!< Master Bedroom. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MUD_ROOM                   = 0x21,        /*!< Mud Room (small room
                                                                                                for coats and boots). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_NURSERY                    = 0x22,        /*!< Nursery. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_PANTRY                     = 0x23,        /*!< Pantry. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OFFICE_2                   = 0x24,        /*!< Office. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OUTSIDE                    = 0x25,        /*!< Outside. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_POOL                       = 0x26,        /*!< Pool. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_PORCH                      = 0x27,        /*!< Porch. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SEWING_ROOM                = 0x28,        /*!< Sewing Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SITTING_ROOM               = 0x29,        /*!< Sitting Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_STAIRWAY                   = 0x2A,        /*!< Stairway. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_YARD                       = 0x2B,        /*!< Yard. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ATTIC                      = 0x2C,        /*!< Attic. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_HOT_TUB                    = 0x2D,        /*!< Hot Tub. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_LIVING_ROOM                = 0x2E,        /*!< Living Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SAUNA                      = 0x2F,        /*!< Sauna. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SHOP_WORKSHOP              = 0x30,        /*!< Shop/Workshop. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GUEST_BEDROOM              = 0x31,        /*!< Guest Bedroom. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GUEST_BATH                 = 0x32,        /*!< Guest Bath. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_POWDER_ROOM                = 0x33,        /*!< Powder Room (1/2
                                                                                                bath). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BACK_YARD                  = 0x34,        /*!< Back Yard. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FRONT_YARD                 = 0x35,        /*!< Front Yard. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_PATIO                      = 0x36,        /*!< Patio. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DRIVEWAY                   = 0x37,        /*!< Driveway. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SUN_ROOM                   = 0x38,        /*!< Sun Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_LIVING_ROOM_2              = 0x39,        /*!< Living Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SPA                        = 0x3A,        /*!< Spa. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_WHIRLPOOL                  = 0x3B,        /*!< Whirlpool. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SHED                       = 0x3C,        /*!< Shed. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_EQUIPMENT_STORAGE          = 0x3D,        /*!< Equipment Storage. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_HOBBY_CRAFT_ROOM           = 0x3E,        /*!< Hobby/Craft Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FOUNTAIN                   = 0x3F,        /*!< Fountain. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_POND                       = 0x40,        /*!< Pond. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_RECEPTION_ROOM             = 0x41,        /*!< Reception Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BREAKFAST_ROOM             = 0x42,        /*!< Breakfast Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_NOOK                       = 0x43,        /*!< Nook. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_GARDEN                     = 0x44,        /*!< Garden. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BALCONY                    = 0x45,        /*!< Balcony. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_PANIC_ROOM                 = 0x46,        /*!< Panic Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TERRACE                    = 0x47,        /*!< Terrace. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_ROOF                       = 0x48,        /*!< Roof. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TOILET                     = 0x49,        /*!< Toilet. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TOILET_MAIN                = 0x4A,        /*!< Toilet Main. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OUTSIDE_TOILET             = 0x4B,        /*!< Outside Toilet. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_SHOWER_ROOM                = 0x4C,        /*!< Shower room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_STUDY                      = 0x4D,        /*!< Study. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FRONT_GARDEN               = 0x4E,        /*!< Front Garden. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BACK_GARDEN                = 0x4F,        /*!< Back Garden. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_KETTLE                     = 0x50,        /*!< Kettle. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TELEVISION                 = 0x51,        /*!< Television. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_STOVE                      = 0x52,        /*!< Stove. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MICROWAVE                  = 0x53,        /*!< Microwave. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TOASTER                    = 0x54,        /*!< Toaster. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_VACUUM                     = 0x55,        /*!< Vacuum. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_APPLIANCE                  = 0x56,        /*!< Appliance. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FRONT_DOOR                 = 0x57,        /*!< Front Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_BACK_DOOR                  = 0x58,        /*!< Back Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FRIDGE_DOOR                = 0x59,        /*!< Fridge Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MEDICATION_CABINET_DOOR    = 0x60,        /*!< Medication Cabinet
                                                                                                Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_WARDROBE_DOOR              = 0x61,        /*!< Wardrobe Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_FRONT_CUPBOARD_DOOR        = 0x62,        /*!< Front Cupboard Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OTHER_DOOR                 = 0x63,        /*!< Other Door. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_WAITING_ROOM               = 0x64,        /*!< Waiting Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_TRIAGE_ROOM                = 0x65,        /*!< Triage Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DOCTORS_OFFICE             = 0x66,        /*!< Doctor’s Office. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_PATIENTS_PRIVATE_ROOM      = 0x67,        /*!< Patient’s Private
                                                                                                Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_CONSULTATION_ROOM          = 0x68,        /*!< Consultation Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_NURSE_STATION              = 0x69,        /*!< Nurse Station. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_WARD                       = 0x6A,        /*!< Ward. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_CORRIDOR                   = 0x6B,        /*!< Corridor. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_OPERATING_THEATRE          = 0x6C,        /*!< Operating Theatre. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DENTAL_SURGERY_ROOM        = 0x6D,        /*!< Dental Surgery Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_MEDICAL_IMAGING_ROOM       = 0x6E,        /*!< Medical Imaging
                                                                                                Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_DECONTAMINATION_ROOM       = 0x6F,        /*!< Decontamination
                                                                                                Room. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_PHYSICAL_ENVIRONMENT_UNKNOWN_ENVIRONMENT        = 0xFF,        /*!< Unknown environment. */

} ZBPRO_ZCL_SapBasicAttrPhysicalEnvironmentId_t;


/**//**
 * \brief   Enumeration of values of DeviceEnabled attribute of Basic ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.11, table 3-10.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrDeviceEnabledId_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_DEVICE_DISABLED    = 0x00,     /*!< Disabled. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_DEVICE_ENABLED     = 0x01,     /*!< Enabled. */

} ZBPRO_ZCL_SapBasicAttrDeviceEnabledId_t;


/**//**
 * \brief   Enumeration of bits comprised in AlarmMask bitmask attribute of Basic ZCL
 *  cluster.
 * \details
 *  This attribute has width of 8 bits. Bits with numbers from 2 to 7 (starting with 0)
 *  have no special names and may be used by ZCL-based profile.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.12, table 3-11.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrAlarmMaskBit_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_ALARM_BIT_GENERAL_HARDWARE_FAULT   = 0,    /*!< General hardware fault alarm source. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_ALARM_BIT_GENERAL_SOFTWARE_FAULT   = 1,    /*!< General software fault alarm source. */

} ZBPRO_ZCL_SapBasicAttrAlarmMaskBit_t;


/**//**
 * \brief   Enumeration of values of single bit of AlarmMask attribute of Basic ZCL
 *  cluster.
 * \note
 *  This enumeration provides values for single bit without taking into account bit
 *  position within the bitmask representing the attribute. One shall shift these values
 *  to appropriate number of bits to come with the absolute value of corresponding bit.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.12, table 3-11.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrAlarmMaskId_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_ALARM_DISABLED     = 0x00,     /*!< Alarm source disabled (masked). */

    ZBPRO_ZCL_SAP_BASIC_ATTR_ALARM_ENABLED      = 0x01,     /*!< Alarm source enabled. */

} ZBPRO_ZCL_SapBasicAttrAlarmMaskId_t;


/**//**
 * \brief   Enumeration of bits comprised in DisableLocalConfig bitmask attribute of Basic
 *  ZCL cluster.
 * \details
 *  This attribute has width of 8 bits. Bits with numbers from 2 to 7 (starting with 0)
 *  have no special names and may be used by ZCL-based profile.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.13, table 3-12.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrDisableLocalConfigBit_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_LOCAL_CONFIG_BIT_RESET_TO_FACTORY_DEFAULTS = 0,    /*!< Reset (to factory defaults)
                                                                                    function. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_LOCAL_CONFIG_BIT_DEVICE_CONFIGURATION      = 1,    /*!< Device configuration function. */

} ZBPRO_ZCL_SapBasicAttrDisableLocalConfigBit_t;


/**//**
 * \brief   Enumeration of values of single bit of DisableLocalConfig attribute of Basic
 *  ZCL cluster.
 * \note
 *  This enumeration provides values for single bit without taking into account bit
 *  position within the bitmask representing the attribute. One shall shift these values
 *  to appropriate number of bits to come with the absolute value of corresponding bit.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.2.2.2.13, table 3-12.
 */
typedef enum _ZBPRO_ZCL_SapBasicAttrDisableLocalConfigId_t
{
    ZBPRO_ZCL_SAP_BASIC_ATTR_LOCAL_CONFIG_ENABLED   = 0x00,     /*!< Function enabled. */

    ZBPRO_ZCL_SAP_BASIC_ATTR_LOCAL_CONFIG_DISABLED  = 0x01,     /*!< Function disabled. */

} ZBPRO_ZCL_SapBasicAttrDisableLocalConfigId_t;


/**//**
 * \brief   Enumeration of client-to-server commands specific to Basic ZCL cluster.
 * \details
 *  These commands are generated by Basic ZCL cluster Client side and received by Server
 *  side.
 * \details
 *  Basic ZCL cluster has no commands generated by Server side and received by Client
 *  side.
 * \note
 *  This implementation of Basic ZCL cluster doesn't provide Client side; and its Server
 *  side doesn't implement reception of any of the listed commands as they all are
 *  optional.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.2.2.3, 3.2.2.4, 3.2.3, table 3-13.
 */
typedef enum _ZBPRO_ZCL_SapBasicClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_BASIC_CMD_ID_RESET_TO_FACTORY_DEFAULTS    = 0x00,     /*!< Reset to Factory Defaults. */

} ZBPRO_ZCL_SapBasicClientToServerCommandId_t;


#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_BASIC_H */
