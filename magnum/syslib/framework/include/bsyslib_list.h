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
* NOTE: This module does not follow the magnum convention for BERR_Code
* as the return type of every function that can error out.  The reason
* for this is that it is much more useful to return the values selected
* in this code.
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bsyslib.h"

#ifndef BSYSLIB_LIST_H__
#define BSYSLIB_LIST_H__

/*
Summary:
*/
typedef struct BSYSlib_List_Impl * BSYSlib_List_Handle;

/*
Summary:
*/
typedef struct BSYSlib_List_IteratorImpl * BSYSlib_List_IteratorHandle;

/*
Summary:
*/
BSYSlib_List_Handle BSYSlib_List_Create(void);

/*
Summary:
*/
void BSYSlib_List_Destroy(
	BSYSlib_List_Handle hList
);

/*
Summary:
*/
void BSYSlib_List_AddElement(
	BSYSlib_List_Handle hList,
	void * pvElement
);

/*
Summary:
*/
void BSYSlib_List_RemoveElement(
	BSYSlib_List_Handle hList,
	void * pvElement
);

/*
Summary:
*/
bool BSYSlib_List_IsEmpty(
	BSYSlib_List_Handle hList
);

/*
Summary:
*/
bool BSYSlib_List_IsEmpty_isr(
	BSYSlib_List_Handle hList
);

/*
Summary:
*/
BSYSlib_List_IteratorHandle BSYSlib_List_AcquireIterator(
	BSYSlib_List_Handle hList
);

/*
Summary:
*/
BSYSlib_List_IteratorHandle BSYSlib_List_AcquireIterator_isr(
	BSYSlib_List_Handle hList
);

/*
Summary:
*/
void BSYSlib_List_ReleaseIterator(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void BSYSlib_List_ReleaseIterator_isr(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void BSYSlib_List_ResetIterator(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void BSYSlib_List_ResetIterator_isr(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
bool BSYSlib_List_HasNext(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
bool BSYSlib_List_HasNext_isr(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void * BSYSlib_List_Next(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void * BSYSlib_List_Next_isr(
	BSYSlib_List_IteratorHandle hIterator
);

/*
Summary:
*/
void * BSYSlib_List_GetByIndex(
	BSYSlib_List_Handle hList,
	unsigned int uiIndex
);

/*
Summary:
*/
void * BSYSlib_List_GetByIndex_isr(
	BSYSlib_List_Handle hList,
	unsigned int uiIndex
);

/*
Summary:
*/
int BSYSlib_List_IndexOf(
	BSYSlib_List_Handle hList,
	void * pvElement
);

/*
Summary:
*/
int BSYSlib_List_IndexOf_isr(
	BSYSlib_List_Handle hList,
	void * pvElement
);

#endif /* BSYSLIB_LIST_H__ */

