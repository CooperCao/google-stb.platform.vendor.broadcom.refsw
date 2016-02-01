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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
*********************************************************************************************/
/*! \file b_dtcp_stack.h
 *  \brief Define DTCP base stack data structure.
 */
#ifndef B_DTCP_STACK_H
#define B_DTCP_STACK_H

#include "b_dtcp_ake_msg.h"
#include "b_dtcp_transport.h"

#ifdef __cplusplus
 extern "C" {
#endif

/*! \skipline forward declaration
 */

struct __b_dtcp_stack;
typedef struct __b_dtcp_stack * B_DTCP_StackHandle_T;

/*! \brief function pointer to start a sink device, to be implemented by protocol specific stack layer
 *  \param[in] pStack The stack handle.
 *  \param[in] pData procotol specified data pointer.
 *  \param[out] AkeHandle result AKE session handle pointer, if AKE succeeded, dereferencing this pointer to obtain handle.
 *  \retval BERR_SUCCESS or other error code.
 */
typedef BERR_Code (* B_DTCP_AkeStartSink_Ptr)(B_DTCP_StackHandle_T pStack, void * pData, B_AkeHandle_T * AkeHandle);

/*! \brief function pointer to stop sink function and close AKE session.
 *  \param[in] pStack The stack handle.
 *  \param[in] AkeHandle Ake handle obtained from AkeStartSink function.
 *  \retval BERR_SUCCESS or other error code.
 */
typedef BERR_Code (* B_DTCP_AkeStopSink_Ptr)(B_DTCP_StackHandle_T pStack, B_AkeHandle_T AkeHandle);

/*! \brief function pointer to start a source device, to be implemented by protocol specific stack layer.
 *  \param[in] pStack The stack handle.
 *  \param[in] pData protocol specified data pointer.
 *  \retval BERR_SUCCESS or other error code
 */
typedef BERR_Code (* B_DTCP_AkeStartSource_Ptr)(B_DTCP_StackHandle_T pStack, void * pData);

/*! \brief function pointer to stop source function.
 *  \param[in] pStack The stack handle.
 *  \retval BERR_SUCCESS or other error code.
 */
typedef BERR_Code (* B_DTCP_AkeStopSource_Ptr)(B_DTCP_StackHandle_T pStack);

/*! \brief function pointer to update SRM
 *  \param[in] pSrm pointer to SRM message buffer.
 *  \param[in] aSrmSize size of the SRM message.
 *  \retval implementation defined.
 * The method of update/store SRM is implementation dependent, The high level application software should implemnent this function.
 */
typedef int (*B_DTCP_UpdateSRM_Ptr)(unsigned char * pSrm, unsigned int aSrmSize);

/*! \brief function pointer for internal error handling
 *  \param[in] pStack The stack handle.
 *  \param[in] AkeHandle current AKE session Handle.
 */
typedef void (* B_DTCP_OnInternalError_Ptr)(B_DTCP_StackHandle_T pStack, B_AkeHandle_T  AkeHandle);

/*! \brief function pointer to verify current exchange key is valid. for sink device only.
 *  \param[in] pStack The stack handle.
 *  \param[in] AkeHandle current Ake session handle.
 *  \param[in,out] Valid boolean variable indicate if the session's exchange key is valid or not.
 *  \retval BERR_SUCCESS or error code.
 */
typedef BERR_Code (* B_DTCP_VerifyExchKey)(B_DTCP_StackHandle_T pStack, B_AkeHandle_T AkeHandle, bool * Valid);

/*! \struct __b_dtcp_stack
 *  \brief DTCP stack control data.
 *
 *  The DTCP base stack state machine is implemented around this data structure
 */
typedef struct __b_dtcp_stack
{
    B_AkeCoreData_T * pAkeCoreData;                     /*!< DTCP Ake core data pointer */
    B_DeviceParams_T * DeviceParams;                    /*!< device's parameters. */
    B_DTCP_AkeMessageInterface_T * Messenger;           /*!< AKE message interface pointer */
    B_DTCP_AkeTransportInterface_T * Transporter;       /*!< AKE transport interface pointer */

    /* Functions to be implemented by specific protocol.*/
    B_DTCP_AkeStartSink_Ptr StartSink_Func;             /*!< function pointer to start sink device.*/
    B_DTCP_AkeStopSink_Ptr  StopSink_Func;              /*!< function pointer to perform AKE procedure for sink */
    B_DTCP_AkeStartSource_Ptr StartSource_Func;         /*!< function pointer to start a source device */
    B_DTCP_AkeStopSource_Ptr StopSource_Func;
    B_DTCP_UpdateSRM_Ptr UpdateSrm_Func;                /*!< function pointer to udpate SRM if needed */
    B_DTCP_OnInternalError_Ptr OnInternalError;         /*!< Internal error handling function pointer */
    B_DTCP_VerifyExchKey VerifyExchKey_Func;            /*!> function poniter to verify session's exchange key */
    B_StackId_T StackId;


    void * pProtocolData;                               /*!< protocol specific data. */
    B_DTCP_KeyFormat_T key_format;
}B_DTCP_Stack_T;

/* --------------------------DTCP base stack implemented function prototypes--------------------------- */

/*! \brief initialize DTCP stack based on stack id, (IP, USB, 1394, etc).
 *  \param[in] DeviceParams device parameter pointer.
 *  \param[in] UpdateSrm_Func application provided update SRM function pointer.
 *  \param[in] StackId id specify which stack to initialize, currently only DTCP_STACK_IP is supported.
 *  \retval A handle to the stack if success, or NULL if failed.
 */
B_DTCP_StackHandle_T B_DTCP_Stack_Init(B_DeviceParams_T * DeviceParams,
        B_DTCP_UpdateSRM_Ptr UpdateSrm_Func, B_StackId_T StackId);

/*! \brief function pointer to cleanup dtcp stack
 *  \param[in] pStack The stack handle.
 *  \retval none.
 */
void B_DTCP_Stack_UnInit(B_DTCP_StackHandle_T pStack);

/*! \brief base stack implemented utility function, send AKE command and wait for response
 *  \param[in] pStack The stack handle.
 *  \param[in] Session AKE session data pointer.
 *  \param[in] NoResponseData if true, the response frame has no data field, so the API only read AKE message header size bytes of data.
 *  if false, the API will read all available data for reponse frame from socket.
 *  \param[in] TimeOut time to wait (in msec) before receiving response from other device.
 *  \param[in,out] Cmd command to send, after received response, it will store the received command of response message.
 *  \param[out] Response received response code.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_AkeSendCmdWaitForResponse(B_DTCP_StackHandle_T pStack, B_AkeCoreSessionData_T * Session, bool NoResponseData,
        int TimeOut, int * Cmd, int * Response);

/*!\brief base stack implemented utility function, wait for , receive and process AKE commands.
 * \param[in] pStack The stack handle.
 * \param[in] Session AKE session data pointer.
 * \param[in] TimeOut time to wait (in msec) before receiving response from other device.
 * \param[out] Cmd received command from other device.
 * \retval BERR_SUCCESS or other error code.
 *
 * This function will block waiting for specified TimeOut value, if it didn't receive command during this period,
 * it will return with BERR_TIMEOUT.
 */
BERR_Code B_DTCP_AkeReceiveCmd(B_DTCP_StackHandle_T pStack, B_AkeCoreSessionData_T * Session, int TimeOut, int * Cmd);
/*! \brief send capability exchange command.
 *
 *  \param[in] Session current AKE session pointer.
 *  \param[in] Stack DTCP stack pointer.
 */
BERR_Code B_DTCP_SendCapabilityExchangeCmd(B_DTCP_Stack_T * Stack, B_AkeCoreSessionData_T * Session);

/*! \brief Send a AKE challenge command.
 *  \param[in] pStack The stack handle.
 *  \param[in] pStack The stack handle.
 *  \param[in] Session pointer to AKE session data.
 *  \retval BERR_SUCCESS or other error code.
 *
 *  protocol specific stack layer use this function to start a sink device AKE session.
 */
BERR_Code B_DTCP_SendChallengeCmd(B_DTCP_StackHandle_T pStack, B_AkeCoreSessionData_T * Session);

/*! \brief process base AKE commands.
 *  \param[in] pStack the stack handle.
 *  \param[in] Stack DTCP base stack pointer.
 *  \param[in] Session AKE session data pointer.
 *  \param[in] Cmd command to be processed.
 *  \retval BERR_SUCCESS or other error code.
 *
 *  This function only process the base AKE command (defined in b_dtcp_ake.h), protocol dependent commands are processed in
 *  seperate function implmented in protocol specific stack layer.
 */

BERR_Code B_DTCP_ProcessAkeCommands(B_DTCP_StackHandle_T pStack, B_AkeCoreSessionData_T * Session,  int Cmd);

void DtcpAppLib_UnSetDtcpHci(B_DTCP_StackHandle_T pStack, B_DeviceParams_T *aDeviceParams);
int DtcpAppLib_SetDtcpHci(B_DTCP_StackHandle_T pStack, B_DeviceParams_T *aDeviceParams);
#ifdef __cplusplus
}
#endif
#endif /* B_DTCP_STACK_H */
