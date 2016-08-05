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
#include "zigbee_dbg.h"
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
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args);                                                              /* TODO: Implement custom tiny formatted print. */
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


static uint8_t rf4ce_Get_numSupportedProfiles()
{
    uint8_t numSupportedProfiles = 0;
    uint8_t status_Get_numSupportedProfiles = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Get_numSupportedProfiles_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            numSupportedProfiles = conf->data.nwkNumSupportedProfiles;
            status_Get_numSupportedProfiles = 1;
        }
    }
    req.params.attrId.attrId = RF4CE_NWK_NUM_SUPPORTED_PROFILES;
    req.callback = rf4ce_Get_numSupportedProfiles_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_numSupportedProfiles);
    return numSupportedProfiles;
}

static uint8_t rf4ce_Get_pairingTableEntriesMax()
{
    uint8_t pairingTableEntriesMax = 0;
    uint8_t status_Get_pairingTableEntriesMax = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Get_pairingTableEntriesMax_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            pairingTableEntriesMax = conf->data.nwkMaxPairingTableEntries;
            status_Get_pairingTableEntriesMax = 1;
        }
    }
    req.params.attrId.attrId = RF4CE_NWK_MAX_PAIRING_TABLE_ENTRIES;
    req.callback = rf4ce_Get_pairingTableEntriesMax_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_pairingTableEntriesMax);
    return pairingTableEntriesMax;
}

static uint8_t rf4ce_Get_antennasAvailable()
{
    uint8_t antennaAvailable = 0;
    uint8_t status_Get_antennasAvailable = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Get_antennasAvailable_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            antennaAvailable = conf->data.antennaAvailable;
            status_Get_antennasAvailable = 1;
        }
    }
    req.params.attrId.attrId = RF4CE_NWK_ANTENNA_AVAILABLE;
    req.callback = rf4ce_Get_antennasAvailable_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_antennasAvailable);
    return antennaAvailable;
}

static void rf4ce_Test_Get_Caps_Ex()
{
    // Since our stack does support all the features, simply the features should include all the features.
    #define RF4CE_CTRL_FEATURE_DIAGNOSTICS (0x00000001)
    #define RF4CE_CTRL_FEATURE_PAIRING_REF (0x00000100)
    #define RF4CE_CTRL_FEATURE_TX_POWER (0x00000200)
    typedef struct
    {
        uint32_t features;
        uint32_t numSupportedProfiles;
        uint32_t pairingTableEntriesMax;
        uint32_t antennasAvailable;
    } RF4CECtrlCapsV1;

    RF4CECtrlCapsV1 capEx = { 0 };
    capEx.features = RF4CE_CTRL_FEATURE_DIAGNOSTICS | RF4CE_CTRL_FEATURE_PAIRING_REF | RF4CE_CTRL_FEATURE_TX_POWER;
    capEx.numSupportedProfiles = rf4ce_Get_numSupportedProfiles();
    capEx.pairingTableEntriesMax = rf4ce_Get_pairingTableEntriesMax();
    capEx.antennasAvailable = rf4ce_Get_antennasAvailable();
    printf("Features : 0x%08X\n", capEx.features);
    printf("numSupportedProfiles : %d\n", capEx.numSupportedProfiles);
    printf("pairingTableEntriesMax : %d\n", capEx.pairingTableEntriesMax);
    printf("antennasAvailable : 0x%08X\n", capEx.antennasAvailable);
    printf("rf4ce_Test_Get_Caps_Ex successfully\r\n");

}

static void rf4ce_Test_Get_Supported_Profiles()
{

    uint8_t antennaAvailable = 0;
    uint8_t status_Get_Supported_Profiles = 0;

    uint8_t nwkSupportedProfiles[RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH];

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Get_Supported_Profiles_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            memcpy(nwkSupportedProfiles, conf->data.nwkSupportedProfiles, sizeof(nwkSupportedProfiles));;
            status_Get_Supported_Profiles = 1;
        }
    }
    req.params.attrId.attrId = RF4CE_NWK_NUM_SUPPORTED_PROFILES;
    req.callback = rf4ce_Get_Supported_Profiles_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_Supported_Profiles);
    // directly use RF4CE_ProfilesIdList
    printf("Dumping the supported profiles\r\n");
    for(int i = 0; i < rf4ce_Get_numSupportedProfiles(); i++)
        printf("%02X\n", nwkSupportedProfiles[i]);
    printf("rf4ce_Test_Get_Supported_Profiles successfully\r\n");
}

static uint16_t rf4ce_Get_PanId()
{
    uint16_t panId = 0;
    uint8_t status_Get_PanId = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Get_PanId_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            panId = conf->data.nwkPanId;
            status_Get_PanId = 1;
        }
    }
    req.params.attrId.attrId = RF4CE_NWK_PAN_ID;
    req.callback = rf4ce_Get_PanId_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!status_Get_PanId);
    return panId;
}

static uint16_t rf4ce_Get_Mac_ShortAddress()
{
    uint16_t shortAddress = 0;
    uint8_t status_Get_Mac_ShortAddress = 0;

    MAC_GetReqDescr_t req = {0};

    void rf4ce_Get_Mac_ShortAddress_Callback(MAC_GetReqDescr_t *request, MAC_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            shortAddress = conf->attributeValue.macShortAddress;
            status_Get_Mac_ShortAddress = 1;
        }
    }
    req.params.attribute = MAC_SHORT_ADDRESS;
    req.callback = rf4ce_Get_Mac_ShortAddress_Callback;

    RF4CE_MAC_GetReq(&req);
    while(!status_Get_Mac_ShortAddress);
    return shortAddress;
}

static MAC_ExtendedAddress_t rf4ce_Get_Mac_IEEEAddress()
{
    MAC_ExtendedAddress_t ieeeAddress = 0;
    uint8_t status_Get_Mac_IEEEAddress = 0;

    MAC_GetReqDescr_t req = {0};

    void rf4ce_Get_Mac_IEEEAddress_Callback(MAC_GetReqDescr_t *request, MAC_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            ieeeAddress = conf->attributeValue.macExtendedAddress;
            status_Get_Mac_IEEEAddress = 1;
        }
    }
    req.params.attribute = MAC_EXTENDED_ADDRESS;
    req.callback = rf4ce_Get_Mac_IEEEAddress_Callback;

    RF4CE_MAC_GetReq(&req);
    while(!status_Get_Mac_IEEEAddress);
    return ieeeAddress;
}

static void rf4ce_Get_NWK_Information()
{
    typedef struct
    {
        MAC_ExtendedAddress_t address;
        uint16_t shortAddress;
        uint16_t panID;
    } RF4CECtrlNetworkInfo;

    RF4CECtrlNetworkInfo nwkInfo = {0};

    nwkInfo.address = rf4ce_Get_Mac_IEEEAddress();
    nwkInfo.shortAddress = rf4ce_Get_Mac_ShortAddress();
    nwkInfo.panID = rf4ce_Get_PanId();
    printf("rf4ce_Get_NWK_Information successfully\r\n");
}


static void rf4ce_Set_Discovery_Duration(uint32_t durationSeconds)
{
    uint8_t status_Set_Discovery_Duration = 0;

    RF4CE_ZRC1_SetAttributeDescr_t req = {0};

    void rf4ce_Set_Discovery_Duration_Callback(RF4CE_ZRC1_SetAttributeDescr_t *request, RF4CE_ZRC1_SetAttributeConfParams_t *conf)
    {
        if(0 == conf->status){
            status_Set_Discovery_Duration = 1;
        }
    }
    req.params.attributeId = GDP_AUTO_DISC_DURATION;
    req.params.data.aplGDPAutoDiscoveryDuration = durationSeconds * 62500;
    req.callback = rf4ce_Set_Discovery_Duration_Callback;

    RF4CE_ZRC1_SetAttributesReq(&req);
    while(!status_Set_Discovery_Duration);
}

static void rf4ce_Test_ZRC1_TargetBinding(uint32_t bindingTimeSeconds)
{
    printf("Starting ZRC1 target Binding\r\n");
    // to follow the CDI, we have to set a customeized pair timeout attribute.
    rf4ce_Set_Discovery_Duration(bindingTimeSeconds);

    static uint8_t statusTargetBinding = 0, statusConf = 0;

    RF4CE_ZRC1_BindReqDescr_t req;

    void rf4ce_Test_ZRC1_TargetBinding_Callback(RF4CE_ZRC1_BindReqDescr_t *request, RF4CE_ZRC1_BindConfParams_t *conf)
    {
        statusTargetBinding = 1;
        statusConf = conf->status;
        if(RF4CE_ZRC1_BOUND == statusConf)
            printf("One remote control has been bound successfully.\r\n");
        else
            printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
    }

    req.callback = rf4ce_Test_ZRC1_TargetBinding_Callback;

    statusTargetBinding = 0;
    RF4CE_ZRC1_TargetBindReq(&req);
    while(!statusTargetBinding);
    if(RF4CE_ZRC1_BOUND == statusConf)
        printf("One remote control has been bound successfully.\r\n");
    else
        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
}


static void rf4ce_Set_TX_Power_Key_Exchange(uint32_t power)
{
    uint8_t status_Set_TX_Power_Key_Exchange = 0;

    RF4CE_NWK_SetReqDescr_t req = {0};

    void rf4ce_Set_TX_Power_Key_Exchange_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        status_Set_TX_Power_Key_Exchange = 1;
    }
    req.params.attrId.attrId = RF4CE_NWK_TX_POWER_KEY_EXCHANGE;
    req.params.data.nwkTxPowerKeyExchange = power;
    req.callback = rf4ce_Set_TX_Power_Key_Exchange_Callback;

    RF4CE_NWK_SetReq(&req);
    while(!status_Set_TX_Power_Key_Exchange);
}


static bool rf4ce_Test_Get_PairingEntry(char pairingRef, RF4CE_PairingTableEntry_t *entry)
{
    bool validPairingEntry = FALSE;
    uint8_t statusGetParingEntry = 0;

    RF4CE_NWK_GetReqDescr_t req = {0};
    void rf4ce_Test_Get_PairingEntry_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            validPairingEntry = TRUE;
            memcpy(entry, &conf->data.nwkPairingTableEntry, sizeof(RF4CE_PairingTableEntry_t));
        }
        statusGetParingEntry = 1;
    }

    req.params.attrId.attrId = RF4CE_NWK_PAIRING_TABLE;
    req.params.attrId.attrIndex = pairingRef;

    req.callback = rf4ce_Test_Get_PairingEntry_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetParingEntry);
    return validPairingEntry;
}

static int rf4ce_Test_Get_Num_Paired_Devices()
{
    int numPairedDevice = 0;
    RF4CE_PairingTableEntry_t entry;
    printf("rf4ce_Get_pairingTableEntriesMax ing ...............\n");
    int numEntries = rf4ce_Get_pairingTableEntriesMax();
    printf("rf4ce_Get_pairingTableEntriesMax : %d\n", numEntries);
    for(int i = 0; i < numEntries; i++){
        if(rf4ce_Test_Get_PairingEntry(i, &entry))
            numPairedDevice++;
    }
    return numPairedDevice;
}

static void dumpPairingEntry(RF4CE_PairingTableEntry_t *entry)
{
    printf("dstIeeeAddr : %016llX\n", entry->dstIeeeAddr);
    printf("dstPanId :    %04X\n", entry->dstPanId);
    printf("dstNetAddr :  %04X\n", entry->dstNetAddr);
}

static void rf4ce_Test_Get_Paired_Devices()
{
    int numPairedDevice = 0;
    RF4CE_PairingTableEntry_t entry;
    for(int i = 0; i < rf4ce_Get_pairingTableEntriesMax(); i++){
        if(rf4ce_Test_Get_PairingEntry(i, &entry)){
            // dump out all the paired information
            dumpPairingEntry(&entry);
        }
    }
}

static void rf4ce_Test_ZRC1_Unpair(uint8_t pairRef)
{
    printf("Starting ZRC1 target Unpair\r\n");
    uint8_t statusUnpair = 0, unpairConfStatus = 0;

    RF4CE_UnpairReqDescr_t req;

    void rf4ce_Test_ZRC1_Unpair_Callback(RF4CE_UnpairReqDescr_t *req, RF4CE_UnpairConfParams_t *conf)
    {
        statusUnpair = 1;
        unpairConfStatus = conf->status;
    }
    req.params.pairingRef = pairRef;
    req.callback = rf4ce_Test_ZRC1_Unpair_Callback;

    RF4CE_UnpairReq(&req);

    while(!statusUnpair);
    if(1 == unpairConfStatus)
        printf("Unpair with pairref %d successfully.\r\n", pairRef);
    else
        printf("Fail to unpair with pairref %d Confirmation Status(%d).\r\n", pairRef, unpairConfStatus);
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
    req.params.channel = 17;

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
    req.params.packetCount = 0x05;
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
    req.params.shortAddressTo = 0xdb80;
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
        printf("RX v:%08x\n",  conf->packetsReceived);
        printf("RX x: %08x\n",  conf->packetsOverflow);
        printf("TX v: %08x\n",  conf->packetsSentOK);
        printf("TX x: %08x\n",  conf->packetsSentError);

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

static void phy_Test_Set_TX_Power(int power)
{
    uint8_t statusphy_Test_Set_TX_Power = 0;

    Phy_Test_Set_TX_Power_ReqDescr_t req = {0};
    void phy_Test_Set_TX_Power_Callback(Phy_Test_Set_TX_Power_ReqDescr_t *request, Phy_Test_Set_TX_Power_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Set_TX_Power = 1;
    }
    req.params.power = power;
    req.callback = phy_Test_Set_TX_Power_Callback;

    Phy_Test_Set_TX_Power_Req(&req);
    while(!statusphy_Test_Set_TX_Power);

    printf("phy_Test_Set_TX_Power successfully\r\n");

}

static void phy_Test_Select_Antenna()
{
    uint8_t statusphy_Test_Select_Antenna = 0;

    Phy_Test_Select_Antenna_ReqDescr_t req = {0};
    void phy_Test_Set_TX_Power_Callback(Phy_Test_Select_Antenna_ReqDescr_t *request, Phy_Test_Select_Antenna_ConfParams_t *conf)
    {
        printf("status : %02x\n",  conf->status);
        statusphy_Test_Select_Antenna = 1;
    }
    req.params.antenna = RF4CE_CTRL_ANTENNA_1;
    req.callback = phy_Test_Set_TX_Power_Callback;

    Phy_Test_SelectAntenna_Req(&req);
    while(!statusphy_Test_Select_Antenna);

    printf("phy_Test_Select_Antenna successfully\r\n");
}

static void rf4ce_Test_RF4CE_Get_Diag_Caps()
{
    uint8_t status_rf4ce_Test_RF4CE_Get_Diag_Caps = 0;

    RF4CE_Diag_Caps_ReqDescr_t req = {0};
    void rf4ce_Test_RF4CE_Get_Diag_Caps_Callback(RF4CE_Diag_Caps_ReqDescr_t *request, RF4CE_Diag_Caps_ConfParams_t *conf)
    {

        status_rf4ce_Test_RF4CE_Get_Diag_Caps = 1;
    }
    req.callback = rf4ce_Test_RF4CE_Get_Diag_Caps_Callback;

    RF4CE_Get_Diag_Caps_Req(&req);
    while(!status_rf4ce_Test_RF4CE_Get_Diag_Caps);

    printf("rf4ce_Test_RF4CE_Get_Diag_Caps successfully\r\n");
}

static void rf4ce_Test_RF4CE_Get_Diag_Agility()
{
    uint8_t status_rf4ce_Test_RF4CE_Get_Diag_Agility = 0;

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_AGILITY;
    void rf4ce_Test_RF4CE_Get_Diag_Agility_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("rf4ce_Test_RF4CE_Get_Diag_Agility_Callback got called\r\n");
        printf("agilityThreshold=%d\n", conf->u.agility.agilityThreshold);
        printf("currentChannel = %d\n", conf->u.agility.operational.logicalChannel);
        printf("bkChan1        = %d\n", conf->u.agility.bkChan1.logicalChannel);
        printf("bkChan2        = %d\n", conf->u.agility.bkChan2.logicalChannel);
        status_rf4ce_Test_RF4CE_Get_Diag_Agility = 1;
    }
    req.callback = rf4ce_Test_RF4CE_Get_Diag_Agility_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_rf4ce_Test_RF4CE_Get_Diag_Agility);

    printf("rf4ce_Test_RF4CE_Get_Diag_Agility successfully\r\n");
}

static void rf4ce_Test_RF4CE_Get_Diag_LinkQuality(uint8_t pairRef)
{
    uint8_t status_rf4ce_Test_RF4CE_Get_Diag_LinkQuality = 0;

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_LINK_QUALITY;
    req.params.u.linkQuality.pairingRef = pairRef;
    void rf4ce_Test_RF4CE_Get_Diag_LinkQuality_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("Link Quality of last received packet for pairRef(%d) = %d\n", pairRef, conf->u.linkQuality.linkQuality);
        status_rf4ce_Test_RF4CE_Get_Diag_LinkQuality = 1;
    }
    req.callback = rf4ce_Test_RF4CE_Get_Diag_LinkQuality_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_rf4ce_Test_RF4CE_Get_Diag_LinkQuality);

    printf("rf4ce_Test_RF4CE_Get_Diag_LinkQuality successfully\r\n");
}

static void rf4ce_Test_RF4CE_Get_Diag_TxPower()
{
    uint8_t status_rf4ce_Test_RF4CE_Get_Diag_TxPower = 0;

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER;
    void rf4ce_Test_RF4CE_Get_Diag_TxPower_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("powerMax = %d\n", conf->u.txPower.powerMax);
        printf("powerMin = %d\n", conf->u.txPower.powerMin);
        printf("power    = %d\n", conf->u.txPower.power);
        status_rf4ce_Test_RF4CE_Get_Diag_TxPower = 1;
    }
    req.callback = rf4ce_Test_RF4CE_Get_Diag_TxPower_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!status_rf4ce_Test_RF4CE_Get_Diag_TxPower);

    printf("rf4ce_Test_RF4CE_Get_Diag_TxPower successfully\r\n");
}

static void rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange()
{
    uint8_t rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange = 0;

    RF4CE_Diag_ReqDescr_t req = {0};
    req.params.constant = RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER_KEY_EXCHANGE;
    void rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange_Callback(RF4CE_Diag_ReqDescr_t *request, RF4CE_Diag_ConfParams_t *conf)
    {
        printf("powerMax = %d\n", conf->u.txPowerKeyExchange.powerMax);
        printf("powerMin = %d\n", conf->u.txPowerKeyExchange.powerMin);
        printf("power    = %d\n", conf->u.txPowerKeyExchange.power);
        rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange = 1;
    }
    req.callback = rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange_Callback;

    RF4CE_Get_Diag_Req(&req);
    while(!rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange);

    printf("rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange successfully\r\n");
}


static void rf4ce_Test_Subscribe_Event()
{
    SYS_EventHandlerMailParams_t eventMap = {0};
    printf("size of SYS_EventHandlerParams_t is %d %d\n", sizeof(SYS_EventHandlerMailParams_t), sizeof(eventMap.subscribedEventsMap));
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED);

    sysEventSubscribeHostHandler(&eventMap);
}

typedef struct
{
    uint32_t version;
    void *capability;
    uint32_t capabilitySize;
} RF4CEZRCInputCapsEx;

typedef struct
{
    uint32_t mode;
    uint16_t profileId;
}RF4CEZRCInputCapsV0;

#define RF4CE_ZRC_INPUT_CAP_V0 (0x00000001)
#define USER_INPUT_MODE_RAW (1)
#define USER_INPUT_MODE_COOKED (2)
#define RF4CE_PROFILE_ZRC (1)
static uint8_t zrc1_Test_Get_Extended_Cap(RF4CEZRCInputCapsEx *cap)
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

static void zrc1_Set_KeyRepeatWaitTime(uint32_t waitTime)
{
    uint8_t status_Set_KeyRepeatWaitTime = 0;

    RF4CE_ZRC1_SetAttributeDescr_t req = {0};

    void zrc1_Set_KeyRepeatWaitTime_Callback(RF4CE_ZRC1_SetAttributeDescr_t *request, RF4CE_ZRC1_SetAttributeConfParams_t *conf)
    {
        status_Set_KeyRepeatWaitTime = 1;
    }
    req.params.attributeId = RF4CE_ZRC1_APL_KEY_REPEAT_WAIT_TIME;
    req.params.data.aplZRC1KeyRepeatWaitTime = waitTime;
    req.callback = zrc1_Set_KeyRepeatWaitTime_Callback;

    RF4CE_ZRC1_SetAttributesReq(&req);
    while(!status_Set_KeyRepeatWaitTime);
}

static void zrc_Test_Input_Configure(uint8_t keyRepeatEnable)
{
    if(keyRepeatEnable)
        zrc1_Set_KeyRepeatWaitTime(RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL << 1);
    else
        zrc1_Set_KeyRepeatWaitTime(0xFFFFFFFF);
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

static void My_SYS_EventNtfy(SYS_EventNotifyParams_t *event)
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

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.SYS_EventNtfy = My_SYS_EventNtfy;
    zcb.RF4CE_PairInd = My_RF4CE_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = My_RF4CE_ZRC_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd = My_RF4CE_ZRC_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd = My_RF4CE_ZRC1_ControlCommandInd;

    Zigbee_Open(&zcb, "127.0.0.1");

    printf("\nPress any key to perform phy test\n");
#if 0
    printf("rf4ce_Test_Get_Caps_Ex\n");
    getchar();
    rf4ce_Test_Get_Caps_Ex();

    printf("rf4ce_Test_Get_Supported_Profiles\n");
    getchar();
    rf4ce_Test_Get_Supported_Profiles();

    printf("rf4ce_Get_NWK_Information\n");
    getchar();
    rf4ce_Get_NWK_Information();

    printf("rf4ce_Test_Get_Num_Paired_Devices\n");
    getchar();
    printf("%d device(s) has been paired\n", rf4ce_Test_Get_Num_Paired_Devices());

    printf("rf4ce_Test_Get_Paired_Devices\n");
    getchar();
    rf4ce_Test_Get_Paired_Devices();

    printf("rf4ce_Get_pairingTableEntriesMax\n");
    getchar();
    printf("Max Pairing Table Entries : %d\n", rf4ce_Get_pairingTableEntriesMax());

    printf("rf4ce_Set_TX_Power_Key_Exchange\n");
    getchar();
    //rf4ce_Set_TX_Power_Key_Exchange(10);
    printf("rf4ce_Test_Subscribe_Event\n");
    getchar();
    rf4ce_Test_Subscribe_Event();

    printf("zrc1_Test_Get_Extended_Cap\n");
    getchar();
    RF4CEZRCInputCapsEx cap;
    cap.version = RF4CE_ZRC_INPUT_CAP_V0;
    cap.capabilitySize = sizeof(RF4CEZRCInputCapsV0);
    zrc1_Test_Get_Extended_Cap(&cap);
#endif
    printf("Phy test\n");
    getchar();
    phy_Test_Get_Caps();
    phy_Test_Set_Channel();
    phy_Test_Continuous_Wave_Start();
    phy_Test_Continuous_Wave_Stop();
    phy_Test_Transmit_Start();
    getchar();
    phy_Test_Transmit_Stop();
    getchar();
    printf("Receive start\n");
    phy_Test_Receive_Start();
    getchar();
    printf("Receive stop\n");
    phy_Test_Receive_Stop();
    getchar();
    printf("phy_Test_Echo_Start ... \n");
    phy_Test_Echo_Start();
    getchar();
    printf("phy_Test_Echo_Stop ... \n");
    phy_Test_Echo_Stop();
    getchar();
    printf("phy_Test_Echo_Stop ... \n");
    phy_Test_Energy_Detect_Scan();
    phy_Test_Get_Stats();
    phy_Test_Reset_Stats();
    phy_Test_Set_TX_Power(10);
    phy_Test_Select_Antenna();
    rf4ce_Test_RF4CE_Get_Diag_Caps();
    rf4ce_Test_RF4CE_Get_Diag_Agility();
    rf4ce_Test_RF4CE_Get_Diag_LinkQuality(0);
    rf4ce_Test_RF4CE_Get_Diag_TxPower();
    rf4ce_Test_RF4CE_Get_Diag_TxPower_KeyExchange();
    printf("PHY_TEST_APP:  closing...\n");

    Zigbee_Close();

    return 0;
}
