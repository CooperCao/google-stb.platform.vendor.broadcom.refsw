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
#ifndef __BRPC_PLAT_H
#define __BRPC_PLAT_H

#include "brpc.h"

typedef BERR_Code (*BRPC_P_CallProc)(
	BRPC_Handle,
	unsigned proc_id,
	const uint32_t *inparams,
	unsigned inparams_num,
	uint32_t *outparams,
	unsigned outparams_num,
	BERR_Code *retVal
	);

typedef BERR_Code (*BRPC_P_CheckNotification)(
	BRPC_Handle handle,
	uint32_t *devId,
	uint32_t *event,
	int32_t timeoutMs
	);

struct BRPC_P_Handle {
	BRPC_P_CallProc callProc; /* use a function pointer for the call proc. this allows us to support multiple RPC
		implementations at runtime if needed. */
	BRPC_P_CheckNotification checkNotification; /* ditto */
	void *private_data; /* implementation specific data */
	bool rpc_disabled; /* if RPC is disabled*/
	uint32_t prev_transactionId; /*previous transactionID*/
};
#endif
