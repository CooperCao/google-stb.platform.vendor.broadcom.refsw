/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/*****************************************************************************
*
* FILENAME: projects/SoC_mailboxHostSideTest/rf4ce_registration/zigbee_rf4ce_registration.c
*
* DESCRIPTION:
*   The RF4CE registration function used to deliver the indication.
*
*****************************************************************************/
#include "bbRF4CEConfig.h"
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKNIBAttributes.h"
#include "bbMailAPI.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_rf4ce_registration_priv.h"
#include "zigbee_rf4ce_indication.h"
#include "sqlite3.h"

#ifdef _HOST_

static RF4CERegistrationServiceDescr_t registrationService;
static sqlite3* sql_db = NULL;

void RF4CE_DeleteRegistrationInfo(int clientId)
{

}

INLINE RF4CERegistrationServiceDescr_t *rf4ceRegistrationServiceDescr(void)
{
    return &registrationService;
}

static RF4CE_RegisterVirtualDeviceInternalInfo_t *findItemByClientId(int clientId)
{
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_RegistrationInfoQueueHeader());

    while (NULL != current)
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        if(info->clientId == clientId)
            return info;
        current = SYS_QueueGetNextQueueElement(current);
    }
    return NULL;

}

static int checkIsRegistered(RF4CE_RegisterVirtualDeviceParams_t *indParams, int newClientId)
{
    int clientId = RF4CE_REGISTER_INVALIDE_CLIENT_ID;
    Bool8_t match = true;
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_RegistrationInfoQueueHeader());
    while (NULL != current)
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        RF4CE_RegisterVirtualDeviceParams_t     *regParams = &info->params;
        if(indParams->fieldValidMask & RF4CE_ORGVENDORID_MASK)
            match = MATCH_VENDOR_ID(regParams, indParams);
        if(match && (indParams->fieldValidMask & RF4CE_ORGUSERSTRING_MASK))
            match = MATCH_USER_STRING(regParams, indParams);
        if(match && (indParams->fieldValidMask & RF4CE_ORGDEVICETYPE_MASK))
            match = MATCH_DEVICE_TYPE(regParams, indParams);
        if(match && (indParams->fieldValidMask & RF4CE_ORGPROFILEID_MASK))
            match = MATCH_PROFILE_ID(regParams, indParams);
        if(match && (indParams->fieldValidMask & RF4CE_MACADDRESS_MASK))
            match = MATCH_MAC_ADDRESS(regParams, indParams);
        if(match && (indParams->fieldValidMask & RF4CE_PAIRREF_MASK))
            match = MATCH_PARI_REF(regParams, indParams);
        if(match){
            clientId = info->clientId;
            info->clientId = newClientId;
            break;
        }
        current = SYS_QueueGetNextQueueElement(current);
    }
    return clientId;
}

static int checkPairingRef(uint8_t pairingRef)
{
    RF4CE_NWK_GetReqDescr_t nwkGetReq;
    int nwkGetReqStatus = 0;
    uint8_t status;
    memset(&nwkGetReq, 0, sizeof(RF4CE_NWK_GetReqDescr_t));
    RF4CE_NWK_AttributeID_t *attrId = &nwkGetReq.params.attrId;
    attrId->attrId = RF4CE_NWK_PAIRING_TABLE;
    attrId->attrIndex = pairingRef;
    void nwkGetReqCallback(RF4CE_NWK_GetReqDescr_t *req, RF4CE_NWK_GetConfParams_t *conf)
    {
        nwkGetReqStatus = 1;
        status = conf->status;
    }
    nwkGetReq.callback = nwkGetReqCallback;
    RF4CE_NWK_GetReq_Call(&nwkGetReq);
    while(nwkGetReqStatus == 0);
    return (status == RF4CE_SUCCESS) ? 0 : -1;
}

static void insertRegistrationInfo(RF4CE_RegisterVirtualDeviceReqDescr_t *reqDesc, int clientId)
{
    RF4CE_RegisterVirtualDeviceInternalInfo_t *info = malloc(sizeof(RF4CE_RegisterVirtualDeviceInternalInfo_t));
    memset(info, 0, sizeof(RF4CE_RegisterVirtualDeviceInternalInfo_t));
    memcpy(&info->params, &reqDesc->params, sizeof(RF4CE_RegisterVirtualDeviceParams_t));
    info->clientId = clientId;
    registrationInfoLock();
    SYS_QueuePutQueueElementToTail(RF4CE_RegistrationInfoQueueHeader(), &info->elem);
    registrationInfoUnlock();
}

uint8_t InsertClientInfoToWaitingPairQueue(int clientId)
{
    uint8_t resultInsert = 0;
    if(SYS_QueueIsEmpty(RF4CE_WaitForPairQueueQueueHeader()))
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *info = malloc(sizeof(RF4CE_RegisterVirtualDeviceInternalInfo_t));
        memset(info, 0, sizeof(RF4CE_RegisterVirtualDeviceInternalInfo_t));
        info->clientId = clientId;
        registerWaitForPairLock();
        SYS_QueuePutQueueElementToTail(RF4CE_WaitForPairQueueQueueHeader(), &info->elem);
        registerWaitForPairUnlock();
        resultInsert = 1;
    }else{
        SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_WaitForPairQueueQueueHeader());
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        if(info->clientId == clientId)
            resultInsert = 1;
    }
    return resultInsert;
}

void registerRemoveRegistryInfoInWaitForPairQueue()
{
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_WaitForPairQueueQueueHeader());
    if (NULL != current)
    {
        registerWaitForPairLock();
        SYS_QueueRemoveQueueElement(RF4CE_WaitForPairQueueQueueHeader(), current);
        registerWaitForPairUnlock();
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        free(info);
    }
}

void registerRemoveRegistryInfoForPairRef(uint8_t pairRef)
{
    /* remove the information in the registration queue */
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_RegistrationInfoQueueHeader());
    while (NULL != current)
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        current = SYS_QueueGetNextQueueElement(current);
        if(PAIRREF_IS_VALID(info) && (info->params.pairRef == pairRef)){
            registrationInfoLock();
            SYS_QueueRemoveQueueElement(RF4CE_RegistrationInfoQueueHeader(), &info->elem);
            registrationInfoUnlock();
            free(info);
        }
    }
}

void registerRemoveRegistrationInfoForClient(int socket)
{
    /* remove the information in wait for pair queue */
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_WaitForPairQueueQueueHeader());
    if (NULL != current)
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        if(info->clientId == socket){
            registerWaitForPairLock();
            SYS_QueueRemoveQueueElement(RF4CE_WaitForPairQueueQueueHeader(), current);
            registerWaitForPairUnlock();
            free(info);
        }
    }
    /* remove the information in the registration queue */
    current = SYS_QueueGetQueueHead(RF4CE_RegistrationInfoQueueHeader());
    while (NULL != current)
    {
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        current = SYS_QueueGetNextQueueElement(current);
        if(info->clientId == socket){
            registrationInfoLock();
            SYS_QueueRemoveQueueElement(RF4CE_RegistrationInfoQueueHeader(), &info->elem);
            registrationInfoUnlock();
            free(info);
        }
    }

}

/* in rpc mode, compiled into the server process, but not the client process */
/* in non-rpc mode, compiled into the server library, not the client application */

void RF4CE_RegisterVirtualDevice(RF4CE_RegisterVirtualDeviceReqDescr_t *reqDesc, int clientId)
{
    /* Update the existing registration item */
    RF4CE_RegisterVirtualDeviceConfParams_t conf;
    conf.status = RF4CE_REGISTER_SUCCESS;
    if(0 == reqDesc->params.fieldValidMask & RF4CE_REGISTERFIELDMASKALL){
        conf.status = RF4CE_REGISTER_ERROR;
        reqDesc->callback(reqDesc, &conf, clientId);
        return;
    }
    if((reqDesc->params.fieldValidMask & RF4CE_PAIRREF_MASK) && (checkPairingRef(reqDesc->params.pairRef) < 0)){
        conf.status = RF4CE_REGISTER_INVALID_PAIRING_REF;
        reqDesc->callback(reqDesc, &conf, clientId);
        return;
    }
    if(checkIsRegistered(&reqDesc->params, clientId) != RF4CE_REGISTER_INVALIDE_CLIENT_ID)
    {
        conf.status = RF4CE_REGISTER_REPLACE_EXISTING;
    }
    else
        insertRegistrationInfo(reqDesc, clientId);
    reqDesc->callback(reqDesc, &conf, clientId);
}

SYS_QueueElement_t *RF4CE_RegistrationInfoQueueHeader(void)
{
    return &rf4ceRegistrationServiceDescr()->registrationInfoQueue;
}

SYS_QueueElement_t *RF4CE_WaitForPairQueueQueueHeader(void)
{
    return &rf4ceRegistrationServiceDescr()->waitForPairQueue;
}

void registrationInfoLockInit(void)
{
    pthread_mutex_init(&rf4ceRegistrationServiceDescr()->registrationInfoLock, NULL);
}

void registrationInfoLock(void)
{
    pthread_mutex_lock(&rf4ceRegistrationServiceDescr()->registrationInfoLock);
}

void registrationInfoUnlock(void)
{
    pthread_mutex_unlock(&rf4ceRegistrationServiceDescr()->registrationInfoLock);
}

void registerWaitForPairLockInit(void)
{
    pthread_mutex_init(&rf4ceRegistrationServiceDescr()->waitForPairLock, NULL);
}

void registerWaitForPairLock(void)
{
    pthread_mutex_lock(&rf4ceRegistrationServiceDescr()->waitForPairLock);
}

void registerWaitForPairUnlock(void)
{
    pthread_mutex_unlock(&rf4ceRegistrationServiceDescr()->waitForPairLock);
}

/************************************************************************************************
  /brief Registration initialization function. We use the sqlite to simplify saveing the
         information to file. We don't need to care about the file format. Just operate it as
         database.
  /param[in]  sqlFileName - Specify the full path filename for sqlite.
  /param[out] None
  /Descr There is only one table in the database which is named registration_info, the columns are
         RegistrationId[integer]  -- Primary key.
         OrgVendorId[blob] -- Vendor id(7 bytes).
         OrgUserString[blob] -- User string(15 bytes).
         OrgDeviceType[blob] -- Device Type(1 byte).
         OrgProfileId[blob] -- Profile Id(1 byte).
         MacAddress[blob] -- Mac Address(8 bytes).
         PairRef[blob] -- Paring ref#(1 byte).
************************************************************************************************/
#if 1
#define SQL_REGISTRATION_ID_STR  "RegistrationId"
#define SQL_ORG_VENDOR_ID_STR    "OrgVendorId"
#define SQL_ORG_USER_STRING_STR  "OrgUserString"
#define SQL_ORG_DEVICE_TYPE_STR  "OrgDeviceType"
#define SQL_ORG_PROFILE_ID_STR  "OrgProfileId"
#define SQL_MAC_ADDRESS_STR  "MacAddress"
#define SQL_PAIR_REF_STR  "PairRef"

#define SQL_SET_ORG_VENDOR_ID_FIELD_HEADER_STR SQL_ORG_VENDOR_ID_STR"=x'"
#define SQL_SET_ORG_USER_STRING_FIELD_HEADER_STR SQL_ORG_USER_STRING_STR"=x'"
#define SQL_SET_ORG_DEVICE_TYPE_FIELD_HEADER_STR SQL_ORG_DEVICE_TYPE_STR"=x'"
#define SQL_SET_ORG_PROFILE_ID_FIELD_HEADER_STR SQL_ORG_PROFILE_ID_STR"=x'"
#define SQL_SET_MAC_ADDRESS_FIELD_HEADER_STR SQL_MAC_ADDRESS_STR"=x'"
#define SQL_SET_PAIR_REF_FIELD_HEADER_STR SQL_PAIR_REF_STR"=x'"

#define SQL_SET_FIELD_COMMON_HEAD_STR "'"
#define SQL_SET_FIELD_COMMON_TAILER_STR  SQL_SET_FIELD_COMMON_HEAD_STR

#define SQL_SET_FIELD_SPLITTER_STR ","
#define SQL_SPACE_STR " "
#define SQL_SET_FIELD_SPLITTER_CHAR ','

#define SQL_SAVE_FIELD_NULL_VALUE  "NULL"
#define SQL_SAVE_FIELD_COMMON_HEAD_STR  SQL_SET_FIELD_COMMON_HEAD_STR
#define SQL_SAVE_FIELD_COMMON_TAILER_STR  SQL_SET_FIELD_COMMON_TAILER_STR
#define SQL_SAVE_FIELD_HEX_MODIFIER_STR  "x"
#define SQL_SAVE_FIELD_SPLITTER_STR SQL_SET_FIELD_SPLITTER_STR
#define SQL_SAVE_VALUES_HEAD_STR    "("
#define SQL_SAVE_VALUES_TAILER_STR  ")"

#define SQL_REGISTRATIONID_FIELD_INDEX   0
#define SQL_ORGVENDORID_FIELD_INDEX   1
#define SQL_ORGUSERSTRING_FIELD_INDEX   2
#define SQL_ORGDEVICETYPE_FIELD_INDEX   3
#define SQL_ORGPROFILEID_FIELD_INDEX   4
#define SQL_MACADDRESS_FIELD_INDEX   5
#define SQL_PAIRREF_FIELD_INDEX   6

#define SQL_DB_FIELD_INDEX

void RF4CE_RegistrationInit(const uint8_t *const sqlFileName)
{
    RF4CERegistrationServiceDescr_t *descr = rf4ceRegistrationServiceDescr();
    memset(descr, 0, sizeof(RF4CERegistrationServiceDescr_t));
    SYS_QueueResetQueue(RF4CE_RegistrationInfoQueueHeader());
    SYS_QueueResetQueue(RF4CE_WaitForPairQueueQueueHeader());
    registrationInfoLockInit();
    registerWaitForPairLockInit();
    return;

    char *errMsg = 0;
    int rc = sqlite3_open(sqlFileName, &sql_db);
    if(rc){
        sqlite3_close(sql_db);
        SYS_DbgLogStr("Error couldn't open SQLite database %s", sqlFileName);
        return;
    }

    uint8_t creatTableSql[] = "CREATE TABLE IF NOT EXISTS `registration_info` ("    \
                              "`RegistrationId` INTEGER PRIMARY KEY AUTOINCREMENT," \
                              "`OrgVendorId` blob,"                        \
                              "`OrgUserString` blob,"                      \
                              "`OrgDeviceType` blob,"                      \
                              "`OrgProfileId` blob,"                       \
                              "`MacAddress` blob,"                         \
                              "`PairRef` blob"                             \
                              ");";
    rc = sqlite3_exec(sql_db, creatTableSql, NULL, 0, &errMsg);
    if( rc != SQLITE_OK ){
        SYS_DbgLogStr("SQL error: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }
    RF4CE_RegisterVirtualDeviceParams_t saveInfo = {
        .orgVendorId = 0x5678,
        .expectedMacAddress = "\x11\x22\x33\x44\x55\x66\x77\x88"
    };
    saveInfo.fieldValidMask = RF4CE_ORGVENDORID_MASK | RF4CE_MACADDRESS_MASK;
    RF4CE_RegistrationSavePairingInfoToDb(&saveInfo);
    saveInfo.fieldValidMask = RF4CE_ORGVENDORID_MASK | RF4CE_PAIRREF_MASK | RF4CE_MACADDRESS_MASK;
    saveInfo.pairRef = 0xfe;
    RF4CE_RegistrationSavePairingInfoToDb(&saveInfo);
    return;
}

INLINE uint8_t *byteArrayToStr(const uint8_t *const byteArray, uint8_t byteArrayLength)
{
    uint8_t *strArray = calloc(byteArrayLength * 2 + 1, 1);
    for(int i = 0; i < byteArrayLength; i++)
        snprintf(strArray + i * 2, (byteArrayLength - i) * 2 + 1, "%02x", byteArray[i]);

    return strArray;
}

INLINE void strToByteArray(uint8_t *const string, uint8_t *const byteArray, uint8_t byteArrayLength)
{
    SYS_DbgAssert(byteArrayLength != 0, 0);
    uint32_t tmpValue;
    memset(byteArray, 0, sizeof(byteArrayLength));
    uint8_t iter = MIN(strlen(string) / 2, byteArrayLength);
    while(--iter){
        sscanf(&string[iter * 2], "%02x", &tmpValue);
        byteArray[iter] = tmpValue & 0xff;
        string[iter*2] = 0;
    }
}

/****************************************************************************************************************
  \brief
    Get the saved record from the database
  \param[out]   savedInfo
    Retrieve the record from database
  \param[in]    macAddress
    The mac address field is unique, which is used to identify the one specific record
  \return
    The primary key of the record if found, 0 otherwise.

    RF4CE_ORGUSERSTRING_MASK   = (1 << 1),
    RF4CE_ORGDEVICETYPE_MASK   = (1 << 2),
    RF4CE_ORGPROFILEID_MASK    = (1 << 3),
    RF4CE_MACADDRESS_MASK      = (1 << 4),
    RF4CE_PAIRREF_MASK         = (1 << 5)

#define SQL_ORGPROFILEID_FIELD_INDEX   4
#define SQL_MACADDRESS_FIELD_INDEX   5
#define SQL_PAIRREF_FIELD_INDEX   6
****************************************************************************************************************/
INLINE  uint32_t getRegistrationRecordFromDb(RF4CE_RegisterVirtualDeviceParams_t *const savedInfo, const uint8_t *const macAddress)
{
    char *errMsg = 0;
    uint8_t querySql[200];
    uint8_t recordId = 0;
    memset(savedInfo, 0, sizeof(RF4CE_RegisterVirtualDeviceParams_t));

    uint8_t *macStr = byteArrayToStr(macAddress, sizeof(savedInfo->expectedMacAddress));
    snprintf(querySql, sizeof(querySql), "select * from registration_info where MacAddress=x'%s'", macStr);
    free(macStr);

    int getPairRefFromDbCallback(void *data, int argc, char **argv, char **azColName)
    {
        if(argc > 0){
            SYS_DbgAssert(argc == 1, 0);
            if(azColName[SQL_REGISTRATIONID_FIELD_INDEX] && \
                !strcmp(azColName[SQL_REGISTRATIONID_FIELD_INDEX], SQL_REGISTRATION_ID_STR)){
                SYS_DbgAssert(argv[0] != NULL, 0);
                sscanf(argv[0], "%d", &recordId);
            }
            if(azColName[SQL_ORGVENDORID_FIELD_INDEX] &&
                !strcmp(azColName[SQL_ORGVENDORID_FIELD_INDEX], SQL_ORG_VENDOR_ID_STR) && argv[SQL_ORGVENDORID_FIELD_INDEX]){
                strToByteArray(argv[SQL_ORGVENDORID_FIELD_INDEX], (char*)&savedInfo->orgVendorId, sizeof(savedInfo->orgVendorId));
                savedInfo->fieldValidMask |= RF4CE_ORGVENDORID_MASK;
            }
            if(azColName[SQL_ORGUSERSTRING_FIELD_INDEX] &&
                !strcmp(azColName[SQL_ORGUSERSTRING_FIELD_INDEX], SQL_ORG_USER_STRING_STR) && argv[SQL_ORGUSERSTRING_FIELD_INDEX]){
                strToByteArray(argv[SQL_ORGUSERSTRING_FIELD_INDEX], savedInfo->orgUserString, sizeof(savedInfo->orgUserString));
                savedInfo->fieldValidMask |= RF4CE_ORGUSERSTRING_MASK;
            }
            if(azColName[SQL_ORGDEVICETYPE_FIELD_INDEX] &&
                !strcmp(azColName[SQL_ORGDEVICETYPE_FIELD_INDEX], SQL_ORG_DEVICE_TYPE_STR) && argv[SQL_ORGDEVICETYPE_FIELD_INDEX]){
                strToByteArray(argv[SQL_ORGDEVICETYPE_FIELD_INDEX], &savedInfo->orgDeviceType, sizeof(savedInfo->orgDeviceType));
                savedInfo->fieldValidMask |= RF4CE_ORGDEVICETYPE_MASK;
            }
            if(azColName[SQL_ORGPROFILEID_FIELD_INDEX] &&
                !strcmp(azColName[SQL_ORGPROFILEID_FIELD_INDEX], SQL_ORG_PROFILE_ID_STR) && argv[SQL_ORGPROFILEID_FIELD_INDEX]){
                strToByteArray(argv[SQL_ORGPROFILEID_FIELD_INDEX], (uint8_t*)&savedInfo->orgProfileID, sizeof(savedInfo->orgProfileID));
                savedInfo->fieldValidMask |= RF4CE_ORGPROFILEID_MASK;
            }
            if(azColName[SQL_MACADDRESS_FIELD_INDEX] &&
                !strcmp(azColName[SQL_MACADDRESS_FIELD_INDEX], SQL_MAC_ADDRESS_STR) && argv[SQL_MACADDRESS_FIELD_INDEX]){
                strToByteArray(argv[SQL_MACADDRESS_FIELD_INDEX], savedInfo->expectedMacAddress, sizeof(savedInfo->expectedMacAddress));
                savedInfo->fieldValidMask |= RF4CE_MACADDRESS_MASK;
            }
            if(azColName[SQL_PAIRREF_FIELD_INDEX] &&
                !strcmp(azColName[SQL_PAIRREF_FIELD_INDEX], SQL_PAIR_REF_STR) && argv[SQL_PAIRREF_FIELD_INDEX]){
                strToByteArray(argv[SQL_PAIRREF_FIELD_INDEX], &savedInfo->pairRef, sizeof(savedInfo->pairRef));
                savedInfo->fieldValidMask |= RF4CE_PAIRREF_MASK;
            }
        }
        return 0;
    }

    int rc = sqlite3_exec(sql_db, querySql, getPairRefFromDbCallback, 0, &errMsg);
    if( rc != SQLITE_OK ){
        SYS_DbgLogStr("SQL error: %s", errMsg);
        sqlite3_free(errMsg);
    }
    return recordId;
}

INLINE uint8_t *sqlSetVendorBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_ORGVENDORID_MASK, 0);
    {
        strncat(bufHead, SQL_SET_ORG_VENDOR_ID_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr((char*)&saveInfo->orgVendorId, sizeof(saveInfo->orgVendorId)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSetUserStringBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_ORGUSERSTRING_MASK, 0);
    {
        strncat(bufHead, SQL_SET_ORG_USER_STRING_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(saveInfo->orgUserString, sizeof(saveInfo->orgUserString)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSetDeviceTypeBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_ORGDEVICETYPE_MASK, 0);
    {
        strncat(bufHead, SQL_SET_ORG_DEVICE_TYPE_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(&saveInfo->orgDeviceType, sizeof(saveInfo->orgDeviceType)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSetProfileIdBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_ORGPROFILEID_MASK, 0);
    {
        strncat(bufHead, SQL_SET_ORG_PROFILE_ID_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr((uint8_t*)&saveInfo->orgProfileID, sizeof(saveInfo->orgProfileID)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSetMACAddressBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_MACADDRESS_MASK, 0);
    {
        strncat(bufHead, SQL_SET_MAC_ADDRESS_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(saveInfo->expectedMacAddress, sizeof(saveInfo->expectedMacAddress)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSetPairRefBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    SYS_DbgAssert(saveInfo->fieldValidMask & RF4CE_PAIRREF_MASK, 0);
    {
        strncat(bufHead, SQL_SET_PAIR_REF_FIELD_HEADER_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(&saveInfo->pairRef, sizeof(saveInfo->pairRef)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SET_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}


INLINE uint8_t *sqlSaveVendorBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_ORGVENDORID_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr((char*)&saveInfo->orgVendorId, sizeof(saveInfo->orgVendorId)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSaveUserStringBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_ORGUSERSTRING_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(saveInfo->orgUserString, sizeof(saveInfo->orgUserString)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSaveDeviceTypeBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_ORGDEVICETYPE_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(&saveInfo->orgDeviceType, sizeof(saveInfo->orgDeviceType)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSaveProfileIdBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_ORGPROFILEID_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr((uint8_t*)&saveInfo->orgProfileID, sizeof(saveInfo->orgProfileID)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSaveMACAddressBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_MACADDRESS_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(saveInfo->expectedMacAddress, sizeof(saveInfo->expectedMacAddress)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}

INLINE uint8_t *sqlSavePairRefBlob(uint8_t *bufHead, uint16_t bufSize, const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t *pTmp = NULL;
    if(!(saveInfo->fieldValidMask & RF4CE_PAIRREF_MASK))
        strncat(bufHead, SQL_SAVE_FIELD_NULL_VALUE, bufSize - strlen(bufHead));
    else{
        strncat(bufHead, SQL_SAVE_FIELD_HEX_MODIFIER_STR SQL_SAVE_FIELD_COMMON_HEAD_STR, bufSize - strlen(bufHead));
        strncat(bufHead, pTmp = byteArrayToStr(&saveInfo->pairRef, sizeof(saveInfo->pairRef)), bufSize - strlen(bufHead));
        free(pTmp);
        strncat(bufHead, SQL_SAVE_FIELD_COMMON_TAILER_STR, bufSize - strlen(bufHead));
    }
    return bufHead;
}



INLINE uint8_t *sqlToSaveConstructor(const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    uint8_t saveSqlHeader[] = "insert into registration_info values"SQL_SAVE_VALUES_HEAD_STR;
    uint16_t sqlStrSize = 500;
    uint8_t *strSql = calloc(sqlStrSize, 1);
    uint8_t *pTmp = NULL;
    strncat(strSql, saveSqlHeader, sqlStrSize - strlen(strSql));
    /* the table entry id */
    {
        strncat(strSql, SQL_SAVE_FIELD_NULL_VALUE, sqlStrSize - strlen(strSql));
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
    }

    {
        sqlSaveVendorBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }

    {
        sqlSaveUserStringBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }

    {
        sqlSaveDeviceTypeBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    {
        sqlSaveProfileIdBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    {
        sqlSaveMACAddressBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SAVE_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    {
        sqlSavePairRefBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
    }
    strncat(strSql, SQL_SAVE_VALUES_TAILER_STR, sqlStrSize - strlen(strSql));
    SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);

    return strSql;
}

INLINE uint8_t *sqlToUpdateConstructor(const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo, uint32_t tableEntryId)
{
    uint8_t saveSqlHeader[] = "update registration_info set";
    uint16_t sqlStrSize = 500;
    uint8_t *strSql = calloc(sqlStrSize, 1);
    uint8_t *pTmp = NULL;
    strncat(strSql, saveSqlHeader, sqlStrSize - strlen(strSql));

    strncat(strSql, SQL_SPACE_STR, sqlStrSize - strlen(strSql));
    if(saveInfo->fieldValidMask & RF4CE_ORGVENDORID_MASK){
        sqlSetVendorBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SET_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }

    if(saveInfo->fieldValidMask & RF4CE_ORGUSERSTRING_MASK){
        sqlSetUserStringBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SET_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }

    if(saveInfo->fieldValidMask & RF4CE_ORGDEVICETYPE_MASK){
        sqlSetDeviceTypeBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SET_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    if(saveInfo->fieldValidMask & RF4CE_ORGPROFILEID_MASK){
        sqlSetProfileIdBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SET_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    if(saveInfo->fieldValidMask & RF4CE_MACADDRESS_MASK){
        sqlSetMACAddressBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
        strncat(strSql, SQL_SET_FIELD_SPLITTER_STR, sqlStrSize - strlen(strSql));
    }
    if(saveInfo->fieldValidMask & RF4CE_PAIRREF_MASK){
        sqlSetPairRefBlob(strSql, sqlStrSize, saveInfo);
        SYS_DbgAssert(strlen(strSql) < sqlStrSize, 0);
    }
    if(SQL_SET_FIELD_SPLITTER_CHAR == strSql[strlen(strSql) - 1])
        strSql[strlen(strSql) - 1] = 0;
    strncat(strSql, SQL_SPACE_STR, sqlStrSize - strlen(strSql));
    sprintf(strSql + strlen(strSql), "where"SQL_SPACE_STR SQL_REGISTRATION_ID_STR"=%d", tableEntryId);
    return strSql;
}

void RF4CE_RegistrationSavePairingInfoToDb(const RF4CE_RegisterVirtualDeviceParams_t *const saveInfo)
{
    char *errMsg = 0;
    uint32_t tableEntryId;
    RF4CE_RegisterVirtualDeviceParams_t savedInfo;
    if(!saveInfo->fieldValidMask & RF4CE_MACADDRESS_MASK)
        return;
    if((tableEntryId = getRegistrationRecordFromDb(&savedInfo, saveInfo->expectedMacAddress)) == 0){
        /*
         *   add a new entry, the format is insert into registration_info values(NULL, x' ', x' ', x' ', x' ', x' ', x' ')
         */
        uint8_t *saveSql = sqlToSaveConstructor(saveInfo);
        SYS_DbgLogStr("saveSql : %s", saveSql);

        int rc = sqlite3_exec(sql_db, (char*)saveSql, NULL, 0,  &errMsg);
        if( rc != SQLITE_OK ){
            SYS_DbgLogStr("SQL error: %s", errMsg);
            sqlite3_free(errMsg);
        }
        free(saveSql);
        return;
    }
    /* Update a existing entry */
    {
        uint8_t *updateSql = sqlToUpdateConstructor(saveInfo, tableEntryId);
        SYS_DbgLogStr("updateSql : %s", updateSql);
        int rc = sqlite3_exec(sql_db, (char*)updateSql, NULL, 0,  &errMsg);
        if( rc != SQLITE_OK ){
            SYS_DbgLogStr("SQL error: %s", errMsg);
            sqlite3_free(errMsg);
        }
        free(updateSql);
        return;
    }
}
#endif

#endif