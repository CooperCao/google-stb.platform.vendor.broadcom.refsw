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
 *     AKE message interface header.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake_msg.h
 *  \brief define Base DTCP AKE Message interface data types and function prototypes.
 */
#ifndef B_DTCP_AKE_MSG_H
#define B_DTCP_AKE_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*B_AkeHeader_SET)(unsigned char * buffer, int value);
typedef void (*B_AkeHeader_GET)(unsigned char * buffer, int * value);
typedef void (*B_AkeHeader_INIT)(unsigned char * buffer, unsigned char label);

/*! \struct B_AkeHeader_ops
 *  \brief Ake header initialization and operation routines.
 *
 *  This is implmented in protocol dependent message interface file.
 */
typedef struct B_AkeHeader_ops
{
    B_AkeHeader_INIT Init;
    B_AkeHeader_SET  SetLength;
    B_AkeHeader_GET  GetLength;
    B_AkeHeader_GET  GetLabel;
    B_AkeHeader_SET  SetCtypeResponse;
    B_AkeHeader_GET  GetCtypeResponse;
    B_AkeHeader_SET  SetCommand;
    B_AkeHeader_GET  GetCommand;
    B_AkeHeader_SET  SetAkeType;
    B_AkeHeader_GET  GetAkeType;
    B_AkeHeader_SET  SetExchKeyType;
    B_AkeHeader_GET  GetExchKeyType;
    B_AkeHeader_SET  SetCmdDependent;
    B_AkeHeader_GET  GetCmdDependent;
    B_AkeHeader_SET  SetStatus;
    B_AkeHeader_GET  GetStatus;
}B_AkeHeader_ops_T;

/*! \brief function pointer to create AKE command messages, including AkE base messages and implementation specifc messages.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AKE session data.
 *  \param[in] Cmd AKE command being sent.
 */
typedef BERR_Code (*B_DTCP_AkeCmdProducer_Ptr)(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        int Cmd);
/*! \brief function pointer to create AKE command response, including AKE base message and implementation specific messages.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AKE session data.
 *  \param[in] Cmd Ake command that this response is responding to.
 *  \param[in] Response Response code.
 */
typedef BERR_Code (*B_DTCP_AkeResponseProducer_Ptr)(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        int Cmd,
        int CmdDep,
        int Response,
        int Status);

/*! \brief function pointer to consume AKE command message, including AKE base messages and protocol specific messages.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AKE session data.
 *  \param[in,out] buffer pointer to command buffer pointer, after command consumed, *buffer = NULL.
 *  \param[in] length message length.
 *  \param[out] Cmd received command code.
 *  \param[out] CmdDep received command dependent value.
 */
typedef BERR_Code (*B_DTCP_AkeCmdConsumer_Ptr)(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int length,
        int *Cmd,
        int *CmdDep);
/*! \brief function pointer to consume a response message.
 *  \param[in,out] Session AKE session data.
 *  \param[in,out] buffer pointer to command buffer pointer, after command consumed, *buffer = NULL.
 *  \param[in] length message length.
 *  \param[out] Cmd the command code that this response is responding to .
 *  \param[out] Response the response code.
 *  \param[out] Status status code.
 */
typedef BERR_Code (*B_DTCP_AkeResponseConsumer_Ptr)(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int length,
        int * Cmd,
        int * Response,
        int * Status);

/*! \brief DTCP Ake messaging interface.
 *
 * The protocol specific file (e.g. DTCP-IP DTCP-USB, etc) implements these functions, and provides
 * GetMessageInterface() function to return the message interface object.
 */
typedef struct B_DTCP_AkeMessageInterface
{
    B_AkeHeader_ops_T * hops;                           /*!< Header operation pointer.*/
    B_DTCP_AkeCmdProducer_Ptr CreateCommand_Func;       /*!< AKE command producer function pointer. */
    B_DTCP_AkeResponseProducer_Ptr CreateResponse_Func; /*!< AKE response producer function pointer */
    B_DTCP_AkeCmdConsumer_Ptr ConsumeCommand_Func;      /*!< AKE command consumer function pointer */
    B_DTCP_AkeResponseConsumer_Ptr ConsumeResponse_Func;    /*!< AKE response consumer functionpointer */
}B_DTCP_AkeMessageInterface_T;

/*! \brief exported function, Initialize the message interface pointer based on stack type.
 *  \param[in] StackId the stack ID, see b_dtcp_types.h for details.
 *  \retval pointer to DTCP message interface pointer.
 */
B_DTCP_AkeMessageInterface_T * B_DTCP_InitMessageInterface(B_StackId_T StackId);
/*! \brief cleanup message interface, free allocated resources.
 *  \param[in] pMessenger. pointer to messange interface.
 */
void B_DTCP_UnInitMessageInterface(B_StackId_T StackId, B_DTCP_AkeMessageInterface_T * pMessenger);


/* prototyes implmented in AKE base message interface file*/
BERR_Code B_DTCP_CreateMsgNoData(B_AkeCoreSessionData_T * Session, int headroom, B_AkeHeader_ops_T * hops,
        int Ctype_Response,
        int Cmd,
        int CmdDep,
        int Status);
BERR_Code B_DTCP_ConsumeMsgNoData(B_AkeCoreSessionData_T * Session, unsigned char * buffer,
        B_AkeHeader_ops_T * hops,
        int * Ctype_Response,
        int * Cmd,
        int * CmdDep,
        int * Status);

BERR_Code B_DTCP_CreateStatusCmd(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_CreateChallenge(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_ConsumeChallenge(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

BERR_Code B_DTCP_CreateResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_ConsumeResponse(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

BERR_Code B_DTCP_CreateExchangeKey(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_ConsumeExchangeKey(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

BERR_Code B_DTCP_CreateSrm(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_ConsumeSrm(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

BERR_Code B_DTCP_CreateAkeCancel(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);


BERR_Code B_DTCP_CreateContentKeyReq(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_CreateContentKeyReqResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops, int Response, int Status);

BERR_Code B_DTCP_ConsumeContKeyReqResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int * Cmd, int * Response, int * Status, int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_CreateResponse2(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_ConsumeResponse2(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

BERR_Code B_DTCP_CreateCapabilityReq(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_CreateCapabilityReqResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops);

BERR_Code B_DTCP_CreateCapabilityExchangeMsg(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T  *hops);
BERR_Code B_DTCP_ConsumeCapabilityExchangeMsg(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom);

extern B_DTCP_AkeMessageInterface_T gIpAkeMessageInterface;

/*! \brief debugging function, dump out AKE header.
 *  \param[in] data data buffer.
 *  \param[out] buffer buffer for decoded AKE message.
 *  \param[in]  BufferLength length of buffer.
 *  \retval none.
 */
void B_DumpHeader(unsigned char * data, char * buffer, int BufferLength);

#ifdef __cplusplus
}
#endif
#endif /* B_DTCP_AKE_MSG_H */
