/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "EGL/egl.h"
#include "display_priv.h"
#include "../helpers/extent.h"
#include "display_nexus.h"
#include "windowinfo.h"

namespace nxpl
{

class Bitmap
{
public:
   Bitmap(void *context, NEXUS_HeapHandle heap, BEGL_PixmapInfoEXT const *bufferRequirements);
   Bitmap(void *context, BEGL_PixmapInfoEXT const *bufferRequirements);

   void Init(void *context, NEXUS_HeapHandle heap, BEGL_PixmapInfoEXT const *bufferRequirements);
   ~Bitmap();

   int GetBpp() const;
   NEXUS_PixelFormat GetNativeFormat() const;

   const BEGL_BufferSettings & GetCreateSettings() const
   {
      return m_settings;
   };

   helper::Extent2D GetExtent2D() const
   {
      return helper::Extent2D(m_settings.width, m_settings.height);
   };

   NEXUS_SurfaceHandle GetSurface() const
   {
      return m_surface;
   };

   void UpdateWindowInfo(nxpl::NativeWindowInfo &info)
   {
      m_info = info;
   };

   nxpl::NativeWindowInfo GetWindowInfo() const
   {
      return m_info;
   }

   NEXUS_VideoOrientation GetOrientation() const
   {
      NEXUS_VideoOrientation orientation;
      switch (m_info.GetType())
      {
      default:
      case NXPL_2D: orientation = NEXUS_VideoOrientation_e2D; break;
      case NXPL_3D_LEFT_RIGHT: orientation = NEXUS_VideoOrientation_e3D_LeftRight; break;
      case NXPL_3D_OVER_UNDER: orientation = NEXUS_VideoOrientation_e3D_OverUnder; break;
      }
      return orientation;
   }

private:
   nxpl::NativeWindowInfo      m_info;
   NXPL_Display               *m_data;
   BEGL_BufferSettings         m_settings;
   NEXUS_SurfaceHandle         m_surface;
   int                         m_vsyncCount;
};

}
