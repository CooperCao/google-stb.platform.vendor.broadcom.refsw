/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bkni.h"

#ifndef BSYSLIB_CALLBACK_H__
#define BSYSLIB_CALLBACK_H__

/*
Summary:
A generic callback signature with only user supplied context
Description:
*/ 
typedef BERR_Code (*BSYSlib_Callback_Signature) 
( 
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2 /* second user context parameter [in] */ 
);

/*
Summary:
*/
typedef struct
{
	BSYSlib_Callback_Signature pfDo;
	void * pvParm1;
	int iParm2;
} BSYSlib_Callback;

/*
Summary:
*/
void BSYSlib_Callback_Init(
	BSYSlib_Callback * psCallback
);

/*
Summary:
*/
BERR_Code BSYSlib_Callback_Invoke(
	BSYSlib_Callback * psCallback
);

#endif /* BSYSLIB_CALLBACK_H__ */

