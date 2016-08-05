/********************************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
 * Module Description:
 *   DTCP stack , DTCP AKE state machine implmentation.
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake_msg.c
 *  \brief implement base DTCP Ake core functionalities's state machine.
 */

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_status_codes.h"
#include "b_dtcp_transport.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_srm.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_ip_stack.h"
#include "b_dtcp_ip_ake_msg.h"
#include <arpa/inet.h>

BDBG_MODULE(b_dtcp_ip);

extern char *state2str[];
/*! \brief Initialize DTCP stack.
 *  \param[in] DeviceParams device parameters.
 *  \param[in] UpdateSrm_Func function pointer to update system renewability message.
 *  \param[in] StackId which protocol specific stack to initialize, currently only IP is supported.
 *  \retval BERR_SUCCESS or other error codes.
 */
B_DTCP_StackHandle_T B_DTCP_Stack_Init(B_DeviceParams_T * DeviceParams, B_DTCP_UpdateSRM_Ptr UpdateSrm_Func, B_StackId_T StackId)
{
    B_Os_Init();
    if( StackId == B_StackId_eIP)
    {
        B_DTCP_StackHandle_T handle;
        handle = B_DTCP_IP_Stack_Init(DeviceParams, UpdateSrm_Func);
        if (!handle) {
            B_Os_Uninit();
        }
        return handle;
    }else {
        BDBG_ERR(("Stack ID : %d is not supported\n", StackId));
        return NULL;
    }
}
/*! \brief Cleanup the stack , free allocated resources.
 *  \param[in] pStack the DTCP stack handle .
 */
void B_DTCP_Stack_UnInit(B_DTCP_StackHandle_T pStack)
{
    B_DTCP_IP_Stack_UnInit(pStack);
    B_Os_Uninit();
}
/*! \brief wrapper function, send AKE command and wait for response.
 *
 *  NOTE: The returned response doesn't have data field in it.
 *
 *  \param[in] Stack DTCP stack pointer.
 *  \param[in] Session current DTCP AKE session data pointer.
 *  \param[in] timeout value for waiting.
 *  \param[in, out] command returned command code in the response message.
 *  \param[out] response code, if response returned from other device.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_AkeSendCmdWaitForResponse(B_DTCP_Stack_T * pStack,
        B_AkeCoreSessionData_T * Session,
        bool NoResponseData,
        int TimeOut,
        int * Cmd,
        int * Response)
{
    BERR_Code ret = BERR_SUCCESS;
    int Status;
    char MsgBuffer[1024];

    BSTD_UNUSED(NoResponseData);
    BDBG_ASSERT(pStack);
    BDBG_ASSERT(Session->CmdBuffer);

    BDBG_MSG(("Sending AKE command:\n"));
    B_DumpHeader(Session->CmdBuffer, MsgBuffer, 1024);
    BDBG_MSG(("%s\n", MsgBuffer));

    ret = pStack->Transporter->Send(pStack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);

    if(ret != BERR_SUCCESS)
    {
        BKNI_Free(Session->CmdBuffer);
        return ret;
    }
    /*
     * Free command buffer after it's sent out.
     */
    BKNI_Free(Session->CmdBuffer);
    Session->CmdBuffer = NULL;
    Session->CmdBufferSize = 0;
   /*
    * This will block for specified Timeout millisecond, untill transport
    * layer reader thread set the event.
    */
    {
        int cmd_not_used;
        ret = B_DTCP_AkeReceiveCmd(pStack, Session, TimeOut, &cmd_not_used);
            if(ret != BERR_SUCCESS)
                return ret;

            ret = pStack->Messenger->ConsumeResponse_Func(
                    Session,
                    Session->CmdBuffer,
                    Session->CmdBufferSize,
                    Cmd,
                    Response,
          &Status );
            if(ret != BERR_SUCCESS)
                return ret;
        }
    return ret;
}
/*! \brief Wrapper function to wait and receive command from transport layer.
 */
BERR_Code B_DTCP_AkeReceiveCmd(B_DTCP_Stack_T * pStack,
        B_AkeCoreSessionData_T * Session, int TimeOut, int * Cmd)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_TransportEvent_T event;
    char MsgBuffer[1024];

    BDBG_ASSERT(pStack);
    BDBG_ASSERT(Session);

    BDBG_ENTER(B_DTCP_AkeReceiveCmd);
    event = pStack->Transporter->WaitForEvent(pStack->Transporter, Session, TimeOut);

    if(event == B_Transport_eTimeOut)
    {
        retValue = BERR_TIMEOUT;
        BDBG_MSG(("Timedout while waiting for AKE response: Timeout=%d\n", TimeOut));
    }else if (event == B_Transport_eDataReady)
    {
        /* read msg header */
        B_IpAkeHeader_T header;
        uint8_t * ptr;
        int bytes_needed = 0;
        int bytes_read = 0;
        int bytes_to_read = 0;
        uint16_t msg_len = 0;
        int header_size = (int)sizeof(header);

        ptr = (uint8_t *)&header;
        BKNI_Memset(&header, 0, header_size);
        bytes_needed = header_size;
        bytes_read = 0;
        while (bytes_read < header_size && (event == B_Transport_eDataReady) ) {
            bytes_to_read = bytes_needed - bytes_read;
            retValue = pStack->Transporter->Recv(pStack->Transporter, Session, &ptr[bytes_read], &bytes_to_read);
            if(retValue != BERR_SUCCESS || bytes_to_read < 0)
            {
                BDBG_ERR(("%s: ret %d, bytes_to_read %d", __FUNCTION__, retValue, bytes_to_read));
                return retValue;
            }
            bytes_read += bytes_to_read;
            if (bytes_read >= header_size)
                break;
            else  {
              event = pStack->Transporter->WaitForEvent(pStack->Transporter, Session, TimeOut);
            }
        }
        if (bytes_read < header_size) {
            BDBG_ERR(("AKE msg header incomplete, read %d bytes, expect %d bytes", bytes_read, header_size));
            return BERR_INVALID_PACKET_HEADER;
        }
        msg_len = htons((uint16_t)header.Length) + 3;
        if (msg_len < header_size) {
            BDBG_ERR(("Invalid msg len %d", msg_len));
            return BERR_INVALID_PACKET_HEADER;
        }
        Session->CmdBufferSize = msg_len;
        if((Session->CmdBuffer = BKNI_Malloc(Session->CmdBufferSize)) == NULL)
        {
            retValue = B_ERROR_OUT_OF_MEMORY;
        }else {
            BKNI_Memcpy(Session->CmdBuffer, ptr, sizeof(header));

            bytes_read = header_size;

            while (bytes_read < Session->CmdBufferSize && (event == B_Transport_eDataReady) ) {
                event = pStack->Transporter->WaitForEvent(pStack->Transporter, Session, TimeOut);

                bytes_to_read = Session->CmdBufferSize - bytes_read;
                retValue = pStack->Transporter->Recv(pStack->Transporter, Session, &Session->CmdBuffer[bytes_read] , &bytes_to_read);
                if(retValue != BERR_SUCCESS || bytes_to_read < 0)
                {
                    BDBG_ERR(("%s: ret %d, bytes_to_read %d", __FUNCTION__, retValue, bytes_to_read));
                return retValue;
            }
                bytes_read += bytes_to_read;
                if (bytes_read >= Session->CmdBufferSize) {
                    BDBG_MSG(("%s: len %d, ctype/resp %02x, cmd 0x%02x, label %02x", __FUNCTION__, msg_len, Session->CmdBuffer[3], Session->CmdBuffer[5], Session->CmdBuffer[9]));
                    break;
                }

            }
            if (bytes_read < Session->CmdBufferSize ) {
                BDBG_ERR(("AKE message incomplete read %d bytes, expect %zu bytes", bytes_read, sizeof(header)));
                BKNI_Free(Session->CmdBuffer);
                Session->CmdBuffer = NULL;
                Session->CmdBufferSize = 0;
                return BERR_INVALID_BUFFER_LENGTH;
            }
            pStack->Messenger->hops->GetCommand(Session->CmdBuffer, Cmd);
            BDBG_MSG(("Recevied AKE command:\n"));
            B_DumpHeader(Session->CmdBuffer, MsgBuffer, 1024);
            BDBG_MSG(("%s\n", MsgBuffer));
        }
    }else if(event == B_Transport_eError){
        BDBG_ERR(("Transport error occurred while receiving AKE command\n"));
        retValue = BERR_SOCKET_ERROR;
    }else if(event == B_Transport_eClosed){
        BDBG_WRN(("Connection closed by remote host\n"));
        retValue = BERR_SOCKET_CLOSE;
    }
    BDBG_LEAVE(B_DTCP_AkeReceiveCmd);
    return retValue;

}
/* Process recevied unknwon command, send a NOT_IMPLEMENTED response */
static BERR_Code B_DTCP_ProcessUnKnownCmd(B_DTCP_Stack_T *pStack, B_AkeCoreSessionData_T *pSession)
{
    int Cmd, CmdDep;
    BERR_Code ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eNotImplemented;
    B_AkeStatus_T Status = B_AkeStatus_eOther;

    ret = pStack->Messenger->ConsumeCommand_Func(pStack->DeviceParams,
        pSession,
        pSession->CmdBuffer,
        pSession->CmdBufferSize,
        &Cmd,
        &CmdDep);
    if ( ret == BERR_NOT_SUPPORTED)
    {
        /* Device is sending unknown command ! */
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
    }

    if((ret = pStack->Messenger->CreateResponse_Func(pStack->DeviceParams,
                pSession,
                Cmd,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        pStack->OnInternalError(pStack, pSession);
        return ret;
    }else
    {
        ret = pStack->Transporter->Send(pStack->Transporter, pSession, pSession->CmdBuffer, pSession->CmdBufferSize);
        BKNI_Free(pSession->CmdBuffer);
        pSession->CmdBuffer = NULL;
        pSession->CmdBufferSize = 0;
    }
    return ret;
}
/*! \brief utility function to check source device's sink limit.
 *  \param[in] Session AKE core session data.
 *  \param[in] AP The Sink device's AP flag.
 *  \ret 1 if sink limit is reached, 0 otherwise.
 */
static int B_DTCP_SourceCheckSinkLimit(B_AkeCoreSessionData_T * Session,
        int AP)
{
    B_DTCP_AuthDeviceId_T * did;
    if (DTCP_SINK_COUNT_LIMIT > Session->pAkeCoreData->AuthDeviceCount)
        return 0;
    /*
     * Refer to Figure 45 of the spec for detail.
     * if sink is registered and AP flag is 0, even sink count limit has been
     * reached, we can still perform AKE. Otherwise reject the AKE.
     */
    if (AP == 0)
    {
        did = BLST_S_FIRST(&Session->pAkeCoreData->AuthDevice_list);
        while(did != NULL)
        {
            if(!BKNI_Memcmp(Session->OtherDeviceId, did->DeviceId, DTCP_DEVICE_ID_SIZE))
            {
                return 0;
            }
            did = BLST_S_NEXT(did, node);
        }
    }
    return 1;
}
/*! \brief send a challenge command.
 *  param[in] Stack DTCP stack pointer.
 *  param[in] Session AKE core session data.
 */
BERR_Code B_DTCP_SendChallengeCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, Response;
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT(Stack);
    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_SendChallengeCmd);

    /*
     * For source device, this function must be called when session's current state is State_eChallenge.
     * For sink device, this function must be called when session's current state is State_eIdle.
     */
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eChallenge)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack, Session);
        return ret;
    }

    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if ( ret != BERR_SUCCESS)
    {
        if(ret == BERR_TIMEOUT)
            return ret;
        else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }else {
        if(Cmd != B_AkeCmd_eChallenge) {
            BDBG_ERR(("Unexpetected reponse received %d\n", Cmd));
            return BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
        }
        Session->CurrentState = B_AkeState_eChallenge;
        B_Time_Get(&Session->ChallengeSentTime);
    }

    BDBG_LEAVE(B_DTCP_SendChallengeCmd);

    return ret;
}
/*! \brief process challenge command
 *
 *  \param[in] Session current AKE session pointer.
 *  \param[in] Stack DTCP stack pointer.
 */
static BERR_Code B_DTCP_ProcessChallengeCmd(B_DTCP_Stack_T * Stack,
        B_AkeCoreSessionData_T * Session)
{
    int Cmd, CmdDep;
    BERR_Code ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ENTER(B_DTCP_ProcessChallengeCmd);
    BDBG_ASSERT(Session);

    if((IS_SOURCE(Session->DeviceMode) && Session->CurrentState == B_AkeState_eCompleted)) {
        /* Client redo AKE, reset state to idle */
        Session->CurrentState = B_AkeState_eIdle;
    }
    /*
     * Consume the command first, so the buffer will be freed.
     */
    ret = Stack->Messenger->ConsumeCommand_Func(Stack->DeviceParams,
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize,
        &Cmd,
        &CmdDep);
    if ( ret != BERR_SUCCESS)
    {
        if ( (int)ret == BERR_CRYPT_FAILED)
        {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }else if ((int)ret == BERR_INVALID_CERTIFICATE)
        {
            Response = B_Response_eRejected;
            Status = B_AkeStatus_eAuthFailed;
        }
    }else if((IS_SOURCE(Session->DeviceMode) && Session->CurrentState != B_AkeState_eIdle) ||
            (IS_SINK(Session->DeviceMode) && Session->CurrentState != B_AkeState_eChallenge))
    {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
        Session->CurrentState = B_AkeState_eError;
    }else if(IS_SOURCE(Session->DeviceMode) && B_DTCP_SourceCheckSinkLimit(Session, Session->OtherAP) )
    {
        /* Sink limit reached! */
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
        Session->CurrentState = B_AkeState_eError;
    }else if(B_DTCP_IsDeviceRevoked(Stack->DeviceParams->Srm, Session->OtherDeviceId))
    {
        /* Device has been revoked! */
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
        Session->CurrentState = B_AkeState_eError;
        BDBG_ERR(("%s %d : revoked device %02x-%02x-%02x-%02x-%02x ", __FUNCTION__, __LINE__,
                  Session->OtherDeviceId[0], Session->OtherDeviceId[1],
                  Session->OtherDeviceId[2], Session->OtherDeviceId[3],
                  Session->OtherDeviceId[4]));
    }else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        Session->CurrentState = B_AkeState_eChallenge;
        B_Time_Get(&Session->ChallengeReceivedTime);

    }
    /*
     * Create and send the response.
     */
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eChallenge,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }
    BDBG_LEAVE(B_DTCP_ProcessChallengeCmd);

    return ret;

}
/*! \brief process capability exchange command.
 *
 *  \param[in] Session current AKE session pointer.
 *  \param[in] Stack DTCP stack pointer.
 */
static BERR_Code B_DTCP_ProcessCapabilityExchangeCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, CmdDep;
    BERR_Code ret;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;
    BDBG_ENTER(B_DTCP_ProcessCapabilityExchangeCmd);
    BDBG_ASSERT(Session);

    if((IS_SOURCE(Session->DeviceMode) && Session->CurrentState == B_AkeState_eCompleted)) {
        /* Client redo AKE, reset state to idle */
        Session->CurrentState = B_AkeState_eIdle;
    }
    ret = Stack->Messenger->ConsumeCommand_Func(Stack->DeviceParams,
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if (ret != BERR_SUCCESS) {
        if (ret == BERR_NOT_SUPPORTED) {
            Response = B_Response_eNotImplemented;
            Status = B_AkeStatus_eOther;
        }else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    } else if (Session->CurrentState != B_AkeState_eIdle) {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
    }
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eCapabilityExchange,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }
    BDBG_LEAVE(B_DTCP_ProcessCapabilityExchangeCmd);

    return ret;
}
/*! \brief send capability exchange command.
 *
 *  \param[in] Session current AKE session pointer.
 *  \param[in] Stack DTCP stack pointer.
 */
BERR_Code B_DTCP_SendCapabilityExchangeCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, Response;
    BERR_Code ret;
    BDBG_ASSERT(Session);

    BDBG_ENTER(B_DTCP_SendCapabilityExchangeCmd);
    if (Session->CurrentState != B_AkeState_eIdle)
        return BERR_DTCP_INVALID_COMMAND_SEQUENCE;

    if ((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eCapabilityExchange)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack, Session);
        return ret;
    }
    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            false,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if ( ret != BERR_SUCCESS)
    {
        if(ret == BERR_TIMEOUT)
            return ret;
        else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }else if(Cmd != B_AkeCmd_eCapabilityExchange) {
            BDBG_ERR(("Unexpetected reponse received %d\n", Cmd));
            ret = BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
    }
    BDBG_LEAVE(B_DTCP_SendCapabilityExchangeCmd);
    return ret;
}
/*! \brief Create and Send response subfunction (challenge-response state).
 *
 *  This function is used to populate the EC-DH first phase value/secret in the
 *  session data, not to be confused with the CreateResposne() function implemented
 *  in the b_dtcp_ake_msg.c file, which is used to marshel the session data into
 *  buffer.
 *
 *  \param[in] Session AKE core session data.
 *  \param[in] DTCP stack pointer.
 */
BERR_Code B_DTCP_SendResponseCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)

{
    int Cmd, Response;
    int ret = BERR_SUCCESS;
    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendResponseCmd);

    if (B_DTCP_GetFirstPhaseValue(Stack->pAkeCoreData->pDeviceParams->hDtcpIpTl, Session->FirstPhaseValue,
                Session->FirstPhaseSecret
                ) != BCRYPT_STATUS_eOK)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return BERR_CRYPT_FAILED;
    }
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eResponse)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }

    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if ( ret != BERR_SUCCESS)
    {
        if(ret == BERR_TIMEOUT)
            return ret;
        else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }else {
        if(Cmd != B_AkeCmd_eResponse) {
            BDBG_ERR(("Unexpetected reponse received %d\n", Cmd));
            return BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
        }
        if(Response == B_Response_eAccepted)
        {
            Session->CurrentState = B_AkeState_eResponse;
            B_Time_Get(&Session->ResponseSentTime);
        }
    }

    BDBG_LEAVE(B_DTCP_SendResponseCmd);
    return ret;

}
/*! \brief Process received response and response2 subfunction (challeng-response state)
 *  \param[in] Session Ake core session data.
 *  \param[in] Stack DTCP stack pointer.
 */
BERR_Code B_DTCP_ProcessResponseCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int ret = BERR_SUCCESS;
    int Cmd, CmdDep;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ENTER(B_DTCP_ProcessResponse);
    BDBG_ASSERT(Session);

    Stack->Messenger->hops->GetCommand(Session->CmdBuffer, &Cmd);
    ret = Stack->Messenger->ConsumeCommand_Func(Stack->DeviceParams,
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(ret != BERR_SUCCESS)
    {
        if(ret == BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID)
        {
            Response = B_Response_eRejected;
            Status = B_AkeStatus_eAuthFailed;
            Session->CurrentState = B_AkeState_eError;
        }else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }else if ((IS_SINK(Stack->DeviceParams->Mode) && Session->CurrentState != B_AkeState_eChallenge) ||
            (IS_SOURCE(Stack->DeviceParams->Mode) && Session->CurrentState != B_AkeState_eResponse) )
    {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
        Session->CurrentState = B_AkeState_eError;
    }
    else if (IS_SOURCE(Stack->DeviceParams->Mode) && Cmd == B_AkeCmd_eResponse2
            && !(Stack->DeviceParams->capability & DTCP_CAPABILITY_CIH_MASK))
    {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
        Session->CurrentState = B_AkeState_eError;
    }else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        Session->CurrentState = B_AkeState_eResponse;
        /* Examine SRM , set update flag if either party's version number is newer */
        if((ret = B_DTCP_CheckSrmUpdate(Stack->DeviceParams, Session)) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to check SRM\n"));
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
        B_Time_Get(&Session->ResponseReceivedTime);
    }

    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                Cmd,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else
    {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }
    BDBG_LEAVE(B_DTCP_ProcessResponse);

    return ret;
}
/*! \brief Create and send exchange key command, called by source device only.
 */
BERR_Code B_DTCP_SendExchKeyCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, Response;
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendExchKeyCmd);
#if 0
    if(Stack->CurrentState == B_Ake_State_eISOInProgress)
        return BERR_UNKNOWN; /*TODO: Should return a value to retry this function later on.*/
#endif
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eExchangeKey)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }
    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);
    if( ret == BERR_SUCCESS && Response == B_Response_eAccepted)
    {
        Session->CurrentState = B_AkeState_eCompleted;
        B_Time_Get(&Session->ExchKeySentTime);
    }

    BDBG_LEAVE(B_DTCP_SendExchKeyCmd);
    return ret;
}
/* \brief process exchange key command received from source device during AKE.
 * param[in] Session AKE core session data.
 * [param[i] Stack DTCP stack pointer.
 */
BERR_Code B_DTCP_ProcessExchKeyCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, CmdDep;
    int ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_ProcessExchKeyCmd);

    ret = Stack->Messenger->ConsumeCommand_Func(Stack->DeviceParams,
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(ret != BERR_SUCCESS)
    {
        Session->CurrentState = B_AkeState_eError;
        if(ret == BERR_NOT_SUPPORTED)
        {
            Response = B_Response_eRejected;
            Status = B_AkeStatus_eOther;
        }else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }else if (Session->CurrentState != B_AkeState_eResponse)
    {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
        Session->CurrentState = B_AkeState_eError;
    }else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        Session->CurrentState = B_AkeState_eCompleted;
        B_Time_Get(&Session->ExchKeyReceivedTime);
    }
    /* TODO: read system time, and put it in ExchangeKeyReceivedTime field in session data. */
    /* Create and send response */
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eExchangeKey,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else
    {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }
    BDBG_LEAVE(B_DTCP_ProcessExchKeyCmd);

    return ret;

}
/*! \brief send SRM command
 */
BERR_Code B_DTCP_SendSrmCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, Response;
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendSrmCmd);
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eSRM)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }
    BDBG_MSG(("SRM UPDATE :: Sending SRM update to other device"));

    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            (int*)&Cmd,
            (int*)&Response);
    if( ret == BERR_SUCCESS && Response == B_Response_eAccepted)
    {
        Session->CurrentState = B_AkeState_eCompleted;
        /*B_Time_Get(&SrmSentTime); */
    }
    else
    {
        Session->CurrentState = B_AkeState_eError;
        BDBG_ERR(("SRM update not processed by remote device. Failing AKE"));
    }

    BDBG_LEAVE(B_DTCP_SendSrmCmd);
    return ret;
}
/*! \brief Process SRM command
 */
BERR_Code B_DTCP_ProcessSrmCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, CmdDep;
    int size;
    int ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_ProcessSrmCmd);

    ret = Stack->Messenger->ConsumeCommand_Func(Stack->DeviceParams,
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize,
        &Cmd,
        &CmdDep);

    if(ret != BERR_SUCCESS)
    {
        if(ret == BERR_INVALID_SRM)
        {
            Response = B_Response_eRejected;
            Status = B_AkeStatus_eOther;
        }else {
            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
            Stack->OnInternalError(Stack,Session);
            return ret;
        }
    }
    else
    {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        B_Time_Get(&Session->SrmReceivedTime);
        /*Update the device's SRM by calling stack supplied function */
        size = B_DTCP_GetSrmLength(Session->CmdBuffer);
        Stack->UpdateSrm_Func(Session->CmdBuffer, size+4);
        Session->CurrentState = B_AkeState_eCompleted;
        BDBG_MSG(("SRM UPDATE :: Response Sent change state to complete"));
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBufferSize = 0;
    }
    /* Create and send response */
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eSRM,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else
    {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }

    BDBG_LEAVE(B_DTCP_ProcessSrmCmd);

    return ret;
}
/*! \brief Send content key request response, called by source device only.
 */
BERR_Code B_DTCP_SendContentKeyReqResponse(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)

{
    BERR_Code ret = BERR_SUCCESS;
    B_AkeResponse_T Response = B_Response_eAccepted;
    B_AkeStatus_T Status = B_AkeStatus_eOK;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendContentKeyReqResponse);

    if (Session->CurrentState != B_AkeState_eCompleted)
    {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
        Session->CurrentState = B_AkeState_eError;
    }
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eContentKeyReq,
                0,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }
    ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
    BKNI_Free(Session->CmdBuffer);
    Session->CmdBuffer = NULL;
    Session->CmdBufferSize = 0;

    BDBG_LEAVE(B_DTCP_SendContentKeyReqResponse);
    return ret;
}
/*! \brief send Ake cancel command.
 */
BERR_Code B_DTCP_SendCancelCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int Cmd, Response;
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendCancelCmd);
    /*
     * Can only be called after "CHALLENGE" command has been sent out (for sink device), or
     * before "EXCHANGE_KEY" command has been sent out (for source device).
     */
    if (Session->CurrentState == B_AkeState_eIdle ||
            Session->CurrentState == B_AkeState_eAuthenticated ||
            Session->CurrentState == B_AkeState_eCompleted)
    {
        return BERR_NOT_SUPPORTED;
    }
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eAkeCancel)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }
    ret = B_DTCP_AkeSendCmdWaitForResponse(Stack,
            Session,
            true,
            DTCP_AKE_CMD_TIMEOUT,
            &Cmd,
            &Response);

    if( ret == BERR_SUCCESS && Response == B_Response_eAccepted)
        Session->CurrentState = B_AkeState_eCanceled;

    BDBG_LEAVE(B_DTCP_SendCancelCmd);
    return ret;
}
/*! \brief process Ake cancel command
 */
BERR_Code B_DTCP_ProcessCancelCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int ret = BERR_SUCCESS;
    int Response = B_Response_eAccepted;
    int Status = B_AkeStatus_eOK;
    int Cmd, CmdDep ;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_ProcessCancelCmd);

    /*
     * The AKE cancel command has no data field, same as a response, so
     * we call ConsumeResponse_Func to free up the buffer.
     */
    ret = Stack->Messenger->ConsumeResponse_Func(
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize, &Cmd,  &Response, &Status );

    if (ret != BERR_SUCCESS) {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
    } else if (Session->CurrentState == B_AkeState_eIdle ||
            Session->CurrentState == B_AkeState_eAuthenticated ||
            Session->CurrentState == B_AkeState_eCompleted) {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
    } else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
        Session->CurrentState = B_AkeState_eCanceled;
    }
    if (IS_SINK(Session->DeviceMode))
        CmdDep = 0;
    else
        CmdDep = 1;
    /* Create and send response */
    if((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eAkeCancel,
                CmdDep,
                Response,
                Status)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }else
    {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }

    BDBG_LEAVE(B_DTCP_ProcessCancelCmd);

    return ret;
}
/*! \brief send capability request command, called by sink device only.
 */
BERR_Code B_DTCP_SendCapabilityRequestCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_SendCapabilityRequestCmd);
    /*
     * Can only be called before AKE challenge is sent
     */
    if (Session->CurrentState != B_AkeState_eIdle )
    {
        return BERR_NOT_SUPPORTED;
    }
    if((ret = Stack->Messenger->CreateCommand_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eCapabilityReq)) != BERR_SUCCESS)
    {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    }
    ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
    BKNI_Free(Session->CmdBuffer);
    Session->CmdBuffer = NULL;
    Session->CmdBufferSize = 0;

    BDBG_LEAVE(B_DTCP_SendCapabilityRequestCmd);
    return ret;
}
/*! \brief process capability request command. called by source device only.
 */
BERR_Code B_DTCP_ProcessCapabilityRequestCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session)
{
    int ret = BERR_SUCCESS;
    int Response = B_Response_eAccepted;
    int Status = B_AkeStatus_eOK;
    int Cmd;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_ENTER(B_DTCP_ProcessCancelCmd);

    /*
     * This command has no data field, same as a response, so
     * we call ConsumeResponse_Func to free up the buffer.
     */
    ret = Stack->Messenger->ConsumeResponse_Func(
        Session,
        Session->CmdBuffer,
        Session->CmdBufferSize, &Cmd,  &Response, &Status );

    if (ret != BERR_SUCCESS) {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eOther;
    } else if (Session->CurrentState != B_AkeState_eIdle ) {
        Response = B_Response_eRejected;
        Status = B_AkeStatus_eIncorrectCmdOrder;
    } else {
        Response = B_Response_eAccepted;
        Status = B_AkeStatus_eOK;
    }
    /*
     * Create and send response, this command could be only
     * send from sink -> source, so command dependent field
     * should always be 1.
     */
    if ((ret = Stack->Messenger->CreateResponse_Func(Stack->DeviceParams,
                Session,
                B_AkeCmd_eCapabilityReq,
                1,
                Response,
                Status)) != BERR_SUCCESS) {
        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
        Stack->OnInternalError(Stack,Session);
        return ret;
    } else {
        ret = Stack->Transporter->Send(Stack->Transporter, Session, Session->CmdBuffer, Session->CmdBufferSize);
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
        Session->CmdBufferSize = 0;
    }

    BDBG_LEAVE(B_DTCP_ProcessCancelCmd);

    return ret;
}
/*! \brief process received AKE command, main AKE state machine implmented here.
 *  \param[in] Session AKE session data poitner.
 *  \param[in] Stack DTCP stack pointer.
 */
BERR_Code B_DTCP_ProcessAkeCommands(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session,
        int Cmd)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(B_DTCP_ProcessAkeCommands);
    BDBG_ASSERT(Session);
    BDBG_ASSERT(Stack);

    BDBG_MSG(("Ake Command : %d\n", Cmd));
    switch(Cmd)
    {
        case B_AkeCmd_eCapabilityExchange:
            ret = B_DTCP_ProcessCapabilityExchangeCmd(Stack, Session);
            break;
        case B_AkeCmd_eChallenge:
            ret = B_DTCP_ProcessChallengeCmd(Stack, Session);
            if( IS_SOURCE(Stack->DeviceParams->Mode) && ret == BERR_SUCCESS)
            {
                if(Session->CurrentState != B_AkeState_eCanceled &&
                    Session->CurrentState != B_AkeState_eError)
                {
                    /* Give sink device some time to read response buffer out */
                    ret = B_DTCP_SendChallengeCmd(Stack, Session);
                }
                if(ret == BERR_SUCCESS && Session->CurrentState != B_AkeState_eCanceled
                        && Session->CurrentState != B_AkeState_eError)
                {
                    ret = B_DTCP_SendResponseCmd(Stack, Session);
                }
            }
            break;
        case B_AkeCmd_eResponse:
        case B_AkeCmd_eResponse2:
            ret = B_DTCP_ProcessResponseCmd(Stack, Session);
            if(ret == BERR_SUCCESS && Session->CurrentState != B_AkeState_eCanceled
                    && Session->CurrentState != B_AkeState_eError)
            {
                if(IS_SOURCE(Stack->DeviceParams->Mode))
                {
                    /*
                     * If we are a source device and received response command,
                     * compute Authentication key.
                     */
                    if(B_DTCP_GetSharedSecret(Stack->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                            Session->AuthKey,
                            Session->FirstPhaseSecret,
                            Session->OtherFirstPhaseValue
                            ) != BCRYPT_STATUS_eOK)
                    {
                        BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
                        Stack->OnInternalError(Stack,Session);
                        return BERR_CRYPT_FAILED;
                    }
#ifdef DTCP_DEMO_MODE
                    BDBG_MSG(("AuthKey:\n"));
                    BDBG_BUFF(Session->AuthKey, DTCP_AUTH_KEY_SIZE);
#endif
                    /*
                     * If both source and sink device's AL flag is set, must perform RTT
                     * test before sending exchange keys, if the sink device is not on
                     * source device's Rtt registry.
                     */
                    if(B_DTCP_IsALRequired(Stack->DeviceParams, Session) == true)
                    {
                        ret = B_DTCP_IP_DoRtt(Stack, Session);
                    }

                    if(ret == BERR_SUCCESS && Session->CurrentState != B_AkeState_eCanceled &&
                            Session->CurrentState != B_AkeState_eError)
                    {
                        ret = B_DTCP_SendExchKeyCmd(Stack, Session);
                        if(ret == BERR_SUCCESS)
                        {
                            if(Session->OtherSrmUpdateRequired == true || Session->OtherSrmReplaceRequired == true)
                            {
                                ret = B_DTCP_SendSrmCmd(Stack, Session);
                            }
                            if(Session->OurSrmUpdateRequired == true || Session->OurSrmReplaceRequired == true)
                            {
                                Session->CurrentState = B_AkeState_eSrm;
                            }

                        }else {
                            Session->CurrentState = B_AkeState_eCompleted;
                        }
                    }
                }else {
                    ret = B_DTCP_SendResponseCmd(Stack, Session);
                    /*
                     * If we are a sink device and response command acceptec by source,
                     * compute Authentication key.
                     */
                    if ( ret == BERR_SUCCESS && Session->CurrentState == B_AkeState_eResponse)
                    {
                        if(B_DTCP_GetSharedSecret(Stack->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                            Session->AuthKey,
                            Session->FirstPhaseSecret,
                            Session->OtherFirstPhaseValue
                            ) != BCRYPT_STATUS_eOK)
                        {
                            BDBG_WRN(("%s %d : Internal Error", __FUNCTION__, __LINE__));
                            Stack->OnInternalError(Stack,Session);
                            return BERR_CRYPT_FAILED;
                        }
#ifdef DTCP_DEMO_MODE
                        BDBG_MSG(("AuthKey:\n"));
                        BDBG_BUFF(Session->AuthKey, DTCP_AUTH_KEY_SIZE);
#endif
                    }
                }
            }
            break;
        case B_AkeCmd_eExchangeKey:
            ret = B_DTCP_ProcessExchKeyCmd(Stack, Session);
            if(ret == BERR_SUCCESS && Session->CurrentState != B_AkeState_eError
                    && Session->CurrentState != B_AkeState_eCanceled
                    && IS_SINK(Stack->DeviceParams->Mode))
            {
                if(Session->OtherSrmUpdateRequired == true || Session->OtherSrmReplaceRequired == true)
                {
                    ret = B_DTCP_SendSrmCmd(Stack, Session);
                }
                if(Session->OurSrmUpdateRequired == true || Session->OurSrmReplaceRequired == true)
                {
                     Session->CurrentState = B_AkeState_eSrm;
                }
            }
            break;
        case B_AkeCmd_eSRM:
            ret = B_DTCP_ProcessSrmCmd(Stack, Session);
            break;
        case B_AkeCmd_eAkeCancel:
            ret = B_DTCP_ProcessCancelCmd(Stack, Session);
            break;
        case B_AkeCmd_eContentKeyReq:
            if(IS_SOURCE(Stack->DeviceParams->Mode))
            {
                ret = B_DTCP_SendContentKeyReqResponse(Stack, Session);
            }
            break;
        case B_AkeCmd_eCapabilityReq:
            ret = B_DTCP_ProcessCapabilityRequestCmd(Stack, Session);
            break;
        default:
            ret = B_DTCP_ProcessUnKnownCmd(Stack, Session);
            if (ret == BERR_SUCCESS)
                ret = BERR_NOT_SUPPORTED;
    }
    BDBG_LEAVE(B_DTCP_ProcessAkeCommands);

    return ret;
}
