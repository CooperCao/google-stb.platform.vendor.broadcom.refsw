/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "wayland_nexus_client.h"
#include <EGL/egl.h>
#include <wayland-client.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct WlSharedBuffer
{
   BEGL_BufferSettings settings;
   struct wl_buffer *buffer;
   struct wl_buffer_listener listener;
} WlSharedBuffer;

typedef struct WlClient
{
   struct wl_display *display;
   bool disconnect;
   struct wl_registry *registry;
   struct wl_event_queue *buffer_events;
   struct wl_nexus *nexus;
} WlClient;

typedef struct WlWindow
{
   WlClient *client;
   struct wl_egl_window *window;
   struct wl_event_queue *frame_events;
   struct wl_event_queue *release_events;
   struct wl_callback *frame_callback;
} WlWindow;

extern bool CreateWlClient(WlClient *client, struct wl_display *display);
extern void DestroyWlClient(WlClient *client);

extern bool CreateWlSharedBuffer(WlSharedBuffer *buffer, WlClient *client,
      const BEGL_BufferSettings *settings,
      void (*release)(void *data, struct wl_buffer *wl_buffer), void *data);
extern void DestroyWlSharedBuffer(WlSharedBuffer *buffer);

extern bool CreateWlWindow(WlWindow *window, WlClient *client,
      struct wl_egl_window *wl_egl_window);
extern void DestroyWlWindow(WlWindow *window);
extern bool DisplayWlSharedBuffer(WlWindow *window, WlSharedBuffer *buffer,
      int dx, int dy);
extern bool WaitWlFrame(WlWindow *window);
extern int WaitWlRelease(WlWindow *window, bool block);

#ifdef __cplusplus
}
#endif
