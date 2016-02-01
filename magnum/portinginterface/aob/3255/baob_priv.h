/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
 ***************************************************************************/

#ifndef BAOB_PRIV_H__
#define BAOB_PRIV_H__

#include "bchp.h"
#include "brpc.h"
#include "brpc_3255.h"
#include "baob.h"
#include "bkni_multi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	The handle for Out-of-Band Downstream module.

Description:

See Also:
	BAOB_Open()

****************************************************************************/

typedef struct BAOB_P_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle hChip;
	BREG_Handle hRegister;
	BINT_Handle hInterrupt;
	BRPC_DevId devId;
	BRPC_Handle hRpc;
	BAOB_CallbackFunc pCallback[BAOB_Callback_eLast];
	void *pCallbackParam[BAOB_Callback_eLast];
	bool enableFEC;						/* enable OOB FEC*/
	BAOB_SpectrumMode spectrum; 		/* current specturm setting*/
	bool isLock;						/* current lock status */
	BKNI_MutexHandle mutex;				/* mutex to protect lock status*/
} BAOB_P_Handle;


#ifdef __cplusplus
}
#endif

#endif

