/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "nexus_memory.h"
#include <EGL/egl.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*BufferGetRequirements)(
      BEGL_PixmapInfoEXT const *bufferRequirements,
      BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct WlDisplayBinding
{
   NEXUS_HeapHandle heap;
   NEXUS_HeapHandle secure_heap;

   struct wl_display *wl_display;
   struct wl_global *wl_global;
} WlDisplayBinding;

typedef struct WlBufferMemory
{
   BEGL_BufferSettings settings;
   NEXUS_MemoryBlockHandle handle;
   int refs;
} WlBufferMemory;

struct wl_resource;

bool BindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display,
      NEXUS_HeapHandle heap, NEXUS_HeapHandle secure_heap);

bool UnbindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display);

WlBufferMemory *GetWlBufferMemory(struct wl_resource *buffer);

void WlBufferMemoryRefInc(WlBufferMemory *memory);
void WlBufferMemoryRefDec(WlBufferMemory *memory);

#ifdef __cplusplus
}
#endif
