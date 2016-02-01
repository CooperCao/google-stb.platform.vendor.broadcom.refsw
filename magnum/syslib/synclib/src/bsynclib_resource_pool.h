
#include "bsyslib_list.h"

#ifndef BSYNCLIB_RESOURCE_POOL_H__
#define BSYNCLIB_RESOURCE_POOL_H__

typedef struct
{
	BSYSlib_List_Handle hResources;
	BSYSlib_List_Handle hFree;
} BSYNClib_ResourcePool;

BSYNClib_ResourcePool * BSYNClib_ResourcePool_Create(void);

void BSYNClib_ResourcePool_Destroy(BSYNClib_ResourcePool * psPool);

void BSYNClib_ResourcePool_Add(BSYNClib_ResourcePool * psPool, void * pvResource);

void BSYNClib_ResourcePool_Remove(BSYNClib_ResourcePool * psPool, void * pvResource);

void * BSYNClib_ResourcePool_Acquire(BSYNClib_ResourcePool * psPool);

void BSYNClib_ResourcePool_Release(BSYNClib_ResourcePool * psPool, void * pvResource);

#endif /* BSYNCLIB_RESOURCE_POOL_H__ */

