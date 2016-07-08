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
//#include "bbMailTestEngine.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
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

void my_EchoReq_callback(TE_EchoCommandReqDescr_t *req, TE_EchoCommandConfParams_t *conf)
{
    uint8_t verifyBuffer[ECHO_PAYLOAD_SIZE];
    int iter;
    SYS_CopyFromPayload(verifyBuffer, &conf->payload, 0, ECHO_PAYLOAD_SIZE);
    for(iter = 0; iter < ECHO_PAYLOAD_SIZE; iter++)
        if(verifyBuffer[iter] != 0xdf){
            printf("loopback data mismatch\n");
            break;
        }
    if(iter == ECHO_PAYLOAD_SIZE)
        printf("loopback data match!!!!!!!!!!!!\n");
    return;
}

static void rf4ce_Test_Show_My_Interest()
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
    req.params.pairRef = 0;
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

void rf4ce_Test_ZRC1_TargetBinding_Callback(RF4CE_ZRC1_BindReqDescr_t *request, RF4CE_ZRC1_BindConfParams_t *conf)
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

    RF4CE_ZRC1_TargetBindReq(&req);
//    while(!statusTargetBinding);
//    if(RF4CE_ZRC1_BOUND == statusConf)
//        printf("One remote control has been bound successfully.\r\n");
//    else
//        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
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

static void rf4ce_Test_Set_Single_Channel(uint8_t channel)
{
    uint8_t statusSetSingleChannel = 0;

    RF4CE_NWK_SetReqDescr_t req = {0};
    void rf4ce_Test_Set_Single_Channel_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetSingleChannel = 1;
    }

    req.params.attrId.attrId = RF4CE_NWK_BASE_CHANNEL;
    req.params.data.nwkBaseChannel = channel;
    req.callback = rf4ce_Test_Set_Single_Channel_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!statusSetSingleChannel);

    printf("Set Single Channel successfully\r\n");
}

static void phy_Test_Get_Caps()
{
    uint8_t statusPhy_Test_Get_Caps = 0;

    Phy_Test_Get_Caps_ReqDescr_t req = {0};
    void phy_Test_Get_Caps_Callback(Phy_Test_Get_Caps_ReqDescr_t *request, Phy_Test_Get_Caps_ConfParams_t *conf)
    {
        printf("%d\n",  conf->channelMin);
        printf("%d\n",  conf->channelMax);
        printf("%d\n",  conf->powerMin);
        printf("%d\n",  conf->powerMax);

        statusPhy_Test_Get_Caps = 1;
    }
    req.callback = phy_Test_Get_Caps_Callback;

    Phy_Test_Get_Caps_Req(&req);
    while(!statusPhy_Test_Get_Caps);

    printf("phy_Test_Get_Caps successfully\r\n");

}

static void phy_Test_Set_Channel()
{
    uint8_t statusphy_Test_Set_Channel = 0;

    Phy_Test_Set_Channel_ReqDescr_t req = {0};
    void phy_Test_Set_Channel_Callback(Phy_Test_Set_Channel_ReqDescr_t *request, Phy_Test_Set_Channel_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Set_Channel = 1;
    }
    req.callback = phy_Test_Set_Channel_Callback;

    Phy_Test_Set_Channel_Req(&req);
    while(!statusphy_Test_Set_Channel);

    printf("phy_Test_Set_Channel successfully\r\n");

}

static void phy_Test_Continuous_Wave_Start()
{
    uint8_t statusphy_Test_Continuous_Wave_Start = 0;

    Phy_Test_Continuous_Wave_Start_ReqDescr_t req = {0};
    void phy_Test_Continuous_Wave_Start_Callback(Phy_Test_Continuous_Wave_Start_ReqDescr_t *request, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Continuous_Wave_Start = 1;
    }
    req.callback = phy_Test_Continuous_Wave_Start_Callback;

    Phy_Test_Continuous_Wave_Start_Req(&req);
    while(!statusphy_Test_Continuous_Wave_Start);

    printf("phy_Test_Continuous_Wave_Start successfully\r\n");

}

static void phy_Test_Continuous_Wave_Stop()
{
    uint8_t statusphy_Test_Continuous_Wave_Stop = 0;

    Phy_Test_Continuous_Wave_Stop_ReqDescr_t req = {0};
    void phy_Test_Continuous_Wave_Stop_Callback(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *request, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Continuous_Wave_Stop = 1;
    }
    req.callback = phy_Test_Continuous_Wave_Stop_Callback;

    Phy_Test_Continuous_Wave_Stop_Req(&req);
    while(!statusphy_Test_Continuous_Wave_Stop);

    printf("phy_Test_Continuous_Wave_Stop successfully\r\n");

}

static void phy_Test_Transmit_Start()
{
    uint8_t statusphy_Test_Transmit_Start = 0;

    Phy_Test_Transmit_Start_ReqDescr_t req = {0};
    void phy_Test_Transmit_Start_Callback(Phy_Test_Transmit_Start_ReqDescr_t *request, Phy_Test_Transmit_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Transmit_Start = 1;
    }
    req.params.intervalMs = 0x1;
    req.params.packetCount = 0x02;
    req.params.payloadLength = 125;
    for(int i = 0; i < sizeof(req.params.payload); i++)
        req.params.payload[i] = i & 0xff;
    req.callback = phy_Test_Transmit_Start_Callback;

    Phy_Test_Transmit_Start_Req(&req);
    while(!statusphy_Test_Transmit_Start);

    printf("phy_Test_Transmit_Start successfully\r\n");

}

static void phy_Test_Transmit_Stop()
{
    uint8_t statusphy_Test_Transmit_Stop = 0;

    Phy_Test_Transmit_Stop_ReqDescr_t req = {0};
    void phy_Test_Transmit_Stop_Callback(Phy_Test_Transmit_Stop_ReqDescr_t *request, Phy_Test_Transmit_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Transmit_Stop = 1;
    }
    req.callback = phy_Test_Transmit_Stop_Callback;

    Phy_Test_Transmit_Stop_Req(&req);
    while(!statusphy_Test_Transmit_Stop);

    printf("phy_Test_Transmit_Stop successfully\r\n");

}

static void phy_Test_Receive_Start()
{
    uint8_t statusphy_Test_Receive_Start = 0;

    Phy_Test_Receive_Start_ReqDescr_t req = {0};
    void phy_Test_Receive_Start_Callback(Phy_Test_Receive_Start_ReqDescr_t *request, Phy_Test_Receive_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Receive_Start = 1;
    }
    req.callback = phy_Test_Receive_Start_Callback;

    Phy_Test_Receive_Start_Req(&req);
    while(!statusphy_Test_Receive_Start);

    printf("phy_Test_Receive_Start successfully\r\n");

}

static void phy_Test_Receive_Stop()
{
    uint8_t statusphy_Test_Receive_Stop = 0;

    Phy_Test_Receive_Stop_ReqDescr_t req = {0};
    void phy_Test_Receive_Stop_Callback(Phy_Test_Receive_Stop_ReqDescr_t *request, Phy_Test_Receive_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Receive_Stop = 1;
    }
    req.callback = phy_Test_Receive_Stop_Callback;

    Phy_Test_Receive_Stop_Req(&req);
    while(!statusphy_Test_Receive_Stop);

    printf("phy_Test_Receive_Stop successfully\r\n");

}

static void phy_Test_Echo_Start()
{
    uint8_t statusphy_Test_Echo_Start = 0;

    Phy_Test_Echo_Start_ReqDescr_t req = {0};
    void phy_Test_Echo_Start_Callback(Phy_Test_Echo_Start_ReqDescr_t *request, Phy_Test_Echo_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Echo_Start = 1;
    }
    req.callback = phy_Test_Echo_Start_Callback;

    Phy_Test_Echo_Start_Req(&req);
    while(!statusphy_Test_Echo_Start);

    printf("phy_Test_Echo_Start successfully\r\n");

}

static void phy_Test_Echo_Stop()
{
    uint8_t statusphy_Test_Echo_Stop = 0;

    Phy_Test_Echo_Stop_ReqDescr_t req = {0};
    void phy_Test_Echo_Stop_Callback(Phy_Test_Echo_Stop_ReqDescr_t *request, Phy_Test_Echo_StartStop_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Echo_Stop = 1;
    }
    req.callback = phy_Test_Echo_Stop_Callback;

    Phy_Test_Echo_Stop_Req(&req);
    while(!statusphy_Test_Echo_Stop);

    printf("phy_Test_Echo_Stop successfully\r\n");

}


static void phy_Test_Energy_Detect_Scan()
{
    uint8_t statusphy_Test_Energy_Detect_Scan = 0;

    Phy_Test_Energy_Detect_Scan_ReqDescr_t req = {0};
    void phy_Test_Energy_Detect_Scan_Callback(Phy_Test_Energy_Detect_Scan_ReqDescr_t *request, Phy_Test_Energy_Detect_Scan_ConfParams_t *conf)
    {
        Phy_Sap_RF4CE_EnergyDetectionScanResults *energies = ALLOCA(SYS_GetPayloadSize(&conf->payload));
        SYS_CopyFromPayload(energies, &conf->payload, 0, SYS_GetPayloadSize(&conf->payload));
        for(int i = 0; i < SYS_GetPayloadSize(&conf->payload) / sizeof(Phy_Sap_RF4CE_EnergyDetectionScanResults); i++){
            printf("%02x\n", energies[i].energy);
        }
        statusphy_Test_Energy_Detect_Scan = 1;
    }
    req.params.numberOfScans = 10;
    req.callback = phy_Test_Energy_Detect_Scan_Callback;

    Phy_Test_Energy_Detect_Scan_Req(&req);
    while(!statusphy_Test_Energy_Detect_Scan);

    printf("phy_Test_Energy_Detect_Scan successfully\r\n");

}

static void phy_Test_Get_Stats()
{
    uint8_t statusPhy_Test_Get_Stats = 0;

    Phy_Test_Get_Stats_ReqDescr_t req = {0};
    void phy_Test_Get_Stats_Callback(Phy_Test_Get_Stats_ReqDescr_t *request, Phy_Test_Get_Stats_ConfParams_t *conf)
    {
        printf("%08x\n",  conf->packetsReceived);
        printf("%08x\n",  conf->packetsOverflow);
        printf("%08x\n",  conf->packetsSentOK);
        printf("%08x\n",  conf->packetsSentError);

        statusPhy_Test_Get_Stats = 1;
    }
    req.callback = phy_Test_Get_Stats_Callback;

    Phy_Test_Get_Stats_Req(&req);
    while(!statusPhy_Test_Get_Stats);

    printf("phy_Test_Get_Stats successfully\r\n");
}

static void phy_Test_Reset_Stats()
{
    uint8_t statusphy_Test_Reset_Stats = 0;

    Phy_Test_Reset_Stats_ReqDescr_t req = {0};
    void phy_Test_Reset_Stats_Callback(Phy_Test_Reset_Stats_ReqDescr_t *request, Phy_Test_Reset_Stats_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Reset_Stats = 1;
    }
    req.callback = phy_Test_Reset_Stats_Callback;

    Phy_Test_Reset_Stats_Req(&req);
    while(!statusphy_Test_Reset_Stats);

    printf("phy_Test_Reset_Stats successfully\r\n");

}

static void phy_Test_Set_TX_Power()
{
    uint8_t statusphy_Test_Set_TX_Power = 0;

    Phy_Test_Set_TX_Power_ReqDescr_t req = {0};
    void phy_Test_Set_TX_Power_Callback(Phy_Test_Set_TX_Power_ReqDescr_t *request, Phy_Test_Set_TX_Power_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Set_TX_Power = 1;
    }
    req.callback = phy_Test_Set_TX_Power_Callback;

    Phy_Test_Set_TX_Power_Req(&req);
    while(!statusphy_Test_Set_TX_Power);

    printf("phy_Test_Set_TX_Power successfully\r\n");

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
        printf("Got the control command as following:\r\n");
        char command[200], msg[10] = {0};
        msg[0] = commandInd->commandCode;
        printf("  symbol='%c' code=%02X\r\n\r\n", msg[0], msg[0]);
    }
}

const char usage[] = "\nrf4ce_reset - rf4ce reset utility\n\n"
                     "    usage: rf4ce_reset [reset_default_settings]\n"
                     "    reset_default_settings\n"
                     "        1 -- reset with factory default settings.\n";

int main(int argc, char *argv[])
{
    if(argc < 2)
        return -1;
    struct zigbeeCallback zcb;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = My_RF4CE_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = My_RF4CE_ZRC_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd = My_RF4CE_ZRC_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd = My_RF4CE_ZRC1_ControlCommandInd;

    Zigbee_Open(&zcb, "127.0.0.1");

    //rf4ce_Restore_Factory_Settings(1);
    //rf4ce_Test_Show_My_Interest();
    int channel = 0;
    sscanf(argv[1], "%d", &channel);
    rf4ce_Test_Set_Single_Channel((uint8_t)channel);
    //rf4ce_Test_Get_WakeUpActionCode();
    //rf4ce_Test_Start_NWK();

    printf("\nPress any key to exist\n");
    getchar();
#if  1
    phy_Test_Get_Caps();
    phy_Test_Set_Channel();
    phy_Test_Continuous_Wave_Start();
    phy_Test_Continuous_Wave_Stop();
    phy_Test_Transmit_Start();
    phy_Test_Transmit_Stop();
    phy_Test_Receive_Start();
    phy_Test_Receive_Stop();
    phy_Test_Echo_Start();
    phy_Test_Echo_Stop();
    phy_Test_Energy_Detect_Scan();
    phy_Test_Get_Stats();
    phy_Test_Reset_Stats();
    phy_Test_Set_TX_Power();
#endif
    printf("MY_RF4CE_APP:  closing...\n");

    Zigbee_Close();

    return 0;
}
