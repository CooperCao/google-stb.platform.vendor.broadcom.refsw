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


/* Called, only once in a lifetime */
void My_RF4CE_ZRC_PairInd(RF4CE_PairingIndParams_t *indication)
{
    printf("MY_RF4CE_APP:  In My_RF4CE_ZRC_PairInd, indication->pairingRef=0x%x\n", indication->pairingRef);
}

/* Called, only once in a lifetime */
void My_RF4CE_ZRC_CheckValidationInd( RF4CE_ZRC2_CheckValidationIndParams_t *indication)
{
    if(fork() == 0)
    {
        //RF4CE_ZRC2_CheckValidationRespConfParams_t conf;
        printf("Please press Y/N to accept/not accept this validation code, Or press Enter to continue\n");
        uint8_t keyCode = getchar();
        switch(keyCode){
            case 'Y':
                //conf.status = RF4CE_GDP_VALIDATION_SUCCESS;
                break;
            case 'N':
                //conf.status = RF4CE_GDP_VALIDATION_FAILURE;
                break;
             default:
                //conf.status = RF4CE_GDP_VALIDATION_PENDING;
                break;
        }
        //indication->callback(indication, &conf);
        exit(0);
    }
}


void my_RF4CE_StartReq_callback(/*RF4CE_StartReqDescr_t*/ void *request, RF4CE_StartResetConfParams_t *conf)
{
    printf("MY_RF4CE_APP:  my_RF4CE_StartReq_callback called!\n", request);
}

void my_RF4CE_ZRC1_GetAttributesReq_callback(RF4CE_ZRC1_GetAttributeDescr_t *req, RF4CE_ZRC1_GetAttributeConfParams_t *conf)
{
    printf("***********%p\n", req);
    printf("\n");
    printf("MY_RF4CE_APP:  my_RF4CE_NWK_GetReq_callback called! status: %02x \n", conf->status);
}


#define ECHO_PAYLOAD_SIZE 200
static TE_EchoCommandReqDescr_t echoRequest;

static void rf4ce_Test_Show_My_Interest(uint8_t pairRef)
{
#define PAN_ID      0x5678
#define PROFILE_ID  0x1234
#define IEEE_ADDR   "\xaa\xbb\xcc\xdd\xee\xff\x00\x11"
#define VENDOR_ID   0x9876
#define VENDOR_STRING "BRCM"
/* To test this function, this kinds of parameters need to match with stack */
    uint8_t statusRegistered = 0;
    RF4CE_RegisterVirtualDeviceReqDescr_t req;
    memset(&req, 0, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    //req.params.fieldValidMask = RF4CE_ORGVENDORID_MASK | RF4CE_ORGPROFILEID_MASK | RF4CE_MACADDRESS_MASK;
    req.params.fieldValidMask = RF4CE_PAIRREF_MASK;
    //req.params.orgVendorId = VENDOR_ID;
    //req.params.orgProfileID = PROFILE_ID;
    req.params.pairRef = pairRef;
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

static uint8_t statusStarted = 0;
void rf4ce_Test_Start_NWK_Callback(RF4CE_StartReqDescr_t *request, RF4CE_StartResetConfParams_t *conf)
{
    //if(RF4CE_START_RESET_OK == conf->status)
    printf("RF4CE_Start_NWK_Callback status : %d\n", conf->status);
    statusStarted = 1;
}

static void rf4ce_Test_Start_NWK()
{
    printf("Starting RF4CE NWK\r\n");

    RF4CE_StartReqDescr_t req;
    req.callback = rf4ce_Test_Start_NWK_Callback;

    RF4CE_StartReq(&req);
    while(!statusStarted);
    printf("Start NWK successfully\r\n");
}

static uint8_t statusTargetBinding = 0, statusConf = 0;

static void rf4ce_Test_ZRC1_TargetBinding_Callback(RF4CE_ZRC1_BindReqDescr_t *request, RF4CE_ZRC1_BindConfParams_t *conf)
{
    statusTargetBinding = 1;
    statusConf = conf->status;
    if(RF4CE_ZRC1_BOUND == statusConf)
        printf("One remote control has been bound successfully.\r\n");
    else
        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
}

static void rf4ce_Test_ZRC1_TargetBinding()
{
    printf("Starting ZRC1 target Binding\r\n");

    RF4CE_ZRC1_BindReqDescr_t req;

    req.callback = rf4ce_Test_ZRC1_TargetBinding_Callback;

    statusTargetBinding=0;
    RF4CE_ZRC1_TargetBindReq(&req);
    while(!statusTargetBinding);
//    if(RF4CE_ZRC1_BOUND == statusConf)
//        printf("One remote control has been bound successfully.\r\n");
//    else
//        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
}

static uint8_t statusEnableBinding = 0;
static void rf4ce_Test_ZRC2_EnableBinding_Callback(RF4CE_ZRC2_BindingReqDescr_t *request, RF4CE_ZRC2_BindingConfParams_t *conf)
{
    statusEnableBinding = 1;

    if(RF4CE_ZRC2GDP2_SUCCESS == conf->status)
        printf("One remote control has been bound successfully.\r\n");
    else
        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
}

static void rf4ce_Test_ZRC2_EnableBinding()
{
    printf("Starting ZRC2 Enable Binding\r\n");
    RF4CE_ZRC2_BindingReqDescr_t req;

    req.callback = rf4ce_Test_ZRC2_EnableBinding_Callback;

    RF4CE_ZRC2_EnableBindingReq(&req);
    while(!statusEnableBinding);
    printf("Enable Binding successfully\r\n");
}


static void rf4ce_Test_Get_WakeUpActionCode()
{
    uint8_t statusGetPowerFilterKey = 0;

    RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t req = {0};

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
    printf("Get power filter key successfully\r\n");
}

static void rf4ce_Test_Set_WakeUpActionCode()
{
    uint8_t statusSetPowerFilterKey = 0;

    RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t req = {0};
    void rf4ce_Test_Set_PowerFilterKey_Callback(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *request, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetPowerFilterKey = 1;
    }
    memcpy(req.params.wakeUpActionCodeFilter, "\x00\x00\x00\x00\x00\x00\x00\x00\x02", 9);
    req.callback = rf4ce_Test_Set_PowerFilterKey_Callback;

    RF4CE_ZRC_SetWakeUpActionCodeReq(&req);
    while(!statusSetPowerFilterKey);
    printf("Set power filter key successfully\r\n");
}

static void dumpPairingEntry(RF4CE_PairingTableEntry_t *entry)
{
    printf("dstIeeeAddr : %016llX\n", entry->dstIeeeAddr);
    printf("dstPanId :    %04X\n", entry->dstPanId);
    printf("dstNetAddr :  %04X\n", entry->dstNetAddr);
}

static void rf4ce_Test_Get_PairingEntry(char pairingRef)
{
    uint8_t statusGetParingEntry = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};
    void rf4ce_Test_Get_PairingEntry_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            RF4CE_PairingTableEntry_t *entry = &conf->data.nwkPairingTableEntry;
            dumpPairingEntry(entry);
        }
        statusGetParingEntry = 1;
    }

    req.params.attrId.attrId = RF4CE_NWK_PAIRING_TABLE;
    req.params.attrId.attrIndex = pairingRef;

    req.callback = rf4ce_Test_Get_PairingEntry_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetParingEntry);
    printf("Get Pairing Entry successfully\r\n");

}

static uint8_t callback_status = 0;
void my_EchoReq_callback(TE_EchoCommandReqDescr_t *req, TE_EchoCommandConfParams_t *conf)
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
        test_passed=false;

    callback_status=(test_passed ? 1 : 0);
    return;
}

void loopback_test(int words_to_test)
{
    uint32_t testPattern[ECHO_PAYLOAD_SIZE];
    //printf("words to test:  %d\n", words_to_test);
    for(int i = 0; i < ECHO_PAYLOAD_SIZE/*sizeof(testPattern)*/; i++)
        testPattern[i] = i; //0xdf;

    memset(&echoRequest, 0, sizeof(TE_EchoCommandReqDescr_t));
    echoRequest.callback = my_EchoReq_callback;
    SYS_MemAlloc(&echoRequest.params.payload, words_to_test*4);
    SYS_CopyToPayload(&echoRequest.params.payload, 0, testPattern, words_to_test*4);
    echoRequest.params.echoSize = words_to_test*4; //ECHO_PAYLOAD_SIZE*4;
    callback_status=2;
    Mail_TestEngineEcho(&echoRequest);
    while (callback_status==2);
    printf("Ping to stack successfully\n");
}

static void rf4ce_Send_Vendor_Frame(uint8_t *vendorFrame, int length)
{
    uint8_t statusSendVendorFrame = 0;

    RF4CE_ZRC1_VendorSpecificReqDescr_t req = {0};
    void rf4ce_Send_Vendor_Frame_Callback(RF4CE_ZRC1_VendorSpecificReqDescr_t *request, RF4CE_ZRC1_VendorSpecificConfParams_t *conf)
    {
        statusSendVendorFrame = 1;
    }
    req.params.pairingRef = 0;
    req.params.profileID = 1;
    req.params.vendorID = 0x1234;
    req.params.nsduLength = length;
    req.params.txFlags = RF4CE_NWK_DATA_TX_SECURITY | RF4CE_NWK_DATA_TX_VENDOR_SPECIFIC;
    SYS_MemAlloc(&req.params.payload, length);
    SYS_CopyToPayload(&req.params.payload, 0, vendorFrame, length);
    req.callback = rf4ce_Send_Vendor_Frame_Callback;

    RF4CE_ZRC1_VendorSpecificReq(&req);
    while(!statusSendVendorFrame);
    printf("rf4ce Sent Vendor Frame successfully\n");
}

static void rf4ce_Restore_Factory_Settings(uint8_t restore)
{
    uint8_t statusRestoreFactorySettings = 0;

    RF4CE_ResetReqDescr_t req = {0};
    void rf4ce_Restore_Factory_Settings_Callback(RF4CE_ResetReqDescr_t *request, RF4CE_StartResetConfParams_t *conf)
    {
        if(RF4CE_START_RESET_OK == conf->status)
            statusRestoreFactorySettings = 1;
    }
    req.params.setDefaultPIBNIB = restore;
    req.callback = rf4ce_Restore_Factory_Settings_Callback;

    RF4CE_ResetReq(&req);
    while(!statusRestoreFactorySettings);
    printf("Restore factory settings successfully\r\n");
}

void My_RF4CE_ZRC_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *commandInd)
{
    {
        printf("Got the control command as following\r\n");
        char command[200], msg[10] = {0};
        RF4CE_ZRC2_Action_t *commands = ALLOCA(SYS_GetPayloadSize(&commandInd->payload));
        SYS_CopyFromPayload(commands, &commandInd->payload, 0, SYS_GetPayloadSize(&commandInd->payload));
        for(int i = 0; i < SYS_GetPayloadSize(&commandInd->payload) / sizeof(RF4CE_ZRC2_Action_t); i++){
            msg[i] = commands[i].code;
            printf("%c\n", msg[i]);
        }
        sprintf(command, "./figlet -f roman -d ./fonts/ %s", msg);
        system(command);
    }
}

void My_RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *commandInd)
{
    {
        char *controlMsg[] = {"", "Pressed", "Repeated", "Released", ""};
        //printf("Got the control command as following:\r\n");
        char command[200], msg[10] = {0};
        msg[0] = commandInd->commandCode;
        uint8_t rxLinkQuality;
        if(SYS_GetPayloadSize(&commandInd->payload)){
            SYS_CopyFromPayload(&rxLinkQuality, &commandInd->payload, 0, sizeof(rxLinkQuality));
        }
        if(rxLinkQuality)
            printf("Key code=%02X is %s  RSSI:%.1f dBm\r\n\r\n", msg[0], controlMsg[commandInd->flags & 0x3], -0.5*(256 - rxLinkQuality));
        else
            printf("Key code=%02X is %s  \r\n\r\n", msg[0], controlMsg[commandInd->flags & 0x3]);
    }
}

void My_RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indParams)
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

const char bindingInstruction[] = "\nNow a remote control can be bound\n\n"
                                  "Binding instruction for RemoteSolution remote control:\n"
                                  "1. Press and hold the Setup button on the remote control,\n"
                                  "   until the Indicator LED changes from red to green.\n"
                                  "2. Press the Info button on the remote control.\n"
                                  "Above procedure should be executed within 30 seconds.\n";

const char prebindingInstruction[] = "\nPlease press 'b' to issue binding procedure,\n"
                                     "press any other key to use the existing pair reference.\n";

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;

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
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = My_RF4CE_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = My_RF4CE_ZRC_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd = My_RF4CE_ZRC_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd = My_RF4CE_ZRC1_ControlCommandInd;
	zcb.RF4CE_ZRC1_VendorSpecificInd = My_RF4CE_ZRC1_VendorSpecificInd;

    Zigbee_Open(&zcb, argv[1]);

    //rf4ce_Restore_Factory_Settings(1);
    //rf4ce_Test_Show_My_Interest();
    //rf4ce_Test_Set_WakeUpActionCode();
    //rf4ce_Test_Get_WakeUpActionCode();
    //rf4ce_Test_Start_NWK();

    printf(prebindingInstruction);
    char key = getchar();
    switch(key){
        case 'b':
            rf4ce_Test_Set_WakeUpActionCode();
            rf4ce_Test_Get_WakeUpActionCode();
            rf4ce_Test_Start_NWK();
            printf(bindingInstruction);
            rf4ce_Test_ZRC1_TargetBinding();
            break;
        case 'e':
            rf4ce_Test_Set_WakeUpActionCode();
            rf4ce_Test_Get_WakeUpActionCode();
            rf4ce_Test_Start_NWK();
            printf(bindingInstruction);
            rf4ce_Test_ZRC1_TargetBinding();
            break;
        default:
            rf4ce_Test_Set_WakeUpActionCode();
            rf4ce_Test_Get_WakeUpActionCode();
            rf4ce_Test_Start_NWK();
            printf("\nPlease input the pair ref#[0-9]\n");
            char key = getchar();
            rf4ce_Test_Show_My_Interest(key - 0x30);
            break;
    }
//}
    printf("\nPress Key 'p' to send echo packet to stack, any other key to exit\n");
    while(1){
        key = getchar();
        switch(key){
            case 'p':
            case 'P':
                loopback_test(20);
                break;
            case 't':
            case 'T':
                printf("\nPlease input the pair ref#[0-9]\n");
                char pairingRef = getchar() - '0';
                rf4ce_Test_Get_PairingEntry(pairingRef);
                break;
            case 's':
            case 'S':
                rf4ce_Send_Vendor_Frame("\x12\x34\x56\x78\x9a\xbc\xde\xf0", 8);
                break;
            default:
                goto _exit;
        }
    }
_exit:
    printf("MY_RF4CE_APP:  closing...\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}
