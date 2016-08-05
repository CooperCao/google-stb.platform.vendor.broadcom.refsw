/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_interface.h"

#include <stddef.h>
#include <assert.h>

DisplayInterfaceResult DisplayInterface_Display(
      const struct DisplayInterface *di, void *surface,
      int render_fence, int *display_fence)
{
   assert(di != NULL);
   assert(di->display != NULL);
   assert(surface != NULL);
   assert(display_fence != NULL);

   return di->display(di->base.context, surface, render_fence, display_fence);
}

bool DisplayInterface_WaitSync(const struct DisplayInterface *di)
{
   assert(di != NULL);

   return di->wait_sync ? di->wait_sync(di->base.context) : false;
}

void DisplayInterface_Stop(const struct DisplayInterface *di)
{
   assert(di != NULL);

   if (di->stop)
      di->stop(di->base.context);
}
