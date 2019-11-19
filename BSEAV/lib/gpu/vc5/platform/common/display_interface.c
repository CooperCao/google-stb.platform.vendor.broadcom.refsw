/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_interface.h"

#include <stddef.h>
#include <assert.h>

DisplayInterfaceResult DisplayInterface_Display(
      const DisplayInterface *di, void *surface,
      const WindowInfo *windowInfo,
      int render_fence, bool create_display_fence,
      int *display_fence)
{
   assert(di != NULL);
   assert(di->display != NULL);
   assert(surface != NULL);
   assert(display_fence != NULL);

   return di->display(di->base.context, surface, windowInfo, render_fence, create_display_fence, display_fence);
}

bool DisplayInterface_WaitSync(const DisplayInterface *di)
{
   assert(di != NULL);

   return di->wait_sync ? di->wait_sync(di->base.context) : false;
}

void DisplayInterface_Release(const DisplayInterface *di)
{
   assert(di != NULL);

   if (di->release)
      di->release(di->base.context);
}

void DisplayInterface_Stop(const DisplayInterface *di)
{
   assert(di != NULL);

   if (di->stop)
      di->stop(di->base.context);
}
