/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/begl_platform.h>
#include "../helpers/extent.h"
#include "display_nx/nxwindowinfo.h"
#include "nexus_surface.h"
#include <stdint.h>

namespace wlpl
{

class NxBitmap
{
public:
   NxBitmap(NEXUS_HeapHandle heap, uint32_t width, uint32_t height,
         BEGL_BufferFormat format);

   ~NxBitmap();

   int GetBpp() const;

   void GetCreateSettings(BEGL_BufferSettings *p) const
   {
      if (p)
         *p = m_settings;
   }

   helper::Extent2D GetExtent2D() const
   {
      return helper::Extent2D(m_settings.width, m_settings.height);
   }

   NEXUS_SurfaceHandle GetSurface() const
   {
      return m_surface;
   }

   void UpdateWindowInfo(wlpl::NxWindowInfo &info)
   {
      m_info = info;
   }

   wlpl::NxWindowInfo GetWindowInfo() const
   {
      return m_info;
   }

private:
   wlpl::NxWindowInfo m_info;
   BEGL_BufferSettings m_settings;
   NEXUS_SurfaceHandle m_surface;
};

}
