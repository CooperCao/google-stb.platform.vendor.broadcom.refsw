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
/*****************************************************************************
*
* FILENAME: projects/SoC_mailboxHostSideTest/rf4ce_registration/zigbee_rf4ce_indication.c
*
* DESCRIPTION:
*   The RF4CE registration function used to deliver the indication.
*
*****************************************************************************/
#include "bbSysTimeoutTask.h"
#include "bbRF4CEConfig.h"
#include "bbRF4CENWK.h"
#include "bbRF4CEPM.h"
//#include "bbRF4CEGDPConstants.h"
//#include "bbRF4CEGDPStartReset.h"
//#include "bbRF4CEGDPBind.h"
//#include "bbRF4CEGDPExternalIndications.h"
#include "bbRF4CEZRCControlCommand.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_rf4ce_registration_priv.h"
#include "zigbee_rf4ce_indication.h"
#include "bbMailService.h"
#include "bbMailAPI.h"
#include "bbMailClient.h"
#include "zigbee_api.h"


static int findMatchClientId(RF4CE_RegisterVirtualDeviceParams_t *indParams)
{
    int clientId = -1;
    Bool8_t match = true;
    if(0 == indParams->fieldValidMask & RF4CE_REGISTERFIELDMASKALL)
        return -1;
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_RegistrationInfoQueueHeader());
    while (NULL != current)
    {
        match = true;
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
            break;
        }
        current = SYS_QueueGetNextQueueElement(current);
    }
    return clientId;
}

static int findClientIdWaitForPair()
{
    int clientId = -1;

    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_WaitForPairQueueQueueHeader());
    if (NULL != current){
        RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
        clientId = info->clientId;
    }
    return clientId;
}


static int findClientIdByPairRef(uint8_t pairRef)
{
    RF4CE_RegisterVirtualDeviceParams_t regParams = {0};
    regParams.fieldValidMask = RF4CE_PAIRREF_MASK;
    regParams.pairRef = pairRef;
    return findMatchClientId(&regParams);
}

static int updatePairRefInRegisterInfo(int clientId, uint8_t pairingRef)
{
    SYS_QueueElement_t *current = SYS_QueueGetQueueHead(RF4CE_WaitForPairQueueQueueHeader());
    registerWaitForPairLock();
    SYS_QueueRemoveQueueElement(RF4CE_WaitForPairQueueQueueHeader(), current);
    registerWaitForPairUnlock();
    RF4CE_RegisterVirtualDeviceInternalInfo_t *const info = GET_PARENT_BY_FIELD(RF4CE_RegisterVirtualDeviceInternalInfo_t, elem, current);
    info->params.pairRef = pairingRef;
    info->params.fieldValidMask |= RF4CE_PAIRREF_MASK;
    registrationInfoLock();
    SYS_QueuePutQueueElementToTail(RF4CE_RegistrationInfoQueueHeader(), current);
    registrationInfoUnlock();
    return 0;
}

void RF4CE_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *const indParams)
{
    int clientId = -1;
    if((clientId = findClientIdByPairRef(indParams->pairingRef)) != -1){
        server_RF4CE_ZRC2_ControlCommandInd(indParams, clientId);
    }
}

void RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *const indParams)
{
    int clientId = -1;
    if((clientId = findClientIdByPairRef(indParams->pairingRef)) != -1){
        server_RF4CE_ZRC1_VendorSpecificInd(indParams, clientId);
    }
	return;
#if 0
    void rf4ceGetPairEntryInfoByPairRefCallback(RF4CE_NWK_GetReqDescr_t *req, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status){
            printf("\n\nHere are additional information about NWK\n");
            RF4CE_PairingTableEntry_t  pairEntryInfo = {0};
            memcpy(&pairEntryInfo, &conf->data.nwkPairingTableEntry, sizeof(pairEntryInfo));
            printf("PAN:               0x%04X\n", pairEntryInfo.dstPanId);
            printf("Control IEEE Addr: ");
            uint8_t *ieeeAddr = (uint8_t *)&pairEntryInfo.dstIeeeAddr;
            int i = sizeof(pairEntryInfo.dstIeeeAddr) - 1;
            for(; i > 0; i --)
                printf("%02X-", ieeeAddr[i]);
            printf("%02X\n", ieeeAddr[i]);
            printf("Contorl NWK Addr:  0x%02X\n", pairEntryInfo.dstNetAddr);
            printf("Target  NWK Addr:  0x%02X\n", pairEntryInfo.srcNetAddr);
            printf("Channel:           0x%02X\n", pairEntryInfo.dstLogicalChannel);
        }
    }

    {
        void RF4CE_CC_Data_CB(NVM_WriteFileIndDescr_t *orgInd, NVM_WriteFileRespParams_t *resp) {  return; }
        NVM_WriteFileIndDescr_t writeReportFileToNVM;
        memset(&writeReportFileToNVM, 0, sizeof(NVM_WriteFileIndDescr_t));
        writeReportFileToNVM.callback = RF4CE_CC_Data_CB;
        writeReportFileToNVM.params.fileIndex = 0xCC;
        writeReportFileToNVM.params.address = 0;
        memcpy(&writeReportFileToNVM.params.payload, &indParams->payload, sizeof(indParams->payload));

        /* Call the NVM indication to save the payload to host side */
        NVM_WriteFileInd(&writeReportFileToNVM);


        RF4CE_NWK_GetReqDescr_t  getDescr;
        memset(&getDescr, 0, sizeof(RF4CE_NWK_GetReqDescr_t));
        getDescr.params.attrId.attrId = RF4CE_NWK_PAIRING_TABLE;
        getDescr.params.attrId.attrIndex = indParams->pairingRef;
        getDescr.callback = rf4ceGetPairEntryInfoByPairRefCallback;
        RF4CE_NWK_GetReq_Call(&getDescr);
    }
#endif
}

void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *const indParams)
{
    //printf("In RF4CE_ZRC1_ControlCommandInd %d\n", indParams->pairingRef);
    int clientId = -1;
    if((clientId = findClientIdByPairRef(indParams->pairingRef)) != -1){
        //printf("Send the control command to client\n");
        server_RF4CE_ZRC1_ControlCommandInd(indParams, clientId);
    }
}

void RF4CE_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t  *ind)
{
    //RF4CE_GDP_CheckValidationRespConfParams_t conf;
    int clientId = -1;
    if((clientId = findClientIdByPairRef(ind->pairingRef)) != -1)
        server_RF4CE_ZRC2_CheckValidationInd(ind, clientId);
}


static void rf4ceComposeRegisterInfo(const RF4CE_PairingTableEntry_t *const pairEntryInfo, RF4CE_RegisterVirtualDeviceParams_t *const registerParam)
{
    memcpy(&registerParam->expectedMacAddress, &pairEntryInfo->dstIeeeAddr, sizeof(registerParam->expectedMacAddress));
    registerParam->fieldValidMask = RF4CE_MACADDRESS_MASK;
}
/********************************************************************************
* The MBA should take care of this pair indication to find
********************************************************************************/
void RF4CE_NWK_PairInd(RF4CE_NWK_PairIndParams_t *indication)
{
    printf("In RF4CE_NWK_PairInd \n");
}

#if 0
static void rf4ceGetPairEntryInfoByPairRefCallback(RF4CE_NWK_GetReqDescr_t *req, RF4CE_NWK_GetConfParams_t *conf)
{
    if(0 == conf->status){
        RF4CE_PairingTableEntry_t  pairEntryInfo = {0};
        RF4CE_RegisterVirtualDeviceParams_t registerParam = {0};
        memcpy(&pairEntryInfo, &conf->data.nwkPairingTableEntry, sizeof(pairEntryInfo));
        rf4ceComposeRegisterInfo(&pairEntryInfo, &registerParam);
        int clientId = findMatchClientId(&registerParam);
        if(clientId != -1){
            printf("In RF4CE_PairInd, found a client shows its interest \n");
            updatePairRefInRegisterInfo(clientId, conf->attrId.attrIndex);
        }
    }
}

static uint8_t rf4ceGetPairEntryInfoByPairRef(uint8_t pairingRef)
{
    RF4CE_NWK_GetReqDescr_t  getDescr;
    memset(&getDescr, 0, sizeof(RF4CE_NWK_GetReqDescr_t));
    getDescr.params.attrId.attrId = RF4CE_NWK_PAIRING_TABLE;
    getDescr.params.attrId.attrIndex = pairingRef;
    getDescr.callback = rf4ceGetPairEntryInfoByPairRefCallback;
    RF4CE_NWK_GetReq_Call(&getDescr);
    return 0;
}
#endif

void RF4CE_PairInd(RF4CE_PairingIndParams_t *indication)
{
    //printf("In RF4CE_PairInd \n");
    RF4CE_RegisterVirtualDeviceParams_t  registerParam;
    memset(&registerParam, 0, sizeof(RF4CE_RegisterVirtualDeviceParams_t));
    //rf4ceGetPairEntryInfoByPairRef(indication->pairingRef);
    int clientId = findClientIdWaitForPair();
    if(clientId != -1){
        printf("In RF4CE_PairInd, found a client shows its interest.\n");
        updatePairRefInRegisterInfo(clientId, indication->pairingRef);
        server_RF4CE_PairInd(indication, clientId);
    }
    else
        printf("In RF4CE_PairInd, did not find a client shows its interest yet.\n");
}

void RF4CE_ZRC2_PairNtfyInd(RF4CE_PairingIndParams_t *indication)
{
    RF4CE_RegisterVirtualDeviceParams_t  registerParam;
    memset(&registerParam, 0, sizeof(RF4CE_RegisterVirtualDeviceParams_t));
    //rf4ceGetPairEntryInfoByPairRef(indication->pairingRef);
    int clientId = findClientIdWaitForPair();
    if(clientId != -1){
        printf("In RF4CE_ZRC2_PairNtfyInd, found a client shows its interest.\n");
        updatePairRefInRegisterInfo(clientId, indication->pairingRef);
        server_RF4CE_PairInd(indication, clientId);
    }
    else
        printf("In RF4CE_ZRC2_PairNtfyInd, did not find a client shows its interest yet.\n");

}
