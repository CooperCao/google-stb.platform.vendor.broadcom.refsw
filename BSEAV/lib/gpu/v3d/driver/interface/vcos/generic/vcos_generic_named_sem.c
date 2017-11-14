/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "interface/vcos/vcos.h"
#include <string.h>
#include <assert.h>

/**
  * \file
  *
  * Named semaphores, primarily for VCFW.
  *
  * Does not actually work across processes; merely emulate the API.
  *
  * The client initialises a VCOS_NAMED_SEMAPHORE_T, but this merely
  * points at the real underlying VCIS_NAMED_SEMAPHORE_IMPL_T.
  *
  *      semaphore_t  ---\
  *                       ----- semaphore_impl_t
  *      semaphore_t  ---/
  *                     /
  *      semaphore_t  -/
  *
  */

/* Maintain a simple array of actual semaphores */
#define MAX_SEMS  256

/** Each actual real semaphore is stored in one of these. Clients just
  * get a structure with a pointer to this in it.
  */
typedef struct VCOS_NAMED_SEMAPHORE_IMPL_T
{
   VCOS_SEMAPHORE_T sem;                        /**< Actual underlying semaphore */
   char name[VCOS_NAMED_SEMAPHORE_NAMELEN];     /**< Name of semaphore, copied */
   unsigned refs;                               /**< Reference count */
} VCOS_NAMED_SEMAPHORE_IMPL_T;

static VCOS_MUTEX_T lock;
static VCOS_NAMED_SEMAPHORE_IMPL_T sems[MAX_SEMS];

VCOS_STATUS_T _vcos_named_semaphore_init(void)
{
   memset(sems,0,sizeof(sems));
   return vcos_mutex_create(&lock,"vcosnmsem");
}

void _vcos_named_semaphore_deinit(void)
{
   vcos_mutex_delete(&lock);
}

VCOS_STATUS_T
vcos_generic_named_semaphore_create(VCOS_NAMED_SEMAPHORE_T *sem, const char *name, VCOS_UNSIGNED count)
{
   VCOS_STATUS_T status;
   int i, name_len;

   vcos_mutex_lock(&lock);

   /* do we already have this semaphore? */

   for (i=0; i<MAX_SEMS; i++)
   {
      if (sems[i].refs && strcmp(name, sems[i].name) == 0)
      {
         sems[i].refs++;
         sem->actual = sems+i;
         sem->sem = &sems[i].sem;
         vcos_mutex_unlock(&lock);
         return VCOS_SUCCESS;
      }
   }

   /* search for unused semaphore */

   name_len = strlen(name);

   if (name_len >= VCOS_NAMED_SEMAPHORE_NAMELEN)
   {
      assert(0);
      vcos_mutex_unlock(&lock);
      return VCOS_EINVAL;
   }

   for (i=0; i<MAX_SEMS; i++)
   {
      if (sems[i].refs == 0)
      {
         status = vcos_semaphore_create(&sems[i].sem, name, count);
         if (status == VCOS_SUCCESS)
         {
            sems[i].refs = 1;
            strcpy(sems[i].name, name); /* already checked length! */
            sem->actual = sems+i;
            sem->sem = &sems[i].sem;
         }
         vcos_mutex_unlock(&lock);
         return status;
      }
   }

   vcos_mutex_unlock(&lock);

   return VCOS_ENOSPC;
}

void vcos_named_semaphore_delete(VCOS_NAMED_SEMAPHORE_T *sem)
{
   VCOS_NAMED_SEMAPHORE_IMPL_T *actual = sem->actual;
   vcos_mutex_lock(&lock);

   assert(actual->refs); /* if this fires, the semaphore has already been deleted */

   if (--actual->refs == 0)
   {
      vcos_semaphore_delete(&actual->sem);
   }
   vcos_mutex_unlock(&lock);
}
