/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "EGL/egl.h"
#include "../helpers/extent.h"
#include "display_nx/nxwindowinfo.h"

namespace wlpl
{

class NxBitmap
{
public:
   NxBitmap() :
         m_surface(NULL)
   {
   }

   NxBitmap(NEXUS_HeapHandle heap, const BEGL_BufferSettings *settings);

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
