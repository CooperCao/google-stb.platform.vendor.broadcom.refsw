
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

