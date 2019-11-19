/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_INTERFACE_H__
#define __DISPLAY_INTERFACE_H__

#include "surface_interface.h"
#include <stdbool.h>
#include "interface.h"
#include "windowinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum DisplayInterfaceResult
{
   eDisplayFailed,
   eDisplayPending /* have to wait on a display_fence before reuse */
} DisplayInterfaceResult;

typedef struct DisplayInterface
{
   Interface base;

   /*
    * Display the next surface on screen.
    *
    * On success the next wait_sync call must result in this surface being
    * displayed, and display_fence must be set to a fence that will be signalled
    * when this surface is no longer used by the display, for example
    * because the next surface is on display or a copy has been made.
    *
    * Display takes ownership of the render fence and it's responsible for
    * closing it. In case of a failure display is not required to wait on the
    * render_fence but it must close it.
    */
   DisplayInterfaceResult (*display)(void *context, void *surface,
         const WindowInfo *windowInfo,
         int render_fence, bool create_display_fence, int *display_fence);

   /*
    * Wait for the next sync event from the display
    *
    * After pending display operation the next sync event guarantees that the
    * last displayed surface is on display. Subsequent calls to this function
    * are used to throttle display if swap interval is greater than one.
    */
   bool (*wait_sync)(void *context);

   /*
    * Release previously displayed surfaces, if they're no longer needed.
    *
    * This is an optional function used to recycle any previously displayed
    * surfaces if they're no longer used.
    */
   void (*release)(void *context);

   /*
    * Stop the display and release the last displayed surface.
    *
    * This call must block until the last display_fence is signalled
    * and the last displayed surface is no longer in use.
    */
   void (*stop)(void *context);
} DisplayInterface;

DisplayInterfaceResult DisplayInterface_Display(
      const DisplayInterface *di, void *surface,
      const WindowInfo *windowInfo,
      int render_fence, bool create_display_fence,
      int *display_fence);

bool DisplayInterface_WaitSync(const DisplayInterface *di);

void DisplayInterface_Release(const DisplayInterface *di);

void DisplayInterface_Stop(const DisplayInterface *di);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_INTERFACE_H__ */
