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
 ******************************************************************************/

/*****************************************************************************
*
* DESCRIPTION:
*   The RF4CE registration function used to deliver the indication.
*
*****************************************************************************/

#ifndef _ZIGBEE_RF4CE_REGISTRATION_H
#define _ZIGBEE_RF4CE_REGISTRATION_H

#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysQueue.h"

typedef enum _RF4CE_RegisterFieldMask_t{
    RF4CE_ORGVENDORID_MASK     = (1 << 0),
    RF4CE_ORGVENDORRSTRING_MASK= (1 << 1),
    RF4CE_ORGUSERSTRING_MASK   = (1 << 2),
    RF4CE_ORGDEVICETYPE_MASK   = (1 << 3),
    RF4CE_ORGPROFILEID_MASK    = (1 << 4),
    RF4CE_MACADDRESS_MASK      = (1 << 5),
    RF4CE_PAIRREF_MASK         = (1 << 6)
}RF4CE_RegisterFieldMask_t;

typedef enum _RF4CE_Register_Status_t{
    RF4CE_REGISTER_INVALID_PAIRING_REF = -2,  /* The paring ref is invalid */
    RF4CE_REGISTER_ERROR = -1,          /* Generic failure */
    RF4CE_REGISTER_SUCCESS = 0,         /* Generic success */
    RF4CE_REGISTER_ADD_NEW,             /* Have added a new registration information */
    RF4CE_REGISTER_REPLACE_EXISTING,    /* Have updated the existing one with the new registration information */
    RF4CE_REGISTER_DELETE_EXISTING      /* Have deleted existing */
}RF4CE_Register_Status_t;

#define RF4CE_REGISTER_INVALIDE_CLIENT_ID  (-1)

#define RF4CE_REGISTERFIELDMASKALL (RF4CE_ORGVENDORID_MASK   |\
                                    RF4CE_ORGVENDORRSTRING_MASK |\
                                    RF4CE_ORGUSERSTRING_MASK |\
                                    RF4CE_ORGDEVICETYPE_MASK |\
                                    RF4CE_ORGPROFILEID_MASK  |\
                                    RF4CE_MACADDRESS_MASK    |\
                                    RF4CE_PAIRREF_MASK)

#define PAIRREF_IS_VALID(regInfo)  ((regInfo)->params.fieldValidMask & RF4CE_PAIRREF_MASK)

typedef struct _RF4CE_RegisterVirtualDeviceParams_t RF4CE_RegisterVirtualDeviceParams_t;

/* please keep consistent with the definition in the RF4CE check validation indication */
typedef struct _RF4CE_RegisterVirtualDeviceParams_t
{
    uint16_t orgVendorId;
    uint8_t orgVendorString[7];
    uint8_t orgUserString[15];
    uint8_t orgDeviceType;
    uint16_t orgProfileID;
    uint8_t expectedMacAddress[8];
    uint8_t pairRef;
    uint8_t  fieldValidMask;    /* mask bits to indicate which parameter is valid */
} RF4CE_RegisterVirtualDeviceParams_t;

typedef struct _RF4CE_RegisterVirtualDeviceConfParams_t  RF4CE_RegisterVirtualDeviceConfParams_t;

typedef struct _RF4CE_RegisterVirtualDeviceConfParams_t
{
    uint32_t  status;
} RF4CE_RegisterVirtualDeviceConfParams_t;


typedef struct _RF4CE_RegisterVirtualDeviceReqDescr_t RF4CE_RegisterVirtualDeviceReqDescr_t;

#ifdef __cplusplus
extern "C"{
#endif

#ifdef SERVER
typedef void RF4CE_RegisterVirtualDeviceConfCallback_t(RF4CE_RegisterVirtualDeviceReqDescr_t   *reqDescr,
                                     RF4CE_RegisterVirtualDeviceConfParams_t *confParams, int socket);
#else
typedef void RF4CE_RegisterVirtualDeviceConfCallback_t(RF4CE_RegisterVirtualDeviceReqDescr_t   *reqDescr,
                                     RF4CE_RegisterVirtualDeviceConfParams_t *confParams);
#endif

/**//**
 * \brief descriptor data type.
 */
typedef struct _RF4CE_RegisterVirtualDeviceReqDescr_t
{
    /* 32-bit data. */
    RF4CE_RegisterVirtualDeviceConfCallback_t *callback;      /*!< Confirm callback function.  */
    RF4CE_RegisterVirtualDeviceParams_t     params;        /*!< Request parameters set.     */
} RF4CE_RegisterVirtualDeviceReqDescr_t;

#ifdef SERVER
void RF4CE_RegisterVirtualDevice(RF4CE_RegisterVirtualDeviceReqDescr_t *reqDesc, int clientId);
#else
void RF4CE_RegisterVirtualDevice(RF4CE_RegisterVirtualDeviceReqDescr_t *reqDesc);
#endif

SYS_QueueElement_t *RF4CE_RegistrationInfoQueueHeader(void);
SYS_QueueElement_t *RF4CE_WaitForPairQueueQueueHeader(void);
void RF4CE_RegistrationSavePairingInfoToDb(const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo);
void RF4CE_RegistrationInit(const uint8_t *const sqlFileName);
uint8_t InsertClientInfoToWaitingPairQueue(int clientId);
void registerRemoveRegistryInfoInWaitForPairQueue(void);
void registerRemoveRegistryInfoForPairRef(uint8_t pairRef);
void registerRemoveRegistrationInfoForClient(int socket);
void registrationInfoLockInit(void);
void registrationInfoLock(void);
void registrationInfoUnlock(void);
void registerWaitForPairLockInit(void);
void registerWaitForPairLock(void);
void registerWaitForPairUnlock(void);

#ifdef __cplusplus
}
#endif

#endif

/* eof zigbee_rf4ce_registration.h */