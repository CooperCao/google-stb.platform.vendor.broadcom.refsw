/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/begl_platform.h>
#include "../helpers/extent.h"
#include "wl_client.h"

#include <memory>
#include <stdio.h>

namespace wlpl
{

class WlBitmap
{
public:
   WlBitmap(WlClient *wlc, uint32_t width, uint32_t height,
         BEGL_BufferFormat format, bool secure,
         void (*release)(void *data, struct wl_buffer *wl_buffer), void *data) :
         m_dx(0),
         m_dy(0)
   {
      if (!CreateWlSharedBuffer(&m_sharedBuffer, wlc, width, height, format,
            secure, release, data))
      {
         fprintf(stderr, "failed to create Wayland shared buffer\n");
         exit(1);
      }
   }

   ~WlBitmap()
   {
      DestroyWlSharedBuffer(&m_sharedBuffer);
   }

   helper::Extent2D GetExtent2D() const
   {
      return helper::Extent2D(
            m_sharedBuffer.settings.width,
            m_sharedBuffer.settings.height);
   }

   WlSharedBuffer &GetWlBuffer()
   {
      return m_sharedBuffer;
   }

   int GetWindowDx() const
   {
      return m_dx;
   }

   int GetWindowDy() const
   {
      return m_dy;
   }

   void SetWindowDisplacement(int dx, int dy)
   {
      m_dx = dx;
      m_dy = dy;
   }

private:
   WlSharedBuffer m_sharedBuffer;
   int m_dx;
   int m_dy;
};
}
