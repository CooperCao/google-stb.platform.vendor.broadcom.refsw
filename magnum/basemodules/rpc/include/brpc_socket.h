/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 *********************************************************************/
#ifndef __BRPC_SOCKET_H
#define __BRPC_SOCKET_H

/*====================== Module Overview ===========================

---++++ *Summary:*

This is one type of implementation of the BRPC API using sockets.
Use these functions to create and destroy the BRPC_Handle, then pass the handle
to other rmagnum modules.

---++++ *Requirements:*
1) It must open and close a BRPC_Handle that can be used with brpc.h.
2) It must provide some notification to system software that a message is ready
   to be read.

***************************************************************************/

/***************************************************************************
Summary:
	Structure for RPC  socket implementation settings
****************************************************************************/
typedef struct BRPC_OpenSocketImplSettings {
	uint32_t host;                              /* host name */
	uint32_t local;				    /* local ip address */
	unsigned short port_req;                    /* request port number */
	unsigned short port_ack;                    /* ack Port number*/
	unsigned timeout;                           /* timeout period to waitfor ack */
	bool rpc_disabled;                          /* if RPC is disabled*/
} BRPC_OpenSocketImplSettings;

/***************************************************************************
Summary:
	This function return default settings

Description:
	This function is responsible for providing the default values.

Returns:
    A copy of the default settings and the status of the request.

See Also:
	BRPC_Open_SocketImpl().
****************************************************************************/
void BRPC_GetDefaultOpenSocketImplSettings(
	BRPC_OpenSocketImplSettings *settings
	);

/***************************************************************************
Summary:
	This function opens and creats the RPC handle.

Description:
	This function is responsible for creating RPC handle.
	This function will open neccessary sockets and allocate buffers.

Returns:
    RPC handle

See Also:
	BRPC_Close_SocketImpl().
****************************************************************************/
BERR_Code BRPC_Open_SocketImpl(
	BRPC_Handle *handle,                            /* [out] RPC handle*/
	const BRPC_OpenSocketImplSettings *Settings     /* [in] Default settings*/
	);

/***************************************************************************
Summary:
	This function closes a previously opened RPC handle.

Description:
	This function is responsible for releasing a previously opened RPC handle.
	This function will close sockets and free pre allocated buffers.
    This is used th undo the actions of the open.

Returns:
    The status of the operation.

See Also:
	BRPC_Open_SocketImpl().
****************************************************************************/
void BRPC_Close_SocketImpl(
	BRPC_Handle handle                            /* [out] RPC handle*/
	);


/***************************************************************************
Summary:
	This function returns socket id for notification

Description:
	This function returns socket id for notification, which is used by poll/select
	to check notification.

Returns:
    The socket id for notification.

See Also:
	BRPC_Open_SocketImpl().
****************************************************************************/
BERR_Code BRPC_GetSocketDescriptor_SocketImpl(
	BRPC_Handle handle,
	int *fd                   /* [out] socket id for notification from remote*/
	);

#endif

