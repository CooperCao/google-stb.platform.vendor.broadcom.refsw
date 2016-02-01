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

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;
    ZbProSspKey_t nwkKey;
    uint32_t retStatus;
    ZBPRO_APS_EndpointId_t myEndpoint = 0xB;

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

    ZBPRO_APS_SimpleDescriptor_t localSimpleDescr;
    zbPro_HA_Init_Combinded_SimpleDescriptor(myEndpoint, &localSimpleDescr);
    dumpSimpleDescr(&localSimpleDescr);
    Zigbee_HA_EndpointRegister(&localSimpleDescr);

    Sys_DbgPrint("Zigbee_HA_StartNetwork\n");
    Zigbee_HA_StartNetwork();

    Sys_DbgPrint("Zigbee_HA_Set_Attribute_zdoPermitJoinDuration\n");
    Zigbee_HA_PermitJoining(0xff);
        Sys_DbgPrint("Waiting for remote device to join\n");
        while(!hasDeviceJoined())
        sleep(0);

        Zigbee_HA_PermitJoining(0);

        Sys_DbgPrint("Got a new joined device, wait for about 5 seconds for normal operation\n");
    sleep(5);

    Zigbee_HA_GetNwkAddr(remoteDeviceExtAddr, &remoteDeviceNwkAddr);
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

    if(ZIGBEE_HA_STATUS_SUCCESS != (retStatus = Zigbee_HA_Bind(myExtAddr, remoteDeviceExtAddr, ZBPRO_ZCL_CLUSTER_ID_ONOFF, 1, 1)))
        Sys_DbgPrint("Failed to call Zigbee_HA_Bind %08x:\n", retStatus);
    while(1){
        sleep(5);
        Sys_DbgPrint("On?\n");
        //char key = getchar();
        //if(key == 'Y')
        {
            Sys_DbgPrint("ON................\n");
            Zigbee_HA_OnOffCmdOn_Indirect(1, 1, 1);
        }
        sleep(5);
        Sys_DbgPrint("Off?\n");
        //key = getchar();
        //if(key == 'Y')
        {
           Sys_DbgPrint("OFF................\n");
           Zigbee_HA_OnOffCmdOn_Indirect(0, 1, 1);
        }
    }
    return 0;
_exit:
    printf("MY_HA_ONOFF_CONTROLLER_APP:  closing...\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}
