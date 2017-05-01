/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"

static int init_refcount;

VCOS_STATUS_T vcos_init(void)
{
   VCOS_STATUS_T st = VCOS_SUCCESS;

   vcos_global_lock();

   if (init_refcount++ == 0)
      st = vcos_platform_init();

   vcos_global_unlock();

   return st;
}

void vcos_deinit(void)
{
   vcos_global_lock();

   assert(init_refcount > 0);

   if (init_refcount > 0 && --init_refcount == 0)
      vcos_platform_deinit();

   vcos_global_unlock();
}

#if defined(__GNUC__) && (__GNUC__ > 2)

void vcos_ctor(void) __attribute__((constructor, used));

void vcos_ctor(void)
{
   vcos_init();
}

void vcos_dtor(void) __attribute__((destructor, used));

void vcos_dtor(void)
{
   vcos_deinit();
}

#endif
