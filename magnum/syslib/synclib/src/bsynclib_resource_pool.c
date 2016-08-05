/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "bstd.h"
#include "bsyslib_list.h"
#include "bsynclib_resource_pool.h"

BDBG_MODULE(synclib_resource_pool);

BSYNClib_ResourcePool * BSYNClib_ResourcePool_Create(void)
{
	BSYNClib_ResourcePool * psPool;

	BDBG_ENTER(BSYNClib_ResourcePool_Create);

	psPool = (BSYNClib_ResourcePool *)BKNI_Malloc(sizeof(BSYNClib_ResourcePool));

	if (psPool)
	{
		psPool->hResources = BSYSlib_List_Create();
		psPool->hFree = BSYSlib_List_Create();
	}

	BDBG_LEAVE(BSYNClib_ResourcePool_Create);
	return psPool;
}

void BSYNClib_ResourcePool_Destroy(BSYNClib_ResourcePool * psPool)
{
	BDBG_ENTER(BSYNClib_ResourcePool_Destroy);
	BDBG_ASSERT(psPool);
	BSYSlib_List_Destroy(psPool->hResources);
	BSYSlib_List_Destroy(psPool->hFree);
	BKNI_Free(psPool);
	BDBG_LEAVE(BSYNClib_ResourcePool_Destroy);
}

void BSYNClib_ResourcePool_Add(BSYNClib_ResourcePool * psPool, void * pvResource)
{
	BDBG_ENTER(BSYNClib_ResourcePool_Add);
	BDBG_ASSERT(psPool);
	BDBG_ASSERT(pvResource);
	BSYSlib_List_AddElement(psPool->hResources, pvResource);
	BSYSlib_List_AddElement(psPool->hFree, pvResource);
	BDBG_LEAVE(BSYNClib_ResourcePool_Add);
}

void BSYNClib_ResourcePool_Remove(BSYNClib_ResourcePool * psPool, void * pvResource)
{
	BDBG_ENTER(BSYNClib_ResourcePool_Remove);
	BDBG_ASSERT(psPool);
	BDBG_ASSERT(pvResource);
	BSYSlib_List_RemoveElement(psPool->hResources, pvResource);
	BSYSlib_List_RemoveElement(psPool->hFree, pvResource);
	BDBG_LEAVE(BSYNClib_ResourcePool_Remove);
}

void * BSYNClib_ResourcePool_Acquire(BSYNClib_ResourcePool * psPool)
{
	void * pvResource = NULL;

	BDBG_ENTER(BSYNClib_ResourcePool_Acquire);

	BDBG_ASSERT(psPool);

	if (!BSYSlib_List_IsEmpty(psPool->hFree))
	{
		pvResource = BSYSlib_List_GetByIndex(psPool->hFree, 0);
		BSYSlib_List_RemoveElement(psPool->hFree, pvResource);
	}

	BDBG_LEAVE(BSYNClib_ResourcePool_Acquire);
	return pvResource;
}

void BSYNClib_ResourcePool_Release(BSYNClib_ResourcePool * psPool, void * pvResource)
{
	BDBG_ENTER(BSYNClib_ResourcePool_Release);
	BDBG_ASSERT(psPool);
	BDBG_ASSERT(pvResource);
	BSYSlib_List_AddElement(psPool->hFree, pvResource);
	BDBG_LEAVE(BSYNClib_ResourcePool_Release);
}
