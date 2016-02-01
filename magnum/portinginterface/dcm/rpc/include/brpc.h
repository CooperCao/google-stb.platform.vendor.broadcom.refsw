/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
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

#ifndef _BRPC_H_
#define _BRPC_H_

/*= Module Overview *********************************************************
<verbatim>

Overview:
RPC module provides similar RPC-styled communication between two Broadcom chips.
The purpose of this module is to realize Rmagnum PI modules (BADS, BTNR_3255ib,
BTNR_3255ob, BAOB, BAUS). It is portable across all software platforms which
Broadcom uses.

Two methods to get notifications from remote:
1) Get the socket file descriptor and use poll/select in your system software.
   This requires a call back into BRPC to process the notification.
2) Manually poll the BRPC interface.

At present, this module is implemented on sockets and only supports Linux
user and kernel mode. Other OS and non-OS supports will be added later.

Interrupt Requirements
No interrrupt processing is done by this module.


Sample Code:
//
// NOTE: The following sample code does not do any error checking.
//
// It manually polls the BRPC interface for notification.

#include "brpc.h"
#include "brpc_docsis.h"
#include "brpc_socket.h"

static BRPC_Handle rpc_handle;
static bool check_rpc_enabled = true;

static void b_check_rpc()
{
	uint32_t device_id, event;

	while (check_rpc_enabled)
	{
		BRPC_CheckNotification(rpc_handle,  &device_id, &event, 0);
		if ((event>>16)) {
			BDBG_MSG(("check_rpc(): notified by server (device_id = %08x) event is %x\n", device_id, event));
		} else {
			BKNI_Sleep(100);
		}
	}
}

void main( void )
{
	BRPC_OpenSocketImplSettings socketSettings;
	BERR_Code rc, retVal;
	BRPC_Param_ADS_GetVersion outVerParam;
	BRPC_Param_XXX_Get Param;

	// open RPC handle
	BRPC_GetDefaultOpenSocketImplSettings(&socketSettings);
	BRPC_Open_SocketImpl(&rpc_handle, &socketSettings);

	// create RPC polling notification thread
	....

	// calling  RPC
	BRPC_CallProc(rpc_handle, BRPC_ProcId_ADS_GetVersion, (const uint32_t *)&Param, sizeInLong(Param), (uint32_t *)&outVerParam, sizeInLong(outVerParam), &retVal);

	// destroy RPC polling notification thread
	check_rpc_enabled = false;
	....

	// close RPC handle
	BRPC_Close_SocketImpl(rpc_handle);
}
</verbatim>
***************************************************************************/

#include "berr_ids.h"

/***************************************************************************
Summary:
	Handle used by RPC funtions. It is created by implementation-specific
	Open/Close calls.
****************************************************************************/
typedef struct BRPC_P_Handle *BRPC_Handle;


/***************************************************************************
Summary:
	Error Codes specific to BRPC

Description:
    These define the set of error codes unique to the RPC functions.
    This list does not limit the returned error codes
    (also uses general errors defined elsewhere).

****************************************************************************/
#define BRPC_ERR_UNKNOWN               BERR_MAKE_CODE(BERR_RPC_ID, 0)
#define BRPC_ERR_OUT_OF_MEMORY         BERR_MAKE_CODE(BERR_RPC_ID, 1)
#define BRPC_ERR_OPEN                  BERR_MAKE_CODE(BERR_RPC_ID, 2)
#define BRPC_ERR_NOTIFICATION          BERR_MAKE_CODE(BERR_RPC_ID, 3)


/***************************************************************************
Summary:
	This function realizes remote procedure call.

Description:
    It sends remote procedure ID and data to remote and receives the return data
    and status from remote.
    The return code of this function is for RPC mechanism itself, not the remote
    procedure call specified by proc_id. The  return code of remote procedure
    call is included as one of the outparams. The unique RPC id which defines the
    procedure as well as the number of in and out params. The client and server
    side must implement the same in and out params data structures.

Returns:
    The status of request.
    The status of remote procdure call
    out parameters received from remote
    number of out parameters received from remote

See Also:
	BRPC_Open_SocketImpl() and BRPC_Close_SocketImpl()

****************************************************************************/
BERR_Code BRPC_CallProc(
    BRPC_Handle,                /* [in] RPC_handle*/
    unsigned proc_id,           /* [in] RPC ID*/
    const uint32_t *inparams,   /* [in] input parameters to pass*/
    unsigned inparams_num,      /* [in] number of members in inparam's array */
    uint32_t *outparams,        /* [out] out parameters received*/
    unsigned outparams_num,     /* [out] number of members in outparam's array*/
    BERR_Code *retVal           /* [out] RPC operation return code */
);

/***************************************************************************
Summary:
	This function checks any notifications sent from remote.

Description:
    It checks if any notification messages have been received from remote.

Returns:
    The status of request.
    notification event code and data
    device id which sends notification

See Also:

****************************************************************************/
BERR_Code BRPC_CheckNotification(
    BRPC_Handle handle,         /* [in] RPC handle*/
    uint32_t *devId,            /* [out] id of device who sends notification*/
    uint32_t *event,            /* [out] notification event code and data*/
    int32_t timeout             /* [in] timeout period to wait for notification*/
);

/***************************************************************************
Summary:
	This function enable/disable RPC communication.

Description:
    It can enable or disable RPC communication.

Returns:
    Always success

See Also:

****************************************************************************/

BERR_Code BRPC_EnableRPC(
    BRPC_Handle handle,        /* [in] RPC handle*/
    bool toEnable              /* [in] true: enable or false: disable*/
);

#endif

