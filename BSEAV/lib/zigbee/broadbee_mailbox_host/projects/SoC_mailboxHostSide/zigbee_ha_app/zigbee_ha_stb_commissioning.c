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
/* Simple RF4CE app */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
//#include "bbMailTestEngine.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_ha_common.h"
# pragma GCC optimize "short-enums"     /* Implement short enums. */


/*
*  Function declarations
*/
static const uint8_t *getClusterDescr(ZBPRO_APS_ClusterId_t clusterId);
static void restoreSignalHandler();
static void installSignalHandler();
static bool shouldExit();
static void resetExitflag();


/*
*  Function implementations
*/
typedef enum _Join_Status_t{
    UNJOINED = 0,
    JOINING,
    JOINED,
}Join_Status;

typedef struct JoinedDeviceCommissioningInfo_t{
    ZBPRO_NWK_ExtAddr_t extAddr;
    ZBPRO_NWK_NwkAddr_t nwkAddr;
    ZBPRO_APS_ClusterId_t   clusterId;
    ZBPRO_APS_EndpointId_t  endpointSrcId;
    ZBPRO_APS_EndpointId_t  endpointId;
    ZbProZdoNodeDescriptor_t    nodeDescr;
    Join_Status             nodeStatus;
    char                    deviceFriendName[50];
}JoinedDeviceCommissioningInfo;

#define MAX_ALLOWED_DEVICE_NUM  10
#define INVALID_INDEX           -1
#define MY_HARD_CODED_ENDPOINT_NUMBER   0x0B

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

#define IS_ONOFF(clusterId) ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_ONOFF)
#define IS_LEVEL(clusterId) ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_LEVEL_CONTROL)
#define IS_LOCK(clusterId)  ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK)
#define IS_ZONE(clusterId)  ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE)
#define IS_TEMPERATURE_MEASUREMENT(clusterId)  ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT)
#define IS_HUMIDITY_MEASUREMENT(clusterId)  ((clusterId) == ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT)

static pthread_t createNewThread(void (*func)(void*), void* param)
{
    pthread_attr_t threadAttr;
    pthread_t      threadId;
    struct sched_param schedParam;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    pthread_attr_getschedparam(&threadAttr, &schedParam);
    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&threadAttr, &schedParam);
    pthread_attr_setstacksize(&threadAttr, 8*1024);
    pthread_create(&threadId,
                        &threadAttr,
                        (void * (*)(void *))func,
                        param);
    return threadId;
}

static bool clusterSupported(ZBPRO_APS_ClusterId_t clusterId)
{
    return IS_ONOFF(clusterId) || IS_LEVEL(clusterId) || IS_LOCK(clusterId) || IS_ZONE(clusterId) || IS_TEMPERATURE_MEASUREMENT(clusterId) || IS_HUMIDITY_MEASUREMENT(clusterId);
}

static bool deviceSupported(ZBPRO_APS_SimpleDescriptor_t *simpleDescr)
{
    bool supported = false;
    ZBPRO_APS_ClusterId_t *inOutClusterList = ALLOCA((simpleDescr->inClusterAmount + simpleDescr->outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
    SYS_CopyFromPayload(inOutClusterList, &simpleDescr->inOutClusterList, 0, (simpleDescr->inClusterAmount + simpleDescr->outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
    for(int in = 0; in < simpleDescr->inClusterAmount; in++){
        if(clusterSupported(inOutClusterList[in])){
            supported = true;
            break;
        }
    }

    return supported;
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

static Join_Status status_Joined = UNJOINED;


static ZBPRO_NWK_ExtAddr_t remoteDeviceExtAddr = 0;
static ZBPRO_NWK_NwkAddr_t remoteDeviceNwkAddr = 0;

static JoinedDeviceCommissioningInfo commissioningInfoTable[MAX_ALLOWED_DEVICE_NUM];

#define INVALID_EXT_ADDR(addr)    ((addr) == 0)
static int findCommissioningInfoTableEntry(ZBPRO_NWK_ExtAddr_t extAddr, ZBPRO_APS_ClusterId_t clusterId)
{
    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++)
        if((commissioningInfoTable[iter].extAddr == extAddr) && (commissioningInfoTable[iter].clusterId == clusterId))
            return iter;

    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++)
        if(INVALID_EXT_ADDR(commissioningInfoTable[iter].extAddr))
            return iter;

    return INVALID_INDEX;
}

/*
* Note :
*   This function maybe update the current table instead of insertion to guarantee
*   the combinantion of extend address and cluster id is unique in the table
*/
static void insertCommissioningInfo(JoinedDeviceCommissioningInfo *info)
{
    int index = findCommissioningInfoTableEntry(info->extAddr, info->clusterId);
    if(index != INVALID_INDEX)
        memcpy(&commissioningInfoTable[index], info, sizeof(JoinedDeviceCommissioningInfo));
    else
        Sys_DbgPrint("\nThe commissioning information table is full\n");
}

inline static ZBPRO_APS_ClusterId_t getClusterId(uint8_t index)
{
    return commissioningInfoTable[index].clusterId;
}

inline static ZBPRO_APS_EndpointId_t getSrcEndpoint(uint8_t index)
{
    return commissioningInfoTable[index].endpointSrcId;
}

inline static ZBPRO_APS_EndpointId_t getDstEndpoint(uint8_t index)
{
    return commissioningInfoTable[index].endpointId;
}

inline static ZBPRO_NWK_ExtAddr_t getDstExtAddr(uint8_t index)
{
    return commissioningInfoTable[index].extAddr;
}

inline static ZBPRO_NWK_NwkAddr_t getDstNwkAddr(uint8_t index)
{
    return commissioningInfoTable[index].nwkAddr;
}

inline static bool isSleepyDevice(uint8_t index)
{
    return !commissioningInfoTable[index].nodeDescr.macCapabilityFlags.rxOnWhenIdle;
}

static void printCommissioningInfo()
{
    uint8_t msg[100] = {0};
    Sys_DbgPrint("\n\n\nAll device information:\n\n");
    //field length  10          20                 20                 10        ....
    Sys_DbgPrint("Index     Device Name         NWK Address         Endpoint  Cluster\n");
    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++){
        if(INVALID_EXT_ADDR(commissioningInfoTable[iter].extAddr))
            continue;
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "%d:", iter);
        sprintf(msg + 10, "%s", commissioningInfoTable[iter].deviceFriendName);
        sprintf(msg + 30, "%04X", commissioningInfoTable[iter].nwkAddr);
        sprintf(msg + 50, "%02X", commissioningInfoTable[iter].endpointId);
        sprintf(msg + 60, "%s", getClusterDescr(commissioningInfoTable[iter].clusterId));
        for(int i = 0; i < sizeof(msg) - 1; i ++)
        {
            if(msg[i] == 0)
               msg[i] = ' ';
        }
        Sys_DbgPrint("%s\n", msg);
    }
}

static bool saveCommissioningInfo()
{
    FILE *f = fopen("/etc/zigbee/commission.bin", "wb");
    if(f == NULL){
        Sys_DbgPrint("Failed to open file %s for writing\n", "commission.bin");
        return false;
    }
    fwrite(commissioningInfoTable, sizeof(commissioningInfoTable), 1, f);
    fflush(f);
    fclose(f);
    return true;
}

static bool retrieveCommissioningInfo()
{
    FILE *f = fopen("/etc/zigbee/commission.bin", "rb");
    if(f == NULL){
        Sys_DbgPrint("Failed to open file %s for reading\n", "commission.bin");
        return false;
    }
    fread(commissioningInfoTable, sizeof(commissioningInfoTable), 1, f);
    fclose(f);
    return true;
}

static bool updateNwkAddress(ZBPRO_NWK_ExtAddr_t extAddr, ZBPRO_NWK_NwkAddr_t nwkAddr)
{
    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++){
        if(commissioningInfoTable[iter].extAddr == extAddr){
            commissioningInfoTable[iter].nwkAddr = nwkAddr;
            return true;
        }
    }
    return false;
}

static void updateNodeStatus(ZBPRO_NWK_ExtAddr_t extAddr)
{
    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++){
        if(commissioningInfoTable[iter].extAddr == extAddr){
            commissioningInfoTable[iter].nodeStatus = JOINED;
        }
    }
}

static bool isNodeJoined(ZBPRO_NWK_ExtAddr_t extAddr)
{
    bool retValue =  false;
    for(int iter = 0; iter < MAX_ALLOWED_DEVICE_NUM; iter++){
        if(commissioningInfoTable[iter].extAddr == extAddr){
            retValue = commissioningInfoTable[iter].nodeStatus == JOINED;
        }
    }
    return retValue;
}

static void My_SYS_EventNtfy(SYS_EventNotifyParams_t *event)
{
    Sys_DbgPrint("\nSYS_EventNtfy : EventId(%d)\n", event->id);
    if(event->id == ZBPRO_NEW_CHILD_EID){
        status_Joined = JOINED;
        ZBPRO_NWK_New_Child_Params *params = (ZBPRO_NWK_New_Child_Params *)event->data;
        memcpy(&remoteDeviceExtAddr, &params->extAddr, sizeof(remoteDeviceExtAddr));
        updateNodeStatus(remoteDeviceExtAddr);
        //remoteDeviceNwkAddr = params->networkAddr;
    }
}

static void My_ZBPRO_ZDO_DeviceAnnceInd(ZBPRO_ZDO_DeviceAnnceIndParams_t *indication)
{
    Sys_DbgPrint("\nMy_ZBPRO_ZDO_DeviceAnnceInd\n");
    Sys_DbgPrint("The device(%016llX:%04X) has sent announcement\n", indication->extAddr, indication->nwkAddr);
}

static bool hasDeviceJoined()
{
    return status_Joined == JOINED;
}

static void clearJoinedInformation()
{
    status_Joined = UNJOINED;
    remoteDeviceExtAddr = 0;
    remoteDeviceNwkAddr = 0;
}

static uint8_t *getInputString()
{
    uint8_t *word = malloc(256);
    scanf("%s" , word);
    while(getchar() != '\n');
    return word;
}

static uint32_t getInputInteger()
{
    uint32_t integer = 0;
    scanf("%u" , &integer);
    while(getchar() != '\n');
    return integer;
}

static uint8_t getInputChar()
{
    uint8_t ch = getchar();
    while(getchar() != '\n');
    return ch;
}

static const uint8_t *getClusterDescr(ZBPRO_APS_ClusterId_t clusterId)
{
    uint8_t *clusterDescr = "Non-support Cluster";
    switch(clusterId){
        case ZBPRO_ZCL_CLUSTER_ID_ONOFF:
            clusterDescr = "On/Off Cluster";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_LEVEL_CONTROL:
            clusterDescr = "Level Cluster";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK:
            clusterDescr = "Door Locl Cluster";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE:
            clusterDescr = "IAS Zone Cluster";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT:
            clusterDescr = "Temperature Measurement Cluster";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT:
            clusterDescr = "Humidity Measurement Cluster";
            break;
    }
    return clusterDescr;
}

static const uint8_t *getExampleDeviceName(ZBPRO_APS_ClusterId_t clusterId)
{
    uint8_t *deviceName = "Non-support Device";
    switch(clusterId){
        case ZBPRO_ZCL_CLUSTER_ID_ONOFF:
            deviceName = "OnOff Controller";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_LEVEL_CONTROL:
            deviceName = "Level Controller";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK:
            deviceName = "Door Lock Controller";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE:
            deviceName = "Secruity Sensor";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT:
            deviceName = "Temperature Sensor";
            break;
        case ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT:
            deviceName = "Humidity Sensor";
            break;
    }
    return deviceName;
}

static void getOnOffState(uint8_t index)
{
#define ONOFF_ATTR_ID  0
#define ONOFF_ATTR_LENGTH  1

    int      attributeSize;
    uint8_t  *attributeData;
    Zigbee_HA_Read_Attributes_Direct(getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index),
                                     ZBPRO_ZCL_CLUSTER_ID_ONOFF, ONOFF_ATTR_ID, &attributeSize, &attributeData, 5);

    if(attributeSize == ONOFF_ATTR_LENGTH){
        Sys_DbgPrint(*(uint8_t*)attributeData == 1 ? "ON":"OFF");
        Sys_DbgPrint("\n\n");
    }
    free(attributeData);
}

static void testLoopForOnOff(uint8_t index)
{
    Sys_DbgPrint("Going to OnOff test loop, press 'Ctrl - C' to exit from this app\n");
    resetExitflag();
    //installSignalHandler();
    do{
        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("On?\n");
        {
            Zigbee_HA_OnOffCmdOn_Direct(1, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index), 5);
        }

        getOnOffState(index);

        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("Off?\n");
        {
            Zigbee_HA_OnOffCmdOn_Direct(0, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index), 5);
        }

        getOnOffState(index);
    }while(1);
    restoreSignalHandler();
}


static void getDoorLockState(uint8_t index)
{
#define LOCKSTATE_ATTR_ID  0
#define LOCKSTATE_ATTR_LENGTH  1

#define LOCKSTATE_NOT_FULLY_LOCKED  00
#define LOCKSTATE_LOCKED  01
#define LOCKSTATE_UNLOCKED  02

    int      attributeSize;
    uint8_t  *attributeData;
    Zigbee_HA_Read_Attributes_DirectByNwk(getSrcEndpoint(index), getDstEndpoint(index), getDstNwkAddr(index),
                                     ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK, LOCKSTATE_ATTR_ID, &attributeSize, &attributeData, 5);

    if(attributeSize == LOCKSTATE_ATTR_LENGTH){
        switch(*(uint8_t*)attributeData){
            case LOCKSTATE_NOT_FULLY_LOCKED:
                Sys_DbgPrint("Not fully locked\n");
                break;
            case LOCKSTATE_LOCKED:
                Sys_DbgPrint("Locked\n");
                break;
            case LOCKSTATE_UNLOCKED:
                Sys_DbgPrint("Unlocked\n");
                break;
        }
    }
    free(attributeData);
}

static void testLoopForDoorLock(uint8_t index)
{
    ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest;
    if(isSleepyDevice(index)){
        Sys_DbgPrint("If you did a power cycle, it's better to wait for its joining\n");
        Sys_DbgPrint("Do you want to wait for its joining, Yes or No?[y/n]\n");
        if(getInputChar() == 'y'){
            Zigbee_HA_PermitJoining(0xff);
            sleep(10);
            Zigbee_HA_PermitJoining(0x0);
        }
        Zigbee_HA_Set_Attribute_macTransactionPersistenceTime(520);  // about 8s MAC transaction persistent time
    }
    Sys_DbgPrint("Going to DoorLock test loop, press 'Ctrl - C' to exit from this app\n");
    resetExitflag();
    //installSignalHandler();
    while(1){
        if((ZIGBEE_HA_STATUS_SUCCESS != Zigbee_HA_GetNwkAddr(getDstExtAddr(index), &nwkAddrOfInterest)) || (nwkAddrOfInterest == 0)){
            Sys_DbgPrint("Failed to get NWK address, please try again later\n");
            return;
        }

        if(updateNwkAddress(getDstExtAddr(index), nwkAddrOfInterest)){
            Sys_DbgPrint("The network address(%04X) of existing device is being updated\n", nwkAddrOfInterest);
            saveCommissioningInfo();
        }
        #define LOCKSTATE_ATTR_ID  0
        #define LOCKSTATE_ATTR_LENGTH  1

        int      attributeSize;
        uint8_t  *attributeData;
        if(ZIGBEE_HA_STATUS_SUCCESS == Zigbee_HA_Read_Attributes_DirectByNwk(getSrcEndpoint(index), getDstEndpoint(index), getDstNwkAddr(index),
                                     ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK, LOCKSTATE_ATTR_ID, &attributeSize, &attributeData, 5))
            break;
        sleep(10);
    }
    do{
        if(shouldExit())
            break;
        Sys_DbgPrint("Lock?\n");
        {
            Zigbee_HA_DoorLockCmdLockUnlock_DirectByNwkAddr(1, getSrcEndpoint(index), getDstEndpoint(index), getDstNwkAddr(index), 5);
            //Zigbee_HA_DoorLockCmdLockUnlock_Direct(1, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index), 5000);
        }
        sleep(1); // wait for the slow action for lock/unlock
        getDoorLockState(index);


        sleep(1);
        if(shouldExit())
            break;
        Sys_DbgPrint("Unlock?\n");
        {
           Zigbee_HA_DoorLockCmdLockUnlock_DirectByNwkAddr(0, getSrcEndpoint(index), getDstEndpoint(index), getDstNwkAddr(index) ,5);
           //Zigbee_HA_DoorLockCmdLockUnlock_Direct(0, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index) ,5000);
        }
        sleep(1); // wait for the slow action for lock/unlock
        getDoorLockState(index);

        sleep(1);
    }while(1);
    restoreSignalHandler();
}

#include <time.h>
static void dumpTimeStamp(const char *prompt)
{
  time_t curtime;
  struct tm *loctime;
  char buffer[256];
  /* Get the current time. */
  curtime = time (NULL);

  /* Convert it to local time representation. */
  loctime = localtime (&curtime);

  /* Print out the date and time in the standard format. */
  Sys_DbgPrint("%s", prompt);
  strftime(buffer, 256, "%F %X", loctime);
  Sys_DbgPrint(" %s ", buffer);
}


/************************************ Temperature Sensor test section **********************************************/
#define TEMPERATURE_ATTR_ID  0
#define TEMPERATURE_ATTR_LENGTH  2

static void testLoopForTemperatureSensor(uint8_t index)
{
    Sys_DbgPrint("Going to TemperatureSensor test loop, press 'Ctrl - C' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("Get temperature?\n");
        {
	    dumpTimeStamp("Time:");
            Sys_DbgPrint("Geting................\n");
            int attributeSize;
            uint8_t *attribute;
            Zigbee_HA_Read_Attributes_Direct(getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index), \
                                             ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT, TEMPERATURE_ATTR_ID, &attributeSize, &attribute, 330);
	    dumpTimeStamp("Time:");
            if(attributeSize == TEMPERATURE_ATTR_LENGTH){
                Sys_DbgPrint("Temperature : %d\n", *(short*)attribute);
                free(attribute);
            }
        }
    }while(1);
    restoreSignalHandler();
}

static bool status_Got_Temperature = false;
static void My_Zigbee_HA_Read_Temperature_Direct_Async_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
    ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
{
    int payloadSize = 0;
    uint8_t *payload = NULL;
    int recordLen = sizeof(short);
    int recordNum = SYS_GetPayloadSize(&confParams->payload)/recordLen;
    payloadSize = SYS_GetPayloadSize(&confParams->payload);
    payload = alloca(payloadSize);
    short *records = (short*)payload;
    SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
    for(int i = 0; i < recordNum; i++){
        Sys_DbgPrint("Temperature : %d", records[i]);
        if(confParams->zclObligatoryPart.remoteApsAddress.addrMode == APS_EXTENDED_ADDRESS_MODE)
            Sys_DbgPrint("From Address : %016llX\n", confParams->zclObligatoryPart.remoteApsAddress.extAddr);
        if(confParams->zclObligatoryPart.remoteApsAddress.addrMode == APS_SHORT_ADDRESS_MODE)
            Sys_DbgPrint("From Address : %04X\n", confParams->zclObligatoryPart.remoteApsAddress.shortAddr);
    }
    status_Got_Temperature = true;

    /* Issue another read attribute request*/
    //ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
}

static ZIGBEE_HA_Status Zigbee_HA_Read_Temperature_Direct_Async(uint8_t index)
{
    ZBPRO_APS_EndpointId_t srcEndpoint = MY_HARD_CODED_ENDPOINT_NUMBER;

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = getDstExtAddr(index);
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = getDstEndpoint(index);
    params->zclObligatoryPart.clusterId = ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT;
    params->zclObligatoryPart.respWaitTimeout = 5; // 5 Seconds
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->zclObligatoryPart.useDefaultResponse = ZBPRO_ZCL_RESPONSE_TYPE_SPECIFIC;
    params->attributeId = TEMPERATURE_ATTR_ID;
    reqDescr->callback = My_Zigbee_HA_Read_Temperature_Direct_Async_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    free(reqDescr);
    return ZIGBEE_HA_STATUS_SUCCESS;
}

static void zigbee_HA_Read_Temperature_thread(void *param)
{
    Sys_DbgPrint("Going to Temperature Sensor test thread, press 'x' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        uint8_t index = (uint8_t)(int)param;
        Sys_DbgPrint("Sending Read Temperature request\n");
        Zigbee_HA_Read_Temperature_Direct_Async(index);
        while(!status_Got_Temperature && (!shouldExit()))
            sleep(1);
        status_Got_Temperature = false;
        if(shouldExit()){
            break;
        }
        sleep(10);
    }while(1);
    Sys_DbgPrint("Exiting from Read Temperature loop\n");
}


static void testLoopForHumiditySensor(uint8_t index)
{
#define HUMIDITY_ATTR_ID  0
#define HUMIDITY_ATTR_LENGTH  2

    Sys_DbgPrint("Going to HumiditySensor test loop, press 'Ctrl - C' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("Get humidity?\n");
        {
            Sys_DbgPrint("Geting................\n");
            int attributeSize;
            uint8_t *attribute;
            Zigbee_HA_Read_Attributes_Direct(getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index), \
                                             ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT, HUMIDITY_ATTR_ID, &attributeSize, &attribute, 330);
            if(attributeSize == HUMIDITY_ATTR_LENGTH){
                Sys_DbgPrint("Humidity : %d\n", *(uint16_t*)attribute);
                free(attribute);
            }
        }
    }while(1);
    restoreSignalHandler();
}


static bool status_Got_Humidity = false;
static void My_Zigbee_HA_Read_Humidity_Direct_Async_Callback(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const reqDescr,
    ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const confParams)
{
    int payloadSize = 0;
    uint8_t *payload = NULL;
    int recordLen = sizeof(short);
    int recordNum = SYS_GetPayloadSize(&confParams->payload)/recordLen;
    payloadSize = SYS_GetPayloadSize(&confParams->payload);
    payload = alloca(payloadSize);
    uint16_t *records = (short*)payload;
    SYS_CopyFromPayload(payload, &confParams->payload, 0, payloadSize);
    for(int i = 0; i < recordNum; i++){
        Sys_DbgPrint("Humidity : %d  ", records[i], confParams->zclObligatoryPart.remoteApsAddress.addrMode);
        if(confParams->zclObligatoryPart.remoteApsAddress.addrMode == APS_EXTENDED_ADDRESS_MODE)
            Sys_DbgPrint("From Address : %016llX\n", confParams->zclObligatoryPart.remoteApsAddress.extAddr);
        if(confParams->zclObligatoryPart.remoteApsAddress.addrMode == APS_SHORT_ADDRESS_MODE)
            Sys_DbgPrint("From Address : %04X\n", confParams->zclObligatoryPart.remoteApsAddress.shortAddr);
    }
    status_Got_Humidity = true;

    /* Issue another read attribute request*/
    //ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
}

static ZIGBEE_HA_Status Zigbee_HA_Read_Humidity_Direct_Async(uint8_t index)
{
    ZBPRO_APS_EndpointId_t srcEndpoint = MY_HARD_CODED_ENDPOINT_NUMBER;

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  *reqDescr = malloc(sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    memset(reqDescr, 0, sizeof(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t));
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t *params = &reqDescr->params;
    params->zclObligatoryPart.remoteApsAddress.addrMode = APS_EXTENDED_ADDRESS_MODE;
    params->zclObligatoryPart.remoteApsAddress.extAddr = getDstExtAddr(index);
    params->zclObligatoryPart.localEndpoint = srcEndpoint;
    params->zclObligatoryPart.remoteEndpoint = getDstEndpoint(index);
    params->zclObligatoryPart.clusterId = ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT;
    params->zclObligatoryPart.respWaitTimeout = 5; // 5 Seconds
    params->zclObligatoryPart.disableDefaultResp = ZBPRO_ZCL_FRAME_DEFAULT_RESPONSE_ENABLED;
    params->zclObligatoryPart.useDefaultResponse = ZBPRO_ZCL_RESPONSE_TYPE_SPECIFIC;
    params->attributeId = HUMIDITY_ATTR_ID;
    reqDescr->callback = My_Zigbee_HA_Read_Humidity_Direct_Async_Callback;

    ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(reqDescr);
    free(reqDescr);
    return ZIGBEE_HA_STATUS_SUCCESS;
}

static void zigbee_HA_Read_Humidity_thread(void *param)
{
    Sys_DbgPrint("Going to Humidity Sensor test thread, press 'Ctrl - C' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        uint8_t index = (uint8_t)(int)param;
        Sys_DbgPrint("Sending Read Humidity request\n");
        Zigbee_HA_Read_Humidity_Direct_Async(index);
        while(!status_Got_Humidity  && (!shouldExit()))
            sleep(1);
        status_Got_Humidity = false;
        if(shouldExit()){
            break;
        }
        sleep(10);
    }while(1);
    Sys_DbgPrint("Exiting from Read Humidity loop\n");
}


volatile sig_atomic_t flag = 0;

static void my_sig_int_handler(int sig)
{ // can be called asynchronously
    flag = 1; // set flag
}

static void resetExitflag()
{ // can be called asynchronously
    flag = 0; // set flag
}

static bool shouldExit()
{
    return (flag == 1);
}

static void installSignalHandler()
{
    signal(SIGINT, my_sig_int_handler);
}

static void restoreSignalHandler()
{
    signal(SIGINT, SIG_DFL);
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

/*
*
*/
static void generateRandomKey(uint8_t *keyBuf, char keyLength)
{
    for (char c = 0; c < keyLength; c++)
        keyBuf[c] = rand() & 0xff;
}

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;
    ZbProSspKey_t nwkKey;
    uint32_t retStatus;
    ZBPRO_APS_EndpointId_t myEndpoint = MY_HARD_CODED_ENDPOINT_NUMBER;

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
    zcb.ZBPRO_ZDO_DeviceAnnceInd = My_ZBPRO_ZDO_DeviceAnnceInd;
    zcb.ZBPRO_ZCL_ProfileWideCmdReportAttributesInd = My_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd;
    Zigbee_Open(&zcb, argv[1]);

    if(!retrieveCommissioningInfo())
        memset(commissioningInfoTable, 0, sizeof(commissioningInfoTable));

    ZBPRO_APS_ExtAddr_t smartPlugExtAddr = 0x000B52000000646EUL;
    ZBPRO_APS_ExtAddr_t geBulbExtAddr = 0x7CE5240000127D5AUL;
    ZBPRO_APS_ExtAddr_t myExtAddr = 0xeeffaabb187623cdUL;

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_apsChannelMask\n");
    Zigbee_HA_Set_Attribute_apsChannelMask(1 << 22);

    Zigbee_HA_Get_MyMacExtendedAddress(&myExtAddr);
    Sys_DbgPrint("myExtAddr : %016llX\n", myExtAddr);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_macExtendedAddress\n");
    Zigbee_HA_Set_Attribute_macExtendedAddress(myExtAddr);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_zdoInitialSecurityStatus\n");
    Zigbee_HA_Set_Attribute_zdoInitialSecurityStatus(ZBPRO_APS_PRECONFIGURED_TRUST_CENTER_KEY);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_nwkDeviceType\n");
    Zigbee_HA_Set_Attribute_nwkDeviceType(ZBPRO_DEVICE_TYPE_COORDINATOR);

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_apsTrustCenterAddress\n");
    Zigbee_HA_Set_Attribute_apsTrustCenterAddress(myExtAddr);

    ZBPRO_APS_SimpleDescriptor_t localSimpleDescr;
    zbPro_HA_Init_Combinded_SimpleDescriptor(myEndpoint, &localSimpleDescr);
    dumpSimpleDescr(&localSimpleDescr);
    Zigbee_HA_EndpointRegister(&localSimpleDescr);

    Sys_DbgPrint("Press key  't' to go to test procedure\n");
    Sys_DbgPrint("Any any other key continue to enable another device to join\n");

    if(getInputChar() == 't')
        goto case_normal_operation;

    if(FALSE == Zigbee_HA_Check_NWK_Existing()){
        /* Setting NWK key is allowed to be called once in the lift time of the network formed by STB */
        Sys_DbgPrint("Zigbee_HA_Set_NWK_Key\n");
        uint8_t  key[16];
        generateRandomKey(key, 16);
        memcpy(nwkKey.raw, key, sizeof(nwkKey.raw));
        Zigbee_HA_Set_NWK_Key(&nwkKey, 0);
        Sys_DbgPrint("Zigbee_HA_Set_Attribute_nwkActiveKeySeqNumber\n");
        Zigbee_HA_Set_Attribute_nwkActiveKeySeqNumber(0);
    }

    Sys_DbgPrint("Zigbee_HA_StartNetwork\n");
    Zigbee_HA_StartNetwork();

    do{
        Sys_DbgPrint("Zigbee_HA_Set_Attribute_zdoPermitJoinDuration\n");
        Zigbee_HA_PermitJoining(0xff);


        Sys_DbgPrint("Waiting for remote device to join\n");
        while(!hasDeviceJoined())
            sleep(0);

        Zigbee_HA_PermitJoining(0);

        Sys_DbgPrint("Got a new joined device, wait for about 5 seconds for normal operation\n");
        sleep(5);

        Zigbee_HA_GetNwkAddr(remoteDeviceExtAddr, &remoteDeviceNwkAddr);
        Sys_DbgPrint("remoteDeviceNwkAddr : %04X\n", remoteDeviceNwkAddr);
        updateNwkAddress(remoteDeviceExtAddr, remoteDeviceNwkAddr);

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

            ZBPRO_APS_ClusterId_t *inOutClusterList = ALLOCA((simpleDescr.inClusterAmount + simpleDescr.outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
            SYS_CopyFromPayload(inOutClusterList, &simpleDescr.inOutClusterList, 0, (simpleDescr.inClusterAmount + simpleDescr.outClusterAmount) * sizeof(ZBPRO_APS_ClusterId_t));
            for(int in = 0; in < simpleDescr.inClusterAmount; in++){

                if(clusterSupported(inOutClusterList[in])){

                    Sys_DbgPrint("\nPlease give a name for this new joined device cluster(%s) used for the following normal operation, such as %s\n",
                                   getClusterDescr(inOutClusterList[in]),
                                   getExampleDeviceName(inOutClusterList[in]));

                    uint8_t *deviceName = getInputString();
                    JoinedDeviceCommissioningInfo info = {
                                .extAddr = remoteDeviceExtAddr,
                                .nwkAddr = remoteDeviceNwkAddr,
                                .clusterId = inOutClusterList[in],
                                .endpointSrcId = myEndpoint,
                                .endpointId = simpleDescr.endpoint,
                                .nodeDescr = nodeDescr,
                    };
                    strncpy(info.deviceFriendName, deviceName, sizeof(info.deviceFriendName));
                    insertCommissioningInfo(&info);
                    if(ZIGBEE_HA_STATUS_SUCCESS != (retStatus = Zigbee_HA_Bind(myExtAddr, remoteDeviceExtAddr, inOutClusterList[in], myEndpoint, simpleDescr.endpoint)))
                        Sys_DbgPrint("Failed to call Zigbee_HA_Bind %08x:\n", retStatus);
                }
            }
            SYS_FreePayload(&simpleDescr.inOutClusterList);
        }

        free(activeEpList);

        clearJoinedInformation();
        Sys_DbgPrint("Press key  'e' to exit from this commissining state and go to test procedure\n");
        Sys_DbgPrint("Any any other key continue to enable another device to join\n");
        if(getInputChar() == 'e'){
            saveCommissioningInfo();
            break;
        }
        Zigbee_HA_PermitJoining(0xff);
    }while(1);


case_normal_operation:
    Sys_DbgPrint("Zigbee_HA_StartNetwork\n");
    Zigbee_HA_StartNetwork();

    /*
    * Test part for different device
    */
    do{
        uint8_t ch;
        if(shouldExit())
            break;
        printCommissioningInfo();
        Sys_DbgPrint("Please select which device you want to test, input the index#. press 'l' to list the commissioning table information\n");
        if((ch = getInputChar()) == 'l')
            continue;
        uint8_t deviceIndex = ch - '0';
        switch(getClusterId(deviceIndex)){
            case ZBPRO_ZCL_CLUSTER_ID_ONOFF:
                testLoopForOnOff(deviceIndex);
                break;
            case ZBPRO_ZCL_CLUSTER_ID_LEVEL_CONTROL:
                break;
            case ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK:
                testLoopForDoorLock(deviceIndex);
                break;
            case ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE:
                break;
            case ZBPRO_ZCL_CLUSTER_ID_TEMPERATURE_MEASUREMENT:
                /*
                * Firstly, let's check if this device is a sleeping device, if Yes, talking with it through Async/Callback mechanisum.
                * If it's not, we just use the Sync function call to communicate.
                */
                if(isSleepyDevice(deviceIndex)){
                    /*
                    * For the sleeping device, we have to set some special attributes
                    */
                    Zigbee_HA_Set_Attribute_macTransactionPersistenceTime(30000);
                    /*
                    * There are two ways to get the temperateure from remote sensor continuousely.
                    *
                    * 1. Directly call Zigbee_HA_Read_Temperature_Direct_Async and call Zigbee_HA_Read_Temperature_Direct_Async again
                    *    in the ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t.callback function. But the problem is, even the remote sensor
                    *    is a sleeping device, if application keeps calling like this way, the remote sensor will keep alive without sleeping.
                    *    This will use up battery and violate the sleep device's goal.
                    *
                    *    Zigbee_HA_Read_Temperature_Direct_Async(deviceIndex);
                    *
                    * 2. Create a thread to keep call Zigbee_HA_Read_Temperature_Direct_Async once during a specific period.
                    * Right now, we are using the second solution
                    *
                    *
                    * Do we need to manage the pthread via pthread_join??? To-DO
                    */
                    createNewThread(zigbee_HA_Read_Temperature_thread, (void*)(int)deviceIndex);
                }
                else
                    testLoopForTemperatureSensor(deviceIndex);
                break;
            case ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT:
                if(isSleepyDevice(deviceIndex)){
                    Zigbee_HA_Set_Attribute_macTransactionPersistenceTime(30000);
                    createNewThread(zigbee_HA_Read_Humidity_thread, (void*)(int)deviceIndex);
                }
                else
                    testLoopForHumiditySensor(deviceIndex);
                break;
        }
    }while(1);

    return 0;
_exit:
    printf("zigbee_ha_stb_commissioning_app:  closing...\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}
