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
 ******************************************************************************/
#ifndef ZIGBEE_HA_COMMON_H
#define ZIGBEE_HA_COMMON_H

typedef unsigned long ZIGBEE_HA_Status;

#define ZIGBEE_HA_STATUS_SUCCESS    0x00000000
#define ZBPRO_MAC_MODULE_ID         0x80000000
#define ZBPRO_NWK_MODULE_ID         0x90000000
#define ZBPRO_APS_MODULE_ID         0xA0000000
#define ZBPRO_ZDO_MODULE_ID         0xB0000000
#define ZBPRO_HA_MODULE_ID          0xC0000000

/*********************** wrapper function declaration ********************************************/
static void Sys_DbgPrint(const char *format, ...);
extern void ZBPRO_APS_SetReq(ZBPRO_APS_SetReqDescr_t *const reqDescr);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_macTransactionPersistenceTime(MAC_TransactionPersistenceTime_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_macExtendedAddress(MAC_ExtendedAddress_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_nwkDeviceType(ZBPRO_NWK_NIB_DeviceType_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_apsTrustCenterAddress(ZBPRO_APS_ExtAddr_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_zdoInitialSecurityStatus(ZBPRO_APS_SecurityInitialStatus_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_nwkActiveKeySeqNumber(ZBPRO_NWK_NIB_ActiveKeySeqNumber_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_zdoPermitJoinDuration(ZBPRO_NWK_PermitJoinDuration_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_zdoZdpResponseTimeout(SYS_Time_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_nwkSequenceNumber(ZBPRO_NWK_NIB_SequenceNumber_t attrValue);
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_nwkFrameCounter(ZbProSspFrameCnt_t attrValue);

static ZIGBEE_HA_Status Zigbee_HA_Get_MyMacExtendedAddress(MAC_ExtendedAddress_t *attrValue);

static ZIGBEE_HA_Status Zigbee_HA_Set_NWK_Key(ZbProSspKey_t *key, ZbProSspNwkKeySeqNum_t keyCounter);
static ZIGBEE_HA_Status Zigbee_HA_StartNetwork();
static ZIGBEE_HA_Status Zigbee_HA_GetActiveEp(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest, uint8_t *activeEpCount, ZBPRO_APS_EndpointId_t **activeEpList, SYS_Time_t timeout);
static ZIGBEE_HA_Status Zigbee_HA_GetSimpleDesc(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest,ZBPRO_ZDO_Endpoint_t endpoint, uint8_t *length, ZBPRO_APS_SimpleDescriptor_t *simpleDesc, SYS_Time_t timeout);
static ZIGBEE_HA_Status Zigbee_HA_GetNodeDesc(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest, ZbProZdoNodeDescriptor_t *nodeDesc, SYS_Time_t timeout);
static ZIGBEE_HA_Status Zigbee_HA_EndpointRegister(ZBPRO_APS_SimpleDescriptor_t *simpleDescr);
static ZIGBEE_HA_Status Zigbee_HA_ConfigureReporting_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                              ZBPRO_APS_EndpointId_t dstEndpoint,
                                                              ZBPRO_ZCL_ClusterId_t   clusterId,
                                                              ZBPRO_ZCL_AttributeId_t attributeID,
                                                              ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t minReportingInterval,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t maxReportingInterval,
                                                              SYS_Time_t timeout
                                                              );
static ZIGBEE_HA_Status Zigbee_HA_Write_Attributes_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                            int      attributeSize,
                                                            uint8_t *attributeData,
                                                            SYS_Time_t timeout
                                                            );

/***************************** wrapper function implementation ***********************************/

/* GET/SET wrapper function implementation */
/*
 *  ZIGBEE_HA_Status - return value. If everything is succesful, just return ZIGBEE_HA_STATUS_SUCCESS.
 *                     else return the module id and corresponding error code. The module id is very
 *                     helpful to user to figure out what's the exact root cause of failure since
 *                     different module has different error code set.
 */
#define ZIGBEE_HA_SET_ATTRIBUTE(ATTR_ID, ATTR_TYPE, ATTR_NAME)                              \
static ZIGBEE_HA_Status Zigbee_HA_Set_Attribute_##ATTR_NAME(ATTR_TYPE attrValue)            \
{                                                                                           \
    ZBPRO_APS_Status_t status = (ZBPRO_APS_Status_t)0xff;                                   \
    void zigbee_HA_Set_##ATTR_NAME##_Callback(ZBPRO_APS_SetReqDescr_t   *const reqDescr,    \
        ZBPRO_APS_SetConfParams_t *const confParams)                                        \
    {                                                                                       \
        status = confParams->status;                                                        \
    }                                                                                       \
    ZBPRO_APS_SetReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_APS_SetReqDescr_t));           \
    memset(reqDescr, 0, sizeof(ZBPRO_APS_SetReqDescr_t));                                   \
    ZBPRO_APS_SetReqParams_t *params = &reqDescr->params;                                   \
    params->id = ATTR_ID;                                                                   \
    params->value.ATTR_NAME = attrValue;                                                    \
    reqDescr->callback = zigbee_HA_Set_##ATTR_NAME##_Callback;                              \
    ZBPRO_APS_SetReq(reqDescr);                                                             \
    while(status == 0xff);                                                                  \
    free(reqDescr);                                                                         \
    return status ? (ZIGBEE_HA_Status)(ZBPRO_APS_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;   \
}

ZIGBEE_HA_SET_ATTRIBUTE(MAC_EXTENDED_ADDRESS, MAC_ExtendedAddress_t, macExtendedAddress)
ZIGBEE_HA_SET_ATTRIBUTE(MAC_TRANSACTION_PERSISTENCE_TIME, MAC_TransactionPersistenceTime_t, macTransactionPersistenceTime)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_NWK_NIB_DEVICE_TYPE, ZBPRO_NWK_NIB_DeviceType_t, nwkDeviceType)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_APS_IB_TRUST_CENTER_ADDRESS_ID, ZBPRO_APS_ExtAddr_t, apsTrustCenterAddress)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_APS_IB_INITIAL_SECURITY_STATUS_ID, ZBPRO_APS_SecurityInitialStatus_t, zdoInitialSecurityStatus)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_NWK_NIB_ACTIVE_KEY_SEQ_NUMBER, ZBPRO_NWK_NIB_ActiveKeySeqNumber_t, nwkActiveKeySeqNumber)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_APS_IB_PERMIT_JOIN_DURATION_ID, ZBPRO_NWK_PermitJoinDuration_t, zdoPermitJoinDuration)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_APS_IB_CHANNEL_MASK_ID, PHY_ChannelMask_t, apsChannelMask)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_APS_IB_ZDP_RESPONSE_TIMEOUT_ID, SYS_Time_t, zdoZdpResponseTimeout)
ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_NWK_NIB_SEQUENCE_NUMBER, ZBPRO_NWK_NIB_SequenceNumber_t, nwkSequenceNumber)
//ZIGBEE_HA_SET_ATTRIBUTE(ZBPRO_NWK_NIB_FRAME_COUNTER, ZbProSspFrameCnt_t, nwkFrameCounter)

static ZIGBEE_HA_Status Zigbee_HA_Get_MyMacExtendedAddress(MAC_ExtendedAddress_t *attrValue)
{
    ZBPRO_APS_Status_t status = (ZBPRO_APS_Status_t)0xff;
    void zigbee_HA_Get_Attribute_macExtendedAddress_Callback(MAC_GetReqDescr_t   *const reqDescr,
        MAC_GetConfParams_t *const confParams)
    {
        *attrValue = confParams->attributeValue.macExtendedAddress;
        status = confParams->status;
    }
    MAC_GetReqDescr_t  *reqDescr = malloc(sizeof(MAC_GetReqDescr_t));
    memset(reqDescr, 0, sizeof(MAC_GetReqDescr_t));
    MAC_GetReqParams_t *params = &reqDescr->params;
    reqDescr->callback = zigbee_HA_Get_Attribute_macExtendedAddress_Callback;
    ZBPRO_MAC_GetReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);
    return status ? (ZIGBEE_HA_Status)(ZBPRO_APS_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static bool Zigbee_HA_Check_NWK_Existing()
{
    ZBPRO_APS_Status_t status = (ZBPRO_APS_Status_t)0xff;
    ZBPRO_NWK_NIB_PanId_t panId;
    void zigbee_HA_Check_NWK_Existing_Callback(ZBPRO_APS_GetReqDescr_t   *const reqDescr,
        ZBPRO_APS_GetConfParams_t *const confParams)
    {
        panId = confParams->value.nwkPanId;
        status = confParams->status;
    }
    ZBPRO_APS_GetReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_APS_GetReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_APS_GetReqDescr_t));
    ZBPRO_APS_GetReqParams_t *params = &reqDescr->params;
    params->id = ZBPRO_NWK_NIB_PAN_ID;
    reqDescr->callback = zigbee_HA_Check_NWK_Existing_Callback;
    ZBPRO_APS_GetReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);
    return 0xFFFF == panId ? FALSE : TRUE;
}

/* network layer wrapper function implementation */
static ZIGBEE_HA_Status Zigbee_HA_Set_NWK_Key(ZbProSspKey_t *key,
                                  ZbProSspNwkKeySeqNum_t keyCounter)
{
    ZBPRO_NWK_Status_t status = (ZBPRO_NWK_Status_t)0xff;
    void zigbee_HA_Set_NWK_Key_Callback(ZBPRO_NWK_SetKeyReqDescr_t *const reqDescr,
        ZBPRO_NWK_SetKeyConfParams_t *const confParams)
    {
        status = confParams->status;
    }

    ZBPRO_NWK_SetKeyReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_NWK_SetKeyReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_NWK_SetKeyReqDescr_t));
    ZBPRO_NWK_SetKeyReqParams_t *params = &reqDescr->params;
    params->keyCounter = keyCounter;
    memcpy(&params->key, key, sizeof(ZbProSspKey_t));
    reqDescr->callback = zigbee_HA_Set_NWK_Key_Callback;
    ZBPRO_NWK_SetKeyReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);
    return status ? (ZIGBEE_HA_Status)(ZBPRO_NWK_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

//(ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_PermitJoiningReqDescr_t;)

static ZIGBEE_HA_Status Zigbee_HA_PermitJoining(ZBPRO_NWK_PermitJoinDuration_t time)
{
    ZBPRO_NWK_Status_t status = (ZBPRO_NWK_Status_t)0xff;
    void zigbee_HA_PermitJoining_Callback(ZBPRO_NWK_PermitJoiningReqDescr_t *const reqDescr,
        ZBPRO_NWK_PermitJoiningConfParams_t *const confParams)
    {
        status = confParams->status;
    }

    ZBPRO_NWK_PermitJoiningReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_NWK_PermitJoiningReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_NWK_PermitJoiningReqDescr_t));
    ZBPRO_NWK_PermitJoiningReqParams_t *params = &reqDescr->params;
    params->permitDuration = time;
    reqDescr->callback = zigbee_HA_PermitJoining_Callback;
    ZBPRO_NWK_PermitJoiningReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);
    return status ? (ZIGBEE_HA_Status)(ZBPRO_NWK_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

/* zdo layer wrapper function implementation */
static ZIGBEE_HA_Status Zigbee_HA_StartNetwork()
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_StartNetwork_Callback(ZBPRO_ZDO_StartNetworkReqDescr_t *const reqDescr,
        ZBPRO_ZDO_StartNetworkConfParams_t *const confParams)
    {
        status = confParams->status;
    }

    ZBPRO_ZDO_StartNetworkReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZDO_StartNetworkReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZDO_StartNetworkReqDescr_t));
    reqDescr->callback = zigbee_HA_StartNetwork_Callback;
    ZBPRO_ZDO_StartNetworkReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_GetNwkAddr(ZBPRO_ZDO_ExtAddr_t dstIeeeAddr, ZBPRO_ZDO_NwkAddr_t *nwkAddrOfInterest)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_GetNwkAddr_Callback(ZBPRO_ZDO_AddrResolvingReqDescr_t *const reqDescr,
        ZBPRO_ZDO_AddrResolvingConfParams_t *const confParams)
    {
        status = confParams->status;
        *nwkAddrOfInterest = confParams->nwkAddrRemoteDev;
    }

    ZBPRO_ZDO_AddrResolvingReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZDO_AddrResolvingReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZDO_AddrResolvingReqDescr_t));
    ZBPRO_ZDO_AddrResolvingReqParams_t *params = &reqDescr->params;
    params->zdpDstAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zdpDstAddress.shortAddr = 0xFFFD;
    params->addrOfInterest.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->addrOfInterest.extAddr = dstIeeeAddr;
    params->requestType = 0;
    params->startIndex = 0;
    reqDescr->callback = zigbee_HA_GetNwkAddr_Callback;
    ZBPRO_ZDO_AddrResolvingReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_GetActiveEp(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest, uint8_t *activeEpCount, ZBPRO_APS_EndpointId_t **activeEpList, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_GetActiveEp_Callback(ZBPRO_ZDO_ActiveEpReqDescr_t *const reqDescr,
        ZBPRO_ZDO_ActiveEpConfParams_t *const confParams)
    {
        *activeEpCount = confParams->activeEpCount;
        ZBPRO_APS_EndpointId_t *buf = malloc(confParams->activeEpCount * sizeof(ZBPRO_APS_EndpointId_t));
        *activeEpList = buf;
        SYS_CopyFromPayload(buf, &confParams->activeEpList, 0, confParams->activeEpCount * sizeof(ZBPRO_APS_EndpointId_t));
        status = confParams->status;
    }

    ZBPRO_ZDO_ActiveEpReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZDO_ActiveEpReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZDO_ActiveEpReqDescr_t));
    ZBPRO_ZDO_ActiveEpReqParams_t *params = &reqDescr->params;
    params->zdpDstAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zdpDstAddress.shortAddr = dstShortAddr;
    params->nwkAddrOfInterest = nwkAddrOfInterest;
    params->respWaitTimeout = timeout;
    reqDescr->callback = zigbee_HA_GetActiveEp_Callback;
    ZBPRO_ZDO_ActiveEpReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_GetSimpleDesc(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest,ZBPRO_ZDO_Endpoint_t endpoint, uint8_t *length, ZBPRO_APS_SimpleDescriptor_t *simpleDesc, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_GetSimpleDesc_Callback(ZBPRO_ZDO_SimpleDescReqDescr_t *const reqDescr,
        ZBPRO_ZDO_SimpleDescConfParams_t *const confParams)
    {
        *length = confParams->length;
        memcpy((uint8_t*)simpleDesc, (uint8_t*)&confParams->simpleDescriptor, sizeof(ZBPRO_APS_SimpleDescriptor_t));
        /*
        * malloc a space to save the in&out cluster list since it will be freed after this return.
        */
        int size_payload = SYS_GetPayloadSize(&confParams->simpleDescriptor.inOutClusterList);
        SYS_SetEmptyPayload(&simpleDesc->inOutClusterList);
        SYS_MemAlloc(&simpleDesc->inOutClusterList, size_payload);
        SYS_CopyPayloadToPayload(&simpleDesc->inOutClusterList, &confParams->simpleDescriptor.inOutClusterList, size_payload);
        status = confParams->status;
    }

    ZBPRO_ZDO_SimpleDescReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZDO_SimpleDescReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZDO_SimpleDescReqDescr_t));
    ZBPRO_ZDO_SimpleDescReqParams_t *params = &reqDescr->params;
    params->zdpDstAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zdpDstAddress.shortAddr = dstShortAddr;
    params->nwkAddrOfInterest = nwkAddrOfInterest;
    params->endpoint = endpoint;
    params->respWaitTimeout = timeout;
    reqDescr->callback = zigbee_HA_GetSimpleDesc_Callback;
    ZBPRO_ZDO_SimpleDescReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_GetNodeDesc(ZBPRO_APS_ShortAddr_t dstShortAddr, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest, ZbProZdoNodeDescriptor_t *nodeDesc, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_GetNodeDesc_Callback(ZBPRO_ZDO_NodeDescReqDescr_t *const reqDescr,
        ZBPRO_ZDO_NodeDescConfParams_t *const confParams)
    {
        memcpy((uint8_t*)nodeDesc, (uint8_t*)&confParams->nodeDescriptor, sizeof(ZbProZdoNodeDescriptor_t));
        status = confParams->status;
    }

    ZBPRO_ZDO_NodeDescReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZDO_NodeDescReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZDO_NodeDescReqDescr_t));
    ZBPRO_ZDO_NodeDescReqParams_t *params = &reqDescr->params;
    params->zdpDstAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zdpDstAddress.shortAddr = dstShortAddr;
    params->nwkAddrOfInterest = nwkAddrOfInterest;
    params->respWaitTimeout = timeout;
    reqDescr->callback = zigbee_HA_GetNodeDesc_Callback;
    ZBPRO_ZDO_NodeDescReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_EndpointRegister(ZBPRO_APS_SimpleDescriptor_t *simpleDescr)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_EndpointRegister_Callback(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr,
        ZBPRO_APS_EndpointRegisterConfParams_t *const confParams)
    {
        status = confParams->status;
    }

    ZBPRO_APS_EndpointRegisterReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    ZBPRO_APS_EndpointRegisterReqParams_t *params = &reqDescr->params;
    memcpy(&params->simpleDescriptor, simpleDescr, sizeof(ZBPRO_APS_SimpleDescriptor_t));
    params->useInternalHandler = TRUE;
    reqDescr->callback = zigbee_HA_EndpointRegister_Callback;
    ZBPRO_APS_EndpointRegisterReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

#define INIT_BIND_PARAMS(PARAMS, SRCEXTADDR, DSTEXTADDR, CLUSTERID, SRCENDP, DSTENDP)  \
{\
    PARAMS->srcAddress = SRCEXTADDR;  \
    PARAMS->dstAddr.addrMode = APS_EXTENDED_ADDRESS_MODE;  \
    PARAMS->dstAddr.extAddr = DSTEXTADDR;           \
    PARAMS->clusterId = CLUSTERID;                      \
    PARAMS->srcEndpoint = SRCENDP;                      \
    PARAMS->dstEndpoint = DSTENDP;                      \
}

static ZIGBEE_HA_Status Zigbee_HA_Bind(ZBPRO_APS_ExtAddr_t srcExtAddr, ZBPRO_APS_ExtAddr_t dstExtAddr, ZBPRO_APS_ClusterId_t clusterId, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Bind_Callback(ZBPRO_APS_BindUnbindReqDescr_t *const reqDescr,
        ZBPRO_APS_BindUnbindConfParams_t *const confParams)
    {
        status = confParams->status;
    }

    ZBPRO_APS_BindUnbindReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_APS_BindUnbindReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_APS_BindUnbindReqDescr_t));
    ZBPRO_APS_BindUnbindReqParams_t *params = &reqDescr->params;
    INIT_BIND_PARAMS(params, srcExtAddr, dstExtAddr, clusterId, srcEndpoint, dstEndpoint);
    reqDescr->callback = zigbee_HA_Bind_Callback;
    ZBPRO_APS_BindReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

/* ZCL helper function implementation */

void ZBPRO_ZCL_OnOffCmdOffReq(
                ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr);
void ZBPRO_ZCL_DoorLockCmdLockReq(
                ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr);
void ZBPRO_ZCL_DoorLockCmdUnlockReq(
                ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr);


static ZIGBEE_HA_Status Zigbee_HA_OnOffCmdOn_Indirect(Bool8_t on, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_OnOffCmdOn_Indirect_Callback(ZBPRO_ZCL_OnOffCmdReqDescr_t *const reqDescr,
        ZBPRO_ZCL_OnOffCmdConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_OnOffCmdReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    ZBPRO_ZCL_OnOffCmdReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_INDIRECT_MODE;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    reqDescr->callback = zigbee_HA_OnOffCmdOn_Indirect_Callback;
    if(on)
        ZBPRO_ZCL_OnOffCmdOnReq(reqDescr);
    else
        ZBPRO_ZCL_OnOffCmdOffReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_OnOffCmdOn_Direct(Bool8_t on, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint, ZBPRO_APS_ExtAddr_t dstExtAddr, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_OnOffCmdOn_Direct_Callback(ZBPRO_ZCL_OnOffCmdReqDescr_t *const reqDescr,
        ZBPRO_ZCL_OnOffCmdConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_OnOffCmdReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    ZBPRO_ZCL_OnOffCmdReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstExtAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    reqDescr->callback = zigbee_HA_OnOffCmdOn_Direct_Callback;
    if(on)
        ZBPRO_ZCL_OnOffCmdOnReq(reqDescr);
    else
        ZBPRO_ZCL_OnOffCmdOffReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_ConfigureReporting_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                              ZBPRO_APS_EndpointId_t dstEndpoint,
                                                              ZBPRO_ZCL_ClusterId_t   clusterId,
                                                              ZBPRO_ZCL_AttributeId_t attributeID,
                                                              ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t minReportingInterval,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t maxReportingInterval,
                                                              SYS_Time_t timeout
                                                              )
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_ConfigureReporting_Indirect_Callback(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t *params = &reqDescr->params;

    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_INDIRECT_MODE;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_DISABLED;
    params->directionReporting = ZBPRO_ZCL_ATTRIBUTE_REPORTING_CONFIGURATION_DIRECTION_REPORT;
    params->attributeID = attributeID;
    params->timeoutPeriod = 0x000A;
    params->minReportingInterval = minReportingInterval;
    params->maxReportingInterval = maxReportingInterval;
    params->attributeDataType = attributeDataType;
    reqDescr->callback = zigbee_HA_ConfigureReporting_Indirect_Callback;
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_ConfigureReporting_Direct(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                              ZBPRO_APS_EndpointId_t dstEndpoint,
                                                              ZBPRO_APS_ExtAddr_t dstExtAddr,
                                                              ZBPRO_ZCL_ClusterId_t   clusterId,
                                                              ZBPRO_ZCL_AttributeId_t attributeID,
                                                              ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                              uint8_t                  attributeDataSize,
                                                              uint8_t                  *reportableAttributeValueChange,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t minReportingInterval,
                                                              ZBPRO_ZCL_AttribureReportingInterval_t maxReportingInterval,
                                                              SYS_Time_t timeout
                                                              )
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_ConfigureReporting_Direct_Callback(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t *params = &reqDescr->params;

    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstExtAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_DISABLED;
    params->directionReporting = ZBPRO_ZCL_ATTRIBUTE_REPORTING_CONFIGURATION_DIRECTION_REPORT;
    params->attributeID = attributeID;
    params->timeoutPeriod = 0x000A;
    params->minReportingInterval = minReportingInterval;
    params->maxReportingInterval = maxReportingInterval;
    params->attributeDataType = attributeDataType;
    SYS_MemAlloc(&params->payload, attributeDataSize);
    SYS_CopyToPayload(&params->payload, 0, reportableAttributeValueChange, attributeDataSize);
    reqDescr->callback = zigbee_HA_ConfigureReporting_Direct_Callback;
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}


static ZIGBEE_HA_Status Zigbee_HA_DoorLockCmdLockUnlock_Indirect(Bool8_t lock, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_DoorLockCmdLockUnlock_Indirect_Callback(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr,
        ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_INDIRECT_MODE;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    reqDescr->callback = zigbee_HA_DoorLockCmdLockUnlock_Indirect_Callback;
    if(lock)
        ZBPRO_ZCL_DoorLockCmdLockReq(reqDescr);
    else
        ZBPRO_ZCL_DoorLockCmdUnlockReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_DoorLockCmdLockUnlock_Direct(Bool8_t lock, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint, ZBPRO_APS_ExtAddr_t dstExtAddr, SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_DoorLockCmdLockUnlock_Direct_Callback(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr,
        ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstExtAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    reqDescr->callback = zigbee_HA_DoorLockCmdLockUnlock_Direct_Callback;
    if(lock)
        ZBPRO_ZCL_DoorLockCmdLockReq(reqDescr);
    else
        ZBPRO_ZCL_DoorLockCmdUnlockReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}

static ZIGBEE_HA_Status Zigbee_HA_DoorLockCmdLockUnlock_DirectByNwkAddr(Bool8_t lock, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint, ZBPRO_APS_ShortAddr_t dstNwkAddr, SYS_Time_t timeout)
{
    static ZBPRO_ZCL_TransSeqNum_t lockTransSeqNum = 10;
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_DoorLockCmdLockUnlock_Direct_Callback(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr,
        ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t));
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstNwkAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.useSpecifiedTsn = FALSE;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    reqDescr->callback = zigbee_HA_DoorLockCmdLockUnlock_Direct_Callback;
    if(lock)
        ZBPRO_ZCL_DoorLockCmdLockReq(reqDescr);
    else
        ZBPRO_ZCL_DoorLockCmdUnlockReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}


static ZIGBEE_HA_Status Zigbee_HA_Write_Attributes_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                            int      attributeSize,
                                                            uint8_t *attributeData,
                                                            SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Write_Attributes_Indirect_Callback(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t *const confParams)
    {
        printf("%s got callback\n", __FUNCTION__);
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_INDIRECT_MODE;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->attributeId = attributeID;
    params->attrDataType = attributeDataType;
    SYS_MemAlloc(&params->payload, attributeSize);
    SYS_CopyToPayload(&params->payload, 0, attributeData, attributeSize);
    reqDescr->callback = zigbee_HA_Write_Attributes_Indirect_Callback;

    ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}


static ZIGBEE_HA_Status Zigbee_HA_Write_Attributes_Direct(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_APS_ExtAddr_t dstExtAddr,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            ZBPRO_ZCL_AttrDataType_t attributeDataType,
                                                            int      attributeSize,
                                                            uint8_t *attributeData,
                                                            SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Write_Attributes_Direct_Callback(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t *const confParams)
    {
        printf("%s got callback\n", __FUNCTION__);
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstExtAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->attributeId = attributeID;
    params->attrDataType = attributeDataType;
    SYS_MemAlloc(&params->payload, attributeSize);
    SYS_CopyToPayload(&params->payload, 0, attributeData, attributeSize);
    reqDescr->callback = zigbee_HA_Write_Attributes_Direct_Callback;

    ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_Read_Attributes_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            int      *attributeSize,
                                                            uint8_t **attributeData,
                                                            SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Read_Attributes_Indirect_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
    {
        int payloadSize = 0;
        uint8_t *payload = NULL;
        *attributeSize = payloadSize = SYS_GetPayloadSize(&confParams->payload);
        payload = malloc(payloadSize);
        SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
        *attributeData = payload;
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_INDIRECT_MODE;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->attributeId = attributeID;
    reqDescr->callback = zigbee_HA_Read_Attributes_Indirect_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_Read_Attributes_Direct(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_APS_ExtAddr_t dstExtAddr,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            int      *attributeSize,
                                                            uint8_t **attributeData,
                                                            SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Read_Attributes_Direct_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
    {
        int payloadSize = 0;
        uint8_t *payload = NULL;
        *attributeSize = payloadSize = SYS_GetPayloadSize(&confParams->payload);
        payload = malloc(payloadSize);
        SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
        *attributeData = payload;
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstExtAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->attributeId = attributeID;
    reqDescr->callback = zigbee_HA_Read_Attributes_Direct_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}

static ZIGBEE_HA_Status Zigbee_HA_Read_Attributes_DirectByNwk(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_APS_ShortAddr_t dstNwkAddr,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            int      *attributeSize,
                                                            uint8_t **attributeData,
                                                            SYS_Time_t timeout)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Read_Attributes_DirectByNwk_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
        ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
    {
        int payloadSize = 0;
        uint8_t *payload = NULL;
        *attributeSize = payloadSize = SYS_GetPayloadSize(&confParams->payload);
        payload = malloc(payloadSize);
        SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
        *attributeData = payload;
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_SHORT_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = dstNwkAddr;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    params->zclObligatoryPart.clusterId = clusterId;
    params->zclObligatoryPart.respWaitTimeout = timeout;
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->attributeId = attributeID;
    reqDescr->callback = zigbee_HA_Read_Attributes_DirectByNwk_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}
static ZIGBEE_HA_Status Zigbee_HA_Cie_Enroll_Start(ZBPRO_APS_ShortAddr_t dstShortAddr,
                                                   SYS_Time_t scanDurationMs,
                                                   SYS_Time_t  permitEnrollDurationMs,
                                                   Bool8_t autoREsponseMode)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Cie_Enroll_Start_Callback(ZBPRO_ZHA_CieEnrollReqDescr_t *const reqDescr,
        ZBPRO_ZHA_CieEnrollConfParams_t *const confParams)
    {
        status = ZIGBEE_HA_STATUS_SUCCESS;
    }

    ZBPRO_ZHA_CieEnrollReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZHA_CieEnrollReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZHA_CieEnrollReqDescr_t));
    ZBPRO_ZHA_CieEnrollReqParams_t *params = &reqDescr->params;

    params->addr.addrMode = APS_SHORT_ADDRESS_MODE;
    params->addr.shortAddr = dstShortAddr;
    params->scanDurationMs = scanDurationMs;
    params->permitEnrollDurationMs = permitEnrollDurationMs;
    params->autoREsponseMode = autoREsponseMode;
    reqDescr->callback = zigbee_HA_Cie_Enroll_Start_Callback;

    ZBPRO_ZHA_CieDeviceEnrollReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}


#define ZBPRO_INIT_ON_OFF_SWITCH_IN_CLUSTER_LIST  ZBPRO_ZCL_CLUSTER_ID_BASIC, ZBPRO_ZCL_CLUSTER_ID_IDENTIFY
#define ZBPRO_INIT_ON_OFF_SWITCH_OUT_CLUSTER_LIST ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_ONOFF

#define ZBPRO_INIT_ON_OFF_SWITCH_SIMPLE_DESCRIPTOR(SIMPLEDESCR, ENDPOINT, INCLUSTERAMOUNT, OUTCLUSTERAMOUNT)  \
{                   \
    SIMPLEDESCR->deviceId = ZHA_DEVICE_ID_ONOFF_SWITCH;   \
    SIMPLEDESCR->profileId = ZBPRO_ZHA_PROFILE_ID;  \
    SIMPLEDESCR->endpoint = ENDPOINT;   \
    SIMPLEDESCR->deviceVersion = 0;  \
    SIMPLEDESCR->inClusterAmount = INCLUSTERAMOUNT;    \
    SIMPLEDESCR->outClusterAmount = OUTCLUSTERAMOUNT;   \
}

void zbPro_HA_Init_OnOff_Switch_SimpleDescriptor(uint8_t endpoint, ZBPRO_APS_SimpleDescriptor_t *descr)
{
    memset(descr, 0, sizeof(ZBPRO_APS_SimpleDescriptor_t));
    uint8_t inClusterAmount = VA_NARGS(ZBPRO_INIT_ON_OFF_SWITCH_IN_CLUSTER_LIST);
    uint8_t outClusterAmount = VA_NARGS(ZBPRO_INIT_ON_OFF_SWITCH_OUT_CLUSTER_LIST);
    ZBPRO_INIT_ON_OFF_SWITCH_SIMPLE_DESCRIPTOR(descr, endpoint, inClusterAmount, outClusterAmount);

    SYS_MemAlloc(&descr->inOutClusterList, (inClusterAmount + outClusterAmount) * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t inClusters[] = { ZBPRO_INIT_ON_OFF_SWITCH_IN_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, 0, inClusters, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t outClusters[] = { ZBPRO_INIT_ON_OFF_SWITCH_OUT_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t), \
                                outClusters, outClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
}


#define ZBPRO_INIT_DOOR_LOCK_IN_CLUSTER_LIST  ZBPRO_ZCL_CLUSTER_ID_BASIC, ZBPRO_ZCL_CLUSTER_ID_IDENTIFY
#define ZBPRO_INIT_DOOR_LOCK_OUT_CLUSTER_LIST ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK

void zbPro_HA_Init_DoorLock_SimpleDescriptor(uint8_t endpoint, ZBPRO_APS_SimpleDescriptor_t *descr)
{
    memset(descr, 0, sizeof(ZBPRO_APS_SimpleDescriptor_t));
    uint8_t inClusterAmount = VA_NARGS(ZBPRO_INIT_DOOR_LOCK_IN_CLUSTER_LIST);
    uint8_t outClusterAmount = VA_NARGS(ZBPRO_INIT_DOOR_LOCK_OUT_CLUSTER_LIST);
    ZBPRO_INIT_ON_OFF_SWITCH_SIMPLE_DESCRIPTOR(descr, endpoint, inClusterAmount, outClusterAmount);

    SYS_MemAlloc(&descr->inOutClusterList, (inClusterAmount + outClusterAmount) * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t inClusters[] = { ZBPRO_INIT_DOOR_LOCK_IN_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, 0, inClusters, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t outClusters[] = { ZBPRO_INIT_DOOR_LOCK_OUT_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t), \
                                outClusters, outClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
}

#define ZBPRO_INIT_IAS_ZONE_IN_CLUSTER_LIST  ZBPRO_ZCL_CLUSTER_ID_BASIC, ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE
#define ZBPRO_INIT_IAS_ZONE_OUT_CLUSTER_LIST ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_IAS_ACE, ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE

#define ZBPRO_INIT_IAS_ACE_SIMPLE_DESCRIPTOR(SIMPLEDESCR, ENDPOINT, INCLUSTERAMOUNT, OUTCLUSTERAMOUNT)  \
{                   \
    SIMPLEDESCR->deviceId = ZHA_DEVICE_ID_IAS_ANCILLARY_CONTROL_EQUIPMENT;   \
    SIMPLEDESCR->profileId = ZBPRO_ZHA_PROFILE_ID;  \
    SIMPLEDESCR->endpoint = ENDPOINT;   \
    SIMPLEDESCR->deviceVersion = 0;  \
    SIMPLEDESCR->inClusterAmount = INCLUSTERAMOUNT;    \
    SIMPLEDESCR->outClusterAmount = OUTCLUSTERAMOUNT;   \
}

void zbPro_HA_Init_Ias_Ace_SimpleDescriptor(uint8_t endpoint, ZBPRO_APS_SimpleDescriptor_t *descr)
{
    memset(descr, 0, sizeof(ZBPRO_APS_SimpleDescriptor_t));
    uint8_t inClusterAmount = VA_NARGS(ZBPRO_INIT_IAS_ZONE_IN_CLUSTER_LIST);
    uint8_t outClusterAmount = VA_NARGS(ZBPRO_INIT_IAS_ZONE_OUT_CLUSTER_LIST);
    ZBPRO_INIT_IAS_ACE_SIMPLE_DESCRIPTOR(descr, endpoint, inClusterAmount, outClusterAmount);

    SYS_MemAlloc(&descr->inOutClusterList, (inClusterAmount + outClusterAmount) * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t inClusters[] = { ZBPRO_INIT_IAS_ZONE_IN_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, 0, inClusters, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t outClusters[] = { ZBPRO_INIT_IAS_ZONE_OUT_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t), \
                                outClusters, outClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
}

#define ZBPRO_INIT_COMBINDED_IN_CLUSTER_LIST  ZBPRO_ZCL_CLUSTER_ID_BASIC, ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_IAS_ACE
#define ZBPRO_INIT_COMBINDED_OUT_CLUSTER_LIST ZBPRO_ZCL_CLUSTER_ID_IDENTIFY, ZBPRO_ZCL_CLUSTER_ID_ONOFF, ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK, \
                                              ZBPRO_ZCL_CLUSTER_ID_IAS_ACE, ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE, ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT, \
                                              ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT

void zbPro_HA_Init_Combinded_SimpleDescriptor(uint8_t endpoint, ZBPRO_APS_SimpleDescriptor_t *descr)
{
    memset(descr, 0, sizeof(ZBPRO_APS_SimpleDescriptor_t));
    uint8_t inClusterAmount = VA_NARGS(ZBPRO_INIT_COMBINDED_IN_CLUSTER_LIST);
    uint8_t outClusterAmount = VA_NARGS(ZBPRO_INIT_COMBINDED_OUT_CLUSTER_LIST);
    ZBPRO_INIT_IAS_ACE_SIMPLE_DESCRIPTOR(descr, endpoint, inClusterAmount, outClusterAmount);

    SYS_MemAlloc(&descr->inOutClusterList, (inClusterAmount + outClusterAmount) * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t inClusters[] = { ZBPRO_INIT_COMBINDED_IN_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, 0, inClusters, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
    const uint16_t outClusters[] = { ZBPRO_INIT_COMBINDED_OUT_CLUSTER_LIST };
    SYS_CopyToPayload(&descr->inOutClusterList, inClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t), \
                                outClusters, outClusterAmount * sizeof(ZBPRO_ZCL_ClusterId_t));
}

#if 0

static ZIGBEE_HA_Status Zigbee_HA_OnOffCmdOn_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_OnOffCmdOn_Indirect_Callback(ZBPRO_ZCL_OnOffCmdReqDescr_t *const reqDescr,
        ZBPRO_ZCL_OnOffCmdReqParams_t *const confParams)
    {
        status = confParams->status;
        free(confParams);
    }

    ZBPRO_ZCL_OnOffCmdReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    ZBPRO_ZCL_OnOffCmdReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = dstEndpoint;
    reqDescr->callback = zigbee_HA_OnOffCmdOn_Indirect_Callback;
    ZBPRO_ZCL_OnOffCmdOnReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_HA_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;
}


static ZIGBEE_HA_Status Zigbee_HA_OnOffCmdOn(ZBPRO_APS_ExtAddr_t srcExtAddr, ZBPRO_APS_ExtAddr_t dstExtAddr, ZBPRO_APS_ClusterId_t clusterId, ZBPRO_APS_EndpointId_t srcEndpoint, ZBPRO_APS_EndpointId_t dstEndpoint)
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;
    void zigbee_HA_Bind_Callback(ZBPRO_ZCL_OnOffCmdReqDescr_t *const reqDescr,
        ZBPRO_ZCL_OnOffCmdReqParams_t *const confParams)
    {
        status = confParams->status;
        free(confParams);
    }

    ZBPRO_ZCL_OnOffCmdReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_OnOffCmdReqDescr_t));
    ZBPRO_ZCL_OnOffCmdReqParams_t *params = &reqDescr->params;
    INIT_BIND_PARAMS(params, srcExtAddr, dstExtAddr, clusterId, srcEndpoint, dstEndpoint);
    reqDescr->callback = zigbee_HA_Bind_Callback;
    ZBPRO_APS_BindReq(reqDescr);
    while(status == 0xff);
    free(reqDescr);  /* TODO: potential bug */
    return status ? (ZIGBEE_HA_Status)(ZBPRO_ZDO_MODULE_ID | status) : ZIGBEE_HA_STATUS_SUCCESS;

}
#endif

/* debug message helper function implementation */
static void Sys_DbgPrint(const char *format, ...)
{
#ifdef  _DEBUG_
    #define SYS_DBG_PRINT_BUFFER_SIZE  255
    char     message[SYS_DBG_PRINT_BUFFER_SIZE];      /* String buffer for the message to be logged. */
    va_list  args;                                  /* Pointer to the variable arguments list of this function. */

    va_start(args, format);
    vsnprintf(message, SYS_DBG_PRINT_BUFFER_SIZE, format, args);                                                              /* TODO: Implement custom tiny formatted print. */
    va_end(args);

    printf(message);
#endif
}

#endif
