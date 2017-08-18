/******************************************************************************
 * Copyright (C) 2004-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "bsyslib_list.h"
#include "bsyslib_list_priv.h"

BDBG_MODULE(syslib_list);

BSYSlib_List_Handle BSYSlib_List_Create(void)
{
	BSYSlib_List_Handle hList;

	BDBG_ENTER(BSYSlib_List_Create);

	hList = (BSYSlib_List_Handle)BKNI_Malloc(sizeof(struct BSYSlib_List_Impl));

	if (hList)
	{
		BKNI_Memset(hList, 0, sizeof(struct BSYSlib_List_Impl));
		/* create at least 1 iterator to be used at isr context */
		hList->hIterator_isr = BSYSlib_List_P_CreateIterator(hList);
	}

	BDBG_LEAVE(BSYSlib_List_Create);
	return hList;
}

void BSYSlib_List_Destroy(
	BSYSlib_List_Handle hList
)
{
	BSYSlib_List_Entry * psEntry;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYSlib_List_Destroy);

	BDBG_ASSERT(hList);

	/* destroy all entries */
	for (psEntry = BLST_Q_FIRST(&hList->sEntries); psEntry; psEntry = BLST_Q_FIRST(&hList->sEntries))
	{
		BLST_Q_REMOVE_HEAD(&hList->sEntries, sLink);
		BKNI_Free(psEntry);
	}

	/* destroy all used iterators */
	/* coverity[alias] */
	/* coverity[use_after_free] */	
	for (hIterator = BLST_Q_FIRST(&hList->sUsedIterators); hIterator; hIterator = BLST_Q_FIRST(&hList->sUsedIterators))
	{
		BSYSlib_List_P_DestroyIterator(hIterator);
	}

	/* destroy all free iterators */
	/* coverity[alias] */
	/* coverity[use_after_free] */	
	for (hIterator = BLST_Q_FIRST(&hList->sFreeIterators); hIterator; hIterator = BLST_Q_FIRST(&hList->sFreeIterators))
	{
		BSYSlib_List_P_DestroyIterator(hIterator);
	}

	BSYSlib_List_P_DestroyIterator(hList->hIterator_isr);
	
	BKNI_Free(hList);

	BDBG_LEAVE(BSYSlib_List_Destroy);
}

void BSYSlib_List_AddElement(
	BSYSlib_List_Handle hList,
	void * pvElement
)
{
	BSYSlib_List_Entry * psEntry;

	BDBG_ENTER(BSYSlib_List_AddElement);

	BDBG_ASSERT(hList);
	BDBG_ASSERT(pvElement); /* NULLs are not allowed to be listed */

	psEntry = (BSYSlib_List_Entry *)BKNI_Malloc(sizeof(struct BSYSlib_List_Entry));

	if (psEntry)
	{
		psEntry->pvData = pvElement;
		BLST_Q_INSERT_TAIL(&hList->sEntries, psEntry, sLink);
	}

	BDBG_LEAVE(BSYSlib_List_AddElement);
}

void BSYSlib_List_RemoveElement(
	BSYSlib_List_Handle hList,
	void * pvElement
)
{
	BSYSlib_List_Entry * psEntry;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYSlib_List_RemoveElement);

	BDBG_ASSERT(hList);
	BDBG_ASSERT(pvElement);

	for (psEntry = BLST_Q_FIRST(&hList->sEntries); psEntry; psEntry = BLST_Q_NEXT(psEntry, sLink))
	{
		if (psEntry->pvData == pvElement) break;
	}

	if (psEntry)
	{
		/* any open iterators that have this element as current will point to the next element */
		for (hIterator = BLST_Q_FIRST(&hList->sUsedIterators); hIterator; hIterator = BLST_Q_NEXT(hIterator, sUsedLink))
		{
			if (hIterator->psCurrent == psEntry)
			{
				hIterator->psCurrent = BLST_Q_NEXT(psEntry, sLink);
			}
		}

		/* remove the entry from this list */
		BLST_Q_REMOVE(&hList->sEntries, psEntry, sLink);
		BKNI_Free(psEntry);
	}

	BDBG_LEAVE(BSYSlib_List_RemoveElement);
}

BSYSlib_List_IteratorHandle BSYSlib_List_AcquireIterator(
	BSYSlib_List_Handle hList
)
{
	BSYSlib_List_IteratorHandle hIterator = NULL;

	BDBG_ENTER(BSYSlib_List_AcquireIterator);

	BDBG_ASSERT(hList);

	if (BLST_Q_EMPTY(&hList->sFreeIterators))
	{
        /* No free iterators -> create one */
		hIterator = BSYSlib_List_P_CreateIterator(hList);
	}
	else
	{
		hIterator = BLST_Q_FIRST(&hList->sFreeIterators);
		BLST_Q_REMOVE(&hList->sFreeIterators, hIterator, sFreeLink);
		hIterator->hParent = hList;
		hIterator->psCurrent = BLST_Q_FIRST(&hList->sEntries); /* points to first element */
	}

    /* mark this iterator as used */
	BLST_Q_INSERT_TAIL(&hList->sUsedIterators, hIterator, sUsedLink);

	BDBG_LEAVE(BSYSlib_List_AcquireIterator);
	return hIterator;
}

BSYSlib_List_IteratorHandle BSYSlib_List_AcquireIterator_isr(
	BSYSlib_List_Handle hList
)
{
	BSYSlib_List_IteratorHandle hIterator = NULL;

	BDBG_ENTER(BSYSlib_List_AcquireIterator_isr);

	BDBG_ASSERT(hList);

	hIterator = hList->hIterator_isr;
	hIterator->hParent = hList;
	hIterator->psCurrent = BLST_Q_FIRST(&hList->sEntries); /* points to first element */
	hList->hIterator_isr = NULL;

	BDBG_LEAVE(BSYSlib_List_AcquireIterator_isr);
	return hIterator;
}

void BSYSlib_List_ReleaseIterator(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BSYSlib_List_Handle hList;

	BDBG_ENTER(BSYSlib_List_ReleaseIterator);

	BDBG_ASSERT(hIterator);

	hList = hIterator->hParent;

	BLST_Q_REMOVE(&hList->sUsedIterators, hIterator, sUsedLink);
	BLST_Q_INSERT_TAIL(&hList->sFreeIterators, hIterator, sFreeLink);

	BDBG_LEAVE(BSYSlib_List_ReleaseIterator);
}

void BSYSlib_List_ReleaseIterator_isr(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BSYSlib_List_Handle hList;

	BDBG_ENTER(BSYSlib_List_ReleaseIterator_isr);

	BDBG_ASSERT(hIterator);

	hList = hIterator->hParent;

	hList->hIterator_isr = hIterator;

	BDBG_LEAVE(BSYSlib_List_ReleaseIterator_isr);
}

void BSYSlib_List_ResetIterator(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BDBG_ENTER(BSYSlib_List_ResetIterator);
	
	BKNI_EnterCriticalSection();
	BSYSlib_List_ResetIterator_isr(hIterator);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_ResetIterator);
}

void BSYSlib_List_ResetIterator_isr(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BSYSlib_List_Handle hList;

	BDBG_ENTER(BSYSlib_List_ResetIterator_isr);

	BDBG_ASSERT(hIterator);

	hList = hIterator->hParent;

	hIterator->psCurrent = BLST_Q_FIRST(&hList->sEntries); /* points to first element */

	BDBG_LEAVE(BSYSlib_List_ResetIterator_isr);
}

BSYSlib_List_IteratorHandle BSYSlib_List_P_CreateIterator(
	BSYSlib_List_Handle hList
)
{
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYSlib_List_P_CreateIterator);

	BDBG_ASSERT(hList);

	hIterator = (BSYSlib_List_IteratorHandle)BKNI_Malloc(sizeof(struct BSYSlib_List_IteratorImpl));

	if (hIterator)
	{
		hIterator->hParent = hList;
		hIterator->psCurrent = BLST_Q_FIRST(&hList->sEntries); /* points to first element */
	}

	BDBG_LEAVE(BSYSlib_List_P_CreateIterator);
	return hIterator;
}

void BSYSlib_List_P_DestroyIterator(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BSYSlib_List_Handle hList;
	BSYSlib_List_IteratorHandle temp;

	BDBG_ENTER(BSYSlib_List_P_DestroyIterator);

	BDBG_ASSERT(hIterator);

	hList = hIterator->hParent;

	for (temp = BLST_Q_FIRST(&hList->sUsedIterators); temp; temp = BLST_Q_NEXT(temp, sUsedLink)) {
		if (temp == hIterator) {
	        BLST_Q_REMOVE(&hList->sUsedIterators, hIterator, sUsedLink);
			break;
		}
	}

	for (temp = BLST_Q_FIRST(&hList->sFreeIterators); temp; temp = BLST_Q_NEXT(temp, sFreeLink)) {
		if (temp == hIterator) {
			BLST_Q_REMOVE(&hList->sFreeIterators, hIterator, sFreeLink);
			break;
		}
	}

	BKNI_Free(hIterator);

	BDBG_LEAVE(BSYSlib_List_P_DestroyIterator);
}

bool BSYSlib_List_HasNext(
	BSYSlib_List_IteratorHandle hIterator
)
{
	bool bHasNext;

	BDBG_ENTER(BSYSlib_List_HasNext);
	
	BKNI_EnterCriticalSection();
	bHasNext = BSYSlib_List_HasNext_isr(hIterator);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_HasNext);
	return bHasNext;
}

bool BSYSlib_List_HasNext_isr(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BDBG_ENTER(BSYSlib_List_HasNext_isr);

	BDBG_ASSERT(hIterator);

	BDBG_LEAVE(BSYSlib_List_HasNext_isr);
	return hIterator->psCurrent != NULL;
}

void * BSYSlib_List_Next(
	BSYSlib_List_IteratorHandle hIterator
)
{
	void * pvElement;

	BDBG_ENTER(BSYSlib_List_Next);
	
	BKNI_EnterCriticalSection();
	pvElement = BSYSlib_List_Next_isr(hIterator);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_Next);
	return pvElement;
}

void * BSYSlib_List_Next_isr(
	BSYSlib_List_IteratorHandle hIterator
)
{
	BSYSlib_List_Entry * psEntry;
	void * pvElement = NULL;

	BDBG_ENTER(BSYSlib_List_Next_isr);

	BDBG_ASSERT(hIterator);

	psEntry = hIterator->psCurrent;

	if (psEntry) /* do we still have elements in the list? */
	{
		pvElement = psEntry->pvData;
		psEntry = BLST_Q_NEXT(psEntry, sLink);
	}

	hIterator->psCurrent = psEntry;

	BDBG_LEAVE(BSYSlib_List_Next_isr);
	return pvElement;
}

void * BSYSlib_List_GetByIndex(
	BSYSlib_List_Handle hList,
	unsigned int uiIndex
)
{
	void * pvElement;

	BDBG_ENTER(BSYSlib_List_GetByIndex);

	BKNI_EnterCriticalSection();
	pvElement = BSYSlib_List_GetByIndex_isr(hList, uiIndex);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_GetByIndex);
	return pvElement;
}

void * BSYSlib_List_GetByIndex_isr(
	BSYSlib_List_Handle hList,
	unsigned int uiIndex
)
{
	unsigned int i = 0;
	void * pvElement = NULL;
	BSYSlib_List_Entry * psEntry;

	BDBG_ENTER(BSYSlib_List_GetByIndex_isr);

	BDBG_ASSERT(hList);

	i = 0;
	for (psEntry = BLST_Q_FIRST(&hList->sEntries); psEntry != NULL; psEntry = BLST_Q_NEXT(psEntry, sLink))
	{
		if (i == uiIndex)
		{
			pvElement = psEntry->pvData;
			break;
		}

		i++;
	}

	BDBG_LEAVE(BSYSlib_List_GetByIndex_isr);
	return pvElement;
}

int BSYSlib_List_IndexOf(
	BSYSlib_List_Handle hList,
	void * pvElement
)
{
	int iIndex;

	BDBG_ENTER(BSYSlib_List_IndexOf);

	BKNI_EnterCriticalSection();
	iIndex = BSYSlib_List_IndexOf_isr(hList, pvElement);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_IndexOf);
	return iIndex;
}

int BSYSlib_List_IndexOf_isr(
	BSYSlib_List_Handle hList,
	void * pvElement
)
{
	BSYSlib_List_Entry * psEntry;
	unsigned int i = 0;
	int iIndex = -1;

	BDBG_ENTER(BSYSlib_List_IndexOf_isr);

	BDBG_ASSERT(hList);

	i = 0;
	for (psEntry = BLST_Q_FIRST(&hList->sEntries); psEntry; psEntry = BLST_Q_NEXT(psEntry, sLink))
	{
		if (psEntry->pvData == pvElement)
		{
			iIndex = i;
			break;
		}

		i++;
	}

	BDBG_LEAVE(BSYSlib_List_IndexOf_isr);
	return iIndex;
}

bool BSYSlib_List_IsEmpty(
	BSYSlib_List_Handle hList
)
{
	bool bEmpty;

	BDBG_ENTER(BSYSlib_List_IsEmpty);

	BKNI_EnterCriticalSection();
	bEmpty = BSYSlib_List_IsEmpty_isr(hList);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYSlib_List_IsEmpty);
	
	return bEmpty;
}

bool BSYSlib_List_IsEmpty_isr(
	BSYSlib_List_Handle hList
)
{
	bool bEmpty = false;
	
	BDBG_ENTER(BSYSlib_List_IsEmpty_isr);

	BDBG_ASSERT(hList);

	bEmpty = BLST_Q_EMPTY(&hList->sEntries);

	BDBG_LEAVE(BSYSlib_List_IsEmpty_isr);
	return bEmpty;
}
