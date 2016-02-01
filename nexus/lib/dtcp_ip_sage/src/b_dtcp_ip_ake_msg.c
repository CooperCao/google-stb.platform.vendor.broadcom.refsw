/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
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
 *     DTCP-IP Ake functions, IP AKE messaging interface.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake_msg.c
 *  \brief implement DTCP-IP specific  Ake functionalities's message producing and consuming
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_ip_ake_msg.h"

BDBG_MODULE(b_dtcp_ip);

/*! \fn static void B_IpAkeHeaderInit(unsigned char * buffer)
 *  \param buffer, input buffer pointer.
 */
static void B_IpAkeHeaderInit(unsigned char * buffer, unsigned char label)
{
    int Number = 0; /* optional */
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T *)buffer;
    BKNI_Memset(buffer, 0, sizeof(B_IpAkeHeader_T));
    h->Type = 0x01;
    h->AkeLabel = label;
    h->Number = (unsigned char)Number;
}
/*! \fn static void B_AkeHeaderSetLength(unsigned char *buffer, int length)
 *  \param buffer, input buffer pointer.
 *  \param length, length value to set.
 */
static void B_IpAkeHeaderSetLength(unsigned char * buffer, int length)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    length -= 3;
    h->Length = htons(0xffff & length);
}
/* WARNING: Get the AKE MSG header length only */
static void B_IpAkeHeaderGetLength(unsigned char * buffer, int * length)
{
#if 0
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T *)buffer;
    *length = ntohs(h->Length);
#endif
    (void)buffer;
    *length = sizeof(B_IpAkeHeader_T);
}
static void B_IpAkeHeaderGetLabel(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = (int)(h->AkeLabel);
}
/*! \fn static void B_AkeHeaderSetCtypeResponse(unsigned char *buffer, int value)
 *  \param buffer, input buffer pointer.
 *  \param length, length value to set.
 */
static void B_IpAkeHeaderSetCtypeResponse(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->Ctype_Response = 0x0F & value;
}
/*! \fn static void B_AkeHeaderGetCtypeResponse(unsigned char *buffer, int *value)
 *  \param buffer, input buffer pointer.
 *  \param value, output returned value.
 */
static void B_IpAkeHeaderGetCtypeResponse(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = 0x0F & h->Ctype_Response;
}
/*! \fn static void B_AkeHeaderSetCmd(unsigned char *buffer, int value)
 *  \param buffer, input buffer pointer.
 *  \param value, length value to set.
 */
static void B_IpAkeHeaderSetCmd(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->Cmd = (unsigned char)value;
}
/*! \fn static void B_AkeHeaderGetCmd(unsigned char *buffer, int *value)
 *  \param buffer, input buffer pointer.
 *  \param value, output returned value.
 */
static void B_IpAkeHeaderGetCmd(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = h->Cmd;
}
/*! \fn static void B_AkeHeaderSetAkeType(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value, input, value to set.
 */
static void B_IpAkeHeaderSetAkeType(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->AkeType = (unsigned char)value;
}
/*! \fn static void B_AkeHeaderGetAkeType(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value, output, returned value.
 */
static void B_IpAkeHeaderGetAkeType(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = 0xFF & h->AkeType;
}
/*! \fn static void B_AkeHeaderSetExchKeyType(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value, input, value to set.
 */
static void B_IpAkeHeaderSetExchKeyType(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->ExchKeyType = (unsigned char)value;
}
/*! \fn static void B_AkeHeaderGetExchKeyType(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value, input, value to set.
 */
static void B_IpAkeHeaderGetExchKeyType(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = h->ExchKeyType;
}
/*! \fn static void B_AkeHeaderSetCmdDependent(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value,  input, value to set.
 */
static void B_IpAkeHeaderSetCmdDependent(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->CmdDep = (unsigned char)value;
}
/*! \fn static void B_AkeHeaderGetCmdDependent(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value,  input, value to set.
 */
static void B_IpAkeHeaderGetCmdDependent(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = h->CmdDep;
}
static void B_IpAkeHeaderSetStatus(unsigned char * buffer, int value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    h->Status = (unsigned char)value;
}
/*! \fn static void B_AkeHeaderGetCmdDependent(unsigned char *buffer, int value)
 *  \param buffer, input, buffer pointer.
 *  \param value,  input, value to set.
 */
static void B_IpAkeHeaderGetStatus(unsigned char * buffer, int * value)
{
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)buffer;
    *value = h->Status;
}

static B_AkeHeader_ops_T IpAkeHeader_ops =
{
    &B_IpAkeHeaderInit,
    &B_IpAkeHeaderSetLength,
    &B_IpAkeHeaderGetLength,
    &B_IpAkeHeaderGetLabel,
    &B_IpAkeHeaderSetCtypeResponse,
    &B_IpAkeHeaderGetCtypeResponse,
    &B_IpAkeHeaderSetCmd,
    &B_IpAkeHeaderGetCmd,
    &B_IpAkeHeaderSetAkeType,
    &B_IpAkeHeaderGetAkeType,
    &B_IpAkeHeaderSetExchKeyType,
    &B_IpAkeHeaderGetExchKeyType,
    &B_IpAkeHeaderSetCmdDependent,
    &B_IpAkeHeaderGetCmdDependent,
    &B_IpAkeHeaderSetStatus,
    &B_IpAkeHeaderGetStatus
};
/*! \brief utility to get CType string for dumping message.
 *  \param[in] Ctype ctype value to be displayed.
 *  \param[in,out] buffer buffer to store the string.
 */
static char * B_GetCtypeString(int Ctype, char * buffer, int size)
{
    switch(Ctype)
    {
        case B_CType_eControl:
            snprintf(buffer, size, "Control");
            break;
        case B_CType_eStatus:
            snprintf(buffer, size, "Status");
            break;
        case B_CType_eSpecificInquiry:
            snprintf(buffer, size, "SpecificInquiry");
            break;
        case B_CType_eNotify:
            snprintf(buffer, size, "Notify");
            break;
        case B_CType_eGeneralInquiry:
            snprintf(buffer, size, "GeneralInquiry");
            break;
        default:
            snprintf(buffer, size, "Unkown CType");
    }
    return buffer;
}
/*! \brief utility to display AKE command string.
 *  \param[in] cmd AKE command to be displayed.
 *  \param[in,out] buffer buffer to store the string.
 */
static char * B_GetCmdString(int cmd, char * buffer, int size)
{
    switch(cmd)
    {
        case B_AkeCmd_eStatus:
            snprintf(buffer, size, "Status");
            break;
        case B_AkeCmd_eChallenge:
            snprintf(buffer,size, "Challenge");
            break;
        case B_AkeCmd_eResponse:
            snprintf(buffer, size, "Response");
            break;
        case B_AkeCmd_eExchangeKey:
            snprintf(buffer, size, "ExchangeKey");
            break;
        case B_AkeCmd_eSRM:
            snprintf(buffer, size, "SRM");
            break;
        case B_AkeCmd_eResponse2:
            snprintf(buffer, size, "Response2");
            break;
        case B_AkeCmd_eAkeCancel:
            snprintf(buffer, size, "AkeCancel");
            break;
        case B_AkeCmd_eContentKeyReq:
            snprintf(buffer, size, "ContentKeyRequest");
            break;
        case B_AkeCmd_eCapabilityReq:
            snprintf(buffer, size, "CapabilityRequest");
            break;
        case B_AkeCmd_eCapabilityExchange:
            snprintf(buffer, size, "CapabilityExchange");
            break;
        case B_IpAkeCmd_eRttReady:
            snprintf(buffer, size, "RttReady");
            break;
        case B_IpAkeCmd_eRttSetup:
            snprintf(buffer, size, "RttSetup");
            break;
        case B_IpAkeCmd_eRttTest:
            snprintf(buffer, size, "RttTest");
            break;
        case B_IpAkeCmd_eContKeyConf:
            snprintf(buffer, size, "ContKeyConf");
            break;
        case B_IpAkeCmd_eRttVerify:
            snprintf(buffer, size, "RttVerify");
            break;
        case B_IpAkeCmd_eBgRttInitiate:
            snprintf(buffer, size, "BgRttInitiate");
            break;
        default:
            snprintf(buffer, size, "Unknown");
    }
    return buffer;
}
/*! \brief utility to display AKE response string.
 *  \param[in] Response response code.
 *  \param[in,out] buffer buffer to store string.
 */
static char * B_GetResponseString(int Response, char * buffer, int size)
{
    switch(Response)
    {
        case B_Response_eNotImplemented:
            snprintf(buffer, size, "Not Implemented");
            break;
        case B_Response_eAccepted:
            snprintf(buffer, size, "Accepted");
            break;
        case B_Response_eRejected:
            snprintf(buffer, size, "Rejected");
            break;
        case B_Response_eInTransition:
            snprintf(buffer, size, "InTransition");
            break;
        case B_Response_eStable:
            snprintf(buffer, size, "Stable");
            break;
        case B_Response_eChanged:
            snprintf(buffer, size, "Changed");
            break;
        case B_Response_eInterim:
            snprintf(buffer, size, "Interim");
            break;
        default:
            snprintf(buffer, size, "Unknown Response");
    }
    return buffer;
}
/*! \brief utility to display statys string.
 *  \param[in] status status code.
 *  \param[in,out] buffer to store the string.
 */
static char *  B_GetStatusString(int status, char * buffer, int size)
{
    switch(status)
    {
        case B_AkeStatus_eOK:
            snprintf(buffer, size, "No Error");
            break;
        case B_AkeStatus_eNoMoreAuth:
            snprintf(buffer, size, "Support for No more auth is currently available");
            break;
        case B_AkeStatus_eNoIso:
        case B_AkeStatus_eNoAC:
            break;
        case B_AkeStatus_eNoP2P:
            snprintf(buffer, size, "NO point-point connection");
            break;
        case B_AkeStatus_eOther:
            snprintf(buffer, size, "Any other error");
            break;
        case B_AkeStatus_eIncorrectCmdOrder:
            snprintf(buffer, size, "Incorrect command order");
            break;
        case B_AkeStatus_eAuthFailed:
            snprintf(buffer, size, "Authentication Failed");
            break;
        case B_AkeStatus_eSyntexError:
            snprintf(buffer, size, "Syntex Error");
            break;
        case B_AkeStatus_eNoInfo:
            snprintf(buffer, size, "No Info");
            break;
        default:
            snprintf(buffer, size, "Unknown status code");
    }
    return buffer;
}
/*! \brief B_DumpHeader debug function, dump AKE message header.
 *  \param[in] data data buffer.
 *  \param[in,out] buffer string buffer contains the decoded message.
 *  \param[in] bufferLength length of the string buffer.
 */
void B_DumpHeader(unsigned char * data, char * buffer, int bufferLength)
{
    char temp[1024];
    char cmd_str[256], status_str[256], ctype_str[256];
    int Ctype_Response;
    B_IpAkeHeader_T * h = (B_IpAkeHeader_T*)data;

    BDBG_ASSERT(buffer);
    BDBG_ASSERT(data);
    Ctype_Response = (h->Ctype_Response) & 0x0F;

    memset(buffer, 0, bufferLength);
    snprintf(buffer, bufferLength, "Length: \t %d \n", ntohs(h->Length));

    if(Ctype_Response <= B_CType_eGeneralInquiry &&
        Ctype_Response >= B_CType_eControl)
    {
        snprintf(temp, 1024, "CType:\t %s\n", B_GetCtypeString(Ctype_Response, ctype_str, 256));
    }else if(Ctype_Response >= B_Response_eNotImplemented &&
        Ctype_Response <= B_Response_eInterim)
    {
        snprintf(temp, 1024, "CType:\t %s\n", B_GetResponseString(Ctype_Response, ctype_str, 256));
    }else
    {
        snprintf(temp, 1024, "CType:\t Unknown %d\n",Ctype_Response);
    }
    strncat(buffer, temp, strlen(temp));
    snprintf(temp, 1024, "Cmd: %s\nAkeType: %d\nExchKeyType: %d\nCmdDependent:%d\nAkeLabel: %d\nNumber=%d\nStatus: %s\n",
        B_GetCmdString((h->Cmd) & 0xFF, cmd_str, 256),
        h->AkeType,
        h->ExchKeyType,
        h->CmdDep,
        h->AkeLabel,
        (h->Number)&0x0F,
        B_GetStatusString((h->Status) & 0x0F, status_str, 256));
    strncat(buffer, temp, strlen(temp));
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("Header Raw Buffer;\n"));
    BDBG_BUFF(data, sizeof(B_IpAkeHeader_T));
    BDBG_MSG(("\n"));
#endif
}
/*! \brief Create Rtt Ready message.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in] Session AkE session data.
 * \param[in] headroom bytes to reserve for message header.
 * \param[in] hops AKE header operation poitner.
 */
BERR_Code B_DTCP_IP_CreateRttReady(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int CmdDep;
    (void)DeviceParams;
    if (IS_SINK(Session->DeviceMode))
        CmdDep = 1;
    else
        CmdDep = 0;
    return B_DTCP_CreateMsgNoData(Session, headroom, hops,
            B_CType_eControl,
            B_IpAkeCmd_eRttReady,
            CmdDep,
            B_AkeStatus_eNoInfo);
}
/*!\brief Create Rtt Ready message, source device only.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in] Session AkE session data.
 * \param[in] headroom bytes reserved in message header.
 * \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_IP_CreateRttSetup(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_CreateRttSetup);
    BDBG_ASSERT(Session);
    (void)DeviceParams;

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_RTT_N_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_RTT_N_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, B_CType_eControl);
    hops->SetCommand(Session->CmdBuffer, B_IpAkeCmd_eRttSetup);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    BKNI_Memcpy(Session->CmdBuffer + headroom, IpAkeData->RttN, DTCP_RTT_N_SIZE);

    BDBG_LEAVE(B_DTCP_IP_CreateRttSetup);
    return BERR_SUCCESS;
}
/*!\brief Consume Rtt Ready message, sink device only
 * \param[in] DeviceParams receiving device's parameter.
 * \param[in,out] Session AkE session data.
 * \param[in] buffer command message buffer.
 * \param[in] len command message buffer length.
 * \param[in] headroom  bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_IP_ConsumeRttSetup(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int len, int headroom)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_IP_ConsumeRttSetup);
    (void)DeviceParams; (void)len;

    BKNI_Memcpy(IpAkeData->RttN, buffer + headroom, DTCP_RTT_N_SIZE);
    BKNI_Free(buffer);
    BDBG_LEAVE(B_DTCP_IP_ConsumeRttSetup);

    return BERR_SUCCESS;
}
/*!\brief Create Rtt Setup response  message, sink device only.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in] Session AkE session data.
 * \param[in] Cmd command codd. RTT_SETUP
 * \param[in] CmdDep command dependent field data, should be 0
 * \param[in] Response Response code.
 * \param[in] Status status code.
 */
BERR_Code B_DTCP_CreateRttSetupResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int Cmd, int CmdDep, int Response, int Status)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;
    int headroom = sizeof(B_IpAkeHeader_T);
    B_AkeHeader_ops_T * hops = &IpAkeHeader_ops;

    BDBG_ASSERT(Session);

    (void)DeviceParams;
    BDBG_ENTER(B_DTCP_CreateRttSetupResponse);

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_RTT_N_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_RTT_N_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, Response);
    hops->SetCommand(Session->CmdBuffer, Cmd);
    hops->SetCmdDependent(Session->CmdBuffer, CmdDep);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetStatus(Session->CmdBuffer, Status);

    BKNI_Memcpy(Session->CmdBuffer + headroom ,
            &IpAkeData->RttN, DTCP_RTT_N_SIZE);

    BDBG_LEAVE(B_DTCP_CreateRttSetupResponse);

    return BERR_SUCCESS;
}
/*! \brief Create Rtt Test message, called by source device only.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AkE session data.
 *  \param[in] headroom  bytes to reserve for message header.
 *  \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_IP_CreateRttTest(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;

    BDBG_ENTER(B_DTCP_CreateRttTest);
    BDBG_ASSERT(Session);
    (void)DeviceParams;

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_RTT_MAC_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_RTT_MAC_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, B_CType_eControl);
    hops->SetCommand(Session->CmdBuffer, B_IpAkeCmd_eRttTest);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    /*
     * Send MSB of RttMAC.
     */
    BKNI_Memcpy(Session->CmdBuffer + headroom , IpAkeData->RttMac, DTCP_RTT_MAC_SIZE);

    BDBG_LEAVE(B_DTCP_CreateRttTest);
    return BERR_SUCCESS;
}
/*! \brief Consume Rtt Test message, called by sink device only,
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AkE session data.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_IP_ConsumeRttTest(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int len, int headroom)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_IP_ConsumeRttTest);
    (void)DeviceParams; (void)len;

    BKNI_Memcpy(IpAkeData->OtherRttMac, buffer + headroom, DTCP_RTT_MAC_SIZE);
    BKNI_Free(buffer);
    BDBG_LEAVE(B_DTCP_IP_ConsumeRttTest);

    return BERR_SUCCESS;
}
/*! \brief Create Rtt test Response  message.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in,out] Session AkE session data.
 * \param[in] Cmd command codd. RTT_SETUP
 * \param[in] CmdDep command dependent field data, should be 0
 * \param[in] Response Response code.
 * \param[in] Status status code.
 */
BERR_Code B_DTCP_IP_CreateRttTestResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int Cmd, int CmdDep, int Response, int Status)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;
    int headroom = sizeof(B_IpAkeHeader_T);
    B_AkeHeader_ops_T * hops = &IpAkeHeader_ops;

    BDBG_ASSERT(Session);

    (void)DeviceParams;
    BDBG_ENTER(B_DTCP_CreateRttTestResponse);

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_RTT_MAC_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_RTT_MAC_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, Response);
    hops->SetCommand(Session->CmdBuffer, Cmd);
    hops->SetCmdDependent(Session->CmdBuffer, CmdDep);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetStatus(Session->CmdBuffer, Status);

    BKNI_Memcpy(Session->CmdBuffer + headroom ,
            &IpAkeData->RttMac[DTCP_RTT_MAC_SIZE], DTCP_RTT_MAC_SIZE);

    BDBG_LEAVE(B_DTCP_CreateRttTestResponse);
    return BERR_SUCCESS;
}
/*! \brief Consume Rtt Test response message, called by source device only.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AkE session data.
 */
BERR_Code B_DTCP_IP_ConsumeRttTestResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int * Cmd, int * Response, int * Status)
{
    B_IpAkeSessionData_T * IpAkeData;
    BERR_Code retValue;
    int CmdDep;

    BDBG_ASSERT(Session);
    IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;
    BDBG_ENTER(B_DTCP_IP_ConsumeRttTestResponse);

    BKNI_Memcpy(&IpAkeData->OtherRttMac[DTCP_RTT_MAC_SIZE], buffer + sizeof(B_IpAkeHeader_T), DTCP_RTT_MAC_SIZE);

    /* Call ConsumeMsgNoData() to free the command buffer! */
    retValue = B_DTCP_ConsumeMsgNoData(Session, buffer, &IpAkeHeader_ops, Response,
            Cmd, &CmdDep, Status);
    BDBG_LEAVE(B_DTCP_IP_ConsumeRttTestResponse);

    return retValue;
}
/*! \brief Create Rtt Verify command message.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] SessionAkE session data.
 *  \param[in] headroom bytes to reserve in message header.
 */
BERR_Code B_DTCP_IP_CreateRttVerify(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    (void)DeviceParams;
    return B_DTCP_CreateMsgNoData(Session, headroom, hops,
                B_CType_eControl,
                B_IpAkeCmd_eRttVerify,
                0,
                B_AkeStatus_eNoInfo);
}
/*! \brief Create Rtt Verify Response  message.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in,out] Session AkE session data.
 * \param[in] Cmd command codd. RTT_SETUP
 * \param[in] CmdDep command dependent field data, should be 0
 * \param[in] Response Response code.
 * \param[in] Status status code.
 */
BERR_Code B_DTCP_IP_CreateRttVerifyResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int Cmd, int CmdDep, int Response, int Status)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;
    int headroom = sizeof(B_IpAkeHeader_T);
    B_AkeHeader_ops_T * hops = &IpAkeHeader_ops;

    BDBG_ASSERT(Session);

    (void)DeviceParams;
    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_RTT_VERIFY_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_RTT_VERIFY_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, Response);
    hops->SetCommand(Session->CmdBuffer, Cmd);
    hops->SetCmdDependent(Session->CmdBuffer, CmdDep);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetStatus(Session->CmdBuffer, Status);

    BKNI_Memcpy(Session->CmdBuffer + headroom ,
            IpAkeData->OkMsg, DTCP_RTT_MAC_SIZE);

    return BERR_SUCCESS;
}
/*! \brief Create Rtt Verify Response  message.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session, AkE session data.
 */
BERR_Code B_DTCP_IP_ConsumeRttVerifyResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int * Cmd, int * Response, int * Status)
{
    B_IpAkeSessionData_T * IpAkeData;
    BERR_Code retValue;
    int CmdDep;

    BDBG_ASSERT(Session);
    IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;
    BKNI_Memcpy(IpAkeData->OtherOkMsg, Session->CmdBuffer + sizeof(B_IpAkeHeader_T), DTCP_RTT_VERIFY_SIZE);
    retValue = B_DTCP_ConsumeMsgNoData(Session, buffer, &IpAkeHeader_ops, Response,
            Cmd, &CmdDep, Status);
    return BERR_SUCCESS;
}
/*! \brief Create Content key confirmation command/response. For sink device, it creates command,
 * for Source device, it creates response.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AkE session data.
 *  \param[in] headroom bytes to reserve in message header.
 *  \param[in] hops Ake header operation pointer.
 */
BERR_Code B_DTCP_IP_CreateContKeyConf(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int offset;
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T *)Session->pProtocolData;

    BDBG_ENTER(B_DTCP_IP_CreateContKeyConf);
    BDBG_ASSERT(Session);
    (void)DeviceParams;

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_CONT_KEY_CONF_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_CONT_KEY_CONF_SIZE;
    }
    /* AKE label is 0 for this command */
    hops->Init(Session->CmdBuffer, 0);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCommand(Session->CmdBuffer, B_IpAkeCmd_eContKeyConf);
    hops->SetAkeType(Session->CmdBuffer, 0);
    hops->SetExchKeyType(Session->CmdBuffer, 0);
    hops->SetCmdDependent(Session->CmdBuffer, 0);
    offset = headroom ;
    /* For sink device, Nc undertest (NcT) is the session's Real-time none value obtained from
     * PCP header.
     * For source device, Nc is the value received from sink's content key confirmation command.
     */
    if (IS_SINK(Session->DeviceMode)) {
        hops->SetCtypeResponse(Session->CmdBuffer, B_CType_eControl);
        hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);
        BKNI_Memcpy(Session->CmdBuffer + offset, Session->RealTimeNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    } else {
        hops->SetCtypeResponse(Session->CmdBuffer, B_Response_eAccepted);
        hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eOK);
        BKNI_Memcpy(Session->CmdBuffer + offset, IpAkeData->NcT, DTCP_CONTENT_KEY_NONCE_SIZE);
    }
    offset += DTCP_CONTENT_KEY_NONCE_SIZE;
    BKNI_Memcpy(Session->CmdBuffer + offset, IpAkeData->ContR, DTCP_CONT_KEY_CONF_R_SIZE);
    offset += DTCP_CONT_KEY_CONF_R_SIZE;
    /*
     * For sink device, send MAC3B (MSB of Mac value),
     * For source device, send MAC4A (LSB of mac value).
     */
    if (IS_SINK(Session->DeviceMode))
        BKNI_Memcpy(Session->CmdBuffer + offset, IpAkeData->ContKeyConfMac,
                DTCP_CONT_KEY_CONF_MAC_SIZE);
    else {
        BKNI_Memcpy(Session->CmdBuffer + offset,
                &IpAkeData->ContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_SIZE],
                DTCP_CONT_KEY_CONF_MAC_SIZE);
    }

    BDBG_LEAVE(B_DTCP_IP_CreateContKeyConf);
    return BERR_SUCCESS;
}
/*! \brief Consume Content key confirm command/response, for sink device, it consumes the
 * response from source device, for source device, it consumes command from sink device.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in] Session AkE session data.
 *  \param[in] buffer command buffer.
 *  \param[in] len command buffer length.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_IP_ConsumeContKeyConf(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int len, int headroom)
{
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_IP_ConsumeContKeyConf);
    (void)len;

    BKNI_Memcpy(IpAkeData->NcT, Session->CmdBuffer + headroom,
            DTCP_CONTENT_KEY_NONCE_SIZE);
    headroom += DTCP_CONTENT_KEY_NONCE_SIZE;
    BKNI_Memcpy(IpAkeData->ContR, buffer + headroom,
            DTCP_CONT_KEY_CONF_R_SIZE);
    headroom += DTCP_CONT_KEY_CONF_R_SIZE;
    /*
     * For sink device, we received MAC4A, LSB of the mac value.
     * For source device, we recevied MAC3B, MSB of the mac value.
     */
    if (IS_SINK(Session->DeviceMode))
        BKNI_Memcpy(&IpAkeData->OtherContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_SIZE],
                buffer + headroom,
                DTCP_CONT_KEY_CONF_MAC_SIZE);
    else
        BKNI_Memcpy(IpAkeData->OtherContKeyConfMac,
                buffer + headroom,
                DTCP_CONT_KEY_CONF_MAC_SIZE);

    BKNI_Free(buffer);
    Session->CmdBuffer = NULL;
    Session->CmdBufferSize = 0;
    BDBG_LEAVE(B_DTCP_IP_ConsumeContKeyConf);

    return BERR_SUCCESS;
}
BERR_Code B_DTCP_IP_ConsumeContKeyConfResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int * Cmd, int * Response, int * Status)
{
    BERR_Code retValue;
    int CmdDep;
    B_IpAkeSessionData_T * IpAkeData = (B_IpAkeSessionData_T*)Session->pProtocolData;
    int headroom = sizeof(B_IpAkeHeader_T);

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_IP_ConsumeContKeyConfResponse);

    BKNI_Memcpy(IpAkeData->NcT, Session->CmdBuffer + headroom,
            DTCP_CONTENT_KEY_NONCE_SIZE);
    headroom += DTCP_CONTENT_KEY_NONCE_SIZE;
    BKNI_Memcpy(IpAkeData->ContR, buffer + headroom,
            DTCP_CONT_KEY_CONF_R_SIZE);
    headroom += DTCP_CONT_KEY_CONF_R_SIZE;
    /*
     * For sink device, we received MAC4A, LSB of the mac value.
     * For source device, we recevied MAC3B, MSB of the mac value.
     */
    BKNI_Memcpy(&IpAkeData->OtherContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_SIZE],
            buffer + headroom,
            DTCP_CONT_KEY_CONF_MAC_SIZE);

    retValue = B_DTCP_ConsumeMsgNoData(Session, buffer, &IpAkeHeader_ops, Response,
            Cmd, &CmdDep, Status);
    BDBG_LEAVE(B_DTCP_IP_ConsumeContKeyConfResponse);

    return retValue;
}
/*! \brief exported interface function to consume AKE response.
 *  \param[in] Session AKE core session data.
 *  \param[in] buffer buffer stores the data .
 *  \param[in] length buffer length.
 *  \param[in,out] Cmd returned AKE command.
 *  \param[in,out] Response returned AKE Response.
 *  \param[in,out] Status returned status code.
 *  \retval BERR_SUCCESS or other error code.
 *
 *  NOTE: This function free the command buffer
 */
BERR_Code B_DTCP_IP_AkeConsumeResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int length,
        int * Cmd,
        int * Response,
        int * Status)
{
    BERR_Code retValue;
    int CmdDep;
    IpAkeHeader_ops.GetCommand(buffer, Cmd);

    switch (*Cmd) {
        case B_IpAkeCmd_eRttTest:
            retValue = B_DTCP_IP_ConsumeRttTestResponse(Session, buffer, Cmd, Response, Status);
            break;
        case B_IpAkeCmd_eRttVerify:
            retValue = B_DTCP_IP_ConsumeRttVerifyResponse(Session, buffer, Cmd, Response, Status);
            break;
        case B_IpAkeCmd_eContKeyConf:
            retValue = B_DTCP_IP_ConsumeContKeyConfResponse(Session, buffer, Cmd, Response, Status);
            break;
        case B_AkeCmd_eContentKeyReq:
            retValue = B_DTCP_ConsumeContKeyReqResponse(Session, buffer, Cmd, Response, Status,
                sizeof(B_IpAkeHeader_T), &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eCapabilityExchange:
            retValue = B_DTCP_ConsumeCapabilityExchangeMsg(Session, buffer, length, sizeof(B_IpAkeHeader_T));
            break;
        default:
            /* NOTE: if response is eRttSetup, we ignored RttN value */
            retValue = B_DTCP_ConsumeMsgNoData(Session, buffer, &IpAkeHeader_ops,
                Response,
                Cmd,
                &CmdDep,
                Status);
            break;
    }
    return retValue;
}
/*! \brief exported interface to create an AKE command message.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in] Session AKE session data.
 *  \param[in] cmd AKE command.
 *  \retval BERR_SUCCESS or other error code.
 *
 *  The command message will be stored in Session->CmdBuffer, the buffer size is
 *  specified by Session->CmdBufferSize.
 */
BERR_Code B_DTCP_IP_AkeCreateCommands(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session, int Cmd)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_IP_AkeCreateCommands);
    BDBG_MSG(("Creating command : %d\n", Cmd));
    switch(Cmd)
    {
        case B_AkeCmd_eStatus:
            ret = B_DTCP_CreateStatusCmd(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eChallenge:
            ret = B_DTCP_CreateChallenge(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eResponse:
            ret = B_DTCP_CreateResponse(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eExchangeKey:
            ret = B_DTCP_CreateExchangeKey(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eSRM:
            ret = B_DTCP_CreateSrm(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eResponse2:
            ret = B_DTCP_CreateResponse2(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eAkeCancel:
            ret = B_DTCP_CreateAkeCancel(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eContentKeyReq:
            ret = B_DTCP_CreateContentKeyReq(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eCapabilityReq:
            ret = B_DTCP_CreateCapabilityReq(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eCapabilityExchange:
            ret = B_DTCP_CreateCapabilityExchangeMsg(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eRttReady:
            ret = B_DTCP_IP_CreateRttReady(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eRttSetup:
            ret = B_DTCP_IP_CreateRttSetup(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eRttTest:
            ret = B_DTCP_IP_CreateRttTest(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eContKeyConf:
            ret = B_DTCP_IP_CreateContKeyConf(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eRttVerify:
            ret = B_DTCP_IP_CreateRttVerify(DeviceParams,
                    Session,
                    sizeof(B_IpAkeHeader_T),
                    &IpAkeHeader_ops);
            break;
        case B_IpAkeCmd_eBgRttInitiate:
        default:
            BDBG_ERR(("Unknown AKE command"));
            ret = BERR_INVALID_PARAMETER;
    }
    BDBG_LEAVE(B_DTCP_IP_AkeCreateCommands);
    return ret;
}
/*! \brief exported interface function to create an AKE response message.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in] Session AKE core session data.
 *  \param[in] Cmd The AKE command that this response is responding to.
 *  \param[in] Response the response code.
 *  \param[in] Status the status code, e.g. accepted/rejected...
 *  \retval BERR_SUCCESS or other error code.
 *
 *  The response message created will be stored in Session->CmdBuffer.
 */
BERR_Code B_DTCP_IP_AkeCreateResponse(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        int Cmd,
        int CmdDep,
        int Response,
        int Status)
{
    BERR_Code retValue = BERR_SUCCESS;
    BDBG_ASSERT(Session);

    /* If command is rejected, no data need to be send */
    if( Response == B_Response_eRejected)
    {
        return  B_DTCP_CreateMsgNoData(Session,
                sizeof(B_IpAkeHeader_T),
                &IpAkeHeader_ops,
                Response,
                Cmd,
                CmdDep,
                Status);
    }
    switch(Cmd)
    {
        case B_AkeCmd_eCapabilityReq:
            retValue = B_DTCP_CreateCapabilityReqResponse(DeviceParams,
                Session,
                sizeof(B_IpAkeHeader_T),
                &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eCapabilityExchange:
            retValue = B_DTCP_CreateCapabilityExchangeMsg(DeviceParams,
                Session,
                sizeof(B_IpAkeHeader_T),
                &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eContentKeyReq:
            retValue = B_DTCP_CreateContentKeyReqResponse(DeviceParams,
                Session,
                sizeof(B_IpAkeHeader_T),
                &IpAkeHeader_ops, Response, Status);
            break;
        case B_IpAkeCmd_eRttSetup:
            retValue = B_DTCP_CreateRttSetupResponse(DeviceParams,
                Session, Cmd, CmdDep, Response, Status);
            break;
        case B_IpAkeCmd_eRttTest:
            retValue = B_DTCP_IP_CreateRttTestResponse( DeviceParams,
                Session, Cmd, CmdDep, Response, Status);
            break;
        case B_IpAkeCmd_eRttVerify:
            retValue = B_DTCP_IP_CreateRttVerifyResponse(DeviceParams,
                Session, Cmd, CmdDep, Response, Status);
            break;
        case B_IpAkeCmd_eContKeyConf:
            retValue = B_DTCP_IP_CreateContKeyConf(DeviceParams,
                Session, sizeof(B_IpAkeHeader_T), &IpAkeHeader_ops);
            break;
        case B_AkeCmd_eChallenge:
        case B_AkeCmd_eResponse:
        case B_AkeCmd_eResponse2:
        case B_AkeCmd_eExchangeKey:
        case B_AkeCmd_eSRM:
        case B_AkeCmd_eAkeCancel:
        case B_AkeCmd_eStatus:
        case B_AkeCmd_eSetDtcpMode:
        case B_IpAkeCmd_eRttReady:
        case B_IpAkeCmd_eBgRttInitiate:
            retValue = B_DTCP_CreateMsgNoData(Session,
                sizeof(B_IpAkeHeader_T),
                &IpAkeHeader_ops,
                Response,
                Cmd,
                CmdDep,
                Status);
            break;
        default:
            retValue = BERR_NOT_SUPPORTED;
    }
    return retValue;
}
/*! \brief exported interface function to consume the AKE message.
 *
 *  This function handle the AKE command and the response with data
 *  field.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in] Session Ake core session data.
 *  \param[in] buffer buffer contains the received AKE message.
 *  \param[in] length length of the buffer.
 *  \retval BERR_SUCCESS or other error code.
 *
 *  The memory pointed by buffer will be freed after this function call.
 */
BERR_Code B_DTCP_IP_AkeConsumeCommands(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int length,
        int *Cmd,
        int *CmdDep)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ASSERT(Session);

    IpAkeHeader_ops.GetCommand(buffer, Cmd);
    IpAkeHeader_ops.GetCmdDependent(buffer, CmdDep);
    switch(*Cmd)
    {
        case B_AkeCmd_eChallenge:
            if(IS_SOURCE(Session->DeviceMode)) {
                int Label = 0;
                B_IpAkeHeaderGetLabel(buffer, &Label);
                Session->AkeLabel = 0xFF & Label;
                BDBG_MSG(("AkeLabel from Client: %d\n", Session->AkeLabel));
            }
            ret = B_DTCP_ConsumeChallenge(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eResponse:
            ret = B_DTCP_ConsumeResponse(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eExchangeKey:
            ret = B_DTCP_ConsumeExchangeKey(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eSRM:
            ret = B_DTCP_ConsumeSrm(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eResponse2:
            ret = B_DTCP_ConsumeResponse2(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_IpAkeCmd_eRttSetup:
            ret = B_DTCP_IP_ConsumeRttSetup(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_IpAkeCmd_eRttTest:
            ret = B_DTCP_IP_ConsumeRttTest(DeviceParams,
                    Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_IpAkeCmd_eContKeyConf:
            ret = B_DTCP_IP_ConsumeContKeyConf(Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eCapabilityExchange:
            ret = B_DTCP_ConsumeCapabilityExchangeMsg(Session,
                    buffer,
                    length,
                    sizeof(B_IpAkeHeader_T)
            );
            break;
        case B_AkeCmd_eStatus:
        case B_AkeCmd_eAkeCancel:
        case B_AkeCmd_eContentKeyReq:
        case B_AkeCmd_eCapabilityReq:
        case B_IpAkeCmd_eRttReady:
        case B_IpAkeCmd_eRttVerify:
        {
            /*
             * These commands have no data field, call
             * ConsumeMsgNoData to free the buffer
             */
            int Ctype_Response, Status;
            ret = B_DTCP_ConsumeMsgNoData(Session, buffer,
                &IpAkeHeader_ops,
                &Ctype_Response,
                Cmd,
                CmdDep,
                &Status);
            break;
        }
        default:
            ret = BERR_NOT_SUPPORTED;
            BKNI_Free(buffer);
    }

    return ret;
}

/* \brief exported IP AKE message interface
 */
B_DTCP_AkeMessageInterface_T gIpAkeMessageInterface =
{
    &IpAkeHeader_ops,
    &B_DTCP_IP_AkeCreateCommands,
    &B_DTCP_IP_AkeCreateResponse,
    &B_DTCP_IP_AkeConsumeCommands,
    &B_DTCP_IP_AkeConsumeResponse
};

/* ENd of File */
