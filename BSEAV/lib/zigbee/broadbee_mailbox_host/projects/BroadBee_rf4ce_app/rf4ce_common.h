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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_dbg.h"


# pragma GCC optimize "short-enums"     /* Implement short enums. */
typedef enum _TEST_enum_t
{
    TEST_ENUM_1 = 0x00,
} TEST_enum_t;

SYS_DbgAssertStatic(sizeof(TEST_enum_t) == 1)

#define SYS_DBG_LOG_BUFFER_SIZE     256

#  define HAL_DbgLogStr(message)                                TEST_DbgLogStr(message)

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
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args);
    va_end(args);

    HAL_DbgLogStr(message);
}

#endif


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


/*************************************************************************************//**
  Functions belows are to show how to access the BroadBee API's and
  these are used by many different applications
*****************************************************************************************/
void BroadBee_SYS_Get_Fw_Rev();
void BroadBee_SYS_loopback_test(int words_to_test);
void BroadBee_ZRC_Start_NWK();
void BroadBee_ZRC1_TargetBinding();
void BroadBee_ZRC1_TargetBinding_BindingTime(uint32_t bindingTimeSeconds);
void BroadBee_ZRC2_EnableBinding();
void BroadBee_ZRC_PairInd(RF4CE_PairingIndParams_t *indication);
void BroadBee_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *commandInd);
void BroadBee_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *commandInd);
void BroadBee_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
void BroadBee_ZRC_Show_My_Interest(uint8_t pairRef);
void BroadBee_ZRC_Set_SupportedDevices();
void BroadBee_ZRC_Get_WakeUpActionCode();
void BroadBee_ZRC_Set_WakeUpActionCode();
void BroadBee_ZRC_Send_Vendor_Frame(uint8_t *vendorFrame, int length);
void BroadBee_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indParams);
void BroadBee_ZRC_Restore_Factory_Settings(uint8_t restore);
void BroadBee_ZRC_Set_Discovery_Duration(uint32_t durationSeconds);
int  BroadBee_ZRC_Get_Num_Paired_Devices();
void BroadBee_ZRC_Get_Paired_Devices();
void BroadBee_ZRC1_Unpair(uint8_t pairRef);
void BroadBee_ZRC1_Set_KeyRepeatWaitTime(uint32_t waitTime);
void BroadBee_ZRC1_Config_KeyRepeat(uint8_t keyRepeatEnable);
void BroadBee_ZRC_EventNtfy(SYS_EventNotifyParams_t *event);
void BroadBee_ZRC_Subscribe_Event();
void BroadBee_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef);
void BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time(uint8_t pairingRef);
void BroadBee_ZRC2_Set_PushButtonStimulus();

uint16_t BroadBee_RF4CE_Get_PanId();
void BroadBee_RF4CE_Get_NWK_Information();
bool BroadBee_RF4CE_Get_PairingTableEntry(char pairingRef);
uint8_t BroadBee_RF4CE_Get_pairingTableEntriesMax();
void BroadBee_RF4CE_Set_Base_Channel(uint8_t channel);
void BroadBee_RF4CE_Get_Supported_Profiles();
uint8_t BroadBee_RF4CE_Get_numSupportedProfiles();
uint8_t BroadBee_RF4CE_Get_antennasAvailable();
void BroadBee_RF4CE_Set_Discovery_Lqi_Threshold(uint8_t threshold);
int BroadBee_RF4CE_Get_FA_Attributes(uint8_t attrId);
void BroadBee_RF4CE_Set_FA_Attributes(uint8_t attrId, int8_t value);
void BroadBee_RF4CE_Get_FA_Threshold();
void BroadBee_RF4CE_Set_FA_Threshold(int8_t threshold);

uint16_t BroadBee_MAC_Get_ShortAddress();
MAC_ExtendedAddress_t BroadBee_MAC_Get_IEEEAddress();

/*****************************************************************************************/

void BroadBee_SYS_Get_Fw_Rev()
{
    uint8_t status = 0;
    Get_FW_Rev_ConfParams_t rev;

    void BroadBee_SYS_Get_Fw_Rev_Callback(Get_FW_Rev_ReqDescr_t *request, Get_FW_Rev_ConfParams_t *conf)
    {
        memcpy(&rev, conf, sizeof(Get_FW_Rev_ConfParams_t));
        status = 1;
    }

    Get_FW_Rev_ReqDescr_t req = {0};
    req.callback = BroadBee_SYS_Get_Fw_Rev_Callback;

    Get_FW_Rev_Req(&req);
    while(!status);

    printf("Get FW revision successfully\r\n");
    printf("Major:%d  Minor:%d\n", rev.FwRevMajor, rev.FwRevMinor);
}


#define ECHO_PAYLOAD_SIZE 200
void BroadBee_SYS_loopback_test(int words_to_test)
{
    uint8_t callback_status = 0;

    void BroadBee_SYS_loopback_test_callback(TE_EchoCommandReqDescr_t *req, TE_EchoCommandConfParams_t *conf)
    {
        uint32_t verifyBuffer[ECHO_PAYLOAD_SIZE];
        uint32_t iter;
        bool test_passed=true;
        SYS_CopyFromPayload(verifyBuffer, &conf->payload, 0, req->params.echoSize/*ECHO_PAYLOAD_SIZE*4*/);
        SYS_FreePayload(&req->params.payload);
        for(iter = 0; iter < req->params.echoSize/4/*ECHO_PAYLOAD_SIZE*/; iter++)
            if(verifyBuffer[iter] != iter/*0xdf*/){
                printf("loopback data mismatch:  %d:  received=%x, expected=%x\n", iter, verifyBuffer[iter], iter);
                break;
            }
        if(iter == req->params.echoSize/4/*ECHO_PAYLOAD_SIZE*/)
            test_passed = false;

        callback_status = (test_passed ? 1 : 0);
        return;
    }

    uint32_t testPattern[ECHO_PAYLOAD_SIZE];
    //printf("words to test:  %d\n", words_to_test);
    for(int i = 0; i < ECHO_PAYLOAD_SIZE/*sizeof(testPattern)*/; i++)
        testPattern[i] = i; //0xdf;

    TE_EchoCommandReqDescr_t echoRequest;
    memset(&echoRequest, 0, sizeof(TE_EchoCommandReqDescr_t));
    echoRequest.callback = BroadBee_SYS_loopback_test_callback;
    SYS_MemAlloc(&echoRequest.params.payload, words_to_test*4);
    SYS_CopyToPayload(&echoRequest.params.payload, 0, testPattern, words_to_test*4);
    echoRequest.params.echoSize = words_to_test*4; //ECHO_PAYLOAD_SIZE*4;
    callback_status=2;

    Mail_TestEngineEcho(&echoRequest);
    while (callback_status == 2);

    printf("Completed the Mailbox loopback test.\n");
}


void BroadBee_ZRC_Start_NWK()
{
    printf("Starting RF4CE NWK\r\n");
    printf("... be patient to wait for a while for scanning ...\r\n");

    RF4CE_StartReqDescr_t req;
    uint8_t statusStarted = 0;
    void BroadBee_ZRC_Start_NWK_Callback(RF4CE_StartReqDescr_t *request, RF4CE_StartResetConfParams_t *conf)
    {
        //if(RF4CE_START_RESET_OK == conf->status)
        printf("BroadBee_ZRC_Start_NWK_Callback status : %d\n", conf->status);
        statusStarted = 1;
    }
    req.callback = BroadBee_ZRC_Start_NWK_Callback;

    RF4CE_StartReq(&req);
    while(!statusStarted);

    printf("Started RF4CE NWK()\r\n");
}


void BroadBee_ZRC1_TargetBinding()
{
    printf("Starting ZRC1 target Binding\r\n");

    uint8_t statusTargetBinding = 0, statusConf = 0;

    void BroadBee_ZRC1_TargetBinding_Callback(RF4CE_ZRC1_BindReqDescr_t *request, RF4CE_ZRC1_BindConfParams_t *conf)
    {
        statusTargetBinding = 1;
        statusConf = conf->status;
        if(RF4CE_ZRC1_BOUND == statusConf)
            printf("One remote control has been bound successfully.\r\n");
        else
            printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
    }

    RF4CE_ZRC1_BindReqDescr_t req;
    req.callback = BroadBee_ZRC1_TargetBinding_Callback;

    RF4CE_ZRC1_TargetBindReq(&req);
    while(!statusTargetBinding);
}


void BroadBee_ZRC1_TargetBinding_BindingTime(uint32_t bindingTimeSeconds)
{
    printf("Starting ZRC1 target Binding\r\n");
    // Set a customeized pair timeout attribute.
    BroadBee_ZRC_Set_Discovery_Duration(bindingTimeSeconds);

    RF4CE_ZRC1_BindReqDescr_t req;

    uint8_t statusTargetBinding = 0, statusConf = 0;
    void BroadBee_ZRC1_TargetBinding_Callback(RF4CE_ZRC1_BindReqDescr_t *request, RF4CE_ZRC1_BindConfParams_t *conf)
    {
        statusTargetBinding = 1;
        statusConf = conf->status;
        if(RF4CE_ZRC1_BOUND == statusConf)
            printf("One remote control has been bound successfully.\r\n");
        else
            printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
    }
    req.callback = BroadBee_ZRC1_TargetBinding_Callback;

    RF4CE_ZRC1_TargetBindReq(&req);
    while(!statusTargetBinding);
}


void BroadBee_ZRC2_EnableBinding()
{
    printf("Starting ZRC2 Enable Binding\r\n");

    RF4CE_ZRC2_BindingReqDescr_t req;

    uint8_t statusEnableBinding = 0;
    void BroadBee_ZRC2_EnableBinding_Callback(RF4CE_ZRC2_BindingReqDescr_t *request, RF4CE_ZRC2_BindingConfParams_t *conf)
    {
        if(RF4CE_ZRC2GDP2_SUCCESS == conf->status)
            statusEnableBinding = 1;
    }
    req.callback = BroadBee_ZRC2_EnableBinding_Callback;

    RF4CE_ZRC2_EnableBindingReq(&req);
    while(!statusEnableBinding);

    printf("Completed Enable Binding successfully\r\n");
}


void BroadBee_ZRC_PairInd(RF4CE_PairingIndParams_t *indication)
{
    printf("In BroadBee_ZRC_PairInd, indication->pairingRef=0x%x\n", indication->pairingRef);
}


void BroadBee_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *commandInd)
{
    char *controlMsg[] = {"", "Pressed", "Repeated", "Released", ""};
    //printf("Got the control command as following:\r\n");
    char command[200], msg[10] = {0};
    msg[0] = commandInd->commandCode;
    uint8_t rxLinkQuality=0;
    if(SYS_GetPayloadSize(&commandInd->payload)){
        SYS_CopyFromPayload(&rxLinkQuality, &commandInd->payload, 0, sizeof(rxLinkQuality));
    }

    if(rxLinkQuality)
        printf("Key code=%02X is %s  RSSI:%.1f dBm\r\n\r\n",
                    msg[0], controlMsg[commandInd->flags & 0x3], -0.5*(256 - rxLinkQuality));
    else
        printf("Key code=%02X is %s  \r\n\r\n", msg[0], controlMsg[commandInd->flags & 0x3]);

}


void BroadBee_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *commandInd)
{
    printf("Got the control command as following\r\n");
    char command[200], msg[10] = {0};
    RF4CE_ZRC2_Action_t *commands = ALLOCA(SYS_GetPayloadSize(&commandInd->payload));
    SYS_CopyFromPayload(commands, &commandInd->payload, 0, SYS_GetPayloadSize(&commandInd->payload));

    for(int i = 0; i < SYS_GetPayloadSize(&commandInd->payload) / sizeof(RF4CE_ZRC2_Action_t); i++){
        msg[i] = commands[i].code;
        //printf("%c\n", msg[i]);   // For figlet
        printf("  symbol='%c' code=%02X\r\n\r\n", msg[i], msg[i]);
    }
    //sprintf(command, "./figlet -f roman -d ./fonts/ %s", msg);    // For figlet
    //system(command);  // For figlet
}


void BroadBee_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication)
{
    if(fork() == 0)
    {
        RF4CE_ZRC2_CheckValidationRespDescr_t  resp = {0};
        printf("Please press Y/N to accept/not accept this validation code, Or press Enter to continue\n");
        uint8_t keyCode = getchar();
        resp.params.pairingRef = indication->pairingRef;
        switch(keyCode){
            case 'Y':
                resp.params.status = RF4CE_ZRC2_VALIDATION_SUCCESS;
                break;
            case 'N':
                resp.params.status = RF4CE_ZRC2_VALIDATION_FAILURE;
                break;
             default:
                resp.params.status = RF4CE_ZRC2_VALIDATION_PENDING;
                break;
        }
        //indication->callback(indication, &conf);
        RF4CE_ZRC2_CheckValidationResp(&resp);
        exit(0);
    }
}


void BroadBee_ZRC_Show_My_Interest(uint8_t pairRef)
{
    /* To test this function, this kinds of parameters need to match with stack */
    RF4CE_RegisterVirtualDeviceReqDescr_t req;
    memset(&req, 0, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    //req.params.fieldValidMask = RF4CE_ORGVENDORID_MASK | RF4CE_ORGPROFILEID_MASK | RF4CE_MACADDRESS_MASK;
    //req.params.orgVendorId = VENDOR_ID;
    //req.params.orgProfileID = PROFILE_ID;
    req.params.fieldValidMask = RF4CE_PAIRREF_MASK;
    req.params.pairRef = pairRef;

    uint8_t statusRegistered = 0;
    void rf4ce_Test_RF4CE_RegisterVirtualDevice_Callback(RF4CE_RegisterVirtualDeviceReqDescr_t *request, \
                                                         RF4CE_RegisterVirtualDeviceConfParams_t *conf)
    {
        printf("RF4CE_RegisterVirtualDevice_Callback status : %d \n", conf->status);
        statusRegistered = 1;
    }
    req.callback = rf4ce_Test_RF4CE_RegisterVirtualDevice_Callback;

    RF4CE_RegisterVirtualDevice(&req);
    while(!statusRegistered);
}


void BroadBee_ZRC_Set_SupportedDevices()
{
    printf("Setting Supported Devices\r\n");

    uint8_t statusSetSupportedDevices = 0;

    void BroadBee_ZRC_Set_SupportedDevices_Callback(RF4CE_SetSupportedDevicesReqDescr_t *req, RF4CE_SetSupportedDevicesConfParams_t *conf)
    {
        statusSetSupportedDevices = 1;
    }

    RF4CE_SetSupportedDevicesReqDescr_t req = {0};
    req.params.numDevices = 3;
    req.params.devices[0] = RF4CE_TELEVISION;
    req.params.devices[1] = RF4CE_SET_TOP_BOX;
    req.params.devices[3] = RF4CE_GENERIC;
    req.callback = BroadBee_ZRC_Set_SupportedDevices_Callback;

    RF4CE_SetSupportedDevicesReq(&req);
    while(!statusSetSupportedDevices);

    printf("Set Supported Devices successfully\r\n");
}


void BroadBee_ZRC_Get_WakeUpActionCode()
{
    RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t req = {0};

    uint8_t statusGetPowerFilterKey = 0;
    void rf4ce_Test_Get_PowerFilterKey_Callback(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *request, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t *conf)
    {
        if(0 == conf->status){
            statusGetPowerFilterKey = 1;
            for(uint8_t i = 0; i < sizeof(conf->wakeUpActionCodeFilter); i++)
                printf(" %02x ", conf->wakeUpActionCodeFilter[i]);
            printf("\n");
        }
    }
    req.callback = rf4ce_Test_Get_PowerFilterKey_Callback;

    RF4CE_ZRC_GetWakeUpActionCodeReq(&req);
    while(!statusGetPowerFilterKey);
    printf("Got the power filter key successfully.\r\n");
}


void BroadBee_ZRC_Set_WakeUpActionCode()
{
    RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t req = {0};

    uint8_t statusSetPowerFilterKey = 0;
    void rf4ce_Test_Set_PowerFilterKey_Callback(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *request, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetPowerFilterKey = 1;
    }
    req.callback = rf4ce_Test_Set_PowerFilterKey_Callback;

//                                               00  10  21  32  33  44  54  65  76
//                                               70  58  36  14  92  70  58  36  14
#if 0
    memcpy(req.params.wakeUpActionCodeFilter, "\x00\x00\x00\x00\x00\x00\x00\x00\x02", 9); // bit 65 (volume up)
#elif 0
    memcpy(req.params.wakeUpActionCodeFilter, "\x00\x00\x00\x00\x00\x00\x01\x00\x00", 9); // bit 48 (channel up)
#else
    memcpy(req.params.wakeUpActionCodeFilter, "\x00\x20\x00\x00\x00\x00\x00\x00\x00", 9); // bit 13 (exit)
#endif

    RF4CE_ZRC_SetWakeUpActionCodeReq(&req);
    while(!statusSetPowerFilterKey);
    printf("Set the power filter key successfully\r\n");
}


void BroadBee_ZRC_Send_Vendor_Frame(uint8_t *vendorFrame, int length)
{
    uint8_t statusSendVendorFrame = 0;

    void BroadBee_ZRC_Send_Vendor_Frame_Callback(RF4CE_ZRC1_VendorSpecificReqDescr_t *request, RF4CE_ZRC1_VendorSpecificConfParams_t *conf)
    {
        statusSendVendorFrame = 1;
    }

    RF4CE_ZRC1_VendorSpecificReqDescr_t req = {0};
    req.params.pairingRef = 0;
    req.params.profileID = 1;
    req.params.vendorID = 0x1234;
    req.params.nsduLength = length;
    req.params.txFlags = RF4CE_NWK_DATA_TX_SECURITY | RF4CE_NWK_DATA_TX_VENDOR_SPECIFIC;
    SYS_MemAlloc(&req.params.payload, length);
    SYS_CopyToPayload(&req.params.payload, 0, vendorFrame, length);
    req.callback = BroadBee_ZRC_Send_Vendor_Frame_Callback;

    RF4CE_ZRC1_VendorSpecificReq(&req);
    while(!statusSendVendorFrame);

    printf("Sent a vendor frame successfully\n");
}


void BroadBee_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indParams)
{
    int length = SYS_GetPayloadSize(&indParams->payload);
    uint8_t *data = calloc(sizeof(char), length);
    SYS_CopyFromPayload(data, &indParams->payload, 0, length);

    printf("PairRef:    %d\n", indParams->pairingRef);
    printf("ProfileId:  0x%02X\n", indParams->profileID);
    printf("VendorId:   0x%02X\n", indParams->vendorID);
    printf("nsduLength: %d\n", indParams->nsduLength, length);
    for(int i = 0; i < length; i++)
        printf(" %02X ", data[i]);
    printf("\n");

    return;
}


void BroadBee_ZRC_Restore_Factory_Settings(uint8_t restore)
{
    uint8_t statusRestoreFactorySettings = 0;

    void BroadBee_ZRC_Restore_Factory_Settings_Callback(RF4CE_ResetReqDescr_t *request, RF4CE_StartResetConfParams_t *conf)
    {
        if(RF4CE_START_RESET_OK == conf->status)
            statusRestoreFactorySettings = 1;
    }

    RF4CE_ResetReqDescr_t req = {0};
    req.params.setDefaultPIBNIB = restore;
    req.callback = BroadBee_ZRC_Restore_Factory_Settings_Callback;

    RF4CE_ResetReq(&req);
    while(!statusRestoreFactorySettings);

    printf("Restore factory settings successfully\r\n");
}


void BroadBee_ZRC_Set_Discovery_Duration(uint32_t durationSeconds)
{
    uint8_t status_Set_Discovery_Duration = 0;

    void BroadBee_ZRC_Set_Discovery_Duration_Callback(RF4CE_ZRC1_SetAttributeDescr_t *request, RF4CE_ZRC1_SetAttributeConfParams_t *conf)
    {
        if(0 == conf->status){
            status_Set_Discovery_Duration = 1;
        }
    }

    RF4CE_ZRC1_SetAttributeDescr_t req = {0};
    req.params.attributeId = GDP_AUTO_DISC_DURATION;
    req.params.data.aplGDPAutoDiscoveryDuration = durationSeconds * 62500;
    req.callback = BroadBee_ZRC_Set_Discovery_Duration_Callback;

    RF4CE_ZRC1_SetAttributesReq(&req);
    while(!status_Set_Discovery_Duration);
}


int BroadBee_ZRC_Get_Num_Paired_Devices()
{
    int numPairedDevice = 0;
    int numEntries = BroadBee_RF4CE_Get_pairingTableEntriesMax();
    printf("BroadBee_RF4CE_Get_pairingTableEntriesMax : %d\n", numEntries);

    for(int i = 0; i < numEntries; i++){
        if(BroadBee_RF4CE_Get_PairingTableEntry(i))
            numPairedDevice++;
    }

    return numPairedDevice;
}


void BroadBee_ZRC_Get_Paired_Devices()
{
    for(int i = 0; i < BroadBee_RF4CE_Get_pairingTableEntriesMax(); i++){
        BroadBee_RF4CE_Get_PairingTableEntry(i);
    }
}


void BroadBee_ZRC1_Unpair(uint8_t pairRef)
{
    printf("Starting ZRC1 target Unpair\r\n");
    uint8_t statusUnpair = 0, unpairConfStatus = 0;

    void BroadBee_ZRC1_Unpair_Callback(RF4CE_UnpairReqDescr_t *req, RF4CE_UnpairConfParams_t *conf)
    {
        statusUnpair = 1;
        unpairConfStatus = conf->status;
    }

    RF4CE_UnpairReqDescr_t req;
    req.params.pairingRef = pairRef;
    req.callback = BroadBee_ZRC1_Unpair_Callback;

    RF4CE_UnpairReq(&req);
    while(!statusUnpair);

    if(1 == unpairConfStatus)
        printf("Unpaired the pairRef %d successfully.\r\n", pairRef);
    else
        printf("Failed to unpair the pairRef %d with the status=%d.\r\n", pairRef, unpairConfStatus);
}


void BroadBee_ZRC1_Set_KeyRepeatWaitTime(uint32_t waitTime)
{
    uint8_t status_Set_KeyRepeatWaitTime = 0;

    void BroadBee_ZRC1_Set_KeyRepeatWaitTime_Callback(RF4CE_ZRC1_SetAttributeDescr_t *request, RF4CE_ZRC1_SetAttributeConfParams_t *conf)
    {
        status_Set_KeyRepeatWaitTime = 1;
    }

    RF4CE_ZRC1_SetAttributeDescr_t req = {0};
    req.params.attributeId = RF4CE_ZRC1_APL_KEY_REPEAT_WAIT_TIME;
    req.params.data.aplZRC1KeyRepeatWaitTime = waitTime;
    req.callback = BroadBee_ZRC1_Set_KeyRepeatWaitTime_Callback;

    RF4CE_ZRC1_SetAttributesReq(&req);
    while(!status_Set_KeyRepeatWaitTime);
}


void BroadBee_ZRC1_Config_KeyRepeat(uint8_t keyRepeatEnable)
{
    if(keyRepeatEnable)
        BroadBee_ZRC1_Set_KeyRepeatWaitTime(RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL << 1);
    else
        BroadBee_ZRC1_Set_KeyRepeatWaitTime(0xFFFFFFFF);
}


void BroadBee_ZRC_EventNtfy(SYS_EventNotifyParams_t *event)
{
    //Sys_DbgPrint("\nSYS_EventNtfy : EventId(%d)\n", event->id);
    switch(event->id){
        case RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED:
            printf("RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED\n");
            break;
        case RF4CE_CTRL_EVENT_PAIRING_DEVICE_NOT_FOUND:
            printf("RF4CE_CTRL_EVENT_PAIRING_DEVICE_NOT_FOUND\n");
            break;
        case RF4CE_CTRL_EVENT_PAIRING_MULTIPLE_DEVICES_FOUND:
            printf("RF4CE_CTRL_EVENT_PAIRING_MULTIPLE_DEVICES_FOUND\n");
            break;
        case RF4CE_CTRL_EVENT_PAIRING_DUPLICATE_PAIR_FOUND:
            printf("RF4CE_CTRL_EVENT_PAIRING_DUPLICATE_PAIR_FOUND\n");
            break;
        case RF4CE_CTRL_EVENT_UNPAIRING_DEVICE_UNPAIRED:
            printf("RF4CE_CTRL_EVENT_UNPAIRING_DEVICE_UNPAIRED\n");
            break;
        case RF4CE_CTRL_EVENT_VENDOR_FRAME_RECEIVED:
            printf("RF4CE_CTRL_EVENT_VENDOR_FRAME_RECEIVED\n");
            break;
        case RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_OK:
            printf("RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_OK\n");
            break;
        case RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_FAILED:
            printf("RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_FAILED\n");
            break;
        case RF4CE_CTRL_EVENT_BAD_VENDOR_FRAME:
            printf("RF4CE_CTRL_EVENT_BAD_VENDOR_FRAME\n");
            break;
        case RF4CE_CTRL_EVENT_PAIRING_GENERAL_FAILURE:
            printf("RF4CE_CTRL_EVENT_PAIRING_GENERAL_FAILURE\n");
            break;
        case RF4CE_CTRL_EVENT_TEST_TRANSMIT_END:
            printf("RF4CE_CTRL_EVENT_TEST_TRANSMIT_END\n");
            break;
        default:
            printf("Unknown event\n");
            break;
    }
}


void BroadBee_ZRC_Subscribe_Event()
{
    SYS_EventHandlerMailParams_t eventMap = {0};
    printf("size of SYS_EventHandlerParams_t is %d %d\n", sizeof(SYS_EventHandlerMailParams_t), sizeof(eventMap.subscribedEventsMap));
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED);

    sysEventSubscribeHostHandler(&eventMap);
}


void BroadBee_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef)
{
    printf("Starting ZRC2 Set Default Check Validation Period\r\n");
    uint8_t status = 0;

    void BroadBee_ZRC2_Set_Default_Check_Validation_Period_Callback(RF4CE_ZRC2_SetAttributesReqDescr_t *request, RF4CE_ZRC2_SetAttributesConfParams_t *conf)
    {
        printf("Set Default Check Validation Period Callback\r\n");
        if(0 == conf->status)
            status = 1;
    }

    RF4CE_ZRC2_SetAttributeId_t *setAttribute;
    RF4CE_ZRC2_SetAttributesReqDescr_t req = {0};    /* CHECK_VALIDATION_PERIOD */
    int size = sizeof(RF4CE_ZRC2_SetAttributeId_t) - 1 + RF4CE_ZRC2_AUTO_CHECK_VALIDATION_PERIOD_SIZE;
    SYS_MemAlloc(&req.params.payload, size);

    uint8_t * const buffSetAttribute = malloc(size);
    setAttribute = (RF4CE_ZRC2_SetAttributeId_t*)buffSetAttribute;
    setAttribute->id.attributeId = 0x87;
    setAttribute->id.index.index = 0;
    setAttribute->size = RF4CE_ZRC2_AUTO_CHECK_VALIDATION_PERIOD_SIZE;
    *(uint16_t*)setAttribute->value = 500;

    SYS_CopyToPayload(&req.params.payload, 0, buffSetAttribute, size);
    free(buffSetAttribute);
    req.params.pairingRef = pairingRef;
    req.callback = BroadBee_ZRC2_Set_Default_Check_Validation_Period_Callback;

    RF4CE_ZRC2_SetAttributesReq(&req);
    while(!status);
    printf("ZRC2 Set Default Check Validation Period successfully\r\n");
}


void BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time(uint8_t pairingRef)
{
    printf("Starting ZRC2 Set Default Link Lost Wait Time\r\n");

    uint8_t status = 0;

    void BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time_Callback(RF4CE_ZRC2_SetAttributesReqDescr_t *request, RF4CE_ZRC2_SetAttributesConfParams_t *conf)
    {
        printf("Set Default Default Link Lost Wait Time Callback\r\n");
        if(0 == conf->status)
            status = 1;
    }

    RF4CE_ZRC2_SetAttributeId_t *setAttribute;
    RF4CE_ZRC2_SetAttributesReqDescr_t req = {0};
    /* LINK_LOST_WAIT_TIME */
    int size = sizeof(RF4CE_ZRC2_SetAttributeId_t) - 1 + RF4CE_ZRC2_LINK_LOST_WAIT_TIME_SIZE;
    SYS_MemAlloc(&req.params.payload, size);

    uint8_t * const buffSetAttribute = malloc(size);
    setAttribute = (RF4CE_ZRC2_SetAttributeId_t*)buffSetAttribute;
    setAttribute->id.attributeId = 0x8A;
    setAttribute->id.index.index = 0;
    setAttribute->size = RF4CE_ZRC2_LINK_LOST_WAIT_TIME_SIZE;
    *(uint16_t*)setAttribute->value = 5000;

    SYS_CopyToPayload(&req.params.payload, 0, buffSetAttribute, size);
    free(buffSetAttribute);
    req.params.pairingRef = pairingRef;
    req.callback = BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time_Callback;

    RF4CE_ZRC2_SetAttributesReq(&req);
    while(!status);
    printf("ZRC2 Set Default Default Link Lost Wait Time successfully\r\n");
}


void BroadBee_ZRC2_Set_PushButtonStimulus()
{
    printf("Starting button stimulus\r\n");

    uint8_t statusButtonStimulus = 0;

    void BroadBee_ZRC2_Set_PushButtonStimulus_Callback(RF4CE_ZRC2_ButtonBindingReqDescr_t *request, RF4CE_ZRC2_BindingConfParams_t *conf)
    {
        printf("conf->status : %d\n", conf->status);
        if(0 == conf->status)
            statusButtonStimulus = 1;
    }

    RF4CE_ZRC2_ButtonBindingReqDescr_t req = {0};
    req.callback = BroadBee_ZRC2_Set_PushButtonStimulus_Callback;

    RF4CE_ZRC2_SetPushButtonStimulusReq(&req);
    while(!statusButtonStimulus);
    printf("Button stimulus trigger successfully\r\n");
}


uint16_t BroadBee_RF4CE_Get_PanId()
{
    uint16_t panId = 0;
    uint8_t status_Get_PanId = 0;

    void BroadBee_RF4CE_Get_PanId_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            panId = conf->data.nwkPanId;
            status_Get_PanId = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_PAN_ID;
    req.callback = BroadBee_RF4CE_Get_PanId_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_PanId);

    return panId;
}


void BroadBee_RF4CE_Get_NWK_Information()
{
    typedef struct
    {
        MAC_ExtendedAddress_t address;
        uint16_t shortAddress;
        uint16_t panID;
    } RF4CECtrlNetworkInfo;

    RF4CECtrlNetworkInfo nwkInfo = {0};

    nwkInfo.address = BroadBee_MAC_Get_IEEEAddress();
    nwkInfo.shortAddress = BroadBee_MAC_Get_ShortAddress();
    nwkInfo.panID = BroadBee_RF4CE_Get_PanId();
    printf("IEEE_Address = 0x%16X\n", nwkInfo.address);
    printf("Short_Address = 0x%04X\n", nwkInfo.shortAddress);
    printf("PAN_ID = 0x%04X\n", nwkInfo.panID);
    printf("Completed BroadBee_RF4CE_Get_NWK_Information successfully.\r\n");
}


bool BroadBee_RF4CE_Get_PairingTableEntry(char pairingRef)
{
    bool validPairingEntry = FALSE;
    uint8_t statusGetParingEntry = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};
    void BroadBee_RF4CE_Get_PairingTableEntry_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            RF4CE_PairingTableEntry_t *entry = &conf->data.nwkPairingTableEntry;
            validPairingEntry = TRUE;
            printf("dstIeeeAddr = %016llX\n", entry->dstIeeeAddr);
            printf("dstPanId = %04X\n", entry->dstPanId);
            printf("dstNetAddr = %04X\n", entry->dstNetAddr);
        }
        statusGetParingEntry = 1;
    }

    req.params.attrId.attrId = RF4CE_NWK_PAIRING_TABLE;
    req.params.attrId.attrIndex = pairingRef;
    req.callback = BroadBee_RF4CE_Get_PairingTableEntry_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetParingEntry);
    printf("Completed BroadBee_RF4CE_Get_pairingTableEntry successfully\r\n");
    return validPairingEntry;
}


uint8_t BroadBee_RF4CE_Get_pairingTableEntriesMax()
{
    uint8_t pairingTableEntriesMax = 0;
    uint8_t status_Get_pairingTableEntriesMax = 0;

    void BroadBee_RF4CE_Get_pairingTableEntriesMax_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            pairingTableEntriesMax = conf->data.nwkMaxPairingTableEntries;
            status_Get_pairingTableEntriesMax = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_MAX_PAIRING_TABLE_ENTRIES;
    req.callback = BroadBee_RF4CE_Get_pairingTableEntriesMax_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_pairingTableEntriesMax);
    printf("Completed BroadBee_RF4CE_Get_pairingTableEntriesMax successfully\r\n");
    return pairingTableEntriesMax;
}


void BroadBee_RF4CE_Set_Base_Channel(uint8_t channel)
{
    uint8_t statusSetSingleChannel = 0;

    void BroadBee_RF4CE_Set_Base_Channel_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetSingleChannel = 1;
    }

    RF4CE_NWK_SetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_BASE_CHANNEL;
    req.params.data.nwkBaseChannel = channel;
    req.callback = BroadBee_RF4CE_Set_Base_Channel_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!statusSetSingleChannel);

    printf("Set the Base Channel successfully.\r\n");
}


void BroadBee_RF4CE_Get_Supported_Profiles()
{

    uint8_t antennaAvailable = 0;
    uint8_t status_Get_Supported_Profiles = 0;

    uint8_t nwkSupportedProfiles[RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH];

    void rf4ce_Get_Supported_Profiles_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            memcpy(nwkSupportedProfiles, conf->data.nwkSupportedProfiles, sizeof(nwkSupportedProfiles));;
            status_Get_Supported_Profiles = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_NUM_SUPPORTED_PROFILES;
    req.callback = rf4ce_Get_Supported_Profiles_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_Supported_Profiles);

    // directly use RF4CE_ProfilesIdList
    printf("The supported profiles are ");
    for(int i = 0; i < BroadBee_RF4CE_Get_numSupportedProfiles(); i++)
        printf("%02X ", nwkSupportedProfiles[i]);
    printf("\r\n");

    printf("Completed BroadBee_RF4CE_Get_Supported_Profiles successfully.\r\n");
}


uint8_t BroadBee_RF4CE_Get_numSupportedProfiles()
{
    uint8_t numSupportedProfiles = 0;
    uint8_t status_Get_numSupportedProfiles = 0;

    void BroadBee_RF4CE_Get_numSupportedProfiles_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            numSupportedProfiles = conf->data.nwkNumSupportedProfiles;
            status_Get_numSupportedProfiles = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_NUM_SUPPORTED_PROFILES;
    req.callback = BroadBee_RF4CE_Get_numSupportedProfiles_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_numSupportedProfiles);

    return numSupportedProfiles;
}


uint8_t BroadBee_RF4CE_Get_antennasAvailable()
{
    uint8_t antennaAvailable = 0;
    uint8_t status_Get_antennasAvailable = 0;

    void BroadBee_RF4CE_Get_antennasAvailable_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            antennaAvailable = conf->data.antennaAvailable;
            status_Get_antennasAvailable = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_ANTENNA_AVAILABLE;
    req.callback = BroadBee_RF4CE_Get_antennasAvailable_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_antennasAvailable);

    return antennaAvailable;
}


void BroadBee_RF4CE_Set_Discovery_Lqi_Threshold(uint8_t threshold)
{
    uint8_t statusSetThreshold = 0;

    void BroadBee_RF4CE_Set_Discovery_Lqi_Threshold_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetThreshold = 1;
    }

    RF4CE_NWK_SetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_DISCOVERY_LQI_THRESHOLD;
    req.params.data.nwkDiscoveryLQIThreshold = threshold;
    req.callback = BroadBee_RF4CE_Set_Discovery_Lqi_Threshold_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!statusSetThreshold);
    printf("Set Discovery Lqi Threshold successfully\r\n");
}


int BroadBee_RF4CE_Get_FA_Attributes(uint8_t attrId)
{
    uint8_t statusGetFaAttr = 0;
    int value;

    void BroadBee_RF4CE_Get_FA_Attributes_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status)
        {
            switch (attrId)
            {
                case RF4CE_NWK_FA_SCAN_THRESHOLD:
                     value= conf->data.nwkFaScanThreshold;
                    break;
                 case RF4CE_NWK_FA_COUNT_THRESHOLD:
                     value=  conf->data.nwkFaCountThreshold;
                    break;
                 case RF4CE_NWK_FA_DECREMENT:
                    value=  conf->data.nwkFaDecrement;
                    break;
            }
            statusGetFaAttr = 1;
        }
        else
            printf("Failed to get the FA Attributes\r\n");
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = attrId;
    req.callback = BroadBee_RF4CE_Get_FA_Attributes_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetFaAttr);

    return value;
}


void BroadBee_RF4CE_Set_FA_Attributes(uint8_t attrId, int8_t value)
{
    uint8_t statusSetFaAttr = 0;

    void BroadBee_RF4CE_Set_FA_Attributes_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetFaAttr = 1;
        else
            printf("Set FA Attributes failed\r\n");
    }

    RF4CE_NWK_SetReqDescr_t req = {0};
    req.params.attrId.attrId = attrId;
    req.callback = BroadBee_RF4CE_Set_FA_Attributes_Callback;
    switch (attrId)
    {
        case RF4CE_NWK_FA_SCAN_THRESHOLD:
            req.params.data.nwkFaScanThreshold = value;
            break;
         case RF4CE_NWK_FA_COUNT_THRESHOLD:
            req.params.data.nwkFaCountThreshold = value;
            break;
         case RF4CE_NWK_FA_DECREMENT:
            req.params.data.nwkFaDecrement = value;
            break;
         default:
            printf("Default\r\n");
    }

    RF4CE_NWK_SetReq(&req);
    while(!statusSetFaAttr);
}


void BroadBee_RF4CE_Get_FA_Threshold()
{
    uint8_t statusGetFaThreshold = 0;

    void BroadBee_RF4CE_Get_FA_Threshold_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            printf("rf4ce fa scan threshold: %d\n", conf->data.nwkFaScanThreshold);
            statusGetFaThreshold = 1;
        }
    }

    RF4CE_NWK_GetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_FA_SCAN_THRESHOLD;
    req.callback = BroadBee_RF4CE_Get_FA_Threshold_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetFaThreshold);
    printf("Got the FA Scan Threshold successfully\r\n");
}


void BroadBee_RF4CE_Set_FA_Threshold(int8_t threshold)
{
    uint8_t statusSetFaThreshold = 0;

    void BroadBee_RF4CE_Set_FA_Threshold_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetFaThreshold = 1;
    }

    RF4CE_NWK_SetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_FA_SCAN_THRESHOLD;
    req.params.data.nwkFaScanThreshold = threshold;
    req.callback = BroadBee_RF4CE_Set_FA_Threshold_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!statusSetFaThreshold);
    printf("Set FA Scan Threshold successfully\r\n");
}


uint16_t BroadBee_MAC_Get_ShortAddress()
{
    uint16_t shortAddress = 0;
    uint8_t status_Get_Mac_ShortAddress = 0;

    void BroadBee_MAC_Get_ShortAddress_Callback(MAC_GetReqDescr_t *request, MAC_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            shortAddress = conf->attributeValue.macShortAddress;
            status_Get_Mac_ShortAddress = 1;
        }
    }

    MAC_GetReqDescr_t req = {0};
    req.params.attribute = MAC_SHORT_ADDRESS;
    req.callback = BroadBee_MAC_Get_ShortAddress_Callback;

    RF4CE_MAC_GetReq(&req);
    while(!status_Get_Mac_ShortAddress);

    return shortAddress;
}


MAC_ExtendedAddress_t BroadBee_MAC_Get_IEEEAddress()
{
    MAC_ExtendedAddress_t ieeeAddress = 0;
    uint8_t status_Get_Mac_IEEEAddress = 0;

    MAC_GetReqDescr_t req = {0};

    void BroadBee_MAC_Get_IEEEAddress_Callback(MAC_GetReqDescr_t *request, MAC_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            ieeeAddress = conf->attributeValue.macExtendedAddress;
            status_Get_Mac_IEEEAddress = 1;
        }
    }

    req.params.attribute = MAC_EXTENDED_ADDRESS;
    req.callback = BroadBee_MAC_Get_IEEEAddress_Callback;

    RF4CE_MAC_GetReq(&req);
    while(!status_Get_Mac_IEEEAddress);

    return ieeeAddress;
}

/*****************************************************************************************/

#define RF4CE_CTRL_FEATURE_DIAGNOSTICS  (0x00000001)
#define RF4CE_CTRL_FEATURE_PAIRING_REF  (0x00000100)
#define RF4CE_CTRL_FEATURE_TX_POWER     (0x00000200)

typedef struct
{
    uint32_t features;
    uint32_t numSupportedProfiles;
    uint32_t pairingTableEntriesMax;
    uint32_t antennasAvailable;
} RF4CECtrlCapsV1;

static void BroadBee_ZRC_Get_Caps_Ex()
{
    RF4CECtrlCapsV1 capEx = { 0 };
    capEx.features = RF4CE_CTRL_FEATURE_DIAGNOSTICS | RF4CE_CTRL_FEATURE_PAIRING_REF | RF4CE_CTRL_FEATURE_TX_POWER;
    capEx.numSupportedProfiles = BroadBee_RF4CE_Get_numSupportedProfiles();
    capEx.pairingTableEntriesMax = BroadBee_RF4CE_Get_pairingTableEntriesMax();
    capEx.antennasAvailable = BroadBee_RF4CE_Get_antennasAvailable();

    printf("Features = 0x%08X\n", capEx.features);
    printf("numSupportedProfiles = %d\n", capEx.numSupportedProfiles);
    printf("pairingTableEntriesMax = %d\n", capEx.pairingTableEntriesMax);
    printf("antennasAvailable = 0x%08X\n", capEx.antennasAvailable);

    printf("Completed BroadBee_ZRC_Get_Caps_Ex()\r\n");
}


#define RF4CE_ZRC_INPUT_CAP_V0 (0x00000001)
#define USER_INPUT_MODE_RAW (1)
#define USER_INPUT_MODE_COOKED (2)
#define RF4CE_PROFILE_ZRC (1)

typedef struct
{
    uint32_t version;
    void     *capability;
    uint32_t capabilitySize;
} RF4CEZRCInputCapsEx;

typedef struct
{
    uint32_t mode;
    uint16_t profileId;
}RF4CEZRCInputCapsV0;

static uint8_t BroadBee_ZRC_Get_Extended_Cap(RF4CEZRCInputCapsEx *cap)
{
    if(cap->version != RF4CE_ZRC_INPUT_CAP_V0)
        return FALSE;
    if(cap->capabilitySize != sizeof(RF4CEZRCInputCapsV0))
        return FALSE;

    RF4CEZRCInputCapsV0 *capV0 = malloc(sizeof(RF4CEZRCInputCapsV0));
    memset(capV0, 0, sizeof(RF4CEZRCInputCapsV0));
    capV0->mode = USER_INPUT_MODE_RAW; // our stack just supports RAW mode
    capV0->profileId = RF4CE_PROFILE_ZRC;
    cap->capability = capV0;

    return TRUE;
}


static void BroadBee_ZRC_Get_Diag_Caps()
{
    uint8_t status_BroadBee_ZRC_Get_Diag_Caps = 0;

    void BroadBee_ZRC_Get_Diag_Caps_Callback(RF4CE_Diag_Caps_ReqDescr_t *request, RF4CE_Diag_Caps_ConfParams_t *conf)
    {

        status_BroadBee_ZRC_Get_Diag_Caps = 1;
    }

    RF4CE_Diag_Caps_ReqDescr_t req = {0};
    req.callback = BroadBee_ZRC_Get_Diag_Caps_Callback;

    RF4CE_Get_Diag_Caps_Req(&req);
    while(!status_BroadBee_ZRC_Get_Diag_Caps);

    printf("Completed BroadBee_ZRC_Get_Diag_Caps()\r\n");
}


static void BroadBee_ZRC_Get_Diag_Agility()
{
    uint8_t status_BroadBee_ZRC_Get_Diag_Agility = 0;

    void BroadBee_ZRC_Get_Diag_Agility_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("BroadBee_ZRC_Get_Diag_Agility_Callback got called\r\n");
        printf("agilityThreshold=%d\n", conf->u.agility.agilityThreshold);
        printf("currentChannel = %d\n", conf->u.agility.operational.logicalChannel);
        printf("bkChan1        = %d\n", conf->u.agility.bkChan1.logicalChannel);
        printf("bkChan2        = %d\n", conf->u.agility.bkChan2.logicalChannel);
        status_BroadBee_ZRC_Get_Diag_Agility = 1;
    }

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_AGILITY;
    req.callback = BroadBee_ZRC_Get_Diag_Agility_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_BroadBee_ZRC_Get_Diag_Agility);

    printf("Completed BroadBee_ZRC_Get_Diag_Agility()\r\n");
}


static void BroadBee_ZRC_Get_Diag_LinkQuality(uint8_t pairRef)
{
    uint8_t status_BroadBee_ZRC_Get_Diag_LinkQuality = 0;

    void BroadBee_ZRC_Get_Diag_LinkQuality_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("Link Quality of last received packet for pairRef(%d) = %d\n", pairRef, conf->u.linkQuality.linkQuality);
        status_BroadBee_ZRC_Get_Diag_LinkQuality = 1;
    }

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_LINK_QUALITY;
    req.params.u.linkQuality.pairingRef = pairRef;
    req.callback = BroadBee_ZRC_Get_Diag_LinkQuality_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_BroadBee_ZRC_Get_Diag_LinkQuality);

    printf("Completed BroadBee_ZRC_Get_Diag_LinkQuality(%d)\r\n", pairRef);
}


static void BroadBee_ZRC_Get_TxPower_KeyExchange()
{
    uint8_t BroadBee_ZRC_Get_TxPower_KeyExchange = 0;

    void BroadBee_ZRC_Get_TxPower_KeyExchange_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("KeyExchangeTxPowerMax = %d\n", conf->u.txPowerKeyExchange.powerMax);
        printf("KeyExchangeTxPowerMin = %d\n", conf->u.txPowerKeyExchange.powerMin);
        printf("KeyExchangeTxPower    = %d\n", conf->u.txPowerKeyExchange.power);
        BroadBee_ZRC_Get_TxPower_KeyExchange = 1;
    }

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER_KEY_EXCHANGE;
    req.callback = BroadBee_ZRC_Get_TxPower_KeyExchange_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!BroadBee_ZRC_Get_TxPower_KeyExchange);

    printf("Completed BroadBee_ZRC_Get_TxPower_KeyExchange()\r\n");
}


static void BroadBee_ZRC_Set_TxPower_KeyExchange(int8_t power)
{
    uint8_t status_Set_TX_Power_Key_Exchange = 0;

    void BroadBee_ZRC_Set_TxPower_KeyExchange_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        status_Set_TX_Power_Key_Exchange = 1;
    }

    RF4CE_NWK_SetReqDescr_t req = {0};
    req.params.attrId.attrId = RF4CE_NWK_TX_POWER_KEY_EXCHANGE;
    req.params.data.nwkTxPowerKeyExchange = power;
    req.callback = BroadBee_ZRC_Set_TxPower_KeyExchange_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!status_Set_TX_Power_Key_Exchange);

    printf("Completed BroadBee_ZRC_Set_TxPower_KeyExchange(%d)\r\n", power);
}


void BroadBee_PHY_Get_Caps()
{
    uint8_t statusPhy_Test_Get_Caps = 0;

    Phy_Test_Get_Caps_ReqDescr_t req = {0};
    void BroadBee_PHY_Get_Caps_Callback(Phy_Test_Get_Caps_ReqDescr_t *request, Phy_Test_Get_Caps_ConfParams_t *conf)
    {
        printf("Channel Min = %d\n",  conf->channelMin);
        printf("Channel Max = %d\n",  conf->channelMax);
        printf("Power Min = %d\n",  conf->powerMin);
        printf("Power Max = %d\n",  conf->powerMax);

        statusPhy_Test_Get_Caps = 1;
    }
    req.callback = BroadBee_PHY_Get_Caps_Callback;

    Phy_Test_Get_Caps_Req(&req);
    while(!statusPhy_Test_Get_Caps);

    printf("Completed BroadBee_PHY_Get_Caps()\r\n");
}


void BroadBee_PHY_Set_Channel(uint8_t channel)
{
    uint8_t statusBroadBee_PHY_Set_Channel = 0;

    Phy_Test_Set_Channel_ReqDescr_t req = {0};
    void BroadBee_PHY_Set_Channel_Callback(Phy_Test_Set_Channel_ReqDescr_t *request, Phy_Test_Set_Channel_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Set_Channel = 1;
    }
    req.callback = BroadBee_PHY_Set_Channel_Callback;
    req.params.channel = channel;

    Phy_Test_Set_Channel_Req(&req);
    while(!statusBroadBee_PHY_Set_Channel);

    printf("Completed BroadBee_PHY_Set_Channel(%d)\r\n", channel);
}


void BroadBee_PHY_Continuous_Wave_Start(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE modulationMode)
{
    uint8_t statusBroadBee_PHY_Continuous_Wave_Start = 0;

    Phy_Test_Continuous_Wave_Start_ReqDescr_t req = {0};
    void BroadBee_PHY_Continuous_Wave_Start_Callback(Phy_Test_Continuous_Wave_Start_ReqDescr_t *request, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Continuous_Wave_Start = 1;
    }
    req.callback = BroadBee_PHY_Continuous_Wave_Start_Callback;
    req.params.mode = modulationMode;

    Phy_Test_Continuous_Wave_Start_Req(&req);
    while(!statusBroadBee_PHY_Continuous_Wave_Start);

    printf("Completed BroadBee_PHY_Continuous_Wave_Start(%d)\r\n", modulationMode);
}


static void BroadBee_PHY_Continuous_Wave_Stop()
{
    uint8_t statusBroadBee_PHY_Continuous_Wave_Stop = 0;

    Phy_Test_Continuous_Wave_Stop_ReqDescr_t req = {0};
    void BroadBee_PHY_Continuous_Wave_Stop_Callback(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *request, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Continuous_Wave_Stop = 1;
    }
    req.callback = BroadBee_PHY_Continuous_Wave_Stop_Callback;

    Phy_Test_Continuous_Wave_Stop_Req(&req);
    while(!statusBroadBee_PHY_Continuous_Wave_Stop);

    printf("Completed BroadBee_PHY_Continuous_Wave_Stop()\r\n");
}


static void BroadBee_PHY_Transmit_Start()
{
    uint8_t statusBroadBee_PHY_Transmit_Start = 0;

    Phy_Test_Transmit_Start_ReqDescr_t req = {0};
    void BroadBee_PHY_Transmit_Start_Callback(Phy_Test_Transmit_Start_ReqDescr_t *request, Phy_Test_Transmit_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Transmit_Start = 1;
    }
    req.params.intervalMs = 1;
    req.params.packetCount = 1000;
    req.params.payloadLength = 127; // Min = 5, Max = 127

    uint8_t minIntervalMs = ((req.params.payloadLength + 6 + 3 + 32) >> 5);
    if (req.params.intervalMs < minIntervalMs)
        req.params.intervalMs = minIntervalMs;

    for(int i = 0; i < sizeof(req.params.payload); i++)
        req.params.payload[i] = i & 0xff;
    req.callback = BroadBee_PHY_Transmit_Start_Callback;

    Phy_Test_Transmit_Start_Req(&req);
    while(!statusBroadBee_PHY_Transmit_Start);

    printf("Completed BroadBee_PHY_Transmit_Start()\r\n");
}


static void BroadBee_PHY_Transmit_Stop()
{
    uint8_t statusBroadBee_PHY_Transmit_Stop = 0;

    Phy_Test_Transmit_Stop_ReqDescr_t req = {0};
    void BroadBee_PHY_Transmit_Stop_Callback(Phy_Test_Transmit_Stop_ReqDescr_t *request, Phy_Test_Transmit_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Transmit_Stop = 1;
    }
    req.callback = BroadBee_PHY_Transmit_Stop_Callback;

    Phy_Test_Transmit_Stop_Req(&req);
    while(!statusBroadBee_PHY_Transmit_Stop);

    printf("Completed BroadBee_PHY_Transmit_Stop()\r\n");
}


static void BroadBee_PHY_Receive_Start()
{
    uint8_t statusBroadBee_PHY_Receive_Start = 0;

    Phy_Test_Receive_Start_ReqDescr_t req = {0};
    req.params.shortAddressTo = 0xdb80;     //????
    void BroadBee_PHY_Receive_Start_Callback(Phy_Test_Receive_Start_ReqDescr_t *request, Phy_Test_Receive_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Receive_Start = 1;
    }
    req.callback = BroadBee_PHY_Receive_Start_Callback;

    Phy_Test_Receive_Start_Req(&req);
    while(!statusBroadBee_PHY_Receive_Start);

    printf("Completed BroadBee_PHY_Receive_Start()\r\n");
}


static void BroadBee_PHY_Receive_Stop()
{
    uint8_t statusBroadBee_PHY_Receive_Stop = 0;

    Phy_Test_Receive_Stop_ReqDescr_t req = {0};
    void BroadBee_PHY_Receive_Stop_Callback(Phy_Test_Receive_Stop_ReqDescr_t *request, Phy_Test_Receive_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Receive_Stop = 1;
    }
    req.callback = BroadBee_PHY_Receive_Stop_Callback;

    Phy_Test_Receive_Stop_Req(&req);
    while(!statusBroadBee_PHY_Receive_Stop);

    printf("Completed BroadBee_PHY_Receive_Stop()\r\n");
}


static void BroadBee_PHY_Echo_Start()
{
    uint8_t statusBroadBee_PHY_Echo_Start = 0;

    Phy_Test_Echo_Start_ReqDescr_t req = {0};
    void BroadBee_PHY_Echo_Start_Callback(Phy_Test_Echo_Start_ReqDescr_t *request, Phy_Test_Echo_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Echo_Start = 1;
    }
    req.callback = BroadBee_PHY_Echo_Start_Callback;

    Phy_Test_Echo_Start_Req(&req);
    while(!statusBroadBee_PHY_Echo_Start);

    printf("Completed BroadBee_PHY_Echo_Start()\r\n");
}


static void BroadBee_PHY_Echo_Stop()
{
    uint8_t statusBroadBee_PHY_Echo_Stop = 0;

    Phy_Test_Echo_Stop_ReqDescr_t req = {0};
    void BroadBee_PHY_Echo_Stop_Callback(Phy_Test_Echo_Stop_ReqDescr_t *request, Phy_Test_Echo_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Echo_Stop = 1;
    }
    req.callback = BroadBee_PHY_Echo_Stop_Callback;

    Phy_Test_Echo_Stop_Req(&req);
    while(!statusBroadBee_PHY_Echo_Stop);

    printf("Completed BroadBee_PHY_Echo_Stop()\r\n");
}


static void BroadBee_PHY_Energy_Detect_Scan()
{
    uint8_t statusBroadBee_PHY_Energy_Detect_Scan = 0;

    Phy_Test_Energy_Detect_Scan_ReqDescr_t req = {0};
    void BroadBee_PHY_Energy_Detect_Scan_Callback(Phy_Test_Energy_Detect_Scan_ReqDescr_t *request, Phy_Test_Energy_Detect_Scan_ConfParams_t *conf)
    {
        Phy_Sap_RF4CE_EnergyDetectionScanResults *energies = ALLOCA(SYS_GetPayloadSize(&conf->payload));
        SYS_CopyFromPayload(energies, &conf->payload, 0, SYS_GetPayloadSize(&conf->payload));
        for(int i = 0; i < SYS_GetPayloadSize(&conf->payload) / sizeof(Phy_Sap_RF4CE_EnergyDetectionScanResults); i++){
            printf("%02x\n", energies[i].energy);
        }
        statusBroadBee_PHY_Energy_Detect_Scan = 1;
    }
    req.params.numberOfScans = 10;
    req.callback = BroadBee_PHY_Energy_Detect_Scan_Callback;

    Phy_Test_Energy_Detect_Scan_Req(&req);
    while(!statusBroadBee_PHY_Energy_Detect_Scan);

    printf("Completed BroadBee_PHY_Energy_Detect_Scan()\r\n");
}


static void BroadBee_PHY_Get_Stats()
{
    uint8_t statusPhy_Test_Get_Stats = 0;

    Phy_Test_Get_Stats_ReqDescr_t req = {0};
    void BroadBee_PHY_Get_Stats_Callback(Phy_Test_Get_Stats_ReqDescr_t *request, Phy_Test_Get_Stats_ConfParams_t *conf)
    {
        printf("RX v: %08x\n",  conf->packetsReceived);
        printf("RX x: %08x\n",  conf->packetsOverflow);
        printf("TX v: %08x\n",  conf->packetsSentOK);
        printf("TX x: %08x\n",  conf->packetsSentError);

        statusPhy_Test_Get_Stats = 1;
    }
    req.callback = BroadBee_PHY_Get_Stats_Callback;

    Phy_Test_Get_Stats_Req(&req);
    while(!statusPhy_Test_Get_Stats);

    printf("Completed BroadBee_PHY_Get_Stats()\r\n");
}


static void BroadBee_PHY_Reset_Stats()
{
    uint8_t statusBroadBee_PHY_Reset_Stats = 0;

    Phy_Test_Reset_Stats_ReqDescr_t req = {0};
    void BroadBee_PHY_Reset_Stats_Callback(Phy_Test_Reset_Stats_ReqDescr_t *request, Phy_Test_Reset_Stats_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Reset_Stats = 1;
    }
    req.callback = BroadBee_PHY_Reset_Stats_Callback;

    Phy_Test_Reset_Stats_Req(&req);
    while(!statusBroadBee_PHY_Reset_Stats);

    printf("Completed BroadBee_PHY_Reset_Stats()\r\n");
}


static void BroadBee_PHY_Get_TxPower()
{
    uint8_t status_BroadBee_PHY_Get_TxPower = 0;

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER;
    void BroadBee_PHY_Get_TxPower_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("powerMax = %d\n", conf->u.txPower.powerMax);
        printf("powerMin = %d\n", conf->u.txPower.powerMin);
        printf("power    = %d\n", conf->u.txPower.power);
        status_BroadBee_PHY_Get_TxPower = 1;
    }
    req.callback = BroadBee_PHY_Get_TxPower_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_BroadBee_PHY_Get_TxPower);

    printf("Completed BroadBee_PHY_Get_TxPower()\r\n");
}


static void BroadBee_PHY_Set_TxPower(int power)
{
    uint8_t statusBroadBee_PHY_Set_TxPower = 0;

    void BroadBee_PHY_Set_TxPower_Callback(Phy_Test_Set_TX_Power_ReqDescr_t *request, Phy_Test_Set_TX_Power_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Set_TxPower = 1;
    }

    Phy_Test_Set_TX_Power_ReqDescr_t req = {0};
    req.params.power = power;
    req.callback = BroadBee_PHY_Set_TxPower_Callback;

    Phy_Test_Set_TX_Power_Req(&req);
    while(!statusBroadBee_PHY_Set_TxPower);

    printf("Completed BroadBee_PHY_Set_TxPower(%d)\r\n", power);
}


static void BroadBee_PHY_Select_Antenna(RF4CE_CTRL_ANTENNA selAnt)
{
    uint8_t statusBroadBee_PHY_Select_Antenna = 0;

    void BroadBee_PHY_Set_TxPower_Callback(Phy_Test_Select_Antenna_ReqDescr_t *request, Phy_Test_Select_Antenna_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusBroadBee_PHY_Select_Antenna = 1;
    }

    Phy_Test_Select_Antenna_ReqDescr_t req = {0};
    req.params.antenna = selAnt;    //RF4CE_ANTENNA_TX_ANT1_RX_ANT1;
    req.callback = BroadBee_PHY_Set_TxPower_Callback;

    Phy_Test_SelectAntenna_Req(&req);
    while(!statusBroadBee_PHY_Select_Antenna);

    printf("Completed BroadBee_PHY_Select_Antenna(%d)\r\n", selAnt);
}


/* eof rf4ce_common.h */
