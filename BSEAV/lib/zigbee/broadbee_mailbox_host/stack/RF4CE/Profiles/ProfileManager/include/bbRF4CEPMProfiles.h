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
 *      This is the header file for the RF4CE profile manager profiles inclusion section.
 *
*******************************************************************************/

#ifndef _RF4CE_PM_PROFILES_H
#define _RF4CE_PM_PROFILES_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CEConfig.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKData.h"
#include "private/bbRF4CEZRCCommands.h"
#include "private/bbRF4CEZRCProfileData.h"
#include "private/bbRF4CEMSOProfileData.h"

/************************* EXTERNAL FUNCTIONS USED *************************************/
extern void RF4CE_MSO_Data(RF4CE_NWK_DataIndParams_t *indication);
extern void RF4CE_CC_Data(RF4CE_NWK_DataIndParams_t *indication);
/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Profiles support inclusive macros.
 */
#define RF4CE_PM_GDP_ID             0
#define RF4CE_PM_ZRC1_ID            1
#define RF4CE_PM_ZRC2_ID            3
#define RF4CE_PM_MSO_ID             0xC0
#define RF4CE_PM_CC_ID              0xCC
/**//**
 * \brief Devices support inclusive enumeration.
 */
typedef enum _RF4CE_DevicesTypes_t
{
    RF4CE_REMOTE_CONTROL           = 0x01,
    RF4CE_TELEVISION               = 0x02,
    RF4CE_PROJECTOR                = 0x03,
    RF4CE_PLAYER                   = 0x04,
    RF4CE_RECORDER                 = 0x05,
    RF4CE_VIDEO_PALYER             = 0x06,
    RF4CE_AUDIO_PLAYER             = 0x07,
    RF4CE_AUDIO_VIDEO_RECORDER     = 0x08,
    RF4CE_SET_TOP_BOX              = 0x09,
    RF4CE_HOME_THEATER_SYSTEM      = 0x0A,
    RF4CE_MEDIA_CENTER_PC          = 0x0B,
    RF4CE_GAME_CONSOLE             = 0x0C,
    RF4CE_SATELLITE_RADIO_RECEIVER = 0x0D,
    RF4CE_IR_EXTENDER              = 0x0E,
    RF4CE_MONITOR                  = 0x0F,
    RF4CE_GENERIC                  = 0xFE
} RF4CE_DevicesTypes_t;

#if (1 == USE_RF4CE_PROFILE_GDP)
#    define GDP_PROFILES_ID         RF4CE_PM_GDP_ID,
#    define GDP_DATA_INDICATION     {RF4CE_PM_GDP_ID, RF4CE_ZRC2_Data},
#else /* (1 == USE_RF4CE_PROFILE_GDP) */
#    define GDP_PROFILES_ID
#    define GDP_DATA_INDICATION
#endif /* (1 == USE_RF4CE_PROFILE_GDP) */

#if (1 == USE_RF4CE_PROFILE_ZRC)
#    if defined(USE_RF4CE_PROFILE_ZRC1) && !defined(USE_RF4CE_PROFILE_ZRC2)
#        define ZRC_PROFILES_ID         RF4CE_PM_ZRC1_ID,
#        define ZRC_DATA_INDICATION_1   {RF4CE_PM_ZRC1_ID, RF4CE_ZRC1_Data},
#        define ZRC_DATA_INDICATION_2
#        define ZRC_PROFILES_ID1        RF4CE_PM_ZRC1_ID,
#        define ZRC_PROFILES_ID2
#    else /* defined(USE_RF4CE_PROFILE_ZRC1) && !defined(USE_RF4CE_PROFILE_ZRC2) */
#        if !defined(USE_RF4CE_PROFILE_ZRC1) && defined(USE_RF4CE_PROFILE_ZRC2)
#            define ZRC_PROFILES_ID         RF4CE_PM_ZRC2_ID,
#            define ZRC_DATA_INDICATION_1
#            define ZRC_DATA_INDICATION_2   {RF4CE_PM_ZRC2_ID, RF4CE_ZRC2_Data},
#            define ZRC_PROFILES_ID1
#            define ZRC_PROFILES_ID2        RF4CE_PM_ZRC2_ID,
#        else /* !defined(USE_RF4CE_PROFILE_ZRC1) && defined(USE_RF4CE_PROFILE_ZRC2) */
#            define ZRC_PROFILES_ID         RF4CE_PM_ZRC1_ID, RF4CE_PM_ZRC2_ID,
#            define ZRC_DATA_INDICATION_1   {RF4CE_PM_ZRC1_ID, RF4CE_ZRC1_Data},
#            define ZRC_DATA_INDICATION_2   {RF4CE_PM_ZRC2_ID, RF4CE_ZRC2_Data},
#            define ZRC_PROFILES_ID1        RF4CE_PM_ZRC1_ID,
#            define ZRC_PROFILES_ID2        RF4CE_PM_ZRC2_ID,
#        endif /* !defined(USE_RF4CE_PROFILE_ZRC1) && defined(USE_RF4CE_PROFILE_ZRC2) */
#    endif /* defined(USE_RF4CE_PROFILE_ZRC1) && !defined(USE_RF4CE_PROFILE_ZRC2) */
#else /* (1 == USE_RF4CE_PROFILE_ZRC) */
#    define ZRC_PROFILES_ID
#    define ZRC_DATA_INDICATION_1
#    define ZRC_DATA_INDICATION_2
#    define ZRC_PROFILES_ID1
#    define ZRC_PROFILES_ID2
#endif /* (1 == USE_RF4CE_PROFILE_ZRC) */

#if (1 == USE_RF4CE_PROFILE_MSO)
#    define MSO_PROFILES_ID         RF4CE_PM_MSO_ID,
#    define MSO_DATA_INDICATION     {RF4CE_PM_MSO_ID, RF4CE_MSO_Data},
#else /* (1 == USE_RF4CE_PROFILE_MSO) */
#    define MSO_PROFILES_ID
#    define MSO_DATA_INDICATION
#endif /* (1 == USE_RF4CE_PROFILE_MSO) */

#if (1 == USE_RF4CE_PROFILE_CC)
#    define CC_PROFILES_ID         RF4CE_PM_CC_ID,
#    define CC_DATA_INDICATION     {RF4CE_PM_CC_ID, RF4CE_CC_Data},
#else /* (1 == USE_RF4CE_PROFILE_CC) */
#    define CC_PROFILES_ID
#    define CC_DATA_INDICATION
#endif /* (1 == USE_RF4CE_PROFILE_CC) */

typedef enum _RF4CE_ProfileState_t
{
    RF4CE_PS_FREE = 0,      /*!< Current member is free */
    RF4CE_PS_IDLE,          /*!< Current member is idle */
    RF4CE_PS_SENDING,       /*!< Current member is sending */
    RF4CE_PS_MAX_PM_STATE   /*!< Maximum value for PM State */
} RF4CE_ProfileState_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief Profiles/data indication data structure.
 * \ingroup RF4CE_ProfileDataInd
 */
typedef struct _RF4CE_PM_ProfilesData_t
{
    uint8_t profileId;                       /*!< Profile ID */
    void (*callback)(RF4CE_NWK_DataIndParams_t *indication); /*!< Data Indication Callback */
} RF4CE_PM_ProfilesData_t;

/**//**
 * \brief Profiles Set Devices Support request parameters.
 * \ingroup RF4CE_SetSupportedDevicesReq
 */
typedef struct _RF4CE_SetSupportedDevicesReqParams_t
{
    uint8_t numDevices;                      /*!< Number of devices */
    uint8_t devices[3];                      /*!< Array of supported devices */
} RF4CE_SetSupportedDevicesReqParams_t;

/**//**
 * \brief Profiles Set Devices Support confirmation parameters.
 * \ingroup RF4CE_SetSupportedDevicesConf
 */
typedef struct _RF4CE_SetSupportedDevicesConfParams_t
{
    Bool8_t status;                          /*!< Confirmation status */
} RF4CE_SetSupportedDevicesConfParams_t;

/**//**
 * \brief Profiles Set Devices Support request declaration.
 * \ingroup RF4CE_SetSupportedDevicesReq
 */
typedef struct _RF4CE_SetSupportedDevicesReqDescr_t RF4CE_SetSupportedDevicesReqDescr_t;

/**//**
 * \brief Profiles Set Devices Support callback declaration.
 * \ingroup RF4CE_SetSupportedDevicesConf
 */
typedef void (*RF4CE_GDP_SetSupportedDevicesCallback_t)(RF4CE_SetSupportedDevicesReqDescr_t *req, RF4CE_SetSupportedDevicesConfParams_t *conf);

/**//**
 * \brief Profiles Set Devices Support request descriptor.
 * \ingroup RF4CE_SetSupportedDevicesReq
 */
struct _RF4CE_SetSupportedDevicesReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;      /*!< Service field */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_SetSupportedDevicesReqParams_t params;      /*!< Request parameters */
    RF4CE_GDP_SetSupportedDevicesCallback_t callback; /*!< Confirmation callback */
};

/**//**
 * \brief Storable Profiles data queue member.
 */
typedef struct _RF4CE_PM_StorableProfilesDataQueue_t
{
    uint8_t profileId;                    /*!< Profile ID occupied */
    Bool8_t profileIdSet;                 /*!< If Profile ID was set */
    uint8_t incomingProfileId;            /*!< Profile ID for incoming packets */
    uint8_t currentState;                 /*!< Current state. Extenable RF4CE_ProfileState_t */
#if (1 == USE_RF4CE_PROFILE_ZRC)
    RF4CE_ZRC_StorableProfileData_t zrc;          /*!< ZRC storable profile private data. */
#endif /* (1 == USE_RF4CE_PROFILE_ZRC) */
#if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_MSO_StorableProfileData_t mso;          /*!< MSO storable profile private data. */
#endif /* (1 == USE_RF4CE_PROFILE_MSO) */
} RF4CE_PM_StorableProfilesDataQueue_t;

/**//**
 * \brief Profiles data queue member.
 */
typedef struct _RF4CE_PM_ProfilesDataQueue_t
{
    RF4CE_NWK_DataReqDescr_t dataRequest; /*!< NWK Data request. */
#if (1 == USE_RF4CE_PROFILE_ZRC)
    RF4CE_ZRC_ProfileData_t zrc;          /*!< ZRC profile private data. */
#endif /* (1 == USE_RF4CE_PROFILE_ZRC) */
#if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_MSO_ProfileData_t mso;          /*!< MSO profile private data. */
#endif /* (1 == USE_RF4CE_PROFILE_MSO) */
} RF4CE_PM_ProfilesDataQueue_t;

/************************* EXPORTED DATA ***********************************************/
extern const uint8_t RF4CE_ProfilesIdList[];
extern const uint8_t RF4CE_ProfilesIdList1[];
extern const uint8_t RF4CE_ProfilesIdList2[];
extern const RF4CE_PM_ProfilesData_t RF4CE_DataIndications[];

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Sets Supported Device List.
 \ingroup RF4CE_PM_Functions

 \param[in] request - ponter to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_SetSupportedDevicesReq(RF4CE_SetSupportedDevicesReqDescr_t *request);

/************************************************************************************//**
 \brief Returns the supported device list.
 \ingroup RF4CE_PM_Functions

 \return The supported device list.
 ****************************************************************************************/
uint8_t* RF4CE_DevicesIdList(void);

/************************************************************************************//**
 \brief Returns size of the DevicesIdList.
 \ingroup RF4CE_PM_Functions

 \return Size of the DevicesIdList.
 ****************************************************************************************/
uint8_t RF4CE_DevicesIdListSize(void);

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList.
 \ingroup RF4CE_PM_Functions

 \return Size of the ProfilesIdList.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize(void);

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList1.
 \ingroup RF4CE_PM_Functions

 \return Size of the ProfilesIdList1.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize1(void);

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList2.
 \ingroup RF4CE_PM_Functions

 \return Size of the ProfilesIdList2.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize2(void);

/************************************************************************************//**
 \brief Returns size of the DataIndications.
 \ingroup RF4CE_PM_Functions

 \return Size of the DataIndications.
 ****************************************************************************************/
uint8_t RF4CE_DataIndicationsSize(void);

/************************************************************************************//**
 \brief Checkes if the pairing reference is used.
 \ingroup RF4CE_PM_Functions

 \param[in] pairRef - pairing reference which should be checked.
 \return true if the pairRef is marked as used in the pairing table in the RF4CE NWK.
 ****************************************************************************************/
bool RF4CE_PM_CheckPairRefUsed(uint8_t pairRef);

#endif /* _RF4CE_PM_PROFILES_H */

/* eof bbRF4CEPMProfiles.h */