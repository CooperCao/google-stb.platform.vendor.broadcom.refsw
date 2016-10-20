/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_INTERFACE_H__
#define __DISPLAY_INTERFACE_H__

#include "surface_interface.h"
#include <stdbool.h>
#include "interface.h"

typedef enum DisplayInterfaceResult
{
   eDisplayFailed,
   eDisplaySuccessful,
   eDisplayPending /* have to wait on a display_fence before reuse */
} DisplayInterfaceResult;

typedef struct DisplayInterface
{
   Interface base;

   DisplayInterfaceResult (*display)(void *context, void *surface,
         int render_fence, int *display_fence);
   bool (*wait_sync)(void *context);
   void (*stop)(void *context);
} DisplayInterface;

DisplayInterfaceResult DisplayInterface_Display(
      const DisplayInterface *di, void *surface,
      int render_fence, int *display_fence);

bool DisplayInterface_WaitSync(const DisplayInterface *di);

void DisplayInterface_Stop(const DisplayInterface *di);

#endif /* __DISPLAY_INTERFACE_H__ */
