/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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

#include "blst_queue.h"
#include "bsyslib.h"

#ifndef BSYSLIB_LIST_PRIV_H__
#define BSYSLIB_LIST_PRIV_H__

typedef struct BSYSlib_List_Entry
{
	BLST_Q_ENTRY(BSYSlib_List_Entry) sLink;
	void * pvData;
} BSYSlib_List_Entry;

struct BSYSlib_List_IteratorImpl
{
	BLST_Q_ENTRY(BSYSlib_List_IteratorImpl) sUsedLink;
	BLST_Q_ENTRY(BSYSlib_List_IteratorImpl) sFreeLink;
	BSYSlib_List_Handle hParent;
	BSYSlib_List_Entry * psCurrent;
};

struct BSYSlib_List_Impl
{
	BLST_Q_HEAD(BSYSlib_List_Head, BSYSlib_List_Entry) sEntries;
	BLST_Q_HEAD(BSYSlib_List_IteratorHead, BSYSlib_List_IteratorImpl) sUsedIterators;
	BLST_Q_HEAD(BSYSlib_List_FreeIteratorHead, BSYSlib_List_IteratorImpl) sFreeIterators;
	BSYSlib_List_IteratorHandle hIterator_isr;
};

BSYSlib_List_IteratorHandle BSYSlib_List_P_CreateIterator(
	BSYSlib_List_Handle hList
);

void BSYSlib_List_P_DestroyIterator(
	BSYSlib_List_IteratorHandle hIterator
);

#endif /* BSYSLIB_LIST_PRIV_H__ */

