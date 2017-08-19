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
 *      This is the source code file for the RF4CE profile manager profiles section implementation.
 *
*******************************************************************************/

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"

#include "bbRF4CEPM.h"
#include "private/bbRF4CEPMPrivateStartReset.h"

/************************* EXPORTED DATA ***********************************************/
const uint8_t RF4CE_ProfilesIdList[] =
{
    GDP_PROFILES_ID
    ZRC_PROFILES_ID
    MSO_PROFILES_ID
};

const uint8_t RF4CE_ProfilesIdList1[] =
{
    GDP_PROFILES_ID
    ZRC_PROFILES_ID1
    MSO_PROFILES_ID
};
const uint8_t RF4CE_ProfilesIdList2[] =
{
    GDP_PROFILES_ID
    ZRC_PROFILES_ID2
    MSO_PROFILES_ID
};

#ifndef _HOST_
const RF4CE_PM_ProfilesData_t RF4CE_DataIndications[] =
{
    GDP_DATA_INDICATION
    ZRC_DATA_INDICATION_1
    ZRC_DATA_INDICATION_2
    MSO_DATA_INDICATION
    CC_DATA_INDICATION
};
#endif  // _HOST_

/************************* IMPLEMENTATION **********************************************/
#ifndef _HOST_

/************************************************************************************//**
 \brief Sets Supported Device List.

 \param[in] request - ponter to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_SetSupportedDevicesReq(RF4CE_SetSupportedDevicesReqDescr_t *request)
{
    SYS_DbgAssert(NULL != request, RF4CE_PM_SET_SUPPORTED_DEVICES_REQ);

    request->service.requestID = RF4CE_PM_SET_SUPPORTED_DEVICES;
    RF4CE_PM_AddRequestToQueue(&request->service.serviceData);
} /* RF4CE_SetSupportedDevicesReq() */

/************************************************************************************//**
 \brief Returns the supported device list.

 \return The supported device list.
 ****************************************************************************************/
uint8_t* RF4CE_DevicesIdList(void)
{
    RF4CE_PM_StaticData_t *pm = GET_RF4CE_PM_STATIC_DATA_FIELD();

    SYS_DbgAssert(NULL != pm, RF4CE_PM_GET_DIVICES_ID_LIST);

    return pm->supportedDevices;
} /* RF4CE_DevicesIdList() */

/************************************************************************************//**
 \brief Profile Manager Set Supported Devices handler.

 \param[in] queueElemet - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_PM_SetSupportedDevicesHandler(SYS_QueueElement_t *queueElement)
{
    RF4CE_SetSupportedDevicesReqDescr_t *request;
    RF4CE_SetSupportedDevicesConfParams_t confirm;
    RF4CE_PM_StaticData_t *pm = GET_RF4CE_PM_STATIC_DATA_FIELD();

    SYS_DbgAssert(NULL != queueElement, RF4CE_PM_SET_SUPPORTED_DEVICES_HANDLER_00);
    SYS_DbgAssert(NULL != pm, RF4CE_PM_SET_SUPPORTED_DEVICES_HANDLER_01);

    confirm.status = false;
    request = GET_PARENT_BY_FIELD(RF4CE_SetSupportedDevicesReqDescr_t, service.serviceData, queueElement);
    SYS_QueueRemoveQueueElement(&pm->reqDescriptor, queueElement);
    if (0 < request->params.numDevices && 4 > request->params.numDevices)
    {
        memcpy(pm->supportedDevices, request->params.devices, request->params.numDevices);
        pm->supportedDevicesSize = request->params.numDevices;
        confirm.status = true;
    } /* if (0 < request->params.numDevices && 4 > request->params.numDevices) */
    if (NULL != request->callback)
    {
#if defined(ENABLE_RF4CE_CONFIRM_LOGIDS)
        SYS_DbgLogId(RF4CE_PM_SetSupportedDevicesConfirm);
#endif
        request->callback(request, &confirm);
    }
    if (!SYS_QueueIsEmpty(&pm->reqDescriptor))
        SYS_SchedulerPostTask(&pm->tasks, RF4CE_PM_TASK_MANAGER);
} /* RF4CE_PM_SetSupportedDevicesHandler() */

/************************************************************************************//**
 \brief Returns size of the DevicesIdList.

 \return Size of the DevicesIdList.
 ****************************************************************************************/
uint8_t RF4CE_DevicesIdListSize(void)
{
    RF4CE_PM_StaticData_t *pm = GET_RF4CE_PM_STATIC_DATA_FIELD();

    SYS_DbgAssert(NULL != pm, RF4CE_PM_GET_DIVICES_ID_LIST_SIZE);

    return pm->supportedDevicesSize;
}

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList.

 \return Size of the ProfilesIdList.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize(void)
{
    return sizeof(RF4CE_ProfilesIdList);
}

/************************************************************************************//**
 \brief Returns the ProfilesIdList.

 \return Size of the ProfilesIdList.
 ****************************************************************************************/
#ifdef _PHY_TEST_HOST_INTERFACE_
uint8_t* RF4CE_ProfilesIdListValue(void)
{
    return (uint8_t*)RF4CE_ProfilesIdList;
}
#endif  //#ifdef _PHY_TEST_HOST_INTERFACE_

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList1.

 \return Size of the ProfilesIdList1.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize1(void)
{
    return sizeof(RF4CE_ProfilesIdList1);
}

/************************************************************************************//**
 \brief Returns size of the ProfilesIdList2.

 \return Size of the ProfilesIdList2.
 ****************************************************************************************/
uint8_t RF4CE_ProfilesIdListSize2(void)
{
    return sizeof(RF4CE_ProfilesIdList2);
}

/************************************************************************************//**
 \brief Returns size of the DataIndications.

 \return Size of the DataIndications.
 ****************************************************************************************/
uint8_t RF4CE_DataIndicationsSize(void)
{
    return sizeof(RF4CE_DataIndications)/sizeof(RF4CE_PM_ProfilesData_t);
}


/************************************************************************************//**
 \brief Checkes if the pairing reference is used.
 \ingroup RF4CE_PM_Functions

 \param[in] - pairRef - pairing reference which should be checked.
 \return true if the pairRef is marked as used in the pairing table in the RF4CE NWK.
 ****************************************************************************************/
bool RF4CE_PM_CheckPairRefUsed(uint8_t pairRef)
{
    if (pairRef >= RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
        return false;
    else
    {
        RF4CE_PairingTableEntry_t *const nwkPairingTable =
            GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable.nwkPairingTable;

        return IS_RF4CE_PAIRING_ENTRY_USED(&nwkPairingTable[pairRef]);
    }
}

#endif  // _HOST_

/* eof bbRF4CEPMProfiles.c */