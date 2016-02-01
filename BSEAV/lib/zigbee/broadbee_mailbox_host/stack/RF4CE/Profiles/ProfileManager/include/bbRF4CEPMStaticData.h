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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ProfileManager/include/bbRF4CEPMStaticData.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Profiles Manager Layer component static
 *   data declaration.
 *
 * $Revision: 3370 $
 * $Date: 2014-08-22 07:38:12Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_PM_STATIC_DATA_H
#define _RF4CE_PM_STATIC_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"
#include "bbRF4CENWK.h"
#include "bbRF4CEPMStartReset.h"
#include "bbRF4CEPMPair.h"
#include "bbRF4CEPMProfiles.h"
#include "private/bbRF4CEPMPrivateStartReset.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE Profiles Manager Pair Indication Callback.
 */
typedef void (*RF4CE_NWK_PairIndCB)(RF4CE_NWK_PairIndParams_t *indication);

/**//**
 * \brief RF4CE Profiles Manager static data structure.
 */
typedef struct _RF4CE_PM_StaticData_t
{
    struct
    {
        RF4CE_NWK_StartReqDescr_t startNWK;
        RF4CE_NWK_ResetReqDescr_t resetNWK;
        RF4CE_NWK_DiscoveryRespDescr_t discoveryResponse;
        RF4CE_NWK_PairRespDescr_t pairResponse;
        RF4CE_NWK_UnpairRespDescr_t unpairResponse;
        RF4CE_StartReqDescr_t startProfile;
        RF4CE_ResetReqDescr_t resetProfile;
        RF4CE_NWK_RXEnableReqDescr_t rxEnable;
        RF4CE_NWK_SetReqDescr_t setReq;
        RF4CE_NWK_UnpairReqDescr_t unpairRequest;
    } nwkRequests;
    uint8_t iProfile;
    uint8_t profileState;
    uint8_t supportedDevices[3];
    uint8_t supportedDevicesSize;
    RF4CE_ResetReqDescr_t resetRequest;
    SYS_SchedulerTaskDescriptor_t tasks;
    SYS_QueueDescriptor_t reqDescriptor;
    RF4CE_PM_ProfilesDataQueue_t profilesData[RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES];
    RF4CE_PM_StorableProfilesDataQueue_t storableProfilesData[RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES];
    SYS_TimeoutTask_t profilesTimeout;
    RF4CE_NWK_PairIndCB pairIndication;
    Bool8_t isPMStartUp;
    Bool8_t isPMShuttingDown;
} RF4CE_PM_StaticData_t;

/************************* DEFINITIONS *************************************************/
#define RF4CE_PM_STATIC_DATA_VAR_NAME rf4cePMStaticData

/**//**
 * \brief Profile states used.
 */
typedef enum _RF4CE_PM_ProfileState_t
{
    RF4CE_PM_COLD_START_DORMANT = 0,
    RF4CE_PM_RESET              = 1,
    RF4CE_PM_RUNNING            = 2
} RF4CE_PM_ProfileState_t;

/**//**
 * \brief Common Stack Static Structure member.
 */
#define RF4CE_PM_STATIC_DATA_FIELD()               RF4CE_PM_StaticData_t RF4CE_PM_StaticDataDataField;

/**//**
 * \brief Common Stack Static Structure member access.
 */
#define GET_RF4CE_PM_STATIC_DATA_FIELD()   (&RF4CE_PM_STATIC_DATA_VAR_NAME.RF4CE_PM_StaticDataDataField)

/**//**
 * \brief Common Stack Static Structure member initialization.
 */
extern const SYS_SchedulerTaskHandler_t rf4cePMTaskHandlers[];
#define INIT_RF4CE_PM_STATIC_DATA_FIELD() \
.RF4CE_PM_StaticDataDataField = \
{ \
    .iProfile = 0, \
    .tasks = \
    { \
        .qElem = \
        { \
            .nextElement = NULL, \
        }, \
        .priority = SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY, \
        .handlers = rf4cePMTaskHandlers, \
        .handlersMask = 0, \
    }, \
    .profileState = RF4CE_PM_COLD_START_DORMANT, \
    .supportedDevices = {RF4CE_GENERIC, 0, 0}, \
    .supportedDevicesSize = 1, \
    .profilesTimeout = \
    { \
        .taskDescriptor = &GET_RF4CE_PM_STATIC_DATA_FIELD()->tasks, \
        .handlerId = RF4CE_PM_TIMER_PROC, \
    }, \
    .isPMStartUp = true, \
},

typedef struct _INT_RF4CE_PM_StaticData_t
{
    RF4CE_PM_STATIC_DATA_FIELD()
} INT_RF4CE_PM_StaticData_t;

extern INT_RF4CE_PM_StaticData_t RF4CE_PM_STATIC_DATA_VAR_NAME;

#endif /* _RF4CE_PM_STATIC_DATA_H */