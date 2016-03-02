/********************************************************************************************
*     (c)2004-2016 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *     main DTCP-IP stack , DTCP-IP AKE state machine implmentation.
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake_msg.c
 *  \brief implement base DTCP Ake core functionalities's state machine.
 */
#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"
#include "b_os_lib.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_transport.h"
#include "b_dtcp_ip_transport_priv.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_ip_stack.h"
#include "b_dtcp_status_codes.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_stream.h"

BDBG_MODULE(b_dtcp_ip);

/* NOTE: unsupported IP AKE command is not defined here*/
#define IS_IPAKE_CMD(cmd)   (cmd == B_IpAkeCmd_eRttReady || \
            cmd == B_IpAkeCmd_eRttSetup || \
            cmd == B_IpAkeCmd_eRttTest || \
            cmd == B_IpAkeCmd_eContKeyConf || \
            cmd == B_IpAkeCmd_eRttVerify)
/* Macro to check if RTT max trial has been reached.*/
#define IS_RTTN_MAX(RttN)   ((RttN[0] << 8 | RttN[1]) >= DTCP_RTT_MAX_RETRIES)

/* Macro to convert RttN buffer to integer */
#define RTTN2INT(RttN)      (RttN[0] << 8 | RttN[1])

/* AKE state enum to description string map*/
char *state2str[] = {
    "Idle", "Challenge", "Response", "Response2", "ExchangeKey", "SRM", "Authenticated", "Canceled", "Error",
    "Completed","RttReady", "RttSetup", "RttTest", "RttVerify", "RttCompleted", "ContKeyConfirm", "BackgroundRTT"};

struct __b_dtcp_ckc_data{
    B_DTCP_Stack_T *Stack;
    B_AkeCoreSessionData_T *Session;
};
struct __b_dtcp_ckc_data ckc;

extern bool B_DTCP_IP_CheckOverFlow(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * a, unsigned char * b, int size);
extern bool B_DTCP_IP_AkeTransportSinkStopped(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session);

/*! \brief check if given device is on source device's RTT registry.
 *  \oaran[in] pStack  dtcp stack pointer.
 *  \param[in] DeviceId given device id.
 *  \param[in,out] hMatchedIf not NULL, matched device list element item will be returned, if device is found on registry.
 */
static bool B_DTCP_IP_IsDeviceOnRttRegistry(B_DTCP_Stack_T * pStack,
        const unsigned char DeviceId[DTCP_DEVICE_ID_SIZE], B_RttRegistryData_T ** hMatched)
{
    B_RttRegistryData_T * iter, * elem = NULL;
    bool retValue = false;
    struct RttRegistries * pRttRegList;

    /* sanity check */
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(pStack->pAkeCoreData);
    pRttRegList = pStack->pAkeCoreData->pProtocolData;
    BDBG_ASSERT(pRttRegList);

    iter = BLST_S_FIRST(pRttRegList);
    while(iter != NULL)
    {
        if(!BKNI_Memcmp(iter->DeviceId, DeviceId, DTCP_DEVICE_ID_SIZE))
        {
            elem = iter;
            break;
        }
        iter = BLST_S_NEXT(iter, node);
    }
    if( elem != NULL)
    {
        retValue = true;
        if( hMatched != NULL)
            *hMatched = elem;
    }else {
        retValue = false;
    }

    return retValue;
}
/*! \brief utility function to add device to RTT registry.
 */
static BERR_Code B_DTCP_IP_AddToRttRegistry(B_DTCP_Stack_T * pStack, const unsigned char DeviceId[DTCP_DEVICE_ID_SIZE])
{
    B_RttRegistryData_T * elem;
    struct RttRegistries * pRegList;

    /* Sanity check */
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(pStack->pAkeCoreData);
    pRegList = pStack->pAkeCoreData->pProtocolData;
    BDBG_ASSERT(pRegList);

    if( B_DTCP_IP_IsDeviceOnRttRegistry(pStack, DeviceId, NULL) == true)
        return BERR_DUPLICATED_ENTRIES;
    if((elem = BKNI_Malloc(sizeof(B_RttRegistryData_T))) == NULL)
        return B_ERROR_OUT_OF_MEMORY;
    BKNI_Memcpy(elem->DeviceId, DeviceId, DTCP_DEVICE_ID_SIZE);
    B_Mutex_Lock(pStack->pAkeCoreData->hMutex);
    BLST_S_INSERT_HEAD(pRegList, elem, node);
    B_Mutex_Unlock(pStack->pAkeCoreData->hMutex);

    return BERR_SUCCESS;
}
/*! \brief remove device from Rtt registry list.
 */
static BERR_Code B_DTCP_IP_RemoveFromRttRegistry(B_DTCP_Stack_T * pStack, const unsigned char DeviceId[DTCP_DEVICE_ID_SIZE], unsigned int RefCnt)
{
    B_RttRegistryData_T *elem;
    struct RttRegistries * pRttRegList = NULL;
    BERR_Code retValue = BERR_SUCCESS;

    /* Sanity check */
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(pStack->pAkeCoreData);
    pRttRegList = pStack->pAkeCoreData->pProtocolData;

    B_Mutex_Lock(pStack->pAkeCoreData->hMutex);
    if (RefCnt == 1)
    {
        if(B_DTCP_IP_IsDeviceOnRttRegistry(pStack, DeviceId, &elem) == false)
        {
            retValue = BERR_NOT_FOUND;
            goto ERR_OUT;
        }else {
            BLST_S_REMOVE(pRttRegList, elem, __b_rtt_registry_data, node);
            BKNI_Free(elem);
        }
    }

ERR_OUT:
    B_Mutex_Unlock(pStack->pAkeCoreData->hMutex);
    return retValue;
}
static void B_DTCP_CleanRttRegistry(B_DTCP_Stack_T *pStack)
{
    B_RttRegistryData_T * iter, * elem = NULL;
    struct RttRegistries * pRttRegList;

    /* sanity check */
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(pStack->pAkeCoreData);
    pRttRegList = pStack->pAkeCoreData->pProtocolData;
    BDBG_ASSERT(pRttRegList);

    if (!BLST_S_EMPTY(pRttRegList))
    {
        iter = BLST_S_FIRST(pRttRegList);
        while (iter) {
            elem = iter;
            iter = BLST_S_NEXT(iter, node);
            BDBG_MSG(("Removing RTT registry: 0x%08x\n", elem));
            BLST_S_REMOVE(pRttRegList, elem, __b_rtt_registry_data, node);
            BKNI_Free(elem);
        }

    }
}
/*! \brief utility function to compare MAC value for RTT or content key confirm.
 *
 *  It assumes the size of MacA and MacB is 2*size
 */
static bool B_DTCP_IP_CompareMAC(const unsigned char MACA[], const unsigned char MACB[], int size, bool msb)
{
    bool retValue = false;
    if(msb == true)
    {
        if(!BKNI_Memcmp(&MACA[0], &MACB[0], size))
            retValue = true;
    }else
    {
        if(!BKNI_Memcmp(&MACA[size], &MACB[size], size))
            retValue = true;
    }
    return retValue;
}
/*! \brief send RTT_READY command */
static BERR_Code B_DTCP_IP_SendRttReadyCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code ret = BERR_SUCCESS;
    int Cmd, Response;
    B_IpAkeSessionData_T * pIpSession;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_SendRttReadyCmd);
    if(pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttReady) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return BERR_UNKNOWN;
    }
    ret = B_DTCP_AkeSendCmdWaitForResponse(pStack,
            pSession,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if( ret == BERR_SUCCESS && Response == B_Response_eAccepted)
    {
        pSession->CurrentState = B_AkeState_eRttReady;
        B_Time_Get(&pIpSession->RttReadySentTime);
        BDBG_MSG(("ready = %d\n", B_AkeState_eRttReady));
    }
    BDBG_MSG(("ret=%d Response=%d\n", ret, Response));
    BDBG_LEAVE(B_DTCP_IP_SendRttReadyCmd);
    return ret;
}
static BERR_Code B_DTCP_IP_ProcessRttReadyCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, CmdDep;
    B_IpAkeSessionData_T * pIpSession;
    BERR_Code ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_ProcessttReadyCmd);
    ret = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(ret != BERR_SUCCESS)
    {
        pSession->CurrentState = B_AkeState_eError;
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return ret;
    }else if ((IS_SINK(pStack->DeviceParams->Mode) && pSession->CurrentState != B_AkeState_eResponse) ||
            (IS_SOURCE(pStack->DeviceParams->Mode) && pSession->CurrentState != B_AkeState_eRttReady))
    {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eIncorrectCmdOrder;
        pSession->CurrentState = B_AkeState_eError;
    }else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        B_Time_Get(&pIpSession->RttReadyReceivedTime);
    }
    if((ret = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttReady,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return ret;
    }else
    {
        ret = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
    }
    BDBG_LEAVE(B_DTCP_ProcessRttReadyCmd);

    return ret;
}
/*! \brief Send RTT Setup command, called by source device only.
 */
static BERR_Code B_DTCP_IP_SendRttSetupCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    int Cmd, Response;
    B_IpAkeSessionData_T * pIpSession;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_SendRttSetupCmd);
    /* Compute our mac value */
    retValue = B_DTCP_IP_ComputeRttMac(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pSession->AuthKey,
                pIpSession->RttN,
                pIpSession->RttMac);
    if(retValue != BCRYPT_STATUS_eOK)
        return BERR_CRYPT_FAILED;
    if((retValue = pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttSetup)) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return retValue;
    }
    retValue = B_DTCP_AkeSendCmdWaitForResponse(pStack,
            pSession,
            false,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if( retValue == BERR_SUCCESS && Response == B_Response_eAccepted)
    {
        if(IS_SOURCE(pStack->DeviceParams->Mode))
            pSession->CurrentState = B_AkeState_eRttSetup;
        B_Time_Get(&pIpSession->RttSetupSentTime);
    }

    BDBG_LEAVE(B_DTCP_IP_SendRttSetupCmd);
    return retValue;
}
/*! \brief process RTT SETUP command, called by sink device only.
 */
static BERR_Code B_DTCP_IP_ProcessRttSetupCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, CmdDep;
    B_IpAkeSessionData_T * pIpSession;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_ProcessRttSetupCmd);
    retValue = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(retValue != BERR_SUCCESS)
    {
        pSession->CurrentState = B_AkeState_eError;
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        goto BERR_OUT;
    }else if (pSession->CurrentState != B_AkeState_eRttReady &&
            pSession->CurrentState != B_AkeState_eRttTest)
    {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eIncorrectCmdOrder;
        pSession->CurrentState = B_AkeState_eError;
    }else {
        /* Generate MAC value */
        retValue = B_DTCP_IP_ComputeRttMac(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pSession->AuthKey,
                pIpSession->RttN,
                pIpSession->RttMac);
        BDBG_MSG(("Rtt N = 0x%x\n", RTTN2INT(pIpSession->RttN)));
        if( retValue != BCRYPT_STATUS_eOK)
        {
            Response = B_Response_eRejected;
            BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
            Status = B_AkeStatus_eOther;
        }else {
            Response = B_Response_eAccepted;
            Status = B_AkeStatus_eOK;
            pSession->CurrentState = B_AkeState_eRttSetup;
            B_Time_Get(&pIpSession->RttSetupReceivedTime);
            /* Increment RTT trail counter */
            /* pIpSession->RttN += 1; */
        }
    }
    if((retValue = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttSetup,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        pSession->CurrentState = B_AkeState_eError;
    }else
    {
        retValue = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
    }
BERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_ProcessRttSetupCmd);

    return retValue;
}
/*! \brief send RTT TEST command , called by source device only.
 */
static BERR_Code B_DTCP_IP_SendRttTestCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code ret = BERR_SUCCESS;
    int Cmd, Response, Status;
    B_IpAkeSessionData_T * pIpSession;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_SendRttTestCmd);
    if((ret = pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttTest)) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return ret;
    }
    ret = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
    BKNI_Free(pSession->CmdBuffer);
    pSession->CmdBufferSize = 0;
    B_Time_Get(&pIpSession->RttTestSentTime);
    if(ret == BERR_SUCCESS)
    {
        int cmd_not_used;
        ret = B_DTCP_AkeReceiveCmd(pStack, pSession,  DTCP_AKE_CMD_TIMEOUT, &cmd_not_used);
                if(ret != BERR_SUCCESS)
            return ret;
            B_Time_Get(&pIpSession->RttTestReceivedTime);
                ret = pStack->Messenger->ConsumeResponse_Func(
                    pSession,
                    pSession->CmdBuffer,
                    pSession->CmdBufferSize,
                    &Cmd,
                    &Response,
                    &Status);
                if(ret != BERR_SUCCESS)
                    return ret;
                else if (Response == B_Response_eAccepted) {
                    pIpSession->RttValue = B_Time_Diff(&pIpSession->RttTestReceivedTime, &pIpSession->RttTestSentTime);
                    pSession->CurrentState = B_AkeState_eRttTest;
                    BDBG_MSG(("Trail=%d RttValue=%d\n", pIpSession->RttN, pIpSession->RttValue));
                }

    }
    BDBG_LEAVE(B_DTCP_IP_SendRttTestCmd);
    return ret;
}
/*! \brief process RTT test command, called by sink device only.
 */
static BERR_Code B_DTCP_IP_ProcessRttTestCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, CmdDep;
    B_IpAkeSessionData_T * pIpSession;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_ProcessRttTestCmd);
    retValue = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);
    if(retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return retValue;
    }else if ( pSession->CurrentState != B_AkeState_eRttSetup)
    {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eIncorrectCmdOrder;
    }else
    {
        pSession->CurrentState = B_AkeState_eRttTest;
    }
    retValue = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
            pSession,
            B_IpAkeCmd_eRttTest,
            CmdDep,
            Response,
            Status);

    if( retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return retValue;
    }else {
        retValue = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
    }

    BDBG_LEAVE(B_DTCP_IP_ProcessRttTestCmd);
    return retValue;
}
/*! \brief Send RTT verify command, called by source device only.
 */
static BERR_Code B_DTCP_IP_SendRttVerifyCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    unsigned char RttN[2];
    int Cmd, Response;
    B_IpAkeSessionData_T * pIpSession;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);
    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_SendRttVerifyCmd);

    /* Prepare OKMSG for verification */
    BKNI_Memcpy(RttN, pIpSession->RttN, DTCP_RTT_N_SIZE);
    B_DTCP_IncrementRttN(RttN);
    retValue = B_DTCP_IP_ComputeRttMac(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, pSession->AuthKey, RttN, pIpSession->OkMsg);

    if(retValue != BCRYPT_STATUS_eOK || pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                pSession,
                B_IpAkeCmd_eRttVerify) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        retValue = BERR_UNKNOWN;
        goto ERR_OUT;
    }

    retValue = B_DTCP_AkeSendCmdWaitForResponse(pStack,
            pSession,
            false,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if( retValue == BERR_SUCCESS && Response == B_Response_eAccepted)
    {
        pSession->CurrentState = B_AkeState_eRttVerify;
        B_Time_Get(&pIpSession->RttVerifySentTime);
        /*pIpSession->RttN += 1; */
    }
ERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_SendRttVerifyCmd);
    return retValue;
}
/*! \brief process RTT VERIFY, called by sink device only.
 */
static BERR_Code B_DTCP_IP_ProcessRttVerifyCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, CmdDep;
    B_IpAkeSessionData_T * pIpSession;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);
    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_ProcessRttVerifyCmd);
    retValue = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        goto ERR_OUT;
    }else if ( pSession->CurrentState != B_AkeState_eRttTest)
    {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eIncorrectCmdOrder;
    }else if (B_DTCP_IP_CompareMAC(pIpSession->RttMac, pIpSession->OtherRttMac, DTCP_RTT_MAC_SIZE, true) == false)
    {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eOther;

    }else {
        unsigned char RttN[DTCP_RTT_N_SIZE];
        BKNI_Memcpy(RttN, pIpSession->RttN, DTCP_RTT_N_SIZE);
        B_DTCP_IncrementRttN(RttN);
        /* Generate OKMSG */
        retValue = B_DTCP_IP_ComputeRttMac(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, pSession->AuthKey, RttN, pIpSession->OkMsg);
        if(retValue == BCRYPT_STATUS_eOK)
        {
            Response = B_Response_eAccepted;
            Status = B_AkeStatus_eOK;
        }else
        {
            Response = B_Response_eRejected;
            Status = B_AkeStatus_eOther;
            pSession->CurrentState = B_AkeState_eRttVerify;
        }

    }
    /* If source is to send RttSetup again, N will be incremented.
     */
    B_DTCP_IncrementRttN(pIpSession->RttN);
    retValue = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
            pSession,
            B_IpAkeCmd_eRttVerify,
            CmdDep,
            Response,
            Status);

    if( retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return retValue;
    }else {
        retValue = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
        /*
         * We've finished Rtt test so far on our side, set the state back to "Response" state, expecting
         * source device to send "Exchange Key" command.
         */
        pSession->CurrentState = B_AkeState_eResponse;
    }
ERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_ProcessRttVerifyCmd);
    return retValue;
}
/*! \brief Send Content Key confirmation command, called by sink device only.
 */
static BERR_Code B_DTCP_IP_SendContKeyConfCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, Response;
    B_IpAkeSessionData_T * pIpSession;
    unsigned char NcT[DTCP_CONTENT_KEY_NONCE_SIZE];
    unsigned char NcTR[DTCP_RTT_MK_SIZE];
    unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
    unsigned char exchangeKeyLabel = 0;
    int NcTR_sz = DTCP_CONTENT_KEY_NONCE_SIZE;

    BERR_Code retValue = BERR_SUCCESS;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_SendContKeyConfCmd);
    /* compute MAC3B and MAc4B */
    B_DTCP_GetSetRealTimeNonce(pSession->hMutex, pSession->RealTimeNonce, NcT);

    retValue = B_ModAdd(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, NcTR, NcT, pIpSession->ContR, gBn160, DTCP_CONTENT_KEY_NONCE_SIZE,
            DTCP_CONT_KEY_CONF_R_SIZE, DTCP_RTT_MK_SIZE);
    if (retValue != BCRYPT_STATUS_eOK)
        goto ERR_OUT;
    if ((retValue = B_DTCP_GetExchKeyFromSession(pSession, B_ExchKeyCipher_eAes,
                exchangeKey, &exchangeKeyLabel)) != BERR_SUCCESS)
        goto ERR_OUT;

    if (B_DTCP_IP_CheckOverFlow(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, NcT, pIpSession->ContR,
            DTCP_CONTENT_KEY_NONCE_SIZE) == true) {
        NcTR_sz += 1;
    }
    retValue = B_DTCP_IP_ComputeRttMac_2(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl,
            exchangeKey, NcTR, NcTR_sz, pIpSession->ContKeyConfMac);

    if ( retValue == BCRYPT_STATUS_eOK) {
        if((retValue = pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                    pSession,
                    B_IpAkeCmd_eContKeyConf)) != BERR_SUCCESS) {
            BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            pStack->OnInternalError(pStack, pSession);
        }else {
            retValue = B_DTCP_AkeSendCmdWaitForResponse(pStack, pSession, false, DTCP_AKE_CMD_TIMEOUT, &Cmd, &Response);
            if( Response == B_Response_eAccepted) {
                pSession->CurrentState = B_AkeState_eContKeyConf;
                retValue = BERR_SUCCESS;
            }
        }
    }
    /* Increment Cont conf R value for next trial*/
    B_DTCP_IncrementNonce(pSession->hMutex, pIpSession->ContR);
ERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_SendContKeyConfCmd);
    return retValue;
}
/*! \brief process content key confirmation command, called by source device only
 */
static BERR_Code B_DTCP_IP_ProcessContKeyConfCmd(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    int Cmd, CmdDep;
    B_IpAkeSessionData_T * pIpSession;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_IP_ProcessContKeyConfCmd);
    retValue = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);
    if (retValue != BERR_SUCCESS) {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        goto ERR_OUT;
    } else if ( pSession->CurrentState != B_AkeState_eCompleted) {
        Response = B_Response_eRejected;
        BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
        Status = B_AkeStatus_eIncorrectCmdOrder;
    } else {
        unsigned char Nc[DTCP_CONTENT_KEY_NONCE_SIZE];
        unsigned char NcT[DTCP_CONTENT_KEY_NONCE_SIZE];
        unsigned char Nc_tmp[DTCP_CONTENT_KEY_NONCE_SIZE];
        unsigned char NcT_tmp[DTCP_CONTENT_KEY_NONCE_SIZE];
        unsigned char NcTR[DTCP_RTT_MK_SIZE];
        unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
        unsigned char exchangeKeyLabel = 0;
        /* Check received NcT against source's current Nc */
        B_DTCP_GetSetRealTimeNonce(pStack->pAkeCoreData->hMutex, pStack->pAkeCoreData->RealTimeNonce, Nc);
        B_DTCP_GetSetRealTimeNonce(pSession->hMutex, pIpSession->NcT, NcT);
        BKNI_Memcpy(Nc_tmp, Nc, DTCP_CONTENT_KEY_NONCE_SIZE);
        BKNI_Memcpy(NcT_tmp, NcT, DTCP_CONTENT_KEY_NONCE_SIZE);
        if (pSession->pcp_ur_cap == 1) {
            BKNI_Memset(Nc_tmp, 0, 2);
            BKNI_Memset(NcT_tmp, 0, 2);
        }
        if (B_DTCP_GetNonceDiff(Nc_tmp, NcT_tmp) < 5) {
            int NcTR_sz = DTCP_CONTENT_KEY_NONCE_SIZE;
            /* Compute our MACA(3/4) value */
#if 0
            if (pSession->pcp_ur_cap == 1) {
                retValue = B_ModAdd(NcTR, &NcT[2], pIpSession->ContR, gBn160, DTCP_CONTENT_KEY_NONCE_SIZE -2,
                    DTCP_CONT_KEY_CONF_R_SIZE, DTCP_RTT_MK_SIZE);
            } else {
                retValue = B_ModAdd(NcTR, NcT, pIpSession->ContR, gBn160, DTCP_CONTENT_KEY_NONCE_SIZE,
                    DTCP_CONT_KEY_CONF_R_SIZE, DTCP_RTT_MK_SIZE);
            }
#endif
            retValue = B_ModAdd(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, NcTR, NcT, pIpSession->ContR, gBn160, DTCP_CONTENT_KEY_NONCE_SIZE,
                DTCP_CONT_KEY_CONF_R_SIZE, DTCP_RTT_MK_SIZE);
            if (retValue != BCRYPT_STATUS_eOK)
                goto ERR_OUT;
            if ((retValue = B_DTCP_GetExchKeyFromCore(pSession->pAkeCoreData, B_ExchKeyCipher_eAes,
                    exchangeKey, &exchangeKeyLabel)) != BERR_SUCCESS)
                goto ERR_OUT;

            if (B_DTCP_IP_CheckOverFlow(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, NcT, pIpSession->ContR,
                    DTCP_CONTENT_KEY_NONCE_SIZE) == true) {
                    NcTR_sz += 1;
            }
            retValue = B_DTCP_IP_ComputeRttMac_2(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, exchangeKey,
                    NcTR, NcTR_sz, pIpSession->ContKeyConfMac);
            if(retValue != BCRYPT_STATUS_eOK)  {
                BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
                pStack->OnInternalError(pStack, pSession);
                retValue = BERR_CRYPT_FAILED;
                goto ERR_OUT;
            }
            /* Compare Mac3A and Mac3B */
            if (!BKNI_Memcmp(pIpSession->ContKeyConfMac,
                        pIpSession->OtherContKeyConfMac,
                        DTCP_CONT_KEY_CONF_MAC_SIZE)) {
                Response = B_Response_eAccepted;
                Status = B_AkeStatus_eOK;
            } else {
                Response = B_Response_eRejected;
                BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
                Status = B_AkeStatus_eOther;
            }
        } else {
            Response = B_Response_eRejected;
            BDBG_ERR(("%s %d : rejected", __FUNCTION__, __LINE__));
            Status = B_AkeStatus_eOther;
        }
    }
    retValue = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
            pSession,
            B_IpAkeCmd_eContKeyConf,
            CmdDep,
            Response,
            Status);

    if( retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
    }else {
        retValue = pStack->Transporter->Send(pStack->Transporter, pSession,
                pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
    }
ERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_ProcessContKeyConfCmd);
    return retValue;
}
/*! \brief process receied DTCP-IP Ake commands.
 */
static BERR_Code B_DTCP_IP_ProcessIpAkeCommands(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession, int Cmd)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_IpAkeSessionData_T * pIpSession = NULL;
    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_MSG(("Processing command code: 0x%x\n", Cmd));

    /* This function only handles IP specific commands */
    switch(Cmd)
    {
        case B_IpAkeCmd_eRttReady:
            /* Both source and sink device can receive this command */
            retValue = B_DTCP_IP_ProcessRttReadyCmd(pStack, pSession);
            if(retValue == BERR_SUCCESS && pSession->CurrentState != B_AkeState_eError)
            {
                if(IS_SINK(pStack->DeviceParams->Mode))
                    retValue = B_DTCP_IP_SendRttReadyCmd(pStack, pSession);
                else if( !IS_RTTN_MAX(pIpSession->RttN))
                    retValue = B_DTCP_IP_SendRttSetupCmd(pStack, pSession);
                else{
                    BDBG_ERR(("Rtt trail counter = %d, exceeded max value\n", RTTN2INT(pIpSession->RttN)));
                    retValue = BERR_RTT_FAILED;
                }
            }
            break;
        case B_IpAkeCmd_eRttSetup:
            /* Only sink device can receive this command */
            retValue = B_DTCP_IP_ProcessRttSetupCmd(pStack, pSession);
            break;
        case B_IpAkeCmd_eRttTest:
            /* Only sink device can receive this command */
            retValue = B_DTCP_IP_ProcessRttTestCmd(pStack, pSession);
            break;
        case B_IpAkeCmd_eRttVerify:
            /* Only sink device can receive this command */
            retValue = B_DTCP_IP_ProcessRttVerifyCmd(pStack, pSession);
            break;
        case B_IpAkeCmd_eContKeyConf:
            /* Only source device can receive this command */
            retValue = B_DTCP_IP_ProcessContKeyConfCmd(pStack, pSession);
            break;
        case B_IpAkeCmd_eBgRttInitiate:
        default:
            retValue = BERR_NOT_SUPPORTED;
    }
    return retValue;

}
/*! \brief perform Rtt, called by RTT-AKE procedure or Background RTT process.
 *
 *  Source device call this function for RTT-AKE after challenge-response procedure is done.
 *  or to initiate background RTT procedure.
 *  Sink device can only call this function to initiate background RTT procedure.
 *
 */
BERR_Code B_DTCP_IP_DoRtt(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_IpAkeSessionData_T * pIpSession = NULL;
    int Cmd;

    /* sanity check */
    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);
    BDBG_ENTER(B_DTCP_IP_DoRtt);

    if( B_DTCP_IP_IsDeviceOnRttRegistry(pStack, pSession->OtherDeviceId, NULL) == true)
    {
        BDBG_MSG(("Device is on Rtt Registry!\n"));
        return BERR_SUCCESS;
    }

    pIpSession->RttN[0] = 0;
    pIpSession->RttN[1] = 0;
    pIpSession->RttValue = 0xFFFF;      /* Rtt unknown yet */

    retValue = B_DTCP_IP_SendRttReadyCmd(pStack, pSession);

    if( retValue == BERR_SUCCESS && pSession->CurrentState == B_AkeState_eRttReady)
    {
        /* Expecting RttReady command from sink device */
        retValue = B_DTCP_AkeReceiveCmd(pStack, pSession, 1000, &Cmd);

        if(retValue == BERR_SUCCESS) {
            retValue = B_DTCP_IP_ProcessIpAkeCommands(pStack, pSession, Cmd);
        }
        BDBG_MSG(("retVal=%d RttN=%02x%02x, RttValue=%d\n", retValue, pIpSession->RttN[0], pIpSession->RttN[1], pIpSession->RttValue));
        if (retValue != BERR_SUCCESS) {
            retValue = BERR_RTT_FAILED;
            pSession->CurrentState = B_AkeState_eError;
            BDBG_ERR(("DoRTT: process cmd %x failed\n", Cmd));
            goto ERR_OUT;
        }
        while(retValue == BERR_SUCCESS &&
                !IS_RTTN_MAX(pIpSession->RttN) &&
                pIpSession->RttValue > DTCP_RTT_MAX_RTT_MS)
        {
            BDBG_MSG(("DoRTT: CurrentState = %d\n", pSession->CurrentState));
            if(retValue == BERR_SUCCESS && pSession->CurrentState == B_AkeState_eRttReady)
            {
                retValue = B_DTCP_IP_SendRttSetupCmd(pStack, pSession);
            }
            if(retValue == BERR_SUCCESS && pSession->CurrentState == B_AkeState_eRttSetup)
            {
                retValue = B_DTCP_IP_SendRttTestCmd(pStack, pSession);
            }
            /*
             * RttTest command measured the Rtt value and stored it in session's RttValue
             */
            if(retValue == BERR_SUCCESS && pIpSession->RttValue <= DTCP_RTT_MAX_RTT_MS)
            {
                /* Verify Mac2 values */
                if( true == B_DTCP_IP_CompareMAC(pIpSession->RttMac, pIpSession->OtherRttMac,
                            DTCP_RTT_MAC_SIZE, false))
                {
                    retValue = B_DTCP_IP_SendRttVerifyCmd(pStack, pSession);

                }else
                {
                    /* Abort if Mac doesn't match */
                    retValue = BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
                    pSession->CurrentState = B_AkeState_eError;
                    BDBG_ERR(("DoRTT: Mac2 verification failure\n"));
                    goto ERR_OUT;
                }
            }else if (retValue == BERR_SUCCESS && pIpSession->RttValue > DTCP_RTT_MAX_RTT_MS)
            {
                /* Repeat RTT setup and RTT test procedure.*/
                pSession->CurrentState = B_AkeState_eRttReady;
            }
            if(retValue == BERR_SUCCESS && pSession->CurrentState == B_AkeState_eRttVerify)
            {
                /* Verify OKMSG */
                if(true == B_DTCP_IP_CompareMAC(pIpSession->OkMsg, pIpSession->OtherOkMsg,
                            DTCP_RTT_MAC_SIZE, true))
                {
                    pSession->CurrentState = B_AkeState_eRttCompleted;
                    retValue = B_DTCP_IP_AddToRttRegistry(pStack, pSession->OtherDeviceId);
                    break;
                }else
                {
                    /* Abort if OKMSG doesn't match */
                    BDBG_ERR(("DoRTT: OKMSG verification failure\n"));
#ifdef DTCP_DEMO_MODE
                    BDBG_MSG(("Our OKMSG\n"));
                    BDBG_BUFF(pIpSession->OkMsg, DTCP_RTT_MAC_DATA_SIZE);
                    BDBG_MSG(("Other OKMSG\n"));
                    BDBG_BUFF(pIpSession->OtherOkMsg, DTCP_RTT_MAC_DATA_SIZE);
#endif
                    retValue = BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
                    pSession->CurrentState = B_AkeState_eError; /* TODO: Set to Idle state? */
                    goto ERR_OUT;
                }

            }
            BDBG_MSG(("DoRTT: currentState = %d RttN = %d\n", pSession->CurrentState, RTTN2INT(pIpSession->RttN)));
            B_DTCP_IncrementRttN(pIpSession->RttN);
        }
    }

ERR_OUT:
    BDBG_LEAVE(B_DTCP_IP_DoRtt);
    return retValue;
}
/*! \brief start Content Key confirmation procedure. Initiate by sink device only.
 */
static BERR_Code B_DTCP_IP_DoContKeyConf(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_IpAkeSessionData_T * pIpSession = NULL;

    /* sanity check */
    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pStack);

    pIpSession = (B_IpAkeSessionData_T*)pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    if (pSession->CurrentState != B_AkeState_eCompleted) {
        BDBG_ERR(("CKC: Device State %s: not authenticated\n",
                    (pSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pSession->CurrentState] ));
        retValue = BERR_CONT_KEY_CONF_FAILED;
    } else
        retValue = B_DTCP_IP_SendContKeyConfCmd(pStack, pSession);
    /*
     * If our(sink) current state == eContKeyconf, source device confirmed content key
     * we need to verify MAC4A == MAC4B on our(sink) side.
     */
#ifdef DTCP_DEMO_MODE
    BDBG_WRN(("ContConfMac :\n"));
    BDBG_BUFF(pIpSession->ContKeyConfMac, SHA1_DIGEST_SIZE);
    BDBG_WRN(("\n"));
    BDBG_WRN(("OtherContConfMac :\n"));
    BDBG_BUFF(pIpSession->OtherContKeyConfMac, SHA1_DIGEST_SIZE);
#endif
    if (retValue == BERR_SUCCESS && pSession->CurrentState == B_AkeState_eContKeyConf) {
        pSession->CurrentState = B_AkeState_eCompleted;
        /* verify MAC4 value */
        if(!BKNI_Memcmp(&pIpSession->ContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_SIZE],
                    &pIpSession->OtherContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_SIZE],
                    DTCP_CONT_KEY_CONF_MAC_SIZE)) {
            pIpSession->ContKeyConfirmed = true;
        } else {
            pIpSession->ContKeyConfirmed = false;
            BDBG_ERR(("DoContKeyConf: MAC4A !=MAC4B"));

        }
    } else {
        pIpSession->ContKeyConfirmed = false;
        BDBG_ERR(("DoContKeyConf: state error!!!"));
    }
    return retValue;

}
/*! \brife content key confirmation thread function.
 *  \param[in] user content key confirmation data.
 */
void B_DTCP_IP_CKCThreadFunc(void *user)
{
    BERR_Code retValue;
    struct __b_dtcp_ckc_data * ckc = (struct __b_dtcp_ckc_data *)user;
    B_IpAkeSessionData_T * IpAkeSession = (B_IpAkeSessionData_T *)ckc->Session->pProtocolData;

    while(IpAkeSession->TerminateCKCThread == false &&
            IpAkeSession->ContKeyConfCount < 50)
    {
        if (IpAkeSession->StartContKeyConf == true) {
            retValue = B_DTCP_IP_DoContKeyConf(ckc->Stack, ckc->Session);
            IpAkeSession->ContKeyConfCount++;
            if (retValue != BERR_SUCCESS) {
                BDBG_ERR(("CKC failed %d\n", retValue));
                break;
            }
            IpAkeSession->StartContKeyConf = false;
        } else {
            B_Thread_Sleep(5000);
            continue;
        }
    }
    BDBG_MSG(("CKC thread terminated\n"));

}

/*! \brief callback function for source device when an authentication request is received from sink.
 *  \param[in] UserData implmentation defined user data, a pointer to __dtcp_ip_stack_data
 */
BERR_Code B_DTCP_IP_OnNewConnection(void * ctx, void * UserData)
{
    B_AkeType_T AkeType;
    int Cmd, Timeout;
    unsigned char devid[DTCP_DEVICE_ID_SIZE];
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeCoreSessionData_T * pAkeSession = NULL;
    B_AkeCoreSessionData_T * pOldAkeSession = NULL;
    B_IpAkeSessionData_T * pIpAkeSession = NULL;
    B_IpAkeSessionData_T * pIpOldAkeSession = NULL;
    struct __dtcp_ip_stack_data * sdata = (struct __dtcp_ip_stack_data *)UserData;
    B_DTCP_Stack_T * pStack = (B_DTCP_Stack_T *)ctx;

    BDBG_ASSERT(sdata);
    BDBG_ASSERT(pStack);

    BDBG_ENTER(B_DTCP_IP_OnNewConnection);
    retValue = B_DTCP_GetAkeTypeFromCertificate(&AkeType, pStack->DeviceParams);

    BDBG_MSG(("Check if Session already exist for ip <%s>", sdata->RemoteIp));
    B_DTCP_IP_GetAkeHandle(pStack, sdata->RemoteIp, (void **)&pAkeSession, true);
    if (pAkeSession == NULL) {
        /* Create IP AKE session */
        BDBG_MSG(("############## Creating AKE session for: %s #############\n", sdata->RemoteIp));
        if(( retValue = B_DTCP_CreateIpAkeSession( pStack->pAkeCoreData,
                    sdata->RemoteIp,
                    sdata->LocalPort,
                    false,
                    AkeType,
                    pStack->DeviceParams->Mode,
                    &pAkeSession)) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to create IP AKE session: %d\n", retValue));
            return retValue;
        }
        B_Mutex_Lock(pAkeSession->hMutexStateMachine);
        pAkeSession->CurrentState = B_AkeState_eIdle;
    }
    else
    {
        B_Mutex_Lock(pAkeSession->hMutexStateMachine);
        BDBG_MSG(("############## AKE session already available for: <%s> Current State <%s>#############\n", sdata->RemoteIp, (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState]));
        /* reset AKE state so AKE negotiation could begin if the previous
         * state was one of the listed */
        switch (pAkeSession->CurrentState)
        {
            case B_AkeState_eCanceled:
            case B_AkeState_eError:
                pAkeSession->CurrentState = B_AkeState_eIdle;
                break;
            default:
                break;
        }
        /*Make this false so that when same device is doing AKE/verifyKey again and state is B_AkeState_Completed
          we make it go to the begining of AKE state machine*/
        pAkeSession->Authenticated = false;
    }

    pIpAkeSession = (B_IpAkeSessionData_T *)pAkeSession->pProtocolData;

    /* Use connect socket fd as session id. */
    BDBG_MSG(("Old Session ID <%d> SocketID <%d>", pAkeSession->SessionId, sdata->ConnectSocket));
    pAkeSession->SessionId = sdata->ConnectSocket;
    BDBG_MSG(("New Session ID <%d> SocketID <%d>", pAkeSession->SessionId, sdata->ConnectSocket));

    /*
     * This while loop implements the AKE state machine.
     */
    while( retValue == BERR_SUCCESS && pAkeSession->CurrentState != B_AkeState_eCanceled
            && pAkeSession->CurrentState != B_AkeState_eError)
    {
        BDBG_MSG(("Start of AKE state machine for Client IP<%s> Curent State <%s>", pIpAkeSession->RemoteIp, (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState] ));
        if(pAkeSession->CurrentState == B_AkeState_eCompleted && pAkeSession->Authenticated == true)
        {
            /*There is no need to hold on to the connection thread once AKE has been finished. Breaking out*/
            BDBG_ERR(("Device Authorized. State is <%s>. Breaking out of the AKE process", (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState] ));
            retValue = BERR_SOCKET_CLOSE;
            break;
        }
        else
            Timeout = DTCP_AKE_CMD_TIMEOUT;

        retValue = B_DTCP_AkeReceiveCmd(pStack, pAkeSession, Timeout, &Cmd);

        BDBG_MSG(("Received AKE command CurrentState: %s\n",
                    (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState] ));

        if (retValue == BERR_SUCCESS) {
            if(IS_IPAKE_CMD(Cmd))
                retValue = B_DTCP_IP_ProcessIpAkeCommands(pStack, pAkeSession, Cmd);
            else
                retValue = B_DTCP_ProcessAkeCommands(pStack, pAkeSession, Cmd);
        } else if(retValue == BERR_TIMEOUT && pAkeSession->CurrentState == B_AkeState_eCompleted) {
            /*
             * This is the case that AKE has completed, we start waiting for other commands,
             * such as content key request, or content key confirm, etc. So we need to go back
             * to receive command if a timeout error happened.
             */
            retValue = BERR_SUCCESS;
        }

        /*
           This is to check if the new Client trying to connect is not already in our Device ID list. If so Source is going to cancel the AKE
           for this client.
        */
        if (pAkeSession->RefCnt == 1 && pAkeSession->CurrentState != B_AkeState_eCompleted)
        {
            BDBG_MSG(("This is a check to verify if the same DEVICE is initialting the AKE or is it a new device Current State is %s ", (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState]));
            if(B_DTCP_IsDeviceAuthenticated(pStack->pAkeCoreData, pAkeSession->OtherDeviceId) == true)
            {
                BDBG_MSG(("This is a check to verify if the same DEVICE is initialting the AKE or is it a new device. This device is already in the Authenticated device list "));
                /*
                  This is to handle the case when for some reason same device tries to connect again with different
                  IP address. The old entry will be removed and new request will be processed
                */
                if (B_DTCP_GetSessionEntryForDeviceId(pStack->pAkeCoreData, pAkeSession->OtherDeviceId, &pOldAkeSession) == true)
                {
                    pIpOldAkeSession = (B_IpAkeSessionData_T *)pOldAkeSession->pProtocolData;
                    BDBG_MSG(("If Old Entry IP address <%s> is different from the new connection IP address <%s> then destroy old entry from Authorized session list", pIpOldAkeSession->RemoteIp, pIpAkeSession->RemoteIp));
                    if (strcmp(pIpAkeSession->RemoteIp, pIpOldAkeSession->RemoteIp))
                    {
                        BDBG_MSG(("Entry Already exist for this device, Removing old entry ......"));
                        /* Close Socket for the previous entry found to that old entries Connection thread start exit process cleanly */
                        B_DTCP_RemoveSessionFromList(pOldAkeSession);
                        retValue = B_DTCP_IP_RemoveFromRttRegistry( pStack, pOldAkeSession->OtherDeviceId, pOldAkeSession->RefCnt);
                        BDBG_MSG(("Destroying entry ......"));
                        B_DTCP_DestroyIpAkeSession(pOldAkeSession);
                    }
                }
            }
        }

        if (pAkeSession->CurrentState == B_AkeState_eSrm)
        {
            Timeout = 1000;
            retValue = B_DTCP_AkeReceiveCmd(pStack, pAkeSession, Timeout, &Cmd);
            if(retValue == BERR_SUCCESS) {
                BDBG_MSG(("Received cmd %d after AKE done\n", Cmd));
                retValue = B_DTCP_ProcessAkeCommands(pStack, pAkeSession, Cmd);
                if (retValue == BERR_SUCCESS)
                {
                    BDBG_MSG(("SRM updated successfully!!"));
                }
            }
            else if(retValue == BERR_TIMEOUT) {
                BDBG_MSG(("Waiting for SRM timedout\n"));
            }
        }

        if(retValue == BERR_SUCCESS && pAkeSession->CurrentState == B_AkeState_eCompleted) {
            /*
             * Add Authenticated AKE session to session list, if device is authenticated.
             */
            BKNI_Memcpy(devid, pAkeSession->OtherDeviceId, DTCP_DEVICE_ID_SIZE);
            BDBG_MSG(("Authenticated DeviceID: %02x %02x %02x %02x %02x\n", devid[0], devid[1],devid[2], devid[3], devid[4]));
#ifdef DTCP_DEMO_MODE
            BDBG_MSG(("Authentication Key:\n"));
            BDBG_BUFF(pAkeSession->AuthKey, DTCP_AUTH_KEY_SIZE);
#endif
            if (pAkeSession->RefCnt == 1)
            {
                    B_DTCP_AddSessionToList(pAkeSession);
            }
            pAkeSession->Authenticated = true;
        }

    }
    BDBG_MSG(("############# Source AKE finished : retval=%d State: %s #############\n",
                retValue, (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState] ));

    /*
     * We reached here if:
     * 1. any error occurred when processing AKE commands.
     * 2. Connection was closed by either source or sink.
     */
    if (retValue != BERR_SUCCESS && pAkeSession->CurrentState != B_AkeState_eCompleted)
    {
        BERR_Code retValue2 = BERR_SUCCESS;
        if (B_DTCP_IsDeviceAuthenticated(pStack->pAkeCoreData, pAkeSession->OtherDeviceId)
                == true)
        {
            B_DTCP_RemoveSessionFromList(pAkeSession);
        }

        retValue2 = B_DTCP_IP_RemoveFromRttRegistry( pStack, pAkeSession->OtherDeviceId, pAkeSession->RefCnt);
        if (retValue2 != BERR_SUCCESS)
        {
            BDBG_WRN(("Unable to remove from RTT Registry, retValue2 = %d", retValue2));
        }

        B_Mutex_Unlock(pAkeSession->hMutexStateMachine); /*Unlock it before destroy else there is a chance mutex has been destroyed*/
        B_DTCP_DestroyIpAkeSession(pAkeSession);
        return retValue;
    }

    B_Mutex_Unlock(pAkeSession->hMutexStateMachine);

    BDBG_LEAVE(B_DTCP_IP_OnNewConnection);
    return retValue;
}

/* Stream Close calls this to decrease the Ref count and clean up the session list */
BERR_Code B_DTCP_IP_SourceSessionClean(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pAkeSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(pAkeSession);

    BDBG_MSG((" CLEANING SESSION LIST DUE TO STREAM CLOSE FROM APP "));
    if (B_DTCP_IsDeviceAuthenticated(pStack->pAkeCoreData, pAkeSession->OtherDeviceId) == true)
    {
        BDBG_MSG(("DEVICE FOUND REMOVE FROM LIST DUE TO STREAM CLOSE"));
        B_DTCP_RemoveSessionFromList(pAkeSession);
    }
    retValue = B_DTCP_IP_RemoveFromRttRegistry( pStack, pAkeSession->OtherDeviceId, pAkeSession->RefCnt);
    B_DTCP_DestroyIpAkeSession(pAkeSession);

    return retValue;
}

/*! \brief start source function, called by AppLib
 *  param[in] pData IP specific data.
 */
BERR_Code B_DTCP_IP_AkeSourceStart(B_DTCP_StackHandle_T pStack, void * pData)
{
    BERR_Code retValue = BERR_SUCCESS;

    struct __dtcp_ip_stack_data * sdata = (struct __dtcp_ip_stack_data *)pData;

#ifdef DTCP_DEMO_MODE
    /*BDBG_SetLevel(BDBG_eTrace); */
#endif
    BDBG_ASSERT(pData);
    BDBG_ASSERT(pStack);
    BDBG_ENTER(B_DTCP_IP_AkeSourceStart);
    /*
     * Start transport layer source function, it will listen to remote
     * connection request.
     */
    retValue = pStack->Transporter->StartSource((void*)pStack, pStack->Transporter, sdata, B_DTCP_IP_OnNewConnection);

    BDBG_LEAVE(B_DTCP_IP_AkeSourceStart);
    return retValue;
}
/*! \brief stop source function, called by Applib.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_IP_AkeSourceStop(B_DTCP_StackHandle_T pStack)
{
    BERR_Code retValue = BERR_SUCCESS;

    BDBG_ASSERT(pStack);
    BDBG_ENTER(B_DTCP_IP_AkeSourceStop);

    /*
     * Stop transport layer source function, this will close listenning socket
     * and thus close all accepted connections.
     */
    retValue = pStack->Transporter->StopSource(pStack->Transporter);
    BDBG_MSG(("Transport layer source function stoped with retValue: %d\n", retValue));

    /* clean all AKE sessions*/
    B_Mutex_Lock(pStack->pAkeCoreData->hMutex);
    B_DTCP_CleanAkeSessionList(pStack->pAkeCoreData, true);
    B_Mutex_Unlock(pStack->pAkeCoreData->hMutex);

    B_DTCP_CleanRttRegistry(pStack);

    BDBG_LEAVE(B_DTCP_IP_AkeSourceStop);
#ifdef DTCP_DEMO_MODE
    /* BDBG_SetLevel(BDBG_eErr); */
#endif
    return retValue;
}
/*! \brief start sink function, called by AppLib
 * param[in] pData procotol specific data pointer.
 * param[in,out] akeHandle pointer to AKe session data handle.
 */
BERR_Code B_DTCP_IP_AkeSinkStart(B_DTCP_StackHandle_T pStack, void * pData, void ** akeHandle)
{
    B_AkeCoreSessionData_T * pAkeSession = NULL;
    B_IpAkeSessionData_T * pIpAkeSession = NULL;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeType_T AkeType;
    int Cmd;
    struct __dtcp_ip_stack_data *pStackData = (struct __dtcp_ip_stack_data *)pData;

#ifdef DTCP_DEMO_MODE
    /* BDBG_SetLevel(BDBG_eTrace);  */
#endif
    BDBG_ENTER(B_DTCP_IP_AkeSinkStart);
    BDBG_ASSERT(pData);
    BDBG_ASSERT(akeHandle);
    BDBG_ASSERT(pStack);

    if( *akeHandle == NULL)
    {
        retValue = B_DTCP_GetAkeTypeFromCertificate(&AkeType, pStack->DeviceParams);

        /* Create IP AKE session */
        retValue = B_DTCP_CreateIpAkeSession( pStack->pAkeCoreData,
                    pStackData->RemoteIp, pStackData->RemotePort,
                    false, AkeType, pStack->DeviceParams->Mode, &pAkeSession);

        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to create IP AKE session: %d\n", retValue));
            return retValue;
        }
        pIpAkeSession = (B_IpAkeSessionData_T *)pAkeSession->pProtocolData;
        /* Making connection to the source device. */
        retValue = pStack->Transporter->StartSink(pStack->Transporter, pAkeSession, NULL);

        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("Transport interface failure: %d \n", retValue));
            B_DTCP_DestroyIpAkeSession(pAkeSession);
            return retValue;
        }
        pAkeSession->CurrentState = B_AkeState_eIdle;
    }else
    {
        pAkeSession = (B_AkeCoreSessionData_T *)*akeHandle;
        pIpAkeSession = (B_IpAkeSessionData_T *)pAkeSession->pProtocolData;
    }

    BDBG_MSG(("####################### DTCP-IP stack: starting sink AKE ######################\n"));

    if (pStack->DeviceParams->pcp_ur_capability == 1) {
        retValue = B_DTCP_SendCapabilityExchangeCmd(pStack, pAkeSession);
        if (retValue != BERR_SUCCESS) {
            BDBG_ERR(("Returned %d for capability exchange command\n",retValue));
            B_DTCP_DestroyIpAkeSession(pAkeSession);
            return retValue;
        }
    }

    retValue = B_DTCP_SendChallengeCmd(pStack, pAkeSession);

    while( retValue == BERR_SUCCESS
            && pAkeSession->CurrentState != B_AkeState_eCanceled
            && pAkeSession->CurrentState != B_AkeState_eError
            && pAkeSession->CurrentState != B_AkeState_eCompleted )
    {
        if (pAkeSession->CurrentState == B_AkeState_eSrm)
            retValue = B_DTCP_AkeReceiveCmd(pStack, pAkeSession, 1000, &Cmd);
        else
        retValue = B_DTCP_AkeReceiveCmd(pStack, pAkeSession, DTCP_AKE_CMD_TIMEOUT, &Cmd);
        BDBG_MSG(("CurrentState: %s :Cmd=%d\n",
                    (pAkeSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pAkeSession->CurrentState], Cmd ));
        if (retValue == BERR_SUCCESS) {
            if(IS_IPAKE_CMD(Cmd))
                retValue = B_DTCP_IP_ProcessIpAkeCommands(pStack, pAkeSession, Cmd);
            else
                retValue = B_DTCP_ProcessAkeCommands(pStack, pAkeSession, Cmd);
        } else {
            /*TODO: Cancel AKE */
        }

    }
    #if 0
    /* Give 1 second period for receiving SRM , if other device send us SRM message. */
    if(retValue == BERR_SUCCESS && pAkeSession->CurrentState == B_AkeState_eCompleted
        && pAkeSession->OtherSrmUpdateRequired == false && pAkeSession->OtherSrmReplaceRequired == false)
    {
        retValue = B_DTCP_AkeReceiveCmd(pStack, pAkeSession, 1000, &Cmd);
        if(retValue == BERR_SUCCESS) {
            BDBG_MSG(("Received cmd %d after AKE done\n", Cmd));
            retValue = B_DTCP_ProcessAkeCommands(pStack, pAkeSession, Cmd);
        }else if(retValue == BERR_TIMEOUT) {
            BDBG_MSG(("Waiting for SRM timedout\n"));
            retValue = BERR_SUCCESS;
        }else if(retValue == BERR_SOCKET_CLOSE) {
            BDBG_MSG(("Socket closed after AKE\n"));
            retValue = BERR_SUCCESS;
        }
    }
    #endif
    BDBG_ERR(("####################### DTCP-IP stack: AKE Finished ######################\n"));

#ifdef BDBG_DEBUG_BUILD
    if(getenv("CLIENT_AKE_SOCKET_CLOSE_AFTER_AKE"))
    {
        /*Only for testing close AKE socket right after AKE on the client side */
        BDBG_ERR(("DTCP-IP stack: AKE Finished  session id %d Closing the socket ######################\n", pAkeSession->SessionId));
        B_SocketShutdown(pAkeSession->SessionId);
        B_SocketClose(pAkeSession->SessionId);
    }
    #endif

    if(retValue == BERR_SUCCESS && pAkeSession->CurrentState == B_AkeState_eCompleted)
    {
        *akeHandle = (void*)pAkeSession;
#ifdef DTCP_DEMO_MODE
        BDBG_MSG(("AKE succeeded: Auth Key:\n"));
        BDBG_BUFF(pAkeSession->AuthKey, DTCP_AUTH_KEY_SIZE);
#endif
        /* Start exchange key timer */
        retValue = B_DTCP_StartExchKeyTimer(pAkeSession, pAkeSession->pAkeCoreData, B_DeviceMode_eSink);
        if (pStack->pAkeCoreData->ckc_check == true)
        {
            if (B_DTCP_IsALRequired(pStack->DeviceParams, pAkeSession) == true) {
                ckc.Stack = pStack;
                ckc.Session = pAkeSession;
                pIpAkeSession->hContKeyConfThread = B_Thread_Create("CKC",
                        B_DTCP_IP_CKCThreadFunc, (void*)&ckc, NULL);
                if(pIpAkeSession->hContKeyConfThread == NULL) {
                    BDBG_ERR(("Failed to launch CKC thread\n"));
                    B_DTCP_DestroyIpAkeSession(pAkeSession);
                    *akeHandle = NULL;
                    retValue = BERR_UNKNOWN;
                }
            } else if (pIpAkeSession->StartContKeyConf == true) {
                pIpAkeSession->StartContKeyConf = false;
            }
        }

    }else {
        BDBG_ERR(("AKE Failed: %d\n", retValue));
        if(pStack->Transporter->StopSink(pStack->Transporter, pAkeSession, NULL) != BERR_SUCCESS)
        {
            BDBG_ERR(("!!!!!!!!!!CLIENT AKE FAILED, UNABLE TO CLOSE CONNECTION. SOME RESOURCE MAY LEAK!!!!!!!!!!"));
        }
        else
        {
            BDBG_ERR(("AKE FAILED. CLEANING UP CONNECTION !!!!!!!"));
        }
        B_DTCP_DestroyIpAkeSession(pAkeSession);
        *akeHandle = NULL;
    }
#ifdef DTCP_DEMO_MODE
    /* BDBG_SetLevel(BDBG_eErr); */
#endif

    BDBG_LEAVE(B_DTCP_IP_AkeSinkStart);
    return retValue;
}
/*! \brief Stop sink function.
 *  \param[in] AkeHandle AKe session obtained from Sink start function.
 */
BERR_Code B_DTCP_IP_AkeSinkStop(B_DTCP_StackHandle_T pStack , B_AkeHandle_T  AkeHandle)
{
    BERR_Code retValue = BERR_SUCCESS;

    BDBG_ASSERT(pStack);
    BDBG_ENTER(B_DTCP_IP_AkeSinkStop);

    if(AkeHandle == NULL)
    {
        /* This is the case when SinkStart() failed and AKE session
         * is not obtained.
         */
        return BERR_SUCCESS;
    }

    /*
     * Stop the transport layer sink function, for DTCP/IP, this is to
     * close connection.
     */
    retValue = pStack->Transporter->StopSink(pStack->Transporter, AkeHandle, NULL);
    if(AkeHandle != NULL)
    {
        /* Destroy AKE session*/
        B_DTCP_DestroyIpAkeSession(AkeHandle);
    }

    BDBG_LEAVE(B_DTCP_IP_AkeSinkStop);
    return retValue;
}
/*! \brief internal error handling
 */
void B_DTCP_IP_OnInternalError(B_DTCP_Stack_T * pStack, B_AkeHandle_T AkeHandle)
{
    /* TODO:*/
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)AkeHandle;
    (void)pStack;
    BDBG_ERR(("Internal Error: CurrentState: %s\n",
                (pSession->CurrentState >= B_AkeState_eMax)? "Unknown":state2str[pSession->CurrentState] ));
    (void)pSession; /* Supress warning when B_REFSW_DEBUG=n */
}
/*! \brief exported function to verify current exchange key is still valid, for sink device only.
 *  \param[in] AkeHandle current AKE handle.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_IP_VerifyExchKey(B_DTCP_StackHandle_T pStack, B_AkeHandle_T AkeHandle, bool * Valid)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeCoreSessionData_T * pSession =(B_AkeCoreSessionData_T *) AkeHandle;
    /*struct __b_dtcp_stream_data  * stream = NULL; */
    int Cmd, Response;

    BDBG_ASSERT(pStack);
    BDBG_ENTER(B_DTCP_IP_VerifyExchKey);

    if(NULL == pSession )
    {
        /* This is the case when SinkStart() failed and AKE session
         * is not obtained.
         */
        *Valid = false;
        return BERR_SUCCESS;
    }
    /*
     * First, check if we expired our exchange key by ourself.
     */
    if(B_DTCP_IsExchKeyValid((void*)pSession, B_ExchKeyCipher_eAes) == false)
    {
        *Valid = false;
        return BERR_SUCCESS;
    }else {

        /*
         * Server might close connection after AKE success.
         */
        /*TODO: this shouldn't be needed, so look into removing Start/Stop sink at some point */
        /* we should be able to handle the case where server closes a socket by re-opening it */
        /* Note: we are storing the original socket value and openning a new session to do the Verify Exch Key. */
        /* This is to handle the case where server may have closed the socket (session associated w/ original AKE) */
        /* thus we will always use a new session for verify exch key purposes */
        BDBG_MSG(("VerifyExchKey ... SessionId %d", pSession->SessionId));
        pSession->origSessionId = pSession->SessionId;
        if ((retValue = pStack->Transporter->StartSink(
                pStack->Transporter, pSession, NULL)) != BERR_SUCCESS) {
            BDBG_ERR(("Transport layer failure: retValue = %d\n", retValue));
            *Valid = false;
            return retValue;
        }

        /*
         * Next, send CONT_KEY_REQ command to source, verify source
         * device didn't expire it's exchange key.
         */
        if((retValue = pStack->Messenger->CreateCommand_Func(pStack->DeviceParams,
                pSession,
                B_AkeCmd_eContentKeyReq)) != BERR_SUCCESS)
        {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            pStack->OnInternalError(pStack, pSession);
            *Valid = false;
        }else {

            retValue = B_DTCP_AkeSendCmdWaitForResponse(pStack,
                pSession,
                false,
                DTCP_AKE_CMD_TIMEOUT,
                &Cmd,
                &Response);

            if( retValue == BERR_SUCCESS && Response == B_Response_eAccepted)
            {
                BDBG_MSG(("Session Label=%d source label=%d\n", pSession->ExchKeyData.Label, pSession->SourceExchKeyLabel));
                if(pSession->ExchKeyData.Label == pSession->SourceExchKeyLabel)
                    *Valid = true;
                else
                {
                    *Valid = false;
                    /*
                     * Exchange key is not valid anymore, source device must have cleared
                     * all authenticated device entries, so we set our AKE state to idle.
                     */
                    pSession->CurrentState = B_AkeState_eIdle;
                }
            }else {
                BDBG_ERR(("Unable to verify content key, response=%d\n", Response));
                *Valid = false;
            }
        }
#if 0
        /* Close packet handle for all streams, if exchange key is invalid */
        if(*Valid == false)
        {
            stream = BLST_S_FIRST(&(pSession->Stream_list));
            while(stream != NULL)
            {
                B_DTCP_Stream_ClosePacketHandle(stream);
                stream = BLST_S_NEXT(stream, node);
            }
        }
#endif

        if ((retValue = pStack->Transporter->StopSink(
                pStack->Transporter, pSession, NULL)) != BERR_SUCCESS) {
            BDBG_WRN(("Transport layer failure: retValue = %d\n", retValue));
/* probably not an error
*           *Valid = false;
*           return retValue;
*/
        }
        pSession->SessionId = pSession->origSessionId;
        BDBG_MSG(("VerifyExchKey Exit ... SessionId %d", pSession->SessionId));

    }

    BDBG_LEAVE(B_DTCP_IP_VerifyExchKey);

    return retValue;
}
/*! \brief exported function to get an updated exchange key, re-authenticating with server, for sink device only.
 *  \param[in] AkeHandle current AKE handle.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_IP_GetNewExchKey(B_DTCP_StackHandle_T pStack, B_AkeHandle_T AkeHandle)
{
    struct __dtcp_ip_stack_data sdata;
    BERR_Code retValue = BERR_SUCCESS;
    B_AkeHandle_T ph = AkeHandle;

    BDBG_ASSERT(pStack);
    BDBG_ENTER(B_DTCP_IP_GetNewExchKey);

    if(AkeHandle == NULL)
        retValue = BERR_INVALID_PARAMETER;
    else {
        ph = AkeHandle;
        retValue = B_DTCP_IP_AkeSinkStart(pStack, &sdata, &ph);
    }
    BDBG_LEAVE(B_DTCP_IP_GetNewExchKey);

    return retValue;
}

/*! \brief initialize DTCP-IP stack object.
 *  \param[in] DeviceParams pointer to device parameter data.
 *  \param[in] UpdateSrm_Func  pointer to a function to update SRM.
 */
B_DTCP_StackHandle_T B_DTCP_IP_Stack_Init(B_DeviceParams_T * DeviceParams, B_DTCP_UpdateSRM_Ptr UpdateSrm_Func)
{
    B_AkeCoreData_T * pCore = NULL;
    B_DTCP_StackHandle_T pStack = NULL;

    BDBG_ENTER(B_DTCP_IP_Stack_Init);
    /* Initialize AKE core data */
    if((pCore = B_DTCP_IpAkeCoreInit( DeviceParams)) == NULL)
    {
        BDBG_ERR(("Failed to init IP AKE core data\n"));
        goto err1;
    }
    if((pStack = BKNI_Malloc(sizeof(B_DTCP_Stack_T))) == NULL)
    {
        BDBG_ERR(("Malloc failed for stack context\n"));
        goto err2;
    }
    BKNI_Memset(pStack, 0, sizeof(B_DTCP_Stack_T));

    if((pStack->Transporter = B_DTCP_InitTransportInterface(B_StackId_eIP)) == NULL)
    {
        BDBG_ERR(("Failed to init transport interface!\n"));
        goto err4;
    }
    if((pStack->Messenger = B_DTCP_InitMessageInterface(B_StackId_eIP)) == NULL)
    {
        BDBG_ERR(("Failed to init transport interface!\n"));
        goto err5;
    }
    pStack->pAkeCoreData = pCore;
    pStack->DeviceParams = DeviceParams;
    pStack->UpdateSrm_Func = UpdateSrm_Func;
    pStack->StartSink_Func = &B_DTCP_IP_AkeSinkStart;
    pStack->StopSink_Func = &B_DTCP_IP_AkeSinkStop;
    pStack->StartSource_Func = &B_DTCP_IP_AkeSourceStart;
    pStack->StopSource_Func = &B_DTCP_IP_AkeSourceStop;
    pStack->OnInternalError = &B_DTCP_IP_OnInternalError,
    pStack->VerifyExchKey_Func = &B_DTCP_IP_VerifyExchKey,
    pStack->StackId = B_StackId_eIP;

    BDBG_LEAVE(B_DTCP_IP_Stack_Init);
    return pStack;

err5:
    B_DTCP_UnInitTransportInterface(B_StackId_eIP, pStack->Transporter);
err4:
    BKNI_Free(pStack);
err2:
    B_DTCP_IpAkeCore_UnInit(pCore);
err1:
    BDBG_LEAVE(B_DTCP_IP_Stack_Init);
    return NULL;
}

/*! \brief cleanup DTCP-IP stack object.
 *  \param[in] pStack the stack handle.
 */
void B_DTCP_IP_Stack_UnInit(B_DTCP_StackHandle_T pStack)
{
    B_DTCP_IpAkeCore_UnInit(pStack->pAkeCoreData);
    B_DTCP_UnInitTransportInterface(B_StackId_eIP, pStack->Transporter);
    B_DTCP_UnInitMessageInterface(B_StackId_eIP, pStack->Messenger);
    BKNI_Free(pStack);
}
/*! \brief exported function to retrieve AKE session handle from authenticated session list.
 *  \param[in] RemoteIp sink device's IP address.
 *  \param[in,out] AkeHandle pointer to AKE session data.
 *  \retval none.
 */
void B_DTCP_IP_GetAkeHandle(B_DTCP_StackHandle_T pStack, const char * RemoteIp, void ** AkeHandle, bool NewConnection)
{
    B_AkeCoreSessionData_T * iter = NULL;
    B_IpAkeSessionData_T * pIpSession = NULL;
    int listCount = 0;

    *AkeHandle = NULL;
    B_Mutex_Lock(pStack->pAkeCoreData->hMutex);
    iter = BLST_S_FIRST(&(pStack->pAkeCoreData->AkeSession_list));
    BDBG_MSG(("Session List :"));
    while(iter != NULL) {
        BDBG_MSG(("List Not NULL"));
        listCount++;
        pIpSession = (B_IpAkeSessionData_T *)(iter->pProtocolData);
        BDBG_MSG(("Session List Node %d: ip<%s> RefCnt<%d>", listCount, pIpSession->RemoteIp, iter->RefCnt ));
        if(!strcmp(pIpSession->RemoteIp, RemoteIp))
        {
            *AkeHandle = (void*)iter;
            /* Only incerasse ref count when New Connection request comes from same client */
            if (NewConnection == true) {
                iter->RefCnt++;
            }
            break;
        }
        iter = BLST_S_NEXT(iter, node);
    }

    if (*AkeHandle != NULL)
    {
        BDBG_MSG(("Entry found for IP <%s>", RemoteIp ));
    }

    B_Mutex_Unlock(pStack->pAkeCoreData->hMutex);
}
