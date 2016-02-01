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
 *     DTCP AKE Transport interface header.
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_transport.h
 *  \brief define DTCP AkE transport module interface.
 *
 *  This file defines the inteface between DTCP AKE core module and DTCP Transport module.
 *  The specific implementation, i.e. DTCP over IP or DTCP over USB, etc. should implements
 *  these interfaces and provide a function to initializ the interface pointer.
 */
#ifndef B_DTCP_TRANSPORT_H
#define B_DTCP_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief type of event that the transport interface may report.
 */
typedef enum __trasnport_event
{
    B_Transport_eReady,     /*!< Transport layer is ready for receiving data */
    B_Transport_eDataReady, /*!< Data ready event. */
    B_Transport_eTimeOut,   /*!< Timeout while receiving data.*/
    B_Transport_eClosed,    /*!< Peer closed connection */
    B_Transport_eError,     /*!< Transport layer error.*/
    B_Transport_eUnknown    /*!< unknown event. */
}B_TransportEvent_T;

/* \skipline forward declaration.
 */
struct __dtcp_transport_interface;

typedef struct __dtcp_transport_interface * B_TransportHandle_T;

/*! \brief function pointer to send data, implemented by transport interface
 *  \param[in] Transporter transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] Buffer Buffer holds output data.
 *  \param[in] length Length of the output buffer.
 *  \retval BERR_SUCCESS or BERR_SOCKET_ERROR.
 */
typedef BERR_Code (*B_DTCP_TransportSendFunc_Ptr) (B_TransportHandle_T Transporter,
        B_AkeCoreSessionData_T * Session, unsigned char * buffer, int length);

/*! \brief function to receive data from transport interfce, implemented by transport interface
 *
 *  This should be a non-blocking function, only when the AKE session get a
 *  data ready event, it can call this function to read the data from transport
 *  interface.
 *  \param[in] Transporter transport handle.
 *  \param[in] Session AKE session pointer.
 *  \param[in] Buffer Receiving buffer pointer.
 *  \param[in] length Length  of the receiving buffer.
 *  \retval BERR_SUCCESS or BERR_UNKNOWN.
 */
typedef BERR_Code (*B_DTCP_TransportRecvFunc_Ptr) (B_TransportHandle_T Transporter,
        B_AkeCoreSessionData_T * Session, unsigned char * buffer, int * length);

/*! \brief Utlity to get the available data size.
 *  \param[in] Session AKE session pointer.
 *  \param[in] UserData pointer to user data.
 *  \retval size of the data available for reading.
 */
typedef int (*B_DTCP_TransportGetDataSizeFunc_Ptr)(B_TransportHandle_T Transporter,
        B_AkeCoreSessionData_T * Session, void * UserData);

/*! \brief function to wait for transport layer event.
 *  \param[in] Session AKE session pointer.
 *  \param[in] timeout Function will return after "timeout" value ellipsed (in msecs)
 *  \retval one of the enumeration value defined in B_TransportEvent_T
 */
typedef B_TransportEvent_T (*B_DTCP_TransportWaitForEventFunc_Ptr)(B_TransportHandle_T Transporter,
        B_AkeCoreSessionData_T * Session, int timeout);

/*! \brief transport layer callback function pointer.
 *  \param[in] pStack the stack context pointer.
 *  \param[in] UserData protocol specific user data pointer.
 *  \retval BERR_SUCCESS or other error codes.
 */
typedef BERR_Code (*B_DTCP_TransportCallbackFunc_Ptr)(void * pStack, void * UserData);


/*! \brief function pointer to start/stop a sink session, implmented by transport interface.
 *  \param[in] Session AKE session pointer.
 *  \param[in] ConnectionClosedCallback a stack provided callback function, when transport layer
 *  closed the connection.
 *  \retval BERR_SUCCES or other error code.
 *
 *  The transport layer implements a start sink function make connection to source device,
 *  and also a stop sink function to close the connection to source device.
 */
typedef BERR_Code (*B_DTCP_TransportSinkFunc_Ptr)(B_TransportHandle_T Transporter,
        B_AkeCoreSessionData_T * Session,
        B_DTCP_TransportCallbackFunc_Ptr ConnectionClosedCallback);

/*! \brief function to start a source Transport layer function
 *  \param[in] pStack DTCP stack pointer.
 *  \param[in] Transporter AKE transport handle.
 *  \param[in] UserData pointer to __dtcp_ip_stack_data structure.
 *  \param[in] ConnectionCallback stack provided callback function when there is a new authentication request.
 *
 *  This is a non-blocking function, a listenning thread will be launched.
 *
 */
typedef BERR_Code (* B_DTCP_TransportSourceStartFunc_Ptr)( void * pStack, B_TransportHandle_T Transporter,
        void * UserData,
        B_DTCP_TransportCallbackFunc_Ptr ConnectionCallback);

/*! \brief function to stop transport layer source listenning
 * \param[in] Transporter AKE transport handle.
 */
typedef BERR_Code (* B_DTCP_TransportSourceStopFunc_Ptr)( B_TransportHandle_T Transporter);

/*! \struct __dtcp_transport_interface
 *  \brief Generic Transport interface data structure.
 */
typedef struct __dtcp_transport_interface
{
    /* Exported interface functions */
    B_DTCP_TransportSendFunc_Ptr Send;                          /*!< Send function.*/
    B_DTCP_TransportRecvFunc_Ptr Recv;                          /*!< Recv function. */
    B_DTCP_TransportWaitForEventFunc_Ptr WaitForEvent;          /*!< wait for event function, must be thread-safe */
    B_DTCP_TransportGetDataSizeFunc_Ptr  GetDataSize;           /*!< Get data size function. */
    B_DTCP_TransportSinkFunc_Ptr StartSink;                     /*!< sink start function. */
    B_DTCP_TransportSinkFunc_Ptr StopSink;                      /*!< sink start function. */
    B_DTCP_TransportSourceStartFunc_Ptr StartSource;            /*!> source start function. */
    B_DTCP_TransportSourceStopFunc_Ptr StopSource;              /*!> source start function. */

    void * pProtocolData;                                       /*!< protocol specific data pointer */
    int id;                                                     /*!< stack id */
}B_DTCP_AkeTransportInterface_T;


/*! \brief Init transport interface.
 *  \param[in] StackId stack id, currently only B_StackId_eIP is supported.
 *  \retval Handle to the transport interface or NULL if failed.
 */
B_TransportHandle_T B_DTCP_InitTransportInterface(int StackId);

/* \brief cleanup transport interface data.
 * \param[in] stack_id, stack identifier, currently only B_StackId_eIP is supported.
 */
void B_DTCP_UnInitTransportInterface(int stack_id, B_TransportHandle_T Handle);

#ifdef __cplusplus
}
#endif

#endif /* B_DTCP_TRANSPORT_H */
