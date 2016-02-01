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
#include <signal.h>
#include <termios.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
#include "bbMailTestEngine.h"
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
typedef struct JoinedDeviceCommissioningInfo_t{
    ZBPRO_NWK_ExtAddr_t extAddr;
    ZBPRO_NWK_NwkAddr_t nwkAddr;
    ZBPRO_APS_ClusterId_t   clusterId;
    ZBPRO_APS_EndpointId_t  endpointSrcId;
    ZBPRO_APS_EndpointId_t  endpointId;
    char                    *deviceFriendName;
}JoinedDeviceCommissioningInfo;

#define MAX_ALLOWED_DEVICE_NUM  10
#define INVALID_INDEX           -1


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


static bool clusterSupported(ZBPRO_APS_ClusterId_t clusterId)
{
    return IS_ONOFF(clusterId) || IS_LEVEL(clusterId) || IS_LOCK(clusterId) || IS_ZONE(clusterId) || IS_TEMPERATURE_MEASUREMENT(clusterId);
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

typedef enum _Join_Status_t{
    UNJOINED = 0,
    JOINING,
    JOINED,
}Join_Status;

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
*   the extend address is unique in the table
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
    for(int iter; iter < MAX_ALLOWED_DEVICE_NUM; iter++){
        if(commissioningInfoTable[iter].extAddr == extAddr){
            commissioningInfoTable[iter].nwkAddr = nwkAddr;
            return true;
        }
    }
    return false;
}

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
    uint32_t integer;
    scanf("%d" , &integer);
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

static void testLoopForOnOff(uint8_t index)
{
    Sys_DbgPrint("Going to OnOff test loop, press 'Ctrl - C' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("On?\n");
        {
            Sys_DbgPrint("ON................\n");
            Zigbee_HA_OnOffCmdOn_Direct(1, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index));
        }
        sleep(5);
        if(shouldExit())
            break;
        Sys_DbgPrint("Off?\n");
        {
            Sys_DbgPrint("OFF................\n");
            Zigbee_HA_OnOffCmdOn_Direct(0, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index));
        }
    }while(1);
    restoreSignalHandler();
}

static void testLoopForDoorLock(uint8_t index)
{
    Sys_DbgPrint("Going to DoorLock test loop, press 'Ctrl - C' to exit from this loop\n");
    resetExitflag();
    installSignalHandler();
    do{
        sleep(15);
        if(shouldExit())
            break;
        Sys_DbgPrint("Lock?\n");
        {
            Sys_DbgPrint("Locking................\n");
            Zigbee_HA_DoorLockCmdLockUnlock_Direct(1, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index));
        }
        sleep(15);
        if(shouldExit())
            break;
        Sys_DbgPrint("Unlock?\n");
        {
           Sys_DbgPrint("Unlocking................\n");
           Zigbee_HA_DoorLockCmdLockUnlock_Direct(0, getSrcEndpoint(index), getDstEndpoint(index), getDstExtAddr(index));
        }
    }while(1);
    restoreSignalHandler();
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

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;
    ZbProSspKey_t nwkKey;
    uint32_t retStatus;
    ZBPRO_APS_EndpointId_t myEndpoint = 0xB;
    if(!retrieveCommissioningInfo())
        memset(commissioningInfoTable, 0, sizeof(commissioningInfoTable));

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
    Zigbee_Open(&zcb, argv[1]);
    Sys_DbgPrint("Zigbee_HA_StartNetwork\n");
    Zigbee_HA_StartNetwork();

    /*
    * Test part for different device
    */
    do{
        uint8_t ch;
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
                break;
            case ZBPRO_ZCL_CLUSTER_ID_RELATIVE_HUMIDITY_MEASUREMENT:
                break;
        }
    }while(1);

    return 0;
_exit:
    printf("zigbee_ha_normal_operation_app:  closing...\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}
