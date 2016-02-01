/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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

/*=********************************************
Generic implemention.
This routes the generic calls into the implementation specific module.
*********************************************/
#include "bstd.h"
#include "brpc_plat.h"

BDBG_MODULE(BRPC);

BERR_Code BRPC_CallProc(BRPC_Handle handle, unsigned proc_id, const uint32_t *inparams, unsigned inparams_num,
	uint32_t *outparams, unsigned outparams_num, BERR_Code *retVal)
{
	if ( handle->rpc_disabled ) {
		*retVal = 0;
		return BERR_SUCCESS;
	} else
	/* Just call into the implementation specific code. */
		return handle->callProc(handle, proc_id, inparams, inparams_num, outparams, outparams_num, retVal);
}

BERR_Code BRPC_CheckNotification(BRPC_Handle handle, uint32_t *devId, uint32_t *event, int32_t timeoutMs)
{
	if ( handle->rpc_disabled ) {
		*event = 0;
		return BERR_SUCCESS;
	} else
		/* Just call into the implementation specific code. */
		return handle->checkNotification(handle, devId, event, timeoutMs);
}

BERR_Code BRPC_EnableRPC(BRPC_Handle handle, bool toEnable)
{
	handle->rpc_disabled = toEnable ? 0:1;
	return BERR_SUCCESS;
}
