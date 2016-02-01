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
/* Simple RF4CE app */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
#include "bbMailTestEngine.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_ha_common.h"
# pragma GCC optimize "short-enums"     /* Implement short enums. */
typedef enum _TEST_enum_t
{
    TEST_ENUM_1 = 0x00,
} TEST_enum_t;

SYS_DbgAssertStatic(sizeof(TEST_enum_t) == 1)

#define SYS_DBG_LOG_BUFFER_SIZE     256

#  define HAL_DbgLogStr(message)                                TEST_DbgLogStr(message)

void TEST_DbgLogStr(const char *const message)
{
    printf(message);
    fflush(stdout);
}

void sysDbgHalt(const uint32_t errorUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                           , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                 )
{
    HAL_IRQ_DISABLE();
# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: %s(%d) - 0x%08X", fileName, fileLine, errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: 0x%08X", errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */

    while(1);
}



#if defined(_DEBUG_LOG_)
/*
 * Logs warning and proceeds with program execution.
 */
void sysDbgLogId(const uint32_t warningUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                              , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                    )
{
# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: %s(%d) - 0x%08X", fileName, fileLine, warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: 0x%08X", warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */
}
#endif /* _DEBUG_LOG_ */


#if defined(_DEBUG_LOG_) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
/*
 * Logs custom formatted debugging string with auxiliary parameters.
 */
void sysDbgLogStr(const char *const format, ...)
{
    char     message[SYS_DBG_LOG_BUFFER_SIZE];      /* String buffer for the message to be logged. */
    va_list  args;                                  /* Pointer to the variable arguments list of this function. */

    va_start(args, format);
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args);                                                              /* TODO: Implement custom tiny formatted print. */
    va_end(args);

    HAL_DbgLogStr(message);
}
#endif

uint32_t TEST_DbgAssert(uint32_t errorUid, const char *fileName, uint16_t line)
{
    char message[200];
    snprintf(message, sizeof(message), "Not expected Assert(%#010x) has been called. \"%s\", L%d", errorUid, fileName, line);
    printf(message);
    return 0;
}

/*************************************************************************************//**
  \brief Logs error and proceeds with program execution.
  \param[in] errorUid - Error identifier corresponding to the mismatched
        expression being asserted.
*****************************************************************************************/
uint32_t HAL_DbgLogId(uint32_t errorUid, const char *fileName, uint16_t line)
{
    TEST_DbgAssert(errorUid, fileName, line);
    return 0;
}


uint32_t HAL_DbgHalt(uint32_t errorUid, const char *fileName, uint16_t line)
{
    HAL_DbgLogId(errorUid, fileName, line);
    return 0;
}

static void dumpSimpleDescr(ZBPRO_APS_SimpleDescriptor_t *simpleDescr)
{
    Sys_DbgPrint("deviceId:         %04X\n", simpleDescr->deviceId);
    Sys_DbgPrint("profileId:        %04X\n", simpleDescr->profileId);
    Sys_DbgPrint("endpoint:         %02X\n", simpleDescr->endpoint);
    Sys_DbgPrint("deviceVersion:    %02X\n", simpleDescr->deviceVersion);
    Sys_DbgPrint("inClusterAmount:  %02X\n", simpleDescr->inClusterAmount);
    Sys_DbgPrint("outClusterAmount: %02X\n", simpleDescr->outClusterAmount);

    ZBPRO_APS_ClusterId_t *inOutClusterList = ALLOCA((simpleDescr->inClusterAmount + simpleDescr->outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
    SYS_CopyFromPayload(inOutClusterList, &simpleDescr->inOutClusterList, 0, (simpleDescr->inClusterAmount + simpleDescr->outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
    Sys_DbgPrint("[In]:\n");
    for(int in = 0; in < simpleDescr->inClusterAmount; in++)
        Sys_DbgPrint("                  %04X\n", inOutClusterList[in]);
    Sys_DbgPrint("\n[out]:\n");
    for(int out = 0; out < simpleDescr->outClusterAmount; out++)
        Sys_DbgPrint("                  %04X\n", inOutClusterList[simpleDescr->inClusterAmount + out]);
    Sys_DbgPrint("\n");
}

static void clearInput()
{
    int c;
    do {
        c = getchar();
    }while(c != '\n' && c != EOF);
}

typedef enum _Join_Status_t{
    UNJOINED = 0,
    JOINING,
    JOINED,
}Join_Status;

static Join_Status status_Joined = UNJOINED;


static ZBPRO_NWK_ExtAddr_t remoteDeviceExtAddr = 0;
static ZBPRO_NWK_NwkAddr_t remoteDeviceNwkAddr = 0;

static void My_SYS_EventNtfy(SYS_EventNotifyParams_t *event)
{
    Sys_DbgPrint("\nSYS_EventNtfy : EventId(%d)\n", event->id);
    if(event->id == ZBPRO_NEW_CHILD_EID){
        status_Joined = JOINED;
        ZBPRO_NWK_New_Child_Params *params = (ZBPRO_NWK_New_Child_Params *)event->data;
        memcpy(&remoteDeviceExtAddr, &params->extAddr, sizeof(remoteDeviceExtAddr));
        //remoteDeviceNwkAddr = params->networkAddr;
    }
}

static bool hasDeviceJoined()
{
    return status_Joined == JOINED;
}
typedef struct PACKED _ZBPRO_ZCL_2BytesSignedAttributeReport_t
{
   ZBPRO_ZCL_ProfileWideCmdAttributeReport_t    attribute;

   short                                        attributeData;

} ZBPRO_ZCL_2BytesSignedAttributeReport_t;

static void My_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd(ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t *indication)
{
    // 2 bytes for signed short 16-bit integer
    int recordLen = sizeof(ZBPRO_ZCL_2BytesSignedAttributeReport_t);
    int recordNum = SYS_GetPayloadSize(&indication->payload)/recordLen;
    ZBPRO_ZCL_2BytesSignedAttributeReport_t *records = ALLOCA(SYS_GetPayloadSize(&indication->payload));
    SYS_CopyFromPayload(records, &indication->payload, 0, SYS_GetPayloadSize(&indication->payload));
    for(int i = 0; i < recordNum; i++){
        uint16_t u16Data;
        printf("Attribute ID:%04x\n", records[i].attribute.attributeID);
        printf("Attribute Data:%d\n", records[i].attributeData);
    }
}

static void My_Zigbee_HA_Read_Attributes_Direct_Async_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
    ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
{
    int payloadSize = 0;
    uint8_t *payload = NULL;
    int recordLen = sizeof(short);
    int recordNum = SYS_GetPayloadSize(&confParams->payload)/recordLen;
    payloadSize = SYS_GetPayloadSize(&confParams->payload);
    printf("%s got callback  attributeSize:%d status:%d\n", __FUNCTION__, payloadSize, confParams->zclObligatoryPart.overallStatus);
    payload = alloca(payloadSize);
    short *records = (short*)payload;
    SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
    for(int i = 0; i < recordNum; i++){
        uint16_t u16Data;
        printf("Attribute Data:%d\n", records[i]);
    }
}

static ZIGBEE_HA_Status Zigbee_HA_Read_Attributes_Direct_Async(ZBPRO_APS_EndpointId_t srcEndpoint,
                                                            ZBPRO_APS_EndpointId_t dstEndpoint,
                                                            ZBPRO_APS_ExtAddr_t dstExtAddr,
                                                            ZBPRO_ZCL_ClusterId_t  clusterId,
                                                            ZBPRO_ZCL_AttributeId_t attributeID,
                                                            SYS_Time_t timeout
                                                            )
{
    ZBPRO_ZDO_Status_t status = (ZBPRO_ZDO_Status_t)0xff;

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
    params->zclObligatoryPart.useDefaultResponse = ZBPRO_ZCL_RESPONSE_TYPE_SPECIFIC;
    params->attributeId = attributeID;
    reqDescr->callback = My_Zigbee_HA_Read_Attributes_Direct_Async_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    free(reqDescr);  /* TODO: potential bug */
    return ZIGBEE_HA_STATUS_SUCCESS;
}

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;
    ZbProSspKey_t nwkKey;
    uint32_t retStatus;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */

    //tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = NULL;
    zcb.RF4CE_ZRC2_CheckValidationInd = NULL;
    zcb.RF4CE_ZRC2_ControlCommandInd = NULL;
    zcb.RF4CE_ZRC1_ControlCommandInd = NULL;
    zcb.SYS_EventNtfy = My_SYS_EventNtfy;
    zcb.ZBPRO_ZCL_ProfileWideCmdReportAttributesInd = My_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd;
    Zigbee_Open(&zcb, argv[1]);

    ZBPRO_APS_ExtAddr_t smartPlugExtAddr = 0x000B52000000646EUL;
    ZBPRO_APS_ExtAddr_t geBulbExtAddr = 0x7CE5240000127D5AUL;
    ZBPRO_APS_ExtAddr_t myExtAddr = 0x0010187623cdUL;

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_apsChannelMask\n");
    Zigbee_HA_Set_Attribute_apsChannelMask(1 << 19);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_macExtendedAddress\n");
    Zigbee_HA_Set_Attribute_macExtendedAddress(myExtAddr);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_zdoInitialSecurityStatus\n");
    Zigbee_HA_Set_Attribute_zdoInitialSecurityStatus(ZBPRO_APS_PRECONFIGURED_TRUST_CENTER_KEY);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_nwkDeviceType\n");
    Zigbee_HA_Set_Attribute_nwkDeviceType(ZBPRO_DEVICE_TYPE_COORDINATOR);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_apsTrustCenterAddress\n");
    Zigbee_HA_Set_Attribute_apsTrustCenterAddress(myExtAddr);

    Sys_DbgPrint("Zigbee_HA_Set_NWK_Key\n");
    memcpy(nwkKey.raw, "\x5A\x69\x67\x42\x65\x65\x41\x6C\x6C\x69\x61\x6E\x63\x65\x30\x39", sizeof(nwkKey.raw));
    Zigbee_HA_Set_NWK_Key(&nwkKey, 0);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_nwkActiveKeySeqNumber\n");
    Zigbee_HA_Set_Attribute_nwkActiveKeySeqNumber(0);

    Sys_DbgPrint("Zigbee_HA_StartNetwork\n");
    Zigbee_HA_StartNetwork();

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_zdoPermitJoinDuration\n");
    Zigbee_HA_PermitJoining(0xff);

    ZBPRO_APS_SimpleDescriptor_t localSimpleDescr;
    zbPro_HA_Init_Combinded_SimpleDescriptor(0x0B, &localSimpleDescr);
    dumpSimpleDescr(&localSimpleDescr);
    Zigbee_HA_EndpointRegister(&localSimpleDescr);

#if 1
        Sys_DbgPrint("Waiting for remote device to join\n");
        while(!hasDeviceJoined())
        sleep(0);

    Zigbee_HA_PermitJoining(0x0);
        Sys_DbgPrint("Got a new joined device, wait for about 5 seconds for normal operation\n");
    sleep(5);
#endif

    Zigbee_HA_Set_Attribute_zdoZdpResponseTimeout(5000);
    if(Zigbee_HA_GetNwkAddr(remoteDeviceExtAddr, &remoteDeviceNwkAddr) != ZIGBEE_HA_STATUS_SUCCESS)
        goto  _exit;
    Sys_DbgPrint("remoteDeviceNwkAddr : %04x\n", remoteDeviceNwkAddr);
    uint8_t activeEpCount;
    ZBPRO_APS_EndpointId_t *activeEpList;
    Sys_DbgPrint("Zigbee_HA_GetActiveEp\n");
    Zigbee_HA_GetActiveEp(remoteDeviceNwkAddr, remoteDeviceNwkAddr, &activeEpCount, &activeEpList, 5000);

    ZbProZdoNodeDescriptor_t nodeDescr;
    Zigbee_HA_GetNodeDesc(remoteDeviceNwkAddr, remoteDeviceNwkAddr, &nodeDescr, 5000);

    ZBPRO_APS_SimpleDescriptor_t  simpleDescr;
    uint8_t length;
    for(int i = 0; i < activeEpCount; i++){
        Sys_DbgPrint("Endpoint %d:\n", activeEpList[i]);
        Zigbee_HA_GetSimpleDesc(remoteDeviceNwkAddr, remoteDeviceNwkAddr, activeEpList[i], &length, &simpleDescr, 5000);
        dumpSimpleDescr(&simpleDescr);
    }

    free(activeEpList);

    //if(ZIGBEE_HA_STATUS_SUCCESS != (retStatus = Zigbee_HA_Bind(myExtAddr, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT/*ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT*/, 1, 0x0a)))
    //    Sys_DbgPrint("Failed to call Zigbee_HA_Bind %08x:\n", retStatus);

    //static ZIGBEE_HA_Status Zigbee_HA_ConfigureReporting_Indirect(ZBPRO_APS_EndpointId_t srcEndpoint,
    //                                                          ZBPRO_APS_EndpointId_t dstEndpoint,
    //                                                          ZBPRO_ZCL_ClusterId_t   clusterId,
    //                                                          ZBPRO_ZCL_AttributeId_t attributeID,
    //                                                          ZBPRO_ZCL_AttrDataType_t attributeDataType,
    //                                                          ZBPRO_ZCL_AttribureReportingInterval_t minReportingInterval,
    //                                                          ZBPRO_ZCL_AttribureReportingInterval_t maxReportingInterval
    //                                                          )
    uint16_t valueChanged = 1;
    //Zigbee_HA_ConfigureReporting_Direct(0x0B, 0x0A, remoteDeviceExtAddr,ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT,
    //                                      0, ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_16BIT, 2, (uint8_t*)&valueChanged, 1, 3, 5000);

    //Zigbee_HA_ConfigureReporting_Direct(0x0B, 0x0A, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT,
    //                                      0, ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_16BIT, 2, (uint8_t*)&valueChanged, 200, 300, 5000);

    //while(1);
    while(1){

        int attributeSize;
        uint8_t *attribute;
        //Zigbee_HA_Read_Attributes_Direct(0x0B, 0x0a, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT, 0, &attributeSize, &attribute, 5000);
        //Zigbee_HA_Read_Attributes_Direct_Async(0x0B, 0x0a, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT, 0, 3);  // 3s timeout
        getchar();
        printf("Zigbee_HA_Read_Attributes_Direct_Async\n");
        Zigbee_HA_Read_Attributes_Direct_Async(0x0B, 0x0a, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT, 0, 3);  // 3s timeout
        //printf("length:%d value:%d\n",attributeSize,  *(uint16_t*)attribute);
    }
    return 0;
_exit:
    printf("MY_HA_ONOFF_CONTROLLER_APP:  closing...\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}
