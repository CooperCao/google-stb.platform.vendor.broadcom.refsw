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
 *    DTCP-IP transport interface
*********************************************************************************************/
/*! \file b_dtcp_ip_transport.c
 *  \brief implement DTCP-IP trasnport interface functions.
 */
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_status_codes.h"
#include "b_dtcp_transport.h"
#include "b_dtcp_ip_transport_priv.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_ip_stack.h"

BDBG_MODULE(b_dtcp_ip);

/*! \brief function to send data.
 *  \param[in] Transporter Transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] Buffer Buffer holds output data.
 *  \param[in] length Length of the output buffer.
 *  \retval BERR_SUCCESS or BERR_SOCKET_ERROR.
 */
BERR_Code B_DTCP_IP_AkeSend(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int length)
{
    B_DTCP_IP_Connection_T * Conn;
    BERR_Code retValue = BERR_NOT_FOUND;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    BDBG_ASSERT(Session);
    BDBG_ASSERT(pProtocol);

    BDBG_MSG(("SessionId=%d\n", Session->SessionId));
  B_Mutex_Lock(pProtocol->ConnectionChgMutex);
    Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    while(Conn != NULL)
    {
        if(Conn->ConnectSocket == Session->SessionId)
        {
            BDBG_MSG(("Sending data, len = %d\n", length));
            if(B_SocketSendData(Conn->ConnectSocket, buffer, length) != length)
            {
                retValue = BERR_SOCKET_ERROR;
                break;
            }else {
                retValue = BERR_SUCCESS;
                break;
            }
        }
        Conn = BLST_S_NEXT(Conn, node);
    }

  B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    return retValue;
}
/*! \brief function to receive data from transport interfce.
 *
 *  This should be a non-blocking function, only when the AKE session get a
 *  data ready event, it can call this function to read the data from transport
 *  interface.
 *  \param[in] Transporter Transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] buffer Receiving buffer pointer.
 *  \param[in,out] length received data length.
 *  \retval BERR_SUCCESS or BERR_UNKNOWN.
 */
BERR_Code B_DTCP_IP_AkeRecv(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int * length)
{
    B_DTCP_IP_Connection_T * Conn;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    BDBG_ASSERT(Session);
    BDBG_ASSERT(pProtocol);

  B_Mutex_Lock(pProtocol->ConnectionChgMutex);
    Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    while(Conn != NULL){
        if(Conn->ConnectSocket == Session->SessionId)
        {
            *length = B_SocketRecvData(Conn->ConnectSocket, buffer, *length);
            B_Event_Set(Conn->hBufCleared);
      B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
      return BERR_SUCCESS;
        }
        Conn = BLST_S_NEXT(Conn, node);
    };
  B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    return BERR_TRACE(BERR_NOT_FOUND);
}
/*! \brief Utlity to get the available data size.
 *  \param[in] Transporter Transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] UserData pointer to user data.
 *  \retval size of the data available for reading.
 */
int B_DTCP_IP_AkeGetDataSize(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session, void * user)
{
    B_DTCP_IP_Connection_T * Conn;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    BDBG_ASSERT(Session);
    BDBG_ASSERT(pProtocol);

    (void)user;

  B_Mutex_Lock(pProtocol->ConnectionChgMutex);
  Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    while(Conn != NULL){
        if(Conn->ConnectSocket == Session->SessionId)
    {
      B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
            return B_DTCP_IP_GetDataSize_Ex(Conn);
    }
        Conn = BLST_S_NEXT(Conn, node);
    };
  B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    return 0;
}
/*! \brief function to wait for network event.
 *  \param[in] Transporter Transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] timeout Function will return after "timeout" value ellipsed (in msecs)
 *  \retval one of the enumeration value defined in B_TransportEvent_T
 */
B_TransportEvent_T B_DTCP_IP_AkeWaitForEvent(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session, int Timeout)
{
    B_DTCP_IP_Connection_T * Conn;
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(pProtocol);

    BDBG_ENTER(B_DTCP_IP_AkeWaitForEvent);
  B_Mutex_Lock(pProtocol->ConnectionChgMutex);
    Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    while(Conn != NULL){
        if(Conn->ConnectSocket == Session->SessionId)
        {
            BDBG_MSG(("Waiting for net event on socket %d..\n", Conn->ConnectSocket));
            retValue = B_Event_Wait(Conn->hNetEvent, Timeout);
            if(retValue == BERR_TIMEOUT)
            {
                retValue =  B_Transport_eTimeOut;
                break;
            }else if(retValue == BERR_OS_ERROR)
            {
                retValue =  B_Transport_eUnknown;
                break;
            }else
            {
                retValue =  B_DTCP_IP_GetEventType_Ex(Conn);
                if(retValue == B_Transport_eDataReady)
                {
                    B_Event_Reset(Conn->hNetEvent);
                }
                break;
            }

        }
        B_Mutex_Lock(pProtocol->ConnectionChgMutex);
        Conn = BLST_S_NEXT(Conn, node);
        B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    }
    BDBG_MSG(("Got net event: %d\n", retValue));
    BDBG_LEAVE(B_DTCP_IP_AkeWaitForEvent);
    return retValue;
}
/*! \brief function pointer to start a sink session.
 *  \param[in] AKE transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] ConnectionClosedCallback a stack provided callback function, when transport layer
 *  closed the connection.
 *  \retval BERR_SUCCES or other error code.
 *
 *  The transport layer make connection to source device,
 */
BERR_Code B_DTCP_IP_AkeTransportSinkStart(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session,
        B_DTCP_TransportCallbackFunc_Ptr ConnectionClosedCallback)
{
    B_IpAkeSessionData_T * IpAkeSession = NULL;
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    BDBG_ENTER(B_DTCP_IP_AkeTransportSinkStart);

    BDBG_ASSERT(Session);
    IpAkeSession = (B_IpAkeSessionData_T *)(Session->pProtocolData);
    BDBG_ASSERT(IpAkeSession);
    BDBG_MSG(("remoteip=%s remoteport=%d\n", IpAkeSession->RemoteIp, IpAkeSession->RemotePort));

    retValue = B_DTCP_IP_OpenConnection(
            B_ConnectionMode_eClient,
            NULL,
            IpAkeSession->RemoteIp,
            IpAkeSession->RemotePort,
            NULL,
            ConnectionClosedCallback,
            pProtocol,
            &Session->SessionId);
    BDBG_LEAVE(B_DTCP_IP_AkeTransportSinkStart);
    return retValue;
}
/*! \brief function pointer to stop a sink session.
 *  \param[in] AKE transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] ConnectionClosedCallback a stack provided callback function, when transport layer
 *  closed the connection.(not used for this function, since closing is initiated by stack layer.)
 *  \retval BERR_SUCCES or other error code.
 */

BERR_Code B_DTCP_IP_AkeTransportSinkStop(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session,
        B_DTCP_TransportCallbackFunc_Ptr ConnectionClosedCallback)
{
    BERR_Code retValue = BERR_NOT_FOUND;
    B_IpAkeSessionData_T * IpAkeSession = NULL;
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    B_DTCP_IP_Connection_T * Conn;

    BDBG_ASSERT(Session);
    IpAkeSession = (B_IpAkeSessionData_T *)(Session->pProtocolData);
    BDBG_ASSERT(IpAkeSession);

    BDBG_ENTER(B_DTCP_IP_AkeTransportSinkStop);

    B_Mutex_Lock(pProtocol->ConnectionChgMutex);
    Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    while(Conn != NULL){
        if(Conn->ConnectSocket == Session->SessionId)
        {
            BDBG_MSG(("Closing connection: remoteip=%s remoteport=%d\n", IpAkeSession->RemoteIp,
                        IpAkeSession->RemotePort));
            B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
            retValue = B_DTCP_IP_CloseConnection(Conn, pProtocol);
            B_Mutex_Lock(pProtocol->ConnectionChgMutex);
            (void)ConnectionClosedCallback;
            break;
        }
        Conn = BLST_S_NEXT(Conn, node);
    }

    if(retValue == BERR_NOT_FOUND) {
        BDBG_ERR(("%s: Couldn't find specified connection with ID: %d \n", __FUNCTION__, Session->SessionId));
    }

    B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    BDBG_LEAVE(B_DTCP_IP_AkeTransportSinkStop);
    return retValue;

}

bool B_DTCP_IP_AkeTransportSinkStopped(B_TransportHandle_T Transporter, B_AkeCoreSessionData_T * Session)
{
    B_DTCP_IP_TransportData_T * pProtocol = Transporter->pProtocolData;
    B_DTCP_IP_Connection_T * Conn;

  B_Mutex_Lock(pProtocol->ConnectionChgMutex);

    Conn = BLST_S_FIRST(&(pProtocol->Connection_list));
    while(Conn != NULL){
        if(Conn->ConnectSocket == Session->SessionId)
        {
      B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
            return false;
        }
        Conn = BLST_S_NEXT(Conn, node);
    }
  B_Mutex_Unlock(pProtocol->ConnectionChgMutex);
    return true;
}

/*! \brief help function to start a source listenning thread.
 *  \param[in] Param pointer to DTCP_IP transport layer protocol specific data.
 *  \retval none.
 */
static void B_DTCP_IP_SourceThreadFunc(void * Param)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_TransportData_T * pProtocol = (B_DTCP_IP_TransportData_T *)Param;

    BDBG_MSG(("Starting source listen thread...on %s: %d \n", pProtocol->LocalIp, pProtocol->LocalPort));

    /*
     * This will block, untill listening socket is closed.
     */
    retValue = B_WaitForConnection(pProtocol->MasterConn, pProtocol);

    BDBG_MSG(("Listening thread terminated with %d\n", retValue));
}
/*! \brief function to start a source Transport layer function
 *  \param[in] Transporter AKE transport handle.
 *  \param[in] UserData pointer to __dtcp_ip_stack_data structure.
 *  \param[in] ConnectionCallback stack provided callback function when there is a new authentication request.
 *
 *  This is a non-blocking function, a listenning thread will be launched.
 *
 */
BERR_Code B_DTCP_IP_AkeTransportSourceStart( void * pStack, B_TransportHandle_T Transporter, void * UserData,
    B_DTCP_TransportCallbackFunc_Ptr ConnectionCallback)
{
    BERR_Code retValue = BERR_SUCCESS;
    int ConnectionId;
    B_DTCP_IP_TransportData_T * pProtocol = (B_DTCP_IP_TransportData_T *)(Transporter->pProtocolData);
    struct __dtcp_ip_stack_data * sdata = (struct __dtcp_ip_stack_data *)UserData;

    BDBG_ASSERT(ConnectionCallback);
    BDBG_ASSERT(UserData);
    BDBG_ASSERT(Transporter);

    BDBG_ENTER(B_DTCP_IP_AkeTransportSourceStart);

    BKNI_Memcpy(pProtocol->LocalIp, sdata->LocalIp, MAX_IP_ADDR_SIZE);
    pProtocol->LocalPort = sdata->LocalPort;
    pProtocol->ConnCallback = ConnectionCallback;
    pProtocol->pStack = pStack;

    retValue =  B_DTCP_IP_OpenConnection(
         B_ConnectionMode_eServer,
         pProtocol->LocalIp,
         NULL,
         pProtocol->LocalPort,
         pProtocol->ConnCallback,
         NULL,
         pProtocol,
         &ConnectionId);
    if (retValue != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to Open listener connection for server\n"));
        return retValue;
    }

    /*
     * Spawn a new thread to listen to incoming connection.
     */
    pProtocol->hListenThread = B_Thread_Create(
            "ListenThread",
            B_DTCP_IP_SourceThreadFunc,
            (void*)pProtocol,
            NULL);
    if(pProtocol->hListenThread == NULL)
    {
        BDBG_ERR(("Failed to create listen thread \n"));
        retValue = BERR_OS_ERROR;
    }

    BDBG_LEAVE(B_DTCP_IP_AkeTransportSourceStart);
    return retValue;

}
/*! \brief function to stop transport layer source listenning
 * \param[in] Transporter AKE transport handle.
 */
BERR_Code B_DTCP_IP_AkeTransportSourceStop(B_TransportHandle_T Transporter)
{
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_TransportData_T * pProtocol;
    BDBG_ASSERT(Transporter);
    pProtocol = (B_DTCP_IP_TransportData_T *)(Transporter->pProtocolData);
    /*
     * Close master connection.
     */
    pProtocol->masterShutDown = true;
    retValue = B_DTCP_IP_CloseConnection(pProtocol->MasterConn, pProtocol);
    if (retValue != BERR_SUCCESS)
    {
        BDBG_WRN(("***** Source unable to Close Connection. May leak resources *******"));
    }
    else
    {
        B_Thread_Destroy(pProtocol->hListenThread);
    }

    return retValue;

}
/*! \brief initialize transport interface.
 *  \param[in] stack_id stack identifier, currently only B_StackId_eIP is suppored.
 *  \retval transporter interface handle.
 */
B_TransportHandle_T B_DTCP_InitTransportInterface(int stack_id)
{
    B_DTCP_AkeTransportInterface_T * AkeIpTransporter;
    B_DTCP_IP_TransportData_T * pProtocol;
    B_MutexHandle tmpMutexHandle;

    if(stack_id != B_StackId_eIP)
    {
        BDBG_ERR(("%s Stackid: %d is not supported!\n", __FUNCTION__, stack_id));
        return NULL;
    }

    if((AkeIpTransporter = BKNI_Malloc(sizeof(B_DTCP_AkeTransportInterface_T))) == NULL)
    {
        BDBG_ERR(("%s: Failed to allocated memory\n", __FUNCTION__));
        return NULL;
    }
    BKNI_Memset(AkeIpTransporter, 0, sizeof(B_DTCP_AkeTransportInterface_T));
    pProtocol = (B_DTCP_IP_TransportData_T *)BKNI_Malloc(sizeof(B_DTCP_IP_TransportData_T));
    if(pProtocol == NULL)
    {
        BDBG_ERR(("%s: Failed to allocate memory for protocol\n", __FUNCTION__));
        BKNI_Free(AkeIpTransporter);
        return NULL;
    }

    tmpMutexHandle = B_Mutex_Create(NULL);
    if (tmpMutexHandle == NULL)
    {
        BDBG_ERR(("%s B_Mutex_Create failed\n", __FUNCTION__));
        return NULL;
    }
    BKNI_Memset(pProtocol, 0, sizeof(B_DTCP_IP_TransportData_T));

    B_Mutex_Lock(tmpMutexHandle);
    BLST_S_INIT(&(pProtocol->Connection_list));
    B_Mutex_Unlock(tmpMutexHandle);
    pProtocol->MasterConn = NULL;
    pProtocol->ConnCallback = NULL;
    pProtocol->ConnectionChgMutex = tmpMutexHandle;
    AkeIpTransporter->pProtocolData = pProtocol;
    AkeIpTransporter->id = stack_id;
    AkeIpTransporter->Send = &B_DTCP_IP_AkeSend;
    AkeIpTransporter->Recv = &B_DTCP_IP_AkeRecv;
    AkeIpTransporter->WaitForEvent = &B_DTCP_IP_AkeWaitForEvent;
    AkeIpTransporter->GetDataSize =  &B_DTCP_IP_AkeGetDataSize;
    AkeIpTransporter->StartSink = &B_DTCP_IP_AkeTransportSinkStart;
    AkeIpTransporter->StopSink = &B_DTCP_IP_AkeTransportSinkStop;
    AkeIpTransporter->StartSource = &B_DTCP_IP_AkeTransportSourceStart;
    AkeIpTransporter->StopSource = &B_DTCP_IP_AkeTransportSourceStop;

    return AkeIpTransporter;
}
/* \brief cleanup transport interface data.
 * \param[in] stack_id, stack identifier, currently only B_StackId_eIP is supported.
 */
void B_DTCP_UnInitTransportInterface(int stack_id, B_TransportHandle_T Transporter)
{
    B_DTCP_IP_TransportData_T * pProtocol;
    if(stack_id != B_StackId_eIP)
    {
        BDBG_ERR(("%s: The stack: %d is not supported\n", __FUNCTION__, stack_id));
        return;
    }
    pProtocol = (B_DTCP_IP_TransportData_T *)(Transporter->pProtocolData);
    if(!BLST_S_EMPTY(&(pProtocol->Connection_list)) )
    {
        BDBG_WRN(("%s: Not all connections are closed!\n", __FUNCTION__));
    }

  B_Mutex_Destroy(pProtocol->ConnectionChgMutex);
    BKNI_Free(pProtocol);
    BKNI_Free(Transporter);
}

/* END OF FILE */
