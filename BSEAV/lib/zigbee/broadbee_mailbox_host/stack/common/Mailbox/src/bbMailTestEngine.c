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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailTestEngine.c $
*
* DESCRIPTION:
*         the mailbox client table implementation.
*
* $Revision: 3943 $
* $Date: 2014-10-07 20:55:38Z $
*
*****************************************************************************************/
/************************* INCLUDES *****************************************************/
#include "bbMailAPI.h"


#ifndef _TEST_HARNESS_ /* STACK and HOST */
#ifdef _MAILBOX_WRAPPERS_TEST_ENGINE_

#include "bbHalReset.h"

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)

#define CID_BUFFER_TEST_REQUEST     0x001c
#define CID_BUFFER_TEST_RESPONSE    0x0054

typedef struct
{
    ZBPRO_APS_DataReqDescr_t apsReq;
    bool isBusy;
} teBuffer_t;
static teBuffer_t bufferPool[2];

static void mailResetBufferPool(void)
{
    memset(bufferPool, 0, sizeof(bufferPool));
}

static ZBPRO_APS_DataReqDescr_t *mailGetApsReqMem(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(bufferPool); ++i)
        if (!bufferPool[i].isBusy)
        {
            bufferPool[i].isBusy = true;
            return &bufferPool[i].apsReq;
        }
    return NULL;
}

static void mailFreeApsReqMem(ZBPRO_APS_DataReqDescr_t *req)
{
    teBuffer_t *const buffer = GET_PARENT_BY_FIELD(teBuffer_t, apsReq, req);
    SYS_DbgAssert(buffer >= bufferPool && buffer < bufferPool + ARRAY_SIZE(bufferPool),
        TEST_ENGINE_WRONG_BUFFER_POINTER);
    buffer->isBusy = false;
}

static void mailApsDataConf(ZBPRO_APS_DataReqDescr_t *const reqDescr, ZBPRO_APS_DataConfParams_t *const confParams)
{
    SYS_DbgAssertLog(ZBPRO_APS_SUCCESS_STATUS == confParams->status,
        TEST_ENGINE_UNSUCCESSFUL_AUTO_RESPONSE_TRANSMISSION);

    SYS_FreePayload(&reqDescr->params.payload);
    mailFreeApsReqMem(reqDescr);
}

void Mail_TestEngine_DataInd(ZBPRO_APS_DataIndParams_t *indParams)
{
    do
    {
        if ((ZBPRO_APS_PROFILE_ID_TEST == indParams->profileId || ZBPRO_APS_PROFILE_ID_WILDCARD == indParams->profileId)
            && CID_BUFFER_TEST_REQUEST == indParams->clusterId)
        {
            ZBPRO_APS_DataReqDescr_t *req = mailGetApsReqMem();
            if (NULL == req)
            {
                SYS_DbgLogId(NOT_ENOUGH_STATIC_MEMORY_FOR_AUTO_RESPONSE_ON_TEST_BUFFER_REQUEST);
                break;
            }

            SYS_SetEmptyPayload(&req->params.payload);
            if (!SYS_DuplicatePayload(&req->params.payload, &indParams->payload))
            {
                mailFreeApsReqMem(req);
                SYS_DbgLogId(NOT_ENOUGH_DYNAMIC_MEMORY_FOR_AUTO_RESPONSE_ON_TEST_BUFFER_REQUEST);
                break;
            }

            req->params.dstAddress = indParams->srcAddress;
            req->params.profileId = ZBPRO_APS_PROFILE_ID_TEST;
            req->params.clusterId = CID_BUFFER_TEST_RESPONSE;
            req->params.dstEndpoint = indParams->srcEndpoint;
            req->params.srcEndpoint = indParams->localEndpoint;
            memset(&req->params.txOptions, 0, sizeof(req->params.txOptions));
            req->params.txOptions.security = (ZBPRO_APS_UNSECURED_STATUS != indParams->securityStatus);
            req->params.radius = 0;

            req->callback = mailApsDataConf;
            ZBPRO_APS_DataReq(req);
        }
    } while (0);

    ZBPRO_APS_DataInd(indParams);
}
#endif /* WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_ */

static uint32_t echoDelayTime = 0;
/*************************************************************************************//**
    \brief Ping request implementation.
    \param[in] req - request pointer.
*****************************************************************************************/
void Mail_TestEnginePing(TE_PingCommandReqDescr_t *const req)
{
    SYS_DbgAssertComplex(req->callback, MAILTESTENGINE_TESTENGINEPING_0);

    //SYS_DbgLogStr("Ping received\n");
    {
        TE_PingCommandConfParams_t conf;

        conf.firmareStacks = FIRMWARE_STACKS;
        conf.firmwareMailboxLevels = FIRMWARE_LEVEL;
        conf.firmwareFeatures = FIRMWARE_FEATURES;
        conf.firmwareProfiles = FIRMWARE_PROFILES;
        req->callback(req, &conf);
    }
}

static void SYS_Delay(uint32_t milliSeconds)
{
    #define SYS_FREQ  27000000
    #define SYS_HZ    (SYS_FREQ/1000/2)
    uint32_t timeElapse = SYS_HZ*milliSeconds;
    while(timeElapse--);
}

/*************************************************************************************//**
    \brief Echo request implementation.
    \param[in] req - request pointer.
*****************************************************************************************/
void Mail_TestEngineEcho(TE_EchoCommandReqDescr_t *const req)
{
    SYS_Delay(echoDelayTime);
    TE_EchoCommandConfParams_t conf;
    SYS_SetEmptyPayload(&conf.payload);
    SYS_DataLength_t size = MIN(SYS_GetPayloadSize(&req->params.payload), req->params.echoSize);
    bool allocResult = SYS_MemAlloc(&conf.payload, size);
    SYS_DbgAssertComplex(allocResult, MAILTESTENGINE_TESTENGINEECHO_2);
    SYS_DbgAssertComplex(NULL != req->callback, MAILTESTENGINE_TESTENGINEECHO_0);

    //SYS_DbgLogStr("Echo received(%d)\n", req->params.echoSize);

    SYS_CopyPayloadToPayload(&conf.payload, &req->params.payload, size);
    conf.status = 0x2A;
    req->callback(req, &conf);
}

/*************************************************************************************
    \brief Set the echo delay timer
    \param[in] req - request pointer
*************************************************************************************/
void Mail_SetEchoDelay(TE_SetEchoDelayCommandReqDescr_t *const req)
{
    echoDelayTime = req->params.delayTime;
    TE_SetEchoDelayCommandConfParams_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.delayTime = echoDelayTime;
    conf.status = 1;
    req->callback(req, &conf);
}

/*************************************************************************************//**
    \brief Reset request implementation
    \param[in] req - request pointer.
*****************************************************************************************/
void Mail_TestEngineReset(TE_ResetCommandReqDescr_t *const req)
{
    // NOTE: HAL_Reset doesn't clear hardware registers.
    HAL_RestartSw(req->params.resetType);
    // NOTE: Next code will be called only for platform then reset function isn't supported.
    Mail_TestEngineSendHello(NULL);
}

#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */
/*************************************************************************************//**
    \brief Send greeting.
*****************************************************************************************/
void Mail_TestEngineSendHello(MailDescriptor_t *const mail)
{
    (void)mail;
#if defined(_MAILBOX_WRAPPERS_TEST_ENGINE_) && !defined(MAILBOX_UNIT_TEST)
    SYS_DbgLogStr("send HELLO");
    TE_HelloCommandIndParams_t ind;

    ind.firmareStacks = FIRMWARE_STACKS;
    ind.firmwareMailboxLevels = FIRMWARE_LEVEL;
    ind.firmwareFeatures = FIRMWARE_FEATURES;
    ind.firmwareProfiles = FIRMWARE_PROFILES;
    Mail_TestEngineHelloInd(&ind);
#endif /* MAILBOX_UNIT_TEST */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
    mailResetBufferPool();
#endif
}
#endif /* TEST_HARNESS */